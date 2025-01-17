/*
    Copyright (C) 2016-2021 Carl Hetherington <cth@carlh.net>

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


#include "colours.h"
#include "content_timeline_atmos_view.h"


using std::shared_ptr;


/** @class ContentTimelineContentView
 *  @brief Content timeline view for AtmosContent.
 */

ContentTimelineAtmosView::ContentTimelineAtmosView(ContentTimeline& tl, shared_ptr<Content> c)
	: TimelineContentView (tl, c)
{

}


wxColour
ContentTimelineAtmosView::background_colour() const
{
	return ATMOS_CONTENT_COLOUR;
}


wxColour
ContentTimelineAtmosView::foreground_colour() const
{
	return wxColour (0, 0, 0, 255);
}
