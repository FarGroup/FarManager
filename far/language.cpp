/*
language.cpp

Работа с lng файлами

*/

#include "headers.hpp"
#pragma hdrstop

#include "language.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "scantree.hpp"
#include "vmenu.hpp"
#include "manager.hpp"

#define LangFileMask "*.lng"

#ifndef pack
#define _PACK_BITS 2
#define _PACK (1 << _PACK_BITS)
#define pck(x,N)            ( ((x) + ((1<<(N))-1) )  & ~((1<<(N))-1) )
#define pack(x)             pck(x,_PACK_BITS)
#endif

Language Lang;
static Language OldLang;

Language::Language()
{
	MsgList=NULL;
	MsgAddr=NULL;
	MsgCount=0;
	MsgSize=0;
}


int Language::Init(char *Path,int CountNeed)
{
	if (MsgList!=NULL)
		return(TRUE);

	int LastError=GetLastError();
	char LangName[LANGUAGENAME_SIZE];
	strcpy(LangName,Opt.Language);
	FILE *LangFile=OpenLangFile(Path,LangFileMask,Opt.Language,MessageFile,FALSE,LangName);

	if (LangFile==NULL)
		return(FALSE);

	if (this == &Lang && *LangName && LocalStricmp(Opt.Language,LangName))
		strcpy(Opt.Language,LangName);

	char ReadStr[1024];

	while (fgets(ReadStr,sizeof(ReadStr),LangFile)!=NULL)
	{
		char DestStr[1024];
		RemoveExternalSpaces(ReadStr);

		if (*ReadStr!='\"')
			continue;

		int SrcLength=(int)strlen(ReadStr);

		if (ReadStr[SrcLength-1]=='\"')
			ReadStr[SrcLength-1]=0;

		ConvertString(ReadStr+1,DestStr);
		int DestLength=pack((int)strlen(DestStr)+1);

		if ((MsgList=(char *)xf_realloc(MsgList,MsgSize+DestLength))==NULL)
		{
			fclose(LangFile);
			return(FALSE);
		}

		*(int*)&MsgList[MsgSize+DestLength-_PACK]=0;
		strcpy(MsgList+MsgSize,DestStr);
		MsgSize+=DestLength;
		MsgCount++;
	}

	/* $ 19.01.2001 SVS
	   Проведем проверку на количество строк в LNG-файлах  */
	if (CountNeed != -1 && CountNeed != MsgCount-1)
	{
		fclose(LangFile);
		return(FALSE);
	}

	/* SVS $ */
	char *CurAddr=MsgList;
	MsgAddr=new LPSTR[MsgCount];

	if (MsgAddr==NULL)
	{
		fclose(LangFile);
		return(FALSE);
	}

	for (int I=0; I<MsgCount; I++)
	{
		MsgAddr[I]=CurAddr;
		CurAddr+=pack(strlen(CurAddr)+1);
	}

	fclose(LangFile);
	SetLastError(LastError);

	if (this == &Lang)
		OldLang.Free();

	LanguageLoaded=TRUE;
	return(TRUE);
}


Language::~Language()
{
	Free();
}

void Language::Free()
{
	if (MsgList)xf_free(MsgList);

	MsgList=NULL;

	if (MsgAddr)delete[] MsgAddr;

	MsgAddr=NULL;
	MsgCount=0;
	MsgSize=0;
}

void Language::Close()
{
	if (this == &Lang)
	{
		if (OldLang.MsgCount)
			OldLang.Free();

		OldLang.MsgList=MsgList;
		OldLang.MsgAddr=MsgAddr;
		OldLang.MsgCount=MsgCount;
		OldLang.MsgSize=MsgSize;
	}

	MsgList=NULL;
	MsgAddr=NULL;
	MsgCount=0;
	MsgSize=0;
	LanguageLoaded=FALSE;
}


void Language::ConvertString(char *Src,char *Dest)
{
	while (*Src)
		switch (*Src)
		{
			case '\\':

				switch (Src[1])
				{
					case '\\':
						*(Dest++)='\\';
						Src+=2;
						break;
					case '\"':
						*(Dest++)='\"';
						Src+=2;
						break;
					case 'n':
						*(Dest++)='\n';
						Src+=2;
						break;
					case 'r':
						*(Dest++)='\r';
						Src+=2;
						break;
					case 'b':
						*(Dest++)='\b';
						Src+=2;
						break;
					case 't':
						*(Dest++)='\t';
						Src+=2;
						break;
					default:
						*(Dest++)='\\';
						Src++;
						break;
				}

				break;
			case '"':
				*(Dest++)='"';
				Src+=(Src[1]=='"') ? 2:1;
				break;
			default:
				*(Dest++)=*(Src++);
				break;
		}

	*Dest=0;
}

