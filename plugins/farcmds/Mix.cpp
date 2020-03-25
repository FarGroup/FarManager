#include <cwchar>
#include <shlobj.h>
#include <plugin.hpp>

#include "FARCmds.hpp"
#include "Lang.hpp"
#include <initguid.h>
#include "guid.hpp"


const wchar_t *GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid,MsgId);
}

// need "delete[]"
wchar_t *ExpandEnv(const wchar_t* Src, DWORD* Length)
{
	DWORD sizeExp=ExpandEnvironmentStrings(Src,NULL,0);
	wchar_t *temp=new wchar_t[sizeExp+1];
	if (temp)
		ExpandEnvironmentStrings(Src,temp,sizeExp);
	else
		sizeExp=0;

	if (Length)
		*Length=sizeExp;

	return temp;
}

/*
	возвращает число, вырезав его из строки, или -2 в случае ошибки
	Start, End - начало и конец строки
*/
int GetInt(const wchar_t *Start, wchar_t *End)
{
	int Ret=-2;

	if (End >= Start)
	{
		int Size=(int)(End-Start);

		if (Size > 0)
		{
			wchar_t *Tmp=new wchar_t[Size+1];
			if (Tmp)
			{
				wmemcpy(Tmp,Start,Size);
				Tmp[Size]=0;

				if (wcschr(Tmp,L'%')) // Env
				{
					wchar_t *Tmp0=ExpandEnv(Tmp,nullptr);
					delete[] Tmp;
					Tmp=Tmp0;
				}
				Ret=FSF.atoi(Tmp);
				delete[] Tmp;
			}
		}
		else
			Ret=0;
	}

	return Ret;
}

/*
	Заменить в строке Str Count вхождений подстроки FindStr на подстроку ReplStr
	Если Count < 0 - заменять "до полной победы"
	Return - количество замен
*/
int ReplaceStrings(wchar_t *Str,const wchar_t *FindStr,const wchar_t *ReplStr,int Count,BOOL IgnoreCase)
{
	int I=0, J=0, Res;
	int LenReplStr=(int)lstrlen(ReplStr);
	int LenFindStr=(int)lstrlen(FindStr);
	int L=(int)lstrlen(Str);

	while (I <= L-LenFindStr)
	{
		Res=IgnoreCase?_memicmp(Str+I, FindStr, LenFindStr*sizeof(wchar_t)):memcmp(Str+I, FindStr, LenFindStr*sizeof(wchar_t));

		if (Res == 0)
		{
			if (LenReplStr > LenFindStr)
				wmemmove(Str+I+(LenReplStr-LenFindStr),Str+I,lstrlen(Str+I)+1); // >>
			else if (LenReplStr < LenFindStr)
				wmemmove(Str+I,Str+I+(LenFindStr-LenReplStr),lstrlen(Str+I+(LenFindStr-LenReplStr))+1); //??

			wmemcpy(Str+I,ReplStr,LenReplStr);
			I += LenReplStr;

			if (++J == Count && Count > 0)
				break;
		}
		else
			I++;

		L=(int)lstrlen(Str);
	}

	return J;
}


