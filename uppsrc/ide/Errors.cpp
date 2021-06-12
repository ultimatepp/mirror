#include "ide.h"

bool Ide::FindLineError(const String& ln, FindLineErrorCache& cache, ErrorInfo& f)
{
	if(!cache.init) {
		VectorMap<String, String> bm = GetMethodVars(method);
		cache.is_java = (bm.Get("BUILDER", Null) == "JDK");
		cache.upp = GetUppDir();
		cache.init = true;
	}
	const char *s0 = ln;
	while(*s0 == ' ' || *s0 == '\t')
		s0++;
	for(;;) {
		const char *s = s0;
		f.file.Clear();
		for(; s < ln.End(); s++) {
			if(*s != '\"' && (byte)*s >= 32 && *s != '(' && (f.file.GetLength() < 3 || *s != ':'))
				f.file.Cat(*s);
			else {
				if(*s == '\"') {
					f.file = Null;
					s++;
					while(*s && *s != '\"')
						f.file.Cat(*s++);
					if(*s)
						s++;
				}
				String file = f.file;
				int e = file.GetLength();
				while(e > 0 && file[e - 1] == ' ')
					e--;
				file.Trim(e);
				file = TrimLeft(file);
	
				int q = cache.file.Find(file);
				if(q < 0) {
					String file0 = file;
				#ifdef PLATFORM_WIN32
					if(file[0] == '\\' || file[0] == '/')
						file = String(cache.upp[0], 1) + ':' + file;
				#endif
					bool exists = false;
					if(!IsFullPath(file) && *file != '\\' && *file != '/') {
						if(cache.wspc_paths.IsEmpty()) {
							::Workspace  wspc;
							wspc.Scan(main);
							for(int i = 0; i < wspc.GetCount(); i++)
								cache.wspc_paths.Add(GetFileDirectory(PackagePath(wspc[i])));
						}
						for(int i = 0; i < cache.wspc_paths.GetCount(); i++) {
							String path = AppendFileName(cache.wspc_paths[i], file);
							String ext = ToLower(GetFileExt(path));
							if(findarg(ext, ".obj", ".lib", ".o", ".so", ".a", ".", "") < 0) {
								FindFile ff;
								if(ff.Search(path) && ff.IsFile()) {
									file = path;
									exists = true;
									break;
								}
							}
						}
					}
					file = FollowCygwinSymlink(file);
					if(!IsFullPath(file) || !exists && !FileExists(file) || !IsTextFile(file))
						file = Null;
					cache.file.Add(file0, file);
				}
				else
					file = cache.file[q];
				DDUMP(file);
				DDUMP(q);
				if(file.GetCount()) {
					f.file = file;
					while(*s && !IsDigit(*s)) {
						if(*s == '/' || IsAlpha(*s))
							return false;
						s++;
					}
					f.lineno = f.linepos = 0;
					CParser p(s);
					try {
						if(p.IsInt())
							f.lineno = p.ReadInt();
						if(p.Char(':') && p.IsInt())
							f.linepos = p.ReadInt();
					}
					catch(CParser::Error) {}
					const char *ms = p.GetPtr();
					if(ln.Find(": warning") >= 0)
						f.kind = 2;
					else if(ln.Find(": error") >= 0 || ln.Find(": fatal error") >= 0)
						f.kind = 1;
					else
						f.kind = 3;
					const char *hs = ms;
					while(!IsLetter(*hs) && *hs)
						hs++;
					f.message = *hs ? hs : ms;
					f.message = TrimLeft(f.message);
					Vector<String> conf = SplitFlags(mainconfigparam, true);
					String uppout = GetVar("OUTPUT");
					int upplen = uppout.GetLength();
					if(cache.is_java && f.file.GetLength() > upplen
					&& !MemICmp(f.file, uppout, upplen) && f.file[upplen] == DIR_SEP) { // check for preprocessed file
						FileIn fi(f.file);
						if(fi.IsOpen()) {
							String fake_file = f.file;
							int fake_line = 1;
							int file_line = 1;
							while(!fi.IsEof())
							{
								String line = fi.GetLine();
								const char *p = line;
								if(p[0] == '/' && p[1] == '/' && p[2] == '#')
								{
									p += 3;
									if(p[0] == 'l' && p[1] == 'i' && p[2] == 'n' && p[3] == 'e')
										p += 4;
									while(*p == ' ' || *p == '\t')
										p++;
									if(IsDigit(*p))
									{
										fake_line = stou(p, &p);
										while(*p == ' ' || *p == '\t')
											p++;
										if(*p == '\"')
											p++;
										fake_file.Clear();
										while(*p && *p != '\"')
											if(*p == '/')
											{
												fake_file.Cat('/');
												if(p[1] == '/')
													p++;
												p++;
											}
											else
												fake_file.Cat(*p++);
									}
									file_line++;
									continue;
								}
								if(f.lineno <= file_line) {
									f.file = fake_file;
									f.lineno = fake_line;
									f.linepos = 0;
									break;
								}
								file_line++;
								fake_line++;
							}
						}
					}
					return f.lineno > 0;
				}
				else
				if(*s == ':' || !strchr(s, '/') && !strchr(s, '\\')) // safe to say this is final
					break;
				else
					f.file.Cat(*s); // File is not complete, e.g.: C:\Program Files (x86)\Microsoft Visual Studio 10.0\Vc\Include\string.h(186)
			}
		}
		while(*s0 != ' ' && *s0 != '\t') { // skip to next whitespace to try again
			if(s0 >= ln.End())
				return false;
			s0++;
		}
		while(*s0 == ' ' || *s0 == '\t')
			s0++;
	}
}

