/*
 *	steelmill/wd_sync.cpp
 *	Copyright © CC Computer Consultants GmbH, 2005 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <sys/time.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <vitalnix/libvxeds/libvxeds.h>
#include <vitalnix/libvxeds/vtable.h>
#include <vitalnix/libvxmdsync/libvxmdsync.h>
#include <vitalnix/libvxdb/libvxdb.h>
#include <vitalnix/libvxdb/xafunc.h>
#include <vitalnix/libvxutil/defines.h>
#include <vitalnix/libvxutil/libvxutil.h>
#include "steelmill/wd_sync.hpp"
#include "steelmill/xu_common.hpp"

/* Definitions */
enum {
	/* StaticText for countdowns */
	ID_CD_BASE          = 0x10,
	ID_CD_COMPARE       = ID_CD_BASE | MDREP_COMPARE,
	ID_CD_ADD           = ID_CD_BASE | MDREP_ADD,
	ID_CD_UPDATE        = ID_CD_BASE | MDREP_UPDATE,
	ID_CD_DSTART        = ID_CD_BASE | MDREP_DSTART,
	ID_CD_DWAIT         = ID_CD_BASE | MDREP_DWAIT,
	ID_CD_DSTOP         = ID_CD_BASE | MDREP_DSTOP,
	ID_CD_DELETE        = ID_CD_BASE | MDREP_DELETE,
	/* Gauges */
	ID_GG_BASE          = 0x20,
	ID_GG_ADD           = ID_GG_BASE | MDREP_ADD,
	ID_GG_UPDATE        = ID_GG_BASE | MDREP_UPDATE,
	ID_GG_DSTART        = ID_GG_BASE | MDREP_DSTART,
	ID_GG_DSTOP         = ID_GG_BASE | MDREP_DSTOP,
	ID_GG_DELETE        = ID_GG_BASE | MDREP_DELETE,
	ID_GG_COMPARE       = ID_GG_BASE | MDREP_COMPARE,
	ID_GG_FIXUP         = ID_GG_BASE | MDREP_FIXUP,
	/* StaticText for % */
	ID_PT_BASE          = 0x40,
	ID_PT_ADD           = ID_PT_BASE | MDREP_ADD,
	ID_PT_UPDATE        = ID_PT_BASE | MDREP_UPDATE,
	ID_PT_DSTART        = ID_PT_BASE | MDREP_DSTART,
	ID_PT_DSTOP         = ID_PT_BASE | MDREP_DSTOP,
	ID_PT_DELETE        = ID_PT_BASE | MDREP_DELETE,
	ID_PT_COMPARE       = ID_PT_BASE | MDREP_COMPARE,
	ID_PT_FIXUP         = ID_PT_BASE | MDREP_FIXUP,
	/* "Details" buttons */
	ID_DT_BASE          = 0x80,
	ID_DT_COMPARE       = ID_DT_BASE | MDREP_COMPARE,
	ID_DT_ADD           = ID_DT_BASE | MDREP_ADD,
	ID_DT_UPDATE        = ID_DT_BASE | MDREP_UPDATE,
	ID_DT_DSTART        = ID_DT_BASE | MDREP_DSTART,
	ID_DT_DWAIT         = ID_DT_BASE | MDREP_DWAIT,
	ID_DT_DSTOP         = ID_DT_BASE | MDREP_DSTOP,
	ID_DT_DELETE        = ID_DT_BASE | MDREP_DELETE,
};

struct pv_sync {
	struct vxdb_state *module_handle;
	struct vxdb_group group;
	struct mdsync_workspace *mdsw;
	bool prompt;
	int open_status;
	long num_eds_users;
	wxWindow *window;
	struct timeval last_print;
};

class WD_SyncCompare : public wxDialog {
    public: /* functions */
	WD_SyncCompare(wxWindow *, struct pv_sync *);
};

class WD_SyncProg : public wxDialog {
    public: /* functions */
	WD_SyncProg(wxWindow *, struct pv_sync *);

    private: /* functions */
	void Continue(wxCommandEvent &);
	void Details_Add(wxCommandEvent &);
	void Details_Update(wxCommandEvent &);
	void Details_Defer_Start(wxCommandEvent &);
	void Details_Defer_Wait(wxCommandEvent &);
	void Details_Defer_Stop(wxCommandEvent &);
	void Details_Delete(wxCommandEvent &);

