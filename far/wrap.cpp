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

char *UnicodeToAnsiBin (const wchar_t *lpwszUnicodeString, int nLength)
{
  char *lpResult = (char*)malloc (nLength);

  memset (lpResult, 0, nLength);

  WideCharToMultiByte (
          CP_OEMCP,
          0,
          lpwszUnicodeString,
          nLength,
          lpResult,
          nLength,
          NULL,
          NULL
          );

  return lpResult;
}

void ConvertPanelItemA(const oldfar::PluginPanelItem *PanelItemA, PluginPanelItem **PanelItemW, int ItemsNumber)
{
	*PanelItemW = (PluginPanelItem *)malloc(ItemsNumber*sizeof(PluginPanelItem));

	memset(*PanelItemW,0,ItemsNumber*sizeof(PluginPanelItem));

	for (int i=0; i<ItemsNumber; i++)
	{
		(*PanelItemW)[i].Flags = PanelItemA[i].Flags;
		(*PanelItemW)[i].NumberOfLinks = PanelItemA[i].NumberOfLinks;

		if (PanelItemA[i].Description)
			(*PanelItemW)[i].Description = AnsiToUnicode(PanelItemA[i].Description);

		if (PanelItemA[i].Owner)
			(*PanelItemW)[i].Owner = AnsiToUnicode(PanelItemA[i].Owner);

		if (PanelItemA[i].CustomColumnNumber)
		{
			(*PanelItemW)[i].CustomColumnNumber = PanelItemA[i].CustomColumnNumber;
			(*PanelItemW)[i].CustomColumnData = (wchar_t **)malloc(PanelItemA[i].CustomColumnNumber*sizeof(wchar_t *));

			for (int j=0; j<PanelItemA[i].CustomColumnNumber; j++)
				(*PanelItemW)[i].CustomColumnData[j] = AnsiToUnicode(PanelItemA[i].CustomColumnData[j]);
		}

		(*PanelItemW)[i].UserData = PanelItemA[i].UserData;
		(*PanelItemW)[i].CRC32 = PanelItemA[i].CRC32;

		(*PanelItemW)[i].FindData.dwFileAttributes = PanelItemA[i].FindData.dwFileAttributes;
		(*PanelItemW)[i].FindData.ftCreationTime = PanelItemA[i].FindData.ftCreationTime;
		(*PanelItemW)[i].FindData.ftLastAccessTime = PanelItemA[i].FindData.ftLastAccessTime;
		(*PanelItemW)[i].FindData.ftLastWriteTime = PanelItemA[i].FindData.ftLastWriteTime;
		(*PanelItemW)[i].FindData.nFileSize = (unsigned __int64)PanelItemA[i].FindData.nFileSizeLow + (((unsigned __int64)PanelItemA[i].FindData.nFileSizeHigh)<<32);
		(*PanelItemW)[i].FindData.nPackSize = (unsigned __int64)PanelItemA[i].PackSize + (((unsigned __int64)PanelItemA[i].PackSizeHigh)<<32);
		(*PanelItemW)[i].FindData.lpwszFileName = AnsiToUnicode(PanelItemA[i].FindData.cFileName);
		(*PanelItemW)[i].FindData.lpwszAlternateFileName = AnsiToUnicode(PanelItemA[i].FindData.cAlternateFileName);
	}
}

