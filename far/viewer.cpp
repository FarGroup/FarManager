/*
viewer.cpp

Internal viewer
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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

#include "viewer.hpp"
#include "codepage.hpp"
#include "macroopcode.hpp"
#include "keyboard.hpp"
#include "flink.hpp"
#include "lang.hpp"
#include "colors.hpp"
#include "keys.hpp"
#include "help.hpp"
#include "dialog.hpp"
#include "panel.hpp"
#include "filepanels.hpp"
#include "fileview.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "TaskBar.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "delete.hpp"
#include "dirmix.hpp"
#include "pathmix.hpp"
#include "filestr.hpp"
#include "mix.hpp"
#include "constitle.hpp"
#include "console.hpp"
#include "wakeful.hpp"

static void PR_ViewerSearchMsg();
static void ViewerSearchMsg(const wchar_t *Name,int Percent);

static int InitHex=FALSE,SearchHex=FALSE;

static int ViewerID=0;

static int utf8_to_WideChar(const char *s, int nc, wchar_t *w1,wchar_t *w2, int wlen, int &tail);

#define REPLACE_CHAR 0xFFFD  // Replacement
#define CONTINUE_CHAR 0x203A // Single Right-Pointing Angle Quotation Mark

Viewer::Viewer(bool bQuickView, UINT aCodePage):
	ViOpt(Opt.ViOpt),
	Reader(ViewFile, 0, 0),
	m_bQuickView(bQuickView)
{
	_OT(SysLog(L"[%p] Viewer::Viewer()", this));

	for (int i=0; i<=MAXSCRY; i++)
	{
		Strings[i] = new ViewerString();
		Strings[i]->lpData = new wchar_t[MAX_VIEWLINEB];
	}

	strLastSearchStr = strGlobalSearchString;
	LastSearchCase=GlobalSearchCase;
	LastSearchRegexp=Opt.ViOpt.SearchRegexp;
	LastSearchWholeWords=GlobalSearchWholeWords;
	LastSearchReverse=GlobalSearchReverse;
	LastSearchHex=GlobalSearchHex;
	VM.CodePage=DefCodePage=aCodePage;
	// Вспомним тип врапа
	VM.Wrap=Opt.ViOpt.ViewerIsWrap;
	VM.WordWrap=Opt.ViOpt.ViewerWrap;
	VM.Hex=InitHex;
	ViewKeyBar=nullptr;
	FilePos=0;
	LeftPos=0;
	SecondPos=0;
	FileSize=0;
	LastPage=0;
	SelectPos=SelectSize=0;
	LastSelPos=0;
	SetStatusMode(TRUE);
	HideCursor=TRUE;
	DeleteFolder=TRUE;
	CodePageChangedByUser=FALSE;
	ReadStdin=FALSE;
	BMSavePos.Clear();
	memset(UndoData,0xff,sizeof(UndoData));
	LastKeyUndo=FALSE;
	InternalKey=FALSE;
	this->ViewerID=::ViewerID++;
	CtrlObject->Plugins.CurViewer=this;
	OpenFailed=false;
	HostFileViewer=nullptr;
	SelectPosOffSet=0;
	bVE_READ_Sent = false;
	Signature = false;

	vgetc_ready = false;
	vgetc_cb = vgetc_ib = 0;
	vgetc_composite = L'\0';

	vread_buffer = new char[vread_buffer_size = 8192];
	Up_buffer = new wchar_t[Up_buffer_size = 2 * (3*MAX_VIEWLINE + 2)];

	memset(&vString, 0, sizeof(vString));
	vString.lpData = new wchar_t[MAX_VIEWLINEB];
}


Viewer::~Viewer()
{
	KeepInitParameters();

	if (ViewFile.Opened())
	{
		ViewFile.Close();

		if (Opt.ViOpt.SavePos)
		{
			string strCacheName=strPluginData.IsEmpty()?strFullFileName:strPluginData+PointToName(strFileName);
			UINT CodePage=0;

			if (CodePageChangedByUser)
			{
				CodePage=VM.CodePage;
			}

			ViewerPosCache poscache;
			poscache.FilePos=FilePos;
			poscache.LeftPos=LeftPos;
			poscache.Hex=VM.Hex;
			poscache.CodePage=CodePage;
			poscache.bm=BMSavePos;

			FilePositionCache::AddPosition(strCacheName,poscache);
		}
	}

	delete[] vString.lpData;
	delete[] Up_buffer;
	delete[] vread_buffer;

	_tran(SysLog(L"[%p] Viewer::~Viewer, TempViewName=[%s]",this,TempViewName));
	/* $ 11.10.2001 IS
	   Удаляем файл только, если нет открытых фреймов с таким именем.
	*/

	if (!strTempViewName.IsEmpty() && !FrameManager->CountFramesWithName(strTempViewName))
	{
		/* $ 14.06.2002 IS
		   Если DeleteFolder сброшен, то удаляем только файл. Иначе - удаляем еще
		   и каталог.
		*/
		if (DeleteFolder)
			DeleteFileWithFolder(strTempViewName);
		else
		{
			apiSetFileAttributes(strTempViewName,FILE_ATTRIBUTE_NORMAL);
			apiDeleteFile(strTempViewName); //BUGBUG
		}
	}

	for (int i=0; i<=MAXSCRY; i++)
	{
		delete [] Strings[i]->lpData;
		delete Strings[i];
	}

	if (!OpenFailed && bVE_READ_Sent)
	{
		CtrlObject->Plugins.CurViewer=this; //HostFileViewer;
		CtrlObject->Plugins.ProcessViewerEvent(VE_CLOSE,&ViewerID);
	}
}


void Viewer::KeepInitParameters()
{
	strGlobalSearchString = strLastSearchStr;
	GlobalSearchCase=LastSearchCase;
	GlobalSearchWholeWords=LastSearchWholeWords;
	GlobalSearchReverse=LastSearchReverse;
	GlobalSearchHex=LastSearchHex;
	Opt.ViOpt.ViewerIsWrap=VM.Wrap;
	Opt.ViOpt.ViewerWrap=VM.WordWrap;
	Opt.ViOpt.SearchRegexp=LastSearchRegexp;
	InitHex=VM.Hex;
}


int Viewer::OpenFile(const wchar_t *Name,int warning)
{
	VM.CodePage=DefCodePage;
	DefCodePage=CP_AUTODETECT;
	OpenFailed=false;

	ViewFile.Close();
	Reader.Clear();

	SelectSize = 0; // Сбросим выделение
	strFileName = Name;

	if (Opt.OnlyEditorViewerUsed && !StrCmp(strFileName, L"-"))
	{
		string strTempName;

		if (!FarMkTempEx(strTempName))
		{
			OpenFailed=TRUE;
			return FALSE;
		}

		if (!ViewFile.Open(strTempName,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE))
		{
			OpenFailed=true;
			return FALSE;
		}

		DWORD ReadSize,WrittenSize;

		while (ReadFile(Console.GetInputHandle(),vread_buffer,(DWORD)vread_buffer_size,&ReadSize,nullptr))
		{
			ViewFile.Write(vread_buffer,ReadSize,WrittenSize);
		}
		ViewFile.SetPointer(0, nullptr, FILE_BEGIN);

		//after reading from the pipe, redirect stdin to the real console stdin
		//CONIN$ must be opened with the exact flags and name as below so apiCreateFile() is not good
		SetStdHandle(STD_INPUT_HANDLE,CreateFile(L"CONIN$",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr));
		ReadStdin=TRUE;
	}
	else
	{
		ViewFile.Open(strFileName, FILE_READ_DATA, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, OPEN_EXISTING);
	}

	if (!ViewFile.Opened())
	{
		/* $ 04.07.2000 tran
		   + 'warning' flag processing, in QuickView it is FALSE
		     so don't show red message box */
		if (warning)
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MViewerTitle),
			        MSG(MViewerCannotOpenFile),strFileName,MSG(MOk));

		OpenFailed=true;
		return FALSE;
	}
	Reader.Init();

	CodePageChangedByUser=FALSE;

	ConvertNameToFull(strFileName,strFullFileName);
	apiGetFindDataEx(strFileName, ViewFindData);
	UINT CachedCodePage=0;

	if (Opt.ViOpt.SavePos && !ReadStdin)
	{
		__int64 NewLeftPos,NewFilePos;
		string strCacheName=strPluginData.IsEmpty()?strFileName:strPluginData+PointToName(strFileName);
		ViewerPosCache poscache;

		FilePositionCache::GetPosition(strCacheName,poscache);
		NewFilePos=poscache.FilePos;
		NewLeftPos=poscache.LeftPos;
		VM.Hex=poscache.Hex;
		CachedCodePage=poscache.CodePage;
		BMSavePos=poscache.bm;

		// Проверяем поддерживается или нет загруженная из кэша кодовая страница
		if (CachedCodePage && !IsCodePageSupported(CachedCodePage))
			CachedCodePage = 0;
		LastSelPos=FilePos=NewFilePos;
		LeftPos=NewLeftPos;
	}
	else
	{
		FilePos=0;
	}

	/* $ 26.07.2002 IS
	     Автоопределение Unicode не должно зависеть от опции
	     "Автоопределение таблицы символов", т.к. Unicode не есть
	     _таблица символов_ для перекодировки.
	*/
	//if(ViOpt.AutoDetectTable)
	{
		bool Detect=false;
		UINT CodePage=0;

		if (VM.CodePage == CP_AUTODETECT || IsUnicodeOrUtfCodePage(VM.CodePage))
		{
			Detect=GetFileFormat(ViewFile,CodePage,&Signature,Opt.ViOpt.AutoDetectCodePage!=0);

			// Проверяем поддерживается или нет задетектированная кодовая страница
			if (Detect)
				Detect = IsCodePageSupported(CodePage);
		}

		if (VM.CodePage==CP_AUTODETECT)
		{
			if (Detect)
			{
				VM.CodePage=CodePage;
			}

			if (CachedCodePage)
			{
				VM.CodePage=CachedCodePage;
				CodePageChangedByUser=TRUE;
			}

			if (VM.CodePage==CP_AUTODETECT)
				VM.CodePage=Opt.ViOpt.AnsiCodePageAsDefault?GetACP():GetOEMCP();
		}
		else
		{
			CodePageChangedByUser=TRUE;
		}

		if (!IsUnicodeOrUtfCodePage(VM.CodePage))
		{
			ViewFile.SetPointer(0, nullptr, FILE_BEGIN);
		}
	}
	SetFileSize();

	if (FilePos>FileSize)
		FilePos=0;

//  if (ViOpt.AutoDetectTable && !TableChangedByUser)
//  {
//  }
	ChangeViewKeyBar();
	AdjustWidth();
	CtrlObject->Plugins.CurViewer=this; // HostFileViewer;
	/* $ 15.09.2001 tran
	   пора легализироваться */
	CtrlObject->Plugins.ProcessViewerEvent(VE_READ,nullptr);
	bVE_READ_Sent = true;

	last_update_check = GetTickCount();
	string strRoot;
	GetPathRoot(strFullFileName, strRoot);
	int DriveType = FAR_GetDriveType(strRoot);
	if (IsDriveTypeCDROM(DriveType))
		DriveType = DRIVE_CDROM;
	switch (DriveType) //??? make it configurable
	{
		case DRIVE_REMOVABLE: update_check_period = -1;   break; // flash drive or floppy: never
		case DRIVE_FIXED:     update_check_period = +1;   break; // hard disk: 1msec
		case DRIVE_REMOTE:    update_check_period = 1000; break; // network drive: 1sec
		case DRIVE_CDROM:     update_check_period = -1;   break; // cd/dvd: never
		case DRIVE_RAMDISK:   update_check_period = +1;   break; // ramdrive: 1msec
		default:              update_check_period = -1;   break; // unknown: never
	}

	return TRUE;
}

/* $ 27.04.2001 DJ
   функция вычисления ширины в зависимости от наличия скроллбара
*/
void Viewer::AdjustWidth()
{
	Width=X2-X1+1;
	XX2=X2;

	if (ViOpt.ShowScrollbar && !m_bQuickView)
	{
		Width--;
		XX2--;
	}
}

