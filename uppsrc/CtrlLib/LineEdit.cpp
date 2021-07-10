#include "CtrlLib.h"

namespace Upp {

#define LLOG(x) //  LOG(x)
#define LTIMING(x) // DTIMING(x)

LineEdit::LineEdit() {
	isdrag = false;
	nohbar = false;
	showtabs = false;
	tabsize = 4;
	font = CourierZ(12);
	SetFrame(ViewFrame());
	sb.NoBox();
	AddFrame(sb);
	sb.WhenScroll = THISBACK(Scroll);
	cutline = true;
	bordercolumn = -1;
	bordercolor = Null;
	overwrite = false;
	filter = NULL;
	showspaces = false;
	showlines = false;
	showreadonly = true;
	dorectsel = false;
	hline = vline = Null;
	vlinex = 0;
	warnwhitespace = false;
}

LineEdit::~LineEdit() {}

void LineEdit::MouseWheel(Point, int zdelta, dword keyflags) {
	if(keyflags & K_SHIFT)
		sb.WheelX(zdelta);
	else
		sb.WheelY(zdelta);
}

void   LineEdit::Clear() {
	gcolumn = 0;
	TextCtrl::Clear();
	sb.SetTotal(0, 0);
	sb.Set(0, 0);
	NewScrollPos();
	PlaceCaret(0, false);
}

LineEdit& LineEdit::TabSize(int n) {
	tabsize = n;
	PlaceCaret0(GetColumnLine(cursor));
	Refresh();
	return *this;
}

LineEdit& LineEdit::BorderColumn(int col, Color c)
{
	bordercolumn = col;
	bordercolor = c;
	Refresh();
	return *this;
}

LineEdit& LineEdit::SetFont(Font f) {
	font = f;
	Layout();
	TabSize(tabsize);
	SetSb();
	return *this;
}

Size LineEdit::GetFontSize() const {
	return Size(font.GetMonoWidth(), font.GetCy());
}

void LineEdit::SetRectSelection(int64 anchor, int64 cursor)
{
	dorectsel = true;
	SetSelection(anchor, cursor);
	dorectsel = false;
}

void LineEdit::SetRectSelection(const Rect& rect)
{
	SetRectSelection(GetGPos(rect.top, rect.left), GetGPos(rect.bottom, rect.right));
}

Rect LineEdit::GetRectSelection() const
{
	if(IsRectSelection()) {
		int64 sell, selh;
		GetSelection(sell, selh);
		Rect r(GetColumnLine(sell), GetColumnLine(selh));
		if(r.left > r.right)
			Swap(r.left, r.right);
		return r;
	}
	return Null;
}

bool LineEdit::GetRectSelection(const Rect& rect, int line, int64& l, int64& h)
{
	if(line >= rect.top && line <= rect.bottom) {
		l = GetGPos(line, rect.left);
		h = GetGPos(line, rect.right);
		return true;
	}
	l = h = 0;
	return false;
}

int LineEdit::RemoveRectSelection()
{
	Rect rect = GetRectSelection();
	WString txt;
	for(int i = rect.top; i <= rect.bottom; i++) {
		int64 l, h;
		CacheLinePos(i);
		GetRectSelection(rect, i, l, h);
		WString s = GetWLine(i);
		s.Remove(int(l - GetPos64(i)), int(h - l));
		txt.Cat(s);
		txt.Cat('\n');
	}
	int l = GetPos32(rect.top);
	int h = GetPos32(rect.bottom) + GetLineLength(rect.bottom);
	if(h < GetLength32())
		h++;
	Remove((int)l, int(h - l));
	Insert((int)l, txt);
	return (int)GetGPos(rect.bottom, rect.left);
}

WString LineEdit::CopyRectSelection()
{
	WString txt;
	Rect rect = GetRectSelection();
	for(int i = rect.top; i <= rect.bottom && txt.GetCount() < max_total; i++) {
		int64 l, h;
		CacheLinePos(i);
		int64 pos = GetPos64(i);
		GetRectSelection(rect, i, l, h);
		txt.Cat(GetWLine(i).Mid(int(l - pos), int(h - l)));
#ifdef PLATFORM_WIN32
		txt.Cat('\r');
#endif
		txt.Cat('\n');
	}
	return txt;
}

int LineEdit::PasteRectSelection(const WString& s)
{
	Vector<WString> cl = Split(s, '\n', false);
	Rect rect = GetRectSelection();
	int64 pos = cursor;
	int n = 0;
	for(int i = 0; i < cl.GetCount() && rect.top + i <= rect.bottom; i++) {
		int64 l, h;
		CacheLinePos(i);
		GetRectSelection(rect, i + rect.top, l, h);
		Remove((int)l, int(h - l));
		int nn = Insert((int)l, cl[i]);
		n += nn;
		pos = l + nn;
	}
	PlaceCaret(pos);
	return n;
}

void LineEdit::PasteColumn(const WString& text)
{
	Vector<WString> cl = Split(text, '\n', false);
	if(cl.GetCount() && cl.Top().IsEmpty())
		cl.Drop();
	if(cl.GetCount() == 0)
		return;
	int pos;
	if(IsRectSelection()) {
		Rect t = GetRectSelection();
		RemoveSelection();
		Point p = t.TopLeft();
		pos = (int)cursor;
		for(int i = 0; i < t.bottom - t.top + 1; i++) {
			CacheLinePos(i + p.y);
			int l = (int)GetGPos(i + p.y, p.x);
			pos = l + Insert(l, cl[i % cl.GetCount()]);
		}
	}
	else {
		RemoveSelection();
		Point p = GetColumnLine(cursor);
		pos = (int)cursor;
		for(int i = 0; i < cl.GetCount(); i++) {
			CacheLinePos(i + p.y);
			int li = p.y + i;
			if(li < GetLineCount()) {
				int l = (int)GetGPos(i + p.y, p.x);
				pos = l + Insert(l, cl[i]);
			}
			else {
				Insert(GetLength32(), cl[i] + "\n");
				pos = GetLength32();
			}
		}
	}
	PlaceCaret(pos);
}

void LineEdit::PasteColumn()
{
	WString w = ReadClipboardUnicodeText();
	if(w.IsEmpty())
		w = ReadClipboardText().ToWString();
	PasteColumn(w);
	Action();
}

bool sSortLineOrder(const WString& l1, const WString& l2)
{
	return ToUpper(l1) < ToUpper(l2);
}

void LineEdit::Sort()
{
	if(!IsRectSelection())
		return;
	CopyRectSelection();
	Rect rect = GetRectSelection();
	Vector<WString> key;
	Vector<WString> ln;
	for(int i = rect.top; i <= rect.bottom; i++) {
		int64 l, h;
		GetRectSelection(rect, i, l, h);
		key.Add(GetW((int)l, int(h - l)));
		ln.Add(GetWLine(i));
	}
	int sell = GetPos32(rect.top);
	int selh = rect.bottom + 1 < GetLineCount() ? GetPos32(rect.bottom + 1) : GetLength32();
	IndexSort(key, ln, sSortLineOrder);
	Remove(sell, selh - sell);
	Insert(sell, Join(ln, "\n"));
}

class sOptimizedRectRenderer {
	Draw& w;
	Rect  cr;
	Color color;

public:
	void DrawRect(const Rect& r, Color color);
	void DrawRect(int x, int y, int cx, int cy, Color color) { DrawRect(RectC(x, y, cx, cy), color); }
	void Flush();

