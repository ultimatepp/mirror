#include "ide.h"
#include "ide.h"

FileSel& sSD()
{
	static bool b;
	static FileSel fs;
	if(!b) {
		fs.AllFilesType();
		b = true;
	}
	return fs;
}

void Ide::SerializeFindInFiles(Stream& s) {
	int version = 7;
	s / version;
	s % ff.files;
	ff.files.SerializeList(s);
	s % ff.folder;
	ff.folder.SerializeList(s);
	if(version >= 2)
	{
		s % ff.replace;
		ff.replace.SerializeList(s);
		s % ff.style;
	}
	if(version >= 1)
		s % sSD();
	if(version >= 3 && version < 7) {
		DropList dummy;
		s % dummy;
	}

	if(version >= 4)
		s % ff.samecase;
	if(version >= 5)
		s % ff.regexp;
	if(version >= 6)
		s % ff.workspace;
}

void SearchForFiles(Index<String>& files, String dir, String mask, int readonly, Progress& pi) {
	FindFile ff(AppendFileName(dir, "*.*"));
	while(ff) {
		if(ff.IsFolder() && *ff.GetName() != '.')
			SearchForFiles(files, AppendFileName(dir, ff.GetName()), mask, readonly, pi);
		else
		if(ff.IsFile() && PatternMatchMulti(mask, ff.GetName())) {
			if(IsNull(readonly) || !!readonly == !!ff.IsReadOnly()) {
				if(pi.StepCanceled()) return;
				files.FindAdd(AppendFileName(dir, ff.GetName()));
			}
		}
		ff.Next();
	}
}

enum {
	WILDANY = 16,
	WILDONE,
	WILDSPACE,
	WILDNUMBER,
	WILDID,
};

bool Match(const char *f, const char *s, bool we, bool ignorecase, int& count) {
	const char *b = s;
	while(*f) {
		if(*f == WILDANY) {
			f++;
			for(;;) {
				if(Match(f, s, we, ignorecase, count)) {
					count += int(s - b);
					return true;
				}
				if(!*s++) break;
			}
			return false;
		}
		else
		if(*f == WILDONE) {
			if(!*s++) return false;
		}
		else
		if(*f == WILDSPACE) {
			if(*s != ' ' && *s != '\t') return false;
			s++;
			while(*s == ' ' || *s == '\t')
				s++;
		}
		else
		if(*f == WILDNUMBER) {
			if(*s < '0' || *s > '9') return false;
			s++;
			while(*s >= '0' && *s <= '9')
				s++;
		}
		else
		if(*f == WILDID) {
			if(!iscib(*s)) return false;
			s++;
			while(iscid(*s)) s++;
		}
		else {
			if(ignorecase ? ToUpper(*s) != ToUpper(*f) : *s != *f) return false;
			s++;
		}
		f++;
	}
	count = int(s - b);
	return we && iscid(*s) ? false : true;
}

void Ide::AddFoundFile(const String& fn, int ln, const String& line, int pos, int count)
{
	ErrorInfo f;
	f.file = fn;
	f.lineno = ln;
	f.linepos = pos + 1;
	f.len = count;
	f.kind = 0;
	f.message = "\1" + EditorSyntax::GetSyntaxForFilename(fn) + "\1" +
	            AsString(pos) + "\1" + AsString(count) + "\1" + (line.GetCount() > 300 ? line.Mid(0, 300) : line);
	FFound().Add(fn, ln, f.message, RawToValue(f));
}

