#include <all_far.h>
#pragma hdrstop

#include "Int.h"

void SetupFileTimeNDescription(int OpMode,Connection *hConnect,LPCSTR nm)
{
	if(!IS_FLAG(OpMode,OPM_DESCR)) return;

	FILE *SrcFile=fopen(nm,"r+b");

	if(!SrcFile) return;

	int   FileSize = (int)Fsize(SrcFile);
	BYTE *Buf      = (BYTE*)malloc(sizeof(BYTE)*FileSize*3+1);
	int   ReadSize = (int)fread(Buf,1,FileSize,SrcFile);
	int WriteSize = hConnect->FromOEM(Buf,ReadSize,sizeof(BYTE)*FileSize*3+1);
	fflush(SrcFile);
	fseek(SrcFile,0,SEEK_SET);
	fflush(SrcFile);
	fwrite(Buf,1,WriteSize,SrcFile);
	fflush(SrcFile);
	fclose(SrcFile);
	free(Buf);
}

/****************************************
   PROCEDURES
     FTP::GetFiles
 ****************************************/
int FTP::_FtpPutFile(LPCSTR lpszLocalFile,LPCSTR lpszNewRemoteFile,BOOL Reput,int AsciiMode)
{
	PROC(("FTP::_FtpPutFile", "\"%s\",\"%s\",%d,%d", lpszLocalFile,lpszNewRemoteFile,Reput,AsciiMode))
	int rc;
	FtpSetRetryCount(hConnect,0);

	do
	{
		SetLastError(ERROR_SUCCESS);

		if(!hConnect)
			return FALSE;

		if((rc=FtpPutFile(hConnect,lpszLocalFile,lpszNewRemoteFile,Reput,AsciiMode)) != FALSE)
			return rc;

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

		if(!hConnect->ConnectMessageTimeout(MCannotUpload,lpszNewRemoteFile,-MRetry))
			return FALSE;

		Reput = TRUE;

		if(FtpCmdLineAlive(hConnect) &&
		        FtpKeepAlive(hConnect))
			continue;

		SaveUsedDirNFile();

		if(!Connect())
			return FALSE;
	}
	while(true);
}

int FTP::PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode)
{
	PROC(("PutFiles",NULL))
	int rc;
	LogPanelItems(PanelItem,ItemsNumber);
	rc = PutFilesINT(PanelItem,ItemsNumber,Move,OpMode);
	FtpCmdBlock(hConnect, FALSE);

	if(rc == FALSE || rc == -1)
		LongBeepEnd(TRUE);

	IncludeMask[0] = 0;
	ExcludeMask[0] = 0;
	Log(("rc:%d", rc));
	return rc;
}

