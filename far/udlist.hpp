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

class UserDefinedListItem
{
	public:
		unsigned int index;
		char *Str;
		UserDefinedListItem():Str(NULL), index(0)
		{
		}
		bool operator==(const UserDefinedListItem &rhs) const;
		int operator<(const UserDefinedListItem &rhs) const;
		const UserDefinedListItem& operator=(const UserDefinedListItem &rhs);
		const UserDefinedListItem& operator=(const char *rhs);
		char *set(const char *Src, unsigned int size);
		~UserDefinedListItem();
};

class UserDefinedList
{
	private:
		TArray<UserDefinedListItem> Array;
		unsigned int CurrentItem;
		BYTE Separator1, Separator2;
		BOOL ProcessBrackets, AddAsterisk, PackAsterisks, Unique, Sort;

	private:
		BOOL CheckSeparators() const; // проверка разделителей на корректность
		void SetDefaultSeparators();
		const char *Skip(const char *Str, int &Length, int &RealLength, BOOL &Error);
		static int __cdecl CmpItems(const UserDefinedListItem **el1,
		                            const UserDefinedListItem **el2);

	private:
		UserDefinedList& operator=(const UserDefinedList& rhs); // чтобы не
		UserDefinedList(const UserDefinedList& rhs); // генерировалось по умолчанию

	public:
		// по умолчанию разделителем считается ';' и ',', а
		// ProcessBrackets=AddAsterisk=PackAsterisks=FALSE
		// Unique=Sort=FALSE
		UserDefinedList();

		// Явно указываются разделители. См. описание SetParameters
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
		BOOL Set(const char *List, BOOL AddToList=FALSE);

		// Добавление к уже существующему списку
		// Фича: если NewItem==NULL, то происходит освобождение занятой ранее
		// памяти
		BOOL AddItem(const char *NewItem)
		{
			return Set(NewItem,TRUE);
		}

		// Вызывать перед началом работы со списком
		void Reset(void);

		// Выдает указатель на очередной элемент списка или NULL
		const char *GetNext(void);

		// Освободить память
		void Free();

		// TRUE, если больше элементов в списке нет
		BOOL IsEmpty();

		// Вернуть количество элементов в списке
		DWORD GetTotal() const { return Array.getSize(); }
};

#endif // __UserDefinedList_HPP
