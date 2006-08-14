/*=============================================================================
Vitalnix User Management Suite
steelmill/wd_overview.hpp
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
#ifndef STEELMILL_WD_OVERVIEW_HPP
#define STEELMILL_WD_OVERVIEW_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

enum {
    OVERVIEW_USERS = 1,
    OVERVIEW_GROUPS,
};

class WD_Overview : public wxDialog {
  public: // functions
    WD_Overview(wxWindow *, unsigned int);

  private: // functions
    wxListCtrl *init_list(unsigned int);

  private: // variables
    DECLARE_EVENT_TABLE();
};

#endif // STEELMILL_WD_OVERVIEW_HPP

//=============================================================================