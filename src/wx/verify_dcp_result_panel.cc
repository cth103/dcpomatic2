/*
    Copyright (C) 2018 Carl Hetherington <cth@carlh.net>

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


#include "dcpomatic_button.h"
#include "file_dialog.h"
#include "verify_dcp_result_panel.h"
#include "wx_util.h"
#include "lib/verify_dcp_job.h"
#include <dcp/raw_convert.h>
#include <dcp/verify.h>
#include <dcp/verify_report.h>
#include <dcp/warnings.h>
LIBDCP_DISABLE_WARNINGS
#include <wx/richtext/richtextctrl.h>
#include <wx/notebook.h>
LIBDCP_ENABLE_WARNINGS
#include <boost/algorithm/string.hpp>


using std::list;
using std::map;
using std::shared_ptr;
using std::string;
using std::vector;


VerifyDCPResultPanel::VerifyDCPResultPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY)
{
	auto sizer = new wxBoxSizer(wxVERTICAL);
	auto notebook = new wxNotebook(this, wxID_ANY);
	sizer->Add(notebook, 1, wxEXPAND | wxALL, DCPOMATIC_DIALOG_BORDER);

	_pages[dcp::VerificationNote::Type::ERROR] = new wxRichTextCtrl(notebook, wxID_ANY, wxEmptyString, wxDefaultPosition, {400, 300}, wxRE_READONLY);
	notebook->AddPage(_pages[dcp::VerificationNote::Type::ERROR], _("Errors"));
	_pages[dcp::VerificationNote::Type::BV21_ERROR] = new wxRichTextCtrl(notebook, wxID_ANY, wxEmptyString, wxDefaultPosition, {400, 300}, wxRE_READONLY);
	notebook->AddPage(_pages[dcp::VerificationNote::Type::BV21_ERROR], _("SMPTE Bv2.1 errors"));
	_pages[dcp::VerificationNote::Type::WARNING] = new wxRichTextCtrl(notebook, wxID_ANY, wxEmptyString, wxDefaultPosition, {400, 300}, wxRE_READONLY);
	notebook->AddPage(_pages[dcp::VerificationNote::Type::WARNING], _("Warnings"));

	_summary = new wxStaticText(this, wxID_ANY, wxT(""));
	sizer->Add(_summary, 0, wxALL, DCPOMATIC_DIALOG_BORDER);

	auto save_sizer = new wxBoxSizer(wxHORIZONTAL);
	_save_text_report = new Button(this, _("Save report as text..."));
	save_sizer->Add(_save_text_report, 0, wxALL, DCPOMATIC_SIZER_GAP);
	_save_html_report = new Button(this, _("Save report as HTML..."));
	save_sizer->Add(_save_html_report, 0, wxALL, DCPOMATIC_SIZER_GAP);
	sizer->Add(save_sizer);

	SetSizer(sizer);
	sizer->Layout();
	sizer->SetSizeHints(this);

	for (auto const& i: _pages) {
		i.second->GetCaret()->Hide();
	}

	_save_text_report->bind(&VerifyDCPResultPanel::save_text_report, this);
	_save_html_report->bind(&VerifyDCPResultPanel::save_html_report, this);

	_save_text_report->Enable(false);
	_save_html_report->Enable(false);
}


void
VerifyDCPResultPanel::fill(shared_ptr<VerifyDCPJob> job)
{
	if (job->finished_ok() && job->result().notes.empty()) {
		_summary->SetLabel(_("DCP validates OK."));
		return;
	}

	vector<dcp::VerificationNote::Type> types = {
		dcp::VerificationNote::Type::WARNING,
		dcp::VerificationNote::Type::BV21_ERROR,
		dcp::VerificationNote::Type::ERROR
	};

	map<dcp::VerificationNote::Type, std::vector<wxString>> notes;
	for (auto type: types) {
		notes[type] = {};
	}

	auto add = [&notes](dcp::VerificationNote note, wxString message) {
		if (note.reference_hash()) {
			message.Replace("%reference_hash", std_to_wx(note.reference_hash().get()));
		}
		if (note.calculated_hash()) {
			message.Replace("%calculated_hash", std_to_wx(note.calculated_hash().get()));
		}
		if (note.frame()) {
			message.Replace("%frame", std_to_wx(dcp::raw_convert<string>(note.frame().get())));
			message.Replace(
				"%timecode",
				std_to_wx(
					dcp::Time(note.frame().get(), note.frame_rate().get(), note.frame_rate().get()).as_string(dcp::Standard::SMPTE)
					));
		}
		if (note.note()) {
			message.Replace("%n", std_to_wx(note.note().get()));
		}
		if (note.file()) {
			message.Replace("%f", std_to_wx(note.file()->filename().string()));
		}
		if (note.line()) {
			message.Replace("%l", std_to_wx(dcp::raw_convert<string>(note.line().get())));
		}
		if (note.component()) {
			message.Replace("%component", std_to_wx(dcp::raw_convert<string>(note.component().get())));
		}
		if (note.size()) {
			message.Replace("%size", std_to_wx(dcp::raw_convert<string>(note.size().get())));
		}
		if (note.id()) {
			message.Replace("%id", std_to_wx(note.id().get()));
		}
		if (note.other_id()) {
			message.Replace("%other_id", std_to_wx(note.other_id().get()));
		}
		if (note.cpl_id()) {
			message.Replace("%cpl", std_to_wx(note.cpl_id().get()));
		}

		notes[note.type()].push_back(message);
	};

	if (job->finished_in_error() && job->error_summary() != "") {
		/* We have an error that did not come from dcp::verify */
		notes[dcp::VerificationNote::Type::ERROR].push_back(std_to_wx(job->error_summary()));
	}

	for (auto i: job->result().notes) {
		switch (i.code()) {
		case dcp::VerificationNote::Code::FAILED_READ:
			add(i, _("Could not read DCP (%n)"));
			break;
		case dcp::VerificationNote::Code::MISMATCHED_CPL_HASHES:
			add(i, _("The hash (%reference_hash) of the CPL %cpl in the PKL does not agree with the CPL file (%calculated_hash).  This probably means that the CPL file is corrupt."));
			break;
		case dcp::VerificationNote::Code::INVALID_PICTURE_FRAME_RATE:
			add(i, _("The picture in a reel has a frame rate of %n, which is not valid."));
			break;
		case dcp::VerificationNote::Code::INCORRECT_PICTURE_HASH:
			add(i, _("The hash (%calculated_hash) of the picture asset %f does not agree with the PKL file (%reference_hash).  This probably means that the asset file is corrupt."));
			break;
		case dcp::VerificationNote::Code::MISMATCHED_PICTURE_HASHES:
			add(i, _("The PKL and CPL hashes disagree for picture asset %f."));
			break;
		case dcp::VerificationNote::Code::INCORRECT_SOUND_HASH:
			add(i, _("The hash (%calculated_hash) of the sound asset %f does not agree with the PKL file (%reference_hash).  This probably means that the asset file is corrupt."));
			break;
		case dcp::VerificationNote::Code::MISMATCHED_SOUND_HASHES:
			add(i, _("The PKL and CPL hashes disagree for sound asset %f."));
			break;
		case dcp::VerificationNote::Code::EMPTY_ASSET_PATH:
			add(i, _("An asset has an empty path in the ASSETMAP."));
			break;
		case dcp::VerificationNote::Code::MISSING_ASSET:
			add(i, _("The asset %f is missing."));
			break;
		case dcp::VerificationNote::Code::MISMATCHED_STANDARD:
			add(i, _("Parts of the DCP are written according to the Interop standard and parts according to SMPTE."));
			break;
		case dcp::VerificationNote::Code::INVALID_XML:
			if (i.line()) {
				add(i, _("The XML in %f is malformed on line %l (%n)."));
			} else {
				add(i, _("The XML in %f is malformed (%n)."));
			}
			break;
		case dcp::VerificationNote::Code::MISSING_ASSETMAP:
			add(i, _("No ASSETMAP or ASSETMAP.xml file was found."));
			break;
		case dcp::VerificationNote::Code::INVALID_INTRINSIC_DURATION:
			add(i, _("The asset %n has an intrinsic duration of less than 1 second, which is invalid."));
			break;
		case dcp::VerificationNote::Code::INVALID_DURATION:
			add(i, _("The asset %n has a duration of less than 1 second, which is invalid."));
			break;
		case dcp::VerificationNote::Code::INVALID_PICTURE_FRAME_SIZE_IN_BYTES:
			add(i, _("At least one frame of the video asset %f is over the limit of 250Mbit/s."));
			break;
		case dcp::VerificationNote::Code::NEARLY_INVALID_PICTURE_FRAME_SIZE_IN_BYTES:
			add(i, _("Frame %frame (timecode %timecode) in asset %f has an instantaneous bit rate that is close to the limit of 250Mbit/s."));
			break;
		case dcp::VerificationNote::Code::EXTERNAL_ASSET:
			add(i, _("This DCP refers to at the asset %n in another DCP (and perhaps others), so it is a \"version file\" (VF)"));
			break;
		case dcp::VerificationNote::Code::THREED_ASSET_MARKED_AS_TWOD:
			add(i, _("The asset %f is 3D but its MXF is marked as 2D."));
			break;
		case dcp::VerificationNote::Code::INVALID_STANDARD:
			add(i, _("This DCP uses the Interop standard, but it should be made with SMPTE."));
			break;
		case dcp::VerificationNote::Code::INVALID_LANGUAGE:
			add(i, _("The invalid language tag %n is used."));
			break;
		case dcp::VerificationNote::Code::INVALID_PICTURE_SIZE_IN_PIXELS:
			add(i, _("The video asset %f uses the invalid image size %n."));
			break;
		case dcp::VerificationNote::Code::INVALID_PICTURE_FRAME_RATE_FOR_2K:
			add(i, _("The video asset %f uses the invalid frame rate %n."));
			break;
		case dcp::VerificationNote::Code::INVALID_PICTURE_FRAME_RATE_FOR_4K:
			add(i, _("The video asset %f uses the frame rate %n which is invalid for 4K video."));
			break;
		case dcp::VerificationNote::Code::INVALID_PICTURE_ASSET_RESOLUTION_FOR_3D:
			add(i, _("The video asset %f uses the frame rate %n which is invalid for 3D video."));
			break;
		case dcp::VerificationNote::Code::INVALID_CLOSED_CAPTION_XML_SIZE_IN_BYTES:
			add(i, _("The XML in the closed caption asset %f takes up %n bytes which is over the 256KB limit."));
			break;
		case dcp::VerificationNote::Code::INVALID_TIMED_TEXT_SIZE_IN_BYTES:
			add(i, _("The timed text asset %f takes up %n bytes which is over the 115MB limit."));
			break;
		case dcp::VerificationNote::Code::INVALID_TIMED_TEXT_FONT_SIZE_IN_BYTES:
			add(i, _("The fonts in the timed text asset %f take up %n bytes which is over the 10MB limit."));
			break;
		case dcp::VerificationNote::Code::MISSING_SUBTITLE_LANGUAGE:
			add(i, _("The subtitle asset %f contains no <Language> tag."));
			break;
		case dcp::VerificationNote::Code::MISMATCHED_SUBTITLE_LANGUAGES:
			add(i, _("Not all subtitle assets specify the same <Language> tag."));
			break;
		case dcp::VerificationNote::Code::MISSING_SUBTITLE_START_TIME:
			add(i, _("The subtitle asset %f contains no <StartTime> tag."));
			break;
		case dcp::VerificationNote::Code::INVALID_SUBTITLE_START_TIME:
			add(i, _("The subtitle asset %f has a <StartTime> which is not zero."));
			break;
		case dcp::VerificationNote::Code::INVALID_SUBTITLE_FIRST_TEXT_TIME:
			add(i, _("The first subtitle or closed caption happens before 4s into the first reel."));
			break;
		case dcp::VerificationNote::Code::INVALID_SUBTITLE_DURATION:
			add(i, _("At least one subtitle lasts less than 15 frames."));
			break;
		case dcp::VerificationNote::Code::INVALID_SUBTITLE_SPACING:
			add(i, _("At least one pair of subtitles is separated by less than 2 frames."));
			break;
		case dcp::VerificationNote::Code::INVALID_SUBTITLE_LINE_COUNT:
			add(i, _("There are more than 3 subtitle lines in at least one place."));
			break;
		case dcp::VerificationNote::Code::NEARLY_INVALID_SUBTITLE_LINE_LENGTH:
			add(i, _("There are more than 52 characters in at least one subtitle line."));
			break;
		case dcp::VerificationNote::Code::INVALID_SUBTITLE_LINE_LENGTH:
			add(i, _("There are more than 79 characters in at least one subtitle line."));
			break;
		case dcp::VerificationNote::Code::INVALID_CLOSED_CAPTION_LINE_COUNT:
			add(i, _("There are more than 3 closed caption lines in at least one place."));
			break;
		case dcp::VerificationNote::Code::INVALID_CLOSED_CAPTION_LINE_LENGTH:
			add(i, _("There are more than 32 characters in at least one closed caption line."));
			break;
		case dcp::VerificationNote::Code::INVALID_SOUND_FRAME_RATE:
			add(i, _("The sound asset %f has an invalid frame rate of %n."));
			break;
		case dcp::VerificationNote::Code::INVALID_SOUND_BIT_DEPTH:
			add(i, _("The sound asset %f has an invalid bit depth of %n."));
			break;
		case dcp::VerificationNote::Code::MISSING_CPL_ANNOTATION_TEXT:
			add(i, _("The CPL %cpl has no <AnnotationText> tag."));
			break;
		case dcp::VerificationNote::Code::MISMATCHED_CPL_ANNOTATION_TEXT:
			add(i, _("The CPL %cpcpl has an <AnnotationText> which is not the same as its <ContentTitleText>."));
			break;
		case dcp::VerificationNote::Code::MISMATCHED_ASSET_DURATION:
			add(i, _("At least one asset in a reel does not have the same duration as the others."));
			break;
		case dcp::VerificationNote::Code::MISSING_MAIN_SUBTITLE_FROM_SOME_REELS:
			add(i, _("The DCP has subtitles but at least one reel has no subtitle asset."));
			break;
		case dcp::VerificationNote::Code::MISMATCHED_CLOSED_CAPTION_ASSET_COUNTS:
			add(i, _("The DCP has closed captions but not every reel has the same number of closed caption assets."));
			break;
		case dcp::VerificationNote::Code::MISSING_SUBTITLE_ENTRY_POINT:
			add(i, _("The subtitle asset %n has no <EntryPoint> tag."));
			break;
		case dcp::VerificationNote::Code::INCORRECT_SUBTITLE_ENTRY_POINT:
			add(i, _("Subtitle asset %n has a non-zero <EntryPoint>."));
			break;
		case dcp::VerificationNote::Code::MISSING_CLOSED_CAPTION_ENTRY_POINT:
			add(i, _("The closed caption asset %n has no <EntryPoint> tag."));
			break;
		case dcp::VerificationNote::Code::INCORRECT_CLOSED_CAPTION_ENTRY_POINT:
			add(i, _("Closed caption asset %n has a non-zero <EntryPoint>."));
			break;
		case dcp::VerificationNote::Code::MISSING_HASH:
			add(i, _("The asset %n has no <Hash> in the CPL."));
			break;
		case dcp::VerificationNote::Code::MISSING_FFEC_IN_FEATURE:
			add(i, _("The DCP is a feature but has no FFEC (first frame of end credits) marker."));
			break;
		case dcp::VerificationNote::Code::MISSING_FFMC_IN_FEATURE:
			add(i, _("The DCP is a feature but has no FFMC (first frame of moving credits) marker."));
			break;
		case dcp::VerificationNote::Code::MISSING_FFOC:
			add(i, _("The DCP has no FFOC (first frame of content) marker."));
			break;
		case dcp::VerificationNote::Code::MISSING_LFOC:
			add(i, _("The DCP has no LFOC (last frame of content) marker."));
			break;
		case dcp::VerificationNote::Code::INCORRECT_FFOC:
			add(i, _("The DCP has a FFOC of %n instead of 1."));
			break;
		case dcp::VerificationNote::Code::INCORRECT_LFOC:
			add(i, _("The DCP has a LFOC of %n instead of the reel duration minus one."));
			break;
		case dcp::VerificationNote::Code::MISSING_CPL_METADATA:
			add(i, _("The CPL %cpl has no CPL metadata tag."));
			break;
		case dcp::VerificationNote::Code::MISSING_CPL_METADATA_VERSION_NUMBER:
			add(i, _("The CPL %cpl has no CPL metadata version number tag."));
			break;
		case dcp::VerificationNote::Code::MISSING_EXTENSION_METADATA:
			add(i, _("The CPL %cpl has no CPL extension metadata tag."));
			break;
		case dcp::VerificationNote::Code::INVALID_EXTENSION_METADATA:
			add(i, _("The CPL %f has an invalid CPL extension metadata tag (%n)"));
			break;
		case dcp::VerificationNote::Code::UNSIGNED_CPL_WITH_ENCRYPTED_CONTENT:
			add(i, _("The CPL %cpl has encrypted content but is not signed."));
			break;
		case dcp::VerificationNote::Code::UNSIGNED_PKL_WITH_ENCRYPTED_CONTENT:
			add(i, _("The PKL %n has encrypted content but is not signed."));
			break;
		case dcp::VerificationNote::Code::MISMATCHED_PKL_ANNOTATION_TEXT_WITH_CPL:
			add(i, _("The PKL %n has an <AnnotationText> which does not match its CPL's <ContentTitleText>."));
			break;
		case dcp::VerificationNote::Code::PARTIALLY_ENCRYPTED:
			add(i, _("The DCP has encrypted content, but not all its assets are encrypted."));
			break;
		case dcp::VerificationNote::Code::INVALID_JPEG2000_CODESTREAM:
			add(i, _("A picture frame has an invalid JPEG2000 codestream (%n)"));
			break;
		case dcp::VerificationNote::Code::INVALID_JPEG2000_GUARD_BITS_FOR_2K:
			add(i, _("A 2K JPEG2000 frame has %n guard bits instead of 1."));
			break;
		case dcp::VerificationNote::Code::INVALID_JPEG2000_GUARD_BITS_FOR_4K:
			add(i, _("A 4K JPEG2000 frame has %n guard bits instead of 2."));
			break;
		case dcp::VerificationNote::Code::INVALID_JPEG2000_TILE_SIZE:
			add(i, _("A JPEG2000 tile size does not match the image size."));
			break;
		case dcp::VerificationNote::Code::INVALID_JPEG2000_CODE_BLOCK_WIDTH:
			add(i, _("A JPEG2000 frame has a code-block width of %n instead of 32."));
			break;
		case dcp::VerificationNote::Code::INVALID_JPEG2000_CODE_BLOCK_HEIGHT:
			add(i, _("A JPEG2000 frame has a code-block height of %n instead of 32."));
			break;
		case dcp::VerificationNote::Code::INCORRECT_JPEG2000_POC_MARKER_COUNT_FOR_2K:
			add(i, _("A 2K JPEG2000 frame has %n POC marker(s) instead of 0."));
			break;
		case dcp::VerificationNote::Code::INCORRECT_JPEG2000_POC_MARKER_COUNT_FOR_4K:
			add(i, _("A 4K JPEG2000 frame has %n POC marker(s) instead of 1."));
			break;
		case dcp::VerificationNote::Code::INCORRECT_JPEG2000_POC_MARKER:
			add(i, _("A JPEG2000 frame contains an invalid POC marker (%n)."));
			break;
		case dcp::VerificationNote::Code::INVALID_JPEG2000_POC_MARKER_LOCATION:
			add(i, _("A JPEG2000 frame contains POC marker in an invalid location."));
			break;
		case dcp::VerificationNote::Code::INVALID_JPEG2000_TILE_PARTS_FOR_2K:
			add(i, _("A 2K JPEG2000 frame contains %n tile parts instead of 3."));
			break;
		case dcp::VerificationNote::Code::INVALID_JPEG2000_TILE_PARTS_FOR_4K:
			add(i, _("A 4K JPEG2000 frame contains %n tile parts instead of 6."));
			break;
		case dcp::VerificationNote::Code::MISSING_JPEG200_TLM_MARKER:
			add(i, _("A JPEG2000 frame has no TLM marker."));
			break;
		case dcp::VerificationNote::Code::SUBTITLE_OVERLAPS_REEL_BOUNDARY:
			add(i, _("A subtitle lasts longer than the reel it is in."));
			break;
		case dcp::VerificationNote::Code::MISMATCHED_TIMED_TEXT_RESOURCE_ID:
			add(i, _("The Resource ID in a timed text MXF did not match the ID of the contained XML."));
			break;
		case dcp::VerificationNote::Code::INCORRECT_TIMED_TEXT_ASSET_ID:
			add(i, _("The Asset ID in a timed text MXF is the same as the Resource ID or that of the contained XML."));
			break;
		case dcp::VerificationNote::Code::MISMATCHED_TIMED_TEXT_DURATION:
		{
			vector<string> parts;
			boost::split(parts, i.note().get(), boost::is_any_of(" "));
			add(i, wxString::Format(_("The reel duration (%s) of some timed text is not the same as the ContainerDuration (%s) of its MXF."), std_to_wx(parts[0]), std_to_wx(parts[1])));
			break;
		}
		case dcp::VerificationNote::Code::MISSED_CHECK_OF_ENCRYPTED:
			add(i, _("Part of the DCP could not be checked because no KDM was available."));
			break;
		case dcp::VerificationNote::Code::EMPTY_TEXT:
			add(i, _("At least one <Text> node in a subtitle or closed caption is empty."));
			break;
		case dcp::VerificationNote::Code::MISMATCHED_CLOSED_CAPTION_VALIGN:
			add(i, _("Some closed <Text> or <Image> nodes have different vertical alignments within a <Subtitle>."));
			break;
		case dcp::VerificationNote::Code::INCORRECT_CLOSED_CAPTION_ORDERING:
			add(i, _("Some closed captions are not listed in the order of their vertical position."));
			break;
		case dcp::VerificationNote::Code::UNEXPECTED_ENTRY_POINT:
			add(i, _("There is a <EntryPoint> tag inside a <MainMarkers>."));
			break;
		case dcp::VerificationNote::Code::UNEXPECTED_DURATION:
			add(i, _("There is a <Duration> tag inside a <MainMarkers>."));
			break;
		case dcp::VerificationNote::Code::INVALID_CONTENT_KIND:
			add(i, _("An invalid <ContentKind> %n has been used."));
			break;
		case dcp::VerificationNote::Code::INVALID_MAIN_PICTURE_ACTIVE_AREA:
			add(i, _("The <MainPictureActiveArea> is either not a multiple of 2, or is bigger than an asset."));
			break;
		case dcp::VerificationNote::Code::DUPLICATE_ASSET_ID_IN_PKL:
			add(i, _("The PKL %n has more than one asset with the same ID."));
			break;
		case dcp::VerificationNote::Code::DUPLICATE_ASSET_ID_IN_ASSETMAP:
			add(i, _("The ASSETMAP %n has more than one asset with the same ID."));
			break;
		case dcp::VerificationNote::Code::MISSING_SUBTITLE:
			add(i, _("The subtitle asset %n contains no subtitles."));
			break;
		case dcp::VerificationNote::Code::INVALID_SUBTITLE_ISSUE_DATE:
			add(i, _("<IssueDate> has an invalid value %n"));
			break;
		case dcp::VerificationNote::Code::MISMATCHED_SOUND_CHANNEL_COUNTS:
			add(i, _("Sound assets do not all have the same channel count."));
			break;
		case dcp::VerificationNote::Code::INVALID_MAIN_SOUND_CONFIGURATION:
			add(i, _("<MainSoundConfiguration> describes incorrect number of channels (%n)"));
			break;
		case dcp::VerificationNote::Code::MISSING_FONT:
			add(i, _("The font file for font ID \"%n\" was not found, or was not referred to in the ASSETMAP."));
			break;
		case dcp::VerificationNote::Code::INVALID_JPEG2000_TILE_PART_SIZE:
			add(i, _("Frame %frame has an image component that is too large (component %component is %size bytes in size)."));
			break;
		case dcp::VerificationNote::Code::INCORRECT_SUBTITLE_NAMESPACE_COUNT:
			add(i, _("The XML in the subtitle asset %n has more than one namespace declaration."));
			break;
		case dcp::VerificationNote::Code::MISSING_LOAD_FONT_FOR_FONT:
			add(i, _("A subtitle or closed caption refers to a font with ID %id that does not have a corresponding <LoadFont> node."));
			break;
		case dcp::VerificationNote::Code::MISSING_LOAD_FONT:
			add(i, _("The SMPTE subtitle asset %id has <Text> nodes but no <LoadFont> node"));
			break;
		case dcp::VerificationNote::Code::MISMATCHED_ASSET_MAP_ID:
			add(i, _("The asset with ID %id in the asset map actually has an id of %other_id"));
			break;
		case dcp::VerificationNote::Code::EMPTY_CONTENT_VERSION_LABEL_TEXT:
			add(i, _("The <LabelText> in a <ContentVersion> in CPL %cpl is empty"));
			break;
		case dcp::VerificationNote::Code::INVALID_CPL_NAMESPACE:
			add(i, _("The CPL %cpl has an invalid namespace %n"));
			break;
		case dcp::VerificationNote::Code::MISSING_CPL_CONTENT_VERSION:
			add(i, _("The CPL %cpl has no <ContentVersion> tag"));
			break;
		case dcp::VerificationNote::Code::MATCHING_CPL_HASHES:
		case dcp::VerificationNote::Code::CORRECT_PICTURE_HASH:
		case dcp::VerificationNote::Code::VALID_PICTURE_FRAME_SIZES_IN_BYTES:
		case dcp::VerificationNote::Code::VALID_RELEASE_TERRITORY:
		case dcp::VerificationNote::Code::VALID_CPL_ANNOTATION_TEXT:
		case dcp::VerificationNote::Code::MATCHING_PKL_ANNOTATION_TEXT_WITH_CPL:
		case dcp::VerificationNote::Code::ALL_ENCRYPTED:
		case dcp::VerificationNote::Code::NONE_ENCRYPTED:
		case dcp::VerificationNote::Code::VALID_CONTENT_KIND:
		case dcp::VerificationNote::Code::VALID_MAIN_PICTURE_ACTIVE_AREA:
		case dcp::VerificationNote::Code::VALID_CONTENT_VERSION_LABEL_TEXT:
			/* These are all "OK" messages which we don't report here */
			break;
		}
	}

	wxString summary_text;

	if (notes[dcp::VerificationNote::Type::ERROR].size() == 1) {
		/// TRANSLATORS: this will be used at the start of a string like "1 error, 2 Bv2.1 errors and 3 warnings."
		summary_text = _("1 error, ");
	} else {
		/// TRANSLATORS: this will be used at the start of a string like "1 error, 2 Bv2.1 errors and 3 warnings."
		summary_text = wxString::Format("%d errors, ", static_cast<int>(notes[dcp::VerificationNote::Type::ERROR].size()));
	}

	if (notes[dcp::VerificationNote::Type::BV21_ERROR].size() == 1) {
		/// TRANSLATORS: this will be used in the middle of a string like "1 error, 2 Bv2.1 errors and 3 warnings."
		summary_text += _("1 Bv2.1 error, ");
	} else {
		/// TRANSLATORS: this will be used in the middle of a string like "1 error, 2 Bv2.1 errors and 3 warnings."
		summary_text += wxString::Format("%d Bv2.1 errors, ", static_cast<int>(notes[dcp::VerificationNote::Type::BV21_ERROR].size()));
	}

	if (notes[dcp::VerificationNote::Type::WARNING].size() == 1) {
		/// TRANSLATORS: this will be used at the end of a string like "1 error, 2 Bv2.1 errors and 3 warnings."
		summary_text += _("and 1 warning.");
	} else {
		/// TRANSLATORS: this will be used at the end of a string like "1 error, 2 Bv2.1 errors and 3 warnings."
		summary_text += wxString::Format("and %d warnings.", static_cast<int>(notes[dcp::VerificationNote::Type::WARNING].size()));
	}

	_summary->SetLabel(summary_text);

	auto add_bullet = [this](dcp::VerificationNote::Type type, wxString message) {
		_pages[type]->BeginStandardBullet(N_("standard/diamond"), 1, 50);
		_pages[type]->WriteText(message);
		_pages[type]->Newline();
		_pages[type]->EndStandardBullet();
	};

	if (notes[dcp::VerificationNote::Type::ERROR].empty()) {
		add_bullet(dcp::VerificationNote::Type::ERROR, _("No errors found."));
	}

	if (notes[dcp::VerificationNote::Type::BV21_ERROR].empty()) {
		add_bullet(dcp::VerificationNote::Type::BV21_ERROR, _("No SMPTE Bv2.1 errors found."));
	}

	if (notes[dcp::VerificationNote::Type::WARNING].empty()) {
		add_bullet(dcp::VerificationNote::Type::WARNING, _("No warnings found."));
	}

	for (auto type: types) {
		std::sort(notes[type].begin(), notes[type].end());
		for (auto i = notes[type].begin(); i != notes[type].end(); ++i) {
			int extra = 0;
			while (std::next(i) != notes[type].end() && *std::next(i) == *i) {
				++i;
				++extra;
			}
			if (extra == 1) {
				add_bullet(type, wxString::Format(_("%s (repeated twice)."), i->SubString(0, i->Length() - 2)));
			} else if (extra > 1) {
				add_bullet(type, wxString::Format(_("%s (repeated %d times)."), i->SubString(0, i->Length() - 2), extra + 1));
			} else {
				add_bullet(type, *i);
			}
		}
	}

	_job = job;
	_save_text_report->Enable(true);
	_save_html_report->Enable(true);
}


template <class T>
void save(wxWindow* parent, wxString filter, dcp::VerificationResult const& result)
{
	FileDialog dialog(parent, _("Verification report"), filter, wxFD_SAVE | wxFD_OVERWRITE_PROMPT, "SaveVerificationReport");
	if (!dialog.show()) {
		return;
	}

	T formatter(dialog.path());
	dcp::verify_report(result, formatter);
}


void
VerifyDCPResultPanel::save_text_report()
{
	if (_job) {
		save<dcp::TextFormatter>(this, wxT("Text files (*.txt)|*.txt"), _job->result());
	}
}


void
VerifyDCPResultPanel::save_html_report()
{
	if (_job) {
		save<dcp::HTMLFormatter>(this, wxT("HTML files (*.htm;*html)|*.htm;*.html"), _job->result());
	}
}