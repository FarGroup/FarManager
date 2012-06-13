#pragma once

/*
filefilterparams.hpp

Параметры Файлового фильтра
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

#include "plugin.hpp"
#include "CFileMask.hpp"
#include "bitflags.hpp"
#include "hilight.hpp"

#define FILEFILTER_SIZE_SIZE 32

#define DEFAULT_SORT_GROUP 10000

enum enumFileFilterFlagsType
{
	FFFT_FIRST = 0, //обязан быть первым

	FFFT_LEFTPANEL = FFFT_FIRST,
	FFFT_RIGHTPANEL,
	FFFT_FINDFILE,
	FFFT_COPY,
	FFFT_SELECT,
	FFFT_CUSTOM,

	FFFT_COUNT, //обязан быть последним
};

enum enumFileFilterFlags
{
	FFF_NONE    = 0x00000000,
	FFF_INCLUDE = 0x00000001,
	FFF_EXCLUDE = 0x00000002,
	FFF_STRONG  = 0x10000000
};

enum enumFDateType
{
	FDATE_MODIFIED=0,
	FDATE_CREATED,
	FDATE_OPENED,
	FDATE_CHANGED,

	FDATE_COUNT, // всегда последний !!!
};

class FileFilterParams
{
	private:

		string m_strTitle;

		struct
		{
			bool Used;
			string strMask;
			CFileMask FilterMask; // Хранилище скомпилированной маски.
		} FMask;

		struct
		{
			ULARGE_INTEGER DateAfter;
			ULARGE_INTEGER DateBefore;
			enumFDateType DateType;
			bool Used;
			bool bRelative;
		} FDate;

		struct
		{
			unsigned __int64 SizeAboveReal; // Здесь всегда будет размер в байтах
			unsigned __int64 SizeBelowReal; // Здесь всегда будет размер в байтах
			wchar_t SizeAbove[FILEFILTER_SIZE_SIZE]; // Здесь всегда будет размер как его ввёл юзер
			wchar_t SizeBelow[FILEFILTER_SIZE_SIZE]; // Здесь всегда будет размер как его ввёл юзер
			bool Used;
		} FSize;

		struct // Новая структура в фильтре, чтобы считать количество жестких ссылок. Пока что реально используем только флаг Used и априорно заданное условие "ссылок больше чем одна"
		{
			bool Used;
			DWORD CountAbove;
			DWORD CountBelow;
		} FHardLinks;

		struct
		{
			bool Used;
			DWORD AttrSet;
			DWORD AttrClear;
		} FAttr;

		struct
		{
			HighlightDataColor Colors;
			int SortGroup;
			bool bContinueProcessing;
		} FHighlight;

		DWORD FFlags[FFFT_COUNT];

	public:

		FileFilterParams();

		FileFilterParams &operator=(const FileFilterParams &FF);

		void SetTitle(const wchar_t *Title);
		void SetMask(bool Used, const wchar_t *Mask);
		void SetDate(bool Used, DWORD DateType, FILETIME DateAfter, FILETIME DateBefore, bool bRelative);
		void SetSize(bool Used, const wchar_t *SizeAbove, const wchar_t *SizeBelow);
		void SetHardLinks(bool Used,DWORD HardLinksAbove, DWORD HardLinksBelow);
		void SetAttr(bool Used, DWORD AttrSet, DWORD AttrClear);
		void SetColors(HighlightDataColor *Colors);
		void SetSortGroup(int SortGroup) { FHighlight.SortGroup = SortGroup; }
		void SetContinueProcessing(bool bContinueProcessing) { FHighlight.bContinueProcessing = bContinueProcessing; }
		void SetFlags(enumFileFilterFlagsType FType, DWORD Flags) { FFlags[FType] = Flags; }
		void ClearAllFlags() { ClearArray(FFlags); }

		const wchar_t *GetTitle() const;
		bool  GetMask(const wchar_t **Mask) const;
		bool  GetDate(DWORD *DateType, FILETIME *DateAfter, FILETIME *DateBefore, bool *bRelative) const;
		bool  GetSize(const wchar_t **SizeAbove, const wchar_t **SizeBelow) const;
		bool  GetHardLinks(DWORD *HardLinksAbove, DWORD *HardLinksBelow) const;
		bool  GetAttr(DWORD *AttrSet, DWORD *AttrClear) const;
		void  GetColors(HighlightDataColor *Colors) const;
		int   GetMarkChar() const;
		int   GetSortGroup() const { return FHighlight.SortGroup; }
		bool  GetContinueProcessing() const { return FHighlight.bContinueProcessing; }
		DWORD GetFlags(enumFileFilterFlagsType FType) const { return FFlags[FType]; }

		// Данный метод вызывается "снаружи" и служит для определения:
		// попадает ли файл fd под условие установленного фильтра.
		// Возвращает true  - попадает;
		//            false - не попадает.
		bool FileInFilter(const FileListItem& fli, unsigned __int64 CurrentTime);
		bool FileInFilter(const FAR_FIND_DATA_EX& fde, unsigned __int64 CurrentTime,const string* FullName=nullptr); //Used in dirinfo, copy, findfile
		bool FileInFilter(const PluginPanelItem& fd, unsigned __int64 CurrentTime);
};

bool FileFilterConfig(FileFilterParams *FF, bool ColorConfig=false);

//Централизованная функция для создания строк меню различных фильтров.
void MenuString(string &strDest, FileFilterParams *FF, bool bHighlightType=false, int Hotkey=0, bool bPanelType=false, const wchar_t *FMask=nullptr, const wchar_t *Title=nullptr);
