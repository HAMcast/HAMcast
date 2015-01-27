// to_bytes_v.hpp, created on 30.06.2009 by Sebastian Mies

#ifndef TO_BYTES_V_HPP_
#define TO_BYTES_V_HPP_

#include <memory>
#include <stdint.h>
#include "vfacade.hpp"

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class to_bytes_v {
public:
	//--- to_bytes_v ----------------------------------------------------------

	/// returns true, if this address has a fixed size in bytes
	virtual bool is_bytes_size_static() const = 0;

	/// returns the number of bytes used for serialization of this address
	virtual size_t to_bytes_size() const = 0;

	/// converts this address to a binary representation
	virtual void to_bytes(uint8_t* bytes) const = 0;

	/// Assigns an address using a bunch of bytes
	virtual bool assign(const uint8_t* bytes, size_t size) = 0;
};

typedef vfacade<to_bytes_v> to_bytes_vf;

template<class NonVirtual, class AdaptorType>
class vobject_hull<NonVirtual, to_bytes_v, AdaptorType> : public to_bytes_v {
private:
	typename AdaptorType::template adaptor_type<NonVirtual> obj;

public:
	template<typename T>
	explicit vobject_hull(T& obj) :
		obj(obj) {
	}

	explicit vobject_hull() :
		obj() {
	}

	//--- to_bytes_v ----------------------------------------------------------

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
		return obj->assign(bytes,size);
	}
};


#endif /* TO_BYTES_V_HPP_ */
