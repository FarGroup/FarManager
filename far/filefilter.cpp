/*
filefilter.cpp

Файловый фильтр

*/

/* Revision: 1.18 01.09.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "filefilter.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "dialog.hpp"
#include "global.hpp"

static DWORD CompEnabled,EncrEnabled,IsNTFS,ReparsePointEnadled;
static DWORD SizeType,DateType;

// Запрет на использование атрибута Directory при фильтровании
static int DisableDir=FALSE;

FileFilter::FileFilter(int DisableDirAttr):
  FmtMask1(L"99%c99%c9999"),
  FmtMask2(L"9999%c99%c99"),
  FmtMask3(L"99%c99%c99"),
  DigitMask(L"99999999999999999999"),
  FilterMasksHistoryName(L"FilterMasks")
{
  int I;

  // Запретим использование атрибута Directory при фильтровании
  DisableDir=DisableDirAttr;

  // Определение параметров даты и времени в системе.
  DateSeparator=GetDateSeparator();
  TimeSeparator=GetTimeSeparator();
  DateFormat=GetDateFormat();

  switch(DateFormat)
  {
    case 0:
      strDateMask.Format (FmtMask1,DateSeparator,DateSeparator);
      break;
    case 1:
      strDateMask.Format (FmtMask1,DateSeparator,DateSeparator);
      break;
    default:
      strDateMask.Format (FmtMask2,DateSeparator,DateSeparator);
      break;
  }
  strTimeMask.Format (FmtMask3,TimeSeparator,TimeSeparator);

  // Настройка списка множителей для типа размера
  TableItemSize=(FarListItem *)xf_malloc(sizeof(FarListItem)*FSIZE_IN_LAST);
  SizeList.Items=TableItemSize;
  SizeList.ItemsNumber=FSIZE_IN_LAST;

  memset(TableItemSize,0,sizeof(FarListItem)*FSIZE_IN_LAST);
  for(I=0; I < FSIZE_IN_LAST; ++I)
    xwcsncpy(TableItemSize[I].Text,UMSG(MFileFilterSizeInBytes+I),(sizeof(TableItemSize[I].Text)-1)/sizeof (wchar_t));

  // Настройка списка типов дат файла
  TableItemDate=(FarListItem *)xf_malloc(sizeof(FarListItem)*DATE_COUNT);
  DateList.Items=TableItemDate;
  DateList.ItemsNumber=DATE_COUNT;

  memset(TableItemDate,0,sizeof(FarListItem)*DATE_COUNT);
  xwcsncpy(TableItemDate[0].Text,UMSG(MFileFilterModified),(sizeof(TableItemDate[0].Text)-1)/sizeof (wchar_t));
  xwcsncpy(TableItemDate[1].Text,UMSG(MFileFilterCreated),(sizeof(TableItemDate[1].Text)-1)/sizeof (wchar_t));
  xwcsncpy(TableItemDate[2].Text,UMSG(MFileFilterOpened),(sizeof(TableItemDate[2].Text)-1)/sizeof (wchar_t));

  CompEnabled=EncrEnabled=IsNTFS=ReparsePointEnadled=FALSE;
  // Том поддерживает компрессию и шифрацию?
  unsigned long FSFlags=0;
  string strFSysName;
  if (apiGetVolumeInformation (NULL,NULL,NULL,NULL,&FSFlags,&strFSysName))
  {
    if (FSFlags & FS_FILE_COMPRESSION)
      CompEnabled=TRUE;

    if (FSFlags & FILE_SUPPORTS_REPARSE_POINTS)
      ReparsePointEnadled=TRUE;

    if ((IsCryptFileASupport) && (FSFlags & FS_FILE_ENCRYPTION))
      EncrEnabled=TRUE;

    if (!wcscmp(strFSysName, L"NTFS"))
      IsNTFS=TRUE;
  }

  // Скопируем текущее состояние фильтра во временное значение,
  // с которым и будем работать
  FF=Opt.OpFilter;

  // Проверка на валидность текущих настроек фильтра
  if ( FF.FMask.strMask.IsEmpty() || (!FilterMask.Set(FF.FMask.strMask,FMF_SILENT)))
    FF.FMask.strMask = L"*.*";

  // Сохраним маску фильтра в члене класса для ускорения процесса проверки файла.
  FilterMask.Set(FF.FMask.strMask,FMF_SILENT);

  SizeType=FF.FSize.SizeType;
  if (SizeType>FSIZE_INMBYTES || SizeType<FSIZE_INBYTES)
  {
    SizeType=0;
    FF.FSize.SizeType=(FSizeType) SizeType;
  }

  DateType=FF.FDate.DateType;
  if (DateType>FDATE_OPENED || DateType<FDATE_MODIFIED)
  {
    DateType=0;
    FF.FDate.DateType=(FDateType) DateType;
  }

}

FileFilter::~FileFilter()
{
  xf_free(TableItemSize);
  xf_free(TableItemDate);

  // Сохраним текущие изменения в глобальном фильтре
  Opt.OpFilter=FF;
}

enum enumFileFilterConfigure {
    ID_FF_TITLE,

    ID_FF_MATCHMASK,
    ID_FF_MASKEDIT,

    ID_FF_SEPARATOR1,

    ID_FF_MATCHSIZE,
    ID_FF_SIZEDIVIDER,
    ID_FF_SIZEFROM,
    ID_FF_SIZEFROMEDIT,
    ID_FF_SIZETO,
    ID_FF_SIZETOEDIT,

    ID_FF_SEPARATOR2,

    ID_FF_MATCHDATE,
    ID_FF_DATETYPE,
    ID_FF_DATEAFTER,
    ID_FF_DATEAFTEREDIT,
    ID_FF_TIMEAFTEREDIT,
    ID_FF_DATEBEFORE,
    ID_FF_DATEBEFOREEDIT,
    ID_FF_TIMEBEFOREEDIT,
    ID_FF_CURRENT,
    ID_FF_BLANK,

    ID_FF_SEPARATOR3,

    ID_FF_MATCHATTRIBUTES,
    ID_FF_READONLY,
    ID_FF_ARCHIVE,
    ID_FF_HIDDEN,
    ID_FF_SYSTEM,
    ID_FF_DIRECTORY,
    ID_FF_COMPRESSED,
    ID_FF_ENCRYPTED,
    ID_FF_NOTINDEXED,
    ID_FF_SPARSE,
    ID_FF_TEMP,
    ID_FF_REPARSEPOINT,

    ID_FF_SEPARATOR4,

    ID_FF_OK,
    ID_FF_RESET,
    ID_FF_CANCEL
    };


long WINAPI FileFilter::FilterDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2)
{
  switch(Msg)
  {
    case DN_LISTCHANGE:
    {
      if (Param1==ID_FF_SIZEDIVIDER)  // In bytes or kilobytes?
        SizeType=Param2;
      if (Param1==ID_FF_DATETYPE) // Modified,created or accessed?
        DateType=Param2;
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
          ConvertDateW(ft,strDate,strTime,8,FALSE,FALSE,TRUE);
        }
        else
          strDate=strTime=L"";

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEAFTEREDIT,(long)(const wchar_t*)strDate);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEAFTEREDIT,(long)(const wchar_t*)strTime);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEBEFOREEDIT,(long)(const wchar_t*)strDate);
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEBEFOREEDIT,(long)(const wchar_t*)strTime);

        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,ID_FF_DATEAFTEREDIT,0);
        COORD r;
        r.X=r.Y=0;
        Dialog::SendDlgMessage(hDlg,DM_SETCURSORPOS,ID_FF_DATEAFTEREDIT,(long)&r);

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
      }
      if (Param1==ID_FF_OK || Param1==ID_FF_CANCEL) // Ok и Cancel
        return FALSE;

      if (Param1==ID_FF_RESET) // Reset
      {
        // очистка диалога
        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_MASKEDIT,(long)L"*.*");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_SIZEFROMEDIT,(long)L"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_SIZETOEDIT,(long)L"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEAFTEREDIT,(long)L"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEAFTEREDIT,(long)L"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_DATEBEFOREEDIT,(long)L"");
        Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,ID_FF_TIMEBEFOREEDIT,(long)L"");

        /* 14.06.2004 KM
           Заменим BSTATE_UNCHECKED на BSTATE_3STATE, в данном
           случае это будет логичнее, т.с. дефолтное значение
        */
        for(int I=ID_FF_READONLY; I <= ID_FF_REPARSEPOINT; ++I)
          Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,BSTATE_3STATE);

        // 6, 13 - позиции в списке
        struct FarListPos LPos={0,0};
        SizeType=0;
        Dialog::SendDlgMessage(hDlg,DM_LISTSETCURPOS,ID_FF_SIZEDIVIDER,(long)&LPos);
        DateType=0;
        Dialog::SendDlgMessage(hDlg,DM_LISTSETCURPOS,ID_FF_DATETYPE,(long)&LPos);

        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHMASK,BSTATE_UNCHECKED);
        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHSIZE,BSTATE_UNCHECKED);
        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHDATE,BSTATE_UNCHECKED);
        Dialog::SendDlgMessage(hDlg,DM_SETCHECK,ID_FF_MATCHATTRIBUTES,BSTATE_UNCHECKED);

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
        break;
      }
      return TRUE;
    }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}


