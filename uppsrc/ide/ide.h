#ifndef IDE_H
#define IDE_H

#include <ide/Common/Common.h>

#include <RichEdit/RichEdit.h>

#include <Report/Report.h>

#include <ide/Browser/Browser.h>
#include <TabBar/TabBar.h>
#include <CodeEditor/CodeEditor.h>
#include <ide/IconDes/IconDes.h>
#include <ide/Java/Java.h>
#include <ide/LayDes/LayDes.h>
#include <ide/Debuggers/Debuggers.h>
#include <TextDiffCtrl/TextDiffCtrl.h>
#include <ide/Designers/Designers.h>
#include <ide/Android/Android.h>
#include <plugin/md/Markdown.h>

#include "About.h"
#include "MethodsCtrls.h"

#define LAYOUTFILE <ide/ide.lay>
#include <CtrlCore/lay.h>

#define IMAGECLASS IdeImg
#define IMAGEFILE  <ide/ide.iml>
#include <Draw/iml_header.h>

#define KEYGROUPNAME "Ide"
#define KEYNAMESPACE IdeKeys
#define KEYFILE      <ide/ide.key>
#include             <CtrlLib/key_header.h>

#include "version.h"

#include <plugin/astyle/astyle.h>

#include <ide/Builders/Builders.h>

const char *FindTag(const char *txt, const char *tag);
const char *FindAfter(const char *txt, const char *tag);
int         IdeLocateLine(String old_file, int old_line, String new_file);

#include "UppDlg.h"

bool IsTextFile(const String& file, int maxline = INT_MAX);

void Puts(const char *s);

Display& BoldDisplay();

class Console : public CodeEditor {
public:
	virtual bool Key(dword key, int count);
	virtual void LeftDouble(Point p, dword);
	virtual void RightDown(Point p, dword);
	virtual void Append(const String& s);

protected:
	struct Slot {
		Slot() : outfile(NULL), quiet(true), exitcode(Null) { serial = INT_MAX; }

		One<AProcess>     process;
		String            cmdline;
		String            output;
		String            key;
		String            group;
		Stream            *outfile;
		bool              quiet;
		int               exitcode;
		int               last_msecs;
		int               serial;
	};

	struct Group {
		Group() : count(0), start_time(::msecs()), finished(false), msecs(0), raw_msecs(0) {}

		int               count;
		int               start_time;
		bool              finished;
		int               msecs;
		int               raw_msecs;
	};
	
	struct Finisher {
		int               serial;
		Event<>           cb;
	};

	Array<Slot> processes;
	ArrayMap<String, Group> groups;
	Array<Finisher> finisher;
	Vector<String> error_keys;
	String current_group;
	String spooled_output;
	int console_lock;
	bool wrap_text;
	FrameBottom<EditString> input;
	String line;
	int    serial;

	void CheckEndGroup();
	void FlushConsole();

public:
	Event<>  WhenSelect;
	Event<Bar&> WhenBar;
	Event<const String&> WhenLine;
	Event<>         WhenRunEnd;
	bool console;
	bool verbosebuild;

	int  Execute(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, bool noconvert = false);
	int  Execute(One<AProcess> pick_ process, const char *cmdline, Stream *out = NULL, bool quiet = false);
	int  GetSlotCount() const { return processes.GetCount(); }
	int  AllocSlot();
	bool Run(const char *cmdline, Stream *out = NULL, const char *endptr = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1);
	bool Run(One<AProcess> pick_ process, const char *cmdline, Stream *out = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1);
	void BeginGroup(String group);
	void EndGroup();

	Console& operator<<(const String& s)      { Append(s); return *this; }

	void ToErrors(const String& s);
	void AppendOutput(const String& s);
	bool IsRunning();
	bool IsRunning(int slot);
	int  Flush();
	void Kill(int slot);
	void Kill();
	void ClearError()                         { error_keys.Clear(); line.Clear(); WhenRunEnd(); }
	Vector<String> PickErrors()               { Vector<String> e = pick(error_keys); error_keys.Clear(); return pick(e); }
	void Wait(int slot);
	bool Wait();
	
	void OnFinish(Event<>  cb);

	void WrapText(bool w)                     { wrap_text = w; }

	void SetSlots(int s);

	void Input(bool b);

	Console();
};

VectorMap<String, Builder *(*)()>& BuilderMap();

String SelectAnyPackage();

String FindInDirs(const Vector<String>& dir, const String& file);
String FindCommand(const Vector<String>& exedir, const String& cmdline);

void DlCharset(DropList& d);
void DlCharsetD(DropList& d);

void                      SetupBuildMethods();
VectorMap<String, String> GetMethodVars(const String& method);
String                    GetDefaultMethod();

bool CheckLicense();
bool Install(bool& hasvars);
void AutoInstantSetup();

#define HELPNAME    "Help Topics"
#define METAPACKAGE "<meta>"

bool IsHelpName(const char *path);

