﻿/*
preservelongname.cpp

class PreserveLongName
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
#include "preservelongname.hpp"

// Internal:
#include "exception.hpp"
#include "log.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:

// External:

//----------------------------------------------------------------------------

PreserveLongName::PreserveLongName(string_view const Name, bool Preserve):
	m_Preserve(Preserve)
{
	if (!Preserve)
		return;

	os::fs::find_data FindData;
	if (!os::fs::get_find_data(Name, FindData))
	{
		m_Preserve = false;
		return;
	}

	m_SaveLongName = FindData.FileName;
	m_SaveShortName = FindData.AlternateFileName();
}


PreserveLongName::~PreserveLongName()
{
	if (!m_Preserve || os::fs::exists(m_SaveLongName) || !os::fs::exists(m_SaveShortName))
		return;

	os::fs::find_data FindData;
	const auto LongNameLost = os::fs::get_find_data(m_SaveShortName, FindData) && FindData.FileName != m_SaveLongName;

	if (!LongNameLost)
		return;

	// BUGBUG check result
	if (!os::fs::move_file(m_SaveShortName, m_SaveLongName))
	{
		LOGWARNING(L"move_file({}, {}): {}"sv, m_SaveShortName, m_SaveLongName, last_error());
	}
}
