#ifndef __HIGHLIGHTFILES_HPP__
#define __HIGHLIGHTFILES_HPP__
/*
hilight.hpp

Files highlighting

*/

/* Revision: 1.03 23.04.2001 $ */

/*
Modify:
  23.04.2001 SVS
    ! КХЕ! Новый вз<ляд на %PATHEXT% - то что редактируем и то, что
      юзаем - разные сущности.
  12.02.2001 SVS
    + Функция ClearData - очистка HiData
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
    struct HighlightData *HiData;
    int HiDataCount;
    int StartHiDataCount;

  private:
    void SaveHiData();
    int EditRecord(int RecPos,int New);
    /* $ 07.07.2000 IS
      В эту функцию я вынес содержимое конструктора, чтобы использовать его
      повторно при восстановлении значений раскраски файлов по умолчанию
    */
    void InitHighlightFiles();
    /* IS $ */
    void ClearData();

    char *GetMask(int Idx);
    BOOL AddMask(struct HighlightData *Dest,char *Mask,struct HighlightData *Src=NULL);
    void DeleteMask(struct HighlightData *CurHighlightData);

  public:
    HighlightFiles();
    ~HighlightFiles();

  public:
    void GetHiColor(char *Path,int Attr,unsigned char &Color,
                    unsigned char &SelColor,unsigned char &CursorColor,
                    unsigned char &CursorSelColor,unsigned char &MarkChar);
    void HiEdit(int MenuPos);
};

#endif	// __HIGHLIGHTFILES_HPP__