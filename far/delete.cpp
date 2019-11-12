﻿/*
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
#include "chgprior.hpp"
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

enum DIRDELTYPE: int
{
	D_DEL,
	D_RECYCLE,
	D_WIPE,
};

enum DEL_RESULT: int
{
	DELETE_SUCCESS,
	DELETE_YES,
	DELETE_SKIP,
	DELETE_CANCEL
};

static void PR_ShellDeleteMsg();

class ShellDelete : noncopyable
{
public:
	ShellDelete(panel_ptr SrcPanel, bool Wipe);

	struct progress
	{
		size_t Value;
		size_t Total;
	};

private:
	DEL_RESULT AskDeleteReadOnly(const string& Name, DWORD Attr, bool Wipe);
	DEL_RESULT ShellRemoveFile(const string& Name, bool Wipe, progress Files);
	DEL_RESULT ERemoveDirectory(const string& Name, DIRDELTYPE Type);
	bool RemoveToRecycleBin(const string& Name, bool dir, DEL_RESULT& ret);

	int ReadOnlyDeleteMode;
	bool m_SkipErrors{};
	int SkipWipeMode;
	bool m_SkipFolderErrors{};
	unsigned ProcessedItems;
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
		std::vector<string> MsgItems
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

static bool WipeFileData(const string& Name, ShellDelete:: progress Files, bool& Cancel)
{
	os::fs::file_walker WipeFile;
	if (!WipeFile.Open(Name, FILE_READ_DATA | FILE_WRITE_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_SEQUENTIAL_SCAN))
		return false;

	unsigned long long FileSize;
	if (!WipeFile.GetSize(FileSize))
		return false;

	if (!FileSize)
		return true; // nothing to do here

	const DWORD BufSize=65536;
	if (!WipeFile.InitWalk(BufSize))
		return false;

	const time_check TimeCheck(time_check::mode::immediate, GetRedrawTimeout());

	std::mt19937 mt(clock()); // std::random_device doesn't work in w2k
	std::uniform_int_distribution<int> CharDist(0, 255);

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

		if (!WipeFile.Write(Buf.data(), WipeFile.GetChunkSize()))
			return false;

		if (TimeCheck)
		{
			if (CheckForEscSilent() && ConfirmAbortOp())
			{
				Cancel=true;
				return false;
			}

			ShellDeleteMsg(Name, DEL_WIPEPROCESS, Files, WipeFile.GetPercent());
		}
	}
	while(WipeFile.Step());

	if (!WipeFile.SetPointer(0, nullptr, FILE_BEGIN))
		return false;

	if (!WipeFile.SetEnd())
		return false;

	return true;
}

static bool WipeFile(const string& Name, ShellDelete::progress Files, bool& Cancel)
{
	if (!os::fs::set_file_attributes(Name, FILE_ATTRIBUTE_NORMAL))
		return false;

	if (!WipeFileData(Name, Files, Cancel))
		return false;

	const auto strTempName = MakeTemp({}, false);

	if (!os::fs::move_file(Name, strTempName))
		return false;

	return os::fs::delete_file(strTempName);
}

static bool WipeDirectory(const string& Name)
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

ShellDelete::ShellDelete(panel_ptr SrcPanel, bool Wipe):
	ReadOnlyDeleteMode(-1),
	SkipWipeMode(-1),
	ProcessedItems(0)
{
	SCOPED_ACTION(ChangePriority)(Global->Opt->DelThreadPriority);
	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<DelPreRedrawItem>());
	string strDeleteFilesMsg;
	bool NeedUpdate = true, NeedSetUpADir = false;
	bool Opt_DeleteToRecycleBin=Global->Opt->DeleteToRecycleBin;
	bool DeleteAllFolders=!Global->Opt->Confirm.DeleteFolder;
	const auto UpdateDiz=(Global->Opt->Diz.UpdateMode==DIZ_UPDATE_ALWAYS ||
	           (SrcPanel->IsDizDisplayed() &&
	            Global->Opt->Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED));

	SCOPE_EXIT
	{
		Global->Opt->DeleteToRecycleBin=Opt_DeleteToRecycleBin;

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

	if (Global->Opt->DeleteToRecycleBin && FAR_GetDriveType(GetPathRoot(ConvertNameToFull(SingleSelData.FileName))) != DRIVE_FIXED)
		Global->Opt->DeleteToRecycleBin = false;

	if (SelCount==1)
	{
		if (SingleSelData.FileName.empty() || IsParentDirectory(SingleSelData))
		{
			NeedUpdate = false;
			return;
		}

		strDeleteFilesMsg = QuoteOuterSpace(SingleSelData.FileName);
	}
	else
	{
		// в зависимости от числа ставим нужное окончание
		auto StrItems = str(SelCount);
		auto Ends = msg(lng::MAskDeleteItemsA);
		if (const auto LenItems = StrItems.size())
		{
			if ((LenItems >= 2 && StrItems[LenItems-2] == L'1') ||
			        StrItems[LenItems-1] >= L'5' ||
			        StrItems[LenItems-1] == L'0')
				Ends=msg(lng::MAskDeleteItemsS);
			else if (StrItems[LenItems-1] == L'1')
				Ends=msg(lng::MAskDeleteItems0);
		}
		strDeleteFilesMsg = format(msg(lng::MAskDeleteItems), SelCount, Ends);
	}

	int Ret = 1;

	//   Обработка "удаления" линков
	if (SelCount == 1 && (SingleSelData.Attributes & FILE_ATTRIBUTE_REPARSE_POINT))
	{
		auto FullName = ConvertNameToFull(SingleSelData.FileName);

		if (GetReparsePointInfo(FullName, FullName))
		{
			NormalizeSymlinkName(FullName);
			auto strAskDeleteLink = msg(lng::MAskDeleteLink);

			const os::fs::file_status Status(FullName);
			if (os::fs::exists(Status))
				append(strAskDeleteLink, L' ', msg(is_directory(Status)? lng::MAskDeleteLinkFolder : lng::MAskDeleteLinkFile));

			Ret=Message(0,
				msg(lng::MDeleteLinkTitle),
				{
					strDeleteFilesMsg,
					strAskDeleteLink,
					FullName
				},
				{ lng::MDeleteLinkDelete, lng::MCancel },
				{}, &DeleteLinkId);

			if (Ret)
				return;
		}
	}

	if (Ret && Global->Opt->Confirm.Delete)
	{
		auto mTitle = Wipe? lng::MDeleteWipeTitle : lng::MDeleteTitle;
		lng mDText;
		string tText;
		auto mDBttn = Wipe? lng::MDeleteWipe : Global->Opt->DeleteToRecycleBin? lng::MDeleteRecycle : lng::MDelete;
		bool bHilite = Global->Opt->DelOpt.HighlightSelected;
		const size_t mshow = std::min(std::max(static_cast<int>(Global->Opt->DelOpt.ShowSelected), 1), ScrY / 2);

		std::vector<string> items{ strDeleteFilesMsg };

		if (SelCount == 1)
		{
			bool folder = (SingleSelData.Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

			if (Wipe && !(SingleSelData.Attributes & FILE_ATTRIBUTE_REPARSE_POINT))
				mDText = folder? lng::MAskWipeFolder : lng::MAskWipeFile;
			else
			{
				if (Global->Opt->DeleteToRecycleBin)
					mDText = folder? lng::MAskDeleteRecycleFolder : lng::MAskDeleteRecycleFile;
				else
					mDText = folder? lng::MAskDeleteFolder : lng::MAskDeleteFile;
			}
			if (bHilite)
			{
				string name, sname;
				if (SrcPanel->GetCurName(name, sname))
				{
					inplace::QuoteOuterSpace(name);
					bHilite = strDeleteFilesMsg != name;
				}
			}
		}
		else
		{
			if (Wipe)
			{
				mDText = lng::MAskWipe;
				mTitle = lng::MDeleteWipeTitle;
			}
			else
				mDText = Global->Opt->DeleteToRecycleBin? lng::MAskDeleteRecycle : lng::MAskDelete;

			if (mshow > 1)
			{
				tText = concat(msg(mDText), L' ', strDeleteFilesMsg);
				items.clear();

				for (const auto& i: SrcPanel->enum_selected())
				{
					items.emplace_back(QuoteOuterSpace(i.FileName));

					if (items.size() > mshow && items.size() < SelCount)
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
			if (bHilite && Msg == DN_CTLCOLORDLGITEM && Param1 >= start_hilite && Param1 <= end_hilite)
			{
				const auto Colors = static_cast<const FarDialogItemColors*>(Param2);
				Colors->Colors[0] = Colors->Colors[1];
			}
			return Dlg->DefProc(Msg, Param1, Param2);
		});

		if (tText.empty())
			tText = msg(mDText);

		Builder.AddText(tText)->Flags = DIF_CENTERTEXT;

		if (bHilite || (mshow > 1 && SelCount > 1))
			Builder.AddSeparator();

		for (auto& i: items)
		{
			inplace::truncate_center(i, ScrX+1-6*2);
			const auto dx = Builder.AddText(i);
			dx->Flags = (SelCount <= 1 || mshow <= 1 ? DIF_CENTERTEXT : 0) | DIF_SHOWAMPERSAND;
			size_t index = Builder.GetLastID();
			end_hilite = index;
			if (!start_hilite)
				start_hilite = index;
		}

		Builder.AddOKCancel(mDBttn, lng::MCancel);
		Builder.SetId(Wipe ? DeleteWipeId : (Global->Opt->DeleteToRecycleBin ? DeleteRecycleId : DeleteFileFolderId));

		if (Wipe || !Global->Opt->DeleteToRecycleBin)
			Builder.SetDialogMode(DMODE_WARNINGSTYLE);

		if (!Builder.ShowDialog())
		{
			NeedUpdate = false;
			return;
		}
	}

	if (UpdateDiz)
		SrcPanel->ReadDiz();

	const auto strDizName = SrcPanel->GetDizName();
	const auto DizPresent = !strDizName.empty() && os::fs::exists(strDizName);

	NeedSetUpADir = CheckUpdateAnotherPanel(SrcPanel, SingleSelData.FileName);

	if (SrcPanel->GetType() == panel_type::TREE_PANEL)
		FarChDir(L"\\"sv);

	{
		ConsoleTitle::SetFarTitle(msg(lng::MDeletingTitle));
		SCOPED_ACTION(taskbar::indeterminate);
		SCOPED_ACTION(wakeful);
		bool Cancel=false;
		SetCursorType(false, 0);
		ReadOnlyDeleteMode=-1;
		m_SkipErrors = false;
		SkipWipeMode=-1;
		m_SkipFolderErrors = false;
		ProcessedItems=0;

		struct
		{
			size_t Items = 0;
			size_t Size = 0;
		}
		Total;

		if (Global->Opt->DelOpt.ShowTotal)
		{
			time_check TimeCheck(time_check::mode::delayed, GetRedrawTimeout());

			const auto DirInfoCallback = [&](string_view const Name, unsigned long long const ItemsCount, unsigned long long const Size)
			{
				if (TimeCheck)
					DirInfoMsg(msg(lng::MDeletingTitle), Name, Total.Items + ItemsCount, Total.Size + Size);
			};

			for (const auto& i: SrcPanel->enum_selected())
			{
				++Total.Items;

				if (i.Attributes & FILE_ATTRIBUTE_DIRECTORY && !os::fs::is_directory_symbolic_link(i))
				{
					DirInfoData Data = {};

					if (GetDirInfo(i.FileName, Data, nullptr, DirInfoCallback, 0) <= 0)
					{
						Cancel = true;
						break;
					}

					Total.Items += Data.FileCount + Data.DirCount;
					Total.Size += Data.FileSize;
				}
			}
		}

		os::fs::find_data SelFindData;
		SrcPanel->GetSelName(nullptr);
		const time_check TimeCheck(time_check::mode::immediate, GetRedrawTimeout());
		bool cannot_recycle_try_delete_folder = false;

		string strSelName;
		string strSelShortName;

		// BUGBUGBUG
		// TODO: enumerator
		while (!Cancel && (cannot_recycle_try_delete_folder || SrcPanel->GetSelName(&strSelName, &strSelShortName, &SelFindData)))
		{
			if (strSelName.empty() || IsRelativeRoot(strSelName) || IsRootPath(strSelName))
				continue;

			if (TimeCheck)
			{
				if (CheckForEscSilent() && ConfirmAbortOp())
				{
					Cancel=true;
					break;
				}

				ShellDeleteMsg(strSelName, Wipe?DEL_WIPE:DEL_DEL, { ProcessedItems, Total.Items }, 0);
			}

			if (SelFindData.Attributes & FILE_ATTRIBUTE_DIRECTORY)
			{
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
							if (Wipe) {
								tit = lng::MWipeFolderTitle; con = lng::MWipeFolderConfirm; del = lng::MDeleteFileWipe;
								guidId = &WipeFolderId;
							}
							else if (Global->Opt->DeleteToRecycleBin)
							{
								con = lng::MRecycleFolderConfirm; del = lng::MDeleteRecycle;
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

						if (MsgCode<0 || MsgCode==3)
						{
							NeedSetUpADir = false;
							break;
						}

						if (MsgCode==1)
							DeleteAllFolders = true;

						if (MsgCode==2)
							continue;
					}
				}

				if (!DirSymLink && (!Global->Opt->DeleteToRecycleBin || Wipe))
				{
					ScanTree ScTree(true, true, FALSE, false);

					const auto strSelFullName = IsAbsolutePath(strSelName)?
						strSelName :
						path::join(SrcPanel->GetCurDir(), strSelName);

					ScTree.SetFindPath(strSelFullName, L"*"sv);
					const time_check TreeTimeCheck(time_check::mode::immediate, GetRedrawTimeout());

					os::fs::find_data FindData;
					string strFullName;
					while (ScTree.GetNextName(FindData,strFullName))
					{
						if (TreeTimeCheck)
						{
							if (CheckForEscSilent())
							{
								int AbortOp = ConfirmAbortOp();

								if (AbortOp)
								{
									Cancel=true;
									break;
								}
							}

							ShellDeleteMsg(strFullName, Wipe?DEL_WIPE:DEL_DEL, { ProcessedItems, Total.Items }, 0);
						}

						if (FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY)
						{
							if (os::fs::is_directory_symbolic_link(FindData))
							{
								if (FindData.Attributes & FILE_ATTRIBUTE_READONLY)
									os::fs::set_file_attributes(strFullName,FILE_ATTRIBUTE_NORMAL);

								int MsgCode=ERemoveDirectory(strFullName, Wipe? D_WIPE : D_DEL);

								if (MsgCode==DELETE_CANCEL)
								{
									Cancel=true;
									break;
								}
								else if (MsgCode==DELETE_SKIP)
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
								const int MsgCode = Message(MSG_WARNING,
									msg(Wipe? lng::MWipeFolderTitle : lng::MDeleteFolderTitle),
									{
										msg(Wipe? lng::MWipeFolderConfirm : lng::MDeleteFolderConfirm),
										strFullName
									},
									{ Wipe? lng::MDeleteFileWipe : lng::MDeleteFileDelete, lng::MDeleteFileAll, lng::MDeleteFileSkip, lng::MDeleteFileCancel },
									{}, Wipe? &WipeFolderId : &DeleteFolderId); // ??? other GUID ???

								if (MsgCode<0 || MsgCode==3)
								{
									Cancel=true;
									break;
								}

								if (MsgCode==1)
									DeleteAllFolders = true;

								if (MsgCode==2)
								{
									ScTree.SkipDir();
									continue;
								}
							}

							if (ScTree.IsDirSearchDone())
							{
								if (FindData.Attributes & FILE_ATTRIBUTE_READONLY)
									os::fs::set_file_attributes(strFullName,FILE_ATTRIBUTE_NORMAL);

								int MsgCode=ERemoveDirectory(strFullName, Wipe? D_WIPE : D_DEL);

								if (MsgCode==DELETE_CANCEL)
								{
									Cancel=true;
									break;
								}
								else if (MsgCode==DELETE_SKIP)
								{
									//ScTree.SkipDir();
									continue;
								}

								TreeList::DelTreeName(strFullName);
							}
						}
						else
						{
							int AskCode=AskDeleteReadOnly(strFullName,FindData.Attributes,Wipe);

							if (AskCode==DELETE_CANCEL)
							{
								Cancel=true;
								break;
							}

							if (AskCode==DELETE_YES)
								if (ShellRemoveFile(strFullName, Wipe, { ProcessedItems, Total.Items }) == DELETE_CANCEL)
								{
									Cancel=true;
									break;
								}
						}
					}
				}

				if (!Cancel)
				{
					if (SelFindData.Attributes & FILE_ATTRIBUTE_READONLY)
						os::fs::set_file_attributes(strSelName,FILE_ATTRIBUTE_NORMAL);

					// нефига здесь выделываться, а надо учесть, что удаление
					// симлинка в корзину чревато потерей оригинала.
					DIRDELTYPE Type = Wipe ? D_WIPE : (Global->Opt->DeleteToRecycleBin && !(DirSymLink && !IsWindowsVistaOrGreater()) ? D_RECYCLE : D_DEL);
					const auto DeleteCode = ERemoveDirectory(strSelName, Type);

					if (cannot_recycle_try_delete_folder)
					{
						cannot_recycle_try_delete_folder = false;
						Global->Opt->DeleteToRecycleBin = true;
					}

					if (DeleteCode==DELETE_CANCEL)
						break;
					else if (DeleteCode==DELETE_SUCCESS)
					{
						TreeList::DelTreeName(strSelName);

						if (UpdateDiz)
							SrcPanel->DeleteDiz(strSelName,strSelShortName);
					}
					else if (DeleteCode == DELETE_YES)
					{
						--ProcessedItems;
						cannot_recycle_try_delete_folder = true;
						Global->Opt->DeleteToRecycleBin = false;
					}
				}
			}
			else
			{
				int AskCode=AskDeleteReadOnly(strSelName, SelFindData.Attributes, Wipe);

				if (AskCode==DELETE_CANCEL)
					break;

				if (AskCode==DELETE_YES)
				{
					int DeleteCode = ShellRemoveFile(strSelName, Wipe, { ProcessedItems, Total.Items });

					if (DeleteCode==DELETE_SUCCESS && UpdateDiz)
					{
						SrcPanel->DeleteDiz(strSelName,strSelShortName);
					}

					if (DeleteCode==DELETE_CANCEL)
						break;
				}
			}
		}
	}

	if (UpdateDiz)
		if (DizPresent==(!strDizName.empty() && os::fs::exists(strDizName)))
			SrcPanel->FlushDiz();
}

DEL_RESULT ShellDelete::AskDeleteReadOnly(const string& Name,DWORD Attr, bool Wipe)
{
	int MsgCode;

	if (!(Attr & FILE_ATTRIBUTE_READONLY))
		return DELETE_YES;

	if (!Global->Opt->Confirm.RO)
		ReadOnlyDeleteMode=1;

	if (ReadOnlyDeleteMode!=-1)
		MsgCode=ReadOnlyDeleteMode;
	else
	{
		MsgCode=Message(MSG_WARNING,
			msg(lng::MWarning),
			{
				msg(lng::MDeleteRO),
				Name,
				msg(Wipe? lng::MAskWipeRO : lng::MAskDeleteRO)
			},
			{ Wipe? lng::MDeleteFileWipe : lng::MDeleteFileDelete, lng::MDeleteFileAll, lng::MDeleteFileSkip, lng::MDeleteFileSkipAll, lng::MDeleteFileCancel },
			{}, Wipe? &DeleteAskWipeROId : &DeleteAskDeleteROId);
	}

	switch (MsgCode)
	{
		case 1:
			ReadOnlyDeleteMode=1;
			break;
		case 2:
			return DELETE_SKIP;
		case 3:
			ReadOnlyDeleteMode=3;
			return DELETE_SKIP;
		case -1:
		case -2:
		case 4:
			return DELETE_CANCEL;
	}

	os::fs::set_file_attributes(Name,FILE_ATTRIBUTE_NORMAL);
	return DELETE_YES;
}

DEL_RESULT ShellDelete::ShellRemoveFile(const string& Name, bool Wipe, progress Files)
{
	ProcessedItems++;
	const auto strFullName = ConvertNameToFull(Name);

	for (;;)
	{
		bool recycle_bin = false;
		if (Wipe)
		{
			int MsgCode = 0;
			if (SkipWipeMode!=-1)
			{
				MsgCode=SkipWipeMode;
			}
			else if (const auto Hardlinks = GetNumberOfLinks(strFullName); Hardlinks && *Hardlinks > 1)
			{
				/*
				                            Файл
				                         "имя файла"
				                Файл имеет несколько жестких ссылок.
				  Уничтожение файла приведет к обнулению всех ссылающихся на него файлов.
				                        Уничтожать файл?
				*/
				MsgCode=Message(MSG_WARNING,
					msg(lng::MError),
					{
						strFullName,
						msg(lng::MDeleteHardLink1),
						msg(lng::MDeleteHardLink2),
						msg(lng::MDeleteHardLink3)
					},
					{ lng::MDeleteFileWipe, lng::MDeleteFileAll, lng::MDeleteFileSkip, lng::MDeleteFileSkipAll, lng::MDeleteCancel },
					{}, &WipeHardLinkId);
			}

			switch (MsgCode)
			{
			case 4: case -1: case -2:
				return DELETE_CANCEL;
			case 3:
				SkipWipeMode = 2;
				[[fallthrough]];
			case 2:
				return DELETE_SKIP;
			case 1:
				SkipWipeMode = 0;
				[[fallthrough]];
			case 0:
				{
					bool Cancel = false;
					if (WipeFile(strFullName, Files, Cancel))
						return DELETE_SUCCESS;
					else if(Cancel)
						return DELETE_CANCEL;
				}
			}
		}
		else if (!Global->Opt->DeleteToRecycleBin)
		{
			if (os::fs::delete_file(strFullName))
				break;
		}
		else
		{
			recycle_bin = true;
			DEL_RESULT ret;
			if (RemoveToRecycleBin(strFullName, false, ret))
				break;

			if (!m_SkipErrors && (ret == DELETE_SKIP || ret == DELETE_CANCEL))
				return ret;

			if (!m_SkipErrors && ret == DELETE_YES)
			{
				recycle_bin = false;
				if (os::fs::delete_file(strFullName))
					break;
			}
		}

		operation MsgCode;

		if (!m_SkipErrors)
		{
			const auto ErrorState = error_state::fetch();

			MsgCode = OperationFailed(ErrorState, strFullName, lng::MError, msg(recycle_bin ? lng::MCannotRecycleFile : lng::MCannotDeleteFile));
		}
		else
		{
			MsgCode = operation::skip;
		}

		switch (MsgCode)
		{
		case operation::retry:
			break;

		case operation::skip:
			return DELETE_SKIP;

		case operation::skip_all:
			m_SkipErrors = true;
			return DELETE_SKIP;

		case operation::cancel:
			return DELETE_CANCEL;
		}
	}

	return DELETE_SUCCESS;
}