void Viewer::ShowPage(int nMode)
{
	int I,Y;
	AdjustWidth();

	if (!ViewFile.Opened())
	{
		if (!strFileName.IsEmpty() && ((nMode == SHOW_RELOAD) || (nMode == SHOW_HEX)))
		{
			SetScreen(X1,Y1,X2,Y2,L' ',COL_VIEWERTEXT);
			GotoXY(X1,Y1);
			SetColor(COL_WARNDIALOGTEXT);
			FS<<fmt::Precision(XX2-X1+1)<<MSG(MViewerCannotOpenFile);
			ShowStatus();
		}

		return;
	}

	if (HideCursor)
		SetCursorType(0,10);

	vseek(FilePos,SEEK_SET);

	if (!SelectSize)
		SelectPos=FilePos;

	switch (nMode)
	{
		case SHOW_HEX:
			CtrlObject->Plugins.CurViewer = this; //HostFileViewer;
			ShowHex();
			break;
		case SHOW_RELOAD:
			CtrlObject->Plugins.CurViewer = this; //HostFileViewer;

			for (I=0,Y=Y1; Y<=Y2; Y++,I++)
			{
				Strings[I]->nFilePos = vtell();

				if (Y==Y1+1 && !veof())
					SecondPos=vtell();

				ReadString(Strings[I], -1);
			}
			break;
		case SHOW_UP:
			if (Y2 > Y1)
			{
				ViewerString *tmp = Strings[Y2-Y1];
				memmove(&Strings[1], &Strings[0], sizeof(Strings[0]) * (Y2-Y1));
				Strings[0] = tmp;
			}
			Strings[0]->nFilePos = FilePos;
			SecondPos = Strings[1]->nFilePos;
			ReadString(Strings[0],(int)(SecondPos-FilePos));
			break;
		case SHOW_DOWN:
			if (Y2 > Y1)
			{
				ViewerString *tmp = Strings[0];
				memmove(&Strings[0], &Strings[1], sizeof(Strings[0]) * (Y2-Y1));
				Strings[Y2-Y1] = tmp;
			}
			FilePos = Strings[0]->nFilePos;
			SecondPos = Strings[1]->nFilePos;
			Strings[Y2-Y1]->nFilePos = Strings[Y2-Y1-1]->nFilePos + Strings[Y2-Y1-1]->nbytes;
			vseek(Strings[Y2-Y1]->nFilePos, SEEK_SET);
			ReadString(Strings[Y2-Y1],-1);
			break;
	}

	if (nMode != SHOW_HEX)
	{
		for (I=0,Y=Y1; Y<=Y2; Y++,I++)
		{
			int StrLen = StrLength(Strings[I]->lpData);
			SetColor(COL_VIEWERTEXT);
			GotoXY(X1,Y);

			if (StrLen > LeftPos)
			{
				if (IsUnicodeOrUtfCodePage(VM.CodePage) && Signature && !I && !Strings[I]->nFilePos)
				{
					FS<<fmt::LeftAlign()<<fmt::Width(Width)<<fmt::Precision(Width)<<&Strings[I]->lpData[static_cast<size_t>(LeftPos+1)];
				}
				else
				{
					FS<<fmt::LeftAlign()<<fmt::Width(Width)<<fmt::Precision(Width)<<&Strings[I]->lpData[static_cast<size_t>(LeftPos)];
				}
			}
			else
			{
				FS<<fmt::Width(Width)<<L"";
			}

			if (SelectSize && Strings[I]->bSelection)
			{
				__int64 SelX1;

				if (LeftPos > Strings[I]->nSelStart)
					SelX1 = X1;
				else
					SelX1 = Strings[I]->nSelStart-LeftPos;

				if (!VM.Wrap && (Strings[I]->nSelStart < LeftPos || Strings[I]->nSelStart > LeftPos+XX2-X1))
				{
					if (AdjustSelPosition)
					{
						LeftPos = Strings[I]->nSelStart-1;
						AdjustSelPosition = FALSE;
						Show();
						return;
					}
				}
				else
				{
					SetColor(COL_VIEWERSELECTEDTEXT);
					GotoXY(static_cast<int>(X1+SelX1),Y);
					__int64 Length = Strings[I]->nSelEnd-Strings[I]->nSelStart;

					if (LeftPos > Strings[I]->nSelStart)
						Length = Strings[I]->nSelEnd-LeftPos;

					if (LeftPos > Strings[I]->nSelEnd)
						Length = 0;

					FS<<fmt::Precision(static_cast<int>(Length))<<&Strings[I]->lpData[static_cast<size_t>(SelX1+LeftPos+SelectPosOffSet)];
				}
			}

			if (StrLen > LeftPos + Width && ViOpt.ShowArrows)
			{
				GotoXY(XX2,Y);
				SetColor(COL_VIEWERARROWS);
				BoxText(0xbb);
			}

			if (LeftPos>0 && *Strings[I]->lpData  && ViOpt.ShowArrows)
			{
				GotoXY(X1,Y);
				SetColor(COL_VIEWERARROWS);
				BoxText(0xab);
			}
		}
	}

	DrawScrollbar();
	ShowStatus();
}

void Viewer::DisplayObject()
{
	ShowPage(VM.Hex?SHOW_HEX:SHOW_RELOAD);
}

void Viewer::ShowHex()
{
	wchar_t OutStr[128],TextStr[20];
	int EndFile;
	__int64 SelSize;
	int X,Y,TextPos;
	int SelStart, SelEnd;
	bool bSelStartFound = false, bSelEndFound = false;
	__int64 HexLeftPos=((LeftPos>80-ObjWidth) ? Max(80-ObjWidth,0):LeftPos);

	for (EndFile=0,Y=Y1; Y<=Y2; Y++)
	{
		bSelStartFound = false;
		bSelEndFound = false;
		SelSize=0;
		SetColor(COL_VIEWERTEXT);
		GotoXY(X1,Y);

		if (EndFile)
		{
			FS<<fmt::Width(ObjWidth)<<L"";
			continue;
		}

		if (Y==Y1+1 && !veof())
			SecondPos=vtell();

		INT64 Ptr=ViewFile.GetPointer();
		_snwprintf(OutStr,ARRAYSIZE(OutStr),L"%010I64X: ", Ptr);
		TextPos=0;
		int HexStrStart = (int)wcslen(OutStr);
		SelStart = HexStrStart;
		SelEnd = SelStart;
		__int64 fpos = vtell();

		if (fpos > SelectPos)
			bSelStartFound = true;

		if (fpos < SelectPos+SelectSize-1)
			bSelEndFound = true;

		if (!SelectSize)
			bSelStartFound = bSelEndFound = false;

		const wchar_t BorderLine[]={BoxSymbols[BS_V1],L' ',0};

		int out_len = (int)wcslen(OutStr);
		int border_len = (int)wcslen(BorderLine);

		if (IsUnicodeCodePage(VM.CodePage))
		{
			wchar_t line[8];
			int nr = vread(line, 8);
			LastPage = EndFile = veof() ? 1 : 0;
			if (nr <= 0)
			{
				*OutStr = L'\0';
			}
			else
			{
				for (X=0; X<8; X++)
				{
					if (SelectSize>0 && (SelectPos == fpos))
					{
						bSelStartFound = true;
						SelStart = (int)wcslen(OutStr);
						SelSize=SelectSize;
					}
					if (SelectSize>0 && (fpos == (SelectPos+SelectSize-1)))
					{
						bSelEndFound = true;
						SelEnd = (int)wcslen(OutStr)+3;
						SelSize=SelectSize;
					}

					if (X < nr)
					{
						unsigned char b1 = HIBYTE(line[X]), b2 = LOBYTE(line[X]);
#if 0																 // !!! поведение изменено !!!
						if (VM.CodePage != CP_REVERSEBOM) // мне кажется в обоих случаях
						{                                 // логичнее показывать неперевернутые коды
							unsigned char t = b1; b1 = b2; b2 = t;
						}
#endif
						_snwprintf(OutStr+out_len, ARRAYSIZE(OutStr)-out_len, L"%02X%02X ", b1, b2);
						TextStr[TextPos++] = line[X] ? line[X] : L' ';
					}
					else
					{
						wcscpy(OutStr+out_len, L"     ");
						TextStr[TextPos++] = L' ';
					}
					out_len += 5;

					if (X == 3)
					{
						wcscpy(OutStr+out_len, BorderLine);
						out_len += border_len;
					}
					++fpos;
				}
			}
		}
		else
		{
			unsigned char line[16+3];
			DWORD nr = 0;
			DWORD nb = CP_UTF8 == VM.CodePage ? 16+3 : 16;
			Reader.Read(line, nb, &nr);
			if (nr > 16)
				Reader.Unread(nr-16);

			LastPage = EndFile = (nr <= nb) && veof() ? 1 : 0;

			if (!nr)
			{
				*OutStr = L'\0';
			}
			else
			{
				if (SelectSize)
				{
					if (SelectPos >= fpos && SelectPos < fpos+16)
					{
						int off = (int)(SelectPos - fpos);
						bSelStartFound = true;
						SelStart = out_len + 3*off + (off < 8 ? 0 : border_len);
						SelSize = SelectSize;
					}
					__int64 selectEnd = SelectPos + SelectSize - 1;
					if (selectEnd >= fpos && selectEnd < fpos+16)
					{
						int off = (int)(selectEnd - fpos);
						bSelEndFound = true;
						SelEnd = out_len + 3*off + (off < 8 ? 0 : border_len) + 1;
						SelSize = SelectSize;
					}
				}
				for (X=0; X<16; X++)
				{
					int off = out_len + 3*X + (X < 8 ? 0 : border_len);
					if (X == 8)
						wcscpy(OutStr+off-border_len, BorderLine);
					if (X < (int)nr)
						_snwprintf(OutStr+off, ARRAYSIZE(OutStr)-off, L"%02X ", (int)line[X]);
					else
						wcscpy(OutStr+off, L"   ");
					TextStr[X] = L' ';
					if (CP_UTF8 != VM.CodePage && X < (int)nr && line[X])
						MultiByteToWideChar(VM.CodePage, 0, (LPCSTR)line+X, 1, TextStr+X, 1);
				}

				if (CP_UTF8 == VM.CodePage)
				{
					wchar_t w1[16], w2[16];
					int tail, nw, ib=0, iw=0;
					nw = utf8_to_WideChar((char *)line, (int)nr, w1, w2, 16, tail);
					bool first = true;
					while (ib < 16 && iw < nw)
					{
						if (first && w1[iw] == REPLACE_CHAR && w2[iw] == L'?')
						{
							TextStr[ib++] = CONTINUE_CHAR; // это может быть не совсем корректно для 'плохих' utf-8
						}                                 // но усложнять из-за этого код на мой взгяд не стоит...
						else
						{
							first = false;
							TextStr[ib++] = w1[iw] ? w1[iw] : L' ';
						}
						int clen = WideCharToMultiByte(CP_UTF8, 0, w2+iw, 1, nullptr, 0, nullptr, nullptr);
						while (--clen > 0 && ib < 16)
							TextStr[ib++] = CONTINUE_CHAR;
						++iw;
					}
				}
				TextPos = 16;
			}
		}

		TextStr[TextPos]=0;
		wcscat(TextStr,L" ");

		if ((SelEnd <= SelStart) && bSelStartFound)
			SelEnd = (int)wcslen(OutStr)-2;

		wcscat(OutStr,L" ");
		wcscat(OutStr,TextStr);

		if (StrLength(OutStr)>HexLeftPos)
		{
			FS<<fmt::LeftAlign()<<fmt::Width(ObjWidth)<<fmt::Precision(ObjWidth)<<OutStr+static_cast<size_t>(HexLeftPos);
		}
		else
		{
			FS<<fmt::Width(ObjWidth)<<L"";
		}

		if (bSelStartFound && bSelEndFound)
		{
			SetColor(COL_VIEWERSELECTEDTEXT);
			GotoXY((int)((__int64)X1+SelStart-HexLeftPos),Y);
			FS<<fmt::Precision(SelEnd-SelStart+1)<<OutStr+static_cast<size_t>(SelStart);
			SelSize = 0;
		}
	}
}

/* $ 27.04.2001 DJ
   отрисовка скроллбара - в отдельную функцию
*/

void Viewer::DrawScrollbar()
{
	if (ViOpt.ShowScrollbar)
	{
		if (m_bQuickView)
			SetColor(COL_PANELSCROLLBAR);
		else
			SetColor(COL_VIEWERSCROLLBAR);

		if (!VM.Hex)
		{
			ScrollBar(X2+(m_bQuickView?1:0),Y1,Y2-Y1+1,(LastPage ? (!FilePos?0:100):ToPercent64(FilePos,FileSize)),100);
		}
		else
		{
			UINT64 Total=FileSize/16+((FileSize%16)?1:0);
			UINT64 Top=FilePos/16+((FilePos%16)?1:0);
ScrollBarEx(X2+(m_bQuickView?1:0),Y1,Y2-Y1+1,LastPage?Top?Total:0:Top,Total);
		}
	}
}


string &Viewer::GetTitle(string &strName,int,int)
{
	if (!strTitle.IsEmpty())
	{
		strName = strTitle;
	}
	else
	{
		if (!IsAbsolutePath(strFileName))
		{
			string strPath;
			ViewNamesList.GetCurDir(strPath);
			AddEndSlash(strPath);
			strName = strPath+strFileName;
		}
		else
		{
			strName = strFileName;
		}
	}

	return strName;
}

void Viewer::ShowStatus()
{
	if (HostFileViewer)
		HostFileViewer->ShowStatus();
}


void Viewer::SetStatusMode(int Mode)
{
	ShowStatusLine=Mode;
}


static inline bool is_space_or_nul( const wchar_t ch )
{
	return L'\0' == ch || IsSpace(ch);
}

static inline int wrapped_char( const wchar_t ch )
{
	static const wchar_t wrapped_chars[] = L",;>)"; // word-wrap enabled after it

	if (is_space_or_nul(ch))
		return +0;
	else if (nullptr != wcschr(wrapped_chars, ch))
		return +1;
	else
		return -1;
}

