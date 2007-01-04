#ifndef __FileMasksWithExclude_HPP
#define __FileMasksWithExclude_HPP

/*
FileMasksWithExclude.hpp

Класс для работы со сложными масками файлов (учитывается наличие масок
исключения).
*/

#include "FileMasksProcessor.hpp"

class FileMasksWithExcludeW:public BaseFileMaskW
{
private:
    void Free();

public:
    FileMasksWithExcludeW();
    ~FileMasksWithExcludeW() {}

public:
    BOOL Set(const wchar_t *Masks, DWORD Flags);
    BOOL Compare(const wchar_t *Name);
    BOOL IsEmpty(void);

private:
    FileMasksProcessorW Include, Exclude;

private:
  FileMasksWithExcludeW& operator=(const FileMasksWithExcludeW& rhs); /* чтобы не */
  FileMasksWithExcludeW(const FileMasksWithExcludeW& rhs); /* генерировалось по умолчанию */
};

#endif // __FileMasksWithExclude_HPP
