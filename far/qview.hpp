#ifndef __QVIEW_HPP__
#define __QVIEW_HPP__
/*
qview.hpp

Quick view panel

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
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
};


#endif	// __QVIEW_HPP__