void Viewer::ReadString(ViewerString *pString, int MaxSize)
{
	AdjustWidth();

	int OutPtr = 0, nTab = 0, wrap_out = -1;
	wchar_t ch, eol_char = L'\0';
	__int64 fpos, sel_end, wrap_pos = -1;
	bool skip_space = false;

	if (VM.Hex)
	{
		vseek(IsUnicodeCodePage(VM.CodePage) ? 8 : 16, FILE_CURRENT);
		pString->lpData[OutPtr] = L'\0';
		LastPage = veof();
		return;
	}

	bool bSelStartFound = false, bSelEndFound = false;
	pString->bSelection = false;
	sel_end = SelectPos + SelectSize;
	fpos = vtell();

	for (;;)
	{
		if (OutPtr >= MAX_VIEWLINE)
			break;

		if (--nTab >= 0)
			ch = L' ';
		else
		{
			fpos = vtell();
			if (!MaxSize-- || !vgetc(&ch))
				break;
		}

		if (SelectSize > 0)
		{
			if (fpos == SelectPos)
			{
				pString->nSelStart = OutPtr;
				bSelStartFound = true;
			}
			if (fpos == sel_end)
			{
				pString->nSelEnd = OutPtr;
				bSelEndFound = true;
			}
		}

		if (L'\t' == ch)
		{
			nTab = ViOpt.TabSize - (OutPtr % ViOpt.TabSize);
			continue;
		}
		if (L'\n' == ch || L'\r' == ch)
		{
			eol_char = ch;
			break;
		}

		pString->lpData[OutPtr++] = ch ? ch : L' ';
		if (!VM.Wrap)
			continue;

		if (OutPtr >= Width)
		{
			if (VM.WordWrap && wrap_out > 0)
			{
				OutPtr = wrap_out;
				vseek(wrap_pos, SEEK_SET);
				while (OutPtr > 0 && is_space_or_nul(pString->lpData[OutPtr-1]))
					--OutPtr;
				skip_space = true;
			}
			break;
		}

		if (!VM.WordWrap)
			continue;

		if (wrapped_char(ch) >= 0)
		{
			wrap_out = OutPtr;
			wrap_pos = vtell();
		}
	}

	if (skip_space || eol_char != L'\n') // skip spaces and/or eol-s if required
	{
		for (;;)
		{
			vgetc(nullptr);
			int ib = vgetc_ib;
			if (!vgetc(&ch))
				break;

			if (skip_space && is_space_or_nul(ch))
				continue;

			if (!eol_char && ch == L'\r')
			{
				eol_char = ch;
				continue;
			}

			if (ch != L'\n')
				vgetc_ib = ib; // ungetc()

			eol_char = ch;
			break;
		}
	}

	pString->have_eol = (eol_char != L'\0');
	pString->lpData[(int)OutPtr]=0;
	pString->nbytes = (int)(vtell() - pString->nFilePos);

	if (SelectSize > 0 && OutPtr > 0)
	{
		if (!bSelStartFound && pString->nFilePos >= SelectPos && pString->nFilePos <= sel_end)
		{
			bSelStartFound = true;
			pString->nSelStart = 0;
		}
		if (bSelStartFound && !bSelEndFound)
		{
			bSelEndFound = true;
			pString->nSelEnd = OutPtr;
		}
		if (bSelEndFound && pString->nSelEnd > OutPtr)
			pString->nSelEnd = OutPtr;

		pString->bSelection = bSelStartFound && bSelEndFound;
	}

	if (!eol_char && veof())
		LastPage = 1;
}

__int64 Viewer::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return (__int64)!FileSize;
		case MCODE_C_SELECTED:
			return (__int64)(SelectSize?TRUE:FALSE);
		case MCODE_C_EOF:
			return (__int64)(LastPage || !ViewFile.Opened());
		case MCODE_C_BOF:
			return (__int64)(!FilePos || !ViewFile.Opened());
		case MCODE_V_VIEWERSTATE:
		{
			DWORD MacroViewerState=0;
			MacroViewerState|=VM.Wrap?0x00000008:0;
			MacroViewerState|=VM.WordWrap?0x00000010:0;
			MacroViewerState|=VM.Hex?0x00000020:0;
			MacroViewerState|=Opt.OnlyEditorViewerUsed?0x08000000|0x00000800:0;
			MacroViewerState|=HostFileViewer && !HostFileViewer->GetCanLoseFocus()?0x00000800:0;
			return (__int64)MacroViewerState;
		}
		case MCODE_V_ITEMCOUNT: // ItemCount - число элементов в текущем объекте
			return (__int64)GetViewFileSize();
		case MCODE_V_CURPOS: // CurPos - текущий индекс в текущем объекте
			return (__int64)(GetViewFilePos()+1);
	}

	return 0;
}

/* $ 28.01.2001
   - Путем проверки ViewFile на nullptr избавляемся от падения
*/
int Viewer::ProcessKey(int Key)
{
	/* $ 22.01.2001 IS
	     Происходят какие-то манипуляции -> снимем выделение
	*/
	if (!ViOpt.PersistentBlocks &&
	        Key!=KEY_IDLE && Key!=KEY_NONE && !(Key==KEY_CTRLINS||Key==KEY_CTRLNUMPAD0) && Key!=KEY_CTRLC)
		SelectSize=0;

	if (!InternalKey && !LastKeyUndo && (FilePos!=UndoData[0].UndoAddr || LeftPos!=UndoData[0].UndoLeft))
	{
		for (int i=ARRAYSIZE(UndoData)-1; i>0; i--)
		{
			UndoData[i].UndoAddr=UndoData[i-1].UndoAddr;
			UndoData[i].UndoLeft=UndoData[i-1].UndoLeft;
		}

		UndoData[0].UndoAddr=FilePos;
		UndoData[0].UndoLeft=LeftPos;
	}

	if (Key!=KEY_ALTBS && Key!=KEY_CTRLZ && Key!=KEY_NONE && Key!=KEY_IDLE)
		LastKeyUndo=FALSE;

	if (Key>=KEY_CTRL0 && Key<=KEY_CTRL9)
	{
		int Pos=Key-KEY_CTRL0;

		if (BMSavePos.FilePos[Pos]!=POS_NONE)
		{
			FilePos=BMSavePos.FilePos[Pos];
			LeftPos=BMSavePos.LeftPos[Pos];
//      LastSelPos=FilePos;
			Show();
		}

		return TRUE;
	}

	if (Key>=KEY_CTRLSHIFT0 && Key<=KEY_CTRLSHIFT9)
		Key=Key-KEY_CTRLSHIFT0+KEY_RCTRL0;

	if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9)
	{
		int Pos=Key-KEY_RCTRL0;
		BMSavePos.FilePos[Pos]=FilePos;
		BMSavePos.LeftPos[Pos]=LeftPos;
		return TRUE;
	}

	switch (Key)
	{
		case KEY_F1:
		{
			Help Hlp(L"Viewer");
			return TRUE;
		}
		case KEY_CTRLU:
		{
//      if (SelectSize)
			{
				SelectSize = 0;
				Show();
			}
			return TRUE;
		}
		case KEY_CTRLC:
		case KEY_CTRLINS:  case KEY_CTRLNUMPAD0:
		{
			if (SelectSize && ViewFile.Opened())
			{
				wchar_t *SelData;
				size_t DataSize = (size_t)SelectSize+(IsUnicodeCodePage(VM.CodePage)?sizeof(wchar_t):1);
				__int64 CurFilePos=vtell();

				if ((SelData=(wchar_t*)xf_malloc(DataSize*sizeof(wchar_t))) )
				{
					wmemset(SelData, 0, DataSize);
					vseek(SelectPos,SEEK_SET);
					vread(SelData, (int)SelectSize);
					CopyToClipboard(SelData);
					xf_free(SelData);
					vseek(CurFilePos,SEEK_SET);
				}
			}

			return TRUE;
		}
		//   включить/выключить скролбар
		case KEY_CTRLS:
		{
			ViOpt.ShowScrollbar=!ViOpt.ShowScrollbar;
			Opt.ViOpt.ShowScrollbar=ViOpt.ShowScrollbar;

			if (m_bQuickView)
				CtrlObject->Cp()->ActivePanel->Redraw();

			Show();
			return (TRUE);
		}
		case KEY_IDLE:
		{
			if (ViewFile.Opened() && update_check_period >= 0)
			{
				DWORD now_ticks = GetTickCount();
				if ((int)(now_ticks - last_update_check) < update_check_period)
					return TRUE;
				last_update_check = now_ticks;

				FAR_FIND_DATA_EX NewViewFindData;
				if (!apiGetFindDataEx(strFullFileName, NewViewFindData))
					return TRUE;

				if (ViewFindData.ftLastWriteTime.dwLowDateTime!=NewViewFindData.ftLastWriteTime.dwLowDateTime
				 || ViewFindData.ftLastWriteTime.dwHighDateTime!=NewViewFindData.ftLastWriteTime.dwHighDateTime
				 || ViewFindData.nFileSize != NewViewFindData.nFileSize)
				{
					ViewFindData = NewViewFindData;
					SetFileSize();

					Reader.Clear(); // иначе зачем вся эта возня?
					ViewFile.FlushBuffers();

					if (FilePos>FileSize)
					{
						ProcessKey(KEY_CTRLEND);
					}
					else
					{
						__int64 PrevLastPage=LastPage;
						Show();

						if (PrevLastPage && !LastPage)
						{
							ProcessKey(KEY_CTRLEND);
							LastPage=TRUE;
						}
					}
				}
			}

			if (Opt.ViewerEditorClock && HostFileViewer && HostFileViewer->IsFullScreen() && Opt.ViOpt.ShowTitleBar)
				ShowTime(FALSE);

			return TRUE;
		}
		case KEY_ALTBS:
		case KEY_CTRLZ:
		{
			for (size_t I=1; I<ARRAYSIZE(UndoData); I++)
			{
				UndoData[I-1].UndoAddr=UndoData[I].UndoAddr;
				UndoData[I-1].UndoLeft=UndoData[I].UndoLeft;
			}

			if (UndoData[0].UndoAddr!=-1)
			{
				FilePos=UndoData[0].UndoAddr;
				LeftPos=UndoData[0].UndoLeft;
				UndoData[ARRAYSIZE(UndoData)-1].UndoAddr=-1;
				UndoData[ARRAYSIZE(UndoData)-1].UndoLeft=-1;
				Show();
			}

			return TRUE;
		}
		case KEY_ADD:
		case KEY_SUBTRACT:
		{
			if (strTempViewName.IsEmpty())
			{
				string strName;
				string strShortName;
				bool NextFileFound;

				if (Key==KEY_ADD)
					NextFileFound=ViewNamesList.GetNextName(strName, strShortName);
				else
					NextFileFound=ViewNamesList.GetPrevName(strName, strShortName);

				if (NextFileFound)
				{
					if (Opt.ViOpt.SavePos)
					{
						string strCacheName=strPluginData.IsEmpty()?strFileName:strPluginData+PointToName(strFileName);
						UINT CodePage=0;

						if (CodePageChangedByUser)
							CodePage=VM.CodePage;

						{
							ViewerPosCache poscache;
							poscache.FilePos=FilePos;
							poscache.LeftPos=LeftPos;
							poscache.Hex=VM.Hex;
							poscache.CodePage=CodePage;
							poscache.bm=BMSavePos;

							FilePositionCache::AddPosition(strCacheName,poscache);

							BMSavePos.Clear(); //Preapare for new file loading
						}
					}

					if (PointToName(strName) == strName)
					{
						string strViewDir;
						ViewNamesList.GetCurDir(strViewDir);

						if (!strViewDir.IsEmpty())
							FarChDir(strViewDir);
					}

					if (OpenFile(strName, TRUE))
					{
						SecondPos=0;
						Show();
					}

					ShowConsoleTitle();
				}
			}

			return TRUE;
		}
		case KEY_SHIFTF2:
		{
			LastPage = 0;
			ProcessTypeWrapMode(!VM.WordWrap);
			return TRUE;
		}
		case KEY_F2:
		{
			LastPage = 0;
			ProcessWrapMode(!VM.Wrap);
			return TRUE;
		}
		case KEY_F4:
		{
			LastPage = 0;
			ProcessHexMode(!VM.Hex);
			return TRUE;
		}
		case KEY_F7:
		{
			Search(0,0);
			return TRUE;
		}
		case KEY_SHIFTF7:
		case KEY_SPACE:
		{
			Search(1,0);
			return TRUE;
		}
		case KEY_ALTF7:
		{
			SearchFlags.Set(REVERSE_SEARCH);
			Search(1,0);
			SearchFlags.Clear(REVERSE_SEARCH);
			return TRUE;
		}
		case KEY_F8:
		{
			LastPage = 0;
			if (IsUnicodeCodePage(VM.CodePage))
			{
				FilePos*=2;
				SetFileSize();
				SelectPos = 0;
				SelectSize = 0;
			}

			VM.CodePage = VM.CodePage==GetOEMCP() ? GetACP() : GetOEMCP();
			ChangeViewKeyBar();
			Show();
			CodePageChangedByUser=TRUE;
			return TRUE;
		}
		case KEY_SHIFTF8:
		{
			LastPage = 0;
			UINT nCodePage = SelectCodePage(VM.CodePage, true, true, false, true);

			if (nCodePage!=(UINT)-1)
			{
				if (nCodePage == (WORD)(CP_AUTODETECT & 0xffff))
				{
					__int64 fpos = vtell();
					bool detect = GetFileFormat(ViewFile,nCodePage,&Signature,true) && IsCodePageSupported(nCodePage);
					vseek(fpos, SEEK_SET);
					if (!detect)
						nCodePage = Opt.ViOpt.AnsiCodePageAsDefault ? GetACP() : GetOEMCP();
				}
				CodePageChangedByUser=TRUE;

				if (IsUnicodeCodePage(VM.CodePage) && !IsUnicodeCodePage(nCodePage))
				{
					FilePos*=2;
					SelectPos = 0;
					SelectSize = 0;
				}
				else if (!IsUnicodeCodePage(VM.CodePage) && IsUnicodeCodePage(nCodePage))
				{
					FilePos=(FilePos+(FilePos&1))/2; //????
					SelectPos = 0;
					SelectSize = 0;
				}

				VM.CodePage=nCodePage;
				SetFileSize();
				ChangeViewKeyBar();
				Show();
			}

			return TRUE;
		}
		case KEY_ALTF8:
		{
			if (ViewFile.Opened())
			{
				LastPage = 0;
				GoTo();
			}

			return TRUE;
		}
		case KEY_F11:
		{
			CtrlObject->Plugins.CommandsMenu(MODALTYPE_VIEWER,0,L"Viewer");
			Show();
			return TRUE;
		}
		/* $ 27.06.2001 VVM
		  + С альтом скролим по 1 */
		case KEY_MSWHEEL_UP:
		case(KEY_MSWHEEL_UP | KEY_ALT):
		{
			int Roll = Key & KEY_ALT?1:Opt.MsWheelDeltaView;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_UP);

			return TRUE;
		}
		case KEY_MSWHEEL_DOWN:
		case(KEY_MSWHEEL_DOWN | KEY_ALT):
		{
			int Roll = Key & KEY_ALT?1:Opt.MsWheelDeltaView;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_DOWN);

			return TRUE;
		}
		case KEY_MSWHEEL_LEFT:
		case(KEY_MSWHEEL_LEFT | KEY_ALT):
		{
			int Roll = Key & KEY_ALT?1:Opt.MsHWheelDeltaView;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_LEFT);

			return TRUE;
		}
		case KEY_MSWHEEL_RIGHT:
		case(KEY_MSWHEEL_RIGHT | KEY_ALT):
		{
			int Roll = Key & KEY_ALT?1:Opt.MsHWheelDeltaView;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_RIGHT);

			return TRUE;
		}
		case KEY_UP: case KEY_NUMPAD8: case KEY_SHIFTNUMPAD8:
		{
			if (FilePos>0 && ViewFile.Opened())
			{
				Up();	// LastPage = 0

				if (VM.Hex)
				{
					//FilePos&=~(IsUnicodeCodePage(VM.CodePage) ? 0x7:0xf);
					Show();
				}
				else
				{
					ShowPage(SHOW_UP);
					ViewerString *end = Strings[Y2-Y1];
					LastPage = end->nFilePos >= FileSize ||
					   (!end->have_eol && end->nFilePos + end->nbytes >= FileSize);
				}
			}

			return TRUE;
		}
		case KEY_DOWN: case KEY_NUMPAD2:  case KEY_SHIFTNUMPAD2:
		{
			if (!LastPage && ViewFile.Opened())
			{
				if (VM.Hex)
				{
					FilePos=SecondPos;
					Show();
				}
				else
					ShowPage(SHOW_DOWN);
			}

			return TRUE;
		}
		case KEY_PGUP: case KEY_NUMPAD9: case KEY_SHIFTNUMPAD9: case KEY_CTRLUP:
		{
			if (ViewFile.Opened())
			{
				for (int i=Y1; i<Y2; i++)
					Up();

				Show();
			}

			return TRUE;
		}
		case KEY_PGDN: case KEY_NUMPAD3:  case KEY_SHIFTNUMPAD3: case KEY_CTRLDOWN:
		{
			if (LastPage || !ViewFile.Opened())
				return TRUE;

			if (VM.Hex)
				FilePos += (IsUnicodeCodePage(VM.CodePage) ? 8 : 16) * (Y2-Y1);
			else
				FilePos = Strings[Y2-Y1]->nFilePos;

			if (Key == KEY_CTRLDOWN)
			{
				vseek(FilePos, SEEK_SET);
				for (int i=Y1; i<=Y2; i++)
				ReadString(&vString,-1);

				if (LastPage)
			{
				InternalKey++;
				ProcessKey(KEY_CTRLPGDN);
				InternalKey--;
				return TRUE;
			}
			}

			Show();
			return TRUE;
		}
		case KEY_LEFT: case KEY_NUMPAD4: case KEY_SHIFTNUMPAD4:
		{
			if (LeftPos>0 && ViewFile.Opened())
			{
				if (VM.Hex && LeftPos>80-Width)
					LeftPos=Max(80-Width,1);

				LeftPos--;
				Show();
			}

			return TRUE;
		}
		case KEY_RIGHT: case KEY_NUMPAD6: case KEY_SHIFTNUMPAD6:
		{
			if (LeftPos<MAX_VIEWLINE && ViewFile.Opened() && !VM.Hex && !VM.Wrap)
			{
				LeftPos++;
				Show();
			}

			return TRUE;
		}
		case KEY_CTRLLEFT: case KEY_CTRLNUMPAD4:
		{
			if (ViewFile.Opened())
			{
				if (VM.Hex)
				{
					FilePos--;

					if (FilePos<0)
						FilePos=0;
				}
				else
				{
					LeftPos-=20;

					if (LeftPos<0)
						LeftPos=0;
				}

				Show();
			}

			return TRUE;
		}
		case KEY_CTRLRIGHT: case KEY_CTRLNUMPAD6:
		{
			if (ViewFile.Opened())
			{
				if (VM.Hex)
				{
					FilePos++;

					if (FilePos >= FileSize)
						FilePos=FileSize-1; //??
				}
				else if (!VM.Wrap)
				{
					LeftPos+=20;

					if (LeftPos>MAX_VIEWLINE)
						LeftPos=MAX_VIEWLINE;
				}

				Show();
			}

			return TRUE;
		}
		case KEY_CTRLSHIFTLEFT:    case KEY_CTRLSHIFTNUMPAD4:

			// Перейти на начало строк
			if (ViewFile.Opened())
			{
				LeftPos = 0;
				Show();
			}

			return TRUE;
		case KEY_CTRLSHIFTRIGHT:     case KEY_CTRLSHIFTNUMPAD6:
		{
			// Перейти на конец строк
			if (ViewFile.Opened())
			{
				int I, Y, Len, MaxLen = 0;

				for (I=0,Y=Y1; Y<=Y2; Y++,I++)
				{
					Len = StrLength(Strings[I]->lpData);

					if (Len > MaxLen)
						MaxLen = Len;
				} /* for */

				if (MaxLen > Width)
					LeftPos = MaxLen - Width;
				else
					LeftPos = 0;

				Show();
			} /* if */

			return TRUE;
		}
		case KEY_CTRLHOME:    case KEY_CTRLNUMPAD7:
		case KEY_HOME:        case KEY_NUMPAD7:   case KEY_SHIFTNUMPAD7:

			// Перейти на начало файла
			if (ViewFile.Opened())
				LeftPos=0;

		case KEY_CTRLPGUP:    case KEY_CTRLNUMPAD9:
			if (ViewFile.Opened())
			{
				FilePos=0;
				LastPage = 0;
				Show();
			}

			return TRUE;
		case KEY_CTRLEND:     case KEY_CTRLNUMPAD1:
		case KEY_END:         case KEY_NUMPAD1: case KEY_SHIFTNUMPAD1:

			// Перейти на конец файла
			if (ViewFile.Opened())
				LeftPos=0;

		case KEY_CTRLPGDN:    case KEY_CTRLNUMPAD3:

			if (ViewFile.Opened())
			{
				/* $ 15.08.2002 IS
				   Для обычного режима, если последняя строка не содержит перевод
				   строки, крутанем вверх на один раз больше - иначе визуально
				   обработка End (и подобных) на такой строке отличается от обработки
				   Down.
				*/
				unsigned int max_counter=Y2-Y1;

				if (VM.Hex)
				{
					vseek(0,SEEK_END);
					FilePos = vtell();
					int line_chars = IsUnicodeCodePage(VM.CodePage) ? 8 : 16;
					if ((FilePos % line_chars) == 0)
						FilePos -= line_chars * (Y2 - Y1 + 1);
					else
						FilePos -= (FilePos % line_chars) + line_chars * (Y2 - Y1);
					if (FilePos < 0)
						FilePos = 0;
				}
				else
				{
					vseek(-1,SEEK_END);
					WCHAR LastSym=0;

					if (vgetc(&LastSym) && LastSym!=L'\n' && LastSym!=L'\r')
						++max_counter;

					FilePos=vtell();

					for (int i=0; static_cast<unsigned int>(i)<max_counter; i++)
						Up();
				}

				Show();
			}

			return TRUE;
		default:

			if (Key>=L' ' && Key<0x10000)
			{
				Search(0,Key);
				return TRUE;
			}
	}

	return FALSE;
}

