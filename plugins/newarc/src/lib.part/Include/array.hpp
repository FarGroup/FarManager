#pragma once

#if defined(_MSC_VER)
#pragma warning(disable:4127) // conditional expression is constant
#endif

typedef int (__cdecl *SORTFUNC) (const void *, const void *, void *);

#define ARRAY_OPTIONS_SKIP		0
#define ARRAY_OPTIONS_DELETE	1
#define ARRAY_OPTIONS_FREE 		2
#define ARRAY_OPTIONS_KEEP		4

#define DEFAULT_ARRAY_DELTA		 5
#define INVALID_INDEX			-1

#define pointer_array AutoPointerArray
#define array AutoArray

//type = data

template <typename type> class AutoArray {

protected:

	type *m_data;

	int m_nCount;
	int m_nAllocatedCount;

	int m_nDelta;

	int m_nOptions;

	bool m_bCreated;

public:

	AutoArray ();
	AutoArray (int options, int delta = DEFAULT_ARRAY_DELTA);

	virtual ~AutoArray ();

	void create (int options, int delta = DEFAULT_ARRAY_DELTA);
	void free ();

	type *add (const type& item);
	type *add ();

	bool remove (const type& item, bool freeitem = true);
	bool remove (int index, bool freeitem = true);
	bool remove (); //removes last item

	type& at (int index);
	int indexof (const type& item);

	type& operator [] (int index)  { return at(index); }

	int count() { return m_nCount; }

	void sort (void *SortFunc, void *Param = NULL);

	type *data() { return m_data; }

private:

	bool SetLimit (int limit);
	virtual void FreeItem (int index) { (void)index; }; //BUGBUG
};


template <typename type> class AutoPointerArray : public AutoArray<type> {
public:

	AutoPointerArray();
	AutoPointerArray(int itemsmode, int delta = DEFAULT_ARRAY_DELTA);

private:
	virtual void FreeItem (int index);
};


template <typename type>
AutoPointerArray<type>::AutoPointerArray () : AutoArray<type>()
{
}

template <typename type>
AutoPointerArray<type>::AutoPointerArray (int itemsmode, int delta)
{
	this->create (itemsmode, delta);
}



template <typename type>
void AutoPointerArray<type>::FreeItem (int index)
{
	if ( this->m_nOptions & ARRAY_OPTIONS_DELETE )
		delete this->m_data[index];

	if ( this->m_nOptions & ARRAY_OPTIONS_FREE )
		::free (this->m_data[index]);
}


template <typename type>
AutoArray<type>::AutoArray ()
{
	m_bCreated = false;

	m_nCount = 0;
	m_nAllocatedCount = 0;
	m_data = NULL;
}


template <typename type>
AutoArray<type>::AutoArray (int itemsmode, int delta)
{
	create (itemsmode, delta);
}

template <typename type>
AutoArray<type>::~AutoArray ()
{
	free ();
}

template <typename type>
void AutoArray<type>::create (int options, int delta)
{
	delta ? m_nDelta = delta : m_nDelta = DEFAULT_ARRAY_DELTA;

	m_nCount = 0;
	m_nAllocatedCount = 0;
	m_data = NULL;

	m_nOptions = options;
	m_bCreated = SetLimit (m_nDelta); //??
}



template <typename type>
void AutoArray<type>::free ()
{
	if ( m_bCreated )
	{
		for (int i = 0; i < m_nCount; i++)
			FreeItem (i);

		m_nCount = 0;
		m_nAllocatedCount = 0;

		if ( !(m_nOptions & ARRAY_OPTIONS_KEEP) )
			::free (m_data);

		m_data = NULL;
		m_bCreated = false;
	}
}


template <typename type>
type *AutoArray<type>::add (const type& item)
{
    bool bResult = true;

	if ( m_nAllocatedCount == m_nCount )
		bResult = SetLimit (m_nAllocatedCount+m_nDelta);

	if ( bResult )
	{
		memcpy (&m_data[m_nCount], &item, sizeof (type));

		type *pResult = &m_data[m_nCount];
		m_nCount++;

		return pResult;
	}

	return NULL;
}

template <typename type>
type* AutoArray<type>::add ()
{
	bool bResult = true;

	if ( m_nAllocatedCount == m_nCount )
		bResult = SetLimit (m_nAllocatedCount+m_nDelta);

	if ( bResult )
	{
		type *pResult = &m_data[m_nCount];
		m_nCount++;

		return pResult;
	}

	return NULL;
}



template <typename type>
bool AutoArray<type>::remove (const type& item, bool freeitem)
{
	int index = indexof (item);
	return remove(index, freeitem);
}

template <typename type>
bool AutoArray<type>::remove ()
{
	return remove(m_nCount-1);
}


template <typename type>
bool AutoArray<type>::remove (int index, bool freeitem)
{
	if ( (index >= 0) && (index < m_nCount) )
	{
		if ( freeitem )
			FreeItem (index);

		if ( index != (m_nCount-1) )
			memcpy (&m_data[index], &m_data[index+1], (m_nCount-index-1)*sizeof(type)); //??

		m_nCount--;

		if ( (m_nAllocatedCount-m_nCount) == m_nDelta )
			SetLimit (m_nAllocatedCount-m_nDelta); //shrink, so no error if can't realloc

		return true;
	}

	return false;
}

template <typename type>
int AutoArray<type>::indexof (const type& item)
{
	for (int i = 0; i < m_nCount; i++)
	{
		if ( !memcmp (&m_data[i], &item, sizeof(type)) )
			return i;
	}

	return INVALID_INDEX;
}

