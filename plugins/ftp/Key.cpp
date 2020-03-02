#include <all_far.h>
#pragma hdrstop

#include "Int.h"

#ifdef _DEBUG
void ProcError(int v = 0)
{
	int sel;
	//Assert( 0 );
	//ProcError();
	void (*p)(int) = 0;
	p(v);
	sel = 0;
	sel /= sel;
}
#endif

BOOL InsertHostsCmd(HANDLE h,BOOL full)
{
	PanelInfo pi;
	FTPHost*  p;
	LPCSTR  m;

	if(!FP_Info->Control(h,FCTL_GETPANELINFO,&pi) ||
	        !pi.ItemsNumber ||
	        pi.CurrentItem < 0 || pi.CurrentItem >= pi.ItemsNumber ||
	        (p=FTPHost::Convert(&pi.PanelItems[pi.CurrentItem])) == NULL)
		return FALSE;

	if(full)
		m = p->HostName;
	else
		m = p->Host;

	if(StrCmp(m,"ftp://",6,FALSE) != 0 &&
	        StrCmp(m,"http://",7,FALSE) != 0)
		FP_Info->Control(h,FCTL_INSERTCMDLINE, (void*)(p->Folder ? "" : "ftp://"));

	return FP_Info->Control(h,FCTL_INSERTCMDLINE,(HANDLE)m);
}

