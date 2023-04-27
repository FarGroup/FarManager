﻿/*
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "help.hpp"

// Internal:
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
#include "uuids.far.dialogs.hpp"
#include "RegExp.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "keybar.hpp"
#include "string_sort.hpp"
#include "cvtname.hpp"
#include "cmdline.hpp"
#include "global.hpp"
#include "window.hpp"
#include "log.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/from_string.hpp"
#include "common/view/select.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static const auto
	FoundContents   = L"__FoundContents__"sv,
	PluginContents  = L"__PluginContents__"sv,
	HelpOnHelpTopic = L":Help"sv,
	HelpContents    = L"Contents"sv,
	HelpMacroStart  = L"<!Macro:"sv,
	HelpMacroEnd    = L">"sv;

static const wchar_t HelpBeginLink = L'<';
static const wchar_t HelpEndLink = L'>';

static bool is_generated_topic(string_view const Topic)
{
	// I hate ADL
	return ::any_of(Topic, PluginContents, FoundContents);
}

enum HELPDOCUMENTSHELPTYPE
{
	HIDX_PLUGINS,                 // Индекс плагинов
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

	bool operator==(const HelpRecord& rhs) const
	{
		return equal_icase(HelpStr, rhs.HelpStr);
	}

	bool operator<(const HelpRecord& rhs) const
	{
		return string_sort::less(HelpStr, rhs.HelpStr);
	}
};

static bool OpenURL(string_view URLPath);

static const auto HelpFormatLink = FSTR(L"<{}\\>{}"sv);
static const auto HelpFormatLinkModule = FSTR(L"<{}>{}"sv);

class Help final: public window
{
	struct private_tag { explicit private_tag() = default; };

public:
	static help_ptr create(string_view Topic, string_view Mask, unsigned long long Flags);
	explicit Help(private_tag);

	bool  ProcessKey(const Manager::Key& Key) override;
	bool  ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	void InitKeyBar() override;
	void SetScreenPosition() override;
	void ResizeConsole() override;
	bool CanFastHide() const override;
	int GetTypeAndName(string &strType, string &strName) override;
	int GetType() const override { return windowtype_help; }
	long long VMProcess(int OpCode, void* vParam, long long iParam) override;
	bool IsKeyBarVisible() const override { return true; }

	bool GetError() const { return ErrorHelp; }

	struct StackHelpData;

private:
	void DisplayObject() override;
	string GetTitle() const override { return {}; }

	void init(string_view Topic, string_view Mask, unsigned long long Flags);
	bool ReadHelp(string_view Mask);
	void AddLine(string_view Line);
	void AddTitle(string_view Title);
	static void HighlightsCorrection(string &strStr);
	void FastShow();
	void DrawWindowFrame() const;
	void OutString(string_view Str);
	int  StringLen(string_view Str) const;
	void CorrectPosition() const;
	bool IsReferencePresent() const;
	bool GetTopic(int realX, int realY, string& strTopic) const;
	void MoveToReference(int Forward, int CurScreen);
	void ReadDocumentsHelp(int TypeIndex);
	void Search(const os::fs::file& HelpFile, uintptr_t nCodePage);
	bool JumpTopic(string_view Topic);
	bool JumpTopic();
	int CanvasHeight() const { return ObjHeight() - 1 - 1; }
	int HeaderHeight() const { return FixCount ? FixCount + 1 : 0; }
	int BodyHeight() const { return CanvasHeight() - HeaderHeight(); }
	int CanvasWidth() const { return ObjWidth() - 1 - 1; }

	std::unique_ptr<StackHelpData> StackData;
	std::stack<StackHelpData, std::vector<StackHelpData>> Stack; // стек возврата
	std::vector<HelpRecord> HelpList; // "хелп" в памяти.
	string  strFullHelpPathName;
	string strCurPluginContents; // помним PluginContents (для отображения в заголовке)
	string strCtrlStartPosChar;

	int FixCount{};             // количество строк непрокручиваемой области

	int MouseDownX{}, MouseDownY{}, BeforeMouseDownX{}, BeforeMouseDownY{};
	int MsX{-1}, MsY{-1};

	// символа - для атрибутов
	FarColor CurColor;             // CurColor - текущий цвет отрисовки
	unsigned CtrlTabSize;          // CtrlTabSize - опция! размер табуляции

	DWORD LastStartPos{};
	DWORD StartPos{};

	wchar_t m_CtrlColorChar{};

	bool MouseDown{};
	bool IsNewTopic{true};
	bool m_TopicFound{};
	bool ErrorHelp{true};

	SearchReplaceDlgParams LastSearchDlgParams;
};

struct Help::StackHelpData
{
	string strHelpMask;           // значение маски
	string strHelpPath;           // путь к хелпам
	string strHelpTopic;          // текущий топик
	string strSelTopic;           // выделенный топик (???)

	unsigned long long Flags{};   // флаги
	int TopStr{};                 // номер верхней видимой строки темы
	int CurX{}, CurY{};           // координаты (???)
};

static bool GetOptionsParam(const os::fs::file& LangFile, string_view const KeyName, string& Value, unsigned CodePage)
{
	return GetLangParam(LangFile, L"Options "sv + KeyName, Value, CodePage);
}

Help::Help(private_tag):
	StackData(std::make_unique<StackHelpData>()),
	CurColor(colors::PaletteColorToFarColor(COL_HELPTEXT)),
	CtrlTabSize(Global->Opt->HelpTabSize),
	LastSearchDlgParams
	{
		.CaseSensitive = Global->GlobalSearchCaseSensitive,
		.WholeWords = Global->GlobalSearchWholeWords,
		.Regexp = Global->Opt->EdOpt.SearchRegexp,
		.Fuzzy = Global->GlobalSearchFuzzy
	}
{
}

help_ptr Help::create(string_view const Topic, string_view const Mask, unsigned long long const Flags)
{
	auto HelpPtr = std::make_shared<Help>(private_tag());
	HelpPtr->init(Topic, Mask, Flags);
	return HelpPtr;
}

void Help::init(string_view const Topic, string_view const Mask, unsigned long long const Flags)
{
	m_windowKeyBar = std::make_unique<KeyBar>(shared_from_this());

	m_CanLoseFocus = false;
	SetRestoreScreenMode(true);

	StackData->Flags=Flags;
	StackData->strHelpMask = Mask; // сохраним маску файла
	StackData->strHelpTopic = Topic;

	if (Global->Opt->FullScreenHelp)
		SetPosition({ 0, 0, ScrX, ScrY });
	else
		SetPosition({ 4, 2, ScrX - 4, ScrY - 2 });

	if (!ReadHelp(StackData->strHelpMask) && (Flags&FHELP_USECONTENTS))
	{
		StackData->strHelpTopic = Topic;

		if (StackData->strHelpTopic.starts_with(HelpBeginLink))
		{
			const auto pos = StackData->strHelpTopic.rfind(HelpEndLink);

			if (pos != string::npos)
				StackData->strHelpTopic.resize(pos + 1);

			append(StackData->strHelpTopic, HelpContents);
		}

		StackData->strHelpPath.clear();
		ReadHelp(StackData->strHelpMask);
	}

	if (HelpList.empty())
		throw MAKE_FAR_KNOWN_EXCEPTION(concat(msg(lng::MHelpTopicNotFound), L'\n', StackData->strHelpTopic));

	InitKeyBar();
	SetMacroMode(MACROAREA_HELP);
	MoveToReference(1,1);
	Global->WindowManager->ExecuteWindow(shared_from_this()); //OT
	Global->WindowManager->ExecuteModal(shared_from_this()); //OT
}

bool Help::ReadHelp(string_view const Mask)
{
	string strPath;

	if (StackData->strHelpTopic.starts_with(HelpBeginLink))
	{
		strPath = StackData->strHelpTopic.substr(1);
		const auto pos = strPath.find(HelpEndLink);

		if (pos == string::npos)
			return false;

		StackData->strHelpTopic.assign(strPath, pos + 1);
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

	const auto [HelpFile, Name, HelpFileCodePage] = OpenLangFile(strPath, Mask.empty()? Global->HelpFileMask : Mask, Global->Opt->strHelpLanguage);
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
						string(Mask)
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
		unsigned UserTabSize;
		if (from_string(strReadStr, UserTabSize))
		{
			if (in_closed_range(0u, UserTabSize, 16u))
			{
				CtrlTabSize = UserTabSize;
			}
			else
			{
				LOGWARNING(L"CtrlTabSize value {} is out of range"sv, UserTabSize);
			}
		}
		else
		{
			LOGWARNING(L"CtrlTabSize value {} is invalid"sv, strReadStr);
		}
	}

	if (GetOptionsParam(HelpFile, L"CtrlColorChar"sv, strReadStr, HelpFileCodePage))
		m_CtrlColorChar = strReadStr.front();
	else
		m_CtrlColorChar = 0;

	if (GetOptionsParam(HelpFile, L"CtrlStartPosChar"sv, strReadStr, HelpFileCodePage))
		strCtrlStartPosChar = strReadStr;
	else
		strCtrlStartPosChar.clear();

	/* $ 29.11.2001 DJ
	   запомним, чего там написано в PluginContents
	*/
	if (!GetLangParam(HelpFile, L"PluginContents"sv, strCurPluginContents,  HelpFileCodePage))
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

	os::fs::filebuf StreamBuffer(HelpFile, std::ios::in);
	std::istream Stream(&StreamBuffer);
	Stream.exceptions(Stream.badbit | Stream.failbit); // BUGBUG, add try/catch

	enum_lines EnumFileLines(Stream, HelpFileCodePage);
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
				strReadStr = FileIterator->Str;
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

			const auto escape = [](string_view const Str)
			{
				string Result(Str);
				replace(Result, L"~"sv, L"~~"sv);
				replace(Result, L"#"sv, L"##"sv);
				replace(Result, L"@"sv, L"@@"sv);
				return Result;
			};

			size_t LastKeySize = 0;

			strReadStr = join(L"\n"sv, select(enum_tokens(strKeyName, L" "sv),
				[&](const auto& i)
				{
					LastKeySize = i.size();
					return concat(L" #"sv, escape(i), L'#');
				})
			);

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
					strReadStr.insert(PosTab, strTabSpace, 0, CtrlTabSize - (PosTab % CtrlTabSize));
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

		if (strReadStr.starts_with(L'@') && !BreakProcess)
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
					StackData->strHelpTopic.assign(strReadStr, 1 + n1 + 1);
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
				if (strReadStr.starts_with(HelpMacroStart) && Global->CtrlObject)
				{
					const auto PosTab = strReadStr.find(HelpMacroEnd);
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

				if (!(strReadStr.starts_with(L'$') && NearTopicFound && any_of(PrevSymbol, L'$', L'@')))
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

				if (strReadStr.starts_with(L'$') && NearTopicFound && any_of(PrevSymbol, L'$', L'@'))
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
						else if (StringLen(strReadStr)<RealMaxLength)
						{
							AddLine(strReadStr);
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
						const string UserSeparator{ L' ', (DrawLineChar ? DrawLineChar : BoxSymbols[BS_H1]), L' ' }; // left-center-right
						int Mul = (DrawLineChar == L'@' || DrawLineChar == L'~' || DrawLineChar == L'#' ? 2 : 1); // Double. See Help::OutString
						AddLine(MakeLine(CanvasWidth() * Mul - (Mul >> 1), line_type::h_user, UserSeparator));
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

					for (int I = static_cast<int>(strSplitLine.size()) - 1; I > 0; I--)
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
	const auto Width = StartPos && Line.starts_with(L' ')? StartPos - 1 : StartPos;
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
		m_windowKeyBar->SetPosition({ 0, ScrY, ScrX, ScrY });

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

	for (const auto& i: irange(CanvasHeight()))
	{
		int StrPos;

		if (i<FixCount)
		{
			StrPos=i;
		}
		else if (i==FixCount && FixCount>0)
		{
			GotoXY(m_Where.left, m_Where.top + i + 1);
			SetColor(COL_HELPBOX);
			DrawLine(ObjWidth(), line_type::h1_to_v2);
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

			if (OutStr.starts_with(L'^'))
			{
				OutStr.remove_prefix(1);
				GotoXY(m_Where.left + 1 + std::max(0, (CanvasWidth() - StringLen(OutStr)) / 2), m_Where.top + i + 1);
			}
			else
			{
				GotoXY(m_Where.left + 1, m_Where.top + i + 1);
			}

			OutString(OutStr);
		}
	}

	SetColor(COL_HELPSCROLLBAR);
	ScrollBar(m_Where.right, m_Where.top + HeaderHeight() + 1, BodyHeight(), StackData->TopStr, HelpList.size() - FixCount);
}

void Help::DrawWindowFrame() const
{
	SetScreen(m_Where, L' ', colors::PaletteColorToFarColor(COL_HELPTEXT));
	Box(m_Where, colors::PaletteColorToFarColor(COL_HELPBOX), DOUBLE_BOX);
	SetColor(COL_HELPBOXTITLE);
	const auto strHelpTitleBuf = truncate_right(concat(msg(lng::MHelpTitle), L" - "sv, strCurPluginContents.empty()? L"Far"s : strCurPluginContents), CanvasWidth() - 2);
	GotoXY(m_Where.left + (ObjWidth() - static_cast<int>(strHelpTitleBuf.size()) - 2) / 2, m_Where.top);
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
	// Note: even though we support 256 and true colors now,
	// this notation SHALL NOT not be extended due to backward compatibility.
	// Use ([[T]FFFFFFFF][:[T]BBBBBBBB]) to utilise extended colors.
		if (Str.size() > 2 && std::iswxdigit(Str[1]) && std::iswxdigit(Str[2]))
	{
		const auto Value = std::to_integer<unsigned>(HexStringToBlob(Str.substr(1, 2))[0]);
		color = colors::NtColorToFarColor(Value);
		Str.remove_prefix(3);
		return true;
	}

	bool Stop;
	const auto Tail = colors::ExtractColorInNewFormat(Str.substr(1), color, Stop);
	if (Tail.size() != Str.size() - 1)
	{
		Str = Tail;
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
		const auto wc = Str[0];
		Str.remove_prefix(1);
		if (Str.starts_with(wc) && any_of(wc, L'~', L'@', L'#', cColor))
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
			const auto Tail = colors::ExtractColorInNewFormat(Str, Color, Stop);
			if (Tail.size() != Str.size())
			{
				Str = Tail;
				continue;
			}
		}
		else if (wc == L'@') // skip topic link //??? is it valid without ~topic~
		{
			Str = SkipLink(Str, nullptr);
			continue;
		}
		else if (wc == L'~') // start/stop topic
		{
			if (start_topic < 0)
				start_topic = x;
			else
			{
				found = (realX >= start_topic && realX < x);
				if (Str.starts_with(L'@'))
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

bool Help::GetTopic(int realX, int realY, string& strTopic) const
{
	strTopic.clear();
	if (realY <= m_Where.top || realY >= m_Where.bottom || realX <= m_Where.left || realX >= m_Where.right)
		return false;

	int y = -1;
	if (realY - m_Where.top <= HeaderHeight())
	{
		if (y != FixCount)
			y = realY - m_Where.top - 1;
	}
	else
		y = realY - m_Where.top - 1 - HeaderHeight() + FixCount + StackData->TopStr;

	if (y < 0 || y >= static_cast<int>(HelpList.size()))
		return false;

	string_view Str = HelpList[y].HelpStr;

	if (Str.empty())
		return false;

	int x = m_Where.left + 1;
	if (Str.front() == L'^') // center
	{
		Str.remove_prefix(1);
		const auto w = StringLen(Str);
		x = m_Where.left + 1 + std::max(0, (CanvasWidth() - w) / 2);
	}

	return FastParseLine(Str, {}, x, realX, &strTopic, m_CtrlColorChar);
}

int Help::StringLen(const string_view Str) const
{
	int len = 0;
	FastParseLine(Str, &len, 0, -1, {}, m_CtrlColorChar);
	return len;
}

void Help::OutString(string_view Str)
{
	const auto MaxWidth = m_Where.width() - 4;

	string OutStr;
	OutStr.reserve(MaxWidth);

	auto StartTopic = Str.data();
	int Highlight = 0, Topic = 0;

	for (;;)
	{
		if (Str.size() > 1 && (
		    (Str[0]==L'~' && Str[1]==L'~') ||
		    (Str[0]==L'#' && Str[1]==L'#') ||
		    (Str[0]==L'@' && Str[1]==L'@') ||
		    (m_CtrlColorChar && Str[0] == m_CtrlColorChar && Str[1] == m_CtrlColorChar)
		))
		{
			OutStr.push_back(Str[0]);
			Str.remove_prefix(2);
			continue;
		}

		if (Str.empty() /*|| *Str==HelpBeginLink*/ || Str[0] == L'~' || ((Str[0] == L'#' || Str[0] == m_CtrlColorChar) && !Topic))
		{
			if (Topic)
			{
				const auto RealCurX = m_Where.left + StackData->CurX + 1;
				const auto RealCurY = m_Where.top + StackData->CurY + HeaderHeight() + 1;
				const auto found = WhereY() == RealCurY && RealCurX >= WhereX() && RealCurX < WhereX() + (Str.data() - StartTopic) - 1;

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

			Text(OutStr, MaxWidth - (WhereX() - m_Where.left - 2));

			OutStr.clear();
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
		else if (!GetHelpColor(Str, m_CtrlColorChar, CurColor))
		{
			OutStr.push_back(Str[0]);
			Str.remove_prefix(1);
		}
	}

	if (WhereX() < m_Where.right)
	{
		SetColor(CurColor);
		Text(string(m_Where.right - WhereX(), L' '));
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

	StackData->TopStr = std::max(0, std::min(StackData->TopStr, static_cast<int>(HelpList.size()) - BodyHeight()));
}

long long Help::VMProcess(int OpCode,void* vParam, long long iParam)
{
	switch (OpCode)
	{
		case MCODE_V_HELPFILENAME: // Help.FileName
			*static_cast<string*>(vParam) = strFullHelpPathName;     // ???
			break;
		case MCODE_V_HELPTOPIC: // Help.Topic
			*static_cast<string*>(vParam) = StackData->strHelpTopic;  // ???
			break;
		case MCODE_V_HELPSELTOPIC: // Help.SELTopic
			*static_cast<string*>(vParam) = StackData->strSelTopic;   // ???
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
			break;

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
			auto n = LocalKey == KEY_MSWHEEL_UP? Global->Opt->MsWheelDeltaHelp : 1;
			while (n-- > 0)
				ProcessKey(Manager::Key(KEY_UP));

			return true;
		}
		case KEY_MSWHEEL_DOWN:
		case KEY_MSWHEEL_DOWN | KEY_ALT:
		case KEY_MSWHEEL_DOWN | KEY_RALT:
		{
			auto n = LocalKey == KEY_MSWHEEL_DOWN? Global->Opt->MsWheelDeltaHelp : 1;
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
				const auto PrevTopStr = StackData->TopStr;
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
			if (StackData->strHelpTopic != HelpOnHelpTopic)
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
			if (StackData->strHelpTopic != HelpContents)
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
			if (StackData->strHelpTopic != FoundContents)
			{
				if (GetSearchReplaceString({}, LastSearchDlgParams, L"HelpSearch"sv, {}, CP_DEFAULT, {}, &HelpSearchId) == SearchReplaceDlgResult::Cancel)
					return true;

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
			if (StackData->strHelpTopic != PluginContents)
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
	StackData->strSelTopic = Topic;
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
	if (StackData->strSelTopic.starts_with(HelpBeginLink) && !StackData->strHelpPath.empty())
	{
		const auto pos = StackData->strSelTopic.find(HelpEndLink, 2);
		if (pos != string::npos)
		{
			const auto Path = string_view(StackData->strSelTopic).substr(1, pos - 1);
			const auto EndSlash = path::is_separator(Path.back());
			const auto FullPath = path::join(StackData->strHelpPath, Path);
			auto Topic = string_view(StackData->strSelTopic).substr(StackData->strSelTopic.find(HelpEndLink, 2) + 1);

			const auto SetSelTopic = [&](const auto FormatString)
			{
				StackData->strSelTopic = format(FormatString, ConvertNameToFull(FullPath), Topic);
			};

			EndSlash? SetSelTopic(HelpFormatLink) : SetSelTopic(HelpFormatLinkModule);
		}
	}

	// URL активатор - это ведь так просто :-)))
	// наверное подразумевается URL
	{
		const auto ColonPos = StackData->strSelTopic.find(L':');
		if (ColonPos != 0 && ColonPos != string::npos &&
			!(StackData->strSelTopic.starts_with(HelpBeginLink) && contains(StackData->strSelTopic, HelpEndLink))
			&& OpenURL(StackData->strSelTopic))
			return false;
	}
	// а вот теперь попробуем...

	string strNewTopic;
	if (
		!StackData->strHelpPath.empty() &&
		StackData->strSelTopic.front() != HelpBeginLink &&
		StackData->strSelTopic != HelpOnHelpTopic &&
		!is_generated_topic(StackData->strSelTopic)
	)
	{
		if (StackData->strSelTopic.starts_with(L':'))
		{
			strNewTopic = StackData->strSelTopic.substr(1);
			StackData->Flags&=~FHELP_CUSTOMFILE;
		}
		else if (StackData->Flags&FHELP_CUSTOMFILE)
			strNewTopic = StackData->strSelTopic;
		else
			strNewTopic = help::make_link(StackData->strHelpPath, StackData->strSelTopic);
	}
	else
	{
		strNewTopic = StackData->strSelTopic.substr(StackData->strSelTopic == HelpOnHelpTopic? 1 : 0);
	}

	// удалим ссылку на .DLL
	size_t EndPos = strNewTopic.rfind(HelpEndLink);

	if (EndPos != string::npos)
	{
		if (!path::is_separator(strNewTopic[EndPos - 1]))
		{
			const auto Pos2 = EndPos;

			while (EndPos != string::npos)
			{
				if (path::is_separator(strNewTopic[EndPos]))
				{
					StackData->Flags|=FHELP_CUSTOMFILE;
					StackData->strHelpMask = strNewTopic.substr(EndPos + 1);
					StackData->strHelpMask.resize(StackData->strHelpMask.find(HelpEndLink));

					strNewTopic.erase(EndPos, Pos2 - EndPos);

					const auto Pos3 = StackData->strHelpMask.rfind(L'.');
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

	if (StackData->strSelTopic.front() != L':' && !is_generated_topic(StackData->strSelTopic))
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

		if (StackData->strHelpTopic.starts_with(HelpBeginLink))
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
	if (IsNewTopic || is_generated_topic(StackData->strSelTopic)) // Это неприятный костыль :-((
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

	const auto prevMsX = MsX , prevMsY = MsY;
	MsX=MouseEvent->dwMousePosition.X;
	MsY=MouseEvent->dwMousePosition.Y;
	bool simple_move = IntKeyState.MouseEventFlags == MOUSE_MOVED;

	if (!simple_move && !m_Where.contains(MouseEvent->dwMousePosition))
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

	if (IntKeyState.MousePos.x == m_Where.right && MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
	{
		const auto ScrollY = m_Where.top + HeaderHeight() + 1;

		if (IntKeyState.MousePos.y == ScrollY)
		{
			while_mouse_button_pressed([&](DWORD const Button)
			{
				if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
					return false;

				ProcessKey(Manager::Key(KEY_UP));
				return true;
			});

			return true;
		}

		if (IntKeyState.MousePos.y == ScrollY + BodyHeight() - 1)
		{
			while_mouse_button_pressed([&](DWORD const Button)
			{
				if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
					return false;
				ProcessKey(Manager::Key(KEY_DOWN));
				return true;
			});

			return true;
		}
		simple_move = false;
	}

	/* $ 15.03.2002 DJ
	   обработаем щелчок в середине скроллбара
	*/
	if (IntKeyState.MousePos.x == m_Where.right)
	{
		const auto ScrollY = m_Where.top + HeaderHeight() + 1;

		if (static_cast<int>(HelpList.size()) > BodyHeight())
		{
			while (IsMouseButtonPressed() == FROM_LEFT_1ST_BUTTON_PRESSED)
			{
				if (IntKeyState.MousePos.y > ScrollY && IntKeyState.MousePos.y < ScrollY + BodyHeight() + 1)
				{
					StackData->CurX=StackData->CurY=0;
					StackData->TopStr = (IntKeyState.MousePos.y - ScrollY - 1) * (static_cast<int>(HelpList.size()) - FixCount - BodyHeight() + 1) / (BodyHeight() - 2);
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
		MouseEvent->dwMousePosition.Y < m_Where.top + 1 + HeaderHeight())
	{
		ProcessKey(Manager::Key(KEY_F5));
		return true;
	}

	/* $ 26.11.2001 VVM
	  + Запомнить нажатие клавиши мышки и только в этом случае реагировать при отпускании */
	if (!MouseEvent->dwEventFlags
	 && (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED)))
	{
		BeforeMouseDownX = StackData->CurX;
		BeforeMouseDownY = StackData->CurY;
		StackData->CurX = MouseDownX = MsX - m_Where.left - 1;
		StackData->CurY = MouseDownY = MsY - m_Where.top - 1 - HeaderHeight();
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
				StackData->CurX = MsX - m_Where.left - 1;
				StackData->CurY = MsY - m_Where.top - 1 - HeaderHeight();
			}
		}
	}

	if (while_mouse_button_pressed([&](DWORD)
	{
		if (MouseEvent->dwMousePosition.Y < m_Where.top + 1 + HeaderHeight())
		{
			ProcessKey(Manager::Key(KEY_UP));
			return true;
		}
		else if (IntKeyState.MousePos.y >= m_Where.bottom)
		{
			ProcessKey(Manager::Key(KEY_DOWN));
			return true;
		}
		return false;
	}))
	{
		return true;
	}

	FastShow();
	os::chrono::sleep_for(1ms);
	return true;
}

bool Help::IsReferencePresent() const
{
	CorrectPosition();
	const auto StrPos = FixCount + StackData->TopStr + StackData->CurY;

	if (StrPos >= static_cast<int>(HelpList.size()))
	{
		return false;
	}

	return contains(HelpList[StrPos].HelpStr, L"~@"sv);
}

void Help::MoveToReference(int Forward,int CurScreen)
{
	auto StartSelection = !StackData->strSelTopic.empty();
	const auto SaveCurX = StackData->CurX;
	const auto SaveCurY = StackData->CurY;
	const auto SaveTopStr = StackData->TopStr;
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
					StartSelection = false;
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
					StartSelection = false;
					StackData->CurX = CanvasWidth() - 1;
					StackData->CurY--;

					if (StackData->TopStr+StackData->CurY<0 ||
					        (CurScreen && StackData->CurY<0))
						break;
				}
			}

			FastShow();

			if (StackData->strSelTopic.empty())
				StartSelection = false;
			else
			{
				// небольшая заплата, артефакты есть но уже меньше :-)
				if (ReferencePresent && CurScreen)
					StartSelection = false;

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
	m_CtrlColorChar = 0;

	AddTitle(LastSearchDlgParams.SearchStr);

	bool TopicFound=false;
	string strCurTopic, strEntryName;

	std::vector<RegExpMatch> Match;
	named_regex_match NamedMatch;
	RegExp re;

	if (LastSearchDlgParams.Regexp.value())
	{
		// Q: что важнее: опция диалога или опция RegExp`а?
		try
		{
			re.Compile(
				LastSearchDlgParams.SearchStr,
				(LastSearchDlgParams.SearchStr.starts_with(L'/')? OP_PERLSTYLE : 0) | OP_OPTIMIZE | (LastSearchDlgParams.CaseSensitive.value()? 0 : OP_IGNORECASE));
		}
		catch (regex_exception const& e)
		{
			ReCompileErrorMessage(e, LastSearchDlgParams.SearchStr);
			return; //BUGBUG
		}
	}

	searchers Searchers;
	const auto& Searcher = init_searcher(Searchers, LastSearchDlgParams.CaseSensitive.value(), LastSearchDlgParams.Fuzzy.value(), LastSearchDlgParams.SearchStr);

	os::fs::filebuf StreamBuffer(HelpFile, std::ios::in);
	std::istream Stream(&StreamBuffer);
	Stream.exceptions(Stream.badbit | Stream.failbit); // BUGBUG, add try/catch

	for (const auto& i: enum_lines(Stream, nCodePage))
	{
		auto Str = trim_right(i.Str);

		if (Str.starts_with(L'@') &&
		    !(Str.size() > 1 && any_of(Str[1], L'+', L'-')) &&
		    !contains(Str, L'='))// && !TopicFound)
		{
			strEntryName.clear();
			strCurTopic.clear();
			Str = trim(Str);
			if (!equal_icase(Str.substr(1), HelpContents))
			{
				strCurTopic = Str;
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
			int CurPos=0;
			int SearchLength;

			if (SearchString(
				Str,
				LastSearchDlgParams.SearchStr,
				Searcher,
				re,
				Match,
				&NamedMatch,
				CurPos,
				{
					.CaseSensitive = LastSearchDlgParams.CaseSensitive.value(),
					.WholeWords = LastSearchDlgParams.WholeWords.value(),
					.Regexp = LastSearchDlgParams.Regexp.value()
				},
				SearchLength,
				Global->Opt->EdOpt.strWordDiv
			))
			{
				AddLine(concat(L"   ~"sv, strEntryName, L'~', strCurTopic, L'@'));
				strCurTopic.clear();
				strEntryName.clear();
				TopicFound=false;
			}
		}
	}

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
	m_CtrlColorChar = 0;
	string_view Title;
	string_view ContentsName;

	switch (TypeIndex)
	{
		case HIDX_PLUGINS:
			Title = msg(lng::MPluginsHelpTitle);
			ContentsName = L"PluginContents"sv;
			break;
		default:
			throw MAKE_FAR_FATAL_EXCEPTION(L"Unsupported index"sv);
	}

	AddTitle(Title);
	/* TODO:
	   1. Поиск (для "документов") не только в каталоге Documets, но
	      и в плагинах
	*/
	switch (TypeIndex)
	{
		case HIDX_PLUGINS:
		{
			for (const auto& i: *Global->CtrlObject->Plugins)
			{
				string_view Path = i->ModuleName();
				CutToSlash(Path);
				const auto [HelpFile, HelpLangName, HelpFileCodePage] = OpenLangFile(Path, Global->HelpFileMask, Global->Opt->strHelpLanguage);
				if (!HelpFile)
					continue;

				string strEntryName;
				if (!GetLangParam(HelpFile, ContentsName, strEntryName, HelpFileCodePage))
					continue;

				AddLine(format(FSTR(L"   ~{}~@{}@"sv), strEntryName, help::make_link(Path, HelpContents)));
			}

			break;
		}
		default:
			throw MAKE_FAR_FATAL_EXCEPTION(L"Unsupported index"sv);
	}

	// сортируем по алфавиту
	std::sort(HelpList.begin()+1, HelpList.end());
}

void Help::SetScreenPosition()
{
	if (Global->Opt->FullScreenHelp)
	{
		m_windowKeyBar->Hide();
		SetPosition({ 0, 0, ScrX, ScrY });
	}
	else
	{
		SetPosition({ 4, 2, ScrX - 4, ScrY - 2 });
	}

	Show();
}

void Help::InitKeyBar()
{
	m_windowKeyBar->SetLabels(lng::MHelpF1);
	m_windowKeyBar->SetCustomLabels(KBA_HELP);
}

static bool OpenURL(string_view const URLPath)
{
	if (!Global->Opt->HelpURLRules)
		return false;

	string FilteredURLPath(URLPath);
	// удалим два идущих подряд ~~
	replace(FilteredURLPath, L"~~"sv, L"~"sv);
	// удалим два идущих подряд ##
	replace(FilteredURLPath, L"##"sv, L"#"sv);

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
			{ lng::MYes, lng::MNo }) != message_result::first_button)
			return false;
	}

	execute_info Info;
	Info.DisplayCommand = FilteredURLPath;
	Info.Command = FilteredURLPath;
	Info.SourceMode = execute_info::source_mode::known_external; // skip plugin prefixes processing
	Global->CtrlObject->CmdLine()->ExecString(Info);
	return true;
}

void Help::ResizeConsole()
{
	const auto OldIsNewTopic = IsNewTopic;
	const auto ErrCannotOpenHelp = m_Flags.Check(FHELPOBJ_ERRCANNOTOPENHELP);
	m_Flags.Set(FHELPOBJ_ERRCANNOTOPENHELP);
	IsNewTopic = false;
	Hide();

	if (Global->Opt->FullScreenHelp)
	{
		m_windowKeyBar->Hide();
		SetPosition({ 0, 0, ScrX, ScrY });
	}
	else
		SetPosition({ 4, 2, ScrX - 4, ScrY - 2 });

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

namespace help
{
	bool show(string_view const Topic, string_view const Mask, unsigned long long const Flags)
	{
		try
		{
			return !Help::create(Topic, Mask, Flags)->GetError();
		}
		catch (far_exception const& e)
		{
			if (!(Flags & FHELP_NOSHOWERROR))
			{
				Message(MSG_WARNING, e,
					msg(lng::MHelpTitle),
					{
					},
					{ lng::MOk });
			}

			return false;
		}
	}

	string make_link(string_view const Path, string_view const Topic)
	{
		return concat(L'<', Path, L"\\>"sv, Topic);
	}

	string make_topic(const Plugin* const pPlugin, string_view const HelpTopic)
	{
		if (HelpTopic.empty())
			return {};

		if (HelpTopic.front() == L':')
			return string(HelpTopic.substr(1));

		auto Topic = pPlugin && HelpTopic.front() != HelpBeginLink?
			format(HelpFormatLinkModule, pPlugin->ModuleName(), HelpTopic) :
			string(HelpTopic);

		if (!Topic.starts_with(HelpBeginLink))
			return Topic;

		const auto EndPos = Topic.find(HelpEndLink);

		if (EndPos == string::npos)
			return {};

		if (EndPos == Topic.size() - 1) // Вона как поперло то...
			append(Topic, HelpContents); // ... значит покажем основную тему. //BUGBUG

		/* А вот теперь разгребем...
		   Формат может быть :
			 "<FullPath>Topic" или "<FullModuleName>Topic"
		   Для случая "FullPath" путь ДОЛЖЕН заканчиваться СЛЕШЕМ!
		   Т.о. мы отличим ЧТО ЭТО - имя модуля или путь!
		*/

		auto SlashPos = EndPos - 1;

		if (!path::is_separator(Topic[SlashPos])) // Это имя модуля?
		{
			// значит удалим это чертово имя :-)
			const auto Pos = FindLastSlash(Topic);
			if (Pos == string::npos)
				return {}; // ВО! Фигня какая-то :-(

			SlashPos = Pos;
		}

		return Topic.erase(SlashPos + 1, EndPos - SlashPos - 1);
	}
}
