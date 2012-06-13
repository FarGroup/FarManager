#pragma once

/*
udlist.hpp

Список чего-либо, перечисленного через символ-разделитель. Если нужно, чтобы
элемент списка содержал разделитель, то этот элемент следует заключить в
кавычки. Если кроме разделителя ничего больше в строке нет, то считается, что
это не разделитель, а простой символ.
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "array.hpp"
#include "bitflags.hpp"

enum UDL_FLAGS
{
	// добавлять '*' к концу элемента списка, если он не содержит '?', '*' и '.'
	ULF_ADDASTERISK    =0x00000001,
	// вместо "*.*" в список помещать просто "*", вместо "***" в список помещать просто "*"
	ULF_PACKASTERISKS  =0x00000002,
	// учитывать квадратные скобки при анализе строки инициализации
	ULF_PROCESSBRACKETS=0x00000004,
	// убирать дублирующиеся элементы
	ULF_UNIQUE         =0x00000010,
	// отсортировать (с учетом регистра)
	ULF_SORT           =0x00000020,
	// не удалять пробелы
	ULF_NOTRIM         =0x00000040,
	// не раскавычивать
	ULF_NOUNQUOTE      =0x00000080,
};


class UserDefinedListItem
{
	public:
		size_t index;
		wchar_t *Str;
		UserDefinedListItem():index(0), Str(nullptr) {}
		bool operator==(const UserDefinedListItem &rhs) const;
		int operator<(const UserDefinedListItem &rhs) const;
		UserDefinedListItem& operator=(const UserDefinedListItem &rhs);
		UserDefinedListItem& operator=(const wchar_t *rhs);
		wchar_t *set(const wchar_t *Src, size_t size);
		~UserDefinedListItem();
};

class UserDefinedList:NonCopyable
{
	public:
		// по умолчанию разделителем считается ';' и ',', а
		// ProcessBrackets=AddAsterisk=PackAsterisks=false
		// Unique=Sort=false
		UserDefinedList();

		// Явно указываются разделители. См. описание SetParameters
		UserDefinedList(DWORD Flags, const wchar_t* Separators = nullptr);
		~UserDefinedList() { Free(); }

		// Сменить символы-разделитель и разрешить или запретить обработку
		// квадратных скобок.
		// Если разделители не заданы, то восстанавливаются разделители по
		// умолчанию (';' & ',').
		// Если AddAsterisk равно true, то к концу элемента списка будет
		// добавляться '*', если этот элемент не содержит '?', '*' и '.'
		// Возвращает false, если один из разделителей является кавычкой или
		// включена обработка скобок и один из разделителей является квадратной
		// скобкой.
		bool SetParameters(DWORD Flags, const wchar_t* Separators = nullptr);

		// Инициализирует список. Принимает список, разделенный разделителями.
		// Возвращает false при неудаче.
		// Фича: если List==nullptr, то происходит освобождение занятой ранее памяти
		bool Set(const wchar_t *List, bool AddToList=false);

		// Добавление к уже существующему списку
		// Фича: если NewItem==nullptr, то происходит освобождение занятой ранее
		// памяти
		bool AddItem(const wchar_t *NewItem)
		{
			return Set(NewItem,true);
		}

		// Вызывать перед началом работы со списком
		void Reset();

		// Выдает указатель на очередной элемент списка или nullptr
		const wchar_t *GetNext();

		// Освободить память
		void Free();

		// true, если больше элементов в списке нет
		bool IsEmpty();

		// Вернуть количество элементов в списке
		size_t GetTotal() const { return Array.getSize(); }

	private:
		bool CheckSeparators() const; // проверка разделителей на корректность
		void SetDefaultSeparators();
		const wchar_t *Skip(const wchar_t *Str, int &Length, int &RealLength, bool &Error);
		static int CmpItems(const UserDefinedListItem **el1, const UserDefinedListItem **el2);
		static void UpdateRemainingItem(UserDefinedListItem *elRemaining, const UserDefinedListItem *elDeleted);

		TArray<UserDefinedListItem> Array;
		size_t CurrentItem;
		string strSeparators;
		BitFlags Flags;
};
