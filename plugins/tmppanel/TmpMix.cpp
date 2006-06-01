/*
TMPMIX.CPP

Temporary panel miscellaneous utility functions

*/

const char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

struct MyInitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  DWORD Flags;
  signed char Data;
};

void InitDialogItems(const struct MyInitDialogItem *Init,struct FarDialogItem *Item,
                    int ItemsNumber)
{
  int i;
  struct FarDialogItem *PItem=Item;
  const struct MyInitDialogItem *PInit=Init;
  for (i=0;i<ItemsNumber;i++,PItem++,PInit++)
  {
    PItem->Type=PInit->Type;
    PItem->X1=PInit->X1;
    PItem->Y1=PInit->Y1;
    PItem->X2=PInit->X2;
    PItem->Y2=PInit->Y2;
    PItem->Flags=PInit->Flags;
    PItem->Focus=0;
    PItem->Selected=0;
    PItem->DefaultButton=0;
    lstrcpy(PItem->Data,PInit->Data!=-1 ? GetMsg(PInit->Data) : "");
  }
}

void FreePanelItems(PluginPanelItem *Items, DWORD Total)
{
  if(Items){
    for (DWORD I=0;I<Total;I++)
      if (Items[I].Owner)
        free (Items[I].Owner);
    free (Items);
  }
}
#if !defined(_MSC_VER)
#if defined(__BORLANDC__)
char * __cdecl strchr (char * string,int ch)
#else
char * __cdecl strchr (const char * string,int ch)
#endif
{
  while (*string && *string != (char)ch)
    string++;
  if (*string == (char)ch)
    return((char *)string);
  return(NULL);
}
#endif

#if !defined(_MSC_VER)
#if defined(__BORLANDC__)
char * __cdecl strrchr(char * string,int ch)
#else
char * __cdecl strrchr(const char * string,int ch)
#endif
{
  char *start = (char *)string;
  while (*string++);
  while (--string != start && *string != (char)ch);
  if (*string == (char)ch)
    return( (char *)string );
  return(NULL);
}
#endif

char *ParseParam(char *& str)
{
  char* p=str;
  char* parm=NULL;
  if(*p=='|'){
    parm=++p;
    p=strchr(p,'|');
    if(p){
      *p='\0';
      str=p+1;
      FSF.LTrim(str);
      return parm;
    }
  }
  return NULL;
}

void GoToFile(const char *Target, BOOL AnotherPanel)
{
  int FCTL_SetPanelDir = AnotherPanel?FCTL_SETANOTHERPANELDIR:FCTL_SETPANELDIR;
  int FCTL_GetPanelInfo = AnotherPanel?FCTL_GETANOTHERPANELINFO:FCTL_GETPANELINFO;
  int FCTL_RedrawPanel = AnotherPanel?FCTL_REDRAWANOTHERPANEL:FCTL_REDRAWPANEL;

  PanelRedrawInfo PRI;
  PanelInfo PInfo;
  char Name[NM], Dir[NM*5];
  int pathlen;

  lstrcpy(Name,FSF.PointToName(const_cast<char*>(Target)));
  pathlen=FSF.PointToName(const_cast<char*>(Target))-Target;
  if(pathlen)
    memcpy(Dir,Target,pathlen);
  Dir[pathlen]=0;

  FSF.Trim(Name);
  FSF.Trim(Dir);
  FSF.Unquote(Name);
  FSF.Unquote(Dir);

  if(*Dir)
    Info.Control(INVALID_HANDLE_VALUE,FCTL_SetPanelDir,&Dir);
  Info.Control(INVALID_HANDLE_VALUE,FCTL_GetPanelInfo,&PInfo);

  PRI.CurrentItem=PInfo.CurrentItem;
  PRI.TopPanelItem=PInfo.TopPanelItem;

  for(int J=0; J < PInfo.ItemsNumber; J++)
  {
    if(!FSF.LStricmp(Name,
       FSF.PointToName(PInfo.PanelItems[J].FindData.cFileName)))
    {
      PRI.CurrentItem=J;
      PRI.TopPanelItem=J;
      break;
    }
  }
  Info.Control(INVALID_HANDLE_VALUE,FCTL_RedrawPanel,&PRI);
}

//extern "C" int __stdcall _DllMainCRTStartup (HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
//{
//  return 1;
//}
