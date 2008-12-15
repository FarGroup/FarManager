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

static unsigned long CalcDifference(int *SrcTable,int *CheckedTable,unsigned char *DecodeTable);

/* 15.09.2000 IS
   Проверяет, установлена ли таблица с распределением частот символов
*/
int DistrTableExist(void)
{
 return (CheckRegValue(L"CodeTables",L"Distribution"));
}

static VMenu *tables;
static DWORD dwCurCP;

const wchar_t SelectedCodeTables[]=L"CodeTables\\Selected";

BOOL __stdcall EnumCodePagesProc (const wchar_t *lpwszCodePage)
{
	int Check=0;
	GetRegKey(SelectedCodeTables,lpwszCodePage,Check,0);
	if(Opt.CPMenuMode && !Check)
		return TRUE;

	DWORD dwCP = _wtoi(lpwszCodePage);
	CPINFOEXW cpi;
	if (GetCPInfoExW (dwCP, 0, &cpi) && cpi.MaxCharSize == 1 )
	{
		MenuItemEx item;
		item.Clear ();
		if(dwCP==dwCurCP)
			item.Flags|=MIF_SELECTED;
		if(Check)
			item.Flags|=MIF_CHECKED;
		item.strName.Format(L"%5d%c %s",dwCP,BoxSymbols[BS_V1],wcschr(cpi.CodePageName,L'(')+1);
		item.strName.SetLength(item.strName.GetLength()-1);
		tables->SetUserData((void*)(DWORD_PTR)dwCP, sizeof (DWORD), tables->AddItem(&item));
	}
	return TRUE;
}


int GetTableEx (DWORD dwCurrent)
{
    int nCP = -1;

    tables = new VMenu (MSG(MGetTableTitle),NULL,0,ScrY-4);
	if(Opt.CPMenuMode)
	{
		string strTableTitle=MSG(MGetTableTitle);
		strTableTitle+=L"*";
		tables->SetTitle(strTableTitle);
	}
	tables->SetBottomTitle(MSG(MGetTableBottomTitle));

    MenuItemEx item;

/*    //unicode
    item.Clear ();
    item.strName = "UNICODE";

    tables->SetUserData((void*)CP_UNICODE, sizeof (DWORD), tables->AddItemW (&item));

    //reversebom
    item.Clear ();
    item.strName = "UNICODE (reverse BOM)";

    tables->SetUserData((void*)CP_REVERSEBOM, sizeof (DWORD), tables->AddItemW (&item));

    //utf-8
    item.Clear ();
    item.strName = "UTF-8";

    tables->SetUserData((void*)CP_UTF8, sizeof (DWORD), tables->AddItemW (&item));

    //utf-7
    item.Clear ();
    item.strName = "UTF-7";

    tables->SetUserData((void*)CP_UTF7, sizeof (DWORD), tables->AddItemW (&item));*/
    dwCurCP=dwCurrent;
    EnumSystemCodePagesW ((CODEPAGE_ENUMPROCW)EnumCodePagesProc, CP_INSTALLED);

    tables->SetFlags(VMENU_WRAPMODE|VMENU_AUTOHIGHLIGHT);
    tables->SetPosition(-1,-1,0,0);
	tables->SortItems(0);

	tables->Show();
	while(!tables->Done())
	{
		int Key=tables->ReadInput();
		switch(Key)
		{
		case KEY_CTRLH:
		{
			Opt.CPMenuMode=!Opt.CPMenuMode;
			tables->DeleteItems();
			EnumSystemCodePagesW((CODEPAGE_ENUMPROCW)EnumCodePagesProc, CP_INSTALLED);
			tables->SetPosition(-1,-1,0,0);
			tables->SortItems(0);
			string strTableTitle=MSG(MGetTableTitle);
			if(Opt.CPMenuMode)
				strTableTitle+=L"*";
			tables->SetTitle(strTableTitle);
			tables->Show();
			break;
		}
		case KEY_INS:
		{
			MenuItemEx *CurItem=tables->GetItemPtr();
			CurItem->SetCheck(!(CurItem->Flags&LIF_CHECKED));
			string strCPName;
			strCPName.Format(L"%d",CurItem->UserData);
			if(CurItem->Flags&LIF_CHECKED)
				SetRegKey(SelectedCodeTables,strCPName,1);
			else
				DeleteRegValue(SelectedCodeTables,strCPName);
			tables->Show();
			break;
		}
		default:
			tables->ProcessInput();
			break;
		}
	}

    if ( tables->Modal::GetExitCode() >= 0 )
        nCP = (int)(INT_PTR)tables->GetUserData(NULL, 0);

    delete tables;

    return nCP;
}

