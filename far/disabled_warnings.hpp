// validator: no-include-guards
/*
disabled_warnings.hpp
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

#ifdef _MSC_VER
// https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warnings-c4000-c5999

// some warnings do not exist in older VC versions, no need to care about them:
#pragma warning(disable: 4616) // #pragma warning : warning number 'number' not a valid compiler warning
#pragma warning(disable: 4619) // #pragma warning : there is no warning number 'number'

#ifdef DISABLE_WARNINGS_IN_STD_HEADERS
// these warnings are triggered in the standard headers only:
#pragma warning(disable: 4091) // 'typedef ': ignored on left of 'type' when no variable is declared
#pragma warning(disable: 4265) // 'class' : class has virtual functions, but destructor is not virtual
#pragma warning(disable: 4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
#pragma warning(disable: 4768) // __declspec attributes before linkage specification are ignored
#pragma warning(disable: 4774) // 'function' : format string expected in argument 'number' is not a string literal
#pragma warning(disable: 4917) // 'declarator' : a GUID can only be associated with a class, interface or namespace
#pragma warning(disable: 4987) // nonstandard extension used: 'throw (...)'
#pragma warning(disable: 4996) // The compiler encountered a deprecated declaration
#pragma warning(disable: 4582) // 'class': constructor is not implicitly called
#pragma warning(disable: 4583) // 'class': destructor is not implicitly called
#pragma warning(disable: 5219) // implicit conversion from 'type1' to 'type2', possible loss of data
#else
// these in the rest of the code as well:
// TODO: some of these might be useful
#pragma warning(disable: 4061) // enumerator 'identifier' in switch of enum 'enumeration' is not explicitly handled by a case label
#pragma warning(disable: 4100) // 'identifier' : unreferenced formal parameter
#pragma warning(disable: 4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable: 4242) // 'identifier' : conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable: 4244) // 'conversion' conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable: 4245) // 'conversion' : conversion from 'type1' to 'type2', signed/unsigned mismatch
#pragma warning(disable: 4250) // 'class1' : inherits 'class2::member' via dominance
#pragma warning(disable: 4324) // 'struct_name' : structure was padded due to __declspec(align())
#pragma warning(disable: 4355) // 'this' : used in base member initializer list
#pragma warning(disable: 4365) // 'action' : conversion from 'type_1' to 'type_2', signed/unsigned mismatch
#pragma warning(disable: 4371) // layout of class may have changed from a previous version of the compiler due to better packing of member 'member'
#pragma warning(disable: 4373) // 'function': virtual function overrides 'function', previous versions of the compiler did not override when parameters only differed by const/volatile qualifiers
#pragma warning(disable: 4435) // 'class1' : Object layout under /vd2 will change due to virtual base 'class2'
#pragma warning(disable: 4464) // relative include path contains '..'
#pragma warning(disable: 4503) // 'identifier' : decorated name length exceeded, name was truncated
#pragma warning(disable: 4512) // 'class' : assignment operator could not be generated
#pragma warning(disable: 4514) // 'function' : unreferenced inline function has been removed
#pragma warning(disable: 4571) // Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
#pragma warning(disable: 4574) // 'identifier' is defined to be '0': did you mean to use '#if identifier'?
#pragma warning(disable: 4623) // 'derived class' : default constructor was implicitly defined as deleted because a base class default constructor is inaccessible or deleted
#pragma warning(disable: 4625) // 'derived class' : copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning(disable: 4626) // 'derived class' : assignment operator could not be generated because a base class assignment operator is inaccessible
#pragma warning(disable: 4640) // 'instance' : construction of local static object is not thread-safe
#pragma warning(disable: 4670) // identifier' : this base class is inaccessible
#pragma warning(disable: 4673) // throwing 'identifier' the following types will not be considered at the catch site
#pragma warning(disable: 4710) // 'function' : function not inlined
#pragma warning(disable: 4711) // function 'function' selected for inline expansion
#pragma warning(disable: 4746) // volatile access of '<expression>' is subject to /volatile:[iso|ms] setting; consider using __iso_volatile_load/store intrinsic functions
#pragma warning(disable: 4770) // partially validated enum 'name' used as index
#pragma warning(disable: 4738) // storing 32-bit float result in memory, possible loss of performance
#pragma warning(disable: 4686) //'user-defined type' : possible change in behavior, change in UDT return calling convention
#pragma warning(disable: 4814) // in C++14 'constexpr' will not imply 'const'; consider explicitly specifying 'const'
#pragma warning(disable: 4820) // 'bytes' bytes padding added after construct 'member_name'
#pragma warning(disable: 4866) // compiler may not enforce left-to-right evaluation order for call to operator_name
#pragma warning(disable: 4868) // compiler may not enforce left-to-right evaluation order in braced initializer list
#pragma warning(disable: 5025) // 'class': move assignment operator was implicitly defined as deleted
#pragma warning(disable: 5026) // 'class': move constructor was implicitly defined as deleted because a base class move constructor is inaccessible or deleted
#pragma warning(disable: 5027) // 'class': move assignment operator was implicitly defined as deleted because a base class move assignment operator is inaccessible or deleted
#pragma warning(disable: 5030) // attribute 'attribute' is not recognized
#pragma warning(disable: 5039) // 'function': pointer or reference to potentially throwing function passed to extern C function under -EHc. Undefined behavior may occur if this function throws an exception.
#pragma warning(disable: 5045) // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#pragma warning(disable: 5052) // Keyword 'char8_t' was introduced in C++20 and requires use of the '/std:c++latest' command-line option
#endif

#ifdef _DEBUG
// Happens only after "Apply code changes"
#pragma warning(disable: 4599) // command line argument number number does not match precompiled header
#endif

#endif


#ifdef __GNUC__
// https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html

#pragma GCC diagnostic ignored "-Wunknown-pragmas"

// TODO: some of these might be useful
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#endif


#ifdef __clang__
// https://clang.llvm.org/docs/DiagnosticsReference.html

#pragma clang diagnostic ignored "-Wunknown-warning-option"

// TODO: some of these might be useful
#pragma clang diagnostic ignored "-Wc++98-compat"
#pragma clang diagnostic ignored "-Wc++98-compat-pedantic"
#pragma clang diagnostic ignored "-Wconversion"
#pragma clang diagnostic ignored "-Wcovered-switch-default"
#pragma clang diagnostic ignored "-Wctad-maybe-unsupported"
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#pragma clang diagnostic ignored "-Wenum-compare"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#pragma clang diagnostic ignored "-Wkeyword-macro"
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#pragma clang diagnostic ignored "-Wmicrosoft-enum-value"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wnested-anon-types"
#pragma clang diagnostic ignored "-Wnonportable-system-include-path"
#pragma clang diagnostic ignored "-Wpadded"
#pragma clang diagnostic ignored "-Wreturn-std-move-in-c++11"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#pragma clang diagnostic ignored "-Wshadow-field"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wswitch-enum"
#pragma clang diagnostic ignored "-Wundefined-func-template"
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wunused-member-function"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wweak-vtables"
#endif
