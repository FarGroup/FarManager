#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#include <Std/hcfgSh.h>

LOCALPRESTRUCT( HConfigItem_SharedInt );

/*******************************************************************
   ITEM
 *******************************************************************/
LOCALSTRUCTBASE( HConfigItem_Int, public HConfigItem )
    PHConfigItem_SharedInt Value;

    virtual int           LeathCount( void );
    virtual int           Index( void );
    virtual PHConfigItem  Leath( int num );
    virtual PHConfigItem  Parent( void );
    virtual HVType        GetType( void );
    virtual CONSTSTR      GetName( void );
    virtual void          Clear( void );

    virtual DWORD       Read( CONSTSTR KeyName,DWORD     Default );
#if !defined(HASNO_BOOL)
    virtual bool        Read( CONSTSTR KeyName,bool      Default );
#endif
#if defined(__HAS_INT64__)
    virtual __int64     Read( CONSTSTR KeyName,__int64   Default );
#endif
    virtual BYTE        Read( CONSTSTR KeyName,BYTE      Default );
    virtual char        Read( CONSTSTR KeyName,char      Default );
    virtual WORD        Read( CONSTSTR KeyName,WORD      Default );
    virtual int         Read( CONSTSTR KeyName,int       Default );
    virtual float       Read( CONSTSTR KeyName,float     Default );
    virtual double      Read( CONSTSTR KeyName,double    Default );
#if defined(__VCL__)
    virtual TColor      Read( CONSTSTR KeyName,TColor    Default );
#endif
    virtual CONSTSTR    Read( CONSTSTR KeyName,CONSTSTR  Default );
    virtual LPVOID      Read( CONSTSTR KeyName,LPVOID Buff,LPVOID Default,DWORD Size );

    virtual DWORD       Write( CONSTSTR KeyName,DWORD    Value );
#if !defined(HASNO_BOOL)
    virtual bool        Write( CONSTSTR KeyName,bool     Value );
#endif
#if defined(__HAS_INT64__)
    virtual __int64     Write( CONSTSTR KeyName,__int64  Value );
#endif
    virtual BYTE        Write( CONSTSTR KeyName,BYTE     Value );
    virtual char        Write( CONSTSTR KeyName,char     Value );
    virtual WORD        Write( CONSTSTR KeyName,WORD     Value );
    virtual int         Write( CONSTSTR KeyName,int      Value );
    virtual float       Write( CONSTSTR KeyName,float    Value );
    virtual double      Write( CONSTSTR KeyName,double   Value );
#if defined(__VCL__)
    virtual TColor      Write( CONSTSTR KeyName,TColor   Value );
#endif
    virtual CONSTSTR    Write( CONSTSTR KeyName,CONSTSTR Value );
    virtual LPVOID      Write( CONSTSTR KeyName,LPVOID Buff,DWORD Size );

    virtual PHConfigItem Create( CONSTSTR PathName );
    virtual PHConfigItem Locate( CONSTSTR PathName );
    virtual void         Delete( PHConfigItem p );
    virtual PHConfigItem ConfigGetCreate( CONSTSTR Name );
};

LOCALSTRUCTBASE( HConfigItem_SharedInt, public SH_Cfg )
  protected:
   virtual PHTreeValue DoCreate( CONSTSTR nm ) { return new HConfigItem_SharedInt(nm); }
  public:
   HConfigItem_Int  IntCfg;
  public:
    HConfigItem_SharedInt( CONSTSTR nm ) : SH_Cfg(nm) { IntCfg.Value = this; }

    PHConfigItem ConfigGetCreate( CONSTSTR Name );
};

// ----------------------------------------------------------------------------------
#define DECL_CFG_TYPE( tp ) tp HConfigItem_Int::Read( CONSTSTR KeyName,tp Default ) { return Value->Read(KeyName,Default); } \
                            tp HConfigItem_Int::Write( CONSTSTR KeyName,tp val )    { return Value->Write(KeyName,val); }

DECL_CFG_TYPE( CONSTSTR )
DECL_CFG_TYPE( DWORD    )
#if !defined(HASNO_BOOL)
DECL_CFG_TYPE( bool     )
#endif
DECL_CFG_TYPE( BYTE     )
DECL_CFG_TYPE( char     )
DECL_CFG_TYPE( WORD     )
DECL_CFG_TYPE( int      )
DECL_CFG_TYPE( float    )
DECL_CFG_TYPE( double   )
#if defined(__VCL__)
DECL_CFG_TYPE( TColor   )
#endif

