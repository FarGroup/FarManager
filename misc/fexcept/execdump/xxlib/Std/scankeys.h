#ifndef __SCAN_KEYS
#define __SCAN_KEYS

/***************************************
          PROCEDURE MACROS
 ***************************************/
#define MK_KEY_ANSII(sh,sc,a)   ((DWORD)(((DWORD)((sh)&0xFFU))<<24) + (((DWORD)((sc)&0xFFU))<<16) + (((DWORD)a)&0xFFFFU))
#define MK_KEY(sh,sc)           MK_KEY_ANSII( sh,sc,scDONT_CARE )
#define KEY_SHIFT( key )        ( (BYTE)(((key)>>24) & 0xFFU) )
#define KEY_SCAN( key )         ( (BYTE)(((key)>>16) & 0xFFU) )
#define KEY_ANSII( key )        ( (WORD)((key) & 0xFFFFU) )
#define IS_KEY( key1,key2)      ( (KEY_SHIFT(key2) == (scDONT_CARE & 0xFFU)   ||                                                    \
                                    KEY_SHIFT(key1) == KEY_SHIFT(key2))       &&  (KEY_SCAN(key2) == (scDONT_CARE & 0xFFU)    ||    \
                                    KEY_SCAN(key1) == KEY_SCAN(key2))         &&  (KEY_ANSII(key2) == (scDONT_CARE & 0xFFFFU) ||    \
                                    KEY_ANSII(key1) == KEY_ANSII(key2))                                                             \
                                )

#define scDONT_CARE 0xFFFFFFFFUL
#define scNOSCAN    0xFFU
#define scNOANSII   0xFFFFU
#define scNOSHIFT   0xFFU

/***************************************
          SHIFT FLAGS MACRO
 ***************************************/
#define mskRELEASE  0x80

#define mskLSHIFT       0x01
#define mskRSHIFT       0x02
#define mskSHIFT        (mskLSHIFT | mskRSHIFT)
#define mskLCONTROL     0x04
#define mskRCONTROL     0x08
#define mskCONTROL      (mskLCONTROL | mskRCONTROL)
#define mskLALT         0x10
#define mskRALT         0x20
#define mskALT          (mskLALT | mskRALT)
#define mskLAPP         0x40
#define mskRAPP         0x80
#define mskAPP          (mskLAPP | mskRAPP)

#define mskALLMASK      (mskSHIFT | mskALT | mskCONTROL | mskAPP)

#if defined(__GNUC__) || defined(__QNX__) || defined(__HDOS__ ) || defined(__HWIN__ )
//!  HARDWARE  scan codes
#define hscL_SHIFT       0x2A
#define hscR_SHIFT       0x36
#define hscSHIFT         hscL_SHIFT
#define hscALT           0x38
#define hscCONTROL       0x1D
#define hscCAPSLOCK      0xBA
#define hscNUMLOCK       0xC5
#define hscSCROLLLOCK    0xC6
#define hscPRINTSCREEN   0xAA
#define hscPAUSE         0xFA

#define hscESC       0x01

#define hscF1        0x3B
#define hscF2        0x3C
#define hscF3        0x3D
#define hscF4        0x3E
#define hscF5        0x3F
#define hscF6        0x40
#define hscF7        0x41
#define hscF8        0x42
#define hscF9        0x43
#define hscF10       0x44
#define hscF11       0x57
#define hscF12       0x58

#define hscTILDA     0x29
#define hsc1         0x02
#define hsc2         0x03
#define hsc3         0x04
#define hsc4         0x05
#define hsc5         0x06
#define hsc6         0x07
#define hsc7         0x08
#define hsc8         0x09
#define hsc9         0x0A
#define hsc0         0x0B
#define hscMINUS     0x0C
#define hscEQUAL     0x0D
#define hscBKSLASH   0x2B
#define hscBACKSPACE 0x0E

#define hscTAB       0x0F
#define hscQ         0x10
#define hscW         0x11
#define hscE         0x12
#define hscR         0x13
#define hscT         0x14
#define hscY         0x15
#define hscU         0x16
#define hscI         0x17
#define hscO         0x18
#define hscP         0x19
#define hscLBOX      0x1A
#define hscRBOX      0x1B
#define hscRETURN    0x1C