int Viewer::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!(MouseEvent->dwButtonState & 3))
		return FALSE;

	/* $ 22.01.2001 IS
	     Происходят какие-то манипуляции -> снимем выделение
	*/
//  SelectSize=0;

	/* $ 10.09.2000 SVS
	   ! Постоянный скроллинг при нажатой клавише
	     Обыкновенный захват мыши
	*/
	/* $ 02.10.2000 SVS
	  > Если нажать в самом низу скролбара, вьюер отмотается на страницу
	  > ниже нижней границы текста. Перед глазами будет пустой экран.
	*/
	if (ViOpt.ShowScrollbar && IntKeyState.MouseX==X2+(m_bQuickView?1:0))
	{
		/* $ 01.09.2000 SVS
		   Небольшая бага с тыканием в верхнюю позицию ScrollBar`а
		*/
		if (IntKeyState.MouseY == Y1)
			while (IsMouseButtonPressed())
				ProcessKey(KEY_UP);
		else if (IntKeyState.MouseY==Y2)
		{
			while (IsMouseButtonPressed())
			{
//        _SVS(SysLog(L"Viewer/ KEY_DOWN= %i, %i",FilePos,FileSize));
				ProcessKey(KEY_DOWN);
			}
		}
		else if (IntKeyState.MouseY == Y1+1)
			ProcessKey(KEY_CTRLHOME);
		else if (IntKeyState.MouseY == Y2-1)
			ProcessKey(KEY_CTRLEND);
		else
		{
			while (IsMouseButtonPressed())
			{
				/* $ 14.05.2001 DJ
				   более точное позиционирование; корректная работа на больших файлах
				*/
				FilePos=(FileSize-1)/(Y2-Y1-1)*(IntKeyState.MouseY-Y1);
				int Perc;

				if (FilePos > FileSize)
				{
					FilePos=FileSize;
					Perc=100;
				}
				else if (FilePos < 0)
				{
					FilePos=0;
					Perc=0;
				}
				else
					Perc=ToPercent64(FilePos,FileSize);

//_SVS(SysLog(L"Viewer/ ToPercent()=%i, %I64d, %I64d, Mouse=[%d:%d]",Perc,FilePos,FileSize,MsX,MsY));
				if (Perc == 100)
					ProcessKey(KEY_CTRLEND);
				else if (!Perc)
					ProcessKey(KEY_CTRLHOME);
				else
				{
					/* $ 27.04.2001 DJ
					   не рвем строки посередине
					*/
					AdjustFilePos();
					Show();
				}
			}
		}

		return (TRUE);
	}

	/* $ 16.12.2000 tran
	   шелчок мышью на статус баре */

	/* $ 12.10.2001 SKV
	  угу, а только если он нсть, statusline...
	*/
	if (IntKeyState.MouseY == (Y1-1) && (HostFileViewer && HostFileViewer->IsTitleBarVisible()))  // Status line
	{
		int XCodePage, XPos, NameLength;
		NameLength=ObjWidth-40;

		if (Opt.ViewerEditorClock && HostFileViewer && HostFileViewer->IsFullScreen())
			NameLength-=6;

		if (NameLength<20)
			NameLength=20;

		XCodePage=NameLength+1;
		XPos=NameLength+1+10+1+10+1;

		while (IsMouseButtonPressed());

		if (IntKeyState.MouseY != Y1-1)
			return TRUE;

		//_D(SysLog(L"MsX=%i, XTable=%i, XPos=%i",MsX,XTable,XPos));
		if (IntKeyState.MouseX>=XCodePage && IntKeyState.MouseX<=XCodePage+10)
		{
			ProcessKey(KEY_SHIFTF8);
			return (TRUE);
		}

		if (IntKeyState.MouseX>=XPos && IntKeyState.MouseX<=XPos+7+1+4+1+3)
		{
			ProcessKey(KEY_ALTF8);
			return (TRUE);
		}
	}

	if (IntKeyState.MouseX<X1 || IntKeyState.MouseX>X2 || IntKeyState.MouseY<Y1 || IntKeyState.MouseY>Y2)
		return FALSE;

	if (IntKeyState.MouseX<X1+7)
		while (IsMouseButtonPressed() && IntKeyState.MouseX<X1+7)
			ProcessKey(KEY_LEFT);
	else if (IntKeyState.MouseX>X2-7)
		while (IsMouseButtonPressed() && IntKeyState.MouseX>X2-7)
			ProcessKey(KEY_RIGHT);
	else if (IntKeyState.MouseY<Y1+(Y2-Y1)/2)
		while (IsMouseButtonPressed() && IntKeyState.MouseY<Y1+(Y2-Y1)/2)
			ProcessKey(KEY_UP);
	else
		while (IsMouseButtonPressed() && IntKeyState.MouseY>=Y1+(Y2-Y1)/2)
			ProcessKey(KEY_DOWN);

	return TRUE;
}

