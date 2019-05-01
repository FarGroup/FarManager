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

// Internal:

// Platform:

// Common:
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

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

constexpr auto CUSTOM_COLUMN_MAX = CUSTOM_COLUMN0 + 99;

enum FILEPANEL_COLUMN_MODES: unsigned long long
{
	COLUMN_MARK                   = 63_bit,
	COLUMN_NAMEONLY               = 62_bit,
	COLUMN_RIGHTALIGN             = 61_bit,
	COLUMN_GROUPDIGITS            = 59_bit, // Вставлять разделитель между тысячами
	COLUMN_THOUSAND               = 58_bit, // Вместо делителя 1024 использовать делитель 1000
	COLUMN_BRIEF                  = 57_bit,
	COLUMN_MONTH                  = 56_bit,
	COLUMN_FLOATSIZE              = 55_bit, // Показывать размер в виде десятичной дроби, используя наиболее подходящую единицу измерения, например 0,97 К, 1,44 М, 53,2 Г.

	COLUMN_ECONOMIC               = 54_bit, // Экономичный режим, не показывать пробел перед суффиксом размера файла (т.е. 0.97K)
	COLUMN_SHOWUNIT               = 52_bit, // Показывать суффиксы B,K,M,G,T,P,E
	COLUMN_FULLOWNER              = 51_bit,
	COLUMN_NOEXTENSION            = 50_bit,
	COLUMN_RIGHTALIGNFORCE        = 48_bit,
	COLUMN_MARK_DYNAMIC           = 47_bit,

	COLUMN_USE_UNIT               = 53_bit, // Минимально допустимая единица измерения при форматировании например, 1 - "размер как минимум в мегабайтах"
	COLUMN_UNIT_K                 = COLUMN_USE_UNIT | 0,
	COLUMN_UNIT_M                 = COLUMN_USE_UNIT | 1,
	COLUMN_UNIT_G                 = COLUMN_USE_UNIT | 2,
	COLUMN_UNIT_T                 = COLUMN_USE_UNIT | 3,
	COLUMN_UNIT_P                 = COLUMN_USE_UNIT | 4,
	COLUMN_UNIT_E                 = COLUMN_USE_UNIT | 5,
	COLUMN_UNIT_MASK              = bit(4) - 1,
};

enum col_width
{
	fixed,
	percent,
};

#endif // PANELCTYPE_HPP_C90AC741_F7E6_44D6_92BA_B4C0047E9E19
