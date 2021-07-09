#include "ide.h"

const char tempaux[] = "<temp-aux>";
const char prjaux[] = "<prj-aux>";
const char ideaux[] = "<ide-aux>";

Image OverLtRed(const Image& m)
{
	Image red = CreateImage(m.GetSize(), Blend(LtRed, SColorPaper));
	Over(red, Point(0, 0), m, m.GetSize());
	return red;
}

Image ImageOverRed(const Image& m)
{
	return MakeImage(m, OverLtRed);
}

Font WorkspaceWork::ListFont()
{
	return StdFont();
};

void WorkspaceWork::SetErrorFiles(const Vector<String>& files)
{
	errorfiles <<= Index<String>(files, 1);
	int i = filelist.GetCursor();
	int s = filelist.GetSbPos();
	SaveLoadPackage(false);
	filelist.SetSbPos(s);
	filelist.SetCursor(i);
	SyncErrorPackages();
}

String WorkspaceWork::PackagePathA(const String& pn) {
	if(pn == prjaux) {
		String nm;
		String cfg = ConfigFile("cfg");
		for(const char *s = main; *s; s++)
			nm.Cat(*s == '\\' || *s == '/' ? '$' : *s);
		RealizeDirectory(cfg);
		return AppendFileName(cfg, ForceExt(nm + '@' + GetVarsName(), ".aux"));
	}
	if(pn == ideaux)
		return ConfigFile("ide.aux");
	if(pn == tempaux)
		return ConfigFile(Sprintf("aux%x.tmp",
#ifdef PLATFORM_WIN32
		          GetCurrentProcessId()
#endif
#ifdef PLATFORM_POSIX
		          getpid()
#endif
		       ));
	if(pn == METAPACKAGE)
		return Null;
	return PackagePath(pn);
}


void WorkspaceWork::SyncErrorPackages()
{
	for(int i = 0; i < package.GetCount(); i++) {
		FileList::File f = package.Get(i);
		if(!IsAux(f.name)) {
			FileList::File ff = f;
			String path = GetFileFolder(PackagePath(f.name));
		#ifdef PLATFORM_WIN32
			path = ToLower(path);
		#endif
			ff.icon = i ? IdeImg::Package() : IdeImg::MainPackage();
			ff.underline = Null;
			for(int q = 0; q < errorfiles.GetCount(); q++) {
				if(errorfiles[q].StartsWith(path)) {
					ff.icon = ImageOverRed(ff.icon);
					ff.underline = LtRed;
					break;
				}
			}
			ff.icon = DPI(ff.icon);
			package.Set(i, ff);
		}
	}
}

struct PackageOrder {
	String mainpath;
	
	int GetMatchLen(const String& x) const {
		if(*x == '<')
			return 0;
		String h = PackagePath(x);
		for(int i = 0; i < mainpath.GetCount(); i++)
			if(mainpath[i] != h[i])
				return i;
		return mainpath.GetCount();
	}
	
	bool operator()(const String& p1, const String& p2) const {
		int l1 = GetMatchLen(p1);
		int l2 = GetMatchLen(p2);
		return l1 != l2 ? l1 > l2 : p1 < p2;
	}
};

void WorkspaceWork::ScanWorkspace() {
	Workspace wspc;
	if(main.GetCount())
		wspc.Scan(main);
	actualpackage.Clear();
	actualfileindex = -1;
	filelist.Clear();
	package.Clear();
	Vector<String> pks;
	for(int i = 0; i < wspc.package.GetCount(); i++)
		pks.Add(wspc.package.GetKey(i));
	if(sort && wspc.GetCount()) {
		PackageOrder po;
		po.mainpath = PackagePath(pks[0]);
		Sort(SubRange(pks, 1, pks.GetCount() - 1), po);
	}
	for(int i = 0; i < wspc.package.GetCount(); i++) {
		String pk = pks[i];
		Font fnt = ListFont();
		if(i == 0)
			fnt.Bold();
		PackageInfo pi = GetPackageInfo(pk);
		if(pi.bold)
			fnt.Bold();
		if(pi.italic)
			fnt.Italic();
		package.Add(pk, Null, fnt, Nvl(AdjustIfDark(pi.ink), SColorText()), false, 0, Null, SColorMark);
	}
	if(!organizer) {
		if(main.GetCount())
			package.Add(prjaux, IdeImg::PrjAux(), ListFont(), AdjustIfDark(Magenta));
		package.Add(ideaux, IdeImg::IdeAux(), ListFont(), AdjustIfDark(Magenta));
		package.Add(tempaux, IdeImg::TempAux(), ListFont(), AdjustIfDark(Magenta));
		if(main.GetCount())
			package.Add(METAPACKAGE, IdeImg::Meta(), ListFont(), AdjustIfDark(Red));
	}
	package.SetCursor(0);
	
	SyncErrorPackages();
}

void WorkspaceWork::SavePackage()
{
	if(IsNull(actualpackage) || actualpackage == METAPACKAGE)
		return;
	InvalidatePackageInfo(actualpackage);
	String pp = PackagePathA(actualpackage);
	if(organizer && backup.Find(pp) < 0) {
		Backup& b = backup.Add(pp);
		FindFile ff(pp);
		if(ff) {
			b.time = ff.GetLastWriteTime();
			b.data = LoadFile(pp);
		}
		else
			b.data = String::GetVoid();
	}
	if(FileExists(pp) || actual.GetCount())
		actual.Save(pp);
}

