/*
platform.security.cpp

*/
/*
Copyright © 2010 Far Group
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
#include "platform.security.hpp"

// Internal:
#include "elevation.hpp"
#include "log.hpp"

// Platform:
#include "platform.hpp"
#include "platform.concurrency.hpp"

// Common:
#include "common.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

namespace
{
	const auto& lookup_privilege_value(const wchar_t* Name)
	{
		static unordered_string_map<std::optional<LUID>> s_Cache;
		static os::critical_section s_CS;

		SCOPED_ACTION(std::scoped_lock)(s_CS);

		const auto [Iterator, IsEmplaced] = s_Cache.try_emplace(Name);

		auto& [MapKey, MapValue] = *Iterator;

		if (IsEmplaced)
		{
			if (LUID Luid; LookupPrivilegeValue(nullptr, MapKey.c_str(), &Luid))
				MapValue = Luid;
			else
				LOGWARNING(L"LookupPrivilegeValue({}): {}"sv, MapKey, os::last_error());
		}

		return MapValue;
	}

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

	string sid_to_name_impl(PSID Sid, const string& Computer)
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
				SCOPED_ACTION(os::last_error_guard);

				if (os::memory::local::ptr<wchar_t> StrSid; ConvertSidToStringSid(Sid, &ptr_setter(StrSid)))
					return StrSid.get();

				return {};
			}
		}

		return DomainLength?
			concat(string_view(DomainName.data(), DomainLength), L'\\', string_view(AccountName.data(), AccountLength)) :
			string{ AccountName.data(), AccountLength };
	}

	bool sid_to_name(PSID Sid, const string& Computer, string& Name)
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

		if (Name = sid_to_name_impl(Sid, Computer); !Name.empty())
		{
			SIDCache.emplace(Sid, Name);
			return true;
		}

		return false;
	}

	auto name_to_sid(const string& Name, const string& Computer)
	{
		sid Sid(os::default_buffer_size);
		auto ReferencedDomainName = os::buffer<wchar_t>();
		auto SidSize = static_cast<DWORD>(Sid.size());
		auto ReferencedDomainNameSize = static_cast<DWORD>(ReferencedDomainName.size());
		SID_NAME_USE Use;
		while (!LookupAccountName(EmptyToNull(Computer), Name.c_str(), Sid.get(), &SidSize, ReferencedDomainName.data(), &ReferencedDomainNameSize, &Use))
		{
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				Sid.reset(SidSize);
				ReferencedDomainName.reset(ReferencedDomainNameSize);
			}
			else
			{
				SCOPED_ACTION(os::last_error_guard);
				if (os::memory::local::ptr<void> SidFromString; ConvertStringSidToSid(Name.c_str(), &ptr_setter(SidFromString)))
					return sid{ SidFromString.get() };

				return sid{};
			}
		}

		return Sid;
	}

	PSID descriptor_get_owner(SECURITY_DESCRIPTOR* SecurityDescriptor)
	{
		PSID Owner;
		BOOL OwnerDefaulted;
		if (!GetSecurityDescriptorOwner(SecurityDescriptor, &Owner, &OwnerDefaulted))
			return {};

		if (!IsValidSid(Owner))
			return {};

		return Owner;
	}

	bool is_owned(string const& Object, SE_OBJECT_TYPE const ObjectType, PSID const Owner)
	{
		const auto SecurityDescriptor = os::security::get_security(Object, ObjectType, OWNER_SECURITY_INFORMATION);
		if (!SecurityDescriptor)
			return false;

		const auto OwnerSid = descriptor_get_owner(SecurityDescriptor.get());
		if (!OwnerSid)
			return false;

		return EqualSid(OwnerSid, Owner);
	}
}

static bool operator==(const LUID& a, const LUID& b)
{
	return a.LowPart == b.LowPart && a.HighPart == b.HighPart;
}

namespace os::security
{
	void detail::sid_deleter::operator()(PSID Sid) const noexcept
	{
		FreeSid(Sid);
	}

	sid_ptr make_sid(PSID_IDENTIFIER_AUTHORITY IdentifierAuthority, BYTE SubAuthorityCount, DWORD SubAuthority0, DWORD SubAuthority1, DWORD SubAuthority2, DWORD SubAuthority3, DWORD SubAuthority4, DWORD SubAuthority5, DWORD SubAuthority6, DWORD SubAuthority7)
	{
		PSID Sid;
		return sid_ptr(AllocateAndInitializeSid(IdentifierAuthority, SubAuthorityCount, SubAuthority0, SubAuthority1, SubAuthority2, SubAuthority3, SubAuthority4, SubAuthority5, SubAuthority6, SubAuthority7, &Sid)? Sid : nullptr);
	}

	sid_ptr make_admin_sid()
	{
		SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
		return make_sid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS);
	}

	bool is_admin()
	{
		static const auto Result = []
		{
			// Vista+
			TOKEN_ELEVATION Elevation;
			DWORD ReturnLength;
			if (GetTokenInformation(GetCurrentProcessToken(), TokenElevation, &Elevation, sizeof(Elevation), &ReturnLength))
				return Elevation.TokenIsElevated != 0;

			// Old method
			const auto AdminSid = make_admin_sid();
			if (!AdminSid)
				return false;

			BOOL IsMember;
			return CheckTokenMembership(nullptr, AdminSid.get(), &IsMember) && IsMember;
		}();

		return Result;
	}

	TOKEN_ELEVATION_TYPE elevation_type()
	{
		TOKEN_ELEVATION_TYPE ElevationType;
		DWORD ReturnLength;
		return GetTokenInformation(GetCurrentProcessToken(), TokenElevationType, &ElevationType, sizeof(ElevationType), &ReturnLength)?
			ElevationType :
			TokenElevationTypeDefault;
	}

	handle open_current_process_token(DWORD const DesiredAccess)
	{
		HANDLE Handle;
		if (!OpenProcessToken(GetCurrentProcess(), DesiredAccess, &Handle))
		{
			LOGWARNING(L"open_current_process_token({}): {}"sv, DesiredAccess, last_error());
			return {};
		}

		return handle(Handle);
	}

	privilege::privilege(std::span<const wchar_t* const> const Names)
	{
		if (Names.empty())
			return;

		const block_ptr<TOKEN_PRIVILEGES> NewState(sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES) * (Names.size() - 1));
		NewState->PrivilegeCount = 0;

		std::vector<size_t> NameIndices;
		NameIndices.reserve(Names.size());

		for (const auto& i: Names)
		{
			const auto& Luid = lookup_privilege_value(i);
			if (!Luid)
				continue;

			NewState->Privileges[NewState->PrivilegeCount++] = { *Luid, SE_PRIVILEGE_ENABLED };
			NameIndices.emplace_back(&i - Names.data());
		}

		m_Token = open_current_process_token(TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY);
		if (!m_Token)
			return;

		DWORD ReturnLength;
		m_SavedState.reset(NewState.size());
		m_Changed = AdjustTokenPrivileges(m_Token.native_handle(), FALSE, NewState.data(), static_cast<DWORD>(m_SavedState.size()), m_SavedState.data(), &ReturnLength) && m_SavedState->PrivilegeCount;
		const auto LastError = last_error();
		if (LastError.Win32Error == ERROR_SUCCESS)
			return;

		LOGWARNING(L"AdjustTokenPrivileges(): {}"sv, LastError);

		if (LastError.Win32Error != ERROR_NOT_ALL_ASSIGNED)
			return;

		std::span const
			Privileges(NewState->Privileges, NewState->PrivilegeCount),
			Changed(m_SavedState->Privileges, m_SavedState->PrivilegeCount);

		for (const auto& i: Privileges)
		{
			if (std::ranges::find(Changed, i.Luid, &LUID_AND_ATTRIBUTES::Luid) == Changed.end())
			{
				LOGWARNING(L"{} not enabled"sv, Names[NameIndices[&i - Privileges.data()]]);
			}
		}
	}

	privilege::~privilege()
	{
		if (!m_Changed)
			return;

		SCOPED_ACTION(os::last_error_guard);

		AdjustTokenPrivileges(m_Token.native_handle(), FALSE, m_SavedState.data(), 0, {}, {});
		if (const auto LastError = last_error(); LastError.Win32Error != ERROR_SUCCESS)
			LOGWARNING(L"AdjustTokenPrivileges(): {}"sv, LastError);
	}

	bool privilege::check(std::span<const wchar_t* const> const Names)
	{
		const auto Token = open_current_process_token(TOKEN_QUERY);
		if (!Token)
			return false;

		const block_ptr<PRIVILEGE_SET> PrivilegeSet(sizeof(PRIVILEGE_SET) + sizeof(LUID_AND_ATTRIBUTES) * (Names.size() - 1));
		PrivilegeSet->PrivilegeCount = 0;
		PrivilegeSet->Control = PRIVILEGE_SET_ALL_NECESSARY;

		for (const auto& i: Names)
		{
			const auto& Luid = lookup_privilege_value(i);
			if (!Luid)
				return false;

			PrivilegeSet->Privilege[PrivilegeSet->PrivilegeCount++] = { *Luid };
		}

		BOOL Result;
		if (!PrivilegeCheck(Token.native_handle(), PrivilegeSet.data(), &Result))
		{
			LOGWARNING(L"PrivilegeCheck(): {}"sv, os::last_error());
			return false;
		}

		return Result != FALSE;
	}

	namespace low
	{
		descriptor get_security(const wchar_t* const Object, SE_OBJECT_TYPE const ObjectType, SECURITY_INFORMATION const RequestedInformation)
		{
			descriptor Descriptor;

			if (const auto Result = GetNamedSecurityInfo(
				Object,
				ObjectType,
				RequestedInformation,
				{},
				{},
				{},
				{},
				std::bit_cast<PSECURITY_DESCRIPTOR*>(&ptr_setter(Descriptor)
				)
			); Result != ERROR_SUCCESS)
				SetLastError(Result);

			return Descriptor;
		}

		bool set_security(const wchar_t* const Object, SE_OBJECT_TYPE const ObjectType, SECURITY_INFORMATION RequestedInformation, SECURITY_DESCRIPTOR* const SecurityDescriptor)
		{
			SECURITY_DESCRIPTOR_CONTROL Control;
			DWORD Revision;
			if (!GetSecurityDescriptorControl(SecurityDescriptor, &Control, &Revision))
				return false;

			BOOL Defaulted;

			PSID Owner{};
			if (!GetSecurityDescriptorOwner(SecurityDescriptor, &Owner, &Defaulted))
				return false;

			PSID Group{};
			if (!GetSecurityDescriptorGroup(SecurityDescriptor, &Group, &Defaulted))
				return false;

			BOOL Present;

			PACL Dacl{};
			if (!GetSecurityDescriptorDacl(SecurityDescriptor, &Present, &Dacl, &Defaulted))
				return false;

			PACL Sacl{};
			if (!GetSecurityDescriptorSacl(SecurityDescriptor, &Present, &Sacl, &Defaulted))
				return false;

			if (RequestedInformation & DACL_SECURITY_INFORMATION)
			{
				RequestedInformation |= Control & SE_DACL_PROTECTED?
					PROTECTED_DACL_SECURITY_INFORMATION :
					UNPROTECTED_DACL_SECURITY_INFORMATION;
			}
			if (RequestedInformation & SACL_SECURITY_INFORMATION)
			{
				RequestedInformation |= Control & SE_SACL_PROTECTED?
					PROTECTED_SACL_SECURITY_INFORMATION :
					UNPROTECTED_SACL_SECURITY_INFORMATION;
			}

			if (const auto Result = SetNamedSecurityInfo(
				const_cast<wchar_t*>(Object),
				ObjectType,
				RequestedInformation,
				Owner,
				Group,
				Dacl,
				Sacl
			); Result != ERROR_SUCCESS)
			{
				SetLastError(Result);
				return false;
			}

			return true;
		}

		bool reset_security(const wchar_t* const Object, SE_OBJECT_TYPE const ObjectType)
		{
			ACL EmptyAcl{};
			if (!InitializeAcl(&EmptyAcl, sizeof(EmptyAcl), ACL_REVISION))
				return false;

			if (const auto Result = SetNamedSecurityInfo(
				const_cast<wchar_t*>(Object),
				ObjectType,
				DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION,
				{},
				{},
				&EmptyAcl,
				{}
			); Result != ERROR_SUCCESS)
			{
				SetLastError(Result);
				return false;
			}

			return true;
		}

		bool set_owner(const wchar_t* const Object, SE_OBJECT_TYPE const ObjectType, string const& Computer, string const& Owner)
		{
			const auto Sid = name_to_sid(Owner, Computer);
			if (!Sid)
				return false;

			if (is_owned(Object, ObjectType, Sid.get()))
				return true;

			SCOPED_ACTION(os::security::privilege) { SE_TAKE_OWNERSHIP_NAME, SE_RESTORE_NAME };

			if (const auto Result = SetNamedSecurityInfo(
				const_cast<wchar_t*>(Object),
				ObjectType,
				OWNER_SECURITY_INFORMATION,
				Sid.get(),
				{},
				{},
				{}
				); Result != ERROR_SUCCESS)
			{
				SetLastError(Result);
				return false;
			}

			return true;
		}
	}

	descriptor get_security(string const& Object, SE_OBJECT_TYPE const ObjectType, SECURITY_INFORMATION RequestedInformation)
	{
		if (auto Result = low::get_security(Object.c_str(), ObjectType, RequestedInformation))
			return Result;

		if (ElevationRequired(ELEVATION_READ_REQUEST))
			return elevation::instance().get_security(Object, ObjectType, RequestedInformation);

		return {};
	}

	bool set_security(string const& Object, SE_OBJECT_TYPE const ObjectType, SECURITY_INFORMATION const RequestedInformation, descriptor const& SecurityDescriptor)
	{
		if (low::set_security(Object.c_str(), ObjectType, RequestedInformation, SecurityDescriptor.get()))
			return true;

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
			return elevation::instance().set_security(Object, ObjectType, RequestedInformation, SecurityDescriptor);

		return false;
	}

	bool reset_security(string const& Object, SE_OBJECT_TYPE const ObjectType)
	{
		if (low::reset_security(Object.c_str(), ObjectType))
			return true;

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
			return elevation::instance().reset_security(Object, ObjectType);

		return false;
	}

	bool get_owner(string const& Object, SE_OBJECT_TYPE const ObjectType, string const& Computer, string& Owner)
	{
		const auto SecurityDescriptor = get_security(Object, ObjectType, OWNER_SECURITY_INFORMATION);
		if (!SecurityDescriptor)
			return false;

		const auto OwnerSid = descriptor_get_owner(SecurityDescriptor.get());
		if (!OwnerSid)
			return false;

		return sid_to_name(OwnerSid, Computer, Owner);
	}

	bool set_owner(string const& Object, SE_OBJECT_TYPE const ObjectType, string const& Computer, string const& Owner)
	{
		if (low::set_owner(Object.c_str(), ObjectType, Computer, Owner))
			return true;

		if (ElevationRequired(ELEVATION_MODIFY_REQUEST))
			return elevation::instance().fSetOwner(Object, ObjectType, Computer, Owner);

		return false;
	}
}
