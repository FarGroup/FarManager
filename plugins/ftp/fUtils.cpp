#include <all_far.h>
#pragma hdrstop

#include "Int.h"
#include "Crypt.inc"

//---------------------------------------------------------------------------------
void FTP::LongBeepEnd(BOOL DoNotBeep /*= FALSE*/)
{
	if(LongBeep)
	{
		if(FP_PeriodEnd(LongBeep) && !DoNotBeep)
		{
			MessageBeep(MB_ICONASTERISK);
		}

		FP_PeriodDestroy(LongBeep);
		LongBeep = NULL;
	}
}
void FTP::LongBeepCreate(void)
{
	if(Opt.LongBeepTimeout)
	{
		LongBeepEnd(FALSE);
		LongBeep = FP_PeriodCreate(Opt.LongBeepTimeout*1000);
	}
}
//---------------------------------------------------------------------------------
void FTP::SaveUsedDirNFile(void)
{
	PROC(("SaveUsedDirNFile","was:(%s,%s)",Host.Home,SelectFile.c_str()))
	PanelInfo pi;

	//Save current file to restore
	if(!ShowHosts && hConnect)
	{
		String s;
		FtpGetCurrentDirectory(hConnect,s);
		StrCpy(Host.Home, s.c_str(), ARRAYSIZE(Host.Home));
	}

	//Save current file to restore
	if(FP_Info->Control(this,FCTL_GETPANELINFO,&pi))
	{
		if(pi.ItemsNumber > 0 && pi.CurrentItem < pi.ItemsNumber)
		{
			SelectFile = FTP_FILENAME(&pi.PanelItems[pi.CurrentItem]);
			Log(("SetLastHost: [%s]", SelectFile.c_str()));
		}

		Log(("Saved (%s,%s)",Host.Home,SelectFile.c_str()));
	}
}

void FTP::GetCurPath(char *buff,int bsz)
{
	if(ShowHosts)
	{
		StrCpy(buff, HostsPath, bsz);
		return;
	}

	String s;

	if(!FtpGetCurrentDirectory(hConnect,s))
	{
		buff[0] = 0;
		return;
	}

	StrCpy(buff, s.c_str(), bsz);
}

void FTP::GetCurPath(String& buff)
{
	if(ShowHosts)
		buff = HostsPath;
	else if(!FtpGetCurrentDirectory(hConnect,buff))
		buff.Null();
}

void FTP::FTP_FixPaths(LPCSTR base, PluginPanelItem *p, int cn, BOOL FromPlugin)
{
	String str;

	if(!base || !base[0])
		return;

	for(; cn--; p++)
	{
		char *CurName = FTP_FILENAME(p);

		if(StrCmp(CurName,"..") == 0 ||
		        StrCmp(CurName,".") == 0)
			continue;

		str.printf("%s%c%s", base, FromPlugin ? '/' : '\\', CurName);
		StrCpy(p->FindData.cFileName, str.c_str(), ARRAYSIZE(p->FindData.cFileName));

		if(str.Length() >= (int)ARRAYSIZE(p->FindData.cFileName))
			FPIL_ADDSET(p, str.Length()+1, strdup(str.c_str()));
		else
			FPIL_ADDSET(p, 0, NULL);
	}
}

int FTP::ExpandList(PluginPanelItem *pi,int icn,FP_SizeItemList* il,BOOL FromPlugin,ExpandListCB cb,LPVOID Param)
{
	PROC(("ExpandList","cn:%d, ilcn:%d/%d, %s, cb:%08X",icn,il ? il->Count() : 0,il ? il->MaxCount : 0,FromPlugin?"PLUGIN":"LOCAL",cb))
	BOOL             pSaved  = Host.Home[0] && SelectFile.Length();
	BOOL             old_ext = hConnect ? hConnect->Host.ExtCmdView : FALSE;
	FTPCurrentStates olds = CurrentState;
	int              rc;
	{
		FTPConnectionBreakable _brk(hConnect,FALSE);
		CurrentState  = fcsExpandList;

		if(hConnect)
		{
			hConnect->Host.ExtCmdView = FALSE;
			hConnect->CurrentState    = fcsExpandList;
		}

		if(!pSaved)
			SaveUsedDirNFile();

		rc = ExpandListINT(pi,icn,il,FromPlugin,cb,Param);

		if(hConnect)
		{
			hConnect->Host.ExtCmdView = old_ext;
			hConnect->CurrentState    = olds;
		}

		CurrentState  = olds;
	}
	Log(("ExpandList rc=%d",rc));
#if defined(__FILELOG__)

	if(rc)
	{
		Log(("Expand succ ends containing:"));

		if(il)
			LogPanelItems(il->Items(), il->Count());
		else
			Log(("Files list does not contains files"));
	}

#endif

	if(!pSaved)
	{
		if(!rc)
		{
			SaveLastError _err;

			if(Host.Home[0])
			{
				char str[MAX_PATH];
				GetCurPath(str, ARRAYSIZE(str));

				if(StrCmpI(str, Host.Home) != 0)
					SetDirectory(Host.Home,FP_LastOpMode);
			}
		}
		else
		{
			SelectFile   = "";
			Host.Home[0] = 0;
		}
	}

	return rc;
}

