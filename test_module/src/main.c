
#include <substrate/substrate.h>
#include <UDynLoad.h>

int (*OSDynLoad_Acquire)(const char* rpl, unsigned int* handle);
int (*OSDynLoad_FindExport)(unsigned int handle, int isdata, const char* symbol, void* address);

void (*OSScreenPutFontEx)(int buffer, int x, int y, char* msg);
void (*DCFlushRange)(void* addr, unsigned int size);

void (*COSSubstrate_PatchFunc)(void* func, void(*callback)(COSSubstrate_FunctionContext*));

void callback(COSSubstrate_FunctionContext* ctx) {
	ctx->args[3] = (unsigned int)("Hello From the Substrate!");
}

void _start() {
	OSDynLoad_Acquire = COSS_SPECIFICS->OSDynLoad_Acquire;
	OSDynLoad_FindExport = COSS_SPECIFICS->OSDynLoad_FindExport;

	unsigned int coreinit_handle;
	OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle);
	OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenPutFontEx", &OSScreenPutFontEx);

	UDynLoad_FindExportDynamic(COSS_SPECIFICS->substrate, COSS_SPECIFICS->substrateDynamic, "COSSubstrate_PatchFunc", (void**)&COSSubstrate_PatchFunc);

	COSSubstrate_PatchFunc(OSScreenPutFontEx, &callback);
}
