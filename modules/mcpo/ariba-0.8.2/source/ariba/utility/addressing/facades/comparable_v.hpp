// comparable_v.hpp, created on 30.06.2009 by Sebastian Mies

#ifndef COMPARABLE_V_HPP_
#define COMPARABLE_V_HPP_

#include "vfacade.hpp"

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class comparable_v {
public:
	//--- comparable_v --------------------------------------------------------

	/// implements comparison operators
	virtual int compare_to(const comparable_v& rhs) const = 0;

	/// convenience method for comparison
	inline int compare_to(const comparable_v* rhs) const {
		return compare_to(*rhs);
	}

	/// convenience method for comparison
	inline bool operator==(const comparable_v& rhs) const {
		return compare_to(rhs) == 0;
	}

	/// convenience method for comparison
	inline bool operator!=(const comparable_v& rhs) const {
		return compare_to(rhs) != 0;
	}

	/// convenience method for comparison
	inline bool operator<(const comparable_v& rhs) const {
		return compare_to(rhs) < 0;
	}

	/// convenience method for comparison
	inline bool operator<=(const comparable_v& rhs) const {
		return compare_to(rhs) <= 0;
	}

	/// convenience method for comparison
	inline bool operator>(const comparable_v& rhs) const {
		return compare_to(rhs) > 0;
	}

	/// convenience method for comparison
	inline bool operator>=(const comparable_v& rhs) const {
		return compare_to(rhs) >= 0;
	}
};

typedef vfacade<comparable_v> comparable_vf;

template<class NonVirtual, class AdaptorType>
class vobject_hull<NonVirtual, comparable_v, AdaptorType> : public comparable_v {
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

	//--- comparable_v --------------------------------------------------------

	/// implements comparison operators
	virtual int compare_to(const comparable_v& rhs) const {
		return obj->compare_to(rhs);
	}
};



#endif /* COMPARABLE_V_HPP_ */