//---------------------------------------------------------------------------------
BOOL FTP::FTP_SetDirectory(LPCSTR dir,BOOL FromPlugin)
{
	PROC(("FTP::FTP_SetDirectory", "%s,%d", dir,FromPlugin))

	if(FromPlugin)
		return SetDirectory(dir,OPM_SILENT) == TRUE;
	else
		return SetCurrentDirectory(dir);
}

BOOL FTP::FTP_GetFindData(PluginPanelItem **PanelItem,int *ItemsNumber,BOOL FromPlugin)
{
	if(FromPlugin)
	{
		Log(("PLUGIN GetFindData"));
		return GetFindData(PanelItem,ItemsNumber,OPM_SILENT);
	}

	PROC(("LOCAL GetFindData", NULL))
	FP_SizeItemList il(FALSE);
	WIN32_FIND_DATA fd;
	PluginPanelItem p;
	HANDLE          h = FindFirstFile("*.*",&fd);

	if(h == INVALID_HANDLE_VALUE)
	{
#if defined(__FILELOG__)
		char path[MAX_PATH];
		path[GetCurrentDirectory(ARRAYSIZE(path),path)] = 0;
		Log(("Files in [%s] not found: %s", path, __WINError()));
#endif
		*PanelItem   = NULL;
		*ItemsNumber = 0;
		return TRUE;
	}
	else
	{
#if defined(__FILELOG__)
		char path[MAX_PATH];
		path[GetCurrentDirectory(ARRAYSIZE(path),path)] = 0;
		Log(("Files in [%s] are found", path));
#endif
	}

	do
	{
		char *CurName = fd.cFileName;

		if(StrCmp(CurName,"..") == 0 || StrCmp(CurName,".") == 0) continue;

		Log(("Found: [%s]%d", fd.cFileName, fd.dwFileAttributes));
		//Reset Reserved becouse it used by plugin but may cantain trash after API call
		fd.dwReserved0 = 0;
		fd.dwReserved1 = 0;
		//Reset plugin structure
		memset(&p,0,sizeof(PluginPanelItem));
		//Copy win32 data
		memmove(&p.FindData,&fd,sizeof(p.FindData));

		if(!il.Add(&p,1))
			return FALSE;
	}
	while(FindNextFile(h,&fd));

	FindClose(h);
	*PanelItem   = il.Items();
	*ItemsNumber = il.Count();
	return TRUE;
}

void FTP::FTP_FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber,BOOL FromPlugin)
{
	if(PanelItem && ItemsNumber)
	{
		if(FromPlugin)
			FreeFindData(PanelItem,ItemsNumber);
		else
			free(PanelItem);
	}
}