void WorkspaceWork::RestoreBackup()
{
	for(int i = 0; i < backup.GetCount(); i++) {
		Backup& b = backup[i];
		String fn = backup.GetKey(i);
		if(b.data.IsVoid())
			DeleteFile(fn);
		else {
			SaveFile(fn, b.data);
			SetFileTime(fn, b.time);
		}
	}
}

void WorkspaceWork::SaveLoadPackageNS(bool sel)
{
	SavePackage();
	String p = actualpackage;
	String f;
	if(IsActiveFile())
		f = ActiveFile();
	int psc = package.GetSbPos();
	int fsc = filelist.GetSbPos();
	ScanWorkspace();
	package.SetSbPos(psc);
	package.FindSetCursor(p);
	if (sel) {
		filelist.SetSbPos(fsc);
		filelist.FindSetCursor(f);
	}
}

void WorkspaceWork::SaveLoadPackage(bool sel)
{
	SaveLoadPackageNS(sel);
	SyncWorkspace();
}

bool PathIsLocal(const String& path)
{
#ifdef PLATFORM_WIN32
	char drive[4] = "?:\\";
	*drive = *path;
	return GetDriveType(drive) == DRIVE_FIXED;
#else
	return false;
#endif
}

void WorkspaceWork::LoadActualPackage()
{
#ifdef PLATFORM_WIN32
	static BOOL (WINAPI *PathIsNetworkPathA)(LPCSTR pszPath);
	ONCELOCK {
		DllFn(PathIsNetworkPathA, "Shlwapi.dll", "PathIsNetworkPathA");
	}
#endif

	Time utime = FileGetTime(ConfigFile("version"));
	filelist.Clear();
	fileindex.Clear();
	bool open = true;
	Time tm = GetSysTime();
	for(int i = 0; i < actual.file.GetCount(); i++) {
		Package::File& f = actual.file[i];
		if(f.separator) {
			open = closed.Find(Sepfo(actualpackage, f)) < 0;
			filelist.Add(f, open ? IdeImg::SeparatorClose() : IdeImg::SeparatorOpen(),
			             ListFont().Bold(), open ? SColorMark : SColorText, true, 0, Null);
			fileindex.Add(i);
		}
		else
		if(open) {
			Color uln = Null;
			String p = SourcePath(GetActivePackage(), f);
			if(showtime && (findarg(actualpackage, "<ide-aux>", "<prj-aux>", "<temp-aux>") < 0 || PathIsLocal(p))
	#ifdef PLATFORM_WIN32
			   && !(PathIsNetworkPathA && PathIsNetworkPathA(p))
	#endif
			) {
				FindFile ff(p);
				if(ff) {
					Time ftm = Time(ff.GetLastWriteTime());
					if(ftm > utime) {
						int64 t = tm - ftm;
						if(t < 24 * 3600)
							uln = SColorMark;
						else
						if(t < 32 * 24 * 3600)
							uln = SColorDisabled;
					}
				}
			}
			Image m = IdeFileImage(f, false, f.pch);
			if(GetFileExt(p) == ".tpp" && IsFolder(p)) {
				if(FileExists(AppendFileName(p, "all.i")))
					m = TopicImg::IGroup();
				else
					m = TopicImg::Group();
			}
		#ifdef PLATFORM_WIN32
			p = ToLower(p);
		#endif
			if(errorfiles.Find(p) >= 0) {
				m = ImageOverRed(m);
				uln = LtRed;
			}
			filelist.Add(f, m, ListFont(), SColorText, false, 0,
			             Null, SColorMark, Null, Null, Null, uln);
			fileindex.Add(i);
		}
	}
}

void WorkspaceWork::TouchFile(const String& path)
{
	if(!showtime)
		return;
	String n = GetFileName(path);
	for(int i = 0; i < filelist.GetCount(); i++) {
		FileList::File f = filelist[i];
		if(f.name == n && path == SourcePath(GetActivePackage(), f.name))
			filelist.Set(i, f.name, f.icon, f.font, f.ink, false, 0,
			             Null, SColorMark, Null, Null, Null, SColorMark);
	}
}

void WorkspaceWork::Fetch(Package& p, const String& pkg)
{
	if(pkg.IsEmpty()) return;
	if(pkg == METAPACKAGE) {
		p.file.Clear();
		p.file.AddPick(Package::File(String(HELPNAME)));
		p.file.AddPick(Package::File(ConfigFile("global.defs")));
		for(String d : GetUppDirs()) {
			Package::File sep(GetFileName(d));
			sep.separator = true;
			p.file.AddPick(pick(sep));
			p.file.AddPick(Package::File(AppendFileName(d, "_.tpp")));
			for(String f : { "readme", "license", "copying" }) {
				for(int u = 0; u < 4; u++) {
					FindFile ff(AppendFileName(d, (u & 1 ? ToUpper(f) : f) + (u & 2 ? ".md" : "")));
					if(ff) {
						p.file.AddPick(Package::File(ff.GetPath()));
						break;
					}
				}
			}
		}
	}
	else {
		String pp = PackagePathA(pkg);
		p.Load(pp);
	}
}

