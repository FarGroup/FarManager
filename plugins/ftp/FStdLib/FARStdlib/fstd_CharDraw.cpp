#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

#define MAX_STR_SIZE 1000

static char *StrBuff = NULL;
static AbortProc oexit;

static void _cdecl idAtExit(void)
{
	delete[] StrBuff;

	if(oexit) oexit();
}

static void InitStrBuff(void)
{
	if(StrBuff) return;

	StrBuff = new char[ MAX_STR_SIZE+1 ];
	oexit = AtExit(idAtExit);
}
//------------------------------------------------------------------------
HCharDrawing::HCharDrawing(void)
{
	InitStrBuff();
	Chars = NULL;
	SizeX = SizeY = 0;
}

HCharDrawing::HCharDrawing(CHAR_INFO *ch, int w, int h)
{
	InitStrBuff();
	Assign(ch, w, h);
}

HCharDrawing::HCharDrawing(CHAR_INFO *ch, const RECT& r)
{
	Chars   = ch;
	SizeX   = r.right  - r.left;
	SizeY   = r.bottom - r.top;
	InitStrBuff();
	Assign(ch, r);
}

void HCharDrawing::Assign(CHAR_INFO *ch, int w, int h)
{
	Chars   = ch;
	SizeX   = w;
	SizeY   = h;
}

void HCharDrawing::Assign(CHAR_INFO *ch, const RECT& r)
{
	Chars   = ch;
	SizeX   = r.right  - r.left;
	SizeY   = r.bottom - r.top;
}

bool HCharDrawing::Char(int x, int y, char ch, int color)
{
	CHAR_INFO *v = GetChars(x,y);

	if(v)
	{
		SetChar(v,ch,color);
		return true;
	}
	else
		return false;
}

void HCharDrawing::VLine(int x, int y, int h, char ch, int color)
{
	CHAR_INFO *v = GetChars(x,y);

	if(!v) return;

	h = Min(MaxH()-y, h);

	if(color == -1)
		for(; h >= 0; v += SizeX+1,h--) SetCharC(v, ch);
	else
		for(; h >= 0; v += SizeX+1,h--) SetChar(v, ch, color);
}

void HCharDrawing::HLine(int x, int y, int w, char ch, int color)
{
	CHAR_INFO *v = GetChars(x,y);

	if(!v) return;

	w = Min(MaxW()-x, w);

	if(color == -1)
		for(; w >= 0; v++,w--) SetCharC(v, ch);
	else
		for(; w >= 0; v++,w--) SetChar(v, ch, color);
}

void HCharDrawing::FillRect(int x, int y, int x1, int y1, char ch, int color)
{
	CHAR_INFO *v = GetChars(x,y);

	if(!v) return;

	y1 = Min(MaxH(), y1);
	x1 = Min(MaxW(), x1);
	int num = SizeX-(x1-x);

	if(color == -1)
		for(; y <= y1; y++,v += num)
			for(int n = x; n <= x1; n++, v++)
				SetCharC(v, ch);
	else
		for(; y <= y1; y++,v += num)
			for(int n = x; n <= x1; n++, v++)
				SetChar(v, ch, color);
}

void HCharDrawing::TypeText(int x, int y, int color)
{
	CHAR_INFO *v = GetChars(x,y);

	if(!v) return;

	LPCSTR m = StrBuff;

	for(int pos = x; *m; m++)
	{
		if(*m == '\n')
		{
			pos = x;
			y++;
			v = GetChars(x,y);

			if(!v) return;

			continue;
		}

		if(pos > MaxW())
		{
			while(m[1] && m[1] != '\n') m++;

			continue;
		}

		SetChar(v++, *m, color);
		pos++;
	}
}

int HCharDrawing::Text(int x, int y, int color, LPCSTR fmt,...)
{
	va_list argptr;
	int     rc;
	va_start(argptr, fmt);
	rc = TextV(x,y,color,fmt,argptr);
	va_end(argptr);
	return rc;
}

int HCharDrawing::TextV(int x, int y, int color, LPCSTR fmt,va_list argptr)
{
	int rc;

	if((rc=VSNprintf(StrBuff, MAX_STR_SIZE, fmt, argptr)) >= MAX_STR_SIZE)
		StrBuff[rc = MAX_STR_SIZE] = 0;

	TypeText(x, y, color);
	return rc;
}

void HCharDrawing::Box(int x,int y,int x1,int y1,char fill,int color,DWORD type /*BOX_xxx*/)
{
	static LPCSTR boxes[] =
	{
		"      ",                    //"      " - None
		"\xC4\xB3\xDA\xBF\xC0\xD9",  //"ĳڿ��" - Signle
		"\xCD\xBA\xC9\xBB\xC8\xBC",  //"ͺɻȼ" - Double
		"\xCD\xB3\xD5\xB8\xD4\xBE",  //"ͳոԾ" - DbTop
		"\xC4\xBA\xD6\xB7\xD3\xBD"   //"ĺַӽ" - DbLeft
	};
	int w = x1-x,
	    h = y1-y;

	if(fill)
		FillRect(x+1,y+1,x1-1,y1-1, fill, color);

#define GTP  (type&0xFFFF)
	HLine(x+1,y,w-2,boxes[type][0],color);
	HLine(x+1,y+h-1,w-2,boxes[type][0],color);
	VLine(x,y+1,h-2,boxes[type][1],color);
	VLine(x+w-1,y+1,h-2,boxes[type][1],color);
	Char(x,y,boxes[type][2],color);
	Char(x+w-1,y,boxes[type][3],color);
	Char(x,y+h-1,boxes[type][4],color);
	Char(x+w-1,y+h-1,boxes[type][5],color);
#undef GTP
}

//------------------------------------------------------------------------
HCharArea::HCharArea(void)
{
}
HCharArea::~HCharArea()
{
	delete Chars; Chars = NULL;
}

void HCharArea::Setup(const RECT& r)
{
	Setup(r.right-r.left+1, r.bottom-r.top+1);
}

void HCharArea::Setup(int w, int h)
{
	if(!Chars || SizeX < w || SizeY < h)
	{
		delete[] Chars;
		Chars = new CHAR_INFO[(w+1)*(h+1)];
		memset(Chars, 0, sizeof(CHAR_INFO)*(w+1)*(h+1));
	}

	SizeX = w;
	SizeY = h;
}

void HCharArea::ScreenPut(int x,int y)
{
	static HANDLE conOut = NULL;

	if(!conOut)
		conOut = CreateFileA("CONOUT$", GENERIC_READ|GENERIC_WRITE,
		                     FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

	if(!conOut)
		return;

	COORD sz, pos;
	SMALL_RECT dst;
	sz.X = SizeX;
	sz.Y = SizeY;
	pos.X = 0;
	pos.Y = 0;
	dst.Left = x; dst.Top = y;
	dst.Right = x+SizeX; dst.Bottom = y+SizeY;
	WriteConsoleOutput(conOut, Chars, sz, pos, &dst);
}

void HCharArea::FarPut(int x,int y)
{
	char str[ 2 ] = " ";

	for(int n = 0; n < SizeX; n++)
		for(int i = 0; i < SizeY; i++)
		{
			CHAR_INFO *ch = GetChars(n,i);
			str[0] = ch->Char.AsciiChar;
			FP_Info->Text(x+n, y+i, ch->Attributes, str);
		}

	FP_Info->Text(0, 0, 0, NULL);
}
