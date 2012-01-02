/*
TMPPANEL.CPP

Temporary panel main plugin code

*/

#include "TmpPanel.hpp"

#include <initguid.h>
#include "guid.hpp"

//wchar_t *PluginRootKey;
unsigned int CurrentCommonPanel;
struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;
wchar_t *AnalyseFileName=nullptr;

PluginPanels CommonPanels[COMMONPANELSNUMBER];

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

static void ProcessList(HANDLE hPlugin, wchar_t *Name, int Mode);
static void ShowMenuFromList(wchar_t *Name);
static HANDLE OpenPanelFromOutput(wchar_t *argv);
static void ShowMenuFromList(wchar_t *Name);

static HANDLE OpenPanelFromOutput(wchar_t *argv)
{
	wchar_t *tempDir=ParseParam(argv);
	BOOL allOK=FALSE;
	StrBuf tempfilename(NT_MAX_PATH); //BUGBUG
	StrBuf cmdparams(NT_MAX_PATH); //BUGBUG
	StrBuf fullcmd;
	FSF.MkTemp(tempfilename,tempfilename.Size(),L"FARTMP");
	lstrcpy(cmdparams,L"%COMSPEC% /c ");
	lstrcat(cmdparams,argv);
	ExpandEnvStrs(cmdparams,fullcmd);
	SECURITY_ATTRIBUTES sa;
	memset(&sa, 0, sizeof(sa));
	sa.nLength=sizeof(sa);
	sa.bInheritHandle=TRUE;
	HANDLE FileHandle;
	FileHandle=CreateFile(tempfilename,GENERIC_WRITE,FILE_SHARE_READ,
	                      &sa,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,NULL);

	if (FileHandle!=INVALID_HANDLE_VALUE)
	{
		STARTUPINFO si;
		memset(&si,0,sizeof(si));
		si.cb=sizeof(si);
		si.dwFlags=STARTF_USESTDHANDLES;
		si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
		si.hStdOutput=FileHandle;
		si.hStdError=FileHandle;
		PROCESS_INFORMATION pi;
		memset(&pi,0,sizeof(pi));
		StrBuf workDir(1); //make empty string just in case

		if (tempDir)
		{
			workDir.Reset(NT_MAX_PATH);  //BUGBUG
			ExpandEnvStrs(tempDir,workDir);
		}
		else
		{
			size_t Size=FSF.GetCurrentDirectory(0,NULL);

			if (Size)
			{
				workDir.Reset((int)Size);
				FSF.GetCurrentDirectory(Size,workDir);
			}
		}

		wchar_t consoleTitle[255];
		DWORD tlen = GetConsoleTitle(consoleTitle, ARRAYSIZE(consoleTitle));
		SetConsoleTitle(argv);
		BOOL Created=CreateProcess(NULL,fullcmd,NULL,NULL,TRUE,0,NULL,workDir,&si,&pi);

		if (Created)
		{
			WaitForSingleObject(pi.hProcess,INFINITE);
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			allOK=TRUE;
		}

		CloseHandle(FileHandle);

		if (tlen)
			SetConsoleTitle(consoleTitle);
	}

	HANDLE hPlugin = INVALID_HANDLE_VALUE;

	if (allOK)
	{
		if (Opt.MenuForFilelist)
		{
			ShowMenuFromList(tempfilename);
		}
		else
		{
			hPlugin=new TmpPanel();

			if (hPlugin==NULL)
				return INVALID_HANDLE_VALUE;

			ProcessList(hPlugin, tempfilename, Opt.Mode);
		}
	}

	DeleteFile(tempfilename);
	return hPlugin;
}

