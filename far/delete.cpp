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

#include "headers.hpp"
#pragma hdrstop

#include "delete.hpp"
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
#include "datetime.hpp"
#include "strmix.hpp"
#include "DlgGuid.hpp"
#include "cvtname.hpp"
#include "fileattr.hpp"

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

struct DelPreRedrawItem : public PreRedrawItem
{
	DelPreRedrawItem():
		PreRedrawItem(PR_ShellDeleteMsg),
		Mode(),
		Percent(),
		WipePercent()
	{}

	string name;
	DEL_MODE Mode;
	int Percent;
	int WipePercent;
};

static void ShellDeleteMsg(const string& Name, DEL_MODE Mode, int Percent, int WipePercent)
{
	string strProgress, strWipeProgress;
	size_t Width=ScrX/2;
	if(Mode==DEL_WIPEPROCESS || Mode==DEL_WIPE)
	{
		strWipeProgress = make_progressbar(Width, WipePercent, true, Percent == -1);
	}

	if (Mode!=DEL_SCAN && Percent!=-1)
	{
		strProgress = make_progressbar(Width, Percent, true, true);
		ConsoleTitle::SetFarTitle(concat(L'{', str(Percent), L"%} "_sv, msg(Mode == DEL_WIPE || Mode == DEL_WIPEPROCESS? lng::MDeleteWipeTitle : lng::MDeleteTitle)));
	}

	auto strOutFileName = Name;
	TruncPathStr(strOutFileName,static_cast<int>(Width));
	inplace::fit_to_center(strOutFileName, Width);

	{
		std::vector<string> MsgItems =
		{
			msg(Mode == DEL_SCAN ? lng::MScanningFolder : (Mode == DEL_WIPE || Mode == DEL_WIPEPROCESS) ? lng::MDeletingWiping : lng::MDeleting),
			strOutFileName
		};

		if (!strWipeProgress.empty())
			MsgItems.emplace_back(strWipeProgress);

		if (!strProgress.empty())
			MsgItems.emplace_back(strProgress);

		Message(0,
			msg((Mode == DEL_WIPE || Mode == DEL_WIPEPROCESS) ? lng::MDeleteWipeTitle : lng::MDeleteTitle),
			std::move(MsgItems),
			{});
	}
	if (!PreRedrawStack().empty())
	{
		const auto item = dynamic_cast<DelPreRedrawItem*>(PreRedrawStack().top());
		assert(item);
		if (item)
		{
			item->name = Name;
			item->Mode = Mode;
			item->Percent = Percent;
			item->WipePercent = WipePercent;
		}
	}
}

static void PR_ShellDeleteMsg()
{
	if (!PreRedrawStack().empty())
	{
		const auto item = dynamic_cast<const DelPreRedrawItem*>(PreRedrawStack().top());
		assert(item);
		if (item)
		{
			ShellDeleteMsg(item->name, item->Mode, item->Percent, item->WipePercent);
		}
	}
}

