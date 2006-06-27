#ifndef __FILEFILTER_HPP__
#define __FILEFILTER_HPP__
/*
filefilter.hpp

�������� ������

*/

/* Revision: 1.08 06.06.2006 $ */

#include "plugin.hpp"
#include "struct.hpp"
#include "CFileMask.hpp"

#define DATE_COUNT  3

class FileFilter
{
  friend long WINAPI FilterDlgProc(HANDLE hDlg, int Msg,int Param1,long Param2);

  private:

    const wchar_t *FmtMask1;               // ����� ���� ��� �������� DD.MM.YYYY � MM.DD.YYYY
    const wchar_t *FmtMask2;               // ����� ���� ��� ������� YYYY.MM.DD
    const wchar_t *FmtMask3;               // ����� �������
    const wchar_t *DigitMask;              // ����� ��� ����� �������� �����
    const wchar_t *FilterMasksHistoryName; // ������� ��� ����� ������

    FarList SizeList;                   // ���� ��� ����������: ����� - ���������
    FarListItem *TableItemSize;
    FarList DateList;                   // ���� ��� ���������� ������� �����
    FarListItem *TableItemDate;

    // ����� ��� ������� ���������
    string strDateMask, strDateStrAfter, strDateStrBefore;
    string strTimeMask, strTimeStrAfter, strTimeStrBefore;

    int DateSeparator;                  // ����������� ����
    int TimeSeparator;                  // ����������� �������
    int DateFormat;                     // ������ ���� � �������

    FilterParams FF;                    // ���������� ��������� ���������� ������������
                                        // ��� ����, ����� �� ������ �������� Opt.OpFilter
                                        // �� "����".

    CFileMaskW FilterMask;               // ��������� ���������������� �����.

  private:

    // ���������� ���������
    static long WINAPI FilterDlgProc(HANDLE hDlg,int Msg,int Param1,long Param2);
    void GetFileDateAndTime(const wchar_t *Src,unsigned *Dst,int Separator);

    // ������������� ��������� ����� ���� � ������� � FILETIME
    FILETIME &StrToDateTime(const wchar_t *CDate,const wchar_t *CTime,FILETIME &ft);

  public:

    FileFilter(int DisableDirAttr=FALSE);
    ~FileFilter();

    // �������� ������� ��������� �������.
    FilterParams GetParams(){return FF;};

    // ������ ����� ���������� "�������" � ������ ��� �����������:
    // �������� �� ���� fd ��� ������� �������������� �������.
    // ���������� TRUE  - ��������;
    //            FALSE - �� ��������.
    int FileInFilter(const FAR_FIND_DATA_EX *fd);
    int FileInFilter(const FAR_FIND_DATA *fd);

    // ������ ����� ���������� ��� ��������� ���������� �������.
    void Configure();
};

#endif  // __FINDFILES_HPP__
