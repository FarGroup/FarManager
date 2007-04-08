/*
viewer.cpp

Internal viewer

*/

#include "headers.hpp"
#pragma hdrstop

#include "viewer.hpp"
#include "macroopcode.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "lang.hpp"
#include "colors.hpp"
#include "keys.hpp"
#include "poscache.hpp"
#include "help.hpp"
#include "dialog.hpp"
#include "panel.hpp"
#include "filepanels.hpp"
#include "fileview.hpp"
#include "savefpos.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"

static void PR_ViewerSearchMsg(void);
static void ViewerSearchMsg(const wchar_t *Name);

static struct CharTableSet InitTableSet;

static int InitHex=FALSE,SearchHex=FALSE;
/* $ 27.09.2000 SVS
   ID вьювера - по аналогии с Editor
*/
static int ViewerID=0;
/* SVS $*/

static const wchar_t BorderLine[]={0x2502,0x020,0x00};

Viewer::Viewer()
{
  _OT(SysLog(L"[%p] Viewer::Viewer()", this));
  /* $ 29.03.2001 IS
       "Наследуем" некоторые глобальные
  */

  memcpy(&ViOpt, &Opt.ViOpt, sizeof(ViewerOptions));
  /* IS $ */
  /* $ 12.07.2000 tran
     alloc memory for OutStr */
  for ( int i=0; i<=MAXSCRY; i++ )
  {
    Strings[i] = new ViewerString;
    memset (Strings[i], 0, sizeof(ViewerString));
    Strings[i]->lpData = new wchar_t[MAX_VIEWLINEB];
  }
  /* tran 12.07.2000 $ */
  strLastSearchStr.SetData (GlobalSearchString, CP_OEMCP); //BUGBUG
  LastSearchCase=GlobalSearchCase;
  /* $ 01.08.2000 KM
     Переменная для поиска "Whole words"
  */
  LastSearchWholeWords=GlobalSearchWholeWords;
  /* KM $ */
  LastSearchReverse=GlobalSearchReverse;
  /* $ 22.09.2003 */
  LastSearchHex=GlobalSearchHex;
  /* KM $ */
  memcpy(&TableSet,&InitTableSet,sizeof(TableSet));
  VM.UseDecodeTable=ViewerInitUseDecodeTable;
  VM.TableNum=ViewerInitTableNum;
  VM.AnsiMode=ViewerInitAnsiText;

  if (VM.AnsiMode && VM.TableNum==0)
  {
    int UseUnicode=TRUE;
    GetTable(&TableSet,TRUE,VM.TableNum,UseUnicode);
    VM.TableNum=0;
    VM.UseDecodeTable=TRUE;
  }
  VM.Unicode=(VM.TableNum==1) && VM.UseDecodeTable;
  /* $ 31.08.2000 SVS
    Вспомним тип врапа
  */
  VM.Wrap=Opt.ViOpt.ViewerIsWrap;
  VM.WordWrap=Opt.ViOpt.ViewerWrap;
  /* SVS $ */
  VM.Hex=InitHex;

  ViewFile=NULL;
  ViewKeyBar=NULL;
  FilePos=0;
  LeftPos=0;
  SecondPos=0;
  FileSize=0;
  LastPage=0;
  SelectPos=SelectSize=0;
  LastSelPos=0;
  SetStatusMode(TRUE);
  HideCursor=TRUE;
  DeleteFolder=TRUE;
  TableChangedByUser=FALSE;
  ReadStdin=FALSE;
  memset(&BMSavePos,0xff,sizeof(BMSavePos));
  memset(UndoData,0xff,sizeof(UndoData));
  LastKeyUndo=FALSE;
  InternalKey=FALSE;
  Viewer::ViewerID=::ViewerID++;
  CtrlObject->Plugins.CurViewer=this;
  OpenFailed=false;
  HostFileViewer=NULL;
  SelectPosOffSet=0;
}


Viewer::~Viewer()
{
  KeepInitParameters();
  if (ViewFile)
  {
    fclose(ViewFile);
    if (Opt.ViOpt.SaveViewerPos)
    {
      string strCacheName;

      if ( !strPluginData.IsEmpty() )
          strCacheName.Format (L"%s%s",(const wchar_t*)strPluginData,PointToName(strFileName));
      else
        strCacheName = strFullFileName;

      unsigned int Table=0;
      if (TableChangedByUser)
      {
        Table=1;
        if (VM.AnsiMode)
          Table=2;
        else
          if (VM.Unicode)
            Table=3;
          else
            if (VM.UseDecodeTable)
              Table=VM.TableNum+3;
      }
      {
        struct /*TPosCache32*/ TPosCache64 PosCache={0};
        PosCache.Param[0]=FilePos;
        PosCache.Param[1]=LeftPos;
        PosCache.Param[2]=VM.Hex;
        //=PosCache.Param[3];
        PosCache.Param[4]=Table;
        if(Opt.ViOpt.SaveViewerShortPos)
        {
          PosCache.Position[0]=BMSavePos.SavePosAddr;
          PosCache.Position[1]=(__int64*)BMSavePos.SavePosLeft;
          //PosCache.Position[2]=;
          //PosCache.Position[3]=;
        }
        CtrlObject->ViewerPosCache->AddPosition(strCacheName,&PosCache);
      }
    }
  }
  _tran(SysLog(L"[%p] Viewer::~Viewer, TempViewName=[%s]",this,TempViewName));
  /* $ 11.10.2001 IS
     Удаляем файл только, если нет открытых фреймов с таким именем.
  */


  if ( !strTempViewName.IsEmpty() && !FrameManager->CountFramesWithName(strTempViewName))
  /* IS $ */
  {
    /* $ 14.06.2002 IS
       Если DeleteFolder сброшен, то удаляем только файл. Иначе - удаляем еще
       и каталог.
    */
    if(DeleteFolder)
      DeleteFileWithFolder(strTempViewName);
    else
    {
      SetFileAttributesW(strTempViewName,FILE_ATTRIBUTE_NORMAL);
      DeleteFileW(strTempViewName); //BUGBUG
    }
    /* IS $ */
  }
  /* $ 12.07.2000 tran
     free memory  */
  for ( int i=0; i<=MAXSCRY; i++ )
  {
    delete [] Strings[i]->lpData;
    delete Strings[i];
  }

  /* tran 12.07.2000 $ */
  if (!OpenFailed)
  {
    CtrlObject->Plugins.CurViewer=this; //HostFileViewer;
    /* $ 15.09.2001 tran
       пора легализироваться */
    CtrlObject->Plugins.ProcessViewerEvent(VE_CLOSE,&ViewerID);
    /* tran $ */
  }
}


void Viewer::KeepInitParameters()
{
  UnicodeToAnsi (strLastSearchStr, GlobalSearchString, sizeof (GlobalSearchString)-1); //BUGBUG
  GlobalSearchCase=LastSearchCase;
  /* $ 01.08.2000 KM
     Сохранение параметра "Whole words" в глобальной GlobalSearchWholeWords
  */
  GlobalSearchWholeWords=LastSearchWholeWords;
  /* KM $ */
  GlobalSearchReverse=LastSearchReverse;
  /* $ 22.09.2003 KM */
  GlobalSearchHex=LastSearchHex;
  /* KM $ */
  memcpy(&InitTableSet,&TableSet,sizeof(InitTableSet));
  ViewerInitUseDecodeTable=VM.UseDecodeTable;
  ViewerInitTableNum=VM.TableNum;
  ViewerInitAnsiText=VM.AnsiMode;
  Opt.ViOpt.ViewerIsWrap=VM.Wrap;
  Opt.ViOpt.ViewerWrap=VM.WordWrap;
  InitHex=VM.Hex;
}