class TopicCtrl : public HelpWindow {
public:
	virtual Topic AcquireTopic(const String& topic);
	virtual void  BarEx(Bar& bar);
	virtual bool  Key(dword key, int count);
	virtual void  FinishText(RichText& text);

private:
	EditString search;
	DropList   lang;
	bool       internal;
	bool       showwords, all;
	
	VectorMap<String, VectorMap<String, Index<String> > > map;
	Index<String> lang_list;
	Vector<int>   spos;
	
	static  Index<String> idelink;

	void OpenTopic();
	void Search();
	void ShowWords();
	void All();
	void Lang();
	void SShow();
	void ScanDirForTpp(const char *dir, const String& rel, Index<String>& donepackage,
	                   Index<String>& lang_list);
	void LoadMap();
	void FocusSearch();
	void Prev();
	void Next();

public:
	Event<>  WhenTopic;

	void SyncDocTree();
	void SearchWord(const String& s);
	void Serialize(Stream& s);

	typedef TopicCtrl CLASSNAME;

	TopicCtrl();
};

struct IdeCalc : CodeEditor {
	virtual bool Key(dword key, int count);
	virtual void LeftDouble(Point p, dword flags);

	ArrayMap<String, EscValue> vars;

	void    Execute();

	IdeCalc();
};

extern bool splash_screen;
void HideSplash();
void ShowSplash();
bool IsSplashOpen();

class RightTabs : public FrameCtrl<Ctrl> {
public:
	virtual void Paint(Draw& w);
	virtual void LeftDown(Point p, dword);
	virtual void MouseMove(Point p, dword);
	virtual void MouseLeave();
	virtual void CancelMode();
	virtual void FramePaint(Draw& w, const Rect& r);
	virtual void FrameLayout(Rect& r);
	virtual void FrameAddSize(Size& sz);

private:
	enum       { SPACE = 7 };

	struct Tab {
		int    y;
		Image  img;
		String tip;

		int    GetHeight() const { return img.GetSize().cy + SPACE; }
		int    GetRight() const  { return y + GetHeight(); }
	};

	Array<Tab> tab;
	int        cx;
	int        hl;
	int        cursor;

	void Repos();
	void PaintTab(Draw& w, int x, int y, int cx, int cy, Color paper, const Image& img, Color hl);
	int  GetPos(Point p);

public:
	void Clear();
	void Add(const Image& img, const String& tip);
	void SetCursor(int i);
	int  GetCursor() const                                       { return cursor; }

	RightTabs();
};

struct EditorTabBar : public FileTabs {
	EditorTabBar();

	String GetFile(int n) const;
	bool   FindSetFile(const String& fn);
	void   SetAddFile(const String& fn);
	void   RenameFile(const String& fn, const String& nn);
	void   SetSplitColor(const String& fn, const Color& c);
	void   ClearSplitColor();
	void   FixIcons();

	typedef EditorTabBar CLASSNAME;
};

int    memcmp_i(const char *s, const char *t, int n);

String ResolveTParam(const String& type, const Vector<String>& tparam);
void   ResolveTParam(Vector<String>& type, const Vector<String>& tparam);
String Qualify(const String& scope, const String& type, const String& usings);

int    CharFilterMacro(int c);
int    CharFilterFileName(int c);

bool   isincludefnchar(int c);

Size   GetDrawFileNameSize(const String& h);
void   DrawFileName(Draw& w, const Rect& r, const String& h, Color ink);

void   AddPath(EditString *es);
void   InsertPath(EditString *es);

void   DlSpellerLangs(DropList& dl);

#include "Assist.h"

void FileSelect(Ctrl& t, Button& b, Event<> ev, const char *types, bool saveas);
void FileSelectSaveAs(Ctrl& t, Button& b, Event<> ev, const char *types = NULL);
void FileSelectOpen(Ctrl& t, Button& b, Event<> ev, const char *types = NULL);
void FileSelectSaveAs(Ctrl& t, Button& b, const char *types = NULL);
void FileSelectOpen(Ctrl& t, Button& b, const char *types = NULL);
void DirSelect(Ctrl& t, Button& b);

void DirSel(EditField& f, FrameRight<Button>& b);
bool CopyFolder(const char *dst, const char *src, Progress *pi);
void RepoSyncDirs(const Vector<String>& working);

struct FindInFilesDlg : WithFindInFilesLayout<TopWindow> {
	WString itext;

	virtual bool Key(dword key, int count);
	
	void Setup(bool replace);
	void Sync();

	typedef FindInFilesDlg CLASSNAME;
	
	FindInFilesDlg();
};

struct WebSearchTab : WithSetupWebSearchTabLayout<ParentCtrl> {
	void Load();
	void Save();
	bool EditDlg(String& name, String& uri, String& zico);
	void Add();
	void Sync();
	void Edit();
	void Default();
	
	WebSearchTab();
};

void SearchEnginesDefaultSetup();
String SearchEnginesFile();

