/*
vc10.cpp

Workaround for VC2010 and old Windows
*/
/*
Copyright © 2011 Far Group
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

static PVOID WINAPI ReturnSamePointer(PVOID Ptr) {return Ptr;}

static const char* ProcNames[] = {"EncodePointer", "DecodePointer"};
static enum {EncodePointerIndex, DecodePointerIndex};

template<int Index>
static PVOID WINAPI Wrapper(PVOID Ptr)
{
	typedef PVOID (WINAPI *PointerFunction)(PVOID);
	static PVOID FunctionAddress = GetProcAddress(GetModuleHandle(L"kernel32"), ProcNames[Index]);
	static PointerFunction ProcessPointer = FunctionAddress? reinterpret_cast<PointerFunction>(FunctionAddress) : ReturnSamePointer;
	return ProcessPointer(Ptr);
}

extern "C"
{
	PVOID WINAPI EncodePointerWrapper(PVOID Ptr) {return Wrapper<EncodePointerIndex>(Ptr);}
	PVOID WINAPI DecodePointerWrapper(PVOID Ptr) {return Wrapper<DecodePointerIndex>(Ptr);}
}
