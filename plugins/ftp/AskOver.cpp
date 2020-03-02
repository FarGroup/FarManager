#include <all_far.h>
#pragma hdrstop

#include "Int.h"

void MkFileInfo(char *buff,int bsz,LPCSTR title,FAR_FIND_DATA* p)
{
	char       str[ 100 ];
	SYSTEMTIME tm;
	//Text
	_snprintf(buff,bsz,"%-18s",FP_GetMsg(title));

	if(p)
	{
		//Size
		FDigit(str,((__int64)p->nFileSizeHigh) << 32 | p->nFileSizeLow,25);
		StrCat(buff,str,bsz);
		//Time
		FileTimeToSystemTime(&p->ftLastWriteTime,&tm);
		_snprintf(str,ARRAYSIZE(str)," %02d.%02d.%04d %02d:%02d:%02d",
		          tm.wDay, tm.wMonth, tm.wYear, tm.wHour, tm.wMinute, tm.wSecond);
		StrCat(buff,str,bsz);
	}
}

overCode FTP::AskOverwrite(int title, BOOL Download,FAR_FIND_DATA* dest,FAR_FIND_DATA* src,overCode last,bool haveTimes)
{
	if(!hConnect)
		return ocCancel;

	InitDialogItem InitItems[]=
	{
		{DI_DOUBLEBOX, 3, 1,68,11, 0,0,0,0, NULL},

		{DI_TEXT,5, 2,0,0,0,  0,0,0,  FMSG(MAlreadyExist)},
		{DI_TEXT,5, 3,0,0,0,  0,0,0,  NULL},

		{DI_TEXT,3, 4,3, 4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },

		{DI_TEXT,4, 5,0,0,0, 0,0,0,   NULL},
		{DI_TEXT,4, 6,0,0,0, 0,0,0,   NULL},

		{DI_TEXT,3, 7,3, 7,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },

		{DI_CHECKBOX,5, 8,0,0,0,  0,0,0,  FMSG(MBtnRemember)},

		{DI_TEXT,3, 9,3, 9,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },

		{DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP, 1,   FMSG(MBtnOverwrite)},
		{DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP,  0,  FMSG(MBtnCopySkip)},
		{DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP, 0,   FMSG(MBtnCopyResume)},
		{DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP, 0,   FMSG(MBtnCopyNewer)},
		{DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP, 0,   FMSG(MBtnCopyCancel)},
	};
	FarDialogItem DialogItems[ARRAYSIZE(InitItems)];

	if(last == ocOverAll || last == ocSkipAll || last == ocNewerAll)
		return last;

	if(hConnect->ResumeSupport && last == ocResumeAll)
		return last;

//Set values
	//Title
	InitItems[ 0].Data = FMSG(title);
//Create items
	InitDialogItems(InitItems,DialogItems,ARRAYSIZE(DialogItems));
//Set flags
	//File name
	StrCpy(DialogItems[2].Data, dest->cFileName, ARRAYSIZE(DialogItems[0].Data));
	DialogItems[2].Data[60] = 0;

	//Gray resume
	if(Download && !hConnect->ResumeSupport)
	{
		SET_FLAG(DialogItems[11].Flags,DIF_DISABLE);
	}

	if(!haveTimes)
	{
		SET_FLAG(DialogItems[12].Flags,DIF_DISABLE);
	}

	//Info
	MkFileInfo(DialogItems[4].Data, ARRAYSIZE(DialogItems[0].Data), FMSG(MBtnCopyNew),      src);
	MkFileInfo(DialogItems[5].Data, ARRAYSIZE(DialogItems[0].Data), FMSG(MBtnCopyExisting), dest);
//Dialog
	int rc = FDialogEx(72,13,NULL,DialogItems,ARRAYSIZE(DialogItems),FDLG_WARNING,NULL);
	int remember = DialogItems[7].Selected;

	if(LongBeep)
		FP_PeriodReset(LongBeep);

	switch(rc)
	{
		case 9:
			return (remember?ocOverAll:ocOver);
		case 10:
			return (remember?ocSkipAll:ocSkip);
		case 11:
			return (remember?ocResumeAll:ocResume);
		case 12:
			return (remember?ocNewerAll:ocNewer);
		default:
			return ocCancel;
	}
}
