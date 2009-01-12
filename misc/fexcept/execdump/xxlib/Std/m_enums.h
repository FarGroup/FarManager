#ifndef __MY_ARRAY_ENUMERATORS
#define __MY_ARRAY_ENUMERATORS

/************************************
   Enumerators:
     SCANNER  - enumerate all items in array
     QSORT    - sort array elements
     BSEARCH  - search in sorted array
 ************************************/
/*  -----------------
  DEF_SCANER     - scaner with one parameter
  DEF_SCANERv    - scanner without parameters

    Defines function inside MyXXXArray descendant for find a array item.

    DEF_SCANER( Type,Name,DataType,Actions )
       Type      - type of object stored in scanning array.
                   ("T*" for MyArray,MyRefArray; "T" for MyValArray)
       Name      - name of itterator
       DataType  - type of user data passed for itterator
       Actions   - user actions for analyze current item.
                   Can use automatically parameters:
                     `Item` - current array item
                     `Data` - user data

    SCANNER_FUNCTION:
      BOOL Cmp( Type Item, int ItemIndex, DataType Data );

      BOOL       - muist return `TRUE` for stop enumeration, `FALSE` for continue to next element
      Type       - type of array item as declared in DEF_SCANNER
      ItemIndex  - index of current array item
      Data       - user data given to scaner (valid only if not DECL_SCANERv used)

    SCANNER:
      Type Name( DataType Data );

      Type       - type of array item as declared in DEF_SCANNER definition
                   Scaner returns NULL if all elements of list was rejected by scanner-function
      Name       - name of scanner. use it as function name
      Data       - user data

    Itterator is a function that called from function that linear scans an array while
      itterator returns FALSE for given element;

    If itterator returns TRUE scan finish and return current array item.
    If itterator returns FALSE to all items in array NULL value returned by scan function.

    Template of function:
      DECL_SCANER          - Scanner: BOOL Cmp( Type Item, int ItemIndex, DataType Data );
                                Scan: Type Name( DataType Data );
      DECL_SCANERv         - Scanner: BOOL Cmp( Type Item, int ItemIndex, char );
                                Scan: Type Name( void );

    Samples:
      STRUCT( TestData )
        int value;
      };
      CLASSBASE( Test, public MyArray<PTestData> )
        DEF_SCANER( PTestData, Locate, int, { return Item->value == Data; } )
       public:
         ...
      };
      ...
      Test test;
      PTestData p;
      ...
      if ( (p=test.Locate(123)) != NULL )
        ; //Value of `123` found if array in element `p`
       else
        ; //Value of `123` NOT found if array
*/
/*
  DECL_SCANER
  DECL_SCANERv

    Declate body of scan function outside compare container

      DECL_SCANER( ArrayClassType, ArrayItemType, ScanerName, DataType )
      DECL_SCANERv( ArrayClassType, ArrayItemType, ScanerName )

    Template of function:
      DECL_SCANER          - Scanner: BOOL Cmp( Type Item, int ItemIndex, DataType Data );
                                Scan: Type Name( DataType Data );
      DECL_SCANERv         - Scanner: BOOL Cmp( Type Item, int ItemIndex, char );
                                Scan: Type Name( void );

    Sample:
      CLASSBASE( Test, public MyArray<PtestData> )
        DEF_SCANER( PTestData, Locate, int, ; )
       public:
         ...
      };
      ...
      DECL_SCANER( PTestData, Test, Locate, int )
        {
       return Item->value == Data;
      }
*/
#define DEF_SCANER( tp,nm,d,dz )        struct Finder_##nm { BOOL Compare( tp Item, int ItemIndex, d Data ) dz }; tp nm( d Data ) { return Enum( Finder_##nm##(),Data ); }
#define DECL_SCANER( cl,tp,nm,d )       BOOL cl::Finder_##nm::Compare( tp Item,int ItemIndex, d Data )
#define DEF_SCANERv( tp,nm,dz )         struct FinderV_##nm { BOOL Compare( tp Item,int ItemIndex, char ) dz }; tp nm( void ) { return Enum( FinderV_##nm##(),'\x0' ); }
#define DECL_SCANERv( cl,tp,nm )        BOOL cl::FinderV_##nm::Compare( tp Item,int ItemIndex, char )

/*  -----------------
  DEF_QSORT
    Declare function to sort template MyXXXArray arrays

    DEF_QSORT( Type,Name,Actions )
      Type     - type of object stored in scanning array.
                 "T" for MyArray,MyRefArray,MyValArray
      Name     - name of sort function
      Actions  - user-defined body of compare function

    SORTER_FUNCTION:
      int Compare( Type Left,Type Right ) const;

      int      - must return difference between two array elements.
                   0  - elements are equal
                   <0 - Left element less then Right
                   >0 - Left element bigger then Right
      Type     - array element type as declared in DEF_QSORT
      Left     - left compared object
      Right    - right compared object

    SORTER
      void Name( void );

    Template of function:
      Comparer of elements:     int RTL_CALLBACK Compare( Type Left,Type Right ) const { ... }
                    Sorter:     void Name( void );

*/
#define DEF_QSORT( tp,nm,dz )  static int RTL_CALLBACK QSort_##nm##Compare( const tp *Left,const tp *Right ) dz void nm( void ) { Sort( QSort_##nm##Compare ); }
#define DECL_QSORT( cl,tp,nm ) int RTL_CALLBACK cl::QSort_##nm##Compare( const tp *Left,const tp *Right )

/*  -----------------
   DEF_BSEARCH      - declare binary search procedure in sorted array
   DECL_BSEARCH     - place the body of search procedure
   DEF_BSEARCH_SORT - declare bsearch procedure, that use a QSort comparer for search

   Declare searcher, that search array for key.
   Returns -1 if search fail or index of found element.
   Array must be sorted before search used.

   DEF_BSEARCH( Type,Name,Actions )
      Type     - type of object stored in scanning array.
                 "T" for MyArray,MyRefArray,MyValArray
      Name     - name of sort function
      Actions  - user-defined body of compare function

   SEARCH_FUNCTION
     int Comparerer( const Type& Item,const Type& Key )

      int      - must return difference between two array elements.
                   0  - elements are equal
                   <0 - Item element less then Key
                   >0 - Item element bigger then Key
      Type     - array element type as declared in DEF_BSEARCH
      Item     - currently compared array object
      Key      - searched key

   SEARCHER
     int Name( const Type& Key )
       int      - returns index of found elemnt if element has been found
                  or -1 if not found
       Type     - array element type as declared in DEF_BSEARCH
       Key      - searched key in form of one of array elemnt ready to be compared
                  with other array objects

   Declarations:
     int Comparerer( const Type& Item,const Type& Key )
     int Name( const Type& Key )
*/
#define DEF_BSEARCH( tp,nm,dz )  static int RTL_CALLBACK BSearch_##nm##Compare( const tp *Left,const tp *Right ) dz int nm( const tp& key,int *pos = NULL ) { return Search( key,BSearch_##nm##Compare,pos ); }
#define DECL_BSEARCH( cl,tp,nm ) int RTL_CALLBACK cl::BSearch_##nm##Compare( const tp *Left,const tp *Right )

#endif
