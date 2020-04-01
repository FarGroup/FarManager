#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

/* COPYRIGHT: Robin Hilliard, Lough Guitane, Killarney, Co. Kerry, Ireland.
              Tel:         [+353] 64-54014
              Internet:    softloft@iruccvax.ucc.ie
              Compu$erve:  100042, 1237

   If you feel like registering, and possibly get notices of updates and
   other items of software, then send me a post card of your home town.
*/

#include <Std/asmtable.inc>

/* variables controlled by command line flags */
static PUnassembeInfo      Info;
static int                 patch87;     /* fudge variable used in 8087 emu patching code */
static char               *ubufp;
static int                 wordop;           /* dealing with word or byte operand */
static int                 must_do_size;  /* used with do_size */
static int                 prefix;            /* segment override prefix byte */
static int                 modrmv;            /* flag for getting modrm byte */
static int                 sibv;              /* flag for getting sib byte   */
static int                 opsize;            /* just like it says ...       */
static int                 addrsize;
static int                 do_distance;       /* default is to use reassemblable instructions */
static jmp_buf             reached_eof; /* jump back when reached eof */

// some defines for extracting instruction bit fields from bytes
#define MOD(a)    (((a)>>6)&7)
#define REG(a)    (((a)>>3)&7)
#define RM(a)     ((a)&7)
#define SCALE(a)  (((a)>>6)&7)
#define INDEX(a)  (((a)>>3)&7)
#define BASE(a)   ((a)&7)

// prototypes
static BYTE      getbyte(void);
static void      ua_str(const char*);
static CONSTSTR  addr_to_hex( DWORD addr,int useSign,int opSize );
static int       modrm(void);
static int       sib(void);
static int       uprintf(const char *, ...);
static void      uputchar(char );
static int       bytes(char );
static void      outhex(char , int , int , int , int );
static void      reg_name(int , char );
static void      do_sib(int );
static void      do_modrm(char );
static void      floating_point(int );
static void      percent( char, char );
static const char* GetOP( BYTE Num );
static const char* GetOPGroup( char subtype );

CONSTSTR UnassembeInfo::HexDigit( DWORD Value,char Sign,BOOL Compress )
  {  static char str[ 20 ];
     char  ch,*m;

     if ( Sign == ' ' )
       if ( ((signed)Value) < 0 ) {
         Value = (DWORD)( -((signed)Value) );
         Sign = '-';
       } else
         Sign = '+';

     m = str + 2 + SNprintf( str+2,sizeof(str)-2, (Sign || Compress) ? "%X" : "%08X", Value );

     if ( IS_FLAG(Flags,DISASM_HEXOUT) && Value > 9 )
       *m++ = 'h';
     *m = 0;

     ch = (char)toupper(str[2]);

     if ( ch >= 'A' && ch <= 'Z' ) {
       str[1] = '0';
       if ( Sign ) {
         str[0] = Sign;
         return str;
       } else
         return str+1;
     } else
     if ( Sign ) {
       str[1] = Sign;
       return str+1;
     } else
       return str+2;
}

static CONSTSTR addr_to_hex(DWORD addr, int useSign, int opSize)
  {  char signchar = (char)( useSign ? '+' : 0 );

//Create digit
     if ( useSign || !IS_FLAG(Info->CurrentFlags,DISASM_FL_CODE) )
       switch( opSize ) {
         case 1: if ( ((signed char)addr) < 0 ) {
                   addr     = (signed long)( -((signed char)addr) );
                   signchar = '-';
                 }
              break;
         case 2: if ( ((signed short)addr) < 0 ) {
                   addr     = (signed long)( -((signed short)addr) );
                   signchar = '-';
                 }
              break;
         case 4:
        default: opSize = 4;
                 if ( ((signed long)addr) < 0 ) {
                   addr     = (signed long)( -((signed long)addr) );
                   signchar = '-';
                 }
              break;
       }

//Call handler
     Info->Operand( addr,signchar,opSize );

 return Info->HexDigit( addr, signchar, TRUE );
}

