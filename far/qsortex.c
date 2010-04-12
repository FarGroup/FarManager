/*
Copyright Prototronics, 1987
Totem Lake P.O. 8117
Kirkland, Washington 98034

(206) 820-1972

Licensed to Zortech. */
/*
Modified by Joe Huffman (d.b.a Prototronics) June 11, 1987 from Ray Gardner's,
(Denver, Colorado) public domain version. */

/*    qsort()  --  Quicksort function
**
**    Usage:   qsort(base, nbr_elements, width_bytes, compare_function);
**                char *base;
**                unsigned int nbr_elements, width_bytes;
**                int (*compare_function)();
**
**    Sorts an array starting at base, of length nbr_elements, each
**    element of size width_bytes, ordered via compare_function; which
**    is called as  (*compare_function)(ptr_to_element1, ptr_to_element2)
**    and returns < 0 if element1 < element2, 0 if element1 = element2,
**    > 0 if element1 > element2.  Most of the refinements are due to
**    R. Sedgewick.  See "Implementing Quicksort Programs", Comm. ACM,
**    Oct. 1978, and Corrigendum, Comm. ACM, June 1979.
*/

#include "headers.hpp"
#pragma hdrstop

static void iswap(int *a, int *b, size_t n_to_swap);       /* swap ints */
static void cswap(char *a, char *b, size_t n_to_swap);     /* swap chars */

//static unsigned int n_to_swap;  /* nbr of chars or ints to swap */
int _maxspan = 7;               /* subfiles of _maxspan or fewer elements */
/* will be sorted by a simple insertion sort */

/* Adjust _maxspan according to relative cost of a swap and a compare.  Reduce
_maxspan (not less than 1) if a swap is very expensive such as when you have
an array of large structures to be sorted, rather than an array of pointers to
structures.  The default value is optimized for a high cost for compares. */

#define SWAP(a,b) (*swap_fp)(a,b,n_to_swap)
#define COMPEX(a,b,u) (*comp_fp)(a,b,u)
#define COMP(a,b) (*comp_fp)(a,b)

typedef void (__cdecl *SWAP_FP)(void *, void *, size_t);

void __cdecl qsortex(char *base, size_t nel, size_t width,
                     int (__cdecl *comp_fp)(const void *, const void *,void*), void *user)
{
	char *stack[40], **sp;                 /* stack and stack pointer        */
	char *i, *j, *limit;                   /* scan and limit pointers        */
	size_t thresh;                         /* size of _maxspan elements in   */
	void (__cdecl  *swap_fp)(void *, void *, size_t);               /* bytes */
	size_t n_to_swap;

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

	thresh = _maxspan * width;             /* init threshold                 */
	sp = stack;                            /* init stack pointer             */
	limit = base + nel * width;            /* pointer past end of array      */

	while (1)                              /* repeat until done then return  */
	{
		while ((size_t)(limit - base) > thresh) /* if more than _maxspan elements */
		{
			/*swap middle, base*/
			SWAP(((size_t)(limit - base) >> 1) -
			     ((((size_t)(limit - base) >> 1)) % width) + base, base);
			i = base + width;                /* i scans from left to right     */
			j = limit - width;               /* j scans from right to left     */

			if (COMPEX(i, j,user) > 0)              /* Sedgewick's                    */
				SWAP(i, j);                    /*    three-element sort          */

			if (COMPEX(base, j,user) > 0)           /*        sets things up          */
				SWAP(base, j);                 /*            so that             */

			if (COMPEX(i, base,user) > 0)           /*              *i <= *base <= *j */
				SWAP(i, base);                 /* *base is the pivot element     */

			while (1)
			{
				do                            /* move i right until *i >= pivot */
					i += width;

				while (COMPEX(i, base,user) < 0);

				do                            /* move j left until *j <= pivot  */
					j -= width;

				while (COMPEX(j, base,user) > 0);

				if (i > j)                    /* break loop if pointers crossed */
					break;

				SWAP(i, j);                   /* else swap elements, keep scanning */
			}

			SWAP(base, j);                   /* move pivot into correct place  */

			if (j - base > limit - i)        /* if left subfile is larger...   */
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

			while (j > base && COMPEX(j - width, j,user) > 0)
			{
				SWAP(j - width, j);
				j -= width;
			}

			i += width;
		}

		if (sp > stack)    /* if any entries on stack...     */
		{
			sp -= 2;         /* pop the base and limit         */
			base = sp[0];
			limit = sp[1];
		}
		else              /* else stack empty, all done     */
			break;          /* Return. */
	}
}

static void iswap(int *a, int *b, size_t n_to_swap)   /* swap ints */
{
	int tmp;

	do
	{
		tmp = *a;
		*a = *b;
		*b = tmp;
		a++; b++;
	}
	while (--n_to_swap);
}

