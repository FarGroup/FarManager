#include <memory>
#include <string>
#include <utility>

#include "Proclist.hpp"

static std::wstring str_vprintf(const wchar_t* format, va_list argptr)
{
	size_t Size = 128;

	for(;;)
	{
		const auto Buffer = std::make_unique<wchar_t[]>(Size *= 2);
		Buffer[Size - 1] = 0;
		const auto Length = _vsnwprintf(Buffer.get(), Size - 1, format, argptr);
		if (Length >= 0)
			return std::wstring( Buffer.get(), Length);
	}
}

size_t PrintToFile(HANDLE File, const wchar_t* Format...)
{
	va_list argptr;
	va_start(argptr, Format);

	const auto Str = str_vprintf(Format, argptr);

	DWORD Written;
	if (!WriteFile(File, Str.data(), static_cast<DWORD>(Str.size() * sizeof(wchar_t)), &Written, {}))
		return 0;

	va_end(argptr);

	return Written / sizeof(wchar_t);
}

size_t PrintToFile(HANDLE File, wchar_t Char)
{
	// BUGBUG this is mental

	DWORD Written;
	if (!WriteFile(File, &Char, sizeof(Char), &Written, {}))
		return 0;

	return Written / sizeof(wchar_t);
}
