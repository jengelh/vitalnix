/*
 *	steelmill/wd_overview.hpp
 *	Copyright © CC Computer Consultants GmbH, 2006 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#ifndef STEELMILL_WD_OVERVIEW_HPP
#define STEELMILL_WD_OVERVIEW_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

enum {
	OVERVIEW_USERS = 1,
	OVERVIEW_GROUPS,
};

class WD_Overview : public wxDialog {
    public: /* functions */
	WD_Overview(wxWindow *, unsigned int);

    private: /* functions */
	wxListCtrl *init_list(unsigned int);

    private: /* variables */
	DECLARE_EVENT_TABLE();
};

#endif /* STEELMILL_WD_OVERVIEW_HPP */
