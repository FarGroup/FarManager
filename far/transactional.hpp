#pragma once

/*
transactional.hpp
*/
/*
Copyright © 2015 Far Group
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

class transactional
{
public:

	virtual ~transactional() {}
	virtual bool BeginTransaction() = 0;
	virtual bool EndTransaction() = 0;
	virtual bool RollbackTransaction() = 0;

	class scoped_transaction: noncopyable
	{
	public:
		scoped_transaction(transactional* parent):m_parent(parent) { m_parent->BeginTransaction(); }
		~scoped_transaction() { if (m_parent) m_parent->EndTransaction(); }
		scoped_transaction(scoped_transaction&& rhs) :m_parent(nullptr) { *this = std::move(rhs); }
		MOVE_OPERATOR_BY_SWAP(scoped_transaction);
		void swap(scoped_transaction& rhs) noexcept { using std::swap; swap(m_parent, rhs.m_parent); }
		FREE_SWAP(scoped_transaction);

	private:
		transactional* m_parent;
	};

	scoped_transaction ScopedTransaction() {return scoped_transaction(this); }
};

