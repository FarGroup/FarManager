#ifndef __HIGHLIGHTFILES_HPP__
#define __HIGHLIGHTFILES_HPP__
/*
hilight.hpp

Files highlighting

*/

/* Revision: 1.11 16.12.2005 $ */

#include "CFileMask.hpp"
#include "struct.hpp"

/* $ 06.07.2001 IS
   ������ "�������" ����� ���������� ��������������� �����
*/
struct HighlightData
{
  string strOriginalMasks;
  CFileMaskW *FMasks;
  /* $ 25.09.2001 IS
       ���� TRUE, �� ����� ������ ������������ � ��������� ���� ������ ��
       ���������.
  */
  BOOL IgnoreMask;
  /* IS $ */
  unsigned int IncludeAttr;
  unsigned int ExcludeAttr;
  struct HighlightDataColor Colors;
};
/* IS $ */

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
      � ��� ������� � ����� ���������� ������������, ����� ������������ ���
      �������� ��� �������������� �������� ��������� ������ �� ���������
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
