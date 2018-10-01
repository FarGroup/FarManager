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

#include "hilight.hpp"

#include "farcolor.hpp"
#include "keys.hpp"
#include "vmenu.hpp"
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
#include "lang.hpp"
#include "DlgGuid.hpp"
#include "elevation.hpp"
#include "filefilter.hpp"
#include "global.hpp"

#include "common/bytes_view.hpp"
#include "common/zip_view.hpp"

#include "format.hpp"

static const struct
{
	const string_view

	NormalColor,
	SelectedColor,
	CursorColor,
	SelectedCursorColor,
	MarkCharNormalColor,
	MarkCharSelectedColor,
	MarkCharCursorColor,
	MarkCharSelectedCursorColor,
	MarkChar,
	ContinueProcessing,
	HighlightEdit,
	HighlightList;
}
HLS
{
	L"NormalColor"sv,
	L"SelectedColor"sv,
	L"CursorColor"sv,
	L"SelectedCursorColor"sv,
	L"MarkCharNormalColor"sv,
	L"MarkCharSelectedColor"sv,
	L"MarkCharCursorColor"sv,
	L"MarkCharSelectedCursorColor"sv,
	L"MarkChar"sv,
	L"ContinueProcessing"sv,
	L"HighlightEdit"sv,
	L"HighlightList"sv
};

static const auto fmtFirstGroup = L"Group"sv;
static const auto fmtUpperGroup = L"UpperGroup"sv;
static const auto fmtLowerGroup = L"LowerGroup"sv;
static const auto fmtLastGroup = L"LastGroup"sv;
static const auto SortGroupsKeyName = L"SortGroups"sv;
static const auto HighlightKeyName = L"Highlight"sv;

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

	const auto MakeFarColor = [](int ConsoleColour)
	{
		auto Colour = colors::ConsoleColorToFarColor(ConsoleColour);
		colors::make_transparent(Colour.BackgroundColor);
		return Colour;
	};

	static const struct
	{
		string_view Mask;
		DWORD IncludeAttr;
		FarColor NormalColor;
		FarColor CursorColor;
	}
	DefaultHighlighting[]
	{
		{ {},          FILE_ATTRIBUTE_HIDDEN,    MakeFarColor(F_CYAN),         MakeFarColor(F_DARKGRAY) },
		{ {},          FILE_ATTRIBUTE_SYSTEM,    MakeFarColor(F_CYAN),         MakeFarColor(F_DARKGRAY) },
		{ {},          FILE_ATTRIBUTE_DIRECTORY, MakeFarColor(F_WHITE),        MakeFarColor(F_WHITE) },
		{ L"<exec>"sv, 0,                        MakeFarColor(F_LIGHTGREEN),   MakeFarColor(F_LIGHTGREEN) },
		{ L"<arc>"sv,  0,                        MakeFarColor(F_LIGHTMAGENTA), MakeFarColor(F_LIGHTMAGENTA) },
		{ L"<temp>"sv, 0,                        MakeFarColor(F_BROWN),        MakeFarColor(F_BROWN) },
	};

	size_t Index = 0;
	for (auto& i: DefaultHighlighting)
	{
		const auto Key = cfg->CreateKey(root, L"Group"sv + str(Index++));
		if (!Key)
			break;

		FileFilterParams Params;
		Params.SetMask(!i.Mask.empty(), i.Mask);
		Params.SetAttr(i.IncludeAttr != 0, i.IncludeAttr, 0);
		FileFilter::SaveFilter(cfg, Key.get(), Params);

		cfg->SetValue(Key, HLS.NormalColor, bytes_view(i.NormalColor));
		cfg->SetValue(Key, HLS.CursorColor, bytes_view(i.CursorColor));

		static const string_view Names[] =
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
			cfg->SetValue(Key, j, bytes_view(DefaultColor));
		}
	}
}

highlight::configuration::configuration()
{
	const auto cfg = ConfigProvider().CreateHighlightConfig();
	SetHighlighting(false, cfg.get());
	InitHighlightFiles(cfg.get());
	UpdateCurrentTime();
}

