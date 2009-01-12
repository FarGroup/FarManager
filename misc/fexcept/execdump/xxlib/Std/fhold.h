#ifndef __CT_DATAHOLDERS
#define __CT_DATAHOLDERS

/***************************************
               DataHolder
 ***************************************/
LOCALCLASS( CTDataHolder )
  protected:
    BOOL OK;
  public:
    CTDataHolder( void )    { OK = TRUE; }
    virtual ~CTDataHolder() {}

    virtual BOOL     isOK( void )                        { return OK; }  //Result of create holder
    virtual BOOL     Assign( CONSTSTR data,DWORD sz )    = 0;  //Setup data holder
    virtual BYTE     Chr( DWORD num )                    = 0;  //Get char from data
    virtual PBYTE    GetData( DWORD off, DWORD size )    = 0;  //Get data
    virtual DWORD    GetSize( void )                     = 0;  //Get data size
    virtual CONSTSTR GetName( void )                     = 0;  //Get data name (FileName f.e.)
    virtual void     SetupMinimumSize( DWORD sz )        = 0;  //Set minimum size of data getted by pointer
    virtual BOOL     StoreData( DWORD off, PBYTE data,
                                        DWORD size )  = 0;  //Set data to holder
    virtual void     Reload( void )                      = 0;  //Reload data possible changed by other
};

#define CT_DATA_WINDOWSIZE 1000

CLASSBASE( CTFileHolder, public CTDataHolder )
    PBYTE              Data;       //Window data
    DWORD              winSize,    //Size of data window
                       Pos;        //Position of window in file
    PathString         File;       //File
    DWORD              readedSize, //Size of readed data from file to window
                       Size;       //Size of data in file
    int                file;

  protected:
    BOOL ReloadData( void );

  public:
    CTFileHolder( CONSTSTR fname, DWORD wSize = CT_DATA_WINDOWSIZE );
    CTFileHolder( void );
    ~CTFileHolder();

    void Close( void );
    int  FHandle( void );

    virtual BOOL     Assign( CONSTSTR data,DWORD sz/*=0 | new window size*/ );
    virtual BYTE     Chr( DWORD num );
    virtual PBYTE    GetData( DWORD off, DWORD size );
    virtual CONSTSTR GetName( void );
    virtual DWORD    GetSize( void );
    virtual void     Reload( void );
    virtual void     SetupMinimumSize( DWORD sz );
    virtual BOOL     StoreData( DWORD off, PBYTE data,DWORD size );
};

CLASSBASE( CTDumpHolder, public CTDataHolder )
    char   Name[ 50 ];
    PBYTE  Data;        //pointer to data
    DWORD  Size;        //Size of data

  public:
    CTDumpHolder( LPCBYTE data,DWORD sz );
    CTDumpHolder( CONSTSTR data );

    virtual BOOL     Assign( CONSTSTR data,DWORD sz );
    virtual BYTE     Chr( DWORD off );
    virtual PBYTE    GetData( DWORD off, DWORD size );
    virtual DWORD    GetSize( void );
    virtual CONSTSTR GetName( void );
    virtual void     SetupMinimumSize( DWORD );
    virtual void     Reload( void );
    virtual BOOL     StoreData( DWORD off, PBYTE data,DWORD size );
};


#endif
