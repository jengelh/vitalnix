/*=============================================================================
Vitalnix User Management Suite
  Copyright © Jan Engelhardt <jengelh [at] linux01 gwdg de>, 2002 - 2005
  -- License restrictions apply (LGPL v2.1)

  This file is part of Vitalnix.
  Vitalnix is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; however ONLY version 2 of the License.

  Vitalnix is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program kit; if not, write to:
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA

  -- For details, see the file named "LICENSE.LGPL2"
=============================================================================*/
#include <sys/file.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libHX.h>
#include "include/accdb.h"
#include "include/accdb_int.h"

#define TOUCH_GROUP_TAG(i) (sp->fgroup.is_chg = (i))
#define TOUCH_USER_TAG(i) \
    (sp->fpasswd.is_chg = sp->fshadow.is_chg = sp->fvxshadow.is_chg = (i))
#define RWLOCKED(sp) (!!((sp)->flags & ADB_WRLOCK))
#define RWLOCK_CHK(sp) if(!RWLOCKED(sp)) { return -EPERM; }
#define array_size(a) (sizeof(a) / sizeof(*a))

enum {
    IDF_NORMAL = 0,
    IDF_SYSTEM,
    CONFIG_READ  = 0,
    CONFIG_FREE,
};

struct spdb_file {
    char *path;
    FILE *fp;
    int fd;
    char is_chg;
};

struct shadow_state {
    unsigned long flags;
    long uid_min, uid_max, gid_min, gid_max;
    struct HXdeque *user, *group;
    struct spdb_file fpasswd, fshadow, fvxshadow, fgroup;
};

static int db_open(struct shadow_state *, int);
static void db_close(struct shadow_state *);
static struct HXdeque *db_read_passwd(FILE *);
static struct HXdeque *db_read_shadow(FILE *, struct HXdeque *);
static struct HXdeque *db_read_vxshadow(FILE *, struct HXdeque *);
static struct HXdeque *db_read_groups(FILE *);
static void db_flush(struct shadow_state *, int);
static void db_flush_users(struct shadow_state *);
static void db_flush_groups(struct shadow_state *);
static void db_write_passwd(FILE *, const struct adb_user *);
static void db_write_shadow(FILE *, const struct adb_user *);
static void db_write_vxshadow(FILE *, const struct adb_user *);

static long find_next_uid(struct shadow_state *, unsigned int);
static long find_next_gid(struct shadow_state *, unsigned int);
static inline void free_single_group(struct adb_group *);
static void free_groups(struct shadow_state *);
static inline void free_single_user(struct adb_user *);
static void free_users(struct shadow_state *);
static inline struct adb_group *get_group(struct HXdeque *, const char *);
static inline struct adb_user *get_user(struct HXdeque *, const char *);
static void read_config(struct shadow_state *, unsigned int);
static void truncate_here(FILE *);

MODULE_NAME("Shadow back-end module");
MODULE_DESC("for /etc/passwd and shadow");
MODULE_INFO("Jan Engelhardt <jengelh [at] linux01 gwdg de>, 2002 - 2005");

static char *Path_passwd   = "/etc/passwd",
            *Path_shadow   = "/etc/shadow",
            *Path_vxshadow = "/etc/vxshadow",
            *Path_group    = "/etc/group";

//-----------------------------------------------------------------------------
int am_init(struct adb_module *this, void *private_data) {
    struct shadow_state *sp;

    if((sp = this->state = calloc(1, sizeof(struct shadow_state))) == NULL) {
        return -errno;
    }

    sp->fpasswd.path   = HX_strdup(Path_passwd);
    sp->fshadow.path   = HX_strdup(Path_shadow);
    sp->fvxshadow.path = HX_strdup(Path_vxshadow);
    sp->fgroup.path    = HX_strdup(Path_group);
    sp->fpasswd.fd     = -1;
    sp->fshadow.fd     = -1;
    sp->fvxshadow.fd   = -1;
    sp->fgroup.fd      = -1;
    read_config(sp, CONFIG_READ);
    return 1;
}

int am_open(struct adb_module *this, long flags) {
    struct shadow_state *sp = this->state;
    int eax;

    if((eax = db_open(sp, flags & ADB_WRLOCK)) <= 0) {
        am_close(this);
        errno = -eax;
        return eax;
    }
    return 1;
}

int am_lock(struct adb_module *this) {
    struct shadow_state *sp = this->state;
    if(RWLOCKED(sp)) {
        return 1;
    }
    db_close(sp);
    free_users(sp);
    free_groups(sp);
    return db_open(sp, ADB_WRLOCK);
}