void Ide::FindError()
{
	FindLineError(console.GetLine(console.GetCursor()));
}

bool Ide::Next(ArrayCtrl& a, int d)
{
	if(a.IsVisible()) {
		int c = a.GetCursor();
		for(;;) {
			c += d;
			if(c >= 0 && c < a.GetCount()) {
				Value v = a.Get(c, "INFO");
				if(v.Is<ErrorInfo>() && !IsNull(v.To<ErrorInfo>().file)) {
					a.SetCursor(c);
					return true;
				}
			}
			else {
				if(d > 0)
					a.GoBegin();
				else
					a.GoEnd();
				return true;
			}
		}
	}
	return false;
}

void Ide::FindNextError()
{
	if(Next(error, 1) || Next(FFound(), 1))
		return;
	int ln = console.GetLine(console.GetCursor());
	int l = ln;
	for(l = ln; l < console.GetLineCount(); l++)
		if(FindLineError(l)) return;
	for(l = 0; l < ln; l++)
		if(FindLineError(l)) return;
}

void Ide::FindPrevError() {
	if(Next(error, -1) || Next(FFound(), -1))
		return;
	int ln = console.GetLine(console.GetCursor());
	int l = ln;
	Host h;
	CreateHost(h, false, disable_uhd);
	for(l = ln - 2; l >= 0; l--)
		if(FindLineError(l)) return;
	for(l = console.GetLineCount() - 1; l > ln; l--)
		if(FindLineError(l)) return;
}

void Ide::ClearErrorEditor()
{
	if(!mark_lines)
		return;

	for(int i = 0; i < filedata.GetCount(); i++) {
		ClearErrorEditor(filedata.GetKey(i));
	}
	
	SetErrorFiles(Vector<String>());
}

void Ide::ClearErrorEditor(String file)
{
	linking = false;

	if(!mark_lines)
		return;
	if(file == editfile)
		editor.ClearErrors();
	else {
		FileData& fd = Filedata(file);
		ClearErrors(fd.lineinfo);
	}
}

