#ifndef __BCB_DEFINES
#define __BCB_DEFINES

#undef __BCB__     //Defined if Borland CPP builder used
#undef __BC__      //Defined if other old compiller used

#undef __BCB1__
#undef __BCB3__
#undef __BCB4__
#undef __BCB5__
#undef __BCB6__

#define __BC31__   0x0410
#define __BC402__  0x0452
#define __BC450__  0x0460
#define __BC500__  0x0500

#define __BCB10__  0x0520
#define __BCB30__  0x0530
#define __BCB40__  0x0540
#define __BCB50__  0x0550
#define __BCB60__  0x0560

/*******************************************************************
   BCB 1.0
 *******************************************************************/
#if __BORLANDC__ <= __BC31__
  #define __BC__    1
#else
/*******************************************************************
   BCB 1.0
 *******************************************************************/
#if __BORLANDC__ >= __BCB10__ && __BORLANDC__ < __BCB30__
  #pragma anon_struct on      // support anonymous structs (within unions)

  #define __BCB1__  1
  #define __BCB__   1
  #define __VCL__   1
#else
/*******************************************************************
   BCB 3.0
 *******************************************************************/
#if __BORLANDC__ >= __BCB30__ && __BORLANDC__ < __BCB40__
  #pragma anon_struct on      // support anonymous structs (within unions)

  #define HAS_ANONSTRUCT  1
  #define __BCB3__  1
  #define __BCB__   1
  #define __VCL__   1
#else
/*******************************************************************
   BCB 4.0
 *******************************************************************/
#if __BORLANDC__ >= __BCB40__ && __BORLANDC__ < __BCB50__

  #define HAS_ANONSTRUCT  1
  #define __BCB4__  1
  #define __BCB__   1
#else
/*******************************************************************
   BCB 5.0
 *******************************************************************/
#if __BORLANDC__ >= __BCB50__ && __BORLANDC__ < __BCB60__

  #define HAS_ANONSTRUCT  1
  #define __BCB5__  1
  #define __BCB__   1
  #define __VCL__   1
#else
/*******************************************************************
   BCB 6.0
 *******************************************************************/
#if __BORLANDC__ >= __BCB60__

  #define HAS_ANONSTRUCT  1
  #define __BCB6__  1
  #define __BCB__   1
  #define __VCL__   1
#else
/*******************************************************************
   BC 3.1
 *******************************************************************/
#if __BORLANDC__ >= __BC310__ && __BORLANDC__ < __BC402__

#else
/*******************************************************************
   BC 4.02
 *******************************************************************/
#if __BORLANDC__ >= __BC402__ && __BORLANDC__ < __BC450__

#else
/*******************************************************************
   BC 4.5
 *******************************************************************/
#if __BORLANDC__ >= __BC450__ && __BORLANDC__ < __BC500__

#else
/*******************************************************************
   BC 5.0
 *******************************************************************/
#if __BORLANDC__ >= __BC500__ && __BORLANDC__ < __BCB10__

#else
  #error "Unknown BCB version. Specify differents for it"
#endif  //BC 3.1
#endif  //BC 4.02
#endif  //BC 4.5
#endif  //BC 5.0
#endif  //BCB 6
#endif  //BCB 5
#endif  //BCB 4
#endif  //BCB 3
#endif  //BCB 1
#endif  //BC 3.1

