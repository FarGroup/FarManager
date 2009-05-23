/*
setattr.cpp

Установка атрибутов файлов
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

#include "flink.hpp"
#include "lang.hpp"
#include "dialog.hpp"
#include "chgprior.hpp"
#include "scantree.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "constitle.hpp"
#include "TPreRedrawFunc.hpp"
#include "imports.hpp"
#include "taskbar.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "config.hpp"
#include "strftime.hpp"
#include "fileattr.hpp"
#include "setattr.hpp"

#define DM_SETATTR      (DM_USER+1)

enum {
  SETATTR_TITLE=0,

  SETATTR_NAME=2,

  SETATTR_ATTR_FIRST=4,  // This should be equal to the next value.
  SETATTR_RO=4,
  SETATTR_ARCHIVE=5,
  SETATTR_HIDDEN=6,
  SETATTR_SYSTEM=7,
  SETATTR_COMPRESSED=8,
  SETATTR_ENCRYPTED=9,
  SETATTR_INDEXED=10,
  SETATTR_SPARSE=11,
  SETATTR_TEMP=12,
  SETATTR_OFFLINE=13,
  SETATTR_VIRTUAL=14,
  SETATTR_ATTR_LAST=14,  // This should be equal to the previous value.

  SETATTR_SUBFOLDERS=16,

  SETATTR_TITLEDATE=18,
  SETATTR_MODIFICATION=19,
  SETATTR_MDATE=20,
  SETATTR_MTIME=21,
  SETATTR_CREATION=22,
  SETATTR_CDATE=23,
  SETATTR_CTIME=24,
  SETATTR_LASTACCESS=25,
  SETATTR_ADATE=26,
  SETATTR_ATIME=27,
  SETATTR_ORIGINAL=28,
  SETATTR_CURRENT=29,
  SETATTR_BLANK=30,

  SETATTR_SET=32,
  SETATTR_CANCEL=33,
	SETATTR_NAMECOMBO=34,
	SETATTR_TITLELINK=35,
};

const wchar_t FmtMask1[]=L"99%c99%c99";
const wchar_t FmtMask2[]=L"99%c99%c9999";
const wchar_t FmtMask3[]=L"9999%c99%c99";

struct SetAttrDlgParam{
  BOOL  Plugin;
  DWORD FileSystemFlags;
  int ModeDialog;
  int DateFormat;
  string strSelName;
  int OriginalCBAttr[16]; // значения CheckBox`ов на момент старта диалога
  int OriginalCBAttr2[16]; //
  DWORD OriginalCBFlag[16];
	int OSubfoldersState, OCompressState, OEncryptState;
  int OLastWriteTime,OCreationTime,OLastAccessTime;

  void Clear()
  {
  	Plugin = 0;
  	FileSystemFlags = 0;
  	ModeDialog = 0;
  	DateFormat = 0;
  	strSelName = L"";
  	memset(OriginalCBAttr,0,sizeof(OriginalCBAttr));
  	memset(OriginalCBAttr2,0,sizeof(OriginalCBAttr2));
  	memset(OriginalCBFlag,0,sizeof(OriginalCBFlag));
		OSubfoldersState = 0;
		OCompressState = 0;
		OEncryptState = 0;
  	OLastWriteTime = 0;
  	OCreationTime = 0;
  	OLastAccessTime = 0;
  }
};

static int IsFileWritable(const wchar_t *Name, DWORD FileAttr, BOOL IsShowErrMsg, int Msg, int SkipMode);
static int ReadFileTime(int Type,const wchar_t *Name,DWORD FileAttr,FILETIME *FileTime,
                       const wchar_t *OSrcDate, const wchar_t *OSrcTime);
static void PR_ShellSetFileAttributesMsg(void);
void ShellSetFileAttributesMsg(const wchar_t *Name);

// обработчик диалога - пока это отлов нажатий нужных кнопок.
LONG_PTR WINAPI SetAttrDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  int FocusPos,I;
	int CompressState, EncryptState, SubfoldersState;
  struct SetAttrDlgParam *DlgParam;

  DlgParam=(struct SetAttrDlgParam *)Dialog::SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
  switch(Msg)
  {
    case DN_BTNCLICK:
      if((Param1 >= SETATTR_ATTR_FIRST && Param1 <= SETATTR_ATTR_LAST) || Param1 == SETATTR_SUBFOLDERS)
      {
        DlgParam->OriginalCBAttr[Param1-SETATTR_ATTR_FIRST] = (int)Param2;
        DlgParam->OriginalCBAttr2[Param1-SETATTR_ATTR_FIRST] = 0;

        FocusPos=(int)Dialog::SendDlgMessage(hDlg,DM_GETFOCUS,0,0);
				CompressState=(int)Dialog::SendDlgMessage(hDlg,DM_GETCHECK,SETATTR_COMPRESSED,0);
				EncryptState=(int)Dialog::SendDlgMessage(hDlg,DM_GETCHECK,SETATTR_ENCRYPTED,0);
				SubfoldersState=(int)Dialog::SendDlgMessage(hDlg,DM_GETCHECK,SETATTR_SUBFOLDERS,0);

        if(!DlgParam->ModeDialog) // =0 - одиночный
        {
					if(((DlgParam->FileSystemFlags & (FILE_FILE_COMPRESSION|FILE_SUPPORTS_ENCRYPTION))==
							(FILE_FILE_COMPRESSION|FILE_SUPPORTS_ENCRYPTION)) &&
             (FocusPos == SETATTR_COMPRESSED || FocusPos == SETATTR_ENCRYPTED))
          {
							if(FocusPos == SETATTR_COMPRESSED && /*CompressState &&*/ EncryptState)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_ENCRYPTED,BSTATE_UNCHECKED);
							if(FocusPos == SETATTR_ENCRYPTED && /*EncryptState &&*/ CompressState)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_COMPRESSED,BSTATE_UNCHECKED);
          }
        }
        else // =1|2 Multi
        {
          // отработаем взаимоисключения
					if(((DlgParam->FileSystemFlags & (FILE_FILE_COMPRESSION|FILE_SUPPORTS_ENCRYPTION))==
							(FILE_FILE_COMPRESSION|FILE_SUPPORTS_ENCRYPTION)) &&
             (FocusPos == SETATTR_COMPRESSED || FocusPos == SETATTR_ENCRYPTED))
          {
						if(FocusPos == SETATTR_COMPRESSED && DlgParam->OCompressState != CompressState) // Состояние изменилось?
            {
							if(CompressState == BSTATE_CHECKED && EncryptState)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_ENCRYPTED,BSTATE_UNCHECKED);
							else if(CompressState == BSTATE_3STATE)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_ENCRYPTED,BSTATE_3STATE);
            }
						else if(FocusPos == SETATTR_ENCRYPTED && DlgParam->OEncryptState != EncryptState) // Состояние изменилось?
            {
							if(EncryptState == BSTATE_CHECKED && CompressState)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_COMPRESSED,BSTATE_UNCHECKED);
							else if(EncryptState == BSTATE_3STATE)
                Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_COMPRESSED,BSTATE_3STATE);
            }

            // еще одна проверка
						if(FocusPos == SETATTR_COMPRESSED && /* DlgParam->OCompressState && */ EncryptState)
              Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_ENCRYPTED,BSTATE_UNCHECKED);
						if(FocusPos == SETATTR_ENCRYPTED && /* DlgParam->OEncryptState && */ CompressState)
              Dialog::SendDlgMessage(hDlg,DM_SETCHECK,SETATTR_COMPRESSED,BSTATE_UNCHECKED);

						DlgParam->OEncryptState=EncryptState;
						DlgParam->OCompressState=CompressState;
          }

          // если снимаем атрибуты для SubFolders
          // этот кусок всегда работает если есть хотя бы одна папка
          // иначе 12-й недоступен и всегда снят.
          if(FocusPos == SETATTR_SUBFOLDERS)
          {
            if(DlgParam->ModeDialog==1) // каталог однозначно!
            {
							if(DlgParam->OSubfoldersState != SubfoldersState) // Состояние изменилось?
              {
                // убираем 3-State
                for(I=SETATTR_ATTR_FIRST; I <= SETATTR_ATTR_LAST; ++I)
                {
									if(!SubfoldersState) // сняли?
                  {
                    Dialog::SendDlgMessage(hDlg,DM_SET3STATE,I,FALSE);
                    Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,DlgParam->OriginalCBAttr[I-SETATTR_ATTR_FIRST]);
                  }
                  else                      // установили?
                  {
                    Dialog::SendDlgMessage(hDlg,DM_SET3STATE,I,TRUE);
                    if(DlgParam->OriginalCBAttr2[I-SETATTR_ATTR_FIRST] == -1)
                      Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,BSTATE_3STATE);
                  }
                }
                if(Opt.SetAttrFolderRules)
                {
                  FAR_FIND_DATA_EX FindData;

                  if ( apiGetFindDataEx (DlgParam->strSelName,&FindData) )
                  {
										if(!SubfoldersState)
                    {
                      if(!DlgParam->OLastWriteTime)
                      {
                        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_MODIFICATION,(LONG_PTR)&FindData.ftLastWriteTime);
                        DlgParam->OLastWriteTime=0;
                      }
                      if(!DlgParam->OCreationTime)
                      {
                        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_CREATION,(LONG_PTR)&FindData.ftCreationTime);
                        DlgParam->OCreationTime=0;
                      }
                      if(!DlgParam->OLastAccessTime)
                      {
                        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_LASTACCESS,(LONG_PTR)&FindData.ftLastAccessTime);
                        DlgParam->OLastAccessTime=0;
                      }
                    }
                    else
                    {
                      if(!DlgParam->OLastWriteTime)
                      {
                        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_MODIFICATION,0);
                        DlgParam->OLastWriteTime=0;
                      }
                      if(!DlgParam->OCreationTime)
                      {
                        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_CREATION,0);
                        DlgParam->OCreationTime=0;
                      }
                      if(!DlgParam->OLastAccessTime)
                      {
                        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_LASTACCESS,0);
                        DlgParam->OLastAccessTime=0;
                      }
                    }
                  }

                }
              }
            }
            else  // много объектов
            {
							if(DlgParam->OSubfoldersState != SubfoldersState) // Состояние изменилось?
              {
                for(I=SETATTR_ATTR_FIRST; I <= SETATTR_ATTR_LAST; ++I)
                {
									if(!SubfoldersState) // сняли?
                  {
                    Dialog::SendDlgMessage(hDlg,DM_SET3STATE,I,
                              ((DlgParam->OriginalCBFlag[I-SETATTR_ATTR_FIRST]&DIF_3STATE)?TRUE:FALSE));
                    Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,DlgParam->OriginalCBAttr[I-SETATTR_ATTR_FIRST]);
                  }
                  else                      // установили?
                  {
                    if(DlgParam->OriginalCBAttr2[I-SETATTR_ATTR_FIRST] == -1)
                    {
                      Dialog::SendDlgMessage(hDlg,DM_SET3STATE,I,TRUE);
                      Dialog::SendDlgMessage(hDlg,DM_SETCHECK,I,BSTATE_3STATE);
                    }
                  }
                }
              }
            }
						DlgParam->OSubfoldersState=SubfoldersState;
          }
        }
        return TRUE;
      }
      // Set Original? / Set All? / Clear All?
      else if(Param1 == SETATTR_ORIGINAL)
      {
        FAR_FIND_DATA_EX FindData;
        DlgParam=(struct SetAttrDlgParam *)Dialog::SendDlgMessage(hDlg,DM_GETDLGDATA,0,0);
        if ( apiGetFindDataEx (DlgParam->strSelName,&FindData) )
        {
          Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_MODIFICATION,(LONG_PTR)&FindData.ftLastWriteTime);
          Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_CREATION,(LONG_PTR)&FindData.ftCreationTime);
          Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_LASTACCESS,(LONG_PTR)&FindData.ftLastAccessTime);
          DlgParam->OLastWriteTime=DlgParam->OCreationTime=DlgParam->OLastAccessTime=0;
        }
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,SETATTR_MDATE,0);
        return TRUE;
      }
      else if(Param1 == SETATTR_CURRENT || Param1 == SETATTR_BLANK)
      {
        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_MODIFICATION,Param1 == SETATTR_CURRENT?-1:0);
        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_CREATION,Param1 == SETATTR_CURRENT?-1:0);
        Dialog::SendDlgMessage(hDlg,DM_SETATTR,SETATTR_LASTACCESS,Param1 == SETATTR_CURRENT?-1:0);
        DlgParam->OLastWriteTime=DlgParam->OCreationTime=DlgParam->OLastAccessTime=1;
        Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,SETATTR_MDATE,0);
        return TRUE;
      }
      break;

    case DN_MOUSECLICK:
     {
       //_SVS(SysLog(L"Msg=DN_MOUSECLICK Param1=%d Param2=%d",Param1,Param2));
       if(Param1 >= SETATTR_MODIFICATION && Param1 <= SETATTR_ATIME)
       {
         if(((MOUSE_EVENT_RECORD*)Param2)->dwEventFlags == DOUBLE_CLICK)
         {
           // Дадим Менеджеру диалогов "попотеть"
           Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
           Dialog::SendDlgMessage(hDlg,DM_SETATTR,Param1,-1);
         }
         if(Param1 == SETATTR_MODIFICATION || Param1 == SETATTR_CREATION || Param1 == SETATTR_LASTACCESS)
           Param1++;
         Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,Param1,0);
         return TRUE;
       }
     }
     break;

    case DN_EDITCHANGE:
    {
			switch (Param1)
			{
			case SETATTR_NAMECOMBO:
				{
					FarListInfo li;
					Dialog::SendDlgMessage(hDlg,DM_LISTINFO,SETATTR_NAMECOMBO,(LONG_PTR)&li);
					string strTmp;
					strTmp.Format(MSG(MSetAttrHardLinks),li.ItemsNumber);
					Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,SETATTR_NAMECOMBO,(LONG_PTR)(const wchar_t*)strTmp);
				}
				break;
			case SETATTR_MDATE:
			case SETATTR_MTIME:
				DlgParam->OLastWriteTime=1;
				break;
			case SETATTR_CDATE:
			case SETATTR_CTIME:
				DlgParam->OCreationTime=1;
				break;
			case SETATTR_ADATE:
			case SETATTR_ATIME:
				DlgParam->OLastAccessTime=1;
				break;
			}
      break;
    }

    case DM_SETATTR:
      {
        FILETIME ft;
        string strDate, strTime;
        int Set1, Set2;
        if(Param2) // Set?
        {
          if(Param2==-1)
            GetSystemTimeAsFileTime(&ft);
          else
            ft=*(FILETIME *)Param2;
          ConvertDate(ft,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
        }
        else if(!Param2) // Clear
        {
            strDate=strTime=L"";
        }

        // Глянем на место, где был клик
             if(Param1 == SETATTR_MODIFICATION) { Set1=SETATTR_MDATE; Set2=SETATTR_MTIME; DlgParam->OLastWriteTime=1;}
        else if(Param1 == SETATTR_CREATION) { Set1=SETATTR_CDATE; Set2=SETATTR_CTIME; DlgParam->OCreationTime=1;}
        else if(Param1 == SETATTR_LASTACCESS) { Set1=SETATTR_ADATE; Set2=SETATTR_ATIME; DlgParam->OLastAccessTime=1;}
        else if(Param1 == SETATTR_MDATE || Param1 == SETATTR_CDATE || Param1 == SETATTR_ADATE) { Set1=Param1; Set2=-1; }
        else { Set1=-1; Set2=Param1; }

        if(Set1 != -1)
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,Set1,(LONG_PTR)(const wchar_t*)strDate);
        if(Set2 != -1)
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,Set2,(LONG_PTR)(const wchar_t*)strTime);
        return TRUE;
      }
  }
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

