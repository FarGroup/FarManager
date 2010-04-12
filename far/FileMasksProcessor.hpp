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

class FileMasksProcessor:public BaseFileMask
{
	public:
		FileMasksProcessor();
		virtual ~FileMasksProcessor() {}

	public:
		virtual BOOL Set(const char *Masks, DWORD Flags);
		virtual BOOL Compare(const char *Name);
		virtual BOOL IsEmpty(void);
		void Free();

	private:
		UserDefinedList Masks; // список масок файлов
		const char *MaskPtr;   // указатель на текущую маску в списке

	private:
		FileMasksProcessor& operator=(const FileMasksProcessor& rhs); /* чтобы не */
		FileMasksProcessor(const FileMasksProcessor& rhs); /* генерировалось по умолчанию */

};

#endif // __FileMasksProcessor_HPP
