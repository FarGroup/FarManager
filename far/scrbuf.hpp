#ifndef __SCREENBUF_HPP__
#define __SCREENBUF_HPP__
/*
scrbuf.hpp

Буферизация вывода на экран, весь вывод идет через этот буфер

*/

/* Revision: 1.03 03.03.2002 $ */

/*
Modify:
  03.03.2002 SVS
    + AppliColor()
  23.07.2001 SKV
    + Scroll
  12.05.2001 DJ
    ! глобальный ScrBuf переехал сюда
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class ScreenBuf
{
  private:
    CHAR_INFO *Buf;
    CHAR_INFO *Shadow;
    CHAR_INFO MacroChar;
    HANDLE hScreen;
    int BufX,BufY;
    int Flushed;
    int LockCount;
    int UseShadow;
    int CurX,CurY,CurVisible,CurSize;
    int FlushedCurPos,FlushedCurType;
  public:
    ScreenBuf();
    ~ScreenBuf();

  public:
    void AllocBuf(int X,int Y);
    void Lock();
    void Unlock();
    int  GetLockCount() {return(LockCount);};
    void SetLockCount(int Count) {LockCount=Count;};
    void SetHandle(HANDLE hScreen);
    void ResetShadow();
    void MoveCursor(int X,int Y);
    void GetCursorPos(int& X,int& Y);
    void SetCursorType(int Visible,int Size);
    void GetCursorType(int &Visible,int &Size);

  public:
    void FillBuf();
    void Read(int X1,int Y1,int X2,int Y2,CHAR_INFO *Text);
    void Write(int X,int Y,CHAR_INFO *Text,int TextLength);
    void RestoreMacroChar();

    void AppliColorMask(int X1,int Y1,int X2,int Y2,WORD ColorMask);
    void AppliColor(int X1,int Y1,int X2,int Y2,int ColorMask);
    void FillRect(int X1,int Y1,int X2,int Y2,int Ch,int Color);

    void Scroll(int);
    void Flush();
};

extern ScreenBuf ScrBuf;

#endif  // __SCREENBUF_HPP__
