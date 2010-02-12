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
#include "ctrlobj.hpp"
#include "constitle.hpp"
#include "TPreRedrawFunc.hpp"
#include "TaskBar.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "config.hpp"
#include "datetime.hpp"
#include "fileattr.hpp"
#include "setattr.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "lasterror.hpp"

enum SETATTRDLG
{
	SA_DOUBLEBOX,
	SA_TEXT_LABEL,
	SA_TEXT_NAME,
	SA_TEXT_SYMLINK,
	SA_EDIT_SYMLINK,
	SA_COMBO_HARDLINK,
	SA_SEPARATOR1,
	SA_ATTR_FIRST,
	SA_CHECKBOX_RO=SA_ATTR_FIRST,
	SA_CHECKBOX_ARCHIVE,
	SA_CHECKBOX_HIDDEN,
	SA_CHECKBOX_SYSTEM,
	SA_CHECKBOX_COMPRESSED,
	SA_CHECKBOX_ENCRYPTED,
	SA_CHECKBOX_NOTINDEXED,
	SA_CHECKBOX_SPARSE,
	SA_CHECKBOX_TEMP,
	SA_CHECKBOX_OFFLINE,
	SA_CHECKBOX_VIRTUAL,
	SA_ATTR_LAST=SA_CHECKBOX_VIRTUAL,
	SA_SEPARATOR2,
	SA_CHECKBOX_SUBFOLDERS,
	SA_SEPARATOR3,
	SA_TEXT_TITLEDATE,
	SA_TEXT_MODIFICATION,
	SA_EDIT_MDATE,
	SA_EDIT_MTIME,
	SA_TEXT_CREATION,
	SA_EDIT_CDATE,
	SA_EDIT_CTIME,
	SA_TEXT_LASTACCESS,
	SA_EDIT_ADATE,
	SA_EDIT_ATIME,
	SA_BUTTON_ORIGINAL,
	SA_BUTTON_CURRENT,
	SA_BUTTON_BLANK,
	SA_SEPARATOR4,
	SA_BUTTON_SET,
	SA_BUTTON_CANCEL,
};

enum DIALOGMODE
{
	MODE_FILE,
	MODE_FOLDER,
	MODE_MULTIPLE,
};

struct SetAttrDlgParam
{
	bool Plugin;
	DWORD FileSystemFlags;
	DIALOGMODE DialogMode;
	string strSelName;
	// значения CheckBox`ов на момент старта диалога
	int OriginalCBAttr[16];
	int OriginalCBAttr2[16];
	DWORD OriginalCBFlag[16];
	FARCHECKEDSTATE OSubfoldersState, OCompressState, OEncryptState;
	bool OLastWriteTime,OCreationTime,OLastAccessTime;
};

#define DM_SETATTR (DM_USER+1)

