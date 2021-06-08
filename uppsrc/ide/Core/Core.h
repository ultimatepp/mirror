#ifndef COMMON_H
#define COMMON_H

#include <Esc/Esc.h>
#include <plugin/bz2/bz2.h>
#include <plugin/lz4/lz4.h>
#include <plugin/lzma/lzma.h>
#include <plugin/zstd/zstd.h>

#include "Logger.h"

using namespace Upp;

int CharFilterCid(int c);

int    ReadLNG(CParser& p);
String MakeLNG(int lang);

bool   OldLang();

String        PrintTime(int msecs);
inline String GetPrintTime(dword time0) { return PrintTime(msecs() - time0); }

bool   SaveChangedFile(const char *path, String data, bool delete_empty = false);

bool IsDoc(String s);

void CopyFile(const String& dst, const String& src, bool brc = false);
void CopyFolder(const char *_dst, const char *_src, Index<String>& used, bool all, bool brc = false);

class Workspace;

struct Ide;

namespace Upp {
class  Ctrl;
class  Image;
}

class IdeContext
{
public:
	virtual bool      IsVerbose() const = 0;
	virtual void      PutConsole(const char *s) = 0;
	virtual void      PutVerbose(const char *s) = 0;
	virtual void      PutLinking() = 0;
	virtual void      PutLinkingEnd(bool ok) = 0;

	virtual const Workspace& IdeWorkspace() const = 0;
	virtual bool             IdeIsBuilding() const = 0;
	virtual String           IdeGetOneFile() const = 0;
	virtual int              IdeConsoleExecute(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, bool noconvert = false) = 0;
	virtual int              IdeConsoleExecuteWithInput(const char *cmdline, Stream *out, const char *envptr, bool quiet, bool noconvert = false) = 0;
	virtual int              IdeConsoleExecute(One<AProcess> pick_ process, const char *cmdline, Stream *out = NULL, bool quiet = false) = 0;
	virtual int              IdeConsoleAllocSlot() = 0;
	virtual bool             IdeConsoleRun(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1) = 0;
	virtual bool             IdeConsoleRun(One<AProcess> pick_ process, const char *cmdline, Stream *out = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1) = 0;
	virtual void             IdeConsoleFlush() = 0;
	virtual void             IdeConsoleBeginGroup(String group) = 0;
	virtual void             IdeConsoleEndGroup() = 0;
	virtual bool             IdeConsoleWait() = 0;
	virtual bool             IdeConsoleWait(int slot) = 0;
	virtual void             IdeConsoleOnFinish(Event<>  cb) = 0;

	virtual bool      IdeIsDebug() const = 0;
	virtual void      IdeEndDebug() = 0;
	virtual void      IdeSetBottom(Ctrl& ctrl) = 0;
	virtual void      IdeActivateBottom() = 0;
	virtual void      IdeRemoveBottom(Ctrl& ctrl) = 0;
	virtual void      IdeSetRight(Ctrl& ctrl) = 0;
	virtual void      IdeRemoveRight(Ctrl& ctrl) = 0;

	virtual String    IdeGetFileName() const = 0;
	virtual int       IdeGetFileLine() = 0;
	virtual String    IdeGetLine(int i) const = 0;

	virtual void      IdeSetDebugPos(const String& fn, int line, const Image& img, int i) = 0;
	virtual void      IdeHidePtr() = 0;
	virtual bool      IdeDebugLock() = 0;
	virtual bool      IdeDebugUnLock() = 0;
	virtual bool      IdeIsDebugLock() const = 0;
	virtual void      IdeSetBar() = 0;
	virtual void      IdeGotoCodeRef(String link) = 0;
	virtual void      IdeOpenTopicFile(const String& file) = 0;
	virtual void      IdeFlushFile() = 0;
	virtual String    IdeGetFileName() = 0;
	virtual String    IdeGetNestFolder() = 0;
	virtual String    IdeGetIncludePath() = 0;

	virtual String                    GetDefaultMethod();
	virtual VectorMap<String, String> GetMethodVars(const String& method);
	virtual String                    GetMethodName(const String& method);

	virtual bool      IsPersistentFindReplace() = 0;

	virtual int       IdeGetHydraThreads() = 0;
	virtual String    IdeGetCurrentBuildMethod() = 0;
	virtual String    IdeGetCurrentMainPackage() = 0;
	virtual void      IdePutErrorLine(const String&) = 0;
	virtual void      IdeGotoFileAndId(const String& path, const String& id) = 0;

	virtual ~IdeContext() {}
};

IdeContext *TheIde();
void        TheIde(IdeContext *context);

bool      IsVerbose();
void      PutConsole(const char *s);
void      PutVerbose(const char *s);
void      PutLinking();
void      PutLinkingEnd(bool ok);

