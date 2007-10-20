/*
 *	steelmill/wd_about.hpp
 *	Copyright © CC Computer Consultants GmbH, 2005 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#ifndef STEELMILL_WD_ABOUT_HPP
#define STEELMILL_WD_ABOUT_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

class WD_About : public wxDialog {
    public: /* functions */
	WD_About(wxWindow *);
};

#endif /* STEELMILL_WD_ABOUT_HPP */