static void cswap(char *a, char *b, size_t n_to_swap)  /* swap chars */
{
	char tmp;

	do
	{
		tmp = *a;
		*a = *b;
		*b = tmp;
		a++; b++;
	}
	while (--n_to_swap);
}


#if 0
#if 0
void qsort(void *base, size_t nel, size_t width,
           int (__cdecl *comp_fp)(const void *, const void *))
{
	char *stack[40], **sp;                 /* stack and stack pointer        */
	char *i, *j, *limit;                   /* scan and limit pointers        */
	unsigned thresh;                       /* size of _maxspan elements in   */
	void (__cdecl *swap_fp)(void *, void *);       /* bytes */

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

	thresh = _maxspan * width;             /* init threshold                 */
	sp = stack;                            /* init stack pointer             */
	limit = (char*)base + nel * width;            /* pointer past end of array      */

	while (1)                              /* repeat until done then return  */
	{
		while (limit - base > thresh)        /* if more than _maxspan elements */
		{
			/*swap middle, base*/
			SWAP(((unsigned)(limit - base) >> 1) -
			     ((((unsigned)(limit - base) >> 1)) % width) + (char*)base, base);
			i = (char*)base + width;                /* i scans from left to right     */
			j = limit - width;               /* j scans from right to left     */

			if (COMP(i, j) > 0)              /* Sedgewick's                    */
				SWAP(i, j);                    /*    three-element sort          */

			if (COMP(base, j) > 0)           /*        sets things up          */
				SWAP(base, j);                 /*            so that             */

			if (COMP(i, base) > 0)           /*              *i <= *base <= *j */
				SWAP(i, base);                 /* *base is the pivot element     */

			while (1)
			{
				do                            /* move i right until *i >= pivot */
					i += width;

				while (COMP(i, base) < 0);

				do                            /* move j left until *j <= pivot  */
					j -= width;

				while (COMP(j, base) > 0);

				if (i > j)                    /* break loop if pointers crossed */
					break;

				SWAP(i, j);                   /* else swap elements, keep scanning */
			}

			SWAP(base, j);                   /* move pivot into correct place  */

			if (j - base > limit - i)        /* if left subfile is larger...   */
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
		i = (char*)base + width;

		while (i < limit)
		{
			j = i;

			while (j > base && COMP(j - width, j) > 0)
			{
				SWAP(j - width, j);
				j -= width;
			}

			i += width;
		}

		if (sp > stack)    /* if any entries on stack...     */
		{
			sp -= 2;         /* pop the base and limit         */
			base = sp[0];
			limit = sp[1];
		}
		else              /* else stack empty, all done     */
			break;          /* Return. */
	}
}
#else
#include <limits.h>
#define MAXSTACK (sizeof(size_t) * CHAR_BIT)

// exchange a,b
static void exchange(void *a, void *b, size_t size)
{
	size_t i;

	for (i = sizeof(int); i <= size; i += sizeof(int))
	{
		int t = *((int *)a);
		*(((int *)a)++) = *((int *)b);
		*(((int *)b)++) = t;
	}

	for (i = i - sizeof(int) + 1; i <= size; i++)
	{
		char t = *((char *)a);
		*(((char *)a)++) = *((char *)b);
		*(((char *)b)++) = t;
	}
}

void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *))
{
	void *lbStack[MAXSTACK], *ubStack[MAXSTACK];
	int sp;
	unsigned int offset;
	lbStack[0] = (char *)base;
	ubStack[0] = (char *)base + (nmemb-1)*size;

	for (sp = 0; sp >= 0; sp--)
	{
		char *lb, *ub, *m;
		char *P, *i, *j;
		lb = lbStack[sp];
		ub = ubStack[sp];

		while (lb < ub)
		{
			/* select pivot and exchange with 1st element */
			offset = (ub - lb) >> 1;
			P = lb + offset - offset % size;
			exchange(lb, P, size);
			/* partition into two segments */
			i = lb + size;
			j = ub;

			while (1)
			{
				while (i < j && compar(lb, i) > 0) i += size;

				while (j >= i && compar(j, lb) > 0) j -= size;

				if (i >= j) break;

				exchange(i, j, size);
				j -= size;
				i += size;
			}

			/* pivot belongs in A[j] */
			exchange(lb, j, size);
			m = j;

			/* keep processing smallest segment, and stack largest */
			if (m - lb <= ub - m)
			{
				if (m + size < ub)
				{
					lbStack[sp] = m + size;
					ubStack[sp++] = ub;
				}

				ub = m - size;
			}
			else
			{
				if (m - size > lb)
				{
					lbStack[sp] = lb;
					ubStack[sp++] = m - size;
				}

				lb = m + size;
			}
		}
	}
}
#endif
#endif