int Viewer::OpenFile(const wchar_t *Name,int warning)
{
  FILE *NewViewFile=NULL;
  OpenFailed=false;

  if (ViewFile)
    fclose(ViewFile);

  ViewFile=NULL;
  SelectSize = 0; // Сбросим выделение

  strFileName = Name;

  if (CmdMode && wcscmp (strFileName, L"-")==0)
  {
    HANDLE OutHandle;
    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
      string strTempName;
      if (!FarMkTempEx(strTempName))
      {
        OpenFailed=TRUE;
        return(FALSE);
      }
      OutHandle=FAR_CreateFileW(strTempName,GENERIC_READ|GENERIC_WRITE,
                FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,
                FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE,NULL);
      if (OutHandle==INVALID_HANDLE_VALUE)
      {
        OpenFailed=true;
        return(FALSE);
      }
      char ReadBuf[8192];
      DWORD ReadSize,WrittenSize;
      while (ReadFile(GetStdHandle(STD_INPUT_HANDLE),ReadBuf,sizeof(ReadBuf),&ReadSize,NULL))
        WriteFile(OutHandle,ReadBuf,ReadSize,&WrittenSize,NULL);
    }
    else
      OutHandle=GetStdHandle(STD_INPUT_HANDLE);
    int InpHandle=_open_osfhandle((intptr_t)OutHandle,O_BINARY);
    if (InpHandle!=-1)
      NewViewFile=fdopen(InpHandle,"rb");
    vseek(NewViewFile,0,SEEK_SET);
    ReadStdin=TRUE;
  }
  else
  {
    NewViewFile=NULL;

    DWORD Flags=0;
    DWORD ShareMode=FILE_SHARE_READ|FILE_SHARE_WRITE;
    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
      Flags|=FILE_FLAG_POSIX_SEMANTICS;
      ShareMode|=FILE_SHARE_DELETE;
    }

    HANDLE hView=FAR_CreateFileW(strFileName,GENERIC_READ,
                            ShareMode,
                            NULL,OPEN_EXISTING,Flags,NULL);
    if (hView==INVALID_HANDLE_VALUE && Flags!=0)
      hView=FAR_CreateFileW(strFileName,GENERIC_READ,
                       ShareMode,
                       NULL,OPEN_EXISTING,0,NULL);
    if (hView!=INVALID_HANDLE_VALUE)
    {
      int ViewHandle=_open_osfhandle((intptr_t)hView,O_BINARY);
      if (ViewHandle!=-1)
        NewViewFile=fdopen(ViewHandle,"rb");
    }
  }

  if (NewViewFile==NULL)
  {
    /* $ 04.07.2000 tran
       + 'warning' flag processing, in QuickView it is FALSE
         so don't show red message box */
    if (warning)
        Message(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MViewerTitle),
            UMSG(MViewerCannotOpenFile),strFileName,UMSG(MOk));
    /* tran 04.07.2000 $ */

    OpenFailed=true;
    return(FALSE);
  }

  TableChangedByUser=FALSE;
  ViewFile=NewViewFile;

  ConvertNameToFull (strFileName,strFullFileName);

  apiGetFindDataEx (strFileName, &ViewFindData);

  /* $ 19.09.2000 SVS
    AutoDecode Unicode
  */
  BOOL IsDecode=FALSE;

  /* $ 26.07.2002 IS
       Автоопределение Unicode не должно зависеть от опции
       "Автоопределение таблицы символов", т.к. Unicode не есть
       _таблица символов_ для перекодировки.
  */
  //if(ViOpt.AutoDetectTable)
  /* IS $ */
  {
    VM.Unicode=0;
    FirstWord=0;
    vseek(ViewFile,0,SEEK_SET);
    fread((wchar_t *)&FirstWord, 1, 2, ViewFile);
    //if(ReadSize == sizeof(FirstWord) &&
    if(FirstWord == 0x0FEFF || FirstWord == 0x0FFFE)
    {
      VM.AnsiMode=VM.UseDecodeTable=0;
      VM.Unicode=1;
      TableChangedByUser=TRUE;
      IsDecode=TRUE;
    }
  }
  /* SVS $ */

  if (Opt.ViOpt.SaveViewerPos && !ReadStdin)
  {
    __int64 NewLeftPos,NewFilePos;
    int Table;
    string strCacheName;

    if ( !strPluginData.IsEmpty() )
      strCacheName.Format (L"%s%s", (const wchar_t*)strPluginData,PointToName(strFileName));
    else
      strCacheName = strFileName;

    {
      struct /*TPosCache32*/ TPosCache64 PosCache={0};
      if(Opt.ViOpt.SaveViewerShortPos)
      {
        PosCache.Position[0]=BMSavePos.SavePosAddr;
        PosCache.Position[1]=(__int64*)BMSavePos.SavePosLeft;
        //PosCache.Position[2]=;
        //PosCache.Position[3]=;
      }
      CtrlObject->ViewerPosCache->GetPosition(strCacheName,&PosCache);
      NewFilePos=PosCache.Param[0];
      NewLeftPos=PosCache.Param[1];
      VM.Hex=(int)PosCache.Param[2];
      //=PosCache.Param[3];
      Table=(int)PosCache.Param[4];
    }

    if(!IsDecode)
    {
      TableChangedByUser=(Table!=0);
      switch(Table)
      {
        case 0:
          break;
        case 1:
          VM.AnsiMode=VM.UseDecodeTable=VM.Unicode=0;
          break;
        case 2:
          {
            VM.AnsiMode=TRUE;
            VM.UseDecodeTable=TRUE;
            VM.Unicode=0;
            VM.TableNum=0;
            int UseUnicode=TRUE;
            GetTable(&TableSet,TRUE,VM.TableNum,UseUnicode);
          }
          break;
        case 3:
          VM.AnsiMode=VM.UseDecodeTable=0;
          VM.Unicode=1;
          break;
        default:
          VM.AnsiMode=VM.Unicode=0;
          VM.UseDecodeTable=1;
          VM.TableNum=Table-3;
          PrepareTable(&TableSet,Table-5);
          break;
      }
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
  if (ViOpt.AutoDetectTable && !TableChangedByUser)
  {
    VM.UseDecodeTable=DetectTable(ViewFile,&TableSet,VM.TableNum);
    if (VM.TableNum>0)
      VM.TableNum++;
    if (VM.Unicode)
    {
      VM.Unicode=0;
      FilePos*=2;
      SetFileSize();
    }
    /* $ 27.04.2001 DJ
       всегда обновляем keybar после загрузки файла;
       вычисление ширины - в отдельную функцию
    */
    if (VM.AnsiMode)
      VM.AnsiMode=FALSE;
  }
  ChangeViewKeyBar();
  AdjustWidth();
  /* DJ $ */
  CtrlObject->Plugins.CurViewer=this; // HostFileViewer;
  /* $ 15.09.2001 tran
     пора легализироваться */
  CtrlObject->Plugins.ProcessViewerEvent(VE_READ,NULL);
  /* tran $ */
  return(TRUE);
}


/* $ 27.04.2001 DJ
   функция вычисления ширины в зависимости от наличия скроллбара
*/

void Viewer::AdjustWidth()
{
  /* $ 19.07.2000 tran
    + вычисление нужного */
  Width=X2-X1+1;
  XX2=X2;

  if ( ViOpt.ShowScrollbar )
  {
     Width--;
     XX2--;
  }

  /* tran 19.07.2000 $ */
}

/* DJ $ */

void Viewer::SetCRSym()
{
  if(!ViewFile)
    return;

  wchar_t Buf[2048];
  int CRCount=0,LFCount=0;
  int ReadSize,I;
  vseek(ViewFile,0,SEEK_SET);
  ReadSize=vread(Buf,sizeof(Buf)/sizeof (wchar_t),ViewFile);
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

void Viewer::ShowPage (int nMode)
{
  int I,Y;

  AdjustWidth();

  if ( ViewFile==NULL )
  {
    if( !strFileName.IsEmpty () && ((nMode == SHOW_RELOAD) || (nMode == SHOW_HEX)) )
    {
      SetScreen(X1,Y1,X2,Y2,L' ',COL_VIEWERTEXT);
      GotoXY(X1,Y1+ShowStatusLine);
      SetColor(COL_WARNDIALOGTEXT);
      mprintf(L"%.*s", XX2-X1+1, UMSG(MViewerCannotOpenFile));
      ShowStatus();
    }

    return;
  }


  if ( HideCursor )
  {
    MoveCursor(79,ScrY);
    SetCursorType(0,10);
  }

  vseek(ViewFile,FilePos,SEEK_SET);

  if (SelectSize == 0)
    SelectPos=FilePos;


  switch ( nMode )
  {
    case SHOW_HEX:
      ShowHex ();
      break;

    case SHOW_RELOAD:
      CtrlObject->Plugins.CurViewer = this; //HostFileViewer;

      ViewY1 = Y1+ShowStatusLine;

      for (I=0,Y=ViewY1;Y<=Y2;Y++,I++)
      {
        Strings[I]->nFilePos = vtell(ViewFile);

        if ( Y==ViewY1+1 && !feof(ViewFile) )
          SecondPos=vtell(ViewFile);

        ReadString(Strings[I],-1,MAX_VIEWLINEB);
      }

      break;

    case SHOW_UP:
      for (I=Y2-ViewY1-1;I>=0;I--)
      {
        Strings[I+1]->nFilePos = Strings[I]->nFilePos;
        Strings[I+1]->nSelStart = Strings[I]->nSelStart;
        Strings[I+1]->nSelEnd = Strings[I]->nSelEnd;
        Strings[I+1]->bSelection = Strings[I]->bSelection;

        wcscpy(Strings[I+1]->lpData, Strings[I]->lpData);
      }

      Strings[0]->nFilePos = FilePos;
      SecondPos = Strings[1]->nFilePos;

      ReadString(Strings[0],(int)(SecondPos-FilePos),MAX_VIEWLINEB);
      break;

    case SHOW_DOWN:

      for (I=0; I<Y2-ViewY1;I++)
      {
        Strings[I]->nFilePos = Strings[I+1]->nFilePos;
        Strings[I]->nSelStart = Strings[I+1]->nSelStart;
        Strings[I]->nSelEnd = Strings[I+1]->nSelEnd;
        Strings[I]->bSelection = Strings[I+1]->bSelection;

        wcscpy(Strings[I]->lpData, Strings[I+1]->lpData);
      }

      FilePos = Strings[0]->nFilePos;
      SecondPos = Strings[1]->nFilePos;

      vseek(ViewFile, Strings[Y2-ViewY1]->nFilePos, SEEK_SET);
      ReadString(Strings[Y2-ViewY1],-1,MAX_VIEWLINEB);
      Strings[Y2-ViewY1]->nFilePos = vtell(ViewFile);
      ReadString(Strings[Y2-ViewY1],-1,MAX_VIEWLINEB);

      break;
  }

  if ( nMode != SHOW_HEX )
  {
    for (I=0,Y=ViewY1;Y<=Y2;Y++,I++)
    {
      int StrLength = (int)wcslen(Strings[I]->lpData);

      SetColor(COL_VIEWERTEXT);
      GotoXY(X1,Y);

      if ( StrLength > LeftPos )
      {
        if(VM.Unicode && (FirstWord == 0x0FEFF || FirstWord == 0x0FFFE) && !I && !Strings[I]->nFilePos)
           mprintf(L"%-*.*s",Width,Width,&Strings[I]->lpData[(int)LeftPos+1]);
        else
           mprintf(L"%-*.*s",Width,Width,&Strings[I]->lpData[(int)LeftPos]);
      }
      else
        mprintf(L"%*s",Width,L"");

      if ( Strings[I]->bSelection )
      {
        __int64 SelX1;

        if ( LeftPos > Strings[I]->nSelStart )
          SelX1 = X1;
        else
          SelX1 = Strings[I]->nSelStart-LeftPos;

        if ( !VM.Wrap && (Strings[I]->nSelStart < LeftPos || Strings[I]->nSelStart > LeftPos+XX2-X1) )
        {
          if ( AdjustSelPosition )
          {
            LeftPos = Strings[I]->nSelStart-1;
            AdjustSelPosition = FALSE;
            Show();
            return;
          }
        }
        else
        {
          SetColor(COL_VIEWERSELECTEDTEXT);

          GotoXY(static_cast<int>(X1+SelX1),Y);

          __int64 Length = Strings[I]->nSelEnd-Strings[I]->nSelStart;

          if ( LeftPos > Strings[I]->nSelStart )
            Length = Strings[I]->nSelEnd-LeftPos;

          if ( LeftPos > Strings[I]->nSelEnd )
            Length = 0;

          mprintf(L"%.*s",(int)Length,&Strings[I]->lpData[(int)(SelX1+LeftPos+SelectPosOffSet)]);
        }
      }

      if (StrLength > LeftPos + Width && ViOpt.ShowArrows)
      {
        GotoXY(XX2,Y);
        SetColor(COL_VIEWERARROWS);
        BoxText(0xbb);
      }

      if (LeftPos>0 && *Strings[I]->lpData!=0  && ViOpt.ShowArrows)
      {
        GotoXY(X1,Y);
        SetColor(COL_VIEWERARROWS);
        BoxText(0xab);
      }
    }
  }

  DrawScrollbar();
  ShowStatus();
}

void Viewer::DisplayObject()
{
  ShowPage (VM.Hex?SHOW_HEX:SHOW_RELOAD);
}

void Viewer::ShowHex()
{
  wchar_t OutStr[MAX_VIEWLINE],TextStr[20];
  int SelPos=0, EndFile;
  __int64 SelSize;
  int Ch,Ch1,X,Y,TextPos;

  int SelStart, SelEnd;
  bool bSelStartFound = false, bSelEndFound = false;

  __int64 HexLeftPos=((LeftPos>80-ObjWidth) ? Max(80-ObjWidth,0):LeftPos);

  for (EndFile=0,Y=ViewY1;Y<=Y2;Y++)
  {
    bSelStartFound = false;
    bSelEndFound = false;

    SelSize=0;

    SetColor(COL_VIEWERTEXT);
    GotoXY(X1,Y);
    if (EndFile)
    {
      mprintf(L"%*s",ObjWidth,L"");
      continue;
    }

    if (Y==ViewY1+1 && !feof(ViewFile))
      SecondPos=vtell(ViewFile);
    swprintf(OutStr,L"%010I64X: ",(__int64)ftell64(ViewFile));

    TextPos=0;

    int HexStrStart = (int)wcslen (OutStr);

    SelStart = HexStrStart;
    SelEnd = SelStart;

    __int64 fpos = vtell(ViewFile);

    if ( fpos > SelectPos )
       bSelStartFound = true;

    if ( fpos < SelectPos+SelectSize-1 )
       bSelEndFound = true;

    if (VM.Unicode)
      for (X=0;X<8;X++)
      {
        __int64 fpos = vtell(ViewFile);

        if (SelectSize>0 && (SelectPos == fpos) )
        {
          bSelStartFound = true;
          SelStart = (int)wcslen (OutStr);
          SelSize=SelectSize;
          /* $ 22.01.2001 IS
              Внимание! Возможно, это не совсем верное решение проблемы
              выделения из плагинов, но мне пока другого в голову не пришло.
              Я приравниваю SelectSize нулю в Process*
          */
          //SelectSize=0;
          /* IS $ */
        }

        if (SelectSize>0 && (fpos == (SelectPos+SelectSize-1)) )
        {
          bSelEndFound = true;
          SelEnd = (int)wcslen (OutStr)+3;

          SelSize=SelectSize;
        }

        if ((Ch=getc(ViewFile))==EOF || (Ch1=getc(ViewFile))==EOF)
        {
          /* $ 28.06.2000 tran
             убираем показ пустой строки, если длина
             файла кратна 16 */
          EndFile=1;
          LastPage=1;
          if ( X==0 )
          {
             wcscpy(OutStr,L"");
             break;
          }
          wcscat(OutStr,L"     ");
          TextStr[TextPos++]=L' ';
          /* tran $ */
        }
        else
        {
          swprintf(OutStr+wcslen(OutStr),L"%02X%02X ",Ch1,Ch);
          char TmpBuf[2];

          wchar_t NewCh;

          /* $ 01.08.2002 tran
          обратный порядок байтов */
          if ( FirstWord == 0x0FFFE )
          {
              TmpBuf[0]=Ch1;
              TmpBuf[1]=Ch;
          }
          else
          {
              TmpBuf[0]=Ch;
              TmpBuf[1]=Ch1;
          }

          memcpy (&NewCh, &TmpBuf, 2);

          if (NewCh==0)
            NewCh=L' ';

          TextStr[TextPos++]=NewCh;
          LastPage=0;
        }
        if (X==3)
          wcscat(OutStr, BorderLine);
      }
    else
    {
      for (X=0;X<16;X++)
      {
        __int64 fpos = vtell(ViewFile);

        if (SelectSize>0 && (SelectPos == fpos) )
        {
          bSelStartFound = true;
          SelStart = (int)wcslen (OutStr);
          SelSize=SelectSize;
          /* $ 22.01.2001 IS
              Внимание! Возможно, это не совсем верное решение проблемы
              выделения из плагинов, но мне пока другого в голову не пришло.
              Я приравниваю SelectSize нулю в Process*
          */
          //SelectSize=0;
          /* IS $ */
        }

        if (SelectSize>0 && (fpos == (SelectPos+SelectSize-1)) )
        {
          bSelEndFound = true;
          SelEnd = (int)wcslen (OutStr)+1;

          SelSize=SelectSize;
        }



        if ((Ch=vgetc(ViewFile))==EOF)
        {
          /* $ 28.06.2000 tran
             убираем показ пустой строки, если длина
             файла кратна 16 */
          EndFile=1;
          LastPage=1;
          if ( X==0 )
          {
             wcscpy(OutStr,L"");
             break;
          }
          /* $ 03.07.2000 tran
             - вместо 5 пробелов тут надо 3 */
          wcscat(OutStr,L"   ");
          /* tran $ */
          TextStr[TextPos++]=L' ';
          /* tran $ */
        }
        else
        {
          char NewCh;

          WideCharToMultiByte(CP_OEMCP, 0, (const wchar_t*)&Ch,1, &NewCh,1," ",NULL);

          swprintf(OutStr+wcslen(OutStr),L"%02X ", NewCh);
          if (Ch==0)
            Ch=L' ';
          TextStr[TextPos++]=Ch;
          LastPage=0;
        }
        if (X==7)
          wcscat(OutStr,BorderLine);
      }
    }
    TextStr[TextPos]=0;
    /*if (VM.UseDecodeTable && !VM.Unicode)
      DecodeString(TextStr,(unsigned char *)TableSet.DecodeTable);*/ //BUGBUG
    wcscat(TextStr,L" ");

    if ( (SelEnd <= SelStart) && bSelStartFound )
       SelEnd = (int)wcslen (OutStr)-2;

    wcscat(OutStr,L" ");

    wcscat(OutStr,TextStr);
    if ((int)wcslen(OutStr)>HexLeftPos)
      mprintf(L"%-*.*s",ObjWidth,ObjWidth,OutStr+(int)HexLeftPos);
    else
      mprintf(L"%*s",ObjWidth,L"");

    if ( bSelStartFound && bSelEndFound )
    {
      SetColor(COL_VIEWERSELECTEDTEXT);
      GotoXY((int)((__int64)X1+SelStart-HexLeftPos),Y);

      mprintf(L"%.*s",SelEnd-SelStart+1,OutStr+(int)SelStart);

      SelSize = 0;
    }
  }
}

/* $ 27.04.2001 DJ
   отрисовка скроллбара - в отдельную функцию
*/

void Viewer::DrawScrollbar()
{
  /* $ 18.07.2000 tran
     рисование скролбара */
  if ( ViOpt.ShowScrollbar )
  {
    SetColor(COL_VIEWERSCROLLBAR);
    /* $ 27.04.2001 DJ
       если status line выключена, рисуем скроллбар до верха окна
    */
    ScrollBar(X2,ViewY1,Y2-ViewY1+1,(LastPage != 0? (!FilePos?0:100):ToPercent64(FilePos,FileSize)),100);
    /* DJ $ */
  }
  /* tran 18.07.2000 $ */
}

/* DJ $ */


void Viewer::GetTitle(string &strName,int,int)
{
  if ( !strTitle.IsEmpty () )
    strName = strTitle;
  else
  {
    if ( !(strFileName.At(1)==L':' && strFileName.At(2)==L'\\') ) //BUGBUG
    {
        string strPath;

        ViewNamesList.GetCurDir (strPath);

        AddEndSlash(strPath);

        strName = strPath+strFileName;
    }
    else
      strName = strFileName;
  }
}

void Viewer::ShowStatus()
{
  string strName;
  string strStatus;

  if (!ShowStatusLine)
    return;

  GetTitle(strName);

  int NameLength=ScrX-43; //???41
  if (Opt.ViewerEditorClock && HostFileViewer!=NULL && HostFileViewer->IsFullScreen())
    NameLength-=6;
  if (NameLength<20)
    NameLength=20;

  /* $ 01.10.2000 IS
     ! Показывать букву диска в статусной строке
  */
  TruncPathStr(strName, NameLength);
  /* IS $ */
  string strTableName;
  string strTmpTableName;
  if (VM.Unicode)
    strTableName=L"Unicode";
  else if (VM.UseDecodeTable)
  {
    strTmpTableName.SetData (TableSet.TableName, CP_OEMCP);
    strTableName=RemoveChar(strTmpTableName,L'&',TRUE);
  }
  else if (VM.AnsiMode)
    strTableName=L"Win";
  else
    strTableName=L"DOS";

  const wchar_t *lpwszStatusFormat = L"%-*s %10.10s %13I64u %7.7s %-4I64d %s%3d%%";

  strStatus.Format (
        lpwszStatusFormat,
        NameLength,
        (const wchar_t*)strName,
        (const wchar_t*)strTableName,
        FileSize,
        UMSG(MViewerStatusCol),
        LeftPos,
        Opt.ViewerEditorClock ? L"":L" ",
        (LastPage ? 100:ToPercent64(FilePos,FileSize))
        );

  SetColor(COL_VIEWERSTATUS);
  GotoXY(X1,Y1);

  mprintf(L"%-*.*s",Width+(ViOpt.ShowScrollbar?1:0),
                   Width+(ViOpt.ShowScrollbar?1:0), (const wchar_t*)strStatus);

  if (Opt.ViewerEditorClock && HostFileViewer!=NULL && HostFileViewer->IsFullScreen())
    ShowTime(FALSE);
}


void Viewer::SetStatusMode(int Mode)
{
  ShowStatusLine=Mode;
  ViewY1=Y1+ShowStatusLine;
}


void Viewer::ReadString (ViewerString *pString, int MaxSize, int StrSize)
{
  int Ch, Ch2;
  __int64 OutPtr;

  bool bSelStartFound = false, bSelEndFound = false;

  pString->bSelection = false;

  AdjustWidth();

  OutPtr=0;

  if (VM.Hex)
  {
    OutPtr=vread(pString->lpData,VM.Unicode ? 8:16,ViewFile);
    pString->lpData[VM.Unicode ? 8:16]=0;
  }
  else
  {
    bool CRSkipped=false;

    if ( SelectSize && vtell (ViewFile) > SelectPos )
    {
      pString->nSelStart = 0;
      bSelStartFound = true;
    }

    while (1)
    {
      if (OutPtr>=StrSize-16)
        break;
      /* $ 12.07.2000 SVS
        ! Wrap - трехпозиционный
      */
      if (VM.Wrap && OutPtr>XX2-X1)
      {
        /* $ 11.07.2000 tran
           + warp are now WORD-WRAP */
        __int64 SavePos=vtell(ViewFile);
        if ((Ch=vgetc(ViewFile))!=CRSym && (Ch!=13 || vgetc(ViewFile)!=CRSym))
        {
          vseek(ViewFile,SavePos,SEEK_SET);
          if (VM.WordWrap && RegVer) // только для зарегестрированных
          {
            if ( !IsSpace(Ch) && !IsSpace(pString->lpData[(int)OutPtr]))
            {
               __int64 SavePtr=OutPtr;
               /* $ 18.07.2000 tran
                  добавил в качестве wordwrap разделителей , ; > ) */
               while (OutPtr)
               {
                  Ch2=pString->lpData[(int)OutPtr];
                  if(IsSpace(Ch2) || Ch2==L',' || Ch2==L';' || Ch2==L'>'|| Ch2==L')')
                    break;
                  OutPtr--;
               }

               Ch2=pString->lpData[(int)OutPtr];
               if (Ch2==L',' || Ch2==L';' || Ch2==L')' || Ch2==L'>')
                   OutPtr++;
               else
                   while (IsSpace(pString->lpData[(int)OutPtr]) && OutPtr<=SavePtr)
                      OutPtr++;

               if (OutPtr)
               {
                  vseek(ViewFile,OutPtr-SavePtr,SEEK_CUR);
                  //
               }
               else
                  OutPtr=SavePtr;
            }
            /* tran 11.07.2000 $ */
            /* $ 13.09.2000 tran
               remove space at WWrap */
            __int64 savepos=vtell(ViewFile);
            while (IsSpace(Ch))
                Ch=vgetc(ViewFile);
            if ( vtell(ViewFile)!=savepos)
                vseek(ViewFile,-1,SEEK_CUR);
            /* tran 13.09.2000 $ */
          }// wwrap
          /* SVS $ */
        }
        break;
      }

      if (SelectSize > 0 && SelectPos==vtell(ViewFile))
      {
         pString->nSelStart = OutPtr+(CRSkipped?1:0);;
         bSelStartFound = true;
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
        pString->lpData[(int)OutPtr++]=13;
      }

      if (Ch==L'\t')
      {
        do
        {
          pString->lpData[(int)OutPtr++]=L' ';
        } while ((OutPtr % ViOpt.TabSize)!=0 && ((int)OutPtr < (MAX_VIEWLINEB-1)));
        // 12.07.2000 SVS - Wrap - 3-x позиционный и если есть регистрация :-)
        // 22.01.2002 IS - Не забудем и про простую свертку не по словам
        if ((VM.Wrap && (!VM.WordWrap || (VM.WordWrap && RegVer)))
            && OutPtr>XX2-X1)
          pString->lpData[XX2-X1+1]=0;
        continue;
      }
      /* $ 20.09.01 IS
         Баг: не учитывали левую границу при свертке
      */
      if (Ch==13)
      {
        CRSkipped=true;
        if(OutPtr>=XX2-X1)
        {
          __int64 SavePos=vtell(ViewFile);
          int nextCh=vgetc(ViewFile);
          if(nextCh!=CRSym && nextCh!=EOF) CRSkipped=false;
          vseek(ViewFile,SavePos,SEEK_SET);
        }
        if(CRSkipped)
           continue;
      }
      /* IS $ */
      if (Ch==0 || Ch==10)
        Ch=L' ';
      pString->lpData[(int)OutPtr++]=Ch;

      if (SelectSize > 0 && (SelectPos+SelectSize)==vtell(ViewFile))
      {
         pString->nSelEnd = OutPtr;
         bSelEndFound = true;
      }
    }
  }

  pString->lpData[(int)OutPtr]=0;

  if ( !bSelEndFound && SelectSize && vtell (ViewFile) < SelectPos+SelectSize )
  {
     bSelEndFound = true;
     pString->nSelEnd = wcslen (pString->lpData);
  }

  if ( bSelStartFound )
  {
    if ( pString->nSelStart > (__int64)wcslen (pString->lpData) )
      bSelStartFound = false;

    if ( bSelEndFound )
       if ( pString->nSelStart > pString->nSelEnd )
          bSelStartFound = false;
  }

  /*if (VM.UseDecodeTable && !VM.Unicode)
    DecodeString(pString->lpData,(unsigned char *)TableSet.DecodeTable);*/ //BUGBUG

  LastPage=feof(ViewFile);

  if ( bSelStartFound && bSelEndFound )
    pString->bSelection = true;
}


int Viewer::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
  switch(OpCode)
  {
    case MCODE_C_EMPTY:
      return FileSize==0;
    case MCODE_C_SELECTED:
      return SelectSize==0?FALSE:TRUE;
    case MCODE_C_EOF:
      return LastPage || ViewFile==NULL;
    case MCODE_C_BOF:
      return !FilePos || ViewFile==NULL;
    case MCODE_V_VIEWERSTATE:
    {
      DWORD MacroViewerState=0;
      MacroViewerState|=VM.UseDecodeTable?0x00000001:0;
      MacroViewerState|=VM.AnsiMode?0x00000002:0;
      MacroViewerState|=VM.Unicode?0x00000004:0;
      MacroViewerState|=VM.Wrap?0x00000008:0;
      MacroViewerState|=VM.WordWrap?0x00000010:0;
      MacroViewerState|=VM.Hex?0x00000020:0;
      return MacroViewerState;
    }
  }

  return 0;
}

/* $ 28.01.2001
   - Путем проверки ViewFile на NULL избавляемся от падения
*/
int Viewer::ProcessKey(int Key)
{
  int I;

  ViewerString vString;

  /* $ 22.01.2001 IS
       Происходят какие-то манипуляции -> снимем выделение
  */
  if (!ViOpt.PersistentBlocks &&
      Key!=KEY_IDLE && Key!=KEY_NONE && !(Key==KEY_CTRLINS||Key==KEY_CTRLNUMPAD0) && Key!=KEY_CTRLC)
    SelectSize=0;
  /* IS $ */

  if (!InternalKey && !LastKeyUndo && (FilePos!=UndoData[0].UndoAddr || LeftPos!=UndoData[0].UndoLeft))
  {
    for (int I=sizeof(UndoData)/sizeof(UndoData[0])-1;I>0;I--)
    {
      UndoData[I].UndoAddr=UndoData[I-1].UndoAddr;
      UndoData[I].UndoLeft=UndoData[I-1].UndoLeft;
    }
    UndoData[0].UndoAddr=FilePos;
    UndoData[0].UndoLeft=LeftPos;
  }

  if (Key!=KEY_ALTBS && Key!=KEY_CTRLZ && Key!=KEY_NONE && Key!=KEY_IDLE)
    LastKeyUndo=FALSE;

  if (Key>=KEY_CTRL0 && Key<=KEY_CTRL9)
  {
    int Pos=Key-KEY_CTRL0;
    if (BMSavePos.SavePosAddr[Pos]!=-1)
    {
      FilePos=BMSavePos.SavePosAddr[Pos];
      LeftPos=BMSavePos.SavePosLeft[Pos];
//      LastSelPos=FilePos;
      Show();
    }
    return(TRUE);
  }
  if (Key>=KEY_CTRLSHIFT0 && Key<=KEY_CTRLSHIFT9)
    Key=Key-KEY_CTRLSHIFT0+KEY_RCTRL0;
  if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9)
  {
    int Pos=Key-KEY_RCTRL0;
    BMSavePos.SavePosAddr[Pos]=FilePos;
    BMSavePos.SavePosLeft[Pos]=LeftPos;
    return(TRUE);
  }


  switch(Key)
  {
    case KEY_F1:
    {
      {
        Help Hlp (L"Viewer");
      }
      return(TRUE);
    }

    case KEY_CTRLU:
    {
//      if (SelectSize)
      {
        SelectSize = 0;
        Show();
      }
      return(TRUE);
    }

    /* $ 05.09.2001 VVM
      + Копирование выделения в клипбоард */
    case KEY_CTRLC:
    case KEY_CTRLINS:  case KEY_CTRLNUMPAD0:
    {
      if (SelectSize && ViewFile)
      {
        wchar_t *SelData;
        int DataSize = (int)SelectSize+(VM.Unicode?2:1);
        __int64 CurFilePos=vtell(ViewFile);

        if ((SelData=(wchar_t*)xf_malloc(DataSize*sizeof (wchar_t))) != NULL)
        {
          memset(SelData, 0, DataSize*sizeof (wchar_t));
          vseek(ViewFile,SelectPos,SEEK_SET);
          vread(SelData, (int)SelectSize, ViewFile);
          /*if (VM.UseDecodeTable && !VM.Unicode) BUGBUG
            DecodeString(SelData, (unsigned char *)TableSet.DecodeTable);*/
          CopyToClipboard(SelData);
          xf_free(SelData);
          vseek(ViewFile,CurFilePos,SEEK_SET);
        } /* if */
      } /* if */
      return(TRUE);
    }
    /* VVM $ */

    /* $ 18.07.2000 tran
       включить/выключить скролбар */
    case KEY_CTRLS:
    {
        ViOpt.ShowScrollbar=!ViOpt.ShowScrollbar;
        /* $ 10.03.2002 tran
           Bugz#275 - CtrlS должне работать глобально */
        Opt.ViOpt.ShowScrollbar=ViOpt.ShowScrollbar;
        /* tran $ */
        Show();
        return (TRUE);
    }
    /* tran 18.07.2000 $ */

    case KEY_IDLE:
    {
      {
        if(ViewFile)
        {
          string strRoot;
          GetPathRoot(strFullFileName, strRoot);
          int DriveType=FAR_GetDriveType(strRoot);
          if (DriveType!=DRIVE_REMOVABLE && !IsDriveTypeCDROM(DriveType))
          {
            FAR_FIND_DATA_EX NewViewFindData;

            if ( !apiGetFindDataEx (strFullFileName,&NewViewFindData) )
              return TRUE;

            fflush(ViewFile);
            vseek(ViewFile,0,SEEK_END);
            __int64 CurFileSize=vtell(ViewFile);
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
                __int64 PrevLastPage=LastPage;
                Show();
                if (PrevLastPage && !LastPage)
                {
                  ProcessKey(KEY_CTRLEND);
                  LastPage=TRUE;
                }
              }
            }
          }
        }
        if (Opt.ViewerEditorClock && HostFileViewer!=NULL && HostFileViewer->IsFullScreen())
          ShowTime(FALSE);
      }
      return(TRUE);
    }

    case KEY_ALTBS:
    case KEY_CTRLZ:
    {
      for (int I=1;I<sizeof(UndoData)/sizeof(UndoData[0]);I++)
      {
        UndoData[I-1].UndoAddr=UndoData[I].UndoAddr;
        UndoData[I-1].UndoLeft=UndoData[I].UndoLeft;
      }
      if (UndoData[0].UndoAddr!=-1)
      {
        FilePos=UndoData[0].UndoAddr;
        LeftPos=UndoData[0].UndoLeft;
        UndoData[sizeof(UndoData)/sizeof(UndoData[0])-1].UndoAddr=-1;
        UndoData[sizeof(UndoData)/sizeof(UndoData[0])-1].UndoLeft=-1;

        Show();
//        LastSelPos=FilePos;
      }
      return(TRUE);
    }

    case KEY_ADD:
    case KEY_SUBTRACT:
    {
      if ( strTempViewName.IsEmpty() )
      {
        string strName;
        string strShortName;
        bool NextFileFound;

        if (Key==KEY_ADD)
          NextFileFound=ViewNamesList.GetNextName(strName, strShortName);
        else
          NextFileFound=ViewNamesList.GetPrevName(strName, strShortName);

        if (NextFileFound)
        {
          if (Opt.ViOpt.SaveViewerPos)
          {
            string strCacheName;

            if ( !strPluginData.IsEmpty() )
              strCacheName.Format (L"%s%s", (const wchar_t*)strPluginData,PointToName(strFileName));
            else
              strCacheName = strFileName;

            unsigned int Table=0;
            if (TableChangedByUser)
            {
              Table=1;
              if (VM.AnsiMode)
                Table=2;
              else
                if (VM.Unicode)
                  Table=3;
                else
                  if (VM.UseDecodeTable)
                    Table=VM.TableNum+3;
            }
            {
              struct /*TPosCache32*/ TPosCache64 PosCache={0};
              PosCache.Param[0]=FilePos;
              PosCache.Param[1]=LeftPos;
              PosCache.Param[2]=VM.Hex;
              //=PosCache.Param[3];
              PosCache.Param[4]=Table;
              if(Opt.ViOpt.SaveViewerShortPos)
              {
                PosCache.Position[0]=BMSavePos.SavePosAddr;
                PosCache.Position[1]=(__int64*)BMSavePos.SavePosLeft;
                //PosCache.Position[2]=;
                //PosCache.Position[3]=;
              }
              CtrlObject->ViewerPosCache->AddPosition(strCacheName,&PosCache);
            }
          }
          if ( PointToName(strName) == strName )
          {
              string strViewDir;

              ViewNamesList.GetCurDir (strViewDir);

              if ( !strViewDir.IsEmpty() )
                  FarChDir(strViewDir);
          }

          if ( OpenFile(strName, TRUE) )
          {
            SecondPos=0;
            Show();
          }

          ShowConsoleTitle();
        }
      }
      return(TRUE);
    }

    case KEY_SHIFTF2:
    {
      ProcessTypeWrapMode(!VM.WordWrap);
      return TRUE;
    }

    case KEY_F2:
    {
      ProcessWrapMode(!VM.Wrap);
      return(TRUE);
    }

    case KEY_F4:
    {
      ProcessHexMode(!VM.Hex);
      return(TRUE);
    }

    case KEY_F7:
    {
      Search(0,0);
      return(TRUE);
    }

    case KEY_SHIFTF7:
    case KEY_SPACE:
    {
      Search(1,0);
      return(TRUE);
    }

    case KEY_ALTF7:
    {
      SearchFlags.Set(REVERSE_SEARCH);
      Search(1,0);
      SearchFlags.Clear(REVERSE_SEARCH);
      return(TRUE);
    }

    case KEY_F8:
    {
      if ((VM.AnsiMode=!VM.AnsiMode)!=0)
      {
        int UseUnicode=TRUE;
        GetTable(&TableSet,TRUE,VM.TableNum,UseUnicode);
      }
      if (VM.Unicode)
      {
        FilePos*=2;
        VM.Unicode=FALSE;
        SetFileSize();

        SelectPos = 0;
        SelectSize = 0;
      }
      VM.TableNum=0;
      VM.UseDecodeTable=VM.AnsiMode;
      ChangeViewKeyBar();
      Show();
//      LastSelPos=FilePos;
      TableChangedByUser=TRUE;
      return(TRUE);
    }

    case KEY_SHIFTF8:
    {
      {
        int UseUnicode=TRUE;
        int GetTableCode=GetTable(&TableSet,FALSE,VM.TableNum,UseUnicode);
        if (GetTableCode!=-1)
        {
          /* $ 08.03.2003 IS
               Заново определим символы конца строки,
               т.к. они другие при изменении
               unicode<->однобайтовая кодировка
          */
          BOOL oldIsUnicode=VM.Unicode;
          if (VM.Unicode && !UseUnicode)
            FilePos*=2;
          if (!VM.Unicode && UseUnicode)
            FilePos=(FilePos+(FilePos&1))/2;
          VM.UseDecodeTable=GetTableCode;
          VM.Unicode=UseUnicode;

          if ( !oldIsUnicode && VM.Unicode )
          {
            SelectPos = 0;
            SelectSize = 0;
          }

          SetFileSize();
          VM.AnsiMode=FALSE;
          ChangeViewKeyBar();
          Show();
//          LastSelPos=FilePos;
          TableChangedByUser=TRUE;
          // IS: определяем символы конца строки только,
          // IS: если включили или выключили юникод
          if((oldIsUnicode && !VM.Unicode) || (!oldIsUnicode && VM.Unicode))
            SetCRSym();
          /* IS $ */
        }
      }
      return(TRUE);
    }

    case KEY_ALTF8:
    {
      if(ViewFile)
        GoTo();
      return(TRUE);
    }

    case KEY_F11:
    {
      CtrlObject->Plugins.CommandsMenu(MODALTYPE_VIEWER,0,L"Viewer");
      Show();
      return(TRUE);
    }

    /* $ 27.06.2001 VVM
      + С альтом скролим по 1 */
    /* $ 27.04.2001 VVM
      + Обработка KEY_MSWHEEL_XXXX */
    case KEY_MSWHEEL_UP:
    case (KEY_MSWHEEL_UP | KEY_ALT):
    {
      int Roll = Key & KEY_ALT?1:Opt.MsWheelDeltaView;
      for (int i=0; i<Roll; i++)
        ProcessKey(KEY_UP);
      return(TRUE);
    }

    case KEY_MSWHEEL_DOWN:
    case (KEY_MSWHEEL_DOWN | KEY_ALT):
    {
      int Roll = Key & KEY_ALT?1:Opt.MsWheelDeltaView;
      for (int i=0; i<Roll; i++)
        ProcessKey(KEY_DOWN);
      return(TRUE);
    }
    /* VVM $ */
    /* VVM $ */

    case KEY_UP: case KEY_NUMPAD8: case KEY_SHIFTNUMPAD8:
    {
      if (FilePos>0 && ViewFile)
      {
        Up();
        if (VM.Hex)
        {
          FilePos&=~(VM.Unicode ? 0x7:0xf);
          Show();
        }
        else
          ShowPage(SHOW_UP);
      }
//      LastSelPos=FilePos;
      return(TRUE);
    }

    case KEY_DOWN: case KEY_NUMPAD2:  case KEY_SHIFTNUMPAD2:
    {
      if (!LastPage && ViewFile)
      {
        if (VM.Hex)
        {
          FilePos=SecondPos;
          Show();
        }
        else
          ShowPage(SHOW_DOWN);
      }
//      LastSelPos=FilePos;
      return(TRUE);
    }

    case KEY_PGUP: case KEY_NUMPAD9: case KEY_SHIFTNUMPAD9: case KEY_CTRLUP:
    {
      if(ViewFile)
      {
        for (I=ViewY1;I<Y2;I++)
          Up();
        Show();
//        LastSelPos=FilePos;
      }
      return(TRUE);
    }

    case KEY_PGDN: case KEY_NUMPAD3:  case KEY_SHIFTNUMPAD3: case KEY_CTRLDOWN:
    {
      vString.lpData = new wchar_t[MAX_VIEWLINEB];

      if (LastPage || ViewFile==NULL)
        return(TRUE);
      vseek(ViewFile,FilePos,SEEK_SET);
      for (I=ViewY1;I<Y2;I++)
      {
        ReadString(&vString,-1, MAX_VIEWLINEB);
        if (LastPage)
          return(TRUE);
      }
      FilePos=vtell(ViewFile);
      for (I=ViewY1;I<=Y2;I++)
        ReadString(&vString,-1, MAX_VIEWLINEB);
      /* $ 02.06.2003 VVM
        + Старое поведение оставим на Ctrl-Down */
      /* $ 21.05.2003 VVM
        + По PgDn листаем всегда по одной странице,
          даже если осталась всего одна строчка.
          Удобно тексты читать */
      if (LastPage && Key == KEY_CTRLDOWN)
      {
        InternalKey++;
        ProcessKey(KEY_CTRLPGDN);
        InternalKey--;
        return(TRUE);
      }
      /* VVM $ */
      /* VVM $ */
      Show();

      delete [] vString.lpData;
//      LastSelPos=FilePos;
      return(TRUE);
    }

    case KEY_LEFT: case KEY_NUMPAD4: case KEY_SHIFTNUMPAD4:
    {
      if (LeftPos>0 && ViewFile)
      {
        if (VM.Hex && LeftPos>80-Width)
          LeftPos=Max(80-Width,1);
        LeftPos--;
        Show();
      }
//      LastSelPos=FilePos;
      return(TRUE);
    }

    case KEY_RIGHT: case KEY_NUMPAD6: case KEY_SHIFTNUMPAD6:
    {
      if (LeftPos<MAX_VIEWLINE && ViewFile && !VM.Hex)
      {
        LeftPos++;
        Show();
      }
//      LastSelPos=FilePos;
      return(TRUE);
    }

    case KEY_CTRLLEFT: case KEY_CTRLNUMPAD4:
    {
      if(ViewFile)
      {
        if(VM.Hex)
        {
          FilePos--;
          if (FilePos<0)
            FilePos=0;
        }
        else
        {
          LeftPos-=20;
          if (LeftPos<0)
            LeftPos=0;
        }
        Show();
//        LastSelPos=FilePos;
      }
      return(TRUE);
    }

    case KEY_CTRLRIGHT: case KEY_CTRLNUMPAD6:
    {
      if(ViewFile)
      {
        if(VM.Hex)
        {
          FilePos++;
          if (FilePos >= FileSize)
            FilePos=FileSize-1; //??
        }
        else
        {
          LeftPos+=20;
          if (LeftPos>MAX_VIEWLINE)
            LeftPos=MAX_VIEWLINE;
        }
        Show();
//        LastSelPos=FilePos;
      }
      return(TRUE);
    }

    case KEY_CTRLSHIFTLEFT:    case KEY_CTRLSHIFTNUMPAD4:
      // Перейти на начало строк
      if (ViewFile)
      {
        LeftPos = 0;
        Show();
      }
      return(TRUE);
    case KEY_CTRLSHIFTRIGHT:     case KEY_CTRLSHIFTNUMPAD6:
    {
        // Перейти на конец строк
        if (ViewFile)
        {
          int I, Y, Len, MaxLen = 0;
          for (I=0,Y=ViewY1;Y<=Y2;Y++,I++)
          {
             Len = (int)wcslen(Strings[I]->lpData);
             if (Len > MaxLen)
               MaxLen = Len;
          } /* for */
          if (MaxLen > Width)
            LeftPos = MaxLen - Width;
          else
            LeftPos = 0;
          Show();
        } /* if */
        return(TRUE);
    }

    case KEY_CTRLHOME:    case KEY_CTRLNUMPAD7:
    case KEY_HOME:        case KEY_NUMPAD7:   case KEY_SHIFTNUMPAD7:
      // Перейти на начало файла
      if(ViewFile)
        LeftPos=0;
    case KEY_CTRLPGUP:    case KEY_CTRLNUMPAD9:
      if(ViewFile)
      {
        FilePos=0;
        Show();
//        LastSelPos=FilePos;
      }
      return(TRUE);

    case KEY_CTRLEND:     case KEY_CTRLNUMPAD1:
    case KEY_END:         case KEY_NUMPAD1: case KEY_SHIFTNUMPAD1:
      // Перейти на конец файла
      if(ViewFile)
        LeftPos=0;
    case KEY_CTRLPGDN:    case KEY_CTRLNUMPAD3:
      if(ViewFile)
      {
        /* $ 15.08.2002 IS
           Для обычного режима, если последняя строка не содержит перевод
           строки, крутанем вверх на один раз больше - иначе визуально
           обработка End (и подобных) на такой строке отличается от обработки
           Down.
        */
        unsigned int max_counter=Y2-ViewY1;
        if(VM.Hex)
          vseek(ViewFile,0,SEEK_END);
        else
        {
          vseek(ViewFile,-1,SEEK_END);
          int LastSym=vgetc(ViewFile);
          if(LastSym!=EOF && LastSym!=CRSym)
            ++max_counter;
        }
        FilePos=vtell(ViewFile);
/*
        {
          char Buf[100];
          sprintf(Buf,"%I64X",FilePos);
          Message(0,1,"End",Buf,"Ok");
        }
*/
        for (I=0;static_cast<unsigned int>(I)<max_counter;I++)
          Up();
        /* IS 15.08.2002 $ */
/*
        {
          char Buf[100];
          sprintf(Buf,"%I64X, %d",FilePos, I);
          Message(0,1,"Up",Buf,"Ok");
        }
*/
        if (VM.Hex)
          FilePos&=~(VM.Unicode ? 0x7:0xf);
/*
        if (VM.Hex)
        {
          char Buf[100];
          sprintf(Buf,"%I64X",FilePos);
          Message(0,1,"VM.Hex",Buf,"Ok");
        }
*/
        Show();
//        LastSelPos=FilePos;
      }
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
/* IS $ */

