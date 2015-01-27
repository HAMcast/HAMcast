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

#ifndef DATA_HPP_
#define DATA_HPP_

//== library includes ==
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <boost/cstdint.hpp>
#include <boost/type_traits/make_unsigned.hpp>

// forward declaration
template<typename T> class DefaultDataModel;
template<typename T = uint8_t, typename DataModel = DefaultDataModel<uint8_t> > class DataTpl;
typedef DataTpl<> Data;
template<typename T, typename DataModel> std::ostream& operator<<(std::ostream& stream, const DataTpl<T, DataModel>& data);

//== internal includes ==
#include "../internal/Utilities.hpp"

//== local includes ==
#include "DataUtilities.hpp"
#include "Serialization.hpp"

/**
 * This class implements a wrapper for binary data. And this time, when the
 * word binary is used -- it really means binary! Length and manipulation
 * are specified and operate in bit precision. Most of the operations are
 * highly effective using inline functions and compiler optimizations.
 *
 * Two versions of binaries are supported: static, non-resizeable binaries
 * and dynamic, resizeable ones. Please note, this class does not manage
 * memory, reference counting etc. you have to take care about memory
 * management yourself! However you can use the <code>release()</code>
 * to delete the underlaying array.
 *
 * @author Sebastian Mies
 */
template<typename T, typename DataModel>
class DataTpl: public Serializeable { SERIALIZEABLE
protected:
	typedef DataTpl<T,DataModel> _Data;
	DataModel model;

public:
	static const size_t word_width = sizeof(T) * 8;

	class DataManipulator {
	private:
		DataModel bits;
		size_t index;

	public:
		finline DataManipulator( DataModel& _bits, size_t _index) :
			bits(_bits), index(_index) {
		}

		template<typename X>
		finline operator X() const {
			return bitget<X>(bits.buffer(), index);
		}

		template<typename X>
		finline DataManipulator& operator=(X value) {
			bitset(value, bits.buffer(), index );
			return *this;
		}

		template<typename X>
		finline void set(X value, size_t length = sizeof(X) * 8, if_uint(X)) {
			bitcpy(value, 0, bits.buffer(), index, length);
		}

		template<typename X>
		finline void set(X value, size_t length = sizeof(X) * 8, if_int(X)) {
			set(_unsigned(value),length);
		}

		template<typename X>
		finline void get(X& value, size_t length = sizeof(X) * 8, if_uint(X)) const {
			value = bitget<X> (bits.buffer(), index, length);
		}

		template<typename X>
		finline void get(X& value, size_t length = sizeof(X) * 8, if_int(X)) const {
			typedef typename boost::make_unsigned<X>::type unsigned_type;
			_unsigned(value) = bitget<unsigned_type> (bits.buffer(), index, length);
		}

		finline void get(bool& value) const {
			value = bitget( bits.buffer(), index );
		}
	};

public:
	static const Data UNSPECIFIED;

	/**
	 * Constructs a dynamic bit-data object of variable size.
	 */
	finline DataTpl() : model() {
	}

	/**
	 * Contructs a copy of an existing data buffer
	 */
	finline DataTpl(const DataTpl<T, DataModel>& copy) {
		this->model = copy.model;
	}

	finline DataTpl( const DataModel& _model ) : model( _model ) {
	}

	/**
	 * Constructs a static bit-data object with the given buffer.
	 */
	finline DataTpl(const T* buffer, size_t length) {
		model.buffer() = buffer;
		model.length() = length;
	}

	/**
	 * Constructs a dynamic bit-data object with the given buffer.
	 */
	finline DataTpl( T* buffer, size_t length ) {
		model.buffer() = buffer;
		model.length() = length;
	}

	/**
	 * Constructs a dynamic bit-data object of variable size with the given
	 * initial size.
	 */
	finline DataTpl(size_t length) : model() {
		model.resize(length);
	}


	/**
	 * Returns the internal buffer of the data.
	 */
	finline T* getBuffer() const {
		return model.buffer();
	}

	/**
	 * Returns the length of the data in bits
	 */
	finline size_t getLength() const {
		return model.length();
	}
	/**
	 * Sets the length of the data in bits
	 */
	void setLength(size_t new_length) {
		model.resize(new_length);
	}

	/**
	 * Ensures that the buffer pointer has the given
	 * number of bits of memory reserved.
	 *
	 * @param neededLength The minimum data length required.
	 */
	finline void ensureLength( size_t neededLength ) {
		if ((int) neededLength > model.length() ) model.resize(neededLength);
	}

	/**
	 * Returns a manipulator object for a certain bit position.
	 *
	 * @param index The index in bits quantitites.
	 * @return A data manipulation object
	 */
	finline DataManipulator operator[](size_t index) {
		return DataManipulator(model, index);
	}

	/**
	 * Returns a constant manipulator object for a certain bit position.
	 *
	 * @param index The index in bits quantitites.
	 * @return A constant data manipulation object
	 */
	finline const DataManipulator operator[](size_t index) const {
		return DataManipulator(model, index);
	}

	_Data sub( size_t index, size_t length = ~0 ) {
		if (length == ~0) length = model.length()-index;
		return _Data(model.sub(index,length));
	}

	/**
	 * Returns a copy of the specified bit range.
	 *
	 * @param index The first bit to copy
	 * @param length The length of the bit range
	 * @return The cloned data object
	 */
	_Data clone( size_t index, size_t length ) const {
		DataModel new_model = model.clone(index,length);
		return _Data(new_model);
	}

