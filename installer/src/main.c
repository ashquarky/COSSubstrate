/*	Cafe OS Substrate
	This is the Installer.
	Licensed under MIT, visit https://github.com/QuarkTheAwesome/COSSubstrate for more details.
*/

#include <stdlib.h>
#include <stdio.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "fs/sd_fat_devoptab.h"
#include "utils/logger.h"
#include "utils/exception.h"

#include "kernel/kernel.h"

/*	At this point, we just test various fundamentals in order to set everything up.
	Not much actual installing is going on here.
*/
int Menu_Main() {
	InitOSFunctionPointers();
	InitSocketFunctionPointers();
	InitFSFunctionPointers();
	InstallExceptionHandler();

	mount_sd_fat("sd");

	log_init("192.168.192.36");
	log_print("Hello World!\n");

	//Do code things here

	log_print("Quitting...\n");
	log_print("------------------------------------\n\n\n");

	unmount_sd_fat("sd");
	log_deinit();

	return 0;
}