void ConvertPanelItemW(const PluginPanelItem *PanelItemW, oldfar::PluginPanelItem **PanelItemA, int ItemsNumber)
{
	*PanelItemA = (oldfar::PluginPanelItem *)malloc(ItemsNumber*sizeof(oldfar::PluginPanelItem));

	memset(*PanelItemA,0,ItemsNumber*sizeof(oldfar::PluginPanelItem));

	for (int i=0; i<ItemsNumber; i++)
	{
		(*PanelItemA)[i].Flags = PanelItemW[i].Flags;
		(*PanelItemA)[i].NumberOfLinks = PanelItemW[i].NumberOfLinks;

		if (PanelItemW[i].Description)
			(*PanelItemA)[i].Description = UnicodeToAnsi(PanelItemW[i].Description);

		if (PanelItemW[i].Owner)
			(*PanelItemA)[i].Owner = UnicodeToAnsi(PanelItemW[i].Owner);

		if (PanelItemW[i].CustomColumnNumber)
		{
			(*PanelItemA)[i].CustomColumnNumber = PanelItemW[i].CustomColumnNumber;
			(*PanelItemA)[i].CustomColumnData = (char **)malloc(PanelItemW[i].CustomColumnNumber*sizeof(char *));

			for (int j=0; j<PanelItemW[i].CustomColumnNumber; j++)
				(*PanelItemA)[i].CustomColumnData[j] = UnicodeToAnsi(PanelItemW[i].CustomColumnData[j]);
		}

		(*PanelItemA)[i].UserData = PanelItemW[i].UserData;
		(*PanelItemA)[i].CRC32 = PanelItemW[i].CRC32;

		(*PanelItemA)[i].FindData.dwFileAttributes = PanelItemW[i].FindData.dwFileAttributes;
		(*PanelItemA)[i].FindData.ftCreationTime = PanelItemW[i].FindData.ftCreationTime;
		(*PanelItemA)[i].FindData.ftLastAccessTime = PanelItemW[i].FindData.ftLastAccessTime;
		(*PanelItemA)[i].FindData.ftLastWriteTime = PanelItemW[i].FindData.ftLastWriteTime;
		(*PanelItemA)[i].FindData.nFileSizeLow = (DWORD)PanelItemW[i].FindData.nFileSize;
		(*PanelItemA)[i].FindData.nFileSizeHigh = (DWORD)(PanelItemW[i].FindData.nFileSize>>32);
		(*PanelItemA)[i].PackSize = (DWORD)PanelItemW[i].FindData.nPackSize;
		(*PanelItemA)[i].PackSizeHigh = (DWORD)(PanelItemW[i].FindData.nPackSize>>32);
		UnicodeToAnsi(PanelItemW[i].FindData.lpwszFileName,(*PanelItemA)[i].FindData.cFileName,sizeof((*PanelItemA)[i].FindData.cFileName));
		UnicodeToAnsi(PanelItemW[i].FindData.lpwszAlternateFileName,(*PanelItemA)[i].FindData.cAlternateFileName,sizeof((*PanelItemA)[i].FindData.cAlternateFileName));
	}
}

void FreePanelItemW(PluginPanelItem *PanelItem, int ItemsNumber)
{
	for (int i=0; i<ItemsNumber; i++)
	{
		if (PanelItem[i].Description)
			free(PanelItem[i].Description);

		if (PanelItem[i].Owner)
			free(PanelItem[i].Owner);

		if (PanelItem[i].CustomColumnNumber)
		{
			for (int j=0; j<PanelItem[i].CustomColumnNumber; j++)
				free(PanelItem[i].CustomColumnData[j]);

			free(PanelItem[i].CustomColumnData);
		}

		free(PanelItem[i].FindData.lpwszFileName);
		free(PanelItem[i].FindData.lpwszAlternateFileName);
	}

	free(PanelItem);
}

void FreePanelItemA(oldfar::PluginPanelItem *PanelItem, int ItemsNumber)
{
	for (int i=0; i<ItemsNumber; i++)
	{
		if (PanelItem[i].Description)
			free(PanelItem[i].Description);

		if (PanelItem[i].Owner)
			free(PanelItem[i].Owner);

		if (PanelItem[i].CustomColumnNumber)
		{
			for (int j=0; j<PanelItem[i].CustomColumnNumber; j++)
				free(PanelItem[i].CustomColumnData[j]);

			free(PanelItem[i].CustomColumnData);
		}
	}

	free(PanelItem);
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
	//BUGBUG, эрфю яЁютхЁ Є№, ўЄю PluginHandle - яырушэ

	Plugin *pPlugin = (Plugin*)PluginHandle;

	string strPath = pPlugin->GetModuleName();

	CutToSlash(strPath);

	if ( pPlugin->InitLang(strPath) )
		return pPlugin->GetMsgA(MsgId);

	return "";
}

