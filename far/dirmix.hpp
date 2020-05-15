#ifndef DIRMIX_HPP_7386031B_A22B_4851_8BC6_24E90C9798D5
#define DIRMIX_HPP_7386031B_A22B_4851_8BC6_24E90C9798D5
#pragma once

/*
dirmix.hpp

Misc functions for working with directories
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

// External:

//----------------------------------------------------------------------------

enum TESTFOLDERCONST  // for TestFolder()
{
	TSTFLD_ERROR     = -2,
	TSTFLD_NOTACCESS = -1,
	TSTFLD_NOTFOUND  =  0,
	TSTFLD_EMPTY     =  1,
	TSTFLD_NOTEMPTY  =  2,
};

void set_drive_env_curdir(string_view Directory);

/* $ 15.02.2002 IS
   Установка нужного диска и каталога и установление соответствующей переменной
   окружения.
*/
bool FarChDir(string_view NewDir);

int TestFolder(string_view Path);
bool CheckShortcutFolder(string& TestPath, bool TryClosest, bool Silent);

void CreatePath(string_view InputPath, bool Simple = false);

#endif // DIRMIX_HPP_7386031B_A22B_4851_8BC6_24E90C9798D5
