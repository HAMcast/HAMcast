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
#ifndef CHORD_ROUTING_TABLE_HPP_
#define CHORD_ROUTING_TABLE_HPP_

#include <vector>
#include <iostream>

#include <boost/foreach.hpp>

#include "distances.hpp"
#include "comparators.hpp"
#include "minimizer_table.hpp"

#include "ariba/Identifiers.h"

using namespace ariba;

using namespace distances;
using namespace comparators;
using namespace std;

// placeholders for route info and nodeid type
typedef ariba::NodeID nodeid_t;
typedef ariba::LinkID routeinfo_t;

/**
 * Implementation of a chord routing table.
*
 * @author Sebastian Mies <mies@tm.uka.de>
 */
class chord_routing_table {
private:
	typedef chord_routing_table self;
	typedef distance_compare<nodeid_t&, nodeid_t, ring_succ_distance>
			succ_compare_type;
	typedef distance_compare<nodeid_t&, nodeid_t, ring_pred_distance>
			pred_compare_type;
	typedef distance_compare<nodeid_t, nodeid_t, ring_distance>
			finger_compare_type;

public:
	typedef minimizer_table<nodeid_t, succ_compare_type, self> succ_table;
	typedef minimizer_table<nodeid_t, pred_compare_type, self> pred_table;
	typedef minimizer_table<nodeid_t, finger_compare_type, self> finger_table;

	// a routing table entry
	class item {
	public:
		uint8_t ref_count;
		nodeid_t id;
		routeinfo_t info;
	};
	typedef vector<item> route_table;


private:
	// maximum number of fingers
	static const size_t max_fingers = 32;

	// the own node id
	nodeid_t id;

	// successor and predecessor tables
	succ_table succ;
	pred_table pred;
	finger_table* finger[max_fingers*2];

	// main routing table
	route_table table;

	// some internal flags
	item* item_added;
	item item_removed;
	bool item_removed_flag;
	bool simulate;

	// finds an item inside the routing table
	item* find_item(const nodeid_t& id) {
		BOOST_FOREACH( item& i, table )
			if ( i.id == id ) return &i;
		return NULL;
	}

	/// Adds a item to the routing table
	item* add_item( const nodeid_t& id ) {
		item i;
		i.id = id;
		i.ref_count = 1;
		table.push_back(i);
		item_added = &table.back();
		return item_added;
	}

	/// Removes an item from the routing table
	bool remove_item( const nodeid_t& id ) {
		for (route_table::iterator i = table.begin(); i!=table.end(); i++) {
			if ( (*i).id == id ) {
				(*i).ref_count--;
				if ((*i).ref_count == 0) {
					item_removed = *i;
					item_removed_flag = true;
					return true;
				}
				break;
			}
		}
		return false;
	}

public:

	// handles events from minimizers
	template<class Table>
	void on_table( table_listener::type_ type, const Table& tab, typename Table::iterator pos ) {
		switch (type) {
		case table_listener::insert: {
			item* i = find_item( *pos );
			if (i == NULL) i = add_item( *pos ); else i->ref_count++;
			break;
		}
		case table_listener::remove: {
			remove_item(*pos);
			break;
		}
		case table_listener::update:
			break;
		}
	}


public:
	/// constructs the reactive chord routing table
	explicit chord_routing_table( const nodeid_t& id, int redundancy = 4 ) :
		id(id),	succ( redundancy, succ_compare_type(this->id), *this ),
		pred( redundancy, pred_compare_type(this->id), *this ) {

		// create finger tables
		nodeid_t nid = Identifier(2);
		for (size_t i=0; i<max_fingers; i++) {
			finger[i*2+0] =
				new finger_table( redundancy,
						finger_compare_type(this->id - nid), *this);
			finger[i*2+1] =
				new finger_table( redundancy,
						finger_compare_type(this->id + nid), *this);
			nid = nid << 1;
		}
	}

	virtual ~chord_routing_table() {
		BOOST_FOREACH( finger_table* f, this->finger){
			delete f;
		}
	}

