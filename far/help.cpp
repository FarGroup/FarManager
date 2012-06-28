/*
help.cpp

Помощь
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

#include "help.hpp"
#include "keyboard.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "scantree.hpp"
#include "savescr.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "macroopcode.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "config.hpp"
#include "execute.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "exitcode.hpp"
#include "filestr.hpp"
#include "palette.hpp"

// Стек возврата
class CallBackStack
{
	private:
		struct ListNode
		{
			UINT64 Flags;             // флаги

			ListNode *Next;

			string strHelpTopic;        // текущий топик
			string strHelpPath;         // путь к хелпам
			string strSelTopic;         // текущее выделение
			string strHelpMask;         // маска

			int   TopStr;            // номер верхней видимой строки темы
			int   CurX,CurY;         // координаты (???)

			ListNode(const StackHelpData *Data, ListNode* n=nullptr)
			{
				strHelpTopic=Data->strHelpTopic;
				strHelpPath=Data->strHelpPath;
				strSelTopic=Data->strSelTopic;
				strHelpMask=Data->strHelpMask;
				Flags=Data->Flags;
				TopStr=Data->TopStr;
				CurX=Data->CurX;
				CurY=Data->CurY;
				Next=n;
			}
			~ListNode()
			{
			}
		};

		ListNode *topOfStack;

	public:
		CallBackStack() {topOfStack=nullptr;};
		~CallBackStack() {ClearStack();};

	public:
		void ClearStack();
		BOOL isEmpty() const {return !topOfStack;}

		void Push(const StackHelpData *Data);
		int Pop(StackHelpData *Data=nullptr);

		void PrintStack(const wchar_t *Title);
};


static const wchar_t *PluginContents=L"__PluginContents__";
static const wchar_t *HelpOnHelpTopic=L":Help";
static const wchar_t *HelpContents=L"Contents";

static int RunURL(const string& Protocol, wchar_t *URLPath);

Help::Help(const wchar_t *Topic, const wchar_t *Mask,UINT64 Flags):
	ErrorHelp(TRUE),
	IsNewTopic(TRUE),
	MouseDown(FALSE),
	CurColor(COL_HELPTEXT),
	CtrlTabSize(8),
	PrevMacroMode(CtrlObject->Macro.GetMode())
{
	MsX = MsY = -1;
	CanLoseFocus=FALSE;
	KeyBarVisible=TRUE;
	/* $ OT По умолчанию все хелпы создаются статически*/
	SetDynamicallyBorn(FALSE);
	CtrlObject->Macro.SetMode(MACRO_HELP);
	Stack=new CallBackStack;
	StackData.Clear();
	StackData.Flags=Flags;
	StackData.strHelpMask = Mask; // сохраним маску файла
	TopScreen=new SaveScreen;
	StackData.strHelpTopic = Topic;

	if (Opt.FullScreenHelp)
		SetPosition(0,0,ScrX,ScrY);
	else
		SetPosition(4,2,ScrX-4,ScrY-2);

	if (!ReadHelp(StackData.strHelpMask) && (Flags&FHELP_USECONTENTS))
	{
		StackData.strHelpTopic = Topic;

		if (StackData.strHelpTopic.At(0) == HelpBeginLink)
		{
			size_t pos;

			if (StackData.strHelpTopic.RPos(pos,HelpEndLink))
				StackData.strHelpTopic.SetLength(pos+1);

			StackData.strHelpTopic += HelpContents;
		}

		StackData.strHelpPath.Clear();
		ReadHelp(StackData.strHelpMask);
	}

	if (HelpList.getSize())
	{
		ScreenObject::Flags.Clear(FHELPOBJ_ERRCANNOTOPENHELP);
		InitKeyBar();
		MacroMode = MACRO_HELP;
		MoveToReference(1,1);
		FrameManager->ExecuteModal(this); //OT
	}
	else
	{
		ErrorHelp=TRUE;

		if (!(Flags&FHELP_NOSHOWERROR))
		{
			if (!ScreenObject::Flags.Check(FHELPOBJ_ERRCANNOTOPENHELP))
			{
				Message(MSG_WARNING,1,MSG(MHelpTitle),MSG(MHelpTopicNotFound),StackData.strHelpTopic,MSG(MOk));
			}

			ScreenObject::Flags.Clear(FHELPOBJ_ERRCANNOTOPENHELP);
		}
	}
}

Help::~Help()
{
	CtrlObject->Macro.SetMode(PrevMacroMode);
	SetRestoreScreenMode(FALSE);

	if (Stack)        delete Stack;

	if (TopScreen)    delete TopScreen;
}


void Help::Hide()
{
	ScreenObject::Hide();
}


