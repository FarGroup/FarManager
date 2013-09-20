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
#include "keys.hpp"
#include "vmenu2.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "filelist.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "message.hpp"
#include "config.hpp"
#include "interf.hpp"
#include "configdb.hpp"
#include "colormix.hpp"
#include "filefilterparams.hpp"
#include "language.hpp"

struct HighlightStrings
{
	const wchar_t *UseAttr,*IncludeAttributes,*ExcludeAttributes,*AttrSet,*AttrClear,
	*IgnoreMask,*UseMask,*Mask,

	*NormalColor,
	*SelectedColor,
	*CursorColor,
	*SelectedCursorColor,
	*MarkCharNormalColor,
	*MarkCharSelectedColor,
	*MarkCharCursorColor,
	*MarkCharSelectedCursorColor,

	*MarkChar,
	*ContinueProcessing,
	*UseDate,*DateType,*DateAfter,*DateBefore,*DateRelative,
	*UseSize,*SizeAbove,*SizeBelow,
	*UseHardLinks,*HardLinksAbove,*HardLinksBelow,
	*HighlightEdit,*HighlightList;
};

static const HighlightStrings HLS=
{
	L"UseAttr",L"IncludeAttributes",L"ExcludeAttributes",L"AttrSet",L"AttrClear",
	L"IgnoreMask",L"UseMask",L"Mask",

	L"NormalColor",
	L"SelectedColor",
	L"CursorColor",
	L"SelectedCursorColor",
	L"MarkCharNormalColor",
	L"MarkCharSelectedColor",
	L"MarkCharCursorColor",
	L"MarkCharSelectedCursorColor",

	L"MarkChar",
	L"ContinueProcessing",
	L"UseDate",L"DateType",L"DateAfter",L"DateBefore",L"DateRelative",
	L"UseSize",L"SizeAboveS",L"SizeBelowS",
	L"UseHardLinks",L"HardLinksAbove",L"HardLinksBelow",
	L"HighlightEdit",L"HighlightList"
};

static const wchar_t fmtFirstGroup[]=L"Group%d";
static const wchar_t fmtUpperGroup[]=L"UpperGroup%d";
static const wchar_t fmtLowerGroup[]=L"LowerGroup%d";
static const wchar_t fmtLastGroup[]=L"LastGroup%d";
static const wchar_t SortGroupsKeyName[]=L"SortGroups";
static const wchar_t HighlightKeyName[]=L"Highlight";

