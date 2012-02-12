/*
stddlg.cpp

Куча разных стандартных диалогов
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

#include "stddlg.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "dialog.hpp"
#include "ctrlobj.hpp"
#include "farexcpt.hpp"
#include "strmix.hpp"
#include "macro.hpp"
#include "keyboard.hpp"

int WINAPI GetSearchReplaceString(
    bool IsReplaceMode,
    string& SearchStr,
    string& ReplaceStr,
    const wchar_t *TextHistoryName,
    const wchar_t *ReplaceHistoryName,
    int& Case,
    int& WholeWords,
    int& Reverse,
    int& SelectFound,
    int& Regexp,
    const wchar_t *HelpTopic)
{
	int Result = 0;

	if (!TextHistoryName)
		TextHistoryName = L"SearchText";

	if (!ReplaceHistoryName)
		ReplaceHistoryName = L"ReplaceText";

	if (IsReplaceMode)
	{
		/*
		  0         1         2         3         4         5         6         7
		  0123456789012345678901234567890123456789012345678901234567890123456789012345
		00
		01   +----------------------------- Replace ------------------------------+
		02   | Search for                                                         |
		03   |                                                                    |
		04   | Replace with                                                       |
		05   |                                                                    |
		06   +--------------------------------------------------------------------+
		07   | [ ] Case sensitive                 [ ] Regular expressions         |
		08   | [ ] Whole words                                                    |
		09   | [ ] Reverse search                                                 |
		10   +--------------------------------------------------------------------+
		11   |                      [ Replace ]  [ Cancel ]                       |
		12   +--------------------------------------------------------------------+
		13
		*/
		FarDialogItem ReplaceDlgData[]=
		{
			{DI_DOUBLEBOX,3,1,72,12,0,nullptr,nullptr,0,MSG(MEditReplaceTitle)},
			{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(MEditSearchFor)},
			{DI_EDIT,5,3,70,3,0,TextHistoryName,nullptr,DIF_FOCUS|DIF_USELASTHISTORY|(*TextHistoryName?DIF_HISTORY:0),SearchStr},
			{DI_TEXT,5,4,0,4,0,nullptr,nullptr,0,MSG(MEditReplaceWith)},
			{DI_EDIT,5,5,70,5,0,ReplaceHistoryName,nullptr,(*ReplaceHistoryName?DIF_HISTORY:0)/*|DIF_USELASTHISTORY*/,ReplaceStr},
			{DI_TEXT,3,6,0,6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_CHECKBOX,5,7,0,7,Case,nullptr,nullptr,0,MSG(MEditSearchCase)},
			{DI_CHECKBOX,5,8,0,8,WholeWords,nullptr,nullptr,0,MSG(MEditSearchWholeWords)},
			{DI_CHECKBOX,5,9,0,9,Reverse,nullptr,nullptr,0,MSG(MEditSearchReverse)},
			{DI_CHECKBOX,40,7,0,7,Regexp,nullptr,nullptr,0,MSG(MEditSearchRegexp)},
			{DI_TEXT,3,10,0,10,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_BUTTON,0,11,0,11,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MEditReplaceReplace)},
			{DI_BUTTON,0,11,0,11,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MEditSearchCancel)},
		};
		MakeDialogItemsEx(ReplaceDlgData,ReplaceDlg);

		Dialog Dlg(ReplaceDlg,ARRAYSIZE(ReplaceDlgData));
		Dlg.SetPosition(-1,-1,76,14);

		if (HelpTopic && *HelpTopic)
			Dlg.SetHelp(HelpTopic);

		Dlg.Process();

		if(Dlg.GetExitCode() == 11)
		{
			Result = 1;
			SearchStr = ReplaceDlg[2].strData;
			ReplaceStr = ReplaceDlg[4].strData;
			Case=ReplaceDlg[6].Selected;
			WholeWords=ReplaceDlg[7].Selected;
			Reverse=ReplaceDlg[8].Selected;
			Regexp=ReplaceDlg[9].Selected;
		}
	}
	else
	{
		/*
		  0         1         2         3         4         5         6         7
		  0123456789012345678901234567890123456789012345678901234567890123456789012345
		00
		01   +------------------------------ Search ------------------------------+
		02   | Search for                                                         |
		03   |                                                                    |
		04   +--------------------------------------------------------------------+
		05   | [ ] Case sensitive                 [ ] Regular expressions         |
		06   | [ ] Whole words                    [ ] Select found                |
		07   | [ ] Reverse search                                                 |
		08   +--------------------------------------------------------------------+
		09   |                       [ Search ]  [ Cancel ]                       |
		10   +--------------------------------------------------------------------+
		*/
		FarDialogItem SearchDlgData[]=
		{
			{DI_DOUBLEBOX,3,1,72,10,0,nullptr,nullptr,0,MSG(MEditSearchTitle)},
			{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(MEditSearchFor)},
			{DI_EDIT,5,3,70,3,0,TextHistoryName,nullptr,DIF_FOCUS|DIF_USELASTHISTORY|(*TextHistoryName?DIF_HISTORY:0),SearchStr},
			{DI_TEXT,3,4,0,4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_CHECKBOX,5,5,0,5,Case,nullptr,nullptr,0,MSG(MEditSearchCase)},
			{DI_CHECKBOX,5,6,0,6,WholeWords,nullptr,nullptr,0,MSG(MEditSearchWholeWords)},
			{DI_CHECKBOX,5,7,0,7,Reverse,nullptr,nullptr,0,MSG(MEditSearchReverse)},
			{DI_CHECKBOX,40,5,0,5,Regexp,nullptr,nullptr,0,MSG(MEditSearchRegexp)},
			{DI_CHECKBOX,40,6,0,6,SelectFound,nullptr,nullptr,0,MSG(MEditSearchSelFound)},
			{DI_TEXT,3,8,0,8,0,nullptr,nullptr,DIF_SEPARATOR,L""},
			{DI_BUTTON,0,9,0,9,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MEditSearchSearch)},
			{DI_BUTTON,0,9,0,9,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MEditSearchAll)},
			{DI_BUTTON,0,9,0,9,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MEditSearchCancel)},
		};
		MakeDialogItemsEx(SearchDlgData,SearchDlg);

		Dialog Dlg(SearchDlg,ARRAYSIZE(SearchDlg));
		Dlg.SetPosition(-1,-1,76,12);

		if (HelpTopic && *HelpTopic)
			Dlg.SetHelp(HelpTopic);

		Dlg.Process();
		int ExitCode = Dlg.GetExitCode();

		if (ExitCode == 10 || ExitCode == 11)
		{
			Result = ExitCode == 10? 1 : 2;
			SearchStr = SearchDlg[2].strData;
			ReplaceStr.Clear();
			Case=SearchDlg[4].Selected;
			WholeWords=SearchDlg[5].Selected;
			Reverse=SearchDlg[6].Selected;
			Regexp=SearchDlg[7].Selected;
			SelectFound=SearchDlg[8].Selected;
		}
	}

	return Result;
}


