#ifndef __RTS_SHARED_CONFIG
#define __RTS_SHARED_CONFIG

/*******************************************************************
   Shared configs
   Classes:
     SH_Cfg1         - implementation of config item version 1
     SH_Config1      - implementation of base config tree version 1
   Macros:
     SH_CFG_VERSION  - last implemented version
     SH_Cfg          - last implemented config item
     SH_ConfigTree   - last implemented config tree

   For declare user configs u can do:
     1) Use configs exactly as it is.
        To do so simple use SH_CfgXX and SH_ConfigXX
     2) Write descendants to include special data in config items
        To do so:
          - write descendants for config item using SH_CfgXX as a base class
          - write base tree config using !! SH_ConfigBase
            as a base class and create a Root object of your config item class in it
            (see SH_Config1 implemetation for details)

 *******************************************************************/
/*******************************************************************
   BASE
 *******************************************************************/
STRUCTBASE( SH_CfgBase, public HTreeRegValueBase )
  public:
    SH_CfgBase( void );

    DWORD       Read( CONSTSTR KeyName,DWORD     Default );
#if !defined(HASNO_BOOL)
    bool        Read( CONSTSTR KeyName,bool      Default );
#endif
    BYTE        Read( CONSTSTR KeyName,BYTE      Default );
    char        Read( CONSTSTR KeyName,char      Default );
    WORD        Read( CONSTSTR KeyName,WORD      Default );
    int         Read( CONSTSTR KeyName,int       Default );
    float       Read( CONSTSTR KeyName,float     Default );
    double      Read( CONSTSTR KeyName,double    Default );
#if defined(__VCL__)
    TColor      Read( CONSTSTR KeyName,TColor    Default ) { return (TColor)Read( KeyName,(DWORD)Default ); }
#endif
    CONSTSTR    Read( CONSTSTR KeyName,CONSTSTR  Default );
    LPVOID      Read( CONSTSTR KeyName,LPVOID Buff,LPVOID Default,DWORD Size );

    DWORD       Write( CONSTSTR KeyName,DWORD    Value );
#if !defined(HASNO_BOOL)
    bool        Write( CONSTSTR KeyName,bool     Value );
#endif
    BYTE        Write( CONSTSTR KeyName,BYTE     Value );
    char        Write( CONSTSTR KeyName,char     Value );
    WORD        Write( CONSTSTR KeyName,WORD     Value );
    int         Write( CONSTSTR KeyName,int      Value );
    float       Write( CONSTSTR KeyName,float    Value );
    double      Write( CONSTSTR KeyName,double   Value );
#if defined(__VCL__)
    TColor      Write( CONSTSTR KeyName,TColor   Value )   { return (TColor)Write( KeyName,(DWORD)Value ); }
#endif
    CONSTSTR    Write( CONSTSTR KeyName,CONSTSTR Value );
    LPVOID      Write( CONSTSTR KeyName,LPVOID Buff,DWORD Size );

    PSH_CfgBase Create( CONSTSTR PathName )  { return (PSH_CfgBase )intAdd(PathName); }
    PSH_CfgBase Locate( CONSTSTR PathName )  { return (PSH_CfgBase )intLocate(PathName); }
    void        Delete( PSH_CfgBase p );

    BOOL        Load( CONSTSTR PathName )    { return intLoad(PathName); }
    BOOL        Save( CONSTSTR PathName )    { return intSave(PathName); }
};

CLASSBASE( SH_ConfigBase, public HTreeBase )
  public:
    PSH_CfgBase Root;
  public:
    SH_ConfigBase( void );

    BOOL Load( CONSTSTR PathName );
    BOOL Save( CONSTSTR PathName );
};


/*******************************************************************
   VERSION 1 [shcfg1.cpp]
 *******************************************************************/
#define SH_CFG_VERSION1  "RTSm0001"

STRUCTBASE( SH_Cfg1, public SH_CfgBase )
   MyString  _Name;
  protected:
   virtual PHTreeValue DoCreate( CONSTSTR nm );
   virtual CONSTSTR    ClassName( void );
   virtual CONSTSTR    GetName( void );
  public:
    SH_Cfg1( CONSTSTR nm );
};

CLASSBASE( SH_Config1, public SH_ConfigBase )
  public:
    SH_Config1( void );
    ~SH_Config1();
};

#define SH_CFG_VERSION   SH_CFG_VERSION1
#define SH_Cfg           SH_Cfg1
#define SH_ConfigTree    SH_Config1

#endif
