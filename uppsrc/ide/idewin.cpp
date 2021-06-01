#include "ide.h"
#include "ide.h"

#define IMAGECLASS IdeImg
#define IMAGEFILE  <ide/ide.iml>
#include <Draw/iml_source.h>

void Ide::ToggleVerboseBuild() {
	console.verbosebuild = !console.verbosebuild;
	
	SetToolBar();
}

void Ide::ToggleStopOnErrors() {
	stoponerrors = !stoponerrors;
}

void Ide::SwapPackagesFiles()
{
	weframe.Show(weframe.IsShown() ? false : true);
}

void Ide::ConsoleClear()
{
	console <<= Null;
}

void Ide::ConsoleCopy()
{
	console.Copy();
}

void Ide::ConsolePaste()
{
	String s = ReadClipboardText();
	if(!IsNull(s)) {
		s.Insert(0, '\n');
		int len = console.GetLength();
		console.Insert(len, s.ToWString());
		console.SetCursor(len + 1);
	}
}

bool Ide::IsVerbose() const
{
	return console.verbosebuild;
}

void Ide::PutConsole(const char *s)
{
	console << s << "\n";
}

void Ide::PutVerbose(const char *s)
{
	LOG("VERBOSE: " << s);
	if(console.verbosebuild) {
		PutConsole(s);
		console.Sync();
	}
}

bool Ide::IdeIsBuilding() const
{
	return idestate == Ide::BUILDING;
}

String Ide::IdeGetOneFile() const
{
	return onefile;
}

int Ide::IdeConsoleExecute(const char *cmdline, Stream *out, const char *envptr, bool quiet, bool noconvert)
{
	return console.Execute(cmdline, out, envptr, quiet, noconvert);
}

int Ide::IdeConsoleExecuteWithInput(const char *cmdline, Stream *out, const char *envptr, bool quiet, bool noconvert)
{
	ShowConsole();
	console.Input(true);
	int r = console.Execute(cmdline, out, envptr, quiet, noconvert);
	console.Input(false);
	return r;
}

int Ide::IdeConsoleExecute(One<AProcess> pick_ process, const char *cmdline, Stream *out, bool quiet)
{
	return console.Execute(pick(process), cmdline, out, quiet);
}

int Ide::IdeConsoleAllocSlot()
{
	return console.AllocSlot();
}

bool Ide::IdeConsoleRun(const char *cmdline, Stream *out, const char *envptr, bool quiet, int slot, String key, int blitz_count)
{
	return console.Run(cmdline, out, envptr, quiet, slot, key, blitz_count);
}

bool Ide::IdeConsoleRun(One<AProcess> pick_ process, const char *cmdline, Stream *out, bool quiet, int slot, String key, int blitz_count)
{
	return console.Run(pick(process), cmdline, out, quiet, slot, key, blitz_count);
}

void Ide::IdeConsoleFlush()
{
	console.Flush();
}

void Ide::IdeConsoleBeginGroup(String group)
{
	console.BeginGroup(group);
}

void Ide::IdeConsoleEndGroup()
{
	console.EndGroup();
}

bool Ide::IdeConsoleWait()
{
	return console.Wait();
}

bool Ide::IdeConsoleWait(int slot)
{
	console.Wait(slot);
	return true;
}

void Ide::IdeConsoleOnFinish(Event<>  cb)
{
	console.OnFinish(cb);
}

void Ide::IdeSetRight(Ctrl& ctrl)
{
	right.Add(ctrl.SizePos());
	right_split.Zoom(-1);
}

void Ide::IdeRemoveRight(Ctrl& ctrl)
{
	ctrl.Remove();
	if(!right.GetFirstChild())
		right_split.Zoom(0);
}

bool Ide::IdeIsDebug() const
{
	return debugger;
}

int Ide::IdeGetHydraThreads()
{
	return hydra1_threads;
}

String Ide::IdeGetCurrentBuildMethod()
{
	return method;
}