//---------------------------------------------------------------------------------
int FTP::ExpandListINT(PluginPanelItem *pi,int icn,FP_SizeItemList* il,BOOL FromPlugin,ExpandListCB cb,LPVOID Param)
{
	PluginPanelItem *DirPanelItem;
	int              DirItemsNumber,res;
	char            *CurName,*m;
	int              n,num;
	__int64          lSz=0,lCn=0;
	String           curp;

	if(!icn)
		return TRUE;

	if(CheckForEsc(FALSE,TRUE))
	{
		Log(("ESC: ExpandListINT cancelled"));
		SetLastError(ERROR_CANCELLED);
		return FALSE;
	}

	for(n = 0; n < icn; n++)
	{
		CurName = FTP_FILENAME(&pi[n]);

		if(StrCmp(CurName,"..") == 0 ||
		        StrCmp(CurName,".") == 0)
			continue;

//============
//FILE
		if(!IS_FLAG(pi[n].FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
		{
			m = PointToName(CurName);

			if(IncludeMask[0] && !FP_InPattern(IncludeMask,m))
			{
				Log(("INC File [%s] was filtered out by [%s]", CurName, IncludeMask));
				continue;
			}

			if(ExcludeMask[0] && FP_InPattern(ExcludeMask,m))
			{
				Log(("EXC File [%s] was filtered out by [%s]", CurName, ExcludeMask));
				continue;
			}

			if(cb && !cb(&pi[n],Param))
				return FALSE;

			if(il)
			{
				il->TotalFullSize += ((__int64)pi[n].FindData.nFileSizeHigh) << 32 | pi[n].FindData.nFileSizeLow;
				il->TotalFiles++;
				//Add
				PluginPanelItem *tmp = il->Add(&pi[n]);
				//Reset spesial plugin fields
				tmp->FindData.dwReserved0 = 0;
				tmp->FindData.dwReserved1 = 0;
			}

			continue;
		}

//============
//DIR
		//Get name
		m = strrchr(CurName, FromPlugin ? '/' : '\\');

		if(m) m = m+1;
		else m = CurName;

		//Remember current
		GetCurPath(curp);

		//Set dir
		if(!FTP_SetDirectory(m,FromPlugin))
			return FALSE;

		if(FromPlugin)
		{
			String newp;
			GetCurPath(newp);

			if(curp == newp)
				continue;
		}

		//Get subdir list
		if(hConnect && !cb)
		{
			String str;
			char   digit[ 50 ];

			if(il)
				str.printf("%sb in %u: %s",
				           FDigit(digit,il->TotalFullSize,-1),
				           (int)il->TotalFiles,
				           CurName);
			else
				str = CurName;

			FtpConnectMessage(hConnect, MScaning, str.c_str());
		}
		else if(hConnect)
			FtpConnectMessage(hConnect, MScaning, PointToName(CurName));

		if(il)
		{
			num = il->Count();
			il->Add(&pi[n]);
		}
		else
			num = -1;

		if(FTP_GetFindData(&DirPanelItem,&DirItemsNumber,FromPlugin))
		{
			FTP_FixPaths(CurName, DirPanelItem, DirItemsNumber, FromPlugin);

			if(num != -1)
			{
				lSz = il->TotalFullSize;
				lCn = il->TotalFiles;
			}

			res = ExpandListINT(DirPanelItem,DirItemsNumber,il,FromPlugin,cb,Param);

			if(num != -1 && res)
			{
				lSz = il->TotalFullSize - lSz;
				lCn = il->TotalFiles    - lCn;
				il->Item(num)->FindData.nFileSizeHigh = (DWORD)((lSz >> 32) & MAX_DWORD);
				il->Item(num)->FindData.nFileSizeLow  = (DWORD)(lSz & MAX_DWORD);
				il->Item(num)->FindData.dwReserved0   = (DWORD)lCn;
			}

			FTP_FreeFindData(DirPanelItem,DirItemsNumber,FromPlugin);
		}
		else
			return FALSE;

		if(!res) return FALSE;

		if(!FTP_SetDirectory("..", FromPlugin))
			return FALSE;

		if(cb && !cb(&pi[n],Param))
			return FALSE;
	}

	return TRUE;
}
//---------------------------------------------------------------------------------
void FTP::Invalidate(void)
{
	FP_Info->Control(this,FCTL_UPDATEPANEL,NULL);
	FP_Info->Control(this,FCTL_REDRAWPANEL,NULL);
}

//---------------------------------------------------------------------------------
BOOL FTP::Reread(void)
{
	PanelInfo pi;
	String    oldp, newp, cur;
	GetCurPath(oldp);
	//Save current file to restore
	FP_Info->Control(this, FCTL_GETPANELINFO, &pi);

	if(pi.ItemsNumber > 0 && pi.CurrentItem < pi.ItemsNumber)
		cur = FTP_FILENAME(&pi.PanelItems[pi.CurrentItem]);

	//Reread
	if(!ShowHosts)
		ResetCache = TRUE;

	FP_Info->Control(this, FCTL_UPDATEPANEL, (void*)1);
	//Redraw
	GetCurPath(newp);
	int rc = oldp == newp;

	if(rc)
	{
		SelectFile = cur;
		Log(("SetLastHost: [%s]", SelectFile.c_str()));
	}

	FP_Info->Control(this, FCTL_REDRAWPANEL, NULL);
	return rc;
}
//---------------------------------------------------------------------------------
void FTP::CopyNamesToClipboard(void)
{
	String     s, FullName, CopyData;
	PanelInfo  pi;
	int        CopySize, n;
	FP_Info->Control(this, FCTL_GETPANELINFO, &pi);
	FtpGetCurrentDirectory(hConnect, s);
	Host.MkUrl(FullName, s.c_str(), "");

	for(CopySize = n = 0; n < pi.SelectedItemsNumber; n++)
		CopySize += FullName.Length() +
		            1/* / */ +
		            static_cast<int>(strlen(FTP_FILENAME(&pi.SelectedItems[n]))) +
		            2/*quote*/ +
		            2/* \r\n */;

	if(!CopyData.Alloc(CopySize+2))
		return;

	for(n = 0; n < pi.SelectedItemsNumber; n++)
	{
		s = FullName;
		AddEndSlash(s, '/');
		s.Add(FTP_FILENAME(&pi.SelectedItems[n]));

		if(Opt.QuoteClipboardNames)
			QuoteStr(s);

		CopyData.Add(s);
		CopyData.Add("\r\n");
	}

	if(CopyData.Length())
		FP_CopyToClipboard(CopyData.c_str(), CopyData.Length());
}

//---------------------------------------------------------------------------------
void FTP::BackToHosts(void)
{
	int num = FP_GetRegKey("LastHostsMode",2);
	Log(("BackToHosts: [%s] h:[%s] md:%d sf: %s", Host.RegKey, Host.Host, num, Host.RegKey));
	SelectFile       = Host.RegKey;
	SwitchingToFTP   = FALSE;
	RereadRequired   = TRUE;
	ShowHosts        = TRUE;
	Host.HostName[0] = 0;
	delete hConnect;
	hConnect = NULL;
	FP_Info->Control(this,FCTL_SETVIEWMODE,&num);
}
