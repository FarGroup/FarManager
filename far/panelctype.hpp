#ifndef PANELCTYPE_HPP_C90AC741_F7E6_44D6_92BA_B4C0047E9E19
#define PANELCTYPE_HPP_C90AC741_F7E6_44D6_92BA_B4C0047E9E19
#pragma once

/*
panelctype.hpp

Файловая панель - типы панелей
*/
/*
Copyright © 2010 Far Group
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


enum PANEL_COLUMN_TYPE
{
	NAME_COLUMN,
	SIZE_COLUMN,
	PACKED_COLUMN,
	DATE_COLUMN,
	TIME_COLUMN,
	WDATE_COLUMN,
	CDATE_COLUMN,
	ADATE_COLUMN,
	CHDATE_COLUMN,
	ATTR_COLUMN,
	DIZ_COLUMN,
	OWNER_COLUMN,
	NUMLINK_COLUMN,
	NUMSTREAMS_COLUMN,
	STREAMSSIZE_COLUMN,
	EXTENSION_COLUMN,
	CUSTOM_COLUMN0,

	COLUMN_TYPES_COUNT
};

#define CUSTOM_COLUMN_MAX (CUSTOM_COLUMN0 + 99)

enum FILEPANEL_COLUMN_MODES: unsigned long long
{
	COLUMN_MARK                   = 0x8000000000000000LL,
	COLUMN_NAMEONLY               = 0x4000000000000000LL,
	COLUMN_RIGHTALIGN             = 0x2000000000000000LL,
	COLUMN_FORMATTED              = 0x1000000000000000LL,
	COLUMN_COMMAS                 = 0x0800000000000000LL, // Вставлять разделитель между тысячами
	COLUMN_THOUSAND               = 0x0400000000000000LL, // Вместо делителя 1024 использовать делитель 1000
	COLUMN_BRIEF                  = 0x0200000000000000LL,
	COLUMN_MONTH                  = 0x0100000000000000LL,
	COLUMN_FLOATSIZE              = 0x0080000000000000LL, // Показывать размер файла в стиле Windows Explorer (т.е. 999 байт будут показаны как 999, а 1000 байт как 0.97 K)
	COLUMN_ECONOMIC               = 0x0040000000000000LL, // Экономичный режим, не показывать пробел перед суффиксом размера файла (т.е. 0.97K)
	COLUMN_MINSIZEINDEX           = 0x0020000000000000LL, // Минимально допустимый индекс при форматировании
	COLUMN_SHOWBYTESINDEX         = 0x0010000000000000LL, // Показывать суффиксы B,K,M,G,T,P,E
	COLUMN_FULLOWNER              = 0x0008000000000000LL,
	COLUMN_NOEXTENSION            = 0x0004000000000000LL,
	COLUMN_CENTERALIGN            = 0x0002000000000000LL,
	COLUMN_RIGHTALIGNFORCE        = 0x0001000000000000LL,
	COLUMN_MARK_DYNAMIC           = 0x0000800000000000LL,

	COLUMN_MINSIZEINDEX_MASK      = 0x0000000000000003LL  // MINSIZEINDEX может быть только 0, 1, 2 или 3 (K,M,G,T), например, 1 - "размер как минимум в мегабайтах"
};

enum col_width
{
	fixed,
	percent,
};

#endif // PANELCTYPE_HPP_C90AC741_F7E6_44D6_92BA_B4C0047E9E19
