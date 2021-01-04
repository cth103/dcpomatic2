/*
    Copyright (C) 2018-2020 Carl Hetherington <cth@carlh.net>

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

#include "spl_entry.h"
#include "dcp_content.h"
#include "dcpomatic_assert.h"
#include "warnings.h"
DCPOMATIC_DISABLE_WARNINGS
#include <libxml++/libxml++.h>
DCPOMATIC_ENABLE_WARNINGS

using std::shared_ptr;
using std::dynamic_pointer_cast;

SPLEntry::SPLEntry (shared_ptr<Content> content)
{
	construct (content);
}

void
SPLEntry::construct (shared_ptr<Content> c)
{
	content = c;
	shared_ptr<DCPContent> dcp = dynamic_pointer_cast<DCPContent> (content);
	digest = content->digest ();
	if (dcp) {
		name = dcp->name ();
		DCPOMATIC_ASSERT (dcp->cpl());
		id = *dcp->cpl();
		kind = dcp->content_kind().get_value_or(dcp::FEATURE);
		encrypted = dcp->encrypted ();
	} else {
		name = content->path(0).filename().string();
		kind = dcp::FEATURE;
	}
}

void
SPLEntry::as_xml (xmlpp::Element* e)
{
	e->add_child("Digest")->add_child_text(digest);
}
