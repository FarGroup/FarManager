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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "hilight.hpp"

// Internal:
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
#include "uuids.far.dialogs.hpp"
#include "elevation.hpp"
#include "filefilter.hpp"
#include "lockscrn.hpp"
#include "global.hpp"
#include "keyboard.hpp"

// Platform:

// Common:
#include "common/2d/matrix.hpp"
#include "common/bytes_view.hpp"
#include "common/string_utils.hpp"
#include "common/view/enumerate.hpp"
#include "common/view/zip.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static const FarColor DefaultColor
{
	FCF_INDEXMASK | FCF_INHERIT_STYLE,
	{colors::transparent(C_BLACK)},
	{colors::transparent(C_BLACK)},
	{colors::transparent(C_BLACK)}
};

namespace names
{
#define STR_INIT(x) x = WIDE_SV_LITERAL(x)

	static const auto
		STR_INIT(NormalColor),
		STR_INIT(SelectedColor),
		STR_INIT(CursorColor),
		STR_INIT(SelectedCursorColor),
		STR_INIT(MarkCharNormalColor),
		STR_INIT(MarkCharSelectedColor),
		STR_INIT(MarkCharCursorColor),
		STR_INIT(MarkCharSelectedCursorColor),
		STR_INIT(MarkChar),
		STR_INIT(Mark),
		STR_INIT(ContinueProcessing),
		STR_INIT(Group),
		STR_INIT(UpperGroup),
		STR_INIT(LowerGroup),
		STR_INIT(LastGroup),
		STR_INIT(SortGroups),
		STR_INIT(Highlight);

#undef STR_INIT

	static auto file_color(size_t const Index)
	{
		static const std::array Names
		{
			NormalColor,
			SelectedColor,
			CursorColor,
			SelectedCursorColor,
		};

		static_assert(std::size(Names) == highlight::color::count);
		return Names[Index];
	}

	static auto mark_color(size_t const Index)
	{
		static const std::array Names
		{
			MarkCharNormalColor,
			MarkCharSelectedColor,
			MarkCharCursorColor,
			MarkCharSelectedCursorColor,
		};

		static_assert(std::size(Names) == highlight::color::count);
		return Names[Index];
	}
}

static void SetHighlighting(bool DeleteOld, HierarchicalConfig& cfg)
{
	SCOPED_ACTION(auto)(cfg.ScopedTransaction());

	if (DeleteOld)
	{
		if (const auto root = cfg.FindByName(cfg.root_key, names::Highlight))
			cfg.DeleteKeyTree(root);
	}

	if (cfg.FindByName(cfg.root_key, names::Highlight))
		return;

	const auto MakeFarColor = [](int ConsoleColour)
	{
		auto Colour = colors::NtColorToFarColor(ConsoleColour);
		colors::make_transparent(Colour.BackgroundColor);
		Colour.Flags |= FCF_INHERIT_STYLE;
		return Colour;
	};

	static const struct
	{
		string_view Mask;
		os::fs::attributes IncludeAttr;
		FarColor NormalColor;
		FarColor CursorColor;
	}
	DefaultHighlighting[]
	{
		{ {},          FILE_ATTRIBUTE_HIDDEN,    MakeFarColor(C_CYAN),         MakeFarColor(C_DARKGRAY) },
		{ {},          FILE_ATTRIBUTE_SYSTEM,    MakeFarColor(C_CYAN),         MakeFarColor(C_DARKGRAY) },
		{ {},          FILE_ATTRIBUTE_DIRECTORY, MakeFarColor(C_WHITE),        MakeFarColor(C_WHITE) },
		{ L"<exec>"sv, 0,                        MakeFarColor(C_LIGHTGREEN),   MakeFarColor(C_LIGHTGREEN) },
		{ L"<arc>"sv,  0,                        MakeFarColor(C_LIGHTMAGENTA), MakeFarColor(C_LIGHTMAGENTA) },
		{ L"<temp>"sv, 0,                        MakeFarColor(C_BROWN),        MakeFarColor(C_BROWN) },
	};

	const auto root = cfg.CreateKey(cfg.root_key, names::Highlight);

	for (const auto& [i, Index]: enumerate(DefaultHighlighting))
	{
		FileFilterParams Params;
		Params.SetMask(!i.Mask.empty(), i.Mask);
		Params.SetAttr(i.IncludeAttr != 0, i.IncludeAttr, 0);

		const auto Key = cfg.CreateKey(root, names::Group + str(Index));
		filters::SaveFilter(cfg, Key.get(), Params);

		cfg.SetValue(Key, names::NormalColor, view_bytes(i.NormalColor));
		cfg.SetValue(Key, names::CursorColor, view_bytes(i.CursorColor));

		static const std::array Default
		{
			names::SelectedColor,
			names::SelectedCursorColor,
			names::MarkCharNormalColor,
			names::MarkCharSelectedColor,
			names::MarkCharCursorColor,
			names::MarkCharSelectedCursorColor,
		};

		for (const auto& j: Default)
		{
			cfg.SetValue(Key, j, view_bytes(DefaultColor));
		}
	}
}

