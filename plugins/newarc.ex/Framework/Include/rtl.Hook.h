#pragma once
#include "Rtl.Base.h"

extern PROC RtlHookImportTable(
		const char *lpModuleName,
		const char *lpFunctionName,
		PROC pfnNew,
		HMODULE hModule
		);
