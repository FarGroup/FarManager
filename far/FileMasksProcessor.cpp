#ifndef __FileMasksProcessor_CPP
#define __FileMasksProcessor_CPP

/*
FileMasksProcessor.cpp

Класс для работы с простыми масками файлов (не учитывается наличие масок
исключения).
*/

/* Revision: 1.00 01.07.2001 $ */

/*
Modify:
  01.07.2001 IS
    + Впервые в эфире
*/
#include "headers.hpp"
#pragma hdrstop

#include "FileMasksProcessor.hpp"
#include "fn.hpp"

FileMasksProcessor::FileMasksProcessor():BaseFileMask()
{
    Masks.SetSeparators(',',';',TRUE); // разделителем масок является не только
                                       // запятая, но и точка с запятой!
}

void FileMasksProcessor::Free()
{
    Masks.Set(NULL);
}

/*
 Инициализирует список масок. Принимает список, разделенных запятой.
 Возвращает FALSE при неудаче (например, одна из
 длина одной из масок равна 0)
*/

BOOL FileMasksProcessor::Set(const char *masks, DWORD /*Flags*/)
{
  return Masks.Set(masks);
}

BOOL FileMasksProcessor::IsEmpty(void)
{
  Masks.Start();
  return Masks.IsEmpty();
}

/* сравнить имя файла со списком масок
   Возвращает TRUE в случае успеха.
   Путь к файлу в FileName НЕ игнорируется */
BOOL FileMasksProcessor::Compare(const char *FileName)
{
  Masks.Start();
  while(NULL!=(MaskPtr=Masks.GetNext()))
  {
    if (CmpName(MaskPtr,FileName, FALSE))
    // SkipPath=FALSE, т.к. в CFileMask вызывается PointToName
       return TRUE;
  }
  return FALSE;
}

#endif
