/*
qsort.c

To implement the qsort() routine for sorting arrays.

*/

#include "headers.hpp"
#pragma hdrstop

#if defined(__BORLANDC__)
#pragma intrinsic memcpy
#else
#pragma intrinsic (memcpy)
#endif

void __cdecl qsort_b(
    void *base,
    size_t num,
    size_t width,
    int (__cdecl *comp)(const void *, const void *));

void __cdecl qsort_m(
    void *base,
    size_t num,
    size_t width,
    int (__cdecl *comp)(const void *, const void *));

void __cdecl far_qsort(
    void *base,
    size_t num,
    size_t width,
    int (__cdecl *comp)(const void *, const void *)
)
{
	if (width >=32) qsort_m(base, num, width, comp);
	else qsort_b(base, num, width, comp);
}

/* prototypes for local routines */
static void  shortsort(char *lo, char *hi, size_t width,
                       int (__cdecl *comp)(const void *, const void *));
static void  swap(char *p, char *q, size_t width);

/* this parameter defines the cutoff between using quick sort and
   insertion sort for arrays; arrays with lengths shorter or equal to the
   below value use insertion sort */

#define CUTOFF 8            /* testing shows that this is good value */

/***
*qsort(base, num, wid, comp) - quicksort function for sorting arrays
*
*Purpose:
*       quicksort the array of elements
*       side effects:  sorts in place
*       maximum array size is number of elements times size of elements,
*       but is limited by the virtual address space of the processor
*
*Entry:
*       char *base = pointer to base of array
*       size_t num  = number of elements in the array
*       size_t width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

/* sort the array between lo and hi (inclusive) */

#define STKSIZ (8*sizeof(void*) - 2)

