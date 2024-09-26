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
#include "clipboard.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "fileedit.hpp"
#include "filepanels.hpp"
#include "filetype.hpp"
#include "global.hpp"
#include "hilight.hpp"
#include "keyboard.hpp"
#include "lang.hpp"
#include "macroopcode.hpp"
#include "macrovalues.hpp"
#include "message.hpp"
#include "plugins.hpp"
#include "scrbuf.hpp"
#include "shortcuts.hpp"
#include "strmix.hpp"
#include "treelist.hpp"
#include "usermenu.hpp"

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

// The below opcodes are hardcoded in plugin LuaMacro

// функции
static_assert(MCODE_F_NOFUNC == 0x80C00);
static_assert(MCODE_F_ABS == 0x80C01);                  // N=abs(N)
static_assert(MCODE_F_AKEY == 0x80C02);                 // V=akey(Mode[,Type])
static_assert(MCODE_F_ASC == 0x80C03);                  // N=asc(S)
static_assert(MCODE_F_ATOI == 0x80C04);                 // N=atoi(S[,radix])
static_assert(MCODE_F_CLIP == 0x80C05);                 // V=clip(N[,V])
static_assert(MCODE_F_CHR == 0x80C06);                  // S=chr(N)
static_assert(MCODE_F_DATE == 0x80C07);                 // S=date([S])
static_assert(MCODE_F_DLG_GETVALUE == 0x80C08);         // V=Dlg->GetValue([Pos[,InfoID]])
static_assert(MCODE_F_EDITOR_SEL == 0x80C09);           // V=Editor.Sel(Action[,Opt])
static_assert(MCODE_F_EDITOR_SET == 0x80C0A);           // N=Editor.Set(N[,Var])
static_assert(MCODE_F_EDITOR_UNDO == 0x80C0B);          // V=Editor.Undo(N)
static_assert(MCODE_F_EDITOR_POS == 0x80C0C);           // N=Editor.Pos(Op,What[,Where])
static_assert(MCODE_F_ENVIRON == 0x80C0D);              // S=Env(S[,Mode[,Value]])
static_assert(MCODE_F_FATTR == 0x80C0E);                // N=fattr(S)
static_assert(MCODE_F_FEXIST == 0x80C0F);               // S=fexist(S)
static_assert(MCODE_F_FSPLIT == 0x80C10);               // S=fsplit(S,N)
static_assert(MCODE_F_IIF == 0x80C11);                  // V=iif(C,V1,V2)
static_assert(MCODE_F_INDEX == 0x80C12);                // S=index(S1,S2[,Mode])
static_assert(MCODE_F_INT == 0x80C13);                  // N=int(V)
static_assert(MCODE_F_ITOA == 0x80C14);                 // S=itoa(N[,radix])
static_assert(MCODE_F_KEY == 0x80C15);                  // S=key(V)
static_assert(MCODE_F_LCASE == 0x80C16);                // S=lcase(S1)
static_assert(MCODE_F_LEN == 0x80C17);                  // N=len(S)
static_assert(MCODE_F_MAX == 0x80C18);                  // N=max(N1,N2)
static_assert(MCODE_F_MENU_CHECKHOTKEY == 0x80C19);     // N=checkhotkey(S[,N])
static_assert(MCODE_F_MENU_GETHOTKEY == 0x80C1A);       // S=gethotkey([N])
static_assert(MCODE_F_MENU_SELECT == 0x80C1B);          // N=Menu.Select(S[,N[,Dir]])
static_assert(MCODE_F_MENU_SHOW == 0x80C1C);            // S=Menu.Show(Items[,Title[,Flags[,FindOrFilter[,X[,Y]]]]])
static_assert(MCODE_F_MIN == 0x80C1D);                  // N=min(N1,N2)
static_assert(MCODE_F_MOD == 0x80C1E);                  // N=mod(a,b) == a %  b
static_assert(MCODE_F_MLOAD == 0x80C1F);                // B=mload(var)
static_assert(MCODE_F_MSAVE == 0x80C20);                // B=msave(var)
static_assert(MCODE_F_MSGBOX == 0x80C21);               // N=msgbox(["Title"[,"Text"[,flags]]])
static_assert(MCODE_F_PANEL_FATTR == 0x80C22);          // N=Panel.FAttr(panelType,fileMask)
static_assert(MCODE_F_PANEL_FEXIST == 0x80C24);         // N=Panel.FExist(panelType,fileMask)
static_assert(MCODE_F_PANEL_SETPOS == 0x80C25);         // N=Panel.SetPos(panelType,fileName)
static_assert(MCODE_F_PANEL_SETPOSIDX == 0x80C26);      // N=Panel.SetPosIdx(panelType,Idx[,InSelection])
static_assert(MCODE_F_PANEL_SELECT == 0x80C27);         // V=Panel.Select(panelType,Action[,Mode[,Items]])
static_assert(MCODE_F_PANELITEM == 0x80C28);            // V=PanelItem(Panel,Index,TypeInfo)
static_assert(MCODE_F_EVAL == 0x80C29);                 // N=eval(S[,N])
static_assert(MCODE_F_RINDEX == 0x80C2A);               // S=rindex(S1,S2[,Mode])
static_assert(MCODE_F_SLEEP == 0x80C2B);                // os::chrono::sleep_for(Nms)
static_assert(MCODE_F_STRING == 0x80C2C);               // S=string(V)
static_assert(MCODE_F_SUBSTR == 0x80C2D);               // S=substr(S,start[,length])
static_assert(MCODE_F_UCASE == 0x80C2E);                // S=ucase(S1)
static_assert(MCODE_F_WAITKEY == 0x80C2F);              // V=waitkey([N,[T]])
static_assert(MCODE_F_XLAT == 0x80C30);                 // S=xlat(S)
static_assert(MCODE_F_FLOCK == 0x80C31);                // N=FLock(N,N)
static_assert(MCODE_F_CALLPLUGIN == 0x80C32);           // V=callplugin(SysID[,param])
static_assert(MCODE_F_REPLACE == 0x80C33);              // S=replace(sS,sF,sR[,Count[,Mode]])
static_assert(MCODE_F_PROMPT == 0x80C34);               // S=prompt(["Title"[,"Prompt"[,flags[, "Src"[, "History"]]]]])
static_assert(MCODE_F_BM_ADD == 0x80C35);               // N=BM.Add()  - добавить текущие координаты и обрезать хвост
static_assert(MCODE_F_BM_CLEAR == 0x80C36);             // N=BM.Clear() - очистить все закладки
static_assert(MCODE_F_BM_DEL == 0x80C37);               // N=BM.Del([Idx]) - удаляет закладку с указанным индексом (x=1...), 0 - удаляет текущую закладку
static_assert(MCODE_F_BM_GET == 0x80C38);               // N=BM.Get(Idx,M) - возвращает координаты строки (M==0) или колонки (M==1) закладки с индексом (Idx=1...)
static_assert(MCODE_F_BM_GOTO == 0x80C39);              // N=BM.Goto([n]) - переход на закладку с указанным индексом (0 --> текущую)
static_assert(MCODE_F_BM_NEXT == 0x80C3A);              // N=BM.Next() - перейти на следующую закладку
static_assert(MCODE_F_BM_POP == 0x80C3B);               // N=BM.Pop() - восстановить текущую позицию из закладки в конце стека и удалить закладку
static_assert(MCODE_F_BM_PREV == 0x80C3C);              // N=BM.Prev() - перейти на предыдущую закладку
static_assert(MCODE_F_BM_BACK == 0x80C3D);              // N=BM.Back() - перейти на предыдущую закладку с возможным сохранением текущей позиции
static_assert(MCODE_F_BM_PUSH == 0x80C3E);              // N=BM.Push() - сохранить текущую позицию в виде закладки в конце стека
static_assert(MCODE_F_BM_STAT == 0x80C3F);              // N=BM.Stat([M]) - возвращает информацию о закладках, N=0 - текущее количество закладок
static_assert(MCODE_F_TRIM == 0x80C40);                 // S=trim(S[,N])
static_assert(MCODE_F_FLOAT == 0x80C41);                // N=float(V)
static_assert(MCODE_F_TESTFOLDER == 0x80C42);           // N=testfolder(S)
static_assert(MCODE_F_PRINT == 0x80C43);                // N=Print(Str)
static_assert(MCODE_F_MMODE == 0x80C44);                // N=MMode(Action[,Value])
static_assert(MCODE_F_EDITOR_SETTITLE == 0x80C45);      // N=Editor.SetTitle([Title])
static_assert(MCODE_F_MENU_GETVALUE == 0x80C46);        // S=Menu.GetValue([N])
static_assert(MCODE_F_MENU_ITEMSTATUS == 0x80C47);      // N=Menu.ItemStatus([N])
static_assert(MCODE_F_BEEP == 0x80C48);                 // N=beep([N])
static_assert(MCODE_F_KBDLAYOUT == 0x80C49);            // N=kbdLayout([N])
static_assert(MCODE_F_WINDOW_SCROLL == 0x80C4A);        // N=Window.Scroll(Lines[,Axis])
static_assert(MCODE_F_KEYBAR_SHOW == 0x80C4B);          // N=KeyBar.Show([N])
static_assert(MCODE_F_HISTORY_DISABLE == 0x80C4C);      // N=History.Disable([State])
static_assert(MCODE_F_FMATCH == 0x80C4D);               // N=FMatch(S,Mask)
static_assert(MCODE_F_PLUGIN_MENU == 0x80C4E);          // N=Plugin.Menu(Uuid[,MenuUuid])
static_assert(MCODE_F_PLUGIN_CALL == 0x80C4F);          // N=Plugin.Config(Uuid[,MenuUuid])
static_assert(MCODE_F_PLUGIN_SYNCCALL == 0x80C50);      // N=Plugin.Call(Uuid[,Item])
static_assert(MCODE_F_PLUGIN_LOAD == 0x80C51);          // N=Plugin.Load(DllPath[,ForceLoad])
static_assert(MCODE_F_PLUGIN_COMMAND == 0x80C52);       // N=Plugin.Command(Uuid[,Command])
static_assert(MCODE_F_PLUGIN_UNLOAD == 0x80C53);        // N=Plugin.UnLoad(DllPath)
static_assert(MCODE_F_PLUGIN_EXIST == 0x80C54);         // N=Plugin.Exist(Uuid)
static_assert(MCODE_F_MENU_FILTER == 0x80C55);          // N=Menu.Filter(Action[,Mode])
static_assert(MCODE_F_MENU_FILTERSTR == 0x80C56);       // S=Menu.FilterStr([Action[,S]])
static_assert(MCODE_F_DLG_SETFOCUS == 0x80C57);         // N=Dlg->SetFocus([ID])
static_assert(MCODE_F_FAR_CFG_GET == 0x80C58);          // V=Far.Cfg.Get(Key,Name)
static_assert(MCODE_F_SIZE2STR == 0x80C59);             // S=Size2Str(N,Flags[,Width])
static_assert(MCODE_F_STRWRAP == 0x80C5A);              // S=StrWrap(Text,Width[,Break[,Flags]])
static_assert(MCODE_F_MACRO_KEYWORD == 0x80C5B);        // S=Macro.Keyword(Index[,Type])
static_assert(MCODE_F_MACRO_FUNC == 0x80C5C);           // S=Macro.Func(Index[,Type])
static_assert(MCODE_F_MACRO_VAR == 0x80C5D);            // S=Macro.Var(Index[,Type])
static_assert(MCODE_F_MACRO_CONST == 0x80C5E);          // S=Macro.Const(Index[,Type])
static_assert(MCODE_F_STRPAD == 0x80C5F);               // S=StrPad(V,Cnt[,Fill[,Op]])
static_assert(MCODE_F_EDITOR_DELLINE == 0x80C60);       // N=Editor.DelLine([Line])
static_assert(MCODE_F_EDITOR_GETSTR == 0x80C61);        // S=Editor.GetStr([Line])
static_assert(MCODE_F_EDITOR_INSSTR == 0x80C62);        // N=Editor.InsStr([S[,Line]])
static_assert(MCODE_F_EDITOR_SETSTR == 0x80C63);        // N=Editor.SetStr([S[,Line]])
static_assert(MCODE_F_CHECKALL == 0x80C64);             // Проверить предварительные условия исполнения макроса
static_assert(MCODE_F_GETOPTIONS == 0x80C65);           // Получить значения некоторых опций Фара
static_assert(MCODE_F_USERMENU == 0x80C66);             // Вывести меню пользователя
static_assert(MCODE_F_SETCUSTOMSORTMODE == 0x80C67);    // Установить пользовательский режим сортировки
static_assert(MCODE_F_KEYMACRO == 0x80C68);             // Набор простых операций
static_assert(MCODE_F_FAR_GETCONFIG == 0x80C69);        // V=Far.GetConfig(Key,Name)
static_assert(MCODE_F_MACROSETTINGS == 0x80C6A);        // Диалог редактирования макроса
static_assert(MCODE_F_LAST == 0x80C6B);                 // marker

