/*
macroapi.cpp

Macro API
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
#include "macroapi.hpp"

// Internal:
#include "clipboard.hpp"
#include "cmdline.hpp"
#include "console.hpp"
#include "ctrlobj.hpp"
#include "datetime.hpp"
#include "dirmix.hpp"
#include "dlgedit.hpp"
#include "elevation.hpp"
#include "fileedit.hpp"
#include "filelist.hpp"
#include "filemasks.hpp"
#include "filepanels.hpp"
#include "flink.hpp"
#include "history.hpp"
#include "global.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "macroopcode.hpp"
#include "macrovalues.hpp"
#include "panelmix.hpp"
#include "pathmix.hpp"
#include "plugapi.hpp"
#include "plugins.hpp"
#include "scrbuf.hpp"
#include "string_sort.hpp"
#include "strmix.hpp"
#include "treelist.hpp"
#include "tvar.hpp"
#include "uuids.far.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "xlat.hpp"

// Platform:
#include "platform.env.hpp"
#include "platform.memory.hpp"

// Common:
#include "common/from_string.hpp"
#include "common/scope_exit.hpp"
#include "common/uuid.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------
static bool ToDouble(long long v, double& d)
{
	if (constexpr long long Limit = bit(std::numeric_limits<double>::digits); v <= -Limit || v >= Limit)
		return false;

	d = static_cast<double>(v);
	return true;
}

static panel_ptr SelectPanel(long long const Which)
{
	switch (Which)
	{
	case 0:  return Global->CtrlObject->Cp()->ActivePanel();
	case 1:  return Global->CtrlObject->Cp()->PassivePanel();
	default: return {};
	}
}

class FarMacroApi
{
public:
	explicit FarMacroApi(FarMacroCall* Data) : mData(Data) {}

	std::vector<TVar> parseParams(size_t Count) const;

	void PushBoolean(bool Param) const          { SendValue(Param); }
	void PushPointer(void* Param) const         { SendValue(Param); }
	void PushValue(double Param) const          { SendValue(Param); }
	void PushValue(const wchar_t* Param) const  { SendValue(NullToEmpty(Param)); }
	void PushValue(const string& Param) const   { SendValue(Param.c_str()); }
	void PushValue(const char* Param) const     { SendValue(Param ? Param : ""); }
	void PushNil() const                        { SendValue(FMVT_NIL); }
	void PushTable() const                      { SendValue(FMVT_NEWTABLE); }
	void SetTable() const                       { SendValue(FMVT_SETTABLE); }
	void PushError(const wchar_t* str) const;
	void PushValue(long long Int) const;
	void PushValue(const TVar& Var) const;
	void SetField(const FarMacroValue &Key, const FarMacroValue &Value) const;

	template<typename T> requires std::integral<T> || std::is_enum_v<T>
	void PushValue(T const Value) const
	{
		return PushValue(static_cast<long long>(Value));
	}

	void absFunc() const;
	void ascFunc() const;
	void atoiFunc() const;
	void beepFunc() const;
	void chrFunc() const;
	void clipFunc() const;
	void dateFunc() const;
	void dlggetvalueFunc() const;
	void dlgsetfocusFunc() const;
	void editordellineFunc() const;
	void editorinsstrFunc() const;
	void editorposFunc() const;
	void editorselFunc() const;
	void editorsetFunc() const;
	void editorsetstrFunc() const;
	void editorsettitleFunc() const;
	void editorundoFunc() const;
	void environFunc() const;
	void farcfggetFunc() const;
	void fargetconfigFunc() const;
	void fattrFunc() const;
	void fexistFunc() const;
	void floatFunc() const;
	void flockFunc() const;
	void fmatchFunc() const;
	void fsplitFunc() const;
	void indexFunc() const;
	void intFunc() const;
	void itowFunc() const;
	void kbdLayoutFunc() const;
	void keyFunc() const;
	void keybarshowFunc() const;
	void lcaseFunc() const;
	void lenFunc() const;
	void maxFunc() const;
	void menushowFunc() const;
	void minFunc() const;
	void modFunc() const;
	void msgBoxFunc() const;
	void panelfattrFunc() const;
	void panelfexistFunc() const;
	void panelitemFunc() const;
	void panelselectFunc() const;
	void panelsetposFunc() const;
	void panelsetposidxFunc() const;
	void pluginexistFunc() const;
	void pluginloadFunc() const;
	void pluginunloadFunc() const;
	void promptFunc() const;
	void replaceFunc() const;
	void rindexFunc() const;
	void size2strFunc() const;
	void sleepFunc() const;
	void stringFunc() const;
	void strpadFunc() const;
	void strwrapFunc() const;
	void substrFunc() const;
	void testfolderFunc() const;
	void trimFunc() const;
	void ucaseFunc() const;
	void waitkeyFunc() const;
	void windowscrollFunc() const;
	void xlatFunc() const;

private:
	void fattrFuncImpl(int Type) const;
	void SendValue(const FarMacroValue &val) const;

	FarMacroCall* mData;
};

void FarMacroApi::SendValue(const FarMacroValue &val) const
{
	mData->Callback(mData->CallbackData, const_cast<FarMacroValue*>(&val), 1);
}

void FarMacroApi::PushError(const wchar_t *str) const
{
	FarMacroValue val(FMVT_ERROR);
	val.String = NullToEmpty(str);
	SendValue(val);
}

void FarMacroApi::PushValue(long long Int) const
{
	double Double;
	if (ToDouble(Int, Double))
		return PushValue(Double);

	SendValue(Int);
}

void FarMacroApi::PushValue(const TVar& Var) const
{
	if (Var.isDouble())
		PushValue(Var.asDouble());
	else if (Var.isString())
		PushValue(Var.asString());
	else
		PushValue(Var.asInteger());
}

void FarMacroApi::SetField(const FarMacroValue &Key, const FarMacroValue &Value) const
{
	SendValue(Key);
	SendValue(Value);
	SendValue(FMVT_SETTABLE);
}

std::vector<TVar> FarMacroApi::parseParams(size_t Count) const
{
	const auto argNum = std::min(mData->Count, Count);
	std::vector<TVar> Params;
	Params.reserve(Count);
	std::ranges::transform(mData->Values, mData->Values + argNum, std::back_inserter(Params), [](const auto& i)
	{
		switch (i.Type)
		{
			case FMVT_INTEGER: return TVar(i.Integer);
			case FMVT_BOOLEAN: return TVar(i.Boolean);
			case FMVT_DOUBLE: return TVar(i.Double);
			case FMVT_STRING: return TVar(i.String);
			default: return TVar();
		}
	});
	Params.resize(Count);
	return Params;
}

class LockOutput: noncopyable
{
public:
	explicit LockOutput(bool Lock): m_Lock(Lock) { if (m_Lock) Global->ScrBuf->Lock(); }
	~LockOutput() { if (m_Lock) Global->ScrBuf->Unlock(); }

private:
	const bool m_Lock;
};

static bool is_active_panel_code(intptr_t const Code)
{
	switch (Code)
	{
	case MCODE_C_APANEL_BOF:
	case MCODE_C_APANEL_EOF:
	case MCODE_C_APANEL_ISEMPTY:
	case MCODE_C_APANEL_SELECTED:
	case MCODE_C_APANEL_ROOT:
	case MCODE_C_APANEL_VISIBLE:
	case MCODE_C_APANEL_PLUGIN:
	case MCODE_C_APANEL_FILEPANEL:
	case MCODE_C_APANEL_FOLDER:
	case MCODE_C_APANEL_LEFT:
	case MCODE_C_APANEL_LFN:
	case MCODE_C_APANEL_FILTER:
	case MCODE_V_APANEL_CURRENT:
	case MCODE_V_APANEL_SELCOUNT:
	case MCODE_V_APANEL_PATH:
	case MCODE_V_APANEL_PATH0:
	case MCODE_V_APANEL_UNCPATH:
	case MCODE_V_APANEL_WIDTH:
	case MCODE_V_APANEL_TYPE:
	case MCODE_V_APANEL_ITEMCOUNT:
	case MCODE_V_APANEL_CURPOS:
	case MCODE_V_APANEL_OPIFLAGS:
	case MCODE_V_APANEL_DRIVETYPE:
	case MCODE_V_APANEL_HEIGHT:
	case MCODE_V_APANEL_COLUMNCOUNT:
	case MCODE_V_APANEL_HOSTFILE:
	case MCODE_V_APANEL_PREFIX:
	case MCODE_V_APANEL_FORMAT:
		return true;

	default:
		return false;
	}
}

void KeyMacro::CallFar(intptr_t CheckCode, FarMacroCall* Data)
{
	os::fs::attributes FileAttr = INVALID_FILE_ATTRIBUTES;
	FarMacroApi api(Data);

	// проверка на область
	if (CheckCode == 0)
	{
		return api.PushValue(Global->WindowManager->GetCurrentWindow()->GetMacroArea());
	}

	const auto ActivePanel = Global->CtrlObject->Cp()? Global->CtrlObject->Cp()->ActivePanel() : nullptr;
	const auto PassivePanel = Global->CtrlObject->Cp()? Global->CtrlObject->Cp()->PassivePanel() : nullptr;
	const auto& SelPanel = is_active_panel_code(CheckCode)? ActivePanel : PassivePanel;

	const auto CurrentWindow = Global->WindowManager->GetCurrentWindow();

	switch (CheckCode)
	{
	case MCODE_C_MSX:             return api.PushValue(GetMacroConst(constMsX));
	case MCODE_C_MSY:             return api.PushValue(GetMacroConst(constMsY));
	case MCODE_C_MSBUTTON:        return api.PushValue(GetMacroConst(constMsButton));
	case MCODE_C_MSCTRLSTATE:     return api.PushValue(GetMacroConst(constMsCtrlState));
	case MCODE_C_MSEVENTFLAGS:    return api.PushValue(GetMacroConst(constMsEventFlags));
	case MCODE_C_MSLASTCTRLSTATE: return api.PushValue(GetMacroConst(constMsLastCtrlState));

	case MCODE_V_FAR_WIDTH:
		return api.PushValue(ScrX + 1);

	case MCODE_V_FAR_HEIGHT:
		return api.PushValue(ScrY + 1);

	case MCODE_V_FAR_TITLE:
		return api.PushValue(console.GetTitle());

	case MCODE_V_FAR_PID:
		return api.PushValue(GetCurrentProcessId());

	case MCODE_V_FAR_UPTIME:
		return api.PushValue(Global->FarUpTime() / 1ms);

	case MCODE_V_MACRO_AREA:
		return api.PushValue(GetArea());

	case MCODE_C_FULLSCREENMODE:
		return api.PushBoolean(IsConsoleFullscreen());

	case MCODE_C_ISUSERADMIN:
		return api.PushBoolean(os::security::is_admin());

	case MCODE_V_DRVSHOWPOS:
		return api.PushValue(Global->Macro_DskShowPosType);

	case MCODE_V_DRVSHOWMODE:
		return api.PushValue(Global->Opt->ChangeDriveMode);

	case MCODE_C_CMDLINE_BOF:
	case MCODE_C_CMDLINE_EOF:
	case MCODE_C_CMDLINE_EMPTY:
	case MCODE_C_CMDLINE_SELECTED:
		return api.PushBoolean(Global->CtrlObject->CmdLine() && Global->CtrlObject->CmdLine()->VMProcess(CheckCode));

	case MCODE_V_CMDLINE_ITEMCOUNT:
	case MCODE_V_CMDLINE_CURPOS:
		return api.PushValue(Global->CtrlObject->CmdLine()? Global->CtrlObject->CmdLine()->VMProcess(CheckCode) : -1);

	case MCODE_V_CMDLINE_VALUE:
		return api.PushValue(Global->CtrlObject->CmdLine()? Global->CtrlObject->CmdLine()->GetString() : L""s);

	case MCODE_C_APANEL_ROOT:
	case MCODE_C_PPANEL_ROOT:
		return api.PushBoolean(SelPanel && SelPanel->VMProcess(MCODE_C_ROOTFOLDER));

	case MCODE_C_APANEL_BOF:
	case MCODE_C_PPANEL_BOF:
	case MCODE_C_APANEL_EOF:
	case MCODE_C_PPANEL_EOF:
		return api.PushBoolean(SelPanel && SelPanel->VMProcess(CheckCode == MCODE_C_APANEL_BOF || CheckCode == MCODE_C_PPANEL_BOF? MCODE_C_BOF : MCODE_C_EOF));

	case MCODE_C_SELECTED:
		{
			if (!CurrentWindow)
				return api.PushBoolean(false);

			const auto NeedType = [&]
			{
				switch (m_Area)
				{
				case MACROAREA_EDITOR: return windowtype_editor;
				case MACROAREA_VIEWER: return windowtype_viewer;
				case MACROAREA_DIALOG: return windowtype_dialog;
				default:               return windowtype_panels;
				}
			}();

			if (none_of(m_Area, MACROAREA_USERMENU, MACROAREA_MAINMENU, MACROAREA_MENU) && CurrentWindow->GetType() == NeedType)
			{
				const auto Result =
					m_Area == MACROAREA_SHELL && Global->CtrlObject->CmdLine()->IsVisible()?
						Global->CtrlObject->CmdLine()->VMProcess(CheckCode):
						CurrentWindow->VMProcess(CheckCode) ;

				return api.PushBoolean(Result != 0);
			}

			return api.PushBoolean(CurrentWindow->VMProcess(CheckCode) != 0);
		}

	case MCODE_C_EMPTY:
	case MCODE_C_BOF:
	case MCODE_C_EOF:
		{
			if (!CurrentWindow)
				return api.PushBoolean(true);

			if (
				none_of(m_Area, MACROAREA_USERMENU, MACROAREA_MAINMENU, MACROAREA_MENU) &&
				CurrentWindow->GetType() == windowtype_panels &&
				none_of(GetArea(), MACROAREA_INFOPANEL, MACROAREA_QVIEWPANEL, MACROAREA_TREEPANEL))
			{
				return api.PushBoolean(CheckCode == MCODE_C_EMPTY?
					Global->CtrlObject->CmdLine()->GetString().empty() :
					Global->CtrlObject->CmdLine()->VMProcess(CheckCode) != 0
				);
			}

			return api.PushBoolean(CurrentWindow->VMProcess(CheckCode) != 0);
		}

	case MCODE_V_DLGITEMCOUNT:
	case MCODE_V_DLGCURPOS:
	case MCODE_V_DLGITEMTYPE:
	case MCODE_V_DLGPREVPOS:
	case MCODE_V_DLGINFOID:
	case MCODE_V_DLGINFOOWNER:
		{
			auto ActualWindow = CurrentWindow;

			if (CurrentWindow)
			{
				if (CurrentWindow->GetType() == windowtype_menu)
				{
					if (const auto idx = Global->WindowManager->IndexOf(CurrentWindow); idx > 0)
						ActualWindow = Global->WindowManager->GetWindow(idx - 1);
				}
				else if (CurrentWindow->GetType() == windowtype_combobox)
					ActualWindow = std::static_pointer_cast<VMenu>(CurrentWindow)->GetDialog();
			}

			const auto IsStringType = any_of(CheckCode, MCODE_V_DLGINFOID, MCODE_V_DLGINFOOWNER);

			if (!ActualWindow || ActualWindow->GetType() != windowtype_dialog)
				return IsStringType?
					api.PushValue(L"") :
					api.PushValue(0);

			return IsStringType?
				api.PushValue(view_as<const wchar_t*>(ActualWindow->VMProcess(CheckCode))) :
				api.PushValue(ActualWindow->VMProcess(CheckCode));
		}

	case MCODE_C_APANEL_VISIBLE:
	case MCODE_C_PPANEL_VISIBLE:
		return api.PushBoolean(SelPanel && SelPanel->IsVisible());

	case MCODE_C_APANEL_ISEMPTY:
	case MCODE_C_PPANEL_ISEMPTY:
		{
			if (!SelPanel)
				return api.PushBoolean(true);

			const auto GetFileCount = SelPanel->GetFileCount();
			if (!GetFileCount)
				return api.PushBoolean(true);

			string Filename;
			if (!SelPanel->GetFileName(Filename, SelPanel->GetCurrentPos(), FileAttr))
				return api.PushBoolean(true);

			return api.PushBoolean(GetFileCount == 1 && IsParentDirectory(Filename));
		}

	case MCODE_C_APANEL_FILTER:
	case MCODE_C_PPANEL_FILTER:
		return api.PushBoolean(SelPanel && SelPanel->VMProcess(MCODE_C_APANEL_FILTER));

	case MCODE_C_APANEL_LFN:
	case MCODE_C_PPANEL_LFN:
		return api.PushBoolean(SelPanel && !SelPanel->GetShowShortNamesMode());

	case MCODE_C_APANEL_LEFT:
	case MCODE_C_PPANEL_LEFT:
		return api.PushBoolean(SelPanel && SelPanel == Global->CtrlObject->Cp()->LeftPanel());

	case MCODE_C_APANEL_FILEPANEL:
	case MCODE_C_PPANEL_FILEPANEL:
		return api.PushBoolean(SelPanel && SelPanel->GetType() == panel_type::FILE_PANEL);

	case MCODE_C_APANEL_PLUGIN:
	case MCODE_C_PPANEL_PLUGIN:
		return api.PushBoolean(SelPanel && SelPanel->GetMode() == panel_mode::PLUGIN_PANEL);

	case MCODE_C_APANEL_FOLDER:
	case MCODE_C_PPANEL_FOLDER:
		{
			if (!SelPanel)
				return api.PushBoolean(false);

			string Filename;
			if (!SelPanel->GetFileName(Filename, SelPanel->GetCurrentPos(), FileAttr))
				return api.PushBoolean(false);

			return api.PushBoolean(os::fs::is_directory(FileAttr));
		}

	case MCODE_C_APANEL_SELECTED:
	case MCODE_C_PPANEL_SELECTED:
		return api.PushBoolean(SelPanel && SelPanel->GetRealSelCount() > 0);

	case MCODE_V_APANEL_CURRENT:
	case MCODE_V_PPANEL_CURRENT:
		{
			if (!SelPanel)
				return api.PushValue(L"");

			string Filename;
			if (!SelPanel->GetFileName(Filename, SelPanel->GetCurrentPos(), FileAttr))
				return api.PushValue(L"");

			return api.PushValue(Filename);
		}

	case MCODE_V_APANEL_SELCOUNT:
	case MCODE_V_PPANEL_SELCOUNT:
		return api.PushValue(SelPanel? SelPanel->GetRealSelCount() : 0);

	case MCODE_V_APANEL_COLUMNCOUNT:
	case MCODE_V_PPANEL_COLUMNCOUNT:
		return api.PushValue(SelPanel? SelPanel->GetColumnsCount() : 0);

	case MCODE_V_APANEL_WIDTH:
	case MCODE_V_PPANEL_WIDTH:
	case MCODE_V_APANEL_HEIGHT:
	case MCODE_V_PPANEL_HEIGHT:
		{
			if (!SelPanel)
				return api.PushValue(0);

			const auto PanelRect = SelPanel->GetPosition();
			return api.PushValue(CheckCode == MCODE_V_APANEL_HEIGHT || CheckCode == MCODE_V_PPANEL_HEIGHT? PanelRect.height() : PanelRect.width());
		}

	case MCODE_V_APANEL_OPIFLAGS:
	case MCODE_V_PPANEL_OPIFLAGS:
	case MCODE_V_APANEL_HOSTFILE:
	case MCODE_V_PPANEL_HOSTFILE:
	case MCODE_V_APANEL_FORMAT:
	case MCODE_V_PPANEL_FORMAT:
		{
			const auto IsStringType = none_of(CheckCode, MCODE_V_APANEL_OPIFLAGS, MCODE_V_PPANEL_OPIFLAGS);

			if (!SelPanel || SelPanel->GetMode() != panel_mode::PLUGIN_PANEL)
				return IsStringType? api.PushValue(L"") : api.PushValue(0);

			OpenPanelInfo Info{ sizeof(Info) };
			SelPanel->GetOpenPanelInfo(&Info);

			switch (CheckCode)
			{
				case MCODE_V_APANEL_OPIFLAGS:
				case MCODE_V_PPANEL_OPIFLAGS:
					return api.PushValue(Info.Flags);

				case MCODE_V_APANEL_HOSTFILE:
				case MCODE_V_PPANEL_HOSTFILE:
					return api.PushValue(Info.HostFile);

				case MCODE_V_APANEL_FORMAT:
				case MCODE_V_PPANEL_FORMAT:
					return api.PushValue(Info.Format);

				default:
					std::unreachable();
			}
		}

	case MCODE_V_APANEL_PREFIX:
	case MCODE_V_PPANEL_PREFIX:
		{
			if (!SelPanel)
				return api.PushValue(L"");

			PluginInfo PInfo{ sizeof(PInfo) };
			if (!SelPanel->VMProcess(MCODE_V_APANEL_PREFIX, &PInfo))
				return api.PushValue(L"");

			return api.PushValue(PInfo.CommandPrefix);
		}

	case MCODE_V_APANEL_PATH0:
	case MCODE_V_PPANEL_PATH0:
		{
			if (!SelPanel)
				return api.PushValue(L"");

			return api.PushValue(SelPanel->GetCurDir());
		}

	case MCODE_V_APANEL_PATH:
	case MCODE_V_PPANEL_PATH:
		{
			if (!SelPanel)
				return api.PushValue(L"");

			string Filename;

			if (SelPanel->GetMode() == panel_mode::PLUGIN_PANEL)
			{
				OpenPanelInfo Info{ sizeof(Info) };
				SelPanel->GetOpenPanelInfo(&Info);
				Filename = NullToEmpty(Info.CurDir);
			}
			else
			{
				Filename = SelPanel->GetCurDir();
			}

			DeleteEndSlash(Filename); // - чтобы у корня диска было C:, тогда можно писать так: APanel.Path + "\\file"

			return api.PushValue(Filename);
		}

	case MCODE_V_APANEL_UNCPATH:
	case MCODE_V_PPANEL_UNCPATH:
		{
			string Filename;
			if (MakePath(SelPanel, false, true, false, Filename))
			{
				DeleteEndSlash(Filename);
			}
			return api.PushValue(Filename);
		}

	case MCODE_V_APANEL_TYPE:
	case MCODE_V_PPANEL_TYPE:
			return api.PushValue(SelPanel? SelPanel->GetType() : panel_type::FILE_PANEL);

	case MCODE_V_APANEL_DRIVETYPE:
	case MCODE_V_PPANEL_DRIVETYPE:
		{
			if (!SelPanel || SelPanel->GetMode() == panel_mode::PLUGIN_PANEL)
				return api.PushValue(-1);

			return api.PushValue(os::fs::drive::get_type(GetPathRoot(SelPanel->GetCurDir())));
		}

	case MCODE_V_APANEL_ITEMCOUNT:
	case MCODE_V_PPANEL_ITEMCOUNT:
			return api.PushValue(SelPanel? SelPanel->GetFileCount() : 0);

	case MCODE_V_APANEL_CURPOS:
	case MCODE_V_PPANEL_CURPOS:
			return api.PushValue(SelPanel? SelPanel->GetCurrentPos() + (SelPanel->GetFileCount()? 1 : 0) : 0);

	case MCODE_V_TITLE:
		{
			string Filename;

			if (std::dynamic_pointer_cast<FilePanels>(CurrentWindow) && ActivePanel)
			{
				Filename = ActivePanel->GetTitle();
			}
			else if (CurrentWindow)
			{
				string strType;

				switch (CurrentWindow->GetTypeAndName(strType, Filename))
				{
					case windowtype_editor:
					case windowtype_viewer:
						Filename = CurrentWindow->GetTitle();
						break;
				}
			}
			inplace::trim(Filename);
			return api.PushValue(Filename);
		}

	case MCODE_V_HEIGHT:
	case MCODE_V_WIDTH:
		{
			if (!CurrentWindow)
				return api.PushValue(0);

			const auto WindowRect = CurrentWindow->GetPosition();
			return api.PushValue(CheckCode == MCODE_V_HEIGHT? WindowRect.height() : WindowRect.width());
		}

	case MCODE_V_MENU_VALUE:
	case MCODE_V_MENUINFOID:
		{
			const auto CurArea = GetArea();

			if (CheckCode == MCODE_V_MENUINFOID && CurrentWindow && CurrentWindow->GetType() == windowtype_menu)
			{
				return api.PushValue(view_as<const wchar_t*>(CurrentWindow->VMProcess(MCODE_V_DLGINFOID)));
			}

			if (IsMenuArea(CurArea) || CurArea == MACROAREA_DIALOG)
			{
				if (CurrentWindow)
				{
					string Value;

					switch(CheckCode)
					{
					case MCODE_V_MENU_VALUE:
						if (CurrentWindow->VMProcess(CheckCode, &Value))
						{
							Value = trim(remove_highlight(Value));
							return api.PushValue(Value);
						}
						break;

					case MCODE_V_MENUINFOID:
						return api.PushValue(view_as<const wchar_t*>(CurrentWindow->VMProcess(CheckCode)));
					}
				}
			}

			return api.PushValue(L"");
		}

	case MCODE_V_ITEMCOUNT:
	case MCODE_V_CURPOS:
		return api.PushValue(CurrentWindow? CurrentWindow->VMProcess(CheckCode) : 0);

	case MCODE_V_EDITORCURLINE:
	case MCODE_V_EDITORSTATE:
	case MCODE_V_EDITORLINES:
	case MCODE_V_EDITORCURPOS:
	case MCODE_V_EDITORREALPOS:
	case MCODE_V_EDITORFILENAME:
	case MCODE_V_EDITORSELVALUE:
		{
			if (GetArea() == MACROAREA_EDITOR)
			{
				if (const auto CurrentEditor = Global->WindowManager->GetCurrentEditor(); CurrentEditor && CurrentEditor->IsVisible())
				{
					switch (CheckCode)
					{
					case MCODE_V_EDITORFILENAME:
						{
							string Type, Filename;
							CurrentEditor->GetTypeAndName(Type, Filename);
							return api.PushValue(Filename);
						}

					case MCODE_V_EDITORSELVALUE:
						{
							string Value;
							CurrentEditor->VMProcess(CheckCode, &Value);
							return api.PushValue(Value);
						}

					default:
						return api.PushValue(CurrentEditor->VMProcess(CheckCode));
					}
				}
			}

			return CheckCode == MCODE_V_EDITORSELVALUE? api.PushValue(L"") : api.PushValue(0);
		}

	case MCODE_V_HELPFILENAME:
	case MCODE_V_HELPTOPIC:
	case MCODE_V_HELPSELTOPIC:
		{
			string Value;
			if (GetArea() != MACROAREA_HELP || !CurrentWindow->VMProcess(CheckCode, &Value, 0))
				return api.PushValue(L"");

			return api.PushValue(Value);
		}

	case MCODE_V_VIEWERFILENAME:
		{
			if (none_of(GetArea(), MACROAREA_VIEWER, MACROAREA_QVIEWPANEL))
				return api.PushValue(L"");

			const auto CurrentViewer = Global->WindowManager->GetCurrentViewer();
			if (!CurrentViewer || !CurrentViewer->IsVisible())
				return api.PushValue(L"");

			return api.PushValue(CurrentViewer->GetFileName());
		}

	case MCODE_V_VIEWERSTATE:
		{
			if (none_of(GetArea(), MACROAREA_VIEWER, MACROAREA_QVIEWPANEL))
				return api.PushValue(0);

			const auto CurrentViewer = Global->WindowManager->GetCurrentViewer();
			if (!CurrentViewer || !CurrentViewer->IsVisible())
				return api.PushValue(0);

			return api.PushValue(CurrentViewer->VMProcess(CheckCode));
		}

	// =========================================================================
	// Functions
	// =========================================================================

	case MCODE_F_ABS:             return api.absFunc();
	case MCODE_F_ASC:             return api.ascFunc();
	case MCODE_F_ATOI:            return api.atoiFunc();
	case MCODE_F_BEEP:            return api.beepFunc();
	case MCODE_F_CHR:             return api.chrFunc();
	case MCODE_F_CLIP:            return api.clipFunc();
	case MCODE_F_DATE:            return api.dateFunc();
	case MCODE_F_DLG_GETVALUE:    return api.dlggetvalueFunc();
	case MCODE_F_DLG_SETFOCUS:    return api.dlgsetfocusFunc();
	case MCODE_F_EDITOR_DELLINE:  return api.editordellineFunc();
	case MCODE_F_EDITOR_INSSTR:   return api.editorinsstrFunc();
	case MCODE_F_EDITOR_POS:      return api.editorposFunc();
	case MCODE_F_EDITOR_SEL:      return api.editorselFunc();
	case MCODE_F_EDITOR_SET:      return api.editorsetFunc();
	case MCODE_F_EDITOR_SETSTR:   return api.editorsetstrFunc();
	case MCODE_F_EDITOR_SETTITLE: return api.editorsettitleFunc();
	case MCODE_F_EDITOR_UNDO:     return api.editorundoFunc();
	case MCODE_F_ENVIRON:         return api.environFunc();
	case MCODE_F_FAR_CFG_GET:     return api.farcfggetFunc();
	case MCODE_F_FAR_GETCONFIG:   return api.fargetconfigFunc();
	case MCODE_F_FATTR:           return api.fattrFunc();
	case MCODE_F_FEXIST:          return api.fexistFunc();
	case MCODE_F_FLOAT:           return api.floatFunc();
	case MCODE_F_FLOCK:           return api.flockFunc();
	case MCODE_F_FMATCH:          return api.fmatchFunc();
	case MCODE_F_FSPLIT:          return api.fsplitFunc();
	case MCODE_F_INDEX:           return api.indexFunc();
	case MCODE_F_INT:             return api.intFunc();
	case MCODE_F_ITOA:            return api.itowFunc();
	case MCODE_F_KBDLAYOUT:       return api.kbdLayoutFunc();
	case MCODE_F_KEY:             return api.keyFunc();
	case MCODE_F_KEYBAR_SHOW:     return api.keybarshowFunc();
	case MCODE_F_LCASE:           return api.lcaseFunc();
	case MCODE_F_LEN:             return api.lenFunc();
	case MCODE_F_MAX:             return api.maxFunc();
	case MCODE_F_MENU_SHOW:       return api.menushowFunc();
	case MCODE_F_MIN:             return api.minFunc();
	case MCODE_F_MOD:             return api.modFunc();
	case MCODE_F_MSGBOX:          return api.msgBoxFunc();
	case MCODE_F_PANEL_FATTR:     return api.panelfattrFunc();
	case MCODE_F_PANEL_FEXIST:    return api.panelfexistFunc();
	case MCODE_F_PANELITEM:       return api.panelitemFunc();
	case MCODE_F_PANEL_SELECT:    return api.panelselectFunc();
	case MCODE_F_PANEL_SETPOS:    return api.panelsetposFunc();
	case MCODE_F_PANEL_SETPOSIDX: return api.panelsetposidxFunc();
	case MCODE_F_PLUGIN_EXIST:    return api.pluginexistFunc();
	case MCODE_F_PLUGIN_LOAD:     return api.pluginloadFunc();
	case MCODE_F_PLUGIN_UNLOAD:   return api.pluginunloadFunc();
	case MCODE_F_REPLACE:         return api.replaceFunc();
	case MCODE_F_RINDEX:          return api.rindexFunc();
	case MCODE_F_SIZE2STR:        return api.size2strFunc();
	case MCODE_F_SLEEP:           return api.sleepFunc();
	case MCODE_F_STRING:          return api.stringFunc();
	case MCODE_F_STRPAD:          return api.strpadFunc();
	case MCODE_F_STRWRAP:         return api.strwrapFunc();
	case MCODE_F_SUBSTR:          return api.substrFunc();
	case MCODE_F_TESTFOLDER:      return api.testfolderFunc();
	case MCODE_F_TRIM:            return api.trimFunc();
	case MCODE_F_UCASE:           return api.ucaseFunc();

	case MCODE_F_WAITKEY:
	{
		SCOPED_ACTION(LockOutput)(IsTopMacroOutputDisabled());
		++m_WaitKey;
		SCOPE_EXIT{ --m_WaitKey; };
		return api.waitkeyFunc();
	}

	case MCODE_F_PLUGIN_CALL:
		{
			if(Data->Count < 2 || Data->Values[0].Type != FMVT_BOOLEAN || Data->Values[1].Type != FMVT_STRING)
				return api.PushBoolean(false);

			const auto SyncCall = Data->Values[0].Boolean == 0;
			const auto SysID = Data->Values[1].String;
			const auto Uuid = uuid::try_parse(SysID);

			if (!Uuid || !Global->CtrlObject->Plugins->FindPlugin(*Uuid))
				return api.PushBoolean(false);

			const auto Values = Data->Count > 2? Data->Values + 2 : nullptr;
			OpenMacroInfo Info{ sizeof(OpenMacroInfo), Data->Count - 2, Values };

			if (SyncCall)
				++m_InternalInput;

			void *Result = Global->CtrlObject->Plugins->CallPluginFromMacro(*Uuid, &Info);

			if (SyncCall)
				--m_InternalInput;

			if (os::memory::is_pointer(Result) && Result != INVALID_HANDLE_VALUE)
				return api.PushPointer(Result);

			return api.PushBoolean(Result != nullptr);
		}

	case MCODE_F_WINDOW_SCROLL:   return api.windowscrollFunc();
	case MCODE_F_XLAT:            return api.xlatFunc();
	case MCODE_F_PROMPT:          return api.promptFunc();

	case MCODE_F_CHECKALL:
		{
			if (Data->Count < 2)
				return api.PushBoolean(false);

			const auto Area = static_cast<FARMACROAREA>(static_cast<int>(Data->Values[0].Double));
			const auto Flags = static_cast<MACROFLAGS_MFLAGS>(static_cast<int>(Data->Values[1].Double));
			const auto Callback = (Data->Count >= 3 && Data->Values[2].Type == FMVT_POINTER)? std::bit_cast<FARMACROCALLBACK>(Data->Values[2].Pointer) : nullptr;
			const auto CallbackId = (Data->Count >= 4 && Data->Values[3].Type == FMVT_POINTER)? Data->Values[3].Pointer : nullptr;
			return api.PushBoolean(CheckAll(Area, Flags) && (!Callback || Callback(CallbackId, AKMFLAGS_NONE)));
		}

	case MCODE_F_GETOPTIONS:
		{
			DWORD Options = 0;
			if (Global->Opt->ReadOnlyConfig)                    Options |= 4_bit;
			return api.PushValue(Options);
		}

	case MCODE_F_USERMENU:
		return ShowUserMenu(Data->Count, Data->Values);

	case MCODE_F_SETCUSTOMSORTMODE:
			{
				if (Data->Count < 3 || Data->Values[0].Type != FMVT_DOUBLE || Data->Values[1].Type != FMVT_DOUBLE || Data->Values[2].Type != FMVT_BOOLEAN)
					return;

				auto panel = ActivePanel;
				if (panel && static_cast<int>(Data->Values[0].Double) == 1)
					panel = Global->CtrlObject->Cp()->GetAnotherPanel(panel);

				if (panel)
				{
					const auto SortMode = panel_sort{ static_cast<int>(Data->Values[1].Double) };
					const auto InvertByDefault = Data->Values[2].Boolean != 0;
					const auto Order = Data->Count < 4 || Data->Values[3].Type != FMVT_DOUBLE || !in_closed_range(static_cast<int>(sort_order::first), static_cast<int>(Data->Values[3].Double), static_cast<int>(sort_order::last))?
						sort_order::flip_or_default :
						sort_order{ static_cast<int>(Data->Values[3].Double) };

					panel->SetCustomSortMode(SortMode, Order, InvertByDefault);
				}
			}
			return;

	case MCODE_F_KEYMACRO:
			{
				if (!Data->Count || Data->Values[0].Type != FMVT_DOUBLE)
					return;

				switch (static_cast<int>(Data->Values[0].Double))
				{
					case IMP_RESTORE_MACROCHAR:
						RestoreMacroChar();
						break;
					case IMP_SCRBUF_LOCK:
						Global->ScrBuf->Lock();
						break;
					case IMP_SCRBUF_UNLOCK:
						Global->ScrBuf->Unlock();
						break;
					case IMP_SCRBUF_RESETLOCKCOUNT:
						Global->ScrBuf->ResetLockCount();
						break;
					case IMP_SCRBUF_GETLOCKCOUNT:
						api.PushValue(Global->ScrBuf->GetLockCount());
						break;
					case IMP_SCRBUF_SETLOCKCOUNT:
						if (Data->Count > 1) Global->ScrBuf->SetLockCount(Data->Values[1].Double);
						break;
					case IMP_GET_USEINTERNALCLIPBOARD:
						api.PushBoolean(default_clipboard_mode::get() == clipboard_mode::internal);
						break;
					case IMP_SET_USEINTERNALCLIPBOARD:
						if (Data->Count > 1) default_clipboard_mode::set(Data->Values[1].Boolean != 0?
							clipboard_mode::internal: clipboard_mode::system);
						break;
					case IMP_KEYNAMETOKEY:
						if (Data->Count > 1)
						{
							const auto Key = KeyNameToKey(Data->Values[1].String);
							api.PushValue(Key? Key : -1);
						}
						break;
					case IMP_KEYTOTEXT:
						if (Data->Count > 1)
						{
							api.PushValue(KeyToText(Data->Values[1].Double));
						}
						break;
				}
			}
			return;

		case MCODE_F_MACROSETTINGS:
			if (Data->Count>=4 && Data->Values[0].Type==FMVT_STRING && Data->Values[1].Type==FMVT_DOUBLE
				&& Data->Values[2].Type==FMVT_STRING && Data->Values[3].Type==FMVT_STRING)
			{
				const auto Key = KeyNameToKey(Data->Values[0].String);
				auto Flags = static_cast<unsigned long long>(Data->Values[1].Double);
				const auto Src = Data->Values[2].String;
				const auto Descr = Data->Values[3].String;
				if (Key && GetMacroSettings(Key, Flags, Src, Descr))
				{
					api.PushValue(Flags);
					api.PushValue(m_RecCode);
					api.PushValue(m_RecDescription);
					return;
				}
			}
			return api.PushBoolean(false);

		case MCODE_F_BM_ADD:              // N=BM.Add()
		case MCODE_F_BM_CLEAR:            // N=BM.Clear()
		case MCODE_F_BM_NEXT:             // N=BM.Next()
		case MCODE_F_BM_PREV:             // N=BM.Prev()
		case MCODE_F_BM_BACK:             // N=BM.Back()
		case MCODE_F_BM_STAT:             // N=BM.Stat([N])
		case MCODE_F_BM_DEL:              // N=BM.Del([Idx]) - удаляет закладку с указанным индексом (x=1...), 0 - удаляет текущую закладку
		case MCODE_F_BM_GET:              // N=BM.Get(Idx,M) - возвращает координаты строки (M==0) или колонки (M==1) закладки с индексом (Idx=1...)
		case MCODE_F_BM_GOTO:             // N=BM.Goto([n]) - переход на закладку с указанным индексом (0 --> текущую)
		case MCODE_F_BM_PUSH:             // N=BM.Push() - сохранить текущую позицию в виде закладки в конце стека
		case MCODE_F_BM_POP:              // N=BM.Pop() - восстановить текущую позицию из закладки в конце стека и удалить закладку
		{
			auto Params = api.parseParams(2);
			auto& p1 = Params[0];
			auto& p2 = Params[1];

			if (!CurrentWindow)
				return api.PushValue(0);

			return api.PushValue(CurrentWindow->VMProcess(CheckCode, ToPtr(p2.toInteger()), p1.toInteger()));
		}

		case MCODE_F_MENU_ITEMSTATUS:     // N=Menu.ItemStatus([N])
		case MCODE_F_MENU_GETVALUE:       // S=Menu.GetValue([N])
		case MCODE_F_MENU_GETHOTKEY:      // S=gethotkey([N])
		{
			auto Params = api.parseParams(1);
			TVar tmpVar=Params[0];

			tmpVar.toInteger();

			int CurArea=GetArea();

			if (IsMenuArea(CurArea) || CurArea == MACROAREA_DIALOG)
			{
				if (CurrentWindow)
				{
					long long MenuItemPos=tmpVar.asInteger()-1;
					if (CheckCode == MCODE_F_MENU_GETHOTKEY)
					{
						if (const auto Result = CurrentWindow->VMProcess(CheckCode, nullptr, MenuItemPos); Result)
						{
							const wchar_t value[]{ static_cast<wchar_t>(Result), 0 };
							tmpVar = value;
						}
						else
							tmpVar = L""sv;
					}
					else if (CheckCode == MCODE_F_MENU_GETVALUE)
					{
						string NewStr;
						if (CurrentWindow->VMProcess(CheckCode,&NewStr,MenuItemPos))
						{
							NewStr = trim(remove_highlight(NewStr));
							tmpVar=NewStr;
						}
						else
							tmpVar = L""sv;
					}
					else if (CheckCode == MCODE_F_MENU_ITEMSTATUS)
					{
						tmpVar = CurrentWindow->VMProcess(CheckCode, nullptr, MenuItemPos);
					}
				}
				else
					tmpVar = L""sv;
			}
			else
				tmpVar = L""sv;

			return api.PushValue(tmpVar);
		}

		case MCODE_F_MENU_SELECT:      // N=Menu.Select(S[,N[,Dir]])
		case MCODE_F_MENU_CHECKHOTKEY: // N=checkhotkey(S[,N])
		{
			auto Params = api.parseParams(3);
			long long Result=-1;
			long long tmpDir=0;

			if (CheckCode == MCODE_F_MENU_SELECT)
				tmpDir=Params[2].asInteger();

			long long tmpMode=Params[1].asInteger();

			if (CheckCode == MCODE_F_MENU_SELECT)
				tmpMode |= (tmpDir << 8);
			else
			{
				if (tmpMode > 0)
					tmpMode--;
			}

			const auto CurArea = GetArea();

			if (IsMenuArea(CurArea) || CurArea == MACROAREA_DIALOG)
			{
				if (CurrentWindow)
				{
					Result = CurrentWindow->VMProcess(CheckCode, UNSAFE_CSTR(Params[0].toString()), tmpMode);
				}
			}

			return api.PushValue(Result);
		}

		case MCODE_F_MENU_FILTER:      // N=Menu.Filter([Action[,Mode]])
		case MCODE_F_MENU_FILTERSTR:   // S=Menu.FilterStr([Action[,S]])
		{
			auto Params = api.parseParams(2);
			bool success=false;
			auto& tmpAction = Params[0];

			TVar tmpVar=Params[1];
			if (tmpAction.isUnknown())
				tmpAction=CheckCode == MCODE_F_MENU_FILTER ? 4 : 0;

			int CurArea=GetArea();

			if (IsMenuArea(CurArea) || CurArea == MACROAREA_DIALOG)
			{
				if (CurrentWindow)
				{
					if (CheckCode == MCODE_F_MENU_FILTER)
					{
						if (tmpVar.isUnknown())
							tmpVar = -1;
						tmpVar = CurrentWindow->VMProcess(CheckCode, std::bit_cast<void*>(static_cast<intptr_t>(tmpVar.toInteger())), tmpAction.toInteger());
						success=true;
					}
					else
					{
						string NewStr;
						if (tmpVar.isString())
							NewStr = tmpVar.toString();
						if (CurrentWindow->VMProcess(MCODE_F_MENU_FILTERSTR, &NewStr, tmpAction.toInteger()))
						{
							tmpVar=NewStr;
							success=true;
						}
					}
				}
			}

			if (!success)
			{
				if (CheckCode == MCODE_F_MENU_FILTER)
					tmpVar = -1;
				else
					tmpVar = L""sv;
			}

			return api.PushValue(tmpVar);
		}

		case MCODE_V_MENU_HORIZONTALALIGNMENT:
		{
			long long Result = -1;

			const auto CurArea = GetArea();

			if (IsMenuArea(CurArea) || CurArea == MACROAREA_DIALOG)
			{
				if (CurrentWindow)
				{
					Result = CurrentWindow->VMProcess(CheckCode);
				}
			}

			return api.PushValue(Result);
		}
	}
}

/* ------------------------------------------------------------------- */
// S=trim(S[,N])
void FarMacroApi::trimFunc() const
{
	auto Params = parseParams(2);

	auto Str = Params[0].toString();

	switch (static_cast<int>(Params[1].asInteger()))
	{
	case 0:
		inplace::trim(Str);
		break;

	case 1:
		inplace::trim_left(Str);
		break;

	case 2:
		inplace::trim_right(Str);
		break;

	default:
		break;
	}

	PushValue(Str);
}

