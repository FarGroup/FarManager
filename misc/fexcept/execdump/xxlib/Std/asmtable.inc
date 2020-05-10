/* Percent tokens in strings:
   First char after '%':
        A - direct address
        C - reg of r/m picks control register
        D - reg of r/m picks debug register
        E - r/m picks operand
        F - flags register
        G - reg of r/m picks general register
        I - immediate data
        J - relative IP offset
+       K - call/jmp distance
        M - r/m picks memory
        O - no r/m, offset only
        R - mod of r/m picks register only
        S - reg of r/m picks segment register
        T - reg of r/m picks test register
        X - DS:ESI
        Y - ES:EDI
        2 - prefix of two-byte opcode
+       e - put in 'e' if use32 (second char is part of reg name)
+           put in 'w' for use16 or 'd' for use32 (second char is 'w')
+       j - put in 'e' in jcxz if prefix==0x66
        f - floating point (second char is esc value)
        g - do r/m group 'n', n==0..7
        p - prefix
        s - size override (second char is a,o)
+       d - put d if double arg, nothing otherwise (pushfd, popfd &c)
+       w - put w if word, d if double arg, nothing otherwise (lodsw/lodsd)
+       P - simple prefix

   Second char after '%':
        a - two words in memory (BOUND)
        b - byte
        c - byte or word
        d - dword
+       f - far call/jmp
+       n - near call/jmp
        p - 32 or 48 bit pointer
+       q - byte/word thingy
        s - six byte pseudo-descriptor
        v - word or dword
        w - word
+       x - sign extended byte
        F - use floating regs in mod/rm
        1-8 - group number, esc value, etc
*/

/* watch out for aad && aam with odd operands */


