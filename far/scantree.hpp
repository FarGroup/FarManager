#ifndef __SCANTREE_HPP__
#define __SCANTREE_HPP__
/*
scantree.hpp

������������ �������� �������� �, �����������, ������������ ��
������� ���� ������

*/

/* Revision: 1.11 06.06.2006 $ */

#include "farconst.hpp"
#include "bitflags.hpp"
#include "UnicodeString.hpp"
#include "struct.hpp"

enum{
  // ��� ����� ����� ������� ������ (������� 8 ���)
  FSCANTREE_RETUPDIR         = 0x00000001, // = FRS_RETUPDIR
  FSCANTREE_RECUR            = 0x00000002, // = FRS_RECUR
  FSCANTREE_SCANSYMLINK      = 0x00000004, // = FRS_SCANSYMLINK

  // � ������� ����� ������� 8 ���� ���������!
  FSCANTREE_SECONDPASS       = 0x00002000, // ��, ��� ������ ���� ���� SecondPass[]
  FSCANTREE_SECONDDIRNAME    = 0x00004000,
  FSCANTREE_INSIDEJUNCTION   = 0x00008000, // - �� ������ ��������?

  // ����� �� �����, ������� ����� ������������ � 3-� ��������� SetFindPath()
  FSCANTREE_FILESFIRST       = 0x00010000, // ������������ ������� �� ��� �������. ������� �����, ����� ��������
};

struct ScanTreeData{
  BitFlags Flags;
  HANDLE FindHandle;
};

class ScanTree
{
  private:
    BitFlags Flags;
    struct ScanTreeData Data[NM/2];
    int FindHandleCount;

    string strFindPath;
    string strFindMask;

  private:
    void Init();

  public:
    ScanTree(int RetUpDir,int Recurse=1,int ScanJunction=-1);
    ~ScanTree();

  public:
    // 3-� �������� - ����� �� �������� �����
    void SetFindPathW(const wchar_t *Path,const wchar_t *Mask, const DWORD NewScanFlags = FSCANTREE_FILESFIRST);
    int GetNextNameW(FAR_FIND_DATA_EX *fdata,string &strFullName);

    void SkipDir();
    int IsDirSearchDone() {return Flags.Check(FSCANTREE_SECONDDIRNAME);};
    int InsideJunction()   {return Flags.Check(FSCANTREE_INSIDEJUNCTION);};
};

#endif  // __SCANTREE_HPP__
