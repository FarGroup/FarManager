#pragma once

/*
library_extensions.hpp

Some useful STL-based templates && macros
*/
/*
Copyright © 2013 Far Group
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

#ifdef __GNUC__
# if _GCC_VER < GCC_VER_(4,6,1)
#  error gcc 4.6.1 (or higher) required
# endif
# define ENUM(ENUM_NAME) enum ENUM_NAME:int
#endif

#ifdef _MSC_VER
# if _MSC_VER>1600
#  define ENUM(ENUM_NAME) enum ENUM_NAME:int
# else
#  define ENUM(ENUM_NAME) enum ENUM_NAME
# endif
#endif

#ifndef CPP_STD_14 //?
// const, reverse and const-reverse versions of std::begin and std::end are missed in C++11,
// possibly they will be added in C++14, until then they are manually defined here:
namespace std
{
	template<class Container>
	inline auto cbegin(const Container& c) -> decltype(c.cbegin()) {return c.cbegin();}
	template<class Container>
	inline auto cend(const Container& c) -> decltype(c.cend()) {return c.cend();}
	
	template<class Type, size_t Size>
	inline const Type* cbegin(const Type (&a)[Size]) {return a;}
	template<class Type, size_t Size>
	inline const Type* cend(const Type (&a)[Size]) {return a + Size;}
	
	template<class Container>
	inline auto rbegin(Container& c) -> decltype(c.rbegin()) {return c.rbegin();}
	template<class Container>
	inline auto rend(Container& c) -> decltype(c.rend()) {return c.rend();}

	template<class PtrType>
	class array_reverse_iterator
	{
	public:
		array_reverse_iterator(PtrType value = nullptr) {this->value = value;}
		PtrType operator ->() const {return value;}
		typename std::remove_pointer<PtrType>::type& operator *() const {return *value;}
		bool operator <(const array_reverse_iterator& rhs) const {return value > rhs.value;}
		bool operator >(const array_reverse_iterator& rhs) const {return value < rhs.value;}
		bool operator <=(const array_reverse_iterator& rhs) const {return value >= rhs.value;}
		bool operator >=(const array_reverse_iterator& rhs) const {return value <= rhs.value;}
		bool operator !=(const array_reverse_iterator& rhs) const {return value != rhs.value;}
		bool operator ==(const array_reverse_iterator& rhs) const {return value == rhs.value;}
		array_reverse_iterator& operator ++() {--value; return *this;}
		array_reverse_iterator& operator --() {++value; return *this;}
		array_reverse_iterator operator ++(int) {return value--;}
		array_reverse_iterator operator --(int) {return value++;}
		array_reverse_iterator& operator +=(size_t Offset) {value -= Offset; return *this;}
		array_reverse_iterator& operator -=(size_t Offset) {value += Offset; return *this;}
		array_reverse_iterator operator +(size_t Offset) const {return value - Offset;}
		array_reverse_iterator operator -(size_t Offset) const {return value + Offset;}
		ptrdiff_t operator -(const array_reverse_iterator& rhs) const {return rhs.value - value;}
	protected:
		PtrType value;
	};

	template<class Type, size_t Size>
	inline auto rbegin(Type (&a)[Size]) -> array_reverse_iterator<Type*> {return a + Size - 1;}
	template<class Type, size_t Size>
	inline auto rend(Type (&a)[Size]) -> array_reverse_iterator<Type*> {return a - 1;}

	template<class Container>
	inline auto crbegin(const Container& c) -> decltype(c.crbegin()) {return c.crbegin();}
	template<class Container>
	inline auto crend(const Container& c) -> decltype(c.crend()) {return c.crend();}

	template<class Type, size_t Size>
	inline auto crbegin(Type (&a)[Size]) -> array_reverse_iterator<const Type*> {return a + Size - 1;}
	template<class Type, size_t Size>
	inline auto crend(Type (&a)[Size]) -> array_reverse_iterator<const Type*> {return a - 1;}
};
#endif

// trick to allow usage of operator :: with decltype(T)
#define DECLTYPE(T) std::enable_if<true, decltype(T)>::type
#define PTRTYPE(T) std::remove_pointer<decltype(T)>::type

template <class Container>
typename std::remove_reference<typename std::remove_pointer<Container>::type>::type::value_type get_value_type(Container& T);

template <class Type, size_t Size>
Type get_value_type(Type (&a)[Size]);

// VALUE_TYPE works with both arrays and containers
#define VALUE_TYPE(T) DECLTYPE(get_value_type(T))

#define DECLTYPEOF(T, subtype) std::remove_reference<typename std::remove_pointer<typename DECLTYPE(T)>::type>::type::subtype
#define ITERATOR(T) DECLTYPEOF(T, iterator)
#define CONST_ITERATOR(T) DECLTYPEOF(T, const_iterator)
#define REVERSE_ITERATOR(T) DECLTYPEOF(T, reverse_iterator)
#define CONST_REVERSE_ITERATOR(T) DECLTYPEOF(T, const_reverse_iterator)

#ifdef __GNUC__
#define T_VALUE_TYPE(T) typename VALUE_TYPE(T)
#else
#define T_VALUE_TYPE(T) VALUE_TYPE(T)
#endif

#define LAMBDA_PREDICATE(T, i, ...) [&](T_VALUE_TYPE(T)& i, ##__VA_ARGS__)
#define CONST_LAMBDA_PREDICATE(T, i, ...) [&](const T_VALUE_TYPE(T)& i, ##__VA_ARGS__)

#define ALL_RANGE(T) std::begin(T), std::end(T)
#define ALL_REVERSE_RANGE(T) std::rbegin(T), std::rend(T)
#define ALL_CONST_RANGE(T) std::cbegin(T), std::cend(T)
#define ALL_CONST_REVERSE_RANGE(T) std::crbegin(T), std::crend(T)

#define RANGE(T, i, ...) ALL_RANGE(T), LAMBDA_PREDICATE(T, i, ##__VA_ARGS__)
#define REVERSE_RANGE(T, i, ...) ALL_REVERSE_RANGE(T), LAMBDA_PREDICATE(T, i, ##__VA_ARGS__)
#define CONST_RANGE(T, i, ...) ALL_CONST_RANGE(T), CONST_LAMBDA_PREDICATE(T, i, ##__VA_ARGS__)
#define CONST_REVERSE_RANGE(T, i, ...) ALL_CONST_REVERSE_RANGE(T), CONST_LAMBDA_PREDICATE(T, i, ##__VA_ARGS__)

#define FOR_RANGE(T, i) for(auto i = std::begin(T); i != std::end(T); ++i)
#define FOR_REVERSE_RANGE(T, i) for(auto i = std::rbegin(T); i != std::rend(T); ++i)
#define FOR_CONST_RANGE(T, i) for(auto i = std::cbegin(T); i != std::cend(T); ++i)
#define FOR_CONST_REVERSE_RANGE(T, i) for(auto i = std::crbegin(T); i != std::crend(T); ++i)


template<class T>
class array_ptr:public std::unique_ptr<T[]>
{
public:
	array_ptr(){}
	array_ptr(array_ptr&& Right){std::unique_ptr<T[]>::swap(Right);}
	array_ptr(size_t size, bool init = false):std::unique_ptr<T[]>(init? new T[size]() : new T[size]){}
	array_ptr& operator=(array_ptr&& Right){std::unique_ptr<T[]>::swap(Right); return *this;}
	void reset(size_t size, bool init = false){std::unique_ptr<T[]>::reset(init? new T[size]() : new T[size]);}
	void reset(){std::unique_ptr<T[]>::reset();}
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
	block_ptr& operator=(block_ptr&& Right){char_ptr::swap(Right); return *this;}
	T* get() const {return reinterpret_cast<T*>(char_ptr::get());}
	T* operator->() const {return get();}
	T& operator*() const {return *get();}
};

template<typename T>
inline void DeleteValues(T& std_container)
{
	std::for_each(ALL_CONST_RANGE(std_container), std::default_delete<typename std::remove_pointer<typename T::value_type>::type>());
}

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
