/*
    steelmill/wd_fixuuid.cpp
    Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006 - 2007

    This file is part of Vitalnix. Vitalnix is free software; you can
    redistribute it and/or modify it under the terms of the GNU Lesser General
    Public License as published by the Free Software Foundation; however ONLY
    version 2 of the License. For details, see the file named "LICENSE.LGPL2".
*/
#include <cstdio>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxpdb/xafunc.h>
#include "steelmill/wd_fixuuid.hpp"
#include "steelmill/xu_common.hpp"
#include "steelmill/xu_database.hpp"

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(WD_FixUUID, wxDialog)
    EVT_BUTTON(wxID_OK,     WD_FixUUID::Ok)
    EVT_COMBOBOX(wxID_FIND, WD_FixUUID::Fill_Defaults)
    EVT_TEXT(wxID_FIND,     WD_FixUUID::Fill_Defaults)
END_EVENT_TABLE()

WD_FixUUID::WD_FixUUID(wxWindow *parent) :
    wxDialog(parent, wxID_ANY, wxT(PROD_NAME " > Fix UUID"),
     wxDPOS, wxDSIZE, wxCFF)
{
    wxBoxSizer *vp = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer *sf = new wxFlexGridSizer(2);
    sf->AddGrowableCol(1);
    sf->Add(new wxStaticText(this, wxID_ANY, wxT("Username:")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
    sf->Add(ct_username = new GW_UserCombo(this, wxID_FIND), 0, wxACV | wxALL, 3);
    sf->Add(new wxStaticText(this, wxID_ANY, wxT("New real name:")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
    sf->Add(ct_realname = new wxTextCtrl(this, wxID_ANY, wxEmptyString), 0, wxGROW | wxACV | wxALL, 3);
    sf->Add(new wxStaticText(this, wxID_ANY, wxT("New birthdate:")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
    sf->Add(ct_bday = new wxTextCtrl(this, wxID_ANY, wxEmptyString), 0, wxGROW | wxACV | wxALL, 3);

    vp->Add(sf, 1, wxGROW | wxALL, 5);
    vp->Add(smc_navgen(this, "o-c"), 0, wxGROW);
    SetSizer(vp);
    vp->SetSizeHints(this);
    smc_size_aspect(this);
    Center();

    return;
}

void WD_FixUUID::Fill_Defaults(wxCommandEvent &event) {
    return;
}

void WD_FixUUID::Ok(wxCommandEvent &event) {
    return;
}

//=============================================================================
