/*=============================================================================
Vitalnix User Management Suite
steelmill/wd_fixuuid.hpp
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006
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
#ifndef STEELMILL_WD_FIXUUID_HPP
#define STEELMILL_WD_FIXUUID_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif
#include <wx/spinctrl.h>
#include "steelmill/xu_common.hpp"

struct vxpdb_state;

class WD_FixUUID : public wxDialog {
  public: // functions
    WD_FixUUID(wxWindow *);

  private: // functions
    void Fill_Defaults(wxCommandEvent &);
    void Fill_Users(wxComboBox *);
    void Ok(wxCommandEvent &);

  private: // variables
    struct vxpdb_state *db_handle;
    wxComboBox *ct_username;
    wxTextCtrl *ct_realname, *ct_bday;
    DECLARE_EVENT_TABLE();
};

#endif // STEELMILL_WD_FIXUUID_HPP

//=============================================================================