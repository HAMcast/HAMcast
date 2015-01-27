// varray.hpp, created on 04.11.2008 by Sebastian Mies
//
// [The FreeBSD Licence]
// Copyright (c) 2008
// Sebastian Mies, Institute of Telematics, Universität Karlsruhe
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// * Neither the name of the author nor the names of its contributors may be
//   used to endorse or promote products derived from this software without
//   specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#ifndef VARRAY_HPP_
#define VARRAY_HPP_

#include "detail/helper.hpp"

#include <memory.h>

#include <boost/mpl/if.hpp>
#include <boost/mpl/placeholders.hpp>

/**
 * This class implements a variable or static sized bit container.
 * The difference to other array classes is that this class morphs between
 * static and dynamic object allocation.
 *
 * @author Sebastian Mies
 */
template<class item_type, size_t _size = 0, typename size_type = size_t>
class varray {
private:
	typedef varray<item_type, _size, size_type> _varray;

	/* dynamic array */
	class dynamic_array {
	private:
		/* array structure */
		typedef struct __array {
			size_type size;
			item_type arr[];
		} __array;
		__array *arr;

		/* returns the underlaying array size from a given length */
		finline size_t array_size(size_t new_size) const {
			return (new_size / (sizeof(item_type) * 8)) + (((new_size
					% (sizeof(item_type) * 8)) == 0) ? 0 : 1);
		}

	public:
		finline dynamic_array() {
			arr = 0;
		}

		finline ~dynamic_array() {
			if (arr != 0) free(arr);
		}

		/* return size in bits */
		finline size_type size() const {
			return (arr == 0) ? 0 : arr->size;
		}

		/* returns the underlaying array size */
		finline size_t array_size() const {
			return array_size(size());
		}

		/* set size in bits */
		finline void resize(size_type new_size) {
			size_t old_arr_size = array_size();
			size_t new_arr_size = array_size(new_size);
			if (old_arr_size != new_arr_size) {
				size_t nbsize = sizeof(__array ) + new_arr_size
						* sizeof(item_type);
				size_t obsize = sizeof(__array ) + old_arr_size
						* sizeof(item_type);
				__array *new_array = (__array *) malloc(nbsize);
				if (arr != 0) {
					size_t csize = std::min(nbsize, obsize);
					memcpy(new_array, arr, csize);
					if (nbsize > obsize) memset(
							((uint8_t*) new_array) + obsize, 0, nbsize
									- obsize);
					free(arr);
				}
				arr = new_array;
			}
			arr->size = new_size;
		}

		/* return the array */
		finline item_type* array() const {
			return (item_type*) &arr->arr;
		}

		finline int get_memory_consumption() const {
			return array_size() * sizeof(item_type) + sizeof(__array) +
				sizeof(void*);
		}
	};

	/* static array */
	class static_array {
	private:
		static const size_t _array_size = (_size / (sizeof(item_type) * 8))
		+ (((_size % (sizeof(item_type) * 8)) == 0) ? 0 : 1);
						item_type arr[_array_size];

	public:
		/* returns the number of bits in this array */
		finline size_type size() const {
			return _size;
		}

		/* returns the underlaying array size */
		finline size_t array_size() const {
			return _array_size;
		}

		/* resizes the array */
		finline void resize(size_type new_size) {
		}

		/* return the array */
		finline item_type* array() const {
			return (item_type*) &arr;
		}

		finline int get_memory_consumption() const {
			return _array_size * sizeof(item_type);
		}
	};

	/* selection of static or dynamic array */
	typename boost::mpl::if_<boost::mpl::bool_<_size == 0>,
	dynamic_array, static_array>::type _array;

public:
	finline bool is_static() const {
		return (_size != 0);
	}

	finline bool is_dynamic() const {
		return !is_static();
	}

	finline size_type size() const {
		return _array.size();
	}

	finline size_t array_size() const {
		return _array.array_size();
	}

	finline void resize(size_type newSize) {
		_array.resize(newSize);
	}

	finline item_type* array() {
		return _array.array();
	}

	finline const item_type* array_const() const {
		return _array.array();
	}

	finline operator item_type*() {
		return _array();
	}

	finline int get_memory_consumption() const {
		return _array.get_memory_consumption();
	}

	finline item_type& operator[] ( size_type index ) {
		return array()[index];
	}
};

#endif /* VARRAY_HPP_ */
