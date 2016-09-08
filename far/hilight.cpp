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

#include "farcolor.hpp"
#include "hilight.hpp"
#include "keys.hpp"
#include "vmenu2.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "filelist.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "message.hpp"
#include "config.hpp"
#include "interf.hpp"
#include "configdb.hpp"
#include "colormix.hpp"
#include "filefilterparams.hpp"
#include "language.hpp"
#include "datetime.hpp"
#include "DlgGuid.hpp"
#include "elevation.hpp"

static const struct
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
}
HLS =
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

static const wchar_t fmtFirstGroup[]=L"Group";
static const wchar_t fmtUpperGroup[]=L"UpperGroup";
static const wchar_t fmtLowerGroup[]=L"LowerGroup";
static const wchar_t fmtLastGroup[]=L"LastGroup";
static const wchar_t SortGroupsKeyName[]=L"SortGroups";
static const wchar_t HighlightKeyName[]=L"Highlight";

static void SetHighlighting(bool DeleteOld, HierarchicalConfig *cfg)
{
	if (DeleteOld)
	{
		if (const auto root = cfg->FindByName(cfg->root_key(), HighlightKeyName))
			cfg->DeleteKeyTree(root);
	}

	if (cfg->FindByName(cfg->root_key(), HighlightKeyName))
		return;

	const auto root = cfg->CreateKey(cfg->root_key(), HighlightKeyName);
	if (!root)
		return;

	static const wchar_t* const Masks[]=
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
		bool IgnoreMask;
		DWORD IncludeAttr;
		BYTE InitNC;
		BYTE InitCC;
		FarColor NormalColor;
		FarColor CursorColor;
	}
	StdHighlightData[]=
	{
		/* 0 */{Masks[0], false, FILE_ATTRIBUTE_HIDDEN, B_BLUE|F_CYAN, B_CYAN|F_DARKGRAY},
		/* 1 */{Masks[0], false, FILE_ATTRIBUTE_SYSTEM, B_BLUE|F_CYAN, B_CYAN|F_DARKGRAY},
		/* 2 */{Masks[3], false, FILE_ATTRIBUTE_DIRECTORY, B_BLUE|F_WHITE, B_CYAN|F_WHITE},
		/* 3 */{Masks[4], false, FILE_ATTRIBUTE_DIRECTORY, 0, 0},
		/* 4 */{Masks[5], false, 0, B_BLUE|F_LIGHTGREEN, B_CYAN|F_LIGHTGREEN},
		/* 5 */{Masks[1], false, 0, B_BLUE|F_LIGHTMAGENTA, B_CYAN|F_LIGHTMAGENTA},
		/* 6 */{Masks[2], false, 0, B_BLUE|F_BROWN, B_CYAN|F_BROWN},
		// это настройка для каталогов на тех панелях, которые должны раскрашиваться
		// без учета масок (например, список хостов в "far navigator")
		/* 7 */{Masks[0], true, FILE_ATTRIBUTE_DIRECTORY, B_BLUE|F_WHITE, B_CYAN|F_WHITE},
	};

	size_t Index = 0;
	for (auto& i: StdHighlightData)
	{
		i.NormalColor = colors::ConsoleColorToFarColor(i.InitNC);
		MAKE_TRANSPARENT(i.NormalColor.BackgroundColor);
		i.CursorColor = colors::ConsoleColorToFarColor(i.InitCC);
		MAKE_TRANSPARENT(i.CursorColor.BackgroundColor);

		const auto Key = cfg->CreateKey(root, L"Group" + std::to_wstring(Index++));
		if (!Key)
			break;
		cfg->SetValue(Key, HLS.Mask, i.Mask);
		cfg->SetValue(Key, HLS.IgnoreMask, i.IgnoreMask);
		cfg->SetValue(Key, HLS.IncludeAttributes, i.IncludeAttr);

		cfg->SetValue(Key, HLS.NormalColor, i.NormalColor);
		cfg->SetValue(Key, HLS.CursorColor, i.CursorColor);

		static const wchar_t* const Names[] =
		{
			HLS.SelectedColor,
			HLS.SelectedCursorColor,
			HLS.MarkCharNormalColor,
			HLS.MarkCharSelectedColor,
			HLS.MarkCharCursorColor,
			HLS.MarkCharSelectedCursorColor,
		};

		for (const auto& j: Names)
		{
			static const FarColor DefaultColor = {FCF_FG_4BIT | FCF_BG_4BIT, 0xff000000, 0x00000000};
			cfg->SetValue(Key, j, DefaultColor);
		}
	}
}

