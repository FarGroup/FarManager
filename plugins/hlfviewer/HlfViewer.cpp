#include <plugin.hpp>
#include <PluginSettings.hpp>
#include <DlgBuilder.hpp>
#include "Lang.hpp"
#include "version.hpp"
#include <rpc.h>

#include "guid.hpp"
#include <initguid.h>
#include "guid.hpp"


wchar_t *GetEditorFileName();
const wchar_t *FindTopic(bool ForwardDirect=false, bool RestorePos=true);
BOOL IsHlf();
void RestorePosition(EditorInfo *ei);
bool ShowHelp(const wchar_t *fullfilename,const wchar_t *topic, bool CmdLine=false, bool ShowError=true);
const wchar_t *GetMsg(int MsgId);
bool ShowCurrentHelpTopic();
static void ShowHelpFromTempFile();
void GetPluginConfig();
static bool inputrecord_compare(const INPUT_RECORD &r1,const INPUT_RECORD &r2);
bool FindPluginHelp(const wchar_t* Name,wchar_t* DestPath);

static struct Options
{
	int ProcessEditorInput;
	int CheckMaskFile;
	wchar_t MaskFile[MAX_PATH];
	wchar_t AssignKeyName[64];
	INPUT_RECORD RecKey;
	int Style;
} Opt;

static PluginStartupInfo PsInfo;
static FarStandardFunctions FSF;

static INPUT_RECORD _DefKey={KEY_EVENT,{{TRUE,1,VK_F1,0x3B,{0},0}}};


static bool FileExists(const wchar_t* Name)
{
	return GetFileAttributes(Name) != INVALID_FILE_ATTRIBUTES;
}

static bool StrToGuid(const wchar_t* Value,GUID* Guid)
{
	return UuidFromString(reinterpret_cast<unsigned short*>(const_cast<wchar_t*>(Value)), Guid) == RPC_S_OK;
}

static bool CheckExtension(const wchar_t *ptrName)
{
	return !(Opt.CheckMaskFile && *Opt.MaskFile) || FSF.ProcessName(Opt.MaskFile, const_cast<wchar_t*>(ptrName), 0, PN_CMPNAMELIST | PN_SKIPPATH);
}

bool ShowHelp(const wchar_t *fullfilename,const wchar_t *topic, bool CmdLine, bool ShowError)
{
	if (fullfilename && (CmdLine || CheckExtension(fullfilename)))
	{
		const wchar_t *Topic=topic;

		if (!Topic)
			Topic=GetMsg(MDefaultTopic);

		return PsInfo.ShowHelp(fullfilename,Topic,FHELP_CUSTOMFILE|(ShowError?0:FHELP_NOSHOWERROR))?true:false;
	}

	return false;
}

const wchar_t *GetMsg(int MsgId)
{
	return PsInfo.GetMsg(&MainGuid,MsgId);
}

void WINAPI GetGlobalInfoW(GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=PLUGIN_VERSION;
	Info->Guid=MainGuid;
	Info->Title=PLUGIN_NAME;
	Info->Description=PLUGIN_DESC;
	Info->Author=PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const PluginStartupInfo *Info)
{
	PsInfo=*Info;
	FSF=*PsInfo.FSF;
	PsInfo.FSF=&FSF;

	GetPluginConfig();
}

