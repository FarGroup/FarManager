#pragma once

/*
message.hpp

Вывод MessageBox
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

#define ADDSPACEFORPSTRFORMESSAGE 16

enum
{
	MSG_WARNING        =0x00000001,
	MSG_ERRORTYPE      =0x00000002,
	MSG_KEEPBACKGROUND =0x00000004,
	MSG_LEFTALIGN      =0x00000008,
	MSG_INSERT_STR1    =0x00000100,
	MSG_INSERT_STR2    =0x00000200,
	MSG_INSERT_STR3    =0x00000400,
	MSG_INSERT_STR4    =0x00000800,
	MSG_INSERT_STR5    =0x00001000,
	MSG_INSERT_STR6    =0x00002000,
	MSG_INSERT_STR7    =0x00004000,
	MSG_INSERT_STR8    =0x00008000,
	MSG_INSERT_STR9    =0x00010000,
	MSG_INSERT_STR10   =0x00020000,
	MSG_INSERT_STR11   =0x00040000,
	MSG_INSERT_STR12   =0x00080000,
	MSG_INSERT_STR13   =0x00100000,
	MSG_INSERT_STR14   =0x00200000,
	MSG_INSERT_STRINGS =0x003fff00,
	MSG_KILLSAVESCREEN =0x10000000,
	MSG_NOPLUGINS      =0x20000000,
};
#define MSG_INSERT_STR(n) (((n) > 0 && (n) < 15) ? (MSG_INSERT_STR1 << ((n)-1)) : 0)

class Plugin;

int Message(DWORD Flags,size_t Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2=nullptr,const wchar_t *Str3=nullptr,const wchar_t *Str4=nullptr,
            Plugin* PluginNumber=nullptr);
int Message(DWORD Flags,size_t Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6=nullptr,const wchar_t *Str7=nullptr,
            Plugin* PluginNumber=nullptr);
int Message(DWORD Flags,size_t Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6,const wchar_t *Str7,
            const wchar_t *Str8,const wchar_t *Str9=nullptr,const wchar_t *Str10=nullptr,
            Plugin* PluginNumber=nullptr);
int Message(DWORD Flags,size_t Buttons,const wchar_t *Title,const wchar_t *Str1,
            const wchar_t *Str2,const wchar_t *Str3,const wchar_t *Str4,
            const wchar_t *Str5,const wchar_t *Str6,const wchar_t *Str7,
            const wchar_t *Str8,const wchar_t *Str9,const wchar_t *Str10,
            const wchar_t *Str11,const wchar_t *Str12=nullptr,const wchar_t *Str13=nullptr,
            const wchar_t *Str14=nullptr, Plugin* PluginNumber=nullptr);

int Message(DWORD Flags,size_t Buttons,const wchar_t *Title,const wchar_t * const *Items,
            size_t ItemsNumber, Plugin* PluginNumber=nullptr,const GUID* Id=nullptr);

void SetMessageHelp(const wchar_t *Topic);
void GetMessagePosition(int &X1,int &Y1,int &X2,int &Y2);

/* $ 12.03.2002 VVM
  Новая функция - пользователь попытался прервать операцию.
  Зададим вопрос.
  Возвращает:
   FALSE - продолжить операцию
   TRUE  - прервать операцию
*/
int AbortMessage();

bool GetErrorString(string &strErrStr);
