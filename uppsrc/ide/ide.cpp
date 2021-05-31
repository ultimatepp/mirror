#include "ide.h"

void Ide::MakeTitle()
{
	String title;
	if(!main.IsEmpty())
		title << main;
	if(!mainconfigname.IsEmpty() &&  mainconfigname == mainconfigparam)
		title << " - " << mainconfigname;
	else
	if(!mainconfigname.IsEmpty()) {
		title << " - " << mainconfigname;
		title << " ( " << mainconfigparam << " )";
	}
	if(!title.IsEmpty())
		title << " - ";
	title << "TheIDE";
	if(designer) {
		title << " - " << designer->GetFileName();
		int cs = designer->GetCharset();
		if(cs >= 0)
			title << " " << CharsetName(cs);
	}
	else
	if(!editfile.IsEmpty()) {
		title << " - " << editfile;
		int chrset = editor.GetCharset();
		title << " " << IdeCharsetName(chrset)
		      << " " << (findarg(Nvl(editfile_line_endings, line_endings), LF, DETECT_LF) >= 0 ? "LF" : "CRLF");
		if(editor.IsTruncated())
			title << " [Truncated]";
		if(editor.IsView())
			title << " [View]";
		else
		if(editor.IsReadOnly())
			title << " [Read Only]";
		if(editor.IsDirty())
			title << " *";
	}
	if(!IsNull(editfile))
		for(int i = 0; i < 10; i++)
			if(NormalizePath(editfile) == NormalizePath(bookmark[i].file))
				title << Format(" <%d>", i);
	title << " { " << GetAssemblyId() << " }";
	if(isscanning)
		title << " (scanning files)";
	Title(title.ToWString());
}

bool Ide::CanToggleReadOnly()
{
	return NormalizePath(GetActiveFilePath()) == NormalizePath(editfile) && !editor.IsView();
}

void Ide::ToggleReadOnly()
{
	if(CanToggleReadOnly() && IsActiveFile()) {
#ifdef PLATFORM_WIN32
		FindFile ff(editfile);
		if(ff && ff.IsReadOnly()) {
			dword attrib = GetFileAttributes(editfile);
			attrib &= ~FILE_ATTRIBUTE_READONLY;
			SetFileAttributes(editfile, attrib);
		}
#endif
		editor.SetEditable(editor.IsReadOnly());
		ActiveFile().readonly = editor.IsReadOnly();
		SavePackage();
		MakeTitle();
		SetBar();
	}
}

void Ide::AdjustMainConfig()
{
	const Workspace& wspc = IdeWorkspace();
	if(!wspc.GetCount())
		return;
	const Array<Package::Config>& f = wspc.GetPackage(0).config;
	for(int i = 0; i < f.GetCount(); i++)
		if(f[i].param == mainconfigparam)
			return;
	if(f.GetCount()) {
		mainconfigparam = f[0].param;
		mainconfigname = f[0].name;
		SetMainConfigList();
	}
}

String Ide::GetFirstFile()
{
	const Workspace& wspc = IdeWorkspace();
	for(int i = 0; i < wspc.GetCount(); i++) {
		const Package& p = wspc.GetPackage(i);
		for(int j = 0; j < p.GetCount(); j++)
			if(!p[j].separator)
				return SourcePath(wspc[i], p[j]);
	}
	return Null;
}

void Ide::SetMain(const String& package)
{
	FlushFile();
	SaveWorkspace();
	transferfilecache.Clear();
	main = package;
	export_dir = GetHomeDirFile(main);
	history.Clear();
	mainconfigname.Clear();
	mainconfigparam.Clear();

	UppHubAuto(main);

	ScanWorkspace();
	editfile.Clear();
	LoadFromFile(THISBACK(SerializeWorkspace), WorkspaceFile());
	tabs.FixIcons();
	editorsplit.Zoom(0);
	String e = editfile;
	UpdateFormat();
	editfile.Clear();
	MakeTitle();
	MakeIcon();
	SyncMainConfigList();
	AdjustMainConfig();
	SyncBuildMode();
	SetHdependDirs();
	SetBar();
	HideBottom();
	SyncUsc();
	InvalidateIncludes();
	if(auto_check)
		NewCodeBase();
	if(IsNull(e))
		e = GetFirstFile();
	EditFile(e);
}

void Ide::Exit()
{
	if(debugger)
		debugger->Stop();
	SaveFile();
	SaveWorkspace();
	FlushFile();
	console.Kill();
	Break(IDOK);
	IdeExit = true;
}

