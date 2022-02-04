#ifndef FILETYPE_HPP_E08E6BC3_545B_4343_9D79_A72830AC30F0
#define FILETYPE_HPP_E08E6BC3_545B_4343_9D79_A72830AC30F0
#pragma once

/*
filetype.hpp

Работа с ассоциациями файлов
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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
#include "common/function_ref.hpp"

// External:

//----------------------------------------------------------------------------

// Работа с ассоциациями файлов
enum FILETYPE_MODE
{
	FILETYPE_EXEC,       // Enter
	FILETYPE_ALTEXEC,    // Ctrl-PgDn
	FILETYPE_VIEW,       // F3
	FILETYPE_ALTVIEW,    // Alt-F3
	FILETYPE_EDIT,       // F4
	FILETYPE_ALTEDIT     // Alt-F4
};

/*
 * Convert pressed key into FILETYPE_MODE operation, if possible.
 * Won't change 'mode' if key is unknown
 * return true if succeed.
 */
bool GetFiletypeOpenMode(int keyPressed, FILETYPE_MODE& mode, bool& shouldForceInternal);

bool ProcessLocalFileTypes(string_view Name, string_view ShortName, FILETYPE_MODE Mode, bool AlwaysWaitFinish, string_view CurrentDirectory, bool AddToHistory = true, bool RunAs = false, function_ref<void(struct execute_info&)> Launcher = nullptr);
void ProcessExternal(string_view Command, string_view Name, string_view ShortName, bool AlwaysWaitFinish, string_view CurrentDirectory);
void EditFileTypes();

#endif // FILETYPE_HPP_E08E6BC3_545B_4343_9D79_A72830AC30F0
