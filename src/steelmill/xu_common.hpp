/*=============================================================================
Vitalnix User Management Suite
steelmill/xu_common.hpp
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
#ifndef STEELMILL_COMMON_HPP
#define STEELMILL_COMMON_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif
#include <wx/gbsizer.h>

// Definitions
#define fU8(s)          wxString((s), wxConvUTF8)
#define fV8(s)          (wxString((s), wxConvUTF8).c_str())
#define tU8(s)          static_cast<const char *>((s).mb_str(wxConvUTF8))
#define wxACV           wxALIGN_CENTER_VERTICAL
#define wxCFF           (wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | \
                        wxCLOSE_BOX | wxMAXIMIZE_BOX | wxMINIMIZE_BOX | \
                        wxFRAME_NO_TASKBAR)
#define wxDPOS          wxDefaultPosition
#define wxDSIZE         wxDefaultSize

#define PROD_NAME       "Vitalnix 2006"

enum {
    FTC_DIRECTORY = 1 << 0,
};

extern wxPanel *smc_logo_panel(wxWindow *);
extern wxBoxSizer *smc_navgen(wxWindow *, const char * = "", unsigned int = 3);
extern void smc_size_aspect(wxWindow *, double = 1.5);
extern void smc_size_minimum(wxWindow *, int, int);
extern void smc_size_minimum(wxWindow *, const wxSize &);

class GW_GroupCombo : public wxComboBox {
  public: // functions
    GW_GroupCombo(wxWindow *, wxWindowID = wxID_ANY, const char * = "*");
    void switch_database(const char *);
};

class GW_Listbox : public wxDialog {
  public: // functions
    GW_Listbox(wxWindow *, const wxString &, const struct HXdeque *, long = 0);
    GW_Listbox(wxWindow *, const wxString & = wxEmptyString,
        void (*)(wxListBox *, const void *) = NULL, const void * = NULL,
        long = 0);
        
    int Append(const wxString &s) { return ct_listbox->Append(s); };

  protected: // variables
    wxListBox *ct_listbox;
};

class GW_Message : public wxDialog {
  public: // functions
    GW_Message(wxWindow *, const wxString & = wxEmptyString,
        const wxString & = wxEmptyString, const char * = "");

  protected: // functions
    void Yes(wxCommandEvent &);
    void No(wxCommandEvent &);
    void Cancel(wxCommandEvent &);

  private: // variables
    DECLARE_EVENT_TABLE();
};

class GW_FTC : public wxPanel {
  public: // functions
    GW_FTC(wxWindow *, const wxString & = wxEmptyString, unsigned int = 0,
        unsigned int = 0);
    wxString GetValue(void) const;

  protected: // functions
    void Browse(wxCommandEvent &);

  protected: // variables
    wxTextCtrl *textfield;
    unsigned int flags;

  private: // variables
    DECLARE_EVENT_TABLE();
};

class GW_UserCombo : public wxComboBox {
  public: // functions
    GW_UserCombo(wxWindow *, wxWindowID = wxID_ANY, const char * = "*");
    void switch_database(const char *);
};

#endif // STEELMILL_COMMON_HPP

//=============================================================================
