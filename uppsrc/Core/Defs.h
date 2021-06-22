// #define MEMORY_SHRINK

#if !defined(flagUSEMALLOC) && !defined(flagSO) && !defined(flagHEAPOVERRIDE)
#define UPP_HEAP
#endif

#define __BREAK__        (*(volatile int *)0 = 0) // kill(getpid(), SIGTRAP)

#ifdef COMPILER_MSC
	#pragma warning(disable : 4800)
	#pragma warning(disable : 4129)
	#pragma warning(disable : 4290)
	#pragma warning(disable : 4068)
	#pragma warning(disable : 4005)
	#pragma warning(disable : 4675)
	#pragma warning(disable : 4996)
	#pragma warning(disable : 4180)

	#pragma setlocale("C")
#endif

bool    IsPanicMode();

void    Panic(const char *msg);

void    AssertFailed(const char *file, int line, const char *cond);

void    InstallPanicMessageBox(void (*mb)(const char *title, const char *text));
void    PanicMessageBox(const char *title, const char *text);

#define VERIFY(x)        ((x) ? (void)0 : ::Upp::AssertFailed(__FILE__, __LINE__, #x))

#ifdef _DEBUG

#define ASSERT_(x, msg)  ((x) ? (void)0 : ::Upp	::AssertFailed(__FILE__, __LINE__, msg))
#define ASSERT(x)        ASSERT_(x, #x)

#else

inline  void LOGNOP__() {}

#define LOG_NOP          Upp::LOGNOP__()

#define ASSERT_(x, msg)  LOG_NOP
#define ASSERT(x)        LOG_NOP

#endif

#define _cm_  ,

#define __countof(a)          int(sizeof(a) / sizeof(a[0]) )

#define ASSTRING_(x)  #x
#define ASSTRING(x)   ASSTRING_(x)

#define COMBINE__(a, b)            a##b
#define COMBINE(a, b)              COMBINE__(a, b)

#define COMBINE3__(a, b, c)        a##b##c
#define COMBINE3(a, b, c)          COMBINE3__(a, b, c)

#define COMBINE4__(a, b, c, d)     a##b##c##d
#define COMBINE4(a, b, c, d)       COMBINE4__(a, b, c, d)

#define COMBINE5__(a, b, c, d, e)  a##b##c##d##e
#define COMBINE5(a, b, c, d, e)    COMBINE5__(a, b, c, d, e)

#define MK__s__(x)       s__s##x
#define MK__s_(x)        MK__s__(x)

#ifdef  BLITZ_INDEX__
#define MK__s            MK__s_(COMBINE3(BLITZ_INDEX__, _, __LINE__))
#else
#define MK__s            MK__s_(__LINE__)
#endif

#ifdef flagCHECKINIT
void InitBlockBegin__(const char *fn, int line);
void InitBlockEnd__(const char *fn, int line);
#else
inline void InitBlockBegin__(const char *, int) {}
inline void InitBlockEnd__(const char *, int) {}
#endif

struct Callinit {
	Callinit(void (*fn)(), const char *cpp, int line) { InitBlockBegin__(cpp, line); fn(); InitBlockEnd__(cpp, line); }
	Callinit(void (*fn)())                            { fn(); }
};

struct Callexit {
	Callexit(void (*fn)()) { atexit(fn); }
};


#define INITBLOCK \
static void COMBINE(MK__s, _fn)(); static UPP::Callinit MK__s(COMBINE(MK__s, _fn), __FILE__, __LINE__); \
static void COMBINE(MK__s, _fn)()

#define INITBLOCK_(x) \
static void COMBINE(x, _fn)(); static UPP::Callinit x(COMBINE(x, _fn), __FILE__, __LINE__); \
static void COMBINE(x, _fn)()


#define EXITBLOCK \
static void COMBINE(MK__s, _fn)(); static UPP::Callexit MK__s(COMBINE(MK__s, _fn)); \
static void COMBINE(MK__s, _fn)()

#define EXITBLOCK_(x) \
static void COMBINE(x, _fn)(); static UPP::Callexit x(COMBINE(x, _fn)); \
static void COMBINE(x, _fn)()