int Help::ReadHelp(const wchar_t *Mask)
{
	wchar_t *ReadStr;
	string strSplitLine;
	int Formatting=TRUE,RepeatLastLine,BreakProcess;
	size_t PosTab;
	const int MaxLength=X2-X1-1;
	string strTabSpace;
	string strPath;

	if (StackData.strHelpTopic.At(0)==HelpBeginLink)
	{
		strPath = StackData.strHelpTopic.CPtr()+1;
		size_t pos;

		if (!strPath.Pos(pos,HelpEndLink))
			return FALSE;

		StackData.strHelpTopic = strPath.CPtr() + pos + 1;
		strPath.SetLength(pos);
		DeleteEndSlash(strPath,true);
		AddEndSlash(strPath);
		StackData.strHelpPath = strPath;
	}
	else
	{
		strPath = !StackData.strHelpPath.IsEmpty() ? StackData.strHelpPath:g_strFarPath;
	}

	if (!StrCmp(StackData.strHelpTopic,PluginContents))
	{
		strFullHelpPathName.Clear();
		ReadDocumentsHelp(HIDX_PLUGINS);
		return TRUE;
	}

	UINT nCodePage = CP_OEMCP;
	FILE *HelpFile=OpenLangFile(strPath,(!*Mask?HelpFileMask:Mask),Opt.strHelpLanguage,strFullHelpPathName, nCodePage);

	if (!HelpFile)
	{
		ErrorHelp=TRUE;

		if (!ScreenObject::Flags.Check(FHELPOBJ_ERRCANNOTOPENHELP))
		{
			ScreenObject::Flags.Set(FHELPOBJ_ERRCANNOTOPENHELP);

			if (!(StackData.Flags&FHELP_NOSHOWERROR))
			{
				Message(MSG_WARNING,1,MSG(MHelpTitle),MSG(MCannotOpenHelp),Mask,MSG(MOk));
			}
		}

		return FALSE;
	}

	string strReadStr;

	if (GetOptionsParam(HelpFile,L"TabSize",strReadStr, nCodePage))
	{
		CtrlTabSize=_wtoi(strReadStr);
	}

	if (CtrlTabSize < 0 || CtrlTabSize > 16)
		CtrlTabSize=Opt.HelpTabSize;

	if (GetOptionsParam(HelpFile,L"CtrlColorChar",strReadStr, nCodePage))
		strCtrlColorChar = strReadStr;
	else
		strCtrlColorChar.Clear();

	if (GetOptionsParam(HelpFile,L"CtrlStartPosChar",strReadStr, nCodePage))
		strCtrlStartPosChar = strReadStr;
	else
		strCtrlStartPosChar.Clear();

	/* $ 29.11.2001 DJ
	   запомним, чего там написано в PluginContents
	*/
	if (!GetLangParam(HelpFile,L"PluginContents",&strCurPluginContents, nullptr, nCodePage))
		strCurPluginContents.Clear();

	HelpList.Free();

	StrCount=0;
	FixCount=0;
	TopicFound=0;
	RepeatLastLine=FALSE;
	BreakProcess=FALSE;
	int NearTopicFound=0;
	wchar_t PrevSymbol=0;

	LPWSTR TabSpace=strTabSpace.GetBuffer(CtrlTabSize+1);
	for (int i=0; i < CtrlTabSize; i++)
	{
		TabSpace[i]=L' ';
	}
	strTabSpace.ReleaseBuffer(CtrlTabSize);

	StartPos = (DWORD)-1;
	LastStartPos = (DWORD)-1;
	int RealMaxLength;
	bool MacroProcess=false;
	int MI=0;
	string strMacroArea;

	OldGetFileString GetStr(HelpFile);
	int nStrLength;
	size_t SizeKeyName=20;

	for (;;)
	{
		if (StartPos != (DWORD)-1)
			RealMaxLength = MaxLength-StartPos;
		else
			RealMaxLength = MaxLength;

		if (!MacroProcess && !RepeatLastLine && !BreakProcess)
		{
			if (GetStr.GetString(&ReadStr, nCodePage, nStrLength) <= 0)
			{
				strReadStr=ReadStr;
				if (StringLen(strSplitLine)<MaxLength)
				{
					if (strSplitLine.At(0))
						AddLine(strSplitLine.CPtr());
				}
				else
				{
					strReadStr.Clear();
					RepeatLastLine=TRUE;
					continue;
				}

				break;
			}
			else
			{
				strReadStr=ReadStr;
			}
		}

		if (MacroProcess)
		{
			string strDescription;
			string strKeyName;
			string strOutTemp;

			if (CtrlObject->Macro.GetMacroKeyInfo(false,CtrlObject->Macro.GetAreaCode(strMacroArea),MI,strKeyName,strDescription) == -1)
			{
				MacroProcess=false;
				MI=0;
				continue;
			}

			if (strKeyName.At(0) == L'~')
			{
				MI++;
				continue;
			}

			ReplaceStrings(strKeyName,L"~",L"~~",-1);
			ReplaceStrings(strKeyName,L"#",L"##",-1);
			ReplaceStrings(strKeyName,L"@",L"@@",-1);

			if (wcschr(strKeyName,L'~')) // корректировка размера
				SizeKeyName++;

			strOutTemp.Format(L" #%-*.*s# ",SizeKeyName,SizeKeyName,strKeyName.CPtr());

			if (!strDescription.IsEmpty())
			{
				ReplaceStrings(strDescription,L"#",L"##",-1);
				ReplaceStrings(strDescription,L"~",L"~~",-1);
				ReplaceStrings(strDescription,L"@",L"@@",-1);
				strOutTemp+=strCtrlStartPosChar;
				strOutTemp+=strDescription;
			}

			strReadStr=strOutTemp;
			MacroProcess=true;
			MI++;
		}

		RepeatLastLine=FALSE;

		while (strReadStr.Pos(PosTab,L'\t'))
		{
			wchar_t *lpwszPtr = strReadStr.GetBuffer();
			lpwszPtr[PosTab]=L' ';
			strReadStr.ReleaseBuffer();

			if (CtrlTabSize > 1) // заменим табулятор по всем праивилам
				strReadStr.Insert(PosTab, strTabSpace.CPtr(), CtrlTabSize - (PosTab % CtrlTabSize));
		}

		RemoveTrailingSpaces(strReadStr);

		if (!strCtrlStartPosChar.IsEmpty() && wcsstr(strReadStr, strCtrlStartPosChar))
		{
			string strLine;
			ReadStr = strReadStr.GetBuffer();
			int Length = (int)(wcsstr(ReadStr, strCtrlStartPosChar)-ReadStr);
			strLine = ReadStr;
			strLine.SetLength(Length);
			LastStartPos = StringLen(strLine);
			wcscpy(ReadStr+Length, ReadStr+Length+strCtrlStartPosChar.GetLength());
			strReadStr.ReleaseBuffer();
		}

		if (TopicFound)
		{
			HighlightsCorrection(strReadStr);
		}

		if (strReadStr.At(0)==L'@' && !BreakProcess)
		{
			if (TopicFound)
			{
				if (!StrCmp(strReadStr,L"@+"))
				{
					Formatting=TRUE;
					PrevSymbol=0;
					continue;
				}

				if (!StrCmp(strReadStr,L"@-"))
				{
					Formatting=FALSE;
					PrevSymbol=0;
					continue;
				}

				if (strSplitLine.At(0))
				{
					BreakProcess=TRUE;
					strReadStr.Clear();
					PrevSymbol=0;
					goto m1;
				}

				break;
			}
			else if (!StrCmpI(strReadStr.CPtr()+1,StackData.strHelpTopic))
			{
				TopicFound=1;
				NearTopicFound=1;
			}
		}
		else
		{
m1:
			if (strReadStr.IsEmpty() && BreakProcess && strSplitLine.IsEmpty())
				break;

			if (TopicFound)
			{
				if (!StrCmpNI(strReadStr.CPtr(),L"<!Macro:",8) && CtrlObject)
				{
					if (strReadStr.Pos(PosTab,L'>') && strReadStr.At(PosTab-1) != L'!')
						continue;

					strMacroArea=strReadStr.SubStr(8,PosTab-1-8); //???
					MacroProcess=true;
					MI=0;
					string strDescription,strKeyName;
					while (CtrlObject->Macro.GetMacroKeyInfo(false,CtrlObject->Macro.GetAreaCode(strMacroArea),MI,strKeyName,strDescription) != -1)
					{
						SizeKeyName=Max(SizeKeyName,strKeyName.GetLength());
						MI++;
					}
					MI=0;
					continue;
				}

				if (!(strReadStr.At(0)==L'$' && NearTopicFound && (PrevSymbol == L'$' || PrevSymbol == L'@')))
					NearTopicFound=0;

				/* $<text> в начале строки, определение темы
				   Определяет не прокручиваемую область помощи
				   Если идут несколько подряд сразу после строки обозначения темы...
				*/
				if (NearTopicFound)
				{
					StartPos = (DWORD)-1;
					LastStartPos = (DWORD)-1;
				}

				if (strReadStr.At(0)==L'$' && NearTopicFound && (PrevSymbol == L'$' || PrevSymbol == L'@'))
				{
					AddLine(strReadStr.CPtr()+1);
					FixCount++;
				}
				else
				{
					NearTopicFound=0;

					if (!strReadStr.At(0) || !Formatting)
					{
						if (!strSplitLine.IsEmpty())
						{
							if (StringLen(strSplitLine)<RealMaxLength)
							{
								AddLine(strSplitLine);
								strSplitLine.Clear();

								if (StringLen(strReadStr)<RealMaxLength)
								{
									AddLine(strReadStr);
									LastStartPos = (DWORD)-1;
									StartPos = (DWORD)-1;
									continue;
								}
							}
							else
								RepeatLastLine=TRUE;
						}
						else if (!strReadStr.IsEmpty())
						{
							if (StringLen(strReadStr)<RealMaxLength)
							{
								AddLine(strReadStr);
								continue;
							}
						}
						else if (strReadStr.IsEmpty() && strSplitLine.IsEmpty())
						{
							AddLine(L"");
							continue;
						}
					}

					if (!strReadStr.IsEmpty() && IsSpace(strReadStr.At(0)) && Formatting)
					{
						if (StringLen(strSplitLine)<RealMaxLength)
						{
							if (!strSplitLine.IsEmpty())
							{
								AddLine(strSplitLine);
								StartPos = (DWORD)-1;
							}

							strSplitLine=strReadStr;
							strReadStr.Clear();
							continue;
						}
						else
							RepeatLastLine=TRUE;
					}

					if (!RepeatLastLine)
					{
						if (!strSplitLine.IsEmpty())
							strSplitLine += L" ";

						strSplitLine += strReadStr;
					}

					if (StringLen(strSplitLine)<RealMaxLength)
					{
						if (strReadStr.IsEmpty() && BreakProcess)
							goto m1;

						continue;
					}

					int Splitted=0;

					for (int I=(int)strSplitLine.GetLength()-1; I > 0; I--)
					{
						if (I > 0 && strSplitLine.At(I)==L'~' && strSplitLine.At(I-1)==L'~')
						{
							I--;
							continue;
						}

						if (I > 0 && strSplitLine.At(I)==L'~' && strSplitLine.At(I-1)!=L'~')
						{
							do
							{
								I--;
							}
							while (I > 0 && strSplitLine.At(I)!=L'~');

							continue;
						}

						if (strSplitLine.At(I)==L' ')
						{
							wchar_t *lpwszPtr = strSplitLine.GetBuffer();
							lpwszPtr[I]=0;

							if (StringLen(lpwszPtr)<RealMaxLength)
							{
								AddLine(lpwszPtr);
								wmemmove(lpwszPtr+1,lpwszPtr+I+1,StrLength(lpwszPtr+I+1)+1);
								*lpwszPtr=L' ';
								strSplitLine.ReleaseBuffer();

								HighlightsCorrection(strSplitLine);
								Splitted=TRUE;
								break;
							}
							else
							{
								lpwszPtr[I]=L' ';
								strSplitLine.ReleaseBuffer();
							}
						}
					}

					if (!Splitted)
					{
						AddLine(strSplitLine);
						strSplitLine.Clear();
					}
					else
					{
						StartPos = LastStartPos;
					}
				}
			}

			if (BreakProcess)
			{
				if (!strSplitLine.IsEmpty())
					goto m1;

				break;
			}
		}

		PrevSymbol=strReadStr.At(0);
	}

	AddLine(L"");
	fclose(HelpFile);
	FixSize=FixCount?FixCount+1:0;
	ErrorHelp=FALSE;

	if (IsNewTopic)
	{
		StackData.CurX=StackData.CurY=0;
		StackData.TopStr=0;
	}

	return TopicFound;
}


