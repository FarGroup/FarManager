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
#include "filelist.hpp"
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

static void ShellDeleteMsg(const wchar_t *Name, DEL_MODE Mode, int Percent, int WipePercent, ConsoleTitle* DeleteTitle)
{
	FormatString strProgress, strWipeProgress;
	size_t Width=ScrX/2;
	size_t Length=Width-5; // -5 под проценты
	if(Mode==DEL_WIPEPROCESS || Mode==DEL_WIPE)
	{
		wchar_t *WipeProgress=strWipeProgress.GetBuffer(Length);
		if (WipeProgress)
		{
			size_t CurPos=std::min(WipePercent,100)*Length/100;
			wmemset(WipeProgress,BoxSymbols[BS_X_DB],CurPos);
			wmemset(WipeProgress+(CurPos),BoxSymbols[BS_X_B0],Length-CurPos);
			strWipeProgress.ReleaseBuffer(Length);
			strWipeProgress<<L" "<<fmt::MinWidth(3)<<WipePercent<<L"%";
		}
		if(Percent==-1)
		{
			Global->TBC->SetProgressValue(WipePercent, 100);
		}
	}

	if (Mode!=DEL_SCAN && Percent!=-1)
	{
		wchar_t *Progress=strProgress.GetBuffer(Length);
		if (Progress)
		{
			size_t CurPos=std::min(Percent,100)*Length/100;
			wmemset(Progress,BoxSymbols[BS_X_DB],CurPos);
			wmemset(Progress+(CurPos),BoxSymbols[BS_X_B0],Length-CurPos);
			strProgress.ReleaseBuffer(Length);
			strProgress<<L" "<<fmt::MinWidth(3)<<Percent<<L"%";
			*DeleteTitle << L"{" << Percent << L"%} " << MSG((Mode==DEL_WIPE || Mode==DEL_WIPEPROCESS)?MDeleteWipeTitle:MDeleteTitle) << fmt::Flush();
		}
		Global->TBC->SetProgressValue(Percent,100);
	}

	string strOutFileName(Name);
	TruncPathStr(strOutFileName,static_cast<int>(Width));
	CenterStr(strOutFileName,strOutFileName,static_cast<int>(Width));
	const wchar_t* Progress1 = nullptr;
	const wchar_t* Progress2 = nullptr;
	if(!strWipeProgress.IsEmpty())
	{
		Progress1 = strWipeProgress.CPtr();
		Progress2 = strProgress.IsEmpty()? nullptr : strProgress.CPtr();
	}
	else
	{
		Progress1 = strProgress.IsEmpty()? nullptr : strProgress.CPtr();
	}
	Message(0,0,
		MSG((Mode==DEL_WIPE || Mode==DEL_WIPEPROCESS)?MDeleteWipeTitle:MDeleteTitle),
		Mode==DEL_SCAN? MSG(MScanningFolder) : MSG((Mode==DEL_WIPE || Mode==DEL_WIPEPROCESS)?MDeletingWiping:MDeleting),
		strOutFileName, Progress1, Progress2);

	if (!Global->PreRedraw->empty())
	{
		PreRedrawItem& preRedrawItem(Global->PreRedraw->top());
		preRedrawItem.Param.Param1=static_cast<void*>(const_cast<wchar_t*>(Name));
		preRedrawItem.Param.Param2=DeleteTitle;
		preRedrawItem.Param.Param4=ToPtr(Mode);
		LARGE_INTEGER i = {(DWORD)Percent, (LONG)WipePercent};
		preRedrawItem.Param.Param5=i.QuadPart;
	}
}

static void PR_ShellDeleteMsg()
{
	if (!Global->PreRedraw->empty())
	{
		const PreRedrawItem& preRedrawItem(Global->PreRedraw->top());
		LARGE_INTEGER i;
		i.QuadPart = preRedrawItem.Param.Param5;
		ShellDeleteMsg(static_cast<const wchar_t*>(preRedrawItem.Param.Param1),static_cast<DEL_MODE>(reinterpret_cast<intptr_t>(preRedrawItem.Param.Param4)), i.LowPart, i.HighPart, reinterpret_cast<ConsoleTitle*>(const_cast<void*>(preRedrawItem.Param.Param2)));
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

	apiSetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);

	FileWalker WipeFile;

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

			DWORD StartTime=GetTickCount();
			while(WipeFile.Step())
			{
				DWORD Written;
				WipeFile.Write(Buf.data(), WipeFile.GetChunkSize(), Written);
				DWORD CurTime=GetTickCount();
				if (CurTime-StartTime>(DWORD)Global->Opt->RedrawTimeout)
				{
					StartTime=CurTime;

					if (CheckForEscSilent() && ConfirmAbortOp())
					{
						Cancel=true;
						return false;
					}

					ShellDeleteMsg(Name, DEL_WIPEPROCESS, TotalPercent, WipeFile.GetPercent(), DeleteTitle);
				}
			}
			WipeFile.SetPointer(0,nullptr,FILE_BEGIN);
			WipeFile.SetEnd();
		}
		WipeFile.Close();
		string strTempName;
		FarMkTempEx(strTempName,nullptr,FALSE);
		Result = apiMoveFile(Name,strTempName) && apiDeleteFile(strTempName);
	}
	return Result;
}

