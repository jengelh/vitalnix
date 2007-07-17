/*
 *	shadow/dbops.c
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2002 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <libHX.h>
#include "drivers/shadow/shadow.h"
#include <vitalnix/libvxpdb/libvxpdb.h>

/* Functions */
static void db_flush_users(struct shadow_state *);

/*
 * The VXDB_SHADOW locking scheme:
 * - databases are opened in read mode by default, no shared/read locks
 * - upon a write attempt (shadow_useradd, etc.) reopen in read-write mode:
 *   steps: close db, reopen with rwlock, reread, perform operation
 * - on db_flush(), write db to disk and release write lock (no close/reread)
 */

//-----------------------------------------------------------------------------
int db_open(struct shadow_state *state, int flags)
{
#define open_fd(n) ((state->n.fd = open(state->n.path, flags)) >= 0)
#define open_fp(n) ((state->n.fp = fdopen(state->n.fd, fpmode)) != NULL)
	const char *fpmode = (flags & PDB_WRLOCK) ? "r+" : "r";
	int ret;

	if (!open_fd(fpasswd) || !open_fd(fgroup))
		goto out;

	/*
	 * Normal users do not get access to /etc/shadow - but should not be
	 * blamed for trying to do. Only error out when someone tries to write
	 * to it. "root" usually succeeds.
	 */
	if (!open_fd(fshadow) && (errno != EACCES || flags != O_RDONLY))
		goto out;

	/*
	 * /etc/vxshadow is an extension, and it is legal to be missing. It is
	 * also legal to be non-world-readable.
	 */
	if (!open_fd(fvxshadow) && errno != ENOENT &&
	    (errno != EACCES || flags != O_RDONLY))
		goto out;

	if (flags == O_RDWR) {
		struct flock lk = {
			.l_type   = F_WRLCK,
			.l_whence = SEEK_SET,
			.l_start  = 0,
			.l_len	= 0,
		};

		if (fcntl(state->fpasswd.fd, F_SETLK, &lk) < 0) {
			fprintf(stderr, "db_shadow: passwd file (%s) "
			        "currently write-locked, waiting\n",
			        state->fpasswd.path);
			if (fcntl(state->fpasswd.fd, F_SETLKW, &lk) < 0)
				goto out;
		}
	}

	if (!open_fp(fpasswd) || !open_fp(fgroup) ||
	    (state->fshadow.fd > 0 && !open_fp(fshadow)) ||
	    (state->fvxshadow.fd > 0 && !open_fp(fvxshadow)))
		goto out;

	state->dq_user  = db_read_passwd(state->fpasswd.fp);
	state->dq_group = db_read_groups(state->fgroup.fp);

	if (state->fshadow.fp != NULL)
		db_read_shadow(state->fshadow.fp, state->dq_user);
	if (state->fvxshadow.fp != NULL)
		db_read_vxshadow(state->fvxshadow.path, state->dq_user);

	state->flags = flags;
	return 1;

 out:
	ret = errno;
	db_close(state);
	return -ret;
#undef open_fd
#undef open_fp
}

void db_close(struct shadow_state *state)
{
#define close_fpd(n) \
	if (state->n.fp != NULL) { \
		fclose(state->n.fp); \
		state->n.fd = -1; \
	} \
	if (state->n.fd >= 0) \
		close(state->n.fd);

	if (state->fpasswd.is_chg | state->fshadow.is_chg |
		state->fvxshadow.is_chg | state->fgroup.is_chg)
		db_flush(state, 1);

	close_fpd(fvxshadow);
	close_fpd(fgroup);
	close_fpd(fshadow);
	close_fpd(fpasswd);
	return;
#undef close_fpd
}

/*
 * db_flush -
 * @state:	Current instance
 * @force:	Force flush even if in-memory copy is recent
 *
 * Flush the in-memory database to disk if required. The files must be open
 * with %O_RDWR when this function is called.
 */
void db_flush(struct shadow_state *state, int force)
{
	if (!force && !(state->flags & PDB_SYNC))
		return;

	if (state->fpasswd.is_chg || state->fshadow.is_chg ||
	    state->fvxshadow.is_chg || force)
		db_flush_users(state);

	if (state->fgroup.is_chg || force)
		db_flush_groups(state);

	return;
}

static void db_flush_users(struct shadow_state *state)
{
	const struct HXdeque_node *travp = state->dq_user->first;
	int has_vx = state->fvxshadow.fp != NULL;

	fseek(state->fpasswd.fp, 0, SEEK_SET);
	fseek(state->fshadow.fp, 0, SEEK_SET);
	if (has_vx) {
		fseek(state->fvxshadow.fp, 0, SEEK_SET);
		fprintf(state->fvxshadow.fp,
			"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
			"<VX3_vxshadow>\n"
		);
	}

	while (travp != NULL) {
		const struct vxpdb_user *u = travp->ptr;
		db_write_passwd(state->fpasswd.fp, u);
		db_write_shadow(state->fshadow.fp, u);
		if (has_vx)
			db_write_vxshadow(state->fvxshadow.fp, u);
		travp = travp->next;
	}

	truncate_here(state->fpasswd.fp);
	truncate_here(state->fshadow.fp);
	if (has_vx) {
		fprintf(state->fvxshadow.fp, "</VX3_vxshadow>\n");
		truncate_here(state->fvxshadow.fp);
	}

	TOUCH_USER_TAG(0);
	return;
}
