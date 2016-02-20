#ifndef FAREXCPT_HPP_F7B85E85_71DD_483D_BD7F_B26B8566AC8E
#define FAREXCPT_HPP_F7B85E85_71DD_483D_BD7F_B26B8566AC8E
#pragma once

/*
exception.cpp

Все про исключения
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

class FarException : public std::runtime_error
{
public:
	FarException(const char* Message) : std::runtime_error(Message) {}
};

class FarRecoverableException : public FarException
{
public:
	FarRecoverableException(const char* Message) : FarException(Message) {}
};

class Plugin;

bool ProcessSEHException(EXCEPTION_POINTERS *xp, const wchar_t* Function, Plugin *Module = nullptr);
bool ProcessStdException(const std::exception& e, const wchar_t* Function, Plugin* Module = nullptr);

LONG WINAPI FarUnhandledExceptionFilter(EXCEPTION_POINTERS *ExceptionInfo);

void RestoreGPFaultUI();

class SException: public std::exception
{
public:
	SException(int Code, EXCEPTION_POINTERS* Info):m_Code(Code), m_Info(Info) {}
	int GetCode() const { return m_Code; }
	EXCEPTION_POINTERS* GetInfo() const { return m_Info; }

private:
	int m_Code;
	EXCEPTION_POINTERS* m_Info;
};

class veh_handler: noncopyable
{
public:
	veh_handler(PVECTORED_EXCEPTION_HANDLER Handler);
	~veh_handler();

private:
	void* m_Handler;
};

LONG WINAPI VectoredExceptionHandler(EXCEPTION_POINTERS *xp);

void EnableSeTranslation();
void attach_debugger();

void RegisterTestExceptionsHook();

bool IsCppException(const EXCEPTION_POINTERS* e);

#endif // FAREXCPT_HPP_F7B85E85_71DD_483D_BD7F_B26B8566AC8E