int GetTable(struct CharTableSet *TableSet,int AnsiText,int &TableNum,
             int &UseUnicode)
{
  int I;
  string strItemName,t,t2;

  if (AnsiText)
  {
    char toUpper[2], toLower[2], decode[2], encode[2];
    toUpper[1] = toLower[1] = decode[1] = encode[1] = 0;
    for (I=0;I<256;I++)
    {
      *toUpper=*toLower=*decode=*encode=I;
      if (I > 127) //AY: символы до 128 не должны перекодироватся
      {
        FAR_CharToOem(decode, decode);
        FAR_OemToChar(encode, encode);
      }
      if(IsCharAlphaA(I))
      {
        CharUpperA(toUpper);
        CharLowerA(toLower);
      }
      TableSet->EncodeTable[I] = *encode;
      TableSet->DecodeTable[I] = *decode;
      TableSet->UpperTable[I] = *toUpper;
      TableSet->LowerTable[I] = *toLower;
    }
    strcpy(TableSet->TableName,"Win");
    return(TRUE);
  }

  VMenu TableList(MSG(MGetTableTitle),NULL,0,ScrY-4);
  TableList.SetFlags(VMENU_WRAPMODE);
  TableList.SetPosition(-1,-1,0,0);

  MenuItemEx ListItem;

  ListItem.Clear ();

  ListItem.SetSelect(!TableNum);
  ListItem.strName = MSG(MGetTableNormalText);
  TableList.SetUserData((void*)0,sizeof(DWORD),TableList.AddItem(&ListItem));

  if (UseUnicode)
  {
    ListItem.SetSelect(TableNum==1);
    ListItem.strName = L"Unicode";
    TableList.SetUserData((void*)1,sizeof(DWORD),TableList.AddItem(&ListItem));
  }

  for (I=0;;I++)
  {
    string strTableKey;
    if (!EnumRegKey(L"CodeTables",I,strTableKey))
      break;

    strItemName = PointToName(strTableKey);
    t.Format (L"CodeTables\\%s", (const wchar_t*)strItemName);
    GetRegKey(t,L"TableName",t2,strItemName);
    ListItem.strName = t2;
    ListItem.SetSelect(I+1+UseUnicode == TableNum);
    TableList.SetUserData((void*)(INT_PTR)(I+1+UseUnicode),sizeof(I),TableList.AddItem(&ListItem));
  }

  //TableList.SetSelectPos(1+UseUnicode == TableNum,1);
  TableList.AssignHighlights(FALSE);
  TableList.SortItems(0,0);
  TableList.Process();

  int Pos=-1;
  if (TableList.Modal::GetExitCode()>=0)
    Pos=(int)(INT_PTR)TableList.GetUserData(NULL,0);

  if (Pos>UseUnicode && !PrepareTable(TableSet,Pos-1-UseUnicode))
    return(FALSE);

  UseUnicode=UseUnicode && Pos==1;


  if (Pos==-1)
    return(-1);
  TableNum=Pos;

  return(Pos!=0);
}




void DecodeString(char *Str,unsigned char *DecodeTable,int Length)
{
  if (Length==-1)
    Length=(int)strlen(Str);
  while (Length--)
  {
    *Str=DecodeTable[(unsigned int)*Str];
    Str++;
  }
}


void EncodeString(char *Str,unsigned char *EncodeTable,int Length)
{
  if (Length==-1)
    Length=(int)strlen(Str);
  while (Length--)
  {
    *Str=EncodeTable[(unsigned int)*Str];
    Str++;
  }
}



int DetectTable(FILE *SrcFile,struct CharTableSet *TableSet,int &TableNum)
{
  unsigned char DistrTable[256],FileDistr[256],FileData[4096];
  int ReadSize;
  if (!GetRegKey(L"CodeTables",L"Distribution",(BYTE *)DistrTable,(BYTE *)NULL,sizeof(DistrTable)))
  {
    TableNum=0;
    return(FALSE);
  }
  SaveFilePos SavePos(SrcFile);
  fseek64(SrcFile,0,SEEK_SET);

  int ProcessedSize=0;
  memset(FileDistr,0,sizeof(FileDistr));
  for (int Attempt=0;;Attempt++)
  {
    if ((ReadSize=(int)fread(FileData,1,sizeof(FileData),SrcFile))==0)
      break;
    int TextData=TRUE;
    for (int I=4;I<ReadSize;I++)
    {
      int Ch=FileData[I];
      if (DistrTable[Ch]==0xff)
        continue;
      if (Ch!=FileData[I-1])
      {
        ProcessedSize++;
        if (++FileDistr[Ch]==254)
          break;
      }
      else
        if (Ch>127 && Ch==FileData[I-2] && Ch==FileData[I-3])
        {
          fseek64(SrcFile,I-ReadSize+256,SEEK_CUR);
          TextData=FALSE;
          break;
        }
    }
    if (ProcessedSize>1024 || (TextData && Attempt>4) || (!TextData && Attempt>8))
      break;
  }

  if (ProcessedSize<10)
  {
    TableNum=0;
    return(FALSE);
  }

  int MaxDistr=0,MaxFileDistr=0;
  for (size_t I=0;I<countof(DistrTable);I++)
    if (DistrTable[I]!=0xff && DistrTable[I]>MaxDistr)
      MaxDistr=DistrTable[I];
  for (size_t I=0;I<countof(FileDistr);I++)
    if (FileDistr[I]!=0xff && FileDistr[I]>MaxFileDistr)
      MaxFileDistr=FileDistr[I];

  int SrcTable[256],CheckedTable[256];
  for (size_t I=0;I<countof(DistrTable);I++)
    if (DistrTable[I]==0xff)
      SrcTable[I]=-1;
    else
      SrcTable[I]=MaxFileDistr*DistrTable[I];
  for (size_t I=0;I<countof(FileDistr);I++)
    if (FileDistr[I]==0xff)
      CheckedTable[I]=-1;
    else
      CheckedTable[I]=MaxDistr*FileDistr[I];

  unsigned long BestValue=CalcDifference(SrcTable,CheckedTable,NULL);
  int BestTable=-1;

  for (int I=0;;I++)
  {
    string strTableKey;
    if (!EnumRegKey(L"CodeTables",I,strTableKey))
      break;

    if(!GetRegKey(strTableKey,L"AutoDetect",1))
      continue;

    unsigned char DecodeTable[256];
    if (!GetRegKey(strTableKey,L"Mapping",(BYTE *)DecodeTable,(BYTE *)NULL,sizeof(DecodeTable)))
      continue; //return(FALSE);

    unsigned long CurValue=CalcDifference(SrcTable,CheckedTable,DecodeTable);
    if (CurValue<(5*BestValue)/6 || (CurValue<BestValue && BestTable!=-1))
    {
      BestValue=CurValue;
      BestTable=I;
    }
  }

  if (BestTable==-1)
  {
    TableNum=0;
    return(FALSE);
  }
  if (!PrepareTable(TableSet,BestTable))
  {
    TableNum=0;
    return(FALSE);
  }
  TableNum=BestTable+1;
  return(TRUE);
}