HANDLE WINAPI OpenW(const OpenInfo *Info)
{
	if (Info->OpenFrom == OPEN_EDITOR || Info->OpenFrom==OPEN_FROMMACRO)
	{
		HANDLE MacroResult=nullptr;
		// в редакторе проверяем файл на принадлежность к системе помощи Far Manager
		if (IsHlf())
		{
			if (ShowCurrentHelpTopic())
				MacroResult=INVALID_HANDLE_VALUE;
		}
		else if (Info->OpenFrom!=OPEN_FROMMACRO)
		{
			const wchar_t *Items[] = { GetMsg(MTitle), GetMsg(MNotAnHLF), GetMsg(MOk) };
			PsInfo.Message(&MainGuid, nullptr, 0, {}, Items, ARRAYSIZE(Items), 1);
		}

		return (Info->OpenFrom==OPEN_FROMMACRO)?MacroResult:nullptr;
	}

	if (Info->OpenFrom==OPEN_COMMANDLINE)
	{
		static wchar_t cmdbuf[1024], FileName[MAX_PATH], *ptrTopic, *ptrName;

		// разбор "параметров ком.строки"
		lstrcpyn(cmdbuf,((OpenCommandLineInfo*)Info->Data)->CommandLine,ARRAYSIZE(cmdbuf));

		if (cmdbuf[0])
		{
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
			if (hasTopic)
			{
				ptrTopic++;
				if (*ptrTopic == L'@')
					ptrTopic++;

				if (lstrlen(ptrTopic))
					FSF.Trim(ptrTopic);
				else
					ptrTopic={};
			}
			else
			{
				if (*ptrName == L'@')
				{
					ptrTopic=ptrName+1;
					FSF.Trim(ptrTopic);
					ptrName={};
				}
				else
					ptrTopic = {};
			}

			wchar_t *ptrCurDir={};

			// Здесь: ptrName - тмя файла/GUID, ptrTopic - имя темы

			// показ темы помощи по GUID`у
			GUID FindGuid;
			bool guidMode=StrToGuid(ptrName,&FindGuid);
			if (!guidMode && *ptrName == L'{')
			{
				ptrName++;
				wchar_t *ptrNameEnd=ptrName+lstrlen(ptrName)-1;

				if (*ptrNameEnd == L'}')
					*ptrNameEnd=0;
				guidMode=StrToGuid(ptrName,&FindGuid);
			}

			if (guidMode)
				PsInfo.ShowHelp((const wchar_t*)&FindGuid,ptrTopic,FHELP_GUID);

			// по GUID`у не найдено, пробуем имя файла
			if (!guidMode)
			{
				wchar_t TempFileName[MAX_PATH*2];
				wchar_t ExpFileName[MAX_PATH*2];
				lstrcpyn(TempFileName,ptrName,ARRAYSIZE(TempFileName));

				// Если имя файла без пути...
				if (FSF.PointToName(ptrName) == ptrName)
				{
					// ...смотрим в текущем каталоге
					size_t Size=FSF.GetCurrentDirectory(0,{});

					if (Size)
					{
						ptrCurDir=new WCHAR[Size+lstrlen(ptrName)+8];
						FSF.GetCurrentDirectory(Size,ptrCurDir);
						lstrcat(ptrCurDir,L"\\");
						lstrcat(ptrCurDir,ptrName);
						if (FileExists(ptrCurDir))
							ptrName=(wchar_t *)ptrCurDir;
					}

					// ...в текущем нет...
					if (FSF.PointToName(ptrName) == ptrName)
					{
						// ...смотрим в %FARHOME%
						ExpandEnvironmentStrings(L"%FARHOME%",ExpFileName,ARRAYSIZE(ExpFileName));
						FSF.AddEndSlash(ExpFileName);
						lstrcat(ExpFileName,ptrName);
						if (!FileExists(ExpFileName))
						{
							// ...в %FARHOME% нет, поищем по путям плагинов.
							if (FindPluginHelp(ptrName,ExpFileName))
								ptrName=ExpFileName;
						}
						else
							ptrName=ExpFileName;
					}
				}
				else
				{
					// ptrName указан с путём.
					ExpandEnvironmentStrings(ptrName,ExpFileName,ARRAYSIZE(ExpFileName));
					ptrName=ExpFileName;
				}

				GetFullPathName(ptrName,MAX_PATH,FileName,&ptrName);

				delete[] ptrCurDir;

				if (!ShowHelp(FileName,ptrTopic,true,(!ptrTopic || !*ptrTopic?false:true)))
				{
					// синтаксис hlf:topic_из_ФАР_хелпа ==> TempFileName
					PsInfo.ShowHelp(nullptr,TempFileName,FHELP_FARHELP);
				}
			}
		}
		else
		{
			// параметры не указаны, выводим подсказку по использованию плагина.
			PsInfo.ShowHelp(PsInfo.ModuleName,L"cmd",FHELP_SELFHELP);
		}
	}

	return nullptr;
}

