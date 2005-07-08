
#if defined(__BORLAND)
  #pragma nopackwarning
  #pragma pack(push,1);
#else
#if defined(__SC__) || defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100))
  #pragma pack(1)
#else
#if defined(__MSOFT)
  #pragma pack(push,1)
#else
  #pragma pack(__push,1);
#endif
#endif
#endif
