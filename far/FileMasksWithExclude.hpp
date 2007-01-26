#ifndef __FileMasksWithExclude_HPP
#define __FileMasksWithExclude_HPP

/*
FileMasksWithExclude.hpp

Класс для работы со сложными масками файлов (учитывается наличие масок
исключения).
*/

#include "FileMasksProcessor.hpp"

class FileMasksWithExclude:public BaseFileMask
{
private:
    void Free();

public:
    FileMasksWithExclude();
    ~FileMasksWithExclude() {}

public:
    BOOL Set(const char *Masks, DWORD Flags);
    BOOL Compare(const char *Name);
    BOOL IsEmpty(void);

private:
    FileMasksProcessor Include, Exclude;

private:
  FileMasksWithExclude& operator=(const FileMasksWithExclude& rhs); /* чтобы не */
  FileMasksWithExclude(const FileMasksWithExclude& rhs); /* генерировалось по умолчанию */
};

#endif // __FileMasksWithExclude_HPP