void Ide::SetErrorEditor()
{
	if(error.GetCount()) {
		SetBottom(BERRORS);
//		if(!error.IsCursor())
//			error.GoBegin();
	}

	if(!mark_lines)
		return;

	bool refresh = false;
	String    hfile;
	EditorBar hbar;
	Vector<String> errorfiles;
	FindLineErrorCache cache;
	for(int i = 0; i < console.GetLineCount(); i++) {
		ErrorInfo f;
		if(FindLineError(console.GetUtf8Line(i), cache, f)) {
			String file = NormalizePath(f.file);
		#ifdef PLATFORM_WIN32
			errorfiles.Add(ToLower(file));
		#else
			errorfiles.Add(file);
		#endif
			if(editfile == file) {
				editor.SetError(f.lineno - 1, f.kind);
				refresh = true;
			}
			else {
				if(hfile != file) {
					if(hfile.GetCount())
						Filedata(hfile).lineinfo = hbar.GetLineInfo();
					hbar.SetLineInfo(Filedata(file).lineinfo, -1);
					hfile = file;
				}
				hbar.SetError(f.lineno - 1, f.kind);
			}
		}
	}
	if(hfile.GetCount())
		Filedata(hfile).lineinfo = hbar.GetLineInfo();
	if(refresh)
		editor.RefreshFrame();
	SetErrorFiles(errorfiles);
}

void Ide::GoToError(const ErrorInfo& f)
{
	if(IsNull(f.file))
		return;
	String file = NormalizePath(f.file);
	DoEditAsText(file);
	EditFile(file);
	int lp = max(f.linepos - 1, 0);
	int pos = editor.GetPos(editor.GetLineNo(f.lineno - 1), lp);
	editor.SetCursor(pos);
	if(*f.message == '\1') {
		Vector<String> h = Split(~f.message + 1, '\1', false);
		if(h.GetCount() >= 4)
			editor.Illuminate(h[3].Mid(atoi(h[1]), atoi(h[2])).ToWString());
	}
	editor.CenterCursor();
	editor.SetFocus();
	Sync();
}

void Ide::GoToError(ArrayCtrl& a)
{
	Value v = a.Get("INFO");
	if(v.Is<ErrorInfo>())
		GoToError(ValueTo<ErrorInfo>(v));
}

bool Ide::FindLineError(int l) {
	ErrorInfo f;
	FindLineErrorCache cache;
	if(FindLineError(console.GetUtf8Line(l), cache, f)) {
		GoToError(f);
		console.SetSelection(console.GetPos64(l), console.GetPos64(l + 1));
		if(btabs.GetCursor() != BCONSOLE && !BottomIsFindInFiles())
			ShowConsole();
		return true;
	}
	return false;
}

void Ide::ClearErrorsPane()
{
	linking = false;
	linking_line.Clear();
	error_cache.Clear();
	error.Clear();
	SyncErrorsMessage();
	error_count = 0;
	warning_count = 0;
}

void Ide::PutLinking()
{
	linking = true;
	linking_line.Clear();
}

void Ide::PutLinkingEnd(bool ok)
{
	if(!ok && linking) {
		addnotes = true;
		error_count++;
		error.Add(Null, Null, AttrText("Linking has failed").Bold()
			                  .NormalPaper(HighlightSetup::GetHlStyle(HighlightSetup::PAPER_ERROR).color));
		for(int i = 0; i < linking_line.GetCount(); i++) {
			ErrorInfo f;
			if(!FindLineError(linking_line[i], error_cache, f)) {
				f.file = Null;
				f.lineno = Null;
				f.message = TrimLeft(linking_line[i]);
			}
			int linecy;
			error.Add(f.file, f.lineno,
			          AttrText(FormatErrorLine(f.message, linecy))
			          .NormalPaper(HighlightSetup::GetHlStyle(HighlightSetup::PAPER_ERROR).color),
			          RawToValue(f));
			error.SetLineCy(error.GetCount() - 1, linecy);
		}
	}
	SyncErrorsMessage();
	linking = false;
}

bool IsDarkMismatch()
{
	return IsDark(SColorPaper()) != IsDark(HighlightSetup::GetHlStyle(HighlightSetup::PAPER_NORMAL).color);
}

