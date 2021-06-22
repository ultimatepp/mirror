#include "Core.h"

namespace Upp {

#define LLOG(x)    // DLOG(x)
#define LTIMING(x) // RTIMING(x)

void CParser::ThrowError(const char *s) {
	LLOG("CParser::Error: " << s);
	LLOG(~String(term, min((int)strlen((const char *)term), 512)));
	Pos pos = GetPos();
	Error err(fn + Format("(%d,%d): ", line, pos.GetColumn()) + s);
//	err.term = (const char *)term;
	throw err;
}

CParser& CParser::SkipComments(bool b)
{
	skipcomments = b;
	term = wspc;
	Spaces0();
	return *this;
}

CParser& CParser::NestComments(bool b)
{
	nestcomments = b;
	term = wspc;
	Spaces0();
	return *this;
}

bool CParser::Spaces0() {
	LTIMING("Spaces");
	if(!term)
		return false;
	if((byte)*term > ' ' &&
	   !(term[0] == '/' && term[1] == '/') &&
	   !(term[0] == '/' && term[1] == '*'))
		return false;
	for(;;) {
		if(*term == LINEINFO_ESC) {
			term++;
			fn.Clear();
			while(*term) {
				if(*term == LINEINFO_ESC) {
					++term;
					break;
				}
				if(*term == '\3') {
					line = atoi(++term);
					while(*term) {
						if(*term == LINEINFO_ESC) {
							++term;
							break;
						}
						term++;
					}
					break;
				}
				fn.Cat(*term++);
			}
			continue;
		}
		else
		if(term[0] == '/' && term[1] == '/' && skipcomments) {
			term += 2;
			while(*term && *term != '\n')
				term++;
		}
		else
		if(term[0] == '/' && term[1] == '*' && skipcomments) {
			if(nestcomments) {
				int count = 1;
				term += 2;
				while(*term) {
					if(term[0] == '*' && term[1] == '/') {
						term += 2;
						count--;
						if (count == 0)
							break;
					}
					else if(term[0] == '/' && term[1] == '*')
						count++;
					
					if(*term++ == '\n') {
						line++;
						lineptr = term;
					}
				}
			}
			else {
				term += 2;
				while(*term) {
					if(term[0] == '*' && term[1] == '/') {
						term += 2;
						break;
					}
					if(*term++ == '\n') {
						line++;
						lineptr = term;
					}
				}
			}
		}
		if(!*term) break;
		if((byte)*term > ' ') break;
		if(*term == '\n') {
			line++;
			lineptr = term + 1;
		}
		term++;
	}
	return true;
}

String CParser::LineInfoComment(const String& file, int line, int column)
{
	return String().Cat() << (char)LINEINFO_ESC << file << '\3'
	                      << line << '\3' << column << (char)LINEINFO_ESC;
}

String CParser::GetLineInfoComment(int tabsize) const
{
	return LineInfoComment(GetFileName(), GetLine(), GetColumn(tabsize));
}

const char *CParser::IsId0(const char *s) const {
	const char *t = term + 1;
	s++;
	while(*s) {
		if(*t != *s)
			return NULL;
		t++;
		s++;
	}
	return IsAlNum(*t) || *t == '_' ? NULL : t;
}

bool CParser::Id0(const char *s) {
	LTIMING("Id");
	const char *t = IsId0(s);
	if(!t)
		return false;
	term = t;
	DoSpaces();
	return true;
}

void CParser::PassId(const char *s) {
	LTIMING("PassId");
	if(!Id(s))
		ThrowError(String("missing '") + s + "\'");
}

void CParser::PassChar(char c) {
	LTIMING("PassChar");
	if(!Char(c))
		ThrowError(String("missing '") + c + "\'");
}

void CParser::PassChar2(char c1, char c2) {
	LTIMING("PassChar2");
	if(!Char2(c1, c2))
		ThrowError(String("missing '") + c1 + c2 + "\'");
}

void CParser::PassChar3(char c1, char c2, char c3) {
	LTIMING("PassChar3");
	if(!Char3(c1, c2, c3))
		ThrowError(String("missing '") + c1 + c2 + c3 + "\'");
}

String CParser::ReadId() {
	if(!IsId())
		ThrowError("missing id");
	String result;
	const char *b = term;
	const char *p = b;
	while(iscid(*p))
		p++;
	term = p;
	DoSpaces();
	return String(b, (int)(uintptr_t)(p - b));
}

String CParser::ReadIdt() {
	if(!IsId())
		ThrowError("missing id");
	StringBuffer result;
	int lvl = 0;
	while(IsAlNum(*term) || *term == '_' || *term == '<' || *term == '>' ||
	      *term == ':' || (*term == ',' || *term == ' ' ) && lvl > 0 ) {
		if(*term == '<') lvl++;
		if(*term == '>') lvl--;
		result.Cat(*term++);
	}
	DoSpaces();
	return String(result);
}

bool CParser::IsInt() const {
	LTIMING("IsInt");
	const char *t = term;
	if(*t == '-' || *t == '+') {
		t++;
		while(*t <= ' ')
			t++;
	}
	return IsDigit(*t);
}

int  CParser::Sgn()
{
	int sign = 1;
	if(*term == '-') {
		sign = -1;
		term++;
	}
	else
	if(*term == '+')
		term++;
	Spaces();
	return sign;
}

int  CParser::ReadInt() {
	LTIMING("ReadInt");
	int n;
	bool overflow = false;
	const char *t = ScanInt<char, byte, dword, int, 10>(n, term, overflow);
	if(!t) ThrowError("missing number");
	if(overflow) ThrowError("number is too big");
	term = t;
	DoSpaces();
	return n;
}

int CParser::ReadInt(int min, int max)
{
	int n = ReadInt();
	if(n < min || n > max)
		ThrowError("number is out of range");
	return n;
}

int64 CParser::ReadInt64()
{
	LTIMING("ReadInt64");
	int64 n;
	bool overflow = false;
	const char *t = ScanInt<char, byte, uint64, int64, 10>(n, term, overflow);
	if(!t) ThrowError("missing number");
	if(overflow) ThrowError("number is too big");
	term = t;
	DoSpaces();
	return n;
}

int64 CParser::ReadInt64(int64 min, int64 max)
{
	int64 n = ReadInt64();
	if(n < min || n > max)
		ThrowError("number is out of range");
	return n;
}

bool CParser::IsNumber(int base) const
{
    if(IsDigit(*term)) {
        int q = *term - '0';
        return q >= 0 && q < base;
    }
	int q = ToUpper(*term) - 'A';
    return q >= 0 && q < base - 10;
}

uint32  CParser::ReadNumber(int base)
{
	LTIMING("ReadNumber");

	uint32 n;
	bool overflow = false;
	const char *t;

	switch(base) {
	case 10:
		t = ScanUint<char, byte, uint32, 10>(n, term, overflow);
		break;
	case 8:
		t = ScanUint<char, byte, uint32, 8>(n, term, overflow);
		break;
	case 2:
		t = ScanUint<char, byte, uint32, 2>(n, term, overflow);
		break;
	case 16:
		t = ScanUint<char, byte, uint32, 16>(n, term, overflow);
		break;
	default:
		uint32 n = 0;
		int q = ctoi(*term);
		if(q < 0 || q >= base)
			ThrowError("missing number");
		for(;;) {
			int c = ctoi(*term);
			if(c < 0 || c >= base)
				break;
			uint32 n1 = n;
			n = base * n + c;
			if(n1 > n)
				ThrowError("number is too big");
			term++;
		}
		DoSpaces();
		return n;
	}
	if(!t) ThrowError("missing number");
	if(overflow) ThrowError("number is too big");
	term = t;
	DoSpaces();
	return n;
}

uint64  CParser::ReadNumber64(int base)
{
	LTIMING("ReadNumber64");

	uint32 n;
	bool overflow = false;
	const char *t;

	switch(base) {
	case 10:
		t = ScanUint<char, byte, uint32, 10>(n, term, overflow);
		break;
	case 8:
		t = ScanUint<char, byte, uint32, 8>(n, term, overflow);
		break;
	case 2:
		t = ScanUint<char, byte, uint32, 2>(n, term, overflow);
		break;
	case 16:
		t = ScanUint<char, byte, uint32, 16>(n, term, overflow);
		break;
	default:
		uint64 n = 0;
		int q = ctoi(*term);
		if(q < 0 || q >= base)
			ThrowError("missing number");
		for(;;) {
			int c = ctoi(*term);
			if(c < 0 || c >= base)
				break;
			uint64 n1 = n;
			n = base * n + c;
			if(n1 > n)
				ThrowError("number is too big");
			term++;
		}
		DoSpaces();
		return n;
	}
	if(!t) ThrowError("missing number");
	if(overflow) ThrowError("number is too big");
	term = t;
	DoSpaces();
	return n;
}

bool CParser::IsDouble2() const
{
	const char *t = term;
	if(*t == '-' || *t == '+') {
		t++;
		while(*t <= ' ')
			t++;
	}
	if(*t == '.') {
		t++;
		while(*t <= ' ')
			t++;
	}
	return IsDigit(*t);
}

double CParser::ReadDouble()
{
	LTIMING("ReadDouble");
	int sign = Sgn();
	if(!IsDigit(*term) && *term != '.')
		ThrowError("missing number");
	double n = 0;
	while(IsDigit(*term))
		n = 10 * n + *term++ - '0';
	if(Char('.')) {
		double q = 1;
		while(IsDigit(*term)) {
			q = 0.1 * q;
			n += q * (*term++ - '0');
		}
	}
	if(Char('e') || Char('E'))
		n *= pow(10.0, ReadInt());
	DoSpaces();
	n = sign * n;
	if(!IsFin(n))
		ThrowError("number is too big");
	return n;
}

dword CParser::ReadHex()
{
	int hex = 0;
	while(IsXDigit(*++term))
		hex = (hex << 4) + ctoi(*term);
	return hex;
}

bool CParser::ReadHex(dword& hex, int n)
{
	hex = 0;
	while(n--) {
		if(!IsXDigit(*++term))
			return false;
		hex = (hex << 4) + ctoi(*term);
	}
	term++;
	return true;
}

String CParser::ReadOneString(int delim, bool chkend) {
	if(!IsChar(delim))
		ThrowError("missing string");
	term++;
	StringBuffer result;
	for(;;) {
		if(*term == delim) {
			term++;
			DoSpaces();
			return String(result);
		}
		else
		if(*term == '\\') {
			switch(*++term) {
			case 'a': result.Cat('\a'); term++; break;
			case 'b': result.Cat('\b'); term++; break;
			case 't': result.Cat('\t'); term++; break;
			case 'v': result.Cat('\v'); term++; break;
			case 'n': result.Cat('\n'); term++; break;
			case 'r': result.Cat('\r'); term++; break;
			case 'f': result.Cat('\f'); term++; break;
			case 'x': {
				int hex = ReadHex();
				result.Cat(hex);
				break;
			}
			case 'u':
				if(uescape) {
					dword hex;
					if(!ReadHex(hex, 4))
						ThrowError("Incomplete universal character");
					if(hex >= 0xD800 && hex < 0xDBFF) {
						if(term[0] == '\\' && term[1] == 'u') {
							term++;
							dword hex2;
							if(!ReadHex(hex2, 4))
								ThrowError("Incomplete universal character");
							if(hex2 >= 0xDC00 && hex2 <= 0xDFFF) {
								result.Cat(ToUtf8(((hex & 0x3ff) << 10) | (hex2 & 0x3ff) + 0x10000));
								break;
							}
						}
						ThrowError("Invalid UTF-16 surrogate pair");
					}
					else
						result.Cat(ToUtf8(hex));
				}
				else
					result.Cat(*term++);
				break;
			case 'U':
				if(uescape) {
					dword hex;
					if(!ReadHex(hex, 8))
						ThrowError("Incomplete universal character");
					if(hex > 0x10ffff)
						ThrowError("Universal character is out of unicode range");
					result.Cat(ToUtf8(hex));
				}
				else
					result.Cat(*term++);
				break;
			default:
				if(*term >= '0' && *term <= '7') {
					int oct = *term++ - '0';
					if(*term >= '0' && *term <= '7')
						oct = 8 * oct + *term++ - '0';
					if(*term >= '0' && *term <= '7')
						oct = 8 * oct + *term++ - '0';
					result.Cat(oct);
				}
				else
					result.Cat(*term++);
				break;
			}
		}
		else {
			if((byte)*term < ' ' && *term != '\t') {
				if(chkend) {
					ThrowError("Unterminated string");
					return String(result);
				}
				if(*term == '\0')
					return String(result);
			}
			result.Cat(*term++);
		}
	}
	DoSpaces();
	return String(result);
}

String CParser::ReadOneString(bool chkend)
{
	return ReadOneString('\"', chkend);
}

String CParser::ReadString(int delim, bool chkend) {
	LTIMING("ReadString");
	String result;
	do
		result.Cat(ReadOneString(delim, chkend));
	while(IsChar(delim));
	return result;
}

String CParser::ReadString(bool chkend)
{
	return ReadString('\"', chkend);
}

char CParser::GetChar()
{
	char c = *term++;
	if(c == '\n') {
		line++;
		lineptr = term;
	}
	return c;
}

void CParser::SkipTerm()
{
	LTIMING("SkipTerm");
	if(IsId())
		while(iscid(*term))
			term++;
	else
	if(IsNumber())
		while(IsDigit(*term))
			term++;
	else
	if(IsString())
		ReadString();
	else
	if(IsChar('\''))
		ReadString('\'', false);
	else
	if(*term) {
		if(*term == '\n') {
			line++;
			lineptr = term + 1;
		}
		term++;
	}
	DoSpaces();
}

CParser::Pos CParser::GetPos() const
{
	Pos p;
	p.line = line;
	p.fn = fn;
	p.ptr = term;
	p.wspc = wspc;
	p.lineptr = lineptr;
	return p;
}

int CParser::Pos::GetColumn(int tabsize) const
{
	int pos = 1;
	for(const char *s = lineptr; s < ptr; s++) {
		if(*s == CParser::LINEINFO_ESC) {
			s++;
			while(s < ptr && *s != CParser::LINEINFO_ESC)
				if(*s++ == '\3')
					pos = atoi(s);
		}
		else
		if(*s == '\t')
			pos = (pos + tabsize - 1) / tabsize * tabsize + 1;
		else
			pos++;
	}
	return pos;
}

int CParser::GetColumn(int tabsize) const
{
	return GetPos().GetColumn(tabsize);
}

void CParser::SetPos(const CParser::Pos& p)
{
	LLOG("SetPos " << p.fn << ":" << p.line);
	line = p.line;
	fn = p.fn;
	term = p.ptr;
	wspc = p.wspc;
	lineptr = p.lineptr;
	if(skipspaces)
		DoSpaces();
}

CParser::CParser(const char *ptr)
: term(ptr), wspc(ptr), lineptr(ptr)
{
	line = 1;
	skipspaces = skipcomments = true;
	nestcomments = false;
	uescape = true;
	Spaces();
}

CParser::CParser(const char *ptr, const char *fn, int line)
: term(ptr), wspc(ptr), lineptr(ptr), line(line), fn(fn)
{
	skipspaces = skipcomments = true;
	nestcomments = false;
	uescape = true;
	Spaces();
}

CParser::CParser()
{
	term = lineptr = wspc = NULL;
	line = 0;
	skipspaces = skipcomments = true;
	nestcomments = false;
	uescape = true;
}

void CParser::Set(const char *_ptr, const char *_fn, int _line)
{
	term = lineptr = wspc = _ptr;
	fn = _fn;
	line = _line;
	if(skipspaces)
		Spaces();
	LLOG("Set " << fn << ":" << line);
}

void CParser::Set(const char *_ptr)
{
	Set(_ptr, "", 1);
}

inline void NextCStringLine(StringBuffer& t, const char *linepfx, int& pl)
{
	t << "\"\r\n" << (linepfx ? linepfx : "") << "\"";
	pl = t.GetLength();
}

inline int HexDigit(int c)
{
	return "0123456789ABCDEF"[c & 15];
}

static inline void sCatHex(StringBuffer& t, word q)
{
	char h[6];
	h[0] = '\\';
	h[1] = 'u';
	h[2] = HexDigit(q >> 12);
	h[3] = HexDigit(q >> 8);
	h[4] = HexDigit(q >> 4);
	h[5] = HexDigit(q);
	t.Cat(h, 6);
}

String AsCString(const char *s, const char *lim, int linemax, const char *linepfx, dword flags)
{
	StringBuffer t;
	t.Cat('\"');
	int pl = 0;
	bool wasspace = false;
	byte cs = GetDefaultCharset();
	bool toutf8 = GetDefaultCharset() != CHARSET_UTF8;
	while(s < lim) {
		if(t.GetLength() - pl > linemax && (!(flags & ASCSTRING_SMART) || wasspace))
			NextCStringLine(t, linepfx, pl);
		wasspace = *s == ' ';
		switch(*s) {
		case '\a': t.Cat("\\a"); break;
		case '\b': t.Cat("\\b"); break;
		case '\f': t.Cat("\\f"); break;
		case '\t': t.Cat("\\t"); break;
		case '\v': t.Cat("\\v"); break;
		case '\r': t.Cat("\\r"); break;
		case '\"': t.Cat("\\\""); break;
		case '\\': t.Cat("\\\\"); break;
		case '\n': t.Cat("\\n"); wasspace = true; break;
		default:
			if(flags & ASCSTRING_JSON) {
				if((byte)*s < 32) {
					sCatHex(t, (byte)*s++);
				}
				else
				if((byte)*s >= 0x7f) {
					if((byte)*s == 0x7f) {
						sCatHex(t, 0x7f);
						s++;
					}
					else {
						const char *s0 = s;
						dword c = toutf8 ? ToUnicode((byte)*s++, cs) : FetchUtf8(s, lim);
						if(c < 0x10000)
							t.Cat(s0, s);
						else {
							c -= 0x10000;
							sCatHex(t, wchar(0xD800 + (0x3ff & (c >> 10))));
							sCatHex(t, wchar(0xDC00 + (0x3ff & c)));
						}
					}
				}
				else
					t.Cat(*s++);
				continue; // skip s++
			}
			else {
				if(byte(*s) < 32 || (byte)*s >= 0x7f && (flags & ASCSTRING_OCTALHI) || (byte)*s == 0xff || (byte)*s == 0x7f) {
					char h[4];
					int q = (byte)*s;
					h[0] = '\\';
					h[1] = (3 & (q >> 6)) + '0';
					h[2] = (7 & (q >> 3)) + '0';
					h[3] = (7 & q) + '0';
					t.Cat(h, 4);
				}
				else
					t.Cat(*s);
			}
			break;
		}
		s++;
	}
	t.Cat('\"');
	return String(t);
}

String AsCString(const char *s, int linemax, const char *linepfx, dword flags)
{
	return AsCString(s, s + strlen(s), linemax, linepfx, flags);
}

String AsCString(const String& s, int linemax, const char *linepfx, dword flags)
{
	return AsCString(s, s.End(), linemax, linepfx, flags);
}

}
