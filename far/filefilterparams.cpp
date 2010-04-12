/*
filefilterparams.cpp

Параметры Файлового фильтра

*/

#include "headers.hpp"
#pragma hdrstop

#include "colors.hpp"
#include "CFileMask.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "filelist.hpp"
#include "filefilterparams.hpp"

FileFilterParams::FileFilterParams()
{
	*m_Title=0;
	SetMask(1,"*");
	SetSize(0,"","");
	memset(&FDate,0,sizeof(FDate));
	memset(&FAttr,0,sizeof(FAttr));
	memset(&FHighlight.Colors,0,sizeof(FHighlight.Colors));
	FHighlight.SortGroup=DEFAULT_SORT_GROUP;
	FHighlight.bContinueProcessing=false;
	ClearAllFlags();
}

const FileFilterParams &FileFilterParams::operator=(const FileFilterParams &FF)
{
	SetTitle(FF.GetTitle());
	const char *Mask;
	FF.GetMask(&Mask);
	SetMask(FF.GetMask(NULL),Mask);
	memcpy(&FSize,&FF.FSize,sizeof(FSize));
	memcpy(&FDate,&FF.FDate,sizeof(FDate));
	memcpy(&FAttr,&FF.FAttr,sizeof(FAttr));
	FF.GetColors(&FHighlight.Colors);
	FHighlight.SortGroup=FF.GetSortGroup();
	FHighlight.bContinueProcessing=FF.GetContinueProcessing();
	memcpy(FFlags,FF.FFlags,sizeof(FFlags));
	return *this;
}

void FileFilterParams::SetTitle(const char *Title)
{
	xstrncpy(m_Title,Title,sizeof(m_Title)-1);
}

