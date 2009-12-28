/*
history.cpp

История (Alt-F8, Alt-F11, Alt-F12)
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

#include "history.hpp"
#include "language.hpp"
#include "keys.hpp"
#include "vmenu.hpp"
#include "lang.hpp"
#include "registry.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "config.hpp"
#include "strmix.hpp"
#include "dialog.hpp"
#include "interf.hpp"

History::History(enumHISTORYTYPE TypeHistory, size_t HistoryCount, const wchar_t *RegKey, const int *EnableSave, bool SaveType)
{
	strRegKey = RegKey;
	History::SaveType=SaveType;
	History::EnableSave=EnableSave;
	History::TypeHistory=TypeHistory;
	History::HistoryCount=HistoryCount;
	EnableAdd=true;
	RemoveDups=1;
	KeepSelectedPos=false;
	CurrentItem=NULL;
}

History::~History()
{
}

/*
   SaveForbid - принудительно запретить запись добавляемой строки.
                Используется на панели плагина
*/
void History::AddToHistory(const wchar_t *Str, int Type, const wchar_t *Prefix, bool SaveForbid)
{
	if (!EnableAdd)
		return;

	AddToHistoryLocal(Str,Prefix,Type);

	if (*EnableSave && !SaveForbid)
		SaveHistory();
}


void History::AddToHistoryLocal(const wchar_t *Str, const wchar_t *Prefix, int Type)
{
	if (!Str || !*Str)
		return;

	HistoryRecord AddRecord;

	if (TypeHistory == HISTORYTYPE_FOLDER && Prefix && *Prefix)
	{
		AddRecord.strName = Prefix;
		AddRecord.strName += L":";
	}

	AddRecord.strName += Str;
	RemoveTrailingSpaces(AddRecord.strName);
	AddRecord.Type=Type;

	if (RemoveDups) // удалять дубликаты?
	{
		for (HistoryRecord *HistoryItem=HistoryList.First(); HistoryItem != NULL; HistoryItem=HistoryList.Next(HistoryItem))
		{
			if (EqualType(AddRecord.Type,HistoryItem->Type))
			{
				if ((RemoveDups==1 && StrCmp(AddRecord.strName,HistoryItem->strName)==0) ||
				        (RemoveDups==2 && StrCmpI(AddRecord.strName,HistoryItem->strName)==0))
				{
					AddRecord.Lock=HistoryItem->Lock;
					HistoryItem=HistoryList.Delete(HistoryItem);
					break;
				}
			}
		}
	}

	if (HistoryList.Count()==HistoryCount)
	{
		HistoryList.Delete(HistoryList.First());
	}

	GetSystemTimeAsFileTime(&AddRecord.Timestamp); // in UTC
	HistoryList.Push(&AddRecord);
	ResetPosition();
}

