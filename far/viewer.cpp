/*
viewer.cpp

Internal viewer

*/

/* Revision: 1.51 26.03.2001 $ */

/*
Modify:
  26.03.2001 SVS
    + При вызове списка плагинов говорим манагеру о том, чтобы он искал
      предопределенный топик "Viewer" для Shift-F1 из списка плагинов
  22.03.2001 IS
    - Баг с переходом по alt-f8: переход происходил только тогда, когда в
      момент закрытия диалога курсор находился в строке ввода (была неверная
      проверка кода возврата из диалога).
  13.02.2001 IS
    ! При исправлении бага с выделением в юникодных файлах (06.02.2001) не учел
      то, что коррекция должна быть только для самой первой строки, поэтому баг
      не до конца был исправлен. Сейчас ситуация уже лучше.
  27.02.2001 VVM
    ! Символы, зависимые от кодовой страницы
      /[\x01-\x08\x0B-\x0C\x0E-\x1F\xB0-\xDF\xF8-\xFF]/
      переведены в коды.
  20.02.2001 VVM
    + GetWrapType()/SetWrapType()
  06.02.2001 IS
   - Бага с выделением, которую добавили, когда убрали показ начального пробела
     в юникодных файлах. См. SelectPosOffSet в SelectText
  29.01.2001 IS
   - баг - затирался StructSize в ViewerInfo
  28.01.2001
   - Путем проверки ViewFile на NULL в ProcessKey  избавляемся от падения
  23.01.2001 SVS
   + Заполнение Info->LeftPos в VCTL_GETINFO.
  22.01.2001 IS
   !  Внимание! Возможно, это не совсем верное решение проблемы
      выделения из плагинов, но мне пока другого в голову не пришло.
      Я приравниваю SelectSize нулю _только_ в Process*
  21.01.2001 IS
   ! Для однообразия с редактором изменил пару названий:
      VCTL_SETPOS -> VCTL_SETPOSITION
      AnsiText -> AnsiMode
  19.01.2001 SVS
    ! Изменен вызов функции GoTo() - три дополнительных параметра
    - Устранение висячих строк при переходе (функция GoTo)
    + Функция выделения SelectText() - как самостоятельная функция
    + VCTL_SELECT
    ! Изменения в VCTL_SETPOS
    + Хотелка:
       "чтобы запоминался режим просмотра файла Hex/норм.
       отдельно для каждого файла"
      стала возможной потому, что были резервы при кешировании :-)
      Остался еще один пункт :-)
  11.01.2000 VVM
    ! Левый край считается за раз, а не итерациями по +4
  21.12.2000 SVS
    ! Не спрячем элемент HEX в поисковом диалоге, а задизаблим (для Unicode)
  16.12.2000 tran
    + шелчок мышью на статус баре (ProcessMouseEvent())
  03.11.2000 OT
    ! Введение проверки возвращаемого значения
  02.11.2000 OT
    ! Введение проверки на длину буфера, отведенного под имя файла.
  20.10.2000 tran
    + обратный порядок байтов в юникоде (fffe)
  18.10.2000 SVS
    - бага: DownDownUp в Уникод-файлах (FEFF)
  02.10.2000 SVS
    - бага со скроллером.
      > Если нажать в самом низу скролбара, вьюер отмотается на страницу
      > ниже нижней границы текста. Перед глазами будет пустой экран.
  01.10.2000 IS
    ! Показывать букву диска в статусной строке
  29.09.2000 SVS
    ! TableNum - 2
  27.09.2000 SVS
    + ViewerControl - (пока только: получить минимум необходимой
      информации - для PrintMan)
    ! Переменные UseDecodeTable,TableNum,AnsiText,Unicode,Wrap, TypeWrap, Hex
      введены в одну структуру ViewerMode.
  27.09.2000 SVS
    + Глюки с определением Unicode при просмотре по '+' & '-'
  24.09.2000 SVS
    + Работа по сохранению/восстановлению позиций в файле по RCtrl+<N>
  19.09.2000 SVS
    ! FEFF-файлы - уточнение алгоритма отображения и распознавания.
  18.09.2000 SVS
    ! Уточнение Warp и KeyBar
  14.09.2000 SVS
    + AutoDecode Unicode - те файлы, которые начинаются с 0xFEFF
  13.09.2000 tran 1.23
    + при WWrap обрезаются пробелы в начале строки
  12.09.2000 SVS
    ! Разделение Wrap/WWrap/UnWrap на 2 составляющих -
      F2 Состояние (Wrap/UnWrap) и Shift-F2 тип (Wrap/WWrap)
  10.09.2000 SVS
    ! Постоянный скроллинг при нажатой клавише
      Обыкновенный захват мыши
  01.09.2000 SVS
    - Небольшая бага с тыканием в верхнюю позицию ScrollBar`а
  31.08.2000 SVS
    + Теперь FAR помнит тип Wrap
    - Бага - без часиков и со ScrollBar неверно отображается верхний статус
  04.08.2000 KM
    ! !!!Неверный предыдущий патч!!!!
  01.08.2000 KM 1.16
    + Добавлен поиск по "Целым словам". Работает в связке
      с поиском по Alt-F7.
  01.08.2000 tran 1.16
    + |DIF_USELASTHISTORY
  19.07.2000 tran 1.15
    + при рисовке скролбара граница уменьшается на 1
  18.07.2000 tran 1.14
    + рисование сколбара и стрелок в зависимости от настроек
      скролбар также реагирует на мышку
      и переключается Ctrl-S
  17.07.2000 tran
    + я не суеверный, 1.13 пойдет :)
      теперь диалог по alt-f8 реагирует на
      nn% - проценты
      0xnn, nnh, nn$ - hex offset
      +/-nn - относительное смещение
  15.07.2000 SVS
    ! Wrap должен показываться следующий, а не текущий
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  12.07.2000 tran
    ! OutStr are dynamic, new, delete,
      and sizeof(OutStr[i]) changed to MAX_VIEWLINEB
  12.07.2000 SVS
    + wrap имеет 3 положения :-) Но только для зарегестрированных.
  11.07.2000 tran
    + wrap are now WORD-WRAP
  11.07.2000 SVS
    ! Изменения для возможности компиляции под BC & VC
  10.07.2000 tran
    ! увеличение длины строки с 512 до MAX_VIEWLINE (2048)
      изменений по тексту - 8, "512" заменено на MAX_VIEWLINE,
      а "511" на MAX_VIEWLINE-1
  04.07.2000 tran
    + параметер warning в методе OpenFile()
      нужно для QuickView
  03.07.2000 tran
    - баг с неверным показом последней строки в hex
      (внесенный мною патчем номер 2)
  30.06.2000 tran
    - баг с двойным путем в имени файла,
      если он показывается файл с temppanel (например)
      в таком случае файл уже имеет путь, и добавлять его не надо
  28.06.2000 IS (22.06.2000)
    + Показывать полное имя файла во вьюере
  28.06.2000 tran
    - показ пустой строки в hex viewer
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

static int InitLastSearchHex=0;

static struct CharTableSet InitTableSet;
static int InitUseDecodeTable=FALSE,InitTableNum=0,InitAnsiText=FALSE;

static int InitHex=FALSE;
/* $ 27.09.2000 SVS
   ID вьювера - по аналогии с Editor
*/
static int ViewerID=0;
/* SVS $*/

