#include <cstdio>
#include <windows.h>

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
		return false;

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
		return false;

	*To = From;

	return VirtualProtect(Info.BaseAddress, Info.RegionSize, Info.Protect, &Protection);
}

static bool patch(HMODULE Module)
{
	const auto Headers = reinterpret_cast<PIMAGE_NT_HEADERS>(PBYTE(Module) + PIMAGE_DOS_HEADER(Module)->e_lfanew);
	if (Headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size <= 0)
		return false;

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

			if (Function && write_memory(reinterpret_cast<void const**>(FirstBound + i), Function))
				AnyPatched = true;
		}
	}

	return AnyPatched;
}

static int WINAPI unknown(int)
{
	return 1;
}

int main(int const argc, const char* const argv[])
{
	if (argc != 3)
		return 1;

	CodePage = atoi(argv[1]);

	const auto HhaModule = LoadLibraryA("hha.dll");
	if (!HhaModule)
		return 1;

	using CompileHHP_t = int WINAPI(const char*, int(*)(const char*, ...), int(WINAPI*)(int), DWORD);
	const auto CompileHHP = reinterpret_cast<CompileHHP_t*>(GetProcAddress(HhaModule, reinterpret_cast<LPCSTR>(319)));
	if (!CompileHHP)
		return 1;

	if (!patch(HhaModule))
		return 1;

	if (!SUCCEEDED(CoInitialize({})))
		return 1;

	const auto Result = CompileHHP(argv[2], printf, unknown, 0);

	CoUninitialize();

	FreeLibrary(HhaModule);

	return Result? EXIT_SUCCESS : EXIT_FAILURE;
}
