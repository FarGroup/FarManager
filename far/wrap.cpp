wchar_t *AnsiToUnicode (const char *lpszAnsiString)
{
  int nLength = (int)strlen (lpszAnsiString)+1;

  wchar_t *lpResult = (wchar_t*)malloc (nLength*sizeof(wchar_t));

  wmemset (lpResult, 0, nLength);

  MultiByteToWideChar (
          CP_OEMCP,
          0,
          lpszAnsiString,
          -1,
          lpResult,
          nLength
          );

  return lpResult;
}

char *WINAPI FarItoaA(int value, char *string, int radix)
{
  if(string)
    return itoa(value,string,radix);
  return NULL;
}

char *WINAPI FarItoa64A(__int64 value, char *string, int radix)
{
  if(string)
    return _i64toa(value, string, radix);
  return NULL;
}

int WINAPI FarAtoiA(const char *s)
{
  if(s)
    return atoi(s);
  return 0;
}

__int64 WINAPI FarAtoi64A(const char *s)
{
  if(s)
    return _atoi64(s);
  return _i64(0);
}

char* WINAPI PointToNameA(char *Path)
{
  if(!Path)
    return NULL;

  char *NamePtr=Path;
  while (*Path)
  {
    if (*Path=='\\' || *Path=='/' || *Path==':' && Path==NamePtr+1)
      NamePtr=Path+1;
    Path++;
  }
  return(NamePtr);
}

void WINAPI UnquoteA(char *Str)
{
  if (!Str)
    return;
  char *Dst=Str;
  while (*Str)
  {
    if (*Str!='\"')
      *Dst++=*Str;
    Str++;
  }
  *Dst=0;
}

char* WINAPI RemoveLeadingSpacesA(char *Str)
{
  char *ChPtr;
  if((ChPtr=Str) == 0)
    return NULL;

  for (; IsSpaceA(*ChPtr); ChPtr++)
         ;
  if (ChPtr!=Str)
    memmove(Str,ChPtr,strlen(ChPtr)+1);
  return Str;
}

/*
char* WINAPI RemoveTrailingSpacesA(char *Str)
{
  if(!Str)
    return NULL;
  if (*Str == '\0')
    return Str;

  char *ChPtr;
  int I;

  for (ChPtr=Str+(I=(int)strlen((char *)Str)-1); I >= 0; I--, ChPtr--)
    if (IsSpaceA(*ChPtr) || IsEolA(*ChPtr))
      *ChPtr=0;
    else
      break;

  return Str;
}
*/

char* WINAPI RemoveExternalSpacesA(char *Str)
{
  return RemoveTrailingSpacesA(RemoveLeadingSpacesA(Str));
}

char* WINAPI TruncStrA(char *Str,int MaxLength)
{
  if(Str)
  {
    int Length;
    if (MaxLength<0)
      MaxLength=0;
    if ((Length=(int)strlen(Str))>MaxLength)
    {
      if (MaxLength>3)
      {
        char *MovePos = Str+Length-MaxLength+3;
        memmove(Str+3,MovePos,strlen(MovePos)+1);
        memcpy(Str,"...",3);
      }
      Str[MaxLength]=0;
    }
  }
  return(Str);
}

char* WINAPI TruncPathStrA(char *Str, int MaxLength)
{
  if (Str)
  {
    int nLength = (int)strlen (Str);

    if (nLength > MaxLength)
    {
      char *lpStart = NULL;

      if ( *Str && (Str[1] == ':') && (Str[2] == '\\') )
         lpStart = Str+3;
      else
      {
        if ( (Str[0] == '\\') && (Str[1] == '\\') )
        {
          if ( (lpStart = strchr (Str+2, '\\')) != NULL )
            if ( (lpStart = strchr (lpStart+1, '\\')) != NULL )
              lpStart++;
        }
      }

      if ( !lpStart || (lpStart-Str > MaxLength-5) )
        return TruncStrA (Str, MaxLength);

      char *lpInPos = lpStart+3+(nLength-MaxLength);
      memmove (lpStart+3, lpInPos, strlen (lpInPos)+1);
      memcpy (lpStart, "...", 3);
    }
  }

  return Str;
}

char *InsertQuoteA(char *Str)
{
  size_t l = strlen(Str);
  if(*Str != '"')
  {
    memmove(Str+1,Str,++l);
    *Str='"';
  }
  if(Str[l-1] != '"')
  {
    Str[l++] = '\"';
    Str[l] = 0;
  }
  return Str;
}

