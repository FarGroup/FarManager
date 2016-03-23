#ifndef MESSAGE_HPP_640AC104_875B_41AE_8EF5_8A99913A6896
#define MESSAGE_HPP_640AC104_875B_41AE_8EF5_8A99913A6896
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

enum
{
	MSG_WARNING        =0x00000001,
	MSG_ERRORTYPE      =0x00000002,
	MSG_KEEPBACKGROUND =0x00000004,
	MSG_LEFTALIGN      =0x00000008,
	MSG_KILLSAVESCREEN =0x10000000,
	MSG_NOPLUGINS      =0x20000000,
};

class Plugin;
class Dialog;

class Message: noncopyable
{
public:
	Message(
		DWORD Flags,
		const string& Title,
		const std::vector<string>& Strings,
		const std::vector<string>& Buttons,
		const wchar_t* HelpTopic = nullptr,
		Plugin* PluginNumber = nullptr,
		const GUID* Id = nullptr,
		const std::vector<string>& Inserts = std::vector<string>()
	);

	Message(DWORD Flags,size_t Buttons,const string& Title, const wchar_t *Str1,
		const wchar_t *Str2=nullptr, const wchar_t *Str3=nullptr, const wchar_t *Str4=nullptr, const wchar_t *Str5=nullptr,
		const wchar_t *Str6=nullptr, const wchar_t *Str7=nullptr, const wchar_t *Str8=nullptr, const wchar_t *Str9=nullptr,
		const wchar_t *Str10=nullptr, const wchar_t *Str11=nullptr, const wchar_t *Str12=nullptr);

	enum
	{
		first_button,
		second_button,
		third_button,
	};

	int GetExitCode() const {return m_ExitCode;}
	void GetMessagePosition(int &X1,int &Y1,int &X2,int &Y2) const;
	operator int() const { return GetExitCode(); }

private:
	void Init(
		DWORD Flags,
		const string& Title,
		const std::vector<string>& Strings,
		const std::vector<string>& Buttons,
		const std::vector<string>& Inserts,
		const wchar_t* HelpTopic,
		Plugin* PluginNumber,
		const GUID* Id
	);
	intptr_t MsgDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2);

	int m_ExitCode;
	int MessageX1,MessageY1,MessageX2,MessageY2;
	int FirstButtonIndex,LastButtonIndex;
	bool IsWarningStyle;
	bool IsErrorType;
};

/* $ 12.03.2002 VVM
  Новая функция - пользователь попытался прервать операцию.
  Зададим вопрос.
  Возвращает:
   FALSE - продолжить операцию
   TRUE  - прервать операцию
*/
bool AbortMessage();

string GetErrorString();

#endif // MESSAGE_HPP_640AC104_875B_41AE_8EF5_8A99913A6896
