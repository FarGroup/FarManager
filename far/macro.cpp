/*
macro.cpp

Макросы
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

// FIXME: for SciTE only.
#if SCITE == 1
#define FAR_LUA
#endif

#ifdef FAR_LUA
#include "headers.hpp"
#pragma hdrstop

#include "macro.hpp"
#include "FarGuid.hpp"
#include "cmdline.hpp"
#include "config.hpp"
#include "configdb.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "frame.hpp" //???
#include "keyboard.hpp"
#include "keys.hpp" //FIXME
#include "lockscrn.hpp"
#include "manager.hpp"
#include "message.hpp"
#include "panel.hpp"
#include "scrbuf.hpp"
#include "strmix.hpp"
#include "syslog.hpp"

#include "macroopcode.hpp"
#include "interf.hpp"
#include "console.hpp"

void SZLOG (const char *fmt, ...)
{
	FILE* log=fopen("c:\\lua.log","at");
	if (log)
	{
		va_list argp;
		fprintf(log, "FAR: ");
		va_start(argp, fmt);
		vfprintf(log, fmt, argp);
		va_end(argp);
		fprintf(log, "\n");
		fclose(log);
	}
}

// для диалога назначения клавиши
struct DlgParam
{
	UINT64 Flags;
	KeyMacro *Handle;
	DWORD Key;
	int Mode;
	int Recurse;
	bool Changed;
};

struct TMacroKeywords
{
	const wchar_t *Name;   // Наименование
	DWORD Value;           // Значение
};

TMacroKeywords MKeywordsArea[] =
{
	{L"Other",                    MACRO_OTHER},
	{L"Shell",                    MACRO_SHELL},
	{L"Viewer",                   MACRO_VIEWER},
	{L"Editor",                   MACRO_EDITOR},
	{L"Dialog",                   MACRO_DIALOG},
	{L"Search",                   MACRO_SEARCH},
	{L"Disks",                    MACRO_DISKS},
	{L"MainMenu",                 MACRO_MAINMENU},
	{L"Menu",                     MACRO_MENU},
	{L"Help",                     MACRO_HELP},
	{L"Info",                     MACRO_INFOPANEL},
	{L"QView",                    MACRO_QVIEWPANEL},
	{L"Tree",                     MACRO_TREEPANEL},
	{L"FindFolder",               MACRO_FINDFOLDER},
	{L"UserMenu",                 MACRO_USERMENU},
	{L"Shell.AutoCompletion",     MACRO_SHELLAUTOCOMPLETION},
	{L"Dialog.AutoCompletion",    MACRO_DIALOGAUTOCOMPLETION},
	{L"Common",                   MACRO_COMMON},
};
TMacroKeywords MKeywordsFlags[] =
{
	// ФЛАГИ
	{L"DisableOutput",      MFLAGS_DISABLEOUTPUT},
	{L"RunAfterFARStart",   MFLAGS_RUNAFTERFARSTART},
	{L"EmptyCommandLine",   MFLAGS_EMPTYCOMMANDLINE},
	{L"NotEmptyCommandLine",MFLAGS_NOTEMPTYCOMMANDLINE},
	{L"EVSelection",        MFLAGS_EDITSELECTION},
	{L"NoEVSelection",      MFLAGS_EDITNOSELECTION},

	{L"NoFilePanels",       MFLAGS_NOFILEPANELS},
	{L"NoPluginPanels",     MFLAGS_NOPLUGINPANELS},
	{L"NoFolders",          MFLAGS_NOFOLDERS},
	{L"NoFiles",            MFLAGS_NOFILES},
	{L"Selection",          MFLAGS_SELECTION},
	{L"NoSelection",        MFLAGS_NOSELECTION},

	{L"NoFilePPanels",      MFLAGS_PNOFILEPANELS},
	{L"NoPluginPPanels",    MFLAGS_PNOPLUGINPANELS},
	{L"NoPFolders",         MFLAGS_PNOFOLDERS},
	{L"NoPFiles",           MFLAGS_PNOFILES},
	{L"PSelection",         MFLAGS_PSELECTION},
	{L"NoPSelection",       MFLAGS_PNOSELECTION},

	{L"NoSendKeysToPlugins",MFLAGS_NOSENDKEYSTOPLUGINS},
};
const wchar_t* GetAreaName(DWORD AreaValue) {return GetNameOfValue(AreaValue, MKeywordsArea);}

const string FlagsToString(FARKEYMACROFLAGS Flags)
{
	return FlagsToString(Flags, MKeywordsFlags);
}

FARKEYMACROFLAGS StringToFlags(const string& strFlags)
{
	return StringToFlags(strFlags, MKeywordsFlags);
}

MacroRecord::MacroRecord(): m_area(MACRO_COMMON),m_flags(0),m_guid(FarGuid),m_id(nullptr),m_callback(nullptr)
{
}

MacroRecord::MacroRecord(MACROMODEAREA Area,MACROFLAGS_MFLAGS Flags,string Name,string Code,string Description): m_area(Area),m_flags(Flags),m_name(Name),m_code(Code),m_description(Description),m_guid(FarGuid),m_id(nullptr),m_callback(nullptr)
{
}

MacroRecord& MacroRecord::operator= (const MacroRecord& src)
{
	if (this != &src)
	{
		m_area = src.m_area;
		m_flags = src.m_flags;
		m_name = src.m_name;
		m_code = src.m_code;
		m_description = src.m_description;
		m_guid = src.m_guid;
		m_id = src.m_id;
		m_callback = src.m_callback;
	}
	return *this;
}

KeyMacro::KeyMacro(): m_Mode(MACRO_SHELL),m_Recording(MACROMODE_NOMACRO),m_RecMode(MACRO_OTHER),m_LockScr(nullptr)
{
	m_State.Push(nullptr);
}

KeyMacro::~KeyMacro()
{
}

int KeyMacro::IsRecording()
{
	return m_Recording;
}

int KeyMacro::IsExecuting()
{
	return m_RunState ?	(m_RunState.m_flags&MFLAGS_NOSENDKEYSTOPLUGINS) ?
		MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON:MACROMODE_NOMACRO;
}

int KeyMacro::IsExecutingLastKey()
{
	return false;
}

int KeyMacro::IsDsableOutput()
{
	return m_RunState && (m_RunState.m_flags&MFLAGS_DISABLEOUTPUT);
}

bool KeyMacro::IsHistoryDisable(int TypeHistory)
{
	return false;
}

void KeyMacro::SetMode(int Mode) //FIXME: int->MACROMODEAREA
{
	m_Mode=(MACROMODEAREA)Mode;
}

MACROMODEAREA KeyMacro::GetMode(void)
{
	return m_Mode;
}

bool KeyMacro::LoadMacros(bool InitedRAM,bool LoadAll)
{
	int ErrCount=0;

	for (int k=0; k<MACRO_LAST; k++)
	{
		m_Macros[k].Free();
	}

	if (Opt.Macro.DisableMacro&MDOL_ALL)
		return false;

	int Areas[MACRO_LAST];

	for (int ii=MACRO_OTHER;ii<MACRO_LAST;++ii)
	{
		Areas[ii]=ii;
	}

	if (!LoadAll)
	{
		// "выведем из строя" ненужные области - будет загружаться только то, что не равно значению MACRO_LAST
		Areas[MACRO_SHELL]=
			Areas[MACRO_SEARCH]=
			Areas[MACRO_DISKS]=
			Areas[MACRO_MAINMENU]=
			Areas[MACRO_INFOPANEL]=
			Areas[MACRO_QVIEWPANEL]=
			Areas[MACRO_TREEPANEL]=
			Areas[MACRO_USERMENU]= // <-- Mantis#0001594
			Areas[MACRO_SHELLAUTOCOMPLETION]=
			Areas[MACRO_FINDFOLDER]=MACRO_LAST;
	}

	for (int ii=MACRO_OTHER;ii<MACRO_LAST;++ii)
	{
		if (Areas[ii]==MACRO_LAST) continue;
		if (!ReadMacro((MACROMODEAREA)ii))
		{
			ErrCount++;
		}
	}

/*
	for(size_t ii=0;ii<MACRO_LAST;++ii)
	{
		{FILE* log=fopen("c:\\plugins.log","at"); if(log) {fprintf(log,"count: %d,%d\n",m_Macros[ii].getSize(),ii); fclose(log);}}
		for(size_t jj=0;jj<m_Macros[ii].getSize();++jj)
		{
			{FILE* log=fopen("c:\\plugins.log","at"); if(log) {fprintf(log,"%ls\n",m_Macros[ii].getItem(jj)->Code().CPtr()); fclose(log);}}
		}
	}
*/
	return ErrCount?false:true;
}

void KeyMacro::SaveMacros()
{
	WriteMacro();
}

int KeyMacro::GetCurRecord(struct MacroRecord* RBuf,int *KeyPos)
{
	return (m_Recording != MACROMODE_NOMACRO) ? m_Recording : IsExecuting();
}

void* KeyMacro::CallPlugin(unsigned Type,void* Data)
{
	void* ptr;
	m_State.Push(nullptr);

	int lockCount=ScrBuf.GetLockCount();
	ScrBuf.SetLockCount(0);
	bool result=CtrlObject->Plugins->CallPlugin(LuamacroGuid,Type,Data,&ptr);
	ScrBuf.SetLockCount(lockCount);

	RunState dummy;
	m_State.Pop(dummy);
	return result?ptr:nullptr;
}

bool KeyMacro::InitMacroExecution(MacroRecord* macro)
{
	FarMacroValue values[2]={{FMVT_STRING,{0}},{FMVT_STRING,{0}}};
	values[0].String=macro->Code();
	values[1].String=macro->Name();
	OpenMacroInfo info={sizeof(OpenMacroInfo),ARRAYSIZE(values),values};
	void* handle=CallPlugin(OPEN_MACROINIT,&info);
	if (handle)
	{
		//{FILE* log=fopen("c:\\lua.log","at"); if(log) {fprintf(log,"handle: %p\n",handle); fclose(log);}}
		m_RunState = RunState(handle,macro->m_flags);
		*m_State.Peek()=m_RunState;
		m_LastKey = L"first_key";
		return true;
	}
	return false;
}

int KeyMacro::ProcessEvent(const struct FAR_INPUT_RECORD *Rec)
{
	if (Rec->IntKey==KEY_IDLE || Rec->IntKey==KEY_NONE || !FrameManager->GetCurrentFrame()) //FIXME: избавиться от Rec->IntKey
		return false;
	//{FILE* log=fopen("c:\\lua.log","at"); if(log) {fprintf(log,"ProcessEvent: %08x\n",Rec->IntKey); fclose(log);}}
	string key;
	//if (InputRecordToText(&Rec->Rec,key))//FIXME: на голом Ctrl даёт код символа, а не текст.
	if (KeyToText(Rec->IntKey,key))
	{
		bool ctrldot=(0==StrCmpI(key,L"Ctrl.")||0==StrCmpI(key,L"RCtrl."));
		bool ctrlshiftdot=(0==StrCmpI(key,L"CtrlShift.")||0==StrCmpI(key,L"RCtrlShift."));

		if (m_Recording==MACROMODE_NOMACRO)
		{
			if (ctrldot||ctrlshiftdot)
			{
				// Полиция 18
				if (Opt.Policies.DisabledOptions&FFPOL_CREATEMACRO)
					return false;

				UpdateLockScreen(false);

				// Где мы?
				m_RecMode=(m_Mode==MACRO_SHELL&&!WaitInMainLoop)?MACRO_OTHER:m_Mode;
				StartMode=m_RecMode;
				// В зависимости от того, КАК НАЧАЛИ писать макрос, различаем общий режим (Ctrl-.
				// с передачей плагину кеев) или специальный (Ctrl-Shift-. - без передачи клавиш плагину)
				m_Recording=ctrldot?MACROMODE_RECORDING_COMMON:MACROMODE_RECORDING;

				m_RecCode.Clear();
				m_RecDescription.Clear();
				ScrBuf.ResetShadow();
				ScrBuf.Flush();
				WaitInFastFind--;
				return true;
			}
			else
			{
				if (!*m_State.Peek())
				{
					int Area, Index;
					Index = GetIndex(&Area,Rec->IntKey,key,m_Mode,true,true);
					if (Index != -1)
					{
						MacroRecord* macro = m_Macros[Area].getItem(Index);
						if (CheckAll(macro->Flags()) && (!macro->m_callback||macro->m_callback(macro->m_id,AKMFLAGS_NONE)))
						{
							return InitMacroExecution(macro);
						}
					}
				}
			}
		}
		else // m_Recording!=MACROMODE_NOMACRO
		{
			if (ctrldot||ctrlshiftdot)
			{
				// Залочить _текущий_ фрейм, а не _последний немодальный_
				FrameManager->GetCurrentFrame()->Lock(); // отменим прорисовку фрейма
				DWORD MacroKey;
				// выставляем флаги по умолчанию.
				UINT64 Flags=MFLAGS_DISABLEOUTPUT|MFLAGS_CALLPLUGINENABLEMACRO; // ???
				int AssignRet=AssignMacroKey(MacroKey,Flags);
				FrameManager->ResetLastInputRecord();
				FrameManager->GetCurrentFrame()->Unlock(); // теперь можно :-)

				if (AssignRet && AssignRet!=2 && !m_RecCode.IsEmpty())
				{
					m_RecCode = L"SendKeys(\"" + m_RecCode + L"\")";
					// добавим проверку на удаление
					// если удаляем или был вызван диалог изменения, то не нужно выдавать диалог настройки.
					//if (MacroKey != (DWORD)-1 && (Key==KEY_CTRLSHIFTDOT || Recording==2) && RecBufferSize)
					if (ctrlshiftdot && !GetMacroSettings(MacroKey,Flags))
					{
						AssignRet=0;
					}
				}

				//> FIXME:
				//> WaitInMainLoop=WaitInMainLoop0;
				//> InternalInput=FALSE;
				if (AssignRet)
				{
					int Area, Pos;
					string strKey;
					KeyToText(MacroKey, strKey);

					// в области common будем искать только при удалении
					Pos = GetIndex(&Area,MacroKey,strKey,StartMode,m_RecCode.IsEmpty(),true);
					Flags |= MFLAGS_NEEDSAVEMACRO|(m_Recording==MACROMODE_RECORDING_COMMON?0:MFLAGS_NOSENDKEYSTOPLUGINS);

					if (Pos == -1)
					{
						if (!m_RecCode.IsEmpty())
						{
							m_Macros[StartMode].addItem(MacroRecord(StartMode,Flags,strKey,m_RecCode,m_RecDescription));
						}
					}
					else
					{
						MacroRecord* macro = m_Macros[Area].getItem(Pos);
						if (!m_RecCode.IsEmpty())
						{
							macro->m_flags = Flags;
							macro->m_code = m_RecCode;
							macro->m_description = m_RecDescription;
						}
						else
						{
							macro->m_flags = MFLAGS_NEEDSAVEMACRO|MFLAGS_DISABLEMACRO;
							macro->m_code.Clear();
						}
					}
				}

				//{FILE* log=fopen("c:\\plugins.log","at"); if(log) {fprintf(log,"%ls\n",m_RecCode.CPtr()); fclose(log);}}
				m_Recording=MACROMODE_NOMACRO;
				m_RecCode.Clear();
				m_RecDescription.Clear();
				ScrBuf.RestoreMacroChar();
				WaitInFastFind++;

				if (Opt.AutoSaveSetup)
					WriteMacro(); // записать только изменения!

				return true;
			}
			else
			{
				//{FILE* log=fopen("c:\\plugins.log","at"); if(log) {fprintf(log,"key: %08x\n",Rec->IntKey); fclose(log);}}
				if (!IsProcessAssignMacroKey)
				{
					if(!m_RecCode.IsEmpty()) m_RecCode+=L" ";
					m_RecCode+=key;
				}
				return false;
			}
		}
	}
	return false;
}

int KeyMacro::GetKey()
{
	if(*m_State.Peek())
	{
		//{FILE* log=fopen("c:\\lua.log","at"); if(log) {fprintf(log,"+GetKey: %d\n",m_State.size()); fclose(log);}}
		wchar_t* key=(wchar_t*)CallPlugin(OPEN_MACROSTEP,m_State.Peek()->m_handle);
		if (key)
		{
			//{FILE* log=fopen("c:\\lua.log","at"); if(log) {fprintf(log,"result: %ls\n\n",key); fclose(log);}}
			if(key[0])
			{
				if ((m_RunState.m_flags&MFLAGS_DISABLEOUTPUT) && ScrBuf.GetLockCount()==0)
				{
					ScrBuf.Lock();
				}
				int iKey = KeyNameToKey(key);
				m_LastKey = key;
				return iKey==-1 ? KEY_NONE:iKey;
			}
			else
			{
				if (CallPlugin(OPEN_MACROFINAL,m_State.Peek()->m_handle)); //FIXME: process condition.
			}
		}
		if (m_RunState.m_flags & MFLAGS_DISABLEOUTPUT)
			ScrBuf.Unlock();

		if (m_State.size()==1)
			ScrBuf.RestoreMacroChar();

		m_RunState=nullptr;
		*m_State.Peek()=nullptr;
		//{FILE* log=fopen("c:\\lua.log","at"); if(log) {fprintf(log,"-GetKey: %d\n",m_State.size()); fclose(log);}}
	}

	if (IsExecuting()==MACROMODE_NOMACRO && !m_MacroQueue.Empty())
	{
		MacroRecord macro = *m_MacroQueue.First();
		m_MacroQueue.Delete(m_MacroQueue.First());
		if (InitMacroExecution(&macro))
		{
			return GetKey();
		}
	}

	return 0;
}

int KeyMacro::PeekKey()
{
	//{FILE* log=fopen("c:\\lua.log","at"); if(log) {fprintf(log,"PeekKey\n"); fclose(log);}}
	int key=0;
	if (IsExecuting())
	{
		if (!StrCmp(m_LastKey, L"first_key")) //FIXME
			return KEY_NONE;
		if ((key=KeyNameToKey(m_LastKey)) == -1)
			key=0;
	}
	return key;
}

// получить код моды по имени
int KeyMacro::GetAreaCode(const wchar_t *AreaName)
{
	for (int i=MACRO_OTHER; i < MACRO_LAST; i++)
		if (!StrCmpI(MKeywordsArea[i].Name,AreaName))
			return i;

	return -4; //FIXME: MACRO_FUNCS-1;
}

int KeyMacro::GetMacroKeyInfo(bool FromDB, int Mode, int Pos, string &strKeyName, string &strDescription)
{
	const int MACRO_FUNCS = -3;
	if (Mode >= MACRO_FUNCS && Mode < MACRO_LAST)
	{
		if (FromDB)
		{
			if (Mode >= MACRO_OTHER)
			{
				// TODO
				return Pos+1;
			}
			else if (Mode == MACRO_FUNCS)
			{
				// TODO: MACRO_FUNCS
				return -1;
			}
			else
			{
				// TODO
				return Pos+1;
			}
		}
		else
		{
			if (Mode >= MACRO_OTHER)
			{
				int Len=CtrlObject->Macro.m_Macros[Mode].getSize();
				if (Len && Pos < Len)
				{
					MacroRecord *MPtr=CtrlObject->Macro.m_Macros[Mode].getItem(Pos);
					strKeyName=MPtr->Name();
					strDescription=NullToEmpty(MPtr->Description());
					return Pos+1;
				}
			}
		}
	}

	return -1;
}

void KeyMacro::SendDropProcess()
{
}

bool KeyMacro::CheckWaitKeyFunc()
{
	return false;
}

// Функция получения индекса нужного макроса в массиве
// Ret=-1 - не найден таковой.
// если CheckMode=-1 - значит пофигу в каком режиме, т.е. первый попавшийся
// StrictKeys=true - не пытаться подменить Левый Ctrl/Alt Правым (если Левый не нашли)
// FIXME: parameter StrictKeys.
int KeyMacro::GetIndex(int* area, int Key, string& strKey, int CheckMode, bool UseCommon, bool StrictKeys)
{
	//{FILE* log=fopen("c:\\plugins.log","at"); if(log) {fprintf(log,"GetIndex: %08x,%ls\n",Key,strKey.CPtr()); fclose(log);}}
	const wchar_t *pKey=strKey;
	string sTemp;
	if (*pKey==0 && KeyToText(Key,sTemp))
		pKey=sTemp;

	int startindex = (CheckMode==-1) ? 0:CheckMode;
	int endindex = (CheckMode==-1) ? MACRO_LAST:CheckMode+1;
	for (int i=startindex; i<endindex; i++)
	{
		for (unsigned j=0; j<m_Macros[i].getSize(); j++)
		{
			MacroRecord* macro = m_Macros[i].getItem(j);
			if (!StrCmpI(macro->Name(),pKey) && !(macro->Flags()&MFLAGS_DISABLEMACRO))
			{
				*area = i;
				return j;
			}
		}
	}
	if (UseCommon && CheckMode!=-1)
		return GetIndex(area, Key, strKey, MACRO_COMMON, false, StrictKeys);

	*area = -1;
	return -1;
}

// Функция, запускающая макросы при старте ФАРа
void KeyMacro::RunStartMacro()
{
	if ((Opt.Macro.DisableMacro&MDOL_ALL) || (Opt.Macro.DisableMacro&MDOL_AUTOSTART) || Opt.OnlyEditorViewerUsed)
		return;

	if (!CtrlObject || !CtrlObject->Cp() || !CtrlObject->Cp()->ActivePanel || !CtrlObject->Plugins->IsPluginsLoaded())
		return;

	static int IsRunStartMacro=FALSE;
	if (IsRunStartMacro)
		return;

	for (unsigned j=0; j<m_Macros[MACRO_SHELL].getSize(); j++)
	{
		MacroRecord* macro = m_Macros[MACRO_SHELL].getItem(j);
		MACROFLAGS_MFLAGS flags = macro->Flags();
		if (!(flags&MFLAGS_DISABLEMACRO) && (flags&MFLAGS_RUNAFTERFARSTART) && CheckAll(flags))
		{
			PostNewMacro(macro->Code(), flags&~MFLAGS_DISABLEOUTPUT); //FIXME
		}
	}
	IsRunStartMacro=TRUE;
}

int KeyMacro::AddMacro(const wchar_t *PlainText,const wchar_t *Description,enum MACROMODEAREA Area,MACROFLAGS_MFLAGS Flags,const INPUT_RECORD& AKey,const GUID& PluginId,void* Id,FARMACROCALLBACK Callback)
{
	if (Area < 0 || Area >= MACRO_LAST)
		return FALSE;

	string strKeyText;
	if (!(InputRecordToText(&AKey, strKeyText) && ParseMacroString(PlainText,true)))
		return FALSE;

	MacroRecord macro(Area,Flags,strKeyText,PlainText,Description);
	macro.m_guid = PluginId;
	macro.m_id = Id;
	macro.m_callback = Callback;
	m_Macros[Area].addItem(macro);
	return TRUE;
}

int KeyMacro::DelMacro(const GUID& PluginId,void* Id)
{
	for (int i=0; i<MACRO_LAST; i++)
	{
		for (unsigned j=0; j<m_Macros[i].getSize(); j++)
		{
			MacroRecord* macro = m_Macros[i].getItem(j);
			if (!(macro->m_flags&MFLAGS_DISABLEMACRO) && macro->m_id==Id && IsEqualGUID(macro->m_guid,PluginId))
			{
				macro->m_flags = MFLAGS_DISABLEMACRO;
				macro->m_name.Clear();
				macro->m_code.Clear();
				macro->m_description.Clear();
				return TRUE;
			}
		}
	}
	return FALSE;
}

int KeyMacro::PostNewMacro(const wchar_t *PlainText,UINT64 Flags,DWORD AKey,bool onlyCheck)
{
	if (ParseMacroString(PlainText, onlyCheck))
	{
		string strKeyText;
		KeyToText(AKey,strKeyText);
		MacroRecord macro(MACRO_COMMON, Flags, strKeyText, PlainText, L"");
		m_MacroQueue.Push(&macro);
		return TRUE;
	}
	return FALSE;
}

bool KeyMacro::ReadMacro(MACROMODEAREA Area)
{
	unsigned __int64 MFlags=0;
	string strKey,strArea,strMFlags;
	string strSequence, strDescription;
	string strGUID;
	int ErrorCount=0;

	strArea=GetAreaName(static_cast<MACROMODEAREA>(Area));

	while(MacroCfg->EnumKeyMacros(strArea, strKey, strMFlags, strSequence, strDescription))
	{
		RemoveExternalSpaces(strKey);
		RemoveExternalSpaces(strSequence);
		RemoveExternalSpaces(strDescription);

		if (strSequence.IsEmpty())
		{
			//ErrorCount++; // Раскомментить, если не допускается пустой "Sequence"
			continue;
		}
		if (!ParseMacroString(strSequence))
		{
			ErrorCount++;
			continue;
		}

		MFlags=StringToFlags(strMFlags);

		if (Area == MACRO_EDITOR || Area == MACRO_DIALOG || Area == MACRO_VIEWER)
		{
			if (MFlags&MFLAGS_SELECTION)
			{
				MFlags&=~MFLAGS_SELECTION;
				MFlags|=MFLAGS_EDITSELECTION;
			}

			if (MFlags&MFLAGS_NOSELECTION)
			{
				MFlags&=~MFLAGS_NOSELECTION;
				MFlags|=MFLAGS_EDITNOSELECTION;
			}
		}
		m_Macros[Area].addItem(MacroRecord(Area,MFlags,strKey,strSequence,strDescription));
	}

	return ErrorCount?false:true;
}

void KeyMacro::WriteMacro(void)
{
	MacroCfg->BeginTransaction();
	for(size_t ii=MACRO_OTHER;ii<MACRO_LAST;++ii)
	{
		for(size_t jj=0;jj<m_Macros[ii].getSize();++jj)
		{
			MacroRecord& rec=*m_Macros[ii].getItem(jj);
			if (rec.IsSave())
			{
				rec.ClearSave();
				string Code = rec.Code();
				RemoveExternalSpaces(Code);
				if (Code.IsEmpty())
					MacroCfg->DeleteKeyMacro(GetAreaName(rec.Area()), rec.Name());
				else
					MacroCfg->SetKeyMacro(GetAreaName(rec.Area()),rec.Name(),FlagsToString(rec.Flags()),Code,rec.Description());
			}
		}
	}
	MacroCfg->EndTransaction();
}

const wchar_t *eStackAsString(int Pos)
{
	return L"";
}

BOOL KeyMacro::CheckEditSelected(UINT64 CurFlags)
{
	if (m_Mode==MACRO_EDITOR || m_Mode==MACRO_DIALOG || m_Mode==MACRO_VIEWER || (m_Mode==MACRO_SHELL&&CtrlObject->CmdLine->IsVisible()))
	{
		int NeedType = m_Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(m_Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(m_Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS));
		Frame* CurFrame=FrameManager->GetCurrentFrame();

		if (CurFrame && CurFrame->GetType()==NeedType)
		{
			int CurSelected;

			if (m_Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
			{
				int SelStart,SelEnd;
				CtrlObject->CmdLine->GetSelection(SelStart,SelEnd);
				CurSelected = (SelStart != -1 && SelStart < SelEnd);
			}
			else
				CurSelected = FALSE;

			if (((CurFlags&MFLAGS_EDITSELECTION) && !CurSelected) ||	((CurFlags&MFLAGS_EDITNOSELECTION) && CurSelected))
				return FALSE;
		}
	}

	return TRUE;
}

BOOL KeyMacro::CheckInsidePlugin(UINT64 CurFlags)
{
	if (CtrlObject && CtrlObject->Plugins->CurPluginItem && (CurFlags&MFLAGS_NOSENDKEYSTOPLUGINS)) // ?????
		//if(CtrlObject && CtrlObject->Plugins->CurEditor && (CurFlags&MFLAGS_NOSENDKEYSTOPLUGINS))
		return FALSE;

	return TRUE;
}

BOOL KeyMacro::CheckCmdLine(int CmdLength,UINT64 CurFlags)
{
	if (((CurFlags&MFLAGS_EMPTYCOMMANDLINE) && CmdLength) || ((CurFlags&MFLAGS_NOTEMPTYCOMMANDLINE) && CmdLength==0))
		return FALSE;

	return TRUE;
}

BOOL KeyMacro::CheckPanel(int PanelMode,UINT64 CurFlags,BOOL IsPassivePanel)
{
	if (IsPassivePanel)
	{
		if ((PanelMode == PLUGIN_PANEL && (CurFlags&MFLAGS_PNOPLUGINPANELS)) || (PanelMode == NORMAL_PANEL && (CurFlags&MFLAGS_PNOFILEPANELS)))
			return FALSE;
	}
	else
	{
		if ((PanelMode == PLUGIN_PANEL && (CurFlags&MFLAGS_NOPLUGINPANELS)) || (PanelMode == NORMAL_PANEL && (CurFlags&MFLAGS_NOFILEPANELS)))
			return FALSE;
	}

	return TRUE;
}

BOOL KeyMacro::CheckFileFolder(Panel *CheckPanel,UINT64 CurFlags, BOOL IsPassivePanel)
{
	string strFileName;
	DWORD FileAttr=INVALID_FILE_ATTRIBUTES;
	CheckPanel->GetFileName(strFileName,CheckPanel->GetCurrentPos(),FileAttr);

	if (FileAttr != INVALID_FILE_ATTRIBUTES)
	{
		if (IsPassivePanel)
		{
			if (((FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_PNOFOLDERS)) || (!(FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_PNOFILES)))
				return FALSE;
		}
		else
		{
			if (((FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_NOFOLDERS)) || (!(FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_NOFILES)))
				return FALSE;
		}
	}

	return TRUE;
}

BOOL KeyMacro::CheckAll(UINT64 CurFlags)
{
	/* $TODO:
		Здесь вместо Check*() попробовать заюзать IfCondition()
		для исключения повторяющегося кода.
	*/
	if (!CheckInsidePlugin(CurFlags))
		return FALSE;

	// проверка на пусто/не пусто в ком.строке (а в редакторе? :-)
	if (CurFlags&(MFLAGS_EMPTYCOMMANDLINE|MFLAGS_NOTEMPTYCOMMANDLINE))
		if (CtrlObject->CmdLine && !CheckCmdLine(CtrlObject->CmdLine->GetLength(),CurFlags))
			return FALSE;

	FilePanels *Cp=CtrlObject->Cp();

	if (!Cp)
		return FALSE;

	// проверки панели и типа файла
	Panel *ActivePanel=Cp->ActivePanel;
	Panel *PassivePanel=Cp->GetAnotherPanel(Cp->ActivePanel);

	if (ActivePanel && PassivePanel)// && (CurFlags&MFLAGS_MODEMASK)==MACRO_SHELL)
	{
		if (CurFlags&(MFLAGS_NOPLUGINPANELS|MFLAGS_NOFILEPANELS))
			if (!CheckPanel(ActivePanel->GetMode(),CurFlags,FALSE))
				return FALSE;

		if (CurFlags&(MFLAGS_PNOPLUGINPANELS|MFLAGS_PNOFILEPANELS))
			if (!CheckPanel(PassivePanel->GetMode(),CurFlags,TRUE))
				return FALSE;

		if (CurFlags&(MFLAGS_NOFOLDERS|MFLAGS_NOFILES))
			if (!CheckFileFolder(ActivePanel,CurFlags,FALSE))
				return FALSE;

		if (CurFlags&(MFLAGS_PNOFOLDERS|MFLAGS_PNOFILES))
			if (!CheckFileFolder(PassivePanel,CurFlags,TRUE))
				return FALSE;

		if (CurFlags&(MFLAGS_SELECTION|MFLAGS_NOSELECTION|MFLAGS_PSELECTION|MFLAGS_PNOSELECTION))
			if (m_Mode!=MACRO_EDITOR && m_Mode != MACRO_DIALOG && m_Mode!=MACRO_VIEWER)
			{
				size_t SelCount=ActivePanel->GetRealSelCount();

				if (((CurFlags&MFLAGS_SELECTION) && SelCount < 1) || ((CurFlags&MFLAGS_NOSELECTION) && SelCount >= 1))
					return FALSE;

				SelCount=PassivePanel->GetRealSelCount();

				if (((CurFlags&MFLAGS_PSELECTION) && SelCount < 1) || ((CurFlags&MFLAGS_PNOSELECTION) && SelCount >= 1))
					return FALSE;
			}
	}

	if (!CheckEditSelected(CurFlags))
		return FALSE;

	return TRUE;
}

static int Set3State(DWORD Flags,DWORD Chk1,DWORD Chk2)
{
	DWORD Chk12=Chk1|Chk2, FlagsChk12=Flags&Chk12;

	if (FlagsChk12 == Chk12 || !FlagsChk12)
		return (2);
	else
		return (Flags&Chk1?1:0);
}

enum MACROSETTINGSDLG
{
	MS_DOUBLEBOX,
	MS_TEXT_SEQUENCE,
	MS_EDIT_SEQUENCE,
	MS_TEXT_DESCR,
	MS_EDIT_DESCR,
	MS_SEPARATOR1,
	MS_CHECKBOX_OUPUT,
	MS_CHECKBOX_START,
	MS_SEPARATOR2,
	MS_CHECKBOX_A_PANEL,
	MS_CHECKBOX_A_PLUGINPANEL,
	MS_CHECKBOX_A_FOLDERS,
	MS_CHECKBOX_A_SELECTION,
	MS_CHECKBOX_P_PANEL,
	MS_CHECKBOX_P_PLUGINPANEL,
	MS_CHECKBOX_P_FOLDERS,
	MS_CHECKBOX_P_SELECTION,
	MS_SEPARATOR3,
	MS_CHECKBOX_CMDLINE,
	MS_CHECKBOX_SELBLOCK,
	MS_SEPARATOR4,
	MS_BUTTON_OK,
	MS_BUTTON_CANCEL,
};

intptr_t WINAPI KeyMacro::ParamMacroDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	static DlgParam *KMParam=nullptr;

	switch (Msg)
	{
		case DN_INITDIALOG:
			KMParam=(DlgParam *)Param2;
			break;
		case DN_BTNCLICK:

			if (Param1==MS_CHECKBOX_A_PANEL || Param1==MS_CHECKBOX_P_PANEL)
				for (int i=1; i<=3; i++)
					SendDlgMessage(hDlg,DM_ENABLE,Param1+i,Param2);

			break;
		case DN_CLOSE:

			if (Param1==MS_BUTTON_OK)
			{
				LPCWSTR Sequence=(LPCWSTR)SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,MS_EDIT_SEQUENCE,0);
				if (*Sequence)
				{
					KeyMacro *Macro=KMParam->Handle;
					if (Macro->ParseMacroString(Sequence))
					{
						Macro->m_RecCode=Sequence;
						Macro->m_RecDescription=(LPCWSTR)SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,MS_EDIT_DESCR,0);
						return TRUE;
					}
				}

				return FALSE;
			}

			break;

		default:
			break;
	}

	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

int KeyMacro::GetMacroSettings(int Key,UINT64 &Flags,const wchar_t *Src,const wchar_t *Descr)
{
	/*
	          1         2         3         4         5         6
	   3456789012345678901234567890123456789012345678901234567890123456789
	 1 г=========== Параметры макрокоманды для 'CtrlP' ==================¬
	 2 | Последовательность:                                             |
	 3 | _______________________________________________________________ |
	 4 | Описание:                                                       |
	 5 | _______________________________________________________________ |
	 6 |-----------------------------------------------------------------|
	 7 | [ ] Разрешить во время выполнения вывод на экран                |
	 8 | [ ] Выполнять после запуска FAR                                 |
	 9 |-----------------------------------------------------------------|
	10 | [ ] Активная панель             [ ] Пассивная панель            |
	11 |   [?] На панели плагина           [?] На панели плагина         |
	12 |   [?] Выполнять для папок         [?] Выполнять для папок       |
	13 |   [?] Отмечены файлы              [?] Отмечены файлы            |
	14 |-----------------------------------------------------------------|
	15 | [?] Пустая командная строка                                     |
	16 | [?] Отмечен блок                                                |
	17 |-----------------------------------------------------------------|
	18 |               [ Продолжить ]  [ Отменить ]                      |
	19 L=================================================================+

	*/
	FarDialogItem MacroSettingsDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,69,19,0,nullptr,nullptr,0,L""},
		{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(MMacroSequence)},
		{DI_EDIT,5,3,67,3,0,L"MacroSequence",nullptr,DIF_FOCUS|DIF_HISTORY,L""},
		{DI_TEXT,5,4,0,4,0,nullptr,nullptr,0,MSG(MMacroDescription)},
		{DI_EDIT,5,5,67,5,0,L"MacroDescription",nullptr,DIF_HISTORY,L""},

		{DI_TEXT,3,6,0,6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,7,0,7,0,nullptr,nullptr,0,MSG(MMacroSettingsEnableOutput)},
		{DI_CHECKBOX,5,8,0,8,0,nullptr,nullptr,0,MSG(MMacroSettingsRunAfterStart)},
		{DI_TEXT,3,9,0,9,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,10,0,10,0,nullptr,nullptr,0,MSG(MMacroSettingsActivePanel)},
		{DI_CHECKBOX,7,11,0,11,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsPluginPanel)},
		{DI_CHECKBOX,7,12,0,12,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsFolders)},
		{DI_CHECKBOX,7,13,0,13,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsSelectionPresent)},
		{DI_CHECKBOX,37,10,0,10,0,nullptr,nullptr,0,MSG(MMacroSettingsPassivePanel)},
		{DI_CHECKBOX,39,11,0,11,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsPluginPanel)},
		{DI_CHECKBOX,39,12,0,12,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsFolders)},
		{DI_CHECKBOX,39,13,0,13,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsSelectionPresent)},
		{DI_TEXT,3,14,0,14,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,15,0,15,2,nullptr,nullptr,DIF_3STATE,MSG(MMacroSettingsCommandLine)},
		{DI_CHECKBOX,5,16,0,16,2,nullptr,nullptr,DIF_3STATE,MSG(MMacroSettingsSelectionBlockPresent)},
		{DI_TEXT,3,17,0,17,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	MakeDialogItemsEx(MacroSettingsDlgData,MacroSettingsDlg);
	string strKeyText;
	KeyToText(Key,strKeyText);
	MacroSettingsDlg[MS_DOUBLEBOX].strData = LangString(MMacroSettingsTitle) << strKeyText;
	//if(!(Key&0x7F000000))
	//MacroSettingsDlg[3].Flags|=DIF_DISABLE;
	MacroSettingsDlg[MS_CHECKBOX_OUPUT].Selected=Flags&MFLAGS_DISABLEOUTPUT?0:1;
	MacroSettingsDlg[MS_CHECKBOX_START].Selected=Flags&MFLAGS_RUNAFTERFARSTART?1:0;
	MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected=Set3State(Flags,MFLAGS_NOFILEPANELS,MFLAGS_NOPLUGINPANELS);
	MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected=Set3State(Flags,MFLAGS_NOFILES,MFLAGS_NOFOLDERS);
	MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected=Set3State(Flags,MFLAGS_SELECTION,MFLAGS_NOSELECTION);
	MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected=Set3State(Flags,MFLAGS_PNOFILEPANELS,MFLAGS_PNOPLUGINPANELS);
	MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected=Set3State(Flags,MFLAGS_PNOFILES,MFLAGS_PNOFOLDERS);
	MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected=Set3State(Flags,MFLAGS_PSELECTION,MFLAGS_PNOSELECTION);
	MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected=Set3State(Flags,MFLAGS_EMPTYCOMMANDLINE,MFLAGS_NOTEMPTYCOMMANDLINE);
	MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected=Set3State(Flags,MFLAGS_EDITSELECTION,MFLAGS_EDITNOSELECTION);
	if (Src && *Src)
	{
		MacroSettingsDlg[MS_EDIT_SEQUENCE].strData=Src;
	}
	else
	{
		MacroSettingsDlg[MS_EDIT_SEQUENCE].strData=m_RecCode;
	}

	MacroSettingsDlg[MS_EDIT_DESCR].strData=(Descr && *Descr)?Descr:(const wchar_t*)m_RecDescription;

	DlgParam Param={0, this, 0, 0, 0, false};
	Dialog Dlg(MacroSettingsDlg,ARRAYSIZE(MacroSettingsDlg),ParamMacroDlgProc,&Param);
	Dlg.SetPosition(-1,-1,73,21);
	Dlg.SetHelp(L"KeyMacroSetting");
	Frame* BottomFrame = FrameManager->GetBottomFrame();
	if(BottomFrame)
	{
		BottomFrame->Lock(); // отменим прорисовку фрейма
	}
	Dlg.Process();
	if(BottomFrame)
	{
		BottomFrame->Unlock(); // теперь можно :-)
	}

	if (Dlg.GetExitCode()!=MS_BUTTON_OK)
		return FALSE;

	Flags=MacroSettingsDlg[MS_CHECKBOX_OUPUT].Selected?0:MFLAGS_DISABLEOUTPUT;
	Flags|=MacroSettingsDlg[MS_CHECKBOX_START].Selected?MFLAGS_RUNAFTERFARSTART:0;

	if (MacroSettingsDlg[MS_CHECKBOX_A_PANEL].Selected)
	{
		Flags|=MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected==0?MFLAGS_NOPLUGINPANELS:MFLAGS_NOFILEPANELS);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected==0?MFLAGS_NOFOLDERS:MFLAGS_NOFILES);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected==0?MFLAGS_NOSELECTION:MFLAGS_SELECTION);
	}

	if (MacroSettingsDlg[MS_CHECKBOX_P_PANEL].Selected)
	{
		Flags|=MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected==0?MFLAGS_PNOPLUGINPANELS:MFLAGS_PNOFILEPANELS);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected==0?MFLAGS_PNOFOLDERS:MFLAGS_PNOFILES);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected==0?MFLAGS_PNOSELECTION:MFLAGS_PSELECTION);
	}

	Flags|=MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected==2?0:
	       (MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected==0?MFLAGS_NOTEMPTYCOMMANDLINE:MFLAGS_EMPTYCOMMANDLINE);
	Flags|=MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected==2?0:
	       (MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected==0?MFLAGS_EDITNOSELECTION:MFLAGS_EDITSELECTION);
	return TRUE;
}

bool KeyMacro::ParseMacroString(const wchar_t *Sequence, bool onlyCheck)
{
	// Перекладываем вывод сообщения об ошибке на плагин, т.к. штатный Message()
	// не умеет сворачивать строки и обрезает сообщение.
	FarMacroValue values[4]={{FMVT_STRING,{0}},{FMVT_INTEGER,{0}},{FMVT_STRING,{0}},{FMVT_STRING,{0}}};
	values[0].String=Sequence;
	values[1].Integer=onlyCheck?1:0;
	values[2].String=MSG(MMacroPErrorTitle);
	values[3].String=MSG(MOk);
	OpenMacroInfo info={sizeof(OpenMacroInfo),ARRAYSIZE(values),values};
	const wchar_t* ErrMsg = (const wchar_t*)CallPlugin(OPEN_MACROPARSE,&info);
	if (ErrMsg && !onlyCheck)
	{
		FrameManager->RefreshFrame(); // Нужно после вывода сообщения плагином. Иначе панели не перерисовываются.
	}
	return !ErrMsg;
}

bool KeyMacro::UpdateLockScreen(bool recreate)
{
	bool oldstate = (m_LockScr!=nullptr);
	if (m_LockScr)
	{
		delete m_LockScr;
		m_LockScr=nullptr;
	}
	if (recreate)
		m_LockScr = new LockScreen;
	return oldstate;
}

void PassString (const wchar_t* str, FarMacroCall* Data)
{
	if (Data->Callback)
	{
		FarMacroValue val;
		val.Type = FMVT_STRING;
		val.String = str;
		Data->Callback(Data->CallbackData, &val);
	}
}

void PassInteger (__int64 Int, FarMacroCall* Data)
{
	if (Data->Callback)
	{
		FarMacroValue val;
		val.Type = FMVT_INTEGER;
		val.Integer = Int;
		Data->Callback(Data->CallbackData, &val);
	}
}

void PassDouble (double dbl, FarMacroCall* Data)
{
	if (Data->Callback)
	{
		FarMacroValue val;
		val.Type = FMVT_DOUBLE;
		val.Double = dbl;
		Data->Callback(Data->CallbackData, &val);
	}
}

int KeyMacro::CallFar(int OpCode, FarMacroCall* Data)
{
	int ret = 0;
	string str;

	switch (OpCode)
	{
		case 0://MCODE_C_FULLSCREENMODE:
			return IsConsoleFullscreen() ? 1:0;

		case 1://MCODE_C_ISUSERADMIN:
			return Opt.IsUserAdmin ? 1:0;

		case 2://MCODE_V_FAR_WIDTH:
			return ScrX+1;

		case 3://MCODE_V_FAR_HEIGHT:
			return ScrY+1;

		case 4://MCODE_V_FAR_TITLE: /*L"Far.Title", */
			Console.GetTitle(str);
			PassString(str, Data);
			return 1;

		case MCODE_V_FAR_UPTIME: /*L"Far.UpTime",*/
			break;

		case MCODE_V_FAR_PID: /*L"Far.PID",   */
			break;

		case MCODE_V_MACRO_AREA: /*L"Macro.Area",*/
			break;
	}
	return ret;
}

#else
#include "headers.hpp"
#pragma hdrstop

#include "macro.hpp"
#include "macroopcode.hpp"
#include "keys.hpp"
#include "keyboard.hpp"
#include "lockscrn.hpp"
#include "viewer.hpp"
#include "fileedit.hpp"
#include "fileview.hpp"
#include "dialog.hpp"
#include "dlgedit.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "cmdline.hpp"
#include "manager.hpp"
#include "scrbuf.hpp"
#include "udlist.hpp"
#include "filelist.hpp"
#include "treelist.hpp"
#include "flink.hpp"
#include "TStack.hpp"
#include "syslog.hpp"
#include "plugapi.hpp"
#include "plugin.hpp"
#include "plugins.hpp"
#include "cddrv.hpp"
#include "interf.hpp"
#include "grabber.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "xlat.hpp"
#include "datetime.hpp"
#include "stddlg.hpp"
#include "pathmix.hpp"
#include "drivemix.hpp"
#include "strmix.hpp"
#include "panelmix.hpp"
#include "constitle.hpp"
#include "dirmix.hpp"
#include "console.hpp"
#include "imports.hpp"
#include "CFileMask.hpp"
#include "vmenu.hpp"
#include "elevation.hpp"
#include "FarGuid.hpp"
#include "configdb.hpp"

// для диалога назначения клавиши
struct DlgParam
{
	UINT64 Flags;
	KeyMacro *Handle;
	DWORD Key;
	int Mode;
	int Recurse;
	bool Changed;
};

TMacroKeywords MKeywords[] =
{
	{0,  L"Other",                    MCODE_C_AREA_OTHER,0},
	{0,  L"Shell",                    MCODE_C_AREA_SHELL,0},
	{0,  L"Viewer",                   MCODE_C_AREA_VIEWER,0},
	{0,  L"Editor",                   MCODE_C_AREA_EDITOR,0},
	{0,  L"Dialog",                   MCODE_C_AREA_DIALOG,0},
	{0,  L"Search",                   MCODE_C_AREA_SEARCH,0},
	{0,  L"Disks",                    MCODE_C_AREA_DISKS,0},
	{0,  L"MainMenu",                 MCODE_C_AREA_MAINMENU,0},
	{0,  L"Menu",                     MCODE_C_AREA_MENU,0},
	{0,  L"Help",                     MCODE_C_AREA_HELP,0},
	{0,  L"Info",                     MCODE_C_AREA_INFOPANEL,0},
	{0,  L"QView",                    MCODE_C_AREA_QVIEWPANEL,0},
	{0,  L"Tree",                     MCODE_C_AREA_TREEPANEL,0},
	{0,  L"FindFolder",               MCODE_C_AREA_FINDFOLDER,0},
	{0,  L"UserMenu",                 MCODE_C_AREA_USERMENU,0},
	{0,  L"Shell.AutoCompletion",     MCODE_C_AREA_SHELL_AUTOCOMPLETION,0},
	{0,  L"Dialog.AutoCompletion",    MCODE_C_AREA_DIALOG_AUTOCOMPLETION,0},

	// ПРОЧЕЕ
	{2,  L"Bof",                MCODE_C_BOF,0},
	{2,  L"Eof",                MCODE_C_EOF,0},
	{2,  L"Empty",              MCODE_C_EMPTY,0},
	{2,  L"Selected",           MCODE_C_SELECTED,0},

	{2,  L"Far.Width",          MCODE_V_FAR_WIDTH,0},
	{2,  L"Far.Height",         MCODE_V_FAR_HEIGHT,0},
	{2,  L"Far.Title",          MCODE_V_FAR_TITLE,0},
	{2,  L"Far.UpTime",         MCODE_V_FAR_UPTIME,0},
	{2,  L"Far.PID",            MCODE_V_FAR_PID,0},
	{2,  L"Macro.Area",         MCODE_V_MACRO_AREA,0},

	{2,  L"ItemCount",          MCODE_V_ITEMCOUNT,0},  // ItemCount - число элементов в текущем объекте
	{2,  L"CurPos",             MCODE_V_CURPOS,0},    // CurPos - текущий индекс в текущем объекте
	{2,  L"Title",              MCODE_V_TITLE,0},
	{2,  L"Height",             MCODE_V_HEIGHT,0},
	{2,  L"Width",              MCODE_V_WIDTH,0},

	{2,  L"APanel.Empty",       MCODE_C_APANEL_ISEMPTY,0},
	{2,  L"PPanel.Empty",       MCODE_C_PPANEL_ISEMPTY,0},
	{2,  L"APanel.Bof",         MCODE_C_APANEL_BOF,0},
	{2,  L"PPanel.Bof",         MCODE_C_PPANEL_BOF,0},
	{2,  L"APanel.Eof",         MCODE_C_APANEL_EOF,0},
	{2,  L"PPanel.Eof",         MCODE_C_PPANEL_EOF,0},
	{2,  L"APanel.Root",        MCODE_C_APANEL_ROOT,0},
	{2,  L"PPanel.Root",        MCODE_C_PPANEL_ROOT,0},
	{2,  L"APanel.Visible",     MCODE_C_APANEL_VISIBLE,0},
	{2,  L"PPanel.Visible",     MCODE_C_PPANEL_VISIBLE,0},
	{2,  L"APanel.Plugin",      MCODE_C_APANEL_PLUGIN,0},
	{2,  L"PPanel.Plugin",      MCODE_C_PPANEL_PLUGIN,0},
	{2,  L"APanel.FilePanel",   MCODE_C_APANEL_FILEPANEL,0},
	{2,  L"PPanel.FilePanel",   MCODE_C_PPANEL_FILEPANEL,0},
	{2,  L"APanel.Folder",      MCODE_C_APANEL_FOLDER,0},
	{2,  L"PPanel.Folder",      MCODE_C_PPANEL_FOLDER,0},
	{2,  L"APanel.Selected",    MCODE_C_APANEL_SELECTED,0},
	{2,  L"PPanel.Selected",    MCODE_C_PPANEL_SELECTED,0},
	{2,  L"APanel.Left",        MCODE_C_APANEL_LEFT,0},
	{2,  L"PPanel.Left",        MCODE_C_PPANEL_LEFT,0},
	{2,  L"APanel.LFN",         MCODE_C_APANEL_LFN,0},
	{2,  L"PPanel.LFN",         MCODE_C_PPANEL_LFN,0},
	{2,  L"APanel.Filter",      MCODE_C_APANEL_FILTER,0},
	{2,  L"PPanel.Filter",      MCODE_C_PPANEL_FILTER,0},

	{2,  L"APanel.Type",        MCODE_V_APANEL_TYPE,0},
	{2,  L"PPanel.Type",        MCODE_V_PPANEL_TYPE,0},
	{2,  L"APanel.ItemCount",   MCODE_V_APANEL_ITEMCOUNT,0},
	{2,  L"PPanel.ItemCount",   MCODE_V_PPANEL_ITEMCOUNT,0},
	{2,  L"APanel.CurPos",      MCODE_V_APANEL_CURPOS,0},
	{2,  L"PPanel.CurPos",      MCODE_V_PPANEL_CURPOS,0},
	{2,  L"APanel.Current",     MCODE_V_APANEL_CURRENT,0},
	{2,  L"PPanel.Current",     MCODE_V_PPANEL_CURRENT,0},
	{2,  L"APanel.SelCount",    MCODE_V_APANEL_SELCOUNT,0},
	{2,  L"PPanel.SelCount",    MCODE_V_PPANEL_SELCOUNT,0},
	{2,  L"APanel.Path",        MCODE_V_APANEL_PATH,0},
	{2,  L"PPanel.Path",        MCODE_V_PPANEL_PATH,0},
	{2,  L"APanel.Path0",       MCODE_V_APANEL_PATH0,0},
	{2,  L"PPanel.Path0",       MCODE_V_PPANEL_PATH0,0},
	{2,  L"APanel.UNCPath",     MCODE_V_APANEL_UNCPATH,0},
	{2,  L"PPanel.UNCPath",     MCODE_V_PPANEL_UNCPATH,0},
	{2,  L"APanel.Height",      MCODE_V_APANEL_HEIGHT,0},
	{2,  L"PPanel.Height",      MCODE_V_PPANEL_HEIGHT,0},
	{2,  L"APanel.Width",       MCODE_V_APANEL_WIDTH,0},
	{2,  L"PPanel.Width",       MCODE_V_PPANEL_WIDTH,0},
	{2,  L"APanel.OPIFlags",    MCODE_V_APANEL_OPIFLAGS,0},
	{2,  L"PPanel.OPIFlags",    MCODE_V_PPANEL_OPIFLAGS,0},
	{2,  L"APanel.DriveType",   MCODE_V_APANEL_DRIVETYPE,0}, // APanel.DriveType - активная панель: тип привода
	{2,  L"PPanel.DriveType",   MCODE_V_PPANEL_DRIVETYPE,0}, // PPanel.DriveType - пассивная панель: тип привода
	{2,  L"APanel.ColumnCount", MCODE_V_APANEL_COLUMNCOUNT,0}, // APanel.ColumnCount - активная панель:  количество колонок
	{2,  L"PPanel.ColumnCount", MCODE_V_PPANEL_COLUMNCOUNT,0}, // PPanel.ColumnCount - пассивная панель: количество колонок
	{2,  L"APanel.HostFile",    MCODE_V_APANEL_HOSTFILE,0},
	{2,  L"PPanel.HostFile",    MCODE_V_PPANEL_HOSTFILE,0},
	{2,  L"APanel.Prefix",      MCODE_V_APANEL_PREFIX,0},
	{2,  L"PPanel.Prefix",      MCODE_V_PPANEL_PREFIX,0},
	{2,  L"APanel.Format",      MCODE_V_APANEL_FORMAT,0},
	{2,  L"PPanel.Format",      MCODE_V_PPANEL_FORMAT,0},

	{2,  L"CmdLine.Bof",        MCODE_C_CMDLINE_BOF,0}, // курсор в начале cmd-строки редактирования?
	{2,  L"CmdLine.Eof",        MCODE_C_CMDLINE_EOF,0}, // курсор в конеце cmd-строки редактирования?
	{2,  L"CmdLine.Empty",      MCODE_C_CMDLINE_EMPTY,0},
	{2,  L"CmdLine.Selected",   MCODE_C_CMDLINE_SELECTED,0},
	{2,  L"CmdLine.ItemCount",  MCODE_V_CMDLINE_ITEMCOUNT,0},
	{2,  L"CmdLine.CurPos",     MCODE_V_CMDLINE_CURPOS,0},
	{2,  L"CmdLine.Value",      MCODE_V_CMDLINE_VALUE,0},

	{2,  L"Editor.FileName",    MCODE_V_EDITORFILENAME,0},
	{2,  L"Editor.CurLine",     MCODE_V_EDITORCURLINE,0},  // текущая линия в редакторе (в дополнении к Count)
	{2,  L"Editor.Lines",       MCODE_V_EDITORLINES,0},
	{2,  L"Editor.CurPos",      MCODE_V_EDITORCURPOS,0},
	{2,  L"Editor.RealPos",     MCODE_V_EDITORREALPOS,0},
	{2,  L"Editor.State",       MCODE_V_EDITORSTATE,0},
	{2,  L"Editor.Value",       MCODE_V_EDITORVALUE,0},
	{2,  L"Editor.SelValue",    MCODE_V_EDITORSELVALUE,0},

	{2,  L"Dlg.ItemType",       MCODE_V_DLGITEMTYPE,0},
	{2,  L"Dlg.ItemCount",      MCODE_V_DLGITEMCOUNT,0},
	{2,  L"Dlg.CurPos",         MCODE_V_DLGCURPOS,0},
	{2,  L"Dlg.PrevPos",        MCODE_V_DLGPREVPOS,0},
	{2,  L"Dlg.Info.Id",        MCODE_V_DLGINFOID,0},
	{2,  L"Dlg.Info.Owner",     MCODE_V_DLGINFOOWNER,0},

	{2,  L"Help.FileName",      MCODE_V_HELPFILENAME, 0},
	{2,  L"Help.Topic",         MCODE_V_HELPTOPIC, 0},
	{2,  L"Help.SelTopic",      MCODE_V_HELPSELTOPIC, 0},

	{2,  L"Drv.ShowPos",        MCODE_V_DRVSHOWPOS,0},
	{2,  L"Drv.ShowMode",       MCODE_V_DRVSHOWMODE,0},

	{2,  L"Viewer.FileName",    MCODE_V_VIEWERFILENAME,0},
	{2,  L"Viewer.State",       MCODE_V_VIEWERSTATE,0},

	{2,  L"Menu.Value",         MCODE_V_MENU_VALUE,0},
	{2,  L"Menu.Info.Id",       MCODE_V_MENUINFOID,0},

	{2,  L"Fullscreen",         MCODE_C_FULLSCREENMODE,0},
	{2,  L"IsUserAdmin",        MCODE_C_ISUSERADMIN,0},
};

TMacroKeywords MKeywordsArea[] =
{
	{0,  L"Funcs",             (DWORD)MACRO_FUNCS,0},
	{0,  L"Consts",            (DWORD)MACRO_CONSTS,0},
	{0,  L"Vars",              (DWORD)MACRO_VARS,0},
	{0,  L"Other",                    MACRO_OTHER,0},
	{0,  L"Shell",                    MACRO_SHELL,0},
	{0,  L"Viewer",                   MACRO_VIEWER,0},
	{0,  L"Editor",                   MACRO_EDITOR,0},
	{0,  L"Dialog",                   MACRO_DIALOG,0},
	{0,  L"Search",                   MACRO_SEARCH,0},
	{0,  L"Disks",                    MACRO_DISKS,0},
	{0,  L"MainMenu",                 MACRO_MAINMENU,0},
	{0,  L"Menu",                     MACRO_MENU,0},
	{0,  L"Help",                     MACRO_HELP,0},
	{0,  L"Info",                     MACRO_INFOPANEL,0},
	{0,  L"QView",                    MACRO_QVIEWPANEL,0},
	{0,  L"Tree",                     MACRO_TREEPANEL,0},
	{0,  L"FindFolder",               MACRO_FINDFOLDER,0},
	{0,  L"UserMenu",                 MACRO_USERMENU,0},
	{0,  L"Shell.AutoCompletion",     MACRO_SHELLAUTOCOMPLETION,0},
	{0,  L"Dialog.AutoCompletion",    MACRO_DIALOGAUTOCOMPLETION,0},
	{0,  L"Common",                   MACRO_COMMON,0},
};

TMacroKeywords MKeywordsVarType[] =
{
	{3,  L"unknown",   vtUnknown, 0},
	{3,  L"integer",   vtInteger, 0},
	{3,  L"text",      vtString,  0},
	{3,  L"real",      vtDouble,  0},
};

TMacroKeywords MKeywordsFlags[] =
{
	// ФЛАГИ
	{1,  L"DisableOutput",      MFLAGS_DISABLEOUTPUT,0},
	{1,  L"RunAfterFARStart",   MFLAGS_RUNAFTERFARSTART,0},
	{1,  L"EmptyCommandLine",   MFLAGS_EMPTYCOMMANDLINE,0},
	{1,  L"NotEmptyCommandLine",MFLAGS_NOTEMPTYCOMMANDLINE,0},
	{1,  L"EVSelection",        MFLAGS_EDITSELECTION,0},
	{1,  L"NoEVSelection",      MFLAGS_EDITNOSELECTION,0},

	{1,  L"NoFilePanels",       MFLAGS_NOFILEPANELS,0},
	{1,  L"NoPluginPanels",     MFLAGS_NOPLUGINPANELS,0},
	{1,  L"NoFolders",          MFLAGS_NOFOLDERS,0},
	{1,  L"NoFiles",            MFLAGS_NOFILES,0},
	{1,  L"Selection",          MFLAGS_SELECTION,0},
	{1,  L"NoSelection",        MFLAGS_NOSELECTION,0},

	{1,  L"NoFilePPanels",      MFLAGS_PNOFILEPANELS,0},
	{1,  L"NoPluginPPanels",    MFLAGS_PNOPLUGINPANELS,0},
	{1,  L"NoPFolders",         MFLAGS_PNOFOLDERS,0},
	{1,  L"NoPFiles",           MFLAGS_PNOFILES,0},
	{1,  L"PSelection",         MFLAGS_PSELECTION,0},
	{1,  L"NoPSelection",       MFLAGS_PNOSELECTION,0},

	{1,  L"NoSendKeysToPlugins",MFLAGS_NOSENDKEYSTOPLUGINS,0},
};

const wchar_t* GetAreaName(DWORD AreaValue) {return GetNameOfValue(AreaValue, MKeywordsArea);}
DWORD GetAreaValue(const wchar_t* AreaName) {return GetValueOfVame(AreaName, MKeywordsArea);}

const wchar_t* GetVarTypeName(DWORD ValueType) {return GetNameOfValue(ValueType, MKeywordsVarType);}
DWORD GetVarTypeValue(const wchar_t* ValueName) {return GetValueOfVame(ValueName, MKeywordsVarType);}

const string FlagsToString(FARKEYMACROFLAGS Flags)
{
	return FlagsToString(Flags, MKeywordsFlags);
}

FARKEYMACROFLAGS StringToFlags(const string& strFlags)
{
	return StringToFlags(strFlags, MKeywordsFlags);
}

// транслирующая таблица - имя <-> код макроклавиши
static struct TKeyCodeName
{
	int Key;
	int Len;
	const wchar_t *Name;
} KeyMacroCodes[]=
{
	{ MCODE_OP_AKEY,                 5, L"$AKey"      }, // клавиша, которой вызвали макрос
	{ MCODE_OP_BREAK,                6, L"$Break"     },
	{ MCODE_OP_CONTINUE,             9, L"$Continue"  },
	{ MCODE_OP_ELSE,                 5, L"$Else"      },
	{ MCODE_OP_END,                  4, L"$End"       },
	{ MCODE_OP_EXIT,                 5, L"$Exit"      },
	{ MCODE_OP_IF,                   3, L"$If"        },
	{ MCODE_OP_REP,                  4, L"$Rep"       },
	{ MCODE_OP_SELWORD,              8, L"$SelWord"   },
	//{ MCODE_OP_PLAINTEXT,            5, L"$Text"      }, // $Text "Plain Text"
	{ MCODE_OP_WHILE,                6, L"$While"     },
	{ MCODE_OP_XLAT,                 5, L"$XLat"      },
};

static bool absFunc(const TMacroFunction*);
static bool ascFunc(const TMacroFunction*);
static bool atoiFunc(const TMacroFunction*);
static bool beepFunc(const TMacroFunction*);
static bool chrFunc(const TMacroFunction*);
static bool clipFunc(const TMacroFunction*);
static bool dateFunc(const TMacroFunction*);
static bool dlggetvalueFunc(const TMacroFunction*);
static bool dlgsetfocusFunc(const TMacroFunction*);
static bool editordellineFunc(const TMacroFunction*);
static bool editorgetstrFunc(const TMacroFunction*);
static bool editorinsstrFunc(const TMacroFunction*);
static bool editorposFunc(const TMacroFunction*);
static bool editorselFunc(const TMacroFunction*);
static bool editorsetFunc(const TMacroFunction*);
static bool editorsetstrFunc(const TMacroFunction*);
static bool editorsettitleFunc(const TMacroFunction*);
static bool editorundoFunc(const TMacroFunction*);
static bool environFunc(const TMacroFunction*);
static bool farcfggetFunc(const TMacroFunction*);
static bool fattrFunc(const TMacroFunction*);
static bool fexistFunc(const TMacroFunction*);
static bool floatFunc(const TMacroFunction*);
static bool flockFunc(const TMacroFunction*);
static bool fmatchFunc(const TMacroFunction*);
static bool fsplitFunc(const TMacroFunction*);
static bool iifFunc(const TMacroFunction*);
static bool indexFunc(const TMacroFunction*);
static bool intFunc(const TMacroFunction*);
static bool itowFunc(const TMacroFunction*);
static bool kbdLayoutFunc(const TMacroFunction*);
static bool keybarshowFunc(const TMacroFunction*);
static bool keyFunc(const TMacroFunction*);
static bool lcaseFunc(const TMacroFunction*);
static bool lenFunc(const TMacroFunction*);
static bool macroenumkwdFunc(const TMacroFunction*);
static bool macroenumfuncFunc(const TMacroFunction*);
static bool macroenumvarFunc(const TMacroFunction*);
static bool macroenumConstFunc(const TMacroFunction*);
static bool maxFunc(const TMacroFunction*);
static bool menushowFunc(const TMacroFunction*);
static bool minFunc(const TMacroFunction*);
static bool mloadFunc(const TMacroFunction*);
static bool modFunc(const TMacroFunction*);
static bool msaveFunc(const TMacroFunction*);
static bool msgBoxFunc(const TMacroFunction*);
static bool panelfattrFunc(const TMacroFunction*);
static bool panelfexistFunc(const TMacroFunction*);
static bool panelitemFunc(const TMacroFunction*);
static bool panelselectFunc(const TMacroFunction*);
static bool panelsetpathFunc(const TMacroFunction*);
static bool panelsetposFunc(const TMacroFunction*);
static bool panelsetposidxFunc(const TMacroFunction*);
static bool pluginsFunc(const TMacroFunction*);
static bool promptFunc(const TMacroFunction*);
static bool replaceFunc(const TMacroFunction*);
static bool rindexFunc(const TMacroFunction*);
static bool size2strFunc(const TMacroFunction*);
static bool sleepFunc(const TMacroFunction*);
static bool stringFunc(const TMacroFunction*);
static bool strwrapFunc(const TMacroFunction*);
static bool strpadFunc(const TMacroFunction*);
static bool substrFunc(const TMacroFunction*);
static bool testfolderFunc(const TMacroFunction*);
static bool trimFunc(const TMacroFunction*);
static bool ucaseFunc(const TMacroFunction*);
static bool usersFunc(const TMacroFunction*);
static bool waitkeyFunc(const TMacroFunction*);
static bool windowscrollFunc(const TMacroFunction*);
static bool xlatFunc(const TMacroFunction*);
static bool pluginloadFunc(const TMacroFunction*);
static bool pluginunloadFunc(const TMacroFunction*);
static bool pluginexistFunc(const TMacroFunction*);

static TMacroFunction intMacroFunction[]=
{
	//Name                fnGUID   Syntax                                                        Func                Buffer BufferSize IntFlags                          Code
	{L"Abs",              nullptr, L"N=Abs(N)",                                                  absFunc,            nullptr, 0, 0,                                      MCODE_F_ABS,             },
	{L"Akey",             nullptr, L"V=Akey(Mode[,Type])",                                       usersFunc,          nullptr, 0, 0,                                      MCODE_F_AKEY,            },
	{L"Asc",              nullptr, L"N=Asc(N)",                                                  ascFunc,            nullptr, 0, 0,                                      MCODE_F_ASC,             },
	{L"Atoi",             nullptr, L"N=Atoi(S[,Radix])",                                         atoiFunc,           nullptr, 0, 0,                                      MCODE_F_ATOI,            },
	{L"Beep",             nullptr, L"N=Beep([N])",                                               beepFunc,           nullptr, 0, 0,                                      MCODE_F_BEEP,            },
	{L"BM.Add",           nullptr, L"N=BM.Add()",                                                usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_ADD,          },
	{L"BM.Clear",         nullptr, L"N=BM.Clear()",                                              usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_CLEAR,        },
	{L"BM.Del",           nullptr, L"N=BM.Del([Idx])",                                           usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_DEL,          },
	{L"BM.Get",           nullptr, L"N=BM.Get(Idx,M)",                                           usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_GET,          },
	{L"BM.Goto",          nullptr, L"N=BM.Goto([N])",                                            usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_GOTO,         },
	{L"BM.Next",          nullptr, L"N=BM.Next()",                                               usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_NEXT,         },
	{L"BM.Pop",           nullptr, L"N=BM.Pop()",                                                usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_POP,          },
	{L"BM.Prev",          nullptr, L"N=BM.Prev()",                                               usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_PREV,         },
	{L"BM.Back",          nullptr, L"N=BM.Back()",                                               usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_BACK,         },
	{L"BM.Push",          nullptr, L"N=BM.Push()",                                               usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_PUSH,         },
	{L"BM.Stat",          nullptr, L"N=BM.Stat([N])",                                            usersFunc,          nullptr, 0, 0,                                      MCODE_F_BM_STAT,         },
	{L"CallPlugin",       nullptr, L"V=CallPlugin(SysID[,param])",                               usersFunc,          nullptr, 0, 0,                                      MCODE_F_CALLPLUGIN,      },
	{L"CheckHotkey",      nullptr, L"N=CheckHotkey(S[,N])",                                      usersFunc,          nullptr, 0, 0,                                      MCODE_F_MENU_CHECKHOTKEY,},
	{L"Chr",              nullptr, L"S=Chr(N)",                                                  chrFunc,            nullptr, 0, 0,                                      MCODE_F_CHR,             },
	{L"Clip",             nullptr, L"V=Clip(N[,V])",                                             clipFunc,           nullptr, 0, 0,                                      MCODE_F_CLIP,            },
	{L"Date",             nullptr, L"S=Date([S])",                                               dateFunc,           nullptr, 0, 0,                                      MCODE_F_DATE,            },
	{L"Dlg.GetValue",     nullptr, L"V=Dlg.GetValue([Pos[,InfoID]])",                            dlggetvalueFunc,    nullptr, 0, 0,                                      MCODE_F_DLG_GETVALUE,    },
	{L"Dlg.SetFocus",     nullptr, L"N=Dlg.SetFocus([ID])",                                      dlgsetfocusFunc,    nullptr, 0, 0,                                      MCODE_F_DLG_SETFOCUS,    },
	{L"Editor.DelLine",   nullptr, L"N=Editor.DelLine([Line])",                                  editordellineFunc,  nullptr, 0, 0,                                      MCODE_F_EDITOR_DELLINE,  },
	{L"Editor.GetStr",    nullptr, L"S=Editor.GetStr([Line])",                                   editorgetstrFunc,   nullptr, 0, 0,                                      MCODE_F_EDITOR_GETSTR,   },
	{L"Editor.InsStr",    nullptr, L"N=Editor.InsStr([S[,Line]])",                               editorinsstrFunc,   nullptr, 0, 0,                                      MCODE_F_EDITOR_INSSTR,   },
	{L"Editor.Pos",       nullptr, L"N=Editor.Pos(Op,What[,Where])",                             editorposFunc,      nullptr, 0, 0,                                      MCODE_F_EDITOR_POS,      },
	{L"Editor.Sel",       nullptr, L"V=Editor.Sel(Action[,Opt])",                                editorselFunc,      nullptr, 0, 0,                                      MCODE_F_EDITOR_SEL,      },
	{L"Editor.Set",       nullptr, L"N=Editor.Set(N,Var)",                                       editorsetFunc,      nullptr, 0, 0,                                      MCODE_F_EDITOR_SET,      },
	{L"Editor.SetStr",    nullptr, L"N=Editor.SetStr([S[,Line]])",                               editorsetstrFunc,   nullptr, 0, 0,                                      MCODE_F_EDITOR_SETSTR,   },
	{L"Editor.Settitle",  nullptr, L"N=Editor.SetTitle([Title])",                                editorsettitleFunc, nullptr, 0, 0,                                      MCODE_F_EDITOR_SETTITLE, },
	{L"Editor.Undo",      nullptr, L"V=Editor.Undo(N)",                                          editorundoFunc,     nullptr, 0, 0,                                      MCODE_F_EDITOR_UNDO,     },
	{L"Env",              nullptr, L"S=Env(S[,Mode[,Value]])",                                   environFunc,        nullptr, 0, 0,                                      MCODE_F_ENVIRON,         },
	{L"Eval",             nullptr, L"N=Eval(S[,N])",                                             usersFunc,          nullptr, 0, 0,                                      MCODE_F_EVAL,            },
	{L"Far.Cfg.Get",      nullptr, L"V=Far.Cfg.Get(Key,Name)",                                   farcfggetFunc,      nullptr, 0, 0,                                      MCODE_F_FAR_CFG_GET,     },
	{L"FAttr",            nullptr, L"N=FAttr(S)",                                                fattrFunc,          nullptr, 0, 0,                                      MCODE_F_FATTR,           },
	{L"FExist",           nullptr, L"N=FExist(S)",                                               fexistFunc,         nullptr, 0, 0,                                      MCODE_F_FEXIST,          },
	{L"Float",            nullptr, L"N=Float(V)",                                                floatFunc,          nullptr, 0, 0,                                      MCODE_F_FLOAT,           },
	{L"FLock",            nullptr, L"N=FLock(N,N)",                                              flockFunc,          nullptr, 0, 0,                                      MCODE_F_FLOCK,           },
	{L"FMatch",           nullptr, L"N=FMatch(S,Mask)",                                          fmatchFunc,         nullptr, 0, 0,                                      MCODE_F_FMATCH,          },
	{L"FSplit",           nullptr, L"S=FSplit(S,N)",                                             fsplitFunc,         nullptr, 0, 0,                                      MCODE_F_FSPLIT,          },
	{L"GetHotkey",        nullptr, L"S=GetHotkey([N])",                                          usersFunc,          nullptr, 0, 0,                                      MCODE_F_MENU_GETHOTKEY,  },
	{L"History.Disable",  nullptr, L"N=History.Disable([State])",                                usersFunc,          nullptr, 0, 0,                                      MCODE_F_HISTIORY_DISABLE,},
	{L"Iif",              nullptr, L"V=Iif(Condition,V1,V2)",                                    iifFunc,            nullptr, 0, 0,                                      MCODE_F_IIF,             },
	{L"Index",            nullptr, L"S=Index(S1,S2[,Mode])",                                     indexFunc,          nullptr, 0, 0,                                      MCODE_F_INDEX,           },
	{L"Int",              nullptr, L"N=Int(V)",                                                  intFunc,            nullptr, 0, 0,                                      MCODE_F_INT,             },
	{L"Itoa",             nullptr, L"S=Itoa(N[,radix])",                                         itowFunc,           nullptr, 0, 0,                                      MCODE_F_ITOA,            },
	{L"KbdLayout",        nullptr, L"N=kbdLayout([N])",                                          kbdLayoutFunc,      nullptr, 0, 0,                                      MCODE_F_KBDLAYOUT,       },
	{L"Key",              nullptr, L"S=Key(V)",                                                  keyFunc,            nullptr, 0, 0,                                      MCODE_F_KEY,             },
	{L"KeyBar.Show",      nullptr, L"N=KeyBar.Show([N])",                                        keybarshowFunc,     nullptr, 0, 0,                                      MCODE_F_KEYBAR_SHOW,     },
	{L"LCase",            nullptr, L"S=LCase(S1)",                                               lcaseFunc,          nullptr, 0, 0,                                      MCODE_F_LCASE,           },
	{L"Len",              nullptr, L"N=Len(S)",                                                  lenFunc,            nullptr, 0, 0,                                      MCODE_F_LEN,             },
	{L"Macro.Const",      nullptr, L"S=Macro.Const(Index[,Type])",                               macroenumConstFunc, nullptr, 0, 0,                                      MCODE_F_MACRO_CONST,     },
	{L"Macro.Func",       nullptr, L"S=Macro.Func(Index[,Type])",                                macroenumfuncFunc,  nullptr, 0, 0,                                      MCODE_F_MACRO_FUNC,      },
	{L"Macro.Keyword",    nullptr, L"S=Macro.Keyword(Index[,Type])",                             macroenumkwdFunc,   nullptr, 0, 0,                                      MCODE_F_MACRO_KEYWORD,   },
	{L"Macro.Var",        nullptr, L"S=Macro.Var(Index[,Type])",                                 macroenumvarFunc,   nullptr, 0, 0,                                      MCODE_F_MACRO_VAR,       },
	{L"Max",              nullptr, L"N=Max(N1,N2)",                                              maxFunc,            nullptr, 0, 0,                                      MCODE_F_MAX,             },
	{L"Menu.Filter",      nullptr, L"N=Menu.Filter([Action[,Mode]])",                            usersFunc,          nullptr, 0, 0,                                      MCODE_F_MENU_FILTER,     },
	{L"Menu.FilterStr",   nullptr, L"N=Menu.FilterStr([Action[,S]])",                            usersFunc,          nullptr, 0, 0,                                      MCODE_F_MENU_FILTERSTR,  },
	{L"Menu.GetValue",    nullptr, L"S=Menu.GetValue([N])",                                      usersFunc,          nullptr, 0, 0,                                      MCODE_F_MENU_GETVALUE,   },
	{L"Menu.ItemStatus",  nullptr, L"N=Menu.ItemStatus([N])",                                    usersFunc,          nullptr, 0, 0,                                      MCODE_F_MENU_ITEMSTATUS, },
	{L"Menu.Select",      nullptr, L"N=Menu.Select(S[,N[,Dir]])",                                usersFunc,          nullptr, 0, 0,                                      MCODE_F_MENU_SELECT,     },
	{L"Menu.Show",        nullptr, L"S=Menu.Show(Items[,Title[,Flags[,FindOrFilter[,X[,Y]]]]])", menushowFunc,       nullptr, 0, IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT, MCODE_F_MENU_SHOW,       },
	{L"Min",              nullptr, L"N=Min(N1,N2)",                                              minFunc,            nullptr, 0, 0,                                      MCODE_F_MIN,             },
	{L"MLoad",            nullptr, L"N=MLoad(S)",                                                mloadFunc,          nullptr, 0, 0,                                      MCODE_F_MLOAD,           },
	{L"MMode",            nullptr, L"N=MMode(Action[,Value])",                                   usersFunc,          nullptr, 0, 0,                                      MCODE_F_MMODE,           },
	{L"Mod",              nullptr, L"N=Mod(a,b)",                                                modFunc,            nullptr, 0, 0,                                      MCODE_F_MOD,             },
	{L"MSave",            nullptr, L"N=MSave(S)",                                                msaveFunc,          nullptr, 0, 0,                                      MCODE_F_MSAVE,           },
	{L"MsgBox",           nullptr, L"N=MsgBox([Title[,Text[,flags]]])",                          msgBoxFunc,         nullptr, 0, IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT, MCODE_F_MSGBOX,          },
	{L"Panel.FAttr",      nullptr, L"N=Panel.FAttr(panelType,fileMask)",                         panelfattrFunc,     nullptr, 0, 0,                                      MCODE_F_PANEL_FATTR,     },
	{L"Panel.FExist",     nullptr, L"N=Panel.FExist(panelType,fileMask)",                        panelfexistFunc,    nullptr, 0, 0,                                      MCODE_F_PANEL_FEXIST,    },
	{L"Panel.Item",       nullptr, L"V=Panel.Item(Panel,Index,TypeInfo)",                        panelitemFunc,      nullptr, 0, 0,                                      MCODE_F_PANELITEM,       },
	{L"Panel.Select",     nullptr, L"V=Panel.Select(panelType,Action[,Mode[,Items]])",           panelselectFunc,    nullptr, 0, 0,                                      MCODE_F_PANEL_SELECT,    },
	{L"Panel.SetPath",    nullptr, L"N=panel.SetPath(panelType,pathName[,fileName])",            panelsetpathFunc,   nullptr, 0, IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT, MCODE_F_PANEL_SETPATH,   },
	{L"Panel.SetPos",     nullptr, L"N=panel.SetPos(panelType,fileName)",                        panelsetposFunc,    nullptr, 0, IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT, MCODE_F_PANEL_SETPOS,    },
	{L"Panel.SetPosIdx",  nullptr, L"N=Panel.SetPosIdx(panelType,Idx[,InSelection])",            panelsetposidxFunc, nullptr, 0, IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT, MCODE_F_PANEL_SETPOSIDX, },
	{L"Plugin.Call",      nullptr, L"N=Plugin.Call(Guid[,Item])",                                usersFunc,          nullptr, 0, 0,                                      MCODE_F_PLUGIN_CALL,     },
	{L"Plugin.Command",   nullptr, L"N=Plugin.Command(Guid[,Command])",                          usersFunc,          nullptr, 0, 0,                                      MCODE_F_PLUGIN_COMMAND,  },
	{L"Plugin.Config",    nullptr, L"N=Plugin.Config(Guid[,MenuGuid])",                          usersFunc,          nullptr, 0, 0,                                      MCODE_F_PLUGIN_CONFIG,   },
	{L"Plugin.Exist",     nullptr, L"N=Plugin.Exist(Guid)",                                      pluginexistFunc,    nullptr, 0, 0,                                      MCODE_F_PLUGIN_EXIST,    },
	{L"Plugin.Load",      nullptr, L"N=Plugin.Load(DllPath[,ForceLoad])",                        pluginloadFunc,     nullptr, 0, 0,                                      MCODE_F_PLUGIN_LOAD,     },
	{L"Plugin.Menu",      nullptr, L"N=Plugin.Menu(Guid[,MenuGuid])",                            usersFunc,          nullptr, 0, 0,                                      MCODE_F_PLUGIN_MENU,     },
	{L"Plugin.UnLoad",    nullptr, L"N=Plugin.UnLoad(DllPath)",                                  pluginunloadFunc,   nullptr, 0, 0,                                      MCODE_F_PLUGIN_UNLOAD,   },
	{L"Print",            nullptr, L"N=Print(Str)",                                              usersFunc,          nullptr, 0, 0,                                      MCODE_F_PRINT,           },
	{L"Prompt",           nullptr, L"S=Prompt([Title[,Prompt[,flags[,Src[,History]]]]])",        promptFunc,         nullptr, 0, IMFF_UNLOCKSCREEN|IMFF_DISABLEINTINPUT, MCODE_F_PROMPT,          },
	{L"Replace",          nullptr, L"S=Replace(Str,Find,Replace[,Cnt[,Mode]])",                  replaceFunc,        nullptr, 0, 0,                                      MCODE_F_REPLACE,         },
	{L"Rindex",           nullptr, L"S=RIndex(S1,S2[,Mode])",                                    rindexFunc,         nullptr, 0, 0,                                      MCODE_F_RINDEX,          },
 	{L"Size2Str",         nullptr, L"S=Size2Str(N,Flags[,Width])",                               size2strFunc,       nullptr, 0, 0,                                      MCODE_F_SIZE2STR,        },
	{L"Sleep",            nullptr, L"N=Sleep(N)",                                                sleepFunc,          nullptr, 0, 0,                                      MCODE_F_SLEEP,           },
	{L"String",           nullptr, L"S=String(V)",                                               stringFunc,         nullptr, 0, 0,                                      MCODE_F_STRING,          },
	{L"StrPad",           nullptr, L"S=StrPad(Src,Cnt[,Fill[,Op]])",                             strpadFunc,         nullptr, 0, 0,                                      MCODE_F_STRPAD,          },
	{L"StrWrap",          nullptr, L"S=StrWrap(Text,Width[,Break[,Flags]])",                     strwrapFunc,        nullptr, 0, 0,                                      MCODE_F_STRWRAP,         },
	{L"SubStr",           nullptr, L"S=SubStr(S,start[,length])",                                substrFunc,         nullptr, 0, 0,                                      MCODE_F_SUBSTR,          },
	{L"TestFolder",       nullptr, L"N=TestFolder(S)",                                           testfolderFunc,     nullptr, 0, 0,                                      MCODE_F_TESTFOLDER,      },
	{L"Trim",             nullptr, L"S=Trim(S[,N])",                                             trimFunc,           nullptr, 0, 0,                                      MCODE_F_TRIM,            },
	{L"UCase",            nullptr, L"S=UCase(S1)",                                               ucaseFunc,          nullptr, 0, 0,                                      MCODE_F_UCASE,           },
	{L"WaitKey",          nullptr, L"V=Waitkey([N,[T]])",                                        waitkeyFunc,        nullptr, 0, 0,                                      MCODE_F_WAITKEY,         },
	{L"Window.Scroll",    nullptr, L"N=Window.Scroll(Lines[,Axis])",                             windowscrollFunc,   nullptr, 0, 0,                                      MCODE_F_WINDOW_SCROLL,   },
	{L"Xlat",             nullptr, L"S=Xlat(S[,Flags])",                                         xlatFunc,           nullptr, 0, 0,                                      MCODE_F_XLAT,            },
};

static_assert(MCODE_F_LAST - KEY_MACRO_F_BASE == ARRAYSIZE(intMacroFunction), "intMacroFunction size != MCODE_F_* count");

int MKeywordsSize = ARRAYSIZE(MKeywords);
int MKeywordsFlagsSize = ARRAYSIZE(MKeywordsFlags);

DWORD KeyMacro::LastOpCodeUF=KEY_MACRO_U_BASE;
size_t KeyMacro::CMacroFunction=0;
size_t KeyMacro::AllocatedFuncCount=0;
TMacroFunction *KeyMacro::AMacroFunction=nullptr;

TVarTable glbVarTable;
TVarTable glbConstTable;
TVMStack VMStack;

static TVar __varTextDate;

bool __CheckCondForSkip(const TVar& Cond,DWORD Op)
{
	if (Cond.isString() && *Cond.s())
		return false;

	__int64 res=Cond.getInteger();
	switch(Op)
	{
		case MCODE_OP_JZ:
			return !res?true:false;
		case MCODE_OP_JNZ:
			return res?true:false;
		case MCODE_OP_JLT:
			return res < 0?true:false;
		case MCODE_OP_JLE:
			return res <= 0?true:false;
		case MCODE_OP_JGT:
			return res > 0?true:false;
		case MCODE_OP_JGE:
			return res >= 0?true:false;
	}
	return false;
}

// функция преобразования кода макроклавиши в текст
BOOL KeyMacroToText(int Key,string &strKeyText0)
{
	string strKeyText;

	for (int I=0; I<int(ARRAYSIZE(KeyMacroCodes)); I++)
	{
		if (Key==KeyMacroCodes[I].Key)
		{
			strKeyText = KeyMacroCodes[I].Name;
			break;
		}
	}

	if (strKeyText.IsEmpty())
	{
		strKeyText0.Clear();
		return FALSE;
	}

	strKeyText0 = strKeyText;
	return TRUE;
}

// функция преобразования названия в код макроклавиши
// вернет -1, если нет эквивалента!
int KeyNameMacroToKey(const wchar_t *Name)
{
	// пройдемся по всем модификаторам
	for (int I=0; I < int(ARRAYSIZE(KeyMacroCodes)); ++I)
		if (!StrCmpI(Name,KeyMacroCodes[I].Name))
			return KeyMacroCodes[I].Key;

	return -1;
}

#if 0
static bool checkMacroFarIntConst(string &strValueName)
{
	return
		strValueName==constMsX ||
		strValueName==constMsY ||
		strValueName==constMsButton ||
		strValueName==constMsCtrlState ||
		strValueName==constMsEventFlags ||
		strValueName==constRCounter ||
		strValueName==constFarCfgErr;
}
#endif

static void initMacroFarIntConst()
{
	INT64 TempValue=0;
	KeyMacro::SetMacroConst(constMsX,TempValue);
	KeyMacro::SetMacroConst(constMsY,TempValue);
	KeyMacro::SetMacroConst(constMsButton,TempValue);
	KeyMacro::SetMacroConst(constMsCtrlState,TempValue);
	KeyMacro::SetMacroConst(constMsEventFlags,TempValue);
	KeyMacro::SetMacroConst(constRCounter,TempValue);
	KeyMacro::SetMacroConst(constFarCfgErr,TempValue);
}

const TVar& TVMStack::Pop()
{
	static TVar temp; //чтоб можно было вернуть по референс.

	if (TStack<TVar>::Pop(temp))
		return temp;

	return Error;
};

void TVMStack::Swap()
{
	TStack<TVar>::Swap();
}

TVar& TVMStack::Pop(TVar &dest)
{
	if (!TStack<TVar>::Pop(dest))
		dest=Error;

	return dest;
};

const TVar& TVMStack::Peek()
{
	TVar *var = TStack<TVar>::Peek();

	if (var)
		return *var;

	return Error;
};

KeyMacro::KeyMacro():
	MacroVersion(2),
	Recording(MACROMODE_NOMACRO),
	InternalInput(0),
	IsRedrawEditor(TRUE),
	Mode(MACRO_SHELL),
	StartMode(MACRO_SHELL),
	CurPCStack(-1),
	StopMacro(false),
	MacroLIBCount(0),
	MacroLIB(nullptr),
	RecBufferSize(0),
	RecBuffer(nullptr),
	RecSrc(nullptr),
	RecDescription(nullptr),
	LockScr(nullptr)
{
	Work.Init(nullptr);
	ClearArray(IndexMode);
}

KeyMacro::~KeyMacro()
{
	InitInternalVars();

	if (Work.AllocVarTable && Work.locVarTable)
		xf_free(Work.locVarTable);

	DestroyMacroLib();

	UnregMacroFunction(-1);
}

void KeyMacro::DestroyMacroLib()
{
	if (MacroLIB)
	{
		while(MacroLIBCount) DelMacro(MacroLIBCount-1);
		xf_free(MacroLIB);
		MacroLIB=nullptr;
	}
}

void KeyMacro::InitInternalLIBVars()
{
	if (MacroLIB)
	{
		for (int ii=0;ii<MacroLIBCount;)
		{
			if (IsEqualGUID(FarGuid,MacroLIB[ii].Guid))
				DelMacro(ii);
			else
				++ii;
		}
		if (0==MacroLIBCount)
		{
			xf_free(MacroLIB);
				MacroLIB=nullptr;
		}
	}
	else
	{
		MacroLIBCount=0;
	}

	if (RecBuffer)
		xf_free(RecBuffer);
	RecBuffer=nullptr;
	RecBufferSize=0;

	ClearArray(IndexMode);
 	//MacroLIBCount=0;
 	//MacroLIB=nullptr;
	//LastOpCodeUF=KEY_MACRO_U_BASE;
}

// инициализация всех переменных
void KeyMacro::InitInternalVars(BOOL InitedRAM)
{
	InitInternalLIBVars();

	if (LockScr)
	{
		delete LockScr;
		LockScr=nullptr;
	}

	if (InitedRAM)
	{
		ReleaseWORKBuffer(TRUE);
		Work.Executing=MACROMODE_NOMACRO;
	}

	Work.HistoryDisable=0;
	RecBuffer=nullptr;
	RecBufferSize=0;
	RecSrc=nullptr;
	RecDescription=nullptr;
	Recording=MACROMODE_NOMACRO;
	InternalInput=FALSE;
	VMStack.Free();
	CurPCStack=-1;
}

// удаление временного буфера, если он создавался динамически
// (динамически - значит в PlayMacros передали строку.
void KeyMacro::ReleaseWORKBuffer(BOOL All)
{
	if (Work.MacroWORK)
	{
		if (All || Work.MacroWORKCount <= 1)
		{
			for (int I=0; I<Work.MacroWORKCount; I++)
			{
				if (Work.MacroWORK[I].BufferSize > 1 && Work.MacroWORK[I].Buffer)
					xf_free(Work.MacroWORK[I].Buffer);

				if (Work.MacroWORK[I].Src)
					xf_free(Work.MacroWORK[I].Src);

				if (Work.MacroWORK[I].Name)
					xf_free(Work.MacroWORK[I].Name);

				if (Work.MacroWORK[I].Description)
					xf_free(Work.MacroWORK[I].Description);
			}

			xf_free(Work.MacroWORK);

			if (Work.AllocVarTable)
			{
				deleteVTable(*Work.locVarTable);
				//xf_free(Work.locVarTable);
				//Work.locVarTable=nullptr;
				//Work.AllocVarTable=false;
			}

			Work.MacroWORK=nullptr;
			Work.MacroWORKCount=0;
		}
		else
		{
			if (Work.MacroWORK[0].BufferSize > 1 && Work.MacroWORK[0].Buffer)
				xf_free(Work.MacroWORK[0].Buffer);

			if (Work.MacroWORK[0].Src)
				xf_free(Work.MacroWORK[0].Src);

			if (Work.MacroWORK[0].Name)
				xf_free(Work.MacroWORK[0].Name);

			if (Work.MacroWORK[0].Description)
				xf_free(Work.MacroWORK[0].Description);

			if (Work.AllocVarTable)
			{
				deleteVTable(*Work.locVarTable);
				//xf_free(Work.locVarTable);
				//Work.locVarTable=nullptr;
				//Work.AllocVarTable=false;
			}

			Work.MacroWORKCount--;
			memmove(Work.MacroWORK,((BYTE*)Work.MacroWORK)+sizeof(MacroRecord),sizeof(MacroRecord)*Work.MacroWORKCount);
			Work.MacroWORK=(MacroRecord *)xf_realloc(Work.MacroWORK,sizeof(MacroRecord)*Work.MacroWORKCount);
		}
	}
}

// загрузка ВСЕХ макросов из реестра
int KeyMacro::LoadMacros(BOOL InitedRAM,BOOL LoadAll)
{
	int ErrCount=0;
	InitInternalVars(InitedRAM);

	if (Opt.Macro.DisableMacro&MDOL_ALL)
		return FALSE;

	string strBuffer;
	ReadVarsConsts();
	ReadPluginFunctions();

	int Areas[MACRO_LAST];

	for (int i=MACRO_OTHER; i < MACRO_LAST; i++)
	{
		Areas[i]=i;
	}

	if (!LoadAll)
	{
		// "выведем из строя" ненужные области - будет загружаться только то, что не равно значению MACRO_LAST
		Areas[MACRO_SHELL]=
			Areas[MACRO_SEARCH]=
			Areas[MACRO_DISKS]=
			Areas[MACRO_MAINMENU]=
			Areas[MACRO_INFOPANEL]=
			Areas[MACRO_QVIEWPANEL]=
			Areas[MACRO_TREEPANEL]=
			Areas[MACRO_USERMENU]= // <-- Mantis#0001594
			Areas[MACRO_SHELLAUTOCOMPLETION]=
			Areas[MACRO_FINDFOLDER]=MACRO_LAST;
	}

	for (int i=MACRO_OTHER; i < MACRO_LAST; i++)
	{
		if (Areas[i] == MACRO_LAST)
			continue;

		if (!ReadKeyMacro(i))
		{
			ErrCount++;
		}
	}

	KeyMacro::Sort();

	return ErrCount?FALSE:TRUE;
}

int KeyMacro::ProcessEvent(const struct FAR_INPUT_RECORD *Rec)
{
	string strKey;
	int Key=Rec->IntKey;

	if (InternalInput || Key==KEY_IDLE || Key==KEY_NONE || !FrameManager->GetCurrentFrame())
		return FALSE;

	if (Recording) // Идет запись?
	{
		// признак конца записи?
		if (Key==Opt.Macro.KeyMacroCtrlDot || Key==Opt.Macro.KeyMacroRCtrlDot
			|| Key==Opt.Macro.KeyMacroCtrlShiftDot || Key==Opt.Macro.KeyMacroRCtrlShiftDot)
		{
			_KEYMACRO(CleverSysLog Clev(L"MACRO End record..."));
			int WaitInMainLoop0=WaitInMainLoop;
			InternalInput=TRUE;
			WaitInMainLoop=FALSE;
			// Залочить _текущий_ фрейм, а не _последний немодальный_
			FrameManager->GetCurrentFrame()->Lock(); // отменим прорисовку фрейма
			DWORD MacroKey;
			// выставляем флаги по умолчанию.
			UINT64 Flags=MFLAGS_DISABLEOUTPUT|MFLAGS_CALLPLUGINENABLEMACRO; // ???
			int AssignRet=AssignMacroKey(MacroKey,Flags);
			FrameManager->ResetLastInputRecord();
			FrameManager->GetCurrentFrame()->Unlock(); // теперь можно :-)
			// добавим проверку на удаление
			// если удаляем или был вызван диалог изменения, то не нужно выдавать диалог настройки.
			//if (MacroKey != (DWORD)-1 && (Key==KEY_CTRLSHIFTDOT || Recording==2) && RecBufferSize)
			if (AssignRet && AssignRet!=2 && RecBufferSize
				&& (Key==Opt.Macro.KeyMacroCtrlShiftDot || Key==Opt.Macro.KeyMacroRCtrlShiftDot))
			{
				if (!GetMacroSettings(MacroKey,Flags))
					AssignRet=0;
			}

			WaitInMainLoop=WaitInMainLoop0;
			InternalInput=FALSE;

			if (!AssignRet)
			{
				if (RecBuffer)
				{
					xf_free(RecBuffer);
					RecBuffer=nullptr;
					RecBufferSize=0;
				}
			}
			else
			{
				// в области common будем искать только при удалении
				int Pos=GetIndex(MacroKey,strKey,StartMode,!(RecBuffer && RecBufferSize),true);

				if (Pos == -1)
				{
					Pos=MacroLIBCount;

					if (RecBufferSize > 0)
					{
						MacroRecord *NewMacroLIB=(MacroRecord *)xf_realloc(MacroLIB,sizeof(*MacroLIB)*(MacroLIBCount+1));

						if (!NewMacroLIB)
						{
							WaitInFastFind++;
							return FALSE;
						}

						MacroLIB=NewMacroLIB;
						MacroLIBCount++;
					}
				}
				else
				{
					if (MacroLIB[Pos].BufferSize > 1 && MacroLIB[Pos].Buffer)
						xf_free(MacroLIB[Pos].Buffer);

					if (MacroLIB[Pos].Src)
						xf_free(MacroLIB[Pos].Src);

					if (MacroLIB[Pos].Name)
						xf_free(MacroLIB[Pos].Name);

					if (MacroLIB[Pos].Description)
						xf_free(MacroLIB[Pos].Description);

					MacroLIB[Pos].Buffer=nullptr;
					MacroLIB[Pos].Src=nullptr;
					MacroLIB[Pos].Name=nullptr;
					MacroLIB[Pos].Callback=nullptr;
					MacroLIB[Pos].Description=nullptr;
				}

				if (Pos < MacroLIBCount)
				{
					MacroRecord Macro = {0};
					Macro.Key=MacroKey;

					if (RecBufferSize > 0 && !RecSrc)
						RecBuffer[RecBufferSize++]=MCODE_OP_ENDKEYS;

					if (RecBufferSize > 1)
						Macro.Buffer=RecBuffer;
					else if (RecBuffer && RecBufferSize > 0)
						Macro.Buffer=reinterpret_cast<DWORD*>((intptr_t)(*RecBuffer));
					else if (!RecBufferSize)
						Macro.Buffer=nullptr;

					Macro.BufferSize=RecBufferSize;
					Macro.Src=RecSrc?RecSrc:MkTextSequence(Macro.Buffer,Macro.BufferSize);
					Macro.Description=RecDescription;

					// если удаляем макрос - скорректируем StartMode,
					// иначе макрос из common получит ту область, в которой его решили удалить.
					if (!Macro.BufferSize||!Macro.Src)
						StartMode=MacroLIB[Pos].Flags&MFLAGS_MODEMASK;

					Macro.Flags=Flags|(StartMode&MFLAGS_MODEMASK)|MFLAGS_NEEDSAVEMACRO|(Recording==MACROMODE_RECORDING_COMMON?0:MFLAGS_NOSENDKEYSTOPLUGINS);
					Macro.Guid=FarGuid;
					Macro.Id=nullptr;
					Macro.Callback=nullptr;

					string strKeyText;
					if (KeyToText(MacroKey, strKeyText))
						Macro.Name=xf_wcsdup(strKeyText);
					else
						Macro.Name=nullptr;

					MacroLIB[Pos]=Macro;
				}
			}

			Recording=MACROMODE_NOMACRO;
			RecBuffer=nullptr;
			RecBufferSize=0;
			RecSrc=nullptr;
			RecDescription=nullptr;
			ScrBuf.RestoreMacroChar();
			WaitInFastFind++;
			KeyMacro::Sort();

			if (Opt.AutoSaveSetup)
				WriteMacroRecords(); // записать только изменения!

			return TRUE;
		}
		else // процесс записи продолжается.
		{
			if ((unsigned int)Key>=KEY_NONE && (unsigned int)Key<=KEY_END_SKEY) // специальные клавиши прокинем
				return FALSE;

			RecBuffer=(DWORD *)xf_realloc(RecBuffer,sizeof(*RecBuffer)*(RecBufferSize+3));

			if (!RecBuffer)
			{
				RecBufferSize=0;
				return FALSE;
			}

			if (IntKeyState.ReturnAltValue) // "подтасовка" фактов ;-)
				Key|=KEY_ALTDIGIT;

			if (!RecBufferSize)
				RecBuffer[RecBufferSize++]=MCODE_OP_KEYS;

			RecBuffer[RecBufferSize++]=Key;
			return FALSE;
		}
	}
	// Начало записи?
	else if (Key==Opt.Macro.KeyMacroCtrlDot || Key==Opt.Macro.KeyMacroRCtrlDot
			|| Key==Opt.Macro.KeyMacroCtrlShiftDot || Key==Opt.Macro.KeyMacroRCtrlShiftDot)
	{
		_KEYMACRO(CleverSysLog Clev(L"MACRO Begin record..."));

		// Полиция 18
		if (Opt.Policies.DisabledOptions&FFPOL_CREATEMACRO)
			return FALSE;

		//if(CtrlObject->Plugins->CheckFlags(PSIF_ENTERTOOPENPLUGIN))
		//	return FALSE;

		if (LockScr)
			delete LockScr;
		LockScr=nullptr;

		// Где мы?
		StartMode=(Mode==MACRO_SHELL && !WaitInMainLoop)?MACRO_OTHER:Mode;
		// тип записи - с вызовом диалога настроек или...
		// В зависимости от того, КАК НАЧАЛИ писать макрос, различаем общий режим (Ctrl-.
		// с передачей плагину кеев) или специальный (Ctrl-Shift-. - без передачи клавиш плагину)
		Recording=(Key==Opt.Macro.KeyMacroCtrlDot || Key==Opt.Macro.KeyMacroRCtrlDot) ? MACROMODE_RECORDING_COMMON:MACROMODE_RECORDING;

		if (RecBuffer)
			xf_free(RecBuffer);
		RecBuffer=nullptr;
		RecBufferSize=0;

		RecSrc=nullptr;
		RecDescription=nullptr;
		ScrBuf.ResetShadow();
		ScrBuf.Flush();
		WaitInFastFind--;
		return TRUE;
	}
	else
	{
		if (Work.Executing == MACROMODE_NOMACRO) // Это еще не режим исполнения?
		{
			//_KEYMACRO(CleverSysLog Clev(L"MACRO find..."));
			//_KEYMACRO(SysLog(L"Param Key=%s",_FARKEY_ToName(Key)));
			UINT64 CurFlags;

			StopMacro=false;

			if ((Key&(~KEY_CTRLMASK)) > 0x01 && (Key&(~KEY_CTRLMASK)) < KEY_FKEY_BEGIN) // 0xFFFF ??
			{
				//Key=KeyToKeyLayout(Key&0x0000FFFF)|(Key&(~0x0000FFFF));
				//Key=Upper(Key&0x0000FFFF)|(Key&(~0x0000FFFF));
				//_KEYMACRO(SysLog(L"Upper(Key)=%s",_FARKEY_ToName(Key)));

				if ((Key&(~KEY_CTRLMASK)) > 0x7F && (Key&(~KEY_CTRLMASK)) < KEY_FKEY_BEGIN)
					Key=KeyToKeyLayout(Key&0x0000FFFF)|(Key&(~0x0000FFFF));

				if ((DWORD)Key < KEY_FKEY_BEGIN)
					Key=Upper(Key&0x0000FFFF)|(Key&(~0x0000FFFF));

			}

			int I=GetIndex(Key,strKey,(Mode==MACRO_SHELL && !WaitInMainLoop) ? MACRO_OTHER:Mode);

			if (I != -1 && !((CurFlags=MacroLIB[I].Flags)&MFLAGS_DISABLEMACRO) && CtrlObject)
			{
				_KEYMACRO(SysLog(L"[%d] Found KeyMacro (I=%d Key=%s,%s)",__LINE__,I,_FARKEY_ToName(Key),_FARKEY_ToName(MacroLIB[I].Key)));

				if (!CheckAll(Mode,CurFlags))
					return FALSE;

				// Скопируем текущее исполнение в MacroWORK
				//PostNewMacro(MacroLIB+I);
				// Подавлять вывод?
				if (CurFlags&MFLAGS_DISABLEOUTPUT)
				{
					if (LockScr)
						delete LockScr;

					LockScr=new LockScreen;
				}

				// различаем общий режим (с передачей плагину кеев) или специальный (без передачи клавиш плагину)
				Work.HistoryDisable=0;
				Work.ExecLIBPos=0;
				PostNewMacro(MacroLIB+I);
				//Work.cRec=*FrameManager->GetLastInputRecord();
				Work.cRec=Rec->Rec;
				_SVS(FarSysLog_INPUT_RECORD_Dump(L"Macro",&Work.cRec));
				Work.MacroPC=I;
				IsRedrawEditor=CtrlObject->Plugins->CheckFlags(PSIF_ENTERTOOPENPLUGIN)?FALSE:TRUE;
				_KEYMACRO(SysLog(L"**** Start Of Execute Macro ****"));
				_KEYMACRO(SysLog(1));
				return TRUE;
			}
		}

		return FALSE;
	}
}

bool KeyMacro::GetPlainText(string& strDest)
{
	strDest.Clear();

	if (!Work.MacroWORK)
		return false;

	MacroRecord *MR=Work.MacroWORK;
	int LenTextBuf=(int)(StrLength((wchar_t*)&MR->Buffer[Work.ExecLIBPos])+1)*sizeof(wchar_t);

	if (LenTextBuf && MR->Buffer[Work.ExecLIBPos])
	{
		strDest=(const wchar_t *)&MR->Buffer[Work.ExecLIBPos];
		_SVS(SysLog(L"strDest='%s'",strDest.CPtr()));
		_SVS(SysLog(L"Work.ExecLIBPos=%d",Work.ExecLIBPos));
		size_t nSize = LenTextBuf/sizeof(DWORD);
		if (LenTextBuf == sizeof(wchar_t) || (LenTextBuf % sizeof(DWORD)) )    // дополнение до sizeof(DWORD) нулями.
			nSize++;
		Work.ExecLIBPos+=static_cast<int>(nSize);
		_SVS(SysLog(L"Work.ExecLIBPos=%d",Work.ExecLIBPos));
		return true;
	}
	else
	{
		Work.ExecLIBPos++;
	}

	return false;
}

int KeyMacro::GetPlainTextSize()
{
	if (!Work.MacroWORK)
		return 0;

	MacroRecord *MR=Work.MacroWORK;
	return StrLength((wchar_t*)&MR->Buffer[Work.ExecLIBPos]);
}

TVar KeyMacro::FARPseudoVariable(UINT64 Flags,DWORD CheckCode,DWORD& Err)
{
	_KEYMACRO(CleverSysLog Clev(L"KeyMacro::FARPseudoVariable()"));
	size_t I;
	TVar Cond(0ll);
	string strFileName;
	DWORD FileAttr=INVALID_FILE_ATTRIBUTES;

	// Найдем индекс нужного кейворда
	for (I=0 ; I < ARRAYSIZE(MKeywords) ; ++I)
		if (MKeywords[I].Value == CheckCode)
			break;

	if (I == ARRAYSIZE(MKeywords))
	{
		Err=1;
		_KEYMACRO(SysLog(L"return; Err=%d",Err));
		return Cond; // здесь TRUE обязательно, чтобы прекратить выполнение макроса, ибо код не распознан.
	}

	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;

	// теперь сделаем необходимые проверки
	switch (MKeywords[I].Type)
	{
		case 0: // проверка на область
		{
			if (WaitInMainLoop) // здесь надо учесть тот самый WaitInMainLoop, хотя могу и ошибаться!!!
				Cond=int(CheckCode-MCODE_C_AREA_OTHER+MACRO_OTHER) == FrameManager->GetCurrentFrame()->GetMacroMode()?1:0;
			else
				Cond=int(CheckCode-MCODE_C_AREA_OTHER+MACRO_OTHER) == CtrlObject->Macro.GetMode()?1:0;

			break;
		}
		case 2:
		{
			Panel *PassivePanel=nullptr;

			if (ActivePanel)
				PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

			Frame* CurFrame=FrameManager->GetCurrentFrame();

			switch (CheckCode)
			{
				case MCODE_V_FAR_WIDTH:
					Cond=(__int64)(ScrX+1);
					break;
				case MCODE_V_FAR_HEIGHT:
					Cond=(__int64)(ScrY+1);
					break;
				case MCODE_V_FAR_TITLE:
					Console.GetTitle(strFileName);
					Cond=strFileName.CPtr();
					break;
				case MCODE_V_FAR_PID:
					Cond=(__int64)GetCurrentProcessId();
					break;
				case MCODE_V_FAR_UPTIME:
				{
					LARGE_INTEGER Frequency, Counter;
					QueryPerformanceFrequency(&Frequency);
					QueryPerformanceCounter(&Counter);
					Cond=((Counter.QuadPart-FarUpTime.QuadPart)*1000)/Frequency.QuadPart;
					break;
				}
				case MCODE_V_MACRO_AREA:
					Cond=GetAreaName(CtrlObject->Macro.GetMode());
					break;
				case MCODE_C_FULLSCREENMODE: // Fullscreen?
					Cond=IsConsoleFullscreen()?1:0;
					break;
				case MCODE_C_ISUSERADMIN: // IsUserAdmin?
					Cond=(__int64)Opt.IsUserAdmin;
					break;
				case MCODE_V_DRVSHOWPOS: // Drv.ShowPos
					Cond=(__int64)Macro_DskShowPosType;
					break;
				case MCODE_V_DRVSHOWMODE: // Drv.ShowMode
					Cond=(__int64)Opt.ChangeDriveMode;
					break;
				case MCODE_C_CMDLINE_BOF:              // CmdLine.Bof - курсор в начале cmd-строки редактирования?
				case MCODE_C_CMDLINE_EOF:              // CmdLine.Eof - курсор в конеце cmd-строки редактирования?
				case MCODE_C_CMDLINE_EMPTY:            // CmdLine.Empty
				case MCODE_C_CMDLINE_SELECTED:         // CmdLine.Selected
				case MCODE_V_CMDLINE_ITEMCOUNT:        // CmdLine.ItemCount
				case MCODE_V_CMDLINE_CURPOS:           // CmdLine.CurPos
				{
					Cond=CtrlObject->CmdLine?CtrlObject->CmdLine->VMProcess(CheckCode):-1;
					break;
				}
				case MCODE_V_CMDLINE_VALUE:            // CmdLine.Value
				{
					if (CtrlObject->CmdLine)
						CtrlObject->CmdLine->GetString(strFileName);
					Cond=strFileName.CPtr();
					break;
				}
				case MCODE_C_APANEL_ROOT:  // APanel.Root
				case MCODE_C_PPANEL_ROOT:  // PPanel.Root
				{
					Panel *SelPanel=(CheckCode==MCODE_C_APANEL_ROOT)?ActivePanel:PassivePanel;

					if (SelPanel)
						Cond=SelPanel->VMProcess(MCODE_C_ROOTFOLDER)?1:0;

					break;
				}
				case MCODE_C_APANEL_BOF:
				case MCODE_C_PPANEL_BOF:
				case MCODE_C_APANEL_EOF:
				case MCODE_C_PPANEL_EOF:
				{
					Panel *SelPanel=(CheckCode==MCODE_C_APANEL_BOF || CheckCode==MCODE_C_APANEL_EOF)?ActivePanel:PassivePanel;

					if (SelPanel)
						Cond=SelPanel->VMProcess(CheckCode==MCODE_C_APANEL_BOF || CheckCode==MCODE_C_PPANEL_BOF?MCODE_C_BOF:MCODE_C_EOF)?1:0;

					break;
				}
				case MCODE_C_SELECTED:    // Selected?
				{
					int NeedType = Mode == MACRO_EDITOR? MODALTYPE_EDITOR : (Mode == MACRO_VIEWER? MODALTYPE_VIEWER : (Mode == MACRO_DIALOG? MODALTYPE_DIALOG : MODALTYPE_PANELS));

					if (!(Mode == MACRO_USERMENU || Mode == MACRO_MAINMENU || Mode == MACRO_MENU) && CurFrame && CurFrame->GetType()==NeedType)
					{
						int CurSelected;

						if (Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
							CurSelected=(int)CtrlObject->CmdLine->VMProcess(CheckCode);
						else
							CurSelected=(int)CurFrame->VMProcess(CheckCode);

						Cond=CurSelected?1:0;
					}
					else
					{
					#if 1
						Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

						//f=f->GetTopModal();
						while (f)
						{
							fo=f;
							f=f->GetTopModal();
						}

						if (!f)
							f=fo;

						if (f)
						{
							Cond=f->VMProcess(CheckCode);
						}
					#else

						Frame *f=FrameManager->GetTopModal();

						if (f)
							Cond=(__int64)f->VMProcess(CheckCode);
					#endif
					}
					break;
				}
				case MCODE_C_EMPTY:   // Empty
				case MCODE_C_BOF:
				case MCODE_C_EOF:
				{
					int CurMMode=CtrlObject->Macro.GetMode();

					if (!(Mode == MACRO_USERMENU || Mode == MACRO_MAINMENU || Mode == MACRO_MENU) && CurFrame && CurFrame->GetType() == MODALTYPE_PANELS && !(CurMMode == MACRO_INFOPANEL || CurMMode == MACRO_QVIEWPANEL || CurMMode == MACRO_TREEPANEL))
					{
						if (CheckCode == MCODE_C_EMPTY)
							Cond=CtrlObject->CmdLine->GetLength()?0:1;
						else
							Cond=CtrlObject->CmdLine->VMProcess(CheckCode);
					}
					else
					{
						//if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
						{
							Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

							//f=f->GetTopModal();
							while (f)
							{
								fo=f;
								f=f->GetTopModal();
							}

							if (!f)
								f=fo;

							if (f)
							{
								Cond=f->VMProcess(CheckCode);
							}
						}
					}

					break;
				}
				case MCODE_V_DLGITEMCOUNT: // Dlg.ItemCount
				case MCODE_V_DLGCURPOS:    // Dlg.CurPos
				case MCODE_V_DLGITEMTYPE:  // Dlg.ItemType
				case MCODE_V_DLGPREVPOS:   // Dlg.PrevPos
				{
					if (CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG) // ?? Mode == MACRO_DIALOG ??
						Cond=(__int64)CurFrame->VMProcess(CheckCode);

					break;
				}
				case MCODE_V_DLGINFOID:        // Dlg.Info.Id
				{
					if (CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG) // ?? Mode == MACRO_DIALOG ??
					{
						Cond=reinterpret_cast<LPCWSTR>(static_cast<intptr_t>(CurFrame->VMProcess(CheckCode)));
					}

					break;
				}
				case MCODE_V_DLGINFOOWNER:        // Dlg.Info.Owner
				{
					if (CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG) // ?? Mode == MACRO_DIALOG ??
					{
						Cond=reinterpret_cast<LPCWSTR>(static_cast<intptr_t>(CurFrame->VMProcess(CheckCode)));
					}

					break;
				}
				case MCODE_C_APANEL_VISIBLE:  // APanel.Visible
				case MCODE_C_PPANEL_VISIBLE:  // PPanel.Visible
				{
					Panel *SelPanel=CheckCode==MCODE_C_APANEL_VISIBLE?ActivePanel:PassivePanel;

					if (SelPanel)
						Cond = SelPanel->IsVisible()?1:0;

					break;
				}
				case MCODE_C_APANEL_ISEMPTY: // APanel.Empty
				case MCODE_C_PPANEL_ISEMPTY: // PPanel.Empty
				{
					Panel *SelPanel=CheckCode==MCODE_C_APANEL_ISEMPTY?ActivePanel:PassivePanel;

					if (SelPanel)
					{
						SelPanel->GetFileName(strFileName,SelPanel->GetCurrentPos(),FileAttr);
						int GetFileCount=SelPanel->GetFileCount();
						Cond=(!GetFileCount ||
						      (GetFileCount == 1 && TestParentFolderName(strFileName)))
						     ?1:0;
					}

					break;
				}
				case MCODE_C_APANEL_FILTER:
				case MCODE_C_PPANEL_FILTER:
				{
					Panel *SelPanel=(CheckCode==MCODE_C_APANEL_FILTER)?ActivePanel:PassivePanel;

					if (SelPanel)
						Cond=SelPanel->VMProcess(MCODE_C_APANEL_FILTER)?1:0;

					break;
				}
				case MCODE_C_APANEL_LFN:
				case MCODE_C_PPANEL_LFN:
				{
					Panel *SelPanel = CheckCode == MCODE_C_APANEL_LFN ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond = SelPanel->GetShowShortNamesMode()?0:1;

					break;
				}
				case MCODE_C_APANEL_LEFT: // APanel.Left
				case MCODE_C_PPANEL_LEFT: // PPanel.Left
				{
					Panel *SelPanel = CheckCode == MCODE_C_APANEL_LEFT ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond = SelPanel == CtrlObject->Cp()->LeftPanel ? 1 : 0;

					break;
				}
				case MCODE_C_APANEL_FILEPANEL: // APanel.FilePanel
				case MCODE_C_PPANEL_FILEPANEL: // PPanel.FilePanel
				{
					Panel *SelPanel = CheckCode == MCODE_C_APANEL_FILEPANEL ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond=SelPanel->GetType() == FILE_PANEL;

					break;
				}
				case MCODE_C_APANEL_PLUGIN: // APanel.Plugin
				case MCODE_C_PPANEL_PLUGIN: // PPanel.Plugin
				{
					Panel *SelPanel=CheckCode==MCODE_C_APANEL_PLUGIN?ActivePanel:PassivePanel;

					if (SelPanel)
						Cond=SelPanel->GetMode() == PLUGIN_PANEL;

					break;
				}
				case MCODE_C_APANEL_FOLDER: // APanel.Folder
				case MCODE_C_PPANEL_FOLDER: // PPanel.Folder
				{
					Panel *SelPanel=CheckCode==MCODE_C_APANEL_FOLDER?ActivePanel:PassivePanel;

					if (SelPanel)
					{
						SelPanel->GetFileName(strFileName,SelPanel->GetCurrentPos(),FileAttr);

						if (FileAttr != INVALID_FILE_ATTRIBUTES)
							Cond=(FileAttr&FILE_ATTRIBUTE_DIRECTORY)?1:0;
					}

					break;
				}
				case MCODE_C_APANEL_SELECTED: // APanel.Selected
				case MCODE_C_PPANEL_SELECTED: // PPanel.Selected
				{
					Panel *SelPanel=CheckCode==MCODE_C_APANEL_SELECTED?ActivePanel:PassivePanel;

					if (SelPanel)
					{
						Cond = SelPanel->GetRealSelCount() > 0; //??
					}

					break;
				}
				case MCODE_V_APANEL_CURRENT: // APanel.Current
				case MCODE_V_PPANEL_CURRENT: // PPanel.Current
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_CURRENT ? ActivePanel : PassivePanel;

					Cond = L"";

					if (SelPanel )
					{
						SelPanel->GetFileName(strFileName,SelPanel->GetCurrentPos(),FileAttr);

						if (FileAttr != INVALID_FILE_ATTRIBUTES)
							Cond = strFileName.CPtr();
					}

					break;
				}
				case MCODE_V_APANEL_SELCOUNT: // APanel.SelCount
				case MCODE_V_PPANEL_SELCOUNT: // PPanel.SelCount
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_SELCOUNT ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond = (__int64)SelPanel->GetRealSelCount();

					break;
				}
				case MCODE_V_APANEL_COLUMNCOUNT:       // APanel.ColumnCount - активная панель:  количество колонок
				case MCODE_V_PPANEL_COLUMNCOUNT:       // PPanel.ColumnCount - пассивная панель: количество колонок
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_COLUMNCOUNT ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond = (__int64)SelPanel->GetColumnsCount();

					break;
				}
				case MCODE_V_APANEL_WIDTH: // APanel.Width
				case MCODE_V_PPANEL_WIDTH: // PPanel.Width
				case MCODE_V_APANEL_HEIGHT: // APanel.Height
				case MCODE_V_PPANEL_HEIGHT: // PPanel.Height
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_WIDTH || CheckCode == MCODE_V_APANEL_HEIGHT? ActivePanel : PassivePanel;

					if (SelPanel )
					{
						int X1, Y1, X2, Y2;
						SelPanel->GetPosition(X1,Y1,X2,Y2);

						if (CheckCode == MCODE_V_APANEL_HEIGHT || CheckCode == MCODE_V_PPANEL_HEIGHT)
							Cond = (__int64)(Y2-Y1+1);
						else
							Cond = (__int64)(X2-X1+1);
					}

					break;
				}

				case MCODE_V_APANEL_OPIFLAGS:  // APanel.OPIFlags
				case MCODE_V_PPANEL_OPIFLAGS:  // PPanel.OPIFlags
				case MCODE_V_APANEL_HOSTFILE: // APanel.HostFile
				case MCODE_V_PPANEL_HOSTFILE: // PPanel.HostFile
				case MCODE_V_APANEL_FORMAT:           // APanel.Format
				case MCODE_V_PPANEL_FORMAT:           // PPanel.Format
				{
					Panel *SelPanel =
							CheckCode == MCODE_V_APANEL_OPIFLAGS ||
							CheckCode == MCODE_V_APANEL_HOSTFILE ||
							CheckCode == MCODE_V_APANEL_FORMAT? ActivePanel : PassivePanel;

					if (CheckCode == MCODE_V_APANEL_HOSTFILE || CheckCode == MCODE_V_PPANEL_HOSTFILE ||
						CheckCode == MCODE_V_APANEL_FORMAT || CheckCode == MCODE_V_PPANEL_FORMAT)
						Cond = L"";

					if (SelPanel )
					{
						if (SelPanel->GetMode() == PLUGIN_PANEL)
						{
							OpenPanelInfo Info={};
							Info.StructSize=sizeof(OpenPanelInfo);
							SelPanel->GetOpenPanelInfo(&Info);
							switch (CheckCode)
							{
								case MCODE_V_APANEL_OPIFLAGS:
								case MCODE_V_PPANEL_OPIFLAGS:
								Cond = (__int64)Info.Flags;
									break;
								case MCODE_V_APANEL_HOSTFILE:
								case MCODE_V_PPANEL_HOSTFILE:
								Cond = Info.HostFile;
									break;
								case MCODE_V_APANEL_FORMAT:
								case MCODE_V_PPANEL_FORMAT:
									Cond = Info.Format;
									break;
						}
					}
					}

					break;
				}

				case MCODE_V_APANEL_PREFIX:           // APanel.Prefix
				case MCODE_V_PPANEL_PREFIX:           // PPanel.Prefix
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_PREFIX ? ActivePanel : PassivePanel;

					Cond = L"";

					if (SelPanel )
					{
						PluginInfo PInfo = {sizeof(PInfo)};
						if (SelPanel->VMProcess(MCODE_V_APANEL_PREFIX,&PInfo))
							Cond = PInfo.CommandPrefix;
					}

					break;
				}

				case MCODE_V_APANEL_PATH0:           // APanel.Path0
				case MCODE_V_PPANEL_PATH0:           // PPanel.Path0
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_PATH0 ? ActivePanel : PassivePanel;

					Cond = L"";

					if (SelPanel )
					{
						if (!SelPanel->VMProcess(CheckCode,&strFileName,0))
							SelPanel->GetCurDir(strFileName);
						Cond = strFileName.CPtr();
					}

					break;
				}

				case MCODE_V_APANEL_PATH: // APanel.Path
				case MCODE_V_PPANEL_PATH: // PPanel.Path
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_PATH ? ActivePanel : PassivePanel;

					Cond = L"";

					if (SelPanel )
					{
						if (SelPanel->GetMode() == PLUGIN_PANEL)
						{
							OpenPanelInfo Info={};
							Info.StructSize=sizeof(OpenPanelInfo);
							SelPanel->GetOpenPanelInfo(&Info);
							strFileName = Info.CurDir;
						}
						else
							SelPanel->GetCurDir(strFileName);
						DeleteEndSlash(strFileName); // - чтобы у корня диска было C:, тогда можно писать так: APanel.Path + "\\file"
						Cond = strFileName.CPtr();
					}

					break;
				}

				case MCODE_V_APANEL_UNCPATH: // APanel.UNCPath
				case MCODE_V_PPANEL_UNCPATH: // PPanel.UNCPath
				{
					Cond = L"";

					if (_MakePath1(CheckCode == MCODE_V_APANEL_UNCPATH?KEY_ALTSHIFTBRACKET:KEY_ALTSHIFTBACKBRACKET,strFileName,L""))
					{
						UnquoteExternal(strFileName);
						DeleteEndSlash(strFileName);
						Cond = strFileName.CPtr();
					}

					break;
				}
				//FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
				case MCODE_V_APANEL_TYPE: // APanel.Type
				case MCODE_V_PPANEL_TYPE: // PPanel.Type
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_TYPE ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond=(__int64)SelPanel->GetType();

					break;
				}
				case MCODE_V_APANEL_DRIVETYPE: // APanel.DriveType - активная панель: тип привода
				case MCODE_V_PPANEL_DRIVETYPE: // PPanel.DriveType - пассивная панель: тип привода
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_DRIVETYPE ? ActivePanel : PassivePanel;
					Cond=-1;

					if (SelPanel  && SelPanel->GetMode() != PLUGIN_PANEL)
					{
						SelPanel->GetCurDir(strFileName);
						GetPathRoot(strFileName, strFileName);
						UINT DriveType=FAR_GetDriveType(strFileName,nullptr,0);

						// BUGBUG: useless, GetPathRoot expands subst itself

						/*if (ParsePath(strFileName) == PATH_DRIVELETTER)
						{
							string strRemoteName;
							strFileName.SetLength(2);

							if (GetSubstName(DriveType,strFileName,strRemoteName))
								DriveType=DRIVE_SUBSTITUTE;
						}*/

						Cond=TVar((__int64)DriveType);
					}

					break;
				}
				case MCODE_V_APANEL_ITEMCOUNT: // APanel.ItemCount
				case MCODE_V_PPANEL_ITEMCOUNT: // PPanel.ItemCount
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_ITEMCOUNT ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond=(__int64)SelPanel->GetFileCount();

					break;
				}
				case MCODE_V_APANEL_CURPOS: // APanel.CurPos
				case MCODE_V_PPANEL_CURPOS: // PPanel.CurPos
				{
					Panel *SelPanel = CheckCode == MCODE_V_APANEL_CURPOS ? ActivePanel : PassivePanel;

					if (SelPanel )
						Cond=SelPanel->GetCurrentPos()+(SelPanel->GetFileCount()>0?1:0);

					break;
				}
				case MCODE_V_TITLE: // Title
				{
					Frame *f=FrameManager->GetTopModal();

					if (f)
					{
						if (CtrlObject->Cp() == f)
						{
							ActivePanel->GetTitle(strFileName);
						}
						else
						{
							string strType;

							switch (f->GetTypeAndName(strType,strFileName))
							{
								case MODALTYPE_EDITOR:
								case MODALTYPE_VIEWER:
									f->GetTitle(strFileName);
									break;
							}
						}

						RemoveExternalSpaces(strFileName);
					}

					Cond=strFileName.CPtr();
					break;
				}
				case MCODE_V_HEIGHT:  // Height - высота текущего объекта
				case MCODE_V_WIDTH:   // Width - ширина текущего объекта
				{
					Frame *f=FrameManager->GetTopModal();

					if (f)
					{
						int X1, Y1, X2, Y2;
						f->GetPosition(X1,Y1,X2,Y2);

						if (CheckCode == MCODE_V_HEIGHT)
							Cond = (__int64)(Y2-Y1+1);
						else
							Cond = (__int64)(X2-X1+1);
					}

					break;
				}
				case MCODE_V_MENU_VALUE: // Menu.Value
				case MCODE_V_MENUINFOID: // Menu.Info.Id
				{
					int CurMMode=GetMode();
					Cond=L"";

					if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
					{
						Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

						//f=f->GetTopModal();
						while (f)
						{
							fo=f;
							f=f->GetTopModal();
						}

						if (!f)
							f=fo;

						if (f)
						{
							string NewStr;

							switch(CheckCode)
							{
								case MCODE_V_MENU_VALUE:
									if (f->VMProcess(CheckCode,&NewStr))
									{
										HiText2Str(strFileName, NewStr);
										RemoveExternalSpaces(strFileName);
										Cond=strFileName.CPtr();
									}
									break;
								case MCODE_V_MENUINFOID:
									Cond=reinterpret_cast<LPCWSTR>(static_cast<intptr_t>(f->VMProcess(CheckCode)));
									break;
							}
						}
					}

					break;
				}
				case MCODE_V_ITEMCOUNT: // ItemCount - число элементов в текущем объекте
				case MCODE_V_CURPOS: // CurPos - текущий индекс в текущем объекте
				{
					#if 1
						Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

						//f=f->GetTopModal();
						while (f)
						{
							fo=f;
							f=f->GetTopModal();
						}

						if (!f)
							f=fo;

						if (f)
						{
							Cond=f->VMProcess(CheckCode);
						}
					#else

						Frame *f=FrameManager->GetTopModal();

						if (f)
							Cond=(__int64)f->VMProcess(CheckCode);
					#endif
					break;
				}
				// *****************
				case MCODE_V_EDITORCURLINE: // Editor.CurLine - текущая линия в редакторе (в дополнении к Count)
				case MCODE_V_EDITORSTATE:   // Editor.State
				case MCODE_V_EDITORLINES:   // Editor.Lines
				case MCODE_V_EDITORCURPOS:  // Editor.CurPos
				case MCODE_V_EDITORREALPOS: // Editor.RealPos
				case MCODE_V_EDITORFILENAME: // Editor.FileName
				case MCODE_V_EDITORVALUE:   // Editor.Value
				case MCODE_V_EDITORSELVALUE: // Editor.SelValue
				{
					if (CheckCode == MCODE_V_EDITORVALUE || CheckCode == MCODE_V_EDITORSELVALUE)
						Cond=L"";

					if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
					{
						if (CheckCode == MCODE_V_EDITORFILENAME)
						{
							string strType;
							CtrlObject->Plugins->CurEditor->GetTypeAndName(strType, strFileName);
							Cond=strFileName.CPtr();
						}
						else if (CheckCode == MCODE_V_EDITORVALUE)
						{
							EditorGetString egs;
							egs.StringNumber=-1;
							CtrlObject->Plugins->CurEditor->EditorControl(ECTL_GETSTRING,&egs);
							Cond=egs.StringText;
						}
						else if (CheckCode == MCODE_V_EDITORSELVALUE)
						{
							CtrlObject->Plugins->CurEditor->VMProcess(CheckCode,&strFileName);
							Cond=strFileName.CPtr();
						}
						else
							Cond=CtrlObject->Plugins->CurEditor->VMProcess(CheckCode);
					}

					break;
				}
				case MCODE_V_HELPFILENAME:  // Help.FileName
				case MCODE_V_HELPTOPIC:     // Help.Topic
				case MCODE_V_HELPSELTOPIC:  // Help.SelTopic
				{
					Cond=L"";

					if (CtrlObject->Macro.GetMode() == MACRO_HELP)
					{
						CurFrame->VMProcess(CheckCode,&strFileName,0);
						Cond=strFileName.CPtr();
					}

					break;
				}
				case MCODE_V_VIEWERFILENAME: // Viewer.FileName
				case MCODE_V_VIEWERSTATE: // Viewer.State
				{
					if (CheckCode == MCODE_V_VIEWERFILENAME)
						Cond=L"";

					if ((CtrlObject->Macro.GetMode()==MACRO_VIEWER || CtrlObject->Macro.GetMode()==MACRO_QVIEWPANEL) &&
					        CtrlObject->Plugins->CurViewer && CtrlObject->Plugins->CurViewer->IsVisible())
					{
						if (CheckCode == MCODE_V_VIEWERFILENAME)
						{
							CtrlObject->Plugins->CurViewer->GetFileName(strFileName);//GetTypeAndName(nullptr,FileName);
							Cond=strFileName.CPtr();
						}
						else
							Cond=CtrlObject->Plugins->CurViewer->VMProcess(MCODE_V_VIEWERSTATE);
					}

					break;
				}
			}

			break;
		}
		default:
		{
			Err=1;
			break;
		}
	}

	_KEYMACRO(SysLog(L"return; Err=%d",Err));
	return Cond;
}

//HERE
static void __parseParams(int Count,TVar* Params)
{
	int stackCount=VMStack.Pop().getInteger();
	while(stackCount>Count)
	{
		VMStack.Pop();
		--stackCount;
	}
	while(stackCount<Count)
	{
		Params[--Count].SetType(vtUnknown);
	}
	for(int ii=stackCount-1;ii>=0;--ii)
	{
		Params[ii]=VMStack.Pop();
	}
}
#define parseParams(c,v) TVar v[c]; __parseParams(c,v)

/* ------------------------------------------------------------------- */
// S=trim(S[,N])
static bool trimFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	int  mode = (int) Params[1].getInteger();
	wchar_t *p = (wchar_t *)Params[0].toString();
	bool Ret=true;

	switch (mode)
	{
		case 0: p=RemoveExternalSpaces(p); break;  // alltrim
		case 1: p=RemoveLeadingSpaces(p); break;   // ltrim
		case 2: p=RemoveTrailingSpaces(p); break;  // rtrim
		default: Ret=false;
	}

	VMStack.Push(p);
	return Ret;
}

// S=substr(S,start[,length])
static bool substrFunc(const TMacroFunction*)
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
	parseParams(3,Params);
	bool Ret=false;

	int  start     = (int)Params[1].getInteger();
	wchar_t *p = (wchar_t *)Params[0].toString();
	int length_str = StrLength(p);
	int length=Params[2].isUnknown()?length_str:(int)Params[2].getInteger();

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

	if (!length)
	{
		VMStack.Push(L"");
	}
	else
	{
		p += start;
		p[length] = 0;
		Ret=true;
		VMStack.Push(p);
	}

	return Ret;
}

static BOOL SplitFileName(const wchar_t *lpFullName,string &strDest,int nFlags)
{
#define FLAG_DISK   1
#define FLAG_PATH   2
#define FLAG_NAME   4
#define FLAG_EXT    8
	const wchar_t *s = lpFullName; //start of sub-string
	const wchar_t *p = s; //current string pointer
	const wchar_t *es = s+StrLength(s); //end of string
	const wchar_t *e; //end of sub-string

	if (!*p)
		return FALSE;

	if ((*p == L'\\') && (*(p+1) == L'\\'))   //share
	{
		p += 2;
		p = wcschr(p, L'\\');

		if (!p)
			return FALSE; //invalid share (\\server\)

		p = wcschr(p+1, L'\\');

		if (!p)
			p = es;

		if ((nFlags & FLAG_DISK) == FLAG_DISK)
		{
			strDest=s;
			strDest.SetLength(p-s);
		}
	}
	else
	{
		if (*(p+1) == L':')
		{
			p += 2;

			if ((nFlags & FLAG_DISK) == FLAG_DISK)
			{
				size_t Length=strDest.GetLength()+p-s;
				strDest+=s;
				strDest.SetLength(Length);
			}
		}
	}

	e = nullptr;
	s = p;

	while (p)
	{
		p = wcschr(p, L'\\');

		if (p)
		{
			e = p;
			p++;
		}
	}

	if (e)
	{
		if ((nFlags & FLAG_PATH))
		{
			size_t Length=strDest.GetLength()+e-s;
			strDest+=s;
			strDest.SetLength(Length);
		}

		s = e+1;
		p = s;
	}

	if (!p)
		p = s;

	e = nullptr;

	while (p)
	{
		p = wcschr(p+1, L'.');

		if (p)
			e = p;
	}

	if (!e)
		e = es;

	if (!strDest.IsEmpty())
		AddEndSlash(strDest);

	if (nFlags & FLAG_NAME)
	{
		const wchar_t *ptr=wcspbrk(s,L":");

		if (ptr)
			s=ptr+1;

		size_t Length=strDest.GetLength()+e-s;
		strDest+=s;
		strDest.SetLength(Length);
	}

	if (nFlags & FLAG_EXT)
		strDest+=e;

	return TRUE;
}


// S=fsplit(S,N)
static bool fsplitFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	int m = (int)Params[1].getInteger();
	const wchar_t *s = Params[0].toString();
	bool Ret=false;
	string strPath;

	if (!SplitFileName(s,strPath,m))
		strPath.Clear();
	else
		Ret=true;

	VMStack.Push(strPath.CPtr());
	return Ret;
}

#if 0
// S=Meta("!.!") - в макросах юзаем ФАРовы метасимволы
static bool metaFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	const wchar_t *s = Params[0].toString();

	if (s && *s)
	{
		char SubstText[512];
		char Name[NM],ShortName[NM];
		xstrncpy(SubstText,s,sizeof(SubstText));
		SubstFileName(SubstText,sizeof(SubstText),Name,ShortName,nullptr,nullptr,TRUE);
		return TVar(SubstText);
	}

	return TVar(L"");
}
#endif


// N=atoi(S[,radix])
static bool atoiFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	bool Ret=true;
	wchar_t *endptr;
	VMStack.Push(TVar(_wcstoi64(Params[0].toString(),&endptr,(int)Params[1].toInteger())));
	return Ret;
}


// N=Window.Scroll(Lines[,Axis])
static bool windowscrollFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	bool Ret=false;
	TVar L=Params[0];

	if (Opt.WindowMode)
	{
		int Lines=(int)Params[0].i(), Columns=0;
		L=0;
		if (Params[1].i())
		{
			Columns=Lines;
			Lines=0;
		}

		if (Console.ScrollWindow(Lines, Columns))
		{
			Ret=true;
			L=1;
		}
	}
	else
		L=0;

	VMStack.Push(L);
	return Ret;
}

// S=itoa(N[,radix])
static bool itowFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	bool Ret=false;

	if (Params[0].isInteger())
	{
		wchar_t value[65];
		int Radix=(int)Params[1].toInteger();

		if (!Radix)
			Radix=10;

		Ret=true;
		Params[0]=TVar(_i64tow(Params[0].toInteger(),value,Radix));
	}

	VMStack.Push(Params[0]);
	return Ret;
}

// N=sleep(N)
static bool sleepFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	long Period=(long)Params[0].getInteger();

	if (Period > 0)
	{
		Sleep((DWORD)Period);
		VMStack.Push(1);
		return true;
	}

	VMStack.Push(0ll);
	return false;
}


// N=KeyBar.Show([N])
static bool keybarshowFunc(const TMacroFunction*)
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
	parseParams(1,Params);
	Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

	//f=f->GetTopModal();
	while (f)
	{
		fo=f;
		f=f->GetTopModal();
	}

	if (!f)
		f=fo;

	VMStack.Push(f?f->VMProcess(MCODE_F_KEYBAR_SHOW,nullptr,Params[0].getInteger())-1:-1);
	return f?true:false;
}


// S=key(V)
static bool keyFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	string strKeyText;

	if (Params[0].isInteger())
	{
		if (Params[0].i())
			KeyToText((int)Params[0].i(),strKeyText);
	}
	else
	{
		// Проверим...
		DWORD Key=(DWORD)KeyNameToKey(Params[0].s());

		if (Key != (DWORD)-1 && Key==(DWORD)Params[0].i())
			strKeyText=Params[0].s();
	}

	VMStack.Push(strKeyText.CPtr());
	return !strKeyText.IsEmpty()?true:false;
}

// V=waitkey([N,[T]])
static bool waitkeyFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	long Type=(long)Params[1].getInteger();
	long Period=(long)Params[0].getInteger();
	DWORD Key=WaitKey((DWORD)-1,Period);

	if (!Type)
	{
		string strKeyText;

		if (Key != KEY_NONE)
			if (!KeyToText(Key,strKeyText))
				strKeyText.Clear();

		VMStack.Push(strKeyText.CPtr());
		return !strKeyText.IsEmpty()?true:false;
	}

	if (Key == KEY_NONE)
		Key=-1;

	VMStack.Push((__int64)Key);
	return Key != (DWORD)-1;
}

// n=min(n1,n2)
static bool minFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	VMStack.Push(Params[1] < Params[0] ? Params[1] : Params[0]);
	return true;
}

// n=max(n1.n2)
static bool maxFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	VMStack.Push(Params[1] > Params[0]  ? Params[1] : Params[0]);
	return true;
}

// n=mod(n1,n2)
static bool modFunc(const TMacroFunction*)
{
	parseParams(2,Params);

	if (!Params[1].i())
	{
		_KEYMACRO(___FILEFUNCLINE___;SysLog(L"Error: Divide (mod) by zero"));
		VMStack.Push(0ll);
		return false;
	}

	VMStack.Push(Params[0] % Params[1]);
	return true;
}

// n=iif(expression,n1,n2)
static bool iifFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	VMStack.Push(__CheckCondForSkip(Params[0],MCODE_OP_JZ) ? Params[2] : Params[1]);
	return true;
}

// N=index(S1,S2[,Mode])
static bool indexFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	const wchar_t *s = Params[0].toString();
	const wchar_t *p = Params[1].toString();
	const wchar_t *i = !Params[2].getInteger() ? StrStrI(s,p) : StrStr(s,p);
	bool Ret= i ? true : false;
	VMStack.Push(TVar((__int64)(i ? i-s : -1)));
	return Ret;
}

// S=rindex(S1,S2[,Mode])
static bool rindexFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	const wchar_t *s = Params[0].toString();
	const wchar_t *p = Params[1].toString();
	const wchar_t *i = !Params[2].getInteger() ? RevStrStrI(s,p) : RevStrStr(s,p);
	bool Ret= i ? true : false;
	VMStack.Push(TVar((__int64)(i ? i-s : -1)));
	return Ret;
}

// S=Size2Str(Size,Flags[,Width])
static bool size2strFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	int Width = (int)Params[2].getInteger();

	string strDestStr;
	FileSizeToStr(strDestStr,Params[0].i(), !Width?-1:Width, Params[1].i());

	VMStack.Push(TVar(strDestStr.CPtr()));
	return true;
}

// S=date([S])
static bool dateFunc(const TMacroFunction*)
{
	parseParams(1,Params);

	if (Params[0].isInteger() && !Params[0].i())
		Params[0]=L"";

	const wchar_t *s = Params[0].toString();
	bool Ret=false;
	string strTStr;

	if (MkStrFTime(strTStr,s))
		Ret=true;
	else
		strTStr.Clear();

	VMStack.Push(TVar(strTStr.CPtr()));
	return Ret;
}

// S=xlat(S[,Flags])
/*
  Flags:
  	XLAT_SWITCHKEYBLAYOUT  = 1
	XLAT_SWITCHKEYBBEEP    = 2
	XLAT_USEKEYBLAYOUTNAME = 4
*/
static bool xlatFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	wchar_t *Str = (wchar_t *)Params[0].toString();
	bool Ret=::Xlat(Str,0,StrLength(Str),Params[1].i())?true:false;
	VMStack.Push(TVar(Str));
	return Ret;
}

// N=beep([N])
static bool beepFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	/*
		MB_ICONASTERISK = 0x00000040
			Звук Звездочка
		MB_ICONEXCLAMATION = 0x00000030
		    Звук Восклицание
		MB_ICONHAND = 0x00000010
		    Звук Критическая ошибка
		MB_ICONQUESTION = 0x00000020
		    Звук Вопрос
		MB_OK = 0x0
		    Стандартный звук
		SIMPLE_BEEP = 0xffffffff
		    Встроенный динамик
	*/
	bool Ret=MessageBeep((UINT)Params[0].i())?true:false;

	/*
		http://msdn.microsoft.com/en-us/library/dd743680%28VS.85%29.aspx
		BOOL PlaySound(
	    	LPCTSTR pszSound,
	    	HMODULE hmod,
	    	DWORD fdwSound
		);

		http://msdn.microsoft.com/en-us/library/dd798676%28VS.85%29.aspx
		BOOL sndPlaySound(
	    	LPCTSTR lpszSound,
	    	UINT fuSound
		);
	*/

	VMStack.Push(Ret?1:0);
	return Ret;
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
static bool kbdLayoutFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	DWORD dwLayout = (DWORD)Params[0].getInteger();

	BOOL Ret=TRUE;
	HKL  Layout=0, RetLayout=0;

	wchar_t LayoutName[1024]={}; // BUGBUG!!!
	if (ifn.GetConsoleKeyboardLayoutNameW(LayoutName))
	{
		wchar_t *endptr;
		DWORD res=wcstoul(LayoutName, &endptr, 16);
		RetLayout=(HKL)(intptr_t)(HIWORD(res)? res : MAKELONG(res,res));
	}

	HWND hWnd = Console.GetWindow();

	if (hWnd && dwLayout)
	{
		WPARAM wParam;

		if ((long)dwLayout == -1)
		{
			wParam=INPUTLANGCHANGE_BACKWARD;
			Layout=(HKL)HKL_PREV;
		}
		else if (dwLayout == 1)
		{
			wParam=INPUTLANGCHANGE_FORWARD;
			Layout=(HKL)HKL_NEXT;
		}
		else
		{
			wParam=0;
			Layout=(HKL)(intptr_t)(HIWORD(dwLayout)? dwLayout : MAKELONG(dwLayout,dwLayout));
		}

		Ret=PostMessage(hWnd,WM_INPUTLANGCHANGEREQUEST, wParam, (LPARAM)Layout);
	}

	VMStack.Push(Ret?TVar(static_cast<INT64>(reinterpret_cast<intptr_t>(RetLayout))):0);

	return Ret?true:false;
}

// S=prompt(["Title"[,"Prompt"[,flags[, "Src"[, "History"]]]]])
static bool promptFunc(const TMacroFunction*)
{
	parseParams(5,Params);
	TVar& ValHistory(Params[4]);
	TVar& ValSrc(Params[3]);
	DWORD Flags = (DWORD)Params[2].getInteger();
	TVar& ValPrompt(Params[1]);
	TVar& ValTitle(Params[0]);
	TVar Result(L"");
	bool Ret=false;

	const wchar_t *history=nullptr;
	const wchar_t *title=nullptr;

	if (!(ValTitle.isInteger() && !ValTitle.i()))
		title=ValTitle.s();

	if (!(ValHistory.isInteger() && !ValHistory.i()))
		history=ValHistory.s();

	const wchar_t *src=L"";

	if (!(ValSrc.isInteger() && !ValSrc.i()))
		src=ValSrc.s();

	const wchar_t *prompt=L"";

	if (!(ValPrompt.isInteger() && !ValPrompt.i()))
		prompt=ValPrompt.s();

	string strDest;

	DWORD oldHistoryDisable=CtrlObject->Macro.GetHistoryDisableMask();

	if (!(history && *history)) // Mantis#0001743: Возможность отключения истории
		CtrlObject->Macro.SetHistoryDisableMask(8); // если не указан history, то принудительно отключаем историю для ЭТОГО prompt()

	if (GetString(title,prompt,history,src,strDest,nullptr,(Flags&~FIB_CHECKBOX)|FIB_ENABLEEMPTY,nullptr,nullptr))
	{
		Result=strDest.CPtr();
		Result.toString();
		Ret=true;
	}
	else
		Result=0;

	CtrlObject->Macro.SetHistoryDisableMask(oldHistoryDisable);

	VMStack.Push(Result);
	return Ret;
}

// N=msgbox(["Title"[,"Text"[,flags]]])
static bool msgBoxFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	DWORD Flags = (DWORD)Params[2].getInteger();
	TVar& ValB(Params[1]);
	TVar& ValT(Params[0]);
	const wchar_t *title = L"";

	if (!(ValT.isInteger() && !ValT.i()))
		title=NullToEmpty(ValT.toString());

	const wchar_t *text  = L"";

	if (!(ValB.isInteger() && !ValB.i()))
		text =NullToEmpty(ValB.toString());

	Flags&=~(FMSG_KEEPBACKGROUND|FMSG_ERRORTYPE);
	Flags|=FMSG_ALLINONE;

	if (!HIWORD(Flags) || HIWORD(Flags) > HIWORD(FMSG_MB_RETRYCANCEL))
		Flags|=FMSG_MB_OK;

	//_KEYMACRO(SysLog(L"title='%s'",title));
	//_KEYMACRO(SysLog(L"text='%s'",text));
	string TempBuf = title;
	TempBuf += L"\n";
	TempBuf += text;
	int Result=pluginapi::apiMessageFn(&FarGuid,&FarGuid,Flags,nullptr,(const wchar_t * const *)TempBuf.CPtr(),0,0)+1;
	/*
	if (Result <= -1) // Break?
		CtrlObject->Macro.SendDropProcess();
	*/
	VMStack.Push((__int64)Result);
	return true;
}


static int WINAPI CompareItems(const MenuItemEx **el1, const MenuItemEx **el2, const SortItemParam *Param)
{
	if (((*el1)->Flags & LIF_SEPARATOR) || ((*el2)->Flags & LIF_SEPARATOR))
		return 0;

	string strName1((*el1)->strName);
	string strName2((*el2)->strName);
	RemoveChar(strName1,L'&',true);
	RemoveChar(strName2,L'&',true);
	int Res = NumStrCmpI(strName1.CPtr()+Param->Offset,strName2.CPtr()+Param->Offset);
	return (Param->Direction?(Res<0?1:(Res>0?-1:0)):Res);
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
static bool menushowFunc(const TMacroFunction*)
{
	parseParams(6,Params);
	TVar& VY(Params[5]);
	TVar& VX(Params[4]);
	TVar& VFindOrFilter(Params[3]);
	DWORD Flags = (DWORD)Params[2].getInteger();
	TVar& Title(Params[1]);

	if (Title.isUnknown())
		Title=L"";

	string strTitle=Title.toString();
	string strBottom;
	TVar& Items(Params[0]);
	string strItems = Items.toString();
	ReplaceStrings(strItems,L"\r\n",L"\n");

	if (!strItems.IsSubStrAt(strItems.GetLength()-1,L"\n"))
		strItems.Append(L"\n");

	TVar Result = -1;
	int BoxType = (Flags & 0x7)?(Flags & 0x7)-1:3;
	bool bResultAsIndex = (Flags & 0x08)?true:false;
	bool bMultiSelect = (Flags & 0x010)?true:false;
	bool bSorting = (Flags & 0x20)?true:false;
	bool bPacking = (Flags & 0x40)?true:false;
	bool bAutohighlight = (Flags & 0x80)?true:false;
	bool bSetMenuFilter = (Flags & 0x100)?true:false;
	bool bAutoNumbering = (Flags & 0x200)?true:false;
	bool bExitAfterNavigate = (Flags & 0x400)?true:false;
	int nLeftShift=bAutoNumbering?9:0;
	int X = -1;
	int Y = -1;
	unsigned __int64 MenuFlags = VMENU_WRAPMODE;

	if (!VX.isUnknown())
		X=VX.toInteger();

	if (!VY.isUnknown())
		Y=VY.toInteger();

	if (bAutohighlight)
		MenuFlags |= VMENU_AUTOHIGHLIGHT;

	int SelectedPos=0;
	int LineCount=0;
	size_t CurrentPos=0;
	size_t PosLF;
	size_t SubstrLen;
	ReplaceStrings(strTitle,L"\r\n",L"\n");
	bool CRFound=strTitle.Pos(PosLF, L"\n");

	if(CRFound)
	{
		strBottom=strTitle.SubStr(PosLF+1);
		strTitle=strTitle.SubStr(0,PosLF);
	}
	VMenu Menu(strTitle.CPtr(),nullptr,0,ScrY-4);
	Menu.SetBottomTitle(strBottom.CPtr());
	Menu.SetFlags(MenuFlags);
	Menu.SetPosition(X,Y,0,0);
	Menu.SetBoxType(BoxType);

	CRFound=strItems.Pos(PosLF, L"\n");
	while(CRFound)
	{
		MenuItemEx NewItem;
		NewItem.Clear();
		SubstrLen=PosLF-CurrentPos;

		if (SubstrLen==0)
			SubstrLen=1;

		NewItem.strName=strItems.SubStr(CurrentPos,SubstrLen);

		if (NewItem.strName!=L"\n")
		{
		wchar_t *CurrentChar=(wchar_t *)NewItem.strName.CPtr();
		bool bContunue=(*CurrentChar<=L'\x4');
		while(*CurrentChar && bContunue)
		{
			switch (*CurrentChar)
			{
				case L'\x1':
					NewItem.Flags|=LIF_SEPARATOR;
					CurrentChar++;
					break;

				case L'\x2':
					NewItem.Flags|=LIF_CHECKED;
					CurrentChar++;
					break;

				case L'\x3':
					NewItem.Flags|=LIF_DISABLE;
					CurrentChar++;
					break;

				case L'\x4':
					NewItem.Flags|=LIF_GRAYED;
					CurrentChar++;
					break;

				default:
				bContunue=false;
				CurrentChar++;
				break;
			}
		}
		NewItem.strName=CurrentChar;
		}
		else
			NewItem.strName.Clear();

		if (bAutoNumbering && !(bSorting || bPacking) && !(NewItem.Flags & LIF_SEPARATOR))
		{
			LineCount++;
			NewItem.strName.Format(L"%6d - %s", LineCount, NewItem.strName.CPtr());
		}
		Menu.AddItem(&NewItem);
		CurrentPos=PosLF+1;
		CRFound=strItems.Pos(PosLF, L"\n",CurrentPos);
	}

	if (bSorting)
		Menu.SortItems(reinterpret_cast<TMENUITEMEXCMPFUNC>(CompareItems));

	if (bPacking)
		Menu.Pack();

	if ((bAutoNumbering) && (bSorting || bPacking))
	{
		for (int i = 0; i < Menu.GetShowItemCount(); i++)
		{
			MenuItemEx *Item=Menu.GetItemPtr(i);
			if (!(Item->Flags & LIF_SEPARATOR))
			{
				LineCount++;
				Item->strName.Format(L"%6d - %s", LineCount, Item->strName.CPtr());
			}
		}
	}

	if (!VFindOrFilter.isUnknown())
	{
		if (bSetMenuFilter)
		{
			Menu.SetFilterEnabled(true);
			Menu.SetFilterString(VFindOrFilter.toString());
			Menu.FilterStringUpdated(true);
			Menu.Show();
		}
		else
		{
			if (VFindOrFilter.isInteger())
			{
				if (VFindOrFilter.toInteger()-1>=0)
					Menu.SetSelectPos(VFindOrFilter.toInteger()-1,1);
				else
					Menu.SetSelectPos(Menu.GetItemCount()+VFindOrFilter.toInteger(),1);
			}
			else
				if (VFindOrFilter.isString())
					Menu.SetSelectPos(Max(0,Menu.FindItem(0, VFindOrFilter.toString())),1);
		}
	}

	Frame *frame;

	if ((frame=FrameManager->GetBottomFrame()) )
		frame->Lock();

	Menu.Show();
	int PrevSelectedPos=Menu.GetSelectPos();
	DWORD Key=0;
	int RealPos;
	bool CheckFlag;
	int X1, Y1, X2, Y2, NewY2;
	while (!Menu.Done() && !CloseFARMenu)
	{
		SelectedPos=Menu.GetSelectPos();
		Key=Menu.ReadInput();
		switch (Key)
		{
			case KEY_NUMPAD0:
			case KEY_INS:
				if (bMultiSelect)
				{
					Menu.SetCheck(!Menu.GetCheck(SelectedPos));
					Menu.Show();
				}
				break;

			case KEY_CTRLADD:
			case KEY_CTRLSUBTRACT:
			case KEY_CTRLMULTIPLY:
			case KEY_RCTRLADD:
			case KEY_RCTRLSUBTRACT:
			case KEY_RCTRLMULTIPLY:
				if (bMultiSelect)
				{
					for(int i=0; i<Menu.GetShowItemCount(); i++)
					{
						RealPos=Menu.VisualPosToReal(i);
						if (Key==KEY_CTRLMULTIPLY || Key==KEY_RCTRLMULTIPLY)
						{
							CheckFlag=Menu.GetCheck(RealPos)?false:true;
						}
						else
						{
							CheckFlag=(Key==KEY_CTRLADD || Key==KEY_RCTRLADD);
						}
						Menu.SetCheck(CheckFlag, RealPos);
					}
					Menu.Show();
				}
				break;

			case KEY_CTRLA:
			case KEY_RCTRLA:
			{
				Menu.GetPosition(X1, Y1, X2, Y2);
				NewY2=Y1+Menu.GetShowItemCount()+1;

				if (NewY2>ScrY-2)
					NewY2=ScrY-2;

				Menu.SetPosition(X1,Y1,X2,NewY2);
				Menu.Show();
				break;
			}

			case KEY_BREAK:
				CtrlObject->Macro.SendDropProcess();
				Menu.SetExitCode(-1);
				break;

			default:
				Menu.ProcessInput();
				break;
		}

		if (bExitAfterNavigate && (PrevSelectedPos!=SelectedPos))
		{
			SelectedPos=Menu.GetSelectPos();
			break;
		}

		PrevSelectedPos=SelectedPos;
	}

	wchar_t temp[65];

	if (Menu.Modal::GetExitCode() >= 0)
	{
		SelectedPos=Menu.GetExitCode();
		if (bMultiSelect)
		{
			Result=L"";
			for(int i=0; i<Menu.GetItemCount(); i++)
			{
				if (Menu.GetCheck(i))
				{
					if (bResultAsIndex)
					{
						_i64tow(i+1,temp,10);
						Result+=temp;
					}
					else
						Result+=(*Menu.GetItemPtr(i)).strName.CPtr()+nLeftShift;
					Result+=L"\n";
				}
			}
			if(Result==L"")
			{
				if (bResultAsIndex)
				{
					_i64tow(SelectedPos+1,temp,10);
					Result=temp;
				}
				else
					Result=(*Menu.GetItemPtr(SelectedPos)).strName.CPtr()+nLeftShift;
			}
		}
		else
			if(!bResultAsIndex)
				Result=(*Menu.GetItemPtr(SelectedPos)).strName.CPtr()+nLeftShift;
			else
				Result=SelectedPos+1;
		Menu.Hide();
	}
	else
	{
		Menu.Hide();
		if (bExitAfterNavigate)
		{
			Result=SelectedPos+1;
			if ((Key == KEY_ESC) || (Key == KEY_F10) || (Key == KEY_BREAK))
				Result=-Result;
		}
		else
		{
			if(bResultAsIndex)
				Result=0;
			else
				Result=L"";
		}
	}

	if (frame )
		frame->Unlock();

	VMStack.Push(Result);
	return true;
}

// S=Env(S[,Mode[,Value]])
static bool environFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	TVar& Value(Params[2]);
	TVar& Mode(Params[1]);
	TVar& S(Params[0]);
	bool Ret=false;
	string strEnv;


	if (apiGetEnvironmentVariable(S.toString(), strEnv))
		Ret=true;
	else
		strEnv.Clear();

	if (Mode.i()) // Mode != 0: Set
	{
		SetEnvironmentVariable(S.toString(),Value.isUnknown() || !*Value.s()?nullptr:Value.toString());
	}

	VMStack.Push(strEnv.CPtr());
	return Ret;
}

// V=Panel.Select(panelType,Action[,Mode[,Items]])
static bool panelselectFunc(const TMacroFunction*)
{
	parseParams(4,Params);
	TVar& ValItems(Params[3]);
	int Mode=(int)Params[2].getInteger();
	DWORD Action=(int)Params[1].getInteger();
	int typePanel=(int)Params[0].getInteger();
	__int64 Result=-1;

	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	Panel *SelPanel = !typePanel ? ActivePanel : (typePanel == 1?PassivePanel:nullptr);

	if (SelPanel)
	{
		__int64 Index=-1;
		if (Mode == 1)
		{
			Index=ValItems.getInteger();
			if (!Index)
				Index=SelPanel->GetCurrentPos();
			else
				Index--;
		}

		if (Mode == 2 || Mode == 3)
		{
			string strStr=ValItems.s();
			ReplaceStrings(strStr,L"\r",L"\n");
			ReplaceStrings(strStr,L"\n\n",L"\n");
			ValItems=strStr.CPtr();
		}

		MacroPanelSelect mps;
		mps.Action      = Action & 0xF;
		mps.ActionFlags = (Action & (~0xF)) >> 4;
		mps.Mode        = Mode;
		mps.Index       = Index;
		mps.Item        = &ValItems;
		Result=SelPanel->VMProcess(MCODE_F_PANEL_SELECT,&mps,0);
	}

	VMStack.Push(Result);
	return Result==-1?false:true;
}

static bool _fattrFunc(int Type)
{
	bool Ret=false;
	DWORD FileAttr=INVALID_FILE_ATTRIBUTES;
	long Pos=-1;

	if (!Type || Type == 2) // не панели: fattr(0) & fexist(2)
	{
		parseParams(1,Params);
		TVar& Str(Params[0]);
		FAR_FIND_DATA_EX FindData;
		apiGetFindDataEx(Str.toString(), FindData);
		FileAttr=FindData.dwFileAttributes;
		Ret=true;
	}
	else // panel.fattr(1) & panel.fexist(3)
	{
		parseParams(2,Params);
		TVar& S(Params[1]);
		int typePanel=(int)Params[0].getInteger();
		const wchar_t *Str = S.toString();
		Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
		Panel *PassivePanel=nullptr;

		if (ActivePanel)
			PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

		//Frame* CurFrame=FrameManager->GetCurrentFrame();
		Panel *SelPanel = !typePanel ? ActivePanel : (typePanel == 1?PassivePanel:nullptr);

		if (SelPanel)
		{
			if (wcspbrk(Str,L"*?") )
				Pos=SelPanel->FindFirst(Str);
			else
				Pos=SelPanel->FindFile(Str,wcspbrk(Str,L"\\/:")?FALSE:TRUE);

			if (Pos >= 0)
			{
				string strFileName;
				SelPanel->GetFileName(strFileName,Pos,FileAttr);
				Ret=true;
			}
		}
	}

	if (Type == 2) // fexist(2)
		FileAttr=(FileAttr!=INVALID_FILE_ATTRIBUTES)?1:0;
	else if (Type == 3) // panel.fexist(3)
		FileAttr=(DWORD)Pos+1;

	VMStack.Push(TVar((__int64)(long)FileAttr));
	return Ret;
}

// N=fattr(S)
static bool fattrFunc(const TMacroFunction*)
{
	return _fattrFunc(0);
}

// N=fexist(S)
static bool fexistFunc(const TMacroFunction*)
{
	return _fattrFunc(2);
}

// N=panel.fattr(S)
static bool panelfattrFunc(const TMacroFunction*)
{
	return _fattrFunc(1);
}

// N=panel.fexist(S)
static bool panelfexistFunc(const TMacroFunction*)
{
	return _fattrFunc(3);
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
static bool flockFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(-1);
	int stateFLock=(int)Params[1].getInteger();
	UINT vkKey=(UINT)Params[0].getInteger();

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
		Ret=(__int64)SetFLockState(vkKey,stateFLock);

	VMStack.Push(Ret);
	return Ret.i()!=-1;
}

// N=Dlg.SetFocus([ID])
static bool dlgsetfocusFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar Ret(-1);
	unsigned Index=(unsigned)Params[0].getInteger()-1;
	Frame* CurFrame=FrameManager->GetCurrentFrame();

	if (CtrlObject->Macro.GetMode()==MACRO_DIALOG && CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG)
	{
		Ret=(__int64)CurFrame->VMProcess(MCODE_V_DLGCURPOS);
		if ((int)Index >= 0)
		{
			if(!SendDlgMessage((HANDLE)CurFrame,DM_SETFOCUS,Index,0))
				Ret=0;
		}
	}

	VMStack.Push(Ret);
	return Ret.i()!=-1; // ?? <= 0 ??
}

// V=Far.Cfg.Get(Key,Name)
bool farcfggetFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar& Name(Params[1]);
	TVar& Key(Params[0]);

	string strValue;
	bool resultGetCfg = GetConfigValue(Key.s(),Name.s(), strValue);
	KeyMacro::SetMacroConst(constFarCfgErr,resultGetCfg?0:1);
	VMStack.Push(resultGetCfg?strValue.CPtr():L"");

	return resultGetCfg;
}

// V=Dlg.GetValue([Pos[,InfoID]])
static bool dlggetvalueFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(-1);
	Frame* CurFrame=FrameManager->GetCurrentFrame();

	if (CtrlObject->Macro.GetMode()==MACRO_DIALOG && CurFrame && CurFrame->GetType()==MODALTYPE_DIALOG)
	{
		TVarType typeIndex=Params[0].type();
		unsigned Index=(unsigned)Params[0].getInteger()-1;
		if (typeIndex == vtUnknown || (typeIndex == vtInteger && (int)Index < -1))
			Index=((Dialog*)CurFrame)->GetDlgFocusPos();

		TVarType typeInfoID=Params[1].type();
		int InfoID=(int)Params[1].getInteger();
		if (typeInfoID == vtUnknown || (typeInfoID == vtInteger && InfoID < 0))
			InfoID=0;

		FarGetValue fgv={InfoID,FMVT_UNKNOWN};
		unsigned DlgItemCount=((Dialog*)CurFrame)->GetAllItemCount();
		const DialogItemEx **DlgItem=((Dialog*)CurFrame)->GetAllItem();
		bool CallDialog=true;

		if (Index == (unsigned)-1)
		{
			SMALL_RECT Rect;

			if (SendDlgMessage((HANDLE)CurFrame,DM_GETDLGRECT,0,&Rect))
			{
				switch (InfoID)
				{
					case 0: Ret=(__int64)DlgItemCount; break;
					case 2: Ret=(__int64)Rect.Left; break;
					case 3: Ret=(__int64)Rect.Top; break;
					case 4: Ret=(__int64)Rect.Right; break;
					case 5: Ret=(__int64)Rect.Bottom; break;
					case 6: Ret=(__int64)(((Dialog*)CurFrame)->GetDlgFocusPos()+1); break;
					default: Ret=0; Ret.SetType(vtUnknown); break;
				}
			}
		}
		else if (Index < DlgItemCount && DlgItem)
		{
			const DialogItemEx *Item=DlgItem[Index];
			FARDIALOGITEMTYPES ItemType=Item->Type;
			FARDIALOGITEMFLAGS ItemFlags=Item->Flags;

			if (!InfoID)
			{
				if (ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
				{
					InfoID=7;
				}
				else if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
				{
					FarListGetItem ListItem={sizeof(FarListGetItem)};
					ListItem.ItemIndex=Item->ListPtr->GetSelectPos();

					if (SendDlgMessage(CurFrame,DM_LISTGETITEM,Index,&ListItem))
					{
						Ret=ListItem.Item.Text;
					}
					else
					{
						Ret=L"";
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
				case 1: Ret=(__int64)ItemType;    break;
				case 2: Ret=(__int64)Item->X1;    break;
				case 3: Ret=(__int64)Item->Y1;    break;
				case 4: Ret=(__int64)Item->X2;    break;
				case 5: Ret=(__int64)Item->Y2;    break;
				case 6: Ret=(__int64)((Item->Flags&DIF_FOCUS)!=0); break;
				case 7:
				{
					if (ItemType == DI_CHECKBOX || ItemType == DI_RADIOBUTTON)
					{
						Ret=(__int64)Item->Selected;
					}
					else if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
					{
						Ret=(__int64)(Item->ListPtr->GetSelectPos()+1);
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
				case 8: Ret=(__int64)ItemFlags; break;
				case 9: Ret=(__int64)((Item->Flags&DIF_DEFAULTBUTTON)!=0); break;
				case 10:
				{
					Ret=Item->strData.CPtr();

					if (IsEdit(ItemType))
					{
						DlgEdit *EditPtr;

						if ((EditPtr = (DlgEdit *)(Item->ObjPtr)) )
							Ret=EditPtr->GetStringAddr();
					}

					break;
				}
				case 11:
				{
					if (ItemType == DI_COMBOBOX || ItemType == DI_LISTBOX)
					{
						Ret=(__int64)(Item->ListPtr->GetItemCount());
					}
					break;
				}
			}
		}
		else if (Index >= DlgItemCount)
		{
			Ret=(__int64)InfoID;
		}
		else
			CallDialog=false;

		if (CallDialog)
		{
			fgv.Value.Type=(FARMACROVARTYPE)Ret.type();
			switch (Ret.type())
			{
				case vtUnknown:
				case vtInteger:
					fgv.Value.Integer=Ret.i();
					break;
				case vtString:
					fgv.Value.String=Ret.s();
					break;
				case vtDouble:
					fgv.Value.Double=Ret.d();
					break;
			}

			if (SendDlgMessage((HANDLE)CurFrame,DN_GETVALUE,Index,&fgv))
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
						Ret=-1;
						break;
				}
			}
		}
	}

	VMStack.Push(Ret);
	return Ret.i()!=-1;
}

// N=Editor.Pos(Op,What[,Where])
// Op: 0 - get, 1 - set
static bool editorposFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	TVar Ret(-1);
	int Where = (int)Params[2].getInteger();
	int What  = (int)Params[1].getInteger();
	int Op    = (int)Params[0].getInteger();

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		EditorInfo ei;
		CtrlObject->Plugins->CurEditor->EditorControl(ECTL_GETINFO,&ei);

		switch (Op)
		{
			case 0: // get
			{
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
			}
			case 1: // set
			{
				EditorSetPosition esp;
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
						break;
					}
					case 6: // Overtype
						esp.Overtype=Where;
						break;
				}

				int Result=CtrlObject->Plugins->CurEditor->EditorControl(ECTL_SETPOSITION,&esp);

				if (Result)
					CtrlObject->Plugins->CurEditor->EditorControl(ECTL_REDRAW,nullptr);

				Ret=Result;
				break;
			}
		}
	}

	VMStack.Push(Ret);
	return Ret.i() != -1;
}

// OldVar=Editor.Set(Idx,Value)
static bool editorsetFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(-1);
	TVar& Value(Params[1]);
	int Index=(int)Params[0].getInteger();

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		long longState=-1L;

		if (Index != 12)
			longState=(long)Value.toInteger();
		else
		{
			if (Value.isString() || Value.i() != -1)
				longState=0;
		}

		EditorOptions EdOpt;
		CtrlObject->Plugins->CurEditor->GetEditorOptions(EdOpt);

		switch (Index)
		{
			case 0:  // TabSize;
				Ret=(__int64)EdOpt.TabSize; break;
			case 1:  // ExpandTabs;
				Ret=(__int64)EdOpt.ExpandTabs; break;
			case 2:  // PersistentBlocks;
				Ret=(__int64)EdOpt.PersistentBlocks; break;
			case 3:  // DelRemovesBlocks;
				Ret=(__int64)EdOpt.DelRemovesBlocks; break;
			case 4:  // AutoIndent;
				Ret=(__int64)EdOpt.AutoIndent; break;
			case 5:  // AutoDetectCodePage;
				Ret=(__int64)EdOpt.AutoDetectCodePage; break;
			case 6:  // AnsiCodePageForNewFile;
				Ret=(__int64)EdOpt.AnsiCodePageForNewFile; break;
			case 7:  // CursorBeyondEOL;
				Ret=(__int64)EdOpt.CursorBeyondEOL; break;
			case 8:  // BSLikeDel;
				Ret=(__int64)EdOpt.BSLikeDel; break;
			case 9:  // CharCodeBase;
				Ret=(__int64)EdOpt.CharCodeBase; break;
			case 10: // SavePos;
				Ret=(__int64)EdOpt.SavePos; break;
			case 11: // SaveShortPos;
				Ret=(__int64)EdOpt.SaveShortPos; break;
			case 12: // char WordDiv[256];
				Ret=TVar(EdOpt.strWordDiv); break;
			case 13: // F7Rules;
				Ret=(__int64)EdOpt.F7Rules; break;
			case 14: // AllowEmptySpaceAfterEof;
				Ret=(__int64)EdOpt.AllowEmptySpaceAfterEof; break;
			case 15: // ShowScrollBar;
				Ret=(__int64)EdOpt.ShowScrollBar; break;
			case 16: // EditOpenedForWrite;
				Ret=(__int64)EdOpt.EditOpenedForWrite; break;
			case 17: // SearchSelFound;
				Ret=(__int64)EdOpt.SearchSelFound; break;
			case 18: // SearchRegexp;
				Ret=(__int64)EdOpt.SearchRegexp; break;
			case 19: // SearchPickUpWord;
				Ret=(__int64)EdOpt.SearchPickUpWord; break;
			case 20: // ShowWhiteSpace;
				Ret=static_cast<INT64>(EdOpt.ShowWhiteSpace); break;
			default:
				Ret=(__int64)-1L;
		}

		if (longState != -1)
		{
			switch (Index)
			{
				case 0:  // TabSize;
					EdOpt.TabSize=longState; break;
				case 1:  // ExpandTabs;
					EdOpt.ExpandTabs=longState; break;
				case 2:  // PersistentBlocks;
					EdOpt.PersistentBlocks=longState != 0; break;
				case 3:  // DelRemovesBlocks;
					EdOpt.DelRemovesBlocks=longState != 0; break;
				case 4:  // AutoIndent;
					EdOpt.AutoIndent=longState != 0; break;
				case 5:  // AutoDetectCodePage;
					EdOpt.AutoDetectCodePage=longState != 0; break;
				case 6:  // AnsiCodePageForNewFile;
					EdOpt.AnsiCodePageForNewFile=longState != 0; break;
				case 7:  // CursorBeyondEOL;
					EdOpt.CursorBeyondEOL=longState != 0; break;
				case 8:  // BSLikeDel;
					EdOpt.BSLikeDel=longState != 0; break;
				case 9:  // CharCodeBase;
					EdOpt.CharCodeBase=longState; break;
				case 10: // SavePos;
					EdOpt.SavePos=longState; break;
				case 11: // SaveShortPos;
					EdOpt.SaveShortPos=longState; break;
				case 12: // char WordDiv[256];
					EdOpt.strWordDiv = Value.toString(); break;
				case 13: // F7Rules;
					EdOpt.F7Rules=longState != 0; break;
				case 14: // AllowEmptySpaceAfterEof;
					EdOpt.AllowEmptySpaceAfterEof=longState != 0; break;
				case 15: // ShowScrollBar;
					EdOpt.ShowScrollBar=longState != 0; break;
				case 16: // EditOpenedForWrite;
					EdOpt.EditOpenedForWrite=longState != 0; break;
				case 17: // SearchSelFound;
					EdOpt.SearchSelFound=longState != 0; break;
				case 18: // SearchRegexp;
					EdOpt.SearchRegexp=longState != 0; break;
				case 19: // SearchPickUpWord;
					EdOpt.SearchPickUpWord=longState != 0; break;
				case 20: // ShowWhiteSpace;
					EdOpt.ShowWhiteSpace=longState; break;
				default:
					Ret=-1;
					break;
			}

			CtrlObject->Plugins->CurEditor->SetEditorOptions(EdOpt);
			CtrlObject->Plugins->CurEditor->ShowStatus();
			if (Index == 0 || Index == 12 || Index == 14 || Index == 15 || Index == 20)
				CtrlObject->Plugins->CurEditor->Show();
		}
	}

	VMStack.Push(Ret);
	return Ret.i()==-1;
}

// b=mload(var)
bool mloadFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& Val(Params[0]);
	TVar TempVar;
	const wchar_t *Name=Val.s();

	if (!Name || *Name!= L'%')
	{
		VMStack.Push(0ll);
		return false;
	}

	__int64 Ret=CtrlObject->Macro.LoadVarFromDB(Name, TempVar);

	if(Ret)
		varInsert(glbVarTable, Name+1)->value = TempVar.s();

	VMStack.Push(Ret);
	return Ret != 0;
}

// b=msave(var)
bool msaveFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& Val(Params[0]);
	TVarTable *t = &glbVarTable;
	const wchar_t *Name=Val.s();

	if (!Name || *Name!= L'%')
	{
		VMStack.Push(0ll);
		return false;
	}

	TVarSet *tmpVarSet=varLook(*t, Name+1);

	if (!tmpVarSet)
	{
		VMStack.Push(0ll);
		return false;
	}

	string strValueName = Val.s();

	__int64 Ret=CtrlObject->Macro.SaveVarToDB(strValueName, tmpVarSet->value);

	VMStack.Push(Ret);
	return Ret != 0;
}

// V=Clip(N[,V])
static bool clipFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar& Val(Params[1]);
	int cmdType=(int)Params[0].getInteger();

	// принудительно второй параметр ставим AS string
	if (cmdType != 5 && Val.isInteger() && !Val.i())
	{
		Val=L"";
		Val.toString();
	}

	int Ret=0;

	switch (cmdType)
	{
		case 0: // Get from Clipboard, "S" - ignore
		{
			wchar_t *ClipText=PasteFromClipboard();

			if (ClipText)
			{
				TVar varClip(ClipText);
				xf_free(ClipText);
				VMStack.Push(varClip);
				return true;
			}

			break;
		}
		case 1: // Put "S" into Clipboard
		{
			if (!*Val.s())
			{
				Clipboard clip;
				if (clip.Open())
				{
					Ret=1;
					clip.Empty();
				}
				clip.Close();
			}
			else
			{
				Ret=CopyToClipboard(Val.s());
			}

			VMStack.Push(TVar((__int64)Ret)); // 0!  ???
			return Ret?true:false;
		}
		case 2: // Add "S" into Clipboard
		{
			TVar varClip(Val.s());
			Clipboard clip;

			Ret=FALSE;

			if (clip.Open())
			{
				wchar_t *CopyData=clip.Paste();

				if (CopyData)
				{
					size_t DataSize=StrLength(CopyData);
					wchar_t *NewPtr=(wchar_t *)xf_realloc(CopyData,(DataSize+StrLength(Val.s())+2)*sizeof(wchar_t));

					if (NewPtr)
					{
						CopyData=NewPtr;
						wcscpy(CopyData+DataSize,Val.s());
						varClip=CopyData;
						xf_free(CopyData);
					}
					else
					{
						xf_free(CopyData);
					}
				}

				Ret=clip.Copy(varClip.s());

				clip.Close();
			}

			VMStack.Push(TVar((__int64)Ret)); // 0!  ???
			return Ret?true:false;
		}
		case 3: // Copy Win to internal, "S" - ignore
		case 4: // Copy internal to Win, "S" - ignore
		{
			Clipboard clip;

			Ret=FALSE;

			if (clip.Open())
			{
				Ret=clip.InternalCopy((cmdType-3)?true:false)?1:0;
				clip.Close();
			}

			VMStack.Push(TVar((__int64)Ret)); // 0!  ???
			return Ret?true:false;
		}
		case 5: // ClipMode
		{
			// 0 - flip, 1 - виндовый буфер, 2 - внутренний, -1 - что сейчас?
			int Action=(int)Val.getInteger();
			bool mode=Clipboard::GetUseInternalClipboardState();
			if (Action >= 0)
			{
				switch (Action)
				{
					case 0: mode=!mode; break;
					case 1: mode=false; break;
					case 2: mode=true;  break;
				}
				mode=Clipboard::SetUseInternalClipboardState(mode);
			}
			VMStack.Push((__int64)(mode?2:1)); // 0!  ???
			return Ret?true:false;
		}
	}

	return Ret?true:false;
}


// N=Panel.SetPosIdx(panelType,Idx[,InSelection])
/*
*/
static bool panelsetposidxFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	int InSelection=(int)Params[2].getInteger();
	long idxItem=(long)Params[1].getInteger();
	int typePanel=(int)Params[0].getInteger();
	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	//Frame* CurFrame=FrameManager->GetCurrentFrame();
	Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;
	__int64 Ret=0;

	if (SelPanel)
	{
		int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL

		if (TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL)
		{
			long EndPos=SelPanel->GetFileCount();
			long I;
			long idxFoundItem=0;

			if (idxItem) // < 0 || > 0
			{
				EndPos--;
				if ( EndPos > 0 )
				{
					long StartPos;
					long Direct=idxItem < 0?-1:1;

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

					for ( I=StartPos ; ; I+=Direct )
					{
						if (Direct > 0)
						{
							if(I > EndPos)
								break;
						}
						else
						{
							if(I < EndPos)
								break;
						}

						if ( (!InSelection || (InSelection && SelPanel->IsSelected(I))) && SelPanel->FileInFilter(I) )
						{
							if (idxFoundItem == idxItem)
							{
								idxItem=I;
								if (SelPanel->FilterIsEnabled())
									idxItem--;
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
						//SelPanel->Show();
						// <Mantis#0000289> - грозно, но со вкусом :-)
						//ShellUpdatePanels(SelPanel);
						SelPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
						FrameManager->RefreshFrame(FrameManager->GetTopModal());
						// </Mantis#0000289>

						if ( !InSelection )
							Ret=(__int64)(SelPanel->GetCurrentPos()+1);
						else
							Ret=(__int64)(idxFoundItem+1);
					}
				}
			}
			else // = 0 - вернем текущую позицию
			{
				if ( !InSelection )
					Ret=(__int64)(SelPanel->GetCurrentPos()+1);
				else
				{
					long CurPos=SelPanel->GetCurrentPos();
					for ( I=0 ; I < EndPos ; I++ )
					{
						if ( SelPanel->IsSelected(I) && SelPanel->FileInFilter(I) )
						{
							if (I == CurPos)
							{
								Ret=(__int64)(idxFoundItem+1);
								break;
							}
							idxFoundItem++;
						}
					}
				}
			}
		}
	}

	VMStack.Push(Ret);
	return Ret?true:false;
}

// N=panel.SetPath(panelType,pathName[,fileName])
static bool panelsetpathFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	TVar& ValFileName(Params[2]);
	TVar& Val(Params[1]);
	int typePanel=(int)Params[0].getInteger();
	__int64 Ret=0;

	if (!(Val.isInteger() && !Val.i()))
	{
		const wchar_t *pathName=Val.s();
		const wchar_t *fileName=L"";

		if (!ValFileName.isInteger())
			fileName=ValFileName.s();

		Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
		Panel *PassivePanel=nullptr;

		if (ActivePanel)
			PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

		//Frame* CurFrame=FrameManager->GetCurrentFrame();
		Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;

		if (SelPanel)
		{
			if (SelPanel->SetCurDir(pathName,SelPanel->GetMode()==PLUGIN_PANEL && IsAbsolutePath(pathName),FrameManager->GetCurrentFrame()->GetType() == MODALTYPE_PANELS))
			{
				ActivePanel=CtrlObject->Cp()->ActivePanel;
				PassivePanel=ActivePanel?CtrlObject->Cp()->GetAnotherPanel(ActivePanel):nullptr;
				SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;

				//восстановим текущую папку из активной панели.
				if (ActivePanel)
					ActivePanel->SetCurPath();
				// Need PointToName()?
				if (SelPanel)
				{
					SelPanel->GoToFile(fileName); // здесь без проверки, т.к. параметр fileName аля опциональный
					//SelPanel->Show();
					// <Mantis#0000289> - грозно, но со вкусом :-)
					//ShellUpdatePanels(SelPanel);
					SelPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
				}
				FrameManager->RefreshFrame(FrameManager->GetTopModal());
				// </Mantis#0000289>
				Ret=1;
			}
		}
	}

	VMStack.Push(Ret);
	return Ret?true:false;
}

// N=Panel.SetPos(panelType,fileName)
static bool panelsetposFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar& Val(Params[1]);
	int typePanel=(int)Params[0].getInteger();
	const wchar_t *fileName=Val.s();

	if (!fileName || !*fileName)
		fileName=L"";

	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	//Frame* CurFrame=FrameManager->GetCurrentFrame();
	Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;
	__int64 Ret=0;

	if (SelPanel)
	{
		int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL

		if (TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL)
		{
			// Need PointToName()?
			if (SelPanel->GoToFile(fileName))
			{
				//SelPanel->Show();
				// <Mantis#0000289> - грозно, но со вкусом :-)
				//ShellUpdatePanels(SelPanel);
				SelPanel->UpdateIfChanged(UIC_UPDATE_NORMAL);
				FrameManager->RefreshFrame(FrameManager->GetTopModal());
				// </Mantis#0000289>
				Ret=(__int64)(SelPanel->GetCurrentPos()+1);
			}
		}
	}

	VMStack.Push(Ret);
	return Ret?true:false;
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
static bool replaceFunc(const TMacroFunction*)
{
	parseParams(5,Params);
	int Mode=(int)Params[4].getInteger();
	TVar& Count(Params[3]);
	TVar& Repl(Params[2]);
	TVar& Find(Params[1]);
	TVar& Src(Params[0]);
	__int64 Ret=1;
	// TODO: Здесь нужно проверить в соответствии с УНИХОДОМ!
	string strStr;
	//int lenS=(int)StrLength(Src.s());
	int lenF=(int)StrLength(Find.s());
	//int lenR=(int)StrLength(Repl.s());
	int cnt=0;

	if( lenF )
	{
		const wchar_t *Ptr=Src.s();
		if( !Mode )
		{
			while ((Ptr=StrStrI(Ptr,Find.s())) )
			{
				cnt++;
				Ptr+=lenF;
			}
		}
		else
		{
			while ((Ptr=StrStr(Ptr,Find.s())) )
			{
				cnt++;
				Ptr+=lenF;
			}
		}
	}

	if (cnt)
	{
		//if (lenR > lenF)
		//	lenS+=cnt*(lenR-lenF+1); //???

		strStr=Src.s();
		cnt=(int)Count.i();

		if (cnt <= 0)
			cnt=-1;

		ReplaceStrings(strStr,Find.s(),Repl.s(),cnt,!Mode);
		VMStack.Push(strStr.CPtr());
	}
	else
		VMStack.Push(Src);

	return Ret?true:false;
}

// V=Panel.Item(typePanel,Index,TypeInfo)
static bool panelitemFunc(const TMacroFunction*)
{
	parseParams(3,Params);
	TVar& P2(Params[2]);
	TVar& P1(Params[1]);
	int typePanel=(int)Params[0].getInteger();
	TVar Ret(0ll);
	Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
	Panel *PassivePanel=nullptr;

	if (ActivePanel)
		PassivePanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);

	//Frame* CurFrame=FrameManager->GetCurrentFrame();
	Panel *SelPanel = typePanel? (typePanel == 1?PassivePanel:nullptr):ActivePanel;

	if (!SelPanel)
	{
		VMStack.Push(Ret);
		return false;
	}

	int TypePanel=SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL

	if (!(TypePanel == FILE_PANEL || TypePanel ==TREE_PANEL))
	{
		VMStack.Push(Ret);
		return false;
	}

	int Index=(int)(P1.toInteger())-1;
	int TypeInfo=(int)P2.toInteger();
	FileListItem filelistItem;

	if (TypePanel == TREE_PANEL)
	{
		TreeItem treeItem;

		if (SelPanel->GetItem(Index,&treeItem) && !TypeInfo)
		{
			VMStack.Push(TVar(treeItem.strName));
			return true;
		}
	}
	else
	{
		string strDate, strTime;

		if (TypeInfo == 11)
			SelPanel->ReadDiz();

		if (!SelPanel->GetItem(Index,&filelistItem))
			TypeInfo=-1;

		switch (TypeInfo)
		{
			case 0:  // Name
				Ret=TVar(filelistItem.strName);
				break;
			case 1:  // ShortName
				Ret=TVar(filelistItem.strShortName);
				break;
			case 2:  // FileAttr
				Ret=TVar((__int64)(long)filelistItem.FileAttr);
				break;
			case 3:  // CreationTime
				ConvertDate(filelistItem.CreationTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate.CPtr());
				break;
			case 4:  // AccessTime
				ConvertDate(filelistItem.AccessTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate.CPtr());
				break;
			case 5:  // WriteTime
				ConvertDate(filelistItem.WriteTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate.CPtr());
				break;
			case 6:  // FileSize
				Ret=TVar((__int64)filelistItem.FileSize);
				break;
			case 7:  // AllocationSize
				Ret=TVar((__int64)filelistItem.AllocationSize);
				break;
			case 8:  // Selected
				Ret=TVar((__int64)((DWORD)filelistItem.Selected));
				break;
			case 9:  // NumberOfLinks
				Ret=TVar((__int64)filelistItem.NumberOfLinks);
				break;
			case 10:  // SortGroup
				Ret=TVar((__int64)filelistItem.SortGroup);
				break;
			case 11:  // DizText
				Ret=TVar((const wchar_t *)filelistItem.DizText);
				break;
			case 12:  // Owner
				Ret=TVar(filelistItem.strOwner);
				break;
			case 13:  // CRC32
				Ret=TVar((__int64)filelistItem.CRC32);
				break;
			case 14:  // Position
				Ret=TVar((__int64)filelistItem.Position);
				break;
			case 15:  // CreationTime (FILETIME)
				Ret=TVar((__int64)FileTimeToUI64(&filelistItem.CreationTime));
				break;
			case 16:  // AccessTime (FILETIME)
				Ret=TVar((__int64)FileTimeToUI64(&filelistItem.AccessTime));
				break;
			case 17:  // WriteTime (FILETIME)
				Ret=TVar((__int64)FileTimeToUI64(&filelistItem.WriteTime));
				break;
			case 18: // NumberOfStreams
				Ret=TVar((__int64)filelistItem.NumberOfStreams);
				break;
			case 19: // StreamsSize
				Ret=TVar((__int64)filelistItem.StreamsSize);
				break;
			case 20:  // ChangeTime
				ConvertDate(filelistItem.ChangeTime,strDate,strTime,8,FALSE,FALSE,TRUE,TRUE);
				strDate += L" ";
				strDate += strTime;
				Ret=TVar(strDate.CPtr());
				break;
			case 21:  // ChangeTime (FILETIME)
				Ret=TVar((__int64)FileTimeToUI64(&filelistItem.ChangeTime));
				break;
			case 22:  // CustomData
				Ret=TVar(filelistItem.strCustomData);
				break;
			case 23:  // ReparseTag
			{
				Ret=TVar((__int64)filelistItem.ReparseTag);
				break;
			}
		}
	}

	VMStack.Push(Ret);
	return false;
}

// N=len(V)
static bool lenFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	VMStack.Push(TVar(StrLength(Params[0].toString())));
	return true;
}

static bool ucaseFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& Val(Params[0]);
	StrUpper((wchar_t *)Val.toString());
	VMStack.Push(Val);
	return true;
}

static bool lcaseFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& Val(Params[0]);
	StrLower((wchar_t *)Val.toString());
	VMStack.Push(Val);
	return true;
}

static bool stringFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& Val(Params[0]);
	Val.toString();
	VMStack.Push(Val);
	return true;
}

// S=StrPad(Src,Cnt[,Fill[,Op]])
static bool strpadFunc(const TMacroFunction*)
{
	string strDest;
	parseParams(4,Params);
	TVar& Src(Params[0]);
	if (Src.isUnknown())
	{
		Src=L"";
		Src.toString();
	}
	int Cnt=(int)Params[1].getInteger();
	TVar& Fill(Params[2]);
	if (Fill.isUnknown())
		Fill=L" ";
	DWORD Op=(DWORD)Params[3].getInteger();

	strDest=Src.s();
	int LengthFill = StrLength(Fill.s());
	if (Cnt > 0 && LengthFill > 0)
	{
		int LengthSrc  = StrLength(Src.s());
		int FineLength = Cnt-LengthSrc;

		if (FineLength > 0)
		{
			wchar_t *NewFill=new wchar_t[FineLength+1];
			if (NewFill)
			{
				const wchar_t *pFill=Fill.s();

				for (int I=0; I < FineLength; ++I)
					NewFill[I]=pFill[I%LengthFill];
				NewFill[FineLength]=0;

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

				string strPad=NewFill;
				strPad.SetLength(CntL);
				strPad+=strDest;
				strPad.Append(NewFill, CntR);
				strDest=strPad;

				delete[] NewFill;
			}
		}
	}

	VMStack.Push(strDest.CPtr());
	return true;
}

// S=StrWrap(Text,Width[,Break[,Flags]])
static bool strwrapFunc(const TMacroFunction*)
{
	parseParams(4,Params);
	DWORD Flags=(DWORD)Params[3].getInteger();
	TVar& Break(Params[2]);
	int Width=(int)Params[1].getInteger();
	TVar& Text(Params[0]);

	if (Break.isInteger() && !Break.i())
	{
		Break=L"";
		Break.toString();
	}


	string strDest;
	FarFormatText(Text.s(),Width,strDest,*Break.s()?Break.s():nullptr,Flags);
	VMStack.Push(strDest.CPtr());
	return true;
}

static bool intFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& Val(Params[0]);
	Val.toInteger();
	VMStack.Push(Val);
	return true;
}

static bool floatFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& Val(Params[0]);
	//Val.toDouble();
	VMStack.Push(Val);
	return true;
}

static bool absFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& tmpVar(Params[0]);

	if (tmpVar < 0ll)
		tmpVar=-tmpVar;

	VMStack.Push(tmpVar);
	return true;
}

static bool ascFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& tmpVar(Params[0]);

	if (tmpVar.isString())
	{
		tmpVar = (__int64)((DWORD)((WORD)*tmpVar.toString()));
		tmpVar.toInteger();
	}

	VMStack.Push(tmpVar);
	return true;
}

static bool chrFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& tmpVar(Params[0]);

	if (tmpVar.isInteger())
	{
		const wchar_t tmp[]={static_cast<wchar_t>(tmpVar.i()), L'\0'};
		tmpVar = tmp;
		tmpVar.toString();
	}

	VMStack.Push(tmpVar);
	return true;
}

// N=FMatch(S,Mask)
static bool fmatchFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar& Mask(Params[1]);
	TVar& S(Params[0]);
	CFileMask FileMask;

	if (FileMask.Set(Mask.toString(), FMF_SILENT))
		VMStack.Push(FileMask.Compare(S.toString()));
	else
		VMStack.Push(-1);
	return true;
}

// V=Editor.Sel(Action[,Opt])
static bool editorselFunc(const TMacroFunction*)
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
	parseParams(2,Params);
	TVar Ret(0ll);
	TVar& Opts(Params[1]);
	TVar& Action(Params[0]);

	int Mode=CtrlObject->Macro.GetMode();
	int NeedType = Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS)); // MACRO_SHELL?
	Frame* CurFrame=FrameManager->GetCurrentFrame();

	if (CurFrame && CurFrame->GetType()==NeedType)
	{
		if (Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
			Ret=CtrlObject->CmdLine->VMProcess(MCODE_F_EDITOR_SEL,ToPtr(Action.toInteger()),Opts.i());
		else
			Ret=CurFrame->VMProcess(MCODE_F_EDITOR_SEL,ToPtr(Action.toInteger()),Opts.i());
	}

	VMStack.Push(Ret);
	return Ret.i() == 1;
}

// V=Editor.Undo(Action)
static bool editorundoFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar Ret(0ll);
	TVar& Action(Params[0]);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		EditorUndoRedo eur;
		eur.Command=static_cast<EDITOR_UNDOREDO_COMMANDS>(Action.toInteger());
		Ret=(__int64)CtrlObject->Plugins->CurEditor->EditorControl(ECTL_UNDOREDO,&eur);
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// N=Editor.SetTitle([Title])
static bool editorsettitleFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar Ret(0ll);
	TVar& Title(Params[0]);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		if (Title.isInteger() && !Title.i())
		{
			Title=L"";
			Title.toString();
		}
		Ret=(__int64)CtrlObject->Plugins->CurEditor->EditorControl(ECTL_SETTITLE,(void*)Title.s());
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// N=Editor.DelLine([Line])
static bool editordellineFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar Ret(0ll);
	TVar& Line(Params[0]);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		if (Line.isNumber())
		{
			Ret=(__int64)CtrlObject->Plugins->CurEditor->VMProcess(MCODE_F_EDITOR_DELLINE, nullptr, Line.getInteger()-1);
		}
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// S=Editor.GetStr([Line])
static bool editorgetstrFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	__int64 Ret=0;
	TVar Res(L"");
	TVar& Line(Params[0]);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		if (Line.isNumber())
		{
			string strRes;
			Ret=(__int64)CtrlObject->Plugins->CurEditor->VMProcess(MCODE_F_EDITOR_GETSTR, &strRes, Line.getInteger()-1);
			Res=strRes.CPtr();
		}
	}

	VMStack.Push(Res);
	return Ret!=0;
}

// N=Editor.InsStr([S[,Line]])
static bool editorinsstrFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(0ll);
	TVar& S(Params[0]);
	TVar& Line(Params[1]);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		if (Line.isNumber())
		{
			if (S.isUnknown())
			{
				S=L"";
				S.toString();
			}

			Ret=(__int64)CtrlObject->Plugins->CurEditor->VMProcess(MCODE_F_EDITOR_INSSTR, (wchar_t *)S.s(), Line.getInteger()-1);
		}
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// N=Editor.SetStr([S[,Line]])
static bool editorsetstrFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(0ll);
	TVar& S(Params[0]);
	TVar& Line(Params[1]);

	if (CtrlObject->Macro.GetMode()==MACRO_EDITOR && CtrlObject->Plugins->CurEditor && CtrlObject->Plugins->CurEditor->IsVisible())
	{
		if (Line.isNumber())
		{
			if (S.isUnknown())
			{
				S=L"";
				S.toString();
			}

			Ret=(__int64)CtrlObject->Plugins->CurEditor->VMProcess(MCODE_F_EDITOR_SETSTR, (wchar_t *)S.s(), Line.getInteger()-1);
		}
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// N=Plugin.Exist(Guid)
static bool pluginexistFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar Ret(0ll);
	TVar& pGuid(Params[0]);

	if (pGuid.s())
	{
		GUID guid;
		Ret=(StrToGuid(pGuid.s(),guid) && CtrlObject->Plugins->FindPlugin(guid))?1:0;
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// N=Plugin.Load(DllPath[,ForceLoad])
static bool pluginloadFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(0ll);
	TVar& ForceLoad(Params[1]);
	TVar& DllPath(Params[0]);
	if (DllPath.s())
		Ret=(__int64)pluginapi::apiPluginsControl(nullptr, !ForceLoad.i()?PCTL_LOADPLUGIN:PCTL_FORCEDLOADPLUGIN, 0, (void*)DllPath.s());
	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// N=Plugin.UnLoad(DllPath)
static bool pluginunloadFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar Ret(0ll);
	TVar& DllPath(Params[0]);
	if (DllPath.s())
	{
		Plugin* p = CtrlObject->Plugins->GetPlugin(DllPath.s());
		if(p)
		{
			Ret=(__int64)pluginapi::apiPluginsControl(p, PCTL_UNLOADPLUGIN, 0, nullptr);
		}
	}

	VMStack.Push(Ret);
	return Ret.i()!=0;
}

// S=Macro.Keyword(Index[,Type])
static bool macroenumkwdFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(L"");
	TVar& Index(Params[0]);
	TVar& Type(Params[1]);

	if (Index.isInteger())
	{
		size_t I=Index.toInteger()-1;

		if ((int)I < 0)
		{
			size_t CountsDefs[]={ARRAYSIZE(MKeywords),ARRAYSIZE(MKeywordsArea)-3,ARRAYSIZE(MKeywordsFlags),ARRAYSIZE(KeyMacroCodes),ARRAYSIZE(MKeywordsVarType)};
			int iType = Type.toInteger();
			Ret=(int)(((unsigned)iType < ARRAYSIZE(CountsDefs))?CountsDefs[iType]:-1);
		}
		else
		{
			switch (Type.toInteger())
			{
				case 0: // Far Keywords
				{
					if (I < ARRAYSIZE(MKeywords))
						Ret=MKeywords[I].Name;
					break;
				}
				case 1: // Area
				{
					I+=3;
					if (I < ARRAYSIZE(MKeywordsArea))
						Ret=MKeywordsArea[I].Name;
					break;
				}
				case 2: // Macro Flags
				{
					if (I < ARRAYSIZE(MKeywordsFlags))
						Ret=MKeywordsFlags[I].Name;
					break;
				}
				case 3: // Macro Operation
				{
					if (I < ARRAYSIZE(KeyMacroCodes))
						Ret=KeyMacroCodes[I].Name;
					break;
				}
				case 4: // type name
				{
					if (I < ARRAYSIZE(MKeywordsVarType))
						Ret=MKeywordsVarType[I].Name;
					break;
				}
			}
		}
	}

	VMStack.Push(Ret);
	return Ret.isString()?(*Ret.s()!=0):(Ret.i() != -1);
}

// S=Macro.Func(Index[,Type])
static bool macroenumfuncFunc(const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(L"");
	TVar& Index(Params[0]);
	TVar& Type(Params[1]);

	if (Index.isInteger())
	{
		size_t I=Index.toInteger()-1;

		if ((int)I < 0)
			Ret=(int)KeyMacro::GetCountMacroFunction();
		else
		{
			const TMacroFunction *MFunc=KeyMacro::GetMacroFunction(I);
			if (MFunc)
			{
				switch (Type.toInteger())
				{
					case 0: // Name
						Ret=(const wchar_t*)MFunc->Name;
						break;
					case 1: // Syntax
						Ret=(const wchar_t*)MFunc->Syntax;
						break;
					case 2: // GUID Host
						Ret=(const wchar_t*)MFunc->fnGUID;
						break;
				}
			}
		}
	}

	VMStack.Push(Ret);
	return Ret.isString()?(*Ret.s()!=0):(Ret.i() != -1);
}

static bool _MacroEnumWords(int TypeTable,const TMacroFunction*)
{
	parseParams(2,Params);
	TVar Ret(0);
	TVar& Index(Params[0]);
	TVar& Type(Params[1]);

	Ret.SetType(vtUnknown);
	if (Index.isInteger())
	{
		int I=static_cast<int>(Index.toInteger()-1);

		//TVarTable *t = KeyMacro::GetLocalVarTable();
		TVarTable *t=(TypeTable==MACRO_VARS)?&glbVarTable:&glbConstTable;

		if (I < 0)
		{
			for (I=0; varEnum(*t,I); ++I)
				;
			Ret=I;
		}
		else
		{
			TVarSet *v=varEnum(*t,I);

			if (v)
			{
				switch (Type.toInteger())
				{
					case 0: // Name
						Ret=(const wchar_t*)v->str;
						break;
					case 1: // Value
						Ret=v->value;
						break;
					case 2: // Type (число)
						Ret=v->value.type();
						break;
					case 3: // TypeName
						Ret=(const wchar_t*)MKeywordsVarType[v->value.type()].Name;
						break;
				}
			}
		}
	}

	VMStack.Push(Ret);
	return Ret.isUnknown()?false:true;
}

// S=Macro.Var(Index[,Type])
static bool macroenumvarFunc(const TMacroFunction *mf)
{
	return _MacroEnumWords(MACRO_VARS,mf);
}

// S=Macro.Const(Index[,Type])
static bool macroenumConstFunc(const TMacroFunction *mf)
{
	return _MacroEnumWords(MACRO_CONSTS,mf);
}

static void VarToFarMacroValue(const TVar& From,FarMacroValue& To)
{
	To.Type=(FARMACROVARTYPE)From.type();
	switch(From.type())
	{
		case vtUnknown:
		case vtInteger:
			To.Integer=From.i();
			break;
		case vtString:
			To.String=xf_wcsdup(From.s());
			break;
		case vtDouble:
			To.Double=From.d();
			break;
		//case vtUnknown:
		//	break;
	}
}

// N=testfolder(S)
/*
возвращает одно состояний тестируемого каталога:

TSTFLD_NOTFOUND   (2) - нет такого
TSTFLD_NOTEMPTY   (1) - не пусто
TSTFLD_EMPTY      (0) - пусто
TSTFLD_NOTACCESS (-1) - нет доступа
TSTFLD_ERROR     (-2) - ошибка (кривые параметры или нехватило памяти для выделения промежуточных буферов)
*/
static bool testfolderFunc(const TMacroFunction*)
{
	parseParams(1,Params);
	TVar& tmpVar(Params[0]);
	__int64 Ret=TSTFLD_ERROR;

	if (tmpVar.isString())
	{
		DisableElevation de;
		Ret=(__int64)TestFolder(tmpVar.s());
	}

	VMStack.Push(Ret);
	return Ret?true:false;
}

// вызов плагиновой функции
static bool pluginsFunc(const TMacroFunction *thisFunc)
{
	TVar V;
	bool Ret=false;
	int nParam=VMStack.Pop().getInteger();
#if defined(MANTIS_0000466)
/*
enum FARMACROVARTYPE
{
	FMVT_UNKNOWN                = 0,
	FMVT_INTEGER                = 1,
	FMVT_STRING                 = 2,
	FMVT_DOUBLE                 = 3,
};

struct FarMacroValue
{
	enum FARMACROVARTYPE Type;
	union
	{
		__int64  Integer;
		double   Double;
		const wchar_t *String;
	}
	Value
	;
};

struct ProcessMacroFuncInfo
{
	size_t StructSize;
	const wchar_t *Name;
	const FarMacroValue *Params;
	int nParams;
	struct FarMacroValue *Results;
	int nResults;
};
*/
	GUID guid;
	if (StrToGuid(thisFunc->fnGUID,guid) && CtrlObject->Plugins->FindPlugin(guid))
	{
		FarMacroValue *vParams=new FarMacroValue[nParam];
		if (vParams)
		{
			int I;
			memset(vParams,0,sizeof(FarMacroValue) * nParam);

			for (I=nParam-1; I >= 0; --I)
			{
				VMStack.Pop(V);
				VarToFarMacroValue(V,*(vParams+I));
			}

			ProcessMacroInfo Info={sizeof(Info),FMIT_PROCESSFUNC};
			Info.Func.StructSize=sizeof(ProcessMacroFuncInfo);
			Info.Func.Name=thisFunc->Name;
			Info.Func.Params=vParams;
			Info.Func.nParams=nParam;

			if (CtrlObject->Plugins->ProcessMacro(guid,&Info))
			{
				if (Info.Func.Results)
				{
					for (I=0; I < Info.Func.nResults; ++I)
					//for (I=nResults-1; I >= 0; --I)
					{
						//V.type()=(TVarType)(Results+I)->Type;
						switch((Info.Func.Results+I)->Type)
						{
							case FMVT_INTEGER:
								V=(Info.Func.Results+I)->Integer;
								break;
							case FMVT_STRING:
								V=(Info.Func.Results+I)->String;
								break;
							case FMVT_DOUBLE:
								V=(Info.Func.Results+I)->Double;
								break;
							case FMVT_UNKNOWN:
								V=0;
								break;
						}
						VMStack.Push(V);
					}
				}
			}

			for (I=0; I < nParam; ++I)
				if((vParams+I)->Type == FMVT_STRING && (vParams+I)->String)
					xf_free((void*)(vParams+I)->String);

			delete[] vParams;
		}
		else
			VMStack.Push(0);
	}
	else
	{
		while(--nParam >= 0)
			VMStack.Pop(V);

		VMStack.Push(0);
	}
#else
	/* времянка */ while(--nParam >= 0) VMStack.Pop(V);
#endif
	return Ret;
}

// вызов пользовательской функции
static bool usersFunc(const TMacroFunction *thisFunc)
{
	TVar V;
	bool Ret=false;

	int nParam=VMStack.Pop().getInteger();
	/* времянка */ while(--nParam >= 0) VMStack.Pop(V);

	VMStack.Push(0);
	return Ret;
}


const wchar_t *eStackAsString(int)
{
	const wchar_t *s=__varTextDate.toString();
	return !s?L"":s;
}

int KeyMacro::GetKey()
{
	MacroRecord *MR;
	TVar tmpVar;
	TVarSet *tmpVarSet=nullptr;

	//_SVS(SysLog(L">KeyMacro::GetKey() InternalInput=%d Executing=%d (%p)",InternalInput,Work.Executing,FrameManager->GetCurrentFrame()));
	if (InternalInput || !FrameManager->GetCurrentFrame())
	{
		//_KEYMACRO(SysLog(L"[%d] return RetKey=%d",__LINE__,RetKey));
		return 0;
	}

	int RetKey=0;  // функция должна вернуть 0 - сигнал о том, что макропоследовательности нет

	if (Work.Executing == MACROMODE_NOMACRO)
	{
		if (!Work.MacroWORK)
		{
			if (CurPCStack >= 0)
			{
				//_KEYMACRO(SysLog(L"[%d] if(CurPCStack >= 0)",__LINE__));
				PopState();
				return RetKey;
			}

			if (Mode==MACRO_EDITOR &&
			        IsRedrawEditor &&
			        CtrlObject->Plugins->CurEditor &&
			        CtrlObject->Plugins->CurEditor->IsVisible() &&
			        LockScr)
			{
				CtrlObject->Plugins->ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL,CtrlObject->Plugins->CurEditor->GetId());
				CtrlObject->Plugins->CurEditor->Show();
			}

			if (CurPCStack < 0)
			{
				if (LockScr)
					delete LockScr;

				LockScr=nullptr;
			}

			if (ConsoleTitle::WasTitleModified())
				ConsoleTitle::SetFarTitle(nullptr);

			Clipboard::SetUseInternalClipboardState(false); //??
			//_KEYMACRO(SysLog(L"[%d] return RetKey=%d",__LINE__,RetKey));
			return RetKey;
		}

		/*
		else if(Work.ExecLIBPos>=MR->BufferSize)
		{
			ReleaseWORKBuffer();
			Work.Executing=MACROMODE_NOMACRO;
			return FALSE;
		}
		else
		*/
		//if(Work.MacroWORK)
		{
			Work.Executing=Work.MacroWORK[0].Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
			Work.ExecLIBPos=0; //?????????????????????????????????
		}
		//else
		//	return FALSE;
	}

initial:

	if (!(MR=Work.MacroWORK) || !MR->Buffer)
	{
		//_KEYMACRO(SysLog(L"[%d] return RetKey=%d",__LINE__,RetKey));
		return 0; // RetKey; ?????
	}

	//_SVS(SysLog(L"KeyMacro::GetKey() initial: Work.ExecLIBPos=%d (%d) %p",Work.ExecLIBPos,MR->BufferSize,Work.MacroWORK));

	// ВНИМАНИЕ! Возможны глюки!
	if (!Work.ExecLIBPos && !LockScr && (MR->Flags&MFLAGS_DISABLEOUTPUT))
		LockScr=new LockScreen;

begin:

	if (!MR || Work.ExecLIBPos>=MR->BufferSize || !MR->Buffer)
	{
done:

		/*$ 10.08.2000 skv
			If we are in editor mode, and CurEditor defined,
			we need to call this events.
			EE_REDRAW EEREDRAW_ALL    - to notify that whole screen updated
			->Show() to actually update screen.

			This duplication take place since ShowEditor method
			will NOT send this event while screen is locked.
		*/
		if (Mode==MACRO_EDITOR &&
		        IsRedrawEditor &&
		        CtrlObject->Plugins->CurEditor &&
		        CtrlObject->Plugins->CurEditor->IsVisible()
		        /* && LockScr*/) // Mantis#0001595
		{
			CtrlObject->Plugins->ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL,CtrlObject->Plugins->CurEditor->GetId());
			CtrlObject->Plugins->CurEditor->Show();
		}

		if (CurPCStack < 0 && (Work.MacroWORKCount-1) <= 0) // mantis#351
		{
			if (LockScr) delete LockScr;
			LockScr=nullptr;
			if (MR)
				MR->Flags&=~MFLAGS_DISABLEOUTPUT; // ????
		}

		Clipboard::SetUseInternalClipboardState(false); //??
		Work.Executing=MACROMODE_NOMACRO;
		ReleaseWORKBuffer();

		// проверим - "а есть ли в временном стеке еще макрЫсы"?
		if (Work.MacroWORKCount > 0)
		{
			// нашлось, запустим механизму по новой
			Work.ExecLIBPos=0;
		}

		if (ConsoleTitle::WasTitleModified())
			ConsoleTitle::SetFarTitle(nullptr); // выставим нужный заголовок по завершению макроса

		//FrameManager->RefreshFrame();
		//FrameManager->PluginCommit();
		_KEYMACRO(SysLog(-1); SysLog(L"[%d] **** End Of Execute Macro ****",__LINE__));
		if (--Work.KeyProcess < 0)
			Work.KeyProcess=0;
		_KEYMACRO(SysLog(L"Work.KeyProcess=%d",Work.KeyProcess));

		if (Work.MacroWORKCount <= 0 && CurPCStack >= 0)
		{
			PopState();
			goto initial;
		}

		ScrBuf.RestoreMacroChar();
		Work.HistoryDisable=0;

		StopMacro=false;

		return KEY_NONE; // Здесь ВСЕГДА!
	}

	if (!Work.ExecLIBPos)
		Work.Executing=Work.MacroWORK[0].Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;

	// Mantis#0000581: Добавить возможность прервать выполнение макроса
	{
		INPUT_RECORD rec;

		//if (PeekInputRecord(&rec) && rec.EventType==KEY_EVENT && rec.Event.KeyEvent.wVirtualKeyCode == VK_CANCEL)
		if (StopMacro)
		{
			GetInputRecord(&rec,true);  // удаляем из очереди эту "клавишу"...
			Work.KeyProcess=0;
			VMStack.Pop();              // Mantis#0000841 - (TODO: возможно здесь одним Pop`ом не обойтись, нужно проверить!)
			goto done;                  // ...и завершаем макрос.
		}
	}

	DWORD Key=!MR?MCODE_OP_EXIT:GetOpCode(MR,Work.ExecLIBPos++);

	string value;
	_KEYMACRO(SysLog(L"[%d] IP=%d Op=%08X ==> %s or %s",__LINE__,Work.ExecLIBPos-1,Key,_MCODE_ToName(Key),_FARKEY_ToName(Key)));

	if (Work.KeyProcess && Key != MCODE_OP_ENDKEYS)
	{
		_KEYMACRO(SysLog(L"[%d] IP=%d  %s (Work.KeyProcess (%d) && Key != MCODE_OP_ENDKEYS)",__LINE__,Work.ExecLIBPos-1,_FARKEY_ToName(Key),Work.KeyProcess));
		goto return_func;
	}

	switch (Key)
	{
		case MCODE_OP_NOP:
			goto begin;
		case MCODE_OP_KEYS:                    // за этим кодом следуют ФАРовы коды клавиш
		{
			_KEYMACRO(SysLog(L"MCODE_OP_KEYS (Work.KeyProcess=%d)",Work.KeyProcess));
			Work.KeyProcess++;
			goto begin;
		}
		case MCODE_OP_ENDKEYS:                 // ФАРовы коды закончились.
		{
			_KEYMACRO(SysLog(L"MCODE_OP_ENDKEYS (Work.KeyProcess=%d)",Work.KeyProcess));
			Work.KeyProcess--;
			goto begin;
		}
		case KEY_ALTINS:
		case KEY_RALTINS:
		{
			if (RunGraber())
				return KEY_NONE;

			break;
		}

		case MCODE_OP_XLAT:               // $XLat
		{
			return KEY_OP_XLAT;
		}
		case MCODE_OP_SELWORD:            // $SelWord
		{
			return KEY_OP_SELWORD;
		}
		case MCODE_F_PRINT:               // N=Print(Str)
		case MCODE_OP_PLAINTEXT:          // $Text "Text"
		{
			if (VMStack.empty())
				return KEY_NONE;

			if (Key == MCODE_F_PRINT)
			{
				parseParams(1,Params);
				__varTextDate=Params[0];
				VMStack.Push(1);
			}
			else
			{
				VMStack.Pop(__varTextDate);
			}
			return KEY_OP_PLAINTEXT;
		}
		case MCODE_OP_EXIT:               // $Exit
		{
			goto done;
		}

		case MCODE_OP_AKEY:               // $AKey
		{
			DWORD aKey=KEY_NONE;
			if (!(MR->Flags&MFLAGS_POSTFROMPLUGIN))
			{
				INPUT_RECORD *inRec=&Work.cRec;
				if (!inRec->EventType)
					inRec->EventType = KEY_EVENT;
				if(inRec->EventType == KEY_EVENT || inRec->EventType == FARMACRO_KEY_EVENT)
					aKey=ShieldCalcKeyCode(inRec,FALSE,nullptr);
			}
			else
				aKey=MR->Key;
			return aKey;
		}

		case MCODE_F_AKEY:                // V=akey(Mode[,Type])
		{
			parseParams(2,Params);
			int tmpType=(int)Params[1].getInteger();
			int tmpMode=(int)Params[0].getInteger();

			DWORD aKey=MR->Key;

			if (!tmpType)
			{
				if (!(MR->Flags&MFLAGS_POSTFROMPLUGIN))
				{
					INPUT_RECORD *inRec=&Work.cRec;
					if (!inRec->EventType)
						inRec->EventType = KEY_EVENT;
					if(inRec->EventType == MOUSE_EVENT || inRec->EventType == KEY_EVENT || inRec->EventType == FARMACRO_KEY_EVENT)
						aKey=ShieldCalcKeyCode(inRec,FALSE,nullptr);
				}
				else if (!aKey)
					aKey=KEY_NONE;
			}

			if (!tmpMode)
				tmpVar=(__int64)aKey;
			else
			{
				KeyToText(aKey,value);
				tmpVar=value.CPtr();
				tmpVar.toString();
			}
			VMStack.Push(tmpVar);
			goto begin;
		}

		case MCODE_F_HISTIORY_DISABLE: // N=History.Disable([State])
		{
		    parseParams(1,Params);
			TVar& State(Params[0]);

			DWORD oldHistoryDisable=Work.HistoryDisable;

			if (!State.isUnknown())
				Work.HistoryDisable=(DWORD)State.getInteger();

			VMStack.Push((__int64)oldHistoryDisable);
			goto begin;
		}

		// $Rep (expr) ... $End
		// -------------------------------------
		//            <expr>
		//            MCODE_OP_SAVEREPCOUNT       1
		// +--------> MCODE_OP_REP                2   p1=*
		// |          <counter>                   3
		// |          <counter>                   4
		// |          MCODE_OP_JZ  ------------+  5   p2=*+2
		// |          ...                      |
		// +--------- MCODE_OP_JMP             |
		//            MCODE_OP_END <-----------+
		case MCODE_OP_SAVEREPCOUNT:
		{
			// получим оригинальное значение счетчика
			// со стека и запишем его в рабочее место
			LARGE_INTEGER Counter;

			if ((Counter.QuadPart=VMStack.Pop().getInteger()) < 0)
				Counter.QuadPart=0;

			SetOpCode(MR,Work.ExecLIBPos+1,Counter.u.HighPart);
			SetOpCode(MR,Work.ExecLIBPos+2,Counter.u.LowPart);
			SetMacroConst(constRCounter,Counter.QuadPart);
			goto begin;
		}
		case MCODE_OP_REP:
		{
			// получим текущее значение счетчика
			LARGE_INTEGER Counter ={GetOpCode(MR,Work.ExecLIBPos+1), (LONG)GetOpCode(MR,Work.ExecLIBPos)};
			// и положим его на вершину стека
			VMStack.Push(Counter.QuadPart);
			SetMacroConst(constRCounter,Counter.QuadPart);
			// уменьшим его и пойдем на MCODE_OP_JZ
			Counter.QuadPart--;
			SetOpCode(MR,Work.ExecLIBPos++,Counter.u.HighPart);
			SetOpCode(MR,Work.ExecLIBPos++,Counter.u.LowPart);
			goto begin;
		}
		case MCODE_OP_END:
			// просто пропустим этот рудимент синтаксиса :)
			goto begin;
		case MCODE_F_MMODE:               // N=MMode(Action[,Value])
		{
			parseParams(2,Params);
			__int64 nValue = (__int64)Params[1].getInteger();
			TVar& Action(Params[0]);

			__int64 Result=0;

			switch (Action.getInteger())
			{
				case 1: // DisableOutput
				{
					Result=LockScr?1:0;

					if (nValue == 2) // изменяет режим отображения ("DisableOutput").
					{
						if (MR->Flags&MFLAGS_DISABLEOUTPUT)
							nValue=0;
						else
							nValue=1;
					}

					switch (nValue)
					{
						case 0: // DisableOutput=0, разлочить экран
							if (LockScr)
							{
								delete LockScr;
								LockScr=nullptr;
							}
							MR->Flags&=~MFLAGS_DISABLEOUTPUT;
							break;
						case 1: // DisableOutput=1, залочить экран
							if (!LockScr)
								LockScr=new LockScreen;
							MR->Flags|=MFLAGS_DISABLEOUTPUT;
							break;
					}

					break;
				}

				case 2: // Get MacroRecord Flags
				{
					Result=(__int64)MR->Flags;
					if ((Result&MFLAGS_MODEMASK) == MACRO_COMMON)
						Result|=0x00FF; // ...что бы Common был всегда последним.
					break;
				}

				case 3: // CallPlugin Rules
				{
					Result=MR->Flags&MFLAGS_CALLPLUGINENABLEMACRO?1:0;

					if (nValue == 2) // изменяет режим
					{
						if (MR->Flags&MFLAGS_CALLPLUGINENABLEMACRO)
							nValue=0;
						else
							nValue=1;
					}

					switch (nValue)
					{
						case 0: // блокировать макросы при вызове плагина функцией CallPlugin
							MR->Flags&=~MFLAGS_CALLPLUGINENABLEMACRO;
							break;
						case 1: // разрешить макросы
							MR->Flags|=MFLAGS_CALLPLUGINENABLEMACRO;
							break;
					}

					break;
				}
			}

			VMStack.Push(Result);
			break;
		}

		case MCODE_OP_DUP:        // продублировать верхнее значение в стеке
			tmpVar=VMStack.Peek();
			VMStack.Push(tmpVar);
			goto begin;

		case MCODE_OP_SWAP:
		{
			VMStack.Swap();
			goto begin;
		}

		case MCODE_OP_DISCARD:    // убрать значение с вершины стека
			VMStack.Pop();
			goto begin;

		/*
		case MCODE_OP_POP:        // 0: pop 1: varname -> присвоить значение переменной и убрать из вершины стека
		{
			VMStack.Pop(tmpVar);
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value);

			if (tmpVarSet)
				tmpVarSet->value=tmpVar;

			goto begin;
		}
        */
		case MCODE_OP_SAVE:
		{
			TVar Val0;
			VMStack.Pop(Val0);    // TODO: Заменить на "Val0=VMStack.Peek();", для удаления из стека есть MCODE_OP_DISCARD
			GetPlainText(value);

			// здесь проверка нужна, т.к. существует вариант вызова функции, без присвоения переменной
			if (!value.IsEmpty())
			{
				TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
				varInsert(*t, value)->value = Val0;
			}

			goto begin;
		}
		/*                               Вместо
			0: MCODE_OP_COPY                 0:   MCODE_OP_PUSHVAR
			1: szVarDest                     1:   VarSrc
			...                              ...
			N: szVarSrc                      N:   MCODE_OP_SAVE
			...                            N+1:   VarDest
			                               N+2:
			                                 ...
		*/
		case MCODE_OP_COPY:       // 0: Copy 1: VarDest 2: VarSrc ==>  %a=%d
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value,true);

			if (tmpVarSet)
				tmpVar=tmpVarSet->value;

			GetPlainText(value);
			t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value,true);

			if (tmpVarSet)
				tmpVar=tmpVarSet->value;

			goto begin;
		}
		case MCODE_OP_PUSHFLOAT:
		{
			union { struct { DWORD l, h; }; double d; } u = {GetOpCode(MR,Work.ExecLIBPos+1), GetOpCode(MR,Work.ExecLIBPos)};
			Work.ExecLIBPos+=2;
			VMStack.Push(u.d);
			goto begin;
		}
		case MCODE_OP_PUSHUNKNOWN:
		case MCODE_OP_PUSHINT: // Положить целое значение на стек.
		{
			LARGE_INTEGER i64 = {GetOpCode(MR,Work.ExecLIBPos+1), (LONG)GetOpCode(MR,Work.ExecLIBPos)};
			Work.ExecLIBPos+=2;
			TVar *ptrVar=VMStack.Push(i64.QuadPart);
			if (Key == MCODE_OP_PUSHUNKNOWN)
				ptrVar->SetType(vtUnknown);
			goto begin;
		}
		case MCODE_OP_PUSHCONST:  // Положить на стек константу.
		{
			GetPlainText(value);
			tmpVarSet=varLook(glbConstTable, value,true);

			if (tmpVarSet)
				VMStack.Push(tmpVarSet->value);
			else
				VMStack.Push(0ll);

			goto begin;
		}
		case MCODE_OP_PUSHVAR: // Положить на стек переменную.
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			// %%name - глобальная переменная
			tmpVarSet=varLook(*t, value,true);

			if (tmpVarSet)
				VMStack.Push(tmpVarSet->value);
			else
				VMStack.Push(0ll);

			goto begin;
		}
		case MCODE_OP_PUSHSTR: // Положить на стек строку-константу.
		{
			GetPlainText(value);
			VMStack.Push(TVar(value.CPtr()));
			goto begin;
		}
		// переходы
		case MCODE_OP_JMP:
			Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
			goto begin;

		case MCODE_OP_JZ:
		case MCODE_OP_JNZ:
		case MCODE_OP_JLT:
		case MCODE_OP_JLE:
		case MCODE_OP_JGT:
		case MCODE_OP_JGE:
			if(__CheckCondForSkip(VMStack.Pop(),Key))
				Work.ExecLIBPos=GetOpCode(MR,Work.ExecLIBPos);
			else
				Work.ExecLIBPos++;

			goto begin;

			// операции
		case MCODE_OP_UPLUS:  /*VMStack.Pop(tmpVar); VMStack.Push(-tmpVar); */ goto begin;
		case MCODE_OP_NEGATE: VMStack.Pop(tmpVar); VMStack.Push(-tmpVar); goto begin;
		case MCODE_OP_NOT:    VMStack.Pop(tmpVar); VMStack.Push(!tmpVar); goto begin;

		case MCODE_OP_LT:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() <  tmpVar); goto begin;
		case MCODE_OP_LE:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() <= tmpVar); goto begin;
		case MCODE_OP_GT:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() >  tmpVar); goto begin;
		case MCODE_OP_GE:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() >= tmpVar); goto begin;
		case MCODE_OP_EQ:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() == tmpVar); goto begin;
		case MCODE_OP_NE:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() != tmpVar); goto begin;

		case MCODE_OP_ADD:    VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() +  tmpVar); goto begin;
		case MCODE_OP_SUB:    VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() -  tmpVar); goto begin;
		case MCODE_OP_MUL:    VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() *  tmpVar); goto begin;
		case MCODE_OP_DIV:
		{
			if (VMStack.Peek()==0ll)
			{
				_KEYMACRO(SysLog(L"[%d] IP=%d/0x%08X Error: Divide by zero",__LINE__,Work.ExecLIBPos,Work.ExecLIBPos));
				goto done;
			}

			VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() /  tmpVar);
			goto begin;
		}
		case MCODE_OP_PREINC:                  // ++var_a
		case MCODE_OP_PREDEC:                  // --var_a
		case MCODE_OP_POSTINC:                 // var_a++
		case MCODE_OP_POSTDEC:                 // var_a--
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value,true);
			switch (Key)
			{
				case MCODE_OP_PREINC:                  // ++var_a
					++tmpVarSet->value;
					tmpVar=tmpVarSet->value;
					break;
				case MCODE_OP_PREDEC:                  // --var_a
					--tmpVarSet->value;
					tmpVar=tmpVarSet->value;
					break;
				case MCODE_OP_POSTINC:                 // var_a++
					tmpVar=tmpVarSet->value;
					tmpVarSet->value++;
					break;
				case MCODE_OP_POSTDEC:                 // var_a--
					tmpVar=tmpVarSet->value;
					tmpVarSet->value--;
					break;
			}
			VMStack.Push(tmpVar);
			goto begin;
		}

			// Logical
		case MCODE_OP_AND:    VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() && tmpVar); goto begin;
		case MCODE_OP_OR:     VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() || tmpVar); goto begin;
		case MCODE_OP_XOR:    VMStack.Pop(tmpVar); VMStack.Push(xor_op(VMStack.Pop(),tmpVar)); goto begin;
			// Bit Op
		case MCODE_OP_BITAND: VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() &  tmpVar); goto begin;
		case MCODE_OP_BITOR:  VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() |  tmpVar); goto begin;
		case MCODE_OP_BITXOR: VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() ^  tmpVar); goto begin;
		case MCODE_OP_BITSHR: VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() >> tmpVar); goto begin;
		case MCODE_OP_BITSHL: VMStack.Pop(tmpVar); VMStack.Push(VMStack.Pop() << tmpVar); goto begin;
		case MCODE_OP_BITNOT: VMStack.Pop(tmpVar); VMStack.Push(~tmpVar); goto begin;

		case MCODE_OP_ADDEQ:                   // var_a +=  exp_b
		case MCODE_OP_SUBEQ:                   // var_a -=  exp_b
		case MCODE_OP_MULEQ:                   // var_a *=  exp_b
		case MCODE_OP_DIVEQ:                   // var_a /=  exp_b
		case MCODE_OP_BITSHREQ:                // var_a >>= exp_b
		case MCODE_OP_BITSHLEQ:                // var_a <<= exp_b
		case MCODE_OP_BITANDEQ:                // var_a &=  exp_b
		case MCODE_OP_BITXOREQ:                // var_a ^=  exp_b
		case MCODE_OP_BITOREQ:                 // var_a |=  exp_b
		{
			GetPlainText(value);
			TVarTable *t = (value.At(0) == L'%') ? &glbVarTable : Work.locVarTable;
			tmpVarSet=varLook(*t, value,true);
			VMStack.Pop(tmpVar);
			switch (Key)
			{
				case MCODE_OP_ADDEQ:    tmpVarSet->value  += tmpVar; break;
				case MCODE_OP_SUBEQ:    tmpVarSet->value  -= tmpVar; break;
				case MCODE_OP_MULEQ:    tmpVarSet->value  *= tmpVar; break;
				case MCODE_OP_BITSHREQ: tmpVarSet->value >>= tmpVar; break;
				case MCODE_OP_BITSHLEQ: tmpVarSet->value <<= tmpVar; break;
				case MCODE_OP_BITANDEQ: tmpVarSet->value  &= tmpVar; break;
				case MCODE_OP_BITXOREQ: tmpVarSet->value  ^= tmpVar; break;
				case MCODE_OP_BITOREQ:  tmpVarSet->value  |= tmpVar; break;
				case MCODE_OP_DIVEQ:
				{
					if (tmpVar == 0ll)
						goto done;
					tmpVarSet->value /= tmpVar;
					break;
				}
			}
			VMStack.Push(tmpVarSet->value);
			goto begin;
		}
			// Function
		case MCODE_F_EVAL: // N=eval(S[,N])
		{
			parseParams(2,Params);
			DWORD Cmd=(DWORD)Params[1].getInteger();
			TVar& Val(Params[0]);
			MacroRecord RBuf={};
			int KeyPos;

			if (!(Val.isInteger() && !Val.i())) // учитываем только нормальное содержимое строки компиляции
			{
				int Ret=-1;

				switch (Cmd)
				{
					case 0:
					{
						GetCurRecord(&RBuf,&KeyPos);
						PushState(true);

						if (!(MR->Flags&MFLAGS_DISABLEOUTPUT))
							RBuf.Flags &= ~MFLAGS_DISABLEOUTPUT;

						if (!PostNewMacro(Val.toString(),RBuf.Flags,RBuf.Key))
							PopState();
						else
							Ret=1;
						VMStack.Push((__int64)__getMacroErrorCode());
						break;
					}

					case 1: // только проверка? (и возврат кода ошибки)
					{
						PostNewMacro(Val.toString(),0,0,TRUE);
						VMStack.Push((__int64)__getMacroErrorCode());
						break;
					}
					case 3: // только проверка? (и возврат строки ошибки)
					{
						string strResult;
						PostNewMacro(Val.toString(),0,0,TRUE);
						if (__getMacroErrorCode() != err_Success)
						{
							string ErrMsg[4];
							GetMacroParseError(&ErrMsg[0],&ErrMsg[1],&ErrMsg[2],&ErrMsg[3]);
							strResult=ErrMsg[3]+L"\n"+ErrMsg[0]+L"\n"+ErrMsg[1]+L"\n"+ErrMsg[2];
						}
						VMStack.Push(strResult.CPtr());
						break;
					}


					case 2: // программный вызов макроса, назначенный на кнопкосочетание
					{
						/*
						   Для этого:
						   а) второй параметр функции установить в 2
						   б) первым параметром указать строку в формате "Area/Key"
						      здесь:
						        "Area" - область, из которой хотим вызвать макрос
						        "/" - разделитель
						        "Key" - название клавиши
						      "Area/" можно не указывать, в этом случае поиск "Key" будет вестись в текущей активной макрообласти,
						         если в текущей области "Key" не найден, то поиск продолжится в области Common.
						         Что бы отключить поиск в области Common (ограничится только "этой" областью),
						         необходимо в качестве "Area" указать точку.

						   Для режима 2 функция вернет
						     -1 - ошибка
						     -2 - нет макроса, заданного кпопкосочетанием (или макрос заблокирован)
						      0 - Ok
						*/
						int _Mode;
						bool UseCommon=true;
						string strKeyName;
						string strVal=Val.toString();
						strVal=RemoveExternalSpaces(strVal);

						wchar_t *lpwszVal = strVal.GetBuffer();
						wchar_t *p=wcsrchr(lpwszVal,L'/');

						if (p  && p[1])
						{
							*p++=0;
							if ((_Mode = GetAreaCode(lpwszVal)) < MACRO_FUNCS)
							{
								_Mode=GetMode();
								if (lpwszVal[0] == L'.' && !lpwszVal[1]) // вариант "./Key" не подразумевает поиск в Common`е
									UseCommon=false;
							}
							else
								UseCommon=false;
						}
						else
						{
							p=lpwszVal;
							_Mode=GetMode();
						}

						strKeyName=(const wchar_t*)p;
						DWORD KeyCode = KeyNameToKey(p);
						strVal.ReleaseBuffer();

						int I=GetIndex(KeyCode,strKeyName,_Mode,UseCommon);

						if (I != -1 && !(MacroLIB[I].Flags&MFLAGS_DISABLEMACRO)) // && CtrlObject)
						{
							PushState(true);
							// __setMacroErrorCode(err_Success); // ???
							PostNewMacro(MacroLIB+I);
							VMStack.Push((__int64)__getMacroErrorCode()); // ???
							Ret=1;
						}
						else
						{
							VMStack.Push(-2);
						}
						break;
					}
				}

				if (Ret > 0)
					goto initial; // т.к.
			}
			else
				VMStack.Push(-1);
			goto begin;
		}

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
			parseParams(2,Params);
			TVar& p1(Params[0]);
			TVar& p2(Params[1]);

			__int64 Result=0;
			Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

			while (f)
			{
				fo=f;
				f=f->GetTopModal();
			}

			if (!f)
				f=fo;

			if (f)
				Result=f->VMProcess(Key,ToPtr(p2.i()),p1.i());

			VMStack.Push(Result);
			goto begin;
		}

		case MCODE_F_MENU_ITEMSTATUS:     // N=Menu.ItemStatus([N])
		case MCODE_F_MENU_GETVALUE:       // S=Menu.GetValue([N])
		case MCODE_F_MENU_GETHOTKEY:      // S=gethotkey([N])
		{
			parseParams(1,Params);
			_KEYMACRO(CleverSysLog Clev(Key == MCODE_F_MENU_GETHOTKEY?L"MCODE_F_MENU_GETHOTKEY":(Key == MCODE_F_MENU_ITEMSTATUS?L"MCODE_F_MENU_ITEMSTATUS":L"MCODE_F_MENU_GETVALUE")));
			tmpVar=Params[0];

			if (!tmpVar.isInteger())
				tmpVar=0ll;

			int CurMMode=CtrlObject->Macro.GetMode();

			if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
			{
				Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

				//f=f->GetTopModal();
				while (f)
				{
					fo=f;
					f=f->GetTopModal();
				}

				if (!f)
					f=fo;

				__int64 Result;

				if (f)
				{
					__int64 MenuItemPos=tmpVar.i()-1;
					if (Key == MCODE_F_MENU_GETHOTKEY)
					{
						if ((Result=f->VMProcess(Key,nullptr,MenuItemPos)) )
						{

							const wchar_t _value[]={static_cast<wchar_t>(Result),0};
							tmpVar=_value;
						}
						else
							tmpVar=L"";
					}
					else if (Key == MCODE_F_MENU_GETVALUE)
					{
						string NewStr;
						if (f->VMProcess(Key,&NewStr,MenuItemPos))
						{
							HiText2Str(NewStr, NewStr);
							RemoveExternalSpaces(NewStr);
							tmpVar=NewStr.CPtr();
						}
						else
							tmpVar=L"";
					}
					else if (Key == MCODE_F_MENU_ITEMSTATUS)
					{
						tmpVar=f->VMProcess(Key,nullptr,MenuItemPos);
					}
				}
				else
					tmpVar=L"";
			}
			else
				tmpVar=L"";

			VMStack.Push(tmpVar);
			goto begin;
		}

		case MCODE_F_MENU_SELECT:      // N=Menu.Select(S[,N[,Dir]])
		case MCODE_F_MENU_CHECKHOTKEY: // N=checkhotkey(S[,N])
		{
			parseParams(3,Params);
			_KEYMACRO(CleverSysLog Clev(Key == MCODE_F_MENU_CHECKHOTKEY? L"MCODE_F_MENU_CHECKHOTKEY":L"MCODE_F_MENU_SELECT"));
			__int64 Result=-1;
			__int64 tmpMode=0;
			__int64 tmpDir=0;

			if (Key == MCODE_F_MENU_SELECT)
				tmpDir=Params[2].getInteger();

			tmpMode=Params[1].getInteger();

			if (Key == MCODE_F_MENU_SELECT)
				tmpMode |= (tmpDir << 8);
			else
			{
				if (tmpMode > 0)
					tmpMode--;
			}

			//const wchar_t *checkStr=Params[0].toString();
			int CurMMode=CtrlObject->Macro.GetMode();

			if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
			{
				Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

				//f=f->GetTopModal();
				while (f)
				{
					fo=f;
					f=f->GetTopModal();
				}

				if (!f)
					f=fo;

				if (f)
					Result=f->VMProcess(Key,(void*)Params[0].toString(),tmpMode);
			}

			VMStack.Push(Result);
			goto begin;
		}

		case MCODE_F_MENU_FILTER:      // N=Menu.Filter([Action[,Mode]])
		case MCODE_F_MENU_FILTERSTR:   // S=Menu.FilterStr([Action[,S]])
		{
			parseParams(2,Params);
			_KEYMACRO(CleverSysLog Clev(Key == MCODE_F_MENU_FILTER? L"MCODE_F_MENU_FILTER":L"MCODE_F_MENU_FILTERSTR"));
			bool succees=false;
			TVar& tmpAction(Params[0]);

			tmpVar=Params[1];
			if (tmpAction.isUnknown())
				tmpAction=Key == MCODE_F_MENU_FILTER ? 4 : 0;

			int CurMMode=CtrlObject->Macro.GetMode();

			if (IsMenuArea(CurMMode) || CurMMode == MACRO_DIALOG)
			{
				Frame *f=FrameManager->GetCurrentFrame(), *fo=nullptr;

				//f=f->GetTopModal();
				while (f)
				{
					fo=f;
					f=f->GetTopModal();
				}

				if (!f)
					f=fo;

				if (f)
				{
					if (Key == MCODE_F_MENU_FILTER)
					{
						if (tmpVar.isUnknown())
							tmpVar = -1;
						tmpVar=f->VMProcess(Key,(void*)static_cast<intptr_t>(tmpVar.toInteger()),tmpAction.toInteger());
						succees=true;
					}
					else
					{
						string NewStr;
						if (tmpVar.isString())
							NewStr = tmpVar.toString();
						if (f->VMProcess(Key,(void*)&NewStr,tmpAction.toInteger()))
						{
							tmpVar=NewStr.CPtr();
							succees=true;
						}
					}
				}
			}

			if (!succees)
			{
				if (Key == MCODE_F_MENU_FILTER)
					tmpVar = -1;
				else
					tmpVar = L"";
			}

			VMStack.Push(tmpVar);
			goto begin;
		}

		case MCODE_F_CALLPLUGIN: // V=callplugin(SysID[,param])
		// Алиас CallPlugin, для общности
		case MCODE_F_PLUGIN_CALL: // V=Plugin.Call(SysID[,param])
		{
			__int64 Ret=0;
			int count=VMStack.Pop().getInteger();
			if(count-->0)
			{
				FarMacroValue *vParams=nullptr;
				if(count>0)
				{
					vParams=new FarMacroValue[count];
					memset(vParams,0,sizeof(FarMacroValue)*count);
					TVar value;
					for(int ii=count-1;ii>=0;--ii)
					{
						VMStack.Pop(value);
						VarToFarMacroValue(value,*(vParams+ii));
					}
				}

				TVar SysID; VMStack.Pop(SysID);
				GUID guid;

				if (StrToGuid(SysID.s(),guid) && CtrlObject->Plugins->FindPlugin(guid))
				{
					OpenMacroInfo info={sizeof(OpenMacroInfo),(size_t)count,vParams};

					int CallPluginRules=GetCurrentCallPluginMode();

					if( CallPluginRules == 1)
					{
						PushState(true);
						VMStack.Push(1);
					}
					else
						InternalInput++;

					int ResultCallPlugin=0;

					if (CtrlObject->Plugins->CallPlugin(guid,OPEN_FROMMACRO,&info,&ResultCallPlugin))
						Ret=(__int64)ResultCallPlugin;

					if (MR != Work.MacroWORK) // ??? Mantis#0002094 ???
						MR=Work.MacroWORK;

					if( CallPluginRules == 1 )
						PopState();
					else
					{
						VMStack.Push(Ret);
						InternalInput--;
					}
				}
				else
					VMStack.Push(Ret);

				if(vParams)
				{
					for(int ii=0;ii<count;++ii)
					{
						if(vParams[ii].Type == FMVT_STRING && vParams[ii].String)
							xf_free((void*)vParams[ii].String);
					}
				}

				if (Work.Executing == MACROMODE_NOMACRO)
					goto return_func;
			}
			else
				VMStack.Push(Ret);
			goto begin;
		}

		case MCODE_F_PLUGIN_MENU:   // N=Plugin.Menu(Guid[,MenuGuid])
		case MCODE_F_PLUGIN_CONFIG: // N=Plugin.Config(Guid[,MenuGuid])
		case MCODE_F_PLUGIN_COMMAND: // N=Plugin.Command(Guid[,Command])
		{
			_KEYMACRO(CleverSysLog Clev(Key == MCODE_F_PLUGIN_MENU?L"Plugin.Menu()":(Key == MCODE_F_PLUGIN_CONFIG?L"Plugin.Config()":L"Plugin.Command()")));
			__int64 Ret=0;
			parseParams(2,Params);
			TVar& Arg = (Params[1]);
			TVar& Guid = (Params[0]);
			GUID guid, menuGuid;
			CallPluginInfo Data={CPT_CHECKONLY};
			wchar_t EmptyStr[1]={};
			bool ItemFailed=false;


			switch (Key)
			{
				case MCODE_F_PLUGIN_MENU:
					Data.CallFlags |= CPT_MENU;
					if (!Arg.isUnknown())
					{
						if (StrToGuid(Arg.s(),menuGuid))
							Data.ItemGuid=&menuGuid;
						else
							ItemFailed=true;
					}
					break;
				case MCODE_F_PLUGIN_CONFIG:
					Data.CallFlags |= CPT_CONFIGURE;
					if (!Arg.isUnknown())
					{
						if (StrToGuid(Arg.s(),menuGuid))
							Data.ItemGuid=&menuGuid;
						else
							ItemFailed=true;
					}
					break;
				case MCODE_F_PLUGIN_COMMAND:
					Data.CallFlags |= CPT_CMDLINE;
					if (Arg.isString())
						Data.Command=Arg.s();
					else
						Data.Command=EmptyStr;
					break;
			}

			if (!ItemFailed && StrToGuid(Guid.s(),guid) && CtrlObject->Plugins->FindPlugin(guid))
			{
				// Чтобы вернуть результат "выполнения" нужно проверить наличие плагина/пункта
				Ret=(__int64)CtrlObject->Plugins->CallPluginItem(guid,&Data);
				VMStack.Push(Ret);

				if (Ret)
				{
					// Если нашли успешно - то теперь выполнение
					Data.CallFlags&=~CPT_CHECKONLY;
					CtrlObject->Plugins->CallPluginItem(guid,&Data);
					if (MR != Work.MacroWORK)
						MR=Work.MacroWORK;
				}
			}
			else
			{
				VMStack.Push(Ret);
			}

			// По аналогии с KEY_F11
			FrameManager->RefreshFrame();

			if (Work.Executing == MACROMODE_NOMACRO)
				goto return_func;

			goto begin;
		}

		default:
		{
			size_t J;

			for (J=0; J < CMacroFunction; ++J)
			{
				const TMacroFunction *MFunc = KeyMacro::GetMacroFunction(J);
				if (MFunc->Code == (TMacroOpCode)Key && MFunc->Func)
				{
					UINT64 Flags=MR->Flags;

					if (MFunc->IntFlags&IMFF_UNLOCKSCREEN)
					{
						if (Flags&MFLAGS_DISABLEOUTPUT) // если был - удалим
						{
							if (LockScr) delete LockScr;

							LockScr=nullptr;
						}
					}

					if ((MFunc->IntFlags&IMFF_DISABLEINTINPUT))
						InternalInput++;

					MFunc->Func(MFunc);

					if ((MFunc->IntFlags&IMFF_DISABLEINTINPUT))
						InternalInput--;

					if (MFunc->IntFlags&IMFF_UNLOCKSCREEN)
					{
						if (Flags&MFLAGS_DISABLEOUTPUT) // если стал - залочим
						{
							if (LockScr) delete LockScr;

							LockScr=new LockScreen;
						}
					}

					break;
				}
			}

			if (J >= CMacroFunction)
			{
				DWORD Err=0;
				tmpVar=FARPseudoVariable(MR->Flags, Key, Err);

				if (!Err)
					VMStack.Push(tmpVar);
				else
				{
					if (Key >= KEY_MACRO_BASE && Key <= KEY_MACRO_ENDBASE)
					{
						// это не клавиша, а неопознанный OpCode, прерываем исполнение макроса
						goto done;
					}
					break; // клавиши будем возвращать
				}
			}

			goto begin;
		} // END default
	} // END: switch(Key)

return_func:

	if (Work.KeyProcess != 0 && (Key&KEY_ALTDIGIT)) // "подтасовка" фактов ;-)
	{
		Key&=~KEY_ALTDIGIT;
		IntKeyState.ReturnAltValue=1;
	}

#if 0

	if (MR==Work.MacroWORK &&
	        (Work.ExecLIBPos>=MR->BufferSize || Work.ExecLIBPos+1==MR->BufferSize && MR->Buffer[Work.ExecLIBPos]==KEY_NONE) &&
	        Mode==MACRO_DIALOG
	   )
	{
		RetKey=Key;
		goto done;
	}

#else

	if (MR && MR==Work.MacroWORK && Work.ExecLIBPos>=MR->BufferSize)
	{
		_KEYMACRO(SysLog(-1); SysLog(L"[%d] **** End Of Execute Macro ****",__LINE__));
		if (--Work.KeyProcess < 0)
			Work.KeyProcess=0;
		_KEYMACRO(SysLog(L"Work.KeyProcess=%d",Work.KeyProcess));
		ReleaseWORKBuffer();
		Work.Executing=MACROMODE_NOMACRO;

		if (ConsoleTitle::WasTitleModified())
			ConsoleTitle::SetFarTitle(nullptr);
	}

#endif
	return(Key);
}

// Проверить - есть ли еще клавиша?
int KeyMacro::PeekKey()
{
	if (InternalInput || !Work.MacroWORK)
		return 0;

	MacroRecord *MR=Work.MacroWORK;

	if ((Work.Executing == MACROMODE_NOMACRO && !Work.MacroWORK) || Work.ExecLIBPos >= MR->BufferSize)
		return FALSE;

	DWORD OpCode=GetOpCode(MR,Work.ExecLIBPos);
	return OpCode;
}

UINT64 KeyMacro::SwitchFlags(UINT64& Flags,UINT64 Value)
{
	if (Flags&Value) Flags&=~Value;
	else Flags|=Value;

	return Flags;
}


/*
  после вызова этой функции нужно удалить память!!!
  функция декомпилит только простые последовательности, т.к.... клавиши
  в противном случае возвращает Src
*/
wchar_t *KeyMacro::MkTextSequence(DWORD *Buffer,int BufferSize,const wchar_t *Src)
{
	string strMacroKeyText;
	string strTextBuffer;

	if (!Buffer)
		return nullptr;

#if 0

	if (BufferSize == 1)
	{
		if (
		    (((DWORD)(intptr_t)Buffer)&KEY_MACRO_ENDBASE) >= KEY_MACRO_BASE && (((DWORD)(intptr_t)Buffer)&KEY_MACRO_ENDBASE) <= KEY_MACRO_ENDBASE ||
		    (((DWORD)(intptr_t)Buffer)&KEY_OP_ENDBASE) >= KEY_OP_BASE && (((DWORD)(intptr_t)Buffer)&KEY_OP_ENDBASE) <= KEY_OP_ENDBASE
		)
		{
			return Src?xf_wcsdup(Src):nullptr;
		}

		if (KeyToText((DWORD)(intptr_t)Buffer,strMacroKeyText))
			return xf_wcsdup(strMacroKeyText.CPtr());

		return nullptr;
	}

#endif
	strTextBuffer.Clear();

	if (Buffer[0] == MCODE_OP_KEYS)
		for (int J=1; J < BufferSize; J++)
		{
			int Key=Buffer[J];

			if (Key == MCODE_OP_ENDKEYS || Key == MCODE_OP_KEYS)
				continue;

			if (/*
				(Key&KEY_MACRO_ENDBASE) >= KEY_MACRO_BASE && (Key&KEY_MACRO_ENDBASE) <= KEY_MACRO_ENDBASE ||
				(Key&KEY_OP_ENDBASE) >= KEY_OP_BASE && (Key&KEY_OP_ENDBASE) <= KEY_OP_ENDBASE ||
				*/
			    !KeyToText(Key,strMacroKeyText)
			)
			{
				return Src?xf_wcsdup(Src):nullptr;
			}

			if (J > 1)
				strTextBuffer += L" ";

			strTextBuffer += strMacroKeyText;
		}

	if (!strTextBuffer.IsEmpty())
		return xf_wcsdup(strTextBuffer.CPtr());

	return nullptr;
}

bool KeyMacro::LoadVarFromDB(const wchar_t *Name, TVar &Value)
{
	bool Ret;
	string TempValue, strType;

	Ret=MacroCfg->GetVarValue(Name, TempValue, strType);

	if(Ret)
	{
		Value=TempValue.CPtr();
		switch (GetVarTypeValue(strType))
		{
			case vtUnknown:
				Value.toInteger();
				Value.SetType(vtUnknown);
				break;
			case vtInteger:
				Value.toInteger();
				break;
			case vtDouble:
				Value.toDouble();
				break;
			case vtString:
				break;
			default:
				Value.toString();
				break;
		}

	}

	return Ret;
}

bool KeyMacro::SaveVarToDB(const wchar_t *Name, TVar Value)
{
	bool Ret;

	MacroCfg->BeginTransaction();
	TVarType type=Value.type();
	Ret = MacroCfg->SetVarValue(Name, Value.s(), GetVarTypeName((DWORD)type)) != 0;
	MacroCfg->EndTransaction();

	return Ret;
}

void KeyMacro::WriteVarsConsts()
{
#if 0
	string strValueName;
	TVarTable *t;

	// Consts
	t= &glbConstTable;

	MacroCfg->BeginTransaction();
	for (int I=0; ; I++)
	{
		TVarSet *var=varEnum(*t,I);

		if (!var)
			break;

		strValueName = var->str;
		if(checkMacroFarIntConst(strValueName))
			continue;
		TVarType type=var->value.type();
		MacroCfg->SetConstValue(strValueName, var->value.s(), GetVarTypeName((DWORD)type));
	}
	MacroCfg->EndTransaction();

	// Vars
	t = &glbVarTable;

	MacroCfg->BeginTransaction();
	for (int I=0; ; I++)
	{
		TVarSet *var=varEnum(*t,I);

		if (!var)
			break;

		strValueName = var->str;
		strValueName = L"%"+strValueName;
		TVarType type=var->value.type();
		MacroCfg->SetVarValue(strValueName, var->value.s(), GetVarTypeName((DWORD)type));
	}
	MacroCfg->EndTransaction();
#endif
}

void KeyMacro::SavePluginFunctionToDB(const TMacroFunction *MF)
{
	// раскомментировать для теста записи встроенных функций
	//MacroCfg->SetFunction(MF->Syntax, MF->Name, MF->IntFlags, MF->Name, MF->Name, MF->Name);

	// закомментировать для теста записи встроенных функций
	if(MF->fnGUID && MF->Name)
		MacroCfg->SetFunction(MF->fnGUID, MF->Name, FlagsToString(MF->IntFlags), nullptr, MF->Syntax, nullptr);
}

void KeyMacro::WritePluginFunctions()
{
	const TMacroFunction *Func;
	MacroCfg->BeginTransaction();
		for (size_t I=0; I < CMacroFunction; ++I)
		{
			Func=GetMacroFunction(I);
			if(Func)
			{
				SavePluginFunctionToDB(Func);
			}
		}
	MacroCfg->EndTransaction();
}

void KeyMacro::SaveMacroRecordToDB(const MacroRecord *MR)
{
	int Area;
	DWORD Flags;

	Flags=MR->Flags;

	Area=Flags & MFLAGS_MODEMASK;
	Flags &= ~(MFLAGS_MODEMASK|MFLAGS_NEEDSAVEMACRO);
	string strKeyName;
	if (MR->Key != -1)
		KeyToText(MR->Key, strKeyName);
	else
		strKeyName=(const wchar_t*)MR->Name;

	MacroCfg->SetKeyMacro(GetAreaName(Area), strKeyName, FlagsToString(Flags), MR->Src, MR->Description);
}

void KeyMacro::WriteMacroRecords()
{
	MacroCfg->BeginTransaction();
	for (int I=0; I<MacroLIBCount; I++)
	{
		if (!MacroLIB[I].BufferSize || !MacroLIB[I].Src)
		{
			string strKeyName;

			if (MacroLIB[I].Key != -1)
				KeyToText(MacroLIB[I].Key, strKeyName);
			else
				strKeyName=(const wchar_t*)MacroLIB[I].Name;

			MacroCfg->DeleteKeyMacro(GetAreaName(MacroLIB[I].Flags & MFLAGS_MODEMASK), strKeyName);
			continue;
		}

		if (!(MacroLIB[I].Flags&MFLAGS_NEEDSAVEMACRO))
			continue;

		SaveMacroRecordToDB(&MacroLIB[I]);
		MacroLIB[I].Flags &= ~MFLAGS_NEEDSAVEMACRO;
	}
	MacroCfg->EndTransaction();
}

// Сохранение ВСЕХ макросов
void KeyMacro::SaveMacros()
{
	WritePluginFunctions();
	WriteMacroRecords();
}

void KeyMacro::ReadVarsConsts()
{
	string strName;
	string Value, strType;

	while (MacroCfg->EnumConsts(strName, Value, strType))
	{
		TVarSet *NewSet=varInsert(glbConstTable, strName);
		NewSet->value = Value.CPtr();
		switch (GetVarTypeValue(strType))
		{
			case vtUnknown:
				NewSet->value.toInteger();
				NewSet->value.SetType(vtUnknown);
				break;
			case vtInteger:
				NewSet->value.toInteger();
				break;
			case vtDouble:
				NewSet->value.toDouble();
				break;
			case vtString:
				break;
			default:
				NewSet->value.toString();
				break;
		}
	}

	while (MacroCfg->EnumVars(strName, Value, strType))
	{
		TVarSet *NewSet=varInsert(glbVarTable, strName.CPtr()+1);
		NewSet->value = Value.CPtr();
		switch (GetVarTypeValue(strType))
		{
			case vtUnknown:
				NewSet->value.toInteger();
				NewSet->value.SetType(vtUnknown);
				break;
			case vtInteger:
				NewSet->value.toInteger();
				break;
			case vtDouble:
				NewSet->value.toDouble();
				break;
			case vtString:
				break;
			default:
				NewSet->value.toString();
				break;
		}
	}

	initMacroFarIntConst();
}

void KeyMacro::SetMacroConst(const wchar_t *ConstName, const TVar& Value)
{
	varLook(glbConstTable, ConstName,1)->value = Value;
}

/*
   KeyMacros\Function
*/
void KeyMacro::ReadPluginFunctions()
{
	/*
	 В реестре держать раздел "KeyMacros\Funcs" - библиотека макрофункций, экспортируемых плагинами (ProcessMacroW)
     Имя подраздела - это имя "функции"
     Значения у каждого подраздела:
       Syntax:reg_sz - синтаксис функции (на будущее - в качестве подсказки)
       Params:reg_dword - количество параметров у функции
       OParams:reg_dword - необязательные параметры функции
       Sequence:reg_sz - тело функции
       Flags:reg_dword - флаги
       GUID:reg_sz - GUID или путь к плагину в терминах PluginsCache (зависит от Flags)
       Description:reg_sz - необязательное описание

     Flags - набор битов
       0: в GUID путь к плагину, как в PluginsCache иначе GUID
       1: использовать Sequence вместо плагина; оно же будет юзаться, если GUID пуст
       2: ...


     Обращение к такой функции, как к обычной abs, mix, len, etc.
     Если Plugin не пуст, Sequence игнорируется.
     Plugin - имя подраздела из ветки PluginsCache

	[HKEY_CURRENT_USER\Software\Far2\KeyMacros\Funcs\math.sin]
	"Syntax"="d=sin(V)"
	"nParams"=dword:1
	"oParams"=dword:0
	"Sequence"=""
	"Flags"=dword:0
	"GUID"="C:/Program Files/Far2/Plugins/Calc/bin/calc.dll"
	"Description"="Вычисление значения синуса в военное время"

	plugin_guid TEXT NOT NULL,
	function_name TEXT NOT NULL,
	nparam INTEGER NOT NULL,
	oparam INTEGER NOT NULL,
	flags TEXT, sequence TEXT,
	syntax TEXT NOT NULL,
	description TEXT

	Flags:
		биты:
			0: в GUID путь к плагину, как в PluginsCache иначе GUID
			1: использовать Sequence вместо плагина; оно же будет юзаться, если GUID пуст
			2:

	$1, $2, $3 - параметры
	*/
#if 1
	string strPluginGUID;
	string strFunctionName;
	unsigned __int64 Flags;
	string strSequence, strFlags;
	string strSyntax;
	string strDescription;

	while (MacroCfg->EnumFunctions(strPluginGUID, strFunctionName, strFlags, strSequence, strSyntax, strDescription))
	{
		RemoveExternalSpaces(strPluginGUID);
		RemoveExternalSpaces(strFunctionName);
		RemoveExternalSpaces(strSequence);
		RemoveExternalSpaces(strSyntax);
		RemoveExternalSpaces(strDescription); //пока не задействовано

		MacroRecord mr={};
		bool UsePluginFunc=true;
		if (!strSequence.IsEmpty())
		{
			if (!ParseMacroString(&mr,strSequence.CPtr()))
				mr.Buffer=0;
		}

		Flags=StringToFlags(strFlags);
		// использовать Sequence вместо плагина; оно же будет юзаться, если GUID пуст
		if ((Flags & 2) && (mr.Buffer || strPluginGUID.IsEmpty()))
		{
			UsePluginFunc=false;
		}

		// зарегистрировать функцию
		TMacroFunction MFunc={
			strFunctionName.CPtr(),
			strPluginGUID.CPtr(),
			strSyntax.CPtr(),
			(UsePluginFunc?pluginsFunc:usersFunc),
			mr.Buffer,
			mr.BufferSize,
			0,
			MCODE_F_NOFUNC,
		};

		RegisterMacroFunction(&MFunc);

		if (mr.Buffer)
			xf_free(mr.Buffer);

	}

#endif
}

void KeyMacro::RegisterMacroIntFunction()
{
	static bool InitedInternalFuncs=false;

	if (!InitedInternalFuncs)
	{
		for(size_t I=0; I < ARRAYSIZE(intMacroFunction); ++I)
			KeyMacro::RegisterMacroFunction(intMacroFunction+I);

		InitedInternalFuncs=true;
	}
}

TMacroFunction *KeyMacro::RegisterMacroFunction(const TMacroFunction *tmfunc)
{
	if (!tmfunc->Name || !tmfunc->Name[0])
		return nullptr;

	TMacroOpCode Code = tmfunc->Code;
	if ( !Code || Code == MCODE_F_NOFUNC) // получить временный OpCode относительно KEY_MACRO_U_BASE
		Code=(TMacroOpCode)GetNewOpCode();

	TMacroFunction *pTemp;

	if (CMacroFunction >= AllocatedFuncCount)
	{
		AllocatedFuncCount=AllocatedFuncCount+64;

		if (!(pTemp=(TMacroFunction *)xf_realloc(AMacroFunction,AllocatedFuncCount*sizeof(TMacroFunction))))
			return nullptr;

		AMacroFunction=pTemp;
	}

	pTemp=AMacroFunction+CMacroFunction;

	pTemp->Name=xf_wcsdup(tmfunc->Name);
	pTemp->fnGUID=tmfunc->fnGUID?xf_wcsdup(tmfunc->fnGUID):nullptr;
	pTemp->Syntax=tmfunc->Syntax?xf_wcsdup(tmfunc->Syntax):nullptr;
	//pTemp->Src=tmfunc->Src?xf_wcsdup(tmfunc->Src):nullptr;
	//pTemp->Description=tmfunc->Description?xf_wcsdup(tmfunc->Description):nullptr;
	pTemp->Code=Code;
	pTemp->BufferSize=tmfunc->BufferSize;

	if (tmfunc->BufferSize > 0)
	{
		pTemp->Buffer=(DWORD *)xf_malloc(sizeof(DWORD)*tmfunc->BufferSize);
		if (pTemp->Buffer)
			memmove(pTemp->Buffer,tmfunc->Buffer,sizeof(DWORD)*tmfunc->BufferSize);
	}
	else
		pTemp->Buffer=nullptr;
	pTemp->IntFlags=tmfunc->IntFlags;
	pTemp->Func=tmfunc->Func;

	CMacroFunction++;
	return pTemp;
}

bool KeyMacro::UnregMacroFunction(size_t Index)
{
	if (static_cast<int>(Index) == -1)
	{
		if (AMacroFunction)
		{
			TMacroFunction *pTemp;
			for (size_t I=0; I < CMacroFunction; ++I)
			{
				pTemp=AMacroFunction+I;
				if (pTemp->Name)        xf_free((void*)pTemp->Name);        pTemp->Name=nullptr;
				if (pTemp->fnGUID)      xf_free((void*)pTemp->fnGUID);      pTemp->fnGUID=nullptr;
				if (pTemp->Syntax)      xf_free((void*)pTemp->Syntax);      pTemp->Syntax=nullptr;
				if (pTemp->Buffer)      xf_free((void*)pTemp->Buffer);      pTemp->Buffer=nullptr;
				//if (pTemp->Src)         xf_free((void*)pTemp->Src);         pTemp->Src=nullptr;
				//if (pTemp->Description) xf_free((void*)pTemp->Description); pTemp->Description=nullptr;
			}
			CMacroFunction=0;
			AllocatedFuncCount=0;
			xf_free(AMacroFunction);
			AMacroFunction=nullptr;
		}
	}
	else
	{
		if (AMacroFunction && Index < CMacroFunction)
			AMacroFunction[Index].Code=MCODE_F_NOFUNC;
		else
			return false;
	}

	return true;
}

const TMacroFunction *KeyMacro::GetMacroFunction(size_t Index)
{
	if (AMacroFunction && Index < CMacroFunction)
		return AMacroFunction+Index;

	return nullptr;
}

size_t KeyMacro::GetCountMacroFunction()
{
	return CMacroFunction;
}

DWORD KeyMacro::GetNewOpCode()
{
	return LastOpCodeUF++;
}

int KeyMacro::ReadKeyMacro(int Area)
{
	MacroRecord CurMacro={};
	int Key;
	unsigned __int64 MFlags=0;
	string strKey,strArea,strMFlags;
	string strSequence, strDescription;
	string strGUID;
	int ErrorCount=0;

	strArea=GetAreaName(static_cast<MACROMODEAREA>(Area));

	while(MacroCfg->EnumKeyMacros(strArea, strKey, strMFlags, strSequence, strDescription))
	{
		RemoveExternalSpaces(strKey);
		Key=KeyNameToKey(strKey);
		//if (Key==-1)
		//	continue;

		RemoveExternalSpaces(strSequence);
		RemoveExternalSpaces(strDescription);

		if (strSequence.IsEmpty())
		{
			//ErrorCount++; // Раскомментить, если не допускается пустой "Sequence"
			continue;
		}

		MFlags=StringToFlags(strMFlags);

		CurMacro.Key=Key;
		CurMacro.Buffer=nullptr;
		CurMacro.Src=nullptr;
		CurMacro.Description=nullptr;
		CurMacro.BufferSize=0;
		CurMacro.Flags=MFlags|(Area&MFLAGS_MODEMASK);

		if (Area == MACRO_EDITOR || Area == MACRO_DIALOG || Area == MACRO_VIEWER)
		{
			if (CurMacro.Flags&MFLAGS_SELECTION)
			{
				CurMacro.Flags&=~MFLAGS_SELECTION;
				CurMacro.Flags|=MFLAGS_EDITSELECTION;
			}

			if (CurMacro.Flags&MFLAGS_NOSELECTION)
			{
				CurMacro.Flags&=~MFLAGS_NOSELECTION;
				CurMacro.Flags|=MFLAGS_EDITNOSELECTION;
			}
		}

		if (!ParseMacroString(&CurMacro,strSequence))
		{
			ErrorCount++;
			continue;
		}

		MacroRecord *NewMacros=(MacroRecord *)xf_realloc(MacroLIB,sizeof(*MacroLIB)*(MacroLIBCount+1));

		if (!NewMacros)
		{
			return FALSE;
		}

		MacroLIB=NewMacros;
		CurMacro.Src=xf_wcsdup(strSequence);
		if (!strDescription.IsEmpty())
		{
			CurMacro.Description=xf_wcsdup(strDescription);
		}
		CurMacro.Name=xf_wcsdup(strKey);

		GUID Guid=FarGuid;
		// BUGBUG!
		/*if (GetRegKey(strRegKeyName,L"GUID",strGUID,L"",&regType))
		{
			if(!StrToGuid(strGUID,Guid))
				Guid=FarGuid;
		}*/
		CurMacro.Guid=Guid;

		MacroLIB[MacroLIBCount]=CurMacro;
		MacroLIBCount++;
	}

	return ErrorCount?FALSE:TRUE;
}

// эта функция будет вызываться из тех классов, которым нужен перезапуск макросов
void KeyMacro::RestartAutoMacro(int /*Mode*/)
{
#if 0
	/*
	Область      Рестарт
	-------------------------------------------------------
	Other         0
	Shell         1 раз, при запуске ФАРа
	Viewer        для каждой новой копии вьювера
	Editor        для каждой новой копии редатора
	Dialog        0
	Search        0
	Disks         0
	MainMenu      0
	Menu          0
	Help          0
	Info          1 раз, при запуске ФАРа и выставлении такой панели
	QView         1 раз, при запуске ФАРа и выставлении такой панели
	Tree          1 раз, при запуске ФАРа и выставлении такой панели
	Common        0
	*/
#endif
}

// Функция, запускающая макросы при старте ФАРа
// если уж вставлять предупреждение о недопустимости выполения
// подобных макросов, то именно сюды!
void KeyMacro::RunStartMacro()
{
	if (Opt.Macro.DisableMacro&MDOL_ALL)
		return;

	if (Opt.Macro.DisableMacro&MDOL_AUTOSTART)
		return;

	// временно оставим старый вариант
#if 1

	if (!(CtrlObject->Cp() && CtrlObject->Cp()->ActivePanel && !Opt.OnlyEditorViewerUsed && CtrlObject->Plugins->IsPluginsLoaded()))
		return;

	static int IsRunStartMacro=FALSE;

	if (IsRunStartMacro)
		return;

	if (!IndexMode[MACRO_SHELL][1])
		return;

	MacroRecord *MR=MacroLIB+IndexMode[MACRO_SHELL][0];

	for (int I=0; I < IndexMode[MACRO_SHELL][1]; ++I)
	{
		UINT64 CurFlags;

		if (((CurFlags=MR[I].Flags)&MFLAGS_MODEMASK)==MACRO_SHELL &&
		        MR[I].BufferSize>0 &&
		        // исполняем не задисабленные макросы
		        !(CurFlags&MFLAGS_DISABLEMACRO) &&
		        (CurFlags&MFLAGS_RUNAFTERFARSTART) && CtrlObject)
		{
			if (CheckAll(MACRO_SHELL,CurFlags))
				PostNewMacro(MR+I);
		}
	}

	IsRunStartMacro=TRUE;

#else
	static int AutoRunMacroStarted=FALSE;

	if (AutoRunMacroStarted || !MacroLIB || !IndexMode[Mode][1])
		return;

	//if (!(CtrlObject->Cp() && CtrlObject->Cp()->ActivePanel && !Opt.OnlyEditorViewerUsed && CtrlObject->Plugins->IsPluginsLoaded()))
	if (!(CtrlObject && CtrlObject->Plugins->IsPluginsLoaded()))
		return;

	MacroRecord *MR=MacroLIB+IndexMode[Mode][0];

	for (int I=0; I < IndexMode[Mode][1]; ++I)
	{
		DWORD CurFlags;

		if (((CurFlags=MR[I].Flags)&MFLAGS_MODEMASK)==Mode &&   // этот макрос из этой оперы?
		        MR[I].BufferSize > 0 &&                             // что-то должно быть
		        !(CurFlags&MFLAGS_DISABLEMACRO) &&                  // исполняем не задисабленные макросы
		        (CurFlags&MFLAGS_RUNAFTERFARSTART) &&               // и тока те, что должны стартовать
		        !(CurFlags&MFLAGS_RUNAFTERFARSTARTED)      // и тем более, которые еще не стартовали
		   )
		{
			if (CheckAll(Mode,CurFlags)) // прежде чем запостить - проверим флаги
			{
				PostNewMacro(MR+I);
				MR[I].Flags|=MFLAGS_RUNAFTERFARSTARTED; // этот макрос успешно запулили на старт
			}
		}
	}

	// посчитаем количество оставшихся автостартующих макросов
	int CntStart=0;

	for (int I=0; I < MacroLIBCount; ++I)
		if ((MacroLIB[I].Flags&MFLAGS_RUNAFTERFARSTART) && !(MacroLIB[I].Flags&MFLAGS_RUNAFTERFARSTARTED))
			CntStart++;

	if (!CntStart) // теперь можно сказать, что все стартануло и в функцию RunStartMacro() нефига лазить
		AutoRunMacroStarted=TRUE;

#endif

	if (Work.Executing == MACROMODE_NOMACRO)
		Work.ExecLIBPos=0;  // А надо ли?
}
#endif

// обработчик диалогового окна назначения клавиши
intptr_t WINAPI KeyMacro::AssignMacroDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	string strKeyText;
	static int LastKey=0;
	static DlgParam *KMParam=nullptr;
	const INPUT_RECORD* record=nullptr;
	int key=0;

	if (Msg == DN_CONTROLINPUT)
	{
		record=(const INPUT_RECORD *)Param2;
		if (record->EventType==KEY_EVENT)
		{
			key = InputRecordToKey((const INPUT_RECORD *)Param2);
		}
	}

	//_SVS(SysLog(L"LastKey=%d Msg=%s",LastKey,_DLGMSG_ToName(Msg)));
	if (Msg == DN_INITDIALOG)
	{
		KMParam=reinterpret_cast<DlgParam*>(Param2);
		LastKey=0;
		// <Клавиши, которые не введешь в диалоге назначения>
		DWORD PreDefKeyMain[]=
		{
			KEY_CTRLDOWN,KEY_RCTRLDOWN,KEY_ENTER,KEY_NUMENTER,KEY_ESC,KEY_F1,KEY_CTRLF5,KEY_RCTRLF5,
		};

		for (size_t i=0; i<ARRAYSIZE(PreDefKeyMain); i++)
		{
			KeyToText(PreDefKeyMain[i],strKeyText);
			SendDlgMessage(hDlg,DM_LISTADDSTR,2,const_cast<wchar_t*>(strKeyText.CPtr()));
		}

		DWORD PreDefKey[]=
		{
			KEY_MSWHEEL_UP,KEY_MSWHEEL_DOWN,KEY_MSWHEEL_LEFT,KEY_MSWHEEL_RIGHT,
			KEY_MSLCLICK,KEY_MSRCLICK,KEY_MSM1CLICK,KEY_MSM2CLICK,KEY_MSM3CLICK,
#if 0
			KEY_MSLDBLCLICK,KEY_MSRDBLCLICK,KEY_MSM1DBLCLICK,KEY_MSM2DBLCLICK,KEY_MSM3DBLCLICK,
#endif
		};
		DWORD PreDefModKey[]=
		{
			0,KEY_CTRL,KEY_RCTRL,KEY_SHIFT,KEY_ALT,KEY_RALT,KEY_CTRLSHIFT,KEY_RCTRLSHIFT,KEY_CTRLALT,KEY_RCTRLRALT,KEY_CTRLRALT,KEY_RCTRLALT,KEY_ALTSHIFT,KEY_RALTSHIFT,
		};

		for (size_t i=0; i<ARRAYSIZE(PreDefKey); i++)
		{
			SendDlgMessage(hDlg,DM_LISTADDSTR,2,const_cast<wchar_t*>(L"\1"));

			for (size_t j=0; j<ARRAYSIZE(PreDefModKey); j++)
			{
				KeyToText(PreDefKey[i]|PreDefModKey[j],strKeyText);
				SendDlgMessage(hDlg,DM_LISTADDSTR,2,const_cast<wchar_t*>(strKeyText.CPtr()));
			}
		}

		/*
		int KeySize=GetRegKeySize("KeyMacros","DlgKeys");
		char *KeyStr;
		if(KeySize &&
			(KeyStr=(char*)xf_malloc(KeySize+1))  &&
			GetRegKey("KeyMacros","DlgKeys",KeyStr,"",KeySize)
		)
		{
			UserDefinedList KeybList;
			if(KeybList.Set(KeyStr))
			{
				KeybList.Start();
				const char *OneKey;
				*KeyText=0;
				while(nullptr!=(OneKey=KeybList.GetNext()))
				{
					xstrncpy(KeyText, OneKey, sizeof(KeyText));
					SendDlgMessage(hDlg,DM_LISTADDSTR,2,(long)KeyText);
				}
			}
			xf_free(KeyStr);
		}
		*/
		SendDlgMessage(hDlg,DM_SETTEXTPTR,2,nullptr);
		// </Клавиши, которые не введешь в диалоге назначения>
	}
	else if (Param1 == 2 && Msg == DN_EDITCHANGE)
	{
		LastKey=0;
		_SVS(SysLog(L"[%d] ((FarDialogItem*)Param2)->PtrData='%s'",__LINE__,((FarDialogItem*)Param2)->Data));
		key=KeyNameToKey(((FarDialogItem*)Param2)->Data);

		if (key != -1 && !KMParam->Recurse)
			goto M1;
	}
	else if (Msg == DN_CONTROLINPUT && record->EventType==KEY_EVENT && (((key&KEY_END_SKEY) < KEY_END_FKEY) ||
	                           (((key&KEY_END_SKEY) > INTERNAL_KEY_BASE) && (key&KEY_END_SKEY) < INTERNAL_KEY_BASE_2)))
	{
		//if((key&0x00FFFFFF) >= 'A' && (key&0x00FFFFFF) <= 'Z' && ShiftPressed)
		//key|=KEY_SHIFT;

		//_SVS(SysLog(L"Macro: Key=%s",_FARKEY_ToName(key)));
		// <Обработка особых клавиш: F1 & Enter>
		// Esc & (Enter и предыдущий Enter) - не обрабатываем
		if (key == KEY_ESC ||
		        ((key == KEY_ENTER||key == KEY_NUMENTER) && (LastKey == KEY_ENTER||LastKey == KEY_NUMENTER)) ||
		        key == KEY_CTRLDOWN || key == KEY_RCTRLDOWN ||
		        key == KEY_F1)
		{
			return FALSE;
		}

		/*
		// F1 - особый случай - нужно жать 2 раза
		// первый раз будет выведен хелп,
		// а второй раз - второй раз уже назначение
		if(key == KEY_F1 && LastKey!=KEY_F1)
		{
		  LastKey=KEY_F1;
		  return FALSE;
		}
		*/
		// Было что-то уже нажато и Enter`ом подтверждаем
		_SVS(SysLog(L"[%d] Assign ==> Param2='%s',LastKey='%s'",__LINE__,_FARKEY_ToName((DWORD)key),(LastKey?_FARKEY_ToName(LastKey):L"")));

		if ((key == KEY_ENTER||key == KEY_NUMENTER) && LastKey && !(LastKey == KEY_ENTER||LastKey == KEY_NUMENTER))
			return FALSE;

		// </Обработка особых клавиш: F1 & Enter>
M1:
		_SVS(SysLog(L"[%d] Assign ==> Param2='%s',LastKey='%s'",__LINE__,_FARKEY_ToName((DWORD)key),LastKey?_FARKEY_ToName(LastKey):L""));
		KeyMacro *MacroDlg=KMParam->Handle;

		if ((key&0x00FFFFFF) > 0x7F && (key&0x00FFFFFF) < 0xFFFF)
			key=KeyToKeyLayout((int)(key&0x0000FFFF))|(DWORD)(key&(~0x0000FFFF));

		if (key<0xFFFF)
		{
			key=Upper(static_cast<wchar_t>(key));
		}

		_SVS(SysLog(L"[%d] Assign ==> Param2='%s',LastKey='%s'",__LINE__,_FARKEY_ToName((DWORD)key),LastKey?_FARKEY_ToName(LastKey):L""));
		KMParam->Key=(DWORD)key;
		KeyToText((int)key,strKeyText);
		string strKey;

#ifdef FAR_LUA
		// если УЖЕ есть такой макрос...
		int Area,Index;
		if ((Index=MacroDlg->GetIndex(&Area,(int)key,strKeyText,KMParam->Mode,true,true)) != -1)
		{
			MacroRecord *Mac = MacroDlg->m_Macros[Area].getItem(Index);

			// общие макросы учитываем только при удалении.
			if (MacroDlg->m_RecCode.IsEmpty() || Area!=MACRO_COMMON)
			{
				string strBufKey=Mac->Code();
				InsertQuote(strBufKey);

				bool DisFlags = (Mac->Flags()&MFLAGS_DISABLEMACRO) != 0;
				bool SetChange = MacroDlg->m_RecCode.IsEmpty();
				LangString strBuf;
				if (Area==MACRO_COMMON)
				{
					strBuf = SetChange ? (DisFlags?MMacroCommonDeleteAssign:MMacroCommonDeleteKey) : MMacroCommonReDefinedKey;
					//"Общая макроклавиша '%1'     не активна              : будет удалена         : уже определена."
				}
				else
				{
					strBuf = SetChange ? (DisFlags?MMacroDeleteAssign:MMacroDeleteKey) : MMacroReDefinedKey;
					//"Макроклавиша '%1'           не активна        : будет удалена   : уже определена."
				}
				strBuf << strKeyText;

				// проверим "а не совпадает ли всё?"
				int Result=0;
				if (DisFlags || MacroDlg->m_RecCode != Mac->Code())
				{
					const wchar_t* NoKey=MSG(DisFlags && !SetChange?MMacroDisAnotherKey:MNo);
					Result=Message(MSG_WARNING,SetChange?3:2,MSG(MWarning),
					          strBuf,
					          MSG(MMacroSequence),
					          strBufKey,
					          MSG(SetChange?MMacroDeleteKey2:
					              (DisFlags?MMacroDisDisabledKey:MMacroReDefinedKey2)),
					          MSG(DisFlags && !SetChange?MMacroDisOverwrite:MYes),
					          (SetChange?MSG(MMacroEditKey):NoKey),
					          (!SetChange?nullptr:NoKey));
				}

				if (!Result)
				{
					if (DisFlags)
					{
						// удаляем из DB только если включен автосейв
						if (Opt.AutoSaveSetup)
						{
							MacroCfg->BeginTransaction();
							// удалим старую запись из DB
							MacroCfg->DeleteKeyMacro(GetAreaName(Area), Mac->Name());
							MacroCfg->EndTransaction();
						}
						// раздисаблим
						Mac->m_flags&=~(MFLAGS_DISABLEMACRO|MFLAGS_NEEDSAVEMACRO);
					}

					// в любом случае - вываливаемся
					SendDlgMessage(hDlg,DM_CLOSE,1,0);
					return TRUE;
				}
				else if (SetChange && Result == 1)
				{
					string strDescription;

					if ( !Mac->Code().IsEmpty() )
						strBufKey=Mac->Code();

					if ( !Mac->Description().IsEmpty() )
						strDescription=Mac->Description();

					if (MacroDlg->GetMacroSettings(key,Mac->m_flags,strBufKey,strDescription))
					{
						KMParam->Flags = Mac->m_flags;
						KMParam->Changed = true;
						// в любом случае - вываливаемся
						SendDlgMessage(hDlg,DM_CLOSE,1,0);
						return TRUE;
					}
				}

				// здесь - здесь мы нажимали "Нет", ну а на нет и суда нет
				//  и значит очистим поле ввода.
				strKeyText.Clear();
			}
		}
#else
		// если УЖЕ есть такой макрос...
		int Index;
		if ((Index=MacroDlg->GetIndex((int)key,strKey,KMParam->Mode,true,true)) != -1)
		{
			MacroRecord *Mac=MacroDlg->MacroLIB+Index;

			// общие макросы учитываем только при удалении.
			if (!MacroDlg->RecBuffer || !MacroDlg->RecBufferSize || (Mac->Flags&0xFF)!=MACRO_COMMON)
			{
				string strBufKey;
				if (Mac->Src )
				{
					strBufKey=Mac->Src;
					InsertQuote(strBufKey);
				}

				DWORD DisFlags=Mac->Flags&MFLAGS_DISABLEMACRO;
				LangString strBuf;
				if ((Mac->Flags&MFLAGS_MODEMASK)==MACRO_COMMON)
				{
					strBuf = !MacroDlg->RecBufferSize? (DisFlags? MMacroCommonDeleteAssign : MMacroCommonDeleteKey) : MMacroCommonReDefinedKey;
				}
				else
				{
					strBuf = !MacroDlg->RecBufferSize? (DisFlags?MMacroDeleteAssign : MMacroDeleteKey) : MMacroReDefinedKey;
				}
				strBuf << strKeyText;

				// проверим "а не совпадает ли всё?"
				int Result=0;
				bool SetChange=!MacroDlg->RecBufferSize;
				if (!(!DisFlags &&
				        Mac->Buffer && MacroDlg->RecBuffer &&
				        Mac->BufferSize == MacroDlg->RecBufferSize &&
				        (
				            (Mac->BufferSize >  1 && !memcmp(Mac->Buffer,MacroDlg->RecBuffer,MacroDlg->RecBufferSize*sizeof(DWORD))) ||
				            (Mac->BufferSize == 1 && (DWORD)(DWORD_PTR)Mac->Buffer == (DWORD)(DWORD_PTR)MacroDlg->RecBuffer)
				        )
				   ))
				{
					const wchar_t* NoKey=MSG(DisFlags && !SetChange?MMacroDisAnotherKey:MNo);
					Result=Message(MSG_WARNING,SetChange?3:2,MSG(MWarning),
					          strBuf,
					          MSG(MMacroSequence),
					          strBufKey,
					          MSG(SetChange?MMacroDeleteKey2:
					              (DisFlags?MMacroDisDisabledKey:MMacroReDefinedKey2)),
					          MSG(DisFlags && !SetChange?MMacroDisOverwrite:MYes),
					          (SetChange?MSG(MMacroEditKey):NoKey),
					          (!SetChange?nullptr:NoKey));
				}

				if (!Result)
				{
					if (DisFlags)
					{
						// удаляем из DB только если включен автосейв
						if (Opt.AutoSaveSetup)
						{
							MacroCfg->BeginTransaction();
							// удалим старую запись из DB
							string strKeyName;
							KeyToText(Mac->Key, strKeyName);
							MacroCfg->DeleteKeyMacro(GetAreaName(Mac->Flags&MFLAGS_MODEMASK), strKeyName);
							MacroCfg->EndTransaction();
						}
						// раздисаблим
						Mac->Flags&=~(MFLAGS_DISABLEMACRO|MFLAGS_NEEDSAVEMACRO);
					}

					// в любом случае - вываливаемся
					SendDlgMessage(hDlg,DM_CLOSE,1,0);
					return TRUE;
				}
				else if (SetChange && Result == 1)
				{
					string strDescription;

					if ( Mac->Src )
						strBufKey=Mac->Src;

					if ( Mac->Description )
						strDescription=Mac->Description;

					if (MacroDlg->GetMacroSettings(key,Mac->Flags,strBufKey,strDescription))
					{
						KMParam->Flags = Mac->Flags;
						KMParam->Changed = true;
						// в любом случае - вываливаемся
						SendDlgMessage(hDlg,DM_CLOSE,1,0);
						return TRUE;
					}
				}

				// здесь - здесь мы нажимали "Нет", ну а на нет и суда нет
				//  и значит очистим поле ввода.
				strKeyText.Clear();
			}
		}
#endif

		KMParam->Recurse++;
		SendDlgMessage(hDlg,DM_SETTEXTPTR,2,const_cast<wchar_t*>(strKeyText.CPtr()));
		KMParam->Recurse--;
		//if(key == KEY_F1 && LastKey == KEY_F1)
		//LastKey=-1;
		//else
		LastKey=(int)key;
		return TRUE;
	}
	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

int KeyMacro::AssignMacroKey(DWORD &MacroKey, UINT64 &Flags)
{
	/*
	  +------ Define macro ------+
	  | Press the desired key    |
	  | ________________________ |
	  +--------------------------+
	*/
	FarDialogItem MacroAssignDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,30,4,0,nullptr,nullptr,0,MSG(MDefineMacroTitle)},
		{DI_TEXT,-1,2,0,2,0,nullptr,nullptr,0,MSG(MDefineMacro)},
		{DI_COMBOBOX,5,3,28,3,0,nullptr,nullptr,DIF_FOCUS|DIF_DEFAULTBUTTON,L""},
	};
	MakeDialogItemsEx(MacroAssignDlgData,MacroAssignDlg);
	DlgParam Param={Flags, this, 0, StartMode, 0, false};
	//_SVS(SysLog(L"StartMode=%d",StartMode));
	IsProcessAssignMacroKey++;
	Dialog Dlg(MacroAssignDlg,ARRAYSIZE(MacroAssignDlg),AssignMacroDlgProc,&Param);
	Dlg.SetPosition(-1,-1,34,6);
	Dlg.SetHelp(L"KeyMacro");
	Dlg.Process();
	IsProcessAssignMacroKey--;

	if (Dlg.GetExitCode() == -1)
		return 0;

	MacroKey = Param.Key;
	Flags = Param.Flags;
	return Param.Changed ? 2 : 1;
}

#ifdef FAR_LUA
#else
static int Set3State(DWORD Flags,DWORD Chk1,DWORD Chk2)
{
	DWORD Chk12=Chk1|Chk2, FlagsChk12=Flags&Chk12;

	if (FlagsChk12 == Chk12 || !FlagsChk12)
		return (2);
	else
		return (Flags&Chk1?1:0);
}

enum MACROSETTINGSDLG
{
	MS_DOUBLEBOX,
	MS_TEXT_SEQUENCE,
	MS_EDIT_SEQUENCE,
	MS_TEXT_DESCR,
	MS_EDIT_DESCR,
	MS_SEPARATOR1,
	MS_CHECKBOX_OUPUT,
	MS_CHECKBOX_START,
	MS_SEPARATOR2,
	MS_CHECKBOX_A_PANEL,
	MS_CHECKBOX_A_PLUGINPANEL,
	MS_CHECKBOX_A_FOLDERS,
	MS_CHECKBOX_A_SELECTION,
	MS_CHECKBOX_P_PANEL,
	MS_CHECKBOX_P_PLUGINPANEL,
	MS_CHECKBOX_P_FOLDERS,
	MS_CHECKBOX_P_SELECTION,
	MS_SEPARATOR3,
	MS_CHECKBOX_CMDLINE,
	MS_CHECKBOX_SELBLOCK,
	MS_SEPARATOR4,
	MS_BUTTON_OK,
	MS_BUTTON_CANCEL,
};

intptr_t WINAPI KeyMacro::ParamMacroDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	static DlgParam *KMParam=nullptr;

	switch (Msg)
	{
		case DN_INITDIALOG:
			KMParam=(DlgParam *)Param2;
			break;
		case DN_BTNCLICK:

			if (Param1==MS_CHECKBOX_A_PANEL || Param1==MS_CHECKBOX_P_PANEL)
				for (int i=1; i<=3; i++)
					SendDlgMessage(hDlg,DM_ENABLE,Param1+i,Param2);

			break;
		case DN_CLOSE:

			if (Param1==MS_BUTTON_OK)
			{
				MacroRecord mr={};
				KeyMacro *Macro=KMParam->Handle;
				LPCWSTR Sequence=(LPCWSTR)SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,MS_EDIT_SEQUENCE,0);

				if (*Sequence)
				{
					if (Macro->ParseMacroString(&mr,Sequence))
					{
						xf_free(Macro->RecBuffer);
						Macro->RecBufferSize=mr.BufferSize;
						Macro->RecBuffer=mr.Buffer;
						Macro->RecSrc=xf_wcsdup(Sequence);
						LPCWSTR Description=(LPCWSTR)SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,MS_EDIT_DESCR,0);
						Macro->RecDescription=xf_wcsdup(Description);
						return TRUE;
					}
				}

				return FALSE;
			}

			break;

		default:
			break;
	}

#if 0
	else if (Msg==DN_KEY && Param2==KEY_ALTF4)
	{
		KeyMacro *MacroDlg=KMParam->Handle;
		(*FrameManager)[0]->UnlockRefresh();
		FILE *MacroFile;
		char MacroFileName[NM];

		if (!FarMkTempEx(MacroFileName) || !(MacroFile=fopen(MacroFileName,"wb")))
			return TRUE;

		char *TextBuffer;
		DWORD Buf[1];
		Buf[0]=MacroDlg->RecBuffer[0];

		if ((TextBuffer=MacroDlg->MkTextSequence((MacroDlg->RecBufferSize==1?Buf:MacroDlg->RecBuffer),MacroDlg->RecBufferSize)) )
		{
			fwrite(TextBuffer,strlen(TextBuffer),1,MacroFile);
			fclose(MacroFile);
			xf_free(TextBuffer);
			{
				//ConsoleTitle *OldTitle=new ConsoleTitle;
				FileEditor ShellEditor(MacroFileName,-1,FFILEEDIT_DISABLEHISTORY,-1,-1,nullptr);
				//delete OldTitle;
				ShellEditor.SetDynamicallyBorn(false);
				FrameManager->EnterModalEV();
				FrameManager->ExecuteModal();
				FrameManager->ExitModalEV();

				if (!ShellEditor.IsFileChanged() || !(MacroFile=fopen(MacroFileName,"rb")))
					;
				else
				{
					MacroRecord NewMacroWORK2={};
					long FileSize=filelen(MacroFile);
					TextBuffer=(char*)xf_malloc(FileSize);

					if (TextBuffer)
					{
						fread(TextBuffer,FileSize,1,MacroFile);

						if (!MacroDlg->ParseMacroString(&NewMacroWORK2,TextBuffer))
						{
							if (NewMacroWORK2.BufferSize > 1)
								xf_free(NewMacroWORK2.Buffer);
						}
						else
						{
							MacroDlg->RecBuffer=NewMacroWORK2.Buffer;
							MacroDlg->RecBufferSize=NewMacroWORK2.BufferSize;
						}
					}

					fclose(MacroFile);
				}
			}
			FrameManager->ResizeAllFrame();
			FrameManager->PluginCommit();
		}
		else
			fclose(MacroFile);

		remove(MacroFileName);
		return TRUE;
	}

#endif
	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

int KeyMacro::GetMacroSettings(int Key,UINT64 &Flags,const wchar_t *Src,const wchar_t *Descr)
{
	/*
	          1         2         3         4         5         6
	   3456789012345678901234567890123456789012345678901234567890123456789
	 1 г=========== Параметры макрокоманды для 'CtrlP' ==================¬
	 2 | Последовательность:                                             |
	 3 | _______________________________________________________________ |
	 4 | Описание:                                                       |
	 5 | _______________________________________________________________ |
	 6 |-----------------------------------------------------------------|
	 7 | [ ] Разрешить во время выполнения вывод на экран                |
	 8 | [ ] Выполнять после запуска FAR                                 |
	 9 |-----------------------------------------------------------------|
	10 | [ ] Активная панель             [ ] Пассивная панель            |
	11 |   [?] На панели плагина           [?] На панели плагина         |
	12 |   [?] Выполнять для папок         [?] Выполнять для папок       |
	13 |   [?] Отмечены файлы              [?] Отмечены файлы            |
	14 |-----------------------------------------------------------------|
	15 | [?] Пустая командная строка                                     |
	16 | [?] Отмечен блок                                                |
	17 |-----------------------------------------------------------------|
	18 |               [ Продолжить ]  [ Отменить ]                      |
	19 L=================================================================+

	*/
	FarDialogItem MacroSettingsDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,69,19,0,nullptr,nullptr,0,L""},
		{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(MMacroSequence)},
		{DI_EDIT,5,3,67,3,0,L"MacroSequence",nullptr,DIF_FOCUS|DIF_HISTORY,L""},
		{DI_TEXT,5,4,0,4,0,nullptr,nullptr,0,MSG(MMacroDescription)},
		{DI_EDIT,5,5,67,5,0,L"MacroDescription",nullptr,DIF_HISTORY,L""},

		{DI_TEXT,3,6,0,6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,7,0,7,0,nullptr,nullptr,0,MSG(MMacroSettingsEnableOutput)},
		{DI_CHECKBOX,5,8,0,8,0,nullptr,nullptr,0,MSG(MMacroSettingsRunAfterStart)},
		{DI_TEXT,3,9,0,9,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,10,0,10,0,nullptr,nullptr,0,MSG(MMacroSettingsActivePanel)},
		{DI_CHECKBOX,7,11,0,11,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsPluginPanel)},
		{DI_CHECKBOX,7,12,0,12,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsFolders)},
		{DI_CHECKBOX,7,13,0,13,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsSelectionPresent)},
		{DI_CHECKBOX,37,10,0,10,0,nullptr,nullptr,0,MSG(MMacroSettingsPassivePanel)},
		{DI_CHECKBOX,39,11,0,11,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsPluginPanel)},
		{DI_CHECKBOX,39,12,0,12,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsFolders)},
		{DI_CHECKBOX,39,13,0,13,2,nullptr,nullptr,DIF_3STATE|DIF_DISABLE,MSG(MMacroSettingsSelectionPresent)},
		{DI_TEXT,3,14,0,14,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_CHECKBOX,5,15,0,15,2,nullptr,nullptr,DIF_3STATE,MSG(MMacroSettingsCommandLine)},
		{DI_CHECKBOX,5,16,0,16,2,nullptr,nullptr,DIF_3STATE,MSG(MMacroSettingsSelectionBlockPresent)},
		{DI_TEXT,3,17,0,17,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,0,18,0,18,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MCancel)},
	};
	MakeDialogItemsEx(MacroSettingsDlgData,MacroSettingsDlg);
	string strKeyText;
	KeyToText(Key,strKeyText);
	MacroSettingsDlg[MS_DOUBLEBOX].strData = LangString(MMacroSettingsTitle) << strKeyText;
	//if(!(Key&0x7F000000))
	//MacroSettingsDlg[3].Flags|=DIF_DISABLE;
	MacroSettingsDlg[MS_CHECKBOX_OUPUT].Selected=Flags&MFLAGS_DISABLEOUTPUT?0:1;
	MacroSettingsDlg[MS_CHECKBOX_START].Selected=Flags&MFLAGS_RUNAFTERFARSTART?1:0;
	MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected=Set3State(Flags,MFLAGS_NOFILEPANELS,MFLAGS_NOPLUGINPANELS);
	MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected=Set3State(Flags,MFLAGS_NOFILES,MFLAGS_NOFOLDERS);
	MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected=Set3State(Flags,MFLAGS_SELECTION,MFLAGS_NOSELECTION);
	MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected=Set3State(Flags,MFLAGS_PNOFILEPANELS,MFLAGS_PNOPLUGINPANELS);
	MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected=Set3State(Flags,MFLAGS_PNOFILES,MFLAGS_PNOFOLDERS);
	MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected=Set3State(Flags,MFLAGS_PSELECTION,MFLAGS_PNOSELECTION);
	MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected=Set3State(Flags,MFLAGS_EMPTYCOMMANDLINE,MFLAGS_NOTEMPTYCOMMANDLINE);
	MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected=Set3State(Flags,MFLAGS_EDITSELECTION,MFLAGS_EDITNOSELECTION);
	if (Src && *Src)
	{
		MacroSettingsDlg[MS_EDIT_SEQUENCE].strData=Src;
	}
	else
	{
		LPWSTR Sequence=MkTextSequence(RecBuffer,RecBufferSize);
		MacroSettingsDlg[MS_EDIT_SEQUENCE].strData=Sequence;
		xf_free(Sequence);
	}

	MacroSettingsDlg[MS_EDIT_DESCR].strData=(Descr && *Descr)?Descr:RecDescription;

	DlgParam Param={0, this, 0, 0, 0, false};
	Dialog Dlg(MacroSettingsDlg,ARRAYSIZE(MacroSettingsDlg),ParamMacroDlgProc,&Param);
	Dlg.SetPosition(-1,-1,73,21);
	Dlg.SetHelp(L"KeyMacroSetting");
	Frame* BottomFrame = FrameManager->GetBottomFrame();
	if(BottomFrame)
	{
		BottomFrame->Lock(); // отменим прорисовку фрейма
	}
	Dlg.Process();
	if(BottomFrame)
	{
		BottomFrame->Unlock(); // теперь можно :-)
	}

	if (Dlg.GetExitCode()!=MS_BUTTON_OK)
		return FALSE;

	Flags=MacroSettingsDlg[MS_CHECKBOX_OUPUT].Selected?0:MFLAGS_DISABLEOUTPUT;
	Flags|=MacroSettingsDlg[MS_CHECKBOX_START].Selected?MFLAGS_RUNAFTERFARSTART:0;

	if (MacroSettingsDlg[MS_CHECKBOX_A_PANEL].Selected)
	{
		Flags|=MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected==0?MFLAGS_NOPLUGINPANELS:MFLAGS_NOFILEPANELS);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected==0?MFLAGS_NOFOLDERS:MFLAGS_NOFILES);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected==0?MFLAGS_NOSELECTION:MFLAGS_SELECTION);
	}

	if (MacroSettingsDlg[MS_CHECKBOX_P_PANEL].Selected)
	{
		Flags|=MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected==0?MFLAGS_PNOPLUGINPANELS:MFLAGS_PNOFILEPANELS);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected==0?MFLAGS_PNOFOLDERS:MFLAGS_PNOFILES);
		Flags|=MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected==2?0:
		       (MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected==0?MFLAGS_PNOSELECTION:MFLAGS_PSELECTION);
	}

	Flags|=MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected==2?0:
	       (MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected==0?MFLAGS_NOTEMPTYCOMMANDLINE:MFLAGS_EMPTYCOMMANDLINE);
	Flags|=MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected==2?0:
	       (MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected==0?MFLAGS_EDITNOSELECTION:MFLAGS_EDITSELECTION);
	return TRUE;
}

int KeyMacro::PostNewMacro(const wchar_t *PlainText,UINT64 Flags,DWORD AKey,bool onlyCheck)
{
	MacroRecord NewMacroWORK2={};
	wchar_t *Buffer=(wchar_t *)PlainText;
	bool allocBuffer=false;

	// сначала смотрим на парсер
	BOOL parsResult=ParseMacroString(&NewMacroWORK2,Buffer,onlyCheck);

	if (allocBuffer && Buffer)
		xf_free(Buffer);

	if (!parsResult)
	{
		if (NewMacroWORK2.BufferSize > 1)
			xf_free(NewMacroWORK2.Buffer);

		return FALSE;
	}

	if (onlyCheck)
	{
		if (NewMacroWORK2.BufferSize > 1)
			xf_free(NewMacroWORK2.Buffer);

		return TRUE;
	}

	NewMacroWORK2.Flags=Flags;
	NewMacroWORK2.Key=AKey;
	// теперь попробуем выделить немного нужной памяти
	MacroRecord *NewMacroWORK;

	if (!(NewMacroWORK=(MacroRecord *)xf_realloc(Work.MacroWORK,sizeof(MacroRecord)*(Work.MacroWORKCount+1))))
	{
		if (NewMacroWORK2.BufferSize > 1)
			xf_free(NewMacroWORK2.Buffer);

		return FALSE;
	}

	// теперь добавим в нашу "очередь" новые данные
	Work.MacroWORK=NewMacroWORK;
	NewMacroWORK=Work.MacroWORK+Work.MacroWORKCount;
	*NewMacroWORK=NewMacroWORK2;
	Work.MacroWORKCount++;

	//Work.Executing=Work.MacroWORK[0].Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
	if (Work.ExecLIBPos > Work.MacroWORK[0].BufferSize)
		Work.ExecLIBPos=0;

	return TRUE;
}

int KeyMacro::PostNewMacro(MacroRecord *MRec,BOOL NeedAddSendFlag,bool IsPluginSend)
{
	if (!MRec)
		return FALSE;

	MacroRecord NewMacroWORK2=*MRec;
	NewMacroWORK2.Src=nullptr;
	NewMacroWORK2.Name=nullptr; //???
	NewMacroWORK2.Description=nullptr;
	//if(MRec->BufferSize > 1)
	{
		if (!(NewMacroWORK2.Buffer=(DWORD*)xf_malloc((MRec->BufferSize+3)*sizeof(DWORD))))
		{
			return FALSE;
		}
	}
	// теперь попробуем выделить немного нужной памяти
	MacroRecord *NewMacroWORK;

	if (!(NewMacroWORK=(MacroRecord *)xf_realloc(Work.MacroWORK,sizeof(MacroRecord)*(Work.MacroWORKCount+1))))
	{
		//if(MRec->BufferSize > 1)
		xf_free(NewMacroWORK2.Buffer);
		return FALSE;
	}

	// теперь добавим в нашу "очередь" новые данные
	if (IsPluginSend)
		NewMacroWORK2.Buffer[0]=MCODE_OP_KEYS;

	if ((MRec->BufferSize+1) > 2)
		memcpy(&NewMacroWORK2.Buffer[IsPluginSend?1:0],MRec->Buffer,sizeof(DWORD)*MRec->BufferSize);
	else if (MRec->Buffer)
		NewMacroWORK2.Buffer[IsPluginSend?1:0]=(DWORD)(intptr_t)MRec->Buffer;

	if (IsPluginSend)
		NewMacroWORK2.Buffer[NewMacroWORK2.BufferSize+1]=MCODE_OP_ENDKEYS;

	//NewMacroWORK2.Buffer[NewMacroWORK2.BufferSize]=MCODE_OP_NOP; // доп.клавиша/пустышка

	if (IsPluginSend)
		NewMacroWORK2.BufferSize+=2;

	Work.MacroWORK=NewMacroWORK;
	NewMacroWORK=Work.MacroWORK+Work.MacroWORKCount;
	*NewMacroWORK=NewMacroWORK2;
	Work.MacroWORKCount++;

	//Work.Executing=Work.MacroWORK[0].Flags&MFLAGS_NOSENDKEYSTOPLUGINS?MACROMODE_EXECUTING:MACROMODE_EXECUTING_COMMON;
	if (Work.ExecLIBPos > Work.MacroWORK[0].BufferSize)
		Work.ExecLIBPos=0;

	return TRUE;
}

int KeyMacro::ParseMacroString(MacroRecord *CurMacro,const wchar_t *BufPtr,bool onlyCheck)
{
	BOOL Result=FALSE;

	if (CurMacro)
	{
		Result=__parseMacroString(CurMacro->Buffer, CurMacro->BufferSize, BufPtr);

		if (!Result && !onlyCheck)
		{
			// TODO: ЭТОТ КУСОК ДОЛЖЕН ПРЕДПОЛАГАТЬ ВОЗМОЖНОСТЬ РЕЖИМА SILENT!
			bool scrLocks=LockScr!=nullptr;
			string ErrMsg[4];

			if (scrLocks) // если был - удалим
			{
				if (LockScr) delete LockScr;

				LockScr=nullptr;
			}

			InternalInput++; // InternalInput - ограничитель того, чтобы макрос не продолжал свое исполнение
			GetMacroParseError(&ErrMsg[0],&ErrMsg[1],&ErrMsg[2],&ErrMsg[3]);
			//if(...)
			string strTitle=MSG(MMacroPErrorTitle);
			if(CurMacro->Key)
			{
				strTitle+=L" ";
				string strKey;
				KeyToText(CurMacro->Key,strKey);
				strTitle.Append(GetAreaName(CurMacro->Flags&MFLAGS_MODEMASK)).Append(L"\\").Append(strKey);
			}
			Message(MSG_WARNING|MSG_LEFTALIGN,1,strTitle,ErrMsg[3]+L":",ErrMsg[0],L"\x1",ErrMsg[1],ErrMsg[2],L"\x1",MSG(MOk));
			//else
			// вывести диагностику в файл
			InternalInput--;

			if (scrLocks) // если стал - залочим
			{
				if (LockScr) delete LockScr;

				LockScr=new LockScreen;
			}
		}
	}

	return Result;
}


void MacroState::Init(TVarTable *tbl)
{
	KeyProcess=Executing=MacroPC=ExecLIBPos=MacroWORKCount=0;
	HistoryDisable=0;
	MacroWORK=nullptr;

	if (!tbl)
	{
		AllocVarTable=true;
		locVarTable=(TVarTable*)xf_malloc(sizeof(TVarTable));
		initVTable(*locVarTable);
	}
	else
	{
		AllocVarTable=false;
		locVarTable=tbl;
	}
}

int KeyMacro::PushState(bool CopyLocalVars)
{
	_KEYMACRO(CleverSysLog Clev(L"KeyMacro::PushState()"));
	if (CurPCStack+1 >= STACKLEVEL)
		return FALSE;

	++CurPCStack;
	Work.UseInternalClipboard=Clipboard::GetUseInternalClipboardState();
	PCStack[CurPCStack]=Work;
	Work.Init(CopyLocalVars?PCStack[CurPCStack].locVarTable:nullptr);
	_KEYMACRO(SysLog(L"CurPCStack=%d",CurPCStack));
	return TRUE;
}

int KeyMacro::PopState()
{
	_KEYMACRO(CleverSysLog Clev(L"KeyMacro::PopState()"));
	if (CurPCStack < 0)
		return FALSE;

	Work=PCStack[CurPCStack];
	Clipboard::SetUseInternalClipboardState(Work.UseInternalClipboard);
	CurPCStack--;
	_KEYMACRO(SysLog(L"CurPCStack=%d",CurPCStack));
	return TRUE;
}

// Функция получения индекса нужного макроса в массиве
// Ret=-1 - не найден таковой.
// если CheckMode=-1 - значит пофигу в каком режиме, т.е. первый попавшийся
// StrictKeys=true - не пытаться подменить Левый Ctrl/Alt Правым (если Левый не нашли)
int KeyMacro::GetIndex(int Key, string& strKey, int CheckMode, bool UseCommon, bool StrictKeys)
{
	if (MacroLIB)
	{
		int KeyParam=Key;
		for (int I=0; I < 2; ++I)
		{
			int Len;
			MacroRecord *MPtr=nullptr;
			Key=KeyParam;

			if (CheckMode == -1)
			{
				Len=MacroLIBCount;
				MPtr=MacroLIB;
			}
			else if (CheckMode >= 0 && CheckMode < MACRO_LAST)
			{
				Len=IndexMode[CheckMode][1];

				if (Len)
					MPtr=MacroLIB+IndexMode[CheckMode][0];

				//_SVS(SysLog(L"CheckMode=%d (%d,%d)",CheckMode,IndexMode[CheckMode][0],IndexMode[CheckMode][1]));
			}
			else
			{
				Len=0;
			}

			if (Len)
			{
				int ctrl = 0;
				if (Key != -1)
					ctrl =(!StrictKeys && (Key&(KEY_RCTRL|KEY_RALT)) && !(Key&(KEY_CTRL|KEY_ALT))) ? 0 : 1;
				MacroRecord *MPtrSave=MPtr;

				for (; ctrl < 2; ctrl++)
				{
					for (int Pos=0; Pos < Len; ++Pos, ++MPtr)
					{
						if (Key != -1)
						{
							if (!((MPtr->Key ^ Key) & ~0xFFFF) &&
							        (Upper(static_cast<WCHAR>(MPtr->Key))==Upper(static_cast<WCHAR>(Key))) &&
							        (MPtr->BufferSize > 0))
							{
								//        && (CheckMode == -1 || (MPtr->Flags&MFLAGS_MODEMASK) == CheckMode))
								//_SVS(SysLog(L"GetIndex: Pos=%d MPtr->Key=0x%08X", Pos,MPtr->Key));
								if (!(MPtr->Flags&MFLAGS_DISABLEMACRO))
								{
								    if(!MPtr->Callback||MPtr->Callback(MPtr->Id,AKMFLAGS_NONE))
								    	return Pos+((CheckMode >= 0)?IndexMode[CheckMode][0]:0);
								}
							}
						}
						else if (!strKey.IsEmpty() && !StrCmpI(strKey,MPtr->Name))
						{
							if (MPtr->BufferSize > 0)
							{
								if (!(MPtr->Flags&MFLAGS_DISABLEMACRO))
								{
								    if(!MPtr->Callback||MPtr->Callback(MPtr->Id,AKMFLAGS_NONE))
								    	return Pos+((CheckMode >= 0)?IndexMode[CheckMode][0]:0);
								}
							}
						}
					}

					if (Key != -1)
					{
						if (!ctrl)
						{
							if (Key & KEY_RCTRL)
							{
								Key &= ~KEY_RCTRL;
								Key |= KEY_CTRL;
							}
							if (Key & KEY_RALT)
							{
								Key &= ~KEY_RALT;
								Key |= KEY_ALT;
							}
							MPtr = MPtrSave;
						}
					}
					else
						MPtr = MPtrSave;
				}
			}

			// здесь смотрим на MACRO_COMMON
			if (CheckMode != -1 && !I && UseCommon)
				CheckMode=MACRO_COMMON;
			else
				break;
		}
	}

	return -1;
}

#if 0
// получение размера, занимаемого указанным макросом
// Ret= 0 - не найден таковой.
// если CheckMode=-1 - значит пофигу в каком режиме, т.е. первый попавшийся
int KeyMacro::GetRecordSize(int Key, int CheckMode)
{
	//BUGBUG: StrictKeys?
	string strKey;
	int Pos=GetIndex(Key,strKey,CheckMode);

	if (Pos == -1)
		return 0;

	return sizeof(MacroRecord)+MacroLIB[Pos].BufferSize;
}
#endif

// получить название моды по коду
const wchar_t* KeyMacro::GetAreaName(int AreaCode)
{
	return (AreaCode >= MACRO_FUNCS && AreaCode < MACRO_LAST)?MKeywordsArea[AreaCode+3].Name:L"";
}

// получить код моды по имени
int KeyMacro::GetAreaCode(const wchar_t *AreaName)
{
	for (int i=MACRO_FUNCS; i < MACRO_LAST; i++)
		if (!StrCmpI(MKeywordsArea[i+3].Name,AreaName))
			return i;

	return MACRO_FUNCS-1;
}

int KeyMacro::GetMacroKeyInfo(bool FromDB, int Mode, int Pos, string &strKeyName, string &strDescription)
{
	if (Mode >= MACRO_FUNCS && Mode < MACRO_LAST)
	{
		if (FromDB)
		{
			if (Mode >= MACRO_OTHER)
			{
				// TODO
				return Pos+1;
			}
			else if (Mode == MACRO_FUNCS)
			{
				// TODO: MACRO_FUNCS
				return -1;
			}
			else
			{
				// TODO
				return Pos+1;
			}
		}
		else
		{
			if (Mode >= MACRO_OTHER)
			{
				int Len=CtrlObject->Macro.IndexMode[Mode][1];

				if (Len && Pos < Len)
				{
					MacroRecord *MPtr=CtrlObject->Macro.MacroLIB+CtrlObject->Macro.IndexMode[Mode][0]+Pos;
					if (MPtr->Key != -1)
						::KeyToText(MPtr->Key,strKeyName);
					else
						strKeyName=MPtr->Name;
					strDescription=NullToEmpty(MPtr->Description);
					return Pos+1;
				}
			}
			else if (Mode == MACRO_FUNCS)
			{
				// TODO: MACRO_FUNCS
				return -1;
			}
			else
			{
				TVarSet *var=varEnum((Mode==MACRO_VARS)?glbVarTable:glbConstTable,Pos);

				if (!var)
					return -1;

				strKeyName = var->str;
				strKeyName = (Mode==MACRO_VARS?L"%":L"")+strKeyName;

				switch (var->value.type())
				{
					case vtInteger:
					{
						__int64 IData64=var->value.i();
						strDescription.Format(L"%I64d (0x%I64X)", IData64, IData64);
						break;
					}
					case vtDouble:
					{
						double FData=var->value.d();
						strDescription.Format(L"%g", FData);
						break;
					}
					case vtString:
						strDescription.Format(L"\"%s\"", var->value.s());
						break;
					default:
						break;
				}

				return Pos+1;
			}
		}
	}

	return -1;
}

BOOL KeyMacro::CheckEditSelected(UINT64 CurFlags)
{
	if (Mode==MACRO_EDITOR || Mode==MACRO_DIALOG || Mode==MACRO_VIEWER || (Mode==MACRO_SHELL&&CtrlObject->CmdLine->IsVisible()))
	{
		int NeedType = Mode == MACRO_EDITOR?MODALTYPE_EDITOR:(Mode == MACRO_VIEWER?MODALTYPE_VIEWER:(Mode == MACRO_DIALOG?MODALTYPE_DIALOG:MODALTYPE_PANELS));
		Frame* CurFrame=FrameManager->GetCurrentFrame();

		if (CurFrame && CurFrame->GetType()==NeedType)
		{
			int CurSelected;

			if (Mode==MACRO_SHELL && CtrlObject->CmdLine->IsVisible())
				CurSelected=(int)CtrlObject->CmdLine->VMProcess(MCODE_C_SELECTED);
			else
				CurSelected=(int)CurFrame->VMProcess(MCODE_C_SELECTED);

			if (((CurFlags&MFLAGS_EDITSELECTION) && !CurSelected) ||	((CurFlags&MFLAGS_EDITNOSELECTION) && CurSelected))
				return FALSE;
		}
	}

	return TRUE;
}

BOOL KeyMacro::CheckInsidePlugin(UINT64 CurFlags)
{
	if (CtrlObject && CtrlObject->Plugins->CurPluginItem && (CurFlags&MFLAGS_NOSENDKEYSTOPLUGINS)) // ?????
		//if(CtrlObject && CtrlObject->Plugins->CurEditor && (CurFlags&MFLAGS_NOSENDKEYSTOPLUGINS))
		return FALSE;

	return TRUE;
}

BOOL KeyMacro::CheckCmdLine(int CmdLength,UINT64 CurFlags)
{
	if (((CurFlags&MFLAGS_EMPTYCOMMANDLINE) && CmdLength) || ((CurFlags&MFLAGS_NOTEMPTYCOMMANDLINE) && CmdLength==0))
		return FALSE;

	return TRUE;
}

BOOL KeyMacro::CheckPanel(int PanelMode,UINT64 CurFlags,BOOL IsPassivePanel)
{
	if (IsPassivePanel)
	{
		if ((PanelMode == PLUGIN_PANEL && (CurFlags&MFLAGS_PNOPLUGINPANELS)) || (PanelMode == NORMAL_PANEL && (CurFlags&MFLAGS_PNOFILEPANELS)))
			return FALSE;
	}
	else
	{
		if ((PanelMode == PLUGIN_PANEL && (CurFlags&MFLAGS_NOPLUGINPANELS)) || (PanelMode == NORMAL_PANEL && (CurFlags&MFLAGS_NOFILEPANELS)))
			return FALSE;
	}

	return TRUE;
}

BOOL KeyMacro::CheckFileFolder(Panel *CheckPanel,UINT64 CurFlags, BOOL IsPassivePanel)
{
	string strFileName;
	DWORD FileAttr=INVALID_FILE_ATTRIBUTES;
	CheckPanel->GetFileName(strFileName,CheckPanel->GetCurrentPos(),FileAttr);

	if (FileAttr != INVALID_FILE_ATTRIBUTES)
	{
		if (IsPassivePanel)
		{
			if (((FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_PNOFOLDERS)) || (!(FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_PNOFILES)))
				return FALSE;
		}
		else
		{
			if (((FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_NOFOLDERS)) || (!(FileAttr&FILE_ATTRIBUTE_DIRECTORY) && (CurFlags&MFLAGS_NOFILES)))
				return FALSE;
		}
	}

	return TRUE;
}

BOOL KeyMacro::CheckAll(int /*CheckMode*/,UINT64 CurFlags)
{
	/* $TODO:
		Здесь вместо Check*() попробовать заюзать IfCondition()
		для исключения повторяющегося кода.
	*/
	if (!CheckInsidePlugin(CurFlags))
		return FALSE;

	// проверка на пусто/не пусто в ком.строке (а в редакторе? :-)
	if (CurFlags&(MFLAGS_EMPTYCOMMANDLINE|MFLAGS_NOTEMPTYCOMMANDLINE))
		if (CtrlObject->CmdLine && !CheckCmdLine(CtrlObject->CmdLine->GetLength(),CurFlags))
			return FALSE;

	FilePanels *Cp=CtrlObject->Cp();

	if (!Cp)
		return FALSE;

	// проверки панели и типа файла
	Panel *ActivePanel=Cp->ActivePanel;
	Panel *PassivePanel=Cp->GetAnotherPanel(Cp->ActivePanel);

	if (ActivePanel && PassivePanel)// && (CurFlags&MFLAGS_MODEMASK)==MACRO_SHELL)
	{
		if (CurFlags&(MFLAGS_NOPLUGINPANELS|MFLAGS_NOFILEPANELS))
			if (!CheckPanel(ActivePanel->GetMode(),CurFlags,FALSE))
				return FALSE;

		if (CurFlags&(MFLAGS_PNOPLUGINPANELS|MFLAGS_PNOFILEPANELS))
			if (!CheckPanel(PassivePanel->GetMode(),CurFlags,TRUE))
				return FALSE;

		if (CurFlags&(MFLAGS_NOFOLDERS|MFLAGS_NOFILES))
			if (!CheckFileFolder(ActivePanel,CurFlags,FALSE))
				return FALSE;

		if (CurFlags&(MFLAGS_PNOFOLDERS|MFLAGS_PNOFILES))
			if (!CheckFileFolder(PassivePanel,CurFlags,TRUE))
				return FALSE;

		if (CurFlags&(MFLAGS_SELECTION|MFLAGS_NOSELECTION|MFLAGS_PSELECTION|MFLAGS_PNOSELECTION))
			if (Mode!=MACRO_EDITOR && Mode != MACRO_DIALOG && Mode!=MACRO_VIEWER)
			{
				size_t SelCount=ActivePanel->GetRealSelCount();

				if (((CurFlags&MFLAGS_SELECTION) && SelCount < 1) || ((CurFlags&MFLAGS_NOSELECTION) && SelCount >= 1))
					return FALSE;

				SelCount=PassivePanel->GetRealSelCount();

				if (((CurFlags&MFLAGS_PSELECTION) && SelCount < 1) || ((CurFlags&MFLAGS_PNOSELECTION) && SelCount >= 1))
					return FALSE;
			}
	}

	if (!CheckEditSelected(CurFlags))
		return FALSE;

	return TRUE;
}

/*
  Return: FALSE - если тестируемый MFLAGS_* не установлен или
                  это не режим исполнения макроса!
          TRUE  - такой флаг(и) установлен(ы)
*/
BOOL KeyMacro::CheckCurMacroFlags(DWORD Flags)
{
	if (Work.Executing && Work.MacroWORK)
	{
		return (Work.MacroWORK[0].Flags&Flags)?TRUE:FALSE;
	}

	return FALSE;
}

/*
  Return: 0 - не в режиме макро, 1 - Executing, 2 - Executing common, 3 - Recording, 4 - Recording common
  See MacroRecordAndExecuteType
*/
int KeyMacro::GetCurRecord(MacroRecord* RBuf,int *KeyPos)
{
	if (KeyPos && RBuf)
	{
		*KeyPos=Work.Executing?Work.ExecLIBPos:0;
		ClearStruct(*RBuf);

		if (Recording == MACROMODE_NOMACRO)
		{
			if (Work.Executing)
			{
				*RBuf=*Work.MacroWORK;   //MacroLIB[Work.MacroPC]; //????
				return Work.Executing;
			}

			ClearStruct(*RBuf);
			return MACROMODE_NOMACRO;
		}

		RBuf->BufferSize=RecBufferSize;
		RBuf->Buffer=RecBuffer;

		return Recording==MACROMODE_RECORDING?MACROMODE_RECORDING:MACROMODE_RECORDING_COMMON;
	}

	return Recording?(Recording==MACROMODE_RECORDING?MACROMODE_RECORDING:MACROMODE_RECORDING_COMMON):(Work.Executing?Work.Executing:MACROMODE_NOMACRO);
}

DWORD KeyMacro::SetHistoryDisableMask(DWORD Mask)
{
	DWORD OldHistoryDisable=Work.HistoryDisable;
	Work.HistoryDisable=Mask;
	return OldHistoryDisable;
}

DWORD KeyMacro::GetHistoryDisableMask()
{
	return Work.HistoryDisable;
}

bool KeyMacro::IsHistoryDisable(int TypeHistory)
{
	return (Work.HistoryDisable & (1 << TypeHistory))? true : false;
}

static int __cdecl SortMacros(const MacroRecord *el1,const MacroRecord *el2)
{
	int result=(el1->Flags&MFLAGS_MODEMASK)-(el2->Flags&MFLAGS_MODEMASK);
	if (result==0)
	{
		result=memcmp(&el1->Guid,&el2->Guid,sizeof(GUID));
		if (result==0)
		{
			result=static_cast<int>(static_cast<char*>(el1->Id)-static_cast<char*>(el2->Id));
}
	}
	return result;
}

// Сортировка элементов списка
void KeyMacro::Sort()
{
	typedef int (__cdecl *qsort_fn)(const void*,const void*);
	// сортируем
	far_qsort(MacroLIB,MacroLIBCount,sizeof(MacroRecord),(qsort_fn)SortMacros);
	// перестраиваем индекс начал
	int CurMode=MACRO_OTHER;
	ClearArray(IndexMode);

	for (int I=0; I<MacroLIBCount; I++)
	{
		int J=MacroLIB[I].Flags&MFLAGS_MODEMASK;

		if (CurMode != J)
		{
			IndexMode[J][0]=I;
			CurMode=J;
		}

		IndexMode[J][1]++;
	}

	//_SVS(for(I=0; I < ARRAYSIZE(IndexMode); ++I)SysLog(L"IndexMode[%02d.%s]=%d,%d",I,GetAreaName(I),IndexMode[I][0],IndexMode[I][1]));
}

DWORD KeyMacro::GetOpCode(MacroRecord *MR,int PC)
{
	DWORD OpCode=(MR->BufferSize > 1)?MR->Buffer[PC]:(DWORD)(intptr_t)MR->Buffer;
	return OpCode;
}

// function for Mantis#0000968
bool KeyMacro::CheckWaitKeyFunc()
{
	if (InternalInput || !Work.MacroWORK || Work.Executing == MACROMODE_NOMACRO)
		return false;

	MacroRecord *MR=Work.MacroWORK;

	if (Work.ExecLIBPos >= MR->BufferSize || Work.ExecLIBPos <= 0)
		return false;

	return (GetOpCode(MR,Work.ExecLIBPos-1) == MCODE_F_WAITKEY)?true:false;
}

// кинуть OpCode в буфер. Возвращает предыдущее значение
DWORD KeyMacro::SetOpCode(MacroRecord *MR,int PC,DWORD OpCode)
{
	DWORD OldOpCode;

	if (MR->BufferSize > 1)
	{
		OldOpCode=MR->Buffer[PC];
		MR->Buffer[PC]=OpCode;
	}
	else
	{
		OldOpCode=(DWORD)(intptr_t)MR->Buffer;
		MR->Buffer=(DWORD*)(intptr_t)OpCode;
	}

	return OldOpCode;
}

// Вот это лечит вот ЭТО:
// BugZ#873 - ACTL_POSTKEYSEQUENCE и заголовок окна
int KeyMacro::IsExecutingLastKey()
{
	if (Work.Executing && Work.MacroWORK)
	{
		return (Work.ExecLIBPos == Work.MacroWORK[0].BufferSize-1);
	}

	return FALSE;
}


void KeyMacro::SendDropProcess()
{
	if (Work.Executing)
		StopMacro=true;
}

void KeyMacro::DropProcess()
{
	if (Work.Executing)
	{
		if (LockScr) delete LockScr;

		LockScr=nullptr;
		Clipboard::SetUseInternalClipboardState(false); //??
		Work.Executing=MACROMODE_NOMACRO;
		ReleaseWORKBuffer();
	}
}

bool checkMacroConst(const wchar_t *name)
{
	return !varLook(glbConstTable, name)?false:true;
}

void initMacroVarTable(int global)
{
	if (global)
	{
		initVTable(glbVarTable);
		initVTable(glbConstTable); //???
	}
}

void doneMacroVarTable(int global)
{
	if (global)
	{
		deleteVTable(glbVarTable);
		deleteVTable(glbConstTable); //???
	}
}

BOOL KeyMacro::GetMacroParseError(DWORD* ErrCode, COORD* ErrPos, string *ErrSrc)
{
	return __getMacroParseError(ErrCode,ErrPos,ErrSrc);
}

BOOL KeyMacro::GetMacroParseError(string *Err1, string *Err2, string *Err3, string *Err4)
{
	return __getMacroParseError(Err1, Err2, Err3, Err4);
}

// это OpCode (за исключением MCODE_OP_ENDKEYS)?
bool KeyMacro::IsOpCode(DWORD p)
{
	return (!(p&KEY_MACRO_BASE) || p == MCODE_OP_ENDKEYS)?false:true;
}

int KeyMacro::AddMacro(const wchar_t *PlainText,const wchar_t *Description,enum MACROMODEAREA Area,MACROFLAGS_MFLAGS Flags,const INPUT_RECORD& AKey,const GUID& PluginId,void* Id,FARMACROCALLBACK Callback)
{
	if (Area < MACRO_OTHER || Area > MACRO_COMMON)
		return FALSE;

	MacroRecord CurMacro={};
	CurMacro.Flags=Area;
	CurMacro.Flags|=Flags;
	CurMacro.Key=InputRecordToKey(&AKey);
	string strKeyText;
	if (KeyToText(CurMacro.Key, strKeyText))
		CurMacro.Name=xf_wcsdup(strKeyText);
	else
		CurMacro.Name=nullptr;
	CurMacro.Src=xf_wcsdup(PlainText);
	CurMacro.Description=xf_wcsdup(Description);
	CurMacro.Guid=PluginId;
	CurMacro.Id=Id;
	CurMacro.Callback=Callback;
	if (ParseMacroString(&CurMacro,PlainText,false))
	{
		MacroRecord *NewMacroLIB=(MacroRecord *)xf_realloc(MacroLIB,sizeof(*MacroLIB)*(MacroLIBCount+1));
		if (NewMacroLIB)
		{
			MacroLIB=NewMacroLIB;
			MacroLIB[MacroLIBCount]=CurMacro;
			++MacroLIBCount;
			KeyMacro::Sort();
			return TRUE;
		}
	}
	return FALSE;
}

int KeyMacro::DelMacro(const GUID& PluginId,void* Id)
{
	if (MacroLIB)
	{
		for (int Area=MACRO_OTHER; Area < MACRO_LAST; ++Area)
		{
			size_t size=IndexMode[Area][0]+IndexMode[Area][1];
			for(size_t ii=IndexMode[Area][0];ii<size;++ii)
			{
				if(MacroLIB[ii].Id==Id && IsEqualGUID(MacroLIB[ii].Guid,PluginId))
				{
					DelMacro(ii);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

void KeyMacro::DelMacro(size_t Index)
{
	if (MacroLIB[Index].BufferSize > 1 && MacroLIB[Index].Buffer)
		xf_free(MacroLIB[Index].Buffer);
	if (MacroLIB[Index].Src)
		xf_free(MacroLIB[Index].Src);
	if (MacroLIB[Index].Name)
		xf_free(MacroLIB[Index].Name);
	if (MacroLIB[Index].Description)
		xf_free(MacroLIB[Index].Description);
	memcpy(MacroLIB+Index,MacroLIB+Index+1,(MacroLIBCount-Index-1)*sizeof(MacroLIB[0]));
	--MacroLIBCount;
	KeyMacro::Sort();
}

int KeyMacro::GetCurrentCallPluginMode()
{
	int Ret=-1;
	MacroRecord *MR=Work.MacroWORK;
	if (MR)
	{
		Ret=MR->Flags&MFLAGS_CALLPLUGINENABLEMACRO?1:0;
	}
	return Ret;
}

TVarTable *KeyMacro::GetLocalVarTable()
{
	if (CtrlObject)
		return CtrlObject->Macro.Work.locVarTable;

	return nullptr;
}
#endif
