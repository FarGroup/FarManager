#ifndef __MY_DATAPOOL
#define __MY_DATAPOOL

/*******************************************************************
   HDPOptions     - Base for all HDataPool descendants options
   HMemTree       - b-tree descendant for use in memory data pool
   HDataPool      - pool for store and IO dynamic data
                    HDataPool may be used on many users, use HSafeObject::Release to free DataPool
     HMemDataPool - data pool to store all dynamic data in memory as b-tree
 *******************************************************************/
STRUCT( HDPOptions )
  DWORD Sizeof;  //Full size of structure
};

/*******************************************************************
   HDataPool - pool for store and IO dynamic data
 *******************************************************************/
LOCALCLASSBASE( HDataPool, public HRefObject )
  public:
    HDataPool( void )    {}

    virtual BOOL   GetOptions( PHDPOptions opt )            { return FALSE; }
    virtual BOOL   SetOptions( PHDPOptions opt )            { return FALSE; }

    virtual void   Clear( void )                            = 0;
    virtual BOOL   LocateData( HANDLE h )                   = 0;
    virtual HANDLE CreateData( LPVOID Data,DWORD sz )       = 0;
    virtual BOOL   DeleteData( HANDLE h )                   = 0;
    virtual BOOL   SetData( HANDLE h,LPVOID Data,DWORD sz ) = 0;
    virtual LPVOID GetData( HANDLE h )                      = 0;
    virtual DWORD  GetDataSize( HANDLE h )                  = 0;

    virtual BOOL   SaveItem( HANDLE h,int File )            { return FALSE; }
    virtual HANDLE LoadItem( int File )                     { return FALSE; }
    virtual BOOL   Load( CONSTSTR FileName )                { return FALSE; }
    virtual BOOL   Save( CONSTSTR FileName )                { return FALSE; }
};

HDECLSPEC PHDataPool MYRTLEXP CreateMemDataPool( void );

#endif
