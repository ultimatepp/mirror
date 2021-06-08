#include "Builders.h"

inline
String SString(const String& s) {
	if (s.Find(' ') >= 0)
		return "\"" << s << "\"";
	return s;
}

inline
String SAppend(String l, const String& r) {
	return l << SString(r);
}

void CppBuilder::AddCCJ(
	MakeFile& makefile,
	String package,
	const Index<String>& common_config,
	bool exporting,
	bool last_ws)
{
	const char* eol = "\n";
	const char* tab = "\t";

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
	Vector<String> macdef;
	String objext = (HasFlag("MSC") || HasFlag("EVC") ? ".obj" : ".o");

	macdef.Append(Macro);

	Vector<String> x(config.GetKeys(), 1);
	Sort(x);
	for(int i = 0; i < x.GetCount(); i++) {
		if(common_config.Find(x[i]) < 0) {
			String str;
			str << "-Dflag" << x[i];
			macdef << str;
		}
		x[i] = InitCaps(x[i]);
	}

	makefile.outdir << outdir << "/";

	if(main) {
		Vector<String> flags;
		if(HasFlag("DEBUG"))
			flags << "-D_DEBUG" << debug_options;
		else
			flags << release_options;
		if(HasFlag("DEBUG_MINIMAL"))
			flags << "-ggdb" << "-g1";
		if(HasFlag("DEBUG_FULL"))
			flags << "-ggdb" << "-g2";
		if(is_shared && !win32)
			flags << "-fPIC";
		flags.Append(GatherV(pkg.option, config.GetKeys()));

		CFLAGS = clone(flags);
		CXXFLAGS = pick(flags);

		CFLAGS << c_options;
		CXXFLAGS << cpp_options;
	}

//	String cmplr = Sys("which " + compiler);
//	cmplr.TrimEnd("\n");
	String cmplr = compiler;

	const String inc_outdir = SAppend("-I", makefile.outdir);
	for(int count = pkg.GetCount(), last = count - 1, i = 0; i < count; i++)
		if(!pkg[i].separator) {
			String gop = Gather(pkg[i].option, config.GetKeys());
			String fn = SourcePath(package, pkg[i]);
			String ext = ToLower(GetFileExt(fn));
			bool isc = ext == ".c";
			const bool ish = ext == ".h";
			const bool isrc = (ext == ".rc" && HasFlag("WIN32"));
			const bool iscpp = (ext == ".cpp" || ext == ".cc" || ext == ".cxx");
			const bool ishpp = (ext == ".hpp" || ext == ".hh" || ext == ".hxx");
			const bool isicpp = (ext == ".icpp");
			if(ext == ".brc") {
				isc = true;
				fn << "c";
			}
			if(isc || ish || isrc || iscpp || ishpp || isicpp) {
				String outfile;
				const String srcfile = SString(fn);
				outfile << makefile.outdir << AdjustMakePath(GetFileTitle(fn));
				if (isrc)
					outfile << "_rc";
				else if (ish || ishpp)
					outfile << "_hpp";
				else
					outfile << "_cpp";
				outfile << objext;
				outfile = SString(outfile);

#if 0
				Json j;
				JsonArray ja;
				ja << cmplr;
				ja << "-c";
				ja << "-x";
				if (isc)
					ja << "c" << CFLAGS;
				else
					ja << "c++" << CXXFLAGS;
				ja << CINC ;
				ja << inc_outdir;
				ja << macdef;
				// ja << gop; // ???
				ja << srcfile << "-o" << outfile;
				// j("arguments", ja)("directory", GetFileFolder(fn))("file", GetFileName(fn));
				// j("arguments", ja)("directory", GetFileFolder(fn))("file", fn);
				j("arguments", ja)("directory", SString(GetFileFolder(fn)))("file", srcfile);
				makefile.rules << tab << j;
				if (i == last && last_ws)
					makefile.rules << eol;
				else
					makefile.rules << "," << eol;
#else
				Json j;
				String ja;
				ja << cmplr;
				ja << " -c";
				ja << " -x";
				if (isc)
					ja << " c " << Join(CFLAGS, " ");
				else
					ja << " c++ " << Join(CXXFLAGS, " ");
				MergeWith(ja, " ", Join(CINC, " "));
				MergeWith(ja, " ", inc_outdir);
				MergeWith(ja, " ", Join(macdef, " "));
				ja << ' ' << srcfile << " -o" << outfile;
				j("command", ja)("directory", SString(GetFileFolder(fn)))("file", srcfile);
				makefile.rules << tab << j;
				if (i == last && last_ws)
					makefile.rules << eol;
				else
					makefile.rules << "," << eol;
#endif
			}
		}
}

void MakeBuild::SaveCCJ(const String& fn, bool exporting)
{
	const char* eol = "\n";
	const char* tab = "\t";

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

	makefile << "[" << eol;

	Index<String> oi;
	if(!exporting)
		for(int i = 0; i < uppdirs.GetCount(); i++) {
			String s;
			s << "-I" << uppdirs[i];
			oi.FindAdd(s);
		}

	for(const String& s: pkg_config) {
		String str;
		String out;
		str << "pkg-config --cflags " << s;
		if (Sys(str, out) < 0)
			continue;
		out.TrimEnd("\n");
		if (out.IsEmpty())
			continue;
		Vector<String> l = Split(out, ' ');
		for(const String& s1: l)
			oi.FindAdd(s1);
	}

	Vector<String> includes = SplitDirs(bm.Get("INCLUDE",""));
	for(int i = 0; i < includes.GetCount(); i++) {
		String str;
		str << SAppend("-I", includes[i]);
		oi.FindAdd(str);
	}

	for (const String& s: allconfig) {
		String str;
		str << "-Dflag" << s;
		b->Macro << str;
	}

	String UPPOUT("-I");
	if (exporting)
		UPPOUT << "_out/";
	else
		UPPOUT << GetMakePath(AdjustMakePath(AppendFileName(uppout, "")), win32);
	oi.FindAdd(pick(UPPOUT));

	b->CINC.Append(oi.PickKeys());

	String config, rules;

	for(int count = wspc.GetCount(), last = count - 1, i = 0; i < count; i++) {
		const String package = wspc[i];
		b->config = PackageConfig(wspc, i, bm, mainconfigparam, host, *b);
		b->version = tm.version;
		b->method = method;
		b->outdir = OutDir(b->config, package, bm);
		MakeFile mf;
		b->AddCCJ(mf, package, allconfig, exporting, i == last);
		config << mf.config;
		rules << mf.rules;
		if(i == 0) // main package
			b->SaveBuildInfo(package);
	}

	makefile
		<< config
		<< rules
		<< "]"
	;

	bool sv = ::SaveFile(fn, makefile);
	if(!exporting) {
		if(sv)
			PutConsole(NFormat("%s(1): compile_commands.json generation complete", fn));
		else
			PutConsole(NFormat("%s: error writing compile_commands.json", fn));
	}
}
