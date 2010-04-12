/*
tvar.cpp

–еализаци€ класса TVar ("кастрированый" вариант - только целое и строковое значение)
(дл€ макросов)

*/

//---------------------------------------------------------------
// If this code works, it was written by Alexander Nazarenko.
// If not, I don't know who wrote it.
//---------------------------------------------------------------

#include "headers.hpp"
#pragma hdrstop

#include "tvar.hpp"

static const char *toString(__int64 num)
{
	static char str[128];
	_i64toa(num, str, 10);
	return str;
};

TVar::~TVar()
{
	if (str)
		delete [] str;
};

TVar::TVar(__int64 v) :
		vType(vtInteger),
		inum(v)
{
	str = NULL;
};

TVar::TVar(const char *v) :
		vType(vtString),
		inum(0)
{
	str = new char[strlen(v)+1];

	if (str)
		strcpy(str, v);
};

TVar::TVar(const TVar& v) :
		vType(v.vType),
		inum(v.inum)
{
	if (v.str)
	{
		str = new char[strlen(v.str)+1];

		if (str)
			strcpy(str, v.str);
	}
	else
		str = NULL;
};

TVar& TVar::operator=(const TVar& v)
{
	vType = v.vType;
	inum = v.inum;

	if (str)
		delete [] str;

	str = NULL;

	if (v.str)
	{
		str = new char[strlen(v.str)+1];

		if (str)
			strcpy(str, v.str);
	}

	return *this;
}

__int64 TVar::i() const
{
	return isInteger() ? inum : (str ? _atoi64(str) : 0);
}

const char *TVar::s() const
{
	if (isString())
		return  str ? str : "";

	return ::toString(inum);
}


const char *TVar::toString()
{
	char s[128];

	switch (vType)
	{
		case vtInteger:
			xstrncpy(s, ::toString(inum),sizeof(s)-1);
			break;
		default:
			return str;
	}

	if (str)
		delete [] str;

	str = new char[strlen(s)+1];

	if (str)
		strcpy(str, s);

	vType = vtString;
	return str;
};

__int64 TVar::toInteger()
{
	switch (vType)
	{
		case vtString:
			inum = str ? _atoi64(str) : 0;
			break;
	}

	vType = vtInteger;
	return inum;
};

int operator==(const TVar& a, const TVar& b)
{
	int r = 0;

	switch (a.vType)
	{

		case vtInteger: if (b.isInteger()) r = a.inum == b.inum;          break;

		case vtString:  if (b.isString())  r = strcmp(a.s(), b.s()) == 0; break;
	}

	return r;
};

int operator!=(const TVar& a, const TVar& b)
{
	int r = 1;

	switch (a.vType)
	{

		case vtInteger: if (b.isInteger()) r = a.inum != b.inum;          break;

		case vtString:  if (b.isString())  r = strcmp(a.s(), b.s()) != 0; break;
	}

	return r;
};

int operator<(const TVar& a, const TVar& b)
{
	int r = 0;

	switch (a.vType)
	{

		case vtInteger: if (b.isInteger()) r = a.inum < b.inum;           break;

		case vtString:  if (b.isString())  r = strcmp(a.s(), b.s()) < 0;  break;
	}

	return r;
};

int operator<=(const TVar& a, const TVar& b)
{
	int r = 0;

	switch (a.vType)
	{

		case vtInteger: if (b.isInteger()) r = a.inum <= b.inum;          break;

		case vtString:  if (b.isString())  r = strcmp(a.s(), b.s()) <= 0; break;
	}

	return r;
};

int operator>(const TVar& a, const TVar& b)
{
	int r = 0;

	switch (a.vType)
	{

		case vtInteger: if (b.isInteger()) r = a.inum > b.inum;           break;

		case vtString:  if (b.isString())  r = strcmp(a.s(), b.s()) > 0;  break;
	}

	return r;
};

