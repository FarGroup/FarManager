#ifndef __MY_XX_DEFINES
#define __MY_XX_DEFINES

enum _STK_StackWalkOptions {
//                                          Description                                              CON       VCL
 STK_READCODE         = 0x00000001ul, //Read code around each stack frame                        Y         Y
 STK_LOADEXPORTS      = 0x00000002ul, //Load export symbols for every module in stack items list Y         Y
 STK_LOADLOCALS       = 0x00000004ul, //Load MAP symbols for every module in stack items list    Y         Y
 STK_ENABLED          = 0x00000008ul, //Exception filter enabled                                 ignored   Y
 STK_FILELOG          = 0x00000010ul, //Write exception info to log file                         Y         Y
 STK_DISASM           = 0x00000020ul, //Disassembly stack frame codes                            Y         Y
 STK_COMMENTCHAR      = 0x00000040ul, //Add DASM_SYM_COMMENT char at start of comment            Y         OFF
 STK_COMMENTIRECTION  = 0x00000080ul, //Add DASM_SYM_UP\DOWN chars in comment                    Y         ON
 STK_COMMENTSYMBOL    = 0x00000100ul, //Add symbol information to comments                       Y         ON
 STK_COMMENTDUMP      = 0x00000200ul, //Add data dump to comments                                Y         Y
 STK_STACKDATA        = 0x00000400ul, //Read and draw stack data for every procedure             Y         Y
 STK_STACKSYSTEM      = 0x00000800ul, //Show stack-walk for system modules                       N         N
 STK_LOADIMPORTS      = 0x00001000ul, //Load imports symbols                                     Y         Y
 STK_EACHDETAILS      = 0x00002000ul, //Print detailed info for each stack entry                 Y         Y

 STK_LOADSYMBOLS      = (STK_LOADIMPORTS|STK_LOADEXPORTS|STK_LOADLOCALS),
 STK_FULL             = (STK_ENABLED | STK_FILELOG | STK_LOADSYMBOLS | STK_READCODE | STK_DISASM |
                         STK_COMMENTCHAR | STK_COMMENTIRECTION | STK_COMMENTSYMBOL | STK_COMMENTDUMP | STK_STACKDATA |
                         STK_EACHDETAILS),
 STK_DEFAULT          = STK_FULL
};

#endif