int am_unlock(struct adb_module *this) {
    struct shadow_state *sp = this->state;
    struct flock lk = {
        .l_type   = F_UNLCK,
        .l_whence = SEEK_SET,
        .l_start  = 0,
        .l_len    = 0,
    };
    if(!RWLOCKED(sp)) {
        return 1;
    }
    if(fcntl(sp->fpasswd.fd, F_SETLK, &lk) != 0) {
        return -errno;
    }
    sp->flags &= ~ADB_WRLOCK;
    return 1;
}

int am_close(struct adb_module *this) {
    struct shadow_state *sp = this->state;
    db_close(sp);
    free_users(sp);
    free_groups(sp);
    read_config(sp, CONFIG_FREE);
    free(sp);
    return 1;
}

long am_modctl(struct adb_module *this, long req, ...) {
    struct shadow_state *sp = this->state;
    int rv = 1;
    va_list argp;
    va_start(argp, req);

    switch(req) {
        case ADB_ADDFLAGS:
            // ...: unsigned long flags_to_add
            sp->flags |= va_arg(argp, unsigned long);
            break;
        case ADB_DELFLAGS:
            // ...: unsigned long flags_to_del
            sp->flags &= ~va_arg(argp, unsigned long);
            break;
        case ADB_FLUSH:
            if((sp->flags & ADB_WRLOCK) == 0) {
                return -EPERM;
            }
            db_flush(sp, 1);
            break;
        case ADB_COUNT_USERS:
            rv = sp->user->itemcount;
            break;
        case ADB_COUNT_GROUPS:
            rv = sp->group->itemcount;
            break;

        case ADB_NEXTUID_SYS:
            rv = find_next_uid(sp, IDF_SYSTEM);
            break;
        case ADB_NEXTUID:
            rv = find_next_uid(sp, IDF_NORMAL);
            break;
        case ADB_NEXTGID_SYS:
            rv = find_next_gid(sp, IDF_SYSTEM);
            break;
        case ADB_NEXTGID:
            rv = find_next_gid(sp, IDF_NORMAL);
            break;

        default:
            rv = -ENOSYS;
            break;
    }

    va_end(argp);
    return rv;
}

//-----------------------------------------------------------------------------
int am_usertrav(struct adb_module *this, void **tp0, struct adb_user *res) {
    const struct HXdeque_node **tp = (const struct HXdeque_node **)tp0;
    const struct adb_user *src;
    struct adb_group qgrp = {.gname = NULL};
    struct shadow_state *sp = this->state;

    if(res == NULL) {
        *tp = sp->user->first;
        return sp->user->itemcount;
    }

    if(*tp == NULL) { return 0; }
    src = (**tp).ptr;

    while(*src->lname == '+' || *src->lname == '-') {
        // skip NIS users
        if((*tp = (**tp).Next) == NULL) { return 0; }
        src = (**tp).ptr;
    }

    memcpy(res, src, sizeof(struct adb_user));
    qgrp.gid = src->gid;
    am_groupinfo(this, &qgrp, &qgrp, 1);
    res->igrp = qgrp.gname;

    *tp = (**tp).Next;
    return 1;
}

int am_userinfo(struct adb_module *this, struct adb_user *rq,
 struct adb_user *dest, size_t s)
{
    struct shadow_state *sp = this->state;
    const struct HXdeque_node *travp = sp->user->first;
    struct adb_user xrq;
    int found = 0;

    if(rq == dest) {
        memcpy(&xrq, rq, sizeof(struct adb_user));
        rq = &xrq;
    }

    while(travp != NULL && (dest == NULL || (dest != NULL && s > 0))) {
        const struct adb_user *src = travp->ptr;

        // happily skip NIS users
        if(*src->lname == '+' || *src->lname == '-' ||
         (rq->lname != NULL && strcmp(rq->lname, src->lname) != 0) ||
         (rq->uid != -1 && rq->uid != src->uid) ||
         (rq->gid != -1 && rq->gid != src->gid)) {
            travp = travp->Next;
            continue;
        }

        if(dest != NULL) {
            struct adb_group qgrp = {.gid = src->gid, .gname = NULL};
            memcpy(dest, src, sizeof(struct adb_user));
            am_groupinfo(this, &qgrp, &qgrp, 1);
            dest->igrp = qgrp.gname;
            ++dest;
            ++found;
            --s;
        } else {
            if(s == 0) { return 1; }
            ++found;
        }
        travp = travp->Next;
    }

    return found;
}

