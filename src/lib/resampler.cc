/*
    Copyright (C) 2013-2015 Carl Hetherington <cth@carlh.net>

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

#include "resampler.h"
#include "audio_buffers.h"
#include "exceptions.h"
#include "compose.hpp"
#include "dcpomatic_assert.h"
#include <samplerate.h>

#include "i18n.h"

using std::cout;
using std::pair;
using std::make_pair;
using boost::shared_ptr;

Resampler::Resampler (int in, int out, int channels)
	: _in_rate (in)
	, _out_rate (out)
	, _channels (channels)
{
	int error;
	_src = src_new (SRC_SINC_BEST_QUALITY, _channels, &error);
	if (!_src) {
		throw StringError (String::compose (N_("could not create sample-rate converter (%1)"), error));
	}
}

Resampler::~Resampler ()
{
	src_delete (_src);
}

shared_ptr<const AudioBuffers>
Resampler::run (shared_ptr<const AudioBuffers> in)
{
	int in_frames = in->frames ();
	int in_offset = 0;
	int out_offset = 0;
	shared_ptr<AudioBuffers> resampled (new AudioBuffers (_channels, 0));

	while (in_frames > 0) {

		/* Compute the resampled frames count and add 32 for luck */
		int const max_resampled_frames = ceil ((double) in_frames * _out_rate / _in_rate) + 32;

		SRC_DATA data;
		data.data_in = new float[in_frames * _channels];

		{
			float** p = in->data ();
			float* q = data.data_in;
			for (int i = 0; i < in_frames; ++i) {
				for (int j = 0; j < _channels; ++j) {
					*q++ = p[j][in_offset + i];
				}
			}
		}

		data.input_frames = in_frames;

		data.data_out = new float[max_resampled_frames * _channels];
		data.output_frames = max_resampled_frames;

		data.end_of_input = 0;
		data.src_ratio = double (_out_rate) / _in_rate;

		int const r = src_process (_src, &data);
		if (r) {
			delete[] data.data_in;
			delete[] data.data_out;
			throw EncodeError (String::compose (N_("could not run sample-rate converter (%1)"), src_strerror (r)));
		}

		if (data.output_frames_gen == 0) {
			break;
		}

		resampled->ensure_size (out_offset + data.output_frames_gen);
		resampled->set_frames (out_offset + data.output_frames_gen);

		{
			float* p = data.data_out;
			float** q = resampled->data ();
			for (int i = 0; i < data.output_frames_gen; ++i) {
				for (int j = 0; j < _channels; ++j) {
					q[j][out_offset + i] = *p++;
				}
			}
		}

		in_frames -= data.input_frames_used;
		in_offset += data.input_frames_used;
		out_offset += data.output_frames_gen;

		delete[] data.data_in;
		delete[] data.data_out;
	}

	return resampled;
}

shared_ptr<const AudioBuffers>
Resampler::flush ()
{
	shared_ptr<AudioBuffers> out (new AudioBuffers (_channels, 0));
	int out_offset = 0;
	int64_t const output_size = 65536;

	float dummy[1];
	float buffer[output_size];

	SRC_DATA data;
	data.data_in = dummy;
	data.input_frames = 0;
	data.data_out = buffer;
	data.output_frames = output_size;
	data.end_of_input = 1;
	data.src_ratio = double (_out_rate) / _in_rate;

	int const r = src_process (_src, &data);
	if (r) {
		throw EncodeError (String::compose (N_("could not run sample-rate converter (%1)"), src_strerror (r)));
	}

	out->ensure_size (out_offset + data.output_frames_gen);

	float* p = data.data_out;
	float** q = out->data ();
	for (int i = 0; i < data.output_frames_gen; ++i) {
		for (int j = 0; j < _channels; ++j) {
			q[j][out_offset + i] = *p++;
		}
	}

	out_offset += data.output_frames_gen;
	out->set_frames (out_offset);

	return out;
}
