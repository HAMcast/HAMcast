/// ----------------------------------------*- mode: C++; -*--
/// @file configuration.cpp
/// A configuration file parser
/// ----------------------------------------------------------
/// $Id: configuration.cpp 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/configuration.cpp $
// ===========================================================
//                      
// Copyright (C) 2005-2007, all rights reserved by
// - Institute of Telematics, Universitaet Karlsruhe (TH)
//
// More information and contact:
// https://projekte.tm.uka.de/trac/NSIS
//                      
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// ===========================================================
#include <iostream>
#include <sstream>

#include "configuration.h"


using namespace natfw;


// only used internally
class parse_error : public std::exception {
  public:
        parse_error(const std::string &msg) throw () : msg(msg) { }
	virtual ~parse_error() throw () { }

        std::string get_msg() const throw () { return msg; }

  private:
        std::string msg;
};


/**
 * Constructor.
 */
configuration::configuration(config_entry rules[]) {
	for (unsigned i=0; rules[i].type != config_entry::T_END; i++)
		values[ rules[i].key ] = rules[i];
}


/**
 * Load a configuration file.
 *
 * If there's a parse error or the file can't be opened, a config_error
 * exception is thrown.
 *
 * @param filename the file to load
 */
void configuration::load(const std::string &filename) throw (config_error) {

	std::ifstream in(filename.c_str());

	if ( ! in )
		throw config_error("cannot open file `" + filename + "'");

	try {
		this->load(in);
	}
	catch ( config_error &e ) {
		in.close();
		throw;
	}
	catch ( ... ) {
		in.close();
		throw config_error("unknown exception thrown");
	}

	in.close();
}


/**
 * Load configuration data from a stream.
 *
 * If there is a parse error, a config_error exception is thrown. This method
 * will read until end of file. It is up to the caller to close the stream.
 *
 * @param in_stream the input stream to read data from
 */
void configuration::load(std::istream &in_stream) throw (config_error) {
	using namespace std;

	for (int line = 1; in_stream; line++) {
		string buf;
		string key;

		getline(in_stream, buf);

		stringstream in(buf);

		// skip leading whitespace
		strip_leading_space(in);

		// skip empty lines and comments
		if ( in.peek() == -1 || in.peek() == '#' )
			continue;

		// read the key
		in >> key;
		if ( key == "")
			throw config_error("parse error", line);

		if ( values.find(key) == values.end() )
			throw config_error("invalid key `" + key + "'", line);

		// skip space between key and '='
		strip_leading_space(in);

		char c = in.get();
		if ( c != '=' )
			throw config_error("parse error", line);

		// skip space between '=' and value
		strip_leading_space(in);

		// no value for this key, we ignore it altogether
		if ( in.peek() == -1 )
			continue;

		try {
			parse_and_assign(key, in);
		}
		catch ( parse_error &e ) {
			throw config_error(e.get_msg(), line);
		}
	}

	if ( ! in_stream.eof() )
		throw config_error("stream error");

	// check if all required config settings are set.
	for ( c_iter i = values.begin(); i != values.end(); i++ ) {
		const config_entry &e = i->second;

		if ( e.required && ! e.defined )
			throw config_error(
				"key `" + e.key + "' required but not set");
	}
}


/**
 * Write the configuration data to a stream.
 *
 * If there is a write error, a config_error exception is thrown. This method
 * doesn't close the stream after writing. It is up to the caller to do that.
 *
 * @param out the output stream to read data from
 */
void configuration::dump(std::ostream &out) throw (config_error) {
	using namespace std;

	out << "# Configuration dump" << endl;

	for ( c_iter i = values.begin(); i != values.end(); i++ ) {
		const config_entry &e = i->second;

		out << e.key << " = ";

		if ( ! e.defined ) {
			out << endl;
			continue;
		}

		switch ( e.type ) {
			case config_entry::T_BOOL:
				out << e.bool_value << endl;
				break;
			case config_entry::T_INT:
				out << e.int_value << endl;
				break;
			case config_entry::T_FLOAT:
				out << e.float_value << endl;
				break;
			case config_entry::T_STR:
				write_string(out, e.str_value);
				out << endl;
				break;
			case config_entry::T_IPv4:
				out << e.ipv4_value << endl;
				break;
			case config_entry::T_IPv6:
				out << e.ipv6_value << endl;
				break;
			case config_entry::T_IPv4_LIST: // fall-through
			case config_entry::T_IPv6_LIST:
				dump_address_list(out, e.address_list);
				out << endl;
				break;
			default:
				assert( false );
		}
	}
}


