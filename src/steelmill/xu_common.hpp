/*
 *	steelmill/xu_common.hpp
 *	Copyright © CC Computer Consultants GmbH, 2005 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#ifndef STEELMILL_COMMON_HPP
#define STEELMILL_COMMON_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif
#include <wx/gbsizer.h>

/* Definitions */
#define PROD_NAME       "Vitalnix 2007"

enum {
	FTC_DIRECTORY = 1 << 0,
};

extern wxPanel *smc_logo_panel(wxWindow *);
extern wxBoxSizer *smc_navgen(wxWindow *, const char * = "", unsigned int = 3);
extern void smc_size_aspect(wxWindow *, double = 1.5);
extern void smc_size_minimum(wxWindow *, int, int);
extern void smc_size_minimum(wxWindow *, const wxSize &);

class GW_FTC : public wxPanel {
    public: /* functions */
	GW_FTC(wxWindow *, const wxString & = wxEmptyString, unsigned int = 0,
		unsigned int = 0);
	wxString GetValue(void) const;

    protected: /* functions */
	void Browse(wxCommandEvent &);

    protected: /* variables */
	wxTextCtrl *textfield;
	unsigned int flags;

    private: /* variables */
	DECLARE_EVENT_TABLE();
	DECLARE_NO_COPY_CLASS(GW_FTC);
};

class GW_GroupCombo : public wxComboBox {
    public: /* functions */
	GW_GroupCombo(wxWindow *, wxWindowID = wxID_ANY, const char * = "*");
	void switch_database(const char *);

    private:
	DECLARE_NO_COPY_CLASS(GW_GroupCombo);
};

class GW_Listbox : public wxDialog {
    public: /* functions */
	GW_Listbox(wxWindow *, const wxString &, const struct HXdeque *,
		long = 0);
	GW_Listbox(wxWindow *, const wxString & = wxEmptyString,
		void (*)(wxListBox *, const void *) = NULL, const void * = NULL,
		long = 0);

	int Append(const wxString &s) { return ct_listbox->Append(s); };

    protected: /* variables */
	wxListBox *ct_listbox;

    private:
	DECLARE_NO_COPY_CLASS(GW_Listbox);
};

class GW_Message : public wxDialog {
    public: /* functions */
	GW_Message(wxWindow *, const wxString & = wxEmptyString,
		const wxString & = wxEmptyString, const char * = "");

    protected: /* functions */
	void Yes(wxCommandEvent &);
	void No(wxCommandEvent &);
	void Cancel(wxCommandEvent &);

    private: /* variables */
	DECLARE_EVENT_TABLE();
	DECLARE_NO_COPY_CLASS(GW_Message);
};

class GW_Table : public wxListCtrl {
    public: /* functions */
	GW_Table(wxWindow *, wxWindowID, const wxString *, int,
		const int *, long = 0);
	~GW_Table(void);
	void Insert(const wxString *);

    protected: /* functions */
	void Resize_Column(wxListEvent &);
	void Resize_Table(wxSizeEvent &);

    protected: /* variables */
	int *column_layout, column_layout_sum;
	int num_columns;

    private: /* variables */
	DECLARE_EVENT_TABLE();
	DECLARE_NO_COPY_CLASS(GW_Table);
};

class GW_UserCombo : public wxComboBox {
    public: /* functions */
	GW_UserCombo(wxWindow *, wxWindowID = wxID_ANY, const char * = "*");
	void switch_database(const char *);

    private:
	DECLARE_NO_COPY_CLASS(GW_UserCombo);
};

#endif /* STEELMILL_COMMON_HPP */
