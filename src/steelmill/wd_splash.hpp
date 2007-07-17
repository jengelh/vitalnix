/*
 *	steelmill/wd_splash.hpp
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2006 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#ifndef STEELMILL_SPLASH_HPP
#define STEELMILL_SPLASH_HPP

#include <wx/wxprec.h>
#ifndef WD_PRECOMP
#	include <wx/wx.h>
#endif
#include "steelmill/xu_common.hpp"

class WD_Splash : public wxDialog {
    public: /* functions */
	WD_Splash(const wxPoint & = wxDPOS);
};

#endif /* STEELMILL_SPLASH_HPP */
