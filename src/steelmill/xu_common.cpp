/*
 *	steelmill/xu_common.cpp
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2005 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#include <cstdio>
#include <libHX.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif
#include <wx/statline.h>
#include <vitalnix/libvxpdb/libvxpdb.h>
#include <vitalnix/libvxpdb/xafunc.h>
#include "steelmill/images.hpp"
#include "steelmill/xu_common.hpp"
#define MAX_BUTTONS 8

//-----------------------------------------------------------------------------
wxPanel *smc_logo_panel(wxWindow *parent) {
    wxPanel *panel = new wxPanel(parent);
    wxBoxSizer *vp = new wxBoxSizer(wxVERTICAL);
    panel->SetBackgroundColour(*wxWHITE);
    panel->SetSizer(vp);
    vp->Add(new wxStaticBitmap(panel, wxID_ANY, *_img_vitalnix_splash), 0, wxALIGN_CENTER | wxALL, 5);
    vp->Add(new wxStaticLine(panel, wxID_ANY), 0, wxGROW);
    return panel;
}

wxBoxSizer *smc_navgen(wxWindow *parent, const char *flags,
  unsigned int border)
{
    wxBoxSizer *hp = new wxBoxSizer(wxHORIZONTAL),
               *vp = new wxBoxSizer(wxVERTICAL);
    wxButton *bt[MAX_BUTTONS];
    wxWindowID id;
    wxString text;
    int cnt = 0;

    memset(bt, 0, sizeof(bt));
    vp->Add(new wxStaticLine(parent, wxID_ANY), 0, wxGROW);

    while(*flags != '\0' && cnt < MAX_BUTTONS) {
        switch(*flags++) {
            case 'b': id = wxID_BACKWARD; text = wxT("<< &Back");  break;
            case 'c': id = wxID_CANCEL;   text = wxT("&Cancel");   break;
            case 'e': id = wxID_EXIT;     text = wxT("&Exit");     break;
            case 'h': id = wxID_HELP;     text = wxT("&Help");     break;
            case 'n': id = wxID_FORWARD;  text = wxT("&Next >>");  break;
            case 'o': id = wxID_OK;       text = wxT("&OK");       break;
            case '-':
                hp->Add(-1, -1, 1);
                continue;
            default:
                fprintf(stderr, "Unknown button flag '%c'\n", *(flags-1));
                continue;
        }
        hp->Add(bt[cnt++] = new wxButton(parent, id, text, wxDPOS, wxDSIZE,
                            0 /* or wxBU_EXACTFIT */), 0, wxALL, border);
    }

    // Make all buttons the same size
    wxSize sz = bt[0]->GetSize();
    for(int i = 1; i < cnt; ++i)
        sz.IncTo(bt[i]->GetSize());
    sz.IncTo(wxSize(3 * sz.GetHeight(), -1));

    // Fit it
    int x = sz.GetWidth(), y = sz.GetHeight();
    for(int i = 0; i < cnt; ++i)
        hp->SetItemMinSize(bt[i], x, y);

    vp->Add(hp, 0, wxGROW);
    return vp;
}

void smc_size_aspect(wxWindow *w, double f) {
    wxSize s = w->GetSize();
    s.IncTo(wxSize((int)(f * s.GetHeight()), -1));
    w->SetSize(s);
    return;
}

void smc_size_minimum(wxWindow *w, int x, int y) {
    wxSize s = w->GetSize();
    s.IncTo(wxSize(x, y));
    w->SetSize(s);
    return;
}

void smc_size_minimum(wxWindow *w, const wxSize &ps) {
    wxSize ns = w->GetSize();
    ns.IncTo(ps);
    w->SetSize(ns);
    return;
}

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(GW_FTC, wxPanel)
    EVT_BUTTON(wxID_OPEN, GW_FTC::Browse)
END_EVENT_TABLE()

GW_FTC::GW_FTC(wxWindow *parent, const wxString &string, unsigned int border,
 unsigned int flags) :
    wxPanel(parent)
{
    wxBoxSizer *vp = new wxBoxSizer(wxHORIZONTAL);
    textfield      = new wxTextCtrl(this, wxID_ANY, string);

    vp->Add(textfield, 1, wxACV | wxALL, border);
    vp->Add(new wxButton(this, wxID_OPEN, wxT("Browse...")), 0, wxACV | wxALL, border);
    SetSizer(vp);
    vp->SetSizeHints(this);
    this->flags = flags;
    return;
}

