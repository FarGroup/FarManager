#include <Rtl.Base.h>

PROC RtlHookImportTable(
		const char *lpModuleName,
		const char *lpFunctionName,
		PROC pfnNew,
		HMODULE hModule
		)
{
	PROC pfnResult = NULL;

	dword dwBase = (dword)hModule;
	dword dwOP;

	PIMAGE_DOS_HEADER pDosHeader;

	pDosHeader = (PIMAGE_DOS_HEADER)dwBase;

	if ( pDosHeader->e_magic != 0x00005A4D )
		return NULL;

	PIMAGE_NT_HEADERS pPEHeader  = (PIMAGE_NT_HEADERS)(dwBase+pDosHeader->e_lfanew);

	if ( pPEHeader->Signature != 0x00004550 )
		return NULL;

	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)(dwBase+pPEHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	const char *lpImportTableFunctionName;

	if ( !pPEHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress )
		return NULL;

	const char *lpImportTableModuleName;

	while ( pImportDesc->Name )
	{
		lpImportTableModuleName = (const char*)(dwBase+(dword)pImportDesc->Name);

		if ( !lstrcmpiA (lpImportTableModuleName, lpModuleName) )
			break;

		pImportDesc++;
	}

	if ( !pImportDesc->Name )
		return NULL;

	PIMAGE_THUNK_DATA pFirstThunk;
	PIMAGE_THUNK_DATA pOriginalThunk;

	pFirstThunk = (PIMAGE_THUNK_DATA)(dwBase+(dword)pImportDesc->FirstThunk);
	pOriginalThunk = (PIMAGE_THUNK_DATA)(dwBase+(dword)pImportDesc->OriginalFirstThunk);

	while ( pFirstThunk->u1.Function )
	{
		lpImportTableFunctionName = (const char*)(dwBase+(dword)((PIMAGE_IMPORT_BY_NAME)pOriginalThunk->u1.AddressOfData)->Name);

		dword dwOldProtect;
		PROC* ppfnOld;

		if ( !lstrcmpiA (lpImportTableFunctionName, lpFunctionName) )
		{
			pfnResult = (PROC)pFirstThunk->u1.Function;
			ppfnOld = (PROC*)&pFirstThunk->u1.Function;

			VirtualProtect (ppfnOld, 4, PAGE_READWRITE, &dwOldProtect);
			WriteProcessMemory(GetCurrentProcess(), ppfnOld, &pfnNew, sizeof pfnNew, NULL);
			VirtualProtect (ppfnOld, 4, dwOldProtect, &dwOldProtect);

			return pfnResult;
		}

        pFirstThunk++;
		pOriginalThunk++;
	}

	return NULL; //error
}
