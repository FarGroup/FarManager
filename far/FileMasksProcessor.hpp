#ifndef __FileMasksProcessor_HPP
#define __FileMasksProcessor_HPP

/*
FileMasksProcessor.hpp

Класс для работы с простыми масками файлов (не учитывается наличие масок
исключения).
*/

/* Revision: 1.00 01.07.2001 $ */

/*
Modify:
  01.07.2001 IS
    + Впервые в эфире
*/

#include "BaseFileMask.hpp"
#include  "udlist.hpp"

class FileMasksProcessor:public BaseFileMask
{
private:
    void Free();

public:
    FileMasksProcessor();
    ~FileMasksProcessor() { Free(); }

public:
    BOOL Set(const char *Masks, DWORD Flags);
    BOOL Compare(const char *Name);
    BOOL IsEmpty(void);

private:
    UserDefinedList Masks; // список масок файлов
    const char *MaskPtr;   // указатель на текущую маску в списке

private:
  FileMasksProcessor& operator=(const FileMasksProcessor& rhs); /* чтобы не */
  FileMasksProcessor(const FileMasksProcessor& rhs); /* генерировалось по умолчанию */

};

#endif // __FileMasksProcessor_HPP
