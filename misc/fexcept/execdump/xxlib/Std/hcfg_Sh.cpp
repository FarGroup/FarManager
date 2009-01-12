#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#include <Std/hcfgSh.h>

HDECLSPEC PHTreeRegValueBase _LastUsedReadValue;
HDECLSPEC PHTreeRegValueBase _LastUsedWriteValue;

PHTreeRegValueBase _LastUsedReadValue = NULL;
PHTreeRegValueBase _LastUsedWriteValue = NULL;

#define DECL_READ( tp,nm )  tp SH_CfgBase::Read( CONSTSTR KeyName,tp Default )                \
                              {  PSH_CfgBase p = Locate( KeyName );                           \
                                 _LastUsedReadValue = p;                                      \
                                 if ( !p ) {                                                  \
                                   p = (PSH_CfgBase)intAdd( KeyName,(DWORD)Default );         \
                                   if ( !p ) return Default;                                  \
                                   p->Set##nm( Default );                                     \
                                 }                                                            \
                                 if ( p->Type == HV##nm || p->isDirectConvertible(HV##nm) )   \
                                   return (tp)p->Get##nm();                                   \
                                  else                                                        \
                                   return ConvertAs##nm();                                    \
                               }

#define DECL_WRITE( tp,nm ) tp SH_CfgBase::Write( CONSTSTR KeyName,tp Default )               \
                              {  PSH_CfgBase p = Locate( KeyName );                           \
                                 _LastUsedWriteValue = p;                                     \
                                 if ( !p ) {                                                  \
                                   p = (PSH_CfgBase)intAdd( KeyName,(DWORD)0 );               \
                                   if ( !p ) return Default;                                  \
                                 }                                                            \
                                 p->Set##nm( Default );                                       \
                                 return p->Get##nm(); }
/*******************************************************************
   SH_CfgBase
 *******************************************************************/
DECL_READ( DWORD,   Dword )
#if !defined(HASNO_BOOL)
//DECL_READ( bool,    Int )
bool SH_CfgBase::Read( CONSTSTR KeyName,bool Default )
                              {  PSH_CfgBase p = Locate( KeyName );
                                 _LastUsedReadValue = p;
                                 if ( !p ) {
                                   p = (PSH_CfgBase)intAdd( KeyName,(DWORD)Default );
                                   if ( !p )
                                     return Default;
                                   p->SetInt( Default );
                                 }
                                 if ( p->Type == HVInt || p->isDirectConvertible(HVInt) )
                                   return (bool)p->GetInt();
                                  else
                                   return ConvertAsInt();
                               }
#endif
DECL_READ( BYTE,    Byte )
DECL_READ( char,    Char )
DECL_READ( WORD,    Word )
DECL_READ( int,     Int )
DECL_READ( float,   Float )
DECL_READ( double,  Double )

SH_CfgBase::SH_CfgBase( void ) {}

CONSTSTR SH_CfgBase::Read( CONSTSTR KeyName,CONSTSTR Default )
  {  PSH_CfgBase p = Locate( KeyName );
     if ( !p ) p = (PSH_CfgBase)intAdd( KeyName,Default );
     if ( !p ) return Default;
     if ( p->Type != HVString ) {
       p->SetString(NULL);
       p->intSetData( (LPVOID)(Default?Default:""),(Default?(int)strlen(Default):0)+1 );
     }
 return p->GetString();
}

LPVOID SH_CfgBase::Read( CONSTSTR KeyName,LPVOID Buff,LPVOID Default,DWORD Size )
  {  PSH_CfgBase p = Locate( KeyName );

     if ( !p ) {
       if ( !Default )
         return NULL;

       p = (PSH_CfgBase)intAdd( KeyName,Default,Size );
       if ( !p )
         return NULL;
     }

     if ( p->Type != HVHandle ) {
       p->SetHandle( NULL );
       p->intSetData( Default,Size );
     }

     Size = Min( p->DataSize(), Size );
     if ( Size )
       MemMove( Buff, (const void *)p->GetHandle(), Size );

 return Buff;
}

DECL_WRITE( DWORD,   Dword )
#if !defined(HASNO_BOOL)
DECL_WRITE( bool,    Int )
#endif
DECL_WRITE( BYTE,    Byte )
DECL_WRITE( char,    Char )
DECL_WRITE( WORD,    Word )
DECL_WRITE( int,     Int )
DECL_WRITE( float,   Float )
DECL_WRITE( double,  Double )

CONSTSTR SH_CfgBase::Write( CONSTSTR KeyName,CONSTSTR Default )
  {  PSH_CfgBase p = Locate( KeyName );
     if ( !p ) p = (PSH_CfgBase)intAdd( KeyName,Default );
     if ( !p ) return Default;
     if ( p->Type != HVString ) p->SetString(NULL);
     p->intSetData( (LPVOID)(Default?Default:""),(Default?(int)strlen(Default):0)+1 );
 return p->GetString();
}

LPVOID SH_CfgBase::Write( CONSTSTR KeyName,LPVOID Buff,DWORD Size )
  {  PSH_CfgBase p = Locate( KeyName );
     if ( !p ) p = (PSH_CfgBase)intAdd( KeyName,Buff,Size );
     if ( !p ) return Buff;
     if ( p->Type != HVHandle ) p->SetHandle( NULL );
     p->intSetData( Buff,Size );
 return (LPVOID)p->GetHandle();
}

void SH_CfgBase::Delete( PSH_CfgBase p )
  {
    if (!p) p = this;
    if ( p->Tree != Tree ) return;
    if (p->Parent) p->Parent->Items.Delete(p);
}
/*******************************************************************
   SH_ConfigBase
 *******************************************************************/
SH_ConfigBase::SH_ConfigBase( void )
   : HTreeBase( CreateMemDataPool() )
  {
}

BOOL SH_ConfigBase::Load( CONSTSTR PathName )
  {
 return Root->Load(PathName);
}

BOOL SH_ConfigBase::Save( CONSTSTR PathName )
  {
 return Root->Save(PathName);
}
