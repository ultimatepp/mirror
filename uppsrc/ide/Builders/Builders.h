#ifndef BUILDERS_H
#define BUILDERS_H

#include <ide/Core/Core.h>
#include <plugin/bz2/bz2.h>

#include "Android.h"
#include "BuilderComponents.h"
#include "Build.h"

void PutCompileTime(int time, int count);

String BrcToC(CParser& binscript, String basedir);

Vector<String> SvnInfo(const String& package);

String MakeIdent(const char *name);

struct CppBuilder : Builder {
	virtual String GetTargetExt() const;
	virtual void   CleanPackage(const String& package, const String& outdir);

	const Workspace& wspc;
	Time             targettime;
	Vector<String>   CFLAGS;
	Vector<String>   CXXFLAGS;

	String                 GetSharedLibPath(const String& package) const;
	int                    AllocSlot();
	bool                   Run(const char *cmdline, int slot, String key, int blitz_count);
	bool                   Run(const char *cmdline, Stream& out, int slot, String key, int blitz_count);
	bool                   Wait();
	bool                   Wait(int slot);
	void                   OnFinish(Event<>  cb);
	bool                   Cp(const String& cmd, const String& package, bool& error);
	bool                   Cd(const String& cmd);
	Vector<String>         CustomStep(const String& path, const String& package, bool& error);

	String                 Includes(const char *sep, const String& package, const Package& pkg);
	void                   SaveBuildInfo(const String& package);
	String                 DefinesTargetTime(const char *sep, const String& package, const Package& pkg);
	String                 IncludesDefinesTargetTime(const String& package, const Package& pkg);
	bool                   HasAnyDebug() const;

	String                 GetMakePath(String fn) const;
	Point                  ExtractVersion() const;

	// POSIX lib files has names in form of libXXXXXX.so.ver.minver(.rel)
	// so we can't simply get file extension
	String                 GetSrcType(String fn) const;

	// from complete lib name/path (libXXX.so.ver.minVer) gets the soname (libXXX.so.ver)
	String                 GetSoname(String libName) const;

	// from complete lib name/path (libXXX.so.ver.minVer) gets the link name (libXXX.so)
	String                 GetSoLinkName(String libName) const;

	void                   ShowTime(int count, int start_time);

	virtual void           AddMakeFile(MakeFile& makefile, String package,
		const Vector<String>& all_uses, const Vector<String>& all_libraries,
		const Index<String>& common_config, bool exporting);

	virtual void AddCCJ(MakeFile& mfinfo, String package,
		const Index<String>& common_config, bool exporting, bool last_ws);

	CppBuilder() : wspc(GetIdeWorkspace()) {}
};

String SourceToObjName(const String& package, const String& srcfile_);

struct GccBuilder : CppBuilder {
	virtual void   AddFlags(Index<String>& cfg);
	virtual bool   BuildPackage(const String& package, Vector<String>& linkfile, Vector<String>& immfile,
	    String& linkoptions, const Vector<String>& all_uses, const Vector<String>& all_libraries, int optimize);
	virtual bool   Link(const Vector<String>& linkfile, const String& linkoptions, bool createmap);
	virtual bool   Preprocess(const String& package, const String& file, const String& target, bool asmout);

	String CompilerName() const;
	String CmdLine(const String& package, const Package& pkg);
	void BinaryToObject(String objfile, CParser& binscript, String basedir, const String& package, const Package& pkg);
	void   CocoaAppBundle();
	bool   CreateLib(const String& product, const Vector<String>& obj,
	                 const Vector<String>& all_uses, const Vector<String>& all_libraries,
	                 const String& link_options);

	String Info_plist; // apple bundle Info.plist
};

struct OwcBuilder : CppBuilder {
	typedef OwcBuilder CLASSNAME;

	virtual void   AddFlags(Index<String>& cfg);
	virtual bool   BuildPackage(const String& package, Vector<String>& linkfile, Vector<String>& immfile,
	    String& linkoptions, const Vector<String>& all_uses, const Vector<String>& all_libraries, int optimize);
	virtual bool   Link(const Vector<String>& linkfile, const String& linkoptions, bool createmap);
	virtual bool   Preprocess(const String& package, const String& file, const String& target, bool asmout);

