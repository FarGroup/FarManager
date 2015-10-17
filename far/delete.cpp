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
#include "filepanels.hpp"
#include "scantree.hpp"
#include "treelist.hpp"
#include "manager.hpp"
#include "constitle.hpp"
#include "TPreRedrawFunc.hpp"
#include "TaskBar.hpp"
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
#include "language.hpp"
#include "FarDlgBuilder.hpp"
#include "datetime.hpp"
#include "strmix.hpp"
#include "DlgGuid.hpp"

enum DEL_MODE
{
	DEL_SCAN,
	DEL_DEL,
	DEL_WIPE,
	DEL_WIPEPROCESS
};

ENUM(DIRDELTYPE)
{
	D_DEL,
	D_RECYCLE,
	D_WIPE,
};

ENUM(DEL_RESULT)
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
		Title(),
		Mode(),
		Percent(),
		WipePercent()
	{}

	string name;
	ConsoleTitle* Title;
	DEL_MODE Mode;
	int Percent;
	int WipePercent;
};

static void ShellDeleteMsg(const string& Name, DEL_MODE Mode, int Percent, int WipePercent, ConsoleTitle* DeleteTitle)
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
		*DeleteTitle << L"{" << Percent << L"%} " << MSG((Mode==DEL_WIPE || Mode==DEL_WIPEPROCESS)?MDeleteWipeTitle:MDeleteTitle) << fmt::Flush();
	}

	string strOutFileName(Name);
	TruncPathStr(strOutFileName,static_cast<int>(Width));
	CenterStr(strOutFileName,strOutFileName,static_cast<int>(Width));
	const wchar_t* Progress1 = nullptr;
	const wchar_t* Progress2 = nullptr;
	if(!strWipeProgress.empty())
	{
		Progress1 = strWipeProgress.data();
		Progress2 = strProgress.empty()? nullptr : strProgress.data();
	}
	else
	{
		Progress1 = strProgress.empty()? nullptr : strProgress.data();
	}
	Message(0,0,
		MSG((Mode==DEL_WIPE || Mode==DEL_WIPEPROCESS)?MDeleteWipeTitle:MDeleteTitle),
		Mode==DEL_SCAN? MSG(MScanningFolder) : MSG((Mode==DEL_WIPE || Mode==DEL_WIPEPROCESS)?MDeletingWiping:MDeleting),
		strOutFileName.data(), Progress1, Progress2);

	if (!PreRedrawStack().empty())
	{
		auto item = dynamic_cast<DelPreRedrawItem*>(PreRedrawStack().top());
		item->name = Name;
		item->Title = DeleteTitle;
		item->Mode = Mode;
		item->Percent = Percent;
		item->WipePercent = WipePercent;
	}
}

static void PR_ShellDeleteMsg()
{
	if (!PreRedrawStack().empty())
	{
		const auto item = dynamic_cast<const DelPreRedrawItem*>(PreRedrawStack().top());
		ShellDeleteMsg(item->name, item->Mode, item->Percent, item->WipePercent, item->Title);
	}
}