	/**
	 * Returns a copy of the data.
	 *
	 * @return A cloned data object
	 */
	_Data clone() const {
		DataModel new_model = model.clone( 0, model.length() );
		return _Data(new_model);
	}

	/**
	 * Returns true, if the data is empty (zero length)
	 *
	 * @return True, if the data is empty (zero length)
	 */
	finline bool isEmpty() const {
		return (model.length() == 0);
	}

	/**
	 * Returns true, if the data buffer is unspecified.
	 * In this case no memory is associated with this data object.
	 *
	 * @return True, if the data buffer is unspecified.
	 */
	finline bool isUnspecified() const {
		return model.isUnspecified();
	}

	/**
	 * This method frees the memory associated with
	 * this data object and sets this object into an unspecified
	 * state.
	 */
	finline void release() {
		model.release();
	}

	// operators

	/**
	 * Assigns another buffer
	 */
	template<typename X>
	_Data& operator= (const DataTpl<X>& source) {
		this->model = source.model;
		return *this;
	}

	/**
	 * Returns true, if the datas buffer's pointer and length
	 * are the same.
	 */
	template<typename X>
	finline bool operator==( DataTpl<X>& data) {
		return (data.model.buffer() == model.buffer() &&
				data.model.length() == model.length() );
	}

	finline _Data& operator&=(_Data& data) {
		return *this;
	}

	finline _Data& operator|=(_Data& data) {
		return *this;
	}

	finline _Data& operator^=(_Data& data) {
		return *this;
	}

	/* implied operators */

	finline _Data operator&(_Data& data) {
		return (this->clone() &= data);
	}

	finline _Data operator|(_Data& data) {
		return (this->clone() |= data);
	}

	finline _Data operator^(_Data& data) {
		return (this->clone() ^= data);
	}
};

/* unspecified type */
template<typename T, typename DataModel>
const Data DataTpl<T, DataModel>::UNSPECIFIED;

/**
 * This class implements the default data model
 *
 * @author Sebastian Mies
 */
template<typename _T>
class DefaultDataModel {
public:
	typedef _T T;
	typedef DefaultDataModel<T> _Model;

private:

	int32_t bufferLen;
	T* bufferPtr;

	static finline int calcLength(int length) {
		if (length<0) return 0;
		return ((length/8)/sizeof(T)+1);
	}
public:
	finline DefaultDataModel() {
		bufferPtr = NULL;
		bufferLen = -1;
	}

	finline DefaultDataModel( void* buffer, size_t length ) {
		bufferPtr = (T*)buffer;
		bufferLen = length;
	}

	finline DefaultDataModel( const _Model& source ) {
		this->bufferPtr = source.bufferPtr;
		this->bufferLen = source.bufferLen;
	}

	finline _Model& operator=( const _Model& source ) {
		this->bufferPtr = source.bufferPtr;
		this->bufferLen = source.bufferLen;
		return *this;
	}

	finline T*& buffer() {
		return bufferPtr;
	}

	finline T* buffer() const {
		return bufferPtr;
	}

	finline int32_t& length() {
		return bufferLen;
	}

	finline int32_t length() const {
		return (bufferLen == -1) ? 0 : bufferLen;
	}

	finline bool isUnspecified() const {
		return bufferLen < 0;
	}

	finline void resize( size_t new_length ) {
		size_t old_length = calcLength(bufferLen);
		size_t res_length = calcLength(new_length);
		if (old_length != res_length) {

			if(res_length <= 0){
				if (bufferPtr != NULL) delete [] bufferPtr;
				bufferPtr = NULL;
				bufferLen = 0;
			}else{
				T* new_buffer = new T[res_length];
				if (new_buffer != NULL) memset(new_buffer, 0, res_length*sizeof(T));
				if (bufferPtr != NULL) {
					size_t clength = res_length < old_length ? res_length : old_length;
					memcpy( new_buffer, bufferPtr, clength*sizeof(T) );
					delete [] bufferPtr;
				}
				bufferPtr = new_buffer;
				bufferLen = new_length;
			}
		}
	}

	finline void release() {
		if (bufferPtr!=NULL && bufferLen>=0) delete [] bufferPtr;
		bufferPtr = NULL;
		bufferLen = -1;
	}

	finline _Model sub( size_t index, size_t length ) {
		return _Model( bufferPtr + index/sizeof(T), length );
	}

	finline _Model clone( size_t index, size_t length ) const {
		_Model new_model;
		new_model.resize( length );
		bitcpy( this->buffer(), index, new_model.buffer(), 0, length );
		return new_model;
	}
};

/* serialization */
sznBeginDefault( Data, X ){
	for (size_t i = 0; i< getLength() / word_width; i++) X && getBuffer()[i];
}sznEnd();

/* default human readable text output */
template<typename T, typename DataModel>
std::ostream& operator<<(std::ostream& stream, const DataTpl<T, DataModel>& data) {
	stream << "[" << bitstr(data.getBuffer(), data.getLength(), 4)
			<< "|'";
	const char* buffer = (const char*) data.getBuffer();
	for (size_t i = 0; i < data.getLength() / 8; i++) {
		char c = buffer[i] < 32 ? '.' : buffer[i];
		stream << c;
	}
	stream << "']";
	return stream;
}
#endif /* DATA_HPP_ */
