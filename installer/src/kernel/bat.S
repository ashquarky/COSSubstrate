/*	Cafe OS Substrate
	This is the Installer.
	Licensed under MIT, visit https://github.com/QuarkTheAwesome/COSSubstrate for more details.
*/

#include "ppc_asm.h"

/*	Kernelmode code to set Data Block Address Translation (DBAT) registers.
	This essentially maps memory for us.
	|---------------------------------|
	|       |  Physical  | Effective  |
	| Start | 0x33800000 | 0x60000000 |
	| End   | 0x34000000 | 0x60800000 |
	|---------------------------------|

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