void WINAPI GetPluginInfoW(PluginInfo *Info)
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
	BOOL Result=FALSE;

	if (Opt.ProcessEditorInput)
	{
		if (InputInfo->Rec.EventType==KEY_EVENT && InputInfo->Rec.Event.KeyEvent.bKeyDown && inputrecord_compare(InputInfo->Rec,Opt.RecKey))
		{
			EditorInfo ei = { sizeof(EditorInfo) };
			PsInfo.EditorControl(-1,ECTL_GETINFO,0,&ei);
			wchar_t *FileName=GetEditorFileName();

			if (IsHlf() || (Opt.CheckMaskFile && CheckExtension(FileName)))
			{
				if (ShowCurrentHelpTopic())
					Result=TRUE;
			}

			delete[] FileName;
		}
	}

	return Result;
}

wchar_t *GetEditorFileName()
{
	wchar_t *FileName=nullptr;
	size_t FileNameSize=PsInfo.EditorControl(-1,ECTL_GETFILENAME,0,{});

	if (FileNameSize)
	{
		FileName=new wchar_t[FileNameSize];
		PsInfo.EditorControl(-1,ECTL_GETFILENAME,FileNameSize,FileName);
	}

	return FileName;
}

bool ShowCurrentHelpTopic()
{
	bool Result=true;
	wchar_t *FileName=GetEditorFileName();
	EditorInfo ei = { sizeof(EditorInfo) };
	PsInfo.EditorControl(-1,ECTL_GETINFO,0,&ei);

	switch (Opt.Style)
	{
		case 1:

			if (!(ei.CurState&ECSTATE_SAVED))
				ShowHelpFromTempFile();
			else
			{
				const wchar_t *Topic=FindTopic(false);
				if (!Topic)
					Topic=FindTopic(true);
				if (Topic && *Topic)
					ShowHelp(FileName,Topic,false);
				else
					Result=false;
			}

			break;
		case 2:

			if (!(ei.CurState&ECSTATE_SAVED))
				PsInfo.EditorControl(-1,ECTL_SAVEFILE, 0, {});
			[[fallthrough]];
		default:
			ShowHelp(FileName,FindTopic(),false);
			break;
	}

	delete[] FileName;

	return Result;
}

static void ShowHelpFromTempFile()
{
	EditorSaveFile esf={sizeof(EditorSaveFile)};
	wchar_t fname[MAX_PATH];
	esf.FileName = fname;

	if (FSF.MkTemp(fname, ARRAYSIZE(fname)-4,L"HLF")>1)
	{
		lstrcat(fname,L".hlf");
		/*
		  esf.FileEOL={};
		  PsInfo.EditorControl(ECTL_SAVEFILE, &esf);
		  ShowHelp(esf.FileName, FindTopic());
		  DeleteFile(esf.FileName);
		*/
		EditorGetString egs={sizeof(EditorGetString)};
		EditorInfo ei={sizeof(EditorInfo)};
		DWORD Count;

		HANDLE Handle=CreateFile(esf.FileName, GENERIC_WRITE, FILE_SHARE_READ,
		                         {}, CREATE_NEW, 0, {});

		if (Handle != INVALID_HANDLE_VALUE)
		{
			PsInfo.EditorControl(-1,ECTL_GETINFO,0,&ei);
			#define SIGN_UNICODE    0xFEFF
			WORD sign=SIGN_UNICODE;
			WriteFile(Handle, &sign, 2, &Count, {});

			for (egs.StringNumber=0; egs.StringNumber<ei.TotalLines; egs.StringNumber++)
			{
				PsInfo.EditorControl(-1,ECTL_GETSTRING,0,&egs);
				WriteFile(Handle, egs.StringText, (DWORD)(egs.StringLength*sizeof(wchar_t)), &Count, {});
				WriteFile(Handle, L"\r\n", 2*sizeof(wchar_t), &Count, {});
			}

			CloseHandle(Handle);

			ShowHelp(esf.FileName, FindTopic());

			DeleteFile(esf.FileName);
		}
	}
}