DEL_RESULT ShellDelete::ERemoveDirectory(const string& Name,DIRDELTYPE Type)
{
	ProcessedItems++;

	bool Success = false;
	while(!Success)
	{
		bool recycle_bin = false;
		switch(Type)
		{
		case D_DEL:
			Success = os::fs::remove_directory(Name);
			break;

		case D_WIPE:
			Success = WipeDirectory(Name) != FALSE;
			break;

		case D_RECYCLE:
			{
				recycle_bin = true;
				DEL_RESULT ret;
				Success = RemoveToRecycleBin(Name, true, ret);

				if (!Success && !m_SkipFolderErrors && ret >= DELETE_YES)
					return ret; // DELETE_YES, DELETE_SKIP, DELETE_CANCEL
			}
			break;
		}

		if(!Success)
		{
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

			case operation::skip:
				return DELETE_SKIP;

			case operation::skip_all:
				m_SkipFolderErrors = true;
				return DELETE_SKIP;

			case operation::cancel:
				return DELETE_CANCEL;
			}
		}
	}

	return DELETE_SUCCESS;
}

bool ShellDelete::RemoveToRecycleBin(const string& Name, bool dir, DEL_RESULT& ret)
{
	auto strFullName = ConvertNameToFull(Name);

	// При удалении в корзину папки с симлинками получим траблу, если предварительно линки не убрать.
	if (!IsWindowsVistaOrGreater() && os::fs::is_directory(Name))
	{
		string strFullName2;
		os::fs::find_data FindData;
		ScanTree ScTree(true, true, FALSE, false);
		ScTree.SetFindPath(Name, L"*"sv);

		bool MessageShown = false;
		bool SkipErrors = false;

		while (ScTree.GetNextName(FindData,strFullName2))
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
						ret = DELETE_CANCEL;
						return false;
					}
				}
				// BUGBUG, check result
				(void)EDeleteReparsePoint(strFullName2, FindData.Attributes, SkipErrors);
			}
		}
	}

	if (os::fs::move_to_recycle_bin(strFullName))
	{
		ret = DELETE_SUCCESS;
		return true;
	}

	if (dir? m_SkipFolderErrors : m_SkipErrors)
	{
		ret = DELETE_SKIP;
		return false;
	}

	ret = DELETE_SUCCESS;
	DWORD dwe = GetLastError(); // probably bad path to recycle bin
	if (ERROR_BAD_PATHNAME == dwe || ERROR_FILE_NOT_FOUND == dwe || (dir && ERROR_PATH_NOT_FOUND==dwe))
	{
		const auto ErrorState = error_state::fetch();

		const int MsgCode = Message(MSG_WARNING, ErrorState,
			msg(lng::MError),
			{
				msg(dir? lng::MCannotRecycleFolder : lng::MCannotRecycleFile),
				QuoteOuterSpace(strFullName)
			},
			{ lng::MDeleteFileDelete, lng::MDeleteSkip, lng::MDeleteSkipAll, lng::MDeleteCancel },
			{}, dir? &CannotRecycleFolderId : &CannotRecycleFileId);

		switch (MsgCode) {
		case 3: case -1: case -2:       // [Cancel]
			ret = DELETE_CANCEL;
			break;
		case 2:                         // [Skip All]
			(dir? m_SkipFolderErrors : m_SkipErrors) = true;
			[[fallthrough]];
		case 1:                         // [Skip]
			ret =  DELETE_SKIP;
			break;
		case 0:                         // {Delete}
			ret = DELETE_YES;
			break;
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
	ScTree.SetFindPath(Dir, L"*"sv	);

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

void Delete(const panel_ptr& SrcPanel, bool const Wipe)
{
	ShellDelete(SrcPanel, Wipe);
}
