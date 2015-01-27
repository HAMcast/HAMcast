#ifndef COMPARE_TO_OPERATORS_HPP_
#define COMPARE_TO_OPERATORS_HPP_

namespace ariba {
namespace addressing {
namespace detail {


/**
 * This class implements some convenient operators for the compare_to method.
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
template<class T>
class compare_to_operators {
public:
	/// convenience method for comparison of addresses
	inline bool operator==( const T& rhs ) const {
		return static_cast<const T*>(this)->compare_to(rhs) == 0;
	}

	/// convenience method for comparison of addresses
	inline bool operator!=( const T& rhs ) const {
		return static_cast<const T*>(this)->compare_to(rhs) != 0;
	}

	/// convenience method for comparison of addresses
	inline bool operator<( const T& rhs ) const {
		return static_cast<const T*>(this)->compare_to(rhs) < 0;
	}

	/// convenience method for comparison of addresses
	inline bool operator<=( const T& rhs ) const {
		return static_cast<const T*>(this)->compare_to(rhs) <= 0;
	}

	/// convenience method for comparison of addresses
	inline bool operator>( const T& rhs ) const {
		return static_cast<const T*>(this)->compare_to(rhs) > 0;
	}

	/// convenience method for comparison of addresses
	inline bool operator>=( const T& rhs ) const {
		return static_cast<const T*>(this)->compare_to(rhs) >= 0;
	}
};

}}} // namespace ariba::addressing::detail

#endif /* COMPARE_TO_OPERATORS_HPP_ */
