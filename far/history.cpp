/*
history.cpp

История (Alt-F8, Alt-F11, Alt-F12)

*/

/* Revision: 1.21 26.02.2002 $ */

/*
Modify:
  26.02.2002 SVS
    ! Для пустого списка (Items > 1) ничего не делаем
  25.01.2002 SVS
    - мааааааааааасссссссссссссссссддддддддддддаааааааааааааааааайййййййййй!!!!
  25.01.2002 SVS
    ! Компонента Name теперь динамическая. Размер в масдае - max 511 байт.
      В NT - сколько есть. Т.е. обрезаение строк будет только в масдае.
  16.01.2002 VVM
    + AddToHistory - новый параметр
      SaveForbid - принудительно запретить запись добавляемой строки.
      Используется на панели плагина
  15,11,2001 SVS
    + Тип истории.
  08.11.2001 SVS
    ! Отмена пред.патча - есть неразрешимые вопросы с макросами.
  06.11.2001 IS
    ! Не добавляем в меню пустую строчку, т.к. непонятно, зачем она нужна
      вообще.
    ! Меню теперь у нас с прокруткой (Wrap).
  27.09.2001 IS
    - Левый размер при использовании strncpy
  26.07.2001 SVS
    ! VFMenu уничтожен как класс
  24.07.2001 SVS
    ! Учтем новую опцию Opt.Confirm.HistoryClear при очистки истории
  23.07.2001 VVM
    + Спросить подтверждение перед очищением истории
  18.07.2001 OT
    ! VFMenu
  06.06.2001 SVS
    ! Mix/Max
  04.06.2001 SVS
    ! 64 -> HISTORY_COUNT
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
    ! Константы MENU_ - в морг
  06.05.2001 DJ
    ! перетрях #include
  09.04.2001 SVS
    + Фича - копирование из истории строки в Clipboard
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

#include "history.hpp"
#include "fn.hpp"
#include "global.hpp"
#include "keys.hpp"
#include "vmenu.hpp"
#include "lang.hpp"

History::History(int TypeHistory,char *RegKey,int *EnableSave,int SaveTitle,int SaveType)
{
  FreeHistory(FALSE);
  strncpy(History::RegKey,RegKey,sizeof(History::RegKey)-1);
  History::SaveTitle=SaveTitle;
  History::SaveType=SaveType;
  History::EnableSave=EnableSave;
  History::TypeHistory=TypeHistory;
  EnableAdd=RemoveDups=TRUE;
  KeepSelectedPos=FALSE;
  ReturnSimilarTemplate=TRUE;
}

History::~History()
{
  FreeHistory(TRUE);
}

void History::FreeHistory(BOOL FreeMemody)
{
  if(FreeMemody)
    for (int I=0;I<sizeof(LastStr)/sizeof(LastStr[0]);I++)
      if(LastStr[I].Name)
        free(LastStr[I].Name);
  memset(LastStr,0,sizeof(LastStr));
  CurLastPtr=LastPtr=CurLastPtr0=LastPtr0=0;
  LastSimilar=0;
}

void History::AddToHistory(char *Str,char *Title,int Type,int SaveForbid)
{
  if (!EnableAdd)
    return;

  if (*EnableSave && !SaveForbid)
  {
    struct HistoryRecord SaveLastStr[HISTORY_COUNT];
    memset(SaveLastStr,0,sizeof(SaveLastStr));
    for (int I=0;I < sizeof(LastStr)/sizeof(LastStr[0]); I++)
    {
      if(LastStr[I].Name)
        SaveLastStr[I].Name=strdup(LastStr[I].Name);
      strcpy(SaveLastStr[I].Title,LastStr[I].Title);
      SaveLastStr[I].Type=LastStr[I].Type;
    }

    unsigned int SaveLastPtr=LastPtr,
                 SaveCurLastPtr=CurLastPtr,
                 SaveLastSimilar=LastSimilar;
    FreeHistory(TRUE);

    ReadHistory();
    LastPtr0=LastPtr=SaveLastPtr;
    CurLastPtr0=CurLastPtr=SaveCurLastPtr;
    LastSimilar=SaveLastSimilar;
    AddToHistoryLocal(Str,Title,Type);
    SaveHistory();

    LastPtr0=LastPtr=SaveLastPtr;
    CurLastPtr0=CurLastPtr=SaveCurLastPtr;
    LastSimilar=SaveLastSimilar;
    memcpy(LastStr,SaveLastStr,sizeof(LastStr));
  }
  AddToHistoryLocal(Str,Title,Type);
}


void History::AddToHistoryLocal(char *Str,char *Title,int Type)
{
  struct HistoryRecord AddRecord;

  if((AddRecord.Name=(char*)malloc((WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS?511:strlen(Str))+1)) == NULL)
    return;
  strncpy(AddRecord.Name,Str,WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS?511:strlen(Str));

  RemoveTrailingSpaces(AddRecord.Name);
  strncpy(AddRecord.Title,NullToEmpty(Title),sizeof(AddRecord.Title)-1);
  RemoveTrailingSpaces(AddRecord.Title);
  AddRecord.Type=Type;

  int OldLastPtr;
  if ((OldLastPtr=LastPtr-1) < 0)
    OldLastPtr=sizeof(LastStr)/sizeof(LastStr[0])-1;

  if (RemoveDups)
  {
    for (int I=0;I<sizeof(LastStr)/sizeof(LastStr[0]);I++)
    {
      if(LastStr[I].Name)
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

            free(LastStr[Dest].Name);
            memcpy(LastStr+Dest,LastStr+Src,sizeof(HistoryRecord));
            memset(LastStr+Src,0,sizeof(HistoryRecord));
          }

          LastStr[OldLastPtr]=AddRecord;
          CurLastPtr0=LastPtr0=CurLastPtr=LastPtr;
          return;
        }
      }
    }
  }

  int Pos=(LastPtr-1) % (sizeof(LastStr)/sizeof(LastStr[0]));

  if(LastStr[Pos].Name && (strcmp(AddRecord.Name,LastStr[Pos].Name)!=0 ||
        strcmp(AddRecord.Title,LastStr[Pos].Title)!=0 ||
        AddRecord.Type!=LastStr[Pos].Type))
    free(LastStr[LastPtr].Name);
   memcpy(LastStr+LastPtr,&AddRecord,sizeof(HistoryRecord));
   if (++LastPtr==sizeof(LastStr)/sizeof(LastStr[0]))
       LastPtr=0;
  CurLastPtr0=LastPtr0=CurLastPtr=LastPtr;
}


BOOL History::SaveHistory()
{
  if (!*EnableSave)
    return TRUE;

  HKEY hKey;

  if ((hKey=CreateRegKey(RegKey))==NULL)
    return FALSE;

  char *Buffer=NULL,*PtrBuffer;
  DWORD Size=0;
  int I;

  for (I=0; I < sizeof(LastStr)/sizeof(LastStr[0]); I++)
  {
    if(LastStr[I].Name)
    {
      PtrBuffer=(char*)realloc(Buffer,Size+strlen(LastStr[I].Name)+16);
      if(!PtrBuffer)
      {
        free(Buffer);
        RegCloseKey(hKey);
        return FALSE;
      }
      Buffer=PtrBuffer;
      strcpy(Buffer+Size,LastStr[I].Name);
      Size+=strlen(LastStr[I].Name)+1;
    }
  }

  if(!Buffer)
  {
    RegCloseKey(hKey);
    return FALSE;
  }

  Buffer[Size++]=0;
  RegSetValueEx(hKey,"Lines",0,REG_BINARY,(unsigned char *)Buffer,Size);

  if (SaveTitle)
  {
    PtrBuffer=(char*)realloc(Buffer,HISTORY_COUNT*(HISTORY_TITLESIZE+1)+16);
    if(!PtrBuffer)
    {
      free(Buffer);
      RegCloseKey(hKey);
      return FALSE;
    }
    Buffer=PtrBuffer;

    Size=0;
    for (I=0; I < sizeof(LastStr)/sizeof(LastStr[0]); I++)
    {
      strcpy(Buffer+Size,LastStr[I].Title);
      Size+=strlen(LastStr[I].Title)+1;
    }
    Buffer[Size++]=0;
    RegSetValueEx(hKey,"Titles",0,REG_BINARY,(unsigned char *)Buffer,Size);
  }

  if (SaveType)
  {
    unsigned char TypesBuffer[sizeof(LastStr)/sizeof(LastStr[0])+1];
    memset(TypesBuffer,0,sizeof(TypesBuffer));
    for (Size=0; Size < sizeof(LastStr)/sizeof(LastStr[0]); Size++)
      TypesBuffer[Size]=LastStr[Size].Type+'0';
    TypesBuffer[Size++]=0;
    RegSetValueEx(hKey,"Types",0,REG_SZ,TypesBuffer,Size);
  }

  RegSetValueEx(hKey,"Position",0,REG_DWORD,(BYTE *)&CurLastPtr,sizeof(CurLastPtr));
  RegCloseKey(hKey);
  free(Buffer);
  return TRUE;
}


BOOL History::ReadHistory()
{
  HKEY hKey;

  if ((hKey=OpenRegKey(RegKey))==NULL)
    return FALSE;

  char *Buffer=NULL,*Buf;
  DWORD Size,Type;

  Size=GetRegKeySize(hKey,"Lines");

  if(!Size) // Нету ничерта
    return TRUE;

  if((Buffer=(char*)malloc(Size)) == NULL)
  {
    RegCloseKey(hKey);
    return FALSE;
  }

  if (RegQueryValueEx(hKey,"Lines",0,&Type,(unsigned char *)Buffer,&Size)==ERROR_SUCCESS)
  {
    int StrPos=0;
    Buf=Buffer;
    while ((int)Size >= 0 && StrPos < sizeof(LastStr)/sizeof(LastStr[0]))
    {
      int Len=strlen(Buf);
      if(WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && Len > 511)
        Len=511;
      if((LastStr[StrPos].Name=(char*)malloc(Len+1)) == NULL)
      {
        free(Buffer);
        FreeHistory(TRUE);
        RegCloseKey(hKey);
        return FALSE;
      }
      strcpy(LastStr[StrPos].Name,Buf);
      StrPos++;
      int Length=strlen(Buf)+1;
      Buf+=Length;
      Size-=Length;
    }
  }

  if (SaveTitle)
  {
    Size=GetRegKeySize(hKey,"Titles");
    if((Buf=(char*)realloc(Buffer,Size)) == NULL)
    {
      free(Buffer);
      FreeHistory(TRUE);
      RegCloseKey(hKey);
      return FALSE;
    }
    Buffer=Buf;
    if(RegQueryValueEx(hKey,"Titles",0,&Type,(unsigned char *)Buffer,&Size)==ERROR_SUCCESS)
    {
      int StrPos=0;
      while ((int)Size >= 0 && StrPos < sizeof(LastStr)/sizeof(LastStr[0]))
      {
        strncpy(LastStr[StrPos].Title,Buf,sizeof(LastStr[StrPos].Title)-1);
        ++StrPos;
        int Length=strlen(Buf)+1;
        Buf+=Length;
        Size-=Length;
      }
    }
  }
  free(Buffer);

  if (SaveType)
  {
    unsigned char TypesBuffer[sizeof(LastStr)/sizeof(LastStr[0])+1];
    Size=sizeof(TypesBuffer);
    memset(TypesBuffer,0,Size);
    if(RegQueryValueEx(hKey,"Types",0,&Type,(unsigned char *)TypesBuffer,&Size)==ERROR_SUCCESS)
    {
      int StrPos=0;
      Buf=(char *)TypesBuffer;
      while (isdigit(*Buf) && StrPos < sizeof(LastStr)/sizeof(LastStr[0]))
      {
        LastStr[StrPos++].Type=*Buf-'0';
        Buf++;
      }
    }
  }

  Size=sizeof(CurLastPtr);
  RegQueryValueEx(hKey,"Position",0,&Type,(BYTE *)&CurLastPtr,&Size);
  RegCloseKey(hKey);

  LastPtr0=CurLastPtr0=LastPtr=CurLastPtr;
  return TRUE;
}


int History::Select(char *Title,char *HelpTopic,char *Str,int &Type,char *ItemTitle)
{
  struct MenuItem HistoryItem;
  memset(&HistoryItem,0,sizeof(HistoryItem));

  int Line,Code,I,Height=ScrY-8,StrPos;
  unsigned int CurCmd;
  int LineToStr[sizeof(LastStr)/sizeof(LastStr[0])+1];
  int RetCode=1;

  {
    VMenu HistoryMenu(Title,NULL,0,Height);
    /* $ 06.11.2001 IS
       ! Меню теперь у нас с прокруткой (Wrap)
    */
    HistoryMenu.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);
    /* IS $ */
    if (HelpTopic!=NULL)
      HistoryMenu.SetHelp(HelpTopic);
    HistoryMenu.SetPosition(-1,-1,0,0);

    for (CurCmd=LastPtr+1, Line=0, I=0; I < sizeof(LastStr)/sizeof(LastStr[0])-1; I++, CurCmd++)
    {
      CurCmd%=sizeof(LastStr)/sizeof(LastStr[0]);
      if (LastStr[CurCmd].Name && *LastStr[CurCmd].Name)
      {
        char Record[1024];
        if (*LastStr[CurCmd].Title)
          sprintf(Record,"%s: %s",LastStr[CurCmd].Title,LastStr[CurCmd].Name);
        else
          strcpy(Record,LastStr[CurCmd].Name);
        TruncStr(Record,Min(ScrX-12,(int)sizeof(HistoryItem.Name)-1));
        strcpy(HistoryItem.Name,Record);
        HistoryItem.SetSelect(CurCmd==CurLastPtr);
        LineToStr[Line++]=CurCmd;
        HistoryMenu.AddItem(&HistoryItem);
      }
    }

    sprintf(HistoryItem.Name,"%20s","");
    HistoryItem.SetSelect(CurLastPtr==LastPtr);
    LineToStr[Line]=-1;
    HistoryMenu.AddItem(&HistoryItem);
    HistoryMenu.AssignHighlights(TRUE);
    HistoryMenu.Show();

    while (!HistoryMenu.Done())
    {
      int Key=HistoryMenu.ReadInput();
      StrPos=HistoryMenu.GetSelectPos();
      if (Key==KEY_CTRLENTER || Key==KEY_SHIFTENTER)
      {
        HistoryMenu.Modal::SetExitCode(StrPos);
        RetCode=(Key==KEY_SHIFTENTER ? 2 : 3);
        continue;
      }
      switch(Key)
      {
        /* $ 09.04.2001 SVS
           Фича - копирование из истории строки в Clipboard
        */
        case KEY_CTRLC:
        case KEY_CTRLINS:
        {
          if((Code=LineToStr[StrPos]) != -1)
            CopyToClipboard(LastStr[Code].Name);
          break;
        }
        /* SVS $ */
        case KEY_DEL:
        /* $ 23.07.2001 VVM
          + Спросить подтверждение перед удалением */
        {
          if(HistoryMenu.GetItemCount() > 1 &&
             (!Opt.Confirm.HistoryClear ||
              (Opt.Confirm.HistoryClear &&
              Message(MSG_WARNING,2,
                   MSG((History::TypeHistory==HISTORYTYPE_CMD?MHistoryTitle:
                        (History::TypeHistory==HISTORYTYPE_FOLDER?MFolderHistoryTitle:
                        MViewHistoryTitle))),
                   MSG(MHistoryClear),
                   MSG(MClear),MSG(MCancel))==0)))
          {
            FreeHistory();
            DeleteRegValue(RegKey,"Lines");
            DeleteRegValue(RegKey,"Titles");
            DeleteRegValue(RegKey,"Types");
            HistoryMenu.Hide();
            return(Select(Title,HelpTopic,Str,Type));
          } /* if */
          break;
        }
        /* VVM $ */
        default:
          HistoryMenu.ProcessInput();
          break;
      }
    }
    if ((Code=HistoryMenu.Modal::GetExitCode())<0)
      return(0);
  }

  if (LineToStr[Code]==-1)
  {
    CurLastPtr0=LastPtr0=CurLastPtr=LastPtr;
    return(0);
  }

  StrPos=LineToStr[Code];
  if (KeepSelectedPos)
    CurLastPtr0=CurLastPtr=StrPos;

  *Str=0;
  if(LastStr[StrPos].Name)
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
    unsigned int NewPtr=(CurLastPtr-1)%(sizeof(LastStr)/sizeof(LastStr[0]));
    if (NewPtr!=LastPtr)
      CurLastPtr=NewPtr;
    else
      break;
  } while (LastStr[CurLastPtr].Name && *LastStr[CurLastPtr].Name==0);

  if(LastStr[CurLastPtr].Name)
    strcpy(Str,LastStr[CurLastPtr].Name);
  else
    *Str=0;
}


void History::GetNext(char *Str)
{
  do
  {
    if (CurLastPtr!=LastPtr)
      CurLastPtr=(CurLastPtr+1)%(sizeof(LastStr)/sizeof(LastStr[0]));
    else
      break;
  } while (LastStr[CurLastPtr].Name && *LastStr[CurLastPtr].Name==0);
  if(LastStr[CurLastPtr].Name)
    strcpy(Str,CurLastPtr==LastPtr ? "":LastStr[CurLastPtr].Name);
  else
    *Str=0;
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
    if (Name && *Name && LocalStrnicmp(Str,Name,Length)==0 && strcmp(Str,Name)!=0)
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
