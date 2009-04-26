#ifndef STEELMILL_WD_ABOUT_HPP
#define STEELMILL_WD_ABOUT_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif

class WD_About : public wxDialog {
    public: /* functions */
	WD_About(wxWindow *);

    private:
	DECLARE_NO_COPY_CLASS(WD_About);
};

#endif /* STEELMILL_WD_ABOUT_HPP */
