#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

//#if defined(DM_FIRST)
//------------------------------------------------------------------------
FP_Multiline::FP_Multiline(int *itms,int maxLines)
	: dlg(NULL)
{
	for(ItemsCount = 0; Items[ItemsCount] != -1; ItemsCount++);

	Items = new int[ ItemsCount ];
	memcpy(Items,itms,sizeof(int)*ItemsCount);
	Init(maxLines);
}
FP_Multiline::FP_Multiline(int from,int to,int maxLines)
	: dlg(NULL)
{
	ItemsCount = to-from+1;
	Items = new int[ ItemsCount ];

	for(int n = 0; n < ItemsCount; n++)
		Items[n] = from+n;

	Init(maxLines);
}
FP_Multiline::~FP_Multiline()
{
	delete[] Items; Items = NULL;
	_Del(Selections); Selections = NULL;
	ClearLines();
	delete[] Lines;
}

void FP_Multiline::Init(int maxLines)
{
	CurrentPos  = 0;
	Editor      = NULL;
	MaxCount    = maxLines;
	Lines       = new char*[ MaxCount+1 ];
	top         = 0;
	Count       = 1;
	Lines[0]    = strdup("");
	CurrentLine = 0;
	dlgVisible  = false;
	SelCount   = 1;
	Selections = (FP_Hilight*)_Alloc(sizeof(FP_Hilight));
	Selections[0].Rect.Empty();
	Selections[0].Color = (int)FP_Info->AdvControl(FP_Info->ModuleNumber,ACTL_GETCOLOR,(void*)COL_DIALOGEDITSELECTED);
}

int FP_Multiline::AddSelection(void)
{
	Selections = (FP_Hilight*)_Realloc(Selections,sizeof(FP_Hilight)*(SelCount+1));
	Assert(Selections);
	Selections[ SelCount ].Rect.Empty();
	Selections[ SelCount ].Color = 7;
	return SelCount++;
}
FP_Hilight* FP_Multiline::GetSelection(int num)
{
	return (((unsigned)num) < ((unsigned)SelCount)) ? (Selections+num) : NULL;
}

void FP_Multiline::DeleteSelection(int num)
{
	if(num > 0 && num < SelCount)
	{
		memmove(Selections+num,Selections+num+1,(SelCount-num)*sizeof(FP_Hilight));
		SelCount--;
	}
}

/*  GetSelectionText

    Gets text of selection or calculate it length.

    Params:
      num   - number of selection.
              -1 - all text
              0  - currently selected text
              X  - user defined selections
      buff  - buffet to place text to
              NULL do not store text, calculate length only
      sz    - maximum size of buffer
              used only if buff != NULL
*/
int FP_Multiline::GetSelectionText(int num /*=-1*/,char *buff/*=NULL*/,int sz/*=0*/)
{
	if(buff) *buff = 0;

	if(num != -1 && ((unsigned)num) >= ((unsigned)SelCount))
		return 0;

	SRect r;

	if(num == -1)
		r.Set(0,0,0,Count);
	else
		r = Selections[num].Rect;

	return GetSelectionText(r,buff,sz);
}

int FP_Multiline::GetSelectionText(const SRect& _r,char *buff,int sz)
{
	SRect r;
	int   n,len,x,x1,
	  cn = 0;
	char *m;

	if(buff)
	{
		*buff = 0;

		if(!sz) return 0;
	}

	r = _r;
	r.Left   = Max(0,(int)r.Left);
	r.Right  = Max(0,(int)r.Right);
	r.Top    = Max(0,Min((int)r.Top,Count));
	r.Bottom = Max(0,Min((int)r.Bottom,Count));

	if(r.isEmpty())
		return 0;

	for(n = r.Y(); n <= r.Y1(); n++)
	{
		if(buff && sz <= 0) break;

		if(n < 0) continue;

		if(n >= Count) break;

		m = Lines[ n ];
		len = strLen(m);
		x  = Min(len,(n == r.Y())  ? r.X() : 0);
		x1 = Min(len,(n == r.Y1()) ? r.X1() : len);

		if(buff)
		{
			len   = Min(sz-1,x1-x);
			memcpy(buff,m+x,len);
			buff += len;
			sz   -= len;
		}
		else
			len = x1-x;

		cn += len;

		if(n != r.Y1())
		{
			if(buff && sz > 1)
			{
				*buff++ = '\n';
				sz--;
			}

			cn++;
		}
	}

	if(buff) *buff = 0;

	return cn;
}

