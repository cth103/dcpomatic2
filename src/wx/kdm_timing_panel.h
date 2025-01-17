/*
    Copyright (C) 2015-2016 Carl Hetherington <cth@carlh.net>

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


#include "wx_util.h"
#include <dcp/utc_offset.h>
#include <dcp/warnings.h>
LIBDCP_DISABLE_WARNINGS
#include <wx/wx.h>
LIBDCP_ENABLE_WARNINGS
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/signals2.hpp>


class Choice;
class TimePicker;
class wxDatePickerCtrl;


class KDMTimingPanel : public wxPanel
{
public:
	explicit KDMTimingPanel (wxWindow* parent);

	dcp::LocalTime from() const;
	dcp::LocalTime until() const;

	bool valid () const;

	/** Give a UTC offset from a cinema that the user just selected.  If the user
	 *  never changed the UTC offset in the panel, the suggested UTC will be set.
	 */
	void suggest_utc_offset(dcp::UTCOffset offset);

	boost::signals2::signal<void ()> TimingChanged;

private:
	void changed () const;
	void utc_offset_changed();
	dcp::UTCOffset utc_offset() const;

	static dcp::LocalTime local_time(wxDatePickerCtrl *, TimePicker *, dcp::UTCOffset offset);

	wxDatePickerCtrl* _from_date;
	wxDatePickerCtrl* _until_date;
	TimePicker* _from_time;
	TimePicker* _until_time;
	Choice* _utc_offset;
	bool _utc_offset_changed_once = false;
	wxStaticText* _warning;
	std::vector<Offset> _offsets;
};