void FileFilter::Configure()
{
/*
    0000000000111111111122222222223333333333444444444455555555556666666666777
    0123456789012345678901234567890123456789012345678901234567890123456789012
00  """""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
01  "  +-------------------- Operations filter -----------------      ---+  "
02  "  | +-[ ] Match file mask(s) -----------------------------      --+ |  "
03  "  | | *.*##################################################       | |  "
04  "  | +-----------------------------------------------------      --+ |  "
05  "  | +-[ ] File size---------------------------------------      --+ |  "
06  "  | | In bytes#################                                   | |  "
07  "  | | Greater than or equal to:        ####################       | |  "
08  "  | | Less than or equal to:           ####################       | |  "
09  "  | +------------------------------------------------------      -+ |  "
10  "  | +-[ ] File date---------------------------------------      --+ |  "
11  "  | | Last modified time#######                                   | |  "
12  "  | | After:                            ##.##.#### ##:##:##       | |  "
13  "  | | Before:                           ##.##.#### ##:##:##       | |  "
14  "  | | [ Current ] [ Blank ]                                       | |  "
15  "  | +-----------------------------------------------------      --+ |  "
16  "  | +-[ ] Attributes-------------------------------------      ---+ |  "
17  "  | | [x] Только чтение   [x] Системный       [x] Разреженный     | |  "
18  "  | | [x] Архивный        [x] Сжатый          [x] Временный       | |  "
19  "  | | [x] Directory       [x] Зашифрованный                       | |  "
20  "  | | [x] Скрытый         [x] Неиндексируемый                     | |  "
21  "  | +--------------------------------------------------------  ---+ |  "
22  "  |                [ Ok ]  [ Reset]  [ Cancel ]                     |  "
23  "  +----------------------------------------------------------  -  --+  "
24  "                                                                       "
25  """""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
*/

  // Временная маска.
  CFileMaskW FileMask;
  int I;

  struct DialogDataEx FilterDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,73,21,0,0,DIF_SHOWAMPERSAND,0,(const wchar_t *)MFileFilterTitle,

  /* 01 */DI_CHECKBOX,5,2,0,0,1,0,DIF_AUTOMATION,0,(const wchar_t *)MFileFilterMatchMask,
  /* 02 */DI_EDIT,7,3,71,3,0,(DWORD)FilterMasksHistoryName,DIF_HISTORY,0,L"",

  /* 03 */DI_TEXT,0,4,0,0,0,0,DIF_SEPARATOR,0,L"",

  /* 04 */DI_CHECKBOX,5,5,0,0,0,0,DIF_AUTOMATION,0,(const wchar_t *)MFileFilterSize,
  /* 05 */DI_COMBOBOX,52,5,71,5,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,L"",
  /* 06 */DI_TEXT,7,6,38,6,0,0,0,0,(const wchar_t *)MFileFilterSizeFrom,
  /* 07 */DI_FIXEDIT,52,6,71,6,0,(DWORD)DigitMask,DIF_MASKEDIT,0,L"",
  /* 08 */DI_TEXT,7,7,38,7,0,0,0,0,(const wchar_t *)MFileFilterSizeTo,
  /* 09 */DI_FIXEDIT,52,7,71,7,0,(DWORD)DigitMask,DIF_MASKEDIT,0,L"",

  /* 10 */DI_TEXT,0,8,0,0,0,0,DIF_SEPARATOR,0,L"",

  /* 11 */DI_CHECKBOX,5,9,0,0,0,0,DIF_AUTOMATION,0,(const wchar_t *)MFileFilterDate,
  /* 12 */DI_COMBOBOX,53,9,71,9,0,0,DIF_DROPDOWNLIST|DIF_LISTNOAMPERSAND,0,L"",
  /* 13 */DI_TEXT,7,10,38,10,0,0,0,0,(const wchar_t *)MFileFilterAfter,
  /* 14 */DI_FIXEDIT,53,10,62,10,0,(DWORD)(const wchar_t*)strDateMask,DIF_MASKEDIT,0,L"",
  /* 15 */DI_FIXEDIT,64,10,71,10,0,(DWORD)(const wchar_t*)strTimeMask,DIF_MASKEDIT,0,L"",
  /* 16 */DI_TEXT,7,11,40,11,0,0,0,0,(const wchar_t *)MFileFilterBefore,
  /* 17 */DI_FIXEDIT,53,11,62,11,0,(DWORD)(const wchar_t*)strDateMask,DIF_MASKEDIT,0,L"",
  /* 18 */DI_FIXEDIT,64,11,71,11,0,(DWORD)(const wchar_t*)strTimeMask,DIF_MASKEDIT,0,L"",
  /* 19 */DI_BUTTON,0,12,0,12,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(const wchar_t *)MFileFilterCurrent,
  /* 20 */DI_BUTTON,0,12,0,12,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(const wchar_t *)MFileFilterBlank,

  /* 21 */DI_TEXT,0,13,0,0,0,0,DIF_SEPARATOR,0,L"",

  /* 22 */DI_CHECKBOX, 5,14,0,0,0,0,DIF_AUTOMATION,0,(const wchar_t *)MFileFilterAttr,
  /* 23 */DI_CHECKBOX, 7,15,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrR,
  /* 24 */DI_CHECKBOX, 7,16,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrA,
  /* 25 */DI_CHECKBOX, 7,17,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrH,
  /* 26 */DI_CHECKBOX, 7,18,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrS,
  /* 27 */DI_CHECKBOX,29,15,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrD,
  /* 28 */DI_CHECKBOX,29,16,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrC,
  /* 29 */DI_CHECKBOX,29,17,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrE,
  /* 30 */DI_CHECKBOX,29,18,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrNI,
  /* 31 */DI_CHECKBOX,51,15,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrSparse,
  /* 32 */DI_CHECKBOX,51,16,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrT,
  /* 33 */DI_CHECKBOX,51,17,0,0,0,0,DIF_3STATE,0,(const wchar_t *)MFileFilterAttrReparse,

  /* 34 */DI_TEXT, 0, 19, 0, 0, 0, 0, DIF_SEPARATOR, 0, L"",

  /* 35 */DI_BUTTON,0,20,0,20,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MFileFilterOk,
  /* 36 */DI_BUTTON,0,20,0,20,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(const wchar_t *)MFileFilterReset,
  /* 37 */DI_BUTTON,0,20,0,20,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MFileFilterCancel,
  };

  MakeDialogItemsEx(FilterDlgData,FilterDlg);

  FilterDlg[ID_FF_MATCHMASK].Selected=FF.FMask.Used;
  FilterDlg[ID_FF_MASKEDIT].strData = FF.FMask.strMask;

  if (!FilterDlg[ID_FF_MATCHMASK].Selected)
    FilterDlg[ID_FF_MASKEDIT].Flags|=DIF_DISABLE;

  FilterDlg[ID_FF_MATCHSIZE].Selected=FF.FSize.Used;
  FilterDlg[ID_FF_SIZEDIVIDER].ListItems=&SizeList;

  SizeType=FF.FSize.SizeType;

  FilterDlg[ID_FF_SIZEDIVIDER].strData = TableItemSize[SizeType].Text;

  /*
  if (FF.FSize.SizeAbove != _i64(-1))
    _ui64toa(FF.FSize.SizeAbove,FilterDlg[ID_FF_SIZEFROMEDIT].Data,10);

  if (FF.FSize.SizeBelow != _i64(-1))
    _ui64toa(FF.FSize.SizeBelow,FilterDlg[ID_FF_SIZETOEDIT].Data,10);
    */ //BUGBUG

  if (!FilterDlg[ID_FF_MATCHSIZE].Selected)
    for(I=ID_FF_SIZEDIVIDER; I <= ID_FF_SIZETOEDIT; ++I)
      FilterDlg[I].Flags|=DIF_DISABLE;

  FilterDlg[ID_FF_MATCHDATE].Selected=FF.FDate.Used;
  FilterDlg[ID_FF_DATETYPE].ListItems=&DateList;

  DateType=FF.FDate.DateType;

  FilterDlg[ID_FF_DATETYPE].strData = TableItemDate[DateType].Text;

  ConvertDateW(FF.FDate.DateAfter,FilterDlg[ID_FF_DATEAFTEREDIT].strData,FilterDlg[ID_FF_TIMEAFTEREDIT].strData,8,FALSE,FALSE,TRUE);
  ConvertDateW(FF.FDate.DateBefore,FilterDlg[ID_FF_DATEBEFOREEDIT].strData,FilterDlg[ID_FF_TIMEBEFOREEDIT].strData,8,FALSE,FALSE,TRUE);

  if (!FilterDlg[ID_FF_MATCHDATE].Selected)
    for(I=ID_FF_DATETYPE; I <= ID_FF_BLANK; ++I)
      FilterDlg[I].Flags|=DIF_DISABLE;

  DWORD AttrSet=FF.FAttr.AttrSet;
  DWORD AttrClear=FF.FAttr.AttrClear;

  FilterDlg[ID_FF_MATCHATTRIBUTES].Selected=FF.FAttr.Used;
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

  if (!FilterDlg[ID_FF_MATCHATTRIBUTES].Selected)
  {
    for(I=ID_FF_READONLY; I <= ID_FF_REPARSEPOINT; ++I)
      FilterDlg[I].Flags|=DIF_DISABLE;
  }
  if (CompEnabled==FALSE)
  {
    FilterDlg[ID_FF_COMPRESSED].Flags|=DIF_DISABLE;
    FilterDlg[ID_FF_COMPRESSED].Selected=2;
  }
  if (EncrEnabled==FALSE)
  {
    FilterDlg[ID_FF_ENCRYPTED].Flags|=DIF_DISABLE;
    FilterDlg[ID_FF_ENCRYPTED].Selected=2;
  }
  if (!IsNTFS)
  {
    FilterDlg[ID_FF_NOTINDEXED].Flags|=DIF_DISABLE;
    FilterDlg[ID_FF_NOTINDEXED].Selected=2;
    FilterDlg[ID_FF_SPARSE].Flags|=DIF_DISABLE;
    FilterDlg[ID_FF_SPARSE].Selected=2;
    FilterDlg[ID_FF_TEMP].Flags|=DIF_DISABLE;
    FilterDlg[ID_FF_TEMP].Selected=2;
  }
  if(!ReparsePointEnadled)
  {
    FilterDlg[ID_FF_REPARSEPOINT].Flags|=DIF_DISABLE;
    FilterDlg[ID_FF_REPARSEPOINT].Selected=2;
  }
  if (DisableDir)
  {
    FilterDlg[ID_FF_DIRECTORY].Flags|=DIF_DISABLE;
    FilterDlg[ID_FF_DIRECTORY].Selected=2;
  }

  Dialog Dlg(FilterDlg,sizeof(FilterDlg)/sizeof(FilterDlg[0]),FilterDlgProc);

  Dlg.SetHelp(L"OpFilter");
  Dlg.SetPosition(-1,-1,77,23);

  Dlg.SetAutomation(ID_FF_MATCHMASK,ID_FF_MASKEDIT,DIF_DISABLE,0,0,DIF_DISABLE);

  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEDIVIDER,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEFROM,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZEFROMEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZETO,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHSIZE,ID_FF_SIZETOEDIT,DIF_DISABLE,0,0,DIF_DISABLE);

  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATETYPE,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEAFTER,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEAFTEREDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_TIMEAFTEREDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEBEFORE,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_DATEBEFOREEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_TIMEBEFOREEDIT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_CURRENT,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHDATE,ID_FF_BLANK,DIF_DISABLE,0,0,DIF_DISABLE);

  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_READONLY,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_ARCHIVE,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_HIDDEN,DIF_DISABLE,0,0,DIF_DISABLE);
  Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_SYSTEM,DIF_DISABLE,0,0,DIF_DISABLE);
  if (CompEnabled)
    Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_COMPRESSED,DIF_DISABLE,0,0,DIF_DISABLE);
  if (EncrEnabled)
    Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_ENCRYPTED,DIF_DISABLE,0,0,DIF_DISABLE);
  if (IsNTFS)
  {
    Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_NOTINDEXED,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_SPARSE,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_TEMP,DIF_DISABLE,0,0,DIF_DISABLE);
  }
  if(ReparsePointEnadled)
  {
    Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_REPARSEPOINT,DIF_DISABLE,0,0,DIF_DISABLE);
  }
  if (!DisableDir)
    Dlg.SetAutomation(ID_FF_MATCHATTRIBUTES,ID_FF_DIRECTORY,DIF_DISABLE,0,0,DIF_DISABLE);

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

      FF.FMask.Used=FilterDlg[ID_FF_MATCHMASK].Selected;
      FF.FMask.strMask = FilterDlg[ID_FF_MASKEDIT].strData;

      // Сохраним маску фильтра в члене класса для ускорения процесса проверки файла.
      FilterMask.Set(FF.FMask.strMask,FMF_SILENT);

      FF.FSize.Used=FilterDlg[ID_FF_MATCHSIZE].Selected;
      FF.FSize.SizeType=(FSizeType) SizeType;

      RemoveExternalSpacesW(FilterDlg[ID_FF_SIZEFROMEDIT].strData);

      if( FilterDlg[ID_FF_SIZEFROMEDIT].strData.IsEmpty() )
        FF.FSize.SizeAbove=_i64(-1);
      else
        FF.FSize.SizeAbove=_wtoi64(FilterDlg[ID_FF_SIZEFROMEDIT].strData);

      RemoveExternalSpacesW(FilterDlg[ID_FF_SIZETOEDIT].strData);

      if ( FilterDlg[ID_FF_SIZETOEDIT].strData.IsEmpty() )
        FF.FSize.SizeBelow=_i64(-1);
      else
        FF.FSize.SizeBelow=_wtoi64(FilterDlg[ID_FF_SIZETOEDIT].strData);

      FF.FDate.Used=FilterDlg[ID_FF_MATCHDATE].Selected;
      FF.FDate.DateType=(FDateType) DateType;
      StrToDateTime(FilterDlg[ID_FF_DATEAFTEREDIT].strData,FilterDlg[ID_FF_TIMEAFTEREDIT].strData,FF.FDate.DateAfter);
      StrToDateTime(FilterDlg[ID_FF_DATEBEFOREEDIT].strData,FilterDlg[ID_FF_TIMEBEFOREEDIT].strData,FF.FDate.DateBefore);

      DWORD AttrSet=0;
      DWORD AttrClear=0;

      FF.FAttr.Used=FilterDlg[ID_FF_MATCHATTRIBUTES].Selected;

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
      FF.FAttr.AttrSet=AttrSet;
      FF.FAttr.AttrClear=AttrClear;

      break;
    }
    else
      break;
  }
}

