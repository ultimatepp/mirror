#include "ide.h"

#if 0
#define LDUMP(x)     DDUMP(x)
#define LDUMPC(x)    DDUMPC(x)
#define LLOG(x)      DLOG(x)
#else
#define LDUMP(x)
#define LDUMPC(x)
#define LLOG(x)
#endif

#define LTIMING(x) // DTIMING(x)

class IndexSeparatorFrameCls : public CtrlFrame {
	virtual void FrameLayout(Rect& r)                   { r.right -= 1; }
	virtual void FramePaint(Draw& w, const Rect& r) {
		w.DrawRect(r.right - 1, r.top, 1, r.Height(), SColorShadow);
	}
	virtual void FrameAddSize(Size& sz) { sz.cx += 2; }
};

Value AssistEditor::AssistItemConvert::Format(const Value& q) const
{
	int ii = q;
	if(ii >= 0 && ii < editor->assist_item_ndx.GetCount()) {
		ii = editor->assist_item_ndx[ii];
		if(ii < editor->assist_item.GetCount())
			return RawToValue(editor->assist_item[ii]);
	}
	CppItemInfo empty;
	return RawToValue(empty);
}

void AssistEditor::SyncNavigatorPlacement()
{
	int sz = navigatorframe.GetSize();
	if(navigator_right)
		navigatorframe.Right(navigatorpane, sz);
	else
		navigatorframe.Left(navigatorpane, sz);
}

AssistEditor::AssistEditor()
{
	assist_convert.editor = this;
	assist.NoHeader();
	assist.NoGrid();
	assist.AddRowNumColumn().Margin(0).SetConvert(assist_convert).SetDisplay(Single<CppItemInfoDisplay>());
	assist.NoWantFocus();
	assist.WhenLeftClick = THISBACK(AssistInsert);
	type.NoHeader();
	type.NoGrid();
	type.AddColumn();
	type.WhenCursor = THISBACK(SyncAssist);
	type.NoWantFocus();
	popup.Horz(type, assist);
	popup.SetPos(2000);
	auto_assist = auto_check = true;
	commentdp = false;

	SyncNavigatorPlacement();
	navigatorframe.Left(navigatorpane, HorzLayoutZoom(140));
	navigating = false;

	int cy = search.GetMinSize().cy;
	navigatorpane.Add(search.TopPos(0, cy).HSizePos(0, cy + 4));
	navigatorpane.Add(sortitems.TopPos(0, cy).RightPos(0, cy));
	navigatorpane.Add(navigator_splitter.VSizePos(cy, 0).HSizePos());
	navigator_splitter.Vert() << scope << list << navlines;
	navigator_splitter.SetPos(1500, 0);
	navigator_splitter.SetPos(9500, 1);

	navigator = true;

	WhenAnnotationMove = THISBACK(SyncAnnotationPopup);
	WhenAnnotationClick = THISBACK1(EditAnnotation, true);
	WhenAnnotationRightClick = THISBACK1(EditAnnotation, false);
	Annotations(Zx(12));
	annotation_popup.Background(SColorPaper());
	annotation_popup.SetFrame(BlackFrame());
	annotation_popup.Margins(6);
	annotation_popup.NoSb();
	
	thisback = false;
	
	cachedpos = INT_MAX;
	cachedln = -1;
	
	parami = 0;

	param_info.Margins(2);
	param_info.Background(SColorPaper());
	param_info.SetFrame(BlackFrame());
	param_info.BackPaint();
	param_info.NoSb();
	
	include_assist = false;
	
	NoFindReplace();
}

int CppItemInfoOrder(const Value& va, const Value& vb) {
	const CppItemInfo& a = ValueTo<CppItemInfo>(va);
	const CppItemInfo& b = ValueTo<CppItemInfo>(vb);
	return CombineCompare(a.name, b.name)(a.natural, b.natural);
}

void AssistEditor::CloseAssist()
{
	if(popup.IsOpen())
		popup.Close();
	if(annotation_popup.IsOpen())
		annotation_popup.Close();
	assist_item.Clear();
	CloseTip();
}

