// EditCase is FAR manager plugin. It allows to change the case of selected text
// or current (nearest) word in the internal editor.
// This plugin can change case to: lower case, Title Case, UPPER CASE and tOGGLE cASE
// Besides, it has ability of cyclic case change like MS Word by ShiftF3
#define _FAR_USE_FARFINDDATA
#include "plugin.hpp"

#ifdef __GNUC__
#include <limits.h>
#define MAXINT INT_MAX
#else
#include <values.h> //MAXINT
#endif
#include "crt.hpp"

#include "EditLng.hpp"
#include "EditCase.hpp"
// Registry operations
#include "WrapReg.cpp"

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) lpReserved;
  (void) dwReason;
  (void) hDll;
  return TRUE;
}
#endif

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
  IsOldFAR=TRUE;
  if(Info->StructSize >= sizeof(struct PluginStartupInfo))
  {
    ::FSF=*Info->FSF;
    ::Info.FSF=&::FSF;
    IsOldFAR=FALSE;

    FSF.sprintf(PluginRootKey,"%s\\EditCase",Info->RootKey);
    WordDivLen=::Info.AdvControl(::Info.ModuleNumber, ACTL_GETSYSWORDDIV, WordDiv);
    char AddWordDiv[sizeof(WordDiv)];
    GetRegKey(HKEY_CURRENT_USER,"","AddWordDiv",AddWordDiv,"#",sizeof(AddWordDiv));
    WordDivLen += lstrlen(AddWordDiv);
    lstrcat(WordDiv, AddWordDiv);
    WordDivLen += sizeof(" \n\r\t");
    lstrcat(WordDiv, " \n\r\t");
  };
}

HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item)
{
  size_t i;
  struct FarMenuItem MenuItems[5], *MenuItem;
  memset(MenuItems,0,sizeof(MenuItems));
  int Msgs[]={MCaseLower, MCaseTitle, MCaseUpper, MCaseToggle, MCaseCyclic};

  for(MenuItem=MenuItems,i=0; i < sizeof(MenuItems)/sizeof(MenuItems[0]); ++i, ++MenuItem)
  {
      MenuItem->Selected=MenuItem->Checked=MenuItem->Separator=0;
      FSF.sprintf(MenuItem->Text, "%s", GetMsg(Msgs[i])); // Text in menu
  };

  // First item is selected
  MenuItems[0].Selected=TRUE;

  // Show menu
  int MenuCode=Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,
                         GetMsg(MCaseConversion),NULL,"Contents",NULL,NULL,
                         MenuItems,sizeof(MenuItems)/sizeof(MenuItems[0]));
  switch(MenuCode)
  {
      // If menu Escaped
      case -1:
        break;

      default:
       struct EditorInfo ei;
       Info.EditorControl(ECTL_GETINFO,&ei);

       // Current line number
       int CurLine=ei.CurLine;
       // Is anything selected
       BOOL IsBlock=FALSE;

       // Nothing selected?
       if (ei.BlockType!=BTYPE_NONE)
       {
         IsBlock=TRUE;
         CurLine=ei.BlockStartLine;
       }

       // Type of Case Change
       int CCType=MenuCode;

       // Temporary string
       char *NewString=0;

       // Forever :-) (Line processing loop)
       for(;;)
       {
         struct EditorGetString egs;

         // Increase CurLine
         egs.StringNumber=CurLine++;

         // If can't get line
         if (!Info.EditorControl(ECTL_GETSTRING,&egs))
           break; // Exit

         // If last selected line was processed or
         // nothing selected and line is empty
         if ((IsBlock && egs.SelStart==-1) || (!IsBlock && egs.StringLength<=0))
           break; // Exit

         // If something selected, but line is empty
         if (egs.StringLength<=0)
           continue; // Get next line

         // If whole line (with EOL) is selected
         if (egs.SelEnd==-1 || egs.SelEnd>egs.StringLength)
         {
           egs.SelEnd=egs.StringLength;
           if (egs.SelEnd<egs.SelStart)
             egs.SelEnd=egs.SelStart;
         }

         // Memory allocation
         NewString=(char *)malloc(egs.StringLength+1);
         // If memory couldn't be allocated
         if(!NewString)
            break;

         struct EditorConvertText ect;

         // If nothing selected - finding word bounds (what'll be converted)
         if (!IsBlock)
         {
           // Making NewString
           memcpy(NewString,egs.StringText,egs.StringLength);
           NewString[egs.StringLength]=0;
           ect.Text=NewString;
           ect.TextLength=egs.StringLength;
           // Convert to OEM
           Info.EditorControl(ECTL_EDITORTOOEM,&ect);

           // Like whole line is selected
           egs.SelStart=0;
           egs.SelEnd=egs.StringLength;

           // Finding word bounds (what'll be converted)
           FindBounds(NewString, egs.StringLength, ei.CurPos, egs.SelStart, egs.SelEnd);
         };

         // Making NewString
         memcpy(NewString,egs.StringText,egs.StringLength);
         NewString[egs.StringLength]=0;
         ect.Text=&NewString[egs.SelStart];
         ect.TextLength=egs.SelEnd-egs.SelStart;
         // Convert to OEM
         Info.EditorControl(ECTL_EDITORTOOEM,&ect);

         // If Conversion Type is unknown or Cyclic
         if(CCType==CCCyclic)
             // Define Conversion Type
             CCType=GetNextCCType(NewString, egs.StringLength, egs.SelStart, egs.SelEnd);

         // NewString contains no words
         if(CCType!=CCCyclic)
         {
             // Do the conversion
             ChangeCase(NewString, egs.SelStart, egs.SelEnd, CCType);

             // Back to editor charset
             Info.EditorControl(ECTL_OEMTOEDITOR,&ect);

             // Put converted string to editor
             struct EditorSetString ess;
             ess.StringNumber=egs.StringNumber;
             ess.StringText=NewString;
             ess.StringEOL=(char*)egs.StringEOL;
             ess.StringLength=egs.StringLength;
             Info.EditorControl(ECTL_SETSTRING,&ess);
         };

         #if 0
         if (!IsBlock)
         {
           struct EditorSelect esel;
           esel.BlockType=BTYPE_STREAM;
           esel.BlockStartLine=-1;
           esel.BlockStartPos=egs.SelStart;
           esel.BlockWidth=egs.SelEnd-egs.SelStart;
           esel.BlockHeight=1;
           Info.EditorControl(ECTL_SELECT,&esel);
         }
         #endif
         // Free memory
         free(NewString);

         // Exit if nothing was selected (single word was converted)
         if(!IsBlock)
             break;
       }
  }; // switch

  return(INVALID_HANDLE_VALUE);
}

