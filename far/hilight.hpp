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

class VMenu2;
class FileFilterParams;
class FileList;
class FileListItem;

class HighlightFiles: noncopyable
{
public:
	enum highlight_color
	{
		NORMAL_COLOR,
		SELECTED_COLOR,
		UNDERCURSOR_COLOR,
		SELECTEDUNDERCURSOR_COLOR,

		HIGHLIGHT_COUNT
	};

	struct highlight_item
	{
	private:
		struct color
		{
			FarColor FileColor;
			FarColor MarkColor;

			bool operator ==(const color& rhs) const { return FileColor == rhs.FileColor && MarkColor == rhs.MarkColor; }
		};

		struct mark
		{
			wchar_t Char;
			bool Transparent;

			bool operator ==(const mark& rhs) const { return Char == rhs.Char && Transparent == rhs.Transparent; }
		};

	public:
		using colors_array = std::array<color, HIGHLIGHT_COUNT>;
		colors_array Color;
		mark Mark;

		bool operator ==(const highlight_item& rhs) const
		{
			return Color == rhs.Color && Mark == rhs.Mark;
		}
	};

	HighlightFiles();

	void UpdateCurrentTime();
	const highlight_item* GetHiColor(const FileListItem& Item, const FileList* Owner, bool UseAttrHighlighting = false);
	int GetGroup(const FileListItem *fli, const FileList* Owner);
	void HiEdit(int MenuPos);
	void UpdateHighlighting(bool RefreshMasks = false);
	void Save(bool always);

	static void ApplyFinalColor(highlight_item::colors_array::value_type& Colors, size_t PaletteIndex);

private:
	void InitHighlightFiles(class HierarchicalConfig* cfg);
	void ClearData();
	int  MenuPosToRealPos(int MenuPos, int*& Count, bool Insert=false);
	void FillMenu(VMenu2 *HiMenu,int MenuPos);
	void ProcessGroups();

	struct highlight_item_hash
	{
		size_t operator()(const highlight_item& item) const
		{
			return std::accumulate(ALL_CONST_RANGE(item.Color), size_t(0), [](auto Value, const auto& i)
			{
				return Value ^ make_hash(i.FileColor) ^ make_hash(i.MarkColor);
			}) ^ make_hash(item.Mark.Char) ^ make_hash(item.Mark.Transparent);
		}
	};

	std::unordered_set<highlight_item, highlight_item_hash> Colors;
	std::vector<FileFilterParams> HiData;

	int FirstCount, UpperCount, LowerCount, LastCount;
	unsigned long long CurrentTime;
	bool Changed;
};

#endif // HILIGHT_HPP_DE941BF9_997F_45B6_A454_3F455C156548