/*
   only one modrm or sib byte per instruction, tho' they need to be
   returned a few times...
*/
static int  modrm(void)
{
  if (modrmv == -1)
    modrmv = getbyte();
  return modrmv;
}

static int  sib(void)
{
  if (sibv == -1)
    sibv = getbyte();
  return sibv;
}

/*------------------------------------------------------------------------*/
static int uprintf( const char *format, ...)
  {  int     ret;
     va_list argptr;
     char    str[ 100 ];

    va_start(argptr,format);
      ret = vsprintf(str,format,argptr);
    va_end(argptr);

    for( char *m = str; *m; m++ )
      uputchar( *m );

  return ret;
}

static void uputchar(char c)
{  DWORD stage = Info->CurrentFlags & DISASM_FL_OPERAND;

  if ( !ubufp ) {
    ubufp = Info->CommandBuffer;
    Info->CurrentFlags = (Info->CurrentFlags & (~DISASM_FL_OPERAND)) | DISASM_FL_COMMAND;
  } else
  if ( stage == DISASM_FL_COMMAND && c == '\t' ) {
    *ubufp = 0;
    ubufp = Info->LeftBuffer;
    Info->CurrentFlags = (Info->CurrentFlags & (~DISASM_FL_OPERAND)) | DISASM_FL_LEFT;
    return;
  } else
  if ( stage == DISASM_FL_LEFT && c == ',' ) {
    *ubufp = 0;
    ubufp = Info->RightBuffer;
    Info->CurrentFlags = (Info->CurrentFlags & (~DISASM_FL_OPERAND)) | DISASM_FL_RIGHT;
    return;
  }

  *ubufp++ = c;
}

/*------------------------------------------------------------------------*/
static int  bytes(char c)
{
       if(c == 'b') return 1;
  else if(c == 'w') return 2;
  else if(c == 'd') return 4;
  else if(c == 'v')
  {
    if (opsize == 32)
       return 4;
    else
      return 2;
  }
  return 0;
}

/*------------------------------------------------------------------------*/
static void  outhex( char subtype, int extend, int optional, int defsize, int sign)
{
  int    n=0,
         s=0,
         i;
  BYTE   buff[6];
  signed long int delta;

  switch (subtype) {
  case 'q': if (wordop)
              n = ( opsize == 32 ) ? 4 : 2;
             else
              n = 1;
            break;
  case 'a':                    break;
  case 'x': extend = 2; n = 1; break;
  case 'b': n = 1;             break;
  case 'w': n = 2;             break;
  case 'd': n = 4;             break;
  case 's': n = 6;             break;
  case 'c':
  case 'v': n = (defsize == 32) ? 4 : 2; break;
  case 'p': n = (defsize == 32) ? 6 : 4; s = 1; break;
  }

  for (i=0; i<n; i++)
    buff[i] = getbyte();

  for (; i<extend; i++)
    buff[i] = (BYTE) ((buff[i-1] & 0x80) ? '\xff' : '\x0');

  if (s) {
    uprintf("%02X%02X:", buff[n-1], buff[n-2]);
    n -= 2;
  }

  delta = 0;
  switch (n) {
    case 1: delta = *(signed char *)buff; break;
    case 2: delta = *(signed int *)buff;  break;
    case 4: delta = *(signed long *)buff; break;
  }

  if (extend > n)
    uprintf( "%s",addr_to_hex(delta,subtype!='x',extend) );
   else
    uprintf( "%s", addr_to_hex(delta,sign,n) );
}

/*------------------------------------------------------------------------*/
static void  reg_name(int regnum, char size)
{
  if (size == 'F') { /* floating point register? */
    uprintf("st(%d)", regnum);
    return;
  }
  if (((size == 'v') && (opsize == 32)) || (size == 'd'))
    uputchar('e');
  if ((size=='q' || size == 'b' || size=='c') && !wordop) {
    uputchar("acdbacdb"[regnum]);
    uputchar("llllhhhh"[regnum]);
  } else {
    uputchar("acdbsbsd"[regnum]);
    uputchar("xxxxppii"[regnum]);
  }
}

