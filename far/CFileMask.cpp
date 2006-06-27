/*
CFileMask.cpp

�������� ����� ��� ������ � ������� ������. ������������ ����� ������ ���.

*/

/* Revision: 1.05 16.03.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "CFileMask.hpp"
#include "FileMasksProcessor.hpp"
#include "FileMasksWithExclude.hpp"
#include "fn.hpp"
#include "lang.hpp"

const int EXCLUDEMASKSEPARATOR=0x7C; // '|'

////////////////////////

CFileMaskW::CFileMaskW()
{
    FileMask=NULL;
}

void CFileMaskW::Free()
{
    if(FileMask)
       delete FileMask;
    FileMask=NULL;
}

/*
 �������������� ������ �����. ��������� ������, ����������� ������� ��� ������
 � �������. ����������� ��������� ����� ����������, ������� �� �� ��������
 �������� '|' ���������� FALSE ��� ������� (��������, ���� �� ����� ����� ��
 ����� ����� 0).
*/

BOOL CFileMaskW::Set(const wchar_t *Masks, DWORD Flags)
{
  Free();
  BOOL rc=FALSE;
  int Silent=Flags & FMF_SILENT;
  DWORD flags=0;
  if(Flags & FMF_ADDASTERISK) flags|=FMPF_ADDASTERISK;
  if (Masks && *Masks)
  {
    if(wcschr(Masks, EXCLUDEMASKSEPARATOR))
    {
      if(!(Flags&FMF_FORBIDEXCLUDE))
        FileMask=new FileMasksWithExcludeW;
    }
    else
      FileMask=new FileMasksProcessorW;

    if(FileMask)
       rc=FileMask->Set(Masks, flags);

    if(!rc)
      Free();
  }

  if(!Silent && !rc)
    MessageW(MSG_DOWN|MSG_WARNING,1,UMSG(MWarning),UMSG(MIncorrectMask), UMSG(MOk));

  return rc;
}

// ���������� TRUE, ���� ������ ����� ������
BOOL CFileMaskW::IsEmpty(void)
{
  return FileMask?FileMask->IsEmpty():TRUE;
}

/* �������� ��� ����� �� ������� �����
   ���������� TRUE � ������ ������.
   ���� � ����� ����� ������������.
*/
BOOL CFileMaskW::Compare(const wchar_t *FileName)
{
  return FileMask?FileMask->Compare(PointToNameW((wchar_t*)FileName)):FALSE;
}
