/*
help.cpp

������

*/

/* Revision: 1.98 07.07.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "help.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "plugin.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "BlockExtKey.hpp"


// ���� ��������
class CallBackStack
{
  private:
    struct ListNode
    {
      ListNode *Next;

      DWORD Flags;             // �����
      int   TopStr;            // ����� ������� ������� ������ ����
      int   CurX,CurY;         // ���������� (???)

      string strHelpTopic;        // ������� �����
      string strHelpPath;         // ���� � ������
      string strSelTopic;         // ������� ���������
      string strHelpMask;         // �����

      ListNode(const struct StackHelpData *Data, ListNode* n=NULL)
      {
        strHelpTopic=Data->strHelpTopic;
        strHelpPath=Data->strHelpPath;
        strSelTopic=Data->strSelTopic;
        strHelpMask=Data->strHelpMask;
        Flags=Data->Flags;
        TopStr=Data->TopStr;
        CurX=Data->CurX;
        CurY=Data->CurY;

        Next=n;
      }
      ~ListNode()
      {
      }
    };

    ListNode *topOfStack;

  public:
    CallBackStack() {topOfStack=NULL;};
   ~CallBackStack() {ClearStack();};

  public:
    void ClearStack();
    BOOL isEmpty() const {return topOfStack==NULL;};

    void Push(const struct StackHelpData *Data);
    int Pop(struct StackHelpData *Data=NULL);

    void PrintStack(const wchar_t *Title);
};


#define MAX_HELP_STRING_LENGTH 300

static const wchar_t *PluginContents=L"__PluginContents__";
#if defined(WORK_HELP_DOCUMS)
static const wchar_t *DocumentContents=L"__DocumentContents__";
#endif
static const wchar_t *HelpOnHelpTopic=L":Help";
static const wchar_t *HelpContents=L"Contents";

static int RunURL(const wchar_t *Protocol, wchar_t *URLPath);

wchar_t *ReadString (FILE *file, wchar_t *lpwszDest, int nDestLength, int nType)
{
    char *lpDest = (char*)xf_malloc ((nDestLength+1)*3); //UTF-8, up to 3 bytes per char support

    memset (lpDest, 0, (nDestLength+1)*3);
    memset (lpwszDest, 0, nDestLength*sizeof (wchar_t));

    if ( (nType == TYPE_UNICODE) || (nType == TYPE_REVERSEBOM) )
    {
        if ( !fgetws (lpwszDest, nDestLength, file) )
        {
            xf_free (lpDest);
            return NULL;
        }

        if ( nType == TYPE_REVERSEBOM )
        {
            swab ((char*)lpwszDest, (char*)lpwszDest, nDestLength*sizeof (wchar_t));

            wchar_t *Ch = lpwszDest;
            int nLength = min (static_cast<int>(wcslen (lpwszDest)), nDestLength);

            while ( *Ch )
            {
                if ( *Ch == L'\n' )
                {
                    *(Ch+1) = 0;
                    break;
                }

                Ch++;
            }

            int nNewLength = min (static_cast<int>(wcslen (lpwszDest)), nDestLength);

            fseek (file, (nNewLength-nLength)*sizeof (wchar_t), SEEK_CUR);
        }

    }
    else

    if ( nType == TYPE_UTF8 )
    {
        if ( fgets (lpDest, nDestLength*3, file) )
            MultiByteToWideChar (CP_UTF8, 0, lpDest, -1, lpwszDest, nDestLength);
        else
        {
            xf_free (lpDest);
            return NULL;
        }

    }
    else

    if ( nType == TYPE_ANSI )
    {
        if ( fgets (lpDest, nDestLength, file) )
            MultiByteToWideChar (CP_OEMCP, 0, lpDest, -1, lpwszDest, nDestLength);
        else
        {
            xf_free (lpDest);
            return NULL;
        }
    }

    xf_free (lpDest);

    return lpwszDest;
}

Help::Help(const wchar_t *Topic, const wchar_t *Mask,DWORD Flags)
{
  /* $ OT �� ��������� ��� ����� ��������� ����������*/
  SetDynamicallyBorn(FALSE);
  CanLoseFocus=FALSE;
  PrevMacroMode=CtrlObject->Macro.GetMode();
  CtrlObject->Macro.SetMode(MACRO_HELP);

  ErrorHelp=TRUE;
  IsNewTopic=TRUE;

  MouseDown = FALSE;

  Stack=new CallBackStack;

  memset(&StackData,0,sizeof(StackData));

  StackData.Flags=Flags;
  StackData.strHelpMask = L"";
  StackData.strHelpPath = L"";
  StackData.strHelpTopic = L"";
  StackData.strSelTopic = L"";

  /* $ 01.09.2000 SVS
     ��������� �� ��������� ������� ���� ���������...
  */
  CurColor=COL_HELPTEXT;
  /* SVS $ */

  /* $ 27.11.2001 DJ
     �� ������� ����������������
  */
  CtrlTabSize = 8;
  /* DJ $ */

  StackData.strHelpMask = NullToEmptyW(Mask); // �������� ����� �����

  KeyBarVisible = TRUE;  // �������� ���������� ������
  TopScreen=new SaveScreen;
  HelpData=NULL;
  StackData.strHelpTopic = Topic;

  if (Opt.FullScreenHelp)
    SetPosition(0,0,ScrX,ScrY);
  else
    SetPosition(4,2,ScrX-4,ScrY-2);

  if(!ReadHelp(StackData.strHelpMask) && (Flags&FHELP_USECONTENTS))
  {
    StackData.strHelpTopic = Topic;
    if( StackData.strHelpTopic.At(0) == HelpBeginLink)
    {
      wchar_t *Ptr = StackData.strHelpTopic.GetBuffer ();

      Ptr=wcsrchr(Ptr,HelpEndLink);

      if ( Ptr )
        *Ptr = 0;

      StackData.strHelpTopic.ReleaseBuffer ();

      StackData.strHelpTopic += HelpContents;
    }
    StackData.strHelpPath=L"";
    ReadHelp(StackData.strHelpMask);
  }


  if (HelpData!=NULL)
  {
    ScreenObject::Flags.Clear(FHELPOBJ_ERRCANNOTOPENHELP);
    InitKeyBar();
    MacroMode = MACRO_HELP;
    MoveToReference(1,1);
    FrameManager->ExecuteModal (this);//OT
  }
  else
  {
    ErrorHelp=TRUE;
    if(!(Flags&FHELP_NOSHOWERROR))
    {
      if(!ScreenObject::Flags.Check(FHELPOBJ_ERRCANNOTOPENHELP))
      {
        BlockExtKey blockExtKey;
        MessageW(MSG_WARNING,1,UMSG(MHelpTitle),UMSG(MHelpTopicNotFound),StackData.strHelpTopic,UMSG(MOk));
      }
      ScreenObject::Flags.Clear(FHELPOBJ_ERRCANNOTOPENHELP);
    }
  }

#if defined(WORK_HELP_FIND)
  xstrncpy((char *)LastSearchStr,GlobalSearchString,sizeof(LastSearchStr));
  LastSearchPos=0;
  LastSearchCase=GlobalSearchCase;
  LastSearchWholeWords=GlobalSearchWholeWords;
  LastSearchReverse=GlobalSearchReverse;
#endif

}

Help::~Help()
{
  CtrlObject->Macro.SetMode(PrevMacroMode);
  SetRestoreScreenMode(FALSE);

  if(HelpData)     xf_free(HelpData);
  if(Stack)        delete Stack;
  if(TopScreen)    delete TopScreen;

#if defined(WORK_HELP_FIND)
  KeepInitParameters();
#endif
}


#if defined(WORK_HELP_FIND)
void Help::KeepInitParameters()
{
  strcpy(GlobalSearchString,(char *)LastSearchStr);
  LastSearchPos=0;
  GlobalSearchCase=LastSearchCase;
  GlobalSearchWholeWords=LastSearchWholeWords;
  GlobalSearchReverse=LastSearchReverse;
}
#endif


void Help::Hide()
{
  ScreenObject::Hide();
}