int operator>=(const TVar& a, const TVar& b)
{
	int r = 0;

	switch (a.vType)
	{

		case vtInteger: if (b.isInteger()) r = a.inum >= b.inum;          break;

		case vtString:  if (b.isString())  r = strcmp(a.s(), b.s()) >= 0; break;
	}

	return r;
};

static TVar addStr(const char *a, const char *b)
{
	TVar r("");
	char *c = new char[strlen(a ? a : "")+strlen(b ? b : "")+1];

	if (c)
	{
		r = strcat(strcpy(c, a ? a : ""), b ? b : "");
		delete [] c;
	}

	return r;
}

TVar operator+(const TVar& a, const TVar& b)
{
	TVar r;

	switch (a.vType)
	{
		case vtInteger:

			switch (b.vType)
			{
				case vtInteger: r = a.inum + b.inum;                   break;
				case vtString:  r = addStr(::toString(a.inum), b.s()); break;
			}

			break;
		case vtString:

			switch (b.vType)
			{
				case vtInteger: r = addStr(a.s(), ::toString(b.inum)); break;
				case vtString:  r = addStr(a.s(), b.s());              break;
			}

			break;
	}

	return r;
};

TVar operator-(const TVar& a, const TVar& b)
{
	TVar r;

	switch (a.vType)
	{
		case vtInteger:

			switch (b.vType)
			{
				case vtInteger: r = a.inum - b.inum;                  break;
				case vtString:  r = a;                                break;
			}

			break;
		case vtString:
			r = a;
			break;
	}

	return r;
};

TVar operator*(const TVar& a, const TVar& b)
{
	TVar r;

	switch (a.vType)
	{
		case vtInteger:

			switch (b.vType)
			{
				case vtInteger: r = a.inum * b.inum;                  break;
				case vtString:  r = a;                                break;
			}

			break;
		case vtString:
			r = a;
			break;
	}

	return r;
};

TVar operator/(const TVar& a, const TVar& b)
{
	TVar r;

	switch (a.vType)
	{
		case vtInteger:

			switch (b.vType)
			{
				case vtInteger:
					r = b.inum ? (a.inum / b.inum) : _i64(0);
					break;
				case vtString:
					r = a;
					break;
			}

			break;
		case vtString:
			r = a;
			break;
	}

	return r;
};

TVar operator%(const TVar& a, const TVar& b)
{
	TVar r;

	switch (a.vType)
	{
		case vtInteger:

			switch (b.vType)
			{
				case vtInteger:
					r = b.inum ? (a.inum % b.inum) : _i64(0);
					break;
				case vtString:
					r = a;
					break;
			}

			break;
		case vtString:
			r = a;
			break;
	}

	return r;
};

TVar operator|(const TVar& a, const TVar& b)
{
	TVar r;

	switch (a.vType)
	{
		case vtInteger:

			switch (b.vType)
			{
				case vtInteger: r = a.inum | b.inum;                  break;
				case vtString:  r = a;                                break;
			}

			break;
		case vtString:
			r = a;
			break;
	}

	return r;
};

TVar operator&(const TVar& a, const TVar& b)
{
	TVar r;

	switch (a.vType)
	{
		case vtInteger:

			switch (b.vType)
			{
				case vtInteger: r = a.inum & b.inum;                  break;
				case vtString:  r = a;                                break;
			}

			break;
		case vtString:
			r = a;
			break;
	}

	return r;
};

TVar operator||(const TVar& a, const TVar& b)
{
	TVar r;

	switch (a.vType)
	{
		case vtInteger:

			switch (b.vType)
			{
				case vtInteger: r = a.inum || b.inum;                 break;
				case vtString:  r = a;                                break;
			}

			break;
		case vtString:
			r = a;
			break;
	}

	return r;
};

TVar operator&&(const TVar& a, const TVar& b)
{
	TVar r;

	switch (a.vType)
	{
		case vtInteger:

			switch (b.vType)
			{
				case vtInteger: r = a.inum && b.inum;                 break;
				case vtString:  r = a;                                break;
			}

			break;
		case vtString:
			r = a;
			break;
	}

	return r;
};

