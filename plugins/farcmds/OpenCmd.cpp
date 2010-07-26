#ifndef __OpenFromCommandLine
#define __OpenFromCommandLine

#ifndef LIGHTGRAY
#define LIGHTGRAY 7
#endif

#define SIGN_UNICODE    0xFEFF
#define SIGN_REVERSEBOM 0xFFFE
#define SIGN_UTF8_LO    0xBBEF
#define SIGN_UTF8_HI    0xBF

#ifdef UNICODE
#define DEF_CP_AUTODETECT	,CP_AUTODETECT
#else
#define DEF_CP_AUTODETECT
#endif


static inline bool vh(HANDLE h)
{
	return h != INVALID_HANDLE_VALUE;
}

static void closeHandle(HANDLE& handle)
{
	if (vh(handle))
		CloseHandle(handle);

	handle = INVALID_HANDLE_VALUE;
}

static void killTemp(TCHAR *TempFileName)
{
	if (FileExists(TempFileName))
	{
		DeleteFile(TempFileName);
		*(TCHAR*)(PointToName(TempFileName)-1) = 0;
		RemoveDirectory(TempFileName);
	}
}

inline bool isDevice(const TCHAR* FileName, const TCHAR* dev_begin)
{
	const int len=lstrlen(dev_begin);

	if (LStrnicmp(FileName, dev_begin, len))
		return false;

	FileName+=len;

	if (!*FileName)
		return false;

	while (*FileName>=_T('0') && *FileName<=_T('9'))
		FileName++;

	return !*FileName;
}

