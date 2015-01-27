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

#ifndef DEMULTIPLEXER_H__
#define DEMULTIPLEXER_H__

#include <list>
#include <iostream>
#include <map>
#include <boost/thread/mutex.hpp>
#include <boost/foreach.hpp>
#include "ariba/utility/messages/Message.h"

using std::cout;
using std::list;
using std::map;
using std::pair;
using ariba::utility::Message;

namespace ariba {
namespace utility {

template<typename S, typename T>
class Demultiplexer
{
private:

	typedef map<S,T> 					SERVICE_LISTENER_MAP;
	typedef pair<S,T> 					SERVICE_LISTENER_PAIR;
	typedef typename SERVICE_LISTENER_MAP::iterator 	SERVICE_LISTENER_MAP_ITERATOR;
	typedef typename SERVICE_LISTENER_MAP::const_iterator 	SERVICE_LISTENER_MAP_CITERATOR;

	typedef map<T,S> 					LISTENER_SERVICE_MAP;
	typedef pair<T,S> 					LISTENER_SERVICE_PAIR;
	typedef typename LISTENER_SERVICE_MAP::iterator   	LISTENER_SERVICE_MAP_ITERATOR;
	typedef typename LISTENER_SERVICE_MAP::const_iterator   LISTENER_SERVICE_MAP_CITERATOR;

	SERVICE_LISTENER_MAP          				mapServiceListener;
	LISTENER_SERVICE_MAP 					mapListenerService;
	boost::mutex						mapMutex;

	void debugprint() {
		cout << "-------------start--------" << std::endl;
		{
			LISTENER_SERVICE_MAP_CITERATOR i = mapListenerService.begin();
			LISTENER_SERVICE_MAP_CITERATOR iend = mapListenerService.end();

			for( ; i != iend; i++ )
				cout << "xxx" << i->first.toString() << " -> " << i->second << std::endl;
		}
		cout << "-----------------------" << std::endl;
		{
			SERVICE_LISTENER_MAP_CITERATOR i = mapServiceListener.begin();
			SERVICE_LISTENER_MAP_CITERATOR iend = mapServiceListener.end();

			for( ; i != iend; i++ )
				cout << "xxx" << i->first << " -> " << i->second.toString() << std::endl;
		}
		cout << "-------------end---------" << std::endl;
	}

public:

	Demultiplexer() {
	}

	~Demultiplexer() {
	}

	void registerItem( S id, T listener ) {
		boost::mutex::scoped_lock lock( mapMutex );

		mapServiceListener.insert( SERVICE_LISTENER_PAIR( id, listener ) );
		mapListenerService.insert( LISTENER_SERVICE_PAIR( listener, id ) );
	}

	void unregisterItem( S id ) {
		T listener = get( id );

		{
			boost::mutex::scoped_lock lock( mapMutex );
			mapServiceListener.erase( id );
			mapListenerService.erase( listener );
		}
	}

	void unregisterItem( T listener ) {
		S id = get( listener );
		unregisterItem( id );
	}

	S get( T listener ) {
		boost::mutex::scoped_lock lock( mapMutex );

		LISTENER_SERVICE_MAP_CITERATOR it = mapListenerService.find( listener );
		return it->second;
	}

	T get( S id ) {
		boost::mutex::scoped_lock lock( mapMutex );

		SERVICE_LISTENER_MAP_CITERATOR it = mapServiceListener.find( id );
		if( it == mapServiceListener.end() ) 	return NULL;
		else					return it->second;
	}

	bool contains( T listener ) {
		boost::mutex::scoped_lock lock( mapMutex );

		LISTENER_SERVICE_MAP_CITERATOR it = mapListenerService.find( listener );
		return ( it != mapListenerService.end() );
	}

	bool contains( S id ) {
		boost::mutex::scoped_lock lock( mapMutex );

		SERVICE_LISTENER_MAP_CITERATOR it = mapServiceListener.find( id );
		return ( it != mapServiceListener.end() );
	}

	typedef list<S> OneList;
	typedef list<T> TwoList;

	OneList getOneList() {
		boost::mutex::scoped_lock lock( mapMutex );
		OneList ret;

		BOOST_FOREACH( SERVICE_LISTENER_PAIR i, mapServiceListener ){
			ret.push_back( i.first );
		}

		return ret;
	}

	TwoList getTwoList() {
		boost::mutex::scoped_lock lock( mapMutex );
		TwoList ret;

		BOOST_FOREACH( SERVICE_LISTENER_PAIR  i, mapServiceListener ){
			ret.push_back( i.first );
		}

		return ret;
	}

};

}} // namespace ariba, common

#endif // DEMULTIPLEXER_H__
