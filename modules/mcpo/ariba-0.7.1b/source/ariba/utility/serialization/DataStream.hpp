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

#ifndef DATASTREAM_HPP_
#define DATASTREAM_HPP_

//== Library includes ==
#include <boost/cstdint.hpp>
#include <boost/type_traits/make_unsigned.hpp>

#include <cassert>
#include <iostream>
#include <vector>
#include <string.h>

#ifndef SERIALIZATION_NS
#define SERIALIZATION_NS ariba::utility::serialization
#define SERIALIZATION_NS_INT SERIALIZATION_NS::internal

/* some words on the namespaces */
#define SERIALIZATION_NS_BEGIN() \
	namespace ariba { \
	namespace utility { \
	namespace serialization {

#define SERIALIZATION_NS_END() \
	} } }
#endif

//== forward declarations ==
SERIALIZATION_NS_BEGIN()

// data stream template forward declaration
template<Mode __mode, int __variant = DEFAULT_V, typename T = uint8_t>
class DataStreamTpl;

// internal explicit serialization forward declaration
namespace internal {
	template<typename T> class ITpl;
	template<typename T> ITpl<T> I( T& value, size_t length = sizeof(T)*8 );
	class cI;
	class T;
	class cT;
}

// virtual serialization forward declaration
bool data_serialization( Mode mode, VSerializeable* obj, Data& data, int variant = DEFAULT_V );
Data data_serialize( const VSerializeable* obj, int variant = DEFAULT_V );
bool data_deserialize( VSerializeable* obj, Data& data, int variant = DEFAULT_V );
size_t data_length( const VSerializeable* obj, int variant = DEFAULT_V );

SERIALIZATION_NS_END()

//== Internal includes ==
#include "../internal/Utilities.hpp"

//== Local includes ==
#include "Serialization.hpp"
#include "DataUtilities.hpp"
#include "Data.hpp"

SERIALIZATION_NS_BEGIN()

/**
 *
 */
template<Mode __mode, int __variant, typename T>
class DataStreamTpl {

	template<typename X>
	friend class SERIALIZATION_NS_INT::ITpl;
	friend class SERIALIZATION_NS_INT::cI;
	friend class SERIALIZATION_NS_INT::T;
	friend class SERIALIZATION_NS_INT::cT;

	template<Mode _a, int _b, typename _c>
	friend std::ostream& operator<<( std::ostream&, DataStreamTpl<_a, _b, _c>& );

private:
	/* index in the buffer */
	size_t index;

	/* binary buffer */
	DataTpl<T> bits;

	/* support serializers */
	template<class X>
	finline void add(const X& ser, if_is_base_of( ExplicitSerializer, X )) {
		ser.SERIALIZATION_METHOD_NAME(*this);
	}

	template<class X>
	finline void remove(const X& ser, if_is_base_of( ExplicitSerializer, X )) {
		ser.SERIALIZATION_METHOD_NAME(*this);
	}

	/* support serializeables */
	template<class X>
	finline void add( X& obj, if_is_base_of( Serializeable, X ) ) {
		get_serializer<__variant>(obj).SERIALIZATION_METHOD_NAME(*this);
	}

	template<class X>
	finline void add( const X& obj, if_is_base_of( Serializeable, X ) ) {
		get_serializer<__variant>((X&)obj).SERIALIZATION_METHOD_NAME(*this);
	}

	template<class X>
	finline void remove( X& obj, if_is_base_of( Serializeable, X ) ) {
		get_serializer<__variant>(obj).SERIALIZATION_METHOD_NAME(*this);
	}

	/* support uints */
	template<typename X>
	finline void add(X& obj, if_uint(X)) {
		if (!isMeasure())
//			bitcpy( obj, 0, bits.getBuffer(), index);
			bitcpy( obj, 0, bits.getBuffer(), index, sizeof(X)*8 );
		index += sizeof(X) * 8;
	}

