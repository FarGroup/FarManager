#ifndef __QVIEW_HPP__
#define __QVIEW_HPP__
/*
qview.hpp

Quick view panel

*/

/* Revision: 1.01 20.07.2000 $ */

/*
Modify:
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
    unsigned long DirCount,FileCount,ClusterSize;
    int64 FileSize,CompressedFileSize,RealFileSize;
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

};


#endif  // __QVIEW_HPP__
