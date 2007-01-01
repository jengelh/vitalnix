/*=============================================================================
Vitalnix User Management Suite
steelmill/wd_splash.cpp
  Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006 - 2007
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
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif
#include <steelmill/images.hpp>
#include "steelmill/wd_splash.hpp"
#include "steelmill/xu_common.hpp"

//-----------------------------------------------------------------------------
WD_Splash::WD_Splash(const wxPoint &pos) :
    wxDialog(NULL, wxID_ANY, wxEmptyString, pos, wxDSIZE,
     wxFRAME_TOOL_WINDOW | wxSTAY_ON_TOP)
{
    wxBoxSizer *vp       = new wxBoxSizer(wxVERTICAL);
    vp->Add(new wxStaticBitmap(this, wxID_ANY, *_img_steelmill_splash, wxDPOS));
    SetSizer(vp);
    vp->SetSizeHints(this);
    Center();
    return;
}

//=============================================================================
