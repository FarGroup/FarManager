/*
history.cpp

История (Alt-F8, Alt-F11, Alt-F12)

*/

/* Revision: 1.24 18.04.2002 $ */

/*
Modify:
  18.04.2002 SVS
    - BugZ#463 - Ext.: -> Edit: (а про "Ext." то я и забыл :-()
  18.03.2002 SVS
    + ReloadTitle() и EqualType()
    ! Titles для истории редактирования/просмотра не сохраняем,
      соответственно и не проверяем Title при поиске дубликатов,
      только по файлу и типу.
    + F3/Numpad 5 и F4
    ! Переделан Select - зачем держать массив, если меню само умеет кое-что :-)
  06.03.2002 SVS
    ! Косметика имени параметра у FreeHistory() - рука дрогнула :-)
    - Жучара с strcpy()
    ! Устранение утечки памяти при добавлении и удалении
    ! У функций Истории появились доп.параметры
    - ну и, наконец, устранен полный бардак с самим механизмом.
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

void History::FreeHistory(BOOL FreeMemory)
{
  if(FreeMemory)
    for (int I=0; I < HISTORY_COUNT;I++)
      if(LastStr[I].Name)
        free(LastStr[I].Name);
  memset(LastStr,0,sizeof(LastStr));
  CurLastPtr=LastPtr=CurLastPtr0=LastPtr0=0;
  LastSimilar=0;
}

void History::ReloadTitle()
{
  if(TypeHistory != HISTORYTYPE_VIEW)
    return;

  int I;
  struct HistoryRecord *PtrLastStr;

  for (PtrLastStr=LastStr,I=0; I < HISTORY_COUNT; I++, PtrLastStr++)
  {
    if(PtrLastStr->Name && *PtrLastStr->Name)
      switch(PtrLastStr->Type)
      {
        case 0:
          strncpy(PtrLastStr->Title,MSG(MHistoryView),HISTORY_TITLESIZE-1);
          break;
        case 1:
        case 4:
          strncpy(PtrLastStr->Title,MSG(MHistoryEdit),HISTORY_TITLESIZE-1);
          break;
        case 2:
        case 3:
          strncpy(PtrLastStr->Title,MSG(MHistoryExt),HISTORY_TITLESIZE-1);
          break;
      }
  }
}


/*
   SaveForbid - принудительно запретить запись добавляемой строки.
                Используется на панели плагина
*/
void History::AddToHistory(char *Str,char *Title,int Type,int SaveForbid)
{
  if (!EnableAdd)
    return;

  if (*EnableSave && !SaveForbid)
  {
    // запоминаем!
    unsigned int SaveLastPtr=LastPtr,
                 SaveCurLastPtr=CurLastPtr,
                 SaveLastSimilar=LastSimilar;

    struct HistoryRecord SaveLastStr[HISTORY_COUNT];
    memcpy(SaveLastStr,LastStr,sizeof(SaveLastStr));
    for (int I=0;I < HISTORY_COUNT; I++)
      if(LastStr[I].Name && LastStr[I].Name[0])
        SaveLastStr[I].Name=strdup(LastStr[I].Name);
      else
        SaveLastStr[I].Name=NULL;

    // т.к. мы все запомнили, то, перед прочтением освободим память
    FreeHistory(TRUE);

    ReadHistory();
    LastPtr0=LastPtr=SaveLastPtr;
    CurLastPtr0=CurLastPtr=SaveCurLastPtr;
    LastSimilar=SaveLastSimilar;
    AddToHistoryLocal(Str,Title,Type);
    SaveHistory();
    FreeHistory(TRUE); // Необходимо, т.к. ReadHistory берет память!

    // восстановим
    LastPtr0=LastPtr=SaveLastPtr;
    CurLastPtr0=CurLastPtr=SaveCurLastPtr;
    LastSimilar=SaveLastSimilar;
    memcpy(LastStr,SaveLastStr,sizeof(LastStr));
  }
  AddToHistoryLocal(Str,Title,Type);
}