void __cdecl qsort_b(
    void *base,
    size_t num,
    size_t width,
    int (__cdecl *comp)(const void *, const void *)
)
{
	/* Note: the number of stack entries required is no more than
	   1 + log2(num), so 30 is sufficient for any array */
	char *lo, *hi;              /* ends of sub-array currently sorting */
	char *mid;                  /* points to middle of subarray */
	char *loguy, *higuy;        /* traveling pointers for partition step */
	size_t size;                /* size of the sub-array */
	char *lostk[STKSIZ], *histk[STKSIZ];
	int stkptr;                 /* stack for saving sub-array to be processed */

	if (num < 2 || width == 0)
		return;                 /* nothing to do */

	stkptr = 0;                 /* initialize stack */
	lo = base;
	hi = (char *)base + width * (num-1);        /* initialize limits */
	/* this entry point is for pseudo-recursion calling: setting
	   lo and hi and jumping to here is like recursion, but stkptr is
	   preserved, locals aren't, so we preserve stuff on the stack */
recurse:
	size = (hi - lo) / width + 1;        /* number of el's to sort */

	/* below a certain size, it is faster to use a O(n^2) sorting method */
	if (size <= CUTOFF)
	{
		shortsort(lo, hi, width, comp);
	}
	else
	{
		/* First we pick a partitioning element.  The efficiency of the
		   algorithm demands that we find one that is approximately the median
		   of the values, but also that we select one fast.  We choose the
		   median of the first, middle, and last elements, to avoid bad
		   performance in the face of already sorted data, or data that is made
		   up of multiple sorted runs appended together.  Testing shows that a
		   median-of-three algorithm provides better performance than simply
		   picking the middle element for the latter case. */
		mid = lo + (size / 2) * width;      /* find middle element */

		/* Sort the first, middle, last elements into order */
		if (comp(lo, mid) > 0)
		{
			swap(lo, mid, width);
		}

		if (comp(lo, hi) > 0)
		{
			swap(lo, hi, width);
		}

		if (comp(mid, hi) > 0)
		{
			swap(mid, hi, width);
		}

		/* We now wish to partition the array into three pieces, one consisting
		   of elements <= partition element, one of elements equal to the
		   partition element, and one of elements > than it.  This is done
		   below; comments indicate conditions established at every step. */
		loguy = lo;
		higuy = hi;

		/* Note that higuy decreases and loguy increases on every iteration,
		   so loop must terminate. */
		for (;;)
		{
			/* lo <= loguy < hi, lo < higuy <= hi,
			   A[i] <= A[mid] for lo <= i <= loguy,
			   A[i] > A[mid] for higuy <= i < hi,
			   A[hi] >= A[mid] */
			/* The doubled loop is to avoid calling comp(mid,mid), since some
			   existing comparison funcs don't work when passed the same
			   value for both pointers. */
			if (mid > loguy)
			{
				do
				{
					loguy += width;
				}
				while (loguy < mid && comp(loguy, mid) <= 0);
			}

			if (mid <= loguy)
			{
				do
				{
					loguy += width;
				}
				while (loguy <= hi && comp(loguy, mid) <= 0);
			}

			/* lo < loguy <= hi+1, A[i] <= A[mid] for lo <= i < loguy,
			   either loguy > hi or A[loguy] > A[mid] */

			do
			{
				higuy -= width;
			}
			while (higuy > mid && comp(higuy, mid) > 0);

			/* lo <= higuy < hi, A[i] > A[mid] for higuy < i < hi,
			   either higuy == lo or A[higuy] <= A[mid] */

			if (higuy < loguy)
				break;

			/* if loguy > hi or higuy == lo, then we would have exited, so
			   A[loguy] > A[mid], A[higuy] <= A[mid],
			   loguy <= hi, higuy > lo */
			swap(loguy, higuy, width);
			/* If the partition element was moved, follow it.  Only need
			   to check for mid == higuy, since before the swap,
			   A[loguy] > A[mid] implies loguy != mid. */

			if (mid == higuy)
				mid = loguy;

			/* A[loguy] <= A[mid], A[higuy] > A[mid]; so condition at top
			   of loop is re-established */
		}

		/*     A[i] <= A[mid] for lo <= i < loguy,
		       A[i] > A[mid] for higuy < i < hi,
		       A[hi] >= A[mid]
		       higuy < loguy
		   implying:
		       higuy == loguy-1
		       or higuy == hi - 1, loguy == hi + 1, A[hi] == A[mid] */
		/* Find adjacent elements equal to the partition element.  The
		   doubled loop is to avoid calling comp(mid,mid), since some
		   existing comparison funcs don't work when passed the same value
		   for both pointers. */
		higuy += width;

		if (mid < higuy)
		{
			do
			{
				higuy -= width;
			}
			while (higuy > mid && comp(higuy, mid) == 0);
		}

		if (mid >= higuy)
		{
			do
			{
				higuy -= width;
			}
			while (higuy > lo && comp(higuy, mid) == 0);
		}

		/* OK, now we have the following:
		      higuy < loguy
		      lo <= higuy <= hi
		      A[i]  <= A[mid] for lo <= i <= higuy
		      A[i]  == A[mid] for higuy < i < loguy
		      A[i]  >  A[mid] for loguy <= i < hi
		      A[hi] >= A[mid] */
		/* We've finished the partition, now we want to sort the subarrays
		   [lo, higuy] and [loguy, hi].
		   We do the smaller one first to minimize stack usage.
		   We only sort arrays of length 2 or more.*/

		if (higuy - lo >= hi - loguy)
		{
			if (lo < higuy)
			{
				lostk[stkptr] = lo;
				histk[stkptr] = higuy;
				++stkptr;
			}                           /* save big recursion for later */

			if (loguy < hi)
			{
				lo = loguy;
				goto recurse;           /* do small recursion */
			}
		}
		else
		{
			if (loguy < hi)
			{
				lostk[stkptr] = loguy;
				histk[stkptr] = hi;
				++stkptr;               /* save big recursion for later */
			}

			if (lo < higuy)
			{
				hi = higuy;
				goto recurse;           /* do small recursion */
			}
		}
	}

	/* We have sorted the array, except for any pending sorts on the stack.
	   Check if there are any, and do them. */
	--stkptr;

	if (stkptr >= 0)
	{
		lo = lostk[stkptr];
		hi = histk[stkptr];
		goto recurse;           /* pop subarray from stack */
	}
	else
		return;                 /* all subarrays done */
}


