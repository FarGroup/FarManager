#include <all_far.h>
#pragma hdrstop

#include "Int.h"

extern LPCSTR StdCommands[];

LPCSTR StdDialogLines[] =
{
	"%*\xFA-61SrcPathname%",
	"%*\xFA-61DestPathname%",

	"\xC4" "Current %Cn%(%CurPc%)%>\xC4%",
	"   %+14CurSize%  %CurTime%      %CurCPS%    %CPS%",
	"   %+14CurFSize%  %CurRTime%         <%CurETime% %CurEDate%>",
	"%.Pc%-[%57Progress%]-",

	"\xC4" "Total %CnF%/%Cn%/%CnSkip%%>\xC4%",
	"   %+14TotSize%  %TotTime%                  %TotCps%",
	"   %+14TotFSize%  %TotRTime%         <%TotETime% %TotEDate%>",
	"%.FPercent%-[%57FProgress%]-",
	"-[%27CurPg%]-[%27SkipPg%]-"
};


void TrafficInformation::FormatLine(int num,LPCSTR line,time_t tm)
{
	char     str[50];
	int      n,cn;
	InfoItem it;

	if(num < 0 || num >= MAX_TRAF_LINES ||
	        !line || !line[0])
		return;

	for(n = 0; *line && n < MAX_TRAF_WIDTH; line++)
	{
		//Char
		if(*line != '%')
		{
			Lines[num][n++] = *line;
			continue;
		}

		//%
		if(line[1] == '%')
		{
			line++;
			Lines[num][n++] = *line;
			continue;
		}

		line++;

		//Code
		if(*line == '\\')
		{
			line++;

			for(cn = 0; *line && strchr("0123456789",*line) && cn < (int)(ARRAYSIZE(str)-1); cn++,line++)
				str[cn] = *line;

			str[cn] = 0;
			cn = atoi(str);

			if(!cn || *line != '%')
			{
				strcpy(Lines[num]+n,"<badCmd>");
				n += 8;
				break;
			}

			Lines[num][n++] = (char)cn;
			continue;
		}

		it.Type  = 0;
		it.Line  = num;
		it.Pos   = n;
		it.Align = tNone;
		it.Size  = 0;

		//Filler
		if(*line == '*')
		{
			line++;
			it.Fill = *line;
			line++;
		}
		else
			it.Fill  = ' ';

		//Align
		if(*line == '-') it.Align = tLeft; else if(*line == '+') it.Align = tRight; else if(*line == '.') it.Align = tCenter; else if(*line == '>')

		{
			//Right line
			line++;
			it.Fill = *line++;

			if(*line != '%')
			{
				strcpy(Lines[num]+n,"<badCmd>");
				n += 8;
				break;
			}

			if(Count >= MAX_TRAF_ITEMS)
			{
				strcpy(Lines[num]+n,"<many>");
				break;
			}

			it.Align = tRightFill;
			Items[Count++] = it;
			continue;
		}

		if(it.Align != tNone)
			line++;

		//Size
		if(isdigit(*line))
		{
			for(cn = 0; *line && strchr("+-0123456789",*line) && cn < (int)(ARRAYSIZE(str)-1); cn++,line++)
				str[cn] = *line;

			str[cn] = 0;
			it.Size = atoi(str);
		}

		//Type
		for(cn = 0; *line && *line != '%' && cn < (int)(ARRAYSIZE(str)-1); cn++,line++)
			str[cn] = *line;

		str[cn] = 0;

		for(cn = 0; StdCommands[cn]; cn++)
			if(StrCmp(StdCommands[cn],str,-1,FALSE) == 0)
				break;

		if(*line == 0 || !StdCommands[cn])
		{
			strcpy(Lines[num]+n,"<badCmd>");
			n += 8;
			break;
		}

		it.Type = cn;

		//Wide center
		if(it.Align == tCenter && !it.Size)
		{
			if(Count >= MAX_TRAF_ITEMS)
			{
				strcpy(Lines[num]+n,"<many>");
				break;
			}

			Items[Count++] = it;
			continue;
		}

		//Draw
		DrawInfo(&it,tm);
		n += it.Size;
	}

	Lines[num][n] = 0;
}

