/*
print.cpp

Печать (Alt-F5)

*/

/* Revision: 1.01 13.07.2000 $ */

/*
Modify:
  13.07.2000 SVS
    ! Некоторые коррекции при использовании new/delete/realloc
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop


/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

static void AddToPrintersMenu(VMenu *PrinterList,PRINTER_INFO_2 *pi,
                              int PrinterNumber);

static int DefaultPrinterFound;

void PrintFiles(Panel *SrcPanel)
{
  char PrinterName[200];
  DWORD Needed,Returned;
  int PrinterNumber;

  DefaultPrinterFound=FALSE;

  int SelCount=SrcPanel->GetSelCount();
  if (SelCount==0)
    return;

  const int pi_count=1024;
  PRINTER_INFO_2 *pi=new PRINTER_INFO_2[pi_count];

  if (pi==NULL || !EnumPrinters(PRINTER_ENUM_LOCAL,NULL,2,(LPBYTE)pi,pi_count*sizeof(PRINTER_INFO_2),&Needed,&Returned))
  {
    /* $ 13.07.2000 SVS
       использовалась new[]
    */
    delete[] pi;
    /* SVS $ */
    return;
  }
  {
    char Title[200];

    if (SelCount==1)
    {
      char Name[NM],SelName[NM];
      int FileAttr;
      SrcPanel->GetSelName(NULL,FileAttr);
      SrcPanel->GetSelName(Name,FileAttr);
      TruncStr(Name,50);
      sprintf(SelName,"\"%s\"",Name);
      sprintf(Title,MSG(MPrintTo),SelName);
    }
    else
      sprintf(Title,MSG(MPrintFilesTo),SelCount);

    VMenu PrinterList(Title,NULL,0,ScrY-4);
    PrinterList.SetFlags(MENU_WRAPMODE|MENU_SHOWAMPERSAND);
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
    PrinterNumber=PrinterList.GetExitCode();
    if (PrinterNumber<0)
    {
      /* $ 13.07.2000 SVS
         использовалась new[]
      */
      delete[] pi;
      /* SVS $ */
      return;
    }
    PrinterList.GetUserData(PrinterName,sizeof(PrinterName));
  }

  HANDLE hPrinter;
  if (!OpenPrinter(PrinterName,&hPrinter,NULL))
  {
    Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MPrintTitle),MSG(MCannotOpenPrinter),
            PrinterName,MSG(MOk));
    /* $ 13.07.2000 SVS
       использовалась new[]
    */
    delete[] pi;
    /* SVS $ */
    return;
  }
  {
    SaveScreen SaveScr;
    SetCursorType(FALSE,0);
    Message(0,0,MSG(MPrintTitle),MSG(MPreparingForPrinting));

    HANDLE hPlugin=SrcPanel->GetPluginHandle();
    int PluginMode=SrcPanel->GetMode()==PLUGIN_PANEL &&
        !CtrlObject->Plugins.UseFarCommand(hPlugin,PLUGIN_FARGETFILE);

    char SelName[NM];
    int FileAttr;
    SrcPanel->GetSelName(NULL,FileAttr);
    while (SrcPanel->GetSelName(SelName,FileAttr))
    {
      if (strcmp(SelName,"..")==0 || (FileAttr & FA_DIREC))
        continue;
      int Success=FALSE;

      char TempDir[NM],TempName[NM];
      *TempName=0;

      FILE *SrcFile=NULL;
      if (PluginMode)
      {
        strcpy(TempDir,Opt.TempPath);
        strcat(TempDir,"FarTmpXXXXXX");
        if (mktemp(TempDir)!=NULL)
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
              RemoveDirectory(TempDir);
          }
        }
      }
      else
        SrcFile=fopen(SelName,"rb");

      if (SrcFile!=NULL)
      {
        DOC_INFO_1 di1;
        char AnsiName[NM];
        OemToChar(SelName,AnsiName);
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
        if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MPrintTitle),MSG(MCannotPrint),
                    SelName,MSG(MSkip),MSG(MCancel))!=0)
          break;
      }
    }
    ClosePrinter(hPrinter);
  }
  SrcPanel->Redraw();
  /* $ 13.07.2000 SVS
     использовалась new[]
  */
  delete[] pi;
  /* SVS $ */
}


static void AddToPrintersMenu(VMenu *PrinterList,PRINTER_INFO_2 *pi,
                              int PrinterNumber)
{
  for (int I=0;I<PrinterNumber;I++)
  {
    char MenuText[200],PrinterName[200];
    struct MenuItem ListItem;
    ListItem.Checked=ListItem.Separator=0;
    CharToOem(NullToEmpty(pi[I].pPrinterName),PrinterName);
    if (pi[I].pComment!=NULL)
      CharToOem(pi[I].pComment,pi[I].pComment);
    sprintf(MenuText,"%-22.22s │ %-10s %3d %s  %s",PrinterName,
            NullToEmpty(pi[I].pPortName),pi[I].cJobs,MSG(MJobs),
            NullToEmpty(pi[I].pComment));
    strncpy(ListItem.Name,MenuText,sizeof(ListItem.Name));
    strncpy(ListItem.UserData,NullToEmpty(pi[I].pPrinterName),sizeof(ListItem.UserData));
    ListItem.UserDataSize=strlen(ListItem.UserData)+1;
    if ((pi[I].Attributes & PRINTER_ATTRIBUTE_DEFAULT) && !DefaultPrinterFound)
    {
      DefaultPrinterFound=TRUE;
      ListItem.Selected=TRUE;
    }
    else
      ListItem.Selected=FALSE;
    PrinterList->AddItem(&ListItem);
  }
}