static void LoadFilter(const HierarchicalConfig* cfg, const HierarchicalConfig::key& key, FileFilterParams& HData, int SortGroup, bool bSortGroup)
{
	FileFilter::LoadFilter(cfg, key.get(), HData);

	HData.SetSortGroup(SortGroup);

	highlight::element Colors = {};
	cfg->GetValue(key,HLS.NormalColor, bytes::reference(Colors.Color[highlight::color::normal].FileColor));
	cfg->GetValue(key,HLS.SelectedColor, bytes::reference(Colors.Color[highlight::color::selected].FileColor));
	cfg->GetValue(key,HLS.CursorColor, bytes::reference(Colors.Color[highlight::color::normal_current].FileColor));
	cfg->GetValue(key,HLS.SelectedCursorColor, bytes::reference(Colors.Color[highlight::color::selected_current].FileColor));
	cfg->GetValue(key,HLS.MarkCharNormalColor, bytes::reference(Colors.Color[highlight::color::normal].MarkColor));
	cfg->GetValue(key,HLS.MarkCharSelectedColor, bytes::reference(Colors.Color[highlight::color::selected].MarkColor));
	cfg->GetValue(key,HLS.MarkCharCursorColor, bytes::reference(Colors.Color[highlight::color::normal_current].MarkColor));
	cfg->GetValue(key,HLS.MarkCharSelectedCursorColor, bytes::reference(Colors.Color[highlight::color::selected_current].MarkColor));

	unsigned long long MarkChar;
	if (cfg->GetValue(key, HLS.MarkChar, MarkChar))
	{
		Colors.Mark.Char = LOWORD(MarkChar);
		Colors.Mark.Transparent = LOBYTE(HIWORD(MarkChar)) == 0xff;
	}
	HData.SetColors(Colors);

	unsigned long long ContinueProcessing = 0;
	cfg->GetValue(key, HLS.ContinueProcessing, ContinueProcessing);
	HData.SetContinueProcessing(ContinueProcessing!=0);
}

void highlight::configuration::InitHighlightFiles(const HierarchicalConfig* cfg)
{
	const struct
	{
		int Delta;
		string_view KeyName;
		string_view GroupName;
		int* Count;
	}
	GroupItems[]
	{
		{ DEFAULT_SORT_GROUP,     HighlightKeyName,  fmtFirstGroup, &FirstCount },
		{ 0,                      SortGroupsKeyName, fmtUpperGroup, &UpperCount },
		{ DEFAULT_SORT_GROUP + 1, SortGroupsKeyName, fmtLowerGroup, &LowerCount },
		{ DEFAULT_SORT_GROUP,     HighlightKeyName,  fmtLastGroup,  &LastCount },
	};

	ClearData();
	FirstCount=UpperCount=LowerCount=LastCount=0;

	for(const auto& Item: GroupItems)
	{
		const auto root = cfg->FindByName(cfg->root_key(), Item.KeyName);
		if (!root)
			continue;

		for (int i=0;; ++i)
		{
			const auto key = cfg->FindByName(root, Item.GroupName + str(i));
			if (!key)
				break;

			FileFilterParams NewItem;
			LoadFilter(cfg, key, NewItem, Item.Delta + (Item.Delta == DEFAULT_SORT_GROUP? 0 : i), Item.Delta != DEFAULT_SORT_GROUP);
			HiData.emplace_back(std::move(NewItem));
			++*Item.Count;
		}
	}
}


void highlight::configuration::ClearData()
{
	HiData.clear();
	FirstCount=UpperCount=LowerCount=LastCount=0;
}

static const DWORD PalColor[] = {COL_PANELTEXT,COL_PANELSELECTEDTEXT,COL_PANELCURSOR,COL_PANELSELECTEDCURSOR};

static void ApplyDefaultStartingColors(highlight::element& Colors)
{
	std::for_each(RANGE(Colors.Color, i)
	{
		colors::make_opaque(i.FileColor.ForegroundColor);
		colors::make_transparent(i.FileColor.BackgroundColor);
		colors::make_opaque(i.MarkColor.ForegroundColor);
		colors::make_transparent(i.MarkColor.BackgroundColor);
	});

	Colors.Mark.Transparent = true;
	Colors.Mark.Char = 0;
}

