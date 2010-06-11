/*
editor.cpp

Редактор
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

#include "editor.hpp"
#include "edit.hpp"
#include "keyboard.hpp"
#include "lang.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "chgprior.hpp"
#include "filestr.hpp"
#include "dialog.hpp"
#include "fileedit.hpp"
#include "savescr.hpp"
#include "scrbuf.hpp"
#include "farexcpt.hpp"
#include "TPreRedrawFunc.hpp"
#include "syslog.hpp"
#include "TaskBar.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "xlat.hpp"
#include "datetime.hpp"
#include "stddlg.hpp"
#include "strmix.hpp"
#include "palette.hpp"
#include "FarDlgBuilder.hpp"

static int ReplaceMode,ReplaceAll;

static int EditorID=0;

// EditorUndoData
enum {UNDO_EDIT=1,UNDO_INSSTR,UNDO_DELSTR,UNDO_BEGIN,UNDO_END};

Editor::Editor(ScreenObject *pOwner,bool DialogUsed):
	UndoPos(nullptr),
	UndoSavePos(nullptr),
	UndoSkipLevel(0),
	LastChangeStrPos(0),
	NumLastLine(0),
	NumLine(0),
	Pasting(0),
	BlockStart(nullptr),
	BlockStartLine(0),
	VBlockStart(nullptr),
	MaxRightPos(0),
	LastSearchCase(GlobalSearchCase),
	LastSearchWholeWords(GlobalSearchWholeWords),
	LastSearchReverse(GlobalSearchReverse),
	LastSearchSelFound(Opt.EdOpt.SearchSelFound),
	LastSearchRegexp(Opt.EdOpt.SearchRegexp),
	m_codepage(CP_OEMCP),
	StartLine(-1),
	StartChar(-1),
	StackPos(0),
	NewStackPos(FALSE),
	EditorID(::EditorID++),
	HostFileEditor(nullptr)
{
	_KEYMACRO(SysLog(L"Editor::Editor()"));
	_KEYMACRO(SysLog(1));
	Opt.EdOpt.CopyTo(EdOpt);
	SetOwner(pOwner);

	if (DialogUsed)
		Flags.Set(FEDITOR_DIALOGMEMOEDIT);

	/* $ 26.10.2003 KM
	   Если установлен глобальный режим поиска 16-ричных кодов, тогда
	   сконвертируем GlobalSearchString в строку, ибо она содержит строку в
	   16-ричном представлении.
	*/
	if (GlobalSearchHex)
		Transform(strLastSearchStr,strGlobalSearchString,L'S');
	else
		strLastSearchStr = strGlobalSearchString;

	UnmarkMacroBlock();
	/* $ 12.01.2002 IS
	   По умолчанию конец строки так или иначе равен \r\n, поэтому нечего
	   пудрить мозги, пропишем его явно.
	*/
	wcscpy(GlobalEOL,DOS_EOL_fmt);
	memset(&SavePos,0xff,sizeof(SavePos));
	InsertString(nullptr, 0);
}


Editor::~Editor()
{
	//_SVS(SysLog(L"[%p] Editor::~Editor()",this));
	FreeAllocatedData();
	KeepInitParameters();
	_KEYMACRO(SysLog(-1));
	_KEYMACRO(SysLog(L"Editor::~Editor()"));
}

void Editor::FreeAllocatedData(bool FreeUndo)
{
	while (EndList)
	{
		Edit *Prev=EndList->m_prev;
		delete EndList;
		EndList=Prev;
	}

	UndoData.Clear();
	UndoSavePos=nullptr;
	UndoPos=nullptr;
	UndoSkipLevel=0;
	ClearStackBookmarks();
	TopList=EndList=CurLine=nullptr;
	NumLastLine = 0;
}

void Editor::KeepInitParameters()
{
	// Установлен глобальный режим поиска 16-ричных данных?
	if (GlobalSearchHex)
		Transform(strGlobalSearchString,strLastSearchStr,L'X');
	else
		strGlobalSearchString = strLastSearchStr;

	GlobalSearchCase=LastSearchCase;
	GlobalSearchWholeWords=LastSearchWholeWords;
	GlobalSearchReverse=LastSearchReverse;
	Opt.EdOpt.SearchSelFound=LastSearchSelFound;
	Opt.EdOpt.SearchRegexp=LastSearchRegexp;
}

/*
	преобразование из буфера в список
*/
int Editor::SetRawData(const wchar_t *SrcBuf,int SizeSrcBuf,int TextFormat)
{
#if defined(PROJECT_DI_MEMOEDIT)
	//InsertString(const wchar_t *lpwszStr, int nLength, Edit *pAfter)
	TextChanged(1);

#endif
	return TRUE;
}

/*
  Editor::Edit2Str - преобразование из списка в буфер с учетом EOL

    DestBuf     - куда сохраняем (выделяется динамически!)
    SizeDestBuf - размер сохранения
    TextFormat  - тип концовки строк
*/
int Editor::GetRawData(wchar_t **DestBuf,int& SizeDestBuf,int TextFormat)
{
#if defined(PROJECT_DI_MEMOEDIT)
	wchar_t* PDest=nullptr;
	SizeDestBuf=0; // общий размер = 0

	const wchar_t *SaveStr, *EndSeq;

	int Length;

	// посчитаем количество строк и общий размер памяти (чтобы не дергать realloc)
	Edit *CurPtr=TopList;

	DWORD AllLength=0;

	while (CurPtr)
	{
		CurPtr->GetBinaryString(&SaveStr,&EndSeq,Length);
		AllLength+=Length+StrLength(!TextFormat?EndSeq:GlobalEOL)+1;
	}

	wchar_t * MemEditStr=reinterpret_cast<wchar_t*>(xf_malloc((AllLength+8)*sizeof(wchar_t)));

	if (MemEditStr)
	{
		*MemEditStr=0;
		PDest=MemEditStr;

		// прйдемся по списку строк
		CurPtr=TopList;

		AllLength=0;

		while (CurPtr)
		{
			CurPtr->GetBinaryString(&SaveStr,&EndSeq,Length);
			wmemcpy(PDest,SaveStr,Length);
			PDest+=Length;

			size_t LenEndSeq;
			if (!TextFormat)
			{
				LenEndSeq=StrLength(EndSeq);
				wmemcpy(PDest,EndSeq,LenEndSeq);
			}
			else
			{
				LenEndSeq=StrLength(GlobalEOL);
				wmemcpy(PDest,GlobalEOL,LenEndSeq);
			}

			PDest+=LenEndSeq;

			AllLength+=LenEndSeq+Length;

			CurPtr=CurPtr->m_next;
		}

		*PDest=0;

		SizeDestBuf=AllLength;
		DestBuf=&MemEditStr;
		return TRUE;
	}
	else
		return FALSE;

#else
	return TRUE;
#endif
}


void Editor::DisplayObject()
{
	ShowEditor(FALSE);
}


void Editor::ShowEditor(int CurLineOnly)
{
	if (Locked() || !TopList)
		return;

	Edit *CurPtr;
	int LeftPos,CurPos,Y;

//_SVS(SysLog(L"Enter to ShowEditor, CurLineOnly=%i",CurLineOnly));
	/*$ 10.08.2000 skv
	  To make sure that CurEditor is set to required value.
	*/
	if (!Flags.Check(FEDITOR_DIALOGMEMOEDIT))
		CtrlObject->Plugins.CurEditor=HostFileEditor; // this;

	XX2=X2-(EdOpt.ShowScrollBar?1:0);
	/* 17.04.2002 skv
	  Что б курсор не бегал при Alt-F9 в конце длинного файла.
	  Если на экране есть свободное место, и есть текст сверху,
	  перепозиционируем.
	*/

	if (!EdOpt.AllowEmptySpaceAfterEof)
	{
		while (CalcDistance(TopScreen,nullptr,Y2-Y1)<Y2-Y1)
		{
			if (TopScreen->m_prev)
				TopScreen=TopScreen->m_prev;
			else
				break;
		}
	}

	/*
	  если курсор удруг оказался "за экраном",
	  подвинем экран под курсор, а не
	  курсор загоним в экран.
	*/

	while (CalcDistance(TopScreen,CurLine,-1)>=Y2-Y1+1)
	{
		TopScreen=TopScreen->m_next;
		//DisableOut=TRUE;
		//ProcessKey(KEY_UP);
		//DisableOut=FALSE;
	}

	CurPos=CurLine->GetTabCurPos();

	if (!EdOpt.CursorBeyondEOL)
	{
		MaxRightPos=CurPos;
		int RealCurPos=CurLine->GetCurPos();
		int Length=CurLine->GetLength();

		if (RealCurPos>Length)
		{
			CurLine->SetCurPos(Length);
			CurLine->SetLeftPos(0);
			CurPos=CurLine->GetTabCurPos();
		}
	}

	if (!Pasting)
	{
		/*$ 10.08.2000 skv
		  Don't send EE_REDRAW while macro is being executed.
		  Send EE_REDRAW with param=2 if text was just modified.

		*/
		_SYS_EE_REDRAW(CleverSysLog Clev(L"Editor::ShowEditor()"));

		if (!ScrBuf.GetLockCount())
		{
			/*$ 13.09.2000 skv
			  EE_REDRAW 1 and 2 replaced.
			*/
			if (Flags.Check(FEDITOR_JUSTMODIFIED))
			{
				Flags.Clear(FEDITOR_JUSTMODIFIED);

				if (!Flags.Check(FEDITOR_DIALOGMEMOEDIT))
				{
					_SYS_EE_REDRAW(SysLog(L"Call ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE)"));
					CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);
				}
			}
			else
			{
				if (!Flags.Check(FEDITOR_DIALOGMEMOEDIT))
				{
					_SYS_EE_REDRAW(SysLog(L"Call ProcessEditorEvent(EE_REDRAW,%s)",(CurLineOnly?"EEREDRAW_LINE":"EEREDRAW_ALL")));
					CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,CurLineOnly?EEREDRAW_LINE:EEREDRAW_ALL);
				}
			}
		}
		_SYS_EE_REDRAW(else SysLog(L"ScrBuf Locked !!!"));
	}

	DrawScrollbar();

	if (!CurLineOnly)
	{
		LeftPos=CurLine->GetLeftPos();
#if 0

		// крайне эксперементальный кусок!
		if (CurPos+LeftPos < XX2)
			LeftPos=0;
		else if (CurLine->X2 < XX2)
			LeftPos=CurLine->GetLength()-CurPos;

		if (LeftPos < 0)
			LeftPos=0;

#endif

		for (CurPtr=TopScreen,Y=Y1; Y<=Y2; Y++)
			if (CurPtr)
			{
				CurPtr->SetEditBeyondEnd(TRUE);
				CurPtr->SetPosition(X1,Y,XX2,Y);
				//CurPtr->SetTables(UseDecodeTable ? &TableSet:nullptr);
				//_D(SysLog(L"Setleftpos 3 to %i",LeftPos));
				CurPtr->SetLeftPos(LeftPos);
				CurPtr->SetTabCurPos(CurPos);
				CurPtr->FastShow();
				CurPtr->SetEditBeyondEnd(EdOpt.CursorBeyondEOL);
				CurPtr=CurPtr->m_next;
			}
			else
			{
				SetScreen(X1,Y,XX2,Y,L' ',COL_EDITORTEXT); //Пустые строки после конца текста
			}
	}

	CurLine->SetOvertypeMode(Flags.Check(FEDITOR_OVERTYPE));
	CurLine->Show();

	if (VBlockStart && VBlockSizeX>0 && VBlockSizeY>0)
	{
		int CurScreenLine=NumLine-CalcDistance(TopScreen,CurLine,-1);
		LeftPos=CurLine->GetLeftPos();

		for (CurPtr=TopScreen,Y=Y1; Y<=Y2; Y++)
		{
			if (CurPtr)
			{
				if (CurScreenLine>=VBlockY && CurScreenLine<VBlockY+VBlockSizeY)
				{
					int BlockX1=VBlockX-LeftPos+X1;
					int BlockX2=VBlockX+VBlockSizeX-1-LeftPos+X1;

					if (BlockX1<X1)
						BlockX1=X1;

					if (BlockX2>XX2)
						BlockX2=XX2;

					if (BlockX1<=XX2 && BlockX2>=X1)
						ChangeBlockColor(BlockX1,Y,BlockX2,Y,COL_EDITORSELECTEDTEXT);
				}

				CurPtr=CurPtr->m_next;
				CurScreenLine++;
			}
		}
	}

	if (HostFileEditor) HostFileEditor->ShowStatus();

//_SVS(SysLog(L"Exit from ShowEditor"));
}


/*$ 10.08.2000 skv
  Wrapper for Modified.
  Set JustModified every call to 1
  to track any text state change.
  Even if state==0, this can be
  last UNDO.
*/
void Editor::TextChanged(int State)
{
	Flags.Change(FEDITOR_MODIFIED,State);
	Flags.Set(FEDITOR_JUSTMODIFIED);
}


bool Editor::CheckLine(Edit* line)
{
	if (line)
	{
		Edit* eLine;

		for (eLine=TopList; eLine; eLine=eLine->m_next)
		{
			if (eLine == line)
				return true;
		}
	}

	return false;
}

int Editor::BlockStart2NumLine(int *Pos)
{
	if (BlockStart || VBlockStart)
	{
		Edit *eBlock=VBlockStart?VBlockStart:BlockStart;

		if (Pos)
		{
			if (VBlockStart)
				*Pos=eBlock->RealPosToTab(eBlock->TabPosToReal(VBlockX));
			else
				*Pos=eBlock->RealPosToTab(eBlock->SelStart);
		}

		return CalcDistance(TopList,eBlock,-1);
	}

	return -1;
}

int Editor::BlockEnd2NumLine(int *Pos)
{
	int iLine=-1, iPos=-1;
	Edit *eBlock=VBlockStart?VBlockStart:BlockStart;

	if (eBlock)
	{
		int StartSel, EndSel;
		Edit *eLine=eBlock;
		iLine=BlockStart2NumLine(nullptr); // получили строку начала блока

		if (VBlockStart)
		{
			for (int Line=VBlockSizeY; eLine  && Line > 0; Line--, eLine=eLine->m_next)
			{
				iPos=eLine->RealPosToTab(eLine->TabPosToReal(VBlockX+VBlockSizeX));
				iLine++;
			}

			iLine--;
		}
		else
		{
			while (eLine)  // поиск строки, содержащую конец блока
			{
				eLine->GetSelection(StartSel,EndSel);

				if (EndSel == -1) // это значит, что конец блока "за строкой"
					eLine->GetRealSelection(StartSel,EndSel);

				if (StartSel == -1)
				{
					// Если в текущей строки нет выделения, это еще не значит что мы в конце. Это может быть только начало :)
					if (eLine->m_next)
					{
						eLine->m_next->GetSelection(StartSel,EndSel);

						if (EndSel == -1) // это значит, что конец блока "за строкой"
							eLine->m_next->GetRealSelection(StartSel,EndSel);

						if (StartSel==-1)
						{
							break;
						}
					}
					else
						break;
				}
				else
				{
					iPos=eLine->RealPosToTab(EndSel);
					iLine++;
				}

				eLine=eLine->m_next;
			}

			iLine--;
		}
	}

	if (Pos)
		*Pos=iPos;

	return iLine;
}


__int64 Editor::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	int CurPos=CurLine->GetCurPos();

	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return (__int64)(!CurLine->m_next && !CurLine->m_prev); //??
		case MCODE_C_EOF:
			return (__int64)(!CurLine->m_next && CurPos>=CurLine->GetLength());
		case MCODE_C_BOF:
			return (__int64)(!CurLine->m_prev && !CurPos);
		case MCODE_C_SELECTED:
			return (__int64)(BlockStart || VBlockStart?TRUE:FALSE);
		case MCODE_V_EDITORCURPOS:
			return (__int64)(CurLine->GetTabCurPos()+1);
		case MCODE_V_EDITORREALPOS:
			return (__int64)(CurLine->GetCurPos()+1);
		case MCODE_V_EDITORCURLINE:
			return (__int64)(NumLine+1);
		case MCODE_V_ITEMCOUNT:
		case MCODE_V_EDITORLINES:
			return (__int64)NumLastLine;
			// работа со стековыми закладками
		case MCODE_F_BM_ADD:
			return AddStackBookmark();
		case MCODE_F_BM_CLEAR:
			return ClearStackBookmarks();
		case MCODE_F_BM_NEXT:
			return NextStackBookmark();
		case MCODE_F_BM_PREV:
			return PrevStackBookmark();
		case MCODE_F_BM_STAT:
			// пока так, т.е. BM.Stat(0) возвращает количество
			return GetStackBookmarks(nullptr);
		case MCODE_F_BM_GET:                   // N=BM.Get(Idx,M) - возвращает координаты строки (M==0) или колонки (M==1) закладки с индексом (Idx=1...)
		{
			__int64 Ret=-1;
			long Val[1];
			EditorBookMarks ebm={0};
			int iMode=(int)((LONG_PTR)vParam);

			switch (iMode)
			{
				case 0: ebm.Line=Val;  break;
				case 1: ebm.Cursor=Val; break;
				case 2: ebm.LeftPos=Val; break;
				case 3: ebm.ScreenLine=Val; break;
				default: iMode=-1; break;
			}

			if (iMode >= 0 && GetStackBookmark((int)iParam-1,&ebm))
				Ret=(__int64)((DWORD)Val[0]+1);

			return Ret;
		}
		case MCODE_F_BM_DEL:                   // N=BM.Del(Idx) - удаляет закладку с указанным индексом (x=1...), 0 - удаляет текущую закладку
			return DeleteStackBookmark(PointerToStackBookmark((int)iParam-1));
		case MCODE_F_EDITOR_SEL:
		{
			int iLine;
			int iPos;
			int Action=(int)((INT_PTR)vParam);

			switch (Action)
			{
				case 0:  // Get Param
				{
					switch (iParam)
					{
						case 0:  // return FirstLine
						{
							return BlockStart2NumLine(nullptr)+1;
						}
						case 1:  // return FirstPos
						{
							if (BlockStart2NumLine(&iPos) != -1)
								return iPos+1;

							return 0;
						}
						case 2:  // return LastLine
						{
							return BlockEnd2NumLine(nullptr)+1;
						}
						case 3:  // return LastPos
						{
							if (BlockEnd2NumLine(&iPos) != -1)
								return iPos;

							return 0;
						}
						case 4: // return block type (0=nothing 1=stream, 2=column)
						{
							return VBlockStart?2:(BlockStart?1:0);
						}
					}

					break;
				}
				case 1:  // Set Pos
				{
					switch (iParam)
					{
						case 0: // begin block (FirstLine & FirstPos)
						case 1: // end block (LastLine & LastPos)
						{
							if (!iParam)
								iLine=BlockStart2NumLine(&iPos);
							else
								iLine=BlockEnd2NumLine(&iPos);

							if (iLine > -1 && iPos > -1)
							{
								GoToLine(iLine);
								CurLine->SetCurPos(CurLine->TabPosToReal(iPos));
								return 1;
							}

							return 0;
						}
					}

					break;
				}
				case 2: // Set Stream Selection Edge
				case 3: // Set Column Selection Edge
				{
					switch (iParam)
					{
						case 0:  // selection start
						{
							MBlockStart=CurLine;
							MBlockStartX=CurLine->GetCurPos();
							return 1;
						}
						case 1:  // selection finish
						{
							int Ret=0;

							if (CheckLine(MBlockStart))
							{
								EditorSelect eSel;
								eSel.BlockType=(Action == 2)?BTYPE_STREAM:BTYPE_COLUMN;
								eSel.BlockStartPos=MBlockStartX;
								eSel.BlockWidth=CurLine->GetCurPos()-MBlockStartX;

								if (eSel.BlockWidth || (Action == 2 && MBlockStart != CurLine))
								{
									int bl=CalcDistance(TopList,MBlockStart,-1);
									int el=CalcDistance(TopList,CurLine,-1);

									if (bl > el)
									{
										eSel.BlockStartLine=el;
										eSel.BlockHeight=CalcDistance(CurLine,MBlockStart,-1)+1;
									}
									else
									{
										eSel.BlockStartLine=bl;
										eSel.BlockHeight=CalcDistance(MBlockStart,CurLine,-1)+1;
									}

									if (bl > el || (bl == el && eSel.BlockWidth<0))
									{
										eSel.BlockWidth*=-1;
										eSel.BlockStartPos=CurLine->GetCurPos();
									}

									Ret=EditorControl(ECTL_SELECT,&eSel);
								}
								else if (!eSel.BlockWidth && MBlockStart == CurLine)
								{
									UnmarkBlock();
								}
							}

							UnmarkMacroBlock();
							Show();
							return (__int64)Ret;
						}
					}

					break;
				}
				case 4: // UnMark sel block
				{
					bool NeedRedraw=BlockStart || VBlockStart;
					UnmarkBlock();
					UnmarkMacroBlock();

					if (NeedRedraw)
						Show();

					return 1;
				}
			}

			break;
		}
		case MCODE_V_EDITORSELVALUE: // Editor.SelValue
		{
			string strText;
			wchar_t *Text;

			if (VBlockStart)
				Text = VBlock2Text(nullptr);
			else
				Text = Block2Text(nullptr);

			if (Text)
			{
				strText = Text;
				xf_free(Text);
			}

			*(string *)vParam=strText;
			return 1;
		}
	}

	return 0;
}


