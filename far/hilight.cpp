/*
hilight.cpp

Files highlighting
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
#include "hilight.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "filelist.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "palette.hpp"
#include "message.hpp"
#include "config.hpp"
#include "interf.hpp"
#include "configdb.hpp"

struct HighlightStrings
{
	const wchar_t *UseAttr,*IncludeAttributes,*ExcludeAttributes,*AttrSet,*AttrClear,
	*IgnoreMask,*UseMask,*Mask,
	*NormalColor,*SelectedColor,*CursorColor,*SelectedCursorColor,
	*MarkCharNormalColor,*MarkCharSelectedColor,*MarkCharCursorColor,*MarkCharSelectedCursorColor,
	*MarkChar,
	*ContinueProcessing,
	*UseDate,*DateType,*DateAfter,*DateBefore,*DateRelative,
	*UseSize,*SizeAbove,*SizeBelow,
	*HighlightEdit,*HighlightList;
};

static const HighlightStrings HLS=
{
	L"UseAttr",L"IncludeAttributes",L"ExcludeAttributes",L"AttrSet",L"AttrClear",
	L"IgnoreMask",L"UseMask",L"Mask",
	L"NormalColor",L"SelectedColor",L"CursorColor",L"SelectedCursorColor",
	L"MarkCharNormalColor",L"MarkCharSelectedColor",L"MarkCharCursorColor",L"MarkCharSelectedCursorColor",
	L"MarkChar",
	L"ContinueProcessing",
	L"UseDate",L"DateType",L"DateAfter",L"DateBefore",L"DateRelative",
	L"UseSize",L"SizeAboveS",L"SizeBelowS",
	L"HighlightEdit",L"HighlightList"
};

static const wchar_t fmtFirstGroup[]=L"Group%d";
static const wchar_t fmtUpperGroup[]=L"UpperGroup%d";
static const wchar_t fmtLowerGroup[]=L"LowerGroup%d";
static const wchar_t fmtLastGroup[]=L"LastGroup%d";
static const wchar_t SortGroupsKeyName[]=L"SortGroups";
static const wchar_t HighlightKeyName[]=L"Highlight";

HighlightFiles::HighlightFiles()
{
	InitHighlightFiles();
	UpdateCurrentTime();
}

void LoadFilter(HierarchicalConfig *cfg, unsigned __int64 key, FileFilterParams *HData, const wchar_t *Mask, int SortGroup, bool bSortGroup)
{
	//Дефолтные значения выбраны так чтоб как можно правильней загрузить
	//настройки старых версий фара.
	if (bSortGroup)
	{
		unsigned __int64 UseMask = 1;
		cfg->GetValue(key,HLS.UseMask,&UseMask);
		HData->SetMask(UseMask!=0, Mask);
	}
	else
	{
		unsigned __int64 IgnoreMask = 0;
		cfg->GetValue(key,HLS.IgnoreMask,&IgnoreMask);
		HData->SetMask(IgnoreMask==0, Mask);
	}

	FILETIME DateAfter = {}, DateBefore = {};
	cfg->GetValue(key,HLS.DateAfter,(char *)&DateAfter,(int)sizeof(DateAfter));
	cfg->GetValue(key,HLS.DateBefore,(char *)&DateBefore,(int)sizeof(DateBefore));
	unsigned __int64 UseDate = 0;
	cfg->GetValue(key,HLS.UseDate,&UseDate);
	unsigned __int64 DateType = 0;
	cfg->GetValue(key,HLS.DateType,&DateType);
	unsigned __int64 DateRelative = 0;
	cfg->GetValue(key,HLS.DateRelative, &DateRelative);
	HData->SetDate(UseDate!=0, (DWORD)DateType, DateAfter, DateBefore, DateRelative!=0);

	string strSizeAbove;
	string strSizeBelow;
	cfg->GetValue(key,HLS.SizeAbove,strSizeAbove);
	cfg->GetValue(key,HLS.SizeBelow,strSizeBelow);
	unsigned __int64 UseSize = 0;
	cfg->GetValue(key,HLS.UseSize,&UseSize);
	HData->SetSize(UseSize!=0, strSizeAbove, strSizeBelow);

	if (bSortGroup)
	{
		unsigned __int64 UseAttr = 1;
		cfg->GetValue(key,HLS.UseAttr,&UseAttr);
		unsigned __int64 AttrSet = 0;
		cfg->GetValue(key,HLS.AttrSet,&AttrSet);
		unsigned __int64 AttrClear = FILE_ATTRIBUTE_DIRECTORY;
		cfg->GetValue(key,HLS.AttrClear,&AttrClear);
		HData->SetAttr(UseAttr!=0, (DWORD)AttrSet, (DWORD)AttrClear);
	}
	else
	{
		unsigned __int64 UseAttr = 1;
		cfg->GetValue(key,HLS.UseAttr,&UseAttr);
		unsigned __int64 IncludeAttributes = 0;
		cfg->GetValue(key,HLS.IncludeAttributes,&IncludeAttributes);
		unsigned __int64 ExcludeAttributes = 0;
		cfg->GetValue(key,HLS.ExcludeAttributes,&ExcludeAttributes);
		HData->SetAttr(UseAttr!=0, (DWORD)IncludeAttributes, (DWORD)ExcludeAttributes);
	}

	HData->SetSortGroup(SortGroup);

	HighlightDataColor Colors = {};
	unsigned __int64 color;
	if (cfg->GetValue(key,HLS.NormalColor,&color))
		Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_NORMAL]=(WORD)color;
	if (cfg->GetValue(key,HLS.SelectedColor,&color))
		Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_SELECTED]=(WORD)color;
	if (cfg->GetValue(key,HLS.CursorColor,&color))
		Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_UNDERCURSOR]=(WORD)color;
	if (cfg->GetValue(key,HLS.SelectedCursorColor,&color))
		Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR]=(WORD)color;
	if (cfg->GetValue(key,HLS.MarkCharNormalColor,&color))
		Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_NORMAL]=(WORD)color;
	if (cfg->GetValue(key,HLS.MarkCharSelectedColor,&color))
		Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_SELECTED]=(WORD)color;
	if (cfg->GetValue(key,HLS.MarkCharCursorColor,&color))
		Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_UNDERCURSOR]=(WORD)color;
	if (cfg->GetValue(key,HLS.MarkCharSelectedCursorColor,&color))
		Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR]=(WORD)color;
	if (cfg->GetValue(key,HLS.MarkChar,&color))
		Colors.MarkChar=(DWORD)color;
	HData->SetColors(&Colors);

	unsigned __int64 ContinueProcessing = 0;
	cfg->GetValue(key,HLS.ContinueProcessing,&ContinueProcessing);
	HData->SetContinueProcessing(ContinueProcessing!=0);
}

void HighlightFiles::InitHighlightFiles()
{
	string strGroupName, strMask;
	const int GroupDelta[4]={DEFAULT_SORT_GROUP,0,DEFAULT_SORT_GROUP+1,DEFAULT_SORT_GROUP};
	const wchar_t *KeyNames[4]={HighlightKeyName,SortGroupsKeyName,SortGroupsKeyName,HighlightKeyName};
	const wchar_t *GroupNames[4]={fmtFirstGroup,fmtUpperGroup,fmtLowerGroup,fmtLastGroup};
	int  *Count[4] = {&FirstCount,&UpperCount,&LowerCount,&LastCount};
	HiData.Free();
	FirstCount=UpperCount=LowerCount=LastCount=0;
	HierarchicalConfig *cfg = CreateHighlightConfig();

	for (int j=0; j<4; j++)
	{
		unsigned __int64 root = cfg->GetKeyID(0,KeyNames[j]);
		if (!root)
			continue;

		for (int i=0;; i++)
		{
			strGroupName.Format(GroupNames[j],i);
			unsigned __int64 key = cfg->GetKeyID(root,strGroupName);
			if (!key)
				break;

			if (!cfg->GetValue(key,HLS.Mask,strMask))
				break;

			FileFilterParams *HData = HiData.addItem();

			if (HData)
			{
				LoadFilter(cfg,key,HData,strMask,GroupDelta[j]+(GroupDelta[j]==DEFAULT_SORT_GROUP?0:i),(GroupDelta[j]==DEFAULT_SORT_GROUP?false:true));
				(*(Count[j]))++;
			}
			else
				break;
		}
	}

	delete cfg;
}


HighlightFiles::~HighlightFiles()
{
	ClearData();
}

void HighlightFiles::ClearData()
{
	HiData.Free();
	FirstCount=UpperCount=LowerCount=LastCount=0;
}

static const DWORD FarColor[] = {COL_PANELTEXT,COL_PANELSELECTEDTEXT,COL_PANELCURSOR,COL_PANELSELECTEDCURSOR};

void ApplyDefaultStartingColors(HighlightDataColor *Colors)
{
	for (int j=0; j<2; j++)
		for (int i=0; i<4; i++)
			Colors->Color[j][i]=0xFF00;

	Colors->MarkChar=0x00FF0000;
}

void ApplyBlackOnBlackColors(HighlightDataColor *Colors)
{
	for (int i=0; i<4; i++)
	{
		//Применим black on black.
		//Для файлов возьмем цвета панели не изменяя прозрачность.
		//Для пометки возьмем цвета файла включая прозрачность.
		if (!(Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i]&0x00FF))
			Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i]=(Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i]&0xFF00)|(0x00FF&Palette[FarColor[i]-COL_FIRSTPALETTECOLOR]);

		if (!(Colors->Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i]&0x00FF))
			Colors->Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i]=Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i];
	}
}

void ApplyColors(HighlightDataColor *DestColors, HighlightDataColor *SrcColors)
{
	//Обработаем black on black чтоб наследовать правильные цвета
	//и чтоб после наследования были правильные цвета.
	ApplyBlackOnBlackColors(DestColors);
	ApplyBlackOnBlackColors(SrcColors);

	for (int j=0; j<2; j++)
	{
		for (int i=0; i<4; i++)
		{
			//Если текущие цвета в Src (fore и/или back) не прозрачные
			//то унаследуем их в Dest.
			if (!(SrcColors->Color[j][i]&0xF000))
				DestColors->Color[j][i]=(DestColors->Color[j][i]&0x0F0F)|(SrcColors->Color[j][i]&0xF0F0);

			if (!(SrcColors->Color[j][i]&0x0F00))
				DestColors->Color[j][i]=(DestColors->Color[j][i]&0xF0F0)|(SrcColors->Color[j][i]&0x0F0F);
		}
	}

	//Унаследуем пометку из Src если она не прозрачная
	if (!(SrcColors->MarkChar&0x00FF0000))
		DestColors->MarkChar=SrcColors->MarkChar;
}

/*
bool HasTransparent(HighlightDataColor *Colors)
{
  for (int j=0; j<2; j++)
    for (int i=0; i<4; i++)
      if (Colors->Color[j][i]&0xFF00)
        return true;

  if (Colors->MarkChar&0x00FF0000)
    return true;

  return false;
}
*/

