#include "SimpleXML.hpp"
namespace SimpleXML {

#if defined(_WIN32) && defined(_MSC_VER) && defined(_DEBUG)
#define STRICT 1
#define LEAN_AND_MEAN
#include <windows.h>
static const char *texts[] =
{ "ok"
, "bad argument(s)"
, "unexpected end of data"
, "unexpected symbol"
, "tag end mismatch"
, "depth overflow"
, "= missed"
, "' or \" missed"
};
static const char *err_text(parseRet ret)
{
	static_assert(static_cast<size_t>(parseRet::COUNT) == _countof(texts), "parseRet mismatch");
	return texts[static_cast<size_t>(ret)];
}
#endif

static const auto bom_utf8 = "\xEF\xBB\xBF"_v;
static const auto xml_header = "<?xml "_v;
static const auto comment_begin = "<!--"_v;
static const auto cdata_begin = "<![CDATA["_v;
static const auto doctype_begin = "<!DOCTYPE"_v;

class ParseException
{
public:
	parseRet   ret;
	const char *ps;
	explicit ParseException(parseRet _Ret, const char *_Pos) : ret(_Ret), ps(_Pos) {}
};

class View : public str_view
{
private:
	mutable str_view saved;

public:
	~View() noexcept {}
	View(const char *_Beg, const char *_End) noexcept : str_view(_Beg, _End), saved(_Beg) {}

	void save_start(const int offset = 0) const noexcept { saved.ps = ps + offset; }
	void save_end(const int offset = 0) const noexcept { saved.pe = ps + offset; }
	const str_view& get_saved() const noexcept { return saved; }
	void move(const int offset) noexcept { ps += offset; }

	static inline bool space(const char c) noexcept { return c==' ' || c=='\t' || c=='\n' || c=='\r' || c=='\0'; }
	static inline bool Name(const char c) noexcept { return (c>='A' && c<='Z') || (c>='a' && c<='z') || c=='_'; }
	static inline bool name(const char c) noexcept { return Name(c) || (c>='0' && c<='9') || c=='-' || c=='.' || c==':'; }

	char skip_spaces() noexcept {
		while (ps < pe && space(*ps)) ++ps;
		return ps < pe ? *ps : '\0';
	}

	void check_end() { if (ps >= pe) throw ParseException(parseRet::UnexpectedEnd, ps); }

	char look_next() const noexcept { return ps < pe ? *ps : '\0'; }
	char get_next() { check_end(); return *ps++; }
	void ensure_next(const char c) { if (c != get_next()) throw ParseException(parseRet::UnexpectedChr, --ps); }
	void ensure_next(const str_view& v) { for (const char c : v) ensure_next(c); }

	bool is_next(const str_view& v, int offset = 0) const noexcept {
		const auto sz = v.size();
		const char *pb = ps + offset;
		return pb + sz <= pe && std::equal(pb, pb + sz, v.begin(), v.end());
	}

	char skip_name(const int offset = 0) {
		ps += offset;
		if (!Name(get_next()))
			throw ParseException(parseRet::UnexpectedChr, --ps);
		while (ps < pe && name(*ps)) ++ps;
		check_end();
		return *ps;
	}