FILETIME &FileFilter::StrToDateTime(const wchar_t *CDate,const wchar_t *CTime,FILETIME &ft)
{
  unsigned DateN[3],TimeN[3];
  SYSTEMTIME st;
  FILETIME lft;

  // Преобразуем введённые пользователем дату и время
  GetFileDateAndTime(CDate,DateN,DateSeparator);
  GetFileDateAndTime(CTime,TimeN,GetTimeSeparator());
  if(DateN[0] == -1 || DateN[1] == -1 || DateN[2] == -1)
  {
    // Пользователь оставил дату пустой, значит обнулим дату и время.
    ZeroMemory(&ft,sizeof(ft));
    return ft;
  }

  ZeroMemory(&st,sizeof(st));

  // "Оформим"
  switch(DateFormat)
  {
    case 0:
      st.wMonth=DateN[0]!=(unsigned)-1?DateN[0]:0;
      st.wDay  =DateN[1]!=(unsigned)-1?DateN[1]:0;
      st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:0;
      break;
    case 1:
      st.wDay  =DateN[0]!=(unsigned)-1?DateN[0]:0;
      st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:0;
      st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:0;
      break;
    default:
      st.wYear =DateN[0]!=(unsigned)-1?DateN[0]:0;
      st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:0;
      st.wDay  =DateN[2]!=(unsigned)-1?DateN[2]:0;
      break;
  }
  st.wHour   = TimeN[0]!=(unsigned)-1?(TimeN[0]):0;
  st.wMinute = TimeN[1]!=(unsigned)-1?(TimeN[1]):0;
  st.wSecond = TimeN[2]!=(unsigned)-1?(TimeN[2]):0;

  if (st.wYear<100)
    if (st.wYear<80)
      st.wYear+=2000;
    else
      st.wYear+=1900;

  // преобразование в "удобоваримый" формат
  SystemTimeToFileTime(&st,&lft);
  LocalFileTimeToFileTime(&lft,&ft);
  return ft;
}