int Viewer::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  if ((MouseEvent->dwButtonState & 3)==0)
    return(FALSE);

  /* $ 18.07.2000 tran
     просто для сокращения кода*/
  int MsX=MouseEvent->dwMousePosition.X;
  int MsY=MouseEvent->dwMousePosition.Y;
  /* tran 18.07.2000 $ */

  /* $ 22.01.2001 IS
       Происходят какие-то манипуляции -> снимем выделение
  */
//  SelectSize=0;
  /* IS $ */

  /* $ 18.07.2000 tran
     обработка сколбара */
  /* $ 10.09.2000 SVS
     ! Постоянный скроллинг при нажатой клавише
       Обыкновенный захват мыши
  */
  /* $ 02.10.2000 SVS
    > Если нажать в самом низу скролбара, вьюер отмотается на страницу
    > ниже нижней границы текста. Перед глазами будет пустой экран.
  */
  if ( ViOpt.ShowScrollbar && MsX==X2)
  {
    /* $ 01.09.2000 SVS
       Небольшая бага с тыканием в верхнюю позицию ScrollBar`а
    */
    if (MsY == Y1+1)
      while (IsMouseButtonPressed())
        ProcessKey(KEY_UP);
    /* SVS $*/
    else if (MsY==Y2)
    {
      while (IsMouseButtonPressed())
      {
//        _SVS(SysLog(L"Viewer/ KEY_DOWN= %i, %i",FilePos,FileSize));
        ProcessKey(KEY_DOWN);
      }
    }
    else if(MsY == Y1+2)
      ProcessKey(KEY_CTRLHOME);
    else if(MsY == Y2-1)
      ProcessKey(KEY_CTRLEND);
    else
    {
      INPUT_RECORD rec;
      while (IsMouseButtonPressed())
      {
        /* $ 14.05.2001 DJ
           более точное позиционирование; корректная работа на больших файлах
        */
        FilePos=(FileSize-1)/(Y2-ViewY1-1)*(MsY-ViewY1);
        /* DJ $ */
        int Perc;
        if(FilePos > FileSize)
        {
          FilePos=FileSize;
          Perc=100;
        }
        else if(FilePos < 0)
        {
          FilePos=0;
          Perc=0;
        }
        else
          Perc=ToPercent64(FilePos,FileSize);
//_SVS(SysLog(L"Viewer/ ToPercent()=%i, %I64d, %I64d, Mouse=[%d:%d]",Perc,FilePos,FileSize,MsX,MsY));
        if(Perc == 100)
          ProcessKey(KEY_CTRLEND);
        else if(!Perc)
          ProcessKey(KEY_CTRLHOME);
        else
        {
          /* $ 27.04.2001 DJ
             не рвем строки посередине
          */
          AdjustFilePos();
          Show();
          /* DJ $ */
        }
        GetInputRecord(&rec);
        MsX=rec.Event.MouseEvent.dwMousePosition.X;
        MsY=rec.Event.MouseEvent.dwMousePosition.Y;
      }
    }
    return (TRUE);
  }
  /* SVS 02.10.2000 $ */
  /* SVS $*/
  /* tran 18.07.2000 $ */

  /* $ 16.12.2000 tran
     шелчок мышью на статус баре */
  /* $ 12.10.2001 SKV
    угу, а только если он нсть, statusline...
  */
  if ( MsY==Y1 && ShowStatusLine) // Status line
  /* SKV$*/
  {
    int XTable, XPos, NameLength;
    NameLength=ObjWidth-40;
    if (Opt.ViewerEditorClock && HostFileViewer!=NULL && HostFileViewer->IsFullScreen())
      NameLength-=6;
    if (NameLength<20)
      NameLength=20;
    XTable=NameLength+1;
    XPos=NameLength+1+10+1+10+1;

    while(IsMouseButtonPressed());

    MsX=MouseX;
    MsY=MouseY;

    if (MsY!=Y1)
      return(TRUE);

    //_D(SysLog(L"MsX=%i, XTable=%i, XPos=%i",MsX,XTable,XPos));
    if ( MsX>=XTable && MsX<=XTable+10 )
    {
        ProcessKey(KEY_SHIFTF8);
        return (TRUE);
    }
    if ( MsX>=XPos && MsX<=XPos+7+1+4+1+3 )
    {
        ProcessKey(KEY_ALTF8);
        return (TRUE);
    }
  }
  /* tran $ */
  if (MsX<X1 || MsX>X2 || MsY<ViewY1 || MsY>Y2)
    return(FALSE);

  if (MsX<X1+7)
    while (IsMouseButtonPressed() && MouseX<X1+7)
      ProcessKey(KEY_LEFT);
  else
    if (MsX>X2-7)
      while (IsMouseButtonPressed() && MouseX>X2-7)
        ProcessKey(KEY_RIGHT);
    else
      if (MsY<ViewY1+(Y2-ViewY1)/2)
        while (IsMouseButtonPressed() && MouseY<ViewY1+(Y2-ViewY1)/2)
          ProcessKey(KEY_UP);
      else
        while (IsMouseButtonPressed() && MouseY>=ViewY1+(Y2-ViewY1)/2)
          ProcessKey(KEY_DOWN);
  return(TRUE);
}