bool isincludefnchar(int c)
{
	return c && c != '<' && c != '>' && c != '?' &&
	       c != ' ' && c != '\"' && c != '/' && c != '\\' && c >= 32 && c < 65536;
}

String AssistEditor::ReadIdBackPos(int& pos, bool include)
{
	String id;
	bool (*test)(int c) = include ? isincludefnchar : iscid;
	while(pos > 0 && (*test)(GetChar(pos - 1)))
		pos--;
	int q = pos;
	while(q < GetLength64() && (*test)(GetChar(q)))
		id << (char)GetChar(q++);
	return id;
}

String AssistEditor::ReadIdBack(int q, bool include, bool *destructor)
{
	String id = ReadIdBackPos(q, include);
	if(destructor) {
		int n = 0;
		while(q > 0 && isspace(GetChar(q - 1)) && n < 100) {
			q--;
			n++;
		}
		*destructor = q > 0 && GetChar(q - 1) == '~';
	}
	return id;
}

void AssistEditor::DirtyFrom(int line)
{
	if(line >= cachedln) {
		cachedpos = INT_MAX;
		cachedline.Clear();
		cachedln = -1;
	}
	CodeEditor::DirtyFrom(line);
}

int AssistEditor::Ch(int i)
{
	if(i >= 0 && i < GetLength64()) {
		if(i < cachedpos || i - cachedpos > cachedline.GetCount()) {
			cachedln = GetLine(i);
			cachedline = GetWLine(cachedln);
			cachedpos = GetPos32(cachedln);
		}
		i -= cachedpos;
		return i < cachedline.GetCount() ? cachedline[i] : '\n';
	}
	return 0;
}

int AssistEditor::ParsBack(int q)
{
	int level = 1;
	--q;
	while(q > 0) {
		if(isrbrkt(Ch(q)))
			level++;
		if(islbrkt(Ch(q)))
			if(--level <= 0)
				break;
		--q;
	}
	return max(0, q);
}

bool IsSpc(int c)
{
	return c > 0 && c <= 32;
}

void AssistEditor::SkipSpcBack(int& q)
{
	while(q > 0 && IsSpc(Ch(q - 1)))
		q--;
}

String AssistEditor::IdBack(int& qq)
{
	String r;
	if(iscid(Ch(qq - 1))) {
		int q = qq;
		while(iscid(Ch(q - 1)))
			q--;
		if(iscib(Ch(q))) {
			qq = q;
			while(q < GetLength64() && iscid(Ch(q)))
				r.Cat(Ch(q++));
		}
	}
	return r;
}

String AssistEditor::CompleteIdBack(int& q, const Index<String>& locals)
{
	String id;
	for(;;) {
		SkipSpcBack(q);
		if(Ch(q - 1) == ',') {
			q--;
			id = ',' + id;
		}
		else
		if(Ch(q - 1) == '>') {
			q--;
			id = '>' + id;
		}
		else
		if(Ch(q - 1) == '<') {
			q--;
			id = '<' + id;
		}
		else
		if(Ch(q - 1) == ':' && Ch(q - 2) == ':') {
			q -= 2;
			id = "::" + id;
		}
		else {
			if(iscib(*id))
				break;
			String nid = IdBack(q);
			if(IsNull(nid))
				break;
			if(locals.Find(nid) >= 0 && findarg(*id, '<', '>') >= 0)
				return id.Mid(1);
			id = nid + id;
		}
	}
	return id;
}