	String         IncludesDefinesTargetTime(const String& package, const Package& pkg);

	String CompilerName(bool isCpp = true) const;
	String LinkerName() const;
	String CmdLine(const String& package, const Package& pkg);
	void   BinObjConsole(String c) { PutConsole(c); }
};

struct MscBuilder : CppBuilder {
	typedef MscBuilder CLASSNAME;

	virtual void   AddFlags(Index<String>& cfg);
	virtual bool   BuildPackage(const String& package, Vector<String>& linkfile, Vector<String>& immfile,
	    String& linkoptions, const Vector<String>& all_uses, const Vector<String>& all_libraries, int optimize);
	virtual bool   Link(const Vector<String>& linkfile, const String& linkoptions, bool createmap);
	virtual bool   Preprocess(const String& package, const String& file, const String& target, bool asmout);

	String CmdLine(const String& package, const Package& pkg);
	String MachineName() const;
	String LinkerName() const;
	String Pdb(String package, int slot, bool separate_pdb) const;
	void   BinObjConsole(String c) { PutConsole(c); }
	bool   IsMsc89() const;
	bool   IsMsc86() const;
	bool   IsMscArm() const;
	bool   IsMsc64() const;
	bool   CreateLib(const String& product, const Vector<String>& obj,
	                 const Vector<String>& all_uses, const Vector<String>& all_libraries,
	                 const String& link_options);

	void BinaryToObject(String objfile, CParser& binscript, String basedir, const String& package, const Package& pkg);
};

struct JavaBuilder : CppBuilder {
	virtual bool   BuildPackage(const String& package, Vector<String>& linkfile, Vector<String>&, String& linkoptions,
		const Vector<String>& all_uses, const Vector<String>& all_libraries, int optimize);
	virtual bool   Link(const Vector<String>& linkfile, const String& linkoptions, bool createmap);
	virtual bool   Preprocess(const String& package, const String& file, const String& target, bool asmout);

	bool           PreprocessJava(String file, String target, String options, String package, const Package& pkg);
	Time           AddClassDeep(String& link, String dir, String reldir);

	String         JavaLine();
	String         JarLine();
};

struct ScriptBuilder : CppBuilder {
public:
	typedef ScriptBuilder CLASSNAME;
	ScriptBuilder() : is_parsed(false), script_error(false) {}

	virtual bool   BuildPackage(const String& package, Vector<String>& linkfile, Vector<String>&, String& linkoptions,
		const Vector<String>& all_uses, const Vector<String>& all_libraries, int optimize);
	virtual bool   Link(const Vector<String>& linkfile, const String& linkoptions, bool createmap);
	virtual bool   Preprocess(const String& package, const String& file, const String& target, bool asmout);

	String         CmdLine();

private:
	void           CheckParse();

	EscValue       ExecuteIf(const char *fn, Vector<EscValue>& args);
	EscValue       ExecuteIf(const char *fn);
	EscValue       ExecuteIf(const char *fn, EscValue arg);
	EscValue       ExecuteIf(const char *fn, EscValue arg1, EscValue arg2);
	EscValue       ExecuteIf(const char *fn, EscValue arg1, EscValue arg2, EscValue arg3);

	void           ESC_Execute(EscEscape& e);
	void           ESC_PutConsole(EscEscape& e);
	void           ESC_PutVerbose(EscEscape& e);

private:
	ArrayMap<String, EscValue> globals;
	bool is_parsed;
	bool script_error;
};

void DeletePCHFile(const String& pch_file);

#ifdef PLATFORM_POSIX
void   CreateBuildMethods();
#endif

String GetDefaultUppOut();

INITIALIZE(GccBuilder)
INITIALIZE(MscBuilder)
INITIALIZE(JavaBuilder)
INITIALIZE(AndroidBuilder)
INITIALIZE(ScriptBuilder)

#endif