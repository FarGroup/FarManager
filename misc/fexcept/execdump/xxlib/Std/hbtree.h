#ifndef __MY_HBINARYTREE
#define __MY_HBINARYTREE

/*******************************************************************
   HBTreeBase            - b-tree object
                           has: Parent,Left,Right
                      virtuals:
                             maCmp Cmp( .. )                       - compare leath\value|leath.
                                                                     Must be overriden for new b-tree type
                             PHBTreeBase DoCreate( HANDLE Param )  - create new b-tree of self type
                                                                     Must be overrided on each new descendants
                             void DoDeleteLeath( PHBTreeBase& p )  - call  on every b-tree item deletion

     HBNumTree           - DWORD-indexed b-tree
                           has: ID
       HBNumPtrTree      - template DWORD-indexed b-tree, that holds an object pointer deleted on leath destroy
                           has: Object
       HBNumObjectTree   - template DWORD-indexed b-tree, that holds an object reference
                           has: Object
     HBNameTree          - name-indexed b-tree
                           has: Name
       HBNamePtrTree     - template name-indexed b-tree, that holds an object pointer deleted on leath destroy
                           has: Object
       HBNameObjectTree  - template DWORD-indexed b-tree, that holds an object reference
                           has: Object
 *******************************************************************/
CLASS( HBTreeBase )
  public:
   typedef BOOL (RTL_CALLBACK *HBTreeEnum)( PHBTreeBase tree,int Level,HANDLE UserPtr );
#if defined(__BCWIN32__)
   typedef BOOL __fastcall   (__closure *HBTreeEnumCL)( PHBTreeBase tree,int Level );
#endif
  public:
   PHBTreeBase Parent,
               Left,
               Right;
  private:
    BOOL DoEnum( void )       const;
    BOOL DoEnumSort( void )   const;
#if defined(__BCWIN32__)
    BOOL DoEnumCL( void )     const;
    BOOL DoEnumSortCL( void ) const;
#endif
  protected:
    virtual maCmp       Cmp( PHBTreeBase p )              = 0;
    virtual maCmp       Cmp( HANDLE Param )               = 0;
    virtual PHBTreeBase DoCreate( HANDLE Param )          = 0;
    virtual void        DoDeleteLeath( PHBTreeBase& p );
  public:
    HBTreeBase( void );
    virtual ~HBTreeBase();

    BOOL        isLeath( void )                       const { return Left == NULL && Right == NULL; }
    BOOL        isRoot( void )                        const { return Parent == NULL; }

    BOOL        Enum( HBTreeEnum CB,HANDLE Ptr = NULL )     const;  //Enumerate all sub-leath except self in creation order
    BOOL        EnumSort( HBTreeEnum CB,HANDLE Ptr = NULL ) const;  //Enumerate all sub-leath except self in sort-inserted order
#if defined(__BCWIN32__)
    BOOL        EnumCL( HBTreeEnumCL CB )                   const;
    BOOL        EnumSortCL( HBTreeEnumCL CB )               const;
#endif
    void        Clear( void )                               { DoDeleteLeath(Left); DoDeleteLeath(Right); }
    void        UnlinkSubtree( void );                        //Remove self from b-tree, all sub-leath stay intact
    void        UnlinkSelf( void );                           //Remove self from b-tree, all sub-leath are inserted into parent tree

    BOOL        Insert( PHBTreeBase p );                      //Insert `p` as sub-leath
    PHBTreeBase Add( HANDLE nm );                             //Create and add new `nm` leath. If `nm` exist returns NULL
    PHBTreeBase Locate( HANDLE nm );                          //Locate a `nm` leath. If `nm` not exist returns NULL
};

//---------------------------------------------------------------------------
CLASSBASE( HBNumTree, public HBTreeBase )
  public:
    HANDLE ID;
  protected:
    virtual maCmp       Cmp( HANDLE Param );
    virtual maCmp       Cmp( PHBTreeBase p );
    virtual PHBTreeBase DoCreate( HANDLE Param );
  public:
    HBNumTree( HANDLE id ) { ID = id; }
};

template <class T> class HBNumPtrTree : public HBNumTree {
  protected:
    virtual PHBTreeBase DoCreate( HANDLE Param ) { return new HBNumPtrTree<T>((HANDLE)Param); }
  public:
    T Object;
  public:
    HBNumPtrTree( HANDLE id = NULL ) : HBNumTree(id) { Object = NULL; }
    ~HBNumPtrTree()                                  { FreePtr(Object); }
};

template <class T> class HBNumObjectTree : public HBNumTree {
  protected:
    virtual PHBTreeBase DoCreate( HANDLE Param ) { return new HBNumObjectTree<T>((HANDLE)Param); }
  public:
    T Object;
  public:
    HBNumObjectTree( HANDLE id ) : HBNumTree(id) {}
};

//---------------------------------------------------------------------------
template <class NameT> class HBNameTree : public HBTreeBase {
  protected:
    virtual maCmp       Cmp( PHBTreeBase p )     { return Cmp( (HANDLE)((HBNameTree<NameT>*)p)->Name.Text() ); }
    virtual maCmp       Cmp( HANDLE Param )      { return Int2Cmp(StrCmp(Name.Text(),(CONSTSTR)Param)); }
    virtual PHBTreeBase DoCreate( HANDLE Param ) { return new HBNameTree<NameT>((CONSTSTR)Param); }
  public:
    NameT Name;
  public:
    HBNameTree( CONSTSTR nm = NULL ) : Name(nm) {}
};

template <class T,class NameT> class HBNamePtrTree : public HBNameTree<NameT> {
  protected:
    virtual PHBTreeBase DoCreate( HANDLE Param ) { return new HBNamePtrTree<T,NameT>((CONSTSTR)Param); }
  public:
    T Object;
  public:
    HBNamePtrTree( CONSTSTR nm = NULL ) : HBNameTree<NameT>(nm) { Object = NULL; }
    ~HBNamePtrTree()                                            { FreePtr(Object); }
};

template <class T,class NameT> class HBNameObjectTree : public HBNameTree<NameT> {
  protected:
    virtual PHBTreeBase DoCreate( HANDLE Param ) { return new HBNameObjectTree<T,NameT>((CONSTSTR)Param); }
  public:
    T Object;
  public:
    HBNameObjectTree( CONSTSTR nm = NULL ) : HBNameTree<NameT>(nm){}
};

#endif
