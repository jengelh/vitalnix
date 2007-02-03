/*
    steelmill/wd_usermod.hpp
    Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006 - 2007

    This file is part of Vitalnix. Vitalnix is free software; you can
    redistribute it and/or modify it under the terms of the GNU Lesser General
    Public License as published by the Free Software Foundation; however ONLY
    version 2 of the License. For details, see the file named "LICENSE.LGPL2".
*/
#ifndef STEELMILL_WD_USERMOD_HPP
#define STEELMILL_WD_USERMOD_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

class WD_Usermod : public wxDialog {
  public: // functions
    WD_Usermod(wxWindow *, unsigned int = 0);

  private: // variables
    DECLARE_EVENT_TABLE();
};

#endif // STEELMILL_WD_USERMOD_HPP