// Функция для коррекции аля Shift-F4 Shift-Enter без отпускания Shift ;-)
static INT_PTR WINAPI GetStringDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	/*
	  if(Msg == DM_KEY)
	  {
	//    char KeyText[50];
	//    KeyToText(Param2,KeyText);
	//    _D(SysLog(L"%s (0x%08X) ShiftPressed=%d",KeyText,Param2,ShiftPressed));
	    if(ShiftPressed && (Param2 == KEY_ENTER||Param2 == KEY_NUMENTER) && !CtrlObject->Macro.IsExecuting())
	    {
	      DWORD Arr[1];
	      Arr[0]=Param2 == KEY_ENTER?KEY_SHIFTENTER:KEY_SHIFTNUMENTER;
	      SendDlgMessage(hDlg,Msg,Param1,(long)Arr);
	      return TRUE;
	    }
	  }
	*/
	return DefDlgProc(hDlg,Msg,Param1,Param2);
}


int WINAPI GetString(
    const wchar_t *Title,
    const wchar_t *Prompt,
    const wchar_t *HistoryName,
    const wchar_t *SrcText,
    string &strDestText,
    const wchar_t *HelpTopic,
    DWORD Flags,
    int *CheckBoxValue,
    const wchar_t *CheckBoxText,
    INT_PTR PluginNumber,
    const GUID* Id
)
{
	int Substract=5; // дополнительная величина :-)
	int ExitCode;
	bool addCheckBox=Flags&FIB_CHECKBOX && CheckBoxValue && CheckBoxText;
	int offset=addCheckBox?2:0;
	FarDialogItem StrDlgData[]=
	{
		{DI_DOUBLEBOX, 3, 1, 72, 4, 0, nullptr, nullptr, 0,                                L""},
		{DI_TEXT,      5, 2,  0, 2, 0, nullptr, nullptr, DIF_SHOWAMPERSAND,                L""},
		{DI_EDIT,      5, 3, 70, 3, 0, nullptr, nullptr, DIF_FOCUS|DIF_DEFAULTBUTTON|(Flags&FIB_EDITPATH?DIF_EDITPATH:0),L""},
		{DI_TEXT,      0, 4,  0, 4, 0, nullptr, nullptr, DIF_SEPARATOR,                    L""},
		{DI_CHECKBOX,  5, 5,  0, 5, 0, nullptr, nullptr, 0,                                L""},
		{DI_TEXT,      0, 6,  0, 6, 0, nullptr, nullptr, DIF_SEPARATOR,                    L""},
		{DI_BUTTON,    0, 7,  0, 7, 0, nullptr, nullptr, DIF_CENTERGROUP,                  L""},
		DI_BUTTON,    0, 7,  0, 7, 0, nullptr, nullptr, DIF_CENTERGROUP,                  L""
	};
	MakeDialogItemsEx(StrDlgData,StrDlg);

	if (addCheckBox)
	{
		Substract-=2;
		StrDlg[0].Y2+=2;
		StrDlg[4].Selected=(*CheckBoxValue)?TRUE:FALSE;
		StrDlg[4].strData = CheckBoxText;
	}

	if (Flags&FIB_BUTTONS)
	{
		Substract-=3;
		StrDlg[0].Y2+=2;
		StrDlg[2].Flags&=~DIF_DEFAULTBUTTON;
		StrDlg[5+offset].Y1=StrDlg[4+offset].Y1=5+offset;
		StrDlg[4+offset].Type=StrDlg[5+offset].Type=DI_BUTTON;
		StrDlg[4+offset].Flags=StrDlg[5+offset].Flags=DIF_CENTERGROUP;
		StrDlg[4+offset].Flags|=DIF_DEFAULTBUTTON;
		StrDlg[4+offset].strData = MSG(MOk);
		StrDlg[5+offset].strData = MSG(MCancel);
	}

	if (Flags&FIB_EXPANDENV)
	{
		StrDlg[2].Flags|=DIF_EDITEXPAND;
	}

	if (Flags&FIB_EDITPATH)
	{
		StrDlg[2].Flags|=DIF_EDITPATH;
	}

	if (HistoryName)
	{
		StrDlg[2].strHistory=HistoryName;
		StrDlg[2].Flags|=DIF_HISTORY|(Flags&FIB_NOUSELASTHISTORY?0:DIF_USELASTHISTORY);
	}

	if (Flags&FIB_PASSWORD)
		StrDlg[2].Type=DI_PSWEDIT;

	if (Title)
		StrDlg[0].strData = Title;

	if (Prompt)
	{
		StrDlg[1].strData = Prompt;
		TruncStrFromEnd(StrDlg[1].strData, 66);

		if (Flags&FIB_NOAMPERSAND)
			StrDlg[1].Flags&=~DIF_SHOWAMPERSAND;
	}

	if (SrcText)
		StrDlg[2].strData = SrcText;

	{
		Dialog Dlg(StrDlg,ARRAYSIZE(StrDlg)-Substract,GetStringDlgProc);
		Dlg.SetPosition(-1,-1,76,offset+((Flags&FIB_BUTTONS)?8:6));
		if(Id) Dlg.SetId(*Id);

		if (HelpTopic)
			Dlg.SetHelp(HelpTopic);

		Dlg.SetPluginOwner(reinterpret_cast<Plugin*>(PluginNumber));
#if 0

		if (Opt.ExceptRules)
		{
			__try
			{
				Dlg.Process();
			}
			__except(xfilter(EXCEPT_FARDIALOG,
			                 GetExceptionInformation(),nullptr,1)) // nullptr=???
			{
				return FALSE;
			}
		}
		else
#endif
		{
			Dlg.Process();
		}

		ExitCode=Dlg.GetExitCode();

		if (ExitCode == -2 && CtrlObject->Macro.IsExecuting() != MACROMODE_NOMACRO)
			CtrlObject->Macro.SendDropProcess();
	}

	if (ExitCode == 2 || ExitCode == 4 || (addCheckBox && ExitCode == 6))
	{
		if (!(Flags&FIB_ENABLEEMPTY) && StrDlg[2].strData.IsEmpty())
			return FALSE;

		strDestText = StrDlg[2].strData;

		if (addCheckBox)
			*CheckBoxValue=StrDlg[4].Selected;

		return TRUE;
	}

	return FALSE;
}

