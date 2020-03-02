#include <all_far.h>
#pragma hdrstop

#include "Int.h"

/****************************************
   PROCEDURES
     FTP::GetFiles
 ****************************************/
void SetupFileTimeNDescription(int OpMode,Connection *hConnect,LPCSTR nm,FILETIME *tm)
{
	HANDLE SrcFile = CreateFile(nm,GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	DWORD  FileSize;
	BYTE  *Buf;

	if(SrcFile == INVALID_HANDLE_VALUE)
		return;

	if(IS_FLAG(OpMode,OPM_DESCR) &&
	        (FileSize=GetFileSize(SrcFile,NULL)) != 0xFFFFFFFFUL)
	{
		Buf      = (BYTE*)malloc(sizeof(BYTE)*FileSize);
		ReadFile(SrcFile,Buf,FileSize,&FileSize,NULL);
		DWORD WriteSize = hConnect->ToOEM(Buf,FileSize);
		SetFilePointer(SrcFile,0,NULL,FILE_BEGIN);
		WriteFile(SrcFile,Buf,WriteSize,&WriteSize,NULL);
		SetEndOfFile(SrcFile);
		free(Buf);
	}

	if(tm)
		SetFileTime(SrcFile,NULL,NULL,tm);

	CloseHandle(SrcFile);
}

int FTP::_FtpGetFile(LPCSTR lpszRemoteFile,LPCSTR lpszNewFile,BOOL Reget,int AsciiMode)
{
	/* Local file allways use '\' as separator.
	   Regardles of remote settings.
	*/
	FixLocalSlash((char*)lpszNewFile);   //Hack it to (char*).
	FtpSetRetryCount(hConnect,0);
	int   rc;

	do
	{
		SetLastError(ERROR_SUCCESS);

		if(!hConnect)
			return FALSE;

		OperateHidden(lpszNewFile, TRUE);

		if((rc=FtpGetFile(hConnect,lpszRemoteFile,lpszNewFile,Reget,AsciiMode)) != FALSE)
		{
			OperateHidden(lpszNewFile, FALSE);
			return rc;
		}

		if(GetLastError() == ERROR_CANCELLED)
		{
			Log(("GetFileCancelled: op:%d",IS_SILENT(FP_LastOpMode)));
			return IS_SILENT(FP_LastOpMode) ? (-1) : FALSE;
		}

		if(hConnect->SysErr())
			return IS_SILENT(FP_LastOpMode) ? (-1) : FALSE;

		int num = FtpGetRetryCount(hConnect);

		if(Opt.RetryCount > 0 && num >= Opt.RetryCount)
			return FALSE;

		FtpSetRetryCount(hConnect,num+1);

		if(!hConnect->ConnectMessageTimeout(MCannotDownload,lpszRemoteFile,-MRetry))
			return FALSE;

		Reget = TRUE;

		if(FtpCmdLineAlive(hConnect) &&
		        FtpKeepAlive(hConnect))
			continue;

		SaveUsedDirNFile();

		if(!Connect())
			return FALSE;
	}
	while(true);
}

int FTP::GetFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,String& DestPath,int OpMode)
{
	PROC(("FTP::GetFiles","cn:%d, %s [%s] op:%08X",ItemsNumber, Move ? "MOVE" : "COPY", DestPath, OpMode))
	LogPanelItems(PanelItem,ItemsNumber);
	//Copy received items to internal data and use copy
	// because FAR delete it on next FCTL_GETPANELINFO
	FP_SizeItemList FilesList;
	FilesList.Add(PanelItem,ItemsNumber);
	++SkipRestoreScreen;
	int rc = GetFilesInterface(FilesList.Items(), FilesList.Count(),
	                           Move, DestPath, OpMode);
	--SkipRestoreScreen;
	FtpCmdBlock(hConnect, FALSE);

	if(rc == FALSE || rc == -1)
		LongBeepEnd(TRUE);

	IncludeMask[0] = 0;
	ExcludeMask[0] = 0;
	Log(("rc:%d", rc));
	return rc;
}