void ReadFileLines(HANDLE hFileMapping, DWORD FileSizeLow, wchar_t **argv, wchar_t *args,
                   UINT *numargs, UINT *numchars)
{
	*numchars = 0;
	*numargs = 0;
	char *FileData=(char *)MapViewOfFile(hFileMapping,FILE_MAP_READ,0,0,FileSizeLow);

	if (!FileData)
		return;

	StrBuf TMP(NT_MAX_PATH); //BUGBUG
	DWORD Len,Pos=0,Size=FileSizeLow;
	bool  ucs2 = false;

	if (Size >= 2) switch (*(LPWORD)FileData)
		{
			case(BOM_UTF8 & 0xFFFF):

				if (Size >= 3 && ((LPBYTE)FileData)[2] == (BYTE)(BOM_UTF8>>16))
				{
				case BOM_UCS2_BE:
					goto done;
				}

			default:
				break;
			case BOM_UCS2:
				ucs2 = true;
				Pos += 2;
				break;
		}

	while (Pos<Size)
	{
		if (!ucs2)
		{
			char c;

			while (Pos<Size && ((c = FileData[Pos]) == '\r' || c == '\n'))
				Pos++;

			DWORD Off = Pos;

			while (Pos<Size && (c = FileData[Pos]) != '\r' && c != '\n')
				Pos++;

			Len = Pos - Off;
			Len = MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED|MB_ERR_INVALID_CHARS,
			                          &FileData[Off], Len, TMP, TMP.Size()-1);
		}
		else
		{
			wchar_t c;
			--Size;

			while (Pos<Size && ((c = *(wchar_t*)&FileData[Pos]) == L'\r' || c == L'\n'))
				Pos += sizeof(wchar_t);

			DWORD Off = Pos;

			while (Pos<Size && (c = *(wchar_t*)&FileData[Pos]) != L'\r' && c != L'\n')
				Pos += sizeof(wchar_t);

			if (Pos < Size)
				++Size;

			Len = (Pos-Off)/sizeof(wchar_t);

			if (Len >= (DWORD)TMP.Size())
				Len = TMP.Size()-1;

			memcpy(TMP.Ptr(), &FileData[Off], Len*sizeof(wchar_t));
		}

		if (!Len)
			continue;

		TMP.Ptr()[Len]=0;
		FSF.Trim(TMP);
		FSF.Unquote(TMP);
		Len = lstrlen(TMP);

		if (!Len)
			continue;

		if (argv)
			*argv++ = args;

		if (args)
		{
			lstrcpy(args,TMP);
			args+=Len+1;
		}

		(*numchars)+=Len+1;
		++*numargs;
	}

done:
	UnmapViewOfFile((LPVOID)FileData);
}

static void ReadFileList(wchar_t *filename, int *argc, wchar_t ***argv)
{
	*argc = 0;
	*argv = NULL;
	StrBuf FullPath;
	GetFullPath(filename, FullPath);
	StrBuf NtPath;
	FormNtPath(FullPath, NtPath);
	HANDLE hFile=CreateFile(NtPath,GENERIC_READ,FILE_SHARE_READ,NULL,
	                        OPEN_EXISTING,0,NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD FileSizeLow=GetFileSize(hFile,NULL);
		HANDLE hFileMapping=CreateFileMapping(hFile,NULL,PAGE_READONLY,0,0,NULL);
		CloseHandle(hFile);

		if (hFileMapping != NULL)
		{
			UINT i;
			ReadFileLines(hFileMapping, FileSizeLow, NULL, NULL, (UINT*)argc, &i);
			*argv = (wchar_t**)malloc(*argc*sizeof(wchar_t*) + i*sizeof(wchar_t));
			ReadFileLines(hFileMapping, FileSizeLow, *argv, (wchar_t*)&(*argv)[*argc],
			              (UINT*)argc, &i);
			CloseHandle(hFileMapping);
		}
	}
}

static void ProcessList(HANDLE hPlugin, wchar_t *Name, int Mode)
{
	if (Mode)
	{
		FreePanelItems(CommonPanels[CurrentCommonPanel].Items,
		               CommonPanels[CurrentCommonPanel].ItemsNumber);
		CommonPanels[CurrentCommonPanel].Items=(PluginPanelItem*)calloc(1,sizeof(PluginPanelItem));
		CommonPanels[CurrentCommonPanel].ItemsNumber=0;
	}

	TmpPanel *Panel=(TmpPanel*)hPlugin;
	int argc;
	wchar_t **argv;
	ReadFileList(Name, &argc, &argv);
	HANDLE hScreen = Panel->BeginPutFiles();

	for (UINT i=0; (int)i<argc; i++)
		Panel->PutOneFile(argv[i]);

	Panel->CommitPutFiles(hScreen, TRUE);

	if (argv)
		free(argv);
}


