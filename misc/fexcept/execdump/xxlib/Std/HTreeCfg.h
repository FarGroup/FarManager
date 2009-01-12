#ifndef __HTREE_CONFIG
#define __HTREE_CONFIG

//---------------------------------------------------------------------------
//  HConfigItem
//  HTreeConfig
//---------------------------------------------------------------------------
/*
  typedef BYTE HVType;
  enum HVTypes {
    HVVoid,
    HVByte      = 1,   //Simple data
    HVChar,
    HVWord,
    HVShort,
    HVDword,
    HVInt,
    HVFloat,
    HVDouble,
    HVHandle,         //Variable size data
    HVString
  };
*/

STRUCT( HConfigItem )
  virtual PHConfigItem      Create( CONSTSTR PathName )                = 0;
  virtual PHConfigItem      Locate( CONSTSTR PathName )                = 0;
  virtual void              Delete( PHConfigItem p )                   = 0;
  virtual void              Clear( void )                              = 0;
  virtual int               LeathCount( void )                         = 0;
  virtual int               Index( void )                              = 0;
  virtual PHConfigItem      Leath( int num )                           = 0;
  virtual PHConfigItem      Parent( void )                             = 0;
  virtual HVType            GetType( void )                            = 0;
  virtual CONSTSTR          GetName( void )                            = 0;

  virtual DWORD             Read( CONSTSTR KeyName,DWORD     Default ) = 0;
#if !defined(HASNO_BOOL)
  virtual bool              Read( CONSTSTR KeyName,bool      Default ) = 0;
#endif
#if defined(__HAS_INT64__)
  virtual __int64           Read( CONSTSTR KeyName,__int64   Default ) = 0;
#endif
  virtual BYTE              Read( CONSTSTR KeyName,BYTE      Default ) = 0;
  virtual char              Read( CONSTSTR KeyName,char      Default ) = 0;
  virtual WORD              Read( CONSTSTR KeyName,WORD      Default ) = 0;
  virtual int               Read( CONSTSTR KeyName,int       Default ) = 0;
  virtual float             Read( CONSTSTR KeyName,float     Default ) = 0;
  virtual double            Read( CONSTSTR KeyName,double    Default ) = 0;
#if defined(__VCL__)
  virtual TColor            Read( CONSTSTR KeyName,TColor    Default ) = 0;
#endif
  virtual CONSTSTR          Read( CONSTSTR KeyName,CONSTSTR  Default ) = 0;

  virtual DWORD             Write( CONSTSTR KeyName,DWORD    Value )   = 0;
#if !defined(HASNO_BOOL)
  virtual bool              Write( CONSTSTR KeyName,bool     Value )   = 0;
#endif
#if defined(__HAS_INT64__)
  virtual __int64           Write( CONSTSTR KeyName,__int64  Value )   = 0;
#endif
  virtual BYTE              Write( CONSTSTR KeyName,BYTE     Value )   = 0;
  virtual char              Write( CONSTSTR KeyName,char     Value )   = 0;
  virtual WORD              Write( CONSTSTR KeyName,WORD     Value )   = 0;
  virtual int               Write( CONSTSTR KeyName,int      Value )   = 0;
  virtual float             Write( CONSTSTR KeyName,float    Value )   = 0;
  virtual double            Write( CONSTSTR KeyName,double   Value )   = 0;
#if defined(__VCL__)
  virtual TColor            Write( CONSTSTR KeyName,TColor   Value )   = 0;
#endif
  virtual CONSTSTR          Write( CONSTSTR KeyName,CONSTSTR Value )   = 0;

  virtual LPVOID            Read( CONSTSTR KeyName,LPVOID Buff,LPVOID Default,DWORD Size ) = 0;
  virtual LPVOID            Write( CONSTSTR KeyName,LPVOID Buff,DWORD Size )               = 0;
  virtual PHConfigItem      ConfigGetCreate( CONSTSTR Name ) = 0;
};

STRUCT( HTreeConfig )
   virtual BOOL         Load( CONSTSTR PathName ) = 0;
   virtual BOOL         Save( CONSTSTR PathName ) = 0;
   virtual PHConfigItem GetRoot( void )           = 0;
};

//---------------------------------------------------------------------------
HDECLSPEC PHTreeConfig MYRTLEXP HConfigCreate( void );
HDECLSPEC void         MYRTLEXP HConfigDestroy( PHTreeConfig p );

//---------------------------------------------------------------------------
LOCALCLASS( PRConfig )
  public:
    PHTreeConfig Config;
  public:
    PRConfig( void )        { Config = HConfigCreate(); }
    ~PRConfig()             { HConfigDestroy(Config); }
};

#endif
