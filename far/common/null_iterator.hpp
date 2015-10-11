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

template <class T>
class null_iterator_t: public std::iterator<std::forward_iterator_tag, T>
{
public:
	null_iterator_t(T* Data): m_Data(Data) {}
	null_iterator_t& operator++() { ++m_Data; return *this; }
	null_iterator_t operator++(int) { return null_iterator_t(m_Data++); }
	T& operator*() { return *m_Data; }
	T* operator->() noexcept { return m_Data; }
	const T& operator*() const { return *m_Data; }
	const T* operator->() const noexcept { return m_Data; }
	static const null_iterator_t& end() { static T Empty[1] = {}; static null_iterator_t Iter(Empty); return Iter; }
	bool operator==(const null_iterator_t& rhs) const { return (!*m_Data && !*rhs.m_Data) || m_Data == rhs.m_Data; }
	bool operator!=(const null_iterator_t& rhs) const { return !(*this == rhs); }

private:
	T* m_Data;
};

template <class T>
null_iterator_t<T> null_iterator(T* Data) { return null_iterator_t<T>(Data); }
