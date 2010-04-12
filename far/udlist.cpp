/*
udlist.cpp

Список чего-либо, перечисленного через символ-разделитель. Если нужно, чтобы
элемент списка содержал разделитель, то этот элемент следует заключить в
кавычки. Если кроме разделителя ничего больше в строке нет, то считается, что
это не разделитель, а простой символ.

*/

#include "headers.hpp"
#pragma hdrstop

#include "udlist.hpp"
#include "fn.hpp"

UserDefinedListItem::~UserDefinedListItem()
{
	if (Str)
		xf_free(Str);
}

bool UserDefinedListItem::operator==(const UserDefinedListItem &rhs) const
{
	return (Str && rhs.Str)?(!LocalStricmp(Str, rhs.Str)):false;
}

int UserDefinedListItem::operator<(const UserDefinedListItem &rhs) const
{
	if (!Str)
		return 1;
	else if (!rhs.Str)
		return -1;
	else
		return LocalStricmp(Str, rhs.Str)<0;
}

const UserDefinedListItem& UserDefinedListItem::operator=(const
        UserDefinedListItem &rhs)
{
	if (this!=&rhs)
	{
		if (Str)
		{
			xf_free(Str);
			Str=NULL;
		}

		if (rhs.Str)
			Str=xf_strdup(rhs.Str);

		index=rhs.index;
	}

	return *this;
}

const UserDefinedListItem& UserDefinedListItem::operator=(const char *rhs)
{
	if (Str!=rhs)
	{
		if (Str)
		{
			xf_free(Str);
			Str=NULL;
		}

		if (rhs)
			Str=xf_strdup(rhs);
	}

	return *this;
}

char *UserDefinedListItem::set(const char *Src, unsigned int size)
{
	if (Str!=Src)
	{
		if (Str)
		{
			xf_free(Str);
			Str=NULL;
		}

		Str=static_cast<char*>(xf_malloc(size+1));

		if (Str)
		{
			memcpy(Str, Src, size);
			Str[size]=0;
		}
	}

	return Str;
}

UserDefinedList::UserDefinedList()
{
	Reset();
	SetParameters(0,0,0);
}

UserDefinedList::UserDefinedList(BYTE separator1, BYTE separator2,
                                 DWORD Flags)
{
	Reset();
	SetParameters(separator1, separator2, Flags);
}

void UserDefinedList::SetDefaultSeparators()
{
	Separator1=';';
	Separator2=',';
}

BOOL UserDefinedList::CheckSeparators() const
{
	return !((Separator1=='\"' || Separator2=='\"') ||
	         (ProcessBrackets && (Separator1=='[' || Separator2=='[' ||
	                              Separator1==']' || Separator2==']'))
	        );
}

BOOL UserDefinedList::SetParameters(BYTE separator1, BYTE separator2,
                                    DWORD Flags)
{
	Free();
	Separator1=separator1;
	Separator2=separator2;
	ProcessBrackets=(Flags & ULF_PROCESSBRACKETS)?TRUE:FALSE;
	AddAsterisk=(Flags & ULF_ADDASTERISK)?TRUE:FALSE;
	PackAsterisks=(Flags & ULF_PACKASTERISKS)?TRUE:FALSE;
	Unique=(Flags & ULF_UNIQUE)?TRUE:FALSE;
	Sort=(Flags & ULF_SORT)?TRUE:FALSE;

	if (!Separator1 && Separator2)
	{
		Separator1=Separator2;
		Separator2=0;
	}

	if (!Separator1 && !Separator2) SetDefaultSeparators();

	return CheckSeparators();
}

void UserDefinedList::Free()
{
	Array.Free();
	Reset();
}

