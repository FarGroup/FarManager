#include <all_far.h>
#pragma hdrstop

#include "Int.h"

//------------------------------------------------------------------------
LPCSTR UType2Str(sliTypes tp)
{
	switch(tp)
	{
		case sltUrlList:
			return "URLS_LIST";
		case    sltTree:
			return "URLS_TREE";
		case   sltGroup:
			return "URLS_GROUP";
		default:
			HAbort("Url type not supported");
			return NULL;
	}
}
sliTypes Str2UType(LPCSTR s)
{
	if(StrCmpI(s,"URLS_LIST") == 0) return sltUrlList;

	if(StrCmpI(s,"URLS_TREE") == 0) return sltTree;

	if(StrCmpI(s,"URLS_GROUP") == 0) return sltGroup;

	return sltNone;
}
//------------------------------------------------------------------------
LPCSTR GetOtherPath(char *path)
{
	PanelInfo pi;

	do
	{
		if(FP_Info->Control(INVALID_HANDLE_VALUE, FCTL_GETANOTHERPANELSHORTINFO, &pi) &&
		        pi.PanelType == PTYPE_FILEPANEL &&
		        !pi.Plugin)
			break;

		if(FP_Info->Control(INVALID_HANDLE_VALUE, FCTL_GETPANELSHORTINFO, &pi) &&
		        pi.PanelType == PTYPE_FILEPANEL &&
		        !pi.Plugin)
			break;

		return FMSG(MFLErrGetInfo);
	}
	while(0);

	StrCpy(path, pi.CurDir, MAX_PATH);
	AddEndSlash(path,'\\',MAX_PATH);
	return NULL;
}

void SayOutError(LPCSTR m,int tp = 0)
{
	LPCSTR itms[] = { FMSG(MFLErrCReate), NULL, FMSG(MOk) };
	itms[1] = m;
	FMessage(tp + FMSG_WARNING,NULL,itms,3,1);
}

