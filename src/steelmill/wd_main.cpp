/*=============================================================================
Vitalnix User Management Suite
steelmill/wd_main.cpp
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2005 - 2006
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
#include <wx/statline.h>
#include "steelmill/wd_about.hpp"
#include "steelmill/wd_fixuuid.hpp"
#include "steelmill/wd_main.hpp"
#include "steelmill/wd_overview.hpp"
#include "steelmill/wd_pwlfmt.hpp"
#include "steelmill/wd_single.hpp"
#include "steelmill/wd_sync.hpp"
#include "steelmill/xu_common.hpp"
#include "steelmill/xu_loader.hpp"

// Definitions
enum {
    ID_SYNC = 1,
    ID_PWLFMT,
    ID_ADDSINGLE,
    ID_FIXUUID,
    ID_VIEWUSERS,
    ID_VIEWGROUPS,
    ID_CONFIGURE,
};

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(WD_MainMenu, wxFrame)
    EVT_BUTTON(ID_SYNC,       WD_MainMenu::Sync)
    EVT_BUTTON(ID_PWLFMT,     WD_MainMenu::Pwl_Format)
    EVT_BUTTON(ID_ADDSINGLE,  WD_MainMenu::Add_Single)
    EVT_BUTTON(ID_FIXUUID,    WD_MainMenu::FixUUID)
    EVT_BUTTON(ID_VIEWUSERS,  WD_MainMenu::View_Users)
    EVT_BUTTON(ID_VIEWGROUPS, WD_MainMenu::View_Groups)
    EVT_BUTTON(wxID_MORE,     WD_MainMenu::Show_Console)
    EVT_BUTTON(wxID_ABOUT,    WD_MainMenu::About)
    EVT_BUTTON(wxID_EXIT,     WD_MainMenu::Exit)
END_EVENT_TABLE()

WD_MainMenu::WD_MainMenu(const char *title, const wxSize &size,
 const wxPoint &pos) :
    wxFrame(NULL, wxID_ANY, fU8(title), pos, size)
{
    SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxNORMAL, wxNORMAL,
            false, wxT("Arial")));

    wxBoxSizer *vp0 = new wxBoxSizer(wxVERTICAL),
         *hp0, *vp1 = new wxBoxSizer(wxHORIZONTAL);

    vp0->Add(smc_logo_panel(this), 0, wxGROW);
    vp1->Add(generate_menu(), 0, wxALIGN_CENTER | wxALL, 5);

    vp0->Add(vp1, 1, wxALIGN_CENTER);
    vp0->Add(hp0 = smc_navgen(this, "-e"), 0, wxGROW);
    hp0->GetItem(1)->GetSizer()->Prepend(new wxButton(this, wxID_MORE, wxT("Console")), 0, wxALL, 3);

    SetSizer(vp0);
    vp0->SetSizeHints(this);
    SetSize(size);
    smc_size_aspect(this);
    Center();
    return;
}

WD_MainMenu::~WD_MainMenu(void) {
    // Kill the console window. Even if it is invisible, it still exists.
    static_cast<Loader *>(wxTheApp)->console->Destroy();
    return;
}

//-----------------------------------------------------------------------------
void WD_MainMenu::About(wxCommandEvent &event) {
    WD_About(this).ShowModal();
    return;
}

void WD_MainMenu::Add_Single(wxCommandEvent &event) {
    WD_Single(this).ShowModal();
    return;
}

void WD_MainMenu::Exit(wxCommandEvent &event) {
    Close(true);
    return;
}

void WD_MainMenu::FixUUID(wxCommandEvent &event) {
    WD_FixUUID(this).ShowModal();
    return;
}

void WD_MainMenu::Pwl_Format(wxCommandEvent &event) {
    WD_Pwlfmt(this).ShowModal();
    return;
}

void WD_MainMenu::Show_Console(wxCommandEvent &event) {
    static_cast<Loader *>(wxTheApp)->console->Show();
    return;
}

void WD_MainMenu::Sync(wxCommandEvent &event) {
    WD_SyncParam(this).ShowModal();
    return;
}

void WD_MainMenu::View_Groups(wxCommandEvent &event) {
    WD_Overview(this, OVERVIEW_GROUPS).ShowModal();
    return;
}

void WD_MainMenu::View_Users(wxCommandEvent &event) {
    WD_Overview(this, OVERVIEW_USERS).ShowModal();
    return;
}

wxFlexGridSizer *WD_MainMenu::generate_menu(void) {
    struct map {
        int id;
        const char *text, *desc;
    };
    static const struct map text[] = {
        {ID_SYNC, "Synchronize >>",
            "Synchronize a group with an external data source, import\n"
            "new users, archive and remove old ones."},
        {ID_PWLFMT, "Print password list >>",
            "Convert a logfile from the synchronization step into\n"
            "a proper TXT/RTF/TEX file for printing."},
        {ID_ADDSINGLE, "Add a single user >>",
            "If the External Data Source does not yet have a\n"
            "user, you can add it here."},
        {ID_FIXUUID, "Fix UUID >>",
            "Fix UUID of a single user should the real name\n"
            "or the birthdate have changed."},
        {ID_VIEWUSERS, "List users >>",
            "Show all users, create or delete accounts and\n"
            "modify account options."},
        {ID_VIEWGROUPS, "List groups >>",
            "\n"},
        {ID_CONFIGURE, "Configure >>",
            "Change the behavior of Vitalnix\n"},
        {wxID_ABOUT, "About >>",
            ""},
        {0},
    };
    const struct map *travp = text;

    wxFlexGridSizer *sizer = new wxFlexGridSizer(3);
    sizer->AddGrowableCol(2);
    while(travp->id != 0) {
        sizer->Add(new wxStaticText(this, wxID_ANY, fU8(travp->desc)), 0, wxGROW | wxALL, 5);
        sizer->Add(25, 1);
        sizer->Add(new wxButton(this, travp->id, fU8(travp->text)), 0, wxGROW | wxALL, 5);
        ++travp;
    }

    return sizer;
}

//=============================================================================
