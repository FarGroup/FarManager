/*
fnparce.cpp

Парсер файловых ассоциаций

*/

/* Revision: 1.17 05.09.2003 $ */

/*
Modify:
  05.09.2003 SVS
    - Некорректно формировался файл-список с полными именами (метасимвол "!@F!")
      на панели плагина TempPanel. Каждый файл дополнялся путем из родительской
      панели.
  05.03.2003 SVS
    ! Закоментим _SVS
  21.01.2003 SVS
    + xf_malloc,xf_realloc,xf_free - обертки вокруг malloc,realloc,free
      Просьба блюсти порядок и прописывать именно xf_* вместо простых.
  13.12.2002 SVS
    ! Из функции SubstFileName парсер метасимволов вынесен в отдельную процедуру.
      Это позволит сделать функцию рекурсивной... а в последствии
      ввести операторные скобоки
  30.09.2002 VVM
    - Вместо strcpy будем пользовать strncpy.
  12.08.2002 SVS
    - Трабла с уничтожением данных и деструктором диалога.
  15.04.2002 SVS
    - Dialog Dlg(DlgData,DlgSize); нужно в скобки брать в функции ReplaceVariables()
  05.03.2002 DJ
    - борьба с переполнением буфера
  01.03.2002 SVS
    ! Есть только одна функция создания временного файла - FarMkTempEx
  26.01.2002 VVM
    ! При обработке "!&" пробел в начале списка не ставится.
  25.01.2002 SVS
    - Бага при выводе меню, когда юзаются метасимволы !@! и !$! - просто
      оставим их как есть.
  07.12.2001 SVS
    - BugZ#149 - Метасимвол !& должен строить список с кавычками
      в список всеже должно попасть имя в кавычках.
  05.10.2001 SVS
    ! Перенос функции Panel::MakeListFile() из panel.cpp - здесь ей место
  25.07.2001 IS
    ! для !` не ставим ведущую точку.
  29.06.2001 IS
    ! Продолжаем убирать обработку кавычек - выкосил все, что имело отношение к
      "quote"
  22.06.2001 SVS
    ! Убираем обработку кавычек
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
#include "global.hpp"
#include "lang.hpp"

struct TSubstData
{
  // параметры функции SubstFileName
  char *Name;           // Длинное имя
  char *ShortName;      // Короткое имя
  char *ListName;       // Длинное имя файла-списка
  char *ShortListName;  // Короткое имя файла-списка

  // локальные переменные
  char AnotherName[NM];
  char AnotherShortName[NM];
  char NameOnly[NM];
  char ShortNameOnly[NM];
  char AnotherNameOnly[NM];
  char AnotherShortNameOnly[NM];
  char CmdDir[NM];
  int  PreserveLFN;
  int  PassivePanel;

  Panel *AnotherPanel;
  Panel *ActivePanel;
};


static void ReplaceVariables(char *Str);
static char *_SubstFileName(char *CurStr,struct TSubstData *PSubstData,char *TempStr,int MaxTempStrSize);

// Str=if exist !#!\!^!.! far:edit < diff -c -p "!#!\!^!.!" !\!.!

/*
  SubstFileName()
  Преобразование метасимволов ассоциации файлов в реальные значения

  Входные ListName и ShortListName обЯзаны иметь размер NM*2 !!!
*/
int SubstFileName(char *Str,            // результирующая строка
                  int   StrSize,        // /* $ 05.03.2002 DJ */ размер буфера результирующей строки /* DJ $ */
                  char *Name,           // Длинное имя
                  char *ShortName,      // Короткое имя
                  char *ListName,       // Длинное имя файла-списка
                  char *ShortListName,  // Короткое имя файла-списка
                  int   IgnoreInput,    // TRUE - не исполнять "!?<title>?<init>!"
                  char *CmdLineDir)     // Каталог исполнения
{
  //_SVS(CleverSysLog clv(Str));

  /* $ 19.06.2001 SVS
    ВНИМАНИЕ! Для альтернативных метасимволов, не основанных на "!",
    нужно будет либо убрать эту проверку либо изменить условие (последнее
    предпочтительнее!)
  */
  if(!strchr(Str,'!'))
    return FALSE;
  /* SVS $ */

  struct TSubstData SubstData, *PSubstData=&SubstData;
  char *ChPtr;

  // Сделаем пока поболее - 10240, но везде нужно проверять размер... (see below)
  char TmpStr[10240];
  char *CurStr;

  // <PreProcess>
  *TmpStr=0; // пока пусто.

  PSubstData->Name=Name;                    // Длинное имя
  PSubstData->ShortName=ShortName;          // Короткое имя
  PSubstData->ListName=ListName;            // Длинное имя файла-списка
  PSubstData->ShortListName=ShortListName;  // Короткое имя файла-списка

  // Если имя текущего каталога не задано...
  if (CmdLineDir!=NULL)
    strcpy(PSubstData->CmdDir,CmdLineDir);
  else // ...спросим у ком.строки
    CtrlObject->CmdLine->GetCurDir(PSubstData->CmdDir);

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
  strcpy(PSubstData->NameOnly,Name);
  if ((ChPtr=strrchr(PSubstData->NameOnly,'.'))!=NULL)
    *ChPtr=0;

  strcpy(PSubstData->ShortNameOnly,ShortName);
  if ((ChPtr=strrchr(PSubstData->ShortNameOnly,'.'))!=NULL)
    *ChPtr=0;

  PSubstData->ActivePanel=CtrlObject->Cp()->ActivePanel;
  PSubstData->AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(PSubstData->ActivePanel);
  PSubstData->AnotherPanel->GetCurName(PSubstData->AnotherName,PSubstData->AnotherShortName);
  strcpy(PSubstData->AnotherNameOnly,PSubstData->AnotherName);
  if ((ChPtr=strrchr(PSubstData->AnotherNameOnly,'.'))!=NULL)
    *ChPtr=0;

  strcpy(PSubstData->AnotherShortNameOnly,PSubstData->AnotherShortName);
  if ((ChPtr=strrchr(PSubstData->AnotherShortNameOnly,'.'))!=NULL)
    *ChPtr=0;

  PSubstData->PreserveLFN=FALSE;
  PSubstData->PassivePanel=FALSE; // первоначально речь идет про активную панель!
  // </PreProcess>

  //_SVS(int Pass=1);

  CurStr=Str;
  while (*CurStr)
  {
    //_SVS(SysLog("***** Pass=%d",Pass));
    if(*CurStr == '!')
      CurStr=_SubstFileName(CurStr,PSubstData,TmpStr,sizeof(TmpStr)-strlen(TmpStr)-1);
    else                                           //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ как способ проверки!
    {
      strncat(TmpStr,CurStr,1);
      CurStr++;
    }
    //_SVS(++Pass);
  }

  if (!IgnoreInput)
    ReplaceVariables(TmpStr);
  strncpy(Str,TmpStr,StrSize-1);

  //_SVS(SysLog("[%s]\n",Str));
  return(PSubstData->PreserveLFN);
}