static const char *opmap1[256] = {
/* 0 */
  "add\t%Eb,%Gb",      "add\t%Ev,%Gv",     "add\t%Gb,%Eb",    "add\t%Gv,%Ev",
  "add\tal,%Ib",       "add\t%eax,%Iv",    "push\tes",        "pop\tes",
  "or\t%Eb,%Gb",       "or\t%Ev,%Gv",      "or\t%Gb,%Eb",     "or\t%Gv,%Ev",
  "or\tal,%Ib",        "or\t%eax,%Iv",     "push\tcs",        "%2 ",
/* 1 */
  "adc\t%Eb,%Gb",      "adc\t%Ev,%Gv",     "adc\t%Gb,%Eb",    "adc\t%Gv,%Ev",
  "adc\tal,%Ib",       "adc\t%eax,%Iv",    "push\tss",        "pop\tss",
  "sbb\t%Eb,%Gb",      "sbb\t%Ev,%Gv",     "sbb\t%Gb,%Eb",    "sbb\t%Gv,%Ev",
  "sbb\tal,%Ib",       "sbb\t%eax,%Iv",    "push\tds",        "pop\tds",
/* 2 */
  "and\t%Eb,%Gb",      "and\t%Ev,%Gv",     "and\t%Gb,%Eb",    "and\t%Gv,%Ev",
  "and\tal,%Ib",       "and\t%eax,%Iv",    "%pe",            "daa",
  "sub\t%Eb,%Gb",      "sub\t%Ev,%Gv",     "sub\t%Gb,%Eb",    "sub\t%Gv,%Ev",
  "sub\tal,%Ib",       "sub\t%eax,%Iv",    "%pc",            "das",
/* 3 */
  "xor\t%Eb,%Gb",      "xor\t%Ev,%Gv",     "xor\t%Gb,%Eb",    "xor\t%Gv,%Ev",
  "xor\tal,%Ib",       "xor\t%eax,%Iv",    "%ps",            "aaa",
  "cmp\t%Eb,%Gb",      "cmp\t%Ev,%Gv",     "cmp\t%Gb,%Eb",    "cmp\t%Gv,%Ev",
  "cmp\tal,%Ib",       "cmp\t%eax,%Iv",    "%pd",            "aas",
/* 4 */
  "inc\t%eax",         "inc\t%ecx",        "inc\t%edx",       "inc\t%ebx",
  "inc\t%esp",         "inc\t%ebp",        "inc\t%esi",       "inc\t%edi",
  "dec\t%eax",         "dec\t%ecx",        "dec\t%edx",       "dec\t%ebx",
  "dec\t%esp",         "dec\t%ebp",        "dec\t%esi",       "dec\t%edi",
/* 5 */
  "push\t%eax",        "push\t%ecx",       "push\t%edx",      "push\t%ebx",
  "push\t%esp",        "push\t%ebp",       "push\t%esi",      "push\t%edi",
  "pop\t%eax",         "pop\t%ecx",        "pop\t%edx",       "pop\t%ebx",
  "pop\t%esp",         "pop\t%ebp",        "pop\t%esi",       "pop\t%edi",
/* 6 */
  "pusha%d",           "popa%d",           "bound\t%Gv,%Ma",  "arpl\t%Ew,%Rw",
  "%pf",               "%pg",              "%so",             "%sa",
  "push\t%Iv",         "imul\t%Gv,%Ev,%Iv","push\t%Ix",       "imul\t%Gv,%Ev,%Ib",
  "insb",              "ins%ew",           "outsb",           "outs%ew",
/* 7 */
  "jo\t%Jb",           "jno\t%Jb",         "jc\t%Jb",         "jnc\t%Jb",
  "je\t%Jb",           "jne\t%Jb",         "jbe\t%Jb",        "ja\t%Jb",
  "js\t%Jb",           "jns\t%Jb",         "jpe\t%Jb",        "jpo\t%Jb",
  "jl\t%Jb",           "jge\t%Jb",         "jle\t%Jb",        "jg\t%Jb",
/* 8 */
  "%g0\t%Eb,%Ib",      "%g0\t%Ev,%Iv",     "%g0\t%Ev,%Ix",    "%g0\t%Ev,%Ix",
  "test\t%Eb,%Gb",     "test\t%Ev,%Gv",    "xchg\t%Eb,%Gb",   "xchg\t%Ev,%Gv",
  "mov\t%Eb,%Gb",      "mov\t%Ev,%Gv",     "mov\t%Gb,%Eb",    "mov\t%Gv,%Ev",
  "mov\t%Ew,%Sw",      "lea\t%Gv,%M",      "mov\t%Sw,%Ew",    "pop\t%Ev",
/* 9 */
  "nop",               "xchg\t%ecx,%eax",  "xchg\t%edx,%eax", "xchg\t%ebx,%eax",
  "xchg\t%esp,%eax",   "xchg\t%ebp,%eax",  "xchg\t%esi,%eax", "xchg\t%edi,%eax",
  "cbw",               "cwd",              "call\t%Ap",       "fwait",
  "pushf%d",           "popf%d",           "sahf",            "lahf",
/* a */
  "mov\tal,%Oc",       "mov\t%eax,%Ov",    "mov\t%Oc,al",     "mov\t%Ov,%eax",
  "%P movsb",          "%P movs%w",        "%P cmpsb",        "%P cmps%w",
  "test\tal,%Ib",      "test\t%eax,%Iv",   "%P stosb",        "%P stos%w",
  "%P lodsb",          "%P lods%w",        "%P scasb",        "%P scas%w",
/* b */
  "mov\tal,%Ib",       "mov\tcl,%Ib",      "mov\tdl,%Ib",     "mov\tbl,%Ib",
  "mov\tah,%Ib",       "mov\tch,%Ib",      "mov\tdh,%Ib",     "mov\tbh,%Ib",
  "mov\t%eax,%Iv",     "mov\t%ecx,%Iv",    "mov\t%edx,%Iv",   "mov\t%ebx,%Iv",
  "mov\t%esp,%Iv",     "mov\t%ebp,%Iv",    "mov\t%esi,%Iv",   "mov\t%edi,%Iv",
/* c */
  "%g1\t%Eb,%Ib",      "%g1\t%Ev,%Ib",     "ret\t%Iw",        "ret",
  "les\t%Gv,%Mp",      "lds\t%Gv,%Mp",     "mov\t%Eb,%Ib",    "mov\t%Ev,%Iv",
  "enter\t%Iw,%Ib",    "leave",            "retf\t%Iw",       "retf",
  "int\t03",           "int\t%Ib",         "into",            "iret",
/* d */
  "%g1\t%Eb,1",        "%g1\t%Ev,1",       "%g1\t%Eb,cl",     "%g1\t%Ev,cl",
  "aam\t%Ib",          "aad\t%Ib",         "setalc",          "xlat",
  "esc\t0,%Ib",        "esc\t1,%Ib",       "esc\t2,%Ib",      "esc\t3,%Ib",
  "esc\t4,%Ib",        "esc\t5,%Ib",       "esc\t6,%Ib",      "esc\t7,%Ib",
/* e */
  "loopne\t%Jb",      "loope\t%Jb",      "loop\t%Jb",      "j%j\tcxz %Jb",
  "in\tal,%Ib",       "in\t%eax,%Ib",    "out\t%Ib,al",    "out\t%Ib,%eax",
  "call\t%Jv",        "jmp\t%Jv",        "jmp\t%Ap",       "jmp\t%Ks%Jb",
  "in\tal,dx",        "in\t%eax,dx",     "out\tdx,al",     "out\tdx,%eax",
/* f */
  "lock\t%p",         0,                 "repne\t%p",      "repe\t%p",
  "hlt",              "cmc",             "%g2",            "%g2",
  "clc",              "stc",             "cli",            "sti",
  "cld",              "std",             "%g3",            "%g4"
};

