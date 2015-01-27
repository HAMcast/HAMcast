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
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OF TELEMATICS OR
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
#ifndef MINIMIZER_TABLE_HPP_
#define MINIMIZER_TABLE_HPP_

#include <vector>
#include <iostream>
#include "table_listener.hpp"

using std::vector;
using std::ostream;

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
template<class Value, class Comparator, typename Listener = table_listener>
class minimizer_table: public vector<Value> {
private:
	// define table type
	typedef vector<Value> super;

	// members
	size_t max_size;
	Comparator compare;

	// listener
	Listener& listener;

public:

	/// delegates from vector<Value>
	typedef	typename super::iterator iterator;

	/// delegates from vector<Value>
	typedef typename super::const_iterator const_iterator;

	/// explicitly construct a minimizer table
	explicit minimizer_table(size_t max_size, Comparator compare = Comparator(),
			Listener& listener = Listener::DEFAULT) :
			super(), max_size(max_size), compare(compare), listener(listener) {
	}

	/// returns true, if the value would fit into the table
	bool insertable( const Value& value ) {
		return insert(value,true) != super::end();
	}

	/// inserts a value into an ordered list
	iterator insert(const Value& value, bool simulate = false ) {
		iterator iter = super::end();

		// table empty? -> just add entry
		if (super::size() == 0) {
			if (!simulate) super::push_back(value);
			iter = super::end()-1;
			if (!simulate) listener.on_table( table_listener::insert, *this, iter );
		} else

		// add at correct position
		for (iterator i = super::begin(); i != super::end(); i++) {
			int cmp = compare(value, *i);
			if (cmp < 0) {
				if (!simulate) {
					iter = super::insert(i, value);
				} else
					iter = i;
				if (!simulate)
					listener.on_table( table_listener::insert, *this, iter );
				break;
			} else
			if (cmp == 0) return iter;
		}

		// no item smaller than item and table not full? -> append
		if (iter == super::end() && super::size() != max_size) {
			if (!simulate)
			super::push_back(value);
			iter = super::end()-1;
			if (!simulate) listener.on_table( table_listener::insert, *this, iter );

		} else

		// drop entries
		if (super::size() > max_size) {
			if (!simulate) {
				listener.on_table( table_listener::remove, *this, super::end()-1 );
				super::pop_back();
			}
		}
		return iter;
	}

	/// remove value from ordered list
	bool remove(const Value& value) {
		for (iterator i = super::begin(); i != super::end(); i++)
		if (*i == value) {
//			listener.on_table( table_listener::remove, *this, i );
			super::erase(i);
			return true;
		}
		return false;
	}

	/// check whether this list contains a specific value
	bool contains(const Value& value) const {
		for (const_iterator i = super::begin(); i != super::end(); i++)
		if (*i == value) return true;
		return false;
	}

	Comparator& get_compare() {
		return compare;
	}
};

/// prints a list of items
template<class V, class C, class L>
std::ostream& operator<<(std::ostream& s, const minimizer_table<V, C, L>& v) {
	typedef minimizer_table<V, C, L> t;
	for (typename t::const_iterator i = v.begin(); i != v.end(); i++) {
		if (i != v.begin()) s << ",";
		s << *i;
	}
	return s;
}

#endif /* MINIMIZER_TABLE_HPP_ */
