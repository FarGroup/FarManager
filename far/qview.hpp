#ifndef __QVIEW_HPP__
#define __QVIEW_HPP__
/*
qview.hpp

Quick view panel

*/

#include "panel.hpp"

class Viewer;

class QuickView:public Panel
{
  private:
    Viewer *QView;
    char CurFileName[NM];
    char CurFileType[80];
    char TempName[NM];
    int Directory;
    int PrevMacroMode;
    unsigned long DirCount,FileCount,ClusterSize;
    unsigned __int64 FileSize,CompressedFileSize,RealFileSize;
    int OldWrapMode;
    int OldWrapType;

  private:
    void DisplayObject();
    void PrintText(char *Str);
    void SetMacroMode(int Restore = FALSE);
    void DynamicUpdateKeyBar();

  public:
    QuickView();
    ~QuickView();

  public:
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void Update(int Mode);
    void ShowFile(char *FileName,int TempFile,HANDLE hDirPlugin);
    void CloseFile();
    void QViewDelTempName();
    virtual int UpdateIfChanged(int UpdateMode);
    /* $ 20.07.2000 tran
       add two new virtual methos - for correct title showing*/
    virtual void SetTitle();
    virtual void GetTitle(char *Title,int LenTitle,int TruncSize=0);
    virtual void SetFocus();
    /* tran 20.07.2000 $ */
    virtual void KillFocus();
    /* $ 30.04.2001 DJ */
    virtual BOOL UpdateKeyBar();
    /* DJ $ */
    virtual int GetCurName(char *Name,char *ShortName);
};


#endif  // __QVIEW_HPP__