static DWORD SHErrorToWinError(DWORD SHError)
{
	switch (SHError)
	{
	case 0x71:    return ERROR_ALREADY_EXISTS;    // DE_SAMEFILE            The source and destination files are the same file.
	case 0x72:    return ERROR_INVALID_PARAMETER; // DE_MANYSRC1DEST        Multiple file paths were specified in the source buffer, but only one destination file path.
	case 0x73:    return ERROR_NOT_SAME_DEVICE;   // DE_DIFFDIR             Rename operation was specified but the destination path is a different directory. Use the move operation instead.
	case 0x74:    return ERROR_INVALID_PARAMETER; // DE_ROOTDIR             The source is a root directory, which cannot be moved or renamed.
	case 0x75:    return ERROR_CANCELLED;         // DE_OPCANCELLED         The operation was canceled by the user, or silently canceled if the appropriate flags were supplied to SHFileOperation.
	case 0x76:    return ERROR_BAD_PATHNAME;      // DE_DESTSUBTREE         The destination is a subtree of the source.
	case 0x78:    return ERROR_ACCESS_DENIED;     // DE_ACCESSDENIEDSRC     Security settings denied access to the source.
	case 0x79:    return ERROR_BUFFER_OVERFLOW;   // DE_PATHTOODEEP         The source or destination path exceeded or would exceed MAX_PATH.
	case 0x7A:    return ERROR_INVALID_PARAMETER; // DE_MANYDEST            The operation involved multiple destination paths, which can fail in the case of a move operation.
	case 0x7C:    return ERROR_BAD_PATHNAME;      // DE_INVALIDFILES        The path in the source or destination or both was invalid.
	case 0x7D:    return ERROR_INVALID_PARAMETER; // DE_DESTSAMETREE        The source and destination have the same parent folder.
	case 0x7E:    return ERROR_ALREADY_EXISTS;    // DE_FLDDESTISFILE       The destination path is an existing file.
	case 0x80:    return ERROR_ALREADY_EXISTS;    // DE_FILEDESTISFLD       The destination path is an existing folder.
	case 0x81:    return ERROR_BUFFER_OVERFLOW;   // DE_FILENAMETOOLONG     The name of the file exceeds MAX_PATH.
	case 0x82:    return ERROR_WRITE_FAULT;       // DE_DEST_IS_CDROM       The destination is a read-only CD-ROM, possibly unformatted.
	case 0x83:    return ERROR_WRITE_FAULT;       // DE_DEST_IS_DVD         The destination is a read-only DVD, possibly unformatted.
	case 0x84:    return ERROR_WRITE_FAULT;       // DE_DEST_IS_CDRECORD    The destination is a writable CD-ROM, possibly unformatted.
	case 0x85:    return ERROR_DISK_FULL;         // DE_FILE_TOO_LARGE      The file involved in the operation is too large for the destination media or file system.
	case 0x86:    return ERROR_READ_FAULT;        // DE_SRC_IS_CDROM        The source is a read-only CD-ROM, possibly unformatted.
	case 0x87:    return ERROR_READ_FAULT;        // DE_SRC_IS_DVD          The source is a read-only DVD, possibly unformatted.
	case 0x88:    return ERROR_READ_FAULT;        // DE_SRC_IS_CDRECORD     The source is a writable CD-ROM, possibly unformatted.
	case 0xB7:    return ERROR_BUFFER_OVERFLOW;   // DE_ERROR_MAX           MAX_PATH was exceeded during the operation.
	case 0x402:   return ERROR_PATH_NOT_FOUND;    //                        An unknown error occurred. This is typically due to an invalid path in the source or destination. This error does not occur on Windows Vista and later.
	case 0x10000: return ERROR_GEN_FAILURE;       // ERRORONDEST            An unspecified error occurred on the destination.
	case 0x10074: return ERROR_INVALID_PARAMETER; // DE_ROOTDIR|ERRORONDEST Destination is a root directory and cannot be renamed.
	default:      return SHError;
	}
}

static bool MoveToRecycleBinInternal(const string& Objects)
{
	SHFILEOPSTRUCT fop={};
	fop.wFunc=FO_DELETE;
	fop.pFrom = Objects.data();
	fop.pTo = L"\0\0";
	fop.fFlags=FOF_NOCONFIRMATION|FOF_SILENT|FOF_ALLOWUNDO;

	auto Result = SHErrorToWinError(SHFileOperation(&fop));

	if (Result == ERROR_ACCESS_DENIED && Global->Opt->ElevationMode&ELEVATION_MODIFY_REQUEST) // Achtung! ShellAPI doesn't set LastNtStatus, so don't use ElevationRequired() here.
		Result = SHErrorToWinError(elevation::instance().fMoveToRecycleBin(fop));

	SetLastError(Result);

	return Result == ERROR_SUCCESS && !fop.fAnyOperationsAborted;
}