#define INITIALIZE(x) \
void x##__initializer(); \
static struct x##__initialize_struct { x##__initialize_struct() { x##__initializer(); } } x##__initialize_dummy;

#define INITIALIZER(x) \
void x##__initializer_fn(); \
void x##__initializer() \
{ \
	ONCELOCK { \
		x##__initializer_fn(); \
	} \
} \
void x##__initializer_fn()



#ifdef DEPRECATED
#define INITCODE(x) \
static void COMBINE(MK__s, _fn)() { x } static UPP::Callinit MK__s(COMBINE(MK__s, _fn), __FILE__, __LINE__);

#define EXITCODE(x) \
static void COMBINE(MK__s, _fn)() { x } static UPP::Callexit MK__s(COMBINE(MK__s, _fn));
#endif

typedef unsigned char      byte;
typedef signed char        int8;
typedef unsigned char      uint8;

typedef short unsigned     word;
typedef short int          int16;
typedef short unsigned     uint16;

#ifdef PLATFORM_WIN32
typedef unsigned long      dword;
typedef long               int32;
typedef unsigned long      uint32;
typedef WCHAR              wchar;
#else
typedef unsigned int       dword;
typedef int                int32;
typedef unsigned int       uint32;
typedef word               wchar;
#endif


#ifdef COMPILER_MSC
typedef __int64            int64;
typedef unsigned __int64   uint64;
#else
typedef long long int      int64;
typedef long long unsigned uint64;
#endif

typedef uint64             qword;

struct m128 {
	int64 i64[2];
	
	static m128 Zero()             { m128 a; a.i64[0] = a.i64[1] = 0; return a; }
};

inline bool IsNaN(double d)        { return std::isnan(d); }
inline bool IsInf(double d)        { return std::isinf(d); }
inline bool IsFin(double d)        { return !IsNaN(d) && !IsInf(d); }

#ifdef COMPILER_MSC
	#define I64(c) ((int64)COMBINE(c, i64))
#else
	#define I64(c) ((int64)COMBINE(c, LL))
#endif

#ifndef INT64_MIN
#define INT64_MIN          I64(-0x8000000000000000)
#endif
#ifndef INT64_MAX
#define INT64_MAX          I64(+0x7FFFFFFFFFFFFFFF)
                                0x9fb6f9ef3be2ec93
#endif

#if !defined(PLATFORM_WIN32)

#define HIBYTE(a)        (byte)((a) >> 8)
#define LOBYTE(a)        byte(a)
#define HIWORD(a)        (word)((a) >> 16)
#define LOWORD(a)        word(a)

#define MAKEWORD(l, h)   ((word)  (((byte) (l)) | ((word) ((byte) (h))) << 8))
#define MAKELONG(l, h)   ((dword) (((word) (l)) | ((dword) ((word) (h))) << 16))

#endif

#define MAKEQWORD(l, h)  ((qword) (((dword) (l)) | ((qword) ((dword) (h))) << 32))
#define HIDWORD(a)       (dword)(((uint64)a) >> 32)
#define LODWORD(a)       dword(a)

#define OFFSETOF(clss, mbr) ((int)(uintptr_t)&(((clss *)1)->mbr) - 1)

template <typename T> T clone(const T& x) { return x; }

#define pick_
#define rval_default(T) T(T&&) = default; T& operator=(T&&) = default;

template <typename T>
auto pick(T&& x) noexcept -> decltype(std::move(x)) { return std::move(x); }

template<class T> class Function;

#ifdef DEPRECATED
#define rval_ &&
#define init_
#endif

#ifdef COMPILER_MSC
#define force_inline __forceinline
#define never_inline __declspec(noinline)
#elif defined(COMPILER_GCC)
#define force_inline __attribute__((always_inline)) inline
#define never_inline __attribute__((noinline))
#else
#define force_inline inline
#define never_inline
#endif

#define BINARY(i, f) \
extern "C" byte *i; \
extern "C" int COMBINE(i, _length);

