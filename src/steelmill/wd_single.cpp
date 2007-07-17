/*
 *	steelmill/wd_single.cpp
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2006 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif
#include "steelmill/wd_single.hpp"
#include "steelmill/xu_common.hpp"

BEGIN_EVENT_TABLE(WD_Single, wxDialog)
END_EVENT_TABLE()

WD_Single::WD_Single(wxWindow *parent) :
	wxDialog(parent, wxID_ANY, wxT(PROD_NAME " > Add single user"),
	         wxDPOS, wxDSIZE, wxCFF)
{
	wxBoxSizer *vp = new wxBoxSizer(wxVERTICAL);

	wxGridBagSizer *sg = new wxGridBagSizer();
	sg->AddGrowableCol(1);

	/*
	 * The text strings are copied from src/clutils/mdsingle. If you change
	 * any of them, change the counterpart accordingly. (But note that the
	 * strings in Steelmill have newlines in them while mdsingle's do not.)
	 */
	sg->Add(new wxStaticText(this, wxID_ANY, wxT(
		"Note that Vitalnix/mdsingle will not check if a user with\n"
		"with same UUID already exists. Make sure you do not add a\n"
		"user twice by accident."
	)), wxGBPosition(0, 0), wxGBSpan(1, 2), wxALIGN_LEFT | wxACV | wxALL, 3);
	sg->Add(new wxStaticText(this, wxID_ANY, wxT("First name")), wxGBPosition(1, 0), wxDefaultSpan, wxALIGN_RIGHT | wxACV | wxALL, 3);
	sg->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString), wxGBPosition(1, 1), wxDefaultSpan, wxGROW | wxACV | wxALL, 3);
	sg->Add(new wxStaticText(this, wxID_ANY, wxT("Last name")), wxGBPosition(2, 0), wxDefaultSpan, wxALIGN_RIGHT | wxACV | wxALL, 3);
	sg->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString), wxGBPosition(2, 1), wxDefaultSpan, wxGROW | wxACV | wxALL, 3);

	sg->Add(new wxStaticText(this, wxID_ANY, wxT(
		"This is the proposed username of the new account,\n"
		"which you may change. If necessary, Vitalnix will add\n"
		"an index number to the username or adjust it to avoid\n"
		"conflict with an already existing username."
	)), wxGBPosition(3, 0), wxGBSpan(1, 2), wxALIGN_LEFT | wxACV | wxALL, 3);
	sg->Add(new wxStaticText(this, wxID_ANY, wxT("Preferred username")), wxGBPosition(4, 0), wxDefaultSpan, wxALIGN_RIGHT | wxACV | wxALL, 3);
	sg->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString), wxGBPosition(4, 1), wxDefaultSpan, wxGROW | wxACV | wxALL, 3);

	sg->Add(new wxStaticText(this, wxID_ANY, wxT("System group")), wxGBPosition(5, 0), wxDefaultSpan, wxALIGN_RIGHT | wxACV | wxALL, 3);
	sg->Add(new GW_GroupCombo(this), wxGBPosition(5, 1), wxDefaultSpan, wxACV | wxALL, 3);


	sg->Add(new wxStaticText(this, wxID_ANY, wxT(
		"Enter a user-defined subgroup of the new user."
	)), wxGBPosition(6, 0), wxGBSpan(1, 2), wxALIGN_LEFT | wxACV | wxALL, 3);
	sg->Add(new wxStaticText(this, wxID_ANY, wxT("Private group")), wxGBPosition(7, 0), wxDefaultSpan, wxALIGN_RIGHT | wxACV | wxALL, 3);
	sg->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString), wxGBPosition(7, 1), wxDefaultSpan, wxGROW | wxACV | wxALL, 3);


	sg->Add(new wxStaticText(this, wxID_ANY, wxT(
		"If there is no external UUID available to uniquely\n"
		"identify the user, you can have one made up from the\n"
		"name-day tuple. In that case, enter the birthdate of\n"
		"the user, otherwise leave blank."
	)), wxGBPosition(8, 0), wxGBSpan(1, 2), wxALIGN_LEFT | wxACV | wxALL, 3);
	sg->Add(new wxStaticText(this, wxID_ANY, wxT("Birth date")), wxGBPosition(9, 0), wxDefaultSpan, wxALIGN_RIGHT | wxACV | wxALL, 3);
	sg->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString), wxGBPosition(9, 1), wxDefaultSpan, wxGROW | wxACV | wxALL, 3);


	sg->Add(new wxStaticText(this, wxID_ANY, wxT(
		"Enter the external UUID of the user. If you specify\n"
		"something here, it will override the automatic UUID\n"
		"generation from the birthdate."
	)), wxGBPosition(10, 0), wxGBSpan(1, 2), wxALIGN_LEFT | wxACV | wxALL, 3);
	sg->Add(new wxStaticText(this, wxID_ANY, wxT("External UUID")), wxGBPosition(11, 0), wxDefaultSpan, wxALIGN_RIGHT | wxACV | wxALL, 3);
	sg->Add(new wxTextCtrl(this, wxID_ANY, wxEmptyString), wxGBPosition(11, 1), wxDefaultSpan, wxGROW | wxACV | wxALL, 3);

	vp->Add(sg);
	vp->Add(smc_navgen(this, "c-n"), 0, wxGROW);
	SetSizer(vp);
	vp->SetSizeHints(this);
	Center();
	return;
}