static void ApplyBlackOnBlackColor(highlight::element::colors_array::value_type& Colors, DWORD PaletteColor)
{
	const auto& InheritColor = [](FarColor& Color, const FarColor& Base)
	{
		if (!colors::color_value(Color.ForegroundColor) && !colors::color_value(Color.BackgroundColor))
		{
			Color.BackgroundColor = colors::alpha_value(Color.BackgroundColor) | colors::color_value(Base.BackgroundColor);
			Color.ForegroundColor = colors::alpha_value(Color.ForegroundColor) | colors::color_value(Base.ForegroundColor);
			Color.Flags &= FCF_EXTENDEDFLAGS;
			Color.Flags |= Base.Flags;
			Color.Reserved = Base.Reserved;
		}
	};

	//Применим black on black.
	//Для файлов возьмем цвета панели не изменяя прозрачность.
	InheritColor(Colors.FileColor, Global->Opt->Palette[PaletteColor]);
	//Для пометки возьмем цвета файла включая прозрачность.
	InheritColor(Colors.MarkColor, Colors.FileColor);
}

static void ApplyBlackOnBlackColors(highlight::element::colors_array& Colors)
{
	for (const auto& i: zip(Colors, PalColor)) std::apply(ApplyBlackOnBlackColor, i);
}

static void ApplyColors(highlight::element& DestColors, const highlight::element& Src)
{
	//Обработаем black on black чтоб наследовать правильные цвета
	//и чтоб после наследования были правильные цвета.
	ApplyBlackOnBlackColors(DestColors.Color);
	auto SrcColors = Src;
	ApplyBlackOnBlackColors(SrcColors.Color);

	for (const auto& i: zip(SrcColors.Color, DestColors.Color))
	{
		const auto& SrcItem = std::get<0>(i);
		auto& DstItem = std::get<1>(i);

		DstItem.FileColor = colors::merge(DstItem.FileColor, SrcItem.FileColor);
		DstItem.MarkColor = colors::merge(DstItem.MarkColor, SrcItem.MarkColor);
	}

	//Унаследуем пометку из Src если она не прозрачная
	if (!SrcColors.Mark.Transparent)
	{
		DestColors.Mark.Transparent = false;
		DestColors.Mark.Char = SrcColors.Mark.Char;
	}
}