	sOptimizedRectRenderer(Draw& w) : w(w) { cr = Null; color = Null; }
	~sOptimizedRectRenderer()              { Flush(); }
};

void sOptimizedRectRenderer::Flush()
{
	LTIMING("RectFlush");
	if(!IsNull(cr)) {
		w.DrawRect(cr, color);
		cr = Null;
	}
}

void sOptimizedRectRenderer::DrawRect(const Rect& r, Color c)
{
	LTIMING("DrawRect");
	if(cr.top == r.top && cr.bottom == r.bottom && cr.right == r.left && c == color) {
		cr.right = r.right;
		return;
	}
	Flush();
	cr = r;
	color = c;
}

#if 1 // This is a more ambitious approach combining non-continual chunks of text, it is a bit faster...
class sOptimizedTextRenderer {
	Draw&       w;
	int         y;
	struct Chrs : Moveable<Chrs> {
		Vector<int> x;
		Vector<int> width;
		WString     text;
	};
	VectorMap< Tuple2<Font, Color>, Chrs > cache;

public:
	void DrawChar(int x, int y, int chr, int width, Font afont, Color acolor);
	void Flush();

	sOptimizedTextRenderer(Draw& w) : w(w) { y = Null; }
	~sOptimizedTextRenderer()              { Flush(); }
};

void sOptimizedTextRenderer::Flush()
{
	if(cache.GetCount() == 0)
		return;
	LTIMING("Flush");
	for(int i = 0; i < cache.GetCount(); i++) {
		Chrs& c = cache[i];
		if(c.x.GetCount()) {
			Tuple2<Font, Color> fc = cache.GetKey(i);
			int x = c.x[0];
			for(int i = 0; i < c.x.GetCount() - 1; i++)
				c.x[i] = c.x[i + 1] - c.x[i];
			c.x.Top() = c.width.Top();
			w.DrawText(x, y, c.text, fc.a, fc.b, c.x);
		}
	}
	cache.Clear();
}

void sOptimizedTextRenderer::DrawChar(int x, int _y, int chr, int width, Font font, Color color)
{
	LTIMING("DrawChar");
	if(y != _y) {
		Flush();
		y = _y;
	}
	Chrs *c;
	{
		LTIMING("Map");
		c = &cache.GetAdd(MakeTuple(font, color));
	}
	if(c->x.GetCount() && c->x.Top() > x || font.IsUnderline() && cache.GetCount() > 1) {
		Flush();
		c = &cache.GetAdd(MakeTuple(font, color));
	}
	c->text.Cat(chr);
	c->width.Add(width);
	c->x.Add(x);
}
#else
class sOptimizedTextRenderer {
	Draw&       w;
	int         x, y;
	int         xpos;
	Vector<int> dx;
	WString     text;
	Font        font;
	Color       color;

public:
	void DrawChar(int x, int y, int chr, int width, Font afont, Color acolor);
	void Flush();