void WorkspaceWork::PackageCursor()
{
	InvalidatePackageCache();
	filelist.WhenBar.Clear();
	actualpackage = GetActivePackage();
	repo_dirs = false;
	Fetch(actual, actualpackage);
	LoadActualPackage();
	filelist.Enable();
	if(actualpackage != METAPACKAGE)
		filelist.WhenBar = THISBACK(FileMenu);
	repo_dirs = RepoDirs(true).GetCount();
}

Vector<String> WorkspaceWork::RepoDirs(bool actual)
{
	Vector<String> u = GetUppDirs();
	for(String& s : u)
		s = NormalizePath(s);
	Index<String> id;
	const Workspace& w = GetIdeWorkspace();
	for(int i = 0; i < w.GetCount(); i++) {
		String pp = PackagePath(w[i]);
		for(String s : u)
			if(pp.StartsWith(s))
				id.FindAdd(s);
	}
	
	Vector<String> d = id.PickKeys();
	
	if (actual && !IsAux())
		d.Insert(0, GetFileFolder(PackagePath(actualpackage)));
	Vector<String> r;
	for(int i = 0; i < d.GetCount(); i++)
		if(GetRepoKind(d[i]))
			r.Add(d[i]);
	return r;
}

void WorkspaceWork::FileCursor()
{
	int i = filelist.GetCursor();
	actualfileindex = -1;
	if(i >= 0 && i < fileindex.GetCount())
		actualfileindex = fileindex[i];
}

String WorkspaceWork::FileName(int i) const
{
	return i >= 0 && i < fileindex.GetCount() ? (String)actual.file[fileindex[i]] : Null;
}

bool WorkspaceWork::IsSeparator(int i) const
{
	return i >= 0 && i < fileindex.GetCount() ? actual.file[fileindex[i]].separator : true;
}

String WorkspaceWork::GetActiveFileName() const
{
	return FileName(filelist.GetCursor());
}

String WorkspaceWork::GetActiveFilePath() const
{
	return SourcePath(GetActivePackage(), GetActiveFileName());
}

bool   WorkspaceWork::IsActiveFile() const
{
	int i = filelist.GetCursor();
	return i >= 0 && i < fileindex.GetCount() && fileindex[i] < actual.file.GetCount();
}

Package::File& WorkspaceWork::ActiveFile()
{
	return actual.file[fileindex[filelist.GetCursor()]];
}

void WorkspaceWork::AddFile(ADDFILE af)
{
	String active = GetActivePackage();
	if(active.IsEmpty()) return;
	FileSel *fs = &OutputFs();
	RealizeDirectory(GetLocalDir());
	switch(af)
	{
	case PACKAGE_FILE: fs = &BasedSourceFs(); fs->BaseDir(GetFileFolder(PackagePathA(active))); break;
	case ANY_FILE:     fs = &AnySourceFs(); break;
	case OUTPUT_FILE:  fs->ActiveDir(GetOutputDir()); break;
	case CONFIG_FILE:  fs->ActiveDir(GetConfigDir()); break;
	case HOME_FILE:    fs->ActiveDir(GetHomeDirectory()); break;
	case LOCAL_FILE:   fs->ActiveDir(GetLocalDir()); break;
	default: ; // GCC warns otherwise
	}
	if(!fs->ExecuteOpen("Add files to package..")) return;
	int fci = filelist.GetCursor();
	int cs = filelist.GetSbPos();
	int ci = fci >= 0 && fci < fileindex.GetCount() ? fileindex[fci] : -1;
	for(int i = 0; i < fs->GetCount(); i++) {
		Package::File& f = ci >= 0 ? actual.file.Insert(ci++) : actual.file.Add();
		f = (*fs)[i];
		f.readonly = fs->GetReadOnly();
	}
	SaveLoadPackage(false);
	filelist.SetSbPos(cs);
	filelist.SetCursor(fci >= 0 ? fci : filelist.GetCount() - 1);
	FileSelected();
}

void WorkspaceWork::AddItem(const String& name, bool separator, bool readonly)
{
	int fci = filelist.GetCursor();
	int cs = filelist.GetSbPos();
	int ci = fci >= 0 && fci < fileindex.GetCount() ? fileindex[fci] : -1;
	Package::File& f = ci >= 0 ? actual.file.Insert(ci) : actual.file.Add();
	f = name;
	f.separator = separator;
	f.readonly = readonly;
	if(separator)
		SaveLoadPackageNS(false);
	else
		SaveLoadPackage(false);
	filelist.SetSbPos(cs);
	filelist.SetCursor(fci >= 0 ? fci : filelist.GetCount() - 1);
	FileSelected();
}

void WorkspaceWork::AddSeparator()
{
	String active = GetActivePackage();
	if(active.IsEmpty()) return;
	String name;
	if(!EditText(name, "Add separator", "Name"))
		return;
	AddItem(~name, true, true);
}

class ImportDlg : public WithImportLayout<TopWindow> {
	typedef ImportDlg CLASSNAME;

	FrameRight<Button> dir;
	
	void SetFolder();

public:
	ImportDlg();
};

void ImportDlg::SetFolder()
{
	if(!AnySourceFs().ExecuteSelectDir()) return;
	folder <<= ~AnySourceFs();
}