/***
*shortsort(hi, lo, width, comp) - insertion sort for sorting short arrays
*
*Purpose:
*       sorts the sub-array of elements between lo and hi (inclusive)
*       side effects:  sorts in place
*       assumes that lo < hi
*
*Entry:
*       char *lo = pointer to low element to sort
*       char *hi = pointer to high element to sort
*       size_t width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void  shortsort(
    char *lo,
    char *hi,
    size_t width,
    int (__cdecl *comp)(const void *, const void *)
)
{
	char *p, *max;
	/* Note: in assertions below, i and j are alway inside original bound of
	   array to sort. */

	while (hi > lo)
	{
		/* A[i] <= A[j] for i <= j, j > hi */
		max = lo;

		for (p = lo+width; p <= hi; p += width)
		{
			/* A[i] <= A[max] for lo <= i < p */
			if (comp(p, max) > 0)
			{
				max = p;
			}

			/* A[i] <= A[max] for lo <= i <= p */
		}

		/* A[i] <= A[max] for lo <= i <= hi */
		swap(max, hi, width);
		/* A[i] <= A[hi] for i <= hi, so A[i] <= A[j] for i <= j, j >= hi */
		hi -= width;
		/* A[i] <= A[j] for i <= j, j > hi, loop top condition established */
	}

	/* A[i] <= A[j] for i <= j, j > lo, which implies A[i] <= A[j] for i < j,
	   so array is sorted */
}


/***
*swap(a, b, width) - swap two elements
*
*Purpose:
*       swaps the two array elements of size width
*
*Entry:
*       char *a, *b = pointer to two elements to swap
*       size_t width = width in bytes of each array element
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void  swap(
    char *a,
    char *b,
    size_t width
)
{
	char tmp;

	if (a != b)
		while (width--)
		{
			tmp = *a;
			*a++ = *b;
			*b++ = tmp;
		}
}

/* Always compile this module for speed, not size */
/* prototypes for local routines */
static void  shortsort_m(char *lo, char *hi, size_t width,
                         int (__cdecl *comp)(const void *, const void *), void* t);
static void  swap_m(char *p, char *q, size_t width, void* t);

/* this parameter defines the cutoff between using quick sort and
   insertion sort for arrays; arrays with lengths shorter or equal to the
   below value use insertion sort */

