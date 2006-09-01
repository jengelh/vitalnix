/*=============================================================================
Vitalnix User Management Suite
steelmill/xu_common.cpp
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2005 - 2006
  -- License restrictions apply (LGPL v2.1)

  This file is part of Vitalnix.
  Vitalnix is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; however ONLY version 2 of the License.

  Vitalnix is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program kit; if not, write to:
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA

  -- For details, see the file named "LICENSE.LGPL2"
=============================================================================*/
#include <cstdio>
#include <libHX.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif
#include <wx/statline.h>
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
BEGIN_EVENT_TABLE(GD_Listbox, wxDialog)
    // wxID_OK inherited
END_EVENT_TABLE()

GD_Listbox::GD_Listbox(wxWindow *parent, const wxString &title,
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

GD_Listbox::GD_Listbox(wxWindow *parent, const wxString &title,
 const struct HXdeque *dq, long style) :
    wxDialog(parent, wxID_ANY, title, wxDPOS, wxDSIZE, wxCFF)
{
    wxBoxSizer *vp = new wxBoxSizer(wxVERTICAL);

    vp->Add(ct_listbox = new wxListBox(this, wxID_ANY, wxDPOS, wxDSIZE, 0,
        static_cast<const wxString *>(NULL), style), 1, wxGROW | wxALL, 10);
    vp->Add(smc_navgen(this, "-o", 3), 0, wxGROW);

    if(dq != NULL)
        for(const struct HXdeque_node *node = dq->first;
         node != NULL; node = node->Next)
                ct_listbox->Append(fU8(static_cast<const char *>(node->ptr)));

    SetSizer(vp);
    vp->SetSizeHints(this);
    smc_size_minimum(this, 400, 100);
    smc_size_aspect(this);
    Center();
    return;
}

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(GD_Message, wxDialog)
    EVT_BUTTON(wxID_YES,    GD_Message::Yes)
    EVT_BUTTON(wxID_NO,     GD_Message::No)
    EVT_BUTTON(wxID_CANCEL, GD_Message::Cancel)
END_EVENT_TABLE()

GD_Message::GD_Message(wxWindow *parent, const wxString &title,
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

void GD_Message::Yes(wxCommandEvent &event) {
    EndModal(wxID_YES);
    return;
}

void GD_Message::No(wxCommandEvent &event) {
    EndModal(wxID_NO);
    return;
}

void GD_Message::Cancel(wxCommandEvent &event) {
    EndModal(wxID_CANCEL);
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

//=============================================================================