static DWORD SHErrorToWinError(DWORD SHError)
{
	DWORD WinError=SHError;

	switch (SHError)
	{
		case 0x71:    WinError=ERROR_ALREADY_EXISTS;    break; // DE_SAMEFILE         The source and destination files are the same file.
		case 0x72:    WinError=ERROR_INVALID_PARAMETER; break; // DE_MANYSRC1DEST     Multiple file paths were specified in the source buffer, but only one destination file path.
		case 0x73:    WinError=ERROR_NOT_SAME_DEVICE;   break; // DE_DIFFDIR          Rename operation was specified but the destination path is a different directory. Use the move operation instead.
		case 0x74:    WinError=ERROR_INVALID_PARAMETER; break; // DE_ROOTDIR          The source is a root directory, which cannot be moved or renamed.
		case 0x75:    WinError=ERROR_CANCELLED;         break; // DE_OPCANCELLED      The operation was cancelled by the user, or silently cancelled if the appropriate flags were supplied to SHFileOperation.
		case 0x76:    WinError=ERROR_BAD_PATHNAME;      break; // DE_DESTSUBTREE      The destination is a subtree of the source.
		case 0x78:    WinError=ERROR_ACCESS_DENIED;     break; // DE_ACCESSDENIEDSRC  Security settings denied access to the source.
		case 0x79:    WinError=ERROR_BUFFER_OVERFLOW;   break; // DE_PATHTOODEEP      The source or destination path exceeded or would exceed MAX_PATH.
		case 0x7A:    WinError=ERROR_INVALID_PARAMETER; break; // DE_MANYDEST         The operation involved multiple destination paths, which can fail in the case of a move operation.
		case 0x7C:    WinError=ERROR_BAD_PATHNAME;      break; // DE_INVALIDFILES     The path in the source or destination or both was invalid.
		case 0x7D:    WinError=ERROR_INVALID_PARAMETER; break; // DE_DESTSAMETREE     The source and destination have the same parent folder.
		case 0x7E:    WinError=ERROR_ALREADY_EXISTS;    break; // DE_FLDDESTISFILE    The destination path is an existing file.
		case 0x80:    WinError=ERROR_ALREADY_EXISTS;    break; // DE_FILEDESTISFLD    The destination path is an existing folder.
		case 0x81:    WinError=ERROR_BUFFER_OVERFLOW;   break; // DE_FILENAMETOOLONG  The name of the file exceeds MAX_PATH.
		case 0x82:    WinError=ERROR_WRITE_FAULT;       break; // DE_DEST_IS_CDROM    The destination is a read-only CD-ROM, possibly unformatted.
		case 0x83:    WinError=ERROR_WRITE_FAULT;       break; // DE_DEST_IS_DVD      The destination is a read-only DVD, possibly unformatted.
		case 0x84:    WinError=ERROR_WRITE_FAULT;       break; // DE_DEST_IS_CDRECORD The destination is a writable CD-ROM, possibly unformatted.
		case 0x85:    WinError=ERROR_DISK_FULL;         break; // DE_FILE_TOO_LARGE   The file involved in the operation is too large for the destination media or file system.
		case 0x86:    WinError=ERROR_READ_FAULT;        break; // DE_SRC_IS_CDROM     The source is a read-only CD-ROM, possibly unformatted.
		case 0x87:    WinError=ERROR_READ_FAULT;        break; // DE_SRC_IS_DVD       The source is a read-only DVD, possibly unformatted.
		case 0x88:    WinError=ERROR_READ_FAULT;        break; // DE_SRC_IS_CDRECORD  The source is a writable CD-ROM, possibly unformatted.
		case 0xB7:    WinError=ERROR_BUFFER_OVERFLOW;   break; // DE_ERROR_MAX        MAX_PATH was exceeded during the operation.
		case 0x402:   WinError=ERROR_PATH_NOT_FOUND;    break; //                     An unknown error occurred. This is typically due to an invalid path in the source or destination. This error does not occur on Windows Vista and later.
		case 0x10000: WinError=ERROR_GEN_FAILURE;       break; // ERRORONDEST         An unspecified error occurred on the destination.
	}

	return WinError;
}

static bool MoveToRecycleBinInternal(LPCWSTR Object)
{
	SHFILEOPSTRUCT fop={};
	fop.wFunc=FO_DELETE;
	fop.pFrom=Object;
	fop.pTo = L"\0\0";
	fop.fFlags=FOF_NOCONFIRMATION|FOF_SILENT|FOF_ALLOWUNDO;
	DWORD Result=SHFileOperation(&fop);

	if (Result == 0x78 // DE_ACCESSDENIEDSRC == ERROR_ACCESS_DENIED
		&& Global->Opt->ElevationMode&ELEVATION_MODIFY_REQUEST) // Achtung! ShellAPI doesn't set LastNtStatus, so don't use ElevationRequired() here.
	{
		Result = Global->Elevation->fMoveToRecycleBin(fop);
	}

	if (Result)
	{
		SetLastError(SHErrorToWinError(Result));
	}

	return !Result && !fop.fAnyOperationsAborted;
}

