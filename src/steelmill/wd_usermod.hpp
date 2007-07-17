/*
 *	steelmill/wd_usermod.hpp
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2006 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#ifndef STEELMILL_WD_USERMOD_HPP
#define STEELMILL_WD_USERMOD_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

class WD_Usermod : public wxDialog {
    public: /* functions */
	WD_Usermod(wxWindow *, unsigned int = 0);

    private: /* variables */
	DECLARE_EVENT_TABLE();
};

#endif /* STEELMILL_WD_USERMOD_HPP */
