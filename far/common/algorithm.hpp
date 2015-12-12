#ifndef ALGORITHM_HPP_BBD588C0_4752_46B2_AAB9_65450622FFF0
#define ALGORITHM_HPP_BBD588C0_4752_46B2_AAB9_65450622FFF0
#pragma once

/*
Copyright © 2014 Far Group
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

// for_each with embedded counter
template<class I, class F>
inline F for_each_cnt(I First, I Last, F Func)
{
	for (size_t Cnt = 0; First != Last; ++First, ++Cnt)
	{
		Func(*First, Cnt);
	}
	return Func;
}

// for_each for 2 containers
template<class A, class B, class F>
inline F for_each_2(A FirstA, A LastA, B FirstB, F Func)
{
	for (; FirstA != LastA; ++FirstA, ++FirstB)
	{
		Func(*FirstA, *FirstB);
	}
	return Func;
}

template<class T>
inline void repeat(size_t count, const T& f)
{
	for(size_t i = 0; i != count; ++i)
	{
		f();
	}
}

template <class I, class T, class P>
void fill_if(I First, I Last, const T& Value, P Predicate)
{
	while (First != Last)
	{
		if (Predicate(*First))
			*First = Value;
		++First;
	}
}

template <class I, class N, class T, class P>
I fill_n_if(I First, N Size, const T& Value, P Predicate)
{
	while (Size > 0)
	{
		if (Predicate(*First))
			*First = Value;
		++First;
		--Size;
	}
	return First;
}

template<class T, class P>
void for_submatrix(T& Matrix, size_t X1, size_t Y1, size_t X2, size_t Y2, P Predicate)
{
	for (auto i = Y1; i <= Y2; ++i)
	{
		for (auto j = X1; j <= X2; ++j)
		{
			Predicate(Matrix[i][j]);
		}
	}
}
#endif // ALGORITHM_HPP_BBD588C0_4752_46B2_AAB9_65450622FFF0