Vector<String> AssistEditor::ReadBack(int q, const Index<String>& locals)
{
	Vector<String> r;
	type.Clear();
	bool wasid = true;
	for(;;) {
		if(r.GetCount() > 200) {
			r.Clear();
			type.Clear();
			break;
		}
		SkipSpcBack(q);
		int c = Ch(q - 1);
		if(c == '>' && !wasid) {
			q--;
			r.Add() = CompleteIdBack(q, locals) + ">";
			wasid = true;
			continue;
		}
		if(iscid(c)) {
			if(wasid)
				break;
			String id;
			for(;;) {
				id = IdBack(q) + id;
				SkipSpcBack(q);
				if(!(Ch(q - 1) == ':' && Ch(q - 2) == ':'))
					break;
				q -= 2;
				id = "::" + id;
				SkipSpcBack(q);
			}
			r.Add() = id;
			wasid = true;
			continue;
		}
		else {
//			if(findarg(c, '(', '[', '{') >= 0)
//				break;
			if(c == ']') {
				if(wasid)
					break;
				r.Add("[]");
				q = ParsBack(q - 1);
				wasid = false;
				continue;
			}
			else
			if(c == ')') {
				if(wasid)
					break;
				r.Add("()");
				q = ParsBack(q - 1);
				wasid = false;
				continue;
			}
			wasid = false;
			c = Ch(q - 1);
			if(c == '>' && Ch(q - 2) == '-') {
				r.Add("->");
				q -= 2;
				continue;
			}
			if(c == '.') {
				r.Add(".");
				q--;
				continue;
			}
		}
		break;
	}
	Reverse(r);
	return r;
}

void AssistEditor::SyncAssist()
{
	LTIMING("SyncAssist");
	bool destructor;
	String name = ReadIdBack(GetCursor32(), include_assist, &destructor);
	String uname = ToUpper(name);
	assist_item_ndx.Clear();
	int typei = type.GetCursor() - 1;
	Buffer<bool> found(assist_item.GetCount(), false);
	for(int pass = 0; pass < 2; pass++) {
		VectorMap<String, int> over;
		for(int i = 0; i < assist_item.GetCount(); i++) {
			const CppItemInfo& m = assist_item[i];
			if(!found[i] &&
			   (typei < 0 || m.typei == typei) &&
			   (pass ? m.uname.StartsWith(uname) : m.name.StartsWith(name)) &&
			   (!destructor || m.kind == DESTRUCTOR && m.scope == current_type + "::")) {
					int q = include_assist ? -1 : over.Find(m.qitem);
					if(q < 0 || over[q] == m.typei && m.scope.GetCount()) {
						found[i] = true;
						assist_item_ndx.Add(i);
						if(q < 0)
							over.Add(m.qitem, m.typei);
					}
			}
		}
	}
	assist.Clear();
	assist.SetVirtualCount(assist_item_ndx.GetCount());
}

bool AssistEditor::IncludeAssist()
{
	Vector<String> include;
	String ln = GetUtf8Line(GetCursorLine());
	CParser p(ln);
	try {
		if(!p.Char('#') || !p.Id("include"))
			return false;
		if(p.Char('\"')) {
			include.Add(GetFileFolder(theide->editfile));
			include_local = true;
		}
		else {
			p.Char('<');
			include = SplitDirs(theide->GetIncludePath());
			include_local = false;
		}
		include_path.Clear();
		include_back = 0;
		if(include_local)
			while(p.Char3('.', '.', '/') || p.Char3('.', '.', '\\')) {
				include.Top() = GetFileFolder(include.Top());
				include_back++;
			}
		for(;;) {
			String dir;
			while(isincludefnchar(p.PeekChar()))
				dir.Cat(p.GetChar());
			if(dir.GetCount() && (p.Char('/') || p.Char('\\'))) {
				if(include_path.GetCount())
					include_path << '/';
				include_path << dir;
			}
			else
				break;
		}
	}
	catch(CParser::Error) {
		return false;
	}
	Vector<String> folder, upper_folder, file, upper_file;
	for(int i = 0; i < include.GetCount(); i++) {
		FindFile ff(AppendFileName(AppendFileName(include[i], include_path), "*.*"));
		while(ff) {
			String fn = ff.GetName();
			if(!ff.IsHidden()) {
				if(ff.IsFolder()) {
					folder.Add(fn);
					upper_folder.Add(ToUpper(fn));
				}
				else {
					static Index<String> ext(Split(".h;.hpp;.hh;.hxx;.i;.lay;.iml;.t;.dli", ';'));
					String fext = GetFileExt(fn);
					if(fext.GetCount() == 0 || ext.Find(ToLower(fext)) >= 0) {
						file.Add(fn);
						upper_file.Add(ToUpper(fn));
					}
				}
			}
			ff.Next();
		}
	}
	IndexSort(upper_folder, folder);
	Index<String> fnset;
	for(int i = 0; i < folder.GetCount(); i++) {
		String fn = folder[i];
		if(fnset.Find(fn) < 0) {
			fnset.Add(fn);
			CppItemInfo& f = assist_item.Add();
			f.name = f.natural = fn;
			f.access = 0;
			f.kind = KIND_INCLUDEFOLDER;
		}
	}
	IndexSort(upper_file, file);
	for(int i = 0; i < file.GetCount(); i++) {
		String fn = file[i];
		CppItemInfo& f = assist_item.Add();
		f.name = f.natural = fn;
		f.access = 0;
		static Index<String> hdr(Split(".h;.hpp;.hh;.hxx", ';'));
		String fext = GetFileExt(fn);
		f.kind = hdr.Find(ToLower(GetFileExt(fn))) >= 0 || fext.GetCount() == 0 ? KIND_INCLUDEFILE
		                                                                        : KIND_INCLUDEFILE_ANY;
	}
	include_assist = true;
	if(include_path.GetCount())
		include_path << "/";
	PopUpAssist();
	return true;
}