#define hscA         0x1E
#define hscS         0x1F
#define hscD         0x20
#define hscF         0x21
#define hscG         0x22
#define hscH         0x23
#define hscJ         0x24
#define hscK         0x25
#define hscL         0x26
#define hscSEMICOLON 0x27
#define hscCHAR      0x28

#define hscZ         0x2C
#define hscX         0x2D
#define hscC         0x2E
#define hscV         0x2F
#define hscB         0x30
#define hscN         0x31
#define hscM         0x32
#define hscLESS      0x33
#define hscMORE      0x34
#define hscQUESTION  0x35

#define hscSPACE     0x39

#define hscSLASH     0x35
#define hscSTAR      0x37
#define hscGRMINUS   0x4A
#define hscGRPLUS    0x4E
#define hscHOME      0x47
#define hscUP        0x48
#define hscPGUP      0x49
#define hscLEFT      0x4B
#define hscCENTER    0x4C
#define hscRIGHT     0x4D
#define hscEND       0x4F
#define hscDOWN      0x50
#define hscPGDOWN    0x51
#define hscINS       0x52
#define hscDEL       0x53
#endif //x86

//! DOS, QNX, PROTDOS
#if defined(__GNUC__) || defined(__QNX__) || defined(__HDOS__ )
/***************************************
        NON SCAN CODE DEFINES
 ***************************************/
#define scNOKEY         0xFF             // no valid SCAN or ANSII kode
#define scSYSTEM        0x70             // System from 0x70 to 0x80
#define scTIMER         0x70
#define scSIZECHANGED   0x71             //Size of screen changed
#define scSCREENDRAW    0x72             //External Draw encounted
#define scSETFOCUS      0x73             //Screen got focus
#define scKILLFOCUS     0x74             //Screen lost focus
#define scSYSTEM_LAST   0xFF

#define scUSER          0x80             // User from 0xA0 to 0zFF

/***************************************
          SCAN CODES MACRO
 ***************************************/
#define scL_SHIFT       hscL_SHIFT
#define scR_SHIFT       hscR_SHIFT
#define scSHIFT         hscSHIFT
#define scALT           hscALT
#define scCONTROL       hscCONTROL
#define scCAPSLOCK      hscCAPSLOCK
#define scNUMLOCK       hscNUMLOCK
#define scSCROLLLOCK    hscSCROLLLOCK
#define scPRINTSCREEN   hscPRINTSCREEN
#define scPAUSE         hscPAUSE

#define scESC           hscESC

#define scF1            hscF1
#define scF2            hscF2
#define scF3            hscF3
#define scF4            hscF4
#define scF5            hscF5
#define scF6            hscF6
#define scF7            hscF7
#define scF8            hscF8
#define scF9            hscF9
#define scF10           hscF10
#define scF11           hscF11
#define scF12           hscF12

#define scTILDA         hscTILDA
#define sc1             hsc1
#define sc2             hsc2
#define sc3             hsc3
#define sc4             hsc4
#define sc5             hsc5
#define sc6             hsc6
#define sc7             hsc7
#define sc8             hsc8
#define sc9             hsc9
#define sc0             hsc0
#define scMINUS         hscMINUS
#define scEQUAL         hscEQUAL
#define scBKSLASH       hscBKSLASH
#define scBACKSPACE     hscBACKSPACE

#define scTAB           hscTAB
#define scQ             hscQ
#define scW             hscW
#define scE             hscE
#define scR             hscR
#define scT             hscT
#define scY             hscY
#define scU             hscU
#define scI             hscI
#define scO             hscO
#define scP             hscP
#define scLBOX          hscLBOX
#define scRBOX          hscRBOX
#define scRETURN        hscRETURN

#define scA             hscA
#define scS             hscS
#define scD             hscD
#define scF             hscF
#define scG             hscG
#define scH             hscH
#define scJ             hscJ
#define scK             hscK
#define scL             hscL
#define scSEMICOLON     hscSEMICOLON
#define scCHAR          hscCHAR

#define scZ             hscZ
#define scX             hscX
#define scC             hscC
#define scV             hscV
#define scB             hscB
#define scN             hscN
#define scM             hscM
#define scLESS          hscLESS
#define scMORE          hscMORE
#define scQUESTION      hscQUESTION

#define scSPACE         hscSPACE

