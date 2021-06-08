#include "Builders.h"

String MakeIdent(const char *name)
{
	String out;
	for(; *name; name++)
		out << (iscid(*name) ? *name : '_');
	return out;
}

static String MakeSourcePath(const Vector<String>& dirs, String fn, bool raw, bool exporting)
{
	fn = UnixPath(fn);
	for(int i = 0; i < dirs.GetCount(); i++)
	{
		int dl = dirs[i].GetLength();
		if(fn.GetLength() >= dl + 2 && !memcmp(fn, dirs[i], dl) && fn[dl] == '/') {
			String s;
			if(!exporting)
				s << "$(UPPDIR" << (i + 1) << ")";
			s << AdjustMakePath(fn.GetIter(dl + 1));
			return s;
		}
	}
	return raw ? String() : AdjustMakePath(fn);
}

String CppBuilder::GetMakePath(String fn) const
{
	return ::GetMakePath(fn, HasFlag("WIN32"));
}

void CppBuilder::AddMakeFile(MakeFile& makefile, String package,
	const Vector<String>& all_uses, const Vector<String>& all_libraries,
	const Index<String>& common_config, bool exporting)
{
	String packagepath = PackagePath(package);
	Package pkg;
	pkg.Load(packagepath);
	String packagedir = GetFileFolder(packagepath);
	Vector<String> src = GetUppDirs();
	for(int i = 0; i < src.GetCount(); i++)
		src[i] = UnixPath(src[i]);

	bool main = HasFlag("MAIN");
	bool is_shared = HasFlag("SO");
	bool libout = !main && !HasFlag("NOLIB");
	bool win32 = HasFlag("WIN32");

	String pack_ident = MakeIdent(package);
	String outdir = "OutDir_" + pack_ident;
	String macros = "Macro_" + pack_ident;
	String macdef = "$(Macro)";
	String objext = (HasFlag("MSC") || HasFlag("EVC") ? ".obj" : ".o");

	Vector<String> x(config.GetKeys(), 1);
	Sort(x);
	for(int i = 0; i < x.GetCount(); i++) {
		if(common_config.Find(x[i]) < 0)
			macdef << " -Dflag" << x[i];
		x[i] = InitCaps(x[i]);
	}

	makefile.outdir << "$(" << outdir << ")";
	makefile.outfile << AdjustMakePath(GetFileTitle(NativePath(package)));
	if(main)
		makefile.outfile << GetTargetExt();
	else if(is_shared)
		makefile.outfile << (win32 ? ".dll" : ".so");
	else
		makefile.outfile << (win32 && HasFlag("MSC") ? ".lib" : ".a");
	makefile.output << (main ? String("$(OutDir)") : makefile.outdir) << makefile.outfile;

	if(main) {
		makefile.config << "CXX = c++\n"
			"LINKER = $(CXX)\n";
		String flags;
		if(HasFlag("DEBUG"))
			flags << " -D_DEBUG " << debug_options;
		else
			flags << ' ' << release_options;
		if(HasFlag("DEBUG_MINIMAL"))
			flags << " -ggdb -g1";
		if(HasFlag("DEBUG_FULL"))
			flags << " -ggdb -g2";
		if(is_shared && !win32)
			flags << " -fPIC ";
		flags << ' ' << Gather(pkg.option, config.GetKeys());
		makefile.config << "CFLAGS =" << Merge(" ", flags, c_options) << "\n"
			"CXXFLAGS =" << Merge(" ", flags, cpp_options) << "\n"
			"LDFLAGS = " << Merge(" ", common_link, HasFlag("DEBUG") ? debug_link : release_link)
			             << " $(LINKOPTIONS)\n"
			"LIBPATH =";
		for(int i = 0; i < libpath.GetCount(); i++)
			makefile.config << " -L" << GetMakePath(AdjustMakePath(GetPathQ(libpath[i])));
		makefile.config << "\n"
			"AR = ar -sr\n\n";
		Vector<String> lib;
		String lnk;
		lnk << "$(LINKER)";
		if(!HasFlag("SHARED"))
			lnk << " -static";
		if(HasFlag("WIN32")) {
			lnk << " -mwindows";
			if(!HasFlag("GUI"))
				makefile.linkfiles << " -mconsole";
		}
		lnk << " -o \"$(OutFile)\"";
		if(HasFlag("DEBUG") || HasFlag("DEBUG_MINIMAL") || HasFlag("DEBUG_FULL"))
			lnk << " -ggdb";
		else
			lnk << (!HasFlag("OSX11") ? " -Wl,-s" : "");

		lnk << " $(LIBPATH)";
		if (!HasFlag("OSX11"))
			lnk << " -Wl,-O,2";
		lnk << " $(LDFLAGS) -Wl,--start-group ";

		makefile.linkfiles = lnk;
	}

	makefile.config << outdir << " = $(UPPOUT)"
		<< GetMakePath(AdjustMakePath(String().Cat() << package << '/' << method << '-' << Join(x, "-") << '/')) << "\n"
		<< macros << " = " << macdef << "\n";

	makefile.install << " \\\n\t$(" << outdir << ")";
	makefile.rules << "$(" << outdir << "):\n\tmkdir -p $(" << outdir << ")\n\n";

	String libdep, libfiles;

	libdep << makefile.output << ":";
	if(is_shared)
	{
		libfiles = "c++ -shared -fPIC"; // -v";
		Point p = ExtractVersion();
		if(!IsNull(p.x)) {
			libfiles << " -Xlinker --major-image-version -Xlinker " << p.x;
			if(!IsNull(p.y))
				libfiles << " -Xlinker --minor-image-version -Xlinker " << p.y;
		}
		libfiles << " -o ";
	}
	else
		libfiles = "$(AR) ";
	libfiles << makefile.output;

	Vector<String> libs = Split(Gather(pkg.library, config.GetKeys()), ' ');
	for(int i = 0; i < libs.GetCount(); i++) {
		String ln = libs[i];
		String ext = ToLower(GetFileExt(ln));
		if(ext == ".a" || ext == ".so" || ext == ".dll")
			makefile.linkfileend << " \\\n\t\t\t" << GetPathQ(FindInDirs(libpath, ln));
		else
			makefile.linkfileend << " \\\n\t\t\t-l" << ln;
	}

	for(int i = 0; i < pkg.GetCount(); i++)
		if(!pkg[i].separator) {
			String gop = Gather(pkg[i].option, config.GetKeys());
			String fn = SourcePath(package, pkg[i]);
			String ext = ToLower(GetFileExt(fn));
			bool isc = ext == ".c";
			bool isrc = (ext == ".rc" && HasFlag("WIN32"));
			bool iscpp = (ext == ".cpp" || ext == ".cc" || ext == ".cxx");
			bool isicpp = (ext == ".icpp");
			if(ext == ".brc") {
				isc = true;
				fn << "c";
			}
			if(isc || isrc || iscpp || isicpp) {
				String outfile;
				outfile << makefile.outdir << AdjustMakePath(GetFileTitle(fn)) << (isrc ? "_rc" : "") << objext;
				String srcfile = GetMakePath(MakeSourcePath(src, fn, false, exporting));
				makefile.rules << outfile << ": " << srcfile;
				Vector<String> dep = HdependGetDependencies(fn, false);
				Sort(dep, GetLanguageInfo());
				for(int d = 0; d < dep.GetCount(); d++) {
					String dfn = MakeSourcePath(src, dep[d], true, exporting);
					if(!IsNull(dfn))
						makefile.rules << " \\\n\t" << GetMakePath(dfn);
				}
				makefile.rules << "\n"
					"\t$(CXX) -c " << (isc ? "-x c $(CFLAGS)" : "-x c++ $(CXXFLAGS)") << " $(CINC) $(" << macros << ") "
						<< gop << " " << srcfile << " -o " << outfile << "\n\n";
				if(!libout || isicpp) {
					makefile.linkdep << " \\\n\t" << outfile;
					makefile.linkfiles << " \\\n\t\t" << outfile;
				}
				else {
					libdep << " \\\n\t" << outfile;
					libfiles << " \\\n\t\t" << outfile;
				}
			}
			else
			if(ext == ".o" || ext == ".obj" || ext == ".a" || ext == ".so" || ext == ".lib" || ext == ".dll") {
				makefile.linkdep << " \\\n\t" << fn;
				makefile.linkfiles << ' ' << fn;
			}
		}

	if(libout) {
		makefile.rules << libdep << "\n\t" << libfiles << "\n\n";
		makefile.linkdep << " \\\n\t" << makefile.output;
		makefile.linkfiles << " \\\n\t\t\t" << makefile.output;
	}
/*
	if(main) {
		if(!HasFlag("SOLARIS")&&!HasFlag("OSX11"))
			makefile.linkfiles << " \\\n\t\t-Wl,--start-group ";
		DDUMPC(all_libraries);
		for(int i = 0; i < all_libraries.GetCount(); i++) {
			String ln = all_libraries[i];
			String ext = ToLower(GetFileExt(ln));
			if(ext == ".a" || ext == ".so" || ext == ".dll")
				makefile.linkfileend << " \\\n\t\t\t" << GetHostPathQ(FindInDirs(libpath, ln));
			else
				makefile.linkfileend << " \\\n\t\t\t-l" << ln;
		}
		if(!HasFlag("SOLARIS")&&!HasFlag("OSX11"))
			makefile.linkfileend << " \\\n\t\t-Wl,--end-group\n\n";
	}
*/
}

