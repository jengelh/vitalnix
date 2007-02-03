/*
    steelmill/wd_fixuuid.hpp
    Copyright © Jan Engelhardt <jengelh [at] gmx de>, 2006 - 2007

    This file is part of Vitalnix. Vitalnix is free software; you can
    redistribute it and/or modify it under the terms of the GNU Lesser General
    Public License as published by the Free Software Foundation; however ONLY
    version 2 of the License. For details, see the file named "LICENSE.LGPL2".
*/
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
    void Ok(wxCommandEvent &);

  private: // variables
    struct vxpdb_state *db_handle;
    wxComboBox *ct_username;
    wxTextCtrl *ct_realname, *ct_bday;
    DECLARE_EVENT_TABLE();
};

#endif // STEELMILL_WD_FIXUUID_HPP
