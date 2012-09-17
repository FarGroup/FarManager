/*
filefilterparams.cpp

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

#include "headers.hpp"
#pragma hdrstop

#include "colors.hpp"
#include "CFileMask.hpp"
#include "FileMasksWithExclude.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "filelist.hpp"
#include "filefilterparams.hpp"
#include "palette.hpp"
#include "message.hpp"
#include "interf.hpp"
#include "setcolor.hpp"
#include "datetime.hpp"
#include "strmix.hpp"
#include "mix.hpp"
#include "console.hpp"
#include "flink.hpp"

FileFilterParams::FileFilterParams()
{
	SetMask(1,L"*");
	SetSize(0,L"",L"");
	SetHardLinks(0,0,0);
	ClearStruct(FDate);
	ClearStruct(FAttr);
	ClearStruct(FHighlight.Colors);
	for(size_t i = 0; i < 2; ++i)
	{
		for(size_t j = 0; j < 4; ++j)
		{
			MAKE_OPAQUE(FHighlight.Colors.Color[i][j].ForegroundColor);
		}
	}

	FHighlight.SortGroup=DEFAULT_SORT_GROUP;
	FHighlight.bContinueProcessing=false;
	ClearAllFlags();
}

FileFilterParams &FileFilterParams::operator=(const FileFilterParams &FF)
{
	if (this != &FF)
	{
		SetTitle(FF.GetTitle());
		const wchar_t *Mask;
		FF.GetMask(&Mask);
		SetMask(FF.GetMask(nullptr),Mask);
		FSize=FF.FSize;
		FDate=FF.FDate;
		FAttr=FF.FAttr;
		FHardLinks = FF.FHardLinks;
		FF.GetColors(&FHighlight.Colors);
		FHighlight.SortGroup=FF.GetSortGroup();
		FHighlight.bContinueProcessing=FF.GetContinueProcessing();
		memcpy(FFlags,FF.FFlags,sizeof(FFlags));
	}

	return *this;
}

void FileFilterParams::SetTitle(const wchar_t *Title)
{
	m_strTitle = Title;
}

void FileFilterParams::SetMask(bool Used, const wchar_t *Mask)
{
	FMask.Used = Used;
	FMask.strMask = Mask;

	// Проверка на валидность текущих настроек фильтра
	if (!FMask.FilterMask.Set(FMask.strMask,FMF_SILENT))
	{
		FMask.strMask = L"*";
		FMask.FilterMask.Set(FMask.strMask,FMF_SILENT);
	}
}

void FileFilterParams::SetDate(bool Used, DWORD DateType, FILETIME DateAfter, FILETIME DateBefore, bool bRelative)
{
	FDate.Used=Used;
	FDate.DateType=(enumFDateType)DateType;

	if (DateType>=FDATE_COUNT)
		FDate.DateType=FDATE_MODIFIED;

	FDate.DateAfter.u.LowPart=DateAfter.dwLowDateTime;
	FDate.DateAfter.u.HighPart=DateAfter.dwHighDateTime;
	FDate.DateBefore.u.LowPart=DateBefore.dwLowDateTime;
	FDate.DateBefore.u.HighPart=DateBefore.dwHighDateTime;
	FDate.bRelative=bRelative;
}

void FileFilterParams::SetSize(bool Used, const wchar_t *SizeAbove, const wchar_t *SizeBelow)
{
	FSize.Used=Used;
	xwcsncpy(FSize.SizeAbove,SizeAbove,ARRAYSIZE(FSize.SizeAbove));
	xwcsncpy(FSize.SizeBelow,SizeBelow,ARRAYSIZE(FSize.SizeBelow));
	FSize.SizeAboveReal=ConvertFileSizeString(FSize.SizeAbove);
	FSize.SizeBelowReal=ConvertFileSizeString(FSize.SizeBelow);
}

void FileFilterParams::SetHardLinks(bool Used, DWORD HardLinksAbove, DWORD HardLinksBelow)
{
	FHardLinks.Used=Used;
	FHardLinks.CountAbove=HardLinksAbove;
	FHardLinks.CountBelow=HardLinksBelow;
}

void FileFilterParams::SetAttr(bool Used, DWORD AttrSet, DWORD AttrClear)
{
	FAttr.Used=Used;
	FAttr.AttrSet=AttrSet;
	FAttr.AttrClear=AttrClear;
}

void FileFilterParams::SetColors(HighlightDataColor *Colors)
{
	FHighlight.Colors=*Colors;
}

const wchar_t *FileFilterParams::GetTitle() const
{
	return m_strTitle;
}

bool FileFilterParams::GetMask(const wchar_t **Mask) const
{
	if (Mask)
		*Mask=FMask.strMask;

	return FMask.Used;
}

bool FileFilterParams::GetDate(DWORD *DateType, FILETIME *DateAfter, FILETIME *DateBefore, bool *bRelative) const
{
	if (DateType)
		*DateType=FDate.DateType;

	if (DateAfter)
	{
		DateAfter->dwLowDateTime=FDate.DateAfter.u.LowPart;
		DateAfter->dwHighDateTime=FDate.DateAfter.u.HighPart;
	}

	if (DateBefore)
	{
		DateBefore->dwLowDateTime=FDate.DateBefore.u.LowPart;
		DateBefore->dwHighDateTime=FDate.DateBefore.u.HighPart;
	}

	if (bRelative)
		*bRelative=FDate.bRelative;

	return FDate.Used;
}

bool FileFilterParams::GetSize(const wchar_t **SizeAbove, const wchar_t **SizeBelow) const
{
	if (SizeAbove)
		*SizeAbove=FSize.SizeAbove;

	if (SizeBelow)
		*SizeBelow=FSize.SizeBelow;

	return FSize.Used;
}

bool FileFilterParams::GetHardLinks(DWORD *HardLinksAbove, DWORD *HardLinksBelow) const
{
	if (HardLinksAbove)
		*HardLinksAbove = FHardLinks.CountAbove;
	if (HardLinksBelow)
		*HardLinksBelow = FHardLinks.CountBelow;
	return FHardLinks.Used;
}


bool FileFilterParams::GetAttr(DWORD *AttrSet, DWORD *AttrClear) const
{
	if (AttrSet)
		*AttrSet=FAttr.AttrSet;

	if (AttrClear)
		*AttrClear=FAttr.AttrClear;

	return FAttr.Used;
}

void FileFilterParams::GetColors(HighlightDataColor *Colors) const
{
	*Colors=FHighlight.Colors;
}

int FileFilterParams::GetMarkChar() const
{
	return FHighlight.Colors.MarkChar;
}

bool FileFilterParams::FileInFilter(const FileListItem& fli, unsigned __int64 CurrentTime)
{
	FAR_FIND_DATA_EX fde;
	fde.dwFileAttributes=fli.FileAttr;
	fde.ftCreationTime=fli.CreationTime;
	fde.ftLastAccessTime=fli.AccessTime;
	fde.ftLastWriteTime=fli.WriteTime;
	fde.ftChangeTime=fli.ChangeTime;
	fde.nFileSize=fli.FileSize;
	fde.nAllocationSize=fli.AllocationSize;
	fde.strFileName=fli.strName;
	fde.strAlternateFileName=fli.strShortName;
	return FileInFilter(fde, CurrentTime, &fli.strName);
}

bool FileFilterParams::FileInFilter(const FAR_FIND_DATA_EX& fde, unsigned __int64 CurrentTime,const string* FullName)
{
	// Режим проверки атрибутов файла включен?
	if (FAttr.Used)
	{
		// Проверка попадания файла по установленным атрибутам
		if ((fde.dwFileAttributes & FAttr.AttrSet) != FAttr.AttrSet)
			return false;

		// Проверка попадания файла по отсутствующим атрибутам
		if (fde.dwFileAttributes & FAttr.AttrClear)
			return false;
	}

	// Режим проверки размера файла включен?
	if (FSize.Used)
	{
		if (*FSize.SizeAbove)
		{
			if (fde.nFileSize < FSize.SizeAboveReal) // Размер файла меньше минимального разрешённого по фильтру?
				return false;                          // Не пропускаем этот файл
		}

		if (*FSize.SizeBelow)
		{
			if (fde.nFileSize > FSize.SizeBelowReal) // Размер файла больше максимального разрешённого по фильтру?
				return false;                          // Не пропускаем этот файл
		}
	}

	// Режим проверки количества жестких ссылок на файл включен?
	// Пока что, при включенном условии, срабатывание происходит при случае "ссылок больше чем одна"
	if (FHardLinks.Used)
	{
		if (fde.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			return false;
		}

		if (!FullName || GetNumberOfLinks(*FullName) < 2)
		{
			return false;
		}
	}

	// Режим проверки времени файла включен?
	if (FDate.Used)
	{
		// Преобразуем FILETIME в беззнаковый __int64
		unsigned __int64 after  = FDate.DateAfter.QuadPart;
		unsigned __int64 before = FDate.DateBefore.QuadPart;

		if (after || before)
		{
			const FILETIME *ft;

			switch (FDate.DateType)
			{
				case FDATE_CREATED:
					ft=&fde.ftCreationTime;
					break;
				case FDATE_OPENED:
					ft=&fde.ftLastAccessTime;
					break;
				case FDATE_CHANGED:
					ft=&fde.ftChangeTime;
					break;
				default: //case FDATE_MODIFIED:
					ft=&fde.ftLastWriteTime;
			}

			ULARGE_INTEGER ftime = {ft->dwLowDateTime, ft->dwHighDateTime};

			if (FDate.bRelative)
			{
				if (after)
					after = CurrentTime - after;

				if (before)
					before = CurrentTime - before;
			}

			// Есть введённая пользователем начальная дата?
			if (after)
			{
				// Дата файла меньше начальной даты по фильтру?
				if (ftime.QuadPart<after)
					// Не пропускаем этот файл
					return false;
			}

			// Есть введённая пользователем конечная дата?
			if (before)
			{
				// Дата файла больше конечной даты по фильтру?
				if (ftime.QuadPart>before)
					return false;
			}
		}
	}

	// Режим проверки маски файла включен?
	if (FMask.Used)
	{
		// Файл не попадает под маску введённую в фильтре?
		if (!FMask.FilterMask.Compare(fde.strFileName))
			// Не пропускаем этот файл
			return false;
	}

	// Да! Файл выдержал все испытания и будет допущен к использованию
	// в вызвавшей эту функцию операции.
	return true;
}

bool FileFilterParams::FileInFilter(const PluginPanelItem& fd, unsigned __int64 CurrentTime)
{
	FAR_FIND_DATA_EX fde;
	PluginPanelItemToFindDataEx(&fd, &fde);
	return FileInFilter(fde, CurrentTime, &fde.strFileName);
}

//Централизованная функция для создания строк меню различных фильтров.
void MenuString(string &strDest, FileFilterParams *FF, bool bHighlightType, int Hotkey, bool bPanelType, const wchar_t *FMask, const wchar_t *Title)
{
	const wchar_t AttrC[] = L"RAHSDCEI$TLOV";
	const DWORD   AttrF[] =
	{
		FILE_ATTRIBUTE_READONLY,
		FILE_ATTRIBUTE_ARCHIVE,
		FILE_ATTRIBUTE_HIDDEN,
		FILE_ATTRIBUTE_SYSTEM,
		FILE_ATTRIBUTE_DIRECTORY,
		FILE_ATTRIBUTE_COMPRESSED,
		FILE_ATTRIBUTE_ENCRYPTED,
		FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
		FILE_ATTRIBUTE_SPARSE_FILE,
		FILE_ATTRIBUTE_TEMPORARY,
		FILE_ATTRIBUTE_REPARSE_POINT,
		FILE_ATTRIBUTE_OFFLINE,
		FILE_ATTRIBUTE_VIRTUAL,
	};
	const wchar_t Format1a[] = L"%-21.21s %c %-26.26s %-3.3s %c %s";
	const wchar_t Format1b[] = L"%-22.22s %c %-26.26s %-3.3s %c %s";
	const wchar_t Format1c[] = L"&%c. %-18.18s %c %-26.26s %-3.3s %c %s";
	const wchar_t Format1d[] = L"   %-18.18s %c %-26.26s %-3.3s %c %s";
	const wchar_t Format2[]  = L"%-3.3s %c %-26.26s %-4.4s %c %s";
	const wchar_t DownArrow=0x2193;
	const wchar_t *Name, *Mask;
	wchar_t MarkChar[]=L"\" \"";
	DWORD IncludeAttr, ExcludeAttr;
	bool UseMask, UseSize, UseHardLinks, UseDate, RelativeDate;

	if (bPanelType)
	{
		Name=Title;
		UseMask=true;
		Mask=FMask;
		IncludeAttr=0;
		ExcludeAttr=FILE_ATTRIBUTE_DIRECTORY;
		RelativeDate=UseDate=UseSize=UseHardLinks=false;
	}
	else
	{
		MarkChar[1]=(wchar_t)FF->GetMarkChar();

		if (!MarkChar[1])
			*MarkChar=0;

		Name=FF->GetTitle();
		UseMask=FF->GetMask(&Mask);

		if (!FF->GetAttr(&IncludeAttr,&ExcludeAttr))
			IncludeAttr=ExcludeAttr=0;

		UseSize=FF->GetSize(nullptr,nullptr);
		UseDate=FF->GetDate(nullptr,nullptr,nullptr,&RelativeDate);
		UseHardLinks=FF->GetHardLinks(nullptr,nullptr);
	}

	wchar_t Attr[ARRAYSIZE(AttrC)*2] = {};

	for (size_t i=0; i<ARRAYSIZE(AttrF); i++)
	{
		wchar_t *Ptr=Attr+i*2;
		*Ptr=AttrC[i];

		if (IncludeAttr&AttrF[i])
			*(Ptr+1)=L'+';
		else if (ExcludeAttr&AttrF[i])
			*(Ptr+1)=L'-';
		else
			*Ptr=*(Ptr+1)=L'.';
	}

	wchar_t SizeDate[5] = L"....";

	if (UseSize)
	{
		SizeDate[0]=L'S';
	}

	if (UseDate)
	{
		if (RelativeDate)
			SizeDate[1]=L'R';
		else
			SizeDate[1]=L'D';
	}

	if (UseHardLinks)
	{
		SizeDate[2]=L'H';
	}

	if (bHighlightType)
	{
		if (FF->GetContinueProcessing())
			SizeDate[3]=DownArrow;

		strDest.Format(Format2, MarkChar, BoxSymbols[BS_V1], Attr, SizeDate, BoxSymbols[BS_V1], UseMask ? Mask : L"");
	}
	else
	{
		SizeDate[3]=0;

		if (!Hotkey && !bPanelType)
		{
			strDest.Format(wcschr(Name, L'&') ? Format1b : Format1a, Name, BoxSymbols[BS_V1], Attr, SizeDate, BoxSymbols[BS_V1], UseMask ? Mask : L"");
		}
		else
		{
			if (Hotkey)
				strDest.Format(Format1c, Hotkey, Name, BoxSymbols[BS_V1], Attr, SizeDate, BoxSymbols[BS_V1], UseMask ? Mask : L"");
			else
				strDest.Format(Format1d, Name, BoxSymbols[BS_V1], Attr, SizeDate, BoxSymbols[BS_V1], UseMask ? Mask : L"");
		}
	}

	RemoveTrailingSpaces(strDest);
}

enum enumFileFilterConfig
{
	ID_FF_TITLE,

	ID_FF_NAME,
	ID_FF_NAMEEDIT,

	ID_FF_SEPARATOR1,

	ID_FF_MATCHMASK,
	ID_FF_MASKEDIT,

	ID_FF_SEPARATOR2,

	ID_FF_MATCHSIZE,
	ID_FF_SIZEFROMSIGN,
	ID_FF_SIZEFROMEDIT,
	ID_FF_SIZETOSIGN,
	ID_FF_SIZETOEDIT,

	ID_FF_MATCHDATE,
	ID_FF_DATETYPE,
	ID_FF_DATERELATIVE,
	ID_FF_DATEBEFORESIGN,
	ID_FF_DATEBEFOREEDIT,
	ID_FF_DAYSBEFOREEDIT,
	ID_FF_TIMEBEFOREEDIT,
	ID_FF_DATEAFTERSIGN,
	ID_FF_DATEAFTEREDIT,
	ID_FF_DAYSAFTEREDIT,
	ID_FF_TIMEAFTEREDIT,
	ID_FF_CURRENT,
	ID_FF_BLANK,

	ID_FF_SEPARATOR3,
	ID_FF_SEPARATOR4,

	ID_FF_MATCHATTRIBUTES,
	ID_FF_READONLY,
	ID_FF_ARCHIVE,
	ID_FF_HIDDEN,
	ID_FF_SYSTEM,
	ID_FF_DIRECTORY,
	ID_FF_COMPRESSED,
	ID_FF_ENCRYPTED,
	ID_FF_NOTINDEXED,
	ID_FF_REPARSEPOINT,
	ID_FF_SPARSE,
	ID_FF_TEMP,
	ID_FF_OFFLINE,
	ID_FF_VIRTUAL,

	ID_HER_SEPARATOR1,
	ID_HER_MARK_TITLE,
	ID_HER_MARKEDIT,
	ID_HER_MARKTRANSPARENT,

	ID_HER_NORMALFILE,
	ID_HER_NORMALMARKING,
	ID_HER_SELECTEDFILE,
	ID_HER_SELECTEDMARKING,
	ID_HER_CURSORFILE,
	ID_HER_CURSORMARKING,
	ID_HER_SELECTEDCURSORFILE,
	ID_HER_SELECTEDCURSORMARKING,

	ID_HER_COLOREXAMPLE,
	ID_HER_CONTINUEPROCESSING,

	ID_FF_SEPARATOR5,

	ID_FF_HARDLINKS,

	ID_FF_SEPARATOR6,

	ID_FF_OK,
	ID_FF_RESET,
	ID_FF_CANCEL,
	ID_FF_MAKETRANSPARENT,
};

void HighlightDlgUpdateUserControl(FAR_CHAR_INFO *VBufColorExample,HighlightDataColor &Colors)
{
	const wchar_t *ptr;
	FarColor Color;
	const PaletteColors PalColor[] = {COL_PANELTEXT,COL_PANELSELECTEDTEXT,COL_PANELCURSOR,COL_PANELSELECTEDCURSOR};

	for (size_t i = 0; i < ARRAYSIZE(PalColor); ++i)
	{
		Color=Colors.Color[HIGHLIGHTCOLORTYPE_FILE][i];

		if (!COLORVALUE(Color.BackgroundColor) && !COLORVALUE(Color.ForegroundColor))
		{
			FARCOLORFLAGS ExFlags = Color.Flags&FCF_EXTENDEDFLAGS;
			Color=ColorIndexToColor(PalColor[i]);
			Color.Flags|=ExFlags;

		}

		if (Colors.MarkChar&0x0000FFFF)
			ptr=MSG(MHighlightExample2);
		else
			ptr=MSG(MHighlightExample1);

		for (int k=0; k<15; k++)
		{
			VBufColorExample[15*i+k].Char=ptr[k];
			VBufColorExample[15*i+k].Attributes=Color;
		}

		if (LOWORD(Colors.MarkChar))
		{
			// inherit only color mode, not style
			VBufColorExample[15*i+1].Attributes.Flags = Color.Flags&FCF_4BITMASK;
			VBufColorExample[15*i+1].Char=LOWORD(Colors.MarkChar);
			if (COLORVALUE(Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i].ForegroundColor) || COLORVALUE(Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i].BackgroundColor))
			{
				VBufColorExample[15*i+1].Attributes=Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i];
			}
			else
			{
				// apply all except color mode
				FARCOLORFLAGS ExFlags = Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i].Flags&FCF_EXTENDEDFLAGS;
				VBufColorExample[15*i+1].Attributes.Flags|=ExFlags;
			}
		}

		VBufColorExample[15*i].Attributes=ColorIndexToColor(COL_PANELBOX);
		VBufColorExample[15*i+14].Attributes=ColorIndexToColor(COL_PANELBOX);
	}
}

void FilterDlgRelativeDateItemsUpdate(HANDLE hDlg, bool bClear)
{
	SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

	if (SendDlgMessage(hDlg,DM_GETCHECK,ID_FF_DATERELATIVE,0))
	{
		SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DATEBEFOREEDIT,0);
		SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DATEAFTEREDIT,0);
		SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_CURRENT,0);
		SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DAYSBEFOREEDIT,ToPtr(1));
		SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DAYSAFTEREDIT,ToPtr(1));
	}
	else
	{
		SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DAYSBEFOREEDIT,0);
		SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DAYSAFTEREDIT,0);
		SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DATEBEFOREEDIT,ToPtr(1));
		SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DATEAFTEREDIT,ToPtr(1));
		SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_CURRENT,ToPtr(1));
	}

	if (bClear)
	{
		SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEAFTEREDIT,nullptr);
		SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DAYSAFTEREDIT,nullptr);
		SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEAFTEREDIT,nullptr);
		SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEBEFOREEDIT,nullptr);
		SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEBEFOREEDIT,nullptr);
		SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DAYSBEFOREEDIT,nullptr);
	}

	SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
}

intptr_t WINAPI FileFilterConfigDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	switch (Msg)
	{
		case DN_INITDIALOG:
		{
			FilterDlgRelativeDateItemsUpdate(hDlg, false);
			return TRUE;
		}
		case DN_BTNCLICK:
		{
			if (Param1==ID_FF_CURRENT || Param1==ID_FF_BLANK) //Current и Blank
			{
				FILETIME ft;
				string strDate, strTime;

				if (Param1==ID_FF_CURRENT)
				{
					GetSystemTimeAsFileTime(&ft);
					ConvertDate(ft,strDate,strTime,12,FALSE,FALSE,2);
				}
				else
				{
					strDate.Clear();
					strTime.Clear();
				}

				SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
				int relative = (int)SendDlgMessage(hDlg,DM_GETCHECK,ID_FF_DATERELATIVE,0);
				int db = relative ? ID_FF_DAYSBEFOREEDIT : ID_FF_DATEBEFOREEDIT;
				int da = relative ? ID_FF_DAYSAFTEREDIT  : ID_FF_DATEAFTEREDIT;
				SendDlgMessage(hDlg,DM_SETTEXTPTR,da,const_cast<wchar_t*>(strDate.CPtr()));
				SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEAFTEREDIT,const_cast<wchar_t*>(strTime.CPtr()));
				SendDlgMessage(hDlg,DM_SETTEXTPTR,db,const_cast<wchar_t*>(strDate.CPtr()));
				SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEBEFOREEDIT,const_cast<wchar_t*>(strTime.CPtr()));
				SendDlgMessage(hDlg,DM_SETFOCUS,da,0);
				COORD r;
				r.X=r.Y=0;
				SendDlgMessage(hDlg,DM_SETCURSORPOS,da,&r);
				SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
				break;
			}
			else if (Param1==ID_FF_RESET) // Reset
			{
				SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
				intptr_t ColorConfig = SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
				SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_MASKEDIT,const_cast<wchar_t*>(L"*"));
				SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_SIZEFROMEDIT,nullptr);
				SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_SIZETOEDIT,nullptr);

				for (int I=ID_FF_READONLY; I <= ID_FF_VIRTUAL; ++I)
				{
					SendDlgMessage(hDlg,DM_SETCHECK,I,ToPtr(BSTATE_3STATE));
				}

				if (!ColorConfig)
					SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_DIRECTORY,ToPtr(BSTATE_UNCHECKED));

				FarListPos LPos={sizeof(FarListPos)};
				SendDlgMessage(hDlg,DM_LISTSETCURPOS,ID_FF_DATETYPE,&LPos);
				SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHMASK,ToPtr(BSTATE_CHECKED));
				SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHSIZE,ToPtr(BSTATE_UNCHECKED));
				SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_HARDLINKS,ToPtr(BSTATE_UNCHECKED));
				SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHDATE,ToPtr(BSTATE_UNCHECKED));
				SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_DATERELATIVE,ToPtr(BSTATE_UNCHECKED));
				FilterDlgRelativeDateItemsUpdate(hDlg, true);
				SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHATTRIBUTES,ToPtr(ColorConfig?BSTATE_UNCHECKED:BSTATE_CHECKED));
				SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
				break;
			}
			else if (Param1==ID_FF_MAKETRANSPARENT)
			{
				HighlightDataColor *Colors = (HighlightDataColor *) SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);

				for (int i=0; i<2; i++)
					for (int j=0; j<4; j++)
					{
						MAKE_TRANSPARENT(Colors->Color[i][j].ForegroundColor);
						MAKE_TRANSPARENT(Colors->Color[i][j].BackgroundColor);
					}

				SendDlgMessage(hDlg,DM_SETCHECK,ID_HER_MARKTRANSPARENT,ToPtr(BSTATE_CHECKED));
				break;
			}
			else if (Param1==ID_FF_DATERELATIVE)
			{
				FilterDlgRelativeDateItemsUpdate(hDlg, true);
				break;
			}
		}
		case DN_CONTROLINPUT:

			if ((Msg==DN_BTNCLICK && Param1 >= ID_HER_NORMALFILE && Param1 <= ID_HER_SELECTEDCURSORMARKING)
			        || (Msg==DN_CONTROLINPUT && Param1==ID_HER_COLOREXAMPLE && ((INPUT_RECORD *)Param2)->EventType == MOUSE_EVENT && ((INPUT_RECORD *)Param2)->Event.MouseEvent.dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED))
			{
				HighlightDataColor *EditData = (HighlightDataColor *) SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);

				if (Msg==DN_CONTROLINPUT)
				{
					Param1 = ID_HER_NORMALFILE + ((INPUT_RECORD *)Param2)->Event.MouseEvent.dwMousePosition.Y*2;

					if (((INPUT_RECORD *)Param2)->Event.MouseEvent.dwMousePosition.X==1 && (EditData->MarkChar&0x0000FFFF))
						Param1 = ID_HER_NORMALMARKING + ((INPUT_RECORD *)Param2)->Event.MouseEvent.dwMousePosition.Y*2;
				}

				//Color[0=file, 1=mark][0=normal,1=selected,2=undercursor,3=selectedundercursor]
				FarColor Color=EditData->Color[(Param1-ID_HER_NORMALFILE)&1][(Param1-ID_HER_NORMALFILE)/2];
				Console.GetColorDialog(Color,true,true);
				EditData->Color[(Param1-ID_HER_NORMALFILE)&1][(Param1-ID_HER_NORMALFILE)/2]=Color;

				size_t Size = SendDlgMessage(hDlg,DM_GETDLGITEM,ID_HER_COLOREXAMPLE,0);
				FarGetDialogItem gdi = {sizeof(FarGetDialogItem), Size, static_cast<FarDialogItem*>(xf_malloc(Size))};
				SendDlgMessage(hDlg,DM_GETDLGITEM,ID_HER_COLOREXAMPLE,&gdi);
				//MarkChar это FIXEDIT размером в 1 символ
				wchar_t MarkChar[2];
				FarDialogItemData item={sizeof(FarDialogItemData),1,MarkChar};
				SendDlgMessage(hDlg,DM_GETTEXT,ID_HER_MARKEDIT,&item);
				EditData->MarkChar=*MarkChar;
				HighlightDlgUpdateUserControl(gdi.Item->VBuf,*EditData);
				SendDlgMessage(hDlg,DM_SETDLGITEM,ID_HER_COLOREXAMPLE,gdi.Item);
				xf_free(gdi.Item);
				return TRUE;
			}

			break;
		case DN_EDITCHANGE:

			if (Param1 == ID_HER_MARKEDIT)
			{
				HighlightDataColor *EditData = (HighlightDataColor *) SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
				size_t Size = SendDlgMessage(hDlg,DM_GETDLGITEM,ID_HER_COLOREXAMPLE,0);
				FarGetDialogItem gdi = {sizeof(FarGetDialogItem), Size, static_cast<FarDialogItem*>(xf_malloc(Size))};
				SendDlgMessage(hDlg,DM_GETDLGITEM,ID_HER_COLOREXAMPLE,&gdi);
				//MarkChar это FIXEDIT размером в 1 символ
				wchar_t MarkChar[2];
				FarDialogItemData item={sizeof(FarDialogItemData),1,MarkChar};
				SendDlgMessage(hDlg,DM_GETTEXT,ID_HER_MARKEDIT,&item);
				EditData->MarkChar=*MarkChar;
				HighlightDlgUpdateUserControl(gdi.Item->VBuf,*EditData);
				SendDlgMessage(hDlg,DM_SETDLGITEM,ID_HER_COLOREXAMPLE,gdi.Item);
				xf_free(gdi.Item);
				return TRUE;
			}

			break;
		case DN_CLOSE:

			if (Param1 == ID_FF_OK && SendDlgMessage(hDlg,DM_GETCHECK,ID_FF_MATCHSIZE,0))
			{
				string strTemp;
				FarDialogItemData item = {sizeof(FarDialogItemData)};
				item.PtrLength = SendDlgMessage(hDlg,DM_GETTEXT,ID_FF_SIZEFROMEDIT,0);
				item.PtrData = strTemp.GetBuffer(item.PtrLength+1);
				SendDlgMessage(hDlg,DM_GETTEXT,ID_FF_SIZEFROMEDIT,&item);
				bool bTemp = !*item.PtrData || CheckFileSizeStringFormat(item.PtrData);
				item.PtrLength = SendDlgMessage(hDlg,DM_GETTEXT,ID_FF_SIZETOEDIT,0);
				item.PtrData = strTemp.GetBuffer(item.PtrLength+1);
				SendDlgMessage(hDlg,DM_GETTEXT,ID_FF_SIZETOEDIT,&item);
				bTemp = bTemp && (!*item.PtrData || CheckFileSizeStringFormat(item.PtrData));

				if (!bTemp)
				{
					intptr_t ColorConfig = SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
					Message(MSG_WARNING,1,ColorConfig?MSG(MFileHilightTitle):MSG(MFileFilterTitle),MSG(MBadFileSizeFormat),MSG(MOk));
					return FALSE;
				}
			}

			break;
		default:
			break;
	}

	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool FileFilterConfig(FileFilterParams *FF, bool ColorConfig)
{
	const wchar_t VerticalLine[] = {BoxSymbols[BS_T_H1V1],BoxSymbols[BS_V1],BoxSymbols[BS_V1],BoxSymbols[BS_V1],BoxSymbols[BS_B_H1V1],0};
	// Временная маска.
	CFileMask FileMask;
	// История для маски файлов
	const wchar_t FilterMasksHistoryName[] = L"FilterMasks";
	// История для имени фильтра
	const wchar_t FilterNameHistoryName[] = L"FilterName";
	// Маски для диалога настройки
	// Маска для ввода дней для относительной даты
	const wchar_t DaysMask[] = L"9999";
	string strDateMask, strTimeMask;
	// Определение параметров даты и времени в системе.
	wchar_t DateSeparator=GetDateSeparator();
	wchar_t TimeSeparator=GetTimeSeparator();
	wchar_t DecimalSeparator=GetDecimalSeparator();
	int DateFormat=GetDateFormat();

	switch (DateFormat)
	{
		case 0:
			// Маска даты для форматов DD.MM.YYYYY и MM.DD.YYYYY
			strDateMask = FormatString() << L"99" << DateSeparator << "99" << DateSeparator << "9999N";
			break;
		case 1:
			// Маска даты для форматов DD.MM.YYYYY и MM.DD.YYYYY
			strDateMask = FormatString() << L"99" << DateSeparator << "99" << DateSeparator << "9999N";
			break;
		default:
			// Маска даты для формата YYYYY.MM.DD
			strDateMask = FormatString() << L"N9999" << DateSeparator << "c99" << DateSeparator << "c99";
			break;
	}

	// Маска времени
	strTimeMask = FormatString() << L"99" << TimeSeparator << "99" << TimeSeparator << "99" << DecimalSeparator << "999";
	FarDialogItem FilterDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,76,20,0,nullptr,nullptr,DIF_SHOWAMPERSAND,MSG(MFileFilterTitle)},

		{DI_TEXT,5,2,0,2,0,nullptr,nullptr,DIF_FOCUS,MSG(MFileFilterName)},
		{DI_EDIT,5,2,74,2,0,FilterNameHistoryName,nullptr,DIF_HISTORY,L""},

		{DI_TEXT,0,3,0,3,0,nullptr,nullptr,DIF_SEPARATOR,L""},

		{DI_CHECKBOX,5,4,0,4,0,nullptr,nullptr,DIF_AUTOMATION,MSG(MFileFilterMatchMask)},
		{DI_EDIT,5,4,74,4,0,FilterMasksHistoryName,nullptr,DIF_HISTORY,L""},

		{DI_TEXT,0,5,0,5,0,nullptr,nullptr,DIF_SEPARATOR,L""},

		{DI_CHECKBOX,5,6,0,6,0,nullptr,nullptr,DIF_AUTOMATION,MSG(MFileFilterSize)},
		{DI_TEXT,7,7,8,7,0,nullptr,nullptr,0,MSG(MFileFilterSizeFromSign)},
		{DI_EDIT,10,7,20,7,0,nullptr,nullptr,0,L""},
		{DI_TEXT,7,8,8,8,0,nullptr,nullptr,0,MSG(MFileFilterSizeToSign)},
		{DI_EDIT,10,8,20,8,0,nullptr,nullptr,0,L""},

		{DI_CHECKBOX,24,6,0,6,0,nullptr,nullptr,DIF_AUTOMATION,MSG(MFileFilterDate)},
		{DI_COMBOBOX,26,7,41,7,0,nullptr,nullptr,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,L""},
		{DI_CHECKBOX,26,8,0,8,0,nullptr,nullptr,0,MSG(MFileFilterDateRelative)},
		{DI_TEXT,48,7,50,7,0,nullptr,nullptr,0,MSG(MFileFilterDateBeforeSign)},
		{DI_FIXEDIT,51,7,61,7,0,nullptr,strDateMask.CPtr(),DIF_MASKEDIT,L""},
		{DI_FIXEDIT,51,7,61,7,0,nullptr,DaysMask,DIF_MASKEDIT,L""},
		{DI_FIXEDIT,63,7,74,7,0,nullptr,strTimeMask.CPtr(),DIF_MASKEDIT,L""},
		{DI_TEXT,48,8,50,8,0,nullptr,nullptr,0,MSG(MFileFilterDateAfterSign)},
		{DI_FIXEDIT,51,8,61,8,0,nullptr,strDateMask.CPtr(),DIF_MASKEDIT,L""},
		{DI_FIXEDIT,51,8,61,8,0,nullptr,DaysMask,DIF_MASKEDIT,L""},
		{DI_FIXEDIT,63,8,74,8,0,nullptr,strTimeMask.CPtr(),DIF_MASKEDIT,L""},
		{DI_BUTTON,0,6,0,6,0,nullptr,nullptr,DIF_BTNNOCLOSE,MSG(MFileFilterCurrent)},
		{DI_BUTTON,0,6,74,6,0,nullptr,nullptr,DIF_BTNNOCLOSE,MSG(MFileFilterBlank)},

		{DI_TEXT,0,9,0,9,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_VTEXT,22,5,22,9,0,nullptr,nullptr,DIF_BOXCOLOR,VerticalLine},

		{DI_CHECKBOX, 5,10,0,10,0,nullptr,nullptr,DIF_AUTOMATION,MSG(MFileFilterAttr)},
		{DI_CHECKBOX, 7,11,0,11,0,nullptr,nullptr,DIF_3STATE,MSG(MFileFilterAttrR)},
		{DI_CHECKBOX, 7,12,0,12,0,nullptr,nullptr,DIF_3STATE,MSG(MFileFilterAttrA)},
		{DI_CHECKBOX, 7,13,0,13,0,nullptr,nullptr,DIF_3STATE,MSG(MFileFilterAttrH)},
		{DI_CHECKBOX, 7,14,0,14,0,nullptr,nullptr,DIF_3STATE,MSG(MFileFilterAttrS)},

		{DI_CHECKBOX,29,11,0,11,0,nullptr,nullptr,DIF_3STATE,MSG(MFileFilterAttrD)},
		{DI_CHECKBOX,29,12,0,12,0,nullptr,nullptr,DIF_3STATE,MSG(MFileFilterAttrC)},
		{DI_CHECKBOX,29,13,0,13,0,nullptr,nullptr,DIF_3STATE,MSG(MFileFilterAttrE)},
		{DI_CHECKBOX,29,14,0,14,0,nullptr,nullptr,DIF_3STATE,MSG(MFileFilterAttrNI)},
		{DI_CHECKBOX,29,15,0,15,0,nullptr,nullptr,DIF_3STATE,MSG(MFileFilterAttrReparse)},

		{DI_CHECKBOX,51,11,0,11,0,nullptr,nullptr,DIF_3STATE,MSG(MFileFilterAttrSparse)},
		{DI_CHECKBOX,51,12,0,12,0,nullptr,nullptr,DIF_3STATE,MSG(MFileFilterAttrT)},
		{DI_CHECKBOX,51,13,0,13,0,nullptr,nullptr,DIF_3STATE,MSG(MFileFilterAttrOffline)},
		{DI_CHECKBOX,51,14,0,14,0,nullptr,nullptr,DIF_3STATE,MSG(MFileFilterAttrVirtual)},

		{DI_TEXT,-1,14,0,14,0,nullptr,nullptr,DIF_SEPARATOR,MSG(MHighlightColors)},
		{DI_TEXT,7,15,0,15,0,nullptr,nullptr,0,MSG(MHighlightMarkChar)},
		{DI_FIXEDIT,5,15,5,15,0,nullptr,nullptr,0,L""},
		{DI_CHECKBOX,0,15,0,15,0,nullptr,nullptr,0,MSG(MHighlightTransparentMarkChar)},

		{DI_BUTTON,5,16,0,16,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,MSG(MHighlightFileName1)},
		{DI_BUTTON,0,16,0,16,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,MSG(MHighlightMarking1)},
		{DI_BUTTON,5,17,0,17,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,MSG(MHighlightFileName2)},
		{DI_BUTTON,0,17,0,17,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,MSG(MHighlightMarking2)},
		{DI_BUTTON,5,18,0,18,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,MSG(MHighlightFileName3)},
		{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,MSG(MHighlightMarking3)},
		{DI_BUTTON,5,19,0,19,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,MSG(MHighlightFileName4)},
		{DI_BUTTON,0,19,0,19,0,nullptr,nullptr,DIF_BTNNOCLOSE|DIF_NOBRACKETS,MSG(MHighlightMarking4)},

		{DI_USERCONTROL,73-15-1,16,73-2,19,0,nullptr,nullptr,DIF_NOFOCUS,L""},
		{DI_CHECKBOX,5,20,0,20,0,nullptr,nullptr,0,MSG(MHighlightContinueProcessing)},

		{DI_TEXT,0,16,0,16,0,nullptr,nullptr,DIF_SEPARATOR,L""},

		{DI_CHECKBOX,5,17,0,17,0,nullptr,nullptr,0,MSG(MFileHardLinksCount)},//добавляем новый чекбокс в панель
		{DI_TEXT,0,18,0,18,0,nullptr,nullptr,DIF_SEPARATOR,L""},// и разделитель

		{DI_BUTTON,0,19,0,19,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,0,19,0,19,0,nullptr,nullptr,DIF_CENTERGROUP|DIF_BTNNOCLOSE,MSG(MFileFilterReset)},
		{DI_BUTTON,0,19,0,19,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MFileFilterCancel)},
		{DI_BUTTON,0,19,0,19,0,nullptr,nullptr,DIF_CENTERGROUP|DIF_BTNNOCLOSE,MSG(MFileFilterMakeTransparent)},
	};
	FilterDlgData[0].Data=MSG(ColorConfig?MFileHilightTitle:MFileFilterTitle);
	MakeDialogItemsEx(FilterDlgData,FilterDlg);

	if (ColorConfig)
	{
		FilterDlg[ID_FF_TITLE].Y2+=5;

		for (int i=ID_FF_NAME; i<=ID_FF_SEPARATOR1; i++)
			FilterDlg[i].Flags|=DIF_HIDDEN;

		for (int i=ID_FF_MATCHMASK; i<=ID_FF_VIRTUAL; i++)
		{
			FilterDlg[i].Y1-=2;
			FilterDlg[i].Y2-=2;
		}

		for (int i=ID_FF_SEPARATOR5; i<=ID_FF_MAKETRANSPARENT; i++)
		{
			FilterDlg[i].Y1+=5;
			FilterDlg[i].Y2+=5;
		}
	}
	else
	{
		for (int i=ID_HER_SEPARATOR1; i<=ID_HER_CONTINUEPROCESSING; i++)
			FilterDlg[i].Flags|=DIF_HIDDEN;

		FilterDlg[ID_FF_MAKETRANSPARENT].Flags=DIF_HIDDEN;
	}

	FilterDlg[ID_FF_NAMEEDIT].X1=FilterDlg[ID_FF_NAME].X1+(int)FilterDlg[ID_FF_NAME].strData.GetLength()-(FilterDlg[ID_FF_NAME].strData.Contains(L'&')?1:0)+1;
	FilterDlg[ID_FF_MASKEDIT].X1=FilterDlg[ID_FF_MATCHMASK].X1+(int)FilterDlg[ID_FF_MATCHMASK].strData.GetLength()-(FilterDlg[ID_FF_MATCHMASK].strData.Contains(L'&')?1:0)+5;
	FilterDlg[ID_FF_BLANK].X1=FilterDlg[ID_FF_BLANK].X2-(int)FilterDlg[ID_FF_BLANK].strData.GetLength()+(FilterDlg[ID_FF_BLANK].strData.Contains(L'&')?1:0)-3;
	FilterDlg[ID_FF_CURRENT].X2=FilterDlg[ID_FF_BLANK].X1-2;
	FilterDlg[ID_FF_CURRENT].X1=FilterDlg[ID_FF_CURRENT].X2-(int)FilterDlg[ID_FF_CURRENT].strData.GetLength()+(FilterDlg[ID_FF_CURRENT].strData.Contains(L'&')?1:0)-3;
	FilterDlg[ID_HER_MARKTRANSPARENT].X1=FilterDlg[ID_HER_MARK_TITLE].X1+(int)FilterDlg[ID_HER_MARK_TITLE].strData.GetLength()-(FilterDlg[ID_HER_MARK_TITLE].strData.Contains(L'&')?1:0)+1;

	for (int i=ID_HER_NORMALMARKING; i<=ID_HER_SELECTEDCURSORMARKING; i+=2)
		FilterDlg[i].X1=FilterDlg[ID_HER_NORMALFILE].X1+(int)FilterDlg[ID_HER_NORMALFILE].strData.GetLength()-(FilterDlg[ID_HER_NORMALFILE].strData.Contains(L'&')?1:0)+1;

	FAR_CHAR_INFO VBufColorExample[15*4]={};
	HighlightDataColor Colors;
	FF->GetColors(&Colors);
	HighlightDlgUpdateUserControl(VBufColorExample,Colors);
	FilterDlg[ID_HER_COLOREXAMPLE].VBuf=VBufColorExample;
	wchar_t MarkChar[] = {static_cast<wchar_t>(Colors.MarkChar), 0};
	FilterDlg[ID_HER_MARKEDIT].strData=MarkChar;
	FilterDlg[ID_HER_MARKTRANSPARENT].Selected=(Colors.MarkChar&0xFF0000?1:0);
	FilterDlg[ID_HER_CONTINUEPROCESSING].Selected=(FF->GetContinueProcessing()?1:0);
	FilterDlg[ID_FF_NAMEEDIT].strData = FF->GetTitle();
	const wchar_t *FMask;
	FilterDlg[ID_FF_MATCHMASK].Selected=FF->GetMask(&FMask)?1:0;
	FilterDlg[ID_FF_MASKEDIT].strData=FMask;

	if (!FilterDlg[ID_FF_MATCHMASK].Selected)
		FilterDlg[ID_FF_MASKEDIT].Flags|=DIF_DISABLE;

	const wchar_t *SizeAbove, *SizeBelow;
	FilterDlg[ID_FF_MATCHSIZE].Selected=FF->GetSize(&SizeAbove,&SizeBelow)?1:0;
	FilterDlg[ID_FF_SIZEFROMEDIT].strData=SizeAbove;
	FilterDlg[ID_FF_SIZETOEDIT].strData=SizeBelow;
	FilterDlg[ID_FF_HARDLINKS].Selected=FF->GetHardLinks(nullptr,nullptr)?1:0; //пока что мы проверям только флаг использования данного условия

	if (!FilterDlg[ID_FF_MATCHSIZE].Selected)
		for (int i=ID_FF_SIZEFROMSIGN; i <= ID_FF_SIZETOEDIT; i++)
			FilterDlg[i].Flags|=DIF_DISABLE;

	// Лист для комбобокса времени файла
	FarList DateList;
	FarListItem TableItemDate[FDATE_COUNT]={};
	// Настройка списка типов дат файла
	DateList.Items=TableItemDate;
	DateList.ItemsNumber=FDATE_COUNT;

	for (int i=0; i < FDATE_COUNT; ++i)
		TableItemDate[i].Text=MSG(MFileFilterWrited+i);

	DWORD DateType;
	FILETIME DateAfter, DateBefore;
	bool bRelative;
	FilterDlg[ID_FF_MATCHDATE].Selected=FF->GetDate(&DateType,&DateAfter,&DateBefore,&bRelative)?1:0;
	FilterDlg[ID_FF_DATERELATIVE].Selected=bRelative?1:0;
	FilterDlg[ID_FF_DATETYPE].ListItems=&DateList;
	TableItemDate[DateType].Flags=LIF_SELECTED;

	if (bRelative)
	{
		ConvertRelativeDate(DateAfter, FilterDlg[ID_FF_DAYSAFTEREDIT].strData, FilterDlg[ID_FF_TIMEAFTEREDIT].strData);
		ConvertRelativeDate(DateBefore, FilterDlg[ID_FF_DAYSBEFOREEDIT].strData, FilterDlg[ID_FF_TIMEBEFOREEDIT].strData);
	}
	else
	{
		ConvertDate(DateAfter,FilterDlg[ID_FF_DATEAFTEREDIT].strData,FilterDlg[ID_FF_TIMEAFTEREDIT].strData,12,FALSE,FALSE,2);
		ConvertDate(DateBefore,FilterDlg[ID_FF_DATEBEFOREEDIT].strData,FilterDlg[ID_FF_TIMEBEFOREEDIT].strData,12,FALSE,FALSE,2);
	}

	if (!FilterDlg[ID_FF_MATCHDATE].Selected)
		for (int i=ID_FF_DATETYPE; i <= ID_FF_BLANK; i++)
			FilterDlg[i].Flags|=DIF_DISABLE;

	DWORD AttrSet, AttrClear;
	FilterDlg[ID_FF_MATCHATTRIBUTES].Selected=FF->GetAttr(&AttrSet,&AttrClear)?1:0;
	FilterDlg[ID_FF_READONLY].Selected=(AttrSet & FILE_ATTRIBUTE_READONLY?1:AttrClear & FILE_ATTRIBUTE_READONLY?0:2);
	FilterDlg[ID_FF_ARCHIVE].Selected=(AttrSet & FILE_ATTRIBUTE_ARCHIVE?1:AttrClear & FILE_ATTRIBUTE_ARCHIVE?0:2);
	FilterDlg[ID_FF_HIDDEN].Selected=(AttrSet & FILE_ATTRIBUTE_HIDDEN?1:AttrClear & FILE_ATTRIBUTE_HIDDEN?0:2);
	FilterDlg[ID_FF_SYSTEM].Selected=(AttrSet & FILE_ATTRIBUTE_SYSTEM?1:AttrClear & FILE_ATTRIBUTE_SYSTEM?0:2);
	FilterDlg[ID_FF_COMPRESSED].Selected=(AttrSet & FILE_ATTRIBUTE_COMPRESSED?1:AttrClear & FILE_ATTRIBUTE_COMPRESSED?0:2);
	FilterDlg[ID_FF_ENCRYPTED].Selected=(AttrSet & FILE_ATTRIBUTE_ENCRYPTED?1:AttrClear & FILE_ATTRIBUTE_ENCRYPTED?0:2);
	FilterDlg[ID_FF_DIRECTORY].Selected=(AttrSet & FILE_ATTRIBUTE_DIRECTORY?1:AttrClear & FILE_ATTRIBUTE_DIRECTORY?0:2);
	FilterDlg[ID_FF_NOTINDEXED].Selected=(AttrSet & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED?1:AttrClear & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED?0:2);
	FilterDlg[ID_FF_SPARSE].Selected=(AttrSet & FILE_ATTRIBUTE_SPARSE_FILE?1:AttrClear & FILE_ATTRIBUTE_SPARSE_FILE?0:2);
	FilterDlg[ID_FF_TEMP].Selected=(AttrSet & FILE_ATTRIBUTE_TEMPORARY?1:AttrClear & FILE_ATTRIBUTE_TEMPORARY?0:2);
	FilterDlg[ID_FF_REPARSEPOINT].Selected=(AttrSet & FILE_ATTRIBUTE_REPARSE_POINT?1:AttrClear & FILE_ATTRIBUTE_REPARSE_POINT?0:2);
	FilterDlg[ID_FF_OFFLINE].Selected=(AttrSet & FILE_ATTRIBUTE_OFFLINE?1:AttrClear & FILE_ATTRIBUTE_OFFLINE?0:2);
	FilterDlg[ID_FF_VIRTUAL].Selected=(AttrSet & FILE_ATTRIBUTE_VIRTUAL?1:AttrClear & FILE_ATTRIBUTE_VIRTUAL?0:2);

	if (!FilterDlg[ID_FF_MATCHATTRIBUTES].Selected)
	{
		for (int i=ID_FF_READONLY; i <= ID_FF_VIRTUAL; i++)
			FilterDlg[i].Flags|=DIF_DISABLE;
	}

	Dialog Dlg(FilterDlg,ARRAYSIZE(FilterDlg),FileFilterConfigDlgProc,ColorConfig?&Colors:nullptr);
	Dlg.SetHelp(ColorConfig?L"HighlightEdit":L"Filter");
	Dlg.SetPosition(-1,-1,FilterDlg[ID_FF_TITLE].X2+4,FilterDlg[ID_FF_TITLE].Y2+2);
	Dlg.SetAutomation(ID_FF_MATCHMASK,ID_FF_MASKEDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEFROMSIGN,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEFROMEDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZETOSIGN,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZETOEDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATETYPE,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATERELATIVE,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEAFTERSIGN,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEAFTEREDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DAYSAFTEREDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_TIMEAFTEREDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEBEFORESIGN,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEBEFOREEDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DAYSBEFOREEDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_TIMEBEFOREEDIT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_CURRENT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_BLANK,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_READONLY,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_ARCHIVE,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_HIDDEN,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_SYSTEM,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_COMPRESSED,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_ENCRYPTED,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_NOTINDEXED,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_SPARSE,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_TEMP,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_REPARSEPOINT,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_OFFLINE,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_VIRTUAL,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_DIRECTORY,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);

	for (;;)
	{
		Dlg.ClearDone();
		Dlg.Process();
		int ExitCode=Dlg.GetExitCode();

		if (ExitCode==ID_FF_OK) // Ok
		{
			// Если введённая пользователем маска не корректна, тогда вернёмся в диалог
			if (FilterDlg[ID_FF_MATCHMASK].Selected && !FileMask.Set(FilterDlg[ID_FF_MASKEDIT].strData,0))
				continue;

			if (FilterDlg[ID_HER_MARKTRANSPARENT].Selected)
				Colors.MarkChar|=0x00FF0000;
			else
				Colors.MarkChar&=0x0000FFFF;

			FF->SetColors(&Colors);
			FF->SetContinueProcessing(FilterDlg[ID_HER_CONTINUEPROCESSING].Selected!=0);
			FF->SetTitle(FilterDlg[ID_FF_NAMEEDIT].strData);
			FF->SetMask(FilterDlg[ID_FF_MATCHMASK].Selected!=0,
			            FilterDlg[ID_FF_MASKEDIT].strData);
			FF->SetSize(FilterDlg[ID_FF_MATCHSIZE].Selected!=0,
			            FilterDlg[ID_FF_SIZEFROMEDIT].strData,
			            FilterDlg[ID_FF_SIZETOEDIT].strData);
			FF->SetHardLinks(FilterDlg[ID_FF_HARDLINKS].Selected!=0,0,0); //пока устанавливаем только флаг использования признака
			bRelative = FilterDlg[ID_FF_DATERELATIVE].Selected!=0;

			LPWSTR TimeBefore = FilterDlg[ID_FF_TIMEBEFOREEDIT].strData.GetBuffer();
			TimeBefore[8] = TimeSeparator;
			FilterDlg[ID_FF_TIMEBEFOREEDIT].strData.ReleaseBuffer(FilterDlg[ID_FF_TIMEBEFOREEDIT].strData.GetLength());

			LPWSTR TimeAfter = FilterDlg[ID_FF_TIMEAFTEREDIT].strData.GetBuffer();
			TimeAfter[8] = TimeSeparator;
			FilterDlg[ID_FF_TIMEAFTEREDIT].strData.ReleaseBuffer(FilterDlg[ID_FF_TIMEAFTEREDIT].strData.GetLength());

			StrToDateTime(FilterDlg[bRelative?ID_FF_DAYSAFTEREDIT:ID_FF_DATEAFTEREDIT].strData,FilterDlg[ID_FF_TIMEAFTEREDIT].strData,DateAfter,DateFormat,DateSeparator,TimeSeparator,bRelative);
			StrToDateTime(FilterDlg[bRelative?ID_FF_DAYSBEFOREEDIT:ID_FF_DATEBEFOREEDIT].strData,FilterDlg[ID_FF_TIMEBEFOREEDIT].strData,DateBefore,DateFormat,DateSeparator,TimeSeparator,bRelative);
			FF->SetDate(FilterDlg[ID_FF_MATCHDATE].Selected!=0,
			            FilterDlg[ID_FF_DATETYPE].ListPos,
			            DateAfter,
			            DateBefore,
			            bRelative);
			AttrSet=0;
			AttrClear=0;
			AttrSet|=(FilterDlg[ID_FF_READONLY].Selected==1?FILE_ATTRIBUTE_READONLY:0);
			AttrSet|=(FilterDlg[ID_FF_ARCHIVE].Selected==1?FILE_ATTRIBUTE_ARCHIVE:0);
			AttrSet|=(FilterDlg[ID_FF_HIDDEN].Selected==1?FILE_ATTRIBUTE_HIDDEN:0);
			AttrSet|=(FilterDlg[ID_FF_SYSTEM].Selected==1?FILE_ATTRIBUTE_SYSTEM:0);
			AttrSet|=(FilterDlg[ID_FF_COMPRESSED].Selected==1?FILE_ATTRIBUTE_COMPRESSED:0);
			AttrSet|=(FilterDlg[ID_FF_ENCRYPTED].Selected==1?FILE_ATTRIBUTE_ENCRYPTED:0);
			AttrSet|=(FilterDlg[ID_FF_DIRECTORY].Selected==1?FILE_ATTRIBUTE_DIRECTORY:0);
			AttrSet|=(FilterDlg[ID_FF_NOTINDEXED].Selected==1?FILE_ATTRIBUTE_NOT_CONTENT_INDEXED:0);
			AttrSet|=(FilterDlg[ID_FF_SPARSE].Selected==1?FILE_ATTRIBUTE_SPARSE_FILE:0);
			AttrSet|=(FilterDlg[ID_FF_TEMP].Selected==1?FILE_ATTRIBUTE_TEMPORARY:0);
			AttrSet|=(FilterDlg[ID_FF_REPARSEPOINT].Selected==1?FILE_ATTRIBUTE_REPARSE_POINT:0);
			AttrSet|=(FilterDlg[ID_FF_OFFLINE].Selected==1?FILE_ATTRIBUTE_OFFLINE:0);
			AttrSet|=(FilterDlg[ID_FF_VIRTUAL].Selected==1?FILE_ATTRIBUTE_VIRTUAL:0);
			AttrClear|=(FilterDlg[ID_FF_READONLY].Selected==0?FILE_ATTRIBUTE_READONLY:0);
			AttrClear|=(FilterDlg[ID_FF_ARCHIVE].Selected==0?FILE_ATTRIBUTE_ARCHIVE:0);
			AttrClear|=(FilterDlg[ID_FF_HIDDEN].Selected==0?FILE_ATTRIBUTE_HIDDEN:0);
			AttrClear|=(FilterDlg[ID_FF_SYSTEM].Selected==0?FILE_ATTRIBUTE_SYSTEM:0);
			AttrClear|=(FilterDlg[ID_FF_COMPRESSED].Selected==0?FILE_ATTRIBUTE_COMPRESSED:0);
			AttrClear|=(FilterDlg[ID_FF_ENCRYPTED].Selected==0?FILE_ATTRIBUTE_ENCRYPTED:0);
			AttrClear|=(FilterDlg[ID_FF_DIRECTORY].Selected==0?FILE_ATTRIBUTE_DIRECTORY:0);
			AttrClear|=(FilterDlg[ID_FF_NOTINDEXED].Selected==0?FILE_ATTRIBUTE_NOT_CONTENT_INDEXED:0);
			AttrClear|=(FilterDlg[ID_FF_SPARSE].Selected==0?FILE_ATTRIBUTE_SPARSE_FILE:0);
			AttrClear|=(FilterDlg[ID_FF_TEMP].Selected==0?FILE_ATTRIBUTE_TEMPORARY:0);
			AttrClear|=(FilterDlg[ID_FF_REPARSEPOINT].Selected==0?FILE_ATTRIBUTE_REPARSE_POINT:0);
			AttrClear|=(FilterDlg[ID_FF_OFFLINE].Selected==0?FILE_ATTRIBUTE_OFFLINE:0);
			AttrClear|=(FilterDlg[ID_FF_VIRTUAL].Selected==0?FILE_ATTRIBUTE_VIRTUAL:0);
			FF->SetAttr(FilterDlg[ID_FF_MATCHATTRIBUTES].Selected!=0,
			            AttrSet,
			            AttrClear);
			return true;
		}
		else
			break;
	}

	return false;
}
