#include <plugin.hpp>

#include "FARCmds.hpp"
#include "Lang.hpp"
#include <initguid.h>
#include "guid.hpp"

#ifndef LIGHTGRAY
#define LIGHTGRAY 7
#endif

static wchar_t* getCurrDir(bool winApi)
{
	wchar_t *CurDir=nullptr;
	size_t Size=winApi?GetCurrentDirectory(0,nullptr):FSF.GetCurrentDirectory(0,nullptr);
	if (Size)
	{
		CurDir=new wchar_t[Size+1];
		if (winApi)
			GetCurrentDirectory(static_cast<DWORD>(Size),CurDir);
		else
			FSF.GetCurrentDirectory(Size,CurDir);
	}
	return CurDir;
}

static void closeHandle(HANDLE& handle)
{
	if (handle != INVALID_HANDLE_VALUE)
		CloseHandle(handle);

	handle = INVALID_HANDLE_VALUE;
}

static void killTemp(wchar_t *TempFileName)
{
	if (FileExists(TempFileName))
	{
		DeleteFile(TempFileName);
		wchar_t *PtrName=(wchar_t*)FSF.PointToName(TempFileName);
		if (PtrName > TempFileName)
			*PtrName = 0;
		RemoveDirectory(TempFileName);
	}
}

static HANDLE createFile(wchar_t *Name,int catchOut)
{
	HANDLE hFile=INVALID_HANDLE_VALUE;

	if (catchOut)
	{
		SECURITY_ATTRIBUTES sa;
		memset(&sa, 0, sizeof(sa));
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;
		hFile = CreateFile(Name,GENERIC_WRITE,FILE_SHARE_READ,&sa,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	}
	else
	{
		killTemp(Name);
		Name[0] = 0;
	}

	return hFile;
}

static void createFileStream(const wchar_t *Name,HANDLE hFile,TShowOutputStreamData *sd, HANDLE StdOutput)
{
	SECURITY_ATTRIBUTES sa;
	memset(&sa, 0, sizeof(sa));
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	sd->hRead = CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	sd->hWrite   = hFile;
	sd->hConsole = StdOutput;
}

#if 0
static void clearScreen(HANDLE StdOutput,HANDLE StdInput,CONSOLE_SCREEN_BUFFER_INFO& csbi)
{
	wchar_t* Blank=new wchar_t[csbi.dwSize.X+1];
	if (Blank)
	{
		wmemset(Blank, L' ', csbi.dwSize.X);
		Blank[csbi.dwSize.X]=0;

		FarColor Color={};
		Color.Flags = FCF_FG_4BIT|FCF_BG_4BIT;
		Color.ForegroundColor = LIGHTGRAY;
		Color.BackgroundColor = 0;
		for (int Y = 0 ; Y < csbi.dwSize.Y ; Y++)
			Info.Text(0, Y, &Color, Blank);

		delete[] Blank;
	}

	Info.Text(0, 0, 0, NULL);
}

static void restoreScreen(HANDLE StdOutput,HANDLE StdInput,DWORD ConsoleMode,CONSOLE_SCREEN_BUFFER_INFO& csbi)
{
	SetConsoleMode(StdInput, ConsoleMode);
	SMALL_RECT src;
	COORD dest;
	CHAR_INFO fill;
	src.Left = 0;
	src.Top = 2;
	src.Right = csbi.dwSize.X;
	src.Bottom = csbi.dwSize.Y;
	dest.X = dest.Y = 0;
	fill.Char.UnicodeChar = L' ';
	fill.Attributes = LIGHTGRAY;
	ScrollConsoleScreenBuffer(StdOutput, &src, NULL, dest, &fill);
}
#endif

inline bool isDevice(const wchar_t* FileName, const wchar_t* dev_begin)
{
	const int len=lstrlen(dev_begin);

	if (FSF.LStrnicmp(FileName, dev_begin, len))
		return false;

	FileName+=len;

	if (!*FileName)
		return false;

	while (*FileName>=L'0' && *FileName<=L'9')
		FileName++;

	return !*FileName;
}

static bool validForView(const wchar_t *FileName, bool viewEmpty, bool editNew)
{
	if (!wcsncmp(FileName, L"\\\\.\\", 4) &&  // специальная обработка имен
			FSF.LIsAlpha(FileName[4]) &&          // вида: \\.\буква:
			FileName[5]==L':' && FileName[6]==0)
		return true;

	if (isDevice(FileName, L"\\\\.\\PhysicalDrive"))
		return true;

	if (isDevice(FileName, L"\\\\.\\cdrom"))
		return true;


	bool Ret=false;
	wchar_t *ptrFileName=new wchar_t[lstrlen(FileName)+1];
	wchar_t *ptrCurDir=nullptr;

	if (ptrFileName)
	{
		lstrcpy(ptrFileName,FileName);

		if (*ptrFileName && FSF.PointToName(ptrFileName) == ptrFileName)
		{
			int dirSize=(int)Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,0,0);

			if (dirSize)
			{
			    FarPanelDirectory* dirInfo=(FarPanelDirectory*)new char[dirSize];
			    if (dirInfo)
			    {
				    dirInfo->StructSize = sizeof(FarPanelDirectory);
					Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,dirSize,dirInfo);

					int Size=lstrlen(dirInfo->Name)+1;
					ptrCurDir=new wchar_t[Size+lstrlen(FileName)+8];
					if (ptrCurDir)
					{
						lstrcpy(ptrCurDir,dirInfo->Name);
						lstrcat(ptrCurDir,L"\\");
						lstrcat(ptrCurDir,ptrFileName);

						delete[] ptrFileName;
						ptrFileName=ptrCurDir;
					}

					delete[](char*)dirInfo;
				}
			}
		}
	}

	if (ptrFileName && *ptrFileName && FileExists(ptrFileName))
	{
		if (viewEmpty)
			Ret = true;
		else
		{
			HANDLE Handle = CreateFile(ptrFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);

			if (Handle != INVALID_HANDLE_VALUE)
			{
				DWORD size = GetFileSize(Handle, NULL);
				CloseHandle(Handle);

				Ret = size && (size != 0xFFFFFFFF);
			}
		}
	}
	else if (editNew)
		Ret=true;

	if (ptrCurDir)
		delete[] ptrCurDir;

	if (ptrFileName)
		delete[] ptrFileName;

	return Ret;
}

