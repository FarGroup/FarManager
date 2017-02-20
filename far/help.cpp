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
#include "farcolor.hpp"
#include "manager.hpp"
#include "ctrlobj.hpp"
#include "macroopcode.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "config.hpp"
#include "execute.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "exitcode.hpp"
#include "filestr.hpp"
#include "colormix.hpp"
#include "stddlg.hpp"
#include "plugins.hpp"
#include "DlgGuid.hpp"
#include "RegExp.hpp"
#include "language.hpp"
#include "keybar.hpp"
#include "local.hpp"
#include "cvtname.hpp"

static constexpr wchar_t FoundContents[]=L"__FoundContents__";
static constexpr wchar_t PluginContents[]=L"__PluginContents__";
static constexpr wchar_t HelpOnHelpTopic[]=L":Help";
static constexpr wchar_t HelpContents[]=L"Contents";

static constexpr wchar_t HelpBeginLink = L'<';
static constexpr wchar_t HelpEndLink = L'>';

enum HELPDOCUMENTSHELPTYPE
{
	HIDX_PLUGINS,                 // Индекс плагинов
	HIDX_DOCUMS,                  // Индекс документов
};

enum
{
	FHELPOBJ_ERRCANNOTOPENHELP  = 0x80000000,
};

class HelpRecord
{
public:
	NONCOPYABLE(HelpRecord);
	TRIVIALLY_MOVABLE(HelpRecord);

	string HelpStr;

	HelpRecord(const string& HStr): HelpStr(HStr) {}

	bool operator ==(const HelpRecord& rhs) const
	{
		return !StrCmpI(HelpStr, rhs.HelpStr);
	}

	bool operator <(const HelpRecord& rhs) const
	{
		return StrCmpI(HelpStr, rhs.HelpStr) < 0;
	}
};

static int RunURL(const string& Protocol, const string& URLPath);

static constexpr wchar_t HelpFormatLink[] = L"<{0}\\>{1}";
static constexpr wchar_t HelpFormatLinkModule[] = L"<{0}>{1}";


struct Help::StackHelpData
{
	TRIVIALLY_COPYABLE(StackHelpData);
	TRIVIALLY_MOVABLE(StackHelpData);

	StackHelpData():
		Flags(),
		TopStr(),
		CurX(),
		CurY()
	{}

	string strHelpMask;           // значение маски
	string strHelpPath;           // путь к хелпам
	string strHelpTopic;          // текущий топик
	string strSelTopic;           // выделенный топик (???)

	UINT64 Flags;                 // флаги
	int   TopStr;                 // номер верхней видимой строки темы
	int   CurX, CurY;             // координаты (???)
};

string Help::MakeLink(const string& path, const string& topic)
{
	return string(L"<") + path + L"\\>" + topic;
}

Help::Help(private_tag):
	StackData(std::make_unique<StackHelpData>()),
	FixCount(0),
	MouseDownX(0),
	MouseDownY(0),
	BeforeMouseDownX(0),
	BeforeMouseDownY(0),
	MsX(-1),
	MsY(-1),
	CurColor(colors::PaletteColorToFarColor(COL_HELPTEXT)),
	CtrlTabSize(0),
	LastStartPos(0),
	StartPos(0),
	MouseDown(false),
	IsNewTopic(true),
	m_TopicFound(false),
	ErrorHelp(true),
	LastSearchCase(Global->GlobalSearchCase),
	LastSearchWholeWords(Global->GlobalSearchWholeWords),
	LastSearchRegexp(Global->Opt->HelpSearchRegexp)
{
}

help_ptr Help::create(const string& Topic, const wchar_t *Mask, UINT64 Flags)
{
	auto HelpPtr = std::make_shared<Help>(private_tag());
	HelpPtr->init(Topic, Mask, Flags);
	return HelpPtr;
}

void Help::init(const string& Topic, const wchar_t *Mask, UINT64 Flags)
{
	m_windowKeyBar = std::make_unique<KeyBar>(shared_from_this());

	m_CanLoseFocus=FALSE;
	m_KeyBarVisible=TRUE;
	SetRestoreScreenMode(true);

	StackData->Flags=Flags;
	StackData->strHelpMask = NullToEmpty(Mask); // сохраним маску файла
	StackData->strHelpTopic = Topic;

	if (Global->Opt->FullScreenHelp)
		SetPosition(0,0,ScrX,ScrY);
	else
		SetPosition(4,2,ScrX-4,ScrY-2);

	if (!ReadHelp(StackData->strHelpMask) && (Flags&FHELP_USECONTENTS))
	{
		StackData->strHelpTopic = Topic;

		if (StackData->strHelpTopic.front() == HelpBeginLink)
		{
			size_t pos = StackData->strHelpTopic.rfind(HelpEndLink);

			if (pos != string::npos)
				StackData->strHelpTopic.resize(pos + 1);

			StackData->strHelpTopic += HelpContents;
		}

		StackData->strHelpPath.clear();
		ReadHelp(StackData->strHelpMask);
	}

	if (!HelpList.empty())
	{
		m_Flags.Clear(FHELPOBJ_ERRCANNOTOPENHELP);
		InitKeyBar();
		SetMacroMode(MACROAREA_HELP);
		MoveToReference(1,1);
		Global->WindowManager->ExecuteWindow(shared_from_this()); //OT
		Global->WindowManager->ExecuteModal(shared_from_this()); //OT
	}
	else
	{
		ErrorHelp = true;

		if (!(Flags&FHELP_NOSHOWERROR))
		{
			if (!m_Flags.Check(FHELPOBJ_ERRCANNOTOPENHELP))
			{
				Message(MSG_WARNING, 1, MSG(lng::MHelpTitle), MSG(lng::MHelpTopicNotFound), StackData->strHelpTopic.data(), MSG(lng::MOk));
			}

			m_Flags.Clear(FHELPOBJ_ERRCANNOTOPENHELP);
		}
	}
}

Help::~Help()
{
}

