/*
 *	steelmill/xu_database.hpp
 *	Copyright © CC Computer Consultants GmbH, 2006 - 2007
 *	Contact: Jan Engelhardt <jengelh [at] computergmbh de>
 *
 *	This file is part of Vitalnix. Vitalnix is free software; you
 *	can redistribute it and/or modify it under the terms of the GNU
 *	Lesser General Public License as published by the Free Software
 *	Foundation; either version 2.1 or 3 of the License.
 */
#ifndef STEELMILL_XU_DATABASE_HPP
#define STEELMILL_XU_DATABASE_HPP 1

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#	include <wx/wx.h>
#endif
#include "steelmill/xu_common.hpp"

struct vxpdb_state;

extern struct vxpdb_state *database_open(long, wxWindow *);

#endif /* STEELMILL_XU_DATABASE_HPP */