/*
 #pragma warn <name> ³ <code>

   Name  Code           Descripton

   -ali  -8086        Incorrect use of #pragma alias "aliasName" = "substitutename" (Default ON)
   amb   8000         Ambiguous operators need parentheses (Default OFF)
   amp   8001         Superfluous & with function (Default OFF)
   -asc  -8002        Restarting compile using assembly (Default ON)
   asm   8003         Unknown assembler instruction (Default OFF)
   -aus  -8004        'identifier' is assigned a value that is never used (Default ON)

   bbf   8005         Bit fields must be signed or unsigned int (Default OFF)
   -bei  -8006        Initializing 'identifier' with 'identifier' (Default ON)
   -big  -8007        Hexadecimal value contains more than three digits (Default ON)
   -ccc  -8008        Condition is always true OR Condition is always false (Default ON)
   cln   8009         Constant is long (Default OFF)
   -cod  -8093        Incorrect use of #pragma codeseg (Default ON)

   -com  -8010        Continuation character \ found in // comment (Default ON)
   -cpt  -8011        Nonportable pointer comparison (Default ON)
   -csu  -8012        Comparing signed and unsigned values (Default ON)
   def   8013         Possible use of 'identifier' before definition (Default OFF)
   -dig  -8014        Declaration ignored (Default ON)
   -dpu  -8015        Declare 'type' prior to use in prototype (Default ON)

   -dsz  -8016        Array size for 'delete' ignored (Default ON)
   -dup  -8017        Redefinition of 'macro' is not identical (Default ON)
   -eas  -8018        Assigning 'type' to 'enum'  (Default ON)
   -eff  -8019        Code has no effect (Default ON)
   -ext  -8020        'identifier' is declared as both external and static (Default ON)
   -hch  -8021        Handler for 'type1' Hidden by Previous Handler for 'type2' (Default ON)

   -hid  -8022        'function1' hides virtual function 'function2' (Default ON)
   -ias  -8023        Array variable 'identifier' is near (Default ON)
   -ibc  -8024        Base class 'class1' is also a base class of 'class2' (Default ON)
   -ifr  -8085        Function 'function' redefined as non-inline (Default ON)
   -ill  -8025        Ill-formed pragma (Default ON)
   -inl  -8026        Functions containing certain constructs are not expanded inline (Default ON)

   -inl  -8027        Functions containing reserved words are not expanded inline (Default ON)
   -lin  -8028        Temporary used to initialize 'identifier' (Default ON)
   -lvc  -8029        Temporary used for parameter 'parameter' (Default ON)
   -lvc  -8030        Temporary used for parameter 'parameter' in call to 'function' (Default ON)
   -lvc  -8031        Temporary used for parameter number (Default ON)

   -lvc  -8032        Temporary used for parameter number in call to 'function' (Default ON)
   -mcs  -8096        Incorrect use of #pragma code_seg (Default ON)
   -mes  -8095        Incorrect use of #pragma message (Default ON)
   -mpc  -8033        Conversion to 'type' fails for members of virtual base 'base' (Default ON)
   -mpd  -8034        Maximum precision used for member pointer type 'type' (Default ON)

   -msg  -8035        User-defined warnings (Default ON)
   nak   8036         Non-ANSI Keyword Used: 'keyword' (Default OFF)
   ote: U of th       is option is required for ANSI conformance)
   -ncf  -8037        Non-const function 'function' called for const object (Default ON)
   -nci  -8038        Constant member 'identifier' is not initialized (Default ON)
   -ncl  -8039        Constructor initializer list ignored (Default ON)

   -nfd  -8040        Function body ignored (Default ON)
   -ngu  -8041        Negating unsigned value (Default ON)
   -nin  -8042        Initializer for object 'identifier' ignored (Default ON)
   -nma  -8043        Macro definition ignored (Default ON)
   -nmu  -8044        #undef directive ignored (Default ON)
   nod   8045         No declaration for function 'function' (Default OFF)
   -nop  -8046        Pragma option pop with no matching option push (Default ON)

   -npp  -8083        Pragma pack pop with no matching pack push (Default ON)
   -nsf  -8047        Declaration of static function 'function(...)' ignored (Default ON)
   -nst  -8048        Use qualified name to access nested type 'type' (Default ON)
   -ntd  -8049        Use '> >' for nested templates instead of '>>' (Default ON)
   -nto  -8050        No type OBJ file present. Disabling external types option. (Default ON)

   -nvf  -8051        Non-volatile function 'function' called for volatile object (Default ON)
   -obi  -8052        Base initialization without a class name is now obsolete (Default ON)
   -obs  -8053        'identifier' is obsolete (Default ON)
   -ofp  -8054        Style of function definition is now obsolete (Default ON)
   -onr  -8097        Not all options can be restored at this time (Default ON)
   -osh  -8055        Possible overflow in shift operation (Default ON)

   -ovf  -8056        Integer arithmetic overflow (Default ON)
   -par  -8057        Parameter 'parameter' is never used (Default ON)
   -pch  -8058        Cannot create pre-compiled header: 'header' (Default ON)
   -pck  -8059        Structure packing size has changed (Default ON)
   -pcm  -8094        Incorrect use of #pragma comment (Default ON)
   -pia  -8060        Possibly incorrect assignment (Default ON)

   pin   8061         Initialization is only partially bracketed (Default OFF)
   -pow  -8062        Previous options and warnings not restored (Default ON)
   -prc  -8084        Suggest parentheses to clarify precedence (Default OFF)
   -pre  -8063        Overloaded prefix operator 'operator' used as a postfix operator (Default ON)
   -pro  -8064        Call to function 'function' with no prototype (Default ON)
   -pro  -8065        Call to function 'function' with no prototype (Default ON)

   -rch  -8066        Unreachable code (Default ON)
   -ret  -8067        Both return and return of a value used (Default ON)
   -rng  -8068        Constant out of range in comparison (Default ON)
   -rpt  -8069        Nonportable pointer conversion (Default ON)
   -rvl  -8070        Function should return a value (Default ON)
   sig   8071         Conversion may lose significant digits (Default OFF)
   -spa  -8072        Suspicious pointer arithmetic (Default ON)

   -stl  -8087        'operator==' must be publicly visible to be contained by a 'name' (Default OFF)
   -stl  -8089        'operator<' must be publicly visible to be contained by a 'name' (Default OFF)
   -stl  -8090        'operator<' must be publicly visible to be used by a 'name' (Default OFF)
   -stl  -8091        'type' argument 'argument' passed to 'function' is a 'type' iterator. 'type' iterator required (Default OFF)
   -stl  -8092        'type' argument 'argument' passed to 'function' is not an iterator. 'type' iterator required (Default OFF)

   stu   8073         Undefined structure 'structure' (Default OFF)
   stv   8074         Structure passed by value (Default OFF)
   -sus  -8075        Suspicious pointer conversion (Default ON)
   -tai  -8076        Template instance 'instance' is already instantiated (Default ON)
   -tes  -8077        Explicitly specializing an explicitly specialized class member makes no sense (Default ON)
   -thr  -8078        Throw expression violates exception specification (Default ON)

   ucp   8079         Mixing pointers to different 'char' types (Default OFF)
   use   8080         'identifier' declared but never used (Default OFF)
   -voi  -8081        void functions may not return a value (Default ON)
   -zdi  -8082        Division by zero (Default ON)
*/

#endif
