/*
FileMasksWithExclude.cpp

Класс для работы со сложными масками файлов (учитывается наличие масок
исключения).
*/

#include "headers.hpp"
#pragma hdrstop

#include "FileMasksWithExclude.hpp"
#include "fn.hpp"

FileMasksWithExclude::FileMasksWithExclude():BaseFileMask()
{
}

void FileMasksWithExclude::Free()
{
	Include.Free();
	Exclude.Free();
}

/*
 Инициализирует список масок. Принимает список, разделенных запятой.
 Возвращает FALSE при неудаче (например, одна из
 длина одной из масок равна 0)
*/

BOOL FileMasksWithExclude::Set(const char *masks, DWORD Flags)
{
	Free();

	if (NULL==masks || !*masks) return FALSE;

	int len=(int)strlen(masks)+2, rc=FALSE;
	char *MasksStr=(char *) xf_malloc(len);

	if (MasksStr)
	{
		rc=TRUE;
		strcpy(MasksStr, masks);
		char *pExclude=strchr(MasksStr,EXCLUDEMASKSEPARATOR);

		if (pExclude)
		{
			*pExclude=0;
			++pExclude;

			if (strchr(pExclude, EXCLUDEMASKSEPARATOR)) rc=FALSE;
		}

		if (rc)
		{
			rc=Include.Set(*MasksStr?MasksStr:"*",
			               (Flags&FMPF_ADDASTERISK)?FMPF_ADDASTERISK:0);

			if (rc) rc=Exclude.Set(pExclude, 0);
		}
	}

	if (!rc)
		Free();

	if (MasksStr)
		xf_free(MasksStr);

	return rc;
}

/* сравнить имя файла со списком масок
   Возвращает TRUE в случае успеха.
   Путь к файлу в FileName НЕ игнорируется */
BOOL FileMasksWithExclude::Compare(const char *FileName)
{
	return (Include.Compare(FileName) && !Exclude.Compare(FileName));
}

BOOL FileMasksWithExclude::IsEmpty(void)
{
	return Include.IsEmpty() && Exclude.IsEmpty();
}
