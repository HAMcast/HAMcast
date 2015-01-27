#ifndef VFACADE_HPP_
#define VFACADE_HPP_

#include <iostream>
#include <typeinfo>

#include <assert.h>
#include <memory.h>

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
/// the virtual object adaptor class
template<class NonVirtual, class Virtual, class Adaptor>
class vobject_hull : public Virtual {
};

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
/// the vfacade adaptor type
struct vfacade_adaptor_type {
	template<class NonVirtual>
	class adaptor_type {
	private:
		NonVirtual* obj_;
	public:
		explicit inline adaptor_type() :
			obj_() {
		}
		explicit inline adaptor_type(NonVirtual& obj) :
			obj_(&obj) {
		}
		explicit inline adaptor_type(const NonVirtual& obj) :
			obj_(&obj) {
		}
		inline void set(NonVirtual& obj) {
			obj_ = &obj;
		}
		inline NonVirtual* operator->() {
			return obj_;
		}
		inline const NonVirtual* operator->() const {
			return obj_;
		}
		inline const NonVirtual& operator*() const {
			return *obj_;
		}
		inline NonVirtual& operator*() {
			return *obj_;
		}
	};
};

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
/// the virtual capsule adaptor type
struct vcapsule_adaptor_type {
	template<class NonVirtual>
	class adaptor_type {
	private:
		NonVirtual obj_;
	public:
		inline adaptor_type() :
			obj_() {
		}
		inline adaptor_type(const NonVirtual& obj) :
			obj_(obj) {
		}
		inline void assign(const NonVirtual& obj) {
			obj_ = obj;
		}
		inline NonVirtual* operator->() {
			return &obj_;
		}
		inline const NonVirtual* operator->() const {
			return &obj_;
		}
		inline const NonVirtual& operator*() const {
			return obj_;
		}
		inline NonVirtual& operator*() {
			return obj_;
		}
	};
};

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
/// placeholder
class vfacade_no_class {};

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
/// a virtual fascade implementation
template<class Virtual, class Extension = vfacade_no_class>
class vfacade : public Extension {
public:
	typedef vfacade<Virtual> self;

private:
	void* vtable;
	void* vadaptor;

	template<class T>
	void assign(T& obj) {
		typedef vobject_hull<T, Virtual, vfacade_adaptor_type> adaptor_type;
		adaptor_type adaptor(obj);
		assert( sizeof(adaptor_type) == sizeof(vfacade) );
		memcpy((void*) this, (void*) &adaptor, sizeof(vfacade));
	}

	template<class T>
	void assign(const T& obj) {
		T& obj_ = *const_cast<T*>(&obj);
		typedef vobject_hull<T, Virtual, vfacade_adaptor_type> adaptor_type;
		adaptor_type adaptor(obj_);
		assert( sizeof(adaptor_type) == sizeof(vfacade) );
		memcpy((void*) this, (void*) &adaptor, sizeof(vfacade));
	}

	void assign(self& copy) {
		this->vtable = copy.vtable;
		this->vadaptor = copy.vadaptor;
	}

	void assign(const self& copy) {
		this->vtable = copy.vtable;
		this->vadaptor = copy.vadaptor;
	}

public:
	/// constructs an undefined virtual facade
	inline vfacade() {
		this->vtable = NULL;
		this->vadaptor = NULL;
	}

	/// constructs an initialized virtual facade
	template<class T>
	inline vfacade(T& obj) {
		assign(*const_cast<T*>(&obj));
	}

	/// assigns a new object to this facade
	template<class T>
	inline self& operator=(T& obj) {
		assign(obj);
		return *this;
	}

	/// returns true, if the facade is unassigned
	inline bool is_null() const {
		return this->vtable == NULL;
	}

	inline operator Virtual* () {
		assert(vtable!=NULL);
		return (Virtual*) this;
	}

	inline operator const Virtual* () const {
		assert(vtable!=NULL);
		return (const Virtual*) this;
	}

	inline operator Virtual& () {
		assert(vtable!=NULL);
		return *(Virtual*) this;
	}

	inline operator const Virtual& () const {
		assert(vtable!=NULL);
		return *(const Virtual*) this;
	}

	/// returns the virtual object of the facade
	inline Virtual* operator->() {
		assert(vtable!=NULL);
		return (Virtual*) this;
	}

	/// returns a virtual object reference
	inline Virtual& operator*() {
		assert(vtable!=NULL);
		return *((Virtual*) this);
	}

	/// returns a virtual object reference
	inline const Virtual& operator*() const {
		assert(vtable!=NULL);
		return *((const Virtual*) this);
	}

	/// returns a constant virtual object of the facade
	inline const Virtual* operator->() const {
		assert(vtable!=NULL);
		return (const Virtual*) this;
	}
};

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
/// creates a virtual capsule for an object
template<class Virtual, class NonVirtual>
Virtual* vcapsule(const NonVirtual& obj) {
	return new vobject_hull<NonVirtual, Virtual, vcapsule_adaptor_type> (obj);
}

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
/// returns the size in bytes of a virtual capsule
template<class Virtual, class NonVirtual>
size_t vcapsule_size(const NonVirtual& obj) {
	return sizeof(vobject_hull<NonVirtual, Virtual, vcapsule_adaptor_type> (obj));
}

#endif /* VFACADE_HPP_ */