	template<typename X>
	finline void remove(X& obj, if_uint(X)) {
		//if (!isMeasure()) bits[index].get(obj);
		if (!isMeasure())
			bitcpy( bits.getBuffer(), index, obj, 0, sizeof(X)*8 );
		index += sizeof(X) * 8;
	}

	/* support signed ints */
	template<typename X>
	finline void add(X& sobj, if_int(X)) {
		typedef typename boost::make_unsigned<X>::type UX;
		UX& obj = *((UX*)&sobj);
		if (!isMeasure())
		bitcpy( sobj, 0, bits.getBuffer(), index);
		index += sizeof(X) * 8;
	}

	template<typename X>
	finline void remove(X& sobj, if_int(X)) {
		typedef typename boost::make_unsigned<X>::type UX;
		UX& obj = *((UX*)&sobj);
		if (!isMeasure()) bits[index].get(obj);
		index += sizeof(X) * 8;
	}

	/* support boolean types */
	finline void add( bool& obj ) {
		if (!isMeasure()) bits[index] = obj;
		index += 1;
	}

	finline void remove( bool& obj ) {
		if (!isMeasure()) bits[index].get(obj);
		index += 1;
	}

	/* support vserializeables */
	finline void add( const VSerializeable* obj ) {
		if (!isMeasure()) {
			Data data = data_serialize( (VSerializeable*)obj, __variant );
			add(data); data.release();
		} else {
			index += data_length( obj, __variant );
		}
	}

	finline void remove( VSerializeable* obj ) {
		Data data( bits.getBuffer() + index / 8, getRemainingLength() );
		size_t length = obj->SERIALIZATION_METHOD_NAME( __mode, data, __variant );
		index += length;
	}

	finline void remove( const VSerializeable* obj ) {
		throw "NOT SUPPORTED!";
	}

	/* data stream typedef */
	typedef DataStreamTpl<__mode, __variant, T> _DataStream;

public:

	finline DataStreamTpl() : bits() {
		index = 0;
	}

	finline DataStreamTpl( Data& binary ) : bits(binary) {
		index = 0;
	}

	finline DataStreamTpl( _DataStream& stream ) {
		throw "Error: DataStreams can not be copied.";
	}

	/**
	 * Returns the current mode of the data stream
	 *
	 * @return the current mode of the data stream
	 */
	finline Mode getMode() const {
		return __mode;
	}

	/**
	 * Returns true, if this is a measure of data length
	 *
	 * @return true, if this is a measure of data length
	 */
	finline bool isMeasure() {
		return __mode == MEASURE;
	}

	finline bool isSerializer() {
		return (__mode == SERIALIZE) || isMeasure();
	}

	finline bool isDeserializer() {
		return __mode == DESERIALIZE;
	}

	/**
	 * Returns the variants that are used.
	 *
	 * @return The variants that are used.
	 */
	finline int getVariant() const {
		return __variant;
	}

	/**
	 * Returns the remaining length.
	 * This method is only applicable in deserialize mode.
	 *
	 * @return The remaining length of the stream.
	 */
	finline size_t getRemainingLength() const {
		return bits.getLength() - index;
	}

	/**
	 * Returns the remaining data in the data buffer.
	 * This method is only applicable in deserialize mode.
	 *
	 * @return The remaining data in the data buffer.
	 */
	finline Data getRemainingData( size_t length ) {
		Data subData = Data(bits.getBuffer() + index / bits.word_width, length);
		index += length;
		return subData;
	}

	/**
	 * Returns the current length of the stream.
	 * This method is only applicable in serialize mode.
	 *
	 * @return The current length of the stream.
	 */
	finline size_t getCurrentLength() const {
		return index;
	}

	/**
	 * Resets the index of the stream
	 */
	finline void reset(Mode mode = UNDEFINED) {
		this->index = 0;
		this->mode = mode;
	}

	/**
	 * Returns a mark of the current position in the stream
	 */
	finline size_t mark() const {
		return index;
	}