HighlightFiles::HighlightFiles()
{
	Changed = false;
	const auto cfg = ConfigProvider().CreateHighlightConfig();
	SetHighlighting(false, cfg.get());
	InitHighlightFiles(cfg.get());
	UpdateCurrentTime();
}

static void LoadFilter(HierarchicalConfig *cfg, const HierarchicalConfig::key& key, FileFilterParams& HData, const string& Mask, int SortGroup, bool bSortGroup)
{
	//Дефолтные значения выбраны так чтоб как можно правильней загрузить
	//настройки старых версий фара.
	if (bSortGroup)
	{
		unsigned __int64 UseMask = 1;
		cfg->GetValue(key, HLS.UseMask, UseMask);
		HData.SetMask(UseMask!=0, Mask);
	}
	else
	{
		unsigned __int64 IgnoreMask = 0;
		cfg->GetValue(key, HLS.IgnoreMask, IgnoreMask);
		HData.SetMask(IgnoreMask==0, Mask);
	}

	FILETIME DateAfter = {};
	cfg->GetValue(key,HLS.DateAfter, DateAfter);
	FILETIME DateBefore = {};
	cfg->GetValue(key,HLS.DateBefore, DateBefore);
	unsigned __int64 UseDate = 0;
	cfg->GetValue(key, HLS.UseDate, UseDate);
	unsigned __int64 DateType = 0;
	cfg->GetValue(key, HLS.DateType, DateType);
	unsigned __int64 DateRelative = 0;
	cfg->GetValue(key, HLS.DateRelative, DateRelative);
	HData.SetDate(UseDate!=0, static_cast<enumFDateType>(DateType), DateAfter, DateBefore, DateRelative!=0);

	string strSizeAbove;
	cfg->GetValue(key,HLS.SizeAbove,strSizeAbove);
	string strSizeBelow;
	cfg->GetValue(key,HLS.SizeBelow,strSizeBelow);
	unsigned __int64 UseSize = 0;
	cfg->GetValue(key, HLS.UseSize, UseSize);
	HData.SetSize(UseSize!=0, strSizeAbove, strSizeBelow);

	unsigned __int64 UseHardLinks = 0;
	cfg->GetValue(key, HLS.UseHardLinks, UseHardLinks);
	unsigned __int64 HardLinksAbove = 0;
	cfg->GetValue(key, HLS.HardLinksAbove, HardLinksAbove);
	unsigned __int64 HardLinksBelow = 0;
	cfg->GetValue(key, HLS.HardLinksBelow, HardLinksBelow);
	HData.SetHardLinks(UseHardLinks!=0,HardLinksAbove,HardLinksBelow);

	if (bSortGroup)
	{
		unsigned __int64 UseAttr = 1;
		cfg->GetValue(key, HLS.UseAttr, UseAttr);
		unsigned __int64 AttrSet = 0;
		cfg->GetValue(key, HLS.AttrSet, AttrSet);
		unsigned __int64 AttrClear = FILE_ATTRIBUTE_DIRECTORY;
		cfg->GetValue(key, HLS.AttrClear, AttrClear);
		HData.SetAttr(UseAttr!=0, (DWORD)AttrSet, (DWORD)AttrClear);
	}
	else
	{
		unsigned __int64 UseAttr = 1;
		cfg->GetValue(key, HLS.UseAttr, UseAttr);
		unsigned __int64 IncludeAttributes = 0;
		cfg->GetValue(key, HLS.IncludeAttributes, IncludeAttributes);
		unsigned __int64 ExcludeAttributes = 0;
		cfg->GetValue(key, HLS.ExcludeAttributes, ExcludeAttributes);
		HData.SetAttr(UseAttr!=0, (DWORD)IncludeAttributes, (DWORD)ExcludeAttributes);
	}

	HData.SetSortGroup(SortGroup);

	HighlightFiles::highlight_item Colors = {};
	cfg->GetValue(key,HLS.NormalColor, Colors.Color[HighlightFiles::NORMAL_COLOR].FileColor);
	cfg->GetValue(key,HLS.SelectedColor, Colors.Color[HighlightFiles::SELECTED_COLOR].FileColor);
	cfg->GetValue(key,HLS.CursorColor, Colors.Color[HighlightFiles::UNDERCURSOR_COLOR].FileColor);
	cfg->GetValue(key,HLS.SelectedCursorColor, Colors.Color[HighlightFiles::SELECTEDUNDERCURSOR_COLOR].FileColor);
	cfg->GetValue(key,HLS.MarkCharNormalColor, Colors.Color[HighlightFiles::NORMAL_COLOR].MarkColor);
	cfg->GetValue(key,HLS.MarkCharSelectedColor, Colors.Color[HighlightFiles::SELECTED_COLOR].MarkColor);
	cfg->GetValue(key,HLS.MarkCharCursorColor, Colors.Color[HighlightFiles::UNDERCURSOR_COLOR].MarkColor);
	cfg->GetValue(key,HLS.MarkCharSelectedCursorColor, Colors.Color[HighlightFiles::SELECTEDUNDERCURSOR_COLOR].MarkColor);

	unsigned __int64 MarkChar;
	if (cfg->GetValue(key, HLS.MarkChar, MarkChar))
	{
		Colors.Mark.Char = LOWORD(MarkChar);
		Colors.Mark.Transparent = LOBYTE(HIWORD(MarkChar)) == 0xff;
	}
	HData.SetColors(Colors);

	unsigned __int64 ContinueProcessing = 0;
	cfg->GetValue(key, HLS.ContinueProcessing, ContinueProcessing);
	HData.SetContinueProcessing(ContinueProcessing!=0);
}