void ApplyFinalColors(HighlightDataColor *Colors)
{
	//Обработаем black on black чтоб после наследования были правильные цвета.
	ApplyBlackOnBlackColors(Colors);

	for (int j=0; j<2; j++)
		for (int i=0; i<4; i++)
		{
			//Если какой то из текущих цветов (fore или back) прозрачный
			//то унаследуем соответствующий цвет с панелей.
			BYTE temp=(BYTE)((Colors->Color[j][i]&0xFF00)>>8);
			Colors->Color[j][i]=((~temp)&(BYTE)Colors->Color[j][i])|(temp&(BYTE)Palette[FarColor[i]-COL_FIRSTPALETTECOLOR]);
		}

	//Если символ пометки прозрачный то его как бы и нет вообще.
	if (Colors->MarkChar&0x00FF0000)
		Colors->MarkChar=0;

	//Параноя но случится может:
	//Обработаем black on black снова чтоб обработались унаследованые цвета.
	ApplyBlackOnBlackColors(Colors);
}

void HighlightFiles::UpdateCurrentTime()
{
	SYSTEMTIME cst;
	FILETIME cft;
	GetSystemTime(&cst);
	SystemTimeToFileTime(&cst, &cft);
	ULARGE_INTEGER current;
	current.u.LowPart  = cft.dwLowDateTime;
	current.u.HighPart = cft.dwHighDateTime;
	CurrentTime = current.QuadPart;
}