static void SetHighlighting(bool DeleteOld, HierarchicalConfig *cfg)
{
	unsigned __int64 root;

	if (DeleteOld)
	{
		root = cfg->GetKeyID(0, HighlightKeyName);
		if (root)
			cfg->DeleteKeyTree(root);
	}

	if (!cfg->GetKeyID(0, HighlightKeyName))
	{
		root = cfg->CreateKey(0,HighlightKeyName);
		if (root)
		{
			static const wchar_t *Masks[]=
			{
				/* 0 */ L"*.*",
				/* 1 */ L"<arc>",
				/* 2 */ L"<temp>",
				/* $ 25.09.2001  IS
					Эта маска для каталогов: обрабатывать все каталоги, кроме тех, что
					являются родительскими (их имена - две точки).
				*/
				/* 3 */ L"*.*|..", // маска для каталогов
				/* 4 */ L"..",     // такие каталоги окрашивать как простые файлы
				/* 5 */ L"<exec>",
			};
			static struct DefaultData
			{
				const wchar_t *Mask;
				int IgnoreMask;
				DWORD IncludeAttr;
				BYTE InitNC;
				BYTE InitCC;
				FarColor NormalColor;
				FarColor CursorColor;
			}
			StdHighlightData[]=
			{
				/* 0 */{Masks[0], 0, FILE_ATTRIBUTE_HIDDEN, B_BLUE|F_CYAN, B_CYAN|F_DARKGRAY},
				/* 1 */{Masks[0], 0, FILE_ATTRIBUTE_SYSTEM, B_BLUE|F_CYAN, B_CYAN|F_DARKGRAY},
				/* 2 */{Masks[3], 0, FILE_ATTRIBUTE_DIRECTORY, B_BLUE|F_WHITE, B_CYAN|F_WHITE},
				/* 3 */{Masks[4], 0, FILE_ATTRIBUTE_DIRECTORY, 0, 0},
				/* 4 */{Masks[5], 0, 0, B_BLUE|F_LIGHTGREEN, B_CYAN|F_LIGHTGREEN},
				/* 5 */{Masks[1], 0, 0, B_BLUE|F_LIGHTMAGENTA, B_CYAN|F_LIGHTMAGENTA},
				/* 6 */{Masks[2], 0, 0, B_BLUE|F_BROWN, B_CYAN|F_BROWN},
				// это настройка для каталогов на тех панелях, которые должны раскрашиваться
				// без учета масок (например, список хостов в "far navigator")
				/* 7 */{Masks[0], 1, FILE_ATTRIBUTE_DIRECTORY, B_BLUE|F_WHITE, B_CYAN|F_WHITE},
			};

			size_t Index = 0;
			FOR_RANGE(StdHighlightData, i)
			{
				i->NormalColor = Colors::ConsoleColorToFarColor(i->InitNC);
				MAKE_TRANSPARENT(i->NormalColor.BackgroundColor);
				i->CursorColor = Colors::ConsoleColorToFarColor(i->InitCC);
				MAKE_TRANSPARENT(i->CursorColor.BackgroundColor);

				unsigned __int64 key = cfg->CreateKey(root, L"Group" + std::to_wstring(Index++));
				if (!key)
					break;
				cfg->SetValue(key,HLS.Mask,i->Mask);
				cfg->SetValue(key,HLS.IgnoreMask,i->IgnoreMask);
				cfg->SetValue(key,HLS.IncludeAttributes,i->IncludeAttr);

				cfg->SetValue(key,HLS.NormalColor, &i->NormalColor, sizeof(FarColor));
				cfg->SetValue(key,HLS.CursorColor, &i->CursorColor, sizeof(FarColor));

				const FarColor DefaultColor = {FCF_FG_4BIT|FCF_BG_4BIT, 0xff000000, 0x00000000};
				cfg->SetValue(key,HLS.SelectedColor, &DefaultColor, sizeof(FarColor));
				cfg->SetValue(key,HLS.SelectedCursorColor, &DefaultColor, sizeof(FarColor));
				cfg->SetValue(key,HLS.MarkCharNormalColor, &DefaultColor, sizeof(FarColor));
				cfg->SetValue(key,HLS.MarkCharSelectedColor, &DefaultColor, sizeof(FarColor));
				cfg->SetValue(key,HLS.MarkCharCursorColor, &DefaultColor, sizeof(FarColor));
				cfg->SetValue(key,HLS.MarkCharSelectedCursorColor, &DefaultColor, sizeof(FarColor));
			}
		}
	}
}

HighlightFiles::HighlightFiles()
{
	Changed = false;
	auto cfg = Global->Db->CreateHighlightConfig();
	SetHighlighting(false, cfg.get());
	InitHighlightFiles(cfg.get());
	UpdateCurrentTime();
}

