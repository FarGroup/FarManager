#include <all_far.h>
#pragma hdrstop

#include "Int.h"

int FTP::GetHostFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,String& DestPath,int OpMode)
{
	PROC(("FTP::GetHostFiles","%d [%s] %s %08X",ItemsNumber,DestPath.c_str(),Move?"MOVE":"COPY",OpMode))
	InitDialogItem InitItems[]=
	{
		{DI_DOUBLEBOX, 3, 1,72,6, 0,0,0,0, NULL},
		{DI_TEXT,5, 2,0,0,0,0,0,0,   NULL},
		{DI_EDIT,5, 3,70, 3,0,0,0,0,NULL},
		{DI_TEXT,3, 4,3, 4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },
		{DI_BUTTON,0, 5,0,0,0,0,DIF_CENTERGROUP,  1, FMSG(MCopy)},
		{DI_BUTTON,0, 5,0,0,0,0,DIF_CENTERGROUP, 0,  FMSG(MCancel)},
	};
	FarDialogItem    DialogItems[ARRAYSIZE(InitItems)];
	FP_SizeItemList  il;

	if(!IS_SILENT(OpMode))
	{
		if(Move)
		{
			InitItems[0].Data = FMSG(MMoveHostTitle);
			InitItems[1].Data = FMSG(MMoveHostTo);
		}
		else
		{
			InitItems[0].Data = FMSG(MCopyHostTitle);
			InitItems[1].Data = FMSG(MCopyHostTo);
		}

		InitDialogItems(InitItems,DialogItems,ARRAYSIZE(DialogItems));
		StrCpy(DialogItems[2].Data, DestPath.c_str(), sizeof(DialogItems[2].Data));
		int AskCode = FDialog(76,8,"FTPCmd",DialogItems,ARRAYSIZE(DialogItems));

		if(AskCode != 4)
			return -1;

		DestPath = DialogItems[2].Data;
	}

	if(!DestPath.Length())
		return -1;

	if(!ExpandList(PanelItem,ItemsNumber,&il,TRUE))
		return 0;

	int      OverwriteAll = FALSE,
	         SkipAll      = FALSE,
	         Rename       = FALSE;
	int      n;
	FTPHost* p;
	FTPHost  h;
	char     CheckKey[ sizeof(HostsPath)+1 ];
	char     DestName[ sizeof(HostsPath)+1 ];

	if(DestPath.Cmp(".."))
	{
		if(*HostsPath==0)
			return 0;
		else
		{
			StrCpy(CheckKey,HostsPath,sizeof(CheckKey));
			char *m = strrchr(CheckKey,'\\');

			if(m) m[1] = 0;
			else CheckKey[0] = 0;

			Rename = TRUE;
		}
	}
	else
	{
		StrCpy(CheckKey, DestPath.c_str(), ARRAYSIZE(CheckKey));

		if(strpbrk(DestPath.c_str(),":\\")==NULL && !FP_CheckRegKey(CheckKey))
		{
			Rename=TRUE;
		}
		else if(FP_GetRegKey(CheckKey,"Folder",0))
		{
			AddEndSlash(CheckKey,'\\',ARRAYSIZE(CheckKey));
			Rename=TRUE;
		}
	}

	AddEndSlash(DestPath, '\\');

//Rename
	if(Rename)
	{
		for(n=0; n < il.Count(); n++)
		{
			p = FTPHost::Convert(&il.List[n]);

			if(!p) continue;

			//Check for folders
			if(p->Folder)
			{
				SayMsg(FMSG(MCanNotMoveFolder));
				return TRUE;
			}

			h.Assign(p);
			h.RegKey[0] = 0;

			if(!h.Write(CheckKey))
				return FALSE;

			if(Move && !p->Folder)
			{
				FP_DeleteRegKey(p->RegKey);

				if(n < ItemsNumber) PanelItem[n].Flags &= ~PPIF_SELECTED;
			}
		}
	}//Rename
	else

//INI
		for(n = 0; n < il.Count(); n++)
		{
			p = FTPHost::Convert(&il.List[n]);

			if(!p) continue;

			if(p->Folder)
			{
				continue;
			}

			p->MkINIFile(DestName, HostsPath, DestPath.c_str());
			DWORD DestAttr=GetFileAttributes(DestName);

			if(!IS_SILENT(OpMode) &&
			        !OverwriteAll &&
			        DestAttr != 0xFFFFFFFF)
			{
				if(SkipAll)
					continue;

				LPCSTR MsgItems[] =
				{
					FMSG(Move ? MMoveHostTitle:MCopyHostTitle),
					FMSG(IS_FLAG(DestAttr,FILE_ATTRIBUTE_READONLY) ? MAlreadyExistRO : MAlreadyExist),
					DestName,
					/*0*/FMSG(MOverwrite),
					/*1*/FMSG(MOverwriteAll),
					/*2*/FMSG(MCopySkip),
					/*3*/FMSG(MCopySkipAll),
					/*4*/FMSG(MCopyCancel)
				};
				int MsgCode = FMessage(FMSG_WARNING,NULL,MsgItems,ARRAYSIZE(MsgItems),5);

				switch(MsgCode)
				{
					case 1:
						OverwriteAll=TRUE;
						break;
					case 3:
						SkipAll=TRUE;
					case 2:
						continue;
					case -1:
					case 4:
						return(-1);
				}
			}

			int WriteFailed = FALSE;

			if(DestAttr!=0xFFFFFFFF)
			{
				if(!DeleteFile(DestName))
					if(!SetFileAttributes(DestName,FILE_ATTRIBUTE_NORMAL) && !DeleteFile(DestName))
						WriteFailed=TRUE;
			}

			if(!WriteFailed)
			{
				if(!p->WriteINI(DestName))
				{
					WriteFailed=TRUE;
					DeleteFile(DestName);
				}
				else if(Move)
					FP_DeleteRegKey(p->RegKey);
			}

			if(WriteFailed)
			{
				LPCSTR MsgItems[] =
				{
					FMSG(MError),
					FMSG(MCannotCopyHost),
					DestName,
					FMSG(MOk)
				};
				FMessage(FMSG_WARNING|FMSG_DOWN|FMSG_ERRORTYPE,NULL,MsgItems,ARRAYSIZE(MsgItems),1);
				return(0);
			}
		}//INI

	if(Move)
		for(n = il.Count()-1; n >= 0; n--)
		{
			if(CheckForEsc(FALSE))
				return -1;

			p = FTPHost::Convert(&il.List[n]);

			if(p && p->Folder)
			{
				FP_DeleteRegKey(p->RegKey);

				if(n < ItemsNumber)
					PanelItem[n].Flags &= ~PPIF_SELECTED;
			}
		}

	return 1;
}
