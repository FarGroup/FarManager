#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

//---------------------------------------------------------------------------------
FP_ItemList::FP_ItemList( BOOL NeedToDelete )
  {
    needToDelete = NeedToDelete;
    List         = NULL;
    ItemsCount   = 0;
    MaxCount     = 0;
}

BOOL FP_ItemList::Realloc( int NewSize )
  {
    if ( !NewSize )
      return FALSE;

    if ( NewSize <= MaxCount )
      return TRUE;

    MaxCount = NewSize;

    if ( !List )
      List = (PluginPanelItem*)_Alloc( sizeof(PluginPanelItem)*MaxCount );
     else
      List = (PluginPanelItem *)_Realloc( List,sizeof(PluginPanelItem)*MaxCount );

    if ( !List ) {
      ItemsCount = 0;
      MaxCount   = 0;
      return FALSE;
    }

 return TRUE;
}
PluginPanelItem *FP_ItemList::Add( const PluginPanelItem *pi,int icn )
  {
    if ( !Realloc(ItemsCount+icn) )
      return NULL;

    PluginPanelItem *p = List + ItemsCount; //!! Do not use Item(ItemsCount) because we need point after last element
    Copy( p, pi, icn );
    ItemsCount += icn;
 return p;
}

void FP_ItemList::Free( void )
  {
    if ( needToDelete )
      Free( List,ItemsCount );

    ItemsCount = 0;
    MaxCount   = 0;
    List       = NULL;
}

void FP_ItemList::Copy( PluginPanelItem *dest,const PluginPanelItem *src,int cn )
  {
    if (!cn) return;

    MemMove( dest, src, sizeof(*dest)*cn );

    for( ; cn; cn--,src++,dest++ ) {

      //User data
      if ( IS_FLAG(src->Flags,PPIF_USERDATA) ) {
        DWORD sz = ( src->UserData && !IsBadReadPtr((void*)src->UserData,sizeof(DWORD)))
                     ? (*((DWORD*)src->UserData))
                     : 0;
        if ( sz && !IsBadReadPtr( (void*)src->UserData,sz ) ) {
          dest->UserData = (DWORD)_Alloc( sz+1 );
          MemMove( (char*)dest->UserData,(char*)src->UserData,sz );
        } else {
          dest->UserData = NULL;
          CLR_FLAG(dest->Flags,PPIF_USERDATA);
        }
      }

      //CustomColumn
      if ( src->CustomColumnNumber ) {
        dest->CustomColumnData = (LPSTR*)_Alloc( sizeof(LPSTR*)*src->CustomColumnNumber );
        for( int n = 0; n < src->CustomColumnNumber; n++ ) {
          dest->CustomColumnData[n] = StrDup(src->CustomColumnData[n]);
        }
      }
      //Description
      if ( src->Description )
        dest->Description = StrDup(src->Description);

      //Owner
      if ( src->Owner )
        dest->Owner = StrDup(src->Owner);

      //Additionals
      if ( FPIL_ADDEXIST(src) ) {
        DWORD  sz  = FPIL_ADDSIZE(src);
        LPVOID ptr = _Alloc( sz );
        if ( ptr ) {
          MemMove( ptr, FPIL_ADDDATA(src), sz );
          FPIL_ADDSET( dest, sz, ptr );
        } else
          FPIL_ADDSET( dest, 0, NULL );

      }
    }
}

void FP_ItemList::Free( PluginPanelItem *List,int count )
  {  PluginPanelItem *p = List;

    for( int i = 0; i < count; i++,List++ ) {
      //UserData
      if ( IS_FLAG(List->Flags,PPIF_USERDATA) ) {
        _Del( (char*)List->UserData );
        List->UserData = 0;
      }
      //CustomColumn
      for( int n = 0; n < List->CustomColumnNumber; n++ )
        _Del( List->CustomColumnData[n] );
      if (List->CustomColumnData)
        _Del( List->CustomColumnData );

      //Description
      if ( List->Description )
        _Del(List->Description), List->Description = NULL;

      //Owner
      if ( List->Owner )
        _Del(List->Owner), List->Owner = NULL;

      //Additionals
      if ( FPIL_ADDEXIST(List) ) {
        _Del( FPIL_ADDDATA(List) );
        List->Reserved[0] = NULL;
        List->Reserved[1] = NULL;
      }
    }
    _Del(p);
}
