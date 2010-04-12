#ifndef __COMMANDLINE_HPP__
#define __COMMANDLINE_HPP__
/*
cmdline.hpp

Командная строка

*/

#include "scrobj.hpp"
#include "edit.hpp"
#include "tstack.hpp"
#include "farconst.hpp"

enum
{
	FCMDOBJ_LOCKUPDATEPANEL   = 0x00010000,
};

struct PushPopRecord
{
	char *Name;

	PushPopRecord()
	{
		Name = NULL;
	}

	~PushPopRecord();
	const PushPopRecord& operator=(const PushPopRecord &rhs);
};


class CommandLine:public ScreenObject
{
	private:
		Edit CmdStr;
		SaveScreen *BackgroundScreen;
		char CurDir[NM];
		char *LastCmdStr;
		int  LastCmdLength;
		int  LastCmdPartLength;
		TStack<PushPopRecord> ppstack;

	private:
		virtual void DisplayObject();
		int CmdExecute(char *CmdLine,int AlwaysWaitFinish,int SeparateWindow,int DirectRun);
		int ProcessOSCommands(char *CmdLine,int SeparateWindow);
		void GetPrompt(char *DestStr);
		BOOL SetLastCmdStr(const char *Ptr,int LenPtr);
		BOOL IntChDir(const char *CmdLine,int ClosePlugin,bool Selent=false);

	public:
		CommandLine();
		virtual ~CommandLine();

	public:
		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
		virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);

		int GetCurDir(char *CurDir);
		BOOL SetCurDir(const char *CurDir);
		void GetString(char *Str,int MaxSize);
		const char *GetStringAddr();
		void SetString(const char *Str,BOOL Redraw=TRUE);
		void ExecString(char *Str,int AlwaysWaitFinish,int SeparateWindow=FALSE,int DirectRun=FALSE);
		void ShowViewEditHistory();
		void InsertString(const char *Str);
		void SetCurPos(int Pos, int LeftPos=0);
		void SetPersistentBlocks(int Mode);
		void SaveBackground(int X1,int Y1,int X2,int Y2);
		void SaveBackground();
		void ShowBackground();
		void CorrectRealScreenCoord();
		virtual void ResizeConsole();
		void LockUpdatePanel(int Mode) {Flags.Change(FCMDOBJ_LOCKUPDATEPANEL,Mode);};
		int GetCurPos() { return CmdStr.GetCurPos(); };
		int GetLeftPos() { return CmdStr.GetLeftPos(); };
		int GetLength() { return CmdStr.GetLength(); };
		void GetSelString(char* Buffer, int MaxLength) { CmdStr.GetSelString(Buffer,MaxLength); };
		void Select(int Start,int End) { CmdStr.Select(Start,End); };
		void GetSelection(int &Start,int &End) { CmdStr.GetSelection(Start,End); };
};

#endif  // __COMMANDLINE_HPP__
