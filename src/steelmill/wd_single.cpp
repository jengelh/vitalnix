/*=============================================================================
Vitalnix User Management Suite
steelmill/wd_single.cpp
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
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif
#include "steelmill/wd_single.hpp"
#include "steelmill/xu_common.hpp"

//-----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(WD_Single, wxDialog)
END_EVENT_TABLE()

WD_Single::WD_Single(wxWindow *parent) :
    wxDialog(parent, wxID_ANY, wxT(PROD_NAME " > Add single user"),
     wxDPOS, wxDSIZE, wxCFF)
{
    wxBoxSizer *vp = new wxBoxSizer(wxVERTICAL);

    SetSizer(vp);
    vp->SetSizeHints(this);
    Center();
    return;
}

//=============================================================================
