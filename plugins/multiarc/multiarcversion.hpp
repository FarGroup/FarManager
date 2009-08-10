#ifndef __MULTIARCVERSION_HPP__
#define __MULTIARCVERSION_HPP__
#include "farversion.hpp"

#define MA_BUILD 187
#define MAPRODUCTNAME "MultiArc"
#define FMTDESCRIPTION "Second-level plugin module for MultiArc FAR Manager plugin"

#define magenericpluginrc(major, minor, name, filename) fullgenericpluginrc_nobuild(major, minor, FMTDESCRIPTION, name, filename, FARCOPYRIGHT, FAR_MAJOR_VER, FAR_MINOR_VER, MA_BUILD, MAPRODUCTNAME)

#endif