void Viewer::Up()
{
  if(!ViewFile)
    return;

  wchar_t Buf[MAX_VIEWLINE];
  int BufSize,StrPos,Skipped,I,J;

  if(FilePos > (__int64)sizeof(Buf)/sizeof (wchar_t))
    BufSize=sizeof(Buf)/sizeof (wchar_t);
  else
    BufSize=(int)FilePos;

  if (BufSize==0)
    return;
  LastPage=0;
  if (VM.Hex)
  {
    int UpSize=VM.Unicode ? 8:16;
    if (FilePos<(__int64)UpSize)
      FilePos=0;
    else
      FilePos-=UpSize;
    return;
  }
  vseek(ViewFile,FilePos-(__int64)BufSize,SEEK_SET);
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
    /* $ 29.11.2001 DJ
       не обращаемся за границу массива (а надо было всего лишь поменять местами условия...)
    */
    if (I==-1 || Buf[I]==CRSym)   /* DJ $ */
      if (!VM.Wrap)
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
          if (StrPos==0 || StrPos >= Width)
          {
            if (J==BufSize)
            {
              if (Skipped==0)
                FilePos--;
              else
                FilePos-=Skipped;
              return;
            }
            if (CalcStrSize(&Buf[J],BufSize-J) <= Width)
            {
              FilePos-=BufSize-J+Skipped;
              return;
            }
            else
              StrPos=0;
          }
          if (J<BufSize)
            if (Buf[J]=='\t')
              StrPos+=ViOpt.TabSize-(StrPos % ViOpt.TabSize);
            else
              if (Buf[J]!=13)
                StrPos++;
        }
      }
  }
  for (I=Min(Width,BufSize);I>0;I-=5)
    if (CalcStrSize(&Buf[BufSize-I],I) <= Width)
    {
      FilePos-=I+Skipped;
      break;
    }
}