void FileFilter::GetFileDateAndTime(const wchar_t *Src,unsigned *Dst,int Separator)
{
  wchar_t Temp[32]; //BUGBUG
  string strDigit;
  const wchar_t *PtrDigit;
  int I;

  xwcsncpy(Temp,Src,(sizeof(Temp)-1)/sizeof (wchar_t));
  Dst[0]=Dst[1]=Dst[2]=(unsigned)-1;
  I=0;
  const wchar_t *Ptr=Temp;
  while((Ptr=GetCommaWordW(Ptr,strDigit,Separator)) != NULL)
  {
    PtrDigit=strDigit;
    while (*PtrDigit && !iswdigit(*PtrDigit))
      PtrDigit++;
    if(*PtrDigit)
      Dst[I]=_wtoi(PtrDigit);
    ++I;
  }
}

int FileFilter::FileInFilter(const FAR_FIND_DATA *fd)
{
    FAR_FIND_DATA_EX fdata;

    apiFindDataToDataEx (fd, &fdata);

    return FileInFilter (&fdata);
}

int FileFilter::FileInFilter(const FAR_FIND_DATA_EX *fd)
{
  // Пустое значение?
  if (fd==NULL)
    return FALSE;

  // Режим проверки маски файла включен?
  if (FF.FMask.Used)
  {
    // Файл не попадает под маску введённую в фильтре?

    if (!FilterMask.Compare(fd->strFileName))
      // Не пропускаем этот файл
      return FALSE;
  }

  // Режим проверки размера файла включен?
  if (FF.FSize.Used)
  {
    unsigned __int64 sizeabove=FF.FSize.SizeAbove;
    unsigned __int64 sizebelow=FF.FSize.SizeBelow;

    // Преобразуем размер из двух DWORD в беззнаковый __int64
    unsigned __int64 fsize=fd->nFileSize;

    if (sizeabove != (unsigned __int64)-1)
    {
      switch (FF.FSize.SizeType)
      {
        case FSIZE_INBYTES:
          // Размер введён в байтах, значит ничего не меняем.
          break;
        case FSIZE_INKBYTES:
          // Размер введён в килобайтах, переведём его в байты.
          // !!! Проверки на превышение максимального значения не делаются !!!
          sizeabove=sizeabove*_i64(1024);
          break;
        case FSIZE_INMBYTES:
          // Задел // Размер введён в мегабайтах, переведём его в байты.
          // !!! Проверки на превышение максимального значения не делаются !!!
          sizeabove=sizeabove*_i64(1024)*_i64(1024);
          break;
        case FSIZE_INGBYTES:
          // Задел // Размер введён в гигабайтах, переведём его в байты.
          // !!! Проверки на превышение максимального значения не делаются !!!
          sizeabove=sizeabove*_i64(1024)*_i64(1024)*_i64(1024);
          break;
        default:
          break;
      }

      if (//sizeabove < 0 &&        // Есть введённый пользователем минимальный размер файла?      "!= 0" ???
          fsize < sizeabove)       // Размер файла меньше минимального разрешённого по фильтру?
         return FALSE;             // Не пропускаем этот файл
    }

    if (sizebelow != (unsigned __int64)-1)
    {
      switch (FF.FSize.SizeType)
      {
        case FSIZE_INBYTES:
          // Размер введён в байтах, значит ничего не меняем.
          break;
        case FSIZE_INKBYTES:
          // Размер введён в килобайтах, переведём его в байты.
          // !!! Проверки на превышение максимального значения не делаются !!!
          sizebelow=sizebelow*_i64(1024);
          break;
        case FSIZE_INMBYTES:
          // Задел // Размер введён в мегабайтах, переведём его в байты.
          // !!! Проверки на превышение максимального значения не делаются !!!
          sizebelow=sizebelow*_i64(1024)*_i64(1024);
          break;
        case FSIZE_INGBYTES:
          // Задел // Размер введён в гигабайтах, переведём его в байты.
          // !!! Проверки на превышение максимального значения не делаются !!!
          sizebelow=sizebelow*_i64(1024)*_i64(1024)*_i64(1024);
          break;
        default:
          break;
      }

      if (//sizebelow < 0 &&        // Есть введённый пользователем максимальный размер файла?     "!= 0" ???
          fsize > sizebelow)       // Размер файла больше максимального разрешённого по фильтру?
         return FALSE;             // Не пропускаем этот файл
    }
  }

  // Режим проверки времени файла включен?
  if (FF.FDate.Used)
  {
    // Преобразуем FILETIME в беззнаковый __int64
    unsigned __int64 &after=(unsigned __int64 &)FF.FDate.DateAfter;
    unsigned __int64 &before=(unsigned __int64 &)FF.FDate.DateBefore;

    if (after!=0 || before!=0)
    {
      unsigned __int64 ftime=0;

      switch (FF.FDate.DateType)
      {
        case FDATE_MODIFIED:
          (unsigned __int64 &)ftime=(unsigned __int64 &)fd->ftLastWriteTime;
          break;
        case FDATE_CREATED:
          (unsigned __int64 &)ftime=(unsigned __int64 &)fd->ftCreationTime;
          break;
        case FDATE_OPENED:
          (unsigned __int64 &)ftime=(unsigned __int64 &)fd->ftLastAccessTime;
          break;
      }

      // Есть введённая пользователем начальная дата?
      if (after!=0)
        // Дата файла меньше начальной даты по фильтру?
        if (ftime<after)
          // Не пропускаем этот файл
          return FALSE;

      // Есть введённая пользователем конечная дата?
      if (before!=0)
        // Дата файла больше конечной даты по фильтру?
        if (ftime>before)
          return FALSE;
    }
  }

  // Режим проверки атрибутов файла включен?
  if (FF.FAttr.Used)
  {
    DWORD AttrSet=FF.FAttr.AttrSet;
    DWORD AttrClear=FF.FAttr.AttrClear;
    if (DisableDir)
    {
      AttrClear&=~FILE_ATTRIBUTE_DIRECTORY;
      AttrSet&=~FILE_ATTRIBUTE_DIRECTORY;
    }

    // Проверка попадания файла по установленным атрибутам
    if ((fd->dwFileAttributes & AttrSet) != AttrSet)
      return FALSE;

    // Проверка попадания файла по отсутствующим атрибутам
    if (fd->dwFileAttributes & AttrClear)
      return FALSE;
  }

  // Да! Файл выдержал все испытания и будет допущен к использованию
  // в вызвавшей эту функцию операции.
  return TRUE;
}
