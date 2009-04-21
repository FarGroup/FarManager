/*
history.cpp

История (Alt-F8, Alt-F11, Alt-F12)

*/

#include "headers.hpp"
#pragma hdrstop

#include "history.hpp"
#include "fn.hpp"
#include "global.hpp"
#include "keys.hpp"
#include "vmenu.hpp"
#include "lang.hpp"

History::History(int TypeHistory,int HistoryCount,const char *RegKey,const int *EnableSave,bool SaveType)
{
  xstrncpy(History::RegKey,RegKey,sizeof(History::RegKey)-1);

  History::SaveType=SaveType;
  History::EnableSave=EnableSave;
  History::TypeHistory=TypeHistory;
  History::HistoryCount=HistoryCount;
  EnableAdd=true;
  RemoveDups=1;
  KeepSelectedPos=false;
}

History::~History()
{
}

/*
   SaveForbid - принудительно запретить запись добавляемой строки.
                Используется на панели плагина
*/
void History::AddToHistory(const char *Str,int Type,const char *Prefix,bool SaveForbid)
{
  if (!EnableAdd)
    return;

  AddToHistoryLocal(Str,Prefix,Type);

  if (*EnableSave && !SaveForbid)
    SaveHistory();
}


void History::AddToHistoryLocal(const char *Str,const char *Prefix,int Type)
{
  if(!Str || *Str == 0)
    return;

  struct HistoryRecord AddRecord;

  size_t LenName=strlen(Str)+8;
  if(TypeHistory == HISTORYTYPE_FOLDER && Prefix && *Prefix)
    LenName+=strlen(Prefix);

  if(!(AddRecord.Name=(char *)xf_malloc(LenName)))
    return;

  if(TypeHistory == HISTORYTYPE_FOLDER && Prefix && *Prefix)
  {
    strcpy(AddRecord.Name,Prefix);
    strcat(AddRecord.Name,":");
  }
  else
    AddRecord.Name[0]=0;

  strcat(AddRecord.Name,Str);
  RemoveTrailingSpaces(AddRecord.Name);
  AddRecord.Type=Type;

  if (RemoveDups) // удалять дубликаты?
  {
    for (const HistoryRecord *HistoryItem=toBegin(); HistoryItem != NULL; HistoryItem=toNext())
    {
      if (EqualType(AddRecord.Type,HistoryItem->Type))
      {
        AddRecord.Lock=HistoryItem->Lock;
        if ((RemoveDups==1 && strcmp(AddRecord.Name,HistoryItem->Name)==0) ||
            (RemoveDups==2 && LocalStricmp(AddRecord.Name,HistoryItem->Name)==0))
        {
          erase();
          break;
        }
      }
    }
  }

  if (size()==(DWORD)HistoryCount)
  {
    toBegin();
    erase();
  }

  push_back(AddRecord);

  ResetPosition();
}

bool History::SaveHistory()
{
  if (!*EnableSave)
    return TRUE;

  if (!size())
  {
    DeleteRegKey(RegKey);
    return true;
  }

  unsigned char *TypesBuffer=NULL;
  if (SaveType)
  {
    if(!(TypesBuffer=(unsigned char *)xf_malloc((size()+1)*sizeof(char))))
      return false;
    memset(TypesBuffer,0,size()+1);
  }

  unsigned char *LocksBuffer=NULL;
  if(!(LocksBuffer=(unsigned char *)xf_malloc((size()+1)*sizeof(char))))
  {
    if (TypesBuffer)
      xf_free(TypesBuffer);
    return false;
  }
  memset(LocksBuffer,0,size()+1);

  bool ret = false;
  HKEY hKey = NULL;

  char *BufferLines=NULL,*PtrBuffer;
  size_t SizeLines=0, SizeTypes=0, SizeLocks=0;

  storePosition();

  const HistoryRecord *SelectedItem = getItem();
  int Position = -1, i=size()-1;

  for (const HistoryRecord *HistoryItem=toEnd(); HistoryItem != NULL; HistoryItem=toPrev())
  {
    int Len=(int)strlen(HistoryItem->Name);

    if ((PtrBuffer=(char*)xf_realloc(BufferLines,(SizeLines+Len+2)*sizeof(char))) == NULL)
    {
      ret = false;
      goto end;
    }

    BufferLines=PtrBuffer;
    xstrncpy(BufferLines+SizeLines,HistoryItem->Name,Len);
    SizeLines+=Len+1;

    if (SaveType)
      TypesBuffer[SizeTypes++]=HistoryItem->Type+'0';

    LocksBuffer[SizeLocks++]=HistoryItem->Lock+'0';

    if (HistoryItem == SelectedItem)
      Position = i;

    i--;
  }

  bool isDeleteRegKey=false;
  if(BufferLines && *BufferLines)
  {
    if((hKey=CreateRegKey(RegKey)) != NULL)
    {
       //BufferLines[SizeLines++]=0;
       RegSetValueEx(hKey,"Lines",0,REG_MULTI_SZ,(unsigned char *)BufferLines,(DWORD)SizeLines); //REG_BINARY
       if (SaveType)
       {
         if(TypesBuffer && *TypesBuffer)
           RegSetValueEx(hKey,"Types",0,REG_SZ,TypesBuffer,(DWORD)SizeTypes);
         else
           RegDeleteValue(hKey,"Types");
       }

       RegSetValueEx(hKey,"Locks",0,REG_SZ,LocksBuffer,(DWORD)SizeLocks);

       RegSetValueEx(hKey,"Position",0,REG_DWORD,(BYTE *)&Position,sizeof(Position));
       RegCloseKey(hKey);
       ret = true;
    }
    else
      isDeleteRegKey=true;
  }
  else
    isDeleteRegKey=true;

end:

  restorePosition();

  if(isDeleteRegKey)
    DeleteRegKey(RegKey);

  if (BufferLines)
    xf_free(BufferLines);
  if (TypesBuffer)
    xf_free(TypesBuffer);
  if (LocksBuffer)
    xf_free(LocksBuffer);

  return ret;
}