BOOL Language::CheckMsgId(int MsgId)
{
	/* $ 19.03.2002 DJ
	   при отрицательном индексе - также покажем сообщение об ошибке
	   (все лучше, чем трапаться)
	*/
	if (MsgId>=MsgCount || MsgId < 0)  /* DJ $ */
	{
		if (this == &Lang && !LanguageLoaded && this != &OldLang && OldLang.CheckMsgId(MsgId))
			return TRUE;

		/* $ 26.03.2002 DJ
		   если менеджер уже в дауне - сообщение не выводим
		*/
		if (!FrameManager->ManagerIsDown())
		{
			/* $ 03.09.2000 IS
			   ! Нормальное сообщение об отсутствии строки в языковом файле
			     (раньше имя файла обрезалось справа и приходилось иногда гадать - в
			     каком же файле ошибка)
			*/
			char Msg1[100],Msg2[100];
			sprintf(Msg1,"Incorrect or damaged %s",MessageFile);
			/* IS $ */
			sprintf(Msg2,"Message %d not found",MsgId);

			if (Message(MSG_WARNING,2,"Error",Msg1,Msg2,"Ok","Quit")==1)
				exit(0);
		}

		/* DJ $ */
		return FALSE;
	}

	return TRUE;
}

char* Language::GetMsg(int MsgId)
{
	if (!CheckMsgId(MsgId))
		return "";

	if (this == &Lang && this != &OldLang && !LanguageLoaded && OldLang.MsgCount > 0)
		return(OldLang.MsgAddr[MsgId]);

	return(MsgAddr[MsgId]);
}


FILE* Language::OpenLangFile(const char *Path,const char *Mask,const char *Language,char *FileName,BOOL StrongLang,char *pLangName)
{
	*FileName=0;
	FILE *LangFile=NULL;
	char FullName[NM], EngFileName[NM];
	WIN32_FIND_DATA FindData;
	char LangName[LANGUAGENAME_SIZE];
	*EngFileName=0;
	ScanTree ScTree(FALSE,FALSE);
	ScTree.SetFindPath(Path,Mask);

	while (ScTree.GetNextName(&FindData,FullName, sizeof(FullName)-1))
	{
		strcpy(FileName,FullName);

		if (Language==NULL)
			break;

		if ((LangFile=fopen(FileName,"rb"))==NULL)
			*FileName=0;
		else
		{
			if (GetLangParam(LangFile,"Language",LangName,NULL) && LocalStricmp(LangName,Language)==0)
				break;

			fclose(LangFile);
			LangFile=NULL;

			if (StrongLang)
			{
				*FileName=*EngFileName=0;
				break;
			}

			if (LocalStricmp(LangName,"English")==0)
				strcpy(EngFileName,FileName);
		}
	}

	if (LangFile==NULL)
	{
		if (*EngFileName)
			strcpy(FileName,EngFileName);

		if (*FileName)
		{
			LangFile=fopen(FileName,"rb");

			if (pLangName)
				strcpy(pLangName,LangName);
		}
	}

	return(LangFile);
}


int Language::GetLangParam(FILE *SrcFile,char *ParamName,char *Param1,char *Param2)
{
	char ReadStr[1024],FullParamName[64];
	sprintf(FullParamName,".%s",ParamName);
	int Length=(int)strlen(FullParamName);
	/* $ 29.11.2001 DJ
	   не поганим позицию в файле; дальше @Contents не читаем
	*/
	BOOL Found = FALSE;
	long OldPos = ftell(SrcFile);
	fseek(SrcFile,0,SEEK_SET);

	while (fgets(ReadStr,sizeof(ReadStr),SrcFile)!=NULL)
	{
		if (strnicmp(ReadStr,FullParamName,Length)==0)
		{
			char *Ptr=strchr(ReadStr,'=');

			if (Ptr)
			{
				RemoveExternalSpaces(strcpy(Param1,Ptr+1));
				char *EndPtr=strchr(Param1,',');

				if (Param2)
					*Param2=0;

				if (EndPtr!=NULL)
				{
					if (Param2)
					{
						strcpy(Param2,EndPtr+1);
						RemoveTrailingSpaces(Param2);
					}

					*EndPtr=0;
				}

				RemoveTrailingSpaces(Param1);
				Found = TRUE;
				break;
			}
		}
		else if (!strnicmp(ReadStr, "@Contents", 9))
			break;
	}

	fseek(SrcFile,OldPos,SEEK_SET);
	/* DJ $ */
	return(Found);
}


