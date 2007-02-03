/*
    steelmill/wd_pwlfmt.cpp
    Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2005 - 2007

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
#include <vitalnix/libvxmdfmt/libvxmdfmt.h>
#include <vitalnix/libvxmdfmt/vtable.h>
#include "steelmill/wd_pwlfmt.hpp"
#include "steelmill/xu_common.hpp"

// Definitions
enum {
    ID_STYLE = 1,
};

class GW_PwlstylesChoice : public wxChoice {
  public: // functions
    GW_PwlstylesChoice(wxWindow *, wxWindowID = wxID_ANY);
};

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(WD_Pwlfmt, wxDialog)
    EVT_CHOICE(ID_STYLE, WD_Pwlfmt::change_style)
    EVT_BUTTON(wxID_OK,  WD_Pwlfmt::process)
END_EVENT_TABLE()

WD_Pwlfmt::WD_Pwlfmt(wxWindow *parent) :
    wxDialog(parent, wxID_ANY, wxT(PROD_NAME " > Print password lists"),
     wxDPOS, wxDSIZE, wxCFF)
{
    wxCommandEvent empty_event;
    wxBoxSizer *vp      = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer *gp = new wxFlexGridSizer(2);

    gp->AddGrowableCol(1);
    gp->AddGrowableRow(3);
    gp->Add(new wxStaticText(this, wxID_ANY, wxT("Input file:")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
    gp->Add(ct_input = new GW_FTC(this, wxEmptyString, 3), 0, wxGROW | wxACV);
    gp->Add(new wxStaticText(this, wxID_ANY, wxT("Output file:")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
    gp->Add(ct_output = new GW_FTC(this, wxEmptyString, 3), 0, wxGROW | wxACV);
    gp->Add(new wxStaticText(this, wxID_ANY, wxT("Style:")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
    gp->Add(ct_style = new GW_PwlstylesChoice(this, ID_STYLE), 0, wxALIGN_LEFT | wxACV | wxALL, 3);
    gp->Add(ct_tpltext = new wxStaticText(this, wxID_ANY, wxT("Template file:")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
    gp->Add(ct_template = new GW_FTC(this, wxEmptyString, 3), 0, wxGROW | wxACV);

    ct_style->SetSelection(0);
    change_style(empty_event);

    vp->Add(gp, 1, wxGROW);
    vp->Add(smc_navgen(this, "o-c"), 0, wxGROW);
    SetSizer(vp);
    vp->SetSizeHints(this);
    smc_size_aspect(this);
    Center();
    return;
}

void WD_Pwlfmt::change_style(wxCommandEvent &event) {
    const struct pwlstyle_vtable *vtable =
        static_cast<const struct pwlstyle_vtable *>
        (ct_style->GetClientData(ct_style->GetSelection()));
    ct_tpltext->Enable(vtable->require_template);
    ct_template->Enable(vtable->require_template);
    return;
}

void WD_Pwlfmt::process(wxCommandEvent &event) {
    return;
}

//-----------------------------------------------------------------------------
GW_PwlstylesChoice::GW_PwlstylesChoice(wxWindow *parent, wxWindowID id) :
    wxChoice(parent, id, wxDPOS, wxDSIZE, 0, NULL)
{
    const struct pwlstyle_vtable *vtable;
    void *trav = NULL;

    while((vtable = pwlstyles_trav(&trav)) != NULL)
        Append(fU8(vtable->name) + wxT(" (") + fU8(vtable->desc) + wxT(")"),
               const_cast<void *>(static_cast<const void *>(vtable)));

    return;
}

//=============================================================================
