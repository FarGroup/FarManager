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
		_snprintf(str,sizeof(str)," %02d.%02d.%04d %02d:%02d:%02d",
		         tm.wDay, tm.wMonth, tm.wYear, tm.wHour, tm.wMinute, tm.wSecond);
		StrCat(buff,str,bsz);
	}
}

overCode FTP::AskOverwrite(int title, BOOL Download,FAR_FIND_DATA* dest,FAR_FIND_DATA* src,overCode last,bool haveTimes)
{
	if(!hConnect)
		return ocCancel;

	static FP_DialogItem InitItems[]=
	{
		/*00*/    FDI_CONTROL(DI_DOUBLEBOX, 3, 1,68,11, 0, NULL)

		/*01*/      FDI_LABEL(5, 2,    FMSG(MAlreadyExist))
		/*02*/      FDI_LABEL(5, 3,    NULL)

		/*03*/      FDI_HLINE(3, 4)

		/*04*/      FDI_LABEL(4, 5,    NULL)
		/*05*/      FDI_LABEL(4, 6,    NULL)

		/*06*/      FDI_HLINE(3, 7)

		/*07*/      FDI_CHECK(5, 8,    FMSG(MBtnRemember))

		/*08*/      FDI_HLINE(3, 9)

		/*09*/ FDI_GDEFBUTTON(0,10,    FMSG(MBtnOverwrite))
		/*10*/    FDI_GBUTTON(0,10,    FMSG(MBtnCopySkip))
		/*11*/    FDI_GBUTTON(0,10,    FMSG(MBtnCopyResume))
		/*12*/    FDI_GBUTTON(0,10,    FMSG(MBtnCopyNewer))
		/*13*/    FDI_GBUTTON(0,10,    FMSG(MBtnCopyCancel))
		{FFDI_NONE,0,0,0,0,0,NULL}
	};
	FarDialogItem  DialogItems[(sizeof(InitItems)/sizeof(InitItems[0])-1)];

	if(last == ocOverAll || last == ocSkipAll || last == ocNewerAll)
		return last;

	if(hConnect->ResumeSupport && last == ocResumeAll)
		return last;

//Set values
	//Title
	InitItems[ 0].Text = FMSG(title);
//Create items
	FP_InitDialogItems(InitItems,DialogItems);
//Set flags
	//File name
	StrCpy(DialogItems[2].Data, dest->cFileName, sizeof(DialogItems[0].Data));
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
	MkFileInfo(DialogItems[4].Data, sizeof(DialogItems[0].Data), FMSG(MBtnCopyNew),      src);
	MkFileInfo(DialogItems[5].Data, sizeof(DialogItems[0].Data), FMSG(MBtnCopyExisting), dest);
//Dialog
	int rc = FDialogEx(72,13,NULL,DialogItems,(sizeof(InitItems)/sizeof(InitItems[0])-1),FDLG_WARNING,NULL);
	int remember = DialogItems[7].Selected;

	if(LongBeep)
		FP_PeriodReset(LongBeep);

	switch(rc)
	{
		case 9: return (remember?ocOverAll:ocOver);
		case 10: return (remember?ocSkipAll:ocSkip);
		case 11: return (remember?ocResumeAll:ocResume);
		case 12: return (remember?ocNewerAll:ocNewer);
		default: return ocCancel;
	}
}