void FP_Multiline::DeleteSelectionText(int num)
{
	if(((unsigned)num) >= ((unsigned)SelCount))
		return;

	SRect r = Selections[num].Rect;
	int   n,len,
	  x,x1,y = -1,y1 = 0;
	char *m;

	if(r.isNull())
		return;

	r.Top    = Max(0,Min(Count-1,(int)r.Top));
	r.Bottom = Max(0,Min(Count-1,(int)r.Bottom));
	r.Left   = Max(0,(int)r.Left);
	r.Right  = Max(0,(int)r.Right);

	for(n = r.Y(); n <= r.Y1(); n++)
	{
		m = Lines[ n ];

		if(n == r.Y() || n == r.Y1())
		{
			len = strLen(m);

			if(len)
			{
				x   = Min(len,(n == r.Y())  ? r.X() : 0);
				x1  = Min(len,(n == r.Y1()) ? r.X1() : len);

				if(x != x1)
					memmove(m+x,m+x1,len-x1);

				len = len - (x1-x);
				m[len] = 0;
			}

			if(n == r.Y1() && r.Height())
			{
				x = r.Y();
				char *tmp = (char*)_Alloc(strlen(Lines[x]) + len + 1);
				strcpy(tmp,Lines[x]);
				strcat(tmp,m);
				_Del(Lines[x]);
				_Del(m);
				Lines[x] = tmp;

				if(y == -1) y = n;

				y1 = n;
			}
		}
		else
		{
			if(y == -1) y = n;

			y1 = n;
			_Del(m);
		}
	}

	Selections[num].Rect.Empty();

	if(y != -1)
	{
		memmove(Lines+y,Lines+y1+1,sizeof(Lines[0])*(Count-y1-1));
		Count -= y1-y+1;
	}

	GoTo(r.Y(),r.X(),true);
	N_Changed();
}

void FP_Multiline::PasteSelection(int num)
{
	LPVOID      data;
	SIZE_T      sz;
	FP_Hilight* p;

	if(((unsigned)num) >= ((unsigned)SelCount) ||
	        !FP_GetFromClipboard(data,sz) ||
	        !data || !sz || *((char*)data) == 0)
		return;

	char **tLines = new char*[MaxCount+1];
	int    tCount,n,pos;
	char  *tmp,
	   *last;
	p   = Selections + num;
	pos = top+CurrentLine;
	p->Rect.Set(Min((int)strlen(Lines[pos]),CurrentPos),pos);
	ParseText((char*)data,tLines,tCount,MaxCount-Count);
	tCount = min(tCount,MaxCount-Count);

	if(tCount)
	{
		pos = p->Rect.Y();

		if(tCount > 1)
			memmove(Lines+pos+tCount, Lines+pos+1, sizeof(Lines[0])*(Count-1-pos));

		last = strdup(Lines[pos] + p->Rect.X());

		if(tCount > 1)
			p->Rect.Right = (SHORT)strlen(tLines[tCount-1]);
		else
			p->Rect.Right += (SHORT)strlen(tLines[tCount-1]);

		tmp  = (char*)_Alloc(p->Rect.X() + strlen(tLines[0]) + 1);
		strncpy(tmp,Lines[pos],p->Rect.X());
		strcpy(tmp+p->Rect.X(),tLines[0]);
		_Del(Lines[pos]);
		_Del(tLines[0]);
		Lines[pos] = tmp;

		for(pos++,n = 1; n < tCount; pos++,n++)
		{
			Lines[ pos ] = tLines[n];
			p->Rect.Bottom++;
			Count++;
		}

		pos--;

		if(*last)
		{
			tmp  = (char*)_Alloc(strlen(last) + strlen(Lines[pos]) + 1);
			strcpy(tmp,Lines[pos]);
			strcat(tmp,last);
			_Del(Lines[pos]);
			Lines[pos] = tmp;
		}

		_Del(last);
	}

	delete[] tLines;
	_Del(data);

	if(!tCount || p->Rect.isEmpty())
		return;

	GoTo(p->Rect.Bottom,p->Rect.Right,true);
	N_Changed();
}

