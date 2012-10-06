#include <CRT/crt.hpp>
#include <plugin.hpp>
#include <PluginSettings.hpp>
#include <DlgBuilder.hpp>
#include "Lang.hpp"
#include "version.hpp"
#include <initguid.h>
#include "guid.hpp"


#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"
{
#endif
	BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
	(void) lpReserved;
	(void) dwReason;
	(void) hDll;
	return TRUE;
}
#endif

const wchar_t *FindTopic(void);
BOOL IsHlf(void);
void RestorePosition(void);
BOOL CheckExtension(const wchar_t *ptrName);
void ShowHelp(const wchar_t *fullfilename,const wchar_t *topic, bool CmdLine=false);
const wchar_t *GetMsg(int MsgId);
void ShowCurrentHelpTopic();
void ShowHelpFromTempFile();
void GetPluginConfig(void);
static bool inputrecord_compare(const INPUT_RECORD &r1,const INPUT_RECORD &r2);

static struct Options
{
	int ProcessEditorInput;
	wchar_t AssignKeyName[64];
	INPUT_RECORD RecKey;
	int Style;
} Opt;

static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;

static struct EditorInfo ei={sizeof(EditorInfo)};
static struct EditorGetString egs={sizeof(EditorGetString)};
static struct EditorSetPosition esp={sizeof(EditorSetPosition)};


static INPUT_RECORD _DefKey={KEY_EVENT,{{TRUE,1,VK_F1,0x3B,{0},0}}};

BOOL CheckExtension(const wchar_t *ptrName)
{
	return (BOOL)FSF.ProcessName(L"*.hlf", (wchar_t*)ptrName, 0, PN_CMPNAME|PN_SKIPPATH);
}

void ShowHelp(const wchar_t *fullfilename,const wchar_t *topic, bool CmdLine)
{
	if (CmdLine || CheckExtension(fullfilename))
	{
		const wchar_t *Topic=topic;

		if (NULL == Topic)
			Topic=GetMsg(MDefaultTopic);

		Info.ShowHelp(fullfilename,Topic,FHELP_CUSTOMFILE);
	}
}

const wchar_t *GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid,MsgId);
}

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=PLUGIN_VERSION;
	Info->Guid=MainGuid;
	Info->Title=PLUGIN_NAME;
	Info->Description=PLUGIN_DESC;
	Info->Author=PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *PSInfo)
{
	Info=*PSInfo;
	FSF=*PSInfo->FSF;

	GetPluginConfig();
}

HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
	if (OInfo->OpenFrom==OPEN_COMMANDLINE)
	{
		const wchar_t* cmd=((OpenCommandLineInfo*)OInfo->Data)->CommandLine;
		if (lstrlen(cmd))
		{
			static wchar_t cmdbuf[1024], FileName[MAX_PATH], *ptrTopic, *ptrName;
			lstrcpyn(cmdbuf,cmd,ARRAYSIZE(cmdbuf));
			FSF.Trim(cmdbuf);
			ptrName=ptrTopic=cmdbuf;

			if (*cmdbuf==L'\"')
			{
				ptrName++;
				ptrTopic++;

				while (*ptrTopic!=L'\"' && *ptrTopic)
					ptrTopic++;
			}
			else
			{
				while (*ptrTopic!=L' ' && *ptrTopic)
					ptrTopic++;
			}

			int hasTopic = (*ptrTopic == L' ');
			*ptrTopic=0;

			wchar_t *ptrCurDir=NULL;

			if (FSF.PointToName(ptrName) == ptrName)
			{
				size_t Size=FSF.GetCurrentDirectory(0,NULL);

				if (Size)
				{
					ptrCurDir=new WCHAR[Size+lstrlen(ptrName)+8];
					FSF.GetCurrentDirectory(Size,ptrCurDir);
					lstrcat(ptrCurDir,L"\\");
					lstrcat(ptrCurDir,ptrName);
					ptrName=(wchar_t *)ptrCurDir;
				}
			}

			GetFullPathName(ptrName,MAX_PATH,FileName,&ptrName);

			if (ptrCurDir)
				delete[] ptrCurDir;

			if (hasTopic)
			{
				ptrTopic++;

				if (lstrlen(ptrTopic))
					FSF.Trim(ptrTopic);
				else
					ptrTopic=NULL;
			}
			else
			{
				ptrTopic = NULL;
			}

			ShowHelp(FileName,ptrTopic,true);
		}
		else
		{
			Info.ShowHelp(Info.ModuleName,L"cmd",FHELP_SELFHELP);
		}
	}
	else if (OInfo->OpenFrom == OPEN_EDITOR)
	{
		if (IsHlf())
		{
			ShowCurrentHelpTopic();
		}
		else
		{
			const wchar_t *Items[] = { GetMsg(MTitle), GetMsg(MNotAnHLF), GetMsg(MOk) };
			Info.Message(&MainGuid, nullptr, 0, NULL, Items, ARRAYSIZE(Items), 1);
		}
	}

	return nullptr;
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=PF_EDITOR|PF_DISABLEPANELS;

	static const wchar_t *PluginConfigStrings[1];
	PluginConfigStrings[0]=GetMsg(MTitle);
	Info->PluginConfig.Guids=&MenuGuid;
	Info->PluginConfig.Strings=PluginConfigStrings;
	Info->PluginConfig.Count=ARRAYSIZE(PluginConfigStrings);

	if (!Opt.ProcessEditorInput)
	{
		static const wchar_t *PluginMenuStrings[1];
		PluginMenuStrings[0]=GetMsg(MShowHelpTopic);
		Info->PluginMenu.Guids=&MenuGuid;
		Info->PluginMenu.Strings=PluginMenuStrings;
		Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
	}

	Info->CommandPrefix=L"HLF";
}

