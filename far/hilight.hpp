#ifndef __HIGHLIGHTFILES_HPP__
#define __HIGHLIGHTFILES_HPP__
/*
hilight.hpp

Files highlighting

*/

#include "CFileMask.hpp"
#include "struct.hpp"
#include "filefilter.hpp"
#include "array.hpp"

class VMenu;
struct FileListItem;

class HighlightFiles
{
  private:
    TPointerArray<FileFilterParams> HiData;
    int StartHiDataCount;

  private:
    void InitHighlightFiles();
    void ClearData();

    void FillMenu(VMenu *HiMenu,int MenuPos);

  public:
    HighlightFiles();
    ~HighlightFiles();

  public:
    void GetHiColor(WIN32_FIND_DATA *fd,struct HighlightDataColor *Colors,bool UseAttrHighlighting=false);
    void GetHiColor(struct FileListItem *FileItem,int FileCount,bool UseAttrHighlighting=false);
    void HiEdit(int MenuPos);

    void SaveHiData();
};

// сама функция в hilight.cpp
char *MkRegKeyHighlightName(char *RegKey);

#endif  // __HIGHLIGHTFILES_HPP__
