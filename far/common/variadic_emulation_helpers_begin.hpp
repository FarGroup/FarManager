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

#define VTE_TYPE(name)         name##_type
#define VTE_TYPENAME(name)     typename VTE_TYPE(name)
#define VTE_ARG(name)          VTE_TYPE(name) name
#define VTE_REF_ARG(name)      VTE_TYPE(name)&& name
#define VTE_FWD_ARG(name)      std::forward<VTE_TYPE(name)>(name)

#define VTE_GENERATE_N(TEMPLATE, n)\
	TEMPLATE(VTE_LIST##n(VTE_TYPENAME),\
	         VTE_LIST##n(VTE_ARG),\
	         VTE_LIST##n(VTE_REF_ARG),\
	         VTE_LIST##n(VTE_FWD_ARG))

#define VTE_LIST1(MODE)                    MODE(a1)
#define VTE_LIST2(MODE)  VTE_LIST1(MODE),  MODE(a2)
#define VTE_LIST3(MODE)  VTE_LIST2(MODE),  MODE(a3)
#define VTE_LIST4(MODE)  VTE_LIST3(MODE),  MODE(a4)
#define VTE_LIST5(MODE)  VTE_LIST4(MODE),  MODE(a5)
#define VTE_LIST6(MODE)  VTE_LIST5(MODE),  MODE(a6)
#define VTE_LIST7(MODE)  VTE_LIST6(MODE),  MODE(a7)
#define VTE_LIST8(MODE)  VTE_LIST7(MODE),  MODE(a8)
#define VTE_LIST9(MODE)  VTE_LIST8(MODE),  MODE(a9)
#define VTE_LIST10(MODE) VTE_LIST9(MODE),  MODE(a10)
#define VTE_LIST11(MODE) VTE_LIST10(MODE), MODE(a11)
#define VTE_LIST12(MODE) VTE_LIST11(MODE), MODE(a12)
#define VTE_LIST13(MODE) VTE_LIST12(MODE), MODE(a13)
#define VTE_LIST14(MODE) VTE_LIST13(MODE), MODE(a14)
#define VTE_LIST15(MODE) VTE_LIST14(MODE), MODE(a15)

// extend here [^] and here [v] if necessary

#define VTE_GENERATE(TEMPLATE)\
	VTE_GENERATE_N(TEMPLATE, 15)\
	VTE_GENERATE_N(TEMPLATE, 14)\
	VTE_GENERATE_N(TEMPLATE, 13)\
	VTE_GENERATE_N(TEMPLATE, 12)\
	VTE_GENERATE_N(TEMPLATE, 11)\
	VTE_GENERATE_N(TEMPLATE, 10)\
	VTE_GENERATE_N(TEMPLATE, 9)\
	VTE_GENERATE_N(TEMPLATE, 8)\
	VTE_GENERATE_N(TEMPLATE, 7)\
	VTE_GENERATE_N(TEMPLATE, 6)\
	VTE_GENERATE_N(TEMPLATE, 5)\
	VTE_GENERATE_N(TEMPLATE, 4)\
	VTE_GENERATE_N(TEMPLATE, 3)\
	VTE_GENERATE_N(TEMPLATE, 2)\
	VTE_GENERATE_N(TEMPLATE, 1)
