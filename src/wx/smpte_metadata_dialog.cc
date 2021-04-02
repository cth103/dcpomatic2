/*
    Copyright (C) 2019-2021 Carl Hetherington <cth@carlh.net>

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


#include "content_version_dialog.h"
#include "editable_list.h"
#include "language_tag_dialog.h"
#include "language_tag_widget.h"
#include "smpte_metadata_dialog.h"
#include "rating_dialog.h"
#include "lib/film.h"
#include <dcp/types.h>
#include <wx/gbsizer.h>
#include <wx/spinctrl.h>


using std::string;
using std::vector;
using boost::optional;
using std::shared_ptr;
using std::weak_ptr;
#if BOOST_VERSION >= 106100
using namespace boost::placeholders;
#endif


static string
ratings_column (dcp::Rating r, int c)
{
	if (c == 0) {
		return r.agency;
	}

	return r.label;
}


static string
content_versions_column (string v, int)
{
	return v;
}


void
SMPTEMetadataDialog::setup_standard (wxPanel* panel, wxSizer* sizer)
{
	MetadataDialog::setup_standard (panel, sizer);

	add_label_to_sizer (sizer, panel, _("Title language"), true, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL);
	_name_language = new LanguageTagWidget(
		panel,
		wxString::Format(_("The language that the film's title (\"%s\") is in"), std_to_wx(film()->name())),
		film()->name_language()
		);
	sizer->Add (_name_language->sizer(), 0, wxEXPAND);

	{
		int flags = wxALIGN_TOP | wxRIGHT | wxTOP;
#ifdef __WXOSX__
		flags |= wxALIGN_RIGHT;
#endif
		auto m = create_label (panel, _("Ratings"), true);
		sizer->Add (m, 0, flags, DCPOMATIC_SIZER_GAP);
	}

	vector<EditableListColumn> columns;
	columns.push_back (EditableListColumn("Agency", 200, true));
	columns.push_back (EditableListColumn("Label", 50, true));
	_ratings = new EditableList<dcp::Rating, RatingDialog> (
		panel,
		columns,
		boost::bind(&SMPTEMetadataDialog::ratings, this),
		boost::bind(&SMPTEMetadataDialog::set_ratings, this, _1),
		boost::bind(&ratings_column, _1, _2),
		true,
		false
		);
	sizer->Add (_ratings, 1, wxEXPAND);
}


void
SMPTEMetadataDialog::setup_advanced (wxPanel* panel, wxSizer* sizer)
{
	MetadataDialog::setup_advanced (panel, sizer);

	add_label_to_sizer (sizer, panel, _("Version number"), true, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL);
	_version_number = new wxSpinCtrl (panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 1000);
	sizer->Add (_version_number, 0);

	add_label_to_sizer (sizer, panel, _("Status"), true, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL);
	_status = new wxChoice (panel, wxID_ANY);
	sizer->Add (_status, 0);

	_enable_chain = new wxCheckBox (panel, wxID_ANY, _("Chain"));
	sizer->Add (_enable_chain, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL);
	_chain = new wxTextCtrl (panel, wxID_ANY);
	sizer->Add (_chain, 1, wxEXPAND);

	_enable_distributor = new wxCheckBox (panel, wxID_ANY, _("Distributor"));
	sizer->Add (_enable_distributor, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL);
	_distributor = new wxTextCtrl (panel, wxID_ANY);
	sizer->Add (_distributor, 1, wxEXPAND);

	_enable_facility = new wxCheckBox (panel, wxID_ANY, _("Facility"));
	sizer->Add (_enable_facility, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL);
	_facility = new wxTextCtrl (panel, wxID_ANY);
	sizer->Add (_facility, 1, wxEXPAND);

	add_label_to_sizer (sizer, panel, _("Luminance"), true, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL);
	{
		auto s = new wxBoxSizer (wxHORIZONTAL);
		_luminance_value = new wxSpinCtrlDouble (panel, wxID_ANY);
		_luminance_value->SetDigits (1);
		_luminance_value->SetIncrement (0.1);
		s->Add (_luminance_value, 0);
		_luminance_unit = new wxChoice (panel, wxID_ANY);
		s->Add (_luminance_unit, 0, wxLEFT, DCPOMATIC_SIZER_X_GAP);
		sizer->Add (s, 1, wxEXPAND);
	}

	{
		int flags = wxALIGN_TOP | wxRIGHT | wxTOP;
#ifdef __WXOSX__
		flags |= wxALIGN_RIGHT;
#endif
		auto m = create_label (panel, _("Content versions"), true);
		sizer->Add (m, 0, flags, DCPOMATIC_SIZER_GAP);
	}

	vector<EditableListColumn> columns;
	columns.push_back (EditableListColumn("Version", 350, true));
	_content_versions = new EditableList<string, ContentVersionDialog> (
		panel,
		columns,
		boost::bind(&SMPTEMetadataDialog::content_versions, this),
		boost::bind(&SMPTEMetadataDialog::set_content_versions, this, _1),
		boost::bind(&content_versions_column, _1, _2),
		true,
		false
		);
	sizer->Add (_content_versions, 1, wxEXPAND);
}


SMPTEMetadataDialog::SMPTEMetadataDialog (wxWindow* parent, weak_ptr<Film> weak_film)
	: MetadataDialog (parent, weak_film)
{

}


void
SMPTEMetadataDialog::setup ()
{
	MetadataDialog::setup ();

	_status->Append (_("Temporary"));
	_status->Append (_("Pre-release"));
	_status->Append (_("Final"));

	_luminance_unit->Append (wxString::FromUTF8(_("candela per m²")));
	_luminance_unit->Append (_("foot lambert"));

	_name_language->Changed.connect (boost::bind(&SMPTEMetadataDialog::name_language_changed, this, _1));
	_version_number->Bind (wxEVT_SPINCTRL, boost::bind(&SMPTEMetadataDialog::version_number_changed, this));
	_status->Bind (wxEVT_CHOICE, boost::bind(&SMPTEMetadataDialog::status_changed, this));
	_enable_chain->Bind (wxEVT_CHECKBOX, boost::bind(&SMPTEMetadataDialog::enable_chain_changed, this));
	_chain->Bind (wxEVT_TEXT, boost::bind(&SMPTEMetadataDialog::chain_changed, this));
	_enable_distributor->Bind (wxEVT_CHECKBOX, boost::bind(&SMPTEMetadataDialog::enable_distributor_changed, this));
	_distributor->Bind (wxEVT_TEXT, boost::bind(&SMPTEMetadataDialog::distributor_changed, this));
	_enable_facility->Bind (wxEVT_CHECKBOX, boost::bind(&SMPTEMetadataDialog::enable_facility_changed, this));
	_facility->Bind (wxEVT_TEXT, boost::bind(&SMPTEMetadataDialog::facility_changed, this));
	_luminance_value->Bind (wxEVT_SPINCTRLDOUBLE, boost::bind(&SMPTEMetadataDialog::luminance_changed, this));
	_luminance_unit->Bind (wxEVT_CHOICE, boost::bind(&SMPTEMetadataDialog::luminance_changed, this));

	film_changed (ChangeType::DONE, Film::Property::NAME_LANGUAGE);
	film_changed (ChangeType::DONE, Film::Property::VERSION_NUMBER);
	film_changed (ChangeType::DONE, Film::Property::STATUS);
	film_changed (ChangeType::DONE, Film::Property::CHAIN);
	film_changed (ChangeType::DONE, Film::Property::DISTRIBUTOR);
	film_changed (ChangeType::DONE, Film::Property::FACILITY);
	film_changed (ChangeType::DONE, Film::Property::CONTENT_VERSIONS);
	film_changed (ChangeType::DONE, Film::Property::LUMINANCE);

	setup_sensitivity ();
}


void
SMPTEMetadataDialog::film_changed (ChangeType type, Film::Property property)
{
	MetadataDialog::film_changed (type, property);

	if (type != ChangeType::DONE || film()->interop()) {
		return;
	}

	if (property == Film::Property::NAME_LANGUAGE) {
		_name_language->set (film()->name_language());
	} else if (property == Film::Property::VERSION_NUMBER) {
		checked_set (_version_number, film()->version_number());
	} else if (property == Film::Property::STATUS) {
		switch (film()->status()) {
		case dcp::Status::TEMP:
			checked_set (_status, 0);
			break;
		case dcp::Status::PRE:
			checked_set (_status, 1);
			break;
		case dcp::Status::FINAL:
			checked_set (_status, 2);
			break;
		}
	} else if (property == Film::Property::CHAIN) {
		checked_set (_enable_chain, static_cast<bool>(film()->chain()));
		if (film()->chain()) {
			checked_set (_chain, *film()->chain());
		}
	} else if (property == Film::Property::DISTRIBUTOR) {
		checked_set (_enable_distributor, static_cast<bool>(film()->distributor()));
		if (film()->distributor()) {
			checked_set (_distributor, *film()->distributor());
		}
	} else if (property == Film::Property::FACILITY) {
		checked_set (_enable_facility, static_cast<bool>(film()->facility()));
		if (film()->facility()) {
			checked_set (_facility, *film()->facility());
		}
	} else if (property == Film::Property::LUMINANCE) {
		auto lum = film()->luminance();
		if (lum) {
			checked_set (_luminance_value, lum->value());
			switch (lum->unit()) {
			case dcp::Luminance::Unit::CANDELA_PER_SQUARE_METRE:
				checked_set (_luminance_unit, 0);
				break;
			case dcp::Luminance::Unit::FOOT_LAMBERT:
				checked_set (_luminance_unit, 1);
				break;
			}
		} else {
			checked_set (_luminance_value, 4.5);
			checked_set (_luminance_unit, 1);
		}
	}
}


vector<dcp::Rating>
SMPTEMetadataDialog::ratings () const
{
	return film()->ratings ();
}


void
SMPTEMetadataDialog::set_ratings (vector<dcp::Rating> r)
{
	film()->set_ratings (r);
}


vector<string>
SMPTEMetadataDialog::content_versions () const
{
	return film()->content_versions ();
}


void
SMPTEMetadataDialog::set_content_versions (vector<string> cv)
{
	film()->set_content_versions (cv);
}


void
SMPTEMetadataDialog::name_language_changed (dcp::LanguageTag tag)
{
	film()->set_name_language (tag);
}


void
SMPTEMetadataDialog::version_number_changed ()
{
	film()->set_version_number (_version_number->GetValue());
}


void
SMPTEMetadataDialog::status_changed ()
{
	switch (_status->GetSelection()) {
	case 0:
		film()->set_status(dcp::Status::TEMP);
		break;
	case 1:
		film()->set_status(dcp::Status::PRE);
		break;
	case 2:
		film()->set_status(dcp::Status::FINAL);
		break;
	}
}


void
SMPTEMetadataDialog::chain_changed ()
{
	film()->set_chain (wx_to_std(_chain->GetValue()));
}


void
SMPTEMetadataDialog::distributor_changed ()
{
	film()->set_distributor (wx_to_std(_distributor->GetValue()));
}


void
SMPTEMetadataDialog::facility_changed ()
{
	film()->set_facility (wx_to_std(_facility->GetValue()));
}


void
SMPTEMetadataDialog::luminance_changed ()
{
	dcp::Luminance::Unit unit;
	switch (_luminance_unit->GetSelection()) {
	case 0:
		unit = dcp::Luminance::Unit::CANDELA_PER_SQUARE_METRE;
		break;
	case 1:
		unit = dcp::Luminance::Unit::FOOT_LAMBERT;
		break;
	default:
		DCPOMATIC_ASSERT (false);
	}

	film()->set_luminance (dcp::Luminance(_luminance_value->GetValue(), unit));
}


void
SMPTEMetadataDialog::setup_sensitivity ()
{
	MetadataDialog::setup_sensitivity ();

	_chain->Enable (_enable_chain->GetValue());
	_distributor->Enable (_enable_distributor->GetValue());
	_facility->Enable (_enable_facility->GetValue());
}


void
SMPTEMetadataDialog::enable_chain_changed ()
{
	setup_sensitivity ();
	if (_enable_chain->GetValue()) {
		film()->set_chain (wx_to_std(_chain->GetValue()));
	} else {
		film()->set_chain ();
	}
}


void
SMPTEMetadataDialog::enable_distributor_changed ()
{
	setup_sensitivity ();
	if (_enable_distributor->GetValue()) {
		film()->set_distributor (wx_to_std(_distributor->GetValue()));
	} else {
		film()->set_distributor ();
	}
}


void
SMPTEMetadataDialog::enable_facility_changed ()
{
	setup_sensitivity ();
	if (_enable_facility->GetValue()) {
		film()->set_facility (wx_to_std(_facility->GetValue()));
	} else {
		film()->set_facility ();
	}
}


