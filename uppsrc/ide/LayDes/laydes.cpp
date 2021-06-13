#include "LayDes.h"

using namespace LayoutKeys;

#define IMAGECLASS LayImg
#define IMAGEFILE  <ide/LayDes/LayDes.iml>
#include <Draw/iml_source.h>

#define LTIMING(x) // TIMING(x)
#define LLOG(x)

#define MARGIN 8

static void sLay1(int& pos, int& r, int align, int a, int b, int sz)
{
	pos = a;
	int size = b;
	switch(align) {
	case Ctrl::CENTER:
		pos = (sz - b) / 2 + a;
		break;
	case Ctrl::RIGHT:
		pos = sz - (a + b);
		break;
	case Ctrl::SIZE:
		size = sz - (a + b);
		break;
	}
	r = pos + max(size, 0);
}

Rect LayDes::CtrlRect(Ctrl::LogPos pos, Size sz)
{
	Rect r;
	sLay1(r.left, r.right, pos.x.GetAlign(), pos.x.GetA(), pos.x.GetB(), sz.cx);
	sLay1(r.top, r.bottom, pos.y.GetAlign(), pos.y.GetA(), pos.y.GetB(), sz.cy);
	return r;
}

Rect LayDes::CtrlRectZ(Ctrl::LogPos pos, Size sz)
{
	Rect r = CtrlRect(pos, sz);
	r.left = HorzLayoutZoom(r.left);
	r.right = HorzLayoutZoom(r.right);
	r.top = VertLayoutZoom(r.top);
	r.bottom = VertLayoutZoom(r.bottom);
	return r;
}

void LayDes::AddHandle(Draw& w, int x, int y)
{
	w.DrawRect(x, y, DPI(6), DPI(6), SColorMark);
	handle.Add(Point(x, y));
}

Point LayDes::Normalize(Point p)
{
	p += sb;
	p.Offset(-MARGIN, -MARGIN);
	return p;
}

Point LayDes::ZPoint(Point p)
{
	Size csz, dsz;
	GetZoomRatio(csz, dsz);
	if(csz.cx && csz.cy && dsz.cx && dsz.cy) {
		p.x = p.x * dsz.cx / csz.cx;
		p.y = p.y * dsz.cy / csz.cy;
	}
	return p;
}

void LayDes::SetSb()
{
	Size sz = Size(0, 0);
	if(!IsNull(currentlayout)) {
		LayoutData& l = CurrentLayout();
		sz = l.size;
		for(int i = 0; i < l.item.GetCount(); i++)
			sz = max(sz, (Size)CtrlRect(l.item[i].pos, l.size).BottomRight());
	}
	sz += Size(MARGIN, MARGIN);
	Size csz, dsz;
	GetZoomRatio(csz, dsz);
	if(csz.cx && csz.cy && dsz.cx && dsz.cy) {
		sz.cx = sz.cx * csz.cx / dsz.cx;
		sz.cy = sz.cy * csz.cy / dsz.cy;
	}
	sb.SetTotal(sz);
	sb.SetPage(sb.GetReducedViewSize());
}

void LayDes::Scroll()
{
	Refresh();
}

void LayDes::Layout()
{
	SetSb();
	list.ScrollIntoCursor();
}

void LayDes::GetSprings(Rect& l, Rect& t, Rect& r, Rect& b)
{
	if(IsNull(currentlayout) || !cursor.GetCount()) {
		l = t = r = b = Null;
		return;
	}
	Rect ir = CtrlRectZ(CurrentItem().pos, CurrentLayout().size);
	int h4 = ir.Height() / 4;
	int w4 = ir.Width() / 4;
	int fv = DPI(5);
	int tn = DPI(10);
	l = RectC(-MARGIN, ir.top + h4 - fv, ir.left + MARGIN, tn);
	t = RectC(ir.left + w4 - fv, -MARGIN, tn, ir.top + MARGIN);
	r = RectC(ir.right, ir.bottom - h4 - fv, 9999, tn);
	b = RectC(ir.right - w4 - fv, ir.bottom, tn, 9999);
}

void LayDes::DrawSpring(Draw& w, const Rect& r, bool horz, bool hard)
{
	if(hard)
		w.DrawRect(horz ? r.DeflatedVert(4) : r.DeflatedHorz(4), Red);
	else
		if(horz)
			for(int x = r.left; x < r.right; x += 4)
				w.DrawRect(x, r.top, 1, r.Height(), SColorShadow);
		else
			for(int y = r.top; y < r.bottom; y += 4)
				w.DrawRect(r.left, y, r.Width(), 1, SColorShadow);
}

int LayDes::ParseLayoutRef(String cls, String& base) const
{
	const char *p = cls;
	if(p[0] != 'W' || p[1] != 'i' || p[2] != 't' || p[3] != 'h')
		return -1;
	const char *b = (p += 4);
	while(IsAlNum(*p) || *p == '_')
		p++;
	if(p <= b)
		return -1;
	String layoutid(b, p);
	int fi = FindFieldIndex(layout, &LayoutData::name, layoutid);
	if(fi < 0)
		return -1;
	while(*p && (byte)*p <= ' ')
		p++;
	if(*p++ != '<')
		return -1;
	while(*p && (byte)*p <= ' ')
		p++;
	const char *e = cls.End();
	while(e > p && (byte)e[-1] <= ' ')
		e--;
	if(e > p && e[-1] == '>')
		e--;
	while(e > p && (byte)e[-1] <= ' ')
		e--;
	base = String(p, e);
	return fi;
}

void LayDes::PaintLayoutItems(Draw& w, int layid, Size size, Index<int>& passed, const Vector<bool>& cursor)
{
	if(passed.Find(layid) >= 0)
		return;
	passed.Add(layid);
	LayoutData& l = layout[layid];
	Buffer<int> lrs(l.item.GetCount());
	String dummy;
	for(int i = 0; i < l.item.GetCount(); i++)
		lrs[i] = ParseLayoutRef(l.item[i].type, dummy);
	for(int i = 0; i < l.item.GetCount(); i++) {
		LayoutItem& m = l.item[i];
		if(m.hide)
			continue;
		Rect r = CtrlRectZ(m.pos, size);
		DrawFrame(w, r, Blend(SLtGray(), SCyan(), 35));
		bool show = true;
		if(i < cursor.GetCount() && !cursor[i] && m.type != "LabelBox") // does this widget hide part of some widget that is current selected?
			for(int j = 0; j < l.item.GetCount(); j++)
				if(j != i && j < cursor.GetCount() && cursor[j]) {
					LayoutItem& mm = l.item[j];
					if(!mm.hide && r.Intersects(CtrlRectZ(mm.pos, size)) && (LayoutTypes().Find(mm.type) >= 0 || lrs[j] >= 0)
					   && mm.type != "LabelBox" && HasCapture() && findarg(draghandle, 11, 12) >= 0) {
						show = false;
						break;
					}
				}
		
		if(show) {
			w.Clipoff(r);
			if(lrs[i] < 0)
				m.Paint(w, r.Size());
			else {
				Size sz = r.GetSize();
				PaintLayoutItems(w, lrs[i], CtrlRect(m.pos, size).GetSize(), passed, Vector<bool>());
				Font fnt = LayFont();
				String s = m.type;
				Size tsz = GetTextSize(s, fnt);
				int tcy2 = tsz.cy / 2;
				if(tsz.cx + tcy2 > sz.cx && s.TrimStart("With"))
					tsz = GetTextSize(s, fnt);
				int q = s.Find('<');
				if(tsz.cx + tcy2 > sz.cx && q >= 0) {
					s.Trim(q);
					tsz = GetTextSize(s, fnt);
				}
				if(tsz.cx + tcy2 > sz.cx && s.TrimEnd("Layout"))
					tsz = GetTextSize(s, fnt);
				w.DrawRect(sz.cx - tsz.cx - tsz.cy, sz.cy - tsz.cy, tsz.cx + tsz.cy, tsz.cy, SGray());
				w.DrawText(sz.cx - tsz.cx - tsz.cy / 2, sz.cy - tsz.cy, s, fnt, SYellow());
			}
			w.End();
		}
	}
	passed.Drop();
}

