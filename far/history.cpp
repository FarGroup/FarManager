/*
history.cpp

История (Alt-F8, Alt-F11, Alt-F12)

*/

/* Revision: 1.03 11.02.2001 $ */

/*
Modify:
  11.02.2001 SVS
    ! Несколько уточнений кода в связи с изменениями в структуре MenuItem
  09.01.2001 SVS
    - Бага с CmdHistoryRule=1
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

History::History(char *RegKey,int *EnableSave,int SaveTitle,int SaveType)
{
  LastPtr=CurLastPtr=LastPtr0=CurLastPtr0=0;
  memset(LastStr,0,sizeof(LastStr));
  History::SaveTitle=SaveTitle;
  History::SaveType=SaveType;
  History::EnableSave=EnableSave;
  strcpy(History::RegKey,RegKey);
  EnableAdd=RemoveDups=TRUE;
  KeepSelectedPos=FALSE;
  LastSimilar=0;
  ReturnSimilarTemplate=TRUE;
}


void History::AddToHistory(char *Str,char *Title,int Type)
{
  if (!EnableAdd)
    return;
  if (*EnableSave)
  {
    struct HistoryRecord SaveLastStr[64];
    unsigned int SaveLastPtr=LastPtr,SaveCurLastPtr=CurLastPtr;
    memcpy(SaveLastStr,LastStr,sizeof(SaveLastStr));
    ReadHistory();
    AddToHistoryLocal(Str,Title,Type);
    SaveHistory();
    LastPtr0=LastPtr=SaveLastPtr;
    CurLastPtr0=CurLastPtr=SaveCurLastPtr;
    memcpy(LastStr,SaveLastStr,sizeof(LastStr));
  }
  AddToHistoryLocal(Str,Title,Type);
}


void History::AddToHistoryLocal(char *Str,char *Title,int Type)
{
  struct HistoryRecord AddRecord;
  strncpy(AddRecord.Name,Str,sizeof(AddRecord.Name));
  strncpy(AddRecord.Title,NullToEmpty(Title),sizeof(AddRecord.Title));
  AddRecord.Type=Type;
  RemoveTrailingSpaces(AddRecord.Name);
  RemoveTrailingSpaces(AddRecord.Title);
  int OldLastPtr;
  if ((OldLastPtr=LastPtr-1) < 0)
    OldLastPtr=sizeof(LastStr)/sizeof(LastStr[0])-1;
  if (RemoveDups)
  {
    for (int I=0;I<sizeof(LastStr)/sizeof(LastStr[0]);I++)
    {
      int Equal=RemoveDups==1 && strcmp(AddRecord.Name,LastStr[I].Name)==0 &&
                strcmp(AddRecord.Title,LastStr[I].Title)==0 ||
                RemoveDups==2 && LocalStricmp(AddRecord.Name,LastStr[I].Name)==0 &&
                LocalStricmp(AddRecord.Title,LastStr[I].Title)==0;
      if (Equal && AddRecord.Type==LastStr[I].Type)
      {
        int Length=OldLastPtr-I;
        if (Length<0)
          Length+=sizeof(LastStr)/sizeof(LastStr[0]);
        for (int J=0;J<=Length;J++)
        {
          int Dest=(I+J) % (sizeof(LastStr)/sizeof(LastStr[0]));
          int Src=(I+J+1) % (sizeof(LastStr)/sizeof(LastStr[0]));
          LastStr[Dest]=LastStr[Src];
        }
        LastStr[OldLastPtr]=AddRecord;
        CurLastPtr0=LastPtr0=CurLastPtr=LastPtr;
        return;
      }
    }
  }
  int Pos=(LastPtr-1) % (sizeof(LastStr)/sizeof(LastStr[0]));
  if (strcmp(AddRecord.Name,LastStr[Pos].Name)!=0 ||
      strcmp(AddRecord.Title,LastStr[Pos].Title)!=0 ||
      AddRecord.Type!=LastStr[Pos].Type)
  {
    LastStr[LastPtr]=AddRecord;
    if (++LastPtr==sizeof(LastStr)/sizeof(LastStr[0]))
      LastPtr=0;
  }
  CurLastPtr0=LastPtr0=CurLastPtr=LastPtr;
}



void History::SaveHistory()
{
  if (!*EnableSave)
    return;

  char Buffer[sizeof(LastStr)];
  DWORD Size;
  HKEY hKey;

  if ((hKey=CreateRegKey(RegKey))==NULL)
    return;

  Size=0;
  for (int I=0;I<sizeof(LastStr)/sizeof(LastStr[0]);I++)
  {
    strcpy(Buffer+Size,LastStr[I].Name);
    Size+=strlen(LastStr[I].Name)+1;
  }
  Buffer[Size++]=0;
  RegSetValueEx(hKey,"Lines",0,REG_BINARY,(unsigned char *)Buffer,Size);

  if (SaveTitle)
  {
    Size=0;
    for (int I=0;I<sizeof(LastStr)/sizeof(LastStr[0]);I++)
    {
      strcpy(Buffer+Size,LastStr[I].Title);
      Size+=strlen(LastStr[I].Title)+1;
    }
    Buffer[Size++]=0;
    RegSetValueEx(hKey,"Titles",0,REG_BINARY,(unsigned char *)Buffer,Size);
  }

  if (SaveType)
  {
    for (Size=0;Size<sizeof(LastStr)/sizeof(LastStr[0]);Size++)
      Buffer[Size]=LastStr[Size].Type+'0';
    Buffer[Size++]=0;
    RegSetValueEx(hKey,"Types",0,REG_SZ,(unsigned char *)Buffer,Size);
  }

  RegSetValueEx(hKey,"Position",0,REG_DWORD,(BYTE *)&CurLastPtr,sizeof(CurLastPtr));

  RegCloseKey(hKey);
}


void History::ReadHistory()
{
  char Buffer[sizeof(LastStr)],*Buf;
  DWORD Size,Type;
  HKEY hKey;

  if ((hKey=OpenRegKey(RegKey))==NULL)
    return;
  Size=sizeof(Buffer);
  if (RegQueryValueEx(hKey,"Lines",0,&Type,(unsigned char *)Buffer,&Size)==ERROR_SUCCESS)
  {
    int StrPos=0;
    Buf=Buffer;
    while ((int)Size>=0 && StrPos<sizeof(LastStr)/sizeof(LastStr[0]))
    {
      strcpy(LastStr[StrPos++].Name,Buf);
      int Length=strlen(Buf)+1;
      Buf+=Length;
      Size-=Length;
    }
  }
  Size=sizeof(Buffer);
  if (SaveTitle && RegQueryValueEx(hKey,"Titles",0,&Type,(unsigned char *)Buffer,&Size)==ERROR_SUCCESS)
  {
    int StrPos=0;
    Buf=Buffer;
    while ((int)Size>=0 && StrPos<sizeof(LastStr)/sizeof(LastStr[0]))
    {
      strcpy(LastStr[StrPos++].Title,Buf);
      int Length=strlen(Buf)+1;
      Buf+=Length;
      Size-=Length;
    }
  }
  Size=sizeof(Buffer);
  if (SaveType && RegQueryValueEx(hKey,"Types",0,&Type,(unsigned char *)Buffer,&Size)==ERROR_SUCCESS)
  {
    int StrPos=0;
    Buf=Buffer;
    while (isdigit(*Buf) && StrPos<sizeof(LastStr)/sizeof(LastStr[0]))
    {
      LastStr[StrPos++].Type=*Buf-'0';
      Buf++;
    }
  }

  Size=sizeof(CurLastPtr);
  RegQueryValueEx(hKey,"Position",0,&Type,(BYTE *)&CurLastPtr,&Size);
  RegCloseKey(hKey);

  LastPtr0=CurLastPtr0=LastPtr=CurLastPtr;
}


int History::Select(char *Title,char *HelpTopic,char *Str,int &Type,char *ItemTitle)
{
  struct MenuItem HistoryItem;
  memset(&HistoryItem,0,sizeof(HistoryItem));

  int Line,CurCmd,Code,I,Height=ScrY-8;
  int LineToStr[sizeof(LastStr)/sizeof(LastStr[0])+1];
  int RetCode=1;

  {
    VMenu HistoryMenu(Title,NULL,0,Height);
    HistoryMenu.SetFlags(MENU_SHOWAMPERSAND);
    if (HelpTopic!=NULL)
      HistoryMenu.SetHelp(HelpTopic);
    HistoryMenu.SetPosition(-1,-1,0,0);
    for (CurCmd=LastPtr+1,Line=0,I=0;I<sizeof(LastStr)/sizeof(LastStr[0])-1;I++,CurCmd++)
    {
      CurCmd%=sizeof(LastStr)/sizeof(LastStr[0]);
      if (*LastStr[CurCmd].Name)
      {
        char Record[1024];
        if (*LastStr[CurCmd].Title)
          sprintf(Record,"%s: %s",LastStr[CurCmd].Title,LastStr[CurCmd].Name);
        else
          strcpy(Record,LastStr[CurCmd].Name);
        TruncStr(Record,Min(ScrX-12,sizeof(HistoryItem.Name)-1));
        strcpy(HistoryItem.Name,Record);
        HistoryItem.Selected=(CurCmd==CurLastPtr);
        LineToStr[Line++]=CurCmd;
        HistoryMenu.AddItem(&HistoryItem);
      }
    }
    sprintf(HistoryItem.Name,"%20s","");
    HistoryItem.Selected=(CurLastPtr==LastPtr);
    LineToStr[Line]=-1;
    HistoryMenu.AddItem(&HistoryItem);
    HistoryMenu.AssignHighlights(TRUE);
    HistoryMenu.Show();
    while (!HistoryMenu.Done())
    {
      int Key=HistoryMenu.ReadInput();
      if (Key==KEY_CTRLENTER || Key==KEY_SHIFTENTER)
      {
        HistoryMenu.SetExitCode(HistoryMenu.GetSelectPos());
        RetCode=(Key==KEY_SHIFTENTER ? 2 : 3);
        continue;
      }
      switch(Key)
      {
        case KEY_DEL:
          memset(LastStr,0,sizeof(LastStr));
          CurLastPtr=LastPtr=0;
          CurLastPtr0=LastPtr0=0;
          SaveHistory();
          HistoryMenu.Hide();
          return(Select(Title,HelpTopic,Str,Type));
        default:
          HistoryMenu.ProcessInput();
          break;
      }
    }
    if ((Code=HistoryMenu.GetExitCode())<0)
      return(0);
  }

  if (LineToStr[Code]==-1)
  {
    CurLastPtr0=LastPtr0=CurLastPtr=LastPtr;
    return(0);
  }
  int StrPos=LineToStr[Code];
  if (KeepSelectedPos)
    CurLastPtr0=CurLastPtr=StrPos;
  strcpy(Str,LastStr[StrPos].Name);
  Type=LastStr[StrPos].Type;
  if (ItemTitle!=NULL)
    strcpy(ItemTitle,LastStr[StrPos].Title);
  return(RetCode);
}


void History::GetPrev(char *Str)
{
  do
  {
    int NewPtr=(CurLastPtr-1)%(sizeof(LastStr)/sizeof(LastStr[0]));
    if (NewPtr!=LastPtr)
      CurLastPtr=NewPtr;
    else
      break;
  } while (*LastStr[CurLastPtr].Name==0);
  strcpy(Str,LastStr[CurLastPtr].Name);
}


void History::GetNext(char *Str)
{
  do
  {
    if (CurLastPtr!=LastPtr)
      CurLastPtr=(CurLastPtr+1)%(sizeof(LastStr)/sizeof(LastStr[0]));
    else
      break;
  } while (*LastStr[CurLastPtr].Name==0);
  strcpy(Str,CurLastPtr==LastPtr ? "":LastStr[CurLastPtr].Name);
}


void History::GetSimilar(char *Str,int LastCmdPartLength)
{
  int Length=strlen(Str);
  if (LastCmdPartLength!=-1 && LastCmdPartLength<Length)
    Length=LastCmdPartLength;
  if (LastCmdPartLength==-1)
    LastSimilar=0;
  for (int I=1;I<sizeof(LastStr)/sizeof(LastStr[0]);I++)
  {
    int Pos=(LastPtr-LastSimilar-I)%(sizeof(LastStr)/sizeof(LastStr[0]));
    char *Name=LastStr[Pos].Name;
    if (*Name && LocalStrnicmp(Str,Name,Length)==0 && strcmp(Str,Name)!=0)
    {
      int NewSimilar=(LastPtr-Pos)%(sizeof(LastStr)/sizeof(LastStr[0]));
      if (NewSimilar<=LastSimilar && ReturnSimilarTemplate)
      {
        ReturnSimilarTemplate=FALSE;
        Str[Length]=0;
      }
      else
      {
        ReturnSimilarTemplate=TRUE;
        strcpy(Str,Name);
        LastSimilar=NewSimilar;
      }
      return;
    }
  }
  LastSimilar=0;
  ReturnSimilarTemplate=TRUE;
  Str[Length]=0;
}


void History::SetAddMode(int EnableAdd,int RemoveDups,int KeepSelectedPos)
{
  History::EnableAdd=EnableAdd;
  History::RemoveDups=RemoveDups;
  History::KeepSelectedPos=KeepSelectedPos;
}

