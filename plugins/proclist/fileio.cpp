#include "Proclist.hpp"

size_t WriteToFile(HANDLE File, const std::wstring_view& Str)
{
	DWORD Written;
	if (!WriteFile(File, Str.data(), static_cast<DWORD>(Str.size() * sizeof(wchar_t)), &Written, {}))
		return 0;

	return Written / sizeof(wchar_t);
}

size_t WriteToFile(HANDLE File, wchar_t Char)
{
	// BUGBUG this is mental
	return WriteToFile(File, std::wstring_view{ &Char, 1 });
}
