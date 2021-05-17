#include "Builders.h"

#include "coff.h"

String MaxLenString(const byte *b, int maxlen)
{
	const byte *e = b + maxlen;
	while(e > b && e[-1] == 0)
		e--;
	return String(b, int(e - b));
}

String COFFSymbolName(const COFF_IMAGE_SYMBOL& sym, const char *strtbl)
{
	String name;
	if(sym.N.Name.Short)
		name = MaxLenString(sym.N.ShortName, 8);
	else
		name = strtbl + sym.N.Name.Long;
	return name;
}

#ifdef PLATFORM_WIN32
static bool HasTail(String s, const char *tail)
{
	int tl = (int)strlen(tail);
	int sl = s.GetLength();
	if(sl < tl)
		return false;
	for(const char *p = s.GetIter(sl - tl); *p; p++, tail++)
		if(*tail != '*' && *tail != *p)
			return false;
	return *tail == 0;
}
#endif

static void AddObjectExports(const char *path, Index<String>& out)
{
#ifdef PLATFORM_WIN32
	FileMapping mapping;
	if(!mapping.Open(path))
		return;
	const byte *begin = mapping.Begin();
	if(!begin)
		return;
	const COFF_IMAGE_FILE_HEADER *hdr = (const COFF_IMAGE_FILE_HEADER *)begin;
	if(hdr->Machine != COFF_IMAGE_FILE_MACHINE_I386)
		return;
	const COFF_IMAGE_SECTION_HEADER *sechdr = (const COFF_IMAGE_SECTION_HEADER *)(begin
		+ sizeof(COFF_IMAGE_FILE_HEADER) + hdr->SizeOfOptionalHeader);
	Index<int> code_sections;
	for(int i = 0; i < hdr->NumberOfSections; i++)
		if(sechdr[i].Characteristics & COFF_IMAGE_SCN_CNT_CODE)
			code_sections.Add(i + 1);
	const COFF_IMAGE_SYMBOL *symtbl = (const COFF_IMAGE_SYMBOL *)(begin + hdr->PointerToSymbolTable);
	const char *strtbl = (const char *)(symtbl + hdr->NumberOfSymbols);
	for(int i = 0; i < (int)hdr->NumberOfSymbols; i++)
	{
		const COFF_IMAGE_SYMBOL& sym = symtbl[i];
		if(sym.StorageClass == COFF_IMAGE_SYM_CLASS_EXTERNAL && code_sections.Find(sym.SectionNumber) >= 0)
		{
			String name = COFFSymbolName(sym, strtbl);
			if(!HasTail(name, "AEPAXI@Z"))
			{
				if(*name == '_' && name.Find('@') < 0)
					name.Remove(0, 1);
				out.FindAdd(name);
			}
		}
		i += sym.NumberOfAuxSymbols;
	}
#endif
}

void   MscBuilder::AddFlags(Index<String>& cfg)
{
	cfg.FindAdd("MSC");
}

String MscBuilder::CmdLine(const String& package, const Package& pkg)
{
	String cc;
	if(HasFlag("ARM"))
		cc = "clarm";
	else
	if(HasFlag("MIPS"))
		cc = "clmips";
	else
	if(HasFlag("SH3"))
		cc = "shcl /Qsh3";
	else
	if(HasFlag("SH4"))
		cc = "shcl /Qsh4";
	else
	if(HasFlag("MSC8ARM"))
		cc = "cl -GS- ";
	else
		cc = HasFlag("INTEL") ? "icl" : "cl";
// TRC 080605-documentation says Wp64 works in 32-bit compilation only
//	cc << (IsMsc64() ? " -nologo -Wp64 -W3 -GR -c" : " -nologo -W3 -GR -c");
	cc << " -nologo -W" << (pkg.nowarnings ? "0" : "3") << " -GR -c";
	cc << IncludesDefinesTargetTime(package, pkg);

	return cc;
}

String MscBuilder::MachineName() const
{
	if(HasFlag("ARM"))     return "ARM";
	if(HasFlag("MIPS"))    return "MIPS";
	if(HasFlag("SH3"))     return "SH3";
	if(HasFlag("SH4"))     return "SH4";
	if(IsMscArm())         return "ARM";
	if(IsMsc64())          return "x64";
	if(HasFlag("WIN32"))   return "I386";
	return "IX86";
}

