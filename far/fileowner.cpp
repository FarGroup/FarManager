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
#include "pathmix.hpp"
#include "privilege.hpp"
#include "elevation.hpp"
#include "mix.hpp"

// эта часть - перспективная фигня, которая значительно ускоряет получение овнеров

static bool SidToName(PSID Sid, string& Name, const string& Computer)
{
	bool Result = false;
	DWORD AccountLength=0,DomainLength=0;
	SID_NAME_USE snu;
	LookupAccountSid(Computer.data(), Sid, nullptr, &AccountLength, nullptr, &DomainLength, &snu);
	if (AccountLength && DomainLength)
	{
		wchar_t_ptr AccountName(AccountLength);
		wchar_t_ptr DomainName(DomainLength);
		if(LookupAccountSid(Computer.data(), Sid, AccountName.get(), &AccountLength, DomainName.get(), &DomainLength, &snu))
		{
			Name.assign(DomainName.get(), DomainLength).append(L"\\").append(AccountName.get(), AccountLength);
			Result = true;
		}
	}
	else
	{
		wchar_t* Ptr;
		if(ConvertSidToStringSid(Sid, &Ptr))
		{
			const auto StrSid = os::memory::local::ptr(Ptr);
			Name = StrSid.get();
			Result = true;
		}
	}
	return Result;
}

static bool SidToNameCached(PSID Sid, string& Name, const string& Computer)
{
	bool Result = false;

	class sid
	{
	public:
		NONCOPYABLE(sid);
		TRIVIALLY_MOVABLE(sid);

		sid(PSID rhs)
		{
			DWORD Size = GetLengthSid(rhs);
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
		Result = true;
	}
	else
	{
		if (SidToName(Sid, Name, Computer))
		{
			SIDCache.emplace(std::move(SidCopy), Name);
			Result = true;
		}
	}
	return Result;
}

bool GetFileOwner(const string& Computer,const string& Name, string &strOwner)
{
	bool Result=false;

	strOwner.clear();
	SECURITY_INFORMATION si=OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION;;
	DWORD LengthNeeded=0;
	NTPath strName(Name);
	static thread_local char sddata[64 * 1024];
	const auto sd = reinterpret_cast<SECURITY_DESCRIPTOR*>(sddata);

	if (GetFileSecurity(strName.data(),si,sd,sizeof(sddata),&LengthNeeded) && LengthNeeded<=sizeof(sddata))
	{
		PSID pOwner;
		BOOL OwnerDefaulted;
		if (GetSecurityDescriptorOwner(sd,&pOwner,&OwnerDefaulted))
		{
			if (IsValidSid(pOwner))
			{
				Result = SidToNameCached(pOwner, strOwner, Computer);
			}
		}
	}
	return Result;
}

bool SetOwnerInternal(const string& Object, const string& Owner)
{
	bool Result = false;

	PSID Ptr = nullptr;
	os::memory::local::ptr_t<void> Sid;
	if (ConvertStringSidToSid(Owner.data(), &Ptr))
	{
		Sid.reset(Ptr);
	}
	else
	{
		SID_NAME_USE Use;
		DWORD cSid=0, ReferencedDomain=0;
		LookupAccountName(nullptr, Owner.data(), nullptr, &cSid, nullptr, &ReferencedDomain, &Use);
		if(cSid)
		{
			Sid = os::memory::local::alloc<PSID>(LMEM_FIXED, cSid);
			if(Sid)
			{
				std::vector<wchar_t> ReferencedDomainName(ReferencedDomain);
				if(LookupAccountName(nullptr, Owner.data(), Sid.get(), &cSid, ReferencedDomainName.data(), &ReferencedDomain, &Use))
				{
					;
				}
			}
		}
	}
	if(Sid)
	{
		SCOPED_ACTION(os::security::privilege)(make_vector<const wchar_t*>(SE_TAKE_OWNERSHIP_NAME, SE_RESTORE_NAME));
		DWORD dwResult = SetNamedSecurityInfo(const_cast<LPWSTR>(Object.data()), SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, Sid.get(), nullptr, nullptr, nullptr);
		if(dwResult == ERROR_SUCCESS)
		{
			Result = true;
		}
		else
		{
			SetLastError(dwResult);
		}
	}
	return Result;
}

bool SetFileOwner(const string& Object, const string& Owner)
{
	NTPath strNtObject(Object);
	bool Result = SetOwnerInternal(strNtObject, Owner);
	if(!Result && ElevationRequired(ELEVATION_MODIFY_REQUEST))
	{
		Result = Global->Elevation->fSetOwner(strNtObject, Owner);
	}
	return Result;
}