static bool validForView(const TCHAR *FileName, int viewEmpty, int editNew)
{
	if (!memcmp(FileName, _T("\\\\.\\"), 4) &&  // специальная обработка имен
			FarIsAlpha(FileName[4]) &&          // вида: \\.\буква:
			FileName[5]==_T(':') && FileName[6]==0)
		return true;

	if (isDevice(FileName, _T("\\\\.\\PhysicalDrive")))
		return true;

	if (isDevice(FileName, _T("\\\\.\\cdrom")))
		return true;

	const TCHAR *ptrFileName=FileName;
	TCHAR *ptrCurDir=NULL;
#ifdef UNICODE

	if (*ptrFileName && PointToName(ptrFileName) == ptrFileName)
	{
		size_t Size=Info.Control(PANEL_ACTIVE,FCTL_GETPANELDIR,0,0);

		if (Size)
		{
			ptrCurDir=new WCHAR[Size+lstrlen(FileName)+8];
			Info.Control(PANEL_ACTIVE,FCTL_GETPANELDIR,(int)Size,reinterpret_cast<LONG_PTR>(ptrCurDir));
			lstrcat(ptrCurDir,_T("\\"));
			lstrcat(ptrCurDir,ptrFileName);
			ptrFileName=(const TCHAR *)ptrCurDir;
		}
	}

#endif

	if (*ptrFileName && FileExists(ptrFileName))
	{
		if (viewEmpty)
		{
			if (ptrCurDir)
				delete[] ptrCurDir;

			return true;
		}

		HANDLE Handle = CreateFile(ptrFileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);

		if (vh(Handle))
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

// нитка параллельного вывода на экран для ":<+"

#define THREADSLEEP  200
#define THREADREDRAW 10

struct TShowOutputStreamData
{
	HANDLE hRead;
	HANDLE hWrite;
	HANDLE hConsole;
};

enum enStream { enStreamOut, enStreamErr, enStreamMAX };
enum enThreadType { enThreadHideOutput, enThreadShowOutput };

struct TThreadData
{
	enThreadType type;
	bool processDone;
	TShowOutputStreamData stream[enStreamMAX];
	TCHAR title[80], cmd[1024];
};

static DWORD showPartOfOutput(TShowOutputStreamData *sd)
{
	DWORD Res = 0;

	if (sd)
	{
		TCHAR ReadBuf[4096+1];
		DWORD BytesRead = 0;

		if (ReadFile(sd->hRead, ReadBuf, sizeof(ReadBuf)-sizeof(TCHAR), &BytesRead, NULL))
		{
			if (BytesRead)
			{
				DWORD dummy;
				ReadBuf[BytesRead] = 0;

				if (vh(sd->hConsole))
					WriteConsole(sd->hConsole, ReadBuf, BytesRead, &dummy, NULL);

				Res = BytesRead;
			}
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

					if (vh(sd->hRead))
						showPartOfOutput(sd);
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

				TCHAR buff[80];
				DWORD sCheck[enStreamMAX];

				for (int i = 0 ; i < enStreamMAX ; i++)
				{
					HANDLE hCheck = td->stream[i].hWrite;

					if (vh(hCheck))
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
						FarSprintf(buff, _T("%lu/%lu"), sCheck[enStreamOut], sCheck[enStreamErr]);
					else
						FarSprintf(buff, _T("%lu"), sCheck[enStreamOut]);
				else if (sCheck[enStreamErr])
					FarSprintf(buff, _T("%lu"), sCheck[enStreamErr]);
				else
					*buff = 0;

				const TCHAR *MsgItems[] = { td->title, td->cmd, buff };
				Info.Message(Info.ModuleNumber, 0, NULL, MsgItems, ArraySize(MsgItems), 0);
			}
		}
	}

	return 0;
}

static bool MakeTempNames(TCHAR* tempFileName1, TCHAR* tempFileName2, size_t szTempNames)
{
	static const TCHAR tmpPrefix[] = _T("FCP");

#ifdef UNICODE
	if (MkTemp(tempFileName1,(DWORD)szTempNames,tmpPrefix) > 1)
#else
	if (MkTemp(tempFileName1,tmpPrefix))
#endif
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
static TCHAR *loadFile(const TCHAR *fn, DWORD maxSize, BOOL outputtofile, size_t& shift, bool& foundFile)
{
	foundFile = false;
	shift=0;

	TCHAR *Ptr = NULL, FileName[MAX_PATH*5];
	ExpandEnvironmentStr(fn, FileName, ArraySize(FileName));
	Unquote(FileName);

#ifdef UNICODE
	FarConvertPath(CPM_NATIVE, FileName, FileName, ArraySize(FileName));
#endif

	TCHAR *ptrFileName=FileName;

	if (*ptrFileName && FileExists(ptrFileName))
	{
		foundFile = true;
		HANDLE Handle = CreateFile(ptrFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

		if (vh(Handle))
		{
			DWORD size = (GetFileSize(Handle, NULL)+(sizeof(TCHAR)/2)) / sizeof(TCHAR);

			if (size >= maxSize)
				size = maxSize-1;

			size *= sizeof(TCHAR);

			if (size)
			{
				DWORD read;

				TCHAR *buff = (TCHAR *)malloc(size+sizeof(TCHAR));

				if (buff)
				{
					if (ReadFile(Handle, buff, size, &read, NULL) && read >= sizeof(TCHAR))
					{
#ifdef UNICODE

						if (read&1)
						{
							buff[read/sizeof(TCHAR)]=buff[read/sizeof(TCHAR)]&0xff;
							read++;
						}

#endif
						buff[read/sizeof(TCHAR)] = 0;
						Ptr = buff;
					}
					else
						free(buff);
				}
			}

			CloseHandle(Handle);
		}

	}

#ifdef UNICODE
	if (Ptr)
	{
		if (Ptr[0]==SIGN_UNICODE)
		{
			shift=1;
		}
		else if (Ptr[0]==SIGN_REVERSEBOM)
		{
			shift=1;
			size_t PtrLength=lstrlen(Ptr);
			swab((char*)Ptr,(char*)Ptr,int(PtrLength*sizeof(TCHAR)));
		}
		else
		{
			UINT cp=outputtofile?GetConsoleOutputCP():GetACP();

			if (Ptr[0]==SIGN_UTF8_LO&&(Ptr[1]&0xff)==SIGN_UTF8_HI)
			{
				shift=1;
				cp=CP_UTF8;
			}

			size_t PtrLength=MultiByteToWideChar(cp,0,(char*)Ptr,-1,NULL,0);

			if (PtrLength)
			{
				TCHAR* NewPtr=(TCHAR*)malloc(PtrLength*sizeof(TCHAR));
				if (NewPtr)
				{
					if (MultiByteToWideChar(cp,0,(char*)Ptr,-1,NewPtr,(int)PtrLength))
					{
						free(Ptr);
						Ptr=NewPtr;
					}
					else
					{
						free(NewPtr);
					}
				}
			}
		}
	}
#endif

	return Ptr;
}

static void ParseCmdSyntax(TCHAR*& pCmd, int& ShowCmdOutput, int& stream)
{
	switch (*pCmd)
	{
		case _T('*'): stream = 0; ++pCmd; break;
		case _T('1'): stream = 1; ++pCmd; break;
		case _T('2'): stream = 2; ++pCmd; break;
		case _T('?'): stream = 3; ++pCmd; break;
	}

	bool flg_stream = false;

	switch (*pCmd)
	{
		case _T('>'): ShowCmdOutput = 0; flg_stream = true; ++pCmd; break;
		case _T('<'): ShowCmdOutput = 1; flg_stream = true; ++pCmd; break;
		case _T('+'): ShowCmdOutput = 2; flg_stream = true; ++pCmd; break;
		case _T(' '): flg_stream = true; break;
		case _T('|'): flg_stream = true; break;
		case _T('"'): flg_stream = true; break;
	}

	if ((!flg_stream) && (stream == 1 || stream == 2))
	{
		--pCmd;
	}

	FarLTrim(pCmd);
}

// тестирует префиск Pref
// сдвигает Src, если нашел (на длину префиска)
// возвращает длину.
static int TestPrefix(TCHAR*& Src,const TCHAR *Pref)
{
	int lenPref=lstrlen(Pref);

	if (!_memicmp(Src,Pref,lenPref))
	{
		Src+=lenPref;
		return lenPref;
	}

	return 0;
}

int OpenFromCommandLine(TCHAR *_farcmd)
{
	if (!_farcmd) return FALSE;

	int View=0,Edit=0,Goto=0,Far=0,Clip=0,WhereIs=0,Macro=0,Link=0,Run=0, Load=0,Unload=0;
	size_t PrefIdx=static_cast<size_t>(-1);
	struct
	{
		int& Pref;
		LPCTSTR Name;
		LPCTSTR HelpName;
	} Pref[]=
	{
		// far:<command>[<options>]<separator><object>
		{Far,_T("FAR"),_T("Contents")},  // ЭТОТ самый первый!
		// view:[<separator>]<object>
		// view<separator><object>
		{View,_T("VIEW"),_T("View")},
		// clip:[<separator>]<object>
		// clip:<separator><object>
		{Clip,_T("CLIP"),_T("Clip")},
		// whereis:[<separator>]<object>
		// whereis<separator><object>
		{WhereIs,_T("WHEREIS"),_T("WhereIs")},
		// edit:[[<options>]<separator>]<object>
		// edit[<options>]<separator><object>
		{Edit,_T("EDIT"),_T("Edit")},
		// goto:[<separator>]<object>
		// goto<separator><object>
		{Goto,_T("GOTO"),_T("Goto")},
		// macro:[<separator>]<object>
		// macro<separator><object>
		{Macro,_T("MACRO"),_T("Macro")},
		// link:[<separator>][<op>]<separator><source><separator><dest>
		// link<separator>[<op>]<separator><source><separator><dest>
		{Link,_T("LINK"),_T("Link")},
		// run:[<separator>]<file> < <command>
		// run<separator><file> < <command>
		{Run,_T("RUN"),_T("Run")},
#ifdef UNICODE
		// load:[<separator>]<file>
		// load<separator><file>
		{Load,_T("LOAD"),_T("Load")},
		// unload:[<separator>]<file>
		// unload<separator><file>
		{Unload,_T("UNLOAD"),_T("Unload")},
#endif
	};

	static TCHAR farcmdbuf[MAX_PATH*10], *farcmd;
	farcmd=farcmdbuf;
	lstrcpy(farcmdbuf, _farcmd);
	FarRTrim(farcmdbuf);
	BOOL showhelp=TRUE;

	if (lstrlen(farcmd) > 3)
	{
		HANDLE FileHandleOut, FileHandleErr;
		int StartLine=-1, StartChar=-1;
		int ShowCmdOutput=Opt.ShowCmdOutput;
		int stream=Opt.CatchMode;
		BOOL outputtofile=0, allOK=TRUE;
		TCHAR *pCmd=NULL;

		for (size_t I=0; I < ArraySize(Pref); ++I)
		{
			Pref[I].Pref=TestPrefix(farcmd,Pref[I].Name);

			if (Pref[I].Pref)
			{
				if (*farcmd == _T(':'))
					farcmd++;

				if (!I)
				{
					//  farcmd = <command>[<options>]<separator><object>
					//  farcmd = <command><separator><object>
					continue;  // для "FAR:" продолжаем
				}

				PrefIdx=I;
				break;
			}
		}

		// farcmd = [[<options>]<separator>]<object>
		// farcmd = [<separator>]<object>
		if (View||Edit||Goto||Clip||WhereIs||Macro||Link||Run||Load||Unload)
		{
			int SeparatorLen=lstrlen(Opt.Separator);
			TCHAR *cBracket=NULL, runFile[MAX_PATH]=_T("");
			BOOL BracketsOk=TRUE;

			if (Edit)
			{
				// edit:['['<options>']'<separator>]<object>
				//  edit['['<options>']'<separator>]<object>
				//      ^---farcmd
				TCHAR *oBracket;
				BracketsOk=FALSE;
				FarLTrim(farcmd);
				oBracket=_tcschr(farcmd,_T('['));

				if (*farcmd != _T('"') && oBracket && oBracket<_tcsstr(farcmd,Opt.Separator))
				{
					if ((cBracket=_tcschr(oBracket,_T(']'))) != 0 && oBracket < cBracket)
					{
						farcmd=cBracket+1;
						TCHAR *comma=_tcschr(oBracket,_T(','));

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
				Far=0;
			}
			else if (Run)   // пятница, 26 апреля 2002, 13:50:08
			{
				pCmd = _tcschr(farcmd,_T('<'));

				if (pCmd)
				{
					*pCmd = 0;
					lstrcpy(runFile, farcmd);
					FarTrim(runFile);
					*pCmd = _T('<');
					farcmd = pCmd;
					showhelp=FALSE;
				}
				Far=0;
			}

			if (Far && BracketsOk && cBracket)
				farcmd=cBracket+1;

			if (*farcmd==_T('<'))
			{
				pCmd=farcmd+1;
				outputtofile=1;
				ParseCmdSyntax(pCmd, ShowCmdOutput, stream);
			}
			else
			{
				/* $ 21.06.2001 SVS
				   Это делаем только, если был префикс "far:",
				   иначе глотается первое слово.
				*/
				if (Far)
					pCmd=_tcsstr(farcmd,Opt.Separator);

				if (pCmd)
				{
					if (Far)
						pCmd+=SeparatorLen;
					else
					{
						TCHAR *Quote=_tcschr(farcmd,_T('\"'));

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
				}
				else
				{
					if (Far)
					{
						if (BracketsOk && cBracket)
							pCmd=farcmd;
					}
					else
						pCmd=farcmd;
				}
			}

			if (pCmd)
			{
				FarLTrim(pCmd);

				if (!outputtofile)
				{
					if (*pCmd==_T('<'))
						outputtofile=1;

					pCmd+=outputtofile;
				}

				if (outputtofile && (View||Edit||Clip))
					ParseCmdSyntax(pCmd, ShowCmdOutput, stream);

				if (*pCmd && BracketsOk)
				{
					showhelp=FALSE;

					if (outputtofile && !(WhereIs || Goto || Link))
						ProcessOSAliases(pCmd,ArraySize(farcmdbuf));

					if (WhereIs || Goto)
					{
						if (outputtofile)
						{
							size_t shift;
							bool foundFile;
							TCHAR *Ptr = loadFile(pCmd, Opt.MaxDataSize/sizeof(TCHAR), outputtofile, shift, foundFile);

							if (Ptr)
							{
								lstrcpyn(selectItem, Ptr+shift, ArraySize(selectItem)-1);
								free(Ptr);

								if (NULL==(Ptr=_tcschr(selectItem,_T('\r'))))
									Ptr=_tcschr(selectItem,_T('\n'));

								if (Ptr!=NULL)
									*Ptr=0;
							}
						}
						else
							lstrcpy(selectItem,pCmd);
					}


					if (Goto)
					{
						TCHAR ExpSelectItem[ArraySize(selectItem)];
						ExpandEnvironmentStr(selectItem,ExpSelectItem,ArraySize(ExpSelectItem));
						lstrcpy(selectItem,ExpSelectItem);
					}
					else if (WhereIs)
					{
						TCHAR pCmdCopy[ArraySize(selectItem)];
						lstrcpy(pCmdCopy,selectItem);
						TCHAR *Path = NULL, *pFile, temp[MAX_PATH*5], *FARHOMEPath = NULL;
#ifndef UNICODE
						int Length=GetCurrentDirectory(ArraySize(cmd),cmd);
#else
						int Length=FSF.GetCurrentDirectory(ArraySize(cmd),cmd);
#endif
						int PathLength=GetEnvironmentVariable(_T("PATH"), Path, 0);
						int FARHOMELength=GetEnvironmentVariable(_T("FARHOME"), FARHOMEPath, 0);
						Unquote(pCmdCopy);
						ExpandEnvironmentStr(pCmdCopy,temp,ArraySize(temp));

						if (Length+PathLength)
						{
							if (NULL!=(Path=(TCHAR *)malloc((Length+PathLength+FARHOMELength+3)*sizeof(TCHAR))))
							{
								FarSprintf(Path,_T("%s;"), cmd);
								GetEnvironmentVariable(_T("FARHOME"), Path+lstrlen(Path), FARHOMELength);
								lstrcat(Path,_T(";"));
								GetEnvironmentVariable(_T("PATH"), Path+lstrlen(Path), PathLength);
								SearchPath(Path, temp, NULL, ArraySize(selectItem), selectItem, &pFile);
#ifndef UNICODE
								CharToOem(selectItem, selectItem);
#endif
								free(Path);
							}

							if (*selectItem==0)
								SearchPath(NULL, temp, NULL, ArraySize(selectItem), selectItem, &pFile);
						}

						if (*selectItem==0)
						{
							HKEY RootFindKey[2]={HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE},hKey;
							TCHAR FullKeyName[512];

							for (size_t I=0; I < ArraySize(RootFindKey); ++I)
							{
								FarSprintf(FullKeyName,_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%s"),pCmdCopy);
#ifndef UNICODE
								OemToChar(FullKeyName, FullKeyName);
#endif

								if (RegOpenKeyEx(RootFindKey[I], FullKeyName, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
								{
									DWORD Type, DataSize=sizeof(selectItem);
									RegQueryValueEx(hKey,_T(""), 0, &Type, (LPBYTE) selectItem, &DataSize);
									RegCloseKey(hKey);
									break;
								}
							}
						}
					}
					else if (Macro)
					{
						ActlKeyMacro command;

						if (!LStrnicmp(pCmd,_T("LOAD"),lstrlen(pCmd)))
							command.Command=MCMD_LOADALL;
						else if (!LStrnicmp(pCmd,_T("SAVE"),lstrlen(pCmd)))
							command.Command=MCMD_SAVEALL;
#ifdef UNICODE
						else if (!LStrnicmp(pCmd,_T("CHECK"),5))
						{
							command.Command=MCMD_CHECKMACRO;
							//command.Param.PlainText.Flags=KSFLAGS_SILENTCHECK;
							command.Param.PlainText.Flags=0;
							pCmd+=5;
						}
#endif
						else if (!LStrnicmp(pCmd,_T("POST"),4))
						{
							command.Command=MCMD_POSTMACROSTRING;
							command.Param.PlainText.Flags=KSFLAGS_DISABLEOUTPUT;
							pCmd+=4;
						}

						switch(command.Command)
						{
							case MCMD_LOADALL:
								Info.AdvControl(Info.ModuleNumber,ACTL_KEYMACRO,&command);
								break;
							case MCMD_SAVEALL:
								Info.AdvControl(Info.ModuleNumber,ACTL_KEYMACRO,&command);
								break;
#ifdef UNICODE
							case MCMD_CHECKMACRO:
#endif
							case MCMD_POSTMACROSTRING:
							{
								TCHAR *SequenceText=NULL;
								TCHAR *Ptr=NULL;
								bool fromFile=false;

								FarLTrim(pCmd);
								if (*pCmd==_T('<'))
								{
									TCHAR *oldCmd=pCmd;
									pCmd++;
									while (*pCmd && IsSpace(*pCmd))
										pCmd++;
									size_t shift;
									bool foundFile;
									Ptr = loadFile(pCmd, Opt.MaxDataSize/sizeof(TCHAR), outputtofile, shift, foundFile);
									if (Ptr)
									{
										SequenceText=Ptr+shift;
										fromFile=true;
									}
									else
									{
										if (foundFile) // файл есть, но была ошибка чтения: все равно из файла
											fromFile=true;
										else
											pCmd=oldCmd;
									}
								}

								if (!fromFile)
								{
									SequenceText=(TCHAR *)malloc((lstrlen(pCmd)+1)*sizeof(TCHAR));
									if (SequenceText)
										lstrcpy((TCHAR*)SequenceText,pCmd);
								}

								if (SequenceText)
								{
									command.Param.PlainText.SequenceText=SequenceText;

									if (!Info.AdvControl(Info.ModuleNumber,ACTL_KEYMACRO,&command))
									{
										;
									}

									if (Ptr)
										free((void*)Ptr);
									else
										free((void*)SequenceText);
								}
								break;
							}
						}

					}
					else if (Link) //link [/msg] [/n] источник назначение
					{
						bool NeedSymLink=false;
						DWORD LinkFlags=0;
						TCHAR *Arg2=NULL;

						while (*pCmd && *pCmd == _T('/'))
						{
							if (!LStrnicmp(pCmd,_T("/MSG"),4))
							{
								LinkFlags|=FLINK_SHOWERRMSG;
								pCmd=FarTrim(pCmd+4);
							}
							else if (!LStrnicmp(pCmd,_T("/N"),2))
							{
								LinkFlags|=FLINK_DONOTUPDATEPANEL;
								pCmd=FarTrim(pCmd+2);
							}
							else if (!LStrnicmp(pCmd,_T("/S"),2))
							{
								NeedSymLink=true;
								pCmd=FarTrim(pCmd+2);
							}
							else
							{
								break;
							}
						}

						if (*pCmd == _T('"'))
						{
							Arg2=_tcschr(pCmd+1,_T('"'));

							if (Arg2)
							{
								*++Arg2=0;
								Arg2=FarTrim(++Arg2);

								if (*Arg2 == _T('"'))
								{
									TCHAR *Arg3=_tcschr(Arg2+1,_T('"'));

									if (Arg3)
										*++Arg3=0;

									Unquote(Arg2);
								}
							}

							Unquote(pCmd);
						}
						else
						{
							Arg2=_tcschr(pCmd+1,_T(' '));

							if (Arg2)
							{
								*Arg2=0;
								Arg2=FarTrim(++Arg2);

								if (*Arg2 == _T('"'))
								{
									TCHAR *Arg3=_tcschr(Arg2+1,_T('"'));

									if (Arg3)
										*++Arg3=0;

									Unquote(Arg2);
								}
							}
						}

#ifdef UNICODE
						TCHAR temp[MAX_PATH*5];
						FSF.ConvertPath(CPM_FULL, pCmd, temp, ArraySize(temp));
						DWORD FTAttr=GetFileAttributes(temp);
#else
						DWORD FTAttr=GetFileAttributes(pCmd);
#endif

						if (FTAttr != 0xFFFFFFFF && Arg2)
						{
							TCHAR Disk[16];

							if (pCmd[1] == _T(':') && ((pCmd[2] == _T('\\') && pCmd[3] == 0) || pCmd[2] == 0))
							{
								if (pCmd[2] == 0)
								{
									lstrcpy(Disk,pCmd);
									lstrcat(Disk,_T("\\"));
									pCmd=Disk;
								}

								LinkFlags|=FLINK_VOLMOUNT;
							}
							else if (FTAttr&FILE_ATTRIBUTE_DIRECTORY)
								LinkFlags|=NeedSymLink?FLINK_SYMLINKDIR:FLINK_JUNCTION;
							else
								LinkFlags|=NeedSymLink?FLINK_SYMLINKFILE:FLINK_HARDLINK;

							MkLink(pCmd,Arg2,LinkFlags);
						}
					}

#ifdef UNICODE
					else if (Load || Unload)
					{
						TCHAR temp[MAX_PATH*5];
						Unquote(pCmd);
						ExpandEnvironmentStr(pCmd,temp,ArraySize(temp));

						if (Load)
							Info.PluginsControl(INVALID_HANDLE_VALUE,PCTL_LOADPLUGIN,PLT_PATH,(LONG_PTR)temp);
						else
							Info.PluginsControl(INVALID_HANDLE_VALUE,PCTL_UNLOADPLUGIN,PLT_PATH,(LONG_PTR)temp);
					}

#endif
					else
					{
						TCHAR *tempDir = NULL, temp[MAX_PATH*5];
						TCHAR TempFileNameOut[MAX_PATH*5], TempFileNameErr[ArraySize(TempFileNameOut)];
						TempFileNameOut[0] = TempFileNameErr[0] = 0;

						if (outputtofile)
						{
							if (*pCmd == _T('|'))
							{
								TCHAR *endTempDir = _tcschr(pCmd+1, _T('|'));

								if (endTempDir)
								{
									*endTempDir = 0;
									tempDir = pCmd+1;
									pCmd = endTempDir;
									FarLTrim(++pCmd);
								}
							}

							lstrcpy(temp,pCmd);
						}
						else
							Unquote(lstrcpy(temp,pCmd));

						TCHAR ExpTemp[ArraySize(temp)];
						ExpandEnvironmentStr(temp,ExpTemp,ArraySize(ExpTemp));
						lstrcpy(temp,ExpTemp);
						// разделение потоков
						int catchStdOutput = stream != 2;
						int catchStdError  = stream != 1;
						int catchSeparate  = (stream == 3) && (View || Edit);
						int catchAllInOne  = 0;

						if (outputtofile)
						{
							if (Run)
							{
								if (*runFile)
								{
									Unquote(runFile);
									ExpandEnvironmentStr(runFile,TempFileNameErr,ArraySize(TempFileNameErr));
									lstrcpy(TempFileNameOut,TempFileNameErr);
									allOK = TRUE;
								}
							}
							else
								allOK = MakeTempNames(TempFileNameOut, TempFileNameErr, ArraySize(TempFileNameOut));

							if (allOK)
							{
								allOK = FALSE;
								lstrcpy(cmd,_T("%COMSPEC% /c "));

								if (*temp == _T('"'))
									lstrcat(cmd, _T("\""));

								lstrcat(cmd, temp);
								ExpandEnvironmentStr(cmd, fullcmd, ArraySize(fullcmd));
								lstrcpy(cmd, temp);

								if (catchStdOutput && catchStdError)
								{
									if (catchSeparate)
									{
										if (ShowCmdOutput == 2)
											ShowCmdOutput = 1;
									}
									else
									{
										catchStdError = 0;
										catchAllInOne = 1;
									}
								}

								if (catchStdOutput)
								{
									static SECURITY_ATTRIBUTES sa;
									memset(&sa, 0, sizeof(sa));
									sa.nLength = sizeof(sa);
									sa.lpSecurityDescriptor = NULL;
									sa.bInheritHandle = TRUE;
									FileHandleOut = CreateFile(TempFileNameOut,GENERIC_WRITE,
									                           FILE_SHARE_READ,&sa,CREATE_ALWAYS,
									                           FILE_FLAG_SEQUENTIAL_SCAN,NULL);
								}
								else
								{
									FileHandleOut = INVALID_HANDLE_VALUE;
									killTemp(TempFileNameOut);
									TempFileNameOut[0] = 0;
								}

								if (catchStdError)
								{
									static SECURITY_ATTRIBUTES sa;
									memset(&sa, 0, sizeof(sa));
									sa.nLength = sizeof(sa);
									sa.lpSecurityDescriptor = NULL;
									sa.bInheritHandle = TRUE;
									FileHandleErr = CreateFile(TempFileNameErr,GENERIC_WRITE,
									                           FILE_SHARE_READ,&sa,CREATE_ALWAYS,
									                           FILE_FLAG_SEQUENTIAL_SCAN,NULL);
								}
								else
								{
									FileHandleErr = INVALID_HANDLE_VALUE;
									killTemp(TempFileNameErr);
									TempFileNameErr[0] = 0;
								}

								if ((!catchStdOutput || vh(FileHandleOut)) && (!catchStdError || vh(FileHandleErr)))
								{
									HANDLE hScreen=NULL, StdInput, StdOutput;
									CONSOLE_SCREEN_BUFFER_INFO csbi;
									DWORD ConsoleMode;
									StdInput  = GetStdHandle(STD_INPUT_HANDLE);
									StdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
									static STARTUPINFO si;
									memset(&si,0,sizeof(si));
									si.cb=sizeof(si);
									si.dwFlags=STARTF_USESTDHANDLES;
									si.hStdInput  = StdInput;
									si.hStdOutput = catchStdOutput ? FileHandleOut : StdOutput;
									si.hStdError  = catchStdError  ? FileHandleErr : StdOutput;

									if (catchAllInOne)
										si.hStdError = si.hStdOutput;

									TThreadData *td = NULL;
									hScreen = Info.SaveScreen(0, 0, -1, -1);

									if (ShowCmdOutput)
									{
										GetConsoleScreenBufferInfo(StdOutput,&csbi);
										TCHAR Blank[1024];
										_tmemset(Blank, _T(' '), csbi.dwSize.X);

										for (int Y = 0 ; Y < csbi.dwSize.Y ; Y++)
											Info.Text(0, Y, LIGHTGRAY, Blank);

										Info.Text(0, 0, 0, NULL);
										COORD C;
										C.X = 0;
										C.Y = csbi.dwCursorPosition.Y;
										SetConsoleCursorPosition(StdOutput,C);
										GetConsoleMode(StdInput,&ConsoleMode);
										SetConsoleMode(StdInput,
										               ENABLE_PROCESSED_INPUT|ENABLE_LINE_INPUT|
										               ENABLE_ECHO_INPUT|ENABLE_MOUSE_INPUT);
									}

									if (ShowCmdOutput == 2)
									{
										// данные для нитки параллельного вывода
										td = (TThreadData*)malloc(sizeof(TThreadData));

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
											{
												TShowOutputStreamData *sd = &(td->stream[enStreamErr]);
												static SECURITY_ATTRIBUTES sa;
												memset(&sa, 0, sizeof(sa));
												sa.nLength = sizeof(sa);
												sa.lpSecurityDescriptor = NULL;
												sa.bInheritHandle = TRUE;
												sd->hRead = CreateFile(TempFileNameErr,GENERIC_READ,
												                       FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,CREATE_ALWAYS,
												                       FILE_FLAG_SEQUENTIAL_SCAN,NULL);
												sd->hWrite   = FileHandleErr;
												sd->hConsole = StdOutput;
											}

											if (catchStdOutput)
											{
												TShowOutputStreamData *sd = &(td->stream[enStreamOut]);
												static SECURITY_ATTRIBUTES sa;
												memset(&sa, 0, sizeof(sa));
												sa.nLength = sizeof(sa);
												sa.lpSecurityDescriptor = NULL;
												sa.bInheritHandle = TRUE;
												sd->hRead = CreateFile(TempFileNameOut,GENERIC_READ,
												                       FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,CREATE_ALWAYS,
												                       FILE_FLAG_SEQUENTIAL_SCAN,NULL);
												sd->hWrite   = FileHandleOut;
												sd->hConsole = StdOutput;
											}
										}
									}

									TCHAR SaveDir[MAX_PATH], workDir[MAX_PATH];
									static PROCESS_INFORMATION pi;
									memset(&pi,0,sizeof(pi));

									if (tempDir)
									{
										GetCurrentDirectory(ArraySize(SaveDir),SaveDir);
										ExpandEnvironmentStr(tempDir,workDir,ArraySize(workDir));
										SetCurrentDirectory(workDir);
									}

									TCHAR consoleTitle[256];
									DWORD tlen = GetConsoleTitle(consoleTitle, 256);
									SetConsoleTitle(cmd);
									LPTSTR CurDir=NULL;
#ifdef UNICODE
									DWORD Size=FSF.GetCurrentDirectory(0,NULL);

									if (Size)
									{
										CurDir=new WCHAR[Size];
										FSF.GetCurrentDirectory(Size,CurDir);
									}

#endif
									BOOL Created=CreateProcess(NULL,fullcmd,NULL,NULL,TRUE,0,NULL,CurDir,&si,&pi);

									if (CurDir)
									{
										delete[] CurDir;
									}

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

												if (vh(sd->hWrite) && vh(sd->hRead))
													while (showPartOfOutput(sd))
														;

												closeHandle(sd->hRead);
											}

											free(td);
										}
										else
										{
											if (ShowCmdOutput == 0)
												td = (TThreadData*)malloc(sizeof(TThreadData));

											if (td)
											{
												td->type = enThreadHideOutput;
												lstrcpy(td->title, GetMsg(MConfig));
												lstrcpy(td->cmd, cmd);
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

												free(td);
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

									if (tlen)
										SetConsoleTitle(consoleTitle);

									if (tempDir)
										SetCurrentDirectory(SaveDir);

									if (ShowCmdOutput)
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
#ifndef UNICODE
										fill.Char.AsciiChar = ' ';
#else
										fill.Char.UnicodeChar = L' ';
#endif
										fill.Attributes = LIGHTGRAY;
										ScrollConsoleScreenBuffer(StdOutput, &src, NULL, dest, &fill);
#ifndef UNICODE
										Info.Control(INVALID_HANDLE_VALUE, FCTL_SETUSERSCREEN, NULL);
#else
										Info.Control(PANEL_ACTIVE, FCTL_SETUSERSCREEN,0,0);
#endif
									}

									Info.RestoreScreen(hScreen);
								}
							}
						}
						else
						{
#ifdef UNICODE
							FSF.ConvertPath(CPM_FULL, temp, TempFileNameOut, ArraySize(TempFileNameOut));
#else
							lstrcpy(TempFileNameOut, temp);
#endif
						}

						if (allOK)
						{
							if (View || Edit)
							{
								TCHAR titleOut[MAX_PATH] = _T(""), titleErr[MAX_PATH] = _T("");

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
										FarSprintf(fullcmd, _T("%s%s"), titleErr, cmd);
										Info.Viewer(TempFileNameErr,outputtofile?fullcmd:NULL,0,0,-1,-1,Flags DEF_CP_AUTODETECT);
									}
									else if (outputtofile)
										killTemp(TempFileNameErr);

									if (validForView(TempFileNameOut, Opt.ViewZeroFiles, 0))
									{
										FarSprintf(fullcmd, _T("%s%s"), titleOut, cmd);
										Info.Viewer(TempFileNameOut,outputtofile?fullcmd:NULL,0,0,-1,-1,Flags DEF_CP_AUTODETECT);
									}
									else if (outputtofile)
										killTemp(TempFileNameOut);

									outputtofile=FALSE;
								}
								else if (Edit)
								{
									DWORD Flags=EF_NONMODAL|EF_CREATENEW|EF_ENABLE_F6|EF_IMMEDIATERETURN;

									if (outputtofile)
										Flags |= EF_DISABLEHISTORY|EF_DELETEONCLOSE;

									if (validForView(TempFileNameErr, Opt.ViewZeroFiles, Opt.EditNewFiles))
									{
										FarSprintf(fullcmd, _T("%s%s"), titleErr, cmd);
										Info.Editor(TempFileNameErr,outputtofile?fullcmd:NULL,0,0,-1,-1,Flags,StartLine,StartChar DEF_CP_AUTODETECT);
									}
									else if (outputtofile)
										killTemp(TempFileNameErr);

									if (validForView(TempFileNameOut, Opt.ViewZeroFiles, Opt.EditNewFiles))
									{
										FarSprintf(fullcmd, _T("%s%s"), titleOut, cmd);
										Info.Editor(TempFileNameOut,outputtofile?fullcmd:NULL,0,0,-1,-1,Flags,StartLine,StartChar DEF_CP_AUTODETECT);
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
								TCHAR *Ptr = loadFile(TempFileNameOut, Opt.MaxDataSize/sizeof(TCHAR), outputtofile, shift, foundFile);

								if (Ptr)
								{
									CopyToClipboard(Ptr+shift);
									free(Ptr);
								}
							}
						}

						if (outputtofile)
						{
							killTemp(TempFileNameOut);
							killTemp(TempFileNameErr);
						}
					}
				} // </if(*pCmd && BracketsOk)>
			} // </if(pCmd)>
		} // </if(View||Edit||Goto)>
	} // </if(lstrlen(farcmd) > 3)>

	if (showhelp)
	{
		Info.ShowHelp(Info.ModuleName,(PrefIdx==static_cast<size_t>(-1))?_T("Contents"):Pref[PrefIdx].HelpName,0);
		return TRUE;
	}

	return FALSE;
}
#endif