// булевые переменные - различные состояния
static_assert(MCODE_C_AREA_OTHER == 0x80400);           // Режим копирования текста с экрана, вертикальные меню
static_assert(MCODE_C_AREA_SHELL == 0x80401);           // Файловые панели
static_assert(MCODE_C_AREA_VIEWER == 0x80402);          // Внутренняя программа просмотра
static_assert(MCODE_C_AREA_EDITOR == 0x80403);          // Редактор
static_assert(MCODE_C_AREA_DIALOG == 0x80404);          // Диалоги
static_assert(MCODE_C_AREA_SEARCH == 0x80405);          // Быстрый поиск в панелях
static_assert(MCODE_C_AREA_DISKS == 0x80406);           // Меню выбора дисков
static_assert(MCODE_C_AREA_MAINMENU == 0x80407);        // Основное меню
static_assert(MCODE_C_AREA_MENU == 0x80408);            // Прочие меню
static_assert(MCODE_C_AREA_HELP == 0x80409);            // Система помощи
static_assert(MCODE_C_AREA_INFOPANEL == 0x8040A);       // Информационная панель
static_assert(MCODE_C_AREA_QVIEWPANEL == 0x8040B);      // Панель быстрого просмотра
static_assert(MCODE_C_AREA_TREEPANEL == 0x8040C);       // Панель дерева папок
static_assert(MCODE_C_AREA_FINDFOLDER == 0x8040D);      // Поиск папок
static_assert(MCODE_C_AREA_USERMENU == 0x8040E);        // Меню пользователя
static_assert(MCODE_C_AREA_SHELL_AUTOCOMPLETION == 0x8040F);// Список автодополнения в панелях в ком.строке
static_assert(MCODE_C_AREA_DIALOG_AUTOCOMPLETION == 0x80410);// Список автодополнения в диалоге
static_assert(MCODE_C_FULLSCREENMODE == 0x80411);       // полноэкранный режим?
static_assert(MCODE_C_ISUSERADMIN == 0x80412);          // Administrator status
static_assert(MCODE_C_BOF == 0x80413);                  // начало файла/активного каталога?
static_assert(MCODE_C_EOF == 0x80414);                  // конец файла/активного каталога?
static_assert(MCODE_C_EMPTY == 0x80415);                // ком.строка пуста?
static_assert(MCODE_C_SELECTED == 0x80416);             // выделенный блок есть?
static_assert(MCODE_C_ROOTFOLDER == 0x80417);           // аналог MCODE_C_APANEL_ROOT для активной панели
static_assert(MCODE_C_APANEL_BOF == 0x80418);           // начало активного  каталога?
static_assert(MCODE_C_PPANEL_BOF == 0x80419);           // начало пассивного каталога?
static_assert(MCODE_C_APANEL_EOF == 0x8041A);           // конец активного  каталога?
static_assert(MCODE_C_PPANEL_EOF == 0x8041B);           // конец пассивного каталога?
static_assert(MCODE_C_APANEL_ISEMPTY == 0x8041C);       // активная панель:  пуста?
static_assert(MCODE_C_PPANEL_ISEMPTY == 0x8041D);       // пассивная панель: пуста?
static_assert(MCODE_C_APANEL_SELECTED == 0x8041E);      // активная панель:  выделенные элементы есть?
static_assert(MCODE_C_PPANEL_SELECTED == 0x8041F);      // пассивная панель: выделенные элементы есть?
static_assert(MCODE_C_APANEL_ROOT == 0x80420);          // это корневой каталог активной панели?
static_assert(MCODE_C_PPANEL_ROOT == 0x80421);          // это корневой каталог пассивной панели?
static_assert(MCODE_C_APANEL_VISIBLE == 0x80422);       // активная панель:  видима?
static_assert(MCODE_C_PPANEL_VISIBLE == 0x80423);       // пассивная панель: видима?
static_assert(MCODE_C_APANEL_PLUGIN == 0x80424);        // активная панель:  плагиновая?
static_assert(MCODE_C_PPANEL_PLUGIN == 0x80425);        // пассивная панель: плагиновая?
static_assert(MCODE_C_APANEL_FILEPANEL == 0x80426);     // активная панель:  файловая?
static_assert(MCODE_C_PPANEL_FILEPANEL == 0x80427);     // пассивная панель: файловая?
static_assert(MCODE_C_APANEL_FOLDER == 0x80428);        // активная панель:  текущий элемент каталог?
static_assert(MCODE_C_PPANEL_FOLDER == 0x80429);        // пассивная панель: текущий элемент каталог?
static_assert(MCODE_C_APANEL_LEFT == 0x8042A);          // активная панель левая?
static_assert(MCODE_C_PPANEL_LEFT == 0x8042B);          // пассивная панель левая?
static_assert(MCODE_C_APANEL_LFN == 0x8042C);           // на активной панели длинные имена?
static_assert(MCODE_C_PPANEL_LFN == 0x8042D);           // на пассивной панели длинные имена?
static_assert(MCODE_C_APANEL_FILTER == 0x8042E);        // на активной панели включен фильтр?
static_assert(MCODE_C_PPANEL_FILTER == 0x8042F);        // на пассивной панели включен фильтр?
static_assert(MCODE_C_CMDLINE_BOF == 0x80430);          // курсор в начале cmd-строки редактирования?
static_assert(MCODE_C_CMDLINE_EOF == 0x80431);          // курсор в конце cmd-строки редактирования?
static_assert(MCODE_C_CMDLINE_EMPTY == 0x80432);        // ком.строка пуста?
static_assert(MCODE_C_CMDLINE_SELECTED == 0x80433);     // в ком.строке есть выделение блока?
static_assert(MCODE_C_MSX == 0x80434);                  // Mouse.X
static_assert(MCODE_C_MSY == 0x80435);                  // Mouse.Y
static_assert(MCODE_C_MSBUTTON == 0x80436);             // Mouse.Button
static_assert(MCODE_C_MSCTRLSTATE == 0x80437);          // Mouse.CtrlState
static_assert(MCODE_C_MSEVENTFLAGS == 0x80438);         // Mouse.EventFlags
static_assert(MCODE_C_MSLASTCTRLSTATE == 0x80439);      // Mouse.LastCtrlState

