/*
dizlist.cpp

Описания файлов

*/

/* Revision: 1.16 05.04.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "dizlist.hpp"
#include "fn.hpp"
#include "global.hpp"
#include "lang.hpp"
#include "savescr.hpp"


static int _cdecl SortDizIndex(const void *el1,const void *el2);
int _cdecl SortDizSearch(const void *key,const void *elem);

#define MAX_DIZ_LENGTH  8192

static DizRecord *SearchDizData;

DizList::DizList()
{
  DizData=NULL;
  DizCount=0;
  IndexData=NULL;
  IndexCount=0;
  strDizFileName=L"";
}


DizList::~DizList()
{
  Reset();
}


void DizList::Reset()
{
  for (int I=0;I<DizCount;I++)
    if(DizData[I].DizText)
      delete[] DizData[I].DizText;
  if(DizData)
    xf_free(DizData);
  DizData=NULL;
  DizCount=0;
  if(IndexData)
    delete[] IndexData;
  IndexData=NULL;
  IndexCount=0;
}

void DizList::PR_ReadingMsg(void)
{
  MessageW(0,0,L"",UMSG(MReadingDiz));
}

void DizList::Read(const wchar_t *Path, const wchar_t *DizName)
{
  Reset();

  const wchar_t *NamePtr=Opt.Diz.strListNames;

  while (1)
  {
    if (DizName!=NULL)
      strDizFileName = DizName;
    else
    {
      string strArgName;
      if ((NamePtr=GetCommaWordW(NamePtr,strArgName))==NULL)
        break;

      strDizFileName = Path;

      AddEndSlashW(strDizFileName);

      strDizFileName += strArgName;
    }

    FILE *DizFile;
    if ((DizFile=_wfopen(strDizFileName,L"rb"))!=NULL)
    {
      string strDizText;
      char DizText[MAX_DIZ_LENGTH]; //BUGBUG
      //SaveScreen *SaveScr=NULL;
      clock_t StartTime=clock();

      SetPreRedrawFunc(DizList::PR_ReadingMsg);
      while (fgets(DizText,sizeof(DizText),DizFile)!=NULL)
      {
        if ((DizCount & 127)==0 && clock()-StartTime>1000)
        {
          //if (SaveScr==NULL)
          {
            //SaveScr=new SaveScreen;
            SetCursorType(FALSE,0);
            PR_ReadingMsg();
          }
          if (CheckForEsc())
            break;
        }

        strDizText.SetData (DizText, CP_OEMCP); //BUGBUG
        RemoveTrailingSpacesW(strDizText);
        AddRecord(strDizText);
      }
      SetPreRedrawFunc(NULL);

      //delete SaveScr;
      fclose(DizFile);
      BuildIndex();
      return;
    }
    if (DizName!=NULL)
      break;
  }
  strDizFileName=L"";
}


void DizList::AddRecord(const wchar_t *DizText)
{
  struct DizRecord *NewDizData=DizData;
  if ((DizCount & 15)==0)
    NewDizData=(struct DizRecord *)xf_realloc(DizData,(DizCount+16+1)*sizeof(*DizData));

  if (NewDizData!=NULL)
  {
    DizData=NewDizData;
    DizData[DizCount].DizText=new wchar_t[wcslen(DizText)+1];
    wcscpy(DizData[DizCount].DizText,DizText);
    DizData[DizCount].Deleted=0;
    DizCount++;
  }
}


const wchar_t* DizList::GetDizTextAddr(const wchar_t *Name,const wchar_t *ShortName,DWORD FileSize)
{
  int TextPos;
  int DizPos=GetDizPosEx(Name,ShortName,&TextPos);
  wchar_t *DizText=NULL;

  if (DizPos!=-1)
  {
    DizText=DizData[DizPos].DizText+TextPos;
    while (*DizText && IsSpaceW(*DizText))
      DizText++;
    if (iswdigit(*DizText))
    {

      wchar_t SizeText[20], *DizPtr=DizText;
      string strSizeText;
      int I,SkipSize;

      swprintf (SizeText, L"%d", FileSize);

      for (I=0,SkipSize=TRUE;SizeText[I]!=0;DizPtr++)
        if (*DizPtr!=L',' && *DizPtr!=L'.')
          if (SizeText[I++]!=*DizPtr)
          {
            SkipSize=FALSE;
            break;
          }
      if (SkipSize && IsSpaceW(*DizPtr))
      {
        DizText=DizPtr;
        while (*DizText && IsSpaceW(*DizText))
          DizText++;
      }
    }
  }
  return(DizText);
}


int DizList::GetDizPosEx(const wchar_t *Name,const wchar_t *ShortName,int *TextPos)
{
  int DizPos=GetDizPos(Name,ShortName,TextPos);
  if (DizPos==-1)
  {
    string strQuotedName, strQuotedShortName;

    strQuotedName.Format (L"\"%s\"", Name);
    strQuotedShortName.Format (L"\"%s\"", ShortName);
    DizPos=GetDizPos(strQuotedName,strQuotedShortName,TextPos);
  }
  return(DizPos);
}


int DizList::GetDizPos(const wchar_t *Name,const wchar_t *ShortName,int *TextPos)
{
  string strQuotedName;

  strQuotedName = Name; // 3 - для кавычек

  QuoteSpaceOnlyW (strQuotedName);

  if (DizData==NULL)
    return(-1);

  struct DizRecord *DizRecordAddr;
  int *DestIndex;
  SearchDizData=DizData;
  DestIndex=(int *)bsearch((const wchar_t*)strQuotedName,IndexData,IndexCount,sizeof(*IndexData),SortDizSearch);

  if (DestIndex!=NULL)
  {
    DizRecordAddr=&DizData[*DestIndex];
    if (!DizRecordAddr->Deleted)
    {
      if (TextPos!=NULL)
      {
        *TextPos=strQuotedName.GetLength()+1;
        int DizLength=wcslen(DizRecordAddr->DizText);
        if (*TextPos>DizLength)
          *TextPos=DizLength;
      }
      return(*DestIndex);
    }
  }

  if (*ShortName)
  {
    DestIndex=(int *)bsearch(ShortName,IndexData,IndexCount,sizeof(*IndexData),SortDizSearch);
    if (DestIndex!=NULL)
    {
      DizRecordAddr=&DizData[*DestIndex];
      if (!DizRecordAddr->Deleted)
      {
        if (TextPos!=NULL)
        {
          *TextPos=wcslen(ShortName)+1;
          int DizLength=wcslen(DizRecordAddr->DizText);
          if (*TextPos>DizLength)
            *TextPos=DizLength;
        }
        return(*DestIndex);
      }
    }
  }
  return(-1);
}


void DizList::BuildIndex()
{
  /* $ 13.07.2000 SVS
       раз уж вызвали new[], то в придачу и delete[] надо... */
  if(IndexData)
    delete[] IndexData;
  /* SVS $ */
  if ((IndexData=new int[DizCount])==NULL)
  {
    Reset();
    return;
  }
  IndexCount=DizCount;

  for (int I=0;I<IndexCount;I++)
    IndexData[I]=I;

  SearchDizData=DizData;
  far_qsort((void *)IndexData,IndexCount,sizeof(*IndexData),SortDizIndex);
}