	char find_tag_end() {
		char q = '\0';
		while (ps < pe && *ps != '<' && (q || *ps != '>')) {
			if (q) { if (q == *ps) q = '\0'; } else { if (*ps == '"' || *ps == '\'') q = *ps; }
			++ps;
		}
		check_end();
		if (*ps != '>')
			throw ParseException(parseRet::UnexpectedChr, ps); else { ++ps; return ps[-2]; }
	}
};

parseRet parse(const char *xml, size_t size, const IParseCallback* cb)
{
	str_view tags[32]{};
	str_view body[_countof(tags)]{};

	const int topmost = static_cast<int>(_countof(tags) - 1);
	int top = 0 - 1;

	View s(xml, xml + size);
	auto res = parseRet::Ok;
#if defined(_WIN32) && defined(_MSC_VER) && defined(_DEBUG)
	bool attr = false;
#endif

	try {
		if (!xml || !size || !cb)
			throw ParseException(parseRet::BadArgs, xml);

		if (s.look_next() == bom_utf8.data()[0]) // [BOM]<?xml ... ?>
			s.ensure_next(bom_utf8);
		s.ensure_next(xml_header);
		if (s.find_tag_end() != '?')
			throw ParseException(parseRet::UnexpectedChr, s.data()-2);

		for (;;)
		{
			if (s.get_next() != '<')
				continue;

			auto c = s.get_next();
			if (c == '!') {
				if (s.is_next(comment_begin, -2)) // <!--...-->
					c = '-';
				else if (s.is_next(cdata_begin, -2)) //  <![CDATA[...]]>
					c = ']';
				else if (s.is_next(doctype_begin, -2)) { // <!DOCTYPE ... > or \n]]>
					do { c = s.get_next(); } while (c != '>' && c != '\n' && c != '\r');
					if (c != '>') {
						for (;;) {
							do { c = s.get_next(); } while (c == '\n' && c == '\r');
							if (c == ']' && s.look_next() == '>') {
								s.move(+1); break;
							}
						}
					}
					continue;
				}
				else
					throw ParseException(parseRet::UnexpectedChr, s.data());
				s.move(static_cast<int>(c == '-' ? comment_begin.size() : cdata_begin.size()) - 2);
				for (;;) {
					while (s.get_next() != c) {}
					if (s.look_next() != c)
						continue;
					s.move(+1);
					if (s.look_next() != '>')
						continue;
					s.move(+1);
					break;
				}
				continue;
			}

			else if (c == '?') { // <? ... ?>
				if (s.find_tag_end() != '?')
					throw ParseException(parseRet::UnexpectedChr, s.data()-2);
				continue;
			}

			else if (c == '/') { // </Tag>
				if (top < 0)
					throw ParseException(parseRet::BadEndTag, s.data()-2);
				s.save_start();
				body[top].pe = s.data() - 2;
				auto t = s.skip_name();
				if (t != '>')
					throw ParseException(parseRet::UnexpectedChr, s.data());
				s.save_end();
				if (!(tags[top] == s.get_saved()))
					throw ParseException(parseRet::BadEndTag, s.data() - 2 - tags[top].size());
				auto r = cb->OnBody(top, tags, body[top]);
				--top;
				if (r == cbRet::Stop || top < 0)
					break;
				s.move(+1);
				continue;
			}

			else if (!s.Name(c)) {
				throw ParseException(parseRet::UnexpectedChr, s.data()-1);
			}

			else { // <Tag> <Tag/> <Tag ...> <Tag .../>
				s.move(-1);
				if (top >= topmost)
					throw ParseException(parseRet::DepthOvflow, s.data());
				s.save_start();
				auto t = s.skip_name();
				s.save_end();
				tags[++top] = s.get_saved();
				s.save_start();
				s.move(+1);
				bool has_attr = false, simple_tag = false;
				if (t != '>') {
					if (t == '/') {
						s.ensure_next('>');
						simple_tag = true;
					}
					else if (s.space(t)) {
						has_attr = true;
						simple_tag = s.find_tag_end() == '/';
					}
					else
						throw ParseException(parseRet::UnexpectedChr, s.data()-1);
				}
				body[top].ps = body[top].pe = s.data();

				s.save_end(simple_tag ? -2 : -1);
				auto all_attributes = s.get_saved();
				auto r = cb->OnTag(top, tags, all_attributes);
				if (has_attr && r != cbRet::Stop && r != cbRet::ContinueSkipAttr)
				{
					View a(all_attributes.begin(), all_attributes.end());
#if defined(_WIN32) && defined(_MSC_VER) && defined(_DEBUG)
					attr = true;
#endif
					for (;;) {
						if (!a.skip_spaces())
							break;

						a.save_start();
						auto n = a.skip_name();
						a.save_end();
						str_view attr_name = a.get_saved();

						n = a.skip_spaces();
						if (n != '=')
							throw ParseException(parseRet::EqualMissed, a.data());

						a.move(+1);
						n = a.skip_spaces();
						if (n != '"' && n != '\'')
							throw ParseException(parseRet::QuoteMissed, a.data());

						a.move(+1);
						a.save_start();
						while (a.get_next() != n) {}
						a.save_end(-1);
						str_view attr_value = a.get_saved();

						r = cb->OnAttr(top, tags, attr_name, attr_value);
						if (r == cbRet::Stop || r == cbRet::ContinueSkipAttr)
							break;
					}
#if defined(_WIN32) && defined(_MSC_VER) && defined(_DEBUG)
					attr = false;
#endif
				}
				if (r == cbRet::Stop)
					break;
				if (simple_tag) {
					r = cb->OnBody(top, tags, body[top]);
					--top;
					if (r == cbRet::Stop)
						break;
				}
			}
		}
	}

	catch (const ParseException& ex) {
		#if defined(_WIN32) && defined(_MSC_VER) && defined(_DEBUG)
		char out[256];
		wsprintfA(out, "SimpleXML parser%s failed: off=%d err=%s\n",
			attr ? "(attr)" : "", (int)(ex.ps-xml), err_text(ex.ret)
		);
		OutputDebugStringA(out);
		#endif
		res = ex.ret;
	}

	return res;
}
} // namespace
