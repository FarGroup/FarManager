#ifndef __CFileMask_HPP
#define __CFileMask_HPP

/*
CFileMask.hpp

Основной класс для работы с масками файлов. Использовать нужно именно его.

*/

/* Revision: 1.00 01.07.2001 $ */

/*
Modify:
  01.07.2001 IS
    + Впервые в эфире
*/

#include "BaseFileMask.hpp"

enum FM_FLAGS
{
  FMF_SILENT        = 0x00000001,
  FMF_FORBIDEXCLUDE = 0x00000002
};

class CFileMask
{
private:
    void Free();

private:
    BaseFileMask *FileMask;

public:
    CFileMask();
    ~CFileMask() { Free(); }

public:
    BOOL Set(const char *Masks, DWORD Flags);
    BOOL Compare(const char *Name);
    BOOL IsEmpty(void);

private:
  CFileMask& operator=(const CFileMask& rhs); /* чтобы не */
  CFileMask(const CFileMask& rhs); /* генерировалось по умолчанию */

};

#endif // __CFileMask_HPP