int Editor::ProcessKey(int Key)
{
	if (Key==KEY_IDLE)
	{
		if (Opt.ViewerEditorClock && HostFileEditor && HostFileEditor->IsFullScreen() && Opt.EdOpt.ShowTitleBar)
			ShowTime(FALSE);

		return TRUE;
	}

	if (Key==KEY_NONE)
		return TRUE;

	_KEYMACRO(CleverSysLog SL(L"Editor::ProcessKey()"));
	_KEYMACRO(SysLog(L"Key=%s",_FARKEY_ToName(Key)));
	int CurPos,CurVisPos,I;
	CurPos=CurLine->GetCurPos();
	CurVisPos=GetLineCurPos();
	int isk=IsShiftKey(Key);
	_SVS(SysLog(L"[%d] isk=%d",__LINE__,isk));

	//if ((!isk || CtrlObject->Macro.IsExecuting()) && !isk && !Pasting)
	if (!isk && !Pasting && !(((unsigned int)Key >= KEY_MACRO_BASE && (unsigned int)Key <= KEY_MACRO_ENDBASE) || ((unsigned int)Key>=KEY_OP_BASE && (unsigned int)Key <=KEY_OP_ENDBASE)))
	{
		_SVS(SysLog(L"[%d] BlockStart=(%d,%d)",__LINE__,BlockStart,VBlockStart));

		if (BlockStart || VBlockStart)
		{
			Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
		}

		if ((BlockStart || VBlockStart) && !EdOpt.PersistentBlocks)
//    if (BlockStart || VBlockStart && !EdOpt.PersistentBlocks)
		{
			Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);

			if (!EdOpt.PersistentBlocks)
			{
				static int UnmarkKeys[]=
				{
					KEY_LEFT,      KEY_NUMPAD4,
					KEY_RIGHT,     KEY_NUMPAD6,
					KEY_HOME,      KEY_NUMPAD7,
					KEY_END,       KEY_NUMPAD1,
					KEY_UP,        KEY_NUMPAD8,
					KEY_DOWN,      KEY_NUMPAD2,
					KEY_PGUP,      KEY_NUMPAD9,
					KEY_PGDN,      KEY_NUMPAD3,
					KEY_CTRLHOME,  KEY_CTRLNUMPAD7,
					KEY_CTRLPGUP,  KEY_CTRLNUMPAD9,
					KEY_CTRLEND,   KEY_CTRLNUMPAD1,
					KEY_CTRLPGDN,  KEY_CTRLNUMPAD3,
					KEY_CTRLLEFT,  KEY_CTRLNUMPAD4,
					KEY_CTRLRIGHT, KEY_CTRLNUMPAD7,
					KEY_CTRLUP,    KEY_CTRLNUMPAD8,
					KEY_CTRLDOWN,  KEY_CTRLNUMPAD2,
					KEY_CTRLN,
					KEY_CTRLE,
					KEY_CTRLS,
				};

				for (size_t I=0; I<ARRAYSIZE(UnmarkKeys); I++)
					if (Key==UnmarkKeys[I])
					{
						UnmarkBlock();
						break;
					}
			}
			else
			{
				int StartSel,EndSel;
//        Edit *BStart=!BlockStart?VBlockStart:BlockStart;
//        BStart->GetRealSelection(StartSel,EndSel);
				BlockStart->GetRealSelection(StartSel,EndSel);
				_SVS(SysLog(L"[%d] PersistentBlocks! StartSel=%d, EndSel=%d",__LINE__,StartSel,EndSel));

				if (StartSel==-1 || StartSel==EndSel)
					UnmarkBlock();
			}
		}
	}

	if (Key==KEY_ALTD)
		Key=KEY_CTRLK;

	// работа с закладками
	if (Key>=KEY_CTRL0 && Key<=KEY_CTRL9)
		return GotoBookmark(Key-KEY_CTRL0);

	if (Key>=KEY_CTRLSHIFT0 && Key<=KEY_CTRLSHIFT9)
		Key=Key-KEY_CTRLSHIFT0+KEY_RCTRL0;

	if (Key>=KEY_RCTRL0 && Key<=KEY_RCTRL9)
		return SetBookmark(Key-KEY_RCTRL0);

	int SelStart=0,SelEnd=0;
	int SelFirst=FALSE;
	int SelAtBeginning=FALSE;
	EditorBlockGuard _bg(*this,&Editor::UnmarkEmptyBlock);

	switch (Key)
	{
		case KEY_SHIFTLEFT:    case KEY_SHIFTRIGHT:
		case KEY_SHIFTUP:      case KEY_SHIFTDOWN:
		case KEY_SHIFTHOME:    case KEY_SHIFTEND:
		case KEY_SHIFTNUMPAD4: case KEY_SHIFTNUMPAD6:
		case KEY_SHIFTNUMPAD8: case KEY_SHIFTNUMPAD2:
		case KEY_SHIFTNUMPAD7: case KEY_SHIFTNUMPAD1:
		case KEY_CTRLSHIFTLEFT:  case KEY_CTRLSHIFTNUMPAD4:   /* 12.11.2002 DJ */
		{
			_KEYMACRO(CleverSysLog SL(L"Editor::ProcessKey(KEY_SHIFT*)"));
			_SVS(SysLog(L"[%d] SelStart=%d, SelEnd=%d",__LINE__,SelStart,SelEnd));
			UnmarkEmptyBlock(); // уберем выделение, если его размер равен 0
			_bg.SetNeedCheckUnmark(true);
			CurLine->GetRealSelection(SelStart,SelEnd);

			if (Flags.Check(FEDITOR_CURPOSCHANGEDBYPLUGIN))
			{
				if (SelStart!=-1 && (CurPos<SelStart || // если курсор до выделения
				                     (SelEnd!=-1 && (CurPos>SelEnd ||    // ... после выделения
				                                     (CurPos>SelStart && CurPos<SelEnd)))) &&
				        CurPos<CurLine->GetLength()) // ... внутри выдления
					Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);

				Flags.Clear(FEDITOR_CURPOSCHANGEDBYPLUGIN);
			}

			_SVS(SysLog(L"[%d] SelStart=%d, SelEnd=%d",__LINE__,SelStart,SelEnd));

			if (!Flags.Check(FEDITOR_MARKINGBLOCK))
			{
				UnmarkBlock();
				Flags.Set(FEDITOR_MARKINGBLOCK);
				BlockStart=CurLine;
				BlockStartLine=NumLine;
				SelFirst=TRUE;
				SelStart=SelEnd=CurPos;
			}
			else
			{
				SelAtBeginning=CurLine==BlockStart && CurPos==SelStart;

				if (SelStart==-1)
				{
					SelStart=SelEnd=CurPos;
				}
			}

			_SVS(SysLog(L"[%d] SelStart=%d, SelEnd=%d",__LINE__,SelStart,SelEnd));
		}
	}

	switch (Key)
	{
		case KEY_CTRLSHIFTPGUP:   case KEY_CTRLSHIFTNUMPAD9:
		case KEY_CTRLSHIFTHOME:   case KEY_CTRLSHIFTNUMPAD7:
		{
			Lock();
			Pasting++;

			while (CurLine!=TopList)
			{
				ProcessKey(KEY_SHIFTPGUP);
			}

			if (Key == KEY_CTRLSHIFTHOME || Key == KEY_CTRLSHIFTNUMPAD7)
				ProcessKey(KEY_SHIFTHOME);

			Pasting--;
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_CTRLSHIFTPGDN:   case KEY_CTRLSHIFTNUMPAD3:
		case KEY_CTRLSHIFTEND:    case KEY_CTRLSHIFTNUMPAD1:
		{
			Lock();
			Pasting++;

			while (CurLine!=EndList)
			{
				ProcessKey(KEY_SHIFTPGDN);
			}

			/* $ 06.02.2002 IS
			   Принудительно сбросим флаг того, что позиция изменена плагином.
			   Для чего:
			     при выполнении "ProcessKey(KEY_SHIFTPGDN)" (см. чуть выше)
			     позиция плагины (в моем случае - колорер) могут дергать
			     ECTL_SETPOSITION, в результате чего выставляется флаг
			     FEDITOR_CURPOSCHANGEDBYPLUGIN. А при обработке KEY_SHIFTEND
			     выделение в подобном случае начинается с нуля, что сводит на нет
			     предыдущее выполнение KEY_SHIFTPGDN.
			*/
			Flags.Clear(FEDITOR_CURPOSCHANGEDBYPLUGIN);

			if (Key == KEY_CTRLSHIFTEND || Key == KEY_CTRLSHIFTNUMPAD1)
				ProcessKey(KEY_SHIFTEND);

			Pasting--;
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_SHIFTPGUP:       case KEY_SHIFTNUMPAD9:
		{
			Pasting++;
			Lock();

			for (I=Y1; I<Y2; I++)
			{
				ProcessKey(KEY_SHIFTUP);

				if (!EdOpt.CursorBeyondEOL)
				{
					if (CurLine->GetCurPos()>CurLine->GetLength())
					{
						CurLine->SetCurPos(CurLine->GetLength());
					}
				}
			}

			Pasting--;
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_SHIFTPGDN:       case KEY_SHIFTNUMPAD3:
		{
			Pasting++;
			Lock();

			for (I=Y1; I<Y2; I++)
			{
				ProcessKey(KEY_SHIFTDOWN);

				if (!EdOpt.CursorBeyondEOL)
				{
					if (CurLine->GetCurPos()>CurLine->GetLength())
					{
						CurLine->SetCurPos(CurLine->GetLength());
					}
				}
			}

			Pasting--;
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_SHIFTHOME:       case KEY_SHIFTNUMPAD7:
		{
			Pasting++;
			Lock();

			if (SelAtBeginning)
			{
				CurLine->Select(0,SelEnd);
			}
			else
			{
				if (!SelStart)
				{
					CurLine->Select(-1,0);
				}
				else
				{
					CurLine->Select(0,SelStart);
				}
			}

			ProcessKey(KEY_HOME);
			Pasting--;
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_SHIFTEND:
		case KEY_SHIFTNUMPAD1:
		{
			{
				int LeftPos=CurLine->GetLeftPos();
				Pasting++;
				Lock();
				int CurLength=CurLine->GetLength();

				if (!SelAtBeginning || SelFirst)
				{
					CurLine->Select(SelStart,CurLength);
				}
				else
				{
					if (SelEnd!=-1)
						CurLine->Select(SelEnd,CurLength);
					else
						CurLine->Select(CurLength,-1);
				}

				CurLine->ObjWidth=XX2-X1;
				ProcessKey(KEY_END);
				Pasting--;
				Unlock();

				if (EdOpt.PersistentBlocks)
					Show();
				else
				{
					CurLine->FastShow();
					ShowEditor(LeftPos==CurLine->GetLeftPos());
				}
			}
			return TRUE;
		}
		case KEY_SHIFTLEFT:  case KEY_SHIFTNUMPAD4:
		{
			_SVS(CleverSysLog SL(L"case KEY_SHIFTLEFT"));

			if (!CurPos && !CurLine->m_prev)return TRUE;

			if (!CurPos) //курсор в начале строки
			{
				if (SelAtBeginning) //курсор в начале блока
				{
					BlockStart=CurLine->m_prev;
					CurLine->m_prev->Select(CurLine->m_prev->GetLength(),-1);
				}
				else // курсор в конце блока
				{
					CurLine->Select(-1,0);
					CurLine->m_prev->GetRealSelection(SelStart,SelEnd);
					CurLine->m_prev->Select(SelStart,CurLine->m_prev->GetLength());
				}
			}
			else
			{
				if (SelAtBeginning || SelFirst)
				{
					CurLine->Select(SelStart-1,SelEnd);
				}
				else
				{
					CurLine->Select(SelStart,SelEnd-1);
				}
			}

			int LeftPos=CurLine->GetLeftPos();
			Edit *OldCur=CurLine;
			int _OldNumLine=NumLine;
			Pasting++;
			ProcessKey(KEY_LEFT);
			Pasting--;

			if (_OldNumLine!=NumLine)
			{
				BlockStartLine=NumLine;
			}

			ShowEditor(OldCur==CurLine && LeftPos==CurLine->GetLeftPos());
			return TRUE;
		}
		case KEY_SHIFTRIGHT:  case KEY_SHIFTNUMPAD6:
		{
			_SVS(CleverSysLog SL(L"case KEY_SHIFTRIGHT"));

			if (!CurLine->m_next && CurPos==CurLine->GetLength() && !EdOpt.CursorBeyondEOL)
			{
				return TRUE;
			}

			if (SelAtBeginning)
			{
				CurLine->Select(SelStart+1,SelEnd);
			}
			else
			{
				CurLine->Select(SelStart,SelEnd+1);
			}

			Edit *OldCur=CurLine;
			int OldLeft=CurLine->GetLeftPos();
			Pasting++;
			ProcessKey(KEY_RIGHT);
			Pasting--;

			if (OldCur!=CurLine)
			{
				if (SelAtBeginning)
				{
					OldCur->Select(-1,0);
					BlockStart=CurLine;
					BlockStartLine=NumLine;
				}
				else
				{
					OldCur->Select(SelStart,-1);
				}
			}

			ShowEditor(OldCur==CurLine && OldLeft==CurLine->GetLeftPos());
			return TRUE;
		}
		case KEY_CTRLSHIFTLEFT:  case KEY_CTRLSHIFTNUMPAD4:
		{
			_SVS(CleverSysLog SL(L"case KEY_CTRLSHIFTLEFT"));
			_SVS(SysLog(L"[%d] Pasting=%d, SelEnd=%d",__LINE__,Pasting,SelEnd));
			{
				int SkipSpace=TRUE;
				Pasting++;
				Lock();
				int CurPos;

				for (;;)
				{
					const wchar_t *Str;
					int Length;
					CurLine->GetBinaryString(&Str,nullptr,Length);
					/* $ 12.11.2002 DJ
					   обеспечим корректную работу Ctrl-Shift-Left за концом строки
					*/
					CurPos=CurLine->GetCurPos();

					if (CurPos>Length)
					{
						int SelStartPos = CurPos;
						CurLine->ProcessKey(KEY_END);
						CurPos=CurLine->GetCurPos();

						if (CurLine->SelStart >= 0)
						{
							if (!SelAtBeginning)
								CurLine->Select(CurLine->SelStart, CurPos);
							else
								CurLine->Select(CurPos, CurLine->SelEnd);
						}
						else
							CurLine->Select(CurPos, SelStartPos);
					}

					if (!CurPos)
						break;

					if (IsSpace(Str[CurPos-1]) || IsWordDiv(EdOpt.strWordDiv,Str[CurPos-1]))
					{
						if (SkipSpace)
						{
							ProcessKey(KEY_SHIFTLEFT);
							continue;
						}
						else
							break;
					}

					SkipSpace=FALSE;
					ProcessKey(KEY_SHIFTLEFT);
				}

				Pasting--;
				Unlock();
				Show();
			}
			return TRUE;
		}
		case KEY_CTRLSHIFTRIGHT:  case KEY_CTRLSHIFTNUMPAD6:
		{
			_SVS(CleverSysLog SL(L"case KEY_CTRLSHIFTRIGHT"));
			_SVS(SysLog(L"[%d] Pasting=%d, SelEnd=%d",__LINE__,Pasting,SelEnd));
			{
				int SkipSpace=TRUE;
				Pasting++;
				Lock();
				int CurPos;

				for (;;)
				{
					const wchar_t *Str;
					int Length;
					CurLine->GetBinaryString(&Str,nullptr,Length);
					CurPos=CurLine->GetCurPos();

					if (CurPos>=Length)
						break;

					if (IsSpace(Str[CurPos]) || IsWordDiv(EdOpt.strWordDiv,Str[CurPos]))
					{
						if (SkipSpace)
						{
							ProcessKey(KEY_SHIFTRIGHT);
							continue;
						}
						else
							break;
					}

					SkipSpace=FALSE;
					ProcessKey(KEY_SHIFTRIGHT);
				}

				Pasting--;
				Unlock();
				Show();
			}
			return TRUE;
		}
		case KEY_SHIFTDOWN:  case KEY_SHIFTNUMPAD2:
		{
			if (!CurLine->m_next)return TRUE;

			CurPos=CurLine->RealPosToTab(CurPos);

			if (SelAtBeginning)//Снимаем выделение
			{
				if (SelEnd==-1)
				{
					CurLine->Select(-1,0);
					BlockStart=CurLine->m_next;
					BlockStartLine=NumLine+1;
				}
				else
				{
					CurLine->Select(SelEnd,-1);
				}

				CurLine->m_next->GetRealSelection(SelStart,SelEnd);

				if (SelStart!=-1)SelStart=CurLine->m_next->RealPosToTab(SelStart);

				if (SelEnd!=-1)SelEnd=CurLine->m_next->RealPosToTab(SelEnd);

				if (SelStart==-1)
				{
					SelStart=0;
					SelEnd=CurPos;
				}
				else
				{
					if (SelEnd!=-1 && SelEnd<CurPos)
					{
						SelStart=SelEnd;
						SelEnd=CurPos;
					}
					else
					{
						SelStart=CurPos;
					}
				}

				if (SelStart!=-1)SelStart=CurLine->m_next->TabPosToReal(SelStart);

				if (SelEnd!=-1)SelEnd=CurLine->m_next->TabPosToReal(SelEnd);

				/*if(!EdOpt.CursorBeyondEOL && SelEnd>CurLine->m_next->GetLength())
				{
				  SelEnd=CurLine->m_next->GetLength();
				}
				if(!EdOpt.CursorBeyondEOL && SelStart>CurLine->m_next->GetLength())
				{
				  SelStart=CurLine->m_next->GetLength();
				}*/
			}
			else //расширяем выделение
			{
				CurLine->Select(SelStart,-1);
				SelStart=0;
				SelEnd=CurPos;

				if (SelStart!=-1)SelStart=CurLine->m_next->TabPosToReal(SelStart);

				if (SelEnd!=-1)SelEnd=CurLine->m_next->TabPosToReal(SelEnd);
			}

			if (!EdOpt.CursorBeyondEOL && SelEnd > CurLine->m_next->GetLength())
			{
				SelEnd=CurLine->m_next->GetLength();
			}

			if (!EdOpt.CursorBeyondEOL && SelStart > CurLine->m_next->GetLength())
			{
				SelStart=CurLine->m_next->GetLength();
			}

//      if(!SelStart && !SelEnd)
//        CurLine->m_next->Select(-1,0);
//      else
			CurLine->m_next->Select(SelStart,SelEnd);
			Down();
			Show();
			return TRUE;
		}
		case KEY_SHIFTUP: case KEY_SHIFTNUMPAD8:
		{
			if (!CurLine->m_prev) return 0;

			if (SelAtBeginning || SelFirst) // расширяем выделение
			{
				CurLine->Select(0,SelEnd);
				SelStart=CurLine->RealPosToTab(CurPos);

				if (!EdOpt.CursorBeyondEOL &&
				        CurLine->m_prev->TabPosToReal(SelStart)>CurLine->m_prev->GetLength())
				{
					SelStart=CurLine->m_prev->RealPosToTab(CurLine->m_prev->GetLength());
				}

				SelStart=CurLine->m_prev->TabPosToReal(SelStart);
				CurLine->m_prev->Select(SelStart,-1);
				BlockStart=CurLine->m_prev;
				BlockStartLine=NumLine-1;
			}
			else // снимаем выделение
			{
				CurPos=CurLine->RealPosToTab(CurPos);

				if (!SelStart)
				{
					CurLine->Select(-1,0);
				}
				else
				{
					CurLine->Select(0,SelStart);
				}

				CurLine->m_prev->GetRealSelection(SelStart,SelEnd);

				if (SelStart!=-1)SelStart=CurLine->m_prev->RealPosToTab(SelStart);

				if (SelStart!=-1)SelEnd=CurLine->m_prev->RealPosToTab(SelEnd);

				if (SelStart==-1)
				{
					BlockStart=CurLine->m_prev;
					BlockStartLine=NumLine-1;
					SelStart=CurLine->m_prev->TabPosToReal(CurPos);
					SelEnd=-1;
				}
				else
				{
					if (CurPos<SelStart)
					{
						SelEnd=SelStart;
						SelStart=CurPos;
					}
					else
					{
						SelEnd=CurPos;
					}

					SelStart=CurLine->m_prev->TabPosToReal(SelStart);
					SelEnd=CurLine->m_prev->TabPosToReal(SelEnd);

					if (!EdOpt.CursorBeyondEOL && SelEnd>CurLine->m_prev->GetLength())
					{
						SelEnd=CurLine->m_prev->GetLength();
					}

					if (!EdOpt.CursorBeyondEOL && SelStart>CurLine->m_prev->GetLength())
					{
						SelStart=CurLine->m_prev->GetLength();
					}
				}

				CurLine->m_prev->Select(SelStart,SelEnd);
			}

			Up();
			Show();
			return TRUE;
		}
		case KEY_CTRLADD:
		{
			Copy(TRUE);
			return TRUE;
		}
		case KEY_CTRLA:
		{
			UnmarkBlock();
			SelectAll();
			return TRUE;
		}
		case KEY_CTRLU:
		{
			UnmarkMacroBlock();
			UnmarkBlock();
			return TRUE;
		}
		case KEY_CTRLC:
		case KEY_CTRLINS:    case KEY_CTRLNUMPAD0:
		{
			if (/*!EdOpt.PersistentBlocks && */!BlockStart && !VBlockStart)
			{
				BlockStart=CurLine;
				BlockStartLine=NumLine;
				CurLine->AddSelect(0,-1);
				Show();
			}

			Copy(FALSE);
			return TRUE;
		}
		case KEY_CTRLP:
		case KEY_CTRLM:
		{
			if (Flags.Check(FEDITOR_LOCKMODE))
				return TRUE;

			if (BlockStart || VBlockStart)
			{
				int SelStart,SelEnd;
				CurLine->GetSelection(SelStart,SelEnd);
				Pasting++;
				bool OldUseInternalClipboard=Clipboard::SetUseInternalClipboardState(true);
				ProcessKey(Key==KEY_CTRLP ? KEY_CTRLINS:KEY_SHIFTDEL);

				/* $ 10.04.2001 SVS
				  ^P/^M - некорректно работали: уловие для CurPos должно быть ">=",
				   а не "меньше".
				*/
				if (Key==KEY_CTRLM && SelStart!=-1 && SelEnd!=-1)
				{
					if (CurPos>=SelEnd)
						CurLine->SetCurPos(CurPos-(SelEnd-SelStart));
					else
						CurLine->SetCurPos(CurPos);
				}

				ProcessKey(KEY_SHIFTINS);
				Pasting--;
				EmptyInternalClipboard();
				Clipboard::SetUseInternalClipboardState(OldUseInternalClipboard);
				/*$ 08.02.2001 SKV
				  всё делалось с pasting'ом, поэтому redraw плагинам не ушел.
				  сделаем его.
				*/
				Show();
			}

			return TRUE;
		}
		case KEY_CTRLX:
		case KEY_SHIFTDEL:
		case KEY_SHIFTNUMDEL:
		case KEY_SHIFTDECIMAL:
		{
			Copy(FALSE);
		}
		case KEY_CTRLD:
		{
			if (Flags.Check(FEDITOR_LOCKMODE))
				return TRUE;

			Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
			DeleteBlock();
			Show();
			return TRUE;
		}
		case KEY_CTRLV:
		case KEY_SHIFTINS: case KEY_SHIFTNUMPAD0:
		{
			if (Flags.Check(FEDITOR_LOCKMODE))
				return TRUE;

			Pasting++;

			if (!EdOpt.PersistentBlocks && !VBlockStart)
				DeleteBlock();

			Paste();
			// MarkingBlock=!VBlockStart;
			Flags.Change(FEDITOR_MARKINGBLOCK,!VBlockStart);
			Flags.Clear(FEDITOR_MARKINGVBLOCK);

			if (!EdOpt.PersistentBlocks)
				UnmarkBlock();

			Pasting--;
			Show();
			return TRUE;
		}
		case KEY_LEFT: case KEY_NUMPAD4:
		{
			Flags.Set(FEDITOR_NEWUNDO);

			if (!CurPos && CurLine->m_prev)
			{
				Up();
				Show();
				CurLine->ProcessKey(KEY_END);
				Show();
			}
			else
			{
				int LeftPos=CurLine->GetLeftPos();
				CurLine->ProcessKey(KEY_LEFT);
				ShowEditor(LeftPos==CurLine->GetLeftPos());
			}

			return TRUE;
		}
		case KEY_INS: case KEY_NUMPAD0:
		{
			Flags.Swap(FEDITOR_OVERTYPE);
			Show();
			return TRUE;
		}
		case KEY_NUMDEL:
		case KEY_DEL:
		{
			if (!Flags.Check(FEDITOR_LOCKMODE))
			{
				// Del в самой последней позиции ничего не удаляет, поэтому не модифицируем...
				if (!CurLine->m_next && CurPos>=CurLine->GetLength() && !BlockStart && !VBlockStart)
					return TRUE;

				/* $ 07.03.2002 IS
				   Снимем выделение, если блок все равно пустой
				*/
				if (!Pasting)
					UnmarkEmptyBlock();

				if (!Pasting && EdOpt.DelRemovesBlocks && (BlockStart || VBlockStart))
					DeleteBlock();
				else
				{
					if (CurPos>=CurLine->GetLength())
					{
						AddUndoData(UNDO_BEGIN);
						AddUndoData(UNDO_EDIT,CurLine->GetStringAddr(),CurLine->GetEOL(),NumLine,CurLine->GetCurPos(),CurLine->GetLength());

						if (!CurLine->m_next)
							CurLine->SetEOL(L"");
						else
						{
							int SelStart,SelEnd,NextSelStart,NextSelEnd;
							int Length=CurLine->GetLength();
							CurLine->GetSelection(SelStart,SelEnd);
							CurLine->m_next->GetSelection(NextSelStart,NextSelEnd);
							const wchar_t *Str;
							int NextLength;
							CurLine->m_next->GetBinaryString(&Str,nullptr,NextLength);
							CurLine->InsertBinaryString(Str,NextLength);
							CurLine->SetEOL(CurLine->m_next->GetEOL());
							CurLine->SetCurPos(CurPos);
							DeleteString(CurLine->m_next,TRUE,NumLine+1);

							if (!NextLength)
								CurLine->SetEOL(L"");

							if (NextSelStart!=-1)
							{
								if (SelStart==-1)
								{
									CurLine->Select(Length+NextSelStart,NextSelEnd==-1 ? -1:Length+NextSelEnd);
									BlockStart=CurLine;
									BlockStartLine=NumLine;
								}
								else
									CurLine->Select(SelStart,NextSelEnd==-1 ? -1:Length+NextSelEnd);
							}
						}

						AddUndoData(UNDO_END);
					}
					else
					{
						AddUndoData(UNDO_EDIT,CurLine->GetStringAddr(),CurLine->GetEOL(),NumLine,CurLine->GetCurPos(),CurLine->GetLength());
						CurLine->ProcessKey(KEY_DEL);
					}

					TextChanged(1);
				}

				Show();
			}

			return TRUE;
		}
		case KEY_BS:
		{
			if (!Flags.Check(FEDITOR_LOCKMODE))
			{
				// Bs в самом начале нихрена ничего не удаляет, посему не будем выставлять
				if (!CurLine->m_prev && !CurPos && !BlockStart && !VBlockStart)
					return TRUE;

				TextChanged(1);
				int IsDelBlock=FALSE;

				if (EdOpt.BSLikeDel)
				{
					if (!Pasting && EdOpt.DelRemovesBlocks && (BlockStart || VBlockStart))
						IsDelBlock=TRUE;
				}
				else
				{
					if (!Pasting && !EdOpt.PersistentBlocks && BlockStart)
						IsDelBlock=TRUE;
				}

				if (IsDelBlock)
					DeleteBlock();
				else if (!CurPos && CurLine->m_prev)
				{
					Pasting++;
					Up();
					CurLine->ProcessKey(KEY_CTRLEND);
					ProcessKey(KEY_DEL);
					Pasting--;
				}
				else
				{
					AddUndoData(UNDO_EDIT,CurLine->GetStringAddr(),CurLine->GetEOL(),NumLine,CurLine->GetCurPos(),CurLine->GetLength());
					CurLine->ProcessKey(KEY_BS);
				}

				Show();
			}

			return TRUE;
		}
		case KEY_CTRLBS:
		{
			if (!Flags.Check(FEDITOR_LOCKMODE))
			{
				TextChanged(1);

				if (!Pasting && !EdOpt.PersistentBlocks && BlockStart)
					DeleteBlock();
				else if (!CurPos && CurLine->m_prev)
					ProcessKey(KEY_BS);
				else
				{
					AddUndoData(UNDO_EDIT,CurLine->GetStringAddr(),CurLine->GetEOL(),NumLine,CurLine->GetCurPos(),CurLine->GetLength());
					CurLine->ProcessKey(KEY_CTRLBS);
				}

				Show();
			}

			return TRUE;
		}
		case KEY_UP: case KEY_NUMPAD8:
		{
			{
				Flags.Set(FEDITOR_NEWUNDO);
				int PrevMaxPos=MaxRightPos;
				Edit *LastTopScreen=TopScreen;
				Up();

				if (TopScreen==LastTopScreen)
					ShowEditor(TRUE);
				else
					Show();

				if (PrevMaxPos>CurLine->GetTabCurPos())
				{
					CurLine->SetTabCurPos(PrevMaxPos);
					CurLine->FastShow();
					CurLine->SetTabCurPos(PrevMaxPos);
					Show();
				}
			}
			return TRUE;
		}
		case KEY_DOWN: case KEY_NUMPAD2:
		{
			{
				Flags.Set(FEDITOR_NEWUNDO);
				int PrevMaxPos=MaxRightPos;
				Edit *LastTopScreen=TopScreen;
				Down();

				if (TopScreen==LastTopScreen)
					ShowEditor(TRUE);
				else
					Show();

				if (PrevMaxPos>CurLine->GetTabCurPos())
				{
					CurLine->SetTabCurPos(PrevMaxPos);
					CurLine->FastShow();
					CurLine->SetTabCurPos(PrevMaxPos);
					Show();
				}
			}
			return TRUE;
		}
		case KEY_MSWHEEL_UP:
		case(KEY_MSWHEEL_UP | KEY_ALT):
		{
			int Roll = Key & KEY_ALT?1:Opt.MsWheelDeltaEdit;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_CTRLUP);

			return TRUE;
		}
		case KEY_MSWHEEL_DOWN:
		case(KEY_MSWHEEL_DOWN | KEY_ALT):
		{
			int Roll = Key & KEY_ALT?1:Opt.MsWheelDeltaEdit;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_CTRLDOWN);

			return TRUE;
		}
		case KEY_MSWHEEL_LEFT:
		case(KEY_MSWHEEL_LEFT | KEY_ALT):
		{
			int Roll = Key & KEY_ALT?1:Opt.MsHWheelDeltaEdit;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_LEFT);

			return TRUE;
		}
		case KEY_MSWHEEL_RIGHT:
		case(KEY_MSWHEEL_RIGHT | KEY_ALT):
		{
			int Roll = Key & KEY_ALT?1:Opt.MsHWheelDeltaEdit;

			for (int i=0; i<Roll; i++)
				ProcessKey(KEY_RIGHT);

			return TRUE;
		}
		case KEY_CTRLUP:  case KEY_CTRLNUMPAD8:
		{
			Flags.Set(FEDITOR_NEWUNDO);
			ScrollUp();
			Show();
			return TRUE;
		}
		case KEY_CTRLDOWN: case KEY_CTRLNUMPAD2:
		{
			Flags.Set(FEDITOR_NEWUNDO);
			ScrollDown();
			Show();
			return TRUE;
		}
		case KEY_PGUP:     case KEY_NUMPAD9:
		{
			Flags.Set(FEDITOR_NEWUNDO);

			for (I=Y1; I<Y2; I++)
				ScrollUp();

			Show();
			return TRUE;
		}
		case KEY_PGDN:    case KEY_NUMPAD3:
		{
			Flags.Set(FEDITOR_NEWUNDO);

			for (I=Y1; I<Y2; I++)
				ScrollDown();

			Show();
			return TRUE;
		}
		case KEY_CTRLHOME:  case KEY_CTRLNUMPAD7:
		case KEY_CTRLPGUP:  case KEY_CTRLNUMPAD9:
		{
			{
				Flags.Set(FEDITOR_NEWUNDO);
				int StartPos=CurLine->GetTabCurPos();
				NumLine=0;
				TopScreen=CurLine=TopList;

				if (Key == KEY_CTRLHOME || Key == KEY_CTRLNUMPAD7)
					CurLine->SetCurPos(0);
				else
					CurLine->SetTabCurPos(StartPos);

				Show();
			}
			return TRUE;
		}
		case KEY_CTRLEND:   case KEY_CTRLNUMPAD1:
		case KEY_CTRLPGDN:  case KEY_CTRLNUMPAD3:
		{
			{
				Flags.Set(FEDITOR_NEWUNDO);
				int StartPos=CurLine->GetTabCurPos();
				NumLine=NumLastLine-1;
				CurLine=EndList;

				for (TopScreen=CurLine,I=Y1; I<Y2 && TopScreen->m_prev; I++)
				{
					TopScreen->SetPosition(X1,I,XX2,I);
					TopScreen=TopScreen->m_prev;
				}

				CurLine->SetLeftPos(0);

				if (Key == KEY_CTRLEND || Key == KEY_CTRLNUMPAD1)
				{
					CurLine->SetCurPos(CurLine->GetLength());
					CurLine->FastShow();
				}
				else
					CurLine->SetTabCurPos(StartPos);

				Show();
			}
			return TRUE;
		}
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (Pasting || !ShiftPressed || CtrlObject->Macro.IsExecuting())
			{
				if (!Pasting && !EdOpt.PersistentBlocks && BlockStart)
					DeleteBlock();

				Flags.Set(FEDITOR_NEWUNDO);
				InsertString();
				CurLine->FastShow();
				Show();
			}

			return TRUE;
		}
		case KEY_CTRLN:
		{
			Flags.Set(FEDITOR_NEWUNDO);

			while (CurLine!=TopScreen)
			{
				CurLine=CurLine->m_prev;
				NumLine--;
			}

			CurLine->SetCurPos(CurPos);
			Show();
			return TRUE;
		}
		case KEY_CTRLE:
		{
			{
				Flags.Set(FEDITOR_NEWUNDO);
				Edit *CurPtr=TopScreen;
				int CurLineFound=FALSE;

				for (I=Y1; I<Y2; I++)
				{
					if (!CurPtr->m_next)
						break;

					if (CurPtr==CurLine)
						CurLineFound=TRUE;

					if (CurLineFound)
						NumLine++;

					CurPtr=CurPtr->m_next;
				}

				CurLine=CurPtr;
				CurLine->SetCurPos(CurPos);
				Show();
			}
			return TRUE;
		}
		case KEY_CTRLL:
		{
			Flags.Swap(FEDITOR_LOCKMODE);

			if (HostFileEditor) HostFileEditor->ShowStatus();

			return TRUE;
		}
		case KEY_CTRLY:
		{
			DeleteString(CurLine,FALSE,NumLine);
			Show();
			return TRUE;
		}
		case KEY_F7:
		{
			int ReplaceMode0=ReplaceMode;
			int ReplaceAll0=ReplaceAll;
			ReplaceMode=ReplaceAll=FALSE;

			if (!Search(FALSE))
			{
				ReplaceMode=ReplaceMode0;
				ReplaceAll=ReplaceAll0;
			}

			return TRUE;
		}
		case KEY_CTRLF7:
		{
			if (!Flags.Check(FEDITOR_LOCKMODE))
			{
				int ReplaceMode0=ReplaceMode;
				int ReplaceAll0=ReplaceAll;
				ReplaceMode=TRUE;
				ReplaceAll=FALSE;

				if (!Search(FALSE))
				{
					ReplaceMode=ReplaceMode0;
					ReplaceAll=ReplaceAll0;
				}
			}

			return TRUE;
		}
		case KEY_SHIFTF7:
		{
			/* $ 20.09.2000 SVS
			   При All после нажатия Shift-F7 надобно снова спросить...
			*/
			//ReplaceAll=FALSE;
			/* $ 07.05.2001 IS
			   Сказано в хелпе "Shift-F7 Продолжить _поиск_"
			*/
			//ReplaceMode=FALSE;
			Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
			Search(TRUE);
			return TRUE;
		}
		case KEY_ALTF7:
		{
			Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
			int LastSearchReversePrev = LastSearchReverse;
			LastSearchReverse = !LastSearchReverse;
			Search(TRUE);
			LastSearchReverse = LastSearchReversePrev;
			return TRUE;
		}
		/*case KEY_F8:
		{
		  Flags.Set(FEDITOR_TABLECHANGEDBYUSER);
		  if ((AnsiText=!AnsiText))
		  {
		    int UseUnicode=FALSE;
		    GetTable(&TableSet,TRUE,TableNum,UseUnicode);
		  }
		  TableNum=0;
		  UseDecodeTable=AnsiText;
		  SetStringsTable();
		  if (HostFileEditor) HostFileEditor->ChangeEditKeyBar();
		  Show();
		  return TRUE;
		} */ //BUGBUGBUG
		/*case KEY_SHIFTF8:
		{
		  {
		    int UseUnicode=FALSE;
		    int GetTableCode=GetTable(&TableSet,FALSE,TableNum,UseUnicode);
		    if (GetTableCode!=-1)
		    {
		      Flags.Set(FEDITOR_TABLECHANGEDBYUSER);
		      UseDecodeTable=GetTableCode;
		      AnsiText=FALSE;
		      SetStringsTable();
		      if (HostFileEditor) HostFileEditor->ChangeEditKeyBar();
		      Show();
		    }
		  }
		  return TRUE; //BUGBUGBUG
		} */
		case KEY_F11:
		{
			/*
			      if(!Flags.Check(FEDITOR_DIALOGMEMOEDIT))
			      {
			        CtrlObject->Plugins.CurEditor=HostFileEditor; // this;
			        if (CtrlObject->Plugins.CommandsMenu(MODALTYPE_EDITOR,0,"Editor"))
			          *PluginTitle=0;
			        Show();
			      }
			*/
			return TRUE;
		}
		case KEY_CTRLSHIFTZ:
		case KEY_ALTBS:
		case KEY_CTRLZ:
		{
			if (!Flags.Check(FEDITOR_LOCKMODE))
			{
				Lock();
				Undo(Key==KEY_CTRLSHIFTZ);
				Unlock();
				Show();
			}

			return TRUE;
		}
		case KEY_ALTF8:
		{
			{
				GoToPosition();

				// <GOTO_UNMARK:1>
				if (!EdOpt.PersistentBlocks)
					UnmarkBlock();

				// </GOTO_UNMARK>
				Show();
			}
			return TRUE;
		}
		case KEY_ALTU:
		{
			if (!Flags.Check(FEDITOR_LOCKMODE))
			{
				BlockLeft();
				Show();
			}

			return TRUE;
		}
		case KEY_ALTI:
		{
			if (!Flags.Check(FEDITOR_LOCKMODE))
			{
				BlockRight();
				Show();
			}

			return TRUE;
		}
		case KEY_ALTSHIFTLEFT:  case KEY_ALTSHIFTNUMPAD4:
		case KEY_ALTLEFT:
		{
			if (!CurPos)
				return TRUE;

			if (!Flags.Check(FEDITOR_MARKINGVBLOCK))
				BeginVBlockMarking();

			Pasting++;
			{
				int Delta=CurLine->GetTabCurPos()-CurLine->RealPosToTab(CurPos-1);

				if (CurLine->GetTabCurPos()>VBlockX)
					VBlockSizeX-=Delta;
				else
				{
					VBlockX-=Delta;
					VBlockSizeX+=Delta;
				}

				/* $ 25.07.2000 tran
				   остатки бага 22 - подправка при перебега за границу блока */
				if (VBlockSizeX<0)
				{
					VBlockSizeX=-VBlockSizeX;
					VBlockX-=VBlockSizeX;
				}

				ProcessKey(KEY_LEFT);
			}
			Pasting--;
			Show();
			//_D(SysLog(L"VBlockX=%i, VBlockSizeX=%i, GetLineCurPos=%i",VBlockX,VBlockSizeX,GetLineCurPos()));
			//_D(SysLog(L"~~~~~~~~~~~~~~~~ KEY_ALTLEFT END, VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
			return TRUE;
		}
		case KEY_ALTSHIFTRIGHT:  case KEY_ALTSHIFTNUMPAD6:
		case KEY_ALTRIGHT:
		{
			/* $ 23.10.2000 tran
			   вместо GetTabCurPos надо вызывать GetCurPos -
			   сравнивать реальную позицию с реальной длиной
			   а было сравнение видимой позицией с реальной длиной*/
			if (!EdOpt.CursorBeyondEOL && CurLine->GetCurPos()>=CurLine->GetLength())
				return TRUE;

			if (!Flags.Check(FEDITOR_MARKINGVBLOCK))
				BeginVBlockMarking();

			//_D(SysLog(L"---------------- KEY_ALTRIGHT, getLineCurPos=%i",GetLineCurPos()));
			Pasting++;
			{
				int Delta;
				/* $ 18.07.2000 tran
				     встань в начало текста, нажми alt-right, alt-pagedown,
				     выделится блок шириной в 1 колонку, нажми еще alt-right
				     выделение сбросится
				*/
				int VisPos=CurLine->RealPosToTab(CurPos),
				           NextVisPos=CurLine->RealPosToTab(CurPos+1);
				//_D(SysLog(L"CurPos=%i, VisPos=%i, NextVisPos=%i",
				//    CurPos,VisPos, NextVisPos); //,CurLine->GetTabCurPos()));
				Delta=NextVisPos-VisPos;
				//_D(SysLog(L"Delta=%i",Delta));

				if (CurLine->GetTabCurPos()>=VBlockX+VBlockSizeX)
					VBlockSizeX+=Delta;
				else
				{
					VBlockX+=Delta;
					VBlockSizeX-=Delta;
				}

				/* $ 25.07.2000 tran
				   остатки бага 22 - подправка при перебега за границу блока */
				if (VBlockSizeX<0)
				{
					VBlockSizeX=-VBlockSizeX;
					VBlockX-=VBlockSizeX;
				}

				ProcessKey(KEY_RIGHT);
				//_D(SysLog(L"VBlockX=%i, VBlockSizeX=%i, GetLineCurPos=%i",VBlockX,VBlockSizeX,GetLineCurPos()));
			}
			Pasting--;
			Show();
			//_D(SysLog(L"~~~~~~~~~~~~~~~~ KEY_ALTRIGHT END, VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
			return TRUE;
		}
		/* $ 29.06.2000 IG
		  + CtrlAltLeft, CtrlAltRight для вертикальный блоков
		*/
		case KEY_CTRLALTLEFT: case KEY_CTRLALTNUMPAD4:
		{
			{
				int SkipSpace=TRUE;
				Pasting++;
				Lock();

				for (;;)
				{
					const wchar_t *Str;
					int Length;
					CurLine->GetBinaryString(&Str,nullptr,Length);
					int CurPos=CurLine->GetCurPos();

					if (CurPos>Length)
					{
						CurLine->ProcessKey(KEY_END);
						CurPos=CurLine->GetCurPos();
					}

					if (!CurPos)
						break;

					if (IsSpace(Str[CurPos-1]) || IsWordDiv(EdOpt.strWordDiv,Str[CurPos-1]))
					{
						if (SkipSpace)
						{
							ProcessKey(KEY_ALTSHIFTLEFT);
							continue;
						}
						else
							break;
					}

					SkipSpace=FALSE;
					ProcessKey(KEY_ALTSHIFTLEFT);
				}

				Pasting--;
				Unlock();
				Show();
			}
			return TRUE;
		}
		case KEY_CTRLALTRIGHT: case KEY_CTRLALTNUMPAD6:
		{
			{
				int SkipSpace=TRUE;
				Pasting++;
				Lock();

				for (;;)
				{
					const wchar_t *Str;
					int Length;
					CurLine->GetBinaryString(&Str,nullptr,Length);
					int CurPos=CurLine->GetCurPos();

					if (CurPos>=Length)
						break;

					if (IsSpace(Str[CurPos]) || IsWordDiv(EdOpt.strWordDiv,Str[CurPos]))
					{
						if (SkipSpace)
						{
							ProcessKey(KEY_ALTSHIFTRIGHT);
							continue;
						}
						else
							break;
					}

					SkipSpace=FALSE;
					ProcessKey(KEY_ALTSHIFTRIGHT);
				}

				Pasting--;
				Unlock();
				Show();
			}
			return TRUE;
		}
		case KEY_ALTSHIFTUP:    case KEY_ALTSHIFTNUMPAD8:
		case KEY_ALTUP:
		{
			if (!CurLine->m_prev)
				return TRUE;

			if (!Flags.Check(FEDITOR_MARKINGVBLOCK))
				BeginVBlockMarking();

			if (!EdOpt.CursorBeyondEOL && VBlockX>=CurLine->m_prev->GetLength())
				return TRUE;

			Pasting++;

			if (NumLine>VBlockY)
				VBlockSizeY--;
			else
			{
				VBlockY--;
				VBlockSizeY++;
				VBlockStart=VBlockStart->m_prev;
				BlockStartLine--;
			}

			ProcessKey(KEY_UP);
			AdjustVBlock(CurVisPos);
			Pasting--;
			Show();
			//_D(SysLog(L"~~~~~~~~ ALT_PGUP, VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
			return TRUE;
		}
		case KEY_ALTSHIFTDOWN:  case KEY_ALTSHIFTNUMPAD2:
		case KEY_ALTDOWN:
		{
			if (!CurLine->m_next)
				return TRUE;

			if (!Flags.Check(FEDITOR_MARKINGVBLOCK))
				BeginVBlockMarking();

			if (!EdOpt.CursorBeyondEOL && VBlockX>=CurLine->m_next->GetLength())
				return TRUE;

			Pasting++;

			if (NumLine>=VBlockY+VBlockSizeY-1)
				VBlockSizeY++;
			else
			{
				VBlockY++;
				VBlockSizeY--;
				VBlockStart=VBlockStart->m_next;
				BlockStartLine++;
			}

			ProcessKey(KEY_DOWN);
			AdjustVBlock(CurVisPos);
			Pasting--;
			Show();
			//_D(SysLog(L"~~~~ Key_AltDOWN: VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
			return TRUE;
		}
		case KEY_ALTSHIFTHOME: case KEY_ALTSHIFTNUMPAD7:
		case KEY_ALTHOME:
		{
			Pasting++;
			Lock();

			while (CurLine->GetCurPos()>0)
				ProcessKey(KEY_ALTSHIFTLEFT);

			Unlock();
			Pasting--;
			Show();
			return TRUE;
		}
		case KEY_ALTSHIFTEND: case KEY_ALTSHIFTNUMPAD1:
		case KEY_ALTEND:
		{
			Pasting++;
			Lock();

			if (CurLine->GetCurPos()<CurLine->GetLength())
				while (CurLine->GetCurPos()<CurLine->GetLength())
					ProcessKey(KEY_ALTSHIFTRIGHT);

			if (CurLine->GetCurPos()>CurLine->GetLength())
				while (CurLine->GetCurPos()>CurLine->GetLength())
					ProcessKey(KEY_ALTSHIFTLEFT);

			Unlock();
			Pasting--;
			Show();
			return TRUE;
		}
		case KEY_ALTSHIFTPGUP: case KEY_ALTSHIFTNUMPAD9:
		case KEY_ALTPGUP:
		{
			Pasting++;
			Lock();

			for (I=Y1; I<Y2; I++)
				ProcessKey(KEY_ALTSHIFTUP);

			Unlock();
			Pasting--;
			Show();
			return TRUE;
		}
		case KEY_ALTSHIFTPGDN: case KEY_ALTSHIFTNUMPAD3:
		case KEY_ALTPGDN:
		{
			Pasting++;
			Lock();

			for (I=Y1; I<Y2; I++)
				ProcessKey(KEY_ALTSHIFTDOWN);

			Unlock();
			Pasting--;
			Show();
			return TRUE;
		}
		case KEY_CTRLALTPGUP: case KEY_CTRLALTNUMPAD9:
		case KEY_CTRLALTHOME: case KEY_CTRLALTNUMPAD7:
		{
			Lock();
			Pasting++;

			while (CurLine!=TopList)
			{
				ProcessKey(KEY_ALTUP);
			}

			Pasting--;
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_CTRLALTPGDN:  case KEY_CTRLALTNUMPAD3:
		case KEY_CTRLALTEND:   case KEY_CTRLALTNUMPAD1:
		{
			Lock();
			Pasting++;

			while (CurLine!=EndList)
			{
				ProcessKey(KEY_ALTDOWN);
			}

			Pasting--;
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_CTRLALTBRACKET:       // Вставить сетевое (UNC) путь из левой панели
		case KEY_CTRLALTBACKBRACKET:   // Вставить сетевое (UNC) путь из правой панели
		case KEY_ALTSHIFTBRACKET:      // Вставить сетевое (UNC) путь из активной панели
		case KEY_ALTSHIFTBACKBRACKET:  // Вставить сетевое (UNC) путь из пассивной панели
		case KEY_CTRLBRACKET:          // Вставить путь из левой панели
		case KEY_CTRLBACKBRACKET:      // Вставить путь из правой панели
		case KEY_CTRLSHIFTBRACKET:     // Вставить путь из активной панели
		case KEY_CTRLSHIFTBACKBRACKET: // Вставить путь из пассивной панели
		case KEY_CTRLSHIFTNUMENTER:
		case KEY_SHIFTNUMENTER:
		case KEY_CTRLSHIFTENTER:
		case KEY_SHIFTENTER:
		{
			if (!Flags.Check(FEDITOR_LOCKMODE))
			{
				Pasting++;
				TextChanged(1);

				if (!EdOpt.PersistentBlocks && BlockStart)
				{
					Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
					DeleteBlock();
				}

				AddUndoData(UNDO_EDIT,CurLine->GetStringAddr(),CurLine->GetEOL(),NumLine,CurLine->GetCurPos(),CurLine->GetLength());
				CurLine->ProcessKey(Key);
				Pasting--;
				Show();
			}

			return TRUE;
		}
		case KEY_CTRLQ:
		{
			if (!Flags.Check(FEDITOR_LOCKMODE))
			{
				Flags.Set(FEDITOR_PROCESSCTRLQ);

				if (HostFileEditor) HostFileEditor->ShowStatus();

				Pasting++;
				TextChanged(1);

				if (!EdOpt.PersistentBlocks && BlockStart)
				{
					Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
					DeleteBlock();
				}

				AddUndoData(UNDO_EDIT,CurLine->GetStringAddr(),CurLine->GetEOL(),NumLine,CurLine->GetCurPos(),CurLine->GetLength());
				CurLine->ProcessCtrlQ();
				Flags.Clear(FEDITOR_PROCESSCTRLQ);
				Pasting--;
				Show();
			}

			return TRUE;
		}
		case KEY_OP_SELWORD:
		{
			int OldCurPos=CurPos;
			int SStart, SEnd;
			Pasting++;
			Lock();
			UnmarkBlock();

			// CurLine->TableSet ??? => UseDecodeTable?CurLine->TableSet:nullptr !!!
			if (CalcWordFromString(CurLine->GetStringAddr(),CurPos,&SStart,&SEnd,EdOpt.strWordDiv))
			{
				CurLine->Select(SStart,SEnd+(SEnd < CurLine->StrSize?1:0));

				if (CurLine->IsSelection())
				{
					Flags.Set(FEDITOR_MARKINGBLOCK);
					BlockStart=CurLine;
					BlockStartLine=NumLine;
					//SelFirst=TRUE;
					SelStart=SStart;
					SelEnd=SEnd;
					//CurLine->ProcessKey(MCODE_OP_SELWORD);
				}
			}

			CurPos=OldCurPos; // возвращаем обратно
			Pasting--;
			Unlock();
			Show();
			return TRUE;
		}
		case KEY_OP_DATE:
		case KEY_OP_PLAINTEXT:
		{
			if (!Flags.Check(FEDITOR_LOCKMODE))
			{
				const wchar_t *Fmt = eStackAsString();
				string strTStr;

				if (Key == KEY_OP_PLAINTEXT)
					strTStr = Fmt;

				if (Key == KEY_OP_PLAINTEXT || MkStrFTime(strTStr, Fmt))
				{
					wchar_t *Ptr=strTStr.GetBuffer();

					while (*Ptr) // заменим L'\n' на L'\r' по правилам Paset ;-)
					{
						if (*Ptr == L'\n')
							*Ptr=L'\r';

						++Ptr;
					}

					strTStr.ReleaseBuffer();
					Pasting++;
					//_SVS(SysLogDump(Fmt,0,TStr,strlen(TStr),nullptr));
					TextChanged(1);
					BOOL IsBlock=VBlockStart || BlockStart;

					if (!EdOpt.PersistentBlocks && IsBlock)
					{
						Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
						DeleteBlock();
					}

					//AddUndoData(UNDO_EDIT,CurLine->GetStringAddr(),CurLine->GetEOL(),NumLine,CurLine->GetCurPos(),CurLine->GetLength());
					Paste(strTStr);
					//if (!EdOpt.PersistentBlocks && IsBlock)
					UnmarkBlock();
					Pasting--;
					Show();
				}
			}

			return TRUE;
		}
		default:
		{
			{
				if ((Key==KEY_CTRLDEL || Key==KEY_CTRLNUMDEL || Key==KEY_CTRLDECIMAL || Key==KEY_CTRLT) && CurPos>=CurLine->GetLength())
				{
					/*$ 08.12.2000 skv
					  - CTRL-DEL в начале строки при выделенном блоке и
					    включенном EditorDelRemovesBlocks
					*/
					int save=EdOpt.DelRemovesBlocks;
					EdOpt.DelRemovesBlocks=0;
					int ret=ProcessKey(KEY_DEL);
					EdOpt.DelRemovesBlocks=save;
					return ret;
				}

				if (!Pasting && !EdOpt.PersistentBlocks && BlockStart)
					if ((Key>=32 && Key<0x10000) || Key==KEY_ADD || Key==KEY_SUBTRACT || // ??? 256 ???
					        Key==KEY_MULTIPLY || Key==KEY_DIVIDE || Key==KEY_TAB)
					{
						DeleteBlock();
						/* $ 19.09.2002 SKV
						  Однако надо.
						  Иначе есди при надичии выделения набирать
						  текст с шифтом флаги не сбросятся и следующий
						  выделенный блок будет глючный.
						*/
						Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
						Show();
					}

				int SkipCheckUndo=(Key==KEY_RIGHT     || Key==KEY_NUMPAD6     ||
				                   Key==KEY_CTRLLEFT  || Key==KEY_CTRLNUMPAD4 ||
				                   Key==KEY_CTRLRIGHT || Key==KEY_CTRLNUMPAD6 ||
				                   Key==KEY_HOME      || Key==KEY_NUMPAD7     ||
				                   Key==KEY_END       || Key==KEY_NUMPAD1     ||
				                   Key==KEY_CTRLS);

				if (Flags.Check(FEDITOR_LOCKMODE) && !SkipCheckUndo)
					return TRUE;

				if ((Key==KEY_CTRLLEFT || Key==KEY_CTRLNUMPAD4) && !CurLine->GetCurPos())
				{
					Pasting++;
					ProcessKey(KEY_LEFT);
					Pasting--;
					/* $ 24.9.2001 SKV
					  fix бага с ctrl-left в начале строки
					  в блоке с переопределённым плагином фоном.
					*/
					ShowEditor(FALSE);
					//if(!Flags.Check(FEDITOR_DIALOGMEMOEDIT)){
					//CtrlObject->Plugins.CurEditor=HostFileEditor; // this;
					//_D(SysLog(L"%08d EE_REDRAW",__LINE__));
					//CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
					//}
					return TRUE;
				}

				if (((!EdOpt.CursorBeyondEOL && (Key==KEY_RIGHT || Key==KEY_NUMPAD6)) || Key==KEY_CTRLRIGHT || Key==KEY_CTRLNUMPAD6) &&
				        CurLine->GetCurPos()>=CurLine->GetLength() &&
				        CurLine->m_next)
				{
					Pasting++;
					ProcessKey(KEY_HOME);
					ProcessKey(KEY_DOWN);
					Pasting--;

					if (!Flags.Check(FEDITOR_DIALOGMEMOEDIT))
					{
						CtrlObject->Plugins.CurEditor=HostFileEditor; // this;
						//_D(SysLog(L"%08d EE_REDRAW",__LINE__));
						_SYS_EE_REDRAW(SysLog(L"Editor::ProcessKey[%d](!EdOpt.CursorBeyondEOL): EE_REDRAW(EEREDRAW_ALL)",__LINE__));
						CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
					}

					/*$ 03.02.2001 SKV
					  А то EEREDRAW_ALL то уходит, а на самом деле
					  только текущая линия перерисовывается.
					*/
					ShowEditor(0);
					return TRUE;
				}

				const wchar_t *Str;

				wchar_t *CmpStr=0;

				int Length,CurPos;

				CurLine->GetBinaryString(&Str,nullptr,Length);

				CurPos=CurLine->GetCurPos();

				if (Key<0x10000 && CurPos>0 && !Length)
				{
					Edit *PrevLine=CurLine->m_prev;

					while (PrevLine && !PrevLine->GetLength())
						PrevLine=PrevLine->m_prev;

					if (PrevLine)
					{
						int TabPos=CurLine->GetTabCurPos();
						CurLine->SetCurPos(0);
						const wchar_t *PrevStr=nullptr;
						int PrevLength=0;
						PrevLine->GetBinaryString(&PrevStr,nullptr,PrevLength);

						for (int I=0; I<PrevLength && IsSpace(PrevStr[I]); I++)
						{
							int NewTabPos=CurLine->GetTabCurPos();

							if (NewTabPos==TabPos)
								break;

							if (NewTabPos>TabPos)
							{
								CurLine->ProcessKey(KEY_BS);

								while (CurLine->GetTabCurPos()<TabPos)
									CurLine->ProcessKey(' ');

								break;
							}

							if (NewTabPos<TabPos)
								CurLine->ProcessKey(PrevStr[I]);
						}

						CurLine->SetTabCurPos(TabPos);
					}
				}

				if (!SkipCheckUndo)
				{
					CurLine->GetBinaryString(&Str,nullptr,Length);
					CurPos=CurLine->GetCurPos();
					CmpStr=new wchar_t[Length+1];
					wmemcpy(CmpStr,Str,Length);
					CmpStr[Length]=0;
				}

				int LeftPos=CurLine->GetLeftPos();

				if (Key == KEY_OP_XLAT)
				{
					Xlat();
					Show();
					return TRUE;
				}

				// <comment> - это требуется для корректной работы логики блоков для Ctrl-K
				int PreSelStart,PreSelEnd;
				CurLine->GetSelection(PreSelStart,PreSelEnd);
				// </comment>
				//AY: Это что бы при FastShow LeftPos не становился в конец строки.
				CurLine->ObjWidth=XX2-X1+1;

				if (CurLine->ProcessKey(Key))
				{
					int SelStart,SelEnd;

					/* $ 17.09.2002 SKV
					  Если находимся в середине блока,
					  в начале строки, и нажимаем tab, который заменяется
					  на пробелы, выделение съедет. Это фикс.
					*/
					if (Key==KEY_TAB && CurLine->GetConvertTabs() &&
					        BlockStart && BlockStart!=CurLine)
					{
						CurLine->GetSelection(SelStart,SelEnd);
						CurLine->Select(SelStart==-1?-1:0,SelEnd);
					}

					if (!SkipCheckUndo)
					{
						const wchar_t *NewCmpStr;
						int NewLength;
						CurLine->GetBinaryString(&NewCmpStr,nullptr,NewLength);

						if (NewLength!=Length || memcmp(CmpStr,NewCmpStr,Length*sizeof(wchar_t)))
						{
							AddUndoData(UNDO_EDIT,CmpStr,CurLine->GetEOL(),NumLine,CurPos,Length); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
							TextChanged(1);
						}

						delete[] CmpStr;
					}

					// <Bug 794>
					// обработаем только первую и последнюю строку с блоком
					if (Key == KEY_CTRLK && EdOpt.PersistentBlocks)
					{
						if (CurLine==BlockStart)
						{
							if (CurPos)
							{
								CurLine->GetSelection(SelStart,SelEnd);

								// 1. блок за концом строки (CurPos был ближе к началу, чем SelStart)
								if ((SelEnd == -1 && PreSelStart > CurPos) || SelEnd > CurPos)
									SelStart=SelEnd=-1; // в этом случае снимаем выделение

								// 2. CurPos внутри блока
								else if (SelEnd == -1 && PreSelEnd > CurPos && SelStart < CurPos)
									SelEnd=PreSelEnd;   // в этом случае усекаем блок

								// 3. блок остался слева от CurPos или выделение нужно снять (см. выше)
								if (SelEnd >= CurPos || SelStart==-1)
									CurLine->Select(SelStart,CurPos);
							}
							else
							{
								CurLine->Select(-1,-1);
								BlockStart=BlockStart->m_next;
							}
						}
						else // ЗДЕСЬ ЗАСАДА !!! ЕСЛИ ВЫДЕЛЕННЫЙ БЛОК ДОСТАТОЧНО БОЛЬШОЙ (ПО СТРОКАМ), ТО ЦИКЛ ПЕРЕБОРА... МОЖЕТ ЗАТЯНУТЬ...
						{
							// найдем эту последнюю строку (и последняя ли она)
							Edit *CurPtrBlock=BlockStart,*CurPtrBlock2=BlockStart;

							while (CurPtrBlock)
							{
								CurPtrBlock->GetRealSelection(SelStart,SelEnd);

								if (SelStart==-1)
									break;

								CurPtrBlock2=CurPtrBlock;
								CurPtrBlock=CurPtrBlock->m_next;
							}

							if (CurLine==CurPtrBlock2)
							{
								if (CurPos)
								{
									CurLine->GetSelection(SelStart,SelEnd);
									CurLine->Select(SelStart,CurPos);
								}
								else
								{
									CurLine->Select(-1,-1);
									CurPtrBlock2=CurPtrBlock2->m_next;
								}
							}
						}
					}

					// </Bug 794>
					ShowEditor(LeftPos==CurLine->GetLeftPos());
					return TRUE;
				}
				else if (!SkipCheckUndo)
					delete[] CmpStr;

				if (VBlockStart)
					Show();
			}
			return FALSE;
		}
	}
}


int Editor::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	Edit *NewPtr;
	int NewDist,Dist;

	// $ 28.12.2000 VVM - Щелчок мышкой снимает непостоянный блок всегда
	if ((MouseEvent->dwButtonState & 3))
	{
		Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);

		if ((!EdOpt.PersistentBlocks) && (BlockStart || VBlockStart))
		{
			UnmarkBlock();
			Show();
		}
	}

	if (EdOpt.ShowScrollBar && MouseEvent->dwMousePosition.X==X2 && !(MouseEvent->dwEventFlags & MOUSE_MOVED))
	{
		if (MouseEvent->dwMousePosition.Y==Y1)
		{
			while (IsMouseButtonPressed())
			{
				ProcessKey(KEY_CTRLUP);
			}
		}
		else if (MouseEvent->dwMousePosition.Y==Y2)
		{
			while (IsMouseButtonPressed())
			{
				ProcessKey(KEY_CTRLDOWN);
			}
		}
		else
		{
			while (IsMouseButtonPressed())
				GoToLine((NumLastLine-1)*(MouseY-Y1)/(Y2-Y1));
		}

		return TRUE;
	}

	if (MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)
	{
		static int EditorPrevDoubleClick=0;
		static COORD EditorPrevPosition={0,0};

		if (GetTickCount()-EditorPrevDoubleClick<=GetDoubleClickTime() && MouseEvent->dwEventFlags!=MOUSE_MOVED &&
		        EditorPrevPosition.X == MouseEvent->dwMousePosition.X && EditorPrevPosition.Y == MouseEvent->dwMousePosition.Y)
		{
			CurLine->Select(0,CurLine->StrSize);

			if (CurLine->IsSelection())
			{
				Flags.Set(FEDITOR_MARKINGBLOCK);
				BlockStart=CurLine;
				BlockStartLine=NumLine;
			}

			EditorPrevDoubleClick=0;
			EditorPrevPosition.X=0;
			EditorPrevPosition.Y=0;
		}

		if (MouseEvent->dwEventFlags==DOUBLE_CLICK)
		{
			Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);

			if (BlockStart || VBlockStart)
				UnmarkBlock();

			ProcessKey(KEY_OP_SELWORD);
			EditorPrevDoubleClick=GetTickCount();
			EditorPrevPosition=MouseEvent->dwMousePosition;
		}
		else
		{
			EditorPrevDoubleClick=0;
			EditorPrevPosition.X=0;
			EditorPrevPosition.Y=0;
		}

		Show();
	}

	if (CurLine->ProcessMouse(MouseEvent))
	{
		if (HostFileEditor) HostFileEditor->ShowStatus();

		if (VBlockStart)
			Show();
		else
		{
			if (!Flags.Check(FEDITOR_DIALOGMEMOEDIT))
			{
				CtrlObject->Plugins.CurEditor=HostFileEditor; // this;
				_SYS_EE_REDRAW(SysLog(L"Editor::ProcessMouse[%08d] ProcessEditorEvent(EE_REDRAW,EEREDRAW_LINE)",__LINE__));
				CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_LINE);
			}
		}

		return TRUE;
	}

	if (!(MouseEvent->dwButtonState & 3))
		return FALSE;

	// scroll up
	if (MouseEvent->dwMousePosition.Y==Y1-1)
	{
		while (IsMouseButtonPressed() && MouseY==Y1-1)
			ProcessKey(KEY_UP);

		return TRUE;
	}

	// scroll down
	if (MouseEvent->dwMousePosition.Y==Y2+1)
	{
		while (IsMouseButtonPressed() && MouseY==Y2+1)
			ProcessKey(KEY_DOWN);

		return TRUE;
	}

	if (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
	        MouseEvent->dwMousePosition.Y<Y1 || MouseEvent->dwMousePosition.Y>Y2)
		return FALSE;

	NewDist=MouseEvent->dwMousePosition.Y-Y1;
	NewPtr=TopScreen;

	while (NewDist-- && NewPtr->m_next)
		NewPtr=NewPtr->m_next;

	Dist=CalcDistance(TopScreen,NewPtr,-1)-CalcDistance(TopScreen,CurLine,-1);

	if (Dist>0)
		while (Dist--)
			Down();
	else
		while (Dist++)
			Up();

	CurLine->ProcessMouse(MouseEvent);
	Show();
	return TRUE;
}


