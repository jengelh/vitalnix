/*=============================================================================
Vitalnix User Management Suite
steelmill/xu_database.cpp
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006
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
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxpdb/xafunc.h>
#include "steelmill/xu_common.hpp"
#include "steelmill/xu_database.hpp"

//-----------------------------------------------------------------------------
struct vxpdb_state *database_open(long open_flags, wxWindow *parent) {
    struct vxpdb_state *dbh;
    int ret;

    if((dbh = vxpdb_load("*")) == NULL) {
        wxString s;
        s.Printf(wxT("Could not load backend module \"%s\": %s\n"),
            wxT("*"), fV8(strerror(errno)));
        GW_Message(parent, wxT("Failure"), s, "-o").ShowModal();
        return NULL;
    }

    if((ret = vxpdb_open(dbh, open_flags)) <= 0) {
        wxString s;
        s.Printf(wxT("Could not open backend module: %s\n"), fV8(strerror(-ret)));
        GW_Message(parent, wxT("Failure"), s, "-o").ShowModal();
        vxpdb_unload(dbh);
        return NULL;
    }

    return dbh;
}

//=============================================================================