static char BorderLine[]={0x0B3,0x020,0x00};

Viewer::Viewer()
{
  /* $ 12.07.2000 tran
     alloc memory for OutStr */
  for ( int i=0; i<=MAXSCRY; i++ )
  {
    OutStr[i]=new char[MAX_VIEWLINEB];
  }
  /* tran 12.07.2000 $ */
  strcpy((char *)LastSearchStr,GlobalSearchString);
  LastSearchCase=GlobalSearchCase;
  /* $ 01.08.2000 KM
     Переменная для поиска "Whole words"
  */
  LastSearchWholeWords=GlobalSearchWholeWords;
  /* KM $ */
  LastSearchReverse=GlobalSearchReverse;
  LastSearchHex=InitLastSearchHex;
  memcpy(&TableSet,&InitTableSet,sizeof(TableSet));
  VM.UseDecodeTable=InitUseDecodeTable;
  VM.TableNum=InitTableNum;
  VM.AnsiMode=InitAnsiText;

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
  VM.Wrap=Opt.ViewerIsWrap;
  VM.TypeWrap=Opt.ViewerWrap;
  /* SVS $ */
  VM.Hex=InitHex;

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
  Viewer::ViewerID=::ViewerID++;
  CtrlObject->Plugins.CurViewer=this;
  OpenFailed=false;
  HostFileViewer=NULL;
  /* $ 06.02.2001 IS
     См. SelectText
  */
  SelectPosOffSet=0;
  /* IS $ */
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
        if (VM.AnsiMode)
          Table=2;
        else
          if (VM.Unicode)
            Table=3;
          else
            if (VM.UseDecodeTable)
              Table=VM.TableNum+3;
      }
      CtrlObject->ViewerPosCache.AddPosition(CacheName,FilePos,LeftPos,VM.Hex,0,Table,
          (long*)(Opt.SaveViewerShortPos?SavePosAddr:NULL),
          (long*)(Opt.SaveViewerShortPos?SavePosLeft:NULL),NULL,NULL);
    }
  }
  if (*TempViewName)
  {
    chmod(TempViewName,S_IREAD|S_IWRITE);
    remove(TempViewName);
    *PointToName(TempViewName)=0;
    RemoveDirectory(TempViewName);
  }
  /* $ 12.07.2000 tran
     free memory  */
  for ( int i=0; i<=MAXSCRY; i++ )
  {
    /* $ 13.07.2000 SVS
      раз уж вызвали new[], то и нужно delete[]
    */
    delete[] OutStr[i];
    /* SVS $ */
  }
  /* tran 12.07.2000 $ */
  if (!OpenFailed)
  {
    CtrlObject->Plugins.CurViewer=this;
//    CtrlObject->Plugins.ProcessViewerEvent(VE_CLOSE,&ViewerID);
  }
}


void Viewer::KeepInitParameters()
{
  strcpy(GlobalSearchString,(char *)LastSearchStr);
  GlobalSearchCase=LastSearchCase;
  /* $ 01.08.2000 KM
     Сохранение параметра "Whole words" в глобальной GlobalSearchWholeWords
  */
  GlobalSearchWholeWords=LastSearchWholeWords;
  /* KM $ */
  GlobalSearchReverse=LastSearchReverse;
  InitLastSearchHex=LastSearchHex;
  memcpy(&InitTableSet,&TableSet,sizeof(InitTableSet));
  InitUseDecodeTable=VM.UseDecodeTable;
  InitTableNum=VM.TableNum;
  InitAnsiText=VM.AnsiMode;
  Opt.ViewerIsWrap=VM.Wrap;
  Opt.ViewerWrap=VM.TypeWrap;
  InitHex=VM.Hex;
}


int Viewer::OpenFile(char *Name,int warning)
{
  FILE *NewViewFile=NULL;
  OpenFailed=false;
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
    /* $ 04.07.2000 tran
       + 'warning' flag processing, in QuickView it is FALSE
         so don't show red message box */
    if (warning)
        Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MViewerTitle),
            MSG(MViewerCannotOpenFile),Name,MSG(MOk));
    /* tran 04.07.2000 $ */

    OpenFailed=true;
    return(FALSE);
  }
  if (ViewFile)
    fclose(ViewFile);
  TableChangedByUser=FALSE;
  ViewFile=NewViewFile;
  strcpy(FileName,Name);
//  ConvertNameToFull(FileName,FullFileName, sizeof(FullFileName));
  if (ConvertNameToFull(FileName,FullFileName, sizeof(FullFileName)) >= sizeof(FullFileName)){
    OpenFailed=false;
    return FALSE;
  }

  HANDLE ViewFindHandle;
  ViewFindHandle=FindFirstFile(FileName,&ViewFindData);
  FindClose(ViewFindHandle);

  /* $ 19.09.2000 SVS
    AutoDecode Unicode
  */
  BOOL IsDecode=FALSE;

  if(Opt.ViewerAutoDetectTable)
  {
    DWORD ReadSize;
    VM.Unicode=0;
    FirstWord=0;
    vseek(ViewFile,0,SEEK_SET);
    ReadSize=vread((char *)&FirstWord,sizeof(FirstWord),ViewFile);
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

  if (Opt.SaveViewerPos && !ReadStdin)
  {
    unsigned int NewLeftPos,TempPos1,TempPos2,Table,NewFilePos;
    char CacheName[NM*3];
    if (*PluginData)
      sprintf(CacheName,"%s%s",PluginData,PointToName(FileName));
    else
      strcpy(CacheName,FileName);
    CtrlObject->ViewerPosCache.GetPosition(CacheName,NewFilePos,NewLeftPos,TempPos1,TempPos2,Table,
          (long*)(Opt.SaveViewerShortPos?SavePosAddr:NULL),
          (long*)(Opt.SaveViewerShortPos?SavePosLeft:NULL),NULL,NULL);

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
    VM.Hex=TempPos1;
  }
  else
    FilePos=0;
  SetFileSize();
  if (FilePos>FileSize)
    FilePos=0;
  SetCRSym();
  if (Opt.ViewerAutoDetectTable && !TableChangedByUser)
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
    if (VM.AnsiMode)
    {
      VM.AnsiMode=FALSE;
      ChangeViewKeyBar();
    }
  }
  /* $ 19.07.2000 tran
    + вычисление нужного */
  Width=X2-X1+1;
  XX2=X2;

  if ( Opt.ViewerShowScrollbar )
  {
     Width--;
     XX2--;
  }
  /* tran 19.07.2000 $ */
  CtrlObject->Plugins.CurViewer=this;