highlight::element::colors::colors():
	FileColor(DefaultColor),
	MarkColor(DefaultColor)
{
}

highlight::configuration::configuration()
{
	const auto cfg = ConfigProvider().CreateHighlightConfig();
	SetHighlighting(false, *cfg);
	Load(*cfg);
	UpdateCurrentTime();
}

static FileFilterParams LoadHighlight(/*const*/ HierarchicalConfig& cfg, const HierarchicalConfig::key& key, int SortGroup)
{
	auto Item = filters::LoadFilter(cfg, key.get());

	Item.SetSortGroup(SortGroup);

	highlight::element Colors{};

	for (const auto& [Color, Index]: enumerate(Colors.Color))
	{
		bytes Blob;
		cfg.GetValue(key, names::file_color(Index), Blob) && deserialise(Blob, Color.FileColor);
		cfg.GetValue(key, names::mark_color(Index), Blob) && deserialise(Blob, Color.MarkColor);
	}

	auto MarkRead = false;
	if (string Mark; cfg.GetValue(key, names::Mark, Mark))
	{
		Colors.Mark.Mark = std::move(Mark);
		MarkRead = true;
	}

	if (unsigned long long MarkChar; cfg.GetValue(key, names::MarkChar, MarkChar))
	{
		// "MarkChar" used to contain both the mark character and the inheritance flag.
		// "Mark" is the main storage of the mark now, but if it's not there, we try the old way:
		const auto Char = static_cast<wchar_t>(extract_integer<WORD, 0>(MarkChar));
		if (!MarkRead && Char)
			Colors.Mark.Mark = Char;

		Colors.Mark.Inherit = extract_integer<BYTE, 0>(extract_integer<WORD, 1>(MarkChar)) == 0xff;
	}
	Item.SetColors(Colors);

	const auto ContinueProcessing = cfg.GetValue<bool>(key, names::ContinueProcessing);
	Item.SetContinueProcessing(ContinueProcessing);

	return Item;
}

void highlight::configuration::Load(/*const*/ HierarchicalConfig& cfg)
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
		{ DEFAULT_SORT_GROUP,         names::Highlight,    names::Group,         &FirstCount },
		{ 0,                          names::SortGroups,   names::UpperGroup,    &UpperCount },
		{ DEFAULT_SORT_GROUP + 1,     names::SortGroups,   names::LowerGroup,    &LowerCount },
		{ DEFAULT_SORT_GROUP,         names::Highlight,    names::LastGroup,     &LastCount },
	};

	ClearData();

	for(const auto& Item: GroupItems)
	{
		const auto root = cfg.FindByName(cfg.root_key, Item.KeyName);
		if (!root)
			continue;

		for (const auto& Key: cfg.KeysEnumerator(root, Item.GroupName))
		{
			HiData.emplace_back(LoadHighlight(cfg, Key, Item.Delta + (Item.Delta == DEFAULT_SORT_GROUP? 0 : *Item.Count)));
			++*Item.Count;
		}
	}
}