/*
	возвращает PipeFound
	NewCmdStr и NewCmdPar после использования удалить
*/
int PartCmdLine(const wchar_t *CmdStr,wchar_t **NewCmdStr,wchar_t **NewCmdPar)
{
	int PipeFound = FALSE;

	if (NewCmdStr)
		*NewCmdStr=0;
	if (NewCmdPar)
		*NewCmdPar=0;

	wchar_t *Temp=ExpandEnv(CmdStr,nullptr);

	if (Temp)
	{
		FSF.Trim(Temp);
		wchar_t *CmdPtr = Temp;
		wchar_t *ParPtr = NULL;
		int QuoteFound = FALSE;

		// Разделим собственно команду для исполнения и параметры.
		// При этом заодно определим наличие символов переопределения потоков
		// Работаем с учетом кавычек. Т.е. пайп в кавычках - не пайп.

		while (*CmdPtr)
		{
			if (*CmdPtr == L'"')
				QuoteFound = !QuoteFound;

			if (!QuoteFound && CmdPtr != Temp)
			{
				if (*CmdPtr == L'>' || *CmdPtr == L'<' ||
				        *CmdPtr == L'|' || *CmdPtr == L' ' ||
				        *CmdPtr == L'/' ||      // вариант "far.exe/?"
				        *CmdPtr == L'&'
				   )
				{
					if (!ParPtr)
						ParPtr = CmdPtr;

					if (*CmdPtr != L' ' && *CmdPtr != L'/')
						PipeFound = TRUE;
				}
			}

			if (ParPtr && PipeFound) // Нам больше ничего не надо узнавать
				break;

			CmdPtr++;
		}

		if (NewCmdPar && ParPtr) // Мы нашли параметры и отделяем мух от котлет
		{
			wchar_t *ptrNewCmdPar=new wchar_t[lstrlen(ParPtr)+1];
			if (ptrNewCmdPar)
				lstrcpy(ptrNewCmdPar, ParPtr);
			*NewCmdPar=ptrNewCmdPar;
			*ParPtr=0;
		}

		if (NewCmdStr)
		{
			wchar_t *ptrNewCmdStr=new wchar_t[lstrlen(Temp)+1];
			if (ptrNewCmdStr)
			{
				lstrcpy(ptrNewCmdStr, Temp);
				FSF.Unquote(ptrNewCmdStr);
			}
			*NewCmdStr=ptrNewCmdStr;
		}

		delete[] Temp;
	}

	return PipeFound;
}

static wchar_t *GetAlias(const wchar_t *ModuleName, const wchar_t *FindAlias)
{
	wchar_t *FoundAlias=nullptr;

	int ret=GetConsoleAliasesLength((LPWSTR)ModuleName);
	if (ret)
	{
		wchar_t *AllAliases=new wchar_t[ret];
		if (AllAliases)
		{
			ret=GetConsoleAliases(AllAliases, ret, (LPWSTR)ModuleName);
			if (ret)
			{
				wchar_t *ptr=AllAliases;
				while(*ptr)
				{
					wchar_t *p=wcschr(ptr,L'=');
					if (p)
					{
						*p=0;
						if (!FSF.LStricmp(ptr,FindAlias))
						{
							FoundAlias=new wchar_t[lstrlen(p+1)+1];
							if (FoundAlias)
								lstrcpy(FoundAlias,p+1);
							break;
						}
						*p=L'=';
					}
					ptr+=lstrlen(ptr)+1;
				}
			}
			delete[] AllAliases;
		}
	}

	return FoundAlias;
}

wchar_t* ProcessOSAliases(const wchar_t *Str)
{
	wchar_t *pNewCmdStr=nullptr;
	wchar_t *pNewCmdPar=nullptr;

	PartCmdLine(Str,&pNewCmdStr,&pNewCmdPar);

	if (!pNewCmdStr)
	{
		if (!pNewCmdPar)
			return nullptr;
	}


	wchar_t *ptrAlias=nullptr;

	if (pNewCmdStr)
	{
		// <GetModuleFileName>
		wchar_t *ModuleName=nullptr;

		{
			DWORD SizeModuleName = 0;
			DWORD BufferSize = MAX_PATH;

			do {
				BufferSize *= 2;
				wchar_t *ModuleNameTemp=(wchar_t*)realloc(ModuleName,BufferSize);
				if (ModuleNameTemp)
				{
					ModuleName=ModuleNameTemp;
					SizeModuleName = GetModuleFileName(NULL, ModuleName, BufferSize);
				}
			} while ((SizeModuleName >= BufferSize) || (!SizeModuleName && GetLastError() == ERROR_INSUFFICIENT_BUFFER));
		}

		// find alias for Far.exe
		if (ModuleName)
		{
			ptrAlias=GetAlias(FSF.PointToName(ModuleName), pNewCmdStr);
			if (ModuleName)
				free(ModuleName);
		}

		if (!ptrAlias)
		{
			// find alias for cmd.exe
			wchar_t *CSModuleName=ExpandEnv(L"%COMSPEC%",nullptr);
			if (CSModuleName)
			{
				ptrAlias=GetAlias(FSF.PointToName(CSModuleName), pNewCmdStr);
				delete[] CSModuleName;
			}
		}
	}

	if (!ptrAlias)
	{
		if (pNewCmdStr)
			delete[] pNewCmdStr;

		if (pNewCmdPar)
			delete[] pNewCmdPar;
		return nullptr;
	}
	else
	{
		if (pNewCmdStr)
			delete[] pNewCmdStr;
	}

	wchar_t *ptrCmdStr=ptrAlias;

	// count "$*"
	size_t countP=1;
	while (*ptrCmdStr)
	{
		wchar_t *p=wcsstr(ptrCmdStr,L"$*");
		if (p)
		{
			countP++;
			ptrCmdStr=p+2;
		}
		else
			break;
	}

	// alloc memory
	wchar_t *tempCmdStr=new wchar_t[lstrlen(ptrAlias)+countP*(pNewCmdPar?lstrlen(pNewCmdPar):0)+1];
	if (tempCmdStr)
	{
		lstrcpy(tempCmdStr,ptrAlias);
		// replace
		if (!ReplaceStrings(tempCmdStr,L"$*",pNewCmdPar?pNewCmdPar:L"",-1,FALSE))
		{
			//... or merge
			if (pNewCmdPar)
			{
				lstrcat(tempCmdStr,L" ");
				lstrcat(tempCmdStr,pNewCmdPar);
			}
		}
		ptrCmdStr=tempCmdStr;
		delete[] ptrAlias;
	}
	else
	{
		delete[] ptrAlias;
		return nullptr;
	}

	if (pNewCmdPar)
		delete[] pNewCmdPar;

	return ptrCmdStr;
}


