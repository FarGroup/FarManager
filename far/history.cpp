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
#include "fn.hpp"
#include "language.hpp"
#include "keys.hpp"
#include "vmenu.hpp"
#include "lang.hpp"
#include "registry.hpp"

History::History(int TypeHistory, int HistoryCount, const wchar_t *RegKey, const int *EnableSave, bool SaveType)
{
	EmptyHistory();

	strRegKey = RegKey;

	History::SaveType=SaveType;
	History::EnableSave=EnableSave;
	History::TypeHistory=TypeHistory;
	History::HistoryCount=HistoryCount;
	EnableAdd=true;
	RemoveDups=1;
	KeepSelectedPos=false;
}

History::~History()
{
	EmptyHistory();
}

void History::EmptyHistory()
{
	HistoryList.clear();
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
		for (const HistoryRecord *HistoryItem=HistoryList.toBegin(); HistoryItem != NULL; HistoryItem=HistoryList.toNext())
		{
			if (EqualType(AddRecord.Type,HistoryItem->Type))
			{
				if ((RemoveDups==1 && StrCmp(AddRecord.strName,HistoryItem->strName)==0) ||
						(RemoveDups==2 && StrCmpI(AddRecord.strName,HistoryItem->strName)==0))
				{
					HistoryList.erase();
					break;
				}
			}
		}
	}

	if (HistoryList.size()==(DWORD)HistoryCount)
	{
		HistoryList.toBegin();
		HistoryList.erase();
	}

	HistoryList.push_back(AddRecord);

	HistoryList.toEnd();
	HistoryList.toNext();
}

bool History::SaveHistory()
{
	if (!*EnableSave)
		return true;

	if (!HistoryList.size())
	{
		DeleteRegKey(strRegKey);
		return true;
	}

	wchar_t *TypesBuffer=NULL;
	if (SaveType)
	{
		TypesBuffer=(wchar_t *)xf_malloc((HistoryList.size()+1)*sizeof(wchar_t));
		if (!TypesBuffer)
			return false;
		wmemset(TypesBuffer,0,HistoryList.size()+1);
	}

	bool ret = false;
	HKEY hKey = NULL;

	wchar_t *BufferLines=NULL, *PtrBuffer;
	size_t SizeLines=0, SizeTypes=0;

	HistoryList.storePosition();

	const HistoryRecord *SelectedItem = HistoryList.getItem();
	int Position = -1, i=HistoryList.size()-1;

	for (const HistoryRecord *HistoryItem=HistoryList.toEnd(); HistoryItem != NULL; HistoryItem=HistoryList.toPrev())
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

		if (HistoryItem == SelectedItem)
			Position = i;

		i--;
	}

	hKey=CreateRegKey(strRegKey);
	if (hKey!=NULL)
	{
		RegSetValueExW(hKey,L"Lines",0,REG_BINARY,(unsigned char *)BufferLines,static_cast<DWORD>(SizeLines*sizeof(wchar_t)));

		if (SaveType)
			RegSetValueExW(hKey,L"Types",0,REG_SZ,(unsigned char *)TypesBuffer,static_cast<DWORD>((SizeTypes+1)*sizeof(wchar_t)));

		RegSetValueExW(hKey,L"Position",0,REG_DWORD,(BYTE *)&Position,sizeof(Position));

		RegCloseKey(hKey);

		ret = true;
	}

end:

	HistoryList.restorePosition();

	if (BufferLines)
		xf_free(BufferLines);
	if (TypesBuffer)
		xf_free(TypesBuffer);

	return ret;
}


