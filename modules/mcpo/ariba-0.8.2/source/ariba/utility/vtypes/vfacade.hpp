// [License]
// The FreeBSD License
//
// Copyright (c) 2008-2009 Sebastian Mies <mies@edcft.de>
//
// Sebastian Mies
// Gebhardstrasse 25
// 76137 Karlsruhe
// Germany
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR  CONTRIBUTORS
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// [License]

#ifndef VFACADE_HPP_
#define VFACADE_HPP_

//--- USAGE INFORMATION --------------------------------------------------------
//
// This header is used to easily implement a virtual facade design pattern. It
// allows to specify a virtual wrapper interface to a non-virtual class. Look
// at this example:
//
// <code>
// // unspecified version
// class to_string_definition : public vfacade_interface {
// public:
//     virtual string to_string() const {
//         return "UNIMPLEMENTED: to_string()";
//     };
// };
//
// // delegation version
// template<class T>
// class vfacade_delegation<to_string_definition, T> :
//     public to_string_definition {
// public:
//     virtual string to_string() const {
//         return get<T>().to_string();
//     }
// };
//
// // final type
// typedef vfacade<to_string_definition> vto_string;
// </code>
//
// The to_string_definition specifies an unimplemented facade interface. Its
// necessary for this class not to be pure-virtual. The vfacade_delegation
// specialization implements a delegation any another non-virtual class T.
// Finally the facade type is defined by using the vfacade template. Now,
// we can use this facade, like in the following example:
//
// <code>
// class A {
// public:
//     string to_string() {
//         return "a hello from class A";
//     }
// };
//
// void main() {
//     A something;
//     vto_string to_str = something;
//     cout << to_str->to_string() << endl;
// }
// </code>
//
// This example outputs "a hello from class A" by using the virtual facade. As
// defined class A does not need to know about the facade -- it just implements
// the needed methods. The to_string method is transparently bound to the
// facade by the delegation implementation.
//
// The purpose of this virtual facade is simple: whenever you need a abstract
// representation of non-virtual structures you can simply use a virtual facade.
// It carries a vtable with the delegation to the non-virtual methods and a
// pointer to the original object. Therefore it binds a vtable to a pointer
// so vtable information is stored with the pointer and not with the object
// itself.
//
//------------------------------------------------------------------------------

// forward declaration
template<class X, class Y>
class vfacade_delegation;

/**
 * This class template implements a virtual facade using a defined interface
 * X and a certain delegation specialization of vfascade_delegation<X,Y>.
 *
 * @author Sebastian Mies <mies@edcft.de>
 */
template<class X>
class vfacade {
private:
	X value;

public:
	vfacade() {
	}

	template<class Y>
	vfacade(const Y& _value) {
		vfacade_delegation<X, Y> v;
		v.set(_value);
		memcpy(&value, &v, sizeof(X));
	}

	template<class Y>
	vfacade<X>& operator=(const Y& _value) {
		vfacade_delegation<X, Y> v;
		v.set(_value);
		memcpy(&value, &v, sizeof(X));
		return *this;
	}

	vfacade(const vfacade<X>& v) {
		memcpy(&value, &v.value, sizeof(X));
	}

	vfacade<X>& operator=(const vfacade<X>& v ) {
		memcpy(&value, &v.value, sizeof(X));
		return *this;
	}

	X* operator->() {
		return &value;
	}

	const X* operator->() const {
		return &value;
	}
};

/**
 * This class is used as base class for virtual facade interfaces.
 *
 * @author Sebastian Mies <mies@edcft.de>
 */
class vfacade_interface {
	template<class X> friend class vfacade;

private:
	void* value;

protected:
	template<class X> X& get() const {
		return *((X*) value);
	}
	template<class X> void set(X& v) {
		value = (void*) &v;
	}

public:
	vfacade_interface() :
		value(NULL) {
	}

	bool is_unspecified() const {
		return value == NULL;
	}

	template<class Y>
	Y* ptr() {
		return (Y*)value;
	}

	template<class Y>
	const Y* ptr() const {
		return (Y*)value;
	}

	template<class Y>
	Y& cast_to() {
		return *ptr<Y>();
	}

	template<class Y>
	const Y& cast_to() const {
		return *ptr<Y>();
	}
};

/**
 * This class is the base template for further delegation specializations.
 *
 * @author Sebastian Mies <mies@edcft.de>
 */
template<class T, class Y>
class vfacade_delegation: public T {

};

#endif
