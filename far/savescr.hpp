#ifndef __SAVESCREEN_HPP__
#define __SAVESCREEN_HPP__
/*
savescr.hpp

Сохраняем и восстанавливааем экран кусками и целиком

*/

/* Revision: 1.01 11.05.2001 $ */

/*
Modify:
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
    CHAR_INFO* GetBufferAddress() {return((CHAR_INFO *)ScreenBuf);};
};

#endif	// __SAVESCREEN_HPP__