// не булевые переменные
static_assert(MCODE_V_FAR_WIDTH == 0x80800);            // Far.Width - ширина консольного окна
static_assert(MCODE_V_FAR_HEIGHT == 0x80801);           // Far.Height - высота консольного окна
static_assert(MCODE_V_FAR_TITLE == 0x80802);            // Far.Title - текущий заголовок консольного окна
static_assert(MCODE_V_FAR_UPTIME == 0x80803);           // Far.UpTime - время работы Far в миллисекундах
static_assert(MCODE_V_FAR_PID == 0x80804);              // Far.PID - содержит ИД текущей запущенной копии Far Manager
static_assert(MCODE_V_MACRO_AREA == 0x80805);           // MacroArea - имя текущей макрос области
static_assert(MCODE_V_APANEL_CURRENT == 0x80806);       // APanel.Current - имя файла на активной панели
static_assert(MCODE_V_PPANEL_CURRENT == 0x80807);       // PPanel.Current - имя файла на пассивной панели
static_assert(MCODE_V_APANEL_SELCOUNT == 0x80808);      // APanel.SelCount - активная панель:  число выделенных элементов
static_assert(MCODE_V_PPANEL_SELCOUNT == 0x80809);      // PPanel.SelCount - пассивная панель: число выделенных элементов
static_assert(MCODE_V_APANEL_PATH == 0x8080A);          // APanel.Path - активная панель:  путь на панели
static_assert(MCODE_V_PPANEL_PATH == 0x8080B);          // PPanel.Path - пассивная панель: путь на панели
static_assert(MCODE_V_APANEL_PATH0 == 0x8080C);         // APanel.Path0 - активная панель:  путь на панели до вызова плагинов
static_assert(MCODE_V_PPANEL_PATH0 == 0x8080D);         // PPanel.Path0 - пассивная панель: путь на панели до вызова плагинов
static_assert(MCODE_V_APANEL_UNCPATH == 0x8080E);       // APanel.UNCPath - активная панель:  UNC-путь на панели
static_assert(MCODE_V_PPANEL_UNCPATH == 0x8080F);       // PPanel.UNCPath - пассивная панель: UNC-путь на панели
static_assert(MCODE_V_APANEL_WIDTH == 0x80810);         // APanel.Width - активная панель:  ширина панели
static_assert(MCODE_V_PPANEL_WIDTH == 0x80811);         // PPanel.Width - пассивная панель: ширина панели
static_assert(MCODE_V_APANEL_TYPE == 0x80812);          // APanel.Type - тип активной панели
static_assert(MCODE_V_PPANEL_TYPE == 0x80813);          // PPanel.Type - тип пассивной панели
static_assert(MCODE_V_APANEL_ITEMCOUNT == 0x80814);     // APanel.ItemCount - активная панель:  число элементов
static_assert(MCODE_V_PPANEL_ITEMCOUNT == 0x80815);     // PPanel.ItemCount - пассивная панель: число элементов
static_assert(MCODE_V_APANEL_CURPOS == 0x80816);        // APanel.CurPos - активная панель:  текущий индекс
static_assert(MCODE_V_PPANEL_CURPOS == 0x80817);        // PPanel.CurPos - пассивная панель: текущий индекс
static_assert(MCODE_V_APANEL_OPIFLAGS == 0x80818);      // APanel.OPIFlags - активная панель: флаги открытого плагина
static_assert(MCODE_V_PPANEL_OPIFLAGS == 0x80819);      // PPanel.OPIFlags - пассивная панель: флаги открытого плагина
static_assert(MCODE_V_APANEL_DRIVETYPE == 0x8081A);     // APanel.DriveType - активная панель: тип привода
static_assert(MCODE_V_PPANEL_DRIVETYPE == 0x8081B);     // PPanel.DriveType - пассивная панель: тип привода
static_assert(MCODE_V_APANEL_HEIGHT == 0x8081C);        // APanel.Height - активная панель:  высота панели
static_assert(MCODE_V_PPANEL_HEIGHT == 0x8081D);        // PPanel.Height - пассивная панель: высота панели
static_assert(MCODE_V_APANEL_COLUMNCOUNT == 0x8081E);   // APanel.ColumnCount - активная панель:  количество колонок
static_assert(MCODE_V_PPANEL_COLUMNCOUNT == 0x8081F);   // PPanel.ColumnCount - пассивная панель: количество колонок
static_assert(MCODE_V_APANEL_HOSTFILE == 0x80820);      // APanel.HostFile - активная панель:  имя Host-файла
static_assert(MCODE_V_PPANEL_HOSTFILE == 0x80821);      // PPanel.HostFile - пассивная панель: имя Host-файла
static_assert(MCODE_V_APANEL_PREFIX == 0x80822);        // APanel.Prefix
static_assert(MCODE_V_PPANEL_PREFIX == 0x80823);        // PPanel.Prefix
static_assert(MCODE_V_APANEL_FORMAT == 0x80824);        // APanel.Format
static_assert(MCODE_V_PPANEL_FORMAT == 0x80825);        // PPanel.Format
static_assert(MCODE_V_ITEMCOUNT == 0x80826);            // ItemCount - число элементов в текущем объекте
static_assert(MCODE_V_CURPOS == 0x80827);               // CurPos - текущий индекс в текущем объекте
static_assert(MCODE_V_TITLE == 0x80828);                // Title - заголовок текущего объекта
static_assert(MCODE_V_HEIGHT == 0x80829);               // Height - высота текущего объекта
static_assert(MCODE_V_WIDTH == 0x8082A);                // Width - ширина текущего объекта
static_assert(MCODE_V_EDITORFILENAME == 0x8082B);       // Editor.FileName - имя редактируемого файла
static_assert(MCODE_V_EDITORLINES == 0x8082C);          // Editor.Lines - количество строк в редакторе
static_assert(MCODE_V_EDITORCURLINE == 0x8082D);        // Editor.CurLine - текущая линия в редакторе (в дополнении к Count)
static_assert(MCODE_V_EDITORCURPOS == 0x8082E);         // Editor.CurPos - текущая поз. в редакторе
static_assert(MCODE_V_EDITORREALPOS == 0x8082F);        // Editor.RealPos - текущая поз. в редакторе без привязки к размеру табуляции
static_assert(MCODE_V_EDITORSTATE == 0x80830);          // Editor.State
static_assert(MCODE_V_EDITORVALUE == 0x80831);          // Editor.Value - содержимое текущей строки
static_assert(MCODE_V_EDITORSELVALUE == 0x80832);       // Editor.SelValue - содержит содержимое выделенного блока
static_assert(MCODE_V_DLGITEMTYPE == 0x80833);          // Dlg->ItemType
static_assert(MCODE_V_DLGITEMCOUNT == 0x80834);         // Dlg->ItemCount
static_assert(MCODE_V_DLGCURPOS == 0x80835);            // Dlg->CurPos
static_assert(MCODE_V_DLGPREVPOS == 0x80836);           // Dlg->PrevPos
static_assert(MCODE_V_DLGINFOID == 0x80837);            // Dlg->Info.Id
static_assert(MCODE_V_DLGINFOOWNER == 0x80838);         // Dlg->Info.Owner
static_assert(MCODE_V_VIEWERFILENAME == 0x80839);       // Viewer.FileName - имя просматриваемого файла
static_assert(MCODE_V_VIEWERSTATE == 0x8083A);          // Viewer.State
static_assert(MCODE_V_CMDLINE_ITEMCOUNT == 0x8083B);    // CmdLine.ItemCount
static_assert(MCODE_V_CMDLINE_CURPOS == 0x8083C);       // CmdLine.CurPos
static_assert(MCODE_V_CMDLINE_VALUE == 0x8083D);        // CmdLine.Value
static_assert(MCODE_V_DRVSHOWPOS == 0x8083E);           // Drv.ShowPos - меню выбора дисков отображено: 1=слева (Alt-F1), 2=справа (Alt-F2), 0="нету его"
static_assert(MCODE_V_DRVSHOWMODE == 0x8083F);          // Drv.ShowMode - режимы отображения меню выбора дисков
static_assert(MCODE_V_HELPFILENAME == 0x80840);         // Help.FileName
static_assert(MCODE_V_HELPTOPIC == 0x80841);            // Help.Topic
static_assert(MCODE_V_HELPSELTOPIC == 0x80842);         // Help.SelTopic
static_assert(MCODE_V_MENU_VALUE == 0x80843);           // Menu.Value
static_assert(MCODE_V_MENUINFOID == 0x80844);           // Menu.Info.Id