void LayDes::Paint(Draw& w)
{
	LTIMING("Paint");
	Size sz = GetSize();
	w.DrawRect(sz, SColorPaper);
	if(!IsNull(fileerror))
		w.DrawText(16, 16, "FILE ERROR: " + fileerror, ArialZ(14).Bold(), Red);
	if(IsNull(currentlayout))
		return;
	w.Offset(-sb.Get());
	LayoutData& l = CurrentLayout();
	w.Offset(MARGIN, MARGIN);
	Size lsz = LayoutZoom(l.size);
	w.DrawRect(0, 0, lsz.cx, lsz.cy, SLtGray);
	if(setting.paintgrid) {
		int gx = minmax((int)~setting.gridx, 1, 32);
		int gy = minmax((int)~setting.gridy, 1, 32);
		for(int x = 0; x < lsz.cx; x += gx)
			for(int y = 0; y < lsz.cy; y += gy)
				w.DrawRect(x, y, 1, 1, SColorPaper);
	}
	DrawFrame(w, -1, -1, lsz.cx + 2, lsz.cy + 2, SColorText);
	handle.Clear();
	AddHandle(w, lsz.cx, lsz.cy / 2 - DPI(3));
	AddHandle(w, lsz.cx / 2 - DPI(3), lsz.cy);
	AddHandle(w, lsz.cx, lsz.cy);
	int i;
	Index<int> passed;
	Vector<bool> cursorflags;
	if(!cursor.IsEmpty()) {
		cursorflags.SetCount(l.item.GetCount(), cursor.IsEmpty());
		for(i = 0; i < cursor.GetCount(); i++)
			cursorflags[cursor[i]] = true;
	}
	PaintLayoutItems(w, currentlayout, l.size, passed, cursorflags);
	if(!HasCapture() || draghandle == 14) {
		for(i = 0; i < cursor.GetCount(); i++) {
			LayoutItem& m = l.item[cursor[i]];
			Rect r = CtrlRectZ(m.pos, l.size);
			DrawFatFrame(w, r, i == cursor.GetCount() - 1 ? Cyan : Brown, DPI(-3));
			if(i == cursor.GetCount() - 1) {
				int lrm = r.left + r.Width() / 2 - DPI(3);
				int tbm = r.top + r.Height() / 2 - DPI(3);
				AddHandle(w, r.left - DPI(6), r.top - DPI(6));
				AddHandle(w, lrm, r.top - DPI(6));
				AddHandle(w, r.right, r.top - DPI(6));
				AddHandle(w, r.left - DPI(6), tbm);
				AddHandle(w, r.right, tbm);
				AddHandle(w, r.left - DPI(6), r.bottom);
				AddHandle(w, lrm, r.bottom);
				AddHandle(w, r.right, r.bottom);
			}
		}
		if(cursor.GetCount()) {
			LayoutItem& m = CurrentItem();
			Rect l, t, r, b;
			GetSprings(l, t, r, b);
			DrawSpring(w, l, true, m.pos.x.GetAlign() & Ctrl::LEFT);
			DrawSpring(w, r, true, m.pos.x.GetAlign() & Ctrl::RIGHT);
			DrawSpring(w, t, false, m.pos.y.GetAlign() & Ctrl::TOP);
			DrawSpring(w, b, false, m.pos.y.GetAlign() & Ctrl::BOTTOM);
		}
	}
	if(HasCapture() && draghandle == 14)
		DrawFrame(w, dragrect.Normalized(), LtRed);
	w.End();
}

void  LayDes::SaveState()
{
	if(IsNull(currentlayout))
		return;
	CurrentLayout().SaveState();
	SetBar();
}

void  LayDes::SetStatus(bool down)
{
	String s;
	if(!IsNull(currentlayout)) {
		Size sz = CurrentLayout().size;
		s << sz;
		if(cursor.GetCount()) {
			Rect r = CtrlRect(CurrentItem().pos, sz);
			s << ": " << r << " - {" << sz.cx - r.right << ", " << sz.cy - r.bottom << '}';
		}
	}
	status.SetLabel(s);
	Refresh();
	SetBar();
	SetSb();
}

int   LayDes::FindHandle(Point p)
{
	for(int i = 0; i < handle.GetCount(); i++) {
		Point h = handle[i];
		if(p.x >= h.x - DPI(1) && p.x < h.x + DPI(7) && p.y >= h.y - DPI(1) && p.y < h.y + DPI(7))
			return i;
	}
	return -1;
}

int   LayDes::FindItem(Point p, bool cursor_first)
{
	if(IsNull(currentlayout))
		return -1;
	LayoutData& l = CurrentLayout();
	int ii = -1;
	int min = INT_MAX;
	for(int pass = !cursor_first; pass < 2 && ii < 0; pass++)
		for(int i = 0; i < (pass ? l.item.GetCount() : cursor.GetCount()); i++) {
			int i2 = pass ? i : cursor[i];
			LayoutItem& m = l.item[i2];
			if(m.hide)
				continue;
			Rect r = CtrlRect(m.pos, l.size);
			if(r.Contains(p)) {
				int mm = r.Width() * r.Height();
				if(mm < min) {
					ii = i2;
					min = mm;
				}
			}
		}
	return ii;
}

Image LayDes::CursorImage(Point p, dword keyflags)
{
	int hi;
	if(HasCapture())
		hi = draghandle;
	else
		hi = FindHandle(Normalize(p));
	Image (*id[11])() = {
		Image::SizeHorz, Image::SizeVert, Image::SizeBottomRight,
		Image::SizeTopLeft, Image::SizeVert, Image::SizeTopRight,
		Image::SizeHorz, Image::SizeHorz,
		Image::SizeBottomLeft, Image::SizeVert, Image::SizeBottomRight,
	};
	if(hi >= 0 && hi < 11)
		return (*id[hi])();
	return Image::Arrow();
}

Ctrl::Logc MakeLogc(int align, int a, int b, int sz)
{
	int isz = b - a;
	switch(align) {
	case Ctrl::LEFT:
		return Ctrl::PosLeft(a, isz);
	case Ctrl::RIGHT:
		return Ctrl::PosRight(sz - b, isz);
	case Ctrl::CENTER:
		return Ctrl::PosCenter(isz, a - (sz - isz) / 2);
	}
	return Ctrl::PosSize(a, sz - b);
}

Ctrl::LogPos MakeLogPos(int ax, int ay, const Rect& r, Size sz)
{
	return Ctrl::LogPos(MakeLogc(ax, r.left, r.right, sz.cx),
	                    MakeLogc(ay, r.top, r.bottom, sz.cy));
}

Ctrl::LogPos MakeLogPos(Ctrl::LogPos p, const Rect& r, Size sz)
{
	return MakeLogPos(p.x.GetAlign(), p.y.GetAlign(), r, sz);
}

