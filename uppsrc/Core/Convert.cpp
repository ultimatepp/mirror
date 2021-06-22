#include "Core.h"

namespace Upp {

double ipow10_table[] = {
	1e-100, 1e-99, 1e-98, 1e-97, 1e-96, 1e-95, 1e-94, 1e-93, 1e-92, 1e-91, 1e-90, 1e-89, 1e-88,
	1e-87, 1e-86, 1e-85, 1e-84, 1e-83, 1e-82, 1e-81, 1e-80, 1e-79, 1e-78, 1e-77, 1e-76, 1e-75,
	1e-74, 1e-73, 1e-72, 1e-71, 1e-70, 1e-69, 1e-68, 1e-67, 1e-66, 1e-65, 1e-64, 1e-63, 1e-62,
	1e-61, 1e-60, 1e-59, 1e-58, 1e-57, 1e-56, 1e-55, 1e-54, 1e-53, 1e-52, 1e-51, 1e-50, 1e-49,
	1e-48, 1e-47, 1e-46, 1e-45, 1e-44, 1e-43, 1e-42, 1e-41, 1e-40, 1e-39, 1e-38, 1e-37, 1e-36,
	1e-35, 1e-34, 1e-33, 1e-32, 1e-31, 1e-30, 1e-29, 1e-28, 1e-27, 1e-26, 1e-25, 1e-24, 1e-23,
	1e-22, 1e-21, 1e-20, 1e-19, 1e-18, 1e-17, 1e-16, 1e-15, 1e-14, 1e-13, 1e-12, 1e-11, 1e-10,
	1e-9, 1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1, 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6,
	1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22,
	1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30, 1e31, 1e32, 1e33, 1e34, 1e35, 1e36, 1e37,
	1e38, 1e39, 1e40, 1e41, 1e42, 1e43, 1e44, 1e45, 1e46, 1e47, 1e48, 1e49, 1e50, 1e51, 1e52,
	1e53, 1e54, 1e55, 1e56, 1e57, 1e58, 1e59, 1e60, 1e61, 1e62, 1e63, 1e64, 1e65, 1e66, 1e67,
	1e68, 1e69, 1e70, 1e71, 1e72, 1e73, 1e74, 1e75, 1e76, 1e77, 1e78, 1e79, 1e80, 1e81, 1e82,
	1e83, 1e84, 1e85, 1e86, 1e87, 1e88, 1e89, 1e90, 1e91, 1e92, 1e93, 1e94, 1e95, 1e96, 1e97,
	1e98, 1e99, 1e100
};

unsigned stou(const char *s, void *endptr, unsigned base)
{
	ASSERT(base >= 2 && base <= 36);
	unsigned digit = ctoi(*s);
	if(digit >= base)
	{ // error
		if(endptr)
			*(const char **)endptr = s;
		return ~0;
	}
	unsigned value = digit;
	while((digit = ctoi(*++s)) < base) {
		unsigned v0 = value;
		value = value * base + digit;
		if(v0 > value) // overflow
			return ~0;
	}
	if(endptr)
		*(const char **)endptr = s;
	return value;
}

unsigned stou(const wchar *s, void *endptr, unsigned base)
{
	ASSERT(base >= 2 && base <= 36);
	unsigned digit = ctoi(*s);
	if(digit >= base)
	{ // error
		if(endptr)
			*(const wchar **)endptr = s;
		return ~0;
	}
	unsigned value = digit;
	while((digit = ctoi(*++s)) < base) {
		unsigned v0 = value;
		value = value * base + digit;
		if(v0 > value) // overflow
			return ~0;
	}
	if(endptr)
		*(const wchar **)endptr = s;
	return value;
}


uint64 stou64(const char *s, void *endptr, unsigned base)
{
	ASSERT(base >= 2 && base <= 36);
	unsigned digit = ctoi(*s);
	if(digit >= base)
	{ // error
		if(endptr)
			*(const char **)endptr = s;
		return ~0;
	}
	uint64 value = digit;
	while((digit = ctoi(*++s)) < base) {
		uint64 v0 = value;
		value = value * base + digit;
		if(v0 > value) // overflow
			return ~0;
	}
	if(endptr)
		*(const char **)endptr = s;
	return value;
}

int ScanInt(const char *ptr, const char **endptr, int base)
{
	const char *s = ptr;
	bool minus = false;
	while(*s && (byte)*s <= ' ')
		s++;
	if(*s == '+' || *s == '-')
		minus = (*s++ == '-');
	unsigned u = stou(s, endptr, base);
	if(~u)
		return (minus ? -(int)u : (int)u);
	else
		return Null;
}

int ScanInt(const wchar *ptr, const wchar **endptr, int base)
{
	const wchar *s = ptr;
	bool minus = false;
	while(*s && *s <= ' ')
		s++;
	if(*s == '+' || *s == '-')
		minus = (*s++ == '-');
	unsigned u = stou(s, endptr, base);
	if(~u)
		return (minus ? -(int)u : (int)u);
	else
		return Null;
}

int64 ScanInt64(const char *ptr, const char **endptr, int base)
{
	const char *s = ptr;
	bool minus = false;
	while(*s && *s <= ' ')
		s++;
	if(*s == '+' || *s == '-')
		minus = (*s++ == '-');
	uint64 u = stou64(s, endptr, base);
	if(~u)
		return (minus ? -(int64)u : (int64)u);
	else
		return Null;
}

int ScanInt(const char *ptr, const char **endptr)
{
	int x;
	bool overflow = false;
	ptr = ScanInt<char, byte, uint32, int, 10>(x, ptr, overflow);
	if(ptr && endptr)
		*endptr = ptr;
	return !overflow && ptr ? x : Null;
}

int ScanInt(const char *ptr)
{
	int x;
	bool overflow = false;
	return ScanInt<char, byte, uint32, int, 10>(x, ptr, overflow) && !overflow ? x : Null;
}

int64 ScanInt64(const char *ptr, const char **endptr)
{
	int64 x;
	bool overflow = false;
	ptr = ScanInt<char, byte, uint64, int64, 10>(x, ptr, overflow);
	if(ptr && endptr)
		*endptr = ptr;
	return !overflow && ptr ? x : Null;
}

int64 ScanInt64(const char *ptr)
{
	int64 x;
	bool overflow = false;
	return ScanInt<char, byte, uint64, int64, 10>(x, ptr, overflow) && !overflow ? x : Null;
}

template <class T>
double ScanDoubleT(const T *p, const T **endptr, bool accept_comma)
{
	const T *begin = p;
	while(*p && (byte)*p <= ' ')
		p++;
	bool neg = false;
	if(endptr)
		*endptr = p;
	if(*p == '+' || *p == '-')
		neg = (*p++ == '-');
	if((byte)(*p - '0') >= 10 && !((*p == '.' || accept_comma && *p == ',') && (byte)(p[1] - '0') < 10)) {
		if(endptr) *endptr = begin;
		return Null;
	}
	double mantissa = 0;
	T c;
	int exp = 0;
	while((byte)(*p - '0') < 10)
		if((c = *p++) != '0') {
			if(exp) { mantissa *= ipow10(exp); exp = 0; }
			mantissa = 10 * mantissa + c - '0';
		}
		else
			exp++;
	int raise = exp;
	if(*p == '.' || accept_comma && *p == ',') // decimal part
		while((byte)((c = *++p) - '0') < 10) {
			if(c != '0') {
				if(raise) {
					mantissa *= ipow10(raise);
					exp -= raise;
					raise = 0;
				}
				exp--;
				mantissa = 10 * mantissa + c - '0';
				if(!IsFin(mantissa))
					return Null;
			}
			else
				raise++;
		}
	if(*p == 'E' || *p == 'e') { // exponent
		int vexp = ScanInt(p + 1, &p);
		if(IsNull(vexp))
			return Null;
		exp += vexp;
	}
	if(endptr) *endptr = p;
	if(exp) {
		double e = ipow10(tabs(exp));
		mantissa = (exp > 0 ? mantissa * e : mantissa / e);
	}
	if(!IsFin(mantissa))
		return Null;
	return neg ? -mantissa : mantissa;
}

double ScanDouble(const char *p, const char **endptr, bool accept_comma)
{
	return ScanDoubleT(p, endptr, accept_comma);
}

double ScanDouble(const wchar *p, const wchar **endptr, bool accept_comma)
{
	return ScanDoubleT(p, endptr, accept_comma);
}

double Atof(const char *s)
{
	return Nvl(ScanDouble(s));
}

Value StrIntValue(const char *s)
{
	if(s && *s) {
		const char *p;
		int64 q = ScanInt64(s, &p);
		if(!IsNull(q))
			while(*p) {
				if((byte)*p > ' ')
					return ErrorValue(t_("Invalid number !"));
				p++;
			}
		return IsNull(q) ? ErrorValue(t_("Invalid number !")) : Value(q);
	}
	return (int)Null;
}

Value StrDblValue(const char *s)
{
	if(s && *s) {
		const char *p;
		double q = ScanDouble(s, &p);
		if(!IsNull(q))
			while(*p) {
				if((byte)*p > ' ')
					return ErrorValue(t_("Invalid number !"));
				p++;
			}
		return IsNull(q) ? ErrorValue(t_("Invalid number !")) : Value(q);
	}
	return (double)Null;
}

Value Scan(dword qtype, const String& text, const Value& def, bool *hastime) {
	Date date;
	const char *s;
	if(hastime)
		*hastime = false;
	switch(qtype) {
	case INT64_V:
	case INT_V:
	case BOOL_V:
		return StrIntValue(text);
	case DATE_V:
		if(text.IsEmpty()) return (Date) Null;
		s = StrToDate(date, text, (Date)def);
		if(s)
			for(;;) {
				if(IsDigit(*s))
					break;
				if(*s == '\0')
					return date;
				s++;
			}
		return ErrorValue(t_("Invalid date !"));
	case TIME_V: {
		if(text.IsEmpty()) return (Time) Null;
		s = StrToDate(date, text, (Date)def);
		if(s)
			try {
				CParser p(s);
				Time tm = ToTime(date);
				Time d = (Time)def;
				tm.hour = d.hour;
				tm.minute = d.minute;
				tm.second = d.second;
				if(p.IsEof())
					return tm;
				if(hastime)
					*hastime = true;
				int q = p.ReadInt();
				if(q < 0 || q > 23)
					throw CParser::Error("");
				tm.hour = q;
				if(p.IsEof())
					return tm;
				p.PassChar(':');
				q = p.ReadInt();
				if(q < 0 || q > 59)
					throw CParser::Error("");
				tm.minute = q;
				if(p.IsEof())
					return tm;
				p.PassChar(':');
				q = p.ReadInt();
				if(q < 0 || q > 59)
					throw CParser::Error("");
				tm.second = q;
				if(p.IsEof())
					return tm;
			}
			catch(CParser::Error) {}
		return ErrorValue(t_("Invalid time !"));
	}
	case STRING_V:
	case WSTRING_V:
		return text;
	case DOUBLE_V:
		return StrDblValue(text);
	default:
		ASSERT(0);
		break;
	}
	return Null;
}

Convert::Convert() {}
Convert::~Convert() {}

Value  Convert::Format(const Value& q) const {
	if(IsVoid(q) || q.IsNull()) return String();
	switch(q.GetType()) {
	case INT64_V:
		return IntStr64((int64)q);
	case INT_V:
	case BOOL_V:
		return IntStr((int)q);
	case DOUBLE_V:
		return DblStr((double)q);
	case DATE_V:
		return UPP::Format(Date(q));
	case TIME_V:
		return UPP::Format(Time(q));
	case STRING_V:
	case WSTRING_V:
		return q;
	}
	return q.ToString();
}

Value  Convert::Scan(const Value& text) const {
	return text;
};

int    Convert::Filter(int chr) const {
	return chr;
}

const Convert& StdConvert()
{
	static Convert h;
	return h;
}

String StdFormat(const Value& q) {
	return StdConvert().Format(q);
}

Value NotNullError() {
	return ErrorValue(t_("Null value not allowed."));
}

Value ConvertInt::Scan(const Value& text) const {
	Value v = UPP::Scan(INT_V, text);
	if(IsError(v)) return v;
	if(IsNull(v)) return notnull ? NotNullError() : v;
	int64 m = v;
	if(m >= minval && m <= maxval) {
		if(m >= INT_MIN && m <= INT_MAX)
			return (int)m;
		else
			return v;
	}
	return ErrorValue(UPP::Format(t_("Number must be between %d and %d."), minval, maxval));
}

int   ConvertInt::Filter(int chr) const {
	return minval >= 0 ? CharFilterDigit(chr) : CharFilterInt(chr);
}

Value ConvertDouble::Format(const Value& q) const
{
	if(IsNull(q))
		return Null;
	return UPP::Format(pattern, (double)q);
}

Value ConvertDouble::Scan(const Value& txt) const {
	String text = txt;
	if(pattern.GetCount() && pattern != "%.10g") { // Fix text with patterns like "%2.!n EUR" (e.g. 1.2 EUR)
		text = UPP::Filter(text, CharFilterDouble);
		while(ToUpper(*text.Last()) == 'E')
			text.Trim(text.GetCount() - 1);
	}
	Value v = UPP::Scan(DOUBLE_V, text);
	if(IsError(v)) return v;
	if(IsNull(v)) return notnull ? NotNullError() : v;
	double m = v;
	if(m >= minval && m <= maxval) return v;
	return ErrorValue(UPP::Format(t_("Number must be between %g and %g."), minval, maxval));
}

int   ConvertDouble::Filter(int chr) const {
	chr = CharFilterDouble(chr);
	return comma && chr == '.' ? ',' : chr;
}

ConvertDouble::ConvertDouble(double minval, double maxval, bool notnull)
  : minval(minval), maxval(maxval), notnull(notnull)
{
	pattern = "%.10g";
	comma = false;
}

ConvertDouble& ConvertDouble::Pattern(const char *p)
{
	pattern = p;
	comma = String(Format(1.1)).Find(',') >= 0;
	return *this;
}

Date& ConvertDate::default_min()
{
	static Date v = Date::Low();
	return v;
}

Date& ConvertDate::default_max()
{
	static Date v = Date::High();
	return v;
}

void ConvertDate::SetDefaultMinMax(Date min, Date max)
{
	default_min() = min;
	default_max() = max;
}

ConvertDate::ConvertDate(Date minval, Date maxval, bool notnull)
: minval(minval), maxval(maxval), notnull(notnull) {
	defaultval = Null;
}

Value ConvertDate::Format(const Value& q) const
{
	if(IsDateTime(q))
		return Convert::Format((Date)q);
	return Convert::Format(q);
}

Value ConvertDate::Scan(const Value& text) const {
	Value v = UPP::Scan(DATE_V, text, defaultval);
	if(IsError(v)) return v;
	if(IsNull(v)) return notnull ? NotNullError() : v;
	Date m = v;
	Date minval = GetMin();
	Date maxval = GetMax();
	if(m >= minval && m <= maxval) return v;
	return ErrorValue(t_("Date must be between ") + UPP::Format(minval) + t_("range\v and ") + UPP::Format(maxval) + ".");
}

int   ConvertDate::Filter(int chr) const {
	return CharFilterDate(chr);
}

ConvertTime::ConvertTime(Time minval, Time maxval, bool notnull)
: minval(minval), maxval(maxval), notnull(notnull), seconds(true) {
	defaultval = Null;
	timealways = false;
	dayend = false;
}

ConvertTime::~ConvertTime()
{
}

Value ConvertTime::Scan(const Value& text) const
{
	bool hastime;
	Value v = UPP::Scan(TIME_V, text, defaultval, &hastime);
	if(IsError(v)) return v;
	if(IsNull(v)) return notnull ? NotNullError() : v;
	Time m = v;
	if(!hastime && dayend) {
		m.hour = 23;
		m.minute = 59;
		m.second = 59;
		v = m;
	}
	Time minval = GetMin();
	Time maxval = GetMax();
	if(m >= minval && m <= maxval) return v;
	return ErrorValue(t_("Time must be between ") + UPP::Format(minval) + t_("range\v and ") + UPP::Format(maxval) + ".");
}

int ConvertTime::Filter(int chr) const
{
	if(IsDigit(chr) || chr == ' ' || chr == '.' || chr == ':')
		return chr;
	if(chr == ',')
		return '.';
	return CharFilterDate(chr);
}

Value ConvertTime::Format(const Value& q) const
{
	if(IsVoid(q) || q.IsNull())
		return String();
	else
	if(q.GetType() == TIME_V || timealways)
		return ToTime((Date)q) != (Time)q || timealways ? UPP::Format(Time(q), seconds) : UPP::Format(Date(q));
	else
		return Convert::Format(q);
}

Value ConvertString::Scan(const Value& text) const {
	if(IsError(text)) return text;
	if(text.GetType() == STRING_V) {
		String s = text;
		if(trimleft)
			s = Upp::TrimLeft(s);
		if(trimright)
			s = Upp::TrimRight(s);
		if(IsNull(s)) return notnull ? NotNullError() : Value(s);
		if(s.GetLength() <= maxlen) return s;
	}
	if(text.GetType() == WSTRING_V) {
		WString s = text;
		if(trimleft)
			s = Upp::TrimLeft(s);
		if(trimright)
			s = Upp::TrimRight(s);
		if(IsNull(s)) return notnull ? NotNullError() : Value(s);
		if(s.GetLength() <= maxlen) return s;
	}
	return ErrorValue(UPP::Format(t_("Please enter no more than %d characters."), maxlen));
}

const ConvertInt& StdConvertInt() { static ConvertInt h; return h; }
const ConvertInt& StdConvertIntNotNull() { static ConvertInt h(-INT_MAX, INT_MAX, true); return h; }

const ConvertDouble& StdConvertDouble() { static ConvertDouble h; return h; }
const ConvertDouble& StdConvertDoubleNotNull() { static ConvertDouble h(DOUBLE_NULL_LIM, -DOUBLE_NULL_LIM, true); return h; }

const ConvertDate& StdConvertDate() { static ConvertDate h; return h; }
const ConvertDate& StdConvertDateNotNull() { static ConvertDate h(Date(0, 0, 0), Date(3000, 12, 31), true); return h; }

const ConvertTime& StdConvertTime() { static ConvertTime h; return h; }
const ConvertTime& StdConvertTimeNotNull() { static ConvertTime h(Null, Null, true); return h; }

const ConvertString& StdConvertString() { static ConvertString h; return h; }
const ConvertString& StdConvertStringNotNull() { static ConvertString h(INT_MAX, true); return h; }

Value  MapConvert::Format(const Value& q) const {
	return map.Get(q, default_value);
}

NoConvertClass::NoConvertClass() {}

Value NoConvertClass::Format(const Value& q) const {
	return q;
}

const NoConvertClass& NoConvert() {
	return Single<NoConvertClass>();
}

Value ErrorConvertClass::Scan(const Value& v) const
{
	return ErrorValue();
}

const ErrorConvertClass& ErrorConvert()
{
	return Single<ErrorConvertClass>();
}

Value JoinConvert::Format(const Value& v) const {
	String r;
	ValueArray a = v;
	for(int i = 0; i < item.GetCount(); i++) {
		const Item& m = item[i];
		if(m.pos < 0)
			r << m.text;
		else
			r << (String) StdConvert().Format(m.convert->Format(a[m.pos]));
	}
	return r;
}

int JoinConvert::NextPos() const {
	for(int i = item.GetCount() - 1; i >= 0; i--)
		if(item[i].pos >= 0) return item[i].pos + 1;
	return 0;
}

JoinConvert& JoinConvert::Add(const char *text) {
	Item& m = item.Add();
	m.pos = -1;
	m.text = text;
	return *this;
}

JoinConvert& JoinConvert::Add(int pos, const Convert& cv) {
	ASSERT(pos >= 0);
	Item& m = item.Add();
	m.pos = pos;
	m.convert = &cv;
	return *this;
}

JoinConvert& JoinConvert::Add(int pos) {
	Add(pos, StdConvert());
	return *this;
}

JoinConvert& JoinConvert::Add(const Convert& cv) {
	Add(NextPos(), cv);
	return *this;
}

JoinConvert& JoinConvert::Add() {
	Add(NextPos());
	return *this;
}

Value FormatConvert::Format(const Value& v) const
{
	ValueArray va;
	if(IsValueArray(v))
		va = v;
	else
		va.Add(v);
	return UPP::Format(format, va.Get());
}

}
