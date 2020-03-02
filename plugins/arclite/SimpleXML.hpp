#ifndef H_SimpleXML_H
#define H_SimpleXML_H

#include <string>

namespace SimpleXML
{//################################################################################################
//#################################################################################################

class str_view
{
public:
	const char *ps;
	const char *pe;
public:
	constexpr str_view() : ps(nullptr), pe(nullptr) {}
	constexpr str_view(const char *_Str) : ps(_Str), pe(_Str) {}
	constexpr str_view(const str_view&) = default;
	constexpr str_view(const char *_Str, size_t _Len) : ps(_Str), pe(_Str+_Len) {}
	constexpr str_view(const char *_Beg, const char *_End) : ps(_Beg), pe(_End) {}
	bool empty() const noexcept { return ps >= pe; }
	size_t size() const noexcept { return static_cast<size_t>(pe - ps); }
	const char *data() const noexcept { return ps; }
	const char *begin() const noexcept { return ps; }
	const char *end() const noexcept { return pe; }
};
constexpr auto operator "" _v(const char *_Str, size_t _Len) noexcept { return (str_view(_Str, _Len)); }
inline bool operator==(const str_view& Lhs, const str_view& Rhs)
{ return std::equal(Lhs.begin(), Lhs.end(), Rhs.begin(), Rhs.end()); }
inline bool operator!=(const str_view& Lhs, const str_view& Rhs) { return !(Lhs == Rhs); }

//#################################################################################################

enum class cbRet { Stop, Continue, ContinueSkipAttr };

class IParseCallback
{
public:
	virtual ~IParseCallback() {}
	virtual cbRet OnTag (int top, const str_view* path, const str_view& attr) const =0;
	virtual cbRet OnBody(int top, const str_view* path, const str_view& body) const =0;
	virtual cbRet OnAttr(int top, const str_view* path, const str_view& name, const str_view& val) const =0;
};

//#################################################################################################

enum class parseRet {
	Ok,
	BadArgs,       // bad argument(s)
	UnexpectedEnd, // unexpected end of data
	UnexpectedChr, // unexpected symbol
	BadEndTag,     // tag end mismatch
	DepthOvflow,   // depth overflow
	EqualMissed,   // = missed
	QuoteMissed,   // ' or " missed
	COUNT
};

// !Note: ---------------------------------------------------------------------
// 1) Multibyte only! (supposed UTF8 decoding in callback)
// 2) &Entity; should be decoded in callback -- can be found in attribute value
// 3) <![CDATA[should be decoded in callback]]> -- can be found inside body
// 4) <?any content skipped ?>
// 5) <!-- comment also skipped silently -->
// 6) <!DOCTYPE IGNORED ... > ... ]]>
//-----------------------------------------------------------------------------
parseRet parse(const char *xml, size_t size, const IParseCallback* cb);

//#################################################################################################
}//################################################################################################
#endif //H_SimpleXML_H