//  CtrlObject->Plugins.ProcessViewerEvent(VE_READ,NULL);
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

  /* $ 19.07.2000 tran
    + вычисление нужного */
  Width=X2-X1+1;
  XX2=X2;

  if ( Opt.ViewerShowScrollbar )
  {
     Width--;
     XX2--;
  }
  /* tran 19.07.2000 $ */


  /* $ 04.07.2000 tran
    + показ строки "Cannot open the file" красным цветом
      в случае невозножности его открытия */
  if (ViewFile==NULL)
  {
    GotoXY(X1,Y1);
    SetColor(COL_WARNDIALOGTEXT);
    mprintf(MSG(MViewerCannotOpenFile));
    return;
  }
  /* tran $ */

  CtrlObject->Plugins.CurViewer=this;

  ViewY1=Y1+ShowStatusLine;

  if (HideCursor)
  {
    MoveCursor(79,24);
    SetCursorType(0,10);
  }
  vseek(ViewFile,FilePos,SEEK_SET);

  if (VM.Hex)
    ShowHex();
  else
  {
    for (I=0,Y=ViewY1;Y<=Y2;Y++,I++)
    {
      StrFilePos[I]=vtell(ViewFile);
      if (Y==ViewY1+1 && !feof(ViewFile))
        SecondPos=vtell(ViewFile);
      ReadString(OutStr[I],-1,MAX_VIEWLINEB,SelPos,SelSize);
      SetColor(COL_VIEWERTEXT);
      GotoXY(X1,Y);
      if (strlen((char *)OutStr[I])>LeftPos)
      {
        /* $ 18.10.2000 SVS
           -Bug: Down Down Up & первый пробел
        */
        if(VM.Unicode &&
             (FirstWord == 0x0FEFF || FirstWord == 0x0FFFE)
             && !I && !LeftPos && !StrFilePos[I])
          mprintf("%-*.*s",Width,Width,&OutStr[I][LeftPos+1]);
        else
          mprintf("%-*.*s",Width,Width,&OutStr[I][LeftPos]);
        /* SVS $*/
        /* $ 18.07.2000 tran -
           проверка флага
        */
        if (strlen(&OutStr[I][LeftPos])>Width && Opt.ViewerShowArrows)
        {
          GotoXY(XX2,Y);
          SetColor(COL_VIEWERARROWS);
          mprintf(">");
        }
      }
      else
        mprintf("%*s",Width,"");
      /* $ 18.07.2000 tran -
         проверка флага
      */
      if (LeftPos>0 && *OutStr[I]!=0  && Opt.ViewerShowArrows)
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
        /* $ 12.07.2000 SVS
           ! Wrap - трех позиционный
        */
        if (!VM.Wrap &&
        /* SVS $ */
           SelX1+SaveSelectSize-1>XX2 && LeftPos<MAX_VIEWLINE
        /* $ 11.01.2000 VVM
           Левый край считается за раз, а не итерациями по +4 */
           && (X1+SelPos+SaveSelectSize-XX2<MAX_VIEWLINE))
        {
//          LeftPos+=4;
          LeftPos=X1+SelPos+SaveSelectSize-XX2;
        /* VVM $ */
          SelectSize=SaveSelectSize;
          Show();
          return;
        }
        SetColor(COL_VIEWERSELECTEDTEXT);
        GotoXY(SelX1,Y);
        if (SelSize>XX2-SelX1+1)
          SelSize=XX2-SelX1+1;
        if (SelSize>0)
        /* $ 06.02.2001 IS
             см. SelectText
        */
          mprintf("%.*s",SelSize,&OutStr[I][SelPos+SelectPosOffSet]);
        /* IS $ */
      }
    }
  } // if (Hex)  - else
  /* $ 18.07.2000 tran
     рисование скролбара */
  if ( Opt.ViewerShowScrollbar )
  {
    SetColor(COL_VIEWERSCROLLBAR);
    ScrollBar(X2,Y1+1,Y2-Y1,FilePos,FileSize);
  }
  /* tran 18.07.2000 $ */


  ShowStatus();
}


void Viewer::ShowHex()
{
  char OutStr[MAX_VIEWLINE],TextStr[20];
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

    if (VM.Unicode)
      for (X=0;X<8;X++)
      {
        if (SelectSize>0 && SelectPos==vtell(ViewFile))
        {
          SelPos=strlen(OutStr);
          SelSize=SelectSize;
          /* $ 22.01.2001 IS
              Внимание! Возможно, это не совсем верное решение проблемы
              выделения из плагинов, но мне пока другого в голову не пришло.
              Я приравниваю SelectSize нулю в Process*
          */
          //SelectSize=0;
          /* IS $ */
        }

        if ((Ch=getc(ViewFile))==EOF || (Ch1=getc(ViewFile))==EOF)
        {
          /* $ 28.06.2000 tran
             убираем показ пустой строки, если длина
             файла кратна 16 */
          EndFile=LastPage=1;
          if ( X==0 )
          {
             strcpy(OutStr,"");
             break;
          }
          strcat(OutStr,"     ");
          TextStr[TextPos++]=' ';
          /* tran $ */
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
          strcat(OutStr,BorderLine);
      }
    else
      for (X=0;X<16;X++)
      {
        if (SelectSize>0 && SelectPos==vtell(ViewFile))
        {
          SelPos=strlen(OutStr);
          SelSize=SelectSize;
          /* $ 22.01.2001 IS
              Внимание! Возможно, это не совсем верное решение проблемы
              выделения из плагинов, но мне пока другого в голову не пришло.
              Я приравниваю SelectSize нулю в Process*
          */
          //SelectSize=0;
          /* IS $ */
        }
        if ((Ch=vgetc(ViewFile))==EOF)
        {
          /* $ 28.06.2000 tran
             убираем показ пустой строки, если длина
             файла кратна 16 */
          EndFile=LastPage=1;
          if ( X==0 )
          {
             strcpy(OutStr,"");
             break;
          }
          /* $ 03.07.2000 tran
             - вместо 5 пробелов тут надо 3 */
          strcat(OutStr,"   ");
          /* tran $ */
          TextStr[TextPos++]=' ';
          /* tran $ */
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
          strcat(OutStr,BorderLine);
      }
    TextStr[TextPos]=0;
    if (VM.UseDecodeTable && !VM.Unicode)
      DecodeString(TextStr,(unsigned char *)TableSet.DecodeTable);
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
      mprintf("%.*s",VM.Unicode ? 4:2,&OutStr[SelPos]);
      SelSize=0;
    }
  }
}


void Viewer::ShowUp()
{
  int Tmp,Y,I;

  /* $ 19.07.2000 tran
    + вычисление нужного */
  Width=X2-X1+1;
  XX2=X2;
  if ( Opt.ViewerShowScrollbar )
  {
     Width--;
     XX2--;
  }
  /* tran 19.07.2000 $ */


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

  ReadString(OutStr[0],SecondPos-FilePos,MAX_VIEWLINEB,Tmp,Tmp);

  for (I=0,Y=ViewY1;Y<=Y2;Y++,I++)
  {
    SetColor(COL_VIEWERTEXT);
    GotoXY(X1,Y);
    if (strlen(OutStr[I])>LeftPos)
    {
      /* $ 18.10.2000 SVS
         -Bug: Down Down Up & первый пробел
      */
      if(VM.Unicode && (FirstWord == 0x0FEFF || FirstWord == 0x0FFFE) && !I && !LeftPos && !StrFilePos[I])
        mprintf("%-*.*s",Width,Width,&OutStr[I][LeftPos+1]);
      else
        mprintf("%-*.*s",Width,Width,&OutStr[I][LeftPos]);
      /* SVS $ */
      if (strlen(&OutStr[I][LeftPos])>Width && Opt.ViewerShowArrows)
      {
        GotoXY(XX2,Y);
        SetColor(COL_VIEWERARROWS);
        mprintf(">");
      }
    }
    else
      mprintf("%*s",Width,"");
    if (LeftPos>0 && *OutStr[I]!=0 && Opt.ViewerShowArrows)
    {
      GotoXY(X1,Y);
      SetColor(COL_VIEWERARROWS);
      mprintf("<");
    }
  }
  ShowStatus();
  /* $ 18.07.2000 tran
     рисование скролбара */
  if ( Opt.ViewerShowScrollbar )
  {
    SetColor(COL_VIEWERSCROLLBAR);
    ScrollBar(X2,Y1+1,Y2-Y1,FilePos,FileSize);
  }
  /* tran 18.07.2000 $ */

}