int Viewer::CalcStrSize(const wchar_t *Str,int Length)
{
  int Size,I;
  for (Size=0,I=0;I<Length;I++)
    switch(Str[I])
    {
      case L'\t':
        Size+=ViOpt.TabSize-(Size % ViOpt.TabSize);
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
    /* $ 12.07.2000 SVS
       Wrap имеет 3 позиции
    */
    /* $ 15.07.2000 SVS
       Wrap должен показываться следующий, а не текущий
    */
    ViewKeyBar->Change(
       UMSG(
       (!VM.Wrap)?((!VM.WordWrap)?MViewF2:MViewShiftF2)
       :MViewF2Unwrap),1);
    ViewKeyBar->Change(KBL_SHIFT,UMSG((VM.WordWrap)?MViewF2:MViewShiftF2),1);
    /* SVS $ */
    /* SVS $ */

    if (VM.Hex)
      ViewKeyBar->Change(UMSG(MViewF4Text),3);
    else
      ViewKeyBar->Change(UMSG(MViewF4),3);

    if (VM.AnsiMode)
      ViewKeyBar->Change(UMSG(MViewF8DOS),7);
    else
      ViewKeyBar->Change(UMSG(MViewF8),7);

    ViewKeyBar->Redraw();
  }
  struct ViewerMode vm;
  memmove(&vm,&VM,sizeof(struct ViewerMode));
  CtrlObject->Plugins.CurViewer=this; //HostFileViewer;
//  CtrlObject->Plugins.ProcessViewerEvent(VE_MODE,&vm);
}

