// [License]
// The Ariba-Underlay Copyright
//
// Copyright (c) 2008-2009, Institute of Telematics, Universität Karlsruhe (TH)
//
// Institute of Telematics
// Universität Karlsruhe (TH)
// Zirkel 2, 76128 Karlsruhe
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
// THIS SOFTWARE IS PROVIDED BY THE INSTITUTE OF TELEMATICS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE ARIBA PROJECT OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation
// are those of the authors and should not be interpreted as representing
// official policies, either expressed or implied, of the Institute of
// Telematics.
// [License]

#ifndef SERIALIZATION_HPP_
#define SERIALIZATION_HPP_

//---------------------------------------------------------------------------
//= namespace adaption =
#ifndef SERIALIZATION_NS

#define SERIALIZATION_NS ariba::utility::serialization
#define SERIALIZATION_NS_INT SERIALIZATION_NS::internal

//---------------------------------------------------------------------------
//= some words on the namespaces =
#define SERIALIZATION_NS_BEGIN() \
	namespace ariba { namespace utility { namespace serialization {

#define SERIALIZATION_NS_END() }}}

#endif

//---------------------------------------------------------------------------
//= some definition of method and class names =
#define SERIALIZATION_METHOD_NAME __mSerialization
#define SERIALIZATION_CLASS_NAME  __cSerialization

//---------------------------------------------------------------------------
//= namespace specifications =
#define USING_SERIALIZATION using namespace SERIALIZATION_NS;
#define using_serialization USING_SERIALIZATION;

//---------------------------------------------------------------------------
//== prototypes ==

// public serializer prototype
template<typename __Y, int __V> class SERIALIZATION_CLASS_NAME;

SERIALIZATION_NS_BEGIN()

// the internal namespace
namespace internal {}

// default variants
const int DEFAULT_V = 0;
const int STRING_V  = 1;

// serialization mode
enum Mode {
	UNDEFINED = 0,
	SERIALIZE = 1,
	DESERIALIZE = 2,
	MEASURE = 3
};

SERIALIZATION_NS_END()

//---------------------------------------------------------------------------
//= Serialization Macros =

#define SERIALIZATION_USE_INTERNAL_NS \
	using namespace SERIALIZATION_NS_INT; \
	using namespace SERIALIZATION_NS;

//= serialization method =
#define ISERIALIZATION_METHOD_BEGIN( Buffer, Const ) \
	public:	template<typename __X> 	\
	finline bool SERIALIZATION_METHOD_NAME( __X& Buffer ) Const { \
	SERIALIZATION_USE_INTERNAL_NS \
	bool __ok = false; do {

#define RSERIALIZATION_METHOD_BEGIN( Buffer ) \
	ISERIALIZATION_METHOD_BEGIN( Buffer, )

#define SERIALIZATION_METHOD_BEGIN( Buffer ) \
	ISERIALIZATION_METHOD_BEGIN( Buffer, const )

#define SERIALIZATION_METHOD_END() \
	__ok = true; } while (false); return __ok; }

// convenience
#define sznMethodBegin( Buffer ) SERIALIZATION_METHOD_BEGIN( Buffer )
#define sznMethodEnd() SERIALIZATION_METHOD_END()

//= serialization class stub =
#define SERIALIZEABLE \
	public: template<typename __Y,int __V> \
	friend class ::SERIALIZATION_CLASS_NAME;

#define sznStub SERIALIZEABLE

//= serialization code generation =
#define SERIALIZATION_BEGIN( Class, Variant, Buffer ) \
	template<> \
	class SERIALIZATION_CLASS_NAME \
		<Class, SERIALIZATION_NS::Variant> : Class { \
	RSERIALIZATION_METHOD_BEGIN( Buffer )

#define SERIALIZATION_END() \
	SERIALIZATION_METHOD_END() };

// convenience
#define sznBegin(Class,Variant,Buffer) \
	SERIALIZATION_BEGIN( Class, Variant, Buffer )
#define sznBeginDefault( Class, Buffer ) \
	SERIALIZATION_BEGIN( Class, DEFAULT_V, Buffer )
#define sznEnd() SERIALIZATION_END()

//= virtual serialization =
#define VSERIALIZEABLE \
	SERIALIZEABLE \
	virtual size_t SERIALIZATION_METHOD_NAME( \
		SERIALIZATION_NS::Mode __mode, \
		Data& __data, \
		int __variant = SERIALIZATION_NS::DEFAULT_V \
	);

#define VSERIALIZATION_BEGIN( Class ) \
	size_t Class::SERIALIZATION_METHOD_NAME( \
		SERIALIZATION_NS::Mode __mode, \
		Data& __data, \
		int __variant \
	) { \
	USING_SERIALIZATION; \
	SERIALIZATION_USE_INTERNAL_NS; \
	switch (__variant) {

#define VSERIALIZATION_END() \
	} return 0;	}

#define VSERIALIZATION_REG( __variant ) \
	case __variant: \
	switch (__mode) { \
	case SERIALIZE: \
	__data = data_serialize_v<__variant>(*this); \
	return __data.getLength(); \
	case DESERIALIZE: \
	return data_deserialize_v<__variant>(*this, __data); \
	case MEASURE: \
	return data_length_v<__variant>(*this); \
	case UNDEFINED: \
	return 0; \
	} break;

#define VSERIALIZATION_DEFAULT( Class ) \
	USING_SERIALIZATION \
	VSERIALIZATION_BEGIN( Class ) \
	VSERIALIZATION_REG( DEFAULT_V ) \
	VSERIALIZATION_END( )

// convenience
#define vsznStub VSERIALIZEABLE
#define vsznBegin( Class ) VSERIALIZAION_BEGIN( Class )
#define vsznEnd() VSERIALIZATION_END()
#define vsznRegister( Variant ) VSERIALIZATION_REG( Variant )
#define vsznDefault( Class ) VSERIALIZATION_DEFAULT( Class )

// new: convenience
#define sznImplBegin(Class) VSERIALIZAION_BEGIN( Class )
#define sznImplDefault(Class) VSERIALIZATION_DEFAULT( Class )

//---------------------------------------------------------------------------
//== includes ==
#include "../internal/Utilities.hpp"
#include <typeinfo>
#include <iostream>
#include <cstdio>

//---------------------------------------------------------------------------
//= Public Serialization Classes =

/**
 * TODO: doc
 */
template<typename __Y, int __V>
class SERIALIZATION_CLASS_NAME: __Y {
public:
	template<class __X>
	finline bool SERIALIZATION_METHOD_NAME(__X& buffer) {
		printf("Serialization not supported for type '%s' "
			"with variant %d for Stream %s.\n",
			typeid(__Y).name(), __V, typeid(__X).name());
		return false;
	}
};

class Serializeable {};

#include "Data.hpp"

class VSerializeable : public Serializeable {
public:
	/**
	 * Serializes/Deserializes an this object in the specified variant.
	 * The special case is, that virtual serializeable objects are bound
	 * to byte boundaries -- so they cannot be smaller than a byte.
	 *
	 * @param data BitData object that holds or is used to serialize data
	 * @param mode Mode of operation (serialize/deserialize)
	 * @param variant Variant of encoding/decoding
	 * @return size of the binary object in bits or zero if unsuccessful
	 */
	virtual size_t SERIALIZATION_METHOD_NAME(
		SERIALIZATION_NS::Mode __mode,
		Data& __binary,
		int __variant = SERIALIZATION_NS::DEFAULT_V
	);
};

/**
 * TODO: doc
 */
class ExplicitSerializer {
	SERIALIZATION_METHOD_BEGIN(X)
		std::cerr << "Serialization unimplemented" << std::endl;
	SERIALIZATION_METHOD_END()
};

//---------------------------------------------------------------------------
//= Serialization Namespace =

SERIALIZATION_NS_BEGIN()

/**
 * TODO: doc
 */
template<int V, typename Y>
finline static SERIALIZATION_CLASS_NAME<Y, V>& get_serializer(Y& obj) {
	return *((SERIALIZATION_CLASS_NAME<Y, V>*)(&obj));
}
SERIALIZATION_NS_END()

#endif /* SERIALIZATION_HPP_ */
