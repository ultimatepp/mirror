struct PackageMode : Moveable<PackageMode> {
	int debug;
	int blitz;

	void Serialize(Stream& s) { s / debug / blitz / blitz; }

	PackageMode() { debug = blitz = Null; }
};

struct TargetMode {
	bool        target_override;
	String      target;
	int         linkmode;
	bool        createmap;
	String      version;
	PackageMode def;
	VectorMap<String, PackageMode> package;

	void        Serialize(Stream& s);

	TargetMode() { def.debug = def.blitz = 0; target_override = false; linkmode = 0; createmap = false; }
};

String NoCr(const char *s);
int CharFilterSlash(int c);

String GetDefaultMethod();
VectorMap<String, String> GetMethodVars(const String& method);

class MakeBuild {
public:
	virtual void ConsoleShow() = 0; //		ShowConsole();	console.Sync();
	virtual void ConsoleSync() = 0; // console.Sync()
	virtual void ConsoleClear() = 0;
	virtual void SetupDefaultMethod() = 0;
	virtual Vector<String> PickErrors() = 0; //console.PickErrors()
	virtual void BeginBuilding(bool clear_console) = 0;
	virtual void EndBuilding(bool ok) = 0;
	virtual void ClearErrorEditor() = 0;
	virtual void DoProcessEvents() = 0;
	virtual void ReQualifyCodeBase() = 0;
	virtual void SetErrorEditor() = 0;
	virtual String GetMain() = 0;

	struct TransferFileInfo
	{
		String sourcepath;
		int    filetime;
		int    filesize;
	};

	ArrayMap<String, TransferFileInfo> transferfilecache;
	String       method;
	int          targetmode;
	TargetMode   debug;
	TargetMode   release;
	String       cmdout;
	String       target;
	String       onefile;
	bool         stoponerrors;
	bool         use_target;
	String       mainconfigparam;
	String       add_includes;
	String       main_conf;
	Index<String> cfg;
	FileTime     start_time;

	bool         makefile_svn_revision = true;

	void CreateHost(Host& host, bool darkmode = false, bool disable_uhd = false);

	const TargetMode& GetTargetMode();
	Index<String> PackageConfig(const Workspace& wspc, int package, const VectorMap<String, String>& bm,
	                            String mainparam, Host& host, Builder& b, String *target = NULL);
	One<Builder> CreateBuilder(Host *host);
	String OutDir(const Index<String>& cfg, const String& package,
	              const VectorMap<String, String>& bm, bool use_target = false);
	void PkgConfig(const Workspace& wspc, const Index<String>& config, Index<String>& pkg_config);
	bool BuildPackage(const Workspace& wspc, int pkindex, int pknumber, int pkcount,
	                  String mainparam, String outfile, Vector<String>& linkfile, Vector<String>& immfile,
	                  String& linkopt, bool link = false);
	void SetHdependDirs();
	Vector<String> GetAllUses(const Workspace& wspc, int f, const VectorMap<String, String>& bm,
	                          String mainparam, Host& host, Builder& builder);
	Vector<String> GetAllLibraries(const Workspace& wspc, int index,
	                               const VectorMap<String, String>& bm, String mainparam,
	                               Host& host, Builder& builder);
	bool Build(const Workspace& wspc, String mainparam, String outfile, bool clear_console = true);
	bool Build();
	void CleanPackage(const Workspace& wspc, int package);
	void Clean();
	void RebuildAll();
	void SaveMakeFile(const String& fn, bool exporting);
	void SaveCCJ(const String& fn, bool exporting);

	MakeBuild();

private:
	static String GetInvalidBuildMethodError(const String& method);
};

extern bool output_per_assembly;