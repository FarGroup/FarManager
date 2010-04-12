#ifndef __INFOLIST_HPP__
#define __INFOLIST_HPP__
/*
infolist.hpp

Информационная панель

*/

#include "panel.hpp"
#include "viewer.hpp"
//class Viewer;

/* $ 12.10.2001 SKV
  заврапим Viewer что бы отслеживать рекурсивность вызова
  методов DizView и случайно не удалить его во время вызова.
*/
class DizViewer: public Viewer
{
	public:
		int InRecursion;
		DizViewer():InRecursion(0) {}
		virtual ~DizViewer() {};
		virtual int ProcessKey(int Key)
		{
			InRecursion++;
			int res=Viewer::ProcessKey(Key);
			InRecursion--;
			return res;
		}
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
		{
			InRecursion++;
			int res=Viewer::ProcessMouse(MouseEvent);
			InRecursion--;
			return res;
		}
};

class InfoList:public Panel
{
	private:
		DizViewer *DizView;
		int  PrevMacroMode;
		int  OldWrapMode;
		int  OldWrapType;
		char DizFileName[NM];

	private:
		virtual void DisplayObject();
		void ShowDirDescription();
		void ShowPluginDescription();
		void PrintText(const char *Str);
		void PrintText(int MsgID);
		void PrintInfo(const char *Str);
		void PrintInfo(int MsgID);
		int  OpenDizFile(char *DizFile);
		void SetMacroMode(int Restore = FALSE);
		void DynamicUpdateKeyBar();

	public:
		InfoList();
		virtual ~InfoList();

	public:
		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
		virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);
		virtual void Update(int Mode);
		virtual void SetFocus();
		virtual const char *GetTitle(char *Title,int LenTitle,int TruncSize=0);
		virtual void KillFocus();
		virtual BOOL UpdateKeyBar();
		virtual void CloseFile();
		/* $ 02.01.2002 IS
		   Получить имя просматриваемого diz-файла
		*/
		virtual int GetCurName(char *Name,char *ShortName);
};

#endif  // __INFOLIST_HPP__
