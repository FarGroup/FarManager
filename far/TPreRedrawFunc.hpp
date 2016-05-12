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


struct PreRedrawItem: noncopyable
{
	typedef std::function<void()> handler_type;

	PreRedrawItem(const handler_type& PreRedrawFunc) : m_PreRedrawFunc(PreRedrawFunc) {}
	virtual ~PreRedrawItem() = default;

	handler_type m_PreRedrawFunc;
};

class TPreRedrawFunc: noncopyable
{
public:
	void push(std::unique_ptr<PreRedrawItem>&& Source){return Items.emplace(std::move(Source));}
	std::unique_ptr<PreRedrawItem> pop() { auto Top = std::move(Items.top()); Items.pop(); return Top; }
	PreRedrawItem* top() { return Items.top().get(); }
	bool empty() const {return Items.empty();}

private:
	friend TPreRedrawFunc& PreRedrawStack();

	TPreRedrawFunc() = default;
	std::stack<std::unique_ptr<PreRedrawItem>> Items;
};

inline TPreRedrawFunc& PreRedrawStack()
{
	static TPreRedrawFunc pr;
	return pr;
}

class TPreRedrawFuncGuard: noncopyable
{
public:
	TPreRedrawFuncGuard(std::unique_ptr<PreRedrawItem>&& Item)
	{
		PreRedrawStack().push(std::move(Item));
	}
	~TPreRedrawFuncGuard()
	{
		PreRedrawStack().pop();
	}
};

#endif // TPREREDRAWFUNC_HPP_57AEAD28_BBC8_4C43_A40A_5B46CBEA2425
