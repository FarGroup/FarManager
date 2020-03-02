#include <stdlib.h>

#if defined(__BORLANDC__)
#pragma option -Od
#elif defined(_MSC_VER)
#pragma optimize("", off)
#endif

#define MakePtr(Type, Base, Offset) ((Type)((DWORD_PTR)(Base)+(DWORD_PTR)(Offset)))

BOOL InterceptDllCall(HMODULE hLocalModule,const char* cDllName,const char* cFuncName,
                      PVOID pApiNew,PVOID* pApiOrig)
{
	PIMAGE_DOS_HEADER pDOSHeader=(PIMAGE_DOS_HEADER)hLocalModule;
	PIMAGE_NT_HEADERS pNTHeader;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
	DWORD dwProtect;
	BOOL bSuccess=FALSE;

	if (IsBadReadPtr(hLocalModule,sizeof(PIMAGE_NT_HEADERS)))
		return FALSE;

	if (pDOSHeader->e_magic!=IMAGE_DOS_SIGNATURE)
		return FALSE;

	pNTHeader=MakePtr(PIMAGE_NT_HEADERS,pDOSHeader,pDOSHeader->e_lfanew);

	if (pNTHeader->Signature!=IMAGE_NT_SIGNATURE)
		return FALSE;

	pImportDesc=MakePtr(PIMAGE_IMPORT_DESCRIPTOR,hLocalModule,
	                    pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT
	                                                           ] /*pNTHeader->OptionalHeader.DataDirectory*/.VirtualAddress) /*MakePtr*/;

	if (pImportDesc==(PIMAGE_IMPORT_DESCRIPTOR)pNTHeader)
		return FALSE;

	while (pImportDesc->Name)
	{
		PIMAGE_THUNK_DATA pOrigThunk;
		PIMAGE_THUNK_DATA pRealThunk;
		PIMAGE_IMPORT_BY_NAME pName;
		pOrigThunk=MakePtr(PIMAGE_THUNK_DATA,hLocalModule,pImportDesc->OriginalFirstThunk) /*MakePtr*/;
		pRealThunk=MakePtr(PIMAGE_THUNK_DATA,hLocalModule,pImportDesc->FirstThunk) /*MakePtr*/;

		while (pOrigThunk->u1.Function)
		{
			if ((pOrigThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) != IMAGE_ORDINAL_FLAG)
			{
				pName=MakePtr(PIMAGE_IMPORT_BY_NAME,hLocalModule,pOrigThunk->u1.AddressOfData);

				if (lstrcmpA((char *)&(pName->Name),cFuncName)==0)
				{
					if (!IsBadWritePtr((LPVOID)(&pRealThunk->u1.Function),sizeof(DWORD_PTR)) /*!IsBadWritePtr*/)
					{
						if (pApiOrig)
							*pApiOrig=PVOID(pRealThunk->u1.Function);

						pRealThunk->u1.Function=(DWORD_PTR)pApiNew;
						bSuccess=TRUE;
						break;
					}
					else
					{
						if (VirtualProtect((LPVOID)(&pRealThunk->u1.Function),sizeof(DWORD_PTR),
						                   PAGE_EXECUTE_READWRITE,&dwProtect) /*VirtualProtect*/)
						{
							DWORD dwNewProtect;

							if (pApiOrig)
								*pApiOrig=PVOID(pRealThunk->u1.Function);

							pRealThunk->u1.Function=(DWORD_PTR)pApiNew;
							bSuccess=TRUE;
							dwNewProtect=dwProtect;
							VirtualProtect((LPVOID)(&pRealThunk->u1.Function),sizeof(DWORD_PTR),
							               dwNewProtect,&dwProtect) /*VirtualProtect*/;
							break;
						} /*if*/
					} /*iff*/
				} /*if*/
			}

			pOrigThunk++;
			pRealThunk++;
		} /*while*/

		if (bSuccess)
			break;

		pImportDesc++;
	} /*while*/

	return bSuccess;
} /*InterceptDllCall(HMODULE,const char*,const char*,PVOID,PVOID*,PVOID)*/


#if defined(__BORLANDC__)
#pragma option -O2
#elif defined(_MSC_VER)
#pragma optimize("", on)
#endif