static void PR_ShellSetFileAttributesMsg(void)
{
  PreRedrawItem preRedrawItem=PreRedraw.Peek();
  ShellSetFileAttributesMsg((wchar_t *)preRedrawItem.Param.Param1);
}

void ShellSetFileAttributesMsg(const wchar_t *Name)
{
  static int Width=54;
  int WidthTemp;
  string strOutFileName;

  if(Name && *Name)
    WidthTemp=Max(StrLength(Name),(int)54);
  else
    Width=WidthTemp=54;

  if(WidthTemp > WidthNameForMessage)
    WidthTemp=WidthNameForMessage; // ширина месага - 38%

  if(Width < WidthTemp)
    Width=WidthTemp;

  if(Name && *Name)
  {
    strOutFileName = Name;
    TruncPathStr(strOutFileName,Width);
    CenterStr(strOutFileName,strOutFileName,Width+4);
  }
  else
  {
    strOutFileName=L"";
    CenterStr(strOutFileName,strOutFileName,Width+4); // подготавливаем нужную ширину (вид!)
  }
  Message(0,0,MSG(MSetAttrTitle),MSG(MSetAttrSetting),(const wchar_t*)strOutFileName);

  PreRedrawItem preRedrawItem=PreRedraw.Peek();
  preRedrawItem.Param.Param1=(void*)Name;
  PreRedraw.SetParam(preRedrawItem.Param);
}

