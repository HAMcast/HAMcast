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

/* This file implements some common bit operations for unsigned integer types
 * and arrays. There are two different approaches in indexing bits inside arrays
 * and integrals:
 *
 * In simple integer types the least significant bit (LSB) is identified by index zero.
 * A length is specified from the LSB to the MSB. On the other hand, there is an
 * inverted form of this representation. In this case, the MSB is identfied by index
 * zero and the length identifies a range from the MSB to the LSB.
 *
 *                   Normal                      Inverted
 *             MSB            LSB           MSB            LSB
 *              v              v             v              v
 *             +----------------+           +----------------+
 *             | uint16_t       |           | uint16_t       |
 *             +----------------+           +----------------+
 *              ^              ^             ^              ^
 *     Index   15              0             0              15
 *     Length          |<------+             +----->|
 *
 * Inside arrays the inverted form is used. Therefore an index of zero identifies the
 * MSB of the first word in the array.
 *
 * TODO: TBC
 */
#ifndef DATAUTILITIES_HPP_
#define DATAUTILITIES_HPP_

#include "../internal/Utilities.hpp"

#include <boost/cstdint.hpp>

#include <memory>
#include <string>

template<typename X> finline
static X shr( const X& value, unsigned int bits ) {
	return (sizeof(X)*8 == bits) ? 0 : (value >> bits);
}

template<typename X> finline
static X shl( const X& value, unsigned int bits ) {
	return (sizeof(X)*8 == bits) ? 0 : (value << bits);
}

/**
 * TODO: Doc
 */
template<typename X> finline
static X bitblk(size_t index, size_t length, bool value, if_uint(X)) {
	if (index == 0 && length >= sizeof(X) * 8) return value ? ~0 : 0;
	X x = shl(( shl( ((X) 1), length) - 1), index);
	return value ? x : ~x;
}

/**
 * TODO: Doc
 */
template<typename X, bool value> finline
static X bitblk(size_t index, size_t length, if_uint(X)) {
	return bitblk<X> (index, length, value);
}

/**
 * TODO: Doc
 */
template<typename X, bool value> finline
static X bitblk(size_t length, if_uint(X)) {
	if (length >= sizeof(X) * 8) return value ? ~0 : 0;
	return value ? (((X) 1 << length) - 1) : ~(((X) 1 << length) - 1);
}

/**
 * This method copies bits from one integral to another
 */
template<typename X, typename Y> finline
static Y bitcpy(X src, size_t srcIdx, Y dst, size_t dstIdx, size_t len =
		sizeof(X) * 8, bool srcInvert = false, bool dstInvert = false,
		if_uint(X),if_uint (Y) ) {
	if (srcInvert) srcIdx = sizeof(X) * 8 - srcIdx - len;
	if (dstInvert) dstIdx = sizeof(Y) * 8 - dstIdx - len;
	Y value = ((Y)shr(src, srcIdx)) << dstIdx;
	Y mask = bitblk<Y, 0> (dstIdx, len);
	return (dst & mask) | (value & ~mask);
}

/**
 * This method copies bits from an array to another of the same
 * integral type.
 */