wxString GW_FTC::GetValue(void) const {
    return textfield->GetValue();
}

void GW_FTC::Browse(wxCommandEvent &event) {
    if(flags & FTC_DIRECTORY) {
        wxDirDialog dlg(this, wxT("Browse"), wxEmptyString, wxDD_NEW_DIR_BUTTON);
        if(dlg.ShowModal() != wxID_OK)
            return;
        textfield->SetValue(dlg.GetPath());
    } else {
        wxFileDialog dlg(this, wxT("Browse"), wxEmptyString, wxEmptyString,
            wxT("All Files (*.*)|*"), wxOPEN | wxFILE_MUST_EXIST);
        if(dlg.ShowModal() != wxID_OK)
            return;
        textfield->SetValue(dlg.GetPath());
    }
    return;
}

//-----------------------------------------------------------------------------
GW_GroupCombo::GW_GroupCombo(wxWindow *parent, wxWindowID id,
 const char *db) :
    wxComboBox(parent, id, wxEmptyString)
{
    switch_database(db);
    return;
}

void GW_GroupCombo::switch_database(const char *db_name) {
    struct vxpdb_group group = {};
    struct vxpdb_state *db;
    void *trav;
    int ret;

    Clear();

    if((db = vxpdb_load(db_name)) == NULL)
        return;

    if((ret = vxpdb_open(db, 0)) <= 0) {
        fprintf(stderr, "%s: vxpdb_open: %s\n", __PRETTY_FUNCTION__,
                strerror(ret));
        goto out_unload;
    }

    if((trav = vxpdb_grouptrav_init(db)) == NULL)
        goto out_close;
    while(vxpdb_grouptrav_walk(db, trav, &group) > 0)
        if(group.gr_gid > 0)
            Append(fU8(group.gr_name));

    vxpdb_grouptrav_free(db, trav);
    vxpdb_group_free(&group, 0);
 out_close:
    vxpdb_close(db);
 out_unload:
    vxpdb_unload(db);
    return;
}

//-----------------------------------------------------------------------------
GW_Listbox::GW_Listbox(wxWindow *parent, const wxString &title,
 void (*callback)(wxListBox *, const void *), const void *uptr, long style) :
    wxDialog(parent, wxID_ANY, title, wxDPOS, wxDSIZE, wxCFF)
{
    wxBoxSizer *vp = new wxBoxSizer(wxVERTICAL);
    vp->Add(ct_listbox = new wxListBox(this, wxID_ANY, wxDPOS, wxDSIZE, 0,
        static_cast<const wxString *>(NULL), style), 1, wxGROW | wxALL, 10);
    vp->Add(smc_navgen(this, "-o", 3), 0, wxGROW);
    callback(ct_listbox, uptr);

    SetSizer(vp);
    vp->SetSizeHints(this);
    smc_size_minimum(this, 400, 100);
    smc_size_aspect(this);
    Center();
    return;
}

GW_Listbox::GW_Listbox(wxWindow *parent, const wxString &title,
 const struct HXdeque *dq, long style) :
    wxDialog(parent, wxID_ANY, title, wxDPOS, wxDSIZE, wxCFF)
{
    wxBoxSizer *vp = new wxBoxSizer(wxVERTICAL);

    vp->Add(ct_listbox = new wxListBox(this, wxID_ANY, wxDPOS, wxDSIZE, 0,
        static_cast<const wxString *>(NULL), style), 1, wxGROW | wxALL, 10);
    vp->Add(smc_navgen(this, "-o", 3), 0, wxGROW);

    if(dq != NULL)
        for(const struct HXdeque_node *node = dq->first;
         node != NULL; node = node->next)
                ct_listbox->Append(fU8(static_cast<const char *>(node->ptr)));

    SetSizer(vp);
    vp->SetSizeHints(this);
    smc_size_minimum(this, 400, 100);
    smc_size_aspect(this);
    Center();
    return;
}

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(GW_Message, wxDialog)
    EVT_BUTTON(wxID_YES,    GW_Message::Yes)
    EVT_BUTTON(wxID_NO,     GW_Message::No)
    EVT_BUTTON(wxID_CANCEL, GW_Message::Cancel)
END_EVENT_TABLE()

