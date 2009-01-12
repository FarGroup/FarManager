#ifndef __MY_HVALUE
#define __MY_HVALUE

/*******************************************************************
   HValue               - holder for all possible data types in one place
     HConvertValue      - descedant to make type conversion
     HStringValue       - descedant to store string value in local place
 *******************************************************************/
typedef BYTE HVType;

enum HVTypes {
  HVVoid,
  HVByte      = 1,   //Simple data
  HVChar,
  HVWord,
  HVShort,
  HVDword,
  HVInt,
  HVFloat,
  HVDouble,
  HVHandle,         //Variable size data
  HVString
};

/*******************************************************************
   HValue               - holder for all possible data types in one place
 *******************************************************************/
STRUCT( HValue )
    union {
      DWORD  _Dword;
      double _Double;
    } Value;
  protected:
    virtual void   DoEmpty( void );
    virtual DWORD  DoSizeofData( void )                    const;
    virtual LPVOID DoPtrData( void )                       const;
    virtual void   DoTypeChange( HVType oldT,HVType newT,LPVOID val );
  public:
    HVType Type;
  public:
    HValue( void );
    virtual ~HValue();

    static DWORD  Sizeof( HVType tp );
    static BOOL   WriteToBuff( const HValue *p,LPVOID Buff,DWORD sz );
    static BOOL   ReadFromBuff( PHValue p,const LPVOID Buff,DWORD sz );

    static BOOL   isSimple( HVType Type );
    static BOOL   isReal( HVType Type );
    static BOOL   isFixed( HVType Type );
    static BOOL   isVariable( HVType Type );
    static BOOL   isSetted( HVType Type );
    static BOOL   isEmpty( HVType Type );

    BOOL     WriteToBuff( LPVOID Buff,DWORD sz )        const;
    BOOL     ReadFromBuff( const LPVOID Buff,DWORD sz );

    DWORD    Sizeof( void )                             const;
    LPVOID   Ptr( void )                                const;
    void     Empty( void );
    LPVOID   Data( void )                               const;
    DWORD    DataSize( void )                           const;

    BOOL     isSimple( void )                           const;
    BOOL     isReal( void )                             const;
    BOOL     isFixed( void )                            const;
    BOOL     isVariable( void )                         const;
    BOOL     isSetted( void )                           const;
    BOOL     isEmpty( void )                            const;

    BYTE     GetByte( void )   const;
    char     GetChar( void )   const;
    WORD     GetWord( void )   const;
    short    GetShort( void )  const;
    DWORD    GetDword( void )  const;
    int      GetInt( void )    const;
    float    GetFloat( void )  const;
    double   GetDouble( void ) const;
    HANDLE   GetHandle( void ) const;
    CONSTSTR GetString( void ) const;

    BYTE     SetByte( BYTE p );
    char     SetChar( char p );
    WORD     SetWord( WORD p );
    short    SetShort( short p );
    DWORD    SetDword( DWORD p );
    int      SetInt( int p );
    float    SetFloat( float p );
    double   SetDouble( double p );
    HANDLE   SetHandle( HANDLE p );
    CONSTSTR SetString( CONSTSTR p );

//hcvvalue.cpp
    BOOL        isDirectConvertible( HVType to );
    static BOOL isDirectConvertible( HVType from, HVType to );

    BYTE     ConvertAsByte( void );
    char     ConvertAsChar( void );
    WORD     ConvertAsWord( void );
    short    ConvertAsShort( void );
    DWORD    ConvertAsDword( void );
    int      ConvertAsInt( void );
    float    ConvertAsFloat( void );
    double   ConvertAsDouble( void );
    HANDLE   ConvertAsHandle( void );
    CONSTSTR ConvertAsString( void );
    HANDLE   ConvertAsHandle( HANDLE p,DWORD sz );
    CONSTSTR ConvertAsString( char *Buff,int BuffSize );
};

/*******************************************************************
   HStringValue      - descedant to store string value in local place
 *******************************************************************/
STRUCTBASE( HStringValue, public HValue )
    MyString String;
  protected:
    virtual DWORD  DoSizeofData( void ) const;
    virtual LPVOID DoPtrData( void )    const;
    virtual void   DoTypeChange( HVType oldT,HVType newT,LPVOID val );
  public:
    HStringValue( void ) {}
};

#endif