#define scSLASH         hscSLASH
#define scSTAR          hscSTAR
#define scGRMINUS       hscGRMINUS
#define scGRPLUS        hscGRPLUS
#define scHOME          hscHOME
#define scUP            hscUP
#define scPGUP          hscPGUP
#define scLEFT          hscLEFT
#define scCENTER        hscCENTER
#define scRIGHT         hscRIGHT
#define scEND           hscEND
#define scDOWN          hscDOWN
#define scPGDOWN        hscPGDOWN
#define scINS           hscINS
#define scDEL           hscDEL

#endif //QNX, DOS

//! WINDOWS
#if defined(__HWIN__)
#define scNOKEY         0xFF             // not valid SCAN or ANSII kode
#define scSYSTEM        0xE0             // System: 0xE0 - 0xF0
#define scTIMER         0xE1
#define scSIZECHANGED   0xE2             //Size of screen changed
#define scSCREENDRAW    0xFE             //External Draw encounted
#define scSETFOCUS      0xFD             //Screen got focus
#define scKILLFOCUS     0xFC             //Screen lost focus
#define scSYSTEM_LAST   0xFF

#define scUSER          0xD0             // User: 0xD0 - 0xFE

#if defined(__HWIN16__)
#define scL_SHIFT       VK_SHIFT
#define scR_SHIFT       VK_SHIFT
#else
#define scL_SHIFT       VK_LSHIFT
#define scR_SHIFT       VK_RSHIFT
#endif

#define scSHIFT         VK_SHIFT
#define scSHIFT         VK_SHIFT
#define scALT           VK_MENU
#define scCONTROL       VK_CONTROL
#define scCAPSLOCK      VK_CAPITAL
#define scNUMLOCK       VK_NUMLOCK
#define scSCROLLLOCK    VK_SCROLL
#define scPRINTSCREEN   VK_PRINT
#define scPAUSE         VK_PAUSE

#define scESC       VK_ESCAPE

#define scF1        VK_F1
#define scF2        VK_F2
#define scF3        VK_F3
#define scF4        VK_F4
#define scF5        VK_F5
#define scF6        VK_F6
#define scF7        VK_F7
#define scF8        VK_F8
#define scF9        VK_F9
#define scF10       VK_F10
#define scF11       VK_F11
#define scF12       VK_F12

#define scTILDA     0x40
#define sc1         '1'
#define sc2         '2'
#define sc3         '3'
#define sc4         '4'
#define sc5         '5'
#define sc6         '6'
#define sc7         '7'
#define sc8         '8'
#define sc9         '9'
#define sc0         '0'
#define scMINUS     0x3D
#define scEQUAL     0x3B
#define scBKSLASH   0x5C
#define scBACKSPACE VK_BACK

#define scTAB       VK_TAB
#define scQ         'Q'
#define scW         'W'
#define scE         'E'
#define scR         'R'
#define scT         'T'
#define scY         'Y'
#define scU         'U'
#define scI         'I'
#define scO         'O'
#define scP         'P'
#define scLBOX      '['
#define scRBOX      ']'
#define scRETURN    VK_RETURN

#define scA         'A'
#define scS         'S'
#define scD         'D'
#define scF         'F'
#define scG         'G'
#define scH         'H'
#define scJ         'J'
#define scK         'K'
#define scL         'L'
#define scSEMICOLON 0x3A
#define scCHAR      0x5E

#define scZ         'Z'
#define scX         'X'
#define scC         'C'
#define scV         'V'
#define scB         'B'
#define scN         'N'
#define scM         'M'
#define scLESS      0x3C
#define scMORE      0x3E
#define scQUESTION  0x3F

#define scSPACE     VK_SPACE

#define scSLASH     VK_DIVIDE
#define scSTAR      VK_MULTIPLY
#define scGRMINUS   VK_SUBTRACT
#define scGRPLUS    VK_ADD
#define scHOME      VK_HOME
#define scUP        VK_UP
#define scPGUP      VK_PRIOR
#define scLEFT      VK_LEFT
#define scCENTER    VK_DECIMAL
#define scRIGHT     VK_RIGHT
#define scEND       VK_END
#define scDOWN      VK_DOWN
#define scPGDOWN    VK_NEXT
#define scINS       VK_INSERT
#define scDEL       VK_DELETE
#endif //Win32

#endif