bool MscBuilder::IsMsc89() const
{
	return IsMsc86() || IsMsc64() || IsMscArm();
}

bool MscBuilder::IsMsc86() const
{
	return HasFlag("MSC8") || HasFlag("MSC9") || HasFlag("MSC10") || HasFlag("MSC11")
		|| HasFlag("MSC12") || HasFlag("MSC15") || HasFlag("MSC14") || HasFlag("MSC17")
		|| HasFlag("MSC19");
}

bool MscBuilder::IsMscArm() const
{
	return HasFlag("MSC8ARM") || HasFlag("MSC9ARM");
}

bool MscBuilder::IsMsc64() const
{
	return HasFlag("MSC8X64") || HasFlag("MSC9X64") || HasFlag("MSC10X64") || HasFlag("MSC11X64")
		|| HasFlag("MSC12X64") || HasFlag("MSC14X64") || HasFlag("MSC15X64") || HasFlag("MSC17X64")
		|| HasFlag("MSC19X64");
}

String MscBuilder::LinkerName() const
{
	if(HasFlag("ULD")) return "uld";
	if(HasFlag("INTEL")) return "xilink";
	return "link";
}

static bool sContainsPchOptions(const String& x)
{
	Index<String> a(Split(x, ' '));
	return  a.Find("-GL") >= 0 || a.Find("/GL") >= 0 || a.Find("-Y-") >= 0 || a.Find("/Y-") >= 0
	     || a.Find("-Yc") >= 0 || a.Find("/Yc") >= 0 || a.Find("-Yd") >= 0 || a.Find("/Yd") >= 0
	     || a.Find("-Yl") >= 0 || a.Find("/Yl") >= 0 || a.Find("-Yu") >= 0 || a.Find("/Yu") >= 0
	     || a.Find("-YX") >= 0 || a.Find("/YX") >= 0;
}

String MscBuilder::Pdb(String package, int slot, bool separate_pdb) const
{
	String pdb_name = GetAnyFileName(package);
	if(separate_pdb)
		pdb_name << '-' << (slot + 1);
	return " -Gy -Fd" + GetPathQ(CatAnyPath(outdir, pdb_name + ".pdb"));
}

void DeletePCHFile(const String& pch_file)
{
	DeleteFile(pch_file);
	PutVerbose("Deleting precompiled header: " + pch_file);
}

