/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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

#define __NEW_H
#include "headers.hpp"
#pragma hdrstop

// maxlen - максимальное число символов, которое может содержать
//          dest БЕЗ учета заключительного нуля, т.е. в общем
//          случае это "sizeof-1"

char * __cdecl xstrncat(char * dest,const char * src, size_t maxlen)
{
	char * start=dest;
	while(*dest)
	{
		dest++;
		maxlen--;
	}
	while(maxlen--)
		if(!(*dest++=*src++))
			return start;
	*dest=0;
	return start;
}

wchar_t * __cdecl xwcsncat(wchar_t * dest,const wchar_t * src, size_t maxlen)
{
	wchar_t * start=dest;
	while(*dest)
	{
		dest++;
		maxlen--;
	}
	while(maxlen--)
		if(!(*dest++=*src++))
			return start;
	*dest=0;
	return start;
}
