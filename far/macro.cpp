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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "macro.hpp"

// Internal:
#include "uuids.far.hpp"
#include "cmdline.hpp"
#include "config.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "keyboard.hpp"
#include "manager.hpp"
#include "message.hpp"
#include "panel.hpp"
#include "scrbuf.hpp"
#include "syslog.hpp"
#include "macroopcode.hpp"
#include "console.hpp"
#include "pathmix.hpp"
#include "panelmix.hpp"
#include "flink.hpp"
#include "fileedit.hpp"
#include "viewer.hpp"
#include "datetime.hpp"
#include "xlat.hpp"
#include "plugapi.hpp"
#include "dlgedit.hpp"
#include "clipboard.hpp"
#include "filelist.hpp"
#include "treelist.hpp"
#include "dirmix.hpp"
#include "elevation.hpp"
#include "stddlg.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "usermenu.hpp"
#include "filemasks.hpp"
#include "plugins.hpp"
#include "interf.hpp"
#include "lang.hpp"
#include "strmix.hpp"
#include "string_sort.hpp"
#include "tvar.hpp"
#include "global.hpp"

// Platform:
#include "platform.env.hpp"
#include "platform.fs.hpp"
#include "platform.security.hpp"

// Common:
#include "common/from_string.hpp"
#include "common/uuid.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

#if 0
void print_opcodes()
{
	FILE* fp=fopen("opcodes.tmp", "w");
	if (!fp) return;

	/* ************************************************************************* */
	// функции
	fprintf(fp, "MCODE_F_NOFUNC=0x%X\n", MCODE_F_NOFUNC);
	fprintf(fp, "MCODE_F_ABS=0x%X // N=abs(N)\n", MCODE_F_ABS);
	fprintf(fp, "MCODE_F_AKEY=0x%X // V=akey(Mode[,Type])\n", MCODE_F_AKEY);
	fprintf(fp, "MCODE_F_ASC=0x%X // N=asc(S)\n", MCODE_F_ASC);
	fprintf(fp, "MCODE_F_ATOI=0x%X // N=atoi(S[,radix])\n", MCODE_F_ATOI);
	fprintf(fp, "MCODE_F_CLIP=0x%X // V=clip(N[,V])\n", MCODE_F_CLIP);
	fprintf(fp, "MCODE_F_CHR=0x%X // S=chr(N)\n", MCODE_F_CHR);
	fprintf(fp, "MCODE_F_DATE=0x%X // S=date([S])\n", MCODE_F_DATE);
	fprintf(fp, "MCODE_F_DLG_GETVALUE=0x%X // V=Dlg->GetValue([Pos[,InfoID]])\n", MCODE_F_DLG_GETVALUE);
	fprintf(fp, "MCODE_F_EDITOR_SEL=0x%X // V=Editor.Sel(Action[,Opt])\n", MCODE_F_EDITOR_SEL);
	fprintf(fp, "MCODE_F_EDITOR_SET=0x%X // N=Editor.Set(N[,Var])\n", MCODE_F_EDITOR_SET);
	fprintf(fp, "MCODE_F_EDITOR_UNDO=0x%X // V=Editor.Undo(N)\n", MCODE_F_EDITOR_UNDO);
	fprintf(fp, "MCODE_F_EDITOR_POS=0x%X // N=Editor.Pos(Op,What[,Where])\n", MCODE_F_EDITOR_POS);
	fprintf(fp, "MCODE_F_ENVIRON=0x%X // S=Env(S[,Mode[,Value]])\n", MCODE_F_ENVIRON);
	fprintf(fp, "MCODE_F_FATTR=0x%X // N=fattr(S)\n", MCODE_F_FATTR);
	fprintf(fp, "MCODE_F_FEXIST=0x%X // S=fexist(S)\n", MCODE_F_FEXIST);
	fprintf(fp, "MCODE_F_FSPLIT=0x%X // S=fsplit(S,N)\n", MCODE_F_FSPLIT);
	fprintf(fp, "MCODE_F_IIF=0x%X // V=iif(C,V1,V2)\n", MCODE_F_IIF);
	fprintf(fp, "MCODE_F_INDEX=0x%X // S=index(S1,S2[,Mode])\n", MCODE_F_INDEX);
	fprintf(fp, "MCODE_F_INT=0x%X // N=int(V)\n", MCODE_F_INT);
	fprintf(fp, "MCODE_F_ITOA=0x%X // S=itoa(N[,radix])\n", MCODE_F_ITOA);
	fprintf(fp, "MCODE_F_KEY=0x%X // S=key(V)\n", MCODE_F_KEY);
	fprintf(fp, "MCODE_F_LCASE=0x%X // S=lcase(S1)\n", MCODE_F_LCASE);
	fprintf(fp, "MCODE_F_LEN=0x%X // N=len(S)\n", MCODE_F_LEN);
	fprintf(fp, "MCODE_F_MAX=0x%X // N=max(N1,N2)\n", MCODE_F_MAX);
	fprintf(fp, "MCODE_F_MENU_CHECKHOTKEY=0x%X // N=checkhotkey(S[,N])\n", MCODE_F_MENU_CHECKHOTKEY);
	fprintf(fp, "MCODE_F_MENU_GETHOTKEY=0x%X // S=gethotkey([N])\n", MCODE_F_MENU_GETHOTKEY);
	fprintf(fp, "MCODE_F_MENU_SELECT=0x%X // N=Menu.Select(S[,N[,Dir]])\n", MCODE_F_MENU_SELECT);
	fprintf(fp, "MCODE_F_MENU_SHOW=0x%X // S=Menu.Show(Items[,Title[,Flags[,FindOrFilter[,X[,Y]]]]])\n", MCODE_F_MENU_SHOW);
	fprintf(fp, "MCODE_F_MIN=0x%X // N=min(N1,N2)\n", MCODE_F_MIN);
	fprintf(fp, "MCODE_F_MOD=0x%X // N=mod(a,b) == a %%  b\n", MCODE_F_MOD);
	fprintf(fp, "MCODE_F_MLOAD=0x%X // B=mload(var)\n", MCODE_F_MLOAD);
	fprintf(fp, "MCODE_F_MSAVE=0x%X // B=msave(var)\n", MCODE_F_MSAVE);
	fprintf(fp, "MCODE_F_MSGBOX=0x%X // N=msgbox([\"Title\"[,\"Text\"[,flags]]])\n", MCODE_F_MSGBOX);
	fprintf(fp, "MCODE_F_PANEL_FATTR=0x%X // N=Panel.FAttr(panelType,fileMask)\n", MCODE_F_PANEL_FATTR);
	fprintf(fp, "MCODE_F_PANEL_FEXIST=0x%X // N=Panel.FExist(panelType,fileMask)\n", MCODE_F_PANEL_FEXIST);
	fprintf(fp, "MCODE_F_PANEL_SETPOS=0x%X // N=Panel.SetPos(panelType,fileName)\n", MCODE_F_PANEL_SETPOS);
	fprintf(fp, "MCODE_F_PANEL_SETPOSIDX=0x%X // N=Panel.SetPosIdx(panelType,Idx[,InSelection])\n", MCODE_F_PANEL_SETPOSIDX);
	fprintf(fp, "MCODE_F_PANEL_SELECT=0x%X // V=Panel.Select(panelType,Action[,Mode[,Items]])\n", MCODE_F_PANEL_SELECT);
	fprintf(fp, "MCODE_F_PANELITEM=0x%X // V=PanelItem(Panel,Index,TypeInfo)\n", MCODE_F_PANELITEM);
	fprintf(fp, "MCODE_F_EVAL=0x%X // N=eval(S[,N])\n", MCODE_F_EVAL);
	fprintf(fp, "MCODE_F_RINDEX=0x%X // S=rindex(S1,S2[,Mode])\n", MCODE_F_RINDEX);
	fprintf(fp, "MCODE_F_SLEEP=0x%X // os::chrono::sleep_for(Nms)\n", MCODE_F_SLEEP);
	fprintf(fp, "MCODE_F_STRING=0x%X // S=string(V)\n", MCODE_F_STRING);
	fprintf(fp, "MCODE_F_SUBSTR=0x%X // S=substr(S,start[,length])\n", MCODE_F_SUBSTR);
	fprintf(fp, "MCODE_F_UCASE=0x%X // S=ucase(S1)\n", MCODE_F_UCASE);
	fprintf(fp, "MCODE_F_WAITKEY=0x%X // V=waitkey([N,[T]])\n", MCODE_F_WAITKEY);
	fprintf(fp, "MCODE_F_XLAT=0x%X // S=xlat(S)\n", MCODE_F_XLAT);
	fprintf(fp, "MCODE_F_FLOCK=0x%X // N=FLock(N,N)\n", MCODE_F_FLOCK);
	fprintf(fp, "MCODE_F_CALLPLUGIN=0x%X // V=callplugin(SysID[,param])\n", MCODE_F_CALLPLUGIN);
	fprintf(fp, "MCODE_F_REPLACE=0x%X // S=replace(sS,sF,sR[,Count[,Mode]])\n", MCODE_F_REPLACE);
	fprintf(fp, "MCODE_F_PROMPT=0x%X // S=prompt([\"Title\"[,\"Prompt\"[,flags[, \"Src\"[, \"History\"]]]]])\n", MCODE_F_PROMPT);
	fprintf(fp, "MCODE_F_BM_ADD=0x%X // N=BM.Add()  - добавить текущие координаты и обрезать хвост\n", MCODE_F_BM_ADD);
	fprintf(fp, "MCODE_F_BM_CLEAR=0x%X // N=BM.Clear() - очистить все закладки\n", MCODE_F_BM_CLEAR);
	fprintf(fp, "MCODE_F_BM_DEL=0x%X // N=BM.Del([Idx]) - удаляет закладку с указанным индексом (x=1...), 0 - удаляет текущую закладку\n", MCODE_F_BM_DEL);
	fprintf(fp, "MCODE_F_BM_GET=0x%X // N=BM.Get(Idx,M) - возвращает координаты строки (M==0) или колонки (M==1) закладки с индексом (Idx=1...)\n", MCODE_F_BM_GET);
	fprintf(fp, "MCODE_F_BM_GOTO=0x%X // N=BM.Goto([n]) - переход на закладку с указанным индексом (0 --> текущую)\n", MCODE_F_BM_GOTO);
	fprintf(fp, "MCODE_F_BM_NEXT=0x%X // N=BM.Next() - перейти на следующую закладку\n", MCODE_F_BM_NEXT);
	fprintf(fp, "MCODE_F_BM_POP=0x%X // N=BM.Pop() - восстановить текущую позицию из закладки в конце стека и удалить закладку\n", MCODE_F_BM_POP);
	fprintf(fp, "MCODE_F_BM_PREV=0x%X // N=BM.Prev() - перейти на предыдущую закладку\n", MCODE_F_BM_PREV);
	fprintf(fp, "MCODE_F_BM_BACK=0x%X // N=BM.Back() - перейти на предыдущую закладку с возможным сохранением текущей позиции\n", MCODE_F_BM_BACK);
	fprintf(fp, "MCODE_F_BM_PUSH=0x%X // N=BM.Push() - сохранить текущую позицию в виде закладки в конце стека\n", MCODE_F_BM_PUSH);
	fprintf(fp, "MCODE_F_BM_STAT=0x%X // N=BM.Stat([M]) - возвращает информацию о закладках, N=0 - текущее количество закладок\n", MCODE_F_BM_STAT);
	fprintf(fp, "MCODE_F_TRIM=0x%X // S=trim(S[,N])\n", MCODE_F_TRIM);
	fprintf(fp, "MCODE_F_FLOAT=0x%X // N=float(V)\n", MCODE_F_FLOAT);
	fprintf(fp, "MCODE_F_TESTFOLDER=0x%X // N=testfolder(S)\n", MCODE_F_TESTFOLDER);
	fprintf(fp, "MCODE_F_PRINT=0x%X // N=Print(Str)\n", MCODE_F_PRINT);
	fprintf(fp, "MCODE_F_MMODE=0x%X // N=MMode(Action[,Value])\n", MCODE_F_MMODE);
	fprintf(fp, "MCODE_F_EDITOR_SETTITLE=0x%X // N=Editor.SetTitle([Title])\n", MCODE_F_EDITOR_SETTITLE);
	fprintf(fp, "MCODE_F_MENU_GETVALUE=0x%X // S=Menu.GetValue([N])\n", MCODE_F_MENU_GETVALUE);
	fprintf(fp, "MCODE_F_MENU_ITEMSTATUS=0x%X // N=Menu.ItemStatus([N])\n", MCODE_F_MENU_ITEMSTATUS);
	fprintf(fp, "MCODE_F_BEEP=0x%X // N=beep([N])\n", MCODE_F_BEEP);
	fprintf(fp, "MCODE_F_KBDLAYOUT=0x%X // N=kbdLayout([N])\n", MCODE_F_KBDLAYOUT);
	fprintf(fp, "MCODE_F_WINDOW_SCROLL=0x%X // N=Window.Scroll(Lines[,Axis])\n", MCODE_F_WINDOW_SCROLL);
	fprintf(fp, "MCODE_F_KEYBAR_SHOW=0x%X // N=KeyBar.Show([N])\n", MCODE_F_KEYBAR_SHOW);
	fprintf(fp, "MCODE_F_HISTORY_DISABLE=0x%X // N=History.Disable([State])\n", MCODE_F_HISTORY_DISABLE);
	fprintf(fp, "MCODE_F_FMATCH=0x%X // N=FMatch(S,Mask)\n", MCODE_F_FMATCH);
	fprintf(fp, "MCODE_F_PLUGIN_MENU=0x%X // N=Plugin.Menu(Uuid[,MenuUuid])\n", MCODE_F_PLUGIN_MENU);
	fprintf(fp, "MCODE_F_PLUGIN_CALL=0x%X // N=Plugin.Config(Uuid[,MenuUuid])\n", MCODE_F_PLUGIN_CALL);
	fprintf(fp, "MCODE_F_PLUGIN_SYNCCALL=0x%X // N=Plugin.Call(Uuid[,Item])\n", MCODE_F_PLUGIN_SYNCCALL);
	fprintf(fp, "MCODE_F_PLUGIN_LOAD=0x%X // N=Plugin.Load(DllPath[,ForceLoad])\n", MCODE_F_PLUGIN_LOAD);
	fprintf(fp, "MCODE_F_PLUGIN_COMMAND=0x%X // N=Plugin.Command(Uuid[,Command])\n", MCODE_F_PLUGIN_COMMAND);
	fprintf(fp, "MCODE_F_PLUGIN_UNLOAD=0x%X // N=Plugin.UnLoad(DllPath)\n", MCODE_F_PLUGIN_UNLOAD);
	fprintf(fp, "MCODE_F_PLUGIN_EXIST=0x%X // N=Plugin.Exist(Uuid)\n", MCODE_F_PLUGIN_EXIST);
	fprintf(fp, "MCODE_F_MENU_FILTER=0x%X // N=Menu.Filter(Action[,Mode])\n", MCODE_F_MENU_FILTER);
	fprintf(fp, "MCODE_F_MENU_FILTERSTR=0x%X // S=Menu.FilterStr([Action[,S]])\n", MCODE_F_MENU_FILTERSTR);
	fprintf(fp, "MCODE_F_DLG_SETFOCUS=0x%X // N=Dlg->SetFocus([ID])\n", MCODE_F_DLG_SETFOCUS);
	fprintf(fp, "MCODE_F_FAR_CFG_GET=0x%X // V=Far.Cfg.Get(Key,Name)\n", MCODE_F_FAR_CFG_GET);
	fprintf(fp, "MCODE_F_SIZE2STR=0x%X // S=Size2Str(N,Flags[,Width])\n", MCODE_F_SIZE2STR);
	fprintf(fp, "MCODE_F_STRWRAP=0x%X // S=StrWrap(Text,Width[,Break[,Flags]])\n", MCODE_F_STRWRAP);
	fprintf(fp, "MCODE_F_MACRO_KEYWORD=0x%X // S=Macro.Keyword(Index[,Type])\n", MCODE_F_MACRO_KEYWORD);
	fprintf(fp, "MCODE_F_MACRO_FUNC=0x%X // S=Macro.Func(Index[,Type])\n", MCODE_F_MACRO_FUNC);
	fprintf(fp, "MCODE_F_MACRO_VAR=0x%X // S=Macro.Var(Index[,Type])\n", MCODE_F_MACRO_VAR);
	fprintf(fp, "MCODE_F_MACRO_CONST=0x%X // S=Macro.Const(Index[,Type])\n", MCODE_F_MACRO_CONST);
	fprintf(fp, "MCODE_F_STRPAD=0x%X // S=StrPad(V,Cnt[,Fill[,Op]])\n", MCODE_F_STRPAD);
	fprintf(fp, "MCODE_F_EDITOR_DELLINE=0x%X // N=Editor.DelLine([Line])\n", MCODE_F_EDITOR_DELLINE);
	fprintf(fp, "MCODE_F_EDITOR_GETSTR=0x%X // S=Editor.GetStr([Line])\n", MCODE_F_EDITOR_GETSTR);
	fprintf(fp, "MCODE_F_EDITOR_INSSTR=0x%X // N=Editor.InsStr([S[,Line]])\n", MCODE_F_EDITOR_INSSTR);
	fprintf(fp, "MCODE_F_EDITOR_SETSTR=0x%X // N=Editor.SetStr([S[,Line]])\n", MCODE_F_EDITOR_SETSTR);
	/* ************************************************************************* */
	fprintf(fp, "MCODE_F_CHECKALL=0x%X // Проверить предварительные условия исполнения макроса\n", MCODE_F_CHECKALL);
	fprintf(fp, "MCODE_F_GETOPTIONS=0x%X // Получить значения некоторых опций Фара\n", MCODE_F_GETOPTIONS);
	fprintf(fp, "MCODE_F_USERMENU=0x%X // Вывести меню пользователя\n", MCODE_F_USERMENU);
	fprintf(fp, "MCODE_F_SETCUSTOMSORTMODE=0x%X // Установить пользовательский режим сортировки\n", MCODE_F_SETCUSTOMSORTMODE);
	fprintf(fp, "MCODE_F_KEYMACRO=0x%X // Набор простых операций\n", MCODE_F_KEYMACRO);
	fprintf(fp, "MCODE_F_FAR_GETCONFIG=0x%X // V=Far.GetConfig(Key,Name)\n", MCODE_F_FAR_GETCONFIG);
	fprintf(fp, "MCODE_F_MACROSETTINGS=0x%X // Диалог редактирования макроса\n", MCODE_F_MACROSETTINGS);
	fprintf(fp, "MCODE_F_LAST=0x%X // marker\n", MCODE_F_LAST);
	/* ************************************************************************* */
	// булевые переменные - различные состояния
	fprintf(fp, "MCODE_C_AREA_OTHER=0x%X // Режим копирования текста с экрана, вертикальные меню\n", MCODE_C_AREA_OTHER);
	fprintf(fp, "MCODE_C_AREA_SHELL=0x%X // Файловые панели\n", MCODE_C_AREA_SHELL);
	fprintf(fp, "MCODE_C_AREA_VIEWER=0x%X // Внутренняя программа просмотра\n", MCODE_C_AREA_VIEWER);
	fprintf(fp, "MCODE_C_AREA_EDITOR=0x%X // Редактор\n", MCODE_C_AREA_EDITOR);
	fprintf(fp, "MCODE_C_AREA_DIALOG=0x%X // Диалоги\n", MCODE_C_AREA_DIALOG);
	fprintf(fp, "MCODE_C_AREA_SEARCH=0x%X // Быстрый поиск в панелях\n", MCODE_C_AREA_SEARCH);
	fprintf(fp, "MCODE_C_AREA_DISKS=0x%X // Меню выбора дисков\n", MCODE_C_AREA_DISKS);
	fprintf(fp, "MCODE_C_AREA_MAINMENU=0x%X // Основное меню\n", MCODE_C_AREA_MAINMENU);
	fprintf(fp, "MCODE_C_AREA_MENU=0x%X // Прочие меню\n", MCODE_C_AREA_MENU);
	fprintf(fp, "MCODE_C_AREA_HELP=0x%X // Система помощи\n", MCODE_C_AREA_HELP);
	fprintf(fp, "MCODE_C_AREA_INFOPANEL=0x%X // Информационная панель\n", MCODE_C_AREA_INFOPANEL);
	fprintf(fp, "MCODE_C_AREA_QVIEWPANEL=0x%X // Панель быстрого просмотра\n", MCODE_C_AREA_QVIEWPANEL);
	fprintf(fp, "MCODE_C_AREA_TREEPANEL=0x%X // Панель дерева папок\n", MCODE_C_AREA_TREEPANEL);
	fprintf(fp, "MCODE_C_AREA_FINDFOLDER=0x%X // Поиск папок\n", MCODE_C_AREA_FINDFOLDER);
	fprintf(fp, "MCODE_C_AREA_USERMENU=0x%X // Меню пользователя\n", MCODE_C_AREA_USERMENU);
	fprintf(fp, "MCODE_C_AREA_SHELL_AUTOCOMPLETION=0x%X // Список автодополнения в панелях в ком.строке\n", MCODE_C_AREA_SHELL_AUTOCOMPLETION);
	fprintf(fp, "MCODE_C_AREA_DIALOG_AUTOCOMPLETION=0x%X // Список автодополнения в диалоге\n", MCODE_C_AREA_DIALOG_AUTOCOMPLETION);

	fprintf(fp, "MCODE_C_FULLSCREENMODE=0x%X // полноэкранный режим?\n", MCODE_C_FULLSCREENMODE);
	fprintf(fp, "MCODE_C_ISUSERADMIN=0x%X // Administrator status\n", MCODE_C_ISUSERADMIN);
	fprintf(fp, "MCODE_C_BOF=0x%X // начало файла/активного каталога?\n", MCODE_C_BOF);
	fprintf(fp, "MCODE_C_EOF=0x%X // конец файла/активного каталога?\n", MCODE_C_EOF);
	fprintf(fp, "MCODE_C_EMPTY=0x%X // ком.строка пуста?\n", MCODE_C_EMPTY);
	fprintf(fp, "MCODE_C_SELECTED=0x%X // выделенный блок есть?\n", MCODE_C_SELECTED);
	fprintf(fp, "MCODE_C_ROOTFOLDER=0x%X // аналог MCODE_C_APANEL_ROOT для активной панели\n", MCODE_C_ROOTFOLDER);

	fprintf(fp, "MCODE_C_APANEL_BOF=0x%X // начало активного  каталога?\n", MCODE_C_APANEL_BOF);
	fprintf(fp, "MCODE_C_PPANEL_BOF=0x%X // начало пассивного каталога?\n", MCODE_C_PPANEL_BOF);
	fprintf(fp, "MCODE_C_APANEL_EOF=0x%X // конец активного  каталога?\n", MCODE_C_APANEL_EOF);
	fprintf(fp, "MCODE_C_PPANEL_EOF=0x%X // конец пассивного каталога?\n", MCODE_C_PPANEL_EOF);
	fprintf(fp, "MCODE_C_APANEL_ISEMPTY=0x%X // активная панель:  пуста?\n", MCODE_C_APANEL_ISEMPTY);
	fprintf(fp, "MCODE_C_PPANEL_ISEMPTY=0x%X // пассивная панель: пуста?\n", MCODE_C_PPANEL_ISEMPTY);
	fprintf(fp, "MCODE_C_APANEL_SELECTED=0x%X // активная панель:  выделенные элементы есть?\n", MCODE_C_APANEL_SELECTED);
	fprintf(fp, "MCODE_C_PPANEL_SELECTED=0x%X // пассивная панель: выделенные элементы есть?\n", MCODE_C_PPANEL_SELECTED);
	fprintf(fp, "MCODE_C_APANEL_ROOT=0x%X // это корневой каталог активной панели?\n", MCODE_C_APANEL_ROOT);
	fprintf(fp, "MCODE_C_PPANEL_ROOT=0x%X // это корневой каталог пассивной панели?\n", MCODE_C_PPANEL_ROOT);
	fprintf(fp, "MCODE_C_APANEL_VISIBLE=0x%X // активная панель:  видима?\n", MCODE_C_APANEL_VISIBLE);
	fprintf(fp, "MCODE_C_PPANEL_VISIBLE=0x%X // пассивная панель: видима?\n", MCODE_C_PPANEL_VISIBLE);
	fprintf(fp, "MCODE_C_APANEL_PLUGIN=0x%X // активная панель:  плагиновая?\n", MCODE_C_APANEL_PLUGIN);
	fprintf(fp, "MCODE_C_PPANEL_PLUGIN=0x%X // пассивная панель: плагиновая?\n", MCODE_C_PPANEL_PLUGIN);
	fprintf(fp, "MCODE_C_APANEL_FILEPANEL=0x%X // активная панель:  файловая?\n", MCODE_C_APANEL_FILEPANEL);
	fprintf(fp, "MCODE_C_PPANEL_FILEPANEL=0x%X // пассивная панель: файловая?\n", MCODE_C_PPANEL_FILEPANEL);
	fprintf(fp, "MCODE_C_APANEL_FOLDER=0x%X // активная панель:  текущий элемент каталог?\n", MCODE_C_APANEL_FOLDER);
	fprintf(fp, "MCODE_C_PPANEL_FOLDER=0x%X // пассивная панель: текущий элемент каталог?\n", MCODE_C_PPANEL_FOLDER);
	fprintf(fp, "MCODE_C_APANEL_LEFT=0x%X // активная панель левая?\n", MCODE_C_APANEL_LEFT);
	fprintf(fp, "MCODE_C_PPANEL_LEFT=0x%X // пассивная панель левая?\n", MCODE_C_PPANEL_LEFT);
	fprintf(fp, "MCODE_C_APANEL_LFN=0x%X // на активной панели длинные имена?\n", MCODE_C_APANEL_LFN);
	fprintf(fp, "MCODE_C_PPANEL_LFN=0x%X // на пассивной панели длинные имена?\n", MCODE_C_PPANEL_LFN);
	fprintf(fp, "MCODE_C_APANEL_FILTER=0x%X // на активной панели включен фильтр?\n", MCODE_C_APANEL_FILTER);
	fprintf(fp, "MCODE_C_PPANEL_FILTER=0x%X // на пассивной панели включен фильтр?\n", MCODE_C_PPANEL_FILTER);

	fprintf(fp, "MCODE_C_CMDLINE_BOF=0x%X // курсор в начале cmd-строки редактирования?\n", MCODE_C_CMDLINE_BOF);
	fprintf(fp, "MCODE_C_CMDLINE_EOF=0x%X // курсор в конце cmd-строки редактирования?\n", MCODE_C_CMDLINE_EOF);
	fprintf(fp, "MCODE_C_CMDLINE_EMPTY=0x%X // ком.строка пуста?\n", MCODE_C_CMDLINE_EMPTY);
	fprintf(fp, "MCODE_C_CMDLINE_SELECTED=0x%X // в ком.строке есть выделение блока?\n", MCODE_C_CMDLINE_SELECTED);

	fprintf(fp, "MCODE_C_MSX=0x%X          // Mouse.X\n", MCODE_C_MSX);
	fprintf(fp, "MCODE_C_MSY=0x%X          // Mouse.Y\n", MCODE_C_MSY);
	fprintf(fp, "MCODE_C_MSBUTTON=0x%X     // Mouse.Button\n", MCODE_C_MSBUTTON);
	fprintf(fp, "MCODE_C_MSCTRLSTATE=0x%X  // Mouse.CtrlState\n", MCODE_C_MSCTRLSTATE);
	fprintf(fp, "MCODE_C_MSEVENTFLAGS=0x%X // Mouse.EventFlags\n", MCODE_C_MSEVENTFLAGS);
	fprintf(fp, "MCODE_C_MSLASTCTRLSTATE=0x%X  // Mouse.LastCtrlState\n", MCODE_C_MSLASTCTRLSTATE);

	/* ************************************************************************* */
	// не булевые переменные
	fprintf(fp, "MCODE_V_FAR_WIDTH=0x%X // Far.Width - ширина консольного окна\n", MCODE_V_FAR_WIDTH);
	fprintf(fp, "MCODE_V_FAR_HEIGHT=0x%X // Far.Height - высота консольного окна\n", MCODE_V_FAR_HEIGHT);
	fprintf(fp, "MCODE_V_FAR_TITLE=0x%X // Far.Title - текущий заголовок консольного окна\n", MCODE_V_FAR_TITLE);
	fprintf(fp, "MCODE_V_FAR_UPTIME=0x%X // Far.UpTime - время работы Far в миллисекундах\n", MCODE_V_FAR_UPTIME);
	fprintf(fp, "MCODE_V_FAR_PID=0x%X // Far.PID - содержит ИД текущей запущенной копии Far Manager\n", MCODE_V_FAR_PID);
	fprintf(fp, "MCODE_V_MACRO_AREA=0x%X // MacroArea - имя текущей макрос области\n", MCODE_V_MACRO_AREA);

	fprintf(fp, "MCODE_V_APANEL_CURRENT=0x%X // APanel.Current - имя файла на активной панели\n", MCODE_V_APANEL_CURRENT);
	fprintf(fp, "MCODE_V_PPANEL_CURRENT=0x%X // PPanel.Current - имя файла на пассивной панели\n", MCODE_V_PPANEL_CURRENT);
	fprintf(fp, "MCODE_V_APANEL_SELCOUNT=0x%X // APanel.SelCount - активная панель:  число выделенных элементов\n", MCODE_V_APANEL_SELCOUNT);
	fprintf(fp, "MCODE_V_PPANEL_SELCOUNT=0x%X // PPanel.SelCount - пассивная панель: число выделенных элементов\n", MCODE_V_PPANEL_SELCOUNT);
	fprintf(fp, "MCODE_V_APANEL_PATH=0x%X // APanel.Path - активная панель:  путь на панели\n", MCODE_V_APANEL_PATH);
	fprintf(fp, "MCODE_V_PPANEL_PATH=0x%X // PPanel.Path - пассивная панель: путь на панели\n", MCODE_V_PPANEL_PATH);
	fprintf(fp, "MCODE_V_APANEL_PATH0=0x%X // APanel.Path0 - активная панель:  путь на панели до вызова плагинов\n", MCODE_V_APANEL_PATH0);
	fprintf(fp, "MCODE_V_PPANEL_PATH0=0x%X // PPanel.Path0 - пассивная панель: путь на панели до вызова плагинов\n", MCODE_V_PPANEL_PATH0);
	fprintf(fp, "MCODE_V_APANEL_UNCPATH=0x%X // APanel.UNCPath - активная панель:  UNC-путь на панели\n", MCODE_V_APANEL_UNCPATH);
	fprintf(fp, "MCODE_V_PPANEL_UNCPATH=0x%X // PPanel.UNCPath - пассивная панель: UNC-путь на панели\n", MCODE_V_PPANEL_UNCPATH);
	fprintf(fp, "MCODE_V_APANEL_WIDTH=0x%X // APanel.Width - активная панель:  ширина панели\n", MCODE_V_APANEL_WIDTH);
	fprintf(fp, "MCODE_V_PPANEL_WIDTH=0x%X // PPanel.Width - пассивная панель: ширина панели\n", MCODE_V_PPANEL_WIDTH);
	fprintf(fp, "MCODE_V_APANEL_TYPE=0x%X // APanel.Type - тип активной панели\n", MCODE_V_APANEL_TYPE);
	fprintf(fp, "MCODE_V_PPANEL_TYPE=0x%X // PPanel.Type - тип пассивной панели\n", MCODE_V_PPANEL_TYPE);
	fprintf(fp, "MCODE_V_APANEL_ITEMCOUNT=0x%X // APanel.ItemCount - активная панель:  число элементов\n", MCODE_V_APANEL_ITEMCOUNT);
	fprintf(fp, "MCODE_V_PPANEL_ITEMCOUNT=0x%X // PPanel.ItemCount - пассивная панель: число элементов\n", MCODE_V_PPANEL_ITEMCOUNT);
	fprintf(fp, "MCODE_V_APANEL_CURPOS=0x%X // APanel.CurPos - активная панель:  текущий индекс\n", MCODE_V_APANEL_CURPOS);
	fprintf(fp, "MCODE_V_PPANEL_CURPOS=0x%X // PPanel.CurPos - пассивная панель: текущий индекс\n", MCODE_V_PPANEL_CURPOS);
	fprintf(fp, "MCODE_V_APANEL_OPIFLAGS=0x%X // APanel.OPIFlags - активная панель: флаги открытого плагина\n", MCODE_V_APANEL_OPIFLAGS);
	fprintf(fp, "MCODE_V_PPANEL_OPIFLAGS=0x%X // PPanel.OPIFlags - пассивная панель: флаги открытого плагина\n", MCODE_V_PPANEL_OPIFLAGS);
	fprintf(fp, "MCODE_V_APANEL_DRIVETYPE=0x%X // APanel.DriveType - активная панель: тип привода\n", MCODE_V_APANEL_DRIVETYPE);
	fprintf(fp, "MCODE_V_PPANEL_DRIVETYPE=0x%X // PPanel.DriveType - пассивная панель: тип привода\n", MCODE_V_PPANEL_DRIVETYPE);
	fprintf(fp, "MCODE_V_APANEL_HEIGHT=0x%X // APanel.Height - активная панель:  высота панели\n", MCODE_V_APANEL_HEIGHT);
	fprintf(fp, "MCODE_V_PPANEL_HEIGHT=0x%X // PPanel.Height - пассивная панель: высота панели\n", MCODE_V_PPANEL_HEIGHT);
	fprintf(fp, "MCODE_V_APANEL_COLUMNCOUNT=0x%X // APanel.ColumnCount - активная панель:  количество колонок\n", MCODE_V_APANEL_COLUMNCOUNT);
	fprintf(fp, "MCODE_V_PPANEL_COLUMNCOUNT=0x%X // PPanel.ColumnCount - пассивная панель: количество колонок\n", MCODE_V_PPANEL_COLUMNCOUNT);
	fprintf(fp, "MCODE_V_APANEL_HOSTFILE=0x%X // APanel.HostFile - активная панель:  имя Host-файла\n", MCODE_V_APANEL_HOSTFILE);
	fprintf(fp, "MCODE_V_PPANEL_HOSTFILE=0x%X // PPanel.HostFile - пассивная панель: имя Host-файла\n", MCODE_V_PPANEL_HOSTFILE);
	fprintf(fp, "MCODE_V_APANEL_PREFIX=0x%X // APanel.Prefix\n", MCODE_V_APANEL_PREFIX);
	fprintf(fp, "MCODE_V_PPANEL_PREFIX=0x%X // PPanel.Prefix\n", MCODE_V_PPANEL_PREFIX);
	fprintf(fp, "MCODE_V_APANEL_FORMAT=0x%X // APanel.Format\n", MCODE_V_APANEL_FORMAT);
	fprintf(fp, "MCODE_V_PPANEL_FORMAT=0x%X // PPanel.Format\n", MCODE_V_PPANEL_FORMAT);

	fprintf(fp, "MCODE_V_ITEMCOUNT=0x%X // ItemCount - число элементов в текущем объекте\n", MCODE_V_ITEMCOUNT);
	fprintf(fp, "MCODE_V_CURPOS=0x%X // CurPos - текущий индекс в текущем объекте\n", MCODE_V_CURPOS);
	fprintf(fp, "MCODE_V_TITLE=0x%X // Title - заголовок текущего объекта\n", MCODE_V_TITLE);
	fprintf(fp, "MCODE_V_HEIGHT=0x%X // Height - высота текущего объекта\n", MCODE_V_HEIGHT);
	fprintf(fp, "MCODE_V_WIDTH=0x%X // Width - ширина текущего объекта\n", MCODE_V_WIDTH);

	fprintf(fp, "MCODE_V_EDITORFILENAME=0x%X // Editor.FileName - имя редактируемого файла\n", MCODE_V_EDITORFILENAME);
	fprintf(fp, "MCODE_V_EDITORLINES=0x%X // Editor.Lines - количество строк в редакторе\n", MCODE_V_EDITORLINES);
	fprintf(fp, "MCODE_V_EDITORCURLINE=0x%X // Editor.CurLine - текущая линия в редакторе (в дополнении к Count)\n", MCODE_V_EDITORCURLINE);
	fprintf(fp, "MCODE_V_EDITORCURPOS=0x%X // Editor.CurPos - текущая поз. в редакторе\n", MCODE_V_EDITORCURPOS);
	fprintf(fp, "MCODE_V_EDITORREALPOS=0x%X // Editor.RealPos - текущая поз. в редакторе без привязки к размеру табуляции\n", MCODE_V_EDITORREALPOS);
	fprintf(fp, "MCODE_V_EDITORSTATE=0x%X // Editor.State\n", MCODE_V_EDITORSTATE);
	fprintf(fp, "MCODE_V_EDITORVALUE=0x%X // Editor.Value - содержимое текущей строки\n", MCODE_V_EDITORVALUE);
	fprintf(fp, "MCODE_V_EDITORSELVALUE=0x%X // Editor.SelValue - содержит содержимое выделенного блока\n", MCODE_V_EDITORSELVALUE);

	fprintf(fp, "MCODE_V_DLGITEMTYPE=0x%X // Dlg->ItemType\n", MCODE_V_DLGITEMTYPE);
	fprintf(fp, "MCODE_V_DLGITEMCOUNT=0x%X // Dlg->ItemCount\n", MCODE_V_DLGITEMCOUNT);
	fprintf(fp, "MCODE_V_DLGCURPOS=0x%X // Dlg->CurPos\n", MCODE_V_DLGCURPOS);
	fprintf(fp, "MCODE_V_DLGPREVPOS=0x%X // Dlg->PrevPos\n", MCODE_V_DLGPREVPOS);
	fprintf(fp, "MCODE_V_DLGINFOID=0x%X // Dlg->Info.Id\n", MCODE_V_DLGINFOID);
	fprintf(fp, "MCODE_V_DLGINFOOWNER=0x%X // Dlg->Info.Owner\n", MCODE_V_DLGINFOOWNER);

	fprintf(fp, "MCODE_V_VIEWERFILENAME=0x%X // Viewer.FileName - имя просматриваемого файла\n", MCODE_V_VIEWERFILENAME);
	fprintf(fp, "MCODE_V_VIEWERSTATE=0x%X // Viewer.State\n", MCODE_V_VIEWERSTATE);

	fprintf(fp, "MCODE_V_CMDLINE_ITEMCOUNT=0x%X // CmdLine.ItemCount\n", MCODE_V_CMDLINE_ITEMCOUNT);
	fprintf(fp, "MCODE_V_CMDLINE_CURPOS=0x%X // CmdLine.CurPos\n", MCODE_V_CMDLINE_CURPOS);
	fprintf(fp, "MCODE_V_CMDLINE_VALUE=0x%X // CmdLine.Value\n", MCODE_V_CMDLINE_VALUE);

	fprintf(fp, "MCODE_V_DRVSHOWPOS=0x%X // Drv.ShowPos - меню выбора дисков отображено: 1=слева (Alt-F1), 2=справа (Alt-F2), 0=\"нету его\"\n", MCODE_V_DRVSHOWPOS);
	fprintf(fp, "MCODE_V_DRVSHOWMODE=0x%X // Drv.ShowMode - режимы отображения меню выбора дисков\n", MCODE_V_DRVSHOWMODE);

	fprintf(fp, "MCODE_V_HELPFILENAME=0x%X // Help.FileName\n", MCODE_V_HELPFILENAME);
	fprintf(fp, "MCODE_V_HELPTOPIC=0x%X // Help.Topic\n", MCODE_V_HELPTOPIC);
	fprintf(fp, "MCODE_V_HELPSELTOPIC=0x%X // Help.SelTopic\n", MCODE_V_HELPSELTOPIC);

	fprintf(fp, "MCODE_V_MENU_VALUE=0x%X // Menu.Value\n", MCODE_V_MENU_VALUE);
	fprintf(fp, "MCODE_V_MENUINFOID=0x%X // Menu.Info.Id\n", MCODE_V_MENUINFOID);

	fclose(fp);
}
#endif