struct IDisplay : public Display {
	Color paper, ink;
	Font  font;
	
	Size GetStdSize(const Value& q) const {
		Size sz = GetSmartTextSize(~q, font);
		sz.cx += 2 * DPI(4);
		return sz;
	}

	void Paint(Draw& w, const Rect& r, const Value& q,
	           Color, Color, dword s) const {
		w.DrawRect(r, paper);
		DrawSmartText(w, r.left + DPI(4), r.top, r.Width(), ~q, font, ink);
	}
	IDisplay(Color paper, Color ink, Font font = StdFont())
		: paper(paper), ink(ink), font(font) {}
};

struct TDisplay : public Display {
	Color paper, ink;
	Font  font;

	Size GetStdSize(const Value& q) const {
		Size sz = GetSmartTextSize(~q, font);
		sz.cx += 2 * DPI(4) + DPI(6) + sz.cy * 8 / 3;
		return sz;
	}

	void Paint(Draw& w, const Rect& r, const Value& q,
	           Color, Color, dword s) const {
		w.DrawRect(r, paper);
		int dx = r.Height() * 8 / 3;
		Image icon = GetTypeIcon(String(q), dx, r.Height(), 1, paper);
		w.DrawImage(r.left + DPI(4), r.top + (r.Height() - icon.GetSize().cy) / 2, icon);
		DrawSmartText(w, r.left + dx + DPI(6), r.top, r.Width(), ~q, font, ink);
	}
	TDisplay(Color paper, Color ink, Font font = StdFont())
		: paper(paper), ink(ink), font(font) {}
};

void LayDes::SyncItems()
{
	LTIMING("SyncItems");
	if(IsNull(currentlayout))
		return;
	int i;
	for(i = 0; i < item.GetCount(); i++)
		SyncItem(i, 0);
	for(i = 0; i < cursor.GetCount(); i++)
		SyncItem(cursor[i], 1);
	if(cursor.GetCount()) {
		SyncItem(cursor.Top(), 2);
		item.SetCursor(cursor.Top());
	}
	else
		item.KillCursor();
	SetStatus();
	SyncProperties(true);
}

void LayDes::SyncItem(int i, int style)
{
	if(CurrentLayout().item[i].hide) {
		static IDisplay dnormal(SColorPaper, SColorText, StdFont().Italic());
		static IDisplay dselect(SColorLtFace, SColorText, StdFont().Italic());
		static IDisplay dcursor(SColorFace, SColorText, StdFont().Italic());
		static TDisplay tnormal(SColorPaper, SColorText, StdFont().Italic());
		static TDisplay tselect(SColorLtFace, SColorText, StdFont().Italic());
		static TDisplay tcursor(SColorFace, SColorText, StdFont().Italic());
		static IDisplay lnormal(SColorPaper, Green, StdFont().Italic());
		static IDisplay lselect(SColorLtFace, Green, StdFont().Italic());
		static IDisplay lcursor(SColorFace, Green, StdFont().Italic());
		const Display *noicon[] = { &dnormal, &dselect, &dcursor };
		const Display *isicon[] = { &tnormal, &tselect, &tcursor };
		const Display *label[] = { &lnormal, &lselect, &lcursor };
		bool icons = setting.showicons;
		item.SetDisplay(i, 0, *(icons ? isicon : noicon)[style]);
		item.SetDisplay(i, 1, *(IsNull(CurrentLayout().item[i].variable) ? label : noicon)[style]);
	}
	else {
		static IDisplay dnormal(SColorPaper, SColorText);
		static IDisplay dselect(SColorLtFace, SColorText);
		static IDisplay dcursor(SColorFace, SColorText);
		static TDisplay tnormal(SColorPaper, SColorText);
		static TDisplay tselect(SColorLtFace, SColorText);
		static TDisplay tcursor(SColorFace, SColorText);
		static IDisplay lnormal(SColorPaper, Green);
		static IDisplay lselect(SColorLtFace, Green);
		static IDisplay lcursor(SColorFace, Green);
		const Display *noicon[] = { &dnormal, &dselect, &dcursor };
		const Display *isicon[] = { &tnormal, &tselect, &tcursor };
		const Display *label[] = { &lnormal, &lselect, &lcursor };
		bool icons = setting.showicons;
		item.SetDisplay(i, 0, *(icons ? isicon : noicon)[style]);
		item.SetDisplay(i, 1, *(IsNull(CurrentLayout().item[i].variable) ? label : noicon)[style]);
	}
}

void LayDes::SyncProperties(bool sync_type_var)
{
	property.Clear();
	if(sync_type_var) {
		type.Disable();
		variable.Disable();
		type <<= variable <<= Null;
	}
	if(cursor.GetCount()) {
		LayoutItem& m = CurrentItem();
		int i;
		for(i = 0; i < m.property.GetCount(); i++) {
			property.Add(m.property[i]);
			m.property[i].WhenAction = THISBACK(PropertyChanged);
		}
		if(sync_type_var) {
			String tp = m.type;
			type <<= m.type;
			for(i = 0; i < cursor.GetCount() - 1; i++)
				if(CurrentLayout().item[cursor[i]].type != tp) {
					type <<= Null;
					break;
				}
			variable <<= m.variable;
			type.Enable();
			variable.Enable();
		}
	}
}

void LayDes::SelectOne(int ii, dword flags)
{
	if(ii < 0) {
		if(flags & (K_SHIFT|K_CTRL))
			return;
		cursor.Clear();
	}
	else
	if(flags & (K_SHIFT|K_CTRL)) {
		int q = FindIndex(cursor, ii);
		if(q >= 0)
			cursor.Remove(q);
		else
			cursor.Add(ii);
	}
	else {
		cursor.Clear();
		cursor.Add(ii);
	}
	SyncItems();
}

void LayDes::StoreItemRects()
{
	if(IsNull(currentlayout))
		return;
	LayoutData& l = CurrentLayout();
	itemrect.SetCount(cursor.GetCount());
	for(int i = 0; i < cursor.GetCount(); i++)
		itemrect[i] = CtrlRect(l.item[cursor[i]].pos, l.size);
}

void  LayDes::LeftDown(Point p, dword keyflags)
{
	if(IsNull(currentlayout))
		return;
	SaveState();
	SetFocus();
	SetCapture();
	LayoutData& l = CurrentLayout();
	draglayoutsize = l.size;
	p = Normalize(p);
	draghandle = FindHandle(p);
	dragbase = ZPoint(p);
	if(draghandle >= 0)
		StoreItemRects();
	else {
		int ii = FindItem(dragbase, true);
		if(ii >= 0) {
			if(GetShift() || GetCtrl() || FindIndex(cursor, ii) < 0)
				SelectOne(ii, keyflags);
			if(cursor.GetCount()) {
				draghandle = 11;
				StoreItemRects();
				SetStatus();
			}
		}
		else {
			if(cursor.GetCount()) {
				LayoutItem& m = CurrentItem();
				StoreItemRects();
				Rect lr, tr, rr, br;
				GetSprings(lr, tr, rr, br);
				int xa = m.pos.x.GetAlign();
				int ya = m.pos.y.GetAlign();
				if(lr.Contains(p))
					xa ^= 1;
				if(rr.Contains(p))
					xa ^= 2;
				if(tr.Contains(p))
					ya ^= 1;
				if(br.Contains(p))
					ya ^= 2;
				if(xa != m.pos.x.GetAlign() || ya != m.pos.y.GetAlign()) {
					SetSprings(MAKELONG(xa, ya));
					SetStatus();
					return;
				}
			}
			dragrect.left = dragrect.right = p.x;
			dragrect.top = dragrect.bottom = p.y;
			draghandle = 14;
			if(GetCtrl() || GetShift())
				basesel = cursor.GetCount();
			else {
				basesel = 0;
				cursor.Clear();
			}
			SyncItems();
		}
	}
	SetStatus(true);
}

