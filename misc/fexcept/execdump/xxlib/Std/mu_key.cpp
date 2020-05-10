#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif


static char ANSIIChars[] =
    "~1234567890"
    "-=\\qwertyuiop"
    "[]asdfghjkl;"
    "\'zxcvbnm,./"
    " /*-+";
static BYTE ScanChars[] = {
    scTILDA,   sc1, sc2, sc3, sc4, sc5, sc6, sc7, sc8, sc9, sc0,
    scMINUS, scEQUAL,scBKSLASH, scQ, scW, scE, scR, scT, scY, scU, scI, scO, scP,
    scLBOX,  scRBOX, scA,       scS, scD, scF, scG, scH, scJ, scK, scL, scSEMICOLON,
    scCHAR,  scZ,    scX,       scC, scV, scB, scN, scM, scLESS, scMORE, scQUESTION,
    scSPACE, scSLASH, scSTAR, scGRMINUS, scGRPLUS
};

static CTScanStruct stdScans[] = {
{ scTIMER        , "Timer" },            { scSIZECHANGED  , "SiseChanged" },
{ scSCREENDRAW   , "ScreenDraw" },       { scSETFOCUS     , "SetFocus" },
{ scKILLFOCUS    , "KillFocus" },        { scL_SHIFT      , "LShift" },
{ scR_SHIFT      , "RSift" },            { scSHIFT        , "Shift" },
{ scALT          , "Alt" },              { scCONTROL      , "Control" },
{ scCAPSLOCK     , "CapsLock" },         { scNUMLOCK      , "Numlock" },
{ scSCROLLLOCK   , "ScrollLock" },       { scPRINTSCREEN  , "PrintScreen" },
{ scPAUSE        , "Pause" },            { scESC          , "ESC" },
{ scF1           , "F1" },               { scF2           , "F2" },
{ scF3           , "F3" },               { scF4           , "F4" },
{ scF5           , "F5" },               { scF6           , "F6" },
{ scF7           , "F7" },               { scF8           , "F8" },
{ scF9           , "F9" },               { scF10          , "F10" },
{ scF11          , "F11" },              { scF12          , "F12" },
{ scTILDA        , "Tilda" },
{ sc1            , "1" },                { sc2            , "2" },
{ sc3            , "3" },                { sc4            , "4" },
{ sc5            , "5" },                { sc6            , "6" },
{ sc7            , "7" },                { sc8            , "8" },
{ sc9            , "9" },                { sc0            , "0" },
{ scMINUS        , "Minus" },            { scEQUAL        , "Equal" },
{ scBKSLASH      , "BkSlash" },          { scBACKSPACE    , "BackSpace" },
{ scTAB          , "Tab" },              { scQ            , "Q" },
{ scW            , "W" },                { scE            , "E" },
{ scR            , "R" },                { scT            , "T" },
{ scY            , "Y" },                { scU            , "U" },
{ scI            , "I" },                { scO            , "O" },
{ scP            , "P" },                { scLBOX         , "LBox" },
{ scRBOX         , "RBox" },             { scRETURN       , "Return" },
{ scA            , "A" },                { scS            , "S" },
{ scD            , "D" },                { scF            , "F" },
{ scG            , "G" },                { scH            , "H" },
{ scJ            , "J" },                { scK            , "K" },
{ scL            , "L" },                { scSEMICOLON    , "Semicolon" },
{ scCHAR         , "Char" },             { scZ            , "Z" },
{ scX            , "X" },                { scC            , "C" },
{ scV            , "V" },                { scB            , "B" },
{ scN            , "N" },                { scM            , "M" },
{ scLESS         , "Less" },             { scMORE         , "More" },
{ scQUESTION     , "Question" },         { scSPACE        , "Space" },
{ scSLASH        , "Slash" },            { scSTAR         , "Star" },
{ scGRMINUS      , "GrMinus" },          { scGRPLUS       , "GrPlus" },
{ scHOME         , "Home" },             { scUP           , "Up" },
{ scPGUP         , "PgUp" },             { scLEFT         , "Left" },
{ scCENTER       , "Center" },           { scRIGHT        , "Right" },
{ scEND          , "End" },              { scDOWN         , "Down" },
{ scPGDOWN       , "PgDown" },           { scINS          , "Ins" },
{ scDEL          , "Del" },              { scSYSTEM       , "System" },
{ scUSER         , "User" },             { scNOKEY        , "NoKey"  },
{ 0,NULL } };

