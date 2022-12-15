#ifndef HILIGHT_HPP_DE941BF9_997F_45B6_A454_3F455C156548
#define HILIGHT_HPP_DE941BF9_997F_45B6_A454_3F455C156548
#pragma once

/*
hilight.hpp

Files highlighting
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
#include "plugin.hpp"

// Platform:
#include "platform.chrono.hpp"

// Common:
#include "common/noncopyable.hpp"

// External:

//----------------------------------------------------------------------------

class FileFilterParams;
class FileList;
class FileListItem;
class HierarchicalConfig;
class VMenu2;

namespace highlight
{
	struct color
	{
		enum
		{
			normal,
			selected,
			normal_current,
			selected_current,

			count
		};
	};

	class element
	{
	private:
		struct colors
		{
			colors();

			FarColor FileColor;
			FarColor MarkColor;

			bool operator==(const colors&) const = default;
		};

		struct mark
		{
			string Mark;
			bool Inherit{true};

			bool operator==(const mark&) const = default;
		};

	public:
		using colors_array = std::array<colors, color::count>;
		colors_array Color;
		mark Mark;

		bool operator==(const element&) const = default;
	};

	class configuration: noncopyable
	{
	public:
		configuration();

		void UpdateCurrentTime();
		const element* GetHiColor(const FileListItem& Item, const FileList* Owner, bool UseAttrHighlighting = false);
		int GetGroup(const FileListItem& Object, const FileList* Owner);
		void HiEdit(int MenuPos);
		void UpdateHighlighting(bool RefreshMasks = false);
		void Save(bool Always);

		static void ApplyFinalColor(element::colors_array::value_type& Colors, size_t PaletteIndex);

	private:
		void Load(/*const*/ HierarchicalConfig& cfg);
		void ClearData();
		int  MenuPosToRealPos(int MenuPos, int*& Count, bool Insert = false);
		void FillMenu(VMenu2 *HiMenu, int MenuPos) const;
		void ProcessGroups();

		struct element_hash
		{
			size_t operator()(const element& item) const;
		};

		std::unordered_set<element, element_hash> m_Colors;
		std::vector<FileFilterParams> HiData;

		int FirstCount{}, UpperCount{}, LowerCount{}, LastCount{};
		os::chrono::time_point CurrentTime;
		bool m_Changed{};
	};
}

#endif // HILIGHT_HPP_DE941BF9_997F_45B6_A454_3F455C156548
