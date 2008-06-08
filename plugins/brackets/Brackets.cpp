#define _FAR_NO_NAMELESS_UNIONS
#define _FAR_USE_FARFINDDATA
#include "plugin.hpp"
#include "CRT/crt.hpp"

#include "Brackets.hpp"
#include "BrackLng.hpp"

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
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

#include "BrackMix.cpp"
#include "BrackCfg.cpp"

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  lstrcpy(PluginRootKey,Info->RootKey);
  lstrcat(PluginRootKey,"\\Brackets");

  HKEY hKey;
  DWORD Type;
  DWORD DataSize=0;
  if ((RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,
         KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
  {
    DataSize=sizeof(Opt);
    RegQueryValueEx(hKey,"Options",0,&Type,(LPBYTE)&Opt,&DataSize);
    RegCloseKey(hKey);
  }

  if(DataSize != sizeof(Opt))
  {
    Opt.IgnoreQuotes=0;
    Opt.IgnoreAfter=0;
    Opt.BracketPrior=1;
    Opt.JumpToPair=1;
    Opt.Beep=0;
    lstrcpy(Opt.QuotesType,"''\"\"`'``");
    lstrcpy(Opt.Brackets1,"<>{}[]()\"\"''%%");
    lstrcpy(Opt.Brackets2,"/**/<\?\?><%%>");
  }
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  static const char *PluginMenuStrings[1]={0};
  PluginMenuStrings[0]=GetMsg(MTitle);
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  Info->DiskMenuStringsNumber=0;
  static const char *PluginCfgStrings[1];
  PluginCfgStrings[0]=GetMsg(MTitle);
  Info->PluginConfigStrings=PluginCfgStrings;
  Info->PluginConfigStringsNumber=sizeof(PluginCfgStrings)/sizeof(PluginCfgStrings[0]);
}


int WINAPI _export Configure(int ItemNumber)
{
  if(!ItemNumber)
    return(Config());
  return(FALSE);
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,INT_PTR Item)
{
  struct EditorInfo ei;
  struct EditorGetString egs;
  struct EditorSetPosition esp,espo;
  struct EditorSelect es;

  char Bracket,Bracket1,Bracket_1;
  char Ch,Ch1,Ch_1;
  char B21=0,B22=0,B23=0,B24=0;

  int nQuotes=0;
  int isSelect=-1;

  int CurPos,i=3,j,k;

  int Direction=0,DirectQuotes;
  int types=BrZERO;

  BOOL found=FALSE;
  int MatchCount=1;

  int idxBrackets1=0;
  int lenBrackets1=0;
  int idxBrackets2=0;
  int lenBrackets2=0;

  Info.EditorControl(ECTL_GETINFO,&ei);

  espo.CurTabPos=ei.CurTabPos;
  espo.TopScreenLine=ei.TopScreenLine;
  espo.LeftPos=ei.LeftPos;
  espo.Overtype=ei.Overtype;

  espo.CurLine=ei.CurLine;
  espo.CurPos=CurPos=ei.CurPos;
  egs.StringNumber=-1;

  Info.EditorControl(ECTL_GETSTRING,&egs);

  if(isSelect == -1)
    if((isSelect=ShowMenu(0)) == -1)
      return(INVALID_HANDLE_VALUE);

  if (CurPos > egs.StringLength)
    return(INVALID_HANDLE_VALUE);

  egs.StringNumber=espo.CurLine;

  isSelect=isSelect == 1;

  Bracket_1=(CurPos-1 >= 0?egs.StringText[CurPos-1]:'\0');
  Bracket1=(CurPos+1 < egs.StringLength?egs.StringText[CurPos+1]:'\0');

  if(!Opt.QuotesType[0])
    Opt.IgnoreQuotes=1;
  else {
    i=lstrlen(Opt.QuotesType);
    if((i&1) == 1)
    {
      if(--i > 0)
        Opt.QuotesType[i]=0;
      else
        Opt.IgnoreQuotes=1;
    }
    nQuotes=i;
  }

  Opt.BracketPrior&=1;
  Opt.IgnoreAfter&=1;

  if(Opt.IgnoreQuotes == 0)
  {
    for(i=0; i < nQuotes; i+=2)
      if(Bracket_1 == Opt.QuotesType[i] && Bracket1 == Opt.QuotesType[i+1])
      return(INVALID_HANDLE_VALUE);
  }

  Bracket=(CurPos == egs.StringLength)?'\0':egs.StringText[CurPos];

  if(((lenBrackets1=lstrlen(Opt.Brackets1)) & 1) != 0)
  {
    lenBrackets1-=(lenBrackets1&1);
    Opt.Brackets1[lenBrackets1]=0;
  }
  lenBrackets1>>=1;

  if(((lenBrackets2=lstrlen(Opt.Brackets2)) & 3) != 0)
  {
    lenBrackets2-=(lenBrackets2&3);
    Opt.Brackets2[lenBrackets2]=0;
  }
  lenBrackets2>>=2;

  i=3;
  short BracketPrior=Opt.BracketPrior;
  while(--i)
  {
    if(BracketPrior == 1)
    {
      BracketPrior--;
      if(types == BrZERO && lenBrackets2)
      {
        for(idxBrackets2=0;lenBrackets2 > 0; --lenBrackets2,idxBrackets2+=4)
        {
          B21=Opt.Brackets2[idxBrackets2+0];
          B22=Opt.Brackets2[idxBrackets2+1];
          B23=Opt.Brackets2[idxBrackets2+2];
          B24=Opt.Brackets2[idxBrackets2+3];
          if(Bracket == B21 || Bracket == B22 || Bracket == B23 || Bracket == B24)
          {
            if ((Bracket==B21 && Bracket1==B22) || (Bracket==B22 && Bracket_1==B21))
            {
              types=BrTwo;
              Direction=1;
            }
            else if ((Bracket_1==B23 && Bracket==B24) || (Bracket==B23 && Bracket1==B24))
            {
              types=BrTwo;
              Direction=-1;
            }
            break;
          }
        }
      }
    }
    else
    {
      BracketPrior++;
      if(types == BrZERO && lenBrackets1)
      {
        int LB=lenBrackets1;
        for(idxBrackets1=0;lenBrackets1 > 0; --lenBrackets1,idxBrackets1+=2)
        {
          B21=Opt.Brackets1[idxBrackets1+0];
          B22=Opt.Brackets1[idxBrackets1+1];

          if (Bracket==B21)
          {
            types=BrOne;
            Direction=1;
            break;
          }
          else if (Bracket==B22)
          {
            types=BrOne;
            Direction=-1;
            break;
          }
        }
        if(types == BrZERO && !Opt.IgnoreAfter)
          for(idxBrackets1=0;LB > 0; --LB,idxBrackets1+=2)
          {
            B21=Opt.Brackets1[idxBrackets1+0];
            B22=Opt.Brackets1[idxBrackets1+1];

            if (Bracket_1==B21)
            {
              types=BrColorer;
              Direction=1;
              break;
            }
            else if (Bracket_1==B22)
            {
              types=BrColorer;
              Direction=-1;
              break;
            }
          }
      }
    }
  }

  if(Opt.IgnoreAfter && types == BrColorer)
    return(INVALID_HANDLE_VALUE);
  if(types == BrZERO)
    return(INVALID_HANDLE_VALUE);

  if(B21 == B22)
  {
    if((DirectQuotes=ShowMenu(1)) == -1)
      return(INVALID_HANDLE_VALUE);
    Direction=DirectQuotes == 0?1:-1;
    types=BrOneMath;
  }

  while (!found)
  {
    CurPos+=Direction;
    if (CurPos >= egs.StringLength)
    {
      if (++egs.StringNumber >= ei.TotalLines)
        break;
      Info.EditorControl(ECTL_GETSTRING,&egs);
      CurPos=0;
    }
    if (CurPos < 0)
    {
      if (--egs.StringNumber < 0)
        break;

      Info.EditorControl(ECTL_GETSTRING,&egs);
      CurPos=egs.StringLength-1;
    }
    if (CurPos > egs.StringLength || CurPos < 0)
      continue;

    Ch=((CurPos == egs.StringLength)?'\0':egs.StringText[CurPos]);
    Ch_1=(CurPos-1 >= 0?egs.StringText[CurPos-1]:'\0');
    Ch1=(CurPos+1 < egs.StringLength?egs.StringText[CurPos+1]:'\0');

    if(Opt.IgnoreQuotes == 0)
    {
      for(k=j=0; j < nQuotes; j+=2)
        if(Ch_1 == Opt.QuotesType[j] && Ch1 == Opt.QuotesType[j+1])
        {
          k++;
          break;
        }
      if(k)
        continue;
    }

    switch(types)
    {
      /***************************************************************/
      case BrOneMath:
      {
        if(Ch == Bracket)
        {
          if ((Bracket_1=='\\' && Ch_1=='\\') ||
              (Bracket_1!='\\' && Ch_1!='\\')
             )
            found=TRUE;
        }
        break;
      }

      /***************************************************************/
      case BrColorer:
      {
        if(Ch == Bracket_1)
        {
          MatchCount++;
        }
        else if ((Ch==B21 && Bracket_1==B22) || (Ch==B22 && Bracket_1==B21))
        {
          --MatchCount;
          if((Direction ==  1 && MatchCount == 0) ||
             (Direction == -1 && MatchCount == 1))
            found=TRUE;
        }
        break;
      }

      /***************************************************************/
      case BrOne:
      {
        if(Ch == Bracket)
        {
          MatchCount++;
        }
        else if ((Ch==B21 && Bracket==B22) || (Ch==B22 && Bracket==B21))
        {
          if(--MatchCount==0)
            found=TRUE;
        }
        break;
      }

      /***************************************************************/

      case BrTwo:
      {
        if((Direction == 1 &&
           ((Bracket==B21 && Ch==B21 && Ch1  == B22) ||
           (Bracket==B22 && Ch==B22 && Ch_1 == B21))
           ) ||
           (Direction == -1 &&
           ((Bracket==B23 && Ch==B23 && Ch1  == B24) ||
           (Bracket==B24 && Ch==B24 && Ch_1 == B23))
           )
          )
        {
          MatchCount++;
        }
        else if (
                 (Bracket==B21 && Ch==B23 && Bracket1 ==B22 && Ch1 ==B24) ||
                 (Bracket==B22 && Ch==B24 && Bracket_1==B21 && Ch_1==B23) ||
                 (Bracket==B23 && Ch==B21 && Bracket1 ==B24 && Ch1 ==B22) ||
                 (Bracket==B24 && Ch==B22 && Bracket_1==B23 && Ch_1==B21)
                )
        {
          if(--MatchCount==0)
            found=TRUE;
        }
        break;
      }
    }
  }

  if(found)
  {
    esp.CurLine=egs.StringNumber;
    if(types == BrTwo)
    {
      if(Bracket == B21 || Bracket == B24)
        CurPos+=Direction;
      else if(Bracket == B22 || Bracket == B23)
        CurPos-=Direction;
    }
    //if(types == BrColorer)
    //  CurPos++;
    esp.CurPos=CurPos;

    esp.CurTabPos=esp.LeftPos=esp.Overtype=-1;
    if (egs.StringNumber<ei.TopScreenLine ||
        egs.StringNumber>=ei.TopScreenLine+ei.WindowSizeY)
    {
      esp.TopScreenLine=esp.CurLine-ei.WindowSizeY/2;
      if (esp.TopScreenLine < 0)
        esp.TopScreenLine=0;
    }
    else
      esp.TopScreenLine=-1;

    if(!isSelect || (isSelect && Opt.JumpToPair))
      Info.EditorControl(ECTL_SETPOSITION,&esp);

    if(Opt.Beep)
       MessageBeep(0);

    if(isSelect)
    {
      es.BlockType=BTYPE_STREAM;
      es.BlockStartLine=min(esp.CurLine,espo.CurLine);
      es.BlockStartPos=(Direction > 0?espo.CurPos:esp.CurPos)
          -(types == BrColorer?1:0);
      if(espo.CurPos == esp.CurPos)
        es.BlockStartPos+=(Direction > 0?1:-1);
      es.BlockHeight=max(esp.CurLine,espo.CurLine)-min(esp.CurLine,espo.CurLine)+1;
      if(Direction > 0)
        es.BlockWidth=esp.CurPos-espo.CurPos+1;
      else
        es.BlockWidth=espo.CurPos-esp.CurPos+1;
      /*
      if(espo.CurPos == esp.CurPos)
      {
        if(Direction > 0)
          es.BlockStartPos--;
        else
          es.BlockStartPos++;
      }
      */
      Info.EditorControl(ECTL_SELECT,(void*)&es);
    }
  }
  return(INVALID_HANDLE_VALUE);
}