void TrafficInformation::DrawInfos(time_t tm)
{
	char str[ MAX_TRAF_WIDTH+1 ];
	char key[ MAX_TRAF_WIDTH+1 ];
	int  n,i;
	Count = 0;
	_snprintf(key,ARRAYSIZE(key),"CopyDialog\\%s",FTP_Info->GetMsg(MLanguage));
	LineCount = Min(MAX_TRAF_LINES,FTP_Info->GetRegKeyFullInt(key,"Count", 0));

	if(!LineCount)
	{
		LineCount = ARRAYSIZE(StdDialogLines);

		for(i = 0; i < LineCount; i++)
			FormatLine(i,StdDialogLines[i],tm);
	}
	else
	{
		for(i = 0; i < LineCount; i++)
		{
			FTP_Info->GetRegKeyFullStr(key, FTP_Info->Message("Line%02d",i+1), str,"",ARRAYSIZE(str));

			if(!str[0]) break;

			FormatLine(i,str,tm);
		}

		LineCount = i;
	}

	if(!Count)
		return;

	int w;

	for(w = n = 0; n < LineCount; n++)
		w = Max(w,static_cast<int>(strlen(Lines[n])));

	for(n = 0; n < LineCount; n++)
		if(Lines[n][0] != '\x1' && Lines[n][0] != '\x2')
		{
			for(i = static_cast<int>(strlen(Lines[n])); i < w; i++)
				Lines[n][i] = ' ';

			Lines[n][i] = 0;
		}

	for(n = 0; n < Count; n++)
	{
		//Center
		if(Items[n].Align == tCenter)
			Items[n].Pos = w/2 - Items[n].Size/2;
		else if(Items[n].Align == tRightFill)
			Items[n].Size = w - Items[n].Pos;

		if(static_cast<int>(strlen(Lines[Items[n].Line])) > Items[n].Pos)
			DrawInfo(&Items[n],tm);
	}

	for(n = 0; n < LineCount; n++)
		Lines[n][w] = 0;
}

void TrafficInformation::Init(HANDLE h,int tMsg,int OpMode,FP_SizeItemList* il)
{
	memset(this,0,sizeof(*this));
	memset(Lines,' ',MAX_TRAF_LINES*MAX_TRAF_WIDTH);
	TitleMsg       = tMsg;
	TotalStartTime = time(NULL);
	FullFileSize   = -1;

	if(IS_FLAG(OpMode,OPM_FIND))
	{
		ShowStatus = FALSE;
	}
	else if(IS_FLAG(OpMode,OPM_VIEW) || IS_FLAG(OpMode,OPM_EDIT))
	{
		if(FTP_Info->GetOpt()->ShowSilentProgress)
			ShowStatus = TRUE;
		else
			ShowStatus = FALSE;
	}
	else
		ShowStatus = TRUE;

	if(il)
		for(int i = 0; i < il->Count(); i++)
			if(!IS_FLAG(il->List[i].FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY) &&
			        il->List[i].FindData.dwReserved1 != MAX_DWORD)
			{
				TotalFullSize += ((__int64)il->List[i].FindData.nFileSizeHigh) << 32 | il->List[i].FindData.nFileSizeLow;
				TotalFiles++;
			}
}

void TrafficInformation::InitFile(__int64 sz,LPCSTR SrcName,LPCSTR DestName)
{
	if(FullFileSize != -1)
	{
		TotalComplete++;
		TotalFiles    = Max(TotalFiles, TotalSkipped+TotalComplete);
		TotalSize    += FullFileSize;
		TotalFullSize = Max(TotalSize, TotalFullSize);
	}

	if(sz == -1) sz = 0;

	FullFileSize       = sz;
	FileSize           = 0;
	LastSize           = 0;
	StartFileSize      = 0;
	Cps                = 0;
	FileStartTime      = time(NULL);
	FileWaitTime       = 0;
	AvCps[0] = 0; AvCps[1] = 0; AvCps[2] = 0;
	GET_TIME(LastTime);
	StrCpy(SrcFileName,  SrcName,  ARRAYSIZE(SrcFileName));
	StrCpy(DestFileName, DestName, ARRAYSIZE(DestFileName));
}

void TrafficInformation::Skip(void)
{
	TotalStartSize  += FullFileSize;
	TotalSkipped++;
	TotalFiles      = Max(TotalFiles,TotalSkipped+TotalComplete);
	StartFileSize   = 0;
	FileSize        = 0;
	FullFileSize    = -1;
}

