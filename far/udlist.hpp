#ifndef __UserDefinedList_HPP__
#define __UserDefinedList_HPP__

/*
udlist.hpp

Список чего-либо, перечисленного через символ-разделитель. Если нужно, чтобы
элемент списка содержал разделитель, то этот элемент следует заключить в
кавычки. Если кроме разделителя ничего больше в строке нет, то считается, что
это не разделитель, а простой символ.

*/

#include "farconst.hpp"
#include "array.hpp"

enum UDL_FLAGS
{
  ULF_ADDASTERISK    =0x00000001, // добавлять '*' к концу элемента списка,
                                  // если он не содержит '?', '*' и '.'
  ULF_PACKASTERISKS  =0x00000002, // вместо "*.*" в список помещать просто "*"
                                  // вместо "***" в список помещать просто "*"
  ULF_PROCESSBRACKETS=0x00000004, // учитывать квадратные скобки при анализе
                                  // строки инициализации
  ULF_UNIQUE         =0x00000010, // убирать дублирующиеся элементы
  ULF_SORT           =0x00000020, // отсортировать (с учетом регистра)
};


class UserDefinedListItemW
{
  public:
   unsigned int index;
   wchar_t *Str;
   UserDefinedListItemW():Str(NULL), index(0)
   {
   }
   bool operator==(const UserDefinedListItemW &rhs) const;
   int operator<(const UserDefinedListItemW &rhs) const;
   const UserDefinedListItemW& operator=(const UserDefinedListItemW &rhs);
   const UserDefinedListItemW& operator=(const wchar_t *rhs);
   wchar_t *set(const wchar_t *Src, unsigned int size);
   ~UserDefinedListItemW();
};

class UserDefinedListW
{
  private:
    TArray<UserDefinedListItemW> Array;
    unsigned int CurrentItem;
    WORD Separator1, Separator2;
    BOOL ProcessBrackets, AddAsterisk, PackAsterisks, Unique, Sort;

  private:
    BOOL CheckSeparators() const; // проверка разделителей на корректность
    void SetDefaultSeparators();
    const wchar_t *Skip(const wchar_t *Str, int &Length, int &RealLength, BOOL &Error);
    static int __cdecl CmpItems(const UserDefinedListItemW **el1,
      const UserDefinedListItemW **el2);

  private:
    UserDefinedListW& operator=(const UserDefinedListW& rhs); // чтобы не
    UserDefinedListW(const UserDefinedListW& rhs); // генерировалось по умолчанию

  public:
    // по умолчанию разделителем считается ';' и ',', а
    // ProcessBrackets=AddAsterisk=PackAsterisks=FALSE
    // Unique=Sort=FALSE
    UserDefinedListW();

    // Явно указываются разделители. См. описание SetParameters
    UserDefinedListW(WORD separator1, WORD separator2, DWORD Flags);
    ~UserDefinedListW() { Free(); }

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
    BOOL SetParameters(WORD Separator1, WORD Separator2, DWORD Flags);

    // Инициализирует список. Принимает список, разделенный разделителями.
    // Возвращает FALSE при неудаче.
    // Фича: если List==NULL, то происходит освобождение занятой ранее памяти
    BOOL Set(const wchar_t *List, BOOL AddToList=FALSE);

    // Добавление к уже существующему списку
    // Фича: если NewItem==NULL, то происходит освобождение занятой ранее
    // памяти
    BOOL AddItem(const wchar_t *NewItem)
    {
      return Set(NewItem,TRUE);
    }

    // Вызывать перед началом работы со списком
    void Reset(void);

    // Выдает указатель на очередной элемент списка или NULL
    const wchar_t *GetNext(void);

    // Освободить память
    void Free();

    // TRUE, если больше элементов в списке нет
    BOOL IsEmpty();

    // Вернуть количество элементов в списке
    DWORD GetTotal () const { return Array.getSize(); }
};

#endif // __UserDefinedList_HPP