void  LayDes::LeftRepeat(Point p, dword keyflags)
{
	MouseMove(p, keyflags);
}

void  LayDes::MouseMove(Point p, dword keyflags)
{
	if(!HasCapture() || IsNull(currentlayout))
		return;
	Point pz = Normalize(p);
	p = ZPoint(pz);
	LayoutData& l = CurrentLayout();
	bool smallmove = max(abs(p.x - dragbase.x), abs(p.y - dragbase.y)) < 4;
	if(draghandle == 14) {
		dragrect.right = pz.x;
		dragrect.bottom = pz.y;
		cursor.SetCount(basesel);
		Rect r = dragrect.Normalized();
		r = Rect(ZPoint(r.TopLeft()), ZPoint(r.BottomRight()));
		int mind = INT_MAX;
		int mini = -1;
		for(int i = 0; i < l.item.GetCount(); i++) {
			LayoutItem& m = l.item[i];
			Rect ir = CtrlRect(m.pos, l.size);
			if(r.Contains(ir) && FindIndex(cursor, i) < 0) {
				Point ip = ir.CenterPoint();
				int mm = (ip.x - dragrect.left) * (ip.x - dragrect.left)
				       + (ip.y - dragrect.top) * (ip.y - dragrect.top);
				if(mm < mind) {
					mind = mm;
					mini = cursor.GetCount();
				}
				cursor.Add(i);
			}
		}
		if(mini >= 0)
			Swap(cursor.Top(), cursor[mini]);
		if(cursor.GetCount())
			item.SetCursor(cursor.Top());
		else
			item.KillCursor();
		SyncItems();
		SetStatus(false);
		LTIMING("MouseMove Sync");
		Sync();
		return;
	}
	int gx = 1;
	int gy = 1;
	if(usegrid == !(keyflags & K_ALT)) {
		gx = minmax((int)~setting.gridx, 1, 32);
		gy = minmax((int)~setting.gridy, 1, 32);
	}
	p -= dragbase;
	if(draghandle < 3) {
		Vector<Rect> r;
		r.SetCount(l.item.GetCount());
		for(int i = 0; i < l.item.GetCount(); i++)
			r[i] = CtrlRect(l.item[i].pos, l.size);
		if((draghandle + 1) & 1)
			l.size.cx = max(0, draglayoutsize.cx + p.x) / gx * gx;
		if((draghandle + 1) & 2)
			l.size.cy = max(0, draglayoutsize.cy + p.y) / gy * gy;
		if(!sizespring)
			for(int i = 0; i < l.item.GetCount(); i++) {
				LayoutItem& m = l.item[i];
				m.pos = MakeLogPos(m.pos, r[i], l.size);
			}
		SetStatus(true);
		SetSb();
		Sync();
		return;
	}
	if(!item.IsCursor())
		return;
	if(draghandle == 11) {
		if(smallmove)
			return;
		draghandle = 12;
	}
	Point md = Point(0, 0);
	for(int i = 0; i < cursor.GetCount(); i++) {
		LayoutItem& m = l.item[cursor[i]];
		Rect r = itemrect[i];
		Size minsize = ignoreminsize ? Size(0, 0) : m.GetMinSize();
		if(keyflags & K_CTRL)
			minsize = Size(0, 0);
		if(draghandle == 3 || draghandle == 4 || draghandle == 5)
			r.top = min(r.bottom - minsize.cy, (r.top + p.y) / gy * gy);
		if(draghandle == 8 || draghandle == 9 || draghandle == 10)
			r.bottom = max(r.top + minsize.cy, (r.bottom + p.y) / gy * gy);
		if(draghandle == 3 || draghandle == 6 || draghandle == 8)
			r.left = min(r.right - minsize.cx, (r.left + p.x) / gy * gy);
		if(draghandle == 5 || draghandle == 7 || draghandle == 10)
			r.right = max(r.left + minsize.cx, (r.right + p.x) / gy * gy);
		if(draghandle == 12) {
			Size sz = r.GetSize();
			if(i == 0) {
				Rect q = r;
				r.left = (r.left + p.x) / gx * gx;
				r.top = (r.top + p.y) / gy * gy;
				md = r.TopLeft() - q.TopLeft();
			}
			else
				r += md;
			r.SetSize(sz);
		}
		m.pos = MakeLogPos(m.pos, r, draglayoutsize);
	}
	SetStatus(true);
	Sync();
}

void  LayDes::LeftUp(Point p, dword keyflags)
{
	if(draghandle == 11 && (keyflags & (K_SHIFT|K_CTRL)) == 0)
		SelectOne(FindItem(ZPoint(Normalize(p))), 0);
	draghandle = -1;
	SyncItems();
}

void LayDes::CreateCtrl(const String& _type)
{
	if(IsNull(currentlayout))
		return;
	LayoutData& l = CurrentLayout();
	int c = l.item.GetCount();
	if(cursor.GetCount())
		c = Max(cursor) + 1;
	LayoutItem& m = l.item.Insert(c);
	m.Create(_type);
	Point p = dragbase;
	Size sza, szb;
	GetZoomRatio(sza, szb);
	if(sza.cx)
		p.x = szb.cx * p.x / sza.cx;
	if(sza.cy)
		p.y = szb.cy * p.y / sza.cy;
	if(usegrid) {
		p.x = p.x / (int)~setting.gridx * (int)~setting.gridx;
		p.y = p.y / (int)~setting.gridy * (int)~setting.gridy;
	}
	Rect r(p, m.GetStdSize());
	m.pos = MakeLogPos(Ctrl::LEFT, Ctrl::TOP, r, l.size);
	cursor.Clear();
	cursor.Add(c);
	ReloadItems();
	if(!search.HasFocus()) {
		if(IsNull(_type))
			type.SetFocus();
		else {
			int q = m.FindProperty("SetLabel");
			if(q >= 0 && findarg(_type, "Label", "LabelBox") >= 0)
				m.property[q].PlaceFocus(0, 0);
			else
				variable.SetFocus();
		}
	}
}

void LayDes::Group(Bar& bar, const String& group)
{
	int i;
	Vector<String> type;
	for(i = 0; i < LayoutTypes().GetCount(); i++) {
		LayoutType& m = LayoutTypes()[i];
		if((IsNull(group) || m.group == group) && m.kind == LAYOUT_CTRL)
			type.Add(LayoutTypes().GetKey(i));
	}
	Sort(type);
	int h = 3 * StdFont().Info().GetHeight() / 2;
	int w = 8 * h / 3;
	if(auto *mb = dynamic_cast<MenuBar *>(&bar))
		mb->LeftGap(w + 6);
	int q = 0;
	for(i = 0; i < type.GetCount(); i++) {
		bar.Add(type[i], GetTypeIcon(type[i], w, h, 0, SLtGray),
		        THISBACK1(CreateCtrl, type[i]));
		if((q++ + 2) % 16 == 0)
			bar.Break();
	}
}

