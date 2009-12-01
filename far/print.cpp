/*
print.cpp

Печать (Alt-F5)
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

#include "lang.hpp"
#include "panel.hpp"
#include "vmenu.hpp"
#include "filelist.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "print.hpp"
#include "delete.hpp"
#include "pathmix.hpp"
#include "mix.hpp"

#define PRINTER_INFO_LEVEL 4
#define GENERATE_PRINTER_INFO(prefix, value, suffix) prefix##value##suffix
#define PRINTER_INFO_X(level) GENERATE_PRINTER_INFO(PRINTER_INFO_, level, W)
#define PRINTER_INFO PRINTER_INFO_X(PRINTER_INFO_LEVEL)

static void AddToPrintersMenu(VMenu *PrinterList, PRINTER_INFO *pi, int PrinterNumber)
{
	// Получаем принтер по умолчанию
	string strDefaultPrinter;
	DWORD pcchBuffer = 0;

	if (!GetDefaultPrinter(NULL, &pcchBuffer) && ERROR_INSUFFICIENT_BUFFER==GetLastError())
	{
		if (!GetDefaultPrinter(strDefaultPrinter.GetBuffer(pcchBuffer), &pcchBuffer))
			strDefaultPrinter.ReleaseBuffer(0);
		else
			strDefaultPrinter.ReleaseBuffer();
	}

	// Элемент меню
	MenuItemEx Item;
	// Признак наличия принтера по умолчанию
	bool bDefaultPrinterFound = false;

	// Заполняем список принтеров
	for (int i=0; i<PrinterNumber; i++)
	{
		PRINTER_INFO *printer = &pi[i];
		Item.Clear();
		Item.strName = printer->pPrinterName;

		if (!StrCmp(printer->pPrinterName, strDefaultPrinter))
		{
			bDefaultPrinterFound = true;
			Item.SetCheck(TRUE);
			Item.SetSelect(TRUE);
		}

		PrinterList->SetUserData(printer->pPrinterName,0,PrinterList->AddItem(&Item));
	}

	if (!bDefaultPrinterFound)
		PrinterList->SetSelectPos(0, 1);
}

static void PR_PrintMsg()
{
	Message(0,0,MSG(MPrintTitle),MSG(MPreparingForPrinting));
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

	PRINTER_INFO *pi = NULL;

	if (EnumPrinters(PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS,NULL,PRINTER_INFO_LEVEL,NULL,0,&Needed,&Returned) || Needed<=0)
		return;

	pi = (PRINTER_INFO *)xf_malloc(Needed);

	if (!EnumPrinters(PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS,NULL,PRINTER_INFO_LEVEL,(LPBYTE)pi,Needed,&Needed,&Returned))
	{
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MPrintTitle),MSG(MCannotEnumeratePrinters),MSG(MOk));
		xf_free(pi);
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
			strSelName=strName;
			InsertQuote(strSelName);
			strTitle.Format(MSG(MPrintTo), (const wchar_t*)strSelName);
		}
		else
		{
			_ALGO(SysLog(L"Correct: SelCount-=DirsCount"));
			SelCount-=DirsCount;
			strTitle.Format(MSG(MPrintFilesTo),SelCount);
		}

		VMenu PrinterList(strTitle,NULL,0,ScrY-4);
		PrinterList.SetFlags(VMENU_WRAPMODE|VMENU_SHOWAMPERSAND);
		PrinterList.SetPosition(-1,-1,0,0);
		AddToPrintersMenu(&PrinterList,pi,Returned);
		PrinterList.Process();
		PrinterNumber=PrinterList.Modal::GetExitCode();

		if (PrinterNumber<0)
		{
			xf_free(pi);
			_ALGO(SysLog(L"ESC"));
			return;
		}

		int nSize = PrinterList.GetUserDataSize();
		wchar_t *PrinterName = strPrinterName.GetBuffer(nSize);
		PrinterList.GetUserData(PrinterName, nSize);
		strPrinterName.ReleaseBuffer();
	}

	HANDLE hPrinter;

	if (!OpenPrinter((wchar_t*)(const wchar_t*)strPrinterName,&hPrinter,NULL))
	{
		Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MPrintTitle),MSG(MCannotOpenPrinter),
		        strPrinterName,MSG(MOk));
		xf_free(pi);
		_ALGO(SysLog(L"Error: Cannot Open Printer"));
		return;
	}

	{
		_ALGO(CleverSysLog clv3(L"Print selected Files"));
		//SaveScreen SaveScr;
		TPreRedrawFuncGuard preRedrawFuncGuard(PR_PrintMsg);
		SetCursorType(FALSE,0);
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
					apiCreateDirectory(strTempDir,NULL);
					FileListItem ListItem;

					if (SrcPanel->GetLastSelectedItem(&ListItem))
					{
						PluginPanelItem PanelItem;
						FileList::FileListToPluginItem(&ListItem,&PanelItem);

						if (CtrlObject->Plugins.GetFile(hPlugin,&PanelItem,strTempDir,strTempName,OPM_SILENT))
							SrcFile=_wfopen(strTempName,L"rb");
						else
							apiRemoveDirectory(strTempDir);

						FileList::FreePluginPanelItem(&PanelItem);
					}
				}
			}
			else
				SrcFile=_wfopen(strSelName, L"rb");

			if (SrcFile!=NULL)
			{
				DOC_INFO_1 di1;
				di1.pDocName=(wchar_t*)(const wchar_t*)strSelName;
				di1.pOutputFile=NULL;
				di1.pDatatype=NULL;

				if (StartDocPrinter(hPrinter,1,(LPBYTE)&di1))
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

			if (!strTempName.IsEmpty())
			{
				DeleteFileWithFolder(strTempName);
			}

			if (Success)
				SrcPanel->ClearLastGetSelection();
			else
			{
				if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MPrintTitle),MSG(MCannotPrint),
				            strSelName,MSG(MSkip),MSG(MCancel))!=0)
					break;
			}
		}

		ClosePrinter(hPrinter);
	}

	SrcPanel->Redraw();
	xf_free(pi);
}