void AssistEditor::Assist()
{
	LTIMING("Assist");
	if(!assist_active)
		return;
	CloseAssist();
	int q = GetCursor32();
	assist_cursor = q;
	assist_type.Clear();
	assist_item.Clear();
	include_assist = false;
	if(IncludeAssist())
		return;
	ParserContext parser;
	Context(parser, GetCursor32());
	Index<String> in_types;
	while(iscid(Ch(q - 1)) || Ch(q - 1) == '~')
		q--;
	SkipSpcBack(q);
	thisback = false;
	current_type.Clear();
	LTIMING("Assist2");
	if(Ch(q - 1) == '(') {
		int qq = q - 1;
		String id = IdBack(qq);
		int tn = findarg(id, "THISBACK", "THISBACK1", "THISBACK2", "THISBACK3", "THISBACK4");
		if(tn >= 0) {
			thisback = true;
			thisbackn = tn > 0;
			GatherItems(parser.current_scope, false, in_types, false);
			RemoveDuplicates();
			PopUpAssist();
			return;
		}
	}
	if(Ch(q - 1) == ':' && Ch(q - 2) == ':') {
		q -= 2;
		Vector<String> tparam;
		String scope = ParseTemplatedType(Qualify(parser.current_scope,
		                                          CompleteIdBack(q, parser.local.GetIndex()),
		                                          parser.context.namespace_using),
		                                  tparam);
		GatherItems(scope, false, in_types, true);
		current_type = scope;
	}
	else {
		String tp;
		Vector<String> xp = ReadBack(q, parser.local.GetIndex());
		bool isok = false;
		if(xp.GetCount() && xp[0].StartsWith("::")) // ::Foo().
			xp[0] = xp[0].Mid(2);
		for(int i = 0; i < xp.GetCount(); i++)
			if(iscib(*xp[i])) {
				isok = true;
				break;
			}
		if(xp.GetCount()) {
			if(isok) { // Do nothing on pressing '.' when there is no identifier before
				Index<String> typeset = EvaluateExpressionType(parser, xp);
				for(int i = 0; i < typeset.GetCount(); i++)
					if(typeset[i].GetCount())
						GatherItems(typeset[i], xp[0] != "this", in_types, false);
			}
		}
		else {
			GatherItems(parser.current_scope, false, in_types, true);
			Vector<String> ns = parser.GetNamespaces();
			for(int i = 0; i < ns.GetCount(); i++)
				if(parser.current_scope != ns[i]) // Do not scan namespace already scanned
					GatherItems(ns[i], false, in_types, true);
		}
	}
	LTIMING("Assist3");
	RemoveDuplicates();
	PopUpAssist();
}

Ptr<Ctrl> AssistEditor::assist_ptr;

