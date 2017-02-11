/*	Cafe OS Substrate

	misc.c - Random stuff.
	Paired with misc.h.

	https://github.com/QuarkTheAwesome/COSSubstrate

	Copyright (c) 2016 Ash (QuarkTheAwesome)
	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to
	deal in the Software without restriction, including without limitation the
	rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
	sell copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:
	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#include "kernel/kernel.h"

void memcpy(unsigned char* dst, unsigned char* src, unsigned int size) {
	int i = 0;
	for (i; i < size; i++) {
		dst[i] = src[i];
	}
}

void memset(unsigned int* dst, unsigned int fill, unsigned int size) {
	int i = 0;
	for (i; i < size; i+=4) {
		dst[i] = fill;
	}
}

unsigned int strlen(char* str) {
	unsigned int i;
	for (i = 0; (!(str[i] == 0)) || (i > 0x10000 /* Hard limit for safety */); i++) {}
	return i;
}
