#include <Core/Core.h>

using namespace Upp;

double Atoff(const char *s)
{
	double result;
	return ScanDbl<char, byte>(result, s) ? result : -999;
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
	SeedRandom(0);
	for(int i = 0; i < 100000; i++) {
		double x = (2 * Randomf() - 1) * pow(10, (int)Random(600) - 300);
		String s = AsString(x);
		double y = Atoff(s);
//		DDUMP(s);
//		DDUMP(atof(s));
//		DDUMP(y);
		ASSERT(abs((x - y) / x) < 0.00000000000001);
	}
	
	
	
	DDUMP(ScanDouble(".23"));
	DDUMP(ScanDouble("1.23"));
	DDUMP(ScanDouble("-1.23e-33"));
	DDUMP(ScanDouble("z"));
	DDUMP(ScanDouble("1,23e33", 0, true));
	DDUMP(ScanDouble("1,23e33", 0, false));

	DDUMP(StrDbl("1.23"));
	DDUMP(StrDbl("1,23e-33"));
	
	const char ns[] = "1 2 3 1.23 1.234e10 -1.345e-10 .23 1,23 ,23";
	
	DLOG("--------");
	const char *s = ns;
	for(;;) {
		double x = ScanDouble(s, &s);
		if(IsNull(x))
			break;
		DDUMP(x);
	}

	DLOG("--------");
	s = ns;
	for(;;) {
		double x = ScanDouble(s, &s, true);
		if(IsNull(x))
			break;
		DDUMP(x);
	}

	DLOG("--------");
	s = ns;
	for(;;) {
		double x = ScanDouble(s, &s, false);
		if(IsNull(x))
			break;
		DDUMP(x);
	}

	DLOG("--------");
	
	DDUMP(CParser("1").ReadDouble());
	DDUMP(CParser("1.23").ReadDouble());
	DDUMP(CParser("1.23e-33").ReadDouble());
	DDUMP(CParser("-1.23e-33").ReadDouble());
	
	DLOG("--------");
	{
		CParser p(ns);
		while(p.IsDouble()) {
			DDUMP(p.ReadDouble());
		}
	}

	DLOG("--------");
	{
		CParser p(ns);
		while(p.IsDouble2()) {
			DDUMP(p.ReadDouble());
		}
	}
	
	DLOG("--------");
	try {
		CParser("x").ReadDouble();
	}
	catch(Exc e) {
		DDUMP(e);
	}
	
	DDUMP(Atof("x"));
	DDUMP(Atof("-1.234E+50"));
	
	CheckLogEtalon();
}