// S=substr(S,start[,length])
void FarMacroApi::substrFunc() const
{
	/*
		TODO: http://bugs.farmanager.com/view.php?id=1480
			если start  >= 0, то вернётся подстрока, начиная со start-символа от начала строки.
			если start  <  0, то вернётся подстрока, начиная со start-символа от конца строки.
			если length >  0, то возвращаемая подстрока будет состоять максимум из length символов исходной строки начиная с start
			если length <  0, то в возвращаемой подстроке будет отсутствовать length символов от конца исходной строки, при том, что она будет начинаться с символа start.
								Или: length - длина того, что берем (если >=0) или отбрасываем (если <0).

			пустая строка возвращается:
				если length = 0
				если ...
	*/
	auto Params = parseParams(3);
	auto start = static_cast<int>(Params[1].asInteger());
	const auto& Str = Params[0].toString();
	const auto length_str = static_cast<int>(Str.size());
	auto length = Params[2].isUnknown()? length_str : static_cast<int>(Params[2].asInteger());

	if (length)
	{
		if (start < 0)
		{
			start=length_str+start;
			if (start < 0)
				start=0;
		}

		if (start >= length_str)
		{
			length=0;
		}
		else
		{
			if (length > 0)
			{
				if (start+length >= length_str)
					length=length_str-start;
			}
			else
			{
				length=length_str-start+length;

				if (length < 0)
				{
					length=0;
				}
			}
		}
	}

	length? PushValue(Str.substr(start, length)) : PushValue(L"");
}

