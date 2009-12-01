/*
cmem.cpp

 CCCC   MM   MM  EEEEEE  MM   MM            v. 2.01
C    C  M M M M  E       M M M M    КОНТРОЛЬ  РАСПРЕДЕЛЕНИЯ
C       M  M  M  EEEE    M  M  M     ПАМЯТИ ДЛЯ РАЗРАБОТКИ
C    C  M     M  E       M     M    ОБЪЕКТНО-ОРИЕНТИРОВАННЫХ
 CCCC   M     M  EEEEEE  M     M           ПРИЛОЖЕНИЙ

Институт Проблем Управления ( л.55 )
Андрей К.Кельманзон

Использование:
--------------
      - скопируйте в рабочую директорию файл CMEM.CPP;
      - добавьте в проект файл CMEM.CPP;
      - по умолчанию программа работает со 100
        одновременно распределенными указателями,
        если Вам необходимо изменить это значение
        Options->Compiler->Code generation->Defines
        MAX_POINTERS=##;
      - после окончания работы вашей программы
        на диске будет создан файл CMEM.RPT,
        который состоит из "дерева" распределения
        памяти.
*/

#include "headers.hpp"
#pragma hdrstop

/* Блок по... */
#if defined(CMEM_INCLUDE) && defined(SYSLOG)

#ifndef ALLOC
static HANDLE FARHeapForNew=NULL;
#endif

#if     !defined(MAX_POINTERS)
#define MAX_POINTERS 8096       // Максимальное число вызовов NEW
#endif

#define MEM_TO0( __nBL )  memset( __nBL,   0, sizeof __nBL )

extern unsigned _stklen = 43210U;

// Описатели ////////////////////////////////////////////////////////
enum flag      { Off, On, End = -1 };
enum line_char { U='\xDA', D='\xC0', H='\xC4',
                 V='\xB3', C='\xC5', S=' '
               };

typedef void (*pvf)();

//static HANDLE FARHeapForNew=NULL;

void GetNameEXE(char name[])
{
	GetModuleFileName(NULL,name,512);
	char *ptr=strrchr(name,'.');

	if (ptr)
	{
		*ptr='\0';
		strcat(name,".rpt");
	}
}

// Описание блока памяти ////////////////////////////////////////////
struct HeapBlock
{
	void      *Addr;         // Адрес
	DWORD      SizeHOST,     // Размер - запрошенный
	SizeALLOC;    //        - распределенный
	flag       Status;       // Статус занятости
	int        NCall;        // Номер в time_call
	// Количество памяти --------------------------------------------
	DWORD      MemBefore,    // До распределения
	MemLater;     // После освобождения
};

// Класс, контролирующий и обрабатывающий ошибки распределения памяти
class ControlMem
{
	public:
		ControlMem(void);                  // Конструктор
		~ControlMem(void);                 // Деструктор

		int  search(void  *);          // Поиск адреса
		int  search_free(void);            // Поиск свободного блока
		// Перегруженные глобальные операторы распределения памяти ------
		friend void  *operator new(size_t);
		friend void   operator delete(void  *);
		friend void  *operator new[](size_t);
		friend void   operator delete[](void  *);

	private:
		struct HeapBlock HBL[ MAX_POINTERS ]; // Массив описателей блоков
		static flag      StatusClass; // Статус присутствия класса
		// On-да, Off-нет
		static flag      CloseBlock;  // Статус "закрыт ли предыдущий
		// блок" On-да, Off-нет
		FILE            *rpt;    // Указатель потока файла отчета
		static int       Meter;  // Счетчик вызовов new & delete
		line_char        ch;     // Символ линии

		flag    time_call[ MAX_POINTERS*2 ];  // Порядок вызовов
		// операций с блоками
		int     find_state(void);             // Возвращает номер
		// в time_call[]
		void    state_end(void);             // Выставляет признак конца
		// Обработка ошибок распределения памяти ------------------------
		pvf  old_handler;   // Указатель на старую функцию
		// обработки ошибок распределения
		// Функции формирования отчетов --------------------------------
		void open_r(void);       // Открытие
		void close_r(void);      // Закрытие
		void out_mem(int);       // Размер максимального блока
		void out_size(int);      // Размер выделеного блока
		void out_addr(int);      // Адресс блока
		void out_tab(int = -1);       // Отступ
		void out_space(void);    // Пробел
		void out_open(int, int = 0);      // Открытие блока
		void out_close(int, int = 0);     // Закрытие блока
		void out_unknown(void *);      // Неизвестный блок
		void out_heap(void);     // Выводит состояние динамически распределяемой
		// области памяти
};

