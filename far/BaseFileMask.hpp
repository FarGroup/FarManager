#ifndef __BaseFileMask_HPP
#define __BaseFileMask_HPP

/*
BaseFileMask.hpp

јбстрактный класс, заведен дл€ удобства работы с масками.
*/

/* Revision: 1.02 21.11.2005 $ */

/*
Modify:
  21.11.2005 WARP
    + BaseFileMaskW
  24.04.2005 AY
    ! GCC
  01.07.2001 IS
    + ¬первые в эфире
*/

extern const int EXCLUDEMASKSEPARATOR;

class BaseFileMask
{
public:
    BaseFileMask() {}
    virtual ~BaseFileMask() {}

public:
    virtual BOOL Set(const char *Masks, DWORD Flags)=0;
    virtual BOOL Compare(const char *Name)=0;
    virtual BOOL IsEmpty(void) { return TRUE; }

private:
  BaseFileMask& operator=(const BaseFileMask& rhs); /* чтобы не */
  BaseFileMask(const BaseFileMask& rhs); /* генерировалось по умолчанию */

};


class BaseFileMaskW
{
public:
    BaseFileMaskW() {}
    virtual ~BaseFileMaskW() {}

public:
    virtual BOOL Set(const wchar_t *Masks, DWORD Flags)=0;
    virtual BOOL Compare(const wchar_t *Name)=0;
    virtual BOOL IsEmpty(void) { return TRUE; }

private:
  BaseFileMaskW& operator=(const BaseFileMaskW& rhs); /* чтобы не */
  BaseFileMaskW(const BaseFileMaskW& rhs); /* генерировалось по умолчанию */

};


#endif // __BaseFileMask_HPP