template<typename X> finline
static void bitcpy(X* src, size_t srcIdx, X* dst, size_t dstIdx, size_t len) {

	// word width
	const size_t w = sizeof(X) * 8;

	// calculate indexes
	size_t dwp = dstIdx % w, swp = srcIdx % w;
	size_t idwp = w - dwp, iswp = w - swp;
	dst += dstIdx / w;
	src += srcIdx / w;

	// check direct copy
	if (dwp == 0 && swp == 0 && len % w == 0) {
		memcpy(dst, src, len / 8);
		return;
	}

	// check if first word only is affected
	if ((dwp + len) <= w) {
		X fw = shl(src[0],swp) | shr(src[1],iswp);
		*dst = bitcpy(fw, 0, *dst, dwp, len, true, true);
		return;
	}

	// set first word
	if (idwp != 0) {
		X fw = shl(src[0],swp) | shr(src[1],iswp);
		*dst = (*dst & ~(((X) 1 << idwp) - 1)) | shr(fw,dwp);

		// recalculate indexes & lengths
		dst++;
		src++;
		len -= idwp;
		swp = (swp + idwp) % w;
		iswp = w - swp;
	}

	X a, b;

	// copy whole words
	if (swp == 0) {
		size_t numWords = len / w;
		// use memory copy
		memcpy(dst, src, numWords * sizeof(X));
		src += numWords;
		dst += numWords;
		len = len % w;
		a = src[0], b = src[1];
	} else {
		// use shifted copy
		a = src[0], b = src[1];
		while (len >= w) {
			*dst = shl(a,swp) | shr(b,iswp);
			dst++;
			src++;
			len -= w;
			a = b;
			b = *src;
		}
	}

	// set last word
	X lw = shl(a,swp) | shr(b,iswp), lm = (shl((X) 1,(w - len)) - 1);
	*dst = (*dst & lm) | (lw & ~lm);
}



/**
 * array -> integral
 */
template<typename X, typename Y> finline
static void bitcpy(X* src, size_t srcIdx, Y& dst, size_t dstIdx, size_t len =
		sizeof(Y) * 8, if_uint(Y),if_uint (X) ) {

	// word width
		const size_t w = sizeof(X) * 8;

		// calculate indexes
		size_t swp = srcIdx % w, iswp = w - swp;
		src += srcIdx / w;

		// mask off bits
		dst &= bitblk<Y,0>(dstIdx,len);

		// copy whole words
		X a = src[0], b = src[1];
		Y value = 0;
		src++;
		while (len >= w) {
			X x = shl(a,swp) | shr(b,iswp);
			value <<= w;
			value |= x;
			src++;
			len -= w;
			a = b; b = *src;
		}

		// copy leftover
		if ( len> 0 ) {
			value <<= len;
			value |= ((shl(a,swp) | shr(b,iswp)) >> (w - len)) & (shl(1,len)-1);
		}

		// set value
		dst |= (value << dstIdx);
	}

/**
 * TODO: Doc
 */
// word -> array
template<typename X> finline
static void bitcpy(X src, size_t srcIdx, X* dst, size_t dstIdx, size_t len =
		sizeof(X) * 8, bool srcInvert = false, if_uint(X)) {

	// check inversion
	if (srcInvert) srcIdx = sizeof(X) * 8 - srcIdx - len;

	// word width
	const size_t w = sizeof(X) * 8;

	// calculate indexes
	size_t dwp = dstIdx % w;
	dst += dstIdx / w;

	// copy directly
	if (dwp == 0 && srcIdx == 0 && len == w) {
		*dst = src;
	} else

	// copy non-overlapping word
	if ((dwp + len) <= w) {
		*dst = bitcpy(src, srcIdx, *dst, dwp, len, false, true);

		// copy overlapping word
	} else {
		size_t idwp = w - dwp;
		src >>= srcIdx;
		X mask1 = ~(shl(1,idwp) - 1);
		dst[0] = (dst[0] & mask1) | (shr(src,(len - idwp)) & ~mask1);
		X mask2 = shl(1,(w - len + idwp)) - 1;
		dst[1] = (dst[1] & mask2) | (shl(src, (w - len + idwp)) & ~mask2);
	}
}

/**
 * TODO: Doc
 */
// integral -> array
template<typename Y, typename X> finline
static void bitcpy(Y src, size_t srcIdx, X* dst, size_t dstIdx, size_t len =
		sizeof(Y) * 8, bool srcInvert = false, if_uint(X),if_uint (Y) ) {

	if (sizeof(Y) <= sizeof(X)) {
		// check inversion
		if (srcInvert)
		srcIdx = sizeof(Y) * 8 - srcIdx - len + (sizeof(X)-sizeof(Y))*8;
		bitcpy((X)src,srcIdx,dst,dstIdx,len,false);
	} else {
		// check inversion
		if (srcInvert) srcIdx = sizeof(Y) * 8 - srcIdx - len;
		src = shr(src, srcIdx);

		const size_t dw = sizeof(X)*8;
		while (len >= dw) {
			X word = (X)shr(src,(len-dw));
			bitcpy(word,0,dst,dstIdx,dw);
			dstIdx += dw;
			len -= dw;
		}
		X word = (X)src;
		bitcpy(word,0,dst,dstIdx,len);
	}
}