ImportDlg::ImportDlg()
{
	CtrlLayoutOKCancel(*this, "Import directory tree into package");
	folder.AddFrame(dir);
	dir <<= THISBACK(SetFolder);
	dir.SetImage(CtrlImg::smallright()).NoWantFocus();
	files <<= "*.cpp *.h *.hpp *.c *.C *.cxx *.cc";
}

bool FileOrder_(const String& a, const String& b)
{
	return stricmp(a, b) < 0;
}

void WorkspaceWork::DoImportTree(const String& dir, const String& mask, bool sep, Progress& pi, int from)
{
	String active = GetActivePackage();
	if(active.IsEmpty()) return;
	FindFile ff(AppendFileName(dir, "*.*"));
	Vector<String> dirs, files;
	while(ff) {
		String p = AppendFileName(dir, ff.GetName());
		if(ff.IsFile() && PatternMatchMulti(mask, ff.GetName()))
			files.Add(p);
		if(ff.IsFolder())
			dirs.Add(p);
		ff.Next();
	}
	String relPath(dir.Mid(from)),
		absPath = SourcePath(active, relPath);
	if(sep && files.GetCount()) {
		if(!DirectoryExists(absPath))
			if(!RealizeDirectory(absPath))
				throw Format("An error occurred while creating the directory:&\1%s", absPath);
		Package::File& f = actual.file.Add();
		f = relPath;
		f.separator = f.readonly = true;
	}
	Sort(files, &FileOrder_);
	Sort(dirs, &FileOrder_);
	for(int i = 0; i < files.GetCount(); i++) {
		if(pi.StepCanceled())
			throw String();
		String name = GetFileName(files[i]);
		if(FileCopy(files[i], AppendFileName(absPath, name))) {
			Package::File& f = actual.file.Add();
			f = AppendFileName(relPath, name);
			f.separator = f.readonly = false;
		}
		else
			throw Format("An error occurred while copying the file:&\1%s", files[i]);
	}
	for(int i = 0; i < dirs.GetCount(); i++)
		DoImportTree(dirs[i], mask, true, pi, from);
}

void WorkspaceWork::DoImportTree(const String& dir, const String& mask, bool sep, Progress& pi)
{
	int from = dir.EndsWith(AsString(DIR_SEP)) ? dir.GetCount() : dir.GetCount() + 1;
	DoImportTree(dir, mask, sep, pi, from);
}

void WorkspaceWork::DoImport(const String& dir, const String& mask, bool sep, Progress& pi)
{
	String active = GetActivePackage();
	if(active.IsEmpty()) return;
	FindFile ff(AppendFileName(dir, "*.*"));
	Vector<String> dirs, files;
	while(ff) {
		String p = AppendFileName(dir, ff.GetName());
		if(ff.IsFile() && PatternMatchMulti(mask, ff.GetName()))
			files.Add(p);
		if(ff.IsFolder())
			dirs.Add(p);
		ff.Next();
	}
	if(sep && files.GetCount()) {
		Package::File& f = actual.file.Add();
		f = GetFileTitle(dir);
		f.separator = f.readonly = true;
	}
	Sort(files, &FileOrder_);
	Sort(dirs, &FileOrder_);
	for(int i = 0; i < files.GetCount(); i++) {
		if(pi.StepCanceled())
			throw String();
		String name = GetFileName(files[i]);
		if(FileCopy(files[i], SourcePath(active, name))) {
			Package::File& f = actual.file.Add();
			f = name;
			f.separator = f.readonly = false;
		}
		else
			throw Format("An error occurred while copying the file:&\1%s", files[i]);
	}
	for(int i = 0; i < dirs.GetCount(); i++)
		DoImport(dirs[i], mask, true, pi);
}

void WorkspaceWork::DoImport(const String& dir, const String& mask, bool sep, Progress& pi, bool tree)
{
	if(tree)
		DoImportTree(dir, mask, sep, pi);
	else
		DoImport(dir, mask, sep, pi);
}

void WorkspaceWork::Import()
{
	String active = GetActivePackage();
	if(active.IsEmpty()) return;
	ImportDlg dlg;
	if(dlg.Execute() != IDOK)
		return;
	Progress pi("Importing file %d");
	int fci = filelist.GetCursor();
	int cs = filelist.GetSbPos();
	try {
		DoImport(~dlg.folder, ~dlg.files, false, pi, ~dlg.tree);
	}
	catch(String msg) {
		if(!msg.IsEmpty())
			Exclamation(msg);
	}
	SaveLoadPackage(false);
	filelist.SetSbPos(cs);
	filelist.SetCursor(fci >= 0 ? fci : filelist.GetCount() - 1);
	FileSelected();
}

String TppName(const String& s)
{
	if(s == "src")
		return "Reference - src";
	if(s == "srcdoc")
		return "Documents - srcdoc";
	if(s == "srcimp")
		return "Implementation - srcimp";
	return s;
}

class Tpp : public WithTppLayout<TopWindow> {
public:
	void Sync() {
		bool en = group.IsCursor() && IsNull(group.GetKey());
		name_lbl.Enable(en);
		name.Enable(en);
		name_tpp.Enable(en);
	}

