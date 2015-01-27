/// ----------------------------------------*- mode: C++; -*--
/// @file configuration.h
/// Handling of simple (key, value) configuration files
/// ----------------------------------------------------------
/// $Id: configuration.h 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/configuration.h $
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
#ifndef NATFW__CONFIG_H
#define NATFW__CONFIG_H

#include <fstream>
#include <map>
#include <list>
#include <exception>

#include "address.h"

namespace natfw {
  using protlib::hostaddress;


/**
 * An exception to be thrown if an invalid configuration is read.
 */
class config_error : public std::exception {
  public:
	config_error(const std::string &msg="Unspecified configuration error",
			int line=-1) throw () : msg(msg), line(line) { }
	virtual ~config_error() throw () { }

	std::string get_msg() const throw () { return msg; }
	int get_line() const throw () { return line; }

  private:
	std::string msg;
	int line;
};

inline std::ostream &operator<<(std::ostream &os, const config_error &err) {
	if ( err.get_line() > 0 )
		return os << err.get_msg() << " at line " << err.get_line();
	else
		return os << err.get_msg();
}


/**
 * The specification of a configuration entry.
 *
 * This specifies the name of the key, the value type, and optionally an
 * initial default value.
 */
class config_entry {
  public:
	enum type_t {
		T_BOOL, T_INT, T_FLOAT, T_STR, T_IPv4, T_IPv6,
		T_IPv4_LIST, T_IPv6_LIST, T_END
	};

	config_entry(std::string key, type_t type, bool required=true)
		: key(key), type(type), required(required) { }

	config_entry(std::string key, bool value)
		: key(key), type(T_BOOL), bool_value(value) { }

	config_entry(std::string key, int value)
		: key(key), type(T_INT), int_value(value) { }

	config_entry(std::string key, float value)
		: key(key), type(T_FLOAT), float_value(value) { }

	config_entry(std::string key, std::string value)
		: key(key), type(T_STR), str_value(value) { }

	config_entry() : type(T_END) { }

  private:
	std::string key;
	type_t type;
	bool required;
	bool defined;

	bool bool_value;
	int int_value;
	float float_value;
	std::string str_value;
	hostaddress ipv4_value;
	hostaddress ipv6_value;
	std::list<hostaddress> address_list; // for both IPv4 and IPv6

	friend class configuration;
};


/**
 * A class for handling simple configuration files.
 *
 * The configuration consists of (key, value) pairs, where both key and value
 * are strings.
 *
 * A configuration file is line-oriented and has the following format:
 *   [space] key [space] = [space] value [space] EOL
 *
 * Value can be a boolean value, an integer, a float, an IP address (either
 * IPv4 or IPv6), or a string. String values have to be quoted using double
 * quotes. If a double quote should appear in the string, you have to quote it
 * using a backslash. A backslash in turn has to be quoted using another
 * backslash.
 *
 * Lines starting with '#' and empty lines are ignored.
 */
class configuration {
  public:
	configuration(config_entry defaults[]);

	void load(const std::string &filename) throw (config_error);
	void load(std::istream &in) throw (config_error);
	void dump(std::ostream &out) throw (config_error);

	bool is_defined(const std::string &key) const throw ();

	std::string get_string(const std::string &key) const throw ();
	bool get_bool(const std::string &key) const throw ();
	int get_int(const std::string &key) const throw ();
	float get_float(const std::string &key) const throw ();
	hostaddress get_ipv4_address(const std::string &key) const throw ();
	hostaddress get_ipv6_address(const std::string &key) const throw ();

	std::list<hostaddress> get_ipv4_address_list(
		const std::string &key) const throw ();
	std::list<hostaddress> get_ipv6_address_list(
		const std::string &key) const throw ();

  private:
	typedef std::map<std::string, config_entry>::const_iterator c_iter;
	std::map<std::string, config_entry> values;

	void strip_leading_space(std::istream &in) const;
	void skip_rest_of_line(std::istream &in) const;
	void parse_and_assign(const std::string &key, std::istream &in);

	bool parse_bool(std::istream &in) const;
	int parse_int(std::istream &in) const;
	float parse_float(std::istream &in) const;
	std::string parse_string(std::istream &in) const;
	hostaddress parse_ipv4_address(std::istream &in) const;
	hostaddress parse_ipv6_address(std::istream &in) const;
	std::list<hostaddress> parse_ipv4_address_list(std::istream &in) const;
	std::list<hostaddress> parse_ipv6_address_list(std::istream &in) const;
	void write_string(std::ostream &out, const std::string &str) const;
	void dump_address_list(std::ostream &out,
		const std::list<hostaddress> &addresses) const;
};


} // namespace natfw

#endif // NATFW__CONFIG_H