int Help::ReadHelp(const wchar_t *Mask)
{
  string strFileName;
  wchar_t ReadStr[2*MAX_HELP_STRING_LENGTH];
  wchar_t SplitLine[2*MAX_HELP_STRING_LENGTH+8],*Ptr;
  int Formatting=TRUE,RepeatLastLine,PosTab,BreakProcess;
  const int MaxLength=X2-X1-1;
  wchar_t TabSpace[32]; //BUGBUG

  string strPath;

  wchar_t *TopicPtr;
  if ( StackData.strHelpTopic.At (0)==HelpBeginLink)
  {
    strPath = (const wchar_t*)StackData.strHelpTopic+1;

    wchar_t *lpwszPath = strPath.GetBuffer();

    if ((TopicPtr=wcschr(lpwszPath,HelpEndLink))==NULL)
      return FALSE;
    StackData.strHelpTopic = TopicPtr+1;
    *TopicPtr=0;
    StackData.strHelpPath = lpwszPath;

    strPath.ReleaseBuffer();
  }
  else
    strPath = !StackData.strHelpPath.IsEmpty() ? StackData.strHelpPath:g_strFarPath;

  if (!wcscmp(StackData.strHelpTopic,PluginContents))
  {
    ReadDocumentsHelp(HIDX_PLUGINS);
    return TRUE;
  }

#if defined(WORK_HELP_DOCUMS)
  if (!strcmp(StackData.HelpTopic,DocumentContents))
  {
    ReadDocumentsHelp(HIDX_DOCUMS);
    return TRUE;
  }
#endif

  int nType = TYPE_ANSI;

  FILE *HelpFile=Language::OpenLangFile(strPath,(!*Mask?HelpFileMask:Mask),Opt.strHelpLanguage,strFileName, nType);

  if (HelpFile==NULL)
  {
    ErrorHelp=TRUE;
    if(!ScreenObject::Flags.Check(FHELPOBJ_ERRCANNOTOPENHELP))
    {
      ScreenObject::Flags.Set(FHELPOBJ_ERRCANNOTOPENHELP);
      if(!(StackData.Flags&FHELP_NOSHOWERROR))
      {
        BlockExtKey blockExtKey;
        MessageW(MSG_WARNING,1,UMSG(MHelpTitle),UMSG(MCannotOpenHelp),Mask,UMSG(MOk));
      }
    }
    return FALSE;
  }

  string strReadStr;

  if (Language::GetOptionsParam(HelpFile,L"TabSize",strReadStr, nType))
  {
    CtrlTabSize=_wtoi(strReadStr);
  }
  if(CtrlTabSize < 0 || CtrlTabSize > 16)
    CtrlTabSize=Opt.HelpTabSize;

  if (Language::GetOptionsParam(HelpFile,L"CtrlColorChar",strReadStr, nType))
    strCtrlColorChar = strReadStr;
  else
    strCtrlColorChar=L"";

  if (Language::GetOptionsParam(HelpFile,L"CtrlStartPosChar",strReadStr, nType))
    strCtrlStartPosChar = strReadStr;
  else
    strCtrlStartPosChar = L"";


  /* $ 29.11.2001 DJ
     ��������, ���� ��� �������� � PluginContents
  */
  if (!Language::GetLangParam (HelpFile,L"PluginContents",&strCurPluginContents, NULL, nType))
    strCurPluginContents = L"";
  /* DJ $ */

  *SplitLine=0;
  if (HelpData)
    xf_free(HelpData);
  HelpData=NULL;
  StrCount=0;
  FixCount=0;
  TopicFound=0;
  RepeatLastLine=FALSE;
  BreakProcess=FALSE;
  int NearTopicFound=0;
  wchar_t PrevSymbol=0;

  for (int i = 0; i < sizeof(TabSpace)/sizeof (wchar_t); i++)
    TabSpace[i] = L' ';

  TabSpace[(sizeof(TabSpace)-1)/sizeof (wchar_t)]=0;

  StartPos = -1;
  LastStartPos = -1;

  int RealMaxLength;

  while (TRUE)
  {
    if ( StartPos != -1 )
      RealMaxLength = MaxLength-StartPos;
    else
      RealMaxLength = MaxLength;

    if (!RepeatLastLine && !BreakProcess && ReadString(HelpFile, ReadStr,sizeof(ReadStr)/2/sizeof(wchar_t), nType)==NULL)
    {
      if (StringLen(SplitLine)<MaxLength)
      {
        if (*SplitLine)
          AddLine(SplitLine);
      }
      else
      {
        *ReadStr=0;
        RepeatLastLine=TRUE;
        continue;
      }
      break;
    }
    RepeatLastLine=FALSE;

    while((Ptr=wcschr(ReadStr,L'\t')) != NULL)
    {
      *Ptr=L' ';
      PosTab=Ptr-ReadStr+1;
      if(CtrlTabSize > 1) // ������� ��������� �� ���� ���������
        InsertStringW(ReadStr,PosTab,TabSpace, CtrlTabSize - (PosTab % CtrlTabSize));
      if(wcslen(ReadStr) > sizeof(ReadStr)/2/sizeof (wchar_t))
        break;
    }

    RemoveTrailingSpacesW(ReadStr);

    if ( !strCtrlStartPosChar.IsEmpty() && wcsstr (ReadStr, strCtrlStartPosChar) )
    {
        wchar_t Line[MAX_HELP_STRING_LENGTH];
        int Length = wcsstr (ReadStr, strCtrlStartPosChar)-ReadStr;

        xwcsncpy (Line, ReadStr, Length);
        LastStartPos = StringLen (Line);

        wcscpy (ReadStr+Length, ReadStr+Length+strCtrlStartPosChar.GetLength());
    }

    if (TopicFound)
    {
      HighlightsCorrection(ReadStr);
    }

    if (*ReadStr==L'@' && !BreakProcess)
    {
      if (TopicFound)
      {
        if (wcscmp(ReadStr,L"@+")==0)
        {
          Formatting=TRUE;
          PrevSymbol=0;
          continue;
        }
        if (wcscmp(ReadStr,L"@-")==0)
        {
          Formatting=FALSE;
          PrevSymbol=0;
          continue;
        }
        if (*SplitLine)
        {
          BreakProcess=TRUE;
          *ReadStr=0;
          PrevSymbol=0;
          goto m1;
        }
        break;
      }
      else
        if (LocalStricmpW(ReadStr+1,StackData.strHelpTopic)==0)
        {
          TopicFound=1;
          NearTopicFound=1;
        }
    }
    else
    {
m1:
      if(!*ReadStr && BreakProcess && !*SplitLine)
        break;
      if (TopicFound)
      {
        /* $<text> � ������ ������, ����������� ����
           ���������� �� �������������� ������� ������
           ���� ���� ��������� ������ ����� ����� ������ ����������� ����...
        */
        if ( NearTopicFound )
        {
          StartPos = -1;
          LastStartPos = -1;
        }


        if (*ReadStr=='$' && NearTopicFound && (PrevSymbol == L'$' || PrevSymbol == L'@'))
        {
          AddLine(ReadStr+1);
          FixCount++;
        }
        else
        {
          NearTopicFound=0;
          if (*ReadStr==0 || !Formatting)
          {
            if (*SplitLine)
            {
              if (StringLen(SplitLine)<RealMaxLength)
              {
                AddLine(SplitLine);

                *SplitLine=0;
                if (StringLen(ReadStr)<RealMaxLength)
                {
                  AddLine(ReadStr);

                  LastStartPos = -1;
                  StartPos = -1;

                  continue;
                }
              }
              else
                RepeatLastLine=TRUE;
            }
            else if(*ReadStr)
            {
              if (StringLen(ReadStr)<RealMaxLength)
              {
                AddLine(ReadStr);
                continue;
              }
            }
            else if(!*ReadStr && !*SplitLine)
            {
              AddLine(L"");
              continue;
            }
          }

          if (*ReadStr && IsSpaceW(*ReadStr) && Formatting)
          {
            if (StringLen(SplitLine)<RealMaxLength)
            {
              if (*SplitLine)
              {
                AddLine(SplitLine);
                StartPos = -1;
              }

              wcscpy(SplitLine,ReadStr);

              *ReadStr=0;
              continue;
            }
            else
              RepeatLastLine=TRUE;
          }

          if (!RepeatLastLine)
          {
            if (*SplitLine)
              wcscat(SplitLine,L" ");
            wcscat(SplitLine,ReadStr);
          }

          if (StringLen(SplitLine)<RealMaxLength)
          {
            if(!*ReadStr && BreakProcess)
              goto m1;
            continue;
          }

          int Splitted=0;

          for (int I=wcslen(SplitLine)-1;I>0;I--)
          {
            if (I>0 && SplitLine[I]==L'~' && SplitLine[I-1]==L'~')
            {
              I--;
              continue;
            }
            if (I>0 && SplitLine[I]==L'~' && SplitLine[I-1]!=L'~')
            {
              do {
                I--;
              } while (I>0 && SplitLine[I]!=L'~');
              continue;
            }
            if (SplitLine[I]==L' ')
            {
              SplitLine[I]=0;
              if (StringLen(SplitLine)<RealMaxLength)
              {
                AddLine(SplitLine);
                memmove(SplitLine+1,SplitLine+I+1,(wcslen(SplitLine+I+1)+1)*sizeof(wchar_t));
                *SplitLine=L' ';
                HighlightsCorrection(SplitLine);
                Splitted=TRUE;
                break;
              }
              else
                SplitLine[I]=L' ';
            }
          }
          if (!Splitted)
          {
            AddLine(SplitLine);
            *SplitLine=0;
          }
          else
            StartPos = LastStartPos;
        }
      }
      if(BreakProcess)
      {
        if(*SplitLine)
          goto m1;
        break;
      }
    }
    PrevSymbol=*ReadStr;
  }
  AddLine(L"");

  fclose(HelpFile);
  FixSize=FixCount+(FixCount!=0);
  ErrorHelp=FALSE;
  if(IsNewTopic)
  {
    StackData.CurX=StackData.CurY=0;
    StackData.TopStr=0;
  }
  return TopicFound != 0;
}


void Help::AddLine(const wchar_t *Line)
{
  wchar_t *NewHelpData=(wchar_t *)xf_realloc(HelpData,(StrCount+1)*MAX_HELP_STRING_LENGTH*sizeof (wchar_t));
  if (NewHelpData==NULL)
    return;
  HelpData=NewHelpData;

  wchar_t *HelpStr = HelpData+StrCount*MAX_HELP_STRING_LENGTH;

  wmemset (HelpStr, 0, MAX_HELP_STRING_LENGTH);

  if ( StartPos != 0xFFFFFFFF )
  {
    for (DWORD i = 0; i < StartPos; i++)
      HelpStr[i] = L' ';

    xwcsncpy(HelpStr+StartPos, (*Line == L' ')?Line+1:Line, MAX_HELP_STRING_LENGTH-1);
  }
  else
    xwcsncpy(HelpStr, Line, MAX_HELP_STRING_LENGTH-1);

  StrCount++;
}