wchar_t *GetShellLinkPath(const wchar_t *LinkFile)
{
	bool Result=false;
	wchar_t *Path=nullptr;

	wchar_t *Temp=ExpandEnv(LinkFile,nullptr);
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

	if (!(*FileName && FileExists(FileName)))
	{
		delete[] FileName;
		return nullptr;
	}

	// <Check lnk-header>
	HANDLE hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );

	if (hFile != INVALID_HANDLE_VALUE)
	{
		struct ShellLinkHeader
		{
			DWORD    HeaderSize;
			BYTE     LinkCLSID[16];
			DWORD    LinkFlags;
			DWORD    FileAttributes;
			FILETIME CreationTime;
			FILETIME AccessTime;
			FILETIME WriteTime;
			DWORD    FileSize;
			DWORD    IconIndex;
			DWORD    ShowCommand;
			WORD     HotKey;
			WORD     Reserved1;
			DWORD    Reserved2;
			DWORD    Reserved3;
		};

		ShellLinkHeader slh = { 0 };
		DWORD read = 0;
		ReadFile( hFile, &slh, sizeof( ShellLinkHeader ), &read, NULL );

		if ( read == sizeof( ShellLinkHeader ) && slh.HeaderSize == 0x0000004C)
		{
			if (!memcmp( slh.LinkCLSID, "\x01\x14\x02\x00\x00\x00\x00\x00\xC0\x00\x00\x00\x00\x00\x00\x46\x9b", 16 ))
				Result=true;
		}

		CloseHandle( hFile );
	}
	// </Check lnk-header>

	if (Result)
	{
		// <get target>
		Result=false;
		const auto CoInited = SUCCEEDED(CoInitialize(nullptr));

		IShellLink* psl = NULL;
		HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
		if (SUCCEEDED(hres))
		{
			IPersistFile* ppf = NULL;
			hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
			if (SUCCEEDED(hres))
			{
				hres = ppf->Load(FileName, STGM_READ);
				if (SUCCEEDED(hres))
				{
					hres = psl->Resolve(NULL, 0);
					if (SUCCEEDED(hres))
					{
						wchar_t TargPath[MAX_PATH] = {0};
						hres = psl->GetPath(TargPath, ARRAYSIZE(TargPath), NULL, SLGP_RAWPATH);
						if (SUCCEEDED(hres))
						{
							Path=new wchar_t[lstrlen(TargPath)+1];
							if (Path)
								lstrcpy(Path, TargPath);
						}
					}
				}
				ppf->Release();
			}
			psl->Release();
		}

		if (CoInited)
			CoUninitialize();
		// </get target>
	}

	delete[] FileName;

	return Path;
}

bool StrToGuid(const wchar_t *Value,GUID *Guid)
{
	return UuidFromString(reinterpret_cast<unsigned short*>((void*)Value), Guid) == RPC_S_OK;
}

