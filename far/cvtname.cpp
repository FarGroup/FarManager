/*
cvtname.cpp

Функций для преобразования имен файлов/путей.

*/

#include "headers.hpp"
#pragma hdrstop

#include "plugin.hpp"
#include "lang.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"

/* $ 01.11.2000 OT
  Исправление логики. Теперь функция должна в обязательном порядке
  получить размер буфера и выдать длину полученного имени файла.
  Если размер буфера мал, то копирование не происходит
*/
void CharBufferTooSmallWarn(int BufSize, int FileNameSize)
{
	if (FileNameSize > MAX_PATH)
		Message(MSG_WARNING,1,MSG(MError),MSG(MFileNameExceedSystem),MSG(MOk));
	else
	{
		char Buf2 [80];
		sprintf(Buf2,MSG(MBuffSizeTooSmall_2), FileNameSize, BufSize);
		Message(MSG_WARNING,1,MSG(MError),MSG(MBuffSizeTooSmall_1),Buf2,MSG(MOk));
	}
}

/* $ 02.07.2001 IS
   Получение длинного имени на основе известного короткого. Медленно, зато с
   гарантией.
   src     - указатель на короткое имя
   dest    - сюда помещать длинное имя
   maxsize - размер dest. В dest будет скопировано не больше (maxsize-1)
             символов
   Возвращается число скопированных символов или 0. Если размер dest
   недостаточен, то возвращается требуемый размер.
   Примечание: разрешено перекрытие src и dest
*/
DWORD RawConvertShortNameToLongName(const char *src, char *dest, DWORD maxsize)
{
	if (!src || !dest)
		return 0;

	if (!*src)
	{
		*dest=0;
		return 1;
	}

	char BuffSrc[2048];
	char *NamePtr;
	*BuffSrc=0;

	if (!GetFullPathName(src,sizeof(BuffSrc)-1,BuffSrc,&NamePtr))
	{
		xstrncpy(BuffSrc,src,sizeof(BuffSrc)-1);
	}

	DWORD SrcSize=(DWORD)strlen(BuffSrc);

	if (SrcSize == 3 && BuffSrc[1] == ':' && (BuffSrc[2] == '\\' || BuffSrc[2] == '/'))
	{
		SrcSize=Min((DWORD)SrcSize,(DWORD)maxsize);
		memmove(dest,BuffSrc,SrcSize);
		dest[SrcSize]=0;
		*dest=toupper(*dest);
		return SrcSize;
	}

	DWORD DestSize=0, FinalSize=0, AddSize;
	BOOL Error=FALSE;
	char *Src, *Dest=0, *DestBuf=NULL,
	                             *SrcBuf=(char *)xf_malloc(Max((DWORD)sizeof(BuffSrc),(DWORD)SrcSize)+1);

	while (SrcBuf)
	{
		strcpy(SrcBuf, BuffSrc);
		Src=SrcBuf;
		WIN32_FIND_DATA wfd;
		char *Slash, *Dots=strchr(Src, ':');

		if (Dots)
		{
			++Dots;

			if ('\\'==*Dots) ++Dots;

			char tmp=*Dots;
			*Dots=0;
			AddSize=(DWORD)strlen(Src);
			FinalSize=AddSize;
			DestBuf=(char *)xf_malloc(AddSize+64);

			if (DestBuf)
			{
				DestSize=AddSize+64;
				Dest=DestBuf;
			}
			else
			{
				Error=TRUE;
				FinalSize=0;
				break;
			}

			strcpy(Dest, Src);
			Dest+=AddSize;
			*Dots=tmp;
			Src=Dots; // +1 ??? зачем ???
		}
		else if (Src[0]=='\\' && Src[1]=='\\')
		{
			Dots=Src+2;

			while (*Dots && '\\'!=*Dots) ++Dots;

			if ('\\'==*Dots)
				++Dots;
			else
			{
				SrcSize=Min((DWORD)SrcSize,(DWORD)maxsize);
				memmove(dest,BuffSrc,SrcSize);
				dest[SrcSize]=0;

				if (SrcBuf) xf_free(SrcBuf);

				return SrcSize;
			}

			while (*Dots && '\\'!=*Dots) ++Dots;

			if ('\\'==*Dots) ++Dots;

			char tmp=*Dots;
			*Dots=0;
			AddSize=(DWORD)strlen(Src);
			FinalSize=AddSize;
			DestBuf=(char *)xf_malloc(AddSize+64);

			if (DestBuf)
			{
				DestSize=AddSize+64;
				Dest=DestBuf;
			}
			else
			{
				Error=TRUE;
				FinalSize=0;
				break;
			}

			strcpy(Dest, Src);
			Dest+=AddSize;
			*Dots=tmp;
			Src=Dots;
		}

		/* $ 03.12.2001 DJ
		   если ничего не осталось - не пытаемся найти пустую строку
		*/
		while (!Error && *Src)  /* DJ $ */
		{
			Slash=strchr(Src, '\\');

			if (Slash) *Slash=0;

			if (GetFileWin32FindData(SrcBuf, &wfd,false))
			{
				AddSize=(DWORD)strlen(wfd.cFileName);
				FinalSize+=AddSize;

				if (FinalSize>=DestSize-1)
				{
					DestBuf=(char *)xf_realloc(DestBuf, FinalSize+64);

					if (DestBuf)
					{
						DestSize+=64;
						Dest=DestBuf+FinalSize-AddSize;
					}
					else
					{
						Error=TRUE;
						FinalSize=0;
						break;
					}
				}

				strcpy(Dest, wfd.cFileName);
				Dest+=AddSize;

				if (Slash)
				{
					*Dest=*Slash='\\';
					++Dest;
					/* $ 03.12.2001 DJ
					   если после слэша ничего нету - надо добавить '\0'
					*/
					*Dest = '\0';
					/* DJ $ */
					++FinalSize;
					++Slash;
					Slash=strchr(Src=Slash, '\\');
				}
				else
					break;
			}
			else
			{
				Error=TRUE;
				FinalSize=0;
				break;
			}
		}

		break;
	}

	if (!Error)
	{
		if (FinalSize<maxsize)
			strcpy(dest, DestBuf);
		else
		{
			*dest=0;
			++FinalSize;
		}
	}

	if (SrcBuf)  xf_free(SrcBuf);

	if (DestBuf) xf_free(DestBuf);

	return FinalSize;
}
/* IS $ */