int am_useradd(struct adb_module *this, struct adb_user *u) {
    struct shadow_state *sp = this->state;
    struct adb_group qgrp;
    struct adb_user *nu;
    int eax;

    if(u->lname == NULL || (u->gid == -1 && u->igrp == NULL)) {
        errno = EINVAL;
        return 0;
    }

    RWLOCK_CHK(sp);

    qgrp.gid   = u->gid;
    qgrp.gname = u->igrp;
    eax = am_groupinfo(this, &qgrp, &qgrp, 1);
    if(eax < 0) { return -errno; }
    if(eax == 0) {
        errno = ENOENT;
        return 0;
    }

    /* We have to be careful with strings here, as we need to duplicate them
    for us, leave the original ones intact and not have a single foreign
    pointer in our structs. */
    if((nu = calloc(1, sizeof(struct adb_user))) == NULL) {
        return -errno;
    }

    nu->lname = HX_strdup(u->lname);
    nu->uid   = (u->uid != -1) ? u->uid : find_next_uid(sp, IDF_NORMAL);
    nu->gid   = qgrp.gid;
    nu->gecos = HX_strdup(u->gecos);
    nu->home  = HX_strdup(u->home);
    nu->shell = HX_strdup(u->shell);
    nu->igrp  = qgrp.gname;
    nu->sgrp  = HX_strdup(u->sgrp);

    nu->passwd      = HX_strdup(u->passwd);
    nu->last_change = u->last_change;
    nu->keep_min    = u->keep_min;
    nu->keep_max    = u->keep_max;
    nu->warn_age    = u->warn_age;
    nu->expire      = u->expire;
    nu->inactive    = u->inactive;

    nu->xuid   = HX_strdup(u->xuid);
    nu->sgroup = HX_strdup(u->sgroup);

    // We leave it up to the application to check for duplicates
    HXdeque_push(sp->user, nu);
    TOUCH_USER_TAG(1);
    db_flush(sp, 0);

    // Update info struct
    u->uid = nu->uid;
    u->gid = nu->gid;
    return 1;
}

int am_usermod(struct adb_module *this, struct adb_user *sm,
 struct adb_user *mm)
{
#define UP_INT(__field) \
        if(mm->__field != -1) { sm->__field = mm->__field; }
#define UP_STR(__field) \
        if(mm->__field != NULL) { HX_strclone(&sm->__field, mm->__field); }
    struct shadow_state *sp = this->state;
    struct adb_group qgrp;
    RWLOCK_CHK(sp);

    // XXX: Reduce this overhead (searching user list twice)
    if(am_userinfo(this, sm, NULL, 0) <= 0 ||
     (sm = get_user(sp->user, sm->lname)) == NULL) {
	errno = ENOENT;
	return 0;
    }

    qgrp.gname = sm->igrp;
    qgrp.gid   = sm->gid;
    am_groupinfo(this, &qgrp, &qgrp, 1);

    UP_STR(lname);
    UP_INT(uid);
    UP_INT(gid);
    UP_STR(gecos);
    UP_STR(home);
    UP_STR(shell);
    UP_STR(igrp);
    UP_STR(sgrp);

    UP_STR(passwd);
    UP_INT(last_change);
    UP_INT(keep_min);
    UP_INT(keep_max);
    UP_INT(warn_age);
    UP_INT(expire);
    UP_INT(inactive);

    UP_STR(xuid);
    UP_STR(sgroup);
    UP_INT(defer_del);

    TOUCH_USER_TAG(1);
    db_flush(sp, 0);
    return 1;
#undef UP_INT
#undef UP_STR
}

int am_userdel(struct adb_module *this, struct adb_user *u) {
    struct shadow_state *sp = this->state;
    RWLOCK_CHK(sp);

    if((u = get_user(sp->user, u->lname)) == NULL) {
	errno = ENOENT;
	return 0;
    }

    HXdeque_del(HXdeque_find(sp->user, u));
    free_single_user(u);
    TOUCH_USER_TAG(1);
    db_flush(sp, 0);
    return 1;
}

//-----------------------------------------------------------------------------
int am_grouptrav(struct adb_module *this, void **tp0, struct adb_group *res) {
    const struct HXdeque_node **tp = (const struct HXdeque_node **)tp0;
    struct shadow_state *sp = this->state;
    struct adb_group *src;

    if(res == NULL) {
        *tp = sp->group->first;
        return sp->group->itemcount;
    }

    if(*tp == NULL) { return 0; }
    src = (**tp).ptr;

    while(*src->gname == '+' || *src->gname == '-') {
        // skip NIS groups
        if((*tp = (**tp).Next) == NULL) { return 0; }
        src = (**tp).ptr;
    }

    memcpy(res, src, sizeof(struct adb_group));
    *tp = (**tp).Next;
    return 1;
}