/*------------------------------------------------------------------------*/
static void  do_sib(int m)
{
  int s, i, b;

  s = SCALE(sib());
  i = INDEX(sib());
  b = BASE(sib());

  switch (b) {     /* pick base */
    case 0: ua_str("%p:[eax"); break;
    case 1: ua_str("%p:[ecx"); break;
    case 2: ua_str("%p:[edx"); break;
    case 3: ua_str("%p:[ebx"); break;
    case 4: ua_str("%p:[esp"); break;
    case 5: if (m == 0) {
              ua_str("%p:[");
              //Flags already set
              outhex('d', 4, 0, addrsize, 0);
            } else {
              ua_str("%p:[ebp");
            }
            break;
    case 6: ua_str("%p:[esi"); break;
    case 7: ua_str("%p:[edi"); break;
  }

  switch (i) {     /* and index */
    case 0: uprintf("+eax"); break;
    case 1: uprintf("+ecx"); break;
    case 2: uprintf("+edx"); break;
    case 3: uprintf("+ebx"); break;
    case 4: break;
    case 5: uprintf("+ebp"); break;
    case 6: uprintf("+esi"); break;
    case 7: uprintf("+edi"); break;
  }

  if (i != 4)
    switch (s) {    /* and scale */
      case 0: uprintf(""); break;
      case 1: uprintf("*2"); break;
      case 2: uprintf("*4"); break;
      case 3: uprintf("*8"); break;
    }
}

/*------------------------------------------------------------------------*/
static void  do_modrm(char subtype)
{
  int mod = MOD(modrm());
  int rm = RM(modrm());
  int extend = (addrsize == 32) ? 4 : 2;

/* specifies two registers */
  if (mod == 3) {
    reg_name(rm, subtype);
    return;
  }

  if (must_do_size) {
    if (wordop) {
      if (addrsize==32 || opsize==32)        /* then must specify size */
        uprintf( Info->GetStringName(DISASM_ID_DWORD_PTR) );
       else
        uprintf( Info->GetStringName(DISASM_ID_WORD_PTR) );
    } else
      uprintf( Info->GetStringName(DISASM_ID_BYTE_PTR) );

    uputchar(' ');
  }

/* mem operand with 32 bit ofs */
  if ((mod == 0) && (rm == 5) && (addrsize == 32)) {
    ua_str("%p:[");
    SET_FLAG( Info->CurrentFlags,DISASM_FL_REF );
    outhex('d', extend, 0, addrsize, 0);
    CLR_FLAG( Info->CurrentFlags,DISASM_FL_REF );
    uputchar(']');
  } else
/* 16 bit dsplcmnt */
  if ((mod == 0) && (rm == 6) && (addrsize == 16)) {
    ua_str("%p:[");
    SET_FLAG( Info->CurrentFlags,DISASM_FL_REF );
    outhex('w', extend, 0, addrsize, 0);
    CLR_FLAG( Info->CurrentFlags,DISASM_FL_REF );
    uputchar(']');
  } else {
/*All other*/
    if ( (addrsize != 32) || (rm != 4) )
      ua_str("%p:[");

    SET_FLAG( Info->CurrentFlags,DISASM_FL_REF | DISASM_FL_REFADD );

    if (addrsize == 16)
      switch (rm) {
        case 0: uprintf("bx+si"); break;
        case 1: uprintf("bx+di"); break;
        case 2: uprintf("bp+si"); break;
        case 3: uprintf("bp+di"); break;
        case 4: uprintf("si"); break;
        case 5: uprintf("di"); break;
        case 6: uprintf("bp"); break;
        case 7: uprintf("bx"); break;
      }
     else
      switch (rm) {
        case 0: uprintf("eax"); break;
        case 1: uprintf("ecx"); break;
        case 2: uprintf("edx"); break;
        case 3: uprintf("ebx"); break;
        case 4: do_sib(mod); break;
        case 5: uprintf("ebp"); break;
        case 6: uprintf("esi"); break;
        case 7: uprintf("edi"); break;
      }

    switch (mod) {
      case 1: outhex('b', extend, 1, addrsize, 0); break;
      case 2: outhex('v', extend, 1, addrsize, 1); break;
    }

    CLR_FLAG( Info->CurrentFlags,DISASM_FL_REF | DISASM_FL_REFADD );
    uputchar(']');
  }
}

