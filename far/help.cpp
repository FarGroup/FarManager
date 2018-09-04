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
#include "pathmix.hpp"
#include "strmix.hpp"
#include "exitcode.hpp"
#include "filestr.hpp"
#include "colormix.hpp"
#include "stddlg.hpp"
#include "plugins.hpp"
#include "DlgGuid.hpp"
#include "RegExp.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "keybar.hpp"
#include "string_sort.hpp"
#include "cvtname.hpp"
#include "cmdline.hpp"
#include "global.hpp"

#include "platform.fs.hpp"

#include "format.hpp"

static const auto
	FoundContents = L"__FoundContents__"sv,
	PluginContents = L"__PluginContents__"sv,
	HelpOnHelpTopic = L":Help"sv,
	HelpContents = L"Contents"sv;

static const wchar_t HelpBeginLink = L'<';
static const wchar_t HelpEndLink = L'>';

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
	MOVABLE(HelpRecord);

	string HelpStr;

	explicit HelpRecord(string HStr): HelpStr(std::move(HStr)) {}

	bool operator ==(const HelpRecord& rhs) const
	{
		return equal_icase(HelpStr, rhs.HelpStr);
	}

	bool operator <(const HelpRecord& rhs) const
	{
		return string_sort::less(HelpStr, rhs.HelpStr);
	}
};

static bool OpenURL(const string& URLPath);

static const wchar_t HelpFormatLink[] = L"<{0}\\>{1}";
static const wchar_t HelpFormatLinkModule[] = L"<{0}>{1}";


struct Help::StackHelpData
{
	COPYABLE(StackHelpData);
	MOVABLE(StackHelpData);

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

	unsigned long long Flags;     // флаги
	int   TopStr;                 // номер верхней видимой строки темы
	int   CurX, CurY;             // координаты (???)
};

string Help::MakeLink(string_view const path, string_view const topic)
{
	return concat(L'<', path, L"\\>"sv, topic);
}

static bool GetOptionsParam(const os::fs::file& LangFile, string_view const KeyName, string& Value, UINT CodePage)
{
	return GetLangParam(LangFile, L"Options "sv + KeyName, Value, nullptr, CodePage);
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
	CtrlTabSize(Global->Opt->HelpTabSize),
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

help_ptr Help::create(string_view const Topic, const wchar_t *Mask, unsigned long long Flags)
{
	auto HelpPtr = std::make_shared<Help>(private_tag());
	HelpPtr->init(Topic, Mask, Flags);
	return HelpPtr;
}

void Help::init(string_view const Topic, const wchar_t *Mask, unsigned long long Flags)
{
	m_windowKeyBar = std::make_unique<KeyBar>(shared_from_this());

	m_CanLoseFocus = false;
	SetRestoreScreenMode(true);

	StackData->Flags=Flags;
	StackData->strHelpMask = NullToEmpty(Mask); // сохраним маску файла
	assign(StackData->strHelpTopic, Topic);

	if (Global->Opt->FullScreenHelp)
		SetPosition(0,0,ScrX,ScrY);
	else
		SetPosition(4,2,ScrX-4,ScrY-2);

	if (!ReadHelp(StackData->strHelpMask) && (Flags&FHELP_USECONTENTS))
	{
		assign(StackData->strHelpTopic, Topic);

		if (starts_with(StackData->strHelpTopic, HelpBeginLink))
		{
			size_t pos = StackData->strHelpTopic.rfind(HelpEndLink);

			if (pos != string::npos)
				StackData->strHelpTopic.resize(pos + 1);

			append(StackData->strHelpTopic, HelpContents);
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
				Message(MSG_WARNING,
					msg(lng::MHelpTitle),
					{
						msg(lng::MHelpTopicNotFound),
						StackData->strHelpTopic
					},
					{ lng::MOk });
			}

			m_Flags.Clear(FHELPOBJ_ERRCANNOTOPENHELP);
		}
	}
}

