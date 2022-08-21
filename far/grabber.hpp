#ifndef GRABBER_HPP_C0A772E7_F4E4_487E_BA04_DE4A8D3AE4B7
#define GRABBER_HPP_C0A772E7_F4E4_487E_BA04_DE4A8D3AE4B7
#pragma once

/*
grabber.hpp

Screen grabber
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
#include "modal.hpp"

// Platform:

// Common:
#include "common/monitored.hpp"

// External:

//----------------------------------------------------------------------------

class Grabber final: public Modal
{
	struct private_tag { explicit private_tag() = default; };

public:
	static grabber_ptr create();
	explicit Grabber(private_tag);

	int GetType() const override { return windowtype_grabber; }
	int GetTypeAndName(string &, string &) override { return windowtype_grabber; }
	void ResizeConsole() override;

private:
	struct grabber_tag { explicit grabber_tag() = default; };

	void DisplayObject() override;
	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	string GetTitle() const override { return {}; }

	void init();
	// (begin, end)
	std::tuple<point&, point&> GetSelection();
	std::tuple<point&, point&> GetSelectionXWise();
	void CopyGrabbedArea(bool Append, bool VerticalBlock);
	void Reset();

	bool empty() const;
	void clear();

	struct
	{
		point Begin;
		point End;
		point Current;
	}
	GArea;
	bool ResetArea{true};
	bool m_VerticalBlock{};
	static inline monitored<bool> m_StreamSelection;
};

bool RunGraber();

#endif // GRABBER_HPP_C0A772E7_F4E4_487E_BA04_DE4A8D3AE4B7