// Инициализация static /////////////////////////////////////////////
flag ControlMem::StatusClass = Off;    // Класс обработки отсутствует
flag ControlMem::CloseBlock  = Off;    // Нет закрытых блоков
int  ControlMem::Meter       = 0;      // Счетчик вызовов

// Объявление класса обработки //////////////////////////////////////
ControlMem MemAlloc;

// Конструктор класса обработки /////////////////////////////////////
ControlMem::ControlMem(void)
{
	ControlMem::StatusClass = On;
	MEM_TO0(HBL);
	MEM_TO0(time_call);
	time_call[0] = End;   // End - Признак конца
	open_r();
}

// Открытие файла отчета ///////////////////////////////////////////
// Принцип работы:
//     в случае невозможности создания файла - stderr
//-------------------------------------------------------------------
void ControlMem::open_r(void)
{
	char n[512] = "";
	HANDLE hFile;
	GetNameEXE(n);

	if ((rpt = fopen(n, "wt")) == NULL)
		rpt = stderr;

	fprintf(rpt,
	        "+-----------------------------------------------------------+\n"
	        "|Control Memory Allocation                                  |\n"
	        "+-----------------------------------------------------------+\n\n"
	       );
}

// Закрытие файла рапорта ///////////////////////////////////////////
void ControlMem::close_r(void)
{
	if (rpt != stderr)
		fclose(rpt);
}

// Размер максимального свободного блока ////////////////////////////
void ControlMem::out_mem(int n)
{
	switch (HBL[ n ].Status)
	{
		case  On:
			fprintf(rpt, "[%lu]\n", (DWORD)HBL[ n ].MemBefore);
			break;
		case Off:
			fprintf(rpt, "[%lu]\n", (DWORD)HBL[ n ].MemLater);
			break;
	}
}

// Запрашиваемый и реально распределенный размеры блока "n" //////////
void ControlMem::out_size(int n)
{
	fprintf(rpt, "          Size: %lu(%lu)\n",
	        (DWORD)HBL[ n ].SizeHOST,
	        (DWORD)HBL[ n ].SizeALLOC
	       );
}

// Адрес блока //////////////////////////////////////////////////////
void ControlMem::out_addr(int n)
{
	fprintf(rpt, "%c%Fp{\n", ch=U, HBL[ n ].Addr);
}

// Отступ состоит из вертикальных линий в соответствии с time_call //
void ControlMem::out_tab(int d)
{
	int i=0;

	if (d != -1) d = HBL[ d ].NCall;

	while (time_call[ i ]!=End)
	{
		if (d==-1 || i<d)
			if (time_call[ i ]) ch = V;
			else ch = S;
		else if (i==d) ch = D;
		else if (i>d)
			if (time_call[ i ]) ch = C;
			else ch = H;

		putc(ch, rpt); i++;
	}
}

// Печатает пробел //////////////////////////////////////////////////
void ControlMem::out_space(void)
{
	putc(' ', rpt);
}

// Печатает информацию об открывающемся блоке //////////////////////
void ControlMem::out_open(int n,int typs)
{
	if (!ControlMem::CloseBlock)
	{
		out_tab(); out_space();

		if (typs) fprintf(rpt, " new[] ");
		else     fprintf(rpt, " new ");

		out_mem(n);
	}

	out_tab(); out_addr(n);
	HBL[ n ].NCall = MemAlloc.find_state();
	time_call[ HBL[ n ].NCall ] = On;
	out_tab(); out_size(n);
	fflush(rpt);
}

// Печатает адрес закрывающегося блока, который не открывался ///////
void ControlMem::out_unknown(void  *p)
{
	out_tab(); fprintf(rpt, "<%Fp>{?}\n", p);
	fflush(rpt);
}

// Закрывает блок "n" ///////////////////////////////////////////////
void ControlMem::out_close(int n, int typs)
{
	out_tab(n); putc('}', rpt);

	if (typs) fprintf(rpt, " delete[] ");
	else     fprintf(rpt, " delete ");

	out_mem(n);
	time_call[ HBL[ n ].NCall ] = Off;
	state_end();
	fflush(rpt);
}

