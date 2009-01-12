#ifndef __MY_REGTREE
#define __MY_REGTREE

/*******************************************************************
     HTreeReg                         - template class for tree of HTreeRegValue
                           template:
                                          NameT - name class such as MyString or StatString
     HTreeRegValueBase                - base class for tree-item that holds HValue data
       HTreeRegValue                  - template class for add name class to tree-item
                           template:
                                          NameT - name class
 *******************************************************************/

#define HT_NAME_REG        "HTreeRegValue"

/*******************************************************************
   HTreeRegValueBase - tree-style data item
 *******************************************************************/
CLASSBASE( HTreeRegValueBase,public HTreeValue )
    HANDLE Handle;
  protected:
    virtual BOOL   DoLoadItem( int File );
    virtual BOOL   DoSaveItem( int File );
    virtual BOOL   DoLoadTree( int File );
    virtual BOOL   DoSaveTree( int File );
  //HValue
   virtual void    DoEmpty( void );
   virtual DWORD   DoSizeofData( void )  const;
   virtual LPVOID  DoPtrData( void )     const;
   virtual void    DoTypeChange( HVType oldT,HVType newT,LPVOID val );
  protected:
    PHTreeValue    intAdd( CONSTSTR PathName );
    PHTreeValue    intAdd( CONSTSTR PathName,DWORD Data );
    PHTreeValue    intAdd( CONSTSTR PathName,LPVOID Data,DWORD DataSize );
    PHTreeValue    intAdd( CONSTSTR PathName,CONSTSTR Data );
    HANDLE         intAddValue( LPVOID Data,DWORD Size );
    BOOL           intDelValue( HANDLE Value );
    BOOL           intGetData( LPVOID Buff,DWORD sz );
    BOOL           intSetData( const LPVOID Buff,DWORD sz );
  public:
    HTreeRegValueBase( void );
    ~HTreeRegValueBase();
};

/*******************************************************************
   HTreeRegValue - tree-style data item
 *******************************************************************/
template <class NameT> class HTreeRegValue : public HTreeRegValueBase {
   NameT _Name;
   typedef HTreeRegValue<NameT> *PT;
  protected:
  //HTreeRegValueBase
   virtual CONSTSTR    GetName( void )         { return _Name.Text(); }
   virtual PHTreeValue DoCreate( CONSTSTR nm ) { return new HTreeRegValue<NameT>(nm); }
   virtual CONSTSTR    ClassName( void )       { return HT_NAME_REG; }
  public:
    HTreeRegValue( CONSTSTR nm ) : _Name(nm) {}

    BOOL SetData( const LPVOID Buff,DWORD sz  )               { return intSetData(Buff,sz); }
    BOOL SetData( CONSTSTR Data )                             { return (Data)?intSetData(Data,strlen(Data)+1):FALSE; }
    PT   Add( CONSTSTR PathName )                             { return (PT)intAdd(PathName); }
    PT   Add( CONSTSTR PathName,DWORD Data )                  { return (PT)intAdd(PathName,Data); }
    PT   Add( CONSTSTR PathName,LPVOID Data,DWORD DataSize )  { return (PT)intAdd(PathName,Data,DataSize); }
    PT   Add( CONSTSTR PathName,CONSTSTR Data )               { return (PT)intAdd(PathName,Data); }

    PT   LocateValue( CONSTSTR Name )                         { return (PT)intFindValue(Name); }
    PT   Locate( CONSTSTR PathName )                          { return (PT)intLocate(Name); }

    BOOL Load( CONSTSTR PathName )                            { return intLoad(PathName); }
    BOOL Save( CONSTSTR PathName )                            { return intSave(PathName); }
};

/*******************************************************************
   HTreeReg - tree for reg data
 *******************************************************************/
template <class T> class HTreeReg : public HTreeBase {
   T Root;
  public:
    HTreeReg( PHDataPool p ) : HTreeBase(p), Root( "" )        { Root.Tree = this; }
    ~HTreeReg()                                                { Root.Clear(); }

    T     *Add( CONSTSTR PathName )                            { return Root.Add(PathName); }
    T     *Add( CONSTSTR PathName,DWORD Data )                 { return Root.Add(PathName,Data); }
    T     *Add( CONSTSTR PathName,LPVOID Data,DWORD DataSize ) { return Root.Add(PathName,Data,DataSize); }
    T     *Add( CONSTSTR PathName,CONSTSTR Data )              { return Root.Add(PathName,Data); }

    T     *First( void )                                       { return &Root; }
    void   Clear( void )                                       { Root.Clear(); }
    T     *Locate( CONSTSTR PathName )                         { return (T*)Root.Locate(PathName); }
    void   Delete( T *p )                                      { if ( !p || p->Tree != this ) return;
                                                                            if (p->Parent) p->Parent->Items.Delete(p);
                                                                            p->Parent = NULL;
                                                                            p->Tree   = NULL; }
    BOOL   Load( CONSTSTR PathName )                           { return Root.Load(PathName); }
    BOOL   Save( CONSTSTR PathName )                           { return Root.Save(PathName); }
};

#endif