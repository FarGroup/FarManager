/*
dizlist.cpp

Описания файлов

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#define STRICT

#if !defined(_INC_WINDOWS) && !defined(_WINDOWS_)
#include <windows.h>
#endif
#ifndef __STRING_H
#include <string.h>
#endif
#ifndef __DIR_H
#include <dir.h>	// chdir
#endif
#ifndef __DOS_H
#include <dos.h>	// FA_*
#endif
#if !defined(__NEW_H)
#pragma option -p-
#include <new.h>
#pragma option -p.
#endif

#ifndef __FARCONST_HPP__
#include "farconst.hpp"
#endif
#ifndef __KEYS_HPP__
#include "keys.hpp"
#endif
#ifndef __FARLANG_HPP__
#include "lang.hpp"
#endif
#ifndef __COLOROS_HPP__
#include "colors.hpp"
#endif
#ifndef __FARSTRUCT_HPP__
#include "struct.hpp"
#endif
#ifndef __PLUGIN_HPP__
#include "plugin.hpp"
#endif
#ifndef __CLASSES_HPP__
#include "classes.hpp"
#endif
#ifndef __FARFUNC_HPP__
#include "fn.hpp"
#endif
#ifndef __FARGLOBAL_HPP__
#include "global.hpp"
#endif

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
    delete DizData[I].DizText;
  delete DizData;
  DizData=NULL;
  DizCount=0;
  delete IndexData;
  IndexData=NULL;
  IndexCount=0;
}


void DizList::Read(char *Path,char *DizName)
{
  Reset();

  char *NamePtr=Opt.Diz.ListNames;
  while (1)
  {
    if (DizName!=NULL)
      strcpy(DizFileName,DizName);
    else
    {
      char ArgName[NM];
      if ((NamePtr=GetCommaWord(NamePtr,ArgName))==NULL)
        break;
      strcpy(DizFileName,Path);
      AddEndSlash(DizFileName);
      strcat(DizFileName,ArgName);
    }
    FILE *DizFile;
    if ((DizFile=fopen(DizFileName,"rb"))!=NULL)
    {
      char DizText[MAX_DIZ_LENGTH];
      SaveScreen *SaveScr=NULL;
      clock_t StartTime=clock();
      while (fgets(DizText,sizeof(DizText),DizFile)!=NULL)
      {
        if ((DizCount & 127)==0 && clock()-StartTime>1000)
        {
          if (SaveScr==NULL)
          {
            SaveScr=new SaveScreen;
            SetCursorType(FALSE,0);
            Message(0,0,"",MSG(MReadingDiz));
          }
          if (CheckForEsc())
            break;
        }
        RemoveTrailingSpaces(DizText);
        AddRecord(DizText);
      }
      delete SaveScr;
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
    NewDizData=(struct DizRecord *)realloc(DizData,(DizCount+16+1)*sizeof(*DizData));
  if (NewDizData!=NULL)
  {
    DizData=NewDizData;
    DizData[DizCount].DizText=new char[strlen(DizText)+1];
    strcpy(DizData[DizCount].DizText,DizText);
    DizData[DizCount].Deleted=0;
    DizCount++;
  }
}


char* DizList::GetDizTextAddr(char *Name,char *ShortName,DWORD FileSize)
{
  int TextPos;
  int DizPos=GetDizPosEx(Name,ShortName,&TextPos);
  char *DizText=NULL;
  if (DizPos!=-1)
  {
    DizText=DizData[DizPos].DizText+TextPos;
    while (*DizText && isspace(*DizText))
      DizText++;
    if (isdigit(*DizText))
    {
      char SizeText[20],*DizPtr=DizText;
      int I,SkipSize;
      sprintf(SizeText,"%d",FileSize);
      for (I=0,SkipSize=TRUE;SizeText[I]!=0;DizPtr++)
        if (*DizPtr!=',' && *DizPtr!='.')
          if (SizeText[I++]!=*DizPtr)
          {
            SkipSize=FALSE;
            break;
          }
      if (SkipSize && isspace(*DizPtr))
      {
        DizText=DizPtr;
        while (*DizText && isspace(*DizText))
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
    sprintf(QuotedName,"\"%s\"",Name);
    sprintf(QuotedShortName,"\"%s\"",ShortName);
    DizPos=GetDizPos(QuotedName,QuotedShortName,TextPos);
  }
  return(DizPos);
}


int DizList::GetDizPos(char *Name,char *ShortName,int *TextPos)
{
  char QuotedName[NM];
  strcpy(QuotedName,Name);
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
        *TextPos=strlen(QuotedName)+1;
        int DizLength=strlen(DizRecordAddr->DizText);
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
          *TextPos=strlen(ShortName)+1;
          int DizLength=strlen(DizRecordAddr->DizText);
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
  delete IndexData;
  if ((IndexData=new int[DizCount])==NULL)
  {
    Reset();
    return;
  }
  IndexCount=DizCount;
  for (int I=0;I<IndexCount;I++)
    IndexData[I]=I;
  SearchDizData=DizData;
  qsort((void *)IndexData,IndexCount,sizeof(*IndexData),SortDizIndex);
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
  int NameLength=strlen(SearchName);
  int CmpCode=LocalStrnicmp(SearchName,TableName,NameLength);
  if (CmpCode==0)
  {
    int Ch=TableName[NameLength];
    if (Ch==0 || isspace(Ch))
      return(0);
    if (Ch=='.')
    {
      int Ch1=TableName[NameLength+1];
      if (Ch1==0 || isspace(Ch1))
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
    if (*DizData[DizPos].DizText && !isspace(DizData[DizPos].DizText[0]))
      break;
    DizData[DizPos].Deleted=TRUE;
    DizPos++;
  }
  return(TRUE);
}


int DizList::Flush(char *Path,char *DizName)
{
  if (DizName!=NULL)
    strcpy(DizFileName,DizName);
  else
    if (*DizFileName==0)
    {
      if (DizData==NULL || Path==NULL)
        return(FALSE);
      strcpy(DizFileName,Path);
      AddEndSlash(DizFileName);
      char ArgName[NM];
      GetCommaWord(Opt.Diz.ListNames,ArgName);
      strcat(DizFileName,ArgName);
    }
  FILE *DizFile;
  int FileAttr=GetFileAttributes(DizFileName);
  if ((FileAttr & FA_RDONLY)==0)
    SetFileAttributes(DizFileName,FA_ARCH);
  if ((DizFile=fopen(DizFileName,"wb"))==NULL)
  {
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
  while (isspace(DizData[DizPos].DizText[TextPos]))
    TextPos++;
  char DizText[MAX_DIZ_LENGTH+NM],QuotedName[NM];
  strcpy(QuotedName,DestName);
  QuoteSpaceOnly(QuotedName);
  sprintf(DizText,"%-*s %s",Opt.Diz.StartPos>1 ? Opt.Diz.StartPos-2:0,QuotedName,&DizData[DizPos].DizText[TextPos]);
  DestDiz->AddDiz(DestName,DestShortName,DizText);
  while (++DizPos<DizCount)
  {
    if (*DizData[DizPos].DizText && !isspace(DizData[DizPos].DizText[0]))
      break;
    DestDiz->AddDiz(DestName,DestShortName,DizData[DizPos].DizText);
  }
  return(TRUE);
}


void DizList::GetDizName(char *DizName)
{
  strcpy(DizName,DizFileName);
}

