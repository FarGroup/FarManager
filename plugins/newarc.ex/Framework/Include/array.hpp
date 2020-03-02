#pragma once

#if defined(_MSC_VER)
#pragma warning(disable:4127) // conditional expression is constant
#endif

typedef int (__cdecl *SORTFUNC) (const void *, const void *, void *);

#define DEFAULT_ARRAY_DELTA		 5
#define INVALID_INDEX			(unsigned int)-1

template <typename type>
class Array {

protected:

	type *m_data;

	unsigned int m_uCount;
	unsigned int m_uAllocatedCount;

	unsigned int m_uDelta;
	bool m_bCreated;

public:

	Array(unsigned int delta = DEFAULT_ARRAY_DELTA);

	virtual ~Array ();

	void create(unsigned int delta = DEFAULT_ARRAY_DELTA);
	void free();
	void reset();

	type *add(const type& item);
	type *add();

	bool remove(const type& item, bool freeitem = true);
	bool remove(unsigned int index, bool freeitem = true);
	bool remove(); //removes last item

	type& at(unsigned int index) const;
	unsigned int indexof(const type& item) const;

	type& operator [] (unsigned int index) const { return at(index); }

	unsigned int count() const { return m_uCount; }

	void sort (void *SortFunc, void *Param = NULL);

	type *data() const { return m_data; }

protected:

	bool SetLimit (unsigned int limit);

	virtual void FreeItem(unsigned int index)
	{
		(void)index; //BUGBUG
	}

	virtual void FreeData()
	{
		::free(m_data);
		m_data = NULL;
	}
};


template <typename type>
Array<type>::Array (unsigned int delta)
{
	create(delta);
}


template <typename type>
Array<type>::~Array ()
{
	free ();
}

template <typename type>
void Array<type>::reset()
{
	free();
	create(m_uDelta);
}

template <typename type>
void Array<type>::create(unsigned int delta)
{
	delta ? m_uDelta = delta : m_uDelta = DEFAULT_ARRAY_DELTA;

	m_uCount = 0;
	m_uAllocatedCount = 0;
	m_data = NULL;

	m_bCreated = SetLimit(m_uDelta); //??
}



template <typename type>
void Array<type>::free()
{
	if ( m_bCreated )
	{
		for (unsigned int i = 0; i < m_uCount; i++)
			FreeItem (i);

		m_uCount = 0;
		m_uAllocatedCount = 0;

		FreeData();

		m_bCreated = false;
	}
}


template <typename type>
type *Array<type>::add(const type& item)
{
    bool bResult = true;

	if ( m_uAllocatedCount == m_uCount )
		bResult = SetLimit(m_uAllocatedCount+m_uDelta);

	if ( bResult )
	{
		memcpy (&m_data[m_uCount], &item, sizeof (type));

		type *pResult = &m_data[m_uCount];
		m_uCount++;

		return pResult;
	}

	return NULL;
}

template <typename type>
type* Array<type>::add()
{
	bool bResult = true;

	if ( m_uAllocatedCount == m_uCount )
		bResult = SetLimit(m_uAllocatedCount+m_uDelta);

	if ( bResult )
	{
		type *pResult = &m_data[m_uCount];
		m_uCount++;

		return pResult;
	}

	return NULL;
}



template <typename type>
bool Array<type>::remove(const type& item, bool freeitem)
{
	unsigned int index = indexof(item);
	return remove(index, freeitem);
}

template <typename type>
bool Array<type>::remove()
{
	return remove(m_uCount-1);
}


template <typename type>
bool Array<type>::remove(unsigned int index, bool freeitem)
{
	if ( (index < m_uCount) && (index != INVALID_INDEX) )
	{
		if ( freeitem )
			FreeItem(index);

		if ( index != (m_uCount-1) )
			memcpy(&m_data[index], &m_data[index+1], (m_uCount-index-1)*sizeof(type)); //??

		m_uCount--;

		if ( (m_uAllocatedCount-m_uCount) == m_uDelta )
			SetLimit(m_uAllocatedCount-m_uDelta); //shrink, so no error if can't realloc

		return true;
	}

	return false;
}

template <typename type>
unsigned int Array<type>::indexof(const type& item) const
{
	for (unsigned int i = 0; i < m_uCount; i++)
	{
		if ( !memcmp(&m_data[i], &item, sizeof(type)) )
			return i;
	}

	return INVALID_INDEX;
}

template <typename type>
type& Array<type>::at(unsigned int index) const
{
	if ( index < m_uCount )
		return m_data[index];

	static type result;

	memset(&result, 0, sizeof (type));
	return result; //???
}


template <typename type>
bool Array<type>::SetLimit(unsigned int limit)
{
	type* newdata = (type*)realloc(m_data, limit*sizeof(type));

	if ( newdata || !limit )
	{
		m_data = newdata;

		if ( limit > m_uAllocatedCount )
			memset (&m_data[m_uAllocatedCount], 0, (limit-m_uAllocatedCount)*sizeof(type));

		m_uAllocatedCount = limit;

		return true;
	}

	return false;
}


struct sort_param {
	PVOID Param;
	SORTFUNC fcmp;
};

inline int __cdecl SortFunction(
		const void **p1,
		const void **p2,
		void *Param
		)
{
	sort_param *IP = (sort_param *)Param;
	return IP->fcmp(*p1, *p2, IP->Param);
}



static void iswap (int *a, int *b, unsigned int n_to_swap)  /* swap ints */
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

static void cswap (char *a, char *b, unsigned int n_to_swap)    /* swap chars */
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
void Array<type>::sort(void* SortFunc, void* Param)
{
	sort_param IP;

	IP.Param = Param;
	IP.fcmp  = (SORTFUNC)SortFunc;

	qsortex((char*)m_data, m_uCount, sizeof(type), (SORTFUNC)SortFunction, &IP);
}


///////////////////////
template <typename type>
class PointerArray : public Array<type> {

public:

	PointerArray(unsigned int delta = DEFAULT_ARRAY_DELTA) : Array<type>(delta) { };
	virtual ~PointerArray() { this->free(); }

protected:

	virtual void FreeItem(unsigned int index)
	{
		::free(this->m_data[index]);
	}
};

/////////////////////////////////////

template <typename type>
class ObjectArray : public Array<type> {
public:

	ObjectArray(unsigned int delta = DEFAULT_ARRAY_DELTA) : Array<type>(delta) { };
	virtual ~ObjectArray() { this->free(); }

protected:

	virtual void FreeItem(unsigned int index)
	{
		delete this->m_data[index];
	}
};

//////////////////////////////////////
template <typename type>
class ConstArray : public Array<type> {
public:

	ConstArray(unsigned int delta = DEFAULT_ARRAY_DELTA) : Array<type>(delta) { };
	virtual ~ConstArray() { this->free(); }; //or virtual methods will be called wrong

protected:

	virtual void FreeData() { }; //keep data
};
