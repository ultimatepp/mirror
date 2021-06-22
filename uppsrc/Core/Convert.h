unsigned      stou(const char *ptr, void *endptr = NULL, unsigned radix = 10);
inline unsigned stou(const byte *ptr, void *endptr = NULL, unsigned radix = 10) { return stou((const char *)ptr, endptr, radix); }
unsigned      stou(const wchar *ptr, void *endptr = NULL, unsigned radix = 10);

uint64        stou64(const char *s, void *endptr = NULL, unsigned radix = 10);
uint64        stou64(const wchar *s, void *endptr = NULL, unsigned radix = 10);

#include "Convert.hpp"

int           ScanInt(const wchar *ptr, const wchar **endptr = NULL, int radix = 10);

int           ScanInt(const char *ptr, const char **endptr, int radix);
int           ScanInt(const char *ptr, const char **endptr);
int           ScanInt(const char *ptr);

int64         ScanInt64(const char *ptr, const char **endptr, int radix);
int64         ScanInt64(const char *ptr, const char **endptr);
int64         ScanInt64(const char *ptr);

double        ScanDouble(const char *ptr, const char **endptr = NULL, bool accept_comma = true);
double        ScanDouble(const wchar *ptr, const wchar **endptr = NULL, bool accept_comma = true);

double        Atof(const char *s);

Value         StrIntValue(const char *s);

inline int     StrInt(const char* s)   { return ScanInt(s); }
inline String  IntStr(int i)           { return FormatInt(i); }

inline int64   StrInt64(const char *s) { return ScanInt64(s); }
inline String  IntStr64(int64 i)       { return FormatInt64(i); }

inline double  StrDbl(const char* s)   { return ScanDouble(s); }
inline String  DblStr(double d)        { return FormatDouble(d, 10); }

inline double  IntDbl(int i)           { return IsNull(i) ? double(Null) : double(i); }
inline int     DblInt(double d)        { return IsNull(d) ? int(Null) : fround(d); }

Value          StrDblValue(const char* s);


Value NotNullError();

class Convert {
public:
	Convert();
	virtual ~Convert();

	virtual Value  Format(const Value& q) const;
	virtual Value  Scan(const Value& text) const;
	virtual int    Filter(int chr) const;

	Value operator()(const Value& q) const              { return Format(q); }
};

const Convert& StdConvert();

String StdFormat(const Value& q);

class ConvertInt : public Convert {
public:
	virtual Value Scan(const Value& text) const;
	virtual int   Filter(int chr) const;

protected:
	int64 minval, maxval;
	bool  notnull;

public:
	ConvertInt& MinMax(int _min, int _max)        { minval = _min; maxval = _max; return *this; }
	ConvertInt& Min(int _min)                     { minval = _min; return *this; }
	ConvertInt& Max(int _max)                     { maxval = _max; return *this; }
	ConvertInt& NotNull(bool b = true)            { notnull = b; return *this; }
	ConvertInt& NoNotNull()                       { return NotNull(false); }
	int         GetMin() const                    { return (int)minval; }
	int         GetMax() const                    { return (int)maxval; }
	bool        IsNotNull() const                 { return notnull; }
	
	static int  GetDefaultMin()                   { return -INT_MAX; }
	static int  GetDefaultMax()                   { return INT_MAX; }

	ConvertInt(int minval = -INT_MAX, int maxval = INT_MAX, bool notnull = false)
		: minval(minval), maxval(maxval), notnull(notnull) {}
};

const ConvertInt& StdConvertInt();
const ConvertInt& StdConvertIntNotNull();

struct ConvertInt64 : public ConvertInt {
	ConvertInt64& MinMax(int64 _min, int64 _max)    { minval = _min; maxval = _max; return *this; }
	ConvertInt64& Min(int64 _min)                   { minval = _min; return *this; }
	ConvertInt64& Max(int64 _max)                   { maxval = _max; return *this; }
	int64         GetMin() const                    { return minval; }
	int64         GetMax() const                    { return maxval; }

	static int64  GetDefaultMin()                   { return -INT64_MAX; }
	static int64  GetDefaultMax()                   { return INT64_MAX; }

	ConvertInt64(int64 minval = -INT64_MAX, int64 maxval = INT64_MAX, bool notnull = false) {
		MinMax(minval, maxval); NotNull(notnull);
	}
};