void highlight::configuration::ClearData()
{
	HiData.clear();
	FirstCount=UpperCount=LowerCount=LastCount=0;
}

static void ApplyBlackOnBlackColor(highlight::element::colors_array::value_type& Colors, DWORD PaletteColor)
{
	const auto InheritColor = [](FarColor& Color, const FarColor& Base)
	{
		const auto LegacyUseDefaultPaletteColor =
			colors::is_opaque(Color.ForegroundColor) && !colors::color_value(Color.ForegroundColor) &&
			colors::is_opaque(Color.BackgroundColor) && !colors::color_value(Color.BackgroundColor);

		if (!LegacyUseDefaultPaletteColor)
			return;

		colors::make_transparent(Color.ForegroundColor);
		colors::make_transparent(Color.BackgroundColor);
		Color = colors::merge(Base, Color);
	};

	InheritColor(Colors.FileColor, Global->Opt->Palette[PaletteColor]);
	InheritColor(Colors.MarkColor, Colors.FileColor);
}

static const DWORD PalColor[]
{
	COL_PANELTEXT,
	COL_PANELSELECTEDTEXT,
	COL_PANELCURSOR,
	COL_PANELSELECTEDCURSOR
};

static void ApplyBlackOnBlackColors(highlight::element::colors_array& Colors)
{
	for (const auto& [Color, Index]: zip(Colors, PalColor))
		ApplyBlackOnBlackColor(Color, Index);
}

static void ApplyColors(highlight::element& DestColors, const highlight::element& Src)
{
	//Обработаем black on black чтоб наследовать правильные цвета
	//и чтоб после наследования были правильные цвета.
	ApplyBlackOnBlackColors(DestColors.Color);
	auto SrcColors = Src;
	ApplyBlackOnBlackColors(SrcColors.Color);

	for (const auto& [SrcItem, DstItem]: zip(SrcColors.Color, DestColors.Color))
	{
		DstItem.FileColor = colors::merge(DstItem.FileColor, SrcItem.FileColor);
		DstItem.MarkColor = colors::merge(DstItem.MarkColor, SrcItem.MarkColor);
	}

	//Унаследуем пометку из Src если она не прозрачная
	if (!SrcColors.Mark.Inherit)
	{
		DestColors.Mark.Inherit = false;
		DestColors.Mark.Mark = SrcColors.Mark.Mark;
	}
}

void highlight::configuration::ApplyFinalColor(element::colors_array::value_type& Colors, size_t PaletteIndex)
{
	const auto PaletteColor = PalColor[PaletteIndex];

	//Обработаем black on black чтоб после наследования были правильные цвета.
	ApplyBlackOnBlackColor(Colors, PaletteColor);

	Colors.FileColor = colors::merge(Global->Opt->Palette[PaletteColor], Colors.FileColor);
	Colors.MarkColor = colors::merge(Colors.FileColor, Colors.MarkColor);

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

	element item;

	for (const auto& i: HiData)
	{
		if (UseAttrHighlighting && i.IsMaskUsed())
			continue;

		if (!i.FileInFilter(Item, Owner, CurrentTime))
			continue;

		ApplyColors(item, i.GetColors());

		if (!i.GetContinueProcessing())
			break;
	}

	// Called from FileList::GetShowColor dynamically instead
	//for (const auto& i: zip(Item.Color, PalColor)) std::apply(ApplyFinalColor, i);

	//Если символ пометки прозрачный то его как бы и нет вообще.
	if (item.Mark.Inherit)
		item.Mark.Mark.clear();

	return std::to_address(m_Colors.emplace(item).first);
}