static bool WipeFile(const string& Name, int TotalPercent, bool& Cancel, ConsoleTitle* DeleteTitle)
{
	bool Result = false;

	os::SetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);

	os::fs::file_walker WipeFile;

	if(WipeFile.Open(Name, FILE_READ_DATA|FILE_WRITE_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_WRITE_THROUGH|FILE_FLAG_SEQUENTIAL_SCAN))
	{
		const DWORD BufSize=65536;
		if(WipeFile.InitWalk(BufSize))
		{
			static std::array<BYTE, BufSize> Buf;
			static bool BufInit = false;
			if(!BufInit)
			{
				Buf.fill(Global->Opt->WipeSymbol); // используем символ заполнитель
				BufInit = true;
			}

			time_check TimeCheck(time_check::immediate, GetRedrawTimeout());
			do
			{
				size_t Written;
				WipeFile.Write(Buf.data(), WipeFile.GetChunkSize(), Written);
				if (TimeCheck)
				{
					if (CheckForEscSilent() && ConfirmAbortOp())
					{
						Cancel=true;
						return false;
					}

					ShellDeleteMsg(Name, DEL_WIPEPROCESS, TotalPercent, WipeFile.GetPercent(), DeleteTitle);
				}
			}
			while(WipeFile.Step());

			WipeFile.SetPointer(0,nullptr,FILE_BEGIN);
			WipeFile.SetEnd();
		}
		WipeFile.Close();
		string strTempName;
		FarMkTempEx(strTempName,nullptr,FALSE);
		Result = os::MoveFile(Name,strTempName) && os::DeleteFile(strTempName);
	}
	return Result;
}

static int WipeDirectory(const string& Name)
{
	string strTempName, strPath = Name;

	if (!CutToParent(strPath))
	{
		strPath.clear();
	}

	FarMkTempEx(strTempName,nullptr, FALSE, strPath.empty()?nullptr:strPath.data());

	if (!os::MoveFile(Name, strTempName))
	{
		return FALSE;
	}

	return os::RemoveDirectory(strTempName);
}