void Help::AddLine(const wchar_t *Line)
{
	string strLine;

	if (StartPos != 0xFFFFFFFF)
	{
		DWORD StartPos0=StartPos;
		if (*Line == L' ')
			StartPos0--;

		if (StartPos0 > 0)
		{
			LPWSTR Space=strLine.GetBuffer(StartPos0+1);
			for (DWORD i=0; i < StartPos0; i++)
			{
				Space[i]=L' ';
			}
			strLine.ReleaseBuffer(StartPos0);
		}
	}

	strLine += Line;

	{
		HelpRecord AddRecord(strLine);
		HelpList.addItem(AddRecord);
	}

	StrCount++;
}

void Help::AddTitle(const wchar_t *Title)
{
	AddLine(FormatString() << L"^ #" << Title << L"#");
}

void Help::HighlightsCorrection(string &strStr)
{
	int I,Count;

	for (I=0,Count=0; strStr.At(I) ; I++)
		if (strStr.At(I) == L'#')
			Count++;

	if ((Count & 1) && strStr.At(0) != L'$')
		strStr.Insert(0, L'#');
}

void Help::DisplayObject()
{
	if (!TopScreen)
		TopScreen=new SaveScreen;

	if (!TopicFound)
	{
		if (!ErrorHelp) // если это убрать, то при несуществующей ссылки
		{              // с нынешним манагером попадаем в бесконечный цикл.
			ErrorHelp=TRUE;

			if (!(StackData.Flags&FHELP_NOSHOWERROR))
			{
				Message(MSG_WARNING,1,MSG(MHelpTitle),MSG(MHelpTopicNotFound),StackData.strHelpTopic,MSG(MOk));
			}

			ProcessKey(KEY_ALTF1);
		}

		return;
	}

	SetCursorType(0,10);

	if (StackData.strSelTopic.IsEmpty())
		MoveToReference(1,1);

	FastShow();

	if (!Opt.FullScreenHelp)
	{
		HelpKeyBar.SetPosition(0,ScrY,ScrX,ScrY);

		if (Opt.ShowKeyBar)
			HelpKeyBar.Show();
	}
	else
	{
		HelpKeyBar.Hide();
	}
}


void Help::FastShow()
{
	if (!Locked())
		DrawWindowFrame();

	CorrectPosition();
	StackData.strSelTopic.Clear();
	/* $ 01.09.2000 SVS
	   Установим по умолчанию текущий цвет отрисовки...
	   чтобы новая тема начиналась с нормальными атрибутами
	*/
	CurColor=COL_HELPTEXT;

	for (int i=0; i<Y2-Y1-1; i++)
	{
		int StrPos;

		if (i<FixCount)
		{
			StrPos=i;
		}
		else if (i==FixCount && FixCount>0)
		{
			if (!Locked())
			{
				GotoXY(X1,Y1+i+1);
				SetColor(COL_HELPBOX);
				ShowSeparator(X2-X1+1,1);
			}

			continue;
		}
		else
		{
			StrPos=i+StackData.TopStr;

			if (FixCount>0)
				StrPos--;
		}

		if (StrPos<StrCount)
		{
			const HelpRecord *rec=GetHelpItem(StrPos);
			const wchar_t *OutStr=rec?rec->HelpStr:nullptr;

			if (!OutStr)
				OutStr=L"";

			if (*OutStr==L'^')
			{
				OutStr++;
				GotoXY(X1+1+Max(0,(X2-X1-1-StringLen(OutStr))/2),Y1+i+1);
			}
			else
			{
				GotoXY(X1+1,Y1+i+1);
			}

			OutString(OutStr);
		}
	}

	if (!Locked())
	{
		SetColor(COL_HELPSCROLLBAR);
		ScrollBarEx(X2,Y1+FixSize+1,Y2-Y1-FixSize-1,StackData.TopStr,StrCount-FixCount);
	}
}

void Help::DrawWindowFrame()
{
	SetScreen(X1,Y1,X2,Y2,L' ',ColorIndexToColor(COL_HELPTEXT));
	Box(X1,Y1,X2,Y2,ColorIndexToColor(COL_HELPBOX),DOUBLE_BOX);
	SetColor(COL_HELPBOXTITLE);
	string strHelpTitleBuf;
	strHelpTitleBuf = MSG(MHelpTitle);
	strHelpTitleBuf += L" - ";

	if (!strCurPluginContents.IsEmpty())
		strHelpTitleBuf += strCurPluginContents;
	else
		strHelpTitleBuf += L"FAR";

	TruncStrFromEnd(strHelpTitleBuf,X2-X1-3);
	GotoXY(X1+(X2-X1+1-(int)strHelpTitleBuf.GetLength()-2)/2,Y1);
	FS<<L" "<<strHelpTitleBuf<<L" ";
}

static const wchar_t *SkipLink( const wchar_t *Str, string *Name )
{
	for (;;)
	{
		while (*Str && *Str != L'@')
		{
			if (Name)
				Name->Append(*Str);
			++Str;
		}
		if (*Str)
			++Str;
		if (*Str != L'@')
			break;
		if (Name)
			Name->Append(*Str);
		++Str;
	}
	return Str;
}

static bool GetHelpColor(const wchar_t* &Str, wchar_t cColor, int &color)
{
	if (!cColor || Str[0] != cColor)
		return false;

   wchar_t wc1 = Str[1];
	if (wc1 == L'-')     // '\-' set default color
	{
		color = COL_HELPTEXT;
		Str += 2;
		return true;
	}

	if (!iswxdigit(wc1)) // '\hh' custom color
		return false;
	wchar_t wc2 = Str[2];
	if (!iswxdigit(wc2))
		return false;

	if (wc1 > L'9') wc1 -= L'A' - 10;
	if (wc2 > L'9') wc2 -= L'A' - 10;
	color = ((wc1 & 0x0f) << 4) | (wc2 & 0x0f);
	Str += 3;
	return true;
}

static bool FastParseLine(
	const wchar_t *Str, int *pLen, int x0, int realX, string *pTopic, wchar_t cColor)
{
	int x = x0, start_topic = -1;
	bool found = false;
	while (*Str)
	{
		wchar_t wc = *Str++;
		if (wc == *Str && (wc == L'~' || wc == L'@' || wc == L'#' || wc == cColor))
			++Str;
		else if (wc == L'#') // start/stop highlighting
			continue;
		else if (cColor && wc == cColor)
		{
			if (*Str == L'-')	// '\-' default color
			{ Str += 2-1; continue; }
			else if (iswxdigit(*Str) && iswxdigit(Str[1])) // '\hh' custom color
			{ Str += 3-1; continue; }
		}
		else if (wc == L'@')	// skip topic link //??? is it valid without ~topic~
		{
			Str = SkipLink(Str, nullptr);
			continue;
		}
		else if (wc == L'~')	// start/stop topic
		{
			if (start_topic < 0)
				start_topic = x;
			else
			{
				found = (realX >= start_topic && realX < x);
				if (*Str == L'@')
					Str = SkipLink(Str+1, found ? pTopic : nullptr);
				if (found)
					break;
				start_topic = -1;
			}
			continue;
		}

      ++x;
		if (realX >= 0 && x > realX && start_topic < 0)
			break;
	}

	if (pLen)
		*pLen = x - x0;
	return found;
}

