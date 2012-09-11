/*
setattr.cpp

Установка атрибутов файлов
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

#include "flink.hpp"
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
#include "network.hpp"
#include "fileowner.hpp"
#include "privilege.hpp"
#include "wakeful.hpp"
#include "DlgGuid.hpp"

enum SETATTRDLG
{
	SA_DOUBLEBOX,
	SA_TEXT_LABEL,
	SA_TEXT_NAME,
	SA_COMBO_HARDLINK,
	SA_TEXT_SYMLINK,
	SA_EDIT_SYMLINK,
	SA_SEPARATOR1,
	SA_TEXT_OWNER,
	SA_EDIT_OWNER,
	SA_SEPARATOR2,
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
	SA_CHECKBOX_REPARSEPOINT,
	SA_CHECKBOX_VIRTUAL,
	SA_ATTR_LAST=SA_CHECKBOX_VIRTUAL,
	SA_SEPARATOR3,
	SA_TEXT_TITLEDATE,
	SA_TEXT_LASTWRITE,
	SA_EDIT_WDATE,
	SA_EDIT_WTIME,
	SA_TEXT_CREATION,
	SA_EDIT_CDATE,
	SA_EDIT_CTIME,
	SA_TEXT_LASTACCESS,
	SA_EDIT_ADATE,
	SA_EDIT_ATIME,
	SA_TEXT_CHANGE,
	SA_EDIT_XDATE,
	SA_EDIT_XTIME,
	SA_BUTTON_ORIGINAL,
	SA_BUTTON_CURRENT,
	SA_BUTTON_BLANK,
	SA_SEPARATOR4,
	SA_CHECKBOX_SUBFOLDERS,
	SA_SEPARATOR5,
	SA_BUTTON_SET,
	SA_BUTTON_SYSTEMDLG,
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
	string strOwner;
	bool OwnerChanged;
	// значения CheckBox`ов на момент старта диалога
	int OriginalCBAttr[SA_ATTR_LAST-SA_ATTR_FIRST+1];
	int OriginalCBAttr2[SA_ATTR_LAST-SA_ATTR_FIRST+1];
	FARDIALOGITEMFLAGS OriginalCBFlag[SA_ATTR_LAST-SA_ATTR_FIRST+1];
	FARCHECKEDSTATE OSubfoldersState, OCompressState, OEncryptState;
	bool OLastWriteTime, OCreationTime, OLastAccessTime, OChangeTime;
};

enum
{
	DM_SETATTR = DM_USER+1,
};

intptr_t WINAPI SetAttrDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	SetAttrDlgParam *DlgParam=reinterpret_cast<SetAttrDlgParam*>(SendDlgMessage(hDlg,DM_GETDLGDATA,0,0));

	switch (Msg)
	{
		case DN_BTNCLICK:

			if ((Param1>=SA_ATTR_FIRST&&Param1<=SA_ATTR_LAST)||Param1==SA_CHECKBOX_SUBFOLDERS)
			{
				if(Param1!=SA_CHECKBOX_SUBFOLDERS)
				{
					DlgParam->OriginalCBAttr[Param1-SA_ATTR_FIRST]=static_cast<int>(reinterpret_cast<intptr_t>(Param2));
					DlgParam->OriginalCBAttr2[Param1-SA_ATTR_FIRST]=0;
				}
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
							SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_ENCRYPTED,ToPtr(BSTATE_UNCHECKED));

						if (FocusPos == SA_CHECKBOX_ENCRYPTED && /*EncryptState &&*/ CompressState)
							SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_COMPRESSED,ToPtr(BSTATE_UNCHECKED));
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
								SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_ENCRYPTED,ToPtr(BSTATE_UNCHECKED));
							else if (CompressState == BSTATE_3STATE)
								SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_ENCRYPTED,ToPtr(BSTATE_3STATE));
						}
						else if (FocusPos == SA_CHECKBOX_ENCRYPTED && DlgParam->OEncryptState != EncryptState) // Состояние изменилось?
						{
							if (EncryptState == BSTATE_CHECKED && CompressState)
								SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_COMPRESSED,ToPtr(BSTATE_UNCHECKED));
							else if (EncryptState == BSTATE_3STATE)
								SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_COMPRESSED,ToPtr(BSTATE_3STATE));
						}

						// еще одна проверка
						if (reinterpret_cast<intptr_t>(Param2)==BSTATE_CHECKED)
						{
							if (FocusPos == SA_CHECKBOX_COMPRESSED && EncryptState)
							{
								SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_ENCRYPTED,ToPtr(BSTATE_UNCHECKED));
							}

							if (FocusPos == SA_CHECKBOX_ENCRYPTED && CompressState)
							{
								SendDlgMessage(hDlg,DM_SETCHECK,SA_CHECKBOX_COMPRESSED,ToPtr(BSTATE_UNCHECKED));
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
								LPCWSTR Owner=reinterpret_cast<LPCWSTR>(SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,SA_EDIT_OWNER,0));
								if(*Owner)
								{
									if(!DlgParam->OwnerChanged)
									{
										DlgParam->OwnerChanged=StrCmpI(Owner,DlgParam->strOwner)!=0;
									}
									DlgParam->strOwner=Owner;
								}
								// установили?
								if (SubfoldersState)
								{
									for (int i=SA_ATTR_FIRST; i<=SA_ATTR_LAST; i++)
									{
										SendDlgMessage(hDlg,DM_SET3STATE,i,ToPtr(TRUE));
										if (DlgParam->OriginalCBAttr2[i-SA_ATTR_FIRST]==-1)
										{
											SendDlgMessage(hDlg,DM_SETCHECK,i,ToPtr(BSTATE_3STATE));
										}
									}
									if(!DlgParam->OwnerChanged)
									{
										SendDlgMessage(hDlg,DM_SETTEXTPTR,SA_EDIT_OWNER,nullptr);
									}
								}
								// сняли?
								else
								{
									for (int i=SA_ATTR_FIRST; i<=SA_ATTR_LAST; i++)
									{
										SendDlgMessage(hDlg,DM_SET3STATE,i,FALSE);
										SendDlgMessage(hDlg,DM_SETCHECK,i,ToPtr(DlgParam->OriginalCBAttr[i-SA_ATTR_FIRST]));
									}
									if(!DlgParam->OwnerChanged)
									{
										SendDlgMessage(hDlg,DM_SETTEXTPTR,SA_EDIT_OWNER,const_cast<wchar_t*>(DlgParam->strOwner.CPtr()));
									}
								}


								FAR_FIND_DATA_EX FindData;
								if (apiGetFindDataEx(DlgParam->strSelName, FindData))
								{
									const SETATTRDLG Items[]={SA_TEXT_LASTWRITE,SA_TEXT_CREATION,SA_TEXT_LASTACCESS,SA_TEXT_CHANGE};
									bool* ParamTimes[]={&DlgParam->OLastWriteTime, &DlgParam->OCreationTime, &DlgParam->OLastAccessTime,&DlgParam->OChangeTime};
									const PFILETIME FDTimes[]={&FindData.ftLastWriteTime,&FindData.ftCreationTime,&FindData.ftLastAccessTime,&FindData.ftChangeTime};

									for (size_t i=0; i<ARRAYSIZE(Items); i++)
									{
										if (!*ParamTimes[i])
										{
											SendDlgMessage(hDlg,DM_SETATTR,Items[i],SubfoldersState?0:FDTimes[i]);
											*ParamTimes[i]=false;
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
								LPCWSTR Owner=reinterpret_cast<LPCWSTR>(SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,SA_EDIT_OWNER,0));
								if(*Owner)
								{
									if(!DlgParam->OwnerChanged)
									{
										DlgParam->OwnerChanged=StrCmpI(Owner,DlgParam->strOwner)!=0;
									}
									DlgParam->strOwner=Owner;
								}
								// установили?
								if (SubfoldersState)
								{
									for (int i=SA_ATTR_FIRST; i<= SA_ATTR_LAST; i++)
									{
										if (DlgParam->OriginalCBAttr2[i-SA_ATTR_FIRST]==-1)
										{
											SendDlgMessage(hDlg,DM_SET3STATE,i,ToPtr(TRUE));
											SendDlgMessage(hDlg,DM_SETCHECK,i,ToPtr(BSTATE_3STATE));
										}
									}
									if(!DlgParam->OwnerChanged)
									{
										SendDlgMessage(hDlg,DM_SETTEXTPTR,SA_EDIT_OWNER,nullptr);
									}
								}
								// сняли?
								else
								{
									for (int i=SA_ATTR_FIRST; i<= SA_ATTR_LAST; i++)
									{
										SendDlgMessage(hDlg,DM_SET3STATE,i,ToPtr((DlgParam->OriginalCBFlag[i-SA_ATTR_FIRST]&DIF_3STATE)?TRUE:FALSE));
										SendDlgMessage(hDlg,DM_SETCHECK,i,ToPtr(DlgParam->OriginalCBAttr[i-SA_ATTR_FIRST]));
									}
									if(!DlgParam->OwnerChanged)
									{
										SendDlgMessage(hDlg,DM_SETTEXTPTR,SA_EDIT_OWNER,const_cast<wchar_t*>(DlgParam->strOwner.CPtr()));
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

				if (apiGetFindDataEx(DlgParam->strSelName, FindData))
				{
					SendDlgMessage(hDlg,DM_SETATTR,SA_TEXT_LASTWRITE,&FindData.ftLastWriteTime);
					SendDlgMessage(hDlg,DM_SETATTR,SA_TEXT_CREATION,&FindData.ftCreationTime);
					SendDlgMessage(hDlg,DM_SETATTR,SA_TEXT_LASTACCESS,&FindData.ftLastAccessTime);
					SendDlgMessage(hDlg,DM_SETATTR,SA_TEXT_CHANGE,&FindData.ftChangeTime);
					DlgParam->OLastWriteTime=DlgParam->OCreationTime=DlgParam->OLastAccessTime=DlgParam->OChangeTime=false;
				}

				SendDlgMessage(hDlg,DM_SETFOCUS,SA_EDIT_WDATE,0);
				return TRUE;
			}
			else if (Param1 == SA_BUTTON_CURRENT || Param1 == SA_BUTTON_BLANK)
			{
				void* Value = 0;
				FILETIME CurrentTime;
				if(Param1 == SA_BUTTON_CURRENT)
				{
					GetSystemTimeAsFileTime(&CurrentTime);
					Value = &CurrentTime;
				}
				SendDlgMessage(hDlg, DM_SETATTR, SA_TEXT_LASTWRITE, Value);
				SendDlgMessage(hDlg, DM_SETATTR, SA_TEXT_CREATION, Value);
				SendDlgMessage(hDlg, DM_SETATTR, SA_TEXT_LASTACCESS, Value);
				SendDlgMessage(hDlg, DM_SETATTR, SA_TEXT_CHANGE, Value);
				DlgParam->OLastWriteTime=DlgParam->OCreationTime=DlgParam->OLastAccessTime=DlgParam->OChangeTime==true;
				SendDlgMessage(hDlg,DM_SETFOCUS,SA_EDIT_WDATE,0);
				return TRUE;
			}

			break;
		//BUGBUG: DefDlgProc вызывается дважды, второй раз Param1 может быть другим.
		case DN_CONTROLINPUT:
		{
			const INPUT_RECORD* record=(const INPUT_RECORD *)Param2;
			if (record->EventType==MOUSE_EVENT)
			{
				//_SVS(SysLog(L"Msg=DN_MOUSECLICK Param1=%d Param2=%d",Param1,Param2));
				if (Param1>=SA_TEXT_LASTWRITE && Param1<=SA_EDIT_XTIME)
				{
					if (record->Event.MouseEvent.dwEventFlags==DOUBLE_CLICK)
					{
						// Дадим Менеджеру диалогов "попотеть"
						DefDlgProc(hDlg,Msg,Param1,Param2);
						SendDlgMessage(hDlg,DM_SETATTR,Param1,ToPtr(-1));
					}

					if (Param1 == SA_TEXT_LASTWRITE || Param1 == SA_TEXT_CREATION || Param1 == SA_TEXT_LASTACCESS || Param1 == SA_TEXT_CHANGE)
					{
						Param1++;
					}

					SendDlgMessage(hDlg,DM_SETFOCUS,Param1,0);
				}
			}
		}
		break;
		case DN_EDITCHANGE:
		{
			switch (Param1)
			{
				case SA_COMBO_HARDLINK:
				{
					FarListInfo li={sizeof(FarListInfo)};
					SendDlgMessage(hDlg,DM_LISTINFO,SA_COMBO_HARDLINK,&li);
					SendDlgMessage(hDlg,DM_SETTEXTPTR,SA_COMBO_HARDLINK,const_cast<wchar_t*>((FormatString()<<MSG(MSetAttrHardLinks)<<L" ("<<li.ItemsNumber<<L")").CPtr()));
				}
				break;
				case SA_EDIT_WDATE:
				case SA_EDIT_WTIME:
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
				case SA_EDIT_XDATE:
				case SA_EDIT_XTIME:
					DlgParam->OChangeTime=true;
					break;
				default:
					break;
			}

			break;
		}

		case DN_GOTFOCUS:
			{
				if(Param1 == SA_EDIT_WDATE || Param1 == SA_EDIT_CDATE || Param1 == SA_EDIT_ADATE || Param1 == SA_EDIT_XDATE)
				{
					if(GetDateFormat()==2)
					{
						if(reinterpret_cast<LPCWSTR>(SendDlgMessage(hDlg, DM_GETCONSTTEXTPTR, Param1, 0))[0] == L' ')
						{
							COORD Pos;
							SendDlgMessage(hDlg, DM_GETCURSORPOS, Param1, &Pos);
							if(Pos.X ==0)
							{
								Pos.X=1;
								SendDlgMessage(hDlg, DM_SETCURSORPOS, Param1, &Pos);
							}
						}
					}
				}
			}
			break;

		case DM_SETATTR:
		{
			string strDate,strTime;

			if (Param2) // Set?
			{
				FILETIME ft;

				if (reinterpret_cast<intptr_t>(Param2)==-1)
				{
					GetSystemTimeAsFileTime(&ft);
				}
				else
				{
					ft=*reinterpret_cast<PFILETIME>(Param2);
				}

				ConvertDate(ft,strDate,strTime,12,FALSE,FALSE,2,TRUE);
			}

			// Глянем на место, где был клик
			int Set1=-1;
			int Set2=Param1;

			switch (Param1)
			{
				case SA_TEXT_LASTWRITE:
					Set1=SA_EDIT_WDATE;
					Set2=SA_EDIT_WTIME;
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
				case SA_TEXT_CHANGE:
					Set1=SA_EDIT_XDATE;
					Set2=SA_EDIT_XTIME;
					DlgParam->OChangeTime=true;
					break;
				case SA_EDIT_WDATE:
				case SA_EDIT_CDATE:
				case SA_EDIT_ADATE:
				case SA_EDIT_XDATE:
					Set1=Param1;
					Set2=-1;
					break;
				default:
					break;
			}

			if (Set1!=-1)
			{
				SendDlgMessage(hDlg,DM_SETTEXTPTR,Set1,const_cast<wchar_t*>(strDate.CPtr()));
			}

			if (Set2!=-1)
			{
				SendDlgMessage(hDlg,DM_SETTEXTPTR,Set2,const_cast<wchar_t*>(strTime.CPtr()));
			}

			return TRUE;
		}
	default:
		break;
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

bool ReadFileTime(int Type,const string& Name,FILETIME& FileTime,const wchar_t *OSrcDate,const wchar_t *OSrcTime)
{
	bool Result=false;
	FAR_FIND_DATA_EX ffd={};

	if (apiGetFindDataEx(Name, ffd))
	{
		LPFILETIME Times[]={&ffd.ftLastWriteTime, &ffd.ftCreationTime, &ffd.ftLastAccessTime, &ffd.ftChangeTime};
		LPFILETIME OriginalFileTime=Times[Type];
		FILETIME oft={};
		if(FileTimeToLocalFileTime(OriginalFileTime,&oft))
		{
			SYSTEMTIME ost={};
			if(FileTimeToSystemTime(&oft,&ost))
			{
				WORD DateN[3]={};
				GetFileDateAndTime(OSrcDate,DateN,ARRAYSIZE(DateN),GetDateSeparator());
				WORD TimeN[4]={};
				GetFileDateAndTime(OSrcTime,TimeN,ARRAYSIZE(TimeN),GetTimeSeparator());
				SYSTEMTIME st={};

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
					st.wYear = static_cast<WORD>(ConvertYearToFull(st.wYear));
				}

				FILETIME lft={};
				if (SystemTimeToFileTime(&st,&lft))
				{
					if(LocalFileTimeToFileTime(&lft,&FileTime))
					{
						Result=CompareFileTime(&FileTime,OriginalFileTime)!=0;
					}
				}
			}
		}
	}
	return Result;
}

void PR_ShellSetFileAttributesMsg()
{
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	ShellSetFileAttributesMsg(static_cast<const wchar_t*>(preRedrawItem.Param.Param1));
}

bool ShellSetFileAttributes(Panel *SrcPanel, const string* Object)
{
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
	short DlgX=70,DlgY=24;
	FarDialogItem AttrDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,DlgX-4,DlgY-2,0,nullptr,nullptr,0,MSG(MSetAttrTitle)},
		{DI_TEXT,-1,2,0,2,0,nullptr,nullptr,0,MSG(MSetAttrFor)},
		{DI_TEXT,-1,3,0,3,0,nullptr,nullptr,DIF_SHOWAMPERSAND,L""},
		{DI_COMBOBOX,5,3,DlgX-6,3,0,nullptr,nullptr,DIF_SHOWAMPERSAND|DIF_DROPDOWNLIST|DIF_LISTWRAPMODE|DIF_HIDDEN,L""},
		{DI_TEXT,5,3,17,3,0,nullptr,nullptr,DIF_HIDDEN,L""},
		{DI_EDIT,18,3,DlgX-6,3,0,nullptr,nullptr,DIF_HIDDEN|DIF_EDITPATH,L""},
		{DI_TEXT,3,4,0,4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_TEXT,5,5,17,5,0,nullptr,nullptr,0,MSG(MSetAttrOwner)},
		{DI_EDIT,18,5,DlgX-6,5,0,nullptr,nullptr,0,L""},
		{DI_TEXT,3,6,0,6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5, 7,0,7,0,nullptr,nullptr,DIF_FOCUS|DIF_3STATE,MSG(MSetAttrRO)},
		{DI_CHECKBOX,5, 8,0,8,0,nullptr,nullptr,DIF_3STATE,MSG(MSetAttrArchive)},
		{DI_CHECKBOX,5, 9,0,9,0,nullptr,nullptr,DIF_3STATE,MSG(MSetAttrHidden)},
		{DI_CHECKBOX,5,10,0,10,0,nullptr,nullptr,DIF_3STATE,MSG(MSetAttrSystem)},
		{DI_CHECKBOX,5,11,0,11,0,nullptr,nullptr,DIF_3STATE,MSG(MSetAttrCompressed)},
		{DI_CHECKBOX,5,12,0,12,0,nullptr,nullptr,DIF_3STATE,MSG(MSetAttrEncrypted)},
		{DI_CHECKBOX,DlgX/2,7,0,7,0,nullptr,nullptr,DIF_3STATE,MSG(MSetAttrNotIndexed)},
		{DI_CHECKBOX,DlgX/2,8,0,8,0,nullptr,nullptr,DIF_3STATE,MSG(MSetAttrSparse)},
		{DI_CHECKBOX,DlgX/2,9,0,9,0,nullptr,nullptr,DIF_3STATE,MSG(MSetAttrTemp)},
		{DI_CHECKBOX,DlgX/2,10,0,10,0,nullptr,nullptr,DIF_3STATE,MSG(MSetAttrOffline)},
		{DI_CHECKBOX,DlgX/2,11,0,11,0,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MSetAttrReparsePoint)},
		{DI_CHECKBOX,DlgX/2,12,0,12,0,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MSetAttrVirtual)},
		{DI_TEXT,3,13,0,13,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_TEXT,DlgX-29,14,0,14,0,nullptr,nullptr,0,L""},
		{DI_TEXT,    5,15,0,15,0,nullptr,nullptr,0,MSG(MSetAttrModification)},
		{DI_FIXEDIT,DlgX-29,15,DlgX-19,15,0,nullptr,nullptr,DIF_MASKEDIT,L""},
		{DI_FIXEDIT,DlgX-17,15,DlgX-6,15,0,nullptr,nullptr,DIF_MASKEDIT,L""},
		{DI_TEXT,    5,16,0,16,0,nullptr,nullptr,0,MSG(MSetAttrCreation)},
		{DI_FIXEDIT,DlgX-29,16,DlgX-19,16,0,nullptr,nullptr,DIF_MASKEDIT,L""},
		{DI_FIXEDIT,DlgX-17,16,DlgX-6,16,0,nullptr,nullptr,DIF_MASKEDIT,L""},
		{DI_TEXT,    5,17,0,17,0,nullptr,nullptr,0,MSG(MSetAttrLastAccess)},
		{DI_FIXEDIT,DlgX-29,17,DlgX-19,17,0,nullptr,nullptr,DIF_MASKEDIT,L""},
		{DI_FIXEDIT,DlgX-17,17,DlgX-6,17,0,nullptr,nullptr,DIF_MASKEDIT,L""},
		{DI_TEXT,    5,18,0,18,0,nullptr,nullptr,0,MSG(MSetAttrChange)},
		{DI_FIXEDIT,DlgX-29,18,DlgX-19,18,0,nullptr,nullptr,DIF_MASKEDIT,L""},
		{DI_FIXEDIT,DlgX-17,18,DlgX-6,18,0,nullptr,nullptr,DIF_MASKEDIT,L""},

		{DI_BUTTON,0,19,0,19,0,nullptr,nullptr,DIF_CENTERGROUP|DIF_BTNNOCLOSE,MSG(MSetAttrOriginal)},
		{DI_BUTTON,0,19,0,19,0,nullptr,nullptr,DIF_CENTERGROUP|DIF_BTNNOCLOSE,MSG(MSetAttrCurrent)},
		{DI_BUTTON,0,19,0,19,0,nullptr,nullptr,DIF_CENTERGROUP|DIF_BTNNOCLOSE,MSG(MSetAttrBlank)},
		{DI_TEXT,3,20,0,20,0,nullptr,nullptr,DIF_SEPARATOR|DIF_HIDDEN,L""},
		{DI_CHECKBOX,5,21,0,21,0,nullptr,nullptr,DIF_DISABLE|DIF_HIDDEN,MSG(MSetAttrSubfolders)},
		{DI_TEXT,3,DlgY-4,0,DlgY-4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,DlgY-3,0,DlgY-3,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MSetAttrSet)},
		{DI_BUTTON,0,DlgY-3,0,DlgY-3,0,nullptr,nullptr,DIF_CENTERGROUP|DIF_DISABLE,MSG(MSetAttrSystemDialog)},
		{DI_BUTTON,0,DlgY-3,0,DlgY-3,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	MakeDialogItemsEx(AttrDlgData,AttrDlg);
	SetAttrDlgParam DlgParam={};
	size_t SelCount=SrcPanel?SrcPanel->GetSelCount():1;

	if (!SelCount)
	{
		return false;
	}

	if(SelCount==1)
	{
		AttrDlg[SA_BUTTON_SYSTEMDLG].Flags&=~DIF_DISABLE;
	}

	if (SrcPanel && SrcPanel->GetMode()==PLUGIN_PANEL)
	{
		OpenPanelInfo Info;
		HANDLE hPlugin=SrcPanel->GetPluginHandle();

		if (hPlugin == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);

		if (!(Info.Flags & OPIF_REALNAMES))
		{
			AttrDlg[SA_BUTTON_SET].Flags|=DIF_DISABLE;
			AttrDlg[SA_BUTTON_SYSTEMDLG].Flags|=DIF_DISABLE;
			DlgParam.Plugin=true;
		}
	}

	FarList NameList={};
	string *strLinks=nullptr;

	if (!DlgParam.Plugin)
	{
		string strRootPathName;
		apiGetCurrentDirectory(strRootPathName);
		GetPathRoot(strRootPathName,strRootPathName);

		if (apiGetVolumeInformation(strRootPathName,nullptr,0,nullptr,&DlgParam.FileSystemFlags,nullptr))
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
		DWORD FileAttr=INVALID_FILE_ATTRIBUTES;
		string strSelName;
		FAR_FIND_DATA_EX FindData;
		if(SrcPanel)
		{
			SrcPanel->GetSelName(nullptr,FileAttr);
			SrcPanel->GetSelName(&strSelName,FileAttr,nullptr,&FindData);
		}
		else
		{
			strSelName=*Object;
			apiGetFindDataEx(strSelName, FindData);
			FileAttr=FindData.dwFileAttributes;
		}

		if (!SelCount || (SelCount==1 && TestParentFolderName(strSelName)))
			return false;

		wchar_t DateSeparator=GetDateSeparator();
		wchar_t TimeSeparator=GetTimeSeparator();
		wchar_t DecimalSeparator=GetDecimalSeparator();
		LPCWSTR FmtMask1=L"99%c99%c99%c999",FmtMask2=L"99%c99%c9999N",FmtMask3=L"N9999%c99%c99";
		string strDMask, strTMask;
		strTMask.Format(FmtMask1,TimeSeparator,TimeSeparator,DecimalSeparator);

		LangString DateFormat;

		switch (GetDateFormat())
		{
			case 0:
				DateFormat = MSetAttrTimeTitle1;
				DateFormat << DateSeparator << DateSeparator << TimeSeparator << TimeSeparator << DecimalSeparator;
				strDMask.Format(FmtMask2,DateSeparator,DateSeparator);
				break;
			case 1:
				DateFormat = MSetAttrTimeTitle2;
				DateFormat << DateSeparator << DateSeparator << TimeSeparator << TimeSeparator << DecimalSeparator;
				strDMask.Format(FmtMask2,DateSeparator,DateSeparator);
				break;
			default:
				DateFormat = MSetAttrTimeTitle3;
				DateFormat << DateSeparator << DateSeparator << TimeSeparator << TimeSeparator << DecimalSeparator;
				strDMask.Format(FmtMask3,DateSeparator,DateSeparator);
				break;
		}
		AttrDlg[SA_TEXT_TITLEDATE].strData = DateFormat;

		AttrDlg[SA_EDIT_WDATE].strMask=AttrDlg[SA_EDIT_CDATE].strMask=AttrDlg[SA_EDIT_ADATE].strMask=AttrDlg[SA_EDIT_XDATE].strMask=strDMask;
		AttrDlg[SA_EDIT_WTIME].strMask=AttrDlg[SA_EDIT_CTIME].strMask=AttrDlg[SA_EDIT_ATIME].strMask=AttrDlg[SA_EDIT_XTIME].strMask=strTMask;
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
			{SA_CHECKBOX_REPARSEPOINT,FILE_ATTRIBUTE_REPARSE_POINT},
			{SA_CHECKBOX_VIRTUAL,FILE_ATTRIBUTE_VIRTUAL},
		};

		if (SelCount==1)
		{
			if (FileAttr&FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!DlgParam.Plugin)
				{
					DWORD AddFileAttr=apiGetFileAttributes(strSelName);
					if (AddFileAttr != INVALID_FILE_ATTRIBUTES)
						FileAttr|=AddFileAttr;
				}

				//_SVS(SysLog(L"SelName=%s  FileAttr=0x%08X",SelName,FileAttr));
				AttrDlg[SA_SEPARATOR4].Flags&=~DIF_HIDDEN;
				AttrDlg[SA_CHECKBOX_SUBFOLDERS].Flags&=~(DIF_DISABLE|DIF_HIDDEN);
				AttrDlg[SA_CHECKBOX_SUBFOLDERS].Selected=Opt.SetAttrFolderRules?BSTATE_UNCHECKED:BSTATE_CHECKED;
				AttrDlg[SA_DOUBLEBOX].Y2+=2;
				for(int i=SA_SEPARATOR5;i<=SA_BUTTON_CANCEL;i++)
				{
					AttrDlg[i].Y1+=2;
					AttrDlg[i].Y2+=2;
				}
				DlgY+=2;

				if (Opt.SetAttrFolderRules)
				{
					if (DlgParam.Plugin || apiGetFindDataEx(strSelName, FindData))
					{
						ConvertDate(FindData.ftLastWriteTime, AttrDlg[SA_EDIT_WDATE].strData,AttrDlg[SA_EDIT_WTIME].strData,12,FALSE,FALSE,2,TRUE);
						ConvertDate(FindData.ftCreationTime,  AttrDlg[SA_EDIT_CDATE].strData,AttrDlg[SA_EDIT_CTIME].strData,12,FALSE,FALSE,2,TRUE);
						ConvertDate(FindData.ftLastAccessTime,AttrDlg[SA_EDIT_ADATE].strData,AttrDlg[SA_EDIT_ATIME].strData,12,FALSE,FALSE,2,TRUE);
						ConvertDate(FindData.ftChangeTime,    AttrDlg[SA_EDIT_XDATE].strData,AttrDlg[SA_EDIT_XTIME].strData,12,FALSE,FALSE,2,TRUE);
					}

					if (FileAttr!=INVALID_FILE_ATTRIBUTES)
					{
						for (size_t i=0; i<ARRAYSIZE(AP); i++)
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
			if (FileAttr!=INVALID_FILE_ATTRIBUTES && (FileAttr&FILE_ATTRIBUTE_REPARSE_POINT))
			{
				DWORD ReparseTag=0;
				DWORD LenJunction=DlgParam.Plugin?0:GetReparsePointInfo(strSelName, strLinkName,&ReparseTag);
				AttrDlg[SA_DOUBLEBOX].Y2++;

				for (size_t i=SA_TEXT_SYMLINK; i<ARRAYSIZE(AttrDlgData); i++)
				{
					AttrDlg[i].Y1++;

					if (AttrDlg[i].Y2)
					{
						AttrDlg[i].Y2++;
					}
				}

				LinkPresent=true;
				NormalizeSymlinkName(strLinkName);
				LNGID ID_Msg=MSetAttrSymlink;

				if (ReparseTag==IO_REPARSE_TAG_MOUNT_POINT)
				{
					bool Root;
					if(ParsePath(strLinkName, nullptr, &Root) == PATH_VOLUMEGUID && Root)
					{
						ID_Msg=MSetAttrVolMount;
					}
					else
					{
						ID_Msg=MSetAttrJunction;
					}
				}

				if (!LenJunction)
					strLinkName=MSG(MSetAttrUnknownJunction);

				AttrDlg[SA_TEXT_SYMLINK].Flags&=~DIF_HIDDEN;
				AttrDlg[SA_TEXT_SYMLINK].strData=MSG(ID_Msg);
				AttrDlg[SA_EDIT_SYMLINK].Flags&=~DIF_HIDDEN;
				AttrDlg[SA_EDIT_SYMLINK].strData=strLinkName.CPtr();

				DlgParam.FileSystemFlags=0;
				string strRoot;
				GetPathRoot(strSelName,strRoot);

				if (apiGetVolumeInformation(strRoot,nullptr,0,nullptr,&DlgParam.FileSystemFlags,nullptr))
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
			NameList.ItemsNumber=(FileAttr&FILE_ATTRIBUTE_DIRECTORY)?1:GetNumberOfLinks(strSelName);

			if (NameList.ItemsNumber>1)
			{
				AttrDlg[SA_TEXT_NAME].Flags|=DIF_HIDDEN;
				AttrDlg[SA_COMBO_HARDLINK].Flags&=~DIF_HIDDEN;
				NameList.Items=new FarListItem[NameList.ItemsNumber]();
				strLinks=new string[NameList.ItemsNumber];
				HANDLE hFind=apiFindFirstFileName(strSelName,0,strLinks[0]);

				if (hFind!=INVALID_HANDLE_VALUE)
				{
					string strRoot;
					GetPathRoot(strSelName,strRoot);
					DeleteEndSlash(strRoot);
					strLinks[0]=strRoot+strLinks[0];
					int Current=0;
					NameList.Items[Current++].Text=strLinks[0];

					while (apiFindNextFileName(hFind,strLinks[Current]))
					{
						strLinks[Current]=strRoot+strLinks[Current];
						NameList.Items[Current].Text=strLinks[Current];
						Current++;
					}

					FindClose(hFind);
					AttrDlg[SA_COMBO_HARDLINK].ListItems=&NameList;
				}
				else
				{
					AttrDlg[SA_COMBO_HARDLINK].Flags|=DIF_DISABLE;
				}

				AttrDlg[SA_COMBO_HARDLINK].strData=FormatString()<<MSG(MSetAttrHardLinks)<<L" ("<<NameList.ItemsNumber<<L")";
			}

			AttrDlg[SA_TEXT_NAME].strData = strSelName;
			TruncStr(AttrDlg[SA_TEXT_NAME].strData,DlgX-10);

			if (FileAttr!=INVALID_FILE_ATTRIBUTES)
			{
				for (size_t i=0; i<ARRAYSIZE(AP); i++)
				{
					AttrDlg[AP[i].Item].Selected=FileAttr&AP[i].Attribute?BSTATE_CHECKED:BSTATE_UNCHECKED;
				}
			}

			const SETATTRDLG Dates[]={SA_EDIT_WDATE,SA_EDIT_CDATE,SA_EDIT_ADATE,SA_EDIT_XDATE},Times[]={SA_EDIT_WTIME,SA_EDIT_CTIME,SA_EDIT_ATIME,SA_EDIT_XTIME};
			const PFILETIME TimeValues[]={&FindData.ftLastWriteTime,&FindData.ftCreationTime,&FindData.ftLastAccessTime,&FindData.ftChangeTime};

			if (DlgParam.Plugin || (!DlgParam.Plugin&&apiGetFindDataEx(strSelName, FindData)))
			{
				for (size_t i=0; i<ARRAYSIZE(Dates); i++)
				{
					ConvertDate(*TimeValues[i],AttrDlg[Dates[i]].strData,AttrDlg[Times[i]].strData,12,FALSE,FALSE,2,TRUE);
				}
			}

			string strComputerName;
			if(SrcPanel)
			{
				string strCurDir;
				SrcPanel->GetCurDir(strCurDir);
				CurPath2ComputerName(strCurDir, strComputerName);
			}
			GetFileOwner(strComputerName,strSelName,AttrDlg[SA_EDIT_OWNER].strData);
		}
		else
		{
			for (size_t i=0; i<ARRAYSIZE(AP); i++)
			{
				AttrDlg[AP[i].Item].Selected=BSTATE_3STATE;
			}

			AttrDlg[SA_EDIT_WDATE].strData.Clear();
			AttrDlg[SA_EDIT_WTIME].strData.Clear();
			AttrDlg[SA_EDIT_CDATE].strData.Clear();
			AttrDlg[SA_EDIT_CTIME].strData.Clear();
			AttrDlg[SA_EDIT_ADATE].strData.Clear();
			AttrDlg[SA_EDIT_ATIME].strData.Clear();
			AttrDlg[SA_EDIT_XDATE].strData.Clear();
			AttrDlg[SA_EDIT_XTIME].strData.Clear();
			AttrDlg[SA_BUTTON_ORIGINAL].Flags|=DIF_DISABLE;
			AttrDlg[SA_TEXT_NAME].strData = MSG(MSetAttrSelectedObjects);

			for (size_t i=SA_ATTR_FIRST; i<=SA_ATTR_LAST; i++)
			{
				AttrDlg[i].Selected=BSTATE_UNCHECKED;
			}

			// проверка - есть ли среди выделенных - каталоги?
			// так же проверка на атрибуты
			if(SrcPanel)
			{
				SrcPanel->GetSelName(nullptr,FileAttr);
			}
			FolderPresent=false;

			if(SrcPanel)
			{
				string strComputerName;
				string strCurDir;
				SrcPanel->GetCurDir(strCurDir);
				CurPath2ComputerName(strCurDir, strComputerName);

				bool CheckOwner=true;
				while (SrcPanel->GetSelName(&strSelName,FileAttr,nullptr,&FindData))
				{
					if (!FolderPresent&&(FileAttr&FILE_ATTRIBUTE_DIRECTORY))
					{
						FolderPresent=true;
						AttrDlg[SA_SEPARATOR4].Flags&=~DIF_HIDDEN;
						AttrDlg[SA_CHECKBOX_SUBFOLDERS].Flags&=~(DIF_DISABLE|DIF_HIDDEN);
						AttrDlg[SA_DOUBLEBOX].Y2+=2;
						for(int i=SA_SEPARATOR5;i<=SA_BUTTON_CANCEL;i++)
						{
							AttrDlg[i].Y1+=2;
							AttrDlg[i].Y2+=2;
						}
						DlgY+=2;
					}

					for (size_t i=0; i<ARRAYSIZE(AP); i++)
					{
						if (FileAttr&AP[i].Attribute)
						{
							AttrDlg[AP[i].Item].Selected++;
						}
					}
					if(CheckOwner)
					{
						string strCurOwner;
						GetFileOwner(strComputerName,strSelName,strCurOwner);
						if(AttrDlg[SA_EDIT_OWNER].strData.IsEmpty())
						{
							AttrDlg[SA_EDIT_OWNER].strData=strCurOwner;
						}
						else if(AttrDlg[SA_EDIT_OWNER].strData != strCurOwner)
						{
							AttrDlg[SA_EDIT_OWNER].strData=MSG(MSetAttrOwnerMultiple);
							CheckOwner=false;
						}
					}
				}
			}
			else
			{
				// BUGBUG, copy-paste
				if (!FolderPresent&&(FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
				{
					FolderPresent=true;
					AttrDlg[SA_SEPARATOR4].Flags&=~DIF_HIDDEN;
					AttrDlg[SA_CHECKBOX_SUBFOLDERS].Flags&=~(DIF_DISABLE|DIF_HIDDEN);
					AttrDlg[SA_DOUBLEBOX].Y2+=2;
					for(int i=SA_SEPARATOR5;i<=SA_BUTTON_CANCEL;i++)
					{
						AttrDlg[i].Y1+=2;
						AttrDlg[i].Y2+=2;
					}
					DlgY+=2;
				}
				for (size_t i=0; i<ARRAYSIZE(AP); i++)
				{
					if (FindData.dwFileAttributes&AP[i].Attribute)
					{
						AttrDlg[AP[i].Item].Selected++;
					}
				}
			}
			if(SrcPanel)
			{
				SrcPanel->GetSelName(nullptr,FileAttr);
				SrcPanel->GetSelName(&strSelName,FileAttr,nullptr,&FindData);
			}

			// выставим "неопределенку" или то, что нужно
			for (size_t i=SA_ATTR_FIRST; i<=SA_ATTR_LAST; i++)
			{
				// снимаем 3-state, если "есть все или нет ничего"
				// за исключением случая, если есть Фолдер среди объектов
				if ((!AttrDlg[i].Selected || static_cast<size_t>(AttrDlg[i].Selected) >= SelCount) && !FolderPresent)
				{
					AttrDlg[i].Flags&=~DIF_3STATE;
				}

				AttrDlg[i].Selected=(static_cast<size_t>(AttrDlg[i].Selected) >= SelCount)?BST_CHECKED:(!AttrDlg[i].Selected?BSTATE_UNCHECKED:BSTATE_3STATE);
			}
		}

		// запомним состояние переключателей.
		for (size_t i=SA_ATTR_FIRST; i<=SA_ATTR_LAST; i++)
		{
			DlgParam.OriginalCBAttr[i-SA_ATTR_FIRST]=AttrDlg[i].Selected;
			DlgParam.OriginalCBAttr2[i-SA_ATTR_FIRST]=-1;
			DlgParam.OriginalCBFlag[i-SA_ATTR_FIRST]=AttrDlg[i].Flags;
		}
		DlgParam.strOwner=AttrDlg[SA_EDIT_OWNER].strData;
		string strInitOwner=AttrDlg[SA_EDIT_OWNER].strData;

		// поведение для каталогов как у 1.65?
		if (FolderPresent && !Opt.SetAttrFolderRules)
		{
			AttrDlg[SA_CHECKBOX_SUBFOLDERS].Selected=BSTATE_CHECKED;
			AttrDlg[SA_EDIT_WDATE].strData.Clear();
			AttrDlg[SA_EDIT_WTIME].strData.Clear();
			AttrDlg[SA_EDIT_CDATE].strData.Clear();
			AttrDlg[SA_EDIT_CTIME].strData.Clear();
			AttrDlg[SA_EDIT_ADATE].strData.Clear();
			AttrDlg[SA_EDIT_ATIME].strData.Clear();
			AttrDlg[SA_EDIT_XDATE].strData.Clear();
			AttrDlg[SA_EDIT_XTIME].strData.Clear();

			for (size_t i=SA_ATTR_FIRST; i<= SA_ATTR_LAST; i++)
			{
				AttrDlg[i].Selected=BSTATE_3STATE;
				AttrDlg[i].Flags|=DIF_3STATE;
			}
			AttrDlg[SA_EDIT_OWNER].strData.Clear();
		}

		DlgParam.DialogMode=((SelCount==1&&!(FileAttr&FILE_ATTRIBUTE_DIRECTORY))?MODE_FILE:(SelCount==1?MODE_FOLDER:MODE_MULTIPLE));
		DlgParam.strSelName=strSelName;
		DlgParam.OSubfoldersState=static_cast<FARCHECKEDSTATE>(AttrDlg[SA_CHECKBOX_SUBFOLDERS].Selected);
		DlgParam.OCompressState=static_cast<FARCHECKEDSTATE>(AttrDlg[SA_CHECKBOX_COMPRESSED].Selected);
		DlgParam.OEncryptState=static_cast<FARCHECKEDSTATE>(AttrDlg[SA_CHECKBOX_ENCRYPTED].Selected);

		Dialog Dlg(AttrDlg,ARRAYSIZE(AttrDlgData),SetAttrDlgProc,&DlgParam);
		Dlg.SetHelp(L"FileAttrDlg");                 //  ^ - это одиночный диалог!
		Dlg.SetId(FileAttrDlgId);

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

		switch(Dlg.GetExitCode())
		{
		case SA_BUTTON_SET:
			{
				//reparse point editor
				if (StrCmpI(AttrDlg[SA_EDIT_SYMLINK].strData,strLinkName))
				{
					string strTarget = AttrDlg[SA_EDIT_SYMLINK].strData;
					Unquote(strTarget);
					if(!ModifyReparsePoint(strSelName, strTarget))
					{
						Message(FMSG_WARNING|FMSG_ERRORTYPE,1,MSG(MError),MSG(MCopyCannotCreateLink),strSelName,MSG(MHOk));
					}
				}

				const size_t Times[]={SA_EDIT_WTIME,SA_EDIT_CTIME,SA_EDIT_ATIME,SA_EDIT_XTIME};

				for (size_t i=0; i<ARRAYSIZE(Times); i++)
				{
					LPWSTR TimePtr=AttrDlg[Times[i]].strData.GetBuffer();
					TimePtr[8]=GetTimeSeparator();
					AttrDlg[Times[i]].strData.ReleaseBuffer(AttrDlg[Times[i]].strData.GetLength());
				}

				TPreRedrawFuncGuard preRedrawFuncGuard(PR_ShellSetFileAttributesMsg);
				ShellSetFileAttributesMsg(SelCount==1?strSelName.CPtr():nullptr);
				int SkipMode=-1;

				if (SelCount==1 && !(FileAttr & FILE_ATTRIBUTE_DIRECTORY))
				{
					DWORD NewAttr=FileAttr&FILE_ATTRIBUTE_DIRECTORY;

					for (size_t i=0; i<ARRAYSIZE(AP); i++)
					{
						if (AttrDlg[AP[i].Item].Selected)
						{
							NewAttr|=AP[i].Attribute;
						}
					}

					if(!AttrDlg[SA_EDIT_OWNER].strData.IsEmpty() && StrCmpI(strInitOwner,AttrDlg[SA_EDIT_OWNER].strData))
					{
						int Result=ESetFileOwner(strSelName,AttrDlg[SA_EDIT_OWNER].strData,SkipMode);
						if(Result==SETATTR_RET_SKIPALL)
						{
							SkipMode=SETATTR_RET_SKIP;
						}
						else if(Result==SETATTR_RET_ERROR)
						{
							break;
						}
					}

					FILETIME LastWriteTime={},CreationTime={},LastAccessTime={}, ChangeTime={};
					int SetWriteTime=     DlgParam.OLastWriteTime  && ReadFileTime(0,strSelName,LastWriteTime,AttrDlg[SA_EDIT_WDATE].strData,AttrDlg[SA_EDIT_WTIME].strData);
					int SetCreationTime=  DlgParam.OCreationTime   && ReadFileTime(1,strSelName,CreationTime,AttrDlg[SA_EDIT_CDATE].strData,AttrDlg[SA_EDIT_CTIME].strData);
					int SetLastAccessTime=DlgParam.OLastAccessTime && ReadFileTime(2,strSelName,LastAccessTime,AttrDlg[SA_EDIT_ADATE].strData,AttrDlg[SA_EDIT_ATIME].strData);
					int SetChangeTime=    DlgParam.OChangeTime     && ReadFileTime(3,strSelName,ChangeTime,AttrDlg[SA_EDIT_XDATE].strData,AttrDlg[SA_EDIT_XTIME].strData);
					//_SVS(SysLog(L"\n\tSetWriteTime=%d\n\tSetCreationTime=%d\n\tSetLastAccessTime=%d",SetWriteTime,SetCreationTime,SetLastAccessTime));

					if (SetWriteTime || SetCreationTime || SetLastAccessTime || SetChangeTime)
					{
						if(ESetFileTime(strSelName,SetWriteTime?&LastWriteTime:nullptr,SetCreationTime?&CreationTime:nullptr,SetLastAccessTime?&LastAccessTime:nullptr,SetChangeTime?&ChangeTime:nullptr,FileAttr,SkipMode)==SETATTR_RET_SKIPALL)
						{
							SkipMode=SETATTR_RET_SKIP;
						}
					}

					if ((NewAttr&FILE_ATTRIBUTE_COMPRESSED) && !(FileAttr&FILE_ATTRIBUTE_COMPRESSED))
					{
						if (ESetFileCompression(strSelName,1,FileAttr,SkipMode)==SETATTR_RET_SKIPALL)
						{
							SkipMode=SETATTR_RET_SKIP;
						}
					}
					else if (!(NewAttr&FILE_ATTRIBUTE_COMPRESSED) && (FileAttr&FILE_ATTRIBUTE_COMPRESSED))
					{
						if(ESetFileCompression(strSelName,0,FileAttr,SkipMode)==SETATTR_RET_SKIPALL)
						{
							SkipMode=SETATTR_RET_SKIP;
						}
					}

					if ((NewAttr&FILE_ATTRIBUTE_ENCRYPTED) && !(FileAttr&FILE_ATTRIBUTE_ENCRYPTED))
					{
						if (ESetFileEncryption(strSelName,1,FileAttr,SkipMode)==SETATTR_RET_SKIPALL)
						{
							SkipMode=SETATTR_RET_SKIP;
						}
					}
					else if (!(NewAttr&FILE_ATTRIBUTE_ENCRYPTED) && (FileAttr&FILE_ATTRIBUTE_ENCRYPTED))
					{
						if (ESetFileEncryption(strSelName,0,FileAttr,SkipMode)==SETATTR_RET_SKIPALL)
						{
							SkipMode=SETATTR_RET_SKIP;
						}
					}

					if ((NewAttr&FILE_ATTRIBUTE_SPARSE_FILE) && !(FileAttr&FILE_ATTRIBUTE_SPARSE_FILE))
					{
						if (ESetFileSparse(strSelName,true,FileAttr,SkipMode)==SETATTR_RET_SKIPALL)
						{
							SkipMode=SETATTR_RET_SKIP;
						}
					}
					else if (!(NewAttr&FILE_ATTRIBUTE_SPARSE_FILE) && (FileAttr&FILE_ATTRIBUTE_SPARSE_FILE))
					{
						if (ESetFileSparse(strSelName,false,FileAttr,SkipMode)==SETATTR_RET_SKIPALL)
						{
							SkipMode=SETATTR_RET_SKIP;
						}
					}

					if ((FileAttr&~(FILE_ATTRIBUTE_ENCRYPTED|FILE_ATTRIBUTE_COMPRESSED|FILE_ATTRIBUTE_SPARSE_FILE))!=(NewAttr&~(FILE_ATTRIBUTE_ENCRYPTED|FILE_ATTRIBUTE_COMPRESSED|FILE_ATTRIBUTE_SPARSE_FILE)))
					{
						if (ESetFileAttributes(strSelName,NewAttr&~(FILE_ATTRIBUTE_ENCRYPTED|FILE_ATTRIBUTE_COMPRESSED|FILE_ATTRIBUTE_SPARSE_FILE),SkipMode)==SETATTR_RET_SKIPALL)
						{
							SkipMode=SETATTR_RET_SKIP;
						}
					}
				}
				/* Multi *********************************************************** */
				else
				{
					int RetCode=1;
					ConsoleTitle SetAttrTitle(MSG(MSetAttrTitle));
					if(SrcPanel)
					{
						CtrlObject->Cp()->GetAnotherPanel(SrcPanel)->CloseFile();
					}
					DWORD SetAttr=0,ClearAttr=0;

					for (size_t i=0; i<ARRAYSIZE(AP); i++)
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

					if(SrcPanel)
					{
						SrcPanel->GetSelName(nullptr,FileAttr);
					}
					TaskBar TB;
					wakeful W;
					bool Cancel=false;
					DWORD LastTime=0;

					bool SingleFileDone=false;
					while ((SrcPanel?SrcPanel->GetSelName(&strSelName,FileAttr,nullptr,&FindData):!SingleFileDone) && !Cancel)
					{
						if(!SrcPanel)
						{
							SingleFileDone=true;
						}
		//_SVS(SysLog(L"SelName='%s'\n\tFileAttr =0x%08X\n\tSetAttr  =0x%08X\n\tClearAttr=0x%08X\n\tResult   =0x%08X",
		//    SelName,FileAttr,SetAttr,ClearAttr,((FileAttr|SetAttr)&(~ClearAttr))));
						DWORD CurTime=GetTickCount();

						if (CurTime-LastTime>(DWORD)Opt.RedrawTimeout)
						{
							LastTime=CurTime;
							ShellSetFileAttributesMsg(strSelName);

							if (CheckForEsc())
								break;
						}

						if(!AttrDlg[SA_EDIT_OWNER].strData.IsEmpty() && StrCmpI(strInitOwner,AttrDlg[SA_EDIT_OWNER].strData))
						{
							int Result=ESetFileOwner(strSelName,AttrDlg[SA_EDIT_OWNER].strData,SkipMode);
							if(Result==SETATTR_RET_SKIPALL)
							{
								SkipMode=SETATTR_RET_SKIP;
							}
							else if(Result==SETATTR_RET_ERROR)
							{
								break;
							}
						}

						FILETIME LastWriteTime,CreationTime,LastAccessTime, ChangeTime;
						int SetWriteTime=     DlgParam.OLastWriteTime  && ReadFileTime(0,strSelName,LastWriteTime,AttrDlg[SA_EDIT_WDATE].strData,AttrDlg[SA_EDIT_WTIME].strData);
						int SetCreationTime=  DlgParam.OCreationTime   && ReadFileTime(1,strSelName,CreationTime,AttrDlg[SA_EDIT_CDATE].strData,AttrDlg[SA_EDIT_CTIME].strData);
						int SetLastAccessTime=DlgParam.OLastAccessTime && ReadFileTime(2,strSelName,LastAccessTime,AttrDlg[SA_EDIT_ADATE].strData,AttrDlg[SA_EDIT_ATIME].strData);
						int SetChangeTime=    DlgParam.OChangeTime     && ReadFileTime(3,strSelName,ChangeTime,AttrDlg[SA_EDIT_XDATE].strData,AttrDlg[SA_EDIT_XTIME].strData);
						RetCode=ESetFileTime(strSelName,SetWriteTime?&LastWriteTime:nullptr,SetCreationTime?&CreationTime:nullptr,SetLastAccessTime?&LastAccessTime:nullptr,SetChangeTime?&ChangeTime:nullptr,FileAttr,SkipMode);

						if (RetCode == SETATTR_RET_ERROR)
							break;
						else if (RetCode == SETATTR_RET_SKIP)
							continue;
						else if (RetCode == SETATTR_RET_SKIPALL)
						{
							SkipMode=SETATTR_RET_SKIP;
							continue;
						}

						if(FileAttr!=INVALID_FILE_ATTRIBUTES)
						{
							if (((FileAttr|SetAttr)&~ClearAttr) != FileAttr)
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
										RetCode=ESetFileEncryption(strSelName, AttrDlg[SA_CHECKBOX_ENCRYPTED].Selected != 0, FileAttr, SkipMode);

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
								DWORD LastTime2=GetTickCount();
								string strFullName;

								while (ScTree.GetNextName(&FindData,strFullName))
								{
									CurTime=GetTickCount();

									if (CurTime-LastTime2>(DWORD)Opt.RedrawTimeout)
									{
										LastTime2=CurTime;
										ShellSetFileAttributesMsg(strFullName);

										if (CheckForEsc())
										{
											Cancel=true;
											break;
										}
									}

									if(!AttrDlg[SA_EDIT_OWNER].strData.IsEmpty() && (DlgParam.OSubfoldersState || StrCmpI(strInitOwner,AttrDlg[SA_EDIT_OWNER].strData)))
									{
										int Result=ESetFileOwner(strFullName,AttrDlg[SA_EDIT_OWNER].strData,SkipMode);
										if(Result==SETATTR_RET_SKIPALL)
										{
											SkipMode=SETATTR_RET_SKIP;
										}
										else if(Result==SETATTR_RET_ERROR)
										{
											break;
										}
									}

									SetWriteTime=     DlgParam.OLastWriteTime  && ReadFileTime(0,strFullName,LastWriteTime,AttrDlg[SA_EDIT_WDATE].strData,AttrDlg[SA_EDIT_WTIME].strData);
									SetCreationTime=  DlgParam.OCreationTime   && ReadFileTime(1,strFullName,CreationTime,AttrDlg[SA_EDIT_CDATE].strData,AttrDlg[SA_EDIT_CTIME].strData);
									SetLastAccessTime=DlgParam.OLastAccessTime && ReadFileTime(2,strFullName,LastAccessTime,AttrDlg[SA_EDIT_ADATE].strData,AttrDlg[SA_EDIT_ATIME].strData);
									SetChangeTime=    DlgParam.OChangeTime     && ReadFileTime(3,strFullName,ChangeTime,AttrDlg[SA_EDIT_XDATE].strData,AttrDlg[SA_EDIT_XTIME].strData);

									if (SetWriteTime || SetCreationTime || SetLastAccessTime || SetChangeTime)
									{
										RetCode=ESetFileTime(strFullName,SetWriteTime?&LastWriteTime:nullptr,SetCreationTime?&CreationTime:nullptr,SetLastAccessTime?&LastAccessTime:nullptr,SetChangeTime?&ChangeTime:nullptr,FindData.dwFileAttributes,SkipMode);

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
												RetCode=ESetFileEncryption(strFullName, AttrDlg[SA_CHECKBOX_ENCRYPTED].Selected!=0, FindData.dwFileAttributes, SkipMode);

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
						}
					} // END: while (SrcPanel->GetSelName(...))
				}
			}
			break;
		case SA_BUTTON_SYSTEMDLG:
			{
				SHELLEXECUTEINFOW seInfo={sizeof(seInfo)};
				seInfo.nShow = SW_SHOW;
				seInfo.fMask = SEE_MASK_INVOKEIDLIST;
				// "\\?\c:\" fails on old windows
				bool Root;
				string strFullName((ParsePath(strSelName, nullptr, &Root) == PATH_DRIVELETTER && Root)?strSelName:NTPath(strSelName));
				if(FindData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
				{
					AddEndSlash(strFullName);
				}
				seInfo.lpFile = strFullName;
				seInfo.lpVerb = L"properties";
				string strCurDir;
				apiGetCurrentDirectory(strCurDir);
				seInfo.lpDirectory=strCurDir;
				ShellExecuteExW(&seInfo);
			}
			break;
		default:
			return false;
		}
	}

	if(SrcPanel)
	{
		SrcPanel->SaveSelection();
		SrcPanel->Update(UPDATE_KEEP_SELECTION);
		SrcPanel->ClearSelection();
		CtrlObject->Cp()->GetAnotherPanel(SrcPanel)->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
	}
	CtrlObject->Cp()->Redraw();
	return true;
}