static DWORD op_flags[256] = {
/* 0 */
  //"add\t%Eb,%Gb",                  "add\t%Ev,%Gv",               "add\t%Gb,%Eb",               "add\t%Gv,%Ev",
  DISASM_FL_MATH|DISASM_FL_ADD,      DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD,
  //"add\tal,%Ib",                   "add\t%eax,%Iv",              "push\tes",                   "pop\tes",
  DISASM_FL_MATH|DISASM_FL_ADD,      DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_PUSH,               DISASM_FL_POP,
  //"or\t%Eb,%Gb",                   "or\t%Ev,%Gv",                "or\t%Gb,%Eb",                "or\t%Gv,%Ev",
  DISASM_FL_MATH|DISASM_FL_BIT,      DISASM_FL_MATH|DISASM_FL_BIT, DISASM_FL_MATH|DISASM_FL_BIT, DISASM_FL_MATH|DISASM_FL_BIT,
  //"or\tal,%Ib",                    "or\t%eax,%Iv",               "push\tcs",                   "%2 ",
  DISASM_FL_MATH|DISASM_FL_BIT,      DISASM_FL_MATH|DISASM_FL_BIT, DISASM_FL_PUSH,               0,
/* 1 */
  //"sbb\tal,%Ib",                   "sbb\t%eax,%Iv",              "push\tds",                   "pop\tds",
  DISASM_FL_MATH|DISASM_FL_ADD,      DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD,
  //"adc\t%Eb,%Gb",                  "adc\t%Ev,%Gv",               "adc\t%Gb,%Eb",               "adc\t%Gv,%Ev",
  DISASM_FL_MATH|DISASM_FL_ADD,      DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_PUSH,               DISASM_FL_POP,
  //"adc\tal,%Ib",                   "adc\t%eax,%Iv",              "push\tss",                   "pop\tss",
  DISASM_FL_MATH|DISASM_FL_ADD,      DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD,
  //"sbb\t%Eb,%Gb",                  "sbb\t%Ev,%Gv",               "sbb\t%Gb,%Eb",               "sbb\t%Gv,%Ev",
  DISASM_FL_MATH|DISASM_FL_ADD,      DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_PUSH,               DISASM_FL_POP,
/* 2 */
  //"and\t%Eb,%Gb",                  "and\t%Ev,%Gv",               "and\t%Gb,%Eb",               "and\t%Gv,%Ev",
  DISASM_FL_MATH|DISASM_FL_ADD,      DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD,
  //"and\tal,%Ib",                   "and\t%eax,%Iv",              "%pe",            "           daa",
  DISASM_FL_MATH|DISASM_FL_ADD,      DISASM_FL_MATH|DISASM_FL_ADD, 0,                            DISASM_FL_MATH|DISASM_FL_ADD,
  //"sub\t%Eb,%Gb",                  "sub\t%Ev,%Gv",               "sub\t%Gb,%Eb",               "sub\t%Gv,%Ev",
  DISASM_FL_MATH|DISASM_FL_ADD,      DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD,
  //"sub\tal,%Ib",                   "sub\t%eax,%Iv",              "%pc",            "           das",
  DISASM_FL_MATH|DISASM_FL_ADD,      DISASM_FL_MATH|DISASM_FL_ADD, 0,                            DISASM_FL_MATH|DISASM_FL_ADD,
/* 3 */
  //"xor\t%Eb,%Gb",                  "xor\t%Ev,%Gv",               "xor\t%Gb,%Eb",               "xor\t%Gv,%Ev",
  DISASM_FL_MATH|DISASM_FL_BIT,      DISASM_FL_MATH|DISASM_FL_BIT, DISASM_FL_MATH|DISASM_FL_BIT, DISASM_FL_MATH|DISASM_FL_BIT,
  //"xor\tal,%Ib",                   "xor\t%eax,%Iv",              "%ps",            "           aaa",
  DISASM_FL_MATH|DISASM_FL_BIT,      DISASM_FL_MATH|DISASM_FL_BIT, 0,                            DISASM_FL_MATH|DISASM_FL_ADD,
  //"cmp\t%Eb,%Gb",                  "cmp\t%Ev,%Gv",               "cmp\t%Gb,%Eb",               "cmp\t%Gv,%Ev",
  DISASM_FL_CMP,                     DISASM_FL_CMP,                DISASM_FL_CMP,                DISASM_FL_CMP,
  //"cmp\tal,%Ib",                   "cmp\t%eax,%Iv",              "%pd",            "           aas",
  DISASM_FL_CMP,                     DISASM_FL_CMP,                0,                            DISASM_FL_MATH|DISASM_FL_ADD,
/* 4 */
  //"inc\t%eax",                     "inc\t%ecx",                  "inc\t%edx",                  "inc\t%ebx",
  DISASM_FL_MATH|DISASM_FL_ADD,      DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD,
  //"inc\t%esp",                     "inc\t%ebp",                  "inc\t%esi",                  "inc\t%edi",
  DISASM_FL_MATH|DISASM_FL_ADD,      DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD,
  //"dec\t%eax",                     "dec\t%ecx",                  "dec\t%edx",                  "dec\t%ebx",
  DISASM_FL_MATH|DISASM_FL_ADD,      DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD,
  //"dec\t%esp",                     "dec\t%ebp",                  "dec\t%esi",                  "dec\t%edi",
  DISASM_FL_MATH|DISASM_FL_ADD,      DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD,
/* 5 */
  //"push\t%eax",                    "push\t%ecx",                 "push\t%edx",                 "push\t%ebx",
  DISASM_FL_PUSH,                    DISASM_FL_PUSH,               DISASM_FL_PUSH,               DISASM_FL_PUSH,
  //"push\t%esp",                    "push\t%ebp",                 "push\t%esi",                 "push\t%edi",
  DISASM_FL_PUSH,                    DISASM_FL_PUSH,               DISASM_FL_PUSH,               DISASM_FL_PUSH,
  //"pop\t%eax",                     "pop\t%ecx",                  "pop\t%edx",                  "pop\t%ebx",
  DISASM_FL_POP,                     DISASM_FL_POP,                DISASM_FL_POP,                DISASM_FL_POP,
  //"pop\t%esp",                     "pop\t%ebp",                  "pop\t%esi",                  "pop\t%edi",
  DISASM_FL_POP,                     DISASM_FL_POP,                DISASM_FL_POP,                DISASM_FL_POP,
/* 6 */
  //"pusha%d",                       "popa%d",                     "bound\t%Gv,%Ma",             "arpl\t%Ew,%Rw",
  DISASM_FL_PUSH,                       DISASM_FL_POP,                   0,                            0,
  //"%pf",                           "%pg",                        "%so",                        "%sa",
  0,                                 0,                            0,                            0,
  //"push\t%Iv",                     "imul\t%Gv,%Ev,%Iv",          "push\t%Ix",                  "imul\t%Gv,%Ev,%Ib",
  DISASM_FL_PUSH,                    DISASM_FL_MATH|DISASM_FL_MUL, DISASM_FL_PUSH,               DISASM_FL_MATH|DISASM_FL_MUL,
  //"insb",                          "ins%ew",                     "outsb",                      "outs%ew",
  DISASM_FL_IO,                      DISASM_FL_IO,                 DISASM_FL_IO,                 DISASM_FL_IO,
/* 7 */
  //"jo\t%Jb",                       "jno\t%Jb",                   "jc\t%Jb",                    "jnc\t%Jb",
  DISASM_FL_JMP,                     DISASM_FL_JMP,                DISASM_FL_JMP,                DISASM_FL_JMP,
  //"je\t%Jb",                       "jne\t%Jb",                   "jbe\t%Jb",                   "ja\t%Jb",
  DISASM_FL_JMP,                     DISASM_FL_JMP,                DISASM_FL_JMP,                DISASM_FL_JMP,
  //"js\t%Jb",                       "jns\t%Jb",                   "jpe\t%Jb",                   "jpo\t%Jb",
  DISASM_FL_JMP,                     DISASM_FL_JMP,                DISASM_FL_JMP,                DISASM_FL_JMP,
  //"jl\t%Jb",                       "jge\t%Jb",                   "jle\t%Jb",                   "jg\t%Jb",
  DISASM_FL_JMP,                     DISASM_FL_JMP,                DISASM_FL_JMP,                DISASM_FL_JMP,
/* 8 */
  //"%g0\t%Eb,%Ib",                  "%g0\t%Ev,%Iv",               "%g0\t%Ev,%Ix",               "%g0\t%Ev,%Ix",
  0,                                 0,                            0,                            0,
  //"test\t%Eb,%Gb",                 "test\t%Ev,%Gv",              "xchg\t%Eb,%Gb",              "xchg\t%Ev,%Gv",
  DISASM_FL_CMP,                     DISASM_FL_CMP,                0,                            0,
  //"mov\t%Eb,%Gb",                  "mov\t%Ev,%Gv",               "mov\t%Gb,%Eb",               "mov\t%Gv,%Ev",
  DISASM_FL_MOV,                     DISASM_FL_MOV,                DISASM_FL_MOV,                DISASM_FL_MOV,
  //"mov\t%Ew,%Sw",                  "lea\t%Gv,%M",                "mov\t%Sw,%Ew",               "pop\t%Ev",
  DISASM_FL_MOV,                     DISASM_FL_MOV,                DISASM_FL_MOV,                DISASM_FL_POP,
/* 9 */
  //"nop",                           "xchg\t%ecx,%eax",            "xchg\t%edx,%eax",            "xchg\t%ebx,%eax",
  DISASM_FL_NOP,                     0,                            0,                            0,
  //"xchg\t%esp,%eax",   ",          "xchg\t%ebp,%eax",            "xchg\t%esi,%eax",            "xchg\t%edi,%eax
  0,                                 0,                            0,                            0,
  //"cbw",                           "cwd",                        "call\t%Ap",                  "fwait",
  0,                                 0,                            DISASM_FL_CALL|DISASM_FL_ABS, DISASM_FL_87,
  //"pushf%d",                       "popf%d",                     "sahf",                       "lahf",
  DISASM_FL_PUSH,                    DISASM_FL_POP,                DISASM_FL_MATH|DISASM_FL_BIT, DISASM_FL_MATH|DISASM_FL_BIT,
/* a */
  //"mov\tal,%Oc",                   "mov\t%eax,%Ov",              "mov\t%Oc,al",                "mov\t%Ov,%eax",
  DISASM_FL_MOV,                     DISASM_FL_MOV,                DISASM_FL_MOV,                DISASM_FL_MOV,
  //"%P movsb",                      "%P movs%w",                  "%P cmpsb",                   "%P cmps%w",
  DISASM_FL_MOV,                     DISASM_FL_MOV,                DISASM_FL_CMP,                DISASM_FL_CMP,
  //"test\tal,%Ib",                  "test\t%eax,%Iv",             "%P stosb",                   "%P stos%w",
  DISASM_FL_CMP,                     DISASM_FL_CMP,                DISASM_FL_MOV,                DISASM_FL_MOV,
  //"%P lodsb",                      "%P lods%w",                  "%P scasb",                   "%P scas%w",
  DISASM_FL_MOV,                     DISASM_FL_MOV,                DISASM_FL_MOV,                DISASM_FL_MOV,
/* b */
  //"mov\tal,%Ib",                   "mov\tcl,%Ib",                "mov\tdl,%Ib",                "mov\tbl,%Ib",
  DISASM_FL_MOV,                     DISASM_FL_MOV,                DISASM_FL_MOV,                DISASM_FL_MOV,
  //"mov\tah,%Ib",                   "mov\tch,%Ib",                "mov\tdh,%Ib",                "mov\tbh,%Ib",
  DISASM_FL_MOV,                     DISASM_FL_MOV,                DISASM_FL_MOV,                DISASM_FL_MOV,
  //"mov\t%eax,%Iv",                 "mov\t%ecx,%Iv",              "mov\t%edx,%Iv",              "mov\t%ebx,%Iv",
  DISASM_FL_MOV,                     DISASM_FL_MOV,                DISASM_FL_MOV,                DISASM_FL_MOV,
  //"mov\t%esp,%Iv",                 "mov\t%ebp,%Iv",              "mov\t%esi,%Iv",              "mov\t%edi,%Iv",
  DISASM_FL_MOV,                     DISASM_FL_MOV,                DISASM_FL_MOV,                DISASM_FL_MOV,
/* c */
  //"%g1\t%Eb,%Ib",                  "%g1\t%Ev,%Ib",               "ret\t%Iw",                   "ret",
  0,                                 0,                            DISASM_FL_RET,                DISASM_FL_RET,
  //"les\t%Gv,%Mp",                  "lds\t%Gv,%Mp",               "mov\t%Eb,%Ib",               "mov\t%Ev,%Iv",
  DISASM_FL_MOV,                     DISASM_FL_MOV,                DISASM_FL_MOV,                DISASM_FL_MOV,
  //"enter\t%Iw,%Ib",                "leave",                      "retf\t%Iw",                  "retf",
  0,                                 0,                            DISASM_FL_RET,                DISASM_FL_RET,
  //"int\t03",                       "int\t%Ib",                   "into",                       "iret",
  DISASM_FL_INT,                     DISASM_FL_INT,                DISASM_FL_INT,                DISASM_FL_RET,
/* d */
  //"%g1\t%Eb,1",                    "%g1\t%Ev,1",                 "%g1\t%Eb,cl",                "%g1\t%Ev,cl",
  0,                                 0,                            0,                            0,
  //"aam\t%Ib",                      "aad\t%Ib",                   "setalc",                     "xlat",
  DISASM_FL_MATH|DISASM_FL_ADD,      DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD,
  //"esc\t0,%Ib",                    "esc\t1,%Ib",                 "esc\t2,%Ib",                 "esc\t3,%Ib",
  0,                                 0,                            0,                            0,
  //"esc\t4,%Ib",                    "esc\t5,%Ib",                 "esc\t6,%Ib",                 "esc\t7,%Ib",
  0,                                 0,                            0,                            0,
/* e */
  //"loopne\t%Jb",                   "loope\t%Jb",                 "loop\t%Jb",                  "j%j\tcxz %Jb",
  DISASM_FL_CALL,                    DISASM_FL_CALL,               DISASM_FL_CALL,               DISASM_FL_JMP,
  //"in\tal,%Ib",                    "in\t%eax,%Ib",               "out\t%Ib,al",                "out\t%Ib,%eax",
  DISASM_FL_IO,                      DISASM_FL_IO,                 DISASM_FL_IO,                 DISASM_FL_IO,
  //"call\t%Jv",                     "jmp\t%Jv",                   "jmp\t%Ap",                   "jmp\t%Ks%Jb",
  DISASM_FL_CALL|DISASM_FL_ABS,      DISASM_FL_JMP|DISASM_FL_ABS,  DISASM_FL_JMP|DISASM_FL_ABS,  DISASM_FL_JMP|DISASM_FL_ABS,
  //"in\tal,dx",                     "in\t%eax,dx",                "out\tdx,al",                 "out\tdx,%eax",
  DISASM_FL_IO,                      DISASM_FL_IO,                 DISASM_FL_IO,                 DISASM_FL_IO,
/* f */
  //"lock\t%p",                      0,                            "repne\t%p",                  "repe\t%p",
  0,                                 0,                            0,                            0,
  //"hlt",                           "cmc",                        "%g2",                        "%g2",
  0,                                 0,                            0,                            0,
  //"clc",                           "stc",                        "cli",                        "sti",
  DISASM_FL_FL,                      DISASM_FL_FL,                 DISASM_FL_FL,                 DISASM_FL_FL,
  //"cld",                           "std",                        "%g3",                        "%g4"
  DISASM_FL_FL,                      DISASM_FL_FL,                 0,                            0
};

