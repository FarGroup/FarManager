﻿/*
platform.version.cpp

*/
/*
Copyright © 2020 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "platform.version.hpp"

// Internal:
#include "imports.hpp"

// Platform:
#include "platform.reg.hpp"

// Common:
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

namespace os::version
{
	template<class T>
	static auto get_value(std::vector<std::byte> const& Data, const string_view SubBlock)
	{
		unsigned Length;
		T* Result;
		return VerQueryValue(Data.data(), null_terminated(SubBlock).c_str(), reinterpret_cast<void**>(&Result), &Length) && Length? Result : nullptr;
	}

	bool file_version::read(string_view const Filename)
	{
		null_terminated const c_Name(Filename);

		const auto Size = GetFileVersionInfoSize(c_Name.c_str(), nullptr);
		if (!Size)
			return false;

		m_Buffer.resize(Size);

		if (!GetFileVersionInfo(c_Name.c_str(), 0, Size, m_Buffer.data()))
			return false;

		const auto Translation = get_value<DWORD>(m_Buffer, L"\\VarFileInfo\\Translation"sv);
		if (!Translation)
			return false;

		m_BlockPath = far::format(
			L"\\StringFileInfo\\{:04X}{:04X}\\"sv,
			extract_integer<WORD, 0>(*Translation),
			extract_integer<WORD, 1>(*Translation)
		);

		return true;
	}

	wchar_t const* file_version::get_string(string_view const Value) const
	{
		return get_value<wchar_t>(m_Buffer, m_BlockPath + Value);
	}

	VS_FIXEDFILEINFO const* file_version::get_fixed_info() const
	{
		return get_value<VS_FIXEDFILEINFO>(m_Buffer, L"\\"sv);
	}


	template<DWORD... Components>
	static unsigned long long condition_mask(DWORD const Operation)
	{
		return (... | VerSetConditionMask(0, Components, Operation));
	}

	bool is_win10_build_or_later(DWORD const Build)
	{
		OSVERSIONINFOEXW osvi
		{
			sizeof(osvi),
			extract_integer<BYTE, 1>(_WIN32_WINNT_WIN10),
			extract_integer<BYTE, 0>(_WIN32_WINNT_WIN10),
			Build
		};

		const auto ConditionMask = condition_mask<
			VER_MAJORVERSION,
			VER_MINORVERSION,
			VER_BUILDNUMBER
		>(VER_GREATER_EQUAL);

		return VerifyVersionInfo(
			&osvi,
			VER_MAJORVERSION |
			VER_MINORVERSION |
			VER_BUILDNUMBER,
			ConditionMask) != FALSE;
	}

	bool is_win10_1607_or_later()
	{
		static const auto Result = is_win10_build_or_later(14393);
		return Result;
	}

	bool is_win10_1703_or_later()
	{
		static const auto Result = is_win10_build_or_later(15063);
		return Result;
	}

	string get_file_version(string_view const Name)
	{
		file_version Version;
		if (!Version.read(Name))
			return last_error().Win32ErrorStr();

		if (const auto Str = Version.get_string(L"FileVersion"sv))
			return Str;

		const auto FixedInfo = Version.get_fixed_info();
		if (!FixedInfo)
			return last_error().Win32ErrorStr();

		return far::format(L"{}.{}.{}.{}"sv,
			extract_integer<WORD, 1>(FixedInfo->dwFileVersionMS),
			extract_integer<WORD, 0>(FixedInfo->dwFileVersionMS),
			extract_integer<WORD, 1>(FixedInfo->dwFileVersionLS),
			extract_integer<WORD, 0>(FixedInfo->dwFileVersionLS)
		);
	}

	static bool get_os_version(OSVERSIONINFOEX& Info)
	{
		const auto InfoPtr = edit_as<OSVERSIONINFO*>(&Info);

		if (imports.RtlGetVersion && NT_SUCCESS(imports.RtlGetVersion(InfoPtr)))
			return true;

WARNING_PUSH()
WARNING_DISABLE_MSC(4996) // 'GetVersionExW': was declared deprecated. So helpful. :(
WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
		return GetVersionEx(InfoPtr) != FALSE;
WARNING_POP()
	}

	static auto windows_platform(int const PlatformId)
	{
		switch (PlatformId)
		{
		case VER_PLATFORM_WIN32_NT:
			return L"Windows NT"s;
		default:
			return far::format(L"Unknown ({})"sv, PlatformId);
		}
	}

	static auto windows_product_type(int const ProductType)
	{
		switch (ProductType)
		{
		case VER_NT_WORKSTATION:
			return L"Workstation"s;
		case VER_NT_DOMAIN_CONTROLLER:
			return L"Domain Controller"s;
		case VER_NT_SERVER:
			return L"Server"s;
		default:
			return far::format(L"Unknown ({})"sv, ProductType);
		}
	}

	static auto windows_service_pack(OSVERSIONINFOEX const& Info)
	{
		if (*Info.szCSDVersion)
			return far::format(L" {} ({}.{})"sv, Info.szCSDVersion, Info.wServicePackMajor, Info.wServicePackMinor);

		if (Info.wServicePackMajor)
			return far::format(L" {}.{}"sv, Info.wServicePackMajor, Info.wServicePackMinor);

		return L""s;
	}

	static string os_version_from_api()
	{
		OSVERSIONINFOEX Info{ sizeof(Info) };
		if (!get_os_version(Info))
			return last_error().Win32ErrorStr();

		DWORD ProductType;
		if (!imports.GetProductInfo || !imports.GetProductInfo(-1, -1, -1, -1, &ProductType))
			ProductType = 0;

		return far::format(L"{} {} {}.{}.{}{}, 0x{:X}/0x{:X}"sv,
			windows_platform(Info.dwPlatformId),
			windows_product_type(Info.wProductType),
			Info.dwMajorVersion,
			Info.dwMinorVersion,
			Info.dwBuildNumber,
			windows_service_pack(Info),
			Info.wSuiteMask,
			ProductType
		);
	}

	// Mental OS - mental methods *facepalm*
	static string os_version_from_registry()
	{
		static const auto NativeKeyFlag = []
		{
			return
#ifndef _WIN64
				IsWow64Process()? KEY_WOW64_64KEY :
#endif
				0;
		}();

		const auto Key = reg::key::local_machine.open(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"sv, KEY_QUERY_VALUE | NativeKeyFlag);
		if (!Key)
			return {};

		string ProductName, DisplayVersion, CurrentBuild;
		unsigned UBR;
		if (
			!Key.get(L"ProductName"sv, ProductName) ||
			(!Key.get(L"DisplayVersion"sv, DisplayVersion) && !Key.get(L"ReleaseId"sv, DisplayVersion)) ||
			!Key.get(L"CurrentBuild"sv, CurrentBuild) ||
			!Key.get(L"UBR"sv, UBR)
		)
			return {};

		return far::format(L" ({}, version {}, OS build {}.{})"sv, ProductName, DisplayVersion, CurrentBuild, UBR);
	}

	string os_version()
	{
		return os_version_from_api() + os_version_from_registry();
	}
}