int Editor::CalcDistance(Edit *From, Edit *To,int MaxDist)
{
	int Distance=0;

	while (From!=To && From->m_next && (MaxDist==-1 || MaxDist-- > 0))
	{
		Distance++;
		From=From->m_next;
	}

	return(Distance);
}



void Editor::DeleteString(Edit *DelPtr,int DeleteLast,int UndoLine)
{
	if (Flags.Check(FEDITOR_LOCKMODE))
		return;

	/* $ 16.12.2000 OT
	   CtrlY на последней строке с выделенным вертикальным блоком не снимал выделение */
	if (VBlockStart && NumLine<VBlockY+VBlockSizeY)
	{
		if (NumLine<VBlockY)
		{
			if (VBlockY>0)
			{
				VBlockY--;
				BlockStartLine--;
			}
		}
		else if (--VBlockSizeY<=0)
			VBlockStart=nullptr;
	}

	TextChanged(1);

	if (!DelPtr->m_next && (!DeleteLast || !DelPtr->m_prev))
	{
		AddUndoData(UNDO_EDIT,DelPtr->GetStringAddr(),DelPtr->GetEOL(),UndoLine,DelPtr->GetCurPos(),DelPtr->GetLength());
		DelPtr->SetString(L"");
		return;
	}

	for (size_t I=0; I<ARRAYSIZE(SavePos.Line); I++)
		if (SavePos.Line[I]!=POS_NONE && UndoLine<static_cast<int>(SavePos.Line[I]))
			SavePos.Line[I]--;

	if (StackPos)
	{
		InternalEditorStackBookMark *sb_temp = StackPos, *sb_new;

		while (sb_temp->prev)
			sb_temp=sb_temp->prev;

		while (sb_temp)
		{
			sb_new = sb_temp->next;

			if (UndoLine < static_cast<int>(sb_temp->Line))
				sb_temp->Line--;
			else
			{
				if (UndoLine == static_cast<int>(sb_temp->Line))
					DeleteStackBookmark(sb_temp);
			}

			sb_temp = sb_new;
		}
	}

	NumLastLine--;

	if (CurLine==DelPtr)
	{
		int LeftPos,CurPos;
		CurPos=DelPtr->GetTabCurPos();
		LeftPos=DelPtr->GetLeftPos();

		if (DelPtr->m_next)
			CurLine=DelPtr->m_next;
		else
		{
			CurLine=DelPtr->m_prev;
			/* $ 04.11.2002 SKV
			  Вроде как если это произошло, номер текущей строки надо изменить.
			*/
			NumLine--;
		}

		CurLine->SetLeftPos(LeftPos);
		CurLine->SetTabCurPos(CurPos);
	}

	if (DelPtr->m_prev)
	{
		DelPtr->m_prev->m_next=DelPtr->m_next;

		if (DelPtr==EndList)
			EndList=EndList->m_prev;
	}

	if (DelPtr->m_next)
		DelPtr->m_next->m_prev=DelPtr->m_prev;

	if (DelPtr==TopScreen)
	{
		if (TopScreen->m_next)
			TopScreen=TopScreen->m_next;
		else
			TopScreen=TopScreen->m_prev;
	}

	if (DelPtr==TopList)
		TopList=TopList->m_next;

	if (DelPtr==BlockStart)
	{
		BlockStart=BlockStart->m_next;

		// Mantis#0000316: Не работает копирование строки
		if (BlockStart && !BlockStart->IsSelection())
			BlockStart=nullptr;
	}

	if (DelPtr==VBlockStart)
		VBlockStart=VBlockStart->m_next;

	if (UndoLine!=-1)
		AddUndoData(UNDO_DELSTR,DelPtr->GetStringAddr(),DelPtr->GetEOL(),UndoLine,0,DelPtr->GetLength());

	delete DelPtr;
}


