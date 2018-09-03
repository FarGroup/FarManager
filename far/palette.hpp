﻿#ifndef PALETTE_HPP_8CFE8272_39B6_4198_9046_E94FEAD9832C
#define PALETTE_HPP_8CFE8272_39B6_4198_9046_E94FEAD9832C
#pragma once

/*
palette.hpp

Таблица цветов
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

struct FarColor;

class palette: noncopyable
{
public:
	palette();
	void Load();
	void Save(bool always);
	void ResetToDefault();
	void ResetToBlack();
	void Set(size_t StartOffset, FarColor* Value, size_t Count);
	void CopyTo(FarColor* Destination, size_t Size) const;
	const FarColor& operator[](size_t Index) const {return CurrentPalette[Index];}
	size_t size() const {return CurrentPalette.size();}

private:
	void Reset(bool Black);
	std::vector<FarColor> CurrentPalette;
	bool PaletteChanged;
};

#endif // PALETTE_HPP_8CFE8272_39B6_4198_9046_E94FEAD9832C
