/*=============================================================================
Vitalnix User Management Suite
steelmill/xu_loader.cpp
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
#if defined(__GNUG__) && !defined(_GNU_SOURCE)
#    define _GNU_SOURCE 1 // fopencookie()
#endif
#include <sys/types.h>
#include <cstdio>
#include <cstdlib>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif
#include <wx/statline.h>
#include <steelmill/images.hpp>
#include "steelmill/xu_common.hpp"
#include "steelmill/xu_loader.hpp"
#include "steelmill/wd_main.hpp"
#include "steelmill/wd_splash.hpp"

// Definitions
enum {
    ID_TICK = 1,
};

//-----------------------------------------------------------------------------
IMPLEMENT_APP(Loader);

BEGIN_EVENT_TABLE(Loader, wxApp)
    EVT_TIMER(ID_TICK, Loader::splash_tick)
END_EVENT_TABLE()

bool Loader::OnInit(void) {
    WD_MainMenu *mm;
    WD_Splash *spl;
    wxTimer *timer;

    wxInitAllImageHandlers();
    initialize_images();
    this->splash_done = 0;

    timer = new wxTimer(this, ID_TICK);
    spl   = new WD_Splash();
    spl->Show(true);
    Yield(); // increases "instantaneousness", better than Update()
//    timer->Start(1000, true);

    mm = new WD_MainMenu(PROD_NAME);
    console = new WD_Console();
    printf("Vitalnix Console initialized\n");

/*    while(this->splash_done < 1)
        Yield(); */

    mm->Show(true);
/*    timer->Start(1000, true);
    while(this->splash_done < 2)
        Yield(); */

    delete spl;
    delete timer;
    return true;
}

void Loader::splash_tick(wxTimerEvent &event) {
    ++this->splash_done;
    return;
}

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(WD_Console, wxFrame)
    EVT_BUTTON(wxID_CLEAR, WD_Console::Clear)
    EVT_BUTTON(wxID_OK,    WD_Console::Ok)
END_EVENT_TABLE()

WD_Console::WD_Console(void) :
    wxFrame(NULL, wxID_ANY, wxT("Vitalnix Console"))
{
    wxBoxSizer *vp = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *hp = new wxBoxSizer(wxHORIZONTAL);
    vp->Add(tc = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDPOS, wxDSIZE, wxTE_MULTILINE), 1, wxGROW | wxALL, 5);
    vp->Add(new wxStaticLine(this, wxID_ANY), 0, wxGROW);
    hp->Add(new wxButton(this, wxID_CLEAR, wxT("Clear")), 0, wxALL, 3);
    hp->Add(-1, -1, 1);
    hp->Add(new wxButton(this, wxID_OK, wxT("Hide")), 0, wxALL, 3);
    vp->Add(hp, 0, wxGROW);
    SetSizer(vp);
    vp->SetSizeHints(this);
    smc_size_minimum(this, ConvertDialogToPixels(wxSize(4 * 80, 8 * 25)));
    smc_size_aspect(this);
    Center();

#ifdef _GNU_SOURCE
    cookie_io_functions_t channel = {};
    saved_stdout  = saved_stderr = NULL;
    channel.write = write_cb;
    if((current = fopencookie(NULL, "w", channel)) != NULL) {
        setvbuf(current, NULL, _IONBF, 0);
        saved_stdout = stdout;
        saved_stderr = stderr;
        stdout = stderr = current;
    }
#endif

    return;
}

WD_Console::~WD_Console(void) {
    if(saved_stdout != NULL)
        stdout = saved_stdout;
    if(saved_stderr != NULL)
        stderr = saved_stderr;
    if(current != NULL)
        fclose(current);
    return;
}

void WD_Console::Append(const char *buffer) {
    tc->WriteText(fU8(buffer));
    return;
}

void WD_Console::Clear(wxCommandEvent &event) {
    tc->Clear();
    return;
}

void WD_Console::Ok(wxCommandEvent &event) {
    Hide();
    return;
}

ssize_t WD_Console::write_cb(void *cookie, const char *buffer, size_t size) {
    static_cast<Loader *>(wxTheApp)->console->Append(buffer);
    return size;
}

//=============================================================================