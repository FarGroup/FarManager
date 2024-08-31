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

// Internal:
#include "bitflags.hpp"
#include "plugin.hpp"

// Platform:
#include "platform.concurrency.hpp"

// Common:
#include "common/2d/matrix.hpp"
#include "common/2d/point.hpp"
#include "common/2d/rectangle.hpp"

// External:

//----------------------------------------------------------------------------

enum class flush_type
{
	screen = 0_bit,
	cursor = 1_bit,
	title  = 2_bit,

	all = screen | cursor | title,

	is_bit_flags
};

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
	void MoveCursor(point Point);
	point GetCursorPos() const;
	void SetCursorType(bool Visible, size_t Size);
	void GetCursorType(bool& Visible, size_t& Size) const;
	const string& GetTitle() const { return m_Title; }
	void SetTitle(string_view Title);

	void FillBuf();
	void Read(rectangle Where, matrix<FAR_CHAR_INFO>& Dest);
	void Write(int X,int Y, std::span<const FAR_CHAR_INFO> Text);
	void RestoreMacroChar();
	void RestoreElevationChar();

	void ApplyShadow(rectangle Where);
	void ApplyColor(rectangle Where, const FarColor& Color);
	void FillRect(rectangle Where, const FAR_CHAR_INFO& Info);

	void Invalidate(flush_type FlushType = flush_type::all);
	void Flush(flush_type FlushType = flush_type::all);

	void SetClearTypeFix(int ClearTypeFix);

private:
	void debug_flush();

	os::critical_section CS;
	FAR_CHAR_INFO MacroChar{};
	FAR_CHAR_INFO ElevationChar{};
	matrix<FAR_CHAR_INFO> Buf;
	matrix<FAR_CHAR_INFO> Shadow;
	string m_Title;
	BitFlags SBFlags;
	int LockCount{};
	int m_ClearTypeFix{};
	size_t CurSize{};
	point m_CurPos{};
	bool MacroCharUsed{};
	bool ElevationCharUsed{};
	bool CurVisible{};
};

#endif // SCRBUF_HPP_9C71BBDC_FEF1_40D7_895F_7DEB31F6FBAC
