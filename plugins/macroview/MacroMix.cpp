#include <stdlib.h>

#if defined(__BORLANDC__)
  #pragma option -Od
#elif defined(_MSC_VER)
  #pragma optimize("", off)
#endif

#define MakePtr(Type, Base, Offset) ((Type)((Base) + (Offset)))

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
      pName=MakePtr(PIMAGE_IMPORT_BY_NAME,hLocalModule,pOrigThunk->u1.AddressOfData);

      if (lstrcmp((char *)&(pName->Name),cFuncName)==0)
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


int __fastcall CmpStr(const char *String1,const char *String2,int ln1,int ln2)
{
  int Result;
  char S1[MAX_PATH_LEN],S2[MAX_PATH_LEN];

  OemToChar(String1,S1);
  OemToChar(String2,S2);

  Result=CompareString(LOCALE_USER_DEFAULT,NORM_IGNORECASE|SORT_STRINGSORT,
           S1,ln1,S2,ln2);

  if (!Result)
    return -2;
  return Result-2;
}


char *__fastcall AllTrim(char *S)
{
  int i,l;

  l=lstrlen(S);
  i=0;
  while ((i<l) && (S[i]==' ')) i++;
  if ((l==0) || (i>=l))
  {
    S[0]=0;
  }
  else
  {
    while (S[l-1]==' ') l--;
    lstrcpyn(S,&S[i],l-i+1);
    S[l-i]=0;
  }
  return S;
}


char *__fastcall UnQuoteText(char *S)
{
  if ((S[0]!=0) && (S[0]=='\"'))
  {
    lstrcpy(S,&S[1]);
    int l=lstrlen(S);
    if (S[l-1]=='\"')
      S[l-1]=0;
  }
  return S;
}


char *__fastcall QuoteText(char *S,BOOL Force)
{
  BOOL isQuoteBegin=(S[0]=='\"');
  BOOL isQuoteEnd=(S[lstrlen(S)-1]=='\"');

  char *TmpStr=new char[lstrlen(S)+3];
  *TmpStr=0;

  if (!isQuoteBegin || Force)
  {
    lstrcpy(TmpStr,"\"");
    lstrcat(TmpStr,S);
  }
  else
    lstrcpy(TmpStr,S);

  if (!isQuoteEnd || Force)
    lstrcat(TmpStr,"\"");

  lstrcpy(S,TmpStr);
  delete[] TmpStr;
  return S;
}


char *__fastcall CheckFirstBackSlash(char *S,BOOL mustbe)
{
  BOOL isSlash=(S[0]=='\\');

  if (mustbe && !isSlash)
  {
    char *TmpStr=new char[lstrlen(S)+2];
    wsprintf(TmpStr,"\\%s",S);
    lstrcpy(S,TmpStr);
    delete[] TmpStr;
  }
  else if (!mustbe && isSlash)
    lstrcpy(S,&S[1]);

  return S;
}


char *__fastcall CheckRLen(char *S,unsigned ln,BOOL AddDots)
{
  char TmpStr[MAX_PATH_LEN];
  int i,j,len;

  if (ln>=3)
  {
    if ((unsigned)lstrlen(S)>ln)
    {
      len=lstrlen(S);
      for (i=len,j=ln;j>0;i--,j--)
        TmpStr[j]=S[i];
      if (AddDots)
      {
        lstrcpy(S,"...");
        lstrcat(S,&TmpStr[3]);
      }
      else
        lstrcpy(S,TmpStr);
    }
  }
  else if (AddDots)
    S[0]='.';
  return S;
}


char *__fastcall CheckLen(char *S,unsigned ln,BOOL AddDots)
{
  if (AddDots)
  {
    if (ln>=3)
    {
      if ((unsigned)lstrlen(S)>ln)
        lstrcpy(&S[ln-3],"...");
    }
    else
      S[1]=0;
  }
  else if ((unsigned)lstrlen(S)>ln)
    S[ln]=0;
  return S;
}


char *__fastcall GetMsg(int MsgId)
{
  return const_cast<char *>(Info.GetMsg(Info.ModuleNumber,MsgId));
}

void __fastcall FlushInputBuffer()
{
  FlushConsoleInputBuffer(Macro->hIn);
}