int ShellSetFileAttributes(Panel *SrcPanel)
{
  int SkipMode=-1;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

/*MSetAttrJunction
00                                             00
01   +------------ Attributes -------------+   01
02   |     Change file attributes for      |   02
03   |                 foo                 |   03
04   |          Link: blach blach          | < 04 <<
05   +-------------------------------------+   05
06   | [ ] Read only        [ ] Sparse     |   06
07   | [ ] Archive          [ ] Temporary  |   07
08   | [ ] Hidden           [ ] Offline    |   08
09   | [ ] System           [ ] Virtual    |   09
10   | [ ] Compressed                      |   10
11   | [ ] Encrypted                       |   11
12   | [ ] Not Indexed                     |   12
13   +-------------------------------------+   13
14   | [x] Process subfolders              |   14
15   +-------------------------------------+   15
16   |  File time      DD.MM.YYYY hh:mm:ss |   16
17   | Modification      .  .       :  :   |   17
18   | Creation          .  .       :  :   |   18
19   | Last access       .  .       :  :   |   19
20   | [ Original ]  [ Current ] [ Blank ] |   10
21   +-------------------------------------+   21
22   |         [ Set ]  [ Cancel ]         |   22
23   +-------------------------------------+   23
24                                             24
*/
  static struct DialogDataEx AttrDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,65,21,0,0,0,0,(wchar_t *)MSetAttrTitle,
  /* 01 */DI_TEXT,-1,2,0,2,0,0,0,0,(wchar_t *)MSetAttrFor,
  /* 02 */DI_TEXT,-1,3,0,3,0,0,DIF_SHOWAMPERSAND,0,L"",
  /* 03 */DI_TEXT,3,4,0,4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",

  /* 04 */DI_CHECKBOX,5, 5,0,5,1,0,DIF_3STATE,0,(wchar_t *)MSetAttrRO,
  /* 05 */DI_CHECKBOX,5, 6,0,6,0,0,DIF_3STATE,0,(wchar_t *)MSetAttrArchive,
  /* 06 */DI_CHECKBOX,5, 7,0,7,0,0,DIF_3STATE,0,(wchar_t *)MSetAttrHidden,
  /* 07 */DI_CHECKBOX,5, 8,0,8,0,0,DIF_3STATE,0,(wchar_t *)MSetAttrSystem,
  /* 08 */DI_CHECKBOX,5, 9,0,9,0,0,DIF_3STATE,0,(wchar_t *)MSetAttrCompressed,
  /* 09 */DI_CHECKBOX,5,10,0,10,0,0,DIF_3STATE,0,(wchar_t *)MSetAttrEncrypted,

  /* 10 */DI_CHECKBOX,35,5,0,5,0,0,DIF_3STATE,0,(wchar_t *)MSetAttrNotIndexed,
	/* 11 */DI_CHECKBOX,35,6,0,6,0,0,DIF_3STATE,0,(wchar_t *)MSetAttrSparse,
	/* 12 */DI_CHECKBOX,35,7,0,7,0,0,DIF_3STATE,0,(wchar_t *)MSetAttrTemp,
	/* 13 */DI_CHECKBOX,35,8,0,8,0,0,DIF_3STATE,0,(wchar_t *)MSetAttrOffline,
  /* 14 */DI_CHECKBOX,35,9,0,9,0,0,DIF_3STATE|DIF_DISABLE,0,(wchar_t *)MSetAttrVirtual,

  /* 15 */DI_TEXT,3,11,0,11,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 16 */DI_CHECKBOX,5,12,0,12,0,0,DIF_DISABLE,0,(wchar_t *)MSetAttrSubfolders,
  /* 17 */DI_TEXT,3,13,0,13,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 18 */DI_TEXT,45,14,0,14,0,0,0,0,L"",
  /* 19 */DI_TEXT,    5,15,0,15,0,0,0,0,(wchar_t *)MSetAttrModification,
  /* 20 */DI_FIXEDIT,45,15,54,15,0,0,DIF_MASKEDIT,0,L"",
  /* 21 */DI_FIXEDIT,56,15,63,15,0,0,DIF_MASKEDIT,0,L"",
  /* 22 */DI_TEXT,    5,16,0,16,0,0,0,0,(wchar_t *)MSetAttrCreation,
  /* 23 */DI_FIXEDIT,45,16,54,16,0,0,DIF_MASKEDIT,0,L"",
  /* 24 */DI_FIXEDIT,56,16,63,16,0,0,DIF_MASKEDIT,0,L"",
  /* 25 */DI_TEXT,    5,17,0,17,0,0,0,0,(wchar_t *)MSetAttrLastAccess,
  /* 26 */DI_FIXEDIT,45,17,54,17,0,0,DIF_MASKEDIT,0,L"",
  /* 27 */DI_FIXEDIT,56,17,63,17,0,0,DIF_MASKEDIT,0,L"",
  /* 28 */DI_BUTTON,0,18,0,18,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(wchar_t *)MSetAttrOriginal,
  /* 29 */DI_BUTTON,0,18,0,18,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(wchar_t *)MSetAttrCurrent,
  /* 30 */DI_BUTTON,0,18,0,18,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,(wchar_t *)MSetAttrBlank,
  /* 31 */DI_TEXT,3,19,0,19,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 32 */DI_BUTTON,0,20,0,20,0,0,DIF_CENTERGROUP,1,(wchar_t *)MSetAttrSet,
  /* 33 */DI_BUTTON,0,20,0,20,0,0,DIF_CENTERGROUP,0,(wchar_t *)MCancel,
	/* 34 */DI_COMBOBOX,5,3,63,3,0,0,DIF_SHOWAMPERSAND|DIF_DROPDOWNLIST|DIF_LISTWRAPMODE|DIF_HIDDEN,0,L"",
	/* 35 */DI_TEXT,-1,4,0,4,0,0,DIF_SHOWAMPERSAND,0,L"",
  };
  MakeDialogItemsEx(AttrDlgData,AttrDlg);
	int DlgCountItems=countof(AttrDlgData)-1;
	FarList NameList={0,NULL};
	string *strLinks=NULL;

  int SelCount, I, J;
  struct SetAttrDlgParam DlgParam;
  DlgParam.Clear();

  if ((SelCount=SrcPanel->GetSelCount())==0)
    return 0;

  if (SrcPanel->GetMode()==PLUGIN_PANEL)
  {
    struct OpenPluginInfo Info;
    HANDLE hPlugin=SrcPanel->GetPluginHandle();
    if(hPlugin == INVALID_HANDLE_VALUE)
      return 0;

    CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);
    if(!(Info.Flags & OPIF_REALNAMES))
    {
      AttrDlg[SETATTR_SET].Flags|=DIF_DISABLE;
      DlgParam.Plugin=TRUE;
    }
  }

  DlgParam.FileSystemFlags=0;

  if (DlgParam.Plugin)
  {
    AttrDlg[SETATTR_COMPRESSED].Flags|=DIF_DISABLE;
    AttrDlg[SETATTR_ENCRYPTED].Flags|=DIF_DISABLE;
  }
  else
  {
		string strRootPathName;
    apiGetCurrentDirectory(strRootPathName);
    GetPathRoot(strRootPathName,strRootPathName);
		if (apiGetVolumeInformation(strRootPathName,NULL,0,NULL,&DlgParam.FileSystemFlags,NULL))
    {
			if (!(DlgParam.FileSystemFlags & FILE_FILE_COMPRESSION))
        AttrDlg[SETATTR_COMPRESSED].Flags|=DIF_DISABLE;

			if (!(DlgParam.FileSystemFlags & FILE_SUPPORTS_ENCRYPTION))
        AttrDlg[SETATTR_ENCRYPTED].Flags|=DIF_DISABLE;

			if(!(DlgParam.FileSystemFlags & FILE_SUPPORTS_SPARSE_FILES))
				AttrDlg[SETATTR_SPARSE].Flags|=DIF_DISABLE;
    }
  }

  {
    string strSelName;
    DWORD FileAttr;
    FILETIME LastWriteTime,CreationTime,LastAccessTime;
    int SetWriteTime,SetCreationTime,SetLastAccessTime;
    int SetWriteTimeRetCode=TRUE;
    FAR_FIND_DATA_EX FindData;

    //SaveScreen SaveScr;

    SrcPanel->GetSelName(NULL,FileAttr);
    SrcPanel->GetSelName(&strSelName,FileAttr,NULL,&FindData);
    if (SelCount==0 || (SelCount==1 && TestParentFolderName(strSelName)))
      return 0;

//    int NewAttr;
    int FolderPresent=FALSE, JunctionPresent=FALSE;

    int DateSeparator=GetDateSeparator();
    int TimeSeparator=GetTimeSeparator();
    string strDMask, strTMask;

    strTMask.Format (FmtMask1,TimeSeparator,TimeSeparator);

    string strAttr;

    switch (DlgParam.DateFormat=GetDateFormat())
    {
      case 0:
        strAttr = AttrDlg[SETATTR_TITLEDATE].strData;
        strAttr.Format (MSG(MSetAttrTimeTitle1),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator);
        strDMask.Format (FmtMask2,DateSeparator,DateSeparator);
        break;
      case 1:
        strAttr = AttrDlg[SETATTR_TITLEDATE].strData;
        strAttr.Format (MSG(MSetAttrTimeTitle2),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator);
        strDMask.Format (FmtMask2,DateSeparator,DateSeparator);
        break;
      default:
        strAttr = AttrDlg[SETATTR_TITLEDATE].strData;
        strAttr.Format (MSG(MSetAttrTimeTitle3),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator);
        strDMask.Format (FmtMask3,DateSeparator,DateSeparator);
        break;
    }

    AttrDlg[SETATTR_TITLEDATE].strData = strAttr;
    AttrDlg[SETATTR_MDATE].Mask=strDMask;
    AttrDlg[SETATTR_MTIME].Mask=strTMask;
    AttrDlg[SETATTR_CDATE].Mask=strDMask;
    AttrDlg[SETATTR_CTIME].Mask=strTMask;
    AttrDlg[SETATTR_ADATE].Mask=strDMask;
    AttrDlg[SETATTR_ATIME].Mask=strTMask;

    if (SelCount==1)
    {
      if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY))
      {
				if (!DlgParam.Plugin)
        {
					FileAttr=apiGetFileAttributes(strSelName);
        }
        //_SVS(SysLog(L"SelName=%s  FileAttr=0x%08X",SelName,FileAttr));
        AttrDlg[SETATTR_SUBFOLDERS].Flags&=~DIF_DISABLE;
        AttrDlg[SETATTR_SUBFOLDERS].Selected=Opt.SetAttrFolderRules == 1?0:1;
        if (Opt.SetAttrFolderRules)
        {
          if ( apiGetFindDataEx (strSelName,&FindData) )
          {
            ConvertDate(FindData.ftLastWriteTime, AttrDlg[SETATTR_MDATE].strData,AttrDlg[SETATTR_MTIME].strData,8,FALSE,FALSE,TRUE,TRUE);
            ConvertDate(FindData.ftCreationTime,  AttrDlg[SETATTR_CDATE].strData,AttrDlg[SETATTR_CTIME].strData,8,FALSE,FALSE,TRUE,TRUE);
            ConvertDate(FindData.ftLastAccessTime,AttrDlg[SETATTR_ADATE].strData,AttrDlg[SETATTR_ATIME].strData,8,FALSE,FALSE,TRUE,TRUE);
          }
					if(FileAttr!=INVALID_FILE_ATTRIBUTES)
					{
						AttrDlg[SETATTR_RO].Selected=(FileAttr & FILE_ATTRIBUTE_READONLY)!=0;
						AttrDlg[SETATTR_ARCHIVE].Selected=(FileAttr & FILE_ATTRIBUTE_ARCHIVE)!=0;
						AttrDlg[SETATTR_HIDDEN].Selected=(FileAttr & FILE_ATTRIBUTE_HIDDEN)!=0;
						AttrDlg[SETATTR_SYSTEM].Selected=(FileAttr & FILE_ATTRIBUTE_SYSTEM)!=0;
						AttrDlg[SETATTR_COMPRESSED].Selected=(FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0;
						AttrDlg[SETATTR_ENCRYPTED].Selected=(FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0;
						AttrDlg[SETATTR_INDEXED].Selected=(FileAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)!=0;
						AttrDlg[SETATTR_SPARSE].Selected=(FileAttr & FILE_ATTRIBUTE_SPARSE_FILE)!=0;
						AttrDlg[SETATTR_TEMP].Selected=(FileAttr & FILE_ATTRIBUTE_TEMPORARY)!=0;
						AttrDlg[SETATTR_OFFLINE].Selected=(FileAttr & FILE_ATTRIBUTE_OFFLINE)!=0;
						AttrDlg[SETATTR_VIRTUAL].Selected=(FileAttr & FILE_ATTRIBUTE_VIRTUAL)!=0;
					}
          // убираем 3-State
          for (I=SETATTR_ATTR_FIRST; I <= SETATTR_ATTR_LAST; ++I)
            AttrDlg[I].Flags&=~DIF_3STATE;
        }
        FolderPresent=TRUE;
      }
      else
      {
        // убираем 3-State
				for (I=SETATTR_ATTR_FIRST; I <= SETATTR_ATTR_LAST; ++I)
          AttrDlg[I].Flags&=~DIF_3STATE;
      }

      // обработка случая, если ЭТО SymLink
      if (FileAttr!=INVALID_FILE_ATTRIBUTES && FileAttr&FILE_ATTRIBUTE_REPARSE_POINT)
      {
        string strJuncName;
        DWORD ReparseTag=0;
        DWORD LenJunction=GetReparsePointInfo(strSelName, strJuncName,&ReparseTag);

        AttrDlg[SETATTR_TITLE].Y2++;
        for (I=3; I < DlgCountItems; ++I)
        {
          AttrDlg[I].Y1++;
          if (AttrDlg[I].Y2)
            AttrDlg[I].Y2++;
        }
        DlgCountItems++;
        JunctionPresent=TRUE;

        int ID_Msg;
        if (ReparseTag==IO_REPARSE_TAG_MOUNT_POINT)
        {
          if (IsLocalVolumePath(strJuncName) && !strJuncName.At(49))
          {
            //"\??\\\?\Volume{..."
            strJuncName.LShift(4);

            string strJuncRoot;
            GetPathRootOne(strJuncName,strJuncRoot);

            if (strJuncRoot.At(1) == L':')
              strJuncName = strJuncRoot;

            ID_Msg=MSetAttrVolMount;
          }
          else
          {
            ID_Msg=MSetAttrJunction;
          }
        }
        else
        {
          ID_Msg=MSetAttrSymlink;
        }

        //"\??\D:\Junc\Src\"
        if (!StrCmpN(strJuncName,L"\\??\\",4))
          strJuncName.LShift(4);

        AttrDlg[SETATTR_TITLELINK].strData.Format (MSG(ID_Msg),
               (LenJunction?
                 (const wchar_t *)TruncPathStr(strJuncName,AttrDlg[SETATTR_TITLE].X2-AttrDlg[SETATTR_TITLE].X1-1-StrLength(MSG(ID_Msg))):
                 MSG(MSetAttrUnknownJunction)));

        /* $ 11.09.2001 SVS
           Уточнение по поводу слинкованной файловой системы отличной от
           NTFS.
        */
        DlgParam.FileSystemFlags=0;
        GetPathRoot(strSelName,strJuncName);
        if (apiGetVolumeInformation (strJuncName,NULL,0,NULL,&DlgParam.FileSystemFlags,NULL))
        {
					if (!(DlgParam.FileSystemFlags & FILE_FILE_COMPRESSION))
            AttrDlg[SETATTR_COMPRESSED].Flags|=DIF_DISABLE;

					if (!(DlgParam.FileSystemFlags & FILE_SUPPORTS_ENCRYPTION))
            AttrDlg[SETATTR_ENCRYPTED].Flags|=DIF_DISABLE;

					if(!(DlgParam.FileSystemFlags & FILE_SUPPORTS_SPARSE_FILES))
						AttrDlg[SETATTR_SPARSE].Flags|=DIF_DISABLE;
        }
        /* SVS $ */
      }

			// обработка случая "несколько хардлинков"
			NameList.ItemsNumber=GetNumberOfLinks(strSelName);
			if (NameList.ItemsNumber>1 && ifn.pfnFindFirstFileNameW && ifn.pfnFindNextFileNameW)
			{
				AttrDlg[SETATTR_NAME].Flags|=DIF_HIDDEN;
				AttrDlg[SETATTR_NAMECOMBO].Flags&=~DIF_HIDDEN;
				NameList.Items=new FarListItem[NameList.ItemsNumber];
				memset(NameList.Items,0,sizeof(FarListItem)*NameList.ItemsNumber);
				strLinks=new string[NameList.ItemsNumber];
				HANDLE hFind=apiFindFirstFileName(strSelName,0,strLinks[0]);
				int Current=0;
				if(hFind!=INVALID_HANDLE_VALUE)
				{
					string strRoot;
					GetPathRoot(strSelName,strRoot);
					DeleteEndSlash(strRoot);
					strLinks[0]=strRoot+strLinks[0];
					NameList.Items[Current++].Text=strLinks[0];
					while(apiFindNextFileName(hFind,strLinks[Current]))
					{
						strLinks[Current]=strRoot+strLinks[Current];
						NameList.Items[Current].Text=strLinks[Current];
						Current++;
					}
					FindClose(hFind);
					AttrDlg[SETATTR_NAMECOMBO].strData.Format(MSG(MSetAttrHardLinks),NameList.ItemsNumber);
					AttrDlg[SETATTR_NAMECOMBO].ListItems=&NameList;
				}
			}

      AttrDlg[SETATTR_NAME].strData = strSelName;
      TruncStr(AttrDlg[SETATTR_NAME].strData,54);

			if(FileAttr!=INVALID_FILE_ATTRIBUTES)
			{
				AttrDlg[SETATTR_RO].Selected=(FileAttr & FILE_ATTRIBUTE_READONLY)!=0;
				AttrDlg[SETATTR_ARCHIVE].Selected=(FileAttr & FILE_ATTRIBUTE_ARCHIVE)!=0;
				AttrDlg[SETATTR_HIDDEN].Selected=(FileAttr & FILE_ATTRIBUTE_HIDDEN)!=0;
				AttrDlg[SETATTR_SYSTEM].Selected=(FileAttr & FILE_ATTRIBUTE_SYSTEM)!=0;
				AttrDlg[SETATTR_COMPRESSED].Selected=(FileAttr & FILE_ATTRIBUTE_COMPRESSED)!=0;
				AttrDlg[SETATTR_ENCRYPTED].Selected=(FileAttr & FILE_ATTRIBUTE_ENCRYPTED)!=0;
				AttrDlg[SETATTR_INDEXED].Selected=(FileAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)!=0;
				AttrDlg[SETATTR_SPARSE].Selected=(FileAttr & FILE_ATTRIBUTE_SPARSE_FILE)!=0;
				AttrDlg[SETATTR_TEMP].Selected=(FileAttr & FILE_ATTRIBUTE_TEMPORARY)!=0;
				AttrDlg[SETATTR_OFFLINE].Selected=(FileAttr & FILE_ATTRIBUTE_OFFLINE)!=0;
				AttrDlg[SETATTR_VIRTUAL].Selected=(FileAttr & FILE_ATTRIBUTE_VIRTUAL)!=0;
			}

      if(DlgParam.Plugin)
      {
        ConvertDate(FindData.ftLastWriteTime,AttrDlg[SETATTR_MDATE].strData,AttrDlg[SETATTR_MTIME].strData,8,FALSE,FALSE,TRUE,TRUE);
        ConvertDate(FindData.ftCreationTime,AttrDlg[SETATTR_CDATE].strData,AttrDlg[SETATTR_CTIME].strData,8,FALSE,FALSE,TRUE,TRUE);
        ConvertDate(FindData.ftLastAccessTime,AttrDlg[SETATTR_ADATE].strData,AttrDlg[SETATTR_ATIME].strData,8,FALSE,FALSE,TRUE,TRUE);
      }
      else
      {
        if ( apiGetFindDataEx (strSelName,&FindData))
        {
          ConvertDate(FindData.ftLastWriteTime,AttrDlg[SETATTR_MDATE].strData,AttrDlg[SETATTR_MTIME].strData,8,FALSE,FALSE,TRUE,TRUE);
          ConvertDate(FindData.ftCreationTime,AttrDlg[SETATTR_CDATE].strData,AttrDlg[SETATTR_CTIME].strData,8,FALSE,FALSE,TRUE,TRUE);
          ConvertDate(FindData.ftLastAccessTime,AttrDlg[SETATTR_ADATE].strData,AttrDlg[SETATTR_ATIME].strData,8,FALSE,FALSE,TRUE,TRUE);
        }
      }
    }
    else
    {
      AttrDlg[SETATTR_RO].Selected=AttrDlg[SETATTR_ARCHIVE].Selected=AttrDlg[SETATTR_HIDDEN].Selected=
      AttrDlg[SETATTR_SYSTEM].Selected=AttrDlg[SETATTR_COMPRESSED].Selected=AttrDlg[SETATTR_ENCRYPTED].Selected=
      AttrDlg[SETATTR_INDEXED].Selected=AttrDlg[SETATTR_SPARSE].Selected=AttrDlg[SETATTR_TEMP].Selected=
      AttrDlg[SETATTR_OFFLINE].Selected=AttrDlg[SETATTR_VIRTUAL].Selected=2;

      AttrDlg[SETATTR_MDATE].strData=AttrDlg[SETATTR_MTIME].strData=AttrDlg[SETATTR_CDATE].strData=
      AttrDlg[SETATTR_CTIME].strData=AttrDlg[SETATTR_ADATE].strData=AttrDlg[SETATTR_ATIME].strData=L"";

      AttrDlg[SETATTR_ORIGINAL].Flags|=DIF_HIDDEN;
      AttrDlg[SETATTR_ORIGINAL].Flags&=~DIF_CENTERGROUP;

      AttrDlg[SETATTR_NAME].strData = MSG(MSetAttrSelectedObjects);
      // выставим -1 - потом учтем этот факт :-)
      for(I=SETATTR_ATTR_FIRST; I <= SETATTR_ATTR_LAST; ++I)
        AttrDlg[I].Selected=0;

      // проверка - есть ли среди выделенных - каталоги?
      // так же проверка на атрибуты
      J=0;
      SrcPanel->GetSelName(NULL,FileAttr);
      while (SrcPanel->GetSelName(&strSelName,FileAttr,NULL,&FindData))
      {
        if(!J && (FileAttr & FILE_ATTRIBUTE_DIRECTORY))
        {
          FolderPresent=TRUE;
          AttrDlg[SETATTR_SUBFOLDERS].Flags&=~DIF_DISABLE;
          J++;
        }
        AttrDlg[SETATTR_RO].Selected+=(FileAttr & FILE_ATTRIBUTE_READONLY)?1:0;
        AttrDlg[SETATTR_ARCHIVE].Selected+=(FileAttr & FILE_ATTRIBUTE_ARCHIVE)?1:0;
        AttrDlg[SETATTR_HIDDEN].Selected+=(FileAttr & FILE_ATTRIBUTE_HIDDEN)?1:0;
        AttrDlg[SETATTR_SYSTEM].Selected+=(FileAttr & FILE_ATTRIBUTE_SYSTEM)?1:0;
        AttrDlg[SETATTR_COMPRESSED].Selected+=(FileAttr & FILE_ATTRIBUTE_COMPRESSED)?1:0;
        AttrDlg[SETATTR_ENCRYPTED].Selected+=(FileAttr & FILE_ATTRIBUTE_ENCRYPTED)?1:0;
        AttrDlg[SETATTR_INDEXED].Selected+=(FileAttr & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)?1:0;
        AttrDlg[SETATTR_SPARSE].Selected+=(FileAttr & FILE_ATTRIBUTE_SPARSE_FILE)?1:0;
        AttrDlg[SETATTR_TEMP].Selected+=(FileAttr & FILE_ATTRIBUTE_TEMPORARY)?1:0;
        AttrDlg[SETATTR_OFFLINE].Selected+=(FileAttr & FILE_ATTRIBUTE_OFFLINE)?1:0;
        AttrDlg[SETATTR_VIRTUAL].Selected+=(FileAttr & FILE_ATTRIBUTE_VIRTUAL)?1:0;
      }
      SrcPanel->GetSelName(NULL,FileAttr);
      SrcPanel->GetSelName(&strSelName,FileAttr,NULL,&FindData);
      // выставим "неопределенку" или то, что нужно
      for(I=SETATTR_ATTR_FIRST; I <= SETATTR_ATTR_LAST; ++I)
      {
        J=AttrDlg[I].Selected;
        // снимаем 3-state, если "есть все или нет ничего"
        // за исключением случая, если есть Фолдер среди объектов
        if((!J || J >= SelCount) && !FolderPresent)
          AttrDlg[I].Flags&=~DIF_3STATE;

        AttrDlg[I].Selected=(J >= SelCount)?1:(!J?0:2);
      }
    }

    // поведение для каталогов как у 1.65?
    if(FolderPresent && !Opt.SetAttrFolderRules)
    {
      AttrDlg[SETATTR_SUBFOLDERS].Selected=1;
      AttrDlg[SETATTR_MDATE].strData=AttrDlg[SETATTR_MTIME].strData=AttrDlg[SETATTR_CDATE].strData=
      AttrDlg[SETATTR_CTIME].strData=AttrDlg[SETATTR_ADATE].strData=AttrDlg[SETATTR_ATIME].strData=L"";
      for(I=SETATTR_ATTR_FIRST; I <= SETATTR_ATTR_LAST; ++I)
      {
        AttrDlg[I].Selected=2;
        AttrDlg[I].Flags|=DIF_3STATE;
      }
    }

    // запомним состояние переключателей.
    for(I=SETATTR_ATTR_FIRST; I <= SETATTR_ATTR_LAST; ++I)
    {
      DlgParam.OriginalCBAttr[I-SETATTR_ATTR_FIRST]=AttrDlg[I].Selected;
      DlgParam.OriginalCBAttr2[I-SETATTR_ATTR_FIRST]=-1;
      DlgParam.OriginalCBFlag[I-SETATTR_ATTR_FIRST]=AttrDlg[I].Flags;
    }

    DlgParam.ModeDialog=((SelCount==1 && (FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0)?0:(SelCount==1?1:2));
    DlgParam.strSelName=strSelName;
		DlgParam.OSubfoldersState=AttrDlg[SETATTR_SUBFOLDERS].Selected;
		DlgParam.OCompressState=AttrDlg[SETATTR_COMPRESSED].Selected;
		DlgParam.OEncryptState=AttrDlg[SETATTR_ENCRYPTED].Selected;

    // <Dialog>
    {
      Dialog Dlg(AttrDlg,DlgCountItems,SetAttrDlgProc,(LONG_PTR)&DlgParam);
      Dlg.SetHelp(L"FileAttrDlg");                 //  ^ - это одиночный диалог!
      Dlg.SetPosition(-1,-1,69,JunctionPresent?24:23);
      Dlg.Process();
			if(NameList.Items)
				delete[] NameList.Items;
			if(strLinks)
				delete[] strLinks;
      if (Dlg.GetExitCode()!=SETATTR_SET)
        return 0;
    }
    // </Dialog>
		TPreRedrawFuncGuard preRedrawFuncGuard(PR_ShellSetFileAttributesMsg);
    ShellSetFileAttributesMsg(SelCount==1?(const wchar_t*)strSelName:NULL);

    if (SelCount==1 && (FileAttr & FILE_ATTRIBUTE_DIRECTORY)==0)
    {
      if(IsFileWritable(strSelName,FileAttr,TRUE,MSetAttrCannotFor,SkipMode) == 1)
      {
        int NewAttr;
        NewAttr=FileAttr & FILE_ATTRIBUTE_DIRECTORY;
        if (AttrDlg[SETATTR_RO].Selected)         NewAttr|=FILE_ATTRIBUTE_READONLY;
        if (AttrDlg[SETATTR_ARCHIVE].Selected)    NewAttr|=FILE_ATTRIBUTE_ARCHIVE;
        if (AttrDlg[SETATTR_HIDDEN].Selected)     NewAttr|=FILE_ATTRIBUTE_HIDDEN;
        if (AttrDlg[SETATTR_SYSTEM].Selected)     NewAttr|=FILE_ATTRIBUTE_SYSTEM;
        if (AttrDlg[SETATTR_COMPRESSED].Selected) NewAttr|=FILE_ATTRIBUTE_COMPRESSED;
        if (AttrDlg[SETATTR_ENCRYPTED].Selected)  NewAttr|=FILE_ATTRIBUTE_ENCRYPTED;
        if (AttrDlg[SETATTR_INDEXED].Selected)    NewAttr|=FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
				if (AttrDlg[SETATTR_SPARSE].Selected)     NewAttr|=FILE_ATTRIBUTE_SPARSE_FILE;
				if (AttrDlg[SETATTR_TEMP].Selected)       NewAttr|=FILE_ATTRIBUTE_TEMPORARY;
				if (AttrDlg[SETATTR_OFFLINE].Selected)    NewAttr|=FILE_ATTRIBUTE_OFFLINE;

        /*
        AY: мы с этими атрибутами не работаем
        if(!(AttrDlg[SETATTR_VIRTUAL].Flags&DIF_DISABLE))
          if (AttrDlg[SETATTR_VIRTUAL].Selected)
            NewAttr|=FILE_ATTRIBUTE_VIRTUAL;
        */

        SetWriteTime=     DlgParam.OLastWriteTime  && ReadFileTime(0,strSelName,FileAttr,&LastWriteTime,AttrDlg[SETATTR_MDATE].strData,AttrDlg[SETATTR_MTIME].strData);
        SetCreationTime=  DlgParam.OCreationTime   && ReadFileTime(1,strSelName,FileAttr,&CreationTime,AttrDlg[SETATTR_CDATE].strData,AttrDlg[SETATTR_CTIME].strData);
        SetLastAccessTime=DlgParam.OLastAccessTime && ReadFileTime(2,strSelName,FileAttr,&LastAccessTime,AttrDlg[SETATTR_ADATE].strData,AttrDlg[SETATTR_ATIME].strData);
  //_SVS(SysLog(L"\n\tSetWriteTime=%d\n\tSetCreationTime=%d\n\tSetLastAccessTime=%d",SetWriteTime,SetCreationTime,SetLastAccessTime));
        if(SetWriteTime || SetCreationTime || SetLastAccessTime)
        {
          if(SkipMode!=-1)
            SetWriteTimeRetCode=SkipMode;
          else
            SetWriteTimeRetCode=ESetFileTime(strSelName,
                                           (SetWriteTime ? &LastWriteTime:NULL),
                                           (SetCreationTime ? &CreationTime:NULL),
                                           (SetLastAccessTime ? &LastAccessTime:NULL),
                                           FileAttr,SkipMode);
        }
        else
          SetWriteTimeRetCode=SETATTR_RET_OK;

  //      if(NewAttr != (FileAttr & (~FILE_ATTRIBUTE_DIRECTORY))) // нужно ли что-нить менять???
        if(SetWriteTimeRetCode == SETATTR_RET_OK) // если время удалось выставить...
        {
          int Ret=SETATTR_RET_OK;
          if((NewAttr&FILE_ATTRIBUTE_COMPRESSED) && !(FileAttr&FILE_ATTRIBUTE_COMPRESSED))
            Ret=ESetFileCompression(strSelName,1,FileAttr,SkipMode);
          else if(!(NewAttr&FILE_ATTRIBUTE_COMPRESSED) && (FileAttr&FILE_ATTRIBUTE_COMPRESSED))
            Ret=ESetFileCompression(strSelName,0,FileAttr,SkipMode);

          if((NewAttr&FILE_ATTRIBUTE_ENCRYPTED) && !(FileAttr&FILE_ATTRIBUTE_ENCRYPTED))
            Ret=ESetFileEncryption(strSelName,1,FileAttr,SkipMode);
          else if(!(NewAttr&FILE_ATTRIBUTE_ENCRYPTED) && (FileAttr&FILE_ATTRIBUTE_ENCRYPTED))
            Ret=ESetFileEncryption(strSelName,0,FileAttr,SkipMode);

					if((NewAttr&FILE_ATTRIBUTE_SPARSE_FILE) && !(FileAttr&FILE_ATTRIBUTE_SPARSE_FILE))
					{
						Ret=ESetFileSparse(strSelName,true,FileAttr,SkipMode);
					}
					else if(!(NewAttr&FILE_ATTRIBUTE_SPARSE_FILE) && (FileAttr&FILE_ATTRIBUTE_SPARSE_FILE))
					{
						Ret=ESetFileSparse(strSelName,false,FileAttr,SkipMode);
					}

					Ret=ESetFileAttributes(strSelName,NewAttr&(~(FILE_ATTRIBUTE_ENCRYPTED|FILE_ATTRIBUTE_COMPRESSED|FILE_ATTRIBUTE_SPARSE_FILE)),SkipMode);
          if(Ret==SETATTR_RET_SKIPALL)
            SkipMode=SETATTR_RET_SKIP;
        }
        else if(SetWriteTimeRetCode==SETATTR_RET_SKIPALL)
          SkipMode=SETATTR_RET_SKIP;
      }
    }

    /* Multi *********************************************************** */
    else
    {
      int RetCode=1;
      ConsoleTitle *SetAttrTitle= new ConsoleTitle(MSG(MSetAttrTitle));
      int SetAttr,ClearAttr,Cancel=0;
      CtrlObject->Cp()->GetAnotherPanel(SrcPanel)->CloseFile();

      SetAttr=0;  ClearAttr=0;

      if (AttrDlg[SETATTR_RO].Selected == 1)         SetAttr|=FILE_ATTRIBUTE_READONLY;
      else if (!AttrDlg[SETATTR_RO].Selected)        ClearAttr|=FILE_ATTRIBUTE_READONLY;
      if (AttrDlg[SETATTR_ARCHIVE].Selected == 1)    SetAttr|=FILE_ATTRIBUTE_ARCHIVE;
      else if (!AttrDlg[SETATTR_ARCHIVE].Selected)   ClearAttr|=FILE_ATTRIBUTE_ARCHIVE;
      if (AttrDlg[SETATTR_HIDDEN].Selected == 1)     SetAttr|=FILE_ATTRIBUTE_HIDDEN;
      else if (!AttrDlg[SETATTR_HIDDEN].Selected)    ClearAttr|=FILE_ATTRIBUTE_HIDDEN;
      if (AttrDlg[SETATTR_SYSTEM].Selected == 1)     SetAttr|=FILE_ATTRIBUTE_SYSTEM;
      else if (!AttrDlg[SETATTR_SYSTEM].Selected)    ClearAttr|=FILE_ATTRIBUTE_SYSTEM;

      if (AttrDlg[SETATTR_COMPRESSED].Selected == 1)
      {
        SetAttr|=FILE_ATTRIBUTE_COMPRESSED;
        ClearAttr|=FILE_ATTRIBUTE_ENCRYPTED;
      }
      else if (!AttrDlg[SETATTR_COMPRESSED].Selected)
        ClearAttr|=FILE_ATTRIBUTE_COMPRESSED;

      if (AttrDlg[SETATTR_ENCRYPTED].Selected == 1)
      {
        SetAttr|=FILE_ATTRIBUTE_ENCRYPTED;
        ClearAttr|=FILE_ATTRIBUTE_COMPRESSED;
      }
      else if (!AttrDlg[SETATTR_ENCRYPTED].Selected)
        ClearAttr|=FILE_ATTRIBUTE_ENCRYPTED;

      if (AttrDlg[SETATTR_INDEXED].Selected == 1)        SetAttr|=FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
      else if (!AttrDlg[SETATTR_INDEXED].Selected)       ClearAttr|=FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;

			if(AttrDlg[SETATTR_SPARSE].Selected==BSTATE_CHECKED)
				SetAttr|=FILE_ATTRIBUTE_SPARSE_FILE;
			else if(AttrDlg[SETATTR_SPARSE].Selected==BSTATE_UNCHECKED)
				ClearAttr|=FILE_ATTRIBUTE_SPARSE_FILE;

			if(AttrDlg[SETATTR_TEMP].Selected==BSTATE_CHECKED)
				SetAttr|=FILE_ATTRIBUTE_TEMPORARY;
			else if(AttrDlg[SETATTR_TEMP].Selected==BSTATE_UNCHECKED)
				ClearAttr|=FILE_ATTRIBUTE_TEMPORARY;
			if(AttrDlg[SETATTR_OFFLINE].Selected==BSTATE_CHECKED)
				SetAttr|=FILE_ATTRIBUTE_OFFLINE;
			else if(AttrDlg[SETATTR_OFFLINE].Selected==BSTATE_UNCHECKED)
				ClearAttr|=FILE_ATTRIBUTE_OFFLINE;

      /*
      AY: мы с этими атрибутами не работаем
      if(!(AttrDlg[SETATTR_VIRTUAL].Flags&DIF_DISABLE))
      {
        if (AttrDlg[SETATTR_VIRTUAL].Selected == 1)        SetAttr|=FILE_ATTRIBUTE_VIRTUAL;
        else if (!AttrDlg[SETATTR_VIRTUAL].Selected)       ClearAttr|=FILE_ATTRIBUTE_VIRTUAL;
      }
      */

      SrcPanel->GetSelName(NULL,FileAttr);

			TaskBar TB;
      while (SrcPanel->GetSelName(&strSelName,FileAttr,NULL,&FindData) && !Cancel)
      {
//_SVS(SysLog(L"SelName='%s'\n\tFileAttr =0x%08X\n\tSetAttr  =0x%08X\n\tClearAttr=0x%08X\n\tResult   =0x%08X",
//    SelName,FileAttr,SetAttr,ClearAttr,((FileAttr|SetAttr)&(~ClearAttr))));
        ShellSetFileAttributesMsg(strSelName);

        if (CheckForEsc())
          break;

        RetCode=IsFileWritable(strSelName,FileAttr,TRUE,MSetAttrCannotFor,SkipMode);
        if(RetCode==SETATTR_RET_ERROR)
          break;
        else if(RetCode == SETATTR_RET_SKIP)
          continue;
        else if(RetCode == SETATTR_RET_SKIPALL)
        {
          SkipMode=SETATTR_RET_SKIP;
          continue;
        }

        SetWriteTime=     DlgParam.OLastWriteTime  && ReadFileTime(0,strSelName,FileAttr,&LastWriteTime,AttrDlg[SETATTR_MDATE].strData,AttrDlg[SETATTR_MTIME].strData);
        SetCreationTime=  DlgParam.OCreationTime   && ReadFileTime(1,strSelName,FileAttr,&CreationTime,AttrDlg[SETATTR_CDATE].strData,AttrDlg[SETATTR_CTIME].strData);
        SetLastAccessTime=DlgParam.OLastAccessTime && ReadFileTime(2,strSelName,FileAttr,&LastAccessTime,AttrDlg[SETATTR_ADATE].strData,AttrDlg[SETATTR_ATIME].strData);
        if(!(FileAttr&FILE_ATTRIBUTE_REPARSE_POINT) && (SetWriteTime || SetCreationTime || SetLastAccessTime))
        {
          if(SkipMode!=-1)
            RetCode=SkipMode;
          else
            RetCode=ESetFileTime(strSelName,
                 (SetWriteTime ? &LastWriteTime:NULL),
                 (SetCreationTime ? &CreationTime:NULL),
                 (SetLastAccessTime ? &LastAccessTime:NULL),
                 FileAttr,SkipMode);
          if(RetCode == SETATTR_RET_ERROR)
            break;
          else if(RetCode == SETATTR_RET_SKIP)
            continue;
          else if(RetCode == SETATTR_RET_SKIPALL)
          {
            SkipMode=SETATTR_RET_SKIP;
            continue;
          }
        }
        if(((FileAttr|SetAttr)&(~ClearAttr)) != FileAttr)
        {
          if (AttrDlg[SETATTR_COMPRESSED].Selected != 2)
          {
            RetCode=ESetFileCompression(strSelName,AttrDlg[SETATTR_COMPRESSED].Selected,FileAttr,SkipMode);
            if(RetCode == SETATTR_RET_ERROR)
              break;
            else if(RetCode == SETATTR_RET_SKIP)
              continue;
            else if(RetCode == SETATTR_RET_SKIPALL)
            {
              SkipMode=SETATTR_RET_SKIP;
              continue;
            }
          }
          if (AttrDlg[SETATTR_ENCRYPTED].Selected != 2) // +E -C
          {
            if(AttrDlg[SETATTR_COMPRESSED].Selected != 1)
            {
              RetCode=ESetFileEncryption(strSelName,AttrDlg[SETATTR_ENCRYPTED].Selected,FileAttr,SkipMode);
              if(RetCode == SETATTR_RET_ERROR)
                break;
              else if(RetCode == SETATTR_RET_SKIP)
                continue;
              else if(RetCode == SETATTR_RET_SKIPALL)
              {
                SkipMode=SETATTR_RET_SKIP;
                continue;
              }
            }
          }

					if(AttrDlg[SETATTR_SPARSE].Selected!=BSTATE_3STATE)
					{
						RetCode=ESetFileSparse(strSelName,AttrDlg[SETATTR_SPARSE].Selected==BSTATE_CHECKED,FileAttr,SkipMode);
						if(RetCode == SETATTR_RET_ERROR)
						{
							break;
						}
						else if(RetCode == SETATTR_RET_SKIP)
						{
							continue;
						}
						else if(RetCode == SETATTR_RET_SKIPALL)
						{
							SkipMode=SETATTR_RET_SKIP;
							continue;
						}
					}

          RetCode=ESetFileAttributes(strSelName,((FileAttr|SetAttr)&(~ClearAttr)),SkipMode);
          if(RetCode == SETATTR_RET_ERROR)
            break;
          else if(RetCode == SETATTR_RET_SKIP)
            continue;
          else if(RetCode == SETATTR_RET_SKIPALL)
          {
            SkipMode=SETATTR_RET_SKIP;
            continue;
          }
        }

        if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) && AttrDlg[SETATTR_SUBFOLDERS].Selected)
        {
          string strFullName;
          ScanTree ScTree(FALSE);

          ScTree.SetFindPath(strSelName,L"*.*");
          while (ScTree.GetNextName(&FindData,strFullName))
          {
            ShellSetFileAttributesMsg(strFullName);
            if (CheckForEsc())
            {
              Cancel=1;
              break;
            }

            RetCode=IsFileWritable(strFullName,FindData.dwFileAttributes,TRUE,MSetAttrCannotFor,SkipMode);
            if(RetCode==SETATTR_RET_ERROR)
            {
              Cancel=1;
              break;
            }
            else if(RetCode == SETATTR_RET_SKIP)
              continue;
            else if(RetCode == SETATTR_RET_SKIPALL)
            {
              SkipMode=SETATTR_RET_SKIP;
              continue;
            }

            SetWriteTime=     DlgParam.OLastWriteTime  && ReadFileTime(0,strFullName,FindData.dwFileAttributes,&LastWriteTime,AttrDlg[SETATTR_MDATE].strData,AttrDlg[SETATTR_MTIME].strData);
            SetCreationTime=  DlgParam.OCreationTime   && ReadFileTime(1,strFullName,FindData.dwFileAttributes,&CreationTime,AttrDlg[SETATTR_CDATE].strData,AttrDlg[SETATTR_CTIME].strData);
            SetLastAccessTime=DlgParam.OLastAccessTime && ReadFileTime(2,strFullName,FindData.dwFileAttributes,&LastAccessTime,AttrDlg[SETATTR_ADATE].strData,AttrDlg[SETATTR_ATIME].strData);
            if(!(FindData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) && (SetWriteTime || SetCreationTime || SetLastAccessTime))
            {
                RetCode=ESetFileTime(strFullName,SetWriteTime ? &LastWriteTime:NULL,
                           SetCreationTime ? &CreationTime:NULL,
                           SetLastAccessTime ? &LastAccessTime:NULL,
                           FindData.dwFileAttributes,SkipMode);
              if(RetCode == SETATTR_RET_ERROR)
              {
                Cancel=1;
                break;
              }
              else if(RetCode == SETATTR_RET_SKIP)
                continue;
              else if(RetCode == SETATTR_RET_SKIPALL)
              {
                SkipMode=SETATTR_RET_SKIP;
                continue;
              }
            }
            if(((FindData.dwFileAttributes|SetAttr)&(~ClearAttr)) !=
                 FindData.dwFileAttributes)
            {
              if (AttrDlg[SETATTR_COMPRESSED].Selected != 2)
              {
                RetCode=ESetFileCompression(strFullName,AttrDlg[SETATTR_COMPRESSED].Selected,FindData.dwFileAttributes,SkipMode);
                if(RetCode == SETATTR_RET_ERROR)
                {
                  Cancel=1;
                  break;
                }
                else if(RetCode == SETATTR_RET_SKIP)
                  continue;
                else if(RetCode == SETATTR_RET_SKIPALL)
                {
                  SkipMode=SETATTR_RET_SKIP;
                  continue;
                }
              }
              if (AttrDlg[SETATTR_ENCRYPTED].Selected != 2) // +E -C
              {
                if(AttrDlg[SETATTR_COMPRESSED].Selected != 1)
                {
                  RetCode=ESetFileEncryption(strFullName,AttrDlg[SETATTR_ENCRYPTED].Selected,FindData.dwFileAttributes,SkipMode);
                  if (RetCode == SETATTR_RET_ERROR)
                  {
                    Cancel=1;
                    break;
                  }
                  else if(RetCode == SETATTR_RET_SKIP)
                    continue;
                  else if(RetCode == SETATTR_RET_SKIPALL)
                  {
                    SkipMode=SETATTR_RET_SKIP;
                    continue;
                  }
                }
              }

							if(AttrDlg[SETATTR_SPARSE].Selected!=BSTATE_3STATE)
							{
								RetCode=ESetFileSparse(strFullName,AttrDlg[SETATTR_SPARSE].Selected==BSTATE_CHECKED,FindData.dwFileAttributes,SkipMode);
								if(RetCode == SETATTR_RET_ERROR)
								{
									Cancel=1;
									break;
								}
								else if(RetCode == SETATTR_RET_SKIP)
								{
									continue;
								}
								else if(RetCode == SETATTR_RET_SKIPALL)
								{
									SkipMode=SETATTR_RET_SKIP;
									continue;
								}
							}

              RetCode=ESetFileAttributes(strFullName,(FindData.dwFileAttributes|SetAttr)&(~ClearAttr),SkipMode);
              if (RetCode == SETATTR_RET_ERROR)
              {
                Cancel=1;
                break;
              }
              else if(RetCode == SETATTR_RET_SKIP)
                continue;
              else if(RetCode == SETATTR_RET_SKIPALL)
              {
                SkipMode=SETATTR_RET_SKIP;
                continue;
              }
            }
          }
        }
      } // END: while (SrcPanel->GetSelName(...))
      delete SetAttrTitle;
    }
  }

  SrcPanel->SaveSelection();
  SrcPanel->Update(UPDATE_KEEP_SELECTION);
  SrcPanel->ClearSelection();
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(SrcPanel);
  AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
  CtrlObject->Cp()->Redraw();
  return 1;
}

