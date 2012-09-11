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
#include "drivemix.hpp"
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
#include "RegExp.hpp"
#include "palette.hpp"

static void PR_ViewerSearchMsg();
static void ViewerSearchMsg(const wchar_t *name, int percent, int search_hex);

static int ViewerID=0;

static int utf8_to_WideChar(const char *s, int nc, wchar_t *w1,wchar_t *w2, int wlen, int &tail);

#define REPLACE_CHAR  0xFFFD // Replacement
#define CONTINUE_CHAR 0x203A // Single Right-Pointing Angle Quotation Mark
#define BOM_CHAR      0xFEFF // Zero Length Space
#define ZERO_CHAR     (ViOpt.Visible0x00 && ViOpt.ZeroChar > 0 ? (wchar_t)(ViOpt.ZeroChar) : L' ')

Viewer::Viewer(bool bQuickView, UINT aCodePage):
	ViOpt(Opt.ViOpt),
	Reader(ViewFile, (Opt.ViOpt.MaxLineSize*2*64 > 64*1024 ? Opt.ViOpt.MaxLineSize*2*64 : 64*1024)),
	m_bQuickView(bQuickView)
{
	_OT(SysLog(L"[%p] Viewer::Viewer()", this));

	Strings = new ViewerString*[MAXSCRY+1];
	for (int i=0; i <= MAXSCRY; i++)
	{
		Strings[i] = new ViewerString();
	}

	strLastSearchStr = strGlobalSearchString;
	LastSearchCase=GlobalSearchCase;
	LastSearchRegexp=Opt.ViOpt.SearchRegexp;
	LastSearchWholeWords=GlobalSearchWholeWords;
	LastSearchReverse=GlobalSearchReverse;
	LastSearchHex=GlobalSearchHex;
	LastSearchDirection = GlobalSearchReverse ? -1 : +1;
	StartSearchPos = 0;
	VM.CodePage=DefCodePage=aCodePage;
	// Вспомним тип врапа
	VM.Wrap=Opt.ViOpt.ViewerIsWrap;
	VM.WordWrap=Opt.ViOpt.ViewerWrap;
	VM.Hex = dump_text_mode = -1;
	ViewKeyBar=nullptr;
	FilePos=0;
	LeftPos=0;
	SecondPos=0;
	FileSize=0;
	LastPage=0;
	SelectPos = 0; SelectSize = -1;
	LastSelectPos = 0; LastSelectSize = -1;
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
	CtrlObject->Plugins->CurViewer=this;
	OpenFailed=false;
	HostFileViewer=nullptr;
	bVE_READ_Sent = false;
	Signature = false;

	vgetc_ready = false;
	vgetc_cb = vgetc_ib = 0;
	vgetc_composite = L'\0';

	vread_buffer_size = Max(MAX_VIEWLINEB, 8192);
	vread_buffer = new char[vread_buffer_size];

	lcache_first = lcache_last = -1;
	lcache_lines = new INT64[lcache_size = 16*1000];
	lcache_count = 0;
	lcache_base = 0;
	lcache_ready = false;
	lcache_wrap = lcache_wwrap = lcache_width = -1;

	int cached_buffer_size = 64*Max(Opt.ViOpt.MaxLineSize*2, 1024);
	max_backward_size = ViewerOptions::eMaxLineSize*3;
	if ( max_backward_size > cached_buffer_size/2 )
		max_backward_size = cached_buffer_size / 2;
	llengths_size = max_backward_size / 40;
	llengths = new int[llengths_size];

	Search_buffer_size = 3 * Max(MAX_VIEWLINEB, 8000);
	Search_buffer = new wchar_t[Search_buffer_size];

	ClearStruct(vString);
	vString.lpData = new wchar_t[MAX_VIEWLINEB];
}


Viewer::~Viewer()
{
	KeepInitParameters();

	if (ViewFile.Opened())
	{
		ViewFile.Close();
		SavePosition();
	}

	delete[] vString.lpData;
	delete[] Search_buffer;
	delete[] llengths;
	delete[] lcache_lines;
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

	for (int i=MAXSCRY; i >= 0; --i)
	{
		if (Strings[i]->lpData)
			delete[] Strings[i]->lpData;
		delete Strings[i];
	}
	delete[] Strings;

	if (!OpenFailed && bVE_READ_Sent)
	{
		CtrlObject->Plugins->CurViewer=this; //HostFileViewer;
		CtrlObject->Plugins->ProcessViewerEvent(VE_CLOSE,nullptr,ViewerID);
	}

	if (this == CtrlObject->Plugins->CurViewer)
		CtrlObject->Plugins->CurViewer = nullptr;
}


void Viewer::SavePosition()
{
	if (Opt.ViOpt.SavePos || Opt.ViOpt.SaveCodepage || Opt.ViOpt.SaveWrapMode)
	{
		ViewerPosCache poscache;

		poscache.FilePos = FilePos;
		poscache.LeftPos = LeftPos;
		poscache.Hex_Wrap = (VM.Hex & 0x03) | 0x10 | (VM.Wrap ? 0x20 : 0x00) | (VM.WordWrap ? 0x40 : 0x00);
		poscache.CodePage = CodePageChangedByUser ? VM.CodePage : 0;
		poscache.bm = BMSavePos;

		string strCacheName = strPluginData.IsEmpty() ? strFullFileName : strPluginData+PointToName(strFileName);
		FilePositionCache::AddPosition(strCacheName, poscache);
	}
}


void Viewer::KeepInitParameters()
{
	strGlobalSearchString = strLastSearchStr;
	GlobalSearchCase=LastSearchCase;
	GlobalSearchWholeWords=LastSearchWholeWords;
	GlobalSearchReverse=LastSearchReverse;
	GlobalSearchHex=LastSearchHex;
	Opt.ViOpt.ViewerIsWrap=VM.Wrap != 0;
	Opt.ViOpt.ViewerWrap=VM.WordWrap != 0;
	Opt.ViOpt.SearchRegexp=LastSearchRegexp;
}


int Viewer::OpenFile(const wchar_t *Name,int warning)
{
	VM.CodePage=DefCodePage;
	DefCodePage=CP_DEFAULT;
	OpenFailed=false;

	ViewFile.Close();
	Reader.Clear();
	vgetc_ready = lcache_ready = false;

	SelectSize = -1; // Сбросим выделение
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

		DWORD ReadSize = 0, WrittenSize;

		while (ReadFile(Console.GetInputHandle(),vread_buffer,(DWORD)vread_buffer_size,&ReadSize,nullptr) && ReadSize)
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
	Reader.AdjustAlignment();

	CodePageChangedByUser=FALSE;

	ConvertNameToFull(strFileName,strFullFileName);
	apiGetFindDataEx(strFileName, ViewFindData);
	UINT CachedCodePage=0;

	if ((Opt.ViOpt.SavePos || Opt.ViOpt.SaveCodepage || Opt.ViOpt.SaveWrapMode) && !ReadStdin)
	{
		__int64 NewLeftPos,NewFilePos;
		string strCacheName=strPluginData.IsEmpty()?strFileName:strPluginData+PointToName(strFileName);
		ViewerPosCache poscache;

		bool found = FilePositionCache::GetPosition(strCacheName,poscache);
		if (Opt.ViOpt.SavePos)
		{
			NewFilePos=poscache.FilePos;
			NewLeftPos=poscache.LeftPos;
			if ( found && VM.Hex == -1 ) // keep VM.Hex if file listed (Grey+ / Gray-)
			{
				if ( 1 != (VM.Hex = (poscache.Hex_Wrap & 0x03)) )
					dump_text_mode = VM.Hex;
			}
			BMSavePos=poscache.bm;

			LastSelectPos=FilePos=NewFilePos;
			LeftPos=NewLeftPos;
		}
		if (Opt.ViOpt.SaveCodepage || Opt.ViOpt.SavePos)
		{
			CachedCodePage=poscache.CodePage;
			if (CachedCodePage && !IsCodePageSupported(CachedCodePage))
				CachedCodePage = 0;
		}
		if (Opt.ViOpt.SaveWrapMode && 0 != (poscache.Hex_Wrap & 0x10))
		{
			VM.Wrap     = (poscache.Hex_Wrap & 0x20 ? 1 : 0);
			VM.WordWrap = (poscache.Hex_Wrap & 0x40 ? 1 : 0);
		}
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

		if (VM.CodePage == CP_DEFAULT || IsUnicodeOrUtfCodePage(VM.CodePage))
		{
			Detect=GetFileFormat(ViewFile,CodePage,&Signature,Opt.ViOpt.AutoDetectCodePage!=0);

			// Проверяем поддерживается или нет задетектированная кодовая страница
			if (Detect)
				Detect = IsCodePageSupported(CodePage);
		}

		if (VM.CodePage==CP_DEFAULT)
		{
			if (Detect)
			{
				VM.CodePage=CodePage;
				//??? CodePageChangedByUser=TRUE;
			}

			if (CachedCodePage)
			{
				VM.CodePage=CachedCodePage;
				CodePageChangedByUser=TRUE;
			}

			if (VM.CodePage==CP_DEFAULT)
				VM.CodePage=Opt.ViOpt.AnsiCodePageAsDefault?GetACP():GetOEMCP();
		}
		else
		{
			CodePageChangedByUser=TRUE;
		}

		ViewFile.SetPointer(0, nullptr, FILE_BEGIN);
	}
	SetFileSize();

	if ( -1 == dump_text_mode )
		dump_text_mode = isBinaryFile() ? 2 : 0;
	if ( -1 == VM.Hex )
		VM.Hex = dump_text_mode;

	if (FilePos > FileSize)
		FilePos=0;
	if ( FilePos )
		AdjustFilePos();

	ChangeViewKeyBar();
	AdjustWidth();
	CtrlObject->Plugins->CurViewer=this; // HostFileViewer;
	/* $ 15.09.2001 tran
	   пора легализироваться */
	CtrlObject->Plugins->ProcessViewerEvent(VE_READ,nullptr,ViewerID);
	bVE_READ_Sent = true;

	last_update_check = GetTickCount();
	string strRoot;
	GetPathRoot(strFullFileName, strRoot);
	int DriveType = FAR_GetDriveType(strRoot, nullptr, 2); // media inserted here
	switch (DriveType) //??? make it configurable
	{
		case DRIVE_REMOVABLE: update_check_period = -1;  break; // floppy: never
		case DRIVE_USBDRIVE:  update_check_period = 500; break; // flash drive: 0.5 sec
		case DRIVE_FIXED:     update_check_period = +1;  break; // hard disk: 1 msec
		case DRIVE_REMOTE:    update_check_period = 500; break; // network drive: 0.5 sec
		case DRIVE_CDROM:     update_check_period = -1;  break; // cd/dvd: never
		case DRIVE_RAMDISK:   update_check_period = +1;  break; // ram-drive: 1 msec
		default:              update_check_period = -1;  break; // unknown: never
	}

	return TRUE;
}