bool Help::GetTopic(int realX, int realY, string& strTopic)
{
	strTopic.Clear();
	if (realY <= Y1 || realY >= Y2 || realX <= X1 || realX >= X2)
		return false;

	int y = -1;
	if (realY-Y1 <= FixSize)
	{
		if (y != FixCount)
			y = realY - Y1 - 1;
	}
	else
		y = realY - Y1 - 1 - FixSize+FixCount + StackData.TopStr;

	if (y < 0 || y >= StrCount)
		return false;
	const HelpRecord *rec = GetHelpItem(y);
	if (!rec || !rec->HelpStr)
		return false;

	int x = X1 + 1;
	const wchar_t *Str = rec->HelpStr;
	if (*Str == L'^') // center
	{
		int w = StringLen(++Str);
		x = X1 + 1 + Max(0, (X2 - X1 - 1 - w)/2);
	}

	return FastParseLine(Str, nullptr, x, realX, &strTopic, strCtrlColorChar.At(0));
}

int Help::StringLen(const wchar_t *Str)
{
	int len = 0;
	FastParseLine(Str, &len, 0, -1, nullptr, strCtrlColorChar.At(0));
	return len;
}

void Help::OutString(const wchar_t *Str)
{
	wchar_t OutStr[512]; //BUGBUG
	const wchar_t *StartTopic=nullptr;
	int OutPos=0,Highlight=0,Topic=0;
	wchar_t cColor = strCtrlColorChar.At(0);

	while (OutPos<(int)(ARRAYSIZE(OutStr)-10))
	{
		if ((Str[0]==L'~' && Str[1]==L'~') ||
		        (Str[0]==L'#' && Str[1]==L'#') ||
		        (Str[0]==L'@' && Str[1]==L'@') ||
		        (cColor && Str[0]==cColor && Str[1]==cColor)
		   )
		{
			OutStr[OutPos++]=*Str;
			Str+=2;
			continue;
		}

		if (*Str==L'~' || ((*Str==L'#' || *Str == cColor) && !Topic) /*|| *Str==HelpBeginLink*/ || !*Str)
		{
			OutStr[OutPos]=0;

			if (Topic)
			{
				int RealCurX,RealCurY;
				RealCurX=X1+StackData.CurX+1;
				RealCurY=Y1+StackData.CurY+FixSize+1;
				bool found = WhereY()==RealCurY && RealCurX>=WhereX() && RealCurX<WhereX()+(Str-StartTopic)-1;

				SetColor(found ? COL_HELPSELECTEDTOPIC : COL_HELPTOPIC);
				if (*Str && Str[1]==L'@')
				{
					Str = SkipLink(Str+2, found ? &StackData.strSelTopic : nullptr);
					Topic = 0;
				}
			}
			else
			{
				SetColor(Highlight ? COL_HELPHIGHLIGHTTEXT : CurColor);
			}

			/* $ 24.09.2001 VVM
			  ! Обрежем длинные строки при показе. Такое будет только при длинных ссылках... */
			if (static_cast<int>(StrLength(OutStr) + WhereX()) > X2)
				OutStr[X2 - WhereX()] = 0;

			if (Locked())
				GotoXY(WhereX()+StrLength(OutStr),WhereY());
			else
				Text(OutStr);

			OutPos=0;
		}

		if (!*Str)
		{
			break;
		}
		else if (*Str==L'~')
		{
			if (!Topic)
				StartTopic = Str;
			Topic = !Topic;
			Str++;
		}
		else if (*Str==L'@')
		{
			Str = SkipLink(Str+1, nullptr);
		}
		else if (*Str==L'#')
		{
			Highlight = !Highlight;
			Str++;
		}
		else if (!GetHelpColor(Str, cColor, CurColor))
		{
			OutStr[OutPos++]=*(Str++);
		}
	}

	if (!Locked() && WhereX()<X2)
	{
		SetColor(CurColor);
		FS<<fmt::MinWidth(X2-WhereX())<<L"";
	}
}


void Help::CorrectPosition()
{
	if (StackData.CurX>X2-X1-2)
		StackData.CurX=X2-X1-2;

	if (StackData.CurX<0)
		StackData.CurX=0;

	if (StackData.CurY>Y2-Y1-2-FixSize)
	{
		StackData.TopStr+=StackData.CurY-(Y2-Y1-2-FixSize);
		StackData.CurY=Y2-Y1-2-FixSize;
	}

	if (StackData.CurY<0)
	{
		StackData.TopStr+=StackData.CurY;
		StackData.CurY=0;
	}

	if (StackData.TopStr>StrCount-FixCount-(Y2-Y1-1-FixSize))
		StackData.TopStr=StrCount-FixCount-(Y2-Y1-1-FixSize);

	if (StackData.TopStr<0)
		StackData.TopStr=0;
}

#ifdef FAR_LUA
#else
__int64 Help::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	switch (OpCode)
	{
		case MCODE_V_HELPFILENAME: // Help.FileName
			*(string *)vParam=strFullHelpPathName;     // ???
			break;
		case MCODE_V_HELPTOPIC: // Help.Topic
			*(string *)vParam=StackData.strHelpTopic;  // ???
			break;
		case MCODE_V_HELPSELTOPIC: // Help.SELTopic
			*(string *)vParam=StackData.strSelTopic;   // ???
			break;
		default:
			return 0;
	}

	return 1;
}
#endif

