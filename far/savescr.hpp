#pragma once

/*
savescr.hpp

Сохраняем и восстанавливааем экран кусками и целиком
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

class SaveScreen
{
		friend class Grabber;
	private:
		FAR_CHAR_INFO* ScreenBuf;
		SHORT CurPosX,CurPosY;
		bool CurVisible;
		DWORD CurSize;
		int X1,Y1,X2,Y2;

		void CleanupBuffer(FAR_CHAR_INFO* Buffer, size_t BufSize);
		int ScreenBufCharCount();
		void CharCopy(FAR_CHAR_INFO* ToBuffer,FAR_CHAR_INFO* FromBuffer,int Count);
		FAR_CHAR_INFO* GetBufferAddress() {return ScreenBuf;};

	public:
		SaveScreen();
		SaveScreen(int X1,int Y1,int X2,int Y2);
		~SaveScreen();

	public:
		void CorrectRealScreenCoord();
		void SaveArea(int X1,int Y1,int X2,int Y2);
		void SaveArea();
		void RestoreArea(int RestoreCursor=TRUE);
		void Discard();
		void AppendArea(SaveScreen *NewArea);
		/*$ 18.05.2001 OT */
		void Resize(int ScrX,int ScrY,DWORD Corner, bool SyncWithConsole);

		void DumpBuffer(const wchar_t *Title);
};
