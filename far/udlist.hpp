#ifndef __UserDefinedList_HPP__
#define __UserDefinedList_HPP__

/*
udlist.hpp

Список чего-либо, перечисленного через символ-разделитель. Если нужно, чтобы
элемент списка содержал разделитель, то этот элемент следует заключить в
кавычки. Если кроме разделителя ничего больше в строке нет, то считается, что
это не разделитель, а простой символ.

*/

/* Revision: 1.05 11.08.2001 $ */

/*
Modify:
  11.08.2001 IS
    + UDL_FLAGS
    ! SetSeparators -> SetParameters
  01.08.2001 IS
    + GetTotal
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

enum UDL_FLAGS
{
  ULF_ADDASTERISK    =0x00000001, // добавлять '*' к концу элемента списка,
                                  // если он не содержит '?', '*' и '.'
  ULF_PACKASTERISKS  =0x00000002, // вместо "*.*" в список помещать просто "*"
  ULF_PROCESSBRACKETS=0x00000004, // учитывать квадратные скобки при анализе
                                  // строки инициализации
};

class UserDefinedList
{
  private:
    DWORD Total;
    char *Data, *DataEnd, *DataCurrent;
    BYTE Separator1, Separator2;
    BOOL ProcessBrackets, AddAsterisk, PackAsterisks;

  private:
    BOOL CheckSeparators() const; // проверка разделителей на корректность
    void SetDefaultSeparators();
    const char *Skip(const char *Str, int &Length, int &RealLength, BOOL &Error);

  private:
    UserDefinedList& operator=(const UserDefinedList& rhs); // чтобы не
    UserDefinedList(const UserDefinedList& rhs); // генерировалось по умолчанию

  public:
    // по умолчанию разделителем считается ';' и ',', а
    // ProcessBrackets=FALSE, AddAsterisk=FALSE, PackAsterisks=FALSE
    UserDefinedList();

    // Явно указываются разделители. См. описание SetSeparators
    UserDefinedList(BYTE separator1, BYTE separator2, DWORD Flags);
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
    BOOL SetParameters(BYTE Separator1, BYTE Separator2, DWORD Flags);

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

    // Вернуть количество элементов в списке
    DWORD GetTotal () const { return Total; }
};

#endif // __UserDefinedList_HPP