struct Ide : public TopWindow, public WorkspaceWork, public IdeContext, public MakeBuild {
public:
	virtual   void   Paint(Draw& w);
	virtual   bool   Key(dword key, int count);
	virtual   bool   HotKey(dword key);
	virtual   void   FileCursor();
	virtual   void   PackageCursor();
	virtual   void   SyncWorkspace();
	virtual   void   BuildFileMenu(Bar& menu);
	virtual   void   ProjectRepo(Bar& bar);
	virtual   void   FilePropertiesMenu0(Bar& menu);
	virtual   void   FilePropertiesMenu(Bar& menu);
	virtual   String GetOutputDir();
	virtual   String GetConfigDir();
	virtual   void   FileSelected();
	virtual   void   Serialize(Stream& s);
	virtual   void   FileRename(const String& nm);
	virtual   bool   FileRemove();
	virtual   void   DragAndDrop(Point p, PasteClip& d);
	virtual   void   DeactivateBy(Ctrl *new_focus);
	virtual   void   Activate();
	virtual   void   Layout();

	virtual   bool   IsVerbose() const;
	virtual   void   PutConsole(const char *s);
	virtual   void   PutVerbose(const char *s);
	virtual   void   PutLinking();
	virtual   void   PutLinkingEnd(bool ok);

	virtual   const Workspace& IdeWorkspace() const;
	virtual   bool             IdeIsBuilding() const;
	virtual   String           IdeGetOneFile() const;
	virtual   int              IdeConsoleExecute(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, bool noconvert = false);
	virtual   int              IdeConsoleExecute(One<AProcess> pick_ process, const char *cmdline, Stream *out = NULL, bool quiet = false);
	virtual   int              IdeConsoleExecuteWithInput(const char *cmdline, Stream *out, const char *envptr, bool quiet, bool noconvert);
	virtual   int              IdeConsoleAllocSlot();
	virtual   bool             IdeConsoleRun(const char *cmdline, Stream *out = NULL, const char *envptr = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1);
	virtual   bool             IdeConsoleRun(One<AProcess> pick_ process, const char *cmdline, Stream *out = NULL, bool quiet = false, int slot = 0, String key = Null, int blitz_count = 1);
	virtual   void             IdeConsoleFlush();
	virtual   void             IdeConsoleBeginGroup(String group);
	virtual   void             IdeConsoleEndGroup();
	virtual   bool             IdeConsoleWait();
	virtual   bool             IdeConsoleWait(int slot);
	virtual   void             IdeConsoleOnFinish(Event<>  cb);

	virtual   bool      IdeIsDebug() const;
	virtual   void      IdeEndDebug();
	virtual   void      IdeSetBottom(Ctrl& ctrl);
	virtual   void      IdeActivateBottom();
	virtual   void      IdeRemoveBottom(Ctrl& ctrl);
	virtual   void      IdeSetRight(Ctrl& ctrl);
	virtual   void      IdeRemoveRight(Ctrl& ctrl);

	virtual   String    IdeGetFileName() const;
	virtual   int       IdeGetFileLine();
	virtual   String    IdeGetLine(int i) const;

	virtual   void      IdeSetDebugPos(const String& fn, int line, const Image& img, int i);
	virtual   void      IdeHidePtr();
	virtual   bool      IdeDebugLock();
	virtual   bool      IdeDebugUnLock();
	virtual   bool      IdeIsDebugLock() const;

	virtual   void      IdeSetBar();
	virtual   void      IdeGotoCodeRef(String coderef);
	virtual   void      IdeOpenTopicFile(const String& file);
	virtual   void      IdeFlushFile();

	virtual   String    IdeGetFileName();
	virtual   String    IdeGetNestFolder();

	virtual   String    IdeGetIncludePath();

	virtual   bool      IsPersistentFindReplace();

	virtual   int       IdeGetHydraThreads();
	virtual   String    IdeGetCurrentBuildMethod();
	virtual   String    IdeGetCurrentMainPackage();
	virtual   void      IdePutErrorLine(const String& e);
	virtual   void      IdeGotoFileAndId(const String& path, const String& id);

	virtual void   ConsoleShow();
	virtual void   ConsoleSync();
	virtual void   ConsoleClear();
	virtual void   SetupDefaultMethod();
	virtual Vector<String> PickErrors();
	virtual void   BeginBuilding(bool clear_console);
	virtual void   EndBuilding(bool ok);
	virtual void   ClearErrorEditor();
	virtual void   DoProcessEvents();
	virtual void   ReQualifyCodeBase();
	virtual void   SetErrorEditor();
	virtual String GetMain();

	enum {
		EDITING, BUILDING, RUNNING, DEBUGGING,
	};

	String     pocfg;

	MenuBar    menubar;
	ToolBar    toolbar;
	StatusBar  statusbar;
	StaticRect editpane;

	int        idestate;
	int        debuglock;
	int        hydra1_threads;
	
	int        chstyle;

	One<IdeDesigner> designer;
	AssistEditor     editor;
	FileIn           view_file;
	AssistEditor     editor2; // no edits happen in editor2, just view
	FileIn           view_file2;
	EditorTabBar     tabs;
	EscValue         macro_api;

	RightTabs   btabs;
	StaticRect  bottom;
	Splitter    editor_bottom;
	Console     console;
	