int Help::ReadHelp(const string& Mask)
{
	string strPath;

	if (StackData->strHelpTopic.front() == HelpBeginLink)
	{
		strPath = StackData->strHelpTopic.substr(1);
		size_t pos = strPath.find(HelpEndLink);

		if (pos == string::npos)
			return FALSE;

		StackData->strHelpTopic = strPath.substr(pos + 1);
		strPath.resize(pos);
		DeleteEndSlash(strPath);
		AddEndSlash(strPath);
		StackData->strHelpPath = strPath;
	}
	else
	{
		strPath = !StackData->strHelpPath.empty() ? StackData->strHelpPath : Global->g_strFarPath;
	}

	if (StackData->strHelpTopic == PluginContents)
	{
		strFullHelpPathName.clear();
		ReadDocumentsHelp(HIDX_PLUGINS);
		return TRUE;
	}

	uintptr_t nCodePage = CP_OEMCP;
	os::fs::file HelpFile;

	if (!OpenLangFile(HelpFile, strPath,(Mask.empty()?Global->HelpFileMask:Mask),Global->Opt->strHelpLanguage,strFullHelpPathName, nCodePage))
	{
		ErrorHelp = true;

		if (!m_Flags.Check(FHELPOBJ_ERRCANNOTOPENHELP))
		{
			m_Flags.Set(FHELPOBJ_ERRCANNOTOPENHELP);

			if (!(StackData->Flags&FHELP_NOSHOWERROR))
			{
				Message(MSG_WARNING,1,MSG(lng::MHelpTitle),MSG(lng::MCannotOpenHelp),Mask.data(),MSG(lng::MOk));
			}
		}

		return FALSE;
	}

	string strReadStr;

	if (GetOptionsParam(HelpFile,L"TabSize",strReadStr, nCodePage))
	{
		CtrlTabSize = std::stoi(strReadStr);
	}

	if (CtrlTabSize < 0 || CtrlTabSize > 16)
		CtrlTabSize=Global->Opt->HelpTabSize;

	if (GetOptionsParam(HelpFile,L"CtrlColorChar",strReadStr, nCodePage))
		strCtrlColorChar = strReadStr;
	else
		strCtrlColorChar.clear();

	if (GetOptionsParam(HelpFile,L"CtrlStartPosChar",strReadStr, nCodePage))
		strCtrlStartPosChar = strReadStr;
	else
		strCtrlStartPosChar.clear();

	/* $ 29.11.2001 DJ
	   запомним, чего там написано в PluginContents
	*/
	if (!GetLangParam(HelpFile,L"PluginContents",&strCurPluginContents, nullptr, nCodePage))
		strCurPluginContents.clear();

	string strTabSpace(CtrlTabSize, L' ');

	HelpList.clear();

	if (StackData->strHelpTopic == FoundContents)
	{
		Search(HelpFile,nCodePage);
		return TRUE;
	}

	FixCount=0;
	m_TopicFound = false;
	bool RepeatLastLine = false;
	bool BreakProcess = false;
	bool Formatting = true;
	int NearTopicFound = 0;
	wchar_t PrevSymbol=0;
	bool drawline = false;
	wchar_t DrawLineChar = 0;
	const int MaxLength = CanvasWidth();

	StartPos = (DWORD)-1;
	LastStartPos = (DWORD)-1;
	int RealMaxLength;
	bool MacroProcess=false;
	int MI=0;
	string strMacroArea;

	GetFileString GetStr(HelpFile, nCodePage);
	const size_t StartSizeKeyName = 20;
	size_t SizeKeyName = StartSizeKeyName;
	string strSplitLine;

	for (;;)
	{
		if (StartPos != (DWORD)-1)
			RealMaxLength = MaxLength-StartPos;
		else
			RealMaxLength = MaxLength;

		if (!MacroProcess && !RepeatLastLine && !BreakProcess)
		{
			if (!GetStr.GetString(strReadStr))
			{
				if (StringLen(strSplitLine)<MaxLength)
				{
					if (!strSplitLine.empty())
						AddLine(strSplitLine);
				}
				else
				{
					strReadStr.clear();
					RepeatLastLine = true;
					continue;
				}

				break;
			}
		}

		if (!RepeatLastLine && MacroProcess)
		{
			string strDescription;
			string strKeyName;

			if (!Global->CtrlObject->Macro.GetMacroKeyInfo(strMacroArea,MI,strKeyName,strDescription))
			{
				MacroProcess=false;
				MI=0;
				continue;
			}

			if (!strKeyName.empty() && strKeyName[0] == L'~')
			{
				MI++;
				continue;
			}

			strReadStr.clear();
			if (strKeyName.size() > SizeKeyName)
			{
				FarFormatText(strKeyName, SizeKeyName, strKeyName, L"\n", 0);

				size_t nl;
				while ((nl = strKeyName.find(L'\n')) != string::npos)
				{
					string keys = strKeyName.substr(0, nl);
					strKeyName.erase(0, nl+1);

					ReplaceStrings(keys, L"~", L"~~");
					ReplaceStrings(keys, L"#", L"##");
					ReplaceStrings(keys, L"@", L"@@");

					append(strReadStr, L" #"s, keys, L"#\n"s);
				}

				if (strKeyName.size() > SizeKeyName)
					strKeyName.resize(SizeKeyName); // cut key names
			}

			ReplaceStrings(strKeyName, L"~", L"~~");
			ReplaceStrings(strKeyName, L"#", L"##");
			ReplaceStrings(strKeyName, L"@", L"@@");

			if (contains(strKeyName, L'~')) // коррекция размера
				SizeKeyName++;

			append(strReadStr, L" #"s, fit_to_left(strKeyName, SizeKeyName), L"# "s);

			if (!strDescription.empty())
			{
				ReplaceStrings(strDescription, L"#", L"##");
				ReplaceStrings(strDescription, L"~", L"~~");
				ReplaceStrings(strDescription, L"@", L"@@");
				strReadStr += strCtrlStartPosChar;
				strReadStr += strDescription;
			}

			MI++;
		}

		RepeatLastLine = false;

		{
			size_t PosTab;
			while ((PosTab = strReadStr.find(L'\t')) != string::npos)
			{
				strReadStr[PosTab] = L' ';

				if (CtrlTabSize > 1) // заменим табулятор по всем правилам
					strReadStr.insert(PosTab, strTabSpace.data(), CtrlTabSize - (PosTab % CtrlTabSize));
			}
		}

		RemoveTrailingSpaces(strReadStr);

		if (!strCtrlStartPosChar.empty())
		{
			size_t pos = strReadStr.find(strCtrlStartPosChar);
			if (pos != string::npos)
			{
				size_t p1 = strReadStr.rfind(L'\n') + 1;
				if (p1 > pos)
					p1 = 0;
				LastStartPos = StringLen(strReadStr.substr(p1, pos-p1));
				strReadStr.erase(pos, strCtrlStartPosChar.size());
			}
		}

		if (m_TopicFound)
		{
			HighlightsCorrection(strReadStr);
		}

		if (!strReadStr.empty() && strReadStr[0]==L'@' && !BreakProcess)
		{
			if (m_TopicFound)
			{
				if (strReadStr == L"@+")
				{
					Formatting = true;
					PrevSymbol=0;
					continue;
				}

				if (strReadStr == L"@-")
				{
					Formatting = false;
					PrevSymbol=0;
					continue;
				}

				// @=[Symbol]
				// '@=' - одинарная горизонтальная линия на всю ширину окна хелпа
				// '@=*' - она же из символов '*'
				if (strReadStr[1] == L'=')
				{
					DrawLineChar = strReadStr[2]; // TODO: hex! ==> \xHHHH
					drawline = true;
					PrevSymbol=0;
					goto m1;
				}

				if (!strSplitLine.empty())
				{
					BreakProcess = true;
					strReadStr.clear();
					PrevSymbol=0;
					goto m1;
				}

				break;
			}
			else if (!StrCmpI(strReadStr.data() + 1, StackData->strHelpTopic.data()))
			{
				m_TopicFound = true;
				NearTopicFound=1;
			}
			else // redirection @SearchTopic=RealTopic
			{
				size_t n1 = StackData->strHelpTopic.size();
				size_t n2 = strReadStr.size();
				if (1 + n1 + 1 < n2 && !StrCmpNI(strReadStr.data() + 1, StackData->strHelpTopic.data(), n1) && strReadStr[1 + n1] == L'=')
				{
					StackData->strHelpTopic = strReadStr.substr(1 + n1 + 1);
					continue;
				}
			}
		}
		else
		{
m1:
			if (strReadStr.empty() && BreakProcess && strSplitLine.empty())
				break;

			if (m_TopicFound)
			{
				const auto MacroTag = L"<!Macro:"_sl;
				if (!StrCmpNI(strReadStr.data(), MacroTag.data(), MacroTag.size()) && Global->CtrlObject)
				{
					const auto PosTab = strReadStr.find(L'>');
					if (PosTab != string::npos && strReadStr[PosTab - 1] != L'!')
						continue;

					strMacroArea=strReadStr.substr(8,PosTab-1-8); //???
					MacroProcess=true;
					MI=0;
					SizeKeyName = StartSizeKeyName;
					string strDescription,strKeyName;
					while (Global->CtrlObject->Macro.GetMacroKeyInfo(strMacroArea,MI,strKeyName,strDescription))
					{
						SizeKeyName = Clamp(strKeyName.size(), SizeKeyName, static_cast<size_t>(MaxLength) / 2);
						MI++;
					}
					MI=0;
					continue;
				}

				if (!((!strReadStr.empty() && strReadStr[0]==L'$') && NearTopicFound && (PrevSymbol == L'$' || PrevSymbol == L'@')))
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

				if ((!strReadStr.empty() && strReadStr[0]==L'$') && NearTopicFound && (PrevSymbol == L'$' || PrevSymbol == L'@'))
				{
					AddLine(strReadStr.data()+1);
					FixCount++;
				}
				else
				{
					NearTopicFound=0;

					if (strReadStr.empty() || !Formatting)
					{
						if (!strSplitLine.empty())
						{
							if (StringLen(strSplitLine)<RealMaxLength)
							{
								AddLine(strSplitLine);
								strSplitLine.clear();

								if (StringLen(strReadStr)<RealMaxLength)
								{
									AddLine(strReadStr);
									LastStartPos = (DWORD)-1;
									StartPos = (DWORD)-1;
									continue;
								}
							}
							else
								RepeatLastLine = true;
						}
						else if (!strReadStr.empty())
						{
							if (StringLen(strReadStr)<RealMaxLength)
							{
								AddLine(strReadStr);
								continue;
							}
						}
						else if (strReadStr.empty() && strSplitLine.empty())
						{
							AddLine(L"");
							continue;
						}
					}

					if (!strReadStr.empty() && IsSpace(strReadStr[0]) && Formatting)
					{
						if (StringLen(strSplitLine)<RealMaxLength)
						{
							if (!strSplitLine.empty())
							{
								AddLine(strSplitLine);
								StartPos = (DWORD)-1;
							}

							for (size_t nl = strReadStr.find(L'\n'); nl != string::npos; )
							{
								AddLine(strReadStr.substr(0, nl));
								strReadStr.erase(0, nl+1);
								nl = strReadStr.find(L'\n');
							}

							strSplitLine = strReadStr;
							strReadStr.clear();
							continue;
						}
						else
							RepeatLastLine = true;
					}

					if (drawline)
					{
						drawline = false;
						if (!strSplitLine.empty())
						{
							AddLine(strSplitLine);
							StartPos = (DWORD)-1;
						}
						wchar_t userSeparator[4] = { L' ', (DrawLineChar ? DrawLineChar : BoxSymbols[BS_H1]), L' ', 0 }; // left-center-right
						int Mul = (DrawLineChar == L'@' || DrawLineChar == L'~' || DrawLineChar == L'#' ? 2 : 1); // Double. See Help::OutString
						AddLine(MakeSeparator(CanvasWidth() * Mul - (Mul>>1), 12, userSeparator)); // 12 -> UserSep horiz
						strReadStr.clear();
						strSplitLine.clear();
						continue;
					}

					if (!RepeatLastLine)
					{
						if (!strSplitLine.empty())
							strSplitLine += L" ";

						strSplitLine += strReadStr;
					}

					if (StringLen(strSplitLine)<RealMaxLength)
					{
						if (strReadStr.empty() && BreakProcess)
							goto m1;

						continue;
					}

					int Splitted=0;

					for (int I=(int)strSplitLine.size()-1; I > 0; I--)
					{
						if (I > 0 && strSplitLine[I]==L'~' && strSplitLine[I - 1] == L'~')
						 {
							I--;
							continue;
						}

						if (I > 0 && strSplitLine[I] == L'~' && strSplitLine[I - 1] != L'~')
						{
							do
							{
								I--;
							}
							while (I > 0 && strSplitLine[I] != L'~');

							continue;
						}

						if (strSplitLine[I] == L' ')
						{
							string FirstPart = strSplitLine.substr(0, I);
							if (StringLen(FirstPart) < RealMaxLength)
							{
								AddLine(FirstPart);
								strSplitLine.erase(1, I);
								strSplitLine[0] = L' ';
								HighlightsCorrection(strSplitLine);
								Splitted=TRUE;
								break;
							}
						}
					}

					if (!Splitted)
					{
						AddLine(strSplitLine);
						strSplitLine.clear();
					}
					else
					{
						StartPos = LastStartPos;
					}
				}
			}

			if (BreakProcess)
			{
				if (!strSplitLine.empty())
					goto m1;

				break;
			}
		}

		PrevSymbol = strReadStr.empty() ? L'\0' : strReadStr[0];
	}

	AddLine(L"");
	ErrorHelp = false;

	if (IsNewTopic)
	{
		StackData->CurX = StackData->CurY = 0;
		StackData->TopStr = 0;
	}

	return m_TopicFound;
}