void FileFilterParams::SetMask(bool Used, const char *Mask)
{
	FMask.Used = Used;
	xstrncpy(FMask.Mask,Mask,sizeof(FMask.Mask)-1);
	/* Обработка %PATHEXT% */
	char CheckMask[FILEFILTER_MASK_SIZE];
	xstrncpy(CheckMask,Mask,sizeof(CheckMask)-1);
	char *Ptr;

	// проверим
	if ((Ptr=strchr(CheckMask,'%')) != NULL && !LocalStrnicmp(Ptr,"%PATHEXT%",9))
	{
		int IQ1=(*(Ptr+9) == ',')?10:9, offsetPtr=(int)(Ptr-CheckMask);
		// Если встречается %pathext%, то допишем в конец...
		memmove(Ptr,Ptr+IQ1,strlen(Ptr+IQ1)+1);
		char Tmp1[FILEFILTER_MASK_SIZE], *pSeparator;
		xstrncpy(Tmp1, CheckMask, sizeof(Tmp1)-1);
		pSeparator=strchr(Tmp1, EXCLUDEMASKSEPARATOR);

		if (pSeparator)
		{
			Ptr=Tmp1+offsetPtr;

			if (Ptr>pSeparator) // PATHEXT находится в масках исключения
				Add_PATHEXT(CheckMask); // добавляем то, чего нету.
			else
			{
				char Tmp2[FILEFILTER_MASK_SIZE];
				xstrncpy(Tmp2, pSeparator+1,sizeof(Tmp2)-1);
				*pSeparator=0;
				Add_PATHEXT(Tmp1);
				sprintf(CheckMask, "%s|%s", Tmp1, Tmp2);
			}
		}
		else
			Add_PATHEXT(CheckMask); // добавляем то, чего нету.
	}

	// Проверка на валидность текущих настроек фильтра
	if ((*CheckMask==0) || (!FMask.FilterMask.Set(CheckMask,FMF_SILENT)))
	{
		xstrncpy(FMask.Mask,"*",sizeof(FMask.Mask)-1);
		FMask.FilterMask.Set(FMask.Mask,FMF_SILENT);
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

void FileFilterParams::SetSize(bool Used, const char *SizeAbove, const char *SizeBelow)
{
	FSize.Used=Used;
	xstrncpy(FSize.SizeAbove,SizeAbove,sizeof(FSize.SizeAbove)-1);
	xstrncpy(FSize.SizeBelow,SizeBelow,sizeof(FSize.SizeBelow)-1);
	FSize.SizeAboveReal=ConvertFileSizeString(FSize.SizeAbove);
	FSize.SizeBelowReal=ConvertFileSizeString(FSize.SizeBelow);
}

void FileFilterParams::SetAttr(bool Used, DWORD AttrSet, DWORD AttrClear)
{
	FAttr.Used=Used;
	FAttr.AttrSet=AttrSet;
	FAttr.AttrClear=AttrClear;
}

void FileFilterParams::SetColors(HighlightDataColor *Colors)
{
	memcpy(&FHighlight.Colors,Colors,sizeof(FHighlight.Colors));
}

const char *FileFilterParams::GetTitle() const
{
	return m_Title;
}

bool FileFilterParams::GetMask(const char **Mask) const
{
	if (Mask)
		*Mask=FMask.Mask;

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

bool FileFilterParams::GetSize(const char **SizeAbove, const char **SizeBelow) const
{
	if (SizeAbove)
		*SizeAbove=FSize.SizeAbove;

	if (SizeBelow)
		*SizeBelow=FSize.SizeBelow;

	return FSize.Used;
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
	memcpy(Colors,&FHighlight.Colors,sizeof(*Colors));
}

int FileFilterParams::GetMarkChar() const
{
	return FHighlight.Colors.MarkChar;
}

bool FileFilterParams::FileInFilter(FileListItem *fli, unsigned __int64 CurrentTime)
{
	WIN32_FIND_DATA fd;
	fd.dwFileAttributes=fli->FileAttr;
	fd.ftCreationTime=fli->CreationTime;
	fd.ftLastAccessTime=fli->AccessTime;
	fd.ftLastWriteTime=fli->WriteTime;
	fd.nFileSizeHigh=fli->UnpSizeHigh;
	fd.nFileSizeLow=fli->UnpSize;
	fd.dwReserved0=fli->PackSizeHigh;
	fd.dwReserved1=fli->PackSize;
	xstrncpy(fd.cFileName,fli->Name,sizeof(fd.cFileName)-1);
	xstrncpy(fd.cAlternateFileName,fli->ShortName,sizeof(fd.cAlternateFileName)-1);
	return FileInFilter(&fd, CurrentTime);
}

bool FileFilterParams::FileInFilter(WIN32_FIND_DATA *fd, unsigned __int64 CurrentTime)
{
	// Пустое значение?
	//if (fd==NULL)
	//return false;

	// Режим проверки атрибутов файла включен?
	if (FAttr.Used)
	{
		// Проверка попадания файла по установленным атрибутам
		if ((fd->dwFileAttributes & FAttr.AttrSet) != FAttr.AttrSet)
			return false;

		// Проверка попадания файла по отсутствующим атрибутам
		if (fd->dwFileAttributes & FAttr.AttrClear)
			return false;
	}

	// Режим проверки размера файла включен?
	if (FSize.Used)
	{
		// Преобразуем размер из двух DWORD в беззнаковый __int64
		unsigned __int64 fsize=(unsigned __int64)fd->nFileSizeLow|((unsigned __int64)fd->nFileSizeHigh<<32);

		if (*FSize.SizeAbove)
		{
			if (fsize < FSize.SizeAboveReal)      // Размер файла меньше минимального разрешённого по фильтру?
				return false;                       // Не пропускаем этот файл
		}

		if (*FSize.SizeBelow)
		{
			if (fsize > FSize.SizeBelowReal)      // Размер файла больше максимального разрешённого по фильтру?
				return false;                       // Не пропускаем этот файл
		}
	}

	// Режим проверки времени файла включен?
	if (FDate.Used)
	{
		// Преобразуем FILETIME в беззнаковый __int64
		unsigned __int64 after  = FDate.DateAfter.QuadPart;
		unsigned __int64 before = FDate.DateBefore.QuadPart;

		if (after!=_ui64(0) || before!=_ui64(0))
		{
			FILETIME *ft;

			switch (FDate.DateType)
			{
				case FDATE_CREATED:
					ft=&fd->ftCreationTime;
					break;
				case FDATE_OPENED:
					ft=&fd->ftLastAccessTime;
					break;
				default: //case FDATE_MODIFIED:
					ft=&fd->ftLastWriteTime;
			}

			ULARGE_INTEGER ftime;
			ftime.u.LowPart  = ft->dwLowDateTime;
			ftime.u.HighPart = ft->dwHighDateTime;

			if (FDate.bRelative)
			{
				if (after!=_ui64(0))
					after = CurrentTime - after;

				if (before!=_ui64(0))
					before = CurrentTime - before;
			}

			// Есть введённая пользователем начальная дата?
			if (after!=_ui64(0))
			{
				// Дата файла меньше начальной даты по фильтру?
				if (ftime.QuadPart<after)
					// Не пропускаем этот файл
					return false;
			}

			// Есть введённая пользователем конечная дата?
			if (before!=_ui64(0))
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
		if (!FMask.FilterMask.Compare(fd->cFileName))
			// Не пропускаем этот файл
			return false;
	}

	// Да! Файл выдержал все испытания и будет допущен к использованию
	// в вызвавшей эту функцию операции.
	return true;
}

//Централизованная функция для создания строк меню различных фильтров.
void MenuString(char *dest, FileFilterParams *FF, bool bHighlightType, int Hotkey, bool bPanelType, const char *FMask, const char *Title)
{
	const char  AttrC[] = "RAHSDCEI$TLOV";
	const DWORD AttrF[] =
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
	const unsigned char VerticalLine=0x0B3;
	const unsigned char DownArrow=0x019;
	const char Format1a[] = "%-21.21s %c %-26.26s %-2.2s %c %-60.60s";
	const char Format1b[] = "%-22.22s %c %-26.26s %-2.2s %c %-60.60s";
	const char Format1c[] = "&%c. %-18.18s %c %-26.26s %-2.2s %c %-60.60s";
	const char Format1d[] = "   %-18.18s %c %-26.26s %-2.2s %c %-60.60s";
	const char Format2[] = "%-3.3s %c %-26.26s %-3.3s %c %-77.77s";
	const char *Name, *Mask;
	char MarkChar[]="\" \"";
	DWORD IncludeAttr, ExcludeAttr;
	bool UseMask, UseSize, UseDate, RelativeDate;

	if (bPanelType)
	{
		Name=Title;
		UseMask=true;
		Mask=FMask;
		IncludeAttr=0;
		ExcludeAttr=FILE_ATTRIBUTE_DIRECTORY;
		RelativeDate=UseDate=UseSize=false;
	}
	else
	{
		MarkChar[1]=(char)FF->GetMarkChar();

		if (MarkChar[1]==0)
			*MarkChar=0;

		Name=FF->GetTitle();
		UseMask=FF->GetMask(&Mask);

		if (!FF->GetAttr(&IncludeAttr,&ExcludeAttr))
			IncludeAttr=ExcludeAttr=0;

		UseSize=FF->GetSize(NULL,NULL);
		UseDate=FF->GetDate(NULL,NULL,NULL,&RelativeDate);
	}

	char Attr[sizeof(AttrC)*2] = {0};

	for (int i=0; i<sizeof(AttrF)/sizeof(AttrF[0]); i++)
	{
		char *Ptr=Attr+i*2;
		*Ptr=AttrC[i];

		if (IncludeAttr&AttrF[i])
			*(Ptr+1)='+';
		else if (ExcludeAttr&AttrF[i])
			*(Ptr+1)='-';
		else
			*Ptr=*(Ptr+1)='.';
	}

	char SizeDate[4] = "...";

	if (UseSize)
	{
		SizeDate[0]='S';
	}

	if (UseDate)
	{
		if (RelativeDate)
			SizeDate[1]='R';
		else
			SizeDate[1]='D';
	}

	if (bHighlightType)
	{
		if (FF->GetContinueProcessing())
			SizeDate[2]=DownArrow;

		sprintf(dest, Format2, MarkChar, VerticalLine, Attr, SizeDate, VerticalLine, UseMask ? Mask : "");
	}
	else
	{
		SizeDate[2]=0;

		if (!Hotkey && !bPanelType)
		{
			sprintf(dest, strstr(Name, "&") ? Format1b : Format1a, Name, VerticalLine, Attr, SizeDate, VerticalLine, UseMask ? Mask : "");
		}
		else
		{
			if (Hotkey)
				sprintf(dest, Format1c, Hotkey, Name, VerticalLine, Attr, SizeDate, VerticalLine, UseMask ? Mask : "");
			else
				sprintf(dest, Format1d, Name, VerticalLine, Attr, SizeDate, VerticalLine, UseMask ? Mask : "");
		}
	}

	RemoveTrailingSpaces(dest);
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
	ID_FF_DATEAFTERSIGN,
	ID_FF_DATEAFTEREDIT,
	ID_FF_DAYSAFTEREDIT,
	ID_FF_TIMEAFTEREDIT,
	ID_FF_DATEBEFORESIGN,
	ID_FF_DATEBEFOREEDIT,
	ID_FF_DAYSBEFOREEDIT,
	ID_FF_TIMEBEFOREEDIT,
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

	ID_FF_OK,
	ID_FF_RESET,
	ID_FF_CANCEL,
	ID_FF_MAKETRANSPARENT,
};

void HighlightDlgUpdateUserControl(CHAR_INFO *VBufColorExample, struct HighlightDataColor &Colors)
{
	const char *ptr;
	DWORD Color;
	const DWORD FarColor[] = {COL_PANELTEXT,COL_PANELSELECTEDTEXT,COL_PANELCURSOR,COL_PANELSELECTEDCURSOR};

	for (int j=0; j<4; j++)
	{
		Color=(DWORD)(Colors.Color[HIGHLIGHTCOLORTYPE_FILE][j]&0x00FF);

		if (!Color)
			Color=FarColorToReal(FarColor[j]);

		if (Colors.MarkChar&0x00FF)
			ptr=MSG(MHighlightExample2);
		else
			ptr=MSG(MHighlightExample1);

		for (int k=0; k<15; k++)
		{
			VBufColorExample[15*j+k].Char.AsciiChar=ptr[k];
			VBufColorExample[15*j+k].Attributes=(WORD)Color;
		}

		if (Colors.MarkChar&0x00FF)
		{
			VBufColorExample[15*j+1].Char.AsciiChar=Colors.MarkChar&0x00FF;

			if (Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][j]&0x00FF)
				VBufColorExample[15*j+1].Attributes=Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][j]&0x00FF;
		}

		VBufColorExample[15*j].Attributes=FarColorToReal(COL_PANELBOX);
		VBufColorExample[15*j+14].Attributes=FarColorToReal(COL_PANELBOX);
	}
}

void FilterDlgRelativeDateItemsUpdate(HANDLE hDlg, bool bClear)
{
	Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

	if (Dialog::SendDlgMessage(hDlg,DM_GETCHECK,ID_FF_DATERELATIVE,0))
	{
		Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DATEBEFOREEDIT,0);
		Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DATEAFTEREDIT,0);
		Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_CURRENT,0);
		Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DAYSBEFOREEDIT,1);
		Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DAYSAFTEREDIT,1);
	}
	else
	{
		Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DAYSBEFOREEDIT,0);
		Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DAYSAFTEREDIT,0);
		Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DATEBEFOREEDIT,1);
		Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_DATEAFTEREDIT,1);
		Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,ID_FF_CURRENT,1);
	}

	if (bClear)
	{
		Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEAFTEREDIT,(LONG_PTR)"");
		Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DAYSAFTEREDIT,(LONG_PTR)"");
		Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEAFTEREDIT,(LONG_PTR)"");
		Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEBEFOREEDIT,(LONG_PTR)"");
		Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEBEFOREEDIT,(LONG_PTR)"");
		Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DAYSBEFOREEDIT,(LONG_PTR)"");
	}

	Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
}

