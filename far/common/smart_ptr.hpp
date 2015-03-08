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

template<class T>
class array_ptr
{
public:
	array_ptr() : m_size() {}
	array_ptr(array_ptr&& other) noexcept: m_size() { *this = std::move(other); }
	array_ptr(size_t size, bool init = false) : m_array(init? new T[size]() : new T[size]), m_size(size) {}
	MOVE_OPERATOR_BY_SWAP(array_ptr);
	void reset(size_t size, bool init = false) { m_array.reset(init? new T[size]() : new T[size]); m_size = size;}
	void reset() { m_array.reset(); m_size = 0; }
	void swap(array_ptr& other) noexcept { using std::swap; m_array.swap(other.m_array); swap(m_size, other.m_size); }
	FREE_SWAP(array_ptr);
	size_t size() const {return m_size;}
	operator bool() const { return get() != nullptr; }
	T* get() const {return m_array.get();}
	T* operator->() const { return get(); }
	T& operator*() const { return *get(); }
	T& operator[](size_t n) const { return get()[n]; }
private:
	std::unique_ptr<T[]> m_array;
	size_t m_size;
};

typedef array_ptr<wchar_t> wchar_t_ptr;
typedef array_ptr<char> char_ptr;

template<class T>
class block_ptr:public char_ptr
{
public:
	block_ptr(){}
	block_ptr(block_ptr&& Right){char_ptr::swap(Right);}
	block_ptr(size_t size, bool init = false):char_ptr(size, init){}
	MOVE_OPERATOR_BY_SWAP(block_ptr);
	T* get() const {return reinterpret_cast<T*>(char_ptr::get());}
	T* operator->() const {return get();}
	T& operator*() const {return *get();}
};

struct file_closer
{
	void operator()(FILE* Object) const
	{
		fclose(Object);
	}
};

typedef std::unique_ptr<FILE, file_closer> file_ptr;