static int WipeDirectory(const string& Name)
{
	string strTempName, strPath;

	if (FirstSlash(Name))
	{
		strPath = Name;
		DeleteEndSlash(strPath);
		CutToSlash(strPath);
	}

	FarMkTempEx(strTempName,nullptr, FALSE, strPath.IsEmpty()?nullptr:strPath.CPtr());

	if (!apiMoveFile(Name, strTempName))
	{
		return FALSE;
	}

	return apiRemoveDirectory(strTempName);
}


ShellDelete::ShellDelete(Panel *SrcPanel,bool Wipe):
	ReadOnlyDeleteMode(-1),
	SkipMode(-1),
	SkipWipeMode(-1),
	SkipFoldersMode(-1),
	ProcessedItems(0)
{
	ChangePriority ChPriority(Global->Opt->DelThreadPriority);
	TPreRedrawFuncGuard preRedrawFuncGuard(PR_ShellDeleteMsg);
	FAR_FIND_DATA FindData;
	string strDeleteFilesMsg;
	string strSelName;
	string strSelShortName;
	string strDizName;
	string strFullName;
	DWORD FileAttr;
	size_t SelCount;
	int UpdateDiz;
	int DizPresent;
	int Ret;
	BOOL NeedUpdate=TRUE, NeedSetUpADir=FALSE;
	bool Opt_DeleteToRecycleBin=Global->Opt->DeleteToRecycleBin;
	/*& 31.05.2001 OT Запретить перерисовку текущего фрейма*/
	Frame *FrameFromLaunched=FrameManager->GetCurrentFrame();
	FrameFromLaunched->Lock();
	bool DeleteAllFolders=!Global->Opt->Confirm.DeleteFolder;
	UpdateDiz=(Global->Opt->Diz.UpdateMode==DIZ_UPDATE_ALWAYS ||
	           (SrcPanel->IsDizDisplayed() &&
	            Global->Opt->Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED));

	if (!(SelCount=SrcPanel->GetSelCount()))
		goto done;

	// Удаление в корзину только для  FIXED-дисков
	{
		string strRoot;
//    char FSysNameSrc[NM];
		SrcPanel->GetSelName(nullptr,FileAttr);
		SrcPanel->GetSelName(&strSelName,FileAttr);
		ConvertNameToFull(strSelName, strRoot);
		GetPathRoot(strRoot,strRoot);

//_SVS(SysLog(L"Del: SelName='%s' Root='%s'",SelName,Root));
		if (Global->Opt->DeleteToRecycleBin && FAR_GetDriveType(strRoot) != DRIVE_FIXED)
			Global->Opt->DeleteToRecycleBin=0;
	}

	if (SelCount==1)
	{
		SrcPanel->GetSelName(nullptr,FileAttr);
		SrcPanel->GetSelName(&strSelName,FileAttr);

		if (TestParentFolderName(strSelName) || strSelName.IsEmpty())
		{
			NeedUpdate=FALSE;
			goto done;
		}

		strDeleteFilesMsg = strSelName;
		QuoteLeadingSpace(strDeleteFilesMsg);
	}
	else
	{
		// в зависимости от числа ставим нужное окончание
		const wchar_t *Ends;
		FormatString StrItems;
		StrItems << SelCount;
		Ends=MSG(MAskDeleteItemsA);
		size_t LenItems = StrItems.GetLength();

		if (LenItems > 0)
		{
			if ((LenItems >= 2 && StrItems.At(LenItems-2) == L'1') ||
			        StrItems.At(LenItems-1) >= L'5' ||
			        StrItems.At(LenItems-1) == L'0')
				Ends=MSG(MAskDeleteItemsS);
			else if (StrItems.At(LenItems-1) == L'1')
				Ends=MSG(MAskDeleteItems0);
		}
		strDeleteFilesMsg = LangString(MAskDeleteItems) << SelCount << Ends;
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
			//SetMessageHelp(L"DeleteLink");
			string strAskDeleteLink=MSG(MAskDeleteLink);
			DWORD dwAttr=apiGetFileAttributes(strJuncName);

			if (dwAttr!=INVALID_FILE_ATTRIBUTES)
			{
				strAskDeleteLink+=L" ";
				strAskDeleteLink+=dwAttr&FILE_ATTRIBUTE_DIRECTORY?MSG(MAskDeleteLinkFolder):MSG(MAskDeleteLinkFile);
			}

			Ret=Message(0,3,MSG(MDeleteLinkTitle),
			            strDeleteFilesMsg,
			            strAskDeleteLink,
			            strJuncName,
						MSG(MDeleteLinkDelete),MSG(MDeleteLinkUnlink),MSG(MCancel));

			if (Ret == 1)
			{
				ConvertNameToFull(strSelName, strJuncName);

				if (Global->Opt->Confirm.Delete)
				{
					; //  ;-%
				}

				if ((NeedSetUpADir=CheckUpdateAnotherPanel(SrcPanel,strSelName)) != -1) //JuncName?
				{
					DeleteReparsePoint(strJuncName);
					ShellUpdatePanels(SrcPanel,NeedSetUpADir);
				}

				goto done;
			}

			if (Ret )
				goto done;
		}
	}

	if (Ret && (Global->Opt->Confirm.Delete || SelCount>1 || (FileAttr & FILE_ATTRIBUTE_DIRECTORY)))
	{
		const wchar_t *DelMsg;
		const wchar_t *TitleMsg=MSG(Wipe?MDeleteWipeTitle:MDeleteTitle);
		/* $ 05.01.2001 IS
		   ! Косметика в сообщениях - разные сообщения в зависимости от того,
		     какие и сколько элементов выделено.
		*/
		BOOL folder=(FileAttr & FILE_ATTRIBUTE_DIRECTORY);

		if (SelCount==1)
		{
			if (Wipe && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
				DelMsg=MSG(folder?MAskWipeFolder:MAskWipeFile);
			else
			{
				if (Global->Opt->DeleteToRecycleBin && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
					DelMsg=MSG(folder?MAskDeleteRecycleFolder:MAskDeleteRecycleFile);
				else
					DelMsg=MSG(folder?MAskDeleteFolder:MAskDeleteFile);
			}
		}
		else
		{
			if (Wipe && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
			{
				DelMsg=MSG(MAskWipe);
				TitleMsg=MSG(MDeleteWipeTitle);
			}
			else if (Global->Opt->DeleteToRecycleBin && !(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
				DelMsg=MSG(MAskDeleteRecycle);
			else
				DelMsg=MSG(MAskDelete);
		}

		const wchar_t* const Items[] = {DelMsg, strDeleteFilesMsg, MSG(Wipe? MDeleteWipe : Global->Opt->DeleteToRecycleBin? MDeleteRecycle : MDelete), MSG(MCancel)};
		if (Message(0, 2, TitleMsg, Items, ARRAYSIZE(Items), L"DeleteFile") != 0)
		{
			NeedUpdate=FALSE;
			goto done;
		}
	}

	if (Global->Opt->Confirm.Delete && SelCount>1)
	{
		//SaveScreen SaveScr;
		SetCursorType(FALSE,0);
		const wchar_t* const Items[] = {MSG(Wipe? MAskWipe : MAskDelete), strDeleteFilesMsg, MSG(MDeleteFileAll), MSG(MDeleteFileCancel)};
		if (Message(MSG_WARNING,2,MSG(Wipe? MWipeFilesTitle : MDeleteFilesTitle), Items, ARRAYSIZE(Items), L"DeleteFile") != 0)
		{
			NeedUpdate=FALSE;
			goto done;
		}
	}

	if (UpdateDiz)
		SrcPanel->ReadDiz();

	SrcPanel->GetDizName(strDizName);
	DizPresent=(!strDizName.IsEmpty() && apiGetFileAttributes(strDizName)!=INVALID_FILE_ATTRIBUTES);

	if ((NeedSetUpADir=CheckUpdateAnotherPanel(SrcPanel,strSelName)) == -1)
		goto done;

	if (SrcPanel->GetType()==TREE_PANEL)
		FarChDir(L"\\");

	{
		ConsoleTitle DeleteTitle(MSG(MDeletingTitle));
		TaskBar TB;
		wakeful W;
		bool Cancel=false;
		//SaveScreen SaveScr;
		SetCursorType(FALSE,0);
		ReadOnlyDeleteMode=-1;
		SkipMode=-1;
		SkipWipeMode=-1;
		SkipFoldersMode=-1;
		ULONG ItemsCount=0;
		ProcessedItems=0;

		if (Global->Opt->DelOpt.DelShowTotal)
		{
			SrcPanel->GetSelName(nullptr,FileAttr);
			DWORD StartTime=GetTickCount();
			bool FirstTime=true;

			while (SrcPanel->GetSelName(&strSelName,FileAttr,&strSelShortName) && !Cancel)
			{
				if (!(FileAttr&FILE_ATTRIBUTE_REPARSE_POINT))
				{
					if (FileAttr&FILE_ATTRIBUTE_DIRECTORY)
					{
						DWORD CurTime=GetTickCount();

						if (ItemsCount > 1 && (CurTime-StartTime>(DWORD)Global->Opt->RedrawTimeout || FirstTime))
						{
							StartTime=CurTime;
							FirstTime=false;

							if (CheckForEscSilent() && ConfirmAbortOp())
							{
								Cancel=true;
								break;
							}

							ShellDeleteMsg(strSelName, DEL_SCAN, 0, 0, &DeleteTitle);
						}
						DirInfoData Data = {};

						if (GetDirInfo(nullptr, strSelName, Data, -1, nullptr, 0) > 0)
						{
							ItemsCount+=Data.FileCount+Data.DirCount+1;
						}
						else
						{
							Cancel=true;
						}
					}
					else
					{
						ItemsCount++;
					}
				}
			}
		}

		SrcPanel->GetSelName(nullptr,FileAttr);
		DWORD StartTime=GetTickCount();
		bool FirstTime=true;

		while (SrcPanel->GetSelName(&strSelName,FileAttr,&strSelShortName) && !Cancel)
		{
			int Length=(int)strSelName.GetLength();

			if (!Length || (strSelName.At(0)==L'\\' && Length<2) ||
			        (strSelName.At(1)==L':' && Length<4))
				continue;

			DWORD CurTime=GetTickCount();
			int TotalPercent = (Global->Opt->DelOpt.DelShowTotal && ItemsCount >1)?(ProcessedItems*100/ItemsCount):-1;
			if (CurTime-StartTime>(DWORD)Global->Opt->RedrawTimeout || FirstTime)
			{
				StartTime=CurTime;
				FirstTime=false;

				if (CheckForEscSilent() && ConfirmAbortOp())
				{
					Cancel=true;
					break;
				}

				ShellDeleteMsg(strSelName, Wipe?DEL_WIPE:DEL_DEL, TotalPercent, 0, &DeleteTitle);
			}

			if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!DeleteAllFolders)
				{
					ConvertNameToFull(strSelName, strFullName);

					if (TestFolder(strFullName) == TSTFLD_NOTEMPTY)
					{
						int MsgCode=0;

						// для symlink`а не нужно подтверждение
						if (!(FileAttr & FILE_ATTRIBUTE_REPARSE_POINT))
							MsgCode=Message(MSG_WARNING,4,MSG(Wipe?MWipeFolderTitle:MDeleteFolderTitle),
							                MSG(Wipe?MWipeFolderConfirm:MDeleteFolderConfirm),strFullName,
							                MSG(Wipe?MDeleteFileWipe:MDeleteFileDelete),MSG(MDeleteFileAll),
							                MSG(MDeleteFileSkip),MSG(MDeleteFileCancel));

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
					ScanTree ScTree(TRUE,TRUE,FALSE);
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
					DWORD StartTime=GetTickCount();

					while (ScTree.GetNextName(&FindData,strFullName))
					{
						DWORD CurTime=GetTickCount();
						int TotalPercent = (Global->Opt->DelOpt.DelShowTotal && ItemsCount >1)?(ProcessedItems*100/ItemsCount):-1;
						if (CurTime-StartTime>(DWORD)Global->Opt->RedrawTimeout)
						{
							StartTime=CurTime;

							if (CheckForEscSilent())
							{
								int AbortOp = ConfirmAbortOp();

								if (AbortOp)
								{
									Cancel=true;
									break;
								}
							}

							ShellDeleteMsg(strFullName,Wipe?DEL_WIPE:DEL_DEL, TotalPercent, 0, &DeleteTitle);
						}

						if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						{
							if (FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
							{
								if (FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
									apiSetFileAttributes(strFullName,FILE_ATTRIBUTE_NORMAL);

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

							if (!DeleteAllFolders && !ScTree.IsDirSearchDone() && TestFolder(strFullName) == TSTFLD_NOTEMPTY)
							{
								int MsgCode=Message(MSG_WARNING,4,MSG(Wipe?MWipeFolderTitle:MDeleteFolderTitle),
								                    MSG(Wipe?MWipeFolderConfirm:MDeleteFolderConfirm),strFullName,
								                    MSG(Wipe?MDeleteFileWipe:MDeleteFileDelete),MSG(MDeleteFileAll),
								                    MSG(MDeleteFileSkip),MSG(MDeleteFileCancel));

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
									apiSetFileAttributes(strFullName,FILE_ATTRIBUTE_NORMAL);

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
								if (ShellRemoveFile(strFullName,Wipe,TotalPercent, &DeleteTitle)==DELETE_CANCEL)
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
						apiSetFileAttributes(strSelName,FILE_ATTRIBUTE_NORMAL);

					int DeleteCode;

					// нефига здесь выделываться, а надо учесть, что удаление
					// симлинка в корзину чревато потерей оригинала.
					DIRDELTYPE Type = Wipe? D_WIPE : D_DEL;
					if (Global->Opt->DeleteToRecycleBin && !(DirSymLink && Global->WinVer() < _WIN32_WINNT_VISTA))
						Type = D_RECYCLE;
					DeleteCode=ERemoveDirectory(strSelName, Type);

					if (DeleteCode==DELETE_CANCEL)
						break;
					else if (DeleteCode==DELETE_SUCCESS)
					{
						TreeList::DelTreeName(strSelName);

						if (UpdateDiz)
							SrcPanel->DeleteDiz(strSelName,strSelShortName);
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
		if (DizPresent==(!strDizName.IsEmpty() && apiGetFileAttributes(strDizName)!=INVALID_FILE_ATTRIBUTES))
			SrcPanel->FlushDiz();

done:
	Global->Opt->DeleteToRecycleBin=Opt_DeleteToRecycleBin;
	// Разрешить перерисовку фрейма
	FrameFromLaunched->Unlock();

	if (NeedUpdate)
	{
		ShellUpdatePanels(SrcPanel,NeedSetUpADir);
	}
}

DEL_RESULT ShellDelete::AskDeleteReadOnly(const string& Name,DWORD Attr, bool Wipe)
{
	int MsgCode;

	if (!(Attr & FILE_ATTRIBUTE_READONLY))
		return(DELETE_YES);

	if (!Global->Opt->Confirm.RO)
		ReadOnlyDeleteMode=1;

	if (ReadOnlyDeleteMode!=-1)
		MsgCode=ReadOnlyDeleteMode;
	else
	{
		MsgCode=Message(MSG_WARNING,5,MSG(MWarning),MSG(MDeleteRO),Name,
		                MSG(Wipe?MAskWipeRO:MAskDeleteRO),MSG(Wipe?MDeleteFileWipe:MDeleteFileDelete),MSG(MDeleteFileAll),
		                MSG(MDeleteFileSkip),MSG(MDeleteFileSkipAll),
		                MSG(MDeleteFileCancel));
	}

	switch (MsgCode)
	{
		case 1:
			ReadOnlyDeleteMode=1;
			break;
		case 2:
			return(DELETE_SKIP);
		case 3:
			ReadOnlyDeleteMode=3;
			return(DELETE_SKIP);
		case -1:
		case -2:
		case 4:
			return(DELETE_CANCEL);
	}

	apiSetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);
	return(DELETE_YES);
}

DEL_RESULT ShellDelete::ShellRemoveFile(const string& Name, bool Wipe, int TotalPercent, ConsoleTitle* DeleteTitle)
{
	ProcessedItems++;
	string strFullName;
	ConvertNameToFull(Name, strFullName);
	int MsgCode=0;

	for (;;)
	{
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
				MsgCode=Message(MSG_WARNING,5,MSG(MError),strFullName,
				                MSG(MDeleteHardLink1),MSG(MDeleteHardLink2),MSG(MDeleteHardLink3),
				                MSG(MDeleteFileWipe),MSG(MDeleteFileAll),MSG(MDeleteFileSkip),MSG(MDeleteFileSkipAll),MSG(MDeleteCancel));
			}

			switch (MsgCode)
			{
				case -1:
				case -2:
				case 4:
					return DELETE_CANCEL;
				case 3:
					SkipWipeMode=2;
				case 2:
					return DELETE_SKIP;
				case 1:
					SkipWipeMode=0;
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
			/*
			        HANDLE hDelete=FAR_CreateFile(Name,GENERIC_WRITE,0,nullptr,OPEN_EXISTING,
			               FILE_FLAG_DELETE_ON_CLOSE|FILE_FLAG_POSIX_SEMANTICS,nullptr);
			        if (hDelete!=INVALID_HANDLE_VALUE && CloseHandle(hDelete))
			          break;
			*/
			if (apiDeleteFile(strFullName))
				break;
		}
		else if (RemoveToRecycleBin(strFullName))
			break;

		if (SkipMode!=-1)
			MsgCode=SkipMode;
		else
		{
			MsgCode=OperationFailed(strFullName, MError, MSG(MCannotDeleteFile));
		}

		switch (MsgCode)
		{
			case -1:
			case -2:
			case 3:
				return DELETE_CANCEL;
			case 2:
				SkipMode=1;
			case 1:
				return DELETE_SKIP;
		}
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
		switch(Type)
		{
		case D_DEL:
			Success = apiRemoveDirectory(Name) != FALSE;
			break;

		case D_WIPE:
			Success = WipeDirectory(Name) != FALSE;
			break;

		case D_RECYCLE:
			Success = RemoveToRecycleBin(Name) != FALSE;
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
				MsgCode=OperationFailed(Name, MError, MSG(MCannotDeleteFolder));
			}

			switch (MsgCode)
			{
				case -1:
				case -2:
				case 3:
					return DELETE_CANCEL;
				case 1:
					return DELETE_SKIP;
				case 2:
					SkipFoldersMode=2;
					return DELETE_SKIP;
			}
		}
	}

	return DELETE_SUCCESS;
}

bool ShellDelete::RemoveToRecycleBin(const string& Name)
{
	string strFullName;
	ConvertNameToFull(Name, strFullName);

	// При удалении в корзину папки с симлинками получим траблу, если предварительно линки не убрать.
	if (Global->WinVer() < _WIN32_WINNT_VISTA && Global->Opt->DeleteToRecycleBinKillLink && apiGetFileAttributes(Name) == FILE_ATTRIBUTE_DIRECTORY)
	{
		string strFullName2;
		FAR_FIND_DATA FindData;
		ScanTree ScTree(TRUE,TRUE,FALSE);
		ScTree.SetFindPath(Name,L"*", 0);

		while (ScTree.GetNextName(&FindData,strFullName2))
		{
			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && FindData.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)
				ERemoveDirectory(strFullName2, D_DEL);
		}
	}

	wchar_t *lpwszName = strFullName.GetBuffer(strFullName.GetLength()+2);
	lpwszName[strFullName.GetLength()+1] = 0; //dirty trick to make strFullName end with DOUBLE zero!!!

	return MoveToRecycleBinInternal(lpwszName);
}


void DeleteDirTree(const string& Dir)
{
	if (!*Dir ||
	        (IsSlash(Dir[0]) && !Dir[1]) ||
	        (Dir[1]==L':' && IsSlash(Dir[2]) && !Dir[3]))
		return;

	string strFullName;
	FAR_FIND_DATA FindData;
	ScanTree ScTree(TRUE,TRUE,FALSE);
	ScTree.SetFindPath(Dir,L"*",0);

	while (ScTree.GetNextName(&FindData, strFullName))
	{
		apiSetFileAttributes(strFullName,FILE_ATTRIBUTE_NORMAL);

		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (ScTree.IsDirSearchDone())
				apiRemoveDirectory(strFullName);
		}
		else
			apiDeleteFile(strFullName);
	}

	apiSetFileAttributes(Dir,FILE_ATTRIBUTE_NORMAL);
	apiRemoveDirectory(Dir);
}

bool DeleteFileWithFolder(const string& FileName)
{
	bool Result = false;
	string strFileOrFolderName(FileName);
	Unquote(strFileOrFolderName);

	if (apiSetFileAttributes(strFileOrFolderName, FILE_ATTRIBUTE_NORMAL))
	{
		if (apiDeleteFile(strFileOrFolderName)) //BUGBUG
		{
			CutToSlash(strFileOrFolderName,true);
			Result = apiRemoveDirectory(strFileOrFolderName) != FALSE;
		}
	}
	return Result;
}