void FP_Multiline::DoDrawHilight(const FP_Hilight& sel)
{
	SRect it;

	if(!dlgVisible || sel.Rect.isEmpty())
		return;

	int   y,x,x1,n,num,len,i;
	char *m;
	char  str[500];

	for(n = 0; n < ItemsCount; n++)
	{
		if(n >= Count) return;

		num = top+n;

		if(num < sel.Rect.Top) continue;

		if(num > sel.Rect.Bottom) break;

		if(!dlg->ItemRect(Items[n],&it)) break;

		m   = Lines[num];
		len = Min(strLen(m),(int)sizeof(str)-1);
		y   = DlgBounds.Top + it.Top;
		x   = (num == sel.Rect.Top)    ? sel.Rect.Left  : 0;
		x1  = (num == sel.Rect.Bottom) ? sel.Rect.Right : (it.Right-it.Left);
		x  = Max(0,Min(x,(int)it.Right-it.Left+1));
		x1 = Max(0,Min(x1,(int)it.Right-it.Left+1));

		if(x1-x <= 0)
			continue;

		for(x1 -= x, i = 0; i < (int)(sizeof(str)-1) && i < x1; i++)
			str[i] = (i+x < len) ? m[i+x] : ' ';

		str[i] = 0;
		FP_Info->Text(DlgBounds.Left + it.Left + x,y,
		              sel.Color,
		              str);
		//FP_Info->Text( 0,0,0,NULL );
	}
}
void FP_Multiline::DoDrawHilights(void)
{
	for(int n = 1; n < SelCount; n++)
		DoDrawHilight(Selections[n]);

	DoDrawHilight(Selections[0]);
}

void FP_Multiline::ClearLines(void)
{
	for(int n = 0; n < Count; n++)
		_Del(Lines[n]);

	top   = 0;
	Count = 0;
	CurrentPos  = 0;
	CurrentLine = 0;
}

void FP_Multiline::GetLine(int num)
{
	if(((unsigned)num) >= ((unsigned)ItemsCount) ||
	        ((unsigned)(num+top)) >= ((unsigned)Count))
		return;

	_Del(Lines[ num+top ]);
	Lines[ num+top ] = strdup(dlg->GetText(Items[num]));
}

int FP_Multiline::FindEdit(int id)
{
	for(int n = 0; n < ItemsCount; n++)
		if(Items[n] == id)
			return n;

	return -1;
}

int FP_Multiline::CurLine(void)
{
	return top + CurrentLine;
}

void FP_Multiline::SetChanged(bool ch)
{
	Changed = ch;

	if(ch)
		dlg->User(DN_TEXTCHHANGED,(BOOL)ch,(LONG_PTR)this);
}

int FP_Multiline::TextLength(void)
{
	return GetSelectionText(-1,NULL,0);
}

int FP_Multiline::GetText(char *buff,int sz)
{
	return GetSelectionText(-1,buff,sz);
}

void FP_Multiline::SetItems(void)
{
	int n,num;

	if(!Count)
	{
		Lines[0] = strdup("");
		Count    = 1;
	}

	dlg->Lock();

	for(n = 0; n < Count; n++)
	{
		if(n >= ItemsCount) break;

		num = Items[n];
		dlg->SetText(num,Lines[top+n]);
		dlg->Visible(num,TRUE);
	}

	for(; n < ItemsCount; n++)
	{
		num = Items[n];
		dlg->SetText(num,"");
		dlg->Visible(num,FALSE);
	}

	DoDrawHilights();
}