	sOptimizedTextRenderer(Draw& w) : w(w) { y = Null; }
	~sOptimizedTextRenderer()              { Flush(); }
};

void sOptimizedTextRenderer::Flush()
{
	if(text.GetCount() == 0)
		return;
	LTIMING("Flush");
	w.DrawText(x, y, text, font, color, dx);
	y = Null;
	text.Clear();
	dx.Clear();
}

void sOptimizedTextRenderer::DrawChar(int _x, int _y, int chr, int width, Font _font, Color _color)
{
	LTIMING("DrawChar");
	if(y == _y && font == _font && color == _color && dx.GetCount() && _x >= xpos - dx.Top())
		dx.Top() += _x - xpos;
	else {
		LTIMING("DrawChar flush");
		Flush();
		x = _x;
		y = _y;
		font = _font;
		color = _color;
	}
	dx.Add(width);
	text.Cat(chr);
	xpos = _x + width;
}
#endif

void   LineEdit::Paint0(Draw& w) {
	LTIMING("LineEdit::Paint0");
	GuiLock __;
	int64 sell, selh;
	GetSelection(sell, selh);
	if(!IsEnabled())
		sell = selh = 0;
	Rect rect(0, 0, 0, 0);
	bool rectsel = IsRectSelection();
	if(rectsel)
		rect = GetRectSelection();
	Size sz = GetSize();
	Size fsz = GetFontSize();
	Point sc = sb;
	int ll = min(GetLineCount(), sz.cy / fsz.cy + sc.y + 1);
	int  y = 0;
	sc.y = minmax(sc.y, 0, GetLineCount() - 1);
	cpos = GetPos64(sc.y);
	cline = sc.y;
	sell -= cpos;
	selh -= cpos;
	int64 pos = cpos;
	int fascent = font.Info().GetAscent();
	int cursorline = GetLine(cursor);
	Highlight ih;
	ih.ink = color[IsShowEnabled() ? INK_NORMAL : INK_DISABLED];
	ih.paper = color[IsReadOnly() && showreadonly || !IsShowEnabled() ? PAPER_READONLY : PAPER_NORMAL];
	if(nobg)
		ih.paper = Null;
	ih.font = font;
	ih.chr = 0;
	for(int i = sc.y; i < ll; i++) {
		Color showcolor = color[WHITESPACE];
		WString tx = GetWLine(i);
		bool warn_whitespace = false;
		if(warnwhitespace && !IsSelection()) {
			int64 pos = GetCursor64();
			int linei = GetLinePos64(pos);
			if(linei != i || pos < tx.GetCount()) {
				int wkind = 0;
				bool empty = true;
				for(const wchar *s = tx; *s; s++) {
					if(*s == '\t') {
						if(wkind == ' ') {
							warn_whitespace = true;
							break;
						}
						wkind = '\t';
					}
					else
					if(*s == ' ')
						wkind = ' ';
					else
					if(*s > ' ') {
						empty = false;
						wkind = 0;
					}
				}
				if(wkind && !empty)
					warn_whitespace = true;
				if(warn_whitespace)
					showcolor = color[WARN_WHITESPACE];
			}
		}
		bool do_highlight = tx.GetCount() < 100000;
		int len = tx.GetLength();
		if(w.IsPainting(0, y, sz.cx, fsz.cy)) {
			LTIMING("PaintLine");
			Vector<Highlight> hl;
			int ln;
			if(do_highlight) {
				hl.SetCount(len + 1, ih);
				for(int q = 0; q < tx.GetCount(); q++)
					hl[q].chr = tx[q];
				LTIMING("HighlightLine");
				HighlightLine(i, hl, pos);
				ln = hl.GetCount() - 1;
			}
			else
				ln = tx.GetCount();
			int lgp = -1;
			for(int pass = 0; pass < 3; pass++) {
				int gp = 0;
				int scx = fsz.cx * sc.x;
				sOptimizedRectRenderer rw(w);
				if(ln >= 0) {
					int q = 0;
					int x = 0;
					int scx2 = scx - max(2, tabsize) * fsz.cx;
					while(q < ln && x < scx2) { // Skip part before left border
						wchar chr = do_highlight ? hl[q++].chr : tx[q++];
						if(chr == '\t') {
							gp = (gp + tabsize) / tabsize * tabsize;
							x = fsz.cx * gp;
						}
						else
						if(IsDoubleWidth(chr)) {
							x += 2 * fsz.cx;
							gp += 2;
						}
						else {
							x += fsz.cx;
							gp++;
						}
					}
					sOptimizedTextRenderer tw(w);
					Highlight lastHighlight;
					while(q < ln) {
						if(q == tx.GetCount())
							lgp = gp;
						Highlight h;
						if(do_highlight)
							h = hl[q];
						else {
							h = ih;
							h.chr = tx[q];
						}
						int pos = min(q, len); // Highligting can add chars at the end of line
						if(rectsel ? i >= rect.top && i <= rect.bottom && gp >= rect.left && gp < rect.right
						           : pos >= sell && pos < selh) {
							h.paper = color[PAPER_SELECTED];
							h.ink = color[INK_SELECTED];
						}
						int x = gp * fsz.cx - scx;
						bool cjk = IsDoubleWidth(h.chr);
						int xx = x + (gp + 1 + cjk) * fsz.cx;
						if(h.chr == '\t') {
							int ngp = (gp + tabsize) / tabsize * tabsize;
							int l = ngp - gp;
							LLOG("Highlight -> tab[" << q << "] paper = " << h.paper);
							if(x >= -fsz.cy * tabsize) {
								if(pass == 0) {
									rw.DrawRect(x, y, fsz.cx * l, fsz.cy, h.paper);
									if((showtabs || warn_whitespace) &&
									   h.paper != SColorHighlight && q < tx.GetLength()) {
										rw.DrawRect(x + 2, y + fsz.cy / 2, l * fsz.cx - 4, 1, showcolor);
										rw.DrawRect(ngp * fsz.cx - scx - 3, y + 3, 1, fsz.cy - 6, showcolor);
									}
									if(bordercolumn > 0 && bordercolumn >= gp && bordercolumn < gp + l)
										rw.DrawRect((bordercolumn - sc.x) * fsz.cx, y, 1, fsz.cy, bordercolor);
								}
								if(pass == 2) // resolve underlined tabs
									tw.DrawChar(x, y, ' ', fsz.cx * l, h.font, h.ink);
							}
							q++;
							gp = ngp;
						}
						else
						if(h.chr == ' ') {
							LLOG("Highlight -> space[" << q << "] paper = " << h.paper);
							if(x >= -fsz.cy) {
								if(pass == 0) {
									rw.DrawRect(x, y, fsz.cx, fsz.cy, h.paper);
									if((showspaces || warn_whitespace)
									   && h.paper != SColorHighlight && q < tx.GetLength()) {
										int n = fsz.cy / 10 + 1;
										rw.DrawRect(x + fsz.cx / 2, y + fsz.cy / 2, n, n, showcolor);
									}
									if(bordercolumn > 0 && bordercolumn >= gp && bordercolumn < gp + 1)
										rw.DrawRect((bordercolumn - sc.x) * fsz.cx, y, 1, fsz.cy, bordercolor);
								}
								if(pass == 2) // resolve underlined spaces
									tw.DrawChar(x, y, ' ', fsz.cx, h.font, h.ink);
							}
							q++;
							gp++;
						}
						else {
							LLOG("Highlight -> paper[" << q << "] = " << h.paper);
							if(max(x, 0) < min(xx, sz.cx) && fsz.cx >= -fsz.cy) {
								if(pass == 0) {
									rw.DrawRect(x, y, (cjk + 1) * fsz.cx, fsz.cy, h.paper);
									if(bordercolumn > 0 && bordercolumn >= gp && bordercolumn < gp + 1 + cjk)
										rw.DrawRect((bordercolumn - sc.x) * fsz.cx, y, 1, fsz.cy, bordercolor);
								}
								if(pass == 1 && (h.flags & SPELLERROR))
									rw.DrawRect(x, max(y, y + fsz.cy - Zy(1)), (cjk + 1) * fsz.cx, Zy(1), LtRed());
								if(pass == 2)
									tw.DrawChar(x + (h.flags & SHIFT_L ? -fsz.cx / 6 : h.flags & SHIFT_R ? fsz.cx / 6 : 0),
									            y + fascent - h.font.GetAscent(),
									            h.chr, (cjk + 1) * fsz.cx, h.font, h.ink);
							}
							q++;
							gp += 1 + cjk;
							if(x > sz.cx)
								break;
						}
					}
				}
				if(pass == 0) {
					int gpx = gp * fsz.cx - scx;
					rw.DrawRect(gpx, y, sz.cx - gpx, fsz.cy,
					            !rectsel && sell <= len && len < selh ? color[PAPER_SELECTED]
					            : (do_highlight ? hl.Top() : ih).paper);
					if(bordercolumn > 0 && bordercolumn >= gp)
						rw.DrawRect((bordercolumn - sc.x) * fsz.cx, y, 1, fsz.cy, bordercolor);
					if((showlines || warn_whitespace)) {
						int yy = 2 * fsz.cy / 3;
						int x = (lgp >= 0 ? lgp : gp) * fsz.cx - scx;
						rw.DrawRect(x, y + yy, fsz.cx / 2, 1, showcolor);
						if(fsz.cx > 2)
							rw.DrawRect(x + 1, y + yy - 1, 1, 3, showcolor);
						if(fsz.cx > 5)
							rw.DrawRect(x + 2, y + yy - 2, 1, 5, showcolor);
						rw.DrawRect(x + fsz.cx / 2, y + yy / 2, 1, yy - yy / 2, showcolor);
					}
					if(sell == selh) {
						if(!IsNull(hline) && i == cursorline) {
							rw.DrawRect(0, y, sz.cx, 1, hline);
							rw.DrawRect(0, y + fsz.cy - 1, sz.cx, 1, hline);
						}
						if(!IsNull(vline))
							rw.DrawRect(caretpos.x, y, 1, fsz.cy, vline);
					}
					if(rectsel && rect.left == rect.right && i >= rect.top && i <= rect.bottom)
						rw.DrawRect(rect.left * fsz.cx - scx, y, 2, fsz.cy, Blend(color[PAPER_SELECTED], color[PAPER_NORMAL]));
				}
			}
		}
		y += fsz.cy;
		sell -= len + 1;
		selh -= len + 1;
		pos += len + 1;
	}
	w.DrawRect(0, y, sz.cx, sz.cy - y, color[IsReadOnly() && showreadonly || !IsShowEnabled() ? PAPER_READONLY : PAPER_NORMAL]);
	DrawTiles(w, DropCaret(), CtrlImg::checkers());
	vlinex = caretpos.x;
}

void LineEdit::Paint(Draw& w)
{
	Paint0(w);
	scroller.Set(sb);
}

struct LineEdit::RefreshDraw : public NilDraw {
	Ctrl  *ctrl;
	bool (*chars)(int c);
	Size   fsz;
	virtual void DrawTextOp(int x, int y, int angle, const wchar *text, Font,
	                        Color, int n, const int *dx) {
		if(dx)
			while(n > 0) {
				if((*chars)(*text))
					ctrl->Refresh(x, y, fsz.cx, fsz.cy);
				text++;
				x += *dx++;
				n--;
			}
	}
	bool IsPaintingOp(const Rect& r) const {
		return true;
	}
};

void LineEdit::RefreshChars(bool (*chars)(int c))
{
	RefreshDraw rw;
	rw.ctrl = this;
	rw.fsz = GetFontSize();
	rw.chars = chars;
	Paint(rw);
}

void   LineEdit::Layout() {
	Size sz = sb.GetReducedViewSize();
	if(nohbar || isdrag)
		sz.cy = GetSize().cy;
	sb.SetPage(sz / GetFontSize());
	SetHBar();
}

int64  LineEdit::GetGPos(int ln, int cl) const {
	ln = minmax(ln, 0, GetLineCount() - 1);
	String h = GetUtf8Line(ln);
	const char *s = h.begin();
	const char *e = h.end();
	const char *b = s;
	int gl = 0;
	int wpos = 0;
	while(s < e) {
		if(*s == '\t')
			gl = (gl + tabsize) / tabsize * tabsize;
		else
		if((byte)*s < 128)
			gl++;
		else {
			WString txt = FromUtf8(s, int(e - s));
			const wchar *b = txt;
			const wchar *e = txt.End();
			const wchar *s = b;
			while(s < e) {
				if(*s == '\t')
					gl = (gl + tabsize) / tabsize * tabsize;
				else
					gl += 1 + IsDoubleWidth(*s);
				if(cl < gl) break;
				s++;
			}
			wpos = int(s - b);
			break;
		}
		if(cl < gl) break;
		s++;
	}
	
	return GetPos64(ln, int(s - b) + wpos);
}

Point LineEdit::GetColumnLine(int64 pos) const {
	Point p;
	if(pos > GetLength64()) pos = GetLength64();
	p.y = GetLinePos64(pos);
	p.x = 0;
	WString txt = GetWLine(p.y);
	const wchar *s = txt;
	while(pos--) {
		if(*s == '\t')
			p.x = (p.x + tabsize) / tabsize * tabsize;
		else
			p.x += 1 + IsDoubleWidth(*s);
		s++;
	}
	return p;
}

Point LineEdit::GetIndexLine(int64 pos) const
{
	Point p;
	if(pos > GetLength64()) pos = GetLength64();
	p.y = GetLinePos64(pos);
	p.x = minmax((int)pos, 0, GetLineLength(p.y));
	return p;
}

int64 LineEdit::GetIndexLinePos(Point pos) const
{
	if(pos.y < 0)
		return 0;
	if(pos.y >= GetLineCount())
		return GetLength64();
	return GetPos64(pos.y, minmax(pos.x, 0, GetLineLength(pos.y)));
}

void LineEdit::RefreshLine(int i) {
	Size sz = GetSize();
	int fcy = GetFontSize().cy;
	Refresh(0, (i - sb.Get().y) * fcy, sz.cx, fcy);
}

Rect LineEdit::GetLineScreenRect(int line) const {
	int fcy = GetFontSize().cy;
	Rect r = RectC(0, (line - sb.Get().y) * fcy, GetSize().cx, fcy);
	r.Offset(GetScreenView().TopLeft());
	return r;
}

void LineEdit::SetSb() {
	sb.SetTotalY(GetLineCount());
	SetHBar();
}

void LineEdit::NewScrollPos() {}
void LineEdit::HighlightLine(int line, Vector<Highlight>& h, int64 pos) {}

void LineEdit::AlignChar() {
	int c = GetCursor32();
	if(c == 0)
		return;
	Point pos = GetColumnLine(c);
	if(pos.x == 0)
		return;
	for(int d = 1; d <= pos.y && d < 100; d++) {
		int lny = pos.y - d;
		WString above = GetWLine(lny);
		int offset = int(GetGPos(lny, pos.x) - GetPos64(lny));
		int end = offset;
		char ch = GetChar(c - 1);
		if(ch == ' ')
		{
			offset++;
			while(end < above.GetLength() && above[end] != ' ')
				end++;
			while(end < above.GetLength() && above[end] == ' ')
				end++;
		}
		else
			while(end < above.GetLength() && above[end] != ch)
				end++;
		if(end < above.GetLength()) {
			int count = end - offset + 1;
			WString s(' ', count);
			Insert(c - 1, s, true);
			SetCursor(c + count);
			return;
		}
	}
}

void LineEdit::PlaceCaret0(Point p) {
	Size fsz = GetFontSize();
	p -= sb;
	caretpos = Point(p.x * fsz.cx, p.y * fsz.cy);
	if(overwrite)
		SetCaret(caretpos.x, caretpos.y + fsz.cy - 2, fsz.cx, 2);
	else
		SetCaret(caretpos.x, caretpos.y, 2, fsz.cy);
}

int LineEdit::PlaceCaretNoG(int64 newcursor, bool sel) {
	if(newcursor > GetLength64()) newcursor = GetLength64();
	Point p = GetColumnLine(newcursor);
	if(sel) {
		if(anchor < 0) {
			anchor = cursor;
		}
		if(rectsel || rectsel != dorectsel)
			Refresh();
		else
			RefreshLines(p.y, GetLine(cursor));
		rectsel = dorectsel;
	}
	else {
		if(anchor >= 0) {
			if(rectsel || dorectsel)
				Refresh();
			else
				RefreshLines(GetLine(cursor), GetLine(anchor));
			anchor = -1;
		}
		rectsel = false;
	}
	RefreshLine(GetColumnLine(cursor).y);
	RefreshLine(p.y);
	cursor = newcursor;
	ScrollIntoCursor();
	PlaceCaret0(p);
	SelectionChanged();
	WhenSel();
	if(IsAnySelection())
		SetSelectionSource(ClipFmtsText());
	if(!IsNull(hline)) {
		Size sz = GetSize();
		Refresh(vlinex, 0, 1, sz.cy);
		Refresh(caretpos.x, 0, 1, sz.cy);
	}
	return p.x;
}

void LineEdit::PlaceCaret(int64 newcursor, bool sel) {
	gcolumn = PlaceCaretNoG(newcursor, sel);
}

void LineEdit::TopCursor(int lines)
{
	sb.SetY(max(0, GetLine(cursor) - lines));
}

void LineEdit::CenterCursor() {
	int cy = sb.GetPage().cy;
	if(cy > 4)
		sb.SetY(max(min(GetLine(cursor) - cy / 2, GetLineCount() - cy), 0));
}

void LineEdit::Scroll() {
	PlaceCaret0(GetColumnLine(cursor));
	scroller.Scroll(*this, GetSize(), sb.Get(), GetFontSize());
	SetHBar();
	NewScrollPos();
	WhenScroll();
}

int64 LineEdit::GetMousePos(Point p) const {
	Size fsz = GetFontSize();
	p = (p + fsz.cx / 2 + fsz * (Size)sb.Get()) / fsz;
	return GetGPos(p.y, p.x);
}

void LineEdit::LeftDown(Point p, dword flags) {
	mpos = GetMousePos(p);
	int64 l, h;
	if(GetSelection(l, h) && mpos >= l && mpos < h) {
		selclick = true;
		return;
	}
	dorectsel = flags & K_ALT;
	PlaceCaret(mpos, (flags & K_SHIFT) || dorectsel);
	dorectsel = false;
	SetFocus();
	SetCapture();
}

void LineEdit::LeftUp(Point p, dword flags)
{
	if(!HasCapture() && selclick && !IsDragAndDropSource()) {
		mpos = GetMousePos(p);
		PlaceCaret(mpos, flags & K_SHIFT);
		SetFocus();
	}
	selclick = false;
	ReleaseCapture();
}

void LineEdit::RightDown(Point p, dword flags)
{
	mpos = GetMousePos(p);
	SetFocus();
	int64 l, h;
	GetSelection(l, h);
	if(!IsAnySelection() || !(mpos >= l && mpos < h))
		PlaceCaret(mpos, false);
	MenuBar::Execute(WhenBar);
}

void LineEdit::LeftDouble(Point, dword)
{
	int64 l, h;
	if(GetWordSelection(cursor, l, h))
		SetSelection(l, h);
}

void LineEdit::LeftTriple(Point, dword)
{
	int64 q = cursor;
	int i = GetLinePos64(q);
	q = cursor - q;
	SetSelection(q, q + GetLineLength(i) + 1);
}

void LineEdit::MouseMove(Point p, dword flags) {
	if((flags & K_MOUSELEFT) && HasFocus() && HasCapture()) {
		int64 c = GetMousePos(p);
		dorectsel = flags & K_ALT;
		PlaceCaret(c, mpos != c || HasCapture());
		dorectsel = false;
	}
}

void LineEdit::LeftRepeat(Point p, dword flags) {
	if(HasCapture()) {
		int64 c = GetMousePos(p);
		if(mpos != c) {
			dorectsel = flags & K_ALT;
			PlaceCaret(c, true);
			dorectsel = false;
		}
	}
}

Image LineEdit::CursorImage(Point, dword) {
	return Image::IBeam();
}

void LineEdit::MoveUpDown(int n, bool sel) {
	int64 cl = cursor;
	int ln = GetLinePos64(cl);
	if(ln + n >= GetLineCount())
		WaitView(ln + n);
	ln = minmax(ln + n, 0, GetLineCount() - 1);
	PlaceCaretNoG(GetGPos(ln, gcolumn), sel);
}

void LineEdit::MoveLeft(bool sel) {
	if(cursor)
		PlaceCaret(cursor - 1, sel);
}

void LineEdit::MoveRight(bool sel) {
	if(cursor < GetLength64())
		PlaceCaret(cursor + 1, sel);
}

void LineEdit::MoveUp(bool sel) {
	MoveUpDown(-1, sel);
}

void LineEdit::MoveDown(bool sel) {
	MoveUpDown(1, sel);
}

void LineEdit::MovePage(int dir, bool sel) {
	int n = dir * max(GetSize().cy / GetFontSize().cy - 2, 2);
	sb.SetY(Point(sb).y + n);
	MoveUpDown(n, sel);
}

void LineEdit::MovePageUp(bool sel) {
	MovePage(-1, sel);
}

void LineEdit::MovePageDown(bool sel) {
	MovePage(1, sel);
}

inline bool sTabSpace(int c) { return c == '\t' || c == ' '; }

void LineEdit::MoveHome(bool sel) {
	int64 cl = cursor;
	int li = GetLinePos64(cl);
	int i = 0;
	WString l = GetWLine(li);
	while(sTabSpace(l[i]))
		i++;
	PlaceCaret(GetPos64(li, cl == i ? 0 : i), sel);
}

void LineEdit::MoveEnd(bool sel) {
	int i = GetLine(cursor);
	PlaceCaret(GetPos64(i, GetLineLength(i)), sel);
}

void LineEdit::MoveTextBegin(bool sel) {
	PlaceCaret(0, sel);
}

void LineEdit::MoveTextEnd(bool sel) {
	WaitView(INT_MAX, true);
	PlaceCaret(GetLength64(), sel);
}

bool LineEdit::InsertChar(dword key, int count, bool canow) {
	if(key == K_TAB && !processtab)
		return false;
	if(filter && key >= 32 && key < 65535)
		key = (*filter)(key);
	if(!IsReadOnly() && (key >= 32 && key < 65536 || key == '\t' || key == '\n' ||
	   key == K_ENTER && processenter || key == K_SHIFT_SPACE)) {
		if(key >= 128 && key < 65536 && (charset != CHARSET_UNICODE && charset != CHARSET_UTF8_BOM)
		   && FromUnicode((wchar)key, charset) == DEFAULTCHAR)
			return true;
		if(!RemoveSelection() && overwrite && key != '\n' && key != K_ENTER && canow) {
			int64 q = cursor;
			int i = GetLinePos64(q);
			if(q + count - 1 < GetLineLength(i))
				Remove((int)cursor, (int)count);
		}
		WString text(key == K_ENTER ? '\n' : key == K_SHIFT_SPACE ? ' ' : key, count);
		Insert((int)cursor, text, true);
		PlaceCaret(cursor + count);
		Action();
		return true;
	}
	return false;
}

void LineEdit::DeleteChar() {
	if(IsReadOnly() || RemoveSelection()) {
		Action();
		return;
	}
	if(cursor < GetLength32()) {
		Remove((int)cursor, 1);
		Action();
	}
}

void LineEdit::Backspace() {
	if(IsReadOnly() || RemoveSelection() || cursor == 0) return;
	MoveLeft();
	DeleteChar();
	Action();
}

void LineEdit::DeleteLine()
{
	int64 b, e;
	if(GetSelection(b, e) && GetLine(b) != GetLine(e)) {
		RemoveSelection();
		return;
	}
	int i = GetLine(cursor);
	int p = GetPos32(i);
	Remove((int)p, GetLineLength(i) + 1);
	PlaceCaret(p);
	Action();
}

void LineEdit::CutLine()
{
	if(IsReadOnly()) return;
	int64 b, e;
	if(GetSelection(b, e) && GetLine(b) != GetLine(e)) {
		Cut();
		return;
	}
	int i = GetLine(cursor);
	int p = GetPos32(i);
	WString txt = Get(p, GetLineLength(i) + 1).ToWString();
	WriteClipboardUnicodeText(txt);
	AppendClipboardText(txt.ToString());
	ClearSelection();
	DeleteLine();
}

LineEdit::EditPos LineEdit::GetEditPos() const {
	EditPos pos;
	pos.sby = sb.Get().y;
	pos.cursor = cursor;
	return pos;
}

void LineEdit::SetEditPos(const LineEdit::EditPos& pos) {
	SetEditPosSbOnly(pos);
	SetCursor(pos.cursor);
}

void LineEdit::SetEditPosSb(const LineEdit::EditPos& pos) {
	SetCursor(pos.cursor);
	SetEditPosSbOnly(pos);
}

void LineEdit::SetEditPosSbOnly(const LineEdit::EditPos& pos) {
	sb.SetY(minmax(pos.sby, 0, GetLineCount() - 1));
}

void LineEdit::SetHBar()
{
	int mpos = 0;
	if(!nohbar && !isdrag) {
		int m = min(sb.y + sb.GetPage().cy + 2, GetLineCount());
		for(int i = sb.y; i < m; i++) {
			int pos = 0;
			WString l = GetWLine(i);
			const wchar *s = l;
			const wchar *e = l.End();
			while(s < e) {
				if(*s == '\t')
					pos = (pos + tabsize) / tabsize * tabsize;
				else
					pos += 1 + IsDoubleWidth(*s);
				s++;
			}
			mpos = max(mpos, pos);
		}
	}
	sb.SetTotalX(mpos + 1);
}

void LineEdit::ScrollIntoCursor()
{
	Point p = GetColumnLine(GetCursor64());
	sb.ScrollInto(p);
	SetHBar();
	sb.ScrollInto(p);
}

bool LineEdit::Key(dword key, int count) {
	NextUndo();
	switch(key) {
	case K_CTRL_UP:
		ScrollUp();
		return true;
	case K_CTRL_DOWN:
		ScrollDown();
		return true;
	case K_INSERT:
		OverWriteMode(!IsOverWriteMode());
		break;
	}
	bool sel = key & K_SHIFT;
	dorectsel = key & K_ALT;
	dword k = key & ~K_SHIFT;
	if((key & (K_SHIFT|K_ALT)) == (K_SHIFT|K_ALT))
		k &= ~K_ALT;
	switch(k) {
	case K_CTRL_LEFT:
		{
			PlaceCaret(GetPrevWord(cursor), sel);
			break;
		}
	case K_CTRL_RIGHT:
		{
			PlaceCaret(GetNextWord(cursor), sel);
			break;
		}
	case K_LEFT:
		MoveLeft(sel);
		break;
	case K_RIGHT:
		MoveRight(sel);
		break;
	case K_HOME:
		MoveHome(sel);
		break;
	case K_END:
		MoveEnd(sel);
		break;
	case K_UP:
		MoveUp(sel);
		break;
	case K_DOWN:
		MoveDown(sel);
		break;
	case K_PAGEUP:
		MovePageUp(sel);
		break;
	case K_PAGEDOWN:
		MovePageDown(sel);
		break;
	case K_CTRL_PAGEUP:
	case K_CTRL_HOME:
		MoveTextBegin(sel);
		break;
	case K_CTRL_PAGEDOWN:
	case K_CTRL_END:
		MoveTextEnd(sel);
		break;
	case K_CTRL_C:
	case K_CTRL_INSERT:
		Copy();
		break;
	case K_CTRL_A:
		SelectAll();
		break;
	default:
		dorectsel = false;
		if(IsReadOnly())
			return MenuBar::Scan(WhenBar, key);
		switch(key) {
		case K_DELETE:
			DeleteChar();
			break;
		case K_BACKSPACE:
		case K_SHIFT|K_BACKSPACE:
			Backspace();
			break;
		case K_SHIFT_TAB:
			AlignChar();
			break;
		case K_CTRL_Y:
		case K_CTRL_L:
			if(cutline) {
				CutLine();
				break;
			}
		default:
			if(InsertChar(key, count, true))
				return true;
			return MenuBar::Scan(WhenBar, key);
		}
		return true;
	}
	dorectsel = false;
	Sync();
	return true;
}

void LineEdit::DragAndDrop(Point p, PasteClip& d)
{
	if(IsReadOnly()) return;
	int c = GetMousePos32(p);
	if(AcceptText(d)) {
		NextUndo();
		int a = sb.y;
		int sell, selh;
		WString text = GetWString(d);
		if(GetSelection32(sell, selh)) {
			if(c >= sell && c < selh) {
				if(!IsReadOnly())
					RemoveSelection();
				if(IsDragAndDropSource())
					d.SetAction(DND_COPY);
				c = sell;
			}
			else
			if(d.GetAction() == DND_MOVE && IsDragAndDropSource()) {
				if(c > sell)
					c -= selh - sell;
				if(!IsReadOnly())
					RemoveSelection();
				d.SetAction(DND_COPY);
			}
		}
		int count = Insert(c, text);
		sb.y = a;
		SetFocus();
		SetSelection(c, c + count);
		Action();
		return;
	}
	if(!d.IsAccepted()) return;
	if(!isdrag) {
		isdrag = true;
		ScrollIntoCursor();
	}
	Point dc = Null;
	if(c >= 0)
		dc = GetColumnLine(c);
	if(dc != dropcaret) {
		RefreshDropCaret();
		dropcaret = dc;
		RefreshDropCaret();
	}
}

Rect LineEdit::DropCaret()
{
	if(IsNull(dropcaret))
		return Rect(0, 0, 0, 0);
	Size fsz = GetFontSize();
	Point p = dropcaret - sb;
	p = Point(p.x * fsz.cx, p.y * fsz.cy);
	return RectC(p.x, p.y, 1, fsz.cy);
}

void LineEdit::RefreshDropCaret()
{
	Refresh(DropCaret());
}

void LineEdit::DragRepeat(Point p)
{
	sb.y = (int)sb.y + GetDragScroll(this, p, 1).y;
}

void LineEdit::DragLeave()
{
	RefreshDropCaret();
	dropcaret = Null;
	isdrag = false;
	Layout();
}

void LineEdit::LeftDrag(Point p, dword flags)
{
	int64 c = GetMousePos(p);
	int64 l, h;
	if(!HasCapture() && GetSelection(l, h) && c >= l && c < h) {
		WString sample = GetW(l, (int)min(h - l, (int64)3000));
		Size sz = StdSampleSize();
		ImageDraw iw(sz);
		iw.DrawRect(sz, Black());
		iw.Alpha().DrawRect(sz, Black());
		DrawTLText(iw.Alpha(), 0, 0, 9999, sample, CourierZ(10), White());
		NextUndo();
		if(DoDragAndDrop(ClipFmtsText(), iw) == DND_MOVE && !IsReadOnly()) {
			RemoveSelection();
			Action();
		}
	}
}

}
