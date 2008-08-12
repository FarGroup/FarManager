/*
print.cpp

ѕечать (Alt-F5)
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
  Message(0,0,UMSG(MPrintTitle),UMSG(MPreparingForPrinting));
}

void PrintFiles(Panel *SrcPanel)
{
  _ALGO(CleverSysLog clv(L"Alt-F5 (PrintFiles)"));
  string strPrinterName;
  DWORD Needed,Returned;
  int PrinterNumber;
  DWORD FileAttr;
  string strSelName;

  long DirsCount=0;
  int SelCount=SrcPanel->GetSelCount();

  if (SelCount==0)
  {
    _ALGO(SysLog(L"Error: SelCount==0"));
    return;
  }

  // проверка каталогов
  _ALGO(SysLog(L"Check for FILE_ATTRIBUTE_DIRECTORY"));
  SrcPanel->GetSelName(NULL,FileAttr);
  while (SrcPanel->GetSelName(&strSelName,FileAttr))
  {
    if (TestParentFolderName(strSelName) || (FileAttr & FILE_ATTRIBUTE_DIRECTORY))
      DirsCount++;
  }

  if (DirsCount==SelCount)
    return;

  DefaultPrinterFound=FALSE;

  const int pi_count=1024;
  PRINTER_INFO_2W *pi=new PRINTER_INFO_2W[pi_count];

  if (pi==NULL || !EnumPrintersW(PRINTER_ENUM_LOCAL,NULL,2,(LPBYTE)pi,pi_count*sizeof(PRINTER_INFO_2W),&Needed,&Returned))
  {
    delete[] pi;
    return;
  }

  {
    _ALGO(CleverSysLog clv2(L"Show Menu"));
    string strTitle;
    string strName;

    if (SelCount==1)
    {
      SrcPanel->GetSelName(NULL,FileAttr);
      SrcPanel->GetSelName(&strName,FileAttr);
      TruncStr(strName,50);
      strSelName.Format (L"\"%s\"", (const wchar_t*)strName);
      strTitle.Format (UMSG(MPrintTo), (const wchar_t*)strSelName);
    }
    else
    {
      _ALGO(SysLog(L"Correct: SelCount-=DirsCount"));
      SelCount-=DirsCount;
      strTitle.Format (UMSG(MPrintFilesTo),SelCount);
    }

    VMenu PrinterList(strTitle,NULL,0,ScrY-4);
    PrinterList.SetFlags(VMENU_WRAPMODE|VMENU_SHOWAMPERSAND);
    PrinterList.SetPosition(-1,-1,0,0);

    AddToPrintersMenu(&PrinterList,pi,Returned);

    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
      DWORD NetReturned;
      if (EnumPrintersW(PRINTER_ENUM_CONNECTIONS,NULL,2,(LPBYTE)pi,pi_count*sizeof(PRINTER_INFO_2W),&Needed,&NetReturned))
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
      _ALGO(SysLog(L"ESC"));
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
    Message(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MPrintTitle),UMSG(MCannotOpenPrinter),
            strPrinterName,UMSG(MOk));
    delete[] pi;
    _ALGO(SysLog(L"Error: Cannot Open Printer"));
    return;
  }

  {
    _ALGO(CleverSysLog clv3(L"Print selected Files"));
    //SaveScreen SaveScr;
    SetCursorType(FALSE,0);

    SetPreRedrawFunc(PR_PrintMsg);
    PR_PrintMsg();

    HANDLE hPlugin=SrcPanel->GetPluginHandle();
    int PluginMode=SrcPanel->GetMode()==PLUGIN_PANEL &&
        !CtrlObject->Plugins.UseFarCommand(hPlugin,PLUGIN_FARGETFILE);

    SrcPanel->GetSelName(NULL,FileAttr);
    while (SrcPanel->GetSelName(&strSelName,FileAttr))
    {
      if (TestParentFolderName(strSelName) || (FileAttr & FILE_ATTRIBUTE_DIRECTORY))
        continue;
      int Success=FALSE;

      FILE *SrcFile=NULL;
      string strTempDir, strTempName;

      if (PluginMode)
      {
        if (FarMkTempEx(strTempDir))
        {
          CreateDirectoryW(strTempDir,NULL);
          struct FileListItem ListItem;
          if (SrcPanel->GetLastSelectedItem(&ListItem))
          {
            struct PluginPanelItem PanelItem;
            FileList::FileListToPluginItem(&ListItem,&PanelItem);
            if (CtrlObject->Plugins.GetFile(hPlugin,&PanelItem,strTempDir,strTempName,OPM_SILENT))
              SrcFile=_wfopen(strTempName,L"rb");
            else
              apiRemoveDirectory(strTempDir);
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

        if (StartDocPrinterW(hPrinter,1,(LPBYTE)&di1))
        {
          char Buffer[8192];
          DWORD Read,Written;
          Success=TRUE;
          while ((Read=(DWORD)fread(Buffer,1,sizeof(Buffer),SrcFile))>0)
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
        DeleteFileWithFolder(strTempName);
      }

      if (Success)
        SrcPanel->ClearLastGetSelection();
      else
      {
        SetPreRedrawFunc(NULL); //??
        if (Message(MSG_WARNING|MSG_ERRORTYPE,2,UMSG(MPrintTitle),UMSG(MCannotPrint),
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

    strPrinterName = pi[I].pPrinterName;

    strMenuText.Format (L"%-22.22s %c %-10s %3d %s  %s", (const wchar_t*)strPrinterName,BoxSymbols[0x0B3-0x0B0],
            NullToEmpty(pi[I].pPortName),pi[I].cJobs,UMSG(MJobs),
            NullToEmpty(pi[I].pComment));

    ListItem.strName = strMenuText;
    if ((pi[I].Attributes & PRINTER_ATTRIBUTE_DEFAULT) && !DefaultPrinterFound)
    {
      DefaultPrinterFound=TRUE;
      ListItem.SetSelect(TRUE);
    }
    else
      ListItem.SetSelect(FALSE);
    IDItem=PrinterList->AddItem(&ListItem);
    // ј вот теперь добавим данные дл€ этого пункта (0 - передаем строку)
    PrinterList->SetUserData((void *)NullToEmpty(pi[I].pPrinterName),0,IDItem);
  }
}