static void LoadFilter(HierarchicalConfig *cfg, unsigned __int64 key, FileFilterParams *HData, const string& Mask, int SortGroup, bool bSortGroup)
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
	cfg->GetValue(key,HLS.DateAfter, &DateAfter, sizeof(DateAfter));
	cfg->GetValue(key,HLS.DateBefore, &DateBefore, sizeof(DateBefore));
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

	unsigned __int64 UseHardLinks = 0;
	unsigned __int64 HardLinksAbove = 0;
	unsigned __int64 HardLinksBelow = 0;
	cfg->GetValue(key,HLS.UseHardLinks,&UseHardLinks);
	cfg->GetValue(key,HLS.HardLinksAbove,&HardLinksAbove);
	cfg->GetValue(key,HLS.HardLinksBelow,&HardLinksBelow);
	HData->SetHardLinks(UseHardLinks!=0,HardLinksAbove,HardLinksBelow);

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
	cfg->GetValue(key,HLS.NormalColor, &Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_NORMAL], sizeof(FarColor));
	cfg->GetValue(key,HLS.SelectedColor, &Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_SELECTED], sizeof(FarColor));
	cfg->GetValue(key,HLS.CursorColor, &Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_UNDERCURSOR], sizeof(FarColor));
	cfg->GetValue(key,HLS.SelectedCursorColor, &Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR], sizeof(FarColor));
	cfg->GetValue(key,HLS.MarkCharNormalColor, &Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_NORMAL], sizeof(FarColor));
	cfg->GetValue(key,HLS.MarkCharSelectedColor, &Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_SELECTED], sizeof(FarColor));
	cfg->GetValue(key,HLS.MarkCharCursorColor, &Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_UNDERCURSOR], sizeof(FarColor));
	cfg->GetValue(key,HLS.MarkCharSelectedCursorColor, &Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR], sizeof(FarColor));

	unsigned __int64 MarkChar;
	if (cfg->GetValue(key,HLS.MarkChar,&MarkChar))
		Colors.MarkChar=static_cast<DWORD>(MarkChar);
	HData->SetColors(&Colors);

	unsigned __int64 ContinueProcessing = 0;
	cfg->GetValue(key,HLS.ContinueProcessing,&ContinueProcessing);
	HData->SetContinueProcessing(ContinueProcessing!=0);
}

void HighlightFiles::InitHighlightFiles(HierarchicalConfig* cfg)
{
	const struct group_item
	{
		int Delta;
		const wchar_t* KeyName;
		const wchar_t* GroupName;
		int* Count;
	}
	GroupItems[] =
	{
		{DEFAULT_SORT_GROUP, HighlightKeyName, fmtFirstGroup, &FirstCount},
		{0, SortGroupsKeyName, fmtUpperGroup, &UpperCount},
		{DEFAULT_SORT_GROUP+1, SortGroupsKeyName, fmtLowerGroup, &LowerCount},
		{DEFAULT_SORT_GROUP, HighlightKeyName, fmtLastGroup, &LastCount},
	};

	HiData.clear();
	FirstCount=UpperCount=LowerCount=LastCount=0;

	std::for_each(CONST_RANGE(GroupItems, Item)
	{
		auto root = cfg->GetKeyID(0, Item.KeyName);
		if (root)
		{
			for (int i=0;; ++i)
			{
				auto key = cfg->GetKeyID(root, str_printf(Item.GroupName, i));
				if (!key)
					break;

				string strMask;
				if (!cfg->GetValue(key,HLS.Mask,strMask))
					break;

				HiData.emplace_back(new FileFilterParams);
				LoadFilter(cfg, key, HiData.back().get(), strMask, Item.Delta + (Item.Delta == DEFAULT_SORT_GROUP? 0 : i), Item.Delta != DEFAULT_SORT_GROUP);
				++*Item.Count;
			}
		}
	});
}


HighlightFiles::~HighlightFiles()
{
	ClearData();
}

void HighlightFiles::ClearData()
{
	HiData.clear();
	FirstCount=UpperCount=LowerCount=LastCount=0;
}

static const DWORD PalColor[] = {COL_PANELTEXT,COL_PANELSELECTEDTEXT,COL_PANELCURSOR,COL_PANELSELECTEDCURSOR};

static void ApplyDefaultStartingColors(HighlightDataColor *Colors)
{
	for (int j=0; j<2; j++)
	{
		for (int i=0; i<4; i++)
		{
			MAKE_OPAQUE(Colors->Color[j][i].ForegroundColor);
			MAKE_TRANSPARENT(Colors->Color[j][i].BackgroundColor);
		}
	}

	Colors->MarkChar=0x00FF0000;
}

