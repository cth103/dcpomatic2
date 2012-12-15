/*
    Copyright (C) 2012 Carl Hetherington <cth@carlh.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

/** @file  src/examine_content_job.cc
 *  @brief A class to run through content at high speed to find its length.
 */

#include <boost/filesystem.hpp>
#include "examine_content_job.h"
#include "options.h"
#include "decoder_factory.h"
#include "decoder.h"
#include "transcoder.h"
#include "log.h"
#include "film.h"
#include "video_decoder.h"

using std::string;
using std::vector;
using std::pair;
using boost::shared_ptr;

ExamineContentJob::ExamineContentJob (shared_ptr<Film> f, shared_ptr<Job> req)
	: Job (f, req)
{

}

ExamineContentJob::~ExamineContentJob ()
{
}

string
ExamineContentJob::name () const
{
	if (_film->name().empty ()) {
		return "Examine content";
	}
	
	return String::compose ("Examine content of %1", _film->name());
}

void
ExamineContentJob::run ()
{
	/* Decode the content to get an accurate length */

	/* We don't want to use any existing length here, as progress
	   will be messed up.
	*/
	_film->unset_length ();
	
	shared_ptr<Options> o (new Options ("", "", ""));
	o->out_size = Size (512, 512);
	o->apply_crop = false;
	o->decode_audio = false;

	descend (1);

	pair<shared_ptr<VideoDecoder>, shared_ptr<AudioDecoder> > decoders = decoder_factory (_film, o, this);

	set_progress_unknown ();
	while (!decoders.first->pass()) {
		/* keep going */
	}

	_film->set_length (decoders.first->video_frame());

	_film->log()->log (String::compose ("Video length is %1 frames", _film->length()));

	ascend ();
	set_progress (1);
	set_state (FINISHED_OK);
}