bool History::SaveHistory()
{
	if (!*EnableSave)
		return true;

	if (!HistoryList.Count())
	{
		DeleteRegKey(strRegKey);
		return true;
	}

	wchar_t *TypesBuffer=NULL;

	if (SaveType)
	{
		TypesBuffer=(wchar_t *)xf_malloc((HistoryList.Count()+1)*sizeof(wchar_t));

		if (!TypesBuffer)
			return false;
	}

	wchar_t *LocksBuffer=NULL;

	if (!(LocksBuffer=(wchar_t *)xf_malloc((HistoryList.Count()+1)*sizeof(wchar_t))))
	{
		if (TypesBuffer)
			xf_free(TypesBuffer);

		return false;
	}

	FILETIME *TimesBuffer=NULL;

	if (!(TimesBuffer=(FILETIME *)xf_malloc((HistoryList.Count()+1)*sizeof(FILETIME))))
	{
		if (LocksBuffer)
			xf_free(LocksBuffer);

		if (TypesBuffer)
			xf_free(TypesBuffer);

		return false;
	}

	memset(TimesBuffer,0,(HistoryList.Count()+1)*sizeof(FILETIME));
	wmemset(LocksBuffer,0,HistoryList.Count()+1);

	if (SaveType)
		wmemset(TypesBuffer,0,HistoryList.Count()+1);

	bool ret = false;
	HKEY hKey = NULL;
	wchar_t *BufferLines=NULL, *PtrBuffer;
	size_t SizeLines=0, SizeTypes=0, SizeLocks=0, SizeTimes=0;
	int Position = -1;
	size_t i=HistoryList.Count()-1;

	for (const HistoryRecord *HistoryItem=HistoryList.Last(); HistoryItem != NULL; HistoryItem=HistoryList.Prev(HistoryItem))
	{
		if ((PtrBuffer=(wchar_t*)xf_realloc(BufferLines,(SizeLines+HistoryItem->strName.GetLength()+2)*sizeof(wchar_t))) == NULL)
		{
			ret = false;
			goto end;
		}

		BufferLines=PtrBuffer;
		xwcsncpy(BufferLines+SizeLines,HistoryItem->strName,HistoryItem->strName.GetLength());
		SizeLines+=HistoryItem->strName.GetLength()+1;

		if (SaveType)
			TypesBuffer[SizeTypes++]=HistoryItem->Type+L'0';

		LocksBuffer[SizeLocks++]=HistoryItem->Lock+L'0';
		TimesBuffer[SizeTimes].dwLowDateTime=HistoryItem->Timestamp.dwLowDateTime;
		TimesBuffer[SizeTimes].dwHighDateTime=HistoryItem->Timestamp.dwHighDateTime;
		SizeTimes++;

		if (HistoryItem == CurrentItem)
			Position = static_cast<int>(i);

		i--;
	}

	hKey=CreateRegKey(strRegKey);

	if (hKey!=NULL)
	{
		RegSetValueEx(hKey,L"Lines",0,REG_MULTI_SZ,(unsigned char *)BufferLines,static_cast<DWORD>(SizeLines*sizeof(wchar_t)));

		if (SaveType)
			RegSetValueEx(hKey,L"Types",0,REG_SZ,(unsigned char *)TypesBuffer,static_cast<DWORD>((SizeTypes+1)*sizeof(wchar_t)));

		RegSetValueEx(hKey,L"Locks",0,REG_SZ,(unsigned char *)LocksBuffer,static_cast<DWORD>((SizeLocks+1)*sizeof(wchar_t)));
		RegSetValueEx(hKey,L"Times",0,REG_BINARY,(unsigned char *)TimesBuffer,(DWORD)SizeTimes*sizeof(FILETIME));
		RegSetValueEx(hKey,L"Position",0,REG_DWORD,(BYTE *)&Position,sizeof(Position));
		RegCloseKey(hKey);
		ret = true;
	}

end:

	if (BufferLines)
		xf_free(BufferLines);

	if (TypesBuffer)
		xf_free(TypesBuffer);

	if (LocksBuffer)
		xf_free(LocksBuffer);

	if (TimesBuffer)
		xf_free(TimesBuffer);

	return ret;
}

bool History::ReadLastItem(const wchar_t *RegKey, string &strStr)
{
	strStr.Clear();
	HKEY hKey=OpenRegKey(RegKey);

	if (!hKey)
		return false;

	DWORD Type;
	DWORD Size=0;

	if (RegQueryValueEx(hKey,L"Lines",0,&Type,NULL,&Size)!=ERROR_SUCCESS || Size<sizeof(wchar_t)) // Нету ничерта
		return false;

	wchar_t *Buffer=(wchar_t*)xf_malloc(Size);

	if (!Buffer)
		return false;

	if (RegQueryValueEx(hKey,L"Lines",0,&Type,(unsigned char *)Buffer,&Size)!=ERROR_SUCCESS)
	{
		xf_free(Buffer);
		return false;
	}

	Buffer[Size/sizeof(wchar_t)-1]=0; //safety
	strStr = Buffer; //last item is first in registry, null terminated
	xf_free(Buffer);
	return true;
}

