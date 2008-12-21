/*
gettable.cpp

Работа с таблицами символов
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "plugin.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "vmenu.hpp"
#include "savefpos.hpp"
#include "keys.hpp"

static VMenu *tables;
static UINT nCurCP;

const wchar_t SelectedCodeTables[]=L"CodeTables\\Selected";

BOOL __stdcall EnumCodePagesProc (const wchar_t *lpwszCodePage)
{
	int Check=0;
	GetRegKey(SelectedCodeTables,lpwszCodePage,Check,0);
	if (Opt.CPMenuMode && !Check)
		return TRUE;

	UINT nCP = _wtoi(lpwszCodePage);
	CPINFOEXW cpi;
	if (GetCPInfoExW (nCP, 0, &cpi) && cpi.MaxCharSize == 1 )
	{
		MenuItemEx item;
		item.Clear ();
		if (nCP==nCurCP)
			item.Flags|=MIF_SELECTED;
		if (Check)
			item.Flags|=MIF_CHECKED;
		item.strName.Format(L"%5u%c %s",nCP,BoxSymbols[BS_V1],wcschr(cpi.CodePageName,L'(')+1);
		item.strName.SetLength(item.strName.GetLength()-1);
		tables->SetUserData((void*)(UINT_PTR)nCP, sizeof (UINT), tables->AddItem(&item));
	}
	return TRUE;
}

void AddUnicodeTables(VMenu &tables,DWORD dwCurrent, bool bShowUnicode, bool bShowUTF)
{
	struct CpInfo
	{
		UINT CP;
		const wchar_t *Name;
		int Type;
	}
	c[]=
	{
		{CP_UNICODE,L"UNICODE",0},
		{CP_REVERSEBOM,L"UNICODE (reverse BOM)",0},
		{CP_UTF7,L"UTF-7",1},
		{CP_UTF8,L"UTF-8",2},
	};

	MenuItemEx item;
	int Pos=0;

	for(size_t i=0;i<countof(c);i++)
	{
		if (c[i].Type == 0 && !bShowUnicode)
			continue;
		if (c[i].Type == 1) //пока что
			continue;
		if (c[i].Type == 2 && !bShowUTF)
			continue;
		item.Clear();
		item.strName.Format(L"%5u%c %s",c[i].CP,BoxSymbols[BS_V1],c[i].Name);
		item.UserData=(char*)c[i].CP;
		item.UserDataSize=sizeof(UINT);
		if(dwCurrent==c[i].CP)
			item.Flags|=MIF_SELECTED;
		tables.AddItem(&item,Pos++);
	}

	item.Clear();
	item.Flags=MIF_SEPARATOR;
	tables.AddItem(&item,Pos++);

	if(!IsUnicodeCP(dwCurrent))
		tables.SetSelectPos(tables.GetSelectPos()+Pos,0);
}

UINT GetTableEx (UINT nCurrent, bool bShowUnicode, bool bShowUTF)
{
	UINT nCP = (UINT)-1;

	tables = new VMenu (MSG(MGetTableTitle),NULL,0,ScrY-4);

	if (Opt.CPMenuMode)
	{
		string strTableTitle=MSG(MGetTableTitle);
		strTableTitle+=L"*";
		tables->SetTitle(strTableTitle);
	}

	tables->SetBottomTitle(MSG(MGetTableBottomTitle));

	nCurCP=nCurrent;
	EnumSystemCodePagesW ((CODEPAGE_ENUMPROCW)EnumCodePagesProc, CP_INSTALLED);

	tables->SetFlags(VMENU_WRAPMODE|VMENU_AUTOHIGHLIGHT);
	tables->SetPosition(-1,-1,0,0);
	tables->SortItems(0);

	AddUnicodeTables(*tables,nCurCP,bShowUnicode,bShowUTF);

	tables->Show();
	while (!tables->Done())
	{
		int Key=tables->ReadInput();
		switch (Key)
		{
		case KEY_CTRLH:
		{
			Opt.CPMenuMode=!Opt.CPMenuMode;
			tables->DeleteItems();
			EnumSystemCodePagesW((CODEPAGE_ENUMPROCW)EnumCodePagesProc, CP_INSTALLED);
			tables->SetPosition(-1,-1,0,0);
			tables->SortItems(0);
			AddUnicodeTables(*tables,nCurCP,bShowUnicode,bShowUTF);
			string strTableTitle=MSG(MGetTableTitle);
			if (Opt.CPMenuMode)
				strTableTitle+=L"*";
			tables->SetTitle(strTableTitle);
			tables->Show();
			break;
		}
		case KEY_INS:
		{
			if(tables->GetSelectPos()>1) // BUGBUG
			{
				MenuItemEx *CurItem=tables->GetItemPtr();
				CurItem->SetCheck(!(CurItem->Flags&LIF_CHECKED));
				string strCPName;
				strCPName.Format(L"%u",CurItem->UserData);
				if(CurItem->Flags&LIF_CHECKED)
					SetRegKey(SelectedCodeTables,strCPName,1);
				else
					DeleteRegValue(SelectedCodeTables,strCPName);
				tables->Show();
			}
			break;
		}
		default:
			tables->ProcessInput();
			break;
		}
	}

	if ( tables->Modal::GetExitCode() >= 0 )
		nCP = (UINT)(UINT_PTR)tables->GetUserData(NULL, 0);

	delete tables;

	return nCP;
}
