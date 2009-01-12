#ifndef __Robin_Hilliard_2ASM
#define __Robin_Hilliard_2ASM

//Disassemble flags
enum _DASM_OperationOptions {
 DISASM_DO_EMUL87   = 0x000000001ul,   //Expand fp87 emulated instructions
 DISASM_SEG_SIZE16  = 0x000000002ul,   //Interprete instructions as 16-bit (32 by default)
 DISASM_DO_SIZE     = 0x000000004ul,   //Add `Xword ptr`
 DISASM_HEXOUT      = 0x000000008ul,   //Add `h` after hexadecimal digit
 DISASM_DISTOUT     = 0x000000010ul    //Add 'far', 'near', 'short' modifiers
};

//Consts
enum _DASM_Constants {
 DISASM_BADBYTE     = 0xF1,           //Indicate bad code byte
 DISASM_CODESIZE    = 13,             //Maximum number of codes in one instruction
 DISASM_CMDSIZE     = 50
};

//Flags of current decoded command
//paraters of command
enum _DASM_CurrentFlag {
 DISASM_FL_REF      = 0x00100000UL,     //Reference ( [ ??? ] )
 DISASM_FL_REFADD   = 0x00200000UL,     //Reference ( [ xxx + ??? ] )
 DISASM_FL_CODE     = 0x00400000UL,     //Reference to code ( jmp, call, loop )
 DISASM_FL_DATA     = 0x00800000UL,     //Reference to data ( push ????, add xxx,???, call [???] )
 DISASM_FL_OFFSET   = 0x01000000UL,     //Relative reference ( xxx+???, je +??? )
 DISASM_FL_ABS      = 0x02000000UL,     //Absolute call
 DISASM_FL_MATH     = 0x04000000UL,     //Any math op (add, sub, mul, div )
 DISASM_FL_DSMALL   = 0x08000000UL,     //Output small digits (without start zeroes)

//Currently decoded part
 DISASM_FL_COMMAND  = 0x08000000UL,     //Processing command name
 DISASM_FL_LEFT     = 0x10000000UL,     //Processing left operand
 DISASM_FL_RIGHT    = 0x20000000UL,     //Processing right operand

//Type of OP-code
 DISASM_FL_CALL     = 0x00000001UL,     //callX
 DISASM_FL_JMP      = 0x00000002UL,     //jmpX
 DISASM_FL_INT      = 0x00000004UL,     //intX
 DISASM_FL_ADD      = 0x00000008UL,     //add, sub
 DISASM_FL_MUL      = 0x00000010UL,     //mul, div
 DISASM_FL_BIT      = 0x00000020UL,     //or, and, xor
 DISASM_FL_CMP      = 0x00000040UL,     //test, cmp
 DISASM_FL_MOV      = 0x00000080UL,     //mov, lea
 DISASM_FL_IO       = 0x00000100UL,     //in, out
 DISASM_FL_PUSH     = 0x00000200UL,     //push
 DISASM_FL_POP      = 0x00000400UL,     //pop
 DISASM_FL_NOP      = 0x00000800UL,     //nop or invalid
 DISASM_FL_RET      = 0x00001000UL,     //ret, iret
 DISASM_FL_87       = 0x00002000UL,    //Float point
 DISASM_FL_FL       = 0x00004000UL,     //Flags manipulation

 DISASM_FL_OPERAND  = (DISASM_FL_COMMAND|DISASM_FL_LEFT|DISASM_FL_RIGHT)
};

enum _DASM_SizeType {
//Types of strings
 DISASM_ID_DWORD_PTR   = 0,
 DISASM_ID_WORD_PTR    = 1,
 DISASM_ID_BYTE_PTR    = 2,
 DISASM_ID_NEAR        = 3,
 DISASM_ID_FAR         = 4,
 DISASM_ID_SHORT       = 5,
};


STRUCT( UnassembeInfo )
//USER DEFINED
  DWORD  Flags;                                                      // Set of DISASM_xxx flags

//FILLED BY Unasseble
  BYTE   CodeBuffer[DISASM_CODESIZE];                                // Contains codes for current instruction
  char   CommandBuffer[DISASM_CMDSIZE];                              // Bufer contains name of command
  char   LeftBuffer[ DISASM_CMDSIZE ];
  char   RightBuffer[ DISASM_CMDSIZE ];

  DWORD  instruction_length;
  DWORD  CurrentFlags;                                               //Set of DISASM_FL_xxx indicating current command

  UnassembeInfo( void );

//Utilites
  CONSTSTR HexDigit( DWORD Value,char SignChar = 0,BOOL DoCompress = FALSE ); //Uses local static string buffer
  BYTE     Code( void );
//User optional
  virtual CONSTSTR GetStringName( DWORD id );                        //Decode DISASM_ID_xxx to string (must not contain trail space)

//User disassembler readers                                          // Get next byte of decoded stream.
  virtual BYTE  getbyte( BOOL *OutOfBounds ) = 0;                    //  Set `OutOfBounds` to nonzero value to indicate EOF reached
  virtual void  returnbyte( BYTE Byte )      = 0;                    // Send readed byte back to buffer.
                                                                     //  Used after preread operations.
  virtual void  Operand( DWORD& addr,char& useSign,int& opSize );
    /*
        Called then any digit value will be placed in output buffer
          `addr`    - value of operand
          `useSign` - used sign ('+' | '-' | 0 if sign not used)
          `opSize`  - is size in bytes of offset operand (1,2 or 4)
    */
};

HDECLSPEC BOOL MYRTLEXP Unassemble( PUnassembeInfo pInfo );

#endif