void Help::AddLine(const string& Line)
{
	string strLine;

	if (StartPos != 0xFFFFFFFF)
	{
		DWORD StartPos0=StartPos;
		if (!Line.empty() && Line[0] == L' ')
			StartPos0--;

		if (StartPos0 > 0)
		{
			strLine.assign(StartPos0, L' ');
		}
	}

	strLine += Line;
	HelpList.emplace_back(strLine);
}

void Help::AddTitle(const string& Title)
{
	AddLine(L"^ #" + Title + L"#");
}

void Help::HighlightsCorrection(string &strStr)
{
	if ((std::count(ALL_CONST_RANGE(strStr), L'#') & 1) && strStr.front() != L'$')
		strStr.insert(0, 1, L'#');
}

void Help::DisplayObject()
{
	if (!m_TopicFound)
	{
		if (!ErrorHelp) // если это убрать, то при несуществующей ссылке
		{               // с нынешним манагером попадаем в бесконечный цикл.
			ErrorHelp = true;

			if (!(StackData->Flags&FHELP_NOSHOWERROR))
			{
				Message(MSG_WARNING, 1, MSG(lng::MHelpTitle), MSG(lng::MHelpTopicNotFound), StackData->strHelpTopic.data(), MSG(lng::MOk));
			}

			ProcessKey(Manager::Key(KEY_ALTF1));
		}

		return;
	}

	SetCursorType(false, 10);

	if (StackData->strSelTopic.empty())
		MoveToReference(1,1);

	FastShow();

	if (!Global->Opt->FullScreenHelp)
	{
		m_windowKeyBar->SetPosition(0, ScrY, ScrX, ScrY);

		if (Global->Opt->ShowKeyBar)
			m_windowKeyBar->Show();
	}
	else
	{
		m_windowKeyBar->Hide();
	}

	Shadow();
}

void Help::FastShow()
{
	if (!IsVisible())
		return;

	DrawWindowFrame();

	CorrectPosition();
	StackData->strSelTopic.clear();
	/* $ 01.09.2000 SVS
	   Установим по умолчанию текущий цвет отрисовки...
	   чтобы новая тема начиналась с нормальными атрибутами
	*/
	CurColor = colors::PaletteColorToFarColor(COL_HELPTEXT);

	for (int i=0; i < CanvasHeight(); i++)
	{
		int StrPos;

		if (i<FixCount)
		{
			StrPos=i;
		}
		else if (i==FixCount && FixCount>0)
		{
			GotoXY(m_X1,m_Y1+i+1);
			SetColor(COL_HELPBOX);
			ShowSeparator(ObjWidth(), 1);
			continue;
		}
		else
		{
			StrPos = i + StackData->TopStr;

			if (FixCount>0)
				StrPos--;
		}

		if (StrPos < static_cast<int>(HelpList.size()))
		{
			const wchar_t *OutStr = HelpList[StrPos].HelpStr.data();

			if (*OutStr==L'^')
			{
				OutStr++;
				GotoXY(m_X1 + 1 + std::max(0, (CanvasWidth() - StringLen(OutStr)) / 2), m_Y1 + i + 1);
			}
			else
			{
				GotoXY(m_X1+1,m_Y1+i+1);
			}

			OutString(OutStr);
		}
	}

	SetColor(COL_HELPSCROLLBAR);
	ScrollBarEx(m_X2, m_Y1 + HeaderHeight() + 1, BodyHeight(), StackData->TopStr, HelpList.size() - FixCount);
}

