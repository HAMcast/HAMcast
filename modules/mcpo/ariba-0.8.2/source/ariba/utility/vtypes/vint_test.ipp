// vint_test.cpp, created 11.11.2008 by Sebastian Mies
// [The FreeBSD Licence]
//
// Copyright (c) 2008
//     Sebastian Mies, Institute of Telematics, Universität Karlsruhe (TH)
//     All rights reserved.
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
#include <iostream>
using namespace std;

#include "vint.hpp"

int test1() {
	vint<4> u = 15;
	vint<4> v = u * 2;
	return v;
}

int test2() {
	vint<8> u = 255;
	u *= 3;
	return u;
}

bool vint_test() {
	char text[4] = { 0x12, 0x34, 0x56, 0x78 };
	cout << _uint(text) << endl;
	const char* num =  "123456789123456789123456789123456789";

	vint<> x = 99999u;
	cout << "x = " << x.to_debug_string(10,20) << " = 99999" << endl;
	cout << "x == 99999  ? => " << (x == 99999) << endl;
	cout << "x <  100000 ? => " << (x < 100000) << endl;
	cout << "x >  100000 ? => " << (x > 100000) << endl;
	cout << "x <  99998  ? => " << (x < 99998 ) << endl;
	cout << "x >  99998  ? => " << (x > 99998 ) << endl;
	cout << x.convert<uint16_t>() << endl;

	vint<128> i = num;
	cout << "i=" << i.to_debug_string() << endl;

	vint<> j = "876543219876543219876543219876543211";
	cout << "j=" << j.to_debug_string() << endl;

	vint<> k = i + j;
	cout << "k=i+j=" << k.to_debug_string() << endl;

/*	j = "1000";
	vint<> l = i * j;
	cout << "l=i*2=" << l.to_debug_string() << endl;
	cout << "l=i*2=" << l.normalized().to_debug_string() << endl;
*/
	vint<4> u = 15;
	vint<4> v = u * 2;

	cout << "u = " << u.to_debug_string() << endl;
	cout << "v = u * 2 = " << v.to_debug_string() << endl;
	cout << "u < v = " << (u<v) << endl;

	vint<8> i1 = 231;
	vint<8> i2 = 23;
	vint<8> i3 = 255;

	cout << i3 << ".is_between_ring(231,10)=" << i3.is_between_ring(231,10) << endl;
	cout << i3 << ".is_between_ring(10,231)=" << i3.is_between_ring(10,231) << endl;

	return true;
}

