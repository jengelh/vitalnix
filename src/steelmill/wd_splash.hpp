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