bool AssistEditor::WheelHook(Ctrl *, bool inframe, int event, Point p, int zdelta, dword keyflags)
{
	if(!inframe && event == MOUSEWHEEL && assist_ptr && assist_ptr->IsOpen()) {
		assist_ptr->MouseWheel(p, zdelta, keyflags);
		return true;
	}
	return false;
}

void AssistEditor::PopUpAssist(bool auto_insert)
{
	LTIMING("PopUpAssist");
	if(assist_item.GetCount() == 0)
		return;
	int lcy = max(16, BrowserFont().Info().GetHeight());
	type.Clear();
	type.Add(AttrText("<all>").Ink(SColorHighlight()));
	if(assist_type.GetCount() == 0)
		popup.Zoom(1);
	else {
		for(int i = 0; i < assist_type.GetCount(); i++) {
			String s = assist_type[i];
			if(s[0] == ':' && s[1] == ':')
				s = s.Mid(2);
			s = Nvl(s, "<globals>");
			if(s[0] == '<')
				type.Add(AttrText(s).Ink(SColorMark()));
			else
				type.Add(Nvl(s, "<globals>"));
		}
		popup.NoZoom();
	}
	type.SetCursor(0);
	if(!assist.GetCount())
		return;
	LTIMING("PopUpAssist2");
	int cy = VertLayoutZoom(304);
	cy += HeaderCtrl::GetStdHeight();
	assist.SetLineCy(lcy);
	Point p = GetCaretPoint() + GetScreenView().TopLeft();
	Rect wa = GetWorkArea();
	int cx = min(wa.Width() - 100, Zx(800));
	if(p.x + cx > wa.right)
		p.x = wa.right - cx;
	popup.SetRect(RectC(p.x, p.y + cy + GetFontSize().cy < wa.bottom ? p.y + GetFontSize().cy : p.y - cy, cx, cy) & wa);
	popup.BackPaint();
	if(auto_insert && assist.GetCount() == 1) {
		assist.GoBegin();
		AssistInsert();
	}
	else {
		popup.Ctrl::PopUp(this, false, false, true);
		assist_ptr = &assist;
		ONCELOCK {
			InstallMouseHook(AssistEditor::WheelHook);
		}
	}
}

bool sILess(const String& a, const String& b)
{
	return ToUpper(a) < ToUpper(b);
}

void AssistEditor::Complete()
{
	CloseAssist();

	int c = GetCursor32();
	String q = IdBack(c);

	Index<String> ids;
	int64 len = 0;
	for(int i = 0; i < GetLineCount() && len < 1000000; i++) {
		String x = GetUtf8Line(i);
		len += x.GetLength();
		CParser p(x);
		try {
			while(!p.IsEof())
				if(p.IsId()) {
					String h = p.ReadId();
					if(h != q)
						ids.FindAdd(h);
				}
				else
					p.SkipTerm();
		}
		catch(CParser::Error) {
			return;
		}
	}
	
	Vector<String> id = ids.PickKeys();
	Upp::Sort(id, sILess);

	if(q.GetCount()) {
		String h;
		for(int i = 0; i < id.GetCount(); i++) {
			String s = id[i];
			if(s.StartsWith(q)) {
				if(IsNull(h))
					h = s;
				else {
					s.Trim(min(s.GetCount(), h.GetCount()));
					for(int j = 0; j < s.GetCount(); j++)
						if(s[j] != h[j]) {
							h.Trim(j);
							break;
						}
				}
			}
		}
		if(h.GetCount() > q.GetCount())
			Paste(h.Mid(q.GetCount()).ToWString());
	}
	for(int i = 0; i < id.GetCount(); i++) {
		CppItemInfo& f = assist_item.Add();
		f.name = f.natural = f.qitem = id[i];
		f.access = 0;
		f.kind = 100;
	}
	assist_type.Clear();
	PopUpAssist(true);
}