LONG_PTR WINAPI FileFilterConfigDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
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
				char Date[16],Time[16];

				if (Param1==ID_FF_CURRENT)
				{
					GetSystemTimeAsFileTime(&ft);
					ConvertDate(ft,Date,Time,8,FALSE,FALSE,TRUE);
				}
				else
					*Date=*Time=0;

				Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
				int relative = (int)Dialog::SendDlgMessage(hDlg,DM_GETCHECK,ID_FF_DATERELATIVE,0);
				int db = relative ? ID_FF_DAYSBEFOREEDIT : ID_FF_DATEBEFOREEDIT;
				int da = relative ? ID_FF_DAYSAFTEREDIT  : ID_FF_DATEAFTEREDIT;
				Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,da,(LONG_PTR)Date);
				Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEAFTEREDIT,(LONG_PTR)Time);
				Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,db,(LONG_PTR)Date);
				Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEBEFOREEDIT,(LONG_PTR)Time);
				Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,da,0);
				COORD r;
				r.X=r.Y=0;
				Dialog::SendDlgMessage(hDlg,DM_SETCURSORPOS,da,(LONG_PTR)&r);
				Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
				break;
			}
			else if (Param1==ID_FF_RESET) // Reset
			{
				Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);
				LONG_PTR ColorConfig = Dialog::SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
				Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_MASKEDIT,(LONG_PTR)"*");
				Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_SIZEFROMEDIT,(LONG_PTR)"");
				Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_SIZETOEDIT,(LONG_PTR)"");

				for (int I=ID_FF_READONLY; I <= ID_FF_VIRTUAL; ++I)
				{
					Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,BSTATE_3STATE);
				}

				if (!ColorConfig)
					Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_DIRECTORY,BSTATE_UNCHECKED);

				struct FarListPos LPos={0,0};
				Dialog::SendDlgMessage(hDlg,DM_LISTSETCURPOS,ID_FF_DATETYPE,(LONG_PTR)&LPos);
				Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHMASK,BSTATE_CHECKED);
				Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHSIZE,BSTATE_UNCHECKED);
				Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHDATE,BSTATE_UNCHECKED);
				Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_DATERELATIVE,BSTATE_UNCHECKED);
				FilterDlgRelativeDateItemsUpdate(hDlg, true);
				Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHATTRIBUTES,ColorConfig?BSTATE_UNCHECKED:BSTATE_CHECKED);
				Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
				break;
			}
			else if (Param1==ID_FF_MAKETRANSPARENT)
			{
				HighlightDataColor *Colors = (HighlightDataColor *) Dialog::SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);

				for (int i=0; i<2; i++)
					for (int j=0; j<4; j++)
						Colors->Color[i][j]|=0xFF00;

				Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_HER_MARKTRANSPARENT,BSTATE_CHECKED);
				break;
			}
			else if (Param1==ID_FF_DATERELATIVE)
			{
				FilterDlgRelativeDateItemsUpdate(hDlg, true);
				break;
			}
		}
		case DN_MOUSECLICK:

			if ((Msg==DN_BTNCLICK && Param1 >= ID_HER_NORMALFILE && Param1 <= ID_HER_SELECTEDCURSORMARKING)
			        || (Msg==DN_MOUSECLICK && Param1==ID_HER_COLOREXAMPLE && ((MOUSE_EVENT_RECORD *)Param2)->dwButtonState==FROM_LEFT_1ST_BUTTON_PRESSED))
			{
				HighlightDataColor *EditData = (HighlightDataColor *) Dialog::SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);

				if (Msg==DN_MOUSECLICK)
				{
					Param1 = ID_HER_NORMALFILE + ((MOUSE_EVENT_RECORD *)Param2)->dwMousePosition.Y*2;

					if (((MOUSE_EVENT_RECORD *)Param2)->dwMousePosition.X==1 && (EditData->MarkChar&0x00FF))
						Param1 = ID_HER_NORMALMARKING + ((MOUSE_EVENT_RECORD *)Param2)->dwMousePosition.Y*2;
				}

				//Color[0=file, 1=mark][0=normal,1=selected,2=undercursor,3=selectedundercursor]
				unsigned int Color=(unsigned int)EditData->Color[(Param1-ID_HER_NORMALFILE)&1][(Param1-ID_HER_NORMALFILE)/2];
				GetColorDialog(Color,true,true);
				EditData->Color[(Param1-ID_HER_NORMALFILE)&1][(Param1-ID_HER_NORMALFILE)/2]=(WORD)Color;
				FarDialogItem MarkChar, ColorExample;
				Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,ID_HER_MARKEDIT,(LONG_PTR)&MarkChar);
				Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,ID_HER_COLOREXAMPLE,(LONG_PTR)&ColorExample);
				EditData->MarkChar=*MarkChar.Data.Data;
				HighlightDlgUpdateUserControl(ColorExample.Param.VBuf,*EditData);
				Dialog::SendDlgMessage(hDlg,DM_SETDLGITEM,ID_HER_COLOREXAMPLE,(LONG_PTR)&ColorExample);
				return TRUE;
			}

			break;
		case DN_EDITCHANGE:

			if (Param1 == ID_HER_MARKEDIT)
			{
				HighlightDataColor *EditData = (HighlightDataColor *) Dialog::SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
				FarDialogItem *MarkChar, ColorExample;
				MarkChar=(FarDialogItem *)Param2;
				Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,ID_HER_COLOREXAMPLE,(LONG_PTR)&ColorExample);
				EditData->MarkChar=*(MarkChar->Data.Data);
				HighlightDlgUpdateUserControl(ColorExample.Param.VBuf,*EditData);
				Dialog::SendDlgMessage(hDlg,DM_SETDLGITEM,ID_HER_COLOREXAMPLE,(LONG_PTR)&ColorExample);
				return TRUE;
			}

			break;
		case DN_CLOSE:

			if (Param1 == ID_FF_OK && Dialog::SendDlgMessage(hDlg,DM_GETCHECK,ID_FF_MATCHSIZE,0))
			{
				char temp[512];
				Dialog::SendDlgMessage(hDlg,DM_GETTEXTPTR,ID_FF_SIZEFROMEDIT,(LONG_PTR)temp);
				bool bTemp = !*temp || CheckFileSizeStringFormat(temp);
				Dialog::SendDlgMessage(hDlg,DM_GETTEXTPTR,ID_FF_SIZETOEDIT,(LONG_PTR)temp);
				bTemp = bTemp && (!*temp || CheckFileSizeStringFormat(temp));

				if (!bTemp)
				{
					LONG_PTR ColorConfig = Dialog::SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
					Message(MSG_WARNING,1,ColorConfig?MSG(MFileHilightTitle):MSG(MFileFilterTitle),MSG(MBadFileSizeFormat),MSG(MOk));
					return FALSE;
				}
			}

			break;
	}

	return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