int am_groupinfo(struct adb_module *this, struct adb_group *rq,
 struct adb_group *dest, size_t s)
{
    struct shadow_state *sp = this->state;
    const struct HXdeque_node *travp = sp->group->first;
    struct adb_group xrq;
    int found = 0;

    if(rq == dest) {
        memcpy(&xrq, rq, sizeof(struct adb_group));
        rq = &xrq;
    }

    while(travp != NULL && (dest == NULL || (dest != NULL && s > 0))) {
        const struct adb_group *i = travp->ptr;

        // happily skip NIS groups
        if(*i->gname == '+' || *i->gname == '-' ||
         (rq->gid != -1 && rq->gid != i->gid) ||
         (rq->gname != NULL && strcmp(rq->gname, i->gname) != 0)) {
            travp = travp->Next;
            continue;
        }

        if(dest != NULL) {
            memcpy(dest, i, sizeof(struct adb_group));
            ++dest;
            ++found;
            --s;
        } else {
            return 1;
        }
        travp = travp->Next;
    }

    return found;
}

int am_groupadd(struct adb_module *this, struct adb_group *g) {
    struct shadow_state *sp = this->state;
    struct adb_group *ng;

    if(g->gname == NULL) {
        errno = EINVAL;
        return 0;
    }

    RWLOCK_CHK(sp);

    if((ng = calloc(1, sizeof(struct adb_group))) == NULL) {
        return -errno;
    }
    ng->gname = HX_strdup(g->gname);
    ng->gid   = (g->gid != -1) ? g->gid : find_next_gid(sp, IDF_NORMAL);

    HXdeque_push(sp->group, ng);
    TOUCH_GROUP_TAG(1);
    db_flush(sp, 0);

    // Update info struct
    g->gid = ng->gid;
    return 1;
}

int am_groupmod(struct adb_module *this, struct adb_group *sm,
 struct adb_group *mm)
{
    struct shadow_state *sp = this->state;
    RWLOCK_CHK(sp);

    /* We first check if the group in SM (search mask) is available, then
    replace SM to point to our internal data structures. */
    if(am_groupinfo(this, sm, NULL, 0) <= 0 ||
     (sm = get_group(sp->group, sm->gname)) == NULL) {
        errno = ENOENT;
        return 0;
    }

    if(mm->gname != NULL) { HX_strclone(&sm->gname, mm->gname); }
    if(mm->gid != -1) { sm->gid = mm->gid; }

    TOUCH_GROUP_TAG(1);
    db_flush(sp, 0);
    return 1;
}

int am_groupdel(struct adb_module *this, struct adb_group *g) {
    struct shadow_state *sp = this->state;
    RWLOCK_CHK(sp);

    if((g = get_group(sp->group, g->gname)) == NULL) {
        errno = ENOENT;
        return 0;
    }

    HXdeque_del(HXdeque_find(sp->group, g));
    free_single_group(g);
    TOUCH_GROUP_TAG(1);
    db_flush(sp, 0);
    return 1;
}

//-----------------------------------------------------------------------------
/* The AM_SHADOW locking scheme:
- databases are opened in read mode by default, no shared/read locks
- upon a write attempt (am_useradd, etc.) reopen in read-write mode:
  steps: close db, reopen with rwlock, reread, perform operation
- on db_flush(), write db to disk and release write lock (no close/reread)
*/

