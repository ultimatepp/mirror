template <typename CHAR>
force_inline
void SkipSpaces__(const CHAR *&s)
{
	while((BYTE)*s <= ' ') s++;
}

template <typename CHAR>
force_inline
int ScanSgn__(const CHAR *&s)
{
	int sign = 1;
	if(*s == '+')
		s++;
	if(*s == '-') {
		sign = -1;
		s++;
	}
	SkipSpaces__(s);
	return sign;
}

template <typename CHAR, typename BYTE, typename UINT, int base>
force_inline
const CHAR *ScanUint0(UINT& result, const CHAR *s, bool& overflow)
{
	auto Get = [&] (int i) -> int {
		int c = (BYTE)s[i];
		if(base > 10) {
			if(c <= '9')
				return c - '0';
			if(c <= 'Z')
				return c - 'A' + 10;
			return c - 'a' + 10;
		}
		return c - '0';
	};
	int c1 = Get(0);
	if(c1 < 0 || c1 >= base)
		return NULL;
	UINT n = 0;
	const int base2 = base * base;
	const int base3 = base * base2;
	const int base4 = base * base3;
	for(;;) {
		UINT n0 = n;
		auto CheckOverflow = [&] { if(n0 > n) overflow = true; };
		if(c1 < 0 || c1 >= base) {
			result = n;
			return s;
		}
		int c2 = Get(1);
		if(c2 < 0 || c2 >= base) {
			result = base * n + c1;
			CheckOverflow();
			return s + 1;
		}
		int c3 = Get(2);
		if(c3 < 0 || c3 >= base) {
			result = base2 * n + base * c1 + c2;
			CheckOverflow();
			return s + 2;
		}
		int c4 = Get(3);
		if(c4 < 0 || c4 >= base) {
			result = base3 * n + base2 * c1 + base * c2 + c3;
			CheckOverflow();
			return s + 3;
		}
		n = base4 * n + base3 * c1 + base2 * c2 + base * c3 + c4;
		CheckOverflow();
		s += 4;
		c1 = Get(0);
	}
}

template <typename CHAR, typename BYTE, typename UINT, int base>
force_inline
const CHAR *ScanUint(UINT& result, const CHAR *s, bool& overflow)
{
	SkipSpaces__(s);
	return ScanUint0<CHAR, BYTE, UINT, base>(result, s, overflow);
}

template <typename CHAR, typename BYTE, typename UINT, int base>
force_inline
const CHAR *ScanUint(UINT& result, const CHAR *s)
{
	bool overflow = false;
	s = ScanUint<CHAR, BYTE, UINT, base>(result, s, overflow);
	return overflow ? NULL : s;
}

template <typename CHAR, typename BYTE, typename UINT, typename INT, int base>
force_inline
const CHAR *ScanInt(INT& result, const CHAR *s, bool& overflow)
{
	SkipSpaces__(s);
	int sgn = ScanSgn__(s);
	UINT uresult;
	s = ScanUint0<CHAR, BYTE, UINT, base>(uresult, s, overflow);
	if(sgn < 0) {
		result = INT(sgn * uresult);
		if(result > 0) overflow = true;
	}
	else {
		result = INT(uresult);
		if(result < 0) overflow = true;
	}
	return s;
}

template <typename CHAR, typename BYTE, typename UINT, typename INT, int base>
force_inline
const CHAR *ScanInt(INT& result, const CHAR *s)
{
	bool overflow = false;
	s = ScanInt<CHAR, BYTE, UINT, INT, base>(result, s, overflow);
	return overflow ? NULL : s;
}

template <typename CHAR, typename BYTE>
force_inline
const CHAR *ScanDbl(double& result, const CHAR *s, int alt_dp = '.')
{
	SkipSpaces__(s);
	double sign = ScanSgn__(s);
	if(!IsDigit(*s) && *s != '.' && *s != alt_dp)
		return NULL;
#ifdef CPU_64
	auto ReadNumber = [&]() -> double {
		uint64 n = 0;
		for(;;) {
			int c1 = (BYTE)s[0] - '0';
			if(c1 < 0 || c1 > 9)
				return n;
			int c2 = (BYTE)s[1] - '0';
			if(c2 < 0 || c2 > 9) {
				s++;
				return 10 * n + c1;
			}
			int c3 = (BYTE)s[2] - '0';
			if(c3 < 0 || c3 > 9) {
				s += 2;
				return 100 * n + 10 * c1 + c2;
			}
			int c4 = (BYTE)s[3] - '0';
			if(c4 < 0 || c4 > 9) {
				s += 3;
				return 1000 * n + 100 * c1 + 10 * c2 + c3;
			}
			n = 10000 * n + 1000 * c1 + 100 * c2 + 10 * c3 + c4;
			s += 4;
			if(n >= (I64(2) << 50)) {
				double f = n;
				for(;;) {
					int c = *s - '0';
					if(c < 0 || c > 9)
						break;
					f = 10 * f + c;
					s++;
				}
				return f;
			}
		}
	};
#else
	auto ReadNumber = [&]() -> double {
		double n = 0;
		for(;;) {
			int c1 = (BYTE)s[0] - '0';
			if(c1 < 0 || c1 > 9)
				return n;
			n = 10 * n + c1;
			s++;
		}
	};
#endif
	double number = (double)ReadNumber();
	extern double ipow10_table[601];
	auto pow10i = [](double i) {
		if(i >= -300 && i <= 300)
			return ipow10_table[(int)i + 300];
		return pow(10.0, i);
	};
	ONCELOCK {
		for(int i = -300; i <= 300; i++)
			ipow10_table[i + 300] = pow(10.0, i);
	}
	if(*s == '.' || *s == alt_dp) {
		s++;
		int dp = 0;
		const CHAR *s0 = s;
		double decimals = (double)ReadNumber();
		number += decimals * pow10i(-(s - s0));
	}
	number *= sign;
	if(*s == 'e' || *s == 'E') {
		s++;
		double sign = ScanSgn__(s);
		double exp = sign * ReadNumber();
		number *= pow10i((int)exp);
	}
	result = number;
	return s;
}
