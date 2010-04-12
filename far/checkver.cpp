/*
checkver.cpp

Проверка регистрации

*/

#include "headers.hpp"
#pragma hdrstop

#include "lang.hpp"
#include "global.hpp"
#include "dialog.hpp"
#include "fn.hpp"

static const char KeyRegistration[]="Registration";

static const unsigned char MyName[]=
{
	'E'^0x50,'u'^0x51,'g'^0x52,'e'^0x53,'n'^0x54,'e'^0x55,
	' '^0x56,'R'^0x57,'o'^0x58,'s'^0x59,'h'^0x5a,'a'^0x5b,'l'^0x5c,
	' '^0x5d,'a'^0x5e,'n'^0x5f,'d'^0x60,
	' '^0x61,'F'^0x62,'A'^0x63,'R'^0x64,
	' '^0x65,'G'^0x66,'r'^0x67,'o'^0x68,'u'^0x69,'p'^0x6a,
};

static const char *GetDaysName(int wDayOfWeek)
{
	static unsigned char Days[7][16]=
	{
		{"\x0C\xF7\xF8\xB6\xF2\xB9\xFF\xBA\xF9\xF0\xB2\xFA"}, // "воскресенье",
		{"\x0c\xFA\xF8\xFA\xFD\xFD\xFF\xF0\xB0\xF0\xF6\xF5"}, // "понедельник",
		{"\x08\xF7\xB4\xF9\xB8\xF4\xF2\xF1"},                 // "вторник",
		{"\x06\xB4\xB6\xF2\xFC\xF9"},                         // "среда",
		{"\x08\xB2\xF3\xB5\xFA\xFC\xBA\xF8"},                 // "четверг",
		{"\x08\xFA\xB9\xB5\xF5\xF1\xBC\xFB"},                 // "пятница",
		{"\x08\xB4\xB5\xF6\xF9\xF7\xB8\xFB"},                 // "суббота"
	};

	if (Days[0][0])
	{
		int I, J;

		for (J=0; J < 7; ++J)
		{
			unsigned char B=0x55;

			for (I=1; I < Days[J][0]; ++I, ++B)
				Days[J][I]^=B;
		}

		Days[0][0]=0x00;
	}

	return (const char*)&Days[wDayOfWeek][1];
}

static const char *GetxUSSRRegName()
{
	// "xUSSR регистрация"
	static unsigned char *xUSSRRegName=(BYTE*)"\x12\x2D\x03\x04\x0B\x0B\x7A\xBB\xF9\xFE\xF6\xBE\x82\x81\xC2\x85\xCC\x8A";
	static unsigned char xUSSRRegNameDec [64];
	static int xUSSRDecrypted = 0;

	if (!xUSSRDecrypted)
	{
		unsigned char B=0x55;
		xUSSRRegNameDec[xUSSRRegName[0]] = 0;

		for (int I=1; I < xUSSRRegName[0]; ++I, ++B)
			xUSSRRegNameDec[I-1] = xUSSRRegName[I] ^ B;

		xUSSRDecrypted = 1;
	}

	return (const char*)xUSSRRegNameDec;
}

#ifndef _MSC_VER
#pragma warn -par
#endif
void __cdecl CheckVersion(void *Param)
{
	Sleep(1000);

	if (!RegVer)
	{
		Opt.ViOpt.TabSize=Opt.EdOpt.TabSize=8;
		Opt.ViewerEditorClock=0;
	}

	if (!RegistrationBugs)
		_endthread();
}
#ifndef _MSC_VER
#pragma warn +par
#endif

