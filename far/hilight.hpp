#ifndef __HIGHLIGHTFILES_HPP__
#define __HIGHLIGHTFILES_HPP__
/*
hilight.hpp

Files highlighting

*/

#include "CFileMask.hpp"
#include "struct.hpp"
#include "filefilterparams.hpp"
#include "array.hpp"

class VMenu;
struct FileListItem;

class HighlightFiles
{
  private:
    TPointerArray<FileFilterParams> HiData;
    int FirstCount, UpperCount, LowerCount, LastCount;

  private:
    void InitHighlightFiles();
    void ClearData();

    int  MenuPosToRealPos(int MenuPos, int **Count, bool Insert=false);
    void FillMenu(VMenu *HiMenu,int MenuPos);
    void ProcessGroups();

  public:
    HighlightFiles();
    ~HighlightFiles();

  public:
    void GetHiColor(const FAR_FIND_DATA *fd,HighlightDataColor *Colors,bool UseAttrHighlighting=false);
    void GetHiColor(FileListItem **FileItem,int FileCount,bool UseAttrHighlighting=false);
    int  GetGroup(const FAR_FIND_DATA *fd);
    int  GetGroup(const FileListItem *fli);
    void HiEdit(int MenuPos);

    void SaveHiData();
};

#endif  // __HIGHLIGHTFILES_HPP__
