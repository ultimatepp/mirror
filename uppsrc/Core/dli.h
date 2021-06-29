//#BLITZ_APPROVE

#define DLISTR__(s) #s
#define DLISTR_(s)  DLISTR__(s)
#define DLISTR(s)   DLISTR_(s)

#define FN(retval, fn, args)           FN_CN(retval, DLLCALL, fn, args, DLISTR(fn))
#define FN_C(retval, call, fn, args)   FN_CN(retval, call, fn, args, DLISTR(fn))
#define FN_N(retval, fn, args, name)   FN_CN(retval, DLLCALL, fn, args, name)

#define FN0(retval, fn, args)          FN0_CN(retval, DLLCALL, fn, args, DLISTR(fn))
#define FN0_C(retval, call, fn, args)  FN0_CN(retval, call, fn, args, DLISTR(fn))
#define FN0_N(retval, fn, args, name)  FN0_CN(retval, DLLCALL, fn, args, name)

#ifndef DLLTYPE
#define DLLTYPE   COMBINE(T_, DLIMODULE)
#endif

#ifndef DLLCALL
#ifdef PLATFORM_WIN32
#define DLLCALL //__cdecl
#else
#define DLLCALL
#endif
#endif

#ifndef DLLSTRUCT
#define DLLSTRUCT DLIMODULE
#endif

// ---------

#if !defined(DLI_HEADER) && !defined(DLI_SOURCE)
#define DLI_HEADER
#define DLI_SOURCE
#endif

#ifdef DLI_HEADER
#undef DLI_HEADER

#define FN_CN(retval, call, fn, args, name)\
	typedef retval (call fn##_type) args; fn##_type *fn;
#define FN0_CN FN_CN

#define DATA(type, name) type *name;

struct DLLTYPE{
	DLLTYPE();

	UPP::String       GetLibName() const        { return libname; }
	void              SetLibName(const char *n) { if(handle) Free(); libname = n; checked = false; }

	bool              Load();
	bool              Load(const char *n)       { SetLibName(n); return Load(); }
	void              Force();
	void              Free();

	operator          bool ()      { return handle; }

#include DLIHEADER
private:
	UPP::String       libname;
	DLLHANDLE         handle;
	bool              checked;
};

#undef FN_CN
#undef FN0_CN
#undef DATA

// ---------

DLLTYPE& DLIMODULE();
DLLTYPE& COMBINE(DLIMODULE, _)();

#endif

#ifdef DLI_SOURCE
#undef DLI_SOURCE

DLLTYPE& COMBINE(DLIMODULE, _)()
{
	static DLLTYPE out;
	return out;
}

DLLTYPE& DLIMODULE()
{
	DLLTYPE& out = COMBINE(DLIMODULE, _)();
	out.Load();
	return out;
}

DLLTYPE::DLLTYPE()
{
	checked = false;
	handle = 0;
	libname =
#ifdef DLLFILENAME
		DLLFILENAME;
#else
		DLISTR(DLLSTRUCT);
#endif
}

bool DLLTYPE::Load()
{
	if(!checked)
	{
		checked = true;

#define FN_CN(retval, call, fn, args, name)  name,
#define FN0_CN(retval, call, fn, args, name) "?" name,
#define DATA(type, name) #name,

		const char *name[] =
		{
		#include DLIHEADER
			0
		};

#undef FN_CN
#undef FN0_CN
#undef DATA

// --- FnPtr map


#define FN_CN(retval, call, fn, args, name)  &fn,
#define FN0_CN FN_CN
#define DATA(type, name) &name,

		void *proc[] =
		{
		#include DLIHEADER
			0
		};

#undef FN_CN
#undef FN0_CN
#undef DATA

		// --- Proc pointers


//		if(handle = LoadDll(file, name, proc))
//			atexit(&Free);
		handle = LoadDll__(libname, name, proc);
	}
	return handle;
}

void DLLTYPE::Force()
{
	if(!DLLTYPE::Load()) {
//		RLOG("Failed to load required dynamic library '" << LibName() << "'");
		exit(1);
	}
}

void DLLTYPE::Free() {
	if(handle) {
		FreeDll__(handle);
		handle = 0;
	}
	checked = false;
}
#endif

#undef FN
#undef FN_C
#undef FN_N

#undef FN0
#undef FN0_C
#undef FN0_N

#undef DLISTR__
#undef DLISTR_
#undef DLISTR

#ifdef DLLFILENAME
#undef DLLFILENAME
#endif

#ifdef DLLAUTOLOAD
#undef DLLAUTOLOAD
#endif

#undef DLLTYPE
#undef DLIMODULE
#undef DLIHEADER
#undef DLLCALL

#ifdef DLLFILENAME
#undef DLLFILENAME
#endif

#ifdef DLLAUTOLOAD
#undef DLLAUTOLOAD
#endif
