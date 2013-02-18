#pragma once

/*
udlist.hpp

—писок чего-либо, перечисленного через символ-разделитель. ≈сли нужно, чтобы
элемент списка содержал разделитель, то этот элемент следует заключить в
кавычки. ≈сли кроме разделител€ ничего больше в строке нет, то считаетс€, что
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


#include "bitflags.hpp"

enum UDL_FLAGS
{
	// вместо "*.*" в список помещать просто "*", вместо "***" в список помещать просто "*"
	ULF_PACKASTERISKS  =0x00000002,
	// учитывать квадратные скобки при анализе строки инициализации
	ULF_PROCESSBRACKETS=0x00000004,
	// убирать дублирующиес€ элементы
	ULF_UNIQUE         =0x00000010,
	// отсортировать (с учетом регистра)
	ULF_SORT           =0x00000020,
	// не удал€ть пробелы
	ULF_NOTRIM         =0x00000040,
	// не раскавычивать
	ULF_NOUNQUOTE      =0x00000080,
};


class UserDefinedListItem
{
public:

	UserDefinedListItem():index(0) {}
	~UserDefinedListItem();
	bool operator==(const UserDefinedListItem &rhs) const;
	int operator<(const UserDefinedListItem &rhs) const;
	const string& Get() const {return Str;}
private:
	string Str;
	size_t index;

	friend class UserDefinedList;
};

class UserDefinedList:NonCopyable
{
	public:
		typedef UserDefinedListItem value_type;

		// по умолчанию разделителем считаетс€ ';' и ',', а
		// ProcessBrackets=AddAsterisk=PackAsterisks=false
		// Unique=Sort=false
		UserDefinedList();

		// явно указываютс€ разделители. —м. описание SetParameters
		UserDefinedList(DWORD Flags, const wchar_t* Separators = nullptr);
		~UserDefinedList() { Free(); }

		// —менить символы-разделитель и разрешить или запретить обработку
		// квадратных скобок.
		// ≈сли разделители не заданы, то восстанавливаютс€ разделители по
		// умолчанию (';' & ',').
		// ¬озвращает false, если один из разделителей €вл€етс€ кавычкой или
		// включена обработка скобок и один из разделителей €вл€етс€ квадратной
		// скобкой.
		bool SetParameters(DWORD Flags, const wchar_t* Separators = nullptr);

		// »нициализирует список. ѕринимает список, разделенный разделител€ми.
		// ¬озвращает false при неудаче.
		bool Set(const string& List, bool AddToList=false);

		// ƒобавление к уже существующему списку
		bool Add(const string& List)
		{
			return Set(List, true);
		}

		std::list<UserDefinedListItem>::iterator begin() {return ItemsList.begin();}
		std::list<UserDefinedListItem>::iterator end() {return ItemsList.end();}
		bool empty() const {return ItemsList.empty();}
		size_t size() const {return ItemsList.size();}

		// ќсвободить пам€ть
		void Free();

	private:
		bool CheckSeparators() const; // проверка разделителей на корректность
		void SetDefaultSeparators();
		const wchar_t *Skip(const wchar_t *Str, int &Length, int &RealLength, bool &Error);

		std::list<UserDefinedListItem> ItemsList;
		string strSeparators;
		BitFlags Flags;
};
