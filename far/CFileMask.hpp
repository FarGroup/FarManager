#ifndef __CFileMask_HPP
#define __CFileMask_HPP

/*
CFileMask.hpp

Основной класс для работы с масками файлов. Использовать нужно именно его.

*/

/* Revision: 1.03 16.03.2006 $ */

#include "BaseFileMask.hpp"

enum FM_FLAGS
{
  FMF_SILENT        = 0x00000001,
  FMF_FORBIDEXCLUDE = 0x00000002,
  FMF_ADDASTERISK   = 0x00000004
};


class CFileMaskW
{
private:
    BaseFileMaskW *FileMask;

public:
    CFileMaskW();
    ~CFileMaskW() { Free(); }

public:
    BOOL Set(const wchar_t *Masks, DWORD Flags);
    BOOL Compare(const wchar_t *Name);
    BOOL IsEmpty(void);
    void Free();

private:
  CFileMaskW& operator=(const CFileMaskW& rhs); /* чтобы не */
  CFileMaskW(const CFileMaskW& rhs); /* генерировалось по умолчанию */

};


#endif // __CFileMask_HPP