static const char*second[] = {
/* 0 */
  "%g5",              "%g6",             "lar\t%Gv,%Ew",   "lsl\t%Gv,%Ew",
  0,                  "loadall",         "clts",           "loadall",
  "invd",             "wbinvd",          0,                0,
  0,                  0,                 0,                0,
/* 1 */
  "mov\t%Eb,%Gb",     "mov\t%Ev,%Gv",    "mov\t%Gb,%Eb",   "mov\t%Gv,%Ev",
  0,                  0,                 0,                0,
  0,                  0,                 0,                0,
  0,                  0,                 0,                0,
/* 2 */
  "mov\t%Rd,%Cd",     "mov\t%Rd,%Dd",    "mov\t%Cd,%Rd",   "mov\t%Dd,%Rd",
  "mov\t%Rd,%Td",     0,                 "mov\t%Td,%Rd",   0,
  0,                  0,                 0,                0,
  0,                  0,                 0,                0,
/* 3 */
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
/* 4 */
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
/* 5 */
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
/* 6 */
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
/* 7 */
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
/* 8 */
  "jo\t%Jv",           "jno\t%Jv",         "jb\t%Jv",         "jnb\t%Jv",
  "jz\t%Jv",           "jnz\t%Jv",         "jbe\t%Jv",        "ja\t%Jv",
  "js\t%Jv",           "jns\t%Jv",         "jp\t%Jv",         "jnp\t%Jv",
  "jl\t%Jv",           "jge\t%Jv",         "jle\t%Jv",        "jg\t%Jv",
/* 9 */
  "seto\t%Eb",         "setno\t%Eb",       "setc\t%Eb",       "setnc\t%Eb",
  "setz\t%Eb",         "setnz\t%Eb",       "setbe\t%Eb",      "setnbe\t%Eb",
  "sets\t%Eb",         "setns\t%Eb",       "setp\t%Eb",       "setnp\t%Eb",
  "setl\t%Eb",         "setge\t%Eb",       "setle\t%Eb",      "setg\t%Eb",
/* a */
  "push\tfs",          "pop\tfs",          0,                "bt\t%Ev,%Gv",
  "shld\t%Ev,%Gv,%Ib", "shld\t%Ev,%Gv,cl", 0,                0,
  "push\tgs",          "pop\tgs",          0,                "bts\t%Ev,%Gv",
  "shrd\t%Ev,%Gv,%Ib", "shrd\t%Ev,%Gv,cl", 0,                "imul\t%Gv,%Ev",
/* b */
  "cmpxchg\t%Eb,%Gb",  "cmpxchg\t%Ev,%Gv", "lss\t%Mp",        "btr\t%Ev,%Gv",
  "lfs\t%Mp",          "lgs\t%Mp",         "movzx\t%Gv,%Eb",  "movzx\t%Gv,%Ew",
  0,                  0,                 "%g7 %Ev,%Ib",    "btc\t%Ev,%Gv",
  "bsf\t%Gv,%Ev",      "bsr\t%Gv,%Ev",     "movsx\t%Gv,%Eb",  "movsx\t%Gv,%Ew",
/* c */
  "xadd\t%Eb,%Gb",     "xadd\t%Ev,%Gv",    0,                0,
  0,                  0,                 0,                0,
  "bswap\teax",        "bswap\tecx",       "bswap\tedx",      "bswap\tebx",
  "bswap\tesp",        "bswap\tebp",       "bswap\tesi",      "bswap\tedi",
/* d */
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
/* e */
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
/* f */
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
};