void LayDes::TemplateGroup(Bar& bar, TempGroup tg)
{
	int i;
	Vector<String> type;
	for(i = 0; i < LayoutTypes().GetCount(); i++) {
		LayoutType& m = LayoutTypes()[i];
		if((IsNull(tg.group) || m.group == tg.group) && m.kind == LAYOUT_CTRL)
			type.Add(LayoutTypes().GetKey(i));
	}
	Sort(type);
	int h = 3 * StdFont().Info().GetHeight() / 2;
	int w = 8 * h / 3;
	if(auto *mb = dynamic_cast<MenuBar *>(&bar))
		mb->LeftGap(w + 6);
	int q = 0;
	for(i = 0; i < type.GetCount(); i++) {
		bar.Add(type[i], GetTypeIcon(type[i], w, h, 0, SLtGray),
		        THISBACK1(CreateCtrl, tg.temp + '<' + type[i] + '>'));
		if((q++ + 2) % 16 == 0)
			bar.Break();
	}
}

void LayDes::Template(Bar& bar, const String& temp)
{
	Index<String> group;
	Vector<String> type;
	int h = 3 * StdFont().Info().GetHeight() / 2;
	int w = 8 * h / 3;
	if(auto *mb = dynamic_cast<MenuBar *>(&bar))
		mb->LeftGap(w + 6);
	int i;
	for(i = 0; i < LayoutTypes().GetCount(); i++) {
		LayoutType& m = LayoutTypes()[i];
		if(!IsNull(m.group))
			group.FindAdd(m.group);
		else
		if(m.kind == LAYOUT_CTRL)
			type.Add(LayoutTypes().GetKey(i));
	}
	Vector<String> sg = group.PickKeys();
	Sort(sg);
	Sort(type);
	int q = 0;
	for(i = 0; i < sg.GetCount(); i++) {
		bar.Add(sg[i], THISBACK1(TemplateGroup, TempGroup(temp, sg[i])));
		if((q++ + 2) % 16 == 0)
			bar.Break();
	}
	bar.Add("All", THISBACK1(TemplateGroup, TempGroup(temp, String())));
	if((q++ + 2) % 16 == 0)
		bar.Break();
	for(i = 0; i < type.GetCount(); i++) {
		bar.Add(type[i], GetTypeIcon(type[i], w, h, 0, SLtGray),
		        THISBACK1(CreateCtrl, temp + '<' + type[i] + '>'));
		if((q++ + 2) % 16 == 0)
			bar.Break();
	}
}

void LayDes::Templates(Bar& bar)
{
	Vector<String> temp;
	int i;
	for(i = 0; i < LayoutTypes().GetCount(); i++)
		if(LayoutTypes()[i].kind == LAYOUT_TEMPLATE)
			temp.Add(LayoutTypes().GetKey(i));
	Sort(temp);
	int q = 0;
	for(i = 0; i < temp.GetCount(); i++) {
		bar.Add(temp[i], THISBACK1(Template, temp[i]));
		if((q++ + 2) % 16 == 0)
			bar.Break();
	}
}

void LayDes::RightDown(Point p, dword keyflags)
{
	if(IsNull(currentlayout) || HasCapture()) return;
	dragbase = Normalize(p);
	MenuBar menu;
	menu.MaxIconSize(Size(64, 64));
	int h = StdFont().Info().GetHeight();
	int w = 8 * h / 3;
	menu.LeftGap(w + 2);
	menu.Add("User class", THISBACK1(CreateCtrl, ""));
	menu.Separator();
	Index<String> group;
	Vector<String> type;
	int i;
	for(i = 0; i < LayoutTypes().GetCount(); i++) {
		LayoutType& m = LayoutTypes()[i];
		if(!IsNull(m.group))
			group.FindAdd(m.group);
		else
		if(m.kind == LAYOUT_CTRL)
			type.Add(LayoutTypes().GetKey(i));
	}
	Vector<String> sg = group.PickKeys();
	Sort(sg);
	Sort(type);
	int q = 0;
	for(i = 0; i < sg.GetCount(); i++) {
		menu.Add(sg[i], THISBACK1(Group, sg[i]));
		if((q++ + 2) % 16 == 0)
			menu.Break();
	}
	menu.Add("All", THISBACK1(Group, String()));
	menu.Add("Templates", THISBACK(Templates));
	if((q++ + 2) % 16 == 0)
		menu.Break();
	for(i = 0; i < type.GetCount(); i++) {
		menu.Add(type[i], GetTypeIcon(type[i], w, h, 0, SLtGray),
		                  THISBACK1(CreateCtrl, type[i]));
		if((q++ + 2) % 16 == 0)
			menu.Break();
	}
	menu.Execute();
}

void  LayDes::LoadItems()
{
	int nitems = CurrentLayout().item.GetCount();
	item.SetCount(nitems);
	for(int i = 0; i < nitems; i++)
		LoadItem(i);
	property.Clear();
}

String GetLabel(const LayoutItem& m)
{
	EscValue l = m.ExecuteMethod("GetLabelMethod");
	if(l.IsVoid())
		for(int p = 0; p < m.property.GetCount(); p++)
			if(m.property[p].name == "SetLabel") {
				Value prop = ~m.property[p];
				return IsString(prop) && !IsNull(prop) ? AsCString(prop) : Null;
			}
	return l;
}

void LayDes::LoadItem(int ii)
{
	const LayoutItem& m = CurrentLayout().item[ii];
	String varlbl = m.variable;
	if(IsNull(varlbl))
		varlbl = GetLabel(m);
	item.Set(ii, 0, m.type);
	item.Set(ii, 1, varlbl);
	item.Set(ii, 2, m.hide);
}

void  LayDes::ReloadItems()
{
	int q = item.GetScroll();
	LoadItems();
	item.ScrollTo(q);
	SyncItems();
}

void  LayDes::Undo()
{
	if(IsNull(currentlayout))
		return;
	if(CurrentLayout().IsUndo()) {
		CurrentLayout().Undo();
		cursor.Clear();
		ReloadItems();
	}
}

void LayDes::Redo()
{
	if(IsNull(currentlayout))
		return;
	if(CurrentLayout().IsRedo()) {
		CurrentLayout().Redo();
		cursor.Clear();
		ReloadItems();
	}
}

void LayDes::Cut()
{
	if(IsNull(currentlayout) || cursor.GetCount() == 0)
		return;
	Copy();
	Delete();
}

void LayDes::Delete()
{
	SaveState();
	Vector<int> sel(cursor, 1);
	Sort(sel);
	cursor.Clear();
	CurrentLayout().item.Remove(sel);
	ReloadItems();
}

String LayDes::SaveSelection(bool scrolled)
{
	return CurrentLayout().Save(cursor, scrolled * ZPoint(sb).y, "\r\n") + "\r\n";
}

LayoutData LayDes::LoadLayoutData(const String& s)
{
	try {
		LayoutData l;
		l.SetCharset(charset);
		CParser p(s);
		l.Read(p);
		return l;
	}
	catch(CParser::Error) {}
	return LayoutData();
}

void LayDes::Copy()
{
	if(IsNull(currentlayout) || cursor.GetCount() == 0)
		return;
	WriteClipboardUnicodeText(ToUnicode(SaveSelection(), charset));
}

void LayDes::SelectAll()
{
	if(IsNull(currentlayout))
		return;
	LayoutData& l = CurrentLayout();
	int q = cursor.GetCount() ? cursor.Top() : -1;
	cursor.Clear();
	for(int i = 0; i < l.item.GetCount(); i++)
		if(i != q)
			cursor.Add(i);
	if(q >= 0)
		cursor.Add(q);
	SyncItems();
}