bool History::ReadHistory(bool bOnlyLines)
{
	HKEY hKey=OpenRegKey(strRegKey);

	if (!hKey)
		return false;

	bool ret = false;
	wchar_t *TypesBuffer=NULL;
	wchar_t *LocksBuffer=NULL;
	FILETIME *TimesBuffer=NULL;
	wchar_t *Buffer=NULL;
	int Position=-1;
	DWORD Size=sizeof(Position);
	DWORD Type;

	if (!bOnlyLines)
		RegQueryValueEx(hKey,L"Position",0,&Type,(BYTE *)&Position,&Size);

	bool NeedReadType=false;
	bool NeedReadLock=false;
	bool NeedReadTime=false;
	Size=0;

	if (!bOnlyLines && SaveType && RegQueryValueEx(hKey,L"Types",0,&Type,NULL,&Size)==ERROR_SUCCESS && Size>0)
	{
		NeedReadType=true;
		Size=Max(Size,(DWORD)((HistoryCount+2)*sizeof(wchar_t)));
		TypesBuffer=(wchar_t *)xf_malloc(Size);

		if (TypesBuffer)
		{
			memset(TypesBuffer,0,Size);

			if (RegQueryValueEx(hKey,L"Types",0,&Type,(BYTE *)TypesBuffer,&Size)!=ERROR_SUCCESS)
				goto end;
		}
		else
			goto end;
	}

	Size=0;

	if (!bOnlyLines && RegQueryValueEx(hKey,L"Locks",0,&Type,NULL,&Size)==ERROR_SUCCESS && Size>0)
	{
		NeedReadLock=true;
		Size=Max(Size,(DWORD)((HistoryCount+2)*sizeof(wchar_t)));
		LocksBuffer=(wchar_t *)xf_malloc(Size);

		if (LocksBuffer)
		{
			memset(LocksBuffer,0,Size);

			if (RegQueryValueEx(hKey,L"Locks",0,&Type,(BYTE *)LocksBuffer,&Size)!=ERROR_SUCCESS)
				goto end;
		}
		else
			goto end;
	}

	Size=0;

	if (!bOnlyLines && RegQueryValueEx(hKey,L"Times",0,&Type,NULL,&Size)==ERROR_SUCCESS && Size>0)
	{
		NeedReadTime=true;
		Size=Max(Size,(DWORD)((HistoryCount+2)*sizeof(FILETIME)));
		TimesBuffer=(FILETIME *)xf_malloc(Size);

		if (TimesBuffer)
		{
			memset(TimesBuffer,0,Size);

			if (RegQueryValueEx(hKey,L"Times",0,&Type,(BYTE *)TimesBuffer,&Size)!=ERROR_SUCCESS)
				goto end;
		}
		else
			goto end;
	}

	Size=0;

	if (RegQueryValueEx(hKey,L"Lines",0,&Type,NULL,&Size)!=ERROR_SUCCESS || !Size) // Нету ничерта
	{
		ret = true;
		goto end;
	}

	if ((Buffer=(wchar_t*)xf_malloc(Size)) == NULL)
		goto end;

	if (RegQueryValueEx(hKey,L"Lines",0,&Type,(unsigned char *)Buffer,&Size)==ERROR_SUCCESS)
	{
		CurrentItem=NULL;
		wchar_t *TypesBuf=TypesBuffer;
		wchar_t *LockBuf=LocksBuffer;
		FILETIME *TimeBuf=TimesBuffer;
		size_t StrPos=0;
		wchar_t *Buf=Buffer;
		Size/=sizeof(wchar_t);
		Buf[Size-1]=0; //safety

		while (Size > 1 && StrPos < HistoryCount)
		{
			int Length=StrLength(Buf)+1;
			HistoryRecord AddRecord;
			AddRecord.strName = Buf;
			Buf+=Length;
			Size-=Length;

			if (NeedReadType)
			{
				if (iswdigit(*TypesBuf))
				{
					AddRecord.Type = *TypesBuf-L'0';
					TypesBuf++;
				}
			}

			if (NeedReadLock)
			{
				if (iswdigit(*LockBuf))
				{
					AddRecord.Lock = (*LockBuf-L'0') == 0?false:true;
					LockBuf++;
				}
			}

			if (NeedReadTime)
			{
				AddRecord.Timestamp.dwLowDateTime=TimeBuf->dwLowDateTime;
				AddRecord.Timestamp.dwHighDateTime=TimeBuf->dwHighDateTime;
				TimeBuf++;
			}

			if (AddRecord.strName.GetLength())
			{
				HistoryList.Unshift(&AddRecord);

				if (StrPos == static_cast<size_t>(Position))
					CurrentItem=HistoryList.First();
			}

			StrPos++;
		}
	}
	else
		goto end;

	ret=true;
end:
	RegCloseKey(hKey);

	if (TypesBuffer)
		xf_free(TypesBuffer);

	if (Buffer)
		xf_free(Buffer);

	if (LocksBuffer)
		xf_free(LocksBuffer);

	if (TimesBuffer)
		xf_free(TimesBuffer);

	//if (!ret)
	//clear();
	return ret;
}

