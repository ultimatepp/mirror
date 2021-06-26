#include <Core/Core.h>

using namespace Upp;

void Test(Convert& cv, const char *s, Value res)
{
	Value v = cv.Scan(s);
	LOG(s << ": " << v);
	if(IsError(res))
		ASSERT(IsError(v));
	else
	if(IsNumber(v)) {
		double x = res;
		double y = v;
		if(y == 0)
			ASSERT(x == 0);
		else
			ASSERT(abs((x - y) / x) < 0.00000000000001);
	}
	else
		ASSERT(res == v);
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_FILE|LOG_COUT);

	SetLanguage(LNG_('C','S','C','Z'));

	ConvertTime cv;

	DUMP(cv.Format(Date(2013, 1, 2)));
	ASSERT(cv.Format(Date(2013, 1, 2)) == "02.01.2013");

	DUMP(cv.Format(Time(2013, 1, 2)));
	ASSERT(cv.Format(Time(2013, 1, 2)) == "02.01.2013");

	DUMP(cv.Format(Time(2013, 1, 2, 11, 12, 13)));
	ASSERT(cv.Format(Time(2013, 1, 2, 11, 12, 13)) == "02.01.2013 11:12:13");
	
	cv.TimeAlways();

	DUMP(cv.Format(Date(2013, 1, 2)));
	ASSERT(cv.Format(Date(2013, 1, 2)) == "02.01.2013 00:00:00");

	DUMP(cv.Format(Time(2013, 1, 2, 11, 12, 13)));
	ASSERT(cv.Format(Time(2013, 1, 2, 11, 12, 13)) == "02.01.2013 11:12:13");
	
	DUMP(cv.Scan("15.6.2010"));
	ASSERT(cv.Scan("15.6.2010") == Time(2010, 6, 15, 0, 0, 0));

	cv.DayEnd();

	DUMP(cv.Scan("15.6.2010"));
	ASSERT(cv.Scan("15.6.2010") == Time(2010, 6, 15, 23, 59, 59));
	
	ConvertDouble d;

	DUMP(d.Scan("12-12"));
	ASSERT(IsError(d.Scan("12-12")));

	Test(d, "12-12", ErrorValue());
	Test(d, "12+12", ErrorValue());
	Test(d, "12.-12", ErrorValue());
	Test(d, "12.+12", ErrorValue());
	Test(d, "a", ErrorValue());
	Test(d, " ", ErrorValue());

	Test(d, "123 ", 123);

	Test(d, "0", 0);
	Test(d, "123", 123);
	Test(d, "+123", 123);
	Test(d, "-123", -123);
	Test(d, "1.1", 1.1);
	Test(d, "+1.1", 1.1);
	Test(d, "-1.1", -1.1);
	Test(d, "1.1e10", 1.1e10);
	Test(d, "+1.1e10", 1.1e10);
	Test(d, "-1.1e10", -1.1e10);
	Test(d, "1.1e+10", 1.1e10);
	Test(d, "+1.1e+10", 1.1e10);
	Test(d, "-1.1e+10", -1.1e10);
	Test(d, "1.1e-10", 1.1e-10);
	Test(d, "+1.1e-10", 1.1e-10);
	Test(d, "-1.1e-10", -1.1e-10);
	Test(d, "1.1E10", 1.1E10);
	Test(d, "+1.1E10", 1.1E10);
	Test(d, "-1.1E10", -1.1E10);
	Test(d, "1.1E+10", 1.1E10);
	Test(d, "+1.1E+10", 1.1E10);
	Test(d, "-1.1E+10", -1.1E10);
	Test(d, "1.1E-10", 1.1E-10);
	Test(d, "+1.1E-10", 1.1E-10);
	Test(d, "-1.1E-10", -1.1E-10);
	
	ConvertInt x;
	Test(x, "12", 12);
	Test(x, "-12", -12);
	Test(x, "12-12", ErrorValue());
	Test(x, "12+12", ErrorValue());
	Test(x, "12.-12", ErrorValue());
	Test(x, "12.+12", ErrorValue());
	Test(x, "a", ErrorValue());
	Test(x, " ", ErrorValue());

	ConvertInt64 xx;
	Test(xx, "12", 12);
	Test(xx, "-12", -12);
	Test(xx, "12-12", ErrorValue());
	Test(xx, "12+12", ErrorValue());
	Test(xx, "12.-12", ErrorValue());
	Test(xx, "12.+12", ErrorValue());
	Test(xx, "a", ErrorValue());
	Test(xx, " ", ErrorValue());
	
	ASSERT(StrDbl("123xxx") == 123);
	ASSERT(StrInt("123xxx") == 123);
	
	ConvertDouble dd;
	dd.Pattern("%2.!n EUR");
	Test(dd, "123 EUR", 123);
	
	LOG("=========== OK");
}
