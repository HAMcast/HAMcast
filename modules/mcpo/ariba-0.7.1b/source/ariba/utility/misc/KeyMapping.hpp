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

#ifndef KEY_MAPPING_H__
#define KEY_MAPPING_H__

#include <map>

using std::map;
using std::pair;

namespace ariba {
namespace utility {

/**
* Template class for the administration
* of a mapping to an index key with networks
*/
template<class T>
class KeyMapping {
private:
	typedef map<T,unsigned long> KeyMap; //< maps an item to an id, e.g. a node pair to a linkid
	typedef map<unsigned long, KeyMap> NetworkKeyMap; //< maps the network to the KeyMap, which map is responsible for this network?

	typedef typename KeyMap::iterator KeyMapIterator;
	typedef typename NetworkKeyMap::iterator NetworkKeyMapIterator;

	NetworkKeyMap networkKeyMap;

	inline unsigned long nextid(){
		return rand()+time(0);
	}

public:
	inline KeyMapping(){
		srand( time(NULL) );
	}

	inline ~KeyMapping(){
	}

	inline bool exists(unsigned long network, T item){

		NetworkKeyMapIterator i = networkKeyMap.find( network );
		if( i == networkKeyMap.end()) return false;

		KeyMapIterator k = i->second.find( item );
		return ( k != i->second.end() );
	}

	inline unsigned long get(unsigned long network, T item){
		assert( exists(network, item) );

		NetworkKeyMapIterator i = networkKeyMap.find( network );
		KeyMapIterator k = i->second.find( item );

		return k->second;
	}

	inline unsigned long insert(unsigned long network, T item){

		KeyMap* keyMap = NULL;

		// if we have no link map yet for this network,
		// insert one and get the reference back to work with

		NetworkKeyMapIterator i = networkKeyMap.find( network );

		if( i == networkKeyMap.end() ){
			pair<NetworkKeyMapIterator, bool> ret =
				networkKeyMap.insert( make_pair(network,KeyMap()) );

			keyMap = &(ret.first->second);
		} else {

			keyMap = &(i->second);
		}

		unsigned long key = nextid();
		keyMap->insert( make_pair( item, key) );

		assert( get(network,item) == key );
		return key;
	}

	inline void remove(unsigned long network, T item){
		NetworkKeyMapIterator i = networkKeyMap.find( network );
		if( i == networkKeyMap.end() ) return;

		i->second.erase( item );
	}

}; // class KeyMapping

}} // namespave spovnet, common

#endif // KEY_MAPPING_H__

