/*
print.cpp

ѕечать (Alt-F5)

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
#include "TPreRedrawFunc.hpp"

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
		delete[] pi;
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
		TPreRedrawFuncGuard preRedrawFuncGuard(PR_PrintMsg);
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

			if (*TempName)
				DeleteFileWithFolder(TempName);

			if (Success)
				SrcPanel->ClearLastGetSelection();
			else
			{
				//preRedrawFunc.Set(NULL); //??
				if (Message(MSG_WARNING|MSG_ERRORTYPE,2,MSG(MPrintTitle),MSG(MCannotPrint),
				            SelName,MSG(MSkip),MSG(MCancel))!=0)
					break;
			}
		}

		ClosePrinter(hPrinter);
	}

	SrcPanel->Redraw();
	delete[] pi;
}


static void AddToPrintersMenu(VMenu *PrinterList,PRINTER_INFO_2 *pi,
                              int PrinterNumber)
{
	int IDItem;

	for (int I=0; I<PrinterNumber; I++)
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
		// ј вот теперь добавим данные дл€ этого пункта (0 - передаем строку)
		PrinterList->SetUserData((void *)NullToEmpty(pi[I].pPrinterName),0,IDItem);
	}
}