	ArrayCtrl   ffound[3];
	int         ffoundi_next = 0;

	ArrayCtrl   error;
	int         error_count;
	int         warning_count;
	bool        addnotes;
	ValueArray  prenotes;
	bool        linking;
	Vector<String> linking_line;
	bool        removing_notes;

	IdeCalc     calc;
	Ptr<Ctrl>   bottomctrl;

	enum Bottoms { BCLOSE, BCONSOLE, BERRORS, BFINDINFILES1, BFINDINFILES2, BFINDINFILES3, BCALC, BDEBUG };

	FileOut    stdout_fout;

	Splitter      editorsplit;
	Splitter      pfsplit;
	StaticRect    wepane;
	SplitterFrame weframe;

	Splitter   right_split;
	StaticRect right;

	String    editfile;
	FileTime  edittime;
	int       editfile_line_endings;
	int       editfile_repo;
	bool      editfile_isfolder;
	String    editfile_includes;

	String    editfile2;

	Vector<String> tablru;
	int            tabi;
	bool           blocktabs;

	struct Bookmark : Moveable<Bookmark> {
		String            file;
		LineEdit::EditPos pos;

		void Serialize(Stream& s)        { s % file; s % pos; }
	}
	bookmark[10];

	struct FileData {
		Time               filetime;
		LineEdit::EditPos  editpos;
		Point              columnline;
		LineEdit::UndoData undodata;
		LineInfo           lineinfo;
		LineInfoRem        lineinforem;

		void Clear()  {
			filetime = Null; editpos.Clear(); undodata.Clear();
			columnline = Null; lineinfo.Clear(); lineinforem.Clear();
		}
		void ClearP() {
			lineinforem.Clear();
			undodata.Clear();
		}
	};

	String posfile[2];
	int    posline[2];
	Image  posimg[2];

	Vector<Bookmark> history;
	int              histi;

	ArrayMap<String, FileData> filedata;
	Index<String> editastext;
	Index<String> editashex;
	
	Vector<String> difflru;

	DropList   mainconfiglist;
	String     mainconfigname;

	int          build_time;

	MultiButton               buildmode;
	PopUpTable                methodlist;
	PopUpTable                modelist;
	VectorMap<String, String> recent_buildmode;

	Vector<String> recentoutput;
	Vector<String> recentflags;

	String    runarg;
	String    recent_runarg;
	String    rundir;
	enum      { RUN_WINDOW, RUN_CONSOLE, RUN_FILE, RUN_FILE_CONSOLE };
	int       runmode;
	bool      runexternal;
	bool      console_utf8;
	int       consolemode;
	bool      disable_uhd = false;
	bool      darkmode = false;
	String    stdout_file;
	String    recent_stdout_file;

// ------------------------------------
	enum LineEndingEnum { LF, CRLF, DETECT_LF, DETECT_CRLF };

	One<Debugger> debugger;

	Time      start_time;
	Time      build_start_time;
	int       stat_build_time;

	Font      editorfont;
	Font      veditorfont;
	Font      consolefont;
	Font      font1;
	Font      font2;
	Font      tfont;
	int       editortabsize;
	int       indent_amount;
	byte      default_charset;
	bool      indent_spaces;
	bool      show_status_bar;
	bool      toolbar_in_row;
	bool      show_tabs;
	bool      show_spaces;
	bool      warnwhitespace;
	int       line_endings;
	bool      tabs_icons;
	int       tabs_crosses;
	bool      tabs_grouping;
	bool      tabs_stacking;
	bool      tabs_serialize;
	bool      no_parenthesis_indent;
	bool      hilite_if_endif;
	bool      thousands_separator;
	bool      hline, vline;
	bool      wrap_console_text;
	bool      mute_sounds;
	bool      line_numbers;
	bool      header_guards;
	int       filetabs;
	bool      auto_enclose;
	bool      mark_lines;
	bool      find_pick_sel;
	bool      find_pick_text;
	bool      deactivate_save;
	int       insert_include;
	int       bordercolumn;
	Color     bordercolor;
	bool      persistent_find_replace;
	bool      find_replace_restore_pos;
	bool      auto_rescan;
	bool      auto_check;
	int       spellcheck_comments;
	bool      wordwrap_comments = true;
	bool      wordwrap = false;
	bool      setmain_newide;
	bool      gui_font_override = false;
	Font      gui_font = StdFont();
	/*
		astyle code formatter control vars
		added 2008.01.27 by Massimo Del Fedele
	*/
	bool	astyle_BracketIndent;
	bool	astyle_NamespaceIndent;
	bool	astyle_BlockIndent;
	bool	astyle_CaseIndent;
	bool	astyle_ClassIndent;
	bool	astyle_LabelIndent;
	bool	astyle_SwitchIndent;
	bool	astyle_PreprocessorIndent;
	int		astyle_MinInStatementIndentLength;
	int		astyle_MaxInStatementIndentLength;
	bool	astyle_BreakClosingHeaderBracketsMode;
	bool	astyle_BreakElseIfsMode;
	bool	astyle_BreakOneLineBlocksMode;
	bool	astyle_SingleStatementsMode;
	bool	astyle_BreakBlocksMode;
	bool	astyle_BreakClosingHeaderBlocksMode;
	int		astyle_BracketFormatMode;
	int		astyle_ParensPaddingMode;
	bool	astyle_ParensUnPaddingMode;
	bool	astyle_OperatorPaddingMode;
	bool	astyle_EmptyLineFill;
	bool	astyle_TabSpaceConversionMode;
	WString	astyle_TestBox;
	
