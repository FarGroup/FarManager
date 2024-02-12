#ifndef SHORTCUTS_HPP_29F659B5_FECB_4C3C_8499_D17E01487D1C
#define SHORTCUTS_HPP_29F659B5_FECB_4C3C_8499_D17E01487D1C
#pragma once

/*
shortcuts.hpp

Folder shortcuts
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

// Internal:

// Platform:

// Common:
#include "common/enumerator.hpp"

// External:

//----------------------------------------------------------------------------

class VMenu2;
struct MenuItemEx;

class Shortcuts
{
public:
	NONCOPYABLE(Shortcuts);

	explicit Shortcuts(size_t Index);
	~Shortcuts();

	struct data
	{
		string Folder;
		string PluginFile;
		string PluginData;
		UUID PluginUuid{};

		bool operator==(const data&) const = default;
	};

	static bool Get(size_t Index, data& Data);
	void Add(string_view Folder, const UUID& PluginUuid, string_view PluginFile, string_view PluginData);

	static int Configure();

	auto Enumerator() const
	{
		using value_type = data;
		return inline_enumerator<value_type>([this, Index = 0uz](bool Reset, value_type& Value) mutable
		{
			if (Reset)
				Index = 0;

			return GetOne(Index++, Value);
		});
	}

	class shortcut;

private:
	std::variant<bool, size_t> GetImpl(data& Data, size_t Index, bool CanSkipMenu);
	std::variant<std::list<shortcut>::const_iterator, size_t> Select(bool Raw, size_t Index);
	bool GetOne(size_t Index, data& Data) const;
	void Save();

	std::list<shortcut> m_Items;
	string m_KeyName;
	bool m_Changed{};
};

#endif // SHORTCUTS_HPP_29F659B5_FECB_4C3C_8499_D17E01487D1C