void TrafficInformation::Resume(LPCSTR nm)
{
	WIN32_FIND_DATA ffi;
	HANDLE          ff = FindFirstFile(nm,&ffi);

	if(ff == INVALID_HANDLE_VALUE)
	{
		StartFileSize   = 0;
		FileSize        = 0;
		return;
	}

	FindClose(ff);
	StartFileSize   = ((__int64)ffi.nFileSizeHigh) << 32 | ffi.nFileSizeLow;
	FileSize        = 0;
}

void TrafficInformation::Resume(__int64 size)
{
	StartFileSize   = size;
	FileSize        = 0;
}

void TrafficInformation::Waiting(time_t tm)
{
	TotalWaitTime += tm;
	FileWaitTime  += tm;
}
/****************************************
    Callback called from GetFiles and PutFiles each time whole buffer
    sended or received to refresh state of copy progress
    Can be called in any plugin state (quite, not show, etc)
 ****************************************/
BOOL TrafficInformation::Callback(int Size)
{
	double tmDiff;
	DWORD     tm;
	time_t        tmt;
	char          str[MAX_PATH];
	double        TotalPercent,db;
	GET_TIME(tm);
	tmDiff = Abs(CMP_TIME(tm,LastTime));
	tmt = time(NULL);
	FileSize     += Size;
	FullFileSize  = Max(FileSize,FullFileSize);

	if(Size && FTP_Info->CheckForEsc(FALSE,FALSE))
	{
		Log(("User cancel"));
		return FALSE;
	}

	if(Size && tmDiff*1000 < FTP_Info->GetOpt()->IdleShowPeriod)
		return TRUE;

	memmove(&LastTime,&tm,sizeof(LastTime));
	TotalPercent = ToPercent(TotalSz(),TotalFullSize);

	//Avg
	if(tmDiff > 0)
	{
		AvCps[0] = AvCps[1];
		AvCps[1] = AvCps[2];
		AvCps[2] = (FileSize - LastSize) / tmDiff;
	}

	LastSize = FileSize;
	//Cur
	db = ((double)tmt-FileStartTime-FileWaitTime);

	if(db > 0)
		Cps = FileSize / db;
	else
		Cps = 0;

	//Total
	db = ((double)tmt-TotalStartTime-TotalWaitTime);

	if(db > 0)
		TotalCps = (TotalSize+FileSize) / db;
	else
		TotalCps = 0;

//Show QUIET progressing
	if(!ShowStatus)
	{
		_snprintf(str,ARRAYSIZE(str),
		         "{%2.1lf%%} %s: %.26s",
		         TotalPercent,
		         FTP_Info->GetMsg(TitleMsg),
		         FTP_Info->PointToName(SrcFileName));
		FTP_Info->IdleMessage(str,FTP_Info->GetOpt()->ProcessColor);
		return TRUE;
	}

//Window caption
	if(FTP_Info->FtpGetRetryCount())
		_snprintf(str,ARRAYSIZE(str),"%d: {%2.1lf%%} %s - Far",
		         FTP_Info->FtpGetRetryCount(), TotalPercent,
		         FTP_Info->GetMsg(TitleMsg));
	else
		_snprintf(str,ARRAYSIZE(str),"{%2.1lf%%} %s - Far",
		         TotalPercent,FTP_Info->GetMsg(TitleMsg));

	if(StrCmp(str,ConsoleTitle,-1,TRUE) != 0)
	{
		if(FTP_Info->WinVer->dwPlatformId != VER_PLATFORM_WIN32_NT)
			OemToChar(str,str);

		SetConsoleTitle(str);
		StrCpy(ConsoleTitle,str,ARRAYSIZE(ConsoleTitle));
	}

//Mark CMD window invisible
	FTP_Info->FtpCmdBlock(TRUE);
//Show message
	LPCSTR MsgItems[MAX_TRAF_LINES+1];
	DrawInfos(tmt);

	if(FTP_Info->FtpGetRetryCount())
		_snprintf(str,ARRAYSIZE(str),"%d: %s",FTP_Info->FtpGetRetryCount(),FTP_Info->GetMsg(TitleMsg));
	else
		StrCpy(str,FTP_Info->GetMsg(TitleMsg),ARRAYSIZE(str));

	MsgItems[0] = str;
	int n;

	for(n = 0; n < LineCount; n++)
		MsgItems[n+1] = Lines[n];

	FTP_Info->FMessage(FMSG_LEFTALIGN, NULL,
	                   (LPCSTR *)MsgItems, LineCount+1, 0);
	return TRUE;
}
