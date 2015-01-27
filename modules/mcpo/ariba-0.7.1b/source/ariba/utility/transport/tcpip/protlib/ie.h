/// ----------------------------------------*- mode: C++; -*--
/// @file ie.h
/// Basic information elements (PDUs and PDU objects)
/// ----------------------------------------------------------
/// $Id: ie.h 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/ie.h $
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

/** @ingroup protlib
 * @todo Use objectpool by deriving classes from class poolobject and
 * linking executables against objectpool.o or including objectpool.o into
 * libie.a.
 *
 * This header file defines the base class of all information elements, the
 * information elements for the protocol and an IE manager object.
 *
 * For performance only pointers are exchanged. The copy member function is
 * the way to duplicate an IE. It does much the same as a copy constructor
 * but returns not the IE but a pointer to it. If the IE contains pointers,
 * their target objects are copied too.
 */

#ifndef _PROTLIB__IE_H_
#define _PROTLIB__IE_H_

#include <boost/unordered_map.hpp>
#include <deque>
#include <string>
#include <iostream>
#include <map>

#include "protlib_types.h"
#include "network_message.h"

using boost::unordered_map;

namespace protlib {



/** @addtogroup protlib
 * @{
 */

/// Catch bad_alloc and call throw_nomem_error
#define catch_bad_alloc(x) try { x; } catch(bad_alloc) { throw_nomem_error(); }

// forward declarations
class IEManager;
class IEErrorList;
class IEError;

/** Abstract Information Element (IE) interface
 */
class IE {
public:
	virtual ~IE() { }

	friend class IEManager;
	/// IE coding sheme
	/** All coding shemes have to be listed here. Each IE should support at
	 * least one listed coding sheme. This is used when serializing or
	 * deserializing from a NetMsg object to (de)code the IE in the right way.
	 * Note that coding schemes and protocol version are not the same.
	 * There could also be a coding sheme to (de)code IEs e.g. for router
	 * configuration.
	 */
	enum coding_t {
	  nslp_v1     = 1,
	  protocol_v1 = 1,
	  protocol_v2 = 2,
	  nslp_v2     = 3,
	  nslp_v2_yoda = 4
	}; // end coding_t
protected:
	/// contructor
	IE(uint16 category);
	/// copy constructor
	IE(const IE& n);
public:
	/// get a new instance of the IE
	virtual IE* new_instance() const = 0;
	/// copy an IE
	virtual IE* copy() const = 0;
	/// deserialization
	virtual IE* deserialize(NetMsg& msg, coding_t coding, IEErrorList& errorlist, uint32& bread, bool skip = true) = 0;
	/// serialize
	virtual void serialize(NetMsg& msg, coding_t coding, uint32& wbytes) const = 0;
protected:
	/// check arguments for deserialization
	bool check_deser_args(coding_t cod, IEErrorList& errorlist, uint32 &bread) const;
	/// check arguments for serialization
	void check_ser_args(coding_t cod, uint32 &wbytes) const;
public:
	/// IE consistency check
	virtual bool check() const = 0;
	/// check if IE supports coding sheme
	virtual bool supports_coding(coding_t c) const = 0;
	/// IE serialized size
	virtual size_t get_serialized_size(coding_t coding) const = 0;
	/// get category
	uint16 get_category() const;
	/// equality
	virtual bool operator==(const IE& ie) const = 0;
	/// not equal
	inline bool operator!=(const IE& ie) const { return (!(*this==ie)); }
	/// get IE name
	/** This is mainly for error reporting. */
	virtual const char* get_ie_name() const = 0;
	/// print to a ostream
	virtual ostream& print(ostream& os, uint32 level, const uint32 indent, const char* name = NULL) const;
	/// get a string representing the IE content
	string to_string(const char* name = 0) const;
	/// input from an istream
	/** Attention: No checking, no warnings. This is ONLY for testing. */
	virtual istream& input(istream& is, bool istty, uint32 level, const uint32 indent, const char* name = NULL);
	/// clear all pointer fields
	virtual void clear_pointers();
protected:
	/// register this IE
	virtual void register_ie(IEManager *iem) const = 0;
	/// IE category
	const uint16 category;
	/// throw a NO_MEM exception
	void throw_nomem_error() const;
}; // end class IE



/**
 * IE Error base class.
 *
 * This is the base for all exceptions thrown during serialize() and
 * deserialize().
 *
 * @warning New code should use the exceptions derived from this class.
 * This class should be abstract but isn't for compatibility reasons.
 */
class IEError : public ProtLibException {
  public:
	/// IE error code - common parsing errors
	enum error_t {
		ERROR_REGISTER,
		ERROR_CODING,
		ERROR_CATEGORY,
		ERROR_NO_IEMANAGER,
		ERROR_MSG_TOO_SHORT,
		ERROR_INVALID_STATE,
		ERROR_WRONG_TYPE,
		ERROR_WRONG_SUBTYPE,
		ERROR_WRONG_LENGTH,
		ERROR_NO_MEM,
		ERROR_TOO_BIG_FOR_IMPL,
		ERROR_UNKNOWN_ERRORCODE,
		ERROR_WRONG_VERSION,
		ERROR_UNEXPECTED_OBJECT,
		ERROR_PDU_SYNTAX,
		ERROR_OBJ_SET_FAILED,
		ERROR_PROT_SPECIFIC
	}; // end error_t