void HighlightFiles::GetHiColor(FileListItem **FileItem,int FileCount,bool UseAttrHighlighting)
{
	if (!FileItem || !FileCount)
		return;

	FileFilterParams *CurHiData;

	for (int FCnt=0; FCnt < FileCount; ++FCnt)
	{
		FileListItem& fli = *FileItem[FCnt];
		ApplyDefaultStartingColors(&fli.Colors);

		for (size_t i=0; i < HiData.getCount(); i++)
		{
			CurHiData = HiData.getItem(i);

			if (UseAttrHighlighting && CurHiData->GetMask(nullptr))
				continue;

			if (CurHiData->FileInFilter(fli, CurrentTime))
			{
				HighlightDataColor TempColors;
				CurHiData->GetColors(&TempColors);
				ApplyColors(&fli.Colors,&TempColors);

				if (!CurHiData->GetContinueProcessing())// || !HasTransparent(&fli->Colors))
					break;
			}
		}

		ApplyFinalColors(&fli.Colors);
	}
}

int HighlightFiles::GetGroup(const FileListItem *fli)
{
	for (int i=FirstCount; i<FirstCount+UpperCount; i++)
	{
		FileFilterParams *CurGroupData=HiData.getItem(i);

		if (CurGroupData->FileInFilter(*fli, CurrentTime))
			return(CurGroupData->GetSortGroup());
	}

	for (int i=FirstCount+UpperCount; i<FirstCount+UpperCount+LowerCount; i++)
	{
		FileFilterParams *CurGroupData=HiData.getItem(i);

		if (CurGroupData->FileInFilter(*fli, CurrentTime))
			return(CurGroupData->GetSortGroup());
	}

	return DEFAULT_SORT_GROUP;
}