	void Load(const char *dir)
	{
		Index<String> exist;
		FindFile ff(AppendFileName(dir, "*.tpp"));
		while(ff) {
			if(ff.IsFolder()) {
				String s = GetFileTitle(ff.GetName());
				group.Add(s, AttrText(TppName(s)).SetFont(StdFont().Bold()));
				exist.Add(s);
			}
			ff.Next();
		}
		static const char *h[4] = { "src.tpp", "srcdoc.tpp", "srcimp.tpp", "app.tpp" };
		for(int i = 0; i < __countof(h); i++) {
			String s = GetFileTitle(h[i]);
			if(exist.Find(s) < 0)
				group.Add(s, TppName(s) + " (new)");
		}
		group.Add(Null, "<other new>");
		group.GoBegin();
	}

	String GetName()
	{
		String s;
		if(group.IsCursor()) {
			s = group.GetKey();
			if(IsNull(s))
				s << ~name;
			s << ".tpp";
		}
		return s;
	}

	typedef Tpp CLASSNAME;

	Tpp() {
		CtrlLayoutOKCancel(*this, "Insert topic group");
		group.AddKey();
		group.AddColumn("Group");
		group.WhenSel = THISBACK(Sync);
		name.SetFilter(CharFilterAlpha);
	}
};

void WorkspaceWork::AddTopicGroup()
{
	String package = GetActivePackage();
	if(IsNull(package)) return;
	Tpp dlg;
	dlg.Load(PackageDirectory(package));
	if(dlg.Run() != IDOK) return;
	String g = dlg.GetName();
	if(g == "app.tpp") {
		String h = SourcePath(package, g);
		RealizeDirectory(h);
		SaveFile(AppendFileName(h, "all.i"), "");
	}
	if(g.GetCount())
		AddItem(g, false, false);
}

void WorkspaceWork::RemoveFile()
{
	int i = filelist.GetCursor();
	int s = filelist.GetSbPos();
	bool separator = false;
	if(i >= 0 && i < fileindex.GetCount()) {
		int fx = fileindex[i];
		separator = actual.file[fx].separator;
		if(separator && closed.Find(GetActiveSepfo()) >= 0) {
			int px = fx;
			while(--px >= 0 && !actual.file[fx].separator)
				;
			if(px >= 0) {
				int c = closed.Find(Sepfo(GetActivePackage(), actual.file[px]));
				if(c >= 0)
					closed.Unlink(c);
			}
		}
		actual.file.Remove(fx);
	}
	if(separator || IsAux())
		SaveLoadPackageNS(false);
	else
		SaveLoadPackage(false);

	if (separator || FileRemove()) {
		filelist.SetSbPos(s);
		filelist.SetCursor(i);
	}
}

void WorkspaceWork::DelFile()
{
	if(!filelist.IsCursor() || filelist[filelist.GetCursor()].isdir) return;
	String file = GetActiveFilePath();
	if(IsFolder(file)) {
		if(!PromptYesNo("Remove the topic group and discard ALL topics?")) return;
		RemoveFile();
		DeleteFolderDeep(file);
	}
	else {
		if(!PromptYesNo("Remove the file from package and discard it?")) return;
		RemoveFile();
		::DeleteFile(file);
	}
}

void WorkspaceWork::RenameFile()
{
	if(!filelist.IsCursor()) return;
	String n = GetActiveFileName();
	int i = filelist.GetCursor();
	if(i < 0 || i >= fileindex.GetCount())
		return;
	int ii = fileindex[i];
	WithEditStringLayout<TopWindow> dlg;
	CtrlLayoutOKCancel(dlg, "Rename file");
	dlg.lbl = "New name";
	dlg.text <<= n;
	dlg.Open();
	dlg.text.SetFocus();
	int l = int(GetFileNamePos(n) - ~n);
	int h = int(GetFileExtPos(n) - ~n);
	if(l >= h)
		l = 0;
	dlg.text.SetSelection(l, h);
	if(dlg.Run() != IDOK)
		return;
	n = ~dlg.text;
	String spath = GetActiveFilePath();
	String dpath = SourcePath(GetActivePackage(), n);
	if(!filelist[i].isdir && GetFileLength(spath) >= 0) {
		if(!::MoveFile(spath, dpath)) {
			Exclamation("Failed to rename the file.&&" + GetErrorMessage(GetLastError()));
			return;
		}
	}
	FileRename(dpath);
	int s = filelist.GetSbPos();
	(String &)actual.file[ii] = n;
	SaveLoadPackage(false);
	filelist.SetSbPos(s);
	filelist.SetCursor(i);
	if(GetFileExt(spath) == ".iml" || GetFileExt(dpath) == ".iml")
		SyncWorkspace();
}

WorkspaceWork::Sepfo WorkspaceWork::GetActiveSepfo()
{
	return Sepfo(GetActivePackage(), GetActiveFileName());
}

void WorkspaceWork::GroupOrFile(Point pos)
{
	if(pos.x < filelist.GetIconWidth())
		Group();
	if(filelist.IsCursor() && !filelist[filelist.GetCursor()].isdir)
		FileSelected();
}

void   WorkspaceWork::Group()
{
	if(!filelist.IsCursor() || !filelist[filelist.GetCursor()].isdir)
		return;
	int i = filelist.GetCursor();
	int s = filelist.GetSbPos();
	Sepfo as = GetActiveSepfo();
	if(closed.Find(as) >= 0)
		closed.UnlinkKey(as);
	else
		closed.Put(as);
	SaveLoadPackage(false);
	filelist.SetSbPos(s);
	filelist.SetCursor(i);
}