bool MscBuilder::BuildPackage(const String& package, Vector<String>& linkfile, Vector<String>& immfile,
    String& linkoptions, const Vector<String>& all_uses, const Vector<String>& all_libraries, int opt)
{
	if(HasFlag("MAKE_MLIB") && !HasFlag("MAIN"))
		return true;
	
	SaveBuildInfo(package);

	int i;
	String packagepath = PackagePath(package);
	Package pkg;
	pkg.Load(packagepath);
	String packagedir = GetFileFolder(packagepath);
	ChDir(packagedir);
	PutVerbose("cd " + packagedir);
	IdeConsoleBeginGroup(package);
	Vector<String> obj;

	bool is_shared = HasFlag("SO"),
		is_clr = HasFlag("CLR");

	Vector<String> sfile, isfile;
	Vector<String> soptions, isoptions;
	Vector<String> sobjfile;
	bool           error = false;

	String pch_header;
	
	Index<String> nopch, noblitz;

	bool blitz = HasFlag("BLITZ");
	bool release = !HasFlag("DEBUG");

	for(i = 0; i < pkg.GetCount(); i++) {
		if(!IdeIsBuilding())
			return false;
		if(!pkg[i].separator) {
			String gop = Gather(pkg[i].option, config.GetKeys());
			Vector<String> srcfile = CustomStep(pkg[i], package, error);
			if(srcfile.GetCount() == 0)
				error = true;
			for(int j = 0; j < srcfile.GetCount(); j++) {
				String fn = srcfile[j];
				String ext = ToLower(GetFileExt(fn));
				if(findarg(ext, ".c", ".cpp", ".cc", ".cxx", ".rc", ".brc") >= 0 ||
				   (!release && blitz && ext == ".icpp")) {
					sfile.Add(fn);
					soptions.Add(gop);
				}
				else
				if(ext == ".icpp") {
					isfile.Add(fn);
					isoptions.Add(gop);
				}
				else
				if(ext == ".obj")
					obj.Add(fn);
				else
				if(ext == ".lib")
					linkfile.Add(fn);
				else
				if(IsHeaderExt(ext) && pkg[i].pch && allow_pch && IsMsc89() && release && !HasAnyDebug() && !blitz) {
					if(pch_header.GetCount())
						PutConsole(GetFileName(fn) + ": multiple PCHs are not allowed. Check your package configuration");
					else
						pch_header = fn;
				}
				if(pkg[i].nopch) {
					nopch.Add(fn);
					if(allow_pch && release)
						noblitz.Add(fn);
				}
				if(pkg[i].noblitz)
					noblitz.Add(fn);
				if(ext == ".c")
					nopch.Add(fn);
			}
		}
	}

	String cc = CmdLine(package, pkg);
	if(HasFlag("EVC")) {
		if(!HasFlag("SH3") && !HasFlag("SH4"))
			cc << " -Gs8192"; // disable stack checking
		cc << " -GF" // read-only string pooling
		      " -GX-"; // turn off exception handling
	}
	else
	if(is_clr)
		cc << " -EHac";
	else
	if(IsMsc89())
		cc << " -EHsc";
	else
		cc << " -GX";
//	String pdb = GetPathQ(CatAnyPath(outdir, GetAnyFileName(package) + ".pdb"));
//	String pch;
//	if(!HasFlag("MSC8")) // MSC8 does not support automatic precompiled headers...
//		pch << " -YX -Fp" << GetPathQ(CatAnyPath(outdir, GetAnyFileName(package) + ".pch")) << ' ';
//	cc << " -Gy -Fd" << pdb;
//	if(HasFlag("SSE2") && !IsMsc64())
//		cc << " /arch:SSE2";
	if(HasFlag("DEBUG_MINIMAL"))
		cc << " -Zd";
	if(HasFlag("DEBUG_FULL"))
		cc << " -Zi";
	cc << ' ' << Gather(pkg.option, config.GetKeys());
	cc << (HasFlag("SHARED") || is_shared || is_clr ? " -MD" : " -MT");

	String cc_size = cc;
	String cc_speed = cc;

	if(release)
		cc << ' ' << release_options;
	else
		cc << "d " << debug_options;
	
	int recompile = 0;
	Blitz b;
	if(blitz) {
		BlitzBuilderComponent bc(this);
		b = bc.MakeBlitzStep(sfile, soptions, obj, immfile, ".obj", noblitz, package);
		recompile = b.build;
	}

	for(i = 0; i < sfile.GetCount(); i++) {
		String fn = sfile[i];
		String ext = ToLower(GetFileExt(fn));
		if(findarg(ext, ".rc", ".brc", ".c") < 0 && HdependFileTime(sfile[i]) > GetFileTime(CatAnyPath(outdir, GetFileTitle(fn) + ".obj")))
			recompile++;
	}

	String pch_use;
	String pch_file;
	
	if(pch_header.GetCount()) {
		String pch_obj = CatAnyPath(outdir, GetFileTitle(pch_header) + "$pch.obj");
		pch_file = CatAnyPath(outdir, GetFileTitle(pch_header) + ".pch");
		RegisterPCHFile(pch_file);
		String pch_common = GetPathQ(pch_header) + " -Fp" + GetPathQ(pch_file) + " -FI" + GetPathQ(pch_header);
		
		if(blitz) // enable MK__s macros
			pch_common.Cat(" -DBLITZ_INDEX__=FPCH");

		if(recompile > 0 || !FileExists(pch_file)) {
			int pch_slot = AllocSlot();
			StringBuffer sb;
			sb << Join(cc, cpp_options) << Pdb(package, pch_slot, false) << " -Yc" << pch_common
			   << " -Tp " << GetPathQ(pch_header) << " -Fo" + GetPathQ(pch_obj);
			PutConsole("Precompiling header: " + GetFileName(pch_header));
			if(pch_slot < 0 || !Run(~sb, pch_slot, pch_obj, 1))
				error = true;
			Wait();
		}

		pch_use = " -Yu" + pch_common;
		obj.Add(pch_obj);
	}

	if(blitz && b.build) {
		PutConsole("BLITZ:" + b.info);
		int slot = AllocSlot();
		String c = Join(cc, cpp_options);
		if(HasAnyDebug())
			c << Pdb(package, slot, false);
		if(slot < 0 ||
		   !Run(c + " -Tp " + GetPathQ(b.path) + " -Fo" + GetPathQ(b.object),
		        slot, b.object, b.count))
			error = true;
	}

	int first_ifile = sfile.GetCount();
	sfile.AppendPick(pick(isfile));
	soptions.AppendPick(pick(isoptions));

	int ccount = 0;

	for(i = 0; i < sfile.GetCount(); i++) {
		if(!IdeIsBuilding())
			return false;
		String fn = sfile[i];
		String ext = ToLower(GetFileExt(fn));
		bool rc = (ext == ".rc");
		bool brc = (ext == ".brc");
		bool init = (i >= first_ifile);
		String objfile = CatAnyPath(outdir, SourceToObjName(package, fn) + (rc ? "$rc.obj" : brc ? "$brc.obj" : ".obj"));
		if(HdependFileTime(fn) > GetFileTime(objfile)) {
			int time = msecs();
			bool execerr = false;
			if(rc) {
				PutConsole(GetFileNamePos(fn));
				int slot = AllocSlot();
				if(slot < 0 || !Run("rc /fo" + GetPathQ(objfile) + Includes(" /i", package, pkg)
				    + DefinesTargetTime(" /d", package, pkg) + (HasFlag("DEBUG")?" /d_DEBUG":"")
					+ ' ' + GetPathQ(fn), slot, objfile, 1))
					execerr = true;
			}
			else
			if(brc) {
				try {
//					String hfn = GetHostPath(fn);
					String brcdata = LoadFile(fn);
					if(brcdata.IsVoid())
						throw Exc(Format("error reading file '%s'", fn));
					CParser parser(brcdata, fn);
					String fo = BrcToC(parser, GetFileDirectory(fn));
					String tmpfile = ForceExt(objfile, ".c");
					SaveFile(tmpfile, fo);
					int slot = AllocSlot();
					StringBuffer cmdline;
					cmdline << cc << Pdb(package, slot, false)
					        << " -Tc " << GetPathQ(tmpfile) << " -Fo" << GetPathQ(objfile);
					if(slot < 0 || !Run(String(cmdline), slot, objfile, 1))
						throw Exc(Format("Error compiling binary object '%s'.", objfile));
				}
				catch(Exc e) {
					PutConsole(e);
					execerr = true;
				}
			}
			else {
				String c = cc;
				int slot = AllocSlot();
				if(HasAnyDebug())
					c << Pdb(package, slot, !sContainsPchOptions(cc) && !sContainsPchOptions(soptions[i]));
				c << " " + soptions[i] + (ext == ".c" ? Join(c_options, " -Tc") : Join(cpp_options, " -Tp")) + ' '
				     + GetPathQ(fn) + " -Fo" + GetPathQ(objfile);
				if(nopch.Find(fn) < 0)
					c << pch_use;
				if(slot < 0 || !Run(c, slot, objfile, 1))
					execerr = true;
			}
			if(execerr)
				DeleteFile(objfile);
			error |= execerr;
			PutVerbose("compiled in " + GetPrintTime(time));
			ccount++;
		}
		if(init)
			linkfile.Add(objfile);
		else
			obj.Add(objfile);
		immfile.Add(objfile);
	}
	if(error) {
		IdeConsoleEndGroup();
		return false;
	}

	bool making_lib = HasFlag("MAKE_LIB") || HasFlag("MAKE_MLIB");

	if(!making_lib) {
		Vector<String> pkglibs = Split(Gather(pkg.library, config.GetKeys()), ' ');
		for(int i = 0; i < pkglibs.GetCount(); i++) {
			String libfile = AppendExt(pkglibs[i], ".lib");
			if(!IsFullPath(libfile)) {
				for(int p = 0; p < libpath.GetCount(); p++) {
					String nf = NormalizePath(libfile, libpath[p]);
					if(FileExists(nf)) {
						libfile = nf;
						break;
					}
				}
			}
			linkfile.Add(libfile);
		}
	}
	linkoptions << ' ' << Gather(pkg.link, config.GetKeys());
	
//	if(pch_file.GetCount())
//		OnFinish(callback1(DeletePCHFile, pch_file));

	if(!HasFlag("MAIN")) {
		if(HasFlag("BLITZ") || HasFlag("NOLIB") || making_lib) {
			linkfile.Append(obj);
//			ShowTime(ccount, time);
			IdeConsoleEndGroup();
			return true;
		}
		String product;
		if(is_shared)
			product = GetSharedLibPath(package);
		else
			product = CatAnyPath(outdir, GetAnyFileName(package) + ".lib");
		Time producttime = GetFileTime(product);
		if(obj.GetCount()) {
			String h = ForceExt(product, ".lib");
			linkfile.Add(h);
			immfile.Add(h);
		}
		if(!Wait()) {
			IdeConsoleEndGroup();
			return false;
		}
		Vector<Host::FileInfo> objinfo = host->GetFileInfo(obj);
		for(int i = 0; i < obj.GetCount(); i++)
			if(objinfo[i] > producttime)
				return CreateLib(product, obj, all_uses, all_libraries, Gather(pkg.link, config.GetKeys()));
		return true;
	}

	IdeConsoleEndGroup();
	obj.Append(linkfile);
	linkfile = pick(obj);
	return true;
}

