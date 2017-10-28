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

#include "headers.hpp"
#pragma hdrstop

#include "fileowner.hpp"
#include "platform.security.hpp"
#include "pathmix.hpp"
#include "elevation.hpp"
#include "mix.hpp"

// эта часть - перспективная фигня, которая значительно ускоряет получение овнеров

static bool SidToName(PSID Sid, string& Name, const string& Computer)
{
	DWORD AccountLength = 0, DomainLength = 0;
	SID_NAME_USE snu;
	LookupAccountSid(Computer.data(), Sid, nullptr, &AccountLength, nullptr, &DomainLength, &snu);
	if (AccountLength && DomainLength)
	{
		wchar_t_ptr_n<MAX_PATH> AccountName(AccountLength), DomainName(DomainLength);
		if (!LookupAccountSid(Computer.data(), Sid, AccountName.get(), &AccountLength, DomainName.get(), &DomainLength, &snu))
			return false;

		Name.clear();
		if (DomainLength)
		{
			Name = concat(string_view(DomainName.get(), DomainLength), L'\\');
		}
		Name.append(AccountName.get(), AccountLength);
		return true;
	}

	os::memory::local::ptr<wchar_t> StrSid;
	if (!ConvertSidToStringSid(Sid, &ptr_setter(StrSid)))
		return false;

	Name = StrSid.get();
	return true;
}

static bool SidToNameCached(PSID Sid, string& Name, const string& Computer)
{
	class sid
	{
	public:
		NONCOPYABLE(sid);
		MOVABLE(sid);

		explicit sid(PSID rhs)
		{
			const auto Size = GetLengthSid(rhs);
			m_Data.reset(Size);
			CopySid(Size, m_Data.get(), rhs);
		}

		bool operator==(const sid& rhs) const
		{
			return EqualSid(m_Data.get(), rhs.m_Data.get()) != FALSE;
		}

		size_t get_hash() const
		{
			return CRC32(0, m_Data.get(), GetLengthSid(m_Data.get()));
		}

	private:
		block_ptr<SID> m_Data;
	};

	struct sid_hash { size_t operator()(const sid& Sid) const { return Sid.get_hash(); } };

	static std::unordered_map<sid, string, sid_hash> SIDCache;

	sid SidCopy(Sid);
	const auto ItemIterator = SIDCache.find(SidCopy);

	if (ItemIterator != SIDCache.cend())
	{
		Name = ItemIterator->second;
		return true;
	}

	if (SidToName(Sid, Name, Computer))
	{
		SIDCache.emplace(std::move(SidCopy), Name);
		return true;
	}

	return false;
}

// TODO: elevation
bool GetFileOwner(const string& Computer,const string& Name, string &strOwner)
{
	const auto SecurityDescriptor = os::fs::get_file_security(Name, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION);
	if (!SecurityDescriptor)
		return false;

	PSID pOwner;
	BOOL OwnerDefaulted;
	if (!GetSecurityDescriptorOwner(SecurityDescriptor.get(), &pOwner, &OwnerDefaulted))
		return false;

	if (!IsValidSid(pOwner))
		return false;

	return SidToNameCached(pOwner, strOwner, Computer);
}

static auto get_sid(const string& Name)
{
	os::memory::local::ptr<void> Sid;
	if (ConvertStringSidToSid(Name.data(), &ptr_setter(Sid)))
		return Sid;

	SID_NAME_USE Use;
	DWORD SidSize = 0, ReferencedDomainNameSize = 0;
	LookupAccountName(nullptr, Name.data(), nullptr, &SidSize, nullptr, &ReferencedDomainNameSize, &Use);
	if (!SidSize)
		return Sid;

	Sid = os::memory::local::alloc<SID>(LMEM_FIXED, SidSize);
	if (!Sid)
		return Sid;

	wchar_t_ptr_n<MAX_PATH> Buffer(ReferencedDomainNameSize);
	LookupAccountName(nullptr, Name.data(), Sid.get(), &SidSize, Buffer.get(), &ReferencedDomainNameSize, &Use);

	return Sid;
}

bool SetOwnerInternal(const string& Object, const string& Owner)
{
	const auto Sid = get_sid(Owner);
	if (!Sid)
		return false;

	SCOPED_ACTION(os::security::privilege){ SE_TAKE_OWNERSHIP_NAME, SE_RESTORE_NAME };

	const auto Result = SetNamedSecurityInfo(const_cast<LPWSTR>(Object.data()), SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, Sid.get(), nullptr, nullptr, nullptr);
	if(Result != ERROR_SUCCESS)
	{
		SetLastError(Result);
		return false;
	}
	return true;
}

bool SetFileOwner(const string& Object, const string& Owner)
{
	NTPath strNtObject(Object);

	if (SetOwnerInternal(strNtObject, Owner))
		return true;

	if(ElevationRequired(ELEVATION_MODIFY_REQUEST))
		return elevation::instance().fSetOwner(strNtObject, Owner);

	return false;
}
