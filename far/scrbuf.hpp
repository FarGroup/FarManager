#ifndef SCRBUF_HPP_9C71BBDC_FEF1_40D7_895F_7DEB31F6FBAC
#define SCRBUF_HPP_9C71BBDC_FEF1_40D7_895F_7DEB31F6FBAC
#pragma once

/*
scrbuf.hpp

Буферизация вывода на экран, весь вывод идет через этот буфер
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

#include "bitflags.hpp"
#include "synchro.hpp"
#include "matrix.hpp"

enum class flush_type
{
	screen = bit(0),
	cursor = bit(1),
	title  = bit(2),

	all = screen | cursor | title
};

inline constexpr auto operator|(flush_type a, flush_type b)
{
	return enum_helpers::operation<std::bit_or<>>(a, b);
}

inline constexpr auto operator&(flush_type a, flush_type b)
{
	return enum_helpers::operation<std::bit_and<>, std::underlying_type_t<flush_type>>(a, b);
}

class ScreenBuf: noncopyable
{
public:
	ScreenBuf();

	void DebugDump() const;

	void AllocBuf(size_t rows, size_t cols);
	void Lock();
	void Unlock();
	int  GetLockCount() const {return LockCount;}
	void SetLockCount(int Count);
	void ResetLockCount() {LockCount=0;}
	void MoveCursor(int X,int Y);
	void GetCursorPos(SHORT& X, SHORT& Y) const;
	void SetCursorType(bool Visible, DWORD Size);
	void GetCursorType(bool& Visible, DWORD& Size) const;
	const string& GetTitle() const { return m_Title; }
	void SetTitle(const string& Title);

	void FillBuf();
	void Read(int X1, int Y1, int X2, int Y2, matrix<FAR_CHAR_INFO>& Dest);
	void Write(int X,int Y,const FAR_CHAR_INFO *Text, size_t Size);
	void RestoreMacroChar();
	void RestoreElevationChar();

	void ApplyShadow(int X1,int Y1,int X2,int Y2);
	void ApplyColor(int X1,int Y1,int X2,int Y2,const FarColor& Color, bool PreserveExFlags = false);
	void ApplyColor(int X1,int Y1,int X2,int Y2,const FarColor& Color,const FarColor& ExceptColor, bool ForceExFlags = false);
	void FillRect(int X1,int Y1,int X2,int Y2,WCHAR Ch,const FarColor& Color);

	void Scroll(size_t Count);
	void Flush(flush_type FlushType = flush_type::all);

private:
	CriticalSection CS;
	FAR_CHAR_INFO MacroChar;
	FAR_CHAR_INFO ElevationChar;
	matrix<FAR_CHAR_INFO> Buf;
	matrix<FAR_CHAR_INFO> Shadow;
	string m_Title;
	BitFlags SBFlags;
	int LockCount;
	DWORD CurSize;
	SHORT CurX, CurY;
	bool MacroCharUsed;
	bool ElevationCharUsed;
	bool CurVisible;
};

#endif // SCRBUF_HPP_9C71BBDC_FEF1_40D7_895F_7DEB31F6FBAC
