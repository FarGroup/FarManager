#ifndef __TREELIST_HPP__
#define __TREELIST_HPP__
/*
treelist.hpp

Tree panel

*/

#include "panel.hpp"

struct TreeItem
{
	char Name[NM];         // им€ папки
	int Last[NM/2];        // ?
	int Depth;             // уровень вложенности
};

enum TREELIST_FLAGS
{
	FTREELIST_TREEISPREPARED          = 0x00010000,
	FTREELIST_UPDATEREQUIRED          = 0x00020000,
	FTREELIST_ISPANEL                 = 0x00040000,
};

class TreeList: public Panel
{
	private:
		int PrevMacroMode;
		char Root[NM];
		struct TreeItem *ListData;
		long TreeCount;
		long WorkDir;
		long GetSelPosition;
		int CaseSensitiveSort;
		int NumericSort;
		int ExitCode; // актуально только дл€ дерева, вызванного из копира!

		struct TreeItem *SaveListData;
		long SaveTreeCount;
		long SaveWorkDir;

	private:
		void SetMacroMode(int Restore = FALSE);
		virtual void DisplayObject();
		void DisplayTree(int Fast);
		void DisplayTreeName(char *Name,int Pos);
		void Up(int Count);
		void Down(int Count);
		void Scroll(int Count);
		void CorrectPosition();
		void FillLastData();
		int CountSlash(char *Str);
		int SetDirPosition(char *NewDir);
		void GetRoot();
		Panel* GetRootPanel();
		void SyncDir();
		void SaveTreeFile();
		int ReadTreeFile();
		virtual int GetSelCount();
		virtual int GetSelName(char *Name,int &FileAttr,char *ShortName=NULL,WIN32_FIND_DATA *fd=NULL);
		void DynamicUpdateKeyBar();
		int GetNextNavPos();
		int GetPrevNavPos();
		static char *MkTreeFileName(const char *RootDir,char *Dest,int DestSize);
		static char *MkTreeCacheFolderName(const char *RootDir,char *Dest,int DestSize);
		static char *CreateTreeFileName(const char *Path,char *Dest,int DestSize);

		bool SaveState();
		bool RestoreState();

	private:
		static int MsgReadTree(int TreeCount,int &FirstCall);
		static int GetCacheTreeName(char *Root,char *Name,int CreateDir);

	public:
		TreeList(int IsPanel=TRUE);
		virtual ~TreeList();

	public:
		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
		virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);
//  virtual void KillFocus();
		virtual void Update(int Mode);
		int ReadTree();
		virtual BOOL SetCurDir(const char *NewDir,int ClosePlugin);
		void SetRootDir(char *NewRootDir);
		virtual int GetCurDir(char *CurDir);
		virtual int GetCurName(char *Name,char *ShortName);
		virtual void UpdateViewPanel();
		virtual void MoveToMouse(MOUSE_EVENT_RECORD *MouseEvent);
		virtual int FindPartName(char *Name,int Next,int Direct=1,int ExcludeSets=0);
		virtual int GoToFile(long idxItem);
		virtual int GoToFile(const char *Name,BOOL OnlyPartName=FALSE);
		virtual long FindFile(const char *Name,BOOL OnlyPartName=FALSE);
		void ProcessEnter();

		virtual long FindFirst(const char *Name);
		virtual long FindNext(int StartPos, const char *Name);

		int GetExitCode() {return ExitCode;}
		virtual long GetFileCount() {return TreeCount;}
		virtual int GetFileName(char *Name,int Pos,int &FileAttr);

		virtual void SetTitle();
		virtual const char *GetTitle(char *Title,int LenTitle,int TruncSize=0);
		virtual void SetFocus();
		virtual void KillFocus();
		virtual BOOL UpdateKeyBar();
		virtual BOOL GetItem(int Index,void *Dest);
		virtual int GetCurrentPos();

	public:
		static void AddTreeName(char *Name);
		static void DelTreeName(char *Name);
		static void RenTreeName(char *SrcName,char *DestName);
		static void ReadSubTree(char *Path);
		static void ClearCache(int EnableFreeMem);
		static void ReadCache(char *TreeRoot);
		static void FlushCache();

		static int MustBeCached(char *Root); // $ 16.10.2000 tran - функци€, определ€юща€€ необходимость кешировани€ файла
		static void PR_MsgReadTree(void);
};

#endif  // __TREELIST_HPP__