int FTP::ProcessKey(int Key,unsigned int ControlState)
{
	PROC(("FTP::ProcessKey", "k:%08X(%c), sh:%08X",Key,__isascii(Key)&&isprint(Key)?((char)Key):' ',ControlState))
	PanelRedrawInfo  ri;
	PanelInfo        pi,otherPI;
	FTPHost          h,*p;

//Skip arrows
	if(ControlState == 0)
		switch(Key)
		{
			case   VK_END:
			case  VK_HOME:
			case    VK_UP:
			case  VK_DOWN:  //Get info

				if(!FP_Info->Control(this,FCTL_GETPANELSHORTINFO,&pi) &&
				        !FP_Info->Control(this,FCTL_GETPANELINFO,&pi))
				{
					Log(("!GetPanelInfo"));
					return FALSE;
				}

				//Skip self processing for QView work correctly
				if(!FP_Info->Control(this, FCTL_GETANOTHERPANELSHORTINFO, &otherPI) ||
				        (otherPI.PanelType == PTYPE_QVIEWPANEL || otherPI.PanelType == PTYPE_TREEPANEL))
					return FALSE;

				//Get current
				ri.TopPanelItem = pi.TopPanelItem;

				switch(Key)
				{
					case   VK_END:
						ri.CurrentItem = pi.ItemsNumber-1;
						break;
					case  VK_HOME:
						ri.CurrentItem = 0;
						break;
					case    VK_UP:
						ri.CurrentItem = pi.CurrentItem - 1;
						break;
					case  VK_DOWN:
						ri.CurrentItem = pi.CurrentItem + 1;
						break;
				}

				//Move cursor
				if(ri.CurrentItem == pi.CurrentItem || ri.CurrentItem < 0 || ri.CurrentItem >= pi.ItemsNumber)
					return TRUE;
				else
					return FP_Info->Control(this,FCTL_REDRAWPANEL,&ri);
		}

//Check for empty command line
	if(!ShowHosts && ControlState==PKF_CONTROL && Key==VK_INSERT)
	{
		char str[ 1024 ];
		FP_Info->Control(this, FCTL_GETCMDLINE, str);

		if(!str[0])
			ControlState = PKF_CONTROL|PKF_SHIFT;
	}

// --------------------------------------------------------------------------
//Ctrl+BkSlash
	if(Key == 0xDC && ControlState == 1)
	{
		Log(("CDROOT"));

		if(ShowHosts)
			SetDirectory("\\", 0);
		else
		{
			FP_Screen _scr;
			SetDirectory("/", 0);
		}

		Reread();
		Invalidate();
		return TRUE;
	}

// --------------------------------------------------------------------------
//Drop full url
	if(ShowHosts && ControlState == PKF_CONTROL)
	{
		if(Key == 'F')
			return InsertHostsCmd(this,TRUE);
		else if(Key == VK_RETURN)
			return InsertHostsCmd(this,FALSE);
	}

// --------------------------------------------------------------------------
//Utils menu
	if(ControlState == PKF_SHIFT && Key == VK_F1)
	{
		static LPCSTR strings[] =
		{
			NULL,
			"",
			FMSG(MHostParams),
			FMSG(MCloseConnection),
			FMSG(MUtilsDir),
			FMSG(MUtilsCmd),
			"",
			FMSG(MUtilsLog),
			NULL, //Sites list
			FMSG(MShowQueue),
#ifdef _DEBUG
			"",
			FMSG("Generate DIVIDE BY ZERO &Bug (Plugin traps!)"),
#endif
			NULL
		};
#ifdef _DEBUG
		strings[ 0] = Message("%s \"" __DATE__ "\" " __TIME__ " (DEBUG) ", FP_GetMsg(MVersionTitle));
#else
		strings[ 0] = Message("%s \"" __DATE__ "\" " __TIME__ " ", FP_GetMsg(MVersionTitle));
#endif
		strings[ 8] = isBackup() ? FMSG(MRemoveFromites) : FMSG(MAddToSites);
		//
		FP_MenuEx mnu(strings);
		int       prev = 0,
		          sel = (ShowHosts || !hConnect) ? 7 : 2,
		          file;
		mnu.Item(0)->isGrayed(TRUE);

		do
		{
			if(ShowHosts || !hConnect)
			{
				mnu.Item(2)->isGrayed(TRUE);
				mnu.Item(3)->isGrayed(TRUE);
				mnu.Item(4)->isGrayed(TRUE);
				mnu.Item(5)->isGrayed(TRUE);
			}

			if(!Opt.UseBackups)
				mnu.Item(8)->isGrayed(TRUE);

			mnu.Item(sel)->isSelected(TRUE);
			sel = mnu.Execute(FMSG(MUtilsCaption),FMENU_WRAPMODE,NULL,"FTPUtils");
			mnu.Item(prev)->isSelected(FALSE);
			prev                     = sel;

			switch(sel)
			{
				case -1:
					return TRUE;
//Version
				case  0:
					break;
//Host parameters
				case  2:

					if(ShowHosts || !hConnect) break;

					do
					{
						FTPHost tmp = Host;

						if(!GetHost(MEditFtpTitle, &tmp, FALSE)) break;

						//Reconnect
						if(!Host.CmpConnected(&tmp))
						{
							Host = tmp;

							if(!FullConnect())
							{
								BackToHosts();
								Invalidate();
							}

							break;
						}

						Host = tmp;
						//Change connection paras
						hConnect->InitData(&Host,-1);
						hConnect->InitIOBuff();
						Invalidate();
					}
					while(0);

					return TRUE;
//Switch to hosts
				case  3:

					if(ShowHosts || !hConnect) break;

					BackToHosts();
					Invalidate();
					return TRUE;
//Dir listing
				case  4:

					if(ShowHosts || !hConnect)
						break;

					{
						char str[MAX_PATH];  //Must be static buff because of MkTemp
						FP_FSF->MkTemp(str,"FTP");
						CreateDirectory(str,NULL);
						AddEndSlash(str,'\\',ARRAYSIZE(str));
						StrCat(str, "FTPDir.txt", ARRAYSIZE(str));
						file = _creat(str,_S_IREAD|_S_IWRITE);

						if(file == -1)
						{
							hConnect->ConnectMessage(MErrorTempFile,str,-MOk);
							return TRUE;
						}
						write(file,hConnect->Output,hConnect->OutputSize);
						close(file);
						FP_Info->Viewer(str,Message("%s: %s {%s}",FP_GetMsg(MDirTitle),PanelTitle,str),
						                0,0,-1,-1,VF_NONMODAL|VF_DELETEONCLOSE);
					}
					return TRUE;
//Show CMD
				case  5:

					if(ShowHosts || !hConnect) break;

					file = hConnect->Host.ExtCmdView;
					hConnect->Host.ExtCmdView = TRUE;
					SetLastError(ERROR_SUCCESS);
					hConnect->ConnectMessage(MNone__,NULL,MOk);
					hConnect->Host.ExtCmdView = file;
					return TRUE;
					// ------------------------------------------------------------
//Show LOG
				case  7:

					if(IsCmdLogFile())
						FP_Info->Viewer(GetCmdLogFile(),FP_GetMsg(MLogTitle),0,0,-1,-1,VF_NONMODAL|VF_ENABLE_F6);

					return TRUE;
//Add\Remove sites list
				case  8:

					if(isBackup())
						DeleteFromBackup();
					else
						AddToBackup();

					return TRUE;
//FTP queque
				case  9:
					QuequeMenu();
					return TRUE;
#ifdef _DEBUG
					// ------------------------------------------------------------
				case 11:
					ProcError();
					break;
#endif
			}
		}
		while(true);
	}

// --------------------------------------------------------------------------
//Copy names
	if(!ShowHosts && ControlState==(PKF_CONTROL|PKF_SHIFT) && Key==VK_INSERT)
	{
		CopyNamesToClipboard();
		return TRUE;
	}

// --------------------------------------------------------------------------
//Table
	if(!ShowHosts && ControlState==PKF_SHIFT && Key==VK_F7)
	{
		SelectTable();
		Invalidate();
		return TRUE;
	}

// --------------------------------------------------------------------------
//Attributes
	if(hConnect && !ShowHosts && ControlState == PKF_CONTROL && Key=='A')
	{
		SetAttributes();
		Invalidate();
		return TRUE;
	}

// --------------------------------------------------------------------------
//Save URL
	if(hConnect && !ShowHosts && ControlState==PKF_ALT && Key==VK_F6)
	{
		SaveURL();
		return TRUE;
	}

// --------------------------------------------------------------------------
//Reread
	if(ControlState==PKF_CONTROL && Key=='R')
	{
		ResetCache=TRUE;
		return FALSE;
	}

// --------------------------------------------------------------------------
//Drop full name
	if(!ShowHosts && hConnect && ControlState==PKF_CONTROL && Key=='F')
	{
		FP_Info->Control(this,FCTL_GETPANELINFO,&pi);

		if(pi.CurrentItem >= pi.ItemsNumber)
			return FALSE;

		String s, path;
		FtpGetCurrentDirectory(hConnect,path);
		Host.MkUrl(s, path.c_str(), FTP_FILENAME(&pi.PanelItems[pi.CurrentItem]));
		QuoteStr(s);
		FP_Info->Control(this, FCTL_INSERTCMDLINE, s.c_str());
		return TRUE;
	}

// --------------------------------------------------------------------------
//Select
	if(ControlState==0 && Key==VK_RETURN)
	{
		PluginPanelItem *cur;
		FP_Info->Control(this,FCTL_GETPANELINFO,&pi);

		if(pi.CurrentItem >= pi.ItemsNumber)
			return FALSE;

		cur = &pi.PanelItems[pi.CurrentItem];

		//Switch to FTP
		if(ShowHosts &&
		        !IS_FLAG(cur->FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
		{
			p = FTPHost::Convert(cur);

			if(!p)
				return FALSE;

			Log(("SelectFileItem [%s] [%s]", p->RegKey, p->HostName));
			Host.Assign(p);
			FullConnect();
			return TRUE;
		}/*FTP SWITCH*/

		//Change directory
		if(!ShowHosts && hConnect)
		{
			if(StrCmp(FTP_FILENAME(cur), ".") != 0 &&
			        StrCmp(FTP_FILENAME(cur), "..") != 0)
				if(IS_FLAG(cur->FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY) ||
				        IS_FLAG(cur->FindData.dwFileAttributes,FILE_ATTRIBUTE_REPARSE_POINT))
				{
					if(SetDirectoryFAR(IS_FLAG(cur->FindData.dwFileAttributes,FILE_ATTRIBUTE_REPARSE_POINT)
					                   ? cur->CustomColumnData[FTP_COL_LINK]
					                   : FTP_FILENAME(cur),
					                   0))
					{
						FP_Info->Control(this,FCTL_UPDATEPANEL,NULL);
						struct PanelRedrawInfo RInfo;
						RInfo.CurrentItem = RInfo.TopPanelItem = 0;
						FP_Info->Control(this,FCTL_REDRAWPANEL,&RInfo);
						return TRUE;
					}
					else
						return TRUE;
				}
		}/*CHANGE DIR*/
	}/*ENTER*/

// --------------------------------------------------------------------------
//New entry
	if(ShowHosts &&
	        ((ControlState==PKF_SHIFT && Key==VK_F4) ||
	         (ControlState==PKF_ALT   && Key==VK_F6)))
	{
		Log(("New host"));
		h.Init();

		if(GetHost(MEnterFtpTitle,&h,FALSE))
		{
			Log(("bw: [%s] [%s] [%s], sf: %s",h.Host,h.User,h.Password,h.RegKey));
			h.Write(HostsPath);
			SelectFile = h.RegKey;
			Invalidate();
		}

		return TRUE;
	}

// --------------------------------------------------------------------------
//Edit
	if(ShowHosts &&
	        ((ControlState==0           && Key==VK_F4) ||
	         (ControlState==PKF_SHIFT   && Key==VK_F6) ||
	         (ControlState==PKF_CONTROL && Key=='Z')))
	{
		FP_Info->Control(this,FCTL_GETPANELINFO,&pi);

		if(pi.CurrentItem >= pi.ItemsNumber)
			return TRUE;

		p = FTPHost::Convert(&pi.PanelItems[pi.CurrentItem]);

		if(!p)
			return TRUE;

		Log(("EditHost: [%s] [%s]", p->RegKey, p->Host));

		if(p->Folder)
		{
			String s(p->Host);

			if(!EditDirectory(s,p->HostDescr,FALSE))
				return TRUE;

			StrCpy(p->Host, s.c_str(), ARRAYSIZE(p->Host));
		}
		else
		{
			if(!GetHost(MEditFtpTitle,p,Key=='Z'))
				return TRUE;
		}

		if(p->Write(HostsPath))
		{
			SelectFile = p->RegKey;
			Log(("SetLastHost: [%s]", SelectFile.c_str()));
			FP_Info->Control(this,FCTL_UPDATEPANEL,NULL);
			FP_Info->Control(this,FCTL_REDRAWPANEL,NULL);
		}

		return TRUE;
	}/*Edit*/

// --------------------------------------------------------------------------
//Copy/Move
	if(!ShowHosts &&
	        (ControlState == 0 || ControlState == PKF_SHIFT) &&
	        Key == VK_F6)
	{
		FTP *ftp = OtherPlugin(this);
		int  rc;

		if(!ftp && ControlState == 0 && Key == VK_F6)
			return FALSE;

		FP_Info->Control(this, FCTL_GETPANELINFO,        &pi);
		FP_Info->Control(this, FCTL_GETANOTHERPANELINFO, &otherPI);

		if(pi.SelectedItemsNumber > 0 &&
		        pi.CurrentItem >= 0 && pi.CurrentItem < pi.ItemsNumber)
		{
			do
			{
				String s;

				if(ControlState == PKF_SHIFT)
				{
					s = FTP_FILENAME(&pi.PanelItems[pi.CurrentItem]);
					rc = GetFiles(&pi.PanelItems[pi.CurrentItem], 1, Key == VK_F6, s, 0);
				}
				else
				{
					s = otherPI.CurDir;
					rc = GetFiles(pi.SelectedItems, pi.SelectedItemsNumber, Key == VK_F6, s, 0);
				}

				if(rc == TRUE || rc == -1)
				{
					FP_Screen::FullRestore();
					FP_Info->Control(this,FCTL_UPDATEPANEL,NULL);
					FP_Info->Control(this,FCTL_REDRAWPANEL,NULL);
					FP_Screen::FullRestore();
					FP_Info->Control(this,FCTL_UPDATEANOTHERPANEL,NULL);
					FP_Info->Control(this,FCTL_REDRAWANOTHERPANEL,NULL);
					FP_Screen::FullRestore();
				}

				return TRUE;
			}
			while(0);
		}
	}

	return FALSE;
}