static unsigned long CalcDifference(int *SrcTable,int *CheckedTable,unsigned char *DecodeTable)
{
  unsigned char EncodeTable[256];
  int CheckedTableProcessed[256];
  int I;
  for (I=0;I<256;I++)
  {
    EncodeTable[I]=I;
    CheckedTableProcessed[I]=FALSE;
  }
  if (DecodeTable!=NULL)
    for (I=0;I<256;I++)
      EncodeTable[DecodeTable[I]]=I;
  unsigned long Diff=0;
  for (I=0;I<256;I++)
    if (SrcTable[I]!=-1)
    {
      int N=EncodeTable[I];
      Diff+=abs(SrcTable[I]-CheckedTable[N]);
      CheckedTableProcessed[N]=TRUE;
    }
  for (I=0;I<256;I++)
    if (SrcTable[I]!=-1 && !CheckedTableProcessed[I])
      Diff+=CheckedTable[I];
  return(Diff);
}


/* $ 17.03.2002 IS
  + PrepareTable: параметр UseTableName - в качестве имени таблицы
    использовать не имя ключа реестра, а соответствующую переменную.
    По умолчанию - FALSE (использовать имя ключа).
*/
int PrepareTable(struct CharTableSet *TableSet,int TableNum,BOOL UseTableName)
{
  int I;
  for (I=0;I<256;I++)
  {
    TableSet->DecodeTable[I]=TableSet->EncodeTable[I]=I;
    TableSet->LowerTable[I]=TableSet->UpperTable[I]=I;
  }
  string strTableKey;
  if (!EnumRegKey(L"CodeTables",TableNum,strTableKey))
    return(FALSE);
  if (!GetRegKey(strTableKey,L"Mapping",(BYTE *)TableSet->DecodeTable,(BYTE *)NULL,sizeof(TableSet->DecodeTable)))
    return(FALSE);

  char TableNameDefault[sizeof(TableSet->TableName)];
  UnicodeToAnsi (PointToName(strTableKey),TableNameDefault,sizeof(TableNameDefault));

  if(UseTableName)
    GetRegKey(strTableKey,L"TableName",(BYTE*)TableSet->TableName,(const BYTE*)TableNameDefault,sizeof(TableSet->TableName));
  else
    xstrncpy(TableSet->TableName,TableNameDefault,sizeof(TableSet->TableName)-1);

  //GetRegKey(TableKey,"RFCCharset",TableSet->RFCCharset,"",sizeof(TableSet->RFCCharset));

  int EncodeSet[256];
  memset(EncodeSet,0,sizeof(EncodeSet));
  for (I=0;I<256;I++)
  {
    int Ch=TableSet->DecodeTable[I];
    if (!EncodeSet[Ch] || Ch>=128)
    {
      TableSet->EncodeTable[Ch]=I;
      EncodeSet[Ch]=TRUE;
    }
  }
  for (I=0;I<256;I++)
  {
    int Ch=(BYTE)TableSet->DecodeTable[I];
    if(I==(BYTE)TableSet->EncodeTable[Ch] && LocalIsalpha(Ch))
    {
      TableSet->LowerTable[I]=TableSet->EncodeTable[LocalLower(Ch)];
      TableSet->UpperTable[I]=TableSet->EncodeTable[LocalUpper(Ch)];
    }
  }
  return(TRUE);
}