void Viewer::ShowStatus()
{
  char Status[200],Name[NM];
  if (!ShowStatusLine)
    return;
  /* $ 22.06.2000 IS
    Показывать полное имя файла во вьюере
    Was: strcpy(Name,*Title ? Title:FileName);
  */
  if(*Title) strcpy(Name,Title);
  else
  {
    /* $ 30.06.2000 tran
       - double path when show file from temp panel */
    if ( ! (FileName[1]==':' && FileName[2]=='\\') )
    {
        ViewNamesList.GetCurDir(Name);
        if(int len=strlen(Name))
            if(Name[len-1]!='\\')
                strcat(Name,"\\");
        strcat(Name,FileName);
    }
    else
        strcpy(Name,FileName);
    /* tran $ */
  }
  /* IS $  */
  int NameLength=ScrX-40;
  if (Opt.ViewerEditorClock)
    NameLength-=6;
  if (NameLength<20)
    NameLength=20;
  /* $ 01.10.2000 IS
     ! Показывать букву диска в статусной строке
  */
  TruncPathStr(Name,NameLength);
  /* IS $ */
  char *TableName;
  if (VM.Unicode)
    TableName="Unicode";
  else
    if (VM.UseDecodeTable)
      TableName=TableSet.TableName;
    else
      if (VM.AnsiMode)
        TableName="Win";
      else
        TableName="DOS";
  sprintf(Status,"%-*s %10.10s %10u %7.7s %-4d %s%3d%%",
          NameLength,Name,TableName,FileSize,MSG(MViewerStatusCol),
          LeftPos,Opt.ViewerEditorClock ? "":" ",
          LastPage ? 100:ToPercent(FilePos,FileSize));
  SetColor(COL_VIEWERSTATUS);
  GotoXY(X1,Y1);
  /* $ 31.08.2000 SVS
     Бага - без часиков неверно отображается верхний статус
  */
  mprintf("%-*.*s",Width+(Opt.ViewerShowScrollbar?1:0),
                   Width+(Opt.ViewerShowScrollbar?1:0),Status);
  /* SVS $ */
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

  /* $ 19.07.2000 tran
    + вычисление нужного */
  Width=X2-X1+1;
  XX2=X2;
  if ( Opt.ViewerShowScrollbar )
  {
     Width--;
     XX2--;
  }
  /* tran 19.07.2000 $ */

  OutPtr=0;
  SelSize=0;
  if (VM.Hex)
  {
    OutPtr=vread(Str,VM.Unicode ? 8:16,ViewFile);
    Str[VM.Unicode ? 8:16]=0;
  }
  else
  {
    bool CRSkipped=false;
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
        unsigned long SavePos=vtell(ViewFile);
        if ((Ch=vgetc(ViewFile))!=CRSym && (Ch!=13 || vgetc(ViewFile)!=CRSym))
        {
          vseek(ViewFile,SavePos,SEEK_SET);
          if (VM.TypeWrap && RegVer) // только для зарегестрированных
          {
            if ( ! (Ch==' ' || Ch=='\t'  ) && !(Str[OutPtr]==' ' || Str[OutPtr]=='\t'))
            {
               unsigned long SavePtr=OutPtr;
               /* $ 18.07.2000 tran
                  добавил в качестве wordwrap разделителей , ; > ) */
               while (OutPtr && !(Str[OutPtr]==' ' || Str[OutPtr]=='\t' || Str[OutPtr]==',' || Str[OutPtr]==';' || Str[OutPtr]=='>'|| Str[OutPtr]==')'))
               /* tran 18.07.2000 $ */
                  OutPtr--;
               if ( Str[OutPtr]==',' || Str[OutPtr]==';' || Str[OutPtr]==')' || Str[OutPtr]=='>' )
                   OutPtr++;
               else
                   while ((Str[OutPtr]==' ' || Str[OutPtr]=='\t' ) && OutPtr<=SavePtr)
                      OutPtr++;

               if ( OutPtr )
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
            unsigned long savepos=vtell(ViewFile);
            while (Ch==' '|| Ch=='\t')
                Ch=vgetc(ViewFile);
            if ( vtell(ViewFile)!=savepos)
                vseek(ViewFile,-1,SEEK_CUR);
            /* tran 13.09.2000 $ */
          }// wwrap
          /* SVS $ */
        }
        break;
      }

      if (SelectSize>0 && SelectPos==vtell(ViewFile))
      {
        SelPos=OutPtr;
        SelSize=SelectSize;
        /* $ 22.01.2001 IS
            Внимание! Возможно, это не совсем верное решение проблемы
            выделения из плагинов, но мне пока другого в голову не пришло.
            Я приравниваю SelectSize нулю в Process*
        */
        //SelectSize=0;
        /* IS $ */
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
        /* $ 12.07.2000 SVS
          Wrap - 3-x позиционный и если есть регистрация :-)
        */
        if ((VM.Wrap && (VM.TypeWrap && RegVer))
        /* SVS $ */
            && OutPtr>XX2-X1)
          Str[XX2-X1+1]=0;
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

  if (VM.UseDecodeTable && !VM.Unicode)
    DecodeString(Str,(unsigned char *)TableSet.DecodeTable);
  LastPage=feof(ViewFile);
}


/* $ 28.01.2001
   - Путем проверки ViewFile на NULL избавляемся от падения
*/
int Viewer::ProcessKey(int Key)
{
  int Tmp,I;
  char ReadStr[528];

  /* $ 22.01.2001 IS
       Происходят какие-то манипуляции -> снимем выделение
  */
  if(Key!=KEY_IDLE && Key!=KEY_NONE) SelectSize=0;
  /* IS $ */

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
    /* $ 18.07.2000 tran
       включить/выключить скролбар */
    case KEY_CTRLS:
        Opt.ViewerShowScrollbar=!Opt.ViewerShowScrollbar;
        Show();
        return (TRUE);
    /* tran 18.07.2000 $ */
    case KEY_IDLE:
      {
        if(ViewFile)
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
            unsigned long CurFileSize=vtell(ViewFile);
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
              if (VM.AnsiMode)
                Table=2;
              else
                if (VM.Unicode)
                  Table=3;
                else
                  if (VM.UseDecodeTable)
                    Table=VM.TableNum+3;
            }
            CtrlObject->ViewerPosCache.AddPosition(CacheName,FilePos,LeftPos,VM.Hex,0,Table,
                    (long*)(Opt.SaveViewerShortPos?SavePosAddr:NULL),
                    (long*)(Opt.SaveViewerShortPos?SavePosLeft:NULL),NULL,NULL);
          }
          if (PointToName(Name)==Name)
          {
            char ViewDir[NM];
            ViewNamesList.GetCurDir(ViewDir);
            chdir(ViewDir);
          }
          /* $ 04.07.2000 tran
             + параметер 'warning' в OpenFile в данном месте он TRUE
          */
          if (OpenFile(Name,TRUE))
          /* tran $ */
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

    case KEY_SHIFTF2:
      if(RegVer)
      {
        VM.TypeWrap=!VM.TypeWrap;
        if(!VM.Wrap)
          VM.Wrap=!VM.Wrap;
        ChangeViewKeyBar();
        Show();
        Opt.ViewerWrap=VM.TypeWrap;
        LastSelPos=FilePos;
      }
      return TRUE;

    case KEY_F2:
      /* $ 12.07.200 SVS
        ! Wrap имеет 3 положения и...
      */
      VM.Wrap=!VM.Wrap;
      ChangeViewKeyBar();
      Show();
      /* $ 31.08.2000 SVS
        Сохраняем тип врапа
      */
      Opt.ViewerIsWrap=VM.Wrap;
      /* SVS $ */
      LastSelPos=FilePos;
      return(TRUE);
    case KEY_F4:
      VM.Hex=!VM.Hex;
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
      }
      VM.TableNum=0;
      VM.UseDecodeTable=VM.AnsiMode;
      ChangeViewKeyBar();
      Show();
      LastSelPos=FilePos;
      TableChangedByUser=TRUE;
      return(TRUE);
    case KEY_SHIFTF8:
      {
        int UseUnicode=TRUE;
        int GetTableCode=GetTable(&TableSet,FALSE,VM.TableNum,UseUnicode);
        if (GetTableCode!=-1)
        {
          if (VM.Unicode && !UseUnicode)
            FilePos*=2;
          if (!VM.Unicode && UseUnicode)
            FilePos=(FilePos+(FilePos&1))/2;
          VM.UseDecodeTable=GetTableCode;
          VM.Unicode=UseUnicode;
          SetFileSize();
          VM.AnsiMode=FALSE;
          ChangeViewKeyBar();
          Show();
          LastSelPos=FilePos;
          TableChangedByUser=TRUE;
        }
      }
      return(TRUE);
    case KEY_ALTF8:
      if(ViewFile)
        GoTo();
      return(TRUE);
    case KEY_F11:
      CtrlObject->Plugins.CommandsMenu(FALSE,TRUE,0,"Viewer");
      Show();
      return(TRUE);
    case KEY_UP:
      if (FilePos>0 && ViewFile)
      {
        Up();
        if (VM.Hex)
        {
          FilePos&=~(VM.Unicode ? 0x7:0xf);
          Show();
        }
        else
          ShowUp();
      }
      LastSelPos=FilePos;
      return(TRUE);
    case KEY_DOWN:
      if (!LastPage && ViewFile)
      {
        FilePos=SecondPos;
        Show();
      }
      LastSelPos=FilePos;
      return(TRUE);
    case KEY_PGUP:
      if(ViewFile)
      {
        for (I=ViewY1;I<Y2;I++)
          Up();
        Show();
        LastSelPos=FilePos;
      }
      return(TRUE);
    case KEY_PGDN:
      if (LastPage || ViewFile==NULL)
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
      if (LeftPos>0 && ViewFile)
      {
        if (VM.Hex && LeftPos>80-Width)
          LeftPos=Max(80-Width,1);
        LeftPos--;
        Show();
      }
      LastSelPos=FilePos;
      return(TRUE);
    case KEY_RIGHT:
      if (LeftPos<MAX_VIEWLINE && ViewFile)
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
      if(ViewFile)
      {
        LeftPos-=20;
        if (LeftPos<0)
          LeftPos=0;
        Show();
        LastSelPos=FilePos;
      }
      return(TRUE);
    case KEY_CTRLRIGHT:
      if(ViewFile)
      {
        LeftPos+=20;
        if (LeftPos>MAX_VIEWLINE)
          LeftPos=MAX_VIEWLINE;
        Show();
        LastSelPos=FilePos;
      }
      return(TRUE);
    case KEY_HOME:
    case KEY_CTRLHOME:
      if(ViewFile)
        LeftPos=0;
    case KEY_CTRLPGUP:
      if(ViewFile)
      {
        FilePos=0;
        Show();
        LastSelPos=FilePos;
      }
      return(TRUE);
    case KEY_END:
    case KEY_CTRLEND:
      if(ViewFile)
        LeftPos=0;
    case KEY_CTRLPGDN:
      if(ViewFile)
      {
        vseek(ViewFile,0,SEEK_END);
        FilePos=vtell(ViewFile);
        for (I=0;I<Y2-ViewY1;I++)
          Up();
        if (VM.Hex)
          FilePos&=~(VM.Unicode ? 0x7:0xf);
        Show();
        LastSelPos=FilePos;
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
  /* $ 18.07.2000 tran
     просто для сокращения кода*/
  int MsX=MouseEvent->dwMousePosition.X;
  int MsY=MouseEvent->dwMousePosition.Y;
  /* tran 18.07.2000 $ */


  if ((MouseEvent->dwButtonState & 3)==0)
    return(FALSE);

  /* $ 22.01.2001 IS
       Происходят какие-то манипуляции -> снимем выделение
  */
  SelectSize=0;
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
  if ( Opt.ViewerShowScrollbar && MsX==X2)
  {
    /* $ 01.09.2000 SVS
       Небольшая бага с тыканием в верхнюю позицию ScrollBar`а
    */
    if (MsY == Y1+1)
      while (IsMouseButtonPressed())
        ProcessKey(KEY_UP);
    /* SVS $*/
    else if (MsY==Y2)
      while (IsMouseButtonPressed())
        ProcessKey(KEY_DOWN);
    else if(MsY == Y1+2)
        ProcessKey(KEY_CTRLHOME);
    else if(MsY == Y2-2)
        ProcessKey(KEY_CTRLEND);
    else
    {
      while (IsMouseButtonPressed())
      {
        INPUT_RECORD rec;
        FilePos=(FileSize-1)*(MsY-Y1)/(Y2-Y1-1);
        if ( FilePos>FileSize )
            FilePos=FileSize;
        //SysLog("Viewer/ ToPercent()=%i, %i, %i",ToPercent(FilePos,FileSize),FilePos,FileSize);
        if(ToPercent(FilePos,FileSize) == 100)
          ProcessKey(KEY_CTRLEND);
        else
          Show();
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
  if ( MsY==Y1 ) // Status line
  {
    int XTable, XPos, NameLength;
    NameLength=ScrX-40;
    if (Opt.ViewerEditorClock)
      NameLength-=6;
    if (NameLength<20)
      NameLength=20;
    XTable=NameLength+1;
    XPos=NameLength+1+10+1+10+1;
    //SysLog("MsX=%i, XTable=%i, XPos=%i",MsX,XTable,XPos);
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
  char Buf[MAX_VIEWLINE];
  int BufSize,StrPos,Skipped,I,J;
  BufSize=Min(sizeof(Buf),FilePos);
  if (BufSize==0)
    return;
  LastPage=0;
  if (VM.Hex)
  {
    int UpSize=VM.Unicode ? 8:16;
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
              StrPos+=Opt.ViewTabSize-(StrPos % Opt.ViewTabSize);
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
    /* $ 12.07.2000 SVS
       Wrap имеет 3 позиции
    */
    /* $ 15.07.2000 SVS
       Wrap должен показываться следующий, а не текущий
    */
    ViewKeyBar->Change(
       MSG(
       (!VM.Wrap)?((!VM.TypeWrap)?MViewF2:MViewShiftF2)
       :MViewF2Unwrap),1);
    ViewKeyBar->Change(KBL_SHIFT,MSG((VM.TypeWrap)?MViewF2:MViewShiftF2),1);
    /* SVS $ */
    /* SVS $ */

    if (VM.Hex)
      ViewKeyBar->Change(MSG(MViewF4Text),3);
    else
      ViewKeyBar->Change(MSG(MViewF4),3);

    if (VM.AnsiMode)
      ViewKeyBar->Change(MSG(MViewF8DOS),7);
    else
      ViewKeyBar->Change(MSG(MViewF8),7);

    ViewKeyBar->Redraw();
  }
  struct ViewerMode vm;
  memmove(&vm,&VM,sizeof(struct ViewerMode));
  CtrlObject->Plugins.CurViewer=this;
//  CtrlObject->Plugins.ProcessViewerEvent(VE_MODE,&vm);
}


void Viewer::Search(int Next,int FirstChar)
{
  const char *TextHistoryName="SearchText";
  /* $ 01.08.2000 KM
     Добавлен новый checkbox для поиска "Whole words"
  */
  static struct DialogData SearchDlgData[]={
  /* 00 */ DI_DOUBLEBOX,3,1,72,10,0,0,0,0,(char *)MViewSearchTitle,
  /* 01 */ DI_TEXT,5,2,0,0,0,0,0,0,(char *)MViewSearchFor,
  /* 02 */ DI_EDIT,5,3,70,3,1,(DWORD)TextHistoryName,DIF_HISTORY|DIF_USELASTHISTORY,0,"",
  /* 03 */ DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 04 */ DI_RADIOBUTTON,5,5,0,0,0,1,DIF_GROUP,0,(char *)MViewSearchForText,
  /* 05 */ DI_RADIOBUTTON,5,6,0,0,0,0,0,0,(char *)MViewSearchForHex,
  /* 06 */ DI_CHECKBOX,40,5,0,0,0,0,0,0,(char *)MViewSearchCase,
  /* 07 */ DI_CHECKBOX,40,6,0,0,0,0,0,0,(char *)MViewSearchWholeWords,
  /* 08 */ DI_CHECKBOX,40,7,0,0,0,0,0,0,(char *)MViewSearchReverse,
  /* 09 */ DI_TEXT,3,8,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 10 */ DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,1,(char *)MViewSearchSearch,
  /* 11 */ DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(char *)MViewSearchCancel
  };
  /* KM $ */
  MakeDialogItems(SearchDlgData,SearchDlg);

  unsigned char SearchStr[256];
  char MsgStr[256];
  unsigned long MatchPos;
  /* $ 01.08.2000 KM
     Добавлена новая переменная WholeWords
  */
  int SearchLength,Case,WholeWords,ReverseSearch,SearchHex,Match;
  /* KM $ */

  if (ViewFile==NULL || Next && *LastSearchStr==0)
    return;

  strncpy(SearchDlg[2].Data,(char *)LastSearchStr,sizeof(SearchDlg[2].Data));
  SearchDlg[4].Selected=!LastSearchHex;
  SearchDlg[5].Selected=LastSearchHex;
  SearchDlg[6].Selected=LastSearchCase;
  /* $ 01.08.2000 KM
     Инициализация checkbox'а "Whole words"
  */
  SearchDlg[7].Selected=LastSearchWholeWords;
  /* KM $ */
  SearchDlg[8].Selected=LastSearchReverse;

  if (VM.Unicode)
  {
    /* $ 21.12.2000 SVS
       Не спрячем HEX, а задизаблим.
    */
    SearchDlg[4].Selected=TRUE;
    SearchDlg[5].Flags|=DIF_DISABLE;
    SearchDlg[5].Selected=FALSE;
    /* SVS $ */
  }

  if (!Next)
  {
    Dialog Dlg(SearchDlg,sizeof(SearchDlg)/sizeof(SearchDlg[0]));
    Dlg.SetPosition(-1,-1,76,12);
    if (FirstChar)
    {
      Dlg.Show();
      Dlg.ProcessKey(FirstChar);
    }
    Dlg.Process();
    if (Dlg.GetExitCode()!=10)
      return;
  }
  strncpy((char *)SearchStr,SearchDlg[2].Data,sizeof(SearchStr));
  SearchHex=SearchDlg[5].Selected;
  Case=SearchDlg[6].Selected;
  /* $ 01.08.2000 KM
     Сохранение состояния checkbox'а "Whole words"
  */
  WholeWords=SearchDlg[7].Selected;
  /* KM $ */
  ReverseSearch=SearchDlg[8].Selected;

  strncpy((char *)LastSearchStr,(char *)SearchStr,sizeof(LastSearchStr));
  LastSearchHex=SearchHex;
  LastSearchCase=Case;
  /* $ 01.08.2000 KM
     Сохранение последнего состояния WholeWords
  */
  LastSearchWholeWords=WholeWords;
  /* KM $ */
  LastSearchReverse=ReverseSearch;

  if ((SearchLength=strlen((char *)SearchStr))==0)
    return;

  {
    SaveScreen SaveScr;
    SetCursorType(FALSE,0);
    sprintf(MsgStr,"\"%s\"",SearchStr);
    Message(0,0,MSG(MViewSearchTitle),MSG(MViewSearchingFor),MsgStr);

    if (SearchHex)
      ConvertToHex((char *)SearchStr,SearchLength);
    else
      if (!Case)
        for (int I=0;I<SearchLength;I++)
          SearchStr[I]=LocalUpper(SearchStr[I]);

    vseek(ViewFile,LastSelPos,SEEK_SET);
    Match=0;
    if (SearchLength>0 && (!ReverseSearch || LastSelPos>0))
    {
      char Buf[8192];
      /* $ 01.08.2000 KM
         Изменён тип CurPos с unsigned long на long
         из-за того, что дальше шла проверка при вычитании
         на -1, а CurPos не мог стать отрицательным и иногда
         выдавался неверный результат
      */
      long CurPos=LastSelPos;
      /* KM $ */
      int BufSize=sizeof(Buf);
      if (ReverseSearch)
      {
        /* $ 01.08.2000 KM
           Изменёно вычисление CurPos с учётом Whole words
        */
        if (WholeWords)
          CurPos-=sizeof(Buf)-SearchLength+1;
        else
          CurPos-=sizeof(Buf)-SearchLength;
        /* KM $ */
        if (CurPos<0)
          BufSize+=CurPos;
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
        if (CheckForEsc())
          return;
        if (VM.UseDecodeTable && !SearchHex && !VM.Unicode)
          for (int I=0;I<ReadSize;I++)
            Buf[I]=TableSet.DecodeTable[Buf[I]];

        /* $ 01.08.2000 KM
           Сделана сразу проверка на Case sensitive и Hex
           и если нет, тогда Buf приводится к верхнему регистру
        */
        if (!Case && !SearchHex)
          LocalUpperBuf(Buf,ReadSize);
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
          int cmpResult;
          int locResultLeft=FALSE;
          int locResultRight=FALSE;
          if (WholeWords)
          {

            int locResultLeft=FALSE;
            int locResultRight=FALSE;
            if (I!=0)
            {
              if (Buf[I]==' ' || Buf[I]=='\t' || Buf[I]=='\n' || Buf[I]=='\r')
                locResultLeft=TRUE;
              if (ReadSize!=BufSize && I+1+SearchLength>=ReadSize)
                locResultRight=TRUE;
              else
                if (Buf[I+1+SearchLength]==' ' || Buf[I+1+SearchLength]=='\t' ||
                    Buf[I+1+SearchLength]=='\n' || Buf[I+1+SearchLength]=='\r')
                  locResultRight=TRUE;

              if (!locResultLeft)
                if (strchr(Opt.WordDiv,Buf[I])!=NULL)
                  locResultLeft=TRUE;
              if (!locResultRight)
                if (strchr(Opt.WordDiv,Buf[I+1+SearchLength])!=NULL)
                  locResultRight=TRUE;

              cmpResult=locResultLeft && locResultRight && SearchStr[0]==Buf[I+1]
                && (SearchLength==1 || SearchStr[1]==Buf[I+2]
                && (SearchLength==2 || memcmp(SearchStr+2,&Buf[I+3],SearchLength-2)==0));
            }
            else
            {
              if (ReadSize!=BufSize && I+SearchLength>=ReadSize)
                locResultRight=TRUE;
              else
                if (Buf[I+SearchLength]==' ' || Buf[I+SearchLength]=='\t' ||
                    Buf[I+SearchLength]=='\n' || Buf[I+SearchLength]=='\r')
                  locResultRight=TRUE;

              if (!locResultRight)
                if (strchr(Opt.WordDiv,Buf[I+1+SearchLength])!=NULL)
                  locResultRight=TRUE;

              cmpResult=locResultRight && SearchStr[0]==Buf[I]
                && (SearchLength==1 || SearchStr[1]==Buf[I+1]
                && (SearchLength==2 || memcmp(SearchStr+2,&Buf[I+2],SearchLength-2)==0));
            }
          }
          else
          {
            cmpResult=SearchStr[0]==Buf[I] && (SearchLength==1 || SearchStr[1]==Buf[I+1]
              && (SearchLength==2 || memcmp(SearchStr+2,&Buf[I+2],SearchLength-2)==0));
          }
          if (cmpResult)
            Match=TRUE;
          else
            Match=FALSE;
          if (Match)
          {
            MatchPos=(WholeWords && I!=0)?CurPos+I+1:CurPos+I;
            break;
          }
            /* KM $ */
        }
        /* KM $ */

        if (ReadSize<sizeof(Buf))
          break;
        if (ReverseSearch)
        {
          if (CurPos<=0)
            break;
          /* $ 01.08.2000 KM
             Изменёно вычисление CurPos с учётом Whole words
          */
          if (WholeWords)
            CurPos-=sizeof(Buf)-SearchLength+1;
          else
            CurPos-=sizeof(Buf)-SearchLength;
        }
        else
        {
          if (WholeWords)
            CurPos+=sizeof(Buf)-SearchLength+1;
          else
            CurPos+=sizeof(Buf)-SearchLength;
        }
        /* KM $ */
      }
    }
  }

  if (Match)
    SelectText(MatchPos,SearchLength,0x1|(ReverseSearch?0x2:0));
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
  return(VM.TypeWrap);
}


void Viewer::SetWrapType(int TypeWrap)
{
  Viewer::VM.TypeWrap=TypeWrap;
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
//  ConvertNameToFull(Name,TempViewName, sizeof(TempViewName));
  if (ConvertNameToFull(Name,TempViewName, sizeof(TempViewName)) >= sizeof(TempViewName)){
    return;
  }
}


void Viewer::SetTitle(char *Title)
{
  if (Title==NULL)
    *Viewer::Title=0;
  else
    strcpy(Viewer::Title,Title);
}


unsigned long Viewer::GetFilePos()
{
  return(FilePos);
}


/* $ 18.07.2000 tran
   * changes 'long' to 'unsigned long' */
void Viewer::SetFilePos(unsigned long Pos)
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
  if (VM.Unicode)
  {
    char TmpBuf[16384+10];
    int i;
    char t;
    int ReadSize=fread(TmpBuf,1,Size*2,SrcFile);
    TmpBuf[ReadSize]=0;
    /* $ 20.10.2000 tran
       обратный порядок байтов */
    TmpBuf[ReadSize+1]=0;
    if ( FirstWord == 0x0FFFE )
    {
        for ( i=0; i<ReadSize; i+=2 )
        {
            t=TmpBuf[i];
            TmpBuf[i]=TmpBuf[i+1];
            TmpBuf[i+1]=t;
        }
    }
    /* tran $ */
    ReadSize+=(ReadSize & 1);
    WideCharToMultiByte(CP_OEMCP,0,(LPCWSTR)TmpBuf,ReadSize/2,Buf,Size," ",NULL);
    return(ReadSize/2);
  }
  else
    return(fread(Buf,1,Size,SrcFile));
}


int Viewer::vseek(FILE *SrcFile,unsigned long Offset,int Whence)
{
  if (VM.Unicode)
    return(fseek(SrcFile,Offset*2,Whence));
  else
    return(fseek(SrcFile,Offset,Whence));
}


unsigned long Viewer::vtell(FILE *SrcFile)
{
  unsigned long Pos=ftell(SrcFile);
  if (VM.Unicode)
    Pos=(Pos+(Pos&1))/2;
  return(Pos);
}


int Viewer::vgetc(FILE *SrcFile)
{
  if (VM.Unicode)
  {
    char TmpBuf[1];
    if (vread(TmpBuf,1,SrcFile)==0)
      return(EOF);
    return(TmpBuf[0]);
  }
  else
    return(getc(SrcFile));
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
  int Relative=0;
  /* tran 17.07.2000 $ */
  const char *LineHistoryName="ViewerOffset";
  static struct DialogData GoToDlgData[]=
  {
    /* 0 */ DI_DOUBLEBOX,3,1,31,7,0,0,0,0,(char *)MViewerGoTo,
    /* 1 */ DI_EDIT,5,2,29,2,1,(DWORD)LineHistoryName,DIF_HISTORY|DIF_USELASTHISTORY,1,"",
    /* 2 */ DI_TEXT,3,3,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    /* 3 */ DI_RADIOBUTTON,5,4,0,0,0,0,DIF_GROUP,0,(char *)MGoToPercent,
    /* 4 */ DI_RADIOBUTTON,5,5,0,0,0,0,0,0,(char *)MGoToHex,
    /* 5 */ DI_RADIOBUTTON,5,6,0,0,0,0,0,0,(char *)MGoToDecimal,
  };
  MakeDialogItems(GoToDlgData,GoToDlg);
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
      Dlg.SetHelp("ViewerGotoPos");
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
      // strncpy(PrevLine,GoToDlg[1].Data,sizeof(PrevLine));
      /* $ 17.07.2000 tran
         тут для сокращения кода ввел ptr, который и анализирую */
      char *ptr=&GoToDlg[1].Data[0];
      if ( ptr[0]=='+' || ptr[0]=='-' )     // юзер хочет относительности
      {
          if (ptr[0]=='+')
              Relative=1;
          else
              Relative=-1;
          memmove(ptr,ptr+1,strlen(ptr)); // если вы думаете про strlen(ptr)-1,
                                          // то вы ошибаетесь :)
      }
      if ( strchr(ptr,'%') )   // он хочет процентов
      {
          GoToDlg[RB_HEX].Selected=GoToDlg[RB_DEC].Selected=0;
          GoToDlg[RB_PRC].Selected=1;
      }
      else if ( strnicmp(ptr,"0x",2)==0 || ptr[0]=='$' || strchr(ptr,'h') ||strchr(ptr,'H') ) // он умный - hex код ввел!
      {
          GoToDlg[RB_PRC].Selected=GoToDlg[RB_DEC].Selected=0;
          GoToDlg[RB_HEX].Selected=1;
          if ( strnicmp(ptr,"0x",2)==0)
              memmove(ptr,ptr+2,strlen(ptr)-1); // а тут надо -1, а не -2  // сдвинем строку
          else if (ptr[0]=='$')
              memmove(ptr,ptr+1,strlen(ptr));
          //Relative=0; // при hex значении никаких относительных значений?
      }
      /* $ 19.07.2000 tran
         при форме NNNd - десятичная форма */
      else if (strchr(ptr,'d') || strchr(ptr,'D'))
      {
          GoToDlg[RB_HEX].Selected=GoToDlg[RB_PRC].Selected=0;
          GoToDlg[RB_DEC].Selected=1;
      }
      /* tran 19.07.2000 $ */
      if (GoToDlg[RB_PRC].Selected)
      {
        //int cPercent=ToPercent(FilePos,FileSize);
        PrevMode=0;
        int Percent=atoi(GoToDlg[1].Data);
        //if ( Relative  && (cPercent+Percent*Relative<0) || (cPercent+Percent*Relative>100)) // за пределы - низя
        //  return;
        if (Percent>100)
          return;
        //if ( Percent<0 )
        //  Percent=0;
        Offset=FileSize/100*Percent;
        if (VM.Unicode)
          Offset*=2;
        while (ToPercent(Offset,FileSize)<Percent)
          Offset++;
      }
      if (GoToDlg[RB_HEX].Selected)
      {
        PrevMode=1;
        char *endptr;
        Offset=strtoul(GoToDlg[1].Data,&endptr,16);
      }
      if (GoToDlg[RB_DEC].Selected)
      {
        PrevMode=2;
        char *endptr;
        Offset=strtoul(GoToDlg[1].Data,&endptr,10);
      }
    }// ShowDlg
    else
    {
      Relative=(Flags&VSP_RELATIVE)*(Offset<0?-1:1);
      if(Flags&VSP_PERCENT)
      {
        int Percent=Offset;
        if (Percent>100)
          return;
        //if ( Percent<0 )
        //  Percent=0;
        Offset=FileSize/100*Percent;
        if (VM.Unicode)
          Offset*=2;
        while (ToPercent(Offset,FileSize)<Percent)
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
  if (!VM.Hex)
  {
    char Buf[1024];
    long StartLinePos=-1,GotoLinePos=FilePos-sizeof(Buf);
    if (GotoLinePos<0)
      GotoLinePos=0;
    vseek(ViewFile,GotoLinePos,SEEK_SET);
    int ReadSize=Min(sizeof(Buf),FilePos-GotoLinePos);
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
  LastSelPos=FilePos;
  if(!(Flags&VSP_NOREDRAW))
    Show();
}


void Viewer::SetFileSize()
{
  SaveFilePos SavePos(ViewFile);
  vseek(ViewFile,0,SEEK_END);
  FileSize=vtell(ViewFile);
}


/* $ 19.01.2001 SVS
   Выделение - в качестве самостоятельной функции.
   Flags=0x01 - показывать (делать Show())
         0x02 - "обратный поиск" ?
*/
void Viewer::SelectText(long MatchPos,int SearchLength, DWORD Flags)
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
//MessageBeep(0);
  vseek(ViewFile,MatchPos+1,SEEK_SET);
  SelectPos=FilePos=MatchPos;
  SelectSize=SearchLength;
  LastSelPos=SelectPos+((Flags&0x2) ? -1:1);
  /* $ 13.03.2001 IS
     Если найденное расположено в самой первой строке юникодного файла и файл
     имеет в начале fffe или feff, то для более правильного выделения, его
     позицию нужно уменьшить на единицу (из-за того, что пустой символ не
     показывается)
  */
  SelectPosOffSet=VM.Unicode && (FirstWord==0x0FFFE || FirstWord==0x0FEFF)
           && (MatchPos+SelectSize<=ObjWidth && MatchPos<strlen(OutStr[0]));
  SelectPos-=SelectPosOffSet;
  /* IS $ */
  if (VM.Hex)
    FilePos&=~(VM.Unicode ? 0x7:0xf);
  else
  {
    if (SelectPos!=StartLinePos)
      Up();
    int Length=SelectPos-StartLinePos-1;
    if (VM.Wrap)
      Length%=ScrX+1;
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
    Show();
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
      struct ViewerInfo *Info=(struct ViewerInfo *)Param;
      if(Info)
      {
        /* $ 29.01.2001 IS
          - баг - затирался StructSize
        */
        memset(&Info->ViewerID,0,Info->StructSize-sizeof(Info->StructSize));
        /* IS */
        Info->ViewerID=Viewer::ViewerID;
        Info->FileName=FullFileName;
        Info->WindowSizeX=ObjWidth;
        Info->WindowSizeY=Y2-Y1;
        Info->FilePos=FilePos;
        Info->FileSize=FileSize;
        memmove(&Info->CurMode,&VM,sizeof(struct ViewerMode));
        Info->CurMode.TableNum=VM.UseDecodeTable ? VM.TableNum-2:-1;
        Info->Options=0;
        if (Opt.SaveViewerPos)         Info->Options|=VOPT_SAVEFILEPOSITION;
        if (Opt.ViewerAutoDetectTable) Info->Options|=VOPT_AUTODETECTTABLE;
        Info->TabSize=Opt.ViewTabSize;

        // сюды писать добавки
        if(Info->StructSize >= sizeof(struct ViewerInfo))
        {
          Info->LeftPos=LeftPos;
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
      if(Param)
      {
        struct ViewerSetPosition vsp=*(struct ViewerSetPosition*)Param;
        bool isReShow=vsp.StartPos != FilePos;
        if((LeftPos=vsp.LeftPos) < 0)
          LeftPos=0;
        GoTo(FALSE, vsp.StartPos, vsp.Flags);
        if (isReShow && !(vsp.Flags&VSP_NOREDRAW))
          ScrBuf.Flush();
        if(!(vsp.Flags&VSP_NORETNEWPOS))
        {
          ((struct ViewerSetPosition*)Param)->StartPos=FilePos;
          ((struct ViewerSetPosition*)Param)->LeftPos=LeftPos;
        }
        return(TRUE);
      }
      break;
    }

    // Param=ViewerSelect
    case VCTL_SELECT:
    {
      if(Param)
      {
        long SPos;
        int SSize;
        SPos=((ViewerSelect*)Param)->BlockStartPos;
        SSize=((ViewerSelect*)Param)->BlockLen;
        if(SPos < FileSize)
        {
          if(SPos+SSize > FileSize)
          {
            SSize=FileSize-SPos;
          }
          SelectText(SPos,SSize,0x1);
          ScrBuf.Flush();
          return(TRUE);
        }
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
        if((long)Param != (long)-1) // не только перерисовать?
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
      Show();
      ScrBuf.Flush();
      return(TRUE);
    }

    // Param=0
    case VCTL_QUIT:
    {
      if (HostFileViewer!=NULL)
        HostFileViewer->SetExitCode(0);
      return(TRUE);
    }
  }
  return(FALSE);
}
/* SVS $ */
