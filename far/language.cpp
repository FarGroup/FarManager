/*
language.cpp

Работа с lng файлами

*/

/* Revision: 1.21 24.05.2002 $ */

/*
Modify:
  24.05.2002 SKV
    - На всякий случай проверяем на NULL то, от чего вызывается delete [] в Close()
  26.03.2002 DJ
    ! не выводим сообщение в GetMsg(), если менеджер уже в дауне
    ! ScanTree::GetNextName() принимает размер буфера для имени файла
  19.03.2002 DJ
    ! не вылетаем при отрицательном индексе в GetMsg()
  27.02.2002 SVS
    ! LIFIND_NOPATTERN -> LIFIND_EXACTMATCH
  25.01.2002 SVS
    ! Обрежим пробелы и слева и справа при чтении из LNG-файла
  24.12.2001 SVS
    + Доп.параметр у OpenLangFile() - StrongLang: "только заданный язык и не более"
  14.12.2001 IS
    ! stricmp -> LocalStricmp
  29.11.2001 DJ
    ! GetLangParam() читает файл только до @Contents и не забывает
      восстановить позицию
  01.10.2001 SVS
    ! Учтем, что может быть, т.н. "DocumentContents"
  01.08.2001 SVS
    - Не допускаем дубликатов!
      Если в каталог с ФАРом положить еще один HLF с одноименным
      языком, то... фигня получается при выборе языка.
  26.07.2001 SVS
    ! VFMenu уничтожен как класс
  27.06.2001 SVS
    ! Позаботимся об информации про LanguageLoaded :-)
  03.06.2001 SVS
    ! Изменения в связи с переделкой UserData в VMenu
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
    ! Константы MENU_ - в морг
  17.05.2001 OT
    ! выравнивание сообщений на границу 4 байт.
  06.05.2001 DJ
    ! перетрях #include
  11.02.2001 SVS
    ! Несколько уточнений кода в связи с изменениями в структуре MenuItem
  19.01.2001 SVS
    + Проведем проверку на количество строк в LNG-файлах
  03.09.2000 IS
    ! Нормальное сообщение об отсутствии строки в языковом файле
      (раньше имя файла обрезалось справа и приходилось иногда гадать - в
      каком же файле ошибка)
  01.09.2000 SVS
    + Новый метод, для получения параметров для .Options
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
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

  FILE *LangFile=OpenLangFile(Path,LangFileMask,Opt.Language,MessageFile);
  if (LangFile==NULL)
    return(FALSE);

  char ReadStr[1024];

  while (fgets(ReadStr,sizeof(ReadStr),LangFile)!=NULL)
  {
    char DestStr[1024];
    RemoveExternalSpaces(ReadStr);
    if (*ReadStr!='\"')
      continue;
    int SrcLength=strlen(ReadStr);
    if (ReadStr[SrcLength-1]=='\"')
      ReadStr[SrcLength-1]=0;
    ConvertString(ReadStr+1,DestStr);
    int DestLength=pack(strlen(DestStr)+1);
    if ((MsgList=(char *)realloc(MsgList,MsgSize+DestLength))==NULL)
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
  if(CountNeed != -1 && CountNeed != MsgCount-1)
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
  for (int I=0;I<MsgCount;I++)
  {
    MsgAddr[I]=CurAddr;
    CurAddr+=pack(strlen(CurAddr)+1);
  }
  fclose(LangFile);
  SetLastError(LastError);
  LanguageLoaded=TRUE;
  return(TRUE);
}


void Language::Close()
{
  /* $ 13.07.2000 SVS
     ни кто не вызывал запрос памяти через new :-)
  */
  if(MsgList)free(MsgList);
  MsgList=NULL;
  /* ну а здесь раз уж вызвали new[], то в придачу и delete[] надо... */
  if(MsgAddr)delete[] MsgAddr;
  /* SVS $ */
  MsgAddr=NULL;
  MsgCount=0;
  MsgSize=0;
  LanguageLoaded=FALSE;
}


