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
	DECLARE_NO_COPY_CLASS(WD_Usermod);
};

#endif /* STEELMILL_WD_USERMOD_HPP */
