void MakeCryptPassword(LPCSTR Src,BYTE Dest[FTP_PWD_LEN])
{
	BYTE    *CurDest;
	clock_t  Random = clock();
	BYTE     XorMask;
	int      Done;
	//Log(( "MakeCryptPassword: [%s]",Src ));
	memset(Dest,0,sizeof(Dest));

	if(!Src || !Src[0])
		return;

	for(int I=0; I < (int)sizeof(Dest); I++)
	{
		CurDest = Dest;
		CurDest[0] = (BYTE)((Random+I)|0x80);
		CurDest[1] = (BYTE)((Random>>8)|0x80);
		strcpy(((char*)CurDest)+2,Src);
		XorMask  = (CurDest[0]^CurDest[1])|80;
		CurDest += 2;
		Done     = TRUE;

		while(*CurDest)
		{
			*CurDest^=XorMask;

			if(*CurDest == 0)
			{
				Done=FALSE;
				break;
			}

			CurDest++;
		}

		if(Done) break;
	}

	//Log(( "MakeCryptPassword: %02X %02X %02X %02X %02X %02X",Dest[0],Dest[1],Dest[2],Dest[3],Dest[4],Dest[5] ));
}

void DecryptPassword(BYTE Src[FTP_PWD_LEN],char *_Dest)
{
	BYTE XorMask = (Src[0]^Src[1]) | 80;
	int  n;
	char *Dest = _Dest;
	//Log(( "DecryptPassword: %02X %02X %02X %02X %02X %02X",Src[0],Src[1],Src[2],Src[3],Src[4],Src[5] ));

	if(Src[0] && Src[1] && Src[2])
		for(n = 2; n < FTP_PWD_LEN; n++,Dest++)
		{
			*Dest = Src[n] ^ XorMask;

			if(*Dest == 0 ||
			        *Dest == XorMask)
				break;
		}

	*Dest = 0;
	//Log(( "DecryptPassword: [%s]",_Dest ));
}
