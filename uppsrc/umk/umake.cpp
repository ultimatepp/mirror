#include "umake.h"

bool SilentMode;

String GetUmkFile(const char *fn)
{
	if(DirectoryExists(fn) || *fn == '.')
		return Null;
	String h = ConfigFile(fn);
	if(FileExists(h))
		return h;
	String cfgdir = GetFileFolder(GetFileFolder(ConfigFile("x")));
	ONCELOCK
		PutVerbose("Config directory: " << cfgdir);
	return GetFileOnPath(fn,
	                     cfgdir + "/umk" + ';' +
	                     cfgdir + "/theide" + ';' +
	                     cfgdir + "/ide" + ';' +
	                     GetHomeDirectory() + ';' +
	                     GetFileFolder(GetExeFilePath()));
}

String GetBuildMethodPath(String method)
{
	if(GetFileExt(method) != ".bm")
		method << ".bm";
	return GetUmkFile(method);
}

String Ide::GetDefaultMethod()
{
	return "GCC";
}

VectorMap<String, String> Ide::GetMethodVars(const String& method)
{
	VectorMap<String, String> map;
	LoadVarFile(GetMethodName(method), map);
	return map;
}

String Ide::GetMethodName(const String& method)
{
	return GetBuildMethodPath(method);
}

void Puts(const char *s)
{
	if(!SilentMode)
		Cout() << s;
}

String GetAndroidSDKPath()
{
	return String();
}

#ifdef flagMAIN