static int db_open(struct shadow_state *sp, int flags) {
    const char *fpmode;
    int eax;

    if(flags & ADB_WRLOCK) {
        fpmode = "r+";
    } else {
        fpmode = "r";
    }

#define open_fd(n) ((sp->n.fd = open(sp->n.path, flags)) >= 0)
#define open_fp(n) ((sp->n.fp = fdopen(sp->n.fd, fpmode)) != NULL)

    if(!open_fd(fpasswd) || !open_fd(fgroup)) {
        eax = -errno;
        goto out;
    }

    /* Normal users do not get access to /etc/shadow - but should not be
    blamed for it. Only error out when someone tries to write to it.
    "root" usually succeeds. */
    if(!open_fd(fshadow) && (errno != EACCES || flags != O_RDONLY)) {
        eax = -errno;
        goto out;
    }

    /* /etc/vxshadow is an extension, and it is legal to be missing. It is
    also legal to be non-world-readable. Condition transform:
      1. if(errno == ENOENT || (errno == EACCES && flags == O_RDONLY)) { OK }
      2. if(!(errno == ENOENT || (errno == EACCES && flags == O_RDONLY))) { F }
      3. if(errno != ENOENT && !(errno == EACCES && flags == O_RDONLY)) { F }
      4. if(errno != ENOENT && (errno != EACCES || flags != O_RDONLY)) { FAIL }
    */
    if(!open_fd(fvxshadow) && errno != ENOENT &&
     (errno != EACCES || flags != O_RDONLY)) {
        eax = -errno;
        goto out;
    }

    if(flags == O_RDWR) {
        struct flock lk = {
            .l_type   = F_WRLCK,
            .l_whence = SEEK_SET,
            .l_start  = 0,
            .l_len    = 0,
        };
        
        if(fcntl(sp->fpasswd.fd, F_SETLK, &lk) < 0) {
            fprintf(stderr, "am_shadow: passwd file (%s) currently "
             "write-locked, waiting\n", sp->fpasswd.path);
            if(fcntl(sp->fpasswd.fd, F_SETLKW, &lk) < 0) {
                eax = -errno;
                goto out;
            }
        }
    }

    if(!open_fp(fpasswd) || !open_fp(fgroup) ||
     (sp->fshadow.fd > 0 && !open_fp(fshadow)) ||
     (sp->fvxshadow.fd > 0 && !open_fp(fvxshadow))) {
        goto out;
    }

#undef open_fd
#undef open_fp

    sp->user  = db_read_passwd(sp->fpasswd.fp);
    sp->group = db_read_groups(sp->fgroup.fp);

    if(sp->fshadow.fp != NULL) {
        db_read_shadow(sp->fshadow.fp, sp->user);
    }
    if(sp->fvxshadow.fp != NULL) {
        db_read_vxshadow(sp->fvxshadow.fp, sp->user);
    }

    sp->flags = flags;
    return 1;

 out:
    eax = -errno;
    db_close(sp);
    errno = -eax;
    return eax;
}

static void db_close(struct shadow_state *sp) {
#define close_fpd(n) \
    if(sp->n.fp != NULL) { \
        fclose(sp->n.fp); \
        sp->n.fd = -1; \
    } \
    if(sp->n.fd >= 0) { close(sp->n.fd); }

    if(sp->fpasswd.is_chg + sp->fshadow.is_chg + sp->fvxshadow.is_chg +
     sp->fgroup.is_chg > 0) {
        db_flush(sp, 1);
    }

    close_fpd(fvxshadow);
    close_fpd(fgroup);
    close_fpd(fshadow);
    close_fpd(fpasswd);
    return;
#undef close_fpd
}

static struct HXdeque *db_read_passwd(FILE *fp) {
    /* In this function, we read all users into memory. This speeds processing
    up, as seeking and scanning in userinfo() / groupinfo() would take long
    otherwise. We also have it handy in a struct. Doing so also allows us to
    commit all changes to the user database at once (the usual case with no
    ADB_SYNC flag set).

    Strings are allocated by us -- and freed by us. The char * pointers the
    application gets are only for reading (and dup'ing). */

    struct HXdeque *dq;
    struct adb_user *u;
    char *ln = NULL;
    if((dq = HXdeque_init()) == NULL) { return NULL; }

    while(HX_getl(&ln, fp) != NULL) {
        char *data[8];

        if(*ln == '#') { continue; }
        if((u = calloc(1, sizeof(struct adb_user))) == NULL) { break; }
        HX_chomp(ln);
        memset(data, 0, sizeof(data));
        if(HX_split5(ln, ":", array_size(data), data) < 4) {
            fprintf(stderr, "am_shadow: bogus line in passwd file\n");
            continue;
        }

        u->lname = HX_strdup(data[0]);
        u->uid   = strtol(data[2], NULL, 0);
        u->gid   = strtol(data[3], NULL, 0);
        u->gecos = HX_strdup(data[4]);
        u->home  = HX_strdup(data[5]);
        u->shell = HX_strdup(data[6]);

        /* Anything after the seven main fields is "reserved for future use",
        (read: or private use), so we need to keep that intact. */
        u->priv = calloc(1, sizeof(void *) * 2);
        ((char **)u->priv)[0] = data[7];

        // In case there is no shadow entry, we need some recovery values
        u->keep_min    = 0;
        u->keep_max    = ADB_KEEPMAX_DEFL;
        u->warn_age    = ADB_WARNAGE_DEFL;
        u->last_change = 0;
        u->expire      = ADB_EXPIRE_NEVER;
        u->inactive    = ADB_INACTIVE_NEVER;

        /* Stuff is pushed in a linked list. The list is unordered, so it has
        the same order as /etc/passwd. This is intentional (and suffices). */
        HXdeque_push(dq, u);
    }