static void SplitPath(string_view const FullPath, string& Dest, int Flags)
{
	size_t DirOffset = 0;
	const auto RootType = ParsePath(FullPath, &DirOffset);
	const auto Root = DeleteEndSlash(FullPath.substr(0, RootType == root_type::unknown? 0 : DirOffset));
	auto Path = FullPath.substr(Root.size());
	const auto FileName = PointToName(Path);
	Path.remove_suffix(FileName.size());
	const auto& [Name, Ext] = name_ext(FileName);

	const std::pair<int, string_view> Mappings[] =
	{
		{ 0_bit, Root },
		{ 1_bit, Path },
		{ 2_bit, Name },
		{ 3_bit, Ext  },
	};

	Dest.clear();

	for (const auto& [Flag, Part]: Mappings)
	{
		if (Flags & Flag)
			append(Dest, Part);
	}
}

// S=fsplit(S,N)
void FarMacroApi::fsplitFunc() const
{
	auto Params = parseParams(2);

	string strPath;
	SplitPath(Params[0].toString(), strPath, Params[1].asInteger());

	PushValue(strPath);
}

// N=atoi(S[,radix])
void FarMacroApi::atoiFunc() const
{
	auto Params = parseParams(2);
	long long Value = 0;
	int radix = static_cast<int>(Params[1].toInteger());

	if (radix == 0 || (radix >= 2 && radix <= 36))
		PushValue(from_string(Params[0].toString(), Value, nullptr, radix)? Value : 0);
	else
		PushValue(0);
}

