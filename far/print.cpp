/*
print.cpp

Печать (Alt-F5)

*/

/* Revision: 1.19 25.04.2005 $ */

/*
Modify:
  24.04.2005 AY
    ! GCC
  12.04.2005 SVS
    ! Из печати исключаем DIRs.
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  01.03.2004 SVS
    ! Обертки FAR_OemTo* и FAR_CharTo* вокруг одноименных WinAPI-функций
      (задел на будущее + править впоследствии только 1 файл)
  20.02.2003 SVS
    ! Заменим strcmp(FooBar,"..") на TestParentFolderName(FooBar)
  26.01.2003 IS
    ! FAR_DeleteFile вместо DeleteFile, FAR_RemoveDirectory вместо
      RemoveDirectory, просьба и впредь их использовать для удаления
      соответственно файлов и каталогов.
  01.03.2002 SVS
    ! Есть только одна функция создания временного файла - FarMkTempEx
  21.10.2001 SVS
    + CALLBACK-функция для избавления от BugZ#85
  27.09.2001 IS
    - Левый размер при использовании strncpy
  26.07.2001 SVS
    ! VFMenu уничтожен как класс
  18.07.2001 OT
    ! VFMenu
  03.06.2001 SVS
    ! Изменения в связи с переделкой UserData в VMenu
  21.05.2001 SVS
    ! struct MenuData|MenuItem
      Поля Selected, Checked, Separator и Disabled преобразованы в DWORD Flags
    ! Константы MENU_ - в морг
  06.05.2001 DJ
    ! перетрях #include
  27.02.2001 VVM
    ! Символы, зависимые от кодовой страницы
      /[\x01-\x08\x0B-\x0C\x0E-\x1F\xB0-\xDF\xF8-\xFF]/
      переведены в коды.
  11.02.2001 SVS
    ! Несколько уточнений кода в связи с изменениями в структуре MenuItem
  11.11.2000 SVS
    ! FarMkTemp() - убираем (как всегда - то ставим, то тут же убираем :-(((
  11.11.2000 SVS
    ! Используем конструкцию FarMkTemp()
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "global.hpp"
#include "lang.hpp"
#include "panel.hpp"
#include "vmenu.hpp"
#include "filelist.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"

static void AddToPrintersMenu(VMenu *PrinterList,PRINTER_INFO_2 *pi,
                              int PrinterNumber);

static int DefaultPrinterFound;

static void PR_PrintMsg(void)
{
  Message(0,0,MSG(MPrintTitle),MSG(MPreparingForPrinting));
}

void PrintFiles(Panel *SrcPanel)
{
  _ALGO(CleverSysLog clv("Alt-F5 (PrintFiles)"));
  char PrinterName[200];
  DWORD Needed,Returned;
  int PrinterNumber;
  int FileAttr;
  char SelName[NM];

  long DirsCount=0;
  int SelCount=SrcPanel->GetSelCount();

  if (SelCount==0)
  {
    _ALGO(SysLog("Error: SelCount==0"));
    return;
  }

  // проверка каталогов
  _ALGO(SysLog("Check for FA_DIREC"));
  SrcPanel->GetSelName(NULL,FileAttr);
  while (SrcPanel->GetSelName(SelName,FileAttr))
  {
    if (TestParentFolderName(SelName) || (FileAttr & FA_DIREC))
      DirsCount++;
  }

  if (DirsCount==SelCount)
  {
    _ALGO(SysLog("Error: DirsCount==SelCount"));
    return;
  }

  DefaultPrinterFound=FALSE;

  const int pi_count=1024;
  PRINTER_INFO_2 *pi=new PRINTER_INFO_2[pi_count];

  _ALGO(SysLog("EnumPrinters"));
  if (pi==NULL || !EnumPrinters(PRINTER_ENUM_LOCAL,NULL,2,(LPBYTE)pi,pi_count*sizeof(PRINTER_INFO_2),&Needed,&Returned))
  {
    /* $ 13.07.2000 SVS
       использовалась new[]
    */
    delete[] pi;
    /* SVS $ */
    _ALGO(SysLog("Error: Printer not found"));
    return;
  }

  {
    _ALGO(CleverSysLog clv2("Show Menu"));
    char Title[200];
    char Name[NM];

    if (SelCount==1)
    {
      SrcPanel->GetSelName(NULL,FileAttr);
      SrcPanel->GetSelName(Name,FileAttr);
      TruncStr(Name,50);
      sprintf(SelName,"\"%s\"",Name);
      sprintf(Title,MSG(MPrintTo),SelName);
    }
    else
    {
      _ALGO(SysLog("Correct: SelCount-=DirsCount"));
      SelCount-=DirsCount;
      sprintf(Title,MSG(MPrintFilesTo),SelCount);
    }

    VMenu PrinterList(Title,NULL,0,ScrY-4);
    PrinterList.SetFlags(VMENU_WRAPMODE|VMENU_SHOWAMPERSAND);
    PrinterList.SetPosition(-1,-1,0,0);

    AddToPrintersMenu(&PrinterList,pi,Returned);

    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
      DWORD NetReturned;
      if (EnumPrinters(PRINTER_ENUM_CONNECTIONS,NULL,2,(LPBYTE)pi,pi_count*sizeof(PRINTER_INFO_2),&Needed,&NetReturned))
      {
        AddToPrintersMenu(&PrinterList,pi,NetReturned);
        Returned+=NetReturned;
      }
    }

    PrinterList.Process();
    PrinterNumber=PrinterList.Modal::GetExitCode();
    if (PrinterNumber<0)
    {
      delete[] pi;
      _ALGO(SysLog("ESC"));
      return;
    }
    PrinterList.GetUserData(PrinterName,sizeof(PrinterName));
  }

  HANDLE hPrinter;
  if (!OpenPrinter(PrinterName,&hPrinter,NULL))
  {
    Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MPrintTitle),MSG(MCannotOpenPrinter),
            PrinterName,MSG(MOk));
    delete[] pi;
    _ALGO(SysLog("Error: Cannot Open Printer"));
    return;
  }

  {
    _ALGO(CleverSysLog clv3("Print selected Files"));
    //SaveScreen SaveScr;
    SetCursorType(FALSE,0);

    SetPreRedrawFunc(PR_PrintMsg);
    PR_PrintMsg();

    HANDLE hPlugin=SrcPanel->GetPluginHandle();
    int PluginMode=SrcPanel->GetMode()==PLUGIN_PANEL &&
        !CtrlObject->Plugins.UseFarCommand(hPlugin,PLUGIN_FARGETFILE);

    SrcPanel->GetSelName(NULL,FileAttr);
    while (SrcPanel->GetSelName(SelName,FileAttr))
    {
      if (TestParentFolderName(SelName) || (FileAttr & FA_DIREC))
        continue;
      int Success=FALSE;

      char TempDir[NM],TempName[NM];
      *TempName=0;

      FILE *SrcFile=NULL;
      if (PluginMode)
      {
        if (FarMkTempEx(TempDir))
        {
          CreateDirectory(TempDir,NULL);
          struct FileListItem ListItem;
          if (SrcPanel->GetLastSelectedItem(&ListItem))
          {
            struct PluginPanelItem PanelItem;
            FileList::FileListToPluginItem(&ListItem,&PanelItem);
            if (CtrlObject->Plugins.GetFile(hPlugin,&PanelItem,TempDir,TempName,OPM_SILENT))
              SrcFile=fopen(TempName,"rb");
            else
              FAR_RemoveDirectory(TempDir);
          }
        }
      }
      else
        SrcFile=fopen(SelName,"rb");

      if (SrcFile!=NULL)
      {
        DOC_INFO_1 di1;
        char AnsiName[NM];
        FAR_OemToChar(SelName,AnsiName);
        di1.pDocName=AnsiName;
        di1.pOutputFile=NULL;
        di1.pDatatype=NULL;

        if (StartDocPrinter(hPrinter,1,(LPBYTE)&di1))
        {
          char Buffer[8192];
          DWORD Read,Written;
          Success=TRUE;
          while ((Read=fread(Buffer,1,sizeof(Buffer),SrcFile))>0)
            if (!WritePrinter(hPrinter,Buffer,Read,&Written))
            {
              Success=FALSE;
              break;
            }
          EndDocPrinter(hPrinter);
        }
        fclose(SrcFile);
      }
      if (*TempName)
        DeleteFileWithFolder(TempName);
      if (Success)
        SrcPanel->ClearLastGetSelection();
      else
      {
        SetPreRedrawFunc(NULL); //??
        if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MPrintTitle),MSG(MCannotPrint),
                    SelName,MSG(MSkip),MSG(MCancel))!=0)
          break;
      }
    }
    ClosePrinter(hPrinter);
    SetPreRedrawFunc(NULL);
  }
  SrcPanel->Redraw();
  delete[] pi;
}