int WINAPI FarMenuFnA(INT_PTR PluginNumber,int X,int Y,int MaxHeight,DWORD Flags,const char *Title,const char *Bottom,const char *HelpTopic,const int *BreakKeys,int *BreakCode,const struct oldfar::FarMenuItem *Item,int ItemsNumber)
{
	string strT((Title?Title:"")), strB((Bottom?Bottom:"")), strHT((HelpTopic?HelpTopic:""));

	if (!Item || !ItemsNumber)	return -1;

	FarMenuItemEx *mi = (FarMenuItemEx *)malloc(ItemsNumber*sizeof(*mi));

	if (Flags&FMENU_USEEXT)
	{
		oldfar::FarMenuItemEx *p = (oldfar::FarMenuItemEx *)Item;

		for (int i=0; i<ItemsNumber; i++)
		{
			mi[i].Flags = p[i].Flags;
			mi[i].Text = AnsiToUnicode(p[i].Flags&MIF_USETEXTPTR?p[i].Text.TextPtr:p[i].Text.Text);
			mi[i].AccelKey = p[i].AccelKey;
			mi[i].Reserved = p[i].Reserved;
			mi[i].UserData = p[i].UserData;
		}
	}
	else
	{
		for (int i=0; i<ItemsNumber; i++)
		{
			mi[i].Flags=0;
			if (Item[i].Selected)
				mi[i].Flags|=MIF_SELECTED;
			if (Item[i].Checked)
			{
				mi[i].Flags|=MIF_CHECKED;
				if (Item[i].Checked>1)
					mi[i].Flags|=Item[i].Checked&0xFF; //BUGBUG надо перекодировать символ пометки в юникод
			}
			if (Item[i].Separator)
				mi[i].Flags|=MIF_SEPARATOR;
			mi[i].Text = AnsiToUnicode(Item[i].Text);
			mi[i].AccelKey = 0;
			mi[i].Reserved = 0;
			mi[i].UserData = 0;
		}
	}

	int ret = FarMenuFn(PluginNumber,X,Y,MaxHeight,Flags|FMENU_USEEXT,(Title?(const wchar_t *)strT:NULL),(Bottom?(const wchar_t *)strB:NULL),(HelpTopic?(const wchar_t *)strHT:NULL),BreakKeys,BreakCode,(FarMenuItem *)mi,ItemsNumber);

	for (int i=0; i<ItemsNumber; i++)
		if (mi[i].Text) free((wchar_t *)mi[i].Text);
	if (mi) free(mi);

	return ret;
}

int WINAPI FarDialogFnA(INT_PTR PluginNumber,int X1,int Y1,int X2,int Y2,const char *HelpTopic,struct oldfar::FarDialogItem *Item,int ItemsNumber)
{
	return -1;
}

