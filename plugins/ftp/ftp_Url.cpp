#include <all_far.h>
#pragma hdrstop

#include "ftp_Int.h"

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

		TStrCpy(p->FileName.cFileName, p->SrcPath.c_str()+num+1);
		p->SrcPath.SetLength(num);
	}

	return TRUE;
}

BOOL FTP::EditUrlItem(FTPUrl* p)
{
	static FP_DialogItem InitItems[]=
	{
		/*00*/    FDI_CONTROL(DI_DOUBLEBOX, 3, 1,72,12, 0, FMSG(MUrlItem))

		/*01*/      FDI_LABEL(5, 2,    FMSG(MCopyFrom))
		/*02*/   FDI_HISTEDIT(5, 3,70, "FTPUrl")
		/*03*/      FDI_LABEL(5, 4,    FMSG(MCopyTo))
		/*04*/   FDI_HISTEDIT(5, 5,70, "FTPUrl")
		/*05*/      FDI_LABEL(5, 6,    FMSG(MFileName))
		/*06*/   FDI_HISTEDIT(5, 7,70, "FTPFileName")

		/*07*/      FDI_HLINE(3, 8)

		/*08*/      FDI_CHECK(5, 9,    FMSG(MUDownlioad))

		/*09*/      FDI_HLINE(3,10)
		/*10*/    FDI_GBUTTON(0,11,    FMSG(MUHost))
		/*11*/ FDI_GDEFBUTTON(0,11,    FMSG(MOk))
		/*12*/    FDI_GBUTTON(0,11,    FMSG(MCancel))
		/*13*/    FDI_GBUTTON(0,11,    FMSG(MUError))
		{FFDI_NONE,0,0,0,0,0,NULL}
	};
	FarDialogItem  DialogItems[(sizeof(InitItems)/sizeof(InitItems[0])-1)];
//Create items
	FP_InitDialogItems(InitItems,DialogItems);
	PreFill(p);

	do
	{
//Set flags
		//From
		TStrCpy(DialogItems[2].Data, p->SrcPath.c_str());
		//To
		TStrCpy(DialogItems[4].Data, p->DestPath.c_str());
		//Name
		TStrCpy(DialogItems[6].Data, p->FileName.cFileName);
		//Flags
		DialogItems[ 8].Selected = p->Download;

		if(!p->Error.Length())
			SET_FLAG(DialogItems[13].Flags,DIF_DISABLE);

//Dialog
		do
		{
			int rc = FDialog(76,14,"FTPQueueItemEdit",DialogItems,(sizeof(InitItems)/sizeof(InitItems[0])-1));
			if(rc == -1 || rc == 12) return FALSE;                               else if(rc == 11)             break;                                      else if(rc == 10)             GetHost(MEditFtpTitle, &p->Host, FALSE);  else if(rc == 13)             ShowHostError(p);
		}
		while(1);

//Get paras
		//From
		p->SrcPath = DialogItems[2].Data;
		//To
		p->DestPath = DialogItems[4].Data;
		//Name
		TStrCpy(p->FileName.cFileName, DialogItems[6].Data);
		//Flags
		p->Download = DialogItems[ 8].Selected;

//Form
		if(!PreFill(p))
			continue;

		return TRUE;
	}
	while(1);
}