void HighlightFiles::FillMenu(VMenu *HiMenu,int MenuPos)
{
	MenuItemEx HiMenuItem;
	const int Count[4][2] =
	{
		{0,                               FirstCount},
		{FirstCount,                      FirstCount+UpperCount},
		{FirstCount+UpperCount,           FirstCount+UpperCount+LowerCount},
		{FirstCount+UpperCount+LowerCount,FirstCount+UpperCount+LowerCount+LastCount}
	};
	HiMenu->DeleteItems();
	HiMenuItem.Clear();

	for (int j=0; j<4; j++)
	{
		for (int i=Count[j][0]; i<Count[j][1]; i++)
		{
			MenuString(HiMenuItem.strName,HiData.getItem(i),true);
			HiMenu->AddItem(&HiMenuItem);
		}

		HiMenuItem.strName.Clear();
		HiMenu->AddItem(&HiMenuItem);

		if (j<3)
		{
			if (!j)
				HiMenuItem.strName = MSG(MHighlightUpperSortGroup);
			else if (j==1)
				HiMenuItem.strName = MSG(MHighlightLowerSortGroup);
			else
				HiMenuItem.strName = MSG(MHighlightLastGroup);

			HiMenuItem.Flags|=LIF_SEPARATOR;
			HiMenu->AddItem(&HiMenuItem);
			HiMenuItem.Flags=0;
		}
	}

	HiMenu->SetSelectPos(MenuPos,1);
}

void HighlightFiles::ProcessGroups()
{
	for (int i=0; i<FirstCount; i++)
		HiData.getItem(i)->SetSortGroup(DEFAULT_SORT_GROUP);

	for (int i=FirstCount; i<FirstCount+UpperCount; i++)
		HiData.getItem(i)->SetSortGroup(i-FirstCount);

	for (int i=FirstCount+UpperCount; i<FirstCount+UpperCount+LowerCount; i++)
		HiData.getItem(i)->SetSortGroup(DEFAULT_SORT_GROUP+1+i-FirstCount-UpperCount);

	for (int i=FirstCount+UpperCount+LowerCount; i<FirstCount+UpperCount+LowerCount+LastCount; i++)
		HiData.getItem(i)->SetSortGroup(DEFAULT_SORT_GROUP);
}

