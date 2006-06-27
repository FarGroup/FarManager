/*
history.cpp

������� (Alt-F8, Alt-F11, Alt-F12)

*/

/* Revision: 1.46 21.04.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "history.hpp"
#include "fn.hpp"
#include "global.hpp"
#include "keys.hpp"
#include "vmenu.hpp"
#include "lang.hpp"


HistoryW::HistoryW(int TypeHistory,int HistoryCount,const wchar_t *RegKey,const int *EnableSave,int SaveTitle,int SaveType)
{
  LastStr=NULL;
  FreeHistory();

  strRegKey = RegKey;

  HistoryW::SaveTitle=SaveTitle;
  HistoryW::SaveType=SaveType;
  HistoryW::EnableSave=EnableSave;
  HistoryW::TypeHistory=TypeHistory;
  HistoryW::HistoryCount=HistoryCount;
  LastStr=(struct HistoryRecordW*)xf_malloc(sizeof(struct HistoryRecordW) * HistoryCount);
  if(LastStr)
    memset(LastStr,0,sizeof(struct HistoryRecordW) * HistoryCount);
  EnableAdd=RemoveDups=TRUE;
  KeepSelectedPos=FALSE;
  ReturnSimilarTemplate=TRUE;
}

HistoryW::~HistoryW()
{
  FreeHistory();
  if(LastStr)
    xf_free(LastStr);
}

void HistoryW::FreeHistory()
{
  if(LastStr)
  {
    for (int I=0; I < HistoryCount;I++)
      if(LastStr[I].Name)
        xf_free(LastStr[I].Name);
    memset(LastStr,0,sizeof(struct HistoryRecordW) * HistoryCount);
  }
  CurLastPtr=LastPtr=CurLastPtr0=LastPtr0=0;
  LastSimilar=0;
}

void HistoryW::ReloadTitle()
{
  if(!LastStr)
    return;

  if(TypeHistory != HISTORYTYPE_VIEW)
    return;

  int I;
  struct HistoryRecordW *PtrLastStr;

  for (PtrLastStr=LastStr,I=0; I < HistoryCount; I++, PtrLastStr++)
  {
    if(PtrLastStr->Name && *PtrLastStr->Name)
      switch(PtrLastStr->Type)
      {
        case 0: // ������
          xwcsncpy(PtrLastStr->Title,UMSG(MHistoryView),HISTORY_TITLESIZE-1);
          break;
        case 1: // ������� �������� � ���������
        case 4: // �������� � �����
          xwcsncpy(PtrLastStr->Title,UMSG(MHistoryEdit),HISTORY_TITLESIZE-1);
          break;
        case 2: // external - ��� ��������
        case 3: // external - AlwaysWaitFinish
          xwcsncpy(PtrLastStr->Title,UMSG(MHistoryExt),HISTORY_TITLESIZE-1);
          break;
      }
  }
}


/*
   SaveForbid - ������������� ��������� ������ ����������� ������.
                ������������ �� ������ �������
*/
void HistoryW::AddToHistory(const wchar_t *Str,const wchar_t *Title,int Type,int SaveForbid)
{
  if(!LastStr)
    return;

  if (!EnableAdd)
    return;

  if (*EnableSave && !SaveForbid)
  {
    // ����������!
    unsigned int SaveLastPtr=LastPtr,
                 SaveCurLastPtr=CurLastPtr,
                 SaveLastSimilar=LastSimilar;

    struct HistoryRecordW *SaveLastStr;

    SaveLastStr=(struct HistoryRecordW *)alloca(HistoryCount*sizeof(struct HistoryRecordW));
    if(!SaveLastStr)
      return;

    memcpy(SaveLastStr,LastStr,HistoryCount*sizeof(struct HistoryRecordW));
    for (int I=0;I < HistoryCount; I++)
      if(LastStr[I].Name && LastStr[I].Name[0])
        SaveLastStr[I].Name=wcsdup(LastStr[I].Name);
      else
        SaveLastStr[I].Name=NULL;

    // �.�. �� ��� ���������, ��, ����� ���������� ��������� ������
    FreeHistory();

    // ������ �� �������
    ReadHistory();

    // ��������� ��������� �� ����������
    LastPtr0=LastPtr=SaveLastPtr;
    CurLastPtr0=CurLastPtr=SaveCurLastPtr;
    LastSimilar=SaveLastSimilar;

    // ��������� ����� �����
    AddToHistoryLocal(Str,Title,Type);

    // ���������
    SaveHistory();

    // ����������� (����������, �.�. ReadHistory ����� ������!)
    FreeHistory();

    // ����������� �����������
    LastPtr0=LastPtr=SaveLastPtr;
    CurLastPtr0=CurLastPtr=SaveCurLastPtr;
    LastSimilar=SaveLastSimilar;
    memcpy(LastStr,SaveLastStr,sizeof(struct HistoryRecordW) * HistoryCount);
  }

  AddToHistoryLocal(Str,Title,Type);
}