int WINAPI FarControlA(HANDLE hPlugin,int Command,void *Param)
{
	static oldfar::PanelInfo PIA={0};
	HANDLE hPluginW = CURRENT_PANEL;

	switch (Command)
	{
		case oldfar::FCTL_CHECKPANELSEXIST:
			return FarControl(hPlugin,FCTL_CHECKPANELSEXIST,Param);

		case oldfar::FCTL_CLOSEPLUGIN:
			{
				wchar_t *ParamW = NULL;
				if (Param)
					ParamW = AnsiToUnicode((const char *)Param);
				int ret = FarControl(hPlugin,FCTL_CLOSEPLUGIN,ParamW);
				if (ParamW) free(ParamW);
				return ret;
			}

		case oldfar::FCTL_GETANOTHERPANELINFO:
			hPluginW = ANOTHER_PANEL;
		case oldfar::FCTL_GETPANELINFO:
			{
				PanelInfo PIW;
				int ret = FarControl(hPluginW,FCTL_GETPANELINFO,(void *)&PIW);
				if (ret)
				{
				}
				//return ret;
			}
	}
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
	static char *gt=NULL;
	static char *geol=NULL;
	static char *fn=NULL;

	switch (Command)
	{
		case ECTL_ADDCOLOR:
		case ECTL_DELETEBLOCK:
		case ECTL_DELETECHAR:
		case ECTL_DELETESTRING:
		case ECTL_EXPANDTABS:
		case ECTL_GETCOLOR:
		case ECTL_GETBOOKMARKS:
		case ECTL_INSERTSTRING:
		case ECTL_QUIT:
		case ECTL_REALTOTAB:
		case ECTL_REDRAW:
		case ECTL_SELECT:
		case ECTL_SETPOSITION:
		case ECTL_TABTOREAL:
		case ECTL_TURNOFFMARKINGBLOCK:
			return FarEditorControl(Command,Param);

		case ECTL_GETSTRING:
			{
				EditorGetString egs;
				oldfar::EditorGetString *oegs=(oldfar::EditorGetString *)Param;
				if (!oegs) return FALSE;
				egs.StringNumber=oegs->StringNumber;
				int ret=FarEditorControl(ECTL_GETSTRING,&egs);
				if (ret)
				{
					oegs->StringNumber=egs.StringNumber;
					oegs->StringLength=egs.StringLength;
					oegs->SelStart=egs.SelStart;
					oegs->SelEnd=egs.SelEnd;
					if (gt) free(gt);
					if (geol) free(geol);
					gt = UnicodeToAnsiBin(egs.StringText,egs.StringLength);
					geol = UnicodeToAnsi(egs.StringEOL);
					oegs->StringText=gt;
					oegs->StringEOL=geol;
					return TRUE;
				}
				return FALSE;
			}

		case ECTL_INSERTTEXT:
		{
			const char *p=(const char *)Param;
			if (!p) return FALSE;
			string strP(p);
			return FarEditorControl(ECTL_INSERTTEXT,(void *)(const wchar_t *)strP);
		}

		case ECTL_GETINFO:
		{
			EditorInfo ei;
			oldfar::EditorInfo *oei=(oldfar::EditorInfo *)Param;
			if (!oei) return FALSE;
			int ret=FarEditorControl(ECTL_GETINFO,&ei);
			if (ret)
			{
				memset(oei,0,sizeof(*oei));
				oei->EditorID=ei.EditorID;
				if (fn)	free(fn);
				fn = UnicodeToAnsi(ei.FileName);
				oei->FileName=fn;
				oei->WindowSizeX=ei.WindowSizeX;
				oei->WindowSizeY=ei.WindowSizeY;
				oei->TotalLines=ei.TotalLines;
				oei->CurLine=ei.CurLine;
				oei->CurPos=ei.CurPos;
				oei->CurTabPos=ei.CurTabPos;
				oei->TopScreenLine=ei.TopScreenLine;
				oei->LeftPos=ei.LeftPos;
				oei->Overtype=ei.Overtype;
				oei->BlockType=ei.BlockType;
				oei->BlockStartLine=ei.BlockStartLine;
				oei->AnsiMode=0;
				oei->TableNum=-1;
				oei->Options=ei.Options;
				oei->TabSize=ei.TabSize;
				oei->BookMarkCount=ei.BookMarkCount;
				oei->CurState=ei.CurState;
				return TRUE;
			}
			return FALSE;
		}

		case ECTL_EDITORTOOEM:
		case ECTL_OEMTOEDITOR:
			return TRUE;

		case ECTL_SAVEFILE:
		case ECTL_PROCESSINPUT:
		case ECTL_PROCESSKEY:
		case ECTL_READINPUT:
		case ECTL_SETKEYBAR:
		case ECTL_SETPARAM:
		case ECTL_SETSTRING:
		case ECTL_SETTITLE:
			return FALSE;

	}

	return FALSE;
}

int WINAPI FarViewerControlA(int Command,void* Param)
{
	return TRUE;
}