void AssistEditor::Abbr()
{
	CloseAssist();
	int c = GetCursor32();
	int ch;
	String s;
	while(IsAlpha(ch = Ch(c - 1))) {
		s.Insert(0, ch);
		--c;
	}
	int len = s.GetCount();
	s = theide->abbr.Get(s, String());
	if(IsNull(s))
		return;
	NextUndo();
	SetCursor(c);
	Remove(c, len);
	int linepos = c;
	int line = GetLinePos32(linepos);
	WString h = GetWLine(line).Mid(0, linepos);
	for(int i = 0; i < s.GetCount(); i++) {
		ch = s[i];
		switch(ch) {
		case '@':
			c = GetCursor32();
			break;
		case '\n':
			InsertChar('\n');
			for(int j = 0; j < h.GetCount(); j++)
				InsertChar(h[j]);
			break;
		default:
			if((byte)s[i] >= ' ' || s[i] == '\t')
				InsertChar(s[i]);
			break;
		}
	}
	SetCursor(c);
}

void AssistEditor::AssistInsert()
{
	if(assist.IsCursor()) {
		int ii = assist.GetCursor();
		if(ii < 0 || ii >= assist_item_ndx.GetCount())
			return;
		ii = assist_item_ndx[ii];
		if(ii >= assist_item.GetCount()) {
			CloseAssist();
			IgnoreMouseUp();
			return;
		}
		const CppItemInfo& f = assist_item[ii];
		if(include_assist) {
			int ln = GetLine(GetCursor32());
			int pos = GetPos32(ln);
			Remove(pos, GetLineLength(ln));
			SetCursor(pos);
			String h;
			for(int i = 0; i < include_back; i++)
				h << "../";
			Paste(ToUnicode(String().Cat() << "#include "
			                << (include_local ? "\"" : "<")
			                << h << include_path
			                << f.name
			                << (f.kind == KIND_INCLUDEFOLDER ? "/" : include_local ? "\"" : ">")
			                , CHARSET_WIN1250));
			if(f.kind == KIND_INCLUDEFOLDER) {
				Assist();
				IgnoreMouseUp();
				return;
			}
			else {
				String pkg = include_path.Left(include_path.GetCount() - 1);
				Vector<String> nests = GetUppDirs();
				for(int i = 0; i < nests.GetCount(); i++){
					if(FileExists(nests[i] + "/" + include_path + GetFileName(pkg) + ".upp")) {
						Ide *ide = dynamic_cast<Ide *>(TheIde());
						if(ide)
							ide->AddPackage(pkg);
						break;
					}
				}
			}
		}
		else {
			String txt = f.name;
			int l = txt.GetCount();
			int pl = txt.GetCount();
			if(!thisback && f.kind >= FUNCTION && f.kind <= INLINEFRIEND)
				txt << "()";
			int cl = GetCursor32();
			int ch = cl;
			while(iscid(Ch(cl - 1)))
				cl--;
			while(iscid(Ch(ch)))
				ch++;
			Remove(cl, ch - cl);
			SetCursor(cl);
			if(thisback)
				for(;;) {
					int c = Ch(cl++);
					if(!c || Ch(cl) == ',' || Ch(cl) == ')')
						break;
					if(c != ' ') {
						if(thisbackn)
							txt << ", ";
						txt << ')';
						break;
					}
				}
			if(findarg(f.kind, CONSTRUCTOR, DESTRUCTOR) >= 0)
				txt << "()";
			int n = Paste(ToUnicode(txt, CHARSET_WIN1250));
			if(!thisback && f.kind >= FUNCTION && f.kind <= INLINEFRIEND) {
				SetCursor(GetCursor32() - 1);
				StartParamInfo(f, cl);
				if(f.natural.EndsWith("()"))
					SetCursor(GetCursor32() + 1);
			}
			else
			if(thisback) {
				if(thisbackn)
					SetCursor(GetCursor32() - 1);
			}
			else
			if(!inbody)
				SetCursor(cl + n - thisbackn);
			else
			if(pl > l)
				SetSelection(cl + l, cl + pl);
			else
				SetCursor(cl + l);
		}
	}
	CloseAssist();
	IgnoreMouseUp();
}

