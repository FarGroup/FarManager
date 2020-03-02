#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

//------------------------------------------------------------------------
FP_Dialog::FP_Dialog(HANDLE h)
{
	Handle = h;
	LockCount = 0;
}

void FP_Dialog::Lock(bool v)
{
	if(v == (LockCount!=0))
		return;
	if(v) LockCount++; else LockCount--;

	FP_Info->SendDlgMessage(Handle,DM_ENABLEREDRAW,v ? FALSE : TRUE,0);
}

void FP_Dialog::FullUnlock(void)
{
	if(!LockCount) return;

	while(LockCount)
		Unlock();
}

bool FP_Dialog::CursorPos(int num,int pos) const
{
	COORD cp;
	cp.X = pos;
	cp.Y = 0;

	if(!FP_Info->SendDlgMessage(Handle,DM_SETCURSORPOS,num,(LONG_PTR)&cp))
		return false;

	return true;
}

int FP_Dialog::SetText(int num, LPCSTR str,int sz) const
{
	FarDialogItemData dd = { sz,(char*)str };
	return (int)FP_Info->SendDlgMessage(Handle,DM_SETTEXT,num,(LONG_PTR)&dd);
}

LPCSTR FP_Dialog::GetText(int num) const
{
	static char buff[ FAR_MAX_DLGITEM ];
	FarDialogItemData dd = { sizeof(buff)-1,buff };
	buff[ FP_Info->SendDlgMessage(Handle,DM_GETTEXT,num,(LONG_PTR)&dd)] = 0;
	return buff;
}

int FP_Dialog::GetText(int num,char *buff,int bSz) const
{
	FarDialogItemData dd = { bSz,buff };
	int rc;
	rc = (int)FP_Info->SendDlgMessage(Handle,DM_GETTEXT,num,(LONG_PTR)&dd);

	if(rc < bSz) buff[rc] = 0;

	return rc;
}