int HighlightFiles::MenuPosToRealPos(int MenuPos, int **Count, bool Insert)
{
	int Pos=MenuPos;
	*Count=nullptr;
	int x = Insert ? 1 : 0;

	if (MenuPos<FirstCount+x)
	{
		*Count=&FirstCount;
	}
	else if (MenuPos>FirstCount+1 && MenuPos<FirstCount+UpperCount+2+x)
	{
		Pos=MenuPos-2;
		*Count=&UpperCount;
	}
	else if (MenuPos>FirstCount+UpperCount+3 && MenuPos<FirstCount+UpperCount+LowerCount+4+x)
	{
		Pos=MenuPos-4;
		*Count=&LowerCount;
	}
	else if (MenuPos>FirstCount+UpperCount+LowerCount+5 && MenuPos<FirstCount+UpperCount+LowerCount+LastCount+6+x)
	{
		Pos=MenuPos-6;
		*Count=&LastCount;
	}

	return Pos;
}

void HighlightFiles::HiEdit(int MenuPos)
{
	VMenu HiMenu(MSG(MHighlightTitle),nullptr,0,ScrY-4);
	HiMenu.SetHelp(HLS.HighlightList);
	HiMenu.SetFlags(VMENU_WRAPMODE|VMENU_SHOWAMPERSAND);
	HiMenu.SetPosition(-1,-1,0,0);
	HiMenu.SetBottomTitle(MSG(MHighlightBottom));
	FillMenu(&HiMenu,MenuPos);
	int NeedUpdate;
	Panel *LeftPanel=CtrlObject->Cp()->LeftPanel;
	Panel *RightPanel=CtrlObject->Cp()->RightPanel;
	HiMenu.Show();

	while (1)
	{
		while (!HiMenu.Done())
		{
			int Key=HiMenu.ReadInput();
			int SelectPos=HiMenu.GetSelectPos();
			NeedUpdate=FALSE;

			switch (Key)
			{
					/* $ 07.07.2000 IS
					  Если нажали ctrl+r, то восстановить значения по умолчанию.
					*/
				case KEY_CTRLR:
				{

					if (Message(MSG_WARNING,2,MSG(MHighlightTitle),
					            MSG(MHighlightWarning),MSG(MHighlightAskRestore),
					            MSG(MYes),MSG(MCancel)))
						break;

					SetHighlighting(true); //delete old settings
					HiMenu.Hide();
					ClearData();
					InitHighlightFiles();
					NeedUpdate=TRUE;
					break;
				}

				case KEY_NUMDEL:
				case KEY_DEL:
				{
					int *Count=nullptr;
					int RealSelectPos=MenuPosToRealPos(SelectPos,&Count);

					if (Count && RealSelectPos<(int)HiData.getCount())
					{
						const wchar_t *Mask;
						HiData.getItem(RealSelectPos)->GetMask(&Mask);

						if (Message(MSG_WARNING,2,MSG(MHighlightTitle),
						            MSG(MHighlightAskDel),Mask,
						            MSG(MDelete),MSG(MCancel)))
							break;

						HiData.deleteItem(RealSelectPos);
						(*Count)--;
						NeedUpdate=TRUE;
					}

					break;
				}

				case KEY_NUMENTER:
				case KEY_ENTER:
				case KEY_F4:
				{
					int *Count=nullptr;
					int RealSelectPos=MenuPosToRealPos(SelectPos,&Count);

					if (Count && RealSelectPos<(int)HiData.getCount())
						if (FileFilterConfig(HiData.getItem(RealSelectPos),true))
							NeedUpdate=TRUE;

					break;
				}

				case KEY_INS: case KEY_NUMPAD0:
				{
					int *Count=nullptr;
					int RealSelectPos=MenuPosToRealPos(SelectPos,&Count,true);

					if (Count)
					{
						FileFilterParams *NewHData = HiData.insertItem(RealSelectPos);

						if (!NewHData)
							break;

						if (FileFilterConfig(NewHData,true))
						{
							(*Count)++;
							NeedUpdate=TRUE;
						}
						else
							HiData.deleteItem(RealSelectPos);
					}

					break;
				}

				case KEY_F5:
				{
					int *Count=nullptr;
					int RealSelectPos=MenuPosToRealPos(SelectPos,&Count);

					if (Count && RealSelectPos<(int)HiData.getCount())
					{
						FileFilterParams *HData = HiData.insertItem(RealSelectPos);

						if (HData)
						{
							*HData = *HiData.getItem(RealSelectPos+1);
							HData->SetTitle(L"");

							if (FileFilterConfig(HData,true))
							{
								NeedUpdate=TRUE;
								(*Count)++;
							}
							else
								HiData.deleteItem(RealSelectPos);
						}
					}

					break;
				}

				case KEY_CTRLUP: case KEY_CTRLNUMPAD8:
				{
					int *Count=nullptr;
					int RealSelectPos=MenuPosToRealPos(SelectPos,&Count);

					if (Count && SelectPos > 0)
					{
						if (UpperCount && RealSelectPos==FirstCount && RealSelectPos<FirstCount+UpperCount)
						{
							FirstCount++;
							UpperCount--;
							SelectPos--;
						}
						else if (LowerCount && RealSelectPos==FirstCount+UpperCount && RealSelectPos<FirstCount+UpperCount+LowerCount)
						{
							UpperCount++;
							LowerCount--;
							SelectPos--;
						}
						else if (LastCount && RealSelectPos==FirstCount+UpperCount+LowerCount)
						{
							LowerCount++;
							LastCount--;
							SelectPos--;
						}
						else
							HiData.swapItems(RealSelectPos,RealSelectPos-1);

						HiMenu.SetCheck(--SelectPos);
						NeedUpdate=TRUE;
						break;
					}

					HiMenu.ProcessInput();
					break;
				}

				case KEY_CTRLDOWN: case KEY_CTRLNUMPAD2:
				{
					int *Count=nullptr;
					int RealSelectPos=MenuPosToRealPos(SelectPos,&Count);

					if (Count && SelectPos < (int)HiMenu.GetItemCount()-2)
					{
						if (FirstCount && RealSelectPos==FirstCount-1)
						{
							FirstCount--;
							UpperCount++;
							SelectPos++;
						}
						else if (UpperCount && RealSelectPos==FirstCount+UpperCount-1)
						{
							UpperCount--;
							LowerCount++;
							SelectPos++;
						}
						else if (LowerCount && RealSelectPos==FirstCount+UpperCount+LowerCount-1)
						{
							LowerCount--;
							LastCount++;
							SelectPos++;
						}
						else
							HiData.swapItems(RealSelectPos,RealSelectPos+1);

						HiMenu.SetCheck(++SelectPos);
						NeedUpdate=TRUE;
					}

					HiMenu.ProcessInput();
					break;
				}

				default:
					HiMenu.ProcessInput();
					break;
			}

			// повторяющийся кусок!
			if (NeedUpdate)
			{
				ScrBuf.Lock(); // отменяем всякую прорисовку
				HiMenu.Hide();
				ProcessGroups();

				if (Opt.AutoSaveSetup)
					SaveHiData();

				//FrameManager->RefreshFrame(); // рефрешим
				LeftPanel->Update(UPDATE_KEEP_SELECTION);
				LeftPanel->Redraw();
				RightPanel->Update(UPDATE_KEEP_SELECTION);
				RightPanel->Redraw();
				FillMenu(&HiMenu,MenuPos=SelectPos);
				HiMenu.SetPosition(-1,-1,0,0);
				HiMenu.Show();
				ScrBuf.Unlock(); // разрешаем прорисовку
			}
		}

		if (HiMenu.Modal::GetExitCode()!=-1)
		{
			HiMenu.ClearDone();
			HiMenu.WriteInput(KEY_F4);
			continue;
		}

		break;
	}
}