const wchar_t *History::GetTitle(int Type)
{
	switch (Type)
	{
		case 0: // вьювер
			return MSG(MHistoryView);
		case 1: // обычное открытие в редакторе
		case 4: // открытие с локом
			return MSG(MHistoryEdit);
		case 2: // external - без ожидания
		case 3: // external - AlwaysWaitFinish
			return MSG(MHistoryExt);
	}

	return L"";
}

int History::Select(const wchar_t *Title, const wchar_t *HelpTopic, string &strStr, int &Type)
{
	int Height=ScrY-8;
	VMenu HistoryMenu(Title,NULL,0,Height);
	HistoryMenu.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);

	if (HelpTopic!=NULL)
		HistoryMenu.SetHelp(HelpTopic);

	HistoryMenu.SetPosition(-1,-1,0,0);
	HistoryMenu.AssignHighlights(TRUE);
	return ProcessMenu(strStr, Title, HistoryMenu, Height, Type, NULL);
}

int History::Select(VMenu &HistoryMenu, int Height, Dialog *Dlg, string &strStr)
{
	int Type=0;
	return ProcessMenu(strStr, NULL, HistoryMenu, Height, Type, Dlg);
}

/*
 Return:
  -1 - Error???
   0 - Esc
   1 - Enter
   2 - Shift-Enter
   3 - Ctrl-Enter
   4 - F3
   5 - F4
   6 - Ctrl-Shift-Enter
*/
int History::ProcessMenu(string &strStr, const wchar_t *Title, VMenu &HistoryMenu, int Height, int &Type, Dialog *Dlg)
{
	MenuItemEx MenuItem;
	HistoryRecord *SelectedRecord=NULL;
	FarListPos Pos={0,0};
	int Code=-1;
	int RetCode=1;
	bool Done=false;
	bool SetUpMenuPos=false;

	if (TypeHistory == HISTORYTYPE_DIALOG && HistoryList.Empty())
		return 0;

	while (!Done)
	{
		bool IsUpdate=false;
		HistoryMenu.DeleteItems();
		HistoryMenu.Modal::ClearDone();

		// заполнение пунктов меню
		for (const HistoryRecord *HistoryItem=TypeHistory==HISTORYTYPE_DIALOG?HistoryList.Last():HistoryList.First(); HistoryItem != NULL; HistoryItem=TypeHistory==HISTORYTYPE_DIALOG?HistoryList.Prev(HistoryItem):HistoryList.Next(HistoryItem))
		{
			string strRecord = HistoryItem->strName;
			strRecord.Clear();

			if (TypeHistory == HISTORYTYPE_VIEW)
			{
				strRecord += GetTitle(HistoryItem->Type);
				strRecord += L":";
				strRecord += (HistoryItem->Type==4?L"-":L" ");
			}

			/*
				TODO: возможно здесь! или выше....
				char Date[16],Time[16], OutStr[32];
				ConvertDate(HistoryItem->Timestamp,Date,Time,5,TRUE,FALSE,TRUE,TRUE);
				а дальше
				strRecord += дату и время
			*/
			strRecord += HistoryItem->strName;;

			if (TypeHistory != HISTORYTYPE_DIALOG)
				ReplaceStrings(strRecord, L"&",L"&&", -1);

			MenuItem.Clear();
			MenuItem.strName = strRecord;
			MenuItem.SetCheck(HistoryItem->Lock?1:0);

			if (!SetUpMenuPos)
				MenuItem.SetSelect(CurrentItem==HistoryItem || (!CurrentItem && HistoryItem==HistoryList.Last()));

			HistoryMenu.SetUserData(HistoryItem,sizeof(HistoryItem),HistoryMenu.AddItem(&MenuItem));
		}

		//MenuItem.Clear ();
		//MenuItem.strName = L"                    ";
		//if (!SetUpMenuPos)
		//MenuItem.SetSelect(CurLastPtr==-1 || CurLastPtr>=HistoryList.Length);
		//HistoryMenu.SetUserData(NULL,sizeof(OneItem *),HistoryMenu.AddItem(&MenuItem));

		if (TypeHistory == HISTORYTYPE_DIALOG)
			Dlg->SetComboBoxPos();
		else
			HistoryMenu.SetPosition(-1,-1,0,0);

		if (SetUpMenuPos)
		{
			Pos.SelectPos=Pos.SelectPos < (int)HistoryList.Count() ? Pos.SelectPos : (int)HistoryList.Count()-1;
			Pos.TopPos=Min(Pos.TopPos,HistoryMenu.GetItemCount()-Height);
			HistoryMenu.SetSelectPos(&Pos);
			SetUpMenuPos=false;
		}

		/*BUGBUG???
			if (TypeHistory == HISTORYTYPE_DIALOG)
			{
					//  Перед отрисовкой спросим об изменении цветовых атрибутов
					BYTE RealColors[VMENU_COLOR_COUNT];
					FarListColors ListColors={0};
					ListColors.ColorCount=VMENU_COLOR_COUNT;
					ListColors.Colors=RealColors;
					HistoryMenu.GetColors(&ListColors);
					if(DlgProc((HANDLE)this,DN_CTLCOLORDLGLIST,CurItem->ID,(LONG_PTR)&ListColors))
						HistoryMenu.SetColors(&ListColors);
				}
		*/
		HistoryMenu.Show();

		while (!HistoryMenu.Done())
		{
			if (TypeHistory == HISTORYTYPE_DIALOG && (!Dlg->GetDropDownOpened() || HistoryList.Empty()))
			{
				HistoryMenu.ProcessKey(KEY_ESC);
				continue;
			}

			int Key=HistoryMenu.ReadInput();

			if (TypeHistory == HISTORYTYPE_DIALOG && Key==KEY_TAB) // Tab в списке хистори диалогов - аналог Enter
			{
				HistoryMenu.ProcessKey(KEY_ENTER);
				continue;
			}

			HistoryMenu.GetSelectPos(&Pos);
			HistoryRecord *CurrentRecord=(HistoryRecord *)HistoryMenu.GetUserData(NULL,sizeof(HistoryRecord *),Pos.SelectPos);

			switch (Key)
			{
				case KEY_CTRLR: // обновить с удалением недоступных
				{
					if (TypeHistory == HISTORYTYPE_FOLDER || TypeHistory == HISTORYTYPE_VIEW)
					{
						bool ModifiedHistory=false;

						for (HistoryRecord *HistoryItem=HistoryList.First(); HistoryItem != NULL; HistoryItem=HistoryList.Next(HistoryItem))
						{
							if (HistoryItem->Lock) // залоченные не трогаем
								continue;

							// убить запись из истории
							if (apiGetFileAttributes(HistoryItem->strName) == INVALID_FILE_ATTRIBUTES)
							{
								HistoryItem=HistoryList.Delete(HistoryItem);
								ModifiedHistory=true;
							}
						}

						if (ModifiedHistory) // избавляемся от лишних телодвижений
						{
							SaveHistory(); // сохранить
							HistoryMenu.Modal::SetExitCode(Pos.SelectPos);
							HistoryMenu.SetUpdateRequired(TRUE);
							IsUpdate=true;
						}

						ResetPosition();
					}

					break;
				}
				case KEY_CTRLSHIFTNUMENTER:
				case KEY_CTRLNUMENTER:
				case KEY_SHIFTNUMENTER:
				case KEY_CTRLSHIFTENTER:
				case KEY_CTRLENTER:
				case KEY_SHIFTENTER:
				{
					if (TypeHistory == HISTORYTYPE_DIALOG)
						break;

					HistoryMenu.Modal::SetExitCode(Pos.SelectPos);
					Done=true;
					RetCode=Key==KEY_CTRLSHIFTENTER||Key==KEY_CTRLSHIFTNUMENTER?6:(Key==KEY_SHIFTENTER||Key==KEY_SHIFTNUMENTER?2:3);
					break;
				}
				case KEY_F3:
				case KEY_F4:
				case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:
				{
					if (TypeHistory == HISTORYTYPE_DIALOG)
						break;

					HistoryMenu.Modal::SetExitCode(Pos.SelectPos);
					Done=true;
					RetCode=(Key==KEY_F4? 5 : 4);
					break;
				}
				// $ 09.04.2001 SVS - Фича - копирование из истории строки в Clipboard
				case KEY_CTRLC:
				case KEY_CTRLINS:  case KEY_CTRLNUMPAD0:
				{
					if (CurrentRecord)
						CopyToClipboard(CurrentRecord->strName);

					break;
				}
				// Lock/Unlock
				case KEY_INS:
				case KEY_NUMPAD0:
				{
					if (HistoryMenu.GetItemCount()/* > 1*/)
					{
						CurrentItem=CurrentRecord;
						CurrentItem->Lock=CurrentItem->Lock?false:true;
						HistoryMenu.Hide();
						ResetPosition();
						SaveHistory();
						HistoryMenu.Modal::SetExitCode(Pos.SelectPos);
						HistoryMenu.SetUpdateRequired(TRUE);
						IsUpdate=true;
						SetUpMenuPos=true;
					}

					break;
				}
				case KEY_SHIFTNUMDEL:
				case KEY_SHIFTDEL:
				{
					if (HistoryMenu.GetItemCount()/* > 1*/)
					{
						if (!CurrentRecord->Lock)
						{
							HistoryMenu.Hide();
							HistoryList.Delete(CurrentRecord);
							ResetPosition();
							SaveHistory();
							HistoryMenu.Modal::SetExitCode(Pos.SelectPos);
							HistoryMenu.SetUpdateRequired(TRUE);
							IsUpdate=true;
							SetUpMenuPos=true;
						}
					}

					break;
				}
				case KEY_NUMDEL:
				case KEY_DEL:
				{
					if (HistoryMenu.GetItemCount()/* > 1*/ &&
					        (!Opt.Confirm.HistoryClear ||
					         (Opt.Confirm.HistoryClear &&
					          Message(MSG_WARNING,2,
					                  MSG((TypeHistory==HISTORYTYPE_CMD || TypeHistory==HISTORYTYPE_DIALOG?MHistoryTitle:
					                       (TypeHistory==HISTORYTYPE_FOLDER?MFolderHistoryTitle:MViewHistoryTitle))),
					                  MSG(MHistoryClear),
					                  MSG(MClear),MSG(MCancel))==0)))
					{
						for (HistoryRecord *HistoryItem=HistoryList.First(); HistoryItem != NULL; HistoryItem=HistoryList.Next(HistoryItem))
						{
							if (HistoryItem->Lock) // залоченные не трогаем
								continue;

							HistoryItem=HistoryList.Delete(HistoryItem);
						}

						ResetPosition();
						HistoryMenu.Hide();
						SaveHistory();
						HistoryMenu.Modal::SetExitCode(Pos.SelectPos);
						HistoryMenu.SetUpdateRequired(TRUE);
						IsUpdate=true;
					}

					break;
				}
				default:
					HistoryMenu.ProcessInput();
					break;
			}
		}

		if (IsUpdate)
			continue;

		Done=true;
		Code=HistoryMenu.Modal::GetExitCode();

		if (Code >= 0)
		{
			SelectedRecord=(HistoryRecord *)HistoryMenu.GetUserData(NULL,sizeof(HistoryRecord *),Code);

			if (!SelectedRecord)
				return -1;

			if (RetCode != 3 && ((TypeHistory == HISTORYTYPE_FOLDER && !SelectedRecord->Type) || TypeHistory == HISTORYTYPE_VIEW) && apiGetFileAttributes(SelectedRecord->strName) == INVALID_FILE_ATTRIBUTES)
			{
				SetLastError(ERROR_FILE_NOT_FOUND);

				if (SelectedRecord->Type == 1 && TypeHistory == HISTORYTYPE_VIEW) // Edit? тогда спросим и если надо создадим
				{
					if (Message(MSG_WARNING|MSG_ERRORTYPE,2,Title,SelectedRecord->strName,MSG(MViewHistoryIsCreate),MSG(MHYes),MSG(MHNo)) == 0)
						break;
				}
				else
				{
					Message(MSG_WARNING|MSG_ERRORTYPE,1,Title,SelectedRecord->strName,MSG(MOk));
				}

				Done=false;
				SetUpMenuPos=true;
				HistoryMenu.Modal::SetExitCode(Pos.SelectPos=Code);
				continue;
			}
		}
	}

	if (Code < 0 || !SelectedRecord)
		return 0;

	if (KeepSelectedPos)
	{
		CurrentItem = SelectedRecord;
	}

	strStr = SelectedRecord->strName;

	if (RetCode < 4 || RetCode == 6)
	{
		Type=SelectedRecord->Type;
	}
	else
	{
		Type=RetCode-4;

		if (Type == 1 && SelectedRecord->Type == 4)
			Type=4;

		RetCode=1;
	}

	return RetCode;
}

