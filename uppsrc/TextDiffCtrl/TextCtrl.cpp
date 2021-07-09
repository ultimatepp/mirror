#include "TextDiffCtrl.h"

namespace Upp {

inline Color HistoryBg() { return Color(255, 255, 0); }

TextCompareCtrl::TextCompareCtrl()
{
	letter = Size(1, 1);
	number_width = 0;
	number_yshift = 0;
	number_bg = WhiteGray();
	SetFrame(FieldFrame());
	AddFrame(scroll);
	SetFont(CourierZ(10), CourierZ(10));
	scroll.NoAutoHide();
	scroll.WhenScroll = THISBACK(SelfScroll);
	maxwidth = 0;
	tabsize = 4;
	gutter_width = 0;
	gutter_bg = AdjustIfDark(Color(151, 190, 239));
	gutter_fg = SGreen;
	cursor = anchor = Null;
	gutter_capture = false;
	show_line_number = true;
	show_white_space = false;
	show_diff_highlight = true;
	change_paper_color = true;
}

int TextCompareCtrl::GetLineNo(int y, int& ii)
{
	ii = scroll.Get().y + y / letter.cy;
	return ii >= 0 && ii < lines.GetCount() ? lines[ii].number : Null;
}

void TextCompareCtrl::DoSelection(int y, bool shift)
{
	int ii;
	int i = GetLineNo(y, ii);
	if(!IsNull(i)) {
		if(shift)
			cursor = i;
		else
			cursor = anchor = i;
		Refresh();
		scroll.ScrollIntoY(ii);
		WhenSel();
	}
}

void TextCompareCtrl::LeftDown(Point pt, dword keyflags)
{
	Size sz = GetSize();
	if(pt.x > sz.cx - gutter_width || HasCapture() && gutter_capture) {
		if(!HasCapture())
			SetCapture();
		int line = (pt.y * lines.GetCount()) / sz.cy;
		int page_lines = sz.cy / letter.cy;
		scroll.SetY(line - page_lines / 2);
		gutter_capture = true;
	}
	else {
		DoSelection(pt.y, keyflags & K_SHIFT);
		SetCapture();
		gutter_capture = false;
	}
	SetWantFocus();
}

void TextCompareCtrl::LeftDouble(Point pt, dword keyflags)
{
	int ii;
	int i = GetLineNo(pt.y, ii);
	if(!IsNull(i))
		WhenLeftDouble(i - 1, ii);
}

void TextCompareCtrl::MouseMove(Point pt, dword flags)
{
	if(HasCapture()) {
		if(gutter_capture)
			LeftDown(pt, flags);
		else
			DoSelection(pt.y, true);
	}
}

void TextCompareCtrl::LeftRepeat(Point pt, dword keyflags)
{
	if(HasCapture() && !gutter_capture)
		DoSelection(pt.y, true);
}

void TextCompareCtrl::LeftUp(Point pt, dword keyflags)
{
	ReleaseCapture();
}

void Upp::TextCompareCtrl::LostFocus()
{
	Refresh();
}

void TextCompareCtrl::Copy()
{
	int sell, selh;
	if(GetSelection(sell, selh)) {
		String clip;
		for(int i = 0; i < lines.GetCount(); i++) {
			const Line& l = lines[i];
			if(l.number >= sell && l.number <= selh) {
				clip << l.text;
			#ifdef PLATFORM_WIN32
				clip << "\r\n";
			#else
				clip << "\n";
			#endif
			}
		}
		ClearClipboard();
		AppendClipboardText(clip);
	}
}

void TextCompareCtrl::RightDown(Point p, dword keyflags)
{
	MenuBar b;
	b.Add(!IsNull(cursor), t_("Copy"), CtrlImg::copy(), THISBACK(Copy)).Key(K_CTRL_C);
	b.Execute();
}

bool TextCompareCtrl::Key(dword key, int repcnt)
{
	Point pos = scroll, newpos = pos, page = scroll.GetPage();
	switch(key) {
		case K_CTRL_A:
			if(lines.GetCount() > 1) {
				cursor = lines[0].number;
				anchor = lines.Top().number;
				Refresh();
			}
			break;
		case K_LEFT:       newpos.x--; break;
		case K_RIGHT:      newpos.x++; break;
		case K_CTRL_LEFT:  newpos.x -= page.x >> 1; break;
		case K_CTRL_RIGHT: newpos.x += page.x >> 1; break;
		case K_UP:         newpos.y--; break;
		case K_DOWN:       newpos.y++; break;
		case K_PAGEUP:     newpos.y -= page.y; break;
		case K_PAGEDOWN:   newpos.y += page.y; break;
		case K_HOME:       newpos.x = 0; break;
		case K_END:        newpos.x = maxwidth - page.x; break;
		case K_CTRL_HOME:  newpos.y = 0; break;
		case K_CTRL_END:   newpos.y = lines.GetCount() - page.y; break;
		case K_CTRL_C:
			Copy();
			break;
		case K_F3: {
			bool found = false;
			int i = max(pos.y + 2, 0);
			while(i < lines.GetCount() && lines[i].level)
				i++;
			while(i < lines.GetCount())
				if(lines[i++].level) {
					newpos.y = i - 2;
					found = true;
					break;
				}
			if(!found) {
				BeepExclamation();
				return true;
			}
			break;
		}
		case K_SHIFT_F3: {
			bool found = false;
			int i = min(pos.y, lines.GetCount() - 1);
			while(i > 0 && lines[i].level)
				i--;
			while(i >= 0)
				if(lines[i--].level) {
					newpos.y = i;
					found = true;
					break;
				}
			if(!found) {
				BeepExclamation();
				return true;
			}
			break;
		}
		default: return false;
	}
	if(newpos != pos)
		scroll.Set(newpos);
	return true;
}

int TextCompareCtrl::GetMatchLen(const wchar *s1, const wchar *s2, int len)
{
	for(int q = 0; q < len; q++)
		if(s1[q] != s2[q])
			return q;
	return len;
}

bool TextCompareCtrl::LineDiff(bool left, Vector<LineEdit::Highlight>& hln, Color eq_color,
                               const wchar *s1, int l1, int h1,
                               const wchar *s2, int l2, int h2, int depth)
{
	int p1 = 0;
	int p2 = 0;
	int matchlen = 0;
	int subscore = 0; // prefer matches that are not in words
	
	for(int pos1 = l1; pos1 < h1; pos1++)
		for(int pos2 = l2; pos2 < h2; pos2++) {
			int ml = GetMatchLen(s1 + pos1, s2 + pos2, min(h1 - pos1, h2 - pos2));
			if(ml && ml >= matchlen) {
				int l = pos1;
				int h = pos1 + matchlen;
				int sc = (l == 0 || IsAlNum(s1[l - 1]) != IsAlNum(s1[l])) +
				         (h == 0 || h >= l1 || IsAlNum(s1[h - 1]) != IsAlNum(s1[h]));
				if(ml > matchlen || sc > subscore) {
					p1 = pos1;
					p2 = pos2;
					matchlen = ml;
					subscore = sc;
				}
			}
		}
	
	if(matchlen > 1 || matchlen && !IsAlNum(s1[p1])) {
		for(int i = 0; i < matchlen; i++)
			hln[(left ? p1 : p2) + i].paper = eq_color;

		if(++depth < 20) {
			LineDiff(left, hln, eq_color, s1, l1, p1, s2, l2, p2, depth);
			LineDiff(left, hln, eq_color, s1, p1 + matchlen, h1, s2, p2 + matchlen, h2, depth);
		}
		return true;
	}
	return false;
}

void TextCompareCtrl::Paint(Draw& draw)
{
	Point sc = scroll.Get();
	Size offset = (Size)sc * letter;
	Size sz = GetSize();

	int lcnt = lines.GetCount();
	int first_line = offset.cy / letter.cy;
	int last_line = min(idivceil(sz.cy + offset.cy, letter.cy), lines.GetCount() - 1);

	if(gutter_width > 0)
	{
		int t = 0, b = 0;
		int gx = sz.cx - gutter_width;
		for(int i = 0; i < lcnt; i++)
			if(lines[i].level > 1) {
				b = idivceil(sz.cy * i, lcnt);
				if(b >= t) {
					draw.DrawRect(gx, t, gutter_width, b - t, gutter_bg);
					draw.DrawRect(gx, b, gutter_width, 1, gutter_fg);
					t = b + 1;
				}
			}

		draw.DrawRect(gx, t, gutter_width, sz.cy - t, gutter_bg);

		int total = letter.cy * lcnt;
		if(total <= 0)
			total = 1;
		int page_height = (sz.cy * sz.cy) / total;
		int ty = max(0, (sz.cy * offset.cy) / total);
		int by = min(sz.cy, ty + page_height);
		draw.DrawRect(gx, ty, gutter_width, 2, Black);
		draw.DrawRect(gx, by - 2, gutter_width, 2, Black);
		draw.DrawRect(gx, ty, 2, by - ty, Black);
		draw.DrawRect(gx + gutter_width - 2, ty, 2, by - ty, Black);
	}
	
	WString test = "č"; // read text/paper colors from highlighting scheme using likely non-highlighted text
	Vector<LineEdit::Highlight> th;
	th.SetCount(2);
	th[0].ink = SColorText();
	th[0].paper = SColorPaper();
	WhenHighlight(th, test);
	Color text_color = th[0].ink;
	Color paper_color = th[0].paper;

	Color diffpaper = IsDark(paper_color) ? Magenta() : Yellow();
	Color missingpaper = IsDark(paper_color) ? Gray() : LtGray();

	int n_width = show_line_number ? number_width : 0;
	if(show_line_number) {
		for(int i = first_line; i <= last_line; i++) {
			const Line& l = lines[i];
			int y = i * letter.cy - offset.cy;
			Color paper = IsNull(l.number) ? missingpaper : l.diff ? diffpaper : SColorPaper();
			Color ink = l.diff ? SRed(): SGray();
			draw.DrawRect(0, y, n_width, letter.cy, paper);
			draw.DrawRect(n_width - 1, y, 1, letter.cy, Gray());
			if(!IsNull(l.number))
				draw.DrawText(0, y + number_yshift, FormatInt(l.number_diff), number_font, ink);
		}
	}
	draw.Clip(n_width, 0, sz.cx - gutter_width - n_width, sz.cy);

	for(int i = first_line; i <= last_line; i++) {
		const Line& l = lines[i];
		int y = i * letter.cy - offset.cy;
		Color ink = text_color;
		Color paper = IsNull(l.number) ? missingpaper : l.diff ? diffpaper : paper_color;
		bool sel = IsSelected(i);
		if(sel) {
			ink = SColorHighlightText();
			paper = HasFocus() ? SColorHighlight() : Blend(SColorHighlight(), SColorDisabled());
		}

		WString ln = l.text.ToWString();
		if(ln.GetCount() > 20000)
			ln.Trim(20000);
		ln = ExpandTabs(ln);
		Vector<LineEdit::Highlight> hln;
		hln.SetCount(ln.GetCount() + 1);
		for(int i = 0; i < hln.GetCount(); i++) {
			LineEdit::Highlight& h = hln[i];
			h.paper = sel ? paper : diffpaper;
			h.ink = ink;
			h.chr = ln[i];
			h.font = StdFont();
		}
		
		bool ldiff = false;

		if(!sel) {
			WhenHighlight(hln, ln);

			for(int i = 0; i < hln.GetCount(); ++i) {
				LineEdit::Highlight& h = hln[i];
				if (change_paper_color)
					h.paper = paper;
			}
			if(show_diff_highlight) {
				WString ln_diff = l.text_diff.ToWString();
				ln_diff = ExpandTabs(ln_diff);
				if((int64)ln_diff.GetCount() * ln.GetCount() < 50000) {
					if(left)
						ldiff = LineDiff(true, hln, paper_color,
						                 ~ln, 0, ln.GetCount(), ~ln_diff, 0, ln_diff.GetCount(), 0);
					else
						ldiff = LineDiff(false, hln, paper_color,
						                 ~ln_diff, 0, ln_diff.GetCount(), ~ln, 0, ln.GetCount(), 0);
				}
			}
			if(show_white_space) {
				for(int i = ln.GetCount(); i >= 0; --i) {
					LineEdit::Highlight& h = hln[i];
					if (ln[i] == 0)
						continue;
					if(ln[i] == 32)
						h.paper = IsDark(paper_color) ? LtRed() : Red();
					else
						break;
				}
			}
			
			if(ldiff)
				paper = paper_color;
		}

		draw.DrawRect(0, y, sz.cx, letter.cy, paper); // paint the end of line
		int x = 0;
		int ii = 0;
		while(ii < hln.GetCount() - 1) {
			Font fnt = font;
			LineEdit::Highlight& h = hln[ii];
			int l = 1;
			while(ii + l < hln.GetCount() - 1 && hln[ii + l].paper == h.paper)
				l++;
			draw.DrawRect(n_width - offset.cx + x, y, l * letter.cx, letter.cy, h.paper);
			x += l * letter.cx;
			ii += l;
		}
		x = 0;
		ii = 0;
		Vector<int> dx;
		while(ii < hln.GetCount() - 1) {
			Font fnt = font;
			LineEdit::Highlight& h = hln[ii];
			WString text;
			text.Cat(h.chr);
			for(;;) {
				int i = ii + text.GetCount();
				if(i < hln.GetCount() - 1 && hln[i].ink == h.ink && hln[i].font == h.font)
					text.Cat(hln[i].chr);
				else
					break;
			}
			fnt.Bold(h.font.IsBold());
			fnt.Italic(h.font.IsItalic());
			fnt.Underline(h.font.IsUnderline());
			dx.SetCount(text.GetCount());
			dx.Set(0, letter.cx, text.GetCount());
			
			draw.DrawText(n_width - offset.cx + x, y, text, fnt, h.ink, dx);

			x += text.GetCount() * letter.cx;
			ii += text.GetCount();
		}
	}
	int lcy = lcnt * letter.cy - offset.cy;
	draw.DrawRect(0, lcy, sz.cx, sz.cy - lcy, SGray());
	draw.End();
	draw.DrawRect(0, lcy, n_width, sz.cy - lcy, number_bg);
}

void TextCompareCtrl::TabSize(int t)
{
	tabsize = t;
	UpdateWidth();
	Layout();
}

void TextCompareCtrl::SetFont(Font f, Font nf)
{
	font = f;
	number_font = nf;
	letter.cy = f.GetCy();
	letter.cx = f.GetMonoWidth();
	number_width = 5 * nf.GetMonoWidth();
	number_yshift = (f.GetCy() - nf.GetCy() + 2) >> 1;
	Layout();
}

void TextCompareCtrl::SetFont(Font f)
{
	SetFont(f, GetNumberFont());
}

void TextCompareCtrl::Layout()
{
	int n_width = show_line_number ? number_width : 0;

	scroll.Set(scroll, (scroll.GetReducedViewSize() - Size(n_width + gutter_width, 0)) / letter, Size(maxwidth, lines.GetCount()));
	Refresh();
}

void TextCompareCtrl::MouseWheel(Point pt, int zdelta, dword keyflags)
{
	scroll.WheelY(zdelta);
}

void TextCompareCtrl::SetCount(int c)
{
	bool rl = (c < lines.GetCount());
	lines.SetCount(c);
	if(rl)
		UpdateWidth();
	Layout();
}

void TextCompareCtrl::AddCount(int c)
{
	lines.InsertN(lines.GetCount(), c);
	Layout();
}

void TextCompareCtrl::Set(int line, String text, bool diff, int number, int level, String text_diff, int number_diff)
{
	Line& l = lines.At(line);
	int tl = MeasureLength(text.ToWString());
	int lt = MeasureLength(l.text.ToWString());
	bool rl = (tl < lt && lt == maxwidth);
	l.number = number;
	l.text = text;
	l.diff = diff;
	l.level = level;
	l.text_diff = text_diff;
	l.number_diff = number_diff;
	if(rl)
		UpdateWidth();
	else if(tl > maxwidth) {
		maxwidth = tl;
		Layout();
	}
}

void TextCompareCtrl::SelfScroll()
{
	Refresh();
	WhenScroll();
}

void TextCompareCtrl::PairScroll(TextCompareCtrl *ctrl)
{
	scroll.Set(ctrl->scroll.Get());
}

void TextCompareCtrl::UpdateWidth()
{
	maxwidth = 0;
	for(int i = 0; i < lines.GetCount(); i++)
		maxwidth = max(maxwidth, MeasureLength(lines[i].text.ToWString()));
}

int TextCompareCtrl::MeasureLength(const wchar *text) const
{
	int pos = 0;
	while(*text)
		if(*text++ == '\t')
			pos += tabsize - pos % tabsize;
		else
			pos++;
	return pos;
}

bool TextCompareCtrl::IsSelected(int i) const
{
	int sell, selh;
	if(HasLine(i) && GetSelection(sell, selh)) {
		i = GetNumber(i);
		return i >= sell && i <= selh;
	}
	return false;
}

bool TextCompareCtrl::GetSelection(int& l, int& h) const
{
	if(IsNull(cursor)) {
		l = h = -1;
		return false;
	}
	l = min(cursor, anchor);
	h = max(cursor, anchor);
	return true;
}

WString TextCompareCtrl::ExpandTabs(const wchar *text) const
{
	WString out;
	for(wchar c; (c = *text++);)
		if(c == '\t')
			out.Cat(' ', tabsize - out.GetLength() % tabsize);
		else
			out.Cat(c);
	return out;
}

Point TextCompareCtrl::GetPos() const
{
	Point pos(0, 0);
	int ltop = minmax(scroll.Get().y, 0, lines.GetCount() - 1);
	while(ltop >= 0 && IsNull(lines[ltop].number)) {
		ltop--;
		pos.y++;
	}
	if(ltop >= 0)
		pos.x = lines[ltop].number;
	return pos;
}

void TextCompareCtrl::SetPos(Point pos)
{
	int l = FindFieldIndex(lines, &Line::number,pos.x);
	if(l < 0)
		l = 0;
	SetSb(l + pos.y);
}

};
