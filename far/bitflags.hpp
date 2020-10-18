#ifndef BITFLAGS_HPP_0F0A29FB_5389_4343_9C50_A84459FAF9D2
#define BITFLAGS_HPP_0F0A29FB_5389_4343_9C50_A84459FAF9D2
#pragma once

/*
bitflags.hpp

Флаги
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

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

template<class T>
class TBitFlags
{
public:
	TBitFlags():m_Flags(0) {}
	explicit TBitFlags(const T& Flags):m_Flags(Flags) {}

	const T& Flags() const { return m_Flags; }
	// установить набор флагов
	const T& Set(const T& FlagsToSet) { m_Flags |= FlagsToSet; return m_Flags;}
	// сбросить набор флагов
	const T& Clear(const T& FlagsToClear) { m_Flags &=~ FlagsToClear; return m_Flags; }
	// проверить набор флагов
	bool CheckAny(const T& FlagsToCheck) const { return (m_Flags & FlagsToCheck) != 0; }
	bool CheckAll(const T& FlagsToCheck) const { return (m_Flags & FlagsToCheck) == FlagsToCheck; }
	// BUGBUG remove this
	bool Check(const T& FlagsToCheck) const { return CheckAny(FlagsToCheck); }
	// изменить состояние набора флагов в заивисмости от Status
	const T& Change(const T& FlagsToChange, bool set) { return set? Set(FlagsToChange) : Clear(FlagsToChange); }
	// инвертировать состояние флагов
	const T& Invert(const T& FlagsToSwap) { return Check(FlagsToSwap)? Clear(FlagsToSwap) : Set(FlagsToSwap); }
	//сбросить все флаги
	void ClearAll() { m_Flags = 0; }
private:
	T m_Flags;
};

using BitFlags = TBitFlags<DWORD>;

#endif // BITFLAGS_HPP_0F0A29FB_5389_4343_9C50_A84459FAF9D2
