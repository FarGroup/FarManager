#ifndef SMART_PTR_HPP_DE65D1E8_C925_40F7_905A_B7E3FF40B486
#define SMART_PTR_HPP_DE65D1E8_C925_40F7_905A_B7E3FF40B486
#pragma once

/*
smart_ptr.hpp
*/
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

template<typename T, size_t StaticSize>
class array_ptr
{
public:
	NONCOPYABLE(array_ptr);
	MOVABLE(array_ptr);

	array_ptr() noexcept = default;

	array_ptr(std::nullptr_t) noexcept
	{
	}

	explicit array_ptr(size_t Size, bool Init = false)
	{
		reset(Size, Init);
	}

	void reset(size_t Size, bool Init = false)
	{
		if (Size > StaticSize)
		{
			m_IsStatic = false;
			m_DynamicBuffer.reset(Init? new T[Size]() : new T[Size]);
		}
		else
		{
			m_IsStatic = true;
			m_DynamicBuffer.reset();
		}

		m_Size = Size;
	}

	void reset() noexcept
	{
		m_IsStatic = false;
		m_DynamicBuffer.reset();
		m_Size = 0;
	}

	[[nodiscard]]
	size_t size() const noexcept
	{
		return m_Size;
	}

	[[nodiscard]]
	explicit operator bool() const noexcept
	{
		return m_IsStatic || m_DynamicBuffer;
	}

	[[nodiscard]]
	T* get() const noexcept
	{
		return m_IsStatic? m_StaticBuffer.data() : m_DynamicBuffer.get();
	}

	[[nodiscard]]
	T& operator*() const
	{
		assert(m_Size);
		return *get();
	}

	[[nodiscard]]
	T& operator[](size_t Index) const
	{
		assert(Index < m_Size);
		return get()[Index];
	}

private:
	mutable std::array<T, StaticSize> m_StaticBuffer;
	std::unique_ptr<T[]> m_DynamicBuffer;
	size_t m_Size{};
	bool m_IsStatic{};
};

template<size_t Size = 1>
using wchar_t_ptr_n = array_ptr<wchar_t, Size>;

template<size_t Size = 1>
using char_ptr_n = array_ptr<char, Size>;

using wchar_t_ptr = wchar_t_ptr_n<1>;
using char_ptr = char_ptr_n<1>;


template<typename T, size_t Size = 1>
class block_ptr: public char_ptr_n<Size>
{
public:
	NONCOPYABLE(block_ptr);
	MOVABLE(block_ptr);

	using char_ptr_n<Size>::char_ptr_n;
	block_ptr() = default;

	[[nodiscard]]
	decltype(auto) get() const noexcept {return reinterpret_cast<T*>(char_ptr_n<Size>::get());}

	[[nodiscard]]
	decltype(auto) operator->() const noexcept { return get(); }

	[[nodiscard]]
	decltype(auto) operator*() const noexcept {return *get();}
};

template <typename T>
class unique_ptr_with_ondestroy
{
public:
	~unique_ptr_with_ondestroy() { OnDestroy(); }

	[[nodiscard]]
	decltype(auto) get() const noexcept { return ptr.get(); }

	[[nodiscard]]
	decltype(auto) operator->() const noexcept { return ptr.operator->(); }

	[[nodiscard]]
	decltype(auto) operator*() const { return *ptr; }

	[[nodiscard]]
	explicit operator bool() const noexcept { return ptr.operator bool(); }

	decltype(auto) operator=(std::unique_ptr<T>&& value) noexcept { OnDestroy(); ptr = std::move(value); return *this; }

private:
	void OnDestroy() { if (ptr) ptr->OnDestroy(); }

	std::unique_ptr<T> ptr;
};

namespace detail
{
	struct file_closer
	{
		void operator()(FILE* Object) const
		{
			fclose(Object);
		}
	};
}

using file_ptr = std::unique_ptr<FILE, detail::file_closer>;

template<typename T>
class ptr_setter
{
public:
	NONCOPYABLE(ptr_setter);

	explicit ptr_setter(T& Ptr): m_Ptr(&Ptr) {}
	~ptr_setter() { m_Ptr->reset(m_RawPtr); }

	[[nodiscard]]
	auto operator&() { return &m_RawPtr; }

private:
	T* m_Ptr;
	typename T::pointer m_RawPtr{};
};

template<typename owner, typename acquire, typename release>
[[nodiscard]]
auto make_raii_wrapper(owner* Owner, const acquire& Acquire, const release& Release)
{
	std::invoke(Acquire, Owner);
	auto&& Releaser = [Release](owner* OwnerPtr)
	{
		std::invoke(Release, OwnerPtr);
	};
	return std::unique_ptr<owner, std::remove_reference_t<decltype(Releaser)>>(Owner, std::move(Releaser));
}

namespace detail
{
	struct nop_deleter { void operator()(void*) const noexcept {} };
}

template<class T>
using movable_ptr = std::unique_ptr<T, detail::nop_deleter>;

#endif // SMART_PTR_HPP_DE65D1E8_C925_40F7_905A_B7E3FF40B486
