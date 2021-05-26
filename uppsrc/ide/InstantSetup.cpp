#include "ide.h"

#ifdef PLATFORM_WIN32

struct DirFinder {
	Progress pi;

	Vector<String> dir;

	String Get(const String& substring, const char *files);
	void   GatherDirs(Index<String>& path, const String& dir);
	
	DirFinder();
};

void DirFinder::GatherDirs(Index<String>& path, const String& dir)
{
	pi.Step();
	path.FindAdd(dir);
	FindFile ff(dir + "/*");
	while(ff) {
		if(ff.IsFolder())
			GatherDirs(path, ff.GetPath());
		ff.Next();
	}
}

DirFinder::DirFinder()
{
	Index<String> path;
	
	pi.SetText("Setting up build methods");

	for(int i = 0; i < 3; i++) {
		HKEY key = 0;
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		                "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\Folders", 0,
		                KEY_READ|decode(i, 0, KEY_WOW64_32KEY, 1, KEY_WOW64_64KEY, 0),
		                &key) == ERROR_SUCCESS) {
			int i = 0;
			for(;;) {
				char  value[255];
				unsigned long valueSize = 250;
				*value = 0;
		
				if(RegEnumValue(key, i++, value, &valueSize, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
					break;
				
				path.FindAdd(value);
				pi.Step();
			}
			RegCloseKey(key);
		}
	}
	
	Array<FileSystemInfo::FileInfo> root = StdFileSystemInfo().Find(Null);
	for(int i = 0; i < root.GetCount(); i++) {
		if(root[i].root_style == FileSystemInfo::ROOT_FIXED) {
			int drive = *root[i].filename;
			String pf = GetProgramsFolderX86();
			pf.Set(0, drive);
			pf = AppendFileName(pf, "Microsoft Visual Studio");
			if(DirectoryExists(pf))
				GatherDirs(path, pf);
		}
	}

	for(String s : path) {
		s = ToLower(s);
		s.Replace("\\", "/");
		while(s.TrimEnd("/"));
		dir.Add(s);
	}
}

