/*	Cafe OS Substrate Installer

	bat.S - Handles PowerPC memory mapping. See below for details.
	Paired with kernel.h.

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

#include "ppc_asm.h"

/*	Kernelmode code to set Data Block Address Translation (DBAT) registers.
	This essentially maps memory for us.
	|---------------------------------|
	|       |  Physical  | Effective  |
	| Start | 0x33800000 | 0x60000000 |
	| End   | 0x34000000 | 0x60800000 |
	|---------------------------------|

	ANY CHANGES MUST BE REFLECTED IN THE SUBSTRATE API HEADER

	_kernelmode_ void SetupBATs();
*/
.globl SetupBATs
SetupBATs:
	lis r3, 0x600000FF@h
	ori r3, r3, 0x600000FF@l
	mtspr 572, r3 //dbat6u
	mtspr 564, r3 //ibat6u

	lis r3, 0x33800012@h
	ori r3, r3, 0x33800012@l
	mtspr 573, r3 //dbat6l
	mtspr 565, r3 //ibat6l

	rfi

/*	Clears our mappings, returning the console to the way it was before.
	_kernelmode_ void ClearBATs();
*/
.globl ClearBATs
ClearBATs:
	li r3, 0
	mtspr 572, r3 //dbat6u
	mtspr 564, r3 //ibat6u
	mtspr 573, r3 //dbat6l
	mtspr 565, r3 //ibat6l

	rfi