static int ReadFileTime(int Type,const wchar_t *Name,DWORD FileAttr,FILETIME *FileTime,
                        const wchar_t *OSrcDate, const wchar_t *OSrcTime)
{
  FILETIME ft, oft;
  SYSTEMTIME st, ost;
  unsigned DateN[3],TimeN[3];
  int DigitCount;
  int /*SetTime,*/GetTime;
  FILETIME *OriginalFileTime=0, OFTModify, OFTCreate, OFTLast;

  /*$ 17.07.2001 SKV
    от греха подальше, занулим.
  */
  ZeroMemory(&st,sizeof(st));
  /* SKV$*/

  // ****** ОБРАБОТКА ДАТЫ ******** //
  GetFileDateAndTime(OSrcDate,DateN,GetDateSeparator());
  // ****** ОБРАБОТКА ВРЕМЕНИ ******** //
  GetFileDateAndTime(OSrcTime,TimeN,GetTimeSeparator());

  // исключаем лишние телодвижения
  if(DateN[0] == (unsigned)-1 || DateN[1] == (unsigned)-1 || DateN[2] == (unsigned)-1 ||
     TimeN[0] == (unsigned)-1 || TimeN[1] == (unsigned)-1 || TimeN[2] == (unsigned)-1)
  {
    // получаем инфу про оригинальную дату и время файла.
    HANDLE hFile= apiCreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                 NULL,OPEN_EXISTING,
                 (FileAttr & FILE_ATTRIBUTE_DIRECTORY) ? FILE_FLAG_BACKUP_SEMANTICS:0);
    if (hFile==INVALID_HANDLE_VALUE)
      return(FALSE);
    GetTime=GetFileTime(hFile,&OFTCreate,&OFTLast,&OFTModify);
    CloseHandle(hFile);

    if(!GetTime)
      return(FALSE);

    switch(Type)
    {
      case 0: // Modif
        OriginalFileTime=&OFTModify;
        break;
      case 1: // Creat
        OriginalFileTime=&OFTCreate;
        break;
      case 2: // Last
        OriginalFileTime=&OFTLast;
        break;
    }

    // конвертнем в локальное время.
    FileTimeToLocalFileTime(OriginalFileTime,&oft);
    FileTimeToSystemTime(&oft,&ost);
    DigitCount=TRUE;
  }
  else
    DigitCount=FALSE;

  // "Оформим"
  switch(GetDateFormat())
  {
    case 0:
      st.wMonth=DateN[0]!=(unsigned)-1?DateN[0]:ost.wMonth;
      st.wDay  =DateN[1]!=(unsigned)-1?DateN[1]:ost.wDay;
      st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:ost.wYear;
      break;
    case 1:
      st.wDay  =DateN[0]!=(unsigned)-1?DateN[0]:ost.wDay;
      st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:ost.wMonth;
      st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:ost.wYear;
      break;
    default:
      st.wYear =DateN[0]!=(unsigned)-1?DateN[0]:ost.wYear;
      st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:ost.wMonth;
      st.wDay  =DateN[2]!=(unsigned)-1?DateN[2]:ost.wDay;
      break;
  }
  st.wHour   = TimeN[0]!=(unsigned)-1? (TimeN[0]):ost.wHour;
  st.wMinute = TimeN[1]!=(unsigned)-1? (TimeN[1]):ost.wMinute;
  st.wSecond = TimeN[2]!=(unsigned)-1? (TimeN[2]):ost.wSecond;

  if (st.wYear<100)
  {
    if (st.wYear<80)
      st.wYear+=2000;
    else
      st.wYear+=1900;
  }

  if(TimeN[0]==(unsigned)-1 && TimeN[1]==(unsigned)-1 && TimeN[2]==(unsigned)-1)
  {
    st.wMilliseconds=ost.wMilliseconds;
    // для правильности выставления wDayOfWeek
    //SystemTimeToFileTime(&st,&ft);
    //FileTimeToSystemTime(&ft,&st);
  }

  // преобразование в "удобоваримый" формат
  SystemTimeToFileTime(&st,&ft);
  LocalFileTimeToFileTime(&ft,FileTime);
  if(DigitCount)
    return (CompareFileTime(FileTime,OriginalFileTime) == 0)?FALSE:TRUE;
  return TRUE;
}