	// Formats a string of code with a given formatter
	WString FormatCodeString(WString const &Src, astyle::ASFormatter &Formatter);

	// Formats editor's code with Ide format parameters
	void FormatCode();

	bool      browser_closeesc;
	bool      bookmark_pos;

	FrameTop<StaticBarArea> bararea;
	Label                   display;


	byte      hilite_scope;
	int       hilite_bracket;
	int       hilite_ifdef;
	bool      barline;
	bool      qtfsel;

	enum { STYLE_NO_REPLACE, STYLE_CONFIRM_REPLACE, STYLE_AUTO_REPLACE };

	String find_file_search_string;
	bool   find_file_search_in_current_package;

	FindInFilesDlg ff;
	String         iwc;

	int           doc_serial;
	TopicCtrl     doc;
	TopicCtrl     windoc;

	int           state_icon;
	
	String        export_dir;
	VectorMap<String, String> abbr;
	
	int           issaving;
	int           isscanning;
	
	String        current_builder;
	
	bool          hlstyle_is_default = true; // default style reacts to dark / light theme settings
	
// ------------------------------------

	Time      config_time;
	Time      ConfigTime();
	void      LoadConfig();
	void      SaveConfig();
	void      FinishConfig();
	void      SaveConfigOnTime();

	int       GetPackageIndex();
	void      AddPackage(const String& p);

	void      BeepMuteExclamation() { if(!mute_sounds) BeepExclamation(); }
	void      BeepMuteInformation() { if(!mute_sounds) BeepInformation(); }

	void      BookKey(int key);
	void      AddHistory();
	int       GetHistory(int d);
	void      History(int dir);
	void      EditorEdit();
	void      GotoBookmark(const Bookmark& b);
	bool      IsHistDiff(int i);
	
	void      IdePaste(String& s);

	bool      HasFileData(const String& file);
	FileData& Filedata(const String& file);

	void      BTabs();
	void      SyncBottom();
	void      HideBottom();
	void      SetBottom(int i);
	void      ToggleBottom(int i);
	void      ShowBottom(int i);
	void      ShowConsole()                      { ShowBottom(1); }
	void      ToggleConsole()                    { ToggleBottom(1); }
	void      SwapBottom();
	bool      IsBottomShown() const;
	bool      BottomIsFindInFiles() const        { return findarg(btabs.GetCursor(), BFINDINFILES1, BFINDINFILES2, BFINDINFILES3) >= 0; }

	void      EditorFontScroll(int d);

	void      MakeTitle();
	void      MakeIcon();
	void      AdjustMainConfig();
	String    GetFirstFile();
	void      SetMain(const String& package);
	void      ChangeFileCharset(const String& name, Package& p, byte charset);
	void      ChangeCharset();
	void      FlushFile();
	void      EditFile0(const String& path, byte charset, int spellcheck_comments,
	                    const String& headername = Null);
	void      EditFile(const String& path);
	void      AddEditFile(const String& path);
	void      ReloadFile();
	void      AddLru();
	bool      CanToggleReadOnly();
	void      ToggleReadOnly();
	void      PosSync();
	String    IncludesMD5();

	void      EditFileAssistSync2();
	void      EditFileAssistSync();
	
	TimeCallback     text_updated;
	std::atomic<int> file_scan;
	bool             file_scanned = false;

	void      TriggerAssistSync();

	void      AKEditor();
	
	void      PackageMenu(Bar& menu);

	void      UscFile(const String& file);
	void      UscProcessDir(const String& dir);
	void      UscProcessDirDeep(const String& dir);
	void      SyncUsc();
	void      CodeBaseSync();

	void      RefreshBrowser();

	void      SetupOutputMode();
	void      SyncBuildMode();
	void      SetupBuildMethods();
	void      DropMethodList();
	void      SetMethod(const String& m);
	void      SelectMethod();
	void      DropModeList();
	void      SelectMode();
	void      SerializeOutputMode(Stream& s);

	void      GotoPos(String path, int line);
	void      GotoCpp(const CppItem& pos);
	
	void      LoadAbbr();
	void      SaveAbbr();

