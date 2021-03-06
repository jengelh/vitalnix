/*
 *	steelmill/xu_database.cpp
 *	Copyright © CC Computer Consultants GmbH, 2006 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif
#include <libHX/wx_helper.hpp>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxdb/xafunc.h>
#include "steelmill/xu_common.hpp"
#include "steelmill/xu_database.hpp"

struct vxdb_state *database_open(long open_flags, wxWindow *parent)
{
	struct vxdb_state *dbh;
	int ret;

	if ((dbh = vxdb_load("*")) == NULL) {
		wxString s;
		s.Printf(wxT("Could not load backend module \"%s\": %s\n"),
		         wxT("*"), wxfv8(strerror(errno)));
		GW_Message(parent, wxT("Failure"), s, "-o").ShowModal();
		return NULL;
	}

	if ((ret = vxdb_open(dbh, open_flags)) <= 0) {
		wxString s;
		s.Printf(wxT("Could not open backend module: %s\n"),
		         wxfv8(strerror(-ret)));
		GW_Message(parent, wxT("Failure"), s, "-o").ShowModal();
		vxdb_unload(dbh);
		return NULL;
	}

	return dbh;
}