void Help::AddTitle(const wchar_t *Title)
{
  string strIndexHelpTitle;
  strIndexHelpTitle.Format (L"^ #%s#", Title);
  AddLine(strIndexHelpTitle);
}

void Help::HighlightsCorrection(wchar_t *Str)
{
  int I,Count;
  for (I=0,Count=0;Str[I]!=0;I++)
    if (Str[I]==L'#')
      Count++;
  if ((Count & 1) && *Str!=L'$')
  {
    memmove(Str+1,Str,(wcslen(Str)+1)*sizeof (wchar_t));
    *Str=L'#';
  }
}


void Help::DisplayObject()
{
  if(!TopScreen)
    TopScreen=new SaveScreen;
  if (!TopicFound)
  {
    if(!ErrorHelp) // ���� ��� ������, �� ��� �������������� ������
    {              // � �������� ��������� �������� � ����������� ����.
      ErrorHelp=TRUE;
      if(!(StackData.Flags&FHELP_NOSHOWERROR))
      {
        BlockExtKey blockExtKey;
        MessageW(MSG_WARNING,1,UMSG(MHelpTitle),UMSG(MHelpTopicNotFound),StackData.strHelpTopic,UMSG(MOk));
      }
      ProcessKey(KEY_ALTF1);
    }
    return;
  }
  SetCursorType(0,10);
  if ( StackData.strSelTopic.IsEmpty() )
    MoveToReference(1,1);
  FastShow();
  if (!Opt.FullScreenHelp)
  {
    HelpKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
    if(Opt.ShowKeyBar)
       HelpKeyBar.Show();
  }
  else
    HelpKeyBar.Hide();
}


void Help::FastShow()
{
  int I;

  /* $ 29.11.2001 DJ
     ��������� ����� -> � ��������� �������
  */
  if (!Locked())
    DrawWindowFrame();
  /* DJ $ */

  CorrectPosition();
  StackData.strSelTopic=L"";
  /* $ 01.09.2000 SVS
     ��������� �� ��������� ������� ���� ���������...
     ����� ����� ���� ���������� � ����������� ����������
  */
  CurColor=COL_HELPTEXT;
  /* SVS $ */
  for (I=0;I<Y2-Y1-1;I++)
  {
    int StrPos;
    if (I<FixCount)
      StrPos=I;
    else
      if (I==FixCount && FixCount>0)
      {
        if (!Locked())
        {
          GotoXY(X1,Y1+I+1);
          SetColor(COL_HELPBOX);
          ShowSeparator(X2-X1+1,1);
        }
        continue;
      }
      else
      {
        StrPos=I+StackData.TopStr;
        if (FixCount>0)
          StrPos--;
      }
    if (StrPos<StrCount)
    {
      wchar_t *OutStr=HelpData+StrPos*MAX_HELP_STRING_LENGTH;
      if (*OutStr==L'^')
      {
        GotoXY(X1+(X2-X1+1-StringLen(OutStr))/2,Y1+I+1);
        OutStr++;
      }
      else
        GotoXY(X1+1,Y1+I+1);
      OutString(OutStr);
    }
  }

  const int ScrollLength=Y2-Y1-FixSize-1;
  if (!Locked() && StrCount-FixCount > ScrollLength)
  {
    int Scrolled=StrCount-FixCount-ScrollLength;
    SetColor(COL_HELPSCROLLBAR);
    ScrollBar(X2,Y1+FixSize+1,ScrollLength,StackData.TopStr,Scrolled);
  }
}

/* $ 29.11.2001 DJ
   �������� �� FastShow; �������� ����� ����, ��� � ��� ����
*/

void Help::DrawWindowFrame()
{
  SetScreen(X1,Y1,X2,Y2,L' ',COL_HELPTEXT);
  Box(X1,Y1,X2,Y2,COL_HELPBOX,DOUBLE_BOX);
  SetColor(COL_HELPBOXTITLE);

  string strHelpTitleBuf;
  strHelpTitleBuf = UMSG(MHelpTitle);
  strHelpTitleBuf += L" - ";
  if ( !strCurPluginContents.IsEmpty() )
    strHelpTitleBuf += strCurPluginContents;
  else
    strHelpTitleBuf += L"FAR";
  /* $ 03.12.2001 DJ
     ������� ������� ���������
  */
  TruncStrFromEndW(strHelpTitleBuf,X2-X1-3);
  /* DJ $ */
  GotoXY(X1+(X2-X1+1-strHelpTitleBuf.GetLength()-2)/2,Y1);
  mprintfW(L" %s ", (const wchar_t*)strHelpTitleBuf);
}

/* DJ $ */

/* $ 01.09.2000 SVS
  ����� ������ CtrlColorChar & CurColor
*/
void Help::OutString(const wchar_t *Str)
{
  wchar_t OutStr[512]; //BUGBUG
  const wchar_t *StartTopic=NULL;
  int OutPos=0,Highlight=0,Topic=0;
  while (OutPos<sizeof(OutStr)/sizeof(wchar_t)-10)
  {
    if (Str[0]==L'~' && Str[1]==L'~' ||
        Str[0]==L'#' && Str[1]==L'#' ||
        Str[0]==L'@' && Str[1]==L'@' ||
        ( !strCtrlColorChar.IsEmpty() && Str[0]==strCtrlColorChar.At(0) && Str[1]==strCtrlColorChar.At(0))
       )
    {
      OutStr[OutPos++]=*Str;
      Str+=2;
      continue;
    }

    if (*Str==L'~' || ((*Str==L'#' || *Str == strCtrlColorChar.At(0)) && !Topic) /*|| *Str==HelpBeginLink*/ || *Str==0)
    {
      OutStr[OutPos]=0;
      if (Topic)
      {
        int RealCurX,RealCurY;
        RealCurX=X1+StackData.CurX+1;
        RealCurY=Y1+StackData.CurY+FixSize+1;
        if (WhereY()==RealCurY && RealCurX>=WhereX() &&
                RealCurX<WhereX()+(Str-StartTopic)-1)
        {
          SetColor(COL_HELPSELECTEDTOPIC);
          if (Str[1]==L'@')
          {
            StackData.strSelTopic = (Str+2);

            wchar_t *EndPtr = StackData.strSelTopic.GetBuffer (StackData.strSelTopic.GetLength()*2);
            EndPtr=wcschr(EndPtr,L'@');
            /* $ 25.08.2000 SVS
               �����, ��� ����� ���� ����� �������: @@ ��� \@
               ���� ������� ������ ��� URL!
            */
            if (EndPtr!=NULL)
            {
              if(*(EndPtr+1) == L'@')
              {
                memmove(EndPtr,EndPtr+1,(wcslen(EndPtr)+1)*sizeof (wchar_t));
                EndPtr++;
              }
              EndPtr=wcschr(EndPtr,L'@');
              if (EndPtr!=NULL)
                *EndPtr=0;
            }

            StackData.strSelTopic.ReleaseBuffer ();
            /* SVS $ */
          }
        }
        else
          SetColor(COL_HELPTOPIC);
      }
      else
        if (Highlight)
          SetColor(COL_HELPHIGHLIGHTTEXT);
        else
          SetColor(CurColor);
      /* $ 24.09.2001 VVM
        ! ������� ������� ������ ��� ������. ����� ����� ������ ��� ������� �������... */
      if (static_cast<int>(wcslen(OutStr) + WhereX()) > X2)
        OutStr[X2 - WhereX()] = 0;
      /* VVM $ */
      if (Locked())
        GotoXY(WhereX()+wcslen(OutStr),WhereY());
      else
        TextW(OutStr);
      OutPos=0;
    }

    if (*Str==0)
      break;

    if (*Str==L'~')
    {
      if (!Topic)
        StartTopic=Str;
      Topic=!Topic;
      Str++;
      continue;
    }
    if (*Str==L'@')
    {
      /* $ 25.08.2000 SVS
         �����, ��� ����� ���� ����� �������: @@
         ���� ������� ������ ��� URL!
      */
      while (*Str)
        if (*(++Str)==L'@' && *(Str-1)!=L'@')
          break;
      /* SVS $ */
      Str++;
      continue;
    }
    if (*Str==L'#')
    {
      Highlight=!Highlight;
      Str++;
      continue;
    }
    if (*Str == strCtrlColorChar.At(0))
    {
      WORD Chr;

      Chr=(BYTE)Str[1];
      if(Chr == L'-') // "\-" - ���������� ���������� ����
      {
        Str+=2;
        CurColor=COL_HELPTEXT;
        continue;
      }
      if(iswxdigit(Chr) && iswxdigit(Str[2]))
      {
        WORD Attr;

        if(Chr >= L'0' && Chr <= L'9') Chr-=L'0';
        else { Chr&=~0x20; Chr=Chr-L'A'+10; }
        Attr=(Chr<<4)&0x00F0;

        // next char
        Chr=Str[2];
        if(Chr >= L'0' && Chr <= L'9') Chr-=L'0';
        else { Chr&=~0x20; Chr=Chr-L'A'+10; }
        Attr|=(Chr&0x000F);
        CurColor=Attr;
        Str+=3;
        continue;
      }
    }

    OutStr[OutPos++]=*(Str++);
  }
  if (!Locked() && WhereX()<X2)
  {
    SetColor(CurColor);
    mprintfW(L"%*s",X2-WhereX(),L"");
  }
}


