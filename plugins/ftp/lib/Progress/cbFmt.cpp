#include <all_far.h>
#pragma hdrstop

#include "Int.h"

/****************************************
   TrafficInformation
 ****************************************/
LPCSTR StdCommands[] =
{
	/*00*/" ",               //Space
	/*01*/"SrcPathname",     //Source filename
	/*02*/"SrcPath",
	/*03*/"SrcName",

	/*04*/"DestPathname",    //Target filename
	/*05*/"DestPath",
	/*06*/"DestName",

	/*07*/"CurSize",         //Current processed size
	/*08*/"CurESize",        //Full size of current file
	/*09*/"CurFSize",
	/*10*/"CurRSize",        //Remain size of current file

	/*11*/"CurTime",         //Time from start of current file
	/*12*/"CurFTime",        //Full time to process current file
	/*13*/"CurRTime",        //Remain time to process current file
	/*14*/"CurETime",        //End time to process current file
	/*15*/"CurEDate",        //End date to process current file

	/*16*/"TotSize",         //Current processed size
	/*17*/"TotESize",        //Full size of current file
	/*18*/"TotFSize",
	/*19*/"TotRSize",        //Remain size of current file

	/*20*/"TotTime",         //Time from start of current file
	/*21*/"TotFTime",        //Full time to process current file
	/*22*/"TotRTime",        //Remain time to process current file
	/*23*/"TotETime",        //End time to process current file
	/*24*/"TotEDate",        //End date to process current file

	/*25*/"CurCPS",          //CP of current file
	/*26*/"CPS",             //Summary CPS
	/*27*/"TotCPS",          //Total CPS

	/*28*/"Progress",        //Current progress bar
	/*29*/"FProgress",       //Total progress bar
	/*30*/"Percent",         //Current "99.9%" percent
	/*31*/"FPercent",        //Total "99.9%" percent

	/*32*/"Cn",              //Complete count
	/*33*/"CnSkip",          //Skipped count
	/*34*/"CnR",             //Remain count
	/*35*/"CnF",             //Count

	/*36*/"CurPc",           //Current percent
	/*37*/"SkipPc",          //Skipped percent
	/*38*/"CurPg",           //Current percent progress
	/*39*/"SkipPg",          //Skipped percent progress
	/*40*/"Pc",              //ALIAS `Percent`
	/*41*/"FPc",             //ALIAS `FPercent`
	/*42*/"Pg",              //ALIAS `Progress`
	/*43*/"FPg",             //ALIAS `FProgress`

	/*44*/"CurCPS4",         //CP of current file
	/*45*/"CPS4",            //Summary CPS
	/*46*/"TotCPS4",         //Total CPS

	/*47*/"IPc",             //Percent without float part
	/*48*/"IFPc",            //FullPercent without float part

	NULL
};

/** Wraper for localtime rtl function

    RTL function may return NULL
*/
struct tm *Localtime(const time_t *timer)
{
	struct tm *p = localtime(timer);

	if(!p)
	{
		time_t tm = time(NULL);
		p = localtime(&tm);
	}

	Assert(p && "Localtime");

	return p;
}

/****************************************
   TrafficInformation
 ****************************************/
