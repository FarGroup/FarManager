/*
language.cpp

Работа с lng файлами

*/

/* Revision: 1.34 06.06.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "language.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "scantree.hpp"
#include "vmenu.hpp"
#include "manager.hpp"

extern wchar_t *ReadString (FILE *file, wchar_t *lpwszDest, int nDestLength, int nType);

#define LangFileMask L"*.lng"

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

  MsgListW = NULL;
  MsgAddrW = NULL;

  MsgCount=0;
  MsgSize=0;
}


int Language::Init(const wchar_t *Path,int CountNeed)
{
  if (MsgList!=NULL)
    return(TRUE);

  int LastError=GetLastError();

  int nCodePage = CP_OEMCP;
  FILE *LangFile=OpenLangFile(Path,LangFileMask,Opt.strLanguage,strMessageFile, nCodePage);

  if (LangFile==NULL)
    return(FALSE);

  wchar_t ReadStr[1024];
  memset (&ReadStr, 0, sizeof (ReadStr));

  while ( ReadString (LangFile, ReadStr, sizeof (ReadStr)/sizeof (wchar_t), nCodePage) !=NULL )
  {
    string strDestStr;
    RemoveExternalSpacesW(ReadStr);
    if ( *ReadStr != L'\"')
      continue;
    int SrcLength=wcslen (ReadStr);

    if (ReadStr[SrcLength-1]==L'\"')
      ReadStr[SrcLength-1]=0;

    ConvertString(ReadStr+1,strDestStr);

    int DestLength=pack(strDestStr.GetLength()+1);

    if ((MsgList=(char *)xf_realloc(MsgList,MsgSize+DestLength))==NULL)
    {
      fclose(LangFile);
      return(FALSE);
    }

    if ( (MsgListW = (wchar_t*)xf_realloc(MsgListW, (MsgSize+DestLength)*sizeof (wchar_t)))==NULL )
    {
      free (MsgList);
      MsgList = NULL;
      fclose(LangFile);
      return(FALSE);
    }

    *(int*)&MsgList[MsgSize+DestLength-_PACK]=0;
    *(int*)&MsgListW[MsgSize+DestLength-_PACK] = 0;

    WideCharToMultiByte(CP_OEMCP, 0, strDestStr, -1, MsgList+MsgSize, DestLength, NULL, NULL);
    wcscpy(MsgListW+MsgSize, strDestStr);

    MsgSize+=DestLength;
    MsgCount++;
  }
  /* $ 19.01.2001 SVS
     Проведем проверку на количество строк в LNG-файлах  */
  if(CountNeed != -1 && CountNeed != MsgCount-1)
  {
    fclose(LangFile);
    return(FALSE);
  }
  /* SVS $ */
  char *CurAddr=MsgList;
  wchar_t *CurAddrW = MsgListW;

  MsgAddr=new LPSTR[MsgCount];
  MsgAddrW = new wchar_t*[MsgCount];

  if (MsgAddr==NULL)
  {
    fclose(LangFile);
    return(FALSE);
  }

  if ( MsgAddrW == NULL )
  {
    free (MsgAddr);
    fclose(LangFile);
    return(FALSE);
  }

  for (int I=0;I<MsgCount;I++)
  {
    MsgAddr[I]=CurAddr;
    MsgAddrW[I]=CurAddrW;

    CurAddrW+=pack(wcslen(CurAddrW)+1);
    CurAddr+=pack(strlen(CurAddr)+1);
  }

  fclose(LangFile);
  SetLastError(LastError);
  if(this == &Lang)
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
  if(MsgList)xf_free(MsgList);
  MsgList=NULL;
  if(MsgAddr)delete[] MsgAddr;
  MsgAddr=NULL;
  MsgCount=0;
  MsgSize=0;
}

