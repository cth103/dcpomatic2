/*
    Copyright (C) 2013-2015 Carl Hetherington <cth@carlh.net>

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

/** @file  test/seek_zero_test.cc
 *  @brief Test seek to zero with a raw FFmpegDecoder (without the player
 *  confusing things as it might in ffmpeg_seek_test).
 */

#include <boost/test/unit_test.hpp>
#include "lib/film.h"
#include "lib/ffmpeg_content.h"
#include "lib/ratio.h"
#include "lib/dcp_content_type.h"
#include "lib/ffmpeg_decoder.h"
#include "lib/ffmpeg_audio_stream.h"
#include "lib/content_video.h"
#include "lib/video_content_scale.h"
#include "lib/video_content.h"
#include "lib/video_decoder.h"
#include "test.h"
#include <iostream>

using std::cout;
using std::list;
using boost::shared_ptr;
using boost::dynamic_pointer_cast;
using boost::optional;

BOOST_AUTO_TEST_CASE (seek_zero_test)
{
	shared_ptr<Film> film = new_test_film ("seek_zero_test");
	film->set_name ("seek_zero_test");
	film->set_container (Ratio::from_id ("185"));
	film->set_dcp_content_type (DCPContentType::from_isdcf_name ("TST"));
	shared_ptr<FFmpegContent> content (new FFmpegContent (film, "test/data/count300bd48.m2ts"));
	film->examine_and_add_content (content);
	wait_for_jobs ();
	content->video->set_scale (VideoContentScale (Ratio::from_id ("185")));

	/* Work out the first video frame index that we will be given, taking into account
	 * the difference between first video and first audio.
	 */
	ContentTime video_delay = content->first_video().get() - content->ffmpeg_audio_streams().front()->first_audio.get();
	if (video_delay < ContentTime ()) {
		video_delay = ContentTime ();
	}

	Frame const first_frame = video_delay.ceil (content->active_video_frame_rate ()).frames_round (content->active_video_frame_rate ());

	FFmpegDecoder decoder (content, film->log());
	list<ContentVideo> a = decoder.video->get (first_frame, true);
	BOOST_CHECK (a.size() == 1);
	BOOST_CHECK_EQUAL (a.front().frame.index(), first_frame);
}