String DirFinder::Get(const String& substring, const char *files)
{
	String path;
	Vector<int> versions;
	Vector<String> fn = Split(files, ';');
	for(const auto& d : dir) {
		pi.Step();
		int p = d.Find(substring);
		if(p >= 0) {
			String cp = d;
			for(const auto& f : fn)
				if(!FileExists(AppendFileName(d, f))) {
					cp.Clear();
					break;
				}
			if(cp.GetCount()) {
				Vector<String> h = Split(cp.Mid(p), [] (int c) { return IsDigit(c) ? 0 : c; });
				Vector<int> vers;
				for(const auto& hh : h)
					vers.Add(atoi(hh));
				if(CompareRanges(vers, versions) > 0) {
					path = cp;
					versions = clone(vers);
				}
			}
		}
	}
	return path;
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
	bool dirty = false;
	String default_method;
	
	String bin = GetExeDirFile("bin");

	if(DirectoryExists(bin + "/clang"))
		for(int x64 = 0; x64 < 2; x64++) {
			String method = x64 ? "CLANGx64" : "CLANG";
		#ifdef INSTANT_TESTING
			method << "Test";
		#endif
			VectorMap<String, String> bm = GetMethodVars(method);
	
			Vector<String> bins = Split(bm.Get("PATH", ""), ';');
			Vector<String> incs = Split(bm.Get("INCLUDE", ""), ';');
			Vector<String> libs = Split(bm.Get("LIB", ""), ';');
		#ifdef INSTANT_TESTING
			if(CheckDirs(bins, 3) && CheckDirs(incs, 2) && CheckDirs(libs, 2)) {
				if(!x64)
					default_method = Nvl(default_method, method);
				continue;
			}
		#endif
	
			bmSet(bm, "BUILDER", "CLANG");
			bmSet(bm, "COMPILER", x64 ? "" : "i686-w64-mingw32-c++");
			bmSet(bm, "COMMON_OPTIONS", "");
			bmSet(bm, "COMMON_CPP_OPTIONS", "");
			bmSet(bm, "COMMON_C_OPTIONS", "");
			bmSet(bm, "COMMON_LINK", "");
			bmSet(bm, "COMMON_FLAGS", "");
			bmSet(bm, "DEBUG_INFO", "2");
			bmSet(bm, "DEBUG_BLITZ", "1");
			bmSet(bm, "DEBUG_LINKMODE", "0");
			bmSet(bm, "DEBUG_OPTIONS", "");
			bmSet(bm, "DEBUG_FLAGS", "");
			bmSet(bm, "DEBUG_LINK", "-Wl,--stack,20000000");
			bmSet(bm, "RELEASE_BLITZ", "1");
			bmSet(bm, "RELEASE_LINKMODE", "0");
			bmSet(bm, "RELEASE_OPTIONS", "-O3 ");
			bmSet(bm, "RELEASE_FLAGS", "");
			bmSet(bm, "RELEASE_LINK", "-Wl,--stack,20000000");
			bmSet(bm, "DEBUGGER", "gdb");
			bmSet(bm, "ALLOW_PRECOMPILED_HEADERS", "1");
			bmSet(bm, "DISABLE_BLITZ", "");
			
	//		bmSet(bm, "LINKMODE_LOCK", "0");
	
			String clang = bin + "/clang";

			bins.At(0) = clang + "/bin";
			bins.At(1) = clang + (x64 ? "/x86_64-w64-mingw32/bin" : "/i686-w64-mingw32/bin");
			bins.At(2) = GetExeDirFile(x64 ? "bin/SDL2/lib/x64" : "bin/SDL2/lib/x86");
			bins.At(3) = GetExeDirFile(x64 ? "bin/pgsql/x64/bin" : "bin/pgsql/x86/bin");
			bins.At(4) = GetExeDirFile(x64 ? "bin/mysql/lib64" : "bin/mysql/lib32");

			incs.At(0) = GetExeDirFile("bin/SDL2/include");
			incs.At(1) = GetExeDirFile(x64 ? "bin/pgsql/x64/include" : "bin/pgsql/x86/include");
			incs.At(2) = GetExeDirFile(x64 ? "bin/mysql/include" : "bin/mysql/include");

			libs.At(0) = GetExeDirFile(x64 ? "bin/SDL2/lib/x64" : "bin/SDL2/lib/x86");
			libs.At(1) = GetExeDirFile(x64 ? "bin/pgsql/x64/lib" : "bin/pgsql/x86/lib");
			libs.At(2) = GetExeDirFile(x64 ? "bin/mysql/lib64" : "bin/mysql/lib32");
	
			bm.GetAdd("PATH") = Join(bins, ";");
			bm.GetAdd("INCLUDE") = Join(incs, ";");
			bm.GetAdd("LIB") = Join(libs, ";");
			
			SaveVarFile(ConfigFile(method + ".bm"), bm);
			dirty = true;
	
			if(x64)
				default_method = Nvl(default_method, method);
		}

	enum { VS_2015, VS_2017, BT_2017, VS_2019, VSP_2019, BT_2019 };
	DirFinder df;

	for(int version = VS_2015; version <= BT_2019; version++)
		for(int x64 = 0; x64 < 2; x64++) {
			String x86method = decode(version, VS_2015, "MSVS15",
			                                   VS_2017, "MSVS17", BT_2017, "MSBT17",
			                                   VS_2019, "MSVS19", VSP_2019, "MSVC19P", BT_2019, "MSBT19",
			                                   "MSBT");
			String x64s = x64 ? "x64" : "";
			String method = x86method + x64s;
			String builder = decode(version, VS_2015, "MSC15",
			                                 VS_2017, "MSC17", BT_2017, "MSC17",
			                                 VS_2019, "MSC19", VSP_2019, "MSC19", BT_2019, "MSC19",
			                                 "MSC19"
			                 ) + ToUpper(x64s);
		
		#ifdef INSTANT_TESTING
			method << "Test";
		#endif
	
			String vc, bin, inc, lib, kit81;
		
			VectorMap<String, String> bm = GetMethodVars(method);
			Vector<String> bins = Split(bm.Get("PATH", ""), ';');
			Vector<String> incs = Split(bm.Get("INCLUDE", ""), ';');
			Vector<String> libs = Split(bm.Get("LIB", ""), ';');
		#ifdef INSTANT_TESTING
			if(CheckDirs(bins, 2) && CheckDirs(incs, 4) && CheckDirs(libs, 3)) {
				if(x64)
					default_method = Nvl(default_method, x86method);
			
				continue;
			}
		#endif

			if(version == VS_2015)
				vc = df.Get("/microsoft visual studio 14.0/vc", "bin/cl.exe;bin/lib.exe;bin/link.exe;bin/mspdb140.dll");
			else
				vc = df.Get(decode(version, BT_2017, "/microsoft visual studio/2017/buildtools/vc/tools/msvc",
				                            VS_2017, "/microsoft visual studio/2017/community/vc/tools/msvc",
				                            BT_2019, "/microsoft visual studio/2019/buildtools/vc/tools/msvc",
				                            VS_2019, "/microsoft visual studio/2019/community/vc/tools/msvc",
				                            VSP_2019, "/microsoft visual studio/2019/professional/vc/tools/msvc",
				                            ""),
				            x64 ? "bin/hostx64/x64/cl.exe;bin/hostx64/x64/mspdb140.dll"
				                : "bin/hostx86/x86/cl.exe;bin/hostx86/x86/mspdb140.dll");

			bin = df.Get("/windows kits/10/bin", "x86/makecat.exe;x86/accevent.exe");
			inc = df.Get("/windows kits/10", "um/adhoc.h");
			lib = df.Get("/windows kits/10", "um/x86/kernel32.lib");
			
			bool ver17 = version >= VS_2017;
	
			if(inc.GetCount() == 0 || lib.GetCount() == 0) // workaround for situation when 8.1 is present, but 10 just partially
				kit81 = df.Get("/windows kits/8.1", "include");
			
			LOG("=============");
			DUMP(method);
			DUMP(vc);
			DUMP(bin);
			DUMP(inc);
			DUMP(kit81);
			DUMP(lib);
	
			if(vc.GetCount() && bin.GetCount() && (inc.GetCount() && lib.GetCount() || kit81.GetCount())) {
				bins.At(0) = vc + (ver17 ? (x64 ? "/bin/hostx64/x64" : "/bin/hostx86/x86") : (x64 ? "/bin/amd64" : "/bin"));
				bins.At(1) = bin + (x64 ? "/x64" : "/x86");

				int ii = 2;

				String& sdlbin = bins.At(ii++);
				if(IsNull(sdlbin) || ToLower(sdlbin).Find("sdl2") >= 0)
					sdlbin = GetExeDirFile(x64 ? "bin/SDL2/lib/x64" : "bin/SDL2/lib/x86");
				
				String& pgbin = bins.At(ii++);
				if(IsNull(pgbin) || ToLower(pgbin).Find("pgsql") >= 0)
					pgbin = GetExeDirFile(x64 ? "bin/pgsql/x64/bin" : "bin/pgsql/x86/bin");

				String& mybin = bins.At(ii++);
				if(IsNull(mybin) || ToLower(mybin).Find("pgsql") >= 0)
					mybin = GetExeDirFile(x64 ? "bin/mysql/lib64" : "bin/mysql/lib32");
				
				incs.At(0) = vc + "/include";
				ii = 1;
				if(inc.GetCount()) {
					incs.At(ii++) = inc + "/um";
					incs.At(ii++) = inc + "/ucrt";
					incs.At(ii++) = inc + "/shared";
				}
				if(kit81.GetCount()) {
					incs.At(ii++) = kit81 + "/include/um";
					incs.At(ii++) = kit81 + "/include/ucrt";
					incs.At(ii++) = kit81 + "/include/shared";
				}
	
				String& sslinc = incs.At(ii++);
				if(IsNull(sslinc) || ToLower(sslinc).Find("openssl") >= 0)
					sslinc = GetExeDirFile("bin/openssl/include");

				String& sdlinc = incs.At(ii++);
				if(IsNull(sdlinc) || ToLower(sdlinc).Find("sdl2") >= 0)
					sdlinc = GetExeDirFile("bin/SDL2/include");

				String& pginc = incs.At(ii++);
				if(IsNull(pginc) || ToLower(pginc).Find("pgsql") >= 0)
					pginc = GetExeDirFile(x64 ? "bin/pgsql/x64/include" : "bin/pgsql/x86/include");

				String& myinc = incs.At(ii++);
				if(IsNull(myinc) || ToLower(myinc).Find("mysql") >= 0)
					myinc = GetExeDirFile(x64 ? "bin/mysql/include" : "bin/mysql/include");
				
				libs.At(0) = vc + (ver17 ? (x64 ? "/lib/x64" : "/lib/x86") : (x64 ? "/lib/amd64" : "/lib"));
				ii = 1;
				if(lib.GetCount()) {
					libs.At(ii++) = lib + (x64 ? "/ucrt/x64" : "/ucrt/x86");
					libs.At(ii++) = lib + (x64 ? "/um/x64" : "/um/x86");
				}
				if(kit81.GetCount()) {
					libs.At(ii++) = kit81 + (x64 ? "/lib/winv6.3/um/x64" : "/lib/winv6.3/um/x86");
				}
				String& ssllib = libs.At(ii++);
				if(IsNull(ssllib) || ToLower(ssllib).Find("openssl") >= 0)
					ssllib = GetExeDirFile(x64 ? "bin/openssl/lib64" : "bin/openssl/lib32");
				
				String& sdllib = libs.At(ii++);
				if(IsNull(sdllib) || ToLower(sdllib).Find("sdl2") >= 0)
					sdllib = GetExeDirFile(x64 ? "bin/SDL2/lib/x64" : "bin/SDL2/lib/x86");

				String& pglib = libs.At(ii++);
				if(IsNull(pglib) || ToLower(pglib).Find("pgsql") >= 0)
					pglib = GetExeDirFile(x64 ? "bin/pgsql/x64/lib" : "bin/pgsql/x86/lib");

				String& mylib = libs.At(ii++);
				if(IsNull(mylib) || ToLower(mylib).Find("pgsql") >= 0)
					mylib = GetExeDirFile(x64 ? "bin/mysql/lib64" : "bin/mysql/lib32");
			
				bm.GetAdd("BUILDER") = builder;
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
				bmSet(bm, "DEBUG_LINK", x64 ? "/STACK:20000000" : "/STACK:10000000");
				bmSet(bm, "RELEASE_BLITZ", "0");
				bmSet(bm, "RELEASE_LINKMODE", "0");
				bmSet(bm, "RELEASE_OPTIONS", "-O2");
				bmSet(bm, "RELEASE_FLAGS", "");
				bmSet(bm, "RELEASE_LINK", x64 ? "/STACK:20000000" : "/STACK:10000000");
				bmSet(bm, "DISABLE_BLITZ", "");
				bmSet(bm, "ALLOW_PRECOMPILED_HEADERS", "1");
				bmSet(bm, "DEBUGGER", findarg(version, BT_2017, BT_2019) >= 0 ? String()
				                      : GetFileFolder(vc) +  "/Common7/IDE/devenv.exe");

				bm.GetAdd("PATH") = Join(bins, ";");
				bm.GetAdd("INCLUDE") = Join(incs, ";");
				bm.GetAdd("LIB") = Join(libs, ";");
				
				SaveVarFile(ConfigFile(method + ".bm"), bm);
				dirty = true;
	
				if(!x64)
					default_method = Nvl(default_method, x86method);
	
				DUMP(ConfigFile(method + ".bm"));
				DUMPC(incs);
				DUMPC(libs);
				DUMPM(bm);
			}
		}

	if(DirectoryExists(bin + "/mingw64")) // hopefully deprecated, but keep it for now
		for(int x64 = 0; x64 < 2; x64++) {
			String method = x64 ? "MINGWx64" : "MINGW";
		#ifdef INSTANT_TESTING
			method << "Test";
		#endif
			VectorMap<String, String> bm = GetMethodVars(method);
	
			Vector<String> bins = Split(bm.Get("PATH", ""), ';');
			Vector<String> incs = Split(bm.Get("INCLUDE", ""), ';');
			Vector<String> libs = Split(bm.Get("LIB", ""), ';');
		#ifdef INSTANT_TESTING
			if(CheckDirs(bins, 3) && CheckDirs(incs, 2) && CheckDirs(libs, 2)) {
				if(!x64)
					default_method = Nvl(default_method, method);
				continue;
			}
		#endif
	
			bmSet(bm, "BUILDER", "GCC");
			bmSet(bm, "COMPILER", "");
			bmSet(bm, "COMMON_OPTIONS", "-msse2 -D__CRT__NO_INLINE");
			bmSet(bm, "COMMON_CPP_OPTIONS", "-std=c++14");
			bmSet(bm, "COMMON_C_OPTIONS", "");
			bmSet(bm, "COMMON_LINK", "");
			bmSet(bm, "COMMON_FLAGS", "");
			bmSet(bm, "DEBUG_INFO", "2");
			bmSet(bm, "DEBUG_BLITZ", "1");
			bmSet(bm, "DEBUG_LINKMODE", "0");
			bmSet(bm, "DEBUG_OPTIONS", "-O0");
			bmSet(bm, "DEBUG_FLAGS", "");
			bmSet(bm, "DEBUG_LINK", "-Wl,--stack,20000000");
			bmSet(bm, "RELEASE_BLITZ", "1");
			bmSet(bm, "RELEASE_LINKMODE", "0");
			bmSet(bm, "RELEASE_OPTIONS", "-O2 -ffunction-sections");
			bmSet(bm, "RELEASE_FLAGS", "");
			bmSet(bm, "RELEASE_LINK", "-Wl,--stack,20000000");
			bmSet(bm, "DEBUGGER", "gdb");
			bmSet(bm, "ALLOW_PRECOMPILED_HEADERS", "1");
			bmSet(bm, "DISABLE_BLITZ", "");
			
	//		bmSet(bm, "LINKMODE_LOCK", "0");
	
			String m = x64 ? "64" : "32";
			String binx = bin + (x64 ? "/mingw64/64" : "/mingw64/32");
			String mingw = binx + (x64 ? "/x86_64-w64-mingw32" : "/i686-w64-mingw32");

			bins.At(0) = binx + "/bin";
			bins.At(1) = binx + "/opt/bin";
			bins.At(2) = binx + "/gdb/bin";
			bins.At(3) = GetExeDirFile(x64 ? "bin/SDL2/lib/x64" : "bin/SDL2/lib/x86");
			bins.At(4) = GetExeDirFile(x64 ? "bin/pgsql/x64/bin" : "bin/pgsql/x86/bin");
			bins.At(5) = GetExeDirFile(x64 ? "bin/mysql/lib64" : "bin/mysql/lib32");

			incs.At(0) = mingw + "/include";
			incs.At(1) = binx + "/opt/include";
			incs.At(2) = GetExeDirFile("bin/SDL2/include");
			incs.At(3) = GetExeDirFile(x64 ? "bin/pgsql/x64/include" : "bin/pgsql/x86/include");
			incs.At(4) = GetExeDirFile(x64 ? "bin/mysql/include" : "bin/mysql/include");

			libs.At(0) = mingw + "/lib";
			libs.At(1) = binx + "/opt/lib";
			libs.At(2) = GetExeDirFile(x64 ? "bin/SDL2/lib/x64" : "bin/SDL2/lib/x86");
			libs.At(3) = GetExeDirFile(x64 ? "bin/pgsql/x64/lib" : "bin/pgsql/x86/lib");
			libs.At(4) = GetExeDirFile(x64 ? "bin/mysql/lib64" : "bin/mysql/lib32");
	
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
	
	static Tuple<const char *, const char *> ass[] = {
		{ "uppsrc", "#/uppsrc" },
		{ "reference", "#/reference;#/uppsrc" },
		{ "examples", "#/examples;#/uppsrc" },
		{ "tutorial", "#/tutorial;#/uppsrc" },
		{ "MyApps", "#/MyApps;#/uppsrc" },
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

void AutoInstantSetup()
{
	String sgn = ToLower(GetFileFolder(GetExeFilePath())) + "\n" +
	             GetWinRegString("MachineGuid", "SOFTWARE\\Microsoft\\Cryptography", HKEY_LOCAL_MACHINE, KEY_WOW64_64KEY);
	String cf = GetExeDirFile("setup-path");
	String sgn0 =  LoadFile(cf);
	if(sgn != sgn0) {
		InstantSetup();
		SaveFile(cf, sgn);
		SaveFile(cf + ".old", sgn0); // forensics
	}
}

#endif