void LayDes::Duplicate()
{
	if(IsNull(currentlayout) || cursor.GetCount() == 0)
		return;
	SaveState();
	LayoutData& l = CurrentLayout();
	LayoutData d = LoadLayoutData(SaveSelection(false));
	int q = Max(cursor) + 1;
	cursor.Clear();
	for(int i = 0; i < d.item.GetCount(); i++) {
		LayoutItem& m = d.item[i];
		d.item[i].pos = MakeLogPos(m.pos, CtrlRect(m.pos, l.size).Offseted(0, 24), l.size);
		cursor.Add(q + i);
	}
	CurrentLayout().item.InsertPick(q, pick(d.item));
	ReloadItems();
}

void LayDes::Matrix()
{
	if(IsNull(currentlayout) || cursor.GetCount() == 0)
		return;
	SaveState();
	if(matrix.Execute() != IDOK)
		return;
	LayoutData& l = CurrentLayout();
	Rect r = CtrlRect(l.item[cursor[0]].pos, l.size);
	for(int i = 1; i < cursor.GetCount(); i++)
		r.Union(CtrlRect(l.item[cursor[i]].pos, l.size));
	String ls = SaveSelection();
	int q = Max(cursor) + 1;
	for(int x = 0; x < Nvl((int)~matrix.nx, 1); x++)
		for(int y = 0; y < Nvl((int)~matrix.ny, 1); y++)
			if(x || y) {
				LayoutData d = LoadLayoutData(ls);
				for(int i = 0; i < d.item.GetCount(); i++) {
					LayoutItem& m = d.item[i];
					Rect r = CtrlRect(m.pos, l.size);
					r.Offset((r.Width() + Nvl((int)~matrix.dx)) * x,
					         (r.Height() + Nvl((int)~matrix.dy)) * y);
					d.item[i].pos = MakeLogPos(m.pos, r, l.size);
					cursor.Add(q + i);
				}
				int w = d.item.GetCount();
				CurrentLayout().item.InsertPick(q, pick(d.item));
				q += w;
			}
	ReloadItems();
}

void LayDes::Paste()
{
	if(IsNull(currentlayout))
		return;
	SaveState();
	try {
		LayoutData l = LoadLayoutData(FromUnicode(ReadClipboardUnicodeText(), charset));
		int q = item.GetCount();
		if(cursor.GetCount())
		{
			q = 0;
			for(int i = 0; i < cursor.GetCount(); i++)
				q = max(q, cursor[i] + 1);
		}
		cursor.Clear();
		for(int i = 0; i < l.item.GetCount(); i++)
			cursor.Add(i + q);
		CurrentLayout().item.InsertPick(q, pick(l.item));
		ReloadItems();
	}
	catch(CParser::Error) {}
}

void LayDes::Align(int type)
{
	if(IsNull(currentlayout) || cursor.GetCount() == 0)
		return;
	SaveState();
	LayoutData& l = CurrentLayout();
	Rect cr = CtrlRect(l.item[cursor.Top()].pos, l.size);
	for(int i = 0; i < cursor.GetCount(); i++) {
		LayoutItem& m = l.item[cursor[i]];
		Rect r = CtrlRect(m.pos, l.size);
		switch(type) {
		case A_LEFT:
			r.OffsetHorz(cr.left - r.left);
			break;
		case A_HCENTER:
			r.OffsetHorz(cr.left + (cr.Width() - r.Width()) / 2 - r.left);
			break;
		case A_RIGHT:
			r.OffsetHorz(cr.right - r.right);
			break;
		case A_TOP:
			r.OffsetVert(cr.top - r.top);
			break;
		case A_VCENTER:
			r.OffsetVert(cr.top + (cr.Height() - r.Height()) / 2 - r.top);
			break;
		case A_BOTTOM:
			r.OffsetVert(cr.bottom - r.bottom);
			break;
		case A_SAMEWIDTH:
			r.right = r.left + cr.Width();
			break;
		case A_SAMEHEIGHT:
			r.bottom = r.top + cr.Height();
			break;
		case A_SAMESIZE:
			r.SetSize(cr.Size());
			break;
		case A_HORZCENTER:
			r.OffsetHorz((l.size.cx - r.Width()) / 2 - r.left);
			break;
		case A_VERTCENTER:
			r.OffsetVert((l.size.cy - r.Height()) / 2 - r.top);
			break;
		case A_MINWIDTH:
			r.SetSize(m.GetMinSize().cx, r.Height());
			break;
		case A_MINHEIGHT:
			r.SetSize(r.Width(), m.GetMinSize().cy);
			break;
		case A_LABEL:
			if(m.type == "Label") {
				Rect rr = r;
				int q = cursor[i] - 1;
				while(q >= 0) {
					if(l.item[q].type != "Label") {
						rr = CtrlRect(l.item[q].pos, l.size);
						break;
					}
					q--;
				}
				q = cursor[i] + 1;
				while(q < l.item.GetCount()) {
					if(l.item[q].type != "Label") {
						rr = CtrlRect(l.item[q].pos, l.size);
						break;
					}
					q++;
				}
				r.OffsetVert(rr.top + (rr.Height() - r.Height()) / 2 - r.top);
			}
			break;
		}
		m.pos = MakeLogPos(m.pos, r, l.size);
//		if(i == cursor.GetCount() - 1)
//			sb.ScrollInto(r.Offseted(MARGIN, MARGIN));
	}
	SetStatus();
}

void LayDes::SetSprings(dword s)
{
	if(IsNull(currentlayout))
		return;
	LayoutData& l = CurrentLayout();
	SaveState();
	int xa = (int16)LOWORD(s);
	int ya = (int16)HIWORD(s);
	for(int i = 0; i < cursor.GetCount(); i++) {
		Ctrl::LogPos& pos = l.item[cursor[i]].pos;
		Rect r = CtrlRect(pos, l.size);
		if(xa >= 0)
			pos.x = MakeLogc(xa, r.left, r.right, l.size.cx);
		if(ya >= 0)
			pos.y = MakeLogc(ya, r.top, r.bottom, l.size.cy);
		if(xa == AUTOSPRING) {
			pos.x = MakeLogc((r.left < l.size.cx / 2 ? LEFT : 0) |
			                 (r.right > l.size.cx / 2 ? RIGHT : 0),
			                 r.left, r.right, l.size.cx);
			pos.y = MakeLogc((r.top < l.size.cy / 2 ? TOP : 0) |
			                 (r.bottom > l.size.cy/ 2 ? BOTTOM : 0),
			                 r.top, r.bottom, l.size.cy);
		}
	}
	SetStatus();
}

void LayDes::ShowSelection(bool s)
{
	if(IsNull(currentlayout) || cursor.GetCount() == 0)
		return;
	LayoutData& l = CurrentLayout();
	for(int i = 0; i < cursor.GetCount(); i++)
		l.item[cursor[i]].hide = !s;
	SyncItems();
	Refresh();
}

void LayDes::MoveUp()
{
	SaveState();
	if(IsNull(currentlayout) || cursor.GetCount() == 0)
		return;
	LayoutData& l = CurrentLayout();
	Vector<int> sc(cursor, 1);
	Sort(sc);
	int q = 0;
	while(q < sc.GetCount() && sc[q] == q)
		q++;
	int im = q;
	while(q < sc.GetCount()) {
		int i = sc[q++];
		l.item.Swap(i, i - 1);
	}
	for(q = 0; q < cursor.GetCount(); q++)
		if(cursor[q] >= im)
			cursor[q]--;
	ReloadItems();
}

