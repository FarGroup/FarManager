#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#include <Std/hcfgSh.h>

/*******************************************************************
   SH_Cfg1
 *******************************************************************/
SH_Cfg1::SH_Cfg1( CONSTSTR nm )
   : _Name(nm)
  {
}

PHTreeValue SH_Cfg1::DoCreate( CONSTSTR nm ) { return new SH_Cfg1(nm); }
CONSTSTR    SH_Cfg1::ClassName( void )       { return SH_CFG_VERSION1; }
CONSTSTR    SH_Cfg1::GetName( void )         { return _Name.Text(); }

/*******************************************************************
   SH_Config1
 *******************************************************************/
SH_Config1::SH_Config1( void )
  {
    Root = new SH_Cfg1("");
    Root->Tree = this;
}

SH_Config1::~SH_Config1()
  {
    delete Root;
}
