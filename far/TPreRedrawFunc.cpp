/*
TPreRedrawFunc.cpp

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


#include "headers.hpp"
#pragma hdrstop

#include "TPreRedrawFunc.hpp"

TPreRedrawFunc PreRedraw;

PreRedrawItem TPreRedrawFunc::errorStack={};

PreRedrawItem TPreRedrawFunc::Pop()
{
	if (Top)
	{
		--Total;
		PreRedrawItem Destination=Top->Item;
		current=Top->Next;
		delete Top;
		Top=current;
		return Destination;
	}

	return TPreRedrawFunc::errorStack;
}

PreRedrawItem TPreRedrawFunc::Peek()
{
	if (Top)
		return Top->Item;

	return TPreRedrawFunc::errorStack;
}

PreRedrawItem TPreRedrawFunc::SetParam(PreRedrawParamStruct Param)
{
	if (Top)
	{
		Top->Item.Param=Param;
		return Top->Item;
	}

	return TPreRedrawFunc::errorStack;
}

PreRedrawItem TPreRedrawFunc::Push(const PreRedrawItem &Source)
{
	current=new OneItem(Source,Top);

	if (current)
	{
		Top=current;
		++Total;
		return Top->Item;
	}

	return TPreRedrawFunc::errorStack;
}

PreRedrawItem TPreRedrawFunc::Push(PREREDRAWFUNC Func,PreRedrawParamStruct *Param)
{
	PreRedrawItem Source;
	Source.PreRedrawFunc=Func;

	if (Param)
		Source.Param=*Param;
	else
		ClearStruct(Source.Param);

	return Push(Source);
}

void TPreRedrawFunc::Free()
{
	while (Top)
	{
		current=Top->Next;
		delete Top;
		Top=current;
	}

	Total=0;
}


TPreRedrawFuncGuard::TPreRedrawFuncGuard(PREREDRAWFUNC Func)
{
	PreRedraw.Push(Func);
}

TPreRedrawFuncGuard::~TPreRedrawFuncGuard()
{
	PreRedraw.Pop();
}