char* WINAPI QuoteSpaceOnlyA(char *Str)
{
  if (Str && strchr(Str,' ')!=NULL)
    InsertQuoteA(Str);
  return(Str);
}

BOOL AddEndSlashA(char *Path,char TypeSlash)
{
  BOOL Result=FALSE;
  if(Path)
  {
    /* $ 06.12.2000 IS
      ! Теперь функция работает с обоими видами слешей, также происходит
        изменение уже существующего конечного слеша на такой, который
        встречается чаще.
    */
    char *end;
    int Slash=0, BackSlash=0;
    if(!TypeSlash)
    {
      end=Path;
      while(*end)
      {
       Slash+=(*end=='\\');
       BackSlash+=(*end=='/');
       end++;
      }
    }
    else
    {
      end=Path+strlen(Path);
      if(TypeSlash == '\\')
        Slash=1;
      else
        BackSlash=1;
    }
    int Length=(int)(end-Path);
    char c=(Slash<BackSlash)?'/':'\\';
    Result=TRUE;
    if (Length==0)
    {
       *end=c;
       end[1]=0;
    }
    else
    {
     end--;
     if (*end!='\\' && *end!='/')
     {
       end[1]=c;
       end[2]=0;
     }
     else
       *end=c;
    }
    /* IS $ */
  }
  return Result;
}

BOOL WINAPI AddEndSlashA(char *Path)
{
  return AddEndSlashA(Path,0);
}

void WINAPI GetPathRootA(const char *Path, char *Root)
{
	string strPath(Path), strRoot;

	GetPathRoot(strPath,strRoot);

	strRoot.GetCharString(Root,strRoot.GetLength());
}

int WINAPI CopyToClipboardA(const char *Data)
{
	wchar_t *p = Data!=NULL?AnsiToUnicode(Data):NULL;
	int ret = CopyToClipboard(p);
	if (p) free(p);
	return ret;
}

char* WINAPI PasteFromClipboardA(void)
{
	wchar_t *p = PasteFromClipboard();
	if (p)
		return UnicodeToAnsi(p);
	return NULL;
}

void WINAPI DeleteBufferA(char *Buffer)
{
	if(Buffer) free(Buffer);
}

int WINAPI ProcessNameA(const char *Param1,char *Param2,DWORD Flags)
{
	string strP1(Param1), strP2(Param2);
	int size = (int)(strP1.GetLength()+strP2.GetLength()+NM)+1; //а хрен ещё как угадать скока там этот Param2
	wchar_t *p=(wchar_t *)malloc(size*sizeof(wchar_t));
	wcscpy(p,strP2);
	int ret = ProcessName(strP1,p,size,Flags);
	UnicodeToAnsi(p,Param2);
	return ret;
}

int WINAPI KeyNameToKeyA(const char *Name)
{
	string strN(Name);
	return KeyNameToKey(strN);
}

BOOL WINAPI FarKeyToNameA(int Key,char *KeyText,int Size)
{
	string strKT;
	int ret=KeyToText(Key,strKT);
	if (ret)
		strKT.GetCharString(KeyText,Size>0?Size:32);
	return ret;
}

char* WINAPI FarMkTempA(char *Dest, const char *Prefix)
{
	string strP((Prefix?Prefix:""));
	wchar_t D[NM] = {0};

	FarMkTemp(D,sizeof(D),strP);

	UnicodeToAnsi(D,Dest);
	return Dest;
}

DWORD WINAPI ExpandEnvironmentStrA(const char *src, char *dest, size_t size)
{
	string strS(src), strD;

	apiExpandEnvironmentStrings(strS,strD);
	DWORD len = (DWORD)min(strD.GetLength(),size-1);

	strD.GetCharString(dest,len+1);
	dest[len]=0;
	return len;
}

int WINAPI FarViewerA(const char *FileName,const char *Title,int X1,int Y1,int X2,int Y2,DWORD Flags)
{
	string strFN(FileName), strT(Title);
	return FarViewer(strFN,strT,X1,Y1,X2,Y2,Flags);
}

int WINAPI FarEditorA(const char *FileName,const char *Title,int X1,int Y1,int X2,int Y2,DWORD Flags,int StartLine,int StartChar)
{
	string strFN(FileName), strT(Title);
	return FarEditor(strFN,strT,X1,Y1,X2,Y2,Flags,StartLine,StartChar);
}

int WINAPI FarCmpNameA(const char *pattern,const char *str,int skippath)
{
	string strP(pattern), strS(str);
  return FarCmpName(strP,strS,skippath);
}

