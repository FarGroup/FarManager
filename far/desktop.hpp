#ifndef DESKTOP_HPP_16E84F3B_443F_487F_A5E6_FC6432462DB5
#define DESKTOP_HPP_16E84F3B_443F_487F_A5E6_FC6432462DB5
#pragma once

/*
desktop.hpp


*/
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

#include "window.hpp"

class desktop: public window
{
	struct private_tag {};

public:
	static desktop_ptr create();
	desktop(private_tag);

	virtual int GetType() const override { return windowtype_desktop; }
	virtual int GetTypeAndName(string& Type, string& Name) override { Type = GetTitle();  return GetType(); }
	virtual void ResizeConsole() override;
	virtual bool ProcessKey(const Manager::Key& Key) override;

	void TakeSnapshot();

private:
	virtual string GetTitle() const override { return L"Desktop"s; } // TODO: localization
	virtual void DisplayObject() override;

	std::unique_ptr<SaveScreen> m_Background;
};

#endif // DESKTOP_HPP_16E84F3B_443F_487F_A5E6_FC6432462DB5
