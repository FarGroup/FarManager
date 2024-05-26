/*
fileowner.cpp

Кэш SID`ов и функция GetOwner
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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
#include "fileowner.hpp"

// Internal:
#include "pathmix.hpp"
#include "elevation.hpp"

// Platform:
#include "platform.memory.hpp"
#include "platform.fs.hpp"
#include "platform.security.hpp"

// Common:
#include "common.hpp"
#include "common/function_ref.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

static bool SidToName(PSID Sid, string& Name, const string& Computer)
{
	auto AccountName = os::buffer<wchar_t>();
	auto DomainName = os::buffer<wchar_t>();
	auto AccountLength = static_cast<DWORD>(AccountName.size());
	auto DomainLength = static_cast<DWORD>(DomainName.size());
	SID_NAME_USE snu;

	while (!LookupAccountSid(EmptyToNull(Computer), Sid, AccountName.data(), &AccountLength, DomainName.data(), &DomainLength, &snu))
	{
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			AccountName.reset(AccountLength);
			DomainName.reset(DomainLength);
		}
		else
		{
			os::memory::local::ptr<wchar_t> StrSid;
			if (!ConvertSidToStringSid(Sid, &ptr_setter(StrSid)))
				return false;

			Name = StrSid.get();
			return true;
		}
	}

	Name.clear();

	if (DomainLength)
		append(Name, string_view(DomainName.data(), DomainLength), L'\\', string_view(AccountName.data(), AccountLength));
	else
		Name.assign(AccountName.data(), AccountLength);

	return true;
}

namespace
{
	class sid
	{
	public:
		NONCOPYABLE(sid);
		MOVE_CONSTRUCTIBLE(sid);

		sid() noexcept = default;

		explicit(false) sid(std::nullptr_t) noexcept
		{
		}

		explicit sid(size_t Size)
		{
			reset(Size);
		}

		explicit sid(PSID rhs)
		{
			const auto Size = GetLengthSid(rhs);
			reset(Size);
			CopySid(Size, get(), rhs);
		}

		bool operator==(const sid& rhs) const
		{
			return *this == rhs.get();
		}

		bool operator==(const PSID rhs) const
		{
			return EqualSid(get(), rhs) != FALSE;
		}

		explicit operator bool() const
		{
			return m_Data.operator bool();
		}

		PSID get() const
		{
			return m_Data.data();
		}

		void reset(size_t Size)
		{
			m_Data.reset(Size);
		}

		auto size() const
		{
			return m_Data.size();
		}

		size_t get_hash() const
		{
			return get_hash(get(), size());
		}

		static size_t get_hash(const PSID Data, size_t Size)
		{
			const auto Begin = static_cast<const std::byte*>(Data);
			return hash_range(std::span(Begin, Size));
		}

	private:
		block_ptr<SID, os::default_buffer_size> m_Data;
	};
}

static bool SidToNameCached(PSID Sid, string& Name, const string& Computer)
{
	struct sid_hash_eq
	{
		using is_transparent = void;

		size_t operator()(const sid& Sid) const { return Sid.get_hash(); }
		size_t operator()(const PSID Sid) const { return sid::get_hash(Sid, GetLengthSid(Sid)); }

		bool operator()(const sid& Sid1, const sid& Sid2) const { return Sid1 == Sid2; }
		bool operator()(const sid& Sid1, const PSID Sid2) const { return Sid1 == Sid2; }
		bool operator()(const PSID Sid1, const sid& Sid2) const { return Sid2 == Sid1; }
	};

	static std::unordered_map<sid, string, sid_hash_eq, sid_hash_eq> SIDCache;

	if (const auto ItemIterator = SIDCache.find(Sid); ItemIterator != SIDCache.cend())
	{
		Name = ItemIterator->second;
		return true;
	}

	if (SidToName(Sid, Name, Computer))
	{
		SIDCache.emplace(Sid, Name);
		return true;
	}

	return false;
}

static bool ProcessFileOwner(string_view const Name, function_ref<bool(PSID)> const Callable)
{
	const auto SecurityDescriptor = os::fs::get_file_security(Name, OWNER_SECURITY_INFORMATION);
	if (!SecurityDescriptor)
		return false;

	PSID pOwner;
	BOOL OwnerDefaulted;
	if (!GetSecurityDescriptorOwner(SecurityDescriptor.data(), &pOwner, &OwnerDefaulted))
		return false;

	if (!IsValidSid(pOwner))
		return false;

	return Callable(pOwner);
}

static bool IsOwned(string_view const Name, PSID const Owner)
{
	return ProcessFileOwner(Name, [&](PSID const Sid)
	{
		return EqualSid(Sid, Owner);
	});
}


// TODO: elevation
bool GetFileOwner(const string& Computer, string_view const Object, string& Owner)
{
	return ProcessFileOwner(Object, [&](PSID const Sid)
	{
		return SidToNameCached(Sid, Owner, Computer);
	});
}

static auto get_sid(const string& Name)
{
	os::memory::local::ptr<void> SidFromString;
	if (ConvertStringSidToSid(Name.c_str(), &ptr_setter(SidFromString)))
	{
		return sid{ SidFromString.get() };
	}

	sid Sid(os::default_buffer_size);
	auto ReferencedDomainName = os::buffer<wchar_t>();
	auto SidSize = static_cast<DWORD>(Sid.size());
	auto ReferencedDomainNameSize = static_cast<DWORD>(ReferencedDomainName.size());
	SID_NAME_USE Use;
	while (!LookupAccountName(nullptr, Name.c_str(), Sid.get(), &SidSize, ReferencedDomainName.data(), &ReferencedDomainNameSize, &Use))
	{
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			Sid.reset(SidSize);
			ReferencedDomainName.reset(ReferencedDomainNameSize);
		}
		else
		{
			return sid{};
		}
	}

	return Sid;
}

bool SetOwnerInternal(const string& Object, const string& Owner)
{
	const auto Sid = get_sid(Owner);
	if (!Sid)
		return false;

	if (IsOwned(Object, Sid.get()))
		return true;

	SCOPED_ACTION(os::security::privilege){ SE_TAKE_OWNERSHIP_NAME, SE_RESTORE_NAME };

	const auto Result = SetNamedSecurityInfo(const_cast<wchar_t*>(Object.c_str()), SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, Sid.get(), nullptr, nullptr, nullptr);
	SetLastError(Result);
	return Result == ERROR_SUCCESS;
}

bool SetFileOwner(string_view const Object, const string& Owner)
{
	const auto NtObject = nt_path(Object);

	if (SetOwnerInternal(NtObject, Owner))
		return true;

	if(ElevationRequired(ELEVATION_MODIFY_REQUEST))
		return elevation::instance().fSetOwner(NtObject, Owner);

	return false;
}
