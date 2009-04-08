#ifndef __TLIST_HPP__
#define __TLIST_HPP__

/*  TList.hpp
    Простой шаблон работы с двусвязным списком / by Spinoza /
    Object должен иметь конструктор по умолчанию и следующие операторы:
      const Object& operator=(const Object &)
*/

#if defined(__BORLANDC__)
  #pragma warn -inl
#endif

template <class Object>
class TList
{
  protected:
    struct OneItem
    {
      Object   Item;
      OneItem *Prev, *Next;
    };

    OneItem *First, *Last, *Current, *Tmp, *SavedPos;

    DWORD Size;

  public:
    TList(): First(NULL), Last(NULL), Current(NULL), SavedPos(0), Size(0)
      {}
    ~TList() { clear(); }

  public:
    // получить количество элементов в списке
    DWORD size() const { return Size; }

    // возвращает TRUE, если список пуст
    bool empty() const { return !Size; }

    //  получить указатель на текущий элемент списка (возвращает NULL при неудаче)
    const Object *getItem() { return Current?&Current->Item:NULL; }

    // возвращает TRUE, если текущая позиция установлена на первый элемент
    bool isBegin() const { return Current==First; }

    // возвращает TRUE, если текущая позиция установлена на последний элемент
    bool isEnd() const { return Current==Last; }

    // идти в начало списка и вернуть указатель на первый элемент
    // Возвращается NULL, если список пуст
    const Object *toBegin()
    {
      Current=First;
      return Current?&Current->Item:NULL;
    }

    // идти в конец списка и вернуть указатель на последний элемент
    // Возвращается NULL, если список пуст
    const Object *toEnd()
    {
      Current=Last;
      return Current?&Current->Item:NULL;
    }

    // идти к следующему элементу списка и вернуть указатель на него
    // (возвращает NULL, если достигнут конец списка или текущий элемент
    // не был определен)
    const Object *toNext()
    {
      return (Current && NULL!=(Current=Current->Next))?&Current->Item:NULL;
    }

    // идти к предыдущему элементу списка и вернуть указатель на него
    // (возвращает NULL, если достигнуто начало списка или текущий элемент
    // не был определен)
    const Object *toPrev()
    {
      return (Current && NULL!=(Current=Current->Prev))?&Current->Item:NULL;
    }

    // добавить элемент в начало списка
    // текущая позиция при успехе устанавливается на этот элемент
    // при неудаче возвращается FALSE
    bool push_front(const Object &Source)
    {
      Tmp=new OneItem;
      if (Tmp) // сработает, если не будет исключения
      {
        Tmp->Item=Source;
        Tmp->Prev=NULL;
        if (First)
          First->Prev=Tmp;
        Tmp->Next=First;
        First=Current=Tmp;
        if (!Last) // список до операции был пуст
          Last=First;
        ++Size;
        return true;
      }
      return false;
    }

    // добавить элемент в конец списка
    // текущая позиция при успехе устанавливается на этот элемент
    // при неудаче возвращается FALSE
    bool push_back(const Object &Source)
    {
      Tmp=new OneItem;
      if (Tmp) // сработает, если не будет исключения
      {
        Tmp->Item=Source;
        if(Last)
          Last->Next=Tmp;
        Tmp->Prev=Last;
        Tmp->Next=NULL;
        Last=Current=Tmp;
        if (!First) // список до операции был пуст
          First=Last;
        ++Size;
        return true;
      }
      return false;
    }

    // вставить элемент после текущей позиции в списке
    // если текущая позиция не определена, то элемент добавляется в конец списка (=push_back)
    // текущая позиция при успехе устанавливается на новый элемент
    // при неудаче возвращается FALSE
    bool insert(const Object &Source)
    {
      if (!Current)
        return push_back(Source);
      Tmp=new OneItem;
      if (Tmp) // сработает, если не будет исключения
      {
        if (isEnd())
          Last=Tmp;
        Tmp->Item=Source;
        Tmp->Next=Current->Next;
        Tmp->Prev=Current;
        Current->Next=Tmp;
        if (Tmp->Next)
          Tmp->Next->Prev=Tmp;
        Current=Tmp;
        ++Size;
        return true;
      }
      return false;
    }

    // удалить элемент, текущая позиция устанавливается на следующий элемент
    // если текущая позиция до операции не определена, то возвращается FALSE
    bool removeToEnd()
    {
      SavedPos=NULL;
      if (Current)
      {
        if (isEnd())
          Last=Last->Prev;
        if (isBegin())
          First=First->Next;
        Tmp=Current->Next;
        if (Current->Next)
          Current->Next->Prev = Current->Prev;
        if (Current->Prev)
          Current->Prev->Next = Current->Next;
        delete Current;
        Current=Tmp;
        --Size;
        if (!Size)
          First=Last=NULL;
        return true;
      }
      return false;
    }

    // синоним removeToEnd
    bool erase() { return removeToEnd(); }

    // удалить элемент, текущая позиция устанавливается на предыдущий элемент
    // если текущая позиция до операции не определена, то возвращается FALSE
    bool removeToBegin()
    {
      SavedPos=NULL;
      if (Current)
      {
        if (isEnd())
          Last=Last->Prev;
        if (isBegin())
          First=First->Next;
        Tmp=Current->Prev;
        if (Current->Next)
          Current->Next->Prev = Current->Prev;
        if (Current->Prev)
          Current->Prev->Next = Current->Next;
        delete Current;
        Current=Tmp;
        --Size;
        if (!Size)
          First=Last=NULL;
        return true;
      }
      return false;
    }

    // замещает список на содержимое списка lst и наоборот
    void swap(TList<Object> &lst)
    {
      OneItem *newFirst=lst.First, *newLast=lst.Last,
              *newCurrent=lst.Current, *newSavedPos=lst.SavedPos;
      lst.First=First;
      lst.Last=Last;
      lst.Current=Current;
      lst.SavedPos=SavedPos;
      First=newFirst;
      Last=newLast;
      Current=newCurrent;
      SavedPos=newSavedPos;
    }

    // сохранить текущую позицию (см. restorePosition) для последующего
    // восстановления. Возвращается FALSE, если текущая позиция была
    // неопределенной
    bool storePosition()
    {
      SavedPos=Current;
      return SavedPos!=NULL;
    }

    // восстановить текущую позицию (см. storePosition) для последующего
    // восстановления. Возвращается FALSE, если текущая позиция стала
    // неопределенной
    bool restorePosition()
    {
      Current=SavedPos;
      return Current!=NULL;
    }

    // очистить список
    void clear()
    {
      toBegin();
      while(erase())
        ;
      SavedPos=NULL;
    }


  private:
    TList& operator=(const TList& rhs); /* чтобы не генерировалось */
    TList(const TList& rhs);            /* по умолчанию            */
};

#if defined(__BORLANDC__)
  #pragma warn .inl
#endif

#endif // __TLIST_HPP__