void HighlightFiles::InitHighlightFiles(HierarchicalConfig* cfg)
{
	static const struct
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

	for(const auto& Item: GroupItems)
	{
		const auto root = cfg->FindByName(cfg->root_key(), Item.KeyName);
		if (!root)
			continue;

		for (int i=0;; ++i)
		{
			const auto key = cfg->FindByName(root, Item.GroupName + std::to_wstring(i));
			if (!key)
				break;

			string strMask;
			if (!cfg->GetValue(key,HLS.Mask,strMask))
				break;

			FileFilterParams NewItem;
			LoadFilter(cfg, key, NewItem, strMask, Item.Delta + (Item.Delta == DEFAULT_SORT_GROUP? 0 : i), Item.Delta != DEFAULT_SORT_GROUP);
			HiData.emplace_back(std::move(NewItem));
			++*Item.Count;
		}
	}
}


void HighlightFiles::ClearData()
{
	HiData.clear();
	FirstCount=UpperCount=LowerCount=LastCount=0;
}

static const DWORD PalColor[] = {COL_PANELTEXT,COL_PANELSELECTEDTEXT,COL_PANELCURSOR,COL_PANELSELECTEDCURSOR};

static void ApplyDefaultStartingColors(HighlightFiles::highlight_item& Colors)
{
	std::for_each(RANGE(Colors.Color, i)
	{
		MAKE_OPAQUE(i.FileColor.ForegroundColor);
		MAKE_TRANSPARENT(i.FileColor.BackgroundColor);
		MAKE_OPAQUE(i.MarkColor.ForegroundColor);
		MAKE_TRANSPARENT(i.MarkColor.BackgroundColor);
	});

	Colors.Mark.Transparent = true;
	Colors.Mark.Char = 0;
}

static void ApplyBlackOnBlackColor(HighlightFiles::highlight_item::colors_array::value_type& Color, DWORD PaletteColor)
{
	const auto InheritColor = [](FarColor& Color, const FarColor& Base)
	{
		if (!COLORVALUE(Color.ForegroundColor) && !COLORVALUE(Color.BackgroundColor))
		{
			Color.BackgroundColor=ALPHAVALUE(Color.BackgroundColor)|COLORVALUE(Base.BackgroundColor);
			Color.ForegroundColor=ALPHAVALUE(Color.ForegroundColor)|COLORVALUE(Base.ForegroundColor);
			Color.Flags &= FCF_EXTENDEDFLAGS;
			Color.Flags |= Base.Flags;
			Color.Reserved = Base.Reserved;
		}
	};

	//Применим black on black.
	//Для файлов возьмем цвета панели не изменяя прозрачность.
	InheritColor(Color.FileColor, Global->Opt->Palette[PaletteColor]);
	//Для пометки возьмем цвета файла включая прозрачность.
	InheritColor(Color.MarkColor, Color.FileColor);
}