void SaveFilter(HierarchicalConfig *cfg, unsigned __int64 key, FileFilterParams *CurHiData, bool bSortGroup)
{
	if (bSortGroup)
	{
		const wchar_t *Mask;
		cfg->SetValue(key,HLS.UseMask,CurHiData->GetMask(&Mask));
		cfg->SetValue(key,HLS.Mask,Mask);
	}
	else
	{
		const wchar_t *Mask;
		cfg->SetValue(key,HLS.IgnoreMask,CurHiData->GetMask(&Mask)?0:1);
		cfg->SetValue(key,HLS.Mask,Mask);
	}

	DWORD DateType;
	FILETIME DateAfter, DateBefore;
	bool bRelative;
	cfg->SetValue(key,HLS.UseDate,CurHiData->GetDate(&DateType, &DateAfter, &DateBefore, &bRelative)?1:0);
	cfg->SetValue(key,HLS.DateType,DateType);
	cfg->SetValue(key,HLS.DateAfter,(const char *)&DateAfter,(int)sizeof(DateAfter));
	cfg->SetValue(key,HLS.DateBefore,(const char *)&DateBefore,(int)sizeof(DateBefore));
	cfg->SetValue(key,HLS.DateRelative,bRelative?1:0);
	const wchar_t *SizeAbove, *SizeBelow;
	cfg->SetValue(key,HLS.UseSize,CurHiData->GetSize(&SizeAbove, &SizeBelow)?1:0);
	cfg->SetValue(key,HLS.SizeAbove,SizeAbove);
	cfg->SetValue(key,HLS.SizeBelow,SizeBelow);
	DWORD AttrSet, AttrClear;
	cfg->SetValue(key,HLS.UseAttr,CurHiData->GetAttr(&AttrSet, &AttrClear)?1:0);
	cfg->SetValue(key,(bSortGroup?HLS.AttrSet:HLS.IncludeAttributes),AttrSet);
	cfg->SetValue(key,(bSortGroup?HLS.AttrClear:HLS.ExcludeAttributes),AttrClear);
	HighlightDataColor Colors;
	CurHiData->GetColors(&Colors);
	cfg->SetValue(key,HLS.NormalColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_NORMAL]);
	cfg->SetValue(key,HLS.SelectedColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_SELECTED]);
	cfg->SetValue(key,HLS.CursorColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_UNDERCURSOR]);
	cfg->SetValue(key,HLS.SelectedCursorColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR]);
	cfg->SetValue(key,HLS.MarkCharNormalColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_NORMAL]);
	cfg->SetValue(key,HLS.MarkCharSelectedColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_SELECTED]);
	cfg->SetValue(key,HLS.MarkCharCursorColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_UNDERCURSOR]);
	cfg->SetValue(key,HLS.MarkCharSelectedCursorColor,(DWORD)Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR]);
	cfg->SetValue(key,HLS.MarkChar,Colors.MarkChar);
	cfg->SetValue(key,HLS.ContinueProcessing,(CurHiData->GetContinueProcessing()?1:0));
}