static bool WipeFileData(const string& Name, int TotalPercent, bool& Cancel)
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

			ShellDeleteMsg(Name, DEL_WIPEPROCESS, TotalPercent, WipeFile.GetPercent());
		}
	}
	while(WipeFile.Step());

	if (!WipeFile.SetPointer(0, nullptr, FILE_BEGIN))
		return false;

	if (!WipeFile.SetEnd())
		return false;

	return true;
}

static bool WipeFile(const string& Name, int TotalPercent, bool& Cancel)
{
	if (!os::fs::set_file_attributes(Name, FILE_ATTRIBUTE_NORMAL))
		return false;

	if (!WipeFileData(Name, TotalPercent, Cancel))
		return false;

	string strTempName;
	if (!FarMkTempEx(strTempName, {}, false))
		return false;

	if (!os::fs::move_file(Name, strTempName))
		return false;;

	return os::fs::delete_file(strTempName);
}

static bool WipeDirectory(const string& Name)
{
	string strTempName, strPath = Name;

	if (!CutToParent(strPath))
	{
		strPath.clear();
	}

	FarMkTempEx(strTempName, {}, false, strPath);

	if (!os::fs::move_file(Name, strTempName))
	{
		return false;
	}

	return os::fs::remove_directory(strTempName);
}

