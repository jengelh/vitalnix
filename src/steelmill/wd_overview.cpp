/*=============================================================================
Vitalnix User Management Suite
steelmill/wd_overview.cpp
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
#include <cstdio>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif
#include "steelmill/wd_overview.hpp"
#include "steelmill/xu_common.hpp"

enum {
    ID_SHOW_USERS = 1,
    ID_SHOW_GROUPS,
};

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(WD_Overview, wxDialog)
END_EVENT_TABLE()

WD_Overview::WD_Overview(wxWindow *parent, unsigned int display_type) :
    wxDialog(parent, wxID_ANY, wxT(PROD_NAME " > User and Group Overview"),
     wxDPOS, wxDSIZE, wxCFF)
{
    wxBoxSizer *vp = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *hp0 = new wxBoxSizer(wxHORIZONTAL);
    hp0->Add(new wxRadioButton(this, ID_SHOW_USERS, wxT("Users")), 0, wxALL, 3);
    hp0->Add(new wxRadioButton(this, ID_SHOW_GROUPS, wxT("Groups")), 0, wxALL, 3);

//    wxBoxSizer *hp1 = new wxBoxSizer(wxHORIZONTAL);

    vp->Add(hp0, 0, wxALIGN_LEFT);
    vp->Add(init_list(display_type));

    SetSizer(vp);
    vp->SetSizeHints(this);
    smc_size_aspect(this);
    Center();
    return;
}

wxListCtrl *WD_Overview::init_list(unsigned int type) {
    wxListCtrl *lc = new wxListCtrl(this, wxID_ANY, wxDPOS, wxDSIZE,
                     wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_SORT_ASCENDING |
                     wxLC_HRULES | wxLC_VRULES);
    wxListItem col;

    col.SetImage(-1);
    col.SetText(wxT("Username"));
    lc->InsertColumn(0, col);
    col.SetText(wxT("UID"));
    lc->InsertColumn(1, col);
    return lc;
}

//=============================================================================