void Language::Close()
{
  if(this == &Lang)
  {
    if(OldLang.MsgCount)
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


void Language::ConvertString(const wchar_t *Src,string &strDest)
{
  wchar_t *Dest = strDest.GetBuffer (wcslen (Src)*2);

  while (*Src)
    switch(*Src)
    {
      case L'\\':
        switch(Src[1])
        {
          case L'\\':
            *(Dest++)=L'\\';
            Src+=2;
            break;
          case L'\"':
            *(Dest++)=L'\"';
            Src+=2;
            break;
          case L'n':
            *(Dest++)=L'\n';
            Src+=2;
            break;
          case L'r':
            *(Dest++)=L'\r';
            Src+=2;
            break;
          case L'b':
            *(Dest++)=L'\b';
            Src+=2;
            break;
          case L't':
            *(Dest++)=L'\t';
            Src+=2;
            break;
          default:
            *(Dest++)=L'\\';
            Src++;
            break;
        }
        break;
      case L'"':
        *(Dest++)=L'"';
        Src+=(Src[1]==L'"') ? 2:1;
        break;
      default:
        *(Dest++)=*(Src++);
        break;
    }
  *Dest=0;

  strDest.ReleaseBuffer();
}

BOOL Language::CheckMsgId(int MsgId)
{
  /* $ 19.03.2002 DJ
     при отрицательном индексе - также покажем сообщение об ошибке
     (все лучше, чем трапаться)
  */
  if (MsgId>=MsgCount || MsgId < 0)  /* DJ $ */
  {
    if(this == &Lang && !LanguageLoaded && this != &OldLang && OldLang.CheckMsgId(MsgId))
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
      string strMsg1, strMsg2, strTmp;
      strTmp = strMessageFile;
      TruncPathStrW(strTmp,41);
      strMsg1.Format(L"Incorrect or damaged %s", (const wchar_t*)strTmp);
      /* IS $ */
      strMsg2.Format(L"Message %d not found",MsgId);
      if (MessageW(MSG_WARNING,2,L"Error",strMsg1,strMsg2,L"Ok",L"Quit")==1)
        exit(0);
    }
    /* DJ $ */
    return FALSE;
  }
  return TRUE;
}

char* Language::GetMsg(int MsgId)
{
  if(!CheckMsgId(MsgId))
    return "";
  if(this == &Lang && this != &OldLang && !LanguageLoaded && OldLang.MsgCount > 0)
    return(OldLang.MsgAddr[MsgId]);
  return(MsgAddr[MsgId]);
}

wchar_t* Language::GetMsgW (int nID)
{
  if( !CheckMsgId (nID) )
    return L"";

  if( this == &Lang && this != &OldLang && !LanguageLoaded && OldLang.MsgCount > 0)
    return(OldLang.MsgAddrW[nID]);

  return(MsgAddrW[nID]);
}


FILE* Language::OpenLangFile(const wchar_t *Path,const wchar_t *Mask,const wchar_t *Language, string &strFileName, int &nCodePage, BOOL StrongLang)
{
  strFileName=L"";

  FILE *LangFile=NULL;
  string strFullName, strEngFileName;
  FAR_FIND_DATA_EX FindData;

  ScanTree ScTree(FALSE,FALSE);
  ScTree.SetFindPathW(Path,Mask);
  while (ScTree.GetNextNameW(&FindData, strFullName))
  {
    strFileName = strFullName;
    if (Language==NULL)
      break;
    if ((LangFile=_wfopen(strFileName,L"rb"))==NULL)
      strFileName=L"";
    else
    {
      nCodePage = GetFileFormat (LangFile);

      string strLangName;
      string strNULL;

      if (GetLangParam(LangFile,L"Language",&strLangName,NULL, nCodePage) && LocalStricmpW(strLangName,Language)==0)
        break;
      fclose(LangFile);
      LangFile=NULL;
      if(StrongLang)
      {
        strFileName=strEngFileName=L"";
        break;
      }
      if (LocalStricmpW(strLangName,L"English")==0)
        strEngFileName = strFileName;
    }
  }

  if (LangFile==NULL)
  {
    if ( !strEngFileName.IsEmpty() )
      strFileName = strEngFileName;
    if ( !strFileName.IsEmpty() )
      LangFile=_wfopen(strFileName,L"rb");
  }

  return(LangFile);
}


int Language::GetLangParam(FILE *SrcFile,const wchar_t *ParamName,string *strParam1, string *strParam2, int nCodePage)
{
  wchar_t ReadStr[1024];

  string strFullParamName = L".";

  strFullParamName += ParamName;

  int Length=strFullParamName.GetLength();
  /* $ 29.11.2001 DJ
     не поганим позицию в файле; дальше @Contents не читаем
  */
  BOOL Found = FALSE;
  long OldPos = ftell (SrcFile);

  while ( ReadString (SrcFile, ReadStr, 1024, nCodePage)!=NULL)
  {
    if (LocalStrnicmpW(ReadStr,strFullParamName,Length)==0)
    {
      wchar_t *Ptr=wcschr(ReadStr,L'=');
      if(Ptr)
      {
          *strParam1 = Ptr+1;
          wchar_t *EndPtr=strParam1->GetBuffer ();

          EndPtr = wcschr(EndPtr,L',');
        if ( strParam2 )
          *strParam2=L"";
        if (EndPtr!=NULL)
        {
          if (strParam2)
          {
            *strParam2 = EndPtr+1;
            RemoveTrailingSpacesW(*strParam2);
          }
          *EndPtr=0;
        }

        strParam1->ReleaseBuffer();

        RemoveTrailingSpacesW(*strParam1);
        Found = TRUE;
        break;
      }
    }
    else if (!LocalStrnicmpW (ReadStr, L"@Contents", 9))
      break;
  }
  fseek (SrcFile,OldPos,SEEK_SET);
  /* DJ $ */
  return(Found);
}


int Language::Select(int HelpLanguage,VMenu **MenuPtr)
{
  const wchar_t *Title,*Mask;
  string *strDest;
  if (HelpLanguage)
  {
    Title=UMSG(MHelpLangTitle);
    Mask=HelpFileMask;
    strDest=&Opt.strHelpLanguage;
  }
  else
  {
    Title=UMSG(MLangTitle);
    Mask=LangFileMask;
    strDest=&Opt.strLanguage;
  }

  MenuItemEx LangMenuItem;

  LangMenuItem.Clear ();
  VMenu *LangMenu=new VMenu(Title,NULL,0,TRUE, ScrY-4);
  *MenuPtr=LangMenu;
  LangMenu->SetFlags(VMENU_WRAPMODE);
  LangMenu->SetPosition(ScrX/2-8+5*HelpLanguage,ScrY/2-4+2*HelpLanguage,0,0);

  string strFullName;
  FAR_FIND_DATA_EX FindData;
  ScanTree ScTree(FALSE,FALSE);
  ScTree.SetFindPathW(g_strFarPath, Mask);
  while (ScTree.GetNextNameW(&FindData,strFullName))
  {
    FILE *LangFile=_wfopen(strFullName,L"rb");
    if (LangFile==NULL)
      continue;

    int nCodePage = GetFileFormat(LangFile, NULL);

    string strLangName, strLangDescr;
    if (GetLangParam(LangFile,L"Language",&strLangName,&strLangDescr,nCodePage))
    {
       string strEntryName;
       if (!HelpLanguage || (!GetLangParam(LangFile,L"PluginContents",&strEntryName,NULL,nCodePage) &&
           !GetLangParam(LangFile,L"DocumentContents",&strEntryName,NULL,nCodePage)))
       {

         LangMenuItem.strName.Format(L"%.40s", !strLangDescr.IsEmpty() ? (const wchar_t*)strLangDescr:(const wchar_t*)strLangName);
         /* $ 01.08.2001 SVS
            Не допускаем дубликатов!
            Если в каталог с ФАРом положить еще один HLF с одноименным
            языком, то... фигня получается при выборе языка.
         */
         if(LangMenu->FindItem(0,LangMenuItem.strName,LIFIND_EXACTMATCH) == -1)
         {
           LangMenuItem.SetSelect(LocalStricmpW(*strDest,strLangName)==0);
           LangMenu->SetUserData((void*)(const wchar_t*)strLangName,0,LangMenu->AddItemW(&LangMenuItem));
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

  wchar_t *lpwszDest = strDest->GetBuffer(LangMenu->GetUserDataSize()/sizeof(wchar_t)+1);

  LangMenu->GetUserData(lpwszDest, LangMenu->GetUserDataSize());

  strDest->ReleaseBuffer();

  return(LangMenu->GetUserDataSize());
}

/* $ 01.09.2000 SVS
  + Новый метод, для получения параметров для .Options
   .Options <KeyName>=<Value>
*/
int Language::GetOptionsParam(FILE *SrcFile,const wchar_t *KeyName,string &strValue, int nCodePage)
{
  wchar_t ReadStr[1024];

  string strFullParamName;

  wchar_t *Ptr;

  int Length=wcslen(L".Options");

  long CurFilePos=ftell(SrcFile);

  while ( ReadString (SrcFile, ReadStr, 1024, nCodePage) !=NULL)
  {
    if (!LocalStrnicmpW(ReadStr,L".Options",Length))
    {
      strFullParamName = RemoveExternalSpacesW(ReadStr+Length);

      Ptr = strFullParamName.GetBuffer ();


      if((Ptr=wcsrchr(Ptr,L'=')) == NULL)
      {
        strFullParamName.ReleaseBuffer ();
        continue;
      }

      *Ptr++=0;

      strValue = RemoveExternalSpacesW(Ptr);

      strFullParamName.ReleaseBuffer ();

      RemoveExternalSpacesW (strFullParamName);

      if (!LocalStricmpW(strFullParamName,KeyName))
        return(TRUE);
    }
  }
  fseek(SrcFile,CurFilePos,SEEK_SET);
  return(FALSE);
}
/* SVS $ */