#define BINARY_ARRAY(i, x, f) \
extern "C" byte *i[]; \
extern "C" int COMBINE(i, _length)[]; \
extern "C" int COMBINE(i, _count);

#define BINARY_MASK(i, m) \
extern "C" byte *i[]; \
extern "C" int COMBINE(i, _length)[]; \
extern "C" int COMBINE(i, _count); \
extern "C" char *COMBINE(i, _files)[];

class NoCopy {
private:
	NoCopy(const NoCopy&);
	void operator=(const NoCopy&);
public:
	NoCopy() {}
};

const int    INT_NULL           =    INT_MIN;
const int64  INT64_NULL         =    INT64_MIN;

const double DOUBLE_NULL        =    -1.0E+308;
const double DOUBLE_NULL_LIM    =    -1.0E+307;

class Nuller {
public:
	operator int() const                { return INT_NULL; }
	operator int64() const              { return INT64_NULL; }
	operator double() const             { return DOUBLE_NULL; }
	operator bool() const               { return false; }

	Nuller() {}
};

extern const Nuller Null;

template <class T> void SetNull(T& x) { x = Null; }

template <class T> bool IsNull(const T& x)       { return x.IsNullInstance(); }

template<> inline bool  IsNull(const int& i)     { return i == INT_NULL; }
template<> inline bool  IsNull(const int64& i)   { return i == INT64_NULL; }
template<> inline bool  IsNull(const double& r)  { return !(std::abs(r) < -DOUBLE_NULL); }
template<> inline bool  IsNull(const bool& r  )  { return false; }

#include "Heap.h"
#ifdef CPU_X86
bool CpuMMX();
bool CpuSSE();
bool CpuSSE2();
bool CpuSSE3();
bool CpuHypervisor();
bool CpuAVX();
#endif

int  CPU_Cores();

void GetSystemMemoryStatus(uint64& total, uint64& available);

template <class T>
inline void Swap(T& a, T& b) { T tmp = pick(a); a = pick(b); b = pick(tmp); }

//Quick fix....
#ifdef PLATFORM_WINCE
const char *FromSysChrSet(const wchar *s);
const wchar *ToSysChrSet(const char *s);
#else
inline const char *FromSysChrSet(const char *s) { return s; }
inline const char *ToSysChrSet(const char *s) { return s; }
#endif

#ifdef _DEBUG
void __LOGF__(const char *format, ...);
#define LOGF             UPP::__LOGF__
#else
inline void __LOGF__(const char *format, ...);
#endif

template <class T>
void IGNORE_RESULT(const T&) {}

// Backward compatibility

#define GLOBAL_VP(type, name, param) \
name() \
{ \
	static type x param; \
	return x; \
}

#define GLOBAL_VARP(type, name, param) \
type& GLOBAL_VP(type, name, param)

#define GLOBAL_V(type, name)   GLOBAL_VP(type, name, init_)
#define GLOBAL_VAR(type, name) type& GLOBAL_V(type, name)

#define GLOBAL_VP_INIT(type, name, param) \
name() \
{ \
	static type x param; \
	return x; \
} \
INITBLOCK { name(); }

#define GLOBAL_VARP_INIT(type, name, param) \
type& GLOBAL_VP_INIT(type, name, param)

#define GLOBAL_V_INIT(type, name)   GLOBAL_VP_INIT(type, name, init_)
#define GLOBAL_VAR_INIT(type, name) type& GLOBAL_V_INIT(type, name)

#if __GNUC__ > 6
	#define NOUBSAN __attribute__((no_sanitize_undefined))
#elif __clang_major__ > 6
	#define NOUBSAN __attribute__((no_sanitize("undefined")))
#else
	#define NOUBSAN
#endif

// DEPRECATED:

