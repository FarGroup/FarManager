#ifndef __VIEWER_HPP__
#define __VIEWER_HPP__
/*
viewer.hpp

Internal viewer

*/

/* Revision: 1.16 30.04.2001 $ */

/*
Modify:
  30.04.2001 DJ
    + GetAnsiMode(), GetHexMode()
  27.04.2001 DJ
    * DrawScrollbar(), AdjustWidth(), AdjustFilePos()
  29.03.2001 IS
    + структура ViOpt и Get/Set для ее обслуживания
  20.02.2001 VVM
    + GetWrapType()/SetWrapType()
  06.02.2001 IS
    + SelectPosOffSet;
  19.01.2001 SVS
    ! GoTo - с параметрами & public member
    + SelectText()
  27.09.2000 SVS
    + ViewerControl - "Ядро" будущего Viewer API :-)
    + FileViewer *HostFileViewer;
    ! Переменные UseDecodeTable,TableNum,AnsiText,Unicode,Wrap, TypeWrap, Hex
      введены в одну структуру ViewerMode.
  14.06.2000 SVS
    + Переменная FirstWord - первое слово из файла
      (для автоопределения Unicode)
  12.09.2000 SVS
    + Введена переменная TypeWrap. Теперь
      Wrap - Состояние (Wrap/UnWrap) и
      TypeWrap - тип (Wrap/WWrap)
  30.07.2000 KM 1.07
    + LastSearchWholeWords
  19.07.2000 tran 1/06
    + Viewer::Width, ::XX2
  18.07.2000 tran 1.05
    * изменил тип параметра у SetFilePos()
      на unsigned
  12.07.2000 tran
    ! OutStr are dynamic, new, delete,
      and sizeof(OutStr[i]) changed to MAX_VIEWLINEB
  12.07.2000 SVS
    - из-за увеличения длины строки до 0x800 вылетал FAR
      по Alt-F7. Сократим MAX_VIEWLINE до 1024 (0x400)
  10.07.2000 tran
    + увеличение длины строки - с 512 на MAX_VIEWLINE
      MAX_VIEWLINEB = MAX_VIEWLINE + 16
  04.07.2000 tran
    + 'warning' parameter in OpenFile() method
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

/* $ 12.07.2000 SVS
  - из-за увеличения длины строки до 0x800 вылетал FAR
    по Alt-F7. Сократим MAX_VIEWLINE до 1024 (0x400)
*/
#define MAX_VIEWLINE  0x800 // 0x400
#define MAX_VIEWLINEB 0x80f // 0x40f
/* SVS $ */

class FileViewer;
class Viewer:public ScreenObject
{
  private:
    /* $ 29.03.2001 IS
         Часть локальных настроек переехала в ViewerOptions
    */
    struct ViewerOptions ViOpt;
    /* IS $ */
    /* $ 14.06.2000 SVS
      + Переменная FirstWord - первое слово из файла
      (для автоопределения Unicode)
    */
    WORD FirstWord;
    /* SVS $ */

    NamesList ViewNamesList;
    KeyBar *ViewKeyBar;
    /* $ 12.07.2000 tran
     dymanic alloc memory for OutStr */
    char *OutStr[MAXSCRY+1]; //[MAX_VIEWLINEB];
    /* tran 12.07.2000 $ */
    int StrFilePos[MAXSCRY+1];
    char FileName[NM];
    char FullFileName[NM];
    FILE *ViewFile;
    WIN32_FIND_DATA ViewFindData;

    unsigned char LastSearchStr[256];
    /* $ 30.07.2000 KM
       Новая переменная для поиска
    */
    int LastSearchCase,LastSearchWholeWords,LastSearchReverse,LastSearchHex;
    /* KM $ */

    struct CharTableSet TableSet;
    /* $ 27.09.2000 SVS
       Переменные "mode" вогнаны под одну крышу
    */
    struct ViewerMode VM;
    /* SVS $ */

    unsigned long FilePos;
    unsigned long SecondPos;
    unsigned long LastScrPos;
    unsigned long FileSize;
    unsigned long LastSelPos;
    int LeftPos;
    int LastPage;
    int CRSym;
    int SelectPos,SelectSize;
    /* $ 06.02.2001 IS
       Используется для коррекции позиции выделения в юникодных файлах
    */
    int SelectPosOffSet;
    /* IS $ */
    int ViewY1;
    int ShowStatusLine,HideCursor;
    char TempViewName[NM];
    char Title[512];
    char PluginData[NM*2];
    int TableChangedByUser;
    int ReadStdin;
    int InternalKey;

