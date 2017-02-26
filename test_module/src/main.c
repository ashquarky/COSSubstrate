#include <substrate/substrate.h>

void (*OSScreenPutFontEx)(int buffer, int x, int y, char* msg);

void callback(COSSubstrate_FunctionContext* ctx) {
	ctx->args[3] = (unsigned int)("Hello From the Substrate!");
}

void _start() {
	unsigned int coreinit_handle;
	COSS_SPECIFICS->OSDynLoad_Acquire("coreinit.rpl", &coreinit_handle);
	COSS_SPECIFICS->OSDynLoad_FindExport(coreinit_handle, 0, "OSScreenPutFontEx", &OSScreenPutFontEx);

	unsigned int substrate_handle;
	COSS_SPECIFICS->COSSDynLoad_Acquire("substrate.cosm", &substrate_handle);
	COSS_SPECIFICS->COSSDynLoad_FindExport(substrate_handle, "COSSubstrate_PatchFunc", &COSSubstrate_PatchFunc);

	COSSubstrate_PatchFunc(OSScreenPutFontEx, &callback);
}