/*------------------------------------------------------------------------*/
static void  floating_point(int e1)
{
  int esc = e1*8 + REG(modrm());

  if (MOD(modrm()) == 3) {
    if (fspecial[esc]) {
      if (fspecial[esc][0][0] == '*') {
        ua_str(fspecial[esc][0]+1);
      } else {
        ua_str(fspecial[esc][RM(modrm())]);
      }
    } else {
      ua_str(floatops[esc]);
      ua_str(" %EF");
    }
  } else {
    ua_str(floatops[esc]);
    ua_str(" %EF");
  }
}

/*------------------------------------------------------------------------*/
/* Main table driver                                                      */
static void  percent( char type, char subtype )
{
  DWORD vofs;
  int   extend = (addrsize == 32) ? 4 : 2;
  BYTE  c;

  switch (type) {
  case 'A':                          /* direct address */
       SET_FLAG( Info->CurrentFlags,DISASM_FL_CODE );
       outhex(subtype, extend, 0, addrsize, 0);
       CLR_FLAG( Info->CurrentFlags,DISASM_FL_CODE );
       break;

  case 'C':                          /* reg(r/m) picks control reg */
       uprintf("C%d", REG(modrm()));
       must_do_size = 0;
       break;

  case 'D':                          /* reg(r/m) picks debug reg */
       uprintf("D%d", REG(modrm()));
       must_do_size = 0;
       break;

  case 'E':                          /* r/m picks operand */
       do_modrm(subtype);
       break;

  case 'G':                          /* reg(r/m) picks register */
       if (subtype == 'F')                 /* 80*87 operand?   */
         reg_name(RM(modrm()), subtype);
       else
         reg_name(REG(modrm()), subtype);
       must_do_size = 0;
       break;

  case 'I':                            /* immed data */
       SET_FLAG( Info->CurrentFlags,DISASM_FL_DATA );
       outhex(subtype, 0, 0, opsize, 0);
       CLR_FLAG( Info->CurrentFlags,DISASM_FL_DATA );
       break;

  case 'J':                            /* relative IP offset */
       vofs = 0;
       switch(bytes(subtype)) {              /* sizeof offset value */
         case 1:
              vofs = (DWORD)getbyte();
            break;
         case 2:
              vofs  = (DWORD)getbyte();
              vofs |= (DWORD)getbyte() << 8;
              vofs &= 0xFFFFu;
            break;
         case 4:
              vofs  = (DWORD)getbyte();           /* yuk! */
              vofs |= (DWORD)getbyte() << 8;
              vofs |= (DWORD)getbyte() << 16;
              vofs |= (DWORD)getbyte() << 24;
            break;
       }
       SET_FLAG( Info->CurrentFlags,DISASM_FL_CODE|DISASM_FL_OFFSET );
       uprintf("%s", addr_to_hex(vofs + Info->instruction_length,1,bytes(subtype)) );
       CLR_FLAG( Info->CurrentFlags,DISASM_FL_CODE|DISASM_FL_OFFSET );
       break;

  case 'K': if (do_distance==0)
              break;
            switch (subtype) {
              case 'f': uprintf( Info->GetStringName(DISASM_ID_FAR) );   uputchar(' '); break;
              case 'n': uprintf( Info->GetStringName(DISASM_ID_NEAR) );  uputchar(' '); break;
              case 's': uprintf( Info->GetStringName(DISASM_ID_SHORT) ); uputchar(' '); break;
            }
       break;

  case 'M':                            /* r/m picks memory */
       do_modrm(subtype);
       break;

  case 'O':                            /* offset only */
       ua_str("%p:[");
       SET_FLAG( Info->CurrentFlags,DISASM_FL_REF );
       outhex(subtype, extend, 0, addrsize, 0);
       CLR_FLAG( Info->CurrentFlags,DISASM_FL_REF );
       uputchar(']');
       break;

  case 'P':                            /* prefix byte (rh) */
       ua_str("%p:");
       break;

  case 'R':                            /* mod(r/m) picks register */
       reg_name(REG(modrm()), subtype);      /* rh */
       must_do_size = 0;
       break;

  case 'S':                            /* reg(r/m) picks segment reg */
       uputchar("ecsdfg"[REG(modrm())]);
       uputchar('s');
       must_do_size = 0;
       break;

  case 'T':                            /* reg(r/m) picks T reg */
       uprintf("tr%d", REG(modrm()));
       must_do_size = 0;
       break;

  case 'X':                            /* ds:si type operator */
       uprintf("ds:[");
       if (addrsize == 32)
         uputchar('e');
       uprintf("si]");
       break;

  case 'Y':                            /* es:di type operator */
       uprintf("es:[");
       if (addrsize == 32)
         uputchar('e');
       uprintf("di]");
       break;

  case '2':                            /* old [pop cs]! now indexes */
       ua_str(second[getbyte()]);      /* instructions in 386/486   */
       break;

  case 'g':                            /* modrm group `subtype' (0--7) */
       ua_str( GetOPGroup(subtype) );
       break;

  case 'd':                             /* sizeof operand==dword? */
       if (opsize == 32)
         uputchar('d');
       uputchar(subtype);
       break;

  case 'w':                             /* insert explicit size specifier */
       if (opsize == 32)
         uputchar('d');
       else
         uputchar('w');
       uputchar(subtype);
       break;

  case 'e':                         /* extended reg name */
       if (opsize == 32) {
         if (subtype == 'w')
           uputchar('d');
         else {
           uputchar('e');
           uputchar(subtype);
         }
       } else
         uputchar(subtype);
       break;

  case 'f':                    /* '87 opcode */
       floating_point(subtype-'0');
       break;

  case 'j':
       if (addrsize==32 || opsize==32) /* both of them?! */
         uputchar('e');
       break;

  case 'p':                    /* prefix byte */
       switch (subtype)  {
       case 'c':
       case 'd':
       case 'e':
       case 'f':
       case 'g':
       case 's':
            prefix = subtype;
            c = getbyte();
            wordop = c & 1;
            ua_str( GetOP(c) );
            break;
       case ':':
            if (prefix)
              uprintf("%cs:", prefix);
            break;
       case ' ':
            c = getbyte();
            wordop = c & 1;
            ua_str( GetOP(c) );
            break;
       }
       break;

  case 's':                           /* size override */
       switch (subtype) {
       case 'a':
            addrsize = 48 - addrsize;
            c = getbyte();
            wordop = c & 1;
            ua_str( GetOP(c) );
            break;
       case 'o':
            opsize = 48 - opsize;
            c = getbyte();
            wordop = c & 1;
            ua_str( GetOP(c) );
            break;
       }
       break;
   }
}

