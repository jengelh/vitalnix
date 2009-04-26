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
	DECLARE_NO_COPY_CLASS(WD_Pwlfmt);
};

#endif /* STEELMILL_WD_PWLFMT_HPP */
