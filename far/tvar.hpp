/*
tvar.hpp

–еализаци€ класса TVar ("кастрированый" вариант - только целое и строковое значение)
(дл€ макросов)

*/

#include "fn.hpp"

//---------------------------------------------------------------
// If this code works, it was written by Alexander Nazarenko.
// If not, I don't know who wrote it.
//---------------------------------------------------------------

#ifndef __TVAR_H
#define __TVAR_H

enum TVarType { vtInteger, vtString };

class TVar
{
	private:
		TVarType vType;
		__int64 inum;
		char *str;
	public:
		TVar(__int64 = 0);
		TVar(const char*);
		TVar(const TVar&);
		~TVar();

		friend TVar operator+(const TVar&, const TVar&);
		friend TVar operator-(const TVar&, const TVar&);
		friend TVar operator*(const TVar&, const TVar&);
		friend TVar operator/(const TVar&, const TVar&);
		friend TVar operator%(const TVar&, const TVar&);

		friend TVar operator&(const TVar&, const TVar&);
		friend TVar operator|(const TVar&, const TVar&);
		friend TVar operator^(const TVar&, const TVar&);
		friend TVar operator>>(const TVar&, const TVar&);
		friend TVar operator<<(const TVar&, const TVar&);

		friend TVar operator&&(const TVar&, const TVar&);
		friend TVar operator||(const TVar&, const TVar&);

		TVar& operator=(const TVar&);

		TVar& operator+=(const TVar& b) { return *this = *this+b; };
		TVar& operator-=(const TVar& b) { return *this = *this-b; };
		TVar& operator*=(const TVar& b) { return *this = *this*b; };
		TVar& operator/=(const TVar& b) { return *this = *this/b; };
		TVar& operator%=(const TVar& b) { return *this = *this%b; };

		TVar operator+();
		TVar operator-();
		TVar operator!();
		TVar operator~();

		friend int operator==(const TVar&, const TVar&);
		friend int operator!=(const TVar&, const TVar&);
		friend int operator<(const TVar&, const TVar&);
		friend int operator<=(const TVar&, const TVar&);
		friend int operator>(const TVar&, const TVar&);
		friend int operator>=(const TVar&, const TVar&);

		TVarType type() { return vType; };

		int isString()   const { return vType == vtString;  }
		int isInteger()  const { return vType == vtInteger; }

		__int64 i()      const;// { return isInteger() ? inum : 0; };
		const char *s()  const;// { return isString() ? ( str ? str : "" ) : ""; };

		const char *toString();
		__int64 toInteger();
};

//---------------------------------------------------------------
// –абота с таблицами имен переменных
//---------------------------------------------------------------

class TAbstractSet
{
	public:
		char *str;
		TAbstractSet *next;
		TAbstractSet(const char *s)
		{
			str = NULL;
			next = NULL;

			if (s)
			{
				str = new char[strlen(s)+1];
				strcpy(str, s);
			}
		}
		~TAbstractSet()
		{
			if (str)
				delete [] str;
		}
};

class TVarSet : public TAbstractSet
{
	public:
		TVar value;
		TVarSet(const char *s) :
				TAbstractSet(s),
				value() {}
};

const int V_TABLE_SIZE = 23;

typedef TVarSet *(TVarTable)[V_TABLE_SIZE];
extern int isVar(TVarTable, const char*);
extern TVarSet *varEnum(TVarTable, int, int);
extern TVarSet *varLook(TVarTable worktable, const char* name, bool ins=false);
extern void varKill(TVarTable, const char*);
extern void initVTable(TVarTable);
extern void deleteVTable(TVarTable);

inline TVarSet *varInsert(TVarTable t, const char *s)
{
	return varLook(t, s, true);
}

#endif // __TVAR_H
