/*
fnparce.cpp

Парсер файловых ассоциаций

*/

/* Revision: 1.01 19.06.2001 $ */

/*
Modify:
  19.06.2001 SVS
    ! Исключим лишний прогон функции SubstFileName() - проверим, а есть ли
      символы '!' в строке.
  18.06.2001 SVS
    + Created - вынесен в отдельный модуль... для удобства :-)
*/

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "panel.hpp"
#include "ctrlobj.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "dialog.hpp"

static void ReplaceVariables(char *Str);

// Str=if exist !#!\!^!.! far:edit < diff -c -p "!#!\!^!.!" !\!.!
/*
  SubstFileName()
  Преобразование метасимволов ассоциации файлов в реальные значения

  Входные ListName и ShortListName обЯзаны иметь размер NM*2 !!!
*/
int SubstFileName(char *Str,            // результирующая строка
                  char *Name,           // Длинное имя
                  char *ShortName,      // Короткое имя
                  char *ListName,       // Длинное имя файла-списка
                  char *ShortListName,  // Короткое имя файла-списка
                  int   IgnoreInput,    // TRUE - не исполнять "!?<title>?<init>!"
                  char *CmdLineDir)     // Каталог исполнения
{
  /* $ 19.06.2001 SVS
    ВНИМАНИЕ! Для альтернативных метасимволов, не основанных на "!",
    нужно будет либо убрать эту проверку либо изменить условие (последнее
    предпочтительнее!)
  */
  if(!strchr(Str,'!'))
    return FALSE;
  /* SVS $ */
  char TmpStr[8192],*CurStr,*ChPtr;
  char AnotherName[NM],AnotherShortName[NM];
  char NameOnly[NM],ShortNameOnly[NM];
  char AnotherNameOnly[NM],AnotherShortNameOnly[NM];
  char QuotedName[NM+2],QuotedShortName[NM+2];
  char AnotherQuotedName[NM+2],AnotherQuotedShortName[NM+2];
  char CmdDir[NM];
  int PreserveLFN=FALSE;

_SVS(SysLog("'%s'",Str));
_SVS(SysLog(1));

  // Если имя текущего каталога не задано...
  if (CmdLineDir!=NULL)
    strcpy(CmdDir,CmdLineDir);
  else // ...спросим у ком.строки
    CtrlObject->CmdLine->GetCurDir(CmdDir);

  *TmpStr=0; // пока пусто.

  // подготовим имена файлов-списков (если нужно)
  if (ListName!=NULL)
  {
    *ListName=0;
    ListName[NM]=0;
  }
  if (ShortListName!=NULL)
  {
    *ShortListName=0;
    ShortListName[NM]=0;
  }

  // Предварительно получим некоторые "константы" :-)
  strcpy(NameOnly,Name);
  if ((ChPtr=strrchr(NameOnly,'.'))!=NULL)
    *ChPtr=0;
  strcpy(ShortNameOnly,ShortName);
  if ((ChPtr=strrchr(ShortNameOnly,'.'))!=NULL)
    *ChPtr=0;
  QuoteSpace(NameOnly);
  QuoteSpace(ShortNameOnly);

  strcpy(QuotedName,Name);
  strcpy(QuotedShortName,ShortName);
  QuoteSpace(QuotedName);
  QuoteSpace(QuotedShortName);

  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel);
  AnotherPanel->GetCurName(AnotherName,AnotherShortName);
  strcpy(AnotherNameOnly,AnotherName);
  if ((ChPtr=strrchr(AnotherNameOnly,'.'))!=NULL)
    *ChPtr=0;
  strcpy(AnotherShortNameOnly,AnotherShortName);
  if ((ChPtr=strrchr(AnotherShortNameOnly,'.'))!=NULL)
    *ChPtr=0;
  strcpy(AnotherQuotedName,AnotherName);
  strcpy(AnotherQuotedShortName,AnotherShortName);
  QuoteSpace(AnotherQuotedName);
  QuoteSpace(AnotherQuotedShortName);

  int SkipQuotes=FALSE;
  char *DirBegin=NULL; // начало имени каталога (!?)
  int PassivePanel=FALSE; // первоначально речь идет про активную панель!
