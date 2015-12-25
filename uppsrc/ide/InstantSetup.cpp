#include "ide.h"

#ifdef PLATFORM_WIN32

class DirFinder {
	Vector<String> dirs;
	VectorMap<String, bool> entry;
	
	Progress pi;
	
	static String Normz(const String& p);
	
	bool Has(const String& d, const Vector<String>& m, bool isfile);
	bool Contains(const String& d, const Vector<String>& m);
	void ScanDirs(const char *dir);

public:
	void Dir(const String& d) { dirs.Add(d); }

	String ScanForDir(const String& dir, const char *ccontains, const char *cfiles, const char *csubdirs);
};

String DirFinder::Normz(const String& p)
{
	String h = ToLower(p);
	h.Replace("\\", "/");
	return h;
}

void DirFinder::ScanDirs(const char *dir)
{
	String h = Normz(dir);
	if(pi.Canceled() || entry.Find(h) >= 0)
		return;
	entry.GetAdd(Normz(dir)) = false;
	pi.SetText("Scanning " + GetFileName(dir));
	for(FindFile ff(AppendFileName(dir, "*.*")); ff; ff.Next()) {
		String p = ff.GetPath();
		if(ff.IsFolder())
			ScanDirs(p);
		else
		if(ff.IsFile())
			entry.GetAdd(Normz(p)) = true;
	}
}

bool DirFinder::Has(const String& d, const Vector<String>& m, bool isfile)
{
	for(int i = 0; i < m.GetCount(); i++) {
		String dd = d + '/' + m[i];
		int q = entry.Find(dd);
		if(q < 0 || entry[q] != isfile)
			return false;
	}
	return true;
}

int CharFilterNotDigit(int c)
{
	return IsDigit(c) ? 0 : c;
}

bool DirFinder::Contains(const String& d, const Vector<String>& m)
{
	for(int i = 0; i < m.GetCount(); i++)
		if(d.Find(m[i]) < 0)
			return false;
	return true;
}

String DirFinder::ScanForDir(const String& dir, const char *ccontains, const char *cfiles, const char *csubdirs)
{
	for(;;) {
		Vector<String> files = Split(cfiles, ';');
		Vector<String> subdirs = Split(csubdirs, ';');
		Vector<String> contains = Split(ccontains, ';');
		Vector<int> max;
		String r;
		for(int i = 0; i < entry.GetCount(); i++) {
			String d = entry.GetKey(i);
			if(entry.GetKey(i).EndsWith(dir) && Has(d, files, true) && Has(d, subdirs, false) && Contains(d, contains)) {
				Vector<String> m0 = Split(d, CharFilterNotDigit);
				Vector<int> m;
				for(int i = 0; i < m0.GetCount(); i++)
					m.Add(atoi(m0[i]));
				if(m > max) {
					max = pick(m);
					r = d;
				}
			}
		}
		if(r.GetCount() || dirs.GetCount() == 0)
			return r;
		pi.Title("Searching for compilers");
		ScanDirs(dirs[0]);
		dirs.Remove(0);
	}
}

bool CheckDirs(const Vector<String>& d, int count)
{
	if(d.GetCount() < count)
		return false;
	for(int i = 0; i < count; i++)
		if(!DirectoryExists(d[i]))
			return false;
	return true;
}

void bmSet(VectorMap<String, String>& bm, const char *id, const String& val)
{
	String& t = bm.GetAdd(id);
	t = Nvl(t, val);
}