bool History::ReadHistory()
{
	bool NeedReadType = SaveType && CheckRegValue(strRegKey, L"Types");

	DWORD Type;
	HKEY hKey=OpenRegKey(strRegKey);
	if (!hKey)
		return false;

	bool ret = false;

	wchar_t *TypesBuffer=NULL;
	wchar_t *Buffer=NULL;
	DWORD Size;

	int Position=-1;
	Size=sizeof(Position);
	RegQueryValueExW(hKey,L"Position",0,&Type,(BYTE *)&Position,&Size);

	if (NeedReadType)
	{
		Size=GetRegKeySize(hKey, L"Types");
		Size=Max(Size,(DWORD)((HistoryCount+2)*sizeof(wchar_t)));
		TypesBuffer=(wchar_t *)xf_malloc(Size);
		if (TypesBuffer)
		{
			memset(TypesBuffer,0,Size);
			if (RegQueryValueExW(hKey,L"Types",0,&Type,(BYTE *)TypesBuffer,&Size)!=ERROR_SUCCESS)
				goto end;
		}
		else
			goto end;
	}

	Size=GetRegKeySize(hKey, L"Lines");
	if (!Size) // Нету ничерта
	{
		ret = true;
		goto end;
	}

	if ((Buffer=(wchar_t*)xf_malloc(Size)) == NULL)
		goto end;

	if (RegQueryValueExW(hKey,L"Lines",0,&Type,(unsigned char *)Buffer,&Size)==ERROR_SUCCESS)
	{
		bool bPosFound = false;
		wchar_t *TypesBuf=TypesBuffer;
		int StrPos=0;
		wchar_t *Buf=Buffer;
		Size/=sizeof(wchar_t);
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

			if (AddRecord.strName.GetLength())
			{
				HistoryList.push_front(AddRecord);
				if (StrPos == Position)
				{
					HistoryList.storePosition();
					bPosFound = true;
				}
			}

			StrPos++;
		}

		if (bPosFound)
		{
			HistoryList.restorePosition();
		}
		else
		{
			HistoryList.toEnd();
			HistoryList.toNext();
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

	//if (!ret)
		//EmptyHistory();

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

/*
 Return:
   0 - Esc
   1 - Enter
   2 - Shift-Enter
   3 - Ctrl-Enter
   4 - F3
   5 - F4
   6 - Ctrl-Shift-Enter
*/
int History::Select(const wchar_t *Title,const wchar_t *HelpTopic, string &strStr,int &Type)
{
	MenuItemEx MenuItem;

	const HistoryRecord *SelectedRecord=NULL;
	int Code=-1,Height=ScrY-8,StrPos=0;
	int RetCode=1;

	{
		VMenu HistoryMenu(Title,NULL,0,Height);
		HistoryMenu.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);
		if (HelpTopic!=NULL)
			HistoryMenu.SetHelp(HelpTopic);
		HistoryMenu.SetPosition(-1,-1,0,0);
		HistoryMenu.AssignHighlights(TRUE);
		bool Done=false;
		bool SetUpMenuPos=false;

		while (!Done)
		{
			bool IsUpdate=false;

			HistoryMenu.DeleteItems();
			HistoryMenu.Modal::ClearDone();
			HistoryMenu.SetPosition(-1,-1,0,0);

			const HistoryRecord *HistoryCurrentItem = HistoryList.getItem();
			HistoryList.storePosition();
			// заполнение пунктов меню
			for (const HistoryRecord *HistoryItem=HistoryList.toBegin(); HistoryItem != NULL; HistoryItem=HistoryList.toNext())
			{
				string strRecord = HistoryItem->strName;

				if (TypeHistory == HISTORYTYPE_VIEW)
				{
					strRecord = GetTitle(HistoryItem->Type);
					strRecord += L":";
					strRecord += (HistoryItem->Type==4?L"-":L" ");
					strRecord += HistoryItem->strName;;
				}
				else
				{
					strRecord = HistoryItem->strName;
				}

				ReplaceStrings(strRecord, L"&",L"&&", -1);

				MenuItem.Clear ();
				MenuItem.strName = strRecord;

				MenuItem.SetSelect(HistoryCurrentItem==HistoryItem || (!HistoryCurrentItem && HistoryList.isEnd()));
				HistoryMenu.SetUserData((void*)HistoryItem,sizeof(HistoryRecord *),HistoryMenu.AddItem(&MenuItem));
			}
			HistoryList.restorePosition();

			//MenuItem.Clear ();
			//MenuItem.strName = L"                    ";

			//if (!SetUpMenuPos)
				//MenuItem.SetSelect(CurLastPtr==-1 || CurLastPtr>=HistoryList.size());
			//HistoryMenu.SetUserData(NULL,sizeof(HistoryRecord *),HistoryMenu.AddItem(&MenuItem));

			if (SetUpMenuPos)
			{
				HistoryMenu.SetSelectPos(StrPos,0);
				SetUpMenuPos=false;
			}

			HistoryMenu.Show();
			while (!HistoryMenu.Done())
			{
				int Key=HistoryMenu.ReadInput();
				StrPos=HistoryMenu.GetSelectPos();

				switch(Key)
				{
					case KEY_CTRLR: // обновить с удалением недоступных
					{
						if (TypeHistory == HISTORYTYPE_FOLDER || TypeHistory == HISTORYTYPE_VIEW)
						{
							bool ModifiedHistory=false;
							for (const HistoryRecord *HistoryItem=HistoryList.toBegin(); HistoryItem != NULL;)
							{
								// убить запись из истории
								if (apiGetFileAttributes(HistoryItem->strName) == INVALID_FILE_ATTRIBUTES)
								{
									HistoryList.erase();
									HistoryItem = HistoryList.getItem();
									ModifiedHistory=true;
								}
								else
								{
									HistoryItem=HistoryList.toNext();
								}
							}
							if (ModifiedHistory) // избавляемся от лишних телодвижений
							{
								SaveHistory(); // сохранить
								HistoryMenu.Modal::SetExitCode(StrPos);
								HistoryMenu.SetUpdateRequired(TRUE);
								IsUpdate=true;
							}
							HistoryList.toEnd(); HistoryList.toNext(); //reset position in list
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
						HistoryMenu.Modal::SetExitCode(StrPos);
						Done=true;
						RetCode=Key==KEY_CTRLSHIFTENTER||Key==KEY_CTRLSHIFTNUMENTER?6:(Key==KEY_SHIFTENTER||Key==KEY_SHIFTNUMENTER?2:3);
						break;
					}

					case KEY_F3:
					case KEY_F4:
					case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:
					{
						HistoryMenu.Modal::SetExitCode(StrPos);
						Done=true;
						RetCode=(Key==KEY_F4? 5 : 4);
						break;
					}

					// $ 09.04.2001 SVS - Фича - копирование из истории строки в Clipboard
					case KEY_CTRLC:
					case KEY_CTRLINS:  case KEY_CTRLNUMPAD0:
					{
						const HistoryRecord *Record=(const HistoryRecord *)HistoryMenu.GetUserData(NULL,sizeof(HistoryRecord *),StrPos);

						if (Record)
							CopyToClipboard(Record->strName);

						break;
					}

					case KEY_NUMDEL:
					case KEY_DEL:
					{
						if (HistoryMenu.GetItemCount()/* > 1*/ &&
								(!Opt.Confirm.HistoryClear ||
								(Opt.Confirm.HistoryClear &&
								Message(MSG_WARNING,2,
										MSG((History::TypeHistory==HISTORYTYPE_CMD?MHistoryTitle:
													(History::TypeHistory==HISTORYTYPE_FOLDER?MFolderHistoryTitle:
													MViewHistoryTitle))),
										MSG(MHistoryClear),
										MSG(MClear),MSG(MCancel))==0)))
						{
							HistoryMenu.Hide();
							EmptyHistory();
							SaveHistory();
							HistoryMenu.Modal::SetExitCode(StrPos);
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
			if (Code > 0)
			{
				SelectedRecord=(const HistoryRecord *)HistoryMenu.GetUserData(NULL,sizeof(HistoryRecord *),Code);

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
					HistoryMenu.Modal::SetExitCode(StrPos=Code);
					continue;
				}
			}
		}
	}

	if (Code < 0 || !SelectedRecord)
		return 0;

	if (KeepSelectedPos)
	{
		for (const HistoryRecord *HistoryItem=HistoryList.toBegin(); HistoryItem != NULL; HistoryItem=HistoryList.toNext())
		{
			if (HistoryItem == SelectedRecord)
				break;
		}
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
	const HistoryRecord *Record = HistoryList.getItem();

	if (!Record)
	{
		HistoryList.toEnd();
	}
	else if (!HistoryList.toPrev())
	{
		HistoryList.toBegin();
	}

	Record = HistoryList.getItem();

	if (Record)
		strStr = Record->strName;
	else
		strStr = L"";
}


void History::GetNext(string &strStr)
{
	const HistoryRecord *Record = HistoryList.toNext();

	if (Record)
		strStr = Record->strName;
	else
		strStr = L"";
}


void History::GetSimilar(string &strStr,int LastCmdPartLength)
{
	int Length=(int)strStr.GetLength ();

	if (LastCmdPartLength!=-1 && LastCmdPartLength<Length)
		Length=LastCmdPartLength;

	HistoryList.storePosition();

	if (LastCmdPartLength==-1)
	{
		HistoryList.toEnd();
		HistoryList.toNext();
	}

	string strTmp;

	while (!HistoryList.isBegin())
	{
		GetPrev(strTmp);
		if (StrCmpNI(strStr,strTmp,Length)==0 && StrCmp(strStr,strTmp)!=0)
		{
			strStr = strTmp;
			return;
		}
	}

	HistoryList.restorePosition();

	const HistoryRecord *StopRecord = HistoryList.getItem();
	HistoryList.toEnd();
	HistoryList.toNext();

	if (StopRecord)
	{
		while (StopRecord != HistoryList.getItem())
		{
			GetPrev(strTmp);
			if (StrCmpNI(strStr,strTmp,Length)==0 && StrCmp(strStr,strTmp)!=0)
			{
				strStr = strTmp;
				return;
			}
		}
	}

	HistoryList.restorePosition();
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