void History::AddToHistoryLocal(char *Str,char *Title,int Type)
{
  if(!Str || *Str == 0)
    return;

  struct HistoryRecord AddRecord;

  AddRecord.Name=strdup(Str);
  if(!AddRecord.Name)
    return;

  RemoveTrailingSpaces(AddRecord.Name);
  strncpy(AddRecord.Title,NullToEmpty(Title),sizeof(AddRecord.Title)-1);
  RemoveTrailingSpaces(AddRecord.Title);
  AddRecord.Type=Type;

  int OldLastPtr=LastPtr-1;
  if (OldLastPtr < 0)
    OldLastPtr=HISTORY_COUNT-1;

  if (RemoveDups)
  {
    struct HistoryRecord *PtrLastStr;
    int I, J;
    for (PtrLastStr=LastStr,I=0; I < HISTORY_COUNT; I++, PtrLastStr++)
    {
      if(PtrLastStr->Name && EqualType(AddRecord.Type,PtrLastStr->Type))
      {
        int Equal;
        if(TypeHistory == HISTORYTYPE_VIEW) // только по файлу и типу
        {
           Equal=RemoveDups==1 && strcmp(AddRecord.Name,PtrLastStr->Name)==0 ||
                 RemoveDups==2 && LocalStricmp(AddRecord.Name,PtrLastStr->Name)==0;
        }
        else
        {
           Equal=RemoveDups==1 &&
                   strcmp(AddRecord.Name,PtrLastStr->Name)==0 &&
                   strcmp(AddRecord.Title,PtrLastStr->Title)==0 ||
                 RemoveDups==2 &&
                   LocalStricmp(AddRecord.Name,PtrLastStr->Name)==0 &&
                   LocalStricmp(AddRecord.Title,PtrLastStr->Title)==0;
        }

        if (Equal)
        {
          int Length=OldLastPtr-I;

          if (Length<0)
            Length+=HISTORY_COUNT;

          for (J=0; J <= Length; J++)
          {
            int Dest=(I+J) % (HISTORY_COUNT);
            int Src=(I+J+1) % (HISTORY_COUNT);

            if(LastStr[Dest].Name)
            {
              free(LastStr[Dest].Name);
              LastStr[Dest].Name=NULL;
            }
            memmove(LastStr+Dest,LastStr+Src,sizeof(HistoryRecord));
            memset(LastStr+Src,0,sizeof(HistoryRecord));
          }

          memcpy(LastStr+OldLastPtr, &AddRecord, sizeof(HistoryRecord));

          CurLastPtr0=LastPtr0=CurLastPtr=LastPtr;
          return;
        }
      }
    }
  }

  int Pos=(LastPtr-1) % (HISTORY_COUNT);

  if(LastStr[Pos].Name && LastStr[LastPtr].Name &&
      (strcmp(AddRecord.Name,LastStr[Pos].Name) != 0 ||
        strcmp(AddRecord.Title,LastStr[Pos].Title) != 0 ||
        !EqualType(AddRecord.Type,LastStr[Pos].Type)))
    free(LastStr[LastPtr].Name);

  memcpy(LastStr+LastPtr,&AddRecord,sizeof(HistoryRecord));

  if (++LastPtr==HISTORY_COUNT)
     LastPtr=0;

  CurLastPtr0=LastPtr0=CurLastPtr=LastPtr;
}