void InstantSetup()
{
	DirFinder df;
	String pf = GetProgramsFolderX86();
	df.Dir(pf + "/microsoft visual studio 14.0/vc/bin");
	df.Dir(pf + "/windows kits/10");
	df.Dir(pf + "/windows kits");
	df.Dir(pf + "/microsoft visual studio 14.0");
	df.Dir(pf);
	df.Dir(GetProgramsFolder());
	
	String default_method;
	
	bool dirty = false;

	for(int x64 = 0; x64 < 2; x64++) {
		String method = x64 ? "MSC15x64" : "MSC15";
	
	#ifdef _DEBUG
		method << "Test";
	#endif

		String vc, bin, inc, lib;
	
		VectorMap<String, String> bm = GetMethodVars(method);
		Vector<String> bins = Split(bm.Get("PATH", ""), ';');
		Vector<String> incs = Split(bm.Get("INCLUDE", ""), ';');
		Vector<String> libs = Split(bm.Get("LIB", ""), ';');
		if(CheckDirs(bins, 2) && CheckDirs(incs, 4) && CheckDirs(libs, 3)) {
			if(!x64)
				default_method = "MSC15";
		
			continue;
		}
		
		vc = df.ScanForDir("/vc", "", "bin/link.exe;bin/cl.exe;bin/mspdb140.dll", "bin/1033");
		bin = df.ScanForDir(x64 ? "bin/x64" : "bin/x86", "/windows kits/", "makecat.exe;accevent.exe", "");
		inc = df.ScanForDir("", "/windows kits/", "um/adhoc.h", "um;ucrt;shared");
		lib = df.ScanForDir("", "/windows kits/", "um/x86/kernel32.lib", "um;ucrt");
		
		if(vc.GetCount() * bin.GetCount() * inc.GetCount() * lib.GetCount()) {
			bins.At(0) = vc + (x64 ? "/bin/amd64" : "/bin");
			bins.At(1) = bin;
			String& sslbin = bins.At(2);
			if(IsNull(sslbin) || ToLower(sslbin).Find("openssl") >= 0)
				sslbin = GetExeDirFile(x64 ? "bin/OpenSSL-Win/bin" : "bin/OpenSSL-Win/bin32");
			
			incs.At(0) = vc + "/include";
			incs.At(1) = inc + "/um";
			incs.At(2) = inc + "/ucrt";
			incs.At(3) = inc + "/shared";
			String& sslinc = incs.At(4);
			if(IsNull(sslinc) || ToLower(sslinc).Find("openssl") >= 0)
				sslinc = GetExeDirFile("bin/OpenSSL-Win/include");
			
			libs.At(0) = vc + (x64 ? "/lib/amd64" : "/lib");
			libs.At(1) = lib + (x64 ? "/ucrt/x64" : "/ucrt/x86");
			libs.At(2) = lib + (x64 ? "/um/x64" : "/um/x86");
			String& ssllib = libs.At(3);
			if(IsNull(ssllib) || ToLower(ssllib).Find("openssl") >= 0)
				ssllib = GetExeDirFile(x64 ? "bin/OpenSSL-Win/lib/VC" : "bin/OpenSSL-Win/lib32/VC");
		
			bmSet(bm, "BUILDER", x64 ? "MSC15X64" : "MSC15");
			bmSet(bm, "COMPILER", "");
			bmSet(bm, "COMMON_OPTIONS", x64 ? "/bigobj" : "/bigobj /D_ATL_XP_TARGETING");
			bmSet(bm, "COMMON_CPP_OPTIONS", "");
			bmSet(bm, "COMMON_C_OPTIONS", "");
			bmSet(bm, "COMMON_FLAGS", "");
			bmSet(bm, "DEBUG_INFO", "2");
			bmSet(bm, "DEBUG_BLITZ", "1");
			bmSet(bm, "DEBUG_LINKMODE", "0");
			bmSet(bm, "DEBUG_OPTIONS", "-Od");
			bmSet(bm, "DEBUG_FLAGS", "");
			bmSet(bm, "DEBUG_LINK", "/STACK:20000000");
			bmSet(bm, "RELEASE_BLITZ", "0");
			bmSet(bm, "RELEASE_LINKMODE", "0");
			bmSet(bm, "RELEASE_OPTIONS", "-O2");
			bmSet(bm, "RELEASE_SIZE_OPTIONS", "-O1");
			bmSet(bm, "RELEASE_FLAGS", "");
			bmSet(bm, "RELEASE_LINK", "/STACK:20000000");
			bmSet(bm, "DEBUGGER", "msdev");
		
			bm.GetAdd("PATH") = Join(bins, ";");
			bm.GetAdd("INCLUDE") = Join(incs, ";");
			bm.GetAdd("LIB") = Join(libs, ";");
			
			SaveVarFile(ConfigFile(method + ".bm"), bm);
			dirty = true;

			if(!x64)
				default_method = "MSC15";
		}
	}

	String bin = GetExeDirFile("bin");
	if(DirectoryExists(bin + "/TDM64"))
		for(int x64 = 0; x64 < 2; x64++) {
			String method = x64 ? "MINGWx64" : "MINGW";
		#ifdef _DEBUG
			method << "Test";
		#endif
			VectorMap<String, String> bm = GetMethodVars(method);
	
			Vector<String> bins = Split(bm.Get("PATH", ""), ';');
			Vector<String> incs = Split(bm.Get("INCLUDE", ""), ';');
			Vector<String> libs = Split(bm.Get("LIB", ""), ';');
			if(CheckDirs(bins, 3) && CheckDirs(incs, 2) && CheckDirs(libs, 2)) {
				if(!x64)
					default_method = Nvl(default_method, method);
				continue;
			}
	
			bmSet(bm, "BUILDER", "GCC");
			bmSet(bm, "COMPILER", "");
			bmSet(bm, "COMMON_OPTIONS", x64 ? "-msse2" : "-msse2 -m32");
			bmSet(bm, "COMMON_CPP_OPTIONS", "-std=c++11");
			bmSet(bm, "COMMON_C_OPTIONS", "");
			bmSet(bm, "COMMON_LINK", x64 ? "" : "-m32");
			bmSet(bm, "COMMON_FLAGS", "");
			bmSet(bm, "DEBUG_INFO", "2");
			bmSet(bm, "DEBUG_BLITZ", "1");
			bmSet(bm, "DEBUG_LINKMODE", "0");
			bmSet(bm, "DEBUG_OPTIONS", "-O0 -ggdb");
			bmSet(bm, "DEBUG_FLAGS", "");
			bmSet(bm, "DEBUG_LINK", "");
			bmSet(bm, "RELEASE_BLITZ", "");
			bmSet(bm, "RELEASE_LINKMODE", "0");
			bmSet(bm, "RELEASE_OPTIONS", "-O1 -ffunction-sections"); // -O3 after TDM is fixed
			bmSet(bm, "RELEASE_SIZE_OPTIONS", "-Os -O1 -finline-limit=20 -ffunction-sections"); // remove -O1 after TDM is fixed
			bmSet(bm, "RELEASE_FLAGS", "");
			bmSet(bm, "RELEASE_LINK", "");
			bmSet(bm, "DEBUGGER", "gdb");
			bmSet(bm, "ALLOW_PRECOMPILED_HEADERS", "1");
	//		bmSet(bm, "LINKMODE_LOCK", "0");
	
			String m = x64 ? "" : "32";
			bins.At(0) = bin + "/TDM64/bin";
			bins.At(1) = bin + "/TDM64/opt/bin" + m;
			bins.At(2) = bin + "/TDM64/gdb" + m + "/bin";
			incs.At(0) = bin + "/TDM64/i686-w64-mingw32/include";
			incs.At(1) = bin + "/TDM64/opt/include";
			libs.At(0) = bin + "/TDM64/i686-w64-mingw32/lib" + m;
			libs.At(1) = bin + "/TDM64/opt/lib" + m;
	
			bm.GetAdd("PATH") = Join(bins, ";");
			bm.GetAdd("INCLUDE") = Join(incs, ";");
			bm.GetAdd("LIB") = Join(libs, ";");
			
			SaveVarFile(ConfigFile(method + ".bm"), bm);
			dirty = true;
	
			if(!x64)
				default_method = Nvl(default_method, method);
		}

	if(default_method.GetCount())
		SaveFile(GetExeDirFile("default_method"), default_method);
	
	static Tuple2<const char *, const char *> ass[] = {
		{ "uppsrc", "#/uppsrc" },
		{ "reference", "#/reference;#/uppsrc" },
		{ "examples", "#/examples;#/uppsrc" },
		{ "tutorial", "#/tutorial;#/uppsrc" },
		{ "examples-bazaar", "#/bazaar;#/uppsrc" },
		{ "MyApps", "#/MyApps;#/uppsrc" },
		{ "MyApps-bazaar", "#/MyApps;#/bazaar;#/uppsrc" },
	};

	String exe = GetExeFilePath();
	String dir = GetFileFolder(exe);
	String out = GetExeDirFile("out");
	RealizeDirectory(out);

	for(int i = 0; i < __countof(ass); i++) {
		String vf = GetExeDirFile(String(ass[i].a) + ".var");
		VectorMap<String, String> map;
		bool ok = LoadVarFile(vf, map);
		if(ok) {
			Vector<String> dir = Split(map.Get("UPP", String()), ';');
			if(dir.GetCount() == 0)
				ok = false;
			else
				for(int j = 0; j < dir.GetCount(); j++) {
					if(!DirectoryExists(dir[j])) {
						ok = false;
						break;
					}
				}
		}
		if(!ok) {
			String b = ass[i].b;
			b.Replace("#", dir);
			SaveFile(vf,
				"UPP = " + AsCString(b) + ";\r\n"
				"OUTPUT = " + AsCString(out) + ";\r\n"
			);
			dirty = true;
		}
	}

	Ide *ide = dynamic_cast<Ide *>(TheIde());
	if(dirty && ide) {
		ide->SyncBuildMode();
		ide->CodeBaseSync();
		ide->SetBar();
	}
}