bool History::ReadHistory()
{
  bool NeedReadType = SaveType && CheckRegValue(RegKey, "Types");
  bool NeedReadLock = CheckRegValue(RegKey, "Locks")?true:false;

  HKEY hKey;
  if ((hKey=OpenRegKey(RegKey))==NULL)
    return false;

  DWORD SizeLines=GetRegKeySize(hKey,"Lines");

  if(!SizeLines) // Нету ничерта
    return true;

  bool ret = false;
  char *Buffer=NULL;
  DWORD Type;
  DWORD Size;

  int Position=-1;
  Size=sizeof(Position);
  RegQueryValueEx(hKey,"Position",0,&Type,(BYTE *)&Position,&Size);

  char *TypesBuffer=NULL;
  char *LocksBuffer=NULL;

  if (NeedReadType)
  {
    Size=GetRegKeySize(hKey, "Types");
    Size=Max(Size,(DWORD)((HistoryCount+2)*sizeof(char)));
    TypesBuffer=(char *)xf_malloc(Size);
    if (TypesBuffer)
    {
      memset(TypesBuffer,0,Size);
      if (RegQueryValueEx(hKey,"Types",0,&Type,(BYTE *)TypesBuffer,&Size)!=ERROR_SUCCESS)
        goto end;
    }
    else
      goto end;
  }

  if (NeedReadLock)
  {
    Size=GetRegKeySize(hKey, "Locks");
    Size=Max(Size,(DWORD)((HistoryCount+2)*sizeof(char)));
    LocksBuffer=(char *)xf_malloc(Size);
    if (LocksBuffer)
    {
      memset(LocksBuffer,0,Size);
      if (RegQueryValueEx(hKey,"Locks",0,&Type,(BYTE *)LocksBuffer,&Size)!=ERROR_SUCCESS)
        goto end;
    }
    else
      goto end;
  }

  if((Buffer=(char*)xf_malloc(SizeLines)) == NULL)
  {
    goto end;
  }

  if (RegQueryValueEx(hKey,"Lines",0,&Type,(unsigned char *)Buffer,&SizeLines)==ERROR_SUCCESS)
  {
    bool bPosFound = false;
    char *TypesBuf=TypesBuffer;
    char *LockBuf=LocksBuffer;

    int StrPos=0;
    char *Buf=Buffer;
    while (SizeLines > 1 && StrPos < HistoryCount)
    {
      size_t Length=strlen(Buf)+1;
      HistoryRecord AddRecord;
      AddRecord.Name=xf_strdup(Buf);
      Buf+=Length;
      SizeLines-=(DWORD)Length;

      if (NeedReadType)
      {
        if (isdigit(*TypesBuf))
        {
          AddRecord.Type = *TypesBuf-'0';
          TypesBuf++;
        }
      }

      if (NeedReadLock)
      {
        if (isdigit(*LockBuf))
        {
          AddRecord.Lock = (*LockBuf-'0') == 0?false:true;
          LockBuf++;
        }
      }

      if (strlen(AddRecord.Name))
      {
        push_front(AddRecord);
        if (StrPos == Position)
        {
          storePosition();
          bPosFound = true;
        }
      }

      StrPos++;
    }
    if (bPosFound)
    {
      restorePosition();
    }
    else
    {
      ResetPosition();
    }
  }
  else
    goto end;

  ret=true;

end:
  RegCloseKey(hKey);
  if (TypesBuffer)
    xf_free(TypesBuffer);
  if (Buffer)
    xf_free(Buffer);
  if (LocksBuffer)
    xf_free(LocksBuffer);

  //if (!ret)
    //clear();

  return ret;
}