void FP_Multiline::ParseText(char *m,char **tLines,int& tCount,int tMaxCount)
{
	char  ch1 = 0;
	char *str;
	tCount = 0;

	if(!m || !m[0])
		return;

	for(str = m; tMaxCount && *m; m++)
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
				char tmp = *m;
				*m = 0;
				tLines[tCount++] = strdup(str);
				tMaxCount--;
				*m = tmp;
			}

			if(m[1] == ch1) m++;

			str = m+1;
		}
	}

	if((str != m || ch1) && tCount < tMaxCount)
		tLines[tCount++] = strdup(str);
}

void FP_Multiline::SetText(char *m)
{
	ClearLines();
	ParseText(m,Lines,Count,MaxCount);
	SetItems();
	N_Changed();
}

void FP_Multiline::SetTextFromCipboard(void)
{
	LPVOID data;
	SIZE_T  sz;
	ClearLines();

	if(FP_GetFromClipboard(data,sz))
	{
		if(data && *(char*)data != 0)
			SetText((char*)data);

		_Del(data);
	}
}

void FP_Multiline::TextToCipboard(void)
{
	int   len  = TextLength();
	char *data = new char[ len+2 ];
	GetText(data,len);
	FP_CopyToClipboard(data,len);
	delete[] data;
}

void FP_Multiline::SetTextFromEditor(int fromLine)
{
	EditorGetString egs;

	if(!Editor->Strings(fromLine,&egs))
		return;

	ClearLines();

	do
	{
		Lines[Count++] = strdup(egs.StringText);
	}
	while(Count < MaxCount && Editor->Strings(++fromLine,&egs));

	SetItems();
}

bool FP_Multiline::GoTo(int y,int x,bool ForceUpdate)
{
	int oldy = CurrentLine + top,
	    nTop = top;
	y = Max(0,Min(y,Count-1));

	if(oldy == y && x == -1)
		return false;
	if(y < top)                 nTop = y; else if(y-nTop >= ItemsCount)    nTop = Max(0,y-ItemsCount+1);

	if(nTop+ItemsCount > Count) nTop = Max(0,y-ItemsCount+1);

	if(y == Count-1)            nTop = Max(0,y-ItemsCount+1);

	if(nTop != top || ForceUpdate)
	{
		if(nTop != -1) top = nTop;

		SetItems();
	}

	if(x != -1)
	{
		if(dlg->CursorPos(Items[y-top],x))
			CurrentPos = x;
	}

	CurrentLine = y-top;
	dlg->Focused(Items[y-top]);
	N_PosChanged();
	return true;
}

void FP_Multiline::NewLine(int iNum,int CharPos)
{
	int   num,n;

	if(Count >= MaxCount)
		return;

	num = top+iNum;

	for(n = Count; n > num; n--)
		Lines[n] = Lines[n-1];

	Count++;

	if(CharPos >= 0 && CharPos < strLen(Lines[num]))
	{
		Lines[num+1]        = strdup(Lines[num] + CharPos);
		Lines[num][CharPos] = 0;
	}
	else
		Lines[num+1] = strdup("");

	GoTo(num+1,0,true);
	N_Changed();
}

void FP_Multiline::DelLine(int iNum)
{
	int n;

	if(((unsigned)iNum) >= ((unsigned)Count))
		return;

	if(Count == 1)
	{
		if(Lines[0][0])
		{
			Lines[0][0] = 0;
			GoTo(0,0,true);
			N_Changed();
		}

		return;
	}

	_Del(Lines[ iNum ]);
	Count--;

	for(n = iNum; n < Count; n++)
		Lines[n] = Lines[n+1];

	GoTo(iNum,CurrentPos,true);
	N_Changed();
}

void FP_Multiline::AddLineTail(int nLine)
{
	if(((unsigned)nLine) >= ((unsigned)Count))
		return;

	char *m = Lines[ nLine ],
	      *tmp;
	int   len = (int)strlen(m);

	if(CurrentPos <= len)
		return;

	tmp = new char[ CurrentPos+1 ];
	strcpy(tmp,m);

	for(m = tmp+len; len < CurrentPos; len++)
		*m++ = ' ';

	*m = 0;
	dlg->SetText(Items[nLine-top],tmp);
	delete[] tmp;
}

