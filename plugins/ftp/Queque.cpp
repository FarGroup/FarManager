#include <all_far.h>
#pragma hdrstop

#include "Int.h"

int AskDeleteQueue(void)
{
	LPCSTR MsgItems[] =
	{
		FMSG(MAttention),
		FMSG(MQDeleteItem),
		FMSG(MQSingleItem), FMSG(MQEntireList), FMSG(MCancel)
	};
	return FMessage(FMSG_WARNING, NULL, MsgItems, ARRAYSIZE(MsgItems),3);
}

void FTP::ClearQueue(void)
{
	FTPUrl *p, *p1;

	for(p = UrlsList; p; p = p1)
	{
		p1 = p->Next;
		delete p;
	}

	UrlsList = UrlsTail = NULL;
	QuequeSize = 0;
}

BOOL FTP::WarnExecuteQueue(QueueExecOptions* op)
{
	InitDialogItem InitItems[]=
	{
		{DI_DOUBLEBOX, 3, 1,72, 9, 0, 0,0,0,FMSG(MQueueParam)},

		{DI_CHECKBOX,5, 3,0,0,0,    0,0,0,FMSG(MQRestore)},
		{DI_CHECKBOX,5, 4,0,0,0,    0,0,0,FMSG(MQRemove)},

		{DI_CHECKBOX,5, 6,0,0,0,    0,0,0,FMSG(MQSave)},

		{DI_TEXT,3, 7,3, 7,DIF_BOXCOLOR|DIF_SEPARATOR,0,0,0,NULL },
		{DI_BUTTON,0, 8,0,0,DIF_CENTERGROUP, 0,0,1,   FMSG(MOk)},
		{DI_BUTTON,0, 8,0,0,DIF_CENTERGROUP, 0,0,0,   FMSG(MCancel)},
	};
	FarDialogItem  DialogItems[ARRAYSIZE(InitItems)];
//Create items
	InitDialogItems(InitItems,DialogItems,ARRAYSIZE(DialogItems));
//Set flags
	//Flags
	DialogItems[ 1].Selected = op->RestoreState;
	DialogItems[ 2].Selected = op->RemoveCompleted;

//Dialog
	if(FDialog(76,11,"FTPProcessQueue",DialogItems,ARRAYSIZE(DialogItems)) != 5)
		return FALSE;

//Get paras
	//Flags
	op->RestoreState     = DialogItems[ 1].Selected;
	op->RemoveCompleted  = DialogItems[ 2].Selected;

	//Save to default
	if(DialogItems[ 3].Selected)
	{
		Opt.RestoreState    = op->RestoreState;
		Opt.RemoveCompleted = op->RemoveCompleted;
		FP_SetRegKey("QueueRestoreState",    Opt.RestoreState);
		FP_SetRegKey("QueueRemoveCompleted", Opt.RemoveCompleted);
	}

	return TRUE;
}

void FTP::SetupQOpt(QueueExecOptions* op)
{
	op->RestoreState    = Opt.RestoreState;
	op->RemoveCompleted = Opt.RemoveCompleted;
}

LPCSTR FTP::InsertCurrentToQueue(void)
{
	PanelInfo   pi, api;
	FP_SizeItemList backup,il;
	FTPCopyInfo ci;

	if(!FP_Info->Control(this, FCTL_GETPANELINFO, &pi) ||
	        !FP_Info->Control(INVALID_HANDLE_VALUE, FCTL_GETANOTHERPANELSHORTINFO, &api))
		return FMSG(MErrGetPanelInfo);

	if(pi.SelectedItemsNumber <= 0 ||
	        pi.SelectedItemsNumber == 1 && !IS_FLAG(pi.SelectedItems[0].Flags,PPIF_SELECTED))
		return FMSG(MErrNoSelection);

	backup.Add(pi.SelectedItems, pi.SelectedItemsNumber);
	BOOL rc = ExpandList(backup.Items(), backup.Count(), &il, TRUE);
	FP_Screen::FullRestore();

	if(!rc)
		return GetLastError() == ERROR_CANCELLED ? NULL : FMSG(MErrExpandList);

	ci.Download = TRUE;

	if(api.PanelType != PTYPE_FILEPANEL || api.Plugin)
		ci.DestPath = "";
	else
		ci.DestPath = api.CurDir;

	ListToQueque(&il, &ci);
	return NULL;
}