void Viewer::Up()
{
	if (!ViewFile.Opened())
		return;

	LastPage = 0;

	if (FilePos <= 0)
		return;

	if (VM.Hex)
	{
		FilePos -= IsUnicodeCodePage(VM.CodePage) ? 8 : 16;
		if (FilePos < 0)
			FilePos = 0;
		return;
	}

	int Skipped = 0, I = -1, BufSize = MAX_VIEWLINE + 2;
	wchar_t *Buf = Up_buffer, *tBuf = Up_buffer, *tBuf2 = nullptr;

	if (!VM.Hex && VM.CodePage == CP_UTF8)
	{
		BufSize = Up_buffer_size / 2; // = 3*MAXLINE + 2;
		tBuf = tBuf2 = Up_buffer + BufSize;
	}

	if (FilePos < (__int64)BufSize)
		BufSize = (int)FilePos;

	if (BufSize >= 512) // first try to find newline in first 256 bytes
	{
		vseek(FilePos-256, SEEK_SET);
		int nr = vread(Buf, 256, tBuf2);

		if (nr > 0 && Buf[nr-1] == L'\n')
		{
			--nr; ++Skipped;
		}
		if (nr > 0 && Buf[nr-1] == L'\r')
		{
			--nr; ++Skipped;
		}

		for (I = nr-1; I >= 0; I--)
		{
			if (Buf[I] == L'\n' || Buf[I] == L'\r')
			{
				BufSize = nr;
				break;
			}
		}
	}

	if (I < 0) // full portion read
	{
		vseek(FilePos - BufSize, SEEK_SET);
		BufSize = vread(Buf, BufSize, tBuf2);
		Skipped=0;

		if (BufSize > 0 && Buf[BufSize-1]==L'\n')
		{
			--BufSize; ++Skipped;
		}
		if (BufSize > 0 && Buf[BufSize-1] == L'\r')
		{
			--BufSize; ++Skipped;
		}

		int I0 = BufSize > MAX_VIEWLINE ? BufSize - MAX_VIEWLINE : 0;
		for (I = BufSize-1; I >= I0; I--)
		{
			if (Buf[I] == L'\n' || Buf[I] == L'\r')
				break;
		}
	}

	if (!VM.Wrap)
	{
		FilePos -= GetStrBytesNum(tBuf + (I+1), BufSize-(I+1)) + Skipped;
	}
	else
	{
		wchar_t *line = tBuf + I+1;
		int len, line_size = BufSize - (I+1);
		for (;;)
		{
			len = WrappedLength(line, line_size);
			if ( len >= line_size )
				break;
			line += len;
			line_size -= len;
		}
		FilePos -= GetStrBytesNum(line, line_size + Skipped);
	}
	return;
}

int Viewer::WrappedLength(wchar_t *str, int line_size)
{
	int width = 0, off = 0, wrap_pos = -1, wrap_ch, tab_size = ViOpt.TabSize;
	bool wwrap = VM.Wrap && VM.WordWrap;

	for (;;)
	{
		if (off >= line_size)
			return off;

		wchar_t ch = str[off++];
		width += L'\t' != ch ? 1 : tab_size - width % tab_size;

		if (width >= Width)
		{
			if (!wwrap || wrap_pos <= 0)
				return off;
			while (wrap_pos < line_size && is_space_or_nul(str[wrap_pos]))
				++wrap_pos;
			return wrap_pos;
		}

		if (!wwrap)
			continue;

		if ((wrap_ch = wrapped_char(ch)) >= 0)
			wrap_pos = off - 1 + wrap_ch;
	}
}

int Viewer::GetStrBytesNum(const wchar_t *Str, int Length)
{
	if (IsUnicodeCodePage(VM.CodePage))
		return Length;
	else
		return WideCharToMultiByte(VM.CodePage, 0, Str, Length, nullptr, 0, nullptr, nullptr);
}

void Viewer::SetViewKeyBar(KeyBar *ViewKeyBar)
{
	Viewer::ViewKeyBar=ViewKeyBar;
	ChangeViewKeyBar();
}


void Viewer::ChangeViewKeyBar()
{
	if (ViewKeyBar)
	{
		/* $ 12.07.2000 SVS
		   Wrap имеет 3 позиции
		*/
		/* $ 15.07.2000 SVS
		   Wrap должен показываться следующий, а не текущий
		*/
		ViewKeyBar->Change(
		    MSG(
		        (!VM.Wrap)?((!VM.WordWrap)?MViewF2:MViewShiftF2)
				        :MViewF2Unwrap),1);
		ViewKeyBar->Change(KBL_SHIFT,MSG((VM.WordWrap)?MViewF2:MViewShiftF2),1);

		if (VM.Hex)
			ViewKeyBar->Change(MSG(MViewF4Text),3);
		else
			ViewKeyBar->Change(MSG(MViewF4),3);

		if (VM.CodePage != GetOEMCP())
			ViewKeyBar->Change(MSG(MViewF8DOS),7);
		else
			ViewKeyBar->Change(MSG(MViewF8),7);

		ViewKeyBar->Redraw();
	}

	CtrlObject->Plugins.CurViewer=this; //HostFileViewer;
}

enum SEARCHDLG
{
	SD_DOUBLEBOX,
	SD_TEXT_SEARCH,
	SD_EDIT_TEXT,
	SD_EDIT_HEX,
	SD_SEPARATOR1,
	SD_RADIO_TEXT,
	SD_RADIO_HEX,
	SD_CHECKBOX_CASE,
	SD_CHECKBOX_WORDS,
	SD_CHECKBOX_REVERSE,
	SD_CHECKBOX_REGEXP,
	SD_SEPARATOR2,
	SD_BUTTON_OK,
	SD_BUTTON_CANCEL,
};

enum
{
 DM_SDSETVISIBILITY = DM_USER+1,
};

INT_PTR WINAPI ViewerSearchDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	switch (Msg)
	{
		case DN_INITDIALOG:
		{
			SendDlgMessage(hDlg,DM_SDSETVISIBILITY,SendDlgMessage(hDlg,DM_GETCHECK,SD_RADIO_HEX,0) == BSTATE_CHECKED,0);
			SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,SD_EDIT_TEXT,ToPtr(1));
			SendDlgMessage(hDlg,DM_EDITUNCHANGEDFLAG,SD_EDIT_HEX,ToPtr(1));
			return TRUE;
		}
		case DM_SDSETVISIBILITY:
		{
			SendDlgMessage(hDlg,DM_SHOWITEM,SD_EDIT_TEXT,ToPtr(!Param1));
			SendDlgMessage(hDlg,DM_SHOWITEM,SD_EDIT_HEX,ToPtr(Param1));
			SendDlgMessage(hDlg,DM_ENABLE,SD_CHECKBOX_CASE,ToPtr(!Param1));
			SendDlgMessage(hDlg,DM_ENABLE,SD_CHECKBOX_WORDS,ToPtr(!Param1));
			//SendDlgMessage(hDlg,DM_ENABLE,SD_CHECKBOX_REGEXP,!Param1);
			return TRUE;
		}
		case DN_BTNCLICK:
		{
			if ((Param1 == SD_RADIO_TEXT || Param1 == SD_RADIO_HEX) && Param2)
			{
				SendDlgMessage(hDlg, DM_ENABLEREDRAW, FALSE, 0);
				Viewer *viewer = (Viewer *)SendDlgMessage(hDlg, DM_GETITEMDATA, SD_EDIT_TEXT, 0);

				int sd_dst = Param1 == SD_RADIO_HEX  ? SD_EDIT_HEX : SD_EDIT_TEXT;
				int sd_src = Param1 == SD_RADIO_TEXT ? SD_EDIT_HEX : SD_EDIT_TEXT;

				const wchar_t *ps = (const wchar_t *)SendDlgMessage(hDlg, DM_GETCONSTTEXTPTR, sd_src, 0);
				string strTo;
				viewer->SearchTextTransform(strTo, ps, Param1 == SD_RADIO_TEXT);

				SendDlgMessage(hDlg, DM_SETTEXTPTR, sd_dst, const_cast<wchar_t*>(strTo.CPtr()));
				SendDlgMessage(hDlg, DM_SDSETVISIBILITY, Param1 == SD_RADIO_HEX, 0);

				if (!strTo.IsEmpty())
				{
					int changed = (int)SendDlgMessage(hDlg, DM_EDITUNCHANGEDFLAG, sd_src, ToPtr(-1));
					SendDlgMessage(hDlg, DM_EDITUNCHANGEDFLAG, sd_dst, ToPtr(changed));
				}

				SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
				return TRUE;
			}
		}
		case DN_HOTKEY:
		{
			if (Param1==SD_TEXT_SEARCH)
			{
				SendDlgMessage(hDlg,DM_SETFOCUS,(SendDlgMessage(hDlg,DM_GETCHECK,SD_RADIO_HEX,0) == BSTATE_CHECKED)?SD_EDIT_HEX:SD_EDIT_TEXT,0);
				return FALSE;
			}
		}
		default:
			break;
	}

	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

static void PR_ViewerSearchMsg()
{
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	ViewerSearchMsg((const wchar_t*)preRedrawItem.Param.Param1,(int)(INT_PTR)preRedrawItem.Param.Param2);
}

void ViewerSearchMsg(const wchar_t *MsgStr,int Percent)
{
	string strProgress;

	if (Percent>=0)
	{
		FormatString strPercent;
		strPercent<<Percent;

		size_t PercentLength=Max(strPercent.GetLength(),(size_t)3);
		size_t Length=Max(Min(static_cast<int>(MAX_WIDTH_MESSAGE-2),StrLength(MsgStr)),40)-PercentLength-2;
		wchar_t *Progress=strProgress.GetBuffer(Length);

		if (Progress)
		{
			size_t CurPos=Min(Percent,100)*Length/100;
			wmemset(Progress,BoxSymbols[BS_X_DB],CurPos);
			wmemset(Progress+(CurPos),BoxSymbols[BS_X_B0],Length-CurPos);
			strProgress.ReleaseBuffer(Length);
			FormatString strTmp;
			strTmp<<L" "<<fmt::Width(PercentLength)<<strPercent<<L"%";
			strProgress+=strTmp;
		}

		TBC.SetProgressValue(Percent,100);
	}

	Message(0,0,MSG(MViewSearchTitle),(SearchHex?MSG(MViewSearchingHex):MSG(MViewSearchingFor)),MsgStr,strProgress.IsEmpty()?nullptr:strProgress.CPtr());
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	preRedrawItem.Param.Param1=(void*)MsgStr;
	preRedrawItem.Param.Param2=(LPVOID)(INT_PTR)Percent;
	PreRedraw.SetParam(preRedrawItem.Param);
}

static void ss2hex(string& to, const char *c1, int len, wchar_t sep = L' ')
{
	wchar_t ss[4];

	for (int i = 0; i < len; ++i)
	{
		if (sep && i > 0)
			to += sep;
		_snwprintf(ss, ARRAYSIZE(ss), L"%02X", (unsigned char)c1[i]);
		to += ss;
	}
}

static int hex2ss(const wchar_t *from, char *c1, int mb)
{
	int nb, i, v, sub = 0;
	wchar_t ch;

	nb = i = v = 0;
	for (;;)
	{
		ch = *from++;
		if      (ch >= L'0' && ch <= L'9') sub = L'0';
		else if (ch >= L'A' && ch <= L'F') sub = L'A' - 10;
		else if (ch >= L'a' && ch <= L'f') sub = L'a' - 10;
		else
		{
			if (i > 0)
			{
				c1[nb++] = (char)v;
				i = 0;
			}
			if (!ch || nb >= mb)
				break;
			else
				continue;
		}
		v = v*16 + ((int)ch - sub);
		if (++i >= 2)
		{
			c1[nb++] = (char)v;
			i = 0;
			if (nb >= mb)
				break;
		}
	}
#ifdef _DEBUG
	if (nb < mb) c1[nb] = '\0';
#endif
	return nb;
}

void Viewer::SearchTextTransform( UnicodeString &to, const wchar_t *from, bool hex2text )
{
	int nb, i, v;
	char c1[128];
	wchar_t ch, ss[ARRAYSIZE(c1)+1];

	if (hex2text)
	{
		nb = hex2ss(from, c1, ARRAYSIZE(c1));

		if (IsUnicodeCodePage(VM.CodePage))
		{
			v = CP_REVERSEBOM == VM.CodePage ? 1 : 0;
			if (nb & 1)
				c1[nb++] = '\0';

			for (i = 0; i < nb; i += 2)
			{
				ch = MAKEWORD(c1[i+v], c1[i+1-v]);
				if (!ch)
					ch = 0xffff; // encode L'\0'
				to += ch;
			}
		}
		else
		{
			int nw = MultiByteToWideChar(VM.CodePage,0, c1,nb, ss,ARRAYSIZE(ss));
			for (i=0; i < nw; ++i)
				if (!ss[i])
					ss[i] = 0xffff;
			ss[nw] = L'\0';
			to = ss;
		}
	}
	else // text2hex
	{
		while (*from)
		{
			ch = *from++;
			if (0xffff == ch)	// 0xffff - invalid unicode char
				ch = 0x0000;   // it used to transfer '\0' in zero terminated string

			switch (VM.CodePage)
			{
				case CP_UNICODE:
					nb = 2; c1[0] = (char)LOBYTE(ch); c1[1] = (char)HIBYTE(ch);
				break;
				case CP_REVERSEBOM:
					nb = 2; c1[0] = (char)HIBYTE(ch); c1[1] = (char)LOBYTE(ch);
				break;
				default:
					nb = WideCharToMultiByte(VM.CodePage,0, &ch, 1, c1, 4, nullptr, nullptr);
				break;
			}

			ss2hex(to, c1, nb, L'\0');
		}
	}
}

/* $ 27.01.2003 VVM
   + Параметр Next может принимать значения:
   0 - Новый поиск
   1 - Продолжить поиск со следующей позиции
   2 - Продолжить поиск с начала файла
*/