// N=Window.Scroll(Lines[,Axis])
void FarMacroApi::windowscrollFunc() const
{
	if (!Global->Opt->WindowMode)
		return PushBoolean(false);

	const auto Params = parseParams(2);

	int Lines = static_cast<int>(Params[0].asInteger()), Columns = 0;

	if (Params[1].asInteger())
	{
		Columns=Lines;
		Lines=0;
	}

	PushBoolean(console.ScrollWindow(Lines, Columns));
}

// S=itoa(N[,radix])
void FarMacroApi::itowFunc() const
{
	auto Params = parseParams(2);

	if (Params[0].isInteger() || Params[0].isDouble())
	{
		wchar_t value[65];
		auto Radix = static_cast<int>(Params[1].toInteger());

		if (Radix < 2 || Radix > 36)
			Radix = 10;

		Params[0] = TVar(_i64tow(Params[0].toInteger(), value, Radix));
	}

	PushValue(Params[0]);
}

// os::chrono::sleep_for(Nms)
void FarMacroApi::sleepFunc() const
{
	const auto Params = parseParams(1);
	const auto Period = Params[0].asInteger();

	if (Period > 0)
	{
		os::chrono::sleep_for(std::chrono::milliseconds(Period));
		return PushValue(1);
	}

	PushValue(0);
}


// N=KeyBar.Show([N])
void FarMacroApi::keybarshowFunc() const
{
	/*
	Mode:
		0 - visible?
			ret: 0 - hide, 1 - show, -1 - KeyBar not found
		1 - show
		2 - hide
		3 - swap
		ret: prev mode or -1 - KeyBar not found
	*/
	const auto Params = parseParams(1);
	const auto f = Global->WindowManager->GetCurrentWindow();

	PushValue(f? f->VMProcess(MCODE_F_KEYBAR_SHOW, nullptr, Params[0].asInteger()) - 1 : -1);
}


// S=key(V)
void FarMacroApi::keyFunc() const
{
	const auto Params = parseParams(1);
	string strKeyText;

	if (Params[0].isInteger() || Params[0].isDouble())
	{
		if (Params[0].asInteger())
			strKeyText = KeyToText(static_cast<int>(Params[0].asInteger()));
	}
	else
	{
		// Проверим...
		if (KeyNameToKey(Params[0].asString()))
			strKeyText=Params[0].asString();
	}

	PushValue(strKeyText);
}

// V=waitkey([N,[T]])
void FarMacroApi::waitkeyFunc() const
{
	const auto Params = parseParams(2);
	const auto Type = static_cast<long>(Params[1].asInteger());

	std::optional<std::chrono::milliseconds> TimeoutOpt;
	if (const auto Timeout = static_cast<long>(Params[0].asInteger()))
		TimeoutOpt = Timeout * 1ms;

	const auto Key = WaitKey(static_cast<DWORD>(-1), TimeoutOpt);

	if (!Type)
	{
		string strKeyText;

		if (Key != KEY_NONE)
			strKeyText = KeyToText(Key);

		return PushValue(strKeyText);
	}

	PushValue(Key == KEY_NONE? -1 : Key);
}

// n=min(n1,n2)
void FarMacroApi::minFunc() const
{
	const auto Params = parseParams(2);
	PushValue(std::min(Params[0], Params[1]));
}

// n=max(n1,n2)
void FarMacroApi::maxFunc() const
{
	const auto Params = parseParams(2);
	PushValue(std::max(Params[0], Params[1]));
}

// n=mod(n1,n2)
void FarMacroApi::modFunc() const
{
	const auto Params = parseParams(2);

	const auto NumeratorType = Params[0].ParseType();
	const auto DenominatorType = Params[1].ParseType();

	TVar Result;

	switch(DenominatorType)
	{
	case TVar::Type::Unknown:
	case TVar::Type::Integer:
		if (const auto Denominator = Params[1].asInteger())
		{
			switch (NumeratorType)
			{
			case TVar::Type::Unknown:
			case TVar::Type::Integer:
				Result = Params[0].asInteger() % Denominator;
				break;

			case TVar::Type::Double:
				Result = std::fmod(Params[0].asDouble(), Denominator);
				break;

			default:
				break;
			}
		}
		break;

	case TVar::Type::Double:
		if (const auto Denominator = Params[1].asDouble())
		{
			switch (NumeratorType)
			{
			case TVar::Type::Unknown:
			case TVar::Type::Integer:
				Result = std::fmod(Params[0].asInteger(), Denominator);
				break;

			case TVar::Type::Double:
				Result = std::fmod(Params[0].asDouble(), Denominator);
				break;

			default:
				break;
			}
		}
		break;

	default:
		break;
	}

	PushValue(Result);
}

// N=index(S1,S2[,Mode])
void FarMacroApi::indexFunc() const
{
	auto Params = parseParams(3);
	const auto& s = Params[0].toString();
	const auto& p = Params[1].toString();

	const auto StrStr = [](const string& Str1, const string& Str2) { return std::ranges::search(Str1, Str2); };
	const auto StrStrI = [](const string& Str1, const string& Str2) { return std::ranges::search(Str1, Str2, string_comparer_icase{}); };

	const auto Result = Params[2].asInteger()? StrStr(s, p) : StrStrI(s, p);
	const auto Position = Result.empty()? -1 : Result.begin() - s.cbegin();
	PushValue(Position);
}

// S=rindex(S1,S2[,Mode])
void FarMacroApi::rindexFunc() const
{
	auto Params = parseParams(3);
	const auto& s = Params[0].toString();
	const auto& p = Params[1].toString();

	const auto RevStrStr = [](const string& Str1, const string& Str2) { return std::ranges::find_end(Str1, Str2); };
	const auto RevStrStrI = [](const string& Str1, const string& Str2) { return std::ranges::find_end(Str1, Str2, string_comparer_icase{}); };

	const auto Result = Params[2].asInteger()? RevStrStr(s, p) : RevStrStrI(s, p);
	const auto Position = Result.empty()? -1 : Result.begin() - s.cbegin();
	PushValue(Position);
}

static auto convert_size2str_flags(uint64_t const Flags)
{
	static constexpr std::pair<uint64_t, uint64_t> FlagsMap[]
	{
		{ 0x0010000000000000, COLFLAGS_SHOW_MULTIPLIER },
		{ 0x0800000000000000, COLFLAGS_GROUPDIGITS },
		{ 0x0080000000000000, COLFLAGS_FLOATSIZE },
		{ 0x0040000000000000, COLFLAGS_ECONOMIC },
		{ 0x0400000000000000, COLFLAGS_THOUSAND },
		{ 0x0020000000000000, COLFLAGS_USE_MULTIPLIER },
	};

	uint64_t InternalFlags = 0;

	for (const auto& [From, To]: FlagsMap)
	{
		if (Flags & From)
			InternalFlags |= To;
	}

	if (InternalFlags & COLFLAGS_USE_MULTIPLIER)
	{
		const auto Multiplier = Flags & COLFLAGS_MULTIPLIER_MASK;
		InternalFlags |= Multiplier;
	}

	return InternalFlags;
}

// S=Size2Str(Size,Flags[,Width])
void FarMacroApi::size2strFunc() const
{
	const auto Params = parseParams(3);
	const auto Size = as_unsigned(Params[0].asInteger());
	const auto Flags = convert_size2str_flags(Params[1].asInteger());
	const auto Width = static_cast<int>(Params[2].asInteger());

	PushValue(FileSizeToStr(Size, Width, Flags));
}

// S=date([S])
void FarMacroApi::dateFunc() const
{
	auto Params = parseParams(1);

	if (Params[0].isInteger() && !Params[0].asInteger())
		Params[0] = L""sv;

	PushValue(MkStrFTime(Params[0].toString()));
}

// S=xlat(S[,Flags])
/*
	Flags:
		XLAT_SWITCHKEYBLAYOUT  = 1
		XLAT_SWITCHKEYBBEEP    = 2
		XLAT_USEKEYBLAYOUTNAME = 4
*/
void FarMacroApi::xlatFunc() const
{
	auto Params = parseParams(2);
	auto StrParam = Params[0].toString();
	Xlat(StrParam, Params[1].asInteger());
	PushValue(StrParam);
}

// N=beep([N])
void FarMacroApi::beepFunc() const
{
	const auto Params = parseParams(1);
	PushBoolean(MessageBeep(static_cast<unsigned>(Params[0].asInteger())) != FALSE);
}

/*
Res=kbdLayout([N])

Параметр N:
а) конкретика: 0x0409 или 0x0419 или...
б) 1 - следующую системную (по кругу)
в) -1 - предыдущую системную (по кругу)
г) 0 или не указан - вернуть текущую раскладку.

Возвращает предыдущую раскладку (для N=0 текущую)
*/
// N=kbdLayout([N])
void FarMacroApi::kbdLayoutFunc() const
{
	const auto Params = parseParams(1);
	const auto dwLayout = static_cast<DWORD>(Params[0].asInteger());

	auto Ret = true;
	const auto RetLayout = console.GetKeyboardLayout();

	const auto hWnd = console.GetWindow();

	if (hWnd && dwLayout)
	{
		WPARAM WParam;
		LPARAM LParam;

		if (as_signed(dwLayout) == -1)
		{
			WParam = INPUTLANGCHANGE_BACKWARD;
			LParam = HKL_PREV;
		}
		else if (dwLayout == 1)
		{
			WParam = INPUTLANGCHANGE_FORWARD;
			LParam = HKL_NEXT;
		}
		else
		{
			WParam = 0;
			LParam = std::bit_cast<LPARAM>(os::make_hkl(dwLayout));
		}

		Ret = PostMessage(hWnd, WM_INPUTLANGCHANGEREQUEST, WParam, LParam) != FALSE;
	}

	PushValue(Ret? std::bit_cast<intptr_t>(RetLayout) : 0);
}

// S=prompt(["Title"[,"Prompt"[,flags[, "Src"[, "History"]]]]])
void FarMacroApi::promptFunc() const
{
	const auto Params = parseParams(5);
	const auto& ValHistory = Params[4];
	const auto& ValSrc = Params[3];
	const auto Flags = static_cast<DWORD>(Params[2].asInteger());
	const auto& ValPrompt = Params[1];
	const auto& ValTitle = Params[0];

	string_view title;
	if (!(ValTitle.isInteger() && ValTitle.asInteger() == 0))
		title = ValTitle.asString();

	string_view history;
	if (!(ValHistory.isInteger() && !ValHistory.asInteger()))
		history = ValHistory.asString();

	string_view src;
	if (!(ValSrc.isInteger() && ValSrc.asInteger() == 0))
		src = ValSrc.asString();

	string_view prompt;
	if (!(ValPrompt.isInteger() && ValPrompt.asInteger() == 0))
		prompt = ValPrompt.asString();

	string strDest;

	const auto oldHistoryDisable = GetHistoryDisableMask();

	// Mantis#0001743: Возможность отключения истории
	// если не указан history, то принудительно отключаем историю для ЭТОГО prompt()
	if (history.empty())
		SetHistoryDisableMask(1 << HISTORYTYPE_DIALOG);

	if (GetString(title, prompt, history, src, strDest, {}, (Flags&~FIB_CHECKBOX) | FIB_ENABLEEMPTY))
		PushValue(strDest);
	else
		PushBoolean(false);

	SetHistoryDisableMask(oldHistoryDisable);
}

// N=msgbox(["Title"[,"Text"[,flags]]])
void FarMacroApi::msgBoxFunc() const
{
	auto Params = parseParams(3);

	auto& ValT = Params[0];
	string_view title;
	if (!(ValT.isInteger() && !ValT.asInteger()))
		title = ValT.toString();

	auto& ValB = Params[1];
	string_view text;
	if (!(ValB.isInteger() && !ValB.asInteger()))
		text = ValB.toString();

	auto Flags = static_cast<DWORD>(Params[2].asInteger());
	Flags&=~(FMSG_KEEPBACKGROUND|FMSG_ERRORTYPE);
	Flags|=FMSG_ALLINONE;

	if (!extract_integer<WORD, 1>(Flags) || extract_integer<WORD, 1>(Flags) > extract_integer<WORD, 1>(FMSG_MB_RETRYCANCEL))
		Flags|=FMSG_MB_OK;

	const auto TempBuf = concat(title, L'\n', text);
	const auto Result = pluginapi::apiMessageFn(&FarUuid, &FarUuid, Flags, nullptr, std::bit_cast<const wchar_t* const*>(TempBuf.c_str()), 0, 0) + 1;
	PushValue(Result);
}