using MACROFLAGS_MFLAGS = unsigned long long;
enum: MACROFLAGS_MFLAGS
{
	MFLAGS_NONE                    = 0,
	// public flags, read from/saved to config
	MFLAGS_ENABLEOUTPUT            = 0_bit, // не подавлять обновление экрана во время выполнения макроса
	MFLAGS_NOSENDKEYSTOPLUGINS     = 1_bit, // НЕ передавать плагинам клавиши во время записи/воспроизведения макроса
	MFLAGS_RUNAFTERFARSTART        = 3_bit, // этот макрос запускается при старте ФАРа
	MFLAGS_EMPTYCOMMANDLINE        = 4_bit, // запускать, если командная линия пуста
	MFLAGS_NOTEMPTYCOMMANDLINE     = 5_bit, // запускать, если командная линия не пуста
	MFLAGS_EDITSELECTION           = 6_bit, // запускать, если есть выделение в редакторе
	MFLAGS_EDITNOSELECTION         = 7_bit, // запускать, если есть нет выделения в редакторе
	MFLAGS_SELECTION               = 8_bit, // активная:  запускать, если есть выделение
	MFLAGS_PSELECTION              = 9_bit, // пассивная: запускать, если есть выделение
	MFLAGS_NOSELECTION             = 10_bit, // активная:  запускать, если есть нет выделения
	MFLAGS_PNOSELECTION            = 11_bit, // пассивная: запускать, если есть нет выделения
	MFLAGS_NOFILEPANELS            = 12_bit, // активная:  запускать, если это плагиновая панель
	MFLAGS_PNOFILEPANELS           = 13_bit, // пассивная: запускать, если это плагиновая панель
	MFLAGS_NOPLUGINPANELS          = 14_bit, // активная:  запускать, если это файловая панель
	MFLAGS_PNOPLUGINPANELS         = 15_bit, // пассивная: запускать, если это файловая панель
	MFLAGS_NOFOLDERS               = 16_bit, // активная:  запускать, если текущий объект "файл"
	MFLAGS_PNOFOLDERS              = 17_bit, // пассивная: запускать, если текущий объект "файл"
	MFLAGS_NOFILES                 = 18_bit, // активная:  запускать, если текущий объект "папка"
	MFLAGS_PNOFILES                = 19_bit, // пассивная: запускать, если текущий объект "папка"
	MFLAGS_PUBLIC_MASK             = 28_bit - 1,
	// private flags, for runtime purposes only
	MFLAGS_PRIVATE_MASK            = ~MFLAGS_PUBLIC_MASK,
	MFLAGS_POSTFROMPLUGIN          = 28_bit  // последовательность пришла от АПИ
};

// для диалога назначения клавиши
struct DlgParam
{
	unsigned long long Flags;
	DWORD Key;
	FARMACROAREA Area;
	int Recurse;
	bool Changed;
};