STRUCT( HWScanCode )
  BYTE HWScan;
  BYTE Scan;
};

static HWScanCode HWScans[] = {
  { hscL_SHIFT,    scL_SHIFT },       { hscR_SHIFT,    scR_SHIFT },     { hscSHIFT,      scSHIFT },         { hscALT,        scALT },
  { hscCONTROL,    scCONTROL },       { hscCAPSLOCK,   scCAPSLOCK },    { hscNUMLOCK,    scNUMLOCK },       { hscSCROLLLOCK, scSCROLLLOCK },
  { hscPRINTSCREEN,scPRINTSCREEN },   { hscPAUSE,      scPAUSE },       { hscESC,       scESC },            { hscF1,        scF1 },
  { hscF2,        scF2 },             { hscF3,        scF3 },           { hscF4,        scF4 },             { hscF5,        scF5 },
  { hscF6,        scF6 },             { hscF7,        scF7 },           { hscF8,        scF8 },             { hscF9,        scF9 },
  { hscF10,       scF10 },            { hscF11,       scF11 },          { hscF12,       scF12 },            { hscTILDA,     scTILDA },
  { hsc1,         sc1 },              { hsc2,         sc2 },            { hsc3,         sc3 },              { hsc4,         sc4 },
  { hsc5,         sc5 },              { hsc6,         sc6 },            { hsc7,         sc7 },              { hsc8,         sc8 },
  { hsc9,         sc9 },              { hsc0,         sc0 },            { hscMINUS,     scMINUS },          { hscEQUAL,     scEQUAL },
  { hscBKSLASH,   scBKSLASH },        { hscBACKSPACE, scBACKSPACE },    { hscTAB,       scTAB },            { hscQ,         scQ },
  { hscW,         scW },              { hscE,         scE },            { hscR,         scR },              { hscT,         scT },
  { hscY,         scY },              { hscU,         scU },            { hscI,         scI },              { hscO,         scO },
  { hscP,         scP },              { hscLBOX,      scLBOX },         { hscRBOX,      scRBOX },           { hscRETURN,    scRETURN },
  { hscA,         scA },              { hscS,         scS },            { hscD,         scD },              { hscF,         scF },
  { hscG,         scG },              { hscH,         scH },            { hscJ,         scJ },              { hscK,         scK },
  { hscL,         scL },              { hscSEMICOLON, scSEMICOLON },    { hscCHAR,      scCHAR },           { hscZ,         scZ },
  { hscX,         scX },              { hscC,         scC },            { hscV,         scV },              { hscB,         scB },
  { hscN,         scN },              { hscM,         scM },            { hscLESS,      scLESS },           { hscMORE,      scMORE },
  { hscQUESTION,  scQUESTION },       { hscSPACE,     scSPACE },        { hscSLASH,     scSLASH },          { hscSTAR,      scSTAR },
  { hscGRMINUS,   scGRMINUS },        { hscGRPLUS,    scGRPLUS },       { hscHOME,      scHOME },           { hscUP,        scUP },
  { hscPGUP,      scPGUP },           { hscLEFT,      scLEFT },         { hscCENTER,    scCENTER },         { hscRIGHT,     scRIGHT },
  { hscEND,       scEND },            { hscDOWN,      scDOWN },         { hscPGDOWN,    scPGDOWN },         { hscINS,       scINS },
  { hscDEL,       scDEL },
{ 0,0 } };