LONG_PTR WINAPI ViewerSearchDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
  /* 23.09.2003 KM */
  Dialog* Dlg=(Dialog*)hDlg;
  char DataStr[NM*2]; //BUGBUG
  struct FarDialogItem Item;

  switch(Msg)
  {
    case DN_INITDIALOG:
    {
      /* $ 22.09.2003 KM
         Переключение видимости строки ввода искомого текста
         в зависимости от Dlg->Item[6].Selected
      */
      Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,6,(LONG_PTR)&Item);

      if (Item.Param.Selected)
      {
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,2,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,3,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,7,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,FALSE);
      }
      else
      {
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,2,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,3,FALSE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,7,TRUE);
        Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,TRUE);
      }

      Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,2,1);
      Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,3,1);
      /* KM $ */

      return TRUE;
    }
    case DN_BTNCLICK:
    {
      if(Param1 == 5 || Param1 == 6)
      {
        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,FALSE,0);

        /* $ 26.10.2003 KM */
        /* $ 22.09.2003 KM
           Переключение видимости строки ввода искомого текста
           в зависимости от установленного режима hex search
        */
        int LenDataStr=sizeof(DataStr);
        if (Param1 == 6 && Param2)
        {
          Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,2,(LONG_PTR)&Item);

          //Transform((unsigned char *)DataStr,LenDataStr,Item.Data.Data,'X'); //BUGBUG
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,3,(LONG_PTR)DataStr);

          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,2,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,3,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,7,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,FALSE);

          if (strlen(DataStr)>0)
          {
            int UnchangeFlag=(int)Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,2,-1);
            Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,3,UnchangeFlag);
          }
        }

        if (Param1 == 5 && Param2)
        {
          Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,3,(LONG_PTR)&Item);
          //Transform((unsigned char *)DataStr,LenDataStr,Item.Data.Data,'S'); //BUGBUG
          Dialog::SendDlgMessage(hDlg,DM_SETTEXTPTR,2,(LONG_PTR)DataStr);

          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,2,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_SHOWITEM,3,FALSE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,7,TRUE);
          Dialog::SendDlgMessage(hDlg,DM_ENABLE,8,TRUE);

          if (strlen(DataStr)>0)
          {
            int UnchangeFlag=(int)Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,3,-1);
            Dialog::SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,2,UnchangeFlag);
          }
        }
        /* KM $ */
        /* KM $ */

        Dialog::SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
        return TRUE;
      }
    }
    /* $ 15.10.2003 KM
        Обработаем "горячие" клавиши
    */
    case DN_HOTKEY:
    {
      if (Param1==1)
      {
        Dialog::SendDlgMessage(hDlg,DM_GETDLGITEM,6,(LONG_PTR)&Item);

        if (Item.Param.Selected)
          Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,3,0);
        else
          Dialog::SendDlgMessage(hDlg,DM_SETFOCUS,2,0);
        return FALSE;
      }
    }
    /* KM $ */
  }
  /* KM $ */
  return Dialog::DefDlgProc(hDlg,Msg,Param1,Param2);
}

static void PR_ViewerSearchMsg(void)
{
  ViewerSearchMsg((const wchar_t*)PreRedrawParam.Param1);
}

void ViewerSearchMsg(const wchar_t *MsgStr)
{
  /* $ 23.09.2003 KM */
  Message(0,0,UMSG(MViewSearchTitle),(SearchHex?UMSG(MViewSearchingHex):UMSG(MViewSearchingFor)),MsgStr);
  /* KM $ */
  PreRedrawParam.Param1=(void*)MsgStr;
}

/* $ 27.01.2003 VVM
   + Параметр Next может принимать значения:
   0 - Новый поиск
   1 - Продолжить поиск со следующей позиции
   2 - Продолжить поиск с начала файла
*/
void Viewer::Search(int Next,int FirstChar)
{
  const wchar_t *TextHistoryName=L"SearchText";
  const wchar_t *HexMask=L"HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH ";
  static struct DialogDataEx SearchDlgData[]={
  /* 00 */ DI_DOUBLEBOX,3,1,72,10,0,0,0,0,(const wchar_t *)MViewSearchTitle,
  /* 01 */ DI_TEXT,5,2,0,2,0,0,0,0,(const wchar_t *)MViewSearchFor,
  /* 02 */ DI_EDIT,5,3,70,3,1,(DWORD_PTR)TextHistoryName,DIF_HISTORY|DIF_USELASTHISTORY,0,L"",
  /* 03 */ DI_FIXEDIT,5,3,70,3,0,(DWORD_PTR)HexMask,DIF_MASKEDIT,0,L"",
  /* 04 */ DI_TEXT,3,4,0,4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 05 */ DI_RADIOBUTTON,5,5,0,5,0,1,DIF_GROUP,0,(const wchar_t *)MViewSearchForText,
  /* 06 */ DI_RADIOBUTTON,5,6,0,6,0,0,0,0,(const wchar_t *)MViewSearchForHex,
  /* 07 */ DI_CHECKBOX,40,5,0,5,0,0,0,0,(const wchar_t *)MViewSearchCase,
  /* 08 */ DI_CHECKBOX,40,6,0,6,0,0,0,0,(const wchar_t *)MViewSearchWholeWords,
  /* 09 */ DI_CHECKBOX,40,7,0,7,0,0,0,0,(const wchar_t *)MViewSearchReverse,
  /* 10 */ DI_TEXT,3,8,0,8,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
  /* 11 */ DI_BUTTON,0,9,0,9,0,0,DIF_CENTERGROUP,1,(const wchar_t *)MViewSearchSearch,
  /* 12 */ DI_BUTTON,0,9,0,9,0,0,DIF_CENTERGROUP,0,(const wchar_t *)MViewSearchCancel
  };
  MakeDialogItemsEx(SearchDlgData,SearchDlg);

  string strSearchStr;
  string strMsgStr;
  __int64 MatchPos=0;
  int SearchLength,Case,WholeWords,ReverseSearch,Match;

  if (ViewFile==NULL || Next && strLastSearchStr.IsEmpty())
    return;

  if ( !strLastSearchStr.IsEmpty() )
    strSearchStr = strLastSearchStr;
  else
    strSearchStr=L"";

  SearchDlg[5].Selected=!LastSearchHex;
  SearchDlg[6].Selected=LastSearchHex;
  SearchDlg[7].Selected=LastSearchCase;

  SearchDlg[8].Selected=LastSearchWholeWords;
  SearchDlg[9].Selected=LastSearchReverse;

  if (SearchFlags.Check(REVERSE_SEARCH))
    SearchDlg[9].Selected = !SearchDlg[9].Selected;

  if (VM.Unicode)
  {
    SearchDlg[5].Selected=TRUE;
    SearchDlg[6].Flags|=DIF_DISABLE;
    SearchDlg[6].Selected=FALSE;
  }

  if(SearchDlg[6].Selected)
    SearchDlg[7].Flags|=DIF_DISABLE;

  if(SearchDlg[6].Selected)
    SearchDlg[3].strData = strSearchStr;
  else
    SearchDlg[2].strData = strSearchStr;

  if (!Next)
  {
    SearchFlags.Flags = 0;
    Dialog Dlg(SearchDlg,sizeof(SearchDlg)/sizeof(SearchDlg[0]),ViewerSearchDlgProc);
    Dlg.SetPosition(-1,-1,76,12);
    Dlg.SetHelp(L"ViewerSearch");
    if (FirstChar)
    {
      Dlg.InitDialog();
      Dlg.Show();
      Dlg.ProcessKey(FirstChar);
    }
    Dlg.Process();
    if (Dlg.GetExitCode()!=11)
      return;
  }

  SearchHex=SearchDlg[6].Selected;
  Case=SearchDlg[7].Selected;
  WholeWords=SearchDlg[8].Selected;
  ReverseSearch=SearchDlg[9].Selected;

  if(SearchHex)
  {
    strSearchStr = SearchDlg[3].strData;
    RemoveTrailingSpaces(strSearchStr);
  }
  else
    strSearchStr = SearchDlg[2].strData;

  strLastSearchStr = strSearchStr;
  LastSearchHex=SearchHex;
  LastSearchCase=Case;
  LastSearchWholeWords=WholeWords;
  if (!SearchFlags.Check(REVERSE_SEARCH))
    LastSearchReverse=ReverseSearch;

  if ((SearchLength=(int)strSearchStr.GetLength ())==0)
    return;

  {
    //SaveScreen SaveScr;
    SetCursorType(FALSE,0);

    strMsgStr = strSearchStr;

    if(strMsgStr.GetLength()+18 > static_cast<DWORD>(ObjWidth))
      TruncStrFromEnd(strMsgStr, ObjWidth-18);
    InsertQuote(strMsgStr);

    SetPreRedrawFunc(PR_ViewerSearchMsg);
    ViewerSearchMsg(strMsgStr);

    /* $ 26.10.2003 KM */
    if (SearchHex)
    {
      /*unsigned char HexStr[NM*2];
      int LenHexStr=sizeof(HexStr);

      Transform(HexStr,LenHexStr,(char *)SearchStr,'S');
      SearchLength=LenHexStr;

      memmove(SearchStr,HexStr,sizeof(SearchStr));*/ //BUGBUG
    }
    /* KM $ */

    if (!Case && !SearchHex)
      strSearchStr.Upper ();
//      for (int I=0;I<SearchLength;I++)
//        SearchStr[I]=LocalUpper(SearchStr[I]);

    SelectSize = 0;
    if (Next)
    {
      if (Next == 2)
      {
        SearchFlags.Set(SEARCH_MODE2);
        LastSelPos = ReverseSearch?FileSize:0;
      }
      else
        LastSelPos = SelectPos + (ReverseSearch?-1:1);
    }
    else
    {
      LastSelPos = FilePos;
      if (LastSelPos == 0 || LastSelPos == FileSize)
        SearchFlags.Set(SEARCH_MODE2);
    }

    vseek(ViewFile,LastSelPos,SEEK_SET);
    Match=0;
    if (SearchLength>0 && (!ReverseSearch || LastSelPos>0))
    {
      wchar_t Buf[8192];
      /* $ 01.08.2000 KM
         Изменён тип CurPos с unsigned long на long
         из-за того, что дальше шла проверка при вычитании
         на -1, а CurPos не мог стать отрицательным и иногда
         выдавался неверный результат
      */
                             // BugZ#1097 - невозможность поиска вхождений в юникод текстовом файле
      __int64 CurPos=LastSelPos+(VM.Unicode && !ReverseSearch?1:0); // добавка к юникоду (+1), т.к. нихрена не ищет в первой строке.
                             //^^^^^^^^^^^^^^^^^  ?????????

      /* KM $ */
      int BufSize=sizeof(Buf)/sizeof (wchar_t);
      if (ReverseSearch)
      {
        /* $ 01.08.2000 KM
           Изменёно вычисление CurPos с учётом Whole words
        */
        if (WholeWords)
          CurPos-=sizeof(Buf)/sizeof (wchar_t)-SearchLength+1;
        else
          CurPos-=sizeof(Buf)/sizeof (wchar_t)-SearchLength;
        /* KM $ */
        if (CurPos<0)
          BufSize+=(int)CurPos;
      }
      int ReadSize;
      while (!Match)
      {
        /* $ 01.08.2000 KM
           Изменена строка if (ReverseSearch && CurPos<0) на if (CurPos<0),
           так как при обычном прямом и LastSelPos=0xFFFFFFFF, поиск
           заканчивался так и не начавшись.
        */
        if (CurPos<0)
          CurPos=0;
        /* KM $ */

        vseek(ViewFile,CurPos,SEEK_SET);
        if ((ReadSize=vread(Buf,BufSize,ViewFile))<=0)
          break;

        if(CheckForEscSilent())
        {
          if (ConfirmAbortOp())
          {
            SetPreRedrawFunc(NULL);
            Redraw ();
            return;
          }
          ViewerSearchMsg(strMsgStr);
        }

        if (VM.UseDecodeTable && !SearchHex && !VM.Unicode)
          for (int I=0;I<ReadSize;I++)
            Buf[I]=TableSet.DecodeTable[Buf[I]];

        /* $ 01.08.2000 KM
           Сделана сразу проверка на Case sensitive и Hex
           и если нет, тогда Buf приводится к верхнему регистру
        */
        if (!Case && !SearchHex)
          CharUpperBuffW (Buf,ReadSize);
        /* KM $ */

        /* $ 01.08.2000 KM
           Убран кусок текста после приведения поисковой строки
           и Buf к единому регистру, если поиск не регистрозависимый
           или не ищется Hex-строка и в связи с этим переработан код поиска
        */
        int MaxSize=ReadSize-SearchLength+1;
        int Increment=ReverseSearch ? -1:+1;
        for (int I=ReverseSearch ? MaxSize-1:0;I<MaxSize && I>=0;I+=Increment)
        {
          /* $ 01.08.2000 KM
             Обработка поиска "Whole words"
          */
          /* $ 26.05.2002 KM
              Исправлены ошибки в поиске по целым словам.
          */
          int locResultLeft=FALSE;
          int locResultRight=FALSE;

          if (WholeWords)
          {
            if (I!=0)
            {
              if (IsSpace(Buf[I-1]) || IsEol(Buf[I-1]) ||
                 (wcschr(Opt.strWordDiv,Buf[I-1])!=NULL))
                locResultLeft=TRUE;
            }
            else
            {
              locResultLeft=TRUE;
            }

            if (ReadSize!=BufSize && I+SearchLength>=ReadSize)
              locResultRight=TRUE;
            else
              if (I+SearchLength<ReadSize &&
                 (IsSpace(Buf[I+SearchLength]) || IsEol(Buf[I+SearchLength]) ||
                 (wcschr(Opt.strWordDiv,Buf[I+SearchLength])!=NULL)))
                locResultRight=TRUE;
          }
          else
          {
            locResultLeft=TRUE;
            locResultRight=TRUE;
          }

          Match=locResultLeft && locResultRight && strSearchStr.At(0)==Buf[I] &&
            (SearchLength==1 || strSearchStr.At(1)==Buf[I+1] &&
            (SearchLength==2 || memcmp((const wchar_t*)strSearchStr+2,&Buf[I+2],(SearchLength-2)*sizeof (wchar_t))==0));

          if (Match)
          {
            MatchPos=CurPos+I;
            break;
          }
          /* KM $ */
          /* KM $ */
        }
        /* KM $ */

        if ((ReverseSearch && CurPos <= 0) || (!ReverseSearch && ReadSize < BufSize))
          break;
        if (ReverseSearch)
        {
          /* $ 01.08.2000 KM
             Изменёно вычисление CurPos с учётом Whole words
          */
          if (WholeWords)
            CurPos-=sizeof(Buf)/sizeof (wchar_t)-SearchLength+1;
          else
            CurPos-=sizeof(Buf)/sizeof (wchar_t)-SearchLength;
        }
        else
        {
          if (WholeWords)
            CurPos+=sizeof(Buf)/sizeof (wchar_t)-SearchLength+1;
          else
            CurPos+=sizeof(Buf)/sizeof (wchar_t)-SearchLength;
        }
        /* KM $ */
      }
    }
  }

  SetPreRedrawFunc(NULL);

  if (Match)
  {
    /* $ 24.01.2003 KM
       ! По окончании поиска отступим от верха экрана на
         треть отображаемой высоты.
    */
    SelectText(MatchPos,SearchLength,ReverseSearch?0x2:0);

    // Покажем найденное на расстоянии трети экрана от верха.
    int FromTop=(ScrY-(Opt.ViOpt.ShowKeyBar?2:1))/4;
    if (FromTop<0 || FromTop>ScrY)
      FromTop=0;

    for (int i=0;i<FromTop;i++)
      Up();

    AdjustSelPosition = TRUE;
    Show();
    AdjustSelPosition = FALSE;
    /* KM $ */
  }
  else
  {
    //Show();
    /* $ 27.01.2003 VVM
       + После окончания поиска спросим о переходе поиска в начало/конец */
    if (SearchFlags.Check(SEARCH_MODE2))
      Message(MSG_DOWN|MSG_WARNING,1,UMSG(MViewSearchTitle),
        (SearchHex?UMSG(MViewSearchCannotFindHex):UMSG(MViewSearchCannotFind)),strMsgStr,UMSG(MOk));
    else
    {
      if (Message(MSG_DOWN|MSG_WARNING,2,UMSG(MViewSearchTitle),
            (SearchHex?UMSG(MViewSearchCannotFindHex):UMSG(MViewSearchCannotFind)),strMsgStr,
            (ReverseSearch?UMSG(MViewSearchFromEnd):UMSG(MViewSearchFromBegin)),
             UMSG(MHYes),UMSG(MHNo)) == 0)
        Search(2,0);
    }
    /* VVM $ */
  }
}