	/**
	 * Seeks to the given position
	 */
	finline void seek( size_t idx ) {
		index = idx;
	}

	/**
	 * Assures that the given length is available in the
	 * buffer.
	 */
	finline void ensureLength( size_t size ) {
		if (!isMeasure()) bits.ensureLength( size + index );
	}

	/**
	 * Returns a binary object, of the current stream content
	 */
	finline Data getData() {
		bits.setLength(index);
		return bits;
	}

	finline uint8_t* bytes( size_t length ) {
		assert((index%bits.word_width)==0);
		if (!isMeasure()) {
			bits.ensureLength( index + length * 8);
			uint8_t* buffer = (uint8_t*)bits.getBuffer()+(index/bits.word_width);
			index += length*8;
			return buffer;
		} else {
			index += length*8;
			return NULL;
		}
	}

	/**
	 * deserialises or serializes an object depending on the bit-streams mode
	 */
	template<typename X>
	finline _DataStream& operator&&( X& obj ) {
		if (isSerializer()) add(obj); else if (isDeserializer()) remove(obj);
		return *this;
	}

	template<typename X>
	finline _DataStream& operator&&( const X& obj ) {
		if (isSerializer()) add(obj); else if (isDeserializer()) remove(obj);
		return *this;
	}

	template<typename X>
	finline _DataStream& operator&&( X* obj ) {
		if (isSerializer()) add(obj); else if (isDeserializer()) remove(obj);
		return *this;
	}

	template<typename X>
	finline _DataStream& operator&&( const X* obj ) {
		if (isSerializer()) add(obj); else if (isDeserializer()) remove(obj);
		return *this;
	}
};

//--------------------------------------------------------------------------

template<Mode _a, int _b, typename _c>
std::ostream& operator<<( std::ostream& stream,
		DataStreamTpl<_a, _b, _c>& binaryStream ) {
	stream << binaryStream.bits << "";
	return stream;
}

//--------------------------------------------------------------------------

namespace internal {
	using_serialization;

	/* integer support - I( value, length ) */
	template<typename T>
	class ITpl : public ExplicitSerializer {
	private:
		T& value;
		size_t length;
	public:
		finline ITpl( T& _value, size_t _length = sizeof(T)*8 )
		: value(_value), length(_length) {}

		sznMethodBegin(X)
		if (X.isSerializer()) {
			if (!X.isMeasure())
			X.bits[X.index].set( value, length );
			X.index += length;
		} else {
			X.bits[X.index].get(value,length);
			X.index += length;
		}
		sznMethodEnd()
	};

	template<typename T>
	finline ITpl<T> I( T& value, size_t length ) {
		return ITpl<T>(value, length);
	}

	/* const int support - cI( uintmax_t ) */
	class cI : public ExplicitSerializer {
	private:
		const uintmax_t value;
		size_t length;
	public:
		finline cI( const uintmax_t _value, size_t _length )
		: value(_value), length(_length) {}

		sznMethodBegin(X)
		if (X.isSerializer()) {
			if (!X.isMeasure())
			X.bits[X.index].set(value,length);
			X.index += length;
		} else {
			uintmax_t _value = 0;
			X.bits[X.index].get(_value,length);
			if (_value != value) break;
			X.index += length;
		}
		sznMethodEnd()
	};

	/* const char* support - cT( const char* ) */
	class cT : public ExplicitSerializer {
	private:
		const char* text;

	public:
		finline cT( const char* _text )
		: text(_text) {}

		sznMethodBegin(X)
		if (X.isSerializer()) {
			size_t length = strlen(text);
			if (!X.isMeasure()) {
				for (size_t i=0; i<length; i++)
				X.bits[X.index+i*8] = (uint8_t)text[i];
			}
			X.index += length * 8;
		} else {
			size_t length = strlen(text);
			for (size_t i=0; i<length; i++) {
				char c = 0;
				X.bits[X.index+i*8].get(c);
				if (c!=text[i]) break;
			}
		}
		sznMethodEnd()
	};