bool IsTextUTF8(const char* Buffer,size_t Length)
{
	bool Ascii=true;
	size_t Octets=0;
	size_t LastOctetsPos = 0;
	const size_t MaxCharSize = 4;

	for (size_t i=0; i<Length; i++)
	{
		BYTE c=Buffer[i];

		if (c&0x80)
			Ascii=false;

		if (Octets)
		{
			if ((c&0xC0)!=0x80)
				return false;

			Octets--;
		}
		else
		{
			LastOctetsPos = i;

			if (c&0x80)
			{
				while (c&0x80)
				{
					c <<= 1;
					Octets++;
				}

				Octets--;

				if (!Octets)
					return false;
			}
		}
	}

	return (!Octets || Length - LastOctetsPos < MaxCharSize) && !Ascii;
}

UINT GetCPBuffer(const void* data, size_t size, size_t* off)
{
	#define SIGN_UNICODE    0xFEFF
	#define SIGN_REVERSEBOM 0xFFFE
	#define SIGN_UTF8_LO    0xBBEF
	#define SIGN_UTF8_HI    0xBF

	UINT cp=(UINT)-1;
	size_t Pos = 0;
	wchar_t* Ptr = (wchar_t *)data;
	size_t PtrSize = size;

	if (Ptr)
	{
		if (Ptr[0]==SIGN_UNICODE)
		{
			Pos += 2;
			cp=CP_UNICODE;
		}
		else if (Ptr[0]==SIGN_REVERSEBOM)
		{
			Pos += 2;
			cp=CP_REVERSEBOM;
		}
		else if (Ptr[0]==SIGN_UTF8_LO&&(Ptr[1]&0xff)==SIGN_UTF8_HI)
		{
			Pos += 3;
			cp=CP_UTF8;
		}
		else
		{
			if (IsTextUTF8((char*)Ptr,PtrSize))
			{
				cp=CP_UTF8;
			}
			else
			{
				int test = IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK | IS_TEXT_UNICODE_NOT_UNICODE_MASK | IS_TEXT_UNICODE_NOT_ASCII_MASK;
				IsTextUnicode(Ptr, static_cast<int>(PtrSize), &test); // return value is ignored, it's ok.
				if (!(test & IS_TEXT_UNICODE_NOT_UNICODE_MASK) || (test & IS_TEXT_UNICODE_ODD_LENGTH)) // ignore odd
				{
					if (test & IS_TEXT_UNICODE_UNICODE_MASK)
					{
						cp=CP_UNICODE;
					}
					else if (test & IS_TEXT_UNICODE_REVERSE_MASK)
					{
						cp=CP_REVERSEBOM;
					}
					else if (test & IS_TEXT_UNICODE_STATISTICS) // !!! допускаем возможность, что это Unicode
					{
						cp=CP_UNICODE;
					}
				}
			}
		}
	}

	if (off)
		*off=Pos;

	return cp;
}

wchar_t *ConvertBuffer(wchar_t* Ptr,size_t PtrSize,BOOL outputtofile, size_t& shift,bool *unicode)
{

	if (Ptr)
	{
		size_t off=0;
		UINT cp=GetCPBuffer(Ptr,PtrSize,&off);

		if (cp == (UINT)-1)
			cp=outputtofile?GetConsoleOutputCP():GetACP();

		switch (cp)
		{
			case CP_UNICODE:
			{
				shift=off/2;
				if (unicode)
					*unicode=true;
				break;
			}

			case CP_REVERSEBOM:
			{
				shift=off/2;
				size_t PtrLength=lstrlen(Ptr);
				swab((char*)Ptr,(char*)Ptr,int(PtrLength*sizeof(wchar_t)));
				if (unicode)
					*unicode=true;
				break;
			}

			//case CP_UTF8:
			default:
			{
				size_t PtrLength=MultiByteToWideChar(cp,0,(char*)Ptr+off,-1,NULL,0);

				if (PtrLength)
				{
					wchar_t* NewPtr=new wchar_t[PtrLength+1];
					if (NewPtr)
					{
						if (MultiByteToWideChar(cp,0,(char*)Ptr+off,-1,NewPtr,(int)PtrLength))
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
				break;
			}

		}
	}
	return Ptr;
}