int _cdecl SortDizIndex(const void *el1,const void *el2)
{
  const wchar_t *Diz1=SearchDizData[*(int *)el1].DizText;
  const wchar_t *Diz2=SearchDizData[*(int *)el2].DizText;
  return(LocalStricmpW(Diz1,Diz2));
}


int _cdecl SortDizSearch(const void *key,const void *elem)
{
  const wchar_t *SearchName=(const wchar_t *)key;
  const wchar_t *TableName=SearchDizData[*(int *)elem].DizText;
  int NameLength=wcslen(SearchName);
  int CmpCode=LocalStrnicmpW(SearchName,TableName,NameLength);

  if (CmpCode==0)
  {
    int Ch=TableName[NameLength];
    if (Ch==0 || IsSpaceW(Ch))
      return(0);
    if (Ch==L'.')
    {
      int Ch1=TableName[NameLength+1];
      if (Ch1==0 || IsSpaceW(Ch1))
        return(0);
    }
    return(-1);
  }
  return(CmpCode);
}


int DizList::DeleteDiz(const wchar_t *Name,const wchar_t *ShortName)
{
  int DizPos=GetDizPosEx(Name,ShortName,NULL);
  if (DizPos==-1)
    return(FALSE);

  DizData[DizPos++].Deleted=TRUE;
  while (DizPos<DizCount)
  {
    if (*DizData[DizPos].DizText && !IsSpaceW(DizData[DizPos].DizText[0]))
      break;
    DizData[DizPos].Deleted=TRUE;
    DizPos++;
  }
  return(TRUE);
}