enum {
	OP_ISEXECUTING              = 1,
	OP_ISDISABLEOUTPUT          = 2,
	OP_HISTORYDISABLEMASK       = 3,
	OP_ISHISTORYDISABLE         = 4,
	OP_ISTOPMACROOUTPUTDISABLED = 5,
	OP_ISPOSTMACROENABLED       = 6,
	OP_SETMACROVALUE            = 8,
	OP_GETINPUTFROMMACRO        = 9,
	OP_GETLASTERROR             = 11,
};

static bool ToDouble(long long v, double& d)
{
	if (constexpr long long Limit = bit(std::numeric_limits<double>::digits); v <= -Limit || v >= Limit)
		// TODO: log?
		return false;

	d = static_cast<double>(v);
	return true;
}

static const wchar_t* GetMacroLanguage(FARKEYMACROFLAGS Flags)
{
	switch(Flags & KMFLAGS_LANGMASK)
	{
		default:
		case KMFLAGS_LUA:        return L"lua";
		case KMFLAGS_MOONSCRIPT: return L"moonscript";
	}
}

static bool CallMacroPlugin(OpenMacroPluginInfo* Info)
{
	void* ptr;
	const auto result = Global->CtrlObject->Plugins->CallPlugin(Global->Opt->KnownIDs.Luamacro.Id, OPEN_LUAMACRO, Info, &ptr) != 0;
	return result && ptr;
}

static bool MacroPluginOp(int OpCode, const FarMacroValue& Param, MacroPluginReturn* Ret = nullptr)
{
	FarMacroValue values[]={static_cast<double> (OpCode),Param};
	FarMacroCall fmc={sizeof(FarMacroCall),2,values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_KEYMACRO,&fmc};
	if (CallMacroPlugin(&info))
	{
		if (Ret) *Ret=info.Ret;
		return true;
	}
	return false;
}

int KeyMacro::GetExecutingState()
{
	MacroPluginReturn Ret;
	return MacroPluginOp(OP_ISEXECUTING,false,&Ret) ? Ret.ReturnType : static_cast<int>(MACROSTATE_NOMACRO);
}

bool KeyMacro::IsOutputDisabled()
{
	MacroPluginReturn Ret;
	return MacroPluginOp(OP_ISDISABLEOUTPUT,false,&Ret)? Ret.ReturnType != 0 : false;
}

static DWORD SetHistoryDisableMask(DWORD Mask)
{
	MacroPluginReturn Ret;
	return MacroPluginOp(OP_HISTORYDISABLEMASK, static_cast<double>(Mask), &Ret)? Ret.ReturnType : 0;
}

static DWORD GetHistoryDisableMask()
{
	MacroPluginReturn Ret;
	return MacroPluginOp(OP_HISTORYDISABLEMASK,false,&Ret) ? Ret.ReturnType : 0;
}

bool KeyMacro::IsHistoryDisabled(int TypeHistory)
{
	MacroPluginReturn Ret;
	return MacroPluginOp(OP_ISHISTORYDISABLE, static_cast<double>(TypeHistory), &Ret)? !!Ret.ReturnType : false;
}

static bool IsTopMacroOutputDisabled()
{
	MacroPluginReturn Ret;
	return MacroPluginOp(OP_ISTOPMACROOUTPUTDISABLED,false,&Ret) ? !!Ret.ReturnType : false;
}

static bool IsPostMacroEnabled()
{
	MacroPluginReturn Ret;
	return MacroPluginOp(OP_ISPOSTMACROENABLED,false,&Ret) && Ret.ReturnType==1;
}

static void SetMacroValue(bool Value)
{
	MacroPluginOp(OP_SETMACROVALUE, Value);
}

