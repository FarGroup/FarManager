/*
dizlist.cpp

Описания файлов
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "dizlist.hpp"
#include "language.hpp"
#include "lang.hpp"
#include "savescr.hpp"
#include "TPreRedrawFunc.hpp"
#include "TaskBar.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "config.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"

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
  Message(0,0,L"",MSG(MReadingDiz));
}

void DizList::Read(const wchar_t *Path, const wchar_t *DizName)
{
  Reset();
  TaskBar TB;
  TPreRedrawFuncGuard preRedrawFuncGuard(DizList::PR_ReadingMsg);

  const wchar_t *NamePtr=Opt.Diz.strListNames;

  while (1)
  {
    if (DizName!=NULL)
      strDizFileName = DizName;
    else
    {
      string strArgName;
      if ((NamePtr=GetCommaWord(NamePtr,strArgName))==NULL)
        break;

      strDizFileName = Path;

      AddEndSlash(strDizFileName);

      strDizFileName += strArgName;
    }

    FILE *DizFile;
    if ((DizFile=_wfopen(strDizFileName,L"rb"))!=NULL)
    {
      string strDizText;
      char DizText[MAX_DIZ_LENGTH]; //BUGBUG
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

        strDizText.SetData (DizText, CP_OEMCP); //BUGBUG
        RemoveTrailingSpaces(strDizText);
        AddRecord(strDizText);
      }

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
	DizRecord *NewDizData=DizData;
  if ((DizCount & 15)==0)
		NewDizData=(DizRecord *)xf_realloc(DizData,(DizCount+16+1)*sizeof(*DizData));

  if (NewDizData!=NULL)
  {
    DizData=NewDizData;
    DizData[DizCount].DizText=new wchar_t[StrLength(DizText)+1];
    wcscpy(DizData[DizCount].DizText,DizText);
    DizData[DizCount].Deleted=0;
    DizCount++;
  }
}


const wchar_t* DizList::GetDizTextAddr(const wchar_t *Name,const wchar_t *ShortName, const __int64 &FileSize)
{
  int TextPos;
  int DizPos=GetDizPosEx(Name,ShortName,&TextPos);
  wchar_t *DizText=NULL;

  if (DizPos!=-1)
  {
    DizText=DizData[DizPos].DizText+TextPos;
    while (*DizText && IsSpace(*DizText))
      DizText++;
    if (iswdigit(*DizText))
    {

      wchar_t SizeText[30], *DizPtr=DizText;
      int I,SkipSize;

      swprintf (SizeText, L"%I64u", FileSize);

      for (I=0,SkipSize=TRUE;SizeText[I]!=0;DizPtr++)
      {
        if (*DizPtr!=L',' && *DizPtr!=L'.')
        {
          if (SizeText[I++]!=*DizPtr)
          {
            SkipSize=FALSE;
            break;
          }
        }
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


int DizList::GetDizPosEx(const wchar_t *Name,const wchar_t *ShortName,int *TextPos)
{
  int DizPos=GetDizPos(Name,ShortName,TextPos);
  if (DizPos==-1)
  {
		string strQuotedName=Name, strQuotedShortName=ShortName;
		InsertQuote(strQuotedName);
		InsertQuote(strQuotedShortName);
    DizPos=GetDizPos(strQuotedName,strQuotedShortName,TextPos);
  }
  return(DizPos);
}


int DizList::GetDizPos(const wchar_t *Name,const wchar_t *ShortName,int *TextPos)
{
  string strQuotedName;

  strQuotedName = Name; // 3 - для кавычек

  QuoteSpaceOnly(strQuotedName);

  if (DizData==NULL)
    return(-1);

	DizRecord *DizRecordAddr;
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
        *TextPos=(int)strQuotedName.GetLength()+1;
        int DizLength=StrLength(DizRecordAddr->DizText);
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
          *TextPos=StrLength(ShortName)+1;
          int DizLength=StrLength(DizRecordAddr->DizText);
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
  const wchar_t *Diz1=SearchDizData[*(int *)el1].DizText;
  const wchar_t *Diz2=SearchDizData[*(int *)el2].DizText;
  return(StrCmpI(Diz1,Diz2));
}


int _cdecl SortDizSearch(const void *key,const void *elem)
{
  const wchar_t *SearchName=(const wchar_t *)key;
  const wchar_t *TableName=SearchDizData[*(int *)elem].DizText;
  int NameLength=StrLength(SearchName);
  int CmpCode=StrCmpNI(SearchName,TableName,NameLength);

  if (CmpCode==0)
  {
    int Ch=TableName[NameLength];
    if (Ch==0 || IsSpace(Ch))
      return(0);
    if (Ch==L'.')
    {
      int Ch1=TableName[NameLength+1];
      if (Ch1==0 || IsSpace(Ch1))
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
    if (*DizData[DizPos].DizText && !IsSpace(DizData[DizPos].DizText[0]))
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
    AddEndSlash(strDizFileName);

    string strArgName;
    GetCommaWord(Opt.Diz.strListNames,strArgName);

    strDizFileName += strArgName;
  }

  FILE *DizFile;
	DWORD FileAttr=apiGetFileAttributes(strDizFileName);

  if(FileAttr != INVALID_FILE_ATTRIBUTES)
  {
    if(Opt.Diz.ROUpdate && (FileAttr&FILE_ATTRIBUTE_READONLY))
			apiSetFileAttributes(strDizFileName,FileAttr&(~FILE_ATTRIBUTE_READONLY));

    if ((FileAttr & FILE_ATTRIBUTE_READONLY)==0)
			apiSetFileAttributes(strDizFileName,FILE_ATTRIBUTE_ARCHIVE);
  }

  if ((DizFile=_wfopen(strDizFileName,L"wb"))==NULL)
  {
    if(!Opt.Diz.ROUpdate && (FileAttr&FILE_ATTRIBUTE_READONLY))
      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotUpdateDiz),MSG(MCannotUpdateRODiz),MSG(MOk));
    else
      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotUpdateDiz),MSG(MOk));
    return(FALSE);
  }

  int AddedDizCount=0;
  for (int I=0;I<DizCount;I++)
    if (!DizData[I].Deleted)
    {
      int len = StrLength(DizData[I].DizText);
      char *lpDizText = (char *) xf_malloc(len+1);

      if (lpDizText)
      {
				WideCharToMultiByte (CP_OEMCP, 0, DizData[I].DizText, len+1, lpDizText, len+1, NULL, NULL);

				fprintf(DizFile,"%s\r\n", lpDizText);

				xf_free (lpDizText);

      	AddedDizCount++;
      }
    }

  int CloseCode=fclose(DizFile);
  if (AddedDizCount==0)
		apiDeleteFile(strDizFileName); //BUGBUG

  if (FileAttr==INVALID_FILE_ATTRIBUTES)
  {
    FileAttr=FILE_ATTRIBUTE_ARCHIVE;
    if (Opt.Diz.SetHidden)
      FileAttr|=FILE_ATTRIBUTE_HIDDEN;
  }

	apiSetFileAttributes(strDizFileName,FileAttr);

  if (CloseCode==EOF)
  {
    clearerr(DizFile);
    fclose(DizFile);
		apiDeleteFile(strDizFileName); //BUGBUG
    Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotUpdateDiz),MSG(MOk));
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

  while (IsSpace(DizData[DizPos].DizText[TextPos]))
    TextPos++;

  string strDizText, strQuotedName;

  strQuotedName = DestName;
  QuoteSpaceOnly(strQuotedName);
  int OptDizStartPos=(Opt.Diz.StartPos>1 ? Opt.Diz.StartPos-2:0);
  int LenQuotedName=(int)strQuotedName.GetLength ();

  if(!OptDizStartPos || OptDizStartPos < LenQuotedName)
    OptDizStartPos=(int)strQuotedName.GetLength();

  strDizText.Format (L"%-*.*s %.*s",
       OptDizStartPos,OptDizStartPos,(const wchar_t*)strQuotedName,
       NM/*(sizeof(DizText)-OptDizStartPos-2)*/,&DizData[DizPos].DizText[TextPos]); //BUGBUG

  DestDiz->AddDiz(DestName,DestShortName,strDizText);
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


void DizList::GetDizName(string &strDizName)
{
  strDizName = strDizFileName;
}
