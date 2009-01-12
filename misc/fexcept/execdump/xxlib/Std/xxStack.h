#ifndef __MY_STACK_UNWIND
#define __MY_STACK_UNWIND

#ifndef _WIN64
#define REG_FMT "%08X"
#define MAX_RVA MAX_DWORD
#else
#define REG_FMT "%16I64X"
#define MAX_RVA ((SIZE_T)0xFFFFFFFFFFFFFFFFui64)
#endif

enum _STK_Constants {
 STK_LEN_CODE     = 0x80,
 STK_LEN_DUMP     = 16,
 STK_LEN_STACK    = 10,
 STK_LEN_DATA     = 16,
 STK_STACK_DEPTH  = 128
};

STRUCT( StackDataItem )
  LPBYTE Ptr;
  BYTE   Data[ STK_LEN_DATA ];
};

STRUCT( StackEntry )
  HMODULE       ModuleHandle;               //Handle of module
  LPVOID        ModuleBase;                 //Load base of module
  BOOL          SysModule;
  char          Module[ MAX_PATH_SIZE ];    //Module where current stack frame placed
  DWORD         FileOffset;                 //Offset of stack frame from start of `Module` image

  char          Section[ 20 ];              //Section of module where stack frame placed
  DWORD         Offset;                     //Offset of stack frame in section

  char          Symbol[ MAX_PATH_SIZE ];    //Name of current stack frame symbol (name of procedure)
                                            //  Empty if not exist or can not be determined
  DWORD         SymbolOffset;               //Offset from known symbol

  SIZE_T        CodeAddr;                   //Address of main command of stack frame in memory
  SIZE_T        FrameAddr;                  //Address of up stack frame

  int           CodeSize;                   //Size of readed code. 0 if not exist
  DWORD         CodePos;                    //Offset of CodeAddr code byte in Code array
  BYTE          Code[ STK_LEN_CODE ];       //Bytes of code around main stack frame instruction

  StackDataItem StackData[ STK_LEN_STACK ]; //Stack contents before EIP:EBP called
};

STRUCT( CallStack )
  MyRefArray<StackEntry> Items;             //Stack entries (Valid only after UnwindCallStack returns)
  DWORD                  Flags;             //Set of STK_xxx flags
  DWORD                  SkipFrames;        //Frames to skip in output call-stack
};

/*  StackDisasm

    An UnassembeInfo descendant created to fill additional
    information about stack frame assembler.
    Set `Comments` to names of known symbols or data.
*/
enum _STK_StackWalkCharacters {
 DASM_SYM_LESS     = '-',
 DASM_SYM_CURRENT  = '>',
 DASM_SYM_MORE     = '+',
 DASM_SYM_COMMENT  = ';',
 DASM_SYM_UP       = '\x18', //
 DASM_SYM_DOWN     = '\x19'  //
};

STRUCTBASE( StackDisasm, public SymDisasm )
    PStackEntry    Stack;
    int            Pos;
  public:
    StackDisasm( PStackEntry p );

    void Assign( PStackEntry p );

    virtual BYTE  getbyte( BOOL *OutOfBounds );
    virtual void  returnbyte( BYTE Byte );
    virtual SIZE_T Position( void );
};

//Create list of stack frames staring from EIP, based of EBP
HDECLSPEC BOOL  MYRTLEXP UnwindCallStack( PCallStack StackInfo,SIZE_T ip,SIZE_T bp,SIZE_T sp );

//Create a human-readable view of unwind stack item
//  (used by MakeStackWalkList for every stack entry)
HDECLSPEC void  MYRTLEXP MakeStackWalkItem( HPrintProc_t PrintProcedure,PStackEntry StackItem );

//Create a human-readable view of unwind stack
//  specified address
HDECLSPEC BOOL  MYRTLEXP MakeStackWalkListIP( HPrintProc_t PrintProcedure,DWORD SkipFrames,DWORD ip,DWORD bp,DWORD Flags = STK_DEFAULT );

//  context with IP:BP set
HDECLSPEC BOOL  MYRTLEXP MakeStackWalkListCTX( HPrintProc_t PrintProcedure,CONTEXT *ctx,DWORD Flags = STK_DEFAULT,DWORD SkipFrames = 0 );

//  current execution point
HDECLSPEC BOOL  MYRTLEXP MakeStackWalkListCUR( HPrintProc_t PrintProcedure,DWORD SkipFrames,DWORD Flags = STK_DEFAULT );

//  ready stack list
HDECLSPEC BOOL  MYRTLEXP MakeStackWalkListSTK( HPrintProc_t PrintProcedure,PCallStack CallStack );


#endif
