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

static void iswap (int *, int *), cswap (char *, char *);

static unsigned int n_to_swap;  /* nbr of chars or ints to swap */
int _maxspan = 7;               /* subfiles of _maxspan or fewer elements */
                                /* will be sorted by a simple insertion sort */

/* Adjust _maxspan according to relative cost of a swap and a compare.  Reduce
_maxspan (not less than 1) if a swap is very expensive such as when you have
an array of large structures to be sorted, rather than an array of pointers to
structures.  The default value is optimized for a high cost for compares. */

#define SWAP(a,b) (*swap_fp)(a,b)
#define COMP(a,b,u) (*comp_fp)(a,b,u)

void qsortex(char *base, unsigned int nel, unsigned int width,
            int (*comp_fp)(void *, void *,void*), void *user)
{
  char *stack[40], **sp;                 /* stack and stack pointer        */
  char *i, *j, *limit;                   /* scan and limit pointers        */
  unsigned thresh;                       /* size of _maxspan elements in   */
  void (*swap_fp) (void *, void *);      /* bytes */

  if (width % sizeof(int) != 0)
  {
    swap_fp = cswap;
    n_to_swap = width;
  }
  else
  {
    swap_fp = iswap;
    n_to_swap = width / sizeof(int);
  }

  thresh = _maxspan * width;             /* init threshold                 */
  sp = stack;                            /* init stack pointer             */
  limit = base + nel * width;            /* pointer past end of array      */
  while (1)                              /* repeat until done then return  */
  {
    while (limit - base > thresh)        /* if more than _maxspan elements */
    {
      /*swap middle, base*/
      SWAP (((unsigned)(limit - base) >> 1) -
           ((((unsigned)(limit - base) >> 1)) % width) + base, base);

      i = base + width;                /* i scans from left to right     */
      j = limit - width;               /* j scans from right to left     */

      if ( COMP(i, j,user) > 0 )            /* Sedgewick's                    */
        SWAP(i, j);                    /*    three-element sort          */
      if ( COMP(base, j,user) > 0 )         /*        sets things up          */
        SWAP(base, j);                 /*            so that             */
      if ( COMP(i, base,user) > 0 )         /*              *i <= *base <= *j */
        SWAP(i, base);                 /* *base is the pivot element     */

      while (1)
      {
        do                            /* move i right until *i >= pivot */
          i += width;
        while (COMP (i, base,user) < 0);
        do                            /* move j left until *j <= pivot  */
          j -= width;
        while (COMP (j, base,user) > 0);
        if (i > j)                    /* break loop if pointers crossed */
          break;
        SWAP (i, j);                  /* else swap elements, keep scanning */
      }
      SWAP (base, j);                  /* move pivot into correct place  */
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
      while (j > base && COMP (j - width, j,user) > 0)
      {
        SWAP (j - width, j);
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

static void iswap (int *a, int *b)  /* swap ints */
{
  int tmp;
  unsigned k = n_to_swap;

  do
  {
    tmp = *a;
    *a = *b;
    *b = tmp;
    a++; b++;
  } while (--k);
}

static void cswap (char *a, char *b)    /* swap chars */
{
  char tmp;
  unsigned k = n_to_swap;

  do
  {
    tmp = *a;
    *a = *b;
    *b = tmp;
    a++; b++;
  } while (--k);
}
