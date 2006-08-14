/*=============================================================================
Vitalnix User Management Suite
steelmill/wd_config.hpp
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
#ifndef STEELMILL_WD_CONFIG_HPP
#define STEELMILL_WD_CONFIG_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include "steelmill/xu_common.hpp"

class WD_Config : public wxDialog {
  public: // functions
    WD_Config(wxWindow *);

  private: // functions
    void Load(wxCommandEvent &);
    void Save(wxCommandEvent &);
    wxPanel *nb_database(wxNotebook *);
    wxPanel *nb_external(wxNotebook *);
    wxPanel *nb_password(wxNotebook *);
    wxPanel *nb_paths(wxNotebook *);

  private: // variables
    // nb_database
    wxComboBox *db_backend, *db_encoding;
    wxCheckBox *db_upaflush;
    wxTextCtrl *ldap_basedn, *ldap_rootdn;
    GW_FTC *sh_passwd, *sh_shadow, *sh_group;

    // nb_external
    GW_FTC *ct_master_preadd, *ct_master_postadd,
           *ct_master_premod, *ct_master_postmod,
           *ct_master_predel, *ct_master_postdel,
           *ct_user_preadd, *ct_user_postadd,
           *ct_user_premod, *ct_user_postmod,
           *ct_user_predel, *ct_user_postdel;

    // nb_password
    wxChoice *pw_method;
    wxSpinCtrl *pw_length;
    wxCheckBox *pw_phonemic;

    // nb_paths
    GW_FTC *pa_home, *pa_shell, *pa_skel;
    wxSpinCtrl *pa_split;

    // general
    DECLARE_EVENT_TABLE();
};

#endif // STEELMILL_WD_CONFIG_HPP

//=============================================================================