bool FileFilterConfig(FileFilterParams *FF, bool ColorConfig)
{
	const char VerticalLine[] = {0x0C2,0x0B3,0x0B3,0x0B3,0x0C1,0};
	// Временная маска.
	CFileMask FileMask;
	// История для маски файлов
	const char FilterMasksHistoryName[] = "FilterMasks";
	// История для имени фильтра
	const char FilterNameHistoryName[] = "FilterName";
	// Маски для диалога настройки
	// Маска для ввода дней для относительной даты
	const char DaysMask[] = "9999";
	char DateMask[16];
	char TimeMask[16];
	// Определение параметров даты и времени в системе.
	int DateSeparator=GetDateSeparator();
	int TimeSeparator=GetTimeSeparator();
	int DateFormat=GetDateFormat();

	switch (DateFormat)
	{
		case 0:
			// Маска даты для форматов DD.MM.YYYY и MM.DD.YYYY
			sprintf(DateMask,"99%c99%c9999",DateSeparator,DateSeparator);
			break;
		case 1:
			// Маска даты для форматов DD.MM.YYYY и MM.DD.YYYY
			sprintf(DateMask,"99%c99%c9999",DateSeparator,DateSeparator);
			break;
		default:
			// Маска даты для формата YYYY.MM.DD
			sprintf(DateMask,"9999%c99%c99",DateSeparator,DateSeparator);
			break;
	}

	// Маска времени
	sprintf(TimeMask,"99%c99%c99",TimeSeparator,TimeSeparator);
	struct DialogData FilterDlgData[]=
	{
		DI_DOUBLEBOX,3,1,73,18,0,0,DIF_SHOWAMPERSAND,0,(char *)MFileFilterTitle,

		DI_TEXT,5,2,0,2,1,0,0,0,(char *)MFileFilterName,
		DI_EDIT,5,2,71,2,0,(DWORD_PTR)FilterNameHistoryName,DIF_HISTORY,0,"",

		DI_TEXT,0,3,0,3,0,0,DIF_SEPARATOR,0,"",

		DI_CHECKBOX,5,4,0,4,0,0,DIF_AUTOMATION,0,(char *)MFileFilterMatchMask,
		DI_EDIT,5,4,71,4,0,(DWORD_PTR)FilterMasksHistoryName,DIF_HISTORY|DIF_VAREDIT,0,"",

		DI_TEXT,0,5,0,5,0,0,DIF_SEPARATOR,0,"",

		DI_CHECKBOX,5,6,0,6,0,0,DIF_AUTOMATION,0,(char *)MFileFilterSize,
		DI_TEXT,7,7,8,7,0,0,0,0,(char *)MFileFilterSizeFromSign,
		DI_EDIT,10,7,20,7,0,0,0,0,"",
		DI_TEXT,7,8,8,8,0,0,0,0,(char *)MFileFilterSizeToSign,
		DI_EDIT,10,8,20,8,0,0,0,0,"",

		DI_CHECKBOX,24,6,0,6,0,0,DIF_AUTOMATION,0,(char *)MFileFilterDate,
		DI_COMBOBOX,26,7,41,7,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,"",
		DI_CHECKBOX,26,8,0,8,0,0,0,0,(char *)MFileFilterDateRelative,
		DI_TEXT,50,7,51,7,0,0,0,0,(char *)MFileFilterDateAfterSign,
		DI_FIXEDIT,53,7,62,7,0,(DWORD_PTR)DateMask,DIF_MASKEDIT,0,"",
		DI_FIXEDIT,53,7,62,7,0,(DWORD_PTR)DaysMask,DIF_MASKEDIT,0,"",
		DI_FIXEDIT,64,7,71,7,0,(DWORD_PTR)TimeMask,DIF_MASKEDIT,0,"",
		DI_TEXT,50,8,51,8,0,0,0,0,(char *)MFileFilterDateBeforeSign,
		DI_FIXEDIT,53,8,62,8,0,(DWORD_PTR)DateMask,DIF_MASKEDIT,0,"",
		DI_FIXEDIT,53,8,62,8,0,(DWORD_PTR)DaysMask,DIF_MASKEDIT,0,"",
		DI_FIXEDIT,64,8,71,8,0,(DWORD_PTR)TimeMask,DIF_MASKEDIT,0,"",
		DI_BUTTON,0,6,0,6,0,0,DIF_BTNNOCLOSE,0,(char *)MFileFilterCurrent,
		DI_BUTTON,0,6,71,6,0,0,DIF_BTNNOCLOSE,0,(char *)MFileFilterBlank,

		DI_TEXT,0,9,0,9,0,0,DIF_SEPARATOR,0,"",
		DI_VTEXT,22,5,22,9,0,0,0,0,(char *)VerticalLine,

		DI_CHECKBOX, 5,10,0,10,0,0,DIF_AUTOMATION,0,(char *)MFileFilterAttr,
		DI_CHECKBOX, 7,11,0,11,0,0,DIF_3STATE,0,(char *)MFileFilterAttrR,
		DI_CHECKBOX, 7,12,0,12,0,0,DIF_3STATE,0,(char *)MFileFilterAttrA,
		DI_CHECKBOX, 7,13,0,13,0,0,DIF_3STATE,0,(char *)MFileFilterAttrH,
		DI_CHECKBOX, 7,14,0,14,0,0,DIF_3STATE,0,(char *)MFileFilterAttrS,

		DI_CHECKBOX,29,11,0,11,0,0,DIF_3STATE,0,(char *)MFileFilterAttrD,
		DI_CHECKBOX,29,12,0,12,0,0,DIF_3STATE,0,(char *)MFileFilterAttrC,
		DI_CHECKBOX,29,13,0,13,0,0,DIF_3STATE,0,(char *)MFileFilterAttrE,
		DI_CHECKBOX,29,14,0,14,0,0,DIF_3STATE,0,(char *)MFileFilterAttrNI,
		DI_CHECKBOX,29,15,0,15,0,0,DIF_3STATE,0,(char *)MFileFilterAttrReparse,

		DI_CHECKBOX,51,11,0,11,0,0,DIF_3STATE,0,(char *)MFileFilterAttrSparse,
		DI_CHECKBOX,51,12,0,12,0,0,DIF_3STATE,0,(char *)MFileFilterAttrT,
		DI_CHECKBOX,51,13,0,13,0,0,DIF_3STATE,0,(char *)MFileFilterAttrOffline,
		DI_CHECKBOX,51,14,0,14,0,0,DIF_3STATE,0,(char *)MFileFilterAttrVirtual,

		DI_TEXT,-1,14,0,14,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,(char *)MHighlightColors,
		DI_TEXT,7,15,0,15,0,0,0,0,(char *)MHighlightMarkChar,
		DI_FIXEDIT,5,15,5,15,0,0,0,0,"",
		DI_CHECKBOX,0,15,0,15,0,0,0,0,(char *)MHighlightTransparentMarkChar,

		DI_BUTTON,5,16,0,16,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(char *)MHighlightFileName1,
		DI_BUTTON,0,16,0,16,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(char *)MHighlightMarking1,
		DI_BUTTON,5,17,0,17,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(char *)MHighlightFileName2,
		DI_BUTTON,0,17,0,17,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(char *)MHighlightMarking2,
		DI_BUTTON,5,18,0,18,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(char *)MHighlightFileName3,
		DI_BUTTON,0,18,0,18,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(char *)MHighlightMarking3,
		DI_BUTTON,5,19,0,19,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(char *)MHighlightFileName4,
		DI_BUTTON,0,19,0,19,0,0,DIF_BTNNOCLOSE|DIF_NOBRACKETS,0,(char *)MHighlightMarking4,

		DI_USERCONTROL,73-15-1,16,73-2,19,0,0,DIF_NOFOCUS,0,"",
		DI_CHECKBOX,5,20,0,20,0,0,0,0,(char *)MHighlightContinueProcessing,

		DI_TEXT,0,16,0,16,0,0,DIF_SEPARATOR,0,"",

		DI_BUTTON,0,17,0,17,0,0,DIF_CENTERGROUP,1,(char *)MOk,
		DI_BUTTON,0,17,0,17,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(char *)MFileFilterReset,
		DI_BUTTON,0,17,0,17,0,0,DIF_CENTERGROUP,0,(char *)MFileFilterCancel,
		DI_BUTTON,0,17,0,17,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(char *)MFileFilterMakeTransparent,
	};
	FilterDlgData[0].Data=(char *)(ColorConfig?MFileHilightTitle:MFileFilterTitle);
	MakeDialogItems(FilterDlgData,FilterDlg);

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

	FilterDlg[ID_FF_NAMEEDIT].X1=FilterDlg[ID_FF_NAME].X1+(int)strlen(FilterDlg[ID_FF_NAME].Data)-(strchr(FilterDlg[ID_FF_NAME].Data,'&')?1:0)+1;
	FilterDlg[ID_FF_MASKEDIT].X1=FilterDlg[ID_FF_MATCHMASK].X1+(int)strlen(FilterDlg[ID_FF_MATCHMASK].Data)-(strchr(FilterDlg[ID_FF_MATCHMASK].Data,'&')?1:0)+5;
	FilterDlg[ID_FF_BLANK].X1=FilterDlg[ID_FF_BLANK].X2-(int)strlen(FilterDlg[ID_FF_BLANK].Data)+(strchr(FilterDlg[ID_FF_BLANK].Data,'&')?1:0)-3;
	FilterDlg[ID_FF_CURRENT].X2=FilterDlg[ID_FF_BLANK].X1-2;
	FilterDlg[ID_FF_CURRENT].X1=FilterDlg[ID_FF_CURRENT].X2-(int)strlen(FilterDlg[ID_FF_CURRENT].Data)+(strchr(FilterDlg[ID_FF_CURRENT].Data,'&')?1:0)-3;
	FilterDlg[ID_HER_MARKTRANSPARENT].X1=FilterDlg[ID_HER_MARK_TITLE].X1+(int)strlen(FilterDlg[ID_HER_MARK_TITLE].Data)-(strchr(FilterDlg[ID_HER_MARK_TITLE].Data,'&')?1:0)+1;

	for (int i=ID_HER_NORMALMARKING; i<=ID_HER_SELECTEDCURSORMARKING; i+=2)
		FilterDlg[i].X1=FilterDlg[ID_HER_NORMALFILE].X1+(int)strlen(FilterDlg[ID_HER_NORMALFILE].Data)-(strchr(FilterDlg[ID_HER_NORMALFILE].Data,'&')?1:0)+1;

	CHAR_INFO VBufColorExample[15*4];
	HighlightDataColor Colors;
	FF->GetColors(&Colors);
	memset(VBufColorExample,0,sizeof(VBufColorExample));
	HighlightDlgUpdateUserControl(VBufColorExample,Colors);
	FilterDlg[ID_HER_COLOREXAMPLE].VBuf=VBufColorExample;
	*FilterDlg[ID_HER_MARKEDIT].Data=Colors.MarkChar&0x00FF;
	FilterDlg[ID_HER_MARKTRANSPARENT].Selected=(Colors.MarkChar&0xFF00?1:0);
	FilterDlg[ID_HER_CONTINUEPROCESSING].Selected=(FF->GetContinueProcessing()?1:0);
	xstrncpy(FilterDlg[ID_FF_NAMEEDIT].Data,FF->GetTitle(),sizeof(FilterDlg[ID_FF_NAMEEDIT].Data));
	const char *FMask;
	FilterDlg[ID_FF_MATCHMASK].Selected=FF->GetMask(&FMask)?1:0;
	char Mask[FILEFILTER_MASK_SIZE];
	xstrncpy(Mask,FMask,sizeof(Mask)-1);
	FilterDlg[ID_FF_MASKEDIT].Ptr.PtrData=Mask;
	FilterDlg[ID_FF_MASKEDIT].Ptr.PtrLength=sizeof(Mask)-1;
	FilterDlg[ID_FF_MASKEDIT].Ptr.PtrFlags=0;

	if (!FilterDlg[ID_FF_MATCHMASK].Selected)
		FilterDlg[ID_FF_MASKEDIT].Flags|=DIF_DISABLE;

	const char *SizeAbove, *SizeBelow;
	FilterDlg[ID_FF_MATCHSIZE].Selected=FF->GetSize(&SizeAbove,&SizeBelow)?1:0;
	xstrncpy(FilterDlg[ID_FF_SIZEFROMEDIT].Data,SizeAbove,sizeof(FilterDlg[ID_FF_SIZEFROMEDIT].Data)-1);
	xstrncpy(FilterDlg[ID_FF_SIZETOEDIT].Data,SizeBelow,sizeof(FilterDlg[ID_FF_SIZETOEDIT].Data)-1);

	if (!FilterDlg[ID_FF_MATCHSIZE].Selected)
		for (int i=ID_FF_SIZEFROMSIGN; i <= ID_FF_SIZETOEDIT; i++)
			FilterDlg[i].Flags|=DIF_DISABLE;

	// Лист для комбобокса времени файла
	FarList DateList;
	FarListItem TableItemDate[FDATE_COUNT];
	// Настройка списка типов дат файла
	DateList.Items=TableItemDate;
	DateList.ItemsNumber=FDATE_COUNT;
	memset(TableItemDate,0,sizeof(TableItemDate));

	for (int i=0; i < FDATE_COUNT; ++i)
		xstrncpy(TableItemDate[i].Text,MSG(MFileFilterModified+i),sizeof(TableItemDate[i].Text)-1);

	DWORD DateType;
	FILETIME DateAfter, DateBefore;
	bool bRelative;
	FilterDlg[ID_FF_MATCHDATE].Selected=FF->GetDate(&DateType,&DateAfter,&DateBefore,&bRelative)?1:0;
	FilterDlg[ID_FF_DATERELATIVE].Selected=bRelative?1:0;
	FilterDlg[ID_FF_DATETYPE].ListItems=&DateList;
	TableItemDate[DateType].Flags=LIF_SELECTED;

	if (bRelative)
	{
		ConvertRelativeDate(DateAfter, FilterDlg[ID_FF_DAYSAFTEREDIT].Data, FilterDlg[ID_FF_TIMEAFTEREDIT].Data);
		ConvertRelativeDate(DateBefore, FilterDlg[ID_FF_DAYSBEFOREEDIT].Data, FilterDlg[ID_FF_TIMEBEFOREEDIT].Data);
	}
	else
	{
		ConvertDate(DateAfter,FilterDlg[ID_FF_DATEAFTEREDIT].Data,FilterDlg[ID_FF_TIMEAFTEREDIT].Data,8,FALSE,FALSE,TRUE);
		ConvertDate(DateBefore,FilterDlg[ID_FF_DATEBEFOREEDIT].Data,FilterDlg[ID_FF_TIMEBEFOREEDIT].Data,8,FALSE,FALSE,TRUE);
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

	Dialog Dlg(FilterDlg,sizeof(FilterDlg)/sizeof(FilterDlg[0]),FileFilterConfigDlgProc,(LONG_PTR)(ColorConfig?&Colors:NULL));
	Dlg.SetHelp(ColorConfig?"HighlightEdit":"Filter");
	Dlg.SetPosition(-1,-1,FilterDlg[ID_FF_TITLE].X2+4,FilterDlg[ID_FF_TITLE].Y2+2);
	Dlg.SetAutomation(ID_FF_MATCHMASK,ID_FF_MASKEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEFROMSIGN,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEFROMEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZETOSIGN,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZETOEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATETYPE,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATERELATIVE,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEAFTERSIGN,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEAFTEREDIT,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DAYSAFTEREDIT,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_TIMEAFTEREDIT,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEBEFORESIGN,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEBEFOREEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DAYSBEFOREEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_TIMEBEFOREEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_CURRENT,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_BLANK,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_READONLY,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_ARCHIVE,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_HIDDEN,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_SYSTEM,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_COMPRESSED,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_ENCRYPTED,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_NOTINDEXED,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_SPARSE,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_TEMP,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_REPARSEPOINT,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_OFFLINE,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_VIRTUAL,DIF_DISABLE,0,0,DIF_DISABLE);
	Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_DIRECTORY,DIF_DISABLE,0,0,DIF_DISABLE);

	for (;;)
	{
		Dlg.ClearDone();
		Dlg.Process();
		int ExitCode=Dlg.GetExitCode();

		if (ExitCode==ID_FF_OK) // Ok
		{
			// Если введённая пользователем маска не корректна, тогда вернёмся в диалог
			if (FilterDlg[ID_FF_MATCHMASK].Selected && !FileMask.Set((char *)FilterDlg[ID_FF_MASKEDIT].Ptr.PtrData,0))
				continue;

			if (FilterDlg[ID_HER_MARKTRANSPARENT].Selected)
				Colors.MarkChar|=0xFF00;
			else
				Colors.MarkChar&=0x00FF;

			FF->SetColors(&Colors);
			FF->SetContinueProcessing(FilterDlg[ID_HER_CONTINUEPROCESSING].Selected!=0);
			FF->SetTitle(FilterDlg[ID_FF_NAMEEDIT].Data);
			FF->SetMask(FilterDlg[ID_FF_MATCHMASK].Selected!=0,
			            (char *)FilterDlg[ID_FF_MASKEDIT].Ptr.PtrData);
			FF->SetSize(FilterDlg[ID_FF_MATCHSIZE].Selected!=0,
			            FilterDlg[ID_FF_SIZEFROMEDIT].Data,
			            FilterDlg[ID_FF_SIZETOEDIT].Data);
			bRelative = FilterDlg[ID_FF_DATERELATIVE].Selected!=0;
			StrToDateTime(FilterDlg[bRelative?ID_FF_DAYSAFTEREDIT:ID_FF_DATEAFTEREDIT].Data,FilterDlg[ID_FF_TIMEAFTEREDIT].Data,DateAfter,DateFormat,DateSeparator,TimeSeparator,bRelative);
			StrToDateTime(FilterDlg[bRelative?ID_FF_DAYSBEFOREEDIT:ID_FF_DATEBEFOREEDIT].Data,FilterDlg[ID_FF_TIMEBEFOREEDIT].Data,DateBefore,DateFormat,DateSeparator,TimeSeparator,bRelative);
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