bool MscBuilder::CreateLib(const String& product, const Vector<String>& obj,
                           const Vector<String>& all_uses, const Vector<String>& all_libraries,
                           const String& link_options)
{
	int linktime = msecs();
	bool isgemsc10 = HasFlag("MSC10") || HasFlag("MSC10X64")
	    || HasFlag("MSC11") || HasFlag("MSC11X64")
		|| HasFlag("MSC12") || HasFlag("MSC12X64")
		|| HasFlag("MSC14") || HasFlag("MSC14X64")
		|| HasFlag("MSC15") || HasFlag("MSC15X64")
		|| HasFlag("MSC17") || HasFlag("MSC17X64")
		|| HasFlag("MSC19") || HasFlag("MSC19X64")
	;
	bool is_shared = HasFlag("SO");
	String linker, lib;
	if(is_shared) {
		linker << LinkerName() << " -dll -nologo ";
		lib << "-machine:" << MachineName()
			<< " -pdb:" << GetPathQ(ForceExt(product, ".pdb"))
			<< " -out:" << GetPathQ(product);
		if(!isgemsc10)
			lib << " -incremental:no";
		if(HasAnyDebug())
			lib << " -debug -OPT:NOREF";
		else
			lib << " -release -OPT:REF,ICF";
		if(IsMscArm())
			lib <<  " -subsystem:windowsce,4.20 /ARMPADCODE";
		else
		if(HasFlag("GUI"))
			lib << (HasFlag("WIN32") ? " -subsystem:windows"
			                         : " -subsystem:windowsce");
		else
			lib << " -subsystem:console";
		Index<String> exports;
		for(int o = 0; o < obj.GetCount(); o++)
			AddObjectExports(obj[o], exports);
		String def;
		def << "LIBRARY " << AsCString(GetFileName(product)) << "\n\n"
			"EXPORTS\n";
		for(int o = 0; o < exports.GetCount(); o++)
			def << '\t' << exports[o] << "\n"; //" @" << (o + 1) << "\n";
		String deffile = ForceExt(product, ".def");
		if(!SaveChangedFile(deffile, def))
		{
			PutConsole(Format("%s: error saving file", deffile));
			return false;
		}
		lib << " -def:" << GetPathQ(deffile);
		for(int i = 0; i < libpath.GetCount(); i++)
			lib << " -LIBPATH:" << GetPathQ(libpath[i]);
		lib << ' ' << link_options;
		for(int i = 0; i < all_uses.GetCount(); i++)
			lib << ' ' << GetPathQ(ForceExt(GetSharedLibPath(all_uses[i]), ".lib"));
		for(int i = 0; i < all_libraries.GetCount(); i++) {
			String libfile = AppendExt(all_libraries[i], ".lib");
			if(!IsFullPath(libfile)) {
				for(int p = 0; p < libpath.GetCount(); p++) {
					String nf = NormalizePath(libfile, libpath[p]);
					if(FileExists(nf)) {
						libfile = nf;
						break;
					}
				}
			}
			lib << ' ' << GetPathQ(libfile);
		}
	}
	else{
		linker << (HasFlag("INTEL") ? "xilib" : "link /lib") << " -nologo ";
		lib << " -out:" << GetPathQ(product) << ' ' << link_options;
	}
	for(int i = 0; i < obj.GetCount(); i++)
		lib << ' ' << GetPathQ(obj[i]);
	PutConsole("Creating library...");
	IdeConsoleEndGroup();
	DeleteFile(product);
	String tmpFileName;
	if(linker.GetCount() + lib.GetCount() >= 8192)
	{
		tmpFileName = GetTempFileName();
		// we can't simply put all data on a single line
		// as it has a limit of around 130000 chars too, so we split
		// in multiple lines
		FileOut f(tmpFileName);
		while(lib != "")
		{
			int found = 0;
			bool quotes = false;
			int lim = min(8192, lib.GetCount());
			for(int i = 0; i < lim; i++)
			{
				char c = lib[i];
				if(isspace(c) && !quotes)
					found = i;
				else if(c == '"')
					quotes = !quotes;
			}
			if(!found)
				found = lib.GetCount();
			f.PutLine(lib.Left(found));
			lib.Remove(0, found);
		}
		f.Close();
		linker << "@" << tmpFileName;
	}
	else
		linker << lib;
	bool res = Execute(linker);
	if(tmpFileName != "")
		FileDelete(tmpFileName);
	if(res) {
		DeleteFile(product);
		return false;
	}
	else
	if((IsMsc86() || IsMsc64()) && is_shared) {
		String mt("mt -nologo -manifest ");
		mt << GetPathQ(product) << ".manifest -outputresource:" << GetPathQ(product) << ";2";
		Execute(mt);
	}
	PutConsole(String().Cat() << product << " (" << GetFileInfo(product).length
	           << " B) created in " << GetPrintTime(linktime));
	return true;
}