void HighlightFiles::SaveHiData()
{
	string strRegKey, strGroupName;
	const wchar_t *KeyNames[4]={HighlightKeyName,SortGroupsKeyName,SortGroupsKeyName,HighlightKeyName};
	const wchar_t *GroupNames[4]={fmtFirstGroup,fmtUpperGroup,fmtLowerGroup,fmtLastGroup};
	const int Count[4][2] =
	{
		{0,                               FirstCount},
		{FirstCount,                      FirstCount+UpperCount},
		{FirstCount+UpperCount,           FirstCount+UpperCount+LowerCount},
		{FirstCount+UpperCount+LowerCount,FirstCount+UpperCount+LowerCount+LastCount}
	};

	HierarchicalConfig *cfg = CreateHighlightConfig();

	unsigned __int64 root = cfg->GetKeyID(0, HighlightKeyName);
	if (root)
		cfg->DeleteKeyTree(root);
	root = cfg->GetKeyID(0, SortGroupsKeyName);
	if (root)
		cfg->DeleteKeyTree(root);

	for (int j=0; j<4; j++)
	{
		root = cfg->CreateKey(0, KeyNames[j]);
		if (!root)
			continue;

		for (int i=Count[j][0]; i<Count[j][1]; i++)
		{
			strGroupName.Format(GroupNames[j],i-Count[j][0]);
			unsigned __int64 key = cfg->CreateKey(root,strGroupName);
			if (!key)
				break;

			FileFilterParams *CurHiData=HiData.getItem(i);

			SaveFilter(cfg,key,CurHiData,(!j || j==3)?false:true);
		}
	}

	delete cfg;
}