void __cdecl qsort_m(
    void *base,
    size_t num,
    size_t width,
    int (__cdecl *comp)(const void *, const void *)
)
{
	/* Note: the number of stack entries required is no more than
	   1 + log2(num), so 30 is sufficient for any array */
	char *lo, *hi;              /* ends of sub-array currently sorting */
	char *mid;                  /* points to middle of subarray */
	char *loguy, *higuy;        /* traveling pointers for partition step */
	size_t size;                /* size of the sub-array */
	char *lostk[STKSIZ], *histk[STKSIZ];
	char* t = (char*)alloca(width);
	int stkptr;                 /* stack for saving sub-array to be processed */

	if (num < 2 || width == 0)
		return;                 /* nothing to do */

	stkptr = 0;                 /* initialize stack */
	lo = base;
	hi = (char *)base + width * (num-1);        /* initialize limits */
	/* this entry point is for pseudo-recursion calling: setting
	   lo and hi and jumping to here is like recursion, but stkptr is
	   preserved, locals aren't, so we preserve stuff on the stack */
recurse:
	size = (hi - lo) / width + 1;        /* number of el's to sort */

	/* below a certain size, it is faster to use a O(n^2) sorting method */
	if (size <= CUTOFF)
	{
		shortsort_m(lo, hi, width, comp, t);
	}
	else
	{
		/* First we pick a partitioning element.  The efficiency of the
		   algorithm demands that we find one that is approximately the median
		   of the values, but also that we select one fast.  We choose the
		   median of the first, middle, and last elements, to avoid bad
		   performance in the face of already sorted data, or data that is made
		   up of multiple sorted runs appended together.  Testing shows that a
		   median-of-three algorithm provides better performance than simply
		   picking the middle element for the latter case. */
		mid = lo + (size / 2) * width;      /* find middle element */

		/* Sort the first, middle, last elements into order */
		if (comp(lo, mid) > 0)
		{
			swap_m(lo, mid, width, t);
		}

		if (comp(lo, hi) > 0)
		{
			swap_m(lo, hi, width, t);
		}

		if (comp(mid, hi) > 0)
		{
			swap_m(mid, hi, width, t);
		}

		/* We now wish to partition the array into three pieces, one consisting
		   of elements <= partition element, one of elements equal to the
		   partition element, and one of elements > than it.  This is done
		   below; comments indicate conditions established at every step. */
		loguy = lo;
		higuy = hi;

		/* Note that higuy decreases and loguy increases on every iteration,
		   so loop must terminate. */
		for (;;)
		{
			/* lo <= loguy < hi, lo < higuy <= hi,
			   A[i] <= A[mid] for lo <= i <= loguy,
			   A[i] > A[mid] for higuy <= i < hi,
			   A[hi] >= A[mid] */
			/* The doubled loop is to avoid calling comp(mid,mid), since some
			   existing comparison funcs don't work when passed the same
			   value for both pointers. */
			if (mid > loguy)
			{
				do
				{
					loguy += width;
				}
				while (loguy < mid && comp(loguy, mid) <= 0);
			}

			if (mid <= loguy)
			{
				do
				{
					loguy += width;
				}
				while (loguy <= hi && comp(loguy, mid) <= 0);
			}

			/* lo < loguy <= hi+1, A[i] <= A[mid] for lo <= i < loguy,
			   either loguy > hi or A[loguy] > A[mid] */

			do
			{
				higuy -= width;
			}
			while (higuy > mid && comp(higuy, mid) > 0);

			/* lo <= higuy < hi, A[i] > A[mid] for higuy < i < hi,
			   either higuy == lo or A[higuy] <= A[mid] */

			if (higuy < loguy)
				break;

			/* if loguy > hi or higuy == lo, then we would have exited, so
			   A[loguy] > A[mid], A[higuy] <= A[mid],
			   loguy <= hi, higuy > lo */
			swap_m(loguy, higuy, width, t);
			/* If the partition element was moved, follow it.  Only need
			   to check for mid == higuy, since before the swap,
			   A[loguy] > A[mid] implies loguy != mid. */

			if (mid == higuy)
				mid = loguy;

			/* A[loguy] <= A[mid], A[higuy] > A[mid]; so condition at top
			   of loop is re-established */
		}

		/*     A[i] <= A[mid] for lo <= i < loguy,
		       A[i] > A[mid] for higuy < i < hi,
		       A[hi] >= A[mid]
		       higuy < loguy
		   implying:
		       higuy == loguy-1
		       or higuy == hi - 1, loguy == hi + 1, A[hi] == A[mid] */
		/* Find adjacent elements equal to the partition element.  The
		   doubled loop is to avoid calling comp(mid,mid), since some
		   existing comparison funcs don't work when passed the same value
		   for both pointers. */
		higuy += width;

		if (mid < higuy)
		{
			do
			{
				higuy -= width;
			}
			while (higuy > mid && comp(higuy, mid) == 0);
		}

		if (mid >= higuy)
		{
			do
			{
				higuy -= width;
			}
			while (higuy > lo && comp(higuy, mid) == 0);
		}

		/* OK, now we have the following:
		      higuy < loguy
		      lo <= higuy <= hi
		      A[i]  <= A[mid] for lo <= i <= higuy
		      A[i]  == A[mid] for higuy < i < loguy
		      A[i]  >  A[mid] for loguy <= i < hi
		      A[hi] >= A[mid] */
		/* We've finished the partition, now we want to sort the subarrays
		   [lo, higuy] and [loguy, hi].
		   We do the smaller one first to minimize stack usage.
		   We only sort arrays of length 2 or more.*/

		if (higuy - lo >= hi - loguy)
		{
			if (lo < higuy)
			{
				lostk[stkptr] = lo;
				histk[stkptr] = higuy;
				++stkptr;
			}                           /* save big recursion for later */

			if (loguy < hi)
			{
				lo = loguy;
				goto recurse;           /* do small recursion */
			}
		}
		else
		{
			if (loguy < hi)
			{
				lostk[stkptr] = loguy;
				histk[stkptr] = hi;
				++stkptr;               /* save big recursion for later */
			}

			if (lo < higuy)
			{
				hi = higuy;
				goto recurse;           /* do small recursion */
			}
		}
	}

	/* We have sorted the array, except for any pending sorts on the stack.
	   Check if there are any, and do them. */
	--stkptr;

	if (stkptr >= 0)
	{
		lo = lostk[stkptr];
		hi = histk[stkptr];
		goto recurse;           /* pop subarray from stack */
	}
	else
	{
		return;                 /* all subarrays done */
	}
}