int __fastcall CmpStr(const TCHAR *String1,const TCHAR *String2,int ln1,int ln2)
{
	int Result;
#ifndef UNICODE
	char S1[MAX_PATH_LEN],S2[MAX_PATH_LEN];
	OemToChar(String1,S1);
	OemToChar(String2,S2);
#else
#define S1  String1
#define S2  String2
#endif
	Result=CompareString(LOCALE_USER_DEFAULT,NORM_IGNORECASE|SORT_STRINGSORT,
	                     S1,ln1,S2,ln2);
#ifdef UNICODE
#undef S1
#undef S2
#endif

	if (!Result)
		return -2;

	return Result-2;
}


TCHAR *__fastcall AllTrim(TCHAR *S)
{
	unsigned i,l;
	l=lstrlen(S);
	i=0;

	while ((i<l) && (S[i]==_T(' '))) i++;

	if ((l==0) || (i>=l))
	{
		S[0]=0;
	}
	else
	{
		while (S[l-1]==_T(' ')) l--;

		lstrcpyn(S,&S[i],l-i+1);
		S[l-i]=0;
	}

	return S;
}


TCHAR *__fastcall UnQuoteText(TCHAR *S)
{
	if (S[0]==_T('\"'))
	{
		lstrcpy(S,&S[1]);
		int l=lstrlen(S);

		if (S[l-1]==_T('\"'))
			S[l-1]=0;
	}

	return S;
}


TCHAR *__fastcall QuoteText(TCHAR *S,BOOL Force)
{
	BOOL isQuoteBegin=(S[0]==_T('\"'));
	BOOL isQuoteEnd=(S[lstrlen(S)-1]==_T('\"'));
	TCHAR *TmpStr=new TCHAR[lstrlen(S)+3];
	*TmpStr=0;

	if (!isQuoteBegin || Force)
	{
		lstrcpy(TmpStr,_T("\""));
		lstrcat(TmpStr,S);
	}
	else
		lstrcpy(TmpStr,S);

	if (!isQuoteEnd || Force)
		lstrcat(TmpStr,_T("\""));

	lstrcpy(S,TmpStr);
	delete[] TmpStr;
	return S;
}


TCHAR *__fastcall CheckFirstBackSlash(TCHAR *S,BOOL mustbe)
{
	BOOL isSlash=(S[0]==_T('\\'));

	if (mustbe && !isSlash)
	{
		TCHAR *TmpStr=new TCHAR[lstrlen(S)+2];
		wsprintf(TmpStr,_T("\\%s"),S);
		lstrcpy(S,TmpStr);
		delete[] TmpStr;
	}
	else if (!mustbe && isSlash)
		lstrcpy(S,&S[1]);

	return S;
}


TCHAR *__fastcall CheckRLen(TCHAR *S,unsigned ln,BOOL AddDots)
{
	TCHAR TmpStr[MAX_PATH_LEN];
	int i,j,len;

	if (ln>=3)
	{
		if ((unsigned)lstrlen(S)>ln)
		{
			len=lstrlen(S);

			for (i=len,j=ln; j>0; i--,j--)
				TmpStr[j]=S[i];

			if (AddDots)
			{
				lstrcpy(S,_T("..."));
				lstrcat(S,&TmpStr[3]);
			}
			else
				lstrcpy(S,TmpStr);
		}
	}
	else if (AddDots)
		S[0]=_T('.');

	return S;
}


TCHAR *__fastcall CheckLen(TCHAR *S,unsigned ln,BOOL AddDots)
{
	if (AddDots)
	{
		if (ln>=3)
		{
			if ((unsigned)lstrlen(S)>ln)
				lstrcpy(&S[ln-3],_T("..."));
		}
		else
			S[1]=0;
	}
	else if ((unsigned)lstrlen(S)>ln)
		S[ln]=0;

	return S;
}


TCHAR *__fastcall GetMsg(int MsgId)
{
	return const_cast<TCHAR *>(Info.GetMsg(Info.ModuleNumber,MsgId));
}

void __fastcall FlushInputBuffer()
{
	FlushConsoleInputBuffer(Macro->hIn);
}