void Help::DrawWindowFrame() const
{
	SetScreen(m_X1,m_Y1,m_X2,m_Y2,L' ',colors::PaletteColorToFarColor(COL_HELPTEXT));
	Box(m_X1,m_Y1,m_X2,m_Y2,colors::PaletteColorToFarColor(COL_HELPBOX),DOUBLE_BOX);
	SetColor(COL_HELPBOXTITLE);
	string strHelpTitleBuf;
	strHelpTitleBuf = MSG(lng::MHelpTitle);
	strHelpTitleBuf += L" - ";

	if (!strCurPluginContents.empty())
		strHelpTitleBuf += strCurPluginContents;
	else
		strHelpTitleBuf += L"FAR";

	TruncStrFromEnd(strHelpTitleBuf, CanvasWidth() - 2);
	GotoXY(m_X1 + (ObjWidth() + (int)strHelpTitleBuf.size() - 2) / 2, m_Y1);
	Text(concat(L' ', strHelpTitleBuf, L' '));
}

static const wchar_t *SkipLink( const wchar_t *Str, string *Name )
{
	for (;;)
	{
		while (*Str && *Str != L'@')
		{
			if (Name)
				Name->push_back(*Str);
			++Str;
		}
		if (*Str)
			++Str;
		if (*Str != L'@')
			break;
		if (Name)
			Name->push_back(*Str);
		++Str;
	}
	return Str;
}

static bool GetHelpColor(const wchar_t* &Str, wchar_t cColor, FarColor &color)
{
	if (!cColor || Str[0] != cColor)
		return false;

	wchar_t wc1 = Str[1];
	if (wc1 == L'-')     // '\-' set default color
	{
		color = colors::PaletteColorToFarColor(COL_HELPTEXT);
		Str += 2;
		return true;
	}

	if (!std::iswxdigit(wc1)) // '\hh' custom color
		return false;
	wchar_t wc2 = Str[2];
	if (!std::iswxdigit(wc2))
		return false;

	if (wc1 > L'9')
		wc1 -= L'A' - 10;
	if (wc2 > L'9')
		wc2 -= L'A' - 10;

	color = colors::ConsoleColorToFarColor(((wc1 & 0x0f) << 4) | (wc2 & 0x0f));
	Str += 3;
	return true;

	// TODO: TrueColor support
}

static bool FastParseLine(const wchar_t *Str, int *pLen, int x0, int realX, string *pTopic, wchar_t cColor)
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
			{
				Str += 2-1;
				continue;
			}
			else if (std::iswxdigit(*Str) && std::iswxdigit(Str[1])) // '\hh' custom color
			{
				Str += 3-1;
				continue;
			}
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
	strTopic.clear();
	if (realY <= m_Y1 || realY >= m_Y2 || realX <= m_X1 || realX >= m_X2)
		return false;

	int y = -1;
	if (realY-m_Y1 <= HeaderHeight())
	{
		if (y != FixCount)
			y = realY - m_Y1 - 1;
	}
	else
		y = realY - m_Y1 - 1 - HeaderHeight() + FixCount + StackData->TopStr;

	if (y < 0 || y >= static_cast<int>(HelpList.size()))
		return false;

	const wchar_t *Str = HelpList[y].HelpStr.data();

	if (!*Str)
		return false;

	int x = m_X1 + 1;
	if (*Str == L'^') // center
	{
		int w = StringLen(++Str);
		x = m_X1 + 1 + std::max(0, (CanvasWidth() - w) / 2);
	}

	return FastParseLine(Str, nullptr, x, realX, &strTopic, strCtrlColorChar.empty()? 0 : strCtrlColorChar[0]);
}

int Help::StringLen(const string& Str)
{
	int len = 0;
	FastParseLine(Str.data(), &len, 0, -1, nullptr, strCtrlColorChar.empty()? 0 : strCtrlColorChar[0]);
	return len;
}

void Help::OutString(const wchar_t *Str)
{
	wchar_t OutStr[512]; //BUGBUG
	const wchar_t *StartTopic=nullptr;
	int OutPos=0,Highlight=0,Topic=0;
	wchar_t cColor = strCtrlColorChar.empty()? 0 : strCtrlColorChar[0];

	while (OutPos<(int)(std::size(OutStr)-10))
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
				int RealCurX = m_X1 + StackData->CurX + 1;
				int RealCurY = m_Y1 + StackData->CurY + HeaderHeight() + 1;
				bool found = WhereY() == RealCurY && RealCurX >= WhereX() && RealCurX < WhereX() + (Str - StartTopic) - 1;

				SetColor(found ? COL_HELPSELECTEDTOPIC : COL_HELPTOPIC);
				if (*Str && Str[1]==L'@')
				{
					Str = SkipLink(Str+2, found ? &StackData->strSelTopic : nullptr);
					Topic = 0;
				}
			}
			else
			{
				SetColor(Highlight ? colors::PaletteColorToFarColor(COL_HELPHIGHLIGHTTEXT) : CurColor);
			}

			/* $ 24.09.2001 VVM
			  ! Обрежем длинные строки при показе. Такое будет только при длинных ссылках... */
			if (StrLength(OutStr) + WhereX() > m_X2)
				OutStr[m_X2 - WhereX()] = 0;

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

	if (WhereX()<m_X2)
	{
		SetColor(CurColor);
		Text(string(m_X2-WhereX(), L' '));
	}
}

void Help::CorrectPosition() const
{
	StackData->CurX = Clamp(StackData->CurX, 0, CanvasWidth() - 1);

	if (StackData->CurY > BodyHeight() - 1)
	{
		StackData->TopStr += StackData->CurY - BodyHeight() - 1;
		StackData->CurY = BodyHeight() - 1;
	}

	if (StackData->CurY<0)
	{
		StackData->TopStr+=StackData->CurY;
		StackData->CurY=0;
	}

	StackData->TopStr = std::max(0, std::min(StackData->TopStr, static_cast<int>(HelpList.size()) - FixCount - BodyHeight()));
}

long long Help::VMProcess(int OpCode,void* vParam, long long iParam)
{
	switch (OpCode)
	{
		case MCODE_V_HELPFILENAME: // Help.FileName
			*(string *)vParam=strFullHelpPathName;     // ???
			break;
		case MCODE_V_HELPTOPIC: // Help.Topic
			*(string *)vParam=StackData->strHelpTopic;  // ???
			break;
		case MCODE_V_HELPSELTOPIC: // Help.SELTopic
			*(string *)vParam=StackData->strSelTopic;   // ???
			break;
		default:
			return 0;
	}

	return 1;
}