int FTP::GetFilesInterface(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,String& DestPath,int OpMode)
{
	FTP             *other = OtherPlugin(this);
	FTPCopyInfo      ci;
	FP_SizeItemList  il;
	char            *CurName;
	FILETIME         CurTime;
	String           DestName;
	FP_Screen        _scr;
	int              i,isDestDir;
	FTPFileInfo      FindData;
	DWORD            DestAttr;
	FILETIME         DestTime;
	int              mTitle;
	int              rc;

	if(!ItemsNumber)
		return FALSE;

	//Hosts
	if(ShowHosts)
		return GetHostFiles(PanelItem,ItemsNumber,Move,DestPath,OpMode);

	if(!ShowHosts && !hConnect)
		return FALSE;

	Log(("Dest: %s", DestPath.c_str()));
	ci.DestPath        = DestPath;
	ci.asciiMode       = Host.AsciiMode;
	ci.ShowProcessList = FALSE;
	ci.AddToQueque     = FALSE;
	ci.MsgCode         = IS_SILENT(OpMode) ? ocOverAll : OverrideMsgCode;
	ci.Download        = TRUE;
	ci.UploadLowCase   = Opt.UploadLowCase;
	ci.FTPRename       = strpbrk(ci.DestPath.c_str(), "/\\") == NULL;

	if(!IS_SILENT(OpMode) && !IS_FLAG(OpMode,OPM_NODIALOG))
	{
		if(!CopyAskDialog(Move,TRUE,&ci))
			return FALSE;

		LastMsgCode = ci.MsgCode;
	}

	i = ci.DestPath.Length()-1;

//Rename only if move
	if(!Move && ci.FTPRename)
		ci.FTPRename = FALSE;

//Unquote
	if(ci.DestPath[0] == '\"' && ci.DestPath[i] == '\"')
	{
		i -= 2;
		ci.DestPath.Del(0, 1);
		ci.DestPath.Del(i, 1);
	}

//Remove prefix
	if(ci.DestPath.Cmp("ftp:",4))
	{
		i -= 4;
		ci.DestPath.Del(0, 4);
	}

	do
	{
		//Copy - allways on local disk
		if(!Move)
		{
			isDestDir = TRUE;
			AddEndSlash(ci.DestPath,'\\');
			break;
		}

		//Rename of server
		if(ci.FTPRename)
		{
			isDestDir = ci.DestPath[i] == '/';
			break;
		}

		//Move to the other panel
		if(Move && other)
		{
			String s;
			other->GetCurPath(s);

			if(ci.DestPath == s)
			{
				isDestDir    = TRUE;
				ci.FTPRename = TRUE;
				break;
			}
		}

		//Single name
		if(strpbrk(ci.DestPath.c_str(), "/\\") == NULL)
		{
			isDestDir    = FALSE;
			ci.FTPRename = TRUE;
			break;
		}

		//Path on server
		if(ci.DestPath.Chr('\\') != -1 && ci.DestPath[i] == '/')
		{
			isDestDir    = TRUE;
			ci.FTPRename = TRUE;
			break;
		}

		//Path on local disk
		isDestDir = TRUE;
		AddEndSlash(ci.DestPath,'\\');
	}
	while(0);

//Check for move to parent folder
	if(ci.FTPRename)
		if(ci.DestPath.Cmp(".."))
		{
			String s;
			FtpGetCurrentDirectory(hConnect, s);
			i = s.Length();

			if(i > 1)
			{
				for(i-=2; i && s[i] != '/' && s[i] != '\\'; i--);

				s.SetLength(i+1);
			}

			ci.DestPath = s;
			isDestDir = TRUE;
		}

//Create items list
	if(ci.FTPRename)
		il.Add(PanelItem, ItemsNumber);
	else if(!ExpandList(PanelItem,ItemsNumber,&il,TRUE,NULL,NULL))
		return FALSE;

	if(ci.ShowProcessList && !ShowFilesList(&il))
		return FALSE;

	if(!ci.FTPRename && ci.AddToQueque)
	{
		Log(("Files added to queue [%d]",il.Count()));
		ListToQueque(&il,&ci);
		return TRUE;
	}

	if(LongBeep)
		FP_PeriodReset(LongBeep);

//Calc full size
	hConnect->TrafficInfo->Init(hConnect, MStatusDownload, OpMode, &il);

//Copy\Rename each item
	for(i = 0; i < il.Count(); i++)
	{
		PluginPanelItem *CurPanelItem;
		CurPanelItem = &il.List[i];
		CurName      = FTP_FILENAME(CurPanelItem);
		CurTime      = CurPanelItem->FindData.ftLastWriteTime;
		DestAttr     = MAX_DWORD;
		DestTime.dwLowDateTime = 0;
		DestTime.dwHighDateTime = 0;

		//Skip deselected in list
		if(CurPanelItem->FindData.dwReserved1 == MAX_DWORD)
			continue;

		//Rename on ftp
		if(ci.FTPRename)
		{
			if(isDestDir)
			{
				DestName = ci.DestPath;

				if(hConnect->Host.ServerType!=FTP_TYPE_MVS)
					AddEndSlash(DestName, '/');

				DestName.cat(CurName);
			}
			else
				DestName = ci.DestPath;

			Log(("Rename [%s] to [%s]",CurName,DestName.c_str()));

			if(FtpFindFirstFile(hConnect, DestName.c_str(), &FindData, NULL))
				if(stricmp(FTP_FILENAME(&FindData), CurName) == 0)
				{
					DestAttr = FindData.FindData.dwFileAttributes;
					DestTime = FindData.FindData.ftLastWriteTime;
				}
		}
		else
		{
			//Copy to local disk
			if(isDestDir)
				DestName.printf("%s%s", ci.DestPath.c_str(), FixFileNameChars(CurName,TRUE));
			else
				DestName = FixFileNameChars(ci.DestPath);

			FixLocalSlash(DestName);

			//Create directory when copy
			if(IS_FLAG(CurPanelItem->FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
				continue;

			//
			if(FRealFile(DestName.c_str(),&FindData.FindData))
			{
				Log(("Real file: [%s]",DestName.c_str()));
				DestAttr = GetFileAttributes(DestName.c_str());
				DestTime = FindData.FindData.ftLastWriteTime;
			}
		}

		//Init current
		hConnect->TrafficInfo->InitFile(CurPanelItem,CurName,DestName.c_str());

		//Query overwrite
		switch(ci.MsgCode)
		{
			case      ocOver:
			case      ocSkip:
			case    ocResume:
			case     ocNewer:
				ci.MsgCode = ocNone;
				break;
		}

		if(DestAttr != MAX_DWORD)
		{
			ci.MsgCode = AskOverwrite(ci.FTPRename ? MRenameTitle : MDownloadTitle, TRUE,
			                          &FindData.FindData, &CurPanelItem->FindData, ci.MsgCode,
			                          ((CurTime.dwLowDateTime || CurTime.dwHighDateTime) && (DestTime.dwLowDateTime || DestTime.dwHighDateTime)));
			LastMsgCode = ci.MsgCode;

			switch(ci.MsgCode)
			{
				case   ocOverAll:
				case      ocOver:
					break;
				case      ocSkip:
				case   ocSkipAll:
					hConnect->TrafficInfo->Skip();
					continue;
				case    ocResume:
				case ocResumeAll:
					break;
				case     ocNewer:
				case  ocNewerAll:

					if(CompareFileTime(&CurTime, &DestTime) <= 0)
					{
						hConnect->TrafficInfo->Skip();
						continue;
					}

					break;
				case    ocCancel:
					return -1;
			}
		}

		//Reset local attrs
		if(!ci.FTPRename && DestAttr != MAX_DWORD &&
		        (DestAttr & (FILE_ATTRIBUTE_READONLY|0/*FILE_ATTRIBUTE_HIDDEN*/)) != 0)
			SetFileAttributes(DestName.c_str(), DestAttr & ~(FILE_ATTRIBUTE_READONLY|0/*FILE_ATTRIBUTE_HIDDEN*/));

		mTitle = MOk;

//Do rename
		if(ci.FTPRename)
		{
			//Rename
			if(!FtpRenameFile(hConnect,CurName,DestName.c_str()))
			{
				FtpConnectMessage(hConnect,
				                  MErrRename,
				                  Message("\"%s\" to \"%s\"",CurName,DestName.c_str()),
				                  -MOk);
				return FALSE;
			}
			else
			{
				SelectFile = DestName;
				ResetCache = TRUE;
			}
		}
		else

//Do download
			if((rc=_FtpGetFile(CurName, DestName.c_str(),
			                   ci.MsgCode == ocResume || ci.MsgCode == ocResumeAll,
			                   ci.asciiMode)) == TRUE)
			{
				/*! FAR has a bug, so PanelItem stored in internal structure.
				    Because of this flags PPIF_SELECTED and PPIF_PROCESSDESCR has no effect at all.

				      if ( i < ItemsNumber ) {
				        CLR_FLAG( PanelItem[i].Flags,PPIF_SELECTED );
				        if (Opt.UpdateDescriptions)
				          SET_FLAG( PanelItem[i].Flags,PPIF_PROCESSDESCR );
				      }
				*/
				//Process description
				SetupFileTimeNDescription(OpMode,hConnect,DestName.c_str(),&CurPanelItem->FindData.ftLastWriteTime);

				//Delete source after download
				if(Move)
				{
					if(FtpDeleteFile(hConnect,CurName))
					{
						if(Opt.UpdateDescriptions && i < ItemsNumber)
							PanelItem[i].Flags |= PPIF_PROCESSDESCR;
					}
					else
						mTitle = MCannotDelete;
				}
			}
			else
			{
//Error downloading
				//Cancelled
				if(rc == -1 || GetLastError() == ERROR_CANCELLED)
					return rc;
				else
					//Other error
					mTitle = MCannotDownload;
			}

//Process current file finished
		//All OK
		if(mTitle == MOk || mTitle == MNone__)
			continue;

		//Connection lost
		if(!hConnect)
		{
			BackToHosts();
			Invalidate();
		}

		//Return error
		return FALSE;
	}/*EACH FILE*/

//Remove empty directories after file deletion complete
	if(Move && !ci.FTPRename)
		for(int i=il.Count()-1; i>=0; i--)
		{
			if(CheckForEsc(FALSE))
				return -1;

			CurName = FTP_FILENAME(&il.List[i]);

			if(IS_FLAG(il.List[i].FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
				if(FtpRemoveDirectory(hConnect,CurName))
				{
					if(i < ItemsNumber)
					{
						PanelItem[i].Flags &= ~PPIF_SELECTED;

						if(Opt.UpdateDescriptions)
							PanelItem[i].Flags |= PPIF_PROCESSDESCR;
					}
				}
		}

	return 1;
}