	/* string and char* support, T( char* | std::string ) */
	class T : public ExplicitSerializer {
	private:
		bool isCharP;
		char** text;
		std::string* string;
		int length;

	public:
		finline T( char*& _text, int length = -1 ) :
			text(&_text), string(NULL) {
			this->isCharP = true;
			this->length = length;
		}

		finline T( std::string& _string, int length = -1 ) :
			text(NULL), string(&_string) {
			this->isCharP = false;
			this->length = length;
		}

		sznMethodBegin(X)
		if (X.isSerializer()) {
			char* textp = isCharP ? *text : (char*)string->c_str();
			size_t plength = length!= -1 ? length : strlen(textp);
			if (!X.isMeasure()) {
				size_t i;
				for (i=0; i<plength && textp[i]!=0; i++)
				X.bits[X.index+i*8] = (uint8_t)textp[i];
				if (length==-1) X.bits[X.index+i*8] = (uint8_t)0;
			}
			X.index += plength * 8 + ((length == -1) ? 8 : 0);
		} else {
			std::string s;
			size_t i = 0;
			while (length == -1 || i < (size_t)length) {
				uint8_t c = 0;
				X.bits[X.index].get(c); X.index += 8;
				if (c == 0 && length == -1) break;
				s += (char)c;
				i++;
			}
			if (isCharP) *text = strdup(s.c_str());
			else string->assign(s);
		}
		sznMethodEnd()
	};

	/* array and vector support */
	template<typename T>
	class ArrayTpl : public ExplicitSerializer {
	private:
		T*& v;
		size_t l;

	public:
		finline ArrayTpl( T*& array, size_t length ) :
		v(array), l(length) {}

		sznMethodBegin(X)
		if (X.isDeserializer()) v = new T[l];
		for (size_t i=0; i<l; i++) X && v[i];
		sznMethodEnd()
	};

	template<typename T>
	class StaticArrayTpl : public ExplicitSerializer {
	private:
		T* v;
		size_t l;

	public:
		finline StaticArrayTpl( T* array, size_t length ) :
		v(array), l(length) {}

		sznMethodBegin(X)
			for (size_t i=0; i<l; i++) X && v[i];
		sznMethodEnd()
	};

	template<typename T>
	class VectorTpl : public ExplicitSerializer {
	private:
		std::vector<T>& v;
		size_t l;

	public:
		finline VectorTpl( std::vector<T>& vec, size_t length ) :
		v(vec), l(length) {}

		sznMethodBegin(X)
		if (X.isDeserializer()) v.resize(l);
		for (size_t i=0; i<l; i++) X && v[i];
		sznMethodEnd()
	};

	template<typename X, typename T>
	finline X V( T& x ) {
		throw "ERROR: No vector serializer for this type";
	}

	template<typename T>
	finline ArrayTpl<T> A( T*& array, size_t length ) {
		return ArrayTpl<T>(array,length);
	}

	template<typename T>
	finline StaticArrayTpl<T> static_A( T* array, size_t length ) {
		return StaticArrayTpl<T>(array,length);
	}

	template<typename T>
	finline VectorTpl<T> A( std::vector<T>& array, size_t length ) {
		return VectorTpl<T>(array,length);
	}

	/* allocated virtual object support */
	template<typename T>
	class VOTpl : ExplicitSerializer {
	private:
		int variant;
		T*& obj;
	public:
		finline VOTpl( T*& mobj, int variant ) : obj(mobj) {
		}
		sznMethodBegin(X)
		if (X.isSerializer()) {
			if (obj!=NULL) X && obj;
		} else {
			obj = new T();
			X && obj;
		}
		sznMethodEnd()
	};

	template<typename T>
	finline VOTpl<T> VO( T*& obj, int variant = DEFAULT_V ) {
		return VOTpl<T>(obj,variant);
	}