void SetHighlighting(bool DeleteOld)
{
	unsigned __int64 root;
	HierarchicalConfig *cfg = CreateHighlightConfig();

	if (DeleteOld)
	{
		root = cfg->GetKeyID(0, HighlightKeyName);
		if (root)
			cfg->DeleteKeyTree(root);
	}

	if (cfg->GetKeyID(0, HighlightKeyName))
	{
		delete cfg;
		return;
	}

	root = cfg->CreateKey(0,HighlightKeyName);
	if (!root)
	{
		delete cfg;
		return;
	}

	static const wchar_t *Masks[]=
	{
		/* 0 */ L"*.*",
		/* 1 */ L"*.rar,*.zip,*.[zj],*.[bg7]z,*.[bg]zip,*.tar,*.t[agbx]z,*.ar[cj],*.r[0-9][0-9],*.a[0-9][0-9],*.bz2,*.cab,*.msi,*.jar,*.lha,*.lzh,*.ha,*.ac[bei],*.pa[ck],*.rk,*.cpio,*.rpm,*.zoo,*.hqx,*.sit,*.ice,*.uc2,*.ain,*.imp,*.777,*.ufa,*.boa,*.bs[2a],*.sea,*.hpk,*.ddi,*.x2,*.rkv,*.[lw]sz,*.h[ay]p,*.lim,*.sqz,*.chz",
		/* 2 */ L"*.bak,*.tmp",                                                                                                                                                                                //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ -> может к терапевту? ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		/* $ 25.09.2001  IS
		    Эта маска для каталогов: обрабатывать все каталоги, кроме тех, что
		    являются родительскими (их имена - две точки).
		*/
		/* 3 */ L"*.*|..", // маска для каталогов
		/* 4 */ L"..",     // такие каталоги окрашивать как простые файлы
	};
	static struct DefaultData
	{
		const wchar_t *Mask;
		int IgnoreMask;
		DWORD IncludeAttr;
		BYTE NormalColor;
		BYTE CursorColor;
	}
	StdHighlightData[]=
	    { /*
             Mask                NormalColor
                          IncludeAttributes
                       IgnoreMask       CursorColor             */
	        /* 0 */{Masks[0], 0, 0x0002, 0x13, 0x38},
	        /* 1 */{Masks[0], 0, 0x0004, 0x13, 0x38},
	        /* 2 */{Masks[3], 0, 0x0010, 0x1F, 0x3F},
	        /* 3 */{Masks[4], 0, 0x0010, 0x00, 0x00},
	        /* 4 */{L"*.exe,*.com,*.bat,*.cmd,%PATHEXT%",0, 0x0000, 0x1A, 0x3A},
	        /* 5 */{Masks[1], 0, 0x0000, 0x1D, 0x3D},
	        /* 6 */{Masks[2], 0, 0x0000, 0x16, 0x36},
	        // это настройка для каталогов на тех панелях, которые должны раскрашиваться
	        // без учета масок (например, список хостов в "far navigator")
	        /* 7 */{Masks[0], 1, 0x0010, 0x1F, 0x3F},
	    };

	for (size_t I=0; I < ARRAYSIZE(StdHighlightData); I++)
	{
		FormatString strKeyName;
		strKeyName << L"Group" << I;
		unsigned __int64 key = cfg->CreateKey(root,strKeyName);
		if (!key)
			break;
		cfg->SetValue(key,HLS.Mask,StdHighlightData[I].Mask);
		cfg->SetValue(key,HLS.IgnoreMask,StdHighlightData[I].IgnoreMask);
		cfg->SetValue(key,HLS.IncludeAttributes,StdHighlightData[I].IncludeAttr);
		cfg->SetValue(key,HLS.NormalColor,StdHighlightData[I].NormalColor);
		cfg->SetValue(key,HLS.CursorColor,StdHighlightData[I].CursorColor);
	}

	delete cfg;
}
