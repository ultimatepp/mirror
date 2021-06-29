#include "Core.h"

#ifdef PLATFORM_WIN32
typedef HMODULE DLLHANDLE;
#else
typedef void   *DLLHANDLE;
#endif

#define LLOG(x) // RLOG(x)

#ifdef PLATFORM_WIN32

#include <winnt.h>

namespace Upp {

class PeFile
{
public:
	PeFile(const char *data = 0) { Open(data); }

	bool                        Open(const char *data);

	int                         GetExportCount() const;
	const char                 *GetExport(int index) const;
	const char                 *FindExportRaw(const char *name, bool case_sensitive = true) const;

	int                         GetSectionIndex(const char *name) const;

private:
	const char                 *data;
	const IMAGE_NT_HEADERS     *headers;
	const IMAGE_SECTION_HEADER *sections;
	const IMAGE_EXPORT_DIRECTORY *exports;
};

bool PeFile::Open(const char *_data)
{
	data = _data;
	if(!data || data[0] != 'M' || data[1] != 'Z')
		return false;
	int pe = Peek32le(data + 0x3C);
	if(IsBadReadPtr(data + pe, sizeof(IMAGE_NT_HEADERS)))
		return false;
	if(memcmp(data + pe, "PE\0\0", 4))
		return false;
	headers = (const IMAGE_NT_HEADERS *)(data + pe);
	sections = (const IMAGE_SECTION_HEADER *)(headers + 1);
	exports = (const IMAGE_EXPORT_DIRECTORY *)(data
		+ headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	return true;
}

int PeFile::GetSectionIndex(const char *name) const
{
	for(int i = 0, n = headers->FileHeader.NumberOfSections; i < n; i++)
		if(!strcmp((const char *)sections[i].Name, name))
			return i;
	return -1;
}

int PeFile::GetExportCount() const
{
	return exports ? exports->NumberOfNames : 0;
}

const char *PeFile::GetExport(int index) const
{
	if(!exports || index < 0 || index >= (int) exports->NumberOfNames)
		return 0;
	return data + ((const dword *)(data + (dword)exports->AddressOfNames))[index];
}

static bool EqualMem(const char *a, const char *b, int len, bool case_sensitive)
{
	if(case_sensitive)
		return !memcmp(a, b, len);
	else
		return !MemICmp(a, b, len);
}

const char *PeFile::FindExportRaw(const char *name, bool case_sensitive) const
{
	if(!exports || !name || !*name)
		return 0;
	int len = (int)strlen(name);
	const dword *pnames = (const dword *)(data + (dword)exports->AddressOfNames);
	for(int i = 0; i < (int) exports->NumberOfNames; i++) {
		const char *exp = data + pnames[i];
		int elen = (int)strlen(exp);
		if(elen < len)
			continue;
		if(elen == len && EqualMem(exp, name, len, case_sensitive))
			return exp;
		if(EqualMem(exp, name, len, case_sensitive) && exp[len] == '@')
			return exp;
		if(exp[0] == '_' && EqualMem(exp + 1, name, len, case_sensitive)
			&& (exp[len + 1] == '@' || exp[len + 1] == 0))
			return exp;
	}
	return 0;
}

}

HMODULE CheckDll__(const char *fn, const char *const *names, UPP::Vector<void *>& plist)
{
	HMODULE hmod = LoadLibrary(fn);

	if(!hmod)
		return 0;

	UPP::PeFile pe;
	if(!pe.Open((const char *)hmod)) {
		FreeLibrary(hmod);
		return 0;
	}

	int missing = 0;
	for(const char *const *p = names; *p; p++) {
		const char *exp = *p;
		bool optional = (*exp == '?');
		if(optional) exp++;
		const char *name = pe.FindExportRaw(exp);
		void *proc = 0;
		if(!name || !(proc = (void *)GetProcAddress(hmod, name)))
			if(!optional) {
				if(!missing) {
					LLOG(fn << " missing exports:");
				}
				LLOG(exp);
				missing++;
			}
		plist.Add(proc);
	}
	if(missing) {
		LLOG(missing << " total");
		FreeLibrary(hmod);
		return 0;
	}
	return hmod;
}

void FreeDll__(HMODULE hmod)
{
	FreeLibrary(hmod);
}

#endif

#ifdef PLATFORM_POSIX

#include <dlfcn.h>

void *CheckDll__(const char *fn, const char *const *names, UPP::Vector<void *>& plist)
{
	void *hmod = dlopen(fn, RTLD_LAZY | RTLD_GLOBAL);
	if(!hmod) {
		RLOG("Error loading library " << fn << ": " << dlerror());
/*
		for(int i = 0; i < 100; i++) {
			hmod = dlopen(fn + ("." + UPP::AsString(i)), RTLD_LAZY | RTLD_GLOBAL);
			if(hmod)
				break;
		}
		if(!hmod)
*/			return 0;
	}

	int missing = 0;
	for(const char *const *p = names; *p; p++) {
		const char *exp = *p;
		bool optional = (*exp == '?');
		if(optional) exp++;
		void *proc = dlsym(hmod, exp);
		if(!proc && !optional) {
			if(!missing) {
				RLOG(fn << " missing exports:");
			}
			RLOG(exp);
		}
		plist.Add(proc);
	}

	if(missing) {
		RLOG(missing << " missing symbols total");
		dlclose(hmod);
		return 0;
	}

	return hmod;
}

void FreeDll__(void *hmod)
{
	if(hmod)
		dlclose(hmod);
}

#endif//PLATFORM_POSIX

DLLHANDLE LoadDll__(UPP::String& inoutfn, const char *const *names, void *const *procs)
{
	const char *fn = inoutfn;
	while(*fn) {
		const char *b = fn;
		while(*fn && *fn != ';'
#ifndef PLATFORM_WIN32
			&& *fn != ':'
#endif
		)
			fn++;
		UPP::String libname(b, fn);
		if(*fn)
			fn++;
		UPP::Vector<void *> plist;
		if(DLLHANDLE hmod = CheckDll__(libname, names, plist)) {
			for(int i = 0; i < plist.GetCount(); i++)
				*(void **)*procs++ = plist[i];
			inoutfn = libname;
			return hmod;
		}
	}
	return 0;
}