void Editor::InsertString()
{
	if (Flags.Check(FEDITOR_LOCKMODE))
		return;

	/*$ 10.08.2000 skv
	  There is only one return - if new will fail.
	  In this case things are realy bad.
	  Move TextChanged to the end of functions
	  AFTER all modifications are made.
	*/
//  TextChanged(1);
	Edit *NewString;
	Edit *SrcIndent=nullptr;
	int SelStart,SelEnd;
	int CurPos;
	int NewLineEmpty=TRUE;
	NewString = InsertString(nullptr, 0, CurLine);

	if (!NewString)
		return;

	//NewString->SetTables(UseDecodeTable ? &TableSet:nullptr); // ??
	int Length;
	const wchar_t *CurLineStr;
	const wchar_t *EndSeq;
	CurLine->GetBinaryString(&CurLineStr,&EndSeq,Length);

	/* $ 13.01.2002 IS
	   Если не был определен тип конца строки, то считаем что конец строки
	   у нас равен DOS_EOL_fmt и установим его явно.
	*/
	if (!*EndSeq)
		CurLine->SetEOL(*GlobalEOL?GlobalEOL:DOS_EOL_fmt);

	CurPos=CurLine->GetCurPos();
	CurLine->GetSelection(SelStart,SelEnd);

	for (size_t I=0; I<ARRAYSIZE(SavePos.Line); I++)
		if (SavePos.Line[I]!=POS_NONE &&
		        (NumLine<(int)SavePos.Line[I] || (NumLine==(int)SavePos.Line[I] && !CurPos)))
			SavePos.Line[I]++;

	if (StackPos)
	{
		InternalEditorStackBookMark *sb_temp = StackPos;

		while (sb_temp->prev)
			sb_temp=sb_temp->prev;

		while (sb_temp)
		{
			if (NumLine < static_cast<int>(sb_temp->Line) || (NumLine==static_cast<int>(sb_temp->Line) && !CurPos))
				sb_temp->Line++;

			sb_temp=sb_temp->next;
		}
	}

	int IndentPos=0;

	if (EdOpt.AutoIndent && !Pasting)
	{
		Edit *PrevLine=CurLine;

		while (PrevLine)
		{
			const wchar_t *Str;
			int Length,Found=FALSE;
			PrevLine->GetBinaryString(&Str,nullptr,Length);

			for (int I=0; I<Length; I++)
				if (!IsSpace(Str[I]))
				{
					PrevLine->SetCurPos(I);
					IndentPos=PrevLine->GetTabCurPos();
					SrcIndent=PrevLine;
					Found=TRUE;
					break;
				}

			if (Found)
				break;

			PrevLine=PrevLine->m_prev;
		}
	}

	int SpaceOnly=TRUE;

	if (CurPos<Length)
	{
		if (IndentPos>0)
			for (int I=0; I<CurPos; I++)
				if (!IsSpace(CurLineStr[I]))
				{
					SpaceOnly=FALSE;
					break;
				}

		NewString->SetBinaryString(&CurLineStr[CurPos],Length-CurPos);

		for (int i0=0; i0<Length-CurPos; i0++)
		{
			if (!IsSpace(CurLineStr[i0+CurPos]))
			{
				NewLineEmpty=FALSE;
				break;
			}
		}

		AddUndoData(UNDO_BEGIN);
		AddUndoData(UNDO_EDIT,CurLine->GetStringAddr(),CurLine->GetEOL(),NumLine,
		            CurLine->GetCurPos(),CurLine->GetLength());
		AddUndoData(UNDO_INSSTR,nullptr,EndList==CurLine?L"":GlobalEOL,NumLine+1,0); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
		AddUndoData(UNDO_END);
		wchar_t *NewCurLineStr = (wchar_t *) xf_malloc((CurPos+1)*sizeof(wchar_t));

		if (!NewCurLineStr)
			return;

		wmemcpy(NewCurLineStr,CurLineStr,CurPos);
		NewCurLineStr[CurPos]=0;
		int StrSize=CurPos;

		if (EdOpt.AutoIndent && NewLineEmpty)
		{
			RemoveTrailingSpaces(NewCurLineStr);
			StrSize=StrLength(NewCurLineStr);
		}

		CurLine->SetBinaryString(NewCurLineStr,StrSize);
		CurLine->SetEOL(EndSeq);
		xf_free(NewCurLineStr);
	}
	else
	{
		NewString->SetString(L"");
		AddUndoData(UNDO_INSSTR,nullptr,L"",NumLine+1,0);// EOL? - CurLine->GetEOL()  GlobalEOL   ""
	}

	if (VBlockStart && NumLine<VBlockY+VBlockSizeY)
	{
		if (NumLine<VBlockY)
		{
			VBlockY++;
			BlockStartLine++;
		}
		else
			VBlockSizeY++;
	}

	if (SelStart!=-1 && (SelEnd==-1 || CurPos<SelEnd))
	{
		if (CurPos>=SelStart)
		{
			CurLine->Select(SelStart,-1);
			NewString->Select(0,SelEnd==-1 ? -1:SelEnd-CurPos);
		}
		else
		{
			CurLine->Select(-1,0);
			NewString->Select(SelStart-CurPos,SelEnd==-1 ? -1:SelEnd-CurPos);
			BlockStart=NewString;
			BlockStartLine++;
		}
	}
	else if (BlockStart && NumLine<BlockStartLine)
		BlockStartLine++;

	NewString->SetEOL(EndSeq);
	CurLine->SetCurPos(0);

	if (CurLine==EndList)
		EndList=NewString;

	Down();

	if (IndentPos>0)
	{
		int OrgIndentPos=IndentPos;
		ShowEditor(FALSE);
		CurLine->GetBinaryString(&CurLineStr,nullptr,Length);

		if (SpaceOnly)
		{
			int Decrement=0;

			for (int I=0; I<IndentPos && I<Length; I++)
			{
				if (!IsSpace(CurLineStr[I]))
					break;

				if (CurLineStr[I]==L' ')
					Decrement++;
				else
				{
					int TabPos=CurLine->RealPosToTab(I);
					Decrement+=EdOpt.TabSize - (TabPos % EdOpt.TabSize);
				}
			}

			IndentPos-=Decrement;
		}

		if (IndentPos>0)
		{
			if (CurLine->GetLength() || !EdOpt.CursorBeyondEOL)
			{
				CurLine->ProcessKey(KEY_HOME);
				int SaveOvertypeMode=CurLine->GetOvertypeMode();
				CurLine->SetOvertypeMode(FALSE);
				const wchar_t *PrevStr=nullptr;
				int PrevLength=0;

				if (SrcIndent)
				{
					SrcIndent->GetBinaryString(&PrevStr,nullptr,PrevLength);
				}

				for (int I=0; CurLine->GetTabCurPos()<IndentPos; I++)
				{
					if (SrcIndent && I<PrevLength && IsSpace(PrevStr[I]))
					{
						CurLine->ProcessKey(PrevStr[I]);
					}
					else
					{
						CurLine->ProcessKey(KEY_SPACE);
					}
				}

				while (CurLine->GetTabCurPos()>IndentPos)
					CurLine->ProcessKey(KEY_BS);

				CurLine->SetOvertypeMode(SaveOvertypeMode);
			}

			CurLine->SetTabCurPos(IndentPos);
		}

		CurLine->GetBinaryString(&CurLineStr,nullptr,Length);
		CurPos=CurLine->GetCurPos();

		if (SpaceOnly)
		{
			int NewPos=0;

			for (int I=0; I<Length; I++)
			{
				NewPos=I;

				if (!IsSpace(CurLineStr[I]))
					break;
			}

			if (NewPos>OrgIndentPos)
				NewPos=OrgIndentPos;

			if (NewPos>CurPos)
				CurLine->SetCurPos(NewPos);
		}
	}

	TextChanged(1);
}



void Editor::Down()
{
	//TODO: "Свертка" - если учесть "!Flags.Check(FSCROBJ_VISIBLE)", то крутить надо до следующей видимой строки
	Edit *CurPtr;
	int LeftPos,CurPos,Y;

	if (!CurLine->m_next)
		return;

	for (Y=0,CurPtr=TopScreen; CurPtr && CurPtr!=CurLine; CurPtr=CurPtr->m_next)
		Y++;

	if (Y>=Y2-Y1)
		TopScreen=TopScreen->m_next;

	CurPos=CurLine->GetTabCurPos();
	LeftPos=CurLine->GetLeftPos();
	CurLine=CurLine->m_next;
	NumLine++;
	CurLine->SetLeftPos(LeftPos);
	CurLine->SetTabCurPos(CurPos);
}


void Editor::ScrollDown()
{
	//TODO: "Свертка" - если учесть "!Flags.Check(FSCROBJ_VISIBLE)", то крутить надо до следующей видимой строки
	int LeftPos,CurPos;

	if (!CurLine->m_next || !TopScreen->m_next)
		return;

	if (!EdOpt.AllowEmptySpaceAfterEof && CalcDistance(TopScreen,EndList,Y2-Y1)<Y2-Y1)
	{
		Down();
		return;
	}

	TopScreen=TopScreen->m_next;
	CurPos=CurLine->GetTabCurPos();
	LeftPos=CurLine->GetLeftPos();
	CurLine=CurLine->m_next;
	NumLine++;
	CurLine->SetLeftPos(LeftPos);
	CurLine->SetTabCurPos(CurPos);
}


void Editor::Up()
{
	//TODO: "Свертка" - если учесть "!Flags.Check(FSCROBJ_VISIBLE)", то крутить надо до следующей видимой строки
	int LeftPos,CurPos;

	if (!CurLine->m_prev)
		return;

	if (CurLine==TopScreen)
		TopScreen=TopScreen->m_prev;

	CurPos=CurLine->GetTabCurPos();
	LeftPos=CurLine->GetLeftPos();
	CurLine=CurLine->m_prev;
	NumLine--;
	CurLine->SetLeftPos(LeftPos);
	CurLine->SetTabCurPos(CurPos);
}


void Editor::ScrollUp()
{
	//TODO: "Свертка" - если учесть "!Flags.Check(FSCROBJ_VISIBLE)", то крутить надо до следующей видимой строки
	int LeftPos,CurPos;

	if (!CurLine->m_prev)
		return;

	if (!TopScreen->m_prev)
	{
		Up();
		return;
	}

	TopScreen=TopScreen->m_prev;
	CurPos=CurLine->GetTabCurPos();
	LeftPos=CurLine->GetLeftPos();
	CurLine=CurLine->m_prev;
	NumLine--;
	CurLine->SetLeftPos(LeftPos);
	CurLine->SetTabCurPos(CurPos);
}