void HistoryW::AddToHistoryLocal(const wchar_t *Str,const wchar_t *Title,int Type)
{
  if(!LastStr)
    return;

  if(!Str || *Str == 0)
    return;

  HistoryRecordW AddRecord;

  if(TypeHistory == HISTORYTYPE_FOLDER)
    AddRecord.Name=(wchar_t *)xf_malloc((wcslen(Str)+wcslen(NullToEmptyW(Title))+2)*sizeof (wchar_t));
  else
    AddRecord.Name=wcsdup(Str);

  if(!AddRecord.Name)
    return;

  // ��� ���������� � ������� ��������� �������� ����� �� ��� ���������, �... ��� ��������.
  if(TypeHistory == HISTORYTYPE_FOLDER)
  {
    AddRecord.Name[0]=0;
    if(Title && *Title)
    {
      wcscat(AddRecord.Name,Title);
      wcscat(AddRecord.Name,L":");
    }
    wcscat(AddRecord.Name,Str);
    AddRecord.Title[0]=0;
  }
  else
  {
    xwcsncpy(AddRecord.Title,NullToEmptyW(Title),(sizeof(AddRecord.Title)-1)*sizeof (wchar_t));
    RemoveTrailingSpacesW(AddRecord.Title);
  }

  RemoveTrailingSpacesW(AddRecord.Name);
  AddRecord.Type=Type;

  int OldLastPtr=LastPtr-1;
  if (OldLastPtr < 0)
    OldLastPtr=HistoryCount-1;

  if (RemoveDups) // ������� ���������?
  {
    struct HistoryRecordW *PtrLastStr;
    int I, J;
    for (PtrLastStr=LastStr,I=0; I < HistoryCount; I++, PtrLastStr++)
    {
      if(PtrLastStr->Name && EqualType(AddRecord.Type,PtrLastStr->Type))
      {
        int Equal;
        if(TypeHistory == HISTORYTYPE_VIEW) // ������ �� ����� � ����
        {
           Equal=RemoveDups==1 && wcscmp(AddRecord.Name,PtrLastStr->Name)==0 ||
                 RemoveDups==2 && LocalStricmpW(AddRecord.Name,PtrLastStr->Name)==0;
        }
        else
        {
           Equal=RemoveDups==1 &&
                   wcscmp(AddRecord.Name,PtrLastStr->Name)==0 &&
                   wcscmp(AddRecord.Title,PtrLastStr->Title)==0 ||
                 RemoveDups==2 &&
                   LocalStricmpW(AddRecord.Name,PtrLastStr->Name)==0 &&
                   LocalStricmpW(AddRecord.Title,PtrLastStr->Title)==0;
        }

        if (Equal)
        {
          int Length=OldLastPtr-I;

          if (Length<0)
            Length+=HistoryCount;

          for (J=0; J <= Length; J++)
          {
            int Dest=(I+J) % HistoryCount;
            int Src=(I+J+1) % HistoryCount;

            if(LastStr[Dest].Name)
            {
              xf_free(LastStr[Dest].Name);
              LastStr[Dest].Name=NULL;
            }
            memmove(LastStr+Dest,LastStr+Src,sizeof(HistoryRecordW));
            memset(LastStr+Src,0,sizeof(HistoryRecordW));
          }

          memcpy(LastStr+OldLastPtr, &AddRecord, sizeof(HistoryRecordW));

          CurLastPtr0=LastPtr0=CurLastPtr=LastPtr;
          return;
        }
      }
    }
  }

  int Pos=(LastPtr-1) % HistoryCount;

  if(LastStr[Pos].Name && LastStr[LastPtr].Name &&
      (wcscmp(AddRecord.Name,LastStr[Pos].Name) != 0 ||
        wcscmp(AddRecord.Title,LastStr[Pos].Title) != 0 ||
        !EqualType(AddRecord.Type,LastStr[Pos].Type)))
    xf_free(LastStr[LastPtr].Name);

  memcpy(LastStr+LastPtr,&AddRecord,sizeof(HistoryRecordW));

  if (++LastPtr==HistoryCount)
     LastPtr=0;

  CurLastPtr0=LastPtr0=CurLastPtr=LastPtr;
}

