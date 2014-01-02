#pragma once

/*
common.hpp

Some useful classes, templates && macros.

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
# define ENUM(ENUM_NAME) enum ENUM_NAME:int
#endif

#ifdef _MSC_VER
# if _MSC_VER>1600
#  define ENUM(ENUM_NAME) enum ENUM_NAME:int
# else
#  define ENUM(ENUM_NAME) enum ENUM_NAME
# endif
#endif

// trick to allow usage of operator :: with decltype(T)
#define DECLTYPE(T) std::enable_if<true, decltype(T)>::type
#define PTRTYPE(T) std::remove_pointer<decltype(T)>::type

#define VALUE_TYPE(T) std::remove_reference<typename DECLTYPE(*std::begin(T))>::type
#define CONST_VALUE_TYPE(T) std::remove_reference<typename DECLTYPE(*std::cbegin(T))>::type

#define DECLTYPEOF(T, subtype) std::remove_reference<typename std::remove_pointer<typename DECLTYPE(T)>::type>::type::subtype
#define ITERATOR(T) DECLTYPEOF(T, iterator)
#define CONST_ITERATOR(T) DECLTYPEOF(T, const_iterator)
#define REVERSE_ITERATOR(T) DECLTYPEOF(T, reverse_iterator)
#define CONST_REVERSE_ITERATOR(T) DECLTYPEOF(T, const_reverse_iterator)

#ifdef __GNUC__
#define T_VALUE_TYPE(T) typename VALUE_TYPE(T)
#define T_CONST_VALUE_TYPE(T) typename CONST_VALUE_TYPE(T)
#else
#define T_VALUE_TYPE(T) VALUE_TYPE(T)
#define T_CONST_VALUE_TYPE(T) CONST_VALUE_TYPE(T)
#endif

#define LAMBDA_PREDICATE(T, i, ...) [&](T_VALUE_TYPE(T)& i, ##__VA_ARGS__)
#define CONST_LAMBDA_PREDICATE(T, i, ...) [&](const T_CONST_VALUE_TYPE(T)& i, ##__VA_ARGS__)

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

// C++11-like range-based for
#if defined _MSC_VER && _MSC_VER < 1700
#define DECORATED(name) _RANGE_FOR_EMULATION_ ## name ## _
#define f_container DECORATED(container)
#define f_stop DECORATED(stop)
#define f_it DECORATED(it)
#define f_stop_it DECORATED(stop_it)
#define FOR(i, c) \
	if (bool f_stop = false); \
	else for (auto&& f_container = c; !f_stop; f_stop = true) \
	for (auto f_it = std::begin(f_container), e = false? f_it : std::end(f_container); f_it != e && !f_stop; ++f_it) \
	if (bool f_stop_it = !(f_stop = true)); \
			else for (i = *f_it; !f_stop_it; f_stop_it = true, f_stop = false)
// { body }
#undef f_stop_it
#undef f_it
#undef f_stop
#undef f_container
#undef DECORATED
#else
#define FOR(i, c) for(i: c)
#endif

template<class T>
class array_ptr
{
public:
	array_ptr() : m_size() {}
	array_ptr(array_ptr&& other) : m_size() { *this = std::move(other); }
	array_ptr(size_t size, bool init = false) : m_array(init? new T[size]() : new T[size]), m_size(size) {}
	array_ptr& operator=(array_ptr&& other) { m_array = std::move(other.m_array); m_size = other.m_size; other.m_size = 0; return *this;}
	void reset(size_t size, bool init = false) { m_array.reset(init? new T[size]() : new T[size]); m_size = size;}
	void reset() { m_array.reset(); m_size = 0; }
	void swap(array_ptr& other) { m_array.swap(other.m_array); std::swap(m_size, other.m_size); }
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
	block_ptr& operator=(block_ptr&& Right){char_ptr::swap(Right); return *this;}
	T* get() const {return reinterpret_cast<T*>(char_ptr::get());}
	T* operator->() const {return get();}
	T& operator*() const {return *get();}
};

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

template<int id>
struct write_t
{
	write_t(const std::wstring& str, size_t n) : m_part(str.substr(0, n)), m_size(n) {}
	std::wstring m_part;
	size_t m_size;
};

typedef write_t<0> write_max;
typedef write_t<1> write_exact;

inline std::wostream& operator <<(std::wostream& stream, const write_max& p)
{
	return stream << p.m_part;
}

inline std::wostream& operator <<(std::wostream& stream, const write_exact& p)
{
	stream.width(p.m_size);
	return stream << p.m_part;
}

class FarException : public std::runtime_error
{
public:
	FarException(const char* Message) : std::runtime_error(Message) {}
};

class FarRecoverableException : public FarException
{
public:
	FarRecoverableException(const char* Message) : FarException(Message) {}
};

class ScopeExit
{
public:
	ScopeExit(const std::function<void()>& f) : m_f(f) {}
	~ScopeExit() { m_f(); }

private:
	std::function<void()> m_f;
};

#define _SCOPE_EXIT_NAME(name, suffix) name ## suffix
#define SCOPE_EXIT_NAME(name, suffix) _SCOPE_EXIT_NAME(name, suffix)
#define SCOPE_EXIT \
	std::function<void()> SCOPE_EXIT_NAME(scope_exit_func_, __LINE__); \
	ScopeExit SCOPE_EXIT_NAME(scope_exit_, __LINE__) = SCOPE_EXIT_NAME(scope_exit_func_, __LINE__) = [&]() /* lambda body here */