int ConvertNameToFull(const char *Src,char *Dest, int DestSize)
{
	int Result = 0;
	char *FullName = (char *) xf_malloc(DestSize*2+1);

	if (!FullName)
		return Result;

	char *AnsiName = (char *) xf_malloc(DestSize*2+1);

	if (!AnsiName)
	{
		xf_free(FullName);
		return Result;
	}

	*FullName = 0;
	*AnsiName = 0;

	char *NamePtr=PointToName(const_cast<char *>(Src));
	Result+=(int)strlen(Src);

	if (NamePtr==Src && (NamePtr[0]!='.' || NamePtr[1]!=0))
	{
		Result+=FarGetCurDir(DestSize,FullName);
		Result+=AddEndSlash(FullName);

		if (Result < DestSize)
		{
			xstrncat(FullName,Src,DestSize-1);
			xstrncpy(Dest,FullName,DestSize-1);
		}
		else
		{
			CharBufferTooSmallWarn(DestSize,Result+1);
		}

		xf_free(AnsiName);
		xf_free(FullName);
		return Result;
	}

	if (PathMayBeAbsolute(Src)) //  (isalpha(Src[0]) && Src[1]==':' || Src[0]=='\\' && Src[1]=='\\') //????
	{
		if (*NamePtr &&
		        (*NamePtr!='.' || NamePtr[1]!=0 && (NamePtr[1]!='.' || NamePtr[2]!=0)) &&
		        (strstr(Src,"\\..\\")==NULL && strstr(Src,"\\.\\")==NULL)
		   )
		{
			if (Dest!=Src)
				xstrncpy(Dest,Src,DestSize-1);

			xf_free(AnsiName);
			xf_free(FullName);
			return Result;
		}
	}

	SetFileApisTo(APIS2ANSI);
	FAR_OemToCharBuff(Src,AnsiName,DestSize-1);

	if (GetFullPathName(AnsiName,DestSize,FullName,&NamePtr))
		FAR_CharToOemBuff(FullName,Dest,DestSize-1);
	else
		xstrncpy(Dest,Src,DestSize-1);

	// это когда ввели в масдае cd //host/share
	// а масдай выдал на гора c:\\host\share
	if (Src[0] == '/' && Src[1] == '/' && Dest[1] == ':' && Dest[3] == '\\')
		memmove(Dest,Dest+2,strlen(Dest+2)+1);

	SetFileApisTo(APIS2OEM);
	xf_free(AnsiName);
	xf_free(FullName);
	return Result;
}

