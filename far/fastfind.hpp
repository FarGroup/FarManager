#ifndef FASTFIND_HPP_EBAA3EB9_1C5E_4A4C_AE66_BFBA288A324E
#define FASTFIND_HPP_EBAA3EB9_1C5E_4A4C_AE66_BFBA288A324E
#pragma once

/*
fastfind.hpp

Fast Find
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

// Internal:
#include "window.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

class Panel;
class EditControl;

class FastFind final: public window
{
	struct private_tag { explicit private_tag() = default; };

public:
	static fastfind_ptr create(Panel* Owner, const Manager::Key& FirstKey);

	FastFind(private_tag, Panel* Owner, const Manager::Key& FirstKey);

	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD* MouseEvent) override;
	int GetType() const override;
	int GetTypeAndName(string&, string&) override;
	void ResizeConsole() override;

	void Process();
	const Manager::Key& KeyToProcess() const;

private:
	void DisplayObject() override;
	string GetTitle() const override;

	void InitPositionAndSize();
	void init();
	void ProcessName(string_view Src) const;
	void ShowBorder() const;
	void Close(int ExitCode);

	Panel* m_Owner;
	Manager::Key m_FirstKey;
	std::unique_ptr<EditControl> m_FindEdit;
	Manager::Key m_KeyToProcess;
};

#endif // FASTFIND_HPP_EBAA3EB9_1C5E_4A4C_AE66_BFBA288A324E
