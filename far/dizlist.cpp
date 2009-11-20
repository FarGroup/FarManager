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
#include "filestr.hpp"
#include "codepage.hpp"

static DizRecord *SearchDizData;
static int _cdecl SortDizIndex(const void *el1,const void *el2);
static int _cdecl SortDizSearch(const void *key,const void *elem);
struct DizSearchKey
{
	const wchar_t *Str;
	const int Len;
};

DizList::DizList()
{
	DizData=NULL;
	DizCount=0;
	IndexData=NULL;
	IndexCount=0;
	strDizFileName=L"";
	Modified=false;
	NeedRebuild=true;
	OrigCodePage=CP_AUTODETECT;
	AnsiBuf=NULL;
}

DizList::~DizList()
{
	Reset();
	if (AnsiBuf)
		xf_free(AnsiBuf);
}

void DizList::Reset()
{
	for (int I=0;I<DizCount;I++)
		if (DizData[I].DizText)
			xf_free(DizData[I].DizText);

	if (DizData)
		xf_free(DizData);
	DizData=NULL;
	DizCount=0;

	if (IndexData)
		xf_free(IndexData);
	IndexData=NULL;
	IndexCount=0;

	Modified=false;
	NeedRebuild=true;
	OrigCodePage=CP_AUTODETECT;
}

void DizList::PR_ReadingMsg()
{
	Message(0,0,L"",MSG(MReadingDiz));
}

void DizList::Read(const wchar_t *Path, const wchar_t *DizName)
{
	Reset();
	TaskBar TB;
	TPreRedrawFuncGuard preRedrawFuncGuard(DizList::PR_ReadingMsg);

	const wchar_t *NamePtr=Opt.Diz.strListNames;

	for(;;)
	{
		if (DizName!=NULL)
		{
			strDizFileName = DizName;
		}
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
			GetFileString GetStr(DizFile);
			wchar_t *DizText;
			int DizLength;
			clock_t StartTime=clock();
			UINT CodePage=CP_AUTODETECT;
			bool bSigFound=false;

			if (!GetFileFormat(DizFile,CodePage,&bSigFound,false) || !bSigFound)
				CodePage = Opt.Diz.AnsiByDefault ? CP_ACP : CP_OEMCP;

			while (GetStr.GetString(&DizText, CodePage, DizLength) > 0)
			{
				if ((DizCount & 127)==0 && clock()-StartTime>1000)
				{
					SetCursorType(FALSE,0);
					PR_ReadingMsg();

					if (CheckForEsc())
						break;
				}

				RemoveTrailingSpaces(DizText);
				if (*DizText)
					AddRecord(DizText);
			}

			OrigCodePage=CodePage;
			Modified=false;
			fclose(DizFile);
			return;
		}

		if (DizName!=NULL)
			break;
	}

	Modified=false;
	strDizFileName=L"";
}


bool DizList::AddRecord(const wchar_t *DizText)
{
	DizRecord *NewDizData=DizData;
	if ((DizCount & 15)==0)
		NewDizData=(DizRecord *)xf_realloc(DizData,(DizCount+16+1)*sizeof(*DizData));

	if (!NewDizData)
		return false;

	DizData=NewDizData;
	DizData[DizCount].DizLength=StrLength(DizText);
	DizData[DizCount].DizText=(wchar_t *)xf_malloc((DizData[DizCount].DizLength+1)*sizeof(wchar_t));

	if (!DizData[DizCount].DizText)
		return false;

	wcscpy(DizData[DizCount].DizText,DizText);

	DizData[DizCount].NameStart=0;
	DizData[DizCount].NameLength=0;

	if (*DizText == L'\"')
	{
		DizText++;
		DizData[DizCount].NameStart++;

		while (*DizText && *DizText!=L'\"')
		{
			DizText++;
			DizData[DizCount].NameLength++;
		}
	}
	else
	{
		while (!IsSpaceOrEos(*DizText))
		{
			DizText++;
			DizData[DizCount].NameLength++;
		}
	}

	DizData[DizCount].Deleted=false;

	NeedRebuild=true;
	Modified=true;
	DizCount++;

	return true;
}


