#ifndef REL_OPS_HPP_216C5B74_491D_4AE0_AA85_4A424C1CF3BA
#define REL_OPS_HPP_216C5B74_491D_4AE0_AA85_4A424C1CF3BA
#pragma once

/*
rel_ops.hpp
*/
/*
Copyright © 2016 Far Group
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

//----------------------------------------------------------------------------

template <class T>
struct rel_ops
{
	[[nodiscard]] friend bool operator!=(const T& a, const T& b) { return !(a == b); }
	[[nodiscard]] friend bool operator> (const T& a, const T& b) { return b < a; }
	[[nodiscard]] friend bool operator<=(const T& a, const T& b) { return !(b < a); }
	[[nodiscard]] friend bool operator>=(const T& a, const T& b) { return !(a < b); }

	template <class Y> [[nodiscard]] friend bool operator!=(const T& a, const Y& b) { return !(a == b); }
	template <class Y> [[nodiscard]] friend bool operator> (const T& a, const Y& b) { return b < a; }
	template <class Y> [[nodiscard]] friend bool operator<=(const T& a, const Y& b) { return !(b < a); }
	template <class Y> [[nodiscard]] friend bool operator>=(const T& a, const Y& b) { return !(a < b); }
	template <class Y> [[nodiscard]] friend bool operator!=(const Y& a, const T& b) { return !(a == b); }
	template <class Y> [[nodiscard]] friend bool operator> (const Y& a, const T& b) { return b < a; }
	template <class Y> [[nodiscard]] friend bool operator<=(const Y& a, const T& b) { return !(b < a); }
	template <class Y> [[nodiscard]] friend bool operator>=(const Y& a, const T& b) { return !(a < b); }
};

#endif // REL_OPS_HPP_216C5B74_491D_4AE0_AA85_4A424C1CF3BA