bool CheckLicense()
{
	if(!FileExists((GetExeDirFile("license.chk"))))
		return true;
	ShowSplash();
	Ctrl::ProcessEvents();
	Sleep(2000);
	HideSplash();
	Ctrl::ProcessEvents();
	WithLicenseLayout<TopWindow> d;
	CtrlLayoutOKCancel(d, "License agreement");
	d.license = GetTopic("ide/app/BSD$en-us").text;
	d.license.Margins(4);
	d.license.SetZoom(Zoom(Zy(18), 100));
	d.ActiveFocus(d.license);
	if(d.Run() != IDOK)
		return false;
	DeleteFile(GetExeDirFile("license.chk"));
	return true;
}

String GetWinRegStringWOW64(const char *value, const char *path, HKEY base_key  = HKEY_LOCAL_MACHINE) {
	HKEY key = 0;
	if(RegOpenKeyEx(base_key, path, 0, KEY_READ|KEY_WOW64_64KEY, &key) != ERROR_SUCCESS)
		return String::GetVoid();
	dword type, data;
	if(RegQueryValueEx(key, value, 0, &type, NULL, &data) != ERROR_SUCCESS)
	{
		RegCloseKey(key);
		return String::GetVoid();
	}
	StringBuffer raw_data(data);
	if(RegQueryValueEx(key, value, 0, 0, (byte *)~raw_data, &data) != ERROR_SUCCESS)
	{
		RegCloseKey(key);
		return String::GetVoid();
	}
	if(data > 0 && (type == REG_SZ || type == REG_EXPAND_SZ))
		data--;
	raw_data.SetLength(data);
	RegCloseKey(key);
	return raw_data;
}

void AutoInstantSetup()
{
	String sgn = ToLower(GetFileFolder(GetExeFilePath())) + "\n" +
	             GetWinRegStringWOW64("MachineGuid", "SOFTWARE\\Microsoft\\Cryptography");
	String cf = GetExeDirFile("setup-path");
	String sgn0 =  LoadFile(cf);
	if(sgn != sgn0) {
		InstantSetup();
		SaveFile(cf, sgn);
		SaveFile(cf + ".old", sgn0); // forensics
	}
}

#endif