const Workspace& GetIdeWorkspace();
bool             IdeIsBuilding();
String           IdeGetOneFile();
int              IdeConsoleExecute(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, bool noconvert = false);
int              IdeConsoleExecuteWithInput(const char *cmdline, Stream *out, const char *envptr, bool quiet, bool noconvert = false);
int              IdeConsoleExecute(One<AProcess> process, const char *cmdline, Stream *out = NULL, bool quiet = false);
int              IdeConsoleAllocSlot();
bool             IdeConsoleRun(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1);
bool             IdeConsoleRun(One<AProcess> pick_ process, const char *cmdline, Stream *out = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1);
void             IdeConsoleFlush();
void             IdeConsoleBeginGroup(String group);
void             IdeConsoleEndGroup();
bool             IdeConsoleWait();
bool             IdeConsoleWait(int slot);
void             IdeConsoleOnFinish(Event<>  cb);
void             IdeGotoCodeRef(String s);

String GetDefaultMethod();
VectorMap<String, String> GetMethodVars(const String& method);
String GetMethodPath(const String& method);

bool      IdeIsDebug();
void      IdeEndDebug();
void      IdeSetBottom(Ctrl& ctrl);
void      IdeActivateBottom();
void      IdeRemoveBottom(Ctrl& ctrl);
void      IdeSetRight(Ctrl& ctrl);
void      IdeRemoveRight(Ctrl& ctrl);

String    IdeGetFileName();
int       IdeGetFileLine();
String    IdeGetLine(int i);

void      IdeSetDebugPos(const String& fn, int line, const Image& img, int i);
void      IdeHidePtr();
bool      IdeDebugLock();
bool      IdeDebugUnLock();
bool      IdeIsDebugLock();

void      IdeSetBar();

void      IdeGotoFileAndId(const String& path, const String& id);

int       IdeGetHydraThreads();
String    IdeGetCurrentBuildMethod();
String    IdeGetCurrentMainPackage();
void      IdePutErrorLine(const String& s);
void      IdeGotoFileAndId(const String& path, const String& id);

#include "Host.h"

struct IdeMacro {
	IdeMacro();

	int hotkey;
	String menu;
	String submenu;
	EscValue code;
};

ArrayMap<String, EscValue>& UscGlobal();
Array<IdeMacro>&            UscMacros();

void UscSetCleanModules(void (*CleanModules)());
void SetIdeModuleUsc(bool (*IdeModuleUsc)(CParser& p));
void UscSetReadMacro(void (*ReadMacro)(CParser& p));

void CleanUsc();
void ParseUscFile(const char *filename);

Point ReadNums(CParser& p);
Point ReadPoint(CParser& p);

Vector<String> SplitDirs(const char *s);

class Nest {
	VectorMap<String, String> var;
	VectorMap<String, String> package_cache;

	String PackagePath0(const String& name);

public:
	bool   Save(const char *path);
	bool   Load(const char *path);
	String Get(const String& id);
	void   Set(const String& id, const String& val);

	void   InvalidatePackageCache();
	String PackagePath(const String& name);
};

Nest& MainNest();

String DefaultHubFilePath();

void   SetHubDir(const String& path);
String GetHubDir();
bool   InUppHub(const String& p);

String VarFilePath();
String VarFilePath(String name);

bool   SaveVarFile(const char *filename, const VectorMap<String, String>& var);
bool   LoadVarFile(const char *name, VectorMap<String, String>& var);
bool   SaveVars(const char *name);
bool   LoadVars(const char *name);
String GetVar(const String& var);
String GetVarsName();
String VarFilePath();
Vector<String> GetUppDirsRaw();
Vector<String> GetUppDirs();
bool   IsHubDir(const String& path);
String GetUppDir();
void   SetVar(const String& var, const String& val, bool save = true);
void   SetMainNest(const String& n);
String GetAssemblyId();

String GetCurrentBuildMethod();
String GetCurrentMainPackage();

String GetAnyFileName(const char *path);
String GetAnyFileTitle(const char *path);
String CatAnyPath(String path, const char *more);

void   InvalidatePackageCache();
String PackagePath(const String& name);
String SourcePath(const String& package, const String& name);
inline
String PackageDirectory(const String& name) { return GetFileDirectory(PackagePath(name)); }
bool   IsNestReadOnly(const String& path);

String GetPackagePathNest(const String& path);

String GetLocalDir();
String LocalPath(const String& filename);

Vector<String> IgnoreList();

bool   IsFullDirectory(const String& d);
bool   IsFolder(const String& path);

bool   IsCSourceFile(const char *path);
bool   IsCHeaderFile(const char *path);