/*
  Стандартный диалог ввода пароля.
  Умеет сам запоминать последнего юзвера и пароль.

  Name      - сюда будет помещен юзвер (max 256 символов!!!)
  Password  - сюда будет помещен пароль (max 256 символов!!!)
  Title     - заголовок диалога (может быть nullptr)
  HelpTopic - тема помощи (может быть nullptr)
  Flags     - флаги (GNP_*)
*/
int WINAPI GetNameAndPassword(const wchar_t *Title, string &strUserName, string &strPassword,const wchar_t *HelpTopic,DWORD Flags)
{
	static string strLastName, strLastPassword;
	const wchar_t *HistoryName=L"NetworkUser";
	int ExitCode;
	/*
	  0         1         2         3         4         5         6         7
	  0123456789012345678901234567890123456789012345678901234567890123456789012345
	|0                                                                             |
	|1   +------------------------------- Title -------------------------------+   |
	|2   | User name                                                           |   |
	|3   | *******************************************************************|   |
	|4   | User password                                                       |   |
	|5   | ******************************************************************* |   |
	|6   +---------------------------------------------------------------------+   |
	|7   |                         [ Ok ]   [ Cancel ]                         |   |
	|8   +---------------------------------------------------------------------+   |
	|9                                                                             |
	*/
	FarDialogItem PassDlgData[]=
	{
		{DI_DOUBLEBOX,  3, 1,72, 8,0,nullptr,nullptr,0,NullToEmpty(Title)},
		{DI_TEXT,       5, 2, 0, 2,0,nullptr,nullptr,0,MSG(MNetUserName)},
		{DI_EDIT,       5, 3,70, 3,0,HistoryName,nullptr,DIF_FOCUS|DIF_USELASTHISTORY|DIF_HISTORY,(Flags&GNP_USELAST)?strLastName:strUserName},
		{DI_TEXT,       5, 4, 0, 4,0,nullptr,nullptr,0,MSG(MNetUserPassword)},
		{DI_PSWEDIT,    5, 5,70, 5,0,nullptr,nullptr,0,(Flags&GNP_USELAST)?strLastPassword:strPassword},
		{DI_TEXT,       3, 6, 0, 6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,     0, 7, 0, 7,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,     0, 7, 0, 7,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	MakeDialogItemsEx(PassDlgData,PassDlg);

	{
		Dialog Dlg(PassDlg,ARRAYSIZE(PassDlg));
		Dlg.SetPosition(-1,-1,76,10);

		if (HelpTopic)
			Dlg.SetHelp(HelpTopic);

		Dlg.Process();
		ExitCode=Dlg.GetExitCode();
	}

	if (ExitCode!=6)
		return FALSE;

	// запоминаем всегда.
	strUserName = PassDlg[2].strData;
	strLastName = strUserName;
	strPassword = PassDlg[4].strData;
	strLastPassword = strPassword;
	return TRUE;
}