/*
  Вначале разберемся с память, а потом... "все или ничего"
*/
BOOL History::SaveHistory()
{
  if (!*EnableSave)
    return TRUE;

  char *BufferLines=NULL,*BufferTitles=NULL,*PtrBuffer;
  unsigned char TypesBuffer[HISTORY_COUNT+1];
  DWORD SizeLines=0, SizeTitles=0, SizeTypes=0;
  int I, Len;

  for (I=0; I < HISTORY_COUNT; I++)
  {
    if(LastStr[I].Name)
    {
      Len=strlen(LastStr[I].Name);
      if(WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && Len > 511)
        Len=511;

      if((PtrBuffer=(char*)realloc(BufferLines,SizeLines+Len+2)) == NULL)
      {
        free(BufferLines);
        return FALSE;
      }
      BufferLines=PtrBuffer;
      strncpy(BufferLines+SizeLines,LastStr[I].Name,Len);
      SizeLines+=Len+1;
    }
  }

  if(!BufferLines)
    return FALSE;

  BufferLines[SizeLines++]=0;

  if (SaveTitle && TypeHistory != HISTORYTYPE_VIEW)
  {
    BufferTitles=(char*)malloc(HISTORY_COUNT*(HISTORY_TITLESIZE+2));
    if(!BufferTitles)
    {
      free(BufferLines);
      return FALSE;
    }

    for (I=0; I < HISTORY_COUNT; I++)
    {
      strcpy(BufferTitles+SizeTitles,LastStr[I].Title);
      SizeTitles+=strlen(LastStr[I].Title)+1;
    }
    BufferTitles[SizeTitles++]=0;
  }

  if (SaveType)
  {
    memset(TypesBuffer,0,sizeof(TypesBuffer));
    for (SizeTypes=0; SizeTypes < HISTORY_COUNT; SizeTypes++)
      TypesBuffer[SizeTypes]=LastStr[SizeTypes].Type+'0';
    TypesBuffer[SizeTypes++]=0;
  }

  HKEY hKey;
  if ((hKey=CreateRegKey(RegKey))!=NULL)
  {
    RegSetValueEx(hKey,"Lines",0,REG_BINARY,(unsigned char *)BufferLines,SizeLines);

    if (SaveTitle && BufferTitles)
      RegSetValueEx(hKey,"Titles",0,REG_BINARY,(unsigned char *)BufferTitles,SizeTitles);

    if (SaveType)
      RegSetValueEx(hKey,"Types",0,REG_SZ,TypesBuffer,SizeTypes);

    RegSetValueEx(hKey,"Position",0,REG_DWORD,(BYTE *)&CurLastPtr,sizeof(CurLastPtr));
    RegCloseKey(hKey);

    if (SaveTitle && BufferTitles)
      free(BufferTitles);
    free(BufferLines);

    return TRUE;
  }
  return FALSE;
}


BOOL History::ReadHistory()
{
  int NeedReadTitle=SaveTitle && CheckRegValue(RegKey,"Titles");
  int NeedReadType =SaveType  && CheckRegValue(RegKey,"Types");

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

  int StrPos, Length;
  if (RegQueryValueEx(hKey,"Lines",0,&Type,(unsigned char *)Buffer,&Size)==ERROR_SUCCESS)
  {
    StrPos=0;
    Buf=Buffer;
    while ((int)Size > 1 && StrPos < HISTORY_COUNT)
    {
      Length=strlen(Buf)+1;
      if((LastStr[StrPos].Name=(char*)malloc(Length)) == NULL)
      {
        free(Buffer);
        FreeHistory(TRUE);
        RegCloseKey(hKey);
        return FALSE;
      }
      strcpy(LastStr[StrPos].Name,Buf);
      StrPos++;
      Buf+=Length;
      Size-=Length;
    }
  }
  else
  {
    free(Buffer);
    RegCloseKey(hKey);
    return FALSE;
  }

  if (NeedReadTitle)
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
      StrPos=0;
      while ((int)Size > 1 && StrPos < HISTORY_COUNT)
      {
        strncpy(LastStr[StrPos].Title,Buf,sizeof(LastStr[StrPos].Title)-1);
        ++StrPos;
        Length=strlen(Buf)+1;
        Buf+=Length;
        Size-=Length;
      }
    }
    else // раз требовали Title, но ничего не получили, значит _ВСЕ_ в морг.
    {
      free(Buffer);
      FreeHistory(TRUE);
      RegCloseKey(hKey);
      return FALSE;
    }
  }
  free(Buffer);

  if (NeedReadType)
  {
    unsigned char TypesBuffer[HISTORY_COUNT+1];
    Size=sizeof(TypesBuffer);
    memset(TypesBuffer,0,Size);
    if(RegQueryValueEx(hKey,"Types",0,&Type,(unsigned char *)TypesBuffer,&Size)==ERROR_SUCCESS)
    {
      StrPos=0;
      Buf=(char *)TypesBuffer;
      while (isdigit(*Buf) && StrPos < HISTORY_COUNT)
      {
        LastStr[StrPos++].Type=*Buf-'0';
        Buf++;
      }
    }
    else // раз требовали Type, но ничего не получили, значит _ВСЕ_ в морг.
    {
      FreeHistory(TRUE);
      RegCloseKey(hKey);
      return FALSE;
    }
  }

  Size=sizeof(CurLastPtr);
  RegQueryValueEx(hKey,"Position",0,&Type,(BYTE *)&CurLastPtr,&Size);
  RegCloseKey(hKey);

  LastPtr0=CurLastPtr0=LastPtr=CurLastPtr;

  if(TypeHistory == HISTORYTYPE_VIEW)
    ReloadTitle();

  return TRUE;
}