// нитка параллельного вывода на экран для ":<+"
static DWORD showPartOfOutput(TShowOutputStreamData *sd, bool mainProc)
{
	DWORD Res = 0;

	if (sd && sd->hConsole != INVALID_HANDLE_VALUE)
	{
		#define READBUFSIZE 4096
		wchar_t *ReadBuf=new wchar_t[READBUFSIZE+1];
		if (ReadBuf)
		{
			DWORD BytesRead = 0;
			if (ReadFile(sd->hRead, ReadBuf, READBUFSIZE, &BytesRead, NULL))
			{
				if (BytesRead)
				{
					DWORD dummy;
					size_t shift=0;
					ReadBuf[BytesRead] = 0;
					bool unicode=false;
					ReadBuf=ConvertBuffer(ReadBuf,BytesRead/sizeof(wchar_t),TRUE,shift,&unicode);

					WriteConsole(sd->hConsole, ReadBuf+shift, unicode?BytesRead/sizeof(wchar_t):lstrlen(ReadBuf+shift), &dummy, NULL);

					Res = BytesRead;
				}
			}

			delete[] ReadBuf;
		}
	}

	return Res;
}

DWORD WINAPI ThreadWhatUpdateScreen(LPVOID par)
{
	if (par)
	{
		TThreadData *td = (TThreadData*)par;

		if (td->type == enThreadShowOutput)
		{
			for (; ;)
			{
				if (WAIT_TIMEOUT != WaitForSingleObject(td->process, THREADSLEEP))
					break;

				for (int i = 0 ; i < enStreamMAX ; i++)
				{
					TShowOutputStreamData *sd = &(td->stream[i]);

					if (sd->hRead != INVALID_HANDLE_VALUE)
						showPartOfOutput(sd,true);
				}
			}
		}
		else
		{
			for (; ;)
			{
				for (int j = 0 ; j < THREADREDRAW ; j++)
				{
					if (WAIT_TIMEOUT != WaitForSingleObject(td->process, THREADSLEEP))
						break;
				}

				if (WAIT_TIMEOUT != WaitForSingleObject(td->process, 0))
					break;

				wchar_t buff[80];
				DWORD sCheck[enStreamMAX];

				for (int i = 0 ; i < enStreamMAX ; i++)
				{
					HANDLE hCheck = td->stream[i].hWrite;

					if (hCheck != INVALID_HANDLE_VALUE)
					{
						sCheck[i] = GetFileSize(hCheck, NULL);

						if (sCheck[i] == 0xFFFFFFFF)
							sCheck[i] = 0;
					}
					else
						sCheck[i] = 0;
				}

				if (sCheck[enStreamOut])
					if (sCheck[enStreamErr])
						FSF.sprintf(buff, L"%lu/%lu", sCheck[enStreamOut], sCheck[enStreamErr]);
					else
						FSF.sprintf(buff, L"%lu", sCheck[enStreamOut]);
				else if (sCheck[enStreamErr])
					FSF.sprintf(buff, L"%lu", sCheck[enStreamErr]);
				else
					*buff = 0;

				const wchar_t *MsgItems[] = { td->title, td->cmd, buff };
				Info.Message(&MainGuid, nullptr, 0, NULL, MsgItems, ARRAYSIZE(MsgItems), 0);
			}
		}
	}

	return 0;
}


// два файла в одном каталоге
static bool MakeTempNames(wchar_t** FileName1, wchar_t** FileName2)
{
	static const wchar_t tmpPrefix[] = L"FCP";

	// create temp-dir
	size_t sizeTempName=FSF.MkTemp(nullptr,0,tmpPrefix);
	wchar_t *NameDir=new wchar_t[sizeTempName+1];
	if (NameDir)
	{
		FSF.MkTemp(NameDir,sizeTempName,tmpPrefix);

		DeleteFile(NameDir);

		if (CreateDirectory(NameDir, NULL))
		{
			bool ok = false;

			wchar_t *tempFileName1=nullptr;
			wchar_t *tempFileName2=nullptr;

			wchar_t fullcmd[MAX_PATH*2]; // ????

			// create temp-file1
			if (GetTempFileName(NameDir, tmpPrefix, 0, fullcmd))
			{
				tempFileName1=new wchar_t[lstrlen(fullcmd)+1];
				if (tempFileName1)
				{
					lstrcpy(tempFileName1, fullcmd);

					// create temp-file2
					if (GetTempFileName(NameDir, tmpPrefix, 0, fullcmd))
					{
						tempFileName2=new wchar_t[lstrlen(fullcmd)+1];
						if (tempFileName2)
						{
							lstrcpy(tempFileName2, fullcmd);
							ok=true;
						}
					}
				}
			}

			if (ok)
			{
				if (FileName1)
					*FileName1=tempFileName1;

				if (FileName2)
					*FileName2=tempFileName2;

				delete[] NameDir;
				return true;
			}

			if (tempFileName1)
			{
				DeleteFile(tempFileName1);
				delete[] tempFileName1;
				tempFileName1=nullptr;
			}

			if (tempFileName2)
			{
				DeleteFile(tempFileName2);
				delete[] tempFileName2;
				tempFileName2=nullptr;
			}

			RemoveDirectory(NameDir);
		}

		delete[] NameDir;
	}

	return false;

}

static const wchar_t* MakeExecuteString(const wchar_t *Cmd)
{
	bool quote_cmd=false, quoted_par=false;
	const wchar_t COMSPEC[]=L"%COMSPEC% /c ";

	// 1. разбор строки на команду и параметры
	wchar_t *NewCmdStr=nullptr, *NewCmdPar=nullptr;
	PartCmdLine(Cmd,&NewCmdStr,&NewCmdPar);

	if (NewCmdStr)
	{
		// 1.1 если команда не содержит полный путь...
		if (!wcschr(NewCmdStr,L'\\'))
		{
			wchar_t *fpath=__proc_WhereIs(false,NewCmdStr,false);
			if (fpath)
			{
				delete[] NewCmdStr;
				NewCmdStr=fpath;
			}
		}
	}

	// 2. обкавычим, если путь к команде получился с пробелами
	if (NewCmdStr && wcschr(NewCmdStr,L' '))
		quote_cmd=true;

	// 3. проверим параметр
	if (NewCmdPar && wcschr(NewCmdPar,L'"'))
		quoted_par=true;


	// 4. собираем всё обратно
	wchar_t* temp=new wchar_t[
		lstrlen(COMSPEC)+
		(NewCmdStr?lstrlen(NewCmdStr)+(quote_cmd?2:0):0)+1+
		(NewCmdPar?lstrlen(NewCmdPar)+(quote_cmd?2:0):0)+1+
		8
	];

	if (temp)
	{
		lstrcpy(temp,COMSPEC);

		if(quoted_par)
			lstrcat(temp,L"\"");

		if (NewCmdStr)
		{
			if(quote_cmd)
				lstrcat(temp,L"\"");
			lstrcat(temp,NewCmdStr);
			if(quote_cmd)
				lstrcat(temp,L"\"");
		}
       	if (NewCmdPar)
       	{
			lstrcat(temp,L" ");
			lstrcat(temp,FSF.Trim(NewCmdPar));
		}

		if(quoted_par)
			lstrcat(temp,L"\"");

		wchar_t *fullcmd=ExpandEnv(temp,nullptr);

		delete[] temp;

		return fullcmd;
	}

	return nullptr;
}