void RestorePosition(EditorInfo *ei)
{
	EditorSetPosition esp={sizeof(EditorSetPosition)};
	esp.CurLine=ei->CurLine;
	esp.CurPos=ei->CurPos;
	esp.TopScreenLine=ei->TopScreenLine;
	esp.LeftPos=ei->LeftPos;
	esp.CurTabPos=-1;
	esp.Overtype=-1;
	PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
}

// это HLF-файл?
// первая строка hlf всегда начинается с ".Language="
BOOL IsHlf()
{
	BOOL ret=FALSE;
	EditorInfo ei={sizeof(EditorInfo)};
	PsInfo.EditorControl(-1,ECTL_GETINFO,0,&ei);
	bool CheckedHlf=true;

	if (Opt.CheckMaskFile)
	{
		wchar_t *FileName=GetEditorFileName();
		if (FileName)
		{
			if (!CheckExtension(FileName))
				CheckedHlf=false;
			delete[] FileName;
		}
	}

	if (CheckedHlf)
	{
		EditorSetPosition esp = { sizeof(EditorSetPosition) };
		EditorGetString egs = { sizeof(EditorGetString) };
		egs.StringNumber=-1;
		intptr_t total=(ei.TotalLines<3)?ei.TotalLines:3;

		if (total>2)
		{
			for (esp.CurLine=0; esp.CurLine<total; esp.CurLine++)
			{
				PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
				PsInfo.EditorControl(-1,ECTL_GETSTRING,0,&egs);

				if (!FSF.LStrnicmp(L".Language=",egs.StringText,10))
				{
					// доп.проверка
					if (FindTopic(true,false))
						ret=TRUE;
					break;
				}
			}

			RestorePosition(&ei);
		}
	}

	return ret;
}