int Help::ProcessKey(int Key)
{
	if (StackData.strSelTopic.IsEmpty())
		StackData.CurX=StackData.CurY=0;

	switch (Key)
	{
		case KEY_NONE:
		case KEY_IDLE:
		{
			break;
		}
		case KEY_F5:
		{
			Opt.FullScreenHelp=!Opt.FullScreenHelp;
			ResizeConsole();
			return TRUE;
		}
		case KEY_ESC:
		case KEY_F10:
		{
			FrameManager->DeleteFrame();
			SetExitCode(XC_QUIT);
			return TRUE;
		}
		case KEY_HOME:        case KEY_NUMPAD7:
		case KEY_CTRLHOME:    case KEY_CTRLNUMPAD7:
		case KEY_RCTRLHOME:   case KEY_RCTRLNUMPAD7:
		case KEY_CTRLPGUP:    case KEY_CTRLNUMPAD9:
		case KEY_RCTRLPGUP:   case KEY_RCTRLNUMPAD9:
		{
			StackData.CurX=StackData.CurY=0;
			StackData.TopStr=0;
			FastShow();

			if (StackData.strSelTopic.IsEmpty())
				MoveToReference(1,1);

			return TRUE;
		}
		case KEY_END:         case KEY_NUMPAD1:
		case KEY_CTRLEND:     case KEY_CTRLNUMPAD1:
		case KEY_RCTRLEND:    case KEY_RCTRLNUMPAD1:
		case KEY_CTRLPGDN:    case KEY_CTRLNUMPAD3:
		case KEY_RCTRLPGDN:   case KEY_RCTRLNUMPAD3:
		{
			StackData.CurX=StackData.CurY=0;
			StackData.TopStr=StrCount;
			FastShow();

			if (StackData.strSelTopic.IsEmpty())
			{
				StackData.CurX=0;
				StackData.CurY=Y2-Y1-2-FixSize;
				MoveToReference(0,1);
			}

			return TRUE;
		}
		case KEY_UP:          case KEY_NUMPAD8:
		{
			if (StackData.TopStr>0)
			{
				StackData.TopStr--;

				if (StackData.CurY<Y2-Y1-2-FixSize)
				{
					StackData.CurX=X2-X1-2;
					StackData.CurY++;
				}

				FastShow();

				if (StackData.strSelTopic.IsEmpty())
					MoveToReference(0,1);
			}
			else
				ProcessKey(KEY_SHIFTTAB);

			return TRUE;
		}
		case KEY_DOWN:        case KEY_NUMPAD2:
		{
			if (StackData.TopStr<StrCount-FixCount-(Y2-Y1-1-FixSize))
			{
				StackData.TopStr++;

				if (StackData.CurY>0)
					StackData.CurY--;

				StackData.CurX=0;
				FastShow();

				if (StackData.strSelTopic.IsEmpty())
					MoveToReference(1,1);
			}
			else
				ProcessKey(KEY_TAB);

			return TRUE;
		}
		/* $ 26.07.2001 VVM
		  + С альтом скролим по 1 */
		case KEY_MSWHEEL_UP:
		case(KEY_MSWHEEL_UP | KEY_ALT):
		case(KEY_MSWHEEL_UP | KEY_RALT):
		{
			int n = (Key == KEY_MSWHEEL_UP ? (int)Opt.MsWheelDeltaHelp : 1);
			while (n-- > 0)
				ProcessKey(KEY_UP);

			return TRUE;
		}
		case KEY_MSWHEEL_DOWN:
		case(KEY_MSWHEEL_DOWN | KEY_ALT):
		case(KEY_MSWHEEL_DOWN | KEY_RALT):
		{
			int n = (Key == KEY_MSWHEEL_DOWN ? (int)Opt.MsWheelDeltaHelp : 1);
			while (n-- > 0)
				ProcessKey(KEY_DOWN);

			return TRUE;
		}
		case KEY_PGUP:      case KEY_NUMPAD9:
		{
			StackData.CurX=StackData.CurY=0;
			StackData.TopStr-=Y2-Y1-2-FixSize;
			FastShow();

			if (StackData.strSelTopic.IsEmpty())
			{
				StackData.CurX=StackData.CurY=0;
				MoveToReference(1,1);
			}

			return TRUE;
		}
		case KEY_PGDN:      case KEY_NUMPAD3:
		{
			{
				int PrevTopStr=StackData.TopStr;
				StackData.TopStr+=Y2-Y1-2-FixSize;
				FastShow();

				if (StackData.TopStr==PrevTopStr)
				{
					ProcessKey(KEY_CTRLPGDN);
					return TRUE;
				}
				else
					StackData.CurX=StackData.CurY=0;

				MoveToReference(1,1);
			}
			return TRUE;
		}
		case KEY_RIGHT:   case KEY_NUMPAD6:   case KEY_MSWHEEL_RIGHT:
		case KEY_TAB:
		{
			MoveToReference(1,0);
			return TRUE;
		}
		case KEY_LEFT:    case KEY_NUMPAD4:   case KEY_MSWHEEL_LEFT:
		case KEY_SHIFTTAB:
		{
			MoveToReference(0,0);
			return TRUE;
		}
		case KEY_F1:
		{
			// не поганим SelTopic, если и так в Help on Help
			if (StrCmpI(StackData.strHelpTopic,HelpOnHelpTopic))
			{
				Stack->Push(&StackData);
				IsNewTopic=TRUE;
				JumpTopic(HelpOnHelpTopic);
				IsNewTopic=FALSE;
				ErrorHelp=FALSE;
			}

			return TRUE;
		}
		case KEY_SHIFTF1:
		{
			//   не поганим SelTopic, если и так в теме Contents
			if (StrCmpI(StackData.strHelpTopic,HelpContents))
			{
				Stack->Push(&StackData);
				IsNewTopic=TRUE;
				JumpTopic(HelpContents);
				ErrorHelp=FALSE;
				IsNewTopic=FALSE;
			}

			return TRUE;
		}
		case KEY_SHIFTF2:
		{
			//   не поганим SelTopic, если и так в PluginContents
			if (StrCmpI(StackData.strHelpTopic,PluginContents))
			{
				Stack->Push(&StackData);
				IsNewTopic=TRUE;
				JumpTopic(PluginContents);
				ErrorHelp=FALSE;
				IsNewTopic=FALSE;
			}

			return TRUE;
		}
		case KEY_ALTF1:
		case KEY_RALTF1:
		case KEY_BS:
		{
			// Если стек возврата пуст - выходим их хелпа
			if (!Stack->isEmpty())
			{
				Stack->Pop(&StackData);
				JumpTopic(StackData.strHelpTopic);
				ErrorHelp=FALSE;
				return TRUE;
			}

			return ProcessKey(KEY_ESC);
		}
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (!StackData.strSelTopic.IsEmpty() && StrCmpI(StackData.strHelpTopic,StackData.strSelTopic))
			{
				Stack->Push(&StackData);
				IsNewTopic=TRUE;

				if (!JumpTopic())
				{
					Stack->Pop(&StackData);
					ReadHelp(StackData.strHelpMask); // вернем то, что отображали.
				}

				ErrorHelp=FALSE;
				IsNewTopic=FALSE;
			}

			return TRUE;
		}
	}

	return FALSE;
}

int Help::JumpTopic(const wchar_t *JumpTopic)
{
	string strNewTopic;
	size_t pos;
	Stack->PrintStack(JumpTopic);

	if (JumpTopic)
		StackData.strSelTopic = JumpTopic;

	/* $ 14.07.2002 IS
	     При переходе по ссылкам используем всегда только абсолютные пути,
	     если это возможно.
	*/

	// Если ссылка на другой файл, путь относительный и есть то, от чего можно
	// вычислить абсолютный путь, то сделаем это
	if (StackData.strSelTopic.At(0)==HelpBeginLink
	        && StackData.strSelTopic.Pos(pos,HelpEndLink,2)
	        && !IsAbsolutePath(StackData.strSelTopic.CPtr()+1)
	        && !StackData.strHelpPath.IsEmpty())
	{
		string strFullPath;
		wchar_t *lpwszHelpTopic = strNewTopic.GetBuffer(pos);
		xwcsncpy(lpwszHelpTopic, StackData.strSelTopic.CPtr()+1,pos);
		strNewTopic.ReleaseBuffer();
		strFullPath = StackData.strHelpPath;
		// уберем _все_ конечные слеши и добавим один
		DeleteEndSlash(strFullPath, true);
		strFullPath += L"\\";
		strFullPath += strNewTopic.CPtr()+(IsSlash(strNewTopic.At(0))?1:0);
		BOOL addSlash=DeleteEndSlash(strFullPath);
		ConvertNameToFull(strFullPath,strNewTopic);
		strFullPath.Format(addSlash?HelpFormatLink:HelpFormatLinkModule, strNewTopic.CPtr(), wcschr(StackData.strSelTopic.CPtr()+2, HelpEndLink)+1);
		StackData.strSelTopic = strFullPath;
	}

	//_SVS(SysLog(L"JumpTopic() = SelTopic=%s",StackData.SelTopic));
	// URL активатор - это ведь так просто :-)))
	{
		strNewTopic = StackData.strSelTopic;

		if (strNewTopic.Pos(pos,L':') && strNewTopic.At(0) != L':') // наверное подразумевается URL
		{
			string Protocol(strNewTopic, pos);
			wchar_t *lpwszTopic = StackData.strSelTopic.GetBuffer();

			if (RunURL(Protocol, lpwszTopic))
			{
				StackData.strSelTopic.ReleaseBuffer();
				return FALSE;
			}
			else
			{
				StackData.strSelTopic.ReleaseBuffer();
			}
		}
	}
	// а вот теперь попробуем...

	//_SVS(SysLog(L"JumpTopic() = SelTopic=%s, StackData.HelpPath=%s",StackData.SelTopic,StackData.HelpPath));
	if (!StackData.strHelpPath.IsEmpty() && StackData.strSelTopic.At(0) !=HelpBeginLink && StrCmp(StackData.strSelTopic,HelpOnHelpTopic))
	{
		if (StackData.strSelTopic.At(0)==L':')
		{
			strNewTopic = StackData.strSelTopic.CPtr()+1;
			StackData.Flags&=~FHELP_CUSTOMFILE;
		}
		else if (StackData.Flags&FHELP_CUSTOMFILE)
			strNewTopic = StackData.strSelTopic;
		else
			strNewTopic.Format(HelpFormatLink,StackData.strHelpPath.CPtr(),StackData.strSelTopic.CPtr());
	}
	else
	{
		strNewTopic = StackData.strSelTopic.CPtr()+(!StrCmp(StackData.strSelTopic,HelpOnHelpTopic)?1:0);
	}

	// удалим ссылку на .DLL
	wchar_t *lpwszNewTopic = strNewTopic.GetBuffer();
	wchar_t *p=wcsrchr(lpwszNewTopic,HelpEndLink);

	if (p)
	{
		if (!IsSlash(*(p-1)))
		{
			const wchar_t *p2=p;

			while (p >= lpwszNewTopic)
			{
				if (IsSlash(*p))
				{
					//++p;
					if (*p)
					{
						StackData.Flags|=FHELP_CUSTOMFILE;
						StackData.strHelpMask = p+1;
						wchar_t *lpwszMask = StackData.strHelpMask.GetBuffer();
						*wcsrchr(lpwszMask,HelpEndLink)=0;
						StackData.strHelpMask.ReleaseBuffer();
					}

					wmemmove(p,p2,StrLength(p2)+1);
					const wchar_t *p3=wcsrchr(StackData.strHelpMask,L'.');

					if (p3 && StrCmpI(p3,L".hlf"))
						StackData.strHelpMask.Clear();

					break;
				}

				--p;
			}
		}
		else
		{
			StackData.Flags&=~FHELP_CUSTOMFILE;
			StackData.Flags|=FHELP_CUSTOMPATH;
		}
	}

	strNewTopic.ReleaseBuffer();

	//_SVS(SysLog(L"HelpMask=%s NewTopic=%s",StackData.HelpMask,NewTopic));
	if (StackData.strSelTopic.At(0) != L':' &&
	        StrCmpI(StackData.strSelTopic,PluginContents)
	   )
	{
		if (!(StackData.Flags&FHELP_CUSTOMFILE) && wcsrchr(strNewTopic,HelpEndLink))
		{
			StackData.strHelpMask.Clear();
		}
	}
	else
	{
		StackData.strHelpMask.Clear();
	}

	StackData.strHelpTopic = strNewTopic;

	if (!(StackData.Flags&FHELP_CUSTOMFILE))
		StackData.strHelpPath.Clear();

	if (!ReadHelp(StackData.strHelpMask))
	{
		StackData.strHelpTopic = strNewTopic;

		if (StackData.strHelpTopic.At(0) == HelpBeginLink)
		{
			if (StackData.strHelpTopic.RPos(pos,HelpEndLink))
			{
				StackData.strHelpTopic.SetLength(pos+1);
				StackData.strHelpTopic += HelpContents;
			}
		}

		StackData.strHelpPath.Clear();
		ReadHelp(StackData.strHelpMask);
	}

	ScreenObject::Flags.Clear(FHELPOBJ_ERRCANNOTOPENHELP);

	if (!HelpList.getSize())
	{
		ErrorHelp=TRUE;

		if (!(StackData.Flags&FHELP_NOSHOWERROR))
		{
			Message(MSG_WARNING,1,MSG(MHelpTitle),MSG(MHelpTopicNotFound),StackData.strHelpTopic,MSG(MOk));
		}

		return FALSE;
	}

	// ResizeConsole();
	if (IsNewTopic
	        || !StrCmpI(StackData.strSelTopic,PluginContents) // Это неприятный костыль :-((
	   )
		MoveToReference(1,1);

	//FrameManager->ImmediateHide();
	FrameManager->RefreshFrame();
	return TRUE;
}