int Help::ProcessKey(const Manager::Key& Key)
{
	const auto LocalKey = Key();
	if (StackData->strSelTopic.empty())
		StackData->CurX=StackData->CurY=0;

	switch (LocalKey)
	{
		case KEY_NONE:
		case KEY_IDLE:
		{
			break;
		}
		case KEY_F5:
		{
			Global->Opt->FullScreenHelp=!Global->Opt->FullScreenHelp;
			ResizeConsole();
			Show();
			return TRUE;
		}
		case KEY_ESC:
		case KEY_F10:
		{
			Hide();
			Global->WindowManager->DeleteWindow(shared_from_this());
			SetExitCode(XC_QUIT);
			return TRUE;
		}
		case KEY_HOME:        case KEY_NUMPAD7:
		case KEY_CTRLHOME:    case KEY_CTRLNUMPAD7:
		case KEY_RCTRLHOME:   case KEY_RCTRLNUMPAD7:
		case KEY_CTRLPGUP:    case KEY_CTRLNUMPAD9:
		case KEY_RCTRLPGUP:   case KEY_RCTRLNUMPAD9:
		{
			StackData->CurX=StackData->CurY=0;
			StackData->TopStr=0;
			FastShow();

			if (StackData->strSelTopic.empty())
				MoveToReference(1,1);

			return TRUE;
		}
		case KEY_END:         case KEY_NUMPAD1:
		case KEY_CTRLEND:     case KEY_CTRLNUMPAD1:
		case KEY_RCTRLEND:    case KEY_RCTRLNUMPAD1:
		case KEY_CTRLPGDN:    case KEY_CTRLNUMPAD3:
		case KEY_RCTRLPGDN:   case KEY_RCTRLNUMPAD3:
		{
			StackData->CurX=StackData->CurY=0;
			StackData->TopStr = static_cast<int>(HelpList.size());
			FastShow();

			if (StackData->strSelTopic.empty())
			{
				StackData->CurX=0;
				StackData->CurY = BodyHeight() - 1;
				MoveToReference(0,1);
			}

			return TRUE;
		}
		case KEY_UP:          case KEY_NUMPAD8:
		{
			if (StackData->TopStr>0)
			{
				StackData->TopStr--;

				if (StackData->CurY < BodyHeight() - 1)
				{
					StackData->CurX = CanvasWidth() - 2 - 1;
					StackData->CurY++;
				}

				FastShow();

				if (StackData->strSelTopic.empty())
					MoveToReference(0,1);
			}
			else
				ProcessKey(Manager::Key(KEY_SHIFTTAB));

			return TRUE;
		}
		case KEY_DOWN:        case KEY_NUMPAD2:
		{
			if (StackData->TopStr < static_cast<int>(HelpList.size()) - FixCount - BodyHeight())
			{
				StackData->TopStr++;

				if (StackData->CurY>0)
					StackData->CurY--;

				StackData->CurX=0;
				FastShow();

				if (StackData->strSelTopic.empty())
					MoveToReference(1,1);
			}
			else
				ProcessKey(Manager::Key(KEY_TAB));

			return TRUE;
		}
		/* $ 26.07.2001 VVM
		  + С альтом скролим по 1 */
		case KEY_MSWHEEL_UP:
		case KEY_MSWHEEL_UP | KEY_ALT:
		case KEY_MSWHEEL_UP | KEY_RALT:
		{
			int n = (LocalKey == KEY_MSWHEEL_UP ? (int)Global->Opt->MsWheelDeltaHelp : 1);
			while (n-- > 0)
				ProcessKey(Manager::Key(KEY_UP));

			return TRUE;
		}
		case KEY_MSWHEEL_DOWN:
		case KEY_MSWHEEL_DOWN | KEY_ALT:
		case KEY_MSWHEEL_DOWN | KEY_RALT:
		{
			int n = (LocalKey == KEY_MSWHEEL_DOWN ? (int)Global->Opt->MsWheelDeltaHelp : 1);
			while (n-- > 0)
				ProcessKey(Manager::Key(KEY_DOWN));

			return TRUE;
		}
		case KEY_PGUP:      case KEY_NUMPAD9:
		{
			StackData->CurX=StackData->CurY=0;
			StackData->TopStr -= BodyHeight() - 1;
			FastShow();

			if (StackData->strSelTopic.empty())
			{
				StackData->CurX=StackData->CurY=0;
				MoveToReference(1,1);
			}

			return TRUE;
		}
		case KEY_PGDN:      case KEY_NUMPAD3:
		{
			{
				int PrevTopStr=StackData->TopStr;
				StackData->TopStr += BodyHeight() - 1;
				FastShow();

				if (StackData->TopStr==PrevTopStr)
				{
					ProcessKey(Manager::Key(KEY_CTRLPGDN));
					return TRUE;
				}
				else
					StackData->CurX=StackData->CurY=0;

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
			if (StrCmpI(StackData->strHelpTopic.data(),HelpOnHelpTopic))
			{
				Stack.emplace(*StackData);
				IsNewTopic = true;
				JumpTopic(HelpOnHelpTopic);
				IsNewTopic = false;
				ErrorHelp = false;
			}

			return TRUE;
		}
		case KEY_SHIFTF1:
		{
			//   не поганим SelTopic, если и так в теме Contents
			if (StrCmpI(StackData->strHelpTopic.data(),HelpContents))
			{
				Stack.emplace(*StackData);
				IsNewTopic = true;
				JumpTopic(HelpContents);
				ErrorHelp = false;
				IsNewTopic = false;
			}

			return TRUE;
		}
		case KEY_F7:
		{
			// не поганим SelTopic, если и так в FoundContents
			if (StrCmpI(StackData->strHelpTopic.data(),FoundContents))
			{
				string strLastSearchStr0=strLastSearchStr;
				bool Case=LastSearchCase;
				bool WholeWords=LastSearchWholeWords;
				bool Regexp=LastSearchRegexp;

				string strTempStr;
				//int RetCode = GetString(MSG(lng::MHelpSearchTitle),MSG(lng::MHelpSearchingFor),L"HelpSearch",strLastSearchStr,strLastSearchStr0);
				int RetCode = GetSearchReplaceString(false, MSG(lng::MHelpSearchTitle), MSG(lng::MHelpSearchingFor), strLastSearchStr0, strTempStr, L"HelpSearch", L"", &Case, &WholeWords, nullptr, &Regexp, nullptr, nullptr, true, &HelpSearchId);

				if (RetCode <= 0)
					return TRUE;

				strLastSearchStr=strLastSearchStr0;
				LastSearchCase=Case;
				LastSearchWholeWords=WholeWords;
				LastSearchRegexp=Regexp;

				Stack.emplace(*StackData);
				IsNewTopic = true;
				JumpTopic(FoundContents);
				ErrorHelp = false;
				IsNewTopic = false;
			}

			return TRUE;

		}
		case KEY_SHIFTF2:
		{
			//   не поганим SelTopic, если и так в PluginContents
			if (StrCmpI(StackData->strHelpTopic.data(),PluginContents))
			{
				Stack.emplace(*StackData);
				IsNewTopic = true;
				JumpTopic(PluginContents);
				ErrorHelp = false;
				IsNewTopic = false;
			}

			return TRUE;
		}
		case KEY_ALTF1:
		case KEY_RALTF1:
		case KEY_BS:
		{
			// Если стек возврата пуст - выходим из хелпа
			if (!Stack.empty())
			{
				*StackData = std::move(Stack.top());
				Stack.pop();
				JumpTopic(StackData->strHelpTopic);
				ErrorHelp = false;
				return TRUE;
			}

			return ProcessKey(Manager::Key(KEY_ESC));
		}
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (!StackData->strSelTopic.empty() && StrCmpI(StackData->strHelpTopic, StackData->strSelTopic))
			{
				Stack.push(*StackData);
				IsNewTopic = true;

				if (!JumpTopic())
				{
					*StackData = std::move(Stack.top());
					Stack.pop();
					ReadHelp(StackData->strHelpMask); // вернем то, что отображали.
				}

				ErrorHelp = false;
				IsNewTopic = false;
			}

			return TRUE;
		}
	}

	return FALSE;
}

int Help::JumpTopic(const string& Topic)
{
	StackData->strSelTopic = Topic;
	return JumpTopic();
}