bool Viewer::isBinaryFile() // very approximate: looks for 0x00,0x00 in first 2048 bytes
{
	unsigned char bf[2048+1]; // not fit to any device sector size
	DWORD nb = static_cast<DWORD>(sizeof(bf)), nr = 0;

	__int64 fpos = vtell();
	vseek(0, FILE_BEGIN);
	bool ok_read = ViewFile.Read(bf, nb, nr, nullptr);
	vseek(fpos, FILE_BEGIN);

	if ( !ok_read )
		return true; // special files like '\\?\C:' are binary

	if ( nr < 2 )
		return (nr > 0 && !bf[0]);

   bool bom =
		( bf[0] == (unsigned char)0xFE && bf[1] == (unsigned char)0xFF ) ||
		( bf[0] == (unsigned char)0xFF && bf[1] == (unsigned char)0xFE );

	for (nb = 0; nb+1 < nr; ++nb)
		if ( !bf[nb] && !bf[nb+1] )
			if ( !bom || 0 == (nb & 1) )
				return true;

	return false;
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
		if (!strFileName.IsEmpty() && ((nMode == SHOW_RELOAD) || (nMode == SHOW_HEX)|| (nMode == SHOW_DUMP)))
		{
			SetScreen(X1,Y1,X2,Y2,L' ',ColorIndexToColor(COL_VIEWERTEXT));
			GotoXY(X1,Y1);
			SetColor(COL_WARNDIALOGTEXT);
			FS<<fmt::MaxWidth(XX2-X1+1)<<MSG(MViewerCannotOpenFile);
			ShowStatus();
		}

		return;
	}

	if (HideCursor)
		SetCursorType(0,10);

	vseek(FilePos,SEEK_SET);
	LastPage = 0;

	if ( SelectSize < 0 )
		SelectPos=FilePos;

	switch (nMode)
	{
		case SHOW_HEX:
			CtrlObject->Plugins->CurViewer = this; //HostFileViewer;
			ShowHex();
			break;
		case SHOW_DUMP:
			CtrlObject->Plugins->CurViewer = this; //HostFileViewer;
			ShowDump();
			break;
		case SHOW_RELOAD:
			CtrlObject->Plugins->CurViewer = this; //HostFileViewer;

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

				Strings[0]->nFilePos = FilePos;
				SecondPos = Strings[1]->nFilePos;
			}
			else
			{
				SecondPos = Strings[0]->nFilePos;
				Strings[0]->nFilePos = FilePos;
			}

			ReadString(Strings[0],(int)(SecondPos-FilePos));
			break;
		case SHOW_DOWN:
			if (Y2 > Y1)
			{
				ViewerString *tmp = Strings[0];
				memmove(&Strings[0], &Strings[1], sizeof(Strings[0]) * (Y2-Y1));
				Strings[Y2-Y1] = tmp;

				FilePos = Strings[0]->nFilePos;
				SecondPos = Strings[1]->nFilePos;
				Strings[Y2-Y1]->nFilePos = Strings[Y2-Y1-1]->nFilePos + Strings[Y2-Y1-1]->linesize;
			}
			else
			{
				Strings[0]->nFilePos += Strings[0]->linesize;
				FilePos = Strings[0]->nFilePos;
				SecondPos = FilePos;
			}
			vseek(Strings[Y2-Y1]->nFilePos, SEEK_SET);
			ReadString(Strings[Y2-Y1],-1);
			break;
	}

	if (nMode != SHOW_HEX && nMode != SHOW_DUMP)
	{
		for (I=0,Y=Y1; Y<=Y2; Y++,I++)
		{
			int StrLen = StrLength(Strings[I]->lpData);
			SetColor(COL_VIEWERTEXT);
			GotoXY(X1,Y);

			if (StrLen > LeftPos)
			{
				FS<<fmt::LeftAlign()<<fmt::ExactWidth(Width)<<&Strings[I]->lpData[static_cast<size_t>(LeftPos)];
			}
			else
			{
				FS<<fmt::MinWidth(Width)<<L"";
			}

			if ( SelectSize >= 0 && Strings[I]->bSelection)
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

					FS<<fmt::MaxWidth(static_cast<size_t>(Length))<<&Strings[I]->lpData[static_cast<size_t>(SelX1+LeftPos)];
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
	ShowPage(VM.Hex ? (VM.Hex > 1 ? SHOW_DUMP : SHOW_HEX) : SHOW_RELOAD);
}


static inline int getCharSize( UINT cp )
{
	if ( CP_UTF8 == cp )
		return -1;
	else if ( CP_UNICODE == cp || CP_REVERSEBOM == cp )
		return +2;
	else
		return +1;
}

static inline int getChSize( UINT cp )
{
	if ( CP_UNICODE == cp || CP_REVERSEBOM == cp )
		return +2;
	else
		return +1;
}

static const int mline = 512;

static void txt_dump(
	UINT cp, const unsigned char *line, DWORD nr, int width, wchar_t *outstr, wchar_t zch )
{
	int tail, nw, ib, iw;
	wchar_t w1[mline], w2[mline];

	if ( cp == CP_UNICODE )
	{
		ib = (int)(nr / 2);
		memcpy(outstr, line, ib*2);
		if ( nr & 1)
			outstr[ib++] = REPLACE_CHAR;
	}
	else if ( cp == CP_REVERSEBOM )
	{
		ib = (int)(nr / 2);
		for ( iw=0; iw < ib; ++iw )
			outstr[iw] = (wchar_t)((line[iw*2] << 8) | line[iw*2+1]);
		if ( nr & 1)
			outstr[ib++] = REPLACE_CHAR;
	}
	else if ( cp != CP_UTF8 )
	{
		ib = MultiByteToWideChar(cp,0, (const char *)line,nr, outstr,width);
		if ( ib < 0)
			ib = 0;
	}
	else
	{
		ib = iw = 0;
		nw = utf8_to_WideChar((char *)line, (int)nr, w1, w2, width, tail);
		bool first = true;
		while (ib < width && iw < nw)
		{
			if (first && w1[iw] == REPLACE_CHAR && w2[iw] == L'?')
			{
				outstr[ib++] = CONTINUE_CHAR; // это может быть не совсем корректно для 'плохих' utf-8
			}                                 // но усложнять из-за этого код на мой взгяд не стоит...
			else
			{
				first = false;
				outstr[ib++] = w1[iw];
			}
			int clen = WideCharToMultiByte(CP_UTF8, 0, w2+iw, 1, NULL,0, NULL,NULL);
			while (--clen > 0 && ib < width)
				outstr[ib++] = CONTINUE_CHAR;
			++iw;
		}
	}

	for ( iw = 0; iw < width; ++iw ) {
		if ( iw >= ib )
			outstr[iw] = L' ';
		else if ( !outstr[iw] )
			outstr[iw] = zch;
	}
	outstr[width] = L'\0';
}


void Viewer::ShowDump()
{
	int Y, EndFile = 0, ch_size = getChSize(VM.CodePage);
	int utf8 = VM.CodePage == CP_UTF8 ? 1 : 0;

	assert(Width <= mline);
	unsigned char line[mline*2];
	wchar_t OutStr[mline+1];
	DWORD nr, nb = (DWORD)Width*ch_size + 3*utf8, mb = (DWORD)Width*ch_size;
	__int64 bpos;

	FilePos -= FilePos % ch_size;
	vseek(SEEK_SET, SecondPos = FilePos);

	for (EndFile=0,Y=Y1; Y<=Y2; Y++)
	{
		SetColor(COL_VIEWERTEXT);
		GotoXY(X1, Y);

		if (EndFile)
		{
			FS<<fmt::MinWidth(ObjWidth)<<L"";
			continue;
		}
		bpos = vtell();
		if (Y == Y1+1)
			SecondPos = bpos;

		nr = 0;
		Reader.Read(line, nb, &nr);
		if (nr > mb)
			Reader.Unread(nr-mb);
		else
			LastPage = EndFile = veof() ? 1 : 0;

		txt_dump(VM.CodePage, line, nr, Width, OutStr, ZERO_CHAR);

		FS<<fmt::LeftAlign()<<fmt::MinWidth(ObjWidth)<<OutStr;
		if ( SelectSize > 0 && bpos < SelectPos+SelectSize && bpos+mb > SelectPos ) {
			int bsel = SelectPos > bpos ? (int)(SelectPos-bpos) / ch_size : 0;
			int esel = SelectPos+SelectSize < bpos+mb ? ((int)(SelectPos+SelectSize-bpos)+ch_size-1)/ch_size: Width;
			SetColor(COL_VIEWERSELECTEDTEXT);
			GotoXY(bsel, Y);
			FS<<fmt::MaxWidth(esel-bsel)<<OutStr+bsel;
		}
	}
}

void Viewer::ShowHex()
{
	wchar_t OutStr[128],TextStr[20];
	int EndFile;
	int X,Y,TextPos;
	int SelStart, SelEnd;
	bool bSelStartFound = false, bSelEndFound = false;
	__int64 HexLeftPos=((LeftPos>80-ObjWidth) ? Max(80-ObjWidth,0):LeftPos);

	const wchar_t BorderLine[] = {BoxSymbols[BS_V1],L' ',0};
	int border_len = (int)wcslen(BorderLine);

	for (LastPage=EndFile=0,Y=Y1; Y<=Y2; Y++)
	{
		bSelStartFound = false;
		bSelEndFound = false;
		__int64 SelSize=0;
		SetColor(COL_VIEWERTEXT);
		GotoXY(X1,Y);

		if (EndFile)
		{
			FS<<fmt::MinWidth(ObjWidth)<<L"";
			continue;
		}

		if (Y==Y1+1 && !veof())
			SecondPos=vtell();

		int out_len = _snwprintf(OutStr,ARRAYSIZE(OutStr),L"%010I64X: ", vtell());
		SelEnd = SelStart = out_len;
		TextPos=0;
		__int64 fpos = vtell();

		if (fpos > SelectPos)
			bSelStartFound = true;

		if (fpos < SelectPos+SelectSize-1)
			bSelEndFound = true;

		if ( SelectSize < 0 )
			bSelStartFound = bSelEndFound = false;

		unsigned char line[16+3];
		DWORD nr = 0;
		DWORD nb = CP_UTF8 == VM.CodePage ? 16+3 : 16;
		Reader.Read(line, nb, &nr);
		if (nr > 16)
			Reader.Unread(nr-16);
		else
			LastPage = EndFile = veof() ? 1 : 0;

		if (nr <= 0)
		{
			*OutStr = L'\0';
		}
		else
		{
			if (IsUnicodeCodePage(VM.CodePage))
			{
				int be = VM.CodePage == CP_REVERSEBOM ? 1 : 0;
				for (X=0; X<16; X += 2)
				{
					if (SelectSize >= 0 && (SelectPos == fpos || SelectPos == fpos+1))
					{
						bool half = SelectPos != fpos;
						bSelStartFound = true;
						SelStart = out_len + (half ? 1+be : 0);
						if ( 0 == (SelSize=SelectSize) )
							SelStart += (half ? be : 0) - 1;
					}
					if (SelectSize >= 0 && (fpos == SelectPos+SelectSize-1 || fpos+1 == SelectPos+SelectSize-1))
					{
						bool half = fpos == SelectPos+SelectSize-1;
						bSelEndFound = true;
						SelEnd = out_len+3 - (half ? 1+be : 0);
						if ( 0 == (SelSize=SelectSize) )
							SelEnd = SelStart;
					}
					else if ( SelectSize == 0 && (SelectPos == fpos || SelectPos == fpos+1) )
					{
						bSelEndFound = true;
						SelSize = 0;
						SelEnd = SelStart;
					}

					if ((DWORD)X < nr-1) // full character
					{
						unsigned ch = line[X+be] + (line[X+1-be] << 8);
						_snwprintf(OutStr+out_len, ARRAYSIZE(OutStr)-out_len, L"%04X ", ch);
						TextStr[TextPos++] = ch ? (wchar_t)ch : ZERO_CHAR;
					}
					else if ((DWORD)X == nr-1) // half character only
					{
						int o1 = 2 * (1 - be);
						_snwprintf(OutStr+out_len+o1, ARRAYSIZE(OutStr)-out_len-o1, L"%02X", line[X]);
						OutStr[out_len+2-o1] = OutStr[out_len+2-o1+1] = L'x';
						OutStr[out_len+4] = L' ';
						TextStr[TextPos++] = REPLACE_CHAR;
					}
					else // no character
					{
						wcscpy(OutStr+out_len, L"     ");
						TextStr[TextPos++] = L' ';
					}
					out_len += 5;

					if (X == 3*2)
					{
						wcscpy(OutStr+out_len, BorderLine);
						out_len += border_len;
					}
					fpos += 2;
				}
			}
			else
			{
				if ( SelectSize >= 0 )
				{
					if (SelectPos >= fpos && SelectPos < fpos+16)
					{
						int off = (int)(SelectPos - fpos);
						bSelStartFound = true;
						SelStart = out_len + 3*off + (off < 8 ? 0 : border_len);
						if ( 0 == (SelSize=SelectSize) )
							--SelStart;
					}
					__int64 selectEnd = SelectPos + SelectSize - 1;
					if (selectEnd >= fpos && selectEnd < fpos+16)
					{
						int off = (int)(selectEnd - fpos);
						bSelEndFound = true;
						SelEnd = (0 == (SelSize=SelectSize) ? SelStart : out_len + 3*off + (off < 8 ? 0 : border_len) + 1);
					}
					else if ( SelectSize == 0 && SelectPos == fpos )
					{
						bSelEndFound = true;
						SelSize = 0;
						SelEnd = SelStart;
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
				}
				out_len += 3*16 + border_len;

				txt_dump(VM.CodePage, line, nr, 16, TextStr, ZERO_CHAR);
				TextPos = 16;
			}
		}

		TextStr[TextPos] = L' ';
		TextStr[TextPos+1] = L'\0';

		if ((SelEnd <= SelStart) && bSelStartFound && bSelEndFound && SelectSize > 0 )
			SelEnd = out_len-2;

		OutStr[out_len] = L' ';
		wcscpy(OutStr+out_len+1, TextStr);

		if (StrLength(OutStr)>HexLeftPos)
		{
			FS<<fmt::LeftAlign()<<fmt::ExactWidth(ObjWidth)<<OutStr+static_cast<size_t>(HexLeftPos);
		}
		else
		{
			FS<<fmt::MinWidth(ObjWidth)<<L"";
		}

		if (bSelStartFound && bSelEndFound)
		{
			SetColor(COL_VIEWERSELECTEDTEXT);
			GotoXY((int)((__int64)X1+SelStart-HexLeftPos),Y);
			FS<<fmt::MaxWidth(SelEnd-SelStart+1)<<OutStr+static_cast<size_t>(SelStart);
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

		UINT x = X2 + (m_bQuickView ? 1 : 0);
		UINT h = Y2 - Y1 + 1;
		UINT64 start, end, total;

		if ( !VM.Hex )
		{
			total = static_cast<UINT64>(FileSize);
			start = static_cast<UINT64>(FilePos);
			ViewerString *last_line = Strings[Y2-Y1];
			end = last_line->nFilePos + last_line->linesize;
			if ( end == static_cast<UINT64>(FileSize) && last_line->linesize > 0 && last_line->have_eol )
				++total;
		}
		else
		{
			int lin_size = VM.Hex < 2 ? 16 : Width*getChSize(VM.CodePage);
			total = FileSize/lin_size + ((FileSize% lin_size) ? 1 : 0);
			start = FilePos /lin_size + ((FilePos % lin_size) ? 1 : 0);
			end = start + h;
		}
		ScrollBarEx3(x,Y1,h, start,end,total);
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

static bool is_word_div ( const wchar_t ch )
{
	static const wchar_t spaces[] = { L' ', L'\t', L'\n', L'\r', BOM_CHAR, REPLACE_CHAR, L'\0' };
	return ( !ch
		|| nullptr != wcschr(spaces, ch)
		|| nullptr != wcschr(Opt.strWordDiv, ch)
	);
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

void Viewer::ReadString( ViewerString *pString, int MaxSize, bool update_cache )
{
	AdjustWidth();

	int OutPtr = 0, nTab = 0, wrap_out = -1;
	wchar_t ch, eol_char = L'\0';
	INT64 fpos=0, fpos1, sel_end, wrap_pos = -1;
	bool skip_space = false;

	if ( !pString->lpData )
		pString->lpData = new wchar_t[MAX_VIEWLINEB];

	if (VM.Hex)
	{
		vseek(VM.Hex < 2 ? 16 : Width*getChSize(VM.CodePage), FILE_CURRENT);
		pString->lpData[OutPtr] = L'\0';
		LastPage = veof();
		return;
	}

	bool bSelStartFound = false, bSelEndFound = false;
	pString->bSelection = false;
	sel_end = SelectPos + SelectSize;

	fpos1 = vtell();
	for (;;)
	{
		fpos = fpos1;

		if (OutPtr >= MAX_VIEWLINE)
			break;

		if (--nTab >= 0)
			ch = L' ';
		else
		{
			if (!MaxSize-- || !vgetc(&ch))
				break;
		}
		fpos1 = vtell();

		if (SelectSize >= 0)
		{
			if (fpos == SelectPos || (fpos < SelectPos && fpos1 > SelectPos))
			{
				pString->nSelStart = OutPtr;
				bSelStartFound = true;
			}
			if (fpos == sel_end || (fpos < sel_end && fpos1 > sel_end))
			{
				pString->nSelEnd = OutPtr + (fpos < sel_end ? 1 : 0);
				bSelEndFound = true;
			}
		}

		if (!fpos && BOM_CHAR == ch)
		{
			continue; // skip BOM
		}
		else if (L'\t' == ch)
		{
			nTab = ViOpt.TabSize - (OutPtr % ViOpt.TabSize);
			continue;
		}
		if (L'\n' == ch || L'\r' == ch)
		{
			eol_char = ch;
			break;
		}

		pString->lpData[OutPtr++] = ch ? ch : ZERO_CHAR;
		if ( !VM.Wrap )
			continue;

		if ( VM.WordWrap && OutPtr <= Width && wrapped_char(ch) >= 0 )
		{
			wrap_out = OutPtr;
			wrap_pos = fpos1;
		}

		if ( OutPtr < Width )
			continue;
		if ( !VM.WordWrap )
			break;

		if ( OutPtr > Width )
		{
			if ( wrap_out <= 0 || is_space_or_nul(ch) )
			{
				wrap_out = OutPtr - 1;
				wrap_pos = fpos;
			}

			OutPtr = wrap_out;
			vseek(wrap_pos, SEEK_SET);
			while (OutPtr > 0 && is_space_or_nul(pString->lpData[OutPtr-1]))
				--OutPtr;

			if ( bSelEndFound && pString->nSelEnd > OutPtr )
				pString->nSelEnd = OutPtr;
			if ( bSelStartFound && pString->nSelStart >= OutPtr )
				bSelStartFound = bSelEndFound = false;

			skip_space = true;
			break;
		}
	}

   int eol_len = (eol_char ? 1 : 0);
	if (skip_space || eol_char != L'\n') // skip spaces and/or eol-s if required
	{
		for (;;)
		{
			vgetc(nullptr);
			int ib = vgetc_ib;
			if (!vgetc(&ch))
				break;

			if (skip_space && !eol_char && is_space_or_nul(ch))
				continue;

			if ( ch == L'\n' )
			{
				++eol_len;            // LF or CRLF
				assert(eol_len <= 2);
			}
			else if ( ch != L'\r' )	 // nor LF nor CR
			{
				vgetc_ib = ib;        // ungetc(1)
				assert(eol_len <= 1); // CR or unterminated
			}
			else                     // CR
			{
				eol_char = ch;
				if (++eol_len == 1)	 // single CR - continue
					continue;

				assert(eol_len == 2); // CRCR...
				if (vgetc(&ch) && ch == L'\n')
					++eol_len;         // CRCRLF
				else
					vgetc_ib = ib;     // CR ungetc(2)
			}
			break;
		}
	}

	pString->have_eol = eol_len;
	pString->lpData[(int)OutPtr]=0;
	pString->linesize = (int)(vtell() - pString->nFilePos);

	if ( update_cache )
		CacheLine(pString->nFilePos, pString->linesize, pString->have_eol != 0);

	if (SelectSize >= 0 && OutPtr > 0)
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


__int64 Viewer::EndOfScreen( int line )
{
	__int64 pos;

	if (!VM.Hex)
	{
		pos = Strings[Y2-Y1+line]->nFilePos + Strings[Y2-Y1+line]->linesize;
		if ( !line && !VM.Wrap && Strings[Y2-Y1]->linesize > 0 )
		{
			vseek(Strings[Y2-Y1]->nFilePos, SEEK_SET);
			int col = 0, rmargin = (int)LeftPos + Width;
			wchar_t ch;
			for (;;)
			{
				if ( !vgetc(&ch) )
					break;
				if ( ch == L'\n' || ch == L'\r' )
					break;
				if ( ch == L'\t' )
					col += ViOpt.TabSize - (col % ViOpt.TabSize);
				else
					++col;
				if ( col >= rmargin )
				{
					pos = vtell();
					break;
				}
			}
		}
	}
	else
		pos =	FilePos + (VM.Hex < 2 ? 16 : Width*getChSize(VM.CodePage)) * (Y2-Y1+1+line);

	if (pos < 0)
		pos = 0;
	else if (pos > FileSize)
		pos = FileSize;

	return pos;
}


__int64 Viewer::BegOfScreen()
{
	__int64 pos = FilePos;

	if (!VM.Hex && !VM.Wrap && LeftPos > 0)
	{
		vseek(FilePos, SEEK_SET);
		int col = 0;
		wchar_t ch;
		pos = -1;
		__int64 prev_pos;
		for (;;)
		{
			prev_pos = vtell();
			if ( !vgetc(&ch) )
				break;
			if ( ch == L'\n' || ch == L'\r' )
			{
				pos = Strings[1]->nFilePos;
				break;
			}
			if ( ch == L'\t' )
				col += ViOpt.TabSize - (col % ViOpt.TabSize);
			else
				++col;
			if ( col > LeftPos )	//!! шеврон закрывает первый символ
				break;				//!! при LeftPos=1 не видны 2 символа
		}
		if ( pos < 0 )
			pos = (col > LeftPos ? prev_pos : vtell());
	}

	return pos;
}


__int64 Viewer::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return (__int64)!FileSize;
		case MCODE_C_SELECTED:
			return (__int64)(SelectSize >= 0 ?TRUE:FALSE);
		case MCODE_C_EOF:
			return (__int64)(LastPage || !ViewFile.Opened());
		case MCODE_C_BOF:
			return (__int64)(!FilePos || !ViewFile.Opened());
		case MCODE_V_VIEWERSTATE:
		{
			DWORD MacroViewerState=0;
			MacroViewerState |= ViOpt.AutoDetectCodePage     ? 0x00000001 : 0; //autodetect
			MacroViewerState |= !ViOpt.AnsiCodePageAsDefault ? 0x00000002 : 0; //not use ANSI as default
			MacroViewerState |=                                0x00000004;     //? always UNICODE
			MacroViewerState |= VM.Wrap                      ? 0x00000008 : 0; //wrap mode
			MacroViewerState |= VM.WordWrap                  ? 0x00000010 : 0; //word wrap
			MacroViewerState |= VM.Hex == 1                  ? 0x00000020 : 0; //hex mode
			MacroViewerState |= VM.Hex  > 1                  ? 0x00000040 : 0; //dump mode -- !!!update help
			MacroViewerState |= Opt.OnlyEditorViewerUsed?0x08000000|0x00000800:0;
			MacroViewerState |= HostFileViewer && !HostFileViewer->GetCanLoseFocus()?0x00000800:0;
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
	if ( !ViOpt.PersistentBlocks &&
			Key!=KEY_IDLE && Key!=KEY_NONE && !(Key==KEY_CTRLINS||Key==KEY_RCTRLINS||Key==KEY_CTRLNUMPAD0||Key==KEY_RCTRLNUMPAD0) &&
			Key!=KEY_CTRLC && Key!=KEY_RCTRLC )
		SelectSize = -1;

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

	if (Key!=KEY_ALTBS && Key!=KEY_RALTBS && Key!=KEY_CTRLZ && Key!=KEY_RCTRLZ && Key!=KEY_NONE && Key!=KEY_IDLE)
		LastKeyUndo=FALSE;

	if (Key>=KEY_CTRL0 && Key<=KEY_CTRL9)
	{
		int Pos=Key-KEY_CTRL0;

		if (BMSavePos.FilePos[Pos]!=POS_NONE)
		{
			FilePos=BMSavePos.FilePos[Pos];
			LeftPos=BMSavePos.LeftPos[Pos];
			Show();
		}

		return TRUE;
	}

	if (Key>=KEY_CTRLSHIFT0 && Key<=KEY_CTRLSHIFT9)
		Key=Key-KEY_CTRLSHIFT0+KEY_RCTRL0;
	else if (Key>=KEY_RCTRLSHIFT0 && Key<=KEY_RCTRLSHIFT9)
		Key&=~KEY_SHIFT;

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
		case KEY_RCTRLU:
		{
			SelectSize = -1;
			Show();
			return TRUE;
		}
		case KEY_CTRLC:
		case KEY_RCTRLC:
		case KEY_CTRLINS:  case KEY_CTRLNUMPAD0:
		case KEY_RCTRLINS: case KEY_RCTRLNUMPAD0:
		{
			if (SelectSize >= 0 && ViewFile.Opened())
			{
				wchar_t *SelData = (wchar_t*)xf_malloc((size_t)SelectSize+1);
				if ( SelData )
				{
					__int64 CurFilePos=vtell();
					wmemset(SelData, 0, (size_t)SelectSize+1);
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
		case KEY_RCTRLS:
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
			if (Opt.ViewerEditorClock && HostFileViewer && HostFileViewer->IsFullScreen() && Opt.ViOpt.ShowTitleBar)
				ShowTime(FALSE);

			if (ViewFile.Opened() && update_check_period >= 0)
			{
				DWORD now_ticks = GetTickCount();
				if ((int)(now_ticks - last_update_check) < update_check_period)
					return TRUE;
				last_update_check = now_ticks;

				FAR_FIND_DATA_EX NewViewFindData;
				if (!apiGetFindDataEx(strFullFileName, NewViewFindData))
					return TRUE;

				// Smart file change check -- thanks Dzirt2005
				//
				bool changed = (
					ViewFindData.ftLastWriteTime.dwLowDateTime!=NewViewFindData.ftLastWriteTime.dwLowDateTime ||
					ViewFindData.ftLastWriteTime.dwHighDateTime!=NewViewFindData.ftLastWriteTime.dwHighDateTime ||
					ViewFindData.nFileSize != NewViewFindData.nFileSize
				);
				if ( changed )
					ViewFindData = NewViewFindData;
				else {
					if ( !ViewFile.GetSize(NewViewFindData.nFileSize) || FileSize == static_cast<__int64>(NewViewFindData.nFileSize) )
						return TRUE;
					changed = FileSize > static_cast<__int64>(NewViewFindData.nFileSize); // true if file shrank
				}

				SetFileSize();
				if ( changed ) // do not reset caches if file just enlarged [make sense on Win7, doesn't matter on XP]
				{
					Reader.Clear(); // иначе зачем вся эта возня?
					ViewFile.FlushBuffers();
					vseek(0, SEEK_CUR); // reset vgetc state
					lcache_ready = false; // reset start-lines cache
				}

				if (FilePos > FileSize)
				{
					ProcessKey(KEY_CTRLEND);
				}
				else
				{
					__int64 PrevLastPage=LastPage;
					LastPage = 0;
					Show();

					if (PrevLastPage && !LastPage)
					{
						ProcessKey(KEY_CTRLEND);
						LastPage=TRUE;
					}
				}
			}
			return TRUE;
		}
		case KEY_ALTBS:
		case KEY_RALTBS:
		case KEY_CTRLZ:
		case KEY_RCTRLZ:
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
					SavePosition();
					BMSavePos.Clear(); //Prepare for new file loading

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
			ProcessTypeWrapMode(!VM.WordWrap);
			return TRUE;
		}
		case KEY_F2:
		{
			ProcessWrapMode(!VM.Wrap);
			return TRUE;
		}
		case KEY_F4:
		{
			VM.Hex = VM.Hex != 1 ? 1 : (dump_text_mode ? 2 : 0);
			ProcessHexMode(VM.Hex);
			return TRUE;
		}
		case KEY_SHIFTF4:
		{
			MenuDataEx ModeListMenu[] = {
				MSG(MViewF4Text),0,0, // Text
				MSG(MViewF4),0,0,     // Hex
				MSG(MViewF4Dump),0,0  // Dump
			};
			int mode;
			{
				VMenu vModes(MSG(MViewMode),ModeListMenu,ARRAYSIZE(ModeListMenu),ScrY-4);
				vModes.SetFlags(VMENU_WRAPMODE | VMENU_AUTOHIGHLIGHT);
				vModes.SetPosition(-1,-1,0,0);
				vModes.SetSelectPos(VM.Hex, +1);
				vModes.Process();
				mode = vModes.Modal::GetExitCode();
			}
			if ( mode >= 0 && mode != VM.Hex )
			{
				if ( mode != 1 )
					dump_text_mode = (mode == 2);
				ProcessHexMode(VM.Hex = mode);
			}
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
		case KEY_RALTF7:
		{
			Search(-1,0);
			return TRUE;
		}
		case KEY_F8:
		{
			VM.CodePage = VM.CodePage==GetACP() ? GetOEMCP() : GetACP();
			lcache_ready = false;
			AdjustFilePos();
			ChangeViewKeyBar();
			Show();
			CodePageChangedByUser=TRUE;
			return TRUE;
		}
		case KEY_SHIFTF8:
		{
			UINT nCodePage = SelectCodePage(VM.CodePage, true, true, false, true);
			if (nCodePage != static_cast<UINT>(-1))
			{
				if (nCodePage == (CP_DEFAULT & 0xffff))
				{
					__int64 fpos = vtell();
					bool detect = GetFileFormat(ViewFile,nCodePage,&Signature,true) && IsCodePageSupported(nCodePage);
					vseek(fpos, SEEK_SET);
					if (!detect)
						nCodePage = Opt.ViOpt.AnsiCodePageAsDefault ? GetACP() : GetOEMCP();
				}
				CodePageChangedByUser=TRUE;
				VM.CodePage=nCodePage;
				lcache_ready = false;
				AdjustFilePos();
				ChangeViewKeyBar();
				Show();
			}

			return TRUE;
		}
		case KEY_ALTF8:
		case KEY_RALTF8:
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
			CtrlObject->Plugins->CommandsMenu(MODALTYPE_VIEWER,0,L"Viewer");
			Show();
			return TRUE;
		}
		/* $ 27.06.2001 VVM
		  + С альтом скролим по 1 */
		case KEY_MSWHEEL_UP:
		case(KEY_MSWHEEL_UP | KEY_ALT):
		case(KEY_MSWHEEL_UP | KEY_RALT):
		{
			int Roll = (Key & (KEY_ALT|KEY_RALT))?1:(int)Opt.MsWheelDeltaView;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_UP);

			return TRUE;
		}
		case KEY_MSWHEEL_DOWN:
		case(KEY_MSWHEEL_DOWN | KEY_ALT):
		case(KEY_MSWHEEL_DOWN | KEY_RALT):
		{
			int Roll = (Key & (KEY_ALT|KEY_RALT))?1:(int)Opt.MsWheelDeltaView;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_DOWN);

			return TRUE;
		}
		case KEY_MSWHEEL_LEFT:
		case(KEY_MSWHEEL_LEFT | KEY_ALT):
		case(KEY_MSWHEEL_LEFT | KEY_RALT):
		{
			int Roll = (Key & (KEY_ALT|KEY_RALT))?1:(int)Opt.MsHWheelDeltaView;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_LEFT);

			return TRUE;
		}
		case KEY_MSWHEEL_RIGHT:
		case(KEY_MSWHEEL_RIGHT | KEY_ALT):
		case(KEY_MSWHEEL_RIGHT | KEY_RALT):
		{
			int Roll = (Key & (KEY_ALT|KEY_RALT))?1:(int)Opt.MsHWheelDeltaView;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_RIGHT);

			return TRUE;
		}
		case KEY_UP: case KEY_NUMPAD8: case KEY_SHIFTNUMPAD8:
		{
			if (FilePos>0 && ViewFile.Opened())
			{
				Up(1); // LastPage = 0

				if (VM.Hex)
				{
					Show();
				}
				else
				{
					ShowPage(SHOW_UP);
					ViewerString *end = Strings[Y2-Y1];
					LastPage = end->nFilePos >= FileSize ||
						(!end->have_eol && end->nFilePos + end->linesize >= FileSize);
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
		case KEY_PGUP: case KEY_NUMPAD9: case KEY_SHIFTNUMPAD9: case KEY_CTRLUP: case KEY_RCTRLUP:
		{
			if (ViewFile.Opened())
			{
				Up(Y2-Y1);
				Show();
			}

			return TRUE;
		}
		case KEY_PGDN: case KEY_NUMPAD3:  case KEY_SHIFTNUMPAD3: case KEY_CTRLDOWN: case KEY_RCTRLDOWN:
		{
			if (LastPage || !ViewFile.Opened())
				return TRUE;

			FilePos = EndOfScreen(-1); // start of last screen line

			if (Key == KEY_CTRLDOWN || Key == KEY_RCTRLDOWN)
			{
				vseek(vString.nFilePos = FilePos, SEEK_SET);
				for (int i=Y1; i<=Y2; i++)
				{
					ReadString(&vString,-1);
					vString.nFilePos += vString.linesize;
				}

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
				if (VM.Hex == 1 && LeftPos > 80-Width)
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
		case KEY_CTRLLEFT:  case KEY_CTRLNUMPAD4:
		case KEY_RCTRLLEFT: case KEY_RCTRLNUMPAD4:
		{
			if (ViewFile.Opened())
			{
				if (VM.Hex)
				{
					int ch_size = VM.Hex < 2 ? 1 : getChSize(VM.CodePage);
					FilePos = FilePos > ch_size ? FilePos-ch_size : 0;
					FilePos -= FilePos % ch_size;
				}
				else
				{
					LeftPos = LeftPos > 20 ? LeftPos-20 : 0;
				}

				Show();
			}

			return TRUE;
		}
		case KEY_CTRLRIGHT:  case KEY_CTRLNUMPAD6:
		case KEY_RCTRLRIGHT: case KEY_RCTRLNUMPAD6:
		{
			if (ViewFile.Opened())
			{
				if (VM.Hex)
				{
					int ch_size = VM.Hex < 2 ? 1 : getChSize(VM.CodePage);
					FilePos -= FilePos % ch_size;
					FilePos = FilePos < FileSize-ch_size ? FilePos+ch_size : FileSize-1;
					FilePos -= FilePos % ch_size;
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
		case KEY_RCTRLSHIFTLEFT:   case KEY_RCTRLSHIFTNUMPAD4:
		{
			// Перейти на начало строк
			if (ViewFile.Opened())
			{
				LeftPos = 0;
				Show();
			}

			return TRUE;
		}
		case KEY_CTRLSHIFTRIGHT:     case KEY_CTRLSHIFTNUMPAD6:
		case KEY_RCTRLSHIFTRIGHT:    case KEY_RCTRLSHIFTNUMPAD6:
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
		case KEY_RCTRLHOME:   case KEY_RCTRLNUMPAD7:
		case KEY_HOME:        case KEY_NUMPAD7:   case KEY_SHIFTNUMPAD7:

			// Перейти на начало файла
			if (ViewFile.Opened())
				LeftPos=0;

		case KEY_CTRLPGUP:    case KEY_CTRLNUMPAD9:
		case KEY_RCTRLPGUP:   case KEY_RCTRLNUMPAD9:
			if (ViewFile.Opened())
			{
				FilePos=0;
				Show();
			}

			return TRUE;
		case KEY_CTRLEND:     case KEY_CTRLNUMPAD1:
		case KEY_RCTRLEND:    case KEY_RCTRLNUMPAD1:
		case KEY_END:         case KEY_NUMPAD1: case KEY_SHIFTNUMPAD1:

			// Перейти на конец файла
			if (ViewFile.Opened())
				LeftPos=0;

		case KEY_CTRLPGDN:    case KEY_CTRLNUMPAD3:
		case KEY_RCTRLPGDN:   case KEY_RCTRLNUMPAD3:

			if (ViewFile.Opened())
			{
				/* $ 15.08.2002 IS
				   Для обычного режима, если последняя строка не содержит перевод
				   строки, крутанем вверх на один раз больше - иначе визуально
				   обработка End (и подобных) на такой строке отличается от обработки
				   Down.
				*/
				int max_counter = Y2 - Y1;
				int ch_size = getChSize(VM.CodePage);

				if (VM.Hex)
				{
					int lin_siz = VM.Hex < 2 ? 16 : Width*ch_size;
					vseek(0,SEEK_END);
					FilePos = vtell();
					FilePos -= FilePos % ch_size;
					if ((FilePos % lin_siz) == 0)
						FilePos -= lin_siz * (Y2 - Y1 + 1);
					else
						FilePos -= (FilePos % lin_siz) + lin_siz * (Y2 - Y1);
					if (FilePos < 0)
						FilePos = 0;
				}
				else
				{
					wchar_t LastSym = L'\0';

					vseek(0, SEEK_END);
					FilePos = vtell() - 1;
					FilePos -= FilePos % ch_size;
					vseek(FilePos, SEEK_SET);
					if (vgetc(&LastSym) && LastSym != L'\n' && LastSym != L'\r')
						++max_counter;

					FilePos=vtell();
					Up(max_counter);
				}

				Show();
			}

			return TRUE;
		default:

			if (IsCharKey(Key))
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
	//SelectSize=0;

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
				//_SVS(SysLog(L"Viewer/ KEY_DOWN= %i, %i",FilePos,FileSize));
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
	  угу, а только если он есть, statusline...
	*/
	if (IntKeyState.MouseY == (Y1-1) && (HostFileViewer && HostFileViewer->IsTitleBarVisible()))
	{
		while (IsMouseButtonPressed()) {}
		if (IntKeyState.MouseY != Y1-1)
			return TRUE;

		int NameLen = Max(20, ObjWidth-40-(Opt.ViewerEditorClock && HostFileViewer && HostFileViewer->IsFullScreen() ? 3+5 : 0));
		wchar_t tt[10];
		int cp_len = wsprintf(tt, L"%u", VM.CodePage);
		//                           ViewMode     CopdePage             Goto
		static const int keys[]   = {KEY_SHIFTF4, KEY_SHIFTF8,          KEY_ALTF8   };
		int xpos[ARRAYSIZE(keys)] = {NameLen,     NameLen+3+(5-cp_len), NameLen+40-4};
		int xlen[ARRAYSIZE(keys)] = {3,           cp_len,                          4};

		for (int i = 0; i < static_cast<int>(ARRAYSIZE(keys)); ++i)
		{
			if (IntKeyState.MouseX >= xpos[i] && IntKeyState.MouseX < xpos[i]+xlen[i])
			{
				ProcessKey(keys[i]);
				return TRUE;
			}
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


void Viewer::CacheLine( __int64 start, int length, bool have_eol )
{
	assert(start >= 0 && length >= 0);
	if (!length) // empty lines beyond EOF
		return;

	if ( lcache_ready
	 && (lcache_wrap != VM.Wrap || lcache_wwrap != VM.WordWrap || lcache_width != Width)
	){
		lcache_ready = false;
	}

	if (!lcache_ready || start > lcache_last || start+length < lcache_first)
	{
		lcache_first = start;
		lcache_last = start + length;

		lcache_count = 2;
		lcache_base = 0;
		lcache_lines[0] = (have_eol ? -start : +start);
		lcache_lines[1] = start + length;

		lcache_wrap = VM.Wrap; lcache_wwrap = VM.WordWrap; lcache_width = Width;
		lcache_ready = true;
	}
	else if (start == lcache_last)
	{
		int i = (lcache_base + lcache_count - 1) % lcache_size;
		lcache_lines[i] = (have_eol ? -start : +start);
		i = (i + 1) % lcache_size;
		lcache_lines[i]	= lcache_last = start + length;
		if (lcache_count < lcache_size)
			++lcache_count;
		else
		{
			lcache_base = (lcache_base + 1) % lcache_size; // ++start
			lcache_first = _abs64(lcache_lines[lcache_base]);
		}
	}
	else if (start+length == lcache_first)
	{
		lcache_base = (lcache_base + lcache_size - 1) % lcache_size; // --start
		lcache_lines[lcache_base] = (have_eol ? -start : +start);
		lcache_first = start;
		if (lcache_count < lcache_size)
			++lcache_count;
		else
		{
			int i = (lcache_base + lcache_size - 1) % lcache_size; // i = start - 1
			lcache_last = _abs64(lcache_lines[i]);
		}
	}
	else
	{
		bool reset = (start < lcache_first || start+length > lcache_last);
		if ( reset )
		{
			int i = CacheFindUp(start+length);
			reset = (i < 0 || _abs64(lcache_lines[i]) != start);
			if ( !reset )
			{
				int j = (i + 1) % lcache_size;
				reset = (_abs64(lcache_lines[j]) != start+length);
			}
		}
#if defined(_DEBUG) && 1 // it is legal case if file changed...
		assert( !reset );
#endif
		if ( reset )
		{
			lcache_first = start;
			lcache_last = start + length;
			lcache_count = 2;
			lcache_base = 0;
			lcache_lines[0] = (have_eol ? -start : +start);
			lcache_lines[1] = start + length;
		}
	}
}

int Viewer::CacheFindUp( __int64 start )
{
	if ( lcache_ready
		&& (lcache_wrap != VM.Wrap || lcache_wwrap != VM.WordWrap || lcache_width != Width)
	){
		lcache_ready = false;
	}
	if ( !lcache_ready || start <= lcache_first || start > lcache_last )
		return -1;

	int i, j, i1 = 0, i2 = lcache_count - 1;
	for (;;)
	{
		if ( i1+1 >= i2 )
			return (lcache_base + i1) % lcache_size;

		i = (i1 + i2) / 2;
		j = (lcache_base + i) % lcache_size;
		if (_abs64(lcache_lines[j]) < start)
			i1 = i;
		else
			i2 = i;
	}
}

void Viewer::Up( int nlines )
{
	assert( nlines > 0 );

	if (!ViewFile.Opened())
		return;

	LastPage = 0;

	if (FilePos <= 0)
		return;

	if (VM.Hex)
	{
		int lin_siz = VM.Hex < 2 ? 16 : Width*getChSize(VM.CodePage);
		FilePos = FilePos > lin_siz*nlines ? FilePos-lin_siz*nlines : 0;
		return;
	}

	__int64 fpos, fpos1;

	int i = CacheFindUp(fpos = FilePos);
	if ( i >= 0 )
	{
		for (;;)
		{
			fpos = _abs64(lcache_lines[i]);
			if (--nlines == 0)
			{
				FilePos = fpos;
				return;
			}
			if (i == lcache_base)
				break;
			i = (i + lcache_size - 1) % lcache_size;
		}
	}

	const int portion_size = 256;

	union {
		char c1[portion_size];
		wchar_t c2[portion_size/(int)sizeof(wchar_t)];
	} buff;

	int j, buff_size, nr, ch_size = getCharSize(VM.CodePage);

	while ( nlines > 0 )
	{
		if ( fpos <= 0 )
		{
			FilePos = 0;
			return;
		}

		fpos1 = fpos;

		// backward CR-LF search
		//
		for ( j = 0; j < max_backward_size/portion_size; ++j )
		{
			buff_size = (fpos > (__int64)portion_size ? portion_size : (int)fpos);
			if ( buff_size <= 0 )
				break;
			fpos -= buff_size;
			vseek(fpos, SEEK_SET);

			if ( ch_size <= 1 )
			{
				DWORD nread = 0;
				Reader.Read(buff.c1, static_cast<DWORD>(buff_size), &nread);
				if ((nr = (int)nread) != buff_size)
				{
					return; //??? error handling
				}
				if ( 0 == j )
				{
					if ( nr > 0 && '\n' == buff.c1[nr-1] )          // LF
					{
						if ( --nr > 0 && '\r' == buff.c1[nr-1] )     // CRLF
							if ( --nr > 0 && '\r' == buff.c1[nr-1] )  // CRCRLF
								--nr;
		 			}
					else if ( nr > 0 && '\r' == buff.c1[nr-1] )     // CR
					{
						--nr;
					}
				}
				for ( i = nr-1; i >= 0; --i )
				{
					if ( '\n' == buff.c1[i] || '\r' == buff.c1[i] )
						break;
				}
				if ( i >= 0 )
				{
					fpos += i + 1;
					break;
				}
			}
			else
			{
				nr = vread(buff.c2, buff_size);
				if ( nr != buff_size / ch_size )
				{
					return; //??? error handling
				}
				if ( 0 == j )
				{
					if ( nr > 0 && L'\n' == buff.c2[nr-1] )	         // LF
					{
						if ( --nr > 0 && L'\r' == buff.c2[nr-1] )    // CRLF
							if ( --nr > 0 && L'\r' == buff.c2[nr-1] )	// CRCRLF
								--nr;
					}
					else if ( nr > 0 && L'\r' == buff.c2[nr-1] )    // CR
					{
						--nr;
					}
				}
				for ( i = nr-1; i >= 0; --i )
				{
					if ( L'\n' == buff.c2[i] || L'\r' == buff.c2[i] )
						break;
				}
				if ( i >= 0 )
				{
					fpos += ch_size * (i + 1);
					break;
				}
			}
		}

		// split read portion
		//
		vseek(vString.nFilePos = fpos, SEEK_SET);
		for (i = 0; i < llengths_size; ++i)
		{
			ReadString(&vString, -1, false);
			llengths[i] = (vString.have_eol ? -1 : +1) * vString.linesize;
			if ((vString.nFilePos += vString.linesize) >= fpos1)
			{
				fpos1 = vString.nFilePos;
				break;
			}
		}
		assert(i < llengths_size);
		if (i >= llengths_size)
			--i;

		while ( i >= 0 )
		{
			int l = llengths[i--];
			bool eol = false;
			if (l < 0)
			{
				eol = true;
				l = -l;
			}
			fpos1 -= l;
			CacheLine(fpos1, l, eol);
			if (--nlines == 0)
				FilePos = fpos1;
		}
	}
}


int Viewer::GetStrBytesNum( const wchar_t *Str, int Length )
{
	int ch_size = getCharSize(VM.CodePage);
	if (ch_size > 0)
		return Length * ch_size;
	else
		return WideCharToMultiByte(VM.CodePage,0, Str,Length, nullptr,0, nullptr,nullptr);
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

		ViewKeyBar->Change(MSG(VM.Hex != 1 ? MViewF4 : (dump_text_mode ? MViewF4Dump : MViewF4Text)), 3);

		if (VM.CodePage != GetOEMCP())
			ViewKeyBar->Change(MSG(MViewF8DOS),7);
		else
			ViewKeyBar->Change(MSG(MViewF8),7);

		ViewKeyBar->Redraw();
	}

	CtrlObject->Plugins->CurViewer=this; //HostFileViewer;
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
	DM_SDSETVISIBILITY = DM_USER + 1,
	DM_SDREXVISIBILITY = DM_USER + 2
};

struct MyDialogData
{
	Viewer      *viewer;
	bool edit_autofocus;
	bool       hex_mode;
	bool      recursive;
};

intptr_t WINAPI ViewerSearchDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
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
			int re = SendDlgMessage(hDlg, DM_GETCHECK,SD_CHECKBOX_REGEXP,0) == BSTATE_CHECKED;
			int ww = !Param1 && !re;
			SendDlgMessage(hDlg,DM_ENABLE,SD_CHECKBOX_WORDS,ToPtr(ww));
			SendDlgMessage(hDlg,DM_ENABLE,SD_CHECKBOX_REGEXP,ToPtr(!Param1));
			SendDlgMessage(hDlg, DM_SDREXVISIBILITY, re && !Param1, 0);
			return TRUE;
		}
		case DM_SDREXVISIBILITY:
		{
			int show = 1;
			if ( Param1 )
			{
				int tlen = (int)SendDlgMessage(hDlg, DM_GETTEXTPTR, SD_EDIT_TEXT, 0);
				show = 0;
				if ( tlen > 0 )
				{
					RegExp re;
					wchar_t t[128], *tmp = t;
					if (tlen > (int)ARRAYSIZE(t))
						tmp = new wchar_t[tlen];
					SendDlgMessage(hDlg, DM_GETTEXTPTR, SD_EDIT_TEXT, tmp);
					string sre = tmp;
					InsertRegexpQuote(sre);
					show = re.Compile(sre.CPtr(), OP_PERLSTYLE);
					if (tmp != t)
						delete[] tmp;
				}
			}
			SendDlgMessage(hDlg, DM_ENABLE, SD_TEXT_SEARCH, ToPtr(show));
			SendDlgMessage(hDlg, DM_ENABLE, SD_BUTTON_OK,   ToPtr(show));
			return TRUE;
		}
		case DN_KILLFOCUS:
		{
			if ( SD_EDIT_TEXT == Param1 || SD_EDIT_HEX == Param1 )
			{
				MyDialogData *my = (MyDialogData *)SendDlgMessage(hDlg, DM_GETITEMDATA, SD_EDIT_TEXT, 0);
				my->hex_mode = (SD_EDIT_HEX == Param1);
			}
			break;
		}
		case DN_BTNCLICK:
		{
			bool need_focus = false;
			MyDialogData *my = (MyDialogData *)SendDlgMessage(hDlg, DM_GETITEMDATA, SD_EDIT_TEXT, 0);
			int cradio = (my->hex_mode ? SD_RADIO_HEX : SD_RADIO_TEXT);

			if ((Param1 == SD_RADIO_TEXT || Param1 == SD_RADIO_HEX) && Param2)
			{
				need_focus = true;
				if ( Param1 != cradio)
				{
					bool new_hex = (Param1 == SD_RADIO_HEX);

					SendDlgMessage(hDlg, DM_ENABLEREDRAW, FALSE, 0);

					int sd_dst = new_hex ? SD_EDIT_HEX : SD_EDIT_TEXT;
					int sd_src = new_hex ? SD_EDIT_TEXT : SD_EDIT_HEX;

					EditorSetPosition esp;
					esp.CurPos = -1;
					SendDlgMessage(hDlg, DM_GETEDITPOSITION, sd_src, &esp);
					const wchar_t *ps = (const wchar_t *)SendDlgMessage(hDlg, DM_GETCONSTTEXTPTR, sd_src, 0);
					string strTo;
					my->viewer->SearchTextTransform(strTo, ps, !new_hex, esp.CurPos);

					SendDlgMessage(hDlg, DM_SETTEXTPTR, sd_dst, ToPtr((intptr_t)strTo.CPtr()));
					SendDlgMessage(hDlg, DM_SDSETVISIBILITY, new_hex, 0);
					if (esp.CurPos >= 0)
					{
						int p = esp.CurPos;
						if (SendDlgMessage(hDlg, DM_GETEDITPOSITION, sd_dst, &esp))
						{
							esp.CurPos = esp.CurTabPos = p;
							esp.LeftPos = 0;
							SendDlgMessage(hDlg, DM_SETEDITPOSITION, sd_dst, &esp);
						}
					}

					if (!strTo.IsEmpty())
					{
						int changed = (int)SendDlgMessage(hDlg, DM_EDITUNCHANGEDFLAG, sd_src, ToPtr(-1));
						SendDlgMessage(hDlg, DM_EDITUNCHANGEDFLAG, sd_dst, ToPtr(changed));
					}

					SendDlgMessage(hDlg,DM_ENABLEREDRAW,TRUE,0);
					my->hex_mode = new_hex;
					if ( !my->edit_autofocus )
						return TRUE;
				}
			}
			else if ( Param1 == SD_CHECKBOX_REGEXP )
			{
				SendDlgMessage(hDlg, DM_SDSETVISIBILITY, my->hex_mode, 0);
			}

			if ( my->edit_autofocus && !my->recursive )
			{
				if ( need_focus
				  || Param1 == SD_CHECKBOX_CASE
				  || Param1 == SD_CHECKBOX_WORDS
				  || Param1 == SD_CHECKBOX_REVERSE
				  || Param1 == SD_CHECKBOX_REGEXP
				){
					my->recursive = true;
					SendDlgMessage(hDlg, DM_SETFOCUS, my->hex_mode ? SD_EDIT_HEX : SD_EDIT_TEXT, 0);
					my->recursive = false;
				}
			}

			if (need_focus)
				return TRUE;
			else
				break;
		}
		case DN_EDITCHANGE:
		{
			if ( Param1 == SD_EDIT_TEXT && SendDlgMessage(hDlg,DM_GETCHECK,SD_CHECKBOX_REGEXP,0) == BSTATE_CHECKED )
				SendDlgMessage(hDlg, DM_SDREXVISIBILITY, 1, 0);
			break;
		}
		case DN_HOTKEY:
		{
			if (Param1==SD_TEXT_SEARCH)
			{
				MyDialogData *my = (MyDialogData *)SendDlgMessage(hDlg, DM_GETITEMDATA, SD_EDIT_TEXT, 0);
				SendDlgMessage(hDlg, DM_SETFOCUS, (my->hex_mode ? SD_EDIT_HEX : SD_EDIT_TEXT), 0);
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
	const wchar_t *name = (const wchar_t*)preRedrawItem.Param.Param1;
	int percent = (int)(intptr_t)preRedrawItem.Param.Param2;
	int search_hex = (int)(intptr_t)preRedrawItem.Param.Param3;
	ViewerSearchMsg(name, percent, search_hex);
}

void ViewerSearchMsg(const wchar_t *MsgStr, int Percent, int SearchHex)
{
	string strProgress;
	string strMsg(SearchHex?MSG(MViewSearchingHex):MSG(MViewSearchingFor));
	strMsg.Append(L" ").Append(MsgStr);
	if (Percent>=0)
	{
		FormatString strPercent;
		strPercent<<Percent;

		size_t PercentLength=Max(strPercent.GetLength(),(size_t)3);
		size_t Length=Max(Min(ScrX-1-10,static_cast<int>(strMsg.GetLength())),40)-PercentLength-2;
		wchar_t *Progress=strProgress.GetBuffer(Length);

		if (Progress)
		{
			size_t CurPos=Min(Percent,100)*Length/100;
			wmemset(Progress,BoxSymbols[BS_X_DB],CurPos);
			wmemset(Progress+(CurPos),BoxSymbols[BS_X_B0],Length-CurPos);
			strProgress.ReleaseBuffer(Length);
			strProgress+=FormatString()<<L" "<<fmt::MinWidth(PercentLength)<<strPercent<<L"%";;
		}

		TBC.SetProgressValue(Percent,100);
	}

	Message(MSG_LEFTALIGN,0,MSG(MViewSearchTitle),strMsg,strProgress.IsEmpty()?nullptr:strProgress.CPtr());
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	preRedrawItem.Param.Param1=(void*)MsgStr;
	preRedrawItem.Param.Param2=(LPVOID)(intptr_t)Percent;
	preRedrawItem.Param.Param3=(LPVOID)(intptr_t)SearchHex;
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

static int hex2ss(const wchar_t *from, char *c1, int mb, int *pos = 0)
{
	int nb, i, v, sub = 0, ps = 0, p0 = (pos ? *pos : -1), p1 = -1;
	wchar_t ch;

	nb = i = v = 0;
	for (;;)
	{
		ch = *from++;
		++ps;

		if      (ch >= L'0' && ch <= L'9') sub = L'0';
		else if (ch >= L'A' && ch <= L'F') sub = L'A' - 10;
		else if (ch >= L'a' && ch <= L'f') sub = L'a' - 10;
		else
		{
			if (i > 0)
			{
				if ( p0 >= 0 && ps > p0 && p1 < 0 )
					p1 = nb;
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
			if ( p0 >= 0 && ps > p0 && p1 < 0 )
				p1 = nb;
			c1[nb++] = (char)v;
			i = 0;
			if (nb >= mb)
				break;
		}
	}
#ifdef _DEBUG
	if (nb < mb) c1[nb] = '\0';
#endif
	if ( p1 >= 0 && pos )
		*pos = p1;
	return nb;
}

void Viewer::SearchTextTransform( UnicodeString &to, const wchar_t *from, bool hex2text, int &pos )
{
	int nb;
	char c1[128];
	wchar_t ch, ss[ARRAYSIZE(c1)+1];

	if (hex2text)
	{
		nb = hex2ss(from, c1, ARRAYSIZE(c1), &pos);

		if (IsUnicodeCodePage(VM.CodePage))
		{
			int v = CP_REVERSEBOM == VM.CodePage ? 1 : 0;
			if (nb & 1)
				c1[nb++] = '\0';

			for (int i = 0; i < nb; i += 2)
			{
				ch = MAKEWORD(c1[i+v], c1[i+1-v]);
				if (!ch)
					ch = 0xffff; // encode L'\0'
				to += ch;
			}
			if ( pos >= 0 )
				pos /= 2;
		}
		else
		{
			int nw = MultiByteToWideChar(VM.CodePage,0, c1,nb, ss,ARRAYSIZE(ss));
			if ( pos >= 0 )
			{
				pos = MultiByteToWideChar(VM.CodePage,0, c1,pos, NULL,0);
			}
			for (int i=0; i < nw; ++i)
				if (!ss[i])
					ss[i] = 0xffff;
			ss[nw] = L'\0';
			to = ss;
		}
	}
	else // text2hex
	{
		int ps = 0, pd = 0, p0 = pos, p1 = -1;
		while (*from)
		{
			if ( ps == p0 )
				p1 = pd;
			++ps;
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
					nb = WideCharToMultiByte(VM.CodePage,0, &ch,1, c1,4, NULL,NULL);
				break;
			}

			ss2hex(to, c1, nb, L'\0');
			pd += nb * 3;
		}
		pos = p1;
	}
}

struct Viewer::search_data
{
	INT64 CurPos;
	INT64 MatchPos;
	const char *search_bytes;
	const wchar_t *search_text;
	int search_len;
	int  ch_size;
	bool is_utf8;
	bool first_Rex;
	RegExp *pRex;

	search_data()
	{
		CurPos = MatchPos = -1;
		search_bytes = nullptr;
		search_text  = nullptr;
		search_len = ch_size = 0;
		is_utf8 = false;
		first_Rex = true;
		pRex = nullptr;
	}

	~search_data()
	{
		 delete pRex;
	}
};

int Viewer::search_hex_forward( search_data* sd )
{
	char *buff = (char *)Search_buffer;
	const char *search_str = sd->search_bytes;
	int bsize = Search_buffer_size * static_cast<int>(sizeof(wchar_t)), slen = sd->search_len;
	INT64 to, cpos = sd->CurPos;

	bool up_half = cpos >= StartSearchPos;
	if ( up_half )
	{
		if ( (to = FileSize) - cpos <= bsize )
			SetFileSize();
		to = FileSize;
	}
	else
	{
		to = StartSearchPos + slen - 1;
		if ( to > FileSize )
			to = FileSize;
	}

	int nb = (to - cpos < bsize ? static_cast<int>(to - cpos) : bsize);
	vseek(cpos, SEEK_SET);
	DWORD nr = 0;
	Reader.Read(buff, static_cast<DWORD>(nb), &nr);
	int n1 = static_cast<int>(nr);
	if ( n1 != nb )
		SetFileSize();

	if ( n1 < slen )
	{
		if ( !up_half || StartSearchPos <= 0 )
			return -1;
		sd->CurPos = 0;
		return 0;
	}

	char *ps = buff;
	while ( nullptr != (ps = static_cast<char *>(memchr(ps, search_str[0], n1-slen+1))) )
	{
		if ( slen <= 1 || 0 == memcmp(ps+1, search_str+1, slen-1) )
		{
			sd->MatchPos = cpos + static_cast<INT64>(ps - buff);
			return +1; // found
		}
		++ps;
		n1 = static_cast<int>(nr - (ps - buff));
	}

	sd->CurPos = cpos + nr - slen + 1;
	return (up_half || sd->CurPos < StartSearchPos ? 0 : -1);
}

int Viewer::search_hex_backward( search_data* sd )
{
	char *buff = (char *)Search_buffer;
	const char *search_str = sd->search_bytes;
	int bsize = Search_buffer_size * static_cast<int>(sizeof(wchar_t)), slen = sd->search_len;
	INT64 to, cpos = sd->CurPos;

	bool lo_half = cpos <= StartSearchPos;
	if ( lo_half )
	{
		to = 0;
	}
	else
	{
		to = StartSearchPos - slen + 1;
		if ( to < 0 )
			to = 0;
	}

	int nb = (cpos - to < bsize ? static_cast<int>(cpos - to) : bsize);
	vseek(cpos-nb, SEEK_SET);
	DWORD nr = 0;
	Reader.Read(buff, static_cast<DWORD>(nb), &nr);
	int n1 = static_cast<int>(nr);
	if ( n1 != nb )
	{
		SetFileSize();
		cpos = vtell();
	}

	if ( n1 < slen )
	{
		if ( !lo_half )
			return -1;
		SetFileSize();
		if ( StartSearchPos >= FileSize )
			return -1;
		sd->CurPos = FileSize;
		return 0;
	}

	for ( char *ps = buff + n1 - 1; ps >= buff + slen - 1; --ps )
	{
		if ( *ps == search_str[slen-1] )
		{
			if ( slen <= 1 || 0 == memcmp(ps-slen+1, search_str, slen-1) )
			{
				sd->MatchPos = static_cast<INT64>(cpos-n1+(ps-slen+1-buff));
				return +1;
			}
		}
	}

	sd->CurPos = cpos - nr + slen - 1;
	return (lo_half || sd->CurPos > StartSearchPos ? 0 : -1);
}

int Viewer::search_text_forward( search_data* sd )
{
	int bsize = 8192, slen = sd->search_len, ww = (LastSearchWholeWords ? 1 : 0);
	wchar_t prev_char = L'\0', *buff = Search_buffer, *t_buff = (sd->is_utf8 ? buff + bsize : nullptr);
	const wchar_t *search_str = sd->search_text;
	INT64 to1, to, cpos = sd->CurPos;

	vseek(cpos, SEEK_SET);
	if ( ww )
		prev_char = vgetc_prev();

	bool up_half = cpos >= StartSearchPos;
	if ( up_half )
	{
		if ( (to = FileSize) - cpos <= bsize )
			SetFileSize();
		to = FileSize;
	}
	else
	{
		to = StartSearchPos;
		if ( to > FileSize )
			to = FileSize;
	}

	int nb = (to - cpos > bsize ? bsize : static_cast<int>(to - cpos));
	int nw = vread(buff, nb, t_buff);
	to1 = vtell();
	if ( !up_half && nb + 3*(slen+ww) < bsize && !veof() )
	{
		int nw1 = vread(buff+nw, 3*(slen+ww), t_buff ? t_buff+nw : nullptr);
		nw1 = Max(nw1, slen+ww-1);
		nw += nw1;
		to1 = to + (t_buff ? GetStrBytesNum(t_buff, nw1) : sd->ch_size * nw1);
	}

	int is_eof = (to1 >= FileSize ? 1 : 0), iLast = nw - slen - ww + ww*is_eof;
	if ( !LastSearchCase )
		CharUpperBuff(buff, nw);

	for ( int i = 0; i <= iLast; ++i )
	{
		if ( ww )
		{
			if ( !is_word_div(i > 0 ? buff[i-1] : prev_char))
				continue;
			if ( !(i == iLast && is_eof) && !is_word_div(buff[i+slen]) )
				continue;
		}
		if ( buff[i] != search_str[0]
		 || (slen > 1 && buff[i+1] != search_str[1])
		 || (slen > 2 && 0 != wmemcmp(buff+i+2, search_str+2, slen-2))
		) continue;

		sd->MatchPos = cpos + GetStrBytesNum(t_buff, i);
		sd->search_len = GetStrBytesNum(t_buff+i, slen);
		return +1;
	}

	if ( up_half && is_eof )
	{
		sd->CurPos = 0;
		return (StartSearchPos > 0 ? 0 : -1);
	}
	else
	{
		if ( iLast < 0 || (!up_half && to1 > StartSearchPos) )
			return -1;
		sd->CurPos = to1 - GetStrBytesNum(t_buff+iLast+1, nw-iLast-1);
		return 0;
	}
}

int Viewer::search_text_backward( search_data* sd )
{
	int bsize = 8192, slen = sd->search_len, ww = (LastSearchWholeWords ? 1 : 0);
	wchar_t *buff = Search_buffer, *t_buff = (sd->is_utf8 ? buff + bsize : nullptr);
	const wchar_t *search_str = sd->search_text;
	INT64 to1, to, cpos = sd->CurPos;

	bool up_half = cpos > StartSearchPos;
	to = (up_half ? StartSearchPos : 0);

	int nb = (cpos - to > bsize ? bsize : static_cast<int>(cpos- to));
	if ( up_half && nb + 3*(slen+ww) < bsize && cpos > nb )
	{
		if ( sd->ch_size > 0 )
		{
			nb += sd->ch_size * (slen + ww - 1);
			if (cpos < nb)
				nb = static_cast<int>(cpos);
		}
		else
		{
			to1 = cpos - nb - 3*(slen + ww - 1);
			if (to1 < 0)
				to1 = 0;
			int nb1 = static_cast<int>(cpos - nb - to1);
			vseek(to1, SEEK_SET);
			int nw1 = vread(buff, nb1, t_buff);
			if ( nw1 > slen + ww - 1 )
				nb1 = GetStrBytesNum(t_buff + nw1 - (slen + ww - 1), slen + ww - 1);
			nb += nb1;
		}
	}

	cpos -= nb;
	vseek(cpos, SEEK_SET);
	int nw = vread(buff, nb, t_buff);
	if ( !LastSearchCase )
		CharUpperBuff(buff, nw);

	int is_eof = (veof() ? 1 : 0), iFirst = ww * (cpos > 0 ? 1 : 0), iLast = nw - slen - ww + ww*is_eof;
	for ( int i = iLast; i >= iFirst; --i )
	{
		if ( ww )
		{
			if ( i > 0 && !is_word_div(buff[i-1]) )
				continue;
			if ( !(i == iLast && is_eof) && !is_word_div(buff[i+slen]) )
				continue;
		}
		if ( buff[i] != search_str[0]
		|| (slen > 1 && buff[i+1] != search_str[1])
		|| (slen > 2 && 0 != wmemcmp(buff+i+2, search_str+2, slen-2))
		) continue;

		sd->MatchPos = cpos + GetStrBytesNum(t_buff, i);
		sd->search_len = GetStrBytesNum(t_buff+i, slen);
		return +1;
	}

	int ret = 0, adjust = 1;
	if ( up_half )
		ret = (cpos <= StartSearchPos ? -1 : 0);
	else
	{
		if ( cpos <= 0 )
		{
			adjust = 0;
			SetFileSize();
			cpos = FileSize;
			ret = (StartSearchPos >= FileSize ? -1 : 0);
		}
	}
	sd->CurPos = cpos + (ret ? 0 : adjust*GetStrBytesNum(t_buff,iFirst+slen-1));
	return ret;
}

int Viewer::read_line(wchar_t *buf, wchar_t *tbuf, INT64 cpos, int adjust, INT64 &lpos, int &lsize)
{
	int llen = 0;

	INT64 save_FilePos = FilePos, save_LastPage = LastPage;
	int save_Hex = VM.Hex, save_Wrap = VM.Wrap, save_WordWrap = VM.WordWrap;
	VM.Hex = VM.Wrap = VM.WordWrap = 0;

	FilePos = cpos;
	if ( adjust )
	{
		if ( adjust > 0 )
			AdjustFilePos();
		else
			Up(1);
	}

	vseek(lpos = vString.nFilePos = FilePos, SEEK_SET);
	vString.linesize = 0;
	ReadString(&vString, -1, false); // read unwrapped text line

	vseek(FilePos, SEEK_SET);
	llen = vread(buf, lsize = vString.linesize, tbuf);
	if ( llen > 0 )
		llen -= vString.have_eol; // remove eol-s
	buf[llen >= 0 ? llen : 0] = L'\0';

	VM.Hex = save_Hex; VM.Wrap = save_Wrap; VM.WordWrap = save_WordWrap;
	FilePos = save_FilePos; LastPage = save_LastPage;
	return llen;
}

int Viewer::search_regex_forward( search_data* sd )
{
	assert(sd->pRex);
	assert(Search_buffer_size >= 2*MAX_VIEWLINEB);

	wchar_t *line = Search_buffer, *t_line = sd->is_utf8 ? Search_buffer + MAX_VIEWLINEB : nullptr;
	INT64 cpos = sd->CurPos, bpos = 0;

	int first = (sd->first_Rex ? +1 : 0);
	sd->first_Rex = false;
	bool up_half = cpos >= StartSearchPos;

	int lsize = 0, nw = read_line(line, t_line, cpos, first, bpos, lsize);
	if ( lsize <= 0 )
	{
		sd->CurPos = 0;
		return (up_half && StartSearchPos > 0 ? 0 : -1);
	}

	int off = 0;
	for (;;)
	{
		if ( off > nw )
			break;

		SMatch m[1];
		int n = static_cast<int>(ARRAYSIZE(m));
		if ( !sd->pRex->SearchEx(line, line+off, line+nw, m, n) )  // doesn't match
			break;

		INT64 fpos = bpos + GetStrBytesNum(t_line, m[0].start);
		if ( fpos < cpos )
		{
			off = m[0].start + 1; // skip
			continue;
		}
		else if ( !up_half && fpos >= StartSearchPos )
		{
			break; // done - not found
		}
		else // found
		{
			sd->MatchPos = fpos;
			sd->search_len = GetStrBytesNum(t_line+off, m[0].end - m[0].start);
			return +1;
		}
	}

	sd->CurPos = vtell();
	if ( up_half )
	{
		if ( veof() )
		{
			sd->CurPos = 0;
			return (StartSearchPos > 0 ? 0 : -1);
		}
		return 0;
	}
	else
	{
		return (sd->CurPos >= StartSearchPos ? -1 : 0);
	}
}

int Viewer::search_regex_backward( search_data* sd )
{
	assert(sd->pRex);
	assert(Search_buffer_size >= 2*MAX_VIEWLINEB);

	wchar_t *line = Search_buffer, *t_line = sd->is_utf8 ? Search_buffer + MAX_VIEWLINEB : nullptr;
	INT64 cpos = sd->CurPos, bpos = 0, prev_pos = -1;

	bool up_half = cpos > StartSearchPos;

	int off=0, lsize=0, nw, prev_len = -1, flen;
	nw = read_line(line, t_line, cpos, -1, bpos, lsize);
	for (;;)
	{
		if ( lsize <= 0 || off > nw )
			break;

		SMatch m[1];
		int n = static_cast<int>(ARRAYSIZE(m));
		if ( !sd->pRex->SearchEx(line, line+off, line+nw, m, n) )
			break;

		INT64 fpos = bpos + GetStrBytesNum(t_line, m[0].start);
		flen = GetStrBytesNum(t_line + m[0].start, m[0].end - m[0].start);
		if ( fpos+flen > cpos )
			break;

		if ( !(up_half && fpos+flen <= StartSearchPos) )
		{
			prev_pos = fpos;
			prev_len = flen;
		}

		off = m[0].start + 1; // skip
		continue;
	}

	if ( prev_len >= 0 )
	{
		sd->MatchPos = prev_pos;
		sd->search_len = prev_len;
		return +1;
	}

	if ( (sd->CurPos = bpos) <= 0 )
	{
		SetFileSize();
		sd->CurPos = FileSize;
		return (StartSearchPos >= FileSize ? -1 : 0);
	}
	return (up_half && bpos <= StartSearchPos ? -1 : 0);
}


/*
 + Параметр Next может принимать значения:
 0 - Новый поиск
 1 - Продолжить поиск со следующей позиции
-1 - Продолжить поиск со следующей позиции в противоположном направлении
 2 - Продолжить поиск с начала/конца файла
*/
void Viewer::Search(int Next,int FirstChar)
{
	if (!ViewFile.Opened() || (Next && strLastSearchStr.IsEmpty()))
		return;

	string strSearchStr, strMsgStr;
	char search_bytes[128];
	bool Case,WholeWords,ReverseSearch,SearchRegexp,SearchHex;

	SearchHex = LastSearchHex;
	Case = LastSearchCase;
	WholeWords = LastSearchWholeWords;
	ReverseSearch = LastSearchReverse;
	SearchRegexp = LastSearchRegexp;
	strSearchStr.Clear();
	if (!strLastSearchStr.IsEmpty())
		strSearchStr = strLastSearchStr;

	search_data sd;
	int (Viewer::* searcher)( Viewer::search_data *p_sd ) = nullptr;

	if ( !Next )
	{
		const wchar_t *TextHistoryName=L"SearchText";
		const wchar_t *HexMask = L"HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH " ;
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

		SearchDlg[SD_RADIO_TEXT].Selected=!LastSearchHex;
		SearchDlg[SD_RADIO_HEX].Selected=LastSearchHex;
		SearchDlg[SD_CHECKBOX_CASE].Selected=LastSearchCase;
		SearchDlg[SD_CHECKBOX_WORDS].Selected=LastSearchWholeWords;
		SearchDlg[SD_CHECKBOX_REVERSE].Selected=LastSearchReverse;
		SearchDlg[SD_CHECKBOX_REGEXP].Selected=LastSearchRegexp;

		if (SearchDlg[SD_RADIO_HEX].Selected)
		{
			int nb = hex2ss(strSearchStr.CPtr(), search_bytes, ARRAYSIZE(search_bytes));
			strSearchStr.Clear();
			ss2hex(strSearchStr, search_bytes, nb, L'\0');
			SearchDlg[SD_EDIT_HEX].strData = strSearchStr;
		}
		else
			SearchDlg[SD_EDIT_TEXT].strData = strSearchStr;

		MyDialogData my;
		//
		my.viewer = this;
		my.edit_autofocus = (ViOpt.SearchEditFocus != 0);
		my.hex_mode = (LastSearchHex != 0);
		my.recursive = false;
		//
		SearchDlg[SD_EDIT_TEXT].UserData = (intptr_t)&my;

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

		SearchHex=SearchDlg[SD_RADIO_HEX].Selected == BSTATE_CHECKED;
		Case=SearchDlg[SD_CHECKBOX_CASE].Selected == BSTATE_CHECKED;
		WholeWords=SearchDlg[SD_CHECKBOX_WORDS].Selected == BSTATE_CHECKED;
		ReverseSearch=SearchDlg[SD_CHECKBOX_REVERSE].Selected == BSTATE_CHECKED;
		SearchRegexp=SearchDlg[SD_CHECKBOX_REGEXP].Selected == BSTATE_CHECKED;

		if (SearchHex)
		{
			strSearchStr = SearchDlg[SD_EDIT_HEX].strData;
			int len = hex2ss(strSearchStr.CPtr(), search_bytes, ARRAYSIZE(search_bytes));
			strSearchStr.Clear();
			ss2hex(strSearchStr, search_bytes, len, L' ');
		}
		else
		{
			strSearchStr = SearchDlg[SD_EDIT_TEXT].strData;
			size_t pos = 0;
			while (strSearchStr.Pos(pos, 0xffff))
				strSearchStr.Replace(pos, 1, L'\0');
		}
	}

	LastSearchCase = Case;
	LastSearchWholeWords = WholeWords;
	LastSearchReverse = ReverseSearch;
	LastSearchRegexp = SearchRegexp;

	if (Next == -1)
		ReverseSearch = !ReverseSearch;

	strMsgStr = strLastSearchStr = strSearchStr;

	sd.search_len = (int)strSearchStr.GetLength();
	if (true == (LastSearchHex = SearchHex))
	{
		sd.search_len = hex2ss(strSearchStr.CPtr(), search_bytes, ARRAYSIZE(search_bytes));
		sd.search_bytes = search_bytes;
		sd.ch_size = 1;
		Case = true;
		WholeWords = SearchRegexp = false;
		searcher = (ReverseSearch ? &Viewer::search_hex_backward : &Viewer::search_hex_forward);
	}
	else
	{
		sd.is_utf8 = VM.CodePage == CP_UTF8;
		sd.ch_size = getCharSize(VM.CodePage);
		sd.search_text = strSearchStr.CPtr();

		if ( SearchRegexp )
		{
			WholeWords = false;
			searcher = (ReverseSearch ? &Viewer::search_regex_backward : &Viewer::search_regex_forward);
			InsertRegexpQuote(strMsgStr);
			sd.pRex = new RegExp;
			string strSlash = strSearchStr;
			InsertRegexpQuote(strSlash);
			if ( !sd.pRex->Compile(strSlash, OP_PERLSTYLE | OP_OPTIMIZE | (Case ? 0 : OP_IGNORECASE)) )
				return; // wrong regular expression...
		}
		else
		{
			searcher = (ReverseSearch ? &Viewer::search_text_backward : &Viewer::search_text_forward);
			InsertQuote(strMsgStr);
		}
	}

	if (!Case && !SearchRegexp)
	{
		strSearchStr.Upper();
		sd.search_text = strSearchStr.CPtr();
	}

	int found = 0;

	int search_direction = ReverseSearch ? -1 : +1;
	switch (Next)
	{
		case 2:
			StartSearchPos = LastSelectPos = (ReverseSearch ? FileSize : 0);
		break;
		case +1: case -1:
			if ( SelectPos >= 0 && LastSelectSize >= 0 )
			{
				if (sd.ch_size >= 1)
					LastSelectPos = SelectPos + (ReverseSearch ? LastSelectSize-sd.ch_size : sd.ch_size);
				else
				{
					INT64 prev_pos = SelectPos;
					vseek(SelectPos, SEEK_SET);
					for (;;)
					{
						wchar_t ch;
						bool ok_getc = vgetc(&ch);
						LastSelectPos = vtell();
						if (!ReverseSearch || !ok_getc)
							break;
						if ( LastSelectPos >= SelectPos + LastSelectSize )
						{
							LastSelectPos = prev_pos;
							break;
						}
						prev_pos = LastSelectPos;
					}
				}
				if (search_direction != LastSearchDirection)
					StartSearchPos = LastSelectPos;
				else if ( LastSelectPos == StartSearchPos ) // боремся с
					found = -1;									  // зацикливанием

				break;
			} // else pass to case 0 (below)
		case 0:
			LastSelectSize = SelectSize = -1;
			StartSearchPos = LastSelectPos = (ReverseSearch ? EndOfScreen(0) : BegOfScreen());
		break;
	}
	LastSearchDirection = search_direction;

	if (!sd.search_len || (__int64)sd.search_len > FileSize)
		return;

	sd.CurPos = LastSelectPos;
	if ( !found )
	{
		TaskBar TB;
		TPreRedrawFuncGuard preRedrawFuncGuard(PR_ViewerSearchMsg);
		SetCursorType(FALSE,0);

		DWORD start_time = GetTickCount();
		for (;;)
		{
			found = (this->*searcher)(&sd);
			if ( found != 0 )
				break;

			DWORD cur_time = GetTickCount();
			if ( cur_time - start_time > (DWORD)Opt.RedrawTimeout )
			{
				start_time = cur_time;

				if (CheckForEscSilent())
				{
					if (ConfirmAbortOp())
					{
						Redraw();
						return;
					}
				}

				int percent = -1;
				INT64 total = FileSize;
				if ( total > 0 )
				{
					INT64 done;
					if ( !ReverseSearch )
					{
						if ( sd.CurPos >= StartSearchPos )
							done = sd.CurPos - StartSearchPos;
						else
							done = (FileSize - StartSearchPos) + sd.CurPos;
					}
					else
					{
						if ( sd.CurPos <= StartSearchPos )
							done = StartSearchPos - sd.CurPos;
						else
							done = StartSearchPos + (FileSize - sd.CurPos);
					}
					percent = static_cast<int>(done*100/total);
				}
				ViewerSearchMsg(strMsgStr, percent, SearchHex);
			}
		}
	}

	if ( sd.MatchPos >= 0 )
	{
		SelectText(sd.MatchPos, sd.search_len, ReverseSearch?0x2:0);
		LastSelectSize = SelectSize;

		// Покажем найденное на расстоянии четверти экрана от верха.
		int FromTop=(ScrY-(Opt.ViOpt.ShowKeyBar?2:1))/4;

		if (FromTop<0 || FromTop>ScrY)
			FromTop=0;

		Up(FromTop);

		AdjustSelPosition = TRUE;
		Show();
		AdjustSelPosition = FALSE;
	}
	else
	{
		Message(
			MSG_WARNING, 1,
			MSG(MViewSearchTitle),
			(SearchHex ? MSG(MViewSearchCannotFindHex) : MSG(MViewSearchCannotFind)),
			strMsgStr,
			MSG(MOk)
		);
	}
}


bool Viewer::GetWrapMode()
{
	return(VM.Wrap != 0);
}

void Viewer::SetWrapMode(bool Wrap)
{
	Viewer::VM.Wrap=Wrap;
}

void Viewer::EnableHideCursor(int HideCursor)
{
	Viewer::HideCursor=HideCursor;
}

bool Viewer::GetWrapType()
{
	return(VM.WordWrap != 0);
}

void Viewer::SetWrapType(bool TypeWrap)
{
	Viewer::VM.WordWrap=TypeWrap;
}

void Viewer::GetFileName(string &strName)
{
	strName = strFullFileName;
}

void Viewer::ShowConsoleTitle()
{
	string strViewerTitleFormat=Opt.strViewerTitleFormat.Get();
	ReplaceStrings(strViewerTitleFormat,L"%Lng",MSG(MInViewer),-1,true);
	ReplaceStrings(strViewerTitleFormat,L"%File",PointToName(strFileName),-1,true);
	ConsoleTitle::SetFarTitle(strViewerTitleFormat);
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
	AdjustFilePos();
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
		unsigned char c1 = ((const unsigned char *)s)[ic++];

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
			unsigned char c2 = ((const unsigned char *)s)[ic];
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
				unsigned char c3 = ((const unsigned char *)s)[ic+1];
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

					unsigned char c4 = ((const unsigned char *)s)[ic+2];
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
		int rev = (CP_REVERSEBOM == VM.CodePage ? 1 : 0);

		Reader.Read(Buf, Count, &ReadSize);
		if (0 != (ReadSize & 1))
		{
			((char *)Buf)[ReadSize-1+rev] = (char)(REPLACE_CHAR & 0xff);
			((char *)Buf)[ReadSize-0-rev] = (char)(REPLACE_CHAR >> 8);
			++ReadSize;
		}
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
	if (FILE_CURRENT == Whence)
	{
		if (vgetc_ready)
		{
			int tail = vgetc_cb - vgetc_ib;
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
			if (vgetc_ib == vgetc_cb-1)
				*pCh = REPLACE_CHAR;
			else
				*pCh = (wchar_t)((vgetc_buffer[vgetc_ib] << 8) | vgetc_buffer[vgetc_ib+1]);
			vgetc_ib += 2;
		break;
		case CP_UNICODE:
			if (vgetc_ib == vgetc_cb-1)
				*pCh = REPLACE_CHAR;
			else
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

wchar_t Viewer::vgetc_prev()
{
	INT64 pos = vtell();
	if ( pos <= 0 )
		return L'\0';

	int ch_size = getCharSize(VM.CodePage);
	if ( pos < ch_size )
		return REPLACE_CHAR;

	int nb = (ch_size >= 1 ? ch_size : (pos > 4 ? 4 : static_cast<int>(pos)));
	DWORD nr = 0;

	char ss[4];
	if ( vseek(-nb, SEEK_CUR) )
		 Reader.Read(ss, static_cast<DWORD>(nb), &nr);

	vseek(pos, SEEK_SET);

	wchar_t ch = REPLACE_CHAR;
	if ( static_cast<int>(nr) == nb )
	{
		switch ( VM.CodePage )
		{
			case CP_REVERSEBOM:
				ch = MAKEWORD(ss[1], ss[0]);
			break;
			case CP_UNICODE:
				ch = MAKEWORD(ss[0], ss[1]);
			break;
			case CP_UTF8:
			{
				int tail = 0;
				wchar_t w[4];
				int nw = utf8_to_WideChar(ss, nb, w,nullptr, 4,tail);
				if ( !tail && nw > 0 )
					ch = w[nw-1];
				break;
			}
			default:
				MultiByteToWideChar(VM.CodePage, 0, (LPCSTR)ss,1, &ch,1);
			break;
		}
	}
	return ch;
}


#define RB_PRC 3
#define RB_HEX 4
#define RB_DEC 5

void Viewer::GoTo(int ShowDlg,__int64 Offset, UINT64 Flags)
{
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

	static int PrevMode = -1;
	if ( PrevMode < 0 )
		PrevMode = (VM.Hex == 1 ? RB_HEX : RB_DEC);

	GoToDlg[RB_PRC].Selected = GoToDlg[RB_HEX].Selected = GoToDlg[RB_DEC].Selected = 0;
	GoToDlg[PrevMode].Selected = 1;
	{
		__int64 Relative=0;
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
				GoToDlg[RB_HEX].Selected = GoToDlg[RB_DEC].Selected = 0;
				GoToDlg[RB_PRC].Selected = 1;
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
				PrevMode = RB_PRC;
				int Percent=_wtoi(GoToDlg[1].strData);

				//if ( Relative  && (cPercent+Percent*Relative<0) || (cPercent+Percent*Relative>100)) // за пределы - низя
				//  return;
				if (Percent>100)
					return;

				//if ( Percent<0 )
				//  Percent=0;
				Offset=FileSize/100*Percent;

				while (ToPercent64(Offset,FileSize)<Percent)
					Offset++;
			}

			if (GoToDlg[RB_HEX].Selected)
			{
				PrevMode = RB_HEX;
				swscanf(GoToDlg[1].strData,L"%I64x",&Offset);
			}

			if (GoToDlg[RB_DEC].Selected)
			{
				PrevMode = RB_DEC;
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

				while (ToPercent64(Offset,FileSize)<Percent)
					Offset++;
			}
		}

		FilePos = (Relative ? FilePos + Offset*Relative : Offset);
		FilePos = (FilePos < 0 ? 0 : (FilePos > FileSize ? FileSize : FilePos));
	}
	AdjustFilePos();

	if (!(Flags&VSP_NOREDRAW))
		Show();
}

void Viewer::AdjustFilePos()
{
	wchar_t ch;

	if (!VM.Hex)
	{
		int ch_size = getCharSize(VM.CodePage);
		FilePos -= FilePos % ch_size;

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
		Up(1);
	}
}

void Viewer::SetFileSize()
{
	if (!ViewFile.Opened())
		return;

	UINT64 uFileSize=0; // BUGBUG, sign
	ViewFile.GetSize(uFileSize);
	FileSize=uFileSize;
}


void Viewer::GetSelectedParam(__int64 &Pos, __int64 &Length, DWORD &Flags)
{
	Pos=SelectPos;
	Length=SelectSize;
	Flags=SelectFlags;
}

// Flags=0x01 - показывать [делать Show()]
//       0x02 - "обратный поиск" ?
//
void Viewer::SelectText(const __int64 &match_pos,const __int64 &search_len, const DWORD flags)
{
	if (!ViewFile.Opened())
		return;

	SelectPos = match_pos;
	SelectSize = search_len;
	SelectFlags = flags;
	if ( SelectSize < 0 )
		return;

	if ( VM.Hex )
	{
		int lin_siz = VM.Hex < 2 ? 16 : Width * getChSize(VM.CodePage);

		FilePos = (FilePos % lin_siz) + lin_siz*(SelectPos / lin_siz);
		FilePos = (FilePos < SelectPos ? FilePos : (FilePos > lin_siz ? FilePos-lin_siz : 0));
	}
	else
	{
		FilePos = SelectPos;
		Up(1);
		LeftPos = 0;

		if ( !VM.Wrap )
		{
			vseek(vString.nFilePos = FilePos, SEEK_SET);
			vString.lpData[0] = L'\0';
			ReadString(&vString, (int)(SelectPos-FilePos), false);

			if ( !vString.have_eol )
			{
				int found_offset = (int)wcslen(vString.lpData);
				if ( found_offset > Width-10 )
					LeftPos = (Width <= 10 ? found_offset : found_offset + 10 - Width);
			}
		}
	}

	if (0 != (flags & 1))
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
			ViewerInfo *Info=(ViewerInfo *)Param;
			if (CheckStructSize(Info))
			{
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

				GoTo(FALSE, vsp->StartPos, vsp->Flags);

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
				SelectSize = -1;
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
				if ((intptr_t)Param != (intptr_t)-1) // не только перерисовать?
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
	VM.Hex=newMode % 3;

	if (!VM.Hex)
		AdjustFilePos();

	if (isRedraw)
	{
		ChangeViewKeyBar();
		Show();
	}

	return oldHex;
}

int Viewer::ProcessWrapMode(int newMode, bool isRedraw)
{
	int oldWrap=VM.Wrap;
	VM.Wrap=newMode&1;

	if (VM.Wrap)
		LeftPos = 0;

	if (!VM.Hex)
		AdjustFilePos();

	if (isRedraw)
	{
		ChangeViewKeyBar();
		Show();
	}

	Opt.ViOpt.ViewerIsWrap = VM.Wrap != 0;
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

	if (!VM.Hex)
		AdjustFilePos();

	if (isRedraw)
	{
		ChangeViewKeyBar();
		Show();
	}

	Opt.ViOpt.ViewerWrap = VM.WordWrap != 0;
	return oldTypeWrap;
}
