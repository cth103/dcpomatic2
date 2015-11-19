/*
    Copyright (C) 2012-2015 Carl Hetherington <cth@carlh.net>

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

#include "self_dkdm_dialog.h"
#include "wx_util.h"
#include "kdm_output_panel.h"
#include "kdm_cpl_panel.h"
#include "lib/film.h"
#include "lib/screen.h"
#include <libcxml/cxml.h>
#ifdef DCPOMATIC_USE_OWN_PICKER
#include "dir_picker_ctrl.h"
#else
#include <wx/filepicker.h>
#endif
#include <wx/treectrl.h>
#include <wx/listctrl.h>
#include <wx/stdpaths.h>
#include <iostream>

using std::string;
using std::map;
using std::list;
using std::pair;
using std::cout;
using std::vector;
using std::make_pair;
using boost::shared_ptr;

SelfDKDMDialog::SelfDKDMDialog (wxWindow* parent, boost::shared_ptr<const Film> film)
	: wxDialog (parent, wxID_ANY, _("Make DKDM for DCP-o-matic"))
{
	/* Main sizer */
	wxBoxSizer* vertical = new wxBoxSizer (wxVERTICAL);

	/* Font for sub-headings */
	wxFont subheading_font (*wxNORMAL_FONT);
	subheading_font.SetWeight (wxFONTWEIGHT_BOLD);

	/* Sub-heading: CPL */
	wxStaticText* h = new wxStaticText (this, wxID_ANY, _("CPL"));
	h->SetFont (subheading_font);
	vertical->Add (h, 0, wxALIGN_CENTER_VERTICAL, DCPOMATIC_SIZER_Y_GAP * 2);
	_cpl = new KDMCPLPanel (this, film->cpls ());
	vertical->Add (_cpl);

	/* Sub-heading: Output */
	h = new wxStaticText (this, wxID_ANY, _("Output"));
	h->SetFont (subheading_font);
	vertical->Add (h, 0, wxALIGN_CENTER_VERTICAL | wxTOP, DCPOMATIC_SIZER_Y_GAP * 2);

	wxFlexGridSizer* table = new wxFlexGridSizer (2, DCPOMATIC_SIZER_X_GAP, DCPOMATIC_SIZER_Y_GAP);

	add_label_to_sizer (table, this, _("Write to"), true);

#ifdef DCPOMATIC_USE_OWN_PICKER
	_folder = new DirPickerCtrl (this);
#else
	_folder = new wxDirPickerCtrl (this, wxID_ANY, wxEmptyString, wxDirSelectorPromptStr, wxDefaultPosition, wxSize (300, -1));
#endif

	_folder->SetPath (wxStandardPaths::Get().GetDocumentsDir());

	table->Add (_folder, 1, wxEXPAND);

	vertical->Add (table);

	/* Make an overall sizer to get a nice border, and put some buttons in */

	wxBoxSizer* overall_sizer = new wxBoxSizer (wxVERTICAL);
	overall_sizer->Add (vertical, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, DCPOMATIC_DIALOG_BORDER);

	wxSizer* buttons = CreateSeparatedButtonSizer (wxOK | wxCANCEL);
	if (buttons) {
		overall_sizer->Add (buttons, 0, wxEXPAND | wxALL, DCPOMATIC_SIZER_Y_GAP);
	}

	setup_sensitivity ();

	SetSizer (overall_sizer);
	overall_sizer->Layout ();
	overall_sizer->SetSizeHints (this);
}

void
SelfDKDMDialog::setup_sensitivity ()
{
	wxButton* ok = dynamic_cast<wxButton *> (FindWindowById (wxID_OK, this));
	if (ok) {
		ok->Enable (_cpl->has_selected ());
	}
}

boost::filesystem::path
SelfDKDMDialog::cpl () const
{
	return _cpl->cpl ();
}

boost::filesystem::path
SelfDKDMDialog::directory () const
{
	return wx_to_std (_folder->GetPath ());
}
