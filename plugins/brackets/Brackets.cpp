#define _FAR_NO_NAMELESS_UNIONS
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

int WINAPI EXP_NAME(GetMinFarVersion)()
{
  return FARMANAGERVERSION;
}

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  lstrcpy(PluginRootKey,Info->RootKey);
  lstrcat(PluginRootKey,_T("\\Brackets"));

  HKEY hKey;
  DWORD Type;
  DWORD DataSize=0;
  if ((RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,
         KEY_QUERY_VALUE,&hKey))==ERROR_SUCCESS)
  {
    DataSize=sizeof(Opt);
    RegQueryValueEx(hKey,_T("Options"),0,&Type,(LPBYTE)&Opt,&DataSize);
    RegCloseKey(hKey);
  }

  if(DataSize != sizeof(Opt))
  {
    Opt.IgnoreQuotes=0;
    Opt.IgnoreAfter=0;
    Opt.BracketPrior=1;
    Opt.JumpToPair=1;
    Opt.Beep=0;
    lstrcpy(Opt.QuotesType,_T("''\"\"`'``"));
    lstrcpy(Opt.Brackets1,_T("<>{}[]()\"\"''%%"));
    lstrcpy(Opt.Brackets2,_T("/**/<\?\?><%%>"));
  }
}


void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
  static const TCHAR *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MTitle);
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=ARRAYSIZE(PluginMenuStrings);
  Info->DiskMenuStringsNumber=0;
  static const TCHAR *PluginCfgStrings[1];
  PluginCfgStrings[0]=GetMsg(MTitle);
  Info->PluginConfigStrings=PluginCfgStrings;
  Info->PluginConfigStringsNumber=ARRAYSIZE(PluginCfgStrings);
}


int WINAPI EXP_NAME(Configure)(int ItemNumber)
{
  if(!ItemNumber)
    return(Config());
  return(FALSE);
}


HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom,INT_PTR Item)
{
  struct EditorGetString egs;
  struct EditorSetPosition esp,espo;
  struct EditorSelect es;

  TCHAR Bracket,Bracket1,Bracket_1;
  TCHAR Ch,Ch1,Ch_1;
  TCHAR B21=0,B22=0,B23=0,B24=0;

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

  EditorInfo ei;
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

  Bracket_1=(CurPos-1 >= 0?egs.StringText[CurPos-1]:_T('\0'));
  Bracket1=(CurPos+1 < egs.StringLength?egs.StringText[CurPos+1]:_T('\0'));

  if(!Opt.QuotesType[0])
    Opt.IgnoreQuotes=1;
  else
  {
    // размер Opt.QuotesType должен быть кратный двум (иначе усекаем)
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

  Bracket=(CurPos == egs.StringLength)?_T('\0'):egs.StringText[CurPos];

  // размер Opt.Brackets1 должен быть кратный двум (иначе усекаем)
  if(((lenBrackets1=lstrlen(Opt.Brackets1)) & 1) != 0)
  {
    lenBrackets1-=(lenBrackets1&1);
    Opt.Brackets1[lenBrackets1]=0;
  }
  lenBrackets1>>=1;

  // размер Opt.Brackets1 должен быть кратный четырем (иначе усекаем)
  if(((lenBrackets2=lstrlen(Opt.Brackets2)) & 3) != 0)
  {
    lenBrackets2-=(lenBrackets2&3);
    Opt.Brackets2[lenBrackets2]=0;
  }
  lenBrackets2>>=2;

  // анализ того, что под курсором
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
              types=BrRight;
              Direction=1;
              break;
            }
            else if (Bracket_1==B22)
            {
              types=BrRight;
              Direction=-1;
              break;
            }
          }
      }
    }
  }

  if(Opt.IgnoreAfter && types == BrRight)
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

  esp.CurPos=esp.CurTabPos=esp.TopScreenLine=esp.LeftPos=esp.Overtype=-1;
  esp.CurLine=egs.StringNumber;
  egs.StringNumber=-1;

  // поиск пары
  while (!found)
  {
    CurPos+=Direction;

    bool cond_gt=CurPos >= egs.StringLength?true:false;

    if (cond_gt || CurPos < 0)
    {
      if (cond_gt)
        esp.CurLine++;
      else
        esp.CurLine--;

      if(esp.CurLine >= ei.TotalLines || esp.CurLine < 0)
        break;

      Info.EditorControl(ECTL_SETPOSITION,&esp);
      Info.EditorControl(ECTL_GETSTRING,&egs);

      if (cond_gt)
        CurPos=0;
      else
        CurPos=egs.StringLength-1;
    }

    if (CurPos > egs.StringLength || CurPos < 0)
      continue;

    Ch_1=(CurPos-1 >= 0?egs.StringText[CurPos-1]:_T('\0'));
    Ch=((CurPos == egs.StringLength)?_T('\0'):egs.StringText[CurPos]);
    Ch1=(CurPos+1 < egs.StringLength?egs.StringText[CurPos+1]:_T('\0'));

    // BUGBUGBUG!!!
    if(Opt.IgnoreQuotes == 1)
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
          if ((Bracket_1==_T('\\') && Ch_1==_T('\\')) ||
              (Bracket_1!=_T('\\') && Ch_1!=_T('\\'))
             )
            found=TRUE;
        }
        break;
      }

      /***************************************************************/
      case BrRight:
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
    egs.StringNumber=esp.CurLine;
    if(types == BrTwo)
    {
      if(Bracket == B21 || Bracket == B24)
        CurPos+=Direction;
      else if(Bracket == B22 || Bracket == B23)
        CurPos-=Direction;
    }

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
      es.BlockStartPos=(Direction > 0?espo.CurPos:esp.CurPos);
      es.BlockHeight=max(esp.CurLine,espo.CurLine)-min(esp.CurLine,espo.CurLine)+1;

      if(Direction > 0)
        es.BlockWidth=esp.CurPos-espo.CurPos+1;
      else
        es.BlockWidth=espo.CurPos-esp.CurPos+1;

      if(types == BrRight)
      {
        if(Direction > 0)
        {
          es.BlockStartPos--;
          es.BlockWidth++;
        }
        else if(Direction < 0)
        {
          es.BlockWidth--;
        }
      }

      Info.EditorControl(ECTL_SELECT,(void*)&es);
    }
  }
  else
  {
    Info.EditorControl(ECTL_SETPOSITION,&espo);
  }
  return(INVALID_HANDLE_VALUE);
}