// Информация о динамически распределяемой области памяти ///////////
void ControlMem::out_heap(void)
{
	struct heapinfo hi;
	int      i = 0;
	fprintf(rpt, "\nHeap:\n");
	fprintf(rpt, "+---+-------+------+\n");
	fprintf(rpt, "| # | Size  |Status|\n");
	fprintf(rpt, "+---+-------+------+\n");
	hi.ptr = NULL;

	while (heapwalk(&hi) == _HEAPOK)
		fprintf(rpt, "|%3d|%7lu| %4s |\n",
		        ++i,
		        (DWORD)hi.size,
		        hi.in_use ? "used" : "free"
		       );

	fprintf(rpt, "+---+-------+------+\n");

	if (heapcheck() == _HEAPCORRUPT)
		fprintf(rpt, "\nHeap is corrupted.\n");
	else
		fprintf(rpt, "\nHeap is OK.\n");

	fflush(rpt);
}

// Деструктор класса обработки //////////////////////////////////////
ControlMem::~ControlMem(void)
{
	ControlMem::StatusClass = Off;
	out_heap();
	close_r();
	//  set_new_handler( old_handler );
}

// Возвращает номер в time_call[] блока i ///////////////////////////
int ControlMem::find_state(void)
{
	int k=0, i=0;

	while (time_call[i]!=End)
		if (time_call[i++]==On) k = i;

	time_call[ k ] = Off; time_call[ k+1 ] = End;
	return k;
}

// Выставляет признак конца //
void ControlMem::state_end(void)
{
	int i=0, k;

	while (time_call[i++]!=End);

	k=(--i);

	while (i>=0)
	{
		switch (time_call[ i ])
		{
			case  On: return;
			case Off: time_call[ k ] = Off;
				time_call[ i ] = End;
				k=i;
		}

		i--;
	}
}

// Поиск информации о блоке /////////////////////////////////////////
// Передается:
//     p-адрес блока
// Возвращает:
//     номер блока (i) или -1, если он не найден
//-------------------------------------------------------------------
int ControlMem::search(void  *p)
{
	for (int i=0; i<MAX_POINTERS; i++)
		if (HBL[ i ].Addr == p)
			return i;

	return -1;
}

// Поиск свободного описателя ///////////////////////////////////////
int ControlMem::search_free(void)
{
	int i = 0;

	do
	{
		if (HBL[ i ].Status == Off)
			return i;
	}
	while (++i < MAX_POINTERS);

	return -1;
}


// Исходный оператор new ////////////////////////////////////////////
void  *o_NEW(size_t s)
{
	extern new_handler _new_handler;
	void  *p;
	s = s ? abs(s) : 1;

	while ((p = malloc(s)) == NULL && _new_handler != NULL)
		_new_handler();

	return p;
}

// Перегруженный оператор new ///////////////////////////////////////
// Принцип работы:
//   если класс обработки отсутствует, работает как обыкновенный new
//-------------------------------------------------------------------
#define coreleft()	0x7FFF8
void  *operator new(size_t size_block)
{
	int        i;
	void  *ptr;
	DWORD      mb = coreleft();
	ptr = o_NEW(size_block);

	if (ControlMem::StatusClass)
	{
		if ((i=MemAlloc.search_free()) == -1)
		{
			ControlMem::StatusClass = Off;
			MemAlloc.out_heap();
			MemAlloc.close_r();
			fprintf(stderr, "\nStack of pointers is full.\n");
			abort(); //exit(0);
		}

		if (!FARHeapForNew)
			FARHeapForNew=GetProcessHeap();

		MemAlloc.HBL[ i ].SizeHOST  = (DWORD)size_block;
		MemAlloc.HBL[ i ].SizeALLOC = HeapSize(FARHeapForNew,0,ptr);
		MemAlloc.HBL[ i ].Status    = On;
		MemAlloc.HBL[ i ].MemBefore = mb;
		MemAlloc.HBL[ i ].Addr      = ptr;
		MemAlloc.out_open(i);
	}

	ControlMem::CloseBlock = Off;
	return ptr;
}

