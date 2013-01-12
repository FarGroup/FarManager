/*
udlist.cpp

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

#include "headers.hpp"
#pragma hdrstop

#include "udlist.hpp"

UserDefinedListItem::~UserDefinedListItem()
{
}

bool UserDefinedListItem::operator==(const UserDefinedListItem &rhs) const
{
	size_t pos;
	return Str.PosI(pos, rhs.Str);
}

int UserDefinedListItem::operator<(const UserDefinedListItem &rhs) const
{
	if (!Str)
		return 1;
	else if (!rhs.Str)
		return -1;
	else
		return StrCmpI(Str, rhs.Str)<0;
}

bool UDItemLess(const UserDefinedListItem& a, const UserDefinedListItem& b)
{
	return a.index < b.index;
}

UserDefinedList::UserDefinedList()
{
	Reset();
	SetParameters(0, nullptr);
}

UserDefinedList::UserDefinedList(DWORD Flags, const wchar_t* Separators)
{
	Reset();
	SetParameters(Flags, Separators);
}

void UserDefinedList::SetDefaultSeparators()
{
	strSeparators=L";,";
}

bool UserDefinedList::CheckSeparators() const
{
	return !(
		(!Flags.Check(ULF_NOUNQUOTE) && strSeparators.Contains(L'\"')) ||
		(Flags.Check(ULF_PROCESSBRACKETS) && strSeparators.ContainsAny(L"[]"))
			);
}

bool UserDefinedList::SetParameters(DWORD Flags, const wchar_t* Separators)
{
	Free();
	this->Flags.Set(Flags);
	if (Separators && *Separators)
	{
		strSeparators = Separators;
	}
	else
	{
		SetDefaultSeparators();
	}
	return CheckSeparators();
}

void UserDefinedList::Free()
{
	ItemsList.clear();
	Reset();
}

bool UserDefinedList::Set(const wchar_t *List, bool AddToList)
{
	if (AddToList)
	{
		if (List && !*List) // пусто, нечего добавл€ть
			return true;
	}
	else
		Free();

	bool rc=false;

	if (CheckSeparators() && List && *List)
	{
		UserDefinedListItem item;
		item.index=ItemsList.size();

		if (!strSeparators.Contains(*List))
		{
			bool Error=false;
			const wchar_t *CurList=List;
			int Length, RealLength;
			while (!Error && CurList && *CurList)
			{
				CurList=Skip(CurList, Length, RealLength, Error);
				if (Length > 0)
				{
					if (Flags.Check(ULF_PACKASTERISKS) && 3==Length && 0==memcmp(CurList, L"*.*", 6))
					{
						item.Str = L"*";
						ItemsList.push_back(item);
					}
					else
					{
						item.Str.Copy(CurList, Length);

						if (Flags.Check(ULF_PACKASTERISKS))
						{
							int i=0;
							bool lastAsterisk=false;

							while (i<Length)
							{
								if (item.Str[i]==L'*')
								{
									if (!lastAsterisk)
										lastAsterisk=true;
									else
									{
										item.Str.Remove(i);
										--i;
									}
								}
								else
									lastAsterisk=false;

								++i;
							}
						}

						if (Flags.Check(ULF_ADDASTERISK) && !wcspbrk(item.Str,L"?*."))
						{
							item.Str+=L"*";
						}
						ItemsList.push_back(item);
					}

					CurList+=RealLength;
					++item.index;
				}
			}

			rc=true;
		}
		else
		{
			const wchar_t *End=List+1;

			if (!Flags.Check(ULF_NOTRIM))
				while (IsSpace(*End)) ++End; // пропустим мусор

			if (!*End) // ≈сли кроме разделител€ ничего больше в строке нет,
			{         // то считаетс€, что это не разделитель, а простой символ
				item.Str = L" ";

				if (item.Str)
				{
					item.Str.Replace(0, *List);

					ItemsList.push_back(item);
					rc=true;
				}
			}
		}
	}

	if (rc)
	{
		if (Flags.Check(ULF_UNIQUE|ULF_SORT))
		{
			ItemsList.sort(UDItemLess);
			if(Flags.Check(ULF_UNIQUE))
			{
				ItemsList.unique([](UserDefinedListItem& a, UserDefinedListItem& b)->bool
				{
					if (a.index > b.index)
						a.index = b.index;
					return a == b;
				});
			}
		}

		size_t n=0;
		std::for_each(RANGE(ItemsList, i)
		{
			i.index = n++;
		});
		Reset();
	}
	else
		Free();

	return rc;
}

const wchar_t *UserDefinedList::Skip(const wchar_t *Str, int &Length, int &RealLength, bool &Error)
{
	Length=RealLength=0;
	Error=false;

	if (!Flags.Check(ULF_NOTRIM))
		while (IsSpace(*Str)) ++Str;

	if (strSeparators.Contains(*Str))
		++Str;

	if (!Flags.Check(ULF_NOTRIM))
		while (IsSpace(*Str)) ++Str;

	if (!*Str) return nullptr;

	const wchar_t *cur=Str;
	bool InBrackets=false, InQoutes = (*cur==L'\"');

	if (!InQoutes) // если мы в кавычках, то обработка будет позже и чуть сложнее
		while (*cur) // важно! проверка *cur должна сто€ть первой
		{
			if (Flags.Check(ULF_PROCESSBRACKETS)) // чтобы не сортировать уже отсортированное
			{
				if (*cur==L']')
					InBrackets=false;

				if (*cur==L'[' && nullptr!=wcschr(cur+1, L']'))
					InBrackets=true;
			}

			if (!InBrackets && strSeparators.Contains(*cur))
				break;

			++cur;
		}

	if (!InQoutes || !*cur)
	{
		RealLength=Length=(int)(cur-Str);
		--cur;

		if (!Flags.Check(ULF_NOTRIM))
			while (IsSpace(*cur))
			{
				--Length;
				--cur;
			}

		return Str;
	}

	// мы в кавычках - захватим все отсюда и до следующих кавычек
	++cur;
	const wchar_t *QuoteEnd=wcschr(cur, L'\"');

	if (!QuoteEnd)
	{
		Error=true;
		return nullptr;
	}

	const wchar_t *End=QuoteEnd+1;

	if (!Flags.Check(ULF_NOTRIM))
		while (IsSpace(*End)) ++End;

	if (!*End || strSeparators.Contains(*End))
	{
		if (!Flags.Check(ULF_NOUNQUOTE))
		{
			Length=(int)(QuoteEnd-cur);
			RealLength=(int)(End-cur);
		}
		else
		{
			Length=(int)(End-cur)+1;
			RealLength=Length;
			--cur;
		}
		return cur;
	}

	Error=true;
	return nullptr;
}

void UserDefinedList::Reset()
{
	CurrentItem=ItemsList.begin();
}

bool UserDefinedList::IsEmpty()
{
	return CurrentItem == ItemsList.end();
}

const wchar_t *UserDefinedList::GetNext()
{
	if(CurrentItem != ItemsList.end())
	{
		auto Result = CurrentItem->Str;
		++CurrentItem;
		return Result;
	}
	return nullptr;
}
