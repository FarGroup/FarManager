#ifndef __QVIEW_HPP__
#define __QVIEW_HPP__
/*
qview.hpp

Quick view panel

*/

/* Revision: 1.03 05.04.2001 $ */

/*
Modify:
  05.04.2001 VVM
    + Переключение макросов в режим MACRO_QVIEWPANEL
  20.02.2001 VVM
    ! Исправление поведения врапа. (Оторвал зависимость от вьюере)
  20.07.2000 tran 1.01
    - bug 21, реализовал два виртуальных метода
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class QuickView:public Panel
{
  private:
    void DisplayObject();
    void PrintText(char *Str);
    Viewer *QView;
    char CurFileName[NM];
    char CurFileType[80];
    char TempName[NM];
    int Directory;
    int PrevMacroMode;
    unsigned long DirCount,FileCount,ClusterSize;
    int64 FileSize,CompressedFileSize,RealFileSize;
    int OldWrapMode;
    int OldWrapType;
    void SetMacroMode(int Restore = FALSE);
  public:
    QuickView();
    ~QuickView();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void Update(int Mode);
    void ShowFile(char *FileName,int TempFile,HANDLE hDirPlugin);
    void CloseFile();
    void QViewDelTempName();
    int UpdateIfChanged();
    /* $ 20.07.2000 tran
       add two new virtual methos - for correct title showing*/
    virtual void SetTitle();
    virtual void SetFocus();
    /* tran 20.07.2000 $ */
    virtual void KillFocus();

};


#endif  // __QVIEW_HPP__
