/*
dizlist.cpp

Описания файлов

*/

#include "headers.hpp"
#pragma hdrstop

#include "dizlist.hpp"
#include "fn.hpp"
#include "global.hpp"
#include "lang.hpp"
#include "savescr.hpp"
#include "TPreRedrawFunc.hpp"
#include "TaskBar.hpp"

struct DizRecord
{
  char *DizText;
  int Deleted;
};

static int _cdecl SortDizIndex(const void *el1,const void *el2);
int _cdecl SortDizSearch(const void *key,const void *elem);

#define MAX_DIZ_LENGTH  8192

static struct DizRecord *SearchDizData;

DizList::DizList()
{
  DizData=NULL;
  DizCount=0;
  IndexData=NULL;
  IndexCount=0;
  *DizFileName=0;
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
  Message(0,0,"",MSG(MReadingDiz));
}

void DizList::Read(char *Path,char *DizName)
{
  Reset();
  TaskBar TB;
  TPreRedrawFuncGuard preRedrawFuncGuard(DizList::PR_ReadingMsg);

  const char *NamePtr=Opt.Diz.ListNames;

  while (1)
  {
    if (DizName!=NULL)
      xstrncpy(DizFileName,DizName,sizeof(DizFileName)-1);
    else
    {
      char ArgName[NM];
      if ((NamePtr=GetCommaWord(NamePtr,ArgName))==NULL)
        break;
      xstrncpy(DizFileName,Path,sizeof(DizFileName)-2); // 2 - на слешь
      AddEndSlash(DizFileName);
      if(strlen(DizFileName)+strlen(ArgName) < sizeof(DizFileName))
        strcat(DizFileName,ArgName);
      else
        break;
    }

    FILE *DizFile;
    if ((DizFile=fopen(DizFileName,"rb"))!=NULL)
    {
      char DizText[MAX_DIZ_LENGTH];
      //SaveScreen *SaveScr=NULL;
      clock_t StartTime=clock();

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
        RemoveTrailingSpaces(DizText);
        AddRecord(DizText);
      }

      //delete SaveScr;
      fclose(DizFile);
      BuildIndex();
      return;
    }
    if (DizName!=NULL)
      break;
  }

  *DizFileName=0;

}


void DizList::AddRecord(char *DizText)
{
  struct DizRecord *NewDizData=DizData;
  if ((DizCount & 15)==0)
    NewDizData=(struct DizRecord *)xf_realloc(DizData,(DizCount+16+1)*sizeof(*DizData));

  if (NewDizData!=NULL)
  {
    DizData=NewDizData;
    DizData[DizCount].DizText=new char[strlen(DizText)+1];
    strcpy(DizData[DizCount].DizText,DizText);
    DizData[DizCount].Deleted=0;
    DizCount++;
  }
}


char* DizList::GetDizTextAddr(char *Name,char *ShortName,unsigned __int64 FileSize)
{
  int TextPos;
  int DizPos=GetDizPosEx(Name,ShortName,&TextPos);
  char *DizText=NULL;

  if (DizPos!=-1)
  {
    DizText=DizData[DizPos].DizText+TextPos;
    while (*DizText && IsSpace(*DizText))
      DizText++;
    if (isdigit(*DizText))
    {
      char SizeText[20],*DizPtr=DizText;
      int I,SkipSize;
      sprintf(SizeText,"%I64u",FileSize);
      for (I=0,SkipSize=TRUE;SizeText[I]!=0;DizPtr++)
        if (*DizPtr!=',' && *DizPtr!='.')
          if (SizeText[I++]!=*DizPtr)
          {
            SkipSize=FALSE;
            break;
          }
      if (SkipSize && IsSpace(*DizPtr))
      {
        DizText=DizPtr;
        while (*DizText && IsSpace(*DizText))
          DizText++;
      }
    }
  }
  return(DizText);
}


int DizList::GetDizPosEx(char *Name,char *ShortName,int *TextPos)
{
  int DizPos=GetDizPos(Name,ShortName,TextPos);
  if (DizPos==-1)
  {
    char QuotedName[2*NM],QuotedShortName[2*NM];
    sprintf(QuotedName,"\"%.*s\"",sizeof(QuotedName)-4,Name);
    sprintf(QuotedShortName,"\"%s\"",ShortName);
    DizPos=GetDizPos(QuotedName,QuotedShortName,TextPos);
  }
  return(DizPos);
}


