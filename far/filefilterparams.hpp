#ifndef FILEFILTERPARAMS_HPP_E3E125BE_F0C2_4DAC_9582_FE7EDD2DA264
#define FILEFILTERPARAMS_HPP_E3E125BE_F0C2_4DAC_9582_FE7EDD2DA264
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

#include "filemasks.hpp"
#include "hilight.hpp"

#include "platform.chrono.hpp"
#include "platform.fwd.hpp"

enum
{
	DEFAULT_SORT_GROUP = 10000,
};

enum enumFileFilterFlagsType: int
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

class filter_dates
{
public:
	explicit filter_dates(os::chrono::duration After = {}, os::chrono::duration Before = {});
	explicit filter_dates(os::chrono::time_point After, os::chrono::time_point Before);

	explicit operator bool() const;

	template<typename callable>
	decltype(auto) visit(const callable& Callable) const
	{
		return m_Relative?
			Callable(m_After, m_Before) :
			Callable(os::chrono::time_point(m_After), os::chrono::time_point(m_Before));
	}

private:
	os::chrono::duration m_After;
	os::chrono::duration m_Before;
	bool m_Relative;
};

class FileFilterParams
{
public:
	NONCOPYABLE(FileFilterParams);
	MOVABLE(FileFilterParams);

	FileFilterParams();

	FileFilterParams Clone() const;

	void SetTitle(string_view Title);
	void SetMask(bool Used, string_view Mask);
	void SetDate(bool Used, enumFDateType DateType, const filter_dates& Dates);
	void SetSize(bool Used, string_view SizeAbove, string_view SizeBelow);
	void SetHardLinks(bool Used, DWORD HardLinksAbove, DWORD HardLinksBelow);
	void SetAttr(bool Used, DWORD AttrSet, DWORD AttrClear);
	void SetColors(const highlight::element& Colors);
	void SetSortGroup(int SortGroup) { FHighlight.SortGroup = SortGroup; }
	void SetContinueProcessing(bool bContinueProcessing) { FHighlight.bContinueProcessing = bContinueProcessing; }
	void SetFlags(enumFileFilterFlagsType FType, DWORD Flags) { FFlags[FType] = Flags; }
	void ClearAllFlags() { FFlags.fill(0); }

	const string& GetTitle() const;
	const string& GetMask() const { return FMask.strMask; }
	bool IsMaskUsed() const { return FMask.Used; }
	bool GetDate(DWORD* DateType, filter_dates* Dates) const;
	bool IsSizeUsed() const {return FSize.Used;}
	const string& GetSizeAbove() const {return FSize.SizeAbove;}
	const string& GetSizeBelow() const {return FSize.SizeBelow;}
	bool  GetHardLinks(DWORD *HardLinksAbove, DWORD *HardLinksBelow) const;
	bool  GetAttr(DWORD *AttrSet, DWORD *AttrClear) const;
	highlight::element GetColors() const;
	wchar_t GetMarkChar() const;
	int   GetSortGroup() const { return FHighlight.SortGroup; }
	bool  GetContinueProcessing() const { return FHighlight.bContinueProcessing; }
	DWORD GetFlags(enumFileFilterFlagsType FType) const { return FFlags[FType]; }
	void RefreshMask() {if(FMask.Used) FMask.FilterMask.Set(FMask.strMask, FMF_SILENT);}


	// Данный метод вызывается "снаружи" и служит для определения:
	// попадает ли файл fd под условие установленного фильтра.
	// Возвращает true  - попадает;
	//            false - не попадает.
	bool FileInFilter(const FileListItem* fli, const FileList* Owner, os::chrono::time_point CurrentTime) const;
	bool FileInFilter(const os::fs::find_data& fde, os::chrono::time_point CurrentTime, const string* FullName = nullptr) const; //Used in dirinfo, copy, findfile
	bool FileInFilter(const PluginPanelItem& fd, os::chrono::time_point CurrentTime) const;


private:
	bool FileInFilter(struct filter_file_object& Object, os::chrono::time_point CurrentTime, const std::function<void(filter_file_object&)>& Getter) const;

	string m_strTitle;

	struct fmask
	{
		bool Used;
		string strMask;
		filemasks FilterMask; // Хранилище скомпилированной маски.
	} FMask;

	struct
	{
		filter_dates Dates;
		enumFDateType DateType;
		bool Used;
	} FDate;

	struct
	{
		unsigned long long SizeAboveReal; // Здесь всегда будет размер в байтах
		unsigned long long SizeBelowReal; // Здесь всегда будет размер в байтах
		string SizeAbove; // Здесь всегда будет размер как его ввёл юзер
		string SizeBelow; // Здесь всегда будет размер как его ввёл юзер
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
		highlight::element Colors;
		int SortGroup;
		bool bContinueProcessing;
	} FHighlight;

	std::array<DWORD, FFFT_COUNT> FFlags;
};

bool FileFilterConfig(FileFilterParams *FF, bool ColorConfig=false);

//Централизованная функция для создания строк меню различных фильтров.
string MenuString(const FileFilterParams* FF, bool bHighlightType=false, wchar_t Hotkey = 0, bool bPanelType=false, const wchar_t *FMask=nullptr, const wchar_t *Title=nullptr);

#endif // FILEFILTERPARAMS_HPP_E3E125BE_F0C2_4DAC_9582_FE7EDD2DA264
