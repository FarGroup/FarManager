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
		virtual void DisplayObject();
		void PrintText(char *Str);
		void SetMacroMode(int Restore = FALSE);
		void DynamicUpdateKeyBar();

	public:
		QuickView();
		virtual ~QuickView();

	public:
		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
		virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);

		virtual void Update(int Mode);
		void ShowFile(char *FileName,int TempFile,HANDLE hDirPlugin);
		virtual void CloseFile();
		virtual void QViewDelTempName();
		virtual int UpdateIfChanged(int UpdateMode);
		virtual void SetTitle();
		virtual const char *GetTitle(char *Title,int LenTitle,int TruncSize=0);
		virtual void SetFocus();
		virtual void KillFocus();
		virtual BOOL UpdateKeyBar();
		virtual int GetCurName(char *Name,char *ShortName);
};


#endif  // __QVIEW_HPP__
