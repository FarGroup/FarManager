#ifndef __SAVESCREEN_HPP__
#define __SAVESCREEN_HPP__
/*
savescr.hpp

Сохраняем и восстанавливааем экран кусками и целиком

*/

class SaveScreen
{
		friend class Grabber;
	private:
		PCHAR_INFO ScreenBuf;
		int CurPosX,CurPosY,CurVisible,CurSize;
		int X1,Y1,X2,Y2;
		int RealScreen;

	private:
		void CleanupBuffer(PCHAR_INFO Buffer, size_t BufSize);
		int ScreenBufCharCount();
		void CharCopy(PCHAR_INFO ToBuffer,PCHAR_INFO FromBuffer,int Count);
		CHAR_INFO* GetBufferAddress() {return ScreenBuf;};

	public:
		SaveScreen();
		SaveScreen(int RealScreen);
		SaveScreen(int X1,int Y1,int X2,int Y2,int RealScreen=FALSE);
		~SaveScreen();

	public:
		void CorrectRealScreenCoord();
		void SaveArea(int X1,int Y1,int X2,int Y2);
		void SaveArea();
		void RestoreArea(int RestoreCursor=TRUE);
		void Discard();
		void AppendArea(SaveScreen *NewArea);
		/*$ 18.05.2001 OT */
		void Resize(int ScrX,int ScrY,DWORD Corner);

		void DumpBuffer(const char *Title);
};

#endif  // __SAVESCREEN_HPP__