void Ide::TopAlignedDisplay::Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
{
	w.DrawRect(r, paper);
	w.DrawText(r.left, r.top, AsString(q));
}

WString Ide::FormatErrorLine(const String& text, int& linecy)
{
	WString txt;
	int cx = max(GetStdFontCy() * 30, error.HeaderObject().GetTabWidth(2) - error.HeaderTab(2).GetMargin() * 2);
	int x = 0;
	Font fnt = StdFont();
	WString h = text.ToWString();
	linecy = fnt.GetCy();
	const wchar *s = h;
	while(findarg(*s, ' ', '\t') >= 0)
		s++;
	while(*s) {
		int chcx = fnt[*s];
		if(x + chcx > cx) {
			txt.Cat('\n');
			x = 0;
			linecy += fnt.GetCy();
		}
		txt.Cat(*s);
		x += chcx;
		s++;
	}
	return txt;
}

WString Ide::FormatErrorLineEP(const String& text, const char *ep, int& linecy)
{
	WString txt;
	int cx = error.HeaderObject().GetTabWidth(2) - error.HeaderTab(2).GetMargin() * 2;
	int x = 0;
	Font fnt = StdFont();
	WString h = text.ToWString();
	linecy = fnt.GetCy();
	const wchar *s = h;
	while(findarg(*s, ' ', '\t') >= 0) {
		s++;
		if(*ep)
			ep++;
	}
	int lep = ' ';
	while(*s) {
		int chcx = fnt[*s];
		if(x + chcx > cx) {
			txt.Cat('\n');
			x = 0;
			linecy += fnt.GetCy();
		}
		if(lep != *ep) {
			txt.Cat(decode(*ep, '~', 2, '^', 3, 1));
			lep = *ep;
		}
		txt.Cat(*s);
		x += chcx;
		s++;
		if(*ep)
			ep++;
	}
	return txt;
}

struct ElepDisplay : public Display {
	Size    DoPaint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const;

	virtual Size GetStdSize(const Value& q) const;
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const;
};

Size ElepDisplay::DoPaint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
{
	w.DrawRect(r, paper);
	WString txt = q;
	int st = 1;
	const wchar *s = txt;
	const wchar *b = s;
	int x = 0;
	int y = 0;
	int cx = 0;
	int linecy = StdFont().GetLineHeight();
	for(;;) {
		if((byte)*s < ' ') {
			int tcx = GetTextSize(b, StdFont(), int(s - b)).cx;
			if(st != 1 && (style & CURSOR) == 0)
				w.DrawRect(x + r.left, y + r.top, tcx, linecy,
				           HighlightSetup::GetHlStyle(st == 2 ? HighlightSetup::PAPER_WARNING
				                                              : HighlightSetup::PAPER_ERROR).color);
			w.DrawText(x + r.left, y + r.top, b, StdFont(), ink, int(s - b));
			x += tcx;
			cx = max(cx, tcx + x);
			if(*s == '\0')
				break;
			if(*s == '\n') {
				x = 0;
				y += linecy;
			}
			else
				st = *s;
			b = ++s;
		}
		else
			s++;
	}
	return Size(cx, y + linecy);
}

Size ElepDisplay::GetStdSize(const Value& q) const
{
	NilDraw w;
	return DoPaint(w, Rect(0, 0, INT_MAX, INT_MAX), q, Null, Null, 0);
}

void ElepDisplay::Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
{
	DoPaint(w, r, q, ink, paper, style);
}

