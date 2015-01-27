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

#include "TestSerialization.h"
#include "Data.hpp"
#include "DataStream.hpp"
#include "Serialization.hpp"

using_serialization;

class Aclass: public VSerializeable { VSERIALIZEABLE;
public:
	uint32_t x;
	uint8_t y;
	int8_t z;
	bool b;
	char* text;
	std::string str;
	std::vector<uint8_t> v;
	Aclass* vclass;
	bool vcls;
	uint8_t* static_array;

	Aclass(bool vcls = false) {
		text = new char[80];
		static_array = new uint8_t[20];
		for (size_t i=0; i<20; i++) static_array[i] = i;
		strcpy(text, "Hallo!");
		str = "std::string:)";
		x = 0x01020304;
		y = 0xF;
		z = -2;
		b = 0;
		this->vcls = vcls;
		v.push_back(0xA0);
		v.push_back(0xB0);
		v.push_back(0xC0);
		v.push_back(0xD0);
		if (vcls) vclass = new Aclass();
		else vclass = NULL;
	}

	void clean() {
		text = NULL;
		str = "";
		x = 0;
		y = 0;
		b = 1;
		z = 0;
		v.clear();
		vclass = NULL;
	}

	void view(bool ret = true) {
		printf("obj=[%08X, %1X, %d, %d, '%s', '%s' ", x, y, z, b, text, str.c_str());
		for (size_t i = 0; i < v.size(); i++)
			printf("%02X ", v[i]);
		if (vclass != NULL) vclass->view(false);
		printf("]");
		if (ret) printf("\n");
	}
};

sznBeginDefault( Aclass, X ){
	double xpos;
	X && x && b && I(y,6) && T(text) && T(str) && A(v,4) && I(z) && vcls;
	X && static_A(static_array,20);
	X && static_A( (uint8_t*)&xpos, 8 );
	if (vcls) X && VO(vclass);
}sznEnd()

vsznDefault( Aclass );

int test_serialization() {
	using namespace std;

	Aclass a(true);
	Data data = data_serialize(a);
	a.view();
	cout << "length=" << data_length(a) / 8 << endl;
	cout << "data=" << data << endl;

	Aclass b(true);
	b.clean();
	b.view();
	data_deserialize(b, data);
	b.view();

	VSerializeable *c = &b;
	cout << "length=" << data_length(c) / 8 << endl;
	cout << "data=" << (data = data_serialize(c)) << endl;
	Aclass d;
	d.clean();
	data_deserialize(&d, data);
	d.view();
	cout << "--- test successful." << endl;

	return 0;
}