template<class T>
inline void resize_nomove(T& container, size_t size)
{
	T(size).swap(container);
}

template<class T>
inline void clear_and_shrink(T& container)
{
	T().swap(container);
}

template <typename T>
bool CheckNullOrStructSize(const T* s) {return !s || (s->StructSize >= sizeof(T));}
template <typename T>
bool CheckStructSize(const T* s) {return s && (s->StructSize >= sizeof(T));}

template <typename type_1, typename type_2>
struct simple_pair
{
	typedef type_1 first_type;
	typedef type_2 second_type;

	first_type first;
	second_type second;
};

template<typename T>
class enumerator
{
public:
	virtual ~enumerator() {}
	class const_iterator: public std::iterator<std::forward_iterator_tag, T>
	{
	public:
		typedef const T value_type;

		const_iterator(enumerator* collection = nullptr, size_t index = -1): m_collection(collection), m_index(index), m_value() {}
		const_iterator(const const_iterator& rhs): m_collection(rhs.m_collection), m_index(rhs.m_index), m_value(rhs.m_value) {}
		const_iterator& operator =(const const_iterator& rhs) { m_collection = rhs.m_collection; m_index = rhs.m_index; m_value = rhs.m_value; return *this;}
		value_type* operator ->() const { return &m_value; }
		value_type& operator *() const { return m_value; }
		const_iterator& operator ++() { if (!m_collection->get(++m_index, m_value)) m_index = size_t(-1); return *this; }
		bool operator ==(const const_iterator& rhs) const { return m_index == rhs.m_index; }
		bool operator !=(const const_iterator& rhs) const { return !(*this == rhs); }

	private:
		friend enumerator;
		enumerator* m_collection;
		size_t m_index;
		T m_value;
	};

	class iterator: public const_iterator
	{
	public:
		typedef T value_type;

		iterator(enumerator* collection = nullptr, size_t index = -1): const_iterator(collection, index) {}
		iterator(const iterator& rhs): const_iterator(rhs) {}
		value_type* operator ->() { return const_cast<T*>(const_iterator::operator->()); }
		value_type& operator *() { return const_cast<T&>(const_iterator::operator*()); }
	};

	iterator begin() { iterator result(this, 0); if (!get(result.m_index, result.m_value)) result.m_index = size_t(-1); return result; }
	iterator end() { return iterator(this, size_t(-1)); }
	const_iterator begin() const { return const_cast<enumerator*>(this)->begin(); }
	const_iterator end() const { return const_cast<enumerator*>(this)->end(); }
	const_iterator cbegin() const { return begin(); }
	const_iterator cend() const { return end(); }

private:
	friend iterator;
	virtual bool get(size_t index, T& value) = 0;
};

// begin() / end() interface for null-terminated strings
template<class T>
class as_string_t: public enumerator<T>
{
public:
	as_string_t(const T* str, size_t size): m_str(str), m_size(size) {}
	virtual bool get(size_t index, T& value) override
	{
		if (m_size == size_t(-1))
		{
			return (value = m_str[index]) != 0;
		}
		else
		{
			if (index == m_size)
			{
				return false;
			}
			else
			{
				value = m_str[index];
				return true;
			}
		}
	}

private:
	const T* m_str;
	size_t m_size;
};

template<class T>
typename std::enable_if<!std::is_array<T>::value, as_string_t<T>>::type as_string(const T* str, size_t size = size_t(-1)) { return as_string_t<T>(str, size); }

// to avoid processing null character in cases like char c[] = "foo".
template <typename T, size_t N>
typename std::enable_if<std::is_array<T>::value, as_string_t<T>>::type as_string(T(&str)[N]) { return as_string_t<T>(str, N - 1); }

template<class T>
size_t as_index(T t) { return static_cast<typename std::make_unsigned<T>::type>(t); }

template<typename T>
inline void ClearStruct(T& s)
{
	static_assert(std::is_pod<T>::value, "ClearStruct template requires a POD type");
	static_assert(!std::is_pointer<T>::value, "ClearStruct template requires a reference to an object");
	memset(&s, 0, sizeof(s));
}

template<typename T>
inline void ClearStructUnsafe(T& s)
{
	static_assert(!std::is_pointer<T>::value, "ClearStruct template requires a reference to an object");
	memset(&s, 0, sizeof(s));
}

template<typename T>
inline void ClearArray(T& a)
{
	static_assert(std::is_pod<T>::value, "ClearArray template requires a POD type");
	static_assert(std::is_array<T>::value, "ClearArray template requires an array");
	memset(a, 0, sizeof(a));
}

template<class T>
inline const T* NullToEmpty(const T* Str) { static const T empty = T(); return Str? Str : &empty; }
template<class T>
inline const T* EmptyToNull(const T* Str) { return (Str && !*Str)? nullptr : Str; }

#define MOVE_OPERATOR_BY_SWAP(Type) \
Type& operator=(Type&& rhs) { swap(rhs); return *this; }

#define STD_SWAP_SPEC(Type) \
namespace std \
{ \
	template<> \
	inline void swap(Type& a, Type& b) \
	{ \
		a.swap(b); \
	} \
}

#define ALLOW_SWAP_ACCESS(Type) \
friend void std::swap<Type>(Type&, Type&);