ShellDelete::ShellDelete(panel_ptr SrcPanel, bool Wipe):
	ReadOnlyDeleteMode(-1),
	m_SkipMode(-1),
	SkipWipeMode(-1),
	SkipFoldersMode(-1),
	ProcessedItems(0)
{
	SCOPED_ACTION(ChangePriority)(Global->Opt->DelThreadPriority);
	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<DelPreRedrawItem>());
	string strDeleteFilesMsg;
	string strSelName;
	string strSelShortName;
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

	DWORD FileAttr;
	// Удаление в корзину только для  FIXED-дисков
	{
		SrcPanel->GetSelName(nullptr,FileAttr);
		SrcPanel->GetSelName(&strSelName,FileAttr);

		if (Global->Opt->DeleteToRecycleBin && FAR_GetDriveType(GetPathRoot(ConvertNameToFull(strSelName))) != DRIVE_FIXED)
			Global->Opt->DeleteToRecycleBin = false;
	}

	if (SelCount==1)
	{
		SrcPanel->GetSelName(nullptr,FileAttr);
		SrcPanel->GetSelName(&strSelName,FileAttr);

		if (TestParentFolderName(strSelName) || strSelName.empty())
		{
			NeedUpdate = false;
			return;
		}

		strDeleteFilesMsg = strSelName;
		QuoteOuterSpace(strDeleteFilesMsg);
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
		strDeleteFilesMsg = format(lng::MAskDeleteItems, SelCount, Ends);
	}

	int Ret = 1;

	//   Обработка "удаления" линков
	if ((FileAttr & FILE_ATTRIBUTE_REPARSE_POINT) && SelCount==1)
	{
		auto strJuncName = ConvertNameToFull(strSelName);

		if (GetReparsePointInfo(strJuncName, strJuncName)) // ? SelName ?
		{
			NormalizeSymlinkName(strJuncName);
			string strAskDeleteLink=msg(lng::MAskDeleteLink);
			os::fs::file_status Status(strJuncName);
			if (os::fs::exists(Status))
			{
				strAskDeleteLink += L' ';
				strAskDeleteLink += msg(is_directory(Status)? lng::MAskDeleteLinkFolder : lng::MAskDeleteLinkFile);
			}

			Ret=Message(0,
				msg(lng::MDeleteLinkTitle),
				{
					strDeleteFilesMsg,
					strAskDeleteLink,
					strJuncName
				},
				{ lng::MDeleteLinkDelete, lng::MCancel },
				nullptr, &DeleteLinkId);

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
		const auto mshow = std::min(std::max((int)Global->Opt->DelOpt.ShowSelected, 1), ScrY / 2);

		std::vector<string> items{ strDeleteFilesMsg };

		if (SelCount == 1)
		{
			bool folder = (FileAttr & FILE_ATTRIBUTE_DIRECTORY) != 0;

			if (Wipe && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
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
				SrcPanel->GetCurName(name, sname);
				QuoteOuterSpace(name);
				bHilite = strDeleteFilesMsg != name;
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
				DWORD attr;
				string name;
				SrcPanel->GetSelName(nullptr, attr);

				for (size_t i = 0; i < SelCount; ++i)
				{
					if (i == (size_t)mshow-1 && i+1 < SelCount)
					{
						items.emplace_back(L"...");
						break;
					}
					SrcPanel->GetSelName(&name, attr);
					QuoteOuterSpace(name);
					items.emplace_back(name);
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

		Builder.AddText(tText.data())->Flags = DIF_CENTERTEXT;

		if (bHilite || (mshow > 1 && SelCount > 1))
			Builder.AddSeparator();

		std::for_each(RANGE(items, i)
		{
			TruncStrFromCenter(i, ScrX+1-6*2);
			const auto dx = Builder.AddText(i.data());
			dx->Flags = (SelCount <= 1 || mshow <= 1 ? DIF_CENTERTEXT : 0) | DIF_SHOWAMPERSAND;
			size_t index = Builder.GetLastID();
			end_hilite = index;
			if (!start_hilite)
				start_hilite = index;
		});

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

	NeedSetUpADir = CheckUpdateAnotherPanel(SrcPanel, strSelName);

	if (SrcPanel->GetType() == panel_type::TREE_PANEL)
		FarChDir(L"\\");

	{
		ConsoleTitle::SetFarTitle(msg(lng::MDeletingTitle));
		SCOPED_ACTION(IndeterminateTaskbar);
		SCOPED_ACTION(wakeful);
		bool Cancel=false;
		SetCursorType(false, 0);
		ReadOnlyDeleteMode=-1;
		m_SkipMode=-1;
		SkipWipeMode=-1;
		SkipFoldersMode=-1;
		ULONG ItemsCount=0;
		ProcessedItems=0;

		if (Global->Opt->DelOpt.ShowTotal)
		{
			SrcPanel->GetSelName(nullptr,FileAttr);
			auto MessageDelay = getdirinfo_default_delay;

			os::fs::find_data SelFindData;
			while (SrcPanel->GetSelName(&strSelName,FileAttr,&strSelShortName, &SelFindData) && !Cancel)
			{
				++ItemsCount;

				if (FileAttr&FILE_ATTRIBUTE_DIRECTORY && !os::fs::is_directory_symbolic_link(SelFindData))
				{
					DirInfoData Data = {};

					if (GetDirInfo(msg(lng::MDeletingTitle), strSelName, Data, MessageDelay, nullptr, GETDIRINFO_NOREDRAW) > 0)
					{
						ItemsCount+=Data.FileCount+Data.DirCount+1;
					}
					else
					{
						Cancel=true;
					}
					MessageDelay = getdirinfo_no_delay;
				}
			}
		}

		os::fs::find_data SelFindData;
		SrcPanel->GetSelName(nullptr, FileAttr);
		const time_check TimeCheck(time_check::mode::immediate, GetRedrawTimeout());
		bool cannot_recycle_try_delete_folder = false;

		while (!Cancel && (cannot_recycle_try_delete_folder || SrcPanel->GetSelName(&strSelName,FileAttr,&strSelShortName, &SelFindData)))
		{
			if (strSelName.empty() || IsRelativeRoot(strSelName) || IsRootPath(strSelName))
				continue;

			int TotalPercent = (Global->Opt->DelOpt.ShowTotal && ItemsCount >1)?(ProcessedItems*100/ItemsCount):-1;
			if (TimeCheck)
			{
				if (CheckForEscSilent() && ConfirmAbortOp())
				{
					Cancel=true;
					break;
				}

				ShellDeleteMsg(strSelName, Wipe?DEL_WIPE:DEL_DEL, TotalPercent, 0);
			}

			if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
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
								nullptr, guidId);
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
					ScanTree ScTree(true, true, FALSE);

					const auto strSelFullName = IsAbsolutePath(strSelName)?
						strSelName :
						path::join(SrcPanel->GetCurDir(), strSelName);

					ScTree.SetFindPath(strSelFullName,L"*", 0);
					const time_check TreeTimeCheck(time_check::mode::immediate, GetRedrawTimeout());

					os::fs::find_data FindData;
					string strFullName;
					while (ScTree.GetNextName(FindData,strFullName))
					{
						int TreeTotalPercent = (Global->Opt->DelOpt.ShowTotal && ItemsCount >1)?(ProcessedItems*100/ItemsCount):-1;
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

							ShellDeleteMsg(strFullName,Wipe?DEL_WIPE:DEL_DEL, TreeTotalPercent, 0);
						}

						if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						{
							if (os::fs::is_directory_symbolic_link(FindData))
							{
								if (FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
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
									nullptr, Wipe? &WipeFolderId : &DeleteFolderId); // ??? other GUID ???

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
								if (FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
									os::fs::set_file_attributes(strFullName,FILE_ATTRIBUTE_NORMAL);

								int MsgCode=ERemoveDirectory(strFullName, Wipe? D_WIPE : D_DEL);

								if (MsgCode==DELETE_CANCEL)
								{
									Cancel=true;;
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
							int AskCode=AskDeleteReadOnly(strFullName,FindData.dwFileAttributes,Wipe);

							if (AskCode==DELETE_CANCEL)
							{
								Cancel=true;
								break;
							}

							if (AskCode==DELETE_YES)
								if (ShellRemoveFile(strFullName, Wipe, TreeTotalPercent) == DELETE_CANCEL)
								{
									Cancel=true;
									break;
								}
						}
					}
				}

				if (!Cancel)
				{
					if (FileAttr & FILE_ATTRIBUTE_READONLY)
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
				int AskCode=AskDeleteReadOnly(strSelName,FileAttr,Wipe);

				if (AskCode==DELETE_CANCEL)
					break;

				if (AskCode==DELETE_YES)
				{
					int DeleteCode = ShellRemoveFile(strSelName, Wipe, TotalPercent);

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
			nullptr, Wipe? &DeleteAskWipeROId : &DeleteAskDeleteROId);
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

DEL_RESULT ShellDelete::ShellRemoveFile(const string& Name, bool Wipe, int TotalPercent)
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
			else if (GetNumberOfLinks(strFullName)>1)
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
					nullptr, &WipeHardLinkId);
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
					if (WipeFile(strFullName, TotalPercent, Cancel))
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

			if (m_SkipMode == -1 && (ret == DELETE_SKIP || ret == DELETE_CANCEL))
				return ret;

			if (m_SkipMode == -1 && ret == DELETE_YES)
			{
				recycle_bin = false;
				if (os::fs::delete_file(strFullName))
					break;
			}
		}

		operation MsgCode;

		if (m_SkipMode != -1)
			MsgCode = static_cast<operation>(m_SkipMode);
		else
		{
			const auto ErrorState = error_state::fetch();

			MsgCode = OperationFailed(ErrorState, strFullName, lng::MError, msg(recycle_bin ? lng::MCannotRecycleFile : lng::MCannotDeleteFile));
		}

		switch (static_cast<operation>(MsgCode))
		{
		case operation::retry:
			break;

		case operation::skip:
			return DELETE_SKIP;

		case operation::skip_all:
			m_SkipMode = static_cast<int>(operation::skip);
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

				if (!Success && SkipFoldersMode == -1 && ret >= DELETE_YES)
					return ret; // DELETE_YES, DELETE_SKIP, DELETE_CANCEL
			}
			break;
		}

		if(!Success)
		{
			operation MsgCode;

			if (SkipFoldersMode!=-1)
			{
				MsgCode = static_cast<operation>(SkipFoldersMode);
			}
			else
			{
				const auto ErrorState = error_state::fetch();

				MsgCode = OperationFailed(ErrorState, Name, lng::MError, msg(recycle_bin? lng::MCannotRecycleFolder : lng::MCannotDeleteFolder));
			}

			switch (MsgCode)
			{
			case operation::retry:
				break;

			case operation::skip:
				return DELETE_SKIP;

			case operation::skip_all:
				SkipFoldersMode = static_cast<int>(operation::skip);
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
		ScanTree ScTree(true, true, FALSE);
		ScTree.SetFindPath(Name,L"*", 0);

		bool MessageShown = false;
		int SkipMode = -1;
		
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
						nullptr, &RecycleFolderConfirmDeleteLinkId
						) != Message::first_button)
					{
						ret = DELETE_CANCEL;
						return false;
					}
				}
				EDeleteReparsePoint(strFullName2, FindData.dwFileAttributes, SkipMode);
			}
		}
	}

	strFullName.push_back(L'\0'); // make strFullName end with DOUBLE zero

	if (MoveToRecycleBinInternal(strFullName))
	{
		ret = DELETE_SUCCESS;
		return true;
	}
	if ((dir && SkipFoldersMode != -1) || (!dir && m_SkipMode != -1))
	{
		ret = DELETE_SKIP;
		return false;
	}

	ret = DELETE_SUCCESS;
	DWORD dwe = GetLastError(); // probably bad path to recycle bin
	if (ERROR_BAD_PATHNAME == dwe || ERROR_FILE_NOT_FOUND == dwe || (dir && ERROR_PATH_NOT_FOUND==dwe))
	{
		const auto ErrorState = error_state::fetch();

		string qName(strFullName);
		QuoteOuterSpace(qName);

		int MsgCode = Message(MSG_WARNING, ErrorState,
			msg(lng::MError),
			{
				msg(dir? lng::MCannotRecycleFolder : lng::MCannotRecycleFile),
				qName
			},
			{ lng::MDeleteFileDelete, lng::MDeleteSkip, lng::MDeleteSkipAll, lng::MDeleteCancel },
			nullptr, dir? &CannotRecycleFolderId : &CannotRecycleFileId);

		switch (MsgCode) {
		case 3: case -1: case -2:       // [Cancel]
			ret = DELETE_CANCEL;
			break;
		case 2:                         // [Skip All]
			if (dir)
				SkipFoldersMode = 2;
			else
				m_SkipMode = 2;
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
	ScanTree ScTree(true, true, FALSE);
	ScTree.SetFindPath(Dir,L"*",0);

	while (ScTree.GetNextName(FindData, strFullName))
	{
		os::fs::set_file_attributes(strFullName,FILE_ATTRIBUTE_NORMAL);

		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (ScTree.IsDirSearchDone())
				os::fs::remove_directory(strFullName);
		}
		else
			os::fs::delete_file(strFullName);
	}

	os::fs::set_file_attributes(Dir,FILE_ATTRIBUTE_NORMAL);
	os::fs::remove_directory(Dir);
}

bool DeleteFileWithFolder(const string& FileName)
{
	auto strFileOrFolderName = unquote(FileName);
	os::fs::set_file_attributes(strFileOrFolderName, FILE_ATTRIBUTE_NORMAL);

	if (!os::fs::delete_file(strFileOrFolderName))
		return false;

	return CutToParent(strFileOrFolderName) && os::fs::remove_directory(strFileOrFolderName);
}

delayed_deleter::delayed_deleter(string pathToDelete):
	m_pathToDelete(std::move(pathToDelete))
{
}

delayed_deleter::~delayed_deleter()
{
	DeleteFileWithFolder(m_pathToDelete);
}