void Viewer::Search(int Next,int FirstChar)
{
	const wchar_t *TextHistoryName=L"SearchText";
	const wchar_t *HexMask=L"HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH ";
	FarDialogItem SearchDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,72,11,0,nullptr,nullptr,0,MSG(MViewSearchTitle)},
		{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(MViewSearchFor)},
		{DI_EDIT,5,3,70,3,0,TextHistoryName,nullptr,DIF_FOCUS|DIF_HISTORY|DIF_USELASTHISTORY,L""},
		{DI_FIXEDIT,5,3,70,3,0,nullptr,HexMask,DIF_MASKEDIT,L""},
		{DI_TEXT,3,4,0,4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_RADIOBUTTON,5,5,0,5,1,nullptr,nullptr,DIF_GROUP,MSG(MViewSearchForText)},
		{DI_RADIOBUTTON,5,6,0,6,0,nullptr,nullptr,0,MSG(MViewSearchForHex)},
		{DI_CHECKBOX,40,5,0,5,0,nullptr,nullptr,0,MSG(MViewSearchCase)},
		{DI_CHECKBOX,40,6,0,6,0,nullptr,nullptr,0,MSG(MViewSearchWholeWords)},
		{DI_CHECKBOX,40,7,0,7,0,nullptr,nullptr,0,MSG(MViewSearchReverse)},
		{DI_CHECKBOX,40,8,0,8,0,nullptr,nullptr,DIF_DISABLE,MSG(MViewSearchRegexp)},
		{DI_TEXT,3,9,0,9,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,10,0,10,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MViewSearchSearch)},
		{DI_BUTTON,0,10,0,10,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MViewSearchCancel)},
	};
	MakeDialogItemsEx(SearchDlgData,SearchDlg);
	string strSearchStr;
	char search_bytes[128];
	string strMsgStr;
	__int64 MatchPos=0;
	int SearchLength,Case,WholeWords,ReverseSearch,Match,SearchRegexp;

	if (!ViewFile.Opened() || (Next && strLastSearchStr.IsEmpty()))
		return;

	if (!strLastSearchStr.IsEmpty())
		strSearchStr = strLastSearchStr;
	else
		strSearchStr.Clear();

	SearchDlg[SD_RADIO_TEXT].Selected=!LastSearchHex;
	SearchDlg[SD_RADIO_HEX].Selected=LastSearchHex;
	SearchDlg[SD_CHECKBOX_CASE].Selected=LastSearchCase;
	SearchDlg[SD_CHECKBOX_WORDS].Selected=LastSearchWholeWords;
	SearchDlg[SD_CHECKBOX_REVERSE].Selected=LastSearchReverse;
	SearchDlg[SD_CHECKBOX_REGEXP].Selected=LastSearchRegexp;

	if (SearchFlags.Check(REVERSE_SEARCH))
		SearchDlg[SD_CHECKBOX_REVERSE].Selected=!SearchDlg[SD_CHECKBOX_REVERSE].Selected;

	if (SearchDlg[SD_RADIO_HEX].Selected)
	{
		int nb = hex2ss(strSearchStr.CPtr(), search_bytes, ARRAYSIZE(search_bytes));
		strSearchStr.Clear();
		ss2hex(strSearchStr, search_bytes, nb, L'\0');
		SearchDlg[SD_EDIT_HEX].strData = strSearchStr;
	}
	else
		SearchDlg[SD_EDIT_TEXT].strData = strSearchStr;

	SearchDlg[SD_EDIT_TEXT].UserData = this;

	if (!Next)
	{
		SearchFlags.Flags = 0;
		Dialog Dlg(SearchDlg,ARRAYSIZE(SearchDlg),ViewerSearchDlgProc);
		Dlg.SetPosition(-1,-1,76,13);
		Dlg.SetHelp(L"ViewerSearch");

		if (FirstChar)
		{
			Dlg.InitDialog();
			Dlg.Show();
			Dlg.ProcessKey(FirstChar);
		}

		Dlg.Process();

		if (Dlg.GetExitCode()!=SD_BUTTON_OK)
			return;
	}

	SearchHex=SearchDlg[SD_RADIO_HEX].Selected;
	Case=SearchDlg[SD_CHECKBOX_CASE].Selected;
	WholeWords=SearchDlg[SD_CHECKBOX_WORDS].Selected;
	ReverseSearch=SearchDlg[SD_CHECKBOX_REVERSE].Selected;
	SearchRegexp=SearchDlg[SD_CHECKBOX_REGEXP].Selected;

	bool t_utf8 = !SearchHex && VM.CodePage == CP_UTF8;

	if (SearchHex)
	{
		strSearchStr = SearchDlg[SD_EDIT_HEX].strData;
		SearchLength = hex2ss(strSearchStr.CPtr(), search_bytes, ARRAYSIZE(search_bytes));
		strSearchStr.Clear();
		ss2hex(strSearchStr, search_bytes, SearchLength, L' ');
	}
	else
	{
		strSearchStr = SearchDlg[SD_EDIT_TEXT].strData;
		SearchLength = (int)strSearchStr.GetLength();
		size_t pos = 0;
		while (strSearchStr.Pos(pos, 0xffff))
			strSearchStr.Replace(pos, 1, L'\0');
	}

	strLastSearchStr = strSearchStr;
	LastSearchHex=SearchHex;
	LastSearchCase=Case;
	LastSearchWholeWords=WholeWords;

	if (!SearchFlags.Check(REVERSE_SEARCH))
		LastSearchReverse=ReverseSearch;

	LastSearchRegexp=SearchRegexp;

	if (!SearchLength)
		return;

	{
		TPreRedrawFuncGuard preRedrawFuncGuard(PR_ViewerSearchMsg);
		//SaveScreen SaveScr;
		SetCursorType(FALSE,0);
		strMsgStr = strSearchStr;

		if (strMsgStr.GetLength()+18 > static_cast<DWORD>(ObjWidth))
			TruncStrFromEnd(strMsgStr, ObjWidth-18);

		if (!SearchHex)
			InsertQuote(strMsgStr);

		if (SearchHex)
			Case = 1 + (WholeWords = 0);

		if (!Case && !SearchHex)
			strSearchStr.Upper();

		SelectSize = 0;

		if (Next)
		{
			if (Next == 2)
			{
				SearchFlags.Set(SEARCH_MODE2);
				LastSelPos = ReverseSearch?FileSize:0;
			}
			else
			{
				LastSelPos = SelectPos + (ReverseSearch?-1:1);
			}
		}
		else
		{
			LastSelPos = FilePos;

			if (!LastSelPos || LastSelPos == FileSize)
				SearchFlags.Set(SEARCH_MODE2);
		}

		vseek(LastSelPos,SEEK_SET);
		Match = 0;

		if (SearchLength > 0 && (!ReverseSearch || LastSelPos > 0))
		{
			wchar_t *Buf = new wchar_t[8192 + 8192], *t_Buf = t_utf8 ? Buf + 8192 : nullptr;
			int buf_size = 8192, seek_size = 1;
			char *byte_buff = nullptr;
			if (SearchHex)
			{
				byte_buff = (char *)Buf;
				buf_size *= 2*(int)sizeof(wchar_t);
				seek_size = IsUnicodeCodePage(VM.CodePage) ? 2 : 1;
			}

			__int64 CurPos=LastSelPos;
			int BufSize = buf_size;

			if (ReverseSearch)
			{
				/* $ 01.08.2000 KM
				   Изменёно вычисление CurPos с учётом Whole words
				*/
				if (WholeWords)
					CurPos -= BufSize/seek_size - SearchLength/seek_size + 1;
				else
					CurPos -= BufSize/seek_size - SearchLength/seek_size;

				if (CurPos<0)
					BufSize += (int)CurPos * seek_size;
			}

			int ReadSize;
			TaskBar TB;
			wakeful W;
			INT64 StartPos = vtell();
			DWORD StartTime = GetTickCount();

			while (!Match)
			{
				/* $ 01.08.2000 KM
				   Изменена строка if (ReverseSearch && CurPos<0) на if (CurPos<0),
				   так как при обычном прямом и LastSelPos=0xFFFFFFFF, поиск
				   заканчивался так и не начавшись.
				*/
				if (CurPos < 0)
					CurPos = 0;

				vseek(CurPos, SEEK_SET);

				if (SearchHex)
				{
					DWORD nr = 0;
					Reader.Read(byte_buff, (DWORD)BufSize, &nr);
					ReadSize = (int)nr;
				}
				else
				{
					ReadSize = vread(Buf, BufSize, t_Buf);
				}
				if (ReadSize <= 0)
					break;

				DWORD CurTime=GetTickCount();

				if (CurTime-StartTime>RedrawTimeout)
				{
					StartTime=CurTime;

					if (CheckForEscSilent())
					{
						if (ConfirmAbortOp())
						{
							Redraw();
							return;
						}
					}

					INT64 Total=ReverseSearch?StartPos:FileSize-StartPos;
					INT64 Current=_abs64(CurPos-StartPos);
					int Percent=Total>0?static_cast<int>(Current*100/Total):-1;
					// В случае если файл увеличивается размере, то количество
					// процентов может быть больше 100. Обрабатываем эту ситуацию.
					if (Percent>100)
					{
						SetFileSize();
						Total=FileSize-StartPos;
						Percent=Total>0?static_cast<int>(Current*100/Total):-1;
						if (Percent>100)
						{
							Percent=100;
						}
					}
					ViewerSearchMsg(strMsgStr,Percent);
				}

				/* $ 01.08.2000 KM
				   Сделана сразу проверка на Case sensitive и Hex
				   и если нет, тогда Buf приводится к верхнему регистру
				*/
				if (!Case && !SearchHex)
					CharUpperBuff(Buf, ReadSize);

				/* $ 01.08.2000 KM
				   Убран кусок текста после приведения поисковой строки
				   и Buf к единому регистру, если поиск не регистрозависимый
				   или не ищется Hex-строка и в связи с этим переработан код поиска
				*/
				int MaxSize=ReadSize-SearchLength+1;
				int Increment=ReverseSearch ? -seek_size : +seek_size;

				for (int I=ReverseSearch ? MaxSize-1:0; I<MaxSize && I>=0; I+=Increment)
				{
					/* $ 01.08.2000 KM
					   Обработка поиска "Whole words"
					*/
					int locResultLeft=FALSE;
					int locResultRight=FALSE;

					if (WholeWords)
					{
						if (I)
						{
							if (IsSpace(Buf[I-1]) || IsEol(Buf[I-1]) ||
							        (wcschr(Opt.strWordDiv,Buf[I-1])))
								locResultLeft=TRUE;
						}
						else
						{
							locResultLeft=TRUE;
						}

						if (ReadSize!=BufSize && I+SearchLength>=ReadSize)
							locResultRight=TRUE;
						else if (I+SearchLength<ReadSize &&
						         (IsSpace(Buf[I+SearchLength]) || IsEol(Buf[I+SearchLength]) ||
						          (wcschr(Opt.strWordDiv,Buf[I+SearchLength]))))
							locResultRight=TRUE;
					}
					else
					{
						locResultLeft=TRUE;
						locResultRight=TRUE;
					}

					if (!SearchHex)
					{
						Match = locResultLeft && locResultRight && strSearchStr.At(0) == Buf[I] &&
							( SearchLength==1 ||
								( strSearchStr.At(1) == Buf[I+1] &&
									(SearchLength==2 || !memcmp(strSearchStr.CPtr()+2,&Buf[I+2],(SearchLength-2)*sizeof(wchar_t)))
								)
							)
						;
					}
					else
					{
						if ((Match = search_bytes[0] == byte_buff[I+0]) && SearchLength > 1)
							if ((Match = search_bytes[1] == byte_buff[I+1]) && SearchLength > 2)
								if ((Match = search_bytes[2] == byte_buff[I+2]) && SearchLength > 3)
									if ((Match = search_bytes[3] == byte_buff[I+3]) && SearchLength > 4)
										Match = !memcmp(search_bytes+4, byte_buff+I+4, SearchLength-4);
					}

					if (Match)
					{
						MatchPos = CurPos + I / seek_size;
						if (t_utf8)
						{
							MatchPos = CurPos + GetStrBytesNum(t_Buf, I);
							SearchLength = GetStrBytesNum(t_Buf+I, SearchLength);
						}
						break;
					}
				}

				if ((ReverseSearch && CurPos <= 0) || (!ReverseSearch && veof()))
					break;

				int asz = buf_size;
				if (t_utf8)
					asz = GetStrBytesNum(t_Buf, ReadSize);

				if (ReverseSearch)
				{
					/* $ 01.08.2000 KM
					   Изменёно вычисление CurPos с учётом Whole words
					*/
					if (WholeWords)
						CurPos -= asz/seek_size - SearchLength/seek_size + 1;
					else
						CurPos -= asz/seek_size - SearchLength/seek_size;
				}
				else
				{
					if (WholeWords)
						CurPos += asz/seek_size - SearchLength/seek_size + 1;
					else
						CurPos += asz/seek_size - SearchLength/seek_size;
				}
			}

			delete[] Buf;
		}
	}

	if (Match)
	{
		/* $ 24.01.2003 KM
		   ! По окончании поиска отступим от верха экрана на
		     треть отображаемой высоты.
		*/
		SelectText(MatchPos,SearchLength,ReverseSearch?0x2:0);
		// Покажем найденное на расстоянии трети экрана от верха.
		int FromTop=(ScrY-(Opt.ViOpt.ShowKeyBar?2:1))/4;

		if (FromTop<0 || FromTop>ScrY)
			FromTop=0;

		for (int i=0; i<FromTop; i++)
			Up();

		AdjustSelPosition = TRUE;
		Show();
		AdjustSelPosition = FALSE;
	}
	else
	{
		//Show();
		/* $ 27.01.2003 VVM
		   + После окончания поиска спросим о переходе поиска в начало/конец */
		if (SearchFlags.Check(SEARCH_MODE2))
		{
			Message(MSG_WARNING,1,MSG(MViewSearchTitle),
			        (SearchHex?MSG(MViewSearchCannotFindHex):MSG(MViewSearchCannotFind)),strMsgStr,MSG(MOk));
		}
		else
		{
			if (!Message(MSG_WARNING,2,MSG(MViewSearchTitle),
			            (SearchHex?MSG(MViewSearchCannotFindHex):MSG(MViewSearchCannotFind)),strMsgStr,
			            (ReverseSearch?MSG(MViewSearchFromEnd):MSG(MViewSearchFromBegin)),
			            MSG(MHYes),MSG(MHNo)))
			{
				Search(2,0);
			}
		}
	}
}


