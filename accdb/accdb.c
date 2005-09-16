/*=============================================================================
Vitalnix User Management Suite
  Copyright © Jan Engelhardt <jengelh [at] linux01 gwdg de>, 2003 - 2005
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
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libHX.h>
#include "include/accdb.h"
#include "accdb/bf.h"

#ifndef _WIN32
#    ifndef __USE_GNU
#        define GNU_WAS_NOT_IN 1 // dirty hack
#        define __USE_GNU 1
#    endif
#    define __USE_GNU 1 // dirty hack
#    include <crypt.h>
#    ifdef GNU_WAS_NOT_IN
#        undef __USE_GNU
#    endif
#    undef GNU_WAS_NOT_IN
#endif

#define MAXFNLEN 256 // max length for filename buffer

enum {
    CONFIG_READ = 1,
    CONFIG_FREE,
};

struct opt {
    char *def_backend;
};

static int genpw_random(char *, size_t, unsigned long, void *);

static int ac_cleanup(struct adb_module *, int, struct opt *);
static void *ac_get_handle(const char *);
static void ac_get_symbols(struct adb_module *);
static void ac_config(struct opt *, unsigned int);

//-----------------------------------------------------------------------------
struct adb_module *adb_load(const char *name, void *priv) {
    struct adb_module *this = NULL;
    struct opt cfg;
    int eax;

    memset(&cfg, 0, sizeof(struct opt));
    ac_config(&cfg, CONFIG_READ);

    if((strcmp(name, "*") == 0 && (name = cfg.def_backend) == NULL)) {
        errno = ac_cleanup(this, errno, &cfg);
    }

    if(*name == '\0') {
        errno = ac_cleanup(NULL, EINVAL, &cfg);
        return NULL;
    }

    if((this = malloc(sizeof(struct adb_module))) == NULL ||
     (this->handle = ac_get_handle(name)) == NULL) {
        errno = ac_cleanup(this, errno, &cfg);
        return NULL;
    }

    ac_get_symbols(this);
    if(this->init != NULL && (eax = this->init(this, priv)) <= 0) {
        errno = ac_cleanup(this, eax, &cfg);
        return NULL;
    }

    ac_cleanup(NULL, 0, &cfg);
    return this;
}

void adb_unload(struct adb_module *this) {
    if(this->deinit != NULL) {
        this->deinit(this);
    }
    ac_cleanup(this, 0, NULL);
    return;
}

int vx_cryptpw(const char *key, const char *salt, int meth, char **crypted) {
    static const char salt_pool[] =
     "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    void *randp = HX_rand_init(HXRAND_DEV_URANDOM | HXRAND_SYS_RAND);
    void *rx = NULL;
    int rv = 1;

    if(randp == NULL) { return -errno; }

    switch(meth) {
#ifndef _WIN32
        case CRYPW_DES: {
            struct crypt_data cd;
            memset(&cd, 0, sizeof(struct crypt_data));
            if(salt == NULL) {
                char my_salt[3], *saltp = my_salt;
                int n = 2;
                while(n--) { *saltp++ = salt_pool[HX_irand(randp, 0, 64)]; }
                *saltp++ = '\0';
                rx = crypt_r(key, my_salt, &cd);
            } else {
                rx = crypt_r(key, salt, &cd);
            }
            if(rx == NULL) { rv = 0; }
            else { *crypted = HX_strdup(rx); }
            break;
        }
        case CRYPW_MD5: {
            struct crypt_data cd;
            memset(&cd, 0, sizeof(struct crypt_data));
            if(salt == NULL) {
                char my_salt[13], *saltp = my_salt;
                int n = 8;

                memset(my_salt, 0, 13);
                *saltp++ = '$';
                *saltp++ = '1';
                *saltp++ = '$';
                while(n--) { *saltp++ = salt_pool[HX_irand(randp, 0, 64)]; }
                *saltp++ = '$';
                rx = crypt_r(key, my_salt, &cd);
            } else {
                rx = crypt_r(key, salt, &cd);
            }
            if(rx == NULL) { rv = 0; }
            else { *crypted = HX_strdup(rx); }
            break;
        }
#endif
        case CRYPW_BLOWFISH: {
            char res[64];
            if(salt == NULL) {
                char my_salt[64], *saltp = res;
                int n = 22;

                while(n--) { *saltp++ = salt_pool[HX_irand(randp, 0, 64)]; }
                *saltp++ = '\0';

                snprintf(my_salt, 64, "$2a$05$%s", res);
                rx = _crypt_blowfish_rn(key, my_salt, res, 64);
            } else {
                rx = _crypt_blowfish_rn(key, salt, res, 64);
            }
            if(rx == NULL) { rv = 0; }
            else { *crypted = HX_strdup(res); }
            break;
        }
        default:
            HX_rand_deinit(randp);
            return 0;
            break;
    }

    HX_rand_deinit(randp);
    return 1;
}

int vx_genpw(char *plain, size_t len, unsigned long flags) {
    void *randp = HX_rand_init(HXRAND_DEV_URANDOM | HXRAND_SYS_RAND);
    if(randp == NULL) { return 0; }

    if(flags & GENPW_PHONEMIC) {
        genpw_phonemic(plain, len, flags & ~GENPW_PHONEMIC, randp);
    } else {
        genpw_random(plain, len, flags, randp);
    }

    HX_rand_deinit(randp);
    return 1;
}

//-----------------------------------------------------------------------------
static int genpw_random(char *plain, size_t size, unsigned long fl, void *rd) {
    char *ptrav = plain, *pcase = NULL;
    size_t cnt = size;

    while(cnt--) {
        // 20% probability for a digit
        *ptrav++ = !HX_irand(rd, 0, 5) ? '0' + HX_irand(rd, 0, 10) :
         'a' + HX_irand(rd, 0, 26);
    }

    if(fl & GENPW_ONE_CASE) {
        pcase = &plain[HX_irand(rd, 0, size)];
        *pcase = toupper(*pcase);
    }

    if(fl & GENPW_ONE_DIGIT) {
        char *pdigit;
        // do not digitize the case letter
        while((pdigit = &plain[HX_irand(rd, 0, size)]) == pcase);
        *pdigit = '0' + HX_irand(rd, 0, 10);
    }

    *ptrav++ = '\0';
    return 1;
}

//-----------------------------------------------------------------------------
static int ac_cleanup(struct adb_module *this, int err, struct opt *cf) {
    if(cf != NULL) {
        ac_config(cf, CONFIG_FREE);
    }
    if(this != NULL) {
        if(this->handle != NULL) {
            HX_dlclose(this->handle);
        }
        free(this);
    }
    return err;
}

static void *ac_get_handle(const char *filename) {
    const char *ext[] = {".so", ".dll", "", NULL}, **extp = ext;
    void *handle;

    // Try plain filename first (as does accdbinfo)
    if((handle = HX_dlopen(filename, HXLD_NOW)) != NULL) {
        return handle;
    }

    // If not, prepend accdb_, append an extension and try again
    errno = ENOENT;
    while(*extp != NULL) {
        char fn[MAXFNLEN];
        snprintf(fn, MAXFNLEN, "libvxam_%s%s", filename, *extp);
        if((handle = HX_dlopen(fn, HXLD_NOW)) != NULL) {
            errno = 0;
            break;
        }
        ++extp;
    }
    return handle;
}

static void ac_get_symbols(struct adb_module *this) {
    this->name      = HX_dlsym(this->handle, "_module_name");
    this->desc      = HX_dlsym(this->handle, "_module_desc");
    this->info      = HX_dlsym(this->handle, "_module_info");

    this->init      = HX_dlsym(this->handle, "am_init");
    this->open      = HX_dlsym(this->handle, "am_open");
    this->lock      = HX_dlsym(this->handle, "am_lock");
    this->unlock    = HX_dlsym(this->handle, "am_unlock");
    this->close     = HX_dlsym(this->handle, "am_close");
    this->modctl    = HX_dlsym(this->handle, "am_modctl");
    this->deinit    = HX_dlsym(this->handle, "am_deinit");

    this->useradd   = HX_dlsym(this->handle, "am_useradd");
    this->usermod   = HX_dlsym(this->handle, "am_usermod");
    this->userdel   = HX_dlsym(this->handle, "am_userdel");
    this->usertrav  = HX_dlsym(this->handle, "am_usertrav");
    this->userinfo  = HX_dlsym(this->handle, "am_userinfo");

    this->groupadd  = HX_dlsym(this->handle, "am_groupadd");
    this->groupmod  = HX_dlsym(this->handle, "am_groupmod");
    this->groupdel  = HX_dlsym(this->handle, "am_groupdel");
    this->grouptrav = HX_dlsym(this->handle, "am_grouptrav");
    this->groupinfo = HX_dlsym(this->handle, "am_groupinfo");
    return;
}

static void ac_config(struct opt *config, unsigned int action) {
    struct shconf_opt table[] = {
        {"DEFAULT_BACKEND", SHCONF_STRING, &config->def_backend},
        {NULL},
    };
    if(action == CONFIG_READ) {
        HX_shconfig("/etc/vitalnix/accdb.conf", table);
    } else if(action == CONFIG_FREE) {
        HX_shconfig_free(table);
    }
    return;
}

//=============================================================================