#define DECL_CFG_TYPE_AS_BIN( tp ) \
  tp HConfigItem_Int::Read( CONSTSTR KeyName,tp Default ) { Value->Read(KeyName,&Default,&Default,sizeof(Default)); return Default; } \
  tp HConfigItem_Int::Write( CONSTSTR KeyName,tp val )    { Value->Write(KeyName,&val,sizeof(val)); return val; }

#if defined(__HAS_INT64__)
DECL_CFG_TYPE_AS_BIN( __int64 )
#endif

LPVOID       HConfigItem_Int::Read( CONSTSTR KeyName,LPVOID Buff,LPVOID Default,DWORD Size ){ return Value->Read(KeyName,Buff,Default,Size); }
LPVOID       HConfigItem_Int::Write( CONSTSTR KeyName,LPVOID Buff,DWORD Size )              { return Value->Write(KeyName,Buff,Size); }
PHConfigItem HConfigItem_Int::Create( CONSTSTR PathName )                                   { PHConfigItem_SharedInt p = (PHConfigItem_SharedInt)Value->Create(PathName); return (p)?(&p->IntCfg):NULL; }
PHConfigItem HConfigItem_Int::Locate( CONSTSTR PathName )                                   { PHConfigItem_SharedInt p = (PHConfigItem_SharedInt)Value->Locate(PathName); return (p)?(&p->IntCfg):NULL; }
void         HConfigItem_Int::Delete( PHConfigItem p )                                      { Value->Delete( ((HConfigItem_Int*)(p?p:this))->Value ); }
int          HConfigItem_Int::LeathCount( void )                                            { return Value->Items.Count(); }
int          HConfigItem_Int::Index( void )                                                 { return (Value->Parent)?Value->Parent->Items.IndexOf(Value):0; }
PHConfigItem HConfigItem_Int::Leath( int num )                                              { PHConfigItem_SharedInt p = (PHConfigItem_SharedInt)Value->Items.Item(num); return (p)?(&p->IntCfg):NULL; }
PHConfigItem HConfigItem_Int::Parent( void )                                                { PHConfigItem_SharedInt p = (PHConfigItem_SharedInt)Value->Parent; return (p)?(&p->IntCfg):NULL; }
HVType       HConfigItem_Int::GetType( void )                                               { return Value->Type; }
CONSTSTR     HConfigItem_Int::GetName( void )                                               { return Value->Name(); }
void         HConfigItem_Int::Clear( void )                                                 { Value->Clear(); }
PHConfigItem HConfigItem_Int::ConfigGetCreate( CONSTSTR Name )                              { return Value->ConfigGetCreate(Name); }

// ----------------------------------------------------------------------------------
PHConfigItem HConfigItem_SharedInt::ConfigGetCreate( CONSTSTR Name )
  {  PHConfigItem_SharedInt p = (PHConfigItem_SharedInt)Locate(Name);
     if ( p ) return &p->IntCfg;
     p = (PHConfigItem_SharedInt)Create(Name);
     if ( p ) return &p->IntCfg;
 return NULL;
}
/*******************************************************************
   TREE
 *******************************************************************/
LOCALPRECLASS( HTreeConfig_SharedInt );

LOCALSTRUCTBASE( HTreeConfig_Int, public HTreeConfig )
   PHTreeConfig_SharedInt Value;

   virtual BOOL         Load( CONSTSTR PathName );
   virtual BOOL         Save( CONSTSTR PathName );
   virtual PHConfigItem GetRoot( void );
};

LOCALCLASSBASE( HTreeConfig_SharedInt, public SH_ConfigBase )
  public:
    HTreeConfig_Int CfgInt;
  public:
    HTreeConfig_SharedInt( void ) { Root = new HConfigItem_SharedInt(""); Root->Tree = this; CfgInt.Value = this; }
    ~HTreeConfig_SharedInt()      { delete Root; }
};

BOOL         HTreeConfig_Int::Load( CONSTSTR PathName )        { return Value->Load(PathName); }
BOOL         HTreeConfig_Int::Save( CONSTSTR PathName )        { return Value->Save(PathName); }
PHConfigItem HTreeConfig_Int::GetRoot( void )                  { return &((PHConfigItem_SharedInt)Value->Root)->IntCfg; }

/*******************************************************************
   Interface
 *******************************************************************/
HDECLSPEC PHTreeConfig MYRTLEXP HConfigCreate( void )
  {  PHTreeConfig_SharedInt p = new HTreeConfig_SharedInt;
   return &p->CfgInt;
}

HDECLSPEC void MYRTLEXP HConfigDestroy( PHTreeConfig p )
  {
     if ( p )
       delete ((PHTreeConfig_Int)p)->Value;
}