void Ide::ConsoleLine(const String& line, bool assist)
{
	if(linking) {
		linking_line.Add(line);
		return;
	}
	ErrorInfo f;
	if(FindLineError(line, error_cache, f)) {
		if(assist)
			f.kind = 1;
		if(findarg(f.kind, 1, 2) >= 0 || error.GetCount() == 0) {
			Color paper = HighlightSetup::GetHlStyle(f.kind == 1 ? HighlightSetup::PAPER_ERROR
			                                                     : HighlightSetup::PAPER_WARNING).color;
			if(f.kind == 1)
				error_count++;
			else
				warning_count++;
			if(IsDarkMismatch())
				paper = SColorPaper();
			int linecy;
			error.Add(f.file, f.lineno,
			          AttrText(FormatErrorLine(f.message, linecy)).NormalPaper(paper),
			          RawToValue(f));
			if(prenotes.GetCount()) {
				error.Set(error.GetCount() - 1, "NOTES", prenotes);
				prenotes.Clear();
			}
			error.SetLineCy(error.GetCount() - 1, linecy);
			SyncErrorsMessage();
			addnotes = true;
			return;
		}
	}
	else {
		int q = line.FindAfter(" from "); // GCC style "included from"
		ErrorInfo fi;
		if(q >= 0 && FindLineError(line.Mid(q), error_cache, fi)) {
			fi.message = line;
			prenotes.Add(RawToValue(fi));
			return;
		}
		f.lineno = Null;
		f.file = Null;
		f.message = line;
	}
	if(addnotes) {
		int cnt = error.GetCount();
		if(cnt == 0)
			return;
		ValueArray n = error.Get(cnt - 1, "NOTES");
		bool iserrorpos = true;
		for(const char *s = f.message; *s; s++)
			if(*s != ' ' && *s != '~' && *s != '^' && *s != '|')
				iserrorpos = false;
		int i = n.GetCount() - 1;
		if(iserrorpos && i >= 0) {
			ErrorInfo f0 = ValueTo<ErrorInfo>(n[i]);
			f0.error_pos = f.message;
			n.Set(i, RawToValue(f0));
		}
		else
			n.Add(RawToValue(f));
		error.Set(cnt - 1, "NOTES", n);
	}
}

void Ide::SyncErrorsMessage()
{
	String h;
	String cnt;
	if(IsDarkMismatch()) {
		h = "Message";
		if(error_count)
			cnt << error_count << " error(s)";
		if(warning_count) {
			if(error_count)
				cnt << ", ";
			cnt << warning_count << " warning(s)";
		}
	}
	else  {
		h = "\1[g Message";
		if(error_count)
			cnt << "[*@r " << error_count << " error" << (error_count > 1 ? "s]" : "]");
		if(warning_count) {
			if(error_count)
				cnt << ", ";
			cnt << "[@o " << warning_count << " warning" << (warning_count > 1 ? "s]" : "]");
		}
	}
	if(cnt.GetCount())
		h << " (" << cnt << ")";
	error.HeaderTab(2).SetText(h);
}

void Ide::ConsoleRunEnd()
{
	addnotes = false;
	prenotes.Clear();
}

void Ide::ShowFound()
{
	if(FFound().IsCursor())
		GoToError(FFound());
}

String Ide::GetErrorsText(bool all, bool src)
{
	int c = error.GetCursor();
	if(!all && c < 0)
		return Null;
	String txt;
	int h = all ? error.GetCount() : error.GetCursor() + 1;
	for(int i = all ? 0 : c; i < h; i++) {
		if(src)
			txt << error.Get(i, 0) << " (" << error.Get(i, 1) << "): ";
		txt << error.Get(i, 2) << "\r\n";
	}
	return txt;
}

void Ide::CopyError(bool all)
{
	String s = GetErrorsText(all, true);
	if(s.GetCount())
		WriteClipboardText(s);
}

void Ide::ErrorMenu(Bar& bar)
{
	bar.Add(error.IsCursor(), "Copy", THISBACK1(CopyError, false));
	bar.Add("Copy all", THISBACK1(CopyError, true));
	bar.Separator();
	bar.Add(error.IsCursor(), "Search the web..", IdeImg::Google(), [=] {
		LaunchWebBrowser("https://www.google.com/search?q=" + GetErrorsText(false, false));
	});
}