LPCSTR FTP::InsertAnotherToQueue(void)
{
	FP_SizeItemList backup,il;
	PanelInfo       pi;
	FTPCopyInfo     ci;

	if(!hConnect || ShowHosts)
		return FMSG(MQErrUploadHosts);

	if(!FP_Info->Control(INVALID_HANDLE_VALUE, FCTL_GETANOTHERPANELINFO, &pi))
		return FMSG(MErrGetPanelInfo);

	if(pi.SelectedItemsNumber <= 0 ||
	        pi.SelectedItemsNumber == 1 && !IS_FLAG(pi.SelectedItems[0].Flags,PPIF_SELECTED))
		return FMSG(MErrNoSelection);

	if(pi.PanelType != PTYPE_FILEPANEL || pi.Plugin)
		return FMSG(MErrNotFiles);

	backup.Add(pi.SelectedItems, pi.SelectedItemsNumber);
	BOOL rc = ExpandList(backup.Items(), backup.Count(), &il, FALSE);
	FP_Screen::FullRestore();

	if(!rc)
		return GetLastError() == ERROR_CANCELLED ? NULL : FMSG(MErrExpandList);

	ci.Download = FALSE;
	GetCurPath(ci.DestPath);
	ListToQueque(&il, &ci);
	return NULL;
}

void FTP::InsertToQueue(void)
{
	static LPCSTR strings[] =
	{
		FMSG(MQISingle),
		FMSG(MQIFTP),
		FMSG(MQIAnother),
		NULL
	};
	FP_Menu  mnu(strings);
	int      sel;
	LPCSTR err;
	FTPUrl   tmp;

	do
	{
		sel = mnu.Execute(FMSG(MQAddTitle),FMENU_WRAPMODE,NULL,"FTPQueueAddItem");

		if(sel == -1)
			return;

		err = NULL;

		switch(sel)
		{
			case 0:
				UrlInit(&tmp);

				if(EditUrlItem(&tmp))
				{
					AddToQueque(&tmp);
					return;
				}

				break;
			case 1:
				err = InsertCurrentToQueue();

				if(!err && GetLastError() != ERROR_CANCELLED) return;

				break;
			case 2:
				err = InsertAnotherToQueue();

				if(!err && GetLastError() != ERROR_CANCELLED) return;

				break;
		}

		if(err)
		{
			static LPCSTR itms[] = { FMSG(MQErrAdding), NULL, FMSG(MOk) };
			itms[1] = err;
			FMessage(FMSG_WARNING,NULL,itms,3,1);
		}
	}
	while(true);
}

void FTP::QuequeMenu(void)
{
	int              n,
	  num;
	int              Breaks[] = { VK_DELETE, VK_INSERT, VK_F4, VK_RETURN, 0 },
	                            BNumber;
	FarMenuItem     *mi = NULL;
	FTPUrl*          p,*p1;
	char             str1[MAX_PATH],
	   str2[MAX_PATH],
	   str3[MAX_PATH];
	QueueExecOptions exOp;
	SetupQOpt(&exOp);
	num = -1;

	do
	{
		mi = (FarMenuItem *)realloc(mi, (QuequeSize+1)*sizeof(FarMenuItem));
		memset(mi, 0, QuequeSize*sizeof(FarMenuItem));

		for(p = UrlsList,n = 0; p; p = p->Next, n++)
		{
			StrCpy(str1, p->SrcPath.c_str(),    20);
			StrCpy(str2, p->DestPath.c_str(),   20);
			StrCpy(str3, p->FileName.cFileName, 20);
			_snprintf(mi[n].Text, ARRAYSIZE(mi[n].Text),
			          "%c%c %-20s%c%-20s%c%-20s",
			          p->Download ? '-' : '<', p->Download ? '>' : '-',
			          str1, FAR_VERT_CHAR,
			          str2, FAR_VERT_CHAR,
			          str3);

			if(p->Error[0])
				mi[n].Checked = TRUE;
		}

		//Title
		char title[MAX_PATH];
		_snprintf(title, ARRAYSIZE(title), "%s: %d %s", FP_GetMsg(MQMenuTitle), n, FP_GetMsg(MQMenuItems));

		//Menu
		if(num != -1 && num < QuequeSize) mi[num].Selected = TRUE;

		n = FP_Info->Menu(FP_Info->ModuleNumber,-1,-1,0,FMENU_SHOWAMPERSAND,
		                  title,
		                  FP_GetMsg(MQMenuFooter),
		                  "FTPQueue", Breaks, &BNumber, mi, QuequeSize);

		//key ESC
		if(BNumber == -1 &&
		        n == -1)
			goto Done;

		//key Enter
		if(BNumber == -1)
		{
			//??
			goto Done;
		}

		//Set selected
		if(num != -1) mi[num].Selected = FALSE;

		num = n;

		//Process keys
		switch(BNumber)
		{
				/*DEL*/
			case 0:

				if(QuequeSize)
					switch(AskDeleteQueue())
					{
						case -1:
						case  2:
							break;
						case  0:
							p = UrlItem(n, &p1);
							DeleteUrlItem(p, p1);
							break;
						case  1:
							ClearQueue();
							break;
					}

				break;
				/*Ins*/
			case 1:
				InsertToQueue();
				break;
				/*F4*/
			case 2:
				p = UrlItem(n, NULL);

				if(p)
					EditUrlItem(p);

				break;
				/*Return*/
			case 3:

				if(QuequeSize &&
				        WarnExecuteQueue(&exOp))
				{
					ExecuteQueue(&exOp);

					if(!QuequeSize)
						goto Done;
				}

				break;
		}
	}
	while(true);

Done:
	free(mi);
}