static void ApplyBlackOnBlackColors(HighlightDataColor *Colors)
{
	for (int i=0; i<4; i++)
	{
		//Применим black on black.
		//Для файлов возьмем цвета панели не изменяя прозрачность.
		//Для пометки возьмем цвета файла включая прозрачность.
		if (!COLORVALUE(Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i].ForegroundColor) && !COLORVALUE(Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i].BackgroundColor))
		{
			FarColor NewColor = Global->Opt->Palette[PalColor[i]-COL_FIRSTPALETTECOLOR];
			Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i].BackgroundColor=ALPHAVALUE(Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i].BackgroundColor)|COLORVALUE(NewColor.BackgroundColor);
			Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i].ForegroundColor=ALPHAVALUE(Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i].ForegroundColor)|COLORVALUE(NewColor.ForegroundColor);
			Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i].Flags&=FCF_EXTENDEDFLAGS;
			Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i].Flags |= NewColor.Flags;
			Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i].Reserved = NewColor.Reserved;
		}
		if (!COLORVALUE(Colors->Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i].ForegroundColor) && !COLORVALUE(Colors->Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i].BackgroundColor))
		{
			Colors->Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i].BackgroundColor=ALPHAVALUE(Colors->Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i].BackgroundColor)|COLORVALUE(Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i].BackgroundColor);
			Colors->Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i].ForegroundColor=ALPHAVALUE(Colors->Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i].ForegroundColor)|COLORVALUE(Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i].ForegroundColor);
			Colors->Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i].Flags&=FCF_EXTENDEDFLAGS;
			Colors->Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i].Flags |= Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i].Flags;
			Colors->Color[HIGHLIGHTCOLORTYPE_MARKCHAR][i].Reserved = Colors->Color[HIGHLIGHTCOLORTYPE_FILE][i].Reserved;
		}
	}
}

static void ApplyColors(HighlightDataColor *DestColors, HighlightDataColor *SrcColors)
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
			if (IS_OPAQUE(SrcColors->Color[j][i].ForegroundColor))
			{
				DestColors->Color[j][i].ForegroundColor=SrcColors->Color[j][i].ForegroundColor;
				SrcColors->Color[j][i].Flags&FCF_FG_4BIT? DestColors->Color[j][i].Flags|=FCF_FG_4BIT : DestColors->Color[j][i].Flags&=~FCF_FG_4BIT;
			}
			if (IS_OPAQUE(SrcColors->Color[j][i].BackgroundColor))
			{
				DestColors->Color[j][i].BackgroundColor=SrcColors->Color[j][i].BackgroundColor;
				SrcColors->Color[j][i].Flags&FCF_BG_4BIT? DestColors->Color[j][i].Flags|=FCF_BG_4BIT : DestColors->Color[j][i].Flags&=~FCF_BG_4BIT;
			}
			DestColors->Color[j][i].Flags |= SrcColors->Color[j][i].Flags&FCF_EXTENDEDFLAGS;
		}
	}

	//Унаследуем пометку из Src если она не прозрачная
	if (!(SrcColors->MarkChar&0x00FF0000))
		DestColors->MarkChar=SrcColors->MarkChar;
}

