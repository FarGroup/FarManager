/*
FileMasksWithExclude.cpp

Класс для работы со сложными масками файлов (учитывается наличие масок
исключения).
*/

/* Revision: 1.02 10.07.2001 $ */

/*
Modify:
  10.07.2001 SVS
    ! В морг для CPP-файлов if/endif
  02.07.2001 IS
    ! Воспользуемся тем, что доступен FileMasksProcessor.Free()
  01.07.2001 IS
    + Впервые в эфире
*/

#include "headers.hpp"
#pragma hdrstop

#include "FileMasksWithExclude.hpp"

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
  if(NULL==masks || !*masks) return FALSE;

  int len=strlen(masks)+2, rc=FALSE;
  char *MasksStr=(char *) malloc(len);
  if(MasksStr)
  {
     rc=TRUE;
     strcpy(MasksStr, masks);
     char *pExclude=strchr(MasksStr,EXCLUDEMASKSEPARATOR);
     if(pExclude)
     {
       *pExclude=0;
       ++pExclude;
       if(strchr(pExclude, EXCLUDEMASKSEPARATOR)) rc=FALSE;
     }

     if(rc)
     {
        rc=Include.Set(*MasksStr?MasksStr:"*",
                       (Flags&FMPF_ADDASTERISK)?FMPF_ADDASTERISK:0);
        if(rc) rc=Exclude.Set(pExclude, 0);
     }
  }

  if(!rc)
    Free();

  if(MasksStr)
    free (MasksStr);

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
