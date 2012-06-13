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
	if (Str)
		xf_free(Str);
}

bool UserDefinedListItem::operator==(const UserDefinedListItem &rhs) const
{
	return (Str && rhs.Str)?(!StrCmpI(Str, rhs.Str)):false;
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

UserDefinedListItem& UserDefinedListItem::operator=(const
        UserDefinedListItem &rhs)
{
	if (this!=&rhs)
	{
		if (Str)
		{
			xf_free(Str);
			Str=nullptr;
		}

		if (rhs.Str)
			Str=xf_wcsdup(rhs.Str);

		index=rhs.index;
	}

	return *this;
}

UserDefinedListItem& UserDefinedListItem::operator=(const wchar_t *rhs)
{
	if (Str!=rhs)
	{
		if (Str)
		{
			xf_free(Str);
			Str=nullptr;
		}

		if (rhs)
			Str=xf_wcsdup(rhs);
	}

	return *this;
}

wchar_t *UserDefinedListItem::set(const wchar_t *Src, size_t size)
{
	if (Str!=Src)
	{
		if (Str)
		{
			xf_free(Str);
			Str=nullptr;
		}

		Str=static_cast<wchar_t*>(xf_malloc((size+1)*sizeof(wchar_t)));

		if (Str)
		{
			wmemcpy(Str,Src,size);
			Str[size]=0;
		}
	}

	return Str;
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
	Array.Free();
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
		item.index=Array.getSize();

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
						item=L"*";

						if (!item.Str || !Array.addItem(item))
							Error=true;
					}
					else
					{
						item.set(CurList, Length);

						if (item.Str)
						{
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
											wmemcpy(item.Str+i, item.Str+i+1, StrLength(item.Str+i+1)+1);
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
								Length=StrLength(item.Str);
								/* $ 18.09.2002 DJ
								   выдел€лось на 1 байт меньше, чем надо
								*/
								item.Str=static_cast<wchar_t*>(xf_realloc(item.Str, (Length+2)*sizeof(wchar_t)));

								/* DJ $ */
								if (item.Str)
								{
									item.Str[Length]=L'*';
									item.Str[Length+1]=0;
								}
								else
									Error=true;
							}

							if (!Error && !Array.addItem(item))
								Error=true;
						}
						else
							Error=true;
					}

					CurList+=RealLength;
					++item.index;
				}
			}

			rc=!Error;
		}
		else
		{
			const wchar_t *End=List+1;

			if (!Flags.Check(ULF_NOTRIM))
				while (IsSpace(*End)) ++End; // пропустим мусор

			if (!*End) // ≈сли кроме разделител€ ничего больше в строке нет,
			{         // то считаетс€, что это не разделитель, а простой символ
				item=L" ";

				if (item.Str)
				{
					*item.Str=*List;

					if (Array.addItem(item))
						rc=true;
				}
			}
		}
	}

	if (rc)
	{
		if (Flags.Check(ULF_UNIQUE))
		{
			Array.Sort();
			Array.Pack(UpdateRemainingItem);
		}

		if (!Flags.Check(ULF_SORT))
			Array.Sort(CmpItems);
		else if (!Flags.Check(ULF_UNIQUE)) // чтобы не сортировать уже отсортированное
			Array.Sort();

		size_t i=0, maxI=Array.getSize();

		for (; i<maxI; ++i)
			Array.getItem(i)->index=i;

		Reset();
	}
	else
		Free();

	return rc;
}

int CDECL UserDefinedList::CmpItems(const UserDefinedListItem **el1,
                                      const UserDefinedListItem **el2)
{
	if (*el1==*el2)
		return 0;
	else if ((**el1).index==(**el2).index)
		return 0;
	else if ((**el1).index<(**el2).index)
		return -1;
	else
		return 1;
}

void CDECL UserDefinedList::UpdateRemainingItem(UserDefinedListItem *elRemaining, const UserDefinedListItem *elDeleted)
{
	if (elRemaining->index > elDeleted->index)
		elRemaining->index = elDeleted->index;
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
	CurrentItem=0;
}

bool UserDefinedList::IsEmpty()
{
	size_t Size=Array.getSize();
	return !Size || CurrentItem>=Size;
}

const wchar_t *UserDefinedList::GetNext()
{
	UserDefinedListItem *item=Array.getItem(CurrentItem);
	++CurrentItem;
	return item?item->Str:nullptr;
}
