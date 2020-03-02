#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

//---------------------------------------------------------------------------------
void FMenuItem::Assign(LPCSTR txt,bool sel,char ch)
{
	if(txt)
	{
		txt = FP_GetMsg(txt);
		StrCpy(Text, txt, sizeof(Text));
	}

	Separator = txt == NULL || txt[0] == 0;
	Selected  = sel;
	Checked   = ch;
}
void FMenuItem::Assign(const FMenuItem& p)
{
	memcpy(this,&p,sizeof(*this));
}

void FMenuItem::Assign(void)
{
	memset(this, 0, sizeof(*this));
	Separator = 1;
}

//---------------------------------------------------------------------------------
void FMenuItemEx::Assign(LPCSTR txt,bool sel,char ch)
{
	memset(this, 0, sizeof(*this));

	if(txt)
	{
		txt = FP_GetMsg(txt);
		StrCpy(Text.Text, txt, sizeof(Text.Text));
	}

	if(txt == NULL || txt[0] == 0) SET_FLAG(Flags, MIF_SEPARATOR);

	if(sel)                        SET_FLAG(Flags, MIF_SELECTED);

	if(ch)                         SET_FLAG(Flags, MIF_CHECKED);
}

void FMenuItemEx::Assign(const FMenuItemEx& p)
{
	memcpy(this, &p, sizeof(*this));
}

void FMenuItemEx::Assign(void)
{
	memset(this, 0, sizeof(*this));
	SET_FLAG(Flags, MIF_SEPARATOR);
}