bool AssistEditor::InCode()
{
	int pos = GetCursor32();
	int line = GetLinePos32(pos);
	One<EditorSyntax> st = GetSyntax(line);
	WString l = GetWLine(line);
	st->ScanSyntax(l, ~l + pos, line, GetTabSize());
	return st->CanAssist();
}

bool isaid(int c)
{
	return c == '~' || iscid(c);
}

bool AssistEditor::Key(dword key, int count)
{
	if(popup.IsOpen()) {
		int k = key & ~K_CTRL;
		ArrayCtrl& kt = key & K_CTRL ? type : assist;
		if(k == K_UP || k == K_PAGEUP || k == K_CTRL_PAGEUP || k == K_CTRL_END) {
			if(kt.IsCursor())
				return kt.Key(k, count);
			else {
				kt.SetCursor(kt.GetCount() - 1);
				return true;
			}
		}
		if(k == K_DOWN || k == K_PAGEDOWN || k == K_CTRL_PAGEDOWN || k == K_CTRL_HOME) {
			if(kt.IsCursor())
				return kt.Key(k, count);
			else {
				kt.SetCursor(0);
				return true;
			}
		}
		if(key == K_ENTER && assist.IsCursor()) {
			AssistInsert();
			return true;
		}
		if(key == K_TAB && !assist.IsCursor() && assist.GetCount()) {
			assist.GoBegin();
			AssistInsert();
			return true;
		}
	}
	int c = GetCursor32();
	int cc = GetChar(c);
	int bcc = c > 0 ? GetChar(c - 1) : 0;
	bool b = CodeEditor::Key(key, count);
	if(b && search.HasFocus())
		SetFocus();
	if(IsReadOnly())
		return b;
	if(assist.IsOpen()) {
		bool (*test)(int c) = include_assist ? isincludefnchar : isaid;
		if(!(*test)(key) &&
		   !((*test)(cc) && (key == K_DELETE || key == K_RIGHT)) &&
		   !((*test)(bcc) && (key == K_LEFT || key == K_BACKSPACE))) {
			if(b) {
				CloseAssist();
				if(include_assist ? (key == '/' || key == '\\') : key == '.')
					Assist();
			}
		}
		else
			SyncAssist();
	}
	else
	if(auto_assist) {
		if(InCode()) {
			if(key == '.' || key == '>' && Ch(GetCursor32() - 2) == '-' ||
			   key == ':' && Ch(GetCursor32() - 2) == ':')
				Assist();
			else
			if(key == '(') {
				int q = GetCursor32() - 1;
				String id = IdBack(q);
				if(id == "THISBACK" || id == "THISBACK1" || id == "THISBACK2" || id == "THISBACK3" || id == "THISBACK4")
					Assist();
			}
		}
		if((key == '\"' || key == '<' || key == '/' || key == '\\') && GetUtf8Line(GetCursorLine()).StartsWith("#include"))
			Assist();
	}
	return b;
}

void AssistEditor::MouseWheel(Point p, int zdelta, dword keyflags)
{
	if(keyflags & K_CTRL)
		WhenFontScroll(sgn(zdelta));
	else
		if(assist.IsOpen())
			assist.MouseWheel(p, zdelta, keyflags);
		else
			CodeEditor::MouseWheel(p, zdelta, keyflags);
}

void AssistEditor::LeftDown(Point p, dword keyflags)
{
	CloseAssist();
	CodeEditor::LeftDown(p, keyflags);
}

void AssistEditor::LostFocus()
{
	CloseAssist();
}

String AssistEditor::RemoveDefPar(const char *s)
{
	String r;
	int lvl = 0;
	bool dp = true;
	while(*s) {
		byte c = *s++;
		if(c == '(')
			lvl++;
		if(lvl == 0) {
			if(c == '=') {
				dp = false;
				if(commentdp)
					r.Cat("/* ");
				else
					while(r.GetCount() && *r.Last() == ' ')
						r.Trim(r.GetCount() - 1);
			}
			if(c == ')') {
				if(!dp && commentdp)
					r.Cat("*/");
				r.Cat(')');
				try {
					if(CParser(s).Char('='))
						break;
				}
				catch(CParser::Error) {}
				r.Cat(s);
				break;
			}
			if(c == ',') {
				if(!dp && commentdp)
					r.Cat("*/");
				dp = true;
			}
		}
		else
		if(c == ')')
			lvl--;
		if(dp || commentdp)
			r.Cat(c);
	}
	return r;
}

