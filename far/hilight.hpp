#ifndef __HIGHLIGHTFILES_HPP__
#define __HIGHLIGHTFILES_HPP__
/*
hilight.hpp

Files highlighting

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/


class HighlightFiles
{
  private:
    void SaveHiData();
    int EditRecord(int RecPos,int New);
    struct HighlightData *HiData;
    int HiDataCount;
    int StartHiDataCount;
  public:
    HighlightFiles();
    ~HighlightFiles();
    void GetHiColor(char *Path,int Attr,unsigned char &Color,
                    unsigned char &SelColor,unsigned char &CursorColor,
                    unsigned char &CursorSelColor,unsigned char &MarkChar);
    void HiEdit(int MenuPos);
};

#endif	// __HIGHLIGHTFILES_HPP__