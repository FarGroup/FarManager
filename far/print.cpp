/*
print.cpp

������ (Alt-F5)

*/

/* Revision: 1.23 17.03.2006 $ */

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

static void AddToPrintersMenu(VMenu *PrinterList,PRINTER_INFO_2W *pi,
                              int PrinterNumber);

static int DefaultPrinterFound;

static void PR_PrintMsg(void)
{
  MessageW(0,0,UMSG(MPrintTitle),UMSG(MPreparingForPrinting));
}

void PrintFiles(Panel *SrcPanel)
{
  _ALGO(CleverSysLog clv("Alt-F5 (PrintFiles)"));
  string strPrinterName;
  DWORD Needed,Returned;
  int PrinterNumber;
  int FileAttr;
  string strSelName;

  long DirsCount=0;
  int SelCount=SrcPanel->GetSelCount();

  if (SelCount==0)
  {
    _ALGO(SysLog("Error: SelCount==0"));
    return;
  }

  // �������� ���������
  _ALGO(SysLog("Check for FA_DIREC"));
  SrcPanel->GetSelNameW(NULL,FileAttr);
  while (SrcPanel->GetSelNameW(&strSelName,FileAttr))
  {
    if (TestParentFolderNameW(strSelName) || (FileAttr & FA_DIREC))
      DirsCount++;
  }

  if (DirsCount==SelCount)
    return;

  DefaultPrinterFound=FALSE;

  const int pi_count=1024;
  PRINTER_INFO_2W *pi=new PRINTER_INFO_2W[pi_count];

  if (pi==NULL || !EnumPrinters(PRINTER_ENUM_LOCAL,NULL,2,(LPBYTE)pi,pi_count*sizeof(PRINTER_INFO_2),&Needed,&Returned))
  {
    /* $ 13.07.2000 SVS
       �������������� new[]
    */
    delete[] pi;
    /* SVS $ */
    return;
  }

  {
    _ALGO(CleverSysLog clv2("Show Menu"));
    string strTitle;
    string strName;

    if (SelCount==1)
    {
      SrcPanel->GetSelNameW(NULL,FileAttr);
      SrcPanel->GetSelNameW(&strName,FileAttr);
      TruncStrW(strName,50);
      strSelName.Format (L"\"%s\"", (const wchar_t*)strName);
      strTitle.Format (UMSG(MPrintTo), (const wchar_t*)strSelName);
    }
    else
    {
      _ALGO(SysLog("Correct: SelCount-=DirsCount"));
      SelCount-=DirsCount;
      strTitle.Format (UMSG(MPrintFilesTo),SelCount);
    }

    VMenu PrinterList(strTitle,NULL,0, TRUE, ScrY-4);
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

    int nSize = PrinterList.GetUserDataSize ();

    wchar_t *PrinterName = strPrinterName.GetBuffer (nSize);

    PrinterList.GetUserData(PrinterName, nSize);

    strPrinterName.ReleaseBuffer ();
  }

  HANDLE hPrinter;
  if (!OpenPrinterW((wchar_t*)(const wchar_t*)strPrinterName,&hPrinter,NULL))
  {
    MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MPrintTitle),UMSG(MCannotOpenPrinter),
            strPrinterName,UMSG(MOk));
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

    SrcPanel->GetSelNameW(NULL,FileAttr);
    while (SrcPanel->GetSelNameW(&strSelName,FileAttr))
    {
      if (TestParentFolderNameW(strSelName) || (FileAttr & FA_DIREC))
        continue;
      int Success=FALSE;

      FILE *SrcFile;
      string strTempDir, strTempName;

      if (PluginMode)
      {
        if (FarMkTempExW(strTempDir))
        {
          CreateDirectoryW(strTempDir,NULL);
          struct FileListItem ListItem;
          if (SrcPanel->GetLastSelectedItem(&ListItem))
          {
            struct PluginPanelItemW PanelItem;
            FileList::FileListToPluginItem(&ListItem,&PanelItem);
            if (CtrlObject->Plugins.GetFile(hPlugin,&PanelItem,strTempDir,strTempName,OPM_SILENT))
              SrcFile=_wfopen(strTempName,L"rb");
            else
              FAR_RemoveDirectoryW(strTempDir);
          }
        }
      }
      else
        SrcFile=_wfopen(strSelName, L"rb");

      if (SrcFile!=NULL)
      {
        DOC_INFO_1W di1;

        di1.pDocName=(wchar_t*)(const wchar_t*)strSelName;
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
      if ( !strTempName.IsEmpty() )
      {
        DeleteFileWithFolderW(strTempName);
      }

      if (Success)
        SrcPanel->ClearLastGetSelection();
      else
      {
        SetPreRedrawFunc(NULL); //??
        if (MessageW(MSG_WARNING|MSG_ERRORTYPE,2,UMSG(MPrintTitle),UMSG(MCannotPrint),
                    strSelName,UMSG(MSkip),UMSG(MCancel))!=0)
          break;
      }
    }
    ClosePrinter(hPrinter);
    SetPreRedrawFunc(NULL);
  }
  SrcPanel->Redraw();
  delete[] pi;
}


static void AddToPrintersMenu(VMenu *PrinterList,PRINTER_INFO_2W *pi,
                              int PrinterNumber)
{
  int IDItem;
  for (int I=0;I<PrinterNumber;I++)
  {
    string strMenuText, strPrinterName;
    MenuItemEx ListItem;
    ListItem.Clear ();

    strPrinterName = NullToEmptyW(pi[I].pPrinterName);

    strMenuText.Format (L"%-22.22s %c %-10s %3d %s  %s", (const wchar_t*)strPrinterName,0x0B3U,
            NullToEmptyW(pi[I].pPortName),pi[I].cJobs,UMSG(MJobs),
            NullToEmptyW(pi[I].pComment));

    ListItem.strName = strMenuText;
    if ((pi[I].Attributes & PRINTER_ATTRIBUTE_DEFAULT) && !DefaultPrinterFound)
    {
      DefaultPrinterFound=TRUE;
      ListItem.SetSelect(TRUE);
    }
    else
      ListItem.SetSelect(FALSE);
    IDItem=PrinterList->AddItemW(&ListItem);
    // � ��� ������ ������� ������ ��� ����� ������ (0 - �������� ������)
    PrinterList->SetUserData((void *)NullToEmptyW(pi[I].pPrinterName),0,IDItem);
  }
}