int DizList::GetDizPos(char *Name,char *ShortName,int *TextPos)
{
  char QuotedName[NM];
  xstrncpy(QuotedName,Name,sizeof(QuotedName)-3); // 3 - для кавычек
  QuoteSpaceOnly(QuotedName);
  if (DizData==NULL)
    return(-1);

  struct DizRecord *DizRecordAddr;
  int *DestIndex;
  SearchDizData=DizData;
  DestIndex=(int *)bsearch(QuotedName,IndexData,IndexCount,sizeof(*IndexData),SortDizSearch);

  if (DestIndex!=NULL)
  {
    DizRecordAddr=&DizData[*DestIndex];
    if (!DizRecordAddr->Deleted)
    {
      if (TextPos!=NULL)
      {
        *TextPos=(int)strlen(QuotedName)+1;
        int DizLength=(int)strlen(DizRecordAddr->DizText);
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
          *TextPos=(int)strlen(ShortName)+1;
          int DizLength=(int)strlen(DizRecordAddr->DizText);
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
  if(IndexData)
    delete[] IndexData;

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
  char *Diz1=SearchDizData[*(int *)el1].DizText;
  char *Diz2=SearchDizData[*(int *)el2].DizText;
  return(LocalStricmp(Diz1,Diz2));
}


int _cdecl SortDizSearch(const void *key,const void *elem)
{
  char *SearchName=(char *)key;
  char *TableName=SearchDizData[*(int *)elem].DizText;
  int NameLength=(int)strlen(SearchName);
  int CmpCode=LocalStrnicmp(SearchName,TableName,NameLength);

  if (CmpCode==0)
  {
    int Ch=TableName[NameLength];
    if (Ch==0 || IsSpace(Ch))
      return(0);
    if (Ch=='.')
    {
      int Ch1=TableName[NameLength+1];
      if (Ch1==0 || IsSpace(Ch1))
        return(0);
    }
    return(-1);
  }
  return(CmpCode);
}


int DizList::DeleteDiz(char *Name,char *ShortName)
{
  int DizPos=GetDizPosEx(Name,ShortName,NULL);
  if (DizPos==-1)
    return(FALSE);

  DizData[DizPos++].Deleted=TRUE;
  while (DizPos<DizCount)
  {
    if (*DizData[DizPos].DizText && !IsSpace(DizData[DizPos].DizText[0]))
      break;
    DizData[DizPos].Deleted=TRUE;
    DizPos++;
  }
  return(TRUE);
}


int DizList::Flush(char *Path,char *DizName)
{
  if (DizName!=NULL)
    xstrncpy(DizFileName,DizName,sizeof(DizFileName)-1);
  else if (*DizFileName==0)
  {
    if (DizData==NULL || Path==NULL)
      return(FALSE);

    xstrncpy(DizFileName,Path,sizeof(DizFileName)-2);
    AddEndSlash(DizFileName);

    char ArgName[NM];
    GetCommaWord(Opt.Diz.ListNames,ArgName);
    if(strlen(DizFileName)+strlen(ArgName) < sizeof(DizFileName))
      strcat(DizFileName,ArgName);
    else
    {
      Message(MSG_WARNING,1,MSG(MError),MSG(MErrorFullPathNameLong),MSG(MCannotUpdateDiz),MSG(MOk));
      return(FALSE);
    }
  }

  FILE *DizFile;
  int FileAttr=GetFileAttributes(DizFileName);

  if(FileAttr != -1)
  {
    if(Opt.Diz.ROUpdate && (FileAttr&FA_RDONLY))
      SetFileAttributes(DizFileName,FileAttr&(~FA_RDONLY));

    if ((FileAttr & FA_RDONLY)==0)
      SetFileAttributes(DizFileName,FA_ARCH);
  }

  if ((DizFile=fopen(DizFileName,"wb"))==NULL)
  {
    if(!Opt.Diz.ROUpdate && (FileAttr&FA_RDONLY))
      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotUpdateDiz),MSG(MCannotUpdateRODiz),MSG(MOk));
    else
      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotUpdateDiz),MSG(MOk));
    return(FALSE);
  }

  int AddedDizCount=0;
  for (int I=0;I<DizCount;I++)
    if (!DizData[I].Deleted)
    {
      fprintf(DizFile,"%s\r\n",DizData[I].DizText);
      AddedDizCount++;
    }

  int CloseCode=fclose(DizFile);
  if (AddedDizCount==0)
    remove(DizFileName);

  if (FileAttr==-1)
  {
    FileAttr=FA_ARCH;
    if (Opt.Diz.SetHidden)
      FileAttr|=FA_HIDDEN;
  }

  SetFileAttributes(DizFileName,FileAttr);

  if (CloseCode==EOF)
  {
    clearerr(DizFile);
    fclose(DizFile);
    remove(DizFileName);
    Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotUpdateDiz),MSG(MOk));
    return(FALSE);
  }

  return(TRUE);
}


void DizList::AddDiz(char *Name,char *ShortName,char *DizText)
{
  DeleteDiz(Name,ShortName);
  AddRecord(DizText);
}


int DizList::CopyDiz(char *Name,char *ShortName,char *DestName,
                     char *DestShortName,DizList *DestDiz)
{
  int TextPos;
  int DizPos=GetDizPosEx(Name,ShortName,&TextPos);
  if (DizPos==-1)
    return(FALSE);

  while (IsSpace(DizData[DizPos].DizText[TextPos]))
    TextPos++;

  char DizText[MAX_DIZ_LENGTH+NM],QuotedName[NM];
  xstrncpy(QuotedName,DestName,sizeof(QuotedName)-3);
  QuoteSpaceOnly(QuotedName);
  int OptDizStartPos=(Opt.Diz.StartPos>1 ? Opt.Diz.StartPos-2:0);
  int LenQuotedName=(int)strlen(QuotedName);

  if(!OptDizStartPos || OptDizStartPos < LenQuotedName)
    OptDizStartPos=(int)strlen(QuotedName);

  sprintf(DizText,"%-*.*s %.*s",
       OptDizStartPos,OptDizStartPos,QuotedName,
       (sizeof(DizText)-OptDizStartPos-2),&DizData[DizPos].DizText[TextPos]);

  DestDiz->AddDiz(DestName,DestShortName,DizText);
  while (++DizPos<DizCount)
  {
    if (*DizData[DizPos].DizText && !IsSpace(DizData[DizPos].DizText[0]))
      break;
    DestDiz->AddDiz(DestName,DestShortName,DizData[DizPos].DizText);
  }
  // BugZ#455 - Некорректная работа описалова при копировании с добавлением
  DestDiz->BuildIndex();
  return(TRUE);
}


void DizList::GetDizName(char *DizName)
{
  strcpy(DizName,DizFileName);
}
