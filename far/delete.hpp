#pragma once

/*
delete.hpp

Удаление файлов
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

enum DIRDELTYPE:int;
enum DEL_RESULT:int;

class ShellDelete
{
public:
	ShellDelete(class Panel *SrcPanel, bool Wipe);

private:
	DEL_RESULT AskDeleteReadOnly(const string& Name, DWORD Attr, bool Wipe);
	DEL_RESULT ShellRemoveFile(const string& Name, bool Wipe, int TotalPercent, class ConsoleTitle* DeleteTitle);
	DEL_RESULT ERemoveDirectory(const string& Name, DIRDELTYPE Type);
	bool RemoveToRecycleBin(const string& Name);

	int ReadOnlyDeleteMode;
	int SkipMode;
	int SkipWipeMode;
	int SkipFoldersMode;
	unsigned ProcessedItems;
};

void DeleteDirTree(const string& Dir);
bool DeleteFileWithFolder(const string& FileName);