void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  if(!IsOldFAR)
  {
      Info->StructSize=sizeof(*Info);
      Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
      static const char *PluginMenuStrings[1];
      // Text in Plugins menu
      PluginMenuStrings[0]=GetMsg(MCaseConversion);
      Info->PluginMenuStrings=PluginMenuStrings;
      Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  };
};

const char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

// What we consider as letter
BOOL MyIsAlpha(int c)
{
    return ( memchr(WordDiv, c, WordDivLen)==NULL ? TRUE : FALSE );
}

// Finding word bounds (what'll be converted) (Str is in OEM)
BOOL FindBounds(char *Str, int Len, int Pos, int &Start, int &End)
{
    int i=1;
    BOOL ret = FALSE;

    // If line isn't empty
    if( Len>Start )
    {
        End=min(End,Len);

        // Pos between [Start, End] ?
        Pos=max(Pos,Start);
        Pos=min(End,Pos);

        // If current character is non letter
        if(!MyIsAlpha(Str[Pos]))
        {
            // Looking for letter on the left and counting radius
            while((Start<=Pos-i) && (!MyIsAlpha(Str[Pos-i])))
                i++;

            // Radius
            int r=MAXINT;

            // Letter was found on the left
            if(Start<=Pos-i)
                r=i; // Storing radius

            i=1;
            // Looking for letter on the right and counting radius
            while((Pos+i<=End) && (!MyIsAlpha(Str[Pos+i])))
                i++;

            // Letter was not found
            if(Pos+i>End)
                i=MAXINT;

            // Here r is left radius and i is right radius

            // If no letters was found
            if( min(r,i)!=MAXINT )
            {
                // What radius is less? Left?
                if( r <= i )
                {
                    End=Pos-r+1;
                    Start=FindStart(Str, Start, End);
                }
                else // Right!
                {
                    Start=Pos+i;
                    End=FindEnd(Str, Start, End);
                };
                ret=TRUE;
            };
        }
        else // Current character is letter!
        {
            Start=FindStart(Str, Start, Pos);
            End=FindEnd(Str, Pos, End);
            ret=TRUE;
        };
    };

    if(!ret)
        Start=End=-1;

    return ret;
};