/* $ 21.01.2001 SVS
   Диалоги поиска/замены выведен из Editor::Search
   в отдельную функцию GetSearchReplaceString
   (файл stddlg.cpp)
*/
BOOL Editor::Search(int Next)
{
	Edit *CurPtr,*TmpPtr;
	string strSearchStr, strReplaceStr;
	static string strLastReplaceStr;
	static int LastSuccessfulReplaceMode=0;
	string strMsgStr;
	const wchar_t *TextHistoryName=L"SearchText",*ReplaceHistoryName=L"ReplaceText";
	int CurPos,Case,WholeWords,ReverseSearch,SelectFound,Regexp,Match,NewNumLine,UserBreak;

	if (Next && strLastSearchStr.IsEmpty())
		return TRUE;

	strSearchStr = strLastSearchStr;
	strReplaceStr = strLastReplaceStr;
	Case=LastSearchCase;
	WholeWords=LastSearchWholeWords;
	ReverseSearch=LastSearchReverse;
	SelectFound=LastSearchSelFound;
	Regexp=LastSearchRegexp;

	if (!Next)
	{
		if (EdOpt.SearchPickUpWord)
		{
			int StartPickPos=-1,EndPickPos=-1;
			const wchar_t *Ptr=CalcWordFromString(CurLine->GetStringAddr(),CurLine->GetCurPos(),&StartPickPos,&EndPickPos, GetWordDiv());

			if (Ptr)
			{
				string strWord(Ptr,(size_t)EndPickPos-StartPickPos+1);
				strSearchStr=strWord;
			}
		}

		if (!GetSearchReplaceString(ReplaceMode,&strSearchStr,
		                            &strReplaceStr,
		                            TextHistoryName,ReplaceHistoryName,
		                            &Case,&WholeWords,&ReverseSearch,&SelectFound,&Regexp,L"EditorSearch"))
			return FALSE;
	}

	strLastSearchStr = strSearchStr;
	strLastReplaceStr = strReplaceStr;
	LastSearchCase=Case;
	LastSearchWholeWords=WholeWords;
	LastSearchReverse=ReverseSearch;
	LastSearchSelFound=SelectFound;
	LastSearchRegexp=Regexp;

	if (strSearchStr.IsEmpty())
		return TRUE;

	LastSuccessfulReplaceMode=ReplaceMode;

	if (!EdOpt.PersistentBlocks || (SelectFound && !ReplaceMode))
		UnmarkBlock();

	{
		//SaveScreen SaveScr;
		TPreRedrawFuncGuard preRedrawFuncGuard(Editor::PR_EditorShowMsg);
		strMsgStr=strSearchStr;
		InsertQuote(strMsgStr);
		SetCursorType(FALSE,-1);
		Match=0;
		UserBreak=0;
		CurPos=CurLine->GetCurPos();
		/* $ 16.10.2000 tran
		   CurPos увеличивается при следующем поиске
		*/
		/* $ 28.11.2000 SVS
		   "О, это не ощибка - это свойство моей программы" :-)
		   Новое поведение стало подконтрольным
		*/
		/* $ 21.12.2000 SVS
		   - В предыдущем исправлении было задано неверное условие для
		     правила EditorF7Rules
		*/
		/* $ 10.06.2001 IS
		   - Баг: зачем-то при продолжении _обратного_ поиска прокручивались на шаг
		     _вперед_.
		*/

		/* $ 09.11.2001 IS
		     проклятое место, блин.
		     опять фиксим, т.к. не соответствует заявленному
		*/
		if (!ReverseSearch && (Next || (EdOpt.F7Rules && !ReplaceMode)))
			CurPos++;

		NewNumLine=NumLine;
		CurPtr=CurLine;
		DWORD StartTime=GetTickCount();
		int StartLine=NumLine;
		TaskBar TB;

		while (CurPtr)
		{
			DWORD CurTime=GetTickCount();

			if (CurTime-StartTime>RedrawTimeout)
			{
				StartTime=CurTime;

				if (CheckForEscSilent())
				{
					if (ConfirmAbortOp())
					{
						UserBreak=TRUE;
						break;
					}
				}

				strMsgStr=strSearchStr;
				InsertQuote(strMsgStr);
				SetCursorType(FALSE,-1);
				int Total=ReverseSearch?StartLine:NumLastLine-StartLine;
				int Current=abs(NewNumLine-StartLine);
				EditorShowMsg(MSG(MEditSearchTitle),MSG(MEditSearchingFor),strMsgStr,Current*100/Total);
				TBC.SetProgressValue(Current,Total);
			}

			int SearchLength=0;
			string strReplaceStrCurrent(ReplaceMode?strReplaceStr:L"");

			if (CurPtr->Search(strSearchStr,strReplaceStrCurrent,CurPos,Case,WholeWords,ReverseSearch,Regexp,&SearchLength))
			{
				if (SelectFound && !ReplaceMode)
				{
					Pasting++;
					Lock();
					UnmarkBlock();
					Flags.Set(FEDITOR_MARKINGBLOCK);
					int iFoundPos = CurPtr->GetCurPos();
					CurPtr->Select(iFoundPos, iFoundPos+SearchLength);
					BlockStart = CurPtr;
					BlockStartLine = NewNumLine;
					Unlock();
					Pasting--;
				}

				int Skip=FALSE;
				/* $ 24.01.2003 KM
				   ! По окончании поиска отступим от верха экрана на треть отображаемой высоты.
				*/
				/* $ 15.04.2003 VVM
				   Отступим на четверть и проверим на перекрытие диалогом замены */
				int FromTop=(ScrY-2)/4;

				if (FromTop<0 || FromTop>=((ScrY-5)/2-2))
					FromTop=0;

				TmpPtr=CurLine=CurPtr;

				for (int i=0; i<FromTop; i++)
				{
					if (TmpPtr->m_prev)
						TmpPtr=TmpPtr->m_prev;
					else
						break;
				}

				TopScreen=TmpPtr;
				NumLine=NewNumLine;
				int LeftPos=CurPtr->GetLeftPos();
				int TabCurPos=CurPtr->GetTabCurPos();

				if (ObjWidth>8 && TabCurPos-LeftPos+SearchLength>ObjWidth-8)
					CurPtr->SetLeftPos(TabCurPos+SearchLength-ObjWidth+8);

				if (ReplaceMode)
				{
					int MsgCode=0;

					if (!ReplaceAll)
					{
						Show();
						SHORT CurX,CurY;
						GetCursorPos(CurX,CurY);
						ScrBuf.ApplyColor(CurX,CurY,CurPtr->RealPosToTab(CurPtr->TabPosToReal(CurX)+SearchLength)-1,CurY,FarColorToReal(COL_EDITORSELECTEDTEXT));
						string strQSearchStr(CurPtr->GetStringAddr()+CurPtr->GetCurPos(),SearchLength), strQReplaceStr=strReplaceStrCurrent;
						InsertQuote(strQSearchStr);
						InsertQuote(strQReplaceStr);
						PreRedrawItem pitem=PreRedraw.Pop();
						MsgCode=Message(0,4,MSG(MEditReplaceTitle),MSG(MEditAskReplace),
						                strQSearchStr,MSG(MEditAskReplaceWith),strQReplaceStr,
						                MSG(MEditReplace),MSG(MEditReplaceAll),MSG(MEditSkip),MSG(MEditCancel));
						PreRedraw.Push(pitem);

						if (MsgCode==1)
							ReplaceAll=TRUE;

						if (MsgCode==2)
							Skip=TRUE;

						if (MsgCode<0 || MsgCode==3)
						{
							UserBreak=TRUE;
							break;
						}
					}

					if (!MsgCode || MsgCode==1)
					{
						Pasting++;

						/*$ 15.08.2000 skv
						  If Replace string doesn't contain control symbols (tab and return),
						  processed with fast method, otherwise use improved old one.
						*/
						if (strReplaceStrCurrent.Contains(L'\t') || strReplaceStrCurrent.Contains(L'\r'))
						{
							int SaveOvertypeMode=Flags.Check(FEDITOR_OVERTYPE);
							Flags.Set(FEDITOR_OVERTYPE);
							CurLine->SetOvertypeMode(TRUE);
							//int CurPos=CurLine->GetCurPos();

							int I=0;
							for (; SearchLength && strReplaceStrCurrent[I]; I++,SearchLength--)
							{
								int Ch=strReplaceStrCurrent[I];

								if (Ch==KEY_TAB)
								{
									Flags.Clear(FEDITOR_OVERTYPE);
									CurLine->SetOvertypeMode(FALSE);
									ProcessKey(KEY_DEL);
									ProcessKey(KEY_TAB);
									Flags.Set(FEDITOR_OVERTYPE);
									CurLine->SetOvertypeMode(TRUE);
									continue;
								}

								/* $ 24.05.2002 SKV
								  Если реплэйсим на Enter, то overtype не спасёт.
								  Нужно сначала удалить то, что заменяем.
								*/
								if (Ch==L'\r')
								{
									ProcessKey(KEY_DEL);
								}

								if (Ch!=KEY_BS && !(Ch==KEY_DEL || Ch==KEY_NUMDEL))
									ProcessKey(Ch);
							}

							if (!SearchLength)
							{
								Flags.Clear(FEDITOR_OVERTYPE);
								CurLine->SetOvertypeMode(FALSE);

								for (; strReplaceStrCurrent[I]; I++)
								{
									int Ch=strReplaceStrCurrent[I];

									if (Ch!=KEY_BS && !(Ch==KEY_DEL || Ch==KEY_NUMDEL))
										ProcessKey(Ch);
								}
							}
							else
							{
								for (; SearchLength; SearchLength--)
								{
									ProcessKey(KEY_DEL);
								}
							}

							int Cnt=0;
							const wchar_t *Tmp=strReplaceStrCurrent;

							while ((Tmp=wcschr(Tmp,L'\r')) )
							{
								Cnt++;
								Tmp++;
							}

							if (Cnt>0)
							{
								CurPtr=CurLine;
								NewNumLine+=Cnt;
							}

							Flags.Change(FEDITOR_OVERTYPE,SaveOvertypeMode);
						}
						else
						{
							/* Fast method */
							const wchar_t *Str,*Eol;
							int StrLen,NewStrLen;
							int SStrLen=SearchLength,
							            RStrLen=(int)strReplaceStrCurrent.GetLength();
							CurLine->GetBinaryString(&Str,&Eol,StrLen);
							int EolLen=StrLength(Eol);
							NewStrLen=StrLen;
							NewStrLen-=SStrLen;
							NewStrLen+=RStrLen;
							NewStrLen+=EolLen;
							wchar_t *NewStr=new wchar_t[NewStrLen+1];
							int CurPos=CurLine->GetCurPos();
							wmemcpy(NewStr,Str,CurPos);
							wmemcpy(NewStr+CurPos,strReplaceStrCurrent,RStrLen);
							wmemcpy(NewStr+CurPos+RStrLen,Str+CurPos+SStrLen,StrLen-CurPos-SStrLen);
							wmemcpy(NewStr+NewStrLen-EolLen,Eol,EolLen);
							AddUndoData(UNDO_EDIT,CurLine->GetStringAddr(),CurLine->GetEOL(),NumLine,CurLine->GetCurPos(),CurLine->GetLength());
							CurLine->SetBinaryString(NewStr,NewStrLen);
							CurLine->SetCurPos(CurPos+RStrLen);

							if (SelectFound && !ReplaceMode)
							{
								UnmarkBlock();
								Flags.Set(FEDITOR_MARKINGBLOCK);
								CurPtr->Select(CurPos, CurPos+RStrLen);
								BlockStart = CurPtr;
								BlockStartLine = NewNumLine;
							}

							delete [] NewStr;
							TextChanged(1);
						}

						/* skv$*/
						//AY: В этом нет никакой надобности и оно приводит к не правильному
						//позиционированию при Replace
						//if (ReverseSearch)
						//CurLine->SetCurPos(CurPos);
						Pasting--;
					}
				}

				Match=1;

				if (!ReplaceMode)
					break;

				CurPos=CurLine->GetCurPos();

				if (Skip)
					if (!ReverseSearch)
						CurPos++;
			}
			else
			{
				if (ReverseSearch)
				{
					CurPtr=CurPtr->m_prev;

					if (!CurPtr)
						break;

					CurPos=CurPtr->GetLength();
					NewNumLine--;
				}
				else
				{
					CurPos=0;
					CurPtr=CurPtr->m_next;
					NewNumLine++;
				}
			}
		}
	}
	Show();

	if (!Match && !UserBreak)
		Message(MSG_WARNING,1,MSG(MEditSearchTitle),MSG(MEditNotFound),
		        strMsgStr,MSG(MOk));

	return TRUE;
}

void Editor::Paste(const wchar_t *Src)
{
	if (Flags.Check(FEDITOR_LOCKMODE))
		return;

	wchar_t *ClipText=(wchar_t *)Src;
	BOOL IsDeleteClipText=FALSE;

	if (!ClipText)
	{
		Clipboard clip;

		if (!clip.Open())
			return;

		if (ClipText=clip.PasteFormat(FAR_VerticalBlock_Unicode))
		{
			VPaste(ClipText);
			clip.Close();
			return;
		}

		if (!(ClipText=clip.Paste()))
		{
			clip.Close();
			return;
		}

		clip.Close();

		IsDeleteClipText=TRUE;
	}

	if (*ClipText)
	{
		AddUndoData(UNDO_BEGIN);
		Flags.Set(FEDITOR_NEWUNDO);
		TextChanged(1);
		int SaveOvertype=Flags.Check(FEDITOR_OVERTYPE);
		UnmarkBlock();
		Pasting++;
		Lock();

		if (Flags.Check(FEDITOR_OVERTYPE))
		{
			Flags.Clear(FEDITOR_OVERTYPE);
			CurLine->SetOvertypeMode(FALSE);
		}

		BlockStart=CurLine;
		BlockStartLine=NumLine;
		/* $ 19.05.2001 IS
		   Решение проблемы непрошеной конвертации табуляции (которая должна быть
		   добавлена в начало строки при автоотступе) в пробелы.
		*/
		int StartPos=CurLine->GetCurPos();
		int oldAutoIndent=EdOpt.AutoIndent;

		for (int I=0; ClipText[I];)
		{
			if (ClipText[I]==L'\n' || ClipText[I]==L'\r')
			{
				CurLine->Select(StartPos,-1);
				StartPos=0;
				EdOpt.AutoIndent=FALSE;
				ProcessKey(KEY_ENTER);

				if (ClipText[I]==L'\r' && ClipText[I+1]==L'\n')
					I++;

				I++;
			}
			else
			{
				if (EdOpt.AutoIndent)      // первый символ вставим так, чтобы
				{                          // сработал автоотступ
					//ProcessKey(UseDecodeTable?TableSet.DecodeTable[(unsigned)ClipText[I]]:ClipText[I]); //BUGBUG
					ProcessKey(ClipText[I]); //BUGBUG
					I++;
					StartPos=CurLine->GetCurPos();

					if (StartPos) StartPos--;
				}

				int Pos=I;

				while (ClipText[Pos] && ClipText[Pos]!=L'\n' && ClipText[Pos]!=L'\r')
					Pos++;

				if (Pos>I)
				{
					const wchar_t *Str;
					int Length,CurPos;
					CurLine->GetBinaryString(&Str,nullptr,Length);
					CurPos=CurLine->GetCurPos();
					AddUndoData(UNDO_EDIT,Str,CurLine->GetEOL(),NumLine,CurPos,Length); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
					CurLine->InsertBinaryString(&ClipText[I],Pos-I);
				}

				I=Pos;
			}
		}

		EdOpt.AutoIndent=oldAutoIndent;
		CurLine->Select(StartPos,CurLine->GetCurPos());
		/* IS $ */

		if (SaveOvertype)
		{
			Flags.Set(FEDITOR_OVERTYPE);
			CurLine->SetOvertypeMode(TRUE);
		}

		Pasting--;
		Unlock();
		AddUndoData(UNDO_END);
	}

	if (IsDeleteClipText)
		xf_free(ClipText);
}


void Editor::Copy(int Append)
{
	if (VBlockStart)
	{
		VCopy(Append);
		return;
	}

	wchar_t *CopyData=nullptr;

	Clipboard clip;

	if (!clip.Open())
		return;

	if (Append)
		CopyData=clip.Paste();

	if ((CopyData=Block2Text(CopyData)) )
	{
		clip.Copy(CopyData);
		xf_free(CopyData);
	}

	clip.Close();
}

wchar_t *Editor::Block2Text(wchar_t *ptrInitData)
{
	size_t DataSize=0;

	if (ptrInitData)
		DataSize = wcslen(ptrInitData);

	size_t TotalChars = DataSize;
	for (Edit *Ptr=BlockStart; Ptr; Ptr=Ptr->m_next)
	{
		TotalChars += Ptr->GetLength();
		TotalChars += 2; // CRLF
	}
	TotalChars++; // '\0'

	wchar_t *CopyData=(wchar_t *)xf_malloc(TotalChars*sizeof(wchar_t));

	if (!CopyData)
	{
		if (ptrInitData)
			xf_free(ptrInitData);

		return nullptr;
	}

	if (ptrInitData)
	{
		wcscpy(CopyData,ptrInitData);
		xf_free(ptrInitData);
	}
	else
	{
		*CopyData=0;
	}

	Edit *CurPtr=BlockStart;

	while (CurPtr)
	{
		int StartSel,EndSel;
		int Length=CurPtr->GetLength()+1;
		CurPtr->GetSelection(StartSel,EndSel);

		if (StartSel==-1)
			break;

		CurPtr->GetSelString(CopyData+DataSize,Length);
		DataSize+=StrLength(CopyData+DataSize);

		if (EndSel==-1)
		{
			wcscpy(CopyData+DataSize,DOS_EOL_fmt);
			DataSize+=2;
		}

		CurPtr=CurPtr->m_next;
	}

	return CopyData;
}


void Editor::DeleteBlock()
{
	if (Flags.Check(FEDITOR_LOCKMODE))
		return;

	if (VBlockStart)
	{
		DeleteVBlock();
		return;
	}

	Edit *CurPtr=BlockStart;
	AddUndoData(UNDO_BEGIN);

	while (CurPtr)
	{
		TextChanged(1);
		int StartSel,EndSel;
		/* $ 17.09.2002 SKV
		  меняем на Real что б ловить выделение за концом строки.
		*/
		CurPtr->GetRealSelection(StartSel,EndSel);

		if (EndSel!=-1 && EndSel>CurPtr->GetLength())
			EndSel=-1;

		if (StartSel==-1)
			break;

		if (!StartSel && EndSel==-1)
		{
			Edit *NextLine=CurPtr->m_next;
			DeleteString(CurPtr,FALSE,BlockStartLine);

			if (BlockStartLine<NumLine)
				NumLine--;

			if (NextLine)
			{
				CurPtr=NextLine;
				continue;
			}
			else
				break;
		}

		int Length=CurPtr->GetLength();

		if (StartSel || EndSel)
			AddUndoData(UNDO_EDIT,CurPtr->GetStringAddr(),CurPtr->GetEOL(),BlockStartLine,CurPtr->GetCurPos(),CurPtr->GetLength());

		/* $ 17.09.2002 SKV
		  опять про выделение за концом строки.
		  InsertBinaryString добавит trailing space'ов
		*/
		if (StartSel>Length)
		{
			Length=StartSel;
			CurPtr->SetCurPos(Length);
			CurPtr->InsertBinaryString(L"",0);
		}

		const wchar_t *CurStr,*EndSeq;

		CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);

		// дальше будет realloc, поэтому тут malloc.
		wchar_t *TmpStr=(wchar_t*)xf_malloc((Length+3)*sizeof(wchar_t));

		wmemcpy(TmpStr,CurStr,Length);

		TmpStr[Length]=0;

		int DeleteNext=FALSE;

		if (EndSel==-1)
		{
			EndSel=Length;

			if (CurPtr->m_next)
				DeleteNext=TRUE;
		}

		// wmemmove(TmpStr+StartSel,TmpStr+EndSel,StrLength(TmpStr+EndSel)+1);
		wmemmove(TmpStr+StartSel,TmpStr+EndSel,Length-EndSel+1);
		int CurPos=StartSel;
		/*    if (CurPos>=StartSel)
		    {
		      CurPos-=(EndSel-StartSel);
		      if (CurPos<StartSel)
		        CurPos=StartSel;
		    }
		*/
		Length-=EndSel-StartSel;

		if (DeleteNext)
		{
			const wchar_t *NextStr,*EndSeq;
			int NextLength,NextStartSel,NextEndSel;
			CurPtr->m_next->GetSelection(NextStartSel,NextEndSel);

			if (NextStartSel==-1)
				NextEndSel=0;

			if (NextEndSel==-1)
				EndSel=-1;
			else
			{
				CurPtr->m_next->GetBinaryString(&NextStr,&EndSeq,NextLength);
				NextLength-=NextEndSel;

				if (NextLength>0)
				{
					TmpStr=(wchar_t *)xf_realloc(TmpStr,(Length+NextLength+3)*sizeof(wchar_t));
					wmemcpy(TmpStr+Length,NextStr+NextEndSel,NextLength);
					Length+=NextLength;
				}
			}

			if (CurLine==CurPtr->m_next)
			{
				CurLine=CurPtr;
				NumLine--;
			}

			if (CurLine==CurPtr && CurPtr->m_next && CurPtr->m_next==TopScreen)
			{
				TopScreen=CurPtr;
			}

			DeleteString(CurPtr->m_next,FALSE,BlockStartLine+1);

			if (BlockStartLine+1<NumLine)
				NumLine--;
		}

		int EndLength=StrLength(EndSeq);
		wmemcpy(TmpStr+Length,EndSeq,EndLength);
		Length+=EndLength;
		CurPtr->SetBinaryString(TmpStr,Length);
		xf_free(TmpStr);
		CurPtr->SetCurPos(CurPos);

		if (DeleteNext && EndSel==-1)
		{
			CurPtr->Select(CurPtr->GetLength(),-1);
		}
		else
		{
			CurPtr->Select(-1,0);
			CurPtr=CurPtr->m_next;
			BlockStartLine++;
		}
	}

	AddUndoData(UNDO_END);
	BlockStart=nullptr;
}


void Editor::UnmarkBlock()
{
	if (!BlockStart && !VBlockStart)
		return;

	VBlockStart=nullptr;
	_SVS(SysLog(L"[%d] Editor::UnmarkBlock()",__LINE__));
	Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);

	while (BlockStart)
	{
		int StartSel,EndSel;
		BlockStart->GetSelection(StartSel,EndSel);

		if (StartSel==-1)
		{
			/* $ 24.06.2002 SKV
			  Если в текущей строки нет выделения,
			  это еще не значит что мы в конце.
			  Это может быть только начало :)
			*/
			if (BlockStart->m_next)
			{
				BlockStart->m_next->GetSelection(StartSel,EndSel);

				if (StartSel==-1)
				{
					break;
				}
			}
			else
				break;
		}

		BlockStart->Select(-1,0);
		BlockStart=BlockStart->m_next;
	}

	BlockStart=nullptr;
	Show();
}

/* $ 07.03.2002 IS
   Удалить выделение, если оно пустое (выделено ноль символов в ширину)
*/
void Editor::UnmarkEmptyBlock()
{
	_SVS(SysLog(L"[%d] Editor::UnmarkEmptyBlock()",__LINE__));

	if (BlockStart || VBlockStart) // присутствует выделение
	{
		int Lines=0,StartSel,EndSel;
		Edit *Block=BlockStart;

		if (VBlockStart)
		{
			if (VBlockSizeX)
				Lines=VBlockSizeY;
		}
		else while (Block) // пробегаем по всем выделенным строкам
			{
				Block->GetRealSelection(StartSel,EndSel);

				if (StartSel==-1)
					break;

				if (StartSel!=EndSel)// выделено сколько-то символов
				{
					++Lines;           // увеличим счетчик непустых строк
					break;
				}

				Block=Block->m_next;
			}

		if (!Lines)            // если выделено ноль символов в ширину, то
			UnmarkBlock();       // перестанем морочить голову и снимем выделение
	}
}

void Editor::UnmarkMacroBlock()
{
	MBlockStart=nullptr;
	MBlockStartX=-1;
}

void Editor::GoToLine(int Line)
{
	if (Line != NumLine)
	{
		bool bReverse = false;
		int LastNumLine=NumLine;
		int CurScrLine=CalcDistance(TopScreen,CurLine,-1);
		int CurPos=CurLine->GetTabCurPos();
		int LeftPos=CurLine->GetLeftPos();

		if (Line < NumLine)
		{
			if (Line > NumLine/2)
			{
				bReverse = true;
			}
			else
			{
				CurLine = TopList;
				NumLine = 0;
			}
		}
		else
		{
			if (Line > (NumLine + (NumLastLine-NumLine)/2))
			{
				bReverse = true;
				CurLine = EndList;
				NumLine = NumLastLine-1;
			}
		}

		if (bReverse)
		{
			for (; NumLine>Line && CurLine->m_prev; NumLine--)
				CurLine=CurLine->m_prev;
		}
		else
		{
			for (; NumLine<Line && CurLine->m_next; NumLine++)
				CurLine=CurLine->m_next;
		}

		CurScrLine+=NumLine-LastNumLine;

		if (CurScrLine<0 || CurScrLine>Y2-Y1)
			TopScreen=CurLine;

		CurLine->SetLeftPos(LeftPos);
		CurLine->SetTabCurPos(CurPos);
	}

// <GOTO_UNMARK:2>
//  if (!EdOpt.PersistentBlocks)
//     UnmarkBlock();
// </GOTO_UNMARK>
	Show();
	return;
}

