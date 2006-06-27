/*
FileMasksProcessor.cpp

����� ��� ������ � �������� ������� ������ (�� ����������� ������� �����
����������).
*/

/* Revision: 1.05 16.03.2006 $ */

#include "headers.hpp"
#pragma hdrstop

#include "FileMasksProcessor.hpp"
#include "fn.hpp"


FileMasksProcessorW::FileMasksProcessorW():BaseFileMaskW()
{
}

void FileMasksProcessorW::Free()
{
    Masks.Free();
}

/*
 �������������� ������ �����. ��������� ������, ����������� �������.
 ���������� FALSE ��� ������� (��������, ���� ��
 ����� ����� �� ����� ����� 0)
*/

BOOL FileMasksProcessorW::Set(const wchar_t *masks, DWORD Flags)
{
  // ������������ ����� �������� �� ������ �������, �� � ����� � �������!
  DWORD flags=ULF_PACKASTERISKS|ULF_PROCESSBRACKETS|ULF_SORT|ULF_UNIQUE;
  if(Flags&FMPF_ADDASTERISK) flags|=ULF_ADDASTERISK;
  Masks.SetParameters(L',',L';',flags);
  return Masks.Set(masks);
}

BOOL FileMasksProcessorW::IsEmpty(void)
{
  Masks.Reset();
  return Masks.IsEmpty();
}

/* �������� ��� ����� �� ������� �����
   ���������� TRUE � ������ ������.
   ���� � ����� � FileName �� ������������ */
BOOL FileMasksProcessorW::Compare(const wchar_t *FileName)
{
  Masks.Reset();
  while(NULL!=(MaskPtr=Masks.GetNext()))
  {
    if (CmpNameW(MaskPtr,FileName, FALSE))
    // SkipPath=FALSE, �.�. � CFileMask ���������� PointToName
       return TRUE;
  }
  return FALSE;
}