void WINAPI FarTextA(int X,int Y,int Color,const char *Str)
{
	if (!Str)	return FarTextA(X,Y,Color,NULL);
	string strS(Str);
	return FarText(X,Y,Color,strS);
}

BOOL WINAPI FarShowHelpA(const char *ModuleName,const char *HelpTopic,DWORD Flags)
{
	string strMN((ModuleName?ModuleName:"")), strHT((HelpTopic?HelpTopic:""));
	return FarShowHelp(strMN,(HelpTopic?(const wchar_t *)strHT:NULL),Flags);
}

int WINAPI FarInputBoxA(const char *Title,const char *Prompt,const char *HistoryName,const char *SrcText,char *DestText,int DestLength,const char *HelpTopic,DWORD Flags)
{
	string strT((Title?Title:"")), strP((Prompt?Prompt:"")), strHN((HistoryName?HistoryName:"")), strST((SrcText?SrcText:"")), strD, strHT((HelpTopic?HelpTopic:""));
	wchar_t *D = strD.GetBuffer(DestLength);
	int ret = FarInputBox((Title?(const wchar_t *)strT:NULL),(Prompt?(const wchar_t *)strP:NULL),(HistoryName?(const wchar_t *)strHN:NULL),(SrcText?(const wchar_t *)strST:NULL),D,DestLength,(HelpTopic?(const wchar_t *)strHT:NULL),Flags);
	strD.ReleaseBuffer();
	if (DestText)
		strD.GetCharString(DestText,DestLength);
	return ret;
}

int WINAPI FarMessageFnA(INT_PTR PluginNumber,DWORD Flags,const char *HelpTopic,const char * const *Items,int ItemsNumber,int ButtonsNumber)
{
	string strHT((HelpTopic?HelpTopic:""));
	wchar_t **p;
	int c=0;

	if (Flags&FMSG_ALLINONE)
	{
		p = (wchar_t **)AnsiToUnicode((const char *)Items);
	}
	else
	{
		c = ItemsNumber;
		p = (wchar_t **)malloc(c*sizeof(wchar_t*));
		for (int i=0; i<c; i++)
			p[i] = AnsiToUnicode(Items[i]);
	}

	int ret = FarMessageFn(PluginNumber,Flags,(HelpTopic?(const wchar_t *)strHT:NULL),p,ItemsNumber,ButtonsNumber);

	for (int i=0; i<c; i++)
		free(p[i]);
	free(p);

	return ret;
}

const char * WINAPI FarGetMsgFnA(INT_PTR PluginHandle,int MsgId)
{
	static char s[] = "abc";
	return s;
}

int WINAPI FarMenuFnA(INT_PTR PluginNumber,int X,int Y,int MaxHeight,DWORD Flags,const char *Title,const char *Bottom,const char *HelpTopic,const int *BreakKeys,int *BreakCode,const struct oldfar::FarMenuItem *Item,int ItemsNumber)
{
	return -1;
}

int WINAPI FarDialogFnA(INT_PTR PluginNumber,int X1,int Y1,int X2,int Y2,const char *HelpTopic,struct oldfar::FarDialogItem *Item,int ItemsNumber)
{
	return -1;
}

int WINAPI FarControlA(HANDLE hPlugin,int Command,void *Param)
{
	return FALSE;
}

int WINAPI FarGetDirListA(const char *Dir,struct oldfar::PluginPanelItem **pPanelItem,int *pItemsNumber)
{
	return FALSE;
}

int WINAPI FarGetPluginDirListA(INT_PTR PluginNumber,HANDLE hPlugin,const char *Dir,struct oldfar::PluginPanelItem **pPanelItem,int *pItemsNumber)
{
	return FALSE;
}

void WINAPI FarFreeDirListA(const struct oldfar::PluginPanelItem *PanelItem)
{
}

INT_PTR WINAPI FarAdvControlA(INT_PTR ModuleNumber,int Command,void *Param)
{
	return 0;
}

int WINAPI FarDialogExA(INT_PTR PluginNumber,int X1,int Y1,int X2,int Y2,const char *HelpTopic,struct oldfar::FarDialogItem *Item,int ItemsNumber,DWORD Reserved,DWORD Flags,oldfar::FARWINDOWPROC DlgProc,LONG_PTR Param)
{
	return -1;
}

int WINAPI FarEditorControlA(int Command,void* Param)
{
	return FALSE;
}

int WINAPI FarViewerControlA(int Command,void* Param)
{
	return TRUE;
}