const char *History::GetTitle(int Type)
{
  switch (Type)
  {
    case 0: // вьювер
      return MSG(MHistoryView);
    case 1: // обычное открытие в редакторе
    case 4: // открытие с локом
      return MSG(MHistoryEdit);
    case 2: // external - без ожидания
    case 3: // external - AlwaysWaitFinish
      return MSG(MHistoryExt);
  }
  return "";
}


/*
 Return:
   0 - Esc
   1 - Enter
   2 - Shift-Enter
   3 - Ctrl-Enter
   4 - F3
   5 - F4
   6 - Ctrl-Shift-Enter
*/

int History::Select(const char *Title,const char *HelpTopic,char *Str,int StrLength,int &Type)
{
  struct MenuItem MenuItem;

  OneItem *SelectedRecord=NULL;
  int Code=-1,Height=ScrY-8;
  FarListPos Pos={0,0};
  int RetCode=1;

  {
    VMenu HistoryMenu(Title,NULL,0,Height);
    HistoryMenu.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);
    if (HelpTopic!=NULL)
      HistoryMenu.SetHelp(HelpTopic);
    HistoryMenu.SetPosition(-1,-1,0,0);
    HistoryMenu.AssignHighlights(TRUE);
    int Done=FALSE;
    int SetUpMenuPos=FALSE;

    while(!Done)
    {
      bool IsUpdate=false;
      HistoryMenu.DeleteItems();
      HistoryMenu.Modal::ClearDone();
      HistoryMenu.SetPosition(-1,-1,0,0);

      const HistoryRecord *HistoryCurrentItem = getItem();
      storePosition();
      // заполнение пунктов меню
      for (const HistoryRecord *HistoryItem=toBegin(); HistoryItem != NULL; HistoryItem=toNext())
      {
        char Record[2048];

        #if defined(__BORLANDC__)
        #define _snprintf FarSnprintf
        #endif
        Record[0]=0;
        if (TypeHistory == HISTORYTYPE_VIEW)
          _snprintf(Record,sizeof(Record)-1,"%s:%c",GetTitle(HistoryItem->Type),(HistoryItem->Type==4?'-':' '));
        strncat(Record,HistoryItem->Name,sizeof(Record)-1);

        ReplaceStrings(Record,"&","&&",-1);
        memset(&MenuItem,0,sizeof(MenuItem));
        MenuItem.Flags|=LIF_USETEXTPTR;
        MenuItem.NamePtr=xf_strdup(Record);
        MenuItem.SetCheck(HistoryItem->Lock?1:0);

        if (!SetUpMenuPos)
          MenuItem.SetSelect(HistoryCurrentItem==HistoryItem || (!HistoryCurrentItem && isEnd()));

        HistoryMenu.SetUserData((void*)Current,sizeof(OneItem *),HistoryMenu.AddItem(&MenuItem));

      }
      restorePosition();


      if (SetUpMenuPos)
      {
        Pos.SelectPos=Pos.SelectPos < (int)size() ? Pos.SelectPos : (int)size()-1;
        Pos.TopPos=Min(Pos.TopPos,HistoryMenu.GetItemCount()-Height);
        HistoryMenu.SetSelectPos(&Pos);
        SetUpMenuPos=false;
      }

      HistoryMenu.Show();
      while (!HistoryMenu.Done())
      {
        int Key=HistoryMenu.ReadInput();
        HistoryMenu.GetSelectPos(&Pos);

        switch(Key)
        {
          case KEY_CTRLR: // обновить с удалением недоступных
          {
            if (TypeHistory == HISTORYTYPE_FOLDER || TypeHistory == HISTORYTYPE_VIEW)
            {
              bool ModifiedHistory=false;
              for (const HistoryRecord *HistoryItem=toBegin(); HistoryItem != NULL;)
              {
                if(HistoryItem->Lock) // залоченные не трогаем
                {
                  HistoryItem=toNext();
                  continue;
                }

                // убить запись из истории
                if (GetFileAttributes(HistoryItem->Name) == INVALID_FILE_ATTRIBUTES)
                {
                  erase();
                  HistoryItem = getItem();
                  ModifiedHistory=true;
                }
                else
                {
                  HistoryItem=toNext();
                }
              }

              if (ModifiedHistory) // избавляемся от лишних телодвижений
              {
                SaveHistory(); // сохранить
                HistoryMenu.Modal::SetExitCode(Pos.SelectPos);
                HistoryMenu.SetUpdateRequired(TRUE);
                IsUpdate=true;
              }
              ResetPosition();
            }
            break;
          }

          case KEY_CTRLSHIFTNUMENTER:
          case KEY_CTRLNUMENTER:
          case KEY_SHIFTNUMENTER:
          case KEY_CTRLSHIFTENTER:
          case KEY_CTRLENTER:
          case KEY_SHIFTENTER:
          {
            HistoryMenu.Modal::SetExitCode(Pos.SelectPos);
            Done=true;
            RetCode=Key==KEY_CTRLSHIFTENTER||Key==KEY_CTRLSHIFTNUMENTER?6:(Key==KEY_SHIFTENTER||Key==KEY_SHIFTNUMENTER?2:3);
            break;
          }

          case KEY_F3:
          case KEY_F4:
          case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:
          {
            HistoryMenu.Modal::SetExitCode(Pos.SelectPos);
            Done=true;
            RetCode=(Key==KEY_F4? 5 : 4);
            break;
          }

          // $ 09.04.2001 SVS - Фича - копирование из истории строки в Clipboard
          case KEY_CTRLC:
          case KEY_CTRLINS:  case KEY_CTRLNUMPAD0:
          {
            OneItem *Record=(OneItem *)HistoryMenu.GetUserData(NULL,sizeof(OneItem *),Pos.SelectPos);

            if (Record)
              CopyToClipboard(Record->Item.Name);

            break;
          }

          // Lock/Unlock
          case KEY_INS:
          case KEY_NUMPAD0:
          {
            if (HistoryMenu.GetItemCount()/* > 1*/)
            {
              Current=(OneItem *)HistoryMenu.GetUserData(NULL,sizeof(OneItem *),Pos.SelectPos);
              Current->Item.Lock=Current->Item.Lock?false:true;
              HistoryMenu.Hide();
              ResetPosition();
              SaveHistory();
              HistoryMenu.Modal::SetExitCode(Pos.SelectPos);
              HistoryMenu.SetUpdateRequired(TRUE);
              IsUpdate=true;
              SetUpMenuPos=true;
            }
            break;
          }

          case KEY_SHIFTNUMDEL:
          case KEY_SHIFTDEL:
          {
            if (HistoryMenu.GetItemCount()/* > 1*/)
            {
              Current=(OneItem *)HistoryMenu.GetUserData(NULL,sizeof(OneItem *),Pos.SelectPos);
              if(!Current->Item.Lock)
              {
                HistoryMenu.Hide();
                erase();
                ResetPosition();
                SaveHistory();
                HistoryMenu.Modal::SetExitCode(Pos.SelectPos);
                HistoryMenu.SetUpdateRequired(TRUE);
                IsUpdate=true;
                SetUpMenuPos=true;
              }
            }
            break;
          }

          case KEY_NUMDEL:
          case KEY_DEL:
          {
            if (HistoryMenu.GetItemCount()/* > 1*/ &&
                (!Opt.Confirm.HistoryClear ||
                (Opt.Confirm.HistoryClear &&
                Message(MSG_WARNING,2,
                    MSG((History::TypeHistory==HISTORYTYPE_CMD?MHistoryTitle:
                          (History::TypeHistory==HISTORYTYPE_FOLDER?MFolderHistoryTitle:
                          MViewHistoryTitle))),
                    MSG(MHistoryClear),
                    MSG(MClear),MSG(MCancel))==0)))
            {
              bool FoundLock=false;
              for (const HistoryRecord *HistoryItem=toBegin(); HistoryItem != NULL; HistoryItem=toNext())
              {
                if(HistoryItem->Lock) // залоченные не трогаем
                {
                  FoundLock=true;
                  ResetPosition();
                  break;
                }
              }
              HistoryMenu.Hide();
              if(!FoundLock)
                clear();
              else
              {
                for (toBegin(); Current; )
                  if(!Current->Item.Lock) // залоченные не трогаем
                    erase();
                  else
                    toNext();
              }
              ResetPosition();
              SaveHistory();
              HistoryMenu.Modal::SetExitCode(Pos.SelectPos);
              HistoryMenu.SetUpdateRequired(TRUE);
              IsUpdate=true;
            }
            break;
          }

          default:
            HistoryMenu.ProcessInput();
            break;

        }

      }

      if (IsUpdate)
        continue;

      Done=true;
      Code=HistoryMenu.Modal::GetExitCode();

      if (Code >= 0)
      {
        SelectedRecord=(OneItem *)HistoryMenu.GetUserData(NULL,sizeof(OneItem *),Code);

        if (!SelectedRecord)
          return -1;

        if (RetCode != 3 && ((TypeHistory == HISTORYTYPE_FOLDER && !SelectedRecord->Item.Type) || TypeHistory == HISTORYTYPE_VIEW) && GetFileAttributes(SelectedRecord->Item.Name) == INVALID_FILE_ATTRIBUTES)
        {
          SetLastError(ERROR_FILE_NOT_FOUND);

          if (SelectedRecord->Item.Type == 1 && TypeHistory == HISTORYTYPE_VIEW) // Edit? тогда спросим и если надо создадим
          {
            if (Message(MSG_WARNING|MSG_ERRORTYPE,2,Title,SelectedRecord->Item.Name,MSG(MViewHistoryIsCreate),MSG(MHYes),MSG(MHNo)) == 0)
              break;
          }
          else
          {
            Message(MSG_WARNING|MSG_ERRORTYPE,1,Title,SelectedRecord->Item.Name,MSG(MOk));
          }

          Done=false;
          SetUpMenuPos=true;
          HistoryMenu.Modal::SetExitCode(Pos.SelectPos=Code);
          continue;
        }
      }

    }
  }

  if (Code < 0 || !SelectedRecord)
    return 0;

  if (KeepSelectedPos)
  {
    Current = SelectedRecord;
  }

  if(Str)
    xstrncpy(Str,SelectedRecord->Item.Name,StrLength-1);

  if (RetCode < 4 || RetCode == 6)
  {
    Type=SelectedRecord->Item.Type;
  }
  else
  {
    Type=RetCode-4;
    if (Type == 1 && SelectedRecord->Item.Type == 4)
      Type=4;
    RetCode=1;
  }

  return RetCode;
}