int Help::JumpTopic()
{
	string strNewTopic;
	size_t pos = 0;

	/* $ 14.07.2002 IS
	     При переходе по ссылкам используем всегда только абсолютные пути,
	     если это возможно.
	*/

	// Если ссылка на другой файл, путь относительный и есть то, от чего можно
	// вычислить абсолютный путь, то сделаем это
	if (StackData->strSelTopic.front()==HelpBeginLink
	        && (pos = StackData->strSelTopic.find(HelpEndLink,2)) != string::npos
	        && !IsAbsolutePath(StackData->strSelTopic.data()+1)
	        && !StackData->strHelpPath.empty())
	{
		strNewTopic = StackData->strSelTopic.substr(1, pos);
		string strFullPath = StackData->strHelpPath;
		// уберем _все_ конечные слеши и добавим один
		DeleteEndSlash(strFullPath);
		strFullPath.append(1, L'\\').append(strNewTopic, IsSlash(strNewTopic.front())? 1 : 0, string::npos);
		BOOL EndSlash = IsSlash(strFullPath.back());
		DeleteEndSlash(strFullPath);
		strNewTopic = ConvertNameToFull(strFullPath);
		strFullPath = format(EndSlash?HelpFormatLink:HelpFormatLinkModule, strNewTopic, wcschr(StackData->strSelTopic.data() + 2, HelpEndLink) + 1);
		StackData->strSelTopic = strFullPath;
	}

	//_SVS(SysLog(L"JumpTopic() = SelTopic=%s",StackData->SelTopic));
	// URL активатор - это ведь так просто :-)))
	{
		strNewTopic = StackData->strSelTopic;
		pos = strNewTopic.find(L':');

		if (pos != string::npos && strNewTopic.front() != L':') // наверное подразумевается URL
		{
			string Protocol(strNewTopic, 0, pos);

			if (RunURL(Protocol, StackData->strSelTopic))
			{
				return FALSE;
			}
		}
	}
	// а вот теперь попробуем...

	//_SVS(SysLog(L"JumpTopic() = SelTopic=%s, StackData->HelpPath=%s",StackData->SelTopic,StackData->HelpPath));
	if (!StackData->strHelpPath.empty() && StackData->strSelTopic.front() !=HelpBeginLink && StackData->strSelTopic != HelpOnHelpTopic)
	{
		if (StackData->strSelTopic.front()==L':')
		{
			strNewTopic = StackData->strSelTopic.substr(1);
			StackData->Flags&=~FHELP_CUSTOMFILE;
		}
		else if (StackData->Flags&FHELP_CUSTOMFILE)
			strNewTopic = StackData->strSelTopic;
		else
			strNewTopic = MakeLink(StackData->strHelpPath, StackData->strSelTopic);
	}
	else
	{
		strNewTopic = StackData->strSelTopic.substr(StackData->strSelTopic == HelpOnHelpTopic? 1 : 0);
	}

	// удалим ссылку на .DLL
	size_t EndPos = strNewTopic.rfind(HelpEndLink);

	if (EndPos != string::npos)
	{
		if (!IsSlash(strNewTopic[EndPos - 1]))
		{
			size_t Pos2 = EndPos;

			while (EndPos != string::npos)
			{
				if (IsSlash(strNewTopic[EndPos]))
				{
					StackData->Flags|=FHELP_CUSTOMFILE;
					StackData->strHelpMask = strNewTopic.substr(EndPos + 1);
					StackData->strHelpMask.resize(StackData->strHelpMask.find(HelpEndLink));

					strNewTopic.erase(EndPos, Pos2 - EndPos);

					size_t Pos3 = StackData->strHelpMask.rfind(L'.');
					if (Pos3 != string::npos && StrCmpI(StackData->strHelpMask.data() + Pos3, L".hlf"))
						StackData->strHelpMask.clear();

					break;
				}

				--EndPos;
			}
		}
		else
		{
			StackData->Flags&=~FHELP_CUSTOMFILE;
			StackData->Flags|=FHELP_CUSTOMPATH;
		}
	}

	//_SVS(SysLog(L"HelpMask=%s NewTopic=%s",StackData->HelpMask,NewTopic));
	if (StackData->strSelTopic.front() != L':' &&
	        (StrCmpI(StackData->strSelTopic.data(),PluginContents) || StrCmpI(StackData->strSelTopic.data(),FoundContents))
	   )
	{
		if (!(StackData->Flags&FHELP_CUSTOMFILE) && wcsrchr(strNewTopic.data(),HelpEndLink))
		{
			StackData->strHelpMask.clear();
		}
	}
	else
	{
		StackData->strHelpMask.clear();
	}

	StackData->strHelpTopic = strNewTopic;

	if (!(StackData->Flags&FHELP_CUSTOMFILE))
		StackData->strHelpPath.clear();

	if (!ReadHelp(StackData->strHelpMask))
	{
		StackData->strHelpTopic = strNewTopic;

		if (StackData->strHelpTopic.front() == HelpBeginLink)
		{
			if ((pos = StackData->strHelpTopic.rfind(HelpEndLink)) != string::npos)
			{
				StackData->strHelpTopic.resize(pos+1);
				StackData->strHelpTopic += HelpContents;
			}
		}

		StackData->strHelpPath.clear();
		ReadHelp(StackData->strHelpMask);
	}

	m_Flags.Clear(FHELPOBJ_ERRCANNOTOPENHELP);

	if (HelpList.empty())
	{
		ErrorHelp = true;

		if (!(StackData->Flags&FHELP_NOSHOWERROR))
		{
			Message(MSG_WARNING,1,MSG(lng::MHelpTitle),MSG(lng::MHelpTopicNotFound),StackData->strHelpTopic.data(),MSG(lng::MOk));
		}

		return FALSE;
	}

	// ResizeConsole();
	if (IsNewTopic
	        || !(StrCmpI(StackData->strSelTopic.data(),PluginContents)||StrCmpI(StackData->strSelTopic.data(),FoundContents)) // Это неприятный костыль :-((
	   )
		MoveToReference(1,1);

	Global->WindowManager->RefreshWindow();
	return TRUE;
}

int Help::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (m_windowKeyBar->ProcessMouse(MouseEvent))
		return TRUE;

	if (MouseEvent->dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED && MouseEvent->dwEventFlags!=MOUSE_MOVED)
	{
		ProcessKey(Manager::Key(KEY_ENTER));
		return TRUE;
	}

	int prevMsX = MsX , prevMsY = MsY;
	MsX=MouseEvent->dwMousePosition.X;
	MsY=MouseEvent->dwMousePosition.Y;
	bool simple_move = (IntKeyState.MouseEventFlags == MOUSE_MOVED);


	if ((MsX<m_X1 || MsY<m_Y1 || MsX>m_X2 || MsY>m_Y2) && IntKeyState.MouseEventFlags != MOUSE_MOVED)
	{
		static constexpr int HELPMODE_CLICKOUTSIDE = 0x20000000; // было нажатие мыши вне хелпа?

		if (m_Flags.Check(HELPMODE_CLICKOUTSIDE))
		{
			// Вываливаем если предыдущий эвент не был двойным кликом
			if (IntKeyState.PreMouseEventFlags != DOUBLE_CLICK)
				ProcessKey(Manager::Key(KEY_ESC));
		}

		if (MouseEvent->dwButtonState)
			m_Flags.Set(HELPMODE_CLICKOUTSIDE);

		return TRUE;
	}

	if (IntKeyState.MouseX==m_X2 && (MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED))
	{
		int ScrollY = m_Y1 + HeaderHeight() + 1;

		if (IntKeyState.MouseY==ScrollY)
		{
			while (IsMouseButtonPressed())
				ProcessKey(Manager::Key(KEY_UP));

			return TRUE;
		}

		if (IntKeyState.MouseY == ScrollY + BodyHeight() - 1)
		{
			while (IsMouseButtonPressed())
				ProcessKey(Manager::Key(KEY_DOWN));

			return TRUE;
		}
		simple_move = false;
	}

	/* $ 15.03.2002 DJ
	   обработаем щелчок в середине скроллбара
	*/
	if (IntKeyState.MouseX == m_X2)
	{
		int ScrollY = m_Y1 + HeaderHeight() + 1;

		if (static_cast<int>(HelpList.size()) > BodyHeight())
		{
			while (IsMouseButtonPressed())
			{
				if (IntKeyState.MouseY > ScrollY && IntKeyState.MouseY < ScrollY + BodyHeight() + 1)
				{
					StackData->CurX=StackData->CurY=0;
					StackData->TopStr = (IntKeyState.MouseY - ScrollY - 1) * (static_cast<int>(HelpList.size()) - FixCount - BodyHeight() + 1) / (BodyHeight() - 2);
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
		MouseEvent->dwMousePosition.Y < m_Y1 + 1 + HeaderHeight())
	{
		ProcessKey(Manager::Key(KEY_F5));
		return TRUE;
	}

	if (MouseEvent->dwMousePosition.Y < m_Y1 + 1 + HeaderHeight())
	{
		while (IsMouseButtonPressed() && IntKeyState.MouseY < m_Y1 + 1 + HeaderHeight())
			ProcessKey(Manager::Key(KEY_UP));

		return TRUE;
	}

	if (MouseEvent->dwMousePosition.Y>=m_Y2)
	{
		while (IsMouseButtonPressed() && IntKeyState.MouseY>=m_Y2)
			ProcessKey(Manager::Key(KEY_DOWN));

		return TRUE;
	}

	/* $ 26.11.2001 VVM
	  + Запомнить нажатие клавиши мышки и только в этом случае реагировать при отпускании */
	if (!MouseEvent->dwEventFlags
	 && (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED)))
	{
		BeforeMouseDownX = StackData->CurX;
		BeforeMouseDownY = StackData->CurY;
		StackData->CurX = MouseDownX = MsX-m_X1-1;
		StackData->CurY = MouseDownY = MsY - m_Y1 - 1 - HeaderHeight();
		MouseDown = true;
		simple_move = false;
	}

	if (!MouseEvent->dwEventFlags
	 && !(MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED))
	 && MouseDown)
	{
		simple_move = false;
		MouseDown = false;
		if (!StackData->strSelTopic.empty())
		{
			if (StackData->CurX == MouseDownX && StackData->CurY == MouseDownY)
				ProcessKey(Manager::Key(KEY_ENTER));
		}
		else
		{
			if (StackData->CurX==MouseDownX && StackData->CurY==MouseDownY)
			{
				StackData->CurX = BeforeMouseDownX;
				StackData->CurY = BeforeMouseDownY;
			}
		}
	}

	if (simple_move && (prevMsX != MsX || prevMsY != MsY))
	{
		string strTopic;
		if (GetTopic(MsX, MsY, strTopic))
		{
			//if (strTopic != StackData->strSelTopic)
			{
				StackData->CurX = MsX-m_X1-1;
				StackData->CurY = MsY - m_Y1 - 1 - HeaderHeight();
			}
		}
	}

	FastShow();
	Sleep(1);
	return TRUE;
}

