/*
    Copyright (C) 2012 Carl Hetherington <cth@carlh.net>

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

/** @file src/config.h
 *  @brief Class holding configuration.
 */

#ifndef DCPOMATIC_CONFIG_H
#define DCPOMATIC_CONFIG_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <boost/filesystem.hpp>
#include <libdcp/metadata.h>
#include "dci_metadata.h"
#include "colour_conversion.h"
#include "server.h"

class ServerDescription;
class Scaler;
class Filter;
class SoundProcessor;
class DCPContentType;
class Ratio;
class Cinema;

/** @class Config
 *  @brief A singleton class holding configuration.
 */
class Config : public boost::noncopyable
{
public:

	/** @return number of threads to use for J2K encoding on the local machine */
	int num_local_encoding_threads () const {
		return _num_local_encoding_threads;
	}

	boost::filesystem::path default_directory () const {
		return _default_directory;
	}

	boost::filesystem::path default_directory_or (boost::filesystem::path a) const;

	/** @return base port number to use for J2K encoding servers */
	int server_port_base () const {
		return _server_port_base;
	}

	void set_use_any_servers (bool u) {
		_use_any_servers = u;
	}

	bool use_any_servers () const {
		return _use_any_servers;
	}

	/** @param s New list of servers */
	void set_servers (std::vector<std::string> s) {
		_servers = s;
	}

	/** @return Host names / IP addresses of J2K encoding servers that should definitely be used */
	std::vector<std::string> servers () const {
		return _servers;
	}

	/** @return The IP address of a TMS that we can copy DCPs to */
	std::string tms_ip () const {
		return _tms_ip;
	}
	
	/** @return The path on a TMS that we should write DCPs to */
	std::string tms_path () const {
		return _tms_path;
	}

	/** @return User name to log into the TMS with */
	std::string tms_user () const {
		return _tms_user;
	}

	/** @return Password to log into the TMS with */
	std::string tms_password () const {
		return _tms_password;
	}

	/** @return The sound processor that we are using */
	SoundProcessor const * sound_processor () const {
		return _sound_processor;
	}

	std::list<boost::shared_ptr<Cinema> > cinemas () const {
		return _cinemas;
	}
	
	std::list<int> allowed_dcp_frame_rates () const {
		return _allowed_dcp_frame_rates;
	}
	
	DCIMetadata default_dci_metadata () const {
		return _default_dci_metadata;
	}

	boost::optional<std::string> language () const {
		return _language;
	}

	int default_still_length () const {
		return _default_still_length;
	}

	Ratio const * default_container () const {
		return _default_container;
	}

	DCPContentType const * default_dcp_content_type () const {
		return _default_dcp_content_type;
	}

	libdcp::XMLMetadata dcp_metadata () const {
		return _dcp_metadata;
	}

	int default_j2k_bandwidth () const {
		return _default_j2k_bandwidth;
	}

	std::vector<PresetColourConversion> colour_conversions () const {
		return _colour_conversions;
	}

	std::string mail_server () const {
		return _mail_server;
	}

	std::string kdm_from () const {
		return _kdm_from;
	}

	std::string kdm_email () const {
		return _kdm_email;
	}

	/** @param n New number of local encoding threads */
	void set_num_local_encoding_threads (int n) {
		_num_local_encoding_threads = n;
	}

	void set_default_directory (boost::filesystem::path d) {
		_default_directory = d;
	}

	/** @param p New server port */
	void set_server_port_base (int p) {
		_server_port_base = p;
	}

	void set_reference_scaler (Scaler const * s) {
		_reference_scaler = s;
	}
	
	void set_reference_filters (std::vector<Filter const *> const & f) {
		_reference_filters = f;
	}

	/** @param i IP address of a TMS that we can copy DCPs to */
	void set_tms_ip (std::string i) {
		_tms_ip = i;
	}

	/** @param p Path on a TMS that we should write DCPs to */
	void set_tms_path (std::string p) {
		_tms_path = p;
	}

	/** @param u User name to log into the TMS with */
	void set_tms_user (std::string u) {
		_tms_user = u;
	}

	/** @param p Password to log into the TMS with */
	void set_tms_password (std::string p) {
		_tms_password = p;
	}

	void add_cinema (boost::shared_ptr<Cinema> c) {
		_cinemas.push_back (c);
	}

	void remove_cinema (boost::shared_ptr<Cinema> c) {
		_cinemas.remove (c);
	}

	void set_allowed_dcp_frame_rates (std::list<int> const & r) {
		_allowed_dcp_frame_rates = r;
	}

	void set_default_dci_metadata (DCIMetadata d) {
		_default_dci_metadata = d;
	}

	void set_language (std::string l) {
		_language = l;
	}

	void unset_language () {
		_language = boost::none;
	}

	void set_default_still_length (int s) {
		_default_still_length = s;
	}

	void set_default_container (Ratio const * c) {
		_default_container = c;
	}

	void set_default_dcp_content_type (DCPContentType const * t) {
		_default_dcp_content_type = t;
	}

	void set_dcp_metadata (libdcp::XMLMetadata m) {
		_dcp_metadata = m;
	}

	void set_default_j2k_bandwidth (int b) {
		_default_j2k_bandwidth = b;
	}

	void set_colour_conversions (std::vector<PresetColourConversion> const & c) {
		_colour_conversions = c;
	}

	void set_mail_server (std::string s) {
		_mail_server = s;
	}

	void set_kdm_from (std::string f) {
		_kdm_from = f;
	}

	void set_kdm_email (std::string e) {
		_kdm_email = e;
	}
	
	void write () const;

	boost::filesystem::path signer_chain_directory () const;

	static Config* instance ();
	static void drop ();

private:
	Config ();
	boost::filesystem::path file (bool) const;
	void read ();
	void read_old_metadata ();

	/** number of threads to use for J2K encoding on the local machine */
	int _num_local_encoding_threads;
	/** default directory to put new films in */
	boost::filesystem::path _default_directory;
	/** base port number to use for J2K encoding servers;
	 *  this port and the one above it will be used.
	 */
	int _server_port_base;
	/** true to broadcast on the `any' address to look for servers */
	bool _use_any_servers;
	/** J2K encoding servers that should definitely be used */
	std::vector<std::string> _servers;
	/** Scaler to use for the "A" part of A/B comparisons */
	Scaler const * _reference_scaler;
	/** Filters to use for the "A" part of A/B comparisons */
	std::vector<Filter const *> _reference_filters;
	/** The IP address of a TMS that we can copy DCPs to */
	std::string _tms_ip;
	/** The path on a TMS that we should write DCPs to */
	std::string _tms_path;
	/** User name to log into the TMS with */
	std::string _tms_user;
	/** Password to log into the TMS with */
	std::string _tms_password;
	/** Our sound processor */
	SoundProcessor const * _sound_processor;
	std::list<int> _allowed_dcp_frame_rates;
	/** Default DCI metadata for newly-created Films */
	DCIMetadata _default_dci_metadata;
	boost::optional<std::string> _language;
	int _default_still_length;
	Ratio const * _default_container;
	DCPContentType const * _default_dcp_content_type;
	libdcp::XMLMetadata _dcp_metadata;
	int _default_j2k_bandwidth;
	std::vector<PresetColourConversion> _colour_conversions;
	std::list<boost::shared_ptr<Cinema> > _cinemas;
	std::string _mail_server;
	std::string _kdm_from;
	std::string _kdm_email;

	/** Singleton instance, or 0 */
	static Config* _instance;
};

#endif
