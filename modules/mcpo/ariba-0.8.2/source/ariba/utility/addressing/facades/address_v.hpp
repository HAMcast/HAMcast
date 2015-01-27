#ifndef ADDRESS_V_H_
#define ADDRESS_V_H_

#include <stdint.h>
#include <string>
#include <iostream>

#include "vfacade.hpp"

#include "to_bytes_v.hpp"
#include "to_string_v.hpp"
#include "comparable_v.hpp"

#include "../detail/address_convenience.hpp"

namespace ariba {
namespace addressing {

using std::string;

/**
 * A virtual interface to a protocol address.
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class address_v: public detail::address_convenience<address_v> {
public:
	//--- to_string_v ---------------------------------------------------------

	/// convert address to a string that can be used to reconstruct the address
	virtual string to_string() const = 0;

	/// Assigns an address using a human-readable
	virtual bool assign(const std::string& text) = 0;

	//--- to_bytes_v ----------------------------------------------------------

	/// returns true, if this address has a fixed size in bytes
	virtual bool is_bytes_size_static() const = 0;

	/// returns the number of bytes used for serialization of this address
	virtual size_t to_bytes_size() const = 0;

	/// converts this address to a binary representation
	virtual void to_bytes(uint8_t* bytes) const = 0;

	/// Assigns an address using a bunch of bytes
	virtual bool assign(const uint8_t* bytes, size_t size) = 0;

	//--- comparable_v --------------------------------------------------------

	/// implements comparison operators
	virtual int compare_to(const address_v& rhs) const = 0;

	//--- assignment ----------------------------------------------------------

	/// Assigns an address
	virtual bool assign(const address_v& rhs) = 0;

	//--- address info --------------------------------------------------------

	/// returns the name of the address
	virtual const string& type_name() const = 0;

	/// sets the type id, if possible
	virtual uint16_t type_id() const = 0;

	//--- cloneing and conversion ---------------------------------------------

	/// Obtain the underlaying data type or null if it does not match the type
	virtual void* data(const std::type_info& type) = 0;

	/// Clones this address
	virtual address_v* clone() const = 0;

	//--- convenience ---------------------------------------------------------

	template<class T>
	bool instanceof() const {
		void* value = const_cast<address_v*>(this)->data(typeid(T));
		return value!=NULL;
	}

	/// cast operator to detailed type
	template<class T>
	inline operator T&() {
		void* value = data(typeid(T));
		assert (value!=NULL);
		return *((T*) value);
	}

	/// cast operator to detailed type
	template<class T>
	inline operator T () const {
		return T( (const T&)*this );
	}

	/// cast operator to detailed type
	template<class T>
	inline operator const T& () const {
		void* value = const_cast<address_v*>(this)->data(typeid(T));
		assert (value!=NULL);
		return *((const T*) value);
	}
};

/// stream operator
std::ostream& operator<<(std::ostream& s, const address_v* addr);

/// define the virtual facade for the address
typedef vfacade<address_v> address_vf;

}} // namespace ariba::addressing

/// the virtual adaptor to certain class of objects
template<class NonVirtual, class AdaptorType>
class vobject_hull<NonVirtual, ariba::addressing::address_v, AdaptorType> : public ariba::addressing::address_v {

private:
	typedef ariba::addressing::address_v self;
	typename AdaptorType::template adaptor_type<NonVirtual> obj;

	static inline NonVirtual& conv(ariba::addressing::address_v* obj) {
		return *((NonVirtual*) obj->data(typeid(NonVirtual)));
	}

	static inline const NonVirtual& conv(const ariba::addressing::address_v* obj) {
		const NonVirtual* v =
				(const NonVirtual*) const_cast<ariba::addressing::address_v*> (obj)->data(
						typeid(NonVirtual));
		return *v;
	}

public:
	template<typename T>
	explicit vobject_hull(T& obj) :
		obj(obj) {
	}
	explicit vobject_hull() :
		obj() {
	}

	//--- assignment ----------------------------------------------------------

	/// Assigns an address
	virtual bool assign(const self& rhs) {
		return obj->assign(conv(&rhs));
	}

	//--- address info --------------------------------------------------------

	/// returns the name of the address
	virtual const string& type_name() const {
		return obj->type_name();
	}

	/// returns the id of the address
	virtual uint16_t type_id() const {
		return obj->type_id();
	}

	/// returns a capsule of the object
	virtual address_v* clone() const {
		return vcapsule<address_v> (*obj);
	}

	/// Obtain the underlaying data type or null if it does not match the type
	virtual void* data(const std::type_info& type) {
		if (typeid(NonVirtual)!=type) return NULL;
		return &(*obj);
	}

	// to_string_v

	/// convert address to a string that can be used to reconstruct the address
	virtual string to_string() const {
		return obj->to_string();
	}

	/// Assigns an address using a human-readable
	virtual bool assign(const std::string& text) {
		return obj->assign(text);
	}

	// comparable_v

	/// implements comparison operators
	virtual int compare_to(const self& rhs) const {
		const address_v* compare = dynamic_cast<const address_v*> (&rhs);
		return obj->compare_to(conv(compare));
	}

	// to_bytes_v

	/// returns true, if this address has a fixed size in bytes
	virtual bool is_bytes_size_static() const {
		return obj->is_bytes_size_static();
	}

	/// returns the number of bytes used for serialization of this address
	virtual size_t to_bytes_size() const {
		return obj->to_bytes_size();
	}

	/// converts this address to a binary representation
	virtual void to_bytes(uint8_t* bytes) const {
		obj->to_bytes(bytes);
	}

	/// Assigns an address using a bunch of bytes
	virtual bool assign(const uint8_t* bytes, size_t size) {
		return obj->assign(bytes, size);
	}

};


#endif /* ADDRESS_V_H_ */