void LayDes::MoveDown()
{
	SaveState();
	if(IsNull(currentlayout) || cursor.GetCount() == 0)
		return;
	LayoutData& l = CurrentLayout();
	Vector<int> sc(cursor, 1);
	Sort(sc);
	int q = sc.GetCount() - 1;
	int u = l.item.GetCount() - 1;
	while(q >= 0 && sc[q] == u--)
		q--;
	int im = q >= 0 ? sc[q] : -1;
	while(q >= 0) {
		int i = sc[q--];
		l.item.Swap(i, i + 1);
	}
	for(q = 0; q < cursor.GetCount(); q++)
		if(cursor[q] <= im)
			cursor[q]++;
	ReloadItems();
}

void LayDes::DnDInsert(int line, PasteClip& d)
{
	if(GetInternalPtr<ArrayCtrl>(d, "layout-item") == &item && item.IsCursor() &&
	   !IsNull(currentlayout) && cursor.GetCount() && d.Accept()) {
		SaveState();
		LayoutData& l = CurrentLayout();
		Buffer<bool> sel(l.item.GetCount(), false);
		int n = l.item.GetCount();
		l.item.InsertN(n, n);
		for(int i = 0; i < cursor.GetCount(); i++)
			sel[cursor[i]] = true;
		cursor.Clear();
		int j = n;
		for(int i = 0; i < line; i++)
			if(!sel[i])
				l.item.Swap(j++, i);
		for(int i = 0; i < n; i++)
			if(sel[i]) {
				cursor.Add(j - n);
				l.item.Swap(j++, i);
			}
		for(int i = line; i < n; i++)
			if(!sel[i])
				l.item.Swap(j++, i);
		l.item.Remove(0, n);
		ReloadItems();
	}
}

void LayDes::Drag()
{
	item.DoDragAndDrop(InternalClip(item, "layout-item"), item.GetDragSample(), DND_MOVE);
}


bool RectLess(const Rect& a, const Rect& b)
{
	int d = min(a.bottom, b.bottom) - max(a.top, b.top);
	int w = min(a.GetHeight(), b.GetHeight());
	return d > w / 2 ? a.left < b.left : a.top < b.top;
}

void LayDes::SortItems()
{
	SaveState();
	if(IsNull(currentlayout) || cursor.GetCount() < 2)
		return;
	LayoutData& l = CurrentLayout();

	Sort(cursor);
	int count = cursor.GetCount();

	Array<LayoutItem> item;
	Vector<Rect> rect;
	for(int i = 0; i < count; ++i) {
		rect.Add(CtrlRect(l.item[cursor[i]].pos, l.size));
		item.Add() = pick(l.item[cursor[i]]);
	}
	l.item.Remove(cursor);

	bool swap = false;
	do {
		swap = false;
		for(int i = 0; i < count - 1; i++)
			if(RectLess(rect[i + 1], rect[i])) {
				Swap(rect[i], rect[i + 1]);
				Swap(item[i], item[i + 1]);
				swap = true;
			}
	}
	while(swap);
	
	int ii = cursor[0];
	l.item.InsertPick(ii, pick(item));
	
	cursor.Clear();
	for(int i = 0; i < count; i++)
		cursor.Add(i + ii);

	ReloadItems();
}

void LayDes::Flush()
{
	currentlayout = Null;
}

LayoutData& LayDes::CurrentLayout()
{
	return layout[currentlayout];
}

void LayDes::LayoutCursor()
{
	Flush();
	draghandle = -1;
	currentlayout = list.GetKey();
	cursor.Clear();
	type.Disable();
	variable.Disable();
	property.Clear();
	if(IsNull(currentlayout))
		return;
	LoadItems();
	SyncItems();
	SetSb();
	if(!search.HasFocus())
		SetFocus();
}

void LayDes::PrevLayout()
{
	list.Key(K_UP, 0);
}

void LayDes::NextLayout()
{
	list.Key(K_DOWN, 0);
}

void LayDes::SyncLayoutList()
{
	int sc = list.GetScroll();
	int c = list.GetKey();
	list.Clear();
	String s = ToUpper((String)~search);
	for(int i = 0; i < layout.GetCount(); i++)
		if(ToUpper(layout[i].name).Find(s) >= 0)
			list.Add(i, layout[i].name);
	list.ScrollTo(sc);
	if(!IsNull(c))
		list.FindSetCursor(c);
	LayoutCursor();
}

void LayDes::Search()
{
	SyncLayoutList();
	if(!list.IsCursor())
		list.GoBegin();
}

void LayDes::GoTo(int key)
{
	if(list.FindSetCursor(key))
		return;
	search <<= Null;
	SyncLayoutList();
	list.FindSetCursor(key);
}

void LayDes::AddLayout(bool insert)
{
	String name;
	for(;;) {
		if(!EditText(name, "Add new layout", "Layout", CharFilterCid))
			return;
		CParser p(name);
		if(p.IsId())
			break;
		Exclamation("Invalid name!");
	}
	int q = list.GetKey();
	if(!insert || IsNull(q) || !(q >= 0 && q < layout.GetCount()))
		q = layout.GetCount();
	layout.Insert(q).name = name;
	SyncLayoutList();
	GoTo(q);
	LayoutCursor();
}

void LayDes::DuplicateLayout()
{
	if(IsNull(currentlayout))
		return;
	LayoutData& c = CurrentLayout();
	String name = c.name;
	for(;;) {
		if(!EditText(name, "Duplicate layout", "Layout", CharFilterCid))
			return;
		CParser p(name);
		if(p.IsId())
			break;
		Exclamation("Invalid name!");
	}
	String data = c.Save(0, "\r\n");
	CParser p(data);
	int next = currentlayout + 1;
	LayoutData& d = layout.Insert(next);
	d.Read(p);
	d.name = name;
	SyncLayoutList();
	GoTo(next);
	LayoutCursor();
}

void LayDes::RenameLayout()
{
	if(IsNull(currentlayout))
		return;
	String name = layout[currentlayout].name;
	if(!EditText(name, "Rename layout", "Layout", CharFilterCid))
		return;
	int q = list.GetKey();
	layout[currentlayout].name = name;
	SyncLayoutList();
	GoTo(q);
	LayoutCursor();
}

void LayDes::RemoveLayout()
{
	if(IsNull(currentlayout) || !PromptYesNo("Remove [* " + DeQtf(layout[currentlayout].name) + "] ?"))
		return;
	int q = list.GetKey();
	layout.Remove(currentlayout);
	SyncLayoutList();
	if(!IsNull(q)) {
		GoTo(q + 1);
		if(!list.IsCursor())
			list.GoEnd();
	}
	LayoutCursor();
}

void LayDes::MoveLayoutUp()
{
	if(!IsNull(search)) {
		search <<= Null;
		SyncLayoutList();
	}
	int q = list.GetKey();
	if(q > 0) {
		layout.Swap(q, q - 1);
		SyncLayoutList();
		GoTo(q - 1);
	}
}

void LayDes::MoveLayoutDown()
{
	if(!IsNull(search)) {
		search <<= Null;
		SyncLayoutList();
	}
	int q = list.GetKey();
	if(q >= 0 && q < layout.GetCount() - 1) {
		layout.Swap(q, q + 1);
		SyncLayoutList();
		GoTo(q + 1);
	}
}

