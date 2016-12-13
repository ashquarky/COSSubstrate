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

int Menu_Main() {
	InitOSFunctionPointers();
	InitSocketFunctionPointers();
	InstallExceptionHandler();

	log_init("192.168.192.36");
	log_print("Hello World!\n");

	InstallKernFunctions();

	unsigned int* nom = malloc(0x10);
	log_printf("nom: 0x%08X\n", nom);
	nom[0] = 0x69696969;
	log_printf("*nom: 0x%08X\n", *nom);
	unsigned int nom2 = kern_read(nom);
	log_printf("post-kern *nom: 0x%08X\n", nom2);

	sleep(2);

	log_deinit();

	return 0;
}
