#ifndef __VIEWER_HPP__
#define __VIEWER_HPP__
/*
viewer.hpp

Internal viewer

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/


class Viewer:public ScreenObject
{
  private:
    void DisplayObject();
    void Up();
    void ShowHex();
    void ShowUp();
    void ShowStatus();
    void ReadString(char *Str,int MaxSize,int StrSize,int &SelPos,int &SelSize);
    int CalcStrSize(char *Str,int Length);
    void ChangeViewKeyBar();
    void SetCRSym();
    void Search(int Next,int FirstChar);
    void ConvertToHex(char *SearchStr,int &SearchLength);
    int HexToNum(int Hex);
    int vread(char *Buf,int Size,FILE *SrcFile);
    int vseek(FILE *SrcFile,long Offset,int Whence);
    unsigned long vtell(FILE *SrcFile);
    int vgetc(FILE *SrcFile);
    void GoTo();
    void SetFileSize();

    NamesList ViewNamesList;
    KeyBar *ViewKeyBar;
    char OutStr[MAXSCRY+1][528];
    int StrFilePos[MAXSCRY+1];
    char FileName[NM];
    char FullFileName[NM];
    FILE *ViewFile;
    WIN32_FIND_DATA ViewFindData;

    unsigned char LastSearchStr[256];
    int LastSearchCase,LastSearchReverse,LastSearchHex;

    struct CharTableSet TableSet;
    int UseDecodeTable,TableNum,AnsiText;
    int Unicode;

    int Wrap,Hex;

    unsigned long FilePos;
    unsigned long SecondPos;
    unsigned long LastScrPos;
    unsigned long FileSize;
    unsigned long LastSelPos;
    int LeftPos;
    int LastPage;
    int CRSym;
    int SelectPos,SelectSize;
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
  public:
    Viewer();
    ~Viewer();
    int OpenFile(char *Name);
    void SetViewKeyBar(KeyBar *ViewKeyBar);
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void SetStatusMode(int Mode);
    void EnableHideCursor(int HideCursor);
    int GetWrapMode();
    void SetWrapMode(int Wrap);
    void KeepInitParameters();
    void GetFileName(char *Name);
    void ShowConsoleTitle();
    void SetTempViewName(char *Name);
    void SetTitle(char *Title);
    long GetFilePos();
    void SetFilePos(long Pos);
    void SetPluginData(char *PluginData);
    void SetNamesList(NamesList *List);
};

#endif	// __VIEWER_HPP__

