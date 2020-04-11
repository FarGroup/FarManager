#pragma once

#include <string.h>
#include "pcre.h"

namespace PCRE
{

  class Match
  {
    int ovector[30];
    int matchCount;
    const pcre *re;
    const char *str;
    int strLen;
    int bufLen;
    char *buffer;

    friend class RegExp;

    Match(const pcre *re, const char *str)
      : re(re),
        str(str),
        strLen(0),
        bufLen(0),
        buffer(0)
    {
      for(const char *p = str; *p++; ++strLen);

      matchCount = pcre_exec(re, 0, str, strLen, 0, 0, ovector, sizeof(ovector) / sizeof(ovector[0]));

      if(matchCount > 0)
        buffer = (char*)pcre_malloc(bufLen = strLen + 10);
    }

  public:

    Match(const Match &m)
      : matchCount(m.matchCount),
        re(m.re),
        str(m.str),
        strLen(m.strLen),
        bufLen(m.bufLen),
        buffer(0)
    {
      for(size_t i = 0; i < sizeof(ovector) / sizeof(ovector[0]); ++i)
        ovector[i] = m.ovector[i];
      if(bufLen > 0)
        buffer = (char*)pcre_malloc(bufLen);
    }

    Match &operator=(const Match &m)
    {
      re = m.re;
      str = m.str;
      strLen = m.strLen;
      matchCount = m.matchCount;
      bufLen = m.bufLen;
      if(buffer)
        pcre_free(buffer);
      for(size_t i = 0; i < sizeof(ovector) / sizeof(ovector[0]); ++i)
        ovector[i] = m.ovector[i];
      if(bufLen > 0)
        buffer = (char*)pcre_malloc(bufLen);
      return *this;
    }

    operator bool()
    {
      return matchCount >= 0;
    }

    const char *operator[] (const char *name)
    {
      int res = pcre_copy_named_substring(re, str, ovector, matchCount, name, buffer, bufLen);
      return res >= 0 ? buffer : 0;
    }

    const char *operator[] (int idx)
    {
      int res = pcre_copy_substring(str, ovector, matchCount, idx, buffer, bufLen);
      return res >= 0 ? buffer : 0;
    }

    ~Match()
    {
      if(buffer)
        pcre_free(buffer);
    }
  };

  class RegExp
  {
    pcre *re;
    const char *lastError;
    int errorOffset;

    RegExp(const RegExp &);         // I'm lazy =)
    RegExp &operator=(const RegExp &);

  public:
    RegExp()
      : re(0),
        lastError(0),
        errorOffset(0)
    {
    }

    RegExp(const char *pattern)
      : re(0),
        lastError(0),
        errorOffset(0)
    {
      compile(pattern);
    }

    RegExp(const char *pattern, const char *options)
      : re(0),
        lastError(0),
        errorOffset(0)
    {
      compile(pattern, options);
    }

    ~RegExp()
    {
      if(re)
        pcre_free(re);
    }

    bool compile(const char *pattern)
    {
      RegExp r("^\\/(.*)\\/([ixms]*)$", "");

      if(Match m = r.match(pattern))
      {
        char options[8] = "";
        strncpy(options, m[2], sizeof(options));
        return compile(m[1], options);
      }
      return false;
    }

    bool compile(const char *pattern, const char *options)
    {
      int nOptions = 0;
      for(const char *opt = options; opt && *opt; ++opt)
      {
        switch(*opt)
        {
          case 'i': nOptions |= PCRE_CASELESS;  break;
          case 'x': nOptions |= PCRE_EXTENDED;  break;
          case 'm': nOptions |= PCRE_MULTILINE; break;
          case 's': nOptions |= PCRE_DOTALL;  break;
        }
      }

      //const char *error = 0;
      //int errorOfs = 0;
      if(re)
        pcre_free(re);
      re = pcre_compile(pattern, nOptions, &lastError, &errorOffset, 0);

      return !!re;
    }

    bool isValid() const
    {
      return !!re;
    }

    const char *getLastError() const
    {
      return lastError;
    }

    int getErrorOffset() const
    {
      return errorOffset;
    }

    Match match(const char *str) const
    {
      return Match(re, str);
    }

  };


}
