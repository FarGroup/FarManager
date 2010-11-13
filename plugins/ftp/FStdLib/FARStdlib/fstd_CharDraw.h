#ifndef __FAR_CHAR_DRAWING
#define __FAR_CHAR_DRAWING

/* HCharDrawing
   class for drawing in CHAR_INFO rectangle

   Construct:
     HCharDrawing( CHAR_INFO *ch, int w, int h );
     HCharDrawing( CHAR_INFO *ch, const RECT& r );

    with:
     ch      - pointer to the two dimentional array of CHAR_INFO allocated for DI_USERCONTROL
     w,h | r - dimentions of control INCLUDING right and bottom line of control.

   Functions:
     MaxW     - returns maximum drawable horizontal coordinate;
     MaxH     - returns maximum drawable vertical coordinate;
     Char     - draws OPT colorized char at specified position;
     HLine,   - draws horizontal or vertical line using OPT colorized character;
     VLine
     FillRect - fills rectangular area INCLUDING right and bottom sides with
                given character and OPT color;
     Text,    - draws text string;
     TextV

   Notes:
     - Functions automatically clips all drawing to be inside specified control size;
     - Any `color` may be set to -1 to indicate DO NOT CHANGE color in poutput position;
     - TextX functions interpretes \n and|or \r characters as line feed and move
       draw position to the next line and to the horizontal position from thwere drawing
       starts;

   Sample:
     void Dialog::PaintElement( PMVPaintInfo p )
       {  HCharDrawing dc( p->Data.Chars, p->Rect );

          dc.FillRect( 0, 0, dc.MaxW(), dc.MaxH(), ' ', FAR_COLOR(fccWHITE,fccBLACK) );
          dc.Text( 1, 1, FAR_COLOR(fccWHITE,fccBLACK), " File: %s", StrToOEMDup(FileName.c_str()) );
          dc.Text( 1, 2, FAR_COLOR(fccWHITE,fccBLACK), "Error: %s", Error.c_str() );
     }
*/

#define BOX_None        0
#define BOX_Single      1
#define BOX_Double      2
#define BOX_DoubleTop   3
#define BOX_DoubleLeft  4

class HCharDrawing
{
	protected:
		CHAR_INFO *Chars;
		int        SizeX,
		  SizeY;

	protected:
		CHAR_INFO *GetChars(int x, int y)                    { return (x > MaxW() || y > MaxH()) ? NULL : (&Chars[(MaxW()+1)*y + x ]); }
		void       SetChar(CHAR_INFO *v, char ch, int color) { v->Char.AsciiChar = ch; if(color != -1) v->Attributes = color; }
		void       SetCharC(CHAR_INFO *v, char ch)           { v->Char.AsciiChar = ch; }
		void       TypeText(int x, int y, int color);

	public:
		HCharDrawing(void);
		HCharDrawing(CHAR_INFO *ch, int w, int h);
		HCharDrawing(CHAR_INFO *ch, const RECT& r);
		virtual ~HCharDrawing(void) {}

		void Assign(CHAR_INFO *ch, int w, int h);
		void Assign(CHAR_INFO *ch, const RECT& r);

		int  MaxW(void) { return SizeX; }
		int  MaxH(void) { return SizeY; }

		bool Char(int x, int y, char ch, int color = -1);
		void HLine(int x, int y, int w, char ch, int color = -1);
		void VLine(int x, int y, int h, char ch, int color = -1);
		void FillRect(int x, int y, int x1, int y1, char ch, int color = -1);
		int  Text(int x, int y, int color, LPCSTR fmt,...);
		int  TextV(int x, int y, int color, LPCSTR fmt,va_list argptr);
		void Box(int x,int y,int x1,int y1, char fill /*=0*/, int color, DWORD type /*BOX_xxx*/);
};

/* Descendant to draw into self-allocated buffer and
   posibilities to put resultin buffer to screen or FAR text buffer.
*/
class HCharArea: public HCharDrawing
{
	public:
		HCharArea(void);
		~HCharArea();

		void Setup(int w, int h);
		void Setup(const RECT& r);

		void ScreenPut(int x,int y);
		void FarPut(int x,int y);
};

#endif
