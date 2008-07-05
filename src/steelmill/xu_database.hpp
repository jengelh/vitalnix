#ifndef STEELMILL_XU_DATABASE_HPP
#define STEELMILL_XU_DATABASE_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif
#include "steelmill/xu_common.hpp"

struct vxdb_state;

extern struct vxdb_state *database_open(long, wxWindow *);

#endif /* STEELMILL_XU_DATABASE_HPP */
