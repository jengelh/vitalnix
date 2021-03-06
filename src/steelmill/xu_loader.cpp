/*
 *	steelmill/xu_loader.cpp
 *	Copyright © CC Computer Consultants GmbH, 2005 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#if defined(__GNUG__) && !defined(_GNU_SOURCE)
#	define _GNU_SOURCE 1 /* fopencookie() */
#endif
#include <sys/types.h>
#include <cstdio>
#include <cstdlib>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif
#include <wx/statline.h>
#include <libHX/wx_helper.hpp>
#include <steelmill/images.hpp>
#include "steelmill/xu_common.hpp"
#include "steelmill/xu_loader.hpp"
#include "steelmill/wd_main.hpp"
#include "steelmill/wd_splash.hpp"

enum {
	ID_TICK = 1,
};

IMPLEMENT_APP(Loader);

BEGIN_EVENT_TABLE(Loader, wxApp)
	EVT_TIMER(ID_TICK, Loader::splash_tick)
END_EVENT_TABLE()

bool Loader::OnInit(void)
{
	WD_MainMenu *mm;
	WD_Splash *spl;
	wxTimer *timer;

	wxInitAllImageHandlers();
	initialize_images();
	this->splash_done = 0;

	timer = new wxTimer(this, ID_TICK);
	spl   = new WD_Splash();
	spl->Show(true);
	Yield(); /* increases "instantaneousness", better than Update() */
	timer->Start(1000, true);

	mm = new WD_MainMenu(PROD_NAME);
	console = new WD_Console();
	printf("Vitalnix Console initialized\n");

#ifndef INSTANT
	while (this->splash_done < 1)
		Yield();
#endif

	mm->Show(true);
	timer->Start(1000, true);
#ifndef INSTANT
	while (this->splash_done < 2)
		Yield();
#endif

	spl->Destroy();
	delete timer;
	return true;
}

void Loader::splash_tick(wxTimerEvent &event)
{
	++splash_done;
}

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(WD_Console, wxFrame)
	EVT_BUTTON(wxID_CLEAR, WD_Console::Clear)
	EVT_BUTTON(wxID_OK,    WD_Console::hide_command)
	EVT_CLOSE(WD_Console::hide_close)
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
	if ((current = fopencookie(NULL, "w", channel)) != NULL) {
		setvbuf(current, NULL, _IONBF, 0);
		saved_stdout = stdout;
		saved_stderr = stderr;
		stdout = stderr = current;
	}
#endif
}

WD_Console::~WD_Console(void)
{
	if (saved_stdout != NULL)
		stdout = saved_stdout;
	if (saved_stderr != NULL)
		stderr = saved_stderr;
	if (current != NULL)
		fclose(current);
}

void WD_Console::Append(const char *buffer)
{
	tc->WriteText(wxfu8(buffer));
}

void WD_Console::Clear(wxCommandEvent &event)
{
	tc->Clear();
}

void WD_Console::hide_command(wxCommandEvent &event)
{
	Hide();
}

void WD_Console::hide_close(wxCloseEvent &event)
{
	if (event.CanVeto()) {
		Hide();
		event.Veto();
	} else {
		wxFrame::Destroy();
	}
}

ssize_t WD_Console::write_cb(void *cookie, const char *buffer, size_t size)
{
	static_cast<Loader *>(wxTheApp)->console->Append(buffer);
	return size;
}
