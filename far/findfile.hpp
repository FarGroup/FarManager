#ifndef __FINDFILES_HPP__
#define __FINDFILES_HPP__
/*
findfile.hpp

Поиск (Alt-F7)

*/

/* Revision: 1.23 06.06.2006 $ */

#include "plugin.hpp"
#include "unicodestring.hpp"
#include "struct.hpp"

#undef SEARCH_ALL
enum {
  SEARCH_ALL=0,
  SEARCH_ALL_BUTNETWORK,
  SEARCH_INPATH,
  SEARCH_ROOT,
  SEARCH_FROM_CURRENT,
  SEARCH_CURRENT_ONLY,
  SEARCH_SELECTED
};

typedef struct _ARCLIST {
  string strArcName;
  HANDLE hPlugin;    // Plugin handle
  DWORD Flags;       // OpenPluginInfo.Flags
  string strRootPath; // Root path in plugin after opening.
} ARCLIST, *LPARCLIST;

typedef struct _FINDLIST {
  FAR_FIND_DATA_EX  FindData;
  DWORD             ArcIndex;
  DWORD             Used;
//  BYTE Addons[6];
} FINDLIST, *LPFINDLIST;

class FindFiles
{
  private:

    static BOOL FindListGrow();
    static BOOL ArcListGrow();
    static DWORD AddFindListItem(FAR_FIND_DATA_EX *FindData);
    static DWORD AddArcListItem(const wchar_t *ArcName, HANDLE hPlugin,
                                DWORD dwFlags, const wchar_t *RootPath);
    static void ClearAllLists();
    static void AdvancedDialog();

    int FindFilesProcess();
    static long WINAPI FindDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    static long WINAPI MainDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    static long WINAPI AdvancedDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);

    static void SetPluginDirectory(const wchar_t *DirName,HANDLE hPlugin,int UpdatePanel=FALSE);
    static void _cdecl PrepareFilesList(void *Param);
    static void _cdecl PreparePluginList(void *Param);
    static void _cdecl WriteDialogData(void *Param);
    static void ScanPluginTree(HANDLE hPlugin, DWORD Flags);
    static int LookForString(const wchar_t *Name);
    static int IsFileIncluded(PluginPanelItemW *FileItem,const wchar_t *FullName,DWORD FileAttr);
    static void ArchiveSearch(const wchar_t *ArcName);
//    static void AddMenuRecord(char *FullName,char *Path,WIN32_FIND_DATA *FindData);
    static void AddMenuRecord(const wchar_t *FullName, FAR_FIND_DATA_EX *FindData);
    static void AddMenuRecord(const wchar_t *FullName, FAR_FIND_DATA *FindData);
//    static void RereadPlugin(HANDLE hPlugin);
    static int GetPluginFile(DWORD ArcIndex, struct PluginPanelItemW *PanelItem,
                             const wchar_t *DestPath, string &strResultName);
    static string &PrepareDriveNameStr(string &strSearchFromRoot,size_t sz);
    static char *RemovePseudoBackSlash(char *FileName);
    static wchar_t *RemovePseudoBackSlashW(wchar_t *FileName);
    static string &RemovePseudoBackSlashW(string &strFileName);
    static __int64 __fastcall GetSearchInFirstW(const wchar_t *DigitStr);

  public:

    FindFiles();
    ~FindFiles();

};


#endif  // __FINDFILES_HPP__
