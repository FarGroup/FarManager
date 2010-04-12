/*
FileMasksProcessor.cpp

Класс для работы с простыми масками файлов (не учитывается наличие масок
исключения).
*/

#include "headers.hpp"
#pragma hdrstop

#include "FileMasksProcessor.hpp"
#include "fn.hpp"

FileMasksProcessor::FileMasksProcessor():BaseFileMask()
{
}

void FileMasksProcessor::Free()
{
	Masks.Free();
}

/*
 Инициализирует список масок. Принимает список, разделенных запятой.
 Возвращает FALSE при неудаче (например, одна из
 длина одной из масок равна 0)
*/

BOOL FileMasksProcessor::Set(const char *masks, DWORD Flags)
{
	// разделителем масок является не только запятая, но и точка с запятой!
	DWORD flags=ULF_PACKASTERISKS|ULF_PROCESSBRACKETS|ULF_SORT|ULF_UNIQUE;

	if (Flags&FMPF_ADDASTERISK) flags|=ULF_ADDASTERISK;

	Masks.SetParameters(',',';',flags);
	return Masks.Set(masks);
}

BOOL FileMasksProcessor::IsEmpty(void)
{
	Masks.Reset();
	return Masks.IsEmpty();
}

/* сравнить имя файла со списком масок
   Возвращает TRUE в случае успеха.
   Путь к файлу в FileName НЕ игнорируется */
BOOL FileMasksProcessor::Compare(const char *FileName)
{
	Masks.Reset();

	while (NULL!=(MaskPtr=Masks.GetNext()))
	{
		if (CmpName(MaskPtr,FileName, FALSE))
			// SkipPath=FALSE, т.к. в CFileMask вызывается PointToName
			return TRUE;
	}

	return FALSE;
}