static void ApplyFinalColors(HighlightDataColor *Colors)
{
	//Обработаем black on black чтоб после наследования были правильные цвета.
	ApplyBlackOnBlackColors(Colors);

	for (int j=0; j<2; j++)
	{
		for (int i=0; i<4; i++)
		{
			//Если какой то из текущих цветов (fore или back) прозрачный
			//то унаследуем соответствующий цвет с панелей.
			if(IS_TRANSPARENT(Colors->Color[j][i].BackgroundColor))
			{
				Colors->Color[j][i].BackgroundColor=Global->Opt->Palette[PalColor[i]-COL_FIRSTPALETTECOLOR].BackgroundColor;
				if(Global->Opt->Palette[PalColor[i]-COL_FIRSTPALETTECOLOR].Flags&FCF_BG_4BIT)
				{
					Colors->Color[j][i].Flags|=FCF_BG_4BIT;
				}
				else
				{
					Colors->Color[j][i].Flags&=~FCF_BG_4BIT;
				}
			}
			if(IS_TRANSPARENT(Colors->Color[j][i].ForegroundColor))
			{
				Colors->Color[j][i].ForegroundColor=Global->Opt->Palette[PalColor[i]-COL_FIRSTPALETTECOLOR].ForegroundColor;
				if(Global->Opt->Palette[PalColor[i]-COL_FIRSTPALETTECOLOR].Flags&FCF_FG_4BIT)
				{
					Colors->Color[j][i].Flags|=FCF_FG_4BIT;
				}
				else
				{
					Colors->Color[j][i].Flags&=~FCF_FG_4BIT;
				}
			}
		}
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
	ULARGE_INTEGER current = {cft.dwLowDateTime, cft.dwHighDateTime};
	CurrentTime = current.QuadPart;
}

void HighlightFiles::GetHiColor(FileListItem* To, bool UseAttrHighlighting)
{
	ApplyDefaultStartingColors(&To->Colors);

	FOR_CONST_RANGE(HiData, i)
	{
		if (UseAttrHighlighting && (*i)->GetMask(nullptr))
			continue;

		if ((*i)->FileInFilter(To, CurrentTime))
		{
			HighlightDataColor TempColors;
			(*i)->GetColors(&TempColors);
			ApplyColors(&To->Colors,&TempColors);

			if (!(*i)->GetContinueProcessing())
				break;
		}
	}
	ApplyFinalColors(&To->Colors);
}

int HighlightFiles::GetGroup(const FileListItem *fli)
{
	for (int i=FirstCount; i<FirstCount+UpperCount; i++)
	{
		if (HiData[i]->FileInFilter(fli, CurrentTime))
			return(HiData[i]->GetSortGroup());
	}

	for (int i=FirstCount+UpperCount; i<FirstCount+UpperCount+LowerCount; i++)
	{
		if (HiData[i]->FileInFilter(fli, CurrentTime))
			return(HiData[i]->GetSortGroup());
	}

	return DEFAULT_SORT_GROUP;
}

void HighlightFiles::FillMenu(VMenu2 *HiMenu,int MenuPos)
{
	const int Count[4][2] =
	{
		{0,                               FirstCount},
		{FirstCount,                      FirstCount+UpperCount},
		{FirstCount+UpperCount,           FirstCount+UpperCount+LowerCount},
		{FirstCount+UpperCount+LowerCount,FirstCount+UpperCount+LowerCount+LastCount}
	};
	HiMenu->DeleteItems();

	for (int j=0; j<4; j++)
	{
		for (int i=Count[j][0]; i<Count[j][1]; i++)
		{
			MenuItemEx HiMenuItem;
			MenuString(HiMenuItem.strName, HiData[i].get(), true);
			HiMenu->AddItem(HiMenuItem);
		}

		MenuItemEx Empty;
		HiMenu->AddItem(Empty);

		if (j<3)
		{
			MenuItemEx HiMenuItem;
			if (!j)
				HiMenuItem.strName = MSG(MHighlightUpperSortGroup);
			else if (j==1)
				HiMenuItem.strName = MSG(MHighlightLowerSortGroup);
			else
				HiMenuItem.strName = MSG(MHighlightLastGroup);

			HiMenuItem.Flags|=LIF_SEPARATOR;
			HiMenu->AddItem(HiMenuItem);
		}
	}

	HiMenu->SetSelectPos(MenuPos,1);
}

void HighlightFiles::ProcessGroups()
{
	for (int i=0; i<FirstCount; i++)
		HiData[i]->SetSortGroup(DEFAULT_SORT_GROUP);

	for (int i=FirstCount; i<FirstCount+UpperCount; i++)
		HiData[i]->SetSortGroup(i-FirstCount);

	for (int i=FirstCount+UpperCount; i<FirstCount+UpperCount+LowerCount; i++)
		HiData[i]->SetSortGroup(DEFAULT_SORT_GROUP+1+i-FirstCount-UpperCount);

	for (int i=FirstCount+UpperCount+LowerCount; i<FirstCount+UpperCount+LowerCount+LastCount; i++)
		HiData[i]->SetSortGroup(DEFAULT_SORT_GROUP);
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

void HighlightFiles::UpdateHighlighting(bool RefreshMasks)
{
	Global->ScrBuf->Lock(); // отменяем всякую прорисовку
	ProcessGroups();

	if(RefreshMasks)
		for (size_t i = 0; i < HiData.size(); ++i)
			HiData[i]->RefreshMask();

	//FrameManager->RefreshFrame(); // рефрешим
	Global->CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
	Global->CtrlObject->Cp()->LeftPanel->Redraw();
	Global->CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
	Global->CtrlObject->Cp()->RightPanel->Redraw();
	Global->ScrBuf->Unlock(); // разрешаем прорисовку
}

void HighlightFiles::HiEdit(int MenuPos)
{
	VMenu2 HiMenu(MSG(MHighlightTitle),nullptr,0,ScrY-4);
	HiMenu.SetHelp(HLS.HighlightList);
	HiMenu.SetFlags(VMENU_WRAPMODE|VMENU_SHOWAMPERSAND);
	HiMenu.SetPosition(-1,-1,0,0);
	HiMenu.SetBottomTitle(MSG(MHighlightBottom));
	FillMenu(&HiMenu,MenuPos);
	int NeedUpdate;

	while (1)
	{
		HiMenu.Run([&](int Key)->int
		{
			int SelectPos=HiMenu.GetSelectPos();
			NeedUpdate=FALSE;

			int KeyProcessed = 1;

			switch (Key)
			{
					/* $ 07.07.2000 IS
					  Если нажали ctrl+r, то восстановить значения по умолчанию.
					*/
				case KEY_CTRLR:
				case KEY_RCTRLR:
				{

					if (Message(MSG_WARNING,2,MSG(MHighlightTitle),
					            MSG(MHighlightWarning),MSG(MHighlightAskRestore),
					            MSG(MYes),MSG(MCancel)) != 0)
						break;

					auto cfg = Global->Db->CreateHighlightConfig();
					SetHighlighting(true, cfg.get()); //delete old settings
					ClearData();
					InitHighlightFiles(cfg.get());
					NeedUpdate=TRUE;
					break;
				}

				case KEY_NUMDEL:
				case KEY_DEL:
				{
					int *Count=nullptr;
					int RealSelectPos=MenuPosToRealPos(SelectPos,&Count);

					if (Count && RealSelectPos<(int)HiData.size())
					{
						const wchar_t *Mask;
						HiData[RealSelectPos]->GetMask(&Mask);

						if (Message(MSG_WARNING,2,MSG(MHighlightTitle),
						            MSG(MHighlightAskDel),Mask,
						            MSG(MDelete),MSG(MCancel)) != 0)
							break;
						HiData.erase(HiData.begin()+RealSelectPos);
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

					if (Count && RealSelectPos<(int)HiData.size())
						if (FileFilterConfig(HiData[RealSelectPos].get(),true))
							NeedUpdate=TRUE;

					break;
				}

				case KEY_INS:
				case KEY_NUMPAD0:
				case KEY_F5:
				{
					int *Count=nullptr;
					int RealSelectPos=MenuPosToRealPos(SelectPos,&Count,true);

					if (Count)
					{
						auto NewHData = std::make_unique<FileFilterParams>();

						if (Key == KEY_F5)
							*NewHData = *HiData[RealSelectPos];

						if (FileFilterConfig(NewHData.get(), true))
						{
							(*Count)++;
							NeedUpdate=TRUE;
							HiData.emplace(HiData.begin()+RealSelectPos, std::move(NewHData));
						}
					}

					break;
				}

				case KEY_CTRLUP:  case KEY_CTRLNUMPAD8:
				case KEY_RCTRLUP: case KEY_RCTRLNUMPAD8:
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
						{
							std::swap(HiData[RealSelectPos], HiData[RealSelectPos-1]);
						}
						HiMenu.SetCheck(--SelectPos);
						NeedUpdate=TRUE;
						break;
					}

					break;
				}

				case KEY_CTRLDOWN:  case KEY_CTRLNUMPAD2:
				case KEY_RCTRLDOWN: case KEY_RCTRLNUMPAD2:
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
						{
							std::swap(HiData[RealSelectPos], HiData[RealSelectPos+1]);
						}
						HiMenu.SetCheck(++SelectPos);
						NeedUpdate=TRUE;
					}

					break;
				}

				default:
					KeyProcessed = 0;
			}

			// повторяющийся кусок!
			if (NeedUpdate)
			{
				Global->ScrBuf->Lock(); // отменяем всякую прорисовку
				Changed = true;
				UpdateHighlighting();
				FillMenu(&HiMenu,MenuPos=SelectPos);

				if (Global->Opt->AutoSaveSetup)
					SaveHiData();
				Global->ScrBuf->Unlock(); // разрешаем прорисовку
			}
			return KeyProcessed;
		});

		if (HiMenu.GetExitCode()!=-1)
		{
			HiMenu.Key(KEY_F4);
			continue;
		}

		break;
	}
}

