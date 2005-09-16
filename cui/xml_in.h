/*=============================================================================
Vitalnix User Management Suite
  Copyright © Jan Engelhardt <jengelh [at] linux01 gwdg de>, 2003 - 2005
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
#ifndef CSPARK_XML_IN_H
#define CSPARK_XML_IN_H 1

#ifdef __cplusplus
extern "C" {
#endif

enum {
    EDSTYPE_NONE = 0,
    EDSTYPE_SDF,
    EDSTYPE_XML,
    EDSTYPE_MAX,
};

struct eds_entry {
    char *lname,
     *vname,
     *nname,
     *rname,  // real name (vname + nname)
     *sgroup, // subgroup (past name: .sclass)
     *xuid;   // any text string to uniquely identify that user
};

extern int eds_open(const char *, void **, int);
extern int eds_read(void *, struct eds_entry *);
extern void eds_close(void *);
extern int eds_derivefromname(const char *);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CSPARK_XML_IN_H

//=============================================================================
