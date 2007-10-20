/*
 *	steelmill/wd_sync.hpp
 *	Copyright © CC Computer Consultants GmbH, 2005 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
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
};

#endif /* STEELMILL_WD_SYNC_HPP */