/*
  ������� ���������� � ������, � �����... "��� ��� ������"
*/
BOOL HistoryW::SaveHistory()
{
  if(!LastStr)
    return FALSE; //??

  if (!*EnableSave)
    return TRUE;

  wchar_t *BufferLines=NULL,*BufferTitles=NULL,*PtrBuffer;
  unsigned char *TypesBuffer;
  TypesBuffer=(unsigned char *)alloca(HistoryCount+1);
  if(!TypesBuffer)
    return FALSE;

  DWORD SizeLines=0, SizeTitles=0, SizeTypes=0;
  int I, Len;

  for (I=0; I < HistoryCount; I++)
  {
    if(LastStr[I].Name)
    {
      Len=wcslen(LastStr[I].Name);
      if(WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && Len > 511)
        Len=511;

      if((PtrBuffer=(wchar_t*)xf_realloc(BufferLines,(SizeLines+Len+2)*sizeof (wchar_t))) == NULL)
      {
        xf_free(BufferLines);
        return FALSE;
      }
      BufferLines=PtrBuffer;
      xwcsncpy(BufferLines+SizeLines,LastStr[I].Name,Len);
      SizeLines+=Len+1;
    }
  }

  if(BufferLines)
  {
    BufferLines[SizeLines++]=0;

    if (SaveTitle && TypeHistory != HISTORYTYPE_VIEW)
    {
      BufferTitles=(wchar_t*)xf_malloc(HistoryCount*(HISTORY_TITLESIZE+2)*sizeof (wchar_t));
      if(BufferTitles)
      {
        for (I=0; I < HistoryCount; I++)
        {
          if(LastStr[I].Name)
          {
            wcscpy(BufferTitles+SizeTitles,LastStr[I].Title);
            SizeTitles+=wcslen(LastStr[I].Title)+1;
          }
        }
        BufferTitles[SizeTitles++]=0;
      }
    }

    if (SaveType)
    {
      memset(TypesBuffer,0,HistoryCount+1);
      for (I=0; I < HistoryCount; I++)
        if(LastStr[I].Name)
          TypesBuffer[SizeTypes++]=LastStr[I].Type+'0';
      TypesBuffer[SizeTypes++]=0;
    }
  }

  HKEY hKey;
  if ((hKey=CreateRegKeyW(strRegKey))!=NULL && BufferLines && *BufferLines)
  {
    if(!BufferLines)
      SizeLines=1;
    RegSetValueExW(hKey,L"Lines",0,REG_BINARY,(unsigned char *)BufferLines,SizeLines*sizeof(wchar_t));

    if (SaveTitle)
    {
      if(BufferTitles && *BufferTitles)
        RegSetValueExW(hKey,L"Titles",0,REG_BINARY,(unsigned char *)BufferTitles,SizeTitles*sizeof(wchar_t));
      else
        RegDeleteValueW(hKey,L"Titles");
    }

    if (SaveType)
    {
      if(TypesBuffer && *TypesBuffer)
        RegSetValueExW(hKey,L"Types",0,REG_SZ,TypesBuffer,SizeTypes);
      else
        RegDeleteValueW(hKey,L"Types");
    }

    RegSetValueExW(hKey,L"Position",0,REG_DWORD,(BYTE *)&CurLastPtr,sizeof(CurLastPtr));

    RegCloseKey(hKey);

    if (SaveTitle && BufferTitles)
      xf_free(BufferTitles);
    xf_free(BufferLines);

    return TRUE;
  }
  else
    DeleteRegKeyW(strRegKey);

  if(BufferLines)
    xf_free(BufferLines);
  if(BufferTitles)
    xf_free(BufferTitles);

  return FALSE;
}