bool MscBuilder::Link(const Vector<String>& linkfile, const String& linkoptions, bool createmap)
{
	int time = msecs();
	if(!Wait())
		return false;

	PutLinking();

	if(HasFlag("MAKE_MLIB") || HasFlag("MAKE_LIB"))
		return CreateLib(ForceExt(target, ".lib"), linkfile, Vector<String>(), Vector<String>(), linkoptions);

	bool isgemsc10 = HasFlag("MSC10") || HasFlag("MSC10X64")
	    || HasFlag("MSC11") || HasFlag("MSC11X64")
	    || HasFlag("MSC12") || HasFlag("MSC12X64")
	    || HasFlag("MSC14") || HasFlag("MSC14X64")
		|| HasFlag("MSC15") || HasFlag("MSC15X64")
		|| HasFlag("MSC17") || HasFlag("MSC17X64")
		|| HasFlag("MSC19") || HasFlag("MSC19X64")
	;
	for(int i = 0; i < linkfile.GetCount(); i++)
		if(GetFileTime(linkfile[i]) > targettime) {
			String link, lib;
			link << LinkerName() << " -nologo -machine:" << MachineName()
			<< " -pdb:" << GetPathQ(ForceExt(target, ".pdb"))
			<< " -out:" << GetPathQ(target);
			if(!isgemsc10)
				if(HasAnyDebug())
					link << " -incremental:yes -debug -OPT:NOREF";
				else
					link << " -incremental:no -release -OPT:REF,ICF";
			else
				if(HasAnyDebug())
					link << " -debug -OPT:NOREF";
				else
					link << " -release -OPT:REF,ICF";
			if(IsMscArm())
				link << " -subsystem:windowsce,4.20 /ARMPADCODE -NODEFAULTLIB:\"oldnames.lib\" ";
			else
			if(HasFlag("GUI") || IsMscArm())
				link << " -subsystem:windows";
			else
				link << " -subsystem:console";
			if(!IsMsc64())
				link << ",5.01"; //,5.01 needed to support WindowsXP
			if(createmap)
				link << " -MAP";
			if(HasFlag("DLL"))
				link << " -DLL";

			for(i = 0; i < libpath.GetCount(); i++)
				link << " -LIBPATH:\"" << libpath[i] << '\"';
			link << ' ' << linkoptions << ' ';
			for(i = 0; i < linkfile.GetCount(); i++)
				lib << ' ' << GetPathQ(AppendExt(linkfile[i], ".lib"));
			PutConsole("Linking...");
			bool error = false;

			String tmpFileName;
			if(link.GetCount() + lib.GetCount() >= 8192)
			{
				tmpFileName = GetTempFileName();
				// we can't simply put all data on a single line
				// as it has a limit of around 130000 chars too, so we split
				// in multiple lines
				FileOut f(tmpFileName);
				while(lib != "")
				{
					int found = 0;
					bool quotes = false;
					int lim = min(8192, lib.GetCount());
					for(int i = 0; i < lim; i++)
					{
						char c = lib[i];
						if(isspace(c) && !quotes)
							found = i;
						else if(c == '"')
							quotes = !quotes;
					}
					if(!found)
						found = lib.GetCount();
					f.PutLine(lib.Left(found));
					lib.Remove(0, found);
				}
				f.Close();
				link << "@" << tmpFileName;
			}
			else
				link << lib;

			CustomStep(".pre-link", Null, error);
			if(!error && Execute(link) == 0) {
				CustomStep(".post-link", Null, error);
				if((IsMsc86() || IsMsc64()) && HasFlag("SHARED")) {
					String mt("mt -nologo -manifest ");
					mt << GetPathQ(target) << ".manifest -outputresource:" << GetPathQ(target)
				           << (HasFlag("DLL") ? ";2" : ";1");
				   Execute(mt);
				}
				PutConsole(String().Cat() << target << " (" << GetFileInfo(target).length
				           << " B) linked in " << GetPrintTime(time));
			}
			else {
				DeleteFile(target);
				error = true;
			}
			if(tmpFileName != "")
				FileDelete(tmpFileName);
			return !error;
		}
	PutConsole(String().Cat() << target << " (" << GetFileInfo(target).length
	           << " B) is up to date.");
	return true;
}

