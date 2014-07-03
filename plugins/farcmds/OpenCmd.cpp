#ifndef __OpenFromCommandLine
#define __OpenFromCommandLine

#ifndef LIGHTGRAY
#define LIGHTGRAY 7
#endif

#define SIGN_UNICODE    0xFEFF
#define SIGN_REVERSEBOM 0xFFFE
#define SIGN_UTF8_LO    0xBBEF
#define SIGN_UTF8_HI    0xBF


static wchar_t* getCurrDir(bool winApi)
{
	wchar_t *CurDir=nullptr;
	DWORD Size=winApi?GetCurrentDirectory(0,nullptr):FSF.GetCurrentDirectory(0,nullptr);
	if (Size)
	{
		CurDir=new wchar_t[Size];
		if (winApi)
			GetCurrentDirectory(Size,CurDir);
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
		*(wchar_t*)(FSF.PointToName(TempFileName)-1) = 0;
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

static bool validForView(const wchar_t *FileName, int viewEmpty, int editNew)
{
	if (!wmemcmp(FileName, L"\\\\.\\", 4) &&  // специальная обработка имен
			FSF.LIsAlpha(FileName[4]) &&          // вида: \\.\буква:
			FileName[5]==L':' && FileName[6]==0)
		return true;

	if (isDevice(FileName, L"\\\\.\\PhysicalDrive"))
		return true;

	if (isDevice(FileName, L"\\\\.\\cdrom"))
		return true;

	const wchar_t *ptrFileName=FileName;
	wchar_t *ptrCurDir=NULL;

	if (*ptrFileName && FSF.PointToName(ptrFileName) == ptrFileName)
	{
		int dirSize=(int)Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,0,0);

		if (dirSize)
		{
		    FarPanelDirectory* dirInfo=(FarPanelDirectory*)new char[dirSize];
		    dirInfo->StructSize = sizeof(FarPanelDirectory);
			Info.PanelControl(PANEL_ACTIVE,FCTL_GETPANELDIRECTORY,dirSize,dirInfo);
			int Size=lstrlen(dirInfo->Name)+1;
			ptrCurDir=new WCHAR[Size+lstrlen(FileName)+8];
			lstrcpy(ptrCurDir,dirInfo->Name);
			lstrcat(ptrCurDir,L"\\");
			lstrcat(ptrCurDir,ptrFileName);
			ptrFileName=(const wchar_t *)ptrCurDir;
			delete[](char*)dirInfo;
		}
	}

	if (*ptrFileName && FileExists(ptrFileName))
	{
		if (viewEmpty)
		{
			if (ptrCurDir)
				delete[] ptrCurDir;

			return true;
		}

		HANDLE Handle = CreateFile(ptrFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);

		if (Handle != INVALID_HANDLE_VALUE)
		{
			DWORD size = GetFileSize(Handle, NULL);
			CloseHandle(Handle);

			if (ptrCurDir)
				delete[] ptrCurDir;

			return size && (size != 0xFFFFFFFF);
		}
	}
	else if (editNew)
	{
		if (ptrCurDir)
			delete[] ptrCurDir;

		return true;
	}

	if (ptrCurDir)
		delete[] ptrCurDir;

	return false;
}

wchar_t *ConvertBuffer(wchar_t* Ptr,size_t PtrSize,BOOL outputtofile, size_t& shift,bool *unicode)
{
	if (Ptr)
	{
		if (Ptr[0]==SIGN_UNICODE)
		{
			shift=1;
			if (unicode) *unicode=true;
		}
		else if (Ptr[0]==SIGN_REVERSEBOM)
		{
			shift=1;
			size_t PtrLength=lstrlen(Ptr);
			swab((char*)Ptr,(char*)Ptr,int(PtrLength*sizeof(wchar_t)));
			if (unicode) *unicode=true;
		}
		else
		{
			UINT cp=outputtofile?GetConsoleOutputCP():GetACP();

			if (Ptr[0]==SIGN_UTF8_LO&&(Ptr[1]&0xff)==SIGN_UTF8_HI)
			{
				shift=1;
				cp=CP_UTF8;
			}
			else
			{
				int test = IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK | IS_TEXT_UNICODE_NOT_UNICODE_MASK;
				IsTextUnicode(Ptr, PtrSize, &test); // return value is ignored, it's ok.
				if (!(test & IS_TEXT_UNICODE_NOT_UNICODE_MASK) || (test & IS_TEXT_UNICODE_ODD_LENGTH)) // ignore odd
				{
					if (test & IS_TEXT_UNICODE_STATISTICS) // !!! допускаем возможность, что это Unicode
					{
						shift=0;
						if (unicode) *unicode=true;
						return Ptr;
					}
				}
			}


			size_t PtrLength=MultiByteToWideChar(cp,0,(char*)Ptr,-1,NULL,0);

			if (PtrLength)
			{
				wchar_t* NewPtr=new wchar_t[PtrLength+1];
				if (NewPtr)
				{
					if (MultiByteToWideChar(cp,0,(char*)Ptr,-1,NewPtr,(int)PtrLength))
					{
						delete[] Ptr;
						Ptr=NewPtr;
					}
					else
					{
						delete[] NewPtr;
					}
				}
			}
		}
	}
	return Ptr;
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
				if (td->processDone)
					break;
				Sleep(THREADSLEEP);

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
					if (td->processDone)
						break;

					Sleep(THREADSLEEP);
				}

				if (td->processDone)
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


static bool MakeTempNames(wchar_t* tempFileName1, wchar_t* tempFileName2, size_t szTempNames)
{
	static const wchar_t tmpPrefix[] = L"FCP";
	wchar_t fullcmd[MAX_PATH*2];

	if (FSF.MkTemp(tempFileName1,szTempNames,tmpPrefix) > 1)
	{
		DeleteFile(tempFileName1);

		if (CreateDirectory(tempFileName1, NULL))
		{
			bool ok = true;

			if (GetTempFileName(tempFileName1, tmpPrefix, 0, fullcmd))
				lstrcpy(tempFileName2, fullcmd);
			else
				ok = false;

			if (ok && GetTempFileName(tempFileName1, tmpPrefix, 0, fullcmd))
				lstrcpy(tempFileName1, fullcmd);
			else
				ok = false;

			if (ok)
				return true;

			RemoveDirectory(tempFileName1);
		}
	}
	return false;
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

	wchar_t *Ptr = NULL, FileName[MAX_PATH*5];
	ExpandEnvironmentStrings(fn, FileName, ARRAYSIZE(FileName));
	FSF.Unquote(FileName);

	FSF.ConvertPath(CPM_NATIVE, FileName, FileName, ARRAYSIZE(FileName));

	wchar_t *ptrFileName=FileName;
	DWORD read=0;

	if (*ptrFileName && FileExists(ptrFileName))
	{
		foundFile = true;
		HANDLE Handle = CreateFile(ptrFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

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

	return ConvertBuffer(Ptr,read,outputtofile,shift,nullptr);
}


// [stream][mode][|path|]command
// Default:
// 	ShowCmdOutput <- Opt.ShowCmdOutput
//	stream        <- Opt.CatchMode
static void ParseCmdSyntax(wchar_t*& pCmd, int& ShowCmdOutput, int& stream)
{
	switch (*pCmd)
	{                                          // stream
		case L'*': stream = 0; ++pCmd; break;  // redirect #stderr# and #stdout# as one stream
		case L'1': stream = 1; ++pCmd; break;  // redirect only standard output stream #stdout#
		case L'2': stream = 2; ++pCmd; break;  // redirect only standard output stream #stderr#
		case L'?': stream = 3; ++pCmd; break;  // redirect #stderr# and #stdout# as different streams
	}

	bool flg_stream = false;

	switch (*pCmd)
	{                                                                   // mode:
		case L'>': ShowCmdOutput = scoHide;    flg_stream = true; ++pCmd; break; // ignore the console output of the program and display only message about its execution.
		case L'<': ShowCmdOutput = scoShow;    flg_stream = true; ++pCmd; break; // save console output and make it available for viewing with #Ctrl-O#.
		case L'+': ShowCmdOutput = scoShowAll; flg_stream = true; ++pCmd; break; // same as #<#, but displays on the screen redirected output of the program along with console output

		case L' ': flg_stream = true; break;
		case L'|': flg_stream = true; break;
		case L'"': flg_stream = true; break;
	}

	if ((!flg_stream) && (stream == 1 || stream == 2))
	{
		;//--pCmd;
	}

	FSF.LTrim(pCmd);
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
		DWORD sizeExp=ExpandEnvironmentStrings(Ptr,NULL,0);
		wchar_t *temp=new wchar_t[sizeExp+1];
		if (temp)
		{
			ExpandEnvironmentStrings(Ptr,temp,sizeExp);

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
			DWORD sizeExp=ExpandEnvironmentStrings(Ptr,NULL,0);
			wchar_t *temp=new wchar_t[sizeExp+1];
			if (temp)
			{
				ExpandEnvironmentStrings(Ptr,temp,sizeExp);

				HANDLE hPlugin = reinterpret_cast<HANDLE>(Info.PluginsControl(INVALID_HANDLE_VALUE,PCTL_FINDPLUGIN,PFM_MODULENAME,temp));
				if(hPlugin)
				{
					Info.PluginsControl(hPlugin,PCTL_UNLOADPLUGIN,0,nullptr);
				}

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
		DWORD sizeExp=ExpandEnvironmentStrings(Ptr,NULL,0);
		wchar_t *temp=new wchar_t[sizeExp+1];
		if (temp)
		{
			ExpandEnvironmentStrings(Ptr,temp,sizeExp);
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
wchar_t* __proc_WhereIs(int outputtofile,wchar_t *pCmd)
{
	wchar_t *DestPath=nullptr;
	wchar_t *Ptr=__getContent(outputtofile,pCmd);

	if (Ptr)
	{
		FSF.Unquote(Ptr);

		DWORD sizeExp=ExpandEnvironmentStrings(Ptr,NULL,0);
		wchar_t *temp=new wchar_t[sizeExp+1];
		if (temp)
		{
			ExpandEnvironmentStrings(Ptr,temp,sizeExp);

			wchar_t *Path = nullptr, *FARHOMEPath = nullptr;

			size_t CurDirLength=FSF.GetCurrentDirectory(0,nullptr);
			int    FARHOMELength=GetEnvironmentVariable(L"FARHOME", FARHOMEPath, 0);
			int    PathLength=GetEnvironmentVariable(L"PATH", Path, 0);

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

				DWORD DestPathSize = SearchPath(AllPath,temp,nullptr,0,nullptr,nullptr);
				DestPath=new wchar_t[DestPathSize+1];
				if (DestPath)
				{
					*DestPath=0;

					wchar_t *pFile;
					SearchPath(AllPath, temp, NULL, DestPathSize, DestPath, &pFile);

					if (*DestPath==0) // 4..6
						SearchPath(NULL, temp, NULL, DestPathSize, DestPath, &pFile);
				}

				delete[] AllPath;
			}

			// 7..8 Contents of the registry key
			if (!DestPath || !*DestPath)
			{
				if (DestPath)
					delete[] DestPath;
				DestPath=nullptr;

				const wchar_t RegPath[]=L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
				wchar_t *FullKeyName=new wchar_t[lstrlen(RegPath)+lstrlen(Ptr)+1];
				if (FullKeyName)
				{
					lstrcpy(FullKeyName,RegPath);
					lstrcat(FullKeyName,Ptr);

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
									Ptr=DestPath;
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
			}
			else
			{
				delete[] Ptr;
				Ptr=DestPath;
			}

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


wchar_t* OpenFromCommandLine(const wchar_t *_farcmd)
{
	if (!_farcmd)
		return nullptr;

	int View=0,Edit=0,Clip=0,Goto=0,WhereIs=0,Load=0,Unload=0,Link=0,Run=0;
	size_t PrefIdx=static_cast<size_t>(-1);
	struct
	{
		int& Pref;
		LPCTSTR Name;
		LPCTSTR HelpName;
	} Pref[]=
	{
		{Run,     L"RUN",     L"Run"},         // run:[<separator>]<file> < <command>
		{View,    L"VIEW",    L"View"},        // view:[<separator>]<object>
		{Clip,    L"CLIP",    L"Clip"},        // clip:[<separator>]<object>
		{WhereIs, L"WHEREIS", L"WhereIs"},     // whereis:[<separator>]<object>
		{Edit,    L"EDIT",    L"Edit"},        // edit:[[<options>]<separator>]<object>
		{Goto,    L"GOTO",    L"Goto"},        // goto:[<separator>]<object>
		{Link,    L"LINK",    L"Link"},        // link:[<separator>][<op>]<separator><source><separator><dest>
		{Load,    L"LOAD",    L"Load"},        // load:[<separator>]<file>
		{Unload,  L"UNLOAD",  L"Unload"},      // unload:[<separator>]<file>
	};

	static wchar_t farcmdbuf[MAX_PATH*10]; // BUGBUG!!!

	wchar_t *farcmd=farcmdbuf;
	lstrcpy(farcmdbuf, _farcmd);
	FSF.RTrim(farcmdbuf);

	BOOL showhelp=TRUE;

	if (lstrlen(farcmd) > 3)
	{
		int ShowCmdOutput=Opt.ShowCmdOutput;
		int stream=Opt.CatchMode;

		int StartLine=-1, StartChar=-1;
		int outputtofile=0;
		BOOL allOK=TRUE;
		wchar_t *pCmd=NULL;

		for (size_t I=0; I < ARRAYSIZE(Pref); ++I)
		{
			int lenPref=lstrlen(Pref[I].Name);

			if (!_memicmp(farcmd,Pref[I].Name,lenPref))
			{
				farcmd+=lenPref;
				Pref[I].Pref=lenPref;
			}

			if (Pref[I].Pref)
			{
				if (*farcmd == L':')
					farcmd++;

				PrefIdx=I;
				break;
			}
		}

		// farcmd = [[<options>]<separator>]<object>
		// farcmd = [<separator>]<object>
		if (View||Edit||Goto||Clip||WhereIs||Link||Run||Load||Unload)
		{
			int SeparatorLen=lstrlen(Opt.Separator);
			wchar_t *cBracket=NULL, *runFile=nullptr;
			BOOL BracketsOk=TRUE;

			if (Edit)
			{
				// edit:['['<options>']'<separator>]<object>
				//  edit['['<options>']'<separator>]<object>
				//      ^---farcmd
				wchar_t *oBracket;
				BracketsOk=FALSE;
				FSF.LTrim(farcmd);
				oBracket=wcschr(farcmd,L'[');

				if (*farcmd != L'"' && oBracket && oBracket<wcsstr(farcmd,Opt.Separator))
				{
					if ((cBracket=wcschr(oBracket,L']')) != 0 && oBracket < cBracket)
					{
						farcmd=cBracket+1;
						wchar_t *comma=wcschr(oBracket,L',');

						if (comma)
						{
							if (comma > oBracket && comma < cBracket)
								if ((StartLine=GetInt(oBracket+1,comma))>-2 && (StartChar=GetInt(comma+1,cBracket))>-2)
									BracketsOk=TRUE;
						}
						else if ((StartLine=GetInt(oBracket+1,cBracket))>-2)
							BracketsOk=TRUE;
					}
				}
				else
					BracketsOk=TRUE;
			}
			else if (Run)
			{
				pCmd = wcschr(farcmd,L'<');

				if (pCmd)
				{
					*pCmd = 0;
					runFile=new wchar_t[lstrlen(farcmd)+1];
					if (runFile)
					{
						lstrcpy(runFile, farcmd);
						FSF.Trim(runFile);
					}
					*pCmd = L'<';
					farcmd = pCmd;
					showhelp=FALSE;
				}
			}

			if (*farcmd==L'<')
			{
				pCmd=farcmd+1;
				outputtofile=1;
				ParseCmdSyntax(pCmd, ShowCmdOutput, stream);
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

				if (!outputtofile)
				{
					if (*pCmd==L'<')
						outputtofile=1;

					pCmd+=outputtofile;
				}

				if (outputtofile && (View||Edit||Clip))
					ParseCmdSyntax(pCmd, ShowCmdOutput, stream);

				if (*pCmd && BracketsOk)
				{
					showhelp=FALSE;

					if (Goto) // goto:[<separator>]<object>
					{
						return __proc_Goto(outputtofile,pCmd);
					}
					else if (WhereIs)
					{
						return __proc_WhereIs(outputtofile,pCmd);
					}
					else if (Load)  // <file>
					{
						return __proc_Load(outputtofile,pCmd);
					}
					else if (Unload)  // <file>
					{
						return __proc_Unload(outputtofile,pCmd);
					}
					else if (Link) //link [/msg] [/n] источник назначение
					{
						if (!__proc_Link(outputtofile,pCmd))
							Info.ShowHelp(Info.ModuleName,(PrefIdx==static_cast<size_t>(-1))?L"Contents":Pref[PrefIdx].HelpName,0);
						return nullptr;
					}
					else
					{
						//HANDLE hScreen = INVALID_HANDLE_VALUE;
						wchar_t *cmd=nullptr;

						if (outputtofile)
							ProcessOSAliases(pCmd,ARRAYSIZE(farcmdbuf));

						wchar_t *tempDir = nullptr;
						wchar_t TempFileNameOut[MAX_PATH*5], TempFileNameErr[ARRAYSIZE(TempFileNameOut)];   // BUGBUG!!!
						TempFileNameOut[0] = TempFileNameErr[0] = 0;
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
							lstrcpy(temp,pCmd);

						if (!outputtofile && temp)
							FSF.Unquote(temp);

						// <NEED???>
						//wchar_t ExpTemp[ARRAYSIZE(temp)];
						//ExpandEnvironmentStrings(temp,ExpTemp,ARRAYSIZE(ExpTemp));
						//lstrcpy(temp,ExpTemp);
						// </NEED???>

						// разделение потоков
						int catchStdOutput = stream != 2;
						int catchStdError  = stream != 1;
						int catchSeparate  = (stream == 3) && (View || Edit || Clip);
						int catchAllInOne  = 0;

						if (outputtofile)
						{
							if (Run)
							{
								if (runFile && *runFile)
								{
									FSF.Unquote(runFile);
									ExpandEnvironmentStrings(runFile,TempFileNameErr,ARRAYSIZE(TempFileNameErr));
									lstrcpy(TempFileNameOut,TempFileNameErr);
									allOK = TRUE;
								}
							}
							else
								allOK = MakeTempNames(TempFileNameOut, TempFileNameErr, ARRAYSIZE(TempFileNameOut));

							if (allOK)
							{
								wchar_t* fullcmd=nullptr;

								DWORD InMode=0, OutMode=0, ErrMode=0;
								GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &OutMode);
								GetConsoleMode(GetStdHandle(STD_ERROR_HANDLE),  &ErrMode);
								GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),  &InMode);

								allOK = FALSE;

								cmd=new wchar_t[lstrlen(temp)+64];
								if (cmd)
								{
									lstrcpy(cmd,L"%COMSPEC% /c ");

									if (*temp == L'"')
										lstrcat(cmd, L"\"");

									lstrcat(cmd, temp);

									DWORD sizeExptCmd=ExpandEnvironmentStrings(cmd,NULL,0);
									fullcmd=new wchar_t[sizeExptCmd+1];
									if (fullcmd)
									{
										ExpandEnvironmentStrings(cmd,fullcmd,sizeExptCmd);
									}

									lstrcpy(cmd, temp);
								}

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

								HANDLE FileHandleOut=createFile(TempFileNameOut,catchStdOutput);
								HANDLE FileHandleErr=createFile(TempFileNameErr,catchStdError);

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

									//if (ShowCmdOutput == scoShow)
									//	hScreen = Info.SaveScreen(0, 0, -1, -1);

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
											td->processDone = false;

											for (int i = 0 ; i < enStreamMAX ; i++)
											{
												TShowOutputStreamData *sd = &(td->stream[i]);
												sd->hRead = sd->hWrite = sd->hConsole = INVALID_HANDLE_VALUE;
											}

											if (catchStdError)
												createFileStream(TempFileNameErr,FileHandleErr,&(td->stream[enStreamErr]),StdOutput);

											if (catchStdOutput)
												createFileStream(TempFileNameOut,FileHandleOut,&(td->stream[enStreamOut]),StdOutput);
										}
									}

									wchar_t *SaveDir=nullptr;
									PROCESS_INFORMATION pi;
									memset(&pi,0,sizeof(pi));

									if (tempDir)
									{
										SaveDir=getCurrDir(true);
										DWORD sizeExp=ExpandEnvironmentStrings(tempDir,NULL,0);
										wchar_t *workDir=new wchar_t[sizeExp+1];
										if (workDir)
										{
											ExpandEnvironmentStrings(tempDir,workDir,sizeExp);
											SetCurrentDirectory(workDir);
											delete[] workDir;
										}
									}

									ConsoleTitle consoleTitle(cmd);

									wchar_t* CurDir=getCurrDir(tempDir?true:false);

									BOOL Created=CreateProcess(NULL,fullcmd,NULL,NULL,TRUE,0,NULL,CurDir,&si,&pi);

									if (CurDir)
										delete[] CurDir;

									if (Created)
									{
										// нитка параллельного вывода
										HANDLE hThread;
										DWORD dummy;

										if (td)
										{
											hThread = CreateThread(NULL, 0xf000, ThreadWhatUpdateScreen, td, 0, &dummy);
											WaitForSingleObject(pi.hProcess, INFINITE);
											closeHandle(FileHandleOut);
											closeHandle(FileHandleErr);
											td->processDone = true;

											if (hThread)
												WaitForSingleObject(hThread, INFINITE);

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

												td->processDone = false;
												hThread = CreateThread(NULL, 0xf000, ThreadWhatUpdateScreen, td, 0, &dummy);
											}

											WaitForSingleObject(pi.hProcess, INFINITE);
											closeHandle(FileHandleOut);
											closeHandle(FileHandleErr);

											if (td)
											{
												td->processDone = true;

												if (hThread)
													WaitForSingleObject(hThread, INFINITE);

												delete td;
												td=nullptr;
											}
										}

										closeHandle(pi.hThread);
										closeHandle(pi.hProcess);
										closeHandle(hThread);
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


									//if (ShowCmdOutput == scoShow && hScreen != INVALID_HANDLE_VALUE)
									//	Info.RestoreScreen(hScreen);

								}

								SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), OutMode);
								SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), ErrMode);
								SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), InMode);

								if (fullcmd)
									delete[] fullcmd;
							}
						}
						else
						{
							FSF.ConvertPath(CPM_FULL, temp, TempFileNameOut, ARRAYSIZE(TempFileNameOut));
						}

						if (allOK)
						{
							if (View || Edit)
							{
								wchar_t titleOut[MAX_PATH] = L"", titleErr[MAX_PATH] = L"";

								if (catchStdError && ((catchStdOutput && catchSeparate) || !catchStdOutput))
									lstrcpy(titleErr, GetMsg(MStdErr));

								if (catchStdError && catchStdOutput && catchSeparate)
									lstrcpy(titleOut, GetMsg(MStdOut));

								if (View)
								{
									DWORD Flags = VF_NONMODAL|VF_ENABLE_F6|VF_IMMEDIATERETURN;

									if (outputtofile)
										Flags |= VF_DISABLEHISTORY|VF_DELETEONCLOSE;

									if (validForView(TempFileNameErr, Opt.ViewZeroFiles, 0))
									{
										MakeVETitle te(titleErr, cmd);
										Info.Viewer(TempFileNameErr,outputtofile?te.Get():NULL,0,0,-1,-1,Flags,CP_DEFAULT);
									}
									else if (outputtofile)
										killTemp(TempFileNameErr);

									if (validForView(TempFileNameOut, Opt.ViewZeroFiles, 0))
									{
										MakeVETitle to(titleOut, cmd);
										Info.Viewer(TempFileNameOut,outputtofile?to.Get():NULL,0,0,-1,-1,Flags,CP_DEFAULT);
									}
									else if (outputtofile)
										killTemp(TempFileNameOut);

									outputtofile=FALSE;
								}
								else if (Edit)
								{
									DWORD Flags=EF_NONMODAL/*|EF_CREATENEW*/|EF_ENABLE_F6|EF_IMMEDIATERETURN;

									if (outputtofile)
										Flags |= EF_DISABLEHISTORY|EF_DELETEONCLOSE;

									if (validForView(TempFileNameErr, Opt.ViewZeroFiles, Opt.EditNewFiles))
									{
										MakeVETitle te(titleErr, cmd);
										Info.Editor(TempFileNameErr,outputtofile?te.Get():NULL,0,0,-1,-1,Flags,StartLine,StartChar,CP_DEFAULT);
									}
									else if (outputtofile)
										killTemp(TempFileNameErr);

									if (validForView(TempFileNameOut, Opt.ViewZeroFiles, Opt.EditNewFiles))
									{
										MakeVETitle to(titleOut, cmd);
										Info.Editor(TempFileNameOut,outputtofile?to.Get():NULL,0,0,-1,-1,Flags,StartLine,StartChar,CP_DEFAULT);
									}
									else if (outputtofile)
										killTemp(TempFileNameOut);

									outputtofile=FALSE;
								}
							}
							else if (Run)
							{
								outputtofile=FALSE;
							}
							else if (Clip)
							{
								size_t shift;
								bool foundFile;
								wchar_t *Ptr = loadFile(TempFileNameOut, Opt.MaxDataSize/sizeof(wchar_t), outputtofile, shift, foundFile);

								FSF.CopyToClipboard(FCT_STREAM,Ptr?Ptr+shift:nullptr);
								if (Ptr)
									delete[] Ptr;
							}

							/*if (ShowCmdOutput == scoShowAll)
							{
								Info.PanelControl(INVALID_HANDLE_VALUE, FCTL_SETUSERSCREEN,0,0);
								Info.AdvControl(&MainGuid,ACTL_REDRAWALL, 0, nullptr);
							}*/

							//if (ShowCmdOutput == scoShow && hScreen != INVALID_HANDLE_VALUE)
							//	Info.RestoreScreen(hScreen);
						}

						if (outputtofile)
						{
							killTemp(TempFileNameOut);
							killTemp(TempFileNameErr);
						}

						if (cmd)
							delete[] cmd;

						if (temp)
							delete[] temp;
					}
				} // </if(*pCmd && BracketsOk)>
			} // </if(pCmd)>

			if (runFile)
				delete[] runFile;
		} // </if(View||Edit||Goto)>
	} // </if(lstrlen(farcmd) > 3)>

	if (showhelp)
	{
		Info.ShowHelp(Info.ModuleName,(PrefIdx==static_cast<size_t>(-1))?L"Contents":Pref[PrefIdx].HelpName,0);
		return nullptr;
	}

	return nullptr;
}
#endif
