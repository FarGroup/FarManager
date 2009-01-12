#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__MSWIN32__)
  #pragma optimize( "gt", on )
  #pragma optimize( "y",  off )
  #pragma auto_inline( on )
#endif

#define lzhN           (1UL<<12) /*12 == 4096*/     /* buffer size */
#define lzhF           ((short)60)                           /* lookahead buffer size */
#define lzhTHRESHOLD   2
#define lzhNIL         lzhN                         /* leaf of tree */
#define lzhN_CHAR      (256 - lzhTHRESHOLD + lzhF)  /* kinds of characters (character code = 0..lzhN_CHAR-1) */
#define lzhT           (lzhN_CHAR * 2 - 1)          /* size of table */
#define lzhR           (lzhT - 1)                   /* position of root */
#define lzhMAX_FREQ    0x8000                       /* updates tree when the */

class LZHMemCompress {
  private:
        DWORD codesize;
        DWORD textsize;

        // LZSS compression

        BYTE text_buf[lzhN + lzhF - 1];
        short match_position, match_length, lson[lzhN + 1], rson[lzhN + 257], dad[lzhN + 1];

        // Huffman coding

        WORD freq[lzhT + 1];       /* frequency table */
        short prnt[lzhT + lzhN_CHAR]; /* pointers to parent nodes, except for the */
                                /* elements [lzhT..lzhT + lzhN_CHAR - 1] which are used to get */
                                /* the positions of leaves corresponding to the codes. */
        short son[lzhT];           /* pointers to child nodes (son[], son[] + 1) */

        WORD code, len;

        BYTE *pMemIn, *pMemOut;
        int nMemInSize, nMemOutSize;
        int nMemInOff, nMemOutOff;

protected:
        WORD getbuf;
        BYTE getlen;

        WORD putbuf;
        BYTE putlen;

protected:
        void InitTree(void);                            // initialize trees
        void InsertNode(short r);                       // insert to tree
        void DeleteNode(short p);                       // remove from tree
        WORD GetBit(void);                              // get one bit
        WORD GetByte(void);                             // get one byte
        BOOL Putcode(short l, WORD c); // output c bits of code
        void StartHuff(void);
        void reconst(void);
        void update(short c);
        BOOL EncodeChar(WORD c);
        BOOL EncodePosition(WORD c);
        short DecodeChar(void);
        short DecodePosition(void);
        BOOL Encode( DWORD textsize );                  // compression
        BOOL Decode(void);                              // recover
        BOOL fnc_getc( BYTE *val );
        BOOL fnc_putc( BYTE val );

public:
        DWORD Compress( void *pInBuffer, int nInBufLen, void *pOutBuf, int nOutBufLen );
        DWORD Decompress( void *pInBuffer, int nInBufLen, void *pOutBuf, int nOutBufLen );
};

/****************************************************
   LZHMemCompress
 ****************************************************/
void LZHMemCompress::InitTree(void)  /* initialize trees */
{
    short  i;

    for (i = lzhN + 1; i <= lzhN + 256; i++)
        rson[i] = lzhNIL;        /* root */
    for (i = 0; i < lzhN; i++)
        dad[i] = lzhNIL;         /* node */
}

void LZHMemCompress::InsertNode(short r)  /* insert to tree */
{
    short  i, p, cmp,c;
    BYTE *key;

    cmp = 1;
    key = &text_buf[r];
    p   = (short)( lzhN + 1 + key[0] );
    rson[r] = lson[r] = lzhNIL;
    match_length = 0;
    for(;;) {
      if (cmp >= 0) {
        if (rson[p] != lzhNIL)
            p = rson[p];
        else {
            rson[p] = r;
            dad[r] = p;
            return;
        }
      } else {
        if (lson[p] != lzhNIL)
            p = lson[p];
        else {
            lson[p] = r;
            dad[r] = p;
            return;
        }
      }
      for (i = 1; i < lzhF; i++)
        if ((cmp = (short)(key[i] - text_buf[p + i])) != 0)
          break;
        if (i > lzhTHRESHOLD) {
          if (i > match_length) {
            match_position = (short)( ((r - p) & (lzhN - 1)) - 1 );
            if ((match_length = i) >= lzhF)
              break;
          }
          if (i == match_length) {
            if ((c = (short)( ((r - p) & (lzhN-1)) - 1) ) < (WORD)match_position) {
              match_position = c;
            }
          }
        }
    }
    dad[r] = dad[p];
    lson[r] = lson[p];
    rson[r] = rson[p];
    dad[lson[p]] = r;
    dad[rson[p]] = r;
    if (rson[dad[p]] == p)
        rson[dad[p]] = r;
    else
        lson[dad[p]] = r;
    dad[p] = lzhNIL; /* remove p */
}

