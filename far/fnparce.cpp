/*
fnparce.cpp

������ �������� ����������

*/

/* Revision: 1.32 07.07.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "panel.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "flink.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "dialog.hpp"
#include "global.hpp"
#include "lang.hpp"


struct TSubstDataW
{
  // ��������� ������� SubstFileName
  const wchar_t *Name;           // ������� ���
  const wchar_t *ShortName;      // �������� ���

  string *pListName;
  string *pAnotherListName;

  string *pShortListName;
  string *pAnotherShortListName;

  // ��������� ����������
  string strAnotherName;
  string strAnotherShortName;
  string strNameOnly;
  string strShortNameOnly;
  string strAnotherNameOnly;
  string strAnotherShortNameOnly;
  string strCmdDir;
  int  PreserveLFN;
  int  PassivePanel;

  Panel *AnotherPanel;
  Panel *ActivePanel;
};


static int IsReplaceVariableW(const wchar_t *str,int *scr = NULL,
                                int *end = NULL,
                                int *beg_scr_break = NULL,
                                int *end_scr_break = NULL,
                                int *beg_txt_break = NULL,
                                int *end_txt_break = NULL);


static int ReplaceVariables(wchar_t *Str,struct TSubstDataW *PSubstData);
static wchar_t *_SubstFileName(wchar_t *CurStr,struct TSubstDataW *PSubstData,wchar_t *TempStr,int MaxTempStrSize);

// Str=if exist !#!\!^!.! far:edit < diff -c -p "!#!\!^!.!" !\!.!

static wchar_t *_SubstFileName(wchar_t *CurStr,struct TSubstDataW *PSubstData,wchar_t *TmpStr,int MaxTempStrSize)
{
  // ���������� ������������� ����������/����������� ������.
  if (wcsncmp(CurStr,L"!#",2)==0)
  {
    CurStr+=2;
    PSubstData->PassivePanel=TRUE;
    //_SVS(SysLog("PassivePanel=TRUE '%s'",CurStr));
    return CurStr;
  }

  if (wcsncmp(CurStr,L"!^",2)==0)
  {
    CurStr+=2;
    PSubstData->PassivePanel=FALSE;
    //_SVS(SysLog("PassivePanel=FALSE '%s'",CurStr));
    return CurStr;
  }

  // !! ������ '!'
  if (wcsncmp(CurStr,L"!!",2)==0 && CurStr[2] != L'?')
  {
    wcsncat(TmpStr,L"!",MaxTempStrSize-1);
    CurStr+=2;
    //_SVS(SysLog("!! TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !.!      ������� ��� ����� � �����������
  if (wcsncmp(CurStr,L"!.!",3)==0 && CurStr[3] != L'?')
  {
    wcsncat(TmpStr,PSubstData->PassivePanel ? (const wchar_t *)PSubstData->strAnotherName:PSubstData->Name, MaxTempStrSize-1);
    CurStr+=3;
    //_SVS(SysLog("!.! TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !~       �������� ��� ����� ��� ����������
  if (wcsncmp(CurStr,L"!~",2)==0)
  {
    wcsncat(TmpStr,PSubstData->PassivePanel ? PSubstData->strAnotherShortNameOnly:PSubstData->strShortNameOnly, MaxTempStrSize-1);
    CurStr+=2;
    //_SVS(SysLog("!~ TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !`  ������� ���������� ����� ��� �����
  if (wcsncmp(CurStr,L"!`",2)==0)
  {
    wchar_t *Ext;
    if(CurStr[2] == L'~')
    {
      Ext=wcsrchr((PSubstData->PassivePanel ? (const wchar_t *)PSubstData->strAnotherShortName:PSubstData->ShortName),L'.');
      CurStr+=3;
    }
    else
    {
      Ext=wcsrchr((PSubstData->PassivePanel ? (const wchar_t *)PSubstData->strAnotherName:PSubstData->Name),L'.');
      CurStr+=2;
    }
    if(Ext && *Ext)
      wcsncat(TmpStr,++Ext, MaxTempStrSize-1);
    //_SVS(SysLog("!` TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !& !&~  ������ ������ ����������� ��������.
  if (!wcsncmp(CurStr,L"!&~",3) && CurStr[3] != L'?' ||
      !wcsncmp(CurStr,L"!&",2) && CurStr[2] != L'?')
  {
    string strFileNameL, strShortNameL;
    Panel *WPanel=PSubstData->PassivePanel?PSubstData->AnotherPanel:PSubstData->ActivePanel;
    int FileAttrL;
    int ShortN0=FALSE;
    int CntSkip=2;
    if(CurStr[2] == L'~')
    {
      ShortN0=TRUE;
      CntSkip++;
    }
    WPanel->GetSelNameW(NULL,FileAttrL);
    int First = TRUE;
    while (WPanel->GetSelNameW(&strFileNameL,FileAttrL,&strShortNameL))
    {
      if (ShortN0)
        strFileNameL = strShortNameL;
      else // � ������ ��� �� ������ ������� ��� � ��������.
        QuoteSpaceOnlyW(strFileNameL);
// ��� ����� ��� ��� ����� - �����/�������...
//   ���� ����� ����� - ��������������� :-)
//          if(FileAttrL & FA_DIREC)
//            AddEndSlash(FileNameL);
      // � ����� �� ��� ������ � ����� ������?
      if (First)
        First = FALSE;
      else
        wcsncat(TmpStr,L" ", MaxTempStrSize-1);
      wcsncat(TmpStr,strFileNameL, MaxTempStrSize-1);
      /* $ 05.03.2002 DJ
         ���� � ����� ������ �� ������ - ������ �� �����
      */
      if (wcslen (TmpStr) >= static_cast<size_t>(MaxTempStrSize-1))
        break;
      /* DJ $ */
    }
    CurStr+=CntSkip;
    //_SVS(SysLog("!& TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !@  ��� �����, ����������� ����� ���������� ������
  //<Skeleton 2003 10 25>
  // !$!      ��� �����, ����������� �������� ����� ���������� ������
  // ���� ���� ���������� ���� ��� ������� ��� !@! ��� � !$!
  //������-�� (�� ������������ �������������� ��� ��) - � !$! ����� ����������� ������������ Q � A
  // �� ����� ����:)
  //<Skeleton>
  if (wcsncmp(CurStr,L"!@",2)==0
        //<Skeleton 2003 10 25>
        || (wcsncmp(CurStr,L"!$",2)==0))
        //<Skeleton>
  {
    wchar_t Modifers[32]=L"", *Ptr; //???

    string *pListName;
    string *pAnotherListName;

    bool ShortN0 = FALSE;

    if (CurStr[1] == L'$')
      ShortN0 = TRUE;

    if ( ShortN0 )
    {
        pListName = PSubstData->pShortListName;
        pAnotherListName = PSubstData->pAnotherShortListName;
    }
    else
    {
        pListName = PSubstData->pListName;
        pAnotherListName = PSubstData->pAnotherListName;
    }


    if((Ptr=wcschr(CurStr+2,L'!')) != NULL)
    {
      if(Ptr[1] != L'?')
      {
        *Ptr=0;

        xwcsncpy(Modifers,CurStr+2,sizeof(Modifers)-1);

        if ( pListName)
        {

          if ( PSubstData->PassivePanel && ( !pAnotherListName->IsEmpty() || PSubstData->AnotherPanel->MakeListFile(*pAnotherListName,FALSE,Modifers)))
          {
            if (ShortN0)
              ConvertNameToShortW(*pAnotherListName, *pAnotherListName);

            wcsncat(TmpStr,*pAnotherListName, MaxTempStrSize-1);

          }

          if ( !PSubstData->PassivePanel && ( !pListName->IsEmpty() || PSubstData->ActivePanel->MakeListFile(*pListName,FALSE,Modifers)))
          {
            if (ShortN0)
              ConvertNameToShortW(*pListName,*pListName);

            wcsncat(TmpStr,*pListName, MaxTempStrSize-1);
          }
        }
        else
        {
          wcsncat(TmpStr,CurStr, MaxTempStrSize-1);
          wcsncat(TmpStr,Modifers, MaxTempStrSize-1);
          wcsncat(TmpStr,L"!", MaxTempStrSize-1);
        }

        /* tran $ */
        CurStr+=Ptr-CurStr+1;
        return CurStr;
      }
    }
  }

  // !-!      �������� ��� ����� � �����������
  if (wcsncmp(CurStr,L"!-!",3)==0 && CurStr[3] != L'?')
  {
    wcsncat(TmpStr,PSubstData->PassivePanel ? (const wchar_t *)PSubstData->strAnotherShortName:PSubstData->ShortName, MaxTempStrSize-1);
    CurStr+=3;
    //_SVS(SysLog("!-! TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !+!      ���������� !-!, �� ���� ������� ��� ����� �������
  //          ����� ���������� �������, FAR ����������� ���
  if (wcsncmp(CurStr,L"!+!",3)==0 && CurStr[3] != L'?')
  {
    wcsncat(TmpStr,PSubstData->PassivePanel ? (const wchar_t *)PSubstData->strAnotherShortName:PSubstData->ShortName, MaxTempStrSize-1);
    CurStr+=3;
    PSubstData->PreserveLFN=TRUE;
    //_SVS(SysLog("!+! TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !:       ������� ����
  //<Skeleton 2003 10 26>
  // ���� - �������� ������ �� \\serv\share
  //<Skeleton>
  if (wcsncmp(CurStr,L"!:",2)==0)
  {
    string strCurDir;
    string strRootDir;
    if (*PSubstData->Name && PSubstData->Name[1]==L':')
      strCurDir = PSubstData->Name;
    else
      if (PSubstData->PassivePanel)
        PSubstData->AnotherPanel->GetCurDirW(strCurDir);
      else
        strCurDir = PSubstData->strCmdDir;

    GetPathRootW(strCurDir,strRootDir);
    DeleteEndSlashW(strRootDir);
    wcsncat(TmpStr,strRootDir, MaxTempStrSize-1);
    CurStr+=2;
    //_SVS(SysLog("!: TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !\       ������� ����
  //<Skeleton 2003 10 26>
  // !/       �������� ��� �������� ����
  // ���� ���� ���������� ���� ��� ������� ��� !\ ��� � !/
  //<Skeleton>
  if (wcsncmp(CurStr,L"!\\",2)==0 || wcsncmp(CurStr,L"!=\\",3)==0
     //<Skeleton 2003 10 26>
     || (wcsncmp(CurStr,L"!/",2)==0) || wcsncmp(CurStr,L"!=/",3)==0)
     //<Skeleton>
  {
    string strCurDir;
    //<Skeleton 2003 10 26>
    bool ShortN0 = FALSE;
    int RealPath= CurStr[1]==L'='?1:0;

    if (CurStr[1] == L'/' || (RealPath && CurStr[2] == L'/'))
    {
      ShortN0 = TRUE;
    }
    //<Skeleton>
    if (PSubstData->PassivePanel)
      PSubstData->AnotherPanel->GetCurDirW(strCurDir);
    else
      strCurDir = PSubstData->strCmdDir;

    if(RealPath)
    {
      _MakePath1W(PSubstData->PassivePanel?KEY_ALTSHIFTBACKBRACKET:KEY_ALTSHIFTBRACKET,strCurDir,L"",ShortN0);
      UnquoteW(strCurDir);
    }

    if (ShortN0)
        ConvertNameToShortW(strCurDir,strCurDir);

    AddEndSlashW(strCurDir);
    if(RealPath)
      QuoteSpaceOnlyW(strCurDir);

    CurStr+=2+RealPath;


    wchar_t *CurDir = strCurDir.GetBuffer ();

    if (*CurStr=='!')
    {
//      strcpy(TmpName,PSubstData->Name);
//      strcpy(TmpShortName,PSubstData->ShortName);
      if (wcspbrk(PSubstData->PassivePanel?(const wchar_t *)PSubstData->strAnotherName:PSubstData->Name,L"\\:")!=NULL)
        *CurDir=0;
    }

    strCurDir.ReleaseBuffer ();
//    if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
    wcsncat(TmpStr, strCurDir, MaxTempStrSize-1);
    //_SVS(SysLog("!\\ TmpStr=[%s] CurDir=[%s]",TmpStr, CurDir));
    return CurStr;
  }

  // !?<title>?<init>!
  if (wcsncmp(CurStr,L"!?",2)==0 && wcschr(CurStr+2,L'!')!=NULL)
  {
    //<Skeleton 2003 11 22>
    //char *NewCurStr=strchr(CurStr+2,'!')+1;
    //strncat(TmpStr,CurStr,NewCurStr-CurStr);
    //CurStr=NewCurStr;

    int j;
    int i = IsReplaceVariableW(CurStr);
    if (i == -1)  // if bad format string
    {             // skip 1 char
      j = 1;
    }
    else
    {
      j = i + 1;
    }
    wcsncat(TmpStr,CurStr,j);
    CurStr += j;
    //<Skeleton>
    //_SVS(SysLog("!? TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !        ������� ��� ����� ��� ����������
  if (*CurStr==L'!')
  {
//    if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
    wcsncat(TmpStr,PointToNameW(PSubstData->PassivePanel ? PSubstData->strAnotherNameOnly:PSubstData->strNameOnly), MaxTempStrSize-1);
    CurStr++;
    //_SVS(SysLog("! TmpStr=[%s]",TmpStr));
  }

  return CurStr;
}


/*
  SubstFileName()
  �������������� ������������ ���������� ������ � �������� ��������

  ������� ListName � ShortListName ������� ����� ������ NM*2 !!!
*/
int SubstFileName(string &strStr,            // �������������� ������
                  const wchar_t *Name,           // ������� ���
                  const wchar_t *ShortName,      // �������� ���

                  string *pListName,
                  string *pAnotherListName,
                  string *pShortListName,
                  string *pAnotherShortListName,
                  int   IgnoreInput,    // TRUE - �� ��������� "!?<title>?<init>!"
                  const wchar_t *CmdLineDir)     // ������� ����������
{
    if ( pListName )
        *pListName = L"";

    if ( pAnotherListName )
        *pAnotherListName = L"";

    if ( pShortListName )
        *pShortListName = L"";

    if ( pAnotherShortListName )
        *pAnotherShortListName = L"";

  /* $ 19.06.2001 SVS
    ��������! ��� �������������� ������������, �� ���������� �� "!",
    ����� ����� ���� ������ ��� �������� ���� �������� ������� (���������
    ����������������!)
  */
  if(!wcschr(strStr,L'!'))
    return FALSE;
  /* SVS $ */

  wchar_t TmpStr2[10240]; //BUGBUGBUGBUGBUGBUG!!!!

  struct TSubstDataW SubstData, *PSubstData=&SubstData;
  wchar_t *ChPtr;

  // ������� ���� ������� - 10240, �� ����� ����� ��������� ������... (see below)
  wchar_t TmpStr[10240]; //BUGBUGBUG!!!
  wchar_t *CurStr;

  // <PreProcess>
  *TmpStr=0; // ���� �����.

  PSubstData->Name=Name;                    // ������� ���
  PSubstData->ShortName=ShortName;          // �������� ���

  PSubstData->pListName=pListName;            // ������� ��� �����-������
  PSubstData->pAnotherListName=pAnotherListName;            // ������� ��� �����-������
  PSubstData->pShortListName=pShortListName;  // �������� ��� �����-������
  PSubstData->pAnotherShortListName=pAnotherShortListName;  // �������� ��� �����-������

  // ���� ��� �������� �������� �� ������...
  if (CmdLineDir!=NULL)
    PSubstData->strCmdDir = CmdLineDir;
  else // ...������� � ���.������
    CtrlObject->CmdLine->GetCurDirW(PSubstData->strCmdDir);


  // �������������� ������� ��������� "���������" :-)
  PSubstData->strNameOnly = Name;

  ChPtr = PSubstData->strNameOnly.GetBuffer ();

  if ((ChPtr=wcsrchr(ChPtr,L'.'))!=NULL)
    *ChPtr=0;

  PSubstData->strNameOnly.ReleaseBuffer ();


  PSubstData->strShortNameOnly = ShortName;

  ChPtr = PSubstData->strShortNameOnly.GetBuffer ();

  if ((ChPtr=wcsrchr(ChPtr, L'.'))!=NULL)
    *ChPtr=0;

  PSubstData->strShortNameOnly.ReleaseBuffer ();

  PSubstData->ActivePanel=CtrlObject->Cp()->ActivePanel;
  PSubstData->AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(PSubstData->ActivePanel);

  PSubstData->AnotherPanel->GetCurNameW(PSubstData->strAnotherName,PSubstData->strAnotherShortName);

  PSubstData->strAnotherNameOnly = PSubstData->strAnotherName;

  ChPtr = PSubstData->strAnotherNameOnly.GetBuffer ();

  if ((ChPtr=wcsrchr(ChPtr, L'.'))!=NULL)
    *ChPtr=0;

  PSubstData->strAnotherNameOnly.ReleaseBuffer ();

  PSubstData->strAnotherShortNameOnly = PSubstData->strAnotherShortName;

  ChPtr = PSubstData->strAnotherShortNameOnly.GetBuffer ();

  if ((ChPtr=wcsrchr(PSubstData->strAnotherShortNameOnly, L'.'))!=NULL)
    *ChPtr=0;

  PSubstData->strAnotherShortNameOnly.ReleaseBuffer ();

  PSubstData->PreserveLFN=FALSE;
  PSubstData->PassivePanel=FALSE; // ������������� ���� ���� ��� �������� ������!
  // </PreProcess>

  wcscpy (TmpStr2, strStr); //BUGBUG

  if (!IgnoreInput)
    ReplaceVariables(TmpStr2,PSubstData);

  //CurStr=Str;
  CurStr=TmpStr2;

  while (*CurStr)
  {
    //_SVS(SysLog("***** Pass=%d",Pass));
    if(*CurStr == L'!')
      CurStr=_SubstFileName(CurStr,PSubstData,TmpStr,sizeof(TmpStr)-wcslen(TmpStr)-1);
    else                                           //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ��� ������ ��������!
    {
      wcsncat(TmpStr,CurStr,1);
      CurStr++;
    }
    //_SVS(++Pass);
  }

  strStr = TmpStr;

  //_SVS(SysLog("[%s]\n",Str));
  return(PSubstData->PreserveLFN);
}

int ReplaceVariables(wchar_t *Str,struct TSubstDataW *PSubstData)
{
  const int MaxSize=20;

  wchar_t *StartStr=Str;

  if (*Str==L'\"')
    while (*Str && *Str!=L'\"')
      Str++;

  struct DialogItemEx *DlgData=NULL;
  int DlgSize=0;
  int StrPos[128],StrEndPos[128],StrPosSize=0;

  while (*Str && DlgSize<MaxSize)
  {
    if (*(Str++)!=L'!')
      continue;
    if (*(Str++)!=L'?')
      continue;

    //<Skeleton 2003 11 20>
    //if (strchr(Str,'!')==NULL)  //<---------�������� ��� �� ������
    //  return 0;                 // �������� ����� ���������� ������� ����������� ������
                                  // ��������� �� �������
    int scr,end, beg_t,end_t,beg_s,end_s;
    scr = end = beg_t = end_t = beg_s = end_s = 0;
    int ii = IsReplaceVariableW(Str-2,&scr,&end,&beg_t,&end_t,&beg_s,&end_s);
    if (ii == -1)
    {
      ::free(DlgData);
      *StartStr=0;
      return 0;
    }

    StrEndPos[StrPosSize] = (Str - StartStr - 2) + ii ; //+1
    StrPos[StrPosSize++]=Str-StartStr-2;
    //<Skeleton>

    DlgData=(struct DialogItemEx *)xf_realloc(DlgData,(DlgSize+2)*sizeof(*DlgData));
    memset(&DlgData[DlgSize],0,2*sizeof(*DlgData)); //BUGBUGBUG!!!!
    DlgData[DlgSize].Type=DI_TEXT;
    DlgData[DlgSize].X1=5;
    DlgData[DlgSize].Y1=DlgSize+2;
    DlgData[DlgSize+1].Type=DI_EDIT;
    DlgData[DlgSize+1].X1=5;
    DlgData[DlgSize+1].X2=70;
    DlgData[DlgSize+1].Y1=DlgSize+3;
    DlgData[DlgSize+1].Flags|=DIF_HISTORY|DIF_USELASTHISTORY;

    wchar_t HistoryName[MaxSize][20];
    int HistoryNumber=DlgSize/2;
    swprintf(HistoryName[HistoryNumber],L"UserVar%d",HistoryNumber);
    /* $ 01.08.2000 SVS
       + .History
    */
    DlgData[DlgSize+1].Selected=(int)HistoryName[HistoryNumber];
    /* SVS $*/

    if (DlgSize==0)
    {
      DlgData[DlgSize+1].Focus=1;
      DlgData[DlgSize+1].DefaultButton=1;
    }

    wchar_t Title[256];
    //<Skeleton 2003 11 22>
    //xstrncpy(Title,Str,sizeof(Title)-1);
    //*strchr(Title,'!')=0;
    //Str+=strlen(Title)+1;
    //char *SrcText=strchr(Title,'?');
    //if (SrcText!=NULL)
    //{
    //  *SrcText=0;
    //  xstrncpy(DlgData[DlgSize+1].Data.Data,SrcText+1,sizeof(DlgData[DlgSize+1].Data.Data)-1);
    //}

    wchar_t Title2[512];
    wchar_t Title3[512];
    wchar_t Txt[512];

    // ��������� �������� ���� ����� �������� �������� - ���� ����
    *Title = *Title2 = *Title3 = 0;

    if (scr > 2)          // if between !? and ? exist some
      wcsncat(Title,Str,scr-2);

    //strcpy(DlgData[DlgSize].Data.Data,Title);

    wchar_t *t = Title;

    if (wcslen(Title)>0)
    {
      if (Title[0] == L'$')        // begin of history name
      {
        wchar_t *p = &Title[1];
        wchar_t *p1 = wcschr(p,L'$');
        if (p1)
        {
          *p1 = 0;
          t = ++p1;
          wcscpy(HistoryName[HistoryNumber],p);
        }
      }
    }

    wchar_t *Title1 = NULL;
    Title1 = (wchar_t *)xf_realloc(Title1,10240);
    wchar_t *Txt1 = NULL;
    Txt1 = (wchar_t *)xf_realloc(Txt1,10240);
    wchar_t * tmp_t = NULL;
    tmp_t = (wchar_t *)xf_realloc(tmp_t,10240);

    *Title1 = 0;
    *Txt1 = 0;
    *tmp_t = 0;


    if ((beg_t == -1) || (end_t == -1))
    {
      //
    }
    else if ((end_t - beg_t) > 1) //if between ( and ) exist some
    {
      int hist_correct = 0;
      if (t != Title) // Title contain name of history
        hist_correct = t - Title; //offset for jump over history

      wcsncat(Title1,Title+hist_correct,beg_t-2-hist_correct);   // !?$zz$xxxx(fffff)ddddd
                                                                 //       ^  ^

      wcsncat(Title2,Title+(end_t-2)+1,scr-end_t-1); // !?$zz$xxxx(fffff)ddddd
                                                     //                  ^   ^

      wcsncat(Title3,Title+(beg_t-2)+1,end_t-beg_t-1);  // !?$zz$xxxx(ffffff)ddddd
                                                        //            ^    ^
      wchar_t *CurStr= Title3;
      while (*CurStr)
      {
        if (*CurStr == L'!')
          CurStr=_SubstFileName(CurStr,PSubstData,tmp_t,10240-wcslen(tmp_t)-1);
        else                                           //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ��� ������ ��������!
        {
          wcsncat(tmp_t,CurStr,1);
          CurStr++;
        }
      }

      wcscat(Title1,tmp_t);
      wcscat(Title1,Title2);
      t = Title1;
    }
    //do it - ���� ����� ��� ��� �������� � �������������
    DlgData[DlgSize].strData = t;

    // ��������� ���� ����� �������� �������� - ���� ����
    wchar_t *s = Txt;
    *Txt = 0;
    *Title2 = 0;
    *Title3 = 0;
    *tmp_t = 0;

    if ((end-scr) > 1)  //if between ? and ! exist some
      wcsncat(Txt,(Str-2)+scr+1,(end-scr)-1);

    if ((beg_s == -1) || (end_s == -1))
    {
      //
    }
    else if ((end_s - beg_s) > 1) //if between ( and ) exist some
    {
      wcsncat(Txt1,Txt,beg_s-scr-1);   // !?$zz$xxxx(fffff)ddddd?rrrr(pppp)qqqqq!
                                       //                        ^  ^

      wcsncat(Title2,Txt+(end_s-scr),end-end_s-1); // !?$zz$xxxx(fffff)ddddd?rrrr(pppp)qqqqq!
                                                   //                                  ^   ^

      wcsncat(Title3,Txt+(beg_s-scr),end_s-beg_s-1);  // !?$zz$xxxx(ffffff)ddddd?rrrr(pppp)qqqqq!
                                                      //                              ^  ^

      wchar_t *CurStr= Title3;
      while (*CurStr)
      {
        if (*CurStr == L'!')
          CurStr=_SubstFileName(CurStr,PSubstData,tmp_t,10240-wcslen(tmp_t)-1);
        else                                           //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ��� ������ ��������!
        {
          wcsncat(tmp_t,CurStr,1);
          CurStr++;
        }
      }

      wcscat(Txt1,tmp_t);
      wcscat(Txt1,Title2);
      s = Txt1;
    }
    DlgData[DlgSize+1].strData = s;


    xf_free(Title1);
    xf_free(Txt1);
    xf_free(tmp_t);
    //<Skeleton>

    /* $ 01.08.2000 SVS
       "���������" ���������
    */
    apiExpandEnvironmentStrings (DlgData[DlgSize].strData,DlgData[DlgSize].strData);
    /* SVS $*/
    DlgSize+=2;
  }
  if (DlgSize==0)
    return 0;
  DlgData=(struct DialogItemEx *)xf_realloc(DlgData,(DlgSize+1)*sizeof(*DlgData));
  memset(&DlgData[DlgSize],0,sizeof(*DlgData));    //BUGBUGBUG
  DlgData[DlgSize].Type=DI_DOUBLEBOX;
  DlgData[DlgSize].X1=3;
  DlgData[DlgSize].Y1=1;
  DlgData[DlgSize].X2=72;
  DlgData[DlgSize].Y2=DlgSize+2;
  DlgSize++;

  int ExitCode;
  {
    Dialog Dlg(DlgData,DlgSize);
    Dlg.SetPosition(-1,-1,76,DlgSize+3);
    Dlg.Process();
    ExitCode=Dlg.GetExitCode();
  }

  if (ExitCode==-1)
  {
    xf_free(DlgData);
    *StartStr=0;
    return 0;
  }

  wchar_t TmpStr[4096];
  *TmpStr=0;
  for (Str=StartStr;*Str!=0;Str++)
  {
    int Replace=-1;
    //<Skeleton 2003 11 22>
    int end_pos=0;
    for (int I=0;I<StrPosSize;I++)
      if (Str-StartStr==StrPos[I])
      {
        Replace=I;
        end_pos = StrEndPos[I];
        break;
      }
    //<Skeleton>
    if (Replace!=-1)
    {
      wcscat(TmpStr,DlgData[Replace*2+1].strData);

      //<Skeleton 2003 11 20>
      //Str=strchr(Str+1,'!');
      Str = StartStr+end_pos;
      //<Skeleton>
    }
    else
      wcsncat(TmpStr,Str,1);
  }
  wcscpy(StartStr,TmpStr);

  ////ExpandEnvironmentStr(TmpStr,StartStr,sizeof(DlgData[0].Data)); ///BUGBUG
  xf_free(DlgData);

  return 1;
}

int Panel::MakeListFile(string &strListFileName,int ShortNames,const wchar_t *Modifers)
{
  FILE *ListFile;

  if (!FarMkTempExW(strListFileName) || (ListFile=_wfopen(strListFileName,L"wb"))==NULL)
  {
    MessageW(MSG_WARNING,1,UMSG(MError),UMSG(MCannotCreateListFile),UMSG(MCannotCreateListTemp),UMSG(MOk));
    return(FALSE);
  }

  string strFileName, strShortName;
  int FileAttr;
  GetSelNameW(NULL,FileAttr);
  while (GetSelNameW(&strFileName,FileAttr,&strShortName))
  {
    if (ShortNames)
      strFileName = strShortName;

    if(Modifers && *Modifers)
    {
      if(wcschr(Modifers,'F') && PointToNameW((const wchar_t*)strFileName) == (const wchar_t*)strFileName) // 'F' - ������������ ������ ����; //BUGBUG
      {
        string strTempFileName;
        strTempFileName.Format (L"%s%s%s", (const wchar_t*)strCurDir,(strCurDir.At(wcslen(strCurDir)-1) != L'\\'?L"\\":L""), (const wchar_t*)strFileName); //BUGBUG
        if (ShortNames)
          ConvertNameToShortW(strTempFileName, strTempFileName);
        strFileName = strTempFileName;
      }
      if(wcschr(Modifers,L'Q')) // 'Q' - ��������� ����� � ��������� � �������;
        QuoteSpaceOnlyW(strFileName);

      if(wcschr(Modifers,L'S')) // 'S' - ������������ '/' ������ '\' � ����� ������;
      {
        int I,Len=strFileName.GetLength();

        wchar_t *Name = strFileName.GetBuffer ();

        for(I=0; I < Len; ++I)
          if(Name[I] == L'\\')
            Name[I]=L'/';

        strFileName.ReleaseBuffer ();
      }
    }

    char *lpFileName = UnicodeToAnsi (strFileName);

//_D(SysLog("%s[%s] %s",__FILE__,Modifers,FileName));
    if (fprintf(ListFile,"%s\r\n", lpFileName)==EOF)
    {
      fclose(ListFile);
      DeleteFileW (strListFileName);
      MessageW(MSG_WARNING,1,UMSG(MError),UMSG(MCannotCreateListFile),UMSG(MCannotCreateListWrite),UMSG(MOk));
      return(FALSE);
    }

    xf_free (lpFileName);
  }
  if (fclose(ListFile)==EOF)
  {
    clearerr(ListFile);
    fclose(ListFile);
    DeleteFileW (strListFileName);
    MessageW(MSG_WARNING,1,UMSG(MError),UMSG(MCannotCreateListFile),UMSG(MOk));
    return(FALSE);
  }
  return(TRUE);
}

static int IsReplaceVariable(char *str,int *scr,
                                int *end,
                                int *beg_scr_break,
                                int *end_scr_break,
                                int *beg_txt_break,
                                int *end_txt_break)
// ��� ����� ������ - ����-�e 4 ��������� - ��� �������� �� str
// ������ ������ � ������ ��������, ����� ���� ������, ������ ������ � ������ ���������� ����������, �� � ����� �����.
// ������ ��� ������� ������ (������� � ��������� �����) ��� �������� ������:
// i = IsReplaceVariable(str) - ���� ��� ���� ������ ��������� ��������� ������ � ������ ?!
// ���  i - ��� ������, ������� ���� ���������, ���� �������� �� ����� ! ��������� !??!
{
  char *s      = str;
  char *scrtxt = str;

  int count_scob = 0;
  int second_count_scob = 0;

  bool was_quest_and_asterics = false; //  ?!
  bool was_quest = false;         //  ?
  bool was_asterics = false;      //  !

  bool in_firstpart_was_scob = false;
  char *beg_firstpart_scob = NULL;
  char *end_firstpart_scob = NULL;

  bool in_secondpart_was_scob = false;
  char *beg_secondpart_scob = NULL;
  char *end_secondpart_scob = NULL;

  if (!s)
    return -1;

  if (strncmp(s,"!?",2) == 0)
    s = s + 2;
  else
    return -1;
  //
  while (true)    // analize from !? to ?
  {
    if (!*s)
      return -1;

    if (*s == '(')
    {
      if (in_firstpart_was_scob)
      {
        //return -1;
      }
      else
      {
        in_firstpart_was_scob = true;
        beg_firstpart_scob = s;     //remember wher is first break
      }
      count_scob += 1;
    }
    else if (*s == ')')
    {
      count_scob -= 1;
      if (count_scob == 0)
      {
        if (!end_firstpart_scob)
          end_firstpart_scob = s;   //remember wher is last break
      }
      else if (count_scob < 0)
        return -1;
    }
    else if ((*s == '?') && ((!beg_firstpart_scob && !end_firstpart_scob) || (beg_firstpart_scob && end_firstpart_scob)))
    {
      was_quest = true;
    }

    s++;
    if (was_quest) break;
  }
  if (count_scob != 0) return -1;
  scrtxt = s - 1; //remember s for return

  while (true)    //analize from ? or !
  {
    if (!*s)
      return -1;

    if (*s == '(')
    {
      if (in_secondpart_was_scob)
      {
        //return -1;
      }
      else
      {
        in_secondpart_was_scob = true;
        beg_secondpart_scob = s;    //remember wher is first break
      }
      second_count_scob += 1;
    }
    else if (*s == ')')
    {
      second_count_scob -= 1;
      if (second_count_scob == 0)
      {
        if (!end_secondpart_scob)
          end_secondpart_scob = s;  //remember wher is last break
      }
      else if (second_count_scob < 0)
        return -1;
    }
    else if ((*s == '!') && ((!beg_secondpart_scob && !end_secondpart_scob) || (beg_secondpart_scob && end_secondpart_scob)))
    {
      was_asterics = true;
    }

    s++;
    if (was_asterics) break;
  }
  if (second_count_scob != 0) return -1;

  //
  if (scr != NULL)
    *scr = scrtxt - str;
  if (end != NULL)
    *end = (s - str) - 1;

  if (in_firstpart_was_scob)
  {
    if (beg_scr_break != NULL)
      *beg_scr_break = beg_firstpart_scob - str;
    if (end_scr_break != NULL)
      *end_scr_break = end_firstpart_scob - str;
  }
  else
  {
    if (beg_scr_break != NULL)
      *beg_scr_break = -1;
    if (end_scr_break != NULL)
      *end_scr_break = -1;
  }

  if (in_secondpart_was_scob)
  {
    if (beg_txt_break != NULL)
      *beg_txt_break = beg_secondpart_scob - str;
    if (end_txt_break != NULL)
      *end_txt_break = end_secondpart_scob - str;
  }
  else
  {
    if (beg_txt_break != NULL)
      *beg_txt_break = -1;
    if (end_txt_break != NULL)
      *end_txt_break = -1;
  }

  return ((s - str) - 1);
}

static int IsReplaceVariableW(const wchar_t *str,
                                int *scr,
                                int *end,
                                int *beg_scr_break,
                                int *end_scr_break,
                                int *beg_txt_break,
                                int *end_txt_break)
// ��� ����� ������ - ����-�e 4 ��������� - ��� �������� �� str
// ������ ������ � ������ ��������, ����� ���� ������, ������ ������ � ������ ���������� ����������, �� � ����� �����.
// ������ ��� ������� ������ (������� � ��������� �����) ��� �������� ������:
// i = IsReplaceVariable(str) - ���� ��� ���� ������ ��������� ��������� ������ � ������ ?!
// ���  i - ��� ������, ������� ���� ���������, ���� �������� �� ����� ! ��������� !??!
{
  const wchar_t *s      = str;
  const wchar_t *scrtxt = str;

  int count_scob = 0;
  int second_count_scob = 0;

  bool was_quest_and_asterics = false; //  ?!
  bool was_quest = false;         //  ?
  bool was_asterics = false;      //  !

  bool in_firstpart_was_scob = false;
  const wchar_t *beg_firstpart_scob = NULL;
  const wchar_t *end_firstpart_scob = NULL;

  bool in_secondpart_was_scob = false;
  const wchar_t *beg_secondpart_scob = NULL;
  const wchar_t *end_secondpart_scob = NULL;

  if (!s)
    return -1;

  if (wcsncmp(s,L"!?",2) == 0)
    s = s + 2;
  else
    return -1;
  //
  while (true)    // analize from !? to ?
  {
    if (!*s)
      return -1;

    if (*s == L'(')
    {
      if (in_firstpart_was_scob)
      {
        //return -1;
      }
      else
      {
        in_firstpart_was_scob = true;
        beg_firstpart_scob = s;     //remember wher is first break
      }
      count_scob += 1;
    }
    else if (*s == L')')
    {
      count_scob -= 1;
      if (count_scob == 0)
      {
        if (!end_firstpart_scob)
          end_firstpart_scob = s;   //remember wher is last break
      }
      else if (count_scob < 0)
        return -1;
    }
    else if ((*s == L'?') && ((!beg_firstpart_scob && !end_firstpart_scob) || (beg_firstpart_scob && end_firstpart_scob)))
    {
      was_quest = true;
    }

    s++;
    if (was_quest) break;
  }
  if (count_scob != 0) return -1;
  scrtxt = s - 1; //remember s for return

  while (true)    //analize from ? or !
  {
    if (!*s)
      return -1;

    if (*s == L'(')
    {
      if (in_secondpart_was_scob)
      {
        //return -1;
      }
      else
      {
        in_secondpart_was_scob = true;
        beg_secondpart_scob = s;    //remember wher is first break
      }
      second_count_scob += 1;
    }
    else if (*s == L')')
    {
      second_count_scob -= 1;
      if (second_count_scob == 0)
      {
        if (!end_secondpart_scob)
          end_secondpart_scob = s;  //remember wher is last break
      }
      else if (second_count_scob < 0)
        return -1;
    }
    else if ((*s == L'!') && ((!beg_secondpart_scob && !end_secondpart_scob) || (beg_secondpart_scob && end_secondpart_scob)))
    {
      was_asterics = true;
    }

    s++;
    if (was_asterics) break;
  }
  if (second_count_scob != 0) return -1;

  //
  if (scr != NULL)
    *scr = scrtxt - str;
  if (end != NULL)
    *end = (s - str) - 1;

  if (in_firstpart_was_scob)
  {
    if (beg_scr_break != NULL)
      *beg_scr_break = beg_firstpart_scob - str;
    if (end_scr_break != NULL)
      *end_scr_break = end_firstpart_scob - str;
  }
  else
  {
    if (beg_scr_break != NULL)
      *beg_scr_break = -1;
    if (end_scr_break != NULL)
      *end_scr_break = -1;
  }

  if (in_secondpart_was_scob)
  {
    if (beg_txt_break != NULL)
      *beg_txt_break = beg_secondpart_scob - str;
    if (end_txt_break != NULL)
      *end_txt_break = end_secondpart_scob - str;
  }
  else
  {
    if (beg_txt_break != NULL)
      *beg_txt_break = -1;
    if (end_txt_break != NULL)
      *end_txt_break = -1;
  }

  return ((s - str) - 1);
}