_SVS(int Pass=1);

  CurStr=Str;
  while (*CurStr)
  {
_SVS(SysLog("***** Pass=%d",Pass));
    if(isspace(*CurStr) && DirBegin && DirBegin != Str && *(DirBegin-1) != '"')
    {
      Unquote(DirBegin);
      QuoteSpaceOnly(DirBegin);
      DirBegin=NULL;
_SVS(SysLog("Unquote? '%c' DirBegin=[%s] %d",*CurStr, DirBegin, SkipQuotes));
    }

    // рассмотрим переключатели активности/пассивности панели.
    if (strncmp(CurStr,"!#",2)==0)
    {
      CurStr+=2;
      PassivePanel=TRUE;
_SVS(SysLog("PassivePanel=TRUE '%s'",CurStr));
    }
    if (strncmp(CurStr,"!^",2)==0)
    {
      CurStr+=2;
      PassivePanel=FALSE;
_SVS(SysLog("PassivePanel=FALSE '%s'",CurStr));
    }

    if (*CurStr=='\"')
      SkipQuotes=(CurStr==Str);

    if (!SkipQuotes)
    {
      // !! символ '!'
      if (strncmp(CurStr,"!!",2)==0 && CurStr[2] != '?')
      {
        strcat(TmpStr,"!");
        CurStr+=2;
_SVS(SysLog("!! TmpStr=[%s]",TmpStr));
        continue;
      }

      // !.!      Длинное имя файла с расширением
      if (strncmp(CurStr,"!.!",3)==0 && CurStr[3] != '?')
      {
        strcat(TmpStr,PassivePanel ? AnotherQuotedName:QuotedName);
        CurStr+=3;
_SVS(SysLog("!.! TmpStr=[%s]",TmpStr));
        continue;
      }

      // !~       Короткое имя файла без расширения
      if (strncmp(CurStr,"!~",2)==0)
      {
        strcat(TmpStr,PassivePanel ? AnotherShortNameOnly:ShortNameOnly);
        CurStr+=2;
_SVS(SysLog("!~ TmpStr=[%s]",TmpStr));
        continue;
      }

      // !`  Длинное расширение файла без имени
      if (strncmp(CurStr,"!`",2)==0)
      {
        char *Ext;
        if(CurStr[2] == '~')
        {
          Ext=strrchr((PassivePanel ? AnotherQuotedShortName:QuotedShortName),'.');
          CurStr+=3;
        }
        else
        {
          Ext=strrchr((PassivePanel ? AnotherQuotedName:QuotedName),'.');
          CurStr+=2;
        }
        if(Ext && *Ext)
          strcat(TmpStr,Ext);
_SVS(SysLog("!` TmpStr=[%s]",TmpStr));
        continue;
      }

      // !\!.!  Полное имя файла
      if (strncmp(CurStr,"!\\!.!",5)==0 && CurStr[5] != '?')
      {
        char CurDir[NM];
        char *FileName=PassivePanel ? AnotherName:Name;
        if (strpbrk(FileName,"\\:")==NULL)
        {
          if (PassivePanel)
            AnotherPanel->GetCurDir(CurDir);
          else
            strcpy(CurDir,CmdDir);
          AddEndSlash(CurDir);
        }
        else
          *CurDir=0;
        strcat(CurDir,FileName);
        QuoteSpace(CurDir);
        CurStr+=5;
        if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
        strcat(TmpStr,CurDir);
_SVS(SysLog("!\\!.! TmpStr=[%s]",TmpStr));
        continue;
      }

      // !& !&~  список файлов разделенных пробелом.
      if (!strncmp(CurStr,"!&~",3) && CurStr[3] != '?' ||
          !strncmp(CurStr,"!&",2) && CurStr[2] != '?')
      {
        char FileNameL[NM],ShortNameL[NM];
        Panel *WPanel=PassivePanel?AnotherPanel:CtrlObject->Cp()->ActivePanel;
        int FileAttrL;
        int ShortN0=FALSE;
        int CntSkip=2;
        if(CurStr[2] == '~')
        {
          ShortN0=TRUE;
          CntSkip++;
        }
        WPanel->GetSelName(NULL,FileAttrL);
        while (WPanel->GetSelName(FileNameL,FileAttrL,ShortNameL))
        {
          if (ShortN0)
            strcpy(FileNameL,ShortNameL);
// Вот здесь фиг его знает - нужно/ненужно...
//   если будет нужно - раскомментируем :-)
//          if(FileAttrL & FA_DIREC)
//            AddEndSlash(FileNameL);
          QuoteSpaceOnly(FileNameL);
          strcat(TmpStr," ");
          strcat(TmpStr,FileNameL);
        }
        CurStr+=CntSkip;
_SVS(SysLog("!& TmpStr=[%s]",TmpStr));
        continue;
      }

      // !@  Имя файла, содержащего имена помеченных файлов
      if (strncmp(CurStr,"!@",2)==0 && ListName!=NULL)
      {
        char Modifers[32]="", *Ptr;

        if((Ptr=strchr(CurStr+2,'!')) != NULL)
        {
          if(Ptr[1] != '?')
          {
            *Ptr=0;
            strncpy(Modifers,CurStr+2,sizeof(Modifers)-1);
            /* $ 02.09.2000 tran
               !@!, !#!@! bug */
            if ( PassivePanel && ( ListName[NM] || AnotherPanel->MakeListFile(ListName+NM,FALSE,Modifers)))
            {
              strcat(TmpStr,ListName+NM);
            }
            if ( !PassivePanel && (*ListName || CtrlObject->Cp()->ActivePanel->MakeListFile(ListName,FALSE,Modifers)))
            {
              strcat(TmpStr,ListName);
            }
            /* tran $ */
            CurStr+=Ptr-CurStr+1;
            continue;
          }
        }
      }

      // !$!      Имя файла, содержащего короткие имена помеченных файлов
      if (strncmp(CurStr,"!$",2)==0 && ShortListName!=NULL)
      {
        char Modifers[32]="", *Ptr;

        if((Ptr=strchr(CurStr+2,'!')) != NULL)
        {
          if(Ptr[1] != '?')
          {
            *Ptr=0;
            strncpy(Modifers,CurStr+2,sizeof(Modifers)-1);
            /* $ 02.09.2000 tran
               !@!, !#!@! bug */
            if ( PassivePanel && (ShortListName[NM] || AnotherPanel->MakeListFile(ShortListName+NM,TRUE,Modifers)))
            {
              /* $ 01.11.2000 IS
                 Имя файла в данном случае должно быть коротким
              */
              ConvertNameToShort(ShortListName+NM,ShortListName+NM);
              /* IS $ */
              strcat(TmpStr,ShortListName+NM);
            }
            if ( !PassivePanel && (*ShortListName || CtrlObject->Cp()->ActivePanel->MakeListFile(ShortListName,TRUE,Modifers)))
            {
              /* $ 01.11.2000 IS
                 Имя файла в данном случае должно быть коротким
              */
              ConvertNameToShort(ShortListName,ShortListName);
              /* IS $ */
              strcat(TmpStr,ShortListName);
            }
            /* tran $ */
            CurStr+=Ptr-CurStr+1;
_SVS(SysLog("!$! TmpStr=[%s]",TmpStr));
            continue;
          }
        }
      }

      // !-!      Короткое имя файла с расширением
      if (strncmp(CurStr,"!-!",3)==0 && CurStr[3] != '?')
      {
        strcat(TmpStr,PassivePanel ? AnotherQuotedShortName:QuotedShortName);
        CurStr+=3;
_SVS(SysLog("!-! TmpStr=[%s]",TmpStr));
        continue;
      }

      // !+!      Аналогично !-!, но если длинное имя файла утеряно
      //          после выполнения команды, FAR восстановит его
      if (strncmp(CurStr,"!+!",3)==0 && CurStr[3] != '?')
      {
        strcat(TmpStr,PassivePanel ? AnotherQuotedShortName:QuotedShortName);
        CurStr+=3;
        PreserveLFN=TRUE;
_SVS(SysLog("!+! TmpStr=[%s]",TmpStr));
        continue;
      }

      // !:       Текущий диск
      if (strncmp(CurStr,"!:",2)==0)
      {
        char CurDir[NM];
        if (*Name && Name[1]==':')
          strcpy(CurDir,Name);
        else
          if (PassivePanel)
            AnotherPanel->GetCurDir(CurDir);
          else
            strcpy(CurDir,CmdDir);
        CurDir[2]=0;
        if (*CurDir && CurDir[1]!=':')
          *CurDir=0;
        if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
        strcat(TmpStr,CurDir);
        CurStr+=2;
_SVS(SysLog("!: TmpStr=[%s]",TmpStr));
        continue;
      }

      // !\       Текущий путь
      if (strncmp(CurStr,"!\\",2)==0)
      {
        char CurDir[NM];
        if (PassivePanel)
          AnotherPanel->GetCurDir(CurDir);
        else
          strcpy(CurDir,CmdDir);
        AddEndSlash(CurDir);
        CurStr+=2;
        if (*CurStr=='!')
        {
          strcpy(QuotedName,Name);
          strcpy(QuotedShortName,ShortName);
          if (strpbrk(Name,"\\:")!=NULL)
            *CurDir=0;
        }
        if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
        strcat(TmpStr,CurDir);
_SVS(SysLog("!\\ TmpStr=[%s] CurDir=[%s]",TmpStr, CurDir));
        continue;
      }

      // !/       Короткое имя текущего пути
      if (strncmp(CurStr,"!/",2)==0)
      {
        char CurDir[NM];
        if (PassivePanel)
          AnotherPanel->GetCurDir(CurDir);
        else
          strcpy(CurDir,CmdDir);
        ConvertNameToShort(CurDir,CurDir);
        AddEndSlash(CurDir);
        CurStr+=2;
        if (*CurStr=='!')
        {
          strcpy(QuotedName,Name);
          strcpy(QuotedShortName,ShortName);
          if (strpbrk(Name,"\\:")!=NULL)
          {
            if (PointToName(ShortName)==ShortName)
            {
              strcpy(QuotedShortName,CurDir);
              AddEndSlash(QuotedShortName);
              strcat(QuotedShortName,ShortName);
            }
            *CurDir=0;
          }
        }
        if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
        strcat(TmpStr,CurDir);
_SVS(SysLog("!/ TmpStr=[%s]",TmpStr));
        continue;
      }

      // !?<title>?<init>!
      if (strncmp(CurStr,"!?",2)==0 && strchr(CurStr+2,'!')!=NULL)
      {
        char *NewCurStr=strchr(CurStr+2,'!')+1;
        strncat(TmpStr,CurStr,NewCurStr-CurStr);
        CurStr=NewCurStr;
_SVS(SysLog("!? TmpStr=[%s]",TmpStr));
        continue;
      }

      // !        Длинное имя файла без расширения
      if (*CurStr=='!')
      {
        if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
        strcat(TmpStr,PointToName(PassivePanel ? AnotherNameOnly:NameOnly));
        CurStr++;
_SVS(SysLog("! TmpStr=[%s]",TmpStr));
        continue;
      }

    } // End: if (!SkipQuotes)

    strncat(TmpStr,CurStr,1);
    CurStr++;
_SVS(++Pass);
  }
  if(DirBegin && DirBegin != Str && *(DirBegin-1) != '"')
  {
    Unquote(DirBegin);
    QuoteSpaceOnly(DirBegin);
  }
  if (!IgnoreInput)
    ReplaceVariables(TmpStr);
  strcpy(Str,TmpStr);