    hmc_free(ln);
    return dq;
}

static struct HXdeque *db_read_shadow(FILE *fp, struct HXdeque *dq) {
    // Update the sp->user table with the data from /etc/shadow
    struct adb_user *u = NULL;
    char *ln = NULL;

    if(dq->itemcount == 0) {
        /* Hai, what a hopeless case. Skip reading shadow, because no entry
        will get a match with /etc/passwd. */
        return dq;
    }

    while(HX_getl(&ln, fp) != NULL) {
        char *data[9];

        if(*ln == '#') { continue; }
        HX_chomp(ln);
        memset(data, 0, sizeof(data));
        HX_split5(ln, ":", array_size(data), data);
        if(*data[0] != '\0' && (u = get_user(dq, data[0])) == NULL) {
            // orphaned entry
            continue;
        }

        u->passwd      = HX_strdup(data[1]);
        u->last_change = strtol(data[2], NULL, 0);
        u->keep_min    = strtol(data[3], NULL, 0);
        u->keep_max    = strtol(data[4], NULL, 0);
        u->warn_age    = strtol(data[5], NULL, 0);

        if(data[6] == NULL || *data[6] == '\0') {
            u->expire = ADB_EXPIRE_NEVER;
        } else {
            u->expire = strtol(data[6], NULL, 0);
        }
        if(data[7] == NULL || *data[7] == '\0') {
            u->inactive = ADB_INACTIVE_NEVER;
        } else {
            u->inactive = strtol(data[7], NULL, 0);
        }

        ((char **)u->priv)[1] = HX_strdup(data[8]);
    }

    hmc_free(ln);
    return dq;
}

static struct HXdeque *db_read_vxshadow(FILE *fp, struct HXdeque *dq) {
    struct adb_user *u;
    char *ln = NULL;
    if(dq->itemcount == 0) { return dq; }

    while(HX_getl(&ln, fp) != NULL) {
        char *data[5];

        if(*ln == '#') { continue; }
        HX_chomp(ln);
        memset(data, 0, sizeof(data));
        HX_split5(ln, ":", array_size(data), data);
        if(*data[0] == '\0' || (u = get_user(dq, data[0])) == NULL) {
            // orphaned entry
            continue;
        }

        u->xuid   = HX_strdup(data[1]);
        u->sgroup = HX_strdup(data[2]);
        u->defer_del = strtol(data[3], NULL, 0);
    }

    hmc_free(ln);
    return dq;
}

static struct HXdeque *db_read_groups(FILE *fp) {
    // Comments to be found in db_read_users()
    struct adb_group *g;
    struct HXdeque *dq;
    char *ln = NULL;
    if((dq = HXdeque_init()) == NULL) { return NULL; }

    while(HX_getl(&ln, fp) != NULL) {
        char *data[4];

        if(*ln == '#') { continue; }
        if((g = malloc(sizeof(struct adb_group))) == NULL) { return NULL; }
        HX_chomp(ln);
        memset(data, 0, sizeof(data));
        if(HX_split5(ln, ":", array_size(data), data) < 3) {
            fprintf(stderr, "am_shadow: bogus line in group file\n");
            continue;
        }

        g->gname = HX_strdup(data[0]);
        g->gid   = strtol(data[2], NULL, 0);
        g->priv  = calloc(1, sizeof(void *) * 2);
        ((char **)g->priv)[0] = HX_strdup(data[3]);
        HXdeque_push(dq, g);
    }

    hmc_free(ln);
    return dq;
}

static void db_flush(struct shadow_state *sp, int force) {
    // Note to self: when db_flush() is called, the db is always in O_RDWR
    if(!force && !(sp->flags & ADB_SYNC)) {
        return;
    }

    if(sp->fpasswd.is_chg || sp->fshadow.is_chg ||
     sp->fvxshadow.is_chg || force) {
        db_flush_users(sp);
    }

    if(sp->fgroup.is_chg || force) {
        db_flush_groups(sp);
    }

    // Note to self: No implicit unlock except on am_close()
    return;
}

