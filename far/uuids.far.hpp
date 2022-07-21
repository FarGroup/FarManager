﻿#ifndef UUIDS_FAR_HPP_49C263EE_12A1_48FD_BA02_52CCE8950C28
#define UUIDS_FAR_HPP_49C263EE_12A1_48FD_BA02_52CCE8950C28
#pragma once

/*
uuids.far.hpp

Far UUIDs
*/
/*
Copyright © 2010 Far Group
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
#include "common/uuid.hpp"

// External:

//----------------------------------------------------------------------------

namespace uuids::far
{
	// It looks like in VS2022 (at least 19.32, Release/x86) this symbol is merged by COMDAT folding (/OPT:ICF)
	// with something from libvcruntime::wcschr of the same size, but a different alignment.
	// Depending on which symbol is discarded, wcschr may or may not crash when accessing it.
	// This manual alignment should at least make them compatible.
	alignas(16)
	constexpr inline auto
		FarUuid = "00000000-0000-0000-0000-000000000000"_uuid;
}

// TODO: Use fully qualified names everywhere
inline namespace uuids_inline
{
	using namespace uuids::far;
}

#endif // UUIDS_FAR_HPP_49C263EE_12A1_48FD_BA02_52CCE8950C28
