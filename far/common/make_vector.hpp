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

template<class T, class Y>
T emplace_back(T&& container, Y value) { container.emplace_back(value); return std::move(container); }

#if defined _MSC_VER && _MSC_VER < 1800
template<class T> std::vector<T> make_vector(T a1)
{ return emplace_back(std::vector<T>(), a1); }

template<class T> std::vector<T> make_vector(T a1, T a2)
{ return emplace_back(make_vector(a1), a2); }

template<class T> std::vector<T> make_vector(T a1, T a2, T a3)
{ return emplace_back(make_vector(a1, a2), a3); }

template<class T> std::vector<T> make_vector(T a1, T a2, T a3, T a4)
{ return emplace_back(make_vector(a1, a2, a3), a4); }

template<class T> std::vector<T> make_vector(T a1, T a2, T a3, T a4, T a5)
{ return emplace_back(make_vector(a1, a2, a3, a4), a5); }

template<class T> std::vector<T> make_vector(T a1, T a2, T a3, T a4, T a5, T a6)
{ return emplace_back(make_vector(a1, a2, a3, a4, a5), a6); }

template<class T> std::vector<T> make_vector(T a1, T a2, T a3, T a4, T a5, T a6, T a7)
{ return emplace_back(make_vector(a1, a2, a3, a4, a5, a6), a7); }

template<class T> std::vector<T> make_vector(T a1, T a2, T a3, T a4, T a5, T a6, T a7, T a8)
{ return emplace_back(make_vector(a1, a2, a3, a4, a5, a6, a7), a8); }

template<class T> std::vector<T> make_vector(T a1, T a2, T a3, T a4, T a5, T a6, T a7, T a8, T a9)
{ return emplace_back(make_vector(a1, a2, a3, a4, a5, a6, a7, a8), a9); }
#else
template<class T, class Y, class... Args>
T emplace_back(T&& container, Y value, Args... args)
{
	container.emplace_back(value);
	return emplace_back(std::move(container), args...);
}

template<class T, class... Args> std::vector<T> make_vector(T value, Args... args)
{
	return emplace_back(std::vector<T>(), value, args...);
}
#endif