static void SaveFilter(HierarchicalConfig *cfg, unsigned __int64 key, FileFilterParams *CurHiData, bool bSortGroup)
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
	cfg->SetValue(key,HLS.DateAfter, &DateAfter, sizeof(DateAfter));
	cfg->SetValue(key,HLS.DateBefore, &DateBefore, sizeof(DateBefore));
	cfg->SetValue(key,HLS.DateRelative,bRelative?1:0);
	cfg->SetValue(key, HLS.UseSize, CurHiData->IsSizeUsed());
	cfg->SetValue(key, HLS.SizeAbove, CurHiData->GetSizeAbove());
	cfg->SetValue(key, HLS.SizeBelow, CurHiData->GetSizeBelow());
	DWORD HardLinksAbove, HardLinksBelow;
	cfg->SetValue(key,HLS.UseHardLinks,CurHiData->GetHardLinks(&HardLinksAbove, &HardLinksBelow)?1:0);
	cfg->SetValue(key,HLS.HardLinksAbove,HardLinksAbove);
	cfg->SetValue(key,HLS.HardLinksBelow,HardLinksBelow);
	DWORD AttrSet, AttrClear;
	cfg->SetValue(key,HLS.UseAttr,CurHiData->GetAttr(&AttrSet, &AttrClear)?1:0);
	cfg->SetValue(key,(bSortGroup?HLS.AttrSet:HLS.IncludeAttributes),AttrSet);
	cfg->SetValue(key,(bSortGroup?HLS.AttrClear:HLS.ExcludeAttributes),AttrClear);
	HighlightDataColor Colors;
	CurHiData->GetColors(&Colors);
	cfg->SetValue(key,HLS.NormalColor, &Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_NORMAL], sizeof(FarColor));
	cfg->SetValue(key,HLS.SelectedColor, &Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_SELECTED], sizeof(FarColor));
	cfg->SetValue(key,HLS.CursorColor, &Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_UNDERCURSOR], sizeof(FarColor));
	cfg->SetValue(key,HLS.SelectedCursorColor, &Colors.Color[HIGHLIGHTCOLORTYPE_FILE][HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR], sizeof(FarColor));
	cfg->SetValue(key,HLS.MarkCharNormalColor, &Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_NORMAL], sizeof(FarColor));
	cfg->SetValue(key,HLS.MarkCharSelectedColor, &Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_SELECTED], sizeof(FarColor));
	cfg->SetValue(key,HLS.MarkCharCursorColor, &Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_UNDERCURSOR], sizeof(FarColor));
	cfg->SetValue(key,HLS.MarkCharSelectedCursorColor, &Colors.Color[HIGHLIGHTCOLORTYPE_MARKCHAR][HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR], sizeof(FarColor));
	cfg->SetValue(key,HLS.MarkChar, Colors.MarkChar);
	cfg->SetValue(key,HLS.ContinueProcessing, CurHiData->GetContinueProcessing()?1:0);
}

void HighlightFiles::SaveHiData()
{
	if (!Changed)
		return;

	Changed = false;

	const wchar_t *KeyNames[4]={HighlightKeyName,SortGroupsKeyName,SortGroupsKeyName,HighlightKeyName};
	const wchar_t *GroupNames[4]={fmtFirstGroup,fmtUpperGroup,fmtLowerGroup,fmtLastGroup};
	const int Count[4][2] =
	{
		{0,                               FirstCount},
		{FirstCount,                      FirstCount+UpperCount},
		{FirstCount+UpperCount,           FirstCount+UpperCount+LowerCount},
		{FirstCount+UpperCount+LowerCount,FirstCount+UpperCount+LowerCount+LastCount}
	};

	auto cfg = Global->Db->CreateHighlightConfig();

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
			unsigned __int64 key = cfg->CreateKey(root, str_printf(GroupNames[j],i-Count[j][0]));
			if (!key)
				break;

			SaveFilter(cfg.get(), key, HiData[i].get(), j && j!=3);
		}
	}
}