/*
  Возвращает указатель на выделенный кусок, которому после использования сделать free

  fn - имя файла, откуда читать
  maxSize - сколько максимум прочитать из файла
  outputtofile - это было перенаправление или...
  shift - начало "правильных данных" в прочитанном буфере, с учетом кодировок...
*/
static wchar_t *loadFile(const wchar_t *fn, DWORD maxSize, BOOL outputtofile, size_t& shift, bool& foundFile)
{
	foundFile = false;
	shift=0;

	wchar_t *Ptr = NULL;

	wchar_t *Temp=ExpandEnv(fn,nullptr);
	if (!Temp)
		return nullptr;

	FSF.Unquote(Temp);

	size_t SizeNativePath=FSF.ConvertPath(CPM_NATIVE, Temp, nullptr, 0);
	wchar_t *FileName=new wchar_t[SizeNativePath+1];
	if (!FileName)
	{
		delete[] Temp;
		return nullptr;
	}
	FSF.ConvertPath(CPM_NATIVE, Temp, FileName, SizeNativePath);
	delete[] Temp;

	DWORD read=0;

	if (*FileName && FileExists(FileName))
	{
		foundFile = true;
		HANDLE Handle = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

		if (Handle != INVALID_HANDLE_VALUE)
		{
			DWORD sizeFile=GetFileSize(Handle, NULL);
			DWORD size = (sizeFile+(sizeof(wchar_t)/2)) / sizeof(wchar_t);

			if (size >= maxSize)
				size = maxSize-1;

			size *= sizeof(wchar_t);

			if (size)
			{
				wchar_t *buff = new wchar_t[size+1];

				if (buff)
				{
					if (ReadFile(Handle, buff, size, &read, NULL) && (read >= sizeof(wchar_t) || (read == 1 && sizeFile == 1)))
					{
						if (read&1)
						{
							buff[read/sizeof(wchar_t)]=buff[read/sizeof(wchar_t)]&0xff;
							read++;
						}

						buff[read/sizeof(wchar_t)] = 0;
						Ptr = buff;
					}
					else
						delete[] buff;
				}
			}

			CloseHandle(Handle);
		}

	}

	delete[] FileName;

	return ConvertBuffer(Ptr,read,outputtofile,shift,nullptr);
}


// [stream][mode][|path|]command
// Default:
// 	ShowCmdOutput <- Opt.ShowCmdOutput
//	stream        <- Opt.CatchMode
static void GetStreamAndMode(wchar_t*& pCmd, int& ShowCmdOutput, int& CatchMode)
{
	// edit:<O
	// edit:<SO
	// edit:<SMO
	bool foundStream=true;

	// edit:<SM object
	//       ^pCmd
	switch (*pCmd)
	{                                          // stream
		case L'*': CatchMode = cmtAllInOne; ++pCmd; break;  // <* - redirect #stderr# and #stdout# as one stream
		case L'1': CatchMode = cmtStdOut;   ++pCmd; break;  // <1 - redirect only standard output stream #stdout#
		case L'2': CatchMode = cmtStdErr;   ++pCmd; break;  // <2 - redirect only standard output stream #stderr#
		case L'?': CatchMode = cmtDiff;     ++pCmd; break;  // <? - redirect #stderr# and #stdout# as different streams
		//default:   foundStream=false;  break;
	}

	static struct {
		wchar_t C;
		enShowCmdOutput M;
	} mode[]={
		{L'>', scoHide},    // ignore the console output of the program and display only message about its execution.
		{L'<', scoShow},    // save console output and make it available for viewing with #Ctrl-O#.
		{L'+', scoShowAll}, // same as #<#, but displays on the screen redirected output of the program along with console output
	};

	// edit:<SM object
	//        ^pCmd
	for(size_t I=0; I < ARRAYSIZE(mode); ++I)
	{
		if (mode[I].C == *pCmd && foundStream)
		{
			ShowCmdOutput = mode[I].M;
			++pCmd;
			break;
		}
	}
	// edit:<SM object
	//         ^pCmd

	FSF.LTrim(pCmd);
	// edit:<SM object
	//          ^pCmd
}


static wchar_t* __getContent(int outputtofile,wchar_t *pCmd)
{
	wchar_t *Ptr=nullptr;

	if (outputtofile)
	{
		bool foundFile=false;
		size_t shift;
		Ptr = loadFile(pCmd, Opt.MaxDataSize/sizeof(wchar_t), outputtofile, shift, foundFile);

		if (Ptr)
		{
			wchar_t *DestPath=new wchar_t[lstrlen(Ptr+shift)+1];
			if (DestPath)
			{
				lstrcpy(DestPath, Ptr+shift);
				delete[] Ptr;

				if (!(Ptr=wcschr(DestPath,L'\r')))
					Ptr=wcschr(DestPath,L'\n');

				if (Ptr)
					*Ptr=0;
				Ptr=DestPath;
			}
			else
			{
				delete[] Ptr;
				Ptr=nullptr;
			}
		}
	}
	else
	{
		Ptr=new wchar_t[lstrlen(pCmd)+1];
		if (Ptr)
			lstrcpy(Ptr,pCmd);
	}

	return Ptr;
}

/*
load:path
load:<path
*/
wchar_t* __proc_Load(int outputtofile,wchar_t *pCmd)
{
	wchar_t *Ptr=__getContent(outputtofile,pCmd);

	if (Ptr)
	{
		FSF.Unquote(Ptr);
		wchar_t *temp=ExpandEnv(Ptr,nullptr);
		if (temp)
		{
			Info.PluginsControl(INVALID_HANDLE_VALUE,PCTL_LOADPLUGIN,PLT_PATH,temp);
			delete[] temp;
		}

		delete[] Ptr;
	}

	return nullptr;
}

