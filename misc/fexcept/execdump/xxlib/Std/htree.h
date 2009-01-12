#ifndef __MY_HTREE
#define __MY_HTREE

#define HT_KEYDELIMITER    "\\/"
#define HT_MAGIC           MK_ID( 'T','r','e','E' )

#define HT_NAME_NAME       "HTreeNameValue"
#define HT_NAME_NAMEPTR    "HTreeNamePtrValue"
#define HT_NAME_NAMEOBJECT "HTreeNameObjectValue"

/*******************************************************************
   HTreeBase                          - base class for all Tree`s
                           has:
                                          Pool   - data pool for store dynamic data (can bee NULL)
     HTree                            - template class for any type of tree-item
                           template:
                                          T - root tree-item
   HTreeValue                         - base class for all tree items
                           has:
                                          Tree   - ptr to tree owner
                                          Parent - ptr to parent element
                                          Items  - array of items
                           virtuals:
                                          DoLoadTree  - load current item and all subitems from file
                                          DoSaveTree  - save current item and all subitems from file
                                          DoLoadItem  - load current item personal data
                                          DoSaveItem  - save current item personal data
                                          DoCreate    - create tree item object of the same type as self
                                          ClassName   - class name for current tree-item (used to save treeitem in file and check it on load)
                                          GetName     - returns name current of tree-item (used on tree item search and creation)
     HTreeNameValue                   - template class, that contain name class
                                        Can be used as treeitem if tree-item may contain only name
                           template:
                                          NameT - name class
       HTreeNamePtrValue                  - template class for additional store with tree-item ptr object
                           has:
                                          Object - ptr to object (will auto delete on destruction)
                           template:
                                          T     - object class
                                          NameT - name class
       HTreeNameObjectValue               - template class for additional store with tree-item object as referense
                           has:
                                          Object - ptr to object (will auto delete on destruction)
                           template:
                                          T     - object class
                                          NameT - name class
 *******************************************************************/
STRUCT( HTreeBase )
    PHDataPool Pool;
  public:
    HTreeBase( PHDataPool p );
    virtual ~HTreeBase();
};

/*******************************************************************
   HTreeValue - tree-style data item
 *******************************************************************/
CLASSBASE( HTreeValue, public HValue )
  public:
   typedef BOOL (RTL_CALLBACK *HTreeEnum)( PHTreeValue tree,int Level,HANDLE UserPtr );
#if defined(__BCWIN32__)
   typedef BOOL __fastcall   (__closure *HTreeEnumCL)( PHTreeValue tree,int Level );
#endif
  protected:
   virtual BOOL        DoLoadTree( int File );
   virtual BOOL        DoSaveTree( int File );
   virtual BOOL        DoLoadItem( int File );
   virtual BOOL        DoSaveItem( int File );
   virtual PHTreeValue DoCreate( CONSTSTR Name ) = 0;
   virtual CONSTSTR    ClassName( void )         = 0;
   virtual CONSTSTR    GetName( void )           = 0;
  protected:
   PHTreeValue         AddItem( CONSTSTR Name,PHTreeValue Into );
   PHTreeValue         DoAdd( CONSTSTR PathName );
   PHTreeValue         intFindValue( CONSTSTR Name );                //Find tree-item by name on current level only
   PHTreeValue         intLocate( CONSTSTR PathName );               //Find tree-item by name on all subtree
   BOOL                intLoad( CONSTSTR PathName );                 //Load treeitem and all subtree items from file
   BOOL                intSave( CONSTSTR PathName );                 //Save treeitem and all subtree items from file
  public:
   PHTreeBase           Tree;
   PHTreeValue          Parent;
   MyArray<PHTreeValue> Items;
  public:
   HTreeValue( void );
   virtual ~HTreeValue();

   static CONSTSTR ReadFileClassName( int File );
   static CONSTSTR ReadFileClassName( CONSTSTR FileName );
   static int      BackCount( HTreeValue const *p );

   BOOL        Enum( HTreeEnum CB,int Level = 0,HANDLE Ptr = NULL )     const; //Enumerate all sub-tree items except self
   BOOL        EnumBack( HTreeEnum CB,int Level = 0,HANDLE Ptr = NULL ) const; //Back enumerate all sub-tree items except self
#if defined(__BCWIN32__)
   BOOL        EnumCL( HTreeEnumCL CB,int Level = 0)      const;     //Enumerate all sub-tree items except self
   BOOL        EnumBackCL( HTreeEnumCL CB,int Level = 0 ) const;     //Back enumerate all sub-tree items except self
#endif

   CONSTSTR    Name( void );             //Query tree-item name
   CONSTSTR    GetClassName( void );     //Get class name
   void        Clear( void );            //Delete all subtree items
   BOOL        isRoot( void );           //Is this tree-item root-item
   BOOL        isLeath( void );          //Is current treeitem has no childs
   int         FullCount( void );        //Calculate count of all subtree items
   int         BackCount( void ) const;
};

template <class NameT> class HTreeNameValue : public HTreeValue {
    NameT _Name;
  protected:
    virtual CONSTSTR    GetName( void )         { return _Name.Text(); }
    virtual PHTreeValue DoCreate( CONSTSTR nm ) { return new HTreeNameValue<NameT>(nm); }
    virtual CONSTSTR    ClassName( void )       { return HT_NAME_NAME; }
  public:
    HTreeNameValue( CONSTSTR nm ) : _Name(nm) {}
};

template <class T,class NameT> class HTreeNamePtrValue : public HTreeNameValue<NameT> {
  public:
    T    *Object;
  protected:
    virtual CONSTSTR    ClassName( void )       { return HT_NAME_NAMEPTR; }
    virtual PHTreeValue DoCreate( CONSTSTR nm ) { return HTreeNamePtrValue<T,NameT>(nm); }
  public:
    HTreeNamePtrValue( CONSTSTR nm ) : HTreeNameValue<NameT>(nm) { Object = NULL; }
    ~HTreeNamePtrValue()                                         { FreePtr(Object); }
};

template <class T,class NameT> class HTreeNameObjectValue : public HTreeNameValue<NameT> {
  public:
    T    Object;
  protected:
    virtual CONSTSTR    ClassName( void )       { return HT_NAME_NAMEOBJECT; }
    virtual PHTreeValue DoCreate( CONSTSTR nm ) { return HTreeNameObjectValue<T,NameT>(nm); }
  public:
    HTreeNameObjectValue( CONSTSTR nm ) : HTreeNameValue<NameT>(nm) {}
};

/*******************************************************************
   HTree - tree-style data holder
 *******************************************************************/
template <class T> class HTree : public HTreeBase {
   T Root;
  public:
    HTree( PHDataPool p ) : HTreeBase(p), Root( "" ) { Root.Tree = this; }
    ~HTree()                                         { Root.Clear(); }

    T     *First( void )                             { return &Root; }
    void   Clear( void )                             { Root.Clear(); }
    T     *Locate( CONSTSTR PathName )               { return (T*)Root.Locate(PathName); }
    void   Delete( T *p )                            { if ( !p || p->Tree != this ) return;
                                                       if (p->Parent) p->Parent->Items.Delete(p);
                                                       p->Parent = NULL;
                                                       p->Tree   = NULL; }
    BOOL   Load( CONSTSTR PathName )                 { return Root.Load(PathName); }
    BOOL   Save( CONSTSTR PathName )                 { return Root.Save(PathName); }
};

#endif
