/*
 *	steelmill/wd_pwlfmt.hpp
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2005 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#ifndef STEELMILL_WD_PWLFMT_HPP
#define STEELMILL_WD_PWLFMT_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif
#include "steelmill/xu_common.hpp"

class WD_Pwlfmt : public wxDialog {
    public: /* functions */
	WD_Pwlfmt(wxWindow *);

    private: /* functions */
	void change_style(wxCommandEvent &);
	void process(wxCommandEvent &);

    private: /* variables */
	GW_FTC *ct_input, *ct_output, *ct_template;
	wxChoice *ct_style;
	wxStaticText *ct_tpltext;
	DECLARE_EVENT_TABLE();
};

#endif /* STEELMILL_WD_PWLFMT_HPP */