bool FP_Multiline::DoArrows(int id,int num,long key)
{
	int x,y,
	    ox = CurrentPos,
	    oy = top+CurrentLine;

	switch(key)
	{
		case             KEY_UP: y = oy-1; x = ox;                break;
		case           KEY_DOWN: y = oy+1; x = ox;                break;
		case           KEY_LEFT: y = oy;   x = ox-1;              break;
		case          KEY_RIGHT: y = oy;   x = ox+1;              break;
		case           KEY_HOME: y = oy;   x = 0;                 break;
		case            KEY_END: y = oy;   x = strLen(Lines[oy]); break;
		case           KEY_PGUP: y = oy-ItemsCount; x = ox;       break;
		case           KEY_PGDN: y = oy+ItemsCount; x = ox;       break;
		case  KEY_CTRL+KEY_PGUP: y = -1; x = ox;                  break;
		case  KEY_CTRL+KEY_HOME: y = -1; x = 0;                   break;
		case   KEY_CTRL+KEY_END:
		case  KEY_CTRL+KEY_PGDN: y = Count; x = ox;               break;
		default: return false;
	}

	if(!Selections[0].Rect.isEmpty() && y == oy && x != ox)
		dlg->Lock();

	Selections[0].Rect.Empty();
	GoTo(y,x);
	return true;
}

bool FP_Multiline::DoSelection(int id,int num,long key)
{
	SRect* sel = &Selections[0].Rect;
	int    x,y,x1,y1;
	char  *m;
	enum SPos
	{
		SStart,
		SEnd,
		SNone
	};
	SPos pos;
	x1  = x = CurrentPos;
	y1  = y = top+num;
	if(sel->X()  == x && sel->Y()  == y) pos = SStart; else if(sel->X1() == x && sel->Y1() == y) pos = SEnd; else

		pos = SNone;

	switch(key)
	{
		case  KEY_SHIFT+KEY_CTRL+KEY_LEFT: WordLeft(x1,y1); break;
		case KEY_SHIFT+KEY_CTRL+KEY_RIGHT: WordRight(x1,y1); break;
		case  KEY_SHIFT+KEY_LEFT: x1--;                        break;
		case KEY_SHIFT+KEY_RIGHT: x1++;                        break;
		case  KEY_SHIFT+KEY_HOME: x1 = 0;                      break;
		case   KEY_SHIFT+KEY_END: x1 = strLen(Lines[top+num]); break;
		case  KEY_CTRL+KEY_SHIFT+KEY_PGUP: y1 = 0;               break;
		case  KEY_CTRL+KEY_SHIFT+KEY_HOME: y1 = 0;       x1 = 0; break;
		case  KEY_CTRL+KEY_SHIFT+KEY_PGDN: y1 = Count-1;         break;
		case   KEY_CTRL+KEY_SHIFT+KEY_END: y1 = Count-1; x1 = (int)strlen(Lines[Count-1]); break;
		case    KEY_SHIFT+KEY_UP: y1--; break;
		case  KEY_SHIFT+KEY_DOWN: y1++; break;
		case  KEY_SHIFT+KEY_PGUP: y1 -= ItemsCount; break;
		case  KEY_SHIFT+KEY_PGDN: y1 += ItemsCount; break;
		case           KEY_CTRLC:

		case    KEY_CTRL+KEY_INS: if(!sel->isEmpty())
			{
				x = GetSelectionText(0,NULL,0);

				if(x)
				{
					m = new char[ x+1 ];
					GetSelectionText(0,m,x+1);
					FP_CopyToClipboard(m,x);
					delete[] m;
				}
			}

			return true;
		case           KEY_CTRLX:

		case   KEY_SHIFT+KEY_DEL: if(!sel->isEmpty())
			{
				x = GetSelectionText(0,NULL,0);

				if(x)
				{
					m = new char[ x+1 ];
					GetSelectionText(0,m,x+1);
					FP_CopyToClipboard(m,x);
					delete[] m;
					DeleteSelectionText(0);
				}
			}

			return false;
		case           KEY_CTRLV:
		case   KEY_SHIFT+KEY_INS: PasteSelection(0);
			return true;
		case              KEY_BS:
		case             KEY_DEL:

		case           KEY_CTRLD: if(!sel->isEmpty())
			{
				DeleteSelectionText(0);
				return true;
			}
			else
				return key == KEY_CTRLD;

		default: return false;
	}

	dlg->Lock();
	y1 = Max(0,Min(y1,Count-1));
	x1 = Max(0,x1);

	if(x != x1 || y != y1)
	{
		if(pos == SNone)
		{
			sel->Left   = x;
			sel->Top    = y;
			sel->Right  = x1;
			sel->Bottom = y1;
		}
		else if(pos == SStart)
		{
			sel->Left = x1;
			sel->Top  = y1;
		}
		else if(pos == SEnd)
		{
			sel->Right  = x1;
			sel->Bottom = y1;
		}

		sel->Normalize();
		GoTo(y1,x1);
	}

	return true;
}

