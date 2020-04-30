/*
delete.cpp

Удаление файлов
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

// Self:
#include "delete.hpp"

// Internal:
#include "flink.hpp"
#include "scantree.hpp"
#include "treelist.hpp"
#include "constitle.hpp"
#include "TPreRedrawFunc.hpp"
#include "taskbar.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "panelmix.hpp"
#include "mix.hpp"
#include "dirinfo.hpp"
#include "elevation.hpp"
#include "wakeful.hpp"
#include "stddlg.hpp"
#include "lang.hpp"
#include "FarDlgBuilder.hpp"
#include "strmix.hpp"
#include "DlgGuid.hpp"
#include "cvtname.hpp"
#include "fileattr.hpp"
#include "copy_progress.hpp"
#include "global.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

enum DEL_MODE
{
	DEL_SCAN,
	DEL_DEL,
	DEL_WIPE,
	DEL_WIPEPROCESS
};

static void PR_ShellDeleteMsg();

class ShellDelete : noncopyable
{
public:
	ShellDelete(panel_ptr SrcPanel, delete_type Type, bool UpdateDiz);

	struct progress
	{
		size_t Value;
		size_t Total;
	};

private:
	bool ConfirmDeleteReadOnlyFile(const string& Name, DWORD Attr);
	bool ShellRemoveFile(const string& Name, progress Files);
	bool ERemoveDirectory(const string& Name, delete_type Type, bool& RetryRecycleAsRemove);
	bool RemoveToRecycleBin(const string& Name, bool dir, bool& RetryRecycleAsRemove);

	int ReadOnlyDeleteMode{-1};
	int SkipWipeMode{-1};
	bool m_SkipFileErrors{};
	bool m_SkipFolderErrors{};
	unsigned ProcessedItems{};
	delete_type m_DeleteType;
};

struct DelPreRedrawItem : public PreRedrawItem
{
	DelPreRedrawItem():
		PreRedrawItem(PR_ShellDeleteMsg)
	{}

	string name;
	DEL_MODE Mode{};
	ShellDelete::progress Files{};
	int WipePercent{};
};

static void ShellDeleteMsgImpl(const string& Name, DEL_MODE Mode, ShellDelete::progress Files, int WipePercent)
{
	string strProgress, strWipeProgress;
	const auto Width = copy_progress::CanvasWidth();

	if(Mode==DEL_WIPEPROCESS || Mode==DEL_WIPE)
	{
		strWipeProgress = make_progressbar(Width, WipePercent, true, !Files.Total);
	}

	if (Mode!=DEL_SCAN && Files.Total)
	{
		const auto Percent = ToPercent(Files.Value, Files.Total);
		strProgress = make_progressbar(Width, Percent, true, true);
		ConsoleTitle::SetFarTitle(concat(L'{', str(Percent), L"%} "sv, msg(Mode == DEL_WIPE || Mode == DEL_WIPEPROCESS? lng::MDeleteWipeTitle : lng::MDeleteTitle)));
	}

	{
		std::vector MsgItems
		{
			msg(Mode == DEL_SCAN ? lng::MScanningFolder : (Mode == DEL_WIPE || Mode == DEL_WIPEPROCESS) ? lng::MDeletingWiping : lng::MDeleting),
			fit_to_left(truncate_path(Name, Width), Width)
		};

		if (!strWipeProgress.empty())
			MsgItems.emplace_back(strWipeProgress);

		MsgItems.emplace_back(L"\x1"sv);
		MsgItems.emplace_back(copy_progress::FormatCounter(lng::MCopyFilesTotalInfo, lng::MCopyBytesTotalInfo, Files.Value, Files.Total, Files.Total != 0, copy_progress::CanvasWidth() - 5));

		if (!strProgress.empty())
			MsgItems.emplace_back(strProgress);

		Message(MSG_LEFTALIGN,
			msg((Mode == DEL_WIPE || Mode == DEL_WIPEPROCESS) ? lng::MDeleteWipeTitle : lng::MDeleteTitle),
			std::move(MsgItems),
			{});
	}
}

static void ShellDeleteMsg(const string& Name, DEL_MODE Mode, ShellDelete::progress Files, int WipePercent)
{
	if (CheckForEscSilent() && ConfirmAbortOp())
		cancel_operation();

	ShellDeleteMsgImpl(Name, Mode, Files, WipePercent);

	TPreRedrawFunc::instance()([&](DelPreRedrawItem& Item)
	{
		Item.name = Name;
		Item.Mode = Mode;
		Item.Files = Files;
		Item.WipePercent = WipePercent;
	});
}

static void PR_ShellDeleteMsg()
{
	TPreRedrawFunc::instance()([](const DelPreRedrawItem& Item)
	{
		ShellDeleteMsgImpl(Item.name, Item.Mode, Item.Files, Item.WipePercent);
	});
}

static bool EraseFileData(const string& Name, ShellDelete::progress Files)
{
	os::fs::file_walker File;
	if (!File.Open(Name, FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_SEQUENTIAL_SCAN))
		return false;

	unsigned long long FileSize;
	if (!File.GetSize(FileSize))
		return false;

	if (!FileSize)
		return true; // nothing to do here

	const DWORD BufSize=65536;
	if (!File.InitWalk(BufSize))
		return false;

	const time_check TimeCheck(time_check::mode::immediate);

	std::mt19937 mt(clock()); // std::random_device doesn't work in w2k
	std::uniform_int_distribution CharDist(0, 255);

	auto BufInit = false;

	do
	{
		static std::array<BYTE, BufSize> Buf;
		if (!BufInit)
		{
			if (Global->Opt->WipeSymbol == -1)
			{
				std::generate(ALL_RANGE(Buf), [&]{ return CharDist(mt); });
			}
			else
			{
				Buf.fill(Global->Opt->WipeSymbol);
				BufInit = true;
			}
		}

		if (!File.Write(Buf.data(), File.GetChunkSize()))
			return false;

		if (TimeCheck)
			ShellDeleteMsg(Name, DEL_WIPEPROCESS, Files, File.GetPercent());
	}
	while(File.Step());

	if (!File.SetPointer(0, nullptr, FILE_BEGIN))
		return false;

	if (!File.SetEnd())
		return false;

	return true;
}

static bool EraseFile(const string& Name, ShellDelete::progress Files)
{
	if (!os::fs::set_file_attributes(Name, FILE_ATTRIBUTE_NORMAL))
		return false;

	if (!EraseFileData(Name, Files))
		return false;

	const auto strTempName = MakeTemp({}, false);

	if (!os::fs::move_file(Name, strTempName))
		return false;

	return os::fs::delete_file(strTempName);
}

static bool EraseDirectory(const string& Name)
{
	string strPath = Name;

	if (!CutToParent(strPath))
	{
		strPath.clear();
	}

	const auto strTempName = MakeTemp({}, false, strPath);

	if (!os::fs::move_file(Name, strTempName))
	{
		return false;
	}

	return os::fs::remove_directory(strTempName);
}

static void show_confirmation(
	panel_ptr const SrcPanel,
	delete_type const DeleteType,
	size_t const SelCount,
	const os::fs::find_data& SingleSelData
)
{
	if (!Global->Opt->Confirm.Delete)
		return;

	string strDeleteFilesMsg;

	if (SelCount == 1)
	{
		strDeleteFilesMsg = QuoteOuterSpace(SingleSelData.FileName);
	}
	else
	{
		// в зависимости от числа ставим нужное окончание
		auto StrItems = str(SelCount);
		auto Ends = msg(lng::MAskDeleteItemsA);
		if (const auto LenItems = StrItems.size())
		{
			if ((LenItems >= 2 && StrItems[LenItems - 2] == L'1') ||
				StrItems[LenItems - 1] >= L'5' ||
				StrItems[LenItems - 1] == L'0')
				Ends = msg(lng::MAskDeleteItemsS);
			else if (StrItems[LenItems - 1] == L'1')
				Ends = msg(lng::MAskDeleteItems0);
		}
		strDeleteFilesMsg = format(msg(lng::MAskDeleteItems), SelCount, Ends);
	}

	lng mTitle, mDText, mDBttn;
	const GUID* Id;

	if (DeleteType == delete_type::erase)
	{
		mTitle = lng::MDeleteWipeTitle;
		mDBttn = lng::MDeleteWipe;
		Id = &DeleteWipeId;
	}
	else if (DeleteType == delete_type::remove)
	{
		mTitle = lng::MDeleteTitle;
		mDBttn = lng::MDelete;
		Id = &DeleteFileFolderId;
	}
	else if (DeleteType == delete_type::recycle)
	{
		mTitle = lng::MDeleteTitle;
		mDBttn = lng::MDeleteRecycle;
		Id = &DeleteRecycleId;
	}
	else
		UNREACHABLE;

	std::vector items{ strDeleteFilesMsg };

	string tText;
	bool HighlightSelected = Global->Opt->DelOpt.HighlightSelected;
	const size_t MaxItems = std::min(std::max(static_cast<int>(Global->Opt->DelOpt.ShowSelected), 1), ScrY / 2);

	if (SelCount == 1)
	{
		const auto folder = (SingleSelData.Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

		if (DeleteType == delete_type::erase)
			mDText = folder? lng::MAskWipeFolder : lng::MAskWipeFile;
		else if (DeleteType == delete_type::remove)
			mDText = folder? lng::MAskDeleteFolder : lng::MAskDeleteFile;
		else if (DeleteType == delete_type::recycle)
			mDText = folder? lng::MAskDeleteRecycleFolder : lng::MAskDeleteRecycleFile;
		else
			UNREACHABLE;

		if (SingleSelData.Attributes & FILE_ATTRIBUTE_REPARSE_POINT)
		{
			if (DeleteType == delete_type::erase)
			{
				mDText = folder? lng::MAskDeleteFolder : lng::MAskDeleteFile;
				mDBttn = lng::MDelete;
			}

			Id = &DeleteLinkId;

			auto FullName = ConvertNameToFull(SingleSelData.FileName);
			auto Str = msg(lng::MAskDeleteLink);

			if (GetReparsePointInfo(FullName, FullName))
			{
				NormalizeSymlinkName(FullName);

				const os::fs::file_status Status(FullName);
				if (os::fs::exists(Status))
					append(Str, L' ', msg(is_directory(Status) ? lng::MAskDeleteLinkFolder : lng::MAskDeleteLinkFile));
			}

			items.emplace_back(std::move(Str));
			items.emplace_back(std::move(FullName));
		}

		if (HighlightSelected)
		{
			string name, sname;
			if (SrcPanel->GetCurName(name, sname))
			{
				inplace::QuoteOuterSpace(name);
				HighlightSelected = strDeleteFilesMsg != name;
			}
		}
	}
	else
	{
		if (DeleteType == delete_type::erase)
			mDText = lng::MAskWipe;
		else if (DeleteType == delete_type::remove)
			mDText = lng::MAskDelete;
		else if (DeleteType == delete_type::recycle)
			mDText = lng::MAskDeleteRecycle;
		else
			UNREACHABLE;

		if (MaxItems > 1)
		{
			tText = concat(msg(mDText), L' ', strDeleteFilesMsg);
			items.clear();

			for (const auto& i: SrcPanel->enum_selected())
			{
				items.emplace_back(QuoteOuterSpace(i.FileName));

				if (items.size() + 1 == MaxItems && items.size() + 1 < SelCount)
				{
					items.emplace_back(L"…"sv);
					break;
				}
			}
		}
	}

	intptr_t start_hilite = 0, end_hilite = 0;

	DialogBuilder Builder(mTitle, {}, [&](Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
	{
		if (HighlightSelected && Msg == DN_CTLCOLORDLGITEM && Param1 >= start_hilite && Param1 <= end_hilite)
		{
			auto& Colors = *static_cast<const FarDialogItemColors*>(Param2);
			Colors.Colors[0] = Colors.Colors[1];
		}
		return Dlg->DefProc(Msg, Param1, Param2);
	});

	if (tText.empty())
		tText = msg(mDText);

	Builder.AddText(tText)->Flags = DIF_CENTERTEXT;

	if (MaxItems > 1 && SelCount > 1)
		Builder.AddSeparator();

	for (auto& i: items)
	{
		inplace::truncate_center(i, ScrX + 1 - 6 * 2);
		const auto dx = Builder.AddText(i);
		dx->Flags = (SelCount <= 1 || MaxItems <= 1 ? DIF_CENTERTEXT : 0) | DIF_SHOWAMPERSAND;
		size_t index = Builder.GetLastID();
		end_hilite = index;
		if (!start_hilite)
			start_hilite = index;
	}

	Builder.AddOKCancel(mDBttn, lng::MCancel);
	Builder.SetId(*Id);

	if (DeleteType != delete_type::recycle)
		Builder.SetDialogMode(DMODE_WARNINGSTYLE);

	if (!Builder.ShowDialog())
		cancel_operation();
}

static auto calculate_total(panel_ptr const SrcPanel)
{
	struct
	{
		size_t Items{};
		size_t Size{};
	}
	Total;

	if (!Global->Opt->DelOpt.ShowTotal)
		return Total;

	const time_check TimeCheck;

	const auto DirInfoCallback = [&](string_view const Name, unsigned long long const ItemsCount, unsigned long long const Size)
	{
		if (TimeCheck)
			DirInfoMsg(msg(lng::MDeletingTitle), Name, Total.Items + ItemsCount, Total.Size + Size);
	};

	// BUGBUG
	for (const auto& i : SrcPanel->enum_selected())
	{
		++Total.Items;

		if (i.Attributes & FILE_ATTRIBUTE_DIRECTORY && !os::fs::is_directory_symbolic_link(i))
		{
			DirInfoData Data = {};

			if (GetDirInfo(i.FileName, Data, nullptr, DirInfoCallback, 0) <= 0)
				continue;

			Total.Items += Data.FileCount + Data.DirCount;
			Total.Size += Data.FileSize;
		}
	}

	return Total;
}

ShellDelete::ShellDelete(panel_ptr SrcPanel, delete_type const Type, bool const UpdateDiz):
	m_DeleteType(Type)
{
	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<DelPreRedrawItem>());

	bool DeleteAllFolders = !Global->Opt->Confirm.DeleteFolder;

	bool NeedUpdate = false;
	bool NeedSetUpADir = false;

	SCOPE_EXIT
	{
		if (NeedUpdate)
		{
			ShellUpdatePanels(SrcPanel,NeedSetUpADir);
		}
	};

	const auto SelCount = SrcPanel->GetSelCount();
	if (!SelCount)
		return;

	os::fs::find_data SingleSelData;
	if (!SrcPanel->get_first_selected(SingleSelData))
		return;

	if (m_DeleteType == delete_type::recycle && FAR_GetDriveType(GetPathRoot(ConvertNameToFull(SingleSelData.FileName))) != DRIVE_FIXED)
		m_DeleteType = delete_type::remove;

	show_confirmation(SrcPanel, m_DeleteType, SelCount, SingleSelData);

	NeedUpdate = true;
	NeedSetUpADir = CheckUpdateAnotherPanel(SrcPanel, SingleSelData.FileName);

	if (SrcPanel->GetType() == panel_type::TREE_PANEL)
		FarChDir(L"\\"sv);

	ConsoleTitle::SetFarTitle(msg(lng::MDeletingTitle));
	SCOPED_ACTION(taskbar::indeterminate);
	SCOPED_ACTION(wakeful);
	SetCursorType(false, 0);

	const auto Total = calculate_total(SrcPanel);

	os::fs::find_data SelFindData;
	SrcPanel->GetSelName(nullptr);
	const time_check TimeCheck(time_check::mode::immediate);
	bool cannot_recycle_try_delete_folder = false;

	string strSelName;
	string strSelShortName;

	// BUGBUGBUG
	// TODO: enumerator
	while (cannot_recycle_try_delete_folder || SrcPanel->GetSelName(&strSelName, &strSelShortName, &SelFindData))
	{
		if (strSelName.empty() || IsRelativeRoot(strSelName) || IsRootPath(strSelName))
			continue;

		if (TimeCheck)
			ShellDeleteMsg(strSelName, m_DeleteType == delete_type::erase? DEL_WIPE : DEL_DEL, { ProcessedItems, Total.Items }, 0);

		if (!(SelFindData.Attributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (ConfirmDeleteReadOnlyFile(strSelName, SelFindData.Attributes))
			{
				if (ShellRemoveFile(strSelName, { ProcessedItems, Total.Items }) && UpdateDiz)
					SrcPanel->DeleteDiz(strSelName, strSelShortName);
			}

			continue;
		}

		const auto DirSymLink = os::fs::is_directory_symbolic_link(SelFindData);

		if (!DeleteAllFolders && !cannot_recycle_try_delete_folder)
		{
			const auto strFullName = ConvertNameToFull(strSelName);

			if (os::fs::is_not_empty_directory(strFullName))
			{
				int MsgCode = 0; // для symlink не нужно подтверждение
				if (!DirSymLink)
				{
					const GUID* guidId = &DeleteFolderId;
					auto tit = lng::MDeleteFolderTitle, con = lng::MDeleteFolderConfirm, del = lng::MDeleteFileDelete;
					if (m_DeleteType == delete_type::erase)
					{
						tit = lng::MWipeFolderTitle;
						con = lng::MWipeFolderConfirm;
						del = lng::MDeleteFileWipe;
						guidId = &WipeFolderId;
					}
					else if (m_DeleteType == delete_type::recycle)
					{
						con = lng::MRecycleFolderConfirm;
						del = lng::MDeleteRecycle;
						guidId = &DeleteFolderRecycleId;
					}

					MsgCode=Message(MSG_WARNING,
						msg(tit),
						{
							msg(con),
							strFullName
						},
						{ del, lng::MDeleteFileAll, lng::MDeleteFileSkip, lng::MDeleteFileCancel },
						{}, guidId);
				}

				if (MsgCode == Message::first_button)
				{
					;
				}
				else if (MsgCode == Message::second_button)
				{
					DeleteAllFolders = true;
				}
				else if (MsgCode == Message::third_button)
				{
					continue;
				}
				else
				{
					NeedSetUpADir = false;
					cancel_operation();
				}
			}
		}

		if (!DirSymLink && m_DeleteType != delete_type::recycle)
		{
			ScanTree ScTree(true, true, FALSE, false);

			const auto strSelFullName = IsAbsolutePath(strSelName)?
				strSelName :
				path::join(SrcPanel->GetCurDir(), strSelName);

			ScTree.SetFindPath(strSelFullName, L"*"sv);
			const time_check TreeTimeCheck(time_check::mode::immediate);

			os::fs::find_data FindData;
			string strFullName;
			while (ScTree.GetNextName(FindData,strFullName))
			{
				if (TreeTimeCheck)
					ShellDeleteMsg(strFullName, m_DeleteType == delete_type::erase? DEL_WIPE : DEL_DEL, { ProcessedItems, Total.Items }, 0);

				if (FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (os::fs::is_directory_symbolic_link(FindData))
					{
						if (FindData.Attributes & FILE_ATTRIBUTE_READONLY)
							os::fs::set_file_attributes(strFullName,FILE_ATTRIBUTE_NORMAL);

						bool Dummy = false;
						if (!ERemoveDirectory(strFullName, m_DeleteType, Dummy))
						{
							ScTree.SkipDir();
							continue;
						}

						TreeList::DelTreeName(strFullName);

						if (UpdateDiz)
							SrcPanel->DeleteDiz(strFullName,strSelShortName);

						continue;
					}

					if (!DeleteAllFolders && !ScTree.IsDirSearchDone() && os::fs::is_not_empty_directory(strFullName))
					{
						const auto MsgCode = Message(MSG_WARNING,
							msg(m_DeleteType == delete_type::erase? lng::MWipeFolderTitle : lng::MDeleteFolderTitle),
							{
								msg(m_DeleteType == delete_type::erase? lng::MWipeFolderConfirm : lng::MDeleteFolderConfirm),
								strFullName
							},
							{ m_DeleteType == delete_type::erase? lng::MDeleteFileWipe : lng::MDeleteFileDelete, lng::MDeleteFileAll, lng::MDeleteFileSkip, lng::MDeleteFileCancel },
							{}, m_DeleteType == delete_type::erase? &WipeFolderId : &DeleteFolderId); // ??? other GUID ???

						if (MsgCode == Message::first_button)
						{
							;
						}
						else if (MsgCode == Message::second_button)
						{
							DeleteAllFolders = true;
						}
						else if (MsgCode == Message::third_button)
						{
							ScTree.SkipDir();
							continue;
						}
						else
						{
							cancel_operation();
						}
					}

					if (ScTree.IsDirSearchDone())
					{
						if (FindData.Attributes & FILE_ATTRIBUTE_READONLY)
							os::fs::set_file_attributes(strFullName,FILE_ATTRIBUTE_NORMAL);

						bool Dummy = false;
						if (!ERemoveDirectory(strFullName, m_DeleteType, Dummy))
						{
							//ScTree.SkipDir();
							continue;
						}

						TreeList::DelTreeName(strFullName);
					}
				}
				else
				{
					if (ConfirmDeleteReadOnlyFile(strFullName,FindData.Attributes))
					{
						// BUGBUG check result
						ShellRemoveFile(strFullName, { ProcessedItems, Total.Items });
					}
				}
			}
		}

		if (SelFindData.Attributes & FILE_ATTRIBUTE_READONLY)
			os::fs::set_file_attributes(strSelName,FILE_ATTRIBUTE_NORMAL);

		bool RetryRecycleAsRemove = false;
		const auto Removed = ERemoveDirectory(
			strSelName,
			m_DeleteType == delete_type::recycle && DirSymLink && !IsWindowsVistaOrGreater()?
				delete_type::remove :
				m_DeleteType,
			RetryRecycleAsRemove
		);

		if (cannot_recycle_try_delete_folder)
		{
			cannot_recycle_try_delete_folder = false;
			m_DeleteType = delete_type::recycle;
		}

		if (Removed)
		{
			TreeList::DelTreeName(strSelName);

			if (UpdateDiz)
				SrcPanel->DeleteDiz(strSelName,strSelShortName);
		}
		else if (RetryRecycleAsRemove)
		{
			--ProcessedItems;
			cannot_recycle_try_delete_folder = true;
			m_DeleteType = delete_type::remove;
		}
	}
}

bool ShellDelete::ConfirmDeleteReadOnlyFile(const string& Name,DWORD Attr)
{
	if (!(Attr & FILE_ATTRIBUTE_READONLY))
		return true;

	if (!Global->Opt->Confirm.RO)
		ReadOnlyDeleteMode = Message::first_button;

	int MsgCode;

	if (ReadOnlyDeleteMode != -1)
	{
		MsgCode = ReadOnlyDeleteMode;
	}
	else
	{
		static constexpr std::tuple
			EraseData{ lng::MAskWipeRO, lng::MDeleteFileWipe, &DeleteAskWipeROId },
			DeleteData{ lng::MAskDeleteRO, lng::MDeleteFileDelete, &DeleteAskDeleteROId };

		const auto& [AskId, ButtonId, UidPtr] = m_DeleteType == delete_type::erase? EraseData : DeleteData;

		MsgCode = Message(MSG_WARNING,
			msg(lng::MWarning),
			{
				msg(lng::MDeleteRO),
				Name,
				msg(AskId)
			},
			{ ButtonId, lng::MDeleteFileAll, lng::MDeleteFileSkip, lng::MDeleteFileSkipAll, lng::MDeleteFileCancel },
			{}, UidPtr);
	}

	switch (MsgCode)
	{
	case Message::second_button:
		ReadOnlyDeleteMode = Message::first_button;
		[[fallthrough]];
	case Message::first_button:
		os::fs::set_file_attributes(Name, FILE_ATTRIBUTE_NORMAL);
		return true;

	case Message::fourth_button:
		ReadOnlyDeleteMode = Message::third_button;
		[[fallthrough]];
	case Message::third_button:
		return false;

	default:
		cancel_operation();
	}
}

static int confirm_erase_file_with_hardlinks(const string& File)
{
	const auto Hardlinks = GetNumberOfLinks(File);
	if (!Hardlinks || *Hardlinks < 2)
		return Message::first_button;

	return Message(MSG_WARNING,
		msg(lng::MError),
		{
			File,
			msg(lng::MDeleteHardLink1),
			msg(lng::MDeleteHardLink2),
			msg(lng::MDeleteHardLink3)
		},
		{ lng::MDeleteFileWipe, lng::MDeleteFileAll, lng::MDeleteFileSkip, lng::MDeleteFileSkipAll, lng::MDeleteCancel },
		{}, &WipeHardLinkId);
}

bool ShellDelete::ShellRemoveFile(const string& Name, progress Files)
{
	ProcessedItems++;
	const auto strFullName = ConvertNameToFull(Name);

	for (;;)
	{
		bool recycle_bin = false;
		if (m_DeleteType == delete_type::erase)
		{
			int MsgCode;

			if (SkipWipeMode != -1)
			{
				MsgCode = SkipWipeMode;
			}
			else
			{
				MsgCode = confirm_erase_file_with_hardlinks(strFullName);
			}

			switch (MsgCode)
			{
			case Message::second_button:
				SkipWipeMode = Message::first_button;
				[[fallthrough]];
			case Message::first_button:
				if (EraseFile(strFullName, Files))
					return true;

			case Message::fourth_button:
				SkipWipeMode = Message::third_button;
				[[fallthrough]];
			case Message::third_button:
				return false;

			default:
				cancel_operation();
			}
		}
		else if (m_DeleteType == delete_type::remove)
		{
			if (os::fs::delete_file(strFullName))
				return true;
		}
		else
		{
			recycle_bin = true;
			bool RetryRecycleAsRemove = false;
			if (RemoveToRecycleBin(strFullName, false, RetryRecycleAsRemove))
				return true;

			if (m_SkipFileErrors)
				return false;

			if (RetryRecycleAsRemove)
			{
				recycle_bin = false;
				if (os::fs::delete_file(strFullName))
					return true;
			}
			else
			{
				return false;
			}

		}

		operation MsgCode;

		if (!m_SkipFileErrors)
		{
			const auto ErrorState = error_state::fetch();

			MsgCode = OperationFailed(ErrorState, strFullName, lng::MError, msg(recycle_bin? lng::MCannotRecycleFile : lng::MCannotDeleteFile));
		}
		else
		{
			MsgCode = operation::skip;
		}

		switch (MsgCode)
		{
		case operation::retry:
			continue;

		case operation::skip_all:
			m_SkipFileErrors = true;
			[[fallthrough]];
		case operation::skip:
			return false;

		case operation::cancel:
			cancel_operation();
		}
	}
}

bool ShellDelete::ERemoveDirectory(const string& Name, delete_type const Type, bool& RetryRecycleAsRemove)
{
	ProcessedItems++;

	for (;;)
	{
		bool recycle_bin = false;
		switch(Type)
		{
		case delete_type::remove:
			if (os::fs::remove_directory(Name))
				return true;

		case delete_type::erase:
			if (EraseDirectory(Name))
				return true;

		case delete_type::recycle:
			{
				recycle_bin = true;
				if (RemoveToRecycleBin(Name, true, RetryRecycleAsRemove))
					return true;

				if (RetryRecycleAsRemove)
					return false;
			}
			break;

		default:
			UNREACHABLE;
		}

		operation MsgCode;

		if (!m_SkipFolderErrors)
		{
			const auto ErrorState = error_state::fetch();

			MsgCode = OperationFailed(ErrorState, Name, lng::MError, msg(recycle_bin? lng::MCannotRecycleFolder : lng::MCannotDeleteFolder));
		}
		else
		{
			MsgCode = operation::skip;
		}

		switch (MsgCode)
		{
		case operation::retry:
			break;

		case operation::skip_all:
			m_SkipFolderErrors = true;
			[[fallthrough]];
		case operation::skip:
			return false;

		case operation::cancel:
			cancel_operation();
		}
	}
}

static void break_links_for_old_os(const string& Name)
{
	// При удалении в корзину папки с симлинками получим траблу, если предварительно линки не убрать.
	if (IsWindowsVistaOrGreater() || !os::fs::is_directory(Name))
		return;

	string strFullName2;
	os::fs::find_data FindData;
	ScanTree ScTree(true, true, FALSE, false);
	ScTree.SetFindPath(Name, L"*"sv);

	bool MessageShown = false;
	bool SkipErrors = false;

	while (ScTree.GetNextName(FindData, strFullName2))
	{
		if (os::fs::is_directory_symbolic_link(FindData))
		{
			if (!MessageShown)
			{
				MessageShown = true;
				if (Message(MSG_WARNING,
					msg(lng::MWarning),
					{
						msg(lng::MRecycleFolderConfirmDeleteLink1),
						msg(lng::MRecycleFolderConfirmDeleteLink2),
						msg(lng::MRecycleFolderConfirmDeleteLink3),
						msg(lng::MRecycleFolderConfirmDeleteLink4)
					},
					{ lng::MYes, lng::MCancel },
					{}, &RecycleFolderConfirmDeleteLinkId
				) != Message::first_button)
				{
					cancel_operation();
				}
			}
			// BUGBUG, check result
			(void)EDeleteReparsePoint(strFullName2, FindData.Attributes, SkipErrors);
		}
	}
}

bool ShellDelete::RemoveToRecycleBin(const string& Name, bool dir, bool& RetryRecycleAsRemove)
{
	RetryRecycleAsRemove = false;
	auto strFullName = ConvertNameToFull(Name);

	break_links_for_old_os(strFullName);

	if (os::fs::move_to_recycle_bin(strFullName))
		return true;

	if (dir? m_SkipFolderErrors : m_SkipFileErrors)
		return false;

	const auto ErrorState = error_state::fetch();

	// probably bad path to recycle bin
	if (
		ErrorState.Win32Error == ERROR_BAD_PATHNAME ||
		ErrorState.Win32Error == ERROR_FILE_NOT_FOUND ||
		ErrorState.Win32Error == ERROR_PATH_NOT_FOUND
	)
	{
		const int MsgCode = Message(MSG_WARNING, ErrorState,
			msg(lng::MError),
			{
				msg(dir? lng::MCannotRecycleFolder : lng::MCannotRecycleFile),
				QuoteOuterSpace(strFullName)
			},
			{ lng::MDeleteFileDelete, lng::MDeleteSkip, lng::MDeleteSkipAll, lng::MDeleteCancel },
			{}, dir? &CannotRecycleFolderId : &CannotRecycleFileId);

		switch (MsgCode)
		{
		case Message::first_button:     // {Delete}
			RetryRecycleAsRemove = true;
			return false;

		case Message::third_button:     // [Skip All]
			(dir? m_SkipFolderErrors : m_SkipFileErrors) = true;
			[[fallthrough]];
		case Message::second_button:    // [Skip]
			return false;

		default:
			cancel_operation();
		}
	}

	return false;
}

void DeleteDirTree(const string& Dir)
{
	if (Dir.empty() ||
	        (Dir.size() == 1 && IsSlash(Dir[0])) ||
	        (Dir.size() == 3 && Dir[1]==L':' && IsSlash(Dir[2])))
		return;

	string strFullName;
	os::fs::find_data FindData;
	ScanTree ScTree(true, true, FALSE, false);
	ScTree.SetFindPath(Dir, L"*"sv);

	while (ScTree.GetNextName(FindData, strFullName))
	{
		os::fs::set_file_attributes(strFullName,FILE_ATTRIBUTE_NORMAL);

		if (FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (ScTree.IsDirSearchDone())
				// BUGBUG check result
				(void)os::fs::remove_directory(strFullName);
		}
		else
			// BUGBUG check result
			(void)os::fs::delete_file(strFullName);
	}

	// BUGBUG check result
	(void)os::fs::set_file_attributes(Dir,FILE_ATTRIBUTE_NORMAL);
	// BUGBUG check result
	(void)os::fs::remove_directory(Dir);
}

bool DeleteFileWithFolder(const string& FileName)
{
	auto strFileOrFolderName = unquote(FileName);
	os::fs::set_file_attributes(strFileOrFolderName, FILE_ATTRIBUTE_NORMAL);

	if (!os::fs::delete_file(strFileOrFolderName))
		return false;

	return CutToParent(strFileOrFolderName) && os::fs::remove_directory(strFileOrFolderName);
}

delayed_deleter::delayed_deleter(bool const WithFolder):
	m_WithFolder(WithFolder)
{
}

delayed_deleter::~delayed_deleter()
{
	for (const auto& i: m_Files)
	{
		if (os::fs::delete_file(i) && m_WithFolder)
		{
			auto Folder = i;
			if (CutToParent(Folder))
				(void)os::fs::remove_directory(Folder);
		}
	}
}

void delayed_deleter::add(string File)
{
	m_Files.emplace_back(std::move(File));
}

bool delayed_deleter::any() const
{
	return !m_Files.empty();
}

void Delete(const panel_ptr& SrcPanel, delete_type const Type)
{
	const auto UpdateDiz = Global->Opt->Diz.UpdateMode == DIZ_UPDATE_ALWAYS || (SrcPanel->IsDizDisplayed() && Global->Opt->Diz.UpdateMode == DIZ_UPDATE_IF_DISPLAYED);

	if (UpdateDiz)
		SrcPanel->ReadDiz();

	const auto strDizName = SrcPanel->GetDizName();
	const auto CheckDiz = [&]{ return !strDizName.empty() && os::fs::exists(strDizName); };
	const auto DizPresent = CheckDiz();

	try
	{
		ShellDelete(SrcPanel, Type, UpdateDiz);
	}
	catch (const operation_cancelled&)
	{
		;
	}

	if (UpdateDiz && DizPresent == CheckDiz())
		SrcPanel->FlushDiz();
}