void configuration::dump_address_list(std::ostream &out,
		const std::list<hostaddress> &addresses) const {

	typedef std::list<hostaddress>::const_iterator addr_iter;

	for ( addr_iter i = addresses.begin(); i != addresses.end(); i++ )
		out << *i << " ";
}


/**
 * Test if the configuration contains the given value.
 *
 * @param key the name of the key
 * @return true, if the configuraiton has that key
 */
bool configuration::is_defined(const std::string &key) const throw () {
	c_iter i = values.find(key);

	if ( values.find(key) == values.end() )
		return false;
	else
		return i->second.defined;
}


/**
 * Get a string configuration value.
 *
 * @param key the name of the key
 * @return the value from the configuration
 */
std::string configuration::get_string(const std::string &key) const throw () {
	c_iter i = values.find(key);
	assert( i != values.end() );
	assert( i->second.type == config_entry::T_STR );

	return i->second.str_value;
}


/**
 * Get a boolean configuration value.
 *
 * @param key the name of the key
 * @return the value from the configuration
 */
bool configuration::get_bool(const std::string &key) const throw () {
	c_iter i = values.find(key);
	assert( i != values.end() );
	assert( i->second.type == config_entry::T_BOOL );

	return i->second.bool_value;
}


/**
 * Get an integer configuration value.
 *
 * @param key the name of the key
 * @return the value from the configuration
 */
int configuration::get_int(const std::string &key) const throw () {
	c_iter i = values.find(key);
	assert( i != values.end() );
	assert( i->second.type == config_entry::T_INT );

	return i->second.int_value;
}


/**
 * Get a floating point configuration value.
 *
 * @param key the name of the key
 * @return the value from the configuration
 */
float configuration::get_float(const std::string &key) const throw () {
	c_iter i = values.find(key);
	assert( i != values.end() );
	assert( i->second.type == config_entry::T_FLOAT );

	return i->second.float_value;
}


/**
 * Get an IPv4 hostaddress configuration value.
 *
 * @param key the name of the key
 * @return the value from the configuration
 */
hostaddress configuration::get_ipv4_address(
		const std::string &key) const throw () {

	c_iter i = values.find(key);
	assert( i != values.end() );
	assert( i->second.type == config_entry::T_IPv4 );

	return i->second.ipv4_value;
}


/**
 * Get an IPv6 hostaddress configuration value.
 *
 * @param key the name of the key
 * @return the value from the configuration
 */
hostaddress configuration::get_ipv6_address(
		const std::string &key) const throw () {

	c_iter i = values.find(key);
	assert( i != values.end() );
	assert( i->second.type == config_entry::T_IPv6 );

	return i->second.ipv6_value;
}


/**
 * Get a list of IPv4 hostaddress objects.
 *
 * @param key the name of the key
 * @return the list of values from the configuration
 */
std::list<hostaddress> configuration::get_ipv4_address_list(
		const std::string &key) const throw () {

	c_iter i = values.find(key);
	assert( i != values.end() );
	assert( i->second.type == config_entry::T_IPv4_LIST );

	return i->second.address_list;
}


/**
 * Get a list of IPv6 hostaddress objects.
 *
 * @param key the name of the key
 * @return the list of values from the configuration
 */
std::list<hostaddress> configuration::get_ipv6_address_list(
		const std::string &key) const throw () {

	c_iter i = values.find(key);
	assert( i != values.end() );
	assert( i->second.type == config_entry::T_IPv6_LIST );

	return i->second.address_list;
}


void configuration::strip_leading_space(std::istream &in) const {
	while ( in && ( in.peek() == ' ' || in.peek() == '\t' ) )
		in.get();
}


// Parse the given buffer and assign the value to the config entry
void configuration::parse_and_assign(const std::string &key, std::istream &in) {

	switch ( values[key].type ) {
		case config_entry::T_BOOL:
			values[key].bool_value = parse_bool(in);
			break;
		case config_entry::T_INT:
			values[key].int_value = parse_int(in);
			break;
		case config_entry::T_FLOAT:
			values[key].float_value = parse_float(in);
			break;
		case config_entry::T_STR:
			values[key].str_value = parse_string(in);
			break;
		case config_entry::T_IPv4:
			values[key].ipv4_value = parse_ipv4_address(in);
			break;
		case config_entry::T_IPv6:
			values[key].ipv6_value = parse_ipv6_address(in);
			break;
		case config_entry::T_IPv4_LIST:
			values[key].address_list = parse_ipv4_address_list(in);
			break;
		case config_entry::T_IPv6_LIST:
			values[key].address_list = parse_ipv6_address_list(in);
			break;
		default:
			assert( false );
			throw parse_error("invalid value"); // not reached
	}

	// no exception thrown until now, so parsing was successful
	values[key].defined = true;
}