int Help::StringLen(const wchar_t *Str)
{
  int Length=0;
  while (*Str)
  {
    if (Str[0]==L'~' && Str[1]==L'~' ||
        Str[0]==L'#' && Str[1]==L'#' ||
        Str[0]==L'@' && Str[1]==L'@' ||
        ( !strCtrlColorChar.IsEmpty() && Str[0]==strCtrlColorChar.At(0) && Str[1]==strCtrlColorChar.At(0))
       )
    {
      Length++;
      Str+=2;
      continue;
    }
    if (*Str==L'@')
    {
      /* $ 25.08.2000 SVS
         �����, ��� ����� ���� ����� �������: @@
         ���� ������� ������ ��� URL!
      */
      while (*Str)
        if (*(++Str)==L'@' && *(Str-1)!=L'@')
          break;
      /* SVS $ */
      Str++;
      continue;
    }
    /* $ 01.09.2000 SVS
       ����� ���� ������������ \XX ��� \-
    */
    if(*Str == strCtrlColorChar.At(0))
    {
      if(Str[1] == L'-')
      {
        Str+=2;
        continue;
      }

      if(iswxdigit(Str[1]) && iswxdigit(Str[2]))
      {
        Str+=3;
        continue;
      }
    }
    /* SVS $ */

    if (*Str!=L'#' && *Str!=L'~')
      Length++;
    Str++;
  }
  return(Length);
}


void Help::CorrectPosition()
{
  if (StackData.CurX>X2-X1-2)
    StackData.CurX=X2-X1-2;
  if (StackData.CurX<0)
    StackData.CurX=0;
  if (StackData.CurY>Y2-Y1-2-FixSize)
  {
    StackData.TopStr+=StackData.CurY-(Y2-Y1-2-FixSize);
    StackData.CurY=Y2-Y1-2-FixSize;
  }
  if (StackData.CurY<0)
  {
    StackData.TopStr+=StackData.CurY;
    StackData.CurY=0;
  }
  if (StackData.TopStr>StrCount-FixCount-(Y2-Y1-1-FixSize))
    StackData.TopStr=StrCount-FixCount-(Y2-Y1-1-FixSize);
  if (StackData.TopStr<0)
    StackData.TopStr=0;
}

#if defined(WORK_HELP_FIND)
/* SVS:
   ��������!!!!!
   ��� ������� ������ ����� (����� �������!)
   ���� � ���� ������ ��� ������� �� ��������� - ������� ������ ;-)
*/
int Help::Search(int Next)
{
  unsigned char SearchStr[SEARCHSTRINGBUFSIZE]
  char MsgStr[512];
  int SearchLength,Case,WholeWords,ReverseSearch,Match;

  if (Next && *LastSearchStr==0)
    return TRUE;

  Match=FALSE;
  xstrncpy((char *)SearchStr,(char *)LastSearchStr,sizeof(SearchStr));
  Case=LastSearchCase;
  WholeWords=LastSearchWholeWords;
  ReverseSearch=LastSearchReverse;

  if (!Next)
    if(!GetSearchReplaceString(FALSE,SearchStr,sizeof(SearchStr),
                   NULL,0,NULL,NULL,
                   NULL/*&Case*/,NULL/*&WholeWords*/,NULL/*&ReverseSearch*/))
      return FALSE;

  xstrncpy((char *)LastSearchStr,(char *)SearchStr,sizeof(LastSearchStr));
  LastSearchCase=Case;
  LastSearchWholeWords=WholeWords;
  LastSearchReverse=ReverseSearch;

  if ((SearchLength=strlen((char *)SearchStr))==0)
    return TRUE;
  else
  {
    SaveScreen SaveScr;
    SetCursorType(FALSE,0);
    char Buf[8192];

    sprintf(MsgStr,"\"%s\"",SearchStr);
    Message(0,0,MSG(MHelpSearchTitle),MSG(MHelpSearchingFor),MsgStr);

    if (!Case)
      for (int I=0;I<SearchLength;I++)
        SearchStr[I]=LocalUpperW(SearchStr[I]);
/*
    //if(!ReadHelp(HelpMask))
    //fseek(ViewFile,LastSearchPos,SEEK_SET);

    while (!Match)
    {

    }
*/
  }
  if (!Match)
  {
    Message(MSG_DOWN|MSG_WARNING,1,MSG(MHelpFindTitle),
              MSG(MHelpSearchCannotFind),MsgStr,MSG(MOk));
    return FALSE;
  }

  // ������ � ���������� ������.
  return TRUE;
}
#endif

int Help::ProcessKey(int Key)
{
  if ( StackData.strSelTopic.IsEmpty() )
    StackData.CurX=StackData.CurY=0;
  switch(Key)
  {
    case KEY_NONE:
    case KEY_IDLE:
    {
      break;
    }

    case KEY_F5:
    {
      Opt.FullScreenHelp=!Opt.FullScreenHelp;
      ResizeConsole();
      return(TRUE);
    }

    case KEY_ESC:
    case KEY_F10:
    {
      FrameManager->DeleteFrame();
      SetExitCode (XC_QUIT);
      return(TRUE);
    }

    case KEY_HOME:        case KEY_NUMPAD7:
    case KEY_CTRLHOME:    case KEY_CTRLNUMPAD7:
    case KEY_CTRLPGUP:    case KEY_CTRLNUMPAD9:
    {
      StackData.CurX=StackData.CurY=0;
      StackData.TopStr=0;
      FastShow();
      if ( StackData.strSelTopic.IsEmpty() )
        MoveToReference(1,1);
      return(TRUE);
    }

    case KEY_END:         case KEY_NUMPAD1:
    case KEY_CTRLEND:     case KEY_CTRLNUMPAD1:
    case KEY_CTRLPGDN:    case KEY_CTRLNUMPAD3:
    {
      StackData.CurX=StackData.CurY=0;
      StackData.TopStr=StrCount;
      FastShow();
      if ( StackData.strSelTopic.IsEmpty() )
      {
        StackData.CurX=0;
        StackData.CurY=Y2-Y1-2-FixSize;
        MoveToReference(0,1);
      }
      return(TRUE);
    }

    case KEY_UP:          case KEY_NUMPAD8:
    {
      if (StackData.TopStr>0)
      {
        StackData.TopStr--;
        if (StackData.CurY<Y2-Y1-2-FixSize)
        {
          StackData.CurX=X2-X1-2;
          StackData.CurY++;
        }
        FastShow();
        if (StackData.strSelTopic.IsEmpty())
          MoveToReference(0,1);
      }
      else
        ProcessKey(KEY_SHIFTTAB);
      return(TRUE);
    }

    case KEY_DOWN:        case KEY_NUMPAD2:
    {
      if (StackData.TopStr<StrCount-FixCount-(Y2-Y1-1-FixSize))
      {
        StackData.TopStr++;
        if (StackData.CurY>0)
          StackData.CurY--;
        StackData.CurX=0;
        FastShow();
        if (StackData.strSelTopic.IsEmpty())
          MoveToReference(1,1);
      }
      else
        ProcessKey(KEY_TAB);
      return(TRUE);
    }

    /* $ 26.07.2001 VVM
      + � ������ ������� �� 1 */
    /* $ 07.05.2001 DJ
      + ��������� KEY_MSWHEEL_XXXX */
    case KEY_MSWHEEL_UP:
    case (KEY_MSWHEEL_UP | KEY_ALT):
    {
      int Roll = Key & KEY_ALT?1:Opt.MsWheelDeltaHelp;
      for (int i=0; i<Roll; i++)
        ProcessKey(KEY_UP);
      return(TRUE);
    }
    case KEY_MSWHEEL_DOWN:
    case (KEY_MSWHEEL_DOWN | KEY_ALT):
    {
      int Roll = Key & KEY_ALT?1:Opt.MsWheelDeltaHelp;
      for (int i=0; i<Roll; i++)
        ProcessKey(KEY_DOWN);
      return(TRUE);
    }
    /* DJ $ */
    /* VVM $ */

    case KEY_PGUP:      case KEY_NUMPAD9:
    {
      StackData.CurX=StackData.CurY=0;
      StackData.TopStr-=Y2-Y1-2-FixSize;
      FastShow();
      if (StackData.strSelTopic.IsEmpty())
      {
        StackData.CurX=StackData.CurY=0;
        MoveToReference(1,1);
      }
      return(TRUE);
    }

    case KEY_PGDN:      case KEY_NUMPAD3:
    {
      {
        int PrevTopStr=StackData.TopStr;
        StackData.TopStr+=Y2-Y1-2-FixSize;
        FastShow();
        if (StackData.TopStr==PrevTopStr)
        {
          ProcessKey(KEY_CTRLPGDN);
          return(TRUE);
        }
        else
          StackData.CurX=StackData.CurY=0;
        MoveToReference(1,1);
      }
      return(TRUE);
    }

    case KEY_RIGHT:   case KEY_NUMPAD6:
    case KEY_TAB:
    {
      MoveToReference(1,0);
      return(TRUE);
    }

    case KEY_LEFT:    case KEY_NUMPAD4:
    case KEY_SHIFTTAB:
    {
      MoveToReference(0,0);
      return(TRUE);
    }

    case KEY_F1:
    {
      // �� ������� SelTopic, ���� � ��� � Help on Help
      if(LocalStricmpW(StackData.strHelpTopic,HelpOnHelpTopic)!=0)
      {
        Stack->Push(&StackData);
        IsNewTopic=TRUE;
        JumpTopic(HelpOnHelpTopic);
        IsNewTopic=FALSE;
        ErrorHelp=FALSE;
      }
      return(TRUE);
    }

    case KEY_SHIFTF1:
    {
      //   �� ������� SelTopic, ���� � ��� � ���� Contents
      if(LocalStricmpW(StackData.strHelpTopic,HelpContents)!=0)
      {
        Stack->Push(&StackData);
        IsNewTopic=TRUE;
        JumpTopic(HelpContents);
        ErrorHelp=FALSE;
        IsNewTopic=FALSE;
      }
      return(TRUE);
    }

    case KEY_SHIFTF2:
    {
      //   �� ������� SelTopic, ���� � ��� � PluginContents
      if(LocalStricmpW(StackData.strHelpTopic,PluginContents)!=0)
      {
        Stack->Push(&StackData);
        IsNewTopic=TRUE;
        JumpTopic(PluginContents);
        ErrorHelp=FALSE;
        IsNewTopic=FALSE;
      }
      return(TRUE);
    }

#if defined(WORK_HELP_DOCUMS)
    case KEY_SHIFTF3: // ��� "����������" :-)
    {
      //   �� ������� SelTopic, ���� � ��� � DocumentContents
      if(LocalStricmpW(StackData.strHelpTopic,DocumentContents)!=0)
      {
        Stack->Push(&StackData);
        IsNewTopic=TRUE;
        JumpTopic(DocumentContents);
        ErrorHelp=FALSE;
        IsNewTopic=FALSE;
      }
      return(TRUE);
    }
#endif

    case KEY_ALTF1:
    case KEY_BS:
    {
      // ���� ���� �������� ���� - ������� �� �����
      if(!Stack->isEmpty())
      {
        Stack->Pop(&StackData);
        JumpTopic(StackData.strHelpTopic);
        ErrorHelp=FALSE;
        return(TRUE);
      }
      return ProcessKey(KEY_ESC);
    }

    case KEY_ENTER:
    {
      if ( !StackData.strSelTopic.IsEmpty() && LocalStricmpW(StackData.strHelpTopic,StackData.strSelTopic)!=0)
      {
        Stack->Push(&StackData);
        IsNewTopic=TRUE;
        if (!JumpTopic())
        {
          Stack->Pop(&StackData);
          ReadHelp(StackData.strHelpMask); // ������ ��, ��� ����������.
        }
        ErrorHelp=FALSE;
        IsNewTopic=FALSE;
      }
      return(TRUE);
    }

#if defined(WORK_HELP_FIND)
    case KEY_F7:
    {
      Search(0);
      return(TRUE);
    }

    case KEY_SHIFTF7:
    {
      Search(1);
      return(TRUE);
    }
#endif

  }
  return(FALSE);
}