static void ApplyBlackOnBlackColors(HighlightFiles::highlight_item::colors_array& Colors)
{
	for (auto i: zip(Colors, PalColor)) std::apply(ApplyBlackOnBlackColor, i);
}

static void ApplyColors(HighlightFiles::highlight_item& DestColors, const HighlightFiles::highlight_item& Src)
{
	//Обработаем black on black чтоб наследовать правильные цвета
	//и чтоб после наследования были правильные цвета.
	ApplyBlackOnBlackColors(DestColors.Color);
	auto SrcColors = Src;
	ApplyBlackOnBlackColors(SrcColors.Color);

	const auto ApplyColor = [](FarColor& Dst, const FarColor& Src)
	{
		//Если текущие цвета в Src (fore и/или back) не прозрачные
		//то унаследуем их в Dest.

		const auto ApplyColorPart = [&](COLORREF FarColor::*Color, const FARCOLORFLAGS Flag)
		{
			if(IS_OPAQUE(Src.*Color))
			{
				Dst.*Color = Src.*Color;
				(Src.Flags & Flag)? (Dst.Flags |= Flag) : (Dst.Flags &= ~Flag);
			}
		};

		ApplyColorPart(&FarColor::BackgroundColor, FCF_BG_4BIT);
		ApplyColorPart(&FarColor::ForegroundColor, FCF_FG_4BIT);

		Dst.Flags |= Src.Flags&FCF_EXTENDEDFLAGS;
	};

	for (auto i: zip(SrcColors.Color, DestColors.Color))
	{
		const auto& SrcItem = std::get<0>(i);
		auto& DstItem = std::get<1>(i);

		ApplyColor(DstItem.FileColor, SrcItem.FileColor);
		ApplyColor(DstItem.MarkColor, SrcItem.MarkColor);
	}

	//Унаследуем пометку из Src если она не прозрачная
	if (!SrcColors.Mark.Transparent)
	{
		DestColors.Mark.Transparent = false;
		DestColors.Mark.Char = SrcColors.Mark.Char;
	}
}

void HighlightFiles::ApplyFinalColor(highlight_item::colors_array::value_type& Colors, size_t PaletteIndex)
{
	const auto PaletteColor = PalColor[PaletteIndex];

	//Обработаем black on black чтоб после наследования были правильные цвета.
	ApplyBlackOnBlackColor(Colors, PaletteColor);

	const auto& PanelColor = Global->Opt->Palette[PaletteColor];

	//Если какой то из текущих цветов (fore или back) прозрачный
	//то унаследуем соответствующий цвет с панелей.
	const auto ApplyColorPart = [&](FarColor& i, COLORREF FarColor::*Color, const FARCOLORFLAGS Flag)
	{
		if(IS_TRANSPARENT(i.*Color))
		{
			i.*Color = PanelColor.*Color;
			MAKE_TRANSPARENT(i.*Color);
			(PanelColor.Flags & Flag)? (i.Flags |= Flag) : (i.Flags &= ~Flag);
		}
	};

	ApplyColorPart(Colors.FileColor, &FarColor::BackgroundColor, FCF_BG_4BIT);
	ApplyColorPart(Colors.FileColor, &FarColor::ForegroundColor, FCF_FG_4BIT);

	ApplyColorPart(Colors.MarkColor, &FarColor::BackgroundColor, FCF_BG_4BIT);
	ApplyColorPart(Colors.MarkColor, &FarColor::ForegroundColor, FCF_FG_4BIT);

	//Паранойя но случится может:
	//Обработаем black on black снова чтоб обработались унаследованные цвета.
	ApplyBlackOnBlackColor(Colors, PaletteColor);
}

void HighlightFiles::UpdateCurrentTime()
{
	CurrentTime = GetCurrentUTCTimeInUI64();
}