String FollowCygwinSymlink(const String& filename);

void   SplitPathMap(const char *path_map, Vector<String>& local, Vector<String>& remote);
String JoinPathMap(const Vector<String>& local, const Vector<String>& remote);
void   SplitHostName(const char *hostname, String& host, int& port);

Vector<String> SplitFlags0(const char *flags);
Vector<String> SplitFlags(const char *flags, bool main = false);
Vector<String> SplitFlags(const char *flags, bool main, const Vector<String>& accepts);

bool   MatchWhen(const String& when, const Vector<String>& flag);
String ReadWhen(CParser& p);
String AsStringWhen(const String& when);

struct OptItem {
	String   when;
	String   text;

	String ToString() const { return when + ": " + text ; }
};

struct CustomStep {
	String when;
	String ext;
	String command;
	String output;

	void   Load(CParser& p);
	String AsString() const;

	String GetExt() const;
	bool   MatchExt(const char *fn) const;
};

Vector<String> Combine(const Vector<String>& conf, const char *flags);
String Gather(const Array<OptItem>& set, const Vector<String>& conf, bool nospace = false);
Vector<String> GatherV(const Array<OptItem>& set, const Vector<String>& conf);

bool   HasFlag(const Vector<String>& conf, const char *flag);

enum {
	FLAG_MISMATCH = -2,
	FLAG_UNDEFINED = -1,
};

int    GetType(const Vector<String>& conf, const char *flags);
int    GetType(const Vector<String>& conf, const char *flags, int def);
bool   GetFlag(const Vector<String>& conf, const char *flag);
String RemoveType(Vector<String>& conf, const char *flags);

enum IdeCharsets {
	CHARSET_UTF8_BOM = 250, // same as TextCtrl::CHARSET_UTF8_BOM; CtrlLib not included here
	CHARSET_UTF16_LE,
	CHARSET_UTF16_BE,
	CHARSET_UTF16_LE_BOM,
	CHARSET_UTF16_BE_BOM
};

String ReadValue(CParser& p);

class Package {
	void Reset();
	void Option(bool& option, const char *name);

public:
	struct File : public String {
		Array<OptItem> option;
		Array<OptItem> depends;
		bool           readonly;
		bool           separator;
		int            tabsize;
		byte           charset;
		int            font;
		String         highlight;
		bool           pch, nopch, noblitz;
		int            spellcheck_comments;

		void operator=(const String& s)   { String::operator=(s); readonly = separator = false; }
		void Init()  { readonly = separator = false; tabsize = Null; charset = 0; font = 0;
		               pch = nopch = noblitz = false; spellcheck_comments = Null; }

		File()                            { Init(); }
		File(const String& s) : String(s) { Init(); }
		rval_default(File);
	};
	struct Config {
		String name;
		String param;
	};
	byte                     charset;
	int                      tabsize;
	bool                     noblitz;
	bool                     nowarnings;
	String                   description;
	Vector<String>           accepts;
	Array<OptItem>           flag;
	Array<OptItem>           uses;
	Array<OptItem>           target;
	Array<OptItem>           library;
	Array<OptItem>           static_library;
	Array<OptItem>           link;
	Array<OptItem>           option;
	Array<OptItem>           include;
	Array<OptItem>           pkg_config;
	Array<File>              file;
	Array<Config>            config;
	Array<CustomStep>        custom;
	Time                     time;
	bool                     bold, italic;
	Color                    ink;
	int                      spellcheck_comments;
	bool                     cr = false;

	int   GetCount() const                { return file.GetCount(); }
	File& operator[](int i)               { return file[i]; }
	const File& operator[](int i) const   { return file[i]; }

	bool  Load(const char *path);
	bool  Save(const char *file) const;

	static void SetPackageResolver(bool (*Resolve)(const String& error, const String& path, int line));

	Package();
};

String IdeCharsetName(byte charset);

class Workspace {
	void     AddUses(Package& p, bool match, const Vector<String>& flag);
	void     AddLoad(const String& name, bool match, const Vector<String>& flag);

public:
	ArrayMap<String, Package> package;

	void           Clear()                     { package.Clear(); }
	String         operator[](int i) const     { return package.GetKey(i); }
	Package&       GetPackage(int i)           { return package[i]; }
	const Package& GetPackage(int i) const     { return package[i]; }
	int            GetCount() const            { return package.GetCount(); }

	void           Scan(const char *prjname);
	void           Scan(const char *prjname, const Vector<String>& flag);

	Vector<String> GetAllAccepts(int pk) const;

	void     Dump();
};

struct Ide;

String FindInDirs(const Vector<String>& dir, const String& file);
String FindCommand(const Vector<String>& exedir, const String& cmdline);