// для "этой темы" ищем её имя (от позиции курсора вверх/вниз по файлу)
const wchar_t *FindTopic(bool ForwardDirect, bool RestorePos)
{
	const wchar_t*ret{};
	EditorInfo ei={sizeof(EditorInfo)};
	PsInfo.EditorControl(-1,ECTL_GETINFO,0,&ei);

	EditorSetPosition esp = { sizeof(EditorSetPosition) };
	EditorGetString egs = { sizeof(EditorGetString) };
	egs.StringNumber=-1;

	intptr_t Direct=ForwardDirect?1:-1;

	for (esp.CurLine=ei.CurLine; (ForwardDirect?esp.CurLine<ei.TotalLines:esp.CurLine>=0); esp.CurLine+=Direct)
	{
		PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
		PsInfo.EditorControl(-1,ECTL_GETSTRING,0,&egs);
		auto tmp=egs.StringText;

		// "Тема": начинается '@', дальше букво-цифры, не содержит '='
		if (lstrlen(tmp)>1 && *tmp==L'@' && *(tmp+1)!=L'-' && *(tmp+1)!=L'+' && !wcschr(tmp,L'='))
		{
			ret=tmp+1;
			break;
		}
	}

	if (RestorePos)
		RestorePosition(&ei);

	return ret;
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


// поиск hlf-файла Name в каталогах плагинов (первый найденный!)
// в DestPath возвращает полный путь к найденному hlf-файлу
static int WINAPI frsuserfunc(const PluginPanelItem *FData,const wchar_t *FullName,void *Param)
{
	lstrcpy((wchar_t*)Param,FullName);
	return FALSE;
}

bool FindPluginHelp(const wchar_t* Name,wchar_t* DestPath)
{
	bool Result=false;
	DestPath[0]=0;

	// 1. Получить количество плагинов в "этом сеансе" Far Manager
	int CountPlugin = (int)PsInfo.PluginsControl(INVALID_HANDLE_VALUE,PCTL_GETPLUGINS,0,{});

	if (CountPlugin > 0)
	{
			HANDLE *hPlugins=new HANDLE[CountPlugin];

			// 2. Получить хэндлы плагинов
			CountPlugin = PsInfo.PluginsControl(INVALID_HANDLE_VALUE, PCTL_GETPLUGINS, CountPlugin, hPlugins);

			// 3. Посмотреть на эти плагины
			for (int I=0; I < CountPlugin; ++I)
			{
				// 4. Для очередного плагина получить размер необходимой памяти по информационные структуры
				int SizeMemory=(int)PsInfo.PluginsControl(hPlugins[I],PCTL_GETPLUGININFORMATION,0,{});
				if (!SizeMemory)
					continue;

				const auto data = new BYTE[SizeMemory];
				const auto fgpi=reinterpret_cast<FarGetPluginInformation*>(data);
				fgpi->StructSize = sizeof(*fgpi);

				wchar_t FoundPath[MAX_PATH];
				// 5. Для очередного плагина получить информационные структуры
				if (PsInfo.PluginsControl(hPlugins[I],PCTL_GETPLUGININFORMATION,SizeMemory,fgpi) == SizeMemory)
				{
						// 6. Путь к плагину
						wchar_t *ModuleName=new wchar_t[lstrlen(fgpi->ModuleName)+1];
						lstrcpy(ModuleName,fgpi->ModuleName);
						const auto ptrModuleName=const_cast<wchar_t*>(FSF.PointToName(ModuleName));
						if (ptrModuleName)
							*ptrModuleName=0;

						// 7. Поиск hlf-файла в "этом каталоге"
						FoundPath[0]=0;
						FSF.FarRecursiveSearch(ModuleName,Name,(FRSUSERFUNC)frsuserfunc,FRS_RECUR|FRS_SCANSYMLINK,FoundPath);

						if (*FoundPath)
						{
							// НАЙДЕНО!
							lstrcpy(DestPath,FoundPath);
							Result=true;
						}

						delete[] ModuleName;
				}

				delete[] data;

				if (Result)
					break;
			}

			delete[] hPlugins;
	}

	return Result;
}


intptr_t WINAPI ConfigureW(const ConfigureInfo* CfgInfo)
{
	GetPluginConfig();

	PluginDialogBuilder Builder(PsInfo, MainGuid, DialogGuid, MConfig, L"Config");

	Builder.StartColumns();
	Builder.AddCheckbox(MProcessEditorInput, &Opt.ProcessEditorInput);
	Builder.AddCheckbox(MCheckMaskFile, &Opt.CheckMaskFile);
	Builder.ColumnBreak();
	FarDialogItem *ItemAssignKeyName=Builder.AddEditField(Opt.AssignKeyName, ARRAYSIZE(Opt.AssignKeyName), 20);
	Builder.AddEditField(Opt.MaskFile, ARRAYSIZE(Opt.MaskFile), 20);
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

		PluginSettings settings(MainGuid, PsInfo.SettingsControl);
		settings.Set(0,L"ProcessEditorInput",Opt.ProcessEditorInput);
		settings.Set(0,L"Style",Opt.Style);
		settings.Set(0,L"EditorKey",Opt.AssignKeyName);
		settings.Set(0,L"CheckMaskFile",Opt.CheckMaskFile);
		settings.Set(0,L"MaskFile",Opt.MaskFile);
		return TRUE;
	}
	return FALSE;
}

void GetPluginConfig()
{
	PluginSettings settings(MainGuid, PsInfo.SettingsControl);

	settings.Get(0,L"EditorKey",Opt.AssignKeyName,ARRAYSIZE(Opt.AssignKeyName),L"F1");

	if (!FSF.FarNameToInputRecord(Opt.AssignKeyName,&Opt.RecKey))
	{
		lstrcpyn(Opt.AssignKeyName,L"F1",ARRAYSIZE(Opt.AssignKeyName));
		Opt.RecKey=_DefKey;
	}

	Opt.ProcessEditorInput=settings.Get(0,L"ProcessEditorInput",1);
	Opt.Style=settings.Get(0,L"Style",0);
	Opt.CheckMaskFile=settings.Get(0,L"CheckMaskFile",1);
	settings.Get(0,L"MaskFile",Opt.MaskFile,ARRAYSIZE(Opt.MaskFile),L"*.hlf");
}