CONSOLE_APP_MAIN
{
#ifdef PLATFORM_POSIX
	setlinebuf(stdout);
	CreateBuildMethods();
#endif

	Ide ide;
	TheIde(&ide);
	ide.console.SetSlots(CPU_Cores());
	ide.console.console = true;

	ide.debug.def.blitz = ide.release.def.blitz = 0;
	ide.debug.def.debug = 2;
	ide.release.def.debug = 0;
	ide.debug.package.Clear();
	ide.release.package.Clear();
	ide.debug.linkmode = ide.release.linkmode = 0;
	ide.release.createmap = ide.debug.createmap = false;
	ide.targetmode = 0;
	ide.use_target = false;
	ide.makefile_svn_revision = false;
	bool clean = false;
	bool makefile = false;
	bool ccfile = false;
	bool deletedir = true;
	int  exporting = 0;
	bool run = false;
	String mkf;

	Vector<String> param, runargs;

	const Vector<String>& args = CommandLine();
	for(int i = 0; i < args.GetCount(); i++) {
		String a = args[i];
		if(*a == '-') {
			for(const char *s = ~a + 1; *s; s++)
				switch(*s) {
				case 'a': clean = true; break;
				case 'r': ide.targetmode = 1; break;
				case 'm': ide.release.createmap = ide.debug.createmap = true; break;
				case 'b': ide.release.def.blitz = ide.debug.def.blitz = 1; break;
				case 's': ide.debug.linkmode = ide.release.linkmode = 1; break;
				case 'd': ide.debug.def.debug = 0; break;
				case 'S': ide.debug.linkmode = ide.release.linkmode = 2; break;
				case 'v': ide.console.verbosebuild = true; break;
				case 'l': SilentMode = true; break;
				case 'x': exporting = 1; break;
				case 'X': exporting = 2; break;
				case 'k': deletedir = false; break;
				case 'u': ide.use_target = true; break;
				case 'j': ccfile = true; break;
				case 'M': {
					makefile = true;
					if(s[1] == '=') {
						mkf = NormalizePath(s + 2);
						PutVerbose("Generating Makefile: " + mkf);
						goto endopt;
					}
					else
						PutVerbose("Generating Makefile");
					break;
				}
				case 'H': {
					int n = 0;
					while(IsDigit(s[1])) {
						n = 10 * n + s[1] - '0';
						s++;
					}
					n = minmax(n, 1, 32);
					PutVerbose("Hydra threads: " + AsString(n));
					ide.console.SetSlots(n);
					break;
				}
				default:
					SilentMode = false;
					Puts("Invalid build option(s)");
					SetExitCode(3);
					return;
				}
		endopt:;
		}
		else
		if(*a == '+')
			ide.mainconfigparam = Filter(~a + 1, [](int c) { return c == ',' ? ' ' : c; });
		else
		if(*a == '!') {
			run = true;
			for(int j = i + 1; j < args.GetCount(); j++)
				runargs.Add(args[j]);
			if(runargs)
				PutVerbose("Set to execute the result with args: " << Join(runargs, " "));
			else
				PutVerbose("Set to execute the result");
			break;
		}
		else
			param.Add(a);
	}

	if(param.GetCount() >= 2) {
		String v = GetUmkFile(param[0] + ".var");
		if(IsNull(v)) {
		#ifdef PLATFORM_POSIX
			Vector<String> h = Split(param[0], [](int c) { return c == ':' || c == ',' ? c : 0; });
		#else
			Vector<String> h = Split(param[0], ',');
		#endif
			for(int i = 0; i < h.GetCount(); i++)
				h[i] = GetFullPath(TrimBoth(h[i]));
			String x = Join(h, ";");
			SetVar("UPP", x, false);
			PutVerbose("Inline assembly: " + x);
			String outdir = GetDefaultUppOut();
			RealizeDirectory(outdir);
			SetVar("OUTPUT", outdir, false);
		}
		else {
			if(!LoadVars(v)) {
				Puts("Invalid assembly\n");
				SetExitCode(2);
				return;
			}
			PutVerbose("Assembly file: " + v);
			PutVerbose("Assembly: " + GetVar("UPP"));
		}
		PutVerbose("Output directory: " + GetVar("OUTPUT"));
		v = SourcePath(param[1], GetFileTitle(param[1]) + ".upp");
		PutVerbose("Main package: " + v);
		if(!FileExists(v)) {
			Puts("Package does not exist\n");
			SetExitCode(2);
			return;
		}
		ide.main = param[1];
		ide.wspc.Scan(ide.main);
		const Workspace& wspc = ide.IdeWorkspace();
		if(!wspc.GetCount()) {
			Puts("Empty assembly\n");
			SetExitCode(4);
			return;
		}
		Index<String> missing;
		for(int i = 0; i < wspc.GetCount(); i++) {
			String p = wspc[i];
			if(!FileExists(PackagePath(p)))
				missing.FindAdd(p);
		}
		if(missing.GetCount()) {
			Puts("Missing package(s): " << Join(missing.GetKeys(), " ") << "\n");
			SetExitCode(5);
			return;
		}
		if(IsNull(ide.mainconfigparam)) {
			const Array<Package::Config>& f = wspc.GetPackage(0).config;
			if(f.GetCount())
				ide.mainconfigparam = f[0].param;
		}
		PutVerbose("Build flags: " << ide.mainconfigparam);
		String m = 2 < param.GetCount() ? param[2] : "CLANG";
		String bp = GetBuildMethodPath(m);
		PutVerbose("Build method: " + bp);
		if(bp.GetCount() == 0) {
			SilentMode = false;
			Puts("Invalid build method\n");
			SetExitCode(3);
			return;
		}

		if(3 < param.GetCount()) {
			ide.debug.target_override = ide.release.target_override = true;
			ide.debug.target = ide.release.target = NormalizePath(param[3]);
			PutVerbose("Target override: " << ide.debug.target);
		}

		ide.method = m;

		if(ccfile) {
			ide.SaveCCJ(GetFileDirectory(PackagePath(ide.main)) + "compile_commands.json", false);
			SetExitCode(0);
			return;
		}

		if(clean)
			ide.Clean();
		if(exporting) {
			mkf = GetFullPath(mkf);
			Cout() << mkf << '\n';
			RealizeDirectory(mkf);
			if(makefile)
				ide.ExportMakefile(mkf);
			else
				ide.ExportProject(mkf, exporting == 2, deletedir);
		}
		else
		if(makefile) {
			ide.SaveMakeFile(IsNull(mkf) ? "Makefile" : mkf, false);
			SetExitCode(0);
		}
		else
		if(ide.Build()) {
			SetExitCode(0);
			if(run) {
				Vector<char *> args;
				Vector<Buffer<char>> buffer;
				auto Add = [&](const String& s) {
					auto& b = buffer.Add();
					b.Alloc(s.GetCount() + 1);
					memcpy(b, s, s.GetCount() + 1);
					args.Add(b);
				};
				Add(ide.target);
				for(const String& s : runargs)
					Add(s);
				args.Add(NULL);
				SetExitCode(execv(ide.target, args.begin()));
			}
		}
		else
			SetExitCode(1);
	}
	else
		Puts("Usage: [-options] umk assembly main_package [build_method] [+flags] [output]\n\n"
		     "Examples: umk examples Bombs GCC -ab +GUI,SHARED ~/bombs\n"
		     "          umk examples,uppsrc Bombs ~/GCC.bm -rv +GUI,SHARED ~/bin\n\n"
		     "See https://www.ultimatepp.org/app$ide$umk$en-us.html for details\n");
}

#endif