/*
FileMasksWithExclude.cpp

����� ��� ������ �� �������� ������� ������ (����������� ������� �����
����������).
*/

/* Revision: 1.05 16.03.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "FileMasksWithExclude.hpp"
#include "fn.hpp"


FileMasksWithExcludeW::FileMasksWithExcludeW():BaseFileMaskW()
{
}

void FileMasksWithExcludeW::Free()
{
    Include.Free();
    Exclude.Free();
}

/*
 �������������� ������ �����. ��������� ������, ����������� �������.
 ���������� FALSE ��� ������� (��������, ���� ��
 ����� ����� �� ����� ����� 0)
*/

BOOL FileMasksWithExcludeW::Set(const wchar_t *masks, DWORD Flags)
{
  Free();
  if(NULL==masks || !*masks) return FALSE;

  int len=wcslen(masks)+2, rc=FALSE;
  wchar_t *MasksStr=(wchar_t *) xf_malloc(len*sizeof (wchar_t));
  if(MasksStr)
  {
     rc=TRUE;
     wcscpy(MasksStr, masks);
     wchar_t *pExclude=wcschr(MasksStr,EXCLUDEMASKSEPARATOR);
     if(pExclude)
     {
       *pExclude=0;
       ++pExclude;
       if(wcschr(pExclude, EXCLUDEMASKSEPARATOR)) rc=FALSE;
     }

     if(rc)
     {
        rc=Include.Set(*MasksStr?MasksStr:L"*",
                       (Flags&FMPF_ADDASTERISK)?FMPF_ADDASTERISK:0);
        if(rc) rc=Exclude.Set(pExclude, 0);
     }
  }

  if(!rc)
    Free();

  if(MasksStr)
    xf_free (MasksStr);

  return rc;
}

/* �������� ��� ����� �� ������� �����
   ���������� TRUE � ������ ������.
   ���� � ����� � FileName �� ������������ */
BOOL FileMasksWithExcludeW::Compare(const wchar_t *FileName)
{
   return (Include.Compare(FileName) && !Exclude.Compare(FileName));
}

BOOL FileMasksWithExcludeW::IsEmpty(void)
{
  return Include.IsEmpty() && Exclude.IsEmpty();
}
