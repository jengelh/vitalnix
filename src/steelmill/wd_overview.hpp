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