bool Ide::OpenMainPackage()
{
	String version = SplashCtrl::GenerateVersionNumber();
	String tt = "Select main package";
#ifdef bmYEAR
	tt << " (TheIDE " << version
	   << Format(" %d-%02d-%02d %d:%02d)", bmYEAR , bmMONTH, bmDAY, bmHOUR, bmMINUTE);
#else
	tt << " (TheIDE " << version << ')';
#endif
	String nest;
	String p = SelectPackage(nest, tt, main, true, true);
	if(p.IsEmpty()) return false;
	InvalidatePackageCache();
	SetMainNest(nest);
	main.Clear();
	if(!IsOpen())
		Open();
	SetMain(p);
	return true;
}

void Ide::NewMainPackage()
{
	if(setmain_newide) {
		Host h;
		CreateHost(h, false, false);
		h.Launch(GetExeFilePath() + " --nosplash");
	}
	else {
		SaveCodeBase();
		OpenMainPackage();
	}
}

void Ide::PackageCursor()
{
	WorkspaceWork::PackageCursor();
	String p = GetActivePackage();
	if(p.IsEmpty()) return;
	String ef = ToLower(NormalizePath(editfile));
	for(int i = 0; i < filelist.GetCount(); i++)
		if(ToLower(NormalizePath(SourcePath(p, filelist[i]))) == ef) {
			filelist.SetCursor(i);
			break;
		}
	SetBar();
}

void Ide::EditWorkspace()
{
	EditPackages(main, GetActivePackage(), pocfg);
	ScanWorkspace();
	SyncWorkspace();
	InvalidateIncludes();
}

String Ide::WorkspaceFile()
{
	String nm;
	for(const char *s = main; *s; s++)
		nm.Cat(*s == '\\' || *s == '/' ? '$' : *s);
	String cfg = ConfigFile("cfg");
	RealizeDirectory(cfg);
	return AppendFileName(cfg, ForceExt(nm + '@' + GetVarsName(), ".cfg"));
}

void Ide::SaveWorkspace()
{
	if(console.console) return;
	if(main.IsEmpty()) return;
	StoreToFile(THISBACK(SerializeWorkspace), WorkspaceFile());
}

void Ide::SyncMainConfigList()
{
	mainconfiglist.Clear();
	const Workspace& wspc = IdeWorkspace();
	if(wspc.GetCount() <= 0) return;
	const Array<Package::Config>& f = wspc.GetPackage(0).config;
	for(int i = 0; i < f.GetCount(); i++)
		mainconfiglist.Add(f[i].param, Nvl(f[i].name, f[i].param));
	SetMainConfigList();
}

void Ide::SetMainConfigList()
{
	mainconfiglist <<= mainconfigparam;
	mainconfigname = mainconfiglist.GetValue();
	mainconfiglist.Tip("Main configuration: " + mainconfigparam);
}

void Ide::OnMainConfigList()
{
	mainconfigparam = ~mainconfiglist;
	SetMainConfigList();
	MakeTitle();
}

void Ide::UscFile(const String& file)
{
	try {
		ParseUscFile(file);
	}
	catch(CParser::Error& e) {
		ShowConsole();
		console << e << "\n";
	}
}

void Ide::UscProcessDir(const String& dir)
{
	for(FindFile ff(AppendFileName(dir, "*.usc")); ff; ff.Next())
		UscFile(AppendFileName(dir, ff.GetName()));
}

void Ide::UscProcessDirDeep(const String& dir)
{
	UscProcessDir(dir);
	for(FindFile ff(AppendFileName(dir, "*")); ff; ff.Next())
		if(ff.IsFolder())
			UscProcessDirDeep(ff.GetPath());
}

void Ide::SyncUsc()
{
	CleanUsc();
	UscProcessDir(GetLocalDir());
	UscProcessDir(GetFileFolder(ConfigFile("x")));

	if(IsNull(main))
		return;
	::Workspace wspc;
	wspc.Scan(main);
	int i;
	for(i = 0; i < wspc.GetCount(); i++) {
		const Package& p = wspc.GetPackage(i);
		for(int j = 0; j < p.file.GetCount(); j++) {
			String file = SourcePath(wspc[i], p.file[j]);
			if(ToLower(GetFileExt(file)) == ".usc")
				UscFile(file);
		}
	}
}

void Ide::CodeBaseSync()
{
	if(auto_check)
		SyncCodeBase();
}

void Ide::SyncWorkspace()
{
	SyncUsc();
	CodeBaseSync();
}