const HighlightFiles::highlight_item* HighlightFiles::GetHiColor(const FileListItem& Item, const FileList* Owner, bool UseAttrHighlighting)
{
	SCOPED_ACTION(elevation::suppress);

	highlight_item item = {};

	ApplyDefaultStartingColors(item);

	std::any_of(CONST_RANGE(HiData, i)
	{
		if (!(UseAttrHighlighting && i.IsMaskUsed()))
		{
			if (i.FileInFilter(&Item, Owner, CurrentTime))
			{
				ApplyColors(item, i.GetColors());
				if (!i.GetContinueProcessing())
					return true;
			}
		}
		return false;
	});

	// Called from FileList::GetShowColor dynamically instead
	//for (auto i: zip(Item.Color, PalColor)) std::apply(ApplyFinalColor, i);

	//Если символ пометки прозрачный то его как бы и нет вообще.
	if (item.Mark.Transparent)
		item.Mark.Char = 0;

	return &*Colors.emplace(item).first;
}

int HighlightFiles::GetGroup(const FileListItem *fli, const FileList* Owner)
{
	const auto Begin = HiData.cbegin() + FirstCount, End = Begin + UpperCount + LowerCount;
	const auto It = std::find_if(Begin, End, [&](const auto& i) { return i.FileInFilter(fli, Owner, CurrentTime); });
	return It != End? It->GetSortGroup() : DEFAULT_SORT_GROUP;
}

void HighlightFiles::FillMenu(VMenu2 *HiMenu,int MenuPos)
{
	HiMenu->clear();

	const struct
	{
		int from;
		int to;
		const wchar_t* next_title;
	}
	Data[] =
	{
		{0, FirstCount, MSG(MHighlightUpperSortGroup)},
		{FirstCount, FirstCount+UpperCount, MSG(MHighlightLowerSortGroup)},
		{FirstCount+UpperCount, FirstCount+UpperCount+LowerCount, MSG(MHighlightLastGroup)},
		{FirstCount+UpperCount+LowerCount,FirstCount+UpperCount+LowerCount+LastCount, nullptr}
	};

	std::for_each(CONST_RANGE(Data, i)
	{
		std::for_each(HiData.cbegin() + i.from, HiData.cbegin() + i.to, [&](const auto& Item)
		{
			HiMenu->AddItem(MenuString(&Item, true));
		});

		HiMenu->AddItem(MenuItemEx());

		if (i.next_title)
		{
			MenuItemEx HiMenuItem(i.next_title);
			HiMenuItem.Flags|=LIF_SEPARATOR;
			HiMenu->AddItem(HiMenuItem);
		}
	});

	HiMenu->SetSelectPos(MenuPos,1);
}

void HighlightFiles::ProcessGroups()
{
	for (int i = 0; i<FirstCount; i++)
		HiData[i].SetSortGroup(DEFAULT_SORT_GROUP);

	for (int i=FirstCount; i<FirstCount+UpperCount; i++)
		HiData[i].SetSortGroup(i-FirstCount);

	for (int i=FirstCount+UpperCount; i<FirstCount+UpperCount+LowerCount; i++)
		HiData[i].SetSortGroup(DEFAULT_SORT_GROUP+1+i-FirstCount-UpperCount);

	for (int i=FirstCount+UpperCount+LowerCount; i<FirstCount+UpperCount+LowerCount+LastCount; i++)
		HiData[i].SetSortGroup(DEFAULT_SORT_GROUP);
}

int HighlightFiles::MenuPosToRealPos(int MenuPos, int*& Count, bool Insert)
{
	int Pos=MenuPos;
	Count = nullptr;
	int x = Insert ? 1 : 0;

	if (MenuPos<FirstCount+x)
	{
		Count = &FirstCount;
	}
	else if (MenuPos>FirstCount+1 && MenuPos<FirstCount+UpperCount+2+x)
	{
		Pos=MenuPos-2;
		Count = &UpperCount;
	}
	else if (MenuPos>FirstCount+UpperCount+3 && MenuPos<FirstCount+UpperCount+LowerCount+4+x)
	{
		Pos=MenuPos-4;
		Count = &LowerCount;
	}
	else if (MenuPos>FirstCount+UpperCount+LowerCount+5 && MenuPos<FirstCount+UpperCount+LowerCount+LastCount+6+x)
	{
		Pos=MenuPos-6;
		Count = &LastCount;
	}

	return Pos;
}

