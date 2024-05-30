#ifndef MACROVALUES_HPP_13716113_3069_4047_9059_9CF2C42DA8DA
#define MACROVALUES_HPP_13716113_3069_4047_9059_9CF2C42DA8DA
#pragma once

/*
macrovalues.hpp

This file defines values used by both Far and plugin LuaMacro
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

enum MACRO_OP
{
	OP_ISEXECUTING                 = 1,
	OP_ISDISABLEOUTPUT             = 2,
	OP_HISTORYDISABLEMASK          = 3,
	OP_ISHISTORYDISABLE            = 4,
	OP_ISTOPMACROOUTPUTDISABLED    = 5,
	OP_ISPOSTMACROENABLED          = 6,
	OP_POSTNEWMACRO                = 7,
	OP_SETMACROVALUE               = 8,
	OP_GETINPUTFROMMACRO           = 9,
	OP_TRYTOPOSTMACRO              = 10,
	OP_GETLASTERROR                = 11,
};

enum MACRO_IMPORT
{
	IMP_RESTORE_MACROCHAR          = 1,
	IMP_SCRBUF_LOCK                = 2,
	IMP_SCRBUF_UNLOCK              = 3,
	IMP_SCRBUF_RESETLOCKCOUNT      = 4,
	IMP_SCRBUF_GETLOCKCOUNT        = 5,
	IMP_SCRBUF_SETLOCKCOUNT        = 6,
	IMP_GET_USEINTERNALCLIPBOARD   = 7,
	IMP_SET_USEINTERNALCLIPBOARD   = 8,
	IMP_KEYNAMETOKEY               = 9,
	IMP_KEYTOTEXT                  = 10,
};

bool CheckAll(FARMACROAREA Area, MACROFLAGS_MFLAGS CurFlags);

#endif // MACROVALUES_HPP_13716113_3069_4047_9059_9CF2C42DA8DA
