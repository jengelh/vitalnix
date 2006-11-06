/*=============================================================================
Vitalnix User Management Suite
steelmill/wd_about.cpp
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
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif
#include <wx/statline.h>
#include <vitalnix/config.h>
#include "steelmill/images.hpp"
#include "steelmill/wd_about.hpp"
#include "steelmill/xu_common.hpp"

//-----------------------------------------------------------------------------
static const char *const text =
    PROD_NAME "\n"
    "(Vitalnix " VITALNIX_VERSION " Steelmill)\n"
    "\n"
    "developed by Jan Engelhardt\n"
    "and Cordula Petzold\n"
    "\n"
    "\n"
    "http://vitalnix.sf.net/\n"
;

WD_About::WD_About(wxWindow *parent) :
    wxDialog(parent, wxID_ANY, wxT(PROD_NAME " > About"),
     wxDPOS, wxDSIZE, wxCFF)
{
    wxBoxSizer *hp = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *vp = new wxBoxSizer(wxVERTICAL);

    hp->Add(new wxStaticBitmap(this, wxID_ANY, *_img_steelmill_side, wxDPOS), 0, wxALIGN_CENTER);
    hp->Add(new wxStaticText(this, wxID_ANY, fU8(text), wxDPOS, wxDSIZE, wxALIGN_CENTER), 1, wxACV | wxALL, 20);
    vp->Add(hp, 1, wxGROW);
    vp->Add(smc_navgen(this, "-o"), 0, wxGROW);

    SetSizer(vp);
    vp->SetSizeHints(this);
    smc_size_aspect(this);
    return;
}

//=============================================================================
