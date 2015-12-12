#ifndef CONDITIONAL_HPP_18900E4A_F2F5_48B9_A92A_DEE70617591B
#define CONDITIONAL_HPP_18900E4A_F2F5_48B9_A92A_DEE70617591B
#pragma once

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

/*
Usage:

class object: public conditional<object>
{
public:
    bool operator!() { return false; }
};

object Object;

if (Object)
    foo();

if (!Object)
    bar();
*/

template<class T>
struct conditional
{
#ifdef NO_EXPLICIT_CONVERSION_OPERATORS
	struct unspecified_bool
	{
		struct OPERATORS_NOT_ALLOWED;
		void true_value(OPERATORS_NOT_ALLOWED*) {}
	};
	typedef void (unspecified_bool::*unspecified_bool_type)(typename unspecified_bool::OPERATORS_NOT_ALLOWED*);

	operator unspecified_bool_type() const
	{
		return !static_cast<const T&>(*this).operator!()? &unspecified_bool::true_value : unspecified_bool_type();
	}
#else
	explicit operator bool() const
	{
		return !static_cast<const T&>(*this).operator!();
	}
#endif
};

#endif // CONDITIONAL_HPP_18900E4A_F2F5_48B9_A92A_DEE70617591B