//S=Menu.Show(Items[,Title[,Flags[,FindOrFilter[,X[,Y]]]]])
//Flags:
//0x001 - BoxType
//0x002 - BoxType
//0x004 - BoxType
//0x008 - возвращаемый результат - индекс или строка
//0x010 - разрешена отметка нескольких пунктов
//0x020 - отсортировать (с учетом регистра)
//0x040 - убирать дублирующиеся пункты
//0x080 - автоматически назначать хоткеи |= VMENU_AUTOHIGHLIGHT
//0x100 - FindOrFilter - найти или отфильтровать
//0x200 - автоматическая нумерация строк
//0x400 - однократное выполнение цикла меню
//0x800 -
void FarMacroApi::menushowFunc() const
{
	auto Params = parseParams(6);
	auto& VY = Params[5];
	auto& VX = Params[4];
	auto& VFindOrFilter(Params[3]);
	const auto Flags = static_cast<DWORD>(Params[2].asInteger());
	auto& Title = Params[1];

	if (Title.isUnknown())
		Title = L""sv;

	auto strTitle = Title.toString();
	string strBottom;
	auto& Items = Params[0];
	auto strItems = Items.toString();
	replace(strItems, L"\r\n"sv, L"\n"sv);

	if (strItems.back() != L'\n')
		strItems += L'\n';

	TVar Result(-1);
	int BoxType = (Flags & 0x7)?(Flags & 0x7)-1:3;
	bool bResultAsIndex = (Flags & 0x08) != 0;
	bool bMultiSelect = (Flags & 0x010) != 0;
	bool bSorting = (Flags & 0x20) != 0;
	bool bPacking = (Flags & 0x40) != 0;
	bool bAutohighlight = (Flags & 0x80) != 0;
	bool bSetMenuFilter = (Flags & 0x100) != 0;
	bool bAutoNumbering = (Flags & 0x200) != 0;
	bool bExitAfterNavigate = (Flags & 0x400) != 0;
	int X = -1;
	int Y = -1;
	unsigned long long MenuFlags = VMENU_WRAPMODE;

	int nLeftShift=0;
	if (bAutoNumbering)
	{
		for (int numlines = std::ranges::count(strItems, L'\n'); numlines; numlines/=10)
		{
			nLeftShift++;
		}
		nLeftShift+=3;
	}

	if (!VX.isUnknown())
		X=VX.toInteger();

	if (!VY.isUnknown())
		Y=VY.toInteger();

	if (bAutohighlight)
		MenuFlags |= VMENU_AUTOHIGHLIGHT;

	int SelectedPos=0;
	int LineCount=0;
	size_t CurrentPos=0;
	replace(strTitle, L"\r\n"sv, L"\n"sv);
	auto PosLF = strTitle.find(L'\n');
	bool CRFound = PosLF != string::npos;

	if(CRFound)
	{
		strBottom=strTitle.substr(PosLF+1);
		strTitle=strTitle.substr(0,PosLF);
	}
	const auto Menu = VMenu2::create(strTitle, {}, ScrY - 4);
	Menu->SetBottomTitle(strBottom);
	Menu->SetMenuFlags(MenuFlags);
	Menu->SetPosition({ X, Y, 0, 0 });
	Menu->SetBoxType(BoxType);

	PosLF = strItems.find(L'\n');
	CRFound = PosLF != string::npos;

	while(CRFound)
	{
		size_t SubstrLen=PosLF-CurrentPos;

		if (SubstrLen==0)
			SubstrLen=1;

		menu_item_ex NewItem{ strItems.substr(CurrentPos, SubstrLen) };

		if (NewItem.Name != L"\n"sv)
		{
			const auto CharToFlag = [](wchar_t c)
			{
				switch (c)
				{
				case L'\x1': return LIF_SEPARATOR;
				case L'\x2': return LIF_CHECKED;
				case L'\x3': return LIF_DISABLE;
				case L'\x4': return LIF_GRAYED;
				default:     return LIF_NONE;
				}
			};

			const auto NewBegin = std::ranges::find_if(NewItem.Name, [&](wchar_t i)
			{
				const auto Flag = CharToFlag(i);
				NewItem.Flags |= Flag;
				return !Flag;
			});

			NewItem.Name.erase(NewItem.Name.cbegin(), NewBegin);
		}
		else
			NewItem.Name.clear();

		if (bAutoNumbering && !(bSorting || bPacking) && !(NewItem.Flags & LIF_SEPARATOR))
		{
			LineCount++;
			NewItem.Name = far::format(L"{:{}} - {}"sv, LineCount, nLeftShift - 3, NewItem.Name);
		}
		Menu->AddItem(NewItem);
		CurrentPos=PosLF+1;
		PosLF = strItems.find(L'\n', CurrentPos);
		CRFound = PosLF != string::npos;
	}

	if (bSorting)
	{
		Menu->SortItems([](const menu_item_ex& a, const menu_item_ex& b, const SortItemParam& Param)
		{
			if (a.Flags & LIF_SEPARATOR || b.Flags & LIF_SEPARATOR)
				return false;

			const auto
				strName1 = remove_highlight(a.Name),
				strName2 = remove_highlight(b.Name);

			const auto Less = string_sort::less(string_view(strName1).substr(Param.Offset), string_view(strName2).substr(Param.Offset));
			return Param.Reverse? !Less : Less;
		});
	}

	if (bPacking)
		Menu->Pack();

	if ((bAutoNumbering) && (bSorting || bPacking))
	{
		for (const auto i: std::views::iota(0, Menu->GetShowItemCount()))
		{
			auto& Item = Menu->at(i);
			if (!(Item.Flags & LIF_SEPARATOR))
			{
				LineCount++;
				Item.Name = far::format(L"{:{}} - {}"sv, LineCount, nLeftShift - 3, Item.Name);
			}
		}
	}

	if (!VFindOrFilter.isUnknown() && !bSetMenuFilter)
	{
		if (VFindOrFilter.isInteger() || VFindOrFilter.isDouble())
		{
			if (VFindOrFilter.toInteger()-1>=0)
				Menu->SetSelectPos(VFindOrFilter.toInteger() - 1, 1);
			else
				Menu->SetSelectPos(static_cast<int>(Menu->size() + VFindOrFilter.toInteger()), 1);
		}
		else
			if (VFindOrFilter.isString())
				Menu->SetSelectPos(std::max(0, Menu->FindItem(0, VFindOrFilter.toString())), 1);
	}

	window_ptr Window;

	int PrevSelectedPos=Menu->GetSelectPos();
	DWORD LastKey=0;

	Menu->Key(KEY_NONE);
	Menu->Run([&](const Manager::Key& RawKey)
	{
		const auto Key=RawKey();
		if (bSetMenuFilter && !VFindOrFilter.isUnknown())
		{
			bSetMenuFilter = false;
			string NewStr=VFindOrFilter.toString();
			Menu->VMProcess(MCODE_F_MENU_FILTERSTR, &NewStr, 1);
		}

		SelectedPos=Menu->GetSelectPos();
		LastKey=Key;
		int KeyProcessed = 1;
		switch (Key)
		{
			case KEY_NUMPAD0:
			case KEY_INS:
				if (bMultiSelect)
					Menu->GetCheck(SelectedPos)? Menu->ClearCheck(SelectedPos) : Menu->SetCheck(SelectedPos);
				break;

			case KEY_CTRLADD:
			case KEY_CTRLSUBTRACT:
			case KEY_CTRLMULTIPLY:
			case KEY_RCTRLADD:
			case KEY_RCTRLSUBTRACT:
			case KEY_RCTRLMULTIPLY:
				if (bMultiSelect)
				{
					for (const auto i: std::views::iota(0uz, Menu->size()))
					{
						if (Menu->at(i).Flags & MIF_HIDDEN)
							continue;

						if (any_of(Key, KEY_CTRLADD, KEY_RCTRLADD) || (any_of(Key, KEY_CTRLMULTIPLY, KEY_RCTRLMULTIPLY) && !Menu->GetCheck(static_cast<int>(i))))
							Menu->SetCheck(static_cast<int>(i));
						else
							Menu->ClearCheck(static_cast<int>(i));
					}
				}
				break;

			case KEY_CTRLA:
			case KEY_RCTRLA:
			{
				int ItemCount=Menu->GetShowItemCount();
				if(ItemCount>ScrY-4)
					ItemCount=ScrY-4;
				Menu->SetMaxHeight(ItemCount);
				break;
			}

			case KEY_BREAK:
				Menu->Close(-1);
				break;

			default:
				KeyProcessed = 0;
		}

		if (bExitAfterNavigate && (PrevSelectedPos!=SelectedPos))
		{
			SelectedPos=Menu->GetSelectPos();
			Menu->Close();
			return KeyProcessed;
		}

		PrevSelectedPos=SelectedPos;
		return KeyProcessed;
	});

	// ReSharper disable once CppTooWideScope
	wchar_t temp[65];

	if (Menu->GetExitCode() >= 0)
	{
		SelectedPos=Menu->GetExitCode();
		if (bMultiSelect)
		{
			string StrResult;

			for (const auto i: std::views::iota(0uz, Menu->size()))
			{
				if (Menu->GetCheck(static_cast<int>(i)))
				{
					if (bResultAsIndex)
					{
						StrResult += str(i + 1);
					}
					else
					{
						StrResult += string_view(Menu->at(i).Name).substr(nLeftShift);
					}

					StrResult += L"\n"sv;
				}
			}

			if (!StrResult.empty())
			{
				Result = StrResult;
			}
			else
			{
				if (bResultAsIndex)
				{
					_i64tow(SelectedPos+1,temp,10);
					Result=temp;
				}
				else
					Result = string_view(Menu->at(SelectedPos).Name).substr(nLeftShift);
			}
		}
		else
			if(!bResultAsIndex)
				Result = string_view(Menu->at(SelectedPos).Name).substr(nLeftShift);
			else
				Result=SelectedPos+1;
	}
	else
	{
		if (bExitAfterNavigate)
		{
			if (any_of(LastKey, KEY_ESC, KEY_F10, KEY_BREAK))
				Result = -(SelectedPos + 1);
			else
				Result = SelectedPos + 1;

		}
		else
		{
			if(bResultAsIndex)
				Result=0;
			else
				Result = L""sv;
		}
	}
	PushValue(Result);
}

// S=Env(S[,Mode[,Value]])
void FarMacroApi::environFunc() const
{
	auto Params = parseParams(3);
	auto& Value = Params[2];
	const auto& Mode = Params[1];
	auto& S = Params[0];

	PushValue(os::env::get(S.toString()));

	if (!Mode.asInteger())
		return;

	Value.isUnknown() || Value.asString().empty()?
		os::env::del(S.toString()) :
		os::env::set(S.toString(), Value.toString());
}

// V=Panel.Select(panelType,Action[,Mode[,Items]])
void FarMacroApi::panelselectFunc() const
{
	auto Params = parseParams(4);
	auto& ValItems = Params[3];
	const auto Mode = static_cast<int>(Params[2].asInteger());
	const auto Action = static_cast<int>(Params[1].asInteger());
	long long Result=-1;

	if (const auto SelPanel = SelectPanel(Params[0].asInteger()))
	{
		long long Index=-1;
		if (Mode == 1)
		{
			Index=ValItems.asInteger();
			if (!Index)
				Index=SelPanel->GetCurrentPos();
			else
				Index--;
		}

		if (Mode == 2 || Mode == 3)
		{
			string strStr=ValItems.asString();
			replace(strStr, L"\r"sv, L"\n"sv);
			replace(strStr, L"\n\n"sv, L"\n"sv);
			ValItems=strStr;
		}

		MacroPanelSelect mps;
		mps.Item = ValItems.asString();
		mps.Action      = Action;
		mps.Mode        = Mode;
		mps.Index       = Index;
		Result=SelPanel->VMProcess(MCODE_F_PANEL_SELECT,&mps,0);
	}

	PushValue(Result);
}

enum
{
	f_fattr_fs,
	f_pattr_panel,
	f_fexist_fs,
	f_fexist_panel,
};

void FarMacroApi::fattrFuncImpl(int Type) const
{
	os::fs::attributes FileAttr = INVALID_FILE_ATTRIBUTES;
	long Pos=-1;

	if (any_of(Type, f_fattr_fs, f_fexist_fs))
	{
		auto Params = parseParams(1);
		auto& Str = Params[0];

		// get_find_data to support wildcards
		if (os::fs::find_data FindData; os::fs::get_find_data(Str.toString(), FindData))
			FileAttr = FindData.Attributes;
		else
			FileAttr = os::fs::get_file_attributes(Str.toString());
	}
	else
	{
		auto Params = parseParams(2);
		auto& S = Params[1];
		const auto& Str = S.toString();

		if (const auto SelPanel = SelectPanel(Params[0].asInteger()))
		{
			Pos = (Str.find_first_of(L"*?"sv) == string::npos)?
				SelPanel->FindFile(Str, Str.find_first_of(L"\\/:"sv) == string::npos) :
				SelPanel->FindFirst(Str);

			if (Pos >= 0)
			{
				string strFileName;
				SelPanel->GetFileName(strFileName, Pos, FileAttr);
			}
		}
	}

	if (Type == f_fexist_fs)
		return PushBoolean(FileAttr != INVALID_FILE_ATTRIBUTES);

	if (Type == f_fexist_panel)
		return PushValue(Pos + 1);

	PushValue(static_cast<long>(FileAttr));
}

// N=fattr(S)
void FarMacroApi::fattrFunc() const
{
	return fattrFuncImpl(f_fattr_fs);
}

// N=fexist(S)
void FarMacroApi::fexistFunc() const
{
	return fattrFuncImpl(f_fexist_fs);
}

// N=panel.fattr(S)
void FarMacroApi::panelfattrFunc() const
{
	return fattrFuncImpl(f_pattr_panel);
}

// N=panel.fexist(S)
void FarMacroApi::panelfexistFunc() const
{
	return fattrFuncImpl(f_fexist_panel);
}

// N=FLock(Nkey,NState)
/*
  Nkey:
     0 - NumLock
     1 - CapsLock
     2 - ScrollLock

  State:
    -1 get state
     0 off
     1 on
     2 flip
*/
void FarMacroApi::flockFunc() const
{
	const auto Params = parseParams(2);
	int Ret = -1;
	const auto stateFLock = static_cast<int>(Params[1].asInteger());
	auto vkKey = static_cast<unsigned>(Params[0].asInteger());

	switch (vkKey)
	{
		case 0:
			vkKey=VK_NUMLOCK;
			break;
		case 1:
			vkKey=VK_CAPITAL;
			break;
		case 2:
			vkKey=VK_SCROLL;
			break;
		default:
			vkKey=0;
			break;
	}

	if (vkKey)
		Ret=SetFLockState(vkKey,stateFLock);

	PushValue(Ret);
}