bool Ide::SearchInFile(const String& fn, const String& pattern, bool wholeword, bool ignorecase,
                       int& n, RegExp *regexp) {
	FileIn in(fn);
	if(!in) return true;
	int ln = 1;
	bool wb = wholeword ? iscid(*pattern) : false;
	bool we = wholeword ? iscid(*pattern.Last()) : false;
	int infile = 0;
	bool sync = false;
	while(!in.IsEof()) {
		String line = in.GetLine();
		bool bw = true;
		int  count;
		if(regexp) {
			if(regexp->Match(line)) {
				AddFoundFile(fn, ln, line, regexp->GetOffset(), regexp->GetLength());
				sync = true;
			}
		}
		else
			for(const char *s = line; *s; s++) {
				if(bw && Match(pattern, s, we, ignorecase, count)) {
					AddFoundFile(fn, ln, line, int(s - line), count);
					sync = true;
					infile++;
					n++;
					break;
				}
				if(wb) bw = !iscid(*s);
			}
		ln++;
	}

	if(sync)
		FFound().Sync();

	in.Close();
	int ffs = ~ff.style;
	if(infile && ffs != STYLE_NO_REPLACE)
	{
		EditFile(fn);
		if(!editor.IsReadOnly()) {
			bool doit = true;
			if(ffs == STYLE_CONFIRM_REPLACE)
			{
				editor.SetCursor(0);
				editor.Find(false, true);
				switch(PromptYesNoCancel(Format("Replace %d lines in [* \1%s\1]?", infile, fn)))
				{
				case 1:  break;
				case 0:  doit = false; break;
				case -1: return false;
				}
			}
			if(doit)
			{
				editor.SelectAll();
				editor.BlockReplace();
				SaveFile();
				FFound().Add(fn, Null, AsString(infile) + " replacements made");
				FFound().Sync();
			}
		}
	}

	return true;
}

void Ide::FindInFiles(bool replace) {
	CodeEditor::FindReplaceData d = editor.GetFindReplaceData();
	CtrlRetriever rf;
	ff.output <<= ffoundi_next;
	rf(ff.find, d.find)
	  (ff.replace, d.replace)
	  (ff.ignorecase, d.ignorecase)
	  (ff.samecase, d.samecase)
	  (ff.wholeword, d.wholeword)
	  (ff.wildcards, d.wildcards)
	  (ff.regexp, d.regexp)
	;
	WriteList(ff.find, d.find_list);
	WriteList(ff.replace, d.replace_list);
	ff.Sync();
	if(IsNull(~ff.folder))
		ff.folder <<= GetUppDir();
	ff.style <<= STYLE_NO_REPLACE;
	ff.Sync();
	ff.itext = editor.GetI();
	ff.Setup(replace);
	
	int c = ff.Execute();

	ff.find.AddHistory();
	ff.replace.AddHistory();

	rf.Retrieve();
	d.find_list = ReadList(ff.find);
	d.replace_list = ReadList(ff.replace);
	editor.SetFindReplaceData(d);
	
	if(c == IDOK) {
		SaveFile();

		SetFFound(~ff.output);

		FFound().HeaderTab(2).SetText("Source line");
		Renumber();
		ff.find.AddHistory();
		ff.files.AddHistory();
		ff.folder.AddHistory();
		ff.replace.AddHistory();
		Progress pi("Found %d files to search.");
		pi.AlignText(ALIGN_LEFT);
		Index<String> files;
		if(ff.workspace) {
			const Workspace& wspc = GetIdeWorkspace();
			for(int i = 0; i < wspc.GetCount(); i++)
				SearchForFiles(files, GetFileFolder(PackagePath(wspc[i])),
					           ~ff.files, ~ff.readonly, pi);
		}
		else
			SearchForFiles(files, NormalizePath(~~ff.folder, GetUppDir()), ~ff.files, ~ff.readonly, pi);
		if(!pi.Canceled()) {
			String pattern;
			RegExp rx, *regexp = NULL;
			if(ff.regexp) {
				rx.SetPattern(~ff.find);
				regexp = &rx;
				pattern = "dummy";
			}
			else
			if(ff.wildcards) {
				String q = ~ff.find;
				for(const char *s = q; *s; s++)
					if(*s == '\\') {
						s++;
						if(*s == '\0') break;
						pattern.Cat(*s);
					}
					else
					switch(*s) {
					case '*': pattern.Cat(WILDANY); break;
					case '?': pattern.Cat(WILDONE); break;
					case '%': pattern.Cat(WILDSPACE); break;
					case '#': pattern.Cat(WILDNUMBER); break;
					case '$': pattern.Cat(WILDID); break;
					default:  pattern.Cat(*s);
					}
			}
			else
				pattern = ~ff.find;
			pi.SetTotal(files.GetCount());
			FFound().Clear();
			pi.SetPos(0);
			int n = 0;
			for(int i = 0; i < files.GetCount(); i++) {
				pi.SetText(files[i]);
				if(pi.StepCanceled()) break;
				if(!IsNull(pattern)) {
					if(!SearchInFile(files[i], pattern, ff.wholeword, ff.ignorecase, n, regexp))
						break;
				}
				else {
					ErrorInfo f;
					f.file = files[i];
					f.lineno = 1;
					f.linepos = 0;
					f.kind = 0;
					f.message = files[i];
					FFound().Add(f.file, 1, f.message, RawToValue(f));
					FFound().Sync();
					n++;
				}
			}
			FFound().HeaderTab(2).SetText(Format("Source line (%d)", FFound().GetCount()));
			if(!IsNull(pattern))
				FFound().Add(Null, Null, AsString(n) + " occurrence(s) have been found.");
			else
				FFound().Add(Null, Null, AsString(n) + "  matching file(s) have been found.");
		}
	}
}


