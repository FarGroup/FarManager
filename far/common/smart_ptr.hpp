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

#include "placement.hpp"
#include "preprocessor.hpp"
#include "range.hpp"
#include "utility.hpp"

//----------------------------------------------------------------------------

template<typename T, size_t MinStaticSize, REQUIRES(std::is_trivially_copyable_v<T>)>
class array_ptr: public span<T>
{
public:
	NONCOPYABLE(array_ptr);

	array_ptr() noexcept
	{
		m_Buffer.template emplace<static_type>();
		init_span(0);
	}

	explicit array_ptr(size_t const Size, bool Init = false):
		array_ptr()
	{
		reset(Size, Init);
	}

	array_ptr(array_ptr&& rhs) noexcept
	{
		move_from(rhs);
	}

	array_ptr& operator=(array_ptr&& rhs) noexcept
	{
		return move_from(rhs);
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

		init_span(Size);
	}

	[[nodiscard]]
	explicit operator bool() const noexcept
	{
		return !this->empty();
	}

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

	void init_span(size_t const Size) noexcept
	{
		static_cast<span<T>&>(*this) =
		{
			std::visit(overload
			{
				[](static_type& Data){ return Data.data(); },
				[](dynamic_type& Data){ return Data.get(); }
			}, m_Buffer),
			Size
		};
	}

	bool is_dynamic(size_t const Size) const noexcept
	{
		return Size > StaticSize;
	}

	bool is_dynamic() const noexcept
	{
		return is_dynamic(this->size());
	}

	array_ptr& move_from(array_ptr& rhs) noexcept
	{
		m_Buffer = std::move(rhs.m_Buffer);
		init_span(rhs.size());
		rhs.init_span(0);

		return *this;
	}

	mutable std::variant<static_type, dynamic_type> m_Buffer;
};

template<size_t Size = 1>
using wchar_t_ptr_n = array_ptr<wchar_t, Size>;

template<size_t Size = 1>
using char_ptr_n = array_ptr<char, Size>;

using wchar_t_ptr = wchar_t_ptr_n<1>;
using char_ptr = char_ptr_n<1>;


template<typename T, size_t Size = 1, REQUIRES(std::is_trivially_copyable_v<T>)>
class block_ptr: public char_ptr_n<Size>
{
public:
	NONCOPYABLE(block_ptr);
	MOVABLE(block_ptr);

	using char_ptr_n<Size>::char_ptr_n;
	block_ptr() noexcept = default;

	[[nodiscard]]
	decltype(auto) data() const noexcept
	{
		assert(this->size() >= sizeof(T));
		return reinterpret_cast<T*>(char_ptr_n<Size>::data());
	}

	[[nodiscard]]
	decltype(auto) operator->() const noexcept { return data(); }

	[[nodiscard]]
	decltype(auto) operator*() const noexcept {return *data();}
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
	auto Releaser = [Release](owner* OwnerPtr)
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
