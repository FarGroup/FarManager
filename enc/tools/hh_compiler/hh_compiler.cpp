#include <cstdio>
#include <windows.h>
#include <shlwapi.h>

int CodePage;

namespace replacement
{
	static int WINAPI GetACP_hook()
	{
		return CodePage;
	}

	static int WINAPI MultiByteToWideChar_hook(UINT cp, DWORD flags, LPCCH lpMultiByte, int cbMultiByte, LPWSTR lpWide, int cchWide)
	{
		return MultiByteToWideChar(cp == CP_ACP? CodePage : cp, flags, lpMultiByte, cbMultiByte, lpWide, cchWide);
	}

	static int WINAPI WideCharToMultiByte_hook(UINT cp, DWORD flags, LPCWCH lpWide, int cchWide, LPSTR lpMultiByte, int cbMultiByte, LPCCH lpDef, LPBOOL lpUsedDef)
	{
		return WideCharToMultiByte(cp == CP_ACP? CodePage : cp, flags, lpWide, cchWide, lpMultiByte, cbMultiByte, lpDef, lpUsedDef);
	}
}

static void* FindByName(const char* Name)
{
	if (!lstrcmpA(Name, "GetACP"))
		return reinterpret_cast<void*>(replacement::GetACP_hook);

	if (!lstrcmpA(Name, "MultiByteToWideChar"))
		return reinterpret_cast<void*>(replacement::MultiByteToWideChar_hook);

	if (!lstrcmpA(Name, "WideCharToMultiByte"))
		return reinterpret_cast<void*>(replacement::WideCharToMultiByte_hook);

	return {};
}

template <class type>
auto FromRva(HMODULE const Module, DWORD const Rva)
{
	return reinterpret_cast<type>(PBYTE(Module) + Rva);
}

static bool write_memory(void const** const To, const void* const From)
{
	MEMORY_BASIC_INFORMATION Info;
	if (!VirtualQuery(To, &Info, sizeof(Info)))
	{
		fprintf(stderr, "[!] VirtualQuery failed for address %p. Error: %lu\n", To, GetLastError());
		return false;
	}

	DWORD Protection;

	switch (Info.Protect)
	{
	case PAGE_READWRITE:
	case PAGE_EXECUTE_READWRITE:
		*To = From;
		return true;

	case PAGE_READONLY:
		Protection = PAGE_READWRITE;
		break;

	default:
		Protection = PAGE_EXECUTE_READWRITE;
		break;
	}

	if (!VirtualProtect(Info.BaseAddress, Info.RegionSize, Protection, &Protection))
	{
		fprintf(stderr, "[!] VirtualProtect (Unlock) failed for address %p. Error: %lu\n", Info.BaseAddress, GetLastError());
		return false;
	}

	*To = From;

	if (!VirtualProtect(Info.BaseAddress, Info.RegionSize, Info.Protect, &Protection))
	{
		fprintf(stderr, "[!] VirtualProtect (Restore) failed for address %p. Error: %lu\n", Info.BaseAddress, GetLastError());
		return false;
	}

	return true;
}

static bool patch(HMODULE Module)
{
	const auto Headers = reinterpret_cast<PIMAGE_NT_HEADERS>(PBYTE(Module) + PIMAGE_DOS_HEADER(Module)->e_lfanew);
	if (Headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size <= 0)
	{
		fprintf(stderr, "[!] Import directory not found or empty in module %p.\n", Module);
		return false;
	}

	bool AnyPatched{};

	for (auto ImportIterator = FromRva<PIMAGE_IMPORT_DESCRIPTOR>(Module, Headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress); ImportIterator->OriginalFirstThunk; ++ImportIterator)
	{
		const auto DllName = FromRva<LPCSTR>(Module, ImportIterator->Name);
		if (lstrcmpiA(DllName, "kernel32.dll"))
			continue;

		const auto FirstUnbound = FromRva<IMAGE_THUNK_DATA const*>(Module, ImportIterator->OriginalFirstThunk);
		const auto FirstBound = FromRva<IMAGE_THUNK_DATA*>(Module, ImportIterator->FirstThunk);

		for (size_t i = 0; FirstBound[i].u1.Function; ++i)
		{
			if (IMAGE_SNAP_BY_ORDINAL(FirstUnbound[i].u1.Ordinal))
				continue;

			const auto ImageImportByName = FromRva<IMAGE_IMPORT_BY_NAME const*>(Module, DWORD(UINT_PTR(FirstUnbound[i].u1.AddressOfData)));
			const auto FunctionName = reinterpret_cast<const char*>(ImageImportByName->Name);
			const auto Function = FindByName(FunctionName);

			if (Function)
			{
				fprintf(stderr, "[*] Patching function: %s\n", FunctionName);
				if (write_memory(reinterpret_cast<void const**>(FirstBound + i), Function))
				{
					AnyPatched = true;
				}
				else
				{
					fprintf(stderr, "[!] Failed to write memory for hook: %s\n", FunctionName);
				}
			}
		}
	}

	if (!AnyPatched)
	{
		fprintf(stderr, "[!] Warning: No functions were patched. 'kernel32.dll' imports not found or target functions missing.\n");
	}

	return AnyPatched;
}