int highlight::configuration::GetGroup(const FileListItem& Object, const FileList* Owner)
{
	const auto Begin = HiData.cbegin() + FirstCount, End = Begin + UpperCount + LowerCount;
	const auto It = std::find_if(Begin, End, [&](const auto& i) { return i.FileInFilter(Object, Owner, CurrentTime); });
	return It != End? It->GetSortGroup() : DEFAULT_SORT_GROUP;
}

void highlight::configuration::FillMenu(VMenu2 *HiMenu,int MenuPos) const
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

	for (const auto& i: Data)
	{
		for (const auto& Item: std::ranges::subrange(HiData.cbegin() + i.from, HiData.cbegin() + i.to))
		{
			HiMenu->AddItem(MenuString(&Item, true));
		}

		HiMenu->AddItem(MenuItemEx());

		if (!i.next_title.empty())
		{
			MenuItemEx HiMenuItem(i.next_title);
			HiMenuItem.Flags|=LIF_SEPARATOR;
			HiMenu->AddItem(HiMenuItem);
		}
	}

	HiMenu->SetSelectPos(MenuPos,1);
}

void highlight::configuration::ProcessGroups()
{
	for (const auto i: std::views::iota(0, FirstCount))
		HiData[i].SetSortGroup(DEFAULT_SORT_GROUP);

	for (const auto i: std::views::iota(FirstCount, FirstCount + UpperCount))
		HiData[i].SetSortGroup(i-FirstCount);

	for (const auto i: std::views::iota(FirstCount + UpperCount, FirstCount + UpperCount + LowerCount))
		HiData[i].SetSortGroup(DEFAULT_SORT_GROUP+1+i-FirstCount-UpperCount);

	for (const auto i: std::views::iota(FirstCount + UpperCount + LowerCount, FirstCount + UpperCount + LowerCount + LastCount))
		HiData[i].SetSortGroup(DEFAULT_SORT_GROUP);
}

size_t highlight::configuration::element_hash::operator()(const element& item) const
{
	size_t Seed = 0;

	hash_combine(Seed, item.Mark.Mark);
	hash_combine(Seed, item.Mark.Inherit);

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
	const auto x = Insert? 1 : 0;

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
	SCOPED_ACTION(LockScreen);

	ProcessGroups();

	if (RefreshMasks)
	{
		for (auto& i: HiData)
		{
			i.RefreshMask();
		}
	}

	//WindowManager->RefreshWindow(); // рефрешим
	Global->CtrlObject->Cp()->LeftPanel()->Update(UPDATE_KEEP_SELECTION);
	Global->CtrlObject->Cp()->LeftPanel()->Redraw();
	Global->CtrlObject->Cp()->RightPanel()->Update(UPDATE_KEEP_SELECTION);
	Global->CtrlObject->Cp()->RightPanel()->Redraw();
}

//BUGBUG
void HighlightDlgUpdateUserControl(matrix_view<FAR_CHAR_INFO> const& VBufColorExample, const highlight::element &Colors);

void HighlightDlgUpdateUserControl(matrix_view<FAR_CHAR_INFO> const& VBufColorExample, const highlight::element &Colors)
{
	const size_t ColorIndices[]{ highlight::color::normal, highlight::color::selected, highlight::color::normal_current, highlight::color::selected_current };

	for (const auto& [ColorRef, Index, Row]: zip(Colors.Color, ColorIndices, VBufColorExample))
	{
		auto BakedColor = ColorRef;
		highlight::configuration::ApplyFinalColor(BakedColor, Index);

		Row.front() = { BoxSymbols[BS_V2], {}, {}, colors::PaletteColorToFarColor(COL_PANELBOX) };

		auto Iterator = Row.begin() + 1;

		if (!Colors.Mark.Mark.empty() && !Colors.Mark.Inherit)
		{
			Iterator->Char = Colors.Mark.Mark.front();
			Iterator->Attributes = BakedColor.MarkColor;
			++Iterator;
		}

		const std::span FileArea(Iterator, Row.end() - 1);
		const auto Str = fit_to_left(msg(lng::MHighlightExample), FileArea.size());

		for (const auto& [Cell, Char]: zip(FileArea, Str))
		{
			Cell = { Char, {}, {}, BakedColor.FileColor };
		}

		Row.back() = { BoxSymbols[BS_V1], {}, {}, Row.front().Attributes };
	}
}