	const error_t err;

	IEError(error_t e);
	virtual ~IEError() throw ();

	virtual const char *getstr() const;

  protected:
	IEError(std::string msg) throw ();

  private:
	static const char* err_str[];
};


/**
 * Syntax error during deserialization.
 *
 * This exception is thrown if a syntax error is detected during
 * deserialization.
 */
class PDUSyntaxError : public IEError {
  public:
	PDUSyntaxError(const char* msg); // deprecated!
	PDUSyntaxError(IE::coding_t coding, uint16 category, uint16 type,
			uint16 subtype, uint32 pos, const char *msg = "");
	virtual ~PDUSyntaxError() throw () { }

	const IE::coding_t coding;
	const uint16 category;
	const uint16 type;
	const uint16 subtype;
	const uint32 errorpos;
	const string message;
};


/**
 * NetMsg is too short to serialize/deserialize.
 *
 * This exception is typically thrown if a NetMsg ended unexpectedly
 * which indicates a truncated message. It may also be thrown if a
 * NetMsg is too short to hold a serialized IE object.
 */
class IEMsgTooShort : public IEError {
  public:
	IEMsgTooShort(IE::coding_t coding, uint16 category, uint32 pos);
	virtual ~IEMsgTooShort() throw () { }

	const IE::coding_t coding;
	const uint16 category;
	const uint32 errorpos;
};


/**
 * Wrong protocol version.
 *
 * This exception is thrown in case of a version conflict, like a
 * deserialize() method not being able to handle a coding.
 */
class IEWrongVersion : public IEError {
  public:
	IEWrongVersion(IE::coding_t coding, uint16 category, uint32 pos);
	virtual ~IEWrongVersion() throw () { }

	const IE::coding_t coding;
	const uint16 category;
	const uint32 errorpos;
};


/**
 * Invalid IE type.
 *
 * Typically thrown if deserialize() fails because there is no matching IE
 * registered with the IEManager.
 */
class IEWrongType : public IEError {
  public:
	// The first constructor is deprecated!
	IEWrongType(IE::coding_t coding, uint16 category, uint32 pos);
	IEWrongType(IE::coding_t coding, uint16 category, uint16 type,
			uint32 pos);
	virtual ~IEWrongType() throw () { }

	const IE::coding_t coding;
	const uint16 category;
	const uint16 type;
	const uint32 errorpos;
};


/**
 * Invalid IE subtype.
 *
 * Typically thrown if deserialize() fails because there is no matching IE
 * registered with the IEManager.
 */
class IEWrongSubtype : public IEError {
  public:
	// The first constructor is deprecated!
	IEWrongSubtype(IE::coding_t coding, uint16 category, uint16 type,
			uint32 pos);
	IEWrongSubtype(IE::coding_t coding, uint16 category, uint16 type,
			uint16 subtype, uint32 pos);
	virtual ~IEWrongSubtype() throw () { }

	const IE::coding_t coding;
	const uint16 category;
	const uint16 type;
	const uint16 subtype;
	const uint32 errorpos;
};


/**
 * An invalid length field was read.
 *
 * This exception is thrown if a length field inside a NetMsg has a wrong
 * length (either too long or too short, but makes no sense). Note that
 * this overlaps with IEMsgTooShort in some cases.
 */
class IEWrongLength : public IEError {
  public:
	IEWrongLength(IE::coding_t coding, uint16 category, uint16 type,
			uint16 subtype, uint32 pos);
	virtual ~IEWrongLength() throw () { }

	const IE::coding_t coding;
	const uint16 category;
	const uint16 type;
	const uint16 subtype;
	const uint32 errorpos;
};


/**
 * Some entity is too big for this implementation.
 *
 * This exception is thrown if an operation can't be performed because
 * of some implementation limit.
 */
class IETooBigForImpl : public IEError {
  public:
	IETooBigForImpl(IE::coding_t coding, uint16 category, uint32 pos);
	virtual ~IETooBigForImpl() throw () { }