	/* allocated non-virtual object serialization support */
	template<typename T>
	class OTpl : ExplicitSerializer {
	private:
		int variant;
		T*& obj;
	public:
		finline OTpl( T*& mobj, int variant ) : obj(mobj) {
		}
		sznMethodBegin(X)
		if (X.isSerializer()) {
			if (obj!=NULL) X && *obj;
		} else {
			obj = new T();
			X && *obj;
		}
		sznMethodEnd()
	};

	template<typename T>
	finline OTpl<T> O( T*& obj, int variant = DEFAULT_V ) {
		return OTpl<T>(obj,variant);
	}

} // namespace internal

//--------------------------------------------------------------------------

//= static serialization =
//= get length =

template<int __variant, typename X> finline
size_t data_length_v( const X& obj ) {
	DataStreamTpl<MEASURE, __variant> stream;
	return (stream && (X&)obj).getCurrentLength();
}

template<typename X> finline
size_t data_length( const X& obj ) {
	return data_length_v<DEFAULT_V>(obj);
}

template<Mode __mode, int __variant, typename X> finline
size_t data_serialization_v( X& obj, Data& data,
		if_is_base_of(Serializeable, X) ) {
	if (__mode == SERIALIZE) {
		DataStreamTpl<SERIALIZE, __variant> stream;
		size_t len = data_length_v<__variant>(obj);
		stream.ensureLength( len );
		return (data = (stream && obj).getData()).getLength();
	} else {
		DataStreamTpl<DESERIALIZE, __variant> stream( data );
		return (stream && obj).getCurrentLength();
	}
}

template<Mode __mode, int __variant, typename X>
size_t slow_data_serialization_v( X& obj, Data& data,
		if_is_base_of(Serializeable, X) ) {
	return slow_data_serialization_v<__mode,__variant>(obj,data);
}

template<Mode __mode, typename X>
size_t data_serialization( X& obj, Data& data,
		if_is_base_of( Serializeable, X ) ) {
	return slow_data_serialization_v<__mode, DEFAULT_V>( obj, data );
}

template<int __variant, typename X>
Data data_serialize_v( const X& obj, if_is_base_of(Serializeable, X) ) {
	Data data; data_serialization_v<SERIALIZE, __variant>( (X&)obj, data );
	return data;
}

template<typename X>
Data data_serialize( const X& obj, if_is_base_of(Serializeable, X) ) {
	return data_serialize_v<DEFAULT_V>(obj);
}

template<int __variant, typename X>
size_t data_deserialize_v( X& obj, Data data, if_is_base_of(Serializeable, X) ) {
	return data_serialization_v<DESERIALIZE,__variant>( obj, data );
}

template<typename X>
size_t data_deserialize( X& obj, Data data,
		if_is_base_of(Serializeable, X) ) {
	return data_deserialize_v<DEFAULT_V>(obj, data);
}

//= virtual serialization =

finline bool data_serialization( VSerializeable* obj, Data& data,
		Mode mode, int variant /*= DEFAULT_V*/) {
	size_t length = obj->SERIALIZATION_METHOD_NAME( mode, data, variant );
	return length != 0;
}

finline Data data_serialize( const VSerializeable* obj,
		int variant /*= DEFAULT_V*/) {
	Data data;
	data_serialization( (VSerializeable*)obj, data, SERIALIZE, variant );
	return data;
}

finline bool data_deserialize( VSerializeable* obj, Data& data,
		int variant /*= DEFAULT_V*/) {
	return data_serialization( obj, data, DESERIALIZE, variant );
}

finline size_t data_length( const VSerializeable* obj, int variant /*= DEFAULT_V*/) {
	return ((VSerializeable*)obj)->SERIALIZATION_METHOD_NAME( MEASURE,
			(Data&)Data::UNSPECIFIED, variant );
}

//--------------------------------------------------------------------------

SERIALIZATION_NS_END()

#endif /* DATASTREAM_HPP_ */