	/// check whether a node could fit the routing table
	bool is_insertable( const nodeid_t& value ) {
//		if (find_item(value)!=NULL) return false;
		bool is_insertable_ = false;
		is_insertable_ |= succ.insertable(value);
		is_insertable_ |= pred.insertable(value);
		for (size_t i=0; i<max_fingers*2; i++)
			is_insertable_ |= finger[i]->insertable(value);
		return is_insertable_;
	}

	/// searches an item
	item* get( const nodeid_t& value ) {
		return find_item(value);
	}

	inline item* operator[] ( size_t index ) {
		return &table[index];
	}

	inline size_t size() const {
		return table.size();
	}

	/// adds a node
	item* insert( const nodeid_t& value ) {
		if (value==id) return NULL;
		item_added = NULL;
		item_removed_flag = false;
		succ.insert(value);
		pred.insert(value);
		for (size_t i=0; i<max_fingers*2; i++) finger[i]->insert(value);
		return item_added;
	}

	/// adds an orphan
	item* insert_orphan( const nodeid_t& value ) {
		item* it = find_item(value);
		if (it!=NULL) return it;
		item i;
		i.id = id;
		i.ref_count = 0;
		table.push_back(i);
		return &table.back();
	}

	/// removes an node
	const item* remove( const nodeid_t& value ) {
		item_removed_flag = false;
		succ.remove(value);
		pred.remove(value);
		for (size_t i=0; i<max_fingers*2; i++) finger[i]->remove(value);
		if (!item_removed_flag) remove_orphan(value);
		return item_removed_flag ? &item_removed : NULL;
	}

	/// removes an orphan
	item* remove_orphan( const nodeid_t& value ) {
		item_removed_flag = false;
		remove_item(value);
		return item_removed_flag ? &item_removed : NULL;
	}

	/// returns the next hop
	const item* get_next_hop( const nodeid_t& value, bool nts = false) {
		ring_distance distance;
		item* best_item = NULL;
		for (size_t i=0; i<table.size(); i++) {
			item* curr = &table[i];
			if (nts && curr->id == value) continue;

			// not not include orphans into routing!
			if (curr->ref_count==0) continue;

			// check if we found a better item
			if (best_item==NULL) {
				best_item = curr;
				continue;
			} else {
				if (distance(value, curr->id)<distance(value, best_item->id))
					best_item = curr;
			}
		}
		if (best_item != NULL && distance(value, id)<distance(value, best_item->id))
			return NULL;
		return best_item;
	}

	const nodeid_t* get_successor() {
		if (succ.size()==NULL) return NULL;
		return &succ.front();
	}

	const nodeid_t* get_predesessor() {
		if (pred.size()==NULL) return NULL;
		return &pred.front();
	}

	bool is_closest_to( const nodeid_t& value ) {
		ring_distance distance;
		nodeid_t delta = distance(value, id);
		if (get_successor() != NULL &&
				delta > distance(value, *get_successor() ) ) return false;
		if (get_predesessor() != NULL &&
				delta > distance(value, *get_predesessor() ) ) return false;
		return true;
	}

	/// returns the whole routing table
	vector<item>& get_table() {
		return table;
	}

	/// returns the last removed item
	const item* get_removed_item() const {
		return item_removed_flag ? &item_removed : NULL;
	}

	/// return successor table
	const succ_table& get_succ_table() const {
		return succ;
	}

	/// return predecessor table
	const pred_table& get_pred_table() const {
		return pred;
	}

	/// return finger table
	finger_table& get_finger_table( size_t index ) {
		return *finger[index];
	}

	/// return number of finger tables
	size_t get_finger_table_size() const {
		return max_fingers*2;
	}
};

/// output routing table
std::ostream& operator<<(std::ostream& s, const chord_routing_table& t) {
	s << "[pred=" << t.get_pred_table() << " succ=" << t.get_succ_table()
			<< "]";
	return s;
}

#endif /* CHORD_ROUTING_TABLE_HPP_ */
