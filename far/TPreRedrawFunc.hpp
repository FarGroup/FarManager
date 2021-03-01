#ifndef TPREREDRAWFUNC_HPP_57AEAD28_BBC8_4C43_A40A_5B46CBEA2425
#define TPREREDRAWFUNC_HPP_57AEAD28_BBC8_4C43_A40A_5B46CBEA2425
#pragma once

/*
TPreRedrawFunc.hpp

Фоновый апдейт

*/
/*
Copyright © 2000 Far Group
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

// Internal:

// Platform:

// Common:
#include "common/function_traits.hpp"
#include "common/singleton.hpp"

// External:

//----------------------------------------------------------------------------

class PreRedrawItem: noncopyable
{
public:
	using handler_type = std::function<void()>;

	explicit PreRedrawItem(handler_type PreRedrawFunc):
		m_PreRedrawFunc(std::move(PreRedrawFunc))
	{
	}

	virtual ~PreRedrawItem() = default;

	auto operator()() const
	{
		m_PreRedrawFunc();
	}

private:
	handler_type m_PreRedrawFunc;
};

class TPreRedrawFunc: public singleton<TPreRedrawFunc>
{
	IMPLEMENTS_SINGLETON;

public:
	void push(std::unique_ptr<PreRedrawItem>&& Source)
	{
		m_Items.emplace(std::move(Source));
	}

	auto pop()
	{
		auto Top = std::move(m_Items.top());
		m_Items.pop();
		return Top;
	}

	template<typename callable>
	void operator()(const callable& Callable)
	{
		if (m_Items.empty())
			return;

		// M#3849 - apparently sometimes the top handler isn't the one we're looking for.
		// TODO: investigate further.
		const auto Handler = dynamic_cast<std::remove_reference_t<typename function_traits<callable>::template arg<0>>*>(m_Items.top().get());
		if (!Handler)
			return;

		Callable(*Handler);
	}

private:
	TPreRedrawFunc() = default;

	std::stack<std::unique_ptr<PreRedrawItem>> m_Items;
};

class TPreRedrawFuncGuard: noncopyable
{
public:
	explicit TPreRedrawFuncGuard(std::unique_ptr<PreRedrawItem>&& Item)
	{
		TPreRedrawFunc::instance().push(std::move(Item));
	}

	~TPreRedrawFuncGuard()
	{
		TPreRedrawFunc::instance().pop();
	}
};

#endif // TPREREDRAWFUNC_HPP_57AEAD28_BBC8_4C43_A40A_5B46CBEA2425