intptr_t WINAPI ProcessEditorInputW(const ProcessEditorInputInfo *InputInfo)
{
	LPWSTR FileName=NULL;
	BOOL Result=FALSE;

	if (Opt.ProcessEditorInput)
	{
		if (InputInfo->Rec.EventType==KEY_EVENT && InputInfo->Rec.Event.KeyEvent.bKeyDown && inputrecord_compare(InputInfo->Rec,Opt.RecKey))
		{
			Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
			size_t FileNameSize=Info.EditorControl(-1,ECTL_GETFILENAME,0,0);

			if (FileNameSize)
			{
				FileName=new wchar_t[FileNameSize];

				if (FileName)
				{
					Info.EditorControl(-1,ECTL_GETFILENAME,FileNameSize,FileName);
				}
			}

			if (CheckExtension(FileName))
			{
				ShowCurrentHelpTopic();
				Result=TRUE;
			}
		}
	}

	if (FileName)
	{
		delete[] FileName;
	}

	return Result;
}

void ShowCurrentHelpTopic()
{
	size_t FileNameSize=Info.EditorControl(-1,ECTL_GETFILENAME,0,0);
	LPWSTR FileName=NULL;
	Info.EditorControl(-1,ECTL_GETINFO,0,&ei);

	if (FileNameSize)
	{
		FileName=new wchar_t[FileNameSize];

		if (FileName)
		{
			Info.EditorControl(-1,ECTL_GETFILENAME,0,FileName);
		}
	}

	switch (Opt.Style)
	{
		case 1:

			if (!(ei.CurState&ECSTATE_SAVED))
				ShowHelpFromTempFile();
			else
				ShowHelp(FileName,FindTopic());

			break;
		case 2:

			if (!(ei.CurState&ECSTATE_SAVED))
				Info.EditorControl(-1,ECTL_SAVEFILE, 0, 0);

		default:
			ShowHelp(FileName,FindTopic());
			break;
	}

	if (FileName)
	{
		delete[] FileName;
	}

}

void ShowHelpFromTempFile()
{
	struct EditorSaveFile esf={sizeof(EditorSaveFile)};
	wchar_t fname[MAX_PATH];
	esf.FileName = fname;

	if (FSF.MkTemp(fname, ARRAYSIZE(fname)-4,L"HLF")>1)
	{
		lstrcat(fname,L".hlf");
		/*
		  esf.FileEOL=NULL;
		  Info.EditorControl(ECTL_SAVEFILE, &esf);
		  ShowHelp(esf.FileName, FindTopic());
		  DeleteFile(esf.FileName);
		*/
		struct EditorGetString egs={sizeof(EditorGetString)};
		struct EditorInfo ei={sizeof(EditorInfo)};
		DWORD Count;

		HANDLE Handle=CreateFile(esf.FileName, GENERIC_WRITE, FILE_SHARE_READ,
		                         NULL, CREATE_NEW, 0, NULL);

		if (Handle != INVALID_HANDLE_VALUE)
		{
			Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
		#define SIGN_UNICODE    0xFEFF
			WORD sign=SIGN_UNICODE;
			WriteFile(Handle, &sign, 2, &Count, NULL);

			for (egs.StringNumber=0; egs.StringNumber<ei.TotalLines; egs.StringNumber++)
			{
				Info.EditorControl(-1,ECTL_GETSTRING,0,&egs);
				WriteFile(Handle, egs.StringText, (DWORD)(egs.StringLength*sizeof(wchar_t)), &Count, NULL);
				WriteFile(Handle, L"\r\n", 2*sizeof(wchar_t), &Count, NULL);
			}

			CloseHandle(Handle);

			ShowHelp(esf.FileName, FindTopic());

			DeleteFile(esf.FileName);
		}
	}
	else
	{
		; //??
	}
}

