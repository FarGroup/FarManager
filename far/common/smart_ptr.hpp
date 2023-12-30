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

#include "preprocessor.hpp"
#include "utility.hpp"

#include <array>
#include <bit>
#include <memory>
#include <variant>

#include <cassert>

//----------------------------------------------------------------------------

template<typename T, size_t MinStaticSize> requires std::is_trivially_copyable_v<T>
class array_ptr
{
public:
	NONCOPYABLE(array_ptr);

	array_ptr() noexcept
	{
		m_Buffer.template emplace<static_type>();
	}

	explicit array_ptr(size_t const Size, bool Init = false):
		array_ptr()
	{
		reset(Size, Init);
	}

	array_ptr(array_ptr&& rhs) noexcept:
		m_Buffer(std::move(rhs.m_Buffer)),
		m_Size(std::exchange(rhs.m_Size, 0))
	{
	}

	array_ptr& operator=(array_ptr&& rhs) noexcept
	{
		m_Buffer = std::move(rhs.m_Buffer);
		m_Size = std::exchange(rhs.m_Size, 0);
		return *this;
	}

	void reset(size_t const Size = 0, bool const Init = false)
	{
		if (Size > StaticSize)
		{
			auto& DynamicBuffer = m_Buffer.template emplace<dynamic_type>();

			// We don't need a strong guarantee here, so it's better to reduce memory usage
			DynamicBuffer.reset();
			DynamicBuffer.reset(Init? new T[Size]() : new T[Size]);
		}
		else
		{
			m_Buffer.template emplace<static_type>();
		}

		m_Size = Size;
	}

	[[nodiscard]] auto data() const { return get_data(); }
	[[nodiscard]] auto size() const { return m_Size; }
	[[nodiscard]] auto empty() const { return !m_Size; }
	[[nodiscard]] auto& operator[](size_t const Index) const { return data()[Index]; }
	[[nodiscard]] explicit operator bool() const { return !empty(); }

	[[nodiscard]] auto begin() const { return data(); }
	[[nodiscard]] auto end() const { return begin() + m_Size; }
	[[nodiscard]] auto cbegin() const { return begin(); }
	[[nodiscard]] auto cend() const { return end(); }

	[[nodiscard]]
	T& operator*() const noexcept
	{
		assert(!this->empty());
		return *this->data();
	}

private:
	using dynamic_type = std::unique_ptr<T[]>;
	constexpr static size_t StaticSize = std::max(MinStaticSize, sizeof(dynamic_type) / sizeof(T));
	using static_type = std::array<T, StaticSize>;

	[[nodiscard]]
	T* get_data() const
	{
		return std::visit(overload
		{
			[](static_type& Data) { return Data.data(); },
			[](dynamic_type& Data) { return Data.get(); }
		}, m_Buffer);
	}

	mutable std::variant<static_type, dynamic_type> m_Buffer;
	size_t m_Size{};
};

template<size_t Size = 1>
using wchar_t_ptr_n = array_ptr<wchar_t, Size>;

template<size_t Size = 1>
using char_ptr_n = array_ptr<char, Size>;

using wchar_t_ptr = wchar_t_ptr_n<1>;
using char_ptr = char_ptr_n<1>;

// Storage for variable size structs, e.g. with data following the struct in the memory block
template<typename T, size_t Size = 1> requires std::is_trivially_copyable_v<T>
class block_ptr: private array_ptr<std::byte, Size>
{
public:
	NONCOPYABLE(block_ptr);
	MOVABLE(block_ptr);

	using base = array_ptr<std::byte, Size>;
	using base::base;
	using base::reset;
	using base::size;
	using base::empty;
	using base::operator bool;

	block_ptr() noexcept = default;

	[[nodiscard]]
	auto data() const noexcept
	{
		assert(this->size() >= sizeof(T));
		return std::bit_cast<T*>(base::data());
	}

	[[nodiscard]]
	auto operator->() const noexcept
	{
		return data();
	}

	[[nodiscard]]
	auto& operator*() const noexcept
	{
		return *data();
	}

	auto& bytes()
	{
		return static_cast<base&>(*this);
	}
};

template<typename T>
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
		void operator()(FILE* Object) const noexcept
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
	auto operator&() && { return &m_RawPtr; }

private:
	T* m_Ptr;
	typename T::pointer m_RawPtr{};
};

template<auto acquire, auto release, typename owner>
[[nodiscard]]
auto make_raii_wrapper(owner* Owner)
{
	std::invoke(acquire, Owner);

	struct releaser
	{
		void operator()(owner* const OwnerPtr) const
		{
			std::invoke(release, OwnerPtr);
		}
	};

	return std::unique_ptr<owner, releaser>(Owner);
}

namespace detail
{
	struct nop_deleter { void operator()(void*) const noexcept {} };
}

template<class T>
using movable_ptr = std::unique_ptr<T, detail::nop_deleter>;

#endif // SMART_PTR_HPP_DE65D1E8_C925_40F7_905A_B7E3FF40B486
