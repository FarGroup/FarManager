#ifndef __FileMasksProcessor_HPP
#define __FileMasksProcessor_HPP

/*
FileMasksProcessor.hpp

Класс для работы с простыми масками файлов (не учитывается наличие масок
исключения).
*/

/* Revision: 1.01 02.07.2001 $ */

/*
Modify:
  02.07.2001 IS
    ! Метод Free стал public
    + FMPF_ADDASTERISK
  01.07.2001 IS
    + Впервые в эфире
*/

#include "BaseFileMask.hpp"
#include  "udlist.hpp"

enum FMP_FLAGS
{
  FMPF_ADDASTERISK = 0x00000001 // Добавлять '*', если маска не содержит
                                // ни одного из следующих
                                // символов: '*', '?', '.'
};

class FileMasksProcessor:public BaseFileMask
{
public:
    FileMasksProcessor();
    ~FileMasksProcessor() {}

public:
    BOOL Set(const char *Masks, DWORD Flags);
    BOOL Compare(const char *Name);
    BOOL IsEmpty(void);
    void Free();

private:
    UserDefinedList Masks; // список масок файлов
    const char *MaskPtr;   // указатель на текущую маску в списке

private:
  FileMasksProcessor& operator=(const FileMasksProcessor& rhs); /* чтобы не */
  FileMasksProcessor(const FileMasksProcessor& rhs); /* генерировалось по умолчанию */

};

#endif // __FileMasksProcessor_HPP