void Editor::GoToPosition()
{
	DialogBuilder Builder(MEditGoToLine, L"EditorGotoPos");
	string strData;
	Builder.AddEditField(&strData,28,L"LineNumber",DIF_FOCUS|DIF_HISTORY|DIF_USELASTHISTORY|DIF_NOAUTOCOMPLETE);
	Builder.AddOKCancel();
	Builder.ShowDialog();
	if(!strData.IsEmpty())
	{
		int LeftPos=CurLine->GetTabCurPos()+1;
		int CurPos=CurLine->GetCurPos();

		int NewLine=0, NewCol=0;
		GetRowCol(strData,&NewLine,&NewCol);
		GoToLine(NewLine);

		if (NewCol == -1)
		{
			CurLine->SetTabCurPos(CurPos);
			CurLine->SetLeftPos(LeftPos);
		}
		else
		{
			CurLine->SetTabCurPos(NewCol);
		}
		Show();
	}
}

void Editor::GetRowCol(const wchar_t *_argv,int *row,int *col)
{
	int x=0xffff,y;
	int l;
	wchar_t *argvx=0;
	int LeftPos=CurLine->GetTabCurPos() + 1;
	string strArg = _argv;
	// что бы не оставить "врагу" выбора - только то, что мы хотим ;-)
	// "прибьем" все внешние пробелы.
	RemoveExternalSpaces(strArg);
	wchar_t *argv = strArg.GetBuffer();
	// получаем индекс вхождения любого разделителя
	// в искомой строке
	l=(int)wcscspn(argv,L",:;. ");
	// если разделителя нету, то l=strlen(argv)

	if (l < StrLength(argv)) // Варианты: "row,col" или ",col"?
	{
		argv[l]=L'\0'; // Вместо разделителя впиндюлим "конец строки" :-)
		argvx=argv+l+1;
		x=_wtoi(argvx);
	}

	y=_wtoi(argv);

	// + переход на проценты
	if (wcschr(argv,L'%'))
		y=NumLastLine * y / 100;

	//   вычисляем относительность
	if (argv[0]==L'-' || argv[0]==L'+')
		y=NumLine+y+1;

	if (argvx)
	{
		if (argvx[0]==L'-' || argvx[0]==L'+')
		{
			x=LeftPos+x;
		}
	}

	strArg.ReleaseBuffer();
	// теперь загоним результат назад
	*row=y;

	if (x!=0xffff)
		*col=x;
	else
		*col=LeftPos;

	(*row)--;

	if (*row< 0)   // если ввели ",Col"
		*row=NumLine;  //   то переходим на текущую строку и колонку

	(*col)--;

	if (*col< -1)
		*col=-1;

	return ;
}

void Editor::AddUndoData(int Type,const wchar_t *Str,const wchar_t *Eol,int StrNum,int StrPos,int Length)
{
	if (Flags.Check(FEDITOR_DISABLEUNDO))
		return;

	if (StrNum==-1)
		StrNum=NumLine;

	for (EditorUndoData *u=UndoData.Next(UndoPos); u;)
	{
		if (u==UndoSavePos)
		{
			UndoSavePos=nullptr;
			Flags.Set(FEDITOR_UNDOSAVEPOSLOST);
		}

		EditorUndoData *nu=UndoData.Next(u);
		UndoData.Delete(u);
		u=nu;
	}

	EditorUndoData *PrevUndo=UndoData.Last();

	if (Type==UNDO_END)
	{
		if (PrevUndo && PrevUndo->Type!=UNDO_BEGIN)
			PrevUndo=UndoData.Prev(PrevUndo);

		if (PrevUndo && PrevUndo->Type==UNDO_BEGIN)
		{
			UndoData.Delete(PrevUndo);
			UndoPos=UndoData.Last();

			if (PrevUndo==UndoSavePos)
				UndoSavePos=UndoPos;

			return;
		}
	}

	if (Type==UNDO_EDIT && !Flags.Check(FEDITOR_NEWUNDO))
	{
		if (PrevUndo && PrevUndo->Type==UNDO_EDIT && StrNum==PrevUndo->StrNum &&
		        (abs(StrPos-PrevUndo->StrPos)<=1 || abs(StrPos-LastChangeStrPos)<=1))
		{
			LastChangeStrPos=StrPos;
			return;
		}
	}

	Flags.Clear(FEDITOR_NEWUNDO);
	UndoPos=UndoData.Push();
	UndoPos->SetData(Type,Str,Eol,StrNum,StrPos,Length);

	if (EdOpt.UndoSize>0)
	{
		while (!UndoData.Empty() && (UndoData.Count()>static_cast<size_t>(EdOpt.UndoSize) || UndoSkipLevel>0))
		{
			EditorUndoData *u=UndoData.First();

			if (u->Type==UNDO_BEGIN)
				++UndoSkipLevel;

			if (u->Type==UNDO_END && UndoSkipLevel>0)
				--UndoSkipLevel;

			if (!UndoSavePos)
				Flags.Set(FEDITOR_UNDOSAVEPOSLOST);

			if (u==UndoSavePos)
				UndoSavePos=nullptr;

			UndoData.Delete(u);
		}

		UndoPos=UndoData.Last();
	}
}

void Editor::Undo(int redo)
{
	EditorUndoData *ustart=redo ? UndoData.Next(UndoPos) : UndoPos;

	if (!ustart)
		return;

	TextChanged(1);
	Flags.Set(FEDITOR_DISABLEUNDO);
	int level=0;
	EditorUndoData *uend;

	for (uend=ustart; uend; uend=redo ? UndoData.Next(uend) : UndoData.Prev(uend))
	{
		if (uend->Type==UNDO_BEGIN || uend->Type==UNDO_END)
		{
			int l=uend->Type==UNDO_BEGIN ? -1 : 1;
			level+=redo ? -l : l;
		}

		if (level<=0)
			break;
	}

	if (level)
		uend=ustart;

	UnmarkBlock();
	EditorUndoData *ud=ustart;

	for (;;)
	{
		if (ud->Type!=UNDO_BEGIN && ud->Type!=UNDO_END)
			GoToLine(ud->StrNum);

		switch (ud->Type)
		{
			case UNDO_INSSTR:
				ud->SetData(UNDO_DELSTR,CurLine->GetStringAddr(),CurLine->GetEOL(),ud->StrNum,ud->StrPos,CurLine->GetLength());
				DeleteString(CurLine,TRUE,NumLine>0 ? NumLine-1:NumLine);
				break;
			case UNDO_DELSTR:
				ud->Type=UNDO_INSSTR;
				Pasting++;

				if (NumLine<ud->StrNum)
				{
					ProcessKey(KEY_END);
					ProcessKey(KEY_ENTER);
				}
				else
				{
					ProcessKey(KEY_HOME);
					ProcessKey(KEY_ENTER);
					ProcessKey(KEY_UP);
				}

				Pasting--;

				if (ud->Str)
				{
					CurLine->SetString(ud->Str,ud->Length);
					CurLine->SetEOL(ud->EOL); // необходимо дополнительно выставлять, т.к. SetString вызывает Edit::SetBinaryString и... дальше по тексту
				}

				break;
			case UNDO_EDIT:
			{
				EditorUndoData tmp;
				tmp.SetData(UNDO_EDIT,CurLine->GetStringAddr(),CurLine->GetEOL(),ud->StrNum,ud->StrPos,CurLine->GetLength());

				if (ud->Str)
				{
					CurLine->SetString(ud->Str,ud->Length);
					CurLine->SetEOL(ud->EOL); // необходимо дополнительно выставлять, т.к. SetString вызывает Edit::SetBinaryString и... дальше по тексту
				}

				CurLine->SetCurPos(ud->StrPos);
				ud->SetData(tmp.Type,tmp.Str,tmp.EOL,tmp.StrNum,tmp.StrPos,tmp.Length);
				break;
			}
		}

		if (ud==uend)
			break;

		ud=redo ? UndoData.Next(ud) : UndoData.Prev(ud);
	}

	UndoPos=redo ? ud : UndoData.Prev(ud);

	if (!Flags.Check(FEDITOR_UNDOSAVEPOSLOST) && UndoPos==UndoSavePos)
		TextChanged(0);

	Flags.Clear(FEDITOR_DISABLEUNDO);
}

void Editor::SelectAll()
{
	Edit *CurPtr;
	BlockStart=TopList;
	BlockStartLine=0;

	for (CurPtr=TopList; CurPtr; CurPtr=CurPtr->m_next)
		if (CurPtr->m_next)
			CurPtr->Select(0,-1);
		else
			CurPtr->Select(0,CurPtr->GetLength());

	Show();
}


void Editor::SetStartPos(int LineNum,int CharNum)
{
	StartLine=LineNum? LineNum:1;
	StartChar=CharNum? CharNum:1;
}


BOOL Editor::IsFileChanged() const
{
	return Flags.Check(FEDITOR_MODIFIED|FEDITOR_WASCHANGED);
}


BOOL Editor::IsFileModified() const
{
	return Flags.Check(FEDITOR_MODIFIED);
}

// используется в FileEditor
long Editor::GetCurPos()
{
	Edit *CurPtr=TopList;
	long TotalSize=0;

	while (CurPtr!=TopScreen)
	{
		const wchar_t *SaveStr,*EndSeq;
		int Length;
		CurPtr->GetBinaryString(&SaveStr,&EndSeq,Length);
		TotalSize+=Length+StrLength(EndSeq);
		CurPtr=CurPtr->m_next;
	}

	return(TotalSize);
}


/*
void Editor::SetStringsTable()
{
  Edit *CurPtr=TopList;
  while (CurPtr)
  {
    CurPtr->SetTables(UseDecodeTable ? &TableSet:nullptr);
    CurPtr=CurPtr->m_next;
  }
}
*/


void Editor::BlockLeft()
{
	if (VBlockStart)
	{
		VBlockShift(TRUE);
		return;
	}

	Edit *CurPtr=BlockStart;
	int LineNum=BlockStartLine;
	/* $ 14.02.2001 VVM
	  + При отсутствии блока AltU/AltI сдвигают текущую строчку */
	int MoveLine = 0;

	if (!CurPtr)
	{
		MoveLine = 1;
		CurPtr = CurLine;
		LineNum = NumLine;
	}

	AddUndoData(UNDO_BEGIN);

	while (CurPtr)
	{
		int StartSel,EndSel;
		CurPtr->GetSelection(StartSel,EndSel);

		/* $ 14.02.2001 VVM
		  + Блока нет - сделаем его искусственно */
		if (MoveLine)
		{
			StartSel = 0; EndSel = -1;
		}

		if (StartSel==-1)
			break;

		int Length=CurPtr->GetLength();
		wchar_t *TmpStr=new wchar_t[Length+EdOpt.TabSize+5];
		const wchar_t *CurStr,*EndSeq;
		CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);
		Length--;

		if (*CurStr==L' ')
			wmemcpy(TmpStr,CurStr+1,Length);
		else if (*CurStr==L'\t')
		{
			wmemset(TmpStr, L' ', EdOpt.TabSize-1);
			wmemcpy(TmpStr+EdOpt.TabSize-1,CurStr+1,Length);
			Length+=EdOpt.TabSize-1;
		}

		if ((EndSel==-1 || EndSel>StartSel) && IsSpace(*CurStr))
		{
			int EndLength=StrLength(EndSeq);
			wmemcpy(TmpStr+Length,EndSeq,EndLength);
			Length+=EndLength;
			TmpStr[Length]=0;
			AddUndoData(UNDO_EDIT,CurStr,CurPtr->GetEOL(),LineNum,0,CurPtr->GetLength()); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
			int CurPos=CurPtr->GetCurPos();
			CurPtr->SetBinaryString(TmpStr,Length);
			CurPtr->SetCurPos(CurPos>0 ? CurPos-1:CurPos);

			if (!MoveLine)
				CurPtr->Select(StartSel>0 ? StartSel-1:StartSel,EndSel>0 ? EndSel-1:EndSel);

			TextChanged(1);
		}

		delete[] TmpStr;
		CurPtr=CurPtr->m_next;
		LineNum++;
		MoveLine = 0;
	}

	AddUndoData(UNDO_END);
}


void Editor::BlockRight()
{
	if (VBlockStart)
	{
		VBlockShift(FALSE);
		return;
	}

	Edit *CurPtr=BlockStart;
	int LineNum=BlockStartLine;
	/* $ 14.02.2001 VVM
	  + При отсутствии блока AltU/AltI сдвигают текущую строчку */
	int MoveLine = 0;

	if (!CurPtr)
	{
		MoveLine = 1;
		CurPtr = CurLine;
		LineNum = NumLine;
	}

	AddUndoData(UNDO_BEGIN);

	while (CurPtr)
	{
		int StartSel,EndSel;
		CurPtr->GetSelection(StartSel,EndSel);

		/* $ 14.02.2001 VVM
		  + Блока нет - сделаем его искусственно */
		if (MoveLine)
		{
			StartSel = 0; EndSel = -1;
		}

		if (StartSel==-1)
			break;

		int Length=CurPtr->GetLength();
		wchar_t *TmpStr=new wchar_t[Length+5];
		const wchar_t *CurStr,*EndSeq;
		CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);
		*TmpStr=L' ';
		wmemcpy(TmpStr+1,CurStr,Length);
		Length++;

		if (EndSel==-1 || EndSel>StartSel)
		{
			int EndLength=StrLength(EndSeq);
			wmemcpy(TmpStr+Length,EndSeq,EndLength);
			TmpStr[Length+EndLength]=0;
			AddUndoData(UNDO_EDIT,CurStr,CurPtr->GetEOL(),LineNum,0,CurPtr->GetLength()); // EOL? - CurLine->GetEOL()  GlobalEOL   ""
			int CurPos=CurPtr->GetCurPos();

			if (Length>1)
				CurPtr->SetBinaryString(TmpStr,Length+EndLength);

			CurPtr->SetCurPos(CurPos+1);

			if (!MoveLine)
				CurPtr->Select(StartSel>0 ? StartSel+1:StartSel,EndSel>0 ? EndSel+1:EndSel);

			TextChanged(1);
		}

		delete[] TmpStr;
		CurPtr=CurPtr->m_next;
		LineNum++;
		MoveLine = 0;
	}

	AddUndoData(UNDO_END);
}


void Editor::DeleteVBlock()
{
	if (Flags.Check(FEDITOR_LOCKMODE) || VBlockSizeX<=0 || VBlockSizeY<=0)
		return;

	AddUndoData(UNDO_BEGIN);

	if (!EdOpt.PersistentBlocks)
	{
		Edit *CurPtr=CurLine;
		Edit *NewTopScreen=TopScreen;

		while (CurPtr)
		{
			if (CurPtr==VBlockStart)
			{
				TopScreen=NewTopScreen;
				CurLine=CurPtr;
				CurPtr->SetTabCurPos(VBlockX);
				break;
			}

			NumLine--;

			if (NewTopScreen==CurPtr && CurPtr->m_prev)
				NewTopScreen=CurPtr->m_prev;

			CurPtr=CurPtr->m_prev;
		}
	}

	Edit *CurPtr=VBlockStart;

	for (int Line=0; CurPtr && Line<VBlockSizeY; Line++,CurPtr=CurPtr->m_next)
	{
		TextChanged(1);
		int TBlockX=CurPtr->TabPosToReal(VBlockX);
		int TBlockSizeX=CurPtr->TabPosToReal(VBlockX+VBlockSizeX)-
		                CurPtr->TabPosToReal(VBlockX);
		const wchar_t *CurStr,*EndSeq;
		int Length;
		CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);

		if (TBlockX>=Length)
			continue;

		AddUndoData(UNDO_EDIT,CurPtr->GetStringAddr(),CurPtr->GetEOL(),BlockStartLine+Line,CurPtr->GetCurPos(),CurPtr->GetLength());
		wchar_t *TmpStr=new wchar_t[Length+3];
		int CurLength=TBlockX;
		wmemcpy(TmpStr,CurStr,TBlockX);

		if (Length>TBlockX+TBlockSizeX)
		{
			int CopySize=Length-(TBlockX+TBlockSizeX);
			wmemcpy(TmpStr+CurLength,CurStr+TBlockX+TBlockSizeX,CopySize);
			CurLength+=CopySize;
		}

		int EndLength=StrLength(EndSeq);
		wmemcpy(TmpStr+CurLength,EndSeq,EndLength);
		CurLength+=EndLength;
		int CurPos=CurPtr->GetCurPos();
		CurPtr->SetBinaryString(TmpStr,CurLength);

		if (CurPos>TBlockX)
		{
			CurPos-=TBlockSizeX;

			if (CurPos<TBlockX)
				CurPos=TBlockX;
		}

		CurPtr->SetCurPos(CurPos);
		delete[] TmpStr;
	}

	AddUndoData(UNDO_END);
	VBlockStart=nullptr;
}

void Editor::VCopy(int Append)
{
	wchar_t *CopyData=nullptr;

	Clipboard clip;

	if (!clip.Open())
		return;

	if (Append)
	{
		CopyData=clip.PasteFormat(FAR_VerticalBlock_Unicode);

		if (!CopyData)
			CopyData=clip.Paste();
	}

	if ((CopyData=VBlock2Text(CopyData)) )
	{
		clip.Copy(CopyData);
		clip.CopyFormat(FAR_VerticalBlock_Unicode,CopyData);
		xf_free(CopyData);
	}

	clip.Close();
}

wchar_t *Editor::VBlock2Text(wchar_t *ptrInitData)
{
	size_t DataSize=0;

	if (ptrInitData)
		DataSize = wcslen(ptrInitData);

	//RealPos всегда <= TabPos, поэтому берём максимальный размер буффера
	size_t TotalChars = DataSize + (VBlockSizeX + 2)*VBlockSizeY + 1;

	wchar_t *CopyData=(wchar_t *)xf_malloc(TotalChars*sizeof(wchar_t));

	if (!CopyData)
	{
		if (ptrInitData)
			xf_free(ptrInitData);

		return nullptr;
	}

	if (ptrInitData)
	{
		wcscpy(CopyData,ptrInitData);
		xf_free(ptrInitData);
	}
	else
	{
		*CopyData=0;
	}

	Edit *CurPtr=VBlockStart;

	for (int Line=0; CurPtr && Line<VBlockSizeY; Line++,CurPtr=CurPtr->m_next)
	{
		int TBlockX=CurPtr->TabPosToReal(VBlockX);
		int TBlockSizeX=CurPtr->TabPosToReal(VBlockX+VBlockSizeX)-TBlockX;
		const wchar_t *CurStr,*EndSeq;
		int Length;
		CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);

		if (Length>TBlockX)
		{
			int CopySize=Length-TBlockX;

			if (CopySize>TBlockSizeX)
				CopySize=TBlockSizeX;

			wmemcpy(CopyData+DataSize,CurStr+TBlockX,CopySize);

			if (CopySize<TBlockSizeX)
				wmemset(CopyData+DataSize+CopySize,L' ',TBlockSizeX-CopySize);
		}
		else
		{
			wmemset(CopyData+DataSize,L' ',TBlockSizeX);
		}

		DataSize+=TBlockSizeX;
		wcscpy(CopyData+DataSize,DOS_EOL_fmt);
		DataSize+=2;
	}

	return CopyData;
}



void Editor::VPaste(wchar_t *ClipText)
{
	if (Flags.Check(FEDITOR_LOCKMODE))
		return;

	if (*ClipText)
	{
		AddUndoData(UNDO_BEGIN);
		Flags.Set(FEDITOR_NEWUNDO);
		TextChanged(1);
		int SaveOvertype=Flags.Check(FEDITOR_OVERTYPE);
		UnmarkBlock();
		Pasting++;
		Lock();

		if (Flags.Check(FEDITOR_OVERTYPE))
		{
			Flags.Clear(FEDITOR_OVERTYPE);
			CurLine->SetOvertypeMode(FALSE);
		}

		VBlockStart=CurLine;
		BlockStartLine=NumLine;
		int StartPos=CurLine->GetTabCurPos();
		VBlockX=StartPos;
		VBlockSizeX=0;
		VBlockY=NumLine;
		VBlockSizeY=0;
		Edit *SavedTopScreen=TopScreen;

		for (int I=0; ClipText[I]; I++)
			if (ClipText[I]!=L'\r' && ClipText[I+1]!=L'\n')
				ProcessKey(ClipText[I]);
			else
			{
				int CurWidth=CurLine->GetTabCurPos()-StartPos;

				if (CurWidth>VBlockSizeX)
					VBlockSizeX=CurWidth;

				VBlockSizeY++;

				if (!CurLine->m_next)
				{
					if (ClipText[I+2])
					{
						ProcessKey(KEY_END);
						ProcessKey(KEY_ENTER);

						/* $ 19.05.2001 IS
						   Не вставляем пробелы тогда, когда нас об этом не просят, а
						   именно - при включенном автоотступе ничего вставлять не нужно,
						   оно само вставится и в другом месте.
						*/
						if (!EdOpt.AutoIndent)
							for (int I=0; I<StartPos; I++)
								ProcessKey(L' ');
					}
				}
				else
				{
					ProcessKey(KEY_DOWN);
					CurLine->SetTabCurPos(StartPos);
					CurLine->SetOvertypeMode(FALSE);
				}

				I++;
				continue;
			}

		int CurWidth=CurLine->GetTabCurPos()-StartPos;

		if (CurWidth>VBlockSizeX)
			VBlockSizeX=CurWidth;

		if (!VBlockSizeY)
			VBlockSizeY++;

		if (SaveOvertype)
		{
			Flags.Set(FEDITOR_OVERTYPE);
			CurLine->SetOvertypeMode(TRUE);
		}

		TopScreen=SavedTopScreen;
		CurLine=VBlockStart;
		NumLine=BlockStartLine;
		CurLine->SetTabCurPos(StartPos);
		Pasting--;
		Unlock();
		AddUndoData(UNDO_END);
	}

	xf_free(ClipText);
}


void Editor::VBlockShift(int Left)
{
	if (Flags.Check(FEDITOR_LOCKMODE) || (Left && !VBlockX) || VBlockSizeX<=0 || VBlockSizeY<=0)
		return;

	Edit *CurPtr=VBlockStart;
	AddUndoData(UNDO_BEGIN);

	for (int Line=0; CurPtr && Line<VBlockSizeY; Line++,CurPtr=CurPtr->m_next)
	{
		TextChanged(1);
		int TBlockX=CurPtr->TabPosToReal(VBlockX);
		int TBlockSizeX=CurPtr->TabPosToReal(VBlockX+VBlockSizeX)-
		                CurPtr->TabPosToReal(VBlockX);
		const wchar_t *CurStr,*EndSeq;
		int Length;
		CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);

		if (TBlockX>Length)
			continue;

		if ((Left && CurStr[TBlockX-1]==L'\t') ||
		        (!Left && TBlockX+TBlockSizeX<Length && CurStr[TBlockX+TBlockSizeX]==L'\t'))
		{
			CurPtr->ReplaceTabs();
			CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);
			TBlockX=CurPtr->TabPosToReal(VBlockX);
			TBlockSizeX=CurPtr->TabPosToReal(VBlockX+VBlockSizeX)-
			            CurPtr->TabPosToReal(VBlockX);
		}

		AddUndoData(UNDO_EDIT,CurPtr->GetStringAddr(),CurPtr->GetEOL(),BlockStartLine+Line,CurPtr->GetCurPos(),CurPtr->GetLength());
		int StrLen=Max(Length,TBlockX+TBlockSizeX+!Left);
		wchar_t *TmpStr=new wchar_t[StrLen+3];
		wmemset(TmpStr,L' ',StrLen);
		wmemcpy(TmpStr,CurStr,Length);

		if (Left)
		{
			WCHAR Ch=TmpStr[TBlockX-1];

			for (int I=TBlockX; I<TBlockX+TBlockSizeX; I++)
				TmpStr[I-1]=TmpStr[I];

			TmpStr[TBlockX+TBlockSizeX-1]=Ch;
		}
		else
		{
			int Ch=TmpStr[TBlockX+TBlockSizeX];

			for (int I=TBlockX+TBlockSizeX-1; I>=TBlockX; I--)
				TmpStr[I+1]=TmpStr[I];

			TmpStr[TBlockX]=Ch;
		}

		while (StrLen>0 && TmpStr[StrLen-1]==L' ')
			StrLen--;

		int EndLength=StrLength(EndSeq);
		wmemcpy(TmpStr+StrLen,EndSeq,EndLength);
		StrLen+=EndLength;
		CurPtr->SetBinaryString(TmpStr,StrLen);
		delete[] TmpStr;
	}

	VBlockX+=Left ? -1:1;
	CurLine->SetTabCurPos(Left ? VBlockX:VBlockX+VBlockSizeX);
	AddUndoData(UNDO_END);
}