String AssistEditor::MakeDefinition(const String& cls, const String& _n)
{
	String n = TrimLeft(_n);
	auto RemoveId = [&](const char *s) {
		int len = strlen(s);
		int q = n.Find(s);
		if(q >= 0 && (q == 0 || !iscid(n[q - 1])) && (q + len >= n.GetCount() || !iscid(n[q + len])))
			n.Remove(q, len);
	};
	RemoveId("override");
	RemoveId("final");
	CParser p(n);
	try {
		bool dest = false;
		const char *beg = n;
		while(!p.IsEof()) {
			const char *b = p.GetPtr();
			if(p.Id("operator"))
				return cls.GetCount() ? NormalizeSpaces(String(beg, b) + ' ' + cls + "::" + b)
				                      : NormalizeSpaces(String(beg, b) + ' ' + b);
			if(p.Char('~')) {
				beg = p.GetPtr();
				dest = true;
			}
			else
			if(p.IsId()) {
				String id = p.ReadId();
				if(p.Char('(')) {
					String rp = RemoveDefPar(p.GetPtr());
					auto merge = [](String a, String b) {
						return IsAlNum(*a.Last()) && IsAlNum(*b) ? a + ' ' + b : a + b;
					};
					if(cls.GetCount() == 0)
						return NormalizeSpaces(merge(String(beg, b), id) + '(' + rp);
					if(dest)
						return NormalizeSpaces(String(beg, b) + cls + "::~" + id + '(' + rp);
					else
						return NormalizeSpaces(merge(String(beg, b), cls) + "::" + id + '(' + rp);
				}
			}
			else
				p.SkipTerm();
		}
	}
	catch(CParser::Error) {}
	return n;
}

void Ide::IdeGotoCodeRef(String coderef)
{
	LLOG("IdeGotoLink " << coderef);
	CodeBaseLock __;
	if(IsNull(coderef)) return;
	String scope, item;
	SplitCodeRef(coderef, scope, item);
	int q = CodeBase().Find(scope);
	if(q < 0)
		return;
	const Array<CppItem>& n = CodeBase()[q];
	q = FindItem(n, item);
	if(q >= 0)
		JumpToDefinition(n, q, scope);
}

bool AssistEditor::Esc()
{
	bool r = false;
	if(assist.IsOpen()) {
		CloseAssist();
		r = true;
	}
	if(param_info.IsOpen()) {
		for(int i = 0; i < PARAMN; i++)
			param[i].line = -1;
		param_info.Close();
		r = true;
	}
	return r;
}

void AssistEditor::SyncNavigatorShow()
{
	navigatorframe.Show(navigator && theide && !theide->designer && !theide->IsEditorMode());
}

void AssistEditor::Navigator(bool nav)
{
	navigator = nav;
	SyncNavigatorShow();
	if(IsNavigator())
		SetFocus();
	SyncNavigator();
	SyncCursor();
}

void AssistEditor::SyncNavigator()
{
	if(navigating)
		return;
	if(IsNavigator()) {
		Search();
		SyncCursor();
	}
	SyncNavigatorShow();
}

void AssistEditor::SelectionChanged()
{
	CodeEditor::SelectionChanged();
	SyncCursor();
	SyncParamInfo();
	WhenSelectionChanged();
}

void AssistEditor::SerializeNavigator(Stream& s)
{
	int version = 5;
	s / version;
	s % navigatorframe;
	s % navigator;
	if(version == 1 || version == 3) {
		Splitter dummy;
		s % dummy;
	}
	if(version >= 4)
		s % navigator_splitter;
	if(version >= 5)
		s % navigator_right;
	Navigator(navigator);
	
	if(s.IsLoading())
		SyncNavigatorPlacement();
}
