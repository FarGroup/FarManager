/*
gettable.cpp

Работа с таблицами символов

*/

/* Revision: 1.21 06.08.2004 $ */

/*
Modify:
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  14.04.2004 SVS
    - BugZ#1055 - проблема с позиционированием начальной кодировки в меню по alt-f8
  01.03.2004 SVS
    ! Обертки FAR_OemTo* и FAR_CharTo* вокруг одноименных WinAPI-функций
      (задел на будущее + править впоследствии только 1 файл)
  14.01.2004 SVS
    ! Если есть "AutoDetect" и он равен 0, то исключаем таблицу из автодетекта
      (см. BugZ#158 - фича)
    ! если есть ключ, но нет "Mapping", то не вываливаем, а продолжаем сканировать
      следующие таблицы!
  10.12.2002 SVS
    ! Уберем условную компиляцию (а зачем ее делали то?)
  29.10.2002 SVS
    - Блин, с этой сраной сортировкой поломал GetTable()...
  22.10.2002 SVS
    ! добавка CharTableSet.RFCCharset, но закомменченная - чтобы потом не
      думать как ЭТО сделать ;-)
    ! изменена GetTable() - сортируем 1 раз, после заполнения списка.
  17.03.2002 IS
    + PrepareTable: параметр UseTableName - в качестве имени таблицы
      использовать не имя ключа реестра, а соответствующую переменную.
      По умолчанию - FALSE (использовать имя ключа).
  22.02.2002 SVS
    ! Заюзаем fseek64
  07.08.2001 IS
    - Баги: некоторые символы считались буквами, даже если они таковыми не
      являлись.
  26.07.2001 SVS
    ! VFMenu уничтожен как класс
  22.07.2001 SVS
    ! Избавляемся от варнингов
  18.07.2001 OT
    ! VFMenu
  03.06.2001 IS
    - Баг: некорректно генерировалась кодовая таблица в PrepareTable, в
      частности, для cp1251 после этой функции символы 0x84 (открывающие
      кавычки) и 0x93 (закрывающие кавычки) путались с "буквами", потому что
      как и буквы имели различные значения в LowerTable и UpperTable.
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
    ! Константы MENU_ - в морг
  06.05.2001 DJ
    ! перетрях #include
  11.02.2001 SVS
    ! Несколько уточнений кода в связи с изменениями в структуре MenuItem
  15.09.2000 IS
    + Функция DistrTableExist - проверяет, установлена ли таблица с
      распределением частот символов, возвращает TRUE в случае успеха
  08.09.2000 tran 1.03
    + menu from registry
  27.08.2000 tran
    + hotkeys as numbers
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
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
/* IS $ */

int GetTable(struct CharTableSet *TableSet,int AnsiText,int &TableNum,
             int &UseUnicode)
{
  int I;
  char ItemName[128],t[128],t2[128];

  if (AnsiText)
  {
    /* $ 07.08.2001 IS
       + Для "небукв" нет смысла извращаться с UpperTable и LowerTable
       ! Исключим лишний цикл
    */
    char toUpper[2], toLower[2], decode[2], encode[2];
    toUpper[1] = toLower[1] = decode[1] = encode[1] = 0;
    for (I=0;I<256;I++)
    {
       *toUpper=*toLower=*decode=*encode=I;
       FAR_CharToOem(decode, decode);
       FAR_OemToChar(encode, encode);
       if(IsCharAlpha(I))
       {
         CharUpper(toUpper);
         CharLower(toLower);
       }
       TableSet->EncodeTable[I] = *encode;
       TableSet->DecodeTable[I] = *decode;
       TableSet->UpperTable[I] = *toUpper;
       TableSet->LowerTable[I] = *toLower;
    }
    /* IS $ */
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

  for (I=0;;I++)
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
    Length=strlen(Str);
  while (Length--)
  {
    *Str=DecodeTable[(unsigned int)*Str];
    Str++;
  }
}


void EncodeString(char *Str,unsigned char *EncodeTable,int Length)
{
  if (Length==-1)
    Length=strlen(Str);
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
  for (int Attempt=0;;Attempt++)
  {
    if ((ReadSize=fread(FileData,1,sizeof(FileData),SrcFile))==0)
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
    if (ProcessedSize>1024 || TextData && Attempt>4 || !TextData && Attempt>8)
      break;
  }

  if (ProcessedSize<10)
  {
    TableNum=0;
    return(FALSE);
  }

  int MaxDistr=0,MaxFileDistr=0,I;
  for (I=0;I<sizeof(DistrTable)/sizeof(DistrTable[0]);I++)
    if (DistrTable[I]!=0xff && DistrTable[I]>MaxDistr)
      MaxDistr=DistrTable[I];
  for (I=0;I<sizeof(FileDistr)/sizeof(FileDistr[0]);I++)
    if (FileDistr[I]!=0xff && FileDistr[I]>MaxFileDistr)
      MaxFileDistr=FileDistr[I];

  int SrcTable[256],CheckedTable[256];
  for (I=0;I<sizeof(DistrTable)/sizeof(DistrTable[0]);I++)
    if (DistrTable[I]==0xff)
      SrcTable[I]=-1;
    else
      SrcTable[I]=MaxFileDistr*DistrTable[I];
  for (I=0;I<sizeof(FileDistr)/sizeof(FileDistr[0]);I++)
    if (FileDistr[I]==0xff)
      CheckedTable[I]=-1;
    else
      CheckedTable[I]=MaxDistr*FileDistr[I];

  unsigned long BestValue=CalcDifference(SrcTable,CheckedTable,NULL);
  int BestTable=-1;

  for (I=0;;I++)
  {
    char TableKey[512];
    if (!EnumRegKey("CodeTables",I,TableKey,sizeof(TableKey)))
      break;

    if(!GetRegKey(TableKey,"AutoDetect",1))
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
  char TableKey[512];
  if (!EnumRegKey("CodeTables",TableNum,TableKey,sizeof(TableKey)))
    return(FALSE);
  if (!GetRegKey(TableKey,"Mapping",(BYTE *)TableSet->DecodeTable,(BYTE *)NULL,sizeof(TableSet->DecodeTable)))
    return(FALSE);

  if(UseTableName)
    GetRegKey(TableKey,"TableName",TableSet->TableName,PointToName(TableKey),sizeof(TableSet->TableName));
  else
    xstrncpy(TableSet->TableName,PointToName(TableKey),sizeof(TableSet->TableName)-1);

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
  /* $ 07.08.2001 IS
     + Для "небукв" нет смысла извращаться с UpperTable и LowerTable
  */
  for (I=0;I<256;I++)
  {
    int Ch=(BYTE)TableSet->DecodeTable[I];
    if(I==(BYTE)TableSet->EncodeTable[Ch] && LocalIsalpha(Ch))
    {
      TableSet->LowerTable[I]=TableSet->EncodeTable[LocalLower(Ch)];
      TableSet->UpperTable[I]=TableSet->EncodeTable[LocalUpper(Ch)];
    }
  }
  /* IS $ */
  return(TRUE);
}
/* IS 17.03.2002 $ */