//! Key
int MYRTLEXP FindHWScan( BYTE scan )
  {
    for ( int n = 0; HWScans[n].Scan; n++ )
      if ( HWScans[n].Scan == scan )
        return n;
 return -1;
}
int MYRTLEXP FindScanHW( BYTE scan )
  {
    for ( int n = 0; HWScans[n].Scan; n++ )
      if ( HWScans[n].HWScan == scan )
        return n;
 return -1;
}
int MYRTLEXP FindScanKey( BYTE scan )
  {
    for ( int n = 0; stdScans[n].Text; n++ )
      if ( stdScans[n].Scan == scan )
        return n;
 return -1;
}
int MYRTLEXP FindScanKey( CONSTSTR str )
  {
    for ( int n = 0; stdScans[n].Text; n++ )
      if ( StrCmp(stdScans[n].Text,str,-1,FALSE) == 0 )
        return n;
 return -1;
}
BYTE MYRTLEXP Scan2HWScan( BYTE sc )
  {  int num = FindHWScan(sc);
  return (BYTE) ((num==-1)?0:HWScans[num].HWScan);
}
BYTE MYRTLEXP HWScan2Scan( BYTE sc )
  {  int num = FindScanHW(sc);
  return (BYTE) ((num==-1)?0:HWScans[num].Scan);
}
CONSTSTR MYRTLEXP Key2Str( DWORD key )
  {  BYTE sc = KEY_SCAN(key),
          sh = KEY_SHIFT(key);
     int  n;
     char ss[10];
     const char* m;
     static char buf[50];

    buf[0] = 0;

    if ( key == scDONT_CARE || KEY_SCAN(key) == scNOKEY ) {
      strcpy( buf,"NoKey" );
      return buf;
    }

    if ( sh == 0xFF )
      strcpy( buf,"x" );
     else {
      if ( (sh&mskCONTROL) != 0 ) {
        if ( buf[0] ) strcat( buf,"+" );
        strcat( buf,"Ctrl" );
      }
      if ( (sh&mskALT) != 0 ) {
        if ( buf[0] ) strcat( buf,"+" );
        strcat( buf,"Alt" );
      }
      if ( (sh&mskSHIFT) != 0 ) {
        if ( buf[0] ) strcat( buf,"+" );
        strcat( buf,"Shift" );
      }
    }
    if ( (n=FindScanKey(sc)) != -1 )
      m = stdScans[n].Text;
     else {
      Sprintf( ss,"0x%X",(short)sc );
      m = ss;
    }
    if ( buf[0] ) strcat( buf,"+" );
    strcat( buf,m );
 return buf;
}

DWORD MYRTLEXP Str2Key( CONSTSTR str )
  {  char s[50],ch;
     int  n,i;
     BYTE sc = 0,
          sh = 0;

    n = 0;
    do{
     for ( i = 0; (ch=str[n]) != 0 && ch != '+'; n++ ) {
       if ( StrChr( "\t\n\r\b",ch)  ) continue;
       s[i] = ch;
       i++;
     }

     s[i] = 0; if ( ch ) ch = str[++n];
     if ( StrCmp(s,"X",-1,FALSE) == 0 && ch ) {
       sh = (BYTE)(scDONT_CARE&0xFFU);
     } else
     if ( StrCmp(s,"CTRL",-1,FALSE) == 0 ) {
       SET_FLAG( sh,mskCONTROL );
     } else
     if ( StrCmp(s,"ALT",-1,FALSE) == 0 ) {
       SET_FLAG( sh,mskALT );
     } else
     if ( StrCmp(s,"SHIFT",-1,FALSE) == 0 ) {
       SET_FLAG( sh,mskSHIFT );
     } else
     if ( s[0] == '0' && (s[1] == 'x' || s[1] == 'X') ) {
       i = Str2Digit( s,16,0 );
       sc = (BYTE)i;
     } else {
       i = FindScanKey( s );
       if ( i == -1 || stdScans[i].Scan == scNOKEY )
         return scDONT_CARE;
        else
         sc = stdScans[i].Scan;
     }
    }while( ch );

 return sc ? MK_KEY(sh,sc) : scDONT_CARE;
}

WORD MYRTLEXP Scan2ANSII( BYTE scan )
  {
    if ( scan == 0 || scan == scNOSCAN ) return scNOANSII;

    for ( int n = 0; ANSIIChars[n]; n++ )
      if ( ScanChars[n] == scan )
        return ANSIIChars[n];
 return scNOANSII;
}

BYTE MYRTLEXP ANSII2Scan( char ans )
  {  char ch = (char)tolower(ans);
    for ( int n = 0; ANSIIChars[n]; n++ )
      if ( ANSIIChars[n] == ch )
        return ScanChars[n];
 return scNOSCAN;
}

BOOL MYRTLEXP IsPrintKey( DWORD key )
  {
 return ( KEY_SHIFT(key) == 0 || KEY_SHIFT(key) == mskSHIFT ) &&
          KEY_ANSII(key) != scNOANSII &&
          Scan2ANSII( KEY_SCAN(key) ) != scNOANSII;
}
