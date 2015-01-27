// to_string_v.hpp, created on 30.06.2009 by Sebastian Mies

#ifndef TO_STRING_V_HPP_
#define TO_STRING_V_HPP_

#include <string>
#include "vfacade.hpp"

using namespace std;

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
/// a to to string interface
class to_string_v {
public:
	//--- to_string_v ---------------------------------------------------------

	/// convert address to a string that can be used to reconstruct the address
	virtual string to_string() const = 0;

	/// Assigns an address using a human-readable
	virtual bool assign(const std::string& text) = 0;

	/// convenience method for assignment of a c-strings
	inline to_string_v& assign(const char* text) {
		to_string_v::assign(std::string(text));
		return *this;
	}

	/// convenience method for assignment of strings
	inline to_string_v& operator=(const std::string& text) {
		assign(text);
		return *this;
	}

	/// convenience method for assignment of strings
	inline to_string_v& operator=(const char* text) {
		assign(text);
		return *this;
	}
};

typedef vfacade<to_string_v> to_string_vf;

/// the object hull for to_string interfaces
template<class NonVirtual, class AdaptorType>
class vobject_hull<NonVirtual, to_string_v, AdaptorType> : public to_string_v {
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

	//--- to_string_v ---------------------------------------------------------

	/// convert address to a string that can be used to reconstruct the address
	virtual string to_string() const {
		return obj->to_string();
	}

	/// Assigns an address using a human-readable
	virtual bool assign(const std::string& text) {
		return obj->assign(text);
	}
};

#endif /* TO_STRING_V_HPP_ */