	const IE::coding_t coding;
	const uint16 category;
	const uint32 errorpos;
};


/**
 * Protocol Specific Error
 *
 * This exception is thrown if a protocol specific error
 * occurred.
 */
class IEProtocolSpecific : public IEError {
  public:
	IEProtocolSpecific(IE::coding_t coding, uint16 category, uint32 pos);
	virtual ~IEProtocolSpecific() throw () { }

	const IE::coding_t coding;
	const uint16 category;
	const uint32 errorpos;
};


/// IE error list
/** This represents a list of IE errors. */
class IEErrorList {
public:
	/// put an IEError on the list
	void put(IEError* iee);
	/// get and remove next IEError
	IEError* get();
	/// is list empty
	bool is_empty() const;
	/// clear list
	void clear();
	/// destructor
	~IEErrorList();
private:
	/// internal queue type
	typedef deque<IEError*> queue_t;
	/// queue iterator type
	typedef queue_t::iterator queueit_t;
	/// IEError storage
	queue_t queue;
}; // end IEErrorList


/**
 * Represents a (category, type, subtype) tuple.
 *
 * Used internally by IEManager.
 */
class IE_Key {
  public:
	inline IE_Key(uint16 category, uint16 type, uint16 subtype)
		: category(category), type(type), subtype(subtype) { }

	inline bool operator==(const IE_Key &other) const {
		return category == other.category && type == other.type
			&& subtype == other.subtype;
	}

	inline uint16 get_category() const { return category; }
	inline uint16 get_type() const { return type; }
	inline uint16 get_subtype() const { return subtype; }

  private:
	uint16 category;
	uint16 type;
	uint16 subtype;
};

struct hash_IE_Key {
	inline size_t operator()(const protlib::IE_Key &k) const {
		return (k.get_category() << 16)
			| ( k.get_type() ^ k.get_subtype() );
	}
};


/** 
 * A registry and factory for IEs.
 *
 * Each IE has to register at the IE Manager. The IEManager then provides
 * methods to deserialize IEs from or serialize IEs to a NetMsg object.
 * IEManager is abstract and thus can't be instantiated.
 *
 * In contrast to earlier implementations, IEManager itself is no longer a
 * singleton. Inheriting from singletons is a tricky business in C++ (and
 * much easier in Java) because static methods can't be virtual. Because
 * of this, multiple badly implemented child classes were unable to coexist
 * in a single program.
 *
 * The following has to be done in each protocol to use IEManager and
 * create a protocol-specific singleton (called PROT_IEManager):
 *
 *   - Inherit from IEManager.
 *   - Make the constructor private.
 *   - Provide a "private: static PROT_IEManager *inst" attribute.
 *   - Provide a "public: static PROT_IEManager *instance()" method.
 *   - Provide a "public: static void clear()" method.
 *   - Define your own category_t with categories appropriate for your protocol.
 * 
 * An example implementation can be found in the QSPEC code. Please note that
 * old code (r286 and earlier) won't compile because of interface changes.
 * The ability to register "default" IEs that are returned if no more specific
 * IE is found has been removed because it depended on coding_t. If derived
 * IEManagers need this functionality, they have to override lookup_ie().
 * 
 */
class IEManager {

  public:
	virtual ~IEManager();

	virtual void register_ie(const IE *ie);
	virtual void register_ie(uint16 category, uint16 type, uint16 subtype,
			const IE *ie);

	virtual IE *new_instance(uint16 category, uint16 type, uint16 subtype);

	virtual void serialize(IE &ie, NetMsg &msg, IE::coding_t coding,
			uint32 &bytes_written) const;

	virtual IE *deserialize(NetMsg &msg, uint16 category,
			IE::coding_t coding, IEErrorList& errorlist,
			uint32 &bytes_read, bool skip = true) = 0;

  protected:
	IEManager();

	virtual IE *lookup_ie(uint16 category, uint16 type, uint16 subtype);

	virtual void throw_nomem_error() const;

  private:
	typedef unordered_map<IE_Key, IE *, hash_IE_Key> category_map_t;

	category_map_t registry;
};


/** This mainly calls the serialize member function of the given IE.
 * @param ie pointer to an IE
 * @param msg NetMsg object that will hold the serialized IE.
 * @param coding IE coding sheme
 * @param wbytes number of output bytes
 */
inline
void IEManager::serialize(IE& ie, NetMsg& msg, IE::coding_t coding, uint32& wbytes) const {
	ie.serialize(msg,coding,wbytes);
} // end serialize


/// print an IE to an ostream
ostream& operator<<(ostream& os, const IE& ie);

/// input operator
istream& operator>>(istream& is, IE& ie);

/// round uint32 up
/** Adds up the given uint32 to fit a 32-bit or 4-byte border. */
inline uint32 round_up4(uint32 i) {
	if (i%4) i = i-(i%4)+4;
	return i;
} // end round_up4

//@}

} // end namespace protlib

#endif // _PROTLIB__IE_H_