BOOL UserDefinedList::Set(const char *List, BOOL AddToList)
{
	if (AddToList)
	{
		if (List && !*List) // пусто, нечего добавлять
			return TRUE;
	}
	else
		Free();

	BOOL rc=FALSE;

	if (CheckSeparators() && List && *List)
	{
		int Length, RealLength;
		UserDefinedListItem item;
		item.index=Array.getSize();

		if (*List!=Separator1 && *List!=Separator2)
		{
			Length=lstrlen(List);
			BOOL Error=FALSE;
			const char *CurList=List;

			while (!Error &&
			        NULL!=(CurList=Skip(CurList, Length, RealLength, Error)))
			{
				if (Length > 0)
				{
					if (PackAsterisks && 3==Length && 0==memcmp(CurList, "*.*", 3))
					{
						item="*";

						if (!item.Str || !Array.addItem(item))
							Error=TRUE;
					}
					else
					{
						item.set(CurList, Length);

						if (item.Str)
						{
							if (PackAsterisks)
							{
								int i=0, lastAsterisk=FALSE;

								while (i<Length)
								{
									if (item.Str[i]=='*')
									{
										if (!lastAsterisk)
											lastAsterisk=TRUE;
										else
										{
											memcpy(item.Str+i, item.Str+i+1,
											       strlen(item.Str+i+1)+1);
											--i;
										}
									}
									else
										lastAsterisk=FALSE;

									++i;
								}
							}

							if (AddAsterisk && strpbrk(item.Str,"?*.")==NULL)
							{
								Length=(int)strlen(item.Str);
								/* $ 18.09.2002 DJ
								   выделялось на 1 байт меньше, чем надо
								*/
								item.Str=static_cast<char*>(xf_realloc(item.Str, Length+2));

								/* DJ $ */
								if (item.Str)
								{
									item.Str[Length]='*';
									item.Str[Length+1]=0;
								}
								else
									Error=TRUE;
							}

							if (!Error && !Array.addItem(item))
								Error=TRUE;
						}
						else
							Error=TRUE;
					}

					CurList+=RealLength;
				}
				else
					Error=TRUE;

				++item.index;
			}

			rc=!Error;
		}
		else
		{
			const char *End=List+1;

			while (IsSpace(*End)) ++End; // пропустим мусор

			if (!*End) // Если кроме разделителя ничего больше в строке нет,
			{         // то считается, что это не разделитель, а простой символ
				item=" ";

				if (item.Str)
				{
					*item.Str=*List;

					if (Array.addItem(item))
						rc=TRUE;
				}
			}
		}
	}

	if (rc)
	{
		if (Unique)
		{
			Array.Sort();
			Array.Pack();
		}

		if (!Sort)
			Array.Sort(reinterpret_cast<TARRAYCMPFUNC>(CmpItems));
		else if (!Unique) // чтобы не сортировать уже отсортированное
			Array.Sort();

		unsigned int i=0, maxI=Array.getSize();

		for (; i<maxI; ++i)
			Array.getItem(i)->index=i;

		Reset();
	}
	else
		Free();

	return rc;
}

int __cdecl UserDefinedList::CmpItems(const UserDefinedListItem **el1,
                                      const UserDefinedListItem **el2)
{
	if (el1==el2)
		return 0;
	else if ((**el1).index==(**el2).index)
		return 0;
	else if ((**el1).index<(**el2).index)
		return -1;
	else
		return 1;
}

const char *UserDefinedList::Skip(const char *Str, int &Length, int &RealLength, BOOL &Error)
{
	Length=RealLength=0;
	Error=FALSE;

	while (IsSpace(*Str)) ++Str;

	if (*Str==Separator1 || *Str==Separator2) ++Str;

	while (IsSpace(*Str)) ++Str;

	if (!*Str) return NULL;

	const char *cur=Str;
	BOOL InBrackets=FALSE, InQoutes = (*cur=='\"');

	if (!InQoutes) // если мы в кавычках, то обработка будет позже и чуть сложнее
		while (*cur) // важно! проверка *cur!=0 должна стоять первой
		{
			if (ProcessBrackets)
			{
				if (*cur==']')
					InBrackets=FALSE;

				if (*cur=='[' && NULL!=strchr(cur+1, ']'))
					InBrackets=TRUE;
			}

			if (!InBrackets && (*cur==Separator1 || *cur==Separator2))
				break;

			++cur;
		}

	if (!InQoutes || !*cur)
	{
		RealLength=Length=(int)(cur-Str);
		--cur;

		while (IsSpace(*cur))
		{
			--Length;
			--cur;
		}

		return Str;
	}

	// мы в кавычках - захватим все отсюда и до следующих кавычек
	++cur;
	const char *QuoteEnd=strchr(cur, '\"');

	if (QuoteEnd==NULL)
	{
		Error=TRUE;
		return NULL;
	}

	const char *End=QuoteEnd+1;

	while (IsSpace(*End)) ++End;

	if (!*End || *End==Separator1 || *End==Separator2)
	{
		Length=(int)(QuoteEnd-cur);
		RealLength=(int)(End-cur);
		return cur;
	}

	Error=TRUE;
	return NULL;
}

void UserDefinedList::Reset(void)
{
	CurrentItem=0;
}

BOOL UserDefinedList::IsEmpty()
{
	unsigned int Size=Array.getSize();
	return !Size || CurrentItem>=Size;
}

const char *UserDefinedList::GetNext()
{
	UserDefinedListItem *item=Array.getItem(CurrentItem);
	++CurrentItem;
	return item?item->Str:NULL;
}