bool Help::IsReferencePresent()
{
	CorrectPosition();
	int StrPos=FixCount+StackData->TopStr+StackData->CurY;

	if (StrPos >= static_cast<int>(HelpList.size()))
	{
		return false;
	}

	return contains(HelpList[StrPos].HelpStr, L"~@");
}

void Help::MoveToReference(int Forward,int CurScreen)
{
	int StartSelection=!StackData->strSelTopic.empty();
	int SaveCurX=StackData->CurX;
	int SaveCurY=StackData->CurY;
	int SaveTopStr=StackData->TopStr;
	StackData->strSelTopic.clear();

	if (!ErrorHelp)
		while (StackData->strSelTopic.empty())
		{
			BOOL ReferencePresent=IsReferencePresent();

			if (Forward)
			{
				if (!StackData->CurX && !ReferencePresent)
					StackData->CurX = CanvasWidth() - 1;

				if (++StackData->CurX >= CanvasWidth() - 1)
				{
					StartSelection=0;
					StackData->CurX=0;
					StackData->CurY++;

					if (StackData->TopStr + StackData->CurY >= static_cast<int>(HelpList.size()) - FixCount || (CurScreen && StackData->CurY > BodyHeight() - 1))
						break;
				}
			}
			else
			{
				if (StackData->CurX == CanvasWidth() - 1 && !ReferencePresent)
					StackData->CurX=0;

				if (--StackData->CurX < 0)
				{
					StartSelection=0;
					StackData->CurX = CanvasWidth() - 1;
					StackData->CurY--;

					if (StackData->TopStr+StackData->CurY<0 ||
					        (CurScreen && StackData->CurY<0))
						break;
				}
			}

			FastShow();

			if (StackData->strSelTopic.empty())
				StartSelection=0;
			else
			{
				// небольшая заплата, артефакты есть но уже меньше :-)
				if (ReferencePresent && CurScreen)
					StartSelection=0;

				if (StartSelection)
					StackData->strSelTopic.clear();
			}
		}

	if (StackData->strSelTopic.empty())
	{
		StackData->CurX=SaveCurX;
		StackData->CurY=SaveCurY;
		StackData->TopStr=SaveTopStr;
	}

	FastShow();
}

void Help::Search(const os::fs::file& HelpFile,uintptr_t nCodePage)
{
	FixCount=1;
	StackData->TopStr=0;
	m_TopicFound = true;
	StackData->CurX=StackData->CurY=0;
	strCtrlColorChar.clear();

	string strTitleLine=strLastSearchStr;
	AddTitle(strTitleLine);

	bool TopicFound=false;
	GetFileString GetStr(HelpFile, nCodePage);
	string strCurTopic, strEntryName, strReadStr;

	string strSlash(strLastSearchStr);
	InsertRegexpQuote(strSlash);
	std::vector<RegExpMatch> m;
	MatchHash hm;
	RegExp re;

	if (LastSearchRegexp)
	{
		// Q: что важнее: опция диалога или опция RegExp`а?
		if (!re.Compile(strSlash.data(), OP_PERLSTYLE|OP_OPTIMIZE|(!LastSearchCase?OP_IGNORECASE:0)))
		{
			ReCompileErrorMessage(re, strSlash);
			return; //BUGBUG
		}

		m.resize(re.GetBracketsCount() * 2);
	}

	string strSearchStrUpper = strLastSearchStr;
	string strSearchStrLower = strLastSearchStr;
	if (!LastSearchCase)
	{
		InplaceUpper(strSearchStrUpper);
		InplaceLower(strSearchStrLower);
	}

	for (;;)
	{
		if (!GetStr.GetString(strReadStr))
		{
			break;
		}

		RemoveTrailingSpaces(strReadStr);

		if ((!strReadStr.empty() && strReadStr[0] == L'@') &&
		    !(strReadStr.size() > 1 && (strReadStr[1] == L'+' || strReadStr[1] == L'-')) &&
		    !contains(strReadStr, L'='))// && !TopicFound)
		{
			strEntryName.clear();
			strCurTopic.clear();
			RemoveExternalSpaces(strReadStr);
			if (StrCmpI(strReadStr.data()+1,HelpContents))
			{
				strCurTopic=strReadStr;
				TopicFound=true;
			}
		}
		else if (TopicFound && strReadStr.size() > 1 && strReadStr[0] == L'$' && !strCurTopic.empty())
		{
			strEntryName=strReadStr.substr(1);
			RemoveExternalSpaces(strEntryName);
			strEntryName.erase(std::remove(ALL_RANGE(strEntryName), L'#'), strEntryName.end());
		}

		if (TopicFound && !strEntryName.empty())
		{
			// !!!BUGBUG: необходимо "очистить" строку strReadStr от элементов разметки !!!
			string ReplaceStr;
			int CurPos=0;
			int SearchLength;
			bool Result = SearchString(strReadStr.data(), (int)strReadStr.size(), strLastSearchStr, strSearchStrUpper, strSearchStrLower, re, m.data(), &hm, ReplaceStr, CurPos, 0, LastSearchCase, LastSearchWholeWords, false, false, LastSearchRegexp, &SearchLength);

			if (Result)
			{
				AddLine(concat(L"   ~"s, strEntryName, L'~', strCurTopic, L'@'));
				strCurTopic.clear();
				strEntryName.clear();
				TopicFound=false;
			}
		}
	}

	AddLine(L"");
	MoveToReference(1,1);
}