int Viewer::GetWrapMode()
{
	return(VM.Wrap);
}


void Viewer::SetWrapMode(int Wrap)
{
	Viewer::VM.Wrap=Wrap;
}


void Viewer::EnableHideCursor(int HideCursor)
{
	Viewer::HideCursor=HideCursor;
}


int Viewer::GetWrapType()
{
	return(VM.WordWrap);
}


void Viewer::SetWrapType(int TypeWrap)
{
	Viewer::VM.WordWrap=TypeWrap;
}


void Viewer::GetFileName(string &strName)
{
	strName = strFullFileName;
}


void Viewer::ShowConsoleTitle()
{
	string strTitle;
	strTitle.Format(MSG(MInViewer), PointToName(strFileName));
	ConsoleTitle::SetFarTitle(strTitle);
}


void Viewer::SetTempViewName(const wchar_t *Name, BOOL DeleteFolder)
{
	if (Name && *Name)
		ConvertNameToFull(Name,strTempViewName);
	else
	{
		strTempViewName.Clear();
		DeleteFolder=FALSE;
	}

	Viewer::DeleteFolder=DeleteFolder;
}


void Viewer::SetTitle(const wchar_t *Title)
{
	if (!Title)
		strTitle.Clear();
	else
		strTitle = Title;
}


void Viewer::SetFilePos(__int64 Pos)
{
	FilePos=Pos;
};


void Viewer::SetPluginData(const wchar_t *PluginData)
{
	Viewer::strPluginData = NullToEmpty(PluginData);
}


void Viewer::SetNamesList(NamesList *List)
{
	if (List)
		List->MoveData(ViewNamesList);
}