void Ide::FindFileAll(const Vector<Tuple<int64, int>>& f)
{
	SetFFound(ffoundi_next);
	FFound().Clear();
	for(auto pos : f) {
		editor.CachePos(pos.a);
		int linei = editor.GetLinePos64(pos.a);
		WString ln = editor.GetWLine(linei);
		AddFoundFile(editfile, linei + 1, ln.ToString(), lenAsUtf8(~ln, (int)pos.a), lenAsUtf8(~ln + pos.a, pos.b));
	}
	FFound().HeaderTab(2).SetText(Format("Source line (%d)", FFound().GetCount()));
	FFound().Add(Null, Null, AsString(f.GetCount()) + " occurrence(s) have been found.");
}
	
void Ide::FindString(bool back)
{
	if(!editor.FindString(back))
		BeepMuteExclamation();
}

void Ide::TranslateString()
{
	if(editor.IsReadOnly()) return;
	int l, h;
	if(editor.GetSelection(l, h)) {
		editor.Insert(l, "t_(");
		editor.Insert(h + 3, ")");
		editor.SetCursor(h + 4);
		FindString(false);
	}
}

void Ide::InsertWildcard(const char *s) {
	iwc = s;
}

void Ide::FindWildcard() {
	int l, h;
	ff.find.GetSelection(l, h);
	iwc = 0;
	FindWildcardMenu(THISBACK(InsertWildcard), ff.find.GetPushScreenRect().TopRight(), false, NULL, ff.regexp);
	if(iwc.GetCount()) {
		ff.wildcards = true;
		ff.find.SetFocus();
		ff.find.SetSelection(l, h);
		ff.find.RemoveSelection();
		ff.find.Insert(iwc);
	}
}

void Ide::FindSetStdDir(String n)
{
	ff.folder <<= n;
}

void Ide::FindStdDir()
{
	String n = GetFileFolder(editfile);
	MenuBar menu;
	if(!IsNull(n))
		menu.Add(n, THISBACK1(FindSetStdDir, n));
	String hub = GetHubDir();
	Vector<String> pd;
	const Workspace& wspc = GetIdeWorkspace();
	for(int i = 0; i < wspc.GetCount(); i++)
		pd.Add(PackageDirectory(wspc[i]));
	for(String d : GetUppDirs())
		if(!IsHubDir(d) || FindMatch(pd, [&](const String& q) { return q.StartsWith(d); }) >= 0)
			menu.Add(d, [=] { FindSetStdDir(d); });
	menu.Add(GetHubDir(), [=] { FindSetStdDir(GetHubDir()); });
	menu.Execute(&ff.folder, ff.folder.GetPushScreenRect().BottomLeft());
}