BOOL HistoryW::ReadHistory()
{
  if(!LastStr)
    return FALSE;

  int NeedReadTitle=SaveTitle && CheckRegValueW(strRegKey, L"Titles");
  int NeedReadType =SaveType  && CheckRegValueW(strRegKey, L"Types");

  HKEY hKey;
  if ((hKey=OpenRegKeyW(strRegKey))==NULL)
    return FALSE;

  wchar_t *Buffer=NULL,*Buf;
  DWORD Size,Type;

  Size=GetRegKeySizeW(hKey, L"Lines");

  if(!Size) // ���� �������
    return TRUE;

  if((Buffer=(wchar_t*)xf_malloc(Size*sizeof (wchar_t))) == NULL)
  {
    RegCloseKey(hKey);
    return FALSE;
  }

  int StrPos, Length;
  if (RegQueryValueExW(hKey,L"Lines",0,&Type,(unsigned char *)Buffer,&Size)==ERROR_SUCCESS)
  {
    StrPos=0;
    Buf=Buffer;
    while ((int)Size > 1 && StrPos < HistoryCount)
    {
      Length=wcslen(Buf)+1;
      if((LastStr[StrPos].Name=(wchar_t*)xf_malloc(Length*sizeof (wchar_t))) == NULL)
      {
        xf_free(Buffer);
        FreeHistory();
        RegCloseKey(hKey);
        return FALSE;
      }
      wcscpy(LastStr[StrPos].Name,Buf);
      StrPos++;
      Buf+=Length;
      Size-=Length*sizeof (wchar_t);
    }
  }
  else
  {
    xf_free(Buffer);
    RegCloseKey(hKey);
    return FALSE;
  }

  if (NeedReadTitle)
  {
    Size=GetRegKeySizeW(hKey, L"Titles");
    if((Buf=(wchar_t*)xf_realloc(Buffer,Size*sizeof(wchar_t))) == NULL)
    {
      xf_free(Buffer);
      FreeHistory();
      RegCloseKey(hKey);
      return FALSE;
    }
    Buffer=Buf;
    if(RegQueryValueExW(hKey,L"Titles",0,&Type,(unsigned char *)Buffer,&Size)==ERROR_SUCCESS)
    {
      StrPos=0;
      while ((int)Size > 1 && StrPos < HistoryCount)
      {
        xwcsncpy(LastStr[StrPos].Title,Buf,(sizeof(LastStr[StrPos].Title)-1)*sizeof (wchar_t));
        ++StrPos;
        Length=wcslen(Buf)+1;
        Buf+=Length;
        Size-=Length*sizeof (wchar_t);
      }
    }
    else // ��� ��������� Title, �� ������ �� ��������, ������ _���_ � ����.
    {
      xf_free(Buffer);
      FreeHistory();
      RegCloseKey(hKey);
      return FALSE;
    }
  }
  xf_free(Buffer);

  if (NeedReadType)
  {
    unsigned char *TypesBuffer;
    TypesBuffer=(unsigned char *)alloca(HistoryCount+2);
    if(TypesBuffer)
      memset(TypesBuffer,0,HistoryCount+1);
    Size=HistoryCount+1;
    if(TypesBuffer && RegQueryValueExW(hKey,L"Types",0,&Type,(unsigned char *)TypesBuffer,&Size)==ERROR_SUCCESS)
    {
      StrPos=0;
      Buf=(wchar_t *)TypesBuffer;
      while (iswdigit(*Buf) && StrPos < HistoryCount)
      {
        LastStr[StrPos++].Type=*Buf-L'0';
        Buf++;
      }
    }
    else // ��� ��������� Type, �� ������ �� ��������, ������ _���_ � ����.
    {
      FreeHistory();
      RegCloseKey(hKey);
      return FALSE;
    }
  }

  Size=sizeof(CurLastPtr);
  RegQueryValueExW(hKey,L"Position",0,&Type,(BYTE *)&CurLastPtr,&Size);
  RegCloseKey(hKey);

  LastPtr0=CurLastPtr0=LastPtr=CurLastPtr;

  if(TypeHistory == HISTORYTYPE_VIEW)
    ReloadTitle();

  return TRUE;
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

int HistoryW::Select(const wchar_t *Title,const wchar_t *HelpTopic, string &strStr,int &Type,string *strItemTitle)
{
  if(!LastStr)
    return -1;

  MenuItemEx HistoryItem;

  int Code=-1,I,Height=ScrY-8,StrPos=0,IsUpdate;
  unsigned int CurCmd;
  int RetCode=1;

  {
    VMenu HistoryMenu(Title,NULL,0,TRUE, Height);
    /* $ 06.11.2001 IS
       ! ���� ������ � ��� � ���������� (Wrap)
    */
    HistoryMenu.SetFlags(VMENU_SHOWAMPERSAND|VMENU_WRAPMODE);
    /* IS $ */
    if (HelpTopic!=NULL)
      HistoryMenu.SetHelp(HelpTopic);
    HistoryMenu.SetPosition(-1,-1,0,0);
    HistoryMenu.AssignHighlights(TRUE);
    int Done=FALSE;
    int SetUpMenuPos=FALSE;

    while(!Done)
    {
      IsUpdate=FALSE;
      HistoryMenu.DeleteItems();
      HistoryMenu.Modal::ClearDone();
      HistoryMenu.SetPosition(-1,-1,0,0);

      // ���������� ������� ����

      for (CurCmd=LastPtr+1, I=0; I < HistoryCount-1; I++, CurCmd++)
      {
        CurCmd%=HistoryCount;

        if (LastStr[CurCmd].Name && *LastStr[CurCmd].Name)
        {
          int SizeTrunc=ScrX-14;

          string strRecord = LastStr[CurCmd].Name;

          TruncPathStrW(strRecord,SizeTrunc);
          ReplaceStringsW(strRecord, L"&",L"&&",-1);

          if (*LastStr[CurCmd].Title)
              strRecord = (string)LastStr[CurCmd].Title+L":"+(LastStr[CurCmd].Type==4?L"-":L" ")+strRecord;

          HistoryItem.Clear ();
          HistoryItem.strName = strRecord;

          if(CurCmd==CurLastPtr)
              HistoryItem.SetSelect(TRUE);
          HistoryMenu.SetUserData((void*)CurCmd,sizeof(DWORD),
                                 HistoryMenu.AddItemW(&HistoryItem));
        }
      }

      HistoryItem.Clear ();
      HistoryItem.strName = L"                    ";

      if(!SetUpMenuPos)
        HistoryItem.SetSelect(CurLastPtr==LastPtr);
      HistoryMenu.SetUserData((void*)-1,sizeof(DWORD),
           HistoryMenu.AddItemW(&HistoryItem));
      if(SetUpMenuPos)
      {
        HistoryMenu.SetSelectPos(StrPos,0);
        SetUpMenuPos=FALSE;
      }

      HistoryMenu.Show();
      while (!HistoryMenu.Done())
      {
        int Key=HistoryMenu.ReadInput();
        StrPos=HistoryMenu.GetSelectPos();

        switch(Key)
        {
          case KEY_CTRLR: // �������� � ��������� �����������
          {
            if(TypeHistory == HISTORYTYPE_FOLDER || TypeHistory == HISTORYTYPE_VIEW)
            {
              int ModifiedHistory=0;
              for(I=0; I < HistoryCount; ++I)
              {
                // ����� ������ �� �������
                if(LastStr[I].Name && *LastStr[I].Name && GetFileAttributesW(LastStr[I].Name) == (DWORD)-1)
                {
                  xf_free(LastStr[I].Name);
                  LastStr[I].Name=NULL;
                  LastStr[I].Title[0]=0;
                  ModifiedHistory++;
                }
              }
              if(ModifiedHistory) // ����������� �� ������ ������������
              {
                SaveHistory(); // ���������
                FreeHistory(); // ��� ��������
                ReadHistory(); // ���������
                /* TODO: ����� ������ Save/Free/Read �� ��� ����� ���� �� ����� ����� ����� PackHistory
                         �.�. ����� ��:
                           PackHistory();
                           SaveHistory();
                */
                HistoryMenu.Modal::SetExitCode(StrPos);
                HistoryMenu.SetUpdateRequired(TRUE);
                IsUpdate=TRUE; //??
              }
            }
            break;
          }

          case KEY_CTRLSHIFTENTER:
          case KEY_CTRLENTER:
          case KEY_SHIFTENTER:
          {
            HistoryMenu.Modal::SetExitCode(StrPos);
            Done=TRUE;
            RetCode=Key==KEY_CTRLSHIFTENTER?6:(Key==KEY_SHIFTENTER?2:3);
            break;
          }

          case KEY_F3:
          case KEY_F4:
          case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:
          {
            HistoryMenu.Modal::SetExitCode(StrPos);
            Done=TRUE;
            RetCode=(Key==KEY_F4? 5 : 4);
            break;
          }
          /* $ 09.04.2001 SVS
             ���� - ����������� �� ������� ������ � Clipboard
          */
          case KEY_CTRLC:
          case KEY_CTRLINS:  case KEY_CTRLNUMPAD0:
          {
            Code=(int)HistoryMenu.GetUserData(NULL,sizeof(DWORD),StrPos);

            if(Code != -1)
              CopyToClipboardW(LastStr[Code].Name);

            break;
          }
          /* SVS $ */
          case KEY_DEL:
          /* $ 23.07.2001 VVM
            + �������� ������������� ����� ��������� */
          {
            if(HistoryMenu.GetItemCount() > 1 &&
               (!Opt.Confirm.HistoryClear ||
                (Opt.Confirm.HistoryClear &&
                MessageW(MSG_WARNING,2,
                     UMSG((HistoryW::TypeHistory==HISTORYTYPE_CMD?MHistoryTitle:
                          (HistoryW::TypeHistory==HISTORYTYPE_FOLDER?MFolderHistoryTitle:
                          MViewHistoryTitle))),
                     UMSG(MHistoryClear),
                     UMSG(MClear),UMSG(MCancel))==0)))
            {
              HistoryMenu.Hide();
              FreeHistory(); // ������ ���� ����� ��������!
              SaveHistory();
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
      {
        StrPos=(int)HistoryMenu.GetUserData(NULL,sizeof(StrPos),Code);
        if(StrPos == -1)
          return -1;
        if(RetCode != 3 && ((TypeHistory == HISTORYTYPE_FOLDER && !LastStr[StrPos].Type) || TypeHistory == HISTORYTYPE_VIEW) && GetFileAttributesW(LastStr[StrPos].Name) == (DWORD)-1)
        {
          string strTruncFileName = LastStr[StrPos].Name;

          if ( !strTruncFileName.IsEmpty () )
              TruncPathStrW(strTruncFileName,ScrX-16);

          SetLastError(ERROR_FILE_NOT_FOUND);

          if(LastStr[StrPos].Type == 1 && TypeHistory == HISTORYTYPE_VIEW) // Edit? ����� ������� � ���� ���� ��������
          {
            if(MessageW(MSG_WARNING|MSG_ERRORTYPE,2,Title,!strTruncFileName.IsEmpty()?strTruncFileName:LastStr[StrPos].Name,UMSG(MViewHistoryIsCreate),UMSG(MHYes),UMSG(MHNo)) == 0)
              break;
          }
          else
            MessageW(MSG_WARNING|MSG_ERRORTYPE,1,Title,!strTruncFileName.IsEmpty()?strTruncFileName:LastStr[StrPos].Name,UMSG(MOk));
          Done=FALSE;
          SetUpMenuPos=TRUE;
          HistoryMenu.Modal::SetExitCode(StrPos=Code);
          continue;
        }
      }
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

  strStr = L"";

  if(LastStr[StrPos].Name)
      strStr = LastStr[StrPos].Name;

  if(RetCode < 4 || RetCode == 6)
    Type=LastStr[StrPos].Type;
  else
  {
    Type=RetCode-4; //????
    if(Type == 1 && LastStr[StrPos].Type == 4) //????
      Type=4;                                  //????
    RetCode=1;
  }

  if (strItemTitle!=NULL)
      *strItemTitle = LastStr[StrPos].Title;

  return RetCode;
}


void HistoryW::GetPrev(string &strStr)
{
  if(!LastStr)
    return;

  do
  {
    unsigned int NewPtr=(CurLastPtr-1)%HistoryCount;
    if (NewPtr!=LastPtr)
      CurLastPtr=NewPtr;
    else
      break;
  } while (!LastStr[CurLastPtr].Name || *LastStr[CurLastPtr].Name==0);

  if(LastStr[CurLastPtr].Name)
    strStr = LastStr[CurLastPtr].Name;
  else
    strStr = L"";
}


void HistoryW::GetNext(string &strStr)
{
  if(!LastStr)
    return;

  do
  {
    if (CurLastPtr!=LastPtr)
      CurLastPtr=(CurLastPtr+1)%HistoryCount;
    else
      break;
  } while (!LastStr[CurLastPtr].Name || *LastStr[CurLastPtr].Name==0);
  if(LastStr[CurLastPtr].Name)
    strStr = CurLastPtr==LastPtr ? L"":LastStr[CurLastPtr].Name;
  else
    strStr = L"";
}


void HistoryW::GetSimilar(string &strStr,int LastCmdPartLength)
{
  if(!LastStr)
    return;

  int Length=strStr.GetLength ();

  if (LastCmdPartLength!=-1 && LastCmdPartLength<Length)
    Length=LastCmdPartLength;

  if (LastCmdPartLength==-1)
    LastSimilar=0;

  for (int I=1;I<HistoryCount;I++)
  {
    int Pos=(LastPtr-LastSimilar-I)%HistoryCount;
    wchar_t *Name=LastStr[Pos].Name;
    if (Name && *Name && LocalStrnicmpW(strStr,Name,Length)==0 && wcscmp(strStr,Name)!=0)
    {
      int NewSimilar=(LastPtr-Pos)%HistoryCount;
      if (NewSimilar<=LastSimilar && ReturnSimilarTemplate)
      {
        ReturnSimilarTemplate=FALSE;
      }
      else
      {
        ReturnSimilarTemplate=TRUE;
        strStr = Name;
        LastSimilar=NewSimilar;
      }
      return;
    }
  }
  LastSimilar=0;
  ReturnSimilarTemplate=TRUE;
}


void HistoryW::SetAddMode(int EnableAdd,int RemoveDups,int KeepSelectedPos)
{
  HistoryW::EnableAdd=EnableAdd;
  HistoryW::RemoveDups=RemoveDups;
  HistoryW::KeepSelectedPos=KeepSelectedPos;
}

BOOL HistoryW::EqualType(int Type1, int Type2)
{
  return Type1 == Type2 || (TypeHistory == HISTORYTYPE_VIEW && (Type1 == 4 && Type2 == 1 || (Type1 == 1 && Type2 == 4)))?TRUE:FALSE;
}