void FTP::SaveList(FP_SizeItemList* il)
{
	LPCSTR m;

	if((m=GetOtherPath(Opt.sli.path)) != NULL)
	{
		SayOutError(m);
		return;
	}

	strcat(Opt.sli.path,"ftplist.lst");

	if(!AskSaveList(&Opt.sli))
		return;

	FILE *f = fopen(Opt.sli.path, Opt.sli.Append ? "a" : "w");

	if(!f)
	{
		SayOutError(FMSG(MFLErrCReate),FMSG_ERRORTYPE);
		return;
	}

	PluginPanelItem* p;
	int              n;
	int              level;
	char             str[1024+2],
	   BasePath[1024+2],
	   CurrentUrlPath[1024+2];
	CurrentUrlPath[0] = 0;
	_snprintf(BasePath, ARRAYSIZE(BasePath),
	          "%s%s%s%s",
	          Opt.sli.AddPrefix ? "ftp://" : "",
	          Opt.sli.AddPasswordAndUser ? Message("%s:%s@",hConnect->UserName,hConnect->UserPassword) : "",
	          hConnect->hostname,
	          hConnect->CurDir.c_str());
	AddEndSlash(BasePath,'/',ARRAYSIZE(BasePath));

	if(Opt.sli.ListType == sltTree)
		fprintf(f,"BASE: \"%s\"\n",BasePath);

	for(n = 0; n < il->Count(); n++)
	{
		p = il->Item(n);

		if(p->FindData.dwReserved1 == MAX_DWORD)
			continue;

		//URLS --------------------------------------
		if(Opt.sli.ListType == sltUrlList)
		{
			if(IS_FLAG(p->FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
				continue;

			FixFTPSlash(FTP_FILENAME(p));
			_snprintf(str,ARRAYSIZE(str),"%s%s",BasePath,FTP_FILENAME(p));

			if(Opt.sli.Quote) QuoteStr(str);

			fprintf(f,"%s\n",str);
		}
		else

			//TREE --------------------------------------
			if(Opt.sli.ListType == sltTree)
			{
				StrCpy(str, FTP_FILENAME(p), ARRAYSIZE(str));
				FixFTPSlash(str);

				for(m = str,level = 0;
				        (m=strchr(m,'/')) != NULL;
				        m++,level++);

				fprintf(f,"%*c", level*2+2, ' ');
				m = strrchr(str,'/');

				if(m) m++;
				else m = str;

				fprintf(f,"%c%s",
				        IS_FLAG(p->FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY) ? '/' : ' ', m);

				if(Opt.sli.Size)
				{
					level = Max(1, Opt.sli.RightBound - 10 - level*2 - 2 - 1 - (int)strlen(m));
					fprintf(f,"%*c",level,' ');

					if(IS_FLAG(p->FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
						fprintf(f,"<DIR>");
					else
						fprintf(f,"%10I64u", ((__int64)p->FindData.nFileSizeHigh) << 32 | p->FindData.nFileSizeLow);
				}

				fprintf(f,"\n");
			}
			else

				//GROUPS ------------------------------------
				if(Opt.sli.ListType == sltGroup)
				{
					if(IS_FLAG(p->FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
						continue;

					FixFTPSlash(FTP_FILENAME(p));
					_snprintf(str, ARRAYSIZE(str), "%s%s", BasePath, FTP_FILENAME(p));

					if(!IS_FLAG(p->FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
						*strrchr(str,'/') = 0;

					if(StrCmp(CurrentUrlPath,str,-1,FALSE) != 0)
					{
						StrCpy(CurrentUrlPath, str, ARRAYSIZE(CurrentUrlPath));
						fprintf(f,"\n[%s]\n", CurrentUrlPath);
					}

					StrCpy(str, FTP_FILENAME(p), ARRAYSIZE(str));
					FixFTPSlash(str);
					m = strrchr(str,'/');

					if(m) m++;
					else m = str;

					fprintf(f," %s", m);

					if(Opt.sli.Size)
					{
						level = Max(1, Opt.sli.RightBound - 10 - (int)strlen(m) - 1);
						fprintf(f,"%*c%10I64u",
						        level,' ',
						        ((__int64)p->FindData.nFileSizeHigh) << 32 | p->FindData.nFileSizeLow);
					}

					fprintf(f,"\n");
				}
	}

	fclose(f);
	LPCSTR itms[] = { FMSG(MFLDoneTitle), FMSG(MFLFile), Opt.sli.path, FMSG(MFLDone), FMSG(MOk) };
	FMessage(FMSG_LEFTALIGN,NULL,itms,5,1);
}
//------------------------------------------------------------------------
#define MNUM( v ) ( ((int*)(v).Text)[ 128/sizeof(int) - 1] )
#define MSZ       (128-(int)sizeof(int))

BOOL FTP::ShowFilesList(FP_SizeItemList* il)
{
	int              cn,n,i,w,
	  num;
	int              Breaks[] = { VK_INSERT, VK_F2, 0 },
	                            BNumber;
	char             str[ 500 ];
	PluginPanelItem *p;
	FarMenuItem     *mi = NULL;
	char            *m,*nm;

	if(!il || !il->Count())
		return FALSE;

	//Create|Recreate
	mi = (FarMenuItem *)realloc(mi,il->Count()*sizeof(FarMenuItem));
	memset(mi, 0, il->Count()*sizeof(FarMenuItem));
	//Scan number of items
	w = cn = 0;

	for(i = n = 0; n < il->Count(); n++)
	{
		p = il->Item(n);
		p->NumberOfLinks        = StrSlashCount(FTP_FILENAME(p));
		p->FindData.dwReserved1 = 0;
		w = Max(w,static_cast<int>(strlen(PointToName(FTP_FILENAME(p)))) + (int)p->NumberOfLinks + 1);
		cn++;
		MNUM(mi[i++]) = n;
	}

	w = Min(Max(60,Min(w,MSZ)),FP_ConWidth()-8);

	if(!cn)
	{
		free(mi);
		return FALSE;
	}

	//Calc length of size and count digits
	int szSize = 0,
	    szCount = 0;

	for(n = 0; n < cn; n++)
	{
		p = il->Item(MNUM(mi[n]));
		FDigit(str, ((__int64)p->FindData.nFileSizeHigh) << 32 | p->FindData.nFileSizeLow, -1);
		szSize = Max(static_cast<int>(strlen(str)),szSize);

		if(IS_FLAG(p->FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
		{
			FDigit(str,p->FindData.dwReserved0,-1);
			szCount = Max(static_cast<int>(strlen(str)),szCount);
		}
	}

	//Filename width
	w -= szSize + szCount + 6;

	//Set menu item text
	for(n = 0; n < cn; n++)
	{
		p = il->Item(MNUM(mi[n]));
		m = mi[n].Text;

		for(i = 0; i < (int)p->NumberOfLinks; i++)
		{
			*(m++) = ' ';
			*(m++) = ' ';
		}

		nm = PointToName(FTP_FILENAME(p));

		for(i = (int)(m - mi[n].Text); i < w; i++)
			if(*nm)
				*(m++) = *(nm++);
			else
				*(m++) = ' ';

		*(m++) = (*nm) ? FAR_SBMENU_CHAR : ' ';

		if(szSize)
		{
			*(m++) = ' ';
			*(m++) = FAR_VERT_CHAR;
			*(m++) = ' ';
			FDigit(str, ((__int64)p->FindData.nFileSizeHigh) << 32 | p->FindData.nFileSizeLow, -1);
			m += sprintf(m,"%*s",szSize,str);
		}

		if(szCount)
		{
			*(m++) = ' ';
			*(m++) = FAR_VERT_CHAR;
			*(m++) = ' ';

			if(IS_FLAG(p->FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
				FDigit(str,p->FindData.dwReserved0,-1);
			else
				str[0] = 0;

			m += sprintf(m,"%*s",szCount,str);
		}

		*m = 0;
	}

	num = -1;

	do
	{
		//Set selected
		for(n = 0; n < cn; n++)
			mi[n].Checked = il->Items()[ MNUM(mi[n])].FindData.dwReserved1 != MAX_DWORD;

		//Title
		__int64 tsz = 0,tcn = 0;

		for(n = 0; n < cn; n++)
		{
			p = il->Item(MNUM(mi[n]));

			if(p->FindData.dwReserved1 != MAX_DWORD &&
			        !IS_FLAG(p->FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
			{
				tsz += ((__int64)p->FindData.nFileSizeHigh) << 32 | p->FindData.nFileSizeLow;
				tcn++;
			}
		}

		StrCpy(str,FP_GetMsg(FMSG(MListTitle)),ARRAYSIZE(str));
		StrCat(str," (",ARRAYSIZE(str));
		StrCat(str,FDigit(NULL,tsz,-1),ARRAYSIZE(str));

		if(il->TotalFullSize != tsz)
		{
			StrCat(str,"{",ARRAYSIZE(str));
			StrCat(str,FDigit(NULL,il->TotalFullSize,-1),ARRAYSIZE(str));
			StrCat(str,"}",ARRAYSIZE(str));
		}

		StrCat(str,"/",ARRAYSIZE(str));
		StrCat(str,FDigit(NULL,tcn,-1),ARRAYSIZE(str));

		if(tcn != il->TotalFiles)
		{
			StrCat(str,"{",ARRAYSIZE(str));
			StrCat(str,FDigit(NULL,il->TotalFiles,-1),ARRAYSIZE(str));
			StrCat(str,"}",ARRAYSIZE(str));
		}

		StrCat(str,")",ARRAYSIZE(str));
		//Menu
		n = FP_Info->Menu(FP_Info->ModuleNumber,-1,-1,0,FMENU_SHOWAMPERSAND,
		                  str,
		                  FP_GetMsg(FMSG(MListFooter)),
		                  "FTPFilesList", Breaks, &BNumber, mi,cn);

		//key ESC
		if(n == -1)
		{
			num = FALSE;
			break;
		}

		//key Enter
		if(BNumber == -1)
		{
			num = TRUE;
			break;
		}

		//Set selected
		if(num != -1) mi[num].Selected = FALSE;

		num = n;
		mi[num].Selected = TRUE;
		//Process keys
		bool set;

		switch(BNumber)
		{
				/*INS*/
			case 0: //Current
				p = il->Item(MNUM(mi[num]));
				//Next item
				n = num+1;
				//Switch selected
				set = p->FindData.dwReserved1 != MAX_DWORD;
				p->FindData.dwReserved1 = set ? MAX_DWORD : 0;

				//Switch all nested
				if(IS_FLAG(p->FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY))
				{
					i = StrSlashCount(FTP_FILENAME(p));

					for(; n < cn; n++)
					{
						p = il->Item(MNUM(mi[n]));

						if(StrSlashCount(FTP_FILENAME(p)) <= i)
							break;

						p->FindData.dwReserved1 = set ? MAX_DWORD : 0;
					}
				}

				//INS-moves-down
				if(n < cn)
				{
					mi[num].Selected = FALSE;
					mi[n].Selected   = TRUE;
					num = n;
				}

				break;
				/*F2*/
			case 1:
				SaveList(il);
				break;
		}
	}
	while(true);

	free(mi);

	if(!num)
		return FALSE;

	return TRUE;
}