    unsigned long SavePosAddr[10];
    int SavePosLeft[10];

    unsigned long UndoAddr[128];
    int UndoLeft[128];
    int LastKeyUndo;
    /* $ 19.07.2000 tran
       новая переменная, используется при расчете ширины при скролбаре */
    int Width,XX2;
    /* tran 19.07.2000 $ */
    /* $ 27.09.2000 SVS
    */
    int ViewerID;
    bool OpenFailed;
    FileViewer *HostFileViewer;
    /* SVS $ */

  private:
    void DisplayObject();
    void Up();
    void ShowHex();
    void ShowUp();
    void ShowStatus();
    /* $ 27.04.2001 DJ
       функции для рисования скроллбара, для корректировки ширины в
       зависимости от наличия скроллбара и для корректировки позиции файла
       на границу строки
    */
    void DrawScrollbar();
    void AdjustWidth();
    void AdjustFilePos();
    /* DJ $ */
    void ReadString(char *Str,int MaxSize,int StrSize,int &SelPos,int &SelSize);
    int CalcStrSize(char *Str,int Length);
    void ChangeViewKeyBar();
    void SetCRSym();
    void Search(int Next,int FirstChar);
    void ConvertToHex(char *SearchStr,int &SearchLength);
    int HexToNum(int Hex);
    int vread(char *Buf,int Size,FILE *SrcFile);
    int vseek(FILE *SrcFile,unsigned long Offset,int Whence);
    unsigned long vtell(FILE *SrcFile);
    int vgetc(FILE *SrcFile);
    void SetFileSize();

  public:
    Viewer();
    ~Viewer();


  public:
    /* $ 04.07.2000 tran
       + 'warning' parameter */
    int OpenFile(char *Name,int warning);
    /* tran $ */
    void SetViewKeyBar(KeyBar *ViewKeyBar);
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void SetStatusMode(int Mode);
    void EnableHideCursor(int HideCursor);
    int GetWrapMode();
    void SetWrapMode(int Wrap);
    int GetWrapType();
    void SetWrapType(int TypeWrap);
    void KeepInitParameters();
    void GetFileName(char *Name);
    void ShowConsoleTitle();
    void SetTempViewName(char *Name);
    void SetTitle(char *Title);
    unsigned long GetFilePos();
    /* $ 18.07.2000 tran - change 'long' to 'unsigned long' */
    void SetFilePos(unsigned long Pos);
    void SetPluginData(char *PluginData);
    void SetNamesList(NamesList *List);
    /* $ 27.09.2000 SVS
       "Ядро" будущего Viewer API :-)
    */
    int  ViewerControl(int Command,void *Param);
    void SetHostFileViewer(FileViewer *Viewer) {HostFileViewer=Viewer;};
    /* SVS $ */

    void GoTo(int ShowDlg=TRUE,__int64 NewPos=0,DWORD Flags=0);
    // Функция выделения - как самостоятельная функция
    void SelectText(long MatchPos,int SearchLength, DWORD Flags=0x1);
    /* $ 29.03.2001 IS
         Манипуляции с ViewerOptions
    */
    int GetTabSize() const { return ViOpt.TabSize; }
    void SetTabSize(int newValue) { ViOpt.TabSize=newValue; }

    int GetAutoDetectTable() const { return ViOpt.AutoDetectTable; }
    void SetAutoDetectTable(int newValue) { ViOpt.AutoDetectTable=newValue; }

    int GetShowScrollbar() const { return ViOpt.ShowScrollbar; }
    void SetShowScrollbar(int newValue) { ViOpt.ShowScrollbar=newValue; }

    int GetShowArrows() const { return ViOpt.ShowArrows; }
    void SetShowArrows(int newValue) { ViOpt.ShowArrows=newValue; }
    /* IS $ */

    /* $ 30.04.2001 DJ */
    int GetAnsiMode() const { return VM.AnsiMode; }
    int GetHexMode() const { return VM.Hex; }
    /* DJ $ */
};

#endif // __VIEWER_HPP__
