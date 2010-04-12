#ifndef __FINDFILES_HPP__
#define __FINDFILES_HPP__
/*
findfile.hpp

Поиск (Alt-F7)

*/


#include "plugin.hpp"

enum FindFileFlags
{
	FFSEARCH_ALL=0,
	FFSEARCH_ALL_BUTNETWORK,
	FFSEARCH_INPATH,
	FFSEARCH_ROOT,
	FFSEARCH_FROM_CURRENT,
	FFSEARCH_CURRENT_ONLY,
	FFSEARCH_SELECTED
};

typedef struct _ARCLIST
{
	char ArcName[NM];
	HANDLE hPlugin;    // Plugin handle
	DWORD Flags;       // OpenPluginInfo.Flags
	char RootPath[NM]; // Root path in plugin after opening.
} ARCLIST, *LPARCLIST;

typedef struct _FINDLIST
{
	WIN32_FIND_DATA   FindData;
	DWORD             ArcIndex;
	DWORD             Used;
//  BYTE Addons[6];
} FINDLIST, *LPFINDLIST;

class FindFiles
{
	private:

		static BOOL FindListGrow();
		static BOOL ArcListGrow();
		static DWORD AddFindListItem(WIN32_FIND_DATA *FindData);
		static DWORD AddArcListItem(const char *ArcName, HANDLE hPlugin,
		                            DWORD dwFlags, const char *RootPath);
		static void ClearAllLists();
		static void AdvancedDialog();

		int FindFilesProcess();
		static LONG_PTR WINAPI FindDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);
		static LONG_PTR WINAPI MainDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);
		static LONG_PTR WINAPI AdvancedDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2);

		static void SetPluginDirectory(char *DirName,HANDLE hPlugin,int UpdatePanel=FALSE);
		static void _cdecl DoScanTree(char* Root, WIN32_FIND_DATA& FindData, char* FullName, size_t cbFullName);
		static void _cdecl PrepareFilesList(void *Param);
		static void _cdecl PreparePluginList(void *Param);
		static void _cdecl WriteDialogData(void *Param);
		static void ScanPluginTree(HANDLE hPlugin, DWORD Flags);
		static int LookForString(char *Name);
		static int IsFileIncluded(PluginPanelItem *FileItem,char *FullName,DWORD FileAttr);
		static void ArchiveSearch(char *ArcName);
//    static void AddMenuRecord(char *FullName,char *Path,WIN32_FIND_DATA *FindData);
		static void AddMenuRecord(char *FullName, WIN32_FIND_DATA *FindData);
//    static void RereadPlugin(HANDLE hPlugin);
		static int GetPluginFile(DWORD ArcIndex, struct PluginPanelItem *PanelItem,
		                         char *DestPath, char *ResultName);
		static char *PrepareDriveNameStr(char *SearchFromRoot,size_t sz);
		static char *RemovePseudoBackSlash(char *FileName);
		static __int64 __fastcall GetSearchInFirst(char *DigitStr);

	public:

		FindFiles();
		~FindFiles();

};


#endif  // __FINDFILES_HPP__