void FTP::AddToQueque(FTPUrl* item,int pos/*-1*/)
{
	FTPUrl *p, *p1, *newi;
	newi = new FTPUrl;
	*newi = *item;

	if(pos == -1) pos = QuequeSize;

	p = UrlItem(pos, &p1);

	if(p1) p1->Next = newi;

	newi->Next = p;

	if(p == UrlsList) UrlsList = newi;

	QuequeSize++;
}

void FTP::AddToQueque(FAR_FIND_DATA* FileName, LPCSTR Path, BOOL Download)
{
	String  str;
	char   *m;
	int     num;
	FTPUrl* p = new FTPUrl;
	memcpy(&p->Host, &Host, sizeof(Host));
	p->Download = Download;
	p->Next     = NULL;
	p->FileName = *FileName;
	p->Error.Null();
	p->DestPath = Path;

	if(Download)
		m = strrchr(FileName->cFileName, '/');
	else
		m = strrchr(FileName->cFileName, '\\');

	if(m)
	{
		*m = 0;
		p->DestPath.Add(m);
		memmove(FileName->cFileName, m+1, m-FileName->cFileName);
	}

	if(Download)
	{
		GetCurPath(p->SrcPath);
		AddEndSlash(p->SrcPath, '/');
		str.printf("%s%s", p->SrcPath.c_str(), FileName->cFileName);
		FixLocalSlash(p->DestPath);
		AddEndSlash(p->DestPath, '\\');
		num = str.Chr('/');
	}
	else
	{
		PanelInfo pi;
		FP_Info->Control(this, FCTL_GETANOTHERPANELINFO, &pi);
		p->SrcPath = pi.CurDir;
		AddEndSlash(p->SrcPath, '\\');
		str.printf("%s%s", p->SrcPath.c_str(), FileName->cFileName);
		FixLocalSlash(str);
		AddEndSlash(p->DestPath, '/');
		num = str.Chr('\\');
	}

	if(num != -1)
	{
		StrCpy(p->FileName.cFileName, str.c_str()+num+1, ARRAYSIZE(p->FileName.cFileName));
		str.SetLength(num);
		p->SrcPath = str;
	}
	else
	{
		StrCpy(p->FileName.cFileName, str.c_str(), ARRAYSIZE(p->FileName.cFileName));
		p->SrcPath.Null();
	}

	if(!UrlsList) UrlsList = p;

	if(UrlsTail)  UrlsTail->Next = p;

	UrlsTail = p;
	QuequeSize++;
}

