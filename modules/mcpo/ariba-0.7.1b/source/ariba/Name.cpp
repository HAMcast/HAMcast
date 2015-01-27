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

#include "Name.h"

#include "ariba/utility/types/Identifier.h"

using ariba::utility::Identifier;

std::ostream& operator<<( std::ostream& s, const ariba::Name& n ) {
	return s << n.toString();
}

namespace ariba {

const Name Name::UNSPECIFIED;

void Name::init(const char* name, int len, bool copy, bool hreadable) {

	// delete the old buffer
	if( _bytes != NULL ) {
		delete[] _bytes;
		_bytes = NULL;
		_length = 0;
	}

	// alloc new stuff
	if (len == -1)
		len = strlen(name);

	if (copy) {

		if ( (name!=NULL) && (len>0) ){
			_bytes = new uint8_t[len];
			memcpy( _bytes, name, len );
		} else {
			len = 0;
			_bytes = NULL;
		}

	} else {
		_bytes = (uint8_t*)name;
	}

	_copy = copy;
	_length = len;
	_hreadable = hreadable;
}

Name::Name()
	: _bytes( NULL ), _length( 0 ), _copy( false ), _hreadable( false) {
}

Name::Name(const char* name, int len, bool copy)
	: _bytes( NULL ), _length( 0 ), _copy( false ), _hreadable( false) {
	init(name, len, copy, len == -1);
}

Name::Name(string name)
	: _bytes( NULL ), _length( 0 ), _copy( false ), _hreadable( false) {
	init(name.c_str(), name.length(), true, true);
}

Name::Name(const Name& name)
	: _bytes( NULL ), _length( 0 ), _copy( false ), _hreadable( false) {
	init((const char*)name.bytes(), name.length(), true, name._hreadable);
}

Name::~Name() {
	if (_copy && (_bytes!=NULL)){
		delete[] _bytes;
		_bytes = NULL;
		_length = 0;
	}
}

Name& Name::operator=( const Name& name ) {
	init((const char*)name.bytes(), name.length(), true, name._hreadable);
	return *this;
}

const uint8_t* Name::bytes() const {
	return _bytes;
}

const size_t Name::length() const {
	return _length;
}

bool Name::operator==(const Name& name) const {

	// unspecified Name objects
	if (_bytes == NULL && name._bytes == NULL &&
		length() == name.length()) return true;

	// specified name objects
	if (_bytes == NULL || name._bytes == NULL) return false;
	if (name.length() != length()) return false;
	return (memcmp(name.bytes(), bytes(), length()) == 0);
}

bool Name::operator!=(const Name& name) const {
	return !(*this == name);
}

bool Name::isUnspecified() const {
	return *this == UNSPECIFIED;
}

Name Name::random() {
	char name[17];
	srand( time(NULL) );

	for (int i=0;i<16; i++)
		name[i] = ("abcdefghijklmnopqrstuvwxyz")[((unsigned)rand())%26];
	name[16] = 0;

	// force use of the std::string ctor with this
	return Name( string(name) );
}

string Name::toString() const {
	if (_hreadable) {
		char str[256];
		for (size_t i=0; i<length(); i++) str[i] = bytes()[i];
		str[length()] = 0;
		return string(str);
	} else {
		return string("<not readable>");
	}
}

NodeID Name::toNodeId() const {
	if( bytes()==NULL || length()==0 ) return NodeID::UNSPECIFIED;
	return NodeID( Identifier::sha1(bytes(),length()) );
}

SpoVNetID Name::toSpoVNetId() const {
	if( bytes()==NULL || length()==0 ) return SpoVNetID::UNSPECIFIED;
	return SpoVNetID( Identifier::sha1(bytes(),length()) );
}

} // namespace ariba
