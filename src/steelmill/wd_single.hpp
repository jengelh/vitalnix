#ifndef STEELMILL_WD_SINGLE_HPP
#define STEELMILL_WD_SINGLE_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

class WD_Single : public wxDialog {
    public: /* functions */
	WD_Single(wxWindow *);

    private: /* variables */
	DECLARE_EVENT_TABLE();
	DECLARE_NO_COPY_CLASS(WD_Single);
};

#endif /* STEELMILL_WD_USERVIEW_HPP */