	void      File(Bar& menu);
		void   EditWorkspace();
		void   EditAnyFile();
		bool   IsProjectFile(const String& f) const;
		void   SaveEditorFile(Stream& out);
		void   SaveFile0(bool always);
		void   SaveFile(bool always = false);
		void   DoSaveFile()    { SaveFile(); }
		void   FileBookmark	(Bar& menu);
		void   SwapPackagesFiles();
		void   Times();
		void   Statistics();
		void   Licenses();
		void   Exit();
		String GetOpposite();
		void   GoOpposite();
		void   Print();
		void   Diff();
		void   DiffWith(const String& path);
		void   DiffFiles(const char *lname, const String& l, const char *rname, const String& r);
		String LoadConflictFile(const String& n);
		void   GotoDiffLeft(int line, DiffDlg *df);
		void   GotoDiffRight(int line, FileDiff *df);

	void      Edit(Bar& menu);
		bool  IsDesignerFile(const String& path);
		void  DoEditAsText(const String& path);
		void  EditAsText();
		void  EditAsHex();
		void  EditUsingDesigner();
		void  EditMakeTabs()     { editor.MakeTabsOrSpaces(true); }
		void  EditMakeSpaces()   { editor.MakeTabsOrSpaces(false); }
		void  EditMakeLineEnds() { editor.MakeLineEnds(); }

		void  FindInFiles(bool replace);
		void  FindFileName();
		void  FindString(bool back);
		void  ClearEditedFile();
		void  ClearEditedAll();
		void  FindFileAll(const Vector<Tuple<int64, int>>& f);
		void  InsertColor();
		void  InsertLay(const String& fn);
		void  InsertIml(const Package& pkg, const String& fn, String classname);
		void  InsertText(const String& text);
		void  InsertAs(const String& data);
		void  InsertAs();
		void  InsertFilePath(bool c);
		void  InsertFileBase64();
		void  InsertMenu(Bar& bar);
		void  InsertInclude(Bar& bar);
		void  InsertAdvanced(Bar& bar);
		void  AssistEdit(Bar& menu);
		void  EditorMenu(Bar& bar);
		void  ToggleWordwrap();

        void  CopyPosition();
        void  GotoPosition();

	void OnlineSearchMenu(Bar& menu);

	void SearchMenu(Bar& bar);
		void  EditFind()                { editor.FindReplace(find_pick_sel, find_pick_text, false); }
		void  EditReplace()             { editor.FindReplace(find_pick_sel, find_pick_text, true); }
		void  EditFindReplacePickText() { editor.FindReplace(true, true, false); }
		void  EditFindNext()            { editor.FindNext(); }
		void  EditFindPrevious()        { editor.FindPrev(); }
		void  EditPaste()               { editor.Paste(); }
		bool  Next(ArrayCtrl& ctrl, int d);
		void  FindNextError();
		void  FindPrevError();
	
	void      EditSpecial(Bar& menu);
		void  TranslateString();
		void  SwapChars()               { editor.SwapChars(); }
		void  CopyWord()                { editor.CopyWord(); }
		void  DuplicateLine()           { editor.DuplicateLine(); }
		void  AlterText(WString (*op)(const WString& in));
		void  TextToUpper();
		void  TextToLower();
		void  TextToAscii();
		void  TextInitCaps();
		void  SwapCase();
		void  ToCString();
		void  ToComment();
		void  CommentLines();
		void  UnComment();
		void  ReformatComment();

	void      MacroMenu(Bar& menu);
		bool  HasMacros();
		void  EditMacroMenu(Bar& menu, ValueArray items);
		void  EditMacro(int i);

	void      Project(Bar& menu);
		void  SyncT(int kind);
		void  ConvertST();
		void  ExportMakefile(const String& ep);
		void  ExportProject(const String& ep, bool all, bool gui, bool deletedir = true);
		void  SyncRepoDirs(const Vector<String>& working);
		void  SyncRepoDir(const String& working);
		void  SyncRepo();

	void      BuildMenu(Bar& menu);
		void BuildPackageMenu(Bar& menu);

		void  DoBuild();
		void  PackageBuild();
		void  StopBuild();
		void  PackageClean();
		void  CreateMakefile();
		void  CleanUppOut();
		void  SwitchHeader();
		void  FileCompile();
		void  Preprocess(bool asmout);
		void  ToggleStopOnErrors();
		void  CreateHostRunDir(Host& h);
		void  PreprocessInternal();

	void      DebugMenu(Bar& menu);
		void  RunArgs();
		bool  ShouldHaveConsole();
		void  BuildAndExecute();
		void  ExecuteBinary();
		void  ExecuteApk();
		void  BuildAndDebug0(const String& srcfile);
		void  BuildAndDebug(bool runto);
		void  BuildAndExtDebug();
		void  BuildAndExtDebugFile();
		bool  IsValgrind();
		void  Valgrind();

		void  StartDebug();
		void  StopDebug();
		void  DebugToggleBreak();
		void  ConditionalBreak();
		void  DebugClearBreakpoints();
		void  OnBreakpoint(int i);

		bool  EditorTip(CodeEditor::MouseTip& mt);

	void      Setup(Bar& menu);
		void  SetupFormat();
		void  DoEditKeys();
		void  ToggleVerboseBuild();
		void  AutoSetup();
		void  CheckUpdates(bool verbose);
		void  CheckUpdatesManual();
		void  SetUpdateTimer(int period);
		void  Abbreviations();
		void  DoMacroManager();
		void  UpgradeTheIDE();
		void  InstallDesktop();
	
