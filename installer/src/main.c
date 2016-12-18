/*	Cafe OS Substrate
	This is the Installer.
	Licensed under MIT, visit https://github.com/QuarkTheAwesome/COSSubstrate for more details.
*/

#include <stdlib.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "utils/logger.h"
#include "utils/exception.h"

#include "kernel/kernel.h"

/*	At this point, we just test various fundamentals in order to set everything up.
	Not much actual installing is going on here.
*/
int Menu_Main() {
	InitOSFunctionPointers();
	InitSocketFunctionPointers();
	InstallExceptionHandler();

	log_init("192.168.192.36");
	log_print("Hello World!\n");

	InstallKernFunctions();

	unsigned int* nom = malloc(0x10);
	log_printf("nom: 0x%08X\n", nom);
	nom[0] = 0x00000000;
	log_printf("*nom: 0x%08X\n", *nom);
	kern_write(nom, 0x69696969);
	log_printf("post-kern *nom: 0x%08X\n", *nom);
	unsigned int nom2 = kern_read(nom);
	log_printf("post-kern *nom: 0x%08X\n", nom2);

	nom2 = RunCodeAsKernel(&kernelCodeTest, (unsigned int)nom);
	log_printf("Custom kernel code *nom: 0x%08X\n", nom2);


	unsigned int ret = RunCodeAsKernel(&SetupBATs, 0);
	log_printf("BATs set up! 0x%08X\n", ret);

	log_printf("SPR572 = 0x%08X\n", RunCodeAsKernel(&ReadSPR572, 0));
	log_printf("SPR573 = 0x%08X\n", RunCodeAsKernel(&ReadSPR573, 0));

	sleep(2);

	unsigned int* bat = (unsigned int*)0x60000010;
	InstallAltExceptionHandler();
	unsigned int res = *bat;
	InstallExceptionHandler();
	log_printf("0x%08X, 0x%08X\n", bat, res);
	log_printf("SPR572 = 0x%08X\n", RunCodeAsKernel(&ReadSPR572, 0));
	log_printf("SPR573 = 0x%08X\n", RunCodeAsKernel(&ReadSPR573, 0));

	RunCodeAsKernel(&ClearBATs, 0);

	sleep(2);

	free(nom);
	log_print("Quitting...\n");
	log_print("------------------------------------\n\n\n");
	log_deinit();

	return 0;
}
