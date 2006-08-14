/*=============================================================================
Vitalnix User Management Suite
steelmill/wd_sync.hpp
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
#ifndef STEELMILL_WD_SYNC_HPP
#define STEELMILL_WD_SYNC_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif
#include <wx/spinctrl.h>
#include "steelmill/xu_common.hpp"

struct pv_sync;

class WD_SyncParam : public wxDialog {
  public: // functions
    WD_SyncParam(wxWindow *);

  private: // functions
    void Compare(wxCommandEvent &);

  private: // variables
    struct pv_sync *priv;
    GW_FTC *ct_source;
    wxChoice *ct_srctype;
    wxComboBox *ct_module, *ct_group;
    GW_FTC *ct_output;
    wxCheckBox *ct_prompt;
    DECLARE_EVENT_TABLE();
};

#endif // STEELMILL_WD_SYNC_HPP

//=============================================================================
