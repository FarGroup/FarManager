#ifndef __FINDFILES_HPP__
#define __FINDFILES_HPP__
/*
findfile.hpp

Поиск (Alt-F7)

*/

/* Revision: 1.03 31.07.2001 $ */

/*
Modify:
  31.07.2001 KM
    Перенос статических функций из модуля в тело класса.
  05.06.2001 SVS
    + Обработчик диалога (без него нажатие на пимпу "[View]" заваливает ФАР)
  06.05.2001 DJ
    ! перетрях #include
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "plugin.hpp"

#undef SEARCH_ALL
enum {
  SEARCH_ALL=0,
  SEARCH_ROOT,
  SEARCH_FROM_CURRENT,
  SEARCH_CURRENT_ONLY,
  SEARCH_SELECTED
};

class FindFiles
{
  private:
    int FindFilesProcess();
    static long WINAPI FindDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    static long WINAPI MainDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);

    static void SetPluginDirectory(char *FileName);
    static void _cdecl PrepareFilesList(void *Param);
    static void _cdecl PreparePluginList(void *Param);
    static void _cdecl WriteDialogData(void *Param);
    static void ScanPluginTree();
    static int LookForString(char *Name);
    static int IsFileIncluded(PluginPanelItem *FileItem,char *FullName,DWORD FileAttr);
    static void ArchiveSearch(char *ArcName);
    static void AddMenuRecord(char *FullName,char *Path,WIN32_FIND_DATA *FindData);
    static void RereadPlugin(HANDLE hPlugin);

  public:
    FindFiles();
    ~FindFiles();

};


#endif	// __FINDFILES_HPP__
