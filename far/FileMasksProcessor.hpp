#ifndef __FileMasksProcessor_HPP
#define __FileMasksProcessor_HPP

/*
FileMasksProcessor.hpp

Класс для работы с простыми масками файлов (не учитывается наличие масок
исключения).
*/

#include "BaseFileMask.hpp"
#include  "udlist.hpp"

enum FMP_FLAGS
{
  FMPF_ADDASTERISK = 0x00000001 // Добавлять '*', если маска не содержит
                                // ни одного из следующих
                                // символов: '*', '?', '.'
};

class FileMasksProcessorW:public BaseFileMaskW
{
public:
    FileMasksProcessorW();
    virtual ~FileMasksProcessorW() {}

public:
    virtual BOOL Set(const wchar_t *Masks, DWORD Flags);
    virtual BOOL Compare(const wchar_t *Name);
    virtual BOOL IsEmpty(void);
    void Free();

private:
    UserDefinedListW Masks; // список масок файлов
    const wchar_t *MaskPtr;   // указатель на текущую маску в списке

private:
  FileMasksProcessorW& operator=(const FileMasksProcessorW& rhs); /* чтобы не */
  FileMasksProcessorW(const FileMasksProcessorW& rhs); /* генерировалось по умолчанию */

};

#endif // __FileMasksProcessor_HPP