ShellDelete::ShellDelete(Panel *SrcPanel,bool Wipe):
	ReadOnlyDeleteMode(-1),
	m_SkipMode(-1),
	SkipWipeMode(-1),
	SkipFoldersMode(-1),
	ProcessedItems(0)
{
	SCOPED_ACTION(ChangePriority)(Global->Opt->DelThreadPriority);
	SCOPED_ACTION(TPreRedrawFuncGuard)(std::make_unique<DelPreRedrawItem>());
	os::FAR_FIND_DATA FindData;
	string strDeleteFilesMsg;
	string strSelName;
	string strSelShortName;
	string strDizName;
	int DizPresent;
	int Ret;
	BOOL NeedUpdate=TRUE, NeedSetUpADir=FALSE;
	bool Opt_DeleteToRecycleBin=Global->Opt->DeleteToRecycleBin;
	/*& 31.05.2001 OT Запретить перерисовку текущего окна*/
	const auto WindowFromLaunched = Global->WindowManager->GetCurrentWindow();
	WindowFromLaunched->Lock();
	bool DeleteAllFolders=!Global->Opt->Confirm.DeleteFolder;
	const auto UpdateDiz=(Global->Opt->Diz.UpdateMode==DIZ_UPDATE_ALWAYS ||
	           (SrcPanel->IsDizDisplayed() &&
	            Global->Opt->Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED));

	SCOPE_EXIT
	{
		Global->Opt->DeleteToRecycleBin=Opt_DeleteToRecycleBin;
		// Разрешить перерисовку окна
		WindowFromLaunched->Unlock();

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
		string strRoot;
		SrcPanel->GetSelName(nullptr,FileAttr);
		SrcPanel->GetSelName(&strSelName,FileAttr);
		ConvertNameToFull(strSelName, strRoot);
		GetPathRoot(strRoot,strRoot);

		if (Global->Opt->DeleteToRecycleBin && FAR_GetDriveType(strRoot) != DRIVE_FIXED)
			Global->Opt->DeleteToRecycleBin = false;
	}

	if (SelCount==1)
	{
		SrcPanel->GetSelName(nullptr,FileAttr);
		SrcPanel->GetSelName(&strSelName,FileAttr);

		if (TestParentFolderName(strSelName) || strSelName.empty())
		{
			NeedUpdate=FALSE;
			return;
		}

		strDeleteFilesMsg = strSelName;
		QuoteOuterSpace(strDeleteFilesMsg);
	}
	else
	{
		// в зависимости от числа ставим нужное окончание
		const wchar_t *Ends;
		FormatString StrItems;
		StrItems << SelCount;
		Ends=MSG(MAskDeleteItemsA);
		size_t LenItems = StrItems.size();

		if (LenItems > 0)
		{
			if ((LenItems >= 2 && StrItems[LenItems-2] == L'1') ||
			        StrItems[LenItems-1] >= L'5' ||
			        StrItems[LenItems-1] == L'0')
				Ends=MSG(MAskDeleteItemsS);
			else if (StrItems[LenItems-1] == L'1')
				Ends=MSG(MAskDeleteItems0);
		}
		strDeleteFilesMsg = string_format(MAskDeleteItems, SelCount, Ends);
	}

	Ret=1;

	//   Обработка "удаления" линков
	if ((FileAttr & FILE_ATTRIBUTE_REPARSE_POINT) && SelCount==1)
	{
		string strJuncName;
		ConvertNameToFull(strSelName,strJuncName);

		if (GetReparsePointInfo(strJuncName, strJuncName)) // ? SelName ?
		{
			NormalizeSymlinkName(strJuncName);
			string strAskDeleteLink=MSG(MAskDeleteLink);
			os::fs::file_status Status(strJuncName);
			if (os::fs::exists(Status))
			{
				strAskDeleteLink+=L" ";
				strAskDeleteLink += MSG(is_directory(Status)? MAskDeleteLinkFolder : MAskDeleteLinkFile);
			}

			Ret=Message(0, MSG(MDeleteLinkTitle),
					make_vector<string>(strDeleteFilesMsg, strAskDeleteLink, strJuncName),
					make_vector<string>(MSG(MDeleteLinkDelete), MSG(MCancel)),
					nullptr, nullptr, &DeleteLinkId);

			if (Ret)
				return;
		}
	}

	if (Ret && Global->Opt->Confirm.Delete)
	{
		LNGID mTitle = Wipe ? MDeleteWipeTitle : MDeleteTitle;
		LNGID mDText;
		string tText;
		LNGID mDBttn = Wipe ? MDeleteWipe : Global->Opt->DeleteToRecycleBin ? MDeleteRecycle : MDelete;
		bool bHilite = Global->Opt->DelOpt.HighlightSelected;
		int mshow = std::min(std::max((int)Global->Opt->DelOpt.ShowSelected, 1), ScrY/2);

		std::vector<string> items;
		items.push_back(strDeleteFilesMsg);

		if (SelCount == 1)
		{
			bool folder = (FileAttr & FILE_ATTRIBUTE_DIRECTORY) != 0;

			if (Wipe && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
				mDText = folder ? MAskWipeFolder : MAskWipeFile;
			else
			{
				if (Global->Opt->DeleteToRecycleBin)
					mDText = folder ? MAskDeleteRecycleFolder : MAskDeleteRecycleFile;
				else
					mDText = folder ? MAskDeleteFolder : MAskDeleteFile;
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
				mDText = MAskWipe;
				mTitle = MDeleteWipeTitle;
			}
			else
				mDText = Global->Opt->DeleteToRecycleBin ? MAskDeleteRecycle : MAskDelete;

			if (mshow > 1)
			{
				tText = MSG(mDText) + string(L" ") + strDeleteFilesMsg;
				items.clear();
				DWORD attr;
				string name;
				SrcPanel->GetSelName(nullptr, attr);

				for (size_t i = 0; i < SelCount; ++i)
				{
					if (i == (size_t)mshow-1 && i+1 < SelCount)
					{
						items.push_back(L"...");
						break;
					}
					SrcPanel->GetSelName(&name, attr);
					QuoteOuterSpace(name);
					items.push_back(name);
				}
			}
		}

		intptr_t start_hilite = 0, end_hilite = 0;

		DialogBuilder Builder(mTitle, nullptr, [&](Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2) -> intptr_t
		{
			if (bHilite && Msg == DN_CTLCOLORDLGITEM && Param1 >= start_hilite && Param1 <= end_hilite)
			{
				const auto Colors = static_cast<const FarDialogItemColors*>(Param2);
				Colors->Colors[0] = Colors->Colors[1];
			}
			return Dlg->DefProc(Msg, Param1, Param2);
		});

		if (tText.empty())
			tText = MSG(mDText);

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

		Builder.AddOKCancel(mDBttn, MCancel);
		Builder.SetId(Wipe ? DeleteWipeId : (Global->Opt->DeleteToRecycleBin ? DeleteRecycleId : DeleteFileFolderId));

		if (Wipe || !Global->Opt->DeleteToRecycleBin)
			Builder.SetDialogMode(DMODE_WARNINGSTYLE);

		if (!Builder.ShowDialog())
		{
			NeedUpdate = FALSE;
			return;
		}
	}

	if (UpdateDiz)
		SrcPanel->ReadDiz();

	SrcPanel->GetDizName(strDizName);
	DizPresent=(!strDizName.empty() && os::fs::exists(strDizName));

	if ((NeedSetUpADir=CheckUpdateAnotherPanel(SrcPanel,strSelName)) == -1)
		return;

	if (SrcPanel->GetType()==TREE_PANEL)
		FarChDir(L"\\");

	{
		ConsoleTitle DeleteTitle(MSG(MDeletingTitle));
		SCOPED_ACTION(IndeterminateTaskBar);
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

			while (SrcPanel->GetSelName(&strSelName,FileAttr,&strSelShortName) && !Cancel)
			{
				if (!(FileAttr&FILE_ATTRIBUTE_REPARSE_POINT))
				{
					if (FileAttr&FILE_ATTRIBUTE_DIRECTORY)
					{
						DirInfoData Data = {};

						if (GetDirInfo(MSG(MDeletingTitle), strSelName, Data, MessageDelay, nullptr, GETDIRINFO_NOREDRAW) > 0)
						{
							ItemsCount+=Data.FileCount+Data.DirCount+1;
						}
						else
						{
							Cancel=true;
						}
						MessageDelay = getdirinfo_no_delay;
					}
					else
					{
						ItemsCount++;
					}
				}
			}
		}

		SrcPanel->GetSelName(nullptr,FileAttr);
		time_check TimeCheck(time_check::immediate, GetRedrawTimeout());
		bool cannot_recycle_try_delete_folder = false;

		while (!Cancel && (cannot_recycle_try_delete_folder || SrcPanel->GetSelName(&strSelName,FileAttr,&strSelShortName)))
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

				ShellDeleteMsg(strSelName, Wipe?DEL_WIPE:DEL_DEL, TotalPercent, 0, &DeleteTitle);
			}

			if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!DeleteAllFolders && !cannot_recycle_try_delete_folder)
				{
					string strFullName;
					ConvertNameToFull(strSelName, strFullName);

					if (os::fs::is_not_empty_directory(strFullName))
					{
						int MsgCode = 0; // для symlink не нужно подтверждение
						if (!(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT)) {
							const GUID* guidId = &DeleteFolderId;
							LNGID tit = MDeleteFolderTitle, con = MDeleteFolderConfirm, del = MDeleteFileDelete;
							if (Wipe) {
								tit = MWipeFolderTitle; con = MWipeFolderConfirm; del = MDeleteFileWipe;
								guidId = &WipeFolderId;
							}
							else if (Global->Opt->DeleteToRecycleBin) {
								con = MRecycleFolderConfirm; del = MDeleteRecycle;
								guidId = &DeleteFolderRecycleId;
							}

							MsgCode=Message(MSG_WARNING, MSG(tit),
									make_vector<string>(MSG(con), strFullName),
									make_vector<string>(MSG(del), MSG(MDeleteFileAll), MSG(MDeleteFileSkip), MSG(MDeleteFileCancel)),
									nullptr, nullptr, guidId);
						}

						if (MsgCode<0 || MsgCode==3)
						{
							NeedSetUpADir=FALSE;
							break;
						}

						if (MsgCode==1)
							DeleteAllFolders = true;

						if (MsgCode==2)
							continue;
					}
				}

				bool DirSymLink=(FileAttr&FILE_ATTRIBUTE_DIRECTORY && FileAttr&FILE_ATTRIBUTE_REPARSE_POINT);

				if (!DirSymLink && (!Global->Opt->DeleteToRecycleBin || Wipe))
				{
					string strFullName;
					ScanTree ScTree(true, true, FALSE);
					string strSelFullName;

					if (IsAbsolutePath(strSelName))
					{
						strSelFullName=strSelName;
					}
					else
					{
						strSelFullName = SrcPanel->GetCurDir();
						AddEndSlash(strSelFullName);
						strSelFullName+=strSelName;
					}

					ScTree.SetFindPath(strSelFullName,L"*", 0);
					time_check TreeTimeCheck(time_check::immediate, GetRedrawTimeout());

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

							ShellDeleteMsg(strFullName,Wipe?DEL_WIPE:DEL_DEL, TreeTotalPercent, 0, &DeleteTitle);
						}

						if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						{
							if (FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
							{
								if (FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
									os::SetFileAttributes(strFullName,FILE_ATTRIBUTE_NORMAL);

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
								int MsgCode=Message(MSG_WARNING, MSG(Wipe?MWipeFolderTitle:MDeleteFolderTitle),
											make_vector<string>(MSG(Wipe? MWipeFolderConfirm : MDeleteFolderConfirm), strFullName),
											make_vector<string>(MSG(Wipe?MDeleteFileWipe:MDeleteFileDelete),MSG(MDeleteFileAll),MSG(MDeleteFileSkip),MSG(MDeleteFileCancel)),
											nullptr, nullptr, (Wipe?&WipeFolderId:&DeleteFolderId)); // ??? other GUID ???

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
									os::SetFileAttributes(strFullName,FILE_ATTRIBUTE_NORMAL);

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
								if (ShellRemoveFile(strFullName,Wipe, TreeTotalPercent, &DeleteTitle)==DELETE_CANCEL)
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
						os::SetFileAttributes(strSelName,FILE_ATTRIBUTE_NORMAL);

					int DeleteCode;

					// нефига здесь выделываться, а надо учесть, что удаление
					// симлинка в корзину чревато потерей оригинала.
					DIRDELTYPE Type = Wipe ? D_WIPE : (Global->Opt->DeleteToRecycleBin && !(DirSymLink && !IsWindowsVistaOrGreater()) ? D_RECYCLE : D_DEL);
					DeleteCode=ERemoveDirectory(strSelName, Type);

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
					int DeleteCode=ShellRemoveFile(strSelName,Wipe,TotalPercent, &DeleteTitle);

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
		MsgCode=Message(MSG_WARNING, MSG(MWarning),
						make_vector<string>(MSG(MDeleteRO), MSG(Wipe?MAskWipeRO:MAskDeleteRO)),
						make_vector<string>(MSG(Wipe?MDeleteFileWipe:MDeleteFileDelete),MSG(MDeleteFileAll),MSG(MDeleteFileSkip),MSG(MDeleteFileSkipAll),MSG(MDeleteFileCancel)),
						nullptr, nullptr, (Wipe?&DeleteAskWipeROId:&DeleteAskDeleteROId));
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

	os::SetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);
	return DELETE_YES;
}

DEL_RESULT ShellDelete::ShellRemoveFile(const string& Name, bool Wipe, int TotalPercent, ConsoleTitle* DeleteTitle)
{
	ProcessedItems++;
	string strFullName;
	ConvertNameToFull(Name, strFullName);
	int MsgCode=0;

	for (;;)
	{
		bool recycle_bin = false;
		if (Wipe)
		{
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
				MsgCode=Message(MSG_WARNING, MSG(MError),
								make_vector<string>(strFullName, MSG(MDeleteHardLink1), MSG(MDeleteHardLink2), MSG(MDeleteHardLink3)),
								make_vector<string>(MSG(MDeleteFileWipe),MSG(MDeleteFileAll),MSG(MDeleteFileSkip),MSG(MDeleteFileSkipAll),MSG(MDeleteCancel)),
								nullptr, nullptr, &WipeHardLinkId);
			}

			switch (MsgCode)
			{
			case 4: case -1: case -2:
				return DELETE_CANCEL;
			case 3:
				SkipWipeMode = 2; // fallthrough down
			case 2:
				return DELETE_SKIP;
			case 1:
				SkipWipeMode = 0;
			case 0:
				{
					bool Cancel = false;
					if (WipeFile(strFullName, TotalPercent, Cancel, DeleteTitle))
						return DELETE_SUCCESS;
					else if(Cancel)
						return DELETE_CANCEL;
				}
			}
		}
		else if (!Global->Opt->DeleteToRecycleBin)
		{
			if (os::DeleteFile(strFullName))
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
				if (os::DeleteFile(strFullName))
					break;
			}
		}

		if (m_SkipMode != -1)
			MsgCode = m_SkipMode;
		else
		{
			Global->CatchError();
			MsgCode = OperationFailed(strFullName, MError, MSG(recycle_bin ? MCannotRecycleFile:MCannotDeleteFile));
		}

		switch (MsgCode)
		{
		case 3: case -1: case -2: // [Cancel]
			return DELETE_CANCEL;
		case 2:                   // [Skip All]
			m_SkipMode = 2;          // fallthrough down
		case 1:                   // [Skip]
			return DELETE_SKIP;
		} // case 0:              // {Retry}
	}

	return DELETE_SUCCESS;
}

