#pragma once

/*
macrocompiler.hpp

Реализация парсера для MacroDrive II

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

#ifdef FAR_LUA
#else
//---------------------------------------------------------------
// If this code works, it was written by Alexander Nazarenko.
// If not, I don't know who wrote it.
//---------------------------------------------------------------

struct TMacroKeywords
{
	int Type;              // Тип: 0=Area, 1=Flags, 2=Condition
	const wchar_t *Name;   // Наименование
	DWORD Value;           // Значение
	DWORD Reserved;
};

// в plugin.hpp это FARMACROPARSEERRORCODE
enum errParseCode
{
	err_Success = 0,
	err_Unrecognized_keyword = 1,
	err_Unrecognized_function = 2,
	err_Func_Param = 3,
	err_Not_expected_ELSE = 4,
	err_Not_expected_END = 5,
	err_Unexpected_EOS = 6,
	err_Expected_Token = 7,
	err_Bad_Hex_Control_Char = 8,
	err_Bad_Control_Char = 9,
	err_Var_Expected = 10,
	err_Expr_Expected = 11,
	err_ZeroLengthMacro = 12,
	err_IntParserError = 13,
	err_Continue_Outside_The_Loop=14,
	err_Break_Outside_The_Loop=15,
};

extern int MKeywordsSize;
extern struct TMacroKeywords MKeywords[];
extern int MKeywordsFlagsSize;
extern struct TMacroKeywords MKeywordsFlags[];

int __parseMacroString(DWORD *&CurMacroBuffer, int &CurMacroBufferSize, const wchar_t *BufPtr);
BOOL __getMacroParseError(DWORD* ErrCode, COORD* ErrPos, string *ErrSrc);
BOOL __getMacroParseError(string* Err1, string* Err2, string* Err3, string* Err4);
int  __getMacroErrorCode(int *nErr=nullptr);
void  __setMacroErrorCode(int ErrCode);
#endif