String Ide::IdeGetCurrentMainPackage()
{
	return main;
}

void Ide::IdePutErrorLine(const String& line)
{
	if(console.verbosebuild) {
		SetBottom(Ide::BERRORS);
		ConsoleRunEnd();
		ConsoleLine(line, true);
	}
}

void Ide::IdeGotoFileAndId(const String& path, const String& id)
{
	GotoFileAndId(path, id);
}

void Ide::IdeEndDebug()
{
	debugger.Clear();
	debuglock = 0;
	MakeTitle();
	editor.EnableBreakpointing();
	SetBar();
	editor.HidePtr();
	for(int i = 0; i < 2; i++)
		posfile[i].Clear();
	PosSync();
	const Workspace& wspc = IdeWorkspace();
	for(int i = 0; i < wspc.GetCount(); i++) {
		const Package& pk = wspc.GetPackage(i);
		String n = wspc[i];
		for(int i = 0; i < pk.file.GetCount(); i++) {
			String file = SourcePath(n, pk.file[i]);
			ValidateBreakpoints(Filedata(file).lineinfo);
		}
	}
	editor.ValidateBreakpoints();
}

void Ide::IdeSetDebugPos(const String& file, int line, const Image& img, int i)
{
	posfile[i] = file;
	posline[i] = line;
	posimg[i] = img;
	EditFile(file);
	editor.GotoLine(line);
	PosSync();
	Size sz = editor.GetPageSize();
	Point p = editor.GetScrollPos();
	int l = editor.GetCursorLine();
	if(sz.cy > 8 && l - p.y > sz.cy - 4)
		editor.SetScrollPos(Point(p.x, max(0, l - 4)));
	SetForeground();
	editor.SetFocus();
}

void Ide::IdeHidePtr()
{
	editor.HidePtr();
}

bool Ide::IdeDebugLock()
{
	if(debuglock == 0) {
		debuglock = 1;
		MakeTitle();
		SetBar();
		Sync();
		return true;
	}
	debuglock++;
	return false;
}

bool Ide::IdeDebugUnLock()
{
	debuglock--;
	ASSERT(debuglock >= 0);
	if(debuglock == 0) {
		MakeTitle();
		SetBar();
		return true;
	}
	return false;
}

bool Ide::IdeIsDebugLock() const
{
	return debuglock;
}

String Ide::IdeGetFileName() const
{
	return !designer ? editfile : Null;
}

int Ide::IdeGetFileLine()
{
	return !designer ? editor.GetNoLine(editor.GetCursorLine()) : 0;
}

String Ide::IdeGetLine(int i) const
{
	if(i >= 0 && i < editor.GetLineCount())
		return editor.GetUtf8Line(i);
	else
		return Null;
}

void Ide::IdeSetBar()
{
	SetBar();
}

void Ide::SetupBars()
{
	ClearFrames();
	
	int r = HorzLayoutZoom(170);
#ifdef PLATFORM_COCOA
	AddFrame(toolbar);
	display.RightPos(4, r).VSizePos(2, 3);
#else
	int l = HorzLayoutZoom(350);
	
	menubar.Transparent();
	if(toolbar_in_row) {
		toolbar.SetFrame(NullFrame());
		int tcy = max(mainconfiglist.GetStdSize().cy + DPI(2), toolbar.GetStdHeight());
		bararea.Add(menubar.LeftPos(0, l).VCenterPos(menubar.GetStdHeight()));
		bararea.Add(toolbar.HSizePos(l, r).VCenterPos(tcy));
		bararea.Add(display.RightPos(4, r).VSizePos(2, 3));
		bararea.Height(max(menubar.GetStdHeight(), tcy));
		AddFrame(bararea);
		toolbar.Transparent();
	}
	else {
		bararea.Add(menubar.LeftPos(0, l).VCenterPos(menubar.GetStdHeight()));
		bararea.Add(display.RightPos(4, r).VSizePos(2, 3));
		bararea.Height(menubar.GetStdHeight());
		AddFrame(bararea);
		AddFrame(TopSeparatorFrame());
		AddFrame(toolbar);
		toolbar.NoTransparent();
	}
#endif
	AddFrame(statusbar);
	SetBar();
}