// N=Dlg->SetFocus([ID])
void FarMacroApi::dlgsetfocusFunc() const
{
	const auto Params = parseParams(1);

	const auto Index = static_cast<unsigned>(Params[0].asInteger()) - 1;

	if (Global->CtrlObject->Macro.GetArea() != MACROAREA_DIALOG)
		return PushValue(-1);

	const auto Dlg = std::dynamic_pointer_cast<Dialog>(Global->WindowManager->GetCurrentWindow());
	if (!Dlg)
		return PushValue(-1);

	auto Ret = Dlg->VMProcess(MCODE_V_DLGCURPOS);
	if (static_cast<int>(Index) >= 0)
	{
		if (!Dlg->SendMessage(DM_SETFOCUS, Index, nullptr))
			Ret = 0;
	}

	PushValue(Ret);
}

// V=Far.Cfg.Get(Key,Name)
void FarMacroApi::farcfggetFunc() const
{
	const auto Params = parseParams(2);
	const auto& Name = Params[1];
	const auto& Key = Params[0];

	const auto option = Global->Opt->GetConfigValue(Key.asString(), Name.asString());
	return option? PushValue(option->toString()) : PushBoolean(false);
}

// V=Far.GetConfig(Key.Name)
void FarMacroApi::fargetconfigFunc() const
{
	const auto Keyname = (mData->Count >= 1 && mData->Values[0].Type==FMVT_STRING) ?
		mData->Values[0].String : L"";

	if (const auto Dot = std::wcsrchr(Keyname, L'.'))
	{
		const string_view Key(Keyname, Dot - Keyname);

		if (const auto option = Global->Opt->GetConfigValue(Key, Dot+1))
		{
			if (const auto OptBool = dynamic_cast<const BoolOption*>(option))
			{
				PushBoolean(OptBool->Get());
				PushValue(L"boolean");
			}
			else if (const auto OptBool3 = dynamic_cast<const Bool3Option*>(option))
			{
				auto Val = OptBool3->Get();
				if (Val == 0 || Val == 1)
					PushBoolean(Val == 1);
				else
					PushValue(L"other");

				PushValue(L"3-state");
			}
			else if (const auto OptInt = dynamic_cast<const IntOption*>(option))
			{
				PushValue(OptInt->Get());
				PushValue(L"integer");
			}
			else if (const auto OptString = dynamic_cast<const StringOption*>(option))
			{
				PushValue(OptString->Get());
				PushValue(L"string");
			}
			else
				PushError(L"unknown option type");
		}
		else
			PushError(L"setting doesn't exist");
	}
	else
		PushError(L"invalid argument #1");
}