/*
unload:path
unload:guid
unload:<path
*/
wchar_t* __proc_Unload(int outputtofile,wchar_t *pCmd)
{
	wchar_t *Ptr=__getContent(outputtofile,pCmd);

	if (Ptr)
	{
		GUID FindGuid;
		bool guidMode=StrToGuid(Ptr,&FindGuid);
		if (!guidMode && *Ptr == L'{')
		{
			wchar_t *PtrTemp=Ptr+1;
			wchar_t *ptrNameEnd=PtrTemp+lstrlen(PtrTemp)-1;

        	if (*ptrNameEnd == L'}')
				*ptrNameEnd=0;
			guidMode=StrToGuid(PtrTemp,&FindGuid);
		}

		if (guidMode)
		{
			HANDLE hPlugin = reinterpret_cast<HANDLE>(Info.PluginsControl(INVALID_HANDLE_VALUE,PCTL_FINDPLUGIN,PFM_GUID,&FindGuid));

			if(hPlugin)
			{
				Info.PluginsControl(hPlugin,PCTL_UNLOADPLUGIN,0,nullptr);
			}
		}
		else
		{
			FSF.Unquote(Ptr);
			wchar_t *temp=ExpandEnv(Ptr,nullptr);
			if (temp)
			{
				HANDLE hPlugin = reinterpret_cast<HANDLE>(Info.PluginsControl(INVALID_HANDLE_VALUE,PCTL_FINDPLUGIN,PFM_MODULENAME,temp));
				if(hPlugin)
					Info.PluginsControl(hPlugin,PCTL_UNLOADPLUGIN,0,nullptr);

				delete[] temp;
			}
		}

		delete[] Ptr;
	}

	return nullptr;
}

/*
goto:path
goto:<path
*/
wchar_t* __proc_Goto(int outputtofile,wchar_t *pCmd)
{
	wchar_t *Ptr=nullptr;

	if (outputtofile)
		Ptr=GetShellLinkPath(pCmd);
	if (!Ptr)
		Ptr=__getContent(outputtofile,pCmd);

	if (Ptr)
	{
		wchar_t *temp=ExpandEnv(Ptr,nullptr);
		if (temp)
		{
			delete[] Ptr;
			Ptr=temp;
		}
		else
		{
			delete[] Ptr;
			Ptr=nullptr;
		}
	}

	return Ptr;
}

