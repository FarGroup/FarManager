#ifndef __FINDFILES_HPP__
#define __FINDFILES_HPP__
/*
findfile.hpp

Поиск (Alt-F7)

*/

/* Revision: 1.08 23.01.2002 $ */

/*
Modify:
  23.01.2002 VVM
    + GetPluginFile() - получить файл для просмотра с панели плагина.
      В отличие от предыдущего подхода - учитывает вложенность папок
      и делает для них SetDirectory()
  16.01.2002 VVM
    ! В функцию AddMenuRecord не передается параметр Path, он там лишний...
  13.10.2001 VVM
    ! Баг при поиске в темп-панели.
    + Новая функция - очистить списки. Что-бы небыло перерасхода памяти
      после нажатия на [New search]
  12.10.2001 VVM
    ! Очередной перетрях поиска. Избавляемся от глобальных переменных,
      которые могут конфликтовать. Список архивов теперь хранит хэндл
      архива и флаги для этого архива.
  09.10.2001 VVM
    ! Переделка поиска - ускорение неимеверное :))))
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

typedef struct _ARCLIST {
  char ArcName[NM];
  HANDLE hPlugin;  // Plugin handle
  DWORD Flags;     // OpenPluginInfo.Flags
} ARCLIST, *LPARCLIST;

typedef struct _FINDLIST {
  WIN32_FIND_DATA   FindData;
  DWORD             ArcIndex;
//  BYTE Addons[6];
} FINDLIST, *LPFINDLIST;

class FindFiles
{
  private:

    static BOOL FindListGrow();
    static BOOL ArcListGrow();
    static DWORD AddFindListItem(WIN32_FIND_DATA *FindData);
    static DWORD AddArcListItem(char *ArcName);
    static void ClearAllLists();

    int FindFilesProcess();
    static long WINAPI FindDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    static long WINAPI MainDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);

    static void SetPluginDirectory(char *FileName, HANDLE hPlugin);
    static void _cdecl PrepareFilesList(void *Param);
    static void _cdecl PreparePluginList(void *Param);
    static void _cdecl WriteDialogData(void *Param);
    static void ScanPluginTree(HANDLE hPlugin, DWORD Flags);
    static int LookForString(char *Name);
    static int IsFileIncluded(PluginPanelItem *FileItem,char *FullName,DWORD FileAttr);
    static void ArchiveSearch(char *ArcName);
//    static void AddMenuRecord(char *FullName,char *Path,WIN32_FIND_DATA *FindData);
    static void AddMenuRecord(char *FullName, WIN32_FIND_DATA *FindData);
    static void RereadPlugin(HANDLE hPlugin);
    static int GetPluginFile(HANDLE hPlugin, struct PluginPanelItem *PanelItem,
                             char *DestPath, char *ResultName);
  public:
    FindFiles();
    ~FindFiles();

};


#endif  // __FINDFILES_HPP__
