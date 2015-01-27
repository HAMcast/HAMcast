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
#ifndef DISTANCES_HPP_
#define DISTANCES_HPP_

/**
 * TODO: Doc
 *
 * @author Sebastian Mies <mies@tm.uka.de>
 */
namespace distances {

/// returns the maximum value of a type
template<class T>
T max_value() {
	return ~(T)0;
}

/// returns the maximum value of a type
template<class T>
T min_value() {
	return 0;
}

/// euclidean distance between two points (metric)
struct default_distance {
	template<typename T>
	T operator ()(const T& x, const T& y) {
		return abs(x - y);
	}
};

/// xor distance (metric)
struct xor_distance {
	template<typename T>
	T operator()(const T& x, const T& y) {
		return x ^ y;
	}
};

/// distance between two points on a ring (metric)
struct ring_distance {
	template<typename T>
	T operator()(const T& x, const T& y) {
		const T m = max_value<T>();
		T d_nor = (x >= y) ? (x-y) : (y-x);
		T d_inv = m - d_nor;
		return d_nor < d_inv ? d_nor : d_inv;
	}
};

/// distance from x to a predecessor y on a ring (not a metric!)
struct ring_pred_distance {
	template<typename T>
	T operator ()(const T& x, const T& y) {
		const T m = max_value<T>();
		if (x >= y) return x - y;
		else return m - y + x;
	}
};

/// distance from x to a successor y on a ring (not a metric!)
struct ring_succ_distance {
	template<typename T>
	T operator ()(const T& y, const T& x) {
		const T m = max_value<T>();
		if (x >= y) return x - y;
		else return m - y + x;
	}
};

}

#endif /* DISTANCES_HPP_ */
