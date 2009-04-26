/*
 *	steelmill/wd_usermod.cpp
 *	Copyright © CC Computer Consultants GmbH, 2006 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
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
#include <wx/notebook.h>
#include <libHX/wx_helper.hpp>
#include "steelmill/wd_usermod.hpp"
#include "steelmill/xu_common.hpp"

BEGIN_EVENT_TABLE(WD_Usermod, wxDialog)
END_EVENT_TABLE()

WD_Usermod::WD_Usermod(wxWindow *parent, unsigned int new_flag) :
	wxDialog(parent, wxID_ANY, wxT(PROD_NAME " > User Management > "
	 "Modify user"), wxDPOS, wxDSIZE, wxCFF)
{
	wxBoxSizer* vp = new wxBoxSizer(wxVERTICAL);
/*
	wxNotebook* itemNotebook3 = new wxNotebook( this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, wxNB_TOP );

	wxPanel* itemPanel4 = new wxPanel( itemNotebook3, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
	itemPanel4->SetSizer(itemBoxSizer5);

	wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(2, 2, 0, 0);
	itemFlexGridSizer6->AddGrowableCol(1);
	itemBoxSizer5->Add(itemFlexGridSizer6, 0, wxGROW|wxALL, 0);
	wxStaticText* itemStaticText7 = new wxStaticText( itemPanel4, wxID_STATIC, _("User's full name"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer6->Add(itemStaticText7, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	wxTextCtrl* itemTextCtrl8 = new wxTextCtrl( itemPanel4, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(10, -1), 0 );
	itemFlexGridSizer6->Add(itemTextCtrl8, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText9 = new wxStaticText( itemPanel4, wxID_STATIC, _("Username"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer6->Add(itemStaticText9, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	wxTextCtrl* itemTextCtrl10 = new wxTextCtrl( itemPanel4, ID_TEXTCTRL1, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer6->Add(itemTextCtrl10, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText11 = new wxStaticText( itemPanel4, wxID_STATIC, _("Password"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer6->Add(itemStaticText11, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	wxTextCtrl* itemTextCtrl12 = new wxTextCtrl( itemPanel4, ID_TEXTCTRL2, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
	itemFlexGridSizer6->Add(itemTextCtrl12, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText13 = new wxStaticText( itemPanel4, wxID_STATIC, _("Confirm password"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer6->Add(itemStaticText13, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	wxTextCtrl* itemTextCtrl14 = new wxTextCtrl( itemPanel4, ID_TEXTCTRL3, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD );
	itemFlexGridSizer6->Add(itemTextCtrl14, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxCheckBox* itemCheckBox15 = new wxCheckBox( itemPanel4, ID_CHECKBOX, _("Disable user login"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
	itemCheckBox15->SetValue(FALSE);
	itemBoxSizer5->Add(itemCheckBox15, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	itemNotebook3->AddPage(itemPanel4, _("User Data"));

	wxPanel* itemPanel16 = new wxPanel( itemNotebook3, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* itemBoxSizer17 = new wxBoxSizer(wxVERTICAL);
	itemPanel16->SetSizer(itemBoxSizer17);

	wxFlexGridSizer* itemFlexGridSizer18 = new wxFlexGridSizer(2, 2, 0, 0);
	itemFlexGridSizer18->AddGrowableCol(1);
	itemBoxSizer17->Add(itemFlexGridSizer18, 0, wxGROW|wxALL, 0);
	wxStaticText* itemStaticText19 = new wxStaticText( itemPanel16, wxID_STATIC, _("User ID (uid)"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer18->Add(itemStaticText19, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	wxSpinCtrl* itemSpinCtrl20 = new wxSpinCtrl( itemPanel16, ID_SPINCTRL, _("7"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 7 );
	itemFlexGridSizer18->Add(itemSpinCtrl20, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText21 = new wxStaticText( itemPanel16, wxID_STATIC, _("Home directory"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer18->Add(itemStaticText21, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	wxTextCtrl* itemTextCtrl22 = new wxTextCtrl( itemPanel16, ID_TEXTCTRL4, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer18->Add(itemTextCtrl22, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText23 = new wxStaticText( itemPanel16, wxID_STATIC, _("Login shell"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer18->Add(itemStaticText23, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	wxString itemComboBox24Strings[] = {
	    _("/bin/ash"),
	    _("/bin/bash"),
	    _("/bin/csh"),
	    _("/bin/false"),
	    _("/bin/sh"),
	    _("/bin/tcsh"),
	    _("/bin/true"),
	    _("/usr/bin/csh"),
	    _("/usr/bin/passwd"),
	    _("/usr/bin/rbash"),
	    _("/usr/bin/tcsh")
	};
	wxComboBox* itemComboBox24 = new wxComboBox( itemPanel16, ID_COMBOBOX, _("/bin/bash"), wxDefaultPosition, wxDefaultSize, 11, itemComboBox24Strings, wxCB_DROPDOWN );
	itemComboBox24->SetStringSelection(_("/bin/bash"));
	itemFlexGridSizer18->Add(itemComboBox24, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText25 = new wxStaticText( itemPanel16, wxID_STATIC, _("Primary group"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer18->Add(itemStaticText25, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	wxString itemChoice26Strings[] = {
	    _("users")
	};
	wxChoice* itemChoice26 = new wxChoice( itemPanel16, ID_CHOICE, wxDefaultPosition, wxDefaultSize, 1, itemChoice26Strings, 0 );
	itemChoice26->SetStringSelection(_("users"));
	itemFlexGridSizer18->Add(itemChoice26, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText27 = new wxStaticText( itemPanel16, wxID_STATIC, _("Secondary groups (max. 32)"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer18->Add(itemStaticText27, 0, wxALIGN_LEFT|wxALIGN_TOP|wxALL|wxADJUST_MINSIZE, 5);

	wxScrolledWindow* itemScrolledWindow28 = new wxScrolledWindow( itemPanel16, ID_SCROLLEDWINDOW, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxHSCROLL|wxVSCROLL );
	itemFlexGridSizer18->Add(itemScrolledWindow28, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, 5);
	itemScrolledWindow28->SetScrollbars(1, 1, 0, 0);
	wxBoxSizer* vp9 = new wxBoxSizer(wxVERTICAL);
	itemScrolledWindow28->SetSizer(vp9);

	wxCheckBox* itemCheckBox30 = new wxCheckBox( itemScrolledWindow28, ID_CHECKBOX1, _("audio"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
	itemCheckBox30->SetValue(FALSE);
	vp9->Add(itemCheckBox30, 0, wxGROW|wxLEFT|wxRIGHT, 5);

	wxCheckBox* itemCheckBox31 = new wxCheckBox( itemScrolledWindow28, ID_CHECKBOX2, _("users"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
	itemCheckBox31->SetValue(FALSE);
	vp9->Add(itemCheckBox31, 0, wxGROW|wxLEFT|wxRIGHT, 5);

	wxCheckBox* itemCheckBox32 = new wxCheckBox( itemScrolledWindow28, ID_CHECKBOX3, _("root"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
	itemCheckBox32->SetValue(FALSE);
	vp9->Add(itemCheckBox32, 0, wxGROW|wxLEFT|wxRIGHT, 5);

	itemNotebook3->AddPage(itemPanel16, _("Details"));

	wxPanel* itemPanel33 = new wxPanel( itemNotebook3, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* itemBoxSizer34 = new wxBoxSizer(wxVERTICAL);
	itemPanel33->SetSizer(itemBoxSizer34);

	wxFlexGridSizer* itemFlexGridSizer35 = new wxFlexGridSizer(2, 2, 0, 0);
	itemFlexGridSizer35->AddGrowableCol(1);
	itemBoxSizer34->Add(itemFlexGridSizer35, 0, wxGROW|wxALL, 0);
	wxStaticText* itemStaticText36 = new wxStaticText( itemPanel33, wxID_STATIC, _("Date of last password change"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer35->Add(itemStaticText36, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	wxTextCtrl* itemTextCtrl37 = new wxTextCtrl( itemPanel33, ID_TEXTCTRL5, _T(""), wxPoint(15, -1), wxDefaultSize, wxTE_READONLY );
	itemFlexGridSizer35->Add(itemTextCtrl37, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText38 = new wxStaticText( itemPanel33, wxID_STATIC, _("Days before experiation to warn"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer35->Add(itemStaticText38, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	wxSpinCtrl* itemSpinCtrl39 = new wxSpinCtrl( itemPanel33, ID_SPINCTRL1, _("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
	itemFlexGridSizer35->Add(itemSpinCtrl39, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText40 = new wxStaticText( itemPanel33, wxID_STATIC, _("Lockdown delay after expiration in days"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer35->Add(itemStaticText40, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	wxSpinCtrl* itemSpinCtrl41 = new wxSpinCtrl( itemPanel33, ID_SPINCTRL2, _("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
	itemFlexGridSizer35->Add(itemSpinCtrl41, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText42 = new wxStaticText( itemPanel33, wxID_STATIC, _("Minimum number of days\nto keep the same password"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer35->Add(itemStaticText42, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	wxSpinCtrl* itemSpinCtrl43 = new wxSpinCtrl( itemPanel33, ID_SPINCTRL3, _("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
	itemFlexGridSizer35->Add(itemSpinCtrl43, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText44 = new wxStaticText( itemPanel33, wxID_STATIC, _("Maximum number of days\nto keep the same password"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer35->Add(itemStaticText44, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	wxSpinCtrl* itemSpinCtrl45 = new wxSpinCtrl( itemPanel33, ID_SPINCTRL4, _("0"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
	itemFlexGridSizer35->Add(itemSpinCtrl45, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	wxStaticText* itemStaticText46 = new wxStaticText( itemPanel33, wxID_STATIC, _("Expiration date"), wxDefaultPosition, wxDefaultSize, 0 );
	itemFlexGridSizer35->Add(itemStaticText46, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

	wxTextCtrl* itemTextCtrl47 = new wxTextCtrl( itemPanel33, ID_TEXTCTRL6, _T(""), wxPoint(15, -1), wxDefaultSize, 0 );
	itemFlexGridSizer35->Add(itemTextCtrl47, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

	itemNotebook3->AddPage(itemPanel33, _("Password settings"));

	vp->Add(itemNotebook3, 0, wxGROW|wxALL, 5);

	vp->Add(1, 1, 1, wxALIGN_CENTER_HORIZONTAL|wxALL, 0);

	wxBoxSizer* itemBoxSizer49 = new wxBoxSizer(wxHORIZONTAL);
	vp->Add(itemBoxSizer49, 0, wxGROW|wxALL, 0);

	wxButton* itemButton50 = new wxButton( this, wxID_CANCEL, _("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	itemBoxSizer49->Add(itemButton50, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	itemBoxSizer49->Add(1, 1, 1, wxALIGN_CENTER_VERTICAL|wxALL, 0);

	wxButton* itemButton52 = new wxButton( this, wxID_OK, _("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
	itemBoxSizer49->Add(itemButton52, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

////@end WD_Useradd content construction*/
	SetSizer(vp);
	vp->SetSizeHints(this);
	Center();
}