	void      SetupMobilePlatforms(Bar& bar);
		void  SetupAndroidMobilePlatform(Bar& bar, const AndroidSDK& androidSDK);
		void  LaunchAndroidSDKManager(const AndroidSDK& androidSDK);
		void  LaunchAndroidAVDManager(const AndroidSDK& androidSDK);
		void  LauchAndroidDeviceMonitor(const AndroidSDK& androidSDK);
	
	void      BrowseMenu(Bar& menu);
		void  CheckCodeBase();
		void  RescanCode();
		void  QueryId();
		void  OpenTopic(const String& topic, const String& createafter, bool before);
		void  OpenTopic(const String& topic);
		void  OpenATopic();
		void  ToggleNavigator();
		void  SearchCode();
		void  Goto();
		void  NavigatorDlg();
		void  ScanFile(bool check_includes);
		bool  SwapSIf(const char *cref);
		void  SwapS();
		void  FindId(const String& id);
		bool  OpenLink(const String& s, int pos);
		void  ContextGoto0(int pos);
		void  ContextGoto();
		void  GoToLine();
		void  CtrlClick(int64 pos);
		void  Qtf();
		void  Xml();
		void  Json();
		void  GotoDirDiffLeft(int line, DirDiffDlg *df);
		void  GotoDirDiffRight(int line, DirDiffDlg *df);
		void  DoDirDiff();
		void  DoPatchDiff();
		void  AsErrors();
		void  RemoveDs();

	void      HelpMenu(Bar& menu);
	    void  ViewIdeLogFile();
		void  About();

	void      ConsoleMenu(Bar& menu);
		void  ConsoleCopy();
		void  ConsolePaste();

	void      MainMenu(Bar& menu);
	void      MainTool(Bar& bar);

	void      SaveWorkspace();
	String    WorkspaceFile();
	void      SerializeWorkspace(Stream& s);

	void      SerializeLastMain(Stream& s);
	void      SaveLastMain();
	void      LoadLastMain();
	void      EditorMode();
	bool      IsEditorMode() const;

//	void      PrintTime(dword time);
	void      SetIdeState(int newstate);

	void      MainConfig();
	void      SyncMainConfigList();
	void      OnMainConfigList();
	void      SetMainConfigList();

	void      FileProperties();

	void      CustomSteps();

	void      CycleFiles();

	void      Renumber();
	
	String    GetTargetLogPath();
	String    GetIdeLogPath();
	void      OpenLog(const String& logFilePath);
	
	String    include_path; // cached value of include path, GetIncludePath
	
	virtual void      InvalidateIncludes();

	virtual void      LaunchTerminal(const char *dir);
	
//	Console&  GetConsole();

	struct FindLineErrorCache {
		VectorMap<String, String> file;
		Vector<String>            wspc_paths;
		String                    upp;
		bool                      is_java;
		bool                      init;
		
		void Clear()            { init = false; file.Clear(); wspc_paths.Clear(); }
	};

	struct ErrorInfo {
		String file;
		int    lineno;
		int    linepos;
		int    len;
		int    kind;
		String message;
		String error_pos;
		
		ErrorInfo() { lineno = linepos = kind = len = 0; }
	};
	
	FindLineErrorCache error_cache;
	void      ConsoleLine(const String& line, bool assist = false);
	void      ConsoleRunEnd();
	void      SyncErrorsMessage();
	String    GetErrorsText(bool all, bool src);
	void      CopyError(bool all);
	void      ErrorMenu(Bar& bar);
	void      ShowError();
	void      SetFFound(int ii);
	ArrayCtrl& FFound();
	void      ShowFound();
	void      CopyFound(bool all);
	void      FFoundMenu(Bar& bar);
	void      SelError();
	void      ClearErrorsPane();
	WString   FormatErrorLine(const String& text, int& linecy);
	WString   FormatErrorLineEP(const String& text, const char *ep, int& linecy);
	
	struct FoundDisplay : Display {
		virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const;
	};

	struct TopAlignedDisplay : Display {
		virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const;
	};

	struct FoundFileDisplay : Display {
		virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const;
		virtual Size GetStdSize(const Value& q) const;
	};
	
	bool      FindLineError(int l);
	void      GoToError(const ErrorInfo& f);
	void      GoToError(ArrayCtrl& a);
	
	bool      FindLineError(const String& ln, FindLineErrorCache& cache, ErrorInfo& f);
	void      FindError();
	void	  ClearErrorEditor(String file);

	void      FindWildcard();
	void      FindFolder();
	void      FindSetStdDir(String n);
	void      FindStdDir();
	void      InsertWildcard(const char *s);
	void      AddFoundFile(const String& fn, int ln, const String& line, int pos, int count);
	bool      SearchInFile(const String& fn, const String& pattern,
		                   bool wholeword, bool ignorecase, int& n, RegExp *regexp);
	void      SyncFindInFiles();
	void      ConstructFindInFiles();
	void      SerializeFindInFiles(Stream& s);

