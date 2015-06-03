/*
    Copyright (C) 2014-2015 Carl Hetherington <cth@carlh.net>

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

#include "audio_processor.h"

class MidSideDecoder : public AudioProcessor
{
public:
	std::string name () const;
	std::string id () const;
	ChannelCount in_channels () const;
	int out_channels () const;
	boost::shared_ptr<AudioProcessor> clone (int) const;
	boost::shared_ptr<AudioBuffers> run (boost::shared_ptr<const AudioBuffers>);
	void make_audio_mapping_default (AudioMapping& mapping) const;
	std::vector<std::string> input_names () const;
};