static int IsFileWritable(const wchar_t *Name, DWORD FileAttr, BOOL IsShowErrMsg, int Msg,int SkipMode)
{
  while (1)
  {
    if (FileAttr & FILE_ATTRIBUTE_READONLY)
			apiSetFileAttributes(Name,FileAttr & ~FILE_ATTRIBUTE_READONLY);

    HANDLE hFile= apiCreateFile(Name,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,(FileAttr & FILE_ATTRIBUTE_DIRECTORY) ? FILE_FLAG_BACKUP_SEMANTICS:0);
    BOOL Writable=TRUE;
    if(hFile == INVALID_HANDLE_VALUE)
      Writable=FALSE;
    else
      CloseHandle(hFile);

    DWORD LastError=GetLastError();
    if (FileAttr & FILE_ATTRIBUTE_READONLY)
			apiSetFileAttributes(Name,FileAttr);
    SetLastError(LastError);

    if (Writable)
      break;

    int Code;
    if(IsShowErrMsg)
    {
      if(SkipMode!=-1)
        Code=SkipMode;
      else
        Code=Message(MSG_DOWN|MSG_WARNING|MSG_ERRORTYPE,4,MSG(MError),
                     MSG(Msg),Name,
                     MSG(MHRetry),MSG(MHSkip),MSG(MHSkipAll),MSG(MHCancel));
    }
    else
       return SETATTR_RET_ERROR;

    switch(Code)
    {
    case -2:
    case -1:
    case 3:
      return SETATTR_RET_ERROR;
    case 1:
      return SETATTR_RET_SKIP;
    case 2:
      return SETATTR_RET_SKIPALL;
    }
  }
  return SETATTR_RET_OK;
}