const wchar_t* DizList::GetDizTextAddr(const wchar_t *Name, const wchar_t *ShortName, const __int64 FileSize)
{
	const wchar_t *DizText=NULL;
	int TextPos;
	int DizPos=GetDizPosEx(Name,ShortName,&TextPos);

	if (DizPos!=-1)
	{
		DizText=DizData[DizPos].DizText+TextPos;
		while (*DizText && IsSpace(*DizText))
			DizText++;

		if (iswdigit(*DizText))
		{
			wchar_t SizeText[30];
			const wchar_t *DizPtr=DizText;
			bool SkipSize=true;

			swprintf (SizeText, L"%I64u", FileSize);

			for (int I=0; SizeText[I]!=0; DizPtr++)
			{
				if (*DizPtr!=L',' && *DizPtr!=L'.')
				{
					if (SizeText[I++]!=*DizPtr)
					{
						SkipSize=false;
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

	return DizText;
}


int DizList::GetDizPosEx(const wchar_t *Name, const wchar_t *ShortName, int *TextPos)
{
	int DizPos=GetDizPos(Name,TextPos);

	if (DizPos==-1)
		DizPos=GetDizPos(ShortName,TextPos);

	//если файл описаний был в OEM/ANSI то имена файлов могут не совпадать с юникодными
	if (DizPos==-1 && !IsUnicodeOrUtfCodePage(OrigCodePage) && OrigCodePage!=CP_AUTODETECT)
	{
		int len=StrLength(Name);
		char *tmp = (char *)xf_realloc_nomove(AnsiBuf, len+1);
		if (!tmp)
			return -1;

		AnsiBuf = tmp;

		WideCharToMultiByte(OrigCodePage, 0, Name, len, AnsiBuf, len, NULL, NULL);
		AnsiBuf[len]=0;
		string strRecoded(AnsiBuf, OrigCodePage);

		if (strRecoded==Name)
			return -1;

		return GetDizPos(strRecoded,TextPos);

	}

	return DizPos;
}


int DizList::GetDizPos(const wchar_t *Name, int *TextPos)
{
	if (DizData==NULL || !*Name)
		return -1;

	if (NeedRebuild)
		BuildIndex();

	SearchDizData=DizData;
	DizSearchKey Key={Name, StrLength(Name)};
	int *DestIndex=(int *)bsearch(&Key,IndexData,IndexCount,sizeof(*IndexData),SortDizSearch);

	if (DestIndex!=NULL)
	{
		if (TextPos!=NULL)
		{
			*TextPos=DizData[*DestIndex].NameStart+DizData[*DestIndex].NameLength;
			if (DizData[*DestIndex].NameStart && DizData[*DestIndex].DizText[*TextPos]==L'\"')
				(*TextPos)++;
		}

		return *DestIndex;
	}

	return -1;
}


void DizList::BuildIndex()
{
	if (!IndexData || IndexCount!=DizCount)
	{
		if (IndexData)
			xf_free(IndexData);

		if ((IndexData=(int *)xf_malloc(DizCount*sizeof(int)))==NULL)
		{
			Reset();
			return;
		}

		IndexCount=DizCount;
	}

	for (int I=0; I<IndexCount; I++)
		IndexData[I]=I;

	SearchDizData=DizData;
	far_qsort((void *)IndexData,IndexCount,sizeof(*IndexData),SortDizIndex);

	NeedRebuild=false;
}


int _cdecl SortDizIndex(const void *el1,const void *el2)
{
	const wchar_t *Diz1=SearchDizData[*(int *)el1].DizText+SearchDizData[*(int *)el1].NameStart;
	const wchar_t *Diz2=SearchDizData[*(int *)el2].DizText+SearchDizData[*(int *)el2].NameStart;
	int Len1=SearchDizData[*(int *)el1].NameLength;
	int Len2=SearchDizData[*(int *)el2].NameLength;

	int CmpCode = StrCmpNI(Diz1,Diz2,Min(Len1,Len2));

	if (CmpCode==0)
	{
		if (Len1>Len2)
			return 1;

		if (Len1<Len2)
			return -1;

		//for equal names, deleted is bigger
		bool Del1=SearchDizData[*(int *)el1].Deleted;
		bool Del2=SearchDizData[*(int *)el2].Deleted;

		if (Del1 && !Del2)
			return 1;

		if (Del2 && !Del1)
			return -1;
	}

	return CmpCode;
}


int _cdecl SortDizSearch(const void *key,const void *elem)
{
	const wchar_t *SearchName=((DizSearchKey *)key)->Str;
	wchar_t *DizName=SearchDizData[*(int *)elem].DizText+SearchDizData[*(int *)elem].NameStart;
	int DizNameLength=SearchDizData[*(int *)elem].NameLength;
	int NameLength=((DizSearchKey *)key)->Len;

	int CmpCode=StrCmpNI(SearchName,DizName,Min(DizNameLength,NameLength));

	if (CmpCode==0)
	{
		if (NameLength>DizNameLength)
			return 1;

		if (NameLength+1<DizNameLength)
			return -1;

		//filename == filename.
		if (NameLength+1==DizNameLength && DizName[NameLength]!=L'.')
			return -1;

		//for equal names, deleted is bigger so deleted items are never matched
		if (SearchDizData[*(int *)elem].Deleted)
			return -1;
	}

	return CmpCode;
}


bool DizList::DeleteDiz(const wchar_t *Name,const wchar_t *ShortName)
{
	int DizPos=GetDizPosEx(Name,ShortName,NULL);

	if (DizPos==-1)
		return false;

	DizData[DizPos++].Deleted=true;

	while (DizPos<DizCount)
	{
		if (*DizData[DizPos].DizText && !IsSpace(DizData[DizPos].DizText[0]))
			break;

		DizData[DizPos].Deleted=true;
		DizPos++;
	}

	Modified=true;
	NeedRebuild=true;

	return true;
}


bool DizList::Flush(const wchar_t *Path,const wchar_t *DizName)
{
	if (!Modified)
		return true;

	if (DizName!=NULL)
	{
		strDizFileName = DizName;
	}
	else if (strDizFileName.IsEmpty())
	{
		if (DizData==NULL || Path==NULL)
			return false;

		strDizFileName = Path;
		AddEndSlash(strDizFileName);

		string strArgName;
		GetCommaWord(Opt.Diz.strListNames,strArgName);

		strDizFileName += strArgName;
	}

	FILE *DizFile;
	DWORD FileAttr=apiGetFileAttributes(strDizFileName);

	if (FileAttr != INVALID_FILE_ATTRIBUTES)
	{
		if (Opt.Diz.ROUpdate && (FileAttr&FILE_ATTRIBUTE_READONLY))
			apiSetFileAttributes(strDizFileName,FileAttr&(~FILE_ATTRIBUTE_READONLY));

		if ((FileAttr & FILE_ATTRIBUTE_READONLY)==0)
			apiSetFileAttributes(strDizFileName,FILE_ATTRIBUTE_ARCHIVE);
	}

	if ((DizFile=_wfopen(strDizFileName,L"wb"))==NULL)
	{
		if (!Opt.Diz.ROUpdate && (FileAttr&FILE_ATTRIBUTE_READONLY))
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotUpdateDiz),MSG(MCannotUpdateRODiz),MSG(MOk));
		else
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotUpdateDiz),MSG(MOk));

		return false;
	}

	int AddedDizCount=0;
	UINT CodePage = Opt.Diz.SaveInUTF ? CP_UTF8 : (Opt.Diz.AnsiByDefault ? CP_ACP : CP_OEMCP);

	if (CodePage == CP_UTF8)
	{
		DWORD dwSignature = SIGN_UTF8;
		fwrite(&dwSignature, 1, 3, DizFile);
	}

	for (int I=0;I<DizCount;I++)
	{
		if (!DizData[I].Deleted)
		{
			int len = DizData[I].DizLength;
			char *lpDizText = (char *) xf_malloc((len+1)*3); //UTF-8, up to 3 bytes per char support

			if (lpDizText)
			{
				WideCharToMultiByte (CodePage, 0, DizData[I].DizText, len+1, lpDizText, (len+1)*3, NULL, NULL);

				fprintf(DizFile,"%s\r\n", lpDizText);

				xf_free (lpDizText);

				AddedDizCount++;
			}
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
		return false;
	}

	Modified=false;

	return true;
}


bool DizList::AddDizText(const wchar_t *Name,const wchar_t *ShortName,const wchar_t *DizText)
{
	DeleteDiz(Name,ShortName);

	string strQuotedName = Name;
	QuoteSpaceOnly(strQuotedName);

	FormatString FString;
	FString<<fmt::LeftAlign()<<fmt::Width(Opt.Diz.StartPos>1?Opt.Diz.StartPos-2:0)<<strQuotedName<<L" "<<DizText;

	return AddRecord(FString);
}


bool DizList::CopyDiz(const wchar_t *Name, const wchar_t *ShortName, const wchar_t *DestName, const wchar_t *DestShortName, DizList *DestDiz)
{
	int TextPos;
	int DizPos=GetDizPosEx(Name,ShortName,&TextPos);

	if (DizPos==-1)
		return false;

	while (IsSpace(DizData[DizPos].DizText[TextPos]))
		TextPos++;

	DestDiz->AddDizText(DestName,DestShortName,&DizData[DizPos].DizText[TextPos]);

	while (++DizPos<DizCount)
	{
		if (*DizData[DizPos].DizText && !IsSpace(DizData[DizPos].DizText[0]))
			break;

		DestDiz->AddRecord(DizData[DizPos].DizText);
	}

	return true;
}


void DizList::GetDizName(string &strDizName)
{
	strDizName = strDizFileName;
}