void History::GetPrev(string &strStr)
{
	CurrentItem=HistoryList.Prev(CurrentItem);

	if (!CurrentItem)
		CurrentItem=HistoryList.First();

	if (CurrentItem)
		strStr = CurrentItem->strName;
	else
		strStr.Clear();
}


void History::GetNext(string &strStr)
{
	if (CurrentItem)
		CurrentItem=HistoryList.Next(CurrentItem);

	if (CurrentItem)
		strStr = CurrentItem->strName;
	else
		strStr.Clear();
}


bool History::GetSimilar(string &strStr, int LastCmdPartLength, bool bAppend)
{
	int Length=(int)strStr.GetLength();

	if (LastCmdPartLength!=-1 && LastCmdPartLength<Length)
		Length=LastCmdPartLength;

	if (LastCmdPartLength==-1)
	{
		ResetPosition();
	}

	for (HistoryRecord *HistoryItem=HistoryList.Prev(CurrentItem); HistoryItem != CurrentItem; HistoryItem=HistoryList.Prev(HistoryItem))
	{
		if (!HistoryItem)
			continue;

		if (StrCmpNI(strStr,HistoryItem->strName,Length)==0 && StrCmp(strStr,HistoryItem->strName)!=0)
		{
			if (bAppend)
				strStr += &HistoryItem->strName[Length];
			else
				strStr = HistoryItem->strName;

			CurrentItem = HistoryItem;
			return true;
		}
	}

	return false;
}

bool History::GetAllSimilar(VMenu &HistoryMenu,const wchar_t *Str)
{
	ResetPosition();
	int Length=StrLength(Str);
	for (HistoryRecord *HistoryItem=HistoryList.First();HistoryItem;HistoryItem=HistoryList.Next(HistoryItem))
	{
		if (!StrCmpNI(Str,HistoryItem->strName,Length) && StrCmp(Str,HistoryItem->strName))
		{
			HistoryMenu.AddItem(HistoryItem->strName);
		}
	}
	return false;
}

void History::SetAddMode(bool EnableAdd, int RemoveDups, bool KeepSelectedPos)
{
	History::EnableAdd=EnableAdd;
	History::RemoveDups=RemoveDups;
	History::KeepSelectedPos=KeepSelectedPos;
}

bool History::EqualType(int Type1, int Type2)
{
	return Type1 == Type2 || (TypeHistory == HISTORYTYPE_VIEW && ((Type1 == 4 && Type2 == 1) || (Type1 == 1 && Type2 == 4)))?true:false;
}