JsonArray& operator<<(JsonArray& array, const Vector<String>& v) {
	for (const String& s: v)
		array << s;
	return array;
}

Vector<String>& operator<<(Vector<String>& array, const Vector<String>& v) {
	array.Append(v);
	return array;
}

Point CppBuilder::ExtractVersion() const
{
	Point v = Point(Null, Null);
	try {
		CParser p(version);
		while(!p.IsEof()) {
			if(p.IsNumber()) {
				v.x = p.ReadNumber();
				break;
			}
			p.GetChar();
			p.Spaces();
		}
		while(!p.IsEof()) {
			if(p.IsNumber()) {
				v.y = p.ReadNumber();
				break;
			}
			p.GetChar();
			p.Spaces();
		}
	}
	catch(CParser::Error) {}
	return v;
}

void CppBuilder::ShowTime(int count, int start_time)
{
	if(count)
		PutConsole(Format("%d file(s) compiled in %s %d msec/file",
			count, GetPrintTime(start_time), msecs(start_time) / count));
}

void MakeBuild::SaveMakeFile(const String& fn, bool exporting)
{
	BeginBuilding(true);

	VectorMap<String, String> bm = GetMethodVars(method);
	Host host;
	CreateHost(host, false, false);
	One<Builder> b = CreateBuilder(&host);

	if(!b)
		return;

	const TargetMode& tm = GetTargetMode();

	String makefile;

	Vector<String> uppdirs = GetUppDirs();
	String uppout = exporting ? GetVar("OUTPUT") : ".cache/upp.out";
	String inclist;

	Index<String> allconfig = PackageConfig(GetIdeWorkspace(), 0, bm, mainconfigparam, host, *b);
	bool win32 = allconfig.Find("WIN32") >= 0;

	Workspace wspc;
	wspc.Scan(GetMain(), allconfig.GetKeys());

	Index<String> pkg_config;
	for(int i = 0; i < wspc.GetCount(); i++) {
		Index<String> modconfig = PackageConfig(wspc, i, bm, mainconfigparam, host, *b);
		PkgConfig(wspc, modconfig, pkg_config);
		if(i)
			for(int a = allconfig.GetCount(); --a >= 0;)
				if(modconfig.Find(allconfig[a]) < 0)
					allconfig.Remove(a);
	}

	if(!exporting)
		for(int i = 0; i < uppdirs.GetCount(); i++) {
			String srcdir = GetMakePath(AdjustMakePath(AppendFileName(uppdirs[i], "")), win32);
			makefile << "UPPDIR" << (i + 1) << " = " << srcdir << "\n";
			inclist << " -I$(UPPDIR" << (i + 1) << ")";
		}

	for(String s : pkg_config)
		inclist << " `pkg-config --cflags " << s << "`";

	Vector<String> includes = SplitDirs(bm.Get("INCLUDE",""));
	for(int i = 0; i < includes.GetCount(); i++)
		inclist << " -I" << includes[i];

	inclist << " -I./";
	inclist << " -I$(UPPOUT)"; // build_info.h is created there

	makefile << "\n"
		"UPPOUT = " << (exporting ? "_out/" : GetMakePath(AdjustMakePath(AppendFileName(uppout, "")), win32)) << "\n"
		"CINC   = " << inclist << "\n"
		"Macro  = ";

	for(int i = 0; i < allconfig.GetCount(); i++)
		makefile << " -Dflag" << allconfig[i];
	makefile << "\n";

	String output, config, install, rules, linkdep, linkfiles, linkfileend;

	for(String s : pkg_config)
		linkfileend << " \\\n\t\t\t`pkg-config --libs " << s << "`";

	for(int i = 0; i < wspc.GetCount(); i++) {
		b->config = PackageConfig(wspc, i, bm, mainconfigparam, host, *b);
		b->version = tm.version;
		b->method = method;
		MakeFile mf;
		b->AddMakeFile(mf, wspc[i], GetAllUses(wspc, i, bm, mainconfigparam, host, *b),
		               GetAllLibraries(wspc, i, bm, mainconfigparam, host, *b), allconfig,
		               exporting);
		if(i == 0) { // main package
			String tdir = mf.outdir;
			String trg;
			if(tm.target_override) {
				trg = GetMakePath(AdjustMakePath(tm.target), win32);
				if(!trg.IsEmpty() && *trg.Last() == (win32 ? '\\' : '/'))
					trg << mf.outfile;
				else if(trg.Find(win32 ? '\\' : '/') < 0)
					trg.Insert(0, "$(OutDir)");
			}
			else
			if(exporting)
				trg = wspc[i] + ".out";
			else
				trg = "./" + wspc[0];
			output = Nvl(trg, mf.output);
			while(DirectoryExists(output))
				output << ".out";
			StringStream ss;
			String svn_info;
			String build_info = "\"$(UPPOUT)/build_info.h\"";
			if(makefile_svn_revision) {
				Vector<String> bi = SvnInfo(wspc[i]);
				for(int i = 0; i < bi.GetCount(); i++)
					svn_info << "	echo '" << bi[i] << "' >> " << build_info << "\n";
			}
			install << "\n"
				"OutDir = " << tdir << "\n"
				"OutFile = " << output << "\n"
				"\n"
				".PHONY: all\n"
				"all: prepare $(OutFile)\n"
				"\n"
				".PHONY: build_info\n"
				"build_info:\n"
				"	(date '+#define bmYEAR    %y%n"
				"#define bmMONTH   %m%n"
				"#define bmDAY     %d%n"
				"#define bmHOUR    %H%n"
				"#define bmMINUTE  %M%n"
				"#define bmSECOND  %S%n"
				"#define bmTIME    Time( %y, %m, %d, %H, %M, %S )' | sed 's| 0\\([[:digit:]]\\)| \\1|g' && \\\n"
				"	echo '#define bmMACHINE \"'`hostname`'\"' && \\\n"
				"	echo '#define bmUSER    \"'`whoami`'\"') > " << build_info << "\n"
				<< svn_info <<
				"\n"
				".PHONY: prepare\n"
				"prepare:";
		}
		config << mf.config;
		install << mf.install;
		rules << mf.rules;
		linkdep << mf.linkdep;
		linkfiles << mf.linkfiles;
		linkfileend << mf.linkfileend;
	}

	makefile
		<< config
		<< install
		<< "\n\n"
		"$(OutFile): build_info " << linkdep << "\n\t" << linkfiles << linkfileend << " -Wl,--end-group\n\n"
		<< rules
		<< ".PHONY: clean\n"
		<< "clean:\n"
		<< "\tif [ \"$(UPPOUT)\" != \"\" -a \"$(UPPOUT)\" != \"/\" -a -d \"$(UPPOUT)\" ] ; then rm -fr \"$(UPPOUT)\" ; fi\n"
	;

	bool sv = ::SaveFile(fn, makefile);
	if(!exporting) {
		if(sv)
			PutConsole(Format("%s(1): makefile generation complete", fn));
		else
			PutConsole(Format("%s: error writing makefile", fn));
	}
	EndBuilding(true);
}