template <typename type>
type& AutoArray<type>::at (int index)
{
	if ( (index >= 0) && (index < m_nCount) )
		return m_data[index];

	static type result;

	memset (&result, 0, sizeof (type));
	return result; //???
}


template <typename type>
bool AutoArray<type>::SetLimit (int limit)
{
	type* newdata = (type*)realloc (m_data, limit*sizeof (type));

	if ( newdata || !limit )
	{
		m_data = newdata;

		if ( limit > m_nAllocatedCount )
			memset (&m_data[m_nAllocatedCount], 0, (limit-m_nAllocatedCount)*sizeof (type));

		m_nAllocatedCount = limit;

		return true;
	}

	return false;
}


struct sort_param {
	PVOID Param;
	SORTFUNC fcmp;
};

inline int __cdecl SortFunction (
		const void **p1,
		const void **p2,
		void *Param
		)
{
	sort_param *IP = (sort_param *)Param;
	return IP->fcmp (*p1, *p2, IP->Param);
}


static void iswap (int *a, int *b,unsigned int n_to_swap)  /* swap ints */
{
  int tmp;

  do
  {
    tmp = *a;
    *a = *b;
    *b = tmp;
    a++; b++;
  } while (--n_to_swap);
}

static void cswap (char *a, char *b,unsigned int n_to_swap)    /* swap chars */
{
  char tmp;
  do
  {
    tmp = *a;
    *a = *b;
    *b = tmp;
    a++; b++;
  } while (--n_to_swap);
}


/* Adjust _maxspan according to relative cost of a swap and a compare.  Reduce
_maxspan (not less than 1) if a swap is very expensive such as when you have
an array of large structures to be sorted, rather than an array of pointers to
structures.  The default value is optimized for a high cost for compares. */

#define SWAP(a,b) (*swap_fp)(a,b,n_to_swap)
#define COMPEX(a,b,u) (*comp_fp)(a,b,u)
#define COMP(a,b) (*comp_fp)(a,b)


typedef void (__cdecl *SWAP_FP) (void *, void *,unsigned int);


static void __cdecl qsortex (
		char *base,
		unsigned int nel,
		unsigned int width,
		int (__cdecl *comp_fp)(const void *, const void *,void*),
		void *user
		)
{
	char *stack[40], **sp;                 /* stack and stack pointer        */
	char *i, *j, *limit;                   /* scan and limit pointers        */

	unsigned thresh;                       /* size of _maxspan elements in   */

	void (__cdecl  *swap_fp) (void *, void *,unsigned int );      /* bytes */

	unsigned int n_to_swap;

	if ((width % sizeof(int)) != 0)
	{
		swap_fp = (SWAP_FP)cswap;
		n_to_swap = width;
	}
	else
	{
		swap_fp = (SWAP_FP)iswap;
		n_to_swap = width / sizeof(int);
	}

	thresh = 7 * width;             /* init threshold                 */

	sp = stack;                            /* init stack pointer             */
	limit = base + nel * width;            /* pointer past end of array      */

	while ( true )                              /* repeat until done then return  */
	{
		while ((unsigned)(limit - base) > thresh) /* if more than _maxspan elements */
		{
			/*swap middle, base*/

			SWAP (((unsigned)(limit - base) >> 1) - ((((unsigned)(limit - base) >> 1)) % width) + base, base);

			i = base + width;                /* i scans from left to right     */
			j = limit - width;               /* j scans from right to left     */

			if ( COMPEX(i, j,user) > 0 )            /* Sedgewick's                    */
				SWAP(i, j);                    /*    three-element sort          */

			if ( COMPEX(base, j,user) > 0 )         /*        sets things up          */
				SWAP(base, j);                 /*            so that             */

			if ( COMPEX(i, base,user) > 0 )         /*              *i <= *base <= *j */
				SWAP(i, base);                 /* *base is the pivot element     */

			while ( true )
			{
				do {                           /* move i right until *i >= pivot */
					i += width;
				} while (COMPEX (i, base,user) < 0);

				do {                           /* move j left until *j <= pivot  */
					j -= width;
				} while (COMPEX (j, base,user) > 0);

				if ( i > j )                    /* break loop if pointers crossed */
					break;

				SWAP (i, j);                  /* else swap elements, keep scanning */
			}

			SWAP (base, j);                  /* move pivot into correct place  */

			if ( j - base > limit - i )        /* if left subfile is larger...   */
			{
				sp[0] = base;                 /* stack left subfile base        */
				sp[1] = j;                    /*    and limit                   */
				base = i;                     /* sort the right subfile         */
			}
			else                             /* else right subfile is larger   */
			{
				sp[0] = i;                    /* stack right subfile base       */
				sp[1] = limit;                /*    and limit                   */
				limit = j;                    /* sort the left subfile          */
			}

			sp += 2;                        /* increment stack pointer        */
		}

	    /* Insertion sort on remaining subfile. */
		i = base + width;

		while (i < limit)
		{
			j = i;

			while ( j > base && COMPEX (j - width, j,user) > 0 )
			{
				SWAP (j - width, j);
				j -= width;
			}

			i += width;
		}

		if ( sp > stack )    /* if any entries on stack...     */
		{
			sp -= 2;         /* pop the base and limit         */
			base = sp[0];
			limit = sp[1];
		}
		else              /* else stack empty, all done     */
			break;          /* Return. */
	}
}



template <typename type>
void AutoArray<type>::sort (void *SortFunc, void *Param)
{
	sort_param IP;

	IP.Param = Param;
	IP.fcmp  = (SORTFUNC)SortFunc;

	qsortex ((char*)m_data, m_nCount, sizeof (type), (SORTFUNC)SortFunction, &IP);
}