bool MscBuilder::Preprocess(const String& package, const String& file, const String& target, bool)
{
	FileOut out(target);
	String packagepath = PackagePath(package);
	Package pkg;
	pkg.Load(packagepath);
	return Execute(CmdLine(package, pkg) + " -E " + file, out);
}

Builder *CreateMscBuilder()
{
	return new MscBuilder;
}

INITIALIZER(MscBuilder)
{
	RegisterBuilder("MSC71", CreateMscBuilder);
	RegisterBuilder("MSC8", CreateMscBuilder);
	RegisterBuilder("MSC8X64", CreateMscBuilder);
	RegisterBuilder("MSC8ARM", CreateMscBuilder);
	RegisterBuilder("MSC9", CreateMscBuilder);
	RegisterBuilder("MSC9X64", CreateMscBuilder);
	RegisterBuilder("MSC9ARM", CreateMscBuilder);
	RegisterBuilder("MSC10", CreateMscBuilder);
	RegisterBuilder("MSC10X64", CreateMscBuilder);
	RegisterBuilder("MSC11", CreateMscBuilder);
	RegisterBuilder("MSC11X64", CreateMscBuilder);
	RegisterBuilder("MSC12", CreateMscBuilder);
	RegisterBuilder("MSC12X64", CreateMscBuilder);
	RegisterBuilder("MSC14", CreateMscBuilder);
	RegisterBuilder("MSC14X64", CreateMscBuilder);
	RegisterBuilder("MSC15", CreateMscBuilder);
	RegisterBuilder("MSC15X64", CreateMscBuilder);
	RegisterBuilder("MSC17", CreateMscBuilder);
	RegisterBuilder("MSC17X64", CreateMscBuilder);
	RegisterBuilder("MSC19", CreateMscBuilder);
	RegisterBuilder("MSC19X64", CreateMscBuilder);
	RegisterBuilder("EVC_ARM", CreateMscBuilder);
	RegisterBuilder("EVC_MIPS", CreateMscBuilder);
	RegisterBuilder("EVC_SH3", CreateMscBuilder);
	RegisterBuilder("EVC_SH4", CreateMscBuilder);
	RegisterBuilder("INTEL", CreateMscBuilder);
}