static void ShowMenuFromList(wchar_t *Name)
{
	int argc;
	wchar_t **argv;
	ReadFileList(Name, &argc, &argv);
	FarMenuItem *fmi=(FarMenuItem*)malloc(argc*sizeof(FarMenuItem));

	if (fmi)
	{
		StrBuf TMP(NT_MAX_PATH); //BUGBUG

		for (int i=0; i<argc; ++i)
		{
			wchar_t *param,*p=TMP;
			ExpandEnvStrs(argv[i],TMP);
			param=ParseParam(p);
			FSF.TruncStr(param?param:p,67);
			fmi[i].Text = wcsdup(param?param:p);
			fmi[i].Flags = !lstrcmp(param,L"-")?MIF_SEPARATOR:0;
		}

//    fmi[0].Selected=TRUE;
		wchar_t Title[128]; //BUGBUG
		FSF.ProcessName(FSF.PointToName(Name),lstrcpy(Title,L"*."),ARRAYSIZE(Title),PN_GENERATENAME);
		FSF.TruncPathStr(Title,64);
		FarKey BreakKeys[]={{VK_RETURN,SHIFT_PRESSED}, {0,0}};
		int BreakCode;
		int ExitCode=Info.Menu(&MainGuid, nullptr, -1, -1, 0,
		                       FMENU_WRAPMODE, Title, NULL, L"Contents",
		                       &BreakKeys[0], &BreakCode, fmi, argc);

		for (int i=0; i<argc; ++i)
			if (fmi[i].Text)
				free((void*)fmi[i].Text);

		free(fmi);

		if ((unsigned)ExitCode<(unsigned)argc)
		{
			ExpandEnvStrs(argv[ExitCode],TMP);
			wchar_t *p=TMP;
			ParseParam(p);
			PluginPanelItem FindData;
			int bShellExecute=BreakCode!=-1;

			if (!bShellExecute)
			{
				if (TmpPanel::GetFileInfoAndValidate(p,&FindData,FALSE))
				{
					if (FindData.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						FarPanelDirectory dirInfo = {sizeof(dirInfo), p, nullptr, {}, nullptr};
						Info.PanelControl(PANEL_ACTIVE, FCTL_SETPANELDIRECTORY, 0, &dirInfo);
					}
					else
					{
						bShellExecute=TRUE;
					}
				}
				else
				{
					Info.PanelControl(PANEL_ACTIVE,FCTL_SETCMDLINE,0,p);
				}
			}

			if (bShellExecute)
				ShellExecute(NULL,L"open",p,NULL,NULL,SW_SHOW);
		}
	}

	if (argv)
		free(argv);
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

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
	::Info=*Info;
	::FSF=*Info->FSF;
	::Info.FSF=&::FSF;
	GetOptions();
	StartupOptFullScreenPanel=Opt.FullScreenPanel;
	StartupOptCommonPanel=Opt.CommonPanel;
	CurrentCommonPanel=0;
	memset(CommonPanels, 0, sizeof(CommonPanels));
	CommonPanels[0].Items=(PluginPanelItem*)calloc(1,sizeof(PluginPanelItem));
	Opt.LastSearchResultsPanel = 0;
}

HANDLE WINAPI OpenW(const struct OpenInfo *Info)
{
	HANDLE hPlugin=INVALID_HANDLE_VALUE;
	GetOptions();
	StartupOpenFrom=Info->OpenFrom;

	if (Info->OpenFrom==OPEN_COMMANDLINE)
	{
		wchar_t *argv=(wchar_t*)Info->Data;
		#define OPT_COUNT 5
		static const wchar_t ParamsStr[OPT_COUNT][8]=
		{
			L"safe",L"any",L"replace",L"menu",L"full"
		};
		const int *ParamsOpt[OPT_COUNT]=
		{
			&Opt.SafeModePanel,&Opt.AnyInPanel,&Opt.Mode,
			&Opt.MenuForFilelist,&Opt.FullScreenPanel
		};

		while (argv && *argv==L' ')
			argv++;

		while (lstrlen(argv)>1 && (*argv==L'+' || *argv==L'-'))
		{
			int k=0;

			while (argv && *argv!=L' ' && *argv!=L'<')
			{
				k++;
				argv++;
			}

			StrBuf tmpTMP(k+1);
			wchar_t *TMP=tmpTMP;
			lstrcpyn(TMP,argv-k,k+1);

			for (int i=0; i<OPT_COUNT; i++)
				if (lstrcmpi(TMP+1,ParamsStr[i])==0)
					*(int*)ParamsOpt[i] = *TMP==L'+';

			if (*(TMP+1)>=L'0' && *(TMP+1)<=L'9')
				CurrentCommonPanel=*(TMP+1)-L'0';

			while (argv && *argv==L' ')
				argv++;
		}

		FSF.Trim(argv);

		if (lstrlen(argv))
		{
			if (*argv==L'<')
			{
				argv++;
				hPlugin = OpenPanelFromOutput(argv);

				if (Opt.MenuForFilelist)
					return INVALID_HANDLE_VALUE;
			}
			else
			{
				FSF.Unquote(argv);
				StrBuf TmpIn;
				ExpandEnvStrs(argv,TmpIn);
				StrBuf TmpOut;

				if (FindListFile(TmpIn,TmpOut))
				{
					if (Opt.MenuForFilelist)
					{
						ShowMenuFromList(TmpOut);
						return INVALID_HANDLE_VALUE;
					}
					else
					{
						hPlugin=new TmpPanel();

						if (hPlugin==NULL)
							return INVALID_HANDLE_VALUE;

						ProcessList(hPlugin, TmpOut, Opt.Mode);
					}
				}
				else
				{
					return INVALID_HANDLE_VALUE;
				}
			}
		}
	}
	else if (Info->OpenFrom == OPEN_ANALYSE)
	{
		if (AnalyseFileName && *AnalyseFileName)
		{
			StrBuf pName(NT_MAX_PATH); //BUGBUG
			lstrcpy(pName, AnalyseFileName);

			if (!FSF.ProcessName(Opt.Mask,pName, pName.Size(),PN_CMPNAMELIST))
				return INVALID_HANDLE_VALUE;

			if (!Opt.MenuForFilelist)
			{
				HANDLE hPlugin=new TmpPanel();

				if (hPlugin == NULL)
					return INVALID_HANDLE_VALUE;

				ProcessList(hPlugin, pName, Opt.Mode);
				return hPlugin;
			}
			else
			{
				ShowMenuFromList(pName);
				return INVALID_HANDLE_VALUE;
			}
		}
		return INVALID_HANDLE_VALUE;
	}

	if (hPlugin==INVALID_HANDLE_VALUE)
	{
		hPlugin=new TmpPanel();

		if (hPlugin==NULL)
			return INVALID_HANDLE_VALUE;
	}

	return hPlugin;
}

