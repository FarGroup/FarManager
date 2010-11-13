#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

//- Editor HELPERS
int FP_EditorSelect::X(void)     const { return BlockStartPos; }
int FP_EditorSelect::Y(void)     const { return BlockStartLine; }
int FP_EditorSelect::X1(void)    const { return BlockStartPos+BlockWidth; }
int FP_EditorSelect::Y1(void)    const { return BlockStartLine+BlockHeight-(BlockType==BTYPE_COLUMN || BlockHeight>0); }

void FP_EditorSelect::Set(int x,int y,int x1,int y1,int tp)
{
	if(tp != -1) BlockType = tp;

	switch(BlockType)
	{
		case BTYPE_STREAM: if(y > y1) Swap(y,y1); else if(y == y1 && x > x1) Swap(x,x1);

			BlockStartLine = y;
			BlockStartPos  = x;
			BlockWidth     = x1 - x;
			BlockHeight    = y1 - y +1;
			break;

		case BTYPE_COLUMN: if(y1 < y) Swap(y,y1);

			if(x1 < x) Swap(x,x1);

			BlockStartLine = y;
			BlockStartPos  = x;
			BlockWidth     = x1 - x;
			BlockHeight    = y1 - y + 1;
			break;
	}
}
//- FP_Editor
FP_Editor::FP_Editor(void)
{
	Assigned = Fresh();
}

bool FP_Editor::Fresh(void)
{
	return FP_Info->EditorControl && FP_Info->EditorControl(ECTL_GETINFO, &EInfo);
}

bool FP_Editor::Strings(int num,EditorGetString* gs)
{
	EditorGetString dat;
	bool            res;

	if(!Assigned)
		return false;

	if(!gs) gs = &dat;

	memset(gs,0,sizeof(*gs));
	gs->StringNumber = num;
	res = FP_Info->EditorControl(ECTL_GETSTRING,gs) != 0;

	if(res && num == FE_CURSTRING)
		gs->StringNumber = EInfo.CurLine;

	return res;
}

int FP_Editor::CurX(void)  const { return EInfo.CurPos; }
int FP_Editor::CurY(void)  const { return EInfo.CurLine; }
int FP_Editor::CurSX(void) const { return EInfo.CurTabPos; }
int FP_Editor::CurSY(void) const { return EInfo.CurLine - EInfo.TopScreenLine; }

int FP_Editor::Cursor2String(int x,int y) const
{
	EditorConvertPos  ec = { y,x,0 };
	FP_Info->EditorControl(ECTL_TABTOREAL,&ec);
	return ec.DestPos;
}

int FP_Editor::String2Cursor(int x,int y) const
{
	EditorConvertPos  ec = { y,x,0 };
	FP_Info->EditorControl(ECTL_REALTOTAB,&ec);
	return ec.DestPos;
}

void FP_Editor::MoveTo(int x,int y)
{
	EditorSetPosition ep = { y,x,-1,-1,-1,-1 };

	if(!Assigned) return;

	FP_Info->EditorControl(ECTL_SETPOSITION,&ep);
	Fresh();
}

bool FP_Editor::Selection(EditorSelect* p)
{
	EditorGetString egs;

	if(!p || !Assigned)
		return false;

	memset(p,0,sizeof(*p));
	p->BlockType = EInfo.BlockType;

	if(p->BlockType == BTYPE_NONE)
		return true;

	memset(&egs,0,sizeof(egs));
	egs.StringNumber = EInfo.BlockStartLine;

	if(!FP_Info->EditorControl(ECTL_GETSTRING,&egs) || egs.SelStart == -1)
		return false;

	p->BlockStartLine = egs.StringNumber;
	p->BlockStartPos  = egs.SelStart;

	if(p->BlockType == BTYPE_STREAM)
	{
		while(egs.StringNumber < EInfo.TotalLines &&
		        egs.SelEnd == -1 &&
		        FP_Info->EditorControl(ECTL_GETSTRING,&egs))
			egs.StringNumber++;

		p->BlockWidth  = egs.SelEnd - p->BlockStartPos;
	}
	else
	{
		p->BlockWidth  = egs.SelEnd - egs.SelStart;

		while(egs.StringNumber < EInfo.TotalLines &&
		        FP_Info->EditorControl(ECTL_GETSTRING,&egs) &&
		        egs.SelStart != -1)
			egs.StringNumber++;
	}

	p->BlockHeight = egs.StringNumber - p->BlockStartLine;
	return true;
}

void FP_Editor::SetSelection(const EditorSelect& p)
{
	FP_Info->EditorControl(ECTL_SELECT,(EditorSelect*)&p);
	Fresh();
}

void FP_Editor::Send(INPUT_RECORD *p)
{
	FP_Info->EditorControl(ECTL_PROCESSINPUT,p);
	Fresh();
}

LPCSTR FP_Editor::GetString(int num,int *Size)
{
	EditorGetString gs;

	if(!Strings(num,&gs))
		return NULL;

	if(Size) *Size = gs.StringLength;

	return gs.StringText;
}

bool FP_Editor::SetString(LPCSTR Text,int num,int StringSize)
{
	EditorSetString gs =
	{
		num,
		(char*)((Text)?Text:""),
		NULL,
		(StringSize==-1) ? ((Text)?strLen(Text):0) : StringSize
		};

	if(!Assigned)
		return false;

	return FP_Info->EditorControl(ECTL_SETSTRING,&gs) != 0;
}

void FP_Editor::Write(char *m)
{
	char  ch,ch1;
	char *str;

	if(!m || !m[0])
		return;

	for(str = m; *m; m++)
	{
		switch(*m)
		{
			case '\n': ch1 = '\r'; break;
			case '\r': ch1 = '\n'; break;
			default: ch1 = 0;    break;
		}

		if(ch1)
		{
			if(str != m)
			{
				ch = *m;
				*m = 0;
				FP_Info->EditorControl(ECTL_INSERTTEXT,str);
				*m = ch;
			}

			if(m[1] == ch1) m++;

			FP_Info->EditorControl(ECTL_INSERTSTRING,NULL);
			str = m;
		}
	}

	if(str != m)
		FP_Info->EditorControl(ECTL_INSERTTEXT,str);

	Fresh();
}

void FP_Editor::Redraw(void)
{
	FP_Info->EditorControl(ECTL_REDRAW,EEREDRAW_ALL);
}