int Editor::EditorControl(int Command,void *Param)
{
	_ECTLLOG(CleverSysLog SL(L"Editor::EditorControl()"));
	_ECTLLOG(SysLog(L"Command=%s Param=[%d/0x%08X]",_ECTL_ToName(Command),Param,Param));

	switch (Command)
	{
		case ECTL_GETSTRING:
		{
			EditorGetString *GetString=(EditorGetString *)Param;

			if (GetString)
			{
				Edit *CurPtr=GetStringByNumber(GetString->StringNumber);

				if (!CurPtr)
				{
					_ECTLLOG(SysLog(L"EditorGetString => GetStringByNumber(%d) return nullptr",GetString->StringNumber));
					return FALSE;
				}

				CurPtr->GetBinaryString(const_cast<const wchar_t **>(&GetString->StringText),
				                        const_cast<const wchar_t **>(&GetString->StringEOL),
				                        GetString->StringLength);
				GetString->SelStart=-1;
				GetString->SelEnd=0;
				int DestLine=GetString->StringNumber;

				if (DestLine==-1)
					DestLine=NumLine;

				if (BlockStart)
				{
					CurPtr->GetRealSelection(GetString->SelStart,GetString->SelEnd);
				}
				else if (VBlockStart && DestLine>=VBlockY && DestLine<VBlockY+VBlockSizeY)
				{
					GetString->SelStart=CurPtr->TabPosToReal(VBlockX);
					GetString->SelEnd=GetString->SelStart+
					                  CurPtr->TabPosToReal(VBlockX+VBlockSizeX)-
					                  CurPtr->TabPosToReal(VBlockX);
				}

				_ECTLLOG(SysLog(L"EditorGetString{"));
				_ECTLLOG(SysLog(L"  StringNumber    =%d",GetString->StringNumber));
				_ECTLLOG(SysLog(L"  StringText      ='%s'",GetString->StringText));
				_ECTLLOG(SysLog(L"  StringEOL       ='%s'",GetString->StringEOL?_SysLog_LinearDump((LPBYTE)GetString->StringEOL,StrLength(GetString->StringEOL)):L"(null)"));
				_ECTLLOG(SysLog(L"  StringLength    =%d",GetString->StringLength));
				_ECTLLOG(SysLog(L"  SelStart        =%d",GetString->SelStart));
				_ECTLLOG(SysLog(L"  SelEnd          =%d",GetString->SelEnd));
				_ECTLLOG(SysLog(L"}"));
				return TRUE;
			}

			break;
		}
		case ECTL_INSERTSTRING:
		{
			if (Flags.Check(FEDITOR_LOCKMODE))
			{
				_ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
				return FALSE;
			}
			else
			{
				int Indent=Param && *(int *)Param!=FALSE;

				if (!Indent)
					Pasting++;

				Flags.Set(FEDITOR_NEWUNDO);
				InsertString();
				Show();

				if (!Indent)
					Pasting--;
			}

			return TRUE;
		}
		case ECTL_INSERTTEXT:
		{
			if (!Param)
				return FALSE;

			_ECTLLOG(SysLog(L"(const wchar_t *)Param='%s'",(const wchar_t *)Param));

			if (Flags.Check(FEDITOR_LOCKMODE))
			{
				_ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
				return FALSE;
			}
			else
			{
				const wchar_t *Str=(const wchar_t *)Param;
				Pasting++;
				Lock();

				while (*Str)
					ProcessKey(*(Str++));

				Unlock();
				Pasting--;
			}

			return TRUE;
		}
		case ECTL_SETSTRING:
		{
			EditorSetString *SetString=(EditorSetString *)Param;

			if (!SetString)
				break;

			_ECTLLOG(SysLog(L"EditorSetString{"));
			_ECTLLOG(SysLog(L"  StringNumber    =%d",SetString->StringNumber));
			_ECTLLOG(SysLog(L"  StringText      ='%s'",SetString->StringText));
			_ECTLLOG(SysLog(L"  StringEOL       ='%s'",SetString->StringEOL?_SysLog_LinearDump((LPBYTE)SetString->StringEOL,StrLength(SetString->StringEOL)):L"(null)"));
			_ECTLLOG(SysLog(L"  StringLength    =%d",SetString->StringLength));
			_ECTLLOG(SysLog(L"}"));

			if (Flags.Check(FEDITOR_LOCKMODE))
			{
				_ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
				break;
			}
			else
			{
				/* $ 06.08.2002 IS
				   Проверяем корректность StringLength и вернем FALSE, если оно меньше
				   нуля.
				*/
				int Length=SetString->StringLength;

				if (Length < 0)
				{
					_ECTLLOG(SysLog(L"SetString->StringLength < 0"));
					return FALSE;
				}

				Edit *CurPtr=GetStringByNumber(SetString->StringNumber);

				if (!CurPtr)
				{
					_ECTLLOG(SysLog(L"GetStringByNumber(%d) return nullptr",SetString->StringNumber));
					return FALSE;
				}

				const wchar_t *EOL=SetString->StringEOL ? SetString->StringEOL:GlobalEOL;

				int LengthEOL=StrLength(EOL);

				wchar_t *NewStr=(wchar_t*)xf_malloc((Length+LengthEOL+1)*sizeof(wchar_t));

				if (!NewStr)
				{
					_ECTLLOG(SysLog(L"xf_malloc(%d) return nullptr",Length+LengthEOL+1));
					return FALSE;
				}

				int DestLine=SetString->StringNumber;

				if (DestLine==-1)
					DestLine=NumLine;

				wmemcpy(NewStr,SetString->StringText,Length);
				wmemcpy(NewStr+Length,EOL,LengthEOL);
				AddUndoData(UNDO_EDIT,CurPtr->GetStringAddr(),CurPtr->GetEOL(),DestLine,CurPtr->GetCurPos(),CurPtr->GetLength());
				int CurPos=CurPtr->GetCurPos();
				CurPtr->SetBinaryString(NewStr,Length+LengthEOL);
				CurPtr->SetCurPos(CurPos);
				TextChanged(1);    // 10.08.2000 skv - Modified->TextChanged
				xf_free(NewStr);
			}

			return TRUE;
		}
		case ECTL_DELETESTRING:
		{
			if (Flags.Check(FEDITOR_LOCKMODE))
			{
				_ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
				return FALSE;
			}

			DeleteString(CurLine,FALSE,NumLine);
			return TRUE;
		}
		case ECTL_DELETECHAR:
		{
			if (Flags.Check(FEDITOR_LOCKMODE))
			{
				_ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
				return FALSE;
			}

			Pasting++;
			ProcessKey(KEY_DEL);
			Pasting--;
			return TRUE;
		}
		case ECTL_GETINFO:
		{
			EditorInfo *Info=(EditorInfo *)Param;

			if (Info)
			{
				Info->EditorID=Editor::EditorID;
				Info->WindowSizeX=ObjWidth;
				Info->WindowSizeY=Y2-Y1+1;
				Info->TotalLines=NumLastLine;
				Info->CurLine=NumLine;
				Info->CurPos=CurLine->GetCurPos();
				Info->CurTabPos=CurLine->GetTabCurPos();
				Info->TopScreenLine=NumLine-CalcDistance(TopScreen,CurLine,-1);
				Info->LeftPos=CurLine->GetLeftPos();
				Info->Overtype=Flags.Check(FEDITOR_OVERTYPE);
				Info->BlockType=VBlockStart?BTYPE_COLUMN:BlockStart?BTYPE_STREAM:BTYPE_NONE;
				Info->BlockStartLine=Info->BlockType==BTYPE_NONE ? 0:BlockStartLine;
				Info->Options=0;

				if (EdOpt.ExpandTabs == EXPAND_ALLTABS)
					Info->Options|=EOPT_EXPANDALLTABS;

				if (EdOpt.ExpandTabs == EXPAND_NEWTABS)
					Info->Options|=EOPT_EXPANDONLYNEWTABS;

				if (EdOpt.PersistentBlocks)
					Info->Options|=EOPT_PERSISTENTBLOCKS;

				if (EdOpt.DelRemovesBlocks)
					Info->Options|=EOPT_DELREMOVESBLOCKS;

				if (EdOpt.AutoIndent)
					Info->Options|=EOPT_AUTOINDENT;

				if (EdOpt.SavePos)
					Info->Options|=EOPT_SAVEFILEPOSITION;

				if (EdOpt.AutoDetectCodePage)
					Info->Options|=EOPT_AUTODETECTCODEPAGE;

				if (EdOpt.CursorBeyondEOL)
					Info->Options|=EOPT_CURSORBEYONDEOL;

				if (EdOpt.ShowWhiteSpace)
					Info->Options|=EOPT_SHOWWHITESPACE;

				Info->TabSize=EdOpt.TabSize;
				Info->BookMarkCount=BOOKMARK_COUNT;
				Info->CurState=Flags.Check(FEDITOR_LOCKMODE)?ECSTATE_LOCKED:0;
				Info->CurState|=!Flags.Check(FEDITOR_MODIFIED)?ECSTATE_SAVED:0;
				Info->CurState|=Flags.Check(FEDITOR_MODIFIED|FEDITOR_WASCHANGED)?ECSTATE_MODIFIED:0;
				Info->CodePage=m_codepage;
				return TRUE;
			}

			_ECTLLOG(SysLog(L"Error: !Param"));
			return FALSE;
		}
		case ECTL_SETPOSITION:
		{
			// "Вначале было слово..."
			if (Param)
			{
				// ...а вот теперь поработаем с тем, что передалаи
				EditorSetPosition *Pos=(EditorSetPosition *)Param;
				_ECTLLOG(SysLog(L"EditorSetPosition{"));
				_ECTLLOG(SysLog(L"  CurLine       = %d",Pos->CurLine));
				_ECTLLOG(SysLog(L"  CurPos        = %d",Pos->CurPos));
				_ECTLLOG(SysLog(L"  CurTabPos     = %d",Pos->CurTabPos));
				_ECTLLOG(SysLog(L"  TopScreenLine = %d",Pos->TopScreenLine));
				_ECTLLOG(SysLog(L"  LeftPos       = %d",Pos->LeftPos));
				_ECTLLOG(SysLog(L"  Overtype      = %d",Pos->Overtype));
				_ECTLLOG(SysLog(L"}"));
				Lock();
				int CurPos=CurLine->GetCurPos();

				// выставим флаг об изменении поз (если надо)
				if ((Pos->CurLine >= 0 || Pos->CurPos >= 0)&&
				        (Pos->CurLine!=NumLine || Pos->CurPos!=CurPos))
					Flags.Set(FEDITOR_CURPOSCHANGEDBYPLUGIN);

				if (Pos->CurLine >= 0) // поменяем строку
				{
					if (Pos->CurLine==NumLine-1)
						Up();
					else if (Pos->CurLine==NumLine+1)
						Down();
					else
						GoToLine(Pos->CurLine);
				}

				if (Pos->TopScreenLine >= 0 && Pos->TopScreenLine<=NumLine)
				{
					TopScreen=CurLine;

					for (int I=NumLine; I>0 && NumLine-I<Y2-Y1 && I!=Pos->TopScreenLine; I--)
						TopScreen=TopScreen->m_prev;
				}

				if (Pos->CurPos >= 0)
					CurLine->SetCurPos(Pos->CurPos);

				if (Pos->CurTabPos >= 0)
					CurLine->SetTabCurPos(Pos->CurTabPos);

				if (Pos->LeftPos >= 0)
					CurLine->SetLeftPos(Pos->LeftPos);

				/* $ 30.08.2001 IS
				   Изменение режима нужно выставлять сразу, в противном случае приходят
				   глюки, т.к. плагинописатель думает, что режим изменен, и ведет себя
				   соответствующе, в результате чего получает неопределенное поведение.
				*/
				if (Pos->Overtype >= 0)
				{
					Flags.Change(FEDITOR_OVERTYPE,Pos->Overtype);
					CurLine->SetOvertypeMode(Flags.Check(FEDITOR_OVERTYPE));
				}

				Unlock();
				return TRUE;
			}

			_ECTLLOG(SysLog(L"Error: !Param"));
			break;
		}
		case ECTL_SELECT:
		{
			if (Param)
			{
				EditorSelect *Sel=(EditorSelect *)Param;
				_ECTLLOG(SysLog(L"EditorSelect{"));
				_ECTLLOG(SysLog(L"  BlockType     =%s (%d)",(Sel->BlockType==BTYPE_NONE?L"BTYPE_NONE":(Sel->BlockType==BTYPE_STREAM?L"":(Sel->BlockType==BTYPE_COLUMN?L"BTYPE_COLUMN":L"BTYPE_?????"))),Sel->BlockType));
				_ECTLLOG(SysLog(L"  BlockStartLine=%d",Sel->BlockStartLine));
				_ECTLLOG(SysLog(L"  BlockStartPos =%d",Sel->BlockStartPos));
				_ECTLLOG(SysLog(L"  BlockWidth    =%d",Sel->BlockWidth));
				_ECTLLOG(SysLog(L"  BlockHeight   =%d",Sel->BlockHeight));
				_ECTLLOG(SysLog(L"}"));

				if (Sel->BlockType==BTYPE_NONE || Sel->BlockStartPos==-1)
				{
					UnmarkBlock();
					return TRUE;
				}

				if (Sel->BlockHeight < 1)
				{
					_ECTLLOG(SysLog(L"Error: EditorSelect::BlockHeight < 1"));
					return FALSE;
				}

				Edit *CurPtr=GetStringByNumber(Sel->BlockStartLine);

				if (!CurPtr)
				{
					_ECTLLOG(SysLog(L"Error: start line BlockStartLine=%d not found, GetStringByNumber(%d) return nullptr",Sel->BlockStartLine,Sel->BlockStartLine));
					return FALSE;
				}

				UnmarkBlock();

				if (Sel->BlockType==BTYPE_STREAM)
				{
					Flags.Set(FEDITOR_MARKINGBLOCK);
					BlockStart=CurPtr;

					if ((BlockStartLine=Sel->BlockStartLine) == -1)
						BlockStartLine=NumLine;

					for (int i=0; i < Sel->BlockHeight; i++)
					{
						int SelStart= i? 0:Sel->BlockStartPos;
						int SelEnd  = (i < Sel->BlockHeight-1) ? -1 : Sel->BlockStartPos+Sel->BlockWidth;
						CurPtr->Select(SelStart,SelEnd);
						CurPtr=CurPtr->m_next;

						if (!CurPtr)
							return TRUE; // ранее было FALSE
					}
				}
				else if (Sel->BlockType==BTYPE_COLUMN)
				{
					Flags.Set(FEDITOR_MARKINGVBLOCK);
					VBlockStart=CurPtr;

					if ((BlockStartLine=Sel->BlockStartLine) == -1)
						BlockStartLine=NumLine;

					VBlockX=Sel->BlockStartPos;

					if ((VBlockY=Sel->BlockStartLine) == -1)
						VBlockY=NumLine;

					VBlockSizeX=Sel->BlockWidth;
					VBlockSizeY=Sel->BlockHeight;

					if (VBlockSizeX < 0)
					{
						VBlockSizeX=-VBlockSizeX;
						VBlockX-=VBlockSizeX;

						if (VBlockX < 0)
							VBlockX=0;
					}
				}

				return TRUE;
			}

			_ECTLLOG(SysLog(L"Error: !Param"));
			break;
		}
		case ECTL_REDRAW:
		{
			Show();
			ScrBuf.Flush();
			return TRUE;
		}
		case ECTL_TABTOREAL:
		{
			if (Param)
			{
				EditorConvertPos *ecp=(EditorConvertPos *)Param;
				Edit *CurPtr=GetStringByNumber(ecp->StringNumber);

				if (!CurPtr)
				{
					_ECTLLOG(SysLog(L"GetStringByNumber(%d) return nullptr",ecp->StringNumber));
					return FALSE;
				}

				ecp->DestPos=CurPtr->TabPosToReal(ecp->SrcPos);
				_ECTLLOG(SysLog(L"EditorConvertPos{"));
				_ECTLLOG(SysLog(L"  StringNumber =%d",ecp->StringNumber));
				_ECTLLOG(SysLog(L"  SrcPos       =%d",ecp->SrcPos));
				_ECTLLOG(SysLog(L"  DestPos      =%d",ecp->DestPos));
				_ECTLLOG(SysLog(L"}"));
				return TRUE;
			}

			break;
		}
		case ECTL_REALTOTAB:
		{
			if (Param)
			{
				EditorConvertPos *ecp=(EditorConvertPos *)Param;
				Edit *CurPtr=GetStringByNumber(ecp->StringNumber);

				if (!CurPtr)
				{
					_ECTLLOG(SysLog(L"GetStringByNumber(%d) return nullptr",ecp->StringNumber));
					return FALSE;
				}

				ecp->DestPos=CurPtr->RealPosToTab(ecp->SrcPos);
				_ECTLLOG(SysLog(L"EditorConvertPos{"));
				_ECTLLOG(SysLog(L"  StringNumber =%d",ecp->StringNumber));
				_ECTLLOG(SysLog(L"  SrcPos       =%d",ecp->SrcPos));
				_ECTLLOG(SysLog(L"  DestPos      =%d",ecp->DestPos));
				_ECTLLOG(SysLog(L"}"));
				return TRUE;
			}

			break;
		}
		case ECTL_EXPANDTABS:
		{
			if (Flags.Check(FEDITOR_LOCKMODE))
			{
				_ECTLLOG(SysLog(L"FEDITOR_LOCKMODE!"));
				return FALSE;
			}
			else
			{
				int StringNumber=*(int *)Param;
				Edit *CurPtr=GetStringByNumber(StringNumber);

				if (!CurPtr)
				{
					_ECTLLOG(SysLog(L"GetStringByNumber(%d) return nullptr",StringNumber));
					return FALSE;
				}

				AddUndoData(UNDO_EDIT,CurPtr->GetStringAddr(),CurPtr->GetEOL(),StringNumber,CurPtr->GetCurPos(),CurPtr->GetLength());
				CurPtr->ReplaceTabs();
			}

			return TRUE;
		}
		// TODO: Если DI_MEMOEDIT не будет юзать раскаску, то должно выполняется в FileEditor::EditorControl(), в диалоге - нафиг ненать
		case ECTL_ADDCOLOR:
		{
			if (Param)
			{
				EditorColor *col=(EditorColor *)Param;
				_ECTLLOG(SysLog(L"EditorColor{"));
				_ECTLLOG(SysLog(L"  StringNumber=%d",col->StringNumber));
				_ECTLLOG(SysLog(L"  ColorItem   =%d (0x%08X)",col->ColorItem,col->ColorItem));
				_ECTLLOG(SysLog(L"  StartPos    =%d",col->StartPos));
				_ECTLLOG(SysLog(L"  EndPos      =%d",col->EndPos));
				_ECTLLOG(SysLog(L"  Color       =%d (0x%08X)",col->Color,col->Color));
				_ECTLLOG(SysLog(L"}"));
				ColorItem newcol;
				newcol.StartPos=col->StartPos+(col->StartPos!=-1?X1:0);
				newcol.EndPos=col->EndPos+X1;
				newcol.Color=col->Color;
				Edit *CurPtr=GetStringByNumber(col->StringNumber);

				if (!CurPtr)
				{
					_ECTLLOG(SysLog(L"GetStringByNumber(%d) return nullptr",col->StringNumber));
					return FALSE;
				}

				if (!col->Color)
					return(CurPtr->DeleteColor(newcol.StartPos));

				CurPtr->AddColor(&newcol);
				return TRUE;
			}

			break;
		}
		// TODO: Если DI_MEMOEDIT не будет юзать раскаску, то должно выполняется в FileEditor::EditorControl(), в диалоге - нафиг ненать
		case ECTL_GETCOLOR:
		{
			if (Param)
			{
				EditorColor *col=(EditorColor *)Param;
				Edit *CurPtr=GetStringByNumber(col->StringNumber);

				if (!CurPtr)
				{
					_ECTLLOG(SysLog(L"GetStringByNumber(%d) return nullptr",col->StringNumber));
					return FALSE;
				}

				ColorItem curcol;

				if (!CurPtr->GetColor(&curcol,col->ColorItem))
				{
					_ECTLLOG(SysLog(L"GetColor() return nullptr"));
					return FALSE;
				}

				col->StartPos=curcol.StartPos-X1;
				col->EndPos=curcol.EndPos-X1;
				col->Color=curcol.Color;
				_ECTLLOG(SysLog(L"EditorColor{"));
				_ECTLLOG(SysLog(L"  StringNumber=%d",col->StringNumber));
				_ECTLLOG(SysLog(L"  ColorItem   =%d (0x%08X)",col->ColorItem,col->ColorItem));
				_ECTLLOG(SysLog(L"  StartPos    =%d",col->StartPos));
				_ECTLLOG(SysLog(L"  EndPos      =%d",col->EndPos));
				_ECTLLOG(SysLog(L"  Color       =%d (0x%08X)",col->Color,col->Color));
				_ECTLLOG(SysLog(L"}"));
				return TRUE;
			}

			break;
		}
		// должно выполняется в FileEditor::EditorControl()
		case ECTL_PROCESSKEY:
		{
			_ECTLLOG(SysLog(L"Key = %s",_FARKEY_ToName((DWORD)Param)));
			ProcessKey((int)(INT_PTR)Param);
			return TRUE;
		}
		/* $ 16.02.2001 IS
		     Изменение некоторых внутренних настроек редактора. Param указывает на
		     структуру EditorSetParameter
		*/
		case ECTL_SETPARAM:
		{
			if (Param)
			{
				EditorSetParameter *espar=(EditorSetParameter *)Param;
				int rc=TRUE;
				_ECTLLOG(SysLog(L"EditorSetParameter{"));
				_ECTLLOG(SysLog(L"  Type        =%s",_ESPT_ToName(espar->Type)));

				switch (espar->Type)
				{
					case ESPT_GETWORDDIV:
						_ECTLLOG(SysLog(L"  wszParam    =(%p)",espar->Param.wszParam));

						if (espar->Param.wszParam && espar->Size)
							xwcsncpy(espar->Param.wszParam,EdOpt.strWordDiv,espar->Size);

						rc=(int)EdOpt.strWordDiv.GetLength()+1;
						break;
					case ESPT_SETWORDDIV:
						_ECTLLOG(SysLog(L"  wszParam    =[%s]",espar->Param.wszParam));
						SetWordDiv((!espar->Param.wszParam || !*espar->Param.wszParam)?Opt.strWordDiv.CPtr():espar->Param.wszParam);
						break;
					case ESPT_TABSIZE:
						_ECTLLOG(SysLog(L"  iParam      =%d",espar->Param.iParam));
						SetTabSize(espar->Param.iParam);
						break;
					case ESPT_EXPANDTABS:
						_ECTLLOG(SysLog(L"  iParam      =%s",espar->Param.iParam?L"On":L"Off"));
						SetConvertTabs(espar->Param.iParam);
						break;
					case ESPT_AUTOINDENT:
						_ECTLLOG(SysLog(L"  iParam      =%s",espar->Param.iParam?L"On":L"Off"));
						SetAutoIndent(espar->Param.iParam);
						break;
					case ESPT_CURSORBEYONDEOL:
						_ECTLLOG(SysLog(L"  iParam      =%s",espar->Param.iParam?L"On":L"Off"));
						SetCursorBeyondEOL(espar->Param.iParam);
						break;
					case ESPT_CHARCODEBASE:
						_ECTLLOG(SysLog(L"  iParam      =%s",(!espar->Param.iParam?L"0 (Oct)":(espar->Param.iParam==1?L"1 (Dec)":(espar->Param.iParam==2?L"2 (Hex)":L"?????")))));
						SetCharCodeBase(espar->Param.iParam);
						break;
						/* $ 07.08.2001 IS сменить кодировку из плагина */
					case ESPT_CODEPAGE:
					{
						//BUGBUG
						if ((UINT)espar->Param.iParam==CP_AUTODETECT)
						{
							rc=FALSE;
						}
						else
						{
							if (HostFileEditor)
							{
								HostFileEditor->SetCodePage(espar->Param.iParam);
								HostFileEditor->CodepageChangedByUser();
							}
							else
							{
								SetCodePage(espar->Param.iParam);
							}

							Show();
						}
					}
					break;
					/* $ 29.10.2001 IS изменение настройки "Сохранять позицию файла" */
					case ESPT_SAVEFILEPOSITION:
						_ECTLLOG(SysLog(L"  iParam      =%s",espar->Param.iParam?L"On":L"Off"));
						SetSavePosMode(espar->Param.iParam, -1);
						break;
						/* $ 23.03.2002 IS запретить/отменить изменение файла */
					case ESPT_LOCKMODE:
						_ECTLLOG(SysLog(L"  iParam      =%s",espar->Param.iParam?L"On":L"Off"));
						Flags.Change(FEDITOR_LOCKMODE, espar->Param.iParam);
						break;
					case ESPT_SHOWWHITESPACE:
						SetShowWhiteSpace(espar->Param.iParam);
						break;
					default:
						_ECTLLOG(SysLog(L"}"));
						return FALSE;
				}

				_ECTLLOG(SysLog(L"}"));
				return rc;
			}

			return  FALSE;
		}
		// Убрать флаг редактора "осуществляется выделение блока"
		case ECTL_TURNOFFMARKINGBLOCK:
		{
			Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
			return TRUE;
		}
		case ECTL_DELETEBLOCK:
		{
			if (Flags.Check(FEDITOR_LOCKMODE) || !(VBlockStart || BlockStart))
			{

				_ECTLLOG(if (Flags.Check(FEDITOR_LOCKMODE))SysLog(L"FEDITOR_LOCKMODE!"));

				_ECTLLOG(if (!(VBlockStart || BlockStart))SysLog(L"Not selected block!"));

				return FALSE;
			}

			Flags.Clear(FEDITOR_MARKINGVBLOCK|FEDITOR_MARKINGBLOCK);
			DeleteBlock();
			Show();
			return TRUE;
		}
		case ECTL_UNDOREDO:
		{
			if (Param)
			{
				EditorUndoRedo *eur=(EditorUndoRedo *)Param;

				switch (eur->Command)
				{
					case EUR_BEGIN:
						AddUndoData(UNDO_BEGIN);
						return TRUE;
					case EUR_END:
						AddUndoData(UNDO_END);
						return TRUE;
					case EUR_UNDO:
					case EUR_REDO:
						Lock();
						Undo(eur->Command==EUR_REDO);
						Unlock();
						return TRUE;
				}
			}

			return FALSE;
		}
	}

	return FALSE;
}

int Editor::SetBookmark(DWORD Pos)
{
	if (Pos < BOOKMARK_COUNT)
	{
		SavePos.Line[Pos]=NumLine;
		SavePos.Cursor[Pos]=CurLine->GetCurPos();
		SavePos.LeftPos[Pos]=CurLine->GetLeftPos();
		SavePos.ScreenLine[Pos]=CalcDistance(TopScreen,CurLine,-1);
		return TRUE;
	}

	return FALSE;
}

