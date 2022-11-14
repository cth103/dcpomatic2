/*
    Copyright (C) 2012-2016 Carl Hetherington <cth@carlh.net>

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


/** @file  src/ffmpeg_decoder.h
 *  @brief A decoder using FFmpeg to decode content.
 */


#include "bitmap_text.h"
#include "decoder.h"
#include "ffmpeg.h"
#include "video_filter_graph_set.h"
#include "util.h"
extern "C" {
#include <libavcodec/avcodec.h>
}
#include <boost/thread/mutex.hpp>
#include <stdint.h>


class AudioBuffers;
class FFmpegAudioStream;
class Image;
class Log;
class VideoFilterGraph;
struct ffmpeg_pts_offset_test;


/** @class FFmpegDecoder
 *  @brief A decoder using FFmpeg to decode content.
 */
class FFmpegDecoder : public FFmpeg, public Decoder
{
public:
	FFmpegDecoder (std::shared_ptr<const Film> film, std::shared_ptr<const FFmpegContent>, bool fast);

	bool pass () override;
	void seek (dcpomatic::ContentTime time, bool) override;

private:
	friend struct ::ffmpeg_pts_offset_test;

	enum class FlushResult {
		DONE,
		AGAIN
	};

	FlushResult flush();

	AVSampleFormat audio_sample_format (std::shared_ptr<FFmpegAudioStream> stream) const;
	int bytes_per_audio_sample (std::shared_ptr<FFmpegAudioStream> stream) const;

	std::shared_ptr<FFmpegAudioStream> audio_stream_from_index (int index) const;
	void process_audio_frame (std::shared_ptr<FFmpegAudioStream> stream);

	void process_video_frame ();

	bool decode_and_process_video_packet (AVPacket* packet);
	void decode_and_process_audio_packet (AVPacket* packet);
	void decode_and_process_subtitle_packet (AVPacket* packet);

	BitmapText process_bitmap_subtitle (AVSubtitleRect const * rect);
	void process_ass_subtitle (std::string ass, dcpomatic::ContentTime from);

	void maybe_add_subtitle ();

	FlushResult flush_codecs();
	FlushResult flush_fill();

	VideoFilterGraphSet _filter_graphs;

	dcpomatic::ContentTime _pts_offset;
	boost::optional<dcpomatic::ContentTime> _current_subtitle_to;
	/** true if we have a subtitle which has not had emit_stop called for it yet */
	bool _have_current_subtitle = false;

	std::shared_ptr<Image> _black_image;

	std::map<std::shared_ptr<FFmpegAudioStream>, boost::optional<dcpomatic::ContentTime>> _next_time;

	enum class FlushState {
		CODECS,
		AUDIO_DECODER,
		FILL,
	};

	FlushState _flush_state = FlushState::CODECS;
};