static void  ua_str(const char* str)
  {  char c,c1;

  if ( !str ) {
    uprintf("<invalid>");
    return;
  }

  if ( strpbrk( str,"CDFGRST" ) ) // specifiers for registers=>no size 2b specified
    must_do_size = 0;

  while( (c = *str++) != 0 )
    if (c == '%') {
      c = *str++;
      if ( c )
        c1 = *str++;
       else
        c1 = 0;
      percent(c, c1);
      if ( !c1 ) break;
    } else
      uputchar(c);
}

static BYTE getbyte(void)
  {  BYTE c;

    BOOL eof = FALSE;
    c = Info->getbyte(&eof);

    if ( eof )
      longjmp(reached_eof, 1);

    Info->CodeBuffer[ Info->instruction_length++ ] = c;

    if (patch87) {
      c -= 0x5C;     /* fixup second byte in emulated '87 instruction */
      patch87 = 0;
    }

  return c;
}

static void SetFlags( DWORD fl )
  {
  if ( (fl&(DISASM_FL_CALL|DISASM_FL_JMP|DISASM_FL_INT|DISASM_FL_RET)) != 0 )
    SET_FLAG( fl,DISASM_FL_CODE );

  if ( (fl&(DISASM_FL_ADD|DISASM_FL_MUL|DISASM_FL_BIT|DISASM_FL_CMP)) != 0 )
    SET_FLAG( fl,DISASM_FL_DATA );

  Info->CurrentFlags = fl;
}