int DizList::Flush(const wchar_t *Path,const wchar_t *DizName)
{
  if (DizName!=NULL)
    strDizFileName = DizName;
  else if ( strDizFileName.IsEmpty() )
  {
    if (DizData==NULL || Path==NULL)
      return(FALSE);

    strDizFileName = Path;
    AddEndSlashW(strDizFileName);

    string strArgName;
    GetCommaWordW(Opt.Diz.strListNames,strArgName);

    strDizFileName += strArgName;
  }

  FILE *DizFile;
  int FileAttr=GetFileAttributesW(strDizFileName);

  if(FileAttr != -1)
  {
    if(Opt.Diz.ROUpdate && (FileAttr&FA_RDONLY))
      SetFileAttributesW(strDizFileName,FileAttr&(~FA_RDONLY));

    if ((FileAttr & FA_RDONLY)==0)
      SetFileAttributesW(strDizFileName,FA_ARCH);
  }

  if ((DizFile=_wfopen(strDizFileName,L"wb"))==NULL)
  {
    if(!Opt.Diz.ROUpdate && (FileAttr&FA_RDONLY))
      MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MError),UMSG(MCannotUpdateDiz),UMSG(MCannotUpdateRODiz),UMSG(MOk));
    else
      MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MError),UMSG(MCannotUpdateDiz),UMSG(MOk));
    return(FALSE);
  }

  int AddedDizCount=0;
  for (int I=0;I<DizCount;I++)
    if (!DizData[I].Deleted)
    {
      char *lpDizText = UnicodeToAnsi (DizData[I].DizText); //BUGBUG

      fprintf(DizFile,"%s\r\n", lpDizText);

      xf_free (lpDizText);

      AddedDizCount++;
    }

  int CloseCode=fclose(DizFile);
  if (AddedDizCount==0)
    DeleteFileW(strDizFileName); //BUGBUG

  if (FileAttr==-1)
  {
    FileAttr=FA_ARCH;
    if (Opt.Diz.SetHidden)
      FileAttr|=FA_HIDDEN;
  }

  SetFileAttributesW(strDizFileName,FileAttr);

  if (CloseCode==EOF)
  {
    clearerr(DizFile);
    fclose(DizFile);
    DeleteFileW(strDizFileName); //BUGBUG
    MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MError),UMSG(MCannotUpdateDiz),UMSG(MOk));
    return(FALSE);
  }

  return(TRUE);
}


void DizList::AddDiz(const wchar_t *Name,const wchar_t *ShortName,const wchar_t *DizText)
{
  DeleteDiz(Name,ShortName);
  AddRecord(DizText);
}


int DizList::CopyDiz(const wchar_t *Name,const wchar_t *ShortName,const wchar_t *DestName,
                     const wchar_t *DestShortName,DizList *DestDiz)
{
  int TextPos;
  int DizPos=GetDizPosEx(Name,ShortName,&TextPos);
  if (DizPos==-1)
    return(FALSE);

  while (IsSpaceW(DizData[DizPos].DizText[TextPos]))
    TextPos++;

  string strDizText, strQuotedName;

  strQuotedName = DestName;
  QuoteSpaceOnlyW(strQuotedName);
  int OptDizStartPos=(Opt.Diz.StartPos>1 ? Opt.Diz.StartPos-2:0);
  int LenQuotedName=strQuotedName.GetLength ();

  if(!OptDizStartPos || OptDizStartPos < LenQuotedName)
    OptDizStartPos=strQuotedName.GetLength();

  strDizText.Format (L"%-*.*s %.*s",
       OptDizStartPos,OptDizStartPos,(const wchar_t*)strQuotedName,
       NM/*(sizeof(DizText)-OptDizStartPos-2)*/,&DizData[DizPos].DizText[TextPos]); //BUGBUG

  DestDiz->AddDiz(DestName,DestShortName,strDizText);
  while (++DizPos<DizCount)
  {
    if (*DizData[DizPos].DizText && !IsSpaceW(DizData[DizPos].DizText[0]))
      break;
    DestDiz->AddDiz(DestName,DestShortName,DizData[DizPos].DizText);
  }
  // BugZ#455 - Некорректная работа описалова при копировании с добавлением
  DestDiz->BuildIndex();
  return(TRUE);
}


void DizList::GetDizName(string &strDizName)
{
  strDizName = strDizFileName;
}