static bool TryToPostMacro(FARMACROAREA Area,const string& TextKey,DWORD IntKey)
{
	FarMacroValue values[] = { 10.0, static_cast<double>(Area), TextKey, static_cast<double>(IntKey) };
	FarMacroCall fmc={sizeof(FarMacroCall),std::size(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_KEYMACRO,&fmc};
	return CallMacroPlugin(&info);
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

KeyMacro::KeyMacro():
	m_Area(MACROAREA_SHELL),
	m_StartMode(MACROAREA_OTHER),
	m_Recording(MACROSTATE_NOMACRO),
	m_InternalInput(0),
	m_WaitKey(0)
{
	//print_opcodes();
}

bool KeyMacro::LoadMacros(bool FromFar, bool InitedRAM, const FarMacroLoad *Data)
{
	if (FromFar)
	{
		if (Global->Opt->Macro.DisableMacro&MDOL_ALL) return false;
	}
	else
	{
		if (!Global->CtrlObject->Plugins->IsPluginsLoaded()) return false;
	}

	m_Recording = MACROSTATE_NOMACRO;

	FarMacroValue values[]={InitedRAM,false,0.0};
	if (Data)
	{
		if (Data->Path) values[1] = Data->Path;
		values[2] = static_cast<double>(Data->Flags);
	}
	FarMacroCall fmc={sizeof(FarMacroCall),std::size(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_LOADMACROS,&fmc};
	return CallMacroPlugin(&info);
}

bool KeyMacro::SaveMacros(bool /*always*/)
{
	OpenMacroPluginInfo info={MCT_WRITEMACROS,nullptr};
	return CallMacroPlugin(&info);
}

static long long msValues[constMsLAST];

void KeyMacro::SetMacroConst(int ConstIndex, long long Value)
{
	msValues[ConstIndex] = Value;
}

int KeyMacro::GetState() const
{
	return (m_Recording != MACROSTATE_NOMACRO) ? m_Recording : GetExecutingState();
}

static bool GetInputFromMacro(MacroPluginReturn *mpr)
{
	return MacroPluginOp(OP_GETINPUTFROMMACRO,false,mpr);
}

void KeyMacro::RestoreMacroChar() const
{
	Global->ScrBuf->RestoreMacroChar();

	if (m_Area==MACROAREA_EDITOR &&
					Global->WindowManager->GetCurrentEditor() &&
					Global->WindowManager->GetCurrentEditor()->IsVisible()
					/* && LockScr*/) // Mantis#0001595
	{
		Global->WindowManager->GetCurrentEditor()->Show();
	}
	else if (m_Area==MACROAREA_VIEWER &&
					Global->WindowManager->GetCurrentViewer() &&
					Global->WindowManager->GetCurrentViewer()->IsVisible())
	{
		Global->WindowManager->GetCurrentViewer()->Show(); // иначе может быть неправильный верхний левый символ экрана
	}
}

struct GetMacroData
{
	FARMACROAREA Area;
	const wchar_t *Code;
	const wchar_t *Description;
	MACROFLAGS_MFLAGS Flags;
	bool IsKeyboardMacro;
};

static bool LM_GetMacro(GetMacroData* Data, FARMACROAREA Area, const string& TextKey, bool UseCommon)
{
	FarMacroValue InValues[] = { static_cast<double>(Area),TextKey,UseCommon };
	FarMacroCall fmc={sizeof(FarMacroCall),std::size(InValues),InValues,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_GETMACRO,&fmc};

	if (CallMacroPlugin(&info) && info.Ret.Count>=5)
	{
		const auto* Values = info.Ret.Values;
		Data->Area        = static_cast<FARMACROAREA>(static_cast<int>(Values[0].Double));
		Data->Code        = Values[1].Type==FMVT_STRING ? Values[1].String : L"";
		Data->Description = Values[2].Type==FMVT_STRING ? Values[2].String : L"";
		Data->Flags       = static_cast<MACROFLAGS_MFLAGS>(Values[3].Double);
		Data->IsKeyboardMacro = Values[4].Boolean != 0;
		return true;
	}
	return false;
}

bool KeyMacro::MacroExists(int Key, FARMACROAREA Area, bool UseCommon)
{
	GetMacroData dummy;
	const auto KeyName = KeyToText(Key);
	return !KeyName.empty() && LM_GetMacro(&dummy, Area, KeyName, UseCommon);
}

static void LM_ProcessRecordedMacro(FARMACROAREA Area, const string& TextKey, const string& Code,
	MACROFLAGS_MFLAGS Flags, const string& Description)
{
	FarMacroValue values[] = { static_cast<double>(Area), TextKey, Code, Flags, Description };
	FarMacroCall fmc={sizeof(FarMacroCall),std::size(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_RECORDEDMACRO,&fmc};
	CallMacroPlugin(&info);
}

bool KeyMacro::ProcessEvent(const FAR_INPUT_RECORD *Rec)
{
	if (m_InternalInput || any_of(Rec->IntKey, KEY_IDLE, KEY_NONE) || !Global->WindowManager->GetCurrentWindow()) //FIXME: избавиться от Rec->IntKey
		return false;

	const auto textKey = KeyToText(Rec->IntKey);
	if (textKey.empty())
		return false;

	const auto ctrldot = Rec->IntKey == Global->Opt->Macro.KeyMacroCtrlDot || Rec->IntKey == Global->Opt->Macro.KeyMacroRCtrlDot;
	const auto ctrlshiftdot = Rec->IntKey == Global->Opt->Macro.KeyMacroCtrlShiftDot || Rec->IntKey == Global->Opt->Macro.KeyMacroRCtrlShiftDot;

	if (m_Recording == MACROSTATE_NOMACRO)
	{
		if ((ctrldot||ctrlshiftdot) && !IsExecuting())
		{
			const auto LuaMacro = Global->CtrlObject->Plugins->FindPlugin(Global->Opt->KnownIDs.Luamacro.Id);
			if (!LuaMacro || LuaMacro->IsPendingRemove())
			{
				Message(MSG_WARNING,
					msg(lng::MError),
					{
						msg(lng::MMacroPluginLuamacroNotLoaded),
						msg(lng::MMacroRecordingIsDisabled)
					},
					{ lng::MOk });
				return false;
			}

			// Где мы?
			m_StartMode=m_Area;
			// В зависимости от того, КАК НАЧАЛИ писать макрос, различаем общий режим (Ctrl-.
			// с передачей плагину кеев) или специальный (Ctrl-Shift-. - без передачи клавиш плагину)
			m_Recording=ctrldot?MACROSTATE_RECORDING_COMMON:MACROSTATE_RECORDING;

			m_RecCode.clear();
			m_RecDescription.clear();
			Global->ScrBuf->Flush();
			return true;
		}
		else
		{
			if (!m_WaitKey && IsPostMacroEnabled())
			{
				auto key = Rec->IntKey;
				if ((key&0x00FFFFFF) > 0x7F && (key&0x00FFFFFF) < 0xFFFF)
					key=KeyToKeyLayout(key&0x0000FFFF)|(key&~0x0000FFFF);

				if (key<0xFFFF)
					key=upper(static_cast<wchar_t>(key));

				if (TryToPostMacro(m_Area, key == Rec->IntKey? textKey : KeyToText(key), Rec->IntKey))
					return true;
			}
		}
	}
	else // m_Recording!=MACROSTATE_NOMACRO
	{
		if (ctrldot||ctrlshiftdot) // признак конца записи?
		{
			m_InternalInput=1;
			DWORD MacroKey;
			// выставляем флаги по умолчанию.
			unsigned long long Flags = 0;
			int AssignRet=AssignMacroKey(MacroKey,Flags);

			if (AssignRet && AssignRet!=2 && !m_RecCode.empty())
			{
				m_RecCode = concat(L"Keys(\""sv, m_RecCode, L"\")"sv);
				// добавим проверку на удаление
				// если удаляем или был вызван диалог изменения, то не нужно выдавать диалог настройки.
				//if (MacroKey != (DWORD)-1 && (Key==KEY_CTRLSHIFTDOT || Recording==2) && RecBufferSize)
				if (ctrlshiftdot && !GetMacroSettings(MacroKey,Flags))
				{
					AssignRet=0;
				}
			}
			m_InternalInput=0;
			if (AssignRet)
			{
				const auto strKey = KeyToText(MacroKey);
				Flags |= m_Recording == MACROSTATE_RECORDING_COMMON? MFLAGS_NONE : MFLAGS_NOSENDKEYSTOPLUGINS;
				LM_ProcessRecordedMacro(m_StartMode, strKey, m_RecCode, Flags, m_RecDescription);
			}

			m_Recording=MACROSTATE_NOMACRO;
			m_RecCode.clear();
			m_RecDescription.clear();
			Global->ScrBuf->RestoreMacroChar();

			if (Global->Opt->AutoSaveSetup)
				SaveMacros(false); // записать только изменения!

			return true;
		}
		else
		{
			if (!Global->IsProcessAssignMacroKey)
			{
				if (!m_RecCode.empty())
					m_RecCode += L' ';

				m_RecCode += textKey == L"\""sv? L"\\\""sv : textKey;
			}
			return false;
		}
	}

	return false;
}

static void ShowUserMenu(size_t Count, const FarMacroValue *Values)
{
	if (Count==0)
		UserMenu(false);
	else if (Values[0].Type==FMVT_BOOLEAN)
		UserMenu(Values[0].Boolean != 0);
	else if (Values[0].Type==FMVT_STRING)
		UserMenu(string(Values[0].String));
}

int KeyMacro::GetKey()
{
	if (m_InternalInput || !Global->WindowManager->GetCurrentWindow())
		return 0;

	MacroPluginReturn mpr;
	while (GetInputFromMacro(&mpr))
	{
		switch (mpr.ReturnType)
		{
			default:
				return 0;

			case MPRT_HASNOMACRO:
				if (m_Area==MACROAREA_EDITOR &&
								Global->WindowManager->GetCurrentEditor() &&
								Global->WindowManager->GetCurrentEditor()->IsVisible() &&
								Global->ScrBuf->GetLockCount())
				{
					Global->WindowManager->GetCurrentEditor()->Show();
				}

				Global->ScrBuf->Unlock();

				default_clipboard_mode::set(clipboard_mode::system);

				return 0;

			case MPRT_KEYS:
			{
				switch (static_cast<int>(mpr.Values[0].Double))
				{
					case 1:
						return KEY_OP_SELWORD;
					case 2:
						return KEY_OP_XLAT;
					default:
						return static_cast<int>(mpr.Values[1].Double);
				}
			}

			case MPRT_PRINT:
			{
				m_StringToPrint = mpr.Values[0].String;
				return KEY_OP_PLAINTEXT;
			}

			case MPRT_PLUGINMENU:   // N=Plugin.Menu(Uuid[,MenuUuid])
			case MPRT_PLUGINCONFIG: // N=Plugin.Config(Uuid[,MenuUuid])
			case MPRT_PLUGINCOMMAND: // N=Plugin.Command(Uuid[,Command])
			{
				SetMacroValue(false);

				if (!mpr.Count || mpr.Values[0].Type != FMVT_STRING)
					break;

				const auto Uuid = uuid::try_parse(string_view(mpr.Values[0].String));
				if (!Uuid)
					break;

				if (!Global->CtrlObject->Plugins->FindPlugin(*Uuid))
					break;

				PluginManager::CallPluginInfo cpInfo = { CPT_CHECKONLY };
				const auto Arg = mpr.Count > 1 && mpr.Values[1].Type == FMVT_STRING? mpr.Values[1].String : L"";

				UUID MenuUuid;
				if (*Arg && (mpr.ReturnType==MPRT_PLUGINMENU || mpr.ReturnType==MPRT_PLUGINCONFIG))
				{
					if (const auto MenuUuidOpt = uuid::try_parse(string_view(Arg)))
					{
						MenuUuid = *MenuUuidOpt;
						cpInfo.ItemUuid = &MenuUuid;
					}
					else
						break;
				}

				if (mpr.ReturnType == MPRT_PLUGINMENU)
					cpInfo.CallFlags |= CPT_MENU;
				else if (mpr.ReturnType == MPRT_PLUGINCONFIG)
					cpInfo.CallFlags |= CPT_CONFIGURE;
				else if (mpr.ReturnType == MPRT_PLUGINCOMMAND)
				{
					cpInfo.CallFlags |= CPT_CMDLINE;
					cpInfo.Command = Arg;
				}

				// Чтобы вернуть результат "выполнения" нужно проверить наличие плагина/пункта
				if (Global->CtrlObject->Plugins->CallPluginItem(*Uuid, &cpInfo))
				{
					// Если нашли успешно - то теперь выполнение
					SetMacroValue(true);
					cpInfo.CallFlags&=~CPT_CHECKONLY;
					Global->CtrlObject->Plugins->CallPluginItem(*Uuid, &cpInfo);
				}
				Global->WindowManager->RefreshWindow();
				//с текущим переключением окон могут быть проблемы с заголовком консоли.
				Global->WindowManager->PluginCommit();

				break;
			}

			case MPRT_USERMENU:
				ShowUserMenu(mpr.Count,mpr.Values);
				break;
		}
	}

	return 0;
}

int KeyMacro::PeekKey() const
{
	return !m_InternalInput && IsExecuting();
}

bool KeyMacro::GetMacroKeyInfo(const string& StrArea, int Pos, string &strKeyName, string &strDescription)
{
	FarMacroValue values[]={StrArea,!Pos};
	FarMacroCall fmc={sizeof(FarMacroCall),std::size(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_ENUMMACROS,&fmc};

	if (CallMacroPlugin(&info) && info.Ret.Count >= 2)
	{
		strKeyName = info.Ret.Values[0].String;
		strDescription = info.Ret.Values[1].String;
		return true;
	}
	return false;
}

bool KeyMacro::CheckWaitKeyFunc() const
{
	return m_WaitKey != 0;
}

// Функция, запускающая макросы при старте ФАРа
void KeyMacro::RunStartMacro()
{
	if (Global->Opt->Macro.DisableMacro & (MDOL_ALL|MDOL_AUTOSTART))
		return;

	if (!Global->CtrlObject || !Global->CtrlObject->Cp() || !Global->CtrlObject->Cp()->ActivePanel() || !Global->CtrlObject->Plugins->IsPluginsLoaded())
		return;

	static bool IsRunStartMacro=false, IsInside=false;

	if (!IsRunStartMacro && !IsInside)
	{
		IsInside = true;
		OpenMacroPluginInfo info = {MCT_RUNSTARTMACRO,nullptr};
		IsRunStartMacro = CallMacroPlugin(&info);
		IsInside = false;
	}
}

bool KeyMacro::AddMacro(const UUID& PluginId, const MacroAddMacroV1* Data)
{
	if (!(Data->Area >= 0 && (Data->Area < MACROAREA_LAST || Data->Area == MACROAREA_COMMON)))
		return false;

	const auto KeyText = InputRecordToText(&Data->AKey);
	if (KeyText.empty())
		return false;

	MACROFLAGS_MFLAGS Flags = 0;
	if (Data->Flags & KMFLAGS_ENABLEOUTPUT)        Flags |= MFLAGS_ENABLEOUTPUT;
	if (Data->Flags & KMFLAGS_NOSENDKEYSTOPLUGINS) Flags |= MFLAGS_NOSENDKEYSTOPLUGINS;

	intptr_t Priority = 50;
	if (Data->StructSize >= sizeof(MacroAddMacro)) Priority = reinterpret_cast<const MacroAddMacro*>(Data)->Priority;
	FarMacroValue values[] = {
		static_cast<double>(Data->Area),
		KeyText,
		GetMacroLanguage(Data->Flags),
		Data->SequenceText,
		Flags,
		Data->Description,
		PluginId,
		{reinterpret_cast<void*>(Data->Callback)},
		Data->Id,
		Priority
	};
	FarMacroCall fmc = {sizeof(FarMacroCall),std::size(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info = {MCT_ADDMACRO,&fmc};
	return CallMacroPlugin(&info);
}

bool KeyMacro::DelMacro(const UUID& PluginId, void* Id)
{
	FarMacroValue values[]={PluginId,Id};
	FarMacroCall fmc={sizeof(FarMacroCall),std::size(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_DELMACRO,&fmc};
	return CallMacroPlugin(&info);
}

bool KeyMacro::PostNewMacro(const wchar_t* Sequence,FARKEYMACROFLAGS InputFlags,DWORD AKey)
{
	const wchar_t* Lang = GetMacroLanguage(InputFlags);
	const auto onlyCheck = (InputFlags & KMFLAGS_SILENTCHECK) != 0;
	MACROFLAGS_MFLAGS Flags = MFLAGS_POSTFROMPLUGIN;
	if (InputFlags & KMFLAGS_ENABLEOUTPUT)        Flags |= MFLAGS_ENABLEOUTPUT;
	if (InputFlags & KMFLAGS_NOSENDKEYSTOPLUGINS) Flags |= MFLAGS_NOSENDKEYSTOPLUGINS;

	FarMacroValue values[] = { 7.0, Lang, Sequence, static_cast<double>(Flags), static_cast<double>(AKey), onlyCheck };
	FarMacroCall fmc={sizeof(FarMacroCall),std::size(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_KEYMACRO,&fmc};
	return CallMacroPlugin(&info);
}

static bool CheckEditSelected(FARMACROAREA Area, unsigned long long CurFlags)
{
	if (Area==MACROAREA_EDITOR || Area==MACROAREA_DIALOG || Area==MACROAREA_VIEWER || (Area==MACROAREA_SHELL&&Global->CtrlObject->CmdLine()->IsVisible()))
	{
		const auto NeedType = Area == MACROAREA_EDITOR?windowtype_editor:(Area == MACROAREA_VIEWER?windowtype_viewer:(Area == MACROAREA_DIALOG?windowtype_dialog:windowtype_panels));
		const auto CurrentWindow = Global->WindowManager->GetCurrentWindow();

		if (CurrentWindow && CurrentWindow->GetType()==NeedType)
		{
			int CurSelected;

			if (Area==MACROAREA_SHELL && Global->CtrlObject->CmdLine()->IsVisible())
				CurSelected = static_cast<int>(Global->CtrlObject->CmdLine()->VMProcess(MCODE_C_SELECTED));
			else
				CurSelected = static_cast<int>(CurrentWindow->VMProcess(MCODE_C_SELECTED));

			if ((CurFlags&MFLAGS_EDITSELECTION && !CurSelected) || (CurFlags&MFLAGS_EDITNOSELECTION && CurSelected))
				return false;
		}
	}

	return true;
}

static bool CheckCmdLine(bool IsEmpty, MACROFLAGS_MFLAGS CurFlags)
{
	return !(!IsEmpty && (CurFlags&MFLAGS_EMPTYCOMMANDLINE))
		&& !(IsEmpty && (CurFlags&MFLAGS_NOTEMPTYCOMMANDLINE));
}

static bool CheckPanel(panel_mode PanelMode, MACROFLAGS_MFLAGS CurFlags, bool IsPassivePanel)
{
	static const MACROFLAGS_MFLAGS FlagsMapping[][2] =
	{
		{ MFLAGS_NOFILEPANELS, MFLAGS_NOPLUGINPANELS },
		{ MFLAGS_PNOFILEPANELS, MFLAGS_PNOPLUGINPANELS },
	};

	const auto Flag = FlagsMapping[IsPassivePanel? 1 : 0][PanelMode == panel_mode::PLUGIN_PANEL? 1 : 0];
	return !(CurFlags & Flag);
}

static bool CheckFileFolder(panel_ptr CheckPanel, MACROFLAGS_MFLAGS CurFlags, bool IsPassivePanel)
{
	string strFileName;
	os::fs::attributes FileAttr;
	if (!CheckPanel->GetFileName(strFileName, CheckPanel->GetCurrentPos(), FileAttr))
		return true;

	static const MACROFLAGS_MFLAGS FlagsMapping[][2] =
	{
		{ MFLAGS_NOFILES, MFLAGS_NOFOLDERS },
		{ MFLAGS_PNOFILES, MFLAGS_PNOFOLDERS },
	};

	const auto Flag = FlagsMapping[IsPassivePanel? 1 : 0][FileAttr & FILE_ATTRIBUTE_DIRECTORY? 1 : 0];
	return !(CurFlags & Flag);
}

static bool CheckAll(FARMACROAREA Area, MACROFLAGS_MFLAGS CurFlags)
{
	/* $TODO:
		Здесь вместо Check*() попробовать заюзать IfCondition()
		для исключения повторяющегося кода.
	*/

	// проверка на пусто/не пусто в ком.строке (а в редакторе? :-)
	if (CurFlags&(MFLAGS_EMPTYCOMMANDLINE|MFLAGS_NOTEMPTYCOMMANDLINE))
		if (Global->CtrlObject->CmdLine() && !CheckCmdLine(Global->CtrlObject->CmdLine()->GetString().empty(), CurFlags))
			return false;

	const auto Cp = Global->CtrlObject->Cp();
	if (!Cp)
		return false;

	// проверки панели и типа файла
	const auto ActivePanel = Cp->ActivePanel();
	const auto PassivePanel = Cp->PassivePanel();

	if (ActivePanel && PassivePanel)// && (CurFlags&MFLAGS_MODEMASK)==MACROAREA_SHELL)
	{
		if (CurFlags & (MFLAGS_NOPLUGINPANELS | MFLAGS_NOFILEPANELS))
			if (!CheckPanel(ActivePanel->GetMode(), CurFlags, false))
				return false;

		if (CurFlags & (MFLAGS_PNOPLUGINPANELS | MFLAGS_PNOFILEPANELS))
			if (!CheckPanel(PassivePanel->GetMode(), CurFlags, true))
				return false;

		if (CurFlags & (MFLAGS_NOFOLDERS | MFLAGS_NOFILES))
			if (!CheckFileFolder(ActivePanel, CurFlags, false))
				return false;

		if (CurFlags & (MFLAGS_PNOFOLDERS | MFLAGS_PNOFILES))
			if (!CheckFileFolder(PassivePanel, CurFlags, true))
				return false;

		if (CurFlags & (MFLAGS_SELECTION | MFLAGS_NOSELECTION | MFLAGS_PSELECTION | MFLAGS_PNOSELECTION))
			if (Area != MACROAREA_EDITOR && Area != MACROAREA_DIALOG && Area != MACROAREA_VIEWER)
			{
				const auto ActiveSelCount = ActivePanel->GetRealSelCount();
				if ((CurFlags&MFLAGS_SELECTION && ActiveSelCount < 1) || (CurFlags&MFLAGS_NOSELECTION && ActiveSelCount >= 1))
					return false;

				const auto PassiveSelCount = PassivePanel->GetRealSelCount();
				if ((CurFlags&MFLAGS_PSELECTION && PassiveSelCount < 1) || (CurFlags&MFLAGS_PNOSELECTION && PassiveSelCount >= 1))
					return false;
			}
	}

	return CheckEditSelected(Area, CurFlags);
}

static int Set3State(DWORD Flags,DWORD Chk1,DWORD Chk2)
{
	const auto Chk12 = Chk1 | Chk2, FlagsChk12 = Flags & Chk12;

	if (FlagsChk12 == Chk12 || !FlagsChk12)
		return 2;
	else
		return (Flags & Chk1)? 1 : 0;
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

	MS_COUNT
};

intptr_t KeyMacro::ParamMacroDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	switch (Msg)
	{
		case DN_BTNCLICK:

			if (Param1==MS_CHECKBOX_A_PANEL || Param1==MS_CHECKBOX_P_PANEL)
				for (int i=1; i<=3; i++)
					Dlg->SendMessage(DM_ENABLE,Param1+i,Param2);

			break;
		case DN_CLOSE:

			if (Param1==MS_BUTTON_OK)
			{
				const auto Sequence = reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, MS_EDIT_SEQUENCE, nullptr));
				if (*Sequence)
				{
					if (ParseMacroString(Sequence,KMFLAGS_LUA,true))
					{
						m_RecCode=Sequence;
						m_RecDescription = reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, MS_EDIT_DESCR, nullptr));
						return TRUE;
					}
				}

				return FALSE;
			}

			break;

		default:
			break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

bool KeyMacro::GetMacroSettings(int Key, unsigned long long& Flags, string_view Src, string_view Descr)
{
	/*
	          1         2         3         4         5         6
	   3456789012345678901234567890123456789012345678901234567890123456789
	 1 ╔══════════════════ Macro settings for 'CtrlP' ═══════════════════╗
	 2 ║ Sequence:                                                       ║
	 3 ║ _______________________________________________________________↓║
	 4 ║ Description:                                                    ║
	 5 ║ _______________________________________________________________↓║
	 6 ╟─────────────────────────────────────────────────────────────────╢
	 7 ║ [ ] Allow screen output while executing macro                   ║
	 8 ║ [ ] Execute after Far start                                     ║
	 9 ╟─────────────────────────────────────────────────────────────────╢
	10 ║ [ ] Active panel                [ ] Passive panel               ║
	11 ║   [?] Plugin panel                [?] Plugin panel              ║
	12 ║   [?] Execute for folders         [?] Execute for folders       ║
	13 ║   [?] Selection present           [?] Selection present         ║
	14 ╟─────────────────────────────────────────────────────────────────╢
	15 ║ [?] Empty command line                                          ║
	16 ║ [?] Selection block present                                     ║
	17 ╟─────────────────────────────────────────────────────────────────╢
	18 ║                        { OK } [ Cancel ]                        ║
	19 ╚═════════════════════════════════════════════════════════════════╝
	*/
	auto MacroSettingsDlg = MakeDialogItems<MS_COUNT>(
	{
		{ DI_DOUBLEBOX, {{3,  1 }, {69, 19}}, DIF_NONE, },
		{ DI_TEXT,      {{5,  2 }, {0,  2 }}, DIF_NONE, msg(lng::MMacroSettingsSequence), },
		{ DI_EDIT,      {{5,  3 }, {67, 3 }}, DIF_FOCUS | DIF_HISTORY, },
		{ DI_TEXT,      {{5,  4 }, {0,  4 }}, DIF_NONE, msg(lng::MMacroSettingsDescription), },
		{ DI_EDIT,      {{5,  5 }, {67, 5 }}, DIF_HISTORY, },
		{ DI_TEXT,      {{-1, 6 }, {0,  6 }}, DIF_SEPARATOR, },
		{ DI_CHECKBOX,  {{5,  7 }, {0,  7 }}, DIF_NONE, msg(lng::MMacroSettingsEnableOutput), },
		{ DI_CHECKBOX,  {{5,  8 }, {0,  8 }}, DIF_NONE, msg(lng::MMacroSettingsRunAfterStart), },
		{ DI_TEXT,      {{-1, 9 }, {0,  9 }}, DIF_SEPARATOR, },
		{ DI_CHECKBOX,  {{5,  10}, {0,  10}}, DIF_NONE, msg(lng::MMacroSettingsActivePanel), },
		{ DI_CHECKBOX,  {{7,  11}, {0,  11}}, DIF_3STATE | DIF_DISABLE, msg(lng::MMacroSettingsPluginPanel), },
		{ DI_CHECKBOX,  {{7,  12}, {0,  12}}, DIF_3STATE | DIF_DISABLE, msg(lng::MMacroSettingsFolders), },
		{ DI_CHECKBOX,  {{7,  13}, {0,  13}}, DIF_3STATE | DIF_DISABLE, msg(lng::MMacroSettingsSelectionPresent), },
		{ DI_CHECKBOX,  {{37, 10}, {0,  10}}, DIF_NONE, msg(lng::MMacroSettingsPassivePanel), },
		{ DI_CHECKBOX,  {{39, 11}, {0,  11}}, DIF_3STATE | DIF_DISABLE, msg(lng::MMacroSettingsPluginPanel), },
		{ DI_CHECKBOX,  {{39, 12}, {0,  12}}, DIF_3STATE | DIF_DISABLE, msg(lng::MMacroSettingsFolders), },
		{ DI_CHECKBOX,  {{39, 13}, {0,  13}}, DIF_3STATE | DIF_DISABLE, msg(lng::MMacroSettingsSelectionPresent), },
		{ DI_TEXT,      {{-1, 14}, {0,  14}}, DIF_SEPARATOR, },
		{ DI_CHECKBOX,  {{5,  15}, {0,  15}}, DIF_3STATE, msg(lng::MMacroSettingsCommandLine), },
		{ DI_CHECKBOX,  {{5,  16}, {0,  16}}, DIF_3STATE, msg(lng::MMacroSettingsSelectionBlockPresent), },
		{ DI_TEXT,      {{-1, 17}, {0,  17}}, DIF_SEPARATOR, },
		{ DI_BUTTON,    {{0,  18}, {0,  18}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MOk), },
		{ DI_BUTTON,    {{0,  18}, {0,  18}}, DIF_CENTERGROUP, msg(lng::MCancel), },
	});

	MacroSettingsDlg[MS_EDIT_SEQUENCE].strHistory = L"MacroSequence"sv;
	MacroSettingsDlg[MS_EDIT_DESCR].strHistory = L"MacroDescription"sv;

	MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected = 2;
	MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected = 2;
	MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected = 2;

	MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected = 2;
	MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected = 2;
	MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected = 2;

	MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected = 2;
	MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected = 2;

	MacroSettingsDlg[MS_DOUBLEBOX].strData = format(msg(lng::MMacroSettingsTitle), KeyToText(Key));
	//if(!(Key&0x7F000000))
	//MacroSettingsDlg[3].Flags|=DIF_DISABLE;
	MacroSettingsDlg[MS_CHECKBOX_OUPUT].Selected=Flags&MFLAGS_ENABLEOUTPUT?1:0;
	MacroSettingsDlg[MS_CHECKBOX_START].Selected=Flags&MFLAGS_RUNAFTERFARSTART?1:0;
	MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected=Set3State(Flags,MFLAGS_NOFILEPANELS,MFLAGS_NOPLUGINPANELS);
	MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected=Set3State(Flags,MFLAGS_NOFILES,MFLAGS_NOFOLDERS);
	MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected=Set3State(Flags,MFLAGS_SELECTION,MFLAGS_NOSELECTION);
	MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected=Set3State(Flags,MFLAGS_PNOFILEPANELS,MFLAGS_PNOPLUGINPANELS);
	MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected=Set3State(Flags,MFLAGS_PNOFILES,MFLAGS_PNOFOLDERS);
	MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected=Set3State(Flags,MFLAGS_PSELECTION,MFLAGS_PNOSELECTION);
	MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected=Set3State(Flags,MFLAGS_EMPTYCOMMANDLINE,MFLAGS_NOTEMPTYCOMMANDLINE);
	MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected=Set3State(Flags,MFLAGS_EDITSELECTION,MFLAGS_EDITNOSELECTION);
	MacroSettingsDlg[MS_EDIT_SEQUENCE].strData = Src.empty()? m_RecCode : Src;
	MacroSettingsDlg[MS_EDIT_DESCR].strData = Descr.empty()? m_RecDescription : Descr;

	DlgParam Param={0, 0, MACROAREA_OTHER, 0, false};
	const auto Dlg = Dialog::create(MacroSettingsDlg, &KeyMacro::ParamMacroDlgProc, this, &Param);
	Dlg->SetPosition({ -1, -1, 73, 21 });
	Dlg->SetHelp(L"KeyMacroSetting"sv);
	Dlg->Process();

	if (Dlg->GetExitCode()!=MS_BUTTON_OK)
		return false;

	enum key_id
	{
		key_output,
		key_start,
		key_pluginpanel,
		key_folders,
		key_selection,
		key_cmdline,
		key_selblock,

		key_count
	};

	static const MACROFLAGS_MFLAGS Mapping[][3] =
	{
		// [ ] [x] [?]
		{ MFLAGS_NONE, MFLAGS_ENABLEOUTPUT, MFLAGS_NONE },
		{ MFLAGS_NONE, MFLAGS_RUNAFTERFARSTART, MFLAGS_NONE },
		{ MFLAGS_NOPLUGINPANELS, MFLAGS_NOFILEPANELS, MFLAGS_NONE },
		{ MFLAGS_NOFOLDERS, MFLAGS_NOFILES, MFLAGS_NONE },
		{ MFLAGS_NOSELECTION, MFLAGS_SELECTION, MFLAGS_NONE },
		{ MFLAGS_NOTEMPTYCOMMANDLINE, MFLAGS_EMPTYCOMMANDLINE, MFLAGS_NONE },
		{ MFLAGS_EDITNOSELECTION, MFLAGS_EDITSELECTION, MFLAGS_NONE },
	};

	static_assert(key_count == std::size(Mapping));

	const auto get_flag = [&](MACROSETTINGSDLG ControlId, key_id KeyId)
	{
		return Mapping[KeyId][MacroSettingsDlg[ControlId].Selected];
	};

	Flags =
		get_flag(MS_CHECKBOX_OUPUT, key_output) |
		get_flag(MS_CHECKBOX_START, key_start) |
		get_flag(MS_CHECKBOX_CMDLINE, key_cmdline) |
		get_flag(MS_CHECKBOX_SELBLOCK, key_selblock);

	if (MacroSettingsDlg[MS_CHECKBOX_A_PANEL].Selected)
	{
		Flags |=
			get_flag(MS_CHECKBOX_A_PLUGINPANEL, key_pluginpanel) |
			get_flag(MS_CHECKBOX_A_FOLDERS, key_folders) |
			get_flag(MS_CHECKBOX_A_SELECTION, key_selection);
	}

	if (MacroSettingsDlg[MS_CHECKBOX_P_PANEL].Selected)
	{
		Flags |=
			get_flag(MS_CHECKBOX_P_PLUGINPANEL, key_pluginpanel) |
			get_flag(MS_CHECKBOX_P_FOLDERS, key_folders) |
			get_flag(MS_CHECKBOX_P_SELECTION, key_selection);
	}

	return true;
}

bool KeyMacro::ParseMacroString(const wchar_t* Sequence, FARKEYMACROFLAGS Flags, bool skipFile) const
{
	const wchar_t* lang = GetMacroLanguage(Flags);
	const auto onlyCheck = (Flags&KMFLAGS_SILENTCHECK) != 0;

	// Перекладываем вывод сообщения об ошибке на плагин, т.к. штатный Message()
	// не умеет сворачивать строки и обрезает сообщение.
	FarMacroValue values[]={lang,Sequence,onlyCheck,skipFile};
	FarMacroCall fmc={sizeof(FarMacroCall),std::size(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_MACROPARSE,&fmc};

	if (CallMacroPlugin(&info))
	{
		if (info.Ret.ReturnType == MPRT_NORMALFINISH)
		{
			return true;
		}
		else if (info.Ret.ReturnType == MPRT_ERRORPARSE)
		{
			if (!onlyCheck)
			{
				RestoreMacroChar();
				Global->WindowManager->RefreshWindow(); // Нужно после вывода сообщения плагином. Иначе панели не перерисовываются.
			}
		}
	}
	return false;
}

bool KeyMacro::ExecuteString(MacroExecuteString *Data)
{
	const auto onlyCheck = (Data->Flags & KMFLAGS_SILENTCHECK) != 0;
	FarMacroValue values[]={GetMacroLanguage(Data->Flags), Data->SequenceText, FarMacroValue(Data->InValues,Data->InCount), onlyCheck};
	FarMacroCall fmc={sizeof(FarMacroCall),std::size(values),values,nullptr,nullptr};
	OpenMacroPluginInfo info={MCT_EXECSTRING,&fmc};

	if (CallMacroPlugin(&info) && info.Ret.ReturnType == MPRT_NORMALFINISH)
	{
		Data->OutValues = info.Ret.Values;
		Data->OutCount = info.Ret.Count;
		return true;
	}
	Data->OutCount = 0;
	return false;
}

DWORD KeyMacro::GetMacroParseError(point& ErrPos, string& ErrSrc)
{
	MacroPluginReturn Ret;
	if (MacroPluginOp(OP_GETLASTERROR, false, &Ret))
	{
		ErrSrc = Ret.Values[0].String;
		ErrPos.y = static_cast<int>(Ret.Values[1].Double);
		ErrPos.x = static_cast<int>(Ret.Values[2].Double);
		return ErrSrc.empty() ? MPEC_SUCCESS : MPEC_ERROR;
	}
	else
	{
		ErrSrc = L"No response from macro plugin"sv;
		ErrPos = {};
		return MPEC_ERROR;
	}
}

class FarMacroApi
{
public:
	explicit FarMacroApi(FarMacroCall* Data) : mData(Data) {}

	int PassBoolean(int b);
	int PassError(const wchar_t* str);
	int PassInteger(long long Int);
	int PassNumber(double dbl);
	int PassString(const wchar_t* str);
	int PassString(const string& str);
	int PassValue(const TVar& Var);

	int absFunc();
	int ascFunc();
	int atoiFunc();
	int beepFunc();
	int chrFunc();
	int clipFunc();
	int dateFunc();
	int dlggetvalueFunc();
	int dlgsetfocusFunc();
	int editordellineFunc();
	int editorinsstrFunc();
	int editorposFunc();
	int editorselFunc();
	int editorsetFunc();
	int editorsetstrFunc();
	int editorsettitleFunc();
	int editorundoFunc();
	int environFunc();
	int farcfggetFunc();
	int fargetconfigFunc();
	int fattrFunc();
	int fexistFunc();
	int floatFunc();
	int flockFunc();
	int fmatchFunc();
	int fsplitFunc();
	int indexFunc();
	int intFunc();
	int itowFunc();
	int kbdLayoutFunc();
	int keyFunc();
	int keybarshowFunc();
	int lcaseFunc();
	int lenFunc();
	int maxFunc();
	int menushowFunc();
	int minFunc();
	int modFunc();
	int msgBoxFunc();
	int panelfattrFunc();
	int panelfexistFunc();
	int panelitemFunc();
	int panelselectFunc();
	int panelsetposFunc();
	int panelsetposidxFunc();
	int pluginexistFunc();
	int pluginloadFunc();
	int pluginunloadFunc();
	int promptFunc();
	int replaceFunc();
	int rindexFunc();
	int size2strFunc();
	int sleepFunc();
	int stringFunc();
	int strpadFunc();
	int strwrapFunc();
	int substrFunc();
	int testfolderFunc();
	int trimFunc();
	int ucaseFunc();
	int waitkeyFunc();
	int windowscrollFunc();
	int xlatFunc();

private:
	int fattrFuncImpl(int Type);

	FarMacroCall* mData;
};

int FarMacroApi::PassString(const wchar_t *str)
{
	FarMacroValue val = NullToEmpty(str);
	mData->Callback(mData->CallbackData, &val, 1);
	return 1;
}

int FarMacroApi::PassString(const string& str)
{
	return PassString(str.c_str());
}

int FarMacroApi::PassError(const wchar_t *str)
{
	FarMacroValue val = NullToEmpty(str);
	val.Type = FMVT_ERROR;
	mData->Callback(mData->CallbackData, &val, 1);
	return 1;
}

int FarMacroApi::PassNumber(double dbl)
{
	FarMacroValue val = dbl;
	mData->Callback(mData->CallbackData, &val, 1);
	return 1;
}

int FarMacroApi::PassInteger(long long Int)
{
	FarMacroValue val = Int;
	mData->Callback(mData->CallbackData, &val, 1);
	return 1;
}

int FarMacroApi::PassBoolean(int b)
{
	FarMacroValue val = (b != 0);
	mData->Callback(mData->CallbackData, &val, 1);
	return 1;
}

int FarMacroApi::PassValue(const TVar& Var)
{
	FarMacroValue val;
	double dd;

	if (Var.isDouble())
		val = Var.asDouble();
	else if (Var.isString())
		val = Var.asString();
	else if (ToDouble(Var.asInteger(), dd))
		val = dd;
	else
		val = Var.asInteger();

	mData->Callback(mData->CallbackData, &val, 1);
	return 1;
}

static auto parseParams(size_t Count, FarMacroCall* Data)
{
	const auto argNum = std::min(Data->Count, Count);
	std::vector<TVar> Params;
	Params.reserve(Count);
	std::transform(Data->Values, Data->Values + argNum, std::back_inserter(Params), [](const auto& i)
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

intptr_t KeyMacro::CallFar(intptr_t CheckCode, FarMacroCall* Data)
{
	intptr_t ret=0;
	os::fs::attributes FileAttr = INVALID_FILE_ATTRIBUTES;
	FarMacroApi api(Data);

	// проверка на область
	if (CheckCode == 0)
	{
		return api.PassNumber (Global->WindowManager->GetCurrentWindow()->GetMacroArea());
	}

	const auto ActivePanel = Global->CtrlObject->Cp() ? Global->CtrlObject->Cp()->ActivePanel() : nullptr;
	const auto PassivePanel = Global->CtrlObject->Cp() ? Global->CtrlObject->Cp()->PassivePanel() : nullptr;

	auto CurrentWindow = Global->WindowManager->GetCurrentWindow();

	switch (CheckCode)
	{
		case MCODE_C_MSX:             return api.PassNumber(msValues[constMsX]);
		case MCODE_C_MSY:             return api.PassNumber(msValues[constMsY]);
		case MCODE_C_MSBUTTON:        return api.PassNumber(msValues[constMsButton]);
		case MCODE_C_MSCTRLSTATE:     return api.PassNumber(msValues[constMsCtrlState]);
		case MCODE_C_MSEVENTFLAGS:    return api.PassNumber(msValues[constMsEventFlags]);
		case MCODE_C_MSLASTCTRLSTATE: return api.PassNumber(msValues[constMsLastCtrlState]);

		case MCODE_V_FAR_WIDTH:
			return ScrX + 1;

		case MCODE_V_FAR_HEIGHT:
			return ScrY + 1;

		case MCODE_V_FAR_TITLE:
			return api.PassString(console.GetTitle());

		case MCODE_V_FAR_PID:
			return api.PassNumber(GetCurrentProcessId());

		case MCODE_V_FAR_UPTIME:
			return api.PassNumber(Global->FarUpTime() / 1ms);

		case MCODE_V_MACRO_AREA:
			return api.PassNumber(GetArea());

		case MCODE_C_FULLSCREENMODE: // Fullscreen?
			return api.PassBoolean(IsConsoleFullscreen());

		case MCODE_C_ISUSERADMIN: // IsUserAdmin?
			return api.PassBoolean(os::security::is_admin());

		case MCODE_V_DRVSHOWPOS: // Drv.ShowPos
			return Global->Macro_DskShowPosType;

		case MCODE_V_DRVSHOWMODE: // Drv.ShowMode
			return Global->Opt->ChangeDriveMode;

		case MCODE_C_CMDLINE_BOF:              // CmdLine.Bof - курсор в начале cmd-строки редактирования?
		case MCODE_C_CMDLINE_EOF:              // CmdLine.Eof - курсор в конце cmd-строки редактирования?
		case MCODE_C_CMDLINE_EMPTY:            // CmdLine.Empty
		case MCODE_C_CMDLINE_SELECTED:         // CmdLine.Selected
		{
			return api.PassBoolean(Global->CtrlObject->CmdLine() && Global->CtrlObject->CmdLine()->VMProcess(CheckCode));
		}

		case MCODE_V_CMDLINE_ITEMCOUNT:        // CmdLine.ItemCount
		case MCODE_V_CMDLINE_CURPOS:           // CmdLine.CurPos
		{
			return Global->CtrlObject->CmdLine()?Global->CtrlObject->CmdLine()->VMProcess(CheckCode):-1;
		}

		case MCODE_V_CMDLINE_VALUE:            // CmdLine.Value
		{
			string Value;
			if (Global->CtrlObject->CmdLine())
				Value = Global->CtrlObject->CmdLine()->GetString();
			return api.PassString(Value);
		}

		case MCODE_C_APANEL_ROOT:  // APanel.Root
		case MCODE_C_PPANEL_ROOT:  // PPanel.Root
		{
			const auto SelPanel = (CheckCode == MCODE_C_APANEL_ROOT)?ActivePanel:PassivePanel;
			return api.PassBoolean((SelPanel ? SelPanel->VMProcess(MCODE_C_ROOTFOLDER) ? 1:0:0));
		}

		case MCODE_C_APANEL_BOF:
		case MCODE_C_PPANEL_BOF:
		case MCODE_C_APANEL_EOF:
		case MCODE_C_PPANEL_EOF:
		{
			const auto SelPanel = (CheckCode == MCODE_C_APANEL_BOF || CheckCode == MCODE_C_APANEL_EOF)?ActivePanel:PassivePanel;
			if (SelPanel)
				ret=SelPanel->VMProcess(CheckCode==MCODE_C_APANEL_BOF || CheckCode==MCODE_C_PPANEL_BOF?MCODE_C_BOF:MCODE_C_EOF)?1:0;
			return api.PassBoolean(ret);
		}

		case MCODE_C_SELECTED:    // Selected?
		{
			int NeedType = m_Area == MACROAREA_EDITOR? windowtype_editor : (m_Area == MACROAREA_VIEWER? windowtype_viewer : (m_Area == MACROAREA_DIALOG? windowtype_dialog : windowtype_panels));

			if (!(m_Area == MACROAREA_USERMENU || m_Area == MACROAREA_MAINMENU || m_Area == MACROAREA_MENU) && CurrentWindow && CurrentWindow->GetType()==NeedType)
			{
				int CurSelected;

				if (m_Area==MACROAREA_SHELL && Global->CtrlObject->CmdLine()->IsVisible())
					CurSelected = static_cast<int>(Global->CtrlObject->CmdLine()->VMProcess(CheckCode));
				else
					CurSelected = static_cast<int>(CurrentWindow->VMProcess(CheckCode));

				return api.PassBoolean(CurSelected);
			}
			else
			{
				if (const auto f = Global->WindowManager->GetCurrentWindow())
				{
					ret=f->VMProcess(CheckCode);
				}
			}
			return api.PassBoolean(ret);
		}

		case MCODE_C_EMPTY:   // Empty
		case MCODE_C_BOF:
		case MCODE_C_EOF:
		{
			int CurArea=GetArea();

			if (!(m_Area == MACROAREA_USERMENU || m_Area == MACROAREA_MAINMENU || m_Area == MACROAREA_MENU) && CurrentWindow && CurrentWindow->GetType() == windowtype_panels && !(CurArea == MACROAREA_INFOPANEL || CurArea == MACROAREA_QVIEWPANEL || CurArea == MACROAREA_TREEPANEL))
			{
				if (CheckCode == MCODE_C_EMPTY)
					ret = Global->CtrlObject->CmdLine()->GetString().empty();
				else
					ret=Global->CtrlObject->CmdLine()->VMProcess(CheckCode);
			}
			else
			{
				if (const auto f = Global->WindowManager->GetCurrentWindow())
				{
					ret=f->VMProcess(CheckCode);
				}
			}

			return api.PassBoolean(ret);
		}

		case MCODE_V_DLGITEMCOUNT: // Dlg->ItemCount
		case MCODE_V_DLGCURPOS:    // Dlg->CurPos
		case MCODE_V_DLGITEMTYPE:  // Dlg->ItemType
		case MCODE_V_DLGPREVPOS:   // Dlg->PrevPos
		case MCODE_V_DLGINFOID:        // Dlg->Info.Id
		case MCODE_V_DLGINFOOWNER:     // Dlg->Info.Owner
		{
			if (CurrentWindow)
			{
				if (CurrentWindow->GetType()==windowtype_menu)
				{
					int idx = Global->WindowManager->IndexOf(CurrentWindow);
					if(idx>0)
						CurrentWindow = Global->WindowManager->GetWindow(idx-1);
				}
				else if (CurrentWindow->GetType()==windowtype_combobox)
					CurrentWindow = std::static_pointer_cast<VMenu>(CurrentWindow)->GetDialog();
			}
			if (!CurrentWindow || CurrentWindow->GetType()!=windowtype_dialog)
				return (CheckCode==MCODE_V_DLGINFOID || CheckCode==MCODE_V_DLGINFOOWNER) ? api.PassString(L"") : 0;

			if (CheckCode==MCODE_V_DLGINFOID || CheckCode==MCODE_V_DLGINFOOWNER)
				return api.PassString(reinterpret_cast<const wchar_t*>(static_cast<intptr_t>(CurrentWindow->VMProcess(CheckCode))));
			else
				return CurrentWindow->VMProcess(CheckCode);
		}

		case MCODE_C_APANEL_VISIBLE:  // APanel.Visible
		case MCODE_C_PPANEL_VISIBLE:  // PPanel.Visible
		{
			const auto SelPanel = CheckCode == MCODE_C_APANEL_VISIBLE?ActivePanel:PassivePanel;
			return api.PassBoolean(SelPanel && SelPanel->IsVisible());
		}

		case MCODE_C_APANEL_ISEMPTY: // APanel.Empty
		case MCODE_C_PPANEL_ISEMPTY: // PPanel.Empty
		{
			const auto SelPanel = CheckCode == MCODE_C_APANEL_ISEMPTY?ActivePanel:PassivePanel;
			if (SelPanel)
			{
				string Filename;
				SelPanel->GetFileName(Filename, SelPanel->GetCurrentPos(), FileAttr);
				size_t GetFileCount=SelPanel->GetFileCount();
				ret = !GetFileCount || (GetFileCount == 1 && IsParentDirectory(Filename));
			}
			return api.PassBoolean(ret);
		}

		case MCODE_C_APANEL_FILTER:
		case MCODE_C_PPANEL_FILTER:
		{
			const auto SelPanel = (CheckCode == MCODE_C_APANEL_FILTER)?ActivePanel:PassivePanel;
			return api.PassBoolean(SelPanel && SelPanel->VMProcess(MCODE_C_APANEL_FILTER));
		}

		case MCODE_C_APANEL_LFN:
		case MCODE_C_PPANEL_LFN:
		{
			const auto SelPanel = CheckCode == MCODE_C_APANEL_LFN ? ActivePanel : PassivePanel;
			return api.PassBoolean(SelPanel && !SelPanel->GetShowShortNamesMode());
		}

		case MCODE_C_APANEL_LEFT: // APanel.Left
		case MCODE_C_PPANEL_LEFT: // PPanel.Left
		{
			const auto SelPanel = CheckCode == MCODE_C_APANEL_LEFT ? ActivePanel : PassivePanel;
			return api.PassBoolean(SelPanel && SelPanel==Global->CtrlObject->Cp()->LeftPanel());
		}

		case MCODE_C_APANEL_FILEPANEL: // APanel.FilePanel
		case MCODE_C_PPANEL_FILEPANEL: // PPanel.FilePanel
		{
			const auto SelPanel = CheckCode == MCODE_C_APANEL_FILEPANEL ? ActivePanel : PassivePanel;
			return api.PassBoolean(SelPanel && SelPanel->GetType() == panel_type::FILE_PANEL);
		}

		case MCODE_C_APANEL_PLUGIN: // APanel.Plugin
		case MCODE_C_PPANEL_PLUGIN: // PPanel.Plugin
		{
			const auto SelPanel = CheckCode == MCODE_C_APANEL_PLUGIN?ActivePanel:PassivePanel;
			return api.PassBoolean(SelPanel && SelPanel->GetMode() == panel_mode::PLUGIN_PANEL);
		}

		case MCODE_C_APANEL_FOLDER: // APanel.Folder
		case MCODE_C_PPANEL_FOLDER: // PPanel.Folder
		{
			const auto SelPanel = CheckCode == MCODE_C_APANEL_FOLDER?ActivePanel:PassivePanel;
			if (SelPanel)
			{
				string Filename;
				SelPanel->GetFileName(Filename, SelPanel->GetCurrentPos(), FileAttr);

				if (FileAttr != INVALID_FILE_ATTRIBUTES)
					ret=(FileAttr&FILE_ATTRIBUTE_DIRECTORY)?1:0;
			}
			return api.PassBoolean(ret);
		}

		case MCODE_C_APANEL_SELECTED: // APanel.Selected
		case MCODE_C_PPANEL_SELECTED: // PPanel.Selected
		{
			const auto SelPanel = CheckCode == MCODE_C_APANEL_SELECTED?ActivePanel:PassivePanel;
			return api.PassBoolean(SelPanel && SelPanel->GetRealSelCount() > 0);
		}

		case MCODE_V_APANEL_CURRENT: // APanel.Current
		case MCODE_V_PPANEL_CURRENT: // PPanel.Current
		{
			const auto SelPanel = CheckCode == MCODE_V_APANEL_CURRENT ? ActivePanel : PassivePanel;
			string Filename;
			if (SelPanel)
			{
				SelPanel->GetFileName(Filename, SelPanel->GetCurrentPos(), FileAttr);
			}
			return api.PassString(Filename);
		}

		case MCODE_V_APANEL_SELCOUNT: // APanel.SelCount
		case MCODE_V_PPANEL_SELCOUNT: // PPanel.SelCount
		{
			const auto SelPanel = CheckCode == MCODE_V_APANEL_SELCOUNT ? ActivePanel : PassivePanel;
			return SelPanel ? SelPanel->GetRealSelCount() : 0;
		}

		case MCODE_V_APANEL_COLUMNCOUNT:       // APanel.ColumnCount - активная панель:  количество колонок
		case MCODE_V_PPANEL_COLUMNCOUNT:       // PPanel.ColumnCount - пассивная панель: количество колонок
		{
			const auto SelPanel = CheckCode == MCODE_V_APANEL_COLUMNCOUNT ? ActivePanel : PassivePanel;
			return SelPanel ? SelPanel->GetColumnsCount() : 0;
		}

		case MCODE_V_APANEL_WIDTH: // APanel.Width
		case MCODE_V_PPANEL_WIDTH: // PPanel.Width
		case MCODE_V_APANEL_HEIGHT: // APanel.Height
		case MCODE_V_PPANEL_HEIGHT: // PPanel.Height
		{
			if (const auto SelPanel = CheckCode == MCODE_V_APANEL_WIDTH || CheckCode == MCODE_V_APANEL_HEIGHT? ActivePanel : PassivePanel)
			{
				const auto PanelRect = SelPanel->GetPosition();
				ret = (CheckCode == MCODE_V_APANEL_HEIGHT || CheckCode == MCODE_V_PPANEL_HEIGHT)? PanelRect.height() : PanelRect.width();
			}
			return ret;
		}

		case MCODE_V_APANEL_OPIFLAGS:  // APanel.OPIFlags
		case MCODE_V_PPANEL_OPIFLAGS:  // PPanel.OPIFlags
		case MCODE_V_APANEL_HOSTFILE: // APanel.HostFile
		case MCODE_V_PPANEL_HOSTFILE: // PPanel.HostFile
		case MCODE_V_APANEL_FORMAT:           // APanel.Format
		case MCODE_V_PPANEL_FORMAT:           // PPanel.Format
		{
			const auto SelPanel =
					CheckCode == MCODE_V_APANEL_OPIFLAGS ||
					CheckCode == MCODE_V_APANEL_HOSTFILE ||
					CheckCode == MCODE_V_APANEL_FORMAT? ActivePanel : PassivePanel;

			const wchar_t* ptr = nullptr;
			if (CheckCode == MCODE_V_APANEL_HOSTFILE || CheckCode == MCODE_V_PPANEL_HOSTFILE ||
				CheckCode == MCODE_V_APANEL_FORMAT || CheckCode == MCODE_V_PPANEL_FORMAT)
				ptr = L"";

			if (SelPanel )
			{
				if (SelPanel->GetMode() == panel_mode::PLUGIN_PANEL)
				{
					OpenPanelInfo Info={};
					Info.StructSize=sizeof(OpenPanelInfo);
					SelPanel->GetOpenPanelInfo(&Info);
					switch (CheckCode)
					{
						case MCODE_V_APANEL_OPIFLAGS:
						case MCODE_V_PPANEL_OPIFLAGS:
							return Info.Flags;
						case MCODE_V_APANEL_HOSTFILE:
						case MCODE_V_PPANEL_HOSTFILE:
							return api.PassString(Info.HostFile);
						case MCODE_V_APANEL_FORMAT:
						case MCODE_V_PPANEL_FORMAT:
							return api.PassString(Info.Format);
					}
				}
			}

			return ptr ? api.PassString(ptr) : 0;
		}

		case MCODE_V_APANEL_PREFIX:           // APanel.Prefix
		case MCODE_V_PPANEL_PREFIX:           // PPanel.Prefix
		{
			const auto SelPanel = CheckCode == MCODE_V_APANEL_PREFIX ? ActivePanel : PassivePanel;
			const wchar_t *ptr = L"";
			if (SelPanel)
			{
				PluginInfo PInfo = {sizeof(PInfo)};
				if (SelPanel->VMProcess(MCODE_V_APANEL_PREFIX,&PInfo))
					ptr = PInfo.CommandPrefix;
			}
			return api.PassString(ptr);
		}

		case MCODE_V_APANEL_PATH0:           // APanel.Path0
		case MCODE_V_PPANEL_PATH0:           // PPanel.Path0
		{
			string Filename;
			if (const auto SelPanel = CheckCode == MCODE_V_APANEL_PATH0? ActivePanel : PassivePanel)
			{
				Filename = SelPanel->GetCurDir();
			}
			return api.PassString(Filename);
		}

		case MCODE_V_APANEL_PATH: // APanel.Path
		case MCODE_V_PPANEL_PATH: // PPanel.Path
		{
			string Filename;
			if (const auto SelPanel = CheckCode == MCODE_V_APANEL_PATH? ActivePanel : PassivePanel)
			{
				if (SelPanel->GetMode() == panel_mode::PLUGIN_PANEL)
				{
					OpenPanelInfo Info={};
					Info.StructSize=sizeof(OpenPanelInfo);
					SelPanel->GetOpenPanelInfo(&Info);
					Filename = NullToEmpty(Info.CurDir);
				}
				else
					Filename = SelPanel->GetCurDir();

				DeleteEndSlash(Filename); // - чтобы у корня диска было C:, тогда можно писать так: APanel.Path + "\\file"
			}
			return api.PassString(Filename);
		}

		case MCODE_V_APANEL_UNCPATH: // APanel.UNCPath
		case MCODE_V_PPANEL_UNCPATH: // PPanel.UNCPath
		{
			string Filename;
			if (MakePath(CheckCode == MCODE_V_APANEL_UNCPATH? Global->CtrlObject->Cp()->ActivePanel() : Global->CtrlObject->Cp()->PassivePanel(), false, true, false, Filename))
			{
				DeleteEndSlash(Filename);
			}
			return api.PassString(Filename);
		}

		//FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL
		case MCODE_V_APANEL_TYPE: // APanel.Type
		case MCODE_V_PPANEL_TYPE: // PPanel.Type
		{
			const auto SelPanel = CheckCode == MCODE_V_APANEL_TYPE ? ActivePanel : PassivePanel;
			return static_cast<intptr_t>(SelPanel? SelPanel->GetType() : panel_type::FILE_PANEL);
		}

		case MCODE_V_APANEL_DRIVETYPE: // APanel.DriveType - активная панель: тип привода
		case MCODE_V_PPANEL_DRIVETYPE: // PPanel.DriveType - пассивная панель: тип привода
		{
			const auto SelPanel = CheckCode == MCODE_V_APANEL_DRIVETYPE ? ActivePanel : PassivePanel;

			if (SelPanel  && SelPanel->GetMode() != panel_mode::PLUGIN_PANEL)
			{
				return os::fs::drive::get_type(GetPathRoot(SelPanel->GetCurDir()));
			}

			return -1;
		}

		case MCODE_V_APANEL_ITEMCOUNT: // APanel.ItemCount
		case MCODE_V_PPANEL_ITEMCOUNT: // PPanel.ItemCount
		{
			const auto SelPanel = CheckCode == MCODE_V_APANEL_ITEMCOUNT ? ActivePanel : PassivePanel;
			return SelPanel ? SelPanel->GetFileCount() : 0;
		}

		case MCODE_V_APANEL_CURPOS: // APanel.CurPos
		case MCODE_V_PPANEL_CURPOS: // PPanel.CurPos
		{
			const auto SelPanel = CheckCode == MCODE_V_APANEL_CURPOS ? ActivePanel : PassivePanel;
			return SelPanel ? SelPanel->GetCurrentPos()+(SelPanel->GetFileCount()>0?1:0) : 0;
		}

		case MCODE_V_TITLE: // Title
		{
			const auto f = Global->WindowManager->GetCurrentWindow();
			string Filename;

			if (std::dynamic_pointer_cast<FilePanels>(f) && ActivePanel)
			{
				Filename = ActivePanel->GetTitle();
			}
			else if (f)
			{
				string strType;

				switch (f->GetTypeAndName(strType, Filename))
				{
					case windowtype_editor:
					case windowtype_viewer:
						Filename = f->GetTitle();
						break;
				}
			}
			inplace::trim(Filename);
			return api.PassString(Filename);
		}

		case MCODE_V_HEIGHT:  // Height - высота текущего объекта
		case MCODE_V_WIDTH:   // Width - ширина текущего объекта
		{
			if (const auto Window = Global->WindowManager->GetCurrentWindow())
			{
				const auto WindowRect = Window->GetPosition();
				ret = CheckCode == MCODE_V_HEIGHT? WindowRect.height() : WindowRect.width();
			}

			return ret;
		}

		case MCODE_V_MENU_VALUE: // Menu.Value
		case MCODE_V_MENUINFOID: // Menu.Info.Id
		{
			int CurArea=GetArea();

			if (CheckCode==MCODE_V_MENUINFOID && CurrentWindow && CurrentWindow->GetType()==windowtype_menu)
			{
				return api.PassString(reinterpret_cast<const wchar_t*>(static_cast<intptr_t>(CurrentWindow->VMProcess(MCODE_V_DLGINFOID))));
			}

			if (IsMenuArea(CurArea) || CurArea == MACROAREA_DIALOG)
			{
				if (const auto f = Global->WindowManager->GetCurrentWindow())
				{
					string Value;

					switch(CheckCode)
					{
						case MCODE_V_MENU_VALUE:
							if (f->VMProcess(CheckCode, &Value))
							{
								Value = trim(HiText2Str(Value));
								return api.PassString(Value);
							}
							break;

						case MCODE_V_MENUINFOID:
							return api.PassString(reinterpret_cast<const wchar_t*>(static_cast<intptr_t>(f->VMProcess(CheckCode))));
					}
				}
			}

			return api.PassString(nullptr);
		}

		case MCODE_V_ITEMCOUNT: // ItemCount - число элементов в текущем объекте
		case MCODE_V_CURPOS: // CurPos - текущий индекс в текущем объекте
		{
			if (const auto Window = Global->WindowManager->GetCurrentWindow())
			{
				ret = Window->VMProcess(CheckCode);
			}
			return ret;
		}

		case MCODE_V_EDITORCURLINE: // Editor.CurLine - текущая линия в редакторе (в дополнении к Count)
		case MCODE_V_EDITORSTATE:   // Editor.State
		case MCODE_V_EDITORLINES:   // Editor.Lines
		case MCODE_V_EDITORCURPOS:  // Editor.CurPos
		case MCODE_V_EDITORREALPOS: // Editor.RealPos
		case MCODE_V_EDITORFILENAME: // Editor.FileName
		case MCODE_V_EDITORSELVALUE: // Editor.SelValue
		{
			if (GetArea()==MACROAREA_EDITOR && Global->WindowManager->GetCurrentEditor() && Global->WindowManager->GetCurrentEditor()->IsVisible())
			{
				switch (CheckCode)
				{
				case MCODE_V_EDITORFILENAME:
					{
						string Type, Filename;
						Global->WindowManager->GetCurrentEditor()->GetTypeAndName(Type, Filename);
						return api.PassString(Filename);
					}

				case MCODE_V_EDITORSELVALUE:
					{
						string Value;
						Global->WindowManager->GetCurrentEditor()->VMProcess(CheckCode, &Value);
						return api.PassString(Value);
					}

				default:
					return Global->WindowManager->GetCurrentEditor()->VMProcess(CheckCode);
				}
			}

			return CheckCode == MCODE_V_EDITORSELVALUE? api.PassString(L"") : 0;
		}

		case MCODE_V_HELPFILENAME:  // Help.FileName
		case MCODE_V_HELPTOPIC:     // Help.Topic
		case MCODE_V_HELPSELTOPIC:  // Help.SelTopic
		{
			string Value;
			if (GetArea() == MACROAREA_HELP)
			{
				CurrentWindow->VMProcess(CheckCode, &Value, 0);
			}
			return api.PassString(Value);
		}

		case MCODE_V_VIEWERFILENAME: // Viewer.FileName
		case MCODE_V_VIEWERSTATE: // Viewer.State
		{
			if ((GetArea()==MACROAREA_VIEWER || GetArea()==MACROAREA_QVIEWPANEL) &&
							Global->WindowManager->GetCurrentViewer() && Global->WindowManager->GetCurrentViewer()->IsVisible())
			{
				if (CheckCode == MCODE_V_VIEWERFILENAME)
					return api.PassString(Global->WindowManager->GetCurrentViewer()->GetFileName());
				else
					return api.PassNumber(Global->WindowManager->GetCurrentViewer()->VMProcess(MCODE_V_VIEWERSTATE));
			}

			return CheckCode == MCODE_V_VIEWERFILENAME? api.PassString(L"") : 0;
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
		case MCODE_F_EDITOR_POS:
		{
			SCOPED_ACTION(LockOutput)(IsTopMacroOutputDisabled());
			return api.editorposFunc();
		}
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
			bool result=api.waitkeyFunc()!=0;
			--m_WaitKey;

			return result;
		}
		case MCODE_F_PLUGIN_CALL:
			if(Data->Count>=2 && Data->Values[0].Type==FMVT_BOOLEAN && Data->Values[1].Type==FMVT_STRING)
			{
				bool SyncCall = (Data->Values[0].Boolean == 0);
				const wchar_t* SysID = Data->Values[1].String;
				const auto Uuid = uuid::try_parse(string_view(SysID));
				if (Uuid && Global->CtrlObject->Plugins->FindPlugin(*Uuid))
				{
					FarMacroValue *Values = Data->Count>2 ? Data->Values+2:nullptr;
					OpenMacroInfo info={sizeof(OpenMacroInfo),Data->Count-2,Values};
					void *ResultCallPlugin = nullptr;

					if (SyncCall) m_InternalInput++;

					if (!Global->CtrlObject->Plugins->CallPlugin(*Uuid, OPEN_FROMMACRO, &info, &ResultCallPlugin))
						ResultCallPlugin = nullptr;

					if (SyncCall) m_InternalInput--;

					//в windows гарантируется, что не бывает указателей меньше 0x10000
					if (reinterpret_cast<uintptr_t>(ResultCallPlugin) >= 0x10000 && ResultCallPlugin != INVALID_HANDLE_VALUE)
					{
						FarMacroValue Result(ResultCallPlugin);
						Data->Callback(Data->CallbackData, &Result, 1);
					}
					else
						api.PassBoolean(ResultCallPlugin != nullptr);

					return 0;
				}
			}
			api.PassBoolean(false);
			return 0;

		case MCODE_F_WINDOW_SCROLL:   return api.windowscrollFunc();
		case MCODE_F_XLAT:            return api.xlatFunc();
		case MCODE_F_PROMPT:          return api.promptFunc();

		case MCODE_F_CHECKALL:
		{
			auto Result = false;
			if (Data->Count >= 2)
			{
				const auto Area = static_cast<FARMACROAREA>(static_cast<int>(Data->Values[0].Double));
				const auto Flags = static_cast<MACROFLAGS_MFLAGS>(static_cast<int>(Data->Values[1].Double));
				const auto Callback = (Data->Count >= 3 && Data->Values[2].Type == FMVT_POINTER)? reinterpret_cast<FARMACROCALLBACK>(Data->Values[2].Pointer) : nullptr;
				const auto CallbackId = (Data->Count >= 4 && Data->Values[3].Type == FMVT_POINTER)? Data->Values[3].Pointer : nullptr;
				Result = CheckAll(Area, Flags) && (!Callback || Callback(CallbackId, AKMFLAGS_NONE));
			}
			api.PassBoolean(Result);
			return 0;
		}

		case MCODE_F_GETOPTIONS:
		{
			DWORD Options = Global->OnlyEditorViewerUsed; // bits 0x1 and 0x2
			if (Global->Opt->Macro.DisableMacro&MDOL_ALL)       Options |= 0x4;
			if (Global->Opt->Macro.DisableMacro&MDOL_AUTOSTART) Options |= 0x8;
			if (Global->Opt->ReadOnlyConfig)                    Options |= 0x10;
			api.PassNumber(Options);
			break;
		}

		case MCODE_F_USERMENU:
			ShowUserMenu(Data->Count,Data->Values);
			break;

		case MCODE_F_SETCUSTOMSORTMODE:
			if (Data->Count>=3 && Data->Values[0].Type==FMVT_DOUBLE  &&
				Data->Values[1].Type==FMVT_DOUBLE && Data->Values[2].Type==FMVT_BOOLEAN)
			{
				auto panel = Global->CtrlObject->Cp()->ActivePanel();
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
			break;

		case MCODE_F_KEYMACRO:
			if (Data->Count && Data->Values[0].Type==FMVT_DOUBLE)
			{
				switch (static_cast<int>(Data->Values[0].Double))
				{
					case 1: RestoreMacroChar(); break;
					case 2: Global->ScrBuf->Lock(); break;
					case 3: Global->ScrBuf->Unlock(); break;
					case 4: Global->ScrBuf->ResetLockCount(); break;
					case 5: api.PassNumber(Global->ScrBuf->GetLockCount()); break;
					case 6: if (Data->Count > 1) Global->ScrBuf->SetLockCount(Data->Values[1].Double); break;
					case 7: api.PassBoolean(default_clipboard_mode::get() == clipboard_mode::internal); break;
					case 8: if (Data->Count > 1) default_clipboard_mode::set(Data->Values[1].Boolean != 0? clipboard_mode::internal: clipboard_mode::system); break;
					case 9: if (Data->Count > 1) api.PassNumber(KeyNameToKey(Data->Values[1].String)); break;
					case 10:
						if (Data->Count > 1)
						{
							api.PassString(KeyToText(Data->Values[1].Double));
						}
						break;
				}
			}
			break;

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
					api.PassNumber(static_cast<double>(Flags));
					api.PassString(m_RecCode);
					api.PassString(m_RecDescription);
					return 0;
				}
			}
			api.PassBoolean(false);
			break;

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
			auto Params = parseParams(2, Data);
			auto& p1(Params[0]);
			auto& p2(Params[1]);

			long long Result=0;

			if (const auto f = Global->WindowManager->GetCurrentWindow())
			{
				Result = f->VMProcess(CheckCode, ToPtr(p2.toInteger()), p1.toInteger());
			}

			return Result;
		}

		case MCODE_F_MENU_ITEMSTATUS:     // N=Menu.ItemStatus([N])
		case MCODE_F_MENU_GETVALUE:       // S=Menu.GetValue([N])
		case MCODE_F_MENU_GETHOTKEY:      // S=gethotkey([N])
		{
			auto Params = parseParams(1, Data);
			TVar tmpVar=Params[0];

			tmpVar.toInteger();

			int CurArea=GetArea();

			if (IsMenuArea(CurArea) || CurArea == MACROAREA_DIALOG)
			{
				if (const auto f = Global->WindowManager->GetCurrentWindow())
				{
					long long MenuItemPos=tmpVar.asInteger()-1;
					if (CheckCode == MCODE_F_MENU_GETHOTKEY)
					{
						long long Result = f->VMProcess(CheckCode,nullptr,MenuItemPos);
						if (Result)
						{

							const wchar_t _value[]={static_cast<wchar_t>(Result),0};
							tmpVar=_value;
						}
						else
							tmpVar = L""sv;
					}
					else if (CheckCode == MCODE_F_MENU_GETVALUE)
					{
						string NewStr;
						if (f->VMProcess(CheckCode,&NewStr,MenuItemPos))
						{
							NewStr = trim(HiText2Str(NewStr));
							tmpVar=NewStr;
						}
						else
							tmpVar = L""sv;
					}
					else if (CheckCode == MCODE_F_MENU_ITEMSTATUS)
					{
						tmpVar=f->VMProcess(CheckCode,nullptr,MenuItemPos);
					}
				}
				else
					tmpVar = L""sv;
			}
			else
				tmpVar = L""sv;

			api.PassValue(tmpVar);
			return 0;
		}

		case MCODE_F_MENU_SELECT:      // N=Menu.Select(S[,N[,Dir]])
		case MCODE_F_MENU_CHECKHOTKEY: // N=checkhotkey(S[,N])
		{
			auto Params = parseParams(3, Data);
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
				if (const auto f = Global->WindowManager->GetCurrentWindow())
				{
					Result = f->VMProcess(CheckCode, UNSAFE_CSTR(Params[0].toString()), tmpMode);
				}
			}

			api.PassNumber(Result);
			return 0;
		}

		case MCODE_F_MENU_FILTER:      // N=Menu.Filter([Action[,Mode]])
		case MCODE_F_MENU_FILTERSTR:   // S=Menu.FilterStr([Action[,S]])
		{
			auto Params = parseParams(2, Data);
			bool success=false;
			auto& tmpAction(Params[0]);

			TVar tmpVar=Params[1];
			if (tmpAction.isUnknown())
				tmpAction=CheckCode == MCODE_F_MENU_FILTER ? 4 : 0;

			int CurArea=GetArea();

			if (IsMenuArea(CurArea) || CurArea == MACROAREA_DIALOG)
			{
				if (const auto f = Global->WindowManager->GetCurrentWindow())
				{
					if (CheckCode == MCODE_F_MENU_FILTER)
					{
						if (tmpVar.isUnknown())
							tmpVar = -1;
						tmpVar = f->VMProcess(CheckCode, reinterpret_cast<void*>(static_cast<intptr_t>(tmpVar.toInteger())), tmpAction.toInteger());
						success=true;
					}
					else
					{
						string NewStr;
						if (tmpVar.isString())
							NewStr = tmpVar.toString();
						if (f->VMProcess(MCODE_F_MENU_FILTERSTR, &NewStr, tmpAction.toInteger()))
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

			api.PassValue(tmpVar);
			return success;
		}
	}

	return 0;
}

/* ------------------------------------------------------------------- */
// S=trim(S[,N])
int FarMacroApi::trimFunc()
{
	auto Params = parseParams(2, mData);
	const auto mode = static_cast<int>(Params[1].asInteger());
	auto p = Params[0].toString();
	int Ret=1;

	switch (mode)
	{
	case 0: inplace::trim(p); break;
	case 1: inplace::trim_left(p); break;
	case 2: inplace::trim_right(p); break;
	default: Ret = 0;
	}

	PassString(p);
	return Ret;
}

// S=substr(S,start[,length])
int FarMacroApi::substrFunc()
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
	auto Params = parseParams(3, mData);
	int Ret=0;

	int  start = static_cast<int>(Params[1].asInteger());
	auto& p = Params[0].toString();
	const auto length_str = static_cast<int>(p.size());
	int length = Params[2].isUnknown()? length_str : static_cast<int>(Params[2].asInteger());

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
		PassString(L"");
	}
	else
	{
		Ret=1;
		PassString(p.substr(start, length));
	}

	return Ret;
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
int FarMacroApi::fsplitFunc()
{
	auto Params = parseParams(2, mData);

	string strPath;
	SplitPath(Params[0].toString(), strPath, Params[1].asInteger());

	PassString(strPath);
	return 1;
}

// N=atoi(S[,radix])
int FarMacroApi::atoiFunc()
{
	auto Params = parseParams(2, mData);
	long long Value = 0;
	const auto Ret = from_string(Params[0].toString(), Value, nullptr, static_cast<int>(Params[1].toInteger()));
	PassInteger(Value);
	return Ret;
}

// N=Window.Scroll(Lines[,Axis])
int FarMacroApi::windowscrollFunc()
{
	auto Params = parseParams(2, mData);
	int Ret=0;

	if (Global->Opt->WindowMode)
	{
		int Lines = static_cast<int>(Params[0].asInteger()), Columns = 0;
		if (Params[1].asInteger())
		{
			Columns=Lines;
			Lines=0;
		}

		if (console.ScrollWindow(Lines, Columns))
		{
			Ret=1;
		}
	}

	PassBoolean(Ret);
	return Ret;
}

// S=itoa(N[,radix])
int FarMacroApi::itowFunc()
{
	auto Params = parseParams(2, mData);
	int Ret=0;

	if (Params[0].isInteger() || Params[0].isDouble())
	{
		wchar_t value[65];
		int Radix = static_cast<int>(Params[1].toInteger());

		if (!Radix)
			Radix=10;

		Ret=1;
		Params[0]=TVar(_i64tow(Params[0].toInteger(),value,Radix));
	}

	PassValue(Params[0]);
	return Ret;
}

// os::chrono::sleep_for(Nms)
int FarMacroApi::sleepFunc()
{
	const auto Params = parseParams(1, mData);
	const auto Period = Params[0].asInteger();

	if (Period > 0)
	{
		os::chrono::sleep_for(std::chrono::milliseconds(Period));
		PassNumber(1);
		return 1;
	}

	PassNumber(0);
	return 0;
}


// N=KeyBar.Show([N])
int FarMacroApi::keybarshowFunc()
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
	auto Params = parseParams(1, mData);
	const auto f = Global->WindowManager->GetCurrentWindow();

	PassNumber(f?f->VMProcess(MCODE_F_KEYBAR_SHOW,nullptr,Params[0].asInteger())-1:-1);
	return f != nullptr;
}


// S=key(V)
int FarMacroApi::keyFunc()
{
	auto Params = parseParams(1, mData);
	string strKeyText;

	if (Params[0].isInteger() || Params[0].isDouble())
	{
		if (Params[0].asInteger())
			strKeyText = KeyToText(static_cast<int>(Params[0].asInteger()));
	}
	else
	{
		// Проверим...
		if (const auto Key = KeyNameToKey(Params[0].asString()))
			strKeyText=Params[0].asString();
	}

	PassString(strKeyText);
	return !strKeyText.empty();
}

// V=waitkey([N,[T]])
int FarMacroApi::waitkeyFunc()
{
	auto Params = parseParams(2, mData);
	const auto Type = static_cast<long>(Params[1].asInteger());
	const auto Period = static_cast<long>(Params[0].asInteger());
	auto Key = WaitKey(static_cast<DWORD>(-1), Period);

	if (!Type)
	{
		string strKeyText;

		if (Key != KEY_NONE)
			strKeyText = KeyToText(Key);

		PassString(strKeyText);
		return !strKeyText.empty();
	}

	if (Key == KEY_NONE)
		Key=-1;

	PassNumber(Key);
	return Key != static_cast<DWORD>(-1);
}

// n=min(n1,n2)
int FarMacroApi::minFunc()
{
	auto Params = parseParams(2, mData);
	PassValue(std::min(Params[0], Params[1]));
	return 1;
}

// n=max(n1,n2)
int FarMacroApi::maxFunc()
{
	auto Params = parseParams(2, mData);
	PassValue(std::max(Params[0], Params[1]));
	return 1;
}

// n=mod(n1,n2)
int FarMacroApi::modFunc()
{
	auto Params = parseParams(2, mData);

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

	PassValue(Result);
	return !Result.isUnknown();
}

// N=index(S1,S2[,Mode])
int FarMacroApi::indexFunc()
{
	auto Params = parseParams(3, mData);
	const auto& s = Params[0].toString();
	const auto& p = Params[1].toString();

	const auto StrStr = [](const string& Str1, const string& Str2) { return std::search(ALL_CONST_RANGE(Str1), ALL_CONST_RANGE(Str2)); };
	const auto StrStrI = [](const string& Str1, const string& Str2) { return std::search(ALL_CONST_RANGE(Str1), ALL_CONST_RANGE(Str2), equal_icase_t{}); };

	const auto i = Params[2].asInteger()? StrStr(s, p) : StrStrI(s, p);
	const auto Position = i != s.cend() ? i - s.cbegin() : -1;
	PassNumber(Position);
	return Position != -1;
}

// S=rindex(S1,S2[,Mode])
int FarMacroApi::rindexFunc()
{
	auto Params = parseParams(3, mData);
	const auto& s = Params[0].toString();
	const auto& p = Params[1].toString();

	const auto RevStrStr = [](const string& Str1, const string& Str2) { return std::find_end(ALL_CONST_RANGE(Str1), ALL_CONST_RANGE(Str2)); };
	const auto RevStrStrI = [](const string& Str1, const string& Str2) { return std::find_end(ALL_CONST_RANGE(Str1), ALL_CONST_RANGE(Str2), equal_icase_t{}); };

	const auto i = Params[2].asInteger()? RevStrStr(s, p) : RevStrStrI(s, p);
	const auto Position = i != s.cend()? i - s.cbegin() : -1;
	PassNumber(Position);
	return Position != -1;
}

// S=Size2Str(Size,Flags[,Width])
int FarMacroApi::size2strFunc()
{
	const auto Params = parseParams(3, mData);
	const auto Width = static_cast<int>(Params[2].asInteger());
	PassString(FileSizeToStr(Params[0].asInteger(), Width, Params[1].asInteger()));
	return 1;
}

// S=date([S])
int FarMacroApi::dateFunc()
{
	auto Params = parseParams(1, mData);

	if (Params[0].isInteger() && !Params[0].asInteger())
		Params[0] = L""sv;

	const auto strTStr = MkStrFTime(Params[0].toString());
	const auto Ret = !strTStr.empty();
	PassString(strTStr);
	return Ret;
}

// S=xlat(S[,Flags])
/*
  Flags:
  	XLAT_SWITCHKEYBLAYOUT  = 1
		XLAT_SWITCHKEYBBEEP    = 2
		XLAT_USEKEYBLAYOUTNAME = 4
*/
int FarMacroApi::xlatFunc()
{
	auto Params = parseParams(2, mData);
	auto StrParam = Params[0].toString();
	Xlat(StrParam, Params[1].asInteger());
	PassString(StrParam);
	return 1;
}

// N=beep([N])
int FarMacroApi::beepFunc()
{
	auto Params = parseParams(1, mData);
	const auto Ret = MessageBeep(static_cast<unsigned>(Params[0].asInteger())) != FALSE;

	PassBoolean(Ret);
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
int FarMacroApi::kbdLayoutFunc()
{
	auto Params = parseParams(1, mData);
	const auto dwLayout = static_cast<DWORD>(Params[0].asInteger());

	int Ret = 1;
	HKL RetLayout = nullptr;

	string LayoutName;
	if (console.GetKeyboardLayoutName(LayoutName))
	{
		const auto Layout = from_string<unsigned long>(LayoutName, nullptr, 16);
		RetLayout = reinterpret_cast<HKL>(static_cast<intptr_t>(HIWORD(Layout)? Layout : MAKELONG(Layout, Layout)));
	}

	const auto hWnd = console.GetWindow();

	if (hWnd && dwLayout)
	{
		HKL Layout;
		WPARAM wParam;

		if (static_cast<long>(dwLayout) == -1)
		{
			wParam=INPUTLANGCHANGE_BACKWARD;
WARNING_PUSH()
WARNING_DISABLE_GCC("-Wzero-as-null-pointer-constant")
			Layout = static_cast<HKL>(HKL_PREV);
WARNING_POP()
		}
		else if (dwLayout == 1)
		{
			wParam=INPUTLANGCHANGE_FORWARD;
			Layout = reinterpret_cast<HKL>(HKL_NEXT);
		}
		else
		{
			wParam=0;
			Layout = reinterpret_cast<HKL>(static_cast<intptr_t>(HIWORD(dwLayout) ? dwLayout : MAKELONG(dwLayout, dwLayout)));
		}

		Ret = PostMessage(hWnd, WM_INPUTLANGCHANGEREQUEST, wParam, reinterpret_cast<LPARAM>(Layout)) != FALSE;
	}

	PassNumber(Ret? reinterpret_cast<intptr_t>(RetLayout) : 0);

	return Ret;
}

// S=prompt(["Title"[,"Prompt"[,flags[, "Src"[, "History"]]]]])
int FarMacroApi::promptFunc()
{
	auto Params = parseParams(5, mData);
	auto& ValHistory(Params[4]);
	auto& ValSrc(Params[3]);
	const auto Flags = static_cast<DWORD>(Params[2].asInteger());
	auto& ValPrompt(Params[1]);
	auto& ValTitle(Params[0]);
	int Ret=0;

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

	if (history.empty()) // Mantis#0001743: Возможность отключения истории
		SetHistoryDisableMask(8); // если не указан history, то принудительно отключаем историю для ЭТОГО prompt()

	if (GetString(title, prompt, history, src, strDest, {}, (Flags&~FIB_CHECKBOX) | FIB_ENABLEEMPTY))
	{
		PassString(strDest);
		Ret=1;
	}
	else
		PassBoolean(0);

	SetHistoryDisableMask(oldHistoryDisable);

	return Ret;
}

// N=msgbox(["Title"[,"Text"[,flags]]])
int FarMacroApi::msgBoxFunc()
{
	auto Params = parseParams(3, mData);

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

	if (!HIWORD(Flags) || HIWORD(Flags) > HIWORD(FMSG_MB_RETRYCANCEL))
		Flags|=FMSG_MB_OK;

	//_KEYMACRO(SysLog(L"title='%s'",title));
	//_KEYMACRO(SysLog(L"text='%s'",text));
	const auto TempBuf = concat(title, L'\n', text);
	const auto Result = pluginapi::apiMessageFn(&FarUuid, &FarUuid, Flags, nullptr, reinterpret_cast<const wchar_t* const*>(TempBuf.c_str()), 0, 0) + 1;
	PassNumber(Result);
	return 1;
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
int FarMacroApi::menushowFunc()
{
	auto Params = parseParams(6, mData);
	auto& VY(Params[5]);
	auto& VX(Params[4]);
	auto& VFindOrFilter(Params[3]);
	const auto Flags = static_cast<DWORD>(Params[2].asInteger());
	auto& Title(Params[1]);

	if (Title.isUnknown())
		Title = L""sv;

	auto strTitle = Title.toString();
	string strBottom;
	auto& Items(Params[0]);
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
		for (int numlines = std::count(ALL_CONST_RANGE(strItems), L'\n'); numlines; numlines/=10)
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
		MenuItemEx NewItem;
		size_t SubstrLen=PosLF-CurrentPos;

		if (SubstrLen==0)
			SubstrLen=1;

		NewItem.Name = strItems.substr(CurrentPos, SubstrLen);

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

			const auto NewBegin = std::find_if(ALL_CONST_RANGE(NewItem.Name), [&](wchar_t i)
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
			NewItem.Name = format(FSTR(L"{0:{1}} - {2}"), LineCount, nLeftShift - 3, NewItem.Name);
		}
		Menu->AddItem(NewItem);
		CurrentPos=PosLF+1;
		PosLF = strItems.find(L'\n', CurrentPos);
		CRFound = PosLF != string::npos;
	}

	if (bSorting)
	{
		Menu->SortItems([](const MenuItemEx& a, const MenuItemEx& b, const SortItemParam& Param)
		{
			if (a.Flags & LIF_SEPARATOR || b.Flags & LIF_SEPARATOR)
				return false;

			string strName1(a.Name);
			string strName2(b.Name);
			RemoveHighlights(strName1);
			RemoveHighlights(strName2);
			const auto Less = string_sort::less(string_view(strName1).substr(Param.Offset), string_view(strName2).substr(Param.Offset));
			return Param.Reverse? !Less : Less;
		});
	}

	if (bPacking)
		Menu->Pack();

	if ((bAutoNumbering) && (bSorting || bPacking))
	{
		for (int i = 0; i < Menu->GetShowItemCount(); i++)
		{
			auto& Item = Menu->at(i);
			if (!(Item.Flags & LIF_SEPARATOR))
			{
				LineCount++;
				Item.Name = format(FSTR(L"{0:{1}} - {2}"), LineCount, nLeftShift - 3, Item.Name);
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
			string NewStr=VFindOrFilter.toString();
			Menu->VMProcess(MCODE_F_MENU_FILTERSTR, &NewStr, 1);
			bSetMenuFilter = false;
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
					for (size_t i = 0, size = Menu->size(); i != size; ++i)
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

	wchar_t temp[65];

	if (Menu->GetExitCode() >= 0)
	{
		SelectedPos=Menu->GetExitCode();
		if (bMultiSelect)
		{
			string StrResult;

			for (size_t i = 0, size = Menu->size(); i != size; ++i)
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
	PassValue(Result);
	return 1;
}

// S=Env(S[,Mode[,Value]])
int FarMacroApi::environFunc()
{
	auto Params = parseParams(3, mData);
	auto& Value(Params[2]);
	auto& Mode(Params[1]);
	auto& S(Params[0]);
	int Ret=0;
	string strEnv;


	if (os::env::get(S.toString(), strEnv))
		Ret=true;
	else
		strEnv.clear();

	if (Mode.asInteger()) // Mode != 0: Set
	{
		if (Value.isUnknown() || Value.asString().empty())
			os::env::del(S.toString());
		else
			os::env::set(S.toString(), Value.toString());
	}

	PassString(strEnv);
	return Ret;
}

// V=Panel.Select(panelType,Action[,Mode[,Items]])
int FarMacroApi::panelselectFunc()
{
	auto Params = parseParams(4, mData);
	auto& ValItems(Params[3]);
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

	PassNumber(Result);
	return Result != -1;
}

int FarMacroApi::fattrFuncImpl(int Type)
{
	int Ret=0;
	os::fs::attributes FileAttr = INVALID_FILE_ATTRIBUTES;
	long Pos=-1;

	if (!Type || Type == 2) // не панели: fattr(0) & fexist(2)
	{
		auto Params = parseParams(1, mData);
		auto& Str(Params[0]);
		os::fs::find_data FindData;
		// BUGBUG check result
		(void)os::fs::get_find_data(Str.toString(), FindData);
		FileAttr=FindData.Attributes;
		Ret=1;
	}
	else // panel.fattr(1) & panel.fexist(3)
	{
		auto Params = parseParams(2, mData);
		auto& S(Params[1]);
		const auto& Str = S.toString();

		if (const auto SelPanel = SelectPanel(Params[0].asInteger()))
		{
			if (Str.find_first_of(L"*?"sv) != string::npos)
				Pos=SelPanel->FindFirst(Str);
			else
				Pos = SelPanel->FindFile(Str, Str.find_first_of(L"\\/:"sv) != string::npos);

			if (Pos >= 0)
			{
				string strFileName;
				SelPanel->GetFileName(strFileName,Pos,FileAttr);
				Ret=1;
			}
		}
	}

	if (Type == 2) // fexist(2)
	{
		PassBoolean(FileAttr!=INVALID_FILE_ATTRIBUTES);
		return 1;
	}

	if (Type == 3) // panel.fexist(3)
		FileAttr=static_cast<DWORD>(Pos)+1;

	PassNumber(static_cast<long>(FileAttr));
	return Ret;
}

// N=fattr(S)
int FarMacroApi::fattrFunc()
{
	return fattrFuncImpl(0);
}

// N=fexist(S)
int FarMacroApi::fexistFunc()
{
	return fattrFuncImpl(2);
}

// N=panel.fattr(S)
int FarMacroApi::panelfattrFunc()
{
	return fattrFuncImpl(1);
}

// N=panel.fexist(S)
int FarMacroApi::panelfexistFunc()
{
	return fattrFuncImpl(3);
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
int FarMacroApi::flockFunc()
{
	auto Params = parseParams(2, mData);
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

	PassNumber(Ret);
	return Ret != -1;
}

// N=Dlg->SetFocus([ID])
int FarMacroApi::dlgsetfocusFunc()
{
	auto Params = parseParams(1, mData);
	TVar Ret(-1);
	const auto Index = static_cast<unsigned>(Params[0].asInteger()) - 1;

	if (Global->CtrlObject->Macro.GetArea() == MACROAREA_DIALOG)
	{
		if (const auto Dlg = std::dynamic_pointer_cast<Dialog>(Global->WindowManager->GetCurrentWindow()))
		{
			Ret = Dlg->VMProcess(MCODE_V_DLGCURPOS);
			if (static_cast<int>(Index) >= 0)
			{
				if (!Dlg->SendMessage(DM_SETFOCUS, Index, nullptr))
					Ret = 0;
			}
		}
	}
	PassValue(Ret);
	return Ret.asInteger() != -1; // ?? <= 0 ??
}

// V=Far.Cfg.Get(Key,Name)
int FarMacroApi::farcfggetFunc()
{
	auto Params = parseParams(2, mData);
	auto& Name(Params[1]);
	auto& Key(Params[0]);

	const auto option = Global->Opt->GetConfigValue(Key.asString(), Name.asString());
	option ? PassString(option->toString()) : PassBoolean(0);
	return option != nullptr;
}

// V=Far.GetConfig(Key,Name)
int FarMacroApi::fargetconfigFunc()
{
	if (mData->Count >= 2 && mData->Values[0].Type==FMVT_STRING && mData->Values[1].Type==FMVT_STRING)
	{
		if (const auto option = Global->Opt->GetConfigValue(mData->Values[0].String, mData->Values[1].String))
		{
			if (const auto Opt = dynamic_cast<const BoolOption*>(option))
			{
				PassNumber(1);
				PassBoolean(Opt->Get());
				return 1;
			}

			if (const auto Opt = dynamic_cast<const Bool3Option*>(option))
			{
				PassNumber(2);
				PassNumber(static_cast<double>(Opt->Get()));
				return 1;
			}

			if (const auto Opt = dynamic_cast<const IntOption*>(option))
			{
				double d;
				PassNumber(3);
				if (ToDouble(Opt->Get(), d))
					PassNumber(d);
				else
					PassInteger(Opt->Get());
				return 1;
			}

			if (const auto Opt = dynamic_cast<const StringOption*>(option))
			{
				PassNumber(4);
				PassString(Opt->Get());
				return 1;
			}
		}
	}
	PassBoolean(0);
	return 0;
}

// V=Dlg->GetValue([Pos[,InfoID]])
int FarMacroApi::dlggetvalueFunc()
{
	auto Params = parseParams(2, mData);
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

		FarGetValue fgv={sizeof(FarGetValue),InfoID,FMVT_UNKNOWN};
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
					default: Ret=0; Ret.SetType(TVar::Type::Unknown); break;
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
					FarListGetItem ListItem={sizeof(FarListGetItem)};
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
				case 1: Ret=ItemType;    break;
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
		else if (Index >= DlgItem.size())
		{
			Ret = static_cast<long long>(InfoID);
		}
		else
			CallDialog=false;

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
						Ret=-1;
						break;
				}
			}
		}
		}
	}

	PassValue(Ret);
	return Ret.asInteger() != -1;
}

// N=Editor.Pos(Op,What[,Where])
// Op: 0 - get, 1 - set
int FarMacroApi::editorposFunc()
{
	auto Params = parseParams(3, mData);
	TVar Ret(-1);
	int Where = static_cast<int>(Params[2].asInteger());
	int What = static_cast<int>(Params[1].asInteger());
	int Op = static_cast<int>(Params[0].asInteger());

	if (Global->CtrlObject->Macro.GetArea()==MACROAREA_EDITOR && Global->WindowManager->GetCurrentEditor() && Global->WindowManager->GetCurrentEditor()->IsVisible())
	{
		EditorInfo ei={sizeof(EditorInfo)};
		Global->WindowManager->GetCurrentEditor()->EditorControl(ECTL_GETINFO,0,&ei);

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
				EditorSetPosition esp={sizeof(EditorSetPosition)};
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

				int Result=Global->WindowManager->GetCurrentEditor()->EditorControl(ECTL_SETPOSITION,0,&esp);

				if (Result)
					Global->WindowManager->GetCurrentEditor()->EditorControl(ECTL_REDRAW,0,nullptr);

				Ret=Result;
				break;
			}
		}
	}

	PassValue(Ret);
	return Ret.asInteger() != -1;
}

// OldVar=Editor.Set(Idx,Value)
int FarMacroApi::editorsetFunc()
{
	auto Params = parseParams(2, mData);
	TVar Ret(-1);
	auto& Value(Params[1]);
	int Index = static_cast<int>(Params[0].asInteger());

	if (Global->CtrlObject->Macro.GetArea()==MACROAREA_EDITOR && Global->WindowManager->GetCurrentEditor() && Global->WindowManager->GetCurrentEditor()->IsVisible())
	{
		long long longState = -1L;

		if (mData->Count > 1)
		{
			if (Index != 12)
				longState = Value.toInteger();
			else
			{
				if (Value.isString() || Value.asInteger() != -1)
					longState = 0;
			}
		}

		Options::EditorOptions EdOpt;
		Global->WindowManager->GetCurrentEditor()->GetEditorOptions(EdOpt);

		switch (Index)
		{
		case 0:  // TabSize;
			Ret = EdOpt.TabSize;
			break;
		case 1:  // ExpandTabs;
			Ret = EdOpt.ExpandTabs;
			break;
		case 2:  // PersistentBlocks;
			Ret = EdOpt.PersistentBlocks;
			break;
		case 3:  // DelRemovesBlocks;
			Ret = EdOpt.DelRemovesBlocks;
			break;
		case 4:  // AutoIndent;
			Ret = EdOpt.AutoIndent;
			break;
		case 5:  // AutoDetectCodePage;
			Ret = EdOpt.AutoDetectCodePage;
			break;
		case 7:  // CursorBeyondEOL;
			Ret = EdOpt.CursorBeyondEOL;
			break;
		case 8:  // BSLikeDel;
			Ret = EdOpt.BSLikeDel;
			break;
		case 9:  // CharCodeBase;
			Ret = EdOpt.CharCodeBase;
			break;
		case 10: // SavePos;
			Ret = EdOpt.SavePos;
			break;
		case 11: // SaveShortPos;
			Ret = EdOpt.SaveShortPos;
			break;
		case 12: // char WordDiv[256];
			Ret = TVar(EdOpt.strWordDiv);
			break;
		case 14: // AllowEmptySpaceAfterEof;
			Ret = EdOpt.AllowEmptySpaceAfterEof;
			break;
		case 15: // ShowScrollBar;
			Ret = EdOpt.ShowScrollBar;
			break;
		case 16: // EditOpenedForWrite;
			Ret = EdOpt.EditOpenedForWrite;
			break;
		case 17: // SearchSelFound;
			Ret = EdOpt.SearchSelFound;
			break;
		case 18: // SearchRegexp;
			Ret = EdOpt.SearchRegexp;
			break;
		case 20: // ShowWhiteSpace;
			Ret = EdOpt.ShowWhiteSpace;
			break;
		default:
			Ret = -1;
			break;
		}

		if (longState != -1)
		{
			switch (Index)
			{
				case 0:  // TabSize;
					if (!EdOpt.TabSize.TrySet(longState))
						Ret = -1;
					break;
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
				case 7:  // CursorBeyondEOL;
					EdOpt.CursorBeyondEOL=longState != 0; break;
				case 8:  // BSLikeDel;
					EdOpt.BSLikeDel=(longState != 0); break;
				case 9:  // CharCodeBase;
					EdOpt.CharCodeBase=longState; break;
				case 10: // SavePos;
					EdOpt.SavePos=(longState != 0); break;
				case 11: // SaveShortPos;
					EdOpt.SaveShortPos=(longState != 0); break;
				case 12: // char WordDiv[256];
					EdOpt.strWordDiv = Value.toString(); break;
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
				case 20: // ShowWhiteSpace;
					EdOpt.ShowWhiteSpace=longState; break;
				default:
					Ret=-1;
					break;
			}

			Global->WindowManager->GetCurrentEditor()->SetEditorOptions(EdOpt);
			Global->WindowManager->GetCurrentEditor()->ShowStatus();
			if (Index == 0 || Index == 12 || Index == 14 || Index == 15 || Index == 20)
				Global->WindowManager->GetCurrentEditor()->Show();
		}
	}

	PassValue(Ret);
	return Ret.asInteger() == -1;
}

// V=Clip(N[,V])
int FarMacroApi::clipFunc()
{
	auto Params = parseParams(2, mData);
	auto& Val(Params[1]);
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
				return 0;

			PassString(ClipText);
			return 1;
		}

	case 1: // Put "S" into Clipboard
		{
			const auto& Str = Val.asString();
			if (!Str.empty())
				return SetClipboardText(Str);

			clipboard_accessor Clip;
			return Clip->Open() && Clip->Clear();
		}

	case 2: // Add "S" into Clipboard
		{
			clipboard_accessor Clip;
			if (!Clip->Open())
				return 0;

			string CopyData;
			if (!Clip->GetText(CopyData))
				return 0;

			return Clip->SetText(CopyData + Val.asString());
		}

	case 3: // Copy Win to internal, "S" - ignore
	case 4: // Copy internal to Win, "S" - ignore
		{
			clipboard_accessor ClipSystem(clipboard_mode::system);
			clipboard_accessor ClipInternal(clipboard_mode::internal);

			return ClipSystem->Open() && ClipInternal->Open() && (cmdType == 3? CopyData(ClipSystem, ClipInternal) : CopyData(ClipInternal, ClipSystem));
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

			PassNumber(PreviousMode == clipboard_mode::internal? 2 : 1);
			return 1;
		}
	}

	return 0;
}


// N=Panel.SetPosIdx(panelType,Idx[,InSelection])
/*
*/
int FarMacroApi::panelsetposidxFunc()
{
	const auto Params = parseParams(3, mData);
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
					for (size_t I=0 ; I < EndPos ; I++ )
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

	PassNumber(Ret);
	return Ret != 0;
}

// N=Panel.SetPos(panelType,fileName)
int FarMacroApi::panelsetposFunc()
{
	auto Params = parseParams(2, mData);
	auto& Val(Params[1]);
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
				SelPanel->UpdateIfChanged(false);
				Global->WindowManager->RefreshWindow(Global->WindowManager->GetCurrentWindow());
				// </Mantis#0000289>
				Ret = static_cast<long long>(SelPanel->GetCurrentPos()) + 1;
			}
		}
	}

	PassNumber(Ret);
	return Ret != 0;
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

int FarMacroApi::replaceFunc()
{
	const auto Params = parseParams(5, mData);
	auto Src = Params[0].asString();
	const auto& Find = Params[1].asString();
	const auto& Repl = Params[2].asString();
	const auto Count = Params[3].asInteger();
	const auto IgnoreCase = !Params[4].asInteger();

	ReplaceStrings(Src, Find, Repl, IgnoreCase, Count <= 0 ? string::npos : static_cast<size_t>(Count));
	PassString(Src);
	return 1;
}

// V=Panel.Item(typePanel,Index,TypeInfo)
int FarMacroApi::panelitemFunc()
{
	auto Params = parseParams(3, mData);
	auto& P2(Params[2]);
	auto& P1(Params[1]);

	//const auto CurrentWindow=WindowManager->GetCurrentWindow();

	const auto SelPanel = SelectPanel(Params[0].asInteger());
	if (!SelPanel)
		return PassError(L"No panel selected");

	const auto PanelType = SelPanel->GetType(); //FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL

	if (!(PanelType == panel_type::FILE_PANEL || PanelType == panel_type::TREE_PANEL))
		return PassError(L"Unsupported panel type");

	const auto Index = static_cast<int>(P1.toInteger()) - 1;
	const auto TypeInfo = static_cast<int>(P2.toInteger());

	if (const auto Tree = std::dynamic_pointer_cast<TreeList>(SelPanel))
	{
		const auto treeItem = Tree->GetItem(Index);
		if (treeItem && !TypeInfo)
			return PassString(treeItem->strName);
	}
	else if (const auto fileList = std::dynamic_pointer_cast<FileList>(SelPanel))
	{
		string strDate, strTime;

		if (TypeInfo == 11)
			fileList->ReadDiz();

		const auto filelistItem = fileList->GetItem(Index);

		if (!filelistItem)
			return PassError(L"No list item found at specified index");

		switch (TypeInfo)
		{
			case 0:  // Name
				return PassString(filelistItem->FileName);
			case 1:  // ShortName
				return PassString(filelistItem->AlternateFileName());
			case 2:  // FileAttr
				return PassNumber(static_cast<long>(filelistItem->Attributes));
			case 3:  // CreationTime
				ConvertDate(filelistItem->CreationTime, strDate, strTime, 8, 1);
				return PassString(concat(strDate, L' ', strTime));
			case 4:  // AccessTime
				ConvertDate(filelistItem->LastAccessTime, strDate, strTime, 8, 1);
				return PassString(concat(strDate, L' ', strTime));
			case 5:  // WriteTime
				ConvertDate(filelistItem->LastWriteTime, strDate, strTime, 8, 1);
				return PassString(concat(strDate, L' ', strTime));
			case 6:  // FileSize
				return PassInteger(filelistItem->FileSize);
			case 7:  // AllocationSize
				return PassInteger(filelistItem->AllocationSize);
			case 8:  // Selected
				return PassBoolean(filelistItem->Selected);
			case 9:  // NumberOfLinks
				return PassNumber(filelistItem->NumberOfLinks(fileList.get()));
			case 10:  // SortGroup
				return PassNumber(filelistItem->SortGroup);
			case 11:  // DizText
				return PassString(filelistItem->DizText);
			case 12:  // Owner
				return PassString(filelistItem->Owner(fileList.get()));
			case 13:  // CRC32
				return PassNumber(filelistItem->CRC32);
			case 14:  // Position
				return PassNumber(filelistItem->Position);
			case 15:  // CreationTime
				return PassInteger(os::chrono::nt_clock::to_hectonanoseconds(filelistItem->CreationTime));
			case 16:  // AccessTime
				return PassInteger(os::chrono::nt_clock::to_hectonanoseconds(filelistItem->LastAccessTime));
			case 17:  // WriteTime
				return PassInteger(os::chrono::nt_clock::to_hectonanoseconds(filelistItem->LastWriteTime));
			case 18: // NumberOfStreams
				return PassNumber(filelistItem->NumberOfStreams(fileList.get()));
			case 19: // StreamsSize
				return PassInteger(filelistItem->StreamsSize(fileList.get()));
			case 20:  // ChangeTime
				ConvertDate(filelistItem->ChangeTime, strDate, strTime, 8, 1);
				return PassString(concat(strDate, L' ', strTime));
			case 21:  // ChangeTime
				return PassInteger(os::chrono::nt_clock::to_hectonanoseconds(filelistItem->ChangeTime));
			case 22:  // ContentData (was: CustomData)
				//Ret=TVar(filelistItem->ContentData.size() ? filelistItem->ContentData[0] : L"");
				break;
			case 23:  // ReparseTag
				return PassNumber(filelistItem->ReparseTag);
		}
	}

	return PassError(L"Operation not supported");
}

// N=len(V)
int FarMacroApi::lenFunc()
{
	auto Params = parseParams(1, mData);
	PassNumber(Params[0].toString().size());
	return 1;
}

int FarMacroApi::ucaseFunc()
{
	auto Params = parseParams(1, mData);
	auto& Val(Params[0]);
	Val = upper(Val.toString());
	PassValue(Val);
	return 1;
}

int FarMacroApi::lcaseFunc()
{
	auto Params = parseParams(1, mData);
	auto& Val(Params[0]);
	Val = lower(Val.toString());
	PassValue(Val);
	return 1;
}

int FarMacroApi::stringFunc()
{
	auto Params = parseParams(1, mData);
	auto& Val(Params[0]);
	Val.toString();
	PassValue(Val);
	return 1;
}

// S=StrPad(Src,Cnt[,Fill[,Op]])
int FarMacroApi::strpadFunc()
{
	auto Params = parseParams(4, mData);
	auto& Src(Params[0]);
	if (Src.isUnknown())
	{
		Src = L""sv;
		Src.toString();
	}
	const auto Cnt = static_cast<int>(Params[1].asInteger());
	auto& Fill(Params[2]);
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

			for (int i = 0; i != FineLength; ++i)
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

	PassString(strDest);
	return 1;
}

// S=StrWrap(Text,Width[,Break[,Flags]])
int FarMacroApi::strwrapFunc()
{
	auto Params = parseParams(3, mData);
	auto& Break = Params[2];
	const size_t Width = Params[1].asInteger();
	const auto& Text = Params[0];

	if (Break.isUnknown())
		Break = L"\n"sv;

	PassString(join(wrapped_text(Text.asString(), Width), Break.asString()));

	return 1;
}

int FarMacroApi::intFunc()
{
	auto Params = parseParams(1, mData);
	auto& Val(Params[0]);
	Val.toInteger();
	PassInteger(Val.asInteger());
	return 1;
}

int FarMacroApi::floatFunc()
{
	auto Params = parseParams(1, mData);
	auto& Val(Params[0]);
	Val.toDouble();
	PassValue(Val);
	return 1;
}

int FarMacroApi::absFunc()
{
	auto Params = parseParams(1, mData);

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

	PassValue(Result);
	return !Result.isUnknown();
}

int FarMacroApi::ascFunc()
{
	auto Params = parseParams(1, mData);
	auto& tmpVar(Params[0]);

	if (tmpVar.isString() && !tmpVar.asString().empty())
		PassNumber(static_cast<DWORD>(tmpVar.asString().front()));
	else
		PassValue(tmpVar);

	return 1;
}

int FarMacroApi::chrFunc()
{
	auto Params = parseParams(1, mData);
	TVar tmpVar(Params[0]);

	if (tmpVar.isNumber())
	{
		const auto Char = static_cast<wchar_t>(tmpVar.asInteger());
		tmpVar = string_view{ &Char, 1 };
	}

	PassValue(tmpVar);
	return 1;
}

// N=FMatch(S,Mask)
int FarMacroApi::fmatchFunc()
{
	auto Params = parseParams(2, mData);
	auto& Mask(Params[1]);
	auto& S(Params[0]);
	filemasks FileMask;

	if (FileMask.assign(Mask.toString(), FMF_SILENT))
		PassNumber(FileMask.check(S.toString()));
	else
		PassNumber(-1);
	return 1;
}

// V=Editor.Sel(Action[,Opt])
int FarMacroApi::editorselFunc()
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
	auto Params = parseParams(2, mData);
	TVar Ret(0ll);
	auto& Opts(Params[1]);
	auto& Action(Params[0]);

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

	PassValue(Ret);
	return Ret.asInteger() == 1;
}

// V=Editor.Undo(Action)
int FarMacroApi::editorundoFunc()
{
	auto Params = parseParams(1, mData);
	TVar Ret(0ll);
	auto& Action(Params[0]);

	if (Global->CtrlObject->Macro.GetArea()==MACROAREA_EDITOR && Global->WindowManager->GetCurrentEditor() && Global->WindowManager->GetCurrentEditor()->IsVisible())
	{
		EditorUndoRedo eur={sizeof(EditorUndoRedo)};
		eur.Command=static_cast<EDITOR_UNDOREDO_COMMANDS>(Action.toInteger());
		Ret = Global->WindowManager->GetCurrentEditor()->EditorControl(ECTL_UNDOREDO,0,&eur);
	}

	PassValue(Ret);
	return Ret.asInteger()!=0;
}

// N=Editor.SetTitle([Title])
int FarMacroApi::editorsettitleFunc()
{
	auto Params = parseParams(1, mData);
	TVar Ret(0ll);
	auto& Title(Params[0]);

	if (Global->CtrlObject->Macro.GetArea()==MACROAREA_EDITOR && Global->WindowManager->GetCurrentEditor() && Global->WindowManager->GetCurrentEditor()->IsVisible())
	{
		if (Title.isInteger() && !Title.asInteger())
		{
			Title = L""sv;
			Title.toString();
		}
		Ret = Global->WindowManager->GetCurrentEditor()->EditorControl(ECTL_SETTITLE, 0, UNSAFE_CSTR(Title.asString()));
	}

	PassValue(Ret);
	return Ret.asInteger()!=0;
}

// N=Editor.DelLine([Line])
int FarMacroApi::editordellineFunc()
{
	auto Params = parseParams(1, mData);
	TVar Ret(0ll);
	auto& Line(Params[0]);

	if (Global->CtrlObject->Macro.GetArea()==MACROAREA_EDITOR && Global->WindowManager->GetCurrentEditor() && Global->WindowManager->GetCurrentEditor()->IsVisible())
	{
		if (Line.isNumber())
		{
			Ret = Global->WindowManager->GetCurrentEditor()->VMProcess(MCODE_F_EDITOR_DELLINE, nullptr, Line.asInteger()-1);
		}
	}

	PassValue(Ret);
	return Ret.asInteger()!=0;
}

// N=Editor.InsStr([S[,Line]])
int FarMacroApi::editorinsstrFunc()
{
	auto Params = parseParams(2, mData);
	TVar Ret(0ll);
	auto& S(Params[0]);
	auto& Line(Params[1]);

	if (Global->CtrlObject->Macro.GetArea()==MACROAREA_EDITOR && Global->WindowManager->GetCurrentEditor() && Global->WindowManager->GetCurrentEditor()->IsVisible())
	{
		if (Line.isNumber())
		{
			if (S.isUnknown())
			{
				S = L""sv;
				S.toString();
			}

			Ret = Global->WindowManager->GetCurrentEditor()->VMProcess(MCODE_F_EDITOR_INSSTR, UNSAFE_CSTR(S.asString()), Line.asInteger()-1);
		}
	}

	PassValue(Ret);
	return Ret.asInteger()!=0;
}

// N=Editor.SetStr([S[,Line]])
int FarMacroApi::editorsetstrFunc()
{
	auto Params = parseParams(2, mData);
	TVar Ret(0ll);
	auto& S(Params[0]);
	auto& Line(Params[1]);

	if (Global->CtrlObject->Macro.GetArea()==MACROAREA_EDITOR && Global->WindowManager->GetCurrentEditor() && Global->WindowManager->GetCurrentEditor()->IsVisible())
	{
		if (Line.isNumber())
		{
			if (S.isUnknown())
			{
				S = L""sv;
				S.toString();
			}

			Ret = Global->WindowManager->GetCurrentEditor()->VMProcess(MCODE_F_EDITOR_SETSTR, UNSAFE_CSTR(S.asString()), Line.asInteger()-1);
		}
	}

	PassValue(Ret);
	return Ret.asInteger()!=0;
}

// N=Plugin.Exist(Uuid)
int FarMacroApi::pluginexistFunc()
{
	bool Ret = false;
	if (mData->Count>0 && mData->Values[0].Type==FMVT_STRING)
	{
		if (const auto Uuid = uuid::try_parse(string_view(mData->Values[0].String)); Global->CtrlObject->Plugins->FindPlugin(*Uuid))
			Ret = true;
	}
	PassBoolean(Ret);
	return Ret;
}

// N=Plugin.Load(DllPath[,ForceLoad])
int FarMacroApi::pluginloadFunc()
{
	auto Params = parseParams(2, mData);
	auto& ForceLoad(Params[1]);
	const auto& DllPath = Params[0].asString();
	const TVar Ret(pluginapi::apiPluginsControl(nullptr, !ForceLoad.asInteger()?PCTL_LOADPLUGIN:PCTL_FORCEDLOADPLUGIN, 0, UNSAFE_CSTR(DllPath)));
	PassValue(Ret);
	return Ret.asInteger()!=0;
}

// N=Plugin.UnLoad(DllPath)
int FarMacroApi::pluginunloadFunc()
{
	int Ret=0;
	if (mData->Count>0 && mData->Values[0].Type==FMVT_STRING)
	{
		if (const auto p = Global->CtrlObject->Plugins->FindPlugin(mData->Values[0].String))
		{
			Ret = static_cast<int>(pluginapi::apiPluginsControl(p, PCTL_UNLOADPLUGIN, 0, nullptr));
		}
	}
	PassNumber(Ret);
	return Ret!=0;
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
int FarMacroApi::testfolderFunc()
{
	auto Params = parseParams(1, mData);
	auto& tmpVar(Params[0]);
	long long Ret=TSTFLD_ERROR;

	if (tmpVar.isString())
	{
		SCOPED_ACTION(elevation::suppress);
		Ret = static_cast<long long>(TestFolder(tmpVar.asString()));
	}

	PassNumber(Ret);
	return Ret != 0;
}

// обработчик диалогового окна назначения клавиши
intptr_t KeyMacro::AssignMacroDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	static unsigned LastKey = 0;
	static DlgParam *KMParam=nullptr;
	const INPUT_RECORD* record=nullptr;
	unsigned key = 0;

	if (Msg == DN_CONTROLINPUT)
	{
		record = static_cast<const INPUT_RECORD*>(Param2);
		if (record->EventType==KEY_EVENT)
		{
			key = InputRecordToKey(static_cast<const INPUT_RECORD*>(Param2));
			if (key&KEY_RCTRL) key = (key&~KEY_RCTRL)|KEY_CTRL;
			if (key&KEY_RALT) key = (key&~KEY_RALT)|KEY_ALT;
		}
	}

	//_SVS(SysLog(L"LastKey=%d Msg=%s",LastKey,_DLGMSG_ToName(Msg)));
	if (Msg == DN_INITDIALOG)
	{
		KMParam = static_cast<DlgParam*>(Param2);
		LastKey=0;
		// <Клавиши, которые нельзя ввести в диалоге назначения>
		static const DWORD PreDefKeyMain[]=
		{
			//KEY_CTRLDOWN,KEY_RCTRLDOWN,KEY_ENTER,KEY_NUMENTER,KEY_ESC,KEY_F1,KEY_CTRLF5,KEY_RCTRLF5,
			KEY_CTRLDOWN,KEY_ENTER,KEY_NUMENTER,KEY_ESC,KEY_F1,KEY_CTRLF5,
		};

		for (const auto& i: PreDefKeyMain)
		{
			Dlg->SendMessage(DM_LISTADDSTR, 2, UNSAFE_CSTR(KeyToText(i)));
		}

		static const DWORD PreDefKey[]=
		{
			KEY_MSWHEEL_UP,KEY_MSWHEEL_DOWN,KEY_MSWHEEL_LEFT,KEY_MSWHEEL_RIGHT,
			KEY_MSLCLICK,KEY_MSRCLICK,KEY_MSM1CLICK,KEY_MSM2CLICK,KEY_MSM3CLICK,
#if 0
			KEY_MSLDBLCLICK,KEY_MSRDBLCLICK,KEY_MSM1DBLCLICK,KEY_MSM2DBLCLICK,KEY_MSM3DBLCLICK,
#endif
		};
		static const DWORD PreDefModKey[]=
		{
			//0,KEY_CTRL,KEY_RCTRL,KEY_SHIFT,KEY_ALT,KEY_RALT,KEY_CTRLSHIFT,KEY_RCTRLSHIFT,KEY_CTRLALT,KEY_RCTRLRALT,KEY_CTRLRALT,KEY_RCTRLALT,KEY_ALTSHIFT,KEY_RALTSHIFT,
			0,KEY_CTRL,KEY_SHIFT,KEY_ALT,KEY_CTRLSHIFT,KEY_CTRLALT,KEY_ALTSHIFT,
		};

		for (const auto& i: PreDefKey)
		{
			Dlg->SendMessage(DM_LISTADDSTR, 2, const_cast<wchar_t*>(L"\1"));

			for (const auto& j: PreDefModKey)
			{
				Dlg->SendMessage(DM_LISTADDSTR, 2, UNSAFE_CSTR(KeyToText(i | j)));
			}
		}

		Dlg->SendMessage(DM_SETTEXTPTR,2,nullptr);
		// </Клавиши, которые нельзя ввести в диалоге назначения>
	}
	else if (Param1 == 2 && Msg == DN_EDITCHANGE)
	{
		LastKey=0;
		_SVS(SysLog(L"[%d] ((FarDialogItem*)Param2)->PtrData='%s'",__LINE__,((FarDialogItem*)Param2)->Data));
		key = KeyNameToKey(static_cast<FarDialogItem*>(Param2)->Data);

		if (key && !KMParam->Recurse)
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
		if (
			key == KEY_ESC ||
			(any_of(key, KEY_ENTER, KEY_NUMENTER) && any_of(LastKey, KEY_ENTER, KEY_NUMENTER)) ||
			any_of(key, KEY_CTRLDOWN, KEY_RCTRLDOWN, KEY_F1)
		)
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

		if (any_of(key, KEY_ENTER, KEY_NUMENTER) && LastKey && none_of(LastKey, KEY_ENTER, KEY_NUMENTER))
			return FALSE;

		// </Обработка особых клавиш: F1 & Enter>
M1:
		_SVS(SysLog(L"[%d] Assign ==> Param2='%s',LastKey='%s'",__LINE__,_FARKEY_ToName((DWORD)key),LastKey?_FARKEY_ToName(LastKey):L""));

		if ((key&0x00FFFFFF) > 0x7F && (key&0x00FFFFFF) < 0xFFFF)
			key=KeyToKeyLayout(key&0x0000FFFF)|(key&~0x0000FFFF);

		if (key<0xFFFF)
		{
			key=upper(static_cast<wchar_t>(key));
		}

		_SVS(SysLog(L"[%d] Assign ==> Param2='%s',LastKey='%s'",__LINE__,_FARKEY_ToName((DWORD)key),LastKey?_FARKEY_ToName(LastKey):L""));
		KMParam->Key = static_cast<DWORD>(key);
		auto strKeyText = KeyToText(key);

		// если УЖЕ есть такой макрос...
		GetMacroData Data;
		if (LM_GetMacro(&Data,KMParam->Area,strKeyText,true) && Data.IsKeyboardMacro)
		{
			// общие макросы учитываем только при удалении.
			if (m_RecCode.empty() || Data.Area!=MACROAREA_COMMON)
			{
				auto strBufKey = quote_unconditional(string(Data.Code));
				const auto SetChange = m_RecCode.empty();
				lng MessageTemplate;
				if (Data.Area==MACROAREA_COMMON)
				{
					MessageTemplate = SetChange? lng::MMacroCommonDeleteKey : lng::MMacroCommonReDefinedKey;
					//"Общая макроклавиша '{0}'   будет удалена : уже определена."
				}
				else
				{
					MessageTemplate = SetChange? lng::MMacroDeleteKey : lng::MMacroReDefinedKey;
					//"Макроклавиша '{0}'   будет удалена : уже определена."
				}
				const auto strBuf = format(msg(MessageTemplate), strKeyText);

				const std::array ChangeButtons{ lng::MYes, lng::MMacroEditKey, lng::MNo };
				const std::array NoChangeButtons{ lng::MYes, lng::MNo };

				const int Result = Message(MSG_WARNING,
					msg(lng::MWarning),
					{
						strBuf,
						msg(lng::MMacroSequence),
						strBufKey,
						msg(SetChange? lng::MMacroDeleteKey2 : lng::MMacroReDefinedKey2)
					},
					SetChange? span(ChangeButtons) : span(NoChangeButtons));

				if (Result == Message::first_button)
				{
					// в любом случае - вываливаемся
					Dlg->SendMessage(DM_CLOSE, 1, nullptr);
					return TRUE;
				}

				if (SetChange && Result == Message::second_button)
				{
					string strDescription;

					if ( *Data.Code )
						strBufKey=Data.Code;

					if ( *Data.Description )
						strDescription=Data.Description;

					if (GetMacroSettings(key, Data.Flags, strBufKey, strDescription))
					{
						KMParam->Flags = Data.Flags;
						KMParam->Changed = true;
						// в любом случае - вываливаемся
						Dlg->SendMessage(DM_CLOSE, 1, nullptr);
						return TRUE;
					}
				}

				// здесь - здесь мы нажимали "Нет", ну а на нет и суда нет
				//  и значит очистим поле ввода.
				strKeyText.clear();
			}
		}

		KMParam->Recurse++;
		Dlg->SendMessage(DM_SETTEXTPTR,2, UNSAFE_CSTR(strKeyText));
		KMParam->Recurse--;
		//if(key == KEY_F1 && LastKey == KEY_F1)
		//LastKey=-1;
		//else
		LastKey = key;
		return TRUE;
	}
	return Dlg->DefProc(Msg,Param1,Param2);
}

int KeyMacro::AssignMacroKey(DWORD &MacroKey, unsigned long long& Flags)
{
	/*
	          1         2         3
	   3456789012345678901234567890
	 1 ╔══════ Define macro ══════╗
	 2 ║  Press the desired key   ║
	 3 ║ ________________________↓║
	 4 ╚══════════════════════════╝
	*/

	enum
	{
		mad_doublebox,
		mad_text,
		mad_combobox,

		mad_count
	};

	auto MacroAssignDlg = MakeDialogItems<mad_count>(
	{
		{DI_DOUBLEBOX, {{3,  1}, {30, 4}}, DIF_NONE, msg(lng::MDefineMacroTitle), },
		{DI_TEXT,      {{-1, 2}, {0,  2}}, DIF_NONE, msg(lng::MDefineMacro), },
		{DI_COMBOBOX,  {{5,  3}, {28, 3}}, DIF_FOCUS | DIF_DEFAULTBUTTON, },
	});

	DlgParam Param={Flags, 0, m_StartMode, 0, false};
	//_SVS(SysLog(L"StartMode=%d",m_StartMode));
	Global->IsProcessAssignMacroKey++;
	const auto Dlg = Dialog::create(MacroAssignDlg, &KeyMacro::AssignMacroDlgProc, this, &Param);
	Dlg->SetPosition({ -1, -1, 34, 6 });
	Dlg->SetHelp(L"KeyMacro"sv);
	Dlg->Process();
	Global->IsProcessAssignMacroKey--;

	if (Dlg->GetExitCode() < 0)
		return 0;

	MacroKey = Param.Key;
	Flags = Param.Flags;
	return Param.Changed ? 2 : 1;
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
		for (int Flags = 1; Flags != 0b1111; ++Flags)
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
#endif
