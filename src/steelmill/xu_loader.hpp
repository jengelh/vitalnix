/*
 *	steelmill/xu_loader.hpp
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2005 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#ifndef STEELMILL_XU_LOADER_HPP
#define STEELMILL_XU_LOADER_HPP 1

#include <sys/types.h>
#include <cstdio>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

class WD_Console : public wxFrame {
    public: /* functions */
	WD_Console(void);
	~WD_Console(void);
	static ssize_t write_cb(void *, const char *, size_t);

    private: /* functions */
	void Append(const char *);
	void Clear(wxCommandEvent &);
	void hide_command(wxCommandEvent &);
	void hide_close(wxCloseEvent &);

    private: /* variables */
	FILE *saved_stdout, *saved_stderr, *current;
	wxTextCtrl *tc;
	DECLARE_EVENT_TABLE();
};

class Loader : public wxApp {
    public: /* functions */
	bool OnInit(void);

    public: /* variables */
	WD_Console *console;

    private: /* functions */
	void splash_tick(wxTimerEvent &);

    private: /* variables */
	unsigned int splash_done;
	DECLARE_EVENT_TABLE();
};

#endif /* STEELMILL_XU_LOADER_HPP */