static void AddToPrintersMenu(VMenu *PrinterList,PRINTER_INFO_2 *pi,
                              int PrinterNumber)
{
  int IDItem;
  for (int I=0;I<PrinterNumber;I++)
  {
    char MenuText[200],PrinterName[200];
    struct MenuItem ListItem;
    memset(&ListItem,0,sizeof(ListItem));
    FAR_CharToOem(NullToEmpty(pi[I].pPrinterName),PrinterName);
    if (pi[I].pComment!=NULL)
      FAR_CharToOem(pi[I].pComment,pi[I].pComment);
    sprintf(MenuText,"%-22.22s %c %-10s %3d %s  %s",PrinterName,0x0B3U,
            NullToEmpty(pi[I].pPortName),pi[I].cJobs,MSG(MJobs),
            NullToEmpty(pi[I].pComment));
    xstrncpy(ListItem.Name,MenuText,sizeof(ListItem.Name)-1);
    if ((pi[I].Attributes & PRINTER_ATTRIBUTE_DEFAULT) && !DefaultPrinterFound)
    {
      DefaultPrinterFound=TRUE;
      ListItem.SetSelect(TRUE);
    }
    else
      ListItem.SetSelect(FALSE);
    IDItem=PrinterList->AddItem(&ListItem);
    // А вот теперь добавим данные для этого пункта (0 - передаем строку)
    PrinterList->SetUserData((void *)NullToEmpty(pi[I].pPrinterName),0,IDItem);
  }
}