static const char* groups[][8] = {   /* group 0 is group 3 for %Ev set */
/* 0 */
  { "add",           "or",            "adc",          "sbb",
    "and",           "sub",           "xor",          "cmp"           },
/* 1 */
  { "rol",           "ror",           "rcl",          "rcr",
    "shl",           "shr",           "shl",          "sar"           },
/* 2 */  /* v   v*/
  { "test\t%Eq,%Iq",   "test\t%Eq,%Iq",   "not\t%Ev",       "neg\t%Ev",
    "mul\t%Ec",        "imul\t%Ec",       "div\t%Ec",       "idiv\t%Ec" },
/* 3 */
  { "inc\t%Eb",        "dec\t%Eb",        0,                0,
    0,                 0,                 0,                0               },
/* 4 */
  { "inc\t%Ev",        "dec\t%Ev",        "call\t%Kn%Ev",   "call\t%Kf%Ep",
    "jmp\t%Kn%Ev",     "jmp\t%Kf%Ep",     "push\t%Ev",      0               },
/* 5 */
  { "sldt\t%Ew",       "str\t%Ew",        "lldt\t%Ew",      "ltr\t%Ew",
    "verr\t%Ew",       "verw\t%Ew",       0,                0               },
/* 6 */
  { "sgdt\t%Ms",       "sidt\t%Ms",       "lgdt\t%Ms",      "lidt\t%Ms",
    "smsw\t%Ew",       0,                 "lmsw\t%Ew",      0               },
/* 7 */
  { 0,                 0,                 0,                0,
    "bt",            "bts",           "btr",          "btc"           }
};

