#ifndef __HIGHLIGHTFILES_HPP__
#define __HIGHLIGHTFILES_HPP__
/*
hilight.hpp

Files highlighting

*/

/* Revision: 1.06 05.09.2001 $ */

/*
Modify:
  12.07.2001 SVS
    ! Вместо полей Color* в структе HighlightData используется
      структура HighlightDataColor
    ! Естественно, сокращено количество параметров у GetHiColor()
    + функция ReWriteWorkColor - задел на будущее
  12.07.2001 SVS
    + Функция дублирования - DupHighlightData()
  06.07.2001 IS
    + В HighlightData используем вместо Masks (рабочая маска) соответствующий
      класс.
    ! В HighlightData OriginalMasks стал самым первым членом.
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

#include "CFileMask.hpp"
#include "struct.hpp"

/* $ 06.07.2001 IS
   вместо "рабочей" маски используем соответствующий класс
*/
struct HighlightData
{
  char *OriginalMasks;
  CFileMask *FMasks;
  unsigned int IncludeAttr;
  unsigned int ExcludeAttr;
  struct HighlightDataColor Colors;
};
/* IS $ */

class VMenu;

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
    int  DupHighlightData(struct HighlightData *Data,char *Mask,int RecPos);

    char *GetMask(int Idx);
    BOOL AddMask(struct HighlightData *Dest,char *Mask,struct HighlightData *Src=NULL);
    void DeleteMask(struct HighlightData *CurHighlightData);
    void FillMenu(VMenu *HiMenu,int MenuPos);

  public:
    HighlightFiles();
    ~HighlightFiles();

  public:
    void GetHiColor(char *Path,int Attr,
                    struct HighlightDataColor *Colors);
    void HiEdit(int MenuPos);

    static void ReWriteWorkColor(struct HighlightDataColor *Colors=NULL);
};

// сама функция в hilight.cpp
char *MkRegKeyHighlightName(char *RegKey);

#endif  // __HIGHLIGHTFILES_HPP__
