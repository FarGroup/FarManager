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

enum class column_type
{
	name,
	size,
	size_compressed,
	date,
	time,
	date_write,
	date_creation,
	date_access,
	date_change,
	attributes,
	description,
	owner,
	links_number,
	streams_number,
	streams_size,
	extension,
	custom_0,

	count,
	custom_max = custom_0 + 99
};


enum FILEPANEL_COLUMN_FLAGS: unsigned long long
{
	COLFLAGS_NONE                   = 0,
	// First 3 bits are for the multipliers below
	COLFLAGS_RESERVED_BIT_0         = 0_bit,
	COLFLAGS_RESERVED_BIT_1         = 1_bit,
	COLFLAGS_RESERVED_BIT_2         = 2_bit,
	COLFLAGS_USE_MULTIPLIER         = 3_bit,
	COLFLAGS_MULTIPLIER_K           = COLFLAGS_USE_MULTIPLIER + 0,
	COLFLAGS_MULTIPLIER_M           = COLFLAGS_USE_MULTIPLIER + 1,
	COLFLAGS_MULTIPLIER_G           = COLFLAGS_USE_MULTIPLIER + 2,
	COLFLAGS_MULTIPLIER_T           = COLFLAGS_USE_MULTIPLIER + 3,
	COLFLAGS_MULTIPLIER_P           = COLFLAGS_USE_MULTIPLIER + 4,
	COLFLAGS_MULTIPLIER_E           = COLFLAGS_USE_MULTIPLIER + 5,
	COLFLAGS_MULTIPLIER_MASK        = COLFLAGS_RESERVED_BIT_0 | COLFLAGS_RESERVED_BIT_1 | COLFLAGS_RESERVED_BIT_2,
	COLFLAGS_SHOW_MULTIPLIER        = 4_bit, // Показывать суффиксы B,K,M,G,T,P,E

	COLFLAGS_MARK                   = 5_bit,
	COLFLAGS_NAMEONLY               = 6_bit,
	COLFLAGS_RIGHTALIGN             = 7_bit,
	COLFLAGS_GROUPDIGITS            = 8_bit, // Вставлять разделитель между тысячами
	COLFLAGS_THOUSAND               = 9_bit, // Вместо делителя 1024 использовать делитель 1000
	COLFLAGS_BRIEF                  = 10_bit,
	COLFLAGS_MONTH                  = 11_bit,
	COLFLAGS_FLOATSIZE              = 12_bit, // Показывать размер в виде десятичной дроби, используя наиболее подходящую единицу измерения, например 0,97 К, 1,44 М, 53,2 Г.
	COLFLAGS_ECONOMIC               = 13_bit, // Экономичный режим, не показывать пробел перед суффиксом размера файла (т.е. 0.97K)
	COLFLAGS_FULLOWNER              = 14_bit,
	COLFLAGS_NOEXTENSION            = 15_bit,
	COLFLAGS_RIGHTALIGNFORCE        = 16_bit,
	COLFLAGS_MARK_DYNAMIC           = 17_bit,
};

enum col_width
{
	fixed,
	percent,
};

#endif // PANELCTYPE_HPP_C90AC741_F7E6_44D6_92BA_B4C0047E9E19