bool FP_Multiline::DoEdit(int id,int num,long key)
{
	bool delP;

	switch(key)
	{
		case          KEY_CTRLY: DelLine(top+num);        return true;
		case          KEY_CTRLN:
		case          KEY_ENTER: NewLine(num,CurrentPos); return true;
		case            KEY_DEL: delP = true;               break;
		case             KEY_BS: delP = false;              break;
		default: return false;
	}

	char *m = Lines[ top+num ];
	int   len = (int)strlen(m);

	if(delP && (CurrentPos < len || CurLine() == Count-1) ||
	        !delP  && (CurrentPos > 0 || CurLine() == 0))
		return false;

	num = CurLine() - (delP == false);
	len = (int)strlen(Lines[num]);
	m   = (char*)_Alloc(len + strlen(Lines[num+1]) + 1);
	strcpy(m,Lines[num]);
	strcat(m,Lines[num+1]);
	_Del(Lines[num]); _Del(Lines[num+1]);
	Lines[num] = m;
	Count--;

	if(num < Count-1)
		memmove(Lines+num+1,Lines+num+2,sizeof(Lines[0])*(Count-num));

	GoTo(num,len,true);
	N_Changed();
	return true;
}

void FP_Multiline::WordLeft(int& x,int& y)
{
	x--;
}

void FP_Multiline::WordRight(int& x,int& y)
{
	x++;
}

bool IsChangeKey(long key)
{
	if((key&(KEY_CTRL|KEY_ALT|KEY_RCTRL|KEY_RALT)) != 0)
		return false;

	if((key&KEY_MASKF) >= 0x20 && (key&KEY_MASKF) < 0x100)
		return true;

	return false;
}
BOOL FP_Multiline::DlgProc(FP_Dialog* d, int Msg, int id, LONG_PTR Param2, long& rc)
{
	int  num;
	dlg = d;
#define DRET( v ) { rc = v; return TRUE; }

	switch(Msg)
	{
		case DN_INITDIALOG: Editor = (FP_Editor*)Param2;
			dlg->DlgRect(&DlgBounds);
			SetItems();
			SetChanged(false);
			break;
		case DN_DRAWDIALOG: dlgVisible = true;
			dlg->DlgRect(&DlgBounds);
			break;

		case DN_DRAWDLGITEM: if(id == Items[ItemsCount-1]+1)
			{
				DoDrawHilights();
			}

			break;
		case        DN_KEY: num = FindEdit(id);

			if(num == -1)
				break;

			CurrentPos  = dlg->CursorPos(id);
			CurrentLine = num;

			if(IsChangeKey((long)Param2))
			{
				DeleteSelectionText(0);
				Selections[0].Rect.Empty();
				AddLineTail(num+top);
			}

			if(DoArrows(id,num,(long)Param2) ||
			        DoSelection(id,num,(long)Param2) ||
			        DoEdit(id,num,(long)Param2))
			{
				DRET(TRUE);
			}

			break;
		case DN_EDITCHANGE: num = FindEdit(id);

			if(num != -1)
			{
				GetLine(num);
				CurrentPos = dlg->CursorPos(id);
				N_Changed();
			}

			break;
	}

	return FALSE;
#undef DRET
}
//#endif //DM_FIRST
