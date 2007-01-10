#ifndef __HIGHLIGHTFILES_HPP__
#define __HIGHLIGHTFILES_HPP__
/*
hilight.hpp

Files highlighting

*/

#include "CFileMask.hpp"
#include "struct.hpp"

struct HighlightData
{
  string strOriginalMasks;
  CFileMaskW *FMasks;
  /* $ 25.09.2001 IS
       ≈сли TRUE, то маска файлов игнорируетс€ и сравнение идет только по
       атрибутам.
  */
  BOOL IgnoreMask;
  /* IS $ */
  unsigned int IncludeAttr;
  unsigned int ExcludeAttr;
  struct HighlightDataColor Colors;

  void Clear()
  {
    strOriginalMasks=L"";
    FMasks=NULL;
    IgnoreMask=0;
    IncludeAttr=0;
    ExcludeAttr=0;
    memset(&Colors,0,sizeof(Colors));
  }

  HighlightData& operator=(const HighlightData &hdCopy)
  {
    strOriginalMasks=hdCopy.strOriginalMasks;
    FMasks=hdCopy.FMasks;
    IgnoreMask=hdCopy.IgnoreMask;
    IncludeAttr=hdCopy.IncludeAttr;
    ExcludeAttr=hdCopy.ExcludeAttr;
    memcpy(&Colors,&hdCopy.Colors,sizeof(Colors));
    return *this;
  }
};

class VMenu;
struct FileListItem;

class HighlightFiles
{
  private:
    struct HighlightData **HiData;
    int HiDataCount;
    int StartHiDataCount;

  private:
    int EditRecord(int RecPos,int New);
    /* $ 07.07.2000 IS
      ¬ эту функцию € вынес содержимое конструктора, чтобы использовать его
      повторно при восстановлении значений раскраски файлов по умолчанию
    */
    void InitHighlightFiles();
    /* IS $ */
    void ClearData();
    int  DupHighlightData(HighlightData *Data,const wchar_t *Mask,BOOL IgnoreMask,int RecPos);

    const wchar_t *GetMask(int Idx);
    BOOL AddMask(HighlightData *Dest,const wchar_t *Mask,BOOL IgnoreMask,struct HighlightData *Src=NULL);
    void DeleteMask(struct HighlightData *CurHighlightData);
    void FillMenu(VMenu *HiMenu,int MenuPos);

  public:
    HighlightFiles();
    ~HighlightFiles();

  public:
    void GetHiColor(const wchar_t *Path,int Attr,
                    struct HighlightDataColor *Colors);
    void GetHiColor(struct FileListItem **FileItemEx,int FileCount);
    void HiEdit(int MenuPos);

    void SaveHiData();

    static void ReWriteWorkColor(struct HighlightDataColor *Colors=NULL);
};

#endif  // __HIGHLIGHTFILES_HPP__