bool IsTextFile(const String& file, int maxline) {
	byte buffer[16384];
	FileIn fi(file);
	if(!fi.IsOpen())
		return false;
	int count = fi.Get(buffer, sizeof(buffer) - 1);
	buffer[count] = 0;
	const byte *end = buffer + count;
	const byte *q = buffer;
	const byte *x = q;
	while(q < end) {
		if(*q < 32) {
			int c = *q;
			if(c == '\n') {
				if(q - x > maxline) return false;
				x = q;
			}
			else
			if(c != '\r' && c != '\t' && c != '\v' && c != '\f' && c != 0x1a)
				return false;
		}
		q++;
	}
	return true;
}

/*
Console& Ide::GetConsole()
{
	int q = btabs.GetCursor();
	return q == BFINDINFILES ? console2 : console;
}
*/
void Ide::Renumber() {
	for(int i = 0; i < filedata.GetCount(); i++)
		::Renumber(filedata[i].lineinfo);
	editor.Renumber();
}

void Ide::CycleFiles()
{
	if(++tabi >= tablru.GetCount())
		tabi = 0;
	if(tabi < tablru.GetCount()) {
		blocktabs = true;
		EditFile(tablru[tabi]);
	}
}

void Ide::DeactivateBy(Ctrl *new_focus)
{
	if(deactivate_save && issaving == 0 && !new_focus && editor.GetLength64() < 1000000) {
		DeactivationSave(true);
		SaveFile();
		DeactivationSave(false);
	}
	TopWindow::DeactivateBy(new_focus);
}

void Ide::Activate()
{
	InvalidateFileTimeCache();
	TopWindow::Activate();
}

bool Ide::Key(dword key, int count)
{
	dword *k = IdeKeys::AK_DELLINE().key;
	if(key == k[0] || key == k[1]) {
		editor.DeleteLine();
		return true;
	}
	k = IdeKeys::AK_CUTLINE().key;
	if(key == k[0] || key == k[1]) {
		editor.CutLine();
		return true;
	}
	switch(key) {
	case K_ALT|K_CTRL_UP:
	case K_ALT|K_CTRL_DOWN:
	case K_ALT_DELETE:
		return filelist.Key(key, count);
	case K_ALT_UP:
		return filelist.Key(K_UP, 0);
	case K_ALT_DOWN:
		return filelist.Key(K_DOWN, 0);
	case K_ALT_PAGEUP:
		return package.Key(K_UP, 0);
	case K_ALT_PAGEDOWN:
		return package.Key(K_DOWN, 0);
	case K_CTRL|K_ALT_LEFT:
		TabsLR( TabBar::JumpDirLeft );
		return true;
	case K_CTRL|K_ALT_RIGHT:
		TabsLR( TabBar::JumpDirRight );
		return true;
	case K_CTRL|K_ALT_B:
		TabsStackLR( TabBar::JumpDirLeft );
		return true;
	case K_CTRL|K_ALT_N:
		TabsStackLR( TabBar::JumpDirRight );
		return true;
	case K_SHIFT|K_CTRL_O:
		AddFile(WorkspaceWork::ANY_FILE);
		return true;
#ifdef PLATFORM_COCOA
	case K_ALT_KEY|K_KEYUP:
	case K_OPTION_KEY|K_KEYUP:
#endif
	case K_CTRL_KEY|K_KEYUP:
		if(tabi) {
			tabi = 0;
			AddLru();
		}
		return true;
	case K_CTRL_TAB:
#ifdef PLATFORM_COCOA
	case K_ALT|K_TAB:
	case K_OPTION|K_TAB:
#endif
		CycleFiles();
		return true;
	case K_ALT_C|K_SHIFT:
		CodeBrowser();
		return true;
	case K_MOUSE_BACKWARD:
		History(-1);
		return true;
	case K_MOUSE_FORWARD:
		History(1);
		return true;
	default:
		if(key >= K_SHIFT_CTRL_0 && key <= K_SHIFT_CTRL_9) {
			Bookmark& b = bookmark[key - K_SHIFT_CTRL_0];
			b.file = editfile;
			b.pos = editor.GetEditPos();
			MakeTitle();
			return true;
		}
		if(key >= K_CTRL_0 && key <= K_CTRL_9) {
			GotoBookmark(bookmark[key - K_CTRL_0]);
			return true;
		}
	}
	return false;
}

void Ide::GotoBookmark(const Bookmark& b)
{
	if(b.file.IsEmpty()) return;
	EditFile(b.file);
	if(bookmark_pos)
		editor.SetEditPos(b.pos);
}

bool Ide::IsHistDiff(int i)
{
	if(i < 0 || i >= history.GetCount())
		return false;
	Bookmark& b = history[i];
	return b.file != editfile || abs(editor.GetCursor64() - b.pos.cursor) > 20;
}