static int utf8_to_WideChar(const char *s, int nc, wchar_t *w1,wchar_t *w2, int wlen, int &tail )
{
	bool need_one = wlen <= 0;
	if (need_one)
		wlen = 2;

	int ic = 0, nw = 0, wc;

	while ( ic < nc )
	{
		unsigned char c4, c3, c2, c1 = ((const unsigned char *)s)[ic++];

		if (c1 < 0x80) // simple ASCII
			wc = (wchar_t)c1;
		else if ( c1 < 0xC2 || c1 >= 0xF5 ) // illegal 1-st byte
			wc = -1;
		else
		{ // multibyte (2, 3, 4)
			if (ic + 0 >= nc )
			{ // unfinished
			unfinished:
				if ( nw > 0 )
					tail = nc - ic + 1;
				else
				{
					tail = 0;
					w1[0] = REPLACE_CHAR;
					if (w2)
						w2[0] = L'?';
					nw = 1;
				}
				return nw;
			}
			c2 = ((const unsigned char *)s)[ic];
			if ( 0x80 != (c2 & 0xC0)        // illegal 2-nd byte
				|| (0xE0 == c1 && c2 <= 0x9F) // illegal 3-byte start (overlaps with 2-byte)
				|| (0xF0 == c1 && c2 <= 0x8F) // illegal 4-byte start (overlaps with 3-byte)
				|| (0xF4 == c1 && c2 >= 0x90) // illegal 4-byte (out of unicode range)
			)
			{
				wc = -1;
			}
			else if ( c1 < 0xE0 )
			{ // legal 2-byte
				++ic;
				wc = ((c1 & 0x1F) << 6) | (c2 & 0x3F);
			}
			else
			{ // 3 or 4-byte
				if (ic + 1 >= nc )
					goto unfinished;
				c3 = ((const unsigned char *)s)[ic+1];
				if ( 0x80 != (c3 & 0xC0) ) // illegal 3-rd byte
					wc = -1;
				else if ( c1 < 0xF0 )
				{ // legal 3-byte
					ic += 2;
					wc = ((c1 & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
				}
				else
				{ // 4-byte
					if (ic + 2 >= nc )
						goto unfinished;

					c4 = ((const unsigned char *)s)[ic+2];
					if ( 0x80 != (c4 & 0xC0) ) // illegal 4-th byte
						wc = -1;
					else
					{ // legal 4-byte (produce 2 WCHARs)
						ic += 3;
						wc = ((c1 & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
						wc -= 0x10000;
						w1[nw] = (wchar_t)(0xD800 + (wc >> 10));
						if (w2)
							w2[nw] = w1[nw];
						++nw;
						wc = 0xDC00 + (wc & 0x3FF);
#if 0
						_ASSERTE(nw < wlen); //??? caller should be fixed to avoid this...
#endif
						if (nw >= wlen)
						{
							--nw;
							wc = REPLACE_CHAR;
						}
					}
				}
			}
		}

		if ( wc >= 0 )
		{
			w1[nw] = (wchar_t)wc;
			if (w2)
				w2[nw] = (wchar_t)wc;
		}
		else
		{
			w1[nw] = REPLACE_CHAR;
			if (w2)
				w2[nw] = L'?';
		}
		if (++nw >= wlen || need_one)
			break;
	}

	tail = nc - ic;
	return nw;
}

int Viewer::vread(wchar_t *Buf, int Count, wchar_t *Buf2)
{
	if (Count <= 0)
		return 0;

	DWORD ReadSize = 0;

	if (IsUnicodeCodePage(VM.CodePage))
	{
		Reader.Read(Buf, Count*2, &ReadSize);
		if (0 != (ReadSize & 1))
			((char *)Buf)[ReadSize++] = '\0';

		if (CP_REVERSEBOM == VM.CodePage)
		{
			for (DWORD i=0; i<ReadSize; i+=2)
			{
				char t = ((char *)Buf)[i];
				((char *)Buf)[i] = ((char *)Buf)[i+1];
				((char *)Buf)[i+1] = t;
			}
		}

		ReadSize /= 2;
	}
	else
	{
		char *TmpBuf = vread_buffer;
		if (Count > vread_buffer_size)
			TmpBuf = new char[Count];

		Reader.Read(TmpBuf, Count, &ReadSize);
		int ConvertSize = (int)ReadSize;

		if (VM.CodePage != CP_UTF8)
		{
			ReadSize = (DWORD)MultiByteToWideChar(VM.CodePage,0, TmpBuf,ConvertSize, Buf,Count);
		}
		else
		{
			int tail;
			ReadSize = (DWORD)utf8_to_WideChar(TmpBuf, ConvertSize, Buf,Buf2, Count, tail);
			if (tail)
				Reader.Unread(tail);
		}

		if (TmpBuf != vread_buffer)
			delete[] TmpBuf;
	}

	return (int)ReadSize;
}

bool Viewer::vseek(__int64 Offset, int Whence)
{
	int seek_size = IsUnicodeCodePage(VM.CodePage) ? 2 : 1;
	Offset *= seek_size;

	if (FILE_CURRENT == Whence)
	{
		if (vgetc_ready)
		{
			int tail = vgetc_cb - vgetc_ib;
			tail += (seek_size - 1) * (tail & 1);
			Offset += tail;
			vgetc_ready = false;
		}
		if (0 == Offset)
			return true;
	}

	vgetc_ready = false;
	return ViewFile.SetPointer(Offset, nullptr, Whence);
}

__int64 Viewer::vtell()
{
	__int64 Ptr = ViewFile.GetPointer();

	if (vgetc_ready)
		Ptr -= (vgetc_cb - vgetc_ib);

	if (IsUnicodeCodePage(VM.CodePage))
		Ptr=(Ptr+(Ptr&1))/2;

	return Ptr;
}

bool Viewer::veof()
{
	if (vgetc_ready && vgetc_ib < vgetc_cb)
		return false;
	else
		return ViewFile.Eof();
}

bool Viewer::vgetc(wchar_t *pCh)
{
	if (!vgetc_ready)
		vgetc_cb = vgetc_ib = (int)(vgetc_composite = 0);

	if (vgetc_cb - vgetc_ib < 4 && !ViewFile.Eof())
	{
		vgetc_cb -= vgetc_ib;
		if (vgetc_cb && vgetc_ib)
			memmove(vgetc_buffer, vgetc_buffer+vgetc_ib, vgetc_cb);
		vgetc_ib = 0;

		DWORD nr = 0;
		Reader.Read(vgetc_buffer + vgetc_cb, (DWORD)(ARRAYSIZE(vgetc_buffer)-vgetc_cb), &nr);
		vgetc_cb += (int)nr;

		if (0 != (vgetc_cb & 1) && IsUnicodeCodePage(VM.CodePage))
			vgetc_buffer[vgetc_cb++] = 0;
	}

	vgetc_ready = true;

	if (vgetc_composite)
	{
		if (pCh)
			*pCh = vgetc_composite;
		vgetc_composite = 0;
		return true;
	}

	if (!pCh)
		return true;

	if (vgetc_cb <= vgetc_ib)
		return false;

	switch (VM.CodePage)
	{
		case CP_REVERSEBOM:
			*pCh = (wchar_t)((vgetc_buffer[vgetc_ib] << 8) | vgetc_buffer[vgetc_ib+1]);
			vgetc_ib += 2;
		break;
		case CP_UNICODE:
			*pCh = (wchar_t)((vgetc_buffer[vgetc_ib+1] << 8) | vgetc_buffer[vgetc_ib]);
			vgetc_ib += 2;
		break;
		case CP_UTF8:
		{
			int tail;
			wchar_t w[2];
			int nw = utf8_to_WideChar((const char *)vgetc_buffer+vgetc_ib, vgetc_cb-vgetc_ib, w,nullptr, -2,tail);
			vgetc_ib = vgetc_cb - tail;
			*pCh = w[0];
			if (nw > 1)
				vgetc_composite = w[1];
			break;
		}
		default:
			MultiByteToWideChar(VM.CodePage, 0, (LPCSTR)vgetc_buffer+vgetc_ib,1, pCh,1);
			++vgetc_ib;
		break;
	}

	return true;
}

#define RB_PRC 3
#define RB_HEX 4
#define RB_DEC 5

void Viewer::GoTo(int ShowDlg,__int64 Offset, UINT64 Flags)
{
	__int64 Relative=0;
	const wchar_t *LineHistoryName=L"ViewerOffset";
	FarDialogItem GoToDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,31,7,0,nullptr,nullptr,0,MSG(MViewerGoTo)},
		{DI_EDIT,5,2,29,2,0,LineHistoryName,nullptr,DIF_FOCUS|DIF_DEFAULTBUTTON|DIF_HISTORY|DIF_USELASTHISTORY,L""},
		{DI_TEXT,3,3,0,3,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_RADIOBUTTON,5,4,0,4,0,nullptr,nullptr,DIF_GROUP,MSG(MGoToPercent)},
		{DI_RADIOBUTTON,5,5,0,5,0,nullptr,nullptr,0,MSG(MGoToHex)},
		{DI_RADIOBUTTON,5,6,0,6,0,nullptr,nullptr,0,MSG(MGoToDecimal)},
	};
	MakeDialogItemsEx(GoToDlgData,GoToDlg);
	static int PrevMode=0;
	GoToDlg[3].Selected=GoToDlg[4].Selected=GoToDlg[5].Selected=0;

	if (VM.Hex)
		PrevMode=1;

	GoToDlg[PrevMode+3].Selected=TRUE;
	{
		if (ShowDlg)
		{
			Dialog Dlg(GoToDlg,ARRAYSIZE(GoToDlg));
			Dlg.SetHelp(L"ViewerGotoPos");
			Dlg.SetPosition(-1,-1,35,9);
			Dlg.Process();

			if (Dlg.GetExitCode()<=0)
				return;

			if (GoToDlg[1].strData.At(0)==L'+' || GoToDlg[1].strData.At(0)==L'-')       // юзер хочет относительности
			{
				if (GoToDlg[1].strData.At(0)==L'+')
					Relative=1;
				else
					Relative=-1;

				GoToDlg[1].strData.LShift(1);
			}

			if (GoToDlg[1].strData.Contains(L'%'))     // он хочет процентов
			{
				GoToDlg[RB_HEX].Selected=GoToDlg[RB_DEC].Selected=0;
				GoToDlg[RB_PRC].Selected=1;
			}
			else if (!StrCmpNI(GoToDlg[1].strData,L"0x",2)
			         || GoToDlg[1].strData.At(0)==L'$'
			         || GoToDlg[1].strData.Contains(L'h')
			         || GoToDlg[1].strData.Contains(L'H'))  // он умный - hex код ввел!
			{
				GoToDlg[RB_PRC].Selected=GoToDlg[RB_DEC].Selected=0;
				GoToDlg[RB_HEX].Selected=1;

				if (!StrCmpNI(GoToDlg[1].strData,L"0x",2))
					GoToDlg[1].strData.LShift(2);
				else if (GoToDlg[1].strData.At(0)==L'$')
					GoToDlg[1].strData.LShift(1);

				//Relative=0; // при hex значении никаких относительных значений?
			}

			if (GoToDlg[RB_PRC].Selected)
			{
				//int cPercent=ToPercent64(FilePos,FileSize);
				PrevMode=0;
				int Percent=_wtoi(GoToDlg[1].strData);

				//if ( Relative  && (cPercent+Percent*Relative<0) || (cPercent+Percent*Relative>100)) // за пределы - низя
				//  return;
				if (Percent>100)
					return;

				//if ( Percent<0 )
				//  Percent=0;
				Offset=FileSize/100*Percent;

				if (IsUnicodeCodePage(VM.CodePage))
					Offset*=2;

				while (ToPercent64(Offset,FileSize)<Percent)
					Offset++;
			}

			if (GoToDlg[RB_HEX].Selected)
			{
				PrevMode=1;
				swscanf(GoToDlg[1].strData,L"%I64x",&Offset);
			}

			if (GoToDlg[RB_DEC].Selected)
			{
				PrevMode=2;
				swscanf(GoToDlg[1].strData,L"%I64d",&Offset);
			}
		}// ShowDlg
		else
		{
			Relative=(Flags&VSP_RELATIVE)*(Offset<0?-1:1);

			if (Flags&VSP_PERCENT)
			{
				__int64 Percent=Offset;

				if (Percent>100)
					return;

				//if ( Percent<0 )
				//  Percent=0;
				Offset=FileSize/100*Percent;

				if (IsUnicodeCodePage(VM.CodePage))
					Offset*=2;

				while (ToPercent64(Offset,FileSize)<Percent)
					Offset++;
			}
		}

		if (Relative)
		{
			if (Relative==-1 && Offset>FilePos)   // меньше нуля, if (FilePos<0) не пройдет - FilePos у нас unsigned long
				FilePos=0;
			else
				FilePos=IsUnicodeCodePage(VM.CodePage)? FilePos+Offset*Relative/2 : FilePos+Offset*Relative;
		}
		else
			FilePos=IsUnicodeCodePage(VM.CodePage) ? Offset/2:Offset;

		if (FilePos>FileSize || FilePos<0)     // и куда его несет?
			FilePos=FileSize;     // там все равно ничего нету
	}
	// коррекция
	AdjustFilePos();

//  LastSelPos=FilePos;
	if (!(Flags&VSP_NOREDRAW))
		Show();
}

void Viewer::AdjustFilePos()
{
	wchar_t ch;

	if (!VM.Hex)
	{
		vseek(FilePos, SEEK_SET);
		if (VM.CodePage != CP_UTF8)
		{
			vgetc(&ch);
		}
		else
		{
			vgetc(nullptr);
			if (vgetc_ib < vgetc_cb)
			{
				if (0x80 == (vgetc_buffer[vgetc_ib] & 0xC0))
		{
					if (++vgetc_ib < vgetc_cb)
					{
						if (0x80 == (vgetc_buffer[vgetc_ib] & 0xC0))
						{
							if (++vgetc_ib < vgetc_cb)
							{
								if (0x80 == (vgetc_buffer[vgetc_ib] & 0xC0))
									++vgetc_ib;
							}
						}
					}
		}
				else
		{
					vgetc(&ch);
				}
			}
		}

		   FilePos = vtell();
			Up();
		}
	}

void Viewer::SetFileSize()
{
	if (!ViewFile.Opened())
		return;

	UINT64 uFileSize=0; // BUGBUG, sign
	ViewFile.GetSize(uFileSize);
	FileSize=uFileSize;

	/* $ 20.02.2003 IS
	   Везде сравниваем FilePos с FileSize, FilePos для юникодных файлов
	   уменьшается в два раза, поэтому FileSize тоже надо уменьшать
	*/
	if (IsUnicodeCodePage(VM.CodePage))
		FileSize=(FileSize+(FileSize&1))/2;
}


void Viewer::GetSelectedParam(__int64 &Pos, __int64 &Length, DWORD &Flags)
{
	Pos=SelectPos;
	Length=SelectSize;
	Flags=SelectFlags;
}

/* $ 19.01.2001 SVS
   Выделение - в качестве самостоятельной функции.
   Flags=0x01 - показывать (делать Show())
         0x02 - "обратный поиск" ?
*/
void Viewer::SelectText(const __int64 &MatchPos,const __int64 &SearchLength, const DWORD Flags)
{
	if (!ViewFile.Opened())
		return;

	int buff_size = 1024;
	wchar_t *Buf = Up_buffer, *tBuf = (!VM.Hex && VM.CodePage == CP_UTF8) ? Buf + buff_size : nullptr;
	__int64 StartLinePos = -1, SearchLinePos = MatchPos - buff_size;
	if (SearchLinePos < 0)
	{
		buff_size += (int)SearchLinePos;
		SearchLinePos = 0;
	}

	vseek(SearchLinePos, SEEK_SET);
	int ReadSize = vread(Buf, buff_size, tBuf);

	for (int I=ReadSize-1; I >= 0; I--)
	if (Buf[I]==L'\n' || Buf[I]==L'\r')
	{
		if (tBuf)
			StartLinePos = MatchPos - GetStrBytesNum(tBuf+I, ReadSize-I);
		else
			StartLinePos = SearchLinePos+I;
		break;
	}

	vseek(MatchPos+1, SEEK_SET);
	SelectPos = FilePos=MatchPos;
	SelectSize = SearchLength;
	SelectFlags = Flags;

//  LastSelPos=SelectPos+((Flags&0x2) ? -1:1);
	if (VM.Hex)
	{
		FilePos&=~(IsUnicodeCodePage(VM.CodePage) ? 0x7:0xf);
	}
	else
	{
		if (SelectPos != StartLinePos)
		{
			Up();
			Show();  //update OutStr
		}

		/* $ 13.03.2001 IS
		   Если найденное расположено в самой первой строке юникодного файла и файл
		   имеет в начале fffe или feff, то для более правильного выделения, его
		   позицию нужно уменьшить на единицу (из-за того, что пустой символ не
		   показывается)
		*/
		SelectPosOffSet=(IsUnicodeOrUtfCodePage(VM.CodePage) && Signature
		                 && (MatchPos+SelectSize<=ObjWidth && MatchPos<(__int64)StrLength(Strings[0]->lpData)))?1:0;
		SelectPos-=SelectPosOffSet;
		__int64 Length=SelectPos-StartLinePos-1;

		if (VM.Wrap)
			Length%=Width+1; //??

		if (Length<=Width)
			LeftPos=0;

		if (Length-LeftPos>Width || Length<LeftPos)
		{
			LeftPos=Length;

			if (LeftPos>(MAX_VIEWLINE-1) || LeftPos<0)
				LeftPos=0;
			else if (LeftPos>10)
				LeftPos-=10;
		}
	}

	if (Flags&1)
	{
		AdjustSelPosition = TRUE;
		Show();
		AdjustSelPosition = FALSE;
	}
}

int Viewer::ViewerControl(int Command,void *Param)
{
	switch (Command)
	{
		case VCTL_GETINFO:
		{
			if (Param)
			{
				ViewerInfo *Info=(ViewerInfo *)Param;
				memset(&Info->ViewerID,0,Info->StructSize-sizeof(Info->StructSize));
				Info->ViewerID=Viewer::ViewerID;
				Info->FileName=strFullFileName;
				Info->WindowSizeX=ObjWidth;
				Info->WindowSizeY=Y2-Y1+1;
				Info->FilePos=FilePos;
				Info->FileSize=FileSize;
				Info->CurMode=VM;
				Info->Options=0;

				if (Opt.ViOpt.SavePos)   Info->Options|=VOPT_SAVEFILEPOSITION;

				if (ViOpt.AutoDetectCodePage)     Info->Options|=VOPT_AUTODETECTCODEPAGE;

				Info->TabSize=ViOpt.TabSize;
				Info->LeftPos=LeftPos;
				return TRUE;
			}

			break;
		}
		/*
		   Param = ViewerSetPosition
		           сюда же будет записано новое смещение
		           В основном совпадает с переданным
		*/
		case VCTL_SETPOSITION:
		{
			if (Param)
			{
				ViewerSetPosition *vsp=(ViewerSetPosition*)Param;
				bool isReShow=vsp->StartPos != FilePos;

				if ((LeftPos=vsp->LeftPos) < 0)
					LeftPos=0;

				/* $ 20.01.2003 IS
				     Если кодировка - юникод, то оперируем числами, уменьшенными в
				     2 раза. Поэтому увеличим StartPos в 2 раза, т.к. функция
				     GoTo принимает смещения в _байтах_.
				*/
				GoTo(FALSE, vsp->StartPos*(IsUnicodeCodePage(VM.CodePage)?2:1), vsp->Flags);

				if (isReShow && !(vsp->Flags&VSP_NOREDRAW))
					ScrBuf.Flush();

				if (!(vsp->Flags&VSP_NORETNEWPOS))
				{
					vsp->StartPos=FilePos;
					vsp->LeftPos=LeftPos;
				}

				return TRUE;
			}

			break;
		}
		// Param=ViewerSelect
		case VCTL_SELECT:
		{
			if (Param)
			{
				ViewerSelect *vs=(ViewerSelect *)Param;
				__int64 SPos=vs->BlockStartPos;
				int SSize=vs->BlockLen;

				if (SPos < FileSize)
				{
					if (SPos+SSize > FileSize)
					{
						SSize=(int)(FileSize-SPos);
					}

					SelectText(SPos,SSize,0x1);
					ScrBuf.Flush();
					return TRUE;
				}
			}
			else
			{
				SelectSize = 0;
				Show();
			}

			break;
		}
		/* Функция установки Keybar Labels
		     Param = nullptr - восстановить, пред. значение
		     Param = -1   - обновить полосу (перерисовать)
		     Param = KeyBarTitles
		*/
		case VCTL_SETKEYBAR:
		{
			KeyBarTitles *Kbt=(KeyBarTitles*)Param;

			if (!Kbt)
			{        // восстановить пред значение!
				if (HostFileViewer)
					HostFileViewer->InitKeyBar();
			}
			else
			{
				if ((INT_PTR)Param != (INT_PTR)-1) // не только перерисовать?
					ViewKeyBar->Change(Kbt);

				ViewKeyBar->Show();
				ScrBuf.Flush(); //?????
			}

			return TRUE;
		}
		// Param=0
		case VCTL_REDRAW:
		{
			ChangeViewKeyBar();
			Show();
			ScrBuf.Flush();
			return TRUE;
		}
		// Param=0
		case VCTL_QUIT:
		{
			/* $ 28.12.2002 IS
			   Разрешаем выполнение VCTL_QUIT только для вьюера, который
			   не является панелью информации и быстрого просмотра (т.е.
			   фактически панелей на экране не видно)
			*/
			if (!FrameManager->IsPanelsActive())
			{
				/* $ 29.09.2002 IS
				   без этого не закрывался вьюер, а просили именно это
				*/
				FrameManager->DeleteFrame(HostFileViewer);

				if (HostFileViewer)
					HostFileViewer->SetExitCode(0);

				return TRUE;
			}
		}
		/* Функция установки режимов
		     Param = ViewerSetMode
		*/
		case VCTL_SETMODE:
		{
			ViewerSetMode *vsmode=(ViewerSetMode *)Param;

			if (vsmode)
			{
				bool isRedraw=vsmode->Flags&VSMFL_REDRAW?true:false;

				switch (vsmode->Type)
				{
					case VSMT_HEX:
						ProcessHexMode(vsmode->iParam,isRedraw);
						return TRUE;
					case VSMT_WRAP:
						ProcessWrapMode(vsmode->iParam,isRedraw);
						return TRUE;
					case VSMT_WORDWRAP:
						ProcessTypeWrapMode(vsmode->iParam,isRedraw);
						return TRUE;
				}
			}

			return FALSE;
		}
	}

	return FALSE;
}

BOOL Viewer::isTemporary()
{
	return !strTempViewName.IsEmpty();
}

int Viewer::ProcessHexMode(int newMode, bool isRedraw)
{
	int oldHex=VM.Hex;
	VM.Hex=newMode&1;

	if (!VM.Hex)
		AdjustFilePos();

	if (isRedraw)
	{
		ChangeViewKeyBar();
		Show();
	}

// LastSelPos=FilePos;
	return oldHex;
}

int Viewer::ProcessWrapMode(int newMode, bool isRedraw)
{
	int oldWrap=VM.Wrap;
	VM.Wrap=newMode&1;

	if (VM.Wrap)
		LeftPos = 0;

	if (!VM.Wrap)
		AdjustFilePos();

	if (isRedraw)
	{
		ChangeViewKeyBar();
		Show();
	}

	Opt.ViOpt.ViewerIsWrap=VM.Wrap;
//  LastSelPos=FilePos;
	return oldWrap;
}

int Viewer::ProcessTypeWrapMode(int newMode, bool isRedraw)
{
	int oldTypeWrap=VM.WordWrap;
	VM.WordWrap=newMode&1;

	if (!VM.Wrap)
	{
		VM.Wrap=!VM.Wrap;
		LeftPos = 0;
	}

	if (isRedraw)
	{
		ChangeViewKeyBar();
		Show();
	}

	Opt.ViOpt.ViewerWrap=VM.WordWrap;
//LastSelPos=FilePos;
	return oldTypeWrap;
}