// these are pre-c++ 11 tools to achieve something like variadic templates
#define __Expand1(x) x(1)
#define __Expand2(x)  __Expand1(x) x(2)
#define __Expand3(x)  __Expand2(x) x(3)
#define __Expand4(x)  __Expand3(x) x(4)
#define __Expand5(x)  __Expand4(x) x(5)
#define __Expand6(x)  __Expand5(x) x(6)
#define __Expand7(x)  __Expand6(x) x(7)
#define __Expand8(x)  __Expand7(x) x(8)
#define __Expand9(x)  __Expand8(x) x(9)
#define __Expand10(x) __Expand9(x) x(10)
#define __Expand11(x) __Expand10(x) x(11)
#define __Expand12(x) __Expand11(x) x(12)
#define __Expand13(x) __Expand12(x) x(13)
#define __Expand14(x) __Expand13(x) x(14)
#define __Expand15(x) __Expand14(x) x(15)
#define __Expand16(x) __Expand15(x) x(16)
#define __Expand17(x) __Expand16(x) x(17)
#define __Expand18(x) __Expand17(x) x(18)
#define __Expand19(x) __Expand18(x) x(19)
#define __Expand20(x) __Expand19(x) x(20)
#define __Expand21(x) __Expand20(x) x(21)
#define __Expand22(x) __Expand21(x) x(22)
#define __Expand23(x) __Expand22(x) x(23)
#define __Expand24(x) __Expand23(x) x(24)
#define __Expand25(x) __Expand24(x) x(25)
#define __Expand26(x) __Expand25(x) x(26)
#define __Expand27(x) __Expand26(x) x(27)
#define __Expand28(x) __Expand27(x) x(28)
#define __Expand29(x) __Expand28(x) x(29)
#define __Expand30(x) __Expand29(x) x(30)
#define __Expand31(x) __Expand30(x) x(31)
#define __Expand32(x) __Expand31(x) x(32)
#define __Expand33(x) __Expand32(x) x(33)
#define __Expand34(x) __Expand33(x) x(34)
#define __Expand35(x) __Expand34(x) x(35)
#define __Expand36(x) __Expand35(x) x(36)
#define __Expand37(x) __Expand36(x) x(37)
#define __Expand38(x) __Expand37(x) x(38)
#define __Expand39(x) __Expand38(x) x(39)
#define __Expand40(x) __Expand39(x) x(40)

#define __Expand(x)   __Expand40(x)

#define __List1(x) x(1)
#define __List2(x)  __List1(x), x(2)
#define __List3(x)  __List2(x), x(3)
#define __List4(x)  __List3(x), x(4)
#define __List5(x)  __List4(x), x(5)
#define __List6(x)  __List5(x), x(6)
#define __List7(x)  __List6(x), x(7)
#define __List8(x)  __List7(x), x(8)
#define __List9(x)  __List8(x), x(9)
#define __List10(x) __List9(x), x(10)
#define __List11(x) __List10(x), x(11)
#define __List12(x) __List11(x), x(12)
#define __List13(x) __List12(x), x(13)
#define __List14(x) __List13(x), x(14)
#define __List15(x) __List14(x), x(15)
#define __List16(x) __List15(x), x(16)
#define __List17(x) __List16(x), x(17)
#define __List18(x) __List17(x), x(18)
#define __List19(x) __List18(x), x(19)
#define __List20(x) __List19(x), x(20)
#define __List21(x) __List20(x), x(21)
#define __List22(x) __List21(x), x(22)
#define __List23(x) __List22(x), x(23)
#define __List24(x) __List23(x), x(24)
#define __List25(x) __List24(x), x(25)
#define __List26(x) __List25(x), x(26)
#define __List27(x) __List26(x), x(27)
#define __List28(x) __List27(x), x(28)
#define __List29(x) __List28(x), x(29)
#define __List30(x) __List29(x), x(30)
#define __List31(x) __List30(x), x(31)
#define __List32(x) __List31(x), x(32)
#define __List33(x) __List32(x), x(33)
#define __List34(x) __List33(x), x(34)
#define __List35(x) __List34(x), x(35)
#define __List36(x) __List35(x), x(36)
#define __List37(x) __List36(x), x(37)
#define __List38(x) __List37(x), x(38)
#define __List39(x) __List38(x), x(39)
#define __List40(x) __List39(x), x(40)

#define E__p(I)       p##I