void FTP::ListToQueque(FP_SizeItemList* il, FTPCopyInfo* ci)
{
	for(int n = 0; n < il->Count(); n++)
	{
		FAR_FIND_DATA* p = &il->Item(n)->FindData;

		//Skip dirs
		if(IS_FLAG(p->dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
			continue;

		//Skip deselected in list
		if(p->dwReserved1 == MAX_DWORD)
			continue;

		AddToQueque(p, ci->DestPath.c_str(), ci->Download);
	}
}

void FTP::ExecuteQueue(QueueExecOptions* op)
{
	if(!QuequeSize) return;

	FTPHost   oHost = Host;
	BOOL      oShowHosts = ShowHosts;
	char      oDir[MAX_PATH];
	GetCurPath(oDir, ARRAYSIZE(oDir));
	OverrideMsgCode = ocNone;
	ExecuteQueueINT(op);
	OverrideMsgCode = ocNone;

//Restore plugin state
	if(op->RestoreState)
	{
		if(oShowHosts)
		{
			BackToHosts();
		}
		else if(!Host.CmpConnected(&oHost))
		{
			Host = oHost;
			FullConnect();
			ResetCache=TRUE;
		}

		SetDirectory(oDir,0);
		Invalidate();
	}
}

void FTP::ExecuteQueueINT(QueueExecOptions* op)
{
	PROC(("ExecuteQueueINT","%d,%d",op->RestoreState,op->RemoveCompleted))
	FP_Screen       _scr;
	String          DefPath, LastPath, LastName;
	BOOL            rc;
	BOOL            needUpdate = FALSE;
	FTPUrl          *prev,*p,*tmp;
	FTPCopyInfo     ci;
	FAR_FIND_DATA   fd, ffd;
	//Copy info
	ci.asciiMode       = Host.AsciiMode;
	ci.ShowProcessList = FALSE;
	ci.AddToQueque     = FALSE;
	ci.MsgCode         = ocNone;
	ci.UploadLowCase   = Opt.UploadLowCase;
	//Check othe panel info
	PanelInfo pi;
	FP_Info->Control(INVALID_HANDLE_VALUE, FCTL_GETANOTHERPANELINFO, &pi);

	if(pi.PanelType != PTYPE_FILEPANEL ||
	        pi.Plugin)
		DefPath.Null();
	else
		DefPath = pi.CurDir;

	//DO full list
	prev        = NULL;
	p           = UrlsList;
	LastPath.Null();
	LastName.Null();

	while(p)
	{
//Check current host the same
		Log(("Queue: Check current host the same"));

		if(!hConnect ||
		        !Host.CmpConnected(&p->Host))
		{
			Host = p->Host;

			if(!FullConnect())
			{
				if(GetLastError() == ERROR_CANCELLED) break;

				p->Error.printf("%s: %s", FP_GetMsg(MQCanNotConnect), __WINError());
				goto Skip;
			}

			ResetCache=TRUE;
		}

//Apply other parameters
		Log(("Queue: Apply other parameters"));
		Host = p->Host;
		hConnect->InitData(&Host,-1);
		hConnect->InitIOBuff();
//Change local dir
		Log(("Queue: Change local dir"));

		do
		{
			char *m = p->Download ? p->DestPath.c_str() : p->SrcPath.c_str();

			if(!m[0]) m = DefPath.c_str();

			if(!m[0])
			{
				p->Error = FP_GetMsg(MQNotLocal);
				goto Skip;
			}

			if(SetCurrentDirectory(m)) break;

			if(DoCreateDirectory(m))
				if(SetCurrentDirectory(m)) break;

			p->Error.printf(FP_GetMsg(MQCanNotChangeLocal), m, __WINError());
			goto Skip;
		}
		while(0);

//Check local file
		Log(("Queue: Check local file"));

		if(!p->Download)
		{
			if(!FRealFile(p->FileName.cFileName, &fd))
			{
				p->Error.printf(FP_GetMsg(MQNotFoundSource), p->FileName.cFileName, __WINError());
				goto Skip;
			}
		}

//IO file
		Log(("Queue: IO file"));
		//Last used FTP path and name
		LastPath = p->Download ? p->SrcPath : p->DestPath;
		LastName = PointToName(p->FileName.cFileName);

		//DOWNLOAD ------------------------------------------------
		if(p->Download)
		{
			ci.Download  = TRUE;
			ci.SrcPath = p->SrcPath;
			AddEndSlash(ci.SrcPath, '/');
			ci.SrcPath.cat(p->FileName.cFileName);

			if(p->DestPath.Length())
			{
				FixFileNameChars(p->DestPath);
				ci.DestPath = p->DestPath;
			}
			else
				ci.DestPath = DefPath;

			AddEndSlash(ci.DestPath, '\\');
			ci.DestPath.cat(FixFileNameChars(p->FileName.cFileName,TRUE));
			__int64 fsz = FtpFileSize(hConnect, ci.SrcPath.c_str());
			hConnect->TrafficInfo->Init(hConnect, MStatusDownload, 0, NULL);
			hConnect->TrafficInfo->InitFile(fsz, ci.SrcPath.c_str(), ci.DestPath.c_str());

			if(FRealFile(ci.DestPath.c_str(),&fd))
			{
				if(fsz != -1)
				{
					ffd = fd;
					ffd.nFileSizeHigh = (DWORD)((fsz >> 32) & MAX_DWORD);
					ffd.nFileSizeLow  = (DWORD)(fsz & MAX_DWORD);
					ci.MsgCode  = AskOverwrite(MDownloadTitle, TRUE, &fd, &ffd, ci.MsgCode, FALSE);
				}
				else
					ci.MsgCode  = AskOverwrite(MDownloadTitle, TRUE, &fd, NULL, ci.MsgCode, FALSE);

				switch(ci.MsgCode)
				{
					case   ocOverAll:
					case      ocOver:
						break;
					case      ocSkip:
					case   ocSkipAll:
						goto Skip;
					case    ocResume:
					case ocResumeAll:
						break;
					case     ocNewer:
					case  ocNewerAll:
						goto Skip;
				}

				if(ci.MsgCode == ocCancel)
				{
					SetLastError(ERROR_CANCELLED);
					break;
				}
			}

			rc = _FtpGetFile(ci.SrcPath.c_str(),
			                 ci.DestPath.c_str(),
			                 ci.MsgCode == ocResume || ci.MsgCode == ocResumeAll,
			                 ci.asciiMode);
		}
		else
		{
			//UPLOAD -------------------------------------------------
			ci.Download  = FALSE;
			ci.SrcPath = p->SrcPath;
			AddEndSlash(ci.SrcPath, '\\');
			ci.SrcPath.cat(PointToName(p->FileName.cFileName));

			if(p->DestPath[0])
				ci.DestPath = p->DestPath;
			else
				GetCurPath(ci.DestPath);

			AddEndSlash(ci.DestPath, '/');
			ci.DestPath.cat(PointToName(p->FileName.cFileName));
			__int64 fsz = FtpFileSize(hConnect, ci.DestPath.c_str());
			hConnect->TrafficInfo->Init(hConnect, MStatusUpload, 0, NULL);
			hConnect->TrafficInfo->InitFile(&fd, ci.SrcPath.c_str(), ci.DestPath.c_str());

			if(fsz != -1)
			{
				ffd = fd;
				ffd.nFileSizeHigh = (DWORD)((fsz >> 32) & MAX_DWORD);
				ffd.nFileSizeLow  = (DWORD)(fsz & MAX_DWORD);
				ci.MsgCode  = AskOverwrite(MUploadTitle, FALSE, &ffd, &fd, ci.MsgCode, FALSE);

				switch(ci.MsgCode)
				{
					case   ocOverAll:
					case      ocOver:
						break;
					case      ocSkip:
					case   ocSkipAll:
						goto Skip;
					case    ocResume:
					case ocResumeAll:
						break;
					case     ocNewer:
					case  ocNewerAll:
						goto Skip;
				}

				if(ci.MsgCode == ocCancel)
				{
					SetLastError(ERROR_CANCELLED);
					break;
				}

				needUpdate = TRUE;
			}

			rc = _FtpPutFile(ci.SrcPath.c_str(),
			                 ci.DestPath.c_str(),
			                 ci.MsgCode == ocResume || ci.MsgCode == ocResumeAll,
			                 ci.asciiMode);
			needUpdate = needUpdate || rc == TRUE;
		}

//IO completed
		if(rc == -1 ||
		        !rc && GetLastError() == ERROR_CANCELLED)
		{
			SetLastError(ERROR_CANCELLED);
			break;
		}

		if(!rc)
		{
			if(p->Download)
				p->Error.printf(FP_GetMsg(MQErrDowload), __WINError());
			else
				p->Error.printf(FP_GetMsg(MQErrUpload), __WINError());

			goto Skip;
		}

//Done
		Log(("Queue: Done"));
		tmp = p->Next;

		if(op->RemoveCompleted)
			DeleteUrlItem(p, prev);

		p = tmp;
		continue;
//Error
Skip:
		Log(("Queue: Error"));
		prev = p;
		p    = p->Next;
	}

//Reread files on FTP in case files are uploaded
	if(!ShowHosts &&
	        hConnect &&
	        FtpCmdLineAlive(hConnect) &&
	        FtpKeepAlive(hConnect))
	{
		if(!op->RestoreState)
		{
			if(LastPath.Length()) SetDirectoryStepped(LastPath.c_str(), TRUE);

			if(LastName.Length()) SelectFile = LastName;
		}

		FP_Screen::FullRestore();

		if(needUpdate)
			Reread();
		else
			Invalidate();
	}
}
