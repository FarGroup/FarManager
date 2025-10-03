#ifndef SHIMS_PRE_HPP_A18E0B5A_ECE5_4B78_96AA_55FE47AB1DEC
#define SHIMS_PRE_HPP_A18E0B5A_ECE5_4B78_96AA_55FE47AB1DEC
#pragma once

/*
shims_pre.hpp

Workarounds for supported compilers & libraries

Here be dragons
*/
/*
Copyright © 2025 Far Group
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

#include "compiler.hpp"

#include <version>

//----------------------------------------------------------------------------

#if COMPILER(CL)



#endif

//----------------------------------------------------------------------------

#if COMPILER(GCC)

// Current implementation of wcschr etc. in gcc removes const from the returned pointer. The issue has been opened since 2007.
// These semi-magical defines and appropriate inline overloads in shims_post.hpp are intended to fix this madness.

// Force C version to return const
#undef _CONST_RETURN
#define _CONST_RETURN const
// Disable broken inline overloads
#define __CORRECT_ISO_CPP_WCHAR_H_PROTO

#endif

//----------------------------------------------------------------------------

#if COMPILER(CLANG)

WARNING_PUSH()
WARNING_DISABLE_CLANG("-Wbuiltin-macro-redefined")
// Seems to be broken in v20 or incompatible with libstdc++ headers
#undef __cpp_explicit_this_parameter
WARNING_POP()

#endif

//----------------------------------------------------------------------------

#endif // SHIMS_PRE_HPP_A18E0B5A_ECE5_4B78_96AA_55FE47AB1DEC
