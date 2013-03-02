#pragma once

/*
TPreRedrawFunc.hpp

Фоновый апдейт

*/
/*
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


struct PreRedrawItem
{
	typedef void (*PREREDRAWFUNC)();

	PreRedrawItem(PREREDRAWFUNC PreRedrawFunc):
		PreRedrawFunc(PreRedrawFunc)
	{
		ClearStruct(Param);
	}

	PREREDRAWFUNC PreRedrawFunc;
	struct
	{
		DWORD Flags;
		const void *Param1;
		const void *Param2;
		const void *Param3;
		const void *Param4;
		__int64 Param5;
	}
	Param;
};

class TPreRedrawFunc
{
public:
	void push(const PreRedrawItem &Source){return Items.push(Source);}
	void pop() {return Items.pop();}
	PreRedrawItem& top() {return Items.top();}
	bool empty() const {return Items.empty();}
private:
	std::stack<PreRedrawItem> Items;
};

class TPreRedrawFuncGuard
{
public:
	TPreRedrawFuncGuard(PreRedrawItem::PREREDRAWFUNC Func)
	{
		Global->PreRedraw->push(Func);
	}
	~TPreRedrawFuncGuard()
	{
		Global->PreRedraw->pop();
	}
};