class ConvertDouble : public Convert {
public:
	virtual Value Format(const Value& q) const;
	virtual Value Scan(const Value& text) const;
	virtual int   Filter(int chr) const;

protected:
	double      minval, maxval;
	bool        notnull, comma;
	String      pattern;

public:
	ConvertDouble& Pattern(const char *p);
	ConvertDouble& MinMax(double _min, double _max)  { minval = _min; maxval = _max; return *this; }
	ConvertDouble& Min(double _min)                  { minval = _min; return *this; }
	ConvertDouble& Max(double _max)                  { maxval = _max; return *this; }
	ConvertDouble& NotNull(bool b = true)            { notnull = b; return *this; }
	ConvertDouble& NoNotNull()                       { return NotNull(false); }
	double         GetMin() const                    { return minval; }
	double         GetMax() const                    { return maxval; }
	bool           IsNotNull() const                 { return notnull; }

	static double  GetDefaultMin()                   { return DOUBLE_NULL_LIM; }
	static double  GetDefaultMax()                   { return -DOUBLE_NULL_LIM; }

	ConvertDouble(double minval = DOUBLE_NULL_LIM, double maxval = -DOUBLE_NULL_LIM,
		          bool notnull = false);
};

const ConvertDouble& StdConvertDouble();
const ConvertDouble& StdConvertDoubleNotNull();

class ConvertDate : public Convert {
public:
	virtual Value Format(const Value& q) const;
	virtual Value Scan(const Value& text) const;
	virtual int   Filter(int chr) const;

protected:
	Date minval, maxval, defaultval;
	bool notnull;
	
	static Date& default_min();
	static Date& default_max();

public:
	ConvertDate& MinMax(Date _min, Date _max)      { minval = _min; maxval = _max; return *this; }
	ConvertDate& Min(Date _min)                    { minval = _min; return *this; }
	ConvertDate& Max(Date _max)                    { maxval = _max; return *this; }
	ConvertDate& NotNull(bool b = true)            { notnull = b; return *this; }
	ConvertDate& NoNotNull()                       { return NotNull(false); }
	ConvertDate& Default(Date d)                   { defaultval = d; return *this; }
	bool         IsNotNull() const                 { return notnull; }
	Date         GetMin() const                    { return max(GetDefaultMin(), minval); }
	Date         GetMax() const                    { return min(GetDefaultMax(), maxval); }

	static void  SetDefaultMinMax(Date min, Date max);
	static Date  GetDefaultMin()                   { return default_min(); }
	static Date  GetDefaultMax()                   { return default_max(); }

	ConvertDate(Date minval = Date::Low(), Date maxval = Date::High(), bool notnull = false);
};

const ConvertDate& StdConvertDate();
const ConvertDate& StdConvertDateNotNull();

class ConvertTime : public Convert {
public:
	virtual Value Scan(const Value& text) const;
	virtual int   Filter(int chr) const;
	virtual Value Format(const Value& q) const;

protected:
	Time minval, maxval, defaultval;
	bool notnull;
	bool seconds;
	bool timealways;
	bool dayend;

public:
	ConvertTime& MinMax(Time _min, Time _max)      { minval = _min; maxval = _max; return *this; }
	ConvertTime& Min(Time _min)                    { minval = _min; return *this; }
	ConvertTime& Max(Time _max)                    { maxval = _max; return *this; }
	ConvertTime& NotNull(bool b = true)            { notnull = b; return *this; }
	ConvertTime& NoNotNull()                       { return NotNull(false); }
	ConvertTime& Seconds(bool b = true)            { seconds = b; return *this; }
	ConvertTime& NoSeconds()                       { return Seconds(false); }
	bool         IsSeconds() const                 { return seconds; }
	ConvertTime& TimeAlways(bool b = true)         { timealways = b; return *this; }
	bool         IsTimeAlways() const              { return timealways; }
	ConvertTime& DayEnd(bool b = true)             { dayend = b; return *this; }
	bool         IsDayEnd() const                  { return dayend; }
	ConvertTime& Default(Time d)                   { defaultval = d; return *this; }
	bool         IsNotNull() const                 { return notnull; }

	Time         GetMin() const                    { return max(minval, GetDefaultMin()); }
	Time         GetMax() const                    { return min(maxval, GetDefaultMax()); }