void LayDes::DnDInsertLayout(int line, PasteClip& d)
{
	if(GetInternalPtr<ArrayCtrl>(d, "layout") == &list && list.IsCursor() &&
	   line >= 0 && line <= layout.GetCount() && d.Accept()) {
		if(!IsNull(search)) {
			search <<= Null;
			SyncLayoutList();
		}
		int c = list.GetKey();
		layout.Move(c, line);
		if(c <= line)
			line--;
		SyncLayoutList();
		GoTo(line);
	}
}

void LayDes::DragLayout()
{
	list.DoDragAndDrop(InternalClip(list, "layout"), list.GetDragSample(), DND_MOVE);
}

void LayDes::LayoutMenu(Bar& bar)
{
	bool iscursor = list.IsCursor();
	bar.Add("Add new layout..", THISBACK1(AddLayout, false));
	bar.Add("Insert new layout..", THISBACK1(AddLayout, true));
	bar.Add(iscursor, "Duplicate layout..", THISBACK(DuplicateLayout));
	bar.Add(iscursor, "Rename layout..", THISBACK(RenameLayout));
	bar.Add(iscursor, "Remove layout..", THISBACK(RemoveLayout));
	bar.Separator();
	int q = list.GetKey();
	bar.Add(iscursor && q > 0,
	        AK_MOVELAYOUTUP, LayImg::MoveUp(), THISBACK(MoveLayoutUp));
	bar.Add(iscursor && q < layout.GetCount() - 1,
	        AK_MOVELAYOUTDOWN, LayImg::MoveDown(), THISBACK(MoveLayoutDown));
}

LayoutItem& LayDes::CurrentItem()
{
	return CurrentLayout().item[cursor.Top()];
}

void LayDes::PropertyChanged()
{
	if(item.IsCursor())
	{
		CurrentItem().Invalidate();
		int c = item.GetCursor();
		LoadItem(c);
		SyncItem(c, 2);
	}
	Refresh();
	SetBar();
}

void LayDes::FrameFocus()
{
	if(property.HasFocusDeep()) {
		SaveState();
		SetStatus();
	}
}

void LayDes::ItemClick()
{
	if(IsNull(currentlayout))
		return;
	SaveState();
	if(GetShift()) {
		if(cursor.GetCount()) {
			int i = minmax(item.GetCursor(), 0, cursor.Top());
			int m = max(item.GetCursor(), cursor.Top());
			cursor.Clear();
			while(i <= m)
				cursor.Add(i++);
			SyncItems();
		}
	}
	else if(item.IsCursor()) {
		if(!GetCtrl())
			cursor.Clear();
		int c = item.GetCursor();
		int q = FindIndex(cursor, c);
		if(q >= 0)
			cursor.Remove(q);
		else
			cursor.Add(c);
	}
	SetFocus();
	SyncItems();
}

void LayDes::SyncUsc()
{
	type.ClearList();
	for(int i = 0; i < LayoutTypes().GetCount(); i++)
		if(LayoutTypes()[i].kind != LAYOUT_SUBCTRL)
			type.AddList(LayoutTypes().GetKey(i));
	if(!IsNull(currentlayout)) {
		LayoutData& d = CurrentLayout();
		for(int i = 0; i < d.item.GetCount(); i++)
			d.item[i].Invalidate();
	}
	Refresh();
}

void LayDes::TypeEdit()
{
	if(IsNull(currentlayout) || cursor.GetCount() == 0)
		return;
	LayoutData& l = CurrentLayout();
	for(int i = 0; i < cursor.GetCount(); i++) {
		LayoutItem& m = l.item[cursor[i]];
		m.SetCharset(charset);
		String s = m.SaveProperties();
		m.Create(~type);
		try {
			CParser p(s);
			m.ReadProperties(p, false);
		}
		catch(CParser::Error&) {}
		item.Set(cursor[i], 0, m.type);
	}
	SyncProperties(false);
	SetStatus();
}

void LayDes::VariableEdit()
{
	if(IsNull(currentlayout) || cursor.GetCount() == 0)
		return;
	LayoutData& l = CurrentLayout();
	LayoutItem& m = l.item[cursor.Top()];
	m.variable = ~variable;
	if(IsNull(m.variable))
		item.Set(cursor.Top(), 1, GetLabel(m));
	else
		item.Set(cursor.Top(), 1, m.variable);
	SyncItem(cursor.Top(), 2);
}

static int RoundStep(int org, int d, int g)
{
	return d ? itimesfloor(org + d * g + (d > 0 ? 0 : g - 1), g) - org : 0;
}

bool LayDes::DoHotKey(dword key)
{
	if(key == K_CTRL_F) {
		search.SetFocus();
		return true;
	}
	return false;
}

bool LayDes::DoKey(dword key, int count)
{
	SaveState();
	Point move(0, 0);
	if(!IsNull(currentlayout) && !cursor.IsEmpty()) {
		switch(key & ~K_CTRL) {
		case K_SHIFT_LEFT:   move.x = -1; break;
		case K_SHIFT_RIGHT:  move.x = +1; break;
		case K_SHIFT_UP:     move.y = -1; break;
		case K_SHIFT_DOWN:   move.y = +1; break;
		}
		if(move.x | move.y) {
			Size grid(1, 1);
			if(usegrid) {
				grid.cx = minmax<int>(~setting.gridx, 1, 32);
				grid.cy = minmax<int>(~setting.gridy, 1, 32);
			}
			LayoutData& l = CurrentLayout();
			Rect master = CtrlRect(l.item[cursor.Top()].pos, l.size);
			Size shift;
			shift.cx = RoundStep(key & K_CTRL ? master.Width()  : master.left, move.x, grid.cx);
			shift.cy = RoundStep(key & K_CTRL ? master.Height() : master.top,  move.y, grid.cy);
			for(int i = 0; i < cursor.GetCount(); i++) {
				LayoutItem& item = l.item[cursor[i]];
				Rect rc = CtrlRect(item.pos, l.size);
				rc.right  += shift.cx;
				rc.bottom += shift.cy;
				if(!(key & K_CTRL)) {
					rc.left += shift.cx;
					rc.top  += shift.cy;
				}
				item.pos = MakeLogPos(item.pos, rc, l.size);
			}
			SetStatus(false);
			return true;
		}
	}
	switch(key) {
	case K_PAGEUP:
		PrevLayout();
		return true;
	case K_PAGEDOWN:
		NextLayout();
		return true;
	case K_UP:
	case K_DOWN:
		{
			dword k = (key == K_PAGEUP ? K_UP : key == K_PAGEDOWN ? K_DOWN : key);
			Ptr<Ctrl> focus = GetFocusCtrl();
			if(!item.IsCursor() && item.GetCount() > 0)
				item.SetCursor(k == K_DOWN ? 0 : item.GetCount() - 1);
			else
				item.Key(k, count);
			ItemClick();
			if(!!focus)
				focus->SetWantFocus();
		}
		return true;
	default:
		if(key >= ' ' && key < 65536) {
			if(IsNull(currentlayout) || cursor.GetCount() == 0)
				return false;
			LayoutItem& m = CurrentItem();
			for(int i = 0; i < m.property.GetCount(); i++)
				if(m.property[i].name == "SetLabel")
					return m.property[i].PlaceFocus(key, count);
		}
		break;
	}
	return MenuBar::Scan(THISBACK(LayoutMenu), key);
}