_SVS(SysLog(-1));
_SVS(SysLog("[%s]\n",Str));
  return(PreserveLFN);
}


void ReplaceVariables(char *Str)
{
  const int MaxSize=20;
  char *StartStr=Str;

  if (*Str=='\"')
    while (*Str && *Str!='\"')
      Str++;

  struct DialogItem *DlgData=NULL;
  int DlgSize=0;
  int StrPos[64],StrPosSize=0;

  while (*Str && DlgSize<MaxSize)
  {
    if (*(Str++)!='!')
      continue;
    if (*(Str++)!='?')
      continue;
    if (strchr(Str,'!')==NULL)
      return;
    StrPos[StrPosSize++]=Str-StartStr-2;
    DlgData=(struct DialogItem *)realloc(DlgData,(DlgSize+2)*sizeof(*DlgData));
    memset(&DlgData[DlgSize],0,2*sizeof(*DlgData));
    DlgData[DlgSize].Type=DI_TEXT;
    DlgData[DlgSize].X1=5;
    DlgData[DlgSize].Y1=DlgSize+2;
    DlgData[DlgSize+1].Type=DI_EDIT;
    DlgData[DlgSize+1].X1=5;
    DlgData[DlgSize+1].X2=70;
    DlgData[DlgSize+1].Y1=DlgSize+3;
    DlgData[DlgSize+1].Flags|=DIF_HISTORY|DIF_USELASTHISTORY;

    char HistoryName[MaxSize][20];
    int HistoryNumber=DlgSize/2;
    sprintf(HistoryName[HistoryNumber],"UserVar%d",HistoryNumber);
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
    char Title[256];
    strcpy(Title,Str);
    *strchr(Title,'!')=0;
    Str+=strlen(Title)+1;
    char *SrcText=strchr(Title,'?');
    if (SrcText!=NULL)
    {
      *SrcText=0;
      strcpy(DlgData[DlgSize+1].Data,SrcText+1);
    }
    strcpy(DlgData[DlgSize].Data,Title);
    /* $ 01.08.2000 SVS
       "расширяем" заголовок
    */
    ExpandEnvironmentStr(DlgData[DlgSize].Data,DlgData[DlgSize].Data,sizeof(DlgData[DlgSize].Data));
    /* SVS $*/
    DlgSize+=2;
  }
  if (DlgSize==0)
    return;
  DlgData=(struct DialogItem *)realloc(DlgData,(DlgSize+1)*sizeof(*DlgData));
  memset(&DlgData[DlgSize],0,sizeof(*DlgData));
  DlgData[DlgSize].Type=DI_DOUBLEBOX;
  DlgData[DlgSize].X1=3;
  DlgData[DlgSize].Y1=1;
  DlgData[DlgSize].X2=72;
  DlgData[DlgSize].Y2=DlgSize+2;
  DlgSize++;
  Dialog Dlg(DlgData,DlgSize);
  Dlg.SetPosition(-1,-1,76,DlgSize+3);
  Dlg.Process();
  if (Dlg.GetExitCode()==-1)
  {
    /* $ 13.07.2000 SVS
       запрос был по realloc
    */
    free(DlgData);
    /* SVS $ */
    *StartStr=0;
    return;
  }
  char TmpStr[4096];
  *TmpStr=0;
  for (Str=StartStr;*Str!=0;Str++)
  {
    int Replace=-1;
    for (int I=0;I<StrPosSize;I++)
      if (Str-StartStr==StrPos[I])
      {
        Replace=I;
        break;
      }
    if (Replace!=-1)
    {
      strcat(TmpStr,DlgData[Replace*2+1].Data);
      Str=strchr(Str+1,'!');
    }
    else
      strncat(TmpStr,Str,1);
  }
  strcpy(StartStr,TmpStr);
  /* $ 01.08.2000 SVS
     после "жмаканья" Enter "расширяем" данные
  */
  ExpandEnvironmentStr(TmpStr,StartStr,sizeof(DlgData[0].Data));
  /* SVS $ */
  /* $ 13.07.2000 SVS
     запрос был по realloc
  */
  free(DlgData);
  /* SVS $ */
}