int FTP::PutFilesINT(struct PluginPanelItem *PanelItem,int ItemsNumber, int Move,int OpMode)
{
	FP_Screen         _scr;
	FTPCopyInfo       ci;
	FP_SizeItemList   il;
	String            DestName;
	char             *CurName;
	FILETIME          CurTime;
	DWORD             DestAttr;
	FILETIME          DestTime;
	FTPFileInfo       FindData;
	int               mTitle;
	DWORD             SrcAttr;
	int               rc;

	if(!ItemsNumber) return 0;

	if(ShowHosts)    return PutHostsFiles(PanelItem,ItemsNumber,Move,OpMode);

	if(!hConnect)    return 0;

	FtpGetCurrentDirectory(hConnect, ci.DestPath);
	ci.asciiMode       = Host.AsciiMode;
	ci.AddToQueque     = FALSE;
	ci.MsgCode         = IS_FLAG(OpMode,OPM_FIND) ? ocOverAll : OverrideMsgCode;
	ci.ShowProcessList = FALSE;
	ci.Download        = FALSE;
	ci.UploadLowCase   = Opt.UploadLowCase;

	if(Opt.ShowUploadDialog &&
	        !IS_SILENT(OpMode) && !IS_FLAG(OpMode,OPM_NODIALOG))
	{
		if(!CopyAskDialog(Move,FALSE,&ci))
			return FALSE;

		LastMsgCode = ci.MsgCode;
	}

	if(hConnect->Host.ServerType!=FTP_TYPE_MVS)
		AddEndSlash(ci.DestPath,'/');

	if(!ExpandList(PanelItem,ItemsNumber,&il,FALSE))
		return FALSE;

	if(ci.ShowProcessList && !ShowFilesList(&il))
		return FALSE;

	if(ci.AddToQueque)
	{
		Log(("Files added to queue [%d]",il.Count()));
		ListToQueque(&il,&ci);
		return TRUE;
	}

	if(LongBeep)
		FP_PeriodReset(LongBeep);

	hConnect->TrafficInfo->Init(hConnect, MStatusUpload, OpMode, &il);

	for(int I = 0; I < il.Count(); I++)
	{
		String tmp;
		SrcAttr = il.List[I].FindData.dwFileAttributes;
		tmp     = FTP_FILENAME(&il.List[I]);
		CurName = tmp.c_str();
		CurTime = il.List[I].FindData.ftLastWriteTime;
		Log(("PutFiles: list[%d], %d-th, att: %d, dw:%08X,%08X \"%s\"",
		     il.Count(), I,
		     SrcAttr, il.List[I].FindData.dwReserved0, il.List[I].FindData.dwReserved1,
		     CurName));

		/* File name may contain relative paths.
		   Local files use '\' as dir separator, so convert
		   it to '/' for remote side, BUT only in case relative paths.
		*/
		if(IsAbsolutePath(CurName))
			CurName = PointToName(CurName);

		FixFTPSlash(tmp);

		//Skip deselected files
		if(il.List[I].FindData.dwReserved1 == MAX_DWORD)
		{
			Log(("PutFiles: skip delselected \"%s\"", CurName));
			continue;
		}

		if(ci.UploadLowCase && !IS_FLAG(SrcAttr,FILE_ATTRIBUTE_DIRECTORY))
		{
			char *Name = PointToName(CurName);
			//if ( !IsCaseMixed(Name))
			LocalLower(Name);
		}

		if(hConnect->Host.ServerType!=FTP_TYPE_MVS)
			DestName.printf("%s%s", ci.DestPath.c_str(), CurName);
		else
		{
			DestName=CurName;
			//if(DestName.Length()>8)DestName.SetLength(8);
		}

		if(IS_FLAG(SrcAttr,FILE_ATTRIBUTE_DIRECTORY))
		{
			if(FTPCreateDirectory(CurName,OpMode))
				if(I < ItemsNumber && Opt.UpdateDescriptions)
					PanelItem[I].Flags|=PPIF_PROCESSDESCR;

			continue;
		}

		DestAttr = MAX_DWORD;
		DestTime.dwLowDateTime = 0; // Local files are considered newer by default
		DestTime.dwHighDateTime = 0; // Local files are considered newer by default
		memset(&FindData.FindData, 0, sizeof(FindData.FindData));
		__int64 sz;
		//Check file exist
		FtpSetRetryCount(hConnect,0);

		do
		{
			SetLastError(ERROR_SUCCESS);

			if(FtpFindFirstFile(hConnect, DestName.c_str(), &FindData, NULL))
			{
				if(stricmp(PointToName(FTP_FILENAME(&FindData)),
				           PointToName(DestName.c_str())) == 0)
					DestAttr = FindData.FindData.dwFileAttributes;

				DestTime = FindData.FindData.ftLastWriteTime;
				break;
			}
			else if(!FtpCmdLineAlive(hConnect))
			{
				;
			}
			else if((sz=FtpFileSize(hConnect,DestName.c_str())) != -1)
			{
				FindData.FindData = il.List[I].FindData;
				FindData.FindData.nFileSizeHigh = (DWORD)((sz >> 32) & MAX_DWORD);
				FindData.FindData.nFileSizeLow  = (DWORD)(sz & MAX_DWORD);
				DestAttr = 0;
				//Can't find file date using FTP commands - leave the default value
				//DestTime = ;
				break;
			}
			else if(!FtpCmdLineAlive(hConnect))
			{
				;
			}
			else
				break;

			Log(("Conn lost!"));

			if(!hConnect)
			{
				BackToHosts();
				Invalidate();
				return FALSE;
			}

			if(GetLastError() == ERROR_CANCELLED)
				return IS_SILENT(FP_LastOpMode) ? (-1) : FALSE;

			int num = FtpGetRetryCount(hConnect);

			if(Opt.RetryCount > 0 && num >= Opt.RetryCount)
				return FALSE;

			FtpSetRetryCount(hConnect,num+1);

			if(!hConnect->ConnectMessageTimeout(MCannotUpload,DestName.c_str(),-MRetry))
				return FALSE;

			SaveUsedDirNFile();

			if(!Connect())
				return FALSE;
		}
		while(true);

		//Init transfer
		hConnect->TrafficInfo->InitFile(&il.List[I], FTP_FILENAME(&il.List[I]), DestName.c_str());

		//Ask over
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
			ci.MsgCode  = AskOverwrite(MUploadTitle, FALSE, &FindData.FindData, &il.List[I].FindData, ci.MsgCode,
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

//Upload
		if((rc=_FtpPutFile(FTP_FILENAME(&il.List[I]),
		                   DestName.c_str(),
		                   DestAttr == MAX_DWORD ? FALSE : (ci.MsgCode == ocResume || ci.MsgCode == ocResumeAll),
		                   ci.asciiMode)) == TRUE)
		{
			/*! FAR has a bug, so PanelItem stored in internal structure.
			    Because of this flags PPIF_SELECTED and PPIF_PROCESSDESCR has no effect at all.

			      if ( I < ItemsNumber ) {
			        CLR_FLAG( PanelItem[I].Flags,PPIF_SELECTED );
			        if (Opt.UpdateDescriptions) SET_FLAG( PanelItem[I].Flags,PPIF_PROCESSDESCR );
			      }
			*/
			//Process description
			SetupFileTimeNDescription(OpMode,hConnect,CurName);

			//Move source
			if(Move)
			{
				SetFileAttributes(CurName,0);
				remove(CurName);
			}

			mTitle = MOk;
		}
		else

//Cancelled
			if(rc == -1 || GetLastError() == ERROR_CANCELLED)
				return rc;
			else
			{
//Error uploading
				mTitle = MCannotUpload;
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
	}

	if(Move)
		for(int I=il.Count()-1; I>=0; I--)
		{
			if(CheckForEsc(FALSE))
				return -1;

			if(IS_FLAG(il.List[I].FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
				if(RemoveDirectory(FTP_FILENAME(&il.List[I])))
					if(I < ItemsNumber)
					{
						PanelItem[I].Flags&=~PPIF_SELECTED;

						if(Opt.UpdateDescriptions)
							PanelItem[I].Flags|=PPIF_PROCESSDESCR;
					}
		}

	return TRUE;
}
