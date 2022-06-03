/*
    Copyright (C) 2022 Carl Hetherington <cth@carlh.net>

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


#include "config.h"
#include "export_config.h"
#include <dcp/raw_convert.h>
#include <dcp/warnings.h>
LIBDCP_DISABLE_WARNINGS
#include <libxml++/libxml++.h>
LIBDCP_ENABLE_WARNINGS


using std::string;


ExportConfig::ExportConfig(Config* parent)
	: _config(parent)
{

}


void
ExportConfig::set_defaults()
{
	_format = ExportFormat::PRORES;
	_mixdown_to_stereo = false;
	_split_reels = false;
	_split_streams = false;
	_x264_crf = 23;
}


void
ExportConfig::read(cxml::ConstNodePtr node)
{
	if (!node) {
		set_defaults();
		return;
	}

	auto const format = node->string_child("Format");

	if (format == "subtitles-dcp") {
		_format = ExportFormat::SUBTITLES_DCP;
	} else if (format == "h264-aac") {
		_format = ExportFormat::H264_AAC;
	} else {
		_format = ExportFormat::PRORES;
	}

	_mixdown_to_stereo = node->bool_child("MixdownToStereo");
	_split_reels = node->bool_child("SplitReels");
	_split_streams = node->bool_child("SplitStreams");
	_x264_crf = node->number_child<int>("X264CRF");
}


void
ExportConfig::write(xmlpp::Element* node) const
{
	string name;

	switch (_format) {
		case ExportFormat::PRORES:
			name = "prores";
			break;
		case ExportFormat::H264_AAC:
			name = "h264-aac";
			break;
		case ExportFormat::SUBTITLES_DCP:
			name = "subtitles-dcp";
			break;
	}

	node->add_child("Format")->add_child_text(name);
	node->add_child("MixdownToStereo")->add_child_text(_mixdown_to_stereo ? "1" : "0");
	node->add_child("SplitReels")->add_child_text(_split_reels ? "1" : "0");
	node->add_child("SplitStreams")->add_child_text(_split_streams ? "1" : "0");
	node->add_child("X264CRF")->add_child_text(dcp::raw_convert<string>(_x264_crf));
}


void
ExportConfig::set_format(ExportFormat format)
{
	_config->maybe_set(_format, format);
}


void
ExportConfig::set_mixdown_to_stereo(bool mixdown)
{
	_config->maybe_set(_mixdown_to_stereo, mixdown);
}


void
ExportConfig::set_split_reels(bool split)
{
	_config->maybe_set(_split_reels, split);
}


void
ExportConfig::set_split_streams(bool split)
{
	_config->maybe_set(_split_streams, split);
}


void
ExportConfig::set_x264_crf(int crf)
{
	_config->maybe_set(_x264_crf, crf);
}

