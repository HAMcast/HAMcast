/// ----------------------------------------*- mode: C++; -*--
/// @file ie.cpp
/// information elements for the protocol, IE manager singleton
/// ----------------------------------------------------------
/// $Id: ie.cpp 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/ie.cpp $
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
 * This file defines the base class of all information elements, the
 * information elements for the GIST protocol and an IE manager singleton
 * object.
 * Although the IEs are closely related to the structure of GIST messages,
 * they may be used in other contexts as well because of the coding shemes.
 */

#include "ie.h"
#include "logfile.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iomanip>

namespace protlib {
  using namespace log;



/** @addtogroup protlib
 * @{
 */

/***** class IE *****/

/** Constructor. Sets category of IE. */
IE::IE(uint16 category) : category(category) {}

IE::IE(const IE& n) : category(n.category) {}

/** Get the category of the IE. */
uint16 IE::get_category() const {
	return category;
} // end get_category

/** Check arguments of IE deserialization member functions.
 * Additionally, bread is set to 0.
 */ 
bool IE::check_deser_args(coding_t cod, IEErrorList& errorlist, uint32 &bread) const {
	bread = 0;
	// check coding
	if (!supports_coding(cod)) {
	  Log(ERROR_LOG,LOG_NORMAL, "IE", "IE::check_deser_args(): IE " << get_ie_name() << " does not support coding " << cod );
	  catch_bad_alloc(errorlist.put(new IEError(IEError::ERROR_CODING)));
	  return false;
	} // end if cod
	return true;
} // end check_deser_args()

/** Check arguments of IE serialization member functions.
 * Additionally, wbytes is set to 0.
 */ 
void IE::check_ser_args(coding_t cod, uint32 &wbytes) const {
	wbytes = 0;
	// check IE state
	if (!check()) {
		IEError err(IEError::ERROR_INVALID_STATE);
		Log(ERROR_LOG,LOG_NORMAL, "IE", "IE::check_ser_args() for IE " << get_ie_name() << ", error: " << err.getstr());
		throw err;
	} // end if !check()
	// check coding
	if (!supports_coding(cod)) {
		IEError err(IEError::ERROR_CODING);
		Log(ERROR_LOG,LOG_NORMAL, "IE", "IE::check_ser_args() for IE " << get_ie_name() << ", error: " << err.getstr() << ", coding: " << cod);
		throw err;
	} // end if cod
	return;
} // end check_ser_args()


/** Print the content of the IE, default behaviour is to print its name.
 * Level and indent control how much space is inserted after a newline.
 * If name points to a string, this string is printed before the
* get_ie_name().
 */
ostream& IE::print(ostream& os, uint32 level, const uint32 indent, const char* name) const {
	os<<setw(level*indent)<<"";
	if (name && (*name!=0)) os<<name<<" ";
	os<<"<"<<get_ie_name()<<">";
	return os;
} // end print

istream& IE::input(istream& is, bool istty, uint32 level, const uint32 indent, const char* name) {
  Log(ERROR_LOG,LOG_NORMAL, "IE", "IE " << get_ie_name() << "::input not yet implemented");
  if (istty) {
    cout<<setw(level*indent)<<"";
    if (name && (*name!=0)) cout<<name<<" ";
    cout<<"<"<<get_ie_name()<<">: Input not yet implemented."<<endl;
  } // end if istty
  return is;
} // end input

/** Print the IE to a string using print() and stringstream. */
string IE::to_string(const char* name) const {
	ostringstream os;
	print(os,0,3,name);
	return os.str();
} // end to_string

/** Log and throw a nomem_error */
void IE::throw_nomem_error() const {
	try {
	  Log(ERROR_LOG,LOG_CRIT, "IE", "Not enough memory for IE " << get_ie_name());
	} catch(...) {}
	throw IEError(IEError::ERROR_NO_MEM);
} // end throw_nomem_error

/** Set all pointers to external data to NULL.
 * By default this does nothing because there are no pointers.
 */
void IE::clear_pointers() {}


/***** class IEError *****/

/// IEError strings
const char* IEError::err_str[] = {
	"Error while registering IE. Either pointer to IE is NULL or registered IE already.",
	"IE coding scheme is not supported by this function.",
	"IE category is not supported by this function or in the applied coding scheme.",
	"IE Manager instance does not exist (possible mem alloc problem?)",
	"NetMsg too short for (de)serialization.",
	"IE is in invalid state.",
	"Wrong/unexpected IE type.",
	"Wrong/unexpected IE subtype.",
	"Wrong IE length.",
	"Not enough memory to allocate IE.",
	"Too big for this protocol implementation.",
	"Unknown error code for this error class.",
	"Wrong or unknown protocol version.",
	"Unexpected object",
	"PDU syntax error",
	"PDU Object set failed (wrong index)",
	"Protocol Specific Error"
};


/**
 * Constructor.
 *
 * Initialize an IEError object by setting the error code.
 *
 * @warning Don't instantiate IEError. Use a child class instead!
 *
 * @param error the kind of error this exception represents
 */
IEError::IEError(error_t error) : ProtLibException(err_str[error]), err(error) {
	// nothing to do
}


IEError::IEError(std::string msg) throw ()
		: ProtLibException(msg), err(ERROR_UNKNOWN_ERRORCODE) {

	// nothing to do
}


/**
 * Destructor.
 *
 * This has only been defined to prevent compiler warnings.
 */
IEError::~IEError() throw () {
	// nothing to do
}


/**
 * Returns an error message.
 *
 * Note: what() returns more useful messages.
 *
 * @return the error message
 */
const char* IEError::getstr() const {
	return err_str[err];
}


/***** class PDUSyntaxError *****/

/**
 * Constructor.
 *
 * @deprecated This constructor shouldn't be used for new code.
 *
 * @param msg an error message describing the problem
 */
PDUSyntaxError::PDUSyntaxError(const char* msg)
		: IEError(IEError::ERROR_PDU_SYNTAX),
		  coding(coding), category(0), type(0), subtype(0),
		  errorpos(0), message(msg) {

	ostringstream ost;
	ost << "[coding " << coding << ", cat " << category << ", type " << type
		<< ", subtype " << subtype << "] " << message;

	error_msg = ost.str();
}


/**
 * Constructor.
 *
 * @param coding the protocol version
 * @param category the IE's category
 * @param type the IE's type
 * @param subtype the IE's subtype
 * @param pos the position in the NetMsg at which the problem was discovered
 * @param msg a message describing the problem
 */
PDUSyntaxError::PDUSyntaxError(IE::coding_t coding, uint16 category,
		uint16 type, uint16 subtype, uint32 pos, const char *msg)
		: IEError(IEError::ERROR_PDU_SYNTAX),
		  coding(coding), category(category), type(type),
		  subtype(subtype), errorpos(pos), message(msg) {

	ostringstream ost;
	ost << "[coding " << coding << ", cat " << category << ", type " << type
		<< ", subtype " << subtype << "] " << message;

	error_msg = ost.str();
}


/***** class IEMsgTooShort *****/

/**
 * Constructor.
 *
 * @param coding the protocol version
 * @param category the IE's category
 * @param pos the position in the NetMsg at which the problem was discovered
 */
IEMsgTooShort::IEMsgTooShort(IE::coding_t coding, uint16 category, uint32 pos) 
		: IEError(ERROR_MSG_TOO_SHORT),
		  coding(coding), category(category), errorpos(pos) {

	ostringstream ost;
	ost << "[coding " << coding << ", cat " << category
		<< ", pos" << errorpos << IEError::getstr();

	error_msg = ost.str();
}


/***** class IEWrongVersion *****/

/**
 * Constructor.
 *
 * @param coding the protocol version
 * @param category the IE's category
 * @param pos the position in the NetMsg at which the problem was discovered
 */
IEWrongVersion::IEWrongVersion(IE::coding_t coding, uint16 category, uint32 pos)
		: IEError(ERROR_WRONG_VERSION),
		  coding(coding), category(category), errorpos(pos) {

	ostringstream ost;
	ost << "[coding " << coding << ", cat " << category
		<< ", pos" << errorpos << IEError::getstr();

	error_msg = ost.str();
}


/***** class IEWrongType *****/

/**
 * Constructor.
 *
 * @deprecated This constructor should not be used for new code.
 *
 * @param coding the protocol version
 * @param category the IE's category
 * @param pos the position in the NetMsg at which the problem was discovered
 */
IEWrongType::IEWrongType(IE::coding_t coding, uint16 category, uint32 pos) 
		: IEError(ERROR_WRONG_TYPE), coding(coding),
		  category(category), type(0), errorpos(pos) {

	ostringstream ost;
	ost << "[coding " << coding << ", cat " << category << ", type " << type
		<< ", pos" << errorpos << IEError::getstr();

	error_msg = ost.str();
}


/**
 * Constructor.
 *
 * @param coding the protocol version
 * @param category the IE's category
 * @param type the IE's type
 * @param pos the position in the NetMsg at which the problem was discovered
 */
IEWrongType::IEWrongType(IE::coding_t coding, uint16 category, uint16 type,
		uint32 pos) 
		: IEError(ERROR_WRONG_TYPE), coding(coding),
		  category(category), type(type), errorpos(pos) {

	ostringstream ost;
	ost << "[coding " << coding << ", cat " << category << ", type " << type
		<< ", pos" << errorpos << IEError::getstr();

	error_msg = ost.str();
}


/***** class IEWrongSubtype *****/

/**
 * Constructor.
 *
 * @deprecated This constructor should not be used for new code.
 *
 * @param coding the protocol version
 * @param category the IE's category
 * @param type the IE's type
 * @param pos the position in the NetMsg at which the problem was discovered
 */
IEWrongSubtype::IEWrongSubtype(IE::coding_t coding, uint16 category,
		uint16 type, uint32 pos) 
		: IEError(ERROR_WRONG_SUBTYPE), coding(coding),
		  category(category), type(type), subtype(0), errorpos(pos) {

	ostringstream ost;
	ost << "[coding " << coding << ", cat " << category << ", type " << type
		<< ", subtype " << subtype << ", pos" << errorpos
		<< IEError::getstr();

	error_msg = ost.str();
}


/**
 * Constructor.
 *
 * @param coding the protocol version
 * @param category the IE's category
 * @param type the IE's type
 * @param subtype the IE's subtype
 * @param pos the position in the NetMsg at which the problem was discovered
 */
IEWrongSubtype::IEWrongSubtype(IE::coding_t coding, uint16 category,
		uint16 type, uint16 subtype, uint32 pos) 
		: IEError(ERROR_WRONG_SUBTYPE),
		  coding(coding), category(category), type(type),
		  subtype(subtype), errorpos(pos) {

	ostringstream ost;
	ost << "[coding " << coding << ", cat " << category << ", type " << type
		<< ", subtype " << subtype << ", pos" << errorpos
		<< IEError::getstr();

	error_msg = ost.str();
}


/***** class IEWrongLength *****/

/**
 * Constructor.
 *
 * The category, type and subtype parameters refer to the IE throwing the
 * exception.
 *
 * @param coding the protocol version
 * @param category the IE's category
 * @param type the IE's type
 * @param subtype the IE's subtype
 * @param pos the position in the NetMsg at which the problem was discovered
 */
IEWrongLength::IEWrongLength(IE::coding_t coding, uint16 category, uint16 type,
		uint16 subtype, uint32 pos) 
		: IEError(ERROR_WRONG_LENGTH),
		  coding(coding), category(category), type(type),
		  subtype(subtype), errorpos(pos) {

	ostringstream ost;
	ost << "[coding " << coding << ", cat " << category << ", type " << type
		<< ", subtype " << subtype << ", pos" << errorpos
		<< IEError::getstr();

	error_msg = ost.str();
}


/***** class IETooBigForImpl *****/

/**
 * Constructor.
 *
 * @param coding the protocol version
 * @param category the IE's category
 * @param pos the position in the NetMsg at which the problem was discovered
 */
IETooBigForImpl::IETooBigForImpl(IE::coding_t coding, uint16 category,
		uint32 pos) 
		: IEError(ERROR_TOO_BIG_FOR_IMPL),
		  coding(coding), category(category), errorpos(pos) {

	ostringstream ost;
	ost << "[coding " << coding << ", cat " << category
		<< ", pos" << errorpos << IEError::getstr();

	error_msg = ost.str();
}


/***** class IEProtocol *****/

/**
 * Constructor.
 *
 * @param coding the protocol version
 * @param category the IE's category
 * @param pos the position in the NetMsg at which the problem was discovered
 */
IEProtocolSpecific::IEProtocolSpecific(IE::coding_t coding, uint16 category,
		uint32 pos) 
		: IEError(ERROR_PROT_SPECIFIC),
		  coding(coding), category(category), errorpos(pos) {

	ostringstream ost;
	ost << "[coding " << coding << ", cat " << category
		<< ", pos" << errorpos << IEError::getstr();

	error_msg = ost.str();
}


/***** class IEErrorList *****/

/** Insert IE Error into list. */
void IEErrorList::put(IEError* iee) {
	if (iee) queue.push_front(iee);
} // end put

/** Get and remove IE Error from the list. 
 * Returns NULL if list is empty.
 */
IEError* IEErrorList::get() {
	register IEError* iee;
	if (queue.empty()) return NULL;
	else {
		iee = queue.back();
		queue.pop_back();
		return iee;
	} // end if
} // end get

/** Is the IE error list empty? */
bool IEErrorList::is_empty() const {
	return queue.empty();
} // end is_empty

/** clear IE error list and destroy all stored IE error objects. */
void IEErrorList::clear() {
	queueit_t qit;
	// delete IEError objects in queue
	for (qit=queue.begin();qit!=queue.end();++qit)
		if (*qit) delete *qit;
	// clear queue
	queue.clear();		
} // end clear

/** Destroy IEError list and all IEError objects in it. */
IEErrorList::~IEErrorList() {
	if (!queue.empty()) {
	  Log(DEBUG_LOG,LOG_CRIT, "IE", "Destroying non-empty IEErrorList, deleting IEError objects.");
	  clear();
	} // end if not empty
} // end ~IEErrorList


/***** class IEManager *****/


/**
 * Constructor.
 */
IEManager::IEManager() {
	// nothing to do
}


/**
 * Destructor.
 *
 * All registered IEs are deleted.
 */
IEManager::~IEManager() {
	// iterator shortcuts
	typedef category_map_t::const_iterator c_iter;

	/*
	 * Walk through all registered IEs and delete them.
	 */
	int num_deleted = 0;

	for (c_iter i = registry.begin(); i != registry.end(); i++) {
		IE *ie = i->second;

		if ( ie != NULL ) {
			delete ie;
			num_deleted++;
		}
	}

	DLog("IEManager", "Deleted " << num_deleted << " IEs");
}


/** 
 * Register an Information Element.
 *
 * Make the IE object passed as the argument register itself with this
 * IEManager. See IE's register_ie() method for the rules.
 *
 * This method exists for convenience only, please see the other register_ie()
 * method for details.
 *
 * @param ie pointer to IE (NULL is not allowed)
 */
void IEManager::register_ie(const IE* ie) {
	if ( ie == NULL )
		throw IEError(IEError::ERROR_REGISTER);

	return ie->register_ie(this);
}


/**
 * Register an Information Element.
 *
 * Register an IE for the given category, type and subtype. It is not allowed
 * to register more than one IE for a (category, type, subtype) triple.
 *
 * It is @em very important that each IE instance is registered only once.
 * If a class can handle multiple (category, type, subtype) triples, one
 * instance per triple has to be registered.
 * 
 * There is no way to unregister an IE. All registered IEs will be deleted
 * by IEManager's destructor.
 *
 * @param category category of the IE
 * @param type IE type
 * @param subtype IE subtype
 * @param ie the IE to register (NULL is not allowed)
 */
void IEManager::register_ie(uint16 category, uint16 type, uint16 subtype,
		const IE* ie) {

	// for logging
	ostringstream triple;
	triple << "(" << category << ", " << type << ", " << subtype << ")";

	if ( ie == NULL )
		throw IEError(IEError::ERROR_REGISTER);

	IE_Key key(category, type, subtype);

	// don't allow overwriting
	if ( registry[key] != NULL ) {
		ERRLog("IEManager",
			"An IE is already " << "registered for " << triple);
		return;
	}


	try {
		registry[key] = const_cast<IE *>(ie);
	}
	catch ( ... ) {
		ERRLog("IEManager", "Cannot register IE for " << triple);

		throw IEError(IEError::ERROR_REGISTER);
	}

	// We made it so far, this means success.
	DLog("IEManager",
		"Registered IE " << ie->get_ie_name() << " for " << triple);
}


/**
 * Create a new instance.
 *
 * Creates a new instance using the appropriate registered IE. The definition
 * of @em appropriate depends on lookup_ie().
 *
 * Note that some old IEManager child classes called this method lookup().
 * This old lookup() method should not be confused with lookup_ie().
 *
 * @param category category of the IE
 * @param type IE type
 * @param subtype IE subtype
 * @return a new instance, or NULL if no matching IE is found
 */
IE *IEManager::new_instance(uint16 category, uint16 type, uint16 subtype) {
	IE *ie = lookup_ie(category, type, subtype);

	if ( ie )
		return ie->copy();
	else
		return NULL;
}


/**
 * Return a registered IE.
 *
 * Returns a registered IE instance. It does @em not create a new instance,
 * it is really an instance you registered earlier. Use new_instance() if
 * you need new instances!
 *
 * This method is an extension point that can be used, for example, to return
 * "default" IEs if no matching IE is found in the registry.
 *
 * @param category category of the IE
 * @param type IE type
 * @param subtype IE subtype
 * @return a registered instance, or NULL if no matching IE is found
 */
IE *IEManager::lookup_ie(uint16 category, uint16 type, uint16 subtype) {
	return registry[IE_Key(category, type, subtype)];
}


/**
 * Throw an exception and log it.
 */
void IEManager::throw_nomem_error() const {
	try {
		ERRLog("IEManager", "Out of memory.");
	}
	catch ( ... ) {
		// There's no way to handle this!
	}

	throw IEError(IEError::ERROR_NO_MEM);
}


/***** operator<< and >> *****/

/** Print the given IE to the ostream using level=0 and indent=3. */
ostream& operator<<(ostream& os, const IE& ie) {
	ie.print(os,0,3);
	return os;
} // end operator<<

istream& operator>>(istream& is, IE& ie) {
	ie.input(is,isatty(0),0,3);
	return is;
} // end operator>>

//@}

} // end namespace protlib
