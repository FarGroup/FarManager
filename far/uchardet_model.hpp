// validator: no-include-guards
/*
uchardet_model.hpp
*/
/*
Copyright © 2025 Far Group
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

#define Cp737_CharToOrderMap                CONCATENATE(UCHARDET_LANGUAGE, _Cp737_CharToOrderMap)
#define Georgian_Academy_CharToOrderMap     CONCATENATE(UCHARDET_LANGUAGE, _Georgian_Academy_CharToOrderMap)
#define Georgian_Ps_CharToOrderMap          CONCATENATE(UCHARDET_LANGUAGE, _Georgian_Ps_CharToOrderMap)
#define Ibm852_CharToOrderMap               CONCATENATE(UCHARDET_LANGUAGE, _Ibm852_CharToOrderMap)
#define Ibm855_CharToOrderMap               CONCATENATE(UCHARDET_LANGUAGE, _Ibm855_CharToOrderMap)
#define Ibm862_CharToOrderMap               CONCATENATE(UCHARDET_LANGUAGE, _Ibm862_CharToOrderMap)
#define Ibm865_CharToOrderMap               CONCATENATE(UCHARDET_LANGUAGE, _Ibm865_CharToOrderMap)
#define Ibm866_CharToOrderMap               CONCATENATE(UCHARDET_LANGUAGE, _Ibm866_CharToOrderMap)
#define Iso_8859_1_CharToOrderMap           CONCATENATE(UCHARDET_LANGUAGE, _Iso_8859_1_CharToOrderMap)
#define Iso_8859_10_CharToOrderMap          CONCATENATE(UCHARDET_LANGUAGE, _Iso_8859_10_CharToOrderMap)
#define Iso_8859_11_CharToOrderMap          CONCATENATE(UCHARDET_LANGUAGE, _Iso_8859_11_CharToOrderMap)
#define Iso_8859_13_CharToOrderMap          CONCATENATE(UCHARDET_LANGUAGE, _Iso_8859_13_CharToOrderMap)
#define Iso_8859_15_CharToOrderMap          CONCATENATE(UCHARDET_LANGUAGE, _Iso_8859_15_CharToOrderMap)
#define Iso_8859_16_CharToOrderMap          CONCATENATE(UCHARDET_LANGUAGE, _Iso_8859_16_CharToOrderMap)
#define Iso_8859_2_CharToOrderMap           CONCATENATE(UCHARDET_LANGUAGE, _Iso_8859_2_CharToOrderMap)
#define Iso_8859_3_CharToOrderMap           CONCATENATE(UCHARDET_LANGUAGE, _Iso_8859_3_CharToOrderMap)
#define Iso_8859_4_CharToOrderMap           CONCATENATE(UCHARDET_LANGUAGE, _Iso_8859_4_CharToOrderMap)
#define Iso_8859_5_CharToOrderMap           CONCATENATE(UCHARDET_LANGUAGE, _Iso_8859_5_CharToOrderMap)
#define Iso_8859_6_CharToOrderMap           CONCATENATE(UCHARDET_LANGUAGE, _Iso_8859_6_CharToOrderMap)
#define Iso_8859_7_CharToOrderMap           CONCATENATE(UCHARDET_LANGUAGE, _Iso_8859_7_CharToOrderMap)
#define Iso_8859_8_CharToOrderMap           CONCATENATE(UCHARDET_LANGUAGE, _Iso_8859_8_CharToOrderMap)
#define Iso_8859_9_CharToOrderMap           CONCATENATE(UCHARDET_LANGUAGE, _Iso_8859_9_CharToOrderMap)
#define Koi8_R_CharToOrderMap               CONCATENATE(UCHARDET_LANGUAGE, _Koi8_R_CharToOrderMap)
#define Mac_Centraleurope_CharToOrderMap    CONCATENATE(UCHARDET_LANGUAGE, _Mac_Centraleurope_CharToOrderMap)
#define Mac_Cyrillic_CharToOrderMap         CONCATENATE(UCHARDET_LANGUAGE, _Mac_Cyrillic_CharToOrderMap)
#define Tis_620_CharToOrderMap              CONCATENATE(UCHARDET_LANGUAGE, _Tis_620_CharToOrderMap)
#define Viscii_CharToOrderMap               CONCATENATE(UCHARDET_LANGUAGE, _Viscii_CharToOrderMap)
#define Windows_1250_CharToOrderMap         CONCATENATE(UCHARDET_LANGUAGE, _Windows_1250_CharToOrderMap)
#define Windows_1251_CharToOrderMap         CONCATENATE(UCHARDET_LANGUAGE, _Windows_1251_CharToOrderMap)
#define Windows_1252_CharToOrderMap         CONCATENATE(UCHARDET_LANGUAGE, _Windows_1252_CharToOrderMap)
#define Windows_1253_CharToOrderMap         CONCATENATE(UCHARDET_LANGUAGE, _Windows_1253_CharToOrderMap)
#define Windows_1255_CharToOrderMap         CONCATENATE(UCHARDET_LANGUAGE, _Windows_1255_CharToOrderMap)
#define Windows_1256_CharToOrderMap         CONCATENATE(UCHARDET_LANGUAGE, _Windows_1256_CharToOrderMap)
#define Windows_1257_CharToOrderMap         CONCATENATE(UCHARDET_LANGUAGE, _Windows_1257_CharToOrderMap)
#define Windows_1258_CharToOrderMap         CONCATENATE(UCHARDET_LANGUAGE, _Windows_1258_CharToOrderMap)
#define Unicode_CharOrder                   CONCATENATE(UCHARDET_LANGUAGE, _Unicode_CharOrder)
#define Unicode_Char_size                   CONCATENATE(UCHARDET_LANGUAGE, _Unicode_Char_size)

#define UCHARDET_MODEL_NAME(NAME) CONCATENATE(CONCATENATE(thirdparty/uchardet/LangModels/Lang, NAME), Model.cpp)

#include EXPAND_TO_LITERAL(UCHARDET_MODEL_NAME(UCHARDET_LANGUAGE))

#undef UCHARDET_MODEL_NAME

#undef Unicode_Char_size
#undef Unicode_CharOrder

#undef UCHARDET_LANGUAGE