/**
 * TODO: Doc
 */
template<typename T, typename X> finline
static T bitget(X* src, size_t idx, size_t len = sizeof(T) * 8, if_uint(X),if_uint (T) ) {
	T x = 0;
	bitcpy(src, idx, x, 0, len );
	return x;
}

/**
 * TODO: Doc
 */
template<typename T, typename X> finline
static T bitget(X src, size_t idx, size_t len = sizeof(T) * 8, if_uint(X),if_uint (T) ) {
	T x = 0;
	bitcpy(src, idx, x, 0, len );
	return x;
}

/**
 * TODO: Doc
 */
template<typename X> finline
static bool bitget(X* src, size_t index, if_uint(X)) {
	uint8_t x = 0;
	bitcpy(src, index, x, 0, 1);
	return x != 0;
}

/**
 * TODO: Doc
 */
template<typename X> finline
static bool bitget(X src, size_t index, if_uint(X)) {
	uint8_t x = 0;
	x = bitcpy(src, index, x, 0, 1);
	return x != 0;
}

/**
 * TODO: Doc
 */
template<typename T, typename X> finline
static void bitset(T src, X* dst, size_t index, if_uint(X),if_uint (T) ) {
	bitcpy(src,0,dst,index);
}

/**
 * TODO: Doc
 */
template<typename X> finline
static void bitset(bool src, X* dst, size_t index, if_uint(X) ) {
	bitcpy(src != 0, 0, dst, index, 1);
}

/**
 * TODO: Doc
 */
template<typename X> finline
static X bitset(bool src, X dst, size_t index, if_uint(X)) {
	return bitcpy(src != 0, 0, dst, index, 1);
}

/**
 * TODO: Doc
 */
template<typename X> finline
static X bitrev(X src, if_uint(X) ) {
	const size_t width = sizeof(X) * 8;
	X dst = 0;
	for (size_t i = 0; i < width; i++)
		dst = bitset(bitget(src, i), dst, width - i - 1);
	return dst;
}

/**
 * TODO: Doc
 */
template<typename X> finline
static void bitrev(X* src, size_t len, if_uint(X) ) {
	for (size_t i = 0; i < len / 2; i++) {
		bool b0 = bitget(src, i);
		bool b1 = bitget(src, len - i - 1);
		bitset(b1, src, i);
		bitset(b0, src, len - i - 1);
	}
}

/**
 * TODO: Doc
 */
template<typename X> finline
static X switch_endian(X src, if_uint(X) ) {
	if (sizeof(X) == 1) return src;
	X ret = 0;
	for (size_t i = 0; i < sizeof(X); i++) {
		ret <<= 8;
		ret |= src & 0xFF;
		src >>= 8;
	}
	return ret;
}

/**
 * TODO: Doc
 */
template<typename X> finline
static std::string bitstr(X src, int log2base = 4, if_uint(X)) {
	const size_t word_width = sizeof(src) * 8;
	const char digit[37] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	std::string str;
	for (int i = word_width - log2base, j = 0; i >= 0; i -= log2base, j++)
		str.append(1, digit[(src >> i) & ((1 << log2base) - 1)]);
	return str;
}

/**
 * TODO: Doc
 */
template<typename X> finline
static std::string bitstr(X* src, size_t len, int log2base = 4, bool dot =
		false, if_uint(X)) {
	std::string str;
	for (size_t i = 0; i < len / 8 / sizeof(X); i++) {
		if (i != 0 && dot) str.append(".");
		str.append(bitstr(src[i], log2base));
	}
	return str;
}

#endif /* DATAUTILITIES_HPP_ */
