/*
TPreRedrawFunc.cpp

Фоновый апдейт

*/

#include "headers.hpp"
#pragma hdrstop

#include "TPreRedrawFunc.hpp"

TPreRedrawFunc PreRedraw;

PreRedrawItem TPreRedrawFunc::errorStack={0};

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
		memmove(&Top->Item.Param,&Param,sizeof(PreRedrawParamStruct));
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
		memmove(&Source.Param,Param,sizeof(PreRedrawParamStruct));
	else
		memset(&Source.Param,0,sizeof(PreRedrawParamStruct));

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