void Help::ReadDocumentsHelp(int TypeIndex)
{
	HelpList.clear();

	strCurPluginContents.clear();
	FixCount=1;
	StackData->TopStr=0;
	m_TopicFound = true;
	StackData->CurX=StackData->CurY=0;
	strCtrlColorChar.clear();
	const wchar_t *PtrTitle = nullptr, *ContentsName = nullptr;
	string strPath, strFullFileName;

	switch (TypeIndex)
	{
		case HIDX_PLUGINS:
			PtrTitle=MSG(lng::MPluginsHelpTitle);
			ContentsName=L"PluginContents";
			break;
		default:
			throw MAKE_FAR_EXCEPTION(L"Unsupported index");
	}

	AddTitle(PtrTitle);
	/* TODO:
	   1. Поиск (для "документов") не только в каталоге Documets, но
	      и в плагинах
	*/
	switch (TypeIndex)
	{
		case HIDX_PLUGINS:
		{
			std::for_each(CONST_RANGE(*Global->CtrlObject->Plugins, i)
			{
				strPath = i->GetModuleName();
				CutToSlash(strPath);
				uintptr_t nCodePage = CP_OEMCP;
				os::fs::file HelpFile;
				if (OpenLangFile(HelpFile,strPath,Global->HelpFileMask,Global->Opt->strHelpLanguage,strFullFileName, nCodePage))
				{
					string strEntryName, strSecondParam;

					if (GetLangParam(HelpFile,ContentsName,&strEntryName,&strSecondParam, nCodePage))
					{
						string strHelpLine = L"   ~" + strEntryName;
						if (!strSecondParam.empty())
						{
							strHelpLine += L"," + strSecondParam;
						}
						strHelpLine += L"~@" + MakeLink(strPath, HelpContents) + L"@";

						AddLine(strHelpLine);
					}
				}
			});

			break;
		}
		default:
			throw MAKE_FAR_EXCEPTION(L"Unsupported index");
	}

	// сортируем по алфавиту
	std::sort(HelpList.begin()+1, HelpList.end());
	// $ 26.06.2000 IS - Устранение глюка с хелпом по f1, shift+f2, end (решение предложил IG)
	AddLine(L"");
}

// Формирование топика с учетом разных факторов
bool Help::MkTopic(const Plugin* pPlugin, const string& HelpTopic, string &strTopic)
{
	strTopic.clear();

	if (!HelpTopic.empty())
	{
		if (HelpTopic.front() == L':')
		{
			strTopic = HelpTopic.substr(1);
		}
		else
		{
			if (pPlugin && HelpTopic.front() != HelpBeginLink)
			{
				strTopic = format(HelpFormatLinkModule, pPlugin->GetModuleName(), HelpTopic);
			}
			else
			{
				strTopic = HelpTopic;
			}

			if (strTopic.front()==HelpBeginLink)
			{
				size_t EndPos = strTopic.find(HelpEndLink);

				if (EndPos == string::npos)
				{
					strTopic.clear();
				}
				else
				{
					if (EndPos == strTopic.size() - 1) // Вона как поперло то...
						strTopic += HelpContents; // ... значит покажем основную тему. //BUGBUG

					/* А вот теперь разгребем...
					   Формат может быть :
					     "<FullPath>Topic" или "<FullModuleName>Topic"
					   Для случая "FullPath" путь ДОЛЖЕН заканчиваться СЛЕШЕМ!
					   Т.о. мы отличим ЧТО ЭТО - имя модуля или путь!
					*/

					size_t SlashPos = EndPos - 1;

					if (!IsSlash(strTopic[SlashPos])) // Это имя модуля?
					{
						// значит удалим это чертово имя :-)
						const auto pos = FindLastSlash(strTopic);
						if (pos != string::npos)
						{
							SlashPos = pos;
						}
						else // ВО! Фигня какая-то :-(
						{
							strTopic.clear();
						}
					}

					if (!strTopic.empty())
					{
						strTopic.erase(SlashPos + 1, EndPos - SlashPos - 1);
					}
				}
			}
		}
	}
	return !strTopic.empty();
}

void Help::SetScreenPosition()
{
	if (Global->Opt->FullScreenHelp)
	{
		m_windowKeyBar->Hide();
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
	m_windowKeyBar->SetLabels(lng::MHelpF1);
	m_windowKeyBar->SetCustomLabels(KBA_HELP);
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
static int RunURL(const string& Protocol, const string& URLPath)
{
	int EditCode=0;

	if (!URLPath.empty() && (Global->Opt->HelpURLRules&0xFF))
	{
		string strType;

		if (GetShellType(Protocol,strType,AT_URLPROTOCOL))
		{
			string strAction;
			bool Success;
			if (strType.find(L"%1") != string::npos)
			{
				strAction = strType;
				Success = true;
			}
			else
			{
				strType += L"\\shell\\open\\command";
				Success = os::reg::GetValue(HKEY_CLASSES_ROOT, strType, L"", strAction);
			}

			if (Success)
			{
				strAction = os::env::expand_strings(strAction);

				string FilteredURLPath(URLPath);
				// удалим два идущих подряд ~~
				ReplaceStrings(FilteredURLPath, L"~~", L"~");
				// удалим два идущих подряд ##
				ReplaceStrings(FilteredURLPath, L"##", L"#");

				int Disposition=0;

				if (Global->Opt->HelpURLRules == 2 || Global->Opt->HelpURLRules == 2+256)
				{
					Disposition=Message(MSG_WARNING,2,MSG(lng::MHelpTitle),
						                MSG(lng::MHelpActivatorURL),
						                strAction.data(),
						                MSG(lng::MHelpActivatorFormat),
						                FilteredURLPath.data(),
						                L"\x01",
						                MSG(lng::MHelpActivatorQ),
						                MSG(lng::MYes),MSG(lng::MNo));
				}

				EditCode=2; // Все Ok!

				if (!Disposition)
				{
					/*
					СЮДЫ НУЖНО ВПИНДЮЛИТЬ МЕНЮХУ С ВОЗМОЖНОСТЬЮ ВЫБОРА
					ТОГО ИЛИ ИНОГО АКТИВАТОРА - ИХ МОЖЕТ БЫТЬ НЕСКОЛЬКО!!!!!
					*/
					const auto strCurDir = os::GetCurrentDirectory();

					if (Global->Opt->HelpURLRules < 256) // SHELLEXECUTEEX_METHOD
					{
						strAction=FilteredURLPath;
						EditCode = ShellExecute(nullptr, nullptr, RemoveExternalSpaces(strAction).data(), nullptr, strCurDir.data(), SW_SHOWNORMAL) ? 1 : 2;
					}
					else
					{
						STARTUPINFO si={sizeof(si)};
						PROCESS_INFORMATION pi={};

						if (ReplaceStrings(strAction, L"%1", FilteredURLPath, false, 1) == 0) //if %1 not found
						{
							strAction += L" ";
							strAction += FilteredURLPath;
						}

						if (!CreateProcess(nullptr, UNSAFE_CSTR(strAction),nullptr,nullptr,TRUE,0,nullptr,strCurDir.data(),&si,&pi))
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

	return EditCode;
}

void Help::ResizeConsole()
{
	bool OldIsNewTopic=IsNewTopic;
	bool ErrCannotOpenHelp=m_Flags.Check(FHELPOBJ_ERRCANNOTOPENHELP);
	m_Flags.Set(FHELPOBJ_ERRCANNOTOPENHELP);
	IsNewTopic = false;
	Hide();

	if (Global->Opt->FullScreenHelp)
	{
		m_windowKeyBar->Hide();
		SetPosition(0,0,ScrX,ScrY);
	}
	else
		SetPosition(4,2,ScrX-4,ScrY-2);

	ReadHelp(StackData->strHelpMask);
	ErrorHelp = false;
	//StackData->CurY--; // ЭТО ЕСМЬ КОСТЫЛЬ (пусть пока будет так!)
	StackData->CurX--;
	MoveToReference(1,1);
	IsNewTopic=OldIsNewTopic;
	m_Flags.Change(FHELPOBJ_ERRCANNOTOPENHELP,ErrCannotOpenHelp);
}

bool Help::CanFastHide() const
{
	return (Global->Opt->AllCtrlAltShiftRule & CASR_HELP) != 0;
}

int Help::GetTypeAndName(string &strType, string &strName)
{
	strType = MSG(lng::MHelpType);
	strName = strFullHelpPathName;
	return windowtype_help;
}
