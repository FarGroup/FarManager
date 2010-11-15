#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

//--------------------------------------------------------------------------------
//-- Color dialog
//--------------------------------------------------------------------------------
#define CLBAR( x,y,cl,fl ) FDI_CONTROL( DI_RADIOBUTTON, x+(cl/4)*3, y + cl%4, 0, 0,fl|DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~cl)&0x7,cl)),  NULL )
#define CLGROUP( x,y )     CLBAR( x,y, 0,DIF_GROUP ) CLBAR( x,y, 1,0 ) CLBAR( x,y, 2,0 ) CLBAR( x,y, 3,0 ) \
	CLBAR( x,y, 4,0 )         CLBAR( x,y, 5,0 ) CLBAR( x,y, 6,0 ) CLBAR( x,y, 7,0 ) \
	CLBAR( x,y, 8,0 )         CLBAR( x,y, 9,0 ) CLBAR( x,y,10,0 ) CLBAR( x,y,11,0 ) \
	CLBAR( x,y,12,0 )         CLBAR( x,y,13,0 ) CLBAR( x,y,14,0 ) CLBAR( x,y,15,0 )
#define cdlgFORE 2
#define cdlgBK   19
#define cdlgTEXT 35
#define cdlgOK   39

static FP_DialogItem ColorDialog[]=
{
	/*00*/          FDI_DBORDER(3, 1,35,13, NULL)
	/*01*/             FDI_SBOX(5, 2,18, 7, NULL)
	/*02*/              CLGROUP(6, 3)
	/*18*/             FDI_SBOX(20, 2,33, 7, NULL)
	/*19*/              CLGROUP(21, 3)
	/*35*/       FDI_COLORLABEL(5, 8, FAR_COLOR(fccYELLOW,fccGREEN), NULL)
	/*36*/       FDI_COLORLABEL(5, 9, FAR_COLOR(fccYELLOW,fccGREEN), NULL)
	/*37*/       FDI_COLORLABEL(5,10, FAR_COLOR(fccYELLOW,fccGREEN), NULL)
	/*38*/            FDI_HLINE(3,11)
	/*39*/       FDI_GDEFBUTTON(0,12, NULL)
	/*40*/          FDI_GBUTTON(0,12, NULL)
	{FFDI_NONE,0,0,0,0,0,NULL}
};

static int  ColorFore;
static int  ColorBk;
static char Title[FAR_MAX_CAPTION];

static LONG_PTR WINAPI CDLG_WndProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
	FarDialogItemData id;
	char str[FAR_MAX_CAPTION+50];

	switch(Msg)
	{

		case        DN_BTNCLICK: if(Param1 >= cdlgFORE && Param1 < cdlgFORE+16)
				ColorFore = Param1 - cdlgFORE;
			else if(Param1 >= cdlgBK && Param1 < cdlgBK+16)
				ColorBk = Param1 - cdlgBK;

			sprintf(str,"%s(%3d 0x%02X %03o)",
			        Title,
			        FAR_COLOR(ColorFore,ColorBk),
			        FAR_COLOR(ColorFore,ColorBk),
			        FAR_COLOR(ColorFore,ColorBk));
			//set caption
			id.PtrLength = strLen(str);
			id.PtrData   = str;
			FP_Info->SendDlgMessage(hDlg,DM_SETTEXT,0,(LONG_PTR)(&id));
			//Invalidate
			FP_Info->SendDlgMessage(hDlg,DM_SETREDRAW,0,0);
			break;

		case DN_CTLCOLORDLGITEM: if(Param1 >= cdlgTEXT && Param1 < cdlgTEXT+3)
				return FAR_COLOR(ColorFore,ColorBk);

			break;
	}

	return FP_Info->DefDlgProc(hDlg,Msg,Param1,Param2);
}

int WINAPI FP_GetColorDialog(int color,FLngColorDialog* p,LPCSTR Help)
{
	FarDialogItem DialogItems[(sizeof(ColorDialog)/sizeof(ColorDialog[0])-1)];
	static FLngColorDialog base =
	{
		FMSG("Color"),
		FMSG("&Foreground"),
		FMSG("&Background"),
		FMSG("&Set"),
		FMSG("&Cancel"),
		FMSG("Text Text Text Text Text Text")
	};

	if(!p) p = &base;

	char str[FAR_MAX_CAPTION+50];
	int  n;
	FP_InitDialogItems(ColorDialog,DialogItems);
	StrCpy(DialogItems[ 0].Data,         FP_GetMsg(p->MTitle?p->MTitle:base.MTitle),    FAR_MAX_CAPTION);
	StrCpy(DialogItems[ 1].Data,         FP_GetMsg(p->MFore?p->MFore:base.MFore),       FAR_MAX_CAPTION);
	StrCpy(DialogItems[18].Data,         FP_GetMsg(p->MBk?p->MBk:base.MBk),             FAR_MAX_CAPTION);
	StrCpy(DialogItems[39].Data,         FP_GetMsg(p->MSet?p->MSet:base.MSet),          FAR_MAX_CAPTION);
	StrCpy(DialogItems[40].Data,         FP_GetMsg(p->MCancel?p->MCancel:base.MCancel), FAR_MAX_CAPTION);
	StrCpy(DialogItems[cdlgTEXT].Data,   FP_GetMsg(p->MText?p->MText:base.MText),       FAR_MAX_CAPTION);
	StrCpy(DialogItems[cdlgTEXT+1].Data, DialogItems[cdlgTEXT].Data);
	StrCpy(DialogItems[cdlgTEXT+2].Data, DialogItems[cdlgTEXT].Data);
	ColorFore = FAR_COLOR_FORE(color);
	ColorBk   = FAR_COLOR_BK(color);

	for(n = 0; n < 16; n++)
	{
		DialogItems[cdlgFORE+n].Selected = FALSE;
		DialogItems[cdlgBK+n].Selected   = FALSE;
		DialogItems[cdlgFORE+n].Focus    = FALSE;
		DialogItems[cdlgBK+n].Focus      = FALSE;
	}

	DialogItems[ cdlgFORE+ColorFore ].Selected = TRUE; DialogItems[ cdlgFORE+ColorFore ].Focus = TRUE;
	DialogItems[ cdlgBK+ColorBk ].Selected     = TRUE;
	StrCpy(Title,DialogItems[0].Data,sizeof(Title));
	sprintf(str,"(%3d 0x%02X %03o)",
	        FAR_COLOR(ColorFore,ColorBk),
	        FAR_COLOR(ColorFore,ColorBk),
	        FAR_COLOR(ColorFore,ColorBk));
	StrCat(DialogItems[0].Data,str,512);
	n = FP_Info->DialogEx(FP_Info->ModuleNumber,-1,-1,39,15,Help,
	                      DialogItems,(sizeof(ColorDialog)/sizeof(ColorDialog[0])-1),0,0,
	                      CDLG_WndProc,0);

	if(n == cdlgOK)
		color = FAR_COLOR(ColorFore,ColorBk);

	StrCpy(DialogItems[0].Data,Title,FAR_MAX_CAPTION);
	return color;
}
