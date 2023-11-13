#ifndef LIBRARY_HPP_3C1EF122_E9E8_4F3D_8E2F_C5E2829E2E32
#define LIBRARY_HPP_3C1EF122_E9E8_4F3D_8E2F_C5E2829E2E32
#pragma once

/*
library.hpp

Library-specific macros and definitions
*/
/*
Copyright © 2022 Far Group
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

#include <version>

#if defined(_MSVC_STL_VERSION) && defined(_MSVC_STL_UPDATE)
#define STANDARD_LIBRARY_NAME L"Microsoft STL"
#define STANDARD_LIBRARY_VERSION_MAJOR (_MSVC_STL_VERSION / 10)
#define STANDARD_LIBRARY_VERSION_MINOR (_MSVC_STL_VERSION % 10)
#define STANDARD_LIBRARY_VERSION_PATCH _MSVC_STL_UPDATE
#elif defined(_GLIBCXX_RELEASE) and defined(__GLIBCXX__)
#define STANDARD_LIBRARY_NAME L"libstdc++"
#define STANDARD_LIBRARY_VERSION_MAJOR _GLIBCXX_RELEASE
#define STANDARD_LIBRARY_VERSION_MINOR 0
#define STANDARD_LIBRARY_VERSION_PATCH __GLIBCXX__
#elif defined(_LIBCPP_VERSION)
#define STANDARD_LIBRARY_NAME L"libc++"
#define STANDARD_LIBRARY_VERSION_MAJOR (_LIBCPP_VERSION / 10000)
#define STANDARD_LIBRARY_VERSION_MINOR ((_LIBCPP_VERSION % 10000) / 100)
#define STANDARD_LIBRARY_VERSION_PATCH (_LIBCPP_VERSION % 100)
#else
#define STANDARD_LIBRARY_NAME L"Unknown standard library"
#define STANDARD_LIBRARY_VERSION_MAJOR 0
#define STANDARD_LIBRARY_VERSION_MINOR 0
#define STANDARD_LIBRARY_VERSION_PATCH 0
#endif

#endif // LIBRARY_HPP_3C1EF122_E9E8_4F3D_8E2F_C5E2829E2E32