int Help::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	if (HelpKeyBar.ProcessMouse(MouseEvent))
		return TRUE;

	if (MouseEvent->dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED && MouseEvent->dwEventFlags!=MOUSE_MOVED)
	{
		ProcessKey(KEY_ENTER);
		return TRUE;
	}

	int prevMsX = MsX , prevMsY = MsY;
	MsX=MouseEvent->dwMousePosition.X;
	MsY=MouseEvent->dwMousePosition.Y;
	bool simple_move = (IntKeyState.MouseEventFlags == MOUSE_MOVED);


	if ((MsX<X1 || MsY<Y1 || MsX>X2 || MsY>Y2) && IntKeyState.MouseEventFlags != MOUSE_MOVED)
	{
		if (Flags.Check(HELPMODE_CLICKOUTSIDE))
		{
			// Вываливаем если предыдущий эвент не был двойным кликом
			if (IntKeyState.PreMouseEventFlags != DOUBLE_CLICK)
				ProcessKey(KEY_ESC);
		}

		if (MouseEvent->dwButtonState)
			Flags.Set(HELPMODE_CLICKOUTSIDE);

		return TRUE;
	}

	if (IntKeyState.MouseX==X2 && (MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED))
	{
		int ScrollY=Y1+FixSize+1;
		int Height=Y2-Y1-FixSize-1;

		if (IntKeyState.MouseY==ScrollY)
		{
			while (IsMouseButtonPressed())
				ProcessKey(KEY_UP);

			return TRUE;
		}

		if (IntKeyState.MouseY==ScrollY+Height-1)
		{
			while (IsMouseButtonPressed())
				ProcessKey(KEY_DOWN);

			return TRUE;
		}
		simple_move = false;
	}

	/* $ 15.03.2002 DJ
	   обработаем щелчок в середине скроллбара
	*/
	if (IntKeyState.MouseX == X2)
	{
		int ScrollY=Y1+FixSize+1;
		int Height=Y2-Y1-FixSize-1;

		if (StrCount > Height)
		{
			while (IsMouseButtonPressed())
			{
				if (IntKeyState.MouseY > ScrollY && IntKeyState.MouseY < ScrollY+Height+1)
				{
					StackData.CurX=StackData.CurY=0;
					StackData.TopStr=(IntKeyState.MouseY-ScrollY-1) * (StrCount-FixCount-Height+1) / (Height-2);
					FastShow();
				}
			}

			return TRUE;
		}
		simple_move = false;
	}

	// DoubliClock - свернуть/развернуть хелп.
	if (MouseEvent->dwEventFlags==DOUBLE_CLICK &&
	        (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
	        MouseEvent->dwMousePosition.Y<Y1+1+FixSize)
	{
		ProcessKey(KEY_F5);
		return TRUE;
	}

	if (MouseEvent->dwMousePosition.Y<Y1+1+FixSize)
	{
		while (IsMouseButtonPressed() && IntKeyState.MouseY<Y1+1+FixSize)
			ProcessKey(KEY_UP);

		return TRUE;
	}

	if (MouseEvent->dwMousePosition.Y>=Y2)
	{
		while (IsMouseButtonPressed() && IntKeyState.MouseY>=Y2)
			ProcessKey(KEY_DOWN);

		return TRUE;
	}

	/* $ 26.11.2001 VVM
	  + Запомнить нажатие клавиши мышки и только в этом случае реагировать при отпускании */
	if (!MouseEvent->dwEventFlags
	 && (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED)))
	{
		BeforeMouseDownX = StackData.CurX;
		BeforeMouseDownY = StackData.CurY;
		StackData.CurX = MouseDownX = MsX-X1-1;
		StackData.CurY = MouseDownY = MsY-Y1-1-FixSize;
		MouseDown = TRUE;
		simple_move = false;
	}

	if (!MouseEvent->dwEventFlags
	 && !(MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED))
	 && MouseDown)
	{
		simple_move = false;
		MouseDown = FALSE;
		if (!StackData.strSelTopic.IsEmpty())
		{
			if (StackData.CurX == MouseDownX && StackData.CurY == MouseDownY)
				ProcessKey(KEY_ENTER);
		}
		else
		{
			if (StackData.CurX==MouseDownX && StackData.CurY==MouseDownY)
			{
				StackData.CurX = BeforeMouseDownX;
				StackData.CurY = BeforeMouseDownY;
			}
		}
	}

   if (simple_move && (prevMsX != MsX || prevMsY != MsY))
	{
		string strTopic;
		if (GetTopic(MsX, MsY, strTopic))
		{
			//if (strTopic != StackData.strSelTopic)
			{
				StackData.CurX = MsX-X1-1;
				StackData.CurY = MsY-Y1-1-FixSize;
			}
		}
	}

	FastShow();
	Sleep(1);
	return TRUE;
}


