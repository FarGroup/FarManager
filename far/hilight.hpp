#ifndef __HIGHLIGHTFILES_HPP__
#define __HIGHLIGHTFILES_HPP__
/*
hilight.hpp

Files highlighting

*/

/* Revision: 1.01 07.07.2000 $ */

/*
Modify:
  07.07.2000 IS
    + Новая функция InitHighlightFiles, в которую я вынес содержимое
      конструктора. Нужна, чтобы повторно использовать один и тот же код.
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
    /* $ 07.07.2000 IS
      В эту функцию я вынес содержимое конструктора, чтобы использовать его
      повторно при восстановлении значений раскраски файлов по умолчанию
    */
    void InitHighlightFiles();
    /* IS $ */
  public:
    HighlightFiles();
    ~HighlightFiles();
    void GetHiColor(char *Path,int Attr,unsigned char &Color,
                    unsigned char &SelColor,unsigned char &CursorColor,
                    unsigned char &CursorSelColor,unsigned char &MarkChar);
    void HiEdit(int MenuPos);
};

#endif	// __HIGHLIGHTFILES_HPP__