int History::Select(char *Title,char *HelpTopic,char *Str,int StrLength,int &Type,char *ItemTitle)
{
  struct MenuItem HistoryItem;

  int Code,I,Height=ScrY-8,StrPos,IsUpdate;
  unsigned int CurCmd;
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
    HistoryMenu.AssignHighlights(TRUE);
    int Done=FALSE;

    while(!Done)
    {
      IsUpdate=FALSE;
      HistoryMenu.DeleteItems();
      HistoryMenu.Modal::ClearDone();
      HistoryMenu.SetPosition(-1,-1,0,0);

      // заполнение пунктов меню
      for (CurCmd=LastPtr+1, I=0; I < HISTORY_COUNT-1; I++, CurCmd++)
      {
        CurCmd%=HISTORY_COUNT;

        if (LastStr[CurCmd].Name && *LastStr[CurCmd].Name)
        {
          int SizeTrunc=Min(ScrX-12,(int)sizeof(HistoryItem.Name)-1)-2;
          char Record[2048], *Ptr=Record;
          if (*LastStr[CurCmd].Title)
          {
            sprintf(Record,"%s:%c",LastStr[CurCmd].Title,(LastStr[CurCmd].Type==4?'-':' '));
            Ptr=Record+strlen(Record);
            strcat(Record,LastStr[CurCmd].Name);
          }
          else
            strcpy(Record,LastStr[CurCmd].Name);

          TruncPathStr(Ptr,SizeTrunc);
          memset(&HistoryItem,0,sizeof(HistoryItem));
          strncpy(HistoryItem.Name,Record,sizeof(HistoryItem.Name)-1);
          HistoryItem.SetSelect(CurCmd==CurLastPtr);
          HistoryMenu.SetUserData((void*)CurCmd,sizeof(DWORD),
                                 HistoryMenu.AddItem(&HistoryItem));
        }
      }

      memset(&HistoryItem,0,sizeof(HistoryItem));
      memset(HistoryItem.Name,' ',20);HistoryItem.Name[20]=0;
      HistoryItem.SetSelect(CurLastPtr==LastPtr);
      HistoryMenu.SetUserData((void*)-1,sizeof(DWORD),
           HistoryMenu.AddItem(&HistoryItem));

      HistoryMenu.Show();
      while (!HistoryMenu.Done())
      {
        int Key=HistoryMenu.ReadInput();
        StrPos=HistoryMenu.GetSelectPos();

        switch(Key)
        {
          case KEY_CTRLENTER:
          case KEY_SHIFTENTER:
          {
            HistoryMenu.Modal::SetExitCode(StrPos);
            Done=TRUE;
            RetCode=(Key==KEY_SHIFTENTER ? 2 : 3);
            break;
          }

          case KEY_F3:
          case KEY_F4:
          case KEY_NUMPAD5:
          {
            HistoryMenu.Modal::SetExitCode(StrPos);
            Done=TRUE;
            RetCode=(Key==KEY_F4? 5 : 4);
            break;
          }
          /* $ 09.04.2001 SVS
             Фича - копирование из истории строки в Clipboard
          */
          case KEY_CTRLC:
          case KEY_CTRLINS:
          {
            Code=(int)HistoryMenu.GetUserData(NULL,sizeof(DWORD),StrPos);
            if(Code != -1)
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
              HistoryMenu.Hide();
              FreeHistory(TRUE); // память тоже нужно очистить!
              HistoryMenu.Modal::SetExitCode(StrPos);
              HistoryMenu.SetUpdateRequired(TRUE);
              IsUpdate=TRUE; //??
            } /* if */
            break;
          }

          /* VVM $ */
          default:
            HistoryMenu.ProcessInput();
            break;
        }
      }
      if(IsUpdate)
        continue;

      Done=TRUE;
      Code=HistoryMenu.Modal::GetExitCode();
      if (Code<0)
        StrPos=-1;
      else
        StrPos=(int)HistoryMenu.GetUserData(NULL,sizeof(StrPos),Code);
    }
  }

  if(Code < 0)
    return 0;

  if (StrPos == -1)
  {
    CurLastPtr0=LastPtr0=CurLastPtr=LastPtr;
    return 0;
  }

  if (KeepSelectedPos)
    CurLastPtr0=CurLastPtr=StrPos;

  *Str=0;
  if(LastStr[StrPos].Name)
    strncpy(Str,LastStr[StrPos].Name,StrLength-1);

  if(RetCode < 4)
    Type=LastStr[StrPos].Type;
  else
  {
    Type=RetCode-4;
    if(Type == 1 && LastStr[StrPos].Type == 4) //????
      Type=4;                                  //????
    RetCode=1;
  }

  if (ItemTitle!=NULL)
    strcpy(ItemTitle,LastStr[StrPos].Title);

  return RetCode;
}