int Help::JumpTopic(const wchar_t *JumpTopic)
{
  string strNewTopic;
  wchar_t *p;

  Stack->PrintStack(JumpTopic);

  if(JumpTopic)
    StackData.strSelTopic = JumpTopic;
  /* $ 14.07.2002 IS
       ��� �������� �� ������� ���������� ������ ������ ���������� ����,
       ���� ��� ��������.
  */
  // ���� ������ �� ������ ����, ���� ������������� � ���� ��, �� ���� �����
  // ��������� ���������� ����, �� ������� ���
  if( StackData.strSelTopic.At(0)==HelpBeginLink &&
     NULL!=(wcschr((const wchar_t*)StackData.strSelTopic+2,HelpEndLink))&&
     !PathMayBeAbsoluteW((const wchar_t*)StackData.strSelTopic+1) &&
     !StackData.strHelpPath.IsEmpty())
  {

    string strFullPath;

    p = wcschr ((const wchar_t*)StackData.strSelTopic+2, HelpEndLink);

    wchar_t *lpwszHelpTopic = strNewTopic.GetBuffer((p-(const wchar_t*)StackData.strSelTopic-1)*sizeof (wchar_t));

    xwcsncpy(lpwszHelpTopic, (const wchar_t*)StackData.strSelTopic+1,(p-(const wchar_t*)StackData.strSelTopic-1)*sizeof(wchar_t));

    strNewTopic.ReleaseBuffer();

    strFullPath = StackData.strHelpPath;

    wchar_t *lpwszFullPath = strFullPath.GetBuffer();
    // ������ _���_ �������� ����� � ������� ����
    int Len=wcslen(lpwszFullPath)-1;
    while(Len>-1 && lpwszFullPath[Len]==L'\\')
    {
      lpwszFullPath[Len]=0;
      --Len;
    }
    if(Len<0)
      Len=0;
    else
      ++Len;
    lpwszFullPath[Len]=L'\\';
    lpwszFullPath[Len+1]=0;

    strFullPath.ReleaseBuffer();

    strFullPath += (const wchar_t*)strNewTopic+((strNewTopic.At(0)==L'\\' || strNewTopic.At(0)==L'/')?1:0);
    BOOL addSlash=DeleteEndSlashW(strFullPath);

    ConvertNameToFullW(strFullPath,strNewTopic);

    strFullPath.Format (addSlash?HelpFormatLink:HelpFormatLinkModule, (const wchar_t*)strNewTopic, wcschr ((const wchar_t*)StackData.strSelTopic+2, HelpEndLink)+1);
    StackData.strSelTopic = strFullPath;
  }
  /* IS 14.07.2002 $ */
//_SVS(SysLog("JumpTopic() = SelTopic=%s",StackData.SelTopic));
  // URL ��������� - ��� ���� ��� ������ :-)))
  {
    strNewTopic = StackData.strSelTopic;

    wchar_t *lpwszNewTopic = strNewTopic.GetBuffer ();

    p=wcschr(lpwszNewTopic,L':');
    if(p && strNewTopic.At(0) != L':') // �������� ��������������� URL
    {
      *p=0;

      wchar_t *lpwszTopic = StackData.strSelTopic.GetBuffer ();

      if(RunURL(lpwszNewTopic, lpwszTopic))
      {
          StackData.strSelTopic.ReleaseBuffer ();

        return(FALSE);
      }
      else
          StackData.strSelTopic.ReleaseBuffer ();
      *p=':';
    }

    strNewTopic.ReleaseBuffer ();
  }
  // � ��� ������ ���������...

//_SVS(SysLog("JumpTopic() = SelTopic=%s, StackData.HelpPath=%s",StackData.SelTopic,StackData.HelpPath));
  if ( !StackData.strHelpPath.IsEmpty() && StackData.strSelTopic.At(0) !=HelpBeginLink && wcscmp(StackData.strSelTopic,HelpOnHelpTopic)!=0)
  {
    if ( StackData.strSelTopic.At(0)==L':')
    {
      strNewTopic = (const wchar_t*)StackData.strSelTopic+1;
      StackData.Flags&=~FHELP_CUSTOMFILE;
    }
    else if(StackData.Flags&FHELP_CUSTOMFILE)
      strNewTopic = StackData.strSelTopic;
    else
      strNewTopic.Format (HelpFormatLink,(const wchar_t*)StackData.strHelpPath,(const wchar_t*)StackData.strSelTopic);
  }
  else
  {
    strNewTopic = (const wchar_t*)StackData.strSelTopic+(!wcscmp(StackData.strSelTopic,HelpOnHelpTopic)?1:0);
  }

  // ������ ������ �� .DLL

  wchar_t *lpwszNewTopic = strNewTopic.GetBuffer();

  p=wcsrchr(lpwszNewTopic,HelpEndLink);
  if(p)
  {
    if(*(p-1) != L'\\')
    {
      wchar_t *p2=p;
      while(p >= lpwszNewTopic)
      {
        if(*p == L'\\')
        {
//          ++p;
          if(*p)
          {
            StackData.Flags|=FHELP_CUSTOMFILE;
            StackData.strHelpMask = p+1;

            wchar_t *lpwszMask = StackData.strHelpMask.GetBuffer ();

            *wcsrchr(lpwszMask,HelpEndLink)=0;

            StackData.strHelpMask.ReleaseBuffer ();
          }
          memmove(p,p2,(wcslen(p2)+1)*sizeof(wchar_t));
          p=wcsrchr(StackData.strHelpMask,L'.');
          if(p && LocalStricmpW(p,L".hlf"))
            StackData.strHelpMask=L"";
          break;
        }
        --p;
      }
    }
    else
    {
      StackData.Flags&=~FHELP_CUSTOMFILE;
      StackData.Flags|=FHELP_CUSTOMPATH;
    }
  }

  strNewTopic.ReleaseBuffer();

//_SVS(SysLog("HelpMask=%s NewTopic=%s",StackData.HelpMask,NewTopic));
  if( StackData.strSelTopic.At(0) != L':' &&
     LocalStricmpW(StackData.strSelTopic,PluginContents)
#if defined(WORK_HELP_DOCUMS)
     && LocalStricmpW(StackData.strSelTopic,DocumentContents)
#endif
    )
  {
    if(!(StackData.Flags&FHELP_CUSTOMFILE) && wcsrchr(strNewTopic,HelpEndLink))
    {
       StackData.strHelpMask=L"";
    }
  }
  else
  {
      StackData.strHelpMask=L"";
  }
  StackData.strHelpTopic = strNewTopic;
  if(!(StackData.Flags&FHELP_CUSTOMFILE))
    StackData.strHelpPath=L"";
  if(!ReadHelp(StackData.strHelpMask))
  {
    StackData.strHelpTopic = strNewTopic;
    if( StackData.strHelpTopic.At(0) == HelpBeginLink)
    {
        wchar_t *Ptr=StackData.strHelpTopic.GetBuffer ();

        Ptr = wcsrchr(Ptr,HelpEndLink);

        if ( Ptr )
        {
            *(Ptr++) = 0;
            StackData.strHelpTopic.ReleaseBuffer ();

            StackData.strHelpTopic += HelpContents;
        }
        else
            StackData.strHelpTopic.ReleaseBuffer ();
    }
    StackData.strHelpPath=L"";
    ReadHelp(StackData.strHelpMask);
  }
  ScreenObject::Flags.Clear(FHELPOBJ_ERRCANNOTOPENHELP);

  if (!HelpData)
  {
    ErrorHelp=TRUE;
    if(!(StackData.Flags&FHELP_NOSHOWERROR))
    {
      BlockExtKey blockExtKey;
      MessageW(MSG_WARNING,1,UMSG(MHelpTitle),UMSG(MHelpTopicNotFound),StackData.strHelpTopic,UMSG(MOk));
    }
    return FALSE;
  }
//  ResizeConsole();
  if(IsNewTopic
    || !LocalStricmpW(StackData.strSelTopic,PluginContents) // ��� ���������� ������� :-((
#if defined(WORK_HELP_DOCUMS)
    || !LocalStricmpW(StackData.strSelTopic,DocumentContents)
#endif
    )
    MoveToReference(1,1);

  //FrameManager->ImmediateHide();
  FrameManager->RefreshFrame();

  return TRUE;
}



