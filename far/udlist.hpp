#ifndef __UserDefinedList_HPP__
#define __UserDefinedList_HPP__

/*
udlist.hpp

Список чего-либо, перечисленного через символ-разделитель. Если нужно, чтобы
элемент списка содержал разделитель, то этот элемент следует заключить в
кавычки. Если кроме разделителя ничего больше в строке нет, то считается, что
это не разделитель, а простой символ.

*/

/* Revision: 1.03 02.07.2001 $ */

/*
Modify:
  02.07.2001 IS
    + AddAsterisk
    ! Метод Free стал public
  12.06.2001 IS
    + Добавлено связанное с обработкой квадратных скобок
    + Функция для проверки корректности разделителей.
  09.06.2001 IS
    + Переписано с учетом второго разделителя. Теперь разделителей два. По
      умолчанию они равны ';' и ','
  02.06.2001 IS
    + Впервые в эфире
*/

#include "farconst.hpp"

class UserDefinedList
{
  private:
    char *Data, *DataEnd, *DataCurrent;
    BYTE Separator1, Separator2;
    BOOL ProcessBrackets, AddAsterisk;

  private:
    BOOL CheckSeparators() const; // проверка разделителей на корректность
    void SetDefaultSeparators();
    const char *Skip(const char *Str, int &Length, int &RealLength, BOOL &Error);

  private:
    UserDefinedList& operator=(const UserDefinedList& rhs); // чтобы не
    UserDefinedList(const UserDefinedList& rhs); // генерировалось по умолчанию

  public:
    // по умолчанию разделителем считается ';' и ',', а
    // ProcessBrackets=FALSE, AddAsterisk=FALSE
    UserDefinedList();

    // Явно указываются разделители. См. описание SetSeparators
    UserDefinedList(BYTE separator1, BYTE separator2,
                    BOOL ProcessBrackets, BOOL AddAsterisk);
    ~UserDefinedList() { Free(); }

  public:
    // Сменить символы-разделитель и разрешить или запретить обработку
    // квадратных скобок.
    // Если один из Separator* равен 0x00, то он игнорируется при компиляции
    // (т.е. в Set)
    // Если оба разделителя равны 0x00, то восстанавливаются разделители по
    // умолчанию (';' & ',').
    // Если AddAsterisk равно TRUE, то к концу элемента списка будет
    // добавляться '*', если этот элемент не содержит '?', '*' и '.'
    // Возвращает FALSE, если один из разделителей является кавычкой или
    // включена обработка скобок и один из разделителей является квадратной
    // скобкой.
    BOOL SetSeparators(BYTE Separator1, BYTE Separator2,
                       BOOL ProcessBrackets, BOOL AddAsterisk);

    // Инициализирует список. Принимает список, разделенный разделителями.
    // Возвращает FALSE при неудаче.
    // Фича: если List==NULL, то происходит освобождение занятой ранее памяти
    BOOL Set(const char *List);

    // Вызывать перед началом работы со списком
    void Start(void);

    // Выдает указатель на очередной элемент списка или NULL
    const char *GetNext(void);

    // Освободить занятую память
    void Free();

    // TRUE, если больше элементов в списке нет
    BOOL IsEmpty();
};

#endif // __UserDefinedList_HPP