void LZHMemCompress::DeleteNode(short p)  /* remove from tree */
{
    short  q;

    if (dad[p] == lzhNIL)
        return;         /* not registered */
    if (rson[p] == lzhNIL)
        q = lson[p];
    else {
                if (lson[p] == lzhNIL)
                        q = rson[p];
                else {
                        q = lson[p];
                        if (rson[q] != lzhNIL) {
                                do {
                                        q = rson[q];
                                } while (rson[q] != lzhNIL);
                                rson[dad[q]] = lson[q];
                                dad[lson[q]] = dad[q];
                                lson[q] = lson[p];
                                dad[lson[p]] = q;
                        }
                        rson[q] = rson[p];
                        dad[rson[p]] = q;
                }
        }
        dad[q] = dad[p];
        if (rson[dad[p]] == p)
                rson[dad[p]] = q;
        else
                lson[dad[p]] = q;
        dad[p] = lzhNIL;
}

/* Huffman coding */
/* table for encoding and decoding the upper 6 bits of position */
/* for encoding */
BYTE p_len[64] = {
    0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05,
                0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06,
                0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
                0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
                0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
                0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
                0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};

BYTE p_code[64] = {
    0x00, 0x20, 0x30, 0x40, 0x50, 0x58, 0x60, 0x68,
                0x70, 0x78, 0x80, 0x88, 0x90, 0x94, 0x98, 0x9C,
                0xA0, 0xA4, 0xA8, 0xAC, 0xB0, 0xB4, 0xB8, 0xBC,
                0xC0, 0xC2, 0xC4, 0xC6, 0xC8, 0xCA, 0xCC, 0xCE,
                0xD0, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA, 0xDC, 0xDE,
                0xE0, 0xE2, 0xE4, 0xE6, 0xE8, 0xEA, 0xEC, 0xEE,
                0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
                0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

/* for decoding */
BYTE d_code[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
                0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
                0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
                0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
                0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
                0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
                0x0C, 0x0C, 0x0C, 0x0C, 0x0D, 0x0D, 0x0D, 0x0D,
                0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F,
                0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11,
                0x12, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13,
                0x14, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15,
                0x16, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x17,
                0x18, 0x18, 0x19, 0x19, 0x1A, 0x1A, 0x1B, 0x1B,
                0x1C, 0x1C, 0x1D, 0x1D, 0x1E, 0x1E, 0x1F, 0x1F,
                0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x23, 0x23,
                0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27,
                0x28, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2B, 0x2B,
                0x2C, 0x2C, 0x2D, 0x2D, 0x2E, 0x2E, 0x2F, 0x2F,
                0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
                0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
};

BYTE d_len[256] = {
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
                0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
                0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
                0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
                0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
                0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
                0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
                0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
                0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
                0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
                0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
                0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
                0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
                0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
                0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
                0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
                0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
                0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
                0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
                0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
                0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};


WORD LZHMemCompress::GetBit(void)    /* get one bit */
{
    WORD i;

    while (getlen <= 8) {
      BYTE bt;
      if ( !fnc_getc(&bt) )
        i = 0;
       else
        i = (WORD)bt;
      getbuf |= i << (8 - getlen);
      getlen += 8;
    }
    i = getbuf;
    getbuf <<= 1;
    getlen--;
    return (WORD)( ((i & 0x8000) >> 15) );
}

WORD LZHMemCompress::GetByte(void)   /* get one byte */
{
    WORD i;

    while (getlen <= 8) {
      BYTE bt;
      if ( !fnc_getc(&bt) )
        i = 0;
       else
        i = (WORD)bt;
      getbuf |= i << (8 - getlen);
      getlen += 8;
    }
    i = getbuf;
    getbuf <<= 8;
    getlen -= 8;
    return (WORD)( ((i & 0xff00) >> 8) );
}

BOOL LZHMemCompress::Putcode(short l, WORD c)     /* output c bits of code */
{
    putbuf |= c >> putlen;
    putlen = (BYTE)( putlen + l );

    if ( putlen >= 8) {
      if( !fnc_putc( (BYTE)( putbuf >> 8 ) ) )
        return FALSE;

      putlen -= 8;
      if( putlen >= 8) {
        if ( !fnc_putc((BYTE)putbuf) )
          return FALSE;

        codesize += 2;
        putlen -= 8;
        putbuf = (WORD)( c << (l - putlen) );
      } else {
        putbuf <<= 8;
        codesize++;
      }
    }
 return TRUE;
}


/* initialization of tree */

void LZHMemCompress::StartHuff(void)
{
    short i, j;

    for (i = 0; i < lzhN_CHAR; i++) {
        freq[i] = 1;
        son[i] = (short)( i + lzhT );
        prnt[i + lzhT] = i;
    }
    i = 0; j = lzhN_CHAR;
    while (j <= lzhR) {
        freq[j] = (WORD)( freq[i] + freq[i + 1] );
        son[j] = i;
        prnt[i] = prnt[i + 1] = j;
        i += 2; j++;
    }
    freq[lzhT] = 0xffff;
    prnt[lzhR] = 0;
}


/* reconstruction of tree */

void LZHMemCompress::reconst(void)
{
    short i, j, k;
    WORD f, l;

    /* collect leaf nodes in the first half of the table */
    /* and replace the freq by (freq + 1) / 2. */
    j = 0;
    for (i = 0; i < lzhT; i++) {
      if (son[i] >= lzhT) {
        freq[j] = (WORD)( (freq[i] + 1) / 2 );
        son[j]  = son[i];
        j++;
      }
    }
    /* begin constructing tree by connecting sons */
    for (i = 0, j = lzhN_CHAR; j < lzhT; i += 2, j++) {
        k = (short)( i + 1 );
        f = freq[j] = (WORD)( freq[i] + freq[k] );
        for (k = (short)( j - 1 ); f < freq[k]; k--);
        k++;
        l = (WORD)( (j - k) * 2 );
        memmove(&freq[k + 1], &freq[k], l);
        freq[k] = f;
        memmove(&son[k + 1], &son[k], l);
        son[k] = i;
    }
    /* connect prnt */
    for (i = 0; i < lzhT; i++) {
        if ((k = son[i]) >= lzhT) {
            prnt[k] = i;
        } else {
            prnt[k] = prnt[k + 1] = i;
        }
    }
}


/* increment frequency of given code by one, and update tree */

void LZHMemCompress::update(short c)
{
    short i, j, k, l;

    if (freq[lzhR] == lzhMAX_FREQ) {
        reconst();
    }
    c = prnt[c + lzhT];
    do {
        k = ++freq[c];

        /* if the order is disturbed, exchange nodes */
        if ((WORD)k > freq[l = (short)(c + 1)]) {
            while ((WORD)k > freq[++l]);
            l--;
            freq[c] = freq[l];
            freq[l] = k;

            i = son[c];
            prnt[i] = l;
            if (i < lzhT) prnt[i + 1] = l;

            j = son[l];
            son[l] = i;

            prnt[j] = c;
            if (j < lzhT) prnt[j + 1] = c;
            son[c] = j;

            c = l;
        }
    } while ((c = prnt[c]) != 0); /* repeat up to root */
}


BOOL LZHMemCompress::EncodeChar(WORD c)
{
    WORD i;
    short j, k;

    i = 0;
    j = 0;
    k = prnt[c + lzhT];

    /* travel from leaf to root */
    do {
        i >>= 1;

        /* if node's address is odd-numbered, choose bigger brother node */
        if (k & 1) i += 0x8000;

        j++;
    } while ((k = prnt[k]) != lzhR);

    if ( !Putcode(j, i) )
      return FALSE;

    code = i;
    len = j;
    update(c);

 return TRUE;
}

BOOL LZHMemCompress::EncodePosition(WORD c)
  {  WORD i = (WORD)( c >> 6 );

 return Putcode(p_len[i], (WORD)(p_code[i] << 8) ) &&  /* output upper 6 bits by table lookup */
        Putcode(6, (WORD)( (c & 0x3f) << 10 ) );               /* output lower 6 bits verbatim */
}

short LZHMemCompress::DecodeChar(void)
{
    WORD c;

    c = son[lzhR];

    /* travel from root to leaf, */
    /* choosing the smaller child node (son[]) if the read bit is 0, */
    /* the bigger (son[]+1} if 1 */
    while (c < lzhT)
      c = son[ c + GetBit() ];
    c -= lzhT;
    update(c);
    return (short)c;
}

short LZHMemCompress::DecodePosition(void)
{
    WORD i, j, c;

    /* recover upper 6 bits from table */
    i = GetByte();
    c = (WORD)( d_code[i] << 6 );
    j = d_len[i];

    /* read lower 6 bits verbatim */
    j -= 2;
    while (j--)
      i = (WORD)( (i << 1) + GetBit() );

 return (short)(c | (i & 0x3f));
}

/* compression */

BOOL LZHMemCompress::Encode( DWORD _textsize )  /* compression */
{
    short  i, len, r, s, last_match_length;
    BYTE   bt;

    codesize = 0;
    textsize = _textsize;

    if ( !fnc_putc( (BYTE)(textsize & 0xff) ) ||
         !fnc_putc((BYTE)((textsize & 0xff00) >> 8)) ||
         !fnc_putc((BYTE)((textsize & 0xff0000L) >> 16)) ||
         !fnc_putc((BYTE)((textsize & 0xff000000L) >> 24)) ||
         textsize == 0 )
      return FALSE;

    getbuf = putbuf = 0;
    getlen = putlen = 0;

    StartHuff();
    InitTree();
    s = 0;
    r = lzhN - lzhF;

    for (i = s; i < r; i++)
        text_buf[i] = 0x20;

    for (len = 0; len < lzhF; len++) {
      if ( !fnc_getc(&bt) )
        return FALSE;
      text_buf[r + len] = bt;
    }

    for (i = 1; i <= lzhF; i++)
      InsertNode( (short)(r - i) );

    InsertNode(r);
    do {
        if (match_length > len)
            match_length = len;

        if (match_length <= lzhTHRESHOLD) {
          match_length = 1;
          if ( !EncodeChar(text_buf[r]) )
            return FALSE;
        } else {
          if ( !EncodeChar( (WORD)(255 - lzhTHRESHOLD + match_length) ) ||
               !EncodePosition(match_position) )
            return FALSE;
        }

        last_match_length = match_length;

        for (i = 0; i < last_match_length; i++) {
            if ( !fnc_getc(&bt) )
              break;
            DeleteNode(s);
            text_buf[s] = bt;
            if (s < lzhF - 1)
                text_buf[s + lzhN] = bt;
            s = (short)( (s + 1) & (lzhN - 1) );
            r = (short)( (r + 1) & (lzhN - 1) );
            InsertNode(r);
        }
        while (i++ < last_match_length) {
            DeleteNode(s);
            s = (short)( (s + 1) & (lzhN - 1) );
            r = (short)( (r + 1) & (lzhN - 1) );
            if (--len) InsertNode(r);
        }
    } while (len > 0);

    if (putlen ) {
      if ( !fnc_putc( (BYTE)( putbuf >> 8 ) ) )
        return FALSE;
      codesize++;
    }

 return TRUE;
}

BOOL LZHMemCompress::Decode(void)  /* recover */
{
    short  i, j, k, r, c;
    DWORD  count;
    BYTE   bt;

    codesize = 0;
    if ( !fnc_getc(&bt) ) return FALSE;  textsize = bt;
    if ( !fnc_getc(&bt) ) return FALSE;  textsize |= (((DWORD)bt) << 8);
    if ( !fnc_getc(&bt) ) return FALSE;  textsize |= (((DWORD)bt) << 16);
    if ( !fnc_getc(&bt) ) return FALSE;  textsize |= (((DWORD)bt) << 24);

    if (textsize == 0)
      return FALSE;

    getbuf = putbuf = 0;
    getlen = putlen = 0;

    StartHuff();
    for (i = 0; i < lzhN - lzhF; i++)
        text_buf[i] = 0x20;
    r = lzhN - lzhF;

    for (count = 0; count < textsize; ) {
        c = DecodeChar();
        if (c < 256) {
            if ( !fnc_putc((BYTE)c) )
              return FALSE;

            text_buf[r++] = (unsigned char)c;
            r &= (lzhN - 1);
            count++;
        } else {
            i = (short)( (r - DecodePosition() - 1) & (lzhN - 1) );
            j = (short)( c - 255 + lzhTHRESHOLD );
            for (k = 0; k < j; k++) {
                c = text_buf[(i + k) & (lzhN - 1)];
                if ( !fnc_putc( (BYTE)c ) )
                  return FALSE;
                text_buf[r++] = (BYTE)c;
                r &= (lzhN - 1);
                count++;
            }
        }

    }
 return TRUE;
}

BOOL LZHMemCompress::fnc_getc( BYTE *val )
  {
    if ( nMemInOff >= nMemInSize )
      return FALSE;

    *val = pMemIn[nMemInOff++];

 return TRUE;
}

BOOL LZHMemCompress::fnc_putc( BYTE val )
  {
    if ( nMemOutOff >= (int)textsize )
      return TRUE;

    if ( nMemOutOff >= nMemOutSize )
      return FALSE;

    pMemOut[nMemOutOff++] = val;

 return TRUE;
}

DWORD LZHMemCompress::Compress(void *pInBuffer, int nInBufLen, void *pOutBuf, int nOutBufLen)
  {
     if ( nInBufLen <= lzhF ) {
       if ( nOutBufLen <= nInBufLen+(int)sizeof(DWORD) ) {
         FIO_SETERRORN( ERROR_INVALID_PARAMETER );
         return 0;
       }
       *((DWORD*)pOutBuf) = MAX_DWORD;
       memcpy( ((BYTE*)pOutBuf) + sizeof(DWORD),pInBuffer,nInBufLen );
       return nInBufLen + sizeof(DWORD);
     }

     pMemIn      = (BYTE *)pInBuffer;
     nMemInSize  = nInBufLen;
     pMemOut     = (BYTE *)pOutBuf;
     nMemOutSize = nOutBufLen;
     nMemInOff   = 0;
     nMemOutOff  = 0;

 return (DWORD)( Encode( nInBufLen ) ? nMemOutOff : 0 );
}

DWORD LZHMemCompress::Decompress(void *pInBuffer, int nInBufLen, void *pOutBuf, int nOutBufLen)
  {
     if ( *((DWORD*)pInBuffer) == MAX_DWORD ) {
       if ( nInBufLen < (int)sizeof(DWORD) ||
            nOutBufLen < nInBufLen-(int)sizeof(DWORD) ) {
         FIO_SETERRORN( ERROR_INVALID_PARAMETER );
         return 0;
       }
       memcpy( pOutBuf,((BYTE*)pInBuffer)+sizeof(DWORD),nInBufLen-sizeof(DWORD) );
       return nInBufLen - (int)sizeof(DWORD);
     }

     pMemIn      = (BYTE *)pInBuffer;
     nMemInSize  = nInBufLen;
     pMemOut     = (BYTE *)pOutBuf;
     nMemOutSize = nOutBufLen;
     nMemInOff   = 0;
     nMemOutOff  = 0;

 return (DWORD)( Decode() ? nMemOutOff : 0 );
}

static LZHMemCompress stdCompress;

DWORD MYRTLEXP LZHCompress( void *pInBuffer, int nInBufLen, void *pOutBuf, int nOutBufLen )
  {
 return stdCompress.Compress(pInBuffer,nInBufLen,pOutBuf,nOutBufLen);
}

DWORD MYRTLEXP LZHDecompress( void *pInBuffer, int nInBufLen, void *pOutBuf, int nOutBufLen )
  {
 return stdCompress.Decompress(pInBuffer,nInBufLen,pOutBuf,nOutBufLen);
}
