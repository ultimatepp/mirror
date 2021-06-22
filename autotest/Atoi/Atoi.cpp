#include <Core/Core.h>

using namespace Upp;

int TestAtoi(const char *s)
{
	int result;
	s = ScanInt<char, byte, uint32, int, 10>(result, s);
	return s ? result : -999;
}

int64 TestAtoi64(const char *s)
{
	int64 result;
	s = ScanInt<char, byte, uint64, int64, 10>(result, s);
	return s ? result : -999;
}

int TestAtoiHex(const char *s)
{
	int result;
	s = ScanInt<char, byte, uint32, int, 16>(result, s);
	return s ? result : -999;
}

int TestAtoiOct(const char *s)
{
	int result;
	s = ScanInt<char, byte, uint32, int, 8>(result, s);
	return s ? result : -999;
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
	SeedRandom(0);
	
	for(int i = 0; i < 500000; i++) {
		int q = (int)Random();
		ASSERT(q == TestAtoi(AsString(q)));
		if(q >= 0) {
			ASSERT(q == TestAtoiHex(FormatIntHex(q)));
			ASSERT(q == TestAtoiOct(FormatIntOct(q, 8)));
		}
	}

	for(int i = 0; i < 500000; i++) {
		int64 q = Random64();
		ASSERT(q == TestAtoi64(AsString(q)));
	}
	
	DDUMP(TestAtoi("2147483647"));
	DDUMP(TestAtoi("2147483648"));

	DDUMP(TestAtoi("-2147483647"));
	DDUMP(TestAtoi("-2147483648"));
	DDUMP(TestAtoi("-2147483649"));

	DDUMP(TestAtoi("0"));
	DDUMP(TestAtoi("-0"));
	
	DDUMP(TestAtoi64("9223372036854775807"));
	DDUMP(TestAtoi64("9223372036854775808"));

	DDUMP(TestAtoi64("-9223372036854775807"));
	DDUMP(TestAtoi64("-9223372036854775808"));
	DDUMP(TestAtoi64("-9223372036854775809"));

	DDUMP(TestAtoi64("0"));
	DDUMP(TestAtoi64("-0"));

	DDUMP(TestAtoi64(""));
	DDUMP(TestAtoi64("z"));
	
	DDUMPHEX(TestAtoiHex("f"));
	DDUMPHEX(TestAtoiHex("7fffFFFF"));
	
	DDUMP(CParser("123456").ReadInt());
	DDUMP(CParser("123456").ReadInt64());
	DDUMP(CParser("-123456").ReadInt());
	DDUMP(CParser("-123456").ReadInt64());
	DDUMP(CParser("123456").ReadNumber());
	DDUMP(CParser("1010").ReadNumber(2));
	DDUMP(CParser("10").ReadNumber(4));
	DDUMP(CParser("123456").ReadNumber(8));
	DDUMP(CParser("123456").ReadNumber(16));
	DDUMP(CParser("123456").ReadNumber64());
	DDUMP(CParser("10").ReadNumber(4));
	DDUMP(CParser("1010").ReadNumber64(2));
	DDUMP(CParser("123456").ReadNumber64(8));
	DDUMP(CParser("123456").ReadNumber64(16));
	CParser p("1 2 3 4 5 6");
	DDUMP(p.ReadInt());
	DDUMP(p.ReadInt());
	DDUMP(p.ReadInt64());
	DDUMP(p.ReadInt64());
	DDUMP(p.ReadNumber());
	DDUMP(p.ReadNumber64());
	
	const char *s = "1 2";
	DDUMP(ScanInt(s, &s));
	DDUMP(ScanInt(s, &s));

	DDUMP(StrInt("123456"));
	DDUMP(StrInt("-123456"));
	
	s = "1 2";
	DDUMP(ScanInt64(s, &s));
	DDUMP(ScanInt64(s, &s));

	DDUMP(StrInt64("123456"));
	DDUMP(StrInt64("-123456"));
	
	try {
		CParser("123456123456123456").ReadInt();
	}
	catch(CParser::Error e) {
		DDUMP(e);
	}
	try {
		CParser("xx").ReadInt();
	}
	catch(CParser::Error e) {
		DDUMP(e);
	}
	try {
		CParser("xx").ReadInt64();
	}
	catch(CParser::Error e) {
		DDUMP(e);
	}
	try {
		CParser("123456123456123456").ReadInt();
	}
	catch(CParser::Error e) {
		DDUMP(e);
	}
	try {
		CParser("123456123456123456123456123456123456123456123456123456").ReadNumber64(8);
	}
	catch(CParser::Error e) {
		DDUMP(e);
	}
	try {
		CParser("123456123456123456123456123456123456123456123456123456").ReadNumber64(16);
	}
	catch(CParser::Error e) {
		DDUMP(e);
	}
	try {
		CParser("123456123456123456123456123456123456123456123456123456").ReadNumber64(10);
	}
	catch(CParser::Error e) {
		DDUMP(e);
	}
	DDUMP(CParser("123456123").ReadNumber64());
	DDUMP(CParser("123456123").ReadNumber64(8));
	DDUMP(CParser("123456123").ReadNumber64(16));
	
	CheckLogEtalon();
}