static int WINAPI unknown(int)
{
	return 1;
}

// Custom logging function for intercepting error messages
static int LogCallback(const char* format, ...)
{
	va_list args;
	
	char buffer[2048];
	va_start(args, format);
	int len = vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	printf("%s", buffer); // original message

	// "HHC6003: Error: The file Itircl.dll has not been registered correctly."
	if (strstr(buffer, "HHC6003") && strstr(buffer, "Itircl.dll"))
	{
		char path[MAX_PATH] = { 0 };
		HMODULE hHha = GetModuleHandleA("hha.dll");
		
		if (hHha && GetModuleFileNameA(hHha, path, MAX_PATH))
		{
			PathRemoveFileSpecA(path);
			PathAppendA(path, "itcc.dll");
		}
		else
		{
			strcpy_s(path, "path\\to\\itcc.dll");
		}

		fprintf(stderr, 
			"\n--------------------------------------------------------------------------------\n"
			"RECOMMENDATION:\n"
			"Make sure that `itcc.dll` exists and run the following command\n"
			"with Administrator privileges:\n\n"
			"regsvr32 \"%s\"\n",
			path);

		if (!PathFileExistsA(path))
		{
			fprintf(stderr,
				"\nERROR:\n"
				"The required file was NOT found at the expected location:\n"
				"%s\n",
				path);
		}

		fprintf(stderr,
			"--------------------------------------------------------------------------------\n");
	}

	return len;
}

int main(int const argc, const char* const argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <CodePage> <InputHHP>\n", argv[0]);
		return 1;
	}

	CodePage = atoi(argv[1]);
	fprintf(stderr, "[*] Target CodePage: %d\n", CodePage);

	fprintf(stderr, "[*] Loading hha.dll...\n");
	const auto HhaModule = LoadLibraryA("hha.dll");
	if (!HhaModule)
	{
		fprintf(stderr, "[!] LoadLibraryA(\"hha.dll\") failed. Error: %lu\n"
			"\n--------------------------------------------------------------------------------\n"
			"RECOMMENDATION:\n"
			"Microsoft HTML Help Workshop should be installed.\n"
			"--------------------------------------------------------------------------------\n",
			GetLastError());
		return 1;
	}

	using CompileHHP_t = int WINAPI(const char*, int(*)(const char*, ...), int(WINAPI*)(int), DWORD);
	const auto CompileHHP = reinterpret_cast<CompileHHP_t*>(GetProcAddress(HhaModule, reinterpret_cast<LPCSTR>(319)));
	if (!CompileHHP)
	{
		fprintf(stderr, "[!] GetProcAddress(319) failed. Error: %lu\n", GetLastError());
		FreeLibrary(HhaModule);
		return 1;
	}

	if (!patch(HhaModule))
	{
		fprintf(stderr, "[!] Patch failed. Aborting.\n");
		FreeLibrary(HhaModule);
		return 1;
	}

	HRESULT hr = CoInitialize({});
	if (FAILED(hr))
	{
		fprintf(stderr, "[!] CoInitialize failed. HRESULT: 0x%08lX\n", hr);
		FreeLibrary(HhaModule);
		return 1;
	}

	fprintf(stderr, "[*] Starting compilation of '%s'...\n", argv[2]);
	const auto Result = CompileHHP(argv[2], LogCallback, unknown, 0);

	if (!Result)
	{
		fprintf(stderr, "[!] CompileHHP returned failure (0).\n");
	}
	else
	{
		fprintf(stderr, "[*] CompileHHP finished successfully.\n");
	}

	CoUninitialize();
	FreeLibrary(HhaModule);

	return Result? EXIT_SUCCESS : EXIT_FAILURE;
}
