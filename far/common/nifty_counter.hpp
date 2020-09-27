#ifndef NIFTY_COUNTER_HPP_81EED24A_897B_4E3E_A23D_4117272E29D9
#define NIFTY_COUNTER_HPP_81EED24A_897B_4E3E_A23D_4117272E29D9
#pragma once

/*
nifty_counter.hpp
*/
/*
Copyright © 2018 Far Group
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

namespace nifty_counter
{
	template<typename type>
	using buffer = std::aligned_storage_t<sizeof(type), alignof(type)>;
}

#define NIFTY_DECLARE(Type, Instance)\
namespace Instance##_nifty_objects\
{\
	static struct initialiser\
	{\
		initialiser();\
		~initialiser();\
	}\
	Initialiser;\
}\
\
extern Type& Instance


#define NIFTY_DEFINE(Type, Instance)\
namespace Instance##_nifty_objects\
{\
	static int InitCounter;\
	static ::nifty_counter::buffer<Type> InitBuffer;\
\
	initialiser::initialiser()\
	{\
		if (!InitCounter++)\
			placement::construct(Instance);\
	}\
\
	initialiser::~initialiser()\
	{\
		if (!--InitCounter)\
			placement::destruct(Instance);\
	}\
}\
\
Type& Instance = reinterpret_cast<Type&>(Instance##_nifty_objects::InitBuffer)

#endif // NIFTY_COUNTER_HPP_81EED24A_897B_4E3E_A23D_4117272E29D9