// V=Dlg->GetValue([Pos[,InfoID]])
void FarMacroApi::dlggetvalueFunc() const
{
	auto Params = parseParams(2);
	TVar Ret(-1);

	if (Global->CtrlObject->Macro.GetArea()==MACROAREA_DIALOG)
	{
		// TODO: fix indentation
		if (const auto Dlg = std::dynamic_pointer_cast<Dialog>(Global->WindowManager->GetCurrentWindow()))
		{
			const auto IndexType = Params[0].type();
			auto Index=static_cast<size_t>(Params[0].asInteger())-1;
			if (IndexType == TVar::Type::Unknown || ((IndexType == TVar::Type::Integer || IndexType == TVar::Type::Double) && static_cast<int>(Index) < -1))
				Index=Dlg->GetDlgFocusPos();

			const auto InfoIDType = Params[1].type();
			auto InfoID=static_cast<int>(Params[1].asInteger());
			if (InfoIDType == TVar::Type::Unknown || (InfoIDType == TVar::Type::Integer && InfoID < 0))
				InfoID=0;

			FarGetValue fgv{ sizeof(fgv), InfoID, FMVT_UNKNOWN };
			auto& DlgItem = Dlg->GetAllItem();
			bool CallDialog=true;

			if (Index == static_cast<size_t>(-1))
			{
				SMALL_RECT Rect;

				if (Dlg->SendMessage(DM_GETDLGRECT,0,&Rect))
				{
					switch (InfoID)
					{
						case 0: Ret = static_cast<long long>(DlgItem.size()); break;
						case 2: Ret=Rect.Left; break;
						case 3: Ret=Rect.Top; break;
						case 4: Ret=Rect.Right; break;
						case 5: Ret=Rect.Bottom; break;
						case 6: Ret = static_cast<long long>(Dlg->GetDlgFocusPos()) + 1; break;
						default: break;
					}
				}
			}
			else if (Index < DlgItem.size() && !DlgItem.empty())
			{
				const DialogItemEx& Item=DlgItem[Index];
				FARDIALOGITEMTYPES ItemType=Item.Type;
				FARDIALOGITEMFLAGS ItemFlags=Item.Flags;

				if (!InfoID)
				{
					if (ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
					{
						InfoID=7;
					}
					else if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
					{
						FarListGetItem ListItem{ sizeof(ListItem) };
						ListItem.ItemIndex=Item.ListPtr->GetSelectPos();

						if (Dlg->SendMessage(DM_LISTGETITEM,Index,&ListItem))
						{
							Ret=ListItem.Item.Text;
						}
						else
						{
							Ret = L""sv;
						}

						InfoID=-1;
					}
					else
					{
						InfoID=10;
					}
				}

				switch (InfoID)
				{
					case 1: Ret=ItemType;   break;
					case 2: Ret=Item.X1;    break;
					case 3: Ret=Item.Y1;    break;
					case 4: Ret=Item.X2;    break;
					case 5: Ret=Item.Y2;    break;
					case 6: Ret=(Item.Flags&DIF_FOCUS)!=0; break;
					case 7:
					{
						if (ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
						{
							Ret=Item.Selected;
						}
						else if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
						{
							Ret=Item.ListPtr->GetSelectPos()+1;
						}
						else
						{
							Ret=0ll;
							/*
							int Item->Selected;
							const char *Item->History;
							const char *Item->Mask;
							FarList *Item->ListItems;
							int  Item->ListPos;
							FAR_CHAR_INFO *Item->VBuf;
							*/
						}

						break;
					}
					case 8: Ret = static_cast<long long>(ItemFlags); break;
					case 9: Ret=(Item.Flags&DIF_DEFAULTBUTTON)!=0; break;
					case 10:
					{
						Ret=Item.strData;

						if (IsEdit(ItemType))
						{
							if (const auto EditPtr = static_cast<const DlgEdit*>(Item.ObjPtr))
								Ret = EditPtr->GetString();
						}

						break;
					}
					case 11:
					{
						if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
						{
							Ret = static_cast<long long>(Item.ListPtr->size());
						}
						break;
					}
				}
			}
			else
			{
				CallDialog=false;
			}

			if (CallDialog)
			{
				switch (Ret.type())
				{
					case TVar::Type::Unknown:
						fgv.Value.Type = FMVT_UNKNOWN;
						fgv.Value.Integer = Ret.asInteger();
						break;

					case TVar::Type::Integer:
						fgv.Value.Type = FMVT_INTEGER;
						fgv.Value.Integer=Ret.asInteger();
						break;

					case TVar::Type::String:
						fgv.Value.Type = FMVT_STRING;
						fgv.Value.String=Ret.asString().c_str();
						break;

					case TVar::Type::Double:
						fgv.Value.Type = FMVT_DOUBLE;
						fgv.Value.Double=Ret.asDouble();
						break;
				}

				if (Dlg->SendMessage(DN_GETVALUE,Index,&fgv))
				{
					switch (fgv.Value.Type)
					{
						case FMVT_UNKNOWN:
							Ret=0;
							break;
						case FMVT_INTEGER:
							Ret=fgv.Value.Integer;
							break;
						case FMVT_DOUBLE:
							Ret=fgv.Value.Double;
							break;
						case FMVT_STRING:
							Ret=fgv.Value.String;
							break;
						default:
							break;
					}
				}
			}
		}
	}

	PushValue(Ret);
}

// N=Editor.Pos(Op,What[,Where])
// Op: 0 - get, 1 - set
void FarMacroApi::editorposFunc() const
{
	SCOPED_ACTION(LockOutput)(IsTopMacroOutputDisabled());

	auto Params = parseParams(3);
	TVar Ret(-1);
	int Where = static_cast<int>(Params[2].asInteger());
	int What = static_cast<int>(Params[1].asInteger());
	int Op = static_cast<int>(Params[0].asInteger());

	if (Global->CtrlObject->Macro.GetArea() != MACROAREA_EDITOR)
		return PushValue(Ret);

	const auto CurrentEditor = Global->WindowManager->GetCurrentEditor();
	if (!CurrentEditor || !CurrentEditor->IsVisible())
		return PushValue(Ret);

	EditorInfo ei{ sizeof(ei) };
	CurrentEditor->EditorControl(ECTL_GETINFO,0,&ei);

	switch (Op)
	{
	case 0: // get
		switch (What)
		{
		case 1: // CurLine
			Ret=ei.CurLine+1;
			break;

		case 2: // CurPos
			Ret=ei.CurPos+1;
			break;

		case 3: // CurTabPos
			Ret=ei.CurTabPos+1;
			break;

		case 4: // TopScreenLine
			Ret=ei.TopScreenLine+1;
			break;

		case 5: // LeftPos
			Ret=ei.LeftPos+1;
			break;

		case 6: // Overtype
			Ret=ei.Overtype;
			break;
		}
		break;

	case 1: // set
		{
			EditorSetPosition esp{ sizeof(esp) };
			esp.CurLine=-1;
			esp.CurPos=-1;
			esp.CurTabPos=-1;
			esp.TopScreenLine=-1;
			esp.LeftPos=-1;
			esp.Overtype=-1;

			switch (What)
			{
			case 1: // CurLine
				esp.CurLine=Where-1;
				if (esp.CurLine < 0)
					esp.CurLine=-1;
				break;

			case 2: // CurPos
				esp.CurPos=Where-1;
				if (esp.CurPos < 0)
					esp.CurPos=-1;
				break;

			case 3: // CurTabPos
				esp.CurTabPos=Where-1;
				if (esp.CurTabPos < 0)
					esp.CurTabPos=-1;
				break;

			case 4: // TopScreenLine
				esp.TopScreenLine=Where-1;
				if (esp.TopScreenLine < 0)
					esp.TopScreenLine=-1;
				break;

			case 5: // LeftPos
				{
					int Delta=Where-1-ei.LeftPos;
					esp.LeftPos=Where-1;

					if (esp.LeftPos < 0)
						esp.LeftPos=-1;

					esp.CurPos=ei.CurPos+Delta;
				}
				break;

			case 6: // Overtype
				esp.Overtype=Where;
				break;
			}

			const auto Result = CurrentEditor->EditorControl(ECTL_SETPOSITION, 0, &esp);

			if (Result)
				CurrentEditor->EditorControl(ECTL_REDRAW, 0, nullptr);

			Ret = Result;
			break;
		}
	}

	PushValue(Ret);
}

// OldVar=Editor.Set(Idx,Value)
void FarMacroApi::editorsetFunc() const
{
	auto Params = parseParams(2);
	TVar Ret(-1);
	auto& Value = Params[1];
	int Index = static_cast<int>(Params[0].asInteger());

	if (Global->CtrlObject->Macro.GetArea() != MACROAREA_EDITOR)
		return PushValue(Ret);

	const auto CurrentEditor = Global->WindowManager->GetCurrentEditor();
	if (!CurrentEditor || !CurrentEditor->IsVisible())
		return PushValue(Ret);

	long long longState = -1;

	enum class editor_options
	{
		TabSize                 = 0,
		ExpandTabs              = 1,
		PersistentBlocks        = 2,
		DelRemovesBlocks        = 3,
		AutoIndent              = 4,
		AutoDetectCodePage      = 5,
		CursorBeyondEOL         = 7,
		BSLikeDel               = 8,
		CharCodeBase            = 9,
		SavePos                 = 10,
		SaveShortPos            = 11,
		WordDiv                 = 12,
		AllowEmptySpaceAfterEof = 14,
		ShowScrollBar           = 15,
		EditOpenedForWrite      = 16,
		SearchSelFound          = 17,
		ShowWhiteSpace          = 20,
	};

	if (mData->Count > 1)
	{
		if (static_cast<editor_options>(Index) == editor_options::WordDiv)
		{
			if (Value.isString() || Value.asInteger() != -1)
				longState = 0;
		}
		else
		{
			longState = Value.toInteger();
		}
	}

	Options::EditorOptions EdOpt;
	CurrentEditor->GetEditorOptions(EdOpt);

	switch (static_cast<editor_options>(Index))
	{
	case editor_options::TabSize:
		Ret = EdOpt.TabSize;
		break;

	case editor_options::ExpandTabs:
		Ret = EdOpt.ExpandTabs;
		break;

	case editor_options::PersistentBlocks:
		Ret = EdOpt.PersistentBlocks;
		break;

	case editor_options::DelRemovesBlocks:
		Ret = EdOpt.DelRemovesBlocks;
		break;

	case editor_options::AutoIndent:
		Ret = EdOpt.AutoIndent;
		break;

	case editor_options::AutoDetectCodePage:
		Ret = EdOpt.AutoDetectCodePage;
		break;

	case editor_options::CursorBeyondEOL:
		Ret = EdOpt.CursorBeyondEOL;
		break;

	case editor_options::BSLikeDel:
		Ret = EdOpt.BSLikeDel;
		break;

	case editor_options::CharCodeBase:
		Ret = EdOpt.CharCodeBase;
		break;

	case editor_options::SavePos:
		Ret = EdOpt.SavePos;
		break;

	case editor_options::SaveShortPos:
		Ret = EdOpt.SaveShortPos;
		break;

	case editor_options::WordDiv:
		Ret = TVar(EdOpt.strWordDiv);
		break;

	case editor_options::AllowEmptySpaceAfterEof:
		Ret = EdOpt.AllowEmptySpaceAfterEof;
		break;

	case editor_options::ShowScrollBar:
		Ret = EdOpt.ShowScrollBar;
		break;

	case editor_options::EditOpenedForWrite:
		Ret = EdOpt.EditOpenedForWrite;
		break;

	case editor_options::SearchSelFound:
		Ret = EdOpt.SearchSelFound;
		break;

	case editor_options::ShowWhiteSpace:
		Ret = EdOpt.ShowWhiteSpace;
		break;

	default:
		Ret = -1;
		break;
	}

	if (longState == -1)
		return PushValue(Ret);

	switch (static_cast<editor_options>(Index))
	{
	case editor_options::TabSize:
		if (!EdOpt.TabSize.TrySet(longState))
			Ret = -1;
		break;

	case editor_options::ExpandTabs:
		EdOpt.ExpandTabs = longState;
		break;

	case editor_options::PersistentBlocks:
		EdOpt.PersistentBlocks = longState != 0;
		break;

	case editor_options::DelRemovesBlocks:
		EdOpt.DelRemovesBlocks = longState != 0;
		break;

	case editor_options::AutoIndent:
		EdOpt.AutoIndent = longState != 0;
		break;

	case editor_options::AutoDetectCodePage:
		EdOpt.AutoDetectCodePage = longState != 0;
		break;

	case editor_options::CursorBeyondEOL:
		EdOpt.CursorBeyondEOL = longState != 0;
		break;

	case editor_options::BSLikeDel:
		EdOpt.BSLikeDel = longState != 0;
		break;

	case editor_options::CharCodeBase:
		EdOpt.CharCodeBase = longState;
		break;

	case editor_options::SavePos:
		EdOpt.SavePos = longState != 0;
		break;

	case editor_options::SaveShortPos:
		EdOpt.SaveShortPos = longState != 0;
		break;

	case editor_options::WordDiv:
		EdOpt.strWordDiv = Value.toString();
		break;

	case editor_options::AllowEmptySpaceAfterEof:
		EdOpt.AllowEmptySpaceAfterEof = longState != 0;
		break;

	case editor_options::ShowScrollBar:
		EdOpt.ShowScrollBar = longState != 0;
		break;

	case editor_options::EditOpenedForWrite:
		EdOpt.EditOpenedForWrite = longState != 0;
		break;

	case editor_options::SearchSelFound:
		EdOpt.SearchSelFound = longState != 0;
		break;

	case editor_options::ShowWhiteSpace:
		EdOpt.ShowWhiteSpace = longState;
		break;

	default:
		Ret = -1;
		break;
	}

	CurrentEditor->SetEditorOptions(EdOpt);
	CurrentEditor->ShowStatus();

	switch (static_cast<editor_options>(Index))
	{
	case editor_options::TabSize:
	case editor_options::WordDiv:
	case editor_options::AllowEmptySpaceAfterEof:
	case editor_options::ShowScrollBar:
	case editor_options::ShowWhiteSpace:
		CurrentEditor->Show();
		break;

	default:
		break;
	}

	PushValue(Ret);
}

// V=Clip(N[,V])
void FarMacroApi::clipFunc() const
{
	auto Params = parseParams(2);
	auto& Val = Params[1];
	const auto cmdType = static_cast<int>(Params[0].asInteger());

	// принудительно второй параметр ставим AS string
	if (cmdType != 5 && Val.isInteger() && !Val.asInteger())
	{
		Val = L""sv;
		Val.toString();
	}

	switch (cmdType)
	{
	case 0: // Get from Clipboard, "S" - ignore
		{
			string ClipText;
			if (!GetClipboardText(ClipText))
				return PushValue(0);

			return PushValue(ClipText);
		}

	case 1: // Put "S" into Clipboard
		{
			const auto& Str = Val.asString();
			return PushValue(Str.empty()? ClearClipboard() : SetClipboardText(Str));
		}

	case 2: // Add "S" into Clipboard
		{
			const clipboard_accessor Clip;
			if (!Clip->Open())
				return PushValue(0);

			string CopyData;
			if (!Clip->GetText(CopyData))
				return PushValue(0);

			return PushValue(Clip->SetText(CopyData + Val.asString()));
		}

	case 3: // Copy Win to internal, "S" - ignore
	case 4: // Copy internal to Win, "S" - ignore
		{
			const clipboard_accessor
				ClipSystem(clipboard_mode::system),
				ClipInternal(clipboard_mode::internal);

			return PushValue(ClipSystem->Open() && ClipInternal->Open() && (cmdType == 3? CopyData(ClipSystem, ClipInternal) : CopyData(ClipInternal, ClipSystem)));
		}

	case 5: // ClipMode
		{
			// 0 - flip, 1 - виндовый буфер, 2 - внутренний, -1 - что сейчас?
			const auto Action = Val.asInteger();
			const auto PreviousMode = default_clipboard_mode::get();

			switch (Action)
			{
			case 0:
				default_clipboard_mode::set(PreviousMode == clipboard_mode::system? clipboard_mode::internal : clipboard_mode::system);
				break;

			case 1:
				default_clipboard_mode::set(clipboard_mode::system);
				break;

			case 2:
				default_clipboard_mode::set(clipboard_mode::internal);
				break;

			default:
				break;
			}

			return PushValue(PreviousMode == clipboard_mode::internal? 2 : 1);
		}
	}
}


// N=Panel.SetPosIdx(panelType,Idx[,InSelection])
/*
*/
void FarMacroApi::panelsetposidxFunc() const
{
	const auto Params = parseParams(3);
	const auto InSelection = static_cast<int>(Params[2].asInteger());
	auto idxItem = static_cast<long>(Params[1].asInteger());
	long long Ret=0;

	if (const auto SelPanel = SelectPanel(Params[0].asInteger()))
	{
		const auto PanelType = SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL

		if (PanelType == panel_type::FILE_PANEL || PanelType == panel_type::TREE_PANEL)
		{
			size_t EndPos=SelPanel->GetFileCount();
			long idxFoundItem=0;

			if (idxItem) // < 0 || > 0
			{
				EndPos--;
				if ( EndPos > 0 )
				{
					size_t StartPos;
					const auto Direct = idxItem < 0 ? -1 : 1;

					if( Direct < 0 )
						idxItem=-idxItem;
					idxItem--;

					if( Direct < 0 )
					{
						StartPos=EndPos;
						EndPos=0;//InSelection?0:idxItem;
					}
					else
						StartPos=0;//!InSelection?0:idxItem;

					bool found=false;

					for (intptr_t I=StartPos ; ; I+=Direct ) // Важно: переменная I должна быть signed!
					{
						if (Direct > 0)
						{
							if(I > static_cast<intptr_t>(EndPos))
								break;
						}
						else
						{
							if(I < static_cast<intptr_t>(EndPos))
								break;
						}

						if (!InSelection || SelPanel->IsSelected(I))
						{
							if (idxFoundItem == idxItem)
							{
								idxItem = static_cast<long>(I);
								found=true;
								break;
							}
							idxFoundItem++;
						}
					}

					if (!found)
						idxItem=-1;

					if (idxItem != -1 && SelPanel->GoToFile(idxItem))
					{
						if (SelPanel->IsVisible())
							SelPanel->Show();
						// <Mantis#0000289> - грозно, но со вкусом :-)
						//ShellUpdatePanels(SelPanel);
						//SelPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
						//WindowManager->RefreshWindow(WindowManager->GetCurrentWindow());
						// </Mantis#0000289>

						Ret = static_cast<long long>(InSelection? idxFoundItem : SelPanel->GetCurrentPos()) + 1;
					}
				}
			}
			else // = 0 - вернем текущую позицию
			{
				if ( !InSelection )
					Ret = static_cast<long long>(SelPanel->GetCurrentPos()) + 1;
				else
				{
					const auto CurPos = SelPanel->GetCurrentPos();
					for (const auto I: std::views::iota(0uz, EndPos))
					{
						if ( SelPanel->IsSelected(I) && SelPanel->FileInFilter(I) )
						{
							if (I == static_cast<size_t>(CurPos))
							{
								Ret = static_cast<long long>(idxFoundItem) + 1;
								break;
							}
							idxFoundItem++;
						}
					}
				}
			}
		}
	}

	PushValue(Ret);
}

// N=Panel.SetPos(panelType,fileName)
void FarMacroApi::panelsetposFunc() const
{
	const auto Params = parseParams(2);
	const auto& Val = Params[1];
	const auto& fileName=Val.asString();

	//const auto CurrentWindow=WindowManager->GetCurrentWindow();
	long long Ret=0;

	if (const auto SelPanel = SelectPanel(Params[0].asInteger()))
	{
		const auto PanelType = SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL

		if (PanelType == panel_type::FILE_PANEL || PanelType == panel_type::TREE_PANEL)
		{
			// Need PointToName()?
			if (SelPanel->GoToFile(fileName))
			{
				//SelPanel->Show();
				// <Mantis#0000289> - грозно, но со вкусом :-)
				//ShellUpdatePanels(SelPanel);
				SelPanel->UpdateIfChanged();
				Global->WindowManager->RefreshWindow(Global->WindowManager->GetCurrentWindow());
				// </Mantis#0000289>
				Ret = static_cast<long long>(SelPanel->GetCurrentPos()) + 1;
			}
		}
	}

	PushValue(Ret);
}

// Result=replace(Str,Find,Replace[,Cnt[,Mode]])
/*
Find=="" - return Str
Cnt==0 - return Str
Replace=="" - return Str (с удалением всех подстрок Find)
Str=="" return ""

Mode:
      0 - case insensitive
      1 - case sensitive

*/

void FarMacroApi::replaceFunc() const
{
	const auto Params = parseParams(5);
	auto Src = Params[0].asString();
	const auto& Find = Params[1].asString();
	const auto& Repl = Params[2].asString();
	const auto Count = Params[3].asInteger();
	const auto IgnoreCase = !Params[4].asInteger();

	ReplaceStrings(Src, Find, Repl, IgnoreCase, Count <= 0 ? string::npos : static_cast<size_t>(Count));
	PushValue(Src);
}

// V=Panel.Item(typePanel,Index,TypeInfo)
void FarMacroApi::panelitemFunc() const
{
	auto Params = parseParams(3);
	auto& P2 = Params[2];
	auto& P1 = Params[1];

	//const auto CurrentWindow=WindowManager->GetCurrentWindow();

	const auto SelPanel = SelectPanel(Params[0].asInteger());
	if (!SelPanel)
		return PushError(L"No panel selected");

	const auto PanelType = SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL

	if (!(PanelType == panel_type::FILE_PANEL || PanelType == panel_type::TREE_PANEL))
		return PushError(L"Unsupported panel type");

	const auto Index = static_cast<int>(P1.toInteger()) - 1;
	const auto TypeInfo = static_cast<int>(P2.toInteger());

	if (const auto Tree = std::dynamic_pointer_cast<TreeList>(SelPanel))
	{
		const auto treeItem = Tree->GetItem(Index);
		if (treeItem && !TypeInfo)
			return PushValue(treeItem->strName);
	}
	else if (const auto fileList = std::dynamic_pointer_cast<FileList>(SelPanel))
	{
		string strDate, strTime;

		if (TypeInfo == 11)
			fileList->ReadDiz();

		const auto filelistItem = fileList->GetItem(Index);

		if (!filelistItem)
			return PushError(L"No list item found at specified index");

		const auto FormatDate = [](const os::chrono::time_point TimePoint)
		{
			const auto& [Date, Time] = time_point_to_string(TimePoint, 8, 1);
			return concat(Date, L' ', Time);
		};

		switch (TypeInfo)
		{
			case 0:  // Name
				return PushValue(filelistItem->FileName);
			case 1:  // ShortName
				return PushValue(filelistItem->AlternateFileName());
			case 2:  // FileAttr
				return PushValue(filelistItem->Attributes);
			case 3:  // CreationTime
				return PushValue(FormatDate(filelistItem->CreationTime));
			case 4:  // AccessTime
				return PushValue(FormatDate(filelistItem->LastAccessTime));
			case 5:  // WriteTime
				return PushValue(FormatDate(filelistItem->LastWriteTime));
			case 6:  // FileSize
				return PushValue(filelistItem->FileSize);
			case 7:  // AllocationSize
				return PushValue(filelistItem->AllocationSize(fileList.get()));
			case 8:  // Selected
				return PushBoolean(filelistItem->Selected);
			case 9:  // NumberOfLinks
				return PushValue(filelistItem->NumberOfLinks(fileList.get()));
			case 10:  // SortGroup
				return PushValue(filelistItem->SortGroup);
			case 11:  // DizText
				return PushValue(filelistItem->DizText);
			case 12:  // Owner
				return PushValue(filelistItem->Owner(fileList.get()));
			case 13:  // CRC32
				return PushValue(filelistItem->CRC32);
			case 14:  // Position
				return PushValue(filelistItem->Position);
			case 15:  // CreationTime
				return PushValue(os::chrono::nt_clock::to_hectonanoseconds(filelistItem->CreationTime));
			case 16:  // AccessTime
				return PushValue(os::chrono::nt_clock::to_hectonanoseconds(filelistItem->LastAccessTime));
			case 17:  // WriteTime
				return PushValue(os::chrono::nt_clock::to_hectonanoseconds(filelistItem->LastWriteTime));
			case 18: // NumberOfStreams
				return PushValue(filelistItem->NumberOfStreams(fileList.get()));
			case 19: // StreamsSize
				return PushValue(filelistItem->StreamsSize(fileList.get()));
			case 20:  // ChangeTime
				return PushValue(FormatDate(filelistItem->ChangeTime));
			case 21:  // ChangeTime
				return PushValue(os::chrono::nt_clock::to_hectonanoseconds(filelistItem->ChangeTime));
			case 22:  // ContentData (was: CustomData)
				//Ret=TVar(filelistItem->ContentData.size() ? filelistItem->ContentData[0] : L"");
				break;
			case 23:  // ReparseTag
				return PushValue(filelistItem->ReparseTag);
		}
	}

	PushError(L"Operation not supported");
}

// N=len(V)
void FarMacroApi::lenFunc() const
{
	auto Params = parseParams(1);
	PushValue(Params[0].toString().size());
}

void FarMacroApi::ucaseFunc() const
{
	auto Params = parseParams(1);
	auto& Val = Params[0];
	Val = upper(Val.toString());
	PushValue(Val);
}

void FarMacroApi::lcaseFunc() const
{
	auto Params = parseParams(1);
	auto& Val = Params[0];
	Val = lower(Val.toString());
	PushValue(Val);
}

void FarMacroApi::stringFunc() const
{
	auto Params = parseParams(1);
	auto& Val = Params[0];
	Val.toString();
	PushValue(Val);
}

// S=StrPad(Src,Cnt[,Fill[,Op]])
void FarMacroApi::strpadFunc() const
{
	auto Params = parseParams(4);
	auto& Src = Params[0];
	if (Src.isUnknown())
	{
		Src = L""sv;
		Src.toString();
	}
	const auto Cnt = static_cast<int>(Params[1].asInteger());
	auto& Fill = Params[2];
	if (Fill.isUnknown())
		Fill = L" "sv;
	const auto Op = static_cast<DWORD>(Params[3].asInteger());

	auto strDest = Src.asString();
	const auto LengthFill = Fill.asString().size();
	if (Cnt > 0 && LengthFill)
	{
		const auto LengthSrc = static_cast<int>(strDest.size());
		const auto FineLength = Cnt - LengthSrc;

		if (FineLength > 0)
		{
			string NewFill;
			NewFill.reserve(FineLength + 1);

			const auto& pFill = Fill.asString();

			for (const auto i: std::views::iota(0uz, static_cast<size_t>(FineLength)))
			{
				NewFill.push_back(pFill[i % LengthFill]);
			}

			int CntL=0, CntR=0;
			switch (Op)
			{
			case 0: // right
				CntR=FineLength;
				break;
			case 1: // left
				CntL=FineLength;
				break;
			case 2: // center
				if (LengthSrc > 0)
				{
					CntL=FineLength / 2;
					CntR=FineLength-CntL;
				}
				else
					CntL=FineLength;
				break;
			}

			strDest = concat(string_view(NewFill).substr(0, CntL), strDest, string_view(NewFill).substr(0, CntR));
		}
	}

	PushValue(strDest);
}

// S=StrWrap(Text,Width[,Break[,Flags]])
void FarMacroApi::strwrapFunc() const
{
	auto Params = parseParams(3);
	auto& Break = Params[2];
	const size_t Width = Params[1].asInteger();
	const auto& Text = Params[0];

	if (Break.isUnknown())
		Break = L"\n"sv;

	PushValue(join(Break.asString(), wrapped_text(Text.asString(), Width)));
}

void FarMacroApi::intFunc() const
{
	const auto Params = parseParams(1);
	const auto& Val = Params[0];
	PushValue(Val.asInteger());
}

void FarMacroApi::floatFunc() const
{
	const auto Params = parseParams(1);
	const auto& Val = Params[0];
	PushValue(Val.asDouble());
}

void FarMacroApi::absFunc() const
{
	const auto Params = parseParams(1);

	TVar Result;

	switch(Params[0].ParseType())
	{
	case TVar::Type::Integer:
		{
			if (const auto i = Params[0].asInteger(); i < 0)
				Result = -i;
			else
				Result = Params[0];
		}
		break;

	case TVar::Type::Double:
		{
		if (const auto d = Params[0].asDouble(); d < 0)
			Result = -d;
		else
			Result = Params[0];
		}
		break;

	default:
		break;
	}

	PushValue(Result);
}

void FarMacroApi::ascFunc() const
{
	const auto Params = parseParams(1);
	const auto& tmpVar = Params[0];

	if (tmpVar.isString() && !tmpVar.asString().empty())
		PushValue(tmpVar.asString().front());
	else
		PushValue(tmpVar);
}

void FarMacroApi::chrFunc() const
{
	const auto Params = parseParams(1);
	auto tmpVar = Params[0];

	if (tmpVar.isNumber())
	{
		const auto Char = static_cast<wchar_t>(tmpVar.asInteger());
		tmpVar = string_view{ &Char, 1 };
	}

	PushValue(tmpVar);
}

// N=FMatch(S,Mask)
void FarMacroApi::fmatchFunc() const
{
	auto Params = parseParams(2);
	auto& Mask = Params[1];
	auto& S = Params[0];
	filemasks FileMask;

	if (FileMask.assign(Mask.toString(), FMF_SILENT))
		PushValue(FileMask.check(S.toString()));
	else
		PushValue(-1);
}

// V=Editor.Sel(Action[,Opt])
void FarMacroApi::editorselFunc() const
{
	/*
	 MCODE_F_EDITOR_SEL
	  Action: 0 = Get Param
	              Opt:  0 = return FirstLine
	                    1 = return FirstPos
	                    2 = return LastLine
	                    3 = return LastPos
	                    4 = return block type (0=nothing 1=stream, 2=column)
	              return: 0 = failure, 1... request value

	          1 = Set Pos
	              Opt:  0 = begin block (FirstLine & FirstPos)
	                    1 = end block (LastLine & LastPos)
	              return: 0 = failure, 1 = success

	          2 = Set Stream Selection Edge
	              Opt:  0 = selection start
	                    1 = selection finish
	              return: 0 = failure, 1 = success

	          3 = Set Column Selection Edge
	              Opt:  0 = selection start
	                    1 = selection finish
	              return: 0 = failure, 1 = success
	          4 = Unmark selected block
	              Opt: ignore
	              return 1
	*/
	auto Params = parseParams(2);
	TVar Ret(0ll);
	const auto& Opts = Params[1];
	auto& Action = Params[0];

	const auto Area = Global->CtrlObject->Macro.GetArea();
	const auto NeedType = Area == MACROAREA_EDITOR? windowtype_editor : (Area == MACROAREA_VIEWER? windowtype_viewer : (Area == MACROAREA_DIALOG ? windowtype_dialog : windowtype_panels)); // MACROAREA_SHELL?
	const auto CurrentWindow = Global->WindowManager->GetCurrentWindow();

	if (CurrentWindow && CurrentWindow->GetType()==NeedType)
	{
		if (Area==MACROAREA_SHELL && Global->CtrlObject->CmdLine()->IsVisible())
			Ret=Global->CtrlObject->CmdLine()->VMProcess(MCODE_F_EDITOR_SEL,ToPtr(Action.toInteger()),Opts.asInteger());
		else
			Ret=CurrentWindow->VMProcess(MCODE_F_EDITOR_SEL,ToPtr(Action.toInteger()),Opts.asInteger());
	}

	PushValue(Ret);
}

// V=Editor.Undo(Action)
void FarMacroApi::editorundoFunc() const
{
	if (Global->CtrlObject->Macro.GetArea() != MACROAREA_EDITOR)
		return PushValue(0);

	const auto CurrentEditor = Global->WindowManager->GetCurrentEditor();
	if (!CurrentEditor || !CurrentEditor->IsVisible())
		return PushValue(0);

	auto Params = parseParams(1);
	auto& Action = Params[0];

	EditorUndoRedo eur{ sizeof(eur) };
	eur.Command=static_cast<EDITOR_UNDOREDO_COMMANDS>(Action.toInteger());
	PushValue(CurrentEditor->EditorControl(ECTL_UNDOREDO, 0, &eur));
}

// N=Editor.SetTitle([Title])
void FarMacroApi::editorsettitleFunc() const
{
	if (Global->CtrlObject->Macro.GetArea() != MACROAREA_EDITOR)
		return PushValue(0);

	const auto CurrentEditor = Global->WindowManager->GetCurrentEditor();
	if (!CurrentEditor || !CurrentEditor->IsVisible())
		return PushValue(0);

	auto Params = parseParams(1);
	auto& Title = Params[0];

	if (Title.isInteger() && !Title.asInteger())
		Title = L""sv;

	PushValue(CurrentEditor->EditorControl(ECTL_SETTITLE, 0, UNSAFE_CSTR(Title.asString())));
}

// N=Editor.DelLine([Line])
void FarMacroApi::editordellineFunc() const
{
	if (Global->CtrlObject->Macro.GetArea() != MACROAREA_EDITOR)
		return PushValue(0);

	const auto CurrentEditor = Global->WindowManager->GetCurrentEditor();
	if (!CurrentEditor || !CurrentEditor->IsVisible())
		return PushValue(0);

	const auto Params = parseParams(1);
	const auto& Line = Params[0];

	if (!Line.isNumber())
		return PushValue(0);

	PushValue(CurrentEditor->VMProcess(MCODE_F_EDITOR_DELLINE, nullptr, Line.asInteger() - 1));
}

// N=Editor.InsStr([S[,Line]])
void FarMacroApi::editorinsstrFunc() const
{
	if (Global->CtrlObject->Macro.GetArea() != MACROAREA_EDITOR)
		return PushValue(0);

	const auto CurrentEditor = Global->WindowManager->GetCurrentEditor();
	if (!CurrentEditor || !CurrentEditor->IsVisible())
		return PushValue(0);

	auto Params = parseParams(2);
	auto& S = Params[0];
	const auto& Line = Params[1];

	if (!Line.isNumber())
		return PushValue(0);

	if (S.isUnknown())
		S = L""sv;

	PushValue(CurrentEditor->VMProcess(MCODE_F_EDITOR_INSSTR, UNSAFE_CSTR(S.asString()), Line.asInteger() - 1));
}

// N=Editor.SetStr([S[,Line]])
void FarMacroApi::editorsetstrFunc() const
{
	if (Global->CtrlObject->Macro.GetArea() != MACROAREA_EDITOR)
		return PushValue(0);

	const auto CurrentEditor = Global->WindowManager->GetCurrentEditor();
	if (!CurrentEditor || !CurrentEditor->IsVisible())
		return PushValue(0);

	auto Params = parseParams(2);
	auto& S = Params[0];
	const auto& Line = Params[1];

	if (!Line.isNumber())
		return PushValue(0);

	if (S.isUnknown())
		S = L""sv;

	PushValue(CurrentEditor->VMProcess(MCODE_F_EDITOR_SETSTR, UNSAFE_CSTR(S.asString()), Line.asInteger() - 1));
}

// N=Plugin.Exist(Uuid)
void FarMacroApi::pluginexistFunc() const
{
	if (!mData->Count || mData->Values[0].Type != FMVT_STRING)
		return PushBoolean(false);

	const auto Uuid = uuid::try_parse(mData->Values[0].String);
	PushBoolean(Uuid && Global->CtrlObject->Plugins->FindPlugin(*Uuid) != nullptr);
}

// N=Plugin.Load(DllPath[,ForceLoad])
void FarMacroApi::pluginloadFunc() const
{
	const auto Params = parseParams(2);
	const auto& ForceLoad = Params[1];
	const auto& DllPath = Params[0].asString();
	const TVar Ret(pluginapi::apiPluginsControl(nullptr, !ForceLoad.asInteger()?PCTL_LOADPLUGIN:PCTL_FORCEDLOADPLUGIN, 0, UNSAFE_CSTR(DllPath)));
	PushValue(Ret);
}

// N=Plugin.UnLoad(DllPath)
void FarMacroApi::pluginunloadFunc() const
{
	int Ret=0;
	if (mData->Count>0 && mData->Values[0].Type==FMVT_STRING)
	{
		if (const auto p = Global->CtrlObject->Plugins->FindPlugin(mData->Values[0].String))
		{
			Ret = static_cast<int>(pluginapi::apiPluginsControl(p, PCTL_UNLOADPLUGIN, 0, nullptr));
		}
	}
	PushValue(Ret);
}

// N=testfolder(S)
/*
возвращает одно состояний тестируемого каталога:

TSTFLD_NOTFOUND   (2) - нет такого
TSTFLD_NOTEMPTY   (1) - не пусто
TSTFLD_EMPTY      (0) - пусто
TSTFLD_NOTACCESS (-1) - нет доступа
TSTFLD_ERROR     (-2) - ошибка (кривые параметры или не хватило памяти для выделения промежуточных буферов)
*/
void FarMacroApi::testfolderFunc() const
{
	const auto Params = parseParams(1);
	const auto& tmpVar = Params[0];
	long long Ret=TSTFLD_ERROR;

	if (tmpVar.isString())
	{
		SCOPED_ACTION(elevation::suppress);
		Ret = static_cast<long long>(TestFolder(tmpVar.asString()));
	}

	PushValue(Ret);
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("macro.ToDouble")
{
	constexpr auto
		Min = std::numeric_limits<long long>::min(),
		Max = std::numeric_limits<long long>::max(),
		Limit = static_cast<long long>(bit(std::numeric_limits<double>::digits));

	static const struct
	{
		long long Value;
		bool Valid;
	}
	Tests[]
	{
		{  Min,         false },
		{  Min + 1,     false },
		{  Min + 2,     false },
		{ -Limit - 2,   false },
		{ -Limit - 1,   false },
		{ -Limit,       false },
		{ -Limit + 1,   true  },
		{ -Limit + 2,   true  },
		{ -2,           true  },
		{ -1,           true  },
		{  0,           true  },
		{  1,           true  },
		{  2,           true  },
		{  Limit - 2,   true  },
		{  Limit - 1,   true  },
		{  Limit,       false },
		{  Limit + 1,   false },
		{  Limit + 2,   false },
		{  Limit + 2,   false },
		{  Max - 2,     false },
		{  Max - 1,     false },
		{  Max,         false },
	};

	for (const auto& i: Tests)
	{
		double Result = 0;
		REQUIRE(ToDouble(i.Value, Result) == i.Valid);

		if (i.Valid)
			REQUIRE(static_cast<long long>(Result) == i.Value);

	}
}

TEST_CASE("macro.splitpath")
{
	enum splitpath_flags
	{
		sp_root = 0_bit,
		sp_path = 1_bit,
		sp_name = 2_bit,
		sp_ext  = 3_bit,
	};

	static const struct
	{
		string_view FullPath;
		string_view Root, Path, Name, Ext;
	}
	Tests[]
	{
		{ L"C:"sv,                                L"C:"sv,                {},            {},        {},        },
		{ L"C:\\"sv,                              L"C:"sv,                L"\\"sv,       {},        {},        },
		{ L"C:\\path"sv,                          L"C:"sv,                L"\\"sv,       L"path"sv, {},        },
		{ L"C:\\.ext"sv,                          L"C:"sv,                L"\\"sv,       {},        L".ext"sv, },
		{ L"C:\\path.ext"sv,                      L"C:"sv,                L"\\"sv,       L"path"sv, L".ext"sv, },
		{ L"C:\\path\\file"sv,                    L"C:"sv,                L"\\path\\"sv, L"file"sv, {},        },
		{ L"C:\\path\\.ext"sv,                    L"C:"sv,                L"\\path\\"sv, {},        L".ext"sv, },
		{ L"C:\\path\\file.ext"sv,                L"C:"sv,                L"\\path\\"sv, L"file"sv, L".ext"sv, },

		{ L"\\\\server\\share"sv,                 L"\\\\server\\share"sv, {},            {},        {},        },
		{ L"\\\\server\\share\\"sv,               L"\\\\server\\share"sv, L"\\"sv,       {},        {},        },
		{ L"\\\\server\\share\\path"sv,           L"\\\\server\\share"sv, L"\\"sv,       L"path"sv, {},        },
		{ L"\\\\server\\share\\.ext"sv,           L"\\\\server\\share"sv, L"\\"sv,       {},        L".ext"sv, },
		{ L"\\\\server\\share\\path.ext"sv,       L"\\\\server\\share"sv, L"\\"sv,       L"path"sv, L".ext"sv, },
		{ L"\\\\server\\share\\path\\file"sv,     L"\\\\server\\share"sv, L"\\path\\"sv, L"file"sv, {},        },
		{ L"\\\\server\\share\\path\\.ext"sv,     L"\\\\server\\share"sv, L"\\path\\"sv, {},        L".ext"sv, },
		{ L"\\\\server\\share\\path\\file.ext"sv, L"\\\\server\\share"sv, L"\\path\\"sv, L"file"sv, L".ext"sv, },
	};

	for (const auto& i: Tests)
	{
		for (const auto Flags: std::views::iota(1, 0b1111))
		{
			string Expected;

			if (Flags & sp_root)
				Expected += i.Root;
			if (Flags & sp_path)
				Expected += i.Path;
			if (Flags & sp_name)
				Expected += i.Name;
			if (Flags & sp_ext)
				Expected += i.Ext;

			string Actual;
			SplitPath(i.FullPath, Actual, Flags);

			REQUIRE(Expected == Actual);
		}
	}
}

TEST_CASE("macro.convert_size2str_flags")
{
	static const struct
	{
		uint64_t Input, Expected;
	}
	Tests[]
	{
		{ 0, 0 },
		{ 1, 0 },
		{ 0x1000000000000000, 0 },
		{ 0x0020000000000005, COLFLAGS_USE_MULTIPLIER | COLFLAGS_MULTIPLIER_E },
		{ as_unsigned(-1ll), COLFLAGS_SHOW_MULTIPLIER | COLFLAGS_GROUPDIGITS | COLFLAGS_FLOATSIZE | COLFLAGS_ECONOMIC | COLFLAGS_THOUSAND | COLFLAGS_USE_MULTIPLIER | COLFLAGS_MULTIPLIER_MASK },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(i.Expected == convert_size2str_flags(i.Input));
	}
}
#endif
