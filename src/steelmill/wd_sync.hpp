#ifndef STEELMILL_WD_SYNC_HPP
#define STEELMILL_WD_SYNC_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif
#include <wx/spinctrl.h>
#include "steelmill/xu_common.hpp"

struct pv_sync;

class WD_SyncParam : public wxDialog {
    public: /* functions */
	WD_SyncParam(wxWindow *);

    private: /* functions */
	void Compare(wxCommandEvent &);

    private: /* variables */
	struct pv_sync *priv;
	GW_FTC *ct_source;
	wxChoice *ct_srctype;
	wxComboBox *ct_module, *ct_group;
	GW_FTC *ct_output;
	wxCheckBox *ct_prompt;

	DECLARE_EVENT_TABLE();
	DECLARE_NO_COPY_CLASS(WD_SyncParam);
};

#endif /* STEELMILL_WD_SYNC_HPP */