/*
whereis:path
whereis:<path
*/
wchar_t* __proc_WhereIs(int outputtofile,wchar_t *pCmd,bool Dir)
{
	wchar_t *DestPath=nullptr;
	wchar_t *Ptr=__getContent(outputtofile,pCmd);

	if (Ptr)
	{
		FSF.Unquote(Ptr);

		wchar_t *temp=ExpandEnv(Ptr,nullptr);
		if (temp)
		{
			wchar_t *Path = nullptr, *FARHOMEPath = nullptr;

			size_t CurDirLength=FSF.GetCurrentDirectory(0,nullptr);
			int    FARHOMELength=GetEnvironmentVariable(L"FARHOME", FARHOMEPath, 0);
			int    PathLength=GetEnvironmentVariable(L"PATH", Path, 0);

			wchar_t *PathExt = nullptr;
			int PathExtLength=GetEnvironmentVariable(L"PATHEXT", PathExt, 0);
			PathExt=new wchar_t[PathExtLength+1];
			if (PathExt)
			{
				GetEnvironmentVariable(L"PATHEXT", PathExt, PathExtLength);
				wchar_t *pPathExt=PathExt;
				while(*pPathExt)
				{
					if (*pPathExt == L';')
						*pPathExt=0;
					pPathExt++;
				}
				PathExt[PathExtLength]=0;
			}

			wchar_t *AllPath=new wchar_t[CurDirLength+FARHOMELength+PathLength+8];
			if (AllPath)
			{
				wchar_t *ptrAllPath=AllPath;

				// 1. Current folder
				FSF.GetCurrentDirectory(CurDirLength,ptrAllPath);
				lstrcat(ptrAllPath,L";");
				ptrAllPath+=lstrlen(ptrAllPath);

				// 2. The directory pointed to by the environment variable %FARHOME%
				GetEnvironmentVariable(L"FARHOME", ptrAllPath, FARHOMELength);
				lstrcat(ptrAllPath,L";");
				ptrAllPath+=lstrlen(ptrAllPath);

				// 3. Folders in the system environment variable #PATH#
				GetEnvironmentVariable(L"PATH", ptrAllPath, PathLength);


				wchar_t *pPathExt=PathExt;
				wchar_t *ptempFind=nullptr;
				wchar_t *tempFind=new wchar_t[lstrlen(temp)+(pPathExt?PathExtLength:0)+1];

				if (tempFind)
				{
					lstrcpy(tempFind,temp);
					ptempFind=tempFind+lstrlen(tempFind);
				}

				for (;;)
				{
					DWORD DestPathSize = SearchPath(AllPath,(tempFind?tempFind:temp),nullptr,0,nullptr,nullptr);
					DestPath=new wchar_t[DestPathSize+1];
					if (DestPath)
					{
						*DestPath=0;

						wchar_t *pFile;
						SearchPath(AllPath, (tempFind?tempFind:temp), NULL, DestPathSize, DestPath, &pFile);

						if (*DestPath==0) // 4..6
							SearchPath(NULL, (tempFind?tempFind:temp), NULL, DestPathSize, DestPath, &pFile);

						if (*DestPath)
						{
							DWORD FTAttr=GetFileAttributes(DestPath);
							if (FTAttr != 0xFFFFFFFF)
							{
								if ((FTAttr&FILE_ATTRIBUTE_DIRECTORY))
								{
									if (Dir)
										break;
								}
								else
								{
									break;
								}
							}
						}
					}

					if (pPathExt)
					{
						pPathExt+=lstrlen(pPathExt)+1;
						if (!*pPathExt)
							break;
						if (ptempFind)
							lstrcpy(ptempFind,pPathExt);
					}
					else
						break;
				}

				if (tempFind)
					delete[] tempFind;

				delete[] AllPath;
			}

			// 7..8 Contents of the registry key
			if (!DestPath || !*DestPath)
			{
				if (DestPath)
					delete[] DestPath;
				DestPath=nullptr;

				wchar_t *pPathExt=PathExt;
				wchar_t *ptempFind=nullptr;
				wchar_t *tempFind=new wchar_t[lstrlen(Ptr)+(pPathExt?PathExtLength:0)+1];

				if (tempFind)
				{
					lstrcpy(tempFind,Ptr);
					ptempFind=tempFind+lstrlen(tempFind);
				}

				const wchar_t RegPath[]=L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";

				bool found=false;
				while (!found)
				{
					wchar_t *FullKeyName=new wchar_t[lstrlen(RegPath)+lstrlen(tempFind?tempFind:Ptr)+1];
					if (FullKeyName)
					{
						lstrcpy(FullKeyName,RegPath);
						lstrcat(FullKeyName,(tempFind?tempFind:Ptr));

						HKEY RootFindKey[2]={HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE},hKey;
						for (size_t I=0; I < ARRAYSIZE(RootFindKey); ++I)
						{
							if (RegOpenKeyEx(RootFindKey[I], FullKeyName, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
							{
								DWORD Type, DestPathSize=0;
								if (RegQueryValueEx(hKey,L"", nullptr, nullptr, nullptr, &DestPathSize) == ERROR_SUCCESS)
								{
									DestPath=new wchar_t[DestPathSize+1];
									if (DestPath)
									{
										RegQueryValueEx(hKey,L"", 0, &Type, (LPBYTE) DestPath, &DestPathSize);
										delete[] Ptr;
										Ptr=ExpandEnv(DestPath,nullptr);
										delete[] DestPath;
										found=true;
									}
								}
								RegCloseKey(hKey);
								break;
							}
						}

						delete[] FullKeyName;
					}
					else
					{
						delete[] Ptr;
						Ptr=nullptr;
					}

					if (pPathExt)
					{
						pPathExt+=lstrlen(pPathExt)+1;
						if (!*pPathExt)
							break;
						if (ptempFind)
							lstrcpy(ptempFind,pPathExt);
					}
					else
						break;
				}
				if (tempFind)
					delete[] tempFind;
			}
			else
			{
				delete[] Ptr;
				Ptr=DestPath;
			}

			if (PathExt)
				delete[] PathExt;

			delete[] temp;
		}
		else
		{
			delete[] Ptr;
			Ptr=nullptr;
		}
	}


	return Ptr;
}

bool __proc_Link(int /*outputtofile*/,wchar_t *pCmd)
{
	bool Ret=false;
	bool NeedSymLink=false;
	MKLINK_FLAGS LinkFlags=MLF_NONE;
	wchar_t *Arg2=NULL;

	while (*pCmd && *pCmd == L'/')
	{
		if (!FSF.LStrnicmp(pCmd,L"/MSG",4))
		{
			LinkFlags|=MLF_SHOWERRMSG;
			pCmd=FSF.Trim(pCmd+4);
		}
		else if (!FSF.LStrnicmp(pCmd,L"/N",2))
		{
			LinkFlags|=MLF_DONOTUPDATEPANEL;
			pCmd=FSF.Trim(pCmd+2);
		}
		else if (!FSF.LStrnicmp(pCmd,L"/S",2))
		{
			NeedSymLink=true;
			pCmd=FSF.Trim(pCmd+2);
		}
		else
		{
			break;
		}
	}

	if (*pCmd == L'"')
	{
		Arg2=wcschr(pCmd+1,L'"');

		if (Arg2)
		{
			*++Arg2=0;
			Arg2=FSF.Trim(++Arg2);

			if (*Arg2 == L'"')
			{
				wchar_t *Arg3=wcschr(Arg2+1,L'"');

				if (Arg3)
					*++Arg3=0;

				FSF.Unquote(Arg2);
			}
		}

		FSF.Unquote(pCmd);
	}
	else
	{
		Arg2=wcschr(pCmd+1,L' ');

		if (Arg2)
		{
			*Arg2=0;
			Arg2=FSF.Trim(++Arg2);

			if (*Arg2 == L'"')
			{
				wchar_t *Arg3=wcschr(Arg2+1,L'"');

				if (Arg3)
					*++Arg3=0;

				FSF.Unquote(Arg2);
			}
		}
	}

	DWORD FTAttr=0xFFFFFFFF;

	{
		size_t tempSize=FSF.ConvertPath(CPM_FULL, pCmd, nullptr, 0);
		wchar_t *temp=new wchar_t[tempSize+1];
		if (temp)
		{
			FSF.ConvertPath(CPM_FULL, pCmd, temp, tempSize);
			FTAttr=GetFileAttributes(temp);
			delete[] temp;
		}
	}

	if (Arg2)
	{
		wchar_t Disk[16];
		LINK_TYPE LinkType=LINK_HARDLINK;

		if (pCmd[1] == L':' && ((pCmd[2] == L'\\' && pCmd[3] == 0) || pCmd[2] == 0))
		{
			if (pCmd[2] == 0)
			{
				Disk[0]=pCmd[0];
				Disk[1]=pCmd[1];
				Disk[2]=L'\\';
				Disk[3]=0;
				pCmd=Disk;
			}

			LinkType=LINK_VOLMOUNT;
		}
		else if (FTAttr != 0xFFFFFFFF)
		{
			if (FTAttr&FILE_ATTRIBUTE_DIRECTORY)
				LinkType=NeedSymLink?LINK_SYMLINKDIR:LINK_JUNCTION;
			else
				LinkType=NeedSymLink?LINK_SYMLINKFILE:LINK_HARDLINK;
		}
		FSF.MkLink(pCmd,Arg2,LinkType,LinkFlags);

		return true;
	}

	return Ret;
}

static wchar_t *GetEditParam(wchar_t *farcmd, int &StartLine, int &StartChar, bool &BracketsOk)
{
	BracketsOk=false;

	if (*farcmd == L'[')
	{
		wchar_t *oBracket=farcmd;
		wchar_t *cBracket=wcschr(oBracket,L']');
		wchar_t *ptrSep=wcsstr(oBracket,Opt.Separator);
		if (cBracket && (!ptrSep || (ptrSep && cBracket<ptrSep)))
		{
			wchar_t *comma=wcschr(oBracket,L',');
			if (comma)
			{
				if (comma > oBracket && comma < cBracket)
				{
					StartLine=GetInt(oBracket+1,comma);
					StartChar=GetInt(comma+1,cBracket);

					if (StartLine>-2 && StartChar>-2)
					{
						farcmd=cBracket+1;
						BracketsOk=true;
					}
				}
			}
			else if ((StartLine=GetInt(oBracket+1,cBracket))>-2)
			{
				farcmd=cBracket+1;
				BracketsOk=true;
			}
		}
		else
			BracketsOk=true;
	}
	else
		BracketsOk=true;

	FSF.LTrim(farcmd);
	return farcmd;
}

wchar_t* OpenFromCommandLine(const wchar_t *_farcmd)
{
	if (!_farcmd)
		return nullptr;

	const wchar_t *PrefHlp=L"Contents";
	BOOL showhelp=TRUE;

	static wchar_t farcmdbuf[MAX_PATH*10]; // BUGBUG!!!

	wchar_t *farcmd=farcmdbuf;
	lstrcpy(farcmdbuf, _farcmd);
	FSF.RTrim(farcmdbuf);

	if (lstrlen(farcmd) > 3)
	{
		int ShowCmdOutput=Opt.ShowCmdOutput;
		int CatchMode=Opt.CatchMode;

		int StartLine=-1, StartChar=-1;
		int outputtofile=0;
		BOOL allOK=TRUE;
		wchar_t *pCmd=NULL;

		enum PrefType {
			prefNone,
			prefView, prefEdit, prefClip,
			prefGoto, prefWhereIs, prefLoad, prefUnload, prefLink,
			prefRun,
		} PrefIdx=prefNone;

		// find pref
		{
			static struct
			{
				PrefType Pref;
				const wchar_t *Name;
				const wchar_t *HelpName;
			} Pref[]=
			{
				{prefRun,     L"RUN",     L"Run"},         // run:[<separator>]<file> < <command>
				{prefView,    L"VIEW",    L"View"},        // view:[<separator>]<object>
				{prefClip,    L"CLIP",    L"Clip"},        // clip:[<separator>]<object>
				{prefWhereIs, L"WHEREIS", L"WhereIs"},     // whereis:[<separator>]<object>
				{prefEdit,    L"EDIT",    L"Edit"},        // edit:[[<options>]<separator>]<object>
				{prefGoto,    L"GOTO",    L"Goto"},        // goto:[<separator>]<object>
				{prefLink,    L"LINK",    L"Link"},        // link:[<separator>][<op>]<separator><source><separator><dest>
				{prefLoad,    L"LOAD",    L"Load"},        // load:[<separator>]<file>
				{prefUnload,  L"UNLOAD",  L"Unload"},      // unload:[<separator>]<file>
			};

			for (size_t I=0; I < ARRAYSIZE(Pref); ++I)
			{
				int lenPref=lstrlen(Pref[I].Name);

				if (!_memicmp(farcmd,Pref[I].Name,lenPref))
				{
					farcmd+=lenPref;
					if (*farcmd == L':')
						farcmd++;
					PrefIdx=Pref[I].Pref;
					PrefHlp=Pref[I].HelpName;
					break;
				}
			}
		}

		// farcmd = [[<options>]<separator>]<object>
		// farcmd = [<separator>]<object>
		if (PrefIdx != prefNone)
		{
			int SeparatorLen=lstrlen(Opt.Separator);
			wchar_t *runFile=nullptr;
			bool BracketsOk=true;

			if (PrefIdx == prefEdit) //edit:
			{
				// [Y,X] object
				// [Y] object
				//  object
				// object
				// <[Y,X] object
				// <[Y] object
				// < object
				// <object
				// ^---farcmd
				farcmd=GetEditParam(farcmd,StartLine,StartChar,BracketsOk);
				// object
				// ^farcmd
			}
			else if (PrefIdx == prefRun) // run:
			{
				// file<command
				// ^farcmd
				pCmd = wcschr(farcmd,L'<');
				if (pCmd)
				{
					// file<command
					//     ^pCmd
					*pCmd = 0;
					runFile=new wchar_t[lstrlen(farcmd)+1];
					if (runFile)
					{
						lstrcpy(runFile, farcmd);
						FSF.Trim(runFile);
					}
					*pCmd = L'<';
					farcmd = pCmd;
					// <command
					// ^farcmd
					// ^pCmd
					showhelp=FALSE;
				}
			}

			FSF.LTrim(farcmd); //???

			// <object
			// <S object
			// <SM object
			// object
			// S object
			// SM object
			// ^farcmd
			if (*farcmd==L'<') // start process?
			{
				pCmd=farcmd+1;
				//object
				//S object
				//SM object
				//^farcmd
				outputtofile=1;
				GetStreamAndMode(pCmd, ShowCmdOutput, CatchMode);
				//object
				//^farcmd
			}
			else
			{
				if (pCmd)
				{
					wchar_t *Quote=wcschr(farcmd,L'\"');

					if (Quote)
					{
						if (Quote<=pCmd)
							pCmd=farcmd;
						else
							pCmd+=SeparatorLen;
					}
					else
						pCmd+=SeparatorLen;
				}
				else
				{
					pCmd=farcmd;
				}
			}

			if (pCmd)
			{
				FSF.LTrim(pCmd);

				/*
				if (!outputtofile)
				{
					if (*pCmd==L'<')
						outputtofile=1;

					pCmd+=outputtofile;
				}

				if (outputtofile && (PrefIdx == prefView||PrefIdx == prefEdit||PrefIdx == prefClip))
					GetStreamAndMode(pCmd, ShowCmdOutput, CatchMode);
                */
				if (*pCmd && BracketsOk)
				{
					showhelp=FALSE;

					switch (PrefIdx)
					{
						case prefGoto: // goto:[<]object
						{
							return __proc_Goto(outputtofile,pCmd);
						}
						case prefWhereIs: // whereis:[<]object
						{
							return __proc_WhereIs(outputtofile,pCmd);
						}
						case prefLoad:  // load:[<]file
						{
							return __proc_Load(outputtofile,pCmd);
						}
						case prefUnload:  // unload:[<]file
						{
							return __proc_Unload(outputtofile,pCmd);
						}
						case prefLink: //link [/msg] [/n] источник назначение
						{
							if (!__proc_Link(outputtofile,pCmd))
								Info.ShowHelp(Info.ModuleName,PrefHlp,0);
							return nullptr;
						}
						default:
						{
							//HANDLE hScreen = INVALID_HANDLE_VALUE;
							wchar_t *cmd=nullptr;

							if (outputtofile)
							{
								wchar_t *resAlias=ProcessOSAliases(pCmd);
								if (resAlias)
								{
									lstrcpyn(pCmd,resAlias,(int)(ARRAYSIZE(farcmdbuf)-(pCmd-farcmdbuf)/sizeof(wchar_t)));
									delete[] resAlias;
								}
							}

							wchar_t *tempDir = nullptr;
							wchar_t *pTempFileNameOut=nullptr, *pTempFileNameErr=nullptr;

							if (outputtofile)
							{
								// check work dir
								if (*pCmd == L'|')
								{
									wchar_t *endTempDir = wcschr(pCmd+1, L'|');
									if (endTempDir)
									{
										*endTempDir = 0;
										tempDir = pCmd+1;
										pCmd = endTempDir;
										FSF.LTrim(++pCmd);
									}
								}
							}

							wchar_t *temp=new wchar_t[lstrlen(pCmd)+1];
							if (temp)
							{
								lstrcpy(temp,pCmd);

								if (!outputtofile)
									FSF.Unquote(temp);

								if (temp)
								{
									wchar_t *EpxTemp=ExpandEnv(temp,nullptr);
									if (EpxTemp)
									{
										delete[] temp;
										temp=EpxTemp;
									}
								}
							}

							const wchar_t *titleOut=L"", *titleErr=L"";

							if (!outputtofile)
							{
								size_t SizeFullPath=FSF.ConvertPath(CPM_FULL, temp, nullptr, 0);
								pTempFileNameOut=new wchar_t[SizeFullPath+1];
								if (pTempFileNameOut)
								{
									FSF.ConvertPath(CPM_FULL, temp, pTempFileNameOut, SizeFullPath);
								}
								else
									allOK = FALSE;
							}
							else // <Start process>
							{
								// разделение потоков
								int catchStdOutput = CatchMode != cmtStdErr;
								int catchStdError  = CatchMode != cmtStdOut;
								int catchSeparate  = (CatchMode == cmtDiff) && (PrefIdx == prefView || PrefIdx == prefEdit || PrefIdx == prefClip);
								int catchAllInOne  = 0;
								/*
									                |CatchMode               |mode         |
									                |<   |<1  |<2  |<*  |<?  |<<  |<>  |<+ |
									catchAllInOne   |0 1 |0 0 |0 0 |0 1 |0 0 |0 1 |0 1 |0 1|
									catchSeparate   |0 0 |0 0 |0 0 |0 0 |1 1 |0 0 |0 0 |0 0|
									catchStdOutput  |1 1 |1 1 |0 0 |1 1 |1 1 |1 1 |1 1 |1 1|
									catchStdError   |1 0 |0 0 |1 1 |1 0 |1 1 |1 0 |1 0 |1 0|
									ShowCmdOutput   |0 0 |0 0 |0 0 |0 0 |0 0 |1 1 |0 0 |2 2|
								*/


								if (PrefIdx == prefRun)
								{
									if (runFile && *runFile)
									{
										FSF.Unquote(runFile);

										wchar_t *pTempFileNameErrExp=ExpandEnv(runFile,nullptr);
										if (pTempFileNameErrExp)
										{
											size_t sizeRunFile=FSF.ConvertPath(CPM_FULL,pTempFileNameErrExp,nullptr,0);

											pTempFileNameErr=new wchar_t[sizeRunFile+1];
											if (pTempFileNameErr)
											{
												FSF.ConvertPath(CPM_FULL,pTempFileNameErrExp,pTempFileNameErr,sizeRunFile);

												pTempFileNameOut=new wchar_t[lstrlen(pTempFileNameErr)+1];
												if (pTempFileNameOut)
												{
													lstrcpy(pTempFileNameOut,pTempFileNameErr);
													allOK = TRUE;
												}
											}
											delete[] pTempFileNameErrExp;

										}
									}
								}
								else
								{
									allOK = MakeTempNames(&pTempFileNameOut, &pTempFileNameErr);
								}


								if (allOK)
								{
									DWORD InMode=0, OutMode=0, ErrMode=0;
									GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &OutMode);
									GetConsoleMode(GetStdHandle(STD_ERROR_HANDLE),  &ErrMode);
									GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),  &InMode);

									allOK = FALSE;

									wchar_t *cmd=temp;
									const wchar_t* fullcmd=MakeExecuteString(cmd);

									if (catchStdOutput && catchStdError)
									{
										if (catchSeparate)
										{
											if (ShowCmdOutput == scoShowAll) // <+
												ShowCmdOutput = scoShow; // <<
										}
										else
										{
											catchStdError = 0;
											catchAllInOne = 1;
										}
									}

									HANDLE FileHandleOut=pTempFileNameOut?createFile(pTempFileNameOut,catchStdOutput):INVALID_HANDLE_VALUE;
									HANDLE FileHandleErr=pTempFileNameErr?createFile(pTempFileNameErr,catchStdError):INVALID_HANDLE_VALUE;

									if ((!catchStdOutput || FileHandleOut != INVALID_HANDLE_VALUE) && (!catchStdError || FileHandleErr != INVALID_HANDLE_VALUE))
									{
										HANDLE StdInput  = GetStdHandle(STD_INPUT_HANDLE);
										HANDLE StdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

										CONSOLE_SCREEN_BUFFER_INFO csbi;
										GetConsoleScreenBufferInfo(StdOutput,&csbi);

										DWORD ConsoleMode;
										GetConsoleMode(StdInput,&ConsoleMode);

										STARTUPINFO si;
										memset(&si,0,sizeof(si));
										si.cb         = sizeof(si);
										si.dwFlags    = STARTF_USESTDHANDLES;
										si.hStdInput  = StdInput;
										si.hStdOutput = catchStdOutput ? FileHandleOut : StdOutput;
										si.hStdError  = catchStdError  ? FileHandleErr : StdOutput;

										if (catchAllInOne)
											si.hStdError = si.hStdOutput;

										TThreadData *td = nullptr;

										//if (ShowCmdOutput == scoHide) hScreen = Info.SaveScreen(0, 0, -1, -1);

										if (ShowCmdOutput)
											Info.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETUSERSCREEN,0,0);

										//if (ShowCmdOutput) clearScreen(StdOutput,StdInput,csbi); // ??? CHECK

										if (ShowCmdOutput) // <+ || <<
										{
											Info.Text(0, 0, 0, NULL);

											COORD C;
											C.X = 0;
											C.Y = csbi.dwCursorPosition.Y;
											SetConsoleCursorPosition(StdOutput,C);
											SetConsoleMode(StdInput,ENABLE_PROCESSED_INPUT|ENABLE_LINE_INPUT|ENABLE_ECHO_INPUT|ENABLE_MOUSE_INPUT);
										}

										if (ShowCmdOutput)// == scoShowAll) // <+
										{
											// данные для нитки параллельного вывода
											td = new TThreadData;

											if (td)
											{
												td->type = enThreadShowOutput;

												for (int i = 0 ; i < enStreamMAX ; i++)
												{
													TShowOutputStreamData *sd = &(td->stream[i]);
													sd->hRead = sd->hWrite = sd->hConsole = INVALID_HANDLE_VALUE;
												}

												if (catchStdError && pTempFileNameErr)
													createFileStream(pTempFileNameErr,FileHandleErr,&(td->stream[enStreamErr]),StdOutput);

												if (catchStdOutput && pTempFileNameOut)
													createFileStream(pTempFileNameOut,FileHandleOut,&(td->stream[enStreamOut]),StdOutput);
											}
										}

										wchar_t *SaveDir=nullptr;
										PROCESS_INFORMATION pi;
										memset(&pi,0,sizeof(pi));

										if (tempDir)
										{
											SaveDir=getCurrDir(true);
											wchar_t *workDir=ExpandEnv(tempDir,nullptr);
											if (workDir)
											{
												SetCurrentDirectory(workDir);
												delete[] workDir;
											}
										}

										ConsoleTitle consoleTitle(cmd);

										wchar_t* CurDir=getCurrDir(tempDir?true:false);

										BOOL Created=CreateProcess(NULL,(LPWSTR)fullcmd,NULL,NULL,TRUE,0,NULL,CurDir,&si,&pi);

										if (CurDir)
											delete[] CurDir;

										if (Created)
										{
											if (td)
											{
												td->process = pi.hProcess;
												ThreadWhatUpdateScreen(td);
												closeHandle(FileHandleOut);
												closeHandle(FileHandleErr);

												// "дочищаем" остатки вывода, которые не успели вывестись в ThreadWhatUpdateScreen()
												for (int i = 0 ; i < enStreamMAX ; i++)
												{
													TShowOutputStreamData *sd = &(td->stream[i]);

													if (sd->hWrite != INVALID_HANDLE_VALUE && sd->hRead != INVALID_HANDLE_VALUE)
														while (showPartOfOutput(sd,false))
															;

													closeHandle(sd->hRead);
												}

												delete td;
												td=nullptr;
											}
											else
											{
												if (ShowCmdOutput == scoHide) // <>
													td = new TThreadData;

												if (td)
												{
													td->type = enThreadHideOutput;
													lstrcpyn(td->title, GetMsg(MConfig), ARRAYSIZE(td->title)-1);
													lstrcpyn(td->cmd, cmd?cmd:L"", ARRAYSIZE(td->cmd)-1);
													td->stream[enStreamOut].hWrite = FileHandleOut;
													td->stream[enStreamErr].hWrite = FileHandleErr;

													td->process = pi.hProcess;
													ThreadWhatUpdateScreen(td);
												}
												else
												{
													WaitForSingleObject(pi.hProcess, INFINITE);
												}
												closeHandle(FileHandleOut);
												closeHandle(FileHandleErr);

												if (td)
												{
													delete td;
													td=nullptr;
												}
											}

											closeHandle(pi.hThread);
											closeHandle(pi.hProcess);
											allOK=TRUE;
										}
										else
										{
											closeHandle(FileHandleOut);
											closeHandle(FileHandleErr);
										}

										if (SaveDir)
										{
											SetCurrentDirectory(SaveDir);
											delete[] SaveDir;
										}

										//if (ShowCmdOutput) restoreScreen(StdOutput,StdInput,ConsoleMode,csbi); // ??? CHECK

										if (ShowCmdOutput)// == scoShowAll)
										{
											Info.PanelControl(INVALID_HANDLE_VALUE, FCTL_SETUSERSCREEN,0,0);
											Info.AdvControl(&MainGuid,ACTL_REDRAWALL, 0, nullptr);
										}


										//if (ShowCmdOutput == scoHide && hScreen != INVALID_HANDLE_VALUE) Info.RestoreScreen(hScreen);

									}

									if (fullcmd)
										delete[] fullcmd;

									SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), OutMode);
									SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), ErrMode);
									SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), InMode);
								}

								if (PrefIdx == prefView || PrefIdx == prefEdit)
								{
									if (catchStdError && ((catchStdOutput && catchSeparate) || !catchStdOutput))
										titleErr = GetMsg(MStdErr);

									if (catchStdError && catchStdOutput && catchSeparate)
										titleOut = GetMsg(MStdOut);
								}

							} // </Start process>

							// <"Show" result>
							if (allOK)
							{
								switch (PrefIdx)
								{
									case prefView:
									{
										DWORD Flags = VF_NONMODAL|VF_ENABLE_F6|VF_IMMEDIATERETURN;

										if (outputtofile)
											Flags |= VF_DISABLEHISTORY|VF_DELETEONCLOSE;

										if (pTempFileNameErr)
										{
											if (validForView(pTempFileNameErr, outputtofile?Opt.ViewZeroFiles!=0:true, false))
											{
												MakeVETitle te(titleErr, cmd);
												Info.Viewer(pTempFileNameErr,outputtofile?te.Get():NULL,0,0,-1,-1,Flags,CP_DEFAULT);
											}
											else if (outputtofile)
												killTemp(pTempFileNameErr);
										}

										if (validForView(pTempFileNameOut, outputtofile?Opt.ViewZeroFiles!=0:true, false))
										{
											MakeVETitle to(titleOut, cmd);
											Info.Viewer(pTempFileNameOut,outputtofile?to.Get():NULL,0,0,-1,-1,Flags,CP_DEFAULT);
										}
										else if (outputtofile)
											killTemp(pTempFileNameOut);

										outputtofile=FALSE;

										break;
									}

									case prefEdit:
									{
										DWORD Flags=EF_NONMODAL/*|EF_CREATENEW*/|EF_ENABLE_F6|EF_IMMEDIATERETURN;

										if (outputtofile)
											Flags |= EF_DISABLEHISTORY|EF_DELETEONCLOSE;

										if (pTempFileNameErr)
										{
											if (validForView(pTempFileNameErr, outputtofile?Opt.ViewZeroFiles!=0:true, Opt.EditNewFiles!=0))
											{
												MakeVETitle te(titleErr, cmd);
												Info.Editor(pTempFileNameErr,outputtofile?te.Get():NULL,0,0,-1,-1,Flags,StartLine,StartChar,CP_DEFAULT);
											}
											else if (outputtofile)
												killTemp(pTempFileNameErr);
										}

										if (validForView(pTempFileNameOut, outputtofile?Opt.ViewZeroFiles!=0:true, Opt.EditNewFiles!=0))
										{
											MakeVETitle to(titleOut, cmd);
											Info.Editor(pTempFileNameOut,outputtofile?to.Get():NULL,0,0,-1,-1,Flags,StartLine,StartChar,CP_DEFAULT);
										}
										else if (outputtofile)
											killTemp(pTempFileNameOut);

										outputtofile=FALSE;

										break;
									}

									case prefRun:
									{
										outputtofile=FALSE;

										break;
									}

									case prefClip:
									{
										size_t shift;
										bool foundFile;
										wchar_t *Ptr = loadFile(pTempFileNameOut, Opt.MaxDataSize/sizeof(wchar_t), outputtofile, shift, foundFile);

										FSF.CopyToClipboard(FCT_STREAM,Ptr?Ptr+shift:nullptr);
										if (Ptr)
											delete[] Ptr;

										break;
									}

									default:
										break;
								}

								/*if (ShowCmdOutput == scoShowAll)
								{
									Info.PanelControl(INVALID_HANDLE_VALUE, FCTL_SETUSERSCREEN,0,0);
									Info.AdvControl(&MainGuid,ACTL_REDRAWALL, 0, nullptr);
								}*/

								//if (ShowCmdOutput == scoShow && hScreen != INVALID_HANDLE_VALUE)
								//	Info.RestoreScreen(hScreen);

							} // </"Show" result>

							if (pTempFileNameOut)
							{
								if (outputtofile)
									killTemp(pTempFileNameOut);

								delete[] pTempFileNameOut;
							}

							if (pTempFileNameErr)
							{
								if (outputtofile)
									killTemp(pTempFileNameErr);

								delete[] pTempFileNameErr;
							}

							if (temp)
								delete[] temp;
						} // default:

					} // </switch (PrefIdx)>
				} // </if(*pCmd && BracketsOk)>
			} // </if(pCmd)>

			if (runFile)
				delete[] runFile;

		} // </if (PrefIdx != prefNone)>
	} // </if(lstrlen(farcmd) > 3)>

	if (showhelp)
	{
		Info.ShowHelp(Info.ModuleName,PrefHlp,0);
		return nullptr;
	}

	return nullptr;
}