int WINAPI AnalyseW(const struct AnalyseInfo *Info)
{
	if (AnalyseFileName)
		free(AnalyseFileName);
	AnalyseFileName=nullptr;

	if (Info->FileName == nullptr || !Info->BufferSize)
		return FALSE;

	if (!FSF.ProcessName(Opt.Mask, (wchar_t*)Info->FileName, lstrlen(Info->FileName),PN_CMPNAMELIST))
		return FALSE;

	AnalyseFileName=wcsdup(Info->FileName);

	return TRUE;
}

void WINAPI ClosePanelW(const struct ClosePanelInfo *Info)
{
	delete(TmpPanel *)Info->hPanel;
}

void WINAPI ExitFARW(const struct ExitInfo *Info)
{
	if (AnalyseFileName)
		free(AnalyseFileName);

	for (int i=0; i<COMMONPANELSNUMBER; ++i)
		FreePanelItems(CommonPanels[i].Items, CommonPanels[i].ItemsNumber);
}


int WINAPI GetFindDataW(struct GetFindDataInfo *Info)
{
	TmpPanel *Panel=(TmpPanel *)Info->hPanel;
	return Panel->GetFindData(&Info->PanelItem,&Info->ItemsNumber,Info->OpMode);
}


void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=PF_PRELOAD;

	if (Opt.AddToDisksMenu)
	{
		static const wchar_t *DiskMenuStrings[1];
		DiskMenuStrings[0]=GetMsg(MDiskMenuString);
    	Info->DiskMenu.Guids=&MenuGuid;
    	Info->DiskMenu.Strings=DiskMenuStrings;
    	Info->DiskMenu.Count=ARRAYSIZE(DiskMenuStrings);
	}

	if (Opt.AddToPluginsMenu)
	{
		static const wchar_t *PluginMenuStrings[1];
		PluginMenuStrings[0]=GetMsg(MTempPanel);
		Info->PluginMenu.Guids=&MenuGuid;
		Info->PluginMenu.Strings=PluginMenuStrings;
		Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
	}

	static const wchar_t *PluginCfgStrings[1];
	PluginCfgStrings[0]=GetMsg(MTempPanel);
	Info->PluginConfig.Guids=&MenuGuid;
	Info->PluginConfig.Strings=PluginCfgStrings;
	Info->PluginConfig.Count=ARRAYSIZE(PluginCfgStrings);
	Info->CommandPrefix=Opt.Prefix;
}

void WINAPI GetOpenPanelInfoW(struct OpenPanelInfo *Info)
{
	TmpPanel *Panel=(TmpPanel *)Info->hPanel;
	Panel->GetOpenPanelInfo(Info);
}

int WINAPI SetDirectoryW(const struct SetDirectoryInfo *Info)
{
	TmpPanel *Panel=(TmpPanel *)Info->hPanel;
	return(Panel->SetDirectory(Info->Dir,Info->OpMode));
}

int WINAPI PutFilesW(const struct PutFilesInfo *Info)
{
	TmpPanel *Panel=(TmpPanel *)Info->hPanel;
	return Panel->PutFiles(Info->PanelItem,Info->ItemsNumber,Info->Move,Info->SrcPath,Info->OpMode);
}

int WINAPI SetFindListW(const struct SetFindListInfo *Info)
{
	TmpPanel *Panel=(TmpPanel *)Info->hPanel;
	return(Panel->SetFindList(Info->PanelItem,Info->ItemsNumber));
}


int WINAPI ProcessPanelEventW(const struct ProcessPanelEventInfo *Info)
{
	return ((TmpPanel *)Info->hPanel)->ProcessEvent(Info->Event,Info->Param);
}

int WINAPI ProcessPanelInputW(const struct ProcessPanelInputInfo *Info)
{
	return ((TmpPanel *)Info->hPanel)->ProcessKey(&Info->Rec);
}

int WINAPI ConfigureW(const struct ConfigureInfo *Info)
{
	return Config();
}