void HighlightFiles::UpdateHighlighting(bool RefreshMasks)
{
	Global->ScrBuf->Lock(); // отменяем всякую прорисовку
	ProcessGroups();

	if(RefreshMasks)
		std::for_each(ALL_RANGE(HiData), std::mem_fn(&FileFilterParams::RefreshMask));

	//WindowManager->RefreshWindow(); // рефрешим
	Global->CtrlObject->Cp()->LeftPanel()->Update(UPDATE_KEEP_SELECTION);
	Global->CtrlObject->Cp()->LeftPanel()->Redraw();
	Global->CtrlObject->Cp()->RightPanel()->Update(UPDATE_KEEP_SELECTION);
	Global->CtrlObject->Cp()->RightPanel()->Redraw();
	Global->ScrBuf->Unlock(); // разрешаем прорисовку
}

void HighlightFiles::HiEdit(int MenuPos)
{
	const auto HiMenu = VMenu2::create(MSG(MHighlightTitle), nullptr, 0, ScrY - 4);
	HiMenu->SetHelp(HLS.HighlightList);
	HiMenu->SetMenuFlags(VMENU_WRAPMODE | VMENU_SHOWAMPERSAND);
	HiMenu->SetPosition(-1,-1,0,0);
	HiMenu->SetBottomTitle(MSG(MHighlightBottom));
	HiMenu->SetId(HighlightMenuId);
	FillMenu(HiMenu.get(), MenuPos);
	int NeedUpdate;

	for (;;)
	{
		HiMenu->Run([&](const Manager::Key& RawKey)
		{
			const auto Key=RawKey();
			int SelectPos=HiMenu->GetSelectPos();
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
					            MSG(MYes),MSG(MCancel)) != Message::first_button)
						break;

					const auto cfg = ConfigProvider().CreateHighlightConfig();
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
					int RealSelectPos=MenuPosToRealPos(SelectPos, Count);

					if (Count && RealSelectPos<(int)HiData.size())
					{
						if (Message(MSG_WARNING,2,MSG(MHighlightTitle),
						            MSG(MHighlightAskDel), HiData[RealSelectPos].GetMask().data(),
						            MSG(MDelete),MSG(MCancel)) != Message::first_button)
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
					int RealSelectPos=MenuPosToRealPos(SelectPos, Count);

					if (Count && RealSelectPos<(int)HiData.size())
						if (FileFilterConfig(&HiData[RealSelectPos], true))
							NeedUpdate=TRUE;

					break;
				}

				case KEY_INS:
				case KEY_NUMPAD0:
				case KEY_F5:
				{
					int *Count=nullptr;
					int RealSelectPos=MenuPosToRealPos(SelectPos, Count,true);

					if (Count)
					{
						FileFilterParams NewHData;

						if (Key == KEY_F5)
							NewHData = HiData[RealSelectPos].Clone();

						if (FileFilterConfig(&NewHData, true))
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
					int RealSelectPos=MenuPosToRealPos(SelectPos, Count);

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
							using std::swap;
							swap(HiData[RealSelectPos], HiData[RealSelectPos-1]);
						}
						HiMenu->SetCheck(--SelectPos);
						NeedUpdate=TRUE;
						break;
					}

					break;
				}

				case KEY_CTRLDOWN:  case KEY_CTRLNUMPAD2:
				case KEY_RCTRLDOWN: case KEY_RCTRLNUMPAD2:
				{
					int *Count=nullptr;
					int RealSelectPos=MenuPosToRealPos(SelectPos, Count);

					if (Count && SelectPos < static_cast<int>(HiMenu->size()-2))
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
							using std::swap;
							swap(HiData[RealSelectPos], HiData[RealSelectPos+1]);
						}
						HiMenu->SetCheck(++SelectPos);
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
				FillMenu(HiMenu.get(), MenuPos = SelectPos);

				if (Global->Opt->AutoSaveSetup)
					Save(false);
				Global->ScrBuf->Unlock(); // разрешаем прорисовку
			}
			return KeyProcessed;
		});

		if (HiMenu->GetExitCode()!=-1)
		{
			HiMenu->Key(KEY_F4);
			continue;
		}

		break;
	}
}

