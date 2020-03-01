/*
    Copyright (C) 2018-2019 Carl Hetherington <cth@carlh.net>

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

#include "swaroop_spl.h"
#include "content_store.h"
#include <libcxml/cxml.h>
#include <dcp/raw_convert.h>
#include <libxml++/libxml++.h>
#include <boost/foreach.hpp>
#include <iostream>

using std::cout;
using std::string;
using boost::shared_ptr;
using dcp::raw_convert;

void
SPL::read (boost::filesystem::path path, ContentStore* store)
{
	_path = path;

	_spl.clear ();
	_missing = false;
	cxml::Document doc ("SPL");
	doc.read_file (path);
	_id = doc.string_child("Id");
	BOOST_FOREACH (cxml::ConstNodePtr i, doc.node_children("Entry")) {
		shared_ptr<Content> c = store->get(i->string_child("Digest"));
		if (c) {
			add (SPLEntry(c, i));
		} else {
			_missing = true;
		}
	}

	_allowed_shows = doc.optional_number_child<int>("AllowedShows");
}

void
SPL::write (boost::filesystem::path path) const
{
	_path = path;

	xmlpp::Document doc;
	xmlpp::Element* root = doc.create_root_node ("SPL");
	root->add_child("Id")->add_child_text (_id);
	BOOST_FOREACH (SPLEntry i, _spl) {
		i.as_xml (root->add_child("Entry"));
	}
	if (_allowed_shows) {
		root->add_child("AllowedShows")->add_child_text(raw_convert<string>(*_allowed_shows));
	}
	doc.write_to_file_formatted (path.string());
}