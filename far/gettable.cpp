/*
gettable.cpp

Работа с таблицами символов

*/

#include "headers.hpp"
#pragma hdrstop

#include "global.hpp"
#include "plugin.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "vmenu.hpp"
#include "savefpos.hpp"

static unsigned long CalcDifference(int *SrcTable,int *CheckedTable,unsigned char *DecodeTable);

/* 15.09.2000 IS
   Проверяет, установлена ли таблица с распределением частот символов
*/
int DistrTableExist(void)
{
	return (CheckRegValue("CodeTables","Distribution"));
}

int GetTable(struct CharTableSet *TableSet,int AnsiText,int &TableNum,
             int &UseUnicode)
{
	int I;
	char ItemName[128],t[128],t2[128];

	if (AnsiText)
	{
		char toUpper[2], toLower[2], decode[2], encode[2];
		toUpper[1] = toLower[1] = decode[1] = encode[1] = 0;

		for (I=0; I<256; I++)
		{
			*toUpper=*toLower=*decode=*encode=I;

			if (I > 127) //AY: символы до 128 не должны перекодироватся
			{
				FAR_CharToOem(decode, decode);
				FAR_OemToChar(encode, encode);
			}

			if (IsCharAlpha(I))
			{
				CharUpper(toUpper);
				CharLower(toLower);
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
	struct MenuItem ListItem;
	memset(&ListItem,0,sizeof(ListItem));
	ListItem.SetSelect(!TableNum);
	strcpy(ListItem.Name,MSG(MGetTableNormalText));
	TableList.SetUserData((void*)0,sizeof(DWORD),TableList.AddItem(&ListItem));

	if (UseUnicode)
	{
		ListItem.SetSelect(TableNum==1);
		strcpy(ListItem.Name,"Unicode");
		TableList.SetUserData((void*)1,sizeof(DWORD),TableList.AddItem(&ListItem));
	}

	for (I=0;; I++)
	{
		char TableKey[512];

		if (!EnumRegKey("CodeTables",I,TableKey,sizeof(TableKey)))
			break;

		FAR_CharToOem(PointToName(TableKey),ItemName);
		sprintf(t,"CodeTables\\%s",ItemName);
		GetRegKey(t,"TableName",t2,ItemName,128);
		strcpy(ListItem.Name,t2);
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

	if (!GetRegKey("CodeTables","Distribution",(BYTE *)DistrTable,(BYTE *)NULL,sizeof(DistrTable)))
	{
		TableNum=0;
		return(FALSE);
	}

	SaveFilePos SavePos(SrcFile);
	fseek64(SrcFile,0,SEEK_SET);
	int ProcessedSize=0;
	memset(FileDistr,0,sizeof(FileDistr));

	for (int Attempt=0;; Attempt++)
	{
		if ((ReadSize=(int)fread(FileData,1,sizeof(FileData),SrcFile))==0)
			break;

		int TextData=TRUE;

		for (int I=4; I<ReadSize; I++)
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
			else if (Ch>127 && Ch==FileData[I-2] && Ch==FileData[I-3])
			{
				fseek64(SrcFile,I-ReadSize+256,SEEK_CUR);
				TextData=FALSE;
				break;
			}
		}

		if (ProcessedSize>1024 || TextData && Attempt>4 || !TextData && Attempt>8)
			break;
	}

	if (ProcessedSize<10)
	{
		TableNum=0;
		return(FALSE);
	}

	int MaxDistr=0,MaxFileDistr=0,I;

	for (I=0; I<sizeof(DistrTable)/sizeof(DistrTable[0]); I++)
		if (DistrTable[I]!=0xff && DistrTable[I]>MaxDistr)
			MaxDistr=DistrTable[I];

	for (I=0; I<sizeof(FileDistr)/sizeof(FileDistr[0]); I++)
		if (FileDistr[I]!=0xff && FileDistr[I]>MaxFileDistr)
			MaxFileDistr=FileDistr[I];

	int SrcTable[256],CheckedTable[256];

	for (I=0; I<sizeof(DistrTable)/sizeof(DistrTable[0]); I++)
		if (DistrTable[I]==0xff)
			SrcTable[I]=-1;
		else
			SrcTable[I]=MaxFileDistr*DistrTable[I];

	for (I=0; I<sizeof(FileDistr)/sizeof(FileDistr[0]); I++)
		if (FileDistr[I]==0xff)
			CheckedTable[I]=-1;
		else
			CheckedTable[I]=MaxDistr*FileDistr[I];

	unsigned long BestValue=CalcDifference(SrcTable,CheckedTable,NULL);
	int BestTable=-1;

	for (I=0;; I++)
	{
		char TableKey[512];

		if (!EnumRegKey("CodeTables",I,TableKey,sizeof(TableKey)))
			break;

		if (!GetRegKey(TableKey,"AutoDetect",1))
			continue;

		unsigned char DecodeTable[256];

		if (!GetRegKey(TableKey,"Mapping",(BYTE *)DecodeTable,(BYTE *)NULL,sizeof(DecodeTable)))
			continue; //return(FALSE);

		unsigned long CurValue=CalcDifference(SrcTable,CheckedTable,DecodeTable);

		if (CurValue<(5*BestValue)/6 || CurValue<BestValue && BestTable!=-1)
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

	for (I=0; I<256; I++)
	{
		EncodeTable[I]=I;
		CheckedTableProcessed[I]=FALSE;
	}

	if (DecodeTable!=NULL)
		for (I=0; I<256; I++)
			EncodeTable[DecodeTable[I]]=I;

	unsigned long Diff=0;

	for (I=0; I<256; I++)
		if (SrcTable[I]!=-1)
		{
			int N=EncodeTable[I];
			Diff+=abs(SrcTable[I]-CheckedTable[N]);
			CheckedTableProcessed[N]=TRUE;
		}

	for (I=0; I<256; I++)
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

	for (I=0; I<256; I++)
	{
		TableSet->DecodeTable[I]=TableSet->EncodeTable[I]=I;
		TableSet->LowerTable[I]=TableSet->UpperTable[I]=I;
	}

	char TableKey[512];

	if (!EnumRegKey("CodeTables",TableNum,TableKey,sizeof(TableKey)))
		return(FALSE);

	if (!GetRegKey(TableKey,"Mapping",(BYTE *)TableSet->DecodeTable,(BYTE *)NULL,sizeof(TableSet->DecodeTable)))
		return(FALSE);

	if (UseTableName)
		GetRegKey(TableKey,"TableName",TableSet->TableName,PointToName(TableKey),sizeof(TableSet->TableName));
	else
		xstrncpy(TableSet->TableName,PointToName(TableKey),sizeof(TableSet->TableName)-1);

	//GetRegKey(TableKey,"RFCCharset",TableSet->RFCCharset,"",sizeof(TableSet->RFCCharset));
	int EncodeSet[256];
	memset(EncodeSet,0,sizeof(EncodeSet));

	for (I=0; I<256; I++)
	{
		int Ch=TableSet->DecodeTable[I];

		if (!EncodeSet[Ch] || Ch>=128)
		{
			TableSet->EncodeTable[Ch]=I;
			EncodeSet[Ch]=TRUE;
		}
	}

	for (I=0; I<256; I++)
	{
		int Ch=(BYTE)TableSet->DecodeTable[I];

		if (I==(BYTE)TableSet->EncodeTable[Ch] && LocalIsalpha(Ch))
		{
			TableSet->LowerTable[I]=TableSet->EncodeTable[LocalLower(Ch)];
			TableSet->UpperTable[I]=TableSet->EncodeTable[LocalUpper(Ch)];
		}
	}

	return(TRUE);
}