int Editor::GotoBookmark(DWORD Pos)
{
	if (Pos < BOOKMARK_COUNT)
	{
		if (SavePos.Line[Pos]!=POS_NONE)
		{
			GoToLine(static_cast<int>(SavePos.Line[Pos]));
			CurLine->SetCurPos(static_cast<int>(SavePos.Cursor[Pos]));
			CurLine->SetLeftPos(static_cast<int>(SavePos.LeftPos[Pos]));
			TopScreen=CurLine;

			for (DWORD I=0; I<SavePos.ScreenLine[Pos] && TopScreen->m_prev; I++)
				TopScreen=TopScreen->m_prev;

			if (!EdOpt.PersistentBlocks)
				UnmarkBlock();

			Show();
		}

		return TRUE;
	}

	return FALSE;
}

int Editor::RestoreStackBookmark()
{
	if (StackPos && ((int)StackPos->Line!=NumLine || (int)StackPos->Cursor!=CurLine->GetCurPos()))
	{
		GoToLine(StackPos->Line);
		CurLine->SetCurPos(StackPos->Cursor);
		CurLine->SetLeftPos(StackPos->LeftPos);
		TopScreen=CurLine;

		for (DWORD I=0; I<StackPos->ScreenLine && TopScreen->m_prev; I++)
			TopScreen=TopScreen->m_prev;

		if (!EdOpt.PersistentBlocks)
			UnmarkBlock();

		Show();
		return TRUE;
	}

	return FALSE;
}

int Editor::AddStackBookmark()
{
	InternalEditorStackBookMark *sb_old=StackPos,*sb_new;

	if (StackPos && StackPos->next)
	{
		StackPos=StackPos->next;
		StackPos->prev=0;
		ClearStackBookmarks();
		StackPos=sb_old;
		StackPos->next = 0;
	}

	if (StackPos && (int)StackPos->Line==NumLine && (int)StackPos->Cursor==CurLine->GetCurPos())
		return TRUE;

	sb_new = (InternalEditorStackBookMark*) xf_malloc(sizeof(InternalEditorStackBookMark));

	if (sb_new)
	{
		if (StackPos)
			StackPos->next=sb_new;

		StackPos=sb_new;
		StackPos->prev=sb_old;
		StackPos->next=0;
		StackPos->Line=NumLine;
		StackPos->Cursor=CurLine->GetCurPos();
		StackPos->LeftPos=CurLine->GetLeftPos();
		StackPos->ScreenLine=CalcDistance(TopScreen,CurLine,-1);
		NewStackPos = TRUE; // When go prev bookmark, we must save current
		return TRUE;
	}

	return FALSE;
}

int Editor::PrevStackBookmark()
{
	if (StackPos)
	{
		if (NewStackPos) // Save last position in new bookmark
		{
			AddStackBookmark();
			NewStackPos = FALSE;
		}

		if (StackPos->prev) // If not first bookmark - go
		{
			StackPos=StackPos->prev;
		}

		return RestoreStackBookmark();
	}

	return FALSE;
}

int Editor::NextStackBookmark()
{
	if (StackPos)
	{
		if (StackPos->next) // If not last bookmark - go
		{
			StackPos=StackPos->next;
		}

		return RestoreStackBookmark();
	}

	return FALSE;
}

int Editor::ClearStackBookmarks()
{
	if (StackPos)
	{
		InternalEditorStackBookMark *sb_prev = StackPos->prev, *sb_next;

		while (StackPos)
		{
			sb_next = StackPos->next;
			xf_free(StackPos);
			StackPos = sb_next;
		}

		StackPos = sb_prev;

		while (StackPos)
		{
			sb_prev = StackPos->prev;
			xf_free(StackPos);
			StackPos = sb_prev;
		}

		NewStackPos = FALSE;
	}

	return TRUE;
}

InternalEditorStackBookMark* Editor::PointerToStackBookmark(int iIdx)
{
	InternalEditorStackBookMark *sb_temp=StackPos;

	if (iIdx!=-1 && sb_temp)
	{
		while (sb_temp->prev)
			sb_temp=sb_temp->prev;

		for (int i=0; i!=iIdx && sb_temp; i++)
			sb_temp=sb_temp->next;
	}

	return sb_temp;
}

int Editor::DeleteStackBookmark(InternalEditorStackBookMark *sb_delete)
{
	if (sb_delete)
	{
		if (sb_delete->next)
			sb_delete->next->prev=sb_delete->prev;

		if (sb_delete->prev)
			sb_delete->prev->next=sb_delete->next;

		if (StackPos==sb_delete)
			StackPos=(sb_delete->next)?sb_delete->next:sb_delete->prev;

		xf_free(sb_delete);
		return TRUE;
	}

	return FALSE;
}

int Editor::GetStackBookmark(int iIdx,EditorBookMarks *Param)
{
	InternalEditorStackBookMark *sb_temp = PointerToStackBookmark(iIdx);

	if (sb_temp && Param)
	{
		if (Param->Line)       Param->Line[0]       =sb_temp->Line;

		if (Param->Cursor)     Param->Cursor[0]     =sb_temp->Cursor;

		if (Param->LeftPos)    Param->LeftPos[0]    =sb_temp->LeftPos;

		if (Param->ScreenLine) Param->ScreenLine[0] =sb_temp->ScreenLine;

		return TRUE;
	}

	return FALSE;
}

int Editor::GetStackBookmarks(EditorBookMarks *Param)
{
	InternalEditorStackBookMark *sb_temp=StackPos, *sb_start;
	int iCount=0;

	if (sb_temp)
	{
		for (; sb_temp->prev; iCount++)
			sb_temp=sb_temp->prev;

		sb_start=sb_temp;

		for (sb_temp=StackPos; sb_temp; iCount++)
			sb_temp=sb_temp->next;

		if (Param)
		{
			if (Param->Line || Param->Cursor || Param->LeftPos || Param->ScreenLine)
			{
				sb_temp=sb_start;

				for (int i=0; i<iCount; i++)
				{
					if (Param->Line)
						Param->Line[i]=sb_temp->Line;

					if (Param->Cursor)
						Param->Cursor[i]=sb_temp->Cursor;

					if (Param->LeftPos)
						Param->LeftPos[i]=sb_temp->LeftPos;

					if (Param->ScreenLine)
						Param->ScreenLine[i]=sb_temp->ScreenLine;

					sb_temp=sb_temp->next;
				}
			}
			else iCount=0;
		}
	}

	return iCount;
}

Edit * Editor::GetStringByNumber(int DestLine)
{
	if (DestLine==NumLine || DestLine<0)
		return(CurLine);

	if (DestLine>NumLastLine)
		return nullptr;

	if (DestLine>NumLine)
	{
		Edit *CurPtr=CurLine;

		for (int Line=NumLine; Line<DestLine; Line++)
		{
			CurPtr=CurPtr->m_next;

			if (!CurPtr)
				return nullptr;
		}

		return(CurPtr);
	}

	if (DestLine<NumLine && DestLine>NumLine/2)
	{
		Edit *CurPtr=CurLine;

		for (int Line=NumLine; Line>DestLine; Line--)
		{
			CurPtr=CurPtr->m_prev;

			if (!CurPtr)
				return nullptr;
		}

		return(CurPtr);
	}

	{
		Edit *CurPtr=TopList;

		for (int Line=0; Line<DestLine; Line++)
		{
			CurPtr=CurPtr->m_next;

			if (!CurPtr)
				return nullptr;
		}

		return(CurPtr);
	}
}

void Editor::SetReplaceMode(int Mode)
{
	::ReplaceMode=Mode;
}

int Editor::GetLineCurPos()
{
	return CurLine->GetTabCurPos();
}

void Editor::BeginVBlockMarking()
{
	UnmarkBlock();
	VBlockStart=CurLine;
	VBlockX=CurLine->GetTabCurPos();
	VBlockSizeX=0;
	VBlockY=NumLine;
	VBlockSizeY=1;
	Flags.Set(FEDITOR_MARKINGVBLOCK);
	BlockStartLine=NumLine;
	//_D(SysLog(L"BeginVBlockMarking, set vblock to  VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
}

void Editor::AdjustVBlock(int PrevX)
{
	int x=GetLineCurPos();
	int c2;

	//_D(SysLog(L"AdjustVBlock, x=%i,   vblock is VBlockY=%i:%i, VBlockX=%i:%i, PrevX=%i",x,VBlockY,VBlockSizeY,VBlockX,VBlockSizeX,PrevX));
	if (x==VBlockX+VBlockSizeX)   // ничего не случилось, никаких табуляций нет
		return;

	if (x>VBlockX)    // курсор убежал внутрь блока
	{
		VBlockSizeX=x-VBlockX;
		//_D(SysLog(L"x>VBlockX");
	}
	else if (x<VBlockX)   // курсор убежал за начало блока
	{
		c2=VBlockX;

		if (PrevX>VBlockX)      // сдвигались вправо, а пришли влево
		{
			VBlockX=x;
			VBlockSizeX=c2-x;   // меняем блок
		}
		else      // сдвигались влево и пришли еще больше влево
		{
			VBlockX=x;
			VBlockSizeX+=c2-x;  // расширяем блок
		}

		//_D(SysLog(L"x<VBlockX"));
	}
	else if (x==VBlockX && x!=PrevX)
	{
		VBlockSizeX=0;  // ширина в 0, потому прыгнули прям на табуляцию
		//_D(SysLog(L"x==VBlockX && x!=PrevX"));
	}

	// примечание
	//   случай x>VBLockX+VBlockSizeX не может быть
	//   потому что курсор прыгает назад на табуляцию, но не вперед
	//_D(SysLog(L"AdjustVBlock, changed vblock  VBlockY=%i:%i, VBlockX=%i:%i",VBlockY,VBlockSizeY,VBlockX,VBlockSizeX));
}


void Editor::Xlat()
{
	Edit *CurPtr;
	int Line;
	BOOL DoXlat=FALSE;
	AddUndoData(UNDO_BEGIN);

	if (VBlockStart)
	{
		CurPtr=VBlockStart;

		for (Line=0; CurPtr && Line<VBlockSizeY; Line++,CurPtr=CurPtr->m_next)
		{
			int TBlockX=CurPtr->TabPosToReal(VBlockX);
			int TBlockSizeX=CurPtr->TabPosToReal(VBlockX+VBlockSizeX)-
			                CurPtr->TabPosToReal(VBlockX);
			const wchar_t *CurStr,*EndSeq;
			int Length;
			CurPtr->GetBinaryString(&CurStr,&EndSeq,Length);
			int CopySize=Length-TBlockX;

			if (CopySize>TBlockSizeX)
				CopySize=TBlockSizeX;

			AddUndoData(UNDO_EDIT,CurPtr->GetStringAddr(),CurPtr->GetEOL(),BlockStartLine+Line,CurLine->GetCurPos(),CurPtr->GetLength());
			::Xlat(CurPtr->Str,TBlockX,TBlockX+CopySize,Opt.XLat.Flags);
		}

		DoXlat=TRUE;
	}
	else
	{
		Line=0;
		CurPtr=BlockStart;

		// $ 25.11.2000 IS
		//     Если нет выделения, то обработаем текущее слово. Слово определяется на
		//     основе специальной группы разделителей.
		if (CurPtr)
		{
			while (CurPtr)
			{
				int StartSel,EndSel;
				CurPtr->GetSelection(StartSel,EndSel);

				if (StartSel==-1)
					break;

				if (EndSel == -1)
					EndSel=CurPtr->GetLength();//StrLength(CurPtr->Str);

				AddUndoData(UNDO_EDIT,CurPtr->GetStringAddr(),CurPtr->GetEOL(),BlockStartLine+Line,CurLine->GetCurPos(),CurPtr->GetLength());
				::Xlat(CurPtr->Str,StartSel,EndSel,Opt.XLat.Flags);
				Line++;
				CurPtr=CurPtr->m_next;
			}

			DoXlat=TRUE;
		}
		else
		{
			wchar_t *Str=CurLine->Str;
			int start=CurLine->GetCurPos(), end, StrSize=CurLine->GetLength();//StrLength(Str);
			// $ 10.12.2000 IS
			//   Обрабатываем только то слово, на котором стоит курсор, или то слово,
			//   что находится левее позиции курсора на 1 символ
			DoXlat=TRUE;

			if (IsWordDiv(Opt.XLat.strWordDivForXlat,Str[start]))
			{
				if (start) start--;

				DoXlat=(!IsWordDiv(Opt.XLat.strWordDivForXlat,Str[start]));
			}

			if (DoXlat)
			{
				while (start>=0 && !IsWordDiv(Opt.XLat.strWordDivForXlat,Str[start]))
					start--;

				start++;
				end=start+1;

				while (end<StrSize && !IsWordDiv(Opt.XLat.strWordDivForXlat,Str[end]))
					end++;

				AddUndoData(UNDO_EDIT,CurLine->GetStringAddr(),CurLine->GetEOL(),NumLine,start,CurLine->GetLength());
				::Xlat(Str,start,end,Opt.XLat.Flags);
			}
		}
	}

	AddUndoData(UNDO_END);

	if (DoXlat)
		TextChanged(1);
}
/* SVS $ */

/* $ 15.02.2001 IS
     Манипуляции с табуляцией на уровне всего загруженного файла.
     Может быть длительной во времени операцией, но тут уж, imho,
     ничего не поделать.
*/
//Обновим размер табуляции
void Editor::SetTabSize(int NewSize)
{
	if (NewSize<1 || NewSize>512)
		NewSize=8;

	if (NewSize!=EdOpt.TabSize) /* Меняем размер табуляции только в том случае, если он
                          на самом деле изменился */
	{
		EdOpt.TabSize=NewSize;
		Edit *CurPtr=TopList;

		while (CurPtr)
		{
			CurPtr->SetTabSize(NewSize);
			CurPtr=CurPtr->m_next;
		}
	}
}

// обновим режим пробелы вместо табуляции
// операция необратима, кстати, т.е. пробелы на табуляцию обратно не изменятся
void Editor::SetConvertTabs(int NewMode)
{
	if (NewMode!=EdOpt.ExpandTabs) /* Меняем режим только в том случае, если он
                              на самом деле изменился */
	{
		EdOpt.ExpandTabs=NewMode;
		Edit *CurPtr=TopList;

		while (CurPtr)
		{
			CurPtr->SetConvertTabs(NewMode);

			if (NewMode == EXPAND_ALLTABS)
				CurPtr->ReplaceTabs();

			CurPtr=CurPtr->m_next;
		}
	}
}

void Editor::SetDelRemovesBlocks(int NewMode)
{
	if (NewMode!=EdOpt.DelRemovesBlocks)
	{
		EdOpt.DelRemovesBlocks=NewMode;
		Edit *CurPtr=TopList;

		while (CurPtr)
		{
			CurPtr->SetDelRemovesBlocks(NewMode);
			CurPtr=CurPtr->m_next;
		}
	}
}

void Editor::SetShowWhiteSpace(int NewMode)
{
	if (NewMode!=EdOpt.ShowWhiteSpace)
	{
		EdOpt.ShowWhiteSpace=NewMode;

		for (Edit *CurPtr=TopList; CurPtr; CurPtr=CurPtr->m_next)
		{
			CurPtr->SetShowWhiteSpace(NewMode);
		}
	}
}

void Editor::SetPersistentBlocks(int NewMode)
{
	if (NewMode!=EdOpt.PersistentBlocks)
	{
		EdOpt.PersistentBlocks=NewMode;
		Edit *CurPtr=TopList;

		while (CurPtr)
		{
			CurPtr->SetPersistentBlocks(NewMode);
			CurPtr=CurPtr->m_next;
		}
	}
}

//     "Курсор за пределами строки"
void Editor::SetCursorBeyondEOL(int NewMode)
{
	if (NewMode!=EdOpt.CursorBeyondEOL)
	{
		EdOpt.CursorBeyondEOL=NewMode;
		Edit *CurPtr=TopList;

		while (CurPtr)
		{
			CurPtr->SetEditBeyondEnd(NewMode);
			CurPtr=CurPtr->m_next;
		}
	}

	/* $ 16.10.2001 SKV
	  Если переключились туда сюда этот режим,
	  то из-за этой штуки возникают нехилые глюки
	  при выделении вертикальных блоков.
	*/
	if (EdOpt.CursorBeyondEOL)
	{
		MaxRightPos=0;
	}
}

void Editor::GetSavePosMode(int &SavePos, int &SaveShortPos)
{
	SavePos=EdOpt.SavePos;
	SaveShortPos=EdOpt.SaveShortPos;
}

// передавайте в качестве значения параметра "-1" для параметра,
// который не нужно менять
void Editor::SetSavePosMode(int SavePos, int SaveShortPos)
{
	if (SavePos!=-1)
		EdOpt.SavePos=SavePos;

	if (SaveShortPos!=-1)
		EdOpt.SaveShortPos=SaveShortPos;
}

void Editor::EditorShowMsg(const wchar_t *Title,const wchar_t *Msg, const wchar_t* Name,int Percent)
{
	string strProgress;

	if (Percent!=-1)
	{
		FormatString strPercent;
		strPercent<<Percent;

		size_t PercentLength=Max(strPercent.strValue().GetLength(),(size_t)3);
		size_t Length=Max(Min(static_cast<int>(MAX_WIDTH_MESSAGE-2),StrLength(Name)),40)-PercentLength-2;
		wchar_t *Progress=strProgress.GetBuffer(Length);

		if (Progress)
		{
			size_t CurPos=(Percent>100?100:Percent)*Length/100;
			wmemset(Progress,BoxSymbols[BS_X_DB],CurPos);
			wmemset(Progress+(CurPos),BoxSymbols[BS_X_B0],Length-CurPos);
			strProgress.ReleaseBuffer(Length);
			FormatString strTmp;
			strTmp<<L" "<<fmt::Width(PercentLength)<<strPercent<<L"%";
			strProgress+=strTmp;
		}

		TBC.SetProgressValue(Percent,100);
	}

	Message(0,0,Title,Msg,Name,strProgress.IsEmpty()?nullptr:strProgress.CPtr());
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	preRedrawItem.Param.Param1=(void *)Title;
	preRedrawItem.Param.Param2=(void *)Msg;
	preRedrawItem.Param.Param3=(void *)Name;
	preRedrawItem.Param.Param4=(void *)(INT_PTR)(Percent);
	PreRedraw.SetParam(preRedrawItem.Param);
}

void Editor::PR_EditorShowMsg()
{
	PreRedrawItem preRedrawItem=PreRedraw.Peek();
	Editor::EditorShowMsg((wchar_t*)preRedrawItem.Param.Param1,(wchar_t*)preRedrawItem.Param.Param2,(wchar_t*)preRedrawItem.Param.Param3,(int)(INT_PTR)preRedrawItem.Param.Param4);
}


Edit *Editor::CreateString(const wchar_t *lpwszStr, int nLength)
{
	Edit *pEdit = new Edit(this, nullptr, lpwszStr ? false : true);

	if (pEdit)
	{
		pEdit->m_next = nullptr;
		pEdit->m_prev = nullptr;
		pEdit->SetTabSize(EdOpt.TabSize);
		pEdit->SetPersistentBlocks(EdOpt.PersistentBlocks);
		pEdit->SetConvertTabs(EdOpt.ExpandTabs);
		pEdit->SetCodePage(m_codepage);

		if (lpwszStr)
			pEdit->SetBinaryString(lpwszStr, nLength);

		pEdit->SetCurPos(0);
		pEdit->SetObjectColor(COL_EDITORTEXT,COL_EDITORSELECTEDTEXT);
		pEdit->SetEditorMode(TRUE);
		pEdit->SetWordDiv(EdOpt.strWordDiv);
		pEdit->SetShowWhiteSpace(EdOpt.ShowWhiteSpace);
	}

	return pEdit;
}

/*bool Editor::AddString (const wchar_t *lpwszStr, int nLength)
{
  Edit *pNewEdit = CreateString (lpwszStr, nLength);

  if ( !pNewEdit )
    return false;

  if ( !TopList || !NumLastLine ) //???
    TopList = EndList = TopScreen = CurLine = pNewEdit;
  else
  {
    Edit *PrevPtr;

    EndList->m_next = pNewEdit;

    PrevPtr = EndList;
    EndList = EndList->m_next;
    EndList->m_prev = PrevPtr;
    EndList->m_next = nullptr;
  }

  NumLastLine++;

  return true;
}*/

Edit *Editor::InsertString(const wchar_t *lpwszStr, int nLength, Edit *pAfter)
{
	Edit *pNewEdit = CreateString(lpwszStr, nLength);

	if (pNewEdit)
	{
		if (!TopList || !NumLastLine)   //???
			TopList = EndList = TopScreen = CurLine = pNewEdit;
		else
		{
			Edit *pWork = pAfter?pAfter:EndList;
			Edit *pNext = pWork->m_next;
			pNewEdit->m_next = pNext;
			pNewEdit->m_prev = pWork;
			pWork->m_next = pNewEdit;

			if (pNext)
				pNext->m_prev = pNewEdit;

			if (!pAfter)
				EndList = pNewEdit;
		}

		NumLastLine++;
	}

	return pNewEdit;
}


void Editor::SetCacheParams(EditorCacheParams *pp)
{
	bool translateTabs=false;
	SavePos=pp->SavePos;
	//m_codepage = pp->Table; //BUGBUG!!!, LoadFile do it itself

	if (StartLine == -2)  // from Viewer!
	{
		Edit *CurPtr=TopList;
		long TotalSize=0;

		while (CurPtr && CurPtr->m_next)
		{
			const wchar_t *SaveStr,*EndSeq;
			int Length;
			CurPtr->GetBinaryString(&SaveStr,&EndSeq,Length);
			TotalSize+=Length+StrLength(EndSeq);

			if (TotalSize > StartChar)
				break;

			CurPtr=CurPtr->m_next;
			NumLine++;
		}

		TopScreen=CurLine=CurPtr;

		if (NumLine == pp->Line - pp->ScreenLine)
		{
			Lock();

			for (DWORD I=0; I < (DWORD)pp->ScreenLine; I++)
				ProcessKey(KEY_DOWN);

			CurLine->SetTabCurPos(pp->LinePos);
			Unlock();
		}

		CurLine->SetLeftPos(pp->LeftPos);
	}
	else if (StartLine != -1 || EdOpt.SavePos)
	{
		if (StartLine!=-1)
		{
			pp->Line = StartLine-1;
			pp->ScreenLine = ObjHeight/2; //ScrY

			if (pp->ScreenLine > pp->Line)
				pp->ScreenLine = pp->Line;

			pp->LinePos = 0;
			if (StartChar > 0)
			{
				pp->LinePos = StartChar-1;
				translateTabs = true;
			}
		}

		if (pp->ScreenLine > ObjHeight) //ScrY //BUGBUG
			pp->ScreenLine=ObjHeight;//ScrY;

		if (pp->Line >= pp->ScreenLine)
		{
			Lock();
			GoToLine(pp->Line-pp->ScreenLine);
			TopScreen = CurLine;

			for (int I=0; I < pp->ScreenLine; I++)
				ProcessKey(KEY_DOWN);

			if(translateTabs) CurLine->SetCurPos(pp->LinePos);
			else CurLine->SetTabCurPos(pp->LinePos);
			CurLine->SetLeftPos(pp->LeftPos);
			Unlock();
		}
	}
}

void Editor::GetCacheParams(EditorCacheParams *pp)
{
	memset(pp, 0, sizeof(EditorCacheParams));
	memset(&pp->SavePos,0xff,sizeof(InternalEditorBookMark));
	pp->Line = NumLine;
	pp->ScreenLine = CalcDistance(TopScreen, CurLine,-1);
	pp->LinePos = CurLine->GetTabCurPos();
	pp->LeftPos = CurLine->GetLeftPos();
	pp->CodePage = m_codepage;

	if (Opt.EdOpt.SaveShortPos)
	{
		pp->SavePos=SavePos;
	}
}


bool Editor::SetCodePage(UINT codepage)
{
	if (m_codepage != codepage)
	{
		m_codepage = codepage;
		Edit *current = TopList;
		DWORD Result=0;

		while (current)
		{
			Result|=current->SetCodePage(m_codepage);
			current = current->m_next;
		}

		Show();
		return !Result; // BUGBUG, more details
	}

	return true;
}

UINT Editor::GetCodePage()
{
	return m_codepage;
}


void Editor::SetDialogParent(DWORD Sets)
{
}

void Editor::SetOvertypeMode(int Mode)
{
}

int Editor::GetOvertypeMode()
{
	return 0;
}

void Editor::SetEditBeyondEnd(int Mode)
{
}

void Editor::SetClearFlag(int Flag)
{
}

int Editor::GetClearFlag()
{
	return 0;
}

int Editor::GetCurCol()
{
	return CurLine->GetCurPos();
}

void Editor::SetCurPos(int NewCol, int NewRow)
{
	Lock();
	GoToLine(NewRow);
	CurLine->SetTabCurPos(NewCol);
	//CurLine->SetLeftPos(LeftPos); ???
	Unlock();
}

void Editor::SetCursorType(int Visible,int Size)
{
	CurLine->SetCursorType(Visible,Size); //???
}

void Editor::GetCursorType(int &Visible,int &Size)
{
	CurLine->GetCursorType(Visible,Size); //???
}

void Editor::SetObjectColor(int Color,int SelColor,int ColorUnChanged)
{
	for (Edit *CurPtr=TopList; CurPtr; CurPtr=CurPtr->m_next) //???
		CurPtr->SetObjectColor(Color,SelColor,ColorUnChanged);
}

void Editor::DrawScrollbar()
{
	if (EdOpt.ShowScrollBar)
	{
		SetColor(COL_EDITORSCROLLBAR);
		XX2=X2-(ScrollBarEx(X2,Y1,Y2-Y1+1,NumLine-CalcDistance(TopScreen,CurLine,-1),NumLastLine)?1:0);
	}
}