static char *_SubstFileName(char *CurStr,struct TSubstData *PSubstData,char *TmpStr,int MaxTempStrSize)
{
  //_SVS(CleverSysLog clv("*** _SubstFileName ***"));

//  char TmpName[NM+2];
//  char TmpShortName[NM+2];
//  char *DirBegin=NULL; // начало имени каталога (!?)
#if 0
  // "НАЧАЛО" операторных скобок
  if(*CurStr == '(')
  {
    if(*++CurStr != '(')
    {
      char *PCurStr=strchr(CurStr,')');
      if(PCurStr)
      {
        *PCurStr=0;
        _SubstFileName(CurStr,PSubstData,TmpStr,MaxTempStrSize);
        CurStr=PCurStr+1;
      }
    }
    return CurStr;
  }
#endif

  // рассмотрим переключатели активности/пассивности панели.
  if (strncmp(CurStr,"!#",2)==0)
  {
    CurStr+=2;
    PSubstData->PassivePanel=TRUE;
    //_SVS(SysLog("PassivePanel=TRUE '%s'",CurStr));
    return CurStr;
  }

  if (strncmp(CurStr,"!^",2)==0)
  {
    CurStr+=2;
    PSubstData->PassivePanel=FALSE;
    //_SVS(SysLog("PassivePanel=FALSE '%s'",CurStr));
    return CurStr;
  }

  // !! символ '!'
  if (strncmp(CurStr,"!!",2)==0 && CurStr[2] != '?')
  {
    strncat(TmpStr,"!",MaxTempStrSize-1);
    CurStr+=2;
    //_SVS(SysLog("!! TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !.!      Длинное имя файла с расширением
  if (strncmp(CurStr,"!.!",3)==0 && CurStr[3] != '?')
  {
    strncat(TmpStr,PSubstData->PassivePanel ? PSubstData->AnotherName:PSubstData->Name, MaxTempStrSize-1);
    CurStr+=3;
    //_SVS(SysLog("!.! TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !~       Короткое имя файла без расширения
  if (strncmp(CurStr,"!~",2)==0)
  {
    strncat(TmpStr,PSubstData->PassivePanel ? PSubstData->AnotherShortNameOnly:PSubstData->ShortNameOnly, MaxTempStrSize-1);
    CurStr+=2;
    //_SVS(SysLog("!~ TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !`  Длинное расширение файла без имени
  if (strncmp(CurStr,"!`",2)==0)
  {
    char *Ext;
    if(CurStr[2] == '~')
    {
      Ext=strrchr((PSubstData->PassivePanel ? PSubstData->AnotherShortName:PSubstData->ShortName),'.');
      CurStr+=3;
    }
    else
    {
      Ext=strrchr((PSubstData->PassivePanel ? PSubstData->AnotherName:PSubstData->Name),'.');
      CurStr+=2;
    }
    if(Ext && *Ext)
      strncat(TmpStr,++Ext, MaxTempStrSize-1);
    //_SVS(SysLog("!` TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !\!.!  Полное имя файла
  if (strncmp(CurStr,"!\\!.!",5)==0 && CurStr[5] != '?')
  {
    char CurDir[NM];
    char *FileName=PSubstData->PassivePanel ? PSubstData->AnotherName:PSubstData->Name;
    if (strpbrk(FileName,"\\:")==NULL)
    {
      if (PSubstData->PassivePanel)
        PSubstData->AnotherPanel->GetCurDir(CurDir);
      else
        strcpy(CurDir,PSubstData->CmdDir);
      AddEndSlash(CurDir);
    }
    else
      *CurDir=0;
    strncat(CurDir,FileName,sizeof (CurDir)-1);
    CurStr+=5;
//    if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
    strncat(TmpStr,CurDir, MaxTempStrSize-1);
    //_SVS(SysLog("!\\!.! TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !& !&~  список файлов разделенных пробелом.
  if (!strncmp(CurStr,"!&~",3) && CurStr[3] != '?' ||
      !strncmp(CurStr,"!&",2) && CurStr[2] != '?')
  {
    char FileNameL[NM],ShortNameL[NM];
    Panel *WPanel=PSubstData->PassivePanel?PSubstData->AnotherPanel:PSubstData->ActivePanel;
    int FileAttrL;
    int ShortN0=FALSE;
    int CntSkip=2;
    if(CurStr[2] == '~')
    {
      ShortN0=TRUE;
      CntSkip++;
    }
    WPanel->GetSelName(NULL,FileAttrL);
    int First = TRUE;
    while (WPanel->GetSelName(FileNameL,FileAttrL,ShortNameL))
    {
      if (ShortN0)
        strcpy(FileNameL,ShortNameL);
      else // в список все же должно попасть имя в кавычках.
        QuoteSpaceOnly(FileNameL);
// Вот здесь фиг его знает - нужно/ненужно...
//   если будет нужно - раскомментируем :-)
//          if(FileAttrL & FA_DIREC)
//            AddEndSlash(FileNameL);
      // А нужен ли нам пробел в самом начале?
      if (First)
        First = FALSE;
      else
        strncat(TmpStr," ", MaxTempStrSize-1);
      strncat(TmpStr,FileNameL, MaxTempStrSize-1);
      /* $ 05.03.2002 DJ
         если в буфер больше не влезет - выйдем из цикла
      */
      if (strlen (TmpStr) >= MaxTempStrSize-1)
        break;
      /* DJ $ */
    }
    CurStr+=CntSkip;
    //_SVS(SysLog("!& TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !@  Имя файла, содержащего имена помеченных файлов
  if (strncmp(CurStr,"!@",2)==0)
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
        if(PSubstData->ListName!=NULL)
        {
          if ( PSubstData->PassivePanel && ( PSubstData->ListName[NM] || PSubstData->AnotherPanel->MakeListFile(PSubstData->ListName+NM,FALSE,Modifers)))
          {
            strncat(TmpStr,PSubstData->ListName+NM, MaxTempStrSize-1);
          }
          if ( !PSubstData->PassivePanel && (*PSubstData->ListName || PSubstData->ActivePanel->MakeListFile(PSubstData->ListName,FALSE,Modifers)))
          {
            strncat(TmpStr,PSubstData->ListName, MaxTempStrSize-1);
          }
        }
        else
        {
          strncat(TmpStr,CurStr, MaxTempStrSize-1);
          strncat(TmpStr,Modifers, MaxTempStrSize-1);
          strncat(TmpStr,"!", MaxTempStrSize-1);
        }
        /* tran $ */
        CurStr+=Ptr-CurStr+1;
        return CurStr;
      }
    }
  }

  // !$!      Имя файла, содержащего короткие имена помеченных файлов
  if (strncmp(CurStr,"!$",2)==0)
  {
    char Modifers[32]="", *Ptr;

    if((Ptr=strchr(CurStr+2,'!')) != NULL)
    {
      if(Ptr[1] != '?')
      {
        *Ptr=0;
        strncpy(Modifers,CurStr+2,MaxTempStrSize-1);
        /* $ 02.09.2000 tran
           !@!, !#!@! bug */
        if(PSubstData->ShortListName!=NULL)
        {
          if ( PSubstData->PassivePanel && (PSubstData->ShortListName[NM] || PSubstData->AnotherPanel->MakeListFile(PSubstData->ShortListName+NM,TRUE,Modifers)))
          {
            /* $ 01.11.2000 IS
               Имя файла в данном случае должно быть коротким
            */
            ConvertNameToShort(PSubstData->ShortListName+NM,PSubstData->ShortListName+NM);
            /* IS $ */
            strncat(TmpStr,PSubstData->ShortListName+NM, MaxTempStrSize-1);
          }
          if ( !PSubstData->PassivePanel && (*PSubstData->ShortListName || PSubstData->ActivePanel->MakeListFile(PSubstData->ShortListName,TRUE,Modifers)))
          {
            /* $ 01.11.2000 IS
               Имя файла в данном случае должно быть коротким
            */
            ConvertNameToShort(PSubstData->ShortListName,PSubstData->ShortListName);
            /* IS $ */
            strncat(TmpStr,PSubstData->ShortListName, MaxTempStrSize-1);
          }
          /* tran $ */
        }
        else
        {
          strncat(TmpStr,CurStr, MaxTempStrSize-1);
          strncat(TmpStr,Modifers, MaxTempStrSize-1);
          strncat(TmpStr,"!", MaxTempStrSize-1);
        }
        CurStr+=Ptr-CurStr+1;
        //_SVS(SysLog("!$! TmpStr=[%s]",TmpStr));
        return CurStr;
      }
    }
  }

  // !-!      Короткое имя файла с расширением
  if (strncmp(CurStr,"!-!",3)==0 && CurStr[3] != '?')
  {
    strncat(TmpStr,PSubstData->PassivePanel ? PSubstData->AnotherShortName:PSubstData->ShortName, MaxTempStrSize-1);
    CurStr+=3;
    //_SVS(SysLog("!-! TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !+!      Аналогично !-!, но если длинное имя файла утеряно
  //          после выполнения команды, FAR восстановит его
  if (strncmp(CurStr,"!+!",3)==0 && CurStr[3] != '?')
  {
    strncat(TmpStr,PSubstData->PassivePanel ? PSubstData->AnotherShortName:PSubstData->ShortName, MaxTempStrSize-1);
    CurStr+=3;
    PSubstData->PreserveLFN=TRUE;
    //_SVS(SysLog("!+! TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !:       Текущий диск
  if (strncmp(CurStr,"!:",2)==0)
  {
    char CurDir[NM];
    if (*PSubstData->Name && PSubstData->Name[1]==':')
      strcpy(CurDir,PSubstData->Name);
    else
      if (PSubstData->PassivePanel)
        PSubstData->AnotherPanel->GetCurDir(CurDir);
      else
        strcpy(CurDir,PSubstData->CmdDir);
    CurDir[2]=0;
    if (*CurDir && CurDir[1]!=':')
      *CurDir=0;
//    if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
    strncat(TmpStr,CurDir, MaxTempStrSize-1);
    CurStr+=2;
    //_SVS(SysLog("!: TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !\       Текущий путь
  if (strncmp(CurStr,"!\\",2)==0)
  {
    char CurDir[NM];
    if (PSubstData->PassivePanel)
      PSubstData->AnotherPanel->GetCurDir(CurDir);
    else
      strcpy(CurDir,PSubstData->CmdDir);
    AddEndSlash(CurDir);
    CurStr+=2;
    if (*CurStr=='!')
    {
//      strcpy(TmpName,PSubstData->Name);
//      strcpy(TmpShortName,PSubstData->ShortName);
      if (strpbrk(PSubstData->Name,"\\:")!=NULL)
        *CurDir=0;
    }
//    if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
    strncat(TmpStr,CurDir, MaxTempStrSize-1);
    //_SVS(SysLog("!\\ TmpStr=[%s] CurDir=[%s]",TmpStr, CurDir));
    return CurStr;
  }

  // !/       Короткое имя текущего пути
  if (strncmp(CurStr,"!/",2)==0)
  {
    char CurDir[NM];
    if (PSubstData->PassivePanel)
      PSubstData->AnotherPanel->GetCurDir(CurDir);
    else
      strcpy(CurDir,PSubstData->CmdDir);
    ConvertNameToShort(CurDir,CurDir);
    AddEndSlash(CurDir);
    CurStr+=2;
    if (*CurStr=='!')
    {
//      strcpy(TmpName,PSubstData->Name);
//      strcpy(TmpShortName,PSubstData->ShortName);
      if (strpbrk(PSubstData->Name,"\\:")!=NULL)
      {
//        if (PointToName(PSubstData->ShortName)==PSubstData->ShortName)
//        {
//          strcpy(TmpShortName,CurDir);
//          AddEndSlash(TmpShortName);
//          strcat(TmpShortName,PSubstData->ShortName);
//        }
        *CurDir=0;
      }
    }
//    if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
    strncat(TmpStr,CurDir, MaxTempStrSize-1);
    //_SVS(SysLog("!/ TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !?<title>?<init>!
  if (strncmp(CurStr,"!?",2)==0 && strchr(CurStr+2,'!')!=NULL)
  {
#if 0
    // "НАЧАЛО" операторных скобок
    char *Ptr=(CurStr+=2), *Ptr2;
    char TempStr2[2048];

    strcat(TmpStr,"!?");
    while(*Ptr)
    {
      if(*Ptr == '(' && Ptr[1] != '(')
      {
        Ptr2=Ptr+1;
        *TempStr2=0;
        while(*Ptr)
        {
          if(*Ptr == ')' && Ptr[1] != ')')
          {
            *Ptr=0;
            while (*Ptr2)
            {
              if(*Ptr2 == '!')
              {
                Ptr2=_SubstFileName(Ptr2,PSubstData,TempStr2,sizeof(TempStr2));
                strncat(TmpStr,TempStr2,strlen(TempStr2));
              }
              else
              {
                strncat(TmpStr,Ptr2,1);
                Ptr2++;
              }
            }
            *Ptr=')';
            break;
          }
          Ptr++;
        }
        if(*Ptr == 0)
          break;
      }
      else if(*Ptr == '!')
      {
        strcat(TmpStr,"!");
        break;
      }
      else if(*Ptr == '?')
      {
        strcat(TmpStr,"?");
      }
      else
        strncat(TmpStr,Ptr,1);
      Ptr++;
    }
    CurStr=Ptr;
#else
    char *NewCurStr=strchr(CurStr+2,'!')+1;
    strncat(TmpStr,CurStr,NewCurStr-CurStr);
    CurStr=NewCurStr;
#endif
    //_SVS(SysLog("!? TmpStr=[%s]",TmpStr));
    return CurStr;
  }

  // !        Длинное имя файла без расширения
  if (*CurStr=='!')
  {
//    if(!DirBegin) DirBegin=TmpStr+strlen(TmpStr);
    strncat(TmpStr,PointToName(PSubstData->PassivePanel ? PSubstData->AnotherNameOnly:PSubstData->NameOnly), MaxTempStrSize-1);
    CurStr++;
    //_SVS(SysLog("! TmpStr=[%s]",TmpStr));
  }

  return CurStr;
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
    DlgData=(struct DialogItem *)xf_realloc(DlgData,(DlgSize+2)*sizeof(*DlgData));
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
    strncpy(Title,Str,sizeof(Title)-1);
    *strchr(Title,'!')=0;
    Str+=strlen(Title)+1;
    char *SrcText=strchr(Title,'?');
    if (SrcText!=NULL)
    {
      *SrcText=0;
      strncpy(DlgData[DlgSize+1].Data,SrcText+1,sizeof(DlgData[DlgSize+1].Data)-1);
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
  DlgData=(struct DialogItem *)xf_realloc(DlgData,(DlgSize+1)*sizeof(*DlgData));
  memset(&DlgData[DlgSize],0,sizeof(*DlgData));
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
  xf_free(DlgData);
  /* SVS $ */
}

int Panel::MakeListFile(char *ListFileName,int ShortNames,char *Modifers)
{
  FILE *ListFile;

  if (!FarMkTempEx(ListFileName) || (ListFile=fopen(ListFileName,"wb"))==NULL)
  {
    Message(MSG_WARNING,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MCannotCreateListTemp),MSG(MOk));
    return(FALSE);
  }

  char FileName[NM*2],ShortName[NM];
  int FileAttr;
  GetSelName(NULL,FileAttr);
  while (GetSelName(FileName,FileAttr,ShortName))
  {
    if (ShortNames)
      strcpy(FileName,ShortName);

    if(Modifers && *Modifers)
    {
      if(strchr(Modifers,'F') && PointToName(FileName) == FileName) // 'F' - использовать полный путь;
      {
        char TempFileName[NM*2];
        strcpy(TempFileName,CurDir);
        sprintf(TempFileName,"%s%s%s",CurDir,(CurDir[strlen(CurDir)-1] != '\\'?"\\":""),FileName);
        if (ShortNames)
          ConvertNameToShort(TempFileName,TempFileName);
        strcpy(FileName,TempFileName);
      }
      if(strchr(Modifers,'Q')) // 'Q' - заключать имена с пробелами в кавычки;
        QuoteSpaceOnly(FileName);
      if(strchr(Modifers,'A')) // 'A' - использовать ANSI кодировку.
        OemToChar(FileName,FileName);

      if(strchr(Modifers,'S')) // 'S' - использовать '/' вместо '\' в путях файлов;
      {
        int I,Len=strlen(FileName);
        for(I=0; I < Len; ++I)
          if(FileName[I] == '\\')
            FileName[I]='/';
      }
    }
//_D(SysLog("%s[%s] %s",__FILE__,Modifers,FileName));
    if (fprintf(ListFile,"%s\r\n",FileName)==EOF)
    {
      fclose(ListFile);
      remove(ListFileName);
      Message(MSG_WARNING,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MCannotCreateListWrite),MSG(MOk));
      return(FALSE);
    }
  }
  if (fclose(ListFile)==EOF)
  {
    clearerr(ListFile);
    fclose(ListFile);
    remove(ListFileName);
    Message(MSG_WARNING,1,MSG(MError),MSG(MCannotCreateListFile),MSG(MOk));
    return(FALSE);
  }
  return(TRUE);
}