struct MakeFile {
	String outdir;
	String outfile;
	String output;
	String config;
	String install;
	String rules;
	String linkdep;
	String linkfiles;
	String linkfileend;
};

String GetMakePath(String fn, bool win32);
String AdjustMakePath(const char *fn);

String Join(const String& a, const String& b, const char *sep = " ");

String GetExeExt();
String NormalizeExePath(String exePath);
String NormalizePathSeparator(String path);

struct Builder {
	Host            *host;
	Index<String>    config;
	String           method;

	String           compiler;
	String           outdir;
	Vector<String>   include;
	Vector<String>   libpath;
	String           target;
	String           cpp_options;
	String           c_options;
	String           debug_options;
	String           release_options;
	String           common_link;
	String           debug_link;
	String           release_link;
	String           version;

	String           script;
	String           mainpackage;

	bool             doall;
	bool             main_conf;
	bool             allow_pch;
	FileTime         start_time;

	Index<String>    pkg_config; // names of packages for pkg-config
	Vector<String>   CINC;
	Vector<String>   Macro;

	VectorMap<String, int> tmpfilei; // for naming automatic response files

	String                 CmdX(const char *s);

	virtual bool BuildPackage(const String& package, Vector<String>& linkfile, Vector<String>& immfile,
	    String& linkoptions, const Vector<String>& all_uses, const Vector<String>& all_libraries, int optimize)
	{ return false; }
	virtual bool Link(const Vector<String>& linkfile, const String& linkoptions, bool createmap)
	{ return false; }
	virtual bool Preprocess(const String& package, const String& file, const String& result, bool asmout)
	{ return false; }
	virtual void CleanPackage(const String& package, const String& outdir) {}
	virtual void AfterClean() {}
	virtual void AddFlags(Index<String>& cfg) {}
	virtual void AddMakeFile(MakeFile& mfinfo, String package,
		const Vector<String>& all_uses, const Vector<String>& all_libraries,
		const Index<String>& common_config, bool exporting) {}
	virtual void AddCCJ(MakeFile& mfinfo, String package,
		const Index<String>& common_config, bool exporting, bool last_ws) {}
	virtual String GetTargetExt() const = 0;
	virtual void SaveBuildInfo(const String& package) {}

	Builder()          { doall = false; main_conf = false; }
	virtual ~Builder() {}

	// TODO: move other methods if needed
	void                   ChDir(const String& path);
	String                 GetPathQ(const String& path) const;
	Vector<Host::FileInfo> GetFileInfo(const Vector<String>& path) const;
	Host::FileInfo         GetFileInfo(const String& path) const;
	Time                   GetFileTime(const String& path) const;
	int                    Execute(const char *cmdline);
	int                    Execute(const char *cl, Stream& out);
	bool                   HasFlag(const char *f) const { return config.Find(f) >= 0; }
};

VectorMap<String, Builder *(*)()>& BuilderMap();
void RegisterBuilder(const char *name, Builder *(*create)());

String                FindIncludeFile(const char *s, const String& filedir, const Vector<String>& incdir);

void                  HdependSetDirs(Vector<String> pick_ id);
void                  HdependTimeDirty();
void                  HdependClearDependencies();
void                  HdependAddDependency(const String& file, const String& depends);
Time                  HdependFileTime(const String& path);
Vector<String>        HdependGetDependencies(const String& file, bool bydefine_too = true);
String                FindIncludeFile(const char *s, const String& filedir);
bool                  HdependBlitzApproved(const String& path);
const Vector<String>& HdependGetDefines(const String& path);
const Vector<String>& HdependGetAllFiles();

bool IsHeaderExt(const String& ext);

class BinObjInfo {
public:
	BinObjInfo();

	void Parse(CParser& binscript, String base_dir);

	struct Block {
		Block() : index(-1), length(0), scriptline(-1), encoding(ENC_PLAIN), flags(0), offset(-1), len_meta_offset(-1) {}

		String ident;
		int    index;
		String file;
		int    length;
		int    scriptline;
		int    encoding;
		enum {
			ENC_PLAIN,
			ENC_ZIP,
			ENC_BZ2,
			ENC_LZ4,
			ENC_LZMA,
			ENC_ZSTD,
		};
		int    flags;
		enum {
			FLG_ARRAY = 0x01,
			FLG_MASK  = 0x02,
		};

		int    offset;
		int    off_meta_offset;
		int    len_meta_offset;

		void Compress(String& data);
	};

	VectorMap< String, ArrayMap<int, Block> > blocks;
};

void RegisterPCHFile(const String& pch_file);
void DeletePCHFiles();

String GetLineEndings(const String& data, const String& default_eol = "\r\n");

#endif