void Ide::IdePaste(String& data)
{
	data.Clear();
	if(AcceptFiles(Clipboard())) {
		Vector<String> s = GetFiles(Clipboard());
		for(int i = 0; i < s.GetCount(); i++)
			if(FileExists(s[i]) && IsTextFile(s[i], 10000)) {
				int64 len = GetFileLength(s[i]);
				if(data.GetLength() + len > 104857600) {
					Exclamation("The paste size breaks the 100MB limit.");
					return;
				}
				data.Cat(LoadFile(s[i]));
			}
	}
}

void Ide::AddHistory()
{
	if(history.GetCount()) {
		if(IsHistDiff(histi))
			++histi;
	}
	else
		histi = 0;
	history.At(histi);
	Bookmark& b = history.Top();
	b.file = editfile;
	b.pos = editor.GetEditPos();
	SetBar();
}

void Ide::EditorEdit()
{
	AddHistory();
	TouchFile(editfile);
}

int  Ide::GetHistory(int d)
{
	if(history.GetCount())
		for(int i = histi + (d > 0); i >= 0 && i < history.GetCount(); i += d)
			if(IsHistDiff(i))
				return i;
	return -1;
}

void Ide::History(int d)
{
	int i = GetHistory(d);
	if(i >= 0) {
		histi = i;
		GotoBookmark(history[histi]);
		SetBar();
	}
}

void Ide::BookKey(int key)
{
	Key(key, 1);
}

void Ide::DoDisplay()
{
	Point p = editor.GetColumnLine(editor.GetCursor64());
	String s;
	s << "Ln " << p.y + 1 << ", Col " << p.x + 1;
	int64 l, h;
	editor.GetSelection(l, h);
	if(h > l)
		s << ", Sel " << h - l;
	display.SetLabel(s);
	
	ManageDisplayVisibility();
}

void Ide::ManageDisplayVisibility()
{
	display.Show(!designer);
}

void Ide::SetIdeState(int newstate)
{
	if(newstate != idestate)
	{
		if(newstate == BUILDING)
			build_start_time = GetSysTime();
		else
		{
			if(idestate == BUILDING && !IsNull(build_start_time))
				stat_build_time += int(GetSysTime() - build_start_time);
			build_start_time = Null;
		}
	}
	idestate = newstate;
	MakeTitle();
	SetBar();
}

void Ide::MakeIcon() {
	Image li = IdeImg::PackageLarge2();
	WString mp = main.ToWString();
	if(!IsNull(mp))
	{
		Size isz = li.GetSize();
		ImageDraw idraw(isz);
		Draw& mdraw = idraw.Alpha();
		idraw.DrawImage(0, 0, li);
		mdraw.DrawImage(0, 0, li, White);
		int fh = DPI(14);
		Size sz(0, 0);
		Font font;
		while(fh > DPI(8)) {
			font = StdFont(fh);
			sz = GetTextSize(mp, font) + Size(4, 2);
			if(sz.cx <= isz.cx)
				break;
			fh--;
		}
		int x = max((isz.cx - sz.cx) / 2, 0);
		int y = isz.cy - sz.cy;
		idraw.DrawRect(x, y, sz.cx, sz.cy, White);
		mdraw.DrawRect(x, y, sz.cx, sz.cy, White);
		idraw.DrawText(x + 2, y + 1, mp, font, Black);
		DrawFrame(idraw, x, y, sz.cx, sz.cy, LtBlue);
		if(state_icon)
			idraw.DrawImage(0, 0, decode(state_icon, 1, IdeImg::IconDebuggingLarge2(),
			                                         2, IdeImg::IconRunningLarge2(),
			                                         IdeImg::IconBuildingLarge2()));
		li = idraw;
	}
	LargeIcon(li);
}

void Ide::SetIcon()
{
	int new_state_icon = 0;
	if((bool)debugger && !IdeIsDebugLock()) {
		new_state_icon = 1;
		return;
	}
	else
	if((GetTimeClick() / 800) & 1) {
		if(debugger)
			new_state_icon = 2;
		else
		if(idestate == BUILDING)
			new_state_icon = 3;
	}
	if(state_icon == new_state_icon)
		return;
	state_icon = new_state_icon;
	MakeIcon();
#ifdef PLATFORM_WIN32
	switch(state_icon) {
	case 1:  Icon(DPI(IdeImg::IconDebugging(), IdeImg::IconDebuggingLarge())); break;
	case 2:  Icon(DPI(IdeImg::IconRunning(), IdeImg::IconRunningLarge())); break;
	case 3:  Icon(DPI(IdeImg::IconBuilding(), IdeImg::IconBuildingLarge())); break;
	default: Icon(DPI(IdeImg::Icon(), IdeImg::PackageLarge()));
	}
#else
	switch(state_icon) {
	case 1:  Icon(IdeImg::IconDebugging()); break;
	case 2:  Icon(IdeImg::IconRunning()); break;
	case 3:  Icon(IdeImg::IconBuilding()); break;
	default: Icon(IdeImg::Icon());
	}
#endif
}

