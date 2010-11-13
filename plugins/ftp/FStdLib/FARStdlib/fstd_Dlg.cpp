#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

int WINAPI FP_ShowMsg(LPCSTR Text, UINT Flags, LPCSTR Help)
{
	if(strchr(Text,'\n') || strchr(Text,'\r'))
		SET_FLAG(Flags,FMSG_ALLINONE);

	return FP_Info->Message(FP_Info->ModuleNumber, Flags, Help, (const char * const *)Text, 1, 1);
}

int WINAPI FP_ShowDialog(int w, int h,FarDialogItem* itms,int cn, LPCSTR Help)
{
	return FP_Info->Dialog(FP_Info->ModuleNumber, -1, -1, w, h, Help, itms, cn);
}

void WINAPI FP_InitDialogItem(const FP_DialogItem *Init,FarDialogItem *Item)
{
	CHK_INITED
	Assert(Init && Item);
	Item->Type          = Init->Type & FFDI_MASK;
	Item->X1            = Init->X1;
	Item->Y1            = Init->Y1;
	Item->X2            = Init->X2;
	Item->Y2            = Init->Y2;
	Item->Focus         = IS_FLAG(Init->Type,FFDI_FOCUSED);
	Item->Flags         = Init->Flags |
	                      (IS_FLAG(Init->Type,FFDI_GRAYED)?DIF_DISABLE:0) |
	                      (IS_FLAG(Init->Type,FFDI_HISTORY)?DIF_HISTORY:0);
	Item->DefaultButton = IS_FLAG(Init->Type,FFDI_DEFAULT);

	if(IS_FLAG(Init->Type,FFDI_MASKED))
	{
		Item->Data[0]  = 0;
		Item->Mask     = (char*)Init->Text;
	}
	else if(IS_FLAG(Init->Type,FFDI_HISTORY))
	{
		Item->Data[0]  = 0;
		Item->History  = (char*)Init->Text;
	}
	else
	{
		Item->Selected     = IS_FLAG(Init->Type,FFDI_SELECTED);
		StrCpy(Item->Data,FP_GetMsg(Init->Text),sizeof(Item->Data));
	}
}

void WINAPI FP_InitDialogItems(const FP_DialogItem *Init,FarDialogItem *Item)
{
	CHK_INITED

	for(int n = 0; Init[n].Type != FFDI_NONE; n++)
		FP_InitDialogItem(Init+n,Item+n);
}