int Help::IsReferencePresent()
{
	CorrectPosition();
	int StrPos=FixCount+StackData.TopStr+StackData.CurY;

	if (StrPos >= StrCount)
	{
		return FALSE;
	}

	const HelpRecord *rec=GetHelpItem(StrPos);
	wchar_t *OutStr=rec?rec->HelpStr:nullptr;
	return (OutStr  && wcschr(OutStr,L'@')  && wcschr(OutStr,L'~') );
}

const HelpRecord* Help::GetHelpItem(int Pos)
{
	if ((unsigned int)Pos < HelpList.getSize())
		return HelpList.getItem(Pos);
	return nullptr;
}

void Help::MoveToReference(int Forward,int CurScreen)
{
	int StartSelection=!StackData.strSelTopic.IsEmpty();
	int SaveCurX=StackData.CurX;
	int SaveCurY=StackData.CurY;
	int SaveTopStr=StackData.TopStr;
	StackData.strSelTopic.Clear();
	Lock();

	if (!ErrorHelp) while (StackData.strSelTopic.IsEmpty())
		{
			BOOL ReferencePresent=IsReferencePresent();

			if (Forward)
			{
				if (!StackData.CurX && !ReferencePresent)
					StackData.CurX=X2-X1-2;

				if (++StackData.CurX >= X2-X1-2)
				{
					StartSelection=0;
					StackData.CurX=0;
					StackData.CurY++;

					if (StackData.TopStr+StackData.CurY>=StrCount-FixCount ||
					        (CurScreen && StackData.CurY>Y2-Y1-2-FixSize))
						break;
				}
			}
			else
			{
				if (StackData.CurX==X2-X1-2 && !ReferencePresent)
					StackData.CurX=0;

				if (--StackData.CurX < 0)
				{
					StartSelection=0;
					StackData.CurX=X2-X1-2;
					StackData.CurY--;

					if (StackData.TopStr+StackData.CurY<0 ||
					        (CurScreen && StackData.CurY<0))
						break;
				}
			}

			FastShow();

			if (StackData.strSelTopic.IsEmpty())
				StartSelection=0;
			else
			{
				// небольшая заплата, артефакты есть но уже меньше :-)
				if (ReferencePresent && CurScreen)
					StartSelection=0;

				if (StartSelection)
					StackData.strSelTopic.Clear();
			}
		}

	Unlock();

	if (StackData.strSelTopic.IsEmpty())
	{
		StackData.CurX=SaveCurX;
		StackData.CurY=SaveCurY;
		StackData.TopStr=SaveTopStr;
	}

	FastShow();
}


static int __cdecl CmpItems(const HelpRecord **el1, const HelpRecord **el2)
{
	if (el1==el2)
		return 0;

	int result=StrCmpI((**el1).HelpStr,(**el2).HelpStr);
	if (!result)
		return 0;
	else if (result < 0)
		return -1;
	else
		return 1;
}

void Help::ReadDocumentsHelp(int TypeIndex)
{
	HelpList.Free();

	strCurPluginContents.Clear();
	StrCount=0;
	FixCount=1;
	FixSize=2;
	StackData.TopStr=0;
	TopicFound=TRUE;
	StackData.CurX=StackData.CurY=0;
	strCtrlColorChar.Clear();
	const wchar_t *PtrTitle=0, *ContentsName=0;
	string strPath, strFullFileName;

	switch (TypeIndex)
	{
		case HIDX_PLUGINS:
			PtrTitle=MSG(MPluginsHelpTitle);
			ContentsName=L"PluginContents";
			break;
	}

	AddTitle(PtrTitle);
	/* TODO:
	   1. Поиск (для "документов") не только в каталоге Documets, но
	      и в плагинах
	*/
	int OldStrCount=StrCount;

	switch (TypeIndex)
	{
		case HIDX_PLUGINS:
		{
			for (size_t I=0; I<CtrlObject->Plugins->GetPluginsCount(); I++)
			{
				strPath = CtrlObject->Plugins->GetPlugin(I)->GetModuleName();
				CutToSlash(strPath);
				UINT nCodePage = CP_OEMCP;
				FILE *HelpFile=OpenLangFile(strPath,HelpFileMask,Opt.strHelpLanguage,strFullFileName, nCodePage);

				if (HelpFile)
				{
					string strEntryName, strHelpLine, strSecondParam;

					if (GetLangParam(HelpFile,ContentsName,&strEntryName,&strSecondParam, nCodePage))
					{
						if (!strSecondParam.IsEmpty())
							strHelpLine.Format(L"   ~%s,%s~@" HelpFormatLink L"@", strEntryName.CPtr(), strSecondParam.CPtr(), strPath.CPtr(),HelpContents);
						else
							strHelpLine.Format(L"   ~%s~@" HelpFormatLink L"@",strEntryName.CPtr(), strPath.CPtr(),HelpContents);

						AddLine(strHelpLine);
					}

					fclose(HelpFile);
				}
			}

			break;
		}
	}

	// сортируем по алфавиту
	HelpList.Sort(CmpItems,OldStrCount);

	// $ 26.06.2000 IS - Устранение глюка с хелпом по f1, shift+f2, end (решение предложил IG)
	AddLine(L"");
}

// Формирование топика с учетом разных факторов
string &Help::MkTopic(Plugin* pPlugin,const wchar_t *HelpTopic,string &strTopic)
{
	strTopic.Clear();

	if (HelpTopic && *HelpTopic)
	{
		if (*HelpTopic==L':')
		{
			strTopic = (HelpTopic+1);
		}
		else
		{
			if (pPlugin && *HelpTopic!=HelpBeginLink)
			{
				strTopic.Format(HelpFormatLinkModule, pPlugin->GetModuleName().CPtr(), HelpTopic);
			}
			else
			{
				strTopic = HelpTopic;
			}

			if (strTopic.At(0)==HelpBeginLink)
			{
				wchar_t *Ptr, *Ptr2;
				wchar_t *lpwszTopic = strTopic.GetBuffer(strTopic.GetLength()*2); //BUGBUG

				if (!(Ptr=wcschr(lpwszTopic,HelpEndLink)))
				{
					*lpwszTopic=0;
				}
				else
				{
					if (!Ptr[1]) // Вона как поперло то...
						wcscat(lpwszTopic,HelpContents); // ... значит покажем основную тему. //BUGBUG

					/* А вот теперь разгребем...
					   Формат может быть :
					     "<FullPath>Topic" или "<FullModuleName>Topic"
					   Для случая "FullPath" путь ДОЛЖЕН заканчиваться СЛЕШЕМ!
					   Т.о. мы отличим ЧТО ЭТО - имя модуля или путь!
					*/
					Ptr2=Ptr-1;

					if (!IsSlash(*Ptr2)) // Это имя модуля?
					{
						// значит удалим это чертово имя :-)
						if (!(Ptr2=const_cast<wchar_t*>(LastSlash(lpwszTopic)))) // ВО! Фигня какая-то :-(
							*lpwszTopic=0;
					}

					if (*lpwszTopic)
					{
						/* $ 21.08.2001 KM
						  - Неверно создавался топик с учётом нового правила,
						    в котором путь для топика должен заканчиваться "/".
						*/
						wmemmove(Ptr2+1,Ptr,StrLength(Ptr)+1); //???
						// А вот ЗДЕСЬ теперь все по правилам Help API!
					}
				}

				strTopic.ReleaseBuffer();
			}
		}
	}

	return strTopic;
}

void Help::SetScreenPosition()
{
	if (Opt.FullScreenHelp)
	{
		HelpKeyBar.Hide();
		SetPosition(0,0,ScrX,ScrY);
	}
	else
	{
		SetPosition(4,2,ScrX-4,ScrY-2);
	}

	Show();
}