int Help::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  if (HelpKeyBar.ProcessMouse(MouseEvent))
    return TRUE;

  if (MouseEvent->dwEventFlags==MOUSE_MOVED &&
     (MouseEvent->dwButtonState & MOUSE_ANY_BUTTON_PRESSED)==0)
    return(FALSE);
  /* $ 26.10.2001 VVM
    + ������� ������� ������� ������ �� ����� */
  if (MouseEvent->dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
  {
    ProcessKey(KEY_ENTER);
    return(TRUE);
  }
  /* VVM $ */

  int MsX,MsY;
  MsX=MouseEvent->dwMousePosition.X;
  MsY=MouseEvent->dwMousePosition.Y;
  if ((MsX<X1 || MsY<Y1 || MsX>X2 || MsY>Y2) && MouseEventFlags != MOUSE_MOVED)
  {
    if (Flags.Check(HELPMODE_CLICKOUTSIDE))
    {
      // ���������� ���� ���������� ����� �� ��� ������� ������
      if(PreMouseEventFlags != DOUBLE_CLICK)
        ProcessKey(KEY_ESC);
    }
    if (MouseEvent->dwButtonState)
      Flags.Set(HELPMODE_CLICKOUTSIDE);
    return(TRUE);
  }

  if (MouseX==X2 && (MouseEvent->dwButtonState & 1))
  {
    int ScrollY=Y1+FixSize+1;
    int Height=Y2-Y1-FixSize-1;
    if (MouseY==ScrollY)
    {
      while (IsMouseButtonPressed())
        ProcessKey(KEY_UP);
      return(TRUE);
    }
    if (MouseY==ScrollY+Height-1)
    {
      while (IsMouseButtonPressed())
        ProcessKey(KEY_DOWN);
      return(TRUE);
    }
  }

  /* $ 15.03.2002 DJ
     ���������� ������ � �������� ����������
  */
  if (MouseX == X2)
  {
    int ScrollY=Y1+FixSize+1;
    int Height=Y2-Y1-FixSize-1;
    if (StrCount > Height)
    {
      while (IsMouseButtonPressed())
      {
        if(MouseY > ScrollY && MouseY < ScrollY+Height+1)
        {
          StackData.CurX=StackData.CurY=0;
          StackData.TopStr=(MouseY-ScrollY-1) * (StrCount-FixCount-Height+1) / (Height-2);
          FastShow();
        }
      }
      return TRUE;
    }
  }
  /* DJ $ */

  // DoubliClock - ��������/���������� ����.
  if (MouseEvent->dwEventFlags==DOUBLE_CLICK &&
      (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
      MouseEvent->dwMousePosition.Y<Y1+1+FixSize)
  {
    ProcessKey(KEY_F5);
    return(TRUE);
  }
  if (MouseEvent->dwMousePosition.Y<Y1+1+FixSize)
  {
    while (IsMouseButtonPressed() && MouseY<Y1+1+FixSize)
      ProcessKey(KEY_UP);
    return(TRUE);
  }
  if (MouseEvent->dwMousePosition.Y>=Y2)
  {
    while (IsMouseButtonPressed() && MouseY>=Y2)
      ProcessKey(KEY_DOWN);
    return(TRUE);
  }
  StackData.CurX=MouseEvent->dwMousePosition.X-X1-1;
  StackData.CurY=MouseEvent->dwMousePosition.Y-Y1-1-FixSize;
  FastShow();
  /* $ 26.11.2001 VVM
    + ��������� ������� ������� ����� � ������ � ���� ������ ����������� ��� ���������� */
  if (MouseEvent->dwEventFlags==0 &&
     (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED)))
    MouseDown = TRUE;
  if (MouseEvent->dwEventFlags==0 &&
     (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED))==0 &&
      MouseDown &&
      !StackData.strSelTopic.IsEmpty())
  {
    MouseDown = FALSE;
    ProcessKey(KEY_ENTER);
  }
  /* VVM $ */
//  if ((MouseEvent->dwButtonState & 3)==0 && *StackData.SelTopic)
//    ProcessKey(KEY_ENTER);
  return(TRUE);
}


int Help::IsReferencePresent()
{
  CorrectPosition();
  int StrPos=FixCount+StackData.TopStr+StackData.CurY;
  /* $ 19.09.2000 OT
    ������ ��� ��������� �����
    */
  if (StrPos >= StrCount) {
    return FALSE;
  }
  /* OT 19.09.2000 $ */
  wchar_t *OutStr=HelpData+StrPos*MAX_HELP_STRING_LENGTH;
  return (wcschr(OutStr,L'@')!=NULL && wcschr(OutStr,L'~')!=NULL);
}


void Help::MoveToReference(int Forward,int CurScreen)
{
  int StartSelection=!StackData.strSelTopic.IsEmpty();
  int SaveCurX=StackData.CurX;
  int SaveCurY=StackData.CurY;
  int SaveTopStr=StackData.TopStr;
  BOOL ReferencePresent;

  StackData.strSelTopic=L"";
  Lock ();

  if(!ErrorHelp) while ( StackData.strSelTopic.IsEmpty() )
  {
    ReferencePresent=IsReferencePresent();
    if (Forward)
    {
      if (StackData.CurX==0 && !ReferencePresent)
        StackData.CurX=X2-X1-2;
      if (++StackData.CurX >= X2-X1-2)
      {
        StartSelection=0;
        StackData.CurX=0;
        StackData.CurY++;
        if (StackData.TopStr+StackData.CurY>=StrCount-FixCount ||
            CurScreen && StackData.CurY>Y2-Y1-2-FixSize)
          break;
      }
    }
    else
    {
      if (StackData.CurX==X2-X1-2 && !ReferencePresent)
        StackData.CurX=0;
      if (--StackData.CurX < 0)
      {
        StartSelection=0;
        StackData.CurX=X2-X1-2;
        StackData.CurY--;
        if (StackData.TopStr+StackData.CurY<0 ||
            CurScreen && StackData.CurY<0)
          break;
      }
    }

    FastShow();

    if ( StackData.strSelTopic.IsEmpty() )
      StartSelection=0;
    else
    {
      // ��������� �������, ��������� ���� �� ��� ������ :-)
      if(ReferencePresent && CurScreen)
        StartSelection=0;

      if (StartSelection)
        StackData.strSelTopic=L"";
    }
  }
  Unlock ();
  if ( StackData.strSelTopic.IsEmpty() )
  {
    StackData.CurX=SaveCurX;
    StackData.CurY=SaveCurY;
    StackData.TopStr=SaveTopStr;
  }
  FastShow();
}