static void db_flush_users(struct shadow_state *sp) {
    const struct HXdeque_node *travp = sp->user->first;
    int has_vx = sp->fvxshadow.fp != NULL;

    fseek(sp->fpasswd.fp, 0, SEEK_SET);
    fseek(sp->fshadow.fp, 0, SEEK_SET);
    if(has_vx) {
        fseek(sp->fvxshadow.fp, 0, SEEK_SET);
        fprintf(sp->fvxshadow.fp, "# user:xuid:class:\n");
    }

    while(travp != NULL) {
        const struct adb_user *u = travp->ptr;
        db_write_passwd(sp->fpasswd.fp, u);
        db_write_shadow(sp->fshadow.fp, u);
        if(has_vx) {
            db_write_vxshadow(sp->fvxshadow.fp, u);
        }
        travp = travp->Next;
    }

    truncate_here(sp->fpasswd.fp);
    truncate_here(sp->fshadow.fp);
    if(has_vx) {
        truncate_here(sp->fvxshadow.fp);
    }

    TOUCH_USER_TAG(0);
    return;
}

static void db_flush_groups(struct shadow_state *sp) {
    const struct HXdeque_node *travp = sp->group->first;
    FILE *fp = sp->fgroup.fp;

    fseek(sp->fgroup.fp, 0, SEEK_SET);

    while(travp != NULL) {
        const struct adb_group *g = travp->ptr;
        const char **pr = g->priv;

        fprintf(fp, "%s:x:%ld:", g->gname, g->gid);
        if(pr != NULL && pr[0] != NULL) {
            fprintf(fp, "%s", pr[0]);
        }
        fprintf(fp, "\n");
        travp = travp->Next;
    }

    truncate_here(sp->fgroup.fp);
    TOUCH_GROUP_TAG(0);
    return;
}

static void db_write_passwd(FILE *fp, const struct adb_user *u) {
    const char **priv = u->priv;

    fprintf(fp, "%s:x:%ld:%ld:%s:%s:%s",
     u->lname, u->uid, u->gid, u->gecos, u->home, u->shell);
    if(priv != NULL && priv[0] != NULL) {
        fprintf(fp, "%s", priv[0]);
    }
    fprintf(fp, "\n");
    return;
}

static void db_write_shadow(FILE *fp, const struct adb_user *u) {
    const char *password = (u->passwd != NULL) ? u->passwd : "!";
    const char **priv = u->priv;

    fprintf(fp, "%s:%s:%ld:%ld:%ld:%ld:", u->lname, password, u->last_change,
     u->keep_min, u->keep_max, u->warn_age);
    if(u->expire != ADB_EXPIRE_NEVER) {
        fprintf(fp, "%ld", u->expire);
    }
    fprintf(fp, ":");
    if(u->inactive != ADB_INACTIVE_NEVER) {
        fprintf(fp, "%ld", u->inactive);
    }
    fprintf(fp, ":");
    if(priv != NULL && priv[1] != NULL) {
        fprintf(fp, "%s", priv[1]);
    }
    fprintf(fp, "\n");
    return;
}

static void db_write_vxshadow(FILE *fp, const struct adb_user *u) {
    if(u->xuid == NULL && u->sgroup == NULL && u->defer_del == 0) {
        return;
    }
    if(fp == NULL) {
        fprintf(stderr, "am_shadow: VXSHADOW content was added, but no vxshadow database present!\n");
        return;
    }
#define znul(p) (((p) == NULL) ? "" : (p))
    fprintf(fp, "%s:%s:%s:%ld:\n", u->lname, znul(u->xuid), znul(u->sgroup),
     u->defer_del);
#undef znul
    return;
}

//-----------------------------------------------------------------------------
static long find_next_uid(struct shadow_state *sp, unsigned int sys) {
    const struct HXdeque_node *travp = sp->user->first;
    long min, max, accept, high = -1;

    min = !sys ? sp->uid_min : 1;
    max = !sys ? sp->uid_max : sp->uid_min - 1;
    accept = min;

    // Find the highest ID
    while(travp != NULL) {
        const struct adb_user *user = travp->ptr;
        long uid = user->uid;
        if(uid >= min && uid <= max && uid > high) {
            high = uid;
        }
        travp = travp->Next;
    }

    // If the successor id is free, take it.
    if(high >= 0 && high < max) { return high + 1; }

    // No? Then just scan for the next
    while(accept <= max) {
        int used = 0;
        travp = sp->user->first;
        while(travp != NULL) {
            const struct adb_user *user = travp->ptr;
            if(user->uid == accept) {
                ++used;
                break;
            }
            travp = travp->Next;
        }
        if(!used) { return accept; }
        ++accept;
    }
    return -(errno = ENOENT);
}