static DWORD group_flags[][8] = {   /* group 0 is group 3 for %Ev set */
/* 0 */
  //{ "add",                      "or",                         "adc",                        "sbb",
  { DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_BIT, DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD,
  //  "and",                      "sub",                        "xor",                        "cmp"       },
    DISASM_FL_MATH|DISASM_FL_BIT, DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_BIT, DISASM_FL_CMP },
/* 1 */
  //{ "rol",                      "ror",                        "rcl",                        "rcr",
  { DISASM_FL_MATH,               DISASM_FL_MATH,               DISASM_FL_MATH,               DISASM_FL_MATH,
  //  "shl",                      "shr",                        "shl",                        "sar"       },
    DISASM_FL_MATH,               DISASM_FL_MATH,               DISASM_FL_MATH,               DISASM_FL_MATH },
/* 2 */  /* v   v*/
  //{ "test\t%Eq,%Iq",            "test\t%Eq,%Iq",              "not\t%Ev",                   "neg\t%Ev",
  { DISASM_FL_CMP,                DISASM_FL_CMP,                DISASM_FL_MATH|DISASM_FL_BIT, DISASM_FL_MATH,
  //  "mul\t%Ec",                 "imul\t%Ec",                  "div\t%Ec",                   "idiv\t%Ec" },
    DISASM_FL_MATH|DISASM_FL_MUL, DISASM_FL_MATH|DISASM_FL_MUL, DISASM_FL_MATH|DISASM_FL_MUL, DISASM_FL_MATH|DISASM_FL_MUL },
/* 3 */
  //{ "inc\t%Eb",                 "dec\t%Eb",                   0,                            0,
  { DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD, 0,                            0,
  //  0,                          0,                            0,                            0           },
      0,                          0,                            0,                            0           },
/* 4 */
  //{ "inc\t%Ev",                 "dec\t%Ev",                   "call\t%Kn%Ev",               "call\t%Kf%Ep",
  { DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_MATH|DISASM_FL_ADD, DISASM_FL_CALL|DISASM_FL_ABS, DISASM_FL_CALL|DISASM_FL_ABS,
  //  "jmp\t%Kn%Ev",              "jmp\t%Kf%Ep",                "push\t%Ev",                  0           },
    DISASM_FL_JMP|DISASM_FL_ABS,  DISASM_FL_JMP|DISASM_FL_ABS,  DISASM_FL_PUSH,               0           },
/* 5 */
  //{ "sldt\t%Ew",                "str\t%Ew",                   "lldt\t%Ew",                  "ltr\t%Ew",
  { DISASM_FL_MOV,                DISASM_FL_MOV,                DISASM_FL_MOV,                DISASM_FL_MOV,
  //  "verr\t%Ew",                "verw\t%Ew",                  0,                            0           },
    0,                            0,                            0,                            0           },
/* 6 */
  //{ "sgdt\t%Ms",                "sidt\t%Ms",                  "lgdt\t%Ms",                  "lidt\t%Ms",
  { 0,                            0,                            DISASM_FL_MOV,                DISASM_FL_MOV,
  //  "smsw\t%Ew",                0,                            "lmsw\t%Ew",                  0           },
    0,                            0,                            0,                            0           },
/* 7 */
  //{ 0,                          0,                            0,                            0,
  { 0,                            0,                            0,                            0,
  //  "bt",                       "bts",                        "btr",                        "btc"       }
    0,                            0,                            0,                            0           }
};