void Register()
{
	static struct DialogData RegDlgData[]=
	{
		DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(char *)MRegTitle,
		DI_TEXT,5,2,0,2,0,0,0,0,(char *)MRegUser,
		DI_EDIT,5,3,70,3,1,0,0,0,"",
		DI_TEXT,5,4,0,4,0,0,0,0,(char *)MRegCode,
		DI_EDIT,5,5,70,5,0,0,0,0,"",
		DI_TEXT,3,6,0,6,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
		DI_BUTTON,0,7,0,7,0,0,DIF_CENTERGROUP,1,(char *)MOk,
		DI_BUTTON,0,7,0,7,0,0,DIF_CENTERGROUP,0,(char *)MCancel
	};
	MakeDialogItems(RegDlgData,RegDlg);
	Dialog Dlg(RegDlg,sizeof(RegDlg)/sizeof(RegDlg[0]));
	Dlg.SetPosition(-1,-1,76,10);
	Dlg.SetHelp("Register");
	Dlg.SetDialogMode(DMODE_MSGINTERNAL);
	FlushInputBuffer();
	Dlg.Process();

	if (Dlg.GetExitCode()!=6)
		return;

	char RegName[256],RegCode[256],RegData[256];
	xstrncpy(RegName,RegDlg[2].Data,sizeof(RegName)-1);
	xstrncpy(RegCode,RegDlg[4].Data,sizeof(RegCode)-1);
	int Length=(int)strlen(RegName);

	if (*RegName==0 || *RegCode==0)
		return;

	unsigned char Xor=17;
	int I;

	for (I=0; RegName[I]!=0; I++)
		Xor^=RegName[I];

	int xUSSR=FALSE;

	if (strcmp(RegName,GetxUSSRRegName())==0)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);

		if (strcmp(RegCode,GetDaysName(st.wDayOfWeek))==0)
			xUSSR=TRUE;
	}

	if (!xUSSR && (Length<4 || (Xor & 0xf)!=ToHex(RegCode[0]) || ((~(Xor>>4))&0xf)!=ToHex(RegCode[3])))
	{
		Message(MSG_WARNING,1,MSG(MError),MSG(MRegFailed),MSG(MOk));
		return;
	}

	Dlg.Hide();
	RegData[0]=static_cast<char>(clock());
	RegData[1]=(char)strlen(RegName);
	RegData[2]=(char)strlen(RegCode);
	strcpy(RegData+3,RegName);
	strcpy(RegData+RegData[1]+3,RegCode);
	int Size=RegData[1]+RegData[2]+3;

	for (I=0; I<Size; I++)
		RegData[I]^=I+7;

	for (I=1; I<Size; I++)
		RegData[I]^=RegData[0];

	SetRegRootKey(HKEY_LOCAL_MACHINE);
	char SaveKey[512];
	strcpy(SaveKey,Opt.RegRoot);
	strcpy(Opt.RegRoot,"Software\\Far");

	if (SetRegKey(KeyRegistration,"Data",(const BYTE *)RegData,Size) != ERROR_SUCCESS)
	{
		// в случае неудачи пишем в HKCU
		SetRegRootKey(HKEY_CURRENT_USER);
		strcpy(Opt.RegRoot,SaveKey);

		if (SetRegKey(KeyRegistration,"Data",(const BYTE *)RegData,Size) != ERROR_SUCCESS)
		{
			Message(MSG_WARNING,1,MSG(MError),MSG(MRegFailed),MSG(MOk));
			return;
		}
	}

	strcpy(Opt.RegRoot,SaveKey);
	SetRegRootKey(HKEY_CURRENT_USER);
	Message(0,1,MSG(MRegTitle),MSG(MRegThanks),MSG(MOk));
}


void __cdecl CheckReg(void *Param)
{
	struct RegInfo *Reg=(struct RegInfo *)Param;
	char RegName[256],RegCode[256],RegData[256];
	DWORD Size=sizeof(RegData);
	SetRegRootKey(HKEY_CURRENT_USER);

	// в первую очередь читаем из HKCU
	if (!CheckRegKey(KeyRegistration))
	{
		// а потом из HKLM
		char SaveKey[512];
		strcpy(SaveKey,Opt.RegRoot);
		strcpy(Opt.RegRoot,"Software\\Far");
		SetRegRootKey(HKEY_LOCAL_MACHINE);
		Size=GetRegKey(KeyRegistration,"Data",(BYTE *)RegData,NULL,Size);
		SetRegRootKey(HKEY_CURRENT_USER);
		strcpy(Opt.RegRoot,SaveKey);
	}
	else
		Size=GetRegKey(KeyRegistration,"Data",(BYTE *)RegData,NULL,Size);

	memset(Reg,0,sizeof(*Reg));

	if (Size==0)
		RegVer=0;
	else
	{
		DWORD I;

		for (I=1; I<Size; I++)
			RegData[I]^=RegData[0];

		for (I=0; I<Size; I++)
			RegData[I]^=I+7;

		xstrncpy(RegName,RegData+3,RegData[1]);
		RegName[RegData[1]]=0;
		xstrncpy(RegCode,RegData+RegData[1]+3,RegData[2]);
		RegCode[RegData[2]]=0;
		RegVer=1;
		unsigned char Xor=17;

		for (I=0; RegName[I]!=0; I++)
			Xor^=RegName[I];

		if ((Xor & 0xf)!=ToHex(RegCode[0]) || ((~(Xor>>4))&0xf)!=ToHex(RegCode[3]))
			RegVer=0;

		if (strcmp(RegName,GetxUSSRRegName())==0)
			RegVer=3;

		strcpy(Reg->RegCode,RegCode);
		strcpy(Reg->RegName,RegName);
		strcpy(::RegName,RegName);
	}

	Reg->Done=TRUE;

	if (!RegistrationBugs)
		_endthread();
}


char ToHex(char Ch)
{
	if (Ch>='0' && Ch<='9')
		return(Ch-'0');

	if (Ch>='a' && Ch<='z')
		return(Ch-'a'+10);

	if (Ch>='A' && Ch<='Z')
		return(Ch-'A'+10);

	return(0);
}


#ifndef _MSC_VER
#pragma warn -par
#endif
void __cdecl ErrRegFn(void *Param)
{
	if (RegVer!=3)
	{
		Message(0,1,MSG(MRegTitle),MSG(MRegFailed),MSG(MOk));
		RegVer=0;
	}

	if (!RegistrationBugs)
		_endthread();
}
#ifndef _MSC_VER
#pragma warn +par
#endif
