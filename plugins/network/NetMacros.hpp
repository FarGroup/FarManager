#ifndef __NETMACROS_HPP__
#define __NETMACROS_HPP__

#define PointToName FSF.PointToName
#define InputBox Info.InputBox

#ifndef UNICODE
#define CharToOEM(s, d) CharToOem(s, d)
#define OEMToChar(s, d) OemToChar(s, d)
#else
#define CharToOEM(s, d) lstrcpy(d, s)
#define OEMToChar(s, d) lstrcpy(d, s)
#endif

#endif // __NETMACROS_HPP__
