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


enum PANEL_COLUMN_TYPE {
	NAME_COLUMN=0,
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
	CUSTOM_COLUMN1,
	CUSTOM_COLUMN2,
	CUSTOM_COLUMN3,
	CUSTOM_COLUMN4,
	CUSTOM_COLUMN5,
	CUSTOM_COLUMN6,
	CUSTOM_COLUMN7,
	CUSTOM_COLUMN8,
	CUSTOM_COLUMN9,
};

enum {
	COUNT_WIDTH=0,
	PERCENT_WIDTH
};

#define PANEL_COLUMNCOUNT  20