intptr_t WINAPI ConfigureW(const ConfigureInfo* CfgInfo)
{
	GetPluginConfig();

	PluginDialogBuilder Builder(Info, MainGuid, DialogGuid, MConfig, L"Config");

	Builder.StartColumns();
	Builder.AddCheckbox(MProcessEditorInput, &Opt.ProcessEditorInput);
	Builder.ColumnBreak();
	FarDialogItem *ItemAssignKeyName=Builder.AddEditField(Opt.AssignKeyName, ARRAYSIZE(Opt.AssignKeyName), 20);
	Builder.EndColumns();

    Builder.AddSeparator();

	Builder.AddText(MStyle);

    const int StyleIDs[] = {MStr1, MStr2, MStr3};
    Builder.AddRadioButtons((int*)&Opt.Style, ARRAYSIZE(StyleIDs), StyleIDs);

    Builder.AddOKCancel(MOk, MCancel);

	if (Builder.ShowDialog())
	{
		lstrcpyn(Opt.AssignKeyName,ItemAssignKeyName->Data,ARRAYSIZE(Opt.AssignKeyName));
		if (!FSF.FarNameToInputRecord(Opt.AssignKeyName,&Opt.RecKey))
		{
			lstrcpyn(Opt.AssignKeyName,L"F1",ARRAYSIZE(Opt.AssignKeyName));
			Opt.RecKey=_DefKey;
		}

		PluginSettings settings(MainGuid, Info.SettingsControl);
		settings.Set(0,L"ProcessEditorInput",Opt.ProcessEditorInput);
		settings.Set(0,L"Style",Opt.Style);
		settings.Set(0,L"EditorKey",Opt.AssignKeyName);
		return TRUE;
	}
	return FALSE;
}

void RestorePosition(void)
{
	esp.CurLine=ei.CurLine;
	esp.CurPos=ei.CurPos;
	esp.TopScreenLine=ei.TopScreenLine;
	esp.LeftPos=ei.LeftPos;
	esp.CurTabPos=-1;
	esp.Overtype=-1;
	Info.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
}

BOOL IsHlf(void)
{
	BOOL ret=FALSE;
	Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
	memset(&esp,-1,sizeof(esp));
	egs.StringNumber=-1;
	intptr_t total=(ei.TotalLines<3)?ei.TotalLines:3;

	if (total>2) for (esp.CurLine=0; esp.CurLine<total; esp.CurLine++)
	{
		Info.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
		Info.EditorControl(-1,ECTL_GETSTRING,0,&egs);

		if (!FSF.LStrnicmp(_T(".Language="),egs.StringText,10))
		{
			ret=TRUE;
			break;
		}
	}

	RestorePosition();
	return ret;
}

const wchar_t *FindTopic(void)
{
	const wchar_t *ret=NULL;
	const wchar_t *tmp;
	Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
	memset(&esp,-1,sizeof(esp));
	egs.StringNumber=-1;

	for (esp.CurLine=ei.CurLine; esp.CurLine>=0; esp.CurLine--)
	{
		Info.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
		Info.EditorControl(-1,ECTL_GETSTRING,0,&egs);
		tmp=egs.StringText;

		if (lstrlen(tmp)>1 && *tmp==L'@' && *(tmp+1)!=L'-' && *(tmp+1)!=L'+')
		{
			ret=tmp+1;
			break;
		}
	}

	RestorePosition();
	return ret;
}

void GetPluginConfig(void)
{
	PluginSettings settings(MainGuid, Info.SettingsControl);

	settings.Get(0,L"EditorKey",Opt.AssignKeyName,ARRAYSIZE(Opt.AssignKeyName),L"F1");

	if (!FSF.FarNameToInputRecord(Opt.AssignKeyName,&Opt.RecKey))
	{
		lstrcpyn(Opt.AssignKeyName,L"F1",ARRAYSIZE(Opt.AssignKeyName));
		Opt.RecKey=_DefKey;
	}

	Opt.ProcessEditorInput=settings.Get(0,L"ProcessEditorInput",1);
	Opt.Style=settings.Get(0,L"Style",0);
}

static bool inputrecord_compare(const INPUT_RECORD &r1,const INPUT_RECORD &r2)
{
	if (r1.EventType == r2.EventType)
	{
		switch(r1.EventType)
		{
			case KEY_EVENT:   // Event contains key event record
				#define RMASK (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED|RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED|SHIFT_PRESSED)

				return r1.Event.KeyEvent.wVirtualKeyCode == r2.Event.KeyEvent.wVirtualKeyCode &&
					(r1.Event.KeyEvent.dwControlKeyState&RMASK) == (r2.Event.KeyEvent.dwControlKeyState&RMASK);

			case MOUSE_EVENT: // Event contains mouse event record
				return r1.Event.MouseEvent.dwButtonState == r2.Event.MouseEvent.dwButtonState &&
					r1.Event.MouseEvent.dwControlKeyState == r2.Event.MouseEvent.dwControlKeyState &&
					r1.Event.MouseEvent.dwEventFlags == r2.Event.MouseEvent.dwEventFlags;
		}

		return false;
	}

	return false;
}