/***
*shortsort_m(hi, lo, width, comp) - insertion sort for sorting short arrays
*
*Purpose:
*       sorts the sub-array of elements between lo and hi (inclusive)
*       side effects:  sorts in place
*       assumes that lo < hi
*
*Entry:
*       char *lo = pointer to low element to sort
*       char *hi = pointer to high element to sort
*       size_t width = width in bytes of each array element
*       int (*comp)() = pointer to function returning analog of strcmp for
*               strings, but supplied by user for comparing the array elements.
*               it accepts 2 pointers to elements and returns neg if 1<2, 0 if
*               1=2, pos if 1>2.
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void  shortsort_m(
    char *lo,
    char *hi,
    size_t width,
    int (__cdecl *comp)(const void *, const void *),
    void* t
)
{
	char *p, *ptrmax;
	/* Note: in assertions below, i and j are alway inside original bound of
	   array to sort. */

	while (hi > lo)
	{
		/* A[i] <= A[j] for i <= j, j > hi */
		ptrmax = lo;

		for (p = lo+width; p <= hi; p += width)
		{
			/* A[i] <= A[ptrmax] for lo <= i < p */
			if (comp(p, ptrmax) > 0)
			{
				ptrmax = p;
			}

			/* A[i] <= A[ptrmax] for lo <= i <= p */
		}

		/* A[i] <= A[ptrmax] for lo <= i <= hi */
		swap_m(ptrmax, hi, width, t);
		/* A[i] <= A[hi] for i <= hi, so A[i] <= A[j] for i <= j, j >= hi */
		hi -= width;
		/* A[i] <= A[j] for i <= j, j > hi, loop top condition established */
	}

	/* A[i] <= A[j] for i <= j, j > lo, which implies A[i] <= A[j] for i < j,
	   so array is sorted */
}


/***
*swap(a, b, width) - swap two elements
*
*Purpose:
*       swaps the two array elements of size width
*
*Entry:
*       char *a, *b = pointer to two elements to swap
*       size_t width = width in bytes of each array element
*
*Exit:
*       returns void
*
*Exceptions:
*
*******************************************************************************/

static void  swap_m(
    char *a,
    char *b,
    size_t width,
    void* t
)
{
	memcpy(t, a, width);
	memcpy(a, b, width);
	memcpy(b, t, width);
}

#if defined(__BORLANDC__)
#pragma intrinsic -memcpy
#else
//#pragma intrinsic -(memcpy)
#endif