/*
  Преобразует Src в полный РЕАЛЬНЫЙ путь с учетом reparse point в Win2K
  Если OS ниже, то вызывается обычный ConvertNameToFull()
*/
int WINAPI ConvertNameToReal(const char *Src,char *Dest, int DestSize, bool Internal)
{
	_SVS(CleverSysLog Clev("ConvertNameToReal()"));
	_SVS(SysLog("Params: Src='%s'",Src));
	char TempDest[2048];
	BOOL IsAddEndSlash=FALSE; // =TRUE, если слеш добавляли самостоятельно
	// в конце мы его того... удавим.
	// Получим сначала полный путь до объекта обычным способом
	int Ret=ConvertNameToFull(Src,TempDest,sizeof(TempDest));
	//RawConvertShortNameToLongName(TempDest,TempDest,sizeof(TempDest));
	_SVS(SysLog("ConvertNameToFull('%s') -> '%s'",Src,TempDest));
	/* $ 14.06.2003 IS
	   Для нелокальных дисков даже и не пытаемся анализировать симлинки
	*/

	// остальное касается Win2K, т.к. в виндах ниже рангом нету некоторых
	// функций, позволяющих узнать истинное имя линка.
	// также ничего не делаем для нелокальных дисков, т.к. для них невозможно узнать
	// корректную информацию про объект, на который указывает симлинк (т.е. невозможно
	// "разыменовать симлинк")
	if (IsLocalDrive(TempDest) &&
	        WinVer.dwPlatformId == VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5)
	{
		_SVS(CleverSysLog Clev("VER_PLATFORM_WIN32_NT && WinVer.dwMajorVersion >= 5"));
		DWORD FileAttr;
		char *Ptr, Chr;
		Ptr=TempDest+strlen(TempDest);
		_SVS(SysLog("%d FileAttr=0x%08X",__LINE__,GetFileAttributes(TempDest)));
		// немного интелектуальности не помешает - корректную инфу мы
		// можем получить только если каталог будет завершен слешем!

		//если не каталог - всё равно пробуем, т.к. ЭТО может быть файл-симлинк
		if ((FileAttr=GetFileAttributes(TempDest)) != INVALID_FILE_ATTRIBUTES)
		{
			if (Ptr[-1] != '\\')
			{
				AddEndSlash(TempDest);
				IsAddEndSlash=TRUE;
				++Ptr;
			}
		}

		char *CtrlChar = TempDest;

		if (strlen(TempDest) > 2 && TempDest[0]=='\\' && TempDest[1]=='\\')
			CtrlChar= strchr(TempDest+2, '\\');

		// обычный цикл прохода имени от корня
		while (CtrlChar)
		{
			while (Ptr > TempDest && *Ptr != '\\')
				--Ptr;

			/* $ 07.01.2003 IS
			   - ошибка: зачем-то обрабатывали путь "буква:" - он равен
			     текущему каталогу на диске "буква", что ведет к
			     непредсказуемым результатам
			*/

			// Если имя UNC, то работаем до имени сервера, не дальше...
			if (*Ptr != '\\' || Ptr == CtrlChar
			        // если дошли до "буква:", то тоже остановимся
			        || *(Ptr-1)==':')
				break;

			Chr=*Ptr;
			*Ptr=0;
			FileAttr=GetFileAttributes(TempDest);
			_SVS(SysLog("%d FileAttr=0x%08X ('%s')",__LINE__,FileAttr,TempDest));

			// О! Это наш клиент - одна из "компонент" пути - симлинк
			if (FileAttr != (DWORD)-1 && (FileAttr&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
			{
				char TempDest2[1024];
//        if(CheckParseJunction(TempDest,sizeof(TempDest)))
				{
					_SVS(SysLog("%d Parse Junction",__LINE__));

					// Получим инфу симлинке
					if (GetReparsePointInfo(TempDest,TempDest2,sizeof(TempDest2)))
					{
						int offset = 0;

						if (!strncmp(TempDest2,"\\??\\",4))
							offset = 4;

						// для случая монтированного диска (не имеющего букву)...
						if (!strnicmp(TempDest2+offset,"Volume{",7))
						{
							char JuncRoot[NM*2];
							JuncRoot[0]=JuncRoot[1]=0;
							// получим либо букву диска, либо...
							GetPathRootOne(TempDest2+offset,JuncRoot);
							// ...но в любом случае пишем полностью.
							// (поправка - если букву не получили - вернём точку монтирования)
							strcpy(TempDest2+offset,(JuncRoot[1]==':'||!Internal)?JuncRoot:TempDest);
						}

						// небольшая метаморфоза с именем, дабы удалить ведущие "\??\"
						// но для "Volume{" начало всегда будет корректным!
						memmove(TempDest2,TempDest2+offset,strlen(TempDest2+offset)+1);
						DeleteEndSlash(TempDest2);
						// Длина пути симлинка
						size_t tempLength = strlen(TempDest2);
						// Получаем длину левой и правой частей пути
						size_t leftLength = strlen(TempDest);
						size_t rightLength = strlen(Ptr + 1); // Измеряем длину пути начиная со следующего симовла после курсора
						*Ptr=Chr; // восстановим символ
						// Если путь симлинка больше левой части пути, увеличиваем буфер
						if (leftLength < tempLength)
						{
							;//TempDest = strTempDest.GetBuffer((int)(strTempDest.GetLength() + tempLength - leftLength));
						}

						// Так как мы производили манипуляции с левой частью пути изменяем указатель на
						// текущую позицию курсора в пути
						Ptr = TempDest + tempLength - 1;

						// Перемещаем правую часть пути на нужное место, только если левая чать отличается по
						// размеру от пути симлинка
						if (leftLength != tempLength)
						{
							// Копируемый буфер включает сам буфер, начальный '/', конечный '/' (если он есть) и '\0'
							memmove(TempDest + tempLength, TempDest + leftLength, rightLength + (IsAddEndSlash ? 3 : 2));
						}

						// Копируем путь к симлинку вначало пути
						memcpy(TempDest, TempDest2, tempLength);
						// Обновляем ссылку на маркер завершения прохождения по пути
						CtrlChar = TempDest;

						if (strlen(TempDest) > 2 && TempDest[0] == '\\' && TempDest[1] == '\\')
							CtrlChar = strchr(TempDest + 2, '\\');

						// Устанавливаем длину возвращаемой строки
						Ret = (int)strlen(TempDest);
						// Переходим к следующему шагу
						continue;
					}
				}
			}

			*Ptr=Chr;
			--Ptr;
		}
	}

	// Если не просили - удалим.
	if (IsAddEndSlash && DeleteEndSlash(TempDest))
		--Ret;

	if (Dest && DestSize)
		xstrncpy(Dest,TempDest,DestSize-1);

	_SVS(SysLog("return Dest='%s'",Dest));
	return Ret;
}

int WINAPI OldConvertNameToReal(const char *Src,char *Dest, int DestSize)
{
	return ConvertNameToReal(Src,Dest,DestSize,false);
}

void ConvertNameToShort(const char *Src,char *Dest,int DestSize)
{
	char *AnsiName=(char*)alloca(strlen(Src)+8);

	if (!AnsiName)
	{
		xstrncpy(Dest,Src,DestSize);
		return;
	}

	SetFileApisTo(APIS2ANSI);
	FAR_OemToChar(Src,AnsiName);
	char ShortName[NM];

	if (GetShortPathName(AnsiName,ShortName,sizeof(ShortName)))
		FAR_CharToOemBuff(ShortName,Dest,DestSize);
	else
		xstrncpy(Dest,Src,DestSize);

	SetFileApisTo(APIS2OEM);
	LocalUpperBuf(Dest,(int)strlen(Dest));
}
