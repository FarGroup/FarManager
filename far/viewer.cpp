/*
viewer.cpp

Internal viewer

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop


#ifndef __FARCONST_HPP__
#include "farconst.hpp"
#endif
#ifndef __FARLANG_HPP__
#include "lang.hpp"
#endif
#ifndef __KEYS_HPP__
#include "keys.hpp"
#endif
#ifndef __COLOROS_HPP__
#include "colors.hpp"
#endif
#ifndef __FARSTRUCT_HPP__
#include "struct.hpp"
#endif
#ifndef __PLUGIN_HPP__
#include "plugin.hpp"
#endif
#ifndef __CLASSES_HPP__
#include "classes.hpp"
#endif
#ifndef __FARFUNC_HPP__
#include "fn.hpp"
#endif
#ifndef __FARGLOBAL_HPP__
#include "global.hpp"
#endif


static int InitLastSearchHex=0;

static struct CharTableSet InitTableSet;
static int InitUseDecodeTable=FALSE,InitTableNum=0,InitAnsiText=FALSE;

static int InitWrap=TRUE,InitHex=FALSE;

Viewer::Viewer()
{
  strcpy(LastSearchStr,GlobalSearchString);
  LastSearchCase=GlobalSearchCase;
  LastSearchReverse=GlobalSearchReverse;
  LastSearchHex=InitLastSearchHex;
  memcpy(&TableSet,&InitTableSet,sizeof(TableSet));
  UseDecodeTable=InitUseDecodeTable;
  TableNum=InitTableNum;
  AnsiText=InitAnsiText;

  if (AnsiText && TableNum==0)
  {
    int UseUnicode=TRUE;
    GetTable(&TableSet,TRUE,TableNum,UseUnicode);
    TableNum=0;
    UseDecodeTable=TRUE;
  }
  Unicode=(TableNum==1) && UseDecodeTable;
  Wrap=InitWrap;
  Hex=InitHex;

  ViewFile=NULL;
  ViewKeyBar=NULL;
  *FileName=0;
  FilePos=0;
  LeftPos=0;
  SecondPos=0;
  FileSize=0;
  LastPage=0;
  SelectPos=SelectSize=0;
  LastSelPos=0;
  SetStatusMode(TRUE);
  HideCursor=TRUE;
  *TempViewName=0;
  *Title=0;
  *PluginData=0;
  TableChangedByUser=FALSE;
  ReadStdin=FALSE;
  memset(SavePosAddr,0xff,sizeof(SavePosAddr));
  memset(SavePosLeft,0xff,sizeof(SavePosLeft));
  memset(UndoAddr,0xff,sizeof(UndoAddr));
  memset(UndoLeft,0xff,sizeof(UndoLeft));
  LastKeyUndo=FALSE;
  InternalKey=FALSE;
}


Viewer::~Viewer()
{
  KeepInitParameters();
  if (ViewFile)
  {
    fclose(ViewFile);
    if (Opt.SaveViewerPos)
    {
      char CacheName[NM*3];
      if (*PluginData)
        sprintf(CacheName,"%s%s",PluginData,PointToName(FileName));
      else
        strcpy(CacheName,FullFileName);
      unsigned int Table=0;
      if (TableChangedByUser)
      {
        Table=1;
        if (AnsiText)
          Table=2;
        else
          if (Unicode)
            Table=3;
          else
            if (UseDecodeTable)
              Table=TableNum+3;
      }
      CtrlObject->ViewerPosCache.AddPosition(CacheName,FilePos,LeftPos,0,0,Table);
    }
  }
  if (*TempViewName)
  {
    chmod(TempViewName,S_IREAD|S_IWRITE);
    remove(TempViewName);
    *PointToName(TempViewName)=0;
    RemoveDirectory(TempViewName);
  }
}


void Viewer::KeepInitParameters()
{
  strcpy(GlobalSearchString,LastSearchStr);
  GlobalSearchCase=LastSearchCase;
  GlobalSearchReverse=LastSearchReverse;
  InitLastSearchHex=LastSearchHex;
  memcpy(&InitTableSet,&TableSet,sizeof(InitTableSet));
  InitUseDecodeTable=UseDecodeTable;
  InitTableNum=TableNum;
  InitAnsiText=AnsiText;
  InitWrap=Wrap;
  InitHex=Hex;
}


int Viewer::OpenFile(char *Name)
{
  FILE *NewViewFile=NULL;
  if (CmdMode && strcmp(Name,"-")==0)
  {
    HANDLE OutHandle;
    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
      char TempPath[NM],TempName[NM];
      GetTempPath(sizeof(TempPath),TempPath);
      GetTempFileName(TempPath,"FAR",0,TempName);
      OutHandle=CreateFile(TempName,GENERIC_READ|GENERIC_WRITE,
                FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,
                FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE,NULL);
      if (OutHandle==INVALID_HANDLE_VALUE)
        return(FALSE);
      char ReadBuf[8192];
      DWORD ReadSize,WrittenSize;
      while (ReadFile(GetStdHandle(STD_INPUT_HANDLE),ReadBuf,sizeof(ReadBuf),&ReadSize,NULL))
        WriteFile(OutHandle,ReadBuf,ReadSize,&WrittenSize,NULL);
    }
    else
      OutHandle=GetStdHandle(STD_INPUT_HANDLE);
    int InpHandle=_open_osfhandle((long)OutHandle,O_BINARY);
    if (InpHandle!=-1)
      NewViewFile=fdopen(InpHandle,"rb");
    vseek(NewViewFile,0,SEEK_SET);
    ReadStdin=TRUE;
  }
  else
  {
    NewViewFile=NULL;

    DWORD Flags=0;
    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
      Flags|=FILE_FLAG_POSIX_SEMANTICS;

    HANDLE hView=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                            NULL,OPEN_EXISTING,Flags,NULL);
    if (hView==INVALID_HANDLE_VALUE && Flags!=0)
      hView=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL,OPEN_EXISTING,0,NULL);
    if (hView!=INVALID_HANDLE_VALUE)
    {
      int ViewHandle=_open_osfhandle((long)hView,O_BINARY);
      if (ViewHandle!=-1)
        NewViewFile=fdopen(ViewHandle,"rb");
    }
  }

  if (NewViewFile==NULL)
  {
    Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MViewerTitle),
            MSG(MViewerCannotOpenFile),Name,MSG(MOk));
    return(FALSE);
  }
  if (ViewFile)
    fclose(ViewFile);
  TableChangedByUser=FALSE;
  ViewFile=NewViewFile;
  strcpy(FileName,Name);
  ConvertNameToFull(FileName,FullFileName);

  HANDLE ViewFindHandle;
  ViewFindHandle=FindFirstFile(FileName,&ViewFindData);
  FindClose(ViewFindHandle);

  if (Opt.SaveViewerPos && !ReadStdin)
  {
    unsigned int NewLeftPos,TempPos1,TempPos2,Table,NewFilePos;
    char CacheName[NM*3];
    if (*PluginData)
      sprintf(CacheName,"%s%s",PluginData,PointToName(FileName));
    else
      strcpy(CacheName,FileName);
    CtrlObject->ViewerPosCache.GetPosition(CacheName,NewFilePos,NewLeftPos,TempPos1,TempPos2,Table);

    TableChangedByUser=(Table!=0);
    switch(Table)
    {
      case 0:
        break;
      case 1:
        AnsiText=UseDecodeTable=Unicode=0;
        break;
      case 2:
        {
          AnsiText=TRUE;
          UseDecodeTable=TRUE;
          Unicode=0;
          TableNum=0;
          int UseUnicode=TRUE;
          GetTable(&TableSet,TRUE,TableNum,UseUnicode);
        }
        break;
      case 3:
        AnsiText=UseDecodeTable=0;
        Unicode=1;
        break;
      default:
        AnsiText=Unicode=0;
        UseDecodeTable=1;
        TableNum=Table-3;
        PrepareTable(&TableSet,Table-5);
        break;
    }

    LastSelPos=FilePos=NewFilePos;
    LeftPos=NewLeftPos;
  }
  else
    FilePos=0;
  SetFileSize();
  if (FilePos>FileSize)
    FilePos=0;
  SetCRSym();
  if (Opt.ViewerAutoDetectTable && !TableChangedByUser)
  {
    UseDecodeTable=DetectTable(ViewFile,&TableSet,TableNum);
    if (TableNum>0)
      TableNum++;
    if (Unicode)
    {
      Unicode=0;
      FilePos*=2;
      SetFileSize();
    }
    if (AnsiText)
    {
      AnsiText=FALSE;
      ChangeViewKeyBar();
    }
  }
  return(TRUE);
}


void Viewer::SetCRSym()
{
  char Buf[2048];
  int CRCount=0,LFCount=0;
  int ReadSize,I;
  vseek(ViewFile,0,SEEK_SET);
  ReadSize=vread(Buf,sizeof(Buf),ViewFile);
  for (I=0;I<ReadSize;I++)
    switch(Buf[I])
    {
      case 10:
        LFCount++;
        break;
      case 13:
        if (I+1>=ReadSize || Buf[I+1]!=10)
          CRCount++;
        break;
    }
  if (LFCount<CRCount)
    CRSym=13;
  else
    CRSym=10;
}


void Viewer::DisplayObject()
{
  int SelPos,SelSize,Y,I;
  int SaveSelectSize=SelectSize;

  if (ViewFile==NULL)
    return;

  ViewY1=Y1+ShowStatusLine;

  if (HideCursor)
  {
    MoveCursor(79,24);
    SetCursorType(0,10);
  }
  vseek(ViewFile,FilePos,SEEK_SET);

  if (Hex)
    ShowHex();
  else
    for (I=0,Y=ViewY1;Y<=Y2;Y++,I++)
    {
      StrFilePos[I]=vtell(ViewFile);
      if (Y==ViewY1+1 && !feof(ViewFile))
        SecondPos=vtell(ViewFile);
      ReadString(OutStr[I],-1,sizeof(OutStr[I]),SelPos,SelSize);
      SetColor(COL_VIEWERTEXT);
      GotoXY(X1,Y);
      if (strlen(OutStr[I])>LeftPos)
      {
        mprintf("%-*.*s",ObjWidth,ObjWidth,&OutStr[I][LeftPos]);
        if (strlen(&OutStr[I][LeftPos])>ObjWidth)
        {
          GotoXY(X2,Y);
          SetColor(COL_VIEWERARROWS);
          mprintf(">");
        }
      }
      else
        mprintf("%*s",ObjWidth,"");
      if (LeftPos>0 && *OutStr[I]!=0)
      {
        GotoXY(X1,Y);
        SetColor(COL_VIEWERARROWS);
        mprintf("<");
      }
      if (SelSize && SelPos<LeftPos)
      {
        LeftPos=SelPos;
        SelectSize=SaveSelectSize;
        Show();
        return;
      }
      if (SelSize && SelPos>=LeftPos)
      {
        int SelX1=X1+SelPos-LeftPos;
        if (!Wrap && SelX1+SaveSelectSize-1>X2 && LeftPos<512)
        {
          LeftPos+=4;
          SelectSize=SaveSelectSize;
          Show();
          return;
        }
        SetColor(COL_VIEWERSELECTEDTEXT);
        GotoXY(SelX1,Y);
        if (SelSize>X2-SelX1+1)
          SelSize=X2-SelX1+1;
        if (SelSize>0)
          mprintf("%.*s",SelSize,&OutStr[I][SelPos]);
      }
    }
  ShowStatus();
}


void Viewer::ShowHex()
{
  char OutStr[512],TextStr[20];
  int SelPos,SelSize,EndFile,TextPos,Ch,Ch1,X,Y;
  SelSize=0;

  int HexLeftPos=LeftPos>80-ObjWidth ? Max(80-ObjWidth,0):LeftPos;

  for (EndFile=0,Y=ViewY1;Y<=Y2;Y++)
  {
    SetColor(COL_VIEWERTEXT);
    GotoXY(X1,Y);
    if (EndFile)
    {
      mprintf("%*s",ObjWidth,"");
      continue;
    }

    if (Y==ViewY1+1 && !feof(ViewFile))
      SecondPos=vtell(ViewFile);
    sprintf(OutStr,"%08X:  ",ftell(ViewFile));

    TextPos=0;

    if (Unicode)
      for (X=0;X<8;X++)
      {
        if (SelectSize>0 && SelectPos==vtell(ViewFile))
        {
          SelPos=strlen(OutStr);
          SelSize=SelectSize;
          SelectSize=0;
        }

        if ((Ch=getc(ViewFile))==EOF || (Ch1=getc(ViewFile))==EOF)
        {
          strcat(OutStr,"     ");
          TextStr[TextPos++]=' ';
          EndFile=LastPage=1;
        }
        else
        {
          sprintf(&OutStr[strlen(OutStr)],"%02X%02X ",Ch1,Ch);
          char TmpBuf[2],NewCh;
          TmpBuf[0]=Ch;
          TmpBuf[1]=Ch1;
          WideCharToMultiByte(CP_OEMCP,0,(LPCWSTR)TmpBuf,1,&NewCh,1," ",NULL);
          if (NewCh==0)
            NewCh=' ';
          TextStr[TextPos++]=NewCh;
          LastPage=0;
        }
        if (X==3)
          strcat(OutStr,"│ ");
      }
    else
      for (X=0;X<16;X++)
      {
        if (SelectSize>0 && SelectPos==vtell(ViewFile))
        {
          SelPos=strlen(OutStr);
          SelSize=SelectSize;
          SelectSize=0;
        }
        if ((Ch=vgetc(ViewFile))==EOF)
        {
          strcat(OutStr,"   ");
          TextStr[TextPos++]=' ';
          EndFile=LastPage=1;
        }
        else
        {
          sprintf(&OutStr[strlen(OutStr)],"%02X ",Ch);
          if (Ch==0)
            Ch=' ';
          TextStr[TextPos++]=Ch;
          LastPage=0;
        }
        if (X==7)
          strcat(OutStr,"│ ");
      }
    TextStr[TextPos]=0;
    if (UseDecodeTable && !Unicode)
      DecodeString(TextStr,TableSet.DecodeTable);
    strcat(TextStr," ");
    strcat(OutStr,"  ");
    strcat(OutStr,TextStr);
    if (strlen(OutStr)>HexLeftPos)
      mprintf("%-*.*s",ObjWidth,ObjWidth,&OutStr[HexLeftPos]);
    else
      mprintf("%*s",ObjWidth,"");
    if (SelSize && SelPos>=HexLeftPos)
    {
      SetColor(COL_VIEWERSELECTEDTEXT);
      GotoXY(X1+SelPos-HexLeftPos,Y);
      mprintf("%.*s",Unicode ? 4:2,&OutStr[SelPos]);
      SelSize=0;
    }
  }
}


void Viewer::ShowUp()
{
  int Tmp,Y,I;

  if (HideCursor)
    SetCursorType(0,10);
  vseek(ViewFile,FilePos,SEEK_SET);

  for (I=Y2-ViewY1-1;I>=0;I--)
  {
    StrFilePos[I+1]=StrFilePos[I];
    strcpy(OutStr[I+1],OutStr[I]);
  }
  StrFilePos[0]=FilePos;
  SecondPos=StrFilePos[1];

  ReadString(OutStr[0],SecondPos-FilePos,sizeof(OutStr[0]),Tmp,Tmp);

  for (I=0,Y=ViewY1;Y<=Y2;Y++,I++)
  {
    SetColor(COL_VIEWERTEXT);
    GotoXY(X1,Y);
    if (strlen(OutStr[I])>LeftPos)
    {
      mprintf("%-*.*s",ObjWidth,ObjWidth,&OutStr[I][LeftPos]);
      if (strlen(&OutStr[I][LeftPos])>ObjWidth)
      {
        GotoXY(X2,Y);
        SetColor(COL_VIEWERARROWS);
        mprintf(">");
      }
    }
    else
      mprintf("%*s",ObjWidth,"");
    if (LeftPos>0 && *OutStr[I]!=0)
    {
      GotoXY(X1,Y);
      SetColor(COL_VIEWERARROWS);
      mprintf("<");
    }
  }
  ShowStatus();
}


void Viewer::ShowStatus()
{
  char Status[200],Name[NM];
  if (!ShowStatusLine)
    return;
  strcpy(Name,*Title ? Title:FileName);
  int NameLength=ScrX-40;
  if (Opt.ViewerEditorClock)
    NameLength-=6;
  if (NameLength<20)
    NameLength=20;
  TruncStr(Name,NameLength);
  char *TableName;
  if (Unicode)
    TableName="Unicode";
  else
    if (UseDecodeTable)
      TableName=TableSet.TableName;
    else
      if (AnsiText)
        TableName="Win";
      else
        TableName="DOS";
  sprintf(Status,"%-*s %10.10s %10u %7.7s %-4d %s%3d%%",
          NameLength,Name,TableName,FileSize,MSG(MViewerStatusCol),
          LeftPos,Opt.ViewerEditorClock ? "":" ",
          LastPage ? 100:ToPercent(FilePos,FileSize));
  SetColor(COL_VIEWERSTATUS);
  GotoXY(X1,Y1);
  mprintf("%-*.*s",ObjWidth,ObjWidth,Status);
  if (Opt.ViewerEditorClock)
    ShowTime(FALSE);
}


void Viewer::SetStatusMode(int Mode)
{
  ShowStatusLine=Mode;
  ViewY1=Y1+ShowStatusLine;
}


void Viewer::ReadString(char *Str,int MaxSize,int StrSize,int &SelPos,int &SelSize)
{
  int OutPtr,Ch;

  OutPtr=0;
  SelSize=0;
  if (Hex)
  {
    OutPtr=vread(Str,Unicode ? 8:16,ViewFile);
    Str[Unicode ? 8:16]=0;
  }
  else
  {
    bool CRSkipped=false;
    while (1)
    {
      if (OutPtr>=StrSize-16)
        break;
      if (Wrap && OutPtr>X2-X1)
      {
        long SavePos=vtell(ViewFile);
        if ((Ch=vgetc(ViewFile))!=CRSym && (Ch!=13 || vgetc(ViewFile)!=CRSym))
          vseek(ViewFile,SavePos,SEEK_SET);
        break;
      }

      if (SelectSize>0 && SelectPos==vtell(ViewFile))
      {
        SelPos=OutPtr;
        SelSize=SelectSize;
        SelectSize=0;
      }

      if (MaxSize-- == 0)
        break;
      if ((Ch=vgetc(ViewFile))==EOF)
        break;
      if (Ch==CRSym)
        break;
      if (CRSkipped)
      {
        CRSkipped=false;
        Str[OutPtr++]=13;
      }

      if (Ch=='\t')
      {
        do
        {
          Str[OutPtr++]=' ';
        } while ((OutPtr % Opt.ViewTabSize)!=0);
        if (Wrap && OutPtr>X2-X1)
          Str[X2-X1+1]=0;
        continue;
      }
      if (Ch==13)
      {
        CRSkipped=true;
        continue;
      }
      if (Ch==0 || Ch==10)
        Ch=' ';
      Str[OutPtr++]=Ch;
    }
  }
  Str[OutPtr]=0;

  if (UseDecodeTable && !Unicode)
    DecodeString(Str,TableSet.DecodeTable);
  LastPage=feof(ViewFile);
}


int Viewer::ProcessKey(int Key)
{
  int Tmp,I;
  char ReadStr[528];

  if (!InternalKey && !LastKeyUndo && (FilePos!=UndoAddr[0] || LeftPos!=UndoLeft[0]))
  {
    for (int I=sizeof(UndoAddr)/sizeof(UndoAddr[0])-1;I>0;I--)
    {
      UndoAddr[I]=UndoAddr[I-1];
      UndoLeft[I]=UndoLeft[I-1];
    }
    UndoAddr[0]=FilePos;
    UndoLeft[0]=LeftPos;
  }

  if (Key!=KEY_ALTBS && Key!=KEY_CTRLZ && Key!=KEY_NONE && Key!=KEY_IDLE)
    LastKeyUndo=FALSE;

  if (Key>=KEY_CTRL0 && Key<=KEY_CTRL9)
  {
    int Pos=Key-KEY_CTRL0;
    if (SavePosAddr[Pos]!=0xffffffff)
    {
      FilePos=SavePosAddr[Pos];
      LeftPos=SavePosLeft[Pos];
      LastSelPos=FilePos;
      Show();
    }
    return(TRUE);
  }
  if (Key>=KEY_CTRLSHIFT0 && Key<=KEY_CTRLSHIFT9)
    Key=Key-KEY_CTRLSHIFT0+KEY_RCTRL0;
  if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9)
  {
    int Pos=Key-KEY_RCTRL0;
    SavePosAddr[Pos]=FilePos;
    SavePosLeft[Pos]=LeftPos;
    return(TRUE);
  }


  switch(Key)
  {
    case KEY_IDLE:
      {
        char Root[NM];
        GetPathRoot(FullFileName,Root);
        int DriveType=GetDriveType(Root);
        if (DriveType!=DRIVE_REMOVABLE && DriveType!=DRIVE_CDROM)
        {
          HANDLE ViewFindHandle;
          WIN32_FIND_DATA NewViewFindData;
          ViewFindHandle=FindFirstFile(FullFileName,&NewViewFindData);
          if (ViewFindHandle==INVALID_HANDLE_VALUE)
            return(TRUE);
          FindClose(ViewFindHandle);
          fflush(ViewFile);
          vseek(ViewFile,0,SEEK_END);
          long CurFileSize=vtell(ViewFile);
          if (ViewFindData.ftLastWriteTime.dwLowDateTime!=NewViewFindData.ftLastWriteTime.dwLowDateTime ||
              ViewFindData.ftLastWriteTime.dwHighDateTime!=NewViewFindData.ftLastWriteTime.dwHighDateTime ||
              CurFileSize!=FileSize)
          {
            ViewFindData=NewViewFindData;
            FileSize=CurFileSize;
            if (FilePos>FileSize)
              ProcessKey(KEY_CTRLEND);
            else
            {
              int PrevLastPage=LastPage;
              Show();
              if (PrevLastPage && !LastPage)
              {
                ProcessKey(KEY_CTRLEND);
                LastPage=TRUE;
              }
            }
          }
        }
        if (Opt.ViewerEditorClock)
          ShowTime(FALSE);
      }
      return(TRUE);
    case KEY_ALTBS:
    case KEY_CTRLZ:
      {
        for (int I=1;I<sizeof(UndoAddr)/sizeof(UndoAddr[0]);I++)
        {
          UndoAddr[I-1]=UndoAddr[I];
          UndoLeft[I-1]=UndoLeft[I];
        }
        if (UndoAddr[0]!=0xffffffff)
        {
          FilePos=UndoAddr[0];
          LeftPos=UndoLeft[0];
          UndoAddr[sizeof(UndoAddr)/sizeof(UndoAddr[0])-1]=0xffffffff;
          UndoLeft[sizeof(UndoAddr)/sizeof(UndoAddr[0])-1]=0xffffffff;

          Show();
          LastSelPos=FilePos;
        }
      }
      return(TRUE);
    case KEY_ADD:
    case KEY_SUBTRACT:
      if (*TempViewName==0)
      {
        char Name[NM];
        bool NextFileFound;

        if (Key==KEY_ADD)
          NextFileFound=ViewNamesList.GetNextName(Name);
        else
          NextFileFound=ViewNamesList.GetPrevName(Name);

        if (NextFileFound)
        {
          if (Opt.SaveViewerPos)
          {
            char CacheName[NM*3];
            if (*PluginData)
              sprintf(CacheName,"%s%s",PluginData,PointToName(FileName));
            else
              strcpy(CacheName,FileName);
            unsigned int Table=0;
            if (TableChangedByUser)
            {
              Table=1;
              if (AnsiText)
                Table=2;
              else
                if (Unicode)
                  Table=3;
                else
                  if (UseDecodeTable)
                    Table=TableNum+3;
            }
            CtrlObject->ViewerPosCache.AddPosition(CacheName,FilePos,LeftPos,0,0,Table);
          }
          if (PointToName(Name)==Name)
          {
            char ViewDir[NM];
            ViewNamesList.GetCurDir(ViewDir);
            chdir(ViewDir);
          }
          if (OpenFile(Name))
          {
            LeftPos=0;
            SecondPos=0;
            LastSelPos=FilePos;
            Show();
            ShowConsoleTitle();
          }
        }
      }
      return(TRUE);
    case KEY_F1:
      {
        Help Hlp("Viewer");
      }
      return(TRUE);
    case KEY_F2:
      Wrap=!Wrap;
      ChangeViewKeyBar();
      Show();
      LastSelPos=FilePos;
      return(TRUE);
    case KEY_F4:
      Hex=!Hex;
      ChangeViewKeyBar();
      Show();
      LastSelPos=FilePos;
      return(TRUE);
    case KEY_F7:
      Search(0,0);
      return(TRUE);
    case KEY_SHIFTF7:
    case KEY_SPACE:
      Search(1,0);
      return(TRUE);
    case KEY_F8:
      if ((AnsiText=!AnsiText)!=0)
      {
        int UseUnicode=TRUE;
        GetTable(&TableSet,TRUE,TableNum,UseUnicode);
      }
      if (Unicode)
      {
        FilePos*=2;
        Unicode=FALSE;
        SetFileSize();
      }
      TableNum=0;
      UseDecodeTable=AnsiText;
      ChangeViewKeyBar();
      Show();
      LastSelPos=FilePos;
      TableChangedByUser=TRUE;
      return(TRUE);
    case KEY_SHIFTF8:
      {
        int UseUnicode=TRUE;
        int GetTableCode=GetTable(&TableSet,FALSE,TableNum,UseUnicode);
        if (GetTableCode!=-1)
        {
          if (Unicode && !UseUnicode)
            FilePos*=2;
          if (!Unicode && UseUnicode)
            FilePos=(FilePos+(FilePos&1))/2;
          UseDecodeTable=GetTableCode;
          Unicode=UseUnicode;
          SetFileSize();
          AnsiText=FALSE;
          ChangeViewKeyBar();
          Show();
          LastSelPos=FilePos;
          TableChangedByUser=TRUE;
        }
      }
      return(TRUE);
    case KEY_ALTF8:
      GoTo();
      return(TRUE);
    case KEY_F11:
      CtrlObject->Plugins.CommandsMenu(FALSE,TRUE,0);
      Show();
      return(TRUE);
    case KEY_UP:
      if (FilePos>0)
      {
        Up();
        if (Hex)
        {
          FilePos&=~(Unicode ? 0x7:0xf);
          Show();
        }
        else
          ShowUp();
      }
      LastSelPos=FilePos;
      return(TRUE);
    case KEY_DOWN:
      if (!LastPage)
      {
        FilePos=SecondPos;
        Show();
      }
      LastSelPos=FilePos;
      return(TRUE);
    case KEY_PGUP:
      for (I=ViewY1;I<Y2;I++)
        Up();
      Show();
      LastSelPos=FilePos;
      return(TRUE);
    case KEY_PGDN:
      if (LastPage)
        return(TRUE);
      vseek(ViewFile,FilePos,SEEK_SET);
      for (I=ViewY1;I<Y2;I++)
      {
        ReadString(ReadStr,-1,sizeof(ReadStr),Tmp,Tmp);
        if (LastPage)
          return(TRUE);
      }
      FilePos=vtell(ViewFile);
      for (I=ViewY1;I<=Y2;I++)
        ReadString(ReadStr,-1,sizeof(ReadStr),Tmp,Tmp);
      if (LastPage)
      {
        InternalKey++;
        ProcessKey(KEY_CTRLPGDN);
        InternalKey--;
        return(TRUE);
      }
      Show();
      LastSelPos=FilePos;
      return(TRUE);
    case KEY_LEFT:
      if (LeftPos>0)
      {
        if (Hex && LeftPos>80-ObjWidth)
          LeftPos=Max(80-ObjWidth,1);
        LeftPos--;
        Show();
      }
      LastSelPos=FilePos;
      return(TRUE);
    case KEY_RIGHT:
      if (LeftPos<512)
      {
        LeftPos++;
        Show();
      }
      LastSelPos=FilePos;
      return(TRUE);
    case KEY_CTRLW:
      ShowProcessList();
      return(TRUE);
    case KEY_CTRLLEFT:
      LeftPos-=20;
      if (LeftPos<0)
        LeftPos=0;
      Show();
      LastSelPos=FilePos;
      return(TRUE);
    case KEY_CTRLRIGHT:
      LeftPos+=20;
      if (LeftPos>512)
        LeftPos=512;
      Show();
      LastSelPos=FilePos;
      return(TRUE);
    case KEY_HOME:
    case KEY_CTRLHOME:
      LeftPos=0;
    case KEY_CTRLPGUP:
      FilePos=0;
      Show();
      LastSelPos=FilePos;
      return(TRUE);
    case KEY_END:
    case KEY_CTRLEND:
      LeftPos=0;
    case KEY_CTRLPGDN:
      vseek(ViewFile,0,SEEK_END);
      FilePos=vtell(ViewFile);
      for (I=0;I<Y2-ViewY1;I++)
        Up();
      if (Hex)
        FilePos&=~(Unicode ? 0x7:0xf);
      Show();
      LastSelPos=FilePos;
      return(TRUE);
    default:
      if (Key>=' ' && Key<=255)
      {
        Search(0,Key);
        return(TRUE);
      }
  }
  return(FALSE);
}


int Viewer::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  if ((MouseEvent->dwButtonState & 3)==0)
    return(FALSE);

  if (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
      MouseEvent->dwMousePosition.Y<ViewY1 || MouseEvent->dwMousePosition.Y>Y2)
    return(FALSE);
  if (MouseEvent->dwMousePosition.X<X1+7)
    while (IsMouseButtonPressed() && MouseX<X1+7)
      ProcessKey(KEY_LEFT);
  else
    if (MouseEvent->dwMousePosition.X>X2-7)
      while (IsMouseButtonPressed() && MouseX>X2-7)
        ProcessKey(KEY_RIGHT);
    else
      if (MouseEvent->dwMousePosition.Y<ViewY1+(Y2-ViewY1)/2)
        while (IsMouseButtonPressed() && MouseY<ViewY1+(Y2-ViewY1)/2)
          ProcessKey(KEY_UP);
      else
        while (IsMouseButtonPressed() && MouseY>=ViewY1+(Y2-ViewY1)/2)
          ProcessKey(KEY_DOWN);
  return(TRUE);
}


void Viewer::Up()
{
  char Buf[512];
  int BufSize,StrPos,Skipped,I,J;
  BufSize=Min(sizeof(Buf),FilePos);
  if (BufSize==0)
    return;
  LastPage=0;
  if (Hex)
  {
    int UpSize=Unicode ? 8:16;
    if (FilePos<UpSize)
      FilePos=0;
    else
      FilePos-=UpSize;
    return;
  }
  vseek(ViewFile,FilePos-BufSize,SEEK_SET);
  vread(Buf,BufSize,ViewFile);
  Skipped=0;
  if (Buf[BufSize-1]==CRSym)
  {
    BufSize--;
    Skipped++;
  }
  if (BufSize>0 && CRSym==10 && Buf[BufSize-1]==13)
  {
    BufSize--;
    Skipped++;
  }
  for (I=BufSize-1;I>=-1;I--)
  {
    if (Buf[I]==CRSym || I==-1)
      if (!Wrap)
      {
        FilePos-=BufSize-(I+1)+Skipped;
        return;
      }
      else
      {
        if (!Skipped && I==-1)
          break;

        for (StrPos=0,J=I+1;J<=BufSize;J++)
        {
          if (StrPos==0 || StrPos >= ObjWidth)
          {
            if (J==BufSize)
            {
              if (Skipped==0)
                FilePos--;
              else
                FilePos-=Skipped;
              return;
            }
            if (CalcStrSize(&Buf[J],BufSize-J) <= ObjWidth)
            {
              FilePos-=BufSize-J+Skipped;
              return;
            }
            else
              StrPos=0;
          }
          if (J<BufSize)
            if (Buf[J]=='\t')
              StrPos+=Opt.ViewTabSize-(StrPos % Opt.ViewTabSize);
            else
              if (Buf[J]!=13)
                StrPos++;
        }
      }
  }
  for (I=Min(ObjWidth,BufSize);I>0;I-=5)
    if (CalcStrSize(&Buf[BufSize-I],I) <= ObjWidth)
    {
      FilePos-=I+Skipped;
      break;
    }
}


int Viewer::CalcStrSize(char *Str,int Length)
{
  int Size,I;
  for (Size=0,I=0;I<Length;I++)
    switch(Str[I])
    {
      case '\t':
        Size+=Opt.ViewTabSize-(Size % Opt.ViewTabSize);
        break;
      case 10:
      case 13:
        break;
      default:
        Size++;
        break;
    }
  return(Size);
}


void Viewer::SetViewKeyBar(KeyBar *ViewKeyBar)
{
  Viewer::ViewKeyBar=ViewKeyBar;
  ChangeViewKeyBar();
}


void Viewer::ChangeViewKeyBar()
{
  if (ViewKeyBar)
  {
    if (Wrap)
      ViewKeyBar->Change(MSG(MViewF2Unwrap),1);
    else
      ViewKeyBar->Change(MSG(MViewF2),1);

    if (Hex)
      ViewKeyBar->Change(MSG(MViewF4Text),3);
    else
      ViewKeyBar->Change(MSG(MViewF4),3);

    if (AnsiText)
      ViewKeyBar->Change(MSG(MViewF8DOS),7);
    else
      ViewKeyBar->Change(MSG(MViewF8),7);

    ViewKeyBar->Redraw();
  }
}


void Viewer::Search(int Next,int FirstChar)
{
  const char *TextHistoryName="SearchText";
  static struct DialogData SearchDlgData[]={
    DI_DOUBLEBOX,3,1,72,9,0,0,0,0,(char *)MViewSearchTitle,
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)MViewSearchFor,
    DI_EDIT,5,3,70,3,1,(DWORD)TextHistoryName,DIF_HISTORY,0,"",
    DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_RADIOBUTTON,5,5,0,0,0,1,DIF_GROUP,0,(char *)MViewSearchForText,
    DI_RADIOBUTTON,5,6,0,0,0,0,0,0,(char *)MViewSearchForHex,
    DI_CHECKBOX,40,5,0,0,0,0,0,0,(char *)MViewSearchCase,
    DI_CHECKBOX,40,6,0,0,0,0,0,0,(char *)MViewSearchReverse,
    DI_TEXT,3,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,8,0,0,0,0,DIF_CENTERGROUP,1,(char *)MViewSearchSearch,
    DI_BUTTON,0,8,0,0,0,0,DIF_CENTERGROUP,0,(char *)MViewSearchCancel
  };
  MakeDialogItems(SearchDlgData,SearchDlg);

  unsigned char SearchStr[256];
  char MsgStr[256];
  long MatchPos;
  int SearchLength,Case,ReverseSearch,SearchHex,Match;

  if (ViewFile==NULL || Next && *LastSearchStr==0)
    return;

  strncpy(SearchDlg[2].Data,LastSearchStr,sizeof(SearchDlg[2].Data));
  SearchDlg[4].Selected=!LastSearchHex;
  SearchDlg[5].Selected=LastSearchHex;
  SearchDlg[6].Selected=LastSearchCase;
  SearchDlg[7].Selected=LastSearchReverse;

  if (Unicode)
  {
    SearchDlg[4].Selected=TRUE;
    SearchDlg[5].Type=DI_TEXT;
    SearchDlg[5].Selected=FALSE;
    *SearchDlg[5].Data=0;
  }

  if (!Next)
  {
    Dialog Dlg(SearchDlg,sizeof(SearchDlg)/sizeof(SearchDlg[0]));
    Dlg.SetPosition(-1,-1,76,11);
    if (FirstChar)
    {
      Dlg.Show();
      Dlg.ProcessKey(FirstChar);
    }
    Dlg.Process();
    if (Dlg.GetExitCode()!=9)
      return;
  }
  strncpy(SearchStr,SearchDlg[2].Data,sizeof(SearchStr));
  SearchHex=SearchDlg[5].Selected;
  Case=SearchDlg[6].Selected;
  ReverseSearch=SearchDlg[7].Selected;

  strncpy(LastSearchStr,SearchStr,sizeof(LastSearchStr));
  LastSearchHex=SearchHex;
  LastSearchCase=Case;
  LastSearchReverse=ReverseSearch;

  if ((SearchLength=strlen(SearchStr))==0)
    return;

  {
    SaveScreen SaveScr;
    SetCursorType(FALSE,0);
    sprintf(MsgStr,"\"%s\"",SearchStr);
    Message(0,0,MSG(MViewSearchTitle),MSG(MViewSearchingFor),MsgStr);

    if (SearchHex)
      ConvertToHex(SearchStr,SearchLength);
    else
      if (!Case)
        for (int I=0;I<SearchLength;I++)
          SearchStr[I]=LocalUpper(SearchStr[I]);

    vseek(ViewFile,LastSelPos,SEEK_SET);
    Match=0;
    if (SearchLength>0 && (!ReverseSearch || LastSelPos>0))
    {
      char Buf[8192];
      long CurPos=LastSelPos;
      int BufSize=sizeof(Buf);
      if (ReverseSearch)
      {
        CurPos-=sizeof(Buf)-SearchLength;
        if (CurPos<0)
          BufSize+=CurPos;
      }
      int ReadSize;
      while (!Match)
      {
        if (ReverseSearch && CurPos<0)
          CurPos=0;

        vseek(ViewFile,CurPos,SEEK_SET);
        if ((ReadSize=vread(Buf,BufSize,ViewFile))<=0)
          break;
        if (CheckForEsc())
          return;
        if (UseDecodeTable && !SearchHex && !Unicode)
          for (int I=0;I<ReadSize;I++)
            Buf[I]=TableSet.DecodeTable[Buf[I]];

        if (Case || SearchHex)
        {
          int FirstCh=SearchStr[0];
          int MaxSize=ReadSize-SearchLength+1;
          int Increment=ReverseSearch ? -1:+1;
          for (int I=ReverseSearch ? MaxSize-1:0;I<MaxSize && I>=0;I+=Increment)
            if (Buf[I]==FirstCh)
            {
              Match=TRUE;
              for (int J=1;J<SearchLength;J++)
                if (Buf[I+J]!=SearchStr[J])
                  Match=FALSE;
              if (Match)
              {
                MatchPos=CurPos+I;
                break;
              }
            }
        }
        else
        {
          int FirstCh=LocalUpper(SearchStr[0]);
          int MaxSize=ReadSize-SearchLength+1;
          int Increment=ReverseSearch ? -1:+1;
          for (int I=ReverseSearch ? MaxSize-1:0;I<MaxSize && I>=0;I+=Increment)
            if (LocalUpper(Buf[I])==FirstCh)
            {
              Match=TRUE;
              for (int J=1;J<SearchLength;J++)
                if (LocalUpper(Buf[I+J])!=SearchStr[J])
                  Match=FALSE;
              if (Match)
              {
                MatchPos=CurPos+I;
                break;
              }
            }
        }

        if (ReadSize<sizeof(Buf))
          break;
        if (ReverseSearch)
        {
          if (CurPos<=0)
            break;
          CurPos-=sizeof(Buf)-SearchLength;
        }
        else
          CurPos+=sizeof(Buf)-SearchLength;
      }
    }
  }
  if (Match)
  {
    char Buf[1024];
    long StartLinePos=-1,SearchLinePos=MatchPos-sizeof(Buf);
    if (SearchLinePos<0)
      SearchLinePos=0;
    vseek(ViewFile,SearchLinePos,SEEK_SET);
    int ReadSize=Min(sizeof(Buf),MatchPos-SearchLinePos);
    ReadSize=vread(Buf,ReadSize,ViewFile);
    for (int I=ReadSize-1;I>=0;I--)
      if (Buf[I]==CRSym)
      {
        StartLinePos=SearchLinePos+I;
        break;
      }
    vseek(ViewFile,MatchPos+1,SEEK_SET);
    SelectPos=FilePos=MatchPos;
    SelectSize=SearchLength;
    LastSelPos=SelectPos+(ReverseSearch ? -1:1);
    if (Hex)
      FilePos&=~(Unicode ? 0x7:0xf);
    else
    {
      if (SelectPos!=StartLinePos)
        Up();
      int Length=SelectPos-StartLinePos-1;
      if (Wrap)
        Length%=ScrX+1;
      if (Length<=ObjWidth)
        LeftPos=0;
      if (Length-LeftPos>ObjWidth || Length<LeftPos)
      {
        LeftPos=Length;
        if (LeftPos>511 || LeftPos<0)
          LeftPos=0;
        else
          if (LeftPos>10)
            LeftPos-=10;
      }
    }
    Show();
  }
  else
    Message(MSG_DOWN|MSG_WARNING,1,MSG(MViewSearchTitle),
            MSG(MViewSearchCannotFind),MsgStr,MSG(MOk));
}


void Viewer::ConvertToHex(char *SearchStr,int &SearchLength)
{
  char OutStr[256],*SrcPtr;
  int OutPos=0,N=0;
  SrcPtr=SearchStr;
  while (*SrcPtr)
  {
    while (isspace(*SrcPtr))
      SrcPtr++;
    if (SrcPtr[0])
      if (SrcPtr[1]==0 || isspace(SrcPtr[1]))
      {
        N=HexToNum(SrcPtr[0]);
        SrcPtr++;
      }
      else
      {
        N=16*HexToNum(SrcPtr[0])+HexToNum(SrcPtr[1]);
        SrcPtr+=2;
      }
    if (N>=0)
      OutStr[OutPos++]=N;
    else
      break;
  }
  memcpy(SearchStr,OutStr,OutPos);
  SearchLength=OutPos;
}


int Viewer::HexToNum(int Hex)
{
  Hex=toupper(Hex);
  if (Hex>='0' && Hex<='9')
    return(Hex-'0');
  if (Hex>='A' && Hex<='F')
    return(Hex-'A'+10);
  return(-1000);
}


int Viewer::GetWrapMode()
{
  return(Wrap);
}


void Viewer::SetWrapMode(int Wrap)
{
  Viewer::Wrap=Wrap;
}


void Viewer::EnableHideCursor(int HideCursor)
{
  Viewer::HideCursor=HideCursor;
}


void Viewer::GetFileName(char *Name)
{
  strcpy(Name,FullFileName);
}


void Viewer::ShowConsoleTitle()
{
  char Title[NM+20];
  sprintf(Title,MSG(MInViewer),PointToName(FileName));
  SetFarTitle(Title);
}


void Viewer::SetTempViewName(char *Name)
{
  ConvertNameToFull(Name,TempViewName);
}


void Viewer::SetTitle(char *Title)
{
  if (Title==NULL)
    *Viewer::Title=0;
  else
    strcpy(Viewer::Title,Title);
}


long Viewer::GetFilePos()
{
  return(FilePos);
}


void Viewer::SetFilePos(long Pos)
{
  FilePos=Pos;
};


void Viewer::SetPluginData(char *PluginData)
{
  strcpy(Viewer::PluginData,NullToEmpty(PluginData));
}


void Viewer::SetNamesList(NamesList *List)
{
  if (List!=NULL)
    List->MoveData(&ViewNamesList);
}


int Viewer::vread(char *Buf,int Size,FILE *SrcFile)
{
  if (Unicode)
  {
    char TmpBuf[16384+10];
    int ReadSize=fread(TmpBuf,1,Size*2,SrcFile);
    TmpBuf[ReadSize]=0;
    ReadSize+=(ReadSize & 1);
    WideCharToMultiByte(CP_OEMCP,0,(LPCWSTR)TmpBuf,ReadSize/2,Buf,Size," ",NULL);
    return(ReadSize/2);
  }
  else
    return(fread(Buf,1,Size,SrcFile));
}


int Viewer::vseek(FILE *SrcFile,long Offset,int Whence)
{
  if (Unicode)
    return(fseek(SrcFile,Offset*2,Whence));
  else
    return(fseek(SrcFile,Offset,Whence));
}


unsigned long Viewer::vtell(FILE *SrcFile)
{
  unsigned long Pos=ftell(SrcFile);
  if (Unicode)
    Pos=(Pos+(Pos&1))/2;
  return(Pos);
}


int Viewer::vgetc(FILE *SrcFile)
{
  if (Unicode)
  {
    char TmpBuf[1];
    if (vread(TmpBuf,1,SrcFile)==0)
      return(EOF);
    return(TmpBuf[0]);
  }
  else
    return(getc(SrcFile));
}


void Viewer::GoTo()
{
  const char *LineHistoryName="ViewerOffset";
  static struct DialogData GoToDlgData[]=
  {
    DI_DOUBLEBOX,3,1,31,7,0,0,0,0,(char *)MViewerGoTo,
    DI_EDIT,5,2,29,2,1,(DWORD)LineHistoryName,DIF_HISTORY,1,"",
    DI_TEXT,3,3,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_RADIOBUTTON,5,4,0,0,0,0,DIF_GROUP,0,(char *)MGoToPercent,
    DI_RADIOBUTTON,5,5,0,0,0,0,0,0,(char *)MGoToHex,
    DI_RADIOBUTTON,5,6,0,0,0,0,0,0,(char *)MGoToDecimal,
  };
  MakeDialogItems(GoToDlgData,GoToDlg);
  static char PrevLine[20];
  static int PrevMode=0;

  strcpy(GoToDlg[1].Data,PrevLine);
  GoToDlg[3].Selected=GoToDlg[4].Selected=GoToDlg[5].Selected=0;
  GoToDlg[PrevMode+3].Selected=TRUE;

  {
    Dialog Dlg(GoToDlg,sizeof(GoToDlg)/sizeof(GoToDlg[0]));
    Dlg.SetPosition(-1,-1,35,9);
    Dlg.Process();
    if (Dlg.GetExitCode()!=1 || !isdigit(*GoToDlg[1].Data))
      return;
    strncpy(PrevLine,GoToDlg[1].Data,sizeof(PrevLine));
    DWORD Offset;
    if (GoToDlg[3].Selected)
    {
      PrevMode=0;
      unsigned int Percent=atoi(GoToDlg[1].Data);
      if (Percent>100)
        return;
      Offset=FileSize/100*Percent;
      if (Unicode)
        Offset*=2;
      while (ToPercent(Offset,FileSize)<Percent)
        Offset++;
    }
    if (GoToDlg[4].Selected)
    {
      PrevMode=1;
      char *endptr;
      Offset=strtoul(GoToDlg[1].Data,&endptr,16);
    }
    if (GoToDlg[5].Selected)
    {
      PrevMode=2;
      char *endptr;
      Offset=strtoul(GoToDlg[1].Data,&endptr,10);
    }
    FilePos=Unicode ? Offset/2:Offset;
  }
  LastSelPos=FilePos;
  Show();
}


void Viewer::SetFileSize()
{
  SaveFilePos SavePos(ViewFile);
  vseek(ViewFile,0,SEEK_END);
  FileSize=vtell(ViewFile);
}
