#ifndef __SAVESCREEN_HPP__
#define __SAVESCREEN_HPP__
/*
savescr.hpp

Сохраняем и восстанавливааем экран кусками и целиком

*/

/* Revision: 1.02 21.05.2001 $ */

/*
Modify:
  21.05.2001 OT
    ! Методы для работы с изменяющимся буфером экрана.
  11.05.2001 OT
    ! Отрисовка Background
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class SaveScreen
{
  private:
    char *ScreenBuf;
    int CurPosX,CurPosY,CurVisible,CurSize;
    int X1,Y1,X2,Y2;
    int RealScreen;
    void CleanupBuffer(char *Buffer, int Height, int Width);
    int ScreenBufSize();
    int ScreenBufSize(int Width,int Height);
    void CharCopy(char *ToBuffer,int ToIndex, char *FromBuffer, int FromIndex, int Count);
    CHAR_INFO* GetBufferAddress() {return((CHAR_INFO *)ScreenBuf);};
    friend class Grabber;
  public:
    SaveScreen();
    SaveScreen(int RealScreen);
    SaveScreen(int X1,int Y1,int X2,int Y2,int RealScreen=FALSE);
    ~SaveScreen();
    void SaveArea(int X1,int Y1,int X2,int Y2);
    void SaveArea();
    void RestoreArea(int RestoreCursor=TRUE);
    void Discard();
    void AppendArea(SaveScreen *NewArea);
    /*$ 18.05.2001 OT */
    void Resize(int ScrX,int ScrY,DWORD Corner);
};

#endif	// __SAVESCREEN_HPP__