	static Time  GetDefaultMin()                   { return ToTime(ConvertDate::GetDefaultMin()); }
	static Time  GetDefaultMax()                   { return ToTime(ConvertDate::GetDefaultMax()); }

	ConvertTime(Time minval = ToTime(Date::Low()), Time maxval = ToTime(Date::High()), bool notnull = false);
	virtual ~ConvertTime();
};

const ConvertTime& StdConvertTime();
const ConvertTime& StdConvertTimeNotNull();

class ConvertString : public Convert {
public:
	virtual Value Scan(const Value& text) const;

protected:
	int  maxlen;
	bool notnull, trimleft, trimright;

public:
	ConvertString& MaxLen(int _maxlen)             { maxlen = _maxlen; return *this; }
	int            GetMaxLength() const            { return maxlen; }
	ConvertString& NotNull(bool b = true)          { notnull = b; return *this; }
	ConvertString& NoNotNull()                     { return NotNull(false); }
	bool           IsNotNull() const               { return notnull; }
	ConvertString& TrimLeft(bool b = true)         { trimleft = b; return *this; }
	ConvertString& TrimRight(bool b = true)        { trimright = b; return *this; }
	ConvertString& TrimBoth(bool b = true)         { return TrimLeft(b).TrimRight(b); }
	bool           IsTrimLeft() const              { return trimleft; }
	bool           IsTrimRight() const             { return trimright; }

	ConvertString(int maxlen = INT_MAX, bool notnull = false)
		: maxlen(maxlen), notnull(notnull) { trimleft = trimright = false; }
};

const ConvertString& StdConvertString();
const ConvertString& StdConvertStringNotNull();

class NoConvertClass : public Convert {
public:
	NoConvertClass();

	virtual Value  Format(const Value& q) const;
};

const NoConvertClass& NoConvert();

class ErrorConvertClass : public Convert {
public:
	Value Scan(const Value& v) const;
};

const ErrorConvertClass& ErrorConvert();

class MapConvert : public Convert {
public:
	virtual Value  Format(const Value& q) const;

protected:
	VectorMap<Value, Value> map;
	Value                   default_value;

public:
	void         Clear()                                 { map.Clear(); }
	MapConvert&  Add(const Value& a, const Value& b)     { map.Add(a, b); return *this; }
	MapConvert&  Default(const Value& v)                 { default_value = v; return *this; }

	int          GetCount() const                        { return map.GetCount(); }
	int          Find(const Value& v) const              { return map.Find(v); }
	const Value& GetKey(int i) const                     { return map.GetKey(i); }
	const Value& GetValue(int i) const                   { return map[i]; }
	const Value& operator[](int i) const                 { return map[i]; }

	virtual ~MapConvert() {}
};

class JoinConvert : public Convert {
public:
	virtual Value Format(const Value& v) const;

protected:
	struct Item {
		int            pos;
		const Convert *convert;
		String         text;
	};
	Array<Item> item;

	int NextPos() const;

public:
	JoinConvert& Add(const char *text);
	JoinConvert& Add(int pos, const Convert& cv);
	JoinConvert& Add(int pos);
	JoinConvert& Add(const Convert& cv);
	JoinConvert& Add();
};

class FormatConvert : public Convert {
public:
	virtual Value Format(const Value& v) const;

private:
	String format;

public:
	void   SetFormat(const char *fmt)           { format = fmt; }
};

Convert& LNGConvert();

template <typename F, typename S, class R>
struct LambdaConvertClass : Convert {
	Function<Value(const Value& w)> format;
	Function<Value(const Value& text)> scan;
	Function<int(int)> filter;

	virtual Value Format(const Value& q) const { return format(q); }
	virtual Value Scan(const Value& text) const { return scan(text); }
	virtual int Filter(int chr) const { return filter(chr); }
	
	LambdaConvertClass(F format, S scan, R filter) : format(format), scan(scan), filter(filter) {}
};

template <typename F, typename S, class R>
const auto& LambdaConvert(F format, S scan, R filter)
{
	static LambdaConvertClass<F, S, R> x(format, scan, filter);
	return x;
}

template <typename F, typename S>
const auto& LambdaConvert(F format, S scan)
{
	return LambdaConvert(format, scan, [](int ch) { return ch; });
}

template <typename F>
const auto& LambdaConvert(F format)
{
	return LambdaConvert(format, [](const Value& v) { return v; });
}