TVar operator^(const TVar& a, const TVar& b)
{
	TVar r;

	switch (a.vType)
	{
		case vtInteger:

			switch (b.vType)
			{
				case vtInteger: r = a.inum ^ b.inum;                  break;
				case vtString:  r = a;                                break;
			}

			break;
		case vtString:
			r = a;
			break;
	}

	return r;
}

TVar operator>>(const TVar& a, const TVar& b)
{
	TVar r;

	switch (a.vType)
	{
		case vtInteger:

			switch (b.vType)
			{
				case vtInteger: r = a.inum >> b.inum;                 break;
				case vtString:  r = a;                                break;
			}

			break;
		case vtString:
			r = a;
			break;
	}

	return r;
}

TVar operator<<(const TVar& a, const TVar& b)
{
	TVar r;

	switch (a.vType)
	{
		case vtInteger:

			switch (b.vType)
			{
				case vtInteger: r = a.inum << b.inum;                 break;
				case vtString:  r = a;                                break;
			}

			break;
		case vtString:
			r = a;
			break;
	}

	return r;
}

TVar TVar::operator+()
{
	return *this;
};


TVar TVar::operator-()
{
	switch (vType)
	{
		case vtInteger:
			return TVar(-inum);
		default:
			return *this;
	}
};

TVar TVar::operator!()
{
	switch (vType)
	{
		case vtInteger:
			return TVar(!inum);
		default:
			return *this;
	}
};

TVar TVar::operator~()
{
	switch (vType)
	{
		case vtInteger:
			return TVar(~inum);
		default:
			return *this;
	}
};

//---------------------------------------------------------------
// –абота с таблицами имен переменных
//---------------------------------------------------------------

int hash(const char *p)
{
	int i = 0;
	char *pp = (char*)p;

	while (*pp)
		i = i << (1^*(pp++));

	if (i < 0)
		i = -i;

	i %= V_TABLE_SIZE;
	return i;
}

int isVar(TVarTable table, const char *p)
{
	int i = hash(p);

	for (TVarSet *n = table[i] ; n ; n = ((TVarSet*)n->next))
		if (!stricmp(n->str, p))
			return 1;

	return 0;
}

TVarSet *varLook(TVarTable table, const char *p, bool ins)
{
	int i = hash(p);

	for (TVarSet *n = table[i] ; n ; n = ((TVarSet*)n->next))
		if (!stricmp(n->str, p))
			return n;

	if (ins)
	{
		TVarSet *nn = new TVarSet(p);
		nn->next = table[i];
		table[i] = nn;
		return nn;
	}

	return NULL;
}

TVarSet *varEnum(TVarTable table,int NumTable, int Index)
{
	if ((DWORD)NumTable >= V_TABLE_SIZE)
		return NULL;

	TVarSet *n = table[NumTable];

	for (int I=0; I < Index && n; ++I)
		n = ((TVarSet*)n->next);

	return n;
}

void varKill(TVarTable table, const char *p)
{
	int i = hash(p);
	TVarSet *nn = table[i];

	for (TVarSet *n = table[i] ; n ; n = ((TVarSet*)n->next))
	{
		if (!stricmp(n->str, p))
		{
			if (n == table[i])
				table[i]=((TVarSet*)n->next);
			else
				nn->next= n->next;

			//( ( n == table[i] ) ? table[i] : nn->next ) = n->next;
			delete n;
			return;
		}

		nn = n;
	}
}

void initVTable(TVarTable table)
{
	for (int i = 0 ; i < V_TABLE_SIZE ; i++)
		table[i] = NULL;
}

void deleteVTable(TVarTable table)
{
	for (int i = 0 ; i < V_TABLE_SIZE ; i++)
		while (table[i] != NULL)
		{
			TVarSet *n = ((TVarSet*)(table[i]->next));
			table[i]->next = NULL;
			delete table[i];
			table[i] = n;
		}
}