void Ide::SelError()
{
	if(removing_notes)
		return;
	if(error.IsCursor()) {
		Value v = error.Get("NOTES");
		if(v != "0") {
			int sc = error.GetScroll();
			removing_notes = true;
			for(int i = error.GetCount() - 1; i >= 0; i--)
				if(error.Get(i, "NOTES") == "0")
					error.Remove(i);
			removing_notes = false;
			error.ScrollTo(sc);
			error.ScrollIntoCursor();
			ValueArray n = v;
			int ii = error.GetCursor();
			for(int i = 0; i < n.GetCount(); i++) {
				const ErrorInfo& f = ValueTo<ErrorInfo>(n[i]);
				error.Insert(++ii);
				error.Set(ii, 0, f.file);
				error.Set(ii, 1, f.lineno);
				int linecy;
				if(f.error_pos.GetCount()) {
					error.Set(ii, 2, FormatErrorLineEP(f.message, f.error_pos, linecy));
					error.SetDisplay(ii, 2, Single<ElepDisplay>());
				}
				else
					error.Set(ii, 2, FormatErrorLine(f.message, linecy));
				error.Set(ii, "INFO", n[i]);
				error.Set(ii, "NOTES", "0");
				error.SetLineCy(ii, linecy);
			}
		}
		GoToError(error);
	}
}

void Ide::ShowError()
{
	if(error.IsCursor())
		GoToError(error);
}

void Ide::FoundDisplay::Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
{
	String s = q;
	if(*s == '\1') {
		Vector<String> h = Split(~s + 1, '\1', false);
		if(h.GetCount() < 4)
			return;
		One<EditorSyntax> es = EditorSyntax::Create(h[0]);
		es->IgnoreErrors();
		WString ln = h[3].ToWString();
		Vector<LineEdit::Highlight> hln;
		hln.SetCount(ln.GetCount() + 1);
		for(int i = 0; i < ln.GetCount(); i++) {
			LineEdit::Highlight& h = hln[i];
			h.paper = paper;
			h.ink = SColorText();
			h.chr = ln[i];
			h.font = StdFont();
		}
		HighlightOutput hl(hln);
		es->Highlight(ln.Begin(), ln.End(), hl, NULL, 0, 0);
		int fcy = GetStdFontCy();
		int y = r.top + (r.GetHeight() - fcy) / 2;
		w.DrawRect(r, paper);
		int sl = utf8len(~h[3], atoi(h[1]));
		int sh = utf8len(~h[3] + sl, atoi(h[2])) + sl;
		for(int text = 0; text < 2; text++) {
			int x = r.left;
			for(int i = 0; i < hln.GetCount(); i++) {
				Font fnt = StdFont();
				int a = fnt.GetAscent();
				LineEdit::Highlight& h = hln[i];
				fnt.Bold(h.font.IsBold());
				fnt.Italic(h.font.IsItalic());
				fnt.Underline(h.font.IsUnderline());
				a -= fnt.GetAscent();
				int cw = fnt[h.chr];
				if(h.chr == '\t')
					cw = 4 * fnt[' '];
				Color hpaper = HighlightSetup::GetHlStyle(HighlightSetup::PAPER_SELWORD).color;
				Color hink = h.ink;
				if(IsDarkMismatch()) {
					hpaper = paper;
					hink = ink;
				}
				if(i >= sl && i < sh && !(style & (CURSOR|SELECT|READONLY)) && !text)
					w.DrawRect(x, y, cw, fcy, hpaper);
				if(h.chr != '\t' && text)
					w.DrawText(x, y + a, &h.chr, fnt, hink, 1);
				x += cw;
			}
		}
	}
	else
		StdDisplay().Paint(w, r, q, ink, paper, style);
}

void Ide::FoundFileDisplay::Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
{
	w.DrawRect(r, paper);
	DrawFileName(w, r, q, ink);
}

Size Ide::FoundFileDisplay::GetStdSize(const Value& q) const
{
	return GetDrawFileNameSize(q);
}