void SetupError(ArrayCtrl& error, const char *s)
{
	error.AddColumn("File").SetDisplay(Single<Ide::FoundFileDisplay>());
	error.AddColumn("Line").SetDisplay(Single<Ide::TopAlignedDisplay>());
	error.AddColumn(s);
	error.AddIndex("INFO");
	error.ColumnWidths("184 44 298");
	error.NoWantFocus();
}

void Ide::Layout()
{
	display.Show(!designer && (menubar.GetSize().cx + display.GetSize().cx < GetSize().cx));
}

static void sHighlightLine(const String& path, Vector<LineEdit::Highlight>& hln, const WString& ln)
{
	One<EditorSyntax> es = EditorSyntax::Create(EditorSyntax::GetSyntaxForFilename(GetFileName(path)));
	es->IgnoreErrors();
	HighlightOutput hl(hln);
	es->Highlight(ln.Begin(), ln.End(), hl, NULL, 0, 0);
}

Ide::Ide()
{
	DiffDlg::WhenHighlight = callback(sHighlightLine);

	editor.theide = this;
	editor.WhenSel << [=] {
		delayed_toolbar.KillSet(150, [=] { SetToolBar(); });
	};
	
	editormode = false;
	
	start_time = GetSysTime();
	stat_build_time = 0;
	build_start_time = Null;
	hydra1_threads = CPU_Cores();
	
	chstyle = 0;

	Sizeable().Zoomable();

	display.SetAlign(ALIGN_RIGHT);

	filelist.Columns(2);
	package.Columns(2);

	filetabs = AlignedFrame::TOP;
	auto_enclose = false;
	mark_lines = true;
	
	persistent_find_replace = false;

	idestate = EDITING;
	debuglock = 0;

	menubar.WhenHelp = ~statusbar;
	menubar.AreaLook(1);
	toolbar.WhenHelp = ~statusbar;
	toolbar.AreaLook(1);
	toolbar_in_row = false;
	WhenClose = THISBACK(Exit);

	editorsplit.Vert(editor, editor2);
	editorsplit.Zoom(0);
	SyncEditorSplit();
	
	editpane.AddFrame(editor.navigatorframe);

	right_split.Horz(editpane, right);
	right_split.Zoom(0);
	
	SetupError(error, "Message");
	error.AddIndex("NOTES");
	error.ColumnWidths("207 41 834");
	error.WhenBar = THISBACK(ErrorMenu);

	for(int i = 0; i < 3; i++) {
		SetupError(ffound[i], "Source");
		ffound[i].ColumnWidths("207 41 834");
		ffound[i].ColumnAt(0).SetDisplay(Single<FoundFileDisplay>());
		ffound[i].ColumnAt(2).SetDisplay(Single<FoundDisplay>());
		ffound[i].WhenBar = THISBACK(FFoundMenu);
		ffound[i].WhenSel = ffound[i].WhenLeftClick = THISBACK(ShowFound);
	}

	error.WhenSel = THISBACK(SelError);
	error.WhenLeftClick = THISBACK(ShowError);
	console.WhenLine = THISBACK1(ConsoleLine, false);
	console.WhenRunEnd = THISBACK(ConsoleRunEnd);
	
	addnotes = false;
	removing_notes = false;

	editor_bottom.Vert(right_split, bottom);
	console.WhenBar = THISBACK(ConsoleMenu);
	editor_bottom.SetPos(8000);
	bottom.SetFrame(btabs);
	bottom.Add(console.SizePos().SetFrame(NullFrame()));
	bottom.Add(error.SizePos().SetFrame(NullFrame()));
	bottom.Add(calc.SizePos().SetFrame(NullFrame()));
	for(int i = 0; i < 3; i++)
		bottom.Add(ffound[i].SizePos().SetFrame(NullFrame()));
	btabs <<= THISBACK(SyncBottom);
	BTabs();
	
	editor.WhenSelectionChanged << [=] {
		editor2.Illuminate(editor.GetIlluminated());
	};

	pfsplit.SetPos(2000);
	pfsplit.Vert(package, filelist);
	wepane.Add(editor_bottom.SizePos());
	wepane.AddFrame(weframe.Left(pfsplit, HorzLayoutZoom(280)));
	Add(wepane.SizePos());

	editor.topsbbutton.ScrollStyle().NoWantFocus().Show();
	editor.topsbbutton1.ScrollStyle().NoWantFocus().Show();
	tabs <<= THISBACK(TabFile);
//	tabs.WhenCloseRest = THISBACK1(CloseRest, &tabs);
//	editor2.SetFrame(NullFrame());
	editor2.theide = this;
	editor2.topsbbutton.ScrollStyle().NoWantFocus().Show();
	editor2.topsbbutton1.ScrollStyle().NoWantFocus().Show();
	editor2.WhenLeftDown = THISBACK(SwapEditors);
	editor.WhenAction = THISBACK(EditorEdit);
	editor.WhenBar = THISBACK(EditorMenu);
	editor.WhenFontScroll = THISBACK(EditorFontScroll);
	editor.WhenOpenFindReplace = THISBACK(AddHistory);
	editor.WhenPaste = THISBACK(IdePaste);
	
	editor.WhenFindAll << THISFN(FindFileAll);

	macro_api = MacroEditor();

	mainconfiglist.WhenClick = THISBACK(MainConfig);
	mainconfiglist <<= THISBACK(OnMainConfigList);
	mainconfiglist.NoDropFocus();
	mainconfiglist.NoWantFocus();

	buildmode.WhenClick = THISBACK(SetupOutputMode);
	buildmode.NoWantFocus();
	buildmode.Tip("Output mode");
	buildmode.AddButton().Tip("Build method").Left() <<= THISBACK(DropMethodList);
	buildmode.AddButton().Tip("Build mode") <<= THISBACK(DropModeList);
	methodlist.Normal();
	methodlist.WhenSelect = THISBACK(SelectMethod);
	modelist.Normal();
	modelist.WhenSelect = THISBACK(SelectMode);
	modelist.Add("Debug");
	modelist.Add("Release");

	tabi = 0;
	blocktabs = false;

	package.WhenBar = THISBACK(PackageMenu);
	editor.WhenState = THISBACK(MakeTitle);

	package.NoWantFocus();
	filelist.NoWantFocus();
	filelist.WhenLeftDouble = THISBACK(Group);

	Icon(IdeImg::Package(), IdeImg::Package());

	tfont = editorfont = font2 = veditorfont = CourierZ(12);
	consolefont = font1 = CourierZ(10);
	editortabsize = 4;
	indent_amount = 4;
	indent_spaces = false;
	show_status_bar = false;
	show_tabs = false;
	show_spaces = false;
	warnwhitespace = true;
	tabs_icons = false;
	tabs_crosses = AlignedFrame::RIGHT;
	tabs_grouping = true;
	tabs_stacking = false;
	tabs_serialize = true;
	no_parenthesis_indent = false;
#ifdef PLATFORM_POSIX
	line_endings = DETECT_LF;
#else
	line_endings = DETECT_CRLF;
#endif
	spellcheck_comments = LNG_ENGLISH;
	wordwrap_comments = true;
#ifdef PLATFORM_COCOA
	setmain_newide = true;
#else
	setmain_newide = false;
#endif
	/*
		astyle code formatter control vars
		added 2008.01.27 by Massimo Del Fedele
	*/
	astyle_BracketIndent = false;
	astyle_NamespaceIndent = true;
	astyle_BlockIndent = false;
	astyle_CaseIndent = true;
	astyle_ClassIndent = true;
	astyle_LabelIndent = true;
	astyle_SwitchIndent = true;
	astyle_PreprocessorIndent = false;
	astyle_MinInStatementIndentLength = 2;
	astyle_MaxInStatementIndentLength = 20;
	astyle_BreakClosingHeaderBracketsMode = true;
	astyle_BreakElseIfsMode = true;
	astyle_BreakOneLineBlocksMode = true;
	astyle_SingleStatementsMode = true;
	astyle_BreakBlocksMode = true;
	astyle_BreakClosingHeaderBlocksMode = true;
	astyle_BracketFormatMode = astyle::BREAK_MODE;
	astyle_ParensPaddingMode = astyle::PAD_BOTH;
	astyle_ParensUnPaddingMode = true;
	astyle_OperatorPaddingMode = true;
	astyle_EmptyLineFill = false;
	astyle_TabSpaceConversionMode = false;
	astyle_TestBox = "#include <stdio.h>\n#ifndef __abcd_h\n#include <abcd.h>\n#endif\n\nvoid test(int a, int b)\n{\n  /* this is a switch */\n  switch(a)\n\n  {\n    case 1:\n      b = 2;\n      break;\n    case 2:\n      b = 4;\n      break;\n    default:\n    break;\n  }\n\n  /* this are more statements on one line */\n  a = 2*a;b=-5;a=2*(b+2)*(a+3)/4;\n\n  /* single line blocks */\n  {int z;z = 2*a+b;}\n\n  /* loop */\n  for(int i = 0;i< 10;i++) { a = b+2*i;}\n\n}\n";
	
	console.WhenSelect = THISBACK(FindError);
	console.SetSlots(hydra1_threads);

	editor.WhenSelection = THISBACK(DoDisplay);
	stoponerrors = true;
	hilite_scope = 1;
	hilite_bracket = 1;
	hilite_ifdef = 1;
	barline = true;
	qtfsel = true;
	hilite_if_endif = false;
	thousands_separator = true;
	hline = vline = true;
	wrap_console_text = true;
	mute_sounds = false;
	line_numbers = false;

	use_target = true;

	runmode = RUN_WINDOW;
	runexternal = false;
	consolemode = 0;
	console_utf8 = false;

	browser_closeesc = true;

	bookmark_pos = true;

	header_guards = true;
	insert_include = 1;

	SetTimeCallback(-20, THISBACK(Periodic), TIMEID_PERIODIC);

	editor.WhenBreakpoint = THISBACK(OnBreakpoint);

	ConstructFindInFiles();

	ff.style <<= STYLE_NO_REPLACE;

	default_charset = CHARSET_UTF8;

	TheIde(this);

	targetmode = 0;

	doc.WhenTopic = THISBACK(OpenATopic);

	editor.NoCutLine();

	bordercolumn = 96;
	bordercolor = SColorFace();

	state_icon = -1;

	histi = 0;

	doc_serial = -1;

	showtime = true;
	
	editor.WhenTip = THISBACK(EditorTip);
	editor.WhenCtrlClick = THISBACK(CtrlClick);
	
	find_pick_sel = true;
	find_pick_text = false;
	
	deactivate_save = true;
	
	output_per_assembly = true;
	
	issaving = 0;
	isscanning = 0;
	
	linking = false;
	
	error_count = 0;
	warning_count = 0;
	
	editor.WhenUpdate = THISBACK(TriggerAssistSync);

	editfile_isfolder = false;
	editfile_repo = NOT_REPO_DIR;
	
	auto_rescan = auto_check = true;
	file_scan = 0;

	editfile_line_endings = Null;

	HideBottom();
	SetupBars();
	SetBar();

#ifdef PLATFORM_COCOA
	WhenDockMenu = [=](Bar& bar) {
		bar.Add("Open main package..", [=] {
			Host h;
			CreateHost(h, false, false);
			h.Launch(GetExeFilePath() + " --nosplash");
		});
	};
#endif
}

Ide::~Ide()
{
	TheIde(NULL);
}

void Ide::Paint(Draw&) {}