static void SaveFilter(HierarchicalConfig *cfg, const HierarchicalConfig::key& key, FileFilterParams *CurHiData, bool bSortGroup)
{
	if (bSortGroup)
	{
		cfg->SetValue(key,HLS.UseMask, CurHiData->IsMaskUsed());
		cfg->SetValue(key, HLS.Mask, CurHiData->GetMask());
	}
	else
	{
		cfg->SetValue(key,HLS.IgnoreMask, !CurHiData->IsMaskUsed());
		cfg->SetValue(key, HLS.Mask, CurHiData->GetMask());
	}

	DWORD DateType;
	FILETIME DateAfter, DateBefore;
	bool bRelative;
	cfg->SetValue(key,HLS.UseDate,CurHiData->GetDate(&DateType, &DateAfter, &DateBefore, &bRelative)?1:0);
	cfg->SetValue(key,HLS.DateType,DateType);
	cfg->SetValue(key,HLS.DateAfter, DateAfter);
	cfg->SetValue(key,HLS.DateBefore, DateBefore);
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
	const auto Colors = CurHiData->GetColors();
	cfg->SetValue(key,HLS.NormalColor, Colors.Color[HighlightFiles::NORMAL_COLOR].FileColor);
	cfg->SetValue(key,HLS.SelectedColor, Colors.Color[HighlightFiles::SELECTED_COLOR].FileColor);
	cfg->SetValue(key,HLS.CursorColor, Colors.Color[HighlightFiles::UNDERCURSOR_COLOR].FileColor);
	cfg->SetValue(key,HLS.SelectedCursorColor, Colors.Color[HighlightFiles::SELECTEDUNDERCURSOR_COLOR].FileColor);
	cfg->SetValue(key,HLS.MarkCharNormalColor, Colors.Color[HighlightFiles::NORMAL_COLOR].MarkColor);
	cfg->SetValue(key,HLS.MarkCharSelectedColor, Colors.Color[HighlightFiles::SELECTED_COLOR].MarkColor);
	cfg->SetValue(key,HLS.MarkCharCursorColor, Colors.Color[HighlightFiles::UNDERCURSOR_COLOR].MarkColor);
	cfg->SetValue(key,HLS.MarkCharSelectedCursorColor, Colors.Color[HighlightFiles::SELECTEDUNDERCURSOR_COLOR].MarkColor);
	cfg->SetValue(key,HLS.MarkChar, MAKELONG(Colors.Mark.Char, MAKEWORD(Colors.Mark.Transparent? 0xff : 0, 0)));
	cfg->SetValue(key,HLS.ContinueProcessing, CurHiData->GetContinueProcessing()?1:0);
}

void HighlightFiles::Save(bool always)
{
	if (!always && !Changed)
		return;

	Changed = false;

	const auto cfg = ConfigProvider().CreateHighlightConfig();
	auto root = cfg->FindByName(cfg->root_key(), HighlightKeyName);

	if (root)
		cfg->DeleteKeyTree(root);

	root = cfg->FindByName(cfg->root_key(), SortGroupsKeyName);

	if (root)
		cfg->DeleteKeyTree(root);

	const struct
	{
		bool IsSort;
		const wchar_t* KeyName;
		const wchar_t* GroupName;
		int from;
		int to;
	}
	Data[] =
	{
		{false, HighlightKeyName, fmtFirstGroup, 0, FirstCount},
		{true, SortGroupsKeyName, fmtUpperGroup, FirstCount, FirstCount+UpperCount},
		{true, SortGroupsKeyName, fmtLowerGroup, FirstCount + UpperCount, FirstCount + UpperCount + LowerCount},
		{false, HighlightKeyName, fmtLastGroup, FirstCount + UpperCount + LowerCount, FirstCount + UpperCount + LowerCount + LastCount},
	};

	for(const auto& i: Data)
	{
		root = cfg->CreateKey(cfg->root_key(), i.KeyName);
		if (!root)
			continue; // TODO: log

		for (int j = i.from; j != i.to; ++j)
		{
			if (const auto Key = cfg->CreateKey(root, i.GroupName + std::to_wstring(j - i.from)))
				SaveFilter(cfg.get(), Key, &HiData[j], i.IsSort);
			// TODO: log
		}
	}
}