void Help::InitKeyBar()
{
	HelpKeyBar.SetAllGroup(KBL_MAIN, MHelpF1, 12);
	HelpKeyBar.SetAllGroup(KBL_SHIFT, MHelpShiftF1, 12);
	HelpKeyBar.SetAllGroup(KBL_ALT, MHelpAltF1, 12);
	HelpKeyBar.SetAllGroup(KBL_CTRL, MHelpCtrlF1, 12);
	HelpKeyBar.SetAllGroup(KBL_CTRLSHIFT, MHelpCtrlShiftF1, 12);
	HelpKeyBar.SetAllGroup(KBL_CTRLALT, MHelpCtrlAltF1, 12);
	HelpKeyBar.SetAllGroup(KBL_ALTSHIFT, MHelpAltShiftF1, 12);
	HelpKeyBar.SetAllGroup(KBL_CTRLALTSHIFT, MHelpCtrlAltShiftF1, 12);
	// Уберем лишнее с глаз долой
	HelpKeyBar.Change(KBL_SHIFT,L"",3-1);
	HelpKeyBar.Change(KBL_MAIN,L"",7-1);
	HelpKeyBar.Change(KBL_SHIFT,L"",7-1);
	HelpKeyBar.ReadRegGroup(L"Help",Opt.strLanguage);
	HelpKeyBar.SetAllRegGroup();
	SetKeyBar(&HelpKeyBar);
}

/* $ 25.08.2000 SVS
   Запуск URL-ссылок... ;-)
   Это ведь так просто... ась?
   Вернет:
     0 - это не URL ссылка (не похожа)
     1 - CreateProcess вернул FALSE
     2 - Все Ок

   Параметры (например):
     Protocol="mailto"
     URLPath ="mailto:vskirdin@mail.ru?Subject=Reversi"
*/
static int RunURL(const string& Protocol, wchar_t *URLPath)
{
	int EditCode=0;

	if (URLPath && *URLPath && (Opt.HelpURLRules&0xFF))
	{
		string strType;

		if (GetShellType(Protocol,strType,AT_URLPROTOCOL))
		{
			strType+=L"\\shell\\open\\command";
			HKEY hKey;

			if (RegOpenKeyEx(HKEY_CLASSES_ROOT,strType,0,KEY_READ,&hKey) == ERROR_SUCCESS)
			{
				string strAction;
				int Disposition=RegQueryStringValue(hKey, L"", strAction, L"");
				RegCloseKey(hKey);
				apiExpandEnvironmentStrings(strAction, strAction);

				if (Disposition == ERROR_SUCCESS)
				{
					// удалим два идущих в подряд ~~
					wchar_t *Ptr=URLPath;

					while (*Ptr && (Ptr=wcsstr(Ptr,L"~~")) )
					{
						wmemmove(Ptr,Ptr+1,StrLength(Ptr+1)+1);
						Ptr++;
					}

					// удалим два идущих в подряд ##
					Ptr=URLPath;

					while (*Ptr && (Ptr=wcsstr(Ptr,L"##")) )
					{
						wmemmove(Ptr,Ptr+1,StrLength(Ptr+1)+1);
						++Ptr;
					}

					Disposition=0;

					if (Opt.HelpURLRules == 2 || Opt.HelpURLRules == 2+256)
					{
						Disposition=Message(MSG_WARNING,2,MSG(MHelpTitle),
						                    MSG(MHelpActivatorURL),
						                    strAction,
						                    MSG(MHelpActivatorFormat),
						                    URLPath,
						                    L"\x01",
						                    MSG(MHelpActivatorQ),
						                    MSG(MYes),MSG(MNo));
					}

					EditCode=2; // Все Ok!

					if (!Disposition)
					{
						/*
						СЮДЫ НУЖНО ВПИНДЮЛИТЬ МЕНЮХУ С ВОЗМОЖНОСТЬЮ ВЫБОРА
						ТОГО ИЛИ ИНОГО АКТИВАТОРА - ИХ МОЖЕТ БЫТЬ НЕСКОЛЬКО!!!!!
						*/
						string strCurDir;
						apiGetCurrentDirectory(strCurDir);

						if (Opt.HelpURLRules < 256) // SHELLEXECUTEEX_METHOD
						{
#if 0
							SHELLEXECUTEINFO sei={sizeof(sei)};
							sei.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_DDEWAIT;
							sei.lpFile=RemoveExternalSpaces(Buf);
							sei.nShow=SW_SHOWNORMAL;
							seInfo.lpDirectory=strCurDir;

							if (ShellExecuteEx(&sei))
								EditCode=1;

#else
							strAction=URLPath;
							EditCode=ShellExecute(0, 0, RemoveExternalSpaces(strAction), 0, strCurDir, SW_SHOWNORMAL)?1:2;
#endif
						}
						else
						{
							STARTUPINFO si={sizeof(si)};
							PROCESS_INFORMATION pi={};

							if (ReplaceStrings(strAction, L"%1", URLPath, 1) == 0) //if %1 not found
							{
								strAction += L" ";
								strAction += URLPath;
							}

							if (!CreateProcess(nullptr,const_cast<wchar_t*>(strAction.CPtr()),nullptr,nullptr,TRUE,0,nullptr,strCurDir,&si,&pi))
							{
								EditCode=1;
							}
							else
							{
								CloseHandle(pi.hThread);
								CloseHandle(pi.hProcess);
							}
						}
					}
				}
			}
		}
	}

	return EditCode;
}

void Help::OnChangeFocus(int Focus)
{
	if (Focus)
	{
		DisplayObject();
	}
}

void Help::ResizeConsole()
{
	int OldIsNewTopic=IsNewTopic;
	BOOL ErrCannotOpenHelp=ScreenObject::Flags.Check(FHELPOBJ_ERRCANNOTOPENHELP);
	ScreenObject::Flags.Set(FHELPOBJ_ERRCANNOTOPENHELP);
	IsNewTopic=FALSE;
	delete TopScreen;
	TopScreen=nullptr;
	Hide();

	if (Opt.FullScreenHelp)
	{
		HelpKeyBar.Hide();
		SetPosition(0,0,ScrX,ScrY);
	}
	else
		SetPosition(4,2,ScrX-4,ScrY-2);

	ReadHelp(StackData.strHelpMask);
	ErrorHelp=FALSE;
	//StackData.CurY--; // ЭТО ЕСМЬ КОСТЫЛЬ (пусть пока будет так!)
	StackData.CurX--;
	MoveToReference(1,1);
	IsNewTopic=OldIsNewTopic;
	ScreenObject::Flags.Change(FHELPOBJ_ERRCANNOTOPENHELP,ErrCannotOpenHelp);
	FrameManager->ImmediateHide();
	FrameManager->RefreshFrame();
}

int Help::FastHide()
{
	return Opt.AllCtrlAltShiftRule & CASR_HELP;
}


int Help::GetTypeAndName(string &strType, string &strName)
{
	strType = MSG(MHelpType);
	strName = strFullHelpPathName;
	return(MODALTYPE_HELP);
}

/* ------------------------------------------------------------------ */
void CallBackStack::ClearStack()
{
	while (!isEmpty())
		Pop();
}

int CallBackStack::Pop(StackHelpData *Dest)
{
	if (!isEmpty())
	{
		ListNode *oldTop = topOfStack;
		topOfStack = topOfStack->Next;

		if (Dest)
		{
			Dest->strHelpTopic = oldTop->strHelpTopic;
			Dest->strHelpPath = oldTop->strHelpPath;
			Dest->strSelTopic = oldTop->strSelTopic;
			Dest->strHelpMask = oldTop->strHelpMask;
			Dest->Flags=oldTop->Flags;
			Dest->TopStr=oldTop->TopStr;
			Dest->CurX=oldTop->CurX;
			Dest->CurY=oldTop->CurY;
		}

		delete oldTop;
		return TRUE;
	}

	return FALSE;
}

void CallBackStack::Push(const StackHelpData *Data)
{
	topOfStack=new ListNode(Data,topOfStack);
}

void CallBackStack::PrintStack(const wchar_t *Title)
{
#if defined(SYSLOG)
	int I=0;
	ListNode *Ptr = topOfStack;
	SysLog(L"Return Stack (%s)",Title);
	SysLog(1);

	while (Ptr)
	{
		SysLog(L"%03d HelpTopic='%s' HelpPath='%s' HelpMask='%s'",I++,Ptr->strHelpTopic.CPtr(),Ptr->strHelpPath.CPtr(),Ptr->strHelpMask.CPtr());
		Ptr=Ptr->Next;
	}

	SysLog(-1);
#endif
}