DEL_RESULT ShellDelete::ERemoveDirectory(const string& Name,DIRDELTYPE Type)
{
	ProcessedItems++;
	string strFullName;
	ConvertNameToFull(Name,strFullName);

	bool Success = false;
	while(!Success)
	{
		bool recycle_bin = false;
		switch(Type)
		{
		case D_DEL:
			Success = os::RemoveDirectory(Name);
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
			int MsgCode;

			if (SkipFoldersMode!=-1)
			{
				MsgCode=SkipFoldersMode;
			}
			else
			{
				Global->CatchError();
				MsgCode=OperationFailed(Name, MError, MSG(recycle_bin ? MCannotRecycleFolder:MCannotDeleteFolder));
			}

			switch (MsgCode)
			{
			case 3: case -1: case -2: // [Cancel]
				return DELETE_CANCEL;
			case 2:						  // [Skip All]
				SkipFoldersMode = 2;	  // fallthrough down
			case 1:						  // [Skip]
				return DELETE_SKIP;
			} // case 0:              // {Retry}
		}
	}

	return DELETE_SUCCESS;
}
#include "fileattr.hpp"
bool ShellDelete::RemoveToRecycleBin(const string& Name, bool dir, DEL_RESULT& ret)
{
	string strFullName;
	ConvertNameToFull(Name, strFullName);

	// При удалении в корзину папки с симлинками получим траблу, если предварительно линки не убрать.
	if (!IsWindowsVistaOrGreater() && os::fs::is_directory(Name))
	{
		string strFullName2;
		os::FAR_FIND_DATA FindData;
		ScanTree ScTree(true, true, FALSE);
		ScTree.SetFindPath(Name,L"*", 0);

		bool MessageShown = false;
		int SkipMode = -1;

		while (ScTree.GetNextName(FindData,strFullName2))
		{
			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && FindData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
			{
				if (!MessageShown)
				{
					MessageShown = true;
					if (Message(MSG_WARNING, MSG(MWarning),
					    make_vector<string>(
					        MSG(MRecycleFolderConfirmDeleteLink1),
					        MSG(MRecycleFolderConfirmDeleteLink2),
					        MSG(MRecycleFolderConfirmDeleteLink3),
					        MSG(MRecycleFolderConfirmDeleteLink4)
					    ),
					    make_vector<string>(MSG(MYes), MSG(MCancel)),
					    nullptr, nullptr, &RecycleFolderConfirmDeleteLinkId
					    ) != 0)
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

	if (MoveToRecycleBinInternal(strFullName.data()))
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
		Global->CatchError();
		string qName(strFullName);
		QuoteOuterSpace(qName);

		int MsgCode = Message(MSG_WARNING|MSG_ERRORTYPE, MSG(MError),
			make_vector<string>(MSG(dir ? MCannotRecycleFolder : MCannotRecycleFile), qName),
			make_vector<string>(MSG(MDeleteFileDelete), MSG(MDeleteSkip), MSG(MDeleteSkipAll), MSG(MDeleteCancel)),
			nullptr, nullptr, (dir ? &CannotRecycleFolderId : &CannotRecycleFileId));

		switch (MsgCode) {
		case 3: case -1: case -2:       // [Cancel]
			ret = DELETE_CANCEL;
			break;
		case 2:                         // [Skip All]
			if (dir)
				SkipFoldersMode = 2;
			else
				m_SkipMode = 2;             // fallthrough down
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
	os::FAR_FIND_DATA FindData;
	ScanTree ScTree(true, true, FALSE);
	ScTree.SetFindPath(Dir,L"*",0);

	while (ScTree.GetNextName(FindData, strFullName))
	{
		os::SetFileAttributes(strFullName,FILE_ATTRIBUTE_NORMAL);

		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (ScTree.IsDirSearchDone())
				os::RemoveDirectory(strFullName);
		}
		else
			os::DeleteFile(strFullName);
	}

	os::SetFileAttributes(Dir,FILE_ATTRIBUTE_NORMAL);
	os::RemoveDirectory(Dir);
}

bool DeleteFileWithFolder(const string& FileName)
{
	bool Result = false;
	string strFileOrFolderName(FileName);
	if (os::SetFileAttributes(Unquote(strFileOrFolderName), FILE_ATTRIBUTE_NORMAL))
	{
		if (os::DeleteFile(strFileOrFolderName)) //BUGBUG
		{
			Result = CutToParent(strFileOrFolderName) && os::RemoveDirectory(strFileOrFolderName);
		}
	}
	return Result;
}
