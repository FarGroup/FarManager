#include <all_far.h>
#pragma hdrstop

#include "Int.h"

//------------------------------------------------------------------------
void ShowHostError(FTPUrl* p)
{
	SayMsg(p->Error.c_str());
}

//------------------------------------------------------------------------
void FTP::UrlInit(FTPUrl* p)
{
	memset(&p->FileName, 0, sizeof(p->FileName));
	p->Host     = Host;
	p->Download = FALSE;
	p->Next     = NULL;
}

FTPUrl* FTP::UrlItem(int num, FTPUrl* *prev)
{
	FTPUrl* p, *p1;

	if(prev) *prev = NULL;

	for(p1 = NULL,p = UrlsList;
	        p && num > 0;
	        p1 = p, p = p->Next)
		num--;

	if(num == 0)
	{
		if(prev) *prev = p1;

		return p;
	}

	return NULL;
}

void FTP::DeleteUrlItem(FTPUrl* p, FTPUrl* prev)
{
	if(!p) return;

	if(prev) prev->Next = p->Next;

	if(UrlsList == p) UrlsList = p->Next;

	if(UrlsTail == p) UrlsTail = prev;

	delete p;
	QuequeSize--;
}

//------------------------------------------------------------------------
BOOL PreFill(FTPUrl* p)
{
	char  ch;

	if(p->SrcPath.Cmp("http://", 6, FALSE) ||
	        p->SrcPath.Cmp("ftp://",  5, FALSE))
		p->Download = TRUE;

	if(p->Download && p->SrcPath[0] != '/')
	{
		p->Host.SetHostName(p->SrcPath.c_str(), NULL, NULL);
		p->SrcPath = p->Host.Home;
		char *m = strstr(p->Host.HostName, p->Host.Home);

		if(m) *m = 0;

		p->Host.Home[0] = 0;
	}

	if(!p->Host.Host[0])
		return FALSE;

	if(p->Download)
	{
		ch = '/';
	}
	else
	{
		FixLocalSlash(p->SrcPath);
		ch = '\\';
	}

	if(strpbrk(p->FileName.cFileName, "\\/") != NULL)
	{
		AddEndSlash(p->SrcPath, ch);
		p->SrcPath.cat(p->FileName.cFileName);
		p->FileName.cFileName[0] = 0;

		if(!p->Download)
			FixLocalSlash(p->SrcPath);
	}

	if(!p->FileName.cFileName[0])
	{
		int num = p->SrcPath.RChr(ch);

		if(num <= 0)
			return FALSE;

		StrCpy(p->FileName.cFileName, p->SrcPath.c_str()+num+1, ARRAYSIZE(p->FileName.cFileName));
		p->SrcPath.SetLength(num);
	}

	return TRUE;
}

BOOL FTP::EditUrlItem(FTPUrl* p)
{
	InitDialogItem InitItems[]=
	{
		{DI_DOUBLEBOX, 3, 1,72,12, 0,0,0,0, FMSG(MUrlItem)},

		{DI_TEXT,5, 2,0,0,0,   0,0,0, FMSG(MCopyFrom)},
		{DI_EDIT,5, 3,70, 3,0,(DWORD_PTR)"FTPUrl" ,DIF_HISTORY,0, NULL},
		{DI_TEXT,5, 4,0,0,0,  0,0,0,  FMSG(MCopyTo)},
		{DI_EDIT,5, 5,70, 5,0,(DWORD_PTR)"FTPUrl",DIF_HISTORY,0, NULL},
		{DI_TEXT,5, 6,0,0,0,  0,0,0,  FMSG(MFileName)},
		{DI_EDIT,5, 7,70, 7,0, (DWORD_PTR)"FTPFileName",DIF_HISTORY,0,NULL},

		{DI_TEXT,3, 8,3, 8,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },

		{DI_CHECKBOX,5, 9,0,0,0,  0,0,0,  FMSG(MUDownlioad)},

		{DI_TEXT,3,10,3,10,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },
		{DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP, 0,   FMSG(MUHost)},
		{DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP, 1,   FMSG(MOk)},
		{DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,  0,  FMSG(MCancel)},
		{DI_BUTTON,0,11,0,0,0,0,DIF_CENTERGROUP,  0,  FMSG(MUError)},
	};
	FarDialogItem DialogItems[ARRAYSIZE(InitItems)];
//Create items
	InitDialogItems(InitItems,DialogItems,ARRAYSIZE(DialogItems));
	PreFill(p);

	do
	{
//Set flags
		//From
		StrCpy(DialogItems[2].Data, p->SrcPath.c_str(), ARRAYSIZE(DialogItems[2].Data));
		//To
		StrCpy(DialogItems[4].Data, p->DestPath.c_str(), ARRAYSIZE(DialogItems[4].Data));
		//Name
		StrCpy(DialogItems[6].Data, p->FileName.cFileName, ARRAYSIZE(DialogItems[6].Data));
		//Flags
		DialogItems[ 8].Selected = p->Download;

		if(!p->Error.Length())
			SET_FLAG(DialogItems[13].Flags,DIF_DISABLE);

//Dialog
		do
		{
			int rc = FDialog(76,14,"FTPQueueItemEdit",DialogItems,ARRAYSIZE(DialogItems));

			if(rc == -1 || rc == 12) return FALSE;
			else if(rc == 11)             break;
			else if(rc == 10)             GetHost(MEditFtpTitle, &p->Host, FALSE);
			else if(rc == 13)             ShowHostError(p);
		}
		while(true);

//Get paras
		//From
		p->SrcPath = DialogItems[2].Data;
		//To
		p->DestPath = DialogItems[4].Data;
		//Name
		StrCpy(p->FileName.cFileName, DialogItems[6].Data, ARRAYSIZE(p->FileName.cFileName));
		//Flags
		p->Download = DialogItems[ 8].Selected;

//Form
		if(!PreFill(p))
			continue;

		return TRUE;
	}
	while(true);
}