void History::GetPrev(char *Str,int StrLength)
{
  do
  {
    unsigned int NewPtr=(CurLastPtr-1)%(HISTORY_COUNT);
    if (NewPtr!=LastPtr)
      CurLastPtr=NewPtr;
    else
      break;
  } while (LastStr[CurLastPtr].Name && *LastStr[CurLastPtr].Name==0);

  if(LastStr[CurLastPtr].Name)
    strncpy(Str,LastStr[CurLastPtr].Name,StrLength-1);
  else
    *Str=0;
}


void History::GetNext(char *Str,int StrLength)
{
  do
  {
    if (CurLastPtr!=LastPtr)
      CurLastPtr=(CurLastPtr+1)%(HISTORY_COUNT);
    else
      break;
  } while (LastStr[CurLastPtr].Name && *LastStr[CurLastPtr].Name==0);
  if(LastStr[CurLastPtr].Name)
    strncpy(Str,CurLastPtr==LastPtr ? "":LastStr[CurLastPtr].Name,StrLength-1);
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
  for (int I=1;I<HISTORY_COUNT;I++)
  {
    int Pos=(LastPtr-LastSimilar-I)%(HISTORY_COUNT);
    char *Name=LastStr[Pos].Name;
    if (Name && *Name && LocalStrnicmp(Str,Name,Length)==0 && strcmp(Str,Name)!=0)
    {
      int NewSimilar=(LastPtr-Pos)%(HISTORY_COUNT);
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

BOOL History::EqualType(int Type1, int Type2)
{
  return Type1 == Type2 || (TypeHistory == HISTORYTYPE_VIEW && (Type1 == 4 && Type2 == 1 || (Type1 == 1 && Type2 == 4)))?TRUE:FALSE;
}