void WorkspaceWork::OpenAllGroups()
{
	for(int i = 0; i < actual.file.GetCount(); i++)
		if(actual.file[i].separator)
			closed.UnlinkKey(Sepfo(GetActivePackage(), actual.file[i]));
	SaveLoadPackage();
}

void WorkspaceWork::CloseAllGroups()
{
	for(int i = 0; i < actual.file.GetCount(); i++)
		if(actual.file[i].separator)
			closed.FindPut(Sepfo(GetActivePackage(), actual.file[i]));
	SaveLoadPackage();
}

void  WorkspaceWork::ShowFile(int pi)
{
	bool open = true;
	Sepfo sf;
	for(int i = 0; i < actual.file.GetCount(); i++) {
		if(actual.file[i].separator) {
			sf = Sepfo(GetActivePackage(), actual.file[i]);
			open = closed.Find(sf) < 0;
		}
		else
		if(i == pi) {
			if(!open) {
				int i = filelist.GetCursor();
				int s = filelist.GetSbPos();
				closed.UnlinkKey(sf);
				SaveLoadPackage(false);
				filelist.SetSbPos(s);
				filelist.SetCursor(i);
			}
			return;
		}
	}
}

bool WorkspaceWork::IsAux(const String& p)
{
	return p == tempaux || p == prjaux || p == ideaux || p == METAPACKAGE;
}

bool WorkspaceWork::IsAux()
{
	return IsAux(actualpackage);
}

void WorkspaceWork::InsertSpecialMenu(Bar& menu)
{
	bool isaux = IsAux();
	menu.Add("Insert any file(s)..", THISBACK1(AddFile, ANY_FILE))
		.Key(K_SHIFT|K_CTRL_I)
		.Help("Insert files from anywhere on disk (discouraged in portable packages)");
	menu.Add(isaux && GetOutputDir().GetCount(), "Insert output directory file(s)..", THISBACK1(AddFile, OUTPUT_FILE))
		.Help("Open file selector in output / intermediate directory for current package");
#ifdef PLATFORM_POSIX
	menu.Add(isaux && GetConfigDir().GetCount(), "Insert config directory file(s)..", THISBACK1(AddFile, CONFIG_FILE))
		.Help("Open file selector in output / intermediate directory for current package");
#endif
	menu.Add(isaux, "Insert Local directory file(s)..", THISBACK1(AddFile, LOCAL_FILE))
		.Help("Open file selector in Local directory for current package");
	menu.Add("Insert home directory file(s)..", THISBACK1(AddFile, HOME_FILE))
		.Help("Open file selector in current user's HOME directory");
}

void WorkspaceWork::SpecialFileMenu(Bar& menu)
{
	InsertSpecialMenu(menu);
	menu.Add("Import directory tree sources..", THISBACK(Import));
}

void WorkspaceWork::OpenFileFolder()
{
	ShellOpenFolder(GetFileDirectory(GetActiveFilePath()));
}

void WorkspaceWork::OpenPackageFolder()
{
	ShellOpenFolder(GetFileDirectory(GetActivePackagePath()));
}

void WorkspaceWork::FileMenu(Bar& menu)
{
	bool sel = filelist.IsCursor() && filelist[filelist.GetCursor()].isdir;
	
	bool isaux = IsAux();
	if(isaux)
		InsertSpecialMenu(menu);
	else {
		menu.Add("New package file..", IdeCommonImg::PageAdd(), [=] { NewPackageFile(); });
		menu.Add(!isaux, "Insert package directory file(s)..", THISBACK1(AddFile, PACKAGE_FILE))
			.Help("Insert file relative to current package");
		menu.Add(!isaux, "Insert topic++ group..", TopicImg::IGroup(), THISBACK(AddTopicGroup));
	}
	menu.Add("Insert separator..", IdeImg::Separator(), [=] { AddSeparator(); })
		.Help("Add text separator line");
	if(!isaux) {
		menu.Add("Insert special", THISBACK(SpecialFileMenu))
		    .Help("Less frequently used methods of adding files to the package");
	}
	menu.Separator();
	if(!organizer) {
		if(sel)
			menu.Add(closed.Find(GetActiveSepfo()) < 0 ? "Close group\t[double-click]"
			                                           : "Open group\t[double-click]", THISBACK(Group));
		menu.Add("Open all groups", THISBACK(OpenAllGroups));
		menu.Add("Close all groups", THISBACK(CloseAllGroups));
		menu.Separator();
		BuildFileMenu(menu);
		menu.Separator();
	}
	menu.Add("Rename...", THISBACK(RenameFile))
	    .Help("Rename file / separator / topic group");
	menu.Add("Remove", THISBACK(RemoveFile))
		.Key(organizer ? K_DELETE : K_ALT_DELETE)
		.Help("Remove file / separator / topic group from package");
	menu.Add(filelist.IsCursor(), "Delete", sel ? THISBACK(RemoveFile) : THISBACK(DelFile))
		.Help("Remove file / topic group reference from package & delete file / folder on disk");
	menu.Separator();
	menu.Add("Open File Directory",THISBACK(OpenFileFolder));
	menu.Add("Copy File Path", callback1(WriteClipboardText, GetActiveFilePath()));
	menu.Add("Terminal at File Directory", [=] { LaunchTerminal(GetFileDirectory(GetActiveFilePath())); });
	menu.Separator();
	menu.Add(filelist.GetCursor() > 0, "Move up", THISBACK1(MoveFile, -1))
		.Key(organizer ? K_CTRL_UP : K_ALT|K_CTRL_UP)
		.Help("Move current file one position towards package beginning");
	menu.Add(filelist.IsCursor() && filelist.GetCursor() < filelist.GetCount() - 1, "Move down",
	         THISBACK1(MoveFile, 1))
		.Key(organizer ? K_CTRL_DOWN : K_ALT|K_CTRL_DOWN)
		.Help("Move current file one position towards package end");
	if(IsActiveFile()) {
		menu.Separator();
		String p = GetActiveFilePath();
		String ext = GetFileExt(p);
		if(ext == ".tpp" && IsFolder(p)) {
			menu.Add("Includeable topic group", THISBACK(ToggleIncludeable))
			    .Check(FileExists(AppendFileName(p, "all.i")));
			if(GetRepoKind(p))
				menu.Add("Repo Synchronize " + p, THISBACK1(SyncSvnDir, p));
		}
		else {
			if(IsHeaderExt(ext))
				menu.Add("Precompile header", THISBACK(TogglePCH))
				    .Check(ActiveFile().pch);
		}
	}
	FilePropertiesMenu(menu);
}