static const char* GetOP( BYTE Num )
  {
    SetFlags( op_flags[ Num ] );
 return opmap1[Num];
}

static const char* GetOPGroup( char subtype )
  {
    SetFlags( group_flags[ subtype-'0' ][ REG(modrm()) ] );
 return groups[subtype-'0'][REG(modrm())];
}

//---------------------------------------------------------------------------
UnassembeInfo::UnassembeInfo( void )
  {
    memset( this,0,sizeof(*this) );
    Flags = DISASM_HEXOUT;
}
CONSTSTR UnassembeInfo::GetStringName( DWORD id )
  {
    switch( id ) {
      case DISASM_ID_DWORD_PTR: return "dword ptr";
      case  DISASM_ID_WORD_PTR: return "word ptr";
      case  DISASM_ID_BYTE_PTR: return "byte ptr";
      case      DISASM_ID_NEAR: return "near";
      case       DISASM_ID_FAR: return "far";
      case     DISASM_ID_SHORT: return "short";
    }
 return "";
}

BYTE UnassembeInfo::Code( void )
  {
 return CodeBuffer[0];
}

void UnassembeInfo::Operand( DWORD& addr,char& useSign,int& opSize )
  {
}

//---------------------------------------------------------------------------
static BOOL INTUnassemble( void )
  {  const char* str;
     BYTE    c, c2;

  prefix             = 0;
  modrmv             = sibv = -1;     /* set modrm and sib flags */
  opsize             = addrsize = (Info->Flags&DISASM_SEG_SIZE16?16:32);
  must_do_size       = Info->Flags&DISASM_DO_SIZE;
  do_distance        = Info->Flags&DISASM_DISTOUT;
  ubufp              = NULL;
  patch87            = 0;

  c      = getbyte();
  wordop = c & 1;

  if (Info->Flags&DISASM_DO_EMUL87) {
    if (c==0xcd) {                 // wanna do emu '87 and ->ing to int?
      BOOL col = FALSE;

      c2 = Info->getbyte(&col);

      if ( col )
        longjmp(reached_eof, 1);

      if (c2 >= 0x34 && c2 <= 0x3E)
        patch87 = 1;               // emulated instruction!  => must repatch two bytes
      Info->returnbyte(c2);
      c -= 0x32;
    }
  }

  if ( (str=GetOP(c)) == NULL ) {
// invalid instruction?
    uprintf( "db\t%s",Info->HexDigit(c) );
    SET_FLAG( Info->CurrentFlags,DISASM_FL_NOP );
  } else
// valid instruction
    ua_str(str);

 return Info->instruction_length > 0;
}

//---------------------------------------------------------------------------
BOOL MYRTLEXP Unassemble( PUnassembeInfo pInfo )
  {
     if (!pInfo) return FALSE;

     Info = pInfo;

     Info->instruction_length = 0;
     Info->CurrentFlags       = 0;

     memset( Info->CodeBuffer, DISASM_BADBYTE, sizeof(Info->CodeBuffer) );
     Info->CommandBuffer[0] = 0;
     Info->LeftBuffer[0]    = 0;
     Info->RightBuffer[0]   = 0;

     BOOL rc;

     if ( setjmp(reached_eof) )
       rc = FALSE;
      else
       rc = INTUnassemble();

     if (ubufp) *ubufp = 0;

 return rc;
}