// для диалога назначения клавиши
struct DlgParam
{
	unsigned long long Flags;
	DWORD Key;
	bool Changed;
};

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
	FarMacroValue values[]{ static_cast<double> (OpCode), Param };
	FarMacroCall fmc{ sizeof(fmc), 2, values };
	OpenMacroPluginInfo info{ MCT_KEYMACRO, &fmc };

	if (!CallMacroPlugin(&info))
		return false;

	if (Ret)
		*Ret=info.Ret;

	return true;
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

DWORD SetHistoryDisableMask(DWORD Mask)
{
	MacroPluginReturn Ret;
	return MacroPluginOp(OP_HISTORYDISABLEMASK, static_cast<double>(Mask), &Ret)? Ret.ReturnType : 0;
}

DWORD GetHistoryDisableMask()
{
	MacroPluginReturn Ret;
	return MacroPluginOp(OP_HISTORYDISABLEMASK,false,&Ret) ? Ret.ReturnType : 0;
}

bool KeyMacro::IsHistoryDisabled(int TypeHistory)
{
	MacroPluginReturn Ret;
	return MacroPluginOp(OP_ISHISTORYDISABLE, TypeHistory, &Ret)? Ret.ReturnType != 0 : false;
}

bool IsTopMacroOutputDisabled()
{
	MacroPluginReturn Ret;
	return MacroPluginOp(OP_ISTOPMACROOUTPUTDISABLED,false,&Ret) ? Ret.ReturnType != 0 : false;
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
	FarMacroValue values[]{ static_cast<double>(OP_TRYTOPOSTMACRO), static_cast<double>(Area),
		TextKey, static_cast<double>(IntKey) };
	FarMacroCall fmc{ sizeof(fmc), std::size(values), values };
	OpenMacroPluginInfo info{ MCT_KEYMACRO, &fmc };
	return CallMacroPlugin(&info);
}