void TrafficInformation::DrawInfo(InfoItem* it,time_t tm)
{
	static char str[MAX_PATH+1];
	double db;
	int    cn;
	time_t tmp;
	struct tm *ltm;
	int    len;

	switch(it->Type)
	{
			//00 " " ---- Space
		case 00:
			break;
			//01 "SrcPathname" ---- Source filename
		case 01: FTP_Info->StrCpy(str,SrcFileName,ARRAYSIZE(str));
			break;
			//02 "SrcPath",
		case 02: FTP_Info->StrCpy(str,SrcFileName,ARRAYSIZE(str));
			FTP_Info->PointToName(str)[0] = 0;
			break;
			//03 "SrcName",
		case 03: FTP_Info->StrCpy(str,FTP_Info->PointToName(SrcFileName),ARRAYSIZE(str));
			break;
			//04 "DestPathname" ---- Target filename
		case 04: FTP_Info->StrCpy(str,DestFileName,ARRAYSIZE(str));
			break;
			//05 "DestPath",
		case 05: FTP_Info->StrCpy(str,DestFileName,ARRAYSIZE(str));
			FTP_Info->PointToName(str)[0] = 0;
			break;
			//06 "DestName",
		case 06: FTP_Info->StrCpy(str,FTP_Info->PointToName(DestFileName),ARRAYSIZE(str));
			break;
			//07 "CurSize" ---- Current processed size
		case 07: FTP_Info->FDigit(str,CurrentSz(),-1);
			break;
			//08 "CurESize" ---- Full size of current file
			//09 "CurFSize",
		case  8:
		case  9: FTP_Info->FDigit(str,FullFileSize,-1);
			break;
			//10 "CurRSize" ---- Remain size of current file
		case 10: FTP_Info->FDigit(str,CurrentRemain(),-1);
			break;
			//11 "CurTime" ---- Time from start of current file
		case 11: cn = (int)(tm - FileStartTime);
			_snprintf(str,ARRAYSIZE(str), "%02d:%02d:%02d", cn/3600, (cn/60)%60, cn%60);
			break;
			//12 "CurFTime" ---- Full time to process current file
		case 12: db = Cps;

			if(db > 0)
				cn = (int)(CurrentDoRemain() / db);
			else
				cn = 0;

			cn = Max(cn,0);
			_snprintf(str,ARRAYSIZE(str), "%02d:%02d:%02d", cn/3600, (cn/60)%60, cn%60);
			break;
			//13 "CurRTime" ---- Remain time to process current file
		case 13: db = Cps;

			if(db > 0)
				cn = (int)(CurrentRemain() / db);
			else
				cn = 0;

			cn = Max(cn,0);
			_snprintf(str,ARRAYSIZE(str), "%02d:%02d:%02d", cn/3600, (cn/60)%60, cn%60);
			break;
			//14 "CurETime" ---- End time to process current file
		case 14: db = Cps;

			if(db > 0)
				tmp = tm + (time_t)(CurrentRemain() / db);
			else
				tmp = tm;

			tmp = Max(tmp,tm);
			ltm = Localtime(&tmp);
			StrTTime(str,ltm);
			break;
			//15 "CurEDate" ---- End date to process current file
		case 15: db = Cps;

			if(db > 0)
				tmp = tm + (time_t)(CurrentRemain() / db);
			else
				tmp = tm;

			tmp = Max(tmp,tm);
			ltm = Localtime(&tmp);
			StrYTime(str,ltm);
			break;
			//16 "TotSize" ---- Current processed size
		case 16: FTP_Info->FDigit(str,TotalSz(),-1);
			break;
			//17 "TotESize" ---- Full size of current file
			//18 "TotFSize",
		case 17:
		case 18: FTP_Info->FDigit(str,TotalFullSize,-1);
			break;
			//19 "TotRSize" ---- Remain size of current file
		case 19: FTP_Info->FDigit(str,TotalRemain(),-1);
			break;
			//20 "TotTime" ---- Time from start of current file
		case 20: cn = (int)(tm - TotalStartTime);
			_snprintf(str,ARRAYSIZE(str), "%02d:%02d:%02d", cn/3600, (cn/60)%60, cn%60);
			break;
			//21 "TotFTime" ---- Full time to process current file
		case 21: db = (AvCps[0] + AvCps[1] + AvCps[2])/3;

			if(db > 0)
				cn = (int)(TotalDoRemain() / db);
			else
				cn = 0;

			cn = Max(cn,0);
			_snprintf(str,ARRAYSIZE(str), "%02d:%02d:%02d", cn/3600, (cn/60)%60, cn%60);
			break;
			//22 "TotRTime" ----- Remain time to process current file
		case 22: db = (AvCps[0] + AvCps[1] + AvCps[2])/3;

			if(db > 0)
				cn = (int)(TotalRemain() / db);
			else
				cn = 0;

			cn = Max(cn,0);
			_snprintf(str,ARRAYSIZE(str), "%02d:%02d:%02d", cn/3600, (cn/60)%60, cn%60);
			break;
			//23 "TotETime" ---- End time to process all files
		case 23: db = TotalCps;

			if(db > 0)
				tmp = tm + (time_t)(TotalRemain() / db);
			else
				tmp = tm;

			ltm = Localtime(&tmp);
			StrTTime(str,ltm);
			break;
			//24 "TotEDate" ----- End date to process current file
		case 24: db = TotalCps;

			if(db > 0)
				tmp = tm + (time_t)(TotalRemain() / db);
			else
				tmp = tm;

			ltm = Localtime(&tmp);
			StrYTime(str,ltm);
			break;
			//25 "CurCPS"   ----- CP of current file
		case 25: FTP_Info->FCps(str,Cps);
			break;
			//26 "CPS"      -----  Summary CPS
		case 26: FTP_Info->FCps(str,(AvCps[0] + AvCps[1] + AvCps[2])/3);
			break;
			//27 "TotCPS"   ----- Total CPS
		case 27: FTP_Info->FCps(str,TotalCps);
			break;
			//28 "Progress" ----- Current progress bar
			//42 "Pg"       ----- ALIAS `Progress`
		case 42:
		case 28: PPercent(str,0,it->Size-1,(int)ToPercent(CurrentSz(),FullFileSize));
			break;
			//29 "FProgress"----- Total progress bar
			//43 "FPg"      ----- ALIAS `FProgress`
		case 43:
		case 29: PPercent(str,0,it->Size-1,(int)ToPercent(TotalSz(),TotalFullSize));
			break;
			//40 "Pc"       ----- ALIAS `Percent`
			//30 "PerCent"  ----- Current "99.9%" percent
		case 40:
		case 30: db = ToPercent(CurrentSz(),FullFileSize);

			if(((int)db) == 100)
				FTP_Info->StrCpy(str,"100%",-1);
			else
				sprintf(str,"%2.1lf%%",db);

			break;
			//41 "FPc" ---- ALIAS `FPercent`
			//31 "FPerCent" ----- Total "99.9%" percent
		case 41:
		case 31: db = ToPercent(TotalSz(),TotalFullSize);

			if(((int)db) == 100)
				FTP_Info->StrCpy(str,"100%",-1);
			else
				sprintf(str,"%2.1lf%%",db);

			break;
			//32 "Cn" ---- Complete count
		case 32: FTP_Info->FDigit(str,TotalComplete,-1);
			break;
			//33 "CnSkip" ---- Skipped count
		case 33: FTP_Info->FDigit(str,TotalSkipped,-1);
			break;
			//34 "CnR" ---- Remain count
		case 34: FTP_Info->FDigit(str,TotalFiles-TotalComplete-TotalSkipped,-1);
			break;
			//35 "CnF" ---- Count
		case 35: FTP_Info->FDigit(str,TotalFiles,-1);
			break;
			//36 "CurPc" ---- Current percent
		case 36: db = ToPercent(TotalComplete,TotalFiles);

			if(((int)db) == 100)
				FTP_Info->StrCpy(str,"100%",-1);
			else
				sprintf(str,"%2.1lf%%",db);

			break;
			//37 "SkipPc" ---- Skipped percent
		case 37: db = ToPercent(TotalSkipped,TotalFiles);

			if(((int)db) == 100)
				FTP_Info->StrCpy(str,"100%",-1);
			else
				sprintf(str,"%2.1lf%%",db);

			break;
			//38 "CurPg"     ---- Current percent progress
		case 38: PPercent(str,0,it->Size-1,(int)ToPercent(TotalComplete,TotalFiles));
			break;
			//39 "SkipPg"    ---- Skipped percent progress
		case 39: PPercent(str,0,it->Size-1,(int)ToPercent(TotalSkipped,TotalFiles));
			break;
			//44 "CurCPS4"   ----- CP of current file
		case 44: FCps4(str,Cps);
			break;
			//45 "CPS4"      -----  Summary CPS
		case 45: FCps4(str,(AvCps[0] + AvCps[1] + AvCps[2])/3);
			break;
			//46 "TotCPS4"   ----- Total CPS
		case 46: FCps4(str,TotalCps);
			break;
			//47 "IPc"       ----- Current "99%" percent
		case 47: db = ToPercent(CurrentSz(),FullFileSize);

			if(((int)db) == 100)
				FTP_Info->StrCpy(str,"100%",-1);
			else
				sprintf(str,"%d%%",(int)db);

			break;
			//48 "IFPc"      ----- Total "99%" percent
		case 48: db = ToPercent(TotalSz(),TotalFullSize);

			if(((int)db) == 100)
				FTP_Info->StrCpy(str,"100%",-1);
			else
				sprintf(str,"%d%%",(int)db);

			break;
	}

	if(it->Type)
	{
		len = static_cast<int>(strlen(str));

		if(!it->Size && it->Align == tCenter)
			it->Pos -= len/2;

		if(!it->Size)
			it->Size = len;

		if(len > it->Size && it->Align == tCenter)
			it->Align = tLeft;

		if(it->Pos+it->Size > MAX_TRAF_WIDTH)
			return;

		if(len > it->Size)
		{
			if(it->Align == tLeft)
			{
				memmove(Lines[it->Line]+it->Pos,str+len-it->Size,it->Size);
				Lines[it->Line][it->Pos] = FAR_LEFT_CHAR;
			}
			else
			{
				memmove(Lines[it->Line]+it->Pos,str,it->Size);
				Lines[it->Line][it->Pos+it->Size] = FAR_RIGHT_CHAR;
			}
		}
		else
		{
			//Clear
			if(it->Align != tCenter)
				memset(Lines[it->Line]+it->Pos,it->Fill,it->Size);

			//Draw
			if(it->Align == tLeft)
				memmove(Lines[it->Line]+it->Pos, str, len);
			else if(it->Align == tRight)
				memmove(Lines[it->Line]+it->Pos + it->Size - len, str, len);
			else
				memmove(Lines[it->Line]+it->Pos + it->Size/2 - len/2, str, len);
		}
	}
	else
	{
		memset(Lines[it->Line]+it->Pos,it->Fill,it->Size);
	}
}
