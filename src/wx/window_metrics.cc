/*
    Copyright (C) 2024 Carl Hetherington <cth@carlh.net>

    This file is part of DCP-o-matic.

    DCP-o-matic is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    DCP-o-matic is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DCP-o-matic.  If not, see <http://www.gnu.org/licenses/>.

*/


#include "window_metrics.h"
#include <dcp/warnings.h>
LIBDCP_DISABLE_WARNINGS
#include <wx/app.h>
#include <wx/wx.h>
LIBDCP_ENABLE_WARNINGS


void
WindowMetrics::show(wxWindow* window) const
{
#ifdef DCPOMATIC_LINUX
	auto const position_before = position;
#endif

	window->Show();

#ifdef DCPOMATIC_LINUX
	wxTheApp->CallAfter([window, position_before] {
		window->SetPosition(position_before);
	});
#endif
}