/* zero here means invalid.  If first entry starts with '*', use st(i) */
/* no assumed %EFs here.  Indexed by RM(modrm())                       */
static const char* f0[]     = { 0, 0, 0, 0, 0, 0, 0, 0};
static const char* fop_9[]  = { "*fxch st,%GF" };
static const char* fop_10[] = { "fnop", 0, 0, 0, 0, 0, 0, 0 };
static const char* fop_12[] = { "fchs", "fabs", 0, 0, "ftst", "fxam", 0, 0 };
static const char* fop_13[] = { "fld1", "fldl2t", "fldl2e", "fldpi",
                                "fldlg2", "fldln2", "fldz", 0 };
static const char* fop_14[] = { "f2xm1", "fyl2x", "fptan", "fpatan",
                                "fxtract", "fprem1", "fdecstp", "fincstp" };
static const char* fop_15[] = { "fprem", "fyl2xp1", "fsqrt", "fsincos",
                                "frndint", "fscale", "fsin", "fcos" };
static const char* fop_21[] = { 0, "fucompp", 0, 0, 0, 0, 0, 0 };
static const char* fop_28[] = { 0, 0, "fclex", "finit", 0, 0, 0, 0 };
static const char* fop_32[] = { "*fadd\t%GF,st" };
static const char* fop_33[] = { "*fmul\t%GF,st" };
static const char* fop_36[] = { "*fsubr\t%GF,st" };
static const char* fop_37[] = { "*fsub\t%GF,st" };
static const char* fop_38[] = { "*fdivr\t%GF,st" };
static const char* fop_39[] = { "*fdiv\t%GF,st" };
static const char* fop_40[] = { "*ffree\t%GF" };
static const char* fop_42[] = { "*fst\t%GF" };
static const char* fop_43[] = { "*fstp\t%GF" };
static const char* fop_44[] = { "*fucom\t%GF" };
static const char* fop_45[] = { "*fucomp\t%GF" };
static const char* fop_48[] = { "*faddp\t%GF,st" };
static const char* fop_49[] = { "*fmulp\t%GF,st" };
static const char* fop_51[] = { 0, "fcompp", 0, 0, 0, 0, 0, 0 };
static const char* fop_52[] = { "*fsubrp\t%GF,st" };
static const char* fop_53[] = { "*fsubp\t%GF,st" };
static const char* fop_54[] = { "*fdivrp\t%GF,st" };
static const char* fop_55[] = { "*fdivp\t%GF,st" };
static const char* fop_60[] = { "fstsw\tax", 0, 0, 0, 0, 0, 0, 0 };