void Language::ConvertString(char *Src,char *Dest)
{
  while (*Src)
    switch(*Src)
    {
      case '\\':
        switch(Src[1])
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


char* Language::GetMsg(int MsgId)
{
  /* $ 19.03.2002 DJ
     при отрицательном индексе - также покажем сообщение об ошибке
     (все лучше, чем трапаться)
  */
  if (MsgId>=MsgCount || MsgId < 0)  /* DJ $ */
  {
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
      char Msg1[100],Msg2[100],Tmp[NM];
      strcpy(Tmp,MessageFile);
      TruncPathStr(Tmp,41);
      sprintf(Msg1,"Incorrect or damaged %s",Tmp);
      /* IS $ */
      sprintf(Msg2,"Message %d not found",MsgId);
      if (Message(MSG_WARNING,2,"Error",Msg1,Msg2,"Ok","Quit")==1)
        exit(0);
    }
    /* DJ $ */
    return("");
  }
  return(MsgAddr[MsgId]);
}

FILE* Language::OpenLangFile(char *Path,char *Mask,char *Language,char *FileName,BOOL StrongLang)
{
  *FileName=0;

  FILE *LangFile=NULL;
  char FullName[NM], EngFileName[NM];
  WIN32_FIND_DATA FindData;

  *EngFileName=0;

  ScanTree ScTree(FALSE,FALSE);
  ScTree.SetFindPath(Path,Mask);
  while (ScTree.GetNextName(&FindData,FullName, sizeof (FullName)-1))
  {
    strcpy(FileName,FullName);
    if (Language==NULL)
      break;
    if ((LangFile=fopen(FileName,"rb"))==NULL)
      *FileName=0;
    else
    {
      char LangName[100];
      if (GetLangParam(LangFile,"Language",LangName,NULL) && LocalStricmp(LangName,Language)==0)
        break;
      fclose(LangFile);
      LangFile=NULL;
      if(StrongLang)
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
      LangFile=fopen(FileName,"rb");
  }

  return(LangFile);
}


int Language::GetLangParam(FILE *SrcFile,char *ParamName,char *Param1,char *Param2)
{
  char ReadStr[1024],FullParamName[64];
  sprintf(FullParamName,".%s=",ParamName);
  int Length=strlen(FullParamName);
  /* $ 29.11.2001 DJ
     не поганим позицию в файле; дальше @Contents не читаем
  */
  BOOL Found = FALSE;
  long OldPos = ftell (SrcFile);
  fseek(SrcFile,0,SEEK_SET);
  while (fgets(ReadStr,sizeof(ReadStr),SrcFile)!=NULL)
  {
    if (strnicmp(ReadStr,FullParamName,Length)==0)
    {
      strcpy(Param1,ReadStr+Length);
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
    else if (!strnicmp (ReadStr, "@Contents", 9))
      break;
  }
  fseek (SrcFile,OldPos,SEEK_SET);
  /* DJ $ */
  return(Found);
}


int Language::Select(int HelpLanguage,VMenu **MenuPtr)
{
  char *Title,*Mask,*Dest;
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
  while (ScTree.GetNextName(&FindData,FullName, sizeof (FullName)-1))
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
         if(LangMenu->FindItem(0,LangMenuItem.Name,LIFIND_EXACTMATCH) == -1)
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
  strcpy(FullParamName,".Options");
  int Length=strlen(FullParamName);
  long CurFilePos=ftell(SrcFile);
  fseek(SrcFile,0,SEEK_SET);
  while (fgets(ReadStr,sizeof(ReadStr),SrcFile)!=NULL)
    if (!strnicmp(ReadStr,FullParamName,Length))
    {
      strcpy(FullParamName,RemoveExternalSpaces(ReadStr+Length));
      if((Ptr=strchr(FullParamName,'=')) == NULL)
        continue;
      *Ptr++=0;
      if (!LocalStricmp(RemoveExternalSpaces(FullParamName),KeyName))
      {
        strcpy(Value,RemoveExternalSpaces(Ptr));
        return(TRUE);
      }
    }
  fseek(SrcFile,CurFilePos,SEEK_SET);
  return(FALSE);
}
/* SVS $ */