KeyMacro::KeyMacro():
	m_Area(MACROAREA_SHELL),
	m_StartArea(MACROAREA_OTHER),
	m_Recording(MACROSTATE_NOMACRO)
{
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

	FarMacroValue values[]{ InitedRAM, false, 0.0 };
	if (Data)
	{
		if (Data->Path) values[1] = Data->Path;
		values[2] = static_cast<double>(Data->Flags);
	}
	FarMacroCall fmc{ sizeof(fmc), std::size(values), values };
	OpenMacroPluginInfo info{ MCT_LOADMACROS, &fmc };
	return CallMacroPlugin(&info);
}

bool KeyMacro::SaveMacros()
{
	OpenMacroPluginInfo info{ MCT_WRITEMACROS };
	return CallMacroPlugin(&info);
}

static long long msValues[constMsLAST];

void KeyMacro::SetMacroConst(int ConstIndex, long long Value)
{
	msValues[ConstIndex] = Value;
}

long long KeyMacro::GetMacroConst(int ConstIndex)
{
	return msValues[ConstIndex];
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

	if (m_Area == MACROAREA_EDITOR)
	{
		if (const auto CurrentEditor = Global->WindowManager->GetCurrentEditor(); CurrentEditor && CurrentEditor->IsVisible())
		{
			CurrentEditor->Show();
		}
	}
	else if (m_Area == MACROAREA_VIEWER)
	{
		if (const auto CurrentViewer = Global->WindowManager->GetCurrentViewer(); CurrentViewer && CurrentViewer->IsVisible())
		{
			CurrentViewer->Show(); // иначе может быть неправильный верхний левый символ экрана
		}
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
	FarMacroValue InValues[]{ static_cast<double>(Area), TextKey, UseCommon };
	FarMacroCall fmc{ sizeof(fmc), std::size(InValues), InValues };
	OpenMacroPluginInfo info{ MCT_GETMACRO, &fmc };

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
	FarMacroValue values[]{ static_cast<double>(Area), TextKey, Code, Flags, Description };
	FarMacroCall fmc{ sizeof(fmc), std::size(values), values };
	OpenMacroPluginInfo info{ MCT_RECORDEDMACRO, &fmc };
	CallMacroPlugin(&info);
}

bool KeyMacro::ProcessEvent(const FAR_INPUT_RECORD *Rec)
{
	if (m_InternalInput || Rec->IntKey == KEY_NONE || !Global->WindowManager->GetCurrentWindow()) //FIXME: избавиться от Rec->IntKey
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
				static auto Processing = false;

				if (Processing)
					return false;

				Processing = true;
				SCOPE_EXIT{ Processing = false; };

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
			m_StartArea=m_Area;
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
				if ((key & 0x00FFFFFF) < 0xFFFF)
					key = upper(KeyToKeyLayout(key & 0x0000FFFF)) | (key & 0xFFFF0000);

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
			DlgParam Param {0,0,false};
			bool AssignRet = AssignMacroKey(&Param);

			if (AssignRet && !Param.Changed && !m_RecCode.empty())
			{
				m_RecCode = concat(L"Keys(\""sv, m_RecCode, L"\")"sv);
				// добавим проверку на удаление
				// если удаляем или был вызван диалог изменения, то не нужно выдавать диалог настройки.
				if (ctrlshiftdot && !GetMacroSettings(Param.Key, Param.Flags))
				{
					AssignRet = false;
				}
			}
			m_InternalInput=0;
			if (AssignRet)
			{
				const auto strKey = KeyToText(Param.Key);
				Param.Flags |= m_Recording == MACROSTATE_RECORDING_COMMON? MFLAGS_NONE : MFLAGS_NOSENDKEYSTOPLUGINS;
				LM_ProcessRecordedMacro(m_StartArea, strKey, m_RecCode, Param.Flags, m_RecDescription);
				if (Global->Opt->AutoSaveSetup)
					SaveMacros();
			}

			m_Recording=MACROSTATE_NOMACRO;
			Global->ScrBuf->RestoreMacroChar();
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

void ShowUserMenu(size_t Count, const FarMacroValue *Values)
{
	if (Count==0)
		user_menu(false);
	else if (Values[0].Type==FMVT_BOOLEAN)
		user_menu(Values[0].Boolean != 0);
	else if (Values[0].Type==FMVT_STRING)
		user_menu(string_view(Values[0].String));
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
				if (m_Area == MACROAREA_EDITOR)
				{
					if (const auto CurrentEditor = Global->WindowManager->GetCurrentEditor(); CurrentEditor && CurrentEditor->IsVisible() && Global->ScrBuf->GetLockCount())
					{
						CurrentEditor->Show();
					}
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

				const auto Uuid = uuid::try_parse(mpr.Values[0].String);
				if (!Uuid)
					break;

				if (!Global->CtrlObject->Plugins->FindPlugin(*Uuid))
					break;

				PluginManager::CallPluginInfo cpInfo{ CPT_CHECKONLY };
				const auto Arg = mpr.Count > 1 && mpr.Values[1].Type == FMVT_STRING? mpr.Values[1].String : L"";

				UUID MenuUuid;
				if (*Arg && (mpr.ReturnType==MPRT_PLUGINMENU || mpr.ReturnType==MPRT_PLUGINCONFIG))
				{
					if (const auto MenuUuidOpt = uuid::try_parse(Arg))
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

			case MPRT_FOLDERSHORTCUTS:
				if (IsPanelsArea(m_Area))
				{
					const auto Result = Shortcuts::Configure();
					if (Result != -1)
						Global->CtrlObject->Cp()->ActivePanel()->ExecShortcutFolder(Result);
				}
				break;

			case MPRT_FILEASSOCIATIONS:
				if (IsPanelsArea(m_Area))
				{
					EditFileTypes();
				}
				break;

			case MPRT_FILEHIGHLIGHT:
				if (IsPanelsArea(m_Area))
				{
					Global->CtrlObject->HiFiles->HiEdit(0);
				}
				break;

			case MPRT_FILEPANELMODES:
				if (IsPanelsArea(m_Area))
				{
					Global->Opt->SetFilePanelModes();
				}
				break;

			case MPRT_FILEMASKGROUPS:
				if (IsPanelsArea(m_Area))
				{
					Global->Opt->MaskGroupsSettings();
				}
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
	FarMacroValue values[]{ StrArea, !Pos };
	FarMacroCall fmc{ sizeof(fmc), std::size(values), values };
	OpenMacroPluginInfo info{ MCT_ENUMMACROS, &fmc };

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
		OpenMacroPluginInfo info{ MCT_RUNSTARTMACRO };
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
	if (Data->StructSize >= sizeof(MacroAddMacro))
		Priority = std::bit_cast<const MacroAddMacro*>(Data)->Priority;

	FarMacroValue values[]
	{
		static_cast<double>(Data->Area),
		KeyText,
		GetMacroLanguage(Data->Flags),
		Data->SequenceText,
		Flags,
		Data->Description,
		PluginId,
		{std::bit_cast<void*>(Data->Callback)},
		Data->Id,
		Priority
	};
	FarMacroCall fmc{ sizeof(fmc), std::size(values), values };
	OpenMacroPluginInfo info{ MCT_ADDMACRO, &fmc };
	return CallMacroPlugin(&info);
}

bool KeyMacro::DelMacro(const UUID& PluginId, void* Id)
{
	FarMacroValue values[]{ PluginId, Id };
	FarMacroCall fmc{ sizeof(fmc), std::size(values), values };
	OpenMacroPluginInfo info{ MCT_DELMACRO, &fmc };
	return CallMacroPlugin(&info);
}

bool KeyMacro::PostNewMacro(const wchar_t* Sequence,FARKEYMACROFLAGS InputFlags,DWORD AKey)
{
	const wchar_t* Lang = GetMacroLanguage(InputFlags);
	const auto onlyCheck = (InputFlags & KMFLAGS_SILENTCHECK) != 0;
	MACROFLAGS_MFLAGS Flags = MFLAGS_POSTFROMPLUGIN;
	if (InputFlags & KMFLAGS_ENABLEOUTPUT)        Flags |= MFLAGS_ENABLEOUTPUT;
	if (InputFlags & KMFLAGS_NOSENDKEYSTOPLUGINS) Flags |= MFLAGS_NOSENDKEYSTOPLUGINS;

	FarMacroValue values[]{ static_cast<double>(OP_POSTNEWMACRO), Lang, Sequence,
		static_cast<double>(Flags), static_cast<double>(AKey), onlyCheck };
	FarMacroCall fmc{ sizeof(fmc), std::size(values), values };
	OpenMacroPluginInfo info{ MCT_KEYMACRO, &fmc };
	return CallMacroPlugin(&info);
}

bool KeyMacro::IsMacroDialog(window_ptr const& Window)
{
	const auto Dlg = dynamic_cast<Dialog*>(Window.get());
	if (!Dlg)
		return false;

	const auto PluginOwner = Dlg->GetPluginOwner();
	if (!PluginOwner)
		return false;

	return PluginOwner->Id() == Global->Opt->KnownIDs.Luamacro.Id;
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

bool CheckAll(FARMACROAREA Area, MACROFLAGS_MFLAGS CurFlags)
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
	bool b1 = (Flags & Chk1) != 0;
	bool b2 = (Flags & Chk2) != 0;
	return b1==b2 ? 2 : b1 ? 1 : 0;
}

static DWORD Get3State(int Selected,DWORD Chk1,DWORD Chk2)
{
	return Selected==2 ? 0 : Selected==0 ? Chk1 : Chk2;
}

enum MACROSETTINGSDLG
{
	MS_DOUBLEBOX,
	MS_TEXT_SEQUENCE,
	MS_EDIT_SEQUENCE,
	MS_TEXT_DESCR,
	MS_EDIT_DESCR,
	MS_SEPARATOR1,
	MS_CHECKBOX_OUTPUT,
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
			{
				for (const auto i: std::views::iota(1, 4))
					Dlg->SendMessage(DM_ENABLE,Param1+i,Param2);
			}
			break;
		case DN_CLOSE:

			if (Param1==MS_BUTTON_OK)
			{
				const auto Sequence = std::bit_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, MS_EDIT_SEQUENCE, nullptr));
				if (*Sequence)
				{
					if (ParseMacroString(Sequence,KMFLAGS_LUA,true))
					{
						m_RecCode=Sequence;
						m_RecDescription = std::bit_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, MS_EDIT_DESCR, nullptr));
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
		{ DI_CHECKBOX,  {{7,  11}, {0,  11}}, DIF_3STATE, msg(lng::MMacroSettingsPluginPanel), },
		{ DI_CHECKBOX,  {{7,  12}, {0,  12}}, DIF_3STATE, msg(lng::MMacroSettingsFolders), },
		{ DI_CHECKBOX,  {{7,  13}, {0,  13}}, DIF_3STATE, msg(lng::MMacroSettingsSelectionPresent), },
		{ DI_CHECKBOX,  {{37, 10}, {0,  10}}, DIF_NONE, msg(lng::MMacroSettingsPassivePanel), },
		{ DI_CHECKBOX,  {{39, 11}, {0,  11}}, DIF_3STATE, msg(lng::MMacroSettingsPluginPanel), },
		{ DI_CHECKBOX,  {{39, 12}, {0,  12}}, DIF_3STATE, msg(lng::MMacroSettingsFolders), },
		{ DI_CHECKBOX,  {{39, 13}, {0,  13}}, DIF_3STATE, msg(lng::MMacroSettingsSelectionPresent), },
		{ DI_TEXT,      {{-1, 14}, {0,  14}}, DIF_SEPARATOR, },
		{ DI_CHECKBOX,  {{5,  15}, {0,  15}}, DIF_3STATE, msg(lng::MMacroSettingsCommandLine), },
		{ DI_CHECKBOX,  {{5,  16}, {0,  16}}, DIF_3STATE, msg(lng::MMacroSettingsSelectionBlockPresent), },
		{ DI_TEXT,      {{-1, 17}, {0,  17}}, DIF_SEPARATOR, },
		{ DI_BUTTON,    {{0,  18}, {0,  18}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MOk), },
		{ DI_BUTTON,    {{0,  18}, {0,  18}}, DIF_CENTERGROUP, msg(lng::MCancel), },
	});

	MacroSettingsDlg[MS_EDIT_SEQUENCE].strHistory = L"MacroSequence"sv;
	MacroSettingsDlg[MS_EDIT_DESCR].strHistory = L"MacroDescription"sv;

	MacroSettingsDlg[MS_DOUBLEBOX].strData = far::vformat(msg(lng::MMacroSettingsTitle), KeyToText(Key));
	//if(!(Key&0x7F000000))
	//MacroSettingsDlg[3].Flags|=DIF_DISABLE;
	MacroSettingsDlg[MS_CHECKBOX_OUTPUT].Selected=Flags&MFLAGS_ENABLEOUTPUT?1:0;
	MacroSettingsDlg[MS_CHECKBOX_START].Selected=Flags&MFLAGS_RUNAFTERFARSTART?1:0;

	int a = Set3State(Flags,MFLAGS_NOFILEPANELS,MFLAGS_NOPLUGINPANELS);
	int b = Set3State(Flags,MFLAGS_NOFILES,MFLAGS_NOFOLDERS);
	int c = Set3State(Flags,MFLAGS_SELECTION,MFLAGS_NOSELECTION);
	MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected = a;
	MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected = b;
	MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected = c;
	MacroSettingsDlg[MS_CHECKBOX_A_PANEL].Selected = (a!=2 || b!=2 || c!= 2) ? 1 : 0;
	if (0 == MacroSettingsDlg[MS_CHECKBOX_A_PANEL].Selected)
	{
		MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Flags |= DIF_DISABLE;
		MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Flags |= DIF_DISABLE;
		MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Flags |= DIF_DISABLE;
	}

	a = Set3State(Flags,MFLAGS_PNOFILEPANELS,MFLAGS_PNOPLUGINPANELS);
	b = Set3State(Flags,MFLAGS_PNOFILES,MFLAGS_PNOFOLDERS);
	c = Set3State(Flags,MFLAGS_PSELECTION,MFLAGS_PNOSELECTION);
	MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected = a;
	MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected = b;
	MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected = c;
	MacroSettingsDlg[MS_CHECKBOX_P_PANEL].Selected = (a!=2 || b!=2 || c!= 2) ? 1 : 0;
	if (0 == MacroSettingsDlg[MS_CHECKBOX_P_PANEL].Selected)
	{
		MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Flags |= DIF_DISABLE;
		MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Flags |= DIF_DISABLE;
		MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Flags |= DIF_DISABLE;
	}

	MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected=Set3State(Flags,MFLAGS_EMPTYCOMMANDLINE,MFLAGS_NOTEMPTYCOMMANDLINE);
	MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected=Set3State(Flags,MFLAGS_EDITSELECTION,MFLAGS_EDITNOSELECTION);
	MacroSettingsDlg[MS_EDIT_SEQUENCE].strData = Src.empty()? m_RecCode : Src;
	MacroSettingsDlg[MS_EDIT_DESCR].strData = Descr.empty()? m_RecDescription : Descr;

	const auto Dlg = Dialog::create(MacroSettingsDlg, std::bind_front(&KeyMacro::ParamMacroDlgProc, this), nullptr);
	Dlg->SetPosition({ -1, -1, 73, 21 });
	Dlg->SetHelp(L"KeyMacroSetting"sv);
	Dlg->Process();

	if (Dlg->GetExitCode()!=MS_BUTTON_OK)
		return false;

	Flags=MacroSettingsDlg[MS_CHECKBOX_OUTPUT].Selected?MFLAGS_ENABLEOUTPUT:MFLAGS_NONE;
	Flags|=MacroSettingsDlg[MS_CHECKBOX_START].Selected?MFLAGS_RUNAFTERFARSTART:MFLAGS_NONE;

	if (MacroSettingsDlg[MS_CHECKBOX_A_PANEL].Selected)
	{
		Flags |= Get3State(MacroSettingsDlg[MS_CHECKBOX_A_PLUGINPANEL].Selected,
		         MFLAGS_NOPLUGINPANELS, MFLAGS_NOFILEPANELS);
		Flags |= Get3State(MacroSettingsDlg[MS_CHECKBOX_A_FOLDERS].Selected,
		         MFLAGS_NOFOLDERS, MFLAGS_NOFILES);
		Flags |= Get3State(MacroSettingsDlg[MS_CHECKBOX_A_SELECTION].Selected,
		         MFLAGS_NOSELECTION, MFLAGS_SELECTION);
	}

	if (MacroSettingsDlg[MS_CHECKBOX_P_PANEL].Selected)
	{
		Flags |= Get3State(MacroSettingsDlg[MS_CHECKBOX_P_PLUGINPANEL].Selected,
		         MFLAGS_PNOPLUGINPANELS, MFLAGS_PNOFILEPANELS);
		Flags |= Get3State(MacroSettingsDlg[MS_CHECKBOX_P_FOLDERS].Selected,
		         MFLAGS_PNOFOLDERS, MFLAGS_PNOFILES);
		Flags |= Get3State(MacroSettingsDlg[MS_CHECKBOX_P_SELECTION].Selected,
		         MFLAGS_PNOSELECTION, MFLAGS_PSELECTION);
	}

	Flags |= Get3State(MacroSettingsDlg[MS_CHECKBOX_CMDLINE].Selected,
	         MFLAGS_NOTEMPTYCOMMANDLINE, MFLAGS_EMPTYCOMMANDLINE);
	Flags |= Get3State(MacroSettingsDlg[MS_CHECKBOX_SELBLOCK].Selected,
	         MFLAGS_EDITNOSELECTION, MFLAGS_EDITSELECTION);
	return true;
}

bool KeyMacro::ParseMacroString(const wchar_t* Sequence, FARKEYMACROFLAGS Flags, bool skipFile) const
{
	const auto lang = GetMacroLanguage(Flags);
	const auto onlyCheck = (Flags&KMFLAGS_SILENTCHECK) != 0;

	// Перекладываем вывод сообщения об ошибке на плагин, т.к. штатный Message()
	// не умеет сворачивать строки и обрезает сообщение.
	FarMacroValue values[]{ lang, Sequence, onlyCheck, skipFile };
	FarMacroCall fmc{ sizeof(fmc), std::size(values), values };
	OpenMacroPluginInfo info{ MCT_MACROPARSE, &fmc };

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
	FarMacroValue values[]{ GetMacroLanguage(Data->Flags), Data->SequenceText, FarMacroValue(Data->InValues, Data->InCount), onlyCheck };
	FarMacroCall fmc{ sizeof(fmc), std::size(values), values };
	OpenMacroPluginInfo info{ MCT_EXECSTRING, &fmc };

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

// обработчик диалогового окна назначения клавиши
intptr_t KeyMacro::AssignMacroDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	static int Recurse;
	static unsigned LastKey;
	static DlgParam *KMParam;
	const INPUT_RECORD* record=nullptr;
	unsigned key = 0;
	bool KeyIsValid = false;

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

	if (Msg == DN_INITDIALOG)
	{
		KMParam = static_cast<DlgParam*>(Param2);
		LastKey=0;
		Recurse=0;
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
		key = KeyNameToKey(static_cast<FarDialogItem*>(Param2)->Data);

		if (key && !Recurse)
			KeyIsValid = true;
	}
	else if (Msg == DN_CONTROLINPUT && record->EventType==KEY_EVENT && (((key&KEY_END_SKEY) < KEY_END_FKEY) ||
	                           (((key&KEY_END_SKEY) > INTERNAL_KEY_BASE) && (key&KEY_END_SKEY) < INTERNAL_KEY_BASE_2)))
	{
		//if((key&0x00FFFFFF) >= 'A' && (key&0x00FFFFFF) <= 'Z' && ShiftPressed)
		//key|=KEY_SHIFT;

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
		if (any_of(key, KEY_ENTER, KEY_NUMENTER) && LastKey && none_of(LastKey, KEY_ENTER, KEY_NUMENTER))
			return FALSE;

		KeyIsValid = true;
	}

	if (KeyIsValid)
	{
		if ((key & 0x00FFFFFF) < 0xFFFF)
			key = upper(KeyToKeyLayout(key & 0x0000FFFF)) | (key & 0xFFFF0000);

		KMParam->Key = static_cast<DWORD>(key);
		auto strKeyText = KeyToText(key);

		// если УЖЕ есть такой макрос...
		GetMacroData Data;
		if (LM_GetMacro(&Data,m_StartArea,strKeyText,true) && Data.IsKeyboardMacro)
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
				const auto strBuf = far::vformat(msg(MessageTemplate), strKeyText);

				const std::array ChangeButtons{ lng::MYes, lng::MMacroEditKey, lng::MNo };
				const std::array NoChangeButtons{ lng::MYes, lng::MNo };

				const auto Result = Message(MSG_WARNING,
					msg(lng::MWarning),
					{
						strBuf,
						msg(lng::MMacroSequence),
						strBufKey,
						msg(SetChange? lng::MMacroDeleteKey2 : lng::MMacroReDefinedKey2)
					},
					SetChange? span(ChangeButtons) : span(NoChangeButtons));

				if (Result == message_result::first_button)
				{
					// в любом случае - вываливаемся
					Dlg->SendMessage(DM_CLOSE, 1, nullptr);
					return TRUE;
				}

				if (SetChange && Result == message_result::second_button)
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

		Recurse++;
		Dlg->SendMessage(DM_SETTEXTPTR,2, UNSAFE_CSTR(strKeyText));
		Recurse--;
		//if(key == KEY_F1 && LastKey == KEY_F1)
		//LastKey=-1;
		//else
		LastKey = key;
		return TRUE;
	}
	return Dlg->DefProc(Msg,Param1,Param2);
}

bool KeyMacro::AssignMacroKey(DlgParam *Param)
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

	Global->IsProcessAssignMacroKey++;
	const auto Dlg = Dialog::create(MacroAssignDlg, std::bind_front(&KeyMacro::AssignMacroDlgProc, this), Param);
	Dlg->SetPosition({ -1, -1, 34, 6 });
	Dlg->SetHelp(L"KeyMacro"sv);
	Dlg->Process();
	Global->IsProcessAssignMacroKey--;

	return (Dlg->GetExitCode() >= 0);
}
