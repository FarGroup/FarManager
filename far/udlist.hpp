#ifndef __UserDefinedList_HPP__
#define __UserDefinedList_HPP__

/*
udlist.hpp

Список чего-либо, перечисленного через символ-разделитель. Если нужно, чтобы
элемент списка содержал разделитель, то этот элемент следует заключить в
кавычки.

*/

/* Revision: 1.00 02.06.2001 $ */

/*
Modify:
  02.06.2001 IS
    + Впервые в эфире
*/

#include "farconst.hpp"

class UserDefinedList
{
  private:
    char *Data, *DataEnd, *DataCurrent;
    BYTE Separator;

  private:
    void Free();
    const char *Skip(const char *Str, int &Length, int &RealLength, BOOL &Error);

  private:
    UserDefinedList& operator=(const UserDefinedList& rhs); // чтобы не
    UserDefinedList(const UserDefinedList& rhs); // генерировалось по умолчанию

  public:
    UserDefinedList();               // по умолчанию разделителем считается ';'
    UserDefinedList(BYTE separator); // явно указывается разделитель
    ~UserDefinedList() { Free(); }

  public:
    // Сменить символ-разделитель
    void SetSeparator(BYTE Separator);

    // Инициализирует список. Принимает список, разделенный символом separator.
    // Возвращает FALSE при неудаче.
    // Фича: если List==NULL, то происходит освобождение занятой ранее памяти
    BOOL Set(const char *List);

    // Вызывать перед началом работы со списком
    void Start(void);

    // Выдает указатель на очередной элемент списка или NULL
    const char *GetNext(void);

    // TRUE, если больше элементов в списке нет
    BOOL IsEmpty();
};

#endif // __UserDefinedList_HPP