void Help::ReadDocumentsHelp(int TypeIndex)
{
  if(HelpData)
    xf_free(HelpData);
  HelpData=NULL;
  /* $ 29.11.2001 DJ
     ��� �� ������ -> ������ CurPluginContents
  */
  strCurPluginContents = L"";
  /* DJ $ */

  StrCount=0;
  FixCount=1;
  FixSize=2;
  StackData.TopStr=0;
  TopicFound=TRUE;
  StackData.CurX=StackData.CurY=0;
  strCtrlColorChar=L"";

  wchar_t *PtrTitle=0, *ContentsName=0;
  string strPath, strFullFileName;
  string strEntryName, strHelpLine, strSecondParam;

  switch(TypeIndex)
  {
    case HIDX_PLUGINS:
      PtrTitle=UMSG(MPluginsHelpTitle);
      ContentsName=L"PluginContents";
      break;
#if defined(WORK_HELP_DOCUMS)
    case HIDX_DOCUMS:
      PtrTitle=MSG(MDocumentsHelpTitle);
      ContentsName="DocumentContents";
      break;
#endif
  }

  AddTitle(PtrTitle);

  /* TODO:
     1. ����� (��� "����������") �� ������ � �������� Documets, ��
        � � ��������
  */
  int OldStrCount=StrCount;
  switch(TypeIndex)
  {
    case HIDX_PLUGINS:
    {
      for (int I=0;I<CtrlObject->Plugins.PluginsCount;I++)
      {
        strPath = CtrlObject->Plugins.PluginsData[I]->strModuleName;

        CutToSlashW(strPath);

        int nType = TYPE_ANSI;

        FILE *HelpFile=Language::OpenLangFile(strPath,HelpFileMask,Opt.strHelpLanguage,strFullFileName, nType);

        if (HelpFile!=NULL)
        {
          string strEntryName, strHelpLine, strSecondParam;
          if (Language::GetLangParam(HelpFile,ContentsName,&strEntryName,&strSecondParam, nType))
          {
            if ( !strSecondParam.IsEmpty() )
              strHelpLine.Format (L"   ~%s,%s~@" HelpFormatLink L"@", (const wchar_t*)strEntryName, (const wchar_t*)strSecondParam, (const wchar_t*)strPath,HelpContents);
            else
              strHelpLine.Format (L"   ~%s~@" HelpFormatLink L"@",(const wchar_t*)strEntryName, (const wchar_t*)strPath,HelpContents);
            AddLine(strHelpLine);
          }

          fclose(HelpFile);
        }
      }
      break;
    }

#if defined(WORK_HELP_DOCUMS)
    case HIDX_DOCUMS:
    {
      // � ��������.
      for (int I=0;I<CtrlObject->Plugins.PluginsCount;I++)
      {
        strcpy(Path,CtrlObject->Plugins.PluginsData[I].ModuleName);
        if ((Slash=strrchr(Path,'\\'))!=NULL)
          *++Slash=0;

        FILE *HelpFile=Language::OpenLangFile(Path,HelpFileMask,Opt.HelpLanguage,FullFileName);

        if (HelpFile!=NULL)
        {
          if (Language::GetLangParam(HelpFile,ContentsName,EntryName,SecondParam))
          {
            if (*SecondParam)
              sprintf(HelpLine,"   ~%s,%s~@<%s>%s@",EntryName,SecondParam,FullFileName,HelpContents);
            else
              sprintf(HelpLine,"   ~%s~@<%s>%s@",EntryName,FullFileName,HelpContents);
            AddLine(HelpLine);
          }

          fclose(HelpFile);
        }
      }

      // � ����������.
      {
        WIN32_FIND_DATA FindData;
        char FMask[NM];
        AddEndSlash(strcpy(Path,FarPath));
        strcat(Path,"Doc");
        ScanTree ScTree(FALSE,FALSE);
        ScTree.SetFindPath(Path,HelpFileMask);
        while (ScTree.GetNextName(&FindData,FullFileName))
        {
          if((PtrPath=strrchr(FullFileName,'\\')) != NULL)
          {
            xstrncpy(FMask,PtrPath+1,sizeof(FMask)-1);
            *++PtrPath=0;
          }
          else
            xstrncpy(FMask,HelpFileMask,sizeof(FMask)-1);

          FILE *HelpFile=Language::OpenLangFile(Path,FMask,Opt.HelpLanguage,FullFileName,TRUE);
          if (HelpFile!=NULL)
          {
            if (Language::GetLangParam(HelpFile,ContentsName,EntryName,SecondParam))
            {
              if (*SecondParam)
                sprintf(HelpLine,"   ~%s,%s~@<%s>%s@",EntryName,SecondParam,FullFileName,HelpContents);
              else
                sprintf(HelpLine,"   ~%s~@<%s>%s@",EntryName,FullFileName,HelpContents);

              AddLine(HelpLine);
            }
            fclose(HelpFile);
          }
        }
      }
      StackData.Flags|=FHELP_CUSTOMFILE;
      break;
    }
#endif
  }
  // ��������� �� ��������
  far_qsort(HelpData+OldStrCount*MAX_HELP_STRING_LENGTH,StrCount-OldStrCount,MAX_HELP_STRING_LENGTH*sizeof (wchar_t),(int (__cdecl*)(const void *,const void *))LocalStricmpW);
  /* $ 26.06.2000 IS
   ���������� ����� � ������ �� f1, shift+f2, end (������� ��������� IG)
  */
  AddLine(L"");
  /* IS $ */
}

// ������������ ������ � ������ ������ ��������
string &Help::MkTopic(int PluginNumber,const wchar_t *HelpTopic,string &strTopic)
{
  strTopic=L"";
  if (HelpTopic && *HelpTopic)
  {
    if (*HelpTopic==L':')
      strTopic = (HelpTopic+1);
    else
    {
      if(PluginNumber != -1 && *HelpTopic!=HelpBeginLink)
      {
         strTopic.Format (
                HelpFormatLinkModule,
                (const wchar_t*)CtrlObject->Plugins.PluginsData[PluginNumber]->strModuleName,
                HelpTopic
                );
      }
      else
        strTopic = HelpTopic;

      if( strTopic.At(0)==HelpBeginLink)
      {
        wchar_t *Ptr, *Ptr2;

        wchar_t *lpwszTopic = strTopic.GetBuffer(strTopic.GetLength()*2); //BUGBUG

        if((Ptr=wcschr(lpwszTopic,HelpEndLink)) == NULL)
          *lpwszTopic=0;
        else
        {
          if(!Ptr[1]) // ���� ��� ������� ��...
            wcscat(lpwszTopic,HelpContents); // ... ������ ������� �������� ����. //BUGBUG

          /* � ��� ������ ���������...
             ������ ����� ���� :
               "<FullPath>Topic" ��� "<FullModuleName>Topic"
             ��� ������ "FullPath" ���� ������ ������������� ������!
             �.�. �� ������� ��� ��� - ��� ������ ��� ����!
          */
          Ptr2=Ptr-1;
          if(*Ptr2 != L'\\') // ��� ��� ������?
          {
            // ������ ������ ��� ������� ��� :-)
            if((Ptr2=wcsrchr(lpwszTopic,L'\\')) == NULL) // ��! ����� �����-�� :-(
              *lpwszTopic=0;
          }
          if( *lpwszTopic )
          {
            /* $ 21.08.2001 KM
              - ������� ���������� ����� � ������ ������ �������,
                � ������� ���� ��� ������ ������ ������������� "/".
            */
            memmove(Ptr2+1,Ptr,(wcslen(Ptr)+1)*sizeof(wchar_t)); //???
            /* KM $ */

            // � ��� ����� ������ ��� �� �������� Help API!
          }
        }

        strTopic.ReleaseBuffer();
      }
    }
  }
  return strTopic;
}


/* $ 28.06.2000 tran
 (NT Console resize)
 resize help*/
void Help::SetScreenPosition()
{
  if (Opt.FullScreenHelp)
  {
    HelpKeyBar.Hide();
    SetPosition(0,0,ScrX,ScrY);
  }
  else
    SetPosition(4,2,ScrX-4,ScrY-2);
  Show();
}
/* tran $ */

/* $ 30.12.2000 SVS
  ������� ������������� KeyBar Labels
*/
void Help::InitKeyBar(void)
{
  wchar_t *FHelpKeys[]={UMSG(MHelpF1),UMSG(MHelpF2),UMSG(MHelpF3),UMSG(MHelpF4),UMSG(MHelpF5),UMSG(MHelpF6),UMSG(MHelpF7),UMSG(MHelpF8),UMSG(MHelpF9),UMSG(MHelpF10),UMSG(MHelpF11),UMSG(MHelpF12)};
  wchar_t *FHelpShiftKeys[]={UMSG(MHelpShiftF1),UMSG(MHelpShiftF2),UMSG(MHelpShiftF3),UMSG(MHelpShiftF4),UMSG(MHelpShiftF5),UMSG(MHelpShiftF6),UMSG(MHelpShiftF7),UMSG(MHelpShiftF8),UMSG(MHelpShiftF9),UMSG(MHelpShiftF10),UMSG(MHelpShiftF11),UMSG(MHelpShiftF12)};
  wchar_t *FHelpAltKeys[]={UMSG(MHelpAltF1),UMSG(MHelpAltF2),UMSG(MHelpAltF3),UMSG(MHelpAltF4),UMSG(MHelpAltF5),UMSG(MHelpAltF6),UMSG(MHelpAltF7),UMSG(MHelpAltF8),UMSG(MHelpAltF9),UMSG(MHelpAltF10),UMSG(MHelpAltF11),UMSG(MHelpAltF12)};
  wchar_t *FHelpCtrlKeys[]={UMSG(MHelpCtrlF1),UMSG(MHelpCtrlF2),UMSG(MHelpCtrlF3),UMSG(MHelpCtrlF4),UMSG(MHelpCtrlF5),UMSG(MHelpCtrlF6),UMSG(MHelpCtrlF7),UMSG(MHelpCtrlF8),UMSG(MHelpCtrlF9),UMSG(MHelpCtrlF10),UMSG(MHelpCtrlF11),UMSG(MHelpCtrlF12)};
//  char *FHelpAltShiftKeys[]={MSG(MHelpAltShiftF1),MSG(MHelpAltShiftF2),MSG(MHelpAltShiftF3),MSG(MHelpAltShiftF4),MSG(MHelpAltShiftF5),MSG(MHelpAltShiftF6),MSG(MHelpAltShiftF7),MSG(MHelpAltShiftF8),MSG(MHelpAltShiftF9),MSG(MHelpAltShiftF10),MSG(MHelpAltShiftF11),MSG(MHelpAltShiftF12)};
//  char *FHelpCtrlShiftKeys[]={MSG(MHelpCtrlShiftF1),MSG(MHelpCtrlShiftF2),MSG(MHelpCtrlShiftF3),MSG(MHelpCtrlShiftF4),MSG(MHelpCtrlShiftF5),MSG(MHelpCtrlShiftF6),MSG(MHelpCtrlShiftF7),MSG(MHelpCtrlShiftF8),MSG(MHelpCtrlShiftF9),MSG(MHelpCtrlShiftF10),MSG(MHelpCtrlShiftF11),MSG(MHelpCtrlShiftF12)};
//  char *FHelpCtrlAltKeys[]={MSG(MHelpCtrlAltF1),MSG(MHelpCtrlAltF2),MSG(MHelpCtrlAltF3),MSG(MHelpCtrlAltF4),MSG(MHelpCtrlAltF5),MSG(MHelpCtrlAltF6),MSG(MHelpCtrlAltF7),MSG(MHelpCtrlAltF8),MSG(MHelpCtrlAltF9),MSG(MHelpCtrlAltF10),MSG(MHelpCtrlAltF11),MSG(MHelpCtrlAltF12)};
  wchar_t *FHelpAltShiftKeys[]={L"",L"",L"",L"",L"",L"",L"",L"",L"",L"",L"",L""};
  wchar_t *FHelpCtrlShiftKeys[]={L"",L"",L"",L"",L"",L"",L"",L"",L"",L"",L"",L""};
  wchar_t *FHelpCtrlAltKeys[]={L"",L"",L"",L"",L"",L"",L"",L"",L"",L"",L"",L""};

  // ������ ������ � ���� �����
#if !defined(WORK_HELP_DOCUMS)
  FHelpShiftKeys[3-1][0]=0;
#endif
#if !defined(WORK_HELP_FIND)
  FHelpKeys[7-1][0]=0;
  FHelpShiftKeys[7-1][0]=0;
#endif

  HelpKeyBar.Set(FHelpKeys,sizeof(FHelpKeys)/sizeof(FHelpKeys[0]));
  HelpKeyBar.SetShift(FHelpShiftKeys,sizeof(FHelpShiftKeys)/sizeof(FHelpShiftKeys[0]));
  HelpKeyBar.SetAlt(FHelpAltKeys,sizeof(FHelpAltKeys)/sizeof(FHelpAltKeys[0]));
  HelpKeyBar.SetCtrl(FHelpCtrlKeys,sizeof(FHelpCtrlKeys)/sizeof(FHelpCtrlKeys[0]));
  HelpKeyBar.SetCtrlAlt(FHelpCtrlAltKeys,sizeof(FHelpCtrlAltKeys)/sizeof(FHelpCtrlAltKeys[0]));
  HelpKeyBar.SetCtrlShift(FHelpCtrlShiftKeys,sizeof(FHelpCtrlShiftKeys)/sizeof(FHelpCtrlShiftKeys[0]));
  HelpKeyBar.SetAltShift(FHelpAltShiftKeys,sizeof(FHelpAltShiftKeys)/sizeof(FHelpAltShiftKeys[0]));

  SetKeyBar(&HelpKeyBar);
}
/* SVS $ */