static long find_next_gid(struct shadow_state *sp, unsigned int sys) {
    const struct HXdeque_node *travp = sp->group->first;
    long min, max, accept, high = -1;

    min = !sys ? sp->gid_min : 1;
    max = !sys ? sp->gid_max : sp->gid_min - 1;
    accept = min;

    // Find the highest GID
    while(travp != NULL) {
        const struct adb_group *group = travp->ptr;
        long gid = group->gid;
        if(gid >= min && gid <= max && gid > high) {
            high = gid;
        }
        travp = travp->Next;
    }

    /* If the highest id is below the max, there must be something free. Take
    the successor id. */
    if(high >= 0 && high < max) { return high + 1; }

    // No? Then just scan for the next
    while(accept <= max) {
        int used = 0;
        travp = sp->group->first;
        while(travp != NULL) {
            const struct adb_group *group = travp->ptr;
            if(group->gid == accept) {
                ++used;
                break;
            }
            travp = travp->Next;
        }
        if(!used) { return accept; }
        ++accept;
    }
    return -(errno = ENOENT);
}

static inline void free_single_user(struct adb_user *u) {
    char **p = u->priv;

    free(u->lname);
    free(u->gecos);
    free(u->home);
    free(u->shell);
    free(u->sgrp);
    free(u->passwd);

    if(p != NULL) {
        free(p[0]);
        free(p[1]);
        free(p);
    }

    free(u);
    return;
}

static void free_users(struct shadow_state *sp) {
    struct HXdeque_node *travp;
    if(sp->user == NULL) { return; }
    travp = sp->user->first;

    while(travp != NULL) {
        free_single_user(travp->ptr);
        travp = travp->Next;
    }

    HXdeque_free(sp->user);
    return;
}

static inline void free_single_group(struct adb_group *g) {
    char **p = g->priv;
    free(g->gname);
    if(p != NULL) {
        free(p[0]);
        free(p[1]);
        free(p);
    }
    free(g);
    return;
}

static void free_groups(struct shadow_state *sp) {
    struct HXdeque_node *travp;
    if(sp->group == NULL) { return; }
    travp = sp->group->first;

    while(travp != NULL) {
        free_single_group(travp->ptr);
        travp = travp->Next;
    }

    HXdeque_free(sp->group);
    return;
}

static inline struct adb_user *get_user(struct HXdeque *dq,
 const char *lname)
{
    const struct HXdeque_node *travp = dq->first;

    while(travp != NULL) {
        struct adb_user *u = travp->ptr;
        if(strcmp(u->lname, lname) == 0) { return u; }
        travp = travp->Next;
    }

    return NULL;
}

static inline struct adb_group *get_group(struct HXdeque *dq,
 const char *name)
{
    const struct HXdeque_node *travp = dq->first;

    while(travp != NULL) {
        struct adb_group *g = travp->ptr;
        if(strcmp(g->gname, name) == 0) { return g; }
        travp = travp->Next;
    }

    return NULL;
}

static void read_config(struct shadow_state *sp, unsigned int action) {
    struct shconf_opt autouid_table[] = {
        {"UID_MIN", SHCONF_LONG, &sp->uid_min},
        {"UID_MAX", SHCONF_LONG, &sp->uid_max},
        {"GID_MIN", SHCONF_LONG, &sp->gid_min},
        {"GID_MAX", SHCONF_LONG, &sp->gid_max},
        {NULL},
    };
    struct shconf_opt options_table[] = {
        {"PASSWD_DB",   SHCONF_STRING, &sp->fpasswd.path},
        {"SHADOW_DB",   SHCONF_STRING, &sp->fshadow.path},
        {"VXSHADOW_DB", SHCONF_STRING, &sp->fvxshadow.path},
        {"GROUP_DB",    SHCONF_STRING, &sp->fgroup.path},
        {NULL},
    };

    if(action == CONFIG_READ) {
        sp->uid_min = sp->gid_min = 1000;
        sp->uid_max = sp->gid_max = 60000;
        HX_shconfig("/etc/vitalnix/autouid.conf", autouid_table);
        HX_shconfig("/etc/vitalnix/am_shadow.conf", options_table);
    } else if(action == CONFIG_FREE) {
        HX_shconfig_free(options_table);
    }

    return;
}

static void truncate_here(FILE *fp) {
    long p = ftell(fp);
    fflush(fp);
    ftruncate(fileno(fp), p);
    fflush(fp);
    fseek(fp, 0, SEEK_SET);
    fflush(fp);
    return;
}

//=============================================================================