GW_Message::GW_Message(wxWindow *parent, const wxString &title,
 const wxString &prompt, const char *button_mask) :
    wxDialog(parent, wxID_ANY, title, wxDPOS, wxDSIZE, wxCFF)
{
    wxBoxSizer *vp = new wxBoxSizer(wxVERTICAL);

    vp->Add(new wxStaticText(this, wxID_ANY, prompt), 0, wxALL | wxGROW, 10);
    if(*button_mask != '\0') {
        vp->Add(-1, 1, 1);
        vp->Add(smc_navgen(this, button_mask), 0, wxGROW);
    }

    SetSizer(vp);
    vp->SetSizeHints(this);
    smc_size_aspect(this);
    Center();
    return;
}

void GW_Message::Yes(wxCommandEvent &event) {
    EndModal(wxID_YES);
    return;
}

void GW_Message::No(wxCommandEvent &event) {
    EndModal(wxID_NO);
    return;
}

void GW_Message::Cancel(wxCommandEvent &event) {
    EndModal(wxID_CANCEL);
    return;
}

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(GW_Table, wxListCtrl)
    EVT_SIZE(GW_Table::Resize_Table)
    EVT_LIST_COL_END_DRAG(wxID_ANY, GW_Table::Resize_Column)
END_EVENT_TABLE()

/*  GW_Table
    @parent:    widget parent
    @id:        identifier
    @names:     column names
    @columns:   number of elements in @col_names
    @layout:    array of column proportions
    @flags:     bitmask of %wxLC_SINGLE_SEL and %wxLC_SORT_ASCENDING

    A simple table with automatic column resizing.
*/
GW_Table::GW_Table(wxWindow *parent, wxWindowID id, const wxString *names,
 int columns, const int *layout, long flags) :
    wxListCtrl(parent, id, wxDPOS, wxDSIZE, wxLC_REPORT |
     wxLC_HRULES | wxLC_VRULES | flags)
{
    wxListItem item;
    int i = 0;

    column_layout     = new int[columns];
    column_layout_sum = 0;
    num_columns       = columns;
    memcpy(column_layout, layout, sizeof(int) * columns);

    while(columns--) {
        column_layout_sum += *layout++;
        item.SetText(*names++);
        InsertColumn(i++, item);
    }
    return;
}

GW_Table::~GW_Table(void)
{
    delete[] column_layout;
    return;
}

/*  GW_Table::insert
    @c: array of strings to insert as a new row

    Insert a row into the table.
*/
void GW_Table::Insert(const wxString *c)
{
    int i = 0, n = num_columns;
    wxListItem item;

    while(i--) {
        item.SetText(*c++);
        InsertColumn(n++, item);
    }
    return;
}

void GW_Table::Resize_Column(wxListEvent &event)
{
    printf("column resized\n");
    event.Skip();
    return;
}

void GW_Table::Resize_Table(wxSizeEvent &event)
{
    wxSize size = event.GetSize();
    int i = 0, n = num_columns;

    while(n--) {
        SetColumnWidth(i, size.GetWidth() * column_layout[i] /
                       column_layout_sum);
        ++i;
    }

    event.Skip();
    return;
}

//-----------------------------------------------------------------------------
GW_UserCombo::GW_UserCombo(wxWindow *parent, wxWindowID id, const char *db) :
    wxComboBox(parent, id, wxEmptyString, wxDPOS, wxDSIZE, 0, NULL, wxCB_SORT)
{
    switch_database(db);
    return;
}

void GW_UserCombo::switch_database(const char *db_name) {
    struct vxpdb_user user = {};
    struct vxpdb_state *db;
    void *trav;
    int ret;

    Clear();

    if((db = vxpdb_load(db_name)) == NULL)
        return;

    if((ret = vxpdb_open(db, 0)) <= 0) {
        fprintf(stderr, "%s: vxpdb_open: %s\n", __PRETTY_FUNCTION__,
                strerror(ret));
        goto out_unload;
    }

    if((trav = vxpdb_usertrav_init(db)) == NULL)
        goto out_close;
    while(vxpdb_usertrav_walk(db, trav, &user) > 0)
        Append(fU8(user.pw_name));

    vxpdb_usertrav_free(db, trav);
    vxpdb_user_free(&user, 0);
 out_close:
    vxpdb_close(db);
 out_unload:
    vxpdb_unload(db);
    return;
}

//=============================================================================
