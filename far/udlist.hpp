#ifndef __UserDefinedList_HPP__
#define __UserDefinedList_HPP__

/*
udlist.hpp

������ ����-����, �������������� ����� ������-�����������. ���� �����, �����
������� ������ �������� �����������, �� ���� ������� ������� ��������� �
�������. ���� ����� ����������� ������ ������ � ������ ���, �� ���������, ���
��� �� �����������, � ������� ������.

*/

/* Revision: 1.09 23.05.2006 $ */

#include "farconst.hpp"
#include "array.hpp"

enum UDL_FLAGS
{
  ULF_ADDASTERISK    =0x00000001, // ��������� '*' � ����� �������� ������,
                                  // ���� �� �� �������� '?', '*' � '.'
  ULF_PACKASTERISKS  =0x00000002, // ������ "*.*" � ������ �������� ������ "*"
                                  // ������ "***" � ������ �������� ������ "*"
  ULF_PROCESSBRACKETS=0x00000004, // ��������� ���������� ������ ��� �������
                                  // ������ �������������
  ULF_UNIQUE         =0x00000010, // ������� ������������� ��������
  ULF_SORT           =0x00000020, // ������������� (� ������ ��������)
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
    BOOL CheckSeparators() const; // �������� ������������ �� ������������
    void SetDefaultSeparators();
    const wchar_t *Skip(const wchar_t *Str, int &Length, int &RealLength, BOOL &Error);
    static int __cdecl CmpItems(const UserDefinedListItemW **el1,
      const UserDefinedListItemW **el2);

  private:
    UserDefinedListW& operator=(const UserDefinedListW& rhs); // ����� ��
    UserDefinedListW(const UserDefinedListW& rhs); // �������������� �� ���������

  public:
    // �� ��������� ������������ ��������� ';' � ',', �
    // ProcessBrackets=AddAsterisk=PackAsterisks=FALSE
    // Unique=Sort=FALSE
    UserDefinedListW();

    // ���� ����������� �����������. ��. �������� SetParameters
    UserDefinedListW(WORD separator1, WORD separator2, DWORD Flags);
    ~UserDefinedListW() { Free(); }

  public:
    // ������� �������-����������� � ��������� ��� ��������� ���������
    // ���������� ������.
    // ���� ���� �� Separator* ����� 0x00, �� �� ������������ ��� ����������
    // (�.�. � Set)
    // ���� ��� ����������� ����� 0x00, �� ����������������� ����������� ��
    // ��������� (';' & ',').
    // ���� AddAsterisk ����� TRUE, �� � ����� �������� ������ �����
    // ����������� '*', ���� ���� ������� �� �������� '?', '*' � '.'
    // ���������� FALSE, ���� ���� �� ������������ �������� �������� ���
    // �������� ��������� ������ � ���� �� ������������ �������� ����������
    // �������.
    BOOL SetParameters(WORD Separator1, WORD Separator2, DWORD Flags);

    // �������������� ������. ��������� ������, ����������� �������������.
    // ���������� FALSE ��� �������.
    // ����: ���� List==NULL, �� ���������� ������������ ������� ����� ������
    BOOL Set(const wchar_t *List, BOOL AddToList=FALSE);

    // ���������� � ��� ������������� ������
    // ����: ���� NewItem==NULL, �� ���������� ������������ ������� �����
    // ������
    BOOL AddItem(const wchar_t *NewItem)
    {
      return Set(NewItem,TRUE);
    }

    // �������� ����� ������� ������ �� �������
    void Reset(void);

    // ������ ��������� �� ��������� ������� ������ ��� NULL
    const wchar_t *GetNext(void);

    // ���������� ������
    void Free();

    // TRUE, ���� ������ ��������� � ������ ���
    BOOL IsEmpty();

    // ������� ���������� ��������� � ������
    DWORD GetTotal () const { return Array.getSize(); }
};

#endif // __UserDefinedList_HPP
