#if defined(__BORLAND)
  #pragma nopackwarning
  #pragma pack(pop);
#else
#if defined(__TEC32__) || defined(__DMC) || defined(__SC__) || defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100))
  #pragma pack()
#else
#if defined(__MSOFT) || defined(__INTEL)
  #pragma pack(pop)
#else
  #pragma pack(__pop)
#endif
#endif
#endif
