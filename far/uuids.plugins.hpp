#ifndef UUIDS_PLUGINS_HPP_346A7786_E86B_4C38_A02F_68FDAABB07AE
#define UUIDS_PLUGINS_HPP_346A7786_E86B_4C38_A02F_68FDAABB07AE
#pragma once

/*
uuids.plugins.hpp

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

namespace uuids::plugins
{
	constexpr inline auto
		NetworkId  = "773B5051-7C5F-4920-A201-68051C4176A4"_uuid,
		EMenuId    = "742910F1-02ED-4542-851F-DEE37C2E13B2"_uuid,
		ArcliteId  = "65642111-AA69-4B84-B4B8-9249579EC4FA"_uuid,
		LuamacroId = "4EBBEFC8-2084-4B7F-94C0-692CE136894D"_uuid,
		NetBoxId   = "42E4AEB1-A230-44F4-B33C-F195BB654931"_uuid,
		ProcListId = "1E26A927-5135-48C6-88B2-845FB8945484"_uuid,
		TmpPanelId = "B77C964B-E31E-4D4C-8FE5-D6B0C6853E7C"_uuid;
}

// TODO: Use fully qualified names everywhere
inline namespace uuids_inline
{
	using namespace uuids::plugins;
}

#endif // UUIDS_PLUGINS_HPP_346A7786_E86B_4C38_A02F_68FDAABB07AE