    private: /* variables */
	struct pv_sync *priv;
	DECLARE_EVENT_TABLE();
};

class GW_EdsformatChoice : public wxChoice {
    public: /* functions */
	GW_EdsformatChoice(wxWindow *, wxWindowID = wxID_ANY);
};

class UserException {};

/* Functions */
static void cb_report(unsigned int, const struct mdsync_workspace *,
	unsigned int, unsigned int);
template<class Object> static inline Object find_window(long, wxWindow *);
static void listbox_fill_eds(wxListBox *, const void *);
static void listbox_fill_user(wxListBox *, const void *);
static void release_priv(struct pv_sync *);
static int time_limit(struct timeval *, const struct timeval *);
static void tv_delta(const struct timeval *, const struct timeval *,
	struct timeval *);
static void update_max(wxWindow *, long);
static wxString wxString_int(unsigned int);

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(WD_SyncParam, wxDialog)
	EVT_BUTTON(wxID_FORWARD, WD_SyncParam::Compare)
END_EVENT_TABLE()

WD_SyncParam::WD_SyncParam(wxWindow *parent) :
	wxDialog(parent, wxID_ANY, wxT(PROD_NAME " > Synchronize"),
	         wxDPOS, wxDSIZE, wxCFF)
{
	wxBoxSizer *vp = new wxBoxSizer(wxVERTICAL);

	wxStaticBoxSizer *sp_src = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Source"));
	wxFlexGridSizer *sf_src  = new wxFlexGridSizer(2);
	sf_src->AddGrowableCol(1);
	sf_src->Add(new wxStaticText(this, wxID_ANY, wxT("Resource Identifier:")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	sf_src->Add(ct_source = new GW_FTC(this, wxT("../../../Vitalnix_SEC/daten3.sdf"), 3), 0, wxGROW | wxACV);
	sf_src->Add(new wxStaticText(this, wxID_ANY, wxT("Type:")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	sf_src->Add(ct_srctype = new GW_EdsformatChoice(this), 0, wxACV | wxALL, 3);
	sp_src->Add(sf_src, 1, wxGROW);

	wxStaticBoxSizer *sp_dst = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Destination"));
	wxFlexGridSizer *sf_dst  = new wxFlexGridSizer(2);
	sf_dst->AddGrowableCol(1);
	sf_dst->Add(new wxStaticText(this, wxID_ANY, wxT("Database")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	sf_dst->Add(ct_module = new wxComboBox(this, wxID_ANY, wxT("*")), 0, wxACV | wxALL, 3);
	sf_dst->Add(new wxStaticText(this, wxID_ANY, wxT("Group or GID")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	sf_dst->Add(ct_group = new GW_GroupCombo(this), 0, wxACV | wxALL, 3);
	sp_dst->Add(sf_dst, 1, wxGROW);

	wxStaticBoxSizer *sp_out = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Output"));
	wxFlexGridSizer *sf_out  = new wxFlexGridSizer(2);
	sf_out->AddGrowableCol(1);
	sf_out->Add(new wxStaticText(this, wxID_ANY, wxT("Logfile:")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	sf_out->Add(ct_output = new GW_FTC(this, wxT("../../Vitalnix-PRV/daten3.log"), 3), 0, wxGROW | wxACV);
	sp_out->Add(sf_out, 1, wxGROW);

	sp_out->Add(ct_prompt = new wxCheckBox(this, wxID_ANY, wxT("Prompt before operations")), 0, wxALIGN_LEFT | wxALL, 3);

	ct_srctype->SetSelection(0);
	ct_group->SetValue(wxT("extern"));

	vp->Add(sp_src, 0, wxGROW | wxALL, 5);
	vp->Add(sp_dst, 0, wxGROW | wxALL, 5);
	vp->Add(sp_out, 0, wxGROW | wxALL, 5);
	vp->Add(-1, 30, 1);
	vp->Add(smc_navgen(this, "c-n"), 0, wxGROW);

	SetSizer(vp);
	vp->SetSizeHints(this);
	smc_size_minimum(this, 400, 300);
	smc_size_aspect(this);
	Center();

	priv = NULL;
}

void WD_SyncParam::Compare(wxCommandEvent &event)
{
	wxString group_name, input_file, output_file, str;
	const char *format;
	int ret;

	try {
		if ((input_file = ct_source->GetValue()) == wxEmptyString) {
			str = wxT("Data Source may not be empty!");
				throw UserException();
		}
		if ((output_file = ct_output->GetValue()) == wxEmptyString) {
				str = wxT("Output logfile may not be empty!");
			throw UserException();
		}
		if (ct_module->GetValue() == wxEmptyString) {
			str = wxT("Database selector may not be empty!");
			throw UserException();
		}
		if ((group_name = ct_group->GetValue()) == wxEmptyString) {
			str = wxT("Group may not be empty!");
			throw UserException();
		}

		priv = new struct pv_sync;
		memset(priv, 0, sizeof(*priv));

		priv->module_handle = vxdb_load(tU8(ct_module->GetValue()));
		if (priv->module_handle == NULL) {
			str.Printf(wxT("Could not load VXDB backend module: %s\n"),
			           fV8(strerror(errno)));
			throw UserException();
		}

		if ((ret = vxdb_open(priv->module_handle, VXDB_WRLOCK)) <= 0) {
			str.Printf(wxT("Could not open VXDB module: %s\n"),
			           fV8(strerror(-ret)));
			throw UserException();
		}

		priv->open_status = 1;
		if ((priv->mdsw = mdsync_init()) == NULL) {
			str.Printf(wxT("mdsync_init(): %s\n"),
			           fV8(strerror(errno)));
			throw UserException();
		}

		priv->mdsw->database     = priv->module_handle;
		priv->mdsw->report       = cb_report;
		priv->mdsw->user_private = priv;
		priv->window             = this;

		if ((ret = mdsync_prepare_group(priv->mdsw, tU8(group_name))) < 0) {
			str.Printf(wxT("Could not look up group \"%s\": %s"),
			           group_name.c_str(), fV8(strerror(-ret)));
			throw UserException();
		} else if (ret == 0) {
			str.Printf(wxT("Group \"%s\" does not exist"), group_name.c_str());
			throw UserException();
		}

		format = static_cast<const char *>
		         (ct_srctype->GetClientData(ct_srctype->GetSelection()));
		if (format == NULL && (format = vxeds_derivefromname(tU8(input_file))) == NULL) {
			str = wxT("Could not determine file type of input data source.");
			throw UserException();
		}

		if ((ret = mdsync_read_file(priv->mdsw, tU8(input_file), format)) <= 0) {
			str.Printf(wxT("Error while reading Data Source: %s\n"),
			           fV8(strerror(-ret)));
			throw UserException();
		}

		if ((ret = mdsync_open_log(priv->mdsw, tU8(output_file))) <= 0) {
			str.Printf(wxT("Error trying to open logfile: %s\n"),
				       fV8(strerror(-ret)));
			throw UserException();
		}

		priv->prompt = ct_prompt->GetValue();

		WD_SyncCompare cmp_dlg(this, priv);
		cmp_dlg.Show();
		wxTheApp->Yield();
		mdsync_compare(priv->mdsw);
		update_max(&cmp_dlg, MDREP_COMPARE);
		mdsync_fixup(priv->mdsw);
		update_max(&cmp_dlg, MDREP_FIXUP);
		cmp_dlg.Hide();

		Hide();
		WD_SyncProg(this, priv).ShowModal();
		Close(true);
	} catch(...) {
		GW_Message(this, wxT("Error"), str, "-o").ShowModal();
	}

	release_priv(priv);
}

//-----------------------------------------------------------------------------
WD_SyncCompare::WD_SyncCompare(wxWindow *parent, struct pv_sync *priv) :
	wxDialog(parent, wxID_ANY,
	         wxT(PROD_NAME " > Synchronize > Compare stage"),
	         wxDPOS, wxDSIZE, wxCFF)
{
	wxFlexGridSizer *st = new wxFlexGridSizer(3);
	wxBoxSizer *vp      = new wxBoxSizer(wxVERTICAL);

	st->AddGrowableCol(1);

	st->Add(new wxStaticText(this, wxID_ANY, wxT("Compare")), 0, wxALIGN_LEFT | wxACV | wxALL, 3);
	st->Add(new wxGauge(this, ID_GG_COMPARE, 1, wxDPOS, wxDSIZE, wxGA_SMOOTH), 0, wxGROW | wxACV | wxALL, 3);
	st->Add(new wxStaticText(this, ID_PT_COMPARE, wxT("0%"), wxDPOS, wxSize(6 * GetCharWidth(), -1), wxALIGN_RIGHT), 0, wxALIGN_RIGHT | wxACV | wxALL, 0);

	st->Add(new wxStaticText(this, wxID_ANY, wxT("Fixup")), 0, wxALIGN_LEFT | wxACV | wxALL, 3);
	st->Add(new wxGauge(this, ID_GG_FIXUP, 1, wxDPOS, wxDSIZE, wxGA_SMOOTH), 0, wxGROW | wxACV | wxALL, 3);
	st->Add(new wxStaticText(this, ID_PT_FIXUP, wxT("0%")), 0, wxALIGN_RIGHT | wxACV | wxALL, 0);

	vp->Add(new wxStaticText(this, wxID_ANY, wxT("Comparing EDS to VXDB... Please wait.")), 0, wxALIGN_LEFT | wxALL, 10);
	vp->Add(st, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 10);
	vp->Add(new wxStaticLine(this, wxID_ANY), 0, wxGROW | wxTOP | wxBOTTOM, 5);

	SetSizer(vp);
	vp->SetSizeHints(this);
	smc_size_aspect(this, 3);
	Center();

	priv->window = this;
}

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(WD_SyncProg, wxDialog)
	EVT_BUTTON(wxID_FORWARD, WD_SyncProg::Continue)
	EVT_BUTTON(ID_DT_ADD,    WD_SyncProg::Details_Add)
	EVT_BUTTON(ID_DT_UPDATE, WD_SyncProg::Details_Update)
	EVT_BUTTON(ID_DT_DSTART, WD_SyncProg::Details_Defer_Start)
	EVT_BUTTON(ID_DT_DWAIT,  WD_SyncProg::Details_Defer_Wait)
	EVT_BUTTON(ID_DT_DSTOP,  WD_SyncProg::Details_Defer_Stop)
	EVT_BUTTON(ID_DT_DELETE, WD_SyncProg::Details_Delete)
END_EVENT_TABLE()

WD_SyncProg::WD_SyncProg(wxWindow *parent, struct pv_sync *priv) :
	wxDialog(parent, wxID_ANY,
	         wxT(PROD_NAME " > Synchronize > Operation in progress"),
	         wxDPOS, wxDSIZE, wxCFF)
{
	struct mdsync_workspace *mdsw = priv->mdsw;
	wxFlexGridSizer *st = new wxFlexGridSizer(5);
	wxBoxSizer *vp0     = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *vp1     = new wxBoxSizer(wxVERTICAL);

	st->AddGrowableCol(2);

	vp1->Add(new wxStaticText(this, wxID_ANY, wxT(
		"Please wait until the selected operations have been completed.\n"
		"This may take a while depending on your configuration.\n"
		"Process might not occur linearly.")),
		0, wxACV | wxALIGN_LEFT | wxALL, 5);

	st->Add(new wxStaticText(this, wxID_ANY, wxString_int(mdsw->num_grp)), 0, wxALIGN_RIGHT | wxALL, 3);
	st->Add(new wxStaticText(this, wxID_ANY, wxT("group member(s) found in VXDB")), 0, wxALIGN_LEFT | wxACV | wxALL, 3);
	st->Add(-1, -1);
	st->Add(-1, -1);
	st->Add(-1, -1);

	st->Add(new wxStaticText(this, ID_CD_ADD, wxString_int(mdsw->add_req->items), wxDPOS, wxSize(8 * GetCharWidth(), -1), wxALIGN_RIGHT), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	st->Add(new wxStaticText(this, wxID_ANY, wxT("new users to add")), 0, wxALIGN_LEFT | wxACV | wxALL, 3);
	st->Add(new wxGauge(this, ID_GG_ADD, 1, wxDPOS, wxDSIZE, wxGA_SMOOTH), 0, wxGROW | wxACV | wxALL, 3);
	st->Add(new wxStaticText(this, ID_PT_ADD, wxT("0%"), wxDPOS, wxSize(6 * GetCharWidth(), -1), wxALIGN_RIGHT), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	st->Add(new wxButton(this, ID_DT_ADD, wxT("Details")), 0, wxGROW | wxACV | wxALL, 3);

	st->Add(new wxStaticText(this, ID_CD_UPDATE, wxString_int(mdsw->update_req->items), wxDPOS, wxDSIZE, wxALIGN_RIGHT), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	st->Add(new wxStaticText(this, wxID_ANY, wxT("to keep and update group descriptors")), 0, wxALIGN_LEFT | wxACV | wxALL, 3);
	st->Add(new wxGauge(this, ID_GG_UPDATE, 1, wxDPOS, wxDSIZE, wxGA_SMOOTH), 0, wxGROW | wxACV | wxALL, 3);
	st->Add(new wxStaticText(this, ID_PT_UPDATE, wxT("0%")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	st->Add(new wxButton(this, ID_DT_UPDATE, wxT("Details")), 0, wxGROW | wxACV | wxALL, 3);

	st->Add(new wxStaticText(this, ID_CD_DSTART, wxString_int(mdsw->defer_start->items)), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	st->Add(new wxStaticText(this, wxID_ANY, wxT("to start deferred deletion timer")), 0, wxALIGN_LEFT | wxACV | wxALL, 3);
	st->Add(new wxGauge(this, ID_GG_DSTART, 1, wxDPOS, wxDSIZE, wxGA_SMOOTH), 0, wxGROW | wxACV | wxALL, 3);
	st->Add(new wxStaticText(this, ID_PT_DSTART, wxT("0%")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	st->Add(new wxButton(this, ID_DT_DSTART, wxT("Details")), 0, wxGROW | wxACV | wxALL, 3);

	st->Add(new wxStaticText(this, wxID_ANY, wxString_int(mdsw->defer_wait->items)), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	st->Add(new wxStaticText(this, wxID_ANY, wxT("to wait for deletion")), 0, wxALIGN_LEFT | wxACV | wxALL, 3);
	st->Add(-1, -1);
	st->Add(-1, -1);
	st->Add(new wxButton(this, ID_DT_DWAIT, wxT("Details")), 0, wxGROW | wxACV | wxALL, 3);

	st->Add(new wxStaticText(this, ID_CD_DSTOP, wxString_int(mdsw->defer_stop->items)), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	st->Add(new wxStaticText(this, wxID_ANY, wxT("to stop deferred deletion timer")), 0, wxALIGN_LEFT | wxACV | wxALL, 3);
	st->Add(new wxGauge(this, ID_GG_DSTOP, 1, wxDPOS, wxDSIZE, wxGA_SMOOTH), 0, wxGROW | wxACV | wxALL, 3);
	st->Add(new wxStaticText(this, ID_PT_DSTOP, wxT("0%")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	st->Add(new wxButton(this, ID_DT_DSTOP, wxT("Details")), 0, wxGROW | wxACV | wxALL, 3);

	st->Add(new wxStaticText(this, ID_CD_DELETE, wxString_int(mdsw->delete_now->items)), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	st->Add(new wxStaticText(this, wxID_ANY, wxT("to delete")), 0, wxALIGN_LEFT | wxACV | wxALL, 3);
	st->Add(new wxGauge(this, ID_GG_DELETE, 1, wxDPOS, wxDSIZE, wxGA_SMOOTH), 0, wxGROW | wxACV | wxALL, 3);
	st->Add(new wxStaticText(this, ID_PT_DELETE, wxT("0%")), 0, wxALIGN_RIGHT | wxACV | wxALL, 3);
	st->Add(new wxButton(this, ID_DT_DELETE, wxT("Details")), 0, wxGROW | wxACV | wxALL, 3);

	vp1->Add(st, 0, wxGROW);
	vp0->Add(vp1, 1, wxGROW | wxALL, 20);
	vp0->Add(smc_navgen(this, "c-n"), 0, wxGROW);

	SetSizer(vp0);
	vp0->SetSizeHints(this);
	smc_size_aspect(this, 2);
	Center();

	this->priv   = priv;
	priv->window = this;
}

void WD_SyncProg::Continue(wxCommandEvent &event)
{
	struct mdsync_workspace *mdsw = priv->mdsw;
	wxButton *btn;

	btn = find_window<wxButton *>(wxID_FORWARD, this);
	btn->Disable();

	GW_Message dlg_add(this, wxT(PROD_NAME " > Synchronize > Adding users"),
		wxT("Continue with adding users?"), "-oc-");
	if (mdsw->add_req->items > 0 && (!priv->prompt ||
	    dlg_add.ShowModal() == wxID_OK))
		mdsync_add(mdsw);

	update_max(this, MDREP_ADD);

	GW_Message dlg_mod(this, wxT(PROD_NAME " > Synchronize > Update timers"),
		wxT("Continue with updating the deferred deletion timers?"), "-oc");
	if (mdsw->defer_start->items + mdsw->defer_stop->items > 0 &&
	    (!priv->prompt || dlg_mod.ShowModal() == wxID_OK))
		mdsync_mod(mdsw);

	update_max(this, MDREP_UPDATE);
	update_max(this, MDREP_DSTART);
	update_max(this, MDREP_DSTOP);

	GW_Message dlg_del(this, wxT(PROD_NAME " > Synchronize > Delete users"),
		wxT("Continue with deleting old users?"), "-oc-");
	if (mdsw->delete_now->items > 0 && (!priv->prompt ||
	    dlg_del.ShowModal() == wxID_OK))
		mdsync_del(mdsw);

	update_max(this, MDREP_DELETE);

	btn = find_window<wxButton *>(wxID_CANCEL, this);
	btn->Disable();

	btn = find_window<wxButton *>(wxID_FORWARD, this);
	btn->SetLabel(wxT("&Finish"));
	btn->SetId(wxID_OK);
	btn->Enable();

	GetSizer()->Layout();
}

void WD_SyncProg::Details_Update(wxCommandEvent &event)
{
	GW_Listbox(this, wxEmptyString, listbox_fill_user,
	           priv->mdsw->update_req->root, wxLB_SORT).ShowModal();
}

void WD_SyncProg::Details_Add(wxCommandEvent &event)
{
	GW_Listbox(this, wxEmptyString, listbox_fill_eds,
	           priv->mdsw->add_req->root, wxLB_SORT).ShowModal();
}

void WD_SyncProg::Details_Defer_Start(wxCommandEvent &event)
{
	GW_Listbox(this, wxEmptyString, priv->mdsw->defer_start,
	           wxLB_SORT).ShowModal();
}

void WD_SyncProg::Details_Defer_Wait(wxCommandEvent &event)
{
	GW_Listbox(this, wxEmptyString, priv->mdsw->defer_wait,
	           wxLB_SORT).ShowModal();
}

void WD_SyncProg::Details_Defer_Stop(wxCommandEvent &event)
{
	GW_Listbox(this, wxEmptyString, priv->mdsw->defer_stop,
	           wxLB_SORT).ShowModal();
}

void WD_SyncProg::Details_Delete(wxCommandEvent &event)
{
	GW_Listbox(this, wxEmptyString, priv->mdsw->delete_now,
	           wxLB_SORT).ShowModal();
}

//-----------------------------------------------------------------------------
GW_EdsformatChoice::GW_EdsformatChoice(wxWindow *parent, wxWindowID id) :
	wxChoice(parent, id, wxDPOS, wxDSIZE)
{
	const struct edsformat_vtable *vtable;
	void *trav = NULL;

	Append(wxT("(autodetect)"), static_cast<void *>(NULL));
	while ((vtable = vxeds_formats_trav(&trav)) != NULL)
		Append(fU8(vtable->desc),
		       const_cast<struct edsformat_vtable *>(vtable));
}

//-----------------------------------------------------------------------------
static void cb_report(unsigned int type, const struct mdsync_workspace *mdsw,
      unsigned int current, unsigned int max)
{
	static const struct timeval report_delta = {0, 100000};
	struct pv_sync *priv = static_cast<struct pv_sync *>(mdsw->user_private);
	wxStaticText *cd, *pt;
	wxGauge *gg;

	if (current == 1)
		memset(&priv->last_print, 0, sizeof(priv->last_print));
	if (!time_limit(&priv->last_print, &report_delta) && current != max)
		return;

	wxString s;
	cd = static_cast<wxStaticText *>(priv->window->FindWindowById(type | ID_CD_BASE, priv->window));
	pt = static_cast<wxStaticText *>(priv->window->FindWindowById(type | ID_PT_BASE, priv->window));
	gg = static_cast<wxGauge *>(priv->window->FindWindowById(type | ID_GG_BASE, priv->window));

	if (cd != NULL) {
		s.Printf(wxT("%u"), max - current);
		cd->SetLabel(s);
	}
	s.Printf(wxT("%u%%"), current * 100 / max);
	pt->SetLabel(s);
	if (current == 1)
		gg->SetRange(max);
	gg->SetValue(current);
	priv->window->GetSizer()->Layout();
	wxTheApp->Yield();
}

template<class Object> static inline Object find_window(long n, wxWindow *w)
{
	return static_cast<Object>(w->FindWindowById(n, w));
}

static void listbox_fill_eds(wxListBox *lb, const void *user_ptr)
{
	const struct HXbtree_node *node;
	const struct vxeds_entry *e;
	wxString s;

	node = static_cast<const struct HXbtree_node *>(user_ptr);
	if (node == NULL)
		return;

	e = static_cast<const struct vxeds_entry *>(node->data);
	s.Printf(wxT("%s (%s)"), fV8(e->username), fV8(e->full_name));
	lb->Append(s);

	if (node->sub[0] != NULL)
		listbox_fill_eds(lb, node->sub[0]);
	if (node->sub[1] != NULL)
		listbox_fill_eds(lb, node->sub[1]);
}

static void listbox_fill_user(wxListBox *lb, const void *user_ptr)
{
	const struct HXbtree_node *node;
	const struct vxdb_user *u;
	wxString s;

	node = static_cast<const struct HXbtree_node *>(user_ptr);
	if (node == NULL)
		return;

	u = static_cast<const struct vxdb_user *>(node->data);
	s.Printf(wxT("%s (%s)"), fV8(u->pw_name), fV8(u->pw_real));
	lb->Append(s);

	if (node->sub[0] != NULL)
		listbox_fill_user(lb, node->sub[0]);
	if (node->sub[1] != NULL)
		listbox_fill_user(lb, node->sub[1]);
}

static void release_priv(struct pv_sync *priv)
{
	if (priv == NULL)
		return;

	vxdb_group_free(&priv->group, false);
	if (priv->open_status)
		vxdb_close(priv->module_handle);
	if (priv->module_handle != NULL)
		vxdb_unload(priv->module_handle);
	delete priv;
}

static int time_limit(struct timeval *last, const struct timeval *max)
{
	struct timeval now, delta;
	int update;

	gettimeofday(&now, NULL);
	tv_delta(last, &now, &delta);
	update = delta.tv_sec > max->tv_sec ||
	         (delta.tv_sec == max->tv_sec && delta.tv_usec > max->tv_usec);

	if (update)
		*last = now;
	return update;
}

static void tv_delta(const struct timeval *past, const struct timeval *now,
    struct timeval *dest)
{
	/*
	 * Calculates the time difference between @past and @now and stores
	 * the result in @dest. All parameters in µs.
	 */
	unsigned long sec = now->tv_sec - past->tv_sec;
	long acc = now->tv_usec - past->tv_usec;

	if (acc < 0) {
		dest->tv_sec = --sec;
		dest->tv_usec = 1000000 + acc;
	} else {
		dest->tv_sec = sec;
		dest->tv_usec = acc;
	}
}

static void update_max(wxWindow *w, long id)
{
	wxStaticText *pt = find_window<wxStaticText *>(id | ID_PT_BASE, w);
	wxGauge *gg      = find_window<wxGauge *>(id | ID_GG_BASE, w);

	if (pt != NULL)
		pt->SetLabel(wxT("100%"));
	if (gg != NULL)
		gg->SetValue(gg->GetRange());
}

static wxString wxString_int(unsigned int n)
{
	wxString s;
	s.Printf(wxT("%u"), n);
	return s;
}