void WorkspaceWork::TogglePCH()
{
	if(IsActiveFile()) {
		ActiveFile().pch = !ActiveFile().pch;
		SaveLoadPackageNS();
	}
}

void WorkspaceWork::ToggleIncludeable()
{
	if(IsActiveFile()) {
		String p = GetActiveFilePath();
		SetTopicGroupIncludeable(p, !FileExists(AppendFileName(p, "all.i")));
		SaveLoadPackageNS();
	}
}

void WorkspaceWork::AddNormalUses()
{
	String p = SelectPackage("Select package");
	if(p.IsEmpty()) return;
	OptItem& m = actual.uses.Add();
	m.text = p;
	SaveLoadPackage();
	InvalidateIncludes();
}

void WorkspaceWork::RemovePackageMenu(Bar& bar)
{
	if(bar.IsScanKeys() || bar.IsScanHelp() || !bar.IsMenuBar())
		return;
	InvalidateIncludes();
	String active = UnixPath(GetActivePackage());
	int usecnt = 0;
	for(int i = 0; i < package.GetCount(); i++) {
		String pn = UnixPath(package[i].name);
		Package prj;
		String pp = PackagePath(pn);
		prj.Load(pp);
		for(int i = 0; i < prj.uses.GetCount(); i++)
			if(UnixPath(prj.uses[i].text) == active) {
				usecnt++;
				bar.Add("Remove from '" + pn + '\'', THISBACK1(RemovePackage, pn))
					.Help(Format("Remove package '%s' from uses section in '%s'", active, pp));
			}
	}
	if(usecnt > 1) {
		bar.MenuSeparator();
		bar.Add("Remove all uses", THISBACK1(RemovePackage, String(Null)))
			.Help(Format("Remove package '%s' from all uses in active project and its submodules", active));
	}
}

void WorkspaceWork::PackageOp(String active, String from_package, String rename)
{
	active = UnixPath(active);
	from_package = UnixPath(from_package);
	rename = UnixPath(rename);
	for(int i = 0; i < package.GetCount(); i++)
		if(*package[i].name != '<' &&
		   (IsNull(from_package) || UnixPath(package[i].name) == from_package)) {
			String pp = PackagePath(package[i].name);
			Package prj;
			if(prj.Load(pp)) {
				for(int i = prj.uses.GetCount(); --i >= 0;)
					if(UnixPath(prj.uses[i].text) == active) {
						if(rename.GetCount())
							prj.uses[i].text = rename;
						else
							prj.uses.Remove(i);
					}
				prj.Save(pp);
			}
		}
	ScanWorkspace();
	SyncWorkspace();
	InvalidateIncludes();
}

void WorkspaceWork::RemovePackage(String from_package)
{
	String active = UnixPath(GetActivePackage());
	if(IsNull(from_package) && !PromptYesNo(Format(
		"Remove package [* \1%s\1] from uses sections of all current packages ?", active)))
		return;
	PackageOp(GetActivePackage(), from_package, Null);
	InvalidateIncludes();
}

void WorkspaceWork::TogglePackageSpeed()
{
	if(!package.IsCursor())
		return;
	SaveLoadPackageNS();
}

void WorkspaceWork::RenamePackage()
{
	if(IsAux() || !package.IsCursor())
		return;
	WithRenamePackageLayout<TopWindow> dlg;
	CtrlLayoutOKCancel(dlg, "Rename package");
	dlg.name.SetFilter(FilterPackageName);
	dlg.name <<= package.Get(package.GetCursor()).name;
	dlg.name.SelectAll();
again:
	if(dlg.Execute() != IDOK)
		return;
	String pn = ~dlg.name;
	String ap = GetActivePackage();
	if(!RenamePackageFs(PackagePath(ap), pn))
		goto again;
	PackageOp(ap, Null, pn);
}