LONG_PTR WINAPI SetAttrDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
	SetAttrDlgParam *DlgParam=reinterpret_cast<SetAttrDlgParam*>(SendDlgMessage(hDlg,DM_GETDLGDATA,0,0));

	switch (Msg)
	{
		case DN_BTNCLICK:

			if ((Param1>=SA_ATTR_FIRST&&Param1<=SA_ATTR_LAST)||Param1==SA_CHECKBOX_SUBFOLDERS)
			{
				DlgParam->OriginalCBAttr[Param1-SA_ATTR_FIRST]=static_cast<int>(Param2);
				DlgParam->OriginalCBAttr2[Param1-SA_ATTR_FIRST]=0;
				int FocusPos=static_cast<int>(SendDlgMessage(hDlg,DM_GETFOCUS,0,0));
				FARCHECKEDSTATE CompressState=static_cast<FARCHECKEDSTATE>(SendDlgMessage(hDlg,DM_GETCHECK,SA_CHECKBOX_COMPRESSED,0));
				FARCHECKEDSTATE EncryptState=static_cast<FARCHECKEDSTATE>(SendDlgMessage(hDlg,DM_GETCHECK,SA_CHECKBOX_ENCRYPTED,0));
				FARCHECKEDSTATE SubfoldersState=static_cast<FARCHECKEDSTATE>(SendDlgMessage(hDlg,DM_GETCHECK,SA_CHECKBOX_SUBFOLDERS,0));

				if (DlgParam->DialogMode==MODE_FILE)
				{
					if (((DlgParam->FileSystemFlags & (FILE_FILE_COMPRESSION|FILE_SUPPORTS_ENCRYPTION))==
					        (FILE_FILE_COMPRESSION|FILE_SUPPORTS_ENCRYPTION)) &&
					        (FocusPos == SA_CHECKBOX_COMPRESSED || FocusPos == SA_CHECKBOX_ENCRYPTED))
					{
						if (FocusPos == SA_CHECKBOX_COMPRESSED && /*CompressState &&*/ EncryptState)
							SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_ENCRYPTED,BSTATE_UNCHECKED);

						if (FocusPos == SA_CHECKBOX_ENCRYPTED && /*EncryptState &&*/ CompressState)
							SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_COMPRESSED,BSTATE_UNCHECKED);
					}
				}
				// =1|2 Multi
				else
				{
					// отработаем взаимоисключения
					if (((DlgParam->FileSystemFlags & (FILE_FILE_COMPRESSION|FILE_SUPPORTS_ENCRYPTION))==
					        (FILE_FILE_COMPRESSION|FILE_SUPPORTS_ENCRYPTION)) &&
					        (FocusPos == SA_CHECKBOX_COMPRESSED || FocusPos == SA_CHECKBOX_ENCRYPTED))
					{
						if (FocusPos == SA_CHECKBOX_COMPRESSED && DlgParam->OCompressState != CompressState) // Состояние изменилось?
						{
							if (CompressState == BSTATE_CHECKED && EncryptState)
								SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_ENCRYPTED,BSTATE_UNCHECKED);
							else if (CompressState == BSTATE_3STATE)
								SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_ENCRYPTED,BSTATE_3STATE);
						}
						else if (FocusPos == SA_CHECKBOX_ENCRYPTED && DlgParam->OEncryptState != EncryptState) // Состояние изменилось?
						{
							if (EncryptState == BSTATE_CHECKED && CompressState)
								SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_COMPRESSED,BSTATE_UNCHECKED);
							else if (EncryptState == BSTATE_3STATE)
								SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_COMPRESSED,BSTATE_3STATE);
						}

						// еще одна проверка
						if (Param2==BSTATE_CHECKED)
						{
							if (FocusPos == SA_CHECKBOX_COMPRESSED && EncryptState)
							{
								SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_ENCRYPTED,BSTATE_UNCHECKED);
							}

							if (FocusPos == SA_CHECKBOX_ENCRYPTED && CompressState)
							{
								SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_COMPRESSED,BSTATE_UNCHECKED);
							}
						}

						DlgParam->OEncryptState=EncryptState;
						DlgParam->OCompressState=CompressState;
					}

					// если снимаем атрибуты для SubFolders
					// этот кусок всегда работает если есть хотя бы одна папка
					// иначе SA_CHECKBOX_SUBFOLDERS недоступен и всегда снят.
					if (FocusPos == SA_CHECKBOX_SUBFOLDERS)
					{
						if (DlgParam->DialogMode==MODE_FOLDER) // каталог однозначно!
						{
							if (DlgParam->OSubfoldersState != SubfoldersState) // Состояние изменилось?
							{
								// убираем 3-State
								for (int i=SA_ATTR_FIRST; i<=SA_ATTR_LAST; i++)
								{
									// сняли?
									if (!SubfoldersState)
									{
										SendDlgMessage(hDlg,DM_SET3STATE,i,FALSE);
										SendDlgMessage(hDlg,DM_SETCHECK,i,DlgParam->OriginalCBAttr[i-SA_ATTR_FIRST]);
									}
									// установили?
									else
									{
										SendDlgMessage(hDlg,DM_SET3STATE,i,TRUE);

										if (DlgParam->OriginalCBAttr2[i-SA_ATTR_FIRST]==-1)
										{
											SendDlgMessage(hDlg,DM_SETCHECK,i,BSTATE_3STATE);
										}
									}
								}

								if (Opt.SetAttrFolderRules)
								{
									FAR_FIND_DATA_EX FindData;

									if (apiGetFindDataEx(DlgParam->strSelName,&FindData))
									{
										const SETATTRDLG Items[]={SA_TEXT_MODIFICATION,SA_TEXT_CREATION,SA_TEXT_LASTACCESS};
										bool* ParamTimes[]={&DlgParam->OLastWriteTime,&DlgParam->OCreationTime,&DlgParam->OLastAccessTime};
										const PFILETIME FDTimes[]={&FindData.ftLastWriteTime,&FindData.ftCreationTime,&FindData.ftLastAccessTime};

										for (size_t i=0; i<countof(Items); i++)
										{
											if (!*ParamTimes[i])
											{
												SendDlgMessage(hDlg,DM_SETATTR,Items[i],SubfoldersState?0:(LONG_PTR)FDTimes[i]);
												*ParamTimes[i]=false;
											}
										}
									}
								}
							}
						}
						// много объектов
						else
						{
							// Состояние изменилось?
							if (DlgParam->OSubfoldersState!=SubfoldersState)
							{
								for (int i=SA_ATTR_FIRST; i<= SA_ATTR_LAST; i++)
								{
									// сняли?
									if (!SubfoldersState)
									{
										SendDlgMessage(hDlg,DM_SET3STATE,i,((DlgParam->OriginalCBFlag[i-SA_ATTR_FIRST]&DIF_3STATE)?TRUE:FALSE));
										SendDlgMessage(hDlg,DM_SETCHECK,i,DlgParam->OriginalCBAttr[i-SA_ATTR_FIRST]);
									}
									// установили?
									else
									{
										if (DlgParam->OriginalCBAttr2[i-SA_ATTR_FIRST]==-1)
										{
											SendDlgMessage(hDlg,DM_SET3STATE,i,TRUE);
											SendDlgMessage(hDlg,DM_SETCHECK,i,BSTATE_3STATE);
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
			else if (Param1 == SA_BUTTON_ORIGINAL)
			{
				FAR_FIND_DATA_EX FindData;

				if (apiGetFindDataEx(DlgParam->strSelName,&FindData))
				{
					SendDlgMessage(hDlg,DM_SETATTR,SA_TEXT_MODIFICATION,(LONG_PTR)&FindData.ftLastWriteTime);
					SendDlgMessage(hDlg,DM_SETATTR,SA_TEXT_CREATION,(LONG_PTR)&FindData.ftCreationTime);
					SendDlgMessage(hDlg,DM_SETATTR,SA_TEXT_LASTACCESS,(LONG_PTR)&FindData.ftLastAccessTime);
					DlgParam->OLastWriteTime=DlgParam->OCreationTime=DlgParam->OLastAccessTime=false;
				}

				SendDlgMessage(hDlg,DM_SETFOCUS,SA_EDIT_MDATE,0);
				return TRUE;
			}
			else if (Param1 == SA_BUTTON_CURRENT || Param1 == SA_BUTTON_BLANK)
			{
				SendDlgMessage(hDlg,DM_SETATTR,SA_TEXT_MODIFICATION,Param1 == SA_BUTTON_CURRENT?-1:0);
				SendDlgMessage(hDlg,DM_SETATTR,SA_TEXT_CREATION,Param1 == SA_BUTTON_CURRENT?-1:0);
				SendDlgMessage(hDlg,DM_SETATTR,SA_TEXT_LASTACCESS,Param1 == SA_BUTTON_CURRENT?-1:0);
				DlgParam->OLastWriteTime=DlgParam->OCreationTime=DlgParam->OLastAccessTime=true;
				SendDlgMessage(hDlg,DM_SETFOCUS,SA_EDIT_MDATE,0);
				return TRUE;
			}

			break;
		case DN_MOUSECLICK:
		{
			//_SVS(SysLog(L"Msg=DN_MOUSECLICK Param1=%d Param2=%d",Param1,Param2));
			if (Param1>=SA_TEXT_MODIFICATION && Param1<=SA_EDIT_ATIME)
			{
				if (reinterpret_cast<MOUSE_EVENT_RECORD*>(Param2)->dwEventFlags==DOUBLE_CLICK)
				{
					// Дадим Менеджеру диалогов "попотеть"
					DefDlgProc(hDlg,Msg,Param1,Param2);
					SendDlgMessage(hDlg,DM_SETATTR,Param1,-1);
				}

				if (Param1 == SA_TEXT_MODIFICATION || Param1 == SA_TEXT_CREATION || Param1 == SA_TEXT_LASTACCESS)
				{
					Param1++;
				}

				SendDlgMessage(hDlg,DM_SETFOCUS,Param1,0);
			}
		}
		break;
		case DN_EDITCHANGE:
		{
			switch (Param1)
			{
				case SA_COMBO_HARDLINK:
				{
					FarListInfo li;
					SendDlgMessage(hDlg,DM_LISTINFO,SA_COMBO_HARDLINK,(LONG_PTR)&li);
					string strTmp;
					strTmp.Format(MSG(MSetAttrHardLinks),li.ItemsNumber);
					SendDlgMessage(hDlg,DM_SETTEXTPTR,SA_COMBO_HARDLINK,(LONG_PTR)strTmp.CPtr());
				}
				break;
				case SA_EDIT_MDATE:
				case SA_EDIT_MTIME:
					DlgParam->OLastWriteTime=true;
					break;
				case SA_EDIT_CDATE:
				case SA_EDIT_CTIME:
					DlgParam->OCreationTime=true;
					break;
				case SA_EDIT_ADATE:
				case SA_EDIT_ATIME:
					DlgParam->OLastAccessTime=true;
					break;
			}

			break;
		}
		case DM_SETATTR:
		{
			string strDate,strTime;

			if (Param2) // Set?
			{
				FILETIME ft;

				if (Param2==-1)
				{
					GetSystemTimeAsFileTime(&ft);
				}
				else
				{
					ft=*reinterpret_cast<PFILETIME>(Param2);
				}

				ConvertDate(ft,strDate,strTime,12,FALSE,FALSE,TRUE,TRUE);
			}

			// Глянем на место, где был клик
			int Set1=-1;
			int Set2=Param1;

			switch (Param1)
			{
				case SA_TEXT_MODIFICATION:
					Set1=SA_EDIT_MDATE;
					Set2=SA_EDIT_MTIME;
					DlgParam->OLastWriteTime=true;
					break;
				case SA_TEXT_CREATION:
					Set1=SA_EDIT_CDATE;
					Set2=SA_EDIT_CTIME;
					DlgParam->OCreationTime=true;
					break;
				case SA_TEXT_LASTACCESS:
					Set1=SA_EDIT_ADATE;
					Set2=SA_EDIT_ATIME;
					DlgParam->OLastAccessTime=true;
					break;
				case SA_EDIT_MDATE:
				case SA_EDIT_CDATE:
				case SA_EDIT_ADATE:
					Set1=Param1;
					Set2=-1;
					break;
			}

			if (Set1!=-1)
			{
				SendDlgMessage(hDlg,DM_SETTEXTPTR,Set1,(LONG_PTR)strDate.CPtr());
			}

			if (Set2!=-1)
			{
				SendDlgMessage(hDlg,DM_SETTEXTPTR,Set2,(LONG_PTR)strTime.CPtr());
			}

			return TRUE;
		}
	}

	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

void ShellSetFileAttributesMsg(const wchar_t *Name)
{
	static int Width=54;
	int WidthTemp;

	if (Name && *Name)
		WidthTemp=Max(StrLength(Name),54);
	else
		Width=WidthTemp=54;

	WidthTemp=Min(WidthTemp,WidthNameForMessage);
	Width=Max(Width,WidthTemp);
	string strOutFileName=Name;
	TruncPathStr(strOutFileName,Width);
	CenterStr(strOutFileName,strOutFileName,Width+4);
	Message(0,0,MSG(MSetAttrTitle),MSG(MSetAttrSetting),strOutFileName);
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	preRedrawItem.Param.Param1=Name;
	PreRedraw.SetParam(preRedrawItem.Param);
}

bool ReadFileTime(int Type,const wchar_t *Name,FILETIME& FileTime,const wchar_t *OSrcDate,const wchar_t *OSrcTime)
{
	bool Result=false;
	FAR_FIND_DATA_EX ffd={0};

	if (apiGetFindDataEx(Name,&ffd))
	{
		WORD DateN[3]={0},TimeN[4]={0};
		GetFileDateAndTime(OSrcDate,DateN,countof(DateN),GetDateSeparator());
		GetFileDateAndTime(OSrcTime,TimeN,countof(TimeN),GetTimeSeparator());
		FILETIME *OriginalFileTime=NULL;
		SYSTEMTIME st={0}, ost={0};

		switch (Type)
		{
			case 0: // Modif
				OriginalFileTime=&ffd.ftLastWriteTime;
				break;
			case 1: // Creat
				OriginalFileTime=&ffd.ftCreationTime;
				break;
			case 2: // Last
				OriginalFileTime=&ffd.ftLastAccessTime;
				break;
		}

		SYSTEMTIME s={0};

		if (FileTimeToSystemTime(OriginalFileTime,&s))
		{
			SystemTimeToFileTime(&s,OriginalFileTime);
		}

		FILETIME oft={0};

		if (FileTimeToLocalFileTime(OriginalFileTime,&oft))
		{
			FileTimeToSystemTime(&oft,&ost);
		}

		// "Оформим"
		switch (GetDateFormat())
		{
			case 0:
				st.wMonth=DateN[0]!=(WORD)-1?DateN[0]:ost.wMonth;
				st.wDay  =DateN[1]!=(WORD)-1?DateN[1]:ost.wDay;
				st.wYear =DateN[2]!=(WORD)-1?DateN[2]:ost.wYear;
				break;
			case 1:
				st.wDay  =DateN[0]!=(WORD)-1?DateN[0]:ost.wDay;
				st.wMonth=DateN[1]!=(WORD)-1?DateN[1]:ost.wMonth;
				st.wYear =DateN[2]!=(WORD)-1?DateN[2]:ost.wYear;
				break;
			default:
				st.wYear =DateN[0]!=(WORD)-1?DateN[0]:ost.wYear;
				st.wMonth=DateN[1]!=(WORD)-1?DateN[1]:ost.wMonth;
				st.wDay  =DateN[2]!=(WORD)-1?DateN[2]:ost.wDay;
				break;
		}

		st.wHour         = TimeN[0]!=(WORD)-1? (TimeN[0]):ost.wHour;
		st.wMinute       = TimeN[1]!=(WORD)-1? (TimeN[1]):ost.wMinute;
		st.wSecond       = TimeN[2]!=(WORD)-1? (TimeN[2]):ost.wSecond;
		st.wMilliseconds = TimeN[3]!=(WORD)-1? (TimeN[3]):ost.wMilliseconds;

		if (st.wYear<100)
		{
			if (st.wYear<80)
				st.wYear+=2000;
			else
				st.wYear+=1900;
		}

		// преобразование в "удобоваримый" формат
		FILETIME lft={0};

		if (SystemTimeToFileTime(&st,&lft))
		{
			LocalFileTimeToFileTime(&lft,&FileTime);
		}

		Result=CompareFileTime(&FileTime,OriginalFileTime)!=0;
	}

	return Result;
}

void PR_ShellSetFileAttributesMsg()
{
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	ShellSetFileAttributesMsg(reinterpret_cast<const wchar_t*>(preRedrawItem.Param.Param1));
}

bool ShellSetFileAttributes(Panel *SrcPanel)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	short DlgX=70,DlgY=23;
	DialogDataEx AttrDlgData[]=
	{
		DI_DOUBLEBOX,3,1,DlgX-4,DlgY-2,0,0,0,0,MSG(MSetAttrTitle),
		DI_TEXT,-1,2,0,2,0,0,0,0,MSG(MSetAttrFor),
		DI_TEXT,-1,3,0,3,0,0,DIF_SHOWAMPERSAND,0,L"",
		DI_TEXT,5,3,14,3,0,0,DIF_HIDDEN,0,L"",
		DI_EDIT,15,3,DlgX-6,3,0,0,DIF_HIDDEN|DIF_EDITPATH,0,L"",
		DI_COMBOBOX,5,3,DlgX-6,3,0,0,DIF_SHOWAMPERSAND|DIF_DROPDOWNLIST|DIF_LISTWRAPMODE|DIF_HIDDEN,0,L"",
		DI_TEXT,3,4,0,4,0,0,DIF_SEPARATOR,0,L"",
		DI_CHECKBOX,5, 5,0,5,1,0,DIF_3STATE,0,MSG(MSetAttrRO),
		DI_CHECKBOX,5, 6,0,6,0,0,DIF_3STATE,0,MSG(MSetAttrArchive),
		DI_CHECKBOX,5, 7,0,7,0,0,DIF_3STATE,0,MSG(MSetAttrHidden),
		DI_CHECKBOX,5, 8,0,8,0,0,DIF_3STATE,0,MSG(MSetAttrSystem),
		DI_CHECKBOX,5, 9,0,9,0,0,DIF_3STATE,0,MSG(MSetAttrCompressed),
		DI_CHECKBOX,5,10,0,10,0,0,DIF_3STATE,0,MSG(MSetAttrEncrypted),
		DI_CHECKBOX,DlgX/2,5,0,5,0,0,DIF_3STATE,0,MSG(MSetAttrNotIndexed),
		DI_CHECKBOX,DlgX/2,6,0,6,0,0,DIF_3STATE,0,MSG(MSetAttrSparse),
		DI_CHECKBOX,DlgX/2,7,0,7,0,0,DIF_3STATE,0,MSG(MSetAttrTemp),
		DI_CHECKBOX,DlgX/2,8,0,8,0,0,DIF_3STATE,0,MSG(MSetAttrOffline),
		DI_CHECKBOX,DlgX/2,9,0,9,0,0,DIF_3STATE|DIF_DISABLE,0,MSG(MSetAttrVirtual),
		DI_TEXT,3,11,0,11,0,0,DIF_SEPARATOR,0,L"",
		DI_CHECKBOX,5,12,0,12,0,0,DIF_DISABLE,0,MSG(MSetAttrSubfolders),
		DI_TEXT,3,13,0,13,0,0,DIF_SEPARATOR,0,L"",
		DI_TEXT,DlgX-28,14,0,14,0,0,0,0,L"",
		DI_TEXT,    5,15,0,15,0,0,0,0,MSG(MSetAttrModification),
		DI_FIXEDIT,DlgX-28,15,DlgX-19,15,0,0,DIF_MASKEDIT,0,L"",
		DI_FIXEDIT,DlgX-17,15,DlgX-6,15,0,0,DIF_MASKEDIT,0,L"",
		DI_TEXT,    5,16,0,16,0,0,0,0,MSG(MSetAttrCreation),
		DI_FIXEDIT,DlgX-28,16,DlgX-19,16,0,0,DIF_MASKEDIT,0,L"",
		DI_FIXEDIT,DlgX-17,16,DlgX-6,16,0,0,DIF_MASKEDIT,0,L"",
		DI_TEXT,    5,17,0,17,0,0,0,0,MSG(MSetAttrLastAccess),
		DI_FIXEDIT,DlgX-28,17,DlgX-19,17,0,0,DIF_MASKEDIT,0,L"",
		DI_FIXEDIT,DlgX-17,17,DlgX-6,17,0,0,DIF_MASKEDIT,0,L"",
		DI_BUTTON,0,18,0,18,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,MSG(MSetAttrOriginal),
		DI_BUTTON,0,18,0,18,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,MSG(MSetAttrCurrent),
		DI_BUTTON,0,18,0,18,0,0,DIF_CENTERGROUP|DIF_BTNNOCLOSE,0,MSG(MSetAttrBlank),
		DI_TEXT,3,DlgY-4,0,DlgY-4,0,0,DIF_SEPARATOR,0,L"",
		DI_BUTTON,0,DlgY-3,0,DlgY-3,0,0,DIF_CENTERGROUP,1,MSG(MSetAttrSet),
		DI_BUTTON,0,DlgY-3,0,DlgY-3,0,0,DIF_CENTERGROUP,0,MSG(MCancel),
	};
	MakeDialogItemsEx(AttrDlgData,AttrDlg);
	SetAttrDlgParam DlgParam={0};
	int SelCount=SrcPanel->GetSelCount();

	if (!SelCount)
	{
		return false;
	}

	if (SrcPanel->GetMode()==PLUGIN_PANEL)
	{
		OpenPluginInfo Info;
		HANDLE hPlugin=SrcPanel->GetPluginHandle();

		if (hPlugin == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		CtrlObject->Plugins.GetOpenPluginInfo(hPlugin,&Info);

		if (!(Info.Flags & OPIF_REALNAMES))
		{
			AttrDlg[SA_BUTTON_SET].Flags|=DIF_DISABLE;
			DlgParam.Plugin=true;
		}
	}

	FarList NameList={0};
	string *strLinks=NULL;

	if (!DlgParam.Plugin)
	{
		string strRootPathName;
		apiGetCurrentDirectory(strRootPathName);
		GetPathRoot(strRootPathName,strRootPathName);

		if (apiGetVolumeInformation(strRootPathName,NULL,0,NULL,&DlgParam.FileSystemFlags,NULL))
		{
			if (!(DlgParam.FileSystemFlags&FILE_FILE_COMPRESSION))
			{
				AttrDlg[SA_CHECKBOX_COMPRESSED].Flags|=DIF_DISABLE;
			}

			if (!(DlgParam.FileSystemFlags&FILE_SUPPORTS_ENCRYPTION))
			{
				AttrDlg[SA_CHECKBOX_ENCRYPTED].Flags|=DIF_DISABLE;
			}

			if (!(DlgParam.FileSystemFlags&FILE_SUPPORTS_SPARSE_FILES))
			{
				AttrDlg[SA_CHECKBOX_SPARSE].Flags|=DIF_DISABLE;
			}
		}
	}

	{
		DWORD FileAttr;
		SrcPanel->GetSelName(NULL,FileAttr);
		string strSelName;
		FAR_FIND_DATA_EX FindData;
		SrcPanel->GetSelName(&strSelName,FileAttr,NULL,&FindData);

		if (SelCount==0 || (SelCount==1 && TestParentFolderName(strSelName)))
			return false;

		wchar_t DateSeparator=GetDateSeparator();
		wchar_t TimeSeparator=GetTimeSeparator();
		wchar_t DecimalSeparator=GetDecimalSeparator();
		LPCWSTR FmtMask1=L"99%c99%c99%c999",FmtMask2=L"99%c99%c9999",FmtMask3=L"9999%c99%c99";
		string strDMask, strTMask;
		strTMask.Format(FmtMask1,TimeSeparator,TimeSeparator,DecimalSeparator);

		switch (GetDateFormat())
		{
			case 0:
				AttrDlg[SA_TEXT_TITLEDATE].strData.Format(MSG(MSetAttrTimeTitle1),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator,DecimalSeparator);
				strDMask.Format(FmtMask2,DateSeparator,DateSeparator);
				break;
			case 1:
				AttrDlg[SA_TEXT_TITLEDATE].strData.Format(MSG(MSetAttrTimeTitle2),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator,DecimalSeparator);
				strDMask.Format(FmtMask2,DateSeparator,DateSeparator);
				break;
			default:
				AttrDlg[SA_TEXT_TITLEDATE].strData.Format(MSG(MSetAttrTimeTitle3),DateSeparator,DateSeparator,TimeSeparator,TimeSeparator,DecimalSeparator);
				strDMask.Format(FmtMask3,DateSeparator,DateSeparator);
				break;
		}

		AttrDlg[SA_EDIT_MDATE].Mask=AttrDlg[SA_EDIT_CDATE].Mask=AttrDlg[SA_EDIT_ADATE].Mask=strDMask;
		AttrDlg[SA_EDIT_MTIME].Mask=AttrDlg[SA_EDIT_CTIME].Mask=AttrDlg[SA_EDIT_ATIME].Mask=strTMask;
		bool FolderPresent=false,LinkPresent=false;
		string strLinkName;
		static struct ATTRIBUTEPAIR
		{
			SETATTRDLG Item;
			DWORD Attribute;
		}
		AP[]=
		{
			{SA_CHECKBOX_RO,FILE_ATTRIBUTE_READONLY},
			{SA_CHECKBOX_ARCHIVE,FILE_ATTRIBUTE_ARCHIVE},
			{SA_CHECKBOX_HIDDEN,FILE_ATTRIBUTE_HIDDEN},
			{SA_CHECKBOX_SYSTEM,FILE_ATTRIBUTE_SYSTEM},
			{SA_CHECKBOX_COMPRESSED,FILE_ATTRIBUTE_COMPRESSED},
			{SA_CHECKBOX_ENCRYPTED,FILE_ATTRIBUTE_ENCRYPTED},
			{SA_CHECKBOX_NOTINDEXED,FILE_ATTRIBUTE_NOT_CONTENT_INDEXED},
			{SA_CHECKBOX_SPARSE,FILE_ATTRIBUTE_SPARSE_FILE},
			{SA_CHECKBOX_TEMP,FILE_ATTRIBUTE_TEMPORARY},
			{SA_CHECKBOX_OFFLINE,FILE_ATTRIBUTE_OFFLINE},
			{SA_CHECKBOX_OFFLINE,FILE_ATTRIBUTE_OFFLINE},
			{SA_CHECKBOX_VIRTUAL,FILE_ATTRIBUTE_VIRTUAL},
		};

		if (SelCount==1)
		{
			if (FileAttr&FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!DlgParam.Plugin)
				{
					FileAttr=apiGetFileAttributes(strSelName);
				}

				//_SVS(SysLog(L"SelName=%s  FileAttr=0x%08X",SelName,FileAttr));
				AttrDlg[SA_CHECKBOX_SUBFOLDERS].Flags&=~DIF_DISABLE;
				AttrDlg[SA_CHECKBOX_SUBFOLDERS].Selected=Opt.SetAttrFolderRules?BSTATE_UNCHECKED:BSTATE_CHECKED;

				if (Opt.SetAttrFolderRules)
				{
					if (DlgParam.Plugin || apiGetFindDataEx(strSelName,&FindData))
					{
						ConvertDate(FindData.ftLastWriteTime, AttrDlg[SA_EDIT_MDATE].strData,AttrDlg[SA_EDIT_MTIME].strData,12,FALSE,FALSE,TRUE,TRUE);
						ConvertDate(FindData.ftCreationTime,  AttrDlg[SA_EDIT_CDATE].strData,AttrDlg[SA_EDIT_CTIME].strData,12,FALSE,FALSE,TRUE,TRUE);
						ConvertDate(FindData.ftLastAccessTime,AttrDlg[SA_EDIT_ADATE].strData,AttrDlg[SA_EDIT_ATIME].strData,12,FALSE,FALSE,TRUE,TRUE);
					}

					if (FileAttr!=INVALID_FILE_ATTRIBUTES)
					{
						for (size_t i=0; i<countof(AP); i++)
						{
							AttrDlg[AP[i].Item].Selected=FileAttr&AP[i].Attribute?BSTATE_CHECKED:BSTATE_UNCHECKED;
						}
					}

					for (size_t i=SA_ATTR_FIRST; i<= SA_ATTR_LAST; i++)
					{
						AttrDlg[i].Flags&=~DIF_3STATE;
					}
				}

				FolderPresent=TRUE;
			}
			else
			{
				for (size_t i=SA_ATTR_FIRST; i<=SA_ATTR_LAST; i++)
				{
					AttrDlg[i].Flags&=~DIF_3STATE;
				}
			}

			// обработка случая, если ЭТО SymLink
			if (FileAttr!=INVALID_FILE_ATTRIBUTES && FileAttr&FILE_ATTRIBUTE_REPARSE_POINT)
			{
				DWORD ReparseTag=0;
				DWORD LenJunction=DlgParam.Plugin?0:GetReparsePointInfo(strSelName, strLinkName,&ReparseTag);
				AttrDlg[SA_DOUBLEBOX].Y2++;

				for (size_t i=SA_TEXT_SYMLINK; i<countof(AttrDlgData); i++)
				{
					AttrDlg[i].Y1++;

					if (AttrDlg[i].Y2)
					{
						AttrDlg[i].Y2++;
					}
				}

				LinkPresent=true;
				NormalizeSymlinkName(strLinkName);
				int ID_Msg=MSetAttrSymlink;

				if (ReparseTag==IO_REPARSE_TAG_MOUNT_POINT)
				{
					if (IsLocalVolumeRootPath(strLinkName))
					{
						string strLinkRoot;
						GetPathRoot(strLinkName,strLinkRoot);

						if (IsLocalPath(strLinkRoot))
						{
							strLinkName = strLinkRoot;
						}

						ID_Msg=MSetAttrVolMount;
					}
					else
					{
						ID_Msg=MSetAttrJunction;
					}
				}

				AttrDlg[SA_TEXT_SYMLINK].Flags&=~DIF_HIDDEN;
				AttrDlg[SA_TEXT_SYMLINK].strData=MSG(ID_Msg);
				AttrDlg[SA_EDIT_SYMLINK].Flags&=~DIF_HIDDEN;
				AttrDlg[SA_EDIT_SYMLINK].strData=LenJunction?strLinkName.CPtr():MSG(MSetAttrUnknownJunction);
				DlgParam.FileSystemFlags=0;
				string strRoot;
				GetPathRoot(strSelName,strRoot);

				if (apiGetVolumeInformation(strRoot,NULL,0,NULL,&DlgParam.FileSystemFlags,NULL))
				{
					if (!(DlgParam.FileSystemFlags&FILE_FILE_COMPRESSION))
					{
						AttrDlg[SA_CHECKBOX_COMPRESSED].Flags|=DIF_DISABLE;
					}

					if (!(DlgParam.FileSystemFlags&FILE_SUPPORTS_ENCRYPTION))
					{
						AttrDlg[SA_CHECKBOX_ENCRYPTED].Flags|=DIF_DISABLE;
					}

					if (!(DlgParam.FileSystemFlags&FILE_SUPPORTS_SPARSE_FILES))
					{
						AttrDlg[SA_CHECKBOX_SPARSE].Flags|=DIF_DISABLE;
					}
				}
			}

			// обработка случая "несколько хардлинков"
			NameList.ItemsNumber=GetNumberOfLinks(strSelName);

			if (NameList.ItemsNumber>1)
			{
				AttrDlg[SA_TEXT_NAME].Flags|=DIF_HIDDEN;
				AttrDlg[SA_COMBO_HARDLINK].Flags&=~DIF_HIDDEN;
				NameList.Items=new FarListItem[NameList.ItemsNumber]();
				strLinks=new string[NameList.ItemsNumber];
				HANDLE hFind=apiFindFirstFileName(strSelName,0,strLinks[0]);
				int Current=0;

				if (hFind!=INVALID_HANDLE_VALUE)
				{
					string strRoot;
					GetPathRoot(strSelName,strRoot);
					DeleteEndSlash(strRoot);
					strLinks[0]=strRoot+strLinks[0];
					NameList.Items[Current++].Text=strLinks[0];

					while (apiFindNextFileName(hFind,strLinks[Current]))
					{
						strLinks[Current]=strRoot+strLinks[Current];
						NameList.Items[Current].Text=strLinks[Current];
						Current++;
					}

					apiFindClose(hFind);
					AttrDlg[SA_COMBO_HARDLINK].ListItems=&NameList;
				}
				else
				{
					AttrDlg[SA_COMBO_HARDLINK].Flags|=DIF_DISABLE;
				}

				AttrDlg[SA_COMBO_HARDLINK].strData.Format(MSG(MSetAttrHardLinks),NameList.ItemsNumber);
			}

			AttrDlg[SA_TEXT_NAME].strData = strSelName;
			TruncStr(AttrDlg[SA_TEXT_NAME].strData,DlgX-10);

			if (FileAttr!=INVALID_FILE_ATTRIBUTES)
			{
				for (size_t i=0; i<countof(AP); i++)
				{
					AttrDlg[AP[i].Item].Selected=FileAttr&AP[i].Attribute?BSTATE_CHECKED:BSTATE_UNCHECKED;
				}
			}

			const SETATTRDLG Dates[]={SA_EDIT_MDATE,SA_EDIT_CDATE,SA_EDIT_ADATE},Times[]={SA_EDIT_MTIME,SA_EDIT_CTIME,SA_EDIT_ATIME};
			const PFILETIME TimeValues[]={&FindData.ftLastWriteTime,&FindData.ftCreationTime,&FindData.ftLastAccessTime};

			if (DlgParam.Plugin || (!DlgParam.Plugin&&apiGetFindDataEx(strSelName,&FindData)))
			{
				for (size_t i=0; i<countof(Dates); i++)
				{
					ConvertDate(*TimeValues[i],AttrDlg[Dates[i]].strData,AttrDlg[Times[i]].strData,12,FALSE,FALSE,TRUE,TRUE);
				}
			}
		}
		else
		{
			for (size_t i=0; i<countof(AP); i++)
			{
				AttrDlg[AP[i].Item].Selected=BSTATE_3STATE;
			}

			AttrDlg[SA_EDIT_MDATE].strData.Clear();
			AttrDlg[SA_EDIT_MTIME].strData.Clear();
			AttrDlg[SA_EDIT_CDATE].strData.Clear();
			AttrDlg[SA_EDIT_CTIME].strData.Clear();
			AttrDlg[SA_EDIT_ADATE].strData.Clear();
			AttrDlg[SA_EDIT_ATIME].strData.Clear();
			AttrDlg[SA_BUTTON_ORIGINAL].Flags|=DIF_DISABLE;
			AttrDlg[SA_TEXT_NAME].strData = MSG(MSetAttrSelectedObjects);

			for (size_t i=SA_ATTR_FIRST; i<=SA_ATTR_LAST; i++)
			{
				AttrDlg[i].Selected=BSTATE_UNCHECKED;
			}

			// проверка - есть ли среди выделенных - каталоги?
			// так же проверка на атрибуты
			SrcPanel->GetSelName(NULL,FileAttr);
			FolderPresent=false;

			while (SrcPanel->GetSelName(&strSelName,FileAttr,NULL,&FindData))
			{
				if (!FolderPresent&&(FileAttr&FILE_ATTRIBUTE_DIRECTORY))
				{
					FolderPresent=true;
					AttrDlg[SA_CHECKBOX_SUBFOLDERS].Flags&=~DIF_DISABLE;
				}

				for (size_t i=0; i<countof(AP); i++)
				{
					if (FileAttr&AP[i].Attribute)
					{
						AttrDlg[AP[i].Item].Selected++;
					}
				}
			}

			SrcPanel->GetSelName(NULL,FileAttr);
			SrcPanel->GetSelName(&strSelName,FileAttr,NULL,&FindData);

			// выставим "неопределенку" или то, что нужно
			for (size_t i=SA_ATTR_FIRST; i<=SA_ATTR_LAST; i++)
			{
				// снимаем 3-state, если "есть все или нет ничего"
				// за исключением случая, если есть Фолдер среди объектов
				if ((!AttrDlg[i].Selected || AttrDlg[i].Selected >= SelCount) && !FolderPresent)
				{
					AttrDlg[i].Flags&=~DIF_3STATE;
				}

				AttrDlg[i].Selected=(AttrDlg[i].Selected >= SelCount)?BST_CHECKED:(!AttrDlg[i].Selected?BSTATE_UNCHECKED:BSTATE_3STATE);
			}
		}

		// поведение для каталогов как у 1.65?
		if (FolderPresent && !Opt.SetAttrFolderRules)
		{
			AttrDlg[SA_CHECKBOX_SUBFOLDERS].Selected=BSTATE_CHECKED;
			AttrDlg[SA_EDIT_MDATE].strData.Clear();
			AttrDlg[SA_EDIT_MTIME].strData.Clear();
			AttrDlg[SA_EDIT_CDATE].strData.Clear();
			AttrDlg[SA_EDIT_CTIME].strData.Clear();
			AttrDlg[SA_EDIT_ADATE].strData.Clear();
			AttrDlg[SA_EDIT_ATIME].strData.Clear();

			for (size_t i=SA_ATTR_FIRST; i<= SA_ATTR_LAST; i++)
			{
				AttrDlg[i].Selected=BSTATE_3STATE;
				AttrDlg[i].Flags|=DIF_3STATE;
			}
		}

		// запомним состояние переключателей.
		for (size_t i=SA_ATTR_FIRST; i<=SA_ATTR_LAST; i++)
		{
			DlgParam.OriginalCBAttr[i-SA_ATTR_FIRST]=AttrDlg[i].Selected;
			DlgParam.OriginalCBAttr2[i-SA_ATTR_FIRST]=-1;
			DlgParam.OriginalCBFlag[i-SA_ATTR_FIRST]=AttrDlg[i].Flags;
		}

		DlgParam.DialogMode=((SelCount==1&&!(FileAttr&FILE_ATTRIBUTE_DIRECTORY))?MODE_FILE:(SelCount==1?MODE_FOLDER:MODE_MULTIPLE));
		DlgParam.strSelName=strSelName;
		DlgParam.OSubfoldersState=static_cast<FARCHECKEDSTATE>(AttrDlg[SA_CHECKBOX_SUBFOLDERS].Selected);
		DlgParam.OCompressState=static_cast<FARCHECKEDSTATE>(AttrDlg[SA_CHECKBOX_COMPRESSED].Selected);
		DlgParam.OEncryptState=static_cast<FARCHECKEDSTATE>(AttrDlg[SA_CHECKBOX_ENCRYPTED].Selected);
		{
			Dialog Dlg(AttrDlg,countof(AttrDlgData),SetAttrDlgProc,(LONG_PTR)&DlgParam);
			Dlg.SetHelp(L"FileAttrDlg");                 //  ^ - это одиночный диалог!

			if (LinkPresent)
			{
				DlgY++;
			}

			Dlg.SetPosition(-1,-1,DlgX,DlgY);
			Dlg.Process();

			if (NameList.Items)
				delete[] NameList.Items;

			if (strLinks)
				delete[] strLinks;

			if (Dlg.GetExitCode()!=SA_BUTTON_SET)
				return false;
		}

		//reparse point editor
		if (StrCmpI(AttrDlg[SA_EDIT_SYMLINK].strData,strLinkName))
		{
			if(!ModifyReparsePoint(strSelName,AttrDlg[SA_EDIT_SYMLINK].strData))
			{
				Message(FMSG_WARNING|FMSG_ERRORTYPE,1,MSG(MError),MSG(MCopyCannotCreateLink),strSelName,MSG(MHOk));
			}
		}

		const size_t Times[]={SA_EDIT_MTIME,SA_EDIT_CTIME,SA_EDIT_ATIME};

		for (size_t i=0; i<countof(Times); i++)
		{
			LPWSTR TimePtr=AttrDlg[Times[i]].strData.GetBuffer();
			TimePtr[8]=GetTimeSeparator();
			AttrDlg[Times[i]].strData.ReleaseBuffer(AttrDlg[Times[i]].strData.GetLength());
		}

		TPreRedrawFuncGuard preRedrawFuncGuard(PR_ShellSetFileAttributesMsg);
		ShellSetFileAttributesMsg(SelCount==1?strSelName.CPtr():NULL);
		int SkipMode=-1;

		if (SelCount==1 && !(FileAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			DWORD NewAttr=FileAttr&FILE_ATTRIBUTE_DIRECTORY;

			for (size_t i=0; i<countof(AP); i++)
			{
				if (AttrDlg[AP[i].Item].Selected)
				{
					NewAttr|=AP[i].Attribute;
				}
			}

			FILETIME LastWriteTime,CreationTime,LastAccessTime;
			int SetWriteTime=     DlgParam.OLastWriteTime  && ReadFileTime(0,strSelName,LastWriteTime,AttrDlg[SA_EDIT_MDATE].strData,AttrDlg[SA_EDIT_MTIME].strData);
			int SetCreationTime=  DlgParam.OCreationTime   && ReadFileTime(1,strSelName,CreationTime,AttrDlg[SA_EDIT_CDATE].strData,AttrDlg[SA_EDIT_CTIME].strData);
			int SetLastAccessTime=DlgParam.OLastAccessTime && ReadFileTime(2,strSelName,LastAccessTime,AttrDlg[SA_EDIT_ADATE].strData,AttrDlg[SA_EDIT_ATIME].strData);
			//_SVS(SysLog(L"\n\tSetWriteTime=%d\n\tSetCreationTime=%d\n\tSetLastAccessTime=%d",SetWriteTime,SetCreationTime,SetLastAccessTime));
			int SetWriteTimeRetCode=SETATTR_RET_OK;

			if (SetWriteTime || SetCreationTime || SetLastAccessTime)
			{
				SetWriteTimeRetCode=SkipMode==-1?ESetFileTime(strSelName,SetWriteTime?&LastWriteTime:NULL,SetCreationTime?&CreationTime:NULL,SetLastAccessTime?&LastAccessTime:NULL,FileAttr,SkipMode):SkipMode;
			}

			//if(NewAttr != (FileAttr & (~FILE_ATTRIBUTE_DIRECTORY))) // нужно ли что-нить менять???
			if (SetWriteTimeRetCode == SETATTR_RET_OK) // если время удалось выставить...
			{
				int Ret=SETATTR_RET_OK;

				if ((NewAttr&FILE_ATTRIBUTE_COMPRESSED) && !(FileAttr&FILE_ATTRIBUTE_COMPRESSED))
					Ret=ESetFileCompression(strSelName,1,FileAttr,SkipMode);
				else if (!(NewAttr&FILE_ATTRIBUTE_COMPRESSED) && (FileAttr&FILE_ATTRIBUTE_COMPRESSED))
					Ret=ESetFileCompression(strSelName,0,FileAttr,SkipMode);

				if ((NewAttr&FILE_ATTRIBUTE_ENCRYPTED) && !(FileAttr&FILE_ATTRIBUTE_ENCRYPTED))
					Ret=ESetFileEncryption(strSelName,1,FileAttr,SkipMode);
				else if (!(NewAttr&FILE_ATTRIBUTE_ENCRYPTED) && (FileAttr&FILE_ATTRIBUTE_ENCRYPTED))
					Ret=ESetFileEncryption(strSelName,0,FileAttr,SkipMode);

				if ((NewAttr&FILE_ATTRIBUTE_SPARSE_FILE) && !(FileAttr&FILE_ATTRIBUTE_SPARSE_FILE))
				{
					Ret=ESetFileSparse(strSelName,true,FileAttr,SkipMode);
				}
				else if (!(NewAttr&FILE_ATTRIBUTE_SPARSE_FILE) && (FileAttr&FILE_ATTRIBUTE_SPARSE_FILE))
				{
					Ret=ESetFileSparse(strSelName,false,FileAttr,SkipMode);
				}

				if ((FileAttr&~(FILE_ATTRIBUTE_ENCRYPTED|FILE_ATTRIBUTE_COMPRESSED|FILE_ATTRIBUTE_SPARSE_FILE))!=(NewAttr&~(FILE_ATTRIBUTE_ENCRYPTED|FILE_ATTRIBUTE_COMPRESSED|FILE_ATTRIBUTE_SPARSE_FILE)))
				{
					Ret=ESetFileAttributes(strSelName,NewAttr&~(FILE_ATTRIBUTE_ENCRYPTED|FILE_ATTRIBUTE_COMPRESSED|FILE_ATTRIBUTE_SPARSE_FILE),SkipMode);
				}

				if (Ret==SETATTR_RET_SKIPALL)
					SkipMode=SETATTR_RET_SKIP;
			}
			else if (SetWriteTimeRetCode==SETATTR_RET_SKIPALL)
				SkipMode=SETATTR_RET_SKIP;
		}
		/* Multi *********************************************************** */
		else
		{
			int RetCode=1;
			ConsoleTitle SetAttrTitle(MSG(MSetAttrTitle));
			CtrlObject->Cp()->GetAnotherPanel(SrcPanel)->CloseFile();
			DWORD SetAttr=0,ClearAttr=0;

			for (size_t i=0; i<countof(AP); i++)
			{
				switch (AttrDlg[AP[i].Item].Selected)
				{
					case BSTATE_CHECKED:
						SetAttr|=AP[i].Attribute;
						break;
					case BSTATE_UNCHECKED:
						ClearAttr|=AP[i].Attribute;
						break;
				}
			}

			if (AttrDlg[SA_CHECKBOX_COMPRESSED].Selected==BSTATE_CHECKED)
			{
				ClearAttr|=FILE_ATTRIBUTE_ENCRYPTED;
			}

			if (AttrDlg[SA_CHECKBOX_ENCRYPTED].Selected==BSTATE_CHECKED)
			{
				ClearAttr|=FILE_ATTRIBUTE_COMPRESSED;
			}

			SrcPanel->GetSelName(NULL,FileAttr);
			TaskBar TB;
			bool Cancel=false;
			DWORD LastTime=GetTickCount();

			while (SrcPanel->GetSelName(&strSelName,FileAttr,NULL,&FindData) && !Cancel)
			{
//_SVS(SysLog(L"SelName='%s'\n\tFileAttr =0x%08X\n\tSetAttr  =0x%08X\n\tClearAttr=0x%08X\n\tResult   =0x%08X",
//    SelName,FileAttr,SetAttr,ClearAttr,((FileAttr|SetAttr)&(~ClearAttr))));
				DWORD CurTime=GetTickCount();

				if (CurTime-LastTime>RedrawTimeout)
				{
					LastTime=CurTime;
					ShellSetFileAttributesMsg(strSelName);

					if (CheckForEsc())
						break;
				}

				FILETIME LastWriteTime,CreationTime,LastAccessTime;
				int SetWriteTime=     DlgParam.OLastWriteTime  && ReadFileTime(0,strSelName,LastWriteTime,AttrDlg[SA_EDIT_MDATE].strData,AttrDlg[SA_EDIT_MTIME].strData);
				int SetCreationTime=  DlgParam.OCreationTime   && ReadFileTime(1,strSelName,CreationTime,AttrDlg[SA_EDIT_CDATE].strData,AttrDlg[SA_EDIT_CTIME].strData);
				int SetLastAccessTime=DlgParam.OLastAccessTime && ReadFileTime(2,strSelName,LastAccessTime,AttrDlg[SA_EDIT_ADATE].strData,AttrDlg[SA_EDIT_ATIME].strData);
				RetCode=SkipMode==-1?ESetFileTime(strSelName,SetWriteTime?&LastWriteTime:NULL,SetCreationTime?&CreationTime:NULL,SetLastAccessTime?&LastAccessTime:NULL,FileAttr,SkipMode):SkipMode;

				if (RetCode == SETATTR_RET_ERROR)
					break;
				else if (RetCode == SETATTR_RET_SKIP)
					continue;
				else if (RetCode == SETATTR_RET_SKIPALL)
				{
					SkipMode=SETATTR_RET_SKIP;
					continue;
				}

				if (((FileAttr|SetAttr)&(~ClearAttr)) != FileAttr)
				{
					if (AttrDlg[SA_CHECKBOX_COMPRESSED].Selected != BSTATE_3STATE)
					{
						RetCode=ESetFileCompression(strSelName,AttrDlg[SA_CHECKBOX_COMPRESSED].Selected,FileAttr,SkipMode);

						if (RetCode == SETATTR_RET_ERROR)
							break;
						else if (RetCode == SETATTR_RET_SKIP)
							continue;
						else if (RetCode == SETATTR_RET_SKIPALL)
						{
							SkipMode=SETATTR_RET_SKIP;
							continue;
						}
					}

					if (AttrDlg[SA_CHECKBOX_ENCRYPTED].Selected != BSTATE_3STATE) // +E -C
					{
						if (AttrDlg[SA_CHECKBOX_COMPRESSED].Selected != BSTATE_CHECKED)
						{
							RetCode=ESetFileEncryption(strSelName,AttrDlg[SA_CHECKBOX_ENCRYPTED].Selected,FileAttr,SkipMode);

							if (RetCode == SETATTR_RET_ERROR)
								break;
							else if (RetCode == SETATTR_RET_SKIP)
								continue;
							else if (RetCode == SETATTR_RET_SKIPALL)
							{
								SkipMode=SETATTR_RET_SKIP;
								continue;
							}
						}
					}

					if (AttrDlg[SA_CHECKBOX_SPARSE].Selected!=BSTATE_3STATE)
					{
						RetCode=ESetFileSparse(strSelName,AttrDlg[SA_CHECKBOX_SPARSE].Selected==BSTATE_CHECKED,FileAttr,SkipMode);

						if (RetCode == SETATTR_RET_ERROR)
						{
							break;
						}
						else if (RetCode == SETATTR_RET_SKIP)
						{
							continue;
						}
						else if (RetCode == SETATTR_RET_SKIPALL)
						{
							SkipMode=SETATTR_RET_SKIP;
							continue;
						}
					}

					RetCode=ESetFileAttributes(strSelName,((FileAttr|SetAttr)&(~ClearAttr)),SkipMode);

					if (RetCode == SETATTR_RET_ERROR)
						break;
					else if (RetCode == SETATTR_RET_SKIP)
						continue;
					else if (RetCode == SETATTR_RET_SKIPALL)
					{
						SkipMode=SETATTR_RET_SKIP;
						continue;
					}
				}

				if ((FileAttr & FILE_ATTRIBUTE_DIRECTORY) && AttrDlg[SA_CHECKBOX_SUBFOLDERS].Selected)
				{
					ScanTree ScTree(FALSE);
					ScTree.SetFindPath(strSelName,L"*");
					DWORD LastTime=GetTickCount();
					string strFullName;

					while (ScTree.GetNextName(&FindData,strFullName))
					{
						DWORD CurTime=GetTickCount();

						if (CurTime-LastTime>RedrawTimeout)
						{
							LastTime=CurTime;
							ShellSetFileAttributesMsg(strFullName);

							if (CheckForEsc())
							{
								Cancel=true;
								break;
							}
						}

						SetWriteTime=     DlgParam.OLastWriteTime  && ReadFileTime(0,strFullName,LastWriteTime,AttrDlg[SA_EDIT_MDATE].strData,AttrDlg[SA_EDIT_MTIME].strData);
						SetCreationTime=  DlgParam.OCreationTime   && ReadFileTime(1,strFullName,CreationTime,AttrDlg[SA_EDIT_CDATE].strData,AttrDlg[SA_EDIT_CTIME].strData);
						SetLastAccessTime=DlgParam.OLastAccessTime && ReadFileTime(2,strFullName,LastAccessTime,AttrDlg[SA_EDIT_ADATE].strData,AttrDlg[SA_EDIT_ATIME].strData);

						if (!(FindData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT) && (SetWriteTime || SetCreationTime || SetLastAccessTime))
						{
							RetCode=ESetFileTime(strFullName,SetWriteTime?&LastWriteTime:NULL,SetCreationTime?&CreationTime:NULL,SetLastAccessTime?&LastAccessTime:NULL,FindData.dwFileAttributes,SkipMode);

							if (RetCode == SETATTR_RET_ERROR)
							{
								Cancel=true;
								break;
							}
							else if (RetCode == SETATTR_RET_SKIP)
								continue;
							else if (RetCode == SETATTR_RET_SKIPALL)
							{
								SkipMode=SETATTR_RET_SKIP;
								continue;
							}
						}

						if (((FindData.dwFileAttributes|SetAttr)&(~ClearAttr)) !=
						        FindData.dwFileAttributes)
						{
							if (AttrDlg[SA_CHECKBOX_COMPRESSED].Selected!=BSTATE_3STATE)
							{
								RetCode=ESetFileCompression(strFullName,AttrDlg[SA_CHECKBOX_COMPRESSED].Selected,FindData.dwFileAttributes,SkipMode);

								if (RetCode == SETATTR_RET_ERROR)
								{
									Cancel=true;
									break;
								}
								else if (RetCode == SETATTR_RET_SKIP)
									continue;
								else if (RetCode == SETATTR_RET_SKIPALL)
								{
									SkipMode=SETATTR_RET_SKIP;
									continue;
								}
							}

							if (AttrDlg[SA_CHECKBOX_ENCRYPTED].Selected!=BSTATE_3STATE) // +E -C
							{
								if (AttrDlg[SA_CHECKBOX_COMPRESSED].Selected != 1)
								{
									RetCode=ESetFileEncryption(strFullName,AttrDlg[SA_CHECKBOX_ENCRYPTED].Selected,FindData.dwFileAttributes,SkipMode);

									if (RetCode == SETATTR_RET_ERROR)
									{
										Cancel=true;
										break;
									}
									else if (RetCode == SETATTR_RET_SKIP)
										continue;
									else if (RetCode == SETATTR_RET_SKIPALL)
									{
										SkipMode=SETATTR_RET_SKIP;
										continue;
									}
								}
							}

							if (AttrDlg[SA_CHECKBOX_SPARSE].Selected!=BSTATE_3STATE)
							{
								RetCode=ESetFileSparse(strFullName,AttrDlg[SA_CHECKBOX_SPARSE].Selected==BSTATE_CHECKED,FindData.dwFileAttributes,SkipMode);

								if (RetCode == SETATTR_RET_ERROR)
								{
									Cancel=true;
									break;
								}
								else if (RetCode == SETATTR_RET_SKIP)
								{
									continue;
								}
								else if (RetCode == SETATTR_RET_SKIPALL)
								{
									SkipMode=SETATTR_RET_SKIP;
									continue;
								}
							}

							RetCode=ESetFileAttributes(strFullName,(FindData.dwFileAttributes|SetAttr)&(~ClearAttr),SkipMode);

							if (RetCode == SETATTR_RET_ERROR)
							{
								Cancel=true;
								break;
							}
							else if (RetCode == SETATTR_RET_SKIP)
								continue;
							else if (RetCode == SETATTR_RET_SKIPALL)
							{
								SkipMode=SETATTR_RET_SKIP;
								continue;
							}
						}
					}
				}
			} // END: while (SrcPanel->GetSelName(...))
		}
	}

	SrcPanel->SaveSelection();
	SrcPanel->Update(UPDATE_KEEP_SELECTION);
	SrcPanel->ClearSelection();
	CtrlObject->Cp()->GetAnotherPanel(SrcPanel)->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	CtrlObject->Cp()->Redraw();
	return true;
}