void Ide::FindFolder()
{
	if(!sSD().ExecuteSelectDir()) return;
	ff.folder <<= ~sSD();
}

void Ide::SyncFindInFiles()
{
	ff.samecase.Enable(ff.ignorecase);
}

void Ide::ConstructFindInFiles() {
	ff.find.AddButton().SetMonoImage(CtrlImg::smallright()).Tip("Wildcard") <<= THISBACK(FindWildcard);
	static const char *defs = "*.cpp *.h *.hpp *.c *.m *.C *.M *.cxx *.cc *.mm *.MM *.icpp *.sch *.lay *.rc";
	ff.files <<= String(defs);
	ff.files.AddList((String)defs);
	ff.files.AddList((String)"*.txt");
	ff.files.AddList((String)"*.*");
	ff.folder.AddButton().SetMonoImage(CtrlImg::smalldown()).Tip("Related folders") <<= THISBACK(FindStdDir);
	ff.folder.AddButton().SetMonoImage(CtrlImg::smallright()).Tip("Select folder") <<= THISBACK(FindFolder);
	editor.PutI(ff.find);
	editor.PutI(ff.replace);
	CtrlLayoutOKCancel(ff, "Find In Files");
	ff.ignorecase <<= THISBACK(SyncFindInFiles);
	ff.samecase <<= true;
	SyncFindInFiles();
}

void FindInFilesDlg::Sync()
{
	replace.Enable((int)~style);
	bool b = !regexp;
	wildcards.Enable(b);
	ignorecase.Enable(b);
	wholeword.Enable(b);
	folder.Enable(!workspace);
	folder_lbl.Enable(!workspace);
}

FindInFilesDlg::FindInFilesDlg()
{
	regexp <<= style <<= THISBACK(Sync);
	readonly.Add(Null, "All files");
	readonly.Add(0, "Writable");
	readonly.Add(1, "Read only");
	readonly <<= Null;
	workspace <<= THISBACK(Sync);
}

void FindInFilesDlg::Setup(bool replacing)
{
	Title(replacing ? "Find and replace in files" : "Find in files");
	replace_lbl.Show(replacing);
	style.Show(replacing);
	replace_lbl2.Show(replacing);
	replace.Show(replacing);
	Size sz = GetLayoutSize();
	if(!replacing)
		sz.cy -= replace.GetRect().bottom - folder.GetRect().bottom;
	Rect r = GetRect();
	r.SetSize(sz);
	SetRect(r);
	ActiveFocus(find);
}

bool FindInFilesDlg::Key(dword key, int count)
{
	if(key == K_CTRL_I) {
		if(find.HasFocus()) {
			find <<= itext;
			return true;
		}
		if(replace.HasFocus()) {
			replace <<= itext;
			return true;
		}
	}
	return TopWindow::Key(key, count);
}

void Ide::SetFFound(int ii)
{
	ii = clamp(ii, 0, 2);
	SetBottom(BFINDINFILES1 + ii);
	ffoundi_next = (ii + 1) % 3;
}
	
ArrayCtrl& Ide::FFound()
{
	int i = btabs.GetCursor() - BFINDINFILES1;
	return i >= 0 && i < 3 ? ffound[i] : ffound[0];
}

void Ide::CopyFound(bool all)
{
	String txt;
	for(int i = 0; i < FFound().GetCount(); i++) {
		if(all)
			txt << FFound().Get(i, 0) << " (" << FFound().Get(i, 1) << "): ";
		String h = FFound().Get(i, 2);
		if(*h == '\1')
			h = Split(~h + 1, '\1', false).Top();
		txt << h << "\r\n";
	}
	WriteClipboardText(txt);
}

void Ide::FFoundMenu(Bar& bar)
{
	bar.Add("Copy text", THISBACK1(CopyFound, false));
	bar.Add("Copy all", THISBACK1(CopyFound, true));
}
