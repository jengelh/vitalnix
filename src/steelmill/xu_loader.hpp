/*=============================================================================
Vitalnix User Management Suite
steelmill/loader.hpp
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
#ifndef STEELMILL_XU_LOADER_HPP
#define STEELMILL_XU_LOADER_HPP 1

#include <sys/types.h>
#include <cstdio>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#    include <wx/wx.h>
#endif

class WD_Console : public wxFrame {
  public: // functions
    WD_Console(void);
    ~WD_Console(void);
    static ssize_t write_cb(void *, const char *, size_t);

  private: // functions
    void Append(const char *);
    void Clear(wxCommandEvent &);
    void hide_command(wxCommandEvent &);
    void hide_close(wxCloseEvent &);

  private: // variables
    FILE *saved_stdout, *saved_stderr, *current;
    wxTextCtrl *tc;
    DECLARE_EVENT_TABLE();
};

class Loader : public wxApp {
  public: // functions
    bool OnInit(void);

  public: // variables
    WD_Console *console;

  private: // functions
    void splash_tick(wxTimerEvent &);

  private: // variables
    unsigned int splash_done;
    DECLARE_EVENT_TABLE();
};

#endif // STEELMILL_XU_LOADER_HPP

//=============================================================================