/*void Viewer::ConvertToHex(char *SearchStr,int &SearchLength)
{
  char OutStr[512],*SrcPtr;
  int OutPos=0,N=0;
  SrcPtr=SearchStr;
  while (*SrcPtr)
  {
    while (IsSpaceA(*SrcPtr))
      SrcPtr++;
    if (SrcPtr[0])
      if (SrcPtr[1]==0 || IsSpaceA(SrcPtr[1]))
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
}*/


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
  return(VM.Wrap);
}


void Viewer::SetWrapMode(int Wrap)
{
  Viewer::VM.Wrap=Wrap;
}


void Viewer::EnableHideCursor(int HideCursor)
{
  Viewer::HideCursor=HideCursor;
}


int Viewer::GetWrapType()
{
  return(VM.WordWrap);
}


void Viewer::SetWrapType(int TypeWrap)
{
  Viewer::VM.WordWrap=TypeWrap;
}


void Viewer::GetFileName(string &strName)
{
    strName = strFullFileName;
}


void Viewer::ShowConsoleTitle()
{
    string strTitle;
    strTitle.Format (UMSG(MInViewer), PointToName(strFileName));

    SetFarTitle(strTitle);
}


void Viewer::SetTempViewName(const wchar_t *Name, BOOL DeleteFolder)
{
  if(Name && *Name)
    ConvertNameToFull(Name,strTempViewName);
  else
  {
    strTempViewName = L"";
    DeleteFolder=FALSE;
  }
  Viewer::DeleteFolder=DeleteFolder;
}


void Viewer::SetTitle(const wchar_t *Title)
{
  if ( Title==NULL )
      strTitle = L"";
  else
      strTitle = Title;
}


/* $ 18.07.2000 tran
   * changes 'long' to 'unsigned long' */
void Viewer::SetFilePos(__int64 Pos)
{
  FilePos=Pos;
};


void Viewer::SetPluginData(const wchar_t *PluginData)
{
  Viewer::strPluginData = NullToEmpty(PluginData);
}


void Viewer::SetNamesList(NamesList *List)
{
  if (List!=NULL)
    List->MoveData(ViewNamesList);
}


int Viewer::vread(wchar_t *Buf,int Count,FILE *SrcFile)
{
  if(!SrcFile)
    return -1;

  if (VM.Unicode)
  {
    // выделяем столько, сколько нужно!
    char *TmpBuf=(char *)alloca(Count*2+16);
    if(!TmpBuf)
      return -1;

    int ReadSize=(int)fread(TmpBuf,1,Count*2,SrcFile);
    TmpBuf[ReadSize]=0;
    /* $ 20.10.2000 tran
       обратный порядок байтов */
    TmpBuf[ReadSize+1]=0;
    if ( FirstWord == 0x0FFFE )
    {
      for (int i=0; i<ReadSize; i+=2 )
      {
        char t=TmpBuf[i];
        TmpBuf[i]=TmpBuf[i+1];
        TmpBuf[i+1]=t;
      }
    }

    memcpy (Buf, TmpBuf, Count*2);

    /* tran $ */
    ReadSize+=(ReadSize & 1);

    return(ReadSize/2);
  }
  else
  {
    char *TmpBuf=(char*)alloca(Count+16);

    int ReadSize=(int)fread(TmpBuf,1,Count,SrcFile);

    MultiByteToWideChar (CP_OEMCP, 0, TmpBuf, ReadSize, Buf, Count);

    return ReadSize;
  }
}


int Viewer::vseek(FILE *SrcFile,__int64 Offset,int Whence)
{
  if(!SrcFile)
    return -1;
  if (VM.Unicode)
    return(fseek64(SrcFile,Offset*2,Whence));
  else
    return(fseek64(SrcFile,Offset,Whence));
}


__int64 Viewer::vtell(FILE *SrcFile)
{
  if(!SrcFile)
    return -1;
  __int64 Pos=ftell64(SrcFile);
  if (VM.Unicode)
    Pos=(Pos+(Pos&1))/2;
  return(Pos);
}


int Viewer::vgetc(FILE *SrcFile)
{
  if(!SrcFile)
    return -1;

  wchar_t Ch;

  if (vread(&Ch, 1,SrcFile)==0)
    return(EOF);

  return Ch;
}


#define RB_PRC 3
#define RB_HEX 4
#define RB_DEC 5

//   ! Изменен вызов функции GoTo() - два дополнительных параметра
//   - Устранение висячих строк при переходе (функция GoTo)
void Viewer::GoTo(int ShowDlg,__int64 Offset, DWORD Flags)
{
  /* $ 17.07.2000 tran
     + new variable*/
  __int64 Relative=0;
  /* tran 17.07.2000 $ */
  const wchar_t *LineHistoryName=L"ViewerOffset";
  static struct DialogDataEx GoToDlgData[]=
  {
    /* 0 */ DI_DOUBLEBOX,3,1,31,7,0,0,0,0,(const wchar_t *)MViewerGoTo,
    /* 1 */ DI_EDIT,5,2,29,2,1,(DWORD_PTR)LineHistoryName,DIF_HISTORY|DIF_USELASTHISTORY,1,L"",
    /* 2 */ DI_TEXT,3,3,0,3,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,L"",
    /* 3 */ DI_RADIOBUTTON,5,4,0,4,0,0,DIF_GROUP,0,(const wchar_t *)MGoToPercent,
    /* 4 */ DI_RADIOBUTTON,5,5,0,5,0,0,0,0,(const wchar_t *)MGoToHex,
    /* 5 */ DI_RADIOBUTTON,5,6,0,6,0,0,0,0,(const wchar_t *)MGoToDecimal,
  };
  MakeDialogItemsEx(GoToDlgData,GoToDlg);
  /* $ 01.08.2000 tran
     с DIF_USELASTHISTORY эот не нужно*/
  //  static char PrevLine[20];
  static int PrevMode=0;

  // strcpy(GoToDlg[1].Data,PrevLine);
  GoToDlg[3].Selected=GoToDlg[4].Selected=GoToDlg[5].Selected=0;
  if ( VM.Hex )
    PrevMode=1;
  GoToDlg[PrevMode+3].Selected=TRUE;

  {
    if(ShowDlg)
    {
      Dialog Dlg(GoToDlg,sizeof(GoToDlg)/sizeof(GoToDlg[0]));
      Dlg.SetHelp(L"ViewerGotoPos");
      Dlg.SetPosition(-1,-1,35,9);
      Dlg.Process();
      /* $ 17.07.2000 tran
         - remove isdigit check()
           кстати, тут баг был
           если ввести ffff при hex offset, то фар все равно никуда не шел */
      /* $ 22.03.2001 IS
         - Переход происходил только тогда, когда в момент закрытия диалога
           курсор находился в строке ввода.
      */
      if (Dlg.GetExitCode()<=0 ) //|| !isdigit(*GoToDlg[1].Data))
        return;
      /* IS $ */
      // xstrncpy(PrevLine,GoToDlg[1].Data,sizeof(PrevLine));
      /* $ 17.07.2000 tran
         тут для сокращения кода ввел ptr, который и анализирую */
      wchar_t *ptr=GoToDlg[1].strData.GetBuffer ();
      if ( ptr[0]==L'+' || ptr[0]==L'-' )     // юзер хочет относительности
      {
          if (ptr[0]==L'+')
              Relative=1;
          else
              Relative=-1;
          memmove(ptr,ptr+1,wcslen(ptr)*sizeof (wchar_t)); // если вы думаете про strlen(ptr)-1,
                                          // то вы ошибаетесь :)
      }
      if ( wcschr(ptr,L'%') )   // он хочет процентов
      {
          GoToDlg[RB_HEX].Selected=GoToDlg[RB_DEC].Selected=0;
          GoToDlg[RB_PRC].Selected=1;
      }
      else if ( LocalStrnicmpW(ptr,L"0x",2)==0 || ptr[0]==L'$' || wcschr(ptr,L'h') || wcschr(ptr,L'H') ) // он умный - hex код ввел!
      {
          GoToDlg[RB_PRC].Selected=GoToDlg[RB_DEC].Selected=0;
          GoToDlg[RB_HEX].Selected=1;
          if ( LocalStrnicmpW(ptr,L"0x",2)==0)
              memmove(ptr,ptr+2,(wcslen(ptr)-1)*sizeof (wchar_t)); // а тут надо -1, а не -2  // сдвинем строку
          else if (ptr[0]=='$')
              memmove(ptr,ptr+1,wcslen(ptr)*sizeof (wchar_t));
          //Relative=0; // при hex значении никаких относительных значений?
      }

      GoToDlg[1].strData.ReleaseBuffer ();

      if (GoToDlg[RB_PRC].Selected)
      {
        //int cPercent=ToPercent64(FilePos,FileSize);
        PrevMode=0;
        int Percent=_wtoi(GoToDlg[1].strData);
        //if ( Relative  && (cPercent+Percent*Relative<0) || (cPercent+Percent*Relative>100)) // за пределы - низя
        //  return;
        if (Percent>100)
          return;
        //if ( Percent<0 )
        //  Percent=0;
        Offset=FileSize/100*Percent;
        if (VM.Unicode)
          Offset*=2;
        while (ToPercent64(Offset,FileSize)<Percent)
          Offset++;
      }
      if (GoToDlg[RB_HEX].Selected)
      {
        PrevMode=1;
        swscanf(GoToDlg[1].strData,L"%I64x",&Offset);
      }
      if (GoToDlg[RB_DEC].Selected)
      {
        PrevMode=2;
        swscanf(GoToDlg[1].strData,L"%I64d",&Offset);
      }
    }// ShowDlg
    else
    {
      Relative=(Flags&VSP_RELATIVE)*(Offset<0?-1:1);
      if(Flags&VSP_PERCENT)
      {
        __int64 Percent=Offset;
        if (Percent>100)
          return;
        //if ( Percent<0 )
        //  Percent=0;
        Offset=FileSize/100*Percent;
        if (VM.Unicode)
          Offset*=2;
        while (ToPercent64(Offset,FileSize)<Percent)
          Offset++;
      }
    }

    if ( Relative )
    {
        if ( Relative==-1 && Offset>FilePos ) // меньше нуля, if (FilePos<0) не пройдет - FilePos у нас unsigned long
            FilePos=0;
        else
            FilePos=VM.Unicode? FilePos+Offset*Relative/2 : FilePos+Offset*Relative;
    }
    else
        FilePos=VM.Unicode ? Offset/2:Offset;
    if ( FilePos>FileSize )   // и куда его несет?
        FilePos=FileSize;     // там все равно ничего нету
    /* tran 17.07.2000 $ */
  }
  // коррекция
  /* $ 27.04.2001 DJ
     коррекция вынесена в отдельную функцию
  */
  AdjustFilePos();
  /* DJ $ */
//  LastSelPos=FilePos;
  if(!(Flags&VSP_NOREDRAW))
    Show();
}


/* $ 27.04.2001 DJ
   корректировка позиции вынесена в отдельную функцию
*/

