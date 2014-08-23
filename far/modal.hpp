#pragma once

/*
modal.hpp

привет автодетектор кодировки!
Parent class для модальных объектов
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


#include "frame.hpp"

class Modal: public Frame
{
public:
	Modal();
	virtual ~Modal() {}

	virtual int ReadInput(INPUT_RECORD *GetReadRec = nullptr);
	virtual void SetExitCode(int Code) override;

	bool Done() const;
	void ClearDone();
	int GetExitCode() const;
	void Process();
	void WriteInput(int Key);
	void ProcessInput();
	void SetHelp(const wchar_t *Topic);
	void ShowHelp();

protected:
	void SetDone(void);
	void Close(int Code);

	INPUT_RECORD m_ReadRec;
	string m_HelpTopic;

private:
	bool m_EndLoop;
	int m_ReadKey;
	int m_WriteKey;
};