// Write the string to the stream, adding quotation marks and escape sequences
void configuration::write_string(
		std::ostream &out, const std::string &str) const {

	std::stringstream stream(str);

	out << '"';

	char c;
	while ( stream.get(c) ) {
		switch ( c ) {
			case '\\':	// fallthrough
			case '"':	out << '\\' << c; break;
			default:	out << c;
		}
	}

	out << '"';
}


// Matches pattern "[^"]*"\s*
std::string configuration::parse_string(std::istream &in) const {

	if ( in.get() != '"' )
		throw parse_error("string doesn't start with a quotation mark");

	bool escape = false;
	std::string tmp;
	char c;

	while ( in.get(c) ) {
		if ( escape ) {
			if ( c == '\\' || c == '"' )
				tmp += c;
			else
				throw parse_error("invalid escape sequence");

			escape = false;
		}
		else {
			if ( c == '"' )
				break;
			else if ( c == '\\' )
				escape = true;
			else
				tmp += c;
		}
	}

	// we should be on the closing quotation mark
	if ( c != '"' )
		throw parse_error("unterminated string");

	skip_rest_of_line(in);

	return tmp;
}


hostaddress configuration::parse_ipv4_address(std::istream &in) const {
	std::string word;
	in >> word;

	bool success;
	hostaddress addr(word.c_str(), &success);

	if ( success || ! addr.is_ipv4() )
		return addr;
	else
		throw parse_error("invalid IPv4 address");

	skip_rest_of_line(in);

	return addr;
}


hostaddress configuration::parse_ipv6_address(std::istream &in) const {
	std::string word;
	in >> word;

	bool success;
	hostaddress addr(word.c_str(), &success);

	if ( success || ! addr.is_ipv6() )
		return addr;
	else
		throw parse_error("invalid IPv6 address");

	skip_rest_of_line(in);

	return addr;
}

std::list<hostaddress> configuration::parse_ipv4_address_list(
		std::istream &in) const {

	std::list<hostaddress> result;

	std::string tmp;
	while ( in >> tmp ) {
		bool success;
		hostaddress addr(tmp.c_str(), &success);

		if ( success && addr.is_ipv4() )
			result.push_back(addr);
		else
			throw parse_error("invalid IPv4 address `" + tmp + "'");
	}

	return result;
}

std::list<hostaddress> configuration::parse_ipv6_address_list(
		std::istream &in) const {

	std::list<hostaddress> result;

	std::string tmp;
	while ( in >> tmp ) {
		bool success;
		hostaddress addr(tmp.c_str(), &success);

		if ( success && addr.is_ipv6() )
			result.push_back(addr);
		else
			throw parse_error("invalid IPv6 address `" + tmp + "'");
	}

	return result;
}

int configuration::parse_int(std::istream &in) const {
	int tmp = -1;

	in >> tmp;

	if ( ! in.good() && ! in.eof() )
		throw parse_error("parsing integer failed");

	skip_rest_of_line(in);

	return tmp;
}


float configuration::parse_float(std::istream &in) const {
	float tmp = 0.0;

	in >> tmp;

	if ( ! in.good() && ! in.eof() )
		throw parse_error("parsing float failed");

	skip_rest_of_line(in);

	return tmp;
}


bool configuration::parse_bool(std::istream &in) const {
	std::string tmp;
	bool value;

	in >> tmp;

	if ( tmp == "true" )
		value = true;
	else if ( tmp == "false" )
		value = false;
	else
		throw parse_error("parsing boolean failed");

	skip_rest_of_line(in);

	return value;
}


// throw an exception if the rest of the line doesn't only contain whitespace
void configuration::skip_rest_of_line(std::istream &in) const {
	char c;
	while ( in.get(c) ) {
		if ( c != ' ' && c != '\t' )
			throw parse_error("junk after value");
	}
}


// EOF