void WorkspaceWork::DeletePackage()
{
	String active = GetActivePackage();
	if(package.GetCursor() == 0) {
		Exclamation("Cannot delete the main package!");
		return;
	}
	if(IsAux() || !package.IsCursor() ||
	   !PromptYesNo("Do you really want to delete package [* \1" + active + "\1]?&&"
	                "[/ Warning:] [* Package will only be removed&"
	                "from packages of current workspace!]"))
		return;
	if(!PromptYesNo("This operation is irreversible.&Do you really want to proceed?"))
		return;
	if(!DeleteFolderDeep(GetFileFolder(GetActivePackagePath()))) {
		Exclamation("Deleting directory has failed.");
		return;
	}
	PackageOp(active, Null, Null);
}

void WorkspaceWork::PackageMenu(Bar& menu)
{
	if(!menu.IsScanKeys()) {
		bool cando = !IsAux() && package.IsCursor();
		String act = UnixPath(GetActivePackage());
		menu.Add(cando, ~Format("Add package to '%s'", act), IdeImg::package_add(), THISBACK(AddNormalUses));
		RemovePackageMenu(menu);
		if(menu.IsMenuBar()) {
			bool main = package.GetCursor() == 0;
			
			menu.Add(cando, "Rename package..", THISBACK(RenamePackage));
			menu.Add(cando && !main, "Delete package", THISBACK(DeletePackage));
			menu.Separator();
			BuildPackageMenu(menu);
			menu.Add("Open Package Directory",THISBACK(OpenPackageFolder));
			menu.Add("Terminal at Package Directory", [=] { LaunchTerminal(GetFileDirectory(GetActivePackagePath())); });
		}
	}
}

void WorkspaceWork::DoMove(int b, bool drag)
{
	int fi = filelist.GetCursor();
	if(fi < 0 || fi >= fileindex.GetCount())
		return;
	int a = fileindex[fi];
	if(a < 0 || b < 0 || a >= actual.file.GetCount() ||
	   (drag ? b > actual.file.GetCount() : b >= actual.file.GetCount()))
		return;
	int s = filelist.GetSbPos();
	ShowFile(a);
	ShowFile(b);
	if(drag) {
		actual.file.Move(a, b);
		if(b >= a)
			b--;
	}
	else
		Swap(actual.file[a], actual.file[b]);
	ShowFile(a);
	ShowFile(b);
	SavePackage();
	LoadActualPackage();
	filelist.SetSbPos(s);
	for(int i = 0; i < fileindex.GetCount(); i++)
		if(fileindex[i] == b) {
			filelist.SetCursor(i);
			break;
		}
	filelist.Sync();
}

void WorkspaceWork::MoveFile(int d)
{
	int bi = filelist.GetCursor() + d;
	if(bi < 0 || bi >= fileindex.GetCount())
		return;
	DoMove(fileindex[bi], false);
}

void WorkspaceWork::DnDInsert(int line, PasteClip& d)
{
	if(GetActivePackage() == METAPACKAGE)
		return;
	if(GetInternalPtr<UppList>(d, "package-file") == &filelist && d.Accept())
		DoMove(line < fileindex.GetCount() ? fileindex[line] : actual.file.GetCount(), true);
	InvalidateIncludes();
}

void WorkspaceWork::Drag()
{
	filelist.DoDragAndDrop(InternalClip(filelist, "package-file"),
	                       filelist.GetDragSample(), DND_MOVE);
}

WorkspaceWork::WorkspaceWork()
{
	package <<= THISBACK(PackageCursor);
	package.WhenBar = THISBACK(PackageMenu);
	package.NoRoundSize();
	package.Columns(2);
	filelist <<= THISBACK(FileCursor);
	filelist.WhenLeftClickPos = THISBACK(GroupOrFile);
	filelist.WhenLeftDouble = THISBACK(Group);
	filelist.Columns(2);
	filelist.NoRoundSize();
	actualfileindex = -1;
	organizer = false;
	package.BackPaintHint();
	filelist.BackPaintHint();
	filelist.WhenDrag = THISBACK(Drag);
	filelist.WhenDropInsert = THISBACK(DnDInsert);
	showtime = false;
	sort = true;
	repo_dirs = false;
}

void WorkspaceWork::SerializeClosed(Stream& s)
{
	Workspace wspc;
	wspc.Scan(main);
	Vector<Sepfo> list;
	for(int i = 0; i < wspc.GetCount(); i++) {
		String pk = wspc[i];
		const Package& p = wspc.GetPackage(i);
		for(int i = 0; i < p.GetCount(); i++)
			if(p[i].separator) {
				Sepfo sf(pk, p[i]);
				if(closed.Find(sf) >= 0)
					list.Add(sf);
			}
	}
	s % list;
	closed = pick(list);
}

void UppList::Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
{
	FileList::File file = ValueTo<FileList::File>(q);
	if(GetFileName(file.name) == "$.tpp" && IsFolder(file.name))
		file.name = GetFileName(GetFileFolder(file.name)) + " templates";
	if(file.name == ConfigFile("global.defs"))
		file.name = "Fixed macros";
	FileList::Paint(w, r, RawToValue(file), ink, paper, style);
}