void * operator new[](size_t size_block)
{
	int        i;
	void  *ptr;
	DWORD      mb = coreleft();
	ptr = o_NEW(size_block);

	if (ControlMem::StatusClass)
	{
		if ((i=MemAlloc.search_free()) == -1)
		{
			ControlMem::StatusClass = Off;
			MemAlloc.out_heap();
			MemAlloc.close_r();
			fprintf(stderr, "\nStack of pointers is full.\n");
			abort(); //exit(0);
		}

		if (!FARHeapForNew)
			FARHeapForNew=GetProcessHeap();

		MemAlloc.HBL[ i ].SizeHOST  = (DWORD)size_block;
		MemAlloc.HBL[ i ].SizeALLOC = HeapSize(FARHeapForNew,0,ptr);
		MemAlloc.HBL[ i ].Status    = On;
		MemAlloc.HBL[ i ].MemBefore = mb;
		MemAlloc.HBL[ i ].Addr      = ptr;
		MemAlloc.out_open(i ,1);
	}

	ControlMem::CloseBlock = Off;
	return ptr;
}

// Исходный оператор delete /////////////////////////////////////////
void o_DELETE(void  *ptr)
{
	if (ptr) free(ptr);
}

// Перегруженный оператор delete ////////////////////////////////////
// Принцип работы:
// если класс обработки отсутствует, работает как обыкновенный delete
//-------------------------------------------------------------------
void operator delete(void  *ptr)
{
	int        i;
	o_DELETE(ptr);

	if (ControlMem::StatusClass)
	{
		if ((i=MemAlloc.search(ptr)) == -1)
		{
			MemAlloc.out_unknown(ptr); return;
		}
		else
		{
			MemAlloc.HBL[ i ].MemLater = coreleft();
			MemAlloc.HBL[ i ].Status = Off;
			MemAlloc.out_close(i);
		}
	}

	ControlMem::CloseBlock = On;
}
void operator delete[](void *ptr)
{
	int        i;
	o_DELETE(ptr);

	if (ControlMem::StatusClass)
	{
		if ((i=MemAlloc.search(ptr)) == -1)
		{
			MemAlloc.out_unknown(ptr); return;
		}
		else
		{
			MemAlloc.HBL[ i ].MemLater = coreleft();
			MemAlloc.HBL[ i ].Status = Off;
			MemAlloc.out_close(i ,1);
		}
	}

	ControlMem::CloseBlock = On;
}


/*
VOID GlobalMemoryStatus(

    LPMEMORYSTATUS lpBuffer 	// pointer to the memory status structure
   );

typedef struct _MEMORYSTATUS { // mst
    DWORD dwLength;        // sizeof(MEMORYSTATUS)
    DWORD dwMemoryLoad;    // percent of memory in use
    DWORD dwTotalPhys;     // bytes of physical memory
    DWORD dwAvailPhys;     // free physical memory bytes
    DWORD dwTotalPageFile; // bytes of paging file
    DWORD dwAvailPageFile; // free bytes of paging file
    DWORD dwTotalVirtual;  // user bytes of address space
    DWORD dwAvailVirtual;  // free user bytes

} MEMORYSTATUS, *LPMEMORYSTATUS;
*/
/*
204800
1413120
*/

#else //у _меня_ - работает, но тестировать еще надо...

#ifdef ALLOC
/*
    ! Свершилось!!! Народ, rtfm - рулезЪ forever :-)))
      Скачал я таки стандарт по C++:
         ftp://ftp.ldz.lv/pub/doc/ansi_iso_iec_14882_1998.pdf (размер 2860601)
      А там все черным по белому... Короче, переопределил я new/delete как надо
      (если быть точным, то пару способов не осуществил, т.к. мы без исключений
      работаем), в крайнем случае, фар у _меня_ больше не грохается! Кому
      интересны подробности, смотрите в указанном стандарте параграф 18.4
*/
void *operator new(size_t size)
{
	extern new_handler _new_handler;
	void *p=NULL;
	size=size?size:1;

	while ((p=malloc(size))==NULL)
	{
		if (_new_handler!=NULL)_new_handler();
		else break;
	}

	return p;
}
void *operator new[](size_t size) {return ::operator new(size);}
void *operator new(size_t size, void *p) {return p;}
void operator delete(void *p) {free(p);}
void operator delete[](void *ptr) {::operator delete(ptr);}
#endif

#endif // CMEM_INCLUDE