int Language::Select(int HelpLanguage,VMenu **MenuPtr)
{
	const char *Title,*Mask;
	char *Dest;

	if (HelpLanguage)
	{
		Title=MSG(MHelpLangTitle);
		Mask=HelpFileMask;
		Dest=Opt.HelpLanguage;
	}
	else
	{
		Title=MSG(MLangTitle);
		Mask=LangFileMask;
		Dest=Opt.Language;
	}

	struct MenuItem LangMenuItem;

	memset(&LangMenuItem,0,sizeof(LangMenuItem));

	VMenu *LangMenu=new VMenu(Title,NULL,0,ScrY-4);

	*MenuPtr=LangMenu;

	LangMenu->SetFlags(VMENU_WRAPMODE);

	LangMenu->SetPosition(ScrX/2-8+5*HelpLanguage,ScrY/2-4+2*HelpLanguage,0,0);

	char FullName[NM];

	WIN32_FIND_DATA FindData;

	ScanTree ScTree(FALSE,FALSE);

	ScTree.SetFindPath(FarPath,Mask);

	while (ScTree.GetNextName(&FindData,FullName, sizeof(FullName)-1))
	{
		FILE *LangFile=fopen(FullName,"rb");

		if (LangFile==NULL)
			continue;

		char LangName[200],LangDescr[200];

		if (GetLangParam(LangFile,"Language",LangName,LangDescr))
		{
			char EntryName[512];

			if (!GetLangParam(LangFile,"PluginContents",EntryName,NULL) &&
			        !GetLangParam(LangFile,"DocumentContents",EntryName,NULL))
			{
				sprintf(LangMenuItem.Name,"%.40s",*LangDescr ? LangDescr:LangName);

				/* $ 01.08.2001 SVS
				   Не допускаем дубликатов!
				   Если в каталог с ФАРом положить еще один HLF с одноименным
				   языком, то... фигня получается при выборе языка.
				*/
				if (LangMenu->FindItem(0,LangMenuItem.Name,LIFIND_EXACTMATCH) == -1)
				{
					LangMenuItem.SetSelect(LocalStricmp(Dest,LangName)==0);
					LangMenu->SetUserData(LangName,0,LangMenu->AddItem(&LangMenuItem));
				}

				/* SVS $ */
			}
		}

		fclose(LangFile);
	}

	LangMenu->AssignHighlights(FALSE);
	LangMenu->Process();

	if (LangMenu->Modal::GetExitCode()<0)
		return(FALSE);

	LangMenu->GetUserData(Dest,sizeof(Opt.Language));
	return(LangMenu->GetUserDataSize());
}

/* $ 01.09.2000 SVS
  + Новый метод, для получения параметров для .Options
   .Options <KeyName>=<Value>
*/
int Language::GetOptionsParam(FILE *SrcFile,char *KeyName,char *Value)
{
	char ReadStr[1024],FullParamName[64], *Ptr;
	memset(FullParamName, 0, 64);
	int Length=(int)strlen(".Options");
	long CurFilePos=ftell(SrcFile);
	fseek(SrcFile,0,SEEK_SET);

	while (fgets(ReadStr,sizeof(ReadStr),SrcFile)!=NULL)
	{
		if (!strnicmp(ReadStr,".Options",Length))
		{
			strcpy(FullParamName,RemoveExternalSpaces(ReadStr+Length));

			if ((Ptr=strchr(FullParamName,'=')) == NULL)
				continue;

			*Ptr++=0;

			if (!LocalStricmp(RemoveExternalSpaces(FullParamName),KeyName))
			{
				strcpy(Value,RemoveExternalSpaces(Ptr));
				return(TRUE);
			}
		}
	}

	fseek(SrcFile,CurFilePos,SEEK_SET);
	return(FALSE);
}
/* SVS $ */