void History::GetPrev(char *Str,int StrLength)
{
  if (!Current)
  {
    toEnd();
  }
  else if (!toPrev())
  {
    toBegin();
  }

  const HistoryRecord *Record = getItem();

  if (Record)
    xstrncpy(Str,Record->Name,StrLength-1);
  else
    *Str=0;
}


void History::GetNext(char *Str,int StrLength)
{
  const HistoryRecord *Record = toNext();

  if (Record)
    xstrncpy(Str,Record->Name,StrLength-1);
  else
    *Str=0;
}


void History::GetSimilar(char *Str,int StrLength,int LastCmdPartLength)
{
  int Length=(int)strlen(Str);

  if (LastCmdPartLength!=-1 && LastCmdPartLength<Length)
    Length=LastCmdPartLength;

  storePosition();

  if (LastCmdPartLength==-1)
    ResetPosition();

  char Tmp[2048];

  while (!isBegin())
  {
    GetPrev(Tmp,sizeof(Tmp));
    if (LocalStrnicmp(Str,Tmp,Length)==0 && strcmp(Str,Tmp)!=0)
    {
      xstrncpy(Str,Tmp,StrLength-1);
      return;
    }
  }

  restorePosition();

  const HistoryRecord *StopRecord = getItem();

  ResetPosition();

  if (StopRecord)
  {
    while (StopRecord != getItem())
    {
      GetPrev(Tmp,sizeof(Tmp));
      if (LocalStrnicmp(Str,Tmp,Length)==0 && strcmp(Str,Tmp)!=0)
      {
        xstrncpy(Str,Tmp,StrLength-1);
        return;
      }
    }
  }

  restorePosition();
}


void History::SetAddMode(bool EnableAdd, int RemoveDups, bool KeepSelectedPos)
{
  History::EnableAdd=EnableAdd;
  History::RemoveDups=RemoveDups;
  History::KeepSelectedPos=KeepSelectedPos;
}

bool History::EqualType(int Type1, int Type2)
{
  return Type1 == Type2 || (TypeHistory == HISTORYTYPE_VIEW && (Type1 == 4 && Type2 == 1 || (Type1 == 1 && Type2 == 4)))?true:false;
}

void History::ResetPosition()
{
  this->Current = NULL;
}


const HistoryRecord& HistoryRecord::operator=(const HistoryRecord &rhs)
{
  Name = xf_strdup(rhs.Name);
  Type = rhs.Type;
  Lock = rhs.Lock;
  return *this;
}
