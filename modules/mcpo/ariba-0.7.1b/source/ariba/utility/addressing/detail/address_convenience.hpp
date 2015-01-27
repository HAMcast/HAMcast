#ifndef ADDRESS_CONVENIENCE_HPP_
#define ADDRESS_CONVENIENCE_HPP_

#include <string>
#include "compare_to_operators.hpp"

namespace ariba {
namespace addressing {
namespace detail {

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
template<class T>
class address_convenience : public compare_to_operators<T> {
public:
	/// convenience method for assignment of addresses
	inline T& operator=( const T& rhs ) {
		static_cast<T*>(this)->assign(rhs);
		return *this;
	}

	/// convenience method for assignment of addresses
	inline T& operator=( const char* text ) {
		static_cast<T*>(this)->assign(text);
		return *this;
	}

	/// convenience method for assignment of addresses
	inline T& operator=( const std::string& text ) {
		static_cast<T*>(this)->assign(text);
		return *this;
	}

	/// convenience method for assignment of a c-string address
	inline T& assign( const char* text ) {
		static_cast<T*>(this)->assign( std::string(text) );
		return *this;
	}
};

}}} // namespace ariba::addressing::detail

#endif /* ADDRESS_CONVENIENCE_HPP_ */