void Ide::Periodic()
{
	CheckFileUpdate();
	SetIcon();
	if(debugger && debugger->IsFinished() && !IdeIsDebugLock())
		IdeEndDebug();
	if(file_scanned) {
		EditFileAssistSync2();
		file_scanned = false;
	}
}

const Workspace& Ide::IdeWorkspace() const
{
	static Workspace wspc;
	static String _main;
	if(main != _main || wspc.GetCount() == 0) {
		wspc.Scan(main);
		_main = main;
	}
	else {
		for(int i = 0; i < wspc.GetCount(); i++)
			if(wspc.GetPackage(i).time != FileGetTime(PackagePath(wspc[i]))) {
				wspc.Scan(main);
				break;
			}
	}
	return wspc;
}

void Ide::AddPackage(const String& p)
{
	const Workspace& wspc = IdeWorkspace();
	for(int i = 0; i < wspc.GetCount(); i++){
		if(wspc[i] == p)
			return;
	}
	if(!PromptOKCancel("Package [* " + p + "] is not yet in the workspace.&Do you want to add it?"))
		return;
	OptItem& m = actual.uses.Add();
	m.text = p;
	SaveLoadPackage();
}

int Ide::GetPackageIndex()
{
	const Workspace& wspc = IdeWorkspace();
	for(int i = 0; i < wspc.GetCount(); i++)
		if(wspc[i] == package.GetCurrentName())
			return i;
	return -1;
}

void Ide::GotoDiffLeft(int line, DiffDlg *df)
{
	EditFile(df->editfile);
	editor.SetCursor(editor.GetPos64(line));
	editor.SetFocus();
}

void Ide::GotoDiffRight(int line, FileDiff *df)
{
	EditFile(df->GetExtPath());
	editor.SetCursor(editor.GetPos64(line));
	editor.SetFocus();
}

void Ide::Diff()
{
	if(IsNull(editfile))
		return;
	FileDiff diffdlg(AnySourceFs());
	diffdlg.diff.WhenLeftLine = THISBACK1(GotoDiffLeft, &diffdlg);
	diffdlg.diff.WhenRightLine = THISBACK1(GotoDiffRight, &diffdlg);
	diffdlg.Execute(editfile);
	String s = diffdlg.GetExtPath();
	if(FileExists(s))
		LruAdd(difflru, s);
}

void Ide::DiffWith(const String& path)
{
	if(IsNull(editfile) || IsNull(path) || max(GetFileLength(editfile), GetFileLength(path)) > 100*1024*1024)
		return;
	FileDiff diffdlg(AnySourceFs());
	diffdlg.diff.WhenLeftLine = THISBACK1(GotoDiffLeft, &diffdlg);
	diffdlg.diff.WhenRightLine = THISBACK1(GotoDiffRight, &diffdlg);
	diffdlg.Execute(editfile, path);
}

struct ConflictDiff : TopWindow {
	Label        left, right;
	TextDiffCtrl diff;
	
	virtual void Layout()
	{
		Size sz = GetSize();
		int  fy = GetStdFont().GetCy() + DPI(5);
		left.LeftPos(0, sz.cx / 2).TopPos(0, fy);
		right.RightPos(0, sz.cx / 2).TopPos(0, fy);
		diff.HSizePos().VSizePos(fy, 0);
	}

	void Set(const char *lname, const String& l, const char *rname, const String& r)
	{
		left = "\1[=* \1" + String(lname);
		right = "\1[=* \1" + String(rname);
		diff.Set(l, r);
	}
	
	ConflictDiff() {
		SetRect(GetWorkArea().Deflated(DPI(32)));
		Sizeable().Zoomable();
		Add(left);
		Add(right);
		Add(diff);
	}
};

String Ide::LoadConflictFile(const String& n)
{
	return n.GetCount() == 1 ? GitCmd(GetFileFolder(editfile), "show :" + n + ":./" + GetFileName(editfile))
	                         : LoadFile(n);
}

void Ide::DiffFiles(const char *lname, const String& l, const char *rname, const String& r)
{
	ConflictDiff diff;
	diff.Set(lname, LoadConflictFile(l), rname, LoadConflictFile(r));
	diff.Execute();
}