bool Help::ReadHelp(const string& Mask)
{
	string strPath;

	if (starts_with(StackData->strHelpTopic, HelpBeginLink))
	{
		strPath = StackData->strHelpTopic.substr(1);
		const auto pos = strPath.find(HelpEndLink);

		if (pos == string::npos)
			return false;

		StackData->strHelpTopic.assign(strPath, pos + 1, string::npos); // gcc 7.3 bug, npos required
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
		return true;
	}

	const auto HelpFileData = OpenLangFile(strPath, Mask.empty()? Global->HelpFileMask : Mask, Global->Opt->strHelpLanguage);
	const auto& HelpFile = std::get<0>(HelpFileData);
	const auto HelpFileCodePage = std::get<2>(HelpFileData);
	if (!HelpFile)
	{
		ErrorHelp = true;

		if (!m_Flags.Check(FHELPOBJ_ERRCANNOTOPENHELP))
		{
			m_Flags.Set(FHELPOBJ_ERRCANNOTOPENHELP);

			if (!(StackData->Flags&FHELP_NOSHOWERROR))
			{
				Message(MSG_WARNING,
					msg(lng::MHelpTitle),
					{
						msg(lng::MCannotOpenHelp),
						Mask
					},
					{ lng::MOk });
			}
		}

		return false;
	}

	strFullHelpPathName = HelpFile.GetName();

	string strReadStr;

	if (GetOptionsParam(HelpFile, L"TabSize"sv, strReadStr, HelpFileCodePage))
	{
		int UserTabSize;
		if (from_string(strReadStr, UserTabSize))
		{
			if (InRange(0, UserTabSize, 16))
			{
				CtrlTabSize = UserTabSize;
			}
			else
			{
				// TODO: log tabsize out of range
			}
		}
		else
		{
			// TODO: log error reading tabsize
		}
	}

	if (GetOptionsParam(HelpFile, L"CtrlColorChar"sv, strReadStr, HelpFileCodePage))
		strCtrlColorChar = strReadStr;
	else
		strCtrlColorChar.clear();

	if (GetOptionsParam(HelpFile, L"CtrlStartPosChar"sv, strReadStr, HelpFileCodePage))
		strCtrlStartPosChar = strReadStr;
	else
		strCtrlStartPosChar.clear();

	/* $ 29.11.2001 DJ
	   запомним, чего там написано в PluginContents
	*/
	if (!GetLangParam(HelpFile, L"PluginContents"sv, strCurPluginContents, nullptr, HelpFileCodePage))
		strCurPluginContents.clear();

	string strTabSpace(CtrlTabSize, L' ');

	HelpList.clear();

	if (StackData->strHelpTopic == FoundContents)
	{
		Search(HelpFile, HelpFileCodePage);
		return true;
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

	StartPos = 0;
	LastStartPos = 0;
	bool MacroProcess=false;
	int MI=0;
	string strMacroArea;

	enum_file_lines EnumFileLines(HelpFile, HelpFileCodePage);
	auto FileIterator = EnumFileLines.begin();
	const size_t StartSizeKeyName = 20;
	size_t SizeKeyName = StartSizeKeyName;
	string strSplitLine;

	bool InHeader = true;

	for (;;)
	{
		const int RealMaxLength = MaxLength - StartPos;

		if (!MacroProcess && !RepeatLastLine && !BreakProcess)
		{
			if (FileIterator != EnumFileLines.end())
			{
				assign(strReadStr, FileIterator->Str);
				++FileIterator;

				if (InHeader)
				{
					// Everything up to the first topic is header, we don't care about it here
					InHeader = strReadStr.empty() || strReadStr.front() != L'@';
					if (InHeader)
						continue;
				}
			}
			else
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

			++MI;

			const auto& escape = [](string_view const Str)
			{
				string Result(Str);
				ReplaceStrings(Result, L"~"sv, L"~~"sv);
				ReplaceStrings(Result, L"#"sv, L"##"sv);
				ReplaceStrings(Result, L"@"sv, L"@@"sv);
				return Result;
			};

			size_t LastKeySize = 0;

			strReadStr = join(select(enum_tokens(strKeyName, L" "sv),
				[&](const auto& i)
				{
					LastKeySize = i.size();
					return concat(L" #"sv, escape(i), L'#');
				}),
				L"\n"sv);

			if (!strDescription.empty())
			{
				strReadStr.append(std::max(LastKeySize, SizeKeyName) - LastKeySize, L' ');
				append(strReadStr, strCtrlStartPosChar, escape(strDescription));
			}
		}

		RepeatLastLine = false;

		{
			size_t PosTab;
			while ((PosTab = strReadStr.find(L'\t')) != string::npos)
			{
				strReadStr[PosTab] = L' ';

				if (CtrlTabSize > 1) // заменим табулятор по всем правилам
					strReadStr.insert(PosTab, strTabSpace.c_str(), CtrlTabSize - (PosTab % CtrlTabSize));
			}
		}

		inplace::trim_right(strReadStr);

		if (!strCtrlStartPosChar.empty())
		{
			size_t pos = strReadStr.find(strCtrlStartPosChar);
			if (pos != string::npos)
			{
				size_t p1 = strReadStr.rfind(L'\n') + 1;
				if (p1 > pos)
					p1 = 0;
				LastStartPos = StringLen(string_view(strReadStr).substr(p1, pos-p1));
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
				if (strReadStr == L"@+"sv)
				{
					Formatting = true;
					PrevSymbol=0;
					continue;
				}

				if (strReadStr == L"@-"sv)
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
			else if (equal_icase(string_view(strReadStr).substr(1), StackData->strHelpTopic))
			{
				m_TopicFound = true;
				NearTopicFound=1;
			}
			else // redirection @SearchTopic=RealTopic
			{
				size_t n1 = StackData->strHelpTopic.size();
				size_t n2 = strReadStr.size();
				if (1 + n1 + 1 < n2 && starts_with_icase(string_view(strReadStr).substr(1), StackData->strHelpTopic) && strReadStr[1 + n1] == L'=')
				{
					StackData->strHelpTopic.assign(strReadStr, 1 + n1 + 1, string::npos); // gcc 7.3 bug, npos required
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
				if (starts_with_icase(strReadStr, L"<!Macro:"sv) && Global->CtrlObject)
				{
					const auto PosTab = strReadStr.find(L'>');
					if (PosTab != string::npos && strReadStr[PosTab - 1] != L'!')
						continue;

					strMacroArea.assign(strReadStr, 8, PosTab - 1 - 8); //???
					MacroProcess=true;
					MI=0;
					SizeKeyName = StartSizeKeyName;
					string strDescription,strKeyName;
					while (Global->CtrlObject->Macro.GetMacroKeyInfo(strMacroArea,MI,strKeyName,strDescription))
					{
						for (const auto& i: enum_tokens(strKeyName, L" "sv))
						{
							SizeKeyName = std::min(std::max(SizeKeyName, i.size()), static_cast<size_t>(MaxLength) / 2);
						}
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
					StartPos = 0;
					LastStartPos = 0;
				}

				if ((!strReadStr.empty() && strReadStr[0]==L'$') && NearTopicFound && (PrevSymbol == L'$' || PrevSymbol == L'@'))
				{
					AddLine(string_view(strReadStr).substr(1));
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
									LastStartPos = 0;
									StartPos = 0;
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
						else
						{
							AddLine({});
							continue;
						}
					}

					if (!strReadStr.empty() && std::iswblank(strReadStr[0]) && Formatting)
					{
						if (StringLen(strSplitLine)<RealMaxLength)
						{
							if (!strSplitLine.empty())
							{
								AddLine(strSplitLine);
								StartPos = 0;
							}

							for (size_t nl = strReadStr.find(L'\n'); nl != string::npos; )
							{
								AddLine(string_view(strReadStr).substr(0, nl));
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
							StartPos = 0;
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
							strSplitLine += L' ';

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
						if (strSplitLine[I]==L'~' && strSplitLine[I - 1] == L'~')
						 {
							I--;
							continue;
						}

						if (strSplitLine[I] == L'~' && strSplitLine[I - 1] != L'~')
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
							const auto FirstPart = string_view(strSplitLine).substr(0, I);
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

	AddLine({});
	ErrorHelp = false;

	if (IsNewTopic)
	{
		StackData->CurX = StackData->CurY = 0;
		StackData->TopStr = 0;
	}

	return m_TopicFound;
}

void Help::AddLine(const string_view Line)
{
	const auto Width = StartPos && !Line.empty() && Line[0] == L' '? StartPos - 1 : StartPos;
	HelpList.emplace_back(string(Width, L' ') + Line);
}

void Help::AddTitle(const string_view Title)
{
	AddLine(concat(L"^ #"sv, Title, L'#'));
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
				Message(MSG_WARNING,
					msg(lng::MHelpTitle),
					{
						msg(lng::MHelpTopicNotFound),
						StackData->strHelpTopic
					},
					{ lng::MOk });
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
			string_view OutStr = HelpList[StrPos].HelpStr;

			if (starts_with(OutStr, L'^'))
			{
				OutStr.remove_prefix(1);
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
	auto strHelpTitleBuf = concat(msg(lng::MHelpTitle), L" - "sv, strCurPluginContents.empty()? L"Far"s : strCurPluginContents);
	TruncStrFromEnd(strHelpTitleBuf, CanvasWidth() - 2);
	GotoXY(m_X1 + (ObjWidth() - (int)strHelpTitleBuf.size() - 2) / 2, m_Y1);
	Text(concat(L' ', strHelpTitleBuf, L' '));
}

static string_view SkipLink(string_view Str, string* const Name)
{
	for (;;)
	{
		while (!Str.empty() && Str[0] != L'@')
		{
			if (Name)
				Name->push_back(Str[0]);
			Str.remove_prefix(1);
		}

		if (!Str.empty())
			Str.remove_prefix(1);

		if (Str.empty() || Str[0] != L'@')
			break;

		if (Name)
			Name->push_back(Str[0]);

		Str.remove_prefix(1);
	}
	return Str;
}

static bool GetHelpColor(string_view& Str, wchar_t cColor, FarColor& color)
{
	if (!cColor || Str.size() < 2 || Str[0] != cColor || Str[1] == cColor)
		return false;

	// '\-' set default color
	if (Str[1] == L'-')
	{
		color = colors::PaletteColorToFarColor(COL_HELPTEXT);
		Str.remove_prefix(2);
		return true;
	}

	// '\hh' custom color index
	if (Str.size() > 2 && std::iswxdigit(Str[1]) && std::iswxdigit(Str[2]))
	{
		const auto b = HexToInt(Str[1]);
		const auto f = HexToInt(Str[2]);
		color = colors::ConsoleColorToFarColor((b & 0x0f) << 4 | (f & 0x0f));
		Str.remove_prefix(3);
		return true;
	}

	bool Stop;
	const auto Next = colors::ExtractColorInNewFormat(Str.cbegin() + 1, Str.cend(), color, Stop);
	if (Next != Str.cbegin() + 1)
	{
		Str = make_string_view(Next, Str.cend());
		return true;
	}

	return false;
}

static bool FastParseLine(string_view Str, int* const pLen, const int x0, const int realX, string* const pTopic, const wchar_t cColor)
{
	int x = x0, start_topic = -1;
	bool found = false;

	while (!Str.empty())
	{
		wchar_t wc = Str[0];
		Str.remove_prefix(1);
		if (!Str.empty() && wc == Str[0] && (wc == L'~' || wc == L'@' || wc == L'#' || wc == cColor))
			Str.remove_prefix(1);
		else if (wc == L'#') // start/stop highlighting
			continue;
		else if (cColor && wc == cColor)
		{
			if (Str.empty())
				break;

			if (Str[0] == L'-') // '\-' default color
			{
				Str.remove_prefix(1);
				continue;
			}

			if (Str.size() > 1 && std::iswxdigit(Str[0]) && std::iswxdigit(Str[1])) // '\hh' custom color
			{
				Str.remove_prefix(2);
				continue;
			}

			FarColor Color;
			bool Stop;
			const auto Next = colors::ExtractColorInNewFormat(Str.cbegin(), Str.cend(), Color, Stop);
			if (Next != Str.cbegin())
			{
				Str = make_string_view(Next, Str.cend());
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
				if (!Str.empty() && Str[0] == L'@')
					Str = SkipLink(Str.substr(1), found ? pTopic : nullptr);
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

	auto Str = HelpList[y].HelpStr.c_str();

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

int Help::StringLen(const string_view Str)
{
	int len = 0;
	FastParseLine(Str, &len, 0, -1, nullptr, strCtrlColorChar.empty()? 0 : strCtrlColorChar[0]);
	return len;
}

void Help::OutString(string_view Str)
{
	wchar_t OutStr[512]; //BUGBUG
	auto StartTopic = Str.data();
	int OutPos=0,Highlight=0,Topic=0;
	wchar_t cColor = strCtrlColorChar.empty()? 0 : strCtrlColorChar[0];

	while (OutPos<(int)(std::size(OutStr)-10))
	{
		if (Str.size() > 1 && (
		    (Str[0]==L'~' && Str[1]==L'~') ||
		    (Str[0]==L'#' && Str[1]==L'#') ||
		    (Str[0]==L'@' && Str[1]==L'@') ||
		    (cColor && Str[0]==cColor && Str[1]==cColor)
		))
		{
			OutStr[OutPos++] = Str[0];
			Str.remove_prefix(2);
			continue;
		}

		if (Str.empty() /*|| *Str==HelpBeginLink*/ || Str[0] == L'~' || ((Str[0] == L'#' || Str[0] == cColor) && !Topic))
		{
			OutStr[OutPos]=0;

			if (Topic)
			{
				int RealCurX = m_X1 + StackData->CurX + 1;
				int RealCurY = m_Y1 + StackData->CurY + HeaderHeight() + 1;
				bool found = WhereY() == RealCurY && RealCurX >= WhereX() && RealCurX < WhereX() + (Str.data() - StartTopic) - 1;

				SetColor(found ? COL_HELPSELECTEDTOPIC : COL_HELPTOPIC);
				if (Str.size() > 1 && Str[1]==L'@')
				{
					Str = SkipLink(Str.substr(2), found ? &StackData->strSelTopic : nullptr);
					Topic = 0;
				}
			}
			else
			{
				SetColor(Highlight ? colors::PaletteColorToFarColor(COL_HELPHIGHLIGHTTEXT) : CurColor);
			}

			/* $ 24.09.2001 VVM
			  ! Обрежем длинные строки при показе. Такое будет только при длинных ссылках... */
			if (static_cast<int>(wcslen(OutStr)) + WhereX() > m_X2)
				OutStr[m_X2 - WhereX()] = 0;

			Text(OutStr);

			OutPos=0;
		}

		if (Str.empty())
		{
			break;
		}

		if (Str[0] == L'~')
		{
			if (!Topic)
				StartTopic = Str.data();
			Topic = !Topic;
			Str.remove_prefix(1);
		}
		else if (Str[0] == L'@')
		{
			Str = SkipLink(Str.substr(1), nullptr);
		}
		else if (Str[0] ==L'#')
		{
			Highlight = !Highlight;
			Str.remove_prefix(1);
		}
		else if (!GetHelpColor(Str, cColor, CurColor))
		{
			OutStr[OutPos++] = Str[0];
			Str.remove_prefix(1);
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
	StackData->CurX = std::clamp(StackData->CurX, 0, CanvasWidth() - 1);

	if (StackData->CurY > BodyHeight() - 1)
	{
		StackData->TopStr += StackData->CurY - (BodyHeight() - 1);
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

bool Help::ProcessKey(const Manager::Key& Key)
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
			return true;
		}
		case KEY_ESC:
		case KEY_F10:
		{
			Hide();
			Global->WindowManager->DeleteWindow(shared_from_this());
			SetExitCode(XC_QUIT);
			return true;
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

			return true;
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

			return true;
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

			return true;
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

			return true;
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

			return true;
		}
		case KEY_MSWHEEL_DOWN:
		case KEY_MSWHEEL_DOWN | KEY_ALT:
		case KEY_MSWHEEL_DOWN | KEY_RALT:
		{
			int n = (LocalKey == KEY_MSWHEEL_DOWN ? (int)Global->Opt->MsWheelDeltaHelp : 1);
			while (n-- > 0)
				ProcessKey(Manager::Key(KEY_DOWN));

			return true;
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

			return true;
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
					return true;
				}
				else
					StackData->CurX=StackData->CurY=0;

				MoveToReference(1,1);
			}
			return true;
		}
		case KEY_RIGHT:   case KEY_NUMPAD6:   case KEY_MSWHEEL_RIGHT:
		case KEY_TAB:
		{
			MoveToReference(1,0);
			return true;
		}
		case KEY_LEFT:    case KEY_NUMPAD4:   case KEY_MSWHEEL_LEFT:
		case KEY_SHIFTTAB:
		{
			MoveToReference(0,0);
			return true;
		}
		case KEY_F1:
		{
			// не поганим SelTopic, если и так в Help on Help
			if (!equal_icase(StackData->strHelpTopic, HelpOnHelpTopic))
			{
				Stack.emplace(*StackData);
				IsNewTopic = true;
				JumpTopic(HelpOnHelpTopic);
				IsNewTopic = false;
				ErrorHelp = false;
			}

			return true;
		}
		case KEY_SHIFTF1:
		{
			//   не поганим SelTopic, если и так в теме Contents
			if (!equal_icase(StackData->strHelpTopic, HelpContents))
			{
				Stack.emplace(*StackData);
				IsNewTopic = true;
				JumpTopic(HelpContents);
				ErrorHelp = false;
				IsNewTopic = false;
			}

			return true;
		}
		case KEY_F7:
		{
			// не поганим SelTopic, если и так в FoundContents
			if (!equal_icase(StackData->strHelpTopic, FoundContents))
			{
				string strLastSearchStr0=strLastSearchStr;
				bool Case=LastSearchCase;
				bool WholeWords=LastSearchWholeWords;
				bool Regexp=LastSearchRegexp;

				string strTempStr;
				//int RetCode = GetString(msg(lng::MHelpSearchTitle),msg(lng::MHelpSearchingFor),L"HelpSearch",strLastSearchStr,strLastSearchStr0);
				int RetCode = GetSearchReplaceString(
					false,
					msg(lng::MHelpSearchTitle).c_str(),
					msg(lng::MHelpSearchingFor).c_str(),
					strLastSearchStr0,
					strTempStr,
					L"HelpSearch",
					{},
					&Case,
					&WholeWords,
					nullptr,
					&Regexp,
					nullptr,
					nullptr,
					true,
					&HelpSearchId);

				if (RetCode <= 0)
					return true;

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

			return true;

		}
		case KEY_SHIFTF2:
		{
			//   не поганим SelTopic, если и так в PluginContents
			if (!equal_icase(StackData->strHelpTopic, PluginContents))
			{
				Stack.emplace(*StackData);
				IsNewTopic = true;
				JumpTopic(PluginContents);
				ErrorHelp = false;
				IsNewTopic = false;
			}

			return true;
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
				return true;
			}

			return ProcessKey(Manager::Key(KEY_ESC));
		}
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (!StackData->strSelTopic.empty() && !equal_icase(StackData->strHelpTopic, StackData->strSelTopic))
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

			return true;
		}
	}

	return false;
}

bool Help::JumpTopic(string_view const Topic)
{
	assign(StackData->strSelTopic, Topic);
	return JumpTopic();
}

bool Help::JumpTopic()
{
	/* $ 14.07.2002 IS
	     При переходе по ссылкам используем всегда только абсолютные пути,
	     если это возможно.
	*/

	// Если ссылка на другой файл, путь относительный и есть то, от чего можно
	// вычислить абсолютный путь, то сделаем это
	if (starts_with(StackData->strSelTopic, HelpBeginLink) && !StackData->strHelpPath.empty())
	{
		const auto pos = StackData->strSelTopic.find(HelpEndLink, 2);
		if (pos != string::npos)
		{
			const auto Path = string_view(StackData->strSelTopic).substr(1, pos - 1);
			const auto EndSlash = IsSlash(Path.back());
			const auto FullPath = path::join(StackData->strHelpPath, Path);
			auto Topic = string_view(StackData->strSelTopic).substr(StackData->strSelTopic.find(HelpEndLink, 2) + 1);
			StackData->strSelTopic = format(EndSlash? HelpFormatLink : HelpFormatLinkModule, ConvertNameToFull(FullPath), Topic);
		}
	}

	//_SVS(SysLog(L"JumpTopic() = SelTopic=%s",StackData->SelTopic));
	// URL активатор - это ведь так просто :-)))
	// наверное подразумевается URL
	{
		const auto ColonPos = StackData->strSelTopic.find(L':');
		if (ColonPos != 0 && ColonPos != string::npos &&
			!(starts_with(StackData->strSelTopic, HelpBeginLink) && contains(StackData->strSelTopic, HelpEndLink))
			&& OpenURL(StackData->strSelTopic))
			return false;
	}
	// а вот теперь попробуем...

	//_SVS(SysLog(L"JumpTopic() = SelTopic=%s, StackData->HelpPath=%s",StackData->SelTopic,StackData->HelpPath));
	string strNewTopic;
	if (!StackData->strHelpPath.empty() && StackData->strSelTopic.front() !=HelpBeginLink && StackData->strSelTopic != HelpOnHelpTopic)
	{
		if (starts_with(StackData->strSelTopic, L':'))
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
					if (Pos3 != string::npos && !equal_icase(string_view(StackData->strHelpMask).substr(Pos3), L".hlf"sv))
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
	        (!equal_icase(StackData->strSelTopic, PluginContents) || !equal_icase(StackData->strSelTopic, FoundContents))
	   )
	{
		if (!(StackData->Flags&FHELP_CUSTOMFILE) && contains(strNewTopic, HelpEndLink))
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

		if (starts_with(StackData->strHelpTopic, HelpBeginLink))
		{
			const auto pos = StackData->strHelpTopic.rfind(HelpEndLink);
			if (pos != string::npos)
			{
				StackData->strHelpTopic.resize(pos+1);
				append(StackData->strHelpTopic, HelpContents);
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
			Message(MSG_WARNING,
				msg(lng::MHelpTitle),
				{
					msg(lng::MHelpTopicNotFound),
					StackData->strHelpTopic
				},
				{ lng::MOk });
		}

		return false;
	}

	// ResizeConsole();
	if (IsNewTopic || !(!equal_icase(StackData->strSelTopic, PluginContents) || !equal_icase(StackData->strSelTopic, FoundContents))) // Это неприятный костыль :-((
		MoveToReference(1,1);

	Global->WindowManager->RefreshWindow();
	return true;
}

bool Help::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (m_windowKeyBar->ProcessMouse(MouseEvent))
		return true;

	if (MouseEvent->dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED && MouseEvent->dwEventFlags!=MOUSE_MOVED)
	{
		ProcessKey(Manager::Key(KEY_ENTER));
		return true;
	}

	int prevMsX = MsX , prevMsY = MsY;
	MsX=MouseEvent->dwMousePosition.X;
	MsY=MouseEvent->dwMousePosition.Y;
	bool simple_move = (IntKeyState.MouseEventFlags == MOUSE_MOVED);


	if ((MsX<m_X1 || MsY<m_Y1 || MsX>m_X2 || MsY>m_Y2) && IntKeyState.MouseEventFlags != MOUSE_MOVED)
	{
		static const int HELPMODE_CLICKOUTSIDE = 0x20000000; // было нажатие мыши вне хелпа?

		if (m_Flags.Check(HELPMODE_CLICKOUTSIDE))
		{
			// Вываливаем если предыдущий эвент не был двойным кликом
			if (IntKeyState.PreMouseEventFlags != DOUBLE_CLICK)
				ProcessKey(Manager::Key(KEY_ESC));
		}

		if (MouseEvent->dwButtonState)
			m_Flags.Set(HELPMODE_CLICKOUTSIDE);

		return true;
	}

	if (IntKeyState.MouseX==m_X2 && (MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED))
	{
		int ScrollY = m_Y1 + HeaderHeight() + 1;

		if (IntKeyState.MouseY==ScrollY)
		{
			while (IsMouseButtonPressed())
				ProcessKey(Manager::Key(KEY_UP));

			return true;
		}

		if (IntKeyState.MouseY == ScrollY + BodyHeight() - 1)
		{
			while (IsMouseButtonPressed())
				ProcessKey(Manager::Key(KEY_DOWN));

			return true;
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

			return true;
		}
		simple_move = false;
	}

	// DoubliClock - свернуть/развернуть хелп.
	if (MouseEvent->dwEventFlags==DOUBLE_CLICK &&
	        (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
		MouseEvent->dwMousePosition.Y < m_Y1 + 1 + HeaderHeight())
	{
		ProcessKey(Manager::Key(KEY_F5));
		return true;
	}

	if (MouseEvent->dwMousePosition.Y < m_Y1 + 1 + HeaderHeight())
	{
		while (IsMouseButtonPressed() && IntKeyState.MouseY < m_Y1 + 1 + HeaderHeight())
			ProcessKey(Manager::Key(KEY_UP));

		return true;
	}

	if (MouseEvent->dwMousePosition.Y>=m_Y2)
	{
		while (IsMouseButtonPressed() && IntKeyState.MouseY>=m_Y2)
			ProcessKey(Manager::Key(KEY_DOWN));

		return true;
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
	return true;
}

bool Help::IsReferencePresent()
{
	CorrectPosition();
	int StrPos=FixCount+StackData->TopStr+StackData->CurY;

	if (StrPos >= static_cast<int>(HelpList.size()))
	{
		return false;
	}

	return contains(HelpList[StrPos].HelpStr, L"~@"sv);
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
			const auto ReferencePresent = IsReferencePresent();

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
	string strCurTopic, strEntryName;

	std::vector<RegExpMatch> m;
	MatchHash hm;
	RegExp re;

	if (LastSearchRegexp)
	{
		const auto strSlash = InsertRegexpQuote(strLastSearchStr);

		// Q: что важнее: опция диалога или опция RegExp`а?
		if (!re.Compile(strSlash.c_str(), OP_PERLSTYLE | OP_OPTIMIZE | (LastSearchCase? 0 : OP_IGNORECASE)))
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
		inplace::upper(strSearchStrUpper);
		inplace::lower(strSearchStrLower);
	}

	for (const auto& i: enum_file_lines(HelpFile, nCodePage))
	{
		auto Str = trim_right(i.Str);

		if ((!Str.empty() && Str[0] == L'@') &&
		    !(Str.size() > 1 && (Str[1] == L'+' || Str[1] == L'-')) &&
		    !contains(Str, L'='))// && !TopicFound)
		{
			strEntryName.clear();
			strCurTopic.clear();
			Str = trim(Str);
			if (!equal_icase(Str.substr(1), HelpContents))
			{
				assign(strCurTopic, Str);
				TopicFound=true;
			}
		}
		else if (TopicFound && Str.size() > 1 && Str[0] == L'$' && !strCurTopic.empty())
		{
			std::remove_copy(Str.begin() + 1, Str.end(), std::back_inserter(strEntryName), L'#');
		}

		if (TopicFound && !strEntryName.empty())
		{
			// !!!BUGBUG: необходимо "очистить" строку strReadStr от элементов разметки !!!
			string ReplaceStr;
			int CurPos=0;
			int SearchLength;

			if (SearchString(
				Str,
				strLastSearchStr,
				strSearchStrUpper,
				strSearchStrLower,
				re,
				m.data(),
				&hm,
				ReplaceStr,
				CurPos,
				LastSearchCase,
				LastSearchWholeWords,
				false,
				LastSearchRegexp,
				false,
				&SearchLength
			))
			{
				AddLine(concat(L"   ~"sv, strEntryName, L'~', strCurTopic, L'@'));
				strCurTopic.clear();
				strEntryName.clear();
				TopicFound=false;
			}
		}
	}

	AddLine({});
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
	const wchar_t *PtrTitle = nullptr;
	string_view ContentsName;

	switch (TypeIndex)
	{
		case HIDX_PLUGINS:
			PtrTitle = msg(lng::MPluginsHelpTitle).c_str();
			ContentsName = L"PluginContents"sv;
			break;
		default:
			throw MAKE_FAR_EXCEPTION(L"Unsupported index"sv);
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
				auto strPath = i->ModuleName();
				CutToSlash(strPath);
				const auto HelpFileData = OpenLangFile(strPath, Global->HelpFileMask, Global->Opt->strHelpLanguage);
				const auto& HelpFile = std::get<0>(HelpFileData);
				const auto HelpFileCodePage = std::get<2>(HelpFileData);
				if (HelpFile)
				{
					string strEntryName, strSecondParam;

					if (GetLangParam(HelpFile, ContentsName, strEntryName, &strSecondParam, HelpFileCodePage))
					{
						string strHelpLine = concat(L"   ~"sv, strEntryName);
						if (!strSecondParam.empty())
						{
							append(strHelpLine, L',', strSecondParam);
						}
						append(strHelpLine, L"~@"sv, MakeLink(strPath, HelpContents), L'@');

						AddLine(strHelpLine);
					}
				}
			});

			break;
		}
		default:
			throw MAKE_FAR_EXCEPTION(L"Unsupported index"sv);
	}

	// сортируем по алфавиту
	std::sort(HelpList.begin()+1, HelpList.end());
	// $ 26.06.2000 IS - Устранение глюка с хелпом по f1, shift+f2, end (решение предложил IG)
	AddLine({});
}

// Формирование топика с учетом разных факторов
bool Help::MkTopic(const Plugin* pPlugin, const string& HelpTopic, string &strTopic)
{
	strTopic.clear();

	if (!HelpTopic.empty())
	{
		if (HelpTopic.front() == L':')
		{
			strTopic.erase(0, 1);
		}
		else
		{
			if (pPlugin && HelpTopic.front() != HelpBeginLink)
			{
				strTopic = format(HelpFormatLinkModule, pPlugin->ModuleName(), HelpTopic);
			}
			else
			{
				strTopic = HelpTopic;
			}

			if (starts_with(strTopic, HelpBeginLink))
			{
				size_t EndPos = strTopic.find(HelpEndLink);

				if (EndPos == string::npos)
				{
					strTopic.clear();
				}
				else
				{
					if (EndPos == strTopic.size() - 1) // Вона как поперло то...
						append(strTopic, HelpContents); // ... значит покажем основную тему. //BUGBUG

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

static bool OpenURL(const string& URLPath)
{
	if (!Global->Opt->HelpURLRules)
		return false;

	string FilteredURLPath(URLPath);
	// удалим два идущих подряд ~~
	ReplaceStrings(FilteredURLPath, L"~~"sv, L"~"sv);
	// удалим два идущих подряд ##
	ReplaceStrings(FilteredURLPath, L"##"sv, L"#"sv);

	if (FilteredURLPath.empty())
		return false;

	if (Global->Opt->HelpURLRules > 1)
	{
		if (Message(MSG_WARNING,
			msg(lng::MHelpTitle),
			{
				msg(lng::MHelpActivatorURL),
				msg(lng::MHelpActivatorFormat),
				FilteredURLPath,
				L"\x01"s,
				msg(lng::MHelpActivatorQ)
			},
			{ lng::MYes, lng::MNo }) != Message::first_button)
			return false;
	}

	execute_info Info;
	Info.Command = FilteredURLPath;
	Info.SourceMode = execute_info::source_mode::known; // skip plugin prefixes processing
	Global->CtrlObject->CmdLine()->ExecString(Info);
	return true;
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
	strType = msg(lng::MHelpType);
	strName = strFullHelpPathName;
	return windowtype_help;
}
