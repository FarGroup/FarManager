#ifndef PLUGSETTINGS_HPP_E3377566_8E58_48BB_9A8A_B205A943BD6F
#define PLUGSETTINGS_HPP_E3377566_8E58_48BB_9A8A_B205A943BD6F
#pragma once

/*
plugsettings.hpp

API для хранения плагинами настроек.
*/
/*
Copyright © 2011 Far Group
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

// Internal:

// Platform:

// Common:
#include "common/noncopyable.hpp"
#include "common/smart_ptr.hpp"

// External:

//----------------------------------------------------------------------------

struct FarSettingsEnum;
struct FarSettingsItem;
struct FarSettingsValue;

class AbstractSettings: noncopyable
{
public:
	virtual ~AbstractSettings() = default;
	virtual bool Set(const FarSettingsItem& Item) = 0;
	virtual bool Get(FarSettingsItem& Item) = 0;
	virtual bool Enum(FarSettingsEnum& Enum) = 0;
	virtual bool Delete(const FarSettingsValue& Value) = 0;
	virtual int SubKey(const FarSettingsValue& Value, bool bCreate) = 0;

	static std::unique_ptr<AbstractSettings> CreateFarSettings();
	static std::unique_ptr<AbstractSettings> CreatePluginSettings(const UUID& Uuid, bool Local);

protected:
	const wchar_t* Add(string_view String);
	const void* Add(const void* Data, size_t Size);

private:
	void* Allocate(size_t Size);

	std::vector<char_ptr> m_Data;
};

#endif // PLUGSETTINGS_HPP_E3377566_8E58_48BB_9A8A_B205A943BD6F