void Viewer::AdjustFilePos()
{
  if (!VM.Hex)
  {
    wchar_t Buf[4096];
    __int64 StartLinePos=-1,GotoLinePos=FilePos-(__int64)sizeof(Buf)/sizeof (wchar_t);
    if (GotoLinePos<0)
      GotoLinePos=0;
    vseek(ViewFile,GotoLinePos,SEEK_SET);
    int ReadSize=(int)Min((__int64)(sizeof(Buf)/sizeof(wchar_t)),(__int64)(FilePos-GotoLinePos));
    ReadSize=vread(Buf,ReadSize,ViewFile);
    for (int I=ReadSize-1;I>=0;I--)
      if (Buf[I]==CRSym)
      {
        StartLinePos=GotoLinePos+I;
        break;
      }
    vseek(ViewFile,FilePos+1,SEEK_SET);
    if (VM.Hex)
      FilePos&=~(VM.Unicode ? 0x7:0xf);
    else
    {
      if (FilePos!=StartLinePos)
        Up();
    }
  }
}
/* DJ $ */

void Viewer::SetFileSize()
{
  if(!ViewFile)
    return;

  SaveFilePos SavePos(ViewFile);
  vseek(ViewFile,0,SEEK_END);
  FileSize=ftell64(ViewFile);
  /* $ 20.02.2003 IS
     Везде сравниваем FilePos с FileSize, FilePos для юникодных файлов
     уменьшается в два раза, поэтому FileSize тоже надо уменьшать
  */
  if (VM.Unicode)
    FileSize=(FileSize+(FileSize&1))/2;
  /* IS $ */
}


void Viewer::GetSelectedParam(__int64 &Pos, __int64 &Length, DWORD &Flags)
{
  Pos=SelectPos;
  Length=SelectSize;
  Flags=SelectFlags;
}

/* $ 19.01.2001 SVS
   Выделение - в качестве самостоятельной функции.
   Flags=0x01 - показывать (делать Show())
         0x02 - "обратный поиск" ?
*/
void Viewer::SelectText(const __int64 &MatchPos,const __int64 &SearchLength, const DWORD Flags)
{
  if(!ViewFile)
    return;

  wchar_t Buf[1024];
  __int64 StartLinePos=-1,SearchLinePos=MatchPos-sizeof(Buf)/sizeof (wchar_t);
  if (SearchLinePos<0)
    SearchLinePos=0;
  vseek(ViewFile,SearchLinePos,SEEK_SET);
  int ReadSize=(int)Min((__int64)(sizeof(Buf)/sizeof(wchar_t)),(__int64)(MatchPos-SearchLinePos));
  ReadSize=vread(Buf,ReadSize,ViewFile);
  for (int I=ReadSize-1;I>=0;I--)
    if (Buf[I]==CRSym)
    {
      StartLinePos=SearchLinePos+I;
      break;
    }
//MessageBeep(0);
  vseek(ViewFile,MatchPos+1,SEEK_SET);
  SelectPos=FilePos=MatchPos;
  SelectSize=SearchLength;
  SelectFlags=Flags;
//  LastSelPos=SelectPos+((Flags&0x2) ? -1:1);
  if (VM.Hex)
    FilePos&=~(VM.Unicode ? 0x7:0xf);
  else
  {
    if (SelectPos!=StartLinePos)
    {
      Up();
      Show (); //update OutStr
    }

  /* $ 13.03.2001 IS
     Если найденное расположено в самой первой строке юникодного файла и файл
     имеет в начале fffe или feff, то для более правильного выделения, его
     позицию нужно уменьшить на единицу (из-за того, что пустой символ не
     показывается)
  */

    SelectPosOffSet=(VM.Unicode && (FirstWord==0x0FFFE || FirstWord==0x0FEFF)
           && (MatchPos+SelectSize<=ObjWidth && MatchPos<(__int64)wcslen(Strings[0]->lpData)))?1:0;

    SelectPos-=SelectPosOffSet;

  /* IS $ */


    __int64 Length=SelectPos-StartLinePos-1;
    if (VM.Wrap)
      Length%=Width+1; //??
    if (Length<=Width)
        LeftPos=0;
    if (Length-LeftPos>Width || Length<LeftPos)
    {
      LeftPos=Length;
      if (LeftPos>(MAX_VIEWLINE-1) || LeftPos<0)
        LeftPos=0;
      else
        if (LeftPos>10)
          LeftPos-=10;
    }
  }
  if(Flags&1)
  {
    AdjustSelPosition = TRUE;
    Show();
    AdjustSelPosition = FALSE;
  }
}
/* SVS $ */


/* $ 27.09.2000 SVS
   "Ядро" будущего Viewer API :-)
*/
int Viewer::ViewerControl(int Command,void *Param)
{
  int I;
  switch(Command)
  {
    case VCTL_GETINFO:
    {
      if(Param && !IsBadReadPtr(Param,sizeof(struct ViewerInfoW)))
      {
        struct ViewerInfoW *Info=(struct ViewerInfoW *)Param;
        /* $ 29.01.2001 IS
          - баг - затирался StructSize
        */
        memset(&Info->ViewerID,0,Info->StructSize-sizeof(Info->StructSize));
        /* IS */
        Info->ViewerID=Viewer::ViewerID;
        Info->FileName=strFullFileName;
        Info->WindowSizeX=ObjWidth;
        Info->WindowSizeY=Y2-Y1;
        Info->FilePos.i64=FilePos;
        Info->FileSize.i64=FileSize;
        memmove(&Info->CurMode,&VM,sizeof(struct ViewerMode));
        Info->CurMode.TableNum=VM.UseDecodeTable ? VM.TableNum-2:-1;
        Info->Options=0;
        if (Opt.ViOpt.SaveViewerPos)   Info->Options|=VOPT_SAVEFILEPOSITION;
        if (ViOpt.AutoDetectTable)     Info->Options|=VOPT_AUTODETECTTABLE;
        Info->TabSize=ViOpt.TabSize;

        // сюды писать добавки
        if(Info->StructSize >= sizeof(struct ViewerInfoW))
        {
          Info->LeftPos=(int)LeftPos;  //???
        }
        return(TRUE);
      }
      break;
    }
    /*
       Param = struct ViewerSetPosition
               сюда же будет записано новое смещение
               В основном совпадает с переданным
    */
    case VCTL_SETPOSITION:
    {
      if(Param && !IsBadReadPtr(Param,sizeof(struct ViewerSetPosition)))
      {
        struct ViewerSetPosition *vsp=(struct ViewerSetPosition*)Param;
        bool isReShow=vsp->StartPos.i64 != FilePos;
        if((LeftPos=vsp->LeftPos) < 0)
          LeftPos=0;
        /* $ 20.01.2003 IS
             Если кодировка - юникод, то оперируем числами, уменьшенными в
             2 раза. Поэтому увеличим StartPos в 2 раза, т.к. функция
             GoTo принимает смещения в _байтах_.
        */
        GoTo(FALSE, vsp->StartPos.i64*(VM.Unicode?2:1), vsp->Flags);
        /* IS $ */
        if (isReShow && !(vsp->Flags&VSP_NOREDRAW))
          ScrBuf.Flush();
        if(!(vsp->Flags&VSP_NORETNEWPOS))
        {
          vsp->StartPos.i64=FilePos;
          vsp->LeftPos=(int)LeftPos; //???
        }
        return(TRUE);
      }
      break;
    }

    // Param=ViewerSelect
    case VCTL_SELECT:
    {
      struct ViewerSelect *vs=(struct ViewerSelect *)Param;
      if(vs && !IsBadReadPtr(vs,sizeof(struct ViewerSelect)))
      {
        __int64 SPos=vs->BlockStartPos.i64;
        int SSize=vs->BlockLen;
        if(SPos < FileSize)
        {
          if(SPos+SSize > FileSize)
          {
            SSize=(int)(FileSize-SPos);
          }
          SelectText(SPos,SSize,0x1);
          ScrBuf.Flush();
          return(TRUE);
        }
      }
      else
      {
        SelectSize = 0;
        Show();
      }
      break;
    }

    /* Функция установки Keybar Labels
         Param = NULL - восстановить, пред. значение
         Param = -1   - обновить полосу (перерисовать)
         Param = KeyBarTitles
    */
    case VCTL_SETKEYBAR:
    {
      struct KeyBarTitles *Kbt=(struct KeyBarTitles*)Param;
      if(!Kbt)
      {        // восстановить пред значение!
        if (HostFileViewer!=NULL)
          HostFileViewer->InitKeyBar();
      }
      else
      {
        if((LONG_PTR)Param != (LONG_PTR)-1 && !IsBadReadPtr(Param,sizeof(struct KeyBarTitles))) // не только перерисовать?
        {
          for(I=0; I < 12; ++I)
          {
            if(Kbt->Titles[I])
              ViewKeyBar->Change(KBL_MAIN,Kbt->Titles[I],I);
            if(Kbt->CtrlTitles[I])
              ViewKeyBar->Change(KBL_CTRL,Kbt->CtrlTitles[I],I);
            if(Kbt->AltTitles[I])
              ViewKeyBar->Change(KBL_ALT,Kbt->AltTitles[I],I);
            if(Kbt->ShiftTitles[I])
              ViewKeyBar->Change(KBL_SHIFT,Kbt->ShiftTitles[I],I);
            if(Kbt->CtrlShiftTitles[I])
              ViewKeyBar->Change(KBL_CTRLSHIFT,Kbt->CtrlShiftTitles[I],I);
            if(Kbt->AltShiftTitles[I])
              ViewKeyBar->Change(KBL_ALTSHIFT,Kbt->AltShiftTitles[I],I);
            if(Kbt->CtrlAltTitles[I])
              ViewKeyBar->Change(KBL_CTRLALT,Kbt->CtrlAltTitles[I],I);
          }
        }
        ViewKeyBar->Show();
        ScrBuf.Flush(); //?????
      }
      return(TRUE);
    }

    // Param=0
    case VCTL_REDRAW:
    {
      ChangeViewKeyBar();
      Show();
      ScrBuf.Flush();
      return(TRUE);
    }

    // Param=0
    case VCTL_QUIT:
    {
      /* $ 28.12.2002 IS
         Разрешаем выполнение VCTL_QUIT только для вьюера, который
         не является панелью информации и быстрого просмотра (т.е.
         фактически панелей на экране не видно)
      */
      if(!FrameManager->IsPanelsActive())
      {
        /* $ 29.09.2002 IS
           без этого не закрывался вьюер, а просили именно это
        */
        FrameManager->DeleteFrame(HostFileViewer);
        /* IS $ */
        if (HostFileViewer!=NULL)
          HostFileViewer->SetExitCode(0);
        return(TRUE);
      }
      /* IS 28.12.2002 $ */
    }

    /* Функция установки режимов
         Param = ViewerSetMode
    */
    case VCTL_SETMODE:
    {
      struct ViewerSetMode *vsmode=(struct ViewerSetMode *)Param;
      if(vsmode && !IsBadReadPtr(vsmode,sizeof(struct ViewerSetMode)))
      {
        bool isRedraw=vsmode->Flags&VSMFL_REDRAW?true:false;
        switch(vsmode->Type)
        {
          case VSMT_HEX:
            ProcessHexMode(vsmode->Param.iParam,isRedraw);
            return TRUE;

          case VSMT_WRAP:
            ProcessWrapMode(vsmode->Param.iParam,isRedraw);
            return TRUE;

          case VSMT_WORDWRAP:
            ProcessTypeWrapMode(vsmode->Param.iParam,isRedraw);
            return TRUE;
        }
      }
      return FALSE;
    }

  }
  return(FALSE);
}
/* SVS $ */

BOOL Viewer::isTemporary()
{
  return !strTempViewName.IsEmpty();
}

int Viewer::ProcessHexMode(int newMode, bool isRedraw)
{
  int oldHex=VM.Hex;
  VM.Hex=newMode&1;
  if(isRedraw)
  {
    ChangeViewKeyBar();
    Show();
  }
// LastSelPos=FilePos;
  return oldHex;
}

int Viewer::ProcessWrapMode(int newMode, bool isRedraw)
{
  int oldWrap=VM.Wrap;
  VM.Wrap=newMode&1;
  if (VM.Wrap)
    LeftPos = 0;

  if ( !VM.Wrap && LastPage )
    Up ();

  if(isRedraw)
  {
    ChangeViewKeyBar();
    Show();
  }

  Opt.ViOpt.ViewerIsWrap=VM.Wrap;
//  LastSelPos=FilePos;
  return oldWrap;
}

int Viewer::ProcessTypeWrapMode(int newMode, bool isRedraw)
{
  int oldTypeWrap=VM.WordWrap;
  if(RegVer)
  {
    VM.WordWrap=newMode&1;
    if(!VM.Wrap)
    {
      VM.Wrap=!VM.Wrap;
      LeftPos = 0;
    }

    if(isRedraw)
    {
      ChangeViewKeyBar();
      Show();
    }

    Opt.ViOpt.ViewerWrap=VM.WordWrap;
//    LastSelPos=FilePos;
  }
  return oldTypeWrap;
}
