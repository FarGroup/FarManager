#ifndef MACROVALUES_HPP_13716113_3069_4047_9059_9CF2C42DA8DA
#define MACROVALUES_HPP_13716113_3069_4047_9059_9CF2C42DA8DA
#pragma once

// This file defines values used by both Far and plugin LuaMacro

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

#endif
