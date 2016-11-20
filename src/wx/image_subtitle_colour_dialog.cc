/*
    Copyright (C) 2016 Carl Hetherington <cth@carlh.net>

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

#include "image_subtitle_colour_dialog.h"
#include "rgba_colour_picker.h"
#include "wx_util.h"
#include "lib/ffmpeg_subtitle_stream.h"
#include "lib/ffmpeg_content.h"

using std::map;
using std::cout;
using boost::shared_ptr;

ImageSubtitleColourDialog::ImageSubtitleColourDialog (wxWindow* parent, shared_ptr<FFmpegContent> content, shared_ptr<FFmpegSubtitleStream> stream)
	: wxDialog (parent, wxID_ANY, _("Subtitle colours"))
	, _content (content)
	, _stream (stream)
{
	wxBoxSizer* overall_sizer = new wxBoxSizer (wxVERTICAL);
	SetSizer (overall_sizer);

	wxScrolled<wxPanel>* colours_panel = new wxScrolled<wxPanel> (this);
	colours_panel->EnableScrolling (false, true);
	colours_panel->ShowScrollbars (wxSHOW_SB_NEVER, wxSHOW_SB_ALWAYS);
	colours_panel->SetScrollRate (0, 16);

	wxFlexGridSizer* table = new wxFlexGridSizer (2, DCPOMATIC_SIZER_X_GAP, DCPOMATIC_SIZER_Y_GAP);
	table->AddGrowableCol (1, 1);

	map<RGBA, RGBA> colours = _stream->colours ();

	wxStaticText* t = new wxStaticText (colours_panel, wxID_ANY, "");
	t->SetLabelMarkup (_("<b>Original colour</b>"));
	table->Add (t, 1, wxEXPAND);
	t = new wxStaticText (colours_panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
	t->SetLabelMarkup (_("<b>New colour</b>"));
	table->Add (t, 1, wxALIGN_CENTER);

	for (map<RGBA, RGBA>::const_iterator i = colours.begin(); i != colours.end(); ++i) {
		wxPanel* from = new wxPanel (colours_panel, wxID_ANY);
		from->SetBackgroundColour (wxColour (i->first.r, i->first.g, i->first.b, i->first.a));
		table->Add (from, 1, wxEXPAND);
		RGBAColourPicker* to = new RGBAColourPicker (colours_panel, i->second);
		table->Add (to, 1, wxEXPAND);
		_pickers[i->first] = to;
	}

	colours_panel->SetSizer (table);

	overall_sizer->Add (colours_panel, 1, wxEXPAND | wxALL, DCPOMATIC_DIALOG_BORDER);

	wxSizer* buttons = CreateSeparatedButtonSizer (wxOK | wxCANCEL);
	if (buttons) {
		overall_sizer->Add (buttons, wxSizerFlags().Expand().DoubleBorder());
	}
}

void
ImageSubtitleColourDialog::apply ()
{
	for (map<RGBA, RGBAColourPicker*>::const_iterator i = _pickers.begin(); i != _pickers.end(); ++i) {
		_stream->set_colour (i->first, i->second->colour ());
	}

	_content->signal_subtitle_stream_changed ();
}