void highlight::configuration::ApplyFinalColor(highlight::element::colors_array::value_type& Colors, size_t PaletteIndex)
{
	const auto PaletteColor = PalColor[PaletteIndex];

	//Обработаем black on black чтоб после наследования были правильные цвета.
	ApplyBlackOnBlackColor(Colors, PaletteColor);

	const auto& PanelColor = Global->Opt->Palette[PaletteColor];

	//Если какой то из текущих цветов (fore или back) прозрачный
	//то унаследуем соответствующий цвет с панелей.
	const auto& ApplyColorPart = [&](FarColor& i, COLORREF FarColor::*ColorAccessor, const FARCOLORFLAGS Flag)
	{
		auto& ColorPart = std::invoke(ColorAccessor, i);
		if(colors::is_transparent(ColorPart))
		{
			ColorPart = colors::transparent(std::invoke(ColorAccessor, PanelColor));
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

void highlight::configuration::UpdateCurrentTime()
{
	CurrentTime = os::chrono::nt_clock::now();
}

const highlight::element* highlight::configuration::GetHiColor(const FileListItem& Item, const FileList* Owner, bool UseAttrHighlighting)
{
	SCOPED_ACTION(elevation::suppress);

	element item = {};

	ApplyDefaultStartingColors(item);

	for (const auto& i: HiData)
	{
		if (UseAttrHighlighting && i.IsMaskUsed())
			continue;

		if (!i.FileInFilter(&Item, Owner, CurrentTime))
			continue;

		ApplyColors(item, i.GetColors());

		if (!i.GetContinueProcessing())
			break;
	}

	// Called from FileList::GetShowColor dynamically instead
	//for (const auto& i: zip(Item.Color, PalColor)) std::apply(ApplyFinalColor, i);

	//Если символ пометки прозрачный то его как бы и нет вообще.
	if (item.Mark.Transparent)
		item.Mark.Char = 0;

	return &*Colors.emplace(item).first;
}

int highlight::configuration::GetGroup(const FileListItem *fli, const FileList* Owner)
{
	const auto Begin = HiData.cbegin() + FirstCount, End = Begin + UpperCount + LowerCount;
	const auto It = std::find_if(Begin, End, [&](const auto& i) { return i.FileInFilter(fli, Owner, CurrentTime); });
	return It != End? It->GetSortGroup() : DEFAULT_SORT_GROUP;
}

void highlight::configuration::FillMenu(VMenu2 *HiMenu,int MenuPos)
{
	HiMenu->clear();

	const struct
	{
		int from;
		int to;
		string_view next_title;
	}
	Data[]
	{
		{ 0,                                    FirstCount,                                       msg(lng::MHighlightUpperSortGroup) },
		{ FirstCount,                           FirstCount + UpperCount,                          msg(lng::MHighlightLowerSortGroup) },
		{ FirstCount + UpperCount,              FirstCount + UpperCount + LowerCount,             msg(lng::MHighlightLastGroup) },
		{ FirstCount + UpperCount + LowerCount, FirstCount + UpperCount + LowerCount + LastCount, {} }
	};

	std::for_each(CONST_RANGE(Data, i)
	{
		std::for_each(HiData.cbegin() + i.from, HiData.cbegin() + i.to, [&](const auto& Item)
		{
			HiMenu->AddItem(MenuString(&Item, true));
		});

		HiMenu->AddItem(MenuItemEx());

		if (!i.next_title.empty())
		{
			MenuItemEx HiMenuItem(i.next_title);
			HiMenuItem.Flags|=LIF_SEPARATOR;
			HiMenu->AddItem(HiMenuItem);
		}
	});

	HiMenu->SetSelectPos(MenuPos,1);
}

void highlight::configuration::ProcessGroups()
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

size_t highlight::configuration::element_hash::operator()(const element& item) const
{
	size_t Seed = 0;

	hash_combine(Seed, item.Mark.Char);
	hash_combine(Seed, item.Mark.Transparent);

	for (const auto& i: item.Color)
	{
		hash_combine(Seed, i.FileColor);
		hash_combine(Seed, i.MarkColor);
	}

	return Seed;
}

int highlight::configuration::MenuPosToRealPos(int MenuPos, int*& Count, bool Insert)
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

void highlight::configuration::UpdateHighlighting(bool RefreshMasks)
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

void highlight::configuration::HiEdit(int MenuPos)
{
	const auto HiMenu = VMenu2::create(msg(lng::MHighlightTitle), {}, ScrY - 4);
	HiMenu->SetHelp(HLS.HighlightList);
	HiMenu->SetMenuFlags(VMENU_WRAPMODE | VMENU_SHOWAMPERSAND);
	HiMenu->SetPosition({ -1, -1, 0, 0 });
	HiMenu->SetBottomTitle(msg(lng::MHighlightBottom));
	HiMenu->SetId(HighlightMenuId);
	FillMenu(HiMenu.get(), MenuPos);
	bool NeedUpdate = false;

	for (;;)
	{
		HiMenu->Run([&](const Manager::Key& RawKey)
		{
			const auto Key=RawKey();
			auto SelectPos = HiMenu->GetSelectPos();
			NeedUpdate = false;

			int KeyProcessed = 1;

			switch (Key)
			{
					/* $ 07.07.2000 IS
					  Если нажали ctrl+r, то восстановить значения по умолчанию.
					*/
				case KEY_CTRLR:
				case KEY_RCTRLR:
				{
					if (Message(MSG_WARNING,
						msg(lng::MHighlightTitle),
						{
							msg(lng::MHighlightWarning),
							msg(lng::MHighlightAskRestore)
						},
						{ lng::MYes, lng::MCancel }) != Message::first_button)
					{
						break;
					}

					const auto cfg = ConfigProvider().CreateHighlightConfig();
					SetHighlighting(true, cfg.get()); //delete old settings

					InitHighlightFiles(cfg.get());
					FillMenu(HiMenu.get(), SelectPos);

					NeedUpdate = true;
					break;
				}

				case KEY_NUMDEL:
				case KEY_DEL:
				{
					int *Count=nullptr;
					int RealSelectPos=MenuPosToRealPos(SelectPos, Count);

					if (Count && RealSelectPos<(int)HiData.size())
					{
						if (Message(MSG_WARNING,
							msg(lng::MHighlightTitle),
							{
								msg(lng::MHighlightAskDel),
								HiData[RealSelectPos].GetMask()
							},
							{ lng::MDelete, lng::MCancel }) != Message::first_button)
						{
							break;
						}
						HiData.erase(HiData.begin()+RealSelectPos);
						HiMenu->DeleteItem(SelectPos);
						(*Count)--;
						NeedUpdate = true;
					}

					break;
				}

				case KEY_NUMENTER:
				case KEY_ENTER:
				case KEY_F4:
				{
					int *Count=nullptr;
					int RealSelectPos=MenuPosToRealPos(SelectPos, Count);

					if (Count && RealSelectPos<(int)HiData.size() && FileFilterConfig(&HiData[RealSelectPos], true))
					{
						HiMenu->DeleteItem(SelectPos);
						HiMenu->AddItem(MenuItemEx(MenuString(&HiData[RealSelectPos], true)), SelectPos);
						HiMenu->SetSelectPos(SelectPos, 1);
						NeedUpdate = true;
					}

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
							const auto Iterator = HiData.emplace(HiData.begin()+RealSelectPos, std::move(NewHData));
							HiMenu->AddItem(MenuItemEx(MenuString(&*Iterator, true)), SelectPos);
							HiMenu->SetSelectPos(SelectPos, 1);
							NeedUpdate = true;
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
							swap(HiMenu->at(SelectPos), HiMenu->at(SelectPos - 1));
						}
						HiMenu->SetSelectPos(--SelectPos);
						NeedUpdate = true;
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
							swap(HiMenu->at(SelectPos), HiMenu->at(SelectPos + 1));
						}
						HiMenu->SetSelectPos(++SelectPos);
						NeedUpdate = true;
					}

					break;
				}

				default:
					KeyProcessed = 0;
			}

			// повторяющийся кусок!
			if (NeedUpdate)
			{
				m_Changed = true;
				UpdateHighlighting();

				if (Global->Opt->AutoSaveSetup)
					Save(false);
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

static void SaveFilter(HierarchicalConfig* const cfg, const HierarchicalConfig::key& key, const FileFilterParams* const CurHiData, bool const bSortGroup)
{
	FileFilter::SaveFilter(cfg, key.get(), *CurHiData);

	const auto Colors = CurHiData->GetColors();
	cfg->SetValue(key,HLS.NormalColor, bytes_view(Colors.Color[highlight::color::normal].FileColor));
	cfg->SetValue(key,HLS.SelectedColor, bytes_view(Colors.Color[highlight::color::selected].FileColor));
	cfg->SetValue(key,HLS.CursorColor, bytes_view(Colors.Color[highlight::color::normal_current].FileColor));
	cfg->SetValue(key,HLS.SelectedCursorColor, bytes_view(Colors.Color[highlight::color::selected_current].FileColor));
	cfg->SetValue(key,HLS.MarkCharNormalColor, bytes_view(Colors.Color[highlight::color::normal].MarkColor));
	cfg->SetValue(key,HLS.MarkCharSelectedColor, bytes_view(Colors.Color[highlight::color::selected].MarkColor));
	cfg->SetValue(key,HLS.MarkCharCursorColor, bytes_view(Colors.Color[highlight::color::normal_current].MarkColor));
	cfg->SetValue(key,HLS.MarkCharSelectedCursorColor, bytes_view(Colors.Color[highlight::color::selected_current].MarkColor));
	cfg->SetValue(key,HLS.MarkChar, MAKELONG(Colors.Mark.Char, MAKEWORD(Colors.Mark.Transparent? 0xff : 0, 0)));
	cfg->SetValue(key,HLS.ContinueProcessing, CurHiData->GetContinueProcessing()?1:0);
}

void highlight::configuration::Save(bool always)
{
	if (!always && !m_Changed)
		return;

	m_Changed = false;

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
		string_view KeyName;
		string_view GroupName;
		int from;
		int to;
	}
	Data[]
	{
		{ false, HighlightKeyName,  fmtFirstGroup, 0,                                    FirstCount },
		{ true,  SortGroupsKeyName, fmtUpperGroup, FirstCount,                           FirstCount + UpperCount },
		{ true,  SortGroupsKeyName, fmtLowerGroup, FirstCount + UpperCount,              FirstCount + UpperCount + LowerCount },
		{ false, HighlightKeyName,  fmtLastGroup,  FirstCount + UpperCount + LowerCount, FirstCount + UpperCount + LowerCount + LastCount },
	};

	for(const auto& i: Data)
	{
		root = cfg->CreateKey(cfg->root_key(), i.KeyName);
		if (!root)
			continue; // TODO: log

		for (int j = i.from; j != i.to; ++j)
		{
			if (const auto Key = cfg->CreateKey(root, i.GroupName + str(j - i.from)))
				SaveFilter(cfg.get(), Key, &HiData[j], i.IsSort);
			// TODO: log
		}
	}
}