/* $ 25.08.2000 SVS
   ������ URL-������... ;-)
   ��� ���� ��� ������... ���?
   ������:
     0 - ��� �� URL ������ (�� ������)
     1 - CreateProcess ������ FALSE
     2 - ��� ��

   ��������� (��������):
     Protocol="mailto"
     URLPath ="mailto:vskirdin@mail.ru?Subject=Reversi"
*/
static int RunURL(const wchar_t *Protocol, wchar_t *URLPath)
{
  int EditCode=0;
  if(Protocol && *Protocol && URLPath && *URLPath && (Opt.HelpURLRules&0xFF))
  {
    wchar_t *Buf=(wchar_t*)xf_malloc(2048); //BUGBUG
    if(Buf)
    {
      HKEY hKey;
      DWORD Disposition, DataSize=250;
      wcscpy(Buf,Protocol);
      wcscat(Buf,L"\\shell\\open\\command");
      if(RegOpenKeyExW(HKEY_CLASSES_ROOT,Buf,0,KEY_READ,&hKey) == ERROR_SUCCESS)
      {
        Disposition=RegQueryValueExW(hKey,L"",0,&Disposition,(LPBYTE)Buf,&DataSize);

        ExpandEnvironmentStringsW(Buf, Buf, 2048); //BUGBUG
        RegCloseKey(hKey);
        if(Disposition == ERROR_SUCCESS)
        {
          wchar_t *pp=wcsrchr(Buf,L'%');
          if(pp) *pp=L'\0'; else wcscat(Buf,L" ");

          // ������ ��� ������ � ������ ~~
          pp=URLPath;
          while(*pp && (pp=wcsstr(pp,L"~~")) != NULL)
          {
            memmove(pp,pp+1,(wcslen(pp+1)+1)*sizeof (wchar_t));
            ++pp;
          }
          // ������ ��� ������ � ������ ##
          pp=URLPath;
          while(*pp && (pp=wcsstr(pp,L"##")) != NULL)
          {
            memmove(pp,pp+1,(wcslen(pp+1)+1)*sizeof (wchar_t));
            ++pp;
          }

          Disposition=0;
          if(Opt.HelpURLRules == 2 || Opt.HelpURLRules == 2+256)
          {
            BlockExtKey blockExtKey;
            Disposition=MessageW(MSG_WARNING,2,UMSG(MHelpTitle),
                        UMSG(MHelpActivatorURL),
                        Buf,
                        UMSG(MHelpActivatorFormat),
                        URLPath,
                        L"\x01",
                        UMSG(MHelpActivatorQ),
                        UMSG(MYes),UMSG(MNo));
          }
          EditCode=2; // ��� Ok!
          if(Disposition == 0)
          {
            /*
              ���� ����� ���������� ������ � ������������ ������
              ���� ��� ����� ���������� - �� ����� ���� ���������!!!!!
            */
            if(Opt.HelpURLRules < 256) // SHELLEXECUTEEX_METHOD
            {
#if 0
              SHELLEXECUTEINFO sei;

              FAR_OemToChar(URLPath,Buf);
              memset(&sei,0,sizeof(sei));
              sei.cbSize=sizeof(sei);
              sei.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_DDEWAIT;
              sei.lpFile=RemoveExternalSpaces(Buf);
              sei.nShow=SW_SHOWNORMAL;
              SetFileApisTo(APIS2ANSI);
              if(ShellExecuteEx(&sei))
                EditCode=1;
              SetFileApisTo(APIS2OEM);
#else
              wcscpy (Buf, URLPath);
              EditCode=ShellExecuteW(0, 0, RemoveExternalSpacesW(Buf), 0, 0, SW_SHOWNORMAL)?1:2;
#endif
            }
            else
            {
              STARTUPINFOW si={0};
              PROCESS_INFORMATION pi={0};
              si.cb=sizeof(si);
              wcscat(Buf,URLPath);
              if(!CreateProcessW(NULL,Buf,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi))
                 EditCode=1;
            }
          }
        }
      }
      xf_free(Buf);
    }
  }
  return EditCode;
}

void Help::OnChangeFocus(int Focus)
{
  if (Focus)
  {
    DisplayObject();
  }
}

void Help::ResizeConsole()
{
  int OldIsNewTopic=IsNewTopic;
  BOOL ErrCannotOpenHelp=ScreenObject::Flags.Check(FHELPOBJ_ERRCANNOTOPENHELP);
  ScreenObject::Flags.Set(FHELPOBJ_ERRCANNOTOPENHELP);
  IsNewTopic=FALSE;
  delete TopScreen;
  TopScreen=NULL;

  Hide();
  if (Opt.FullScreenHelp)
  {
    HelpKeyBar.Hide();
    SetPosition(0,0,ScrX,ScrY);
  }
  else
    SetPosition(4,2,ScrX-4,ScrY-2);
  ReadHelp(StackData.strHelpMask);
  ErrorHelp=FALSE;
//  StackData.CurY--; // ��� ���� ������� (����� ���� ����� ���!)
  StackData.CurX--;
  MoveToReference(1,1);
  IsNewTopic=OldIsNewTopic;
  ScreenObject::Flags.Change(FHELPOBJ_ERRCANNOTOPENHELP,ErrCannotOpenHelp);
  FrameManager->ImmediateHide();
  FrameManager->RefreshFrame();
}

int Help::FastHide()
{
  return Opt.AllCtrlAltShiftRule & CASR_HELP;
}


int Help::GetTypeAndName(string &strType, string &strName)
{
  strType = UMSG(MHelpType);
  strName = L"";

  return(MODALTYPE_HELP);
}

/* ------------------------------------------------------------------ */
void CallBackStack::ClearStack()
{
  while(!isEmpty())
    Pop();
}

int CallBackStack::Pop(struct StackHelpData *Dest)
{
  if(!isEmpty())
  {
    ListNode *oldTop = topOfStack;
    topOfStack = topOfStack->Next;
    if(Dest!=NULL)
    {
      Dest->strHelpTopic = oldTop->strHelpTopic;
      Dest->strHelpPath = oldTop->strHelpPath;
      Dest->strSelTopic = oldTop->strSelTopic;
      Dest->strHelpMask = oldTop->strHelpMask;

      Dest->Flags=oldTop->Flags;
      Dest->TopStr=oldTop->TopStr;
      Dest->CurX=oldTop->CurX;
      Dest->CurY=oldTop->CurY;
    }
    delete oldTop;
    return TRUE;
  }
  return FALSE;
}

void CallBackStack::Push(const struct StackHelpData *Data)
{
  topOfStack=new ListNode(Data,topOfStack);
}

void CallBackStack::PrintStack(const wchar_t *Title)
{
#if defined(SYSLOG)
  int I=0;
  ListNode *Ptr = topOfStack;
  SysLog("Return Stack (%s)",Title);
  SysLog(1);
  while(Ptr)
  {
    SysLog("%03d HelpTopic='%S' HelpPath='%S' HelpMask='%S'",I++,(const wchar_t*)Ptr->strHelpTopic,(const wchar_t*)Ptr->strHelpPath,(const wchar_t*)Ptr->strHelpMask);
    Ptr=Ptr->Next;
  }
  SysLog(-1);
#endif
}