void highlight::configuration::HiEdit(int MenuPos)
{
	const auto HiMenu = VMenu2::create(msg(lng::MHighlightTitle), {}, ScrY - 4);
	HiMenu->SetHelp(L"HighlightList"sv);
	HiMenu->SetMenuFlags(VMENU_WRAPMODE);
	HiMenu->SetPosition({ -1, -1, 0, 0 });
	HiMenu->SetBottomTitle(KeysToLocalizedText(KEY_INS, KEY_DEL, KEY_F4, KEY_F5, KEY_CTRLUP, KEY_CTRLDOWN, KEY_CTRLR));
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
						{ lng::MYes, lng::MCancel }) != message_result::first_button)
					{
						break;
					}

					const auto cfg = ConfigProvider().CreateHighlightConfig();
					SetHighlighting(true, *cfg); //delete old settings

					Load(*cfg);
					FillMenu(HiMenu.get(), SelectPos);

					NeedUpdate = true;
					break;
				}

				case KEY_NUMDEL:
				case KEY_DEL:
				{
					int *Count=nullptr;
					const auto RealSelectPos = MenuPosToRealPos(SelectPos, Count);

					if (Count && RealSelectPos < static_cast<int>(HiData.size()))
					{
						if (Message(MSG_WARNING,
							msg(lng::MHighlightTitle),
							{
								msg(lng::MHighlightAskDel),
								HiData[RealSelectPos].GetMask()
							},
							{ lng::MDelete, lng::MCancel }) != message_result::first_button)
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
					const auto RealSelectPos = MenuPosToRealPos(SelectPos, Count);

					if (Count && RealSelectPos < static_cast<int>(HiData.size()) && FileFilterConfig(HiData[RealSelectPos], true))
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
					const auto RealSelectPos = MenuPosToRealPos(SelectPos, Count, true);

					if (Count)
					{
						FileFilterParams NewHData;

						if (Key == KEY_F5)
							NewHData = HiData[RealSelectPos].Clone();

						if (FileFilterConfig(NewHData, true))
						{
							(*Count)++;
							const auto Iterator = HiData.emplace(HiData.begin()+RealSelectPos, std::move(NewHData));
							HiMenu->AddItem(MenuItemEx(MenuString(std::to_address(Iterator), true)), SelectPos);
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
					const auto RealSelectPos = MenuPosToRealPos(SelectPos, Count);

					if (Count && SelectPos > 0)
					{
						using std::ranges::swap;
						swap(HiMenu->at(SelectPos), HiMenu->at(SelectPos - 1));
						if (UpperCount && RealSelectPos==FirstCount && RealSelectPos<FirstCount+UpperCount)
						{
							FirstCount++;
							UpperCount--;
							SelectPos--;
							swap(HiMenu->at(SelectPos), HiMenu->at(SelectPos - 1));
						}
						else if (LowerCount && RealSelectPos==FirstCount+UpperCount && RealSelectPos<FirstCount+UpperCount+LowerCount)
						{
							UpperCount++;
							LowerCount--;
							SelectPos--;
							swap(HiMenu->at(SelectPos), HiMenu->at(SelectPos - 1));
						}
						else if (LastCount && RealSelectPos==FirstCount+UpperCount+LowerCount)
						{
							LowerCount++;
							LastCount--;
							SelectPos--;
							swap(HiMenu->at(SelectPos), HiMenu->at(SelectPos - 1));
						}
						else
						{
							swap(HiData[RealSelectPos], HiData[RealSelectPos-1]);
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
					const auto RealSelectPos = MenuPosToRealPos(SelectPos, Count);

					if (Count && SelectPos < static_cast<int>(HiMenu->size()-2))
					{
						using std::swap;
						swap(HiMenu->at(SelectPos), HiMenu->at(SelectPos + 1));
						if (FirstCount && RealSelectPos==FirstCount-1)
						{
							FirstCount--;
							UpperCount++;
							SelectPos++;
							swap(HiMenu->at(SelectPos), HiMenu->at(SelectPos + 1));
						}
						else if (UpperCount && RealSelectPos==FirstCount+UpperCount-1)
						{
							UpperCount--;
							LowerCount++;
							SelectPos++;
							swap(HiMenu->at(SelectPos), HiMenu->at(SelectPos + 1));
						}
						else if (LowerCount && RealSelectPos==FirstCount+UpperCount+LowerCount-1)
						{
							LowerCount--;
							LastCount++;
							SelectPos++;
							swap(HiMenu->at(SelectPos), HiMenu->at(SelectPos + 1));
						}
						else
						{
							swap(HiData[RealSelectPos], HiData[RealSelectPos+1]);
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

static void SaveHighlight(HierarchicalConfig& cfg, const HierarchicalConfig::key& key, FileFilterParams const& Item)
{
	filters::SaveFilter(cfg, key.get(), Item);

	const auto Colors = Item.GetColors();

	for (const auto& [Color, Index]: enumerate(Colors.Color))
	{
		cfg.SetValue(key, names::file_color(Index), view_bytes(Color.FileColor));
		cfg.SetValue(key, names::mark_color(Index), view_bytes(Color.MarkColor));
	}

	cfg.SetValue(key, names::Mark, Colors.Mark.Mark);

	// "MarkChar" used to contain both the mark character and the inheritance flag.
	// "Mark" is the main storage of the mark now, but we still store the first character in "MarkChar" for compatibility:
	cfg.SetValue(key, names::MarkChar, make_integer<uint32_t>(
		uint16_t{ Colors.Mark.Mark.empty()? wchar_t{} : Colors.Mark.Mark.front() },
		make_integer<uint16_t, uint8_t>(Colors.Mark.Inherit? 0xff : 0, 0))
	);
	cfg.SetValue(key, names::ContinueProcessing, Item.GetContinueProcessing());
}

void highlight::configuration::Save(bool Always)
{
	if (!m_Changed && !Always)
		return;

	m_Changed = false;

	const auto cfg = ConfigProvider().CreateHighlightConfig();

	SCOPED_ACTION(auto)(cfg->ScopedTransaction());

	if (const auto Key = cfg->FindByName(cfg->root_key, names::Highlight))
		cfg->DeleteKeyTree(Key);

	if (const auto Key = cfg->FindByName(cfg->root_key, names::SortGroups))
		cfg->DeleteKeyTree(Key);

	const struct
	{
		string_view KeyName;
		string_view GroupName;
		int from;
		int to;
	}
	Data[]
	{
		{ names::Highlight,  names::Group,      0,                                    FirstCount },
		{ names::SortGroups, names::UpperGroup, FirstCount,                           FirstCount + UpperCount },
		{ names::SortGroups, names::LowerGroup, FirstCount + UpperCount,              FirstCount + UpperCount + LowerCount },
		{ names::Highlight,  names::LastGroup,  FirstCount + UpperCount + LowerCount, FirstCount + UpperCount + LowerCount + LastCount },
	};

	for(const auto& i: Data)
	{
		const auto root = cfg->CreateKey(cfg->root_key, i.KeyName);

		for (const auto j: std::views::iota(i.from, i.to))
		{
			SaveHighlight(*cfg, cfg->CreateKey(root, i.GroupName + str(j - i.from)), HiData[j]);
		}
	}
}