int FindStart(char *Str, int Start, int End)
{
    // Current pos in Str
    int CurPos=End-1;

    // While current character is letter
    while( CurPos>=Start && MyIsAlpha(Str[CurPos]) )
        CurPos--; // Moving to left

    return CurPos+1;
};

int FindEnd(char *Str, int Start, int End)
{
    // Current pos in Str
    int CurPos=Start;

    // While current character is letter
    while( CurPos<End && MyIsAlpha(Str[CurPos]))
        CurPos++; // Moving to right

    return CurPos;
};

// Changes Case of NewString from position Start till End
// to CCType and returns amount of changes
int ChangeCase(char *NewString, int Start, int End, int CCType)
{
    // If previous symbol is letter, then IsPrevSymbAlpha!=0
    BOOL IsPrevSymbAlpha=FALSE;
    // Amount of changes
    int ChangeCount=0;

    // Main loop (position inside line)
    for(int i=Start; i<End; i++)
    {
      if (MyIsAlpha(NewString[i]))// && ReverseOem==NewString[i])
      {
        switch(CCType)
        {
          case CCLower:
              NewString[i]=(char)FSF.LLower(NewString[i]);
              break;

          case CCTitle:
              if(IsPrevSymbAlpha)
                  NewString[i]=(char)FSF.LLower(NewString[i]);
              else
                  NewString[i]=(char)FSF.LUpper(NewString[i]);
              break;

          case CCUpper:
              NewString[i]=(char)FSF.LUpper(NewString[i]);
              break;

          case CCToggle:
              if(FSF.LIsLower(NewString[i]))
                  NewString[i]=(char)FSF.LUpper(NewString[i]);
              else
                  NewString[i]=(char)FSF.LLower(NewString[i]);
              break;

        };
        // Put converted letter back to string
        IsPrevSymbAlpha=TRUE;
        ChangeCount++;
      }
      else
          IsPrevSymbAlpha=FALSE;
    };

    return ChangeCount;
};

// Return CCType by rule: lower->Title->UPPER
// If Str contains no letters, then return CCCyclic
int GetNextCCType(char *Str, int StrLen, int Start, int End)
{
    int SignalWordStart=Start,
        SignalWordEnd=End;
    int SignalWordLen=max(Start,End);
    // Default conversion is to lower case
    int CCType=CCLower;

    Start=min(Start,End);
    End=SignalWordLen;

    if(StrLen<Start)
        return CCCyclic;

    // Looking for SignalWord (the 1-st word)
    if(!FindBounds(Str, StrLen,
                   SignalWordStart, SignalWordStart, SignalWordEnd))
        return CCCyclic;

    SignalWordLen=SignalWordEnd-SignalWordStart;

    char *SignalWord=(char *)malloc(SignalWordLen+1);

    if( SignalWord != NULL )
    {
        char *WrappedWord=(char *)malloc(SignalWordLen+1);

        if( WrappedWord != NULL )
        {

            FSF.sprintf(SignalWord, "%.*s", SignalWordLen, &Str[SignalWordStart]);
            FSF.sprintf(WrappedWord, "%s", SignalWord);

            // if UPPER then Title
            FSF.LUpperBuf(WrappedWord, SignalWordLen);

            if (SignalWordLen == 1 && lstrcmp(SignalWord, WrappedWord)==0)
              CCType=CCLower;
            else
            {
              if (SignalWordLen == 1)
                CCType=CCUpper;
              else
              {
                if(lstrcmp(SignalWord, WrappedWord)==0)
                    CCType=CCTitle;
                else
                {
                    // if lower then UPPER
                    FSF.LLowerBuf(WrappedWord, SignalWordLen);
                    if(lstrcmp(SignalWord,WrappedWord)==0)
                        CCType=CCUpper;
                    else
                    {
                        // if Title then lower
                        WrappedWord[0]=FSF.LUpper(WrappedWord[0]);
                        if(lstrcmp(SignalWord,WrappedWord)==0)
                            CCType=CCLower;
                        else
                        {
                            // if upper case letters amount more than lower case letters
                            // then tOGGLE
                            FSF.LUpperBuf(WrappedWord, SignalWordLen);
                            int Counter=SignalWordLen/2+1;
                            for(int i=0; i<SignalWordLen && Counter; i++)
                                if(SignalWord[i]==WrappedWord[i])
                                    Counter--;
                            if(!Counter)
                                CCType=CCToggle;
                        }
                    };
                };
              }
            }
            free(WrappedWord);
        };
        free(SignalWord);
    };

    return CCType;
};