static const char **fspecial[] = { /* 0=use st(i), 1=undefined 0 in fop_* means undefined */
  0, 0, 0, 0, 0, 0, 0, 0,
  0, fop_9, fop_10, 0, fop_12, fop_13, fop_14, fop_15,
  f0, f0, f0, f0, f0, fop_21, f0, f0,
  f0, f0, f0, f0, fop_28, f0, f0, f0,
  fop_32, fop_33, f0, f0, fop_36, fop_37, fop_38, fop_39,
  fop_40, f0, fop_42, fop_43, fop_44, fop_45, f0, f0,
  fop_48, fop_49, f0, fop_51, fop_52, fop_53, fop_54, fop_55,
  f0, f0, f0, f0, fop_60, f0, f0, f0,
};

static const char* floatops[] = { /* assumed " %EF" at end of each.  mod != 3 only */
/*00*/ "fadd", "fmul", "fcom", "fcomp",
       "fsub", "fsubr", "fdiv", "fdivr",
/*08*/ "fld", 0, "fst", "fstp",
       "fldenv", "fldcw", "fstenv", "fstcw",
/*16*/ "fiadd", "fimul", "ficomw", "ficompw",
       "fisub", "fisubr", "fidiv", "fidivr",
/*24*/ "fild", 0, "fist", "fistp",
       "frstor", "fldt", 0, "fstpt",
/*32*/ "faddq", "fmulq", "fcomq", "fcompq",
       "fsubq", "fsubrq", "fdivq", "fdivrq",
/*40*/ "fldq", 0, "fstq", "fstpq",
       0, 0, "fsave", "fstsw",
/*48*/ "fiaddw", "fimulw", "ficomw", "ficompw",
       "fisubw", "fisubrw", "fidivw", "fidivr",
/*56*/ "fildw", 0, "fistw", "fistpw",
       "fbldt", "fildq", "fbstpt", "fistpq"
};
