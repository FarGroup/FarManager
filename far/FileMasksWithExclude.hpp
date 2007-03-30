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
	virtual ~FileMasksWithExclude() {}

public:
	virtual BOOL Set(const wchar_t *Masks, DWORD Flags);
	virtual BOOL Compare(const wchar_t *Name);
	virtual BOOL IsEmpty(void);

private:
	FileMasksProcessor Include, Exclude;

private:
	FileMasksWithExclude& operator=(const FileMasksWithExclude& rhs); /* чтобы не */
	FileMasksWithExclude(const FileMasksWithExclude& rhs); /* генерировалось по умолчанию */
};

#endif // __FileMasksWithExclude_HPP