	void      SetupEditor(int font, String highlight, String fn);
	void      SetupEditor();

	void      DoDisplay();
	void      ManageDisplayVisibility();

	void      SetIcon();
	bool      IsCppBaseFile();
	void      CheckFileUpdate();
	void      Periodic();

	void      PassEditor();
	void      SyncEditorSplit();
	void      SplitEditor(bool horz);
	void      CloseSplit();
	void      KeySplit(bool horz);
	void      SwapEditors();
	void      TabFile();
	void      ClearTab();
	void      ClearTabs();
	void      CloseRest(EditorTabBar *tabs);
	void      TabsLR( int jd );
	void      TabsStackLR( int jd );

	void      RefreshFrame(bool auto_disasm);
	void      RefreshLine(int frame, bool auto_disasm);

	void      SetBar();
	void      SetMenuBar();
	void      SetToolBar();
	TimeCallback delayed_toolbar;
	
	
	void      UpdateFormat(CodeEditor& editor);
	void      UpdateFormat();
	void      ReadHlStyles(ArrayCtrl& hlstyle);

	bool      OpenMainPackage();
	void      NewMainPackage();

	bool      GotoDesignerFile(const String& path, const String& scope, const String& name, int line);
	void      JumpToDefinition(const Array<CppItem>& n, int q, const String& scope);
	void      GotoFileAndId(const String& path, const String& id);
	void      SearchTopics();
	void      ShowTopics();
	void      ShowTopicsWin();

	String    GetIncludePath();

	void      TopicBack();

	void      SetupBars();

	EscValue  MacroEditor();
		void  MacroGetLength(EscEscape& e);
		void  MacroGetLineCount(EscEscape& e);
		void  MacroGetLinePos(EscEscape& e);
		void  MacroGetLineLength(EscEscape& e);
		void  MacroGetCursor(EscEscape& e);
		void  MacroGetLine(EscEscape& e);
		void  MacroGetColumn(EscEscape& e);
		void  MacroGetSelBegin(EscEscape& e);
		void  MacroGetSelEnd(EscEscape& e);
		void  MacroGetSelCount(EscEscape& e);
		void  MacroSetCursor(EscEscape& e);
		void  MacroSetSelection(EscEscape& e);
		void  MacroClearSelection(EscEscape& e);
		void  MacroGet(EscEscape& e);
		void  MacroRemove(EscEscape& e);
		void  MacroInsert(EscEscape& e);
		void  MacroFind(EscEscape& e);
		void  MacroReplace(EscEscape& e);
		void  MacroFindMatchingBrace(EscEscape& e);
		void  MacroFindClosingBrace(EscEscape& e);
		void  MacroFindOpeningBrace(EscEscape& e);
		void  MacroMoveLeft(EscEscape& e);
		void  MacroMoveRight(EscEscape& e);
		void  MacroMoveWordLeft(EscEscape& e);
		void  MacroMoveWordRight(EscEscape& e);
		void  MacroMoveUp(EscEscape& e);
		void  MacroMoveDown(EscEscape& e);
		void  MacroMoveHome(EscEscape& e);
		void  MacroMoveEnd(EscEscape& e);
		void  MacroMovePageUp(EscEscape& e);
		void  MacroMovePageDown(EscEscape& e);
		void  MacroMoveTextBegin(EscEscape& e);
		void  MacroMoveTextEnd(EscEscape& e);
		void  MacroInput(EscEscape& e);
		void  MacroBuild(EscEscape& e);
		void  MacroBuildProject(EscEscape& e);
		void  MacroExecute(EscEscape& e);
		void  MacroLaunch(EscEscape& e);
		void  MacroClearConsole(EscEscape& e);
		void  MacroEditFile(EscEscape& e);
		void  MacroSaveCurrentFile(EscEscape& e);
		void  MacroFileName(EscEscape& e);
		void  MacroMainPackage(EscEscape& e);
		void  MacroActivePackage(EscEscape& e);
		void  MacroPackageDir(EscEscape& e);
		void  MacroAssembly(EscEscape& e);
		void  MacroBuildMethod(EscEscape& e);
		void  MacroBuildMode(EscEscape& e);
		void  MacroFlags(EscEscape& e);
		void  MacroEcho(EscEscape& e);
		void  MacroCloseFile(EscEscape& e);
		void  MacroPackageFiles(EscEscape& e);
		void  MacroAllPackages(EscEscape& e);
		void  MacroTarget(EscEscape& e);
	
	String GetAndroidSdkPath();
	
	typedef   Ide CLASSNAME;

	enum {
		TIMEID_PERIODIC = TopWindow::TIMEID_COUNT,
		TIMEID_COUNT,
	};

	Ide();
	~Ide();
};

inline void ShowConsole() { if(TheIde()) ((Ide *)TheIde())->ShowConsole(); }

void InstantSetup();

bool SetupSVNTrunk();

String UppHub();
void   UppHubAuto(const String& s);

#include "urepo.h"

#endif
