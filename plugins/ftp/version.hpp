#ifndef __VERSION_HPP__
#define __VERSION_HPP__
#include "farversion.hpp"

#define FTP_BUILD 265
#define FTPPRODUCTNAME "FarFtp"

#define ftpgenericpluginrc(major, minor, desc, name, filename) fullgenericpluginrc_nobuild(major, minor, desc, name, filename, FARCOPYRIGHT, FAR_MAJOR_VER, FAR_MINOR_VER, FTP_BUILD, FTPPRODUCTNAME)

#endif
