/*
 *	steelmill/wd_splash.cpp
 *	Copyright © Jan Engelhardt <jengelh [at] computergmbh de>, 2006 - 2007
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
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
