#include "RichText.h"

namespace Upp {

void RichPara::Smh(Lines& lines, HeightInfo *th, int cx) const
{
	Line& l = lines.line.Top();
	l.ascent = l.descent = l.external = 0;
	const HeightInfo *he = th + l.pos + l.len;
	for(const HeightInfo *h = th + l.pos; h < he; h++) {
		if(h->ascent > l.ascent) l.ascent = h->ascent;
		if(h->descent > l.descent) l.descent = h->descent;
		if(h->external > l.external) l.external = h->external;
	}
	if(format.linespacing == LSP115) {
		l.ascent = 115 * l.ascent / 100;
		l.descent = 115 * l.descent / 100;
	}
	if(format.linespacing == LSP15) {
		l.ascent = (3 * l.ascent) >> 1;
		l.descent = (3 * l.descent) >> 1;
	}
	if(format.linespacing == LSP20) {
		l.ascent = 2 * l.ascent;
		l.descent = 2 * l.descent;
	}
	l.xpos = format.lm;
	cx -= format.lm + format.rm;
	l.xpos += lines.GetCount() == 1 ? lines.first_indent : lines.next_indent;
	if(!l.withtabs)
		switch(format.align) {
		case ALIGN_RIGHT:
			l.xpos += cx - l.cx;
			break;
		case ALIGN_CENTER:
			l.xpos += (cx - l.cx) / 2;
			break;
		}
	l.cx -= lines.GetCount() == 1 ? lines.first_indent : lines.next_indent;
}

RichPara::Tab RichPara::GetNextTab(int pos, int cx) const
{
	int tabi = -1;
	int dist = INT_MAX;
	for(int i = 0; i < format.tab.GetCount(); i++) {
		const Tab& tab = format.tab[i];
		int tabpos = tab.pos;
		if(tabpos & TAB_RIGHTPOS)
			tabpos = cx - (tabpos & ~TAB_RIGHTPOS);
		if(tabpos > pos && tabpos - pos < dist) {
			tabi = i;
			dist = tabpos - pos;
		}
	}
	if(format.bullet == BULLET_TEXT) {
		int q = format.indent + format.lm;
		if(q > pos && q - pos < dist) {
			Tab tab;
			tab.align = ALIGN_LEFT;
			tab.pos = q;
			return tab;
		}
	}
	if(tabi < 0) {
		Tab tab;
		tab.pos = format.tabsize ? (pos + format.tabsize) / format.tabsize * format.tabsize : 0;
		tab.align = ALIGN_LEFT;
		return tab;
	}
	Tab tab = format.tab[tabi];
	if(tab.pos & TAB_RIGHTPOS)
		tab.pos = cx - (tab.pos & ~TAB_RIGHTPOS);
	return tab;
}

struct RichPara::StorePart {
	wchar             *t;
	int               *w;
	int               *p;
	const CharFormat **f;
	HeightInfo        *h;
	int                pos;
	FontInfo           pfi;

	void Store(Lines& lines, const Part& p, int pinc);
};

void RichPara::StorePart::Store(Lines& lines, const Part& part, int pinc)
{
	CharFormat& pfmt = lines.hformat.Add();
	pfmt = part.format;
	if(part.field && pinc) {
		for(int i = 0; i < part.fieldpart.GetCount(); i++)
			Store(lines, part.fieldpart[i], 0);
		pos++;
	}
	else
	if(part.object) {
		*f++ = &pfmt;
		Size sz = part.object.GetSize();
		*w++ = sz.cx;
		h->ydelta = part.object.GetYDelta();
		h->ascent = sz.cy - h->ydelta;
		h->descent = max(h->ydelta, 0);
		h->external = 0;
		lines.object.Add(part.object);
		h->object = &lines.object.Top();		
		h++;
		*t++ = 'x';
		*p++ = pos;
		pos += pinc;
	}
	else {
		const wchar *s = part.text;
		const wchar *lim = part.text.End();
		Font fnt = part.format;
		FontInfo fi = fnt.Info();
		FontInfo wfi = fi;
		if(part.format.sscript) {
			fnt.Height(fnt.GetHeight() * 3 / 5);
			wfi = fnt.Info();
		}
		if(part.format.capitals) {
			CharFormat& cfmt = lines.hformat.Add();
			cfmt = part.format;
			cfmt.Height(cfmt.GetHeight() * 4 / 5);
			FontInfo cfi = cfmt.Info();
			FontInfo cwfi = cfi;
			if(part.format.sscript) {
				Font fnt = cfmt;
				fnt.Height(fnt.GetHeight() * 3 / 5);
				cwfi = fnt.Info();
			}

			while(s < lim) {
				wchar c = *s++;
				if(c == 9) {
					*f++ = &pfmt;
					h->ascent = pfi.GetAscent();
					h->descent = pfi.GetDescent();
					h->external = pfi.GetExternal();
					*w++ = 0;
				}
				else
				if(IsLower(c)) {
					*f++ = &cfmt;
					c = (wchar)ToUpper(c);
					h->ascent = cfi.GetAscent();
					h->descent = cfi.GetDescent();
					h->external = cfi.GetExternal();
					*w++ = c >= 32 ? cwfi[c] : 0;
				}
				else {
					*f++ = &pfmt;
					h->ascent = fi.GetAscent();
					h->descent = fi.GetDescent();
					h->external = fi.GetExternal();
					*w++ = c >= 32 ? wfi[c] : 0;
				}
				h->object = NULL;
				*t++ = c;
				*p++ = pos;
				pos += pinc;
				h++;
			}
		}
		else {
			while(s < lim) {
				wchar c = *s++;
				*f++ = &pfmt;
				if(c == 9) {
					h->ascent = pfi.GetAscent();
					h->descent = pfi.GetDescent();
					h->external = pfi.GetExternal();
				}
				else {
					h->ascent = fi.GetAscent();
					h->descent = fi.GetDescent();
					h->external = fi.GetExternal();
				}
				h->object = NULL;
				*p++ = pos;
				pos += pinc;
				h++;
				*w++ = c >= 32 ? wfi[c] : 0;
				*t++ = c;
			}
		}
	}
}

static int CountChars(const Array<RichPara::Part>& part)
{
	int n = 0;
	for(int i = 0; i < part.GetCount(); i++) {
		const RichPara::Part& p = part[i];
		if(p.field)
			n += CountChars(p.fieldpart);
		else
			n += p.GetLength();
	}
	return n;
}

RichPara::Lines::Lines()
{
	justified = false;
	incache = false;
	cacheid = 0;
}

Array<RichPara::Lines>& RichPara::Lines::Cache()
{
	static Array<Lines> x;
	return x;
}

RichPara::Lines::~Lines()
{
	if(cacheid && line.GetCount() && !incache) {
		Mutex::Lock __(cache_lock);
		Array<Lines>& cache = Cache();
		incache = true;
		cache.Insert(0) = pick(*this);
//		cache.SetCount(1);
		int total = 0;
		for(int i = 1; i < cache.GetCount(); i++) {
			total += cache[i].clen;
			if(total > 10000 || i > 64) {
				cache.SetCount(i);
				break;
			}
		}
	}
}

RichPara::Lines RichPara::FormatLines(int acx) const
{
	Lines lines;
	if(cacheid) {
		Mutex::Lock __(cache_lock);
		Array<Lines>& cache = Lines::Cache();
		for(int i = 0; i < cache.GetCount(); i++)
			if(cache[i].cacheid == cacheid && cache[i].cx == acx) {
				lines = pick(cache[i]);
				lines.incache = false;
				cache.Remove(i);
				return lines;
			}
	}

	int i;
	lines.cacheid = cacheid;
	lines.cx = acx;
	lines.len = GetLength();
	lines.clen = CountChars(part);
	lines.first_indent = lines.next_indent = format.indent;
	if(format.bullet == BULLET_TEXT)
		lines.first_indent = 0;
	else
	if(!format.bullet && !format.IsNumbered())
		lines.next_indent = 0;

	FontInfo pfi = format.Info();
	if(lines.len == 0) {
		Line& l = lines.line.Add();
		l.pos = 0;
		l.ppos = 0;
		l.plen = 0;
		l.len = 0;
		l.cx = lines.first_indent;
		l.withtabs = false;
		HeightInfo dummy;
		Smh(lines, &dummy, lines.cx);
		l.ascent = pfi.GetAscent();
		l.descent = pfi.GetDescent();
		l.external = pfi.GetExternal();
		return lines;
	}

	lines.text.Alloc(lines.clen);
	lines.width.Alloc(lines.clen);
	lines.pos.Alloc(lines.clen);
	lines.format.Alloc(lines.clen);
	lines.height.Alloc(lines.clen);

	StorePart sp;
	sp.t = lines.text;
	sp.w = lines.width;
	sp.p = lines.pos;
	sp.f = lines.format;
	sp.h = lines.height;
	sp.pfi = pfi;
	sp.pos = 0;

	for(i = 0; i < part.GetCount(); i++)
		sp.Store(lines, part[i], 1);

	wchar *s = lines.text;
	wchar *text = s;
	wchar *end = lines.text + lines.clen;
	wchar *space = NULL;
	int *w = lines.width;
	int cx = lines.first_indent;
	int rcx = lines.cx - format.lm - format.rm;
	bool withtabs = false;
	int scx = cx;
	while(s < end) {
		Tab t;
		if(*s == ' ') {
			space = s;
			scx = cx;
		}
		else {
			if(*s == '\t') {
				t = GetNextTab(cx + format.lm, rcx);
				space = NULL;
			}
			if(*s == '\n' || // fieldparts with newline
			   cx + *w > rcx && s > text ||
			   *s == '\t' && (t.align == ALIGN_RIGHT ? t.pos - format.lm > rcx
			                                         : t.pos - format.lm >= rcx)) {
				if(*s == '\n') // fieldparts with newline
					space = NULL;Line& l = lines.line.Add();
				l.withtabs = withtabs;
				l.pos = (int)(text - lines.text);
				if(space) {
					l.len = (int)(space - text) + 1;
					l.cx = scx;
					text = s = space + 1;
				}
				else {
					l.len = (int)(s - text);
					l.cx = cx;
					text = s;
				}
				Smh(lines, lines.height, lines.cx);
				cx = lines.next_indent;
				w = text - ~lines.text + lines.width;
				space = NULL;
				rcx = lines.cx - format.lm - format.rm;
				withtabs = false;
				t = GetNextTab(cx + format.lm, acx);
			}
		}
		if(*s == '\t') {
			*s += t.fillchar;
			if(t.align == ALIGN_LEFT) {
				*w++ = t.pos - format.lm - cx;
				cx = t.pos - format.lm;
			}
			else {
				int tcx = 0;
				int *tw = w + 1;
				for(wchar *ts = s + 1; ts < end && *ts != '\t'; ts++)
					tcx += *tw++;
				int ww = t.pos - format.lm - cx - (t.align == ALIGN_RIGHT ? tcx : tcx / 2);
				if(ww > 0) {
					*w++ = ww;
					cx += ww;
				}
				else
					*w++ = 0;
			}
			withtabs = true;
		}
		else
			cx += *w++;
		s++;
	}
	Line& l = lines.line.Add();
	l.withtabs = withtabs;
	l.pos = (int)(text - lines.text);
	l.len = (int)(s - text);
	l.cx = cx;
	Smh(lines, lines.height, lines.cx);
	for(i = 0; i < lines.line.GetCount(); i++) {
		Line& l = lines.line[i];
		l.ppos = lines.pos[l.pos];
		l.plen = (l.pos + l.len < lines.clen ? lines.pos[l.pos + l.len] : lines.len) - l.ppos;
	}

	return lines;
}

void RichPara::Lines::Justify(const RichPara::Format& format)
{
	if(justified)
		return;
	justified = true;
	if(format.align != ALIGN_JUSTIFY) return;
	for(int i = 0; i < line.GetCount() - 1; i++) {
		const Line& li = line[i];
		if(!li.withtabs && li.len) {
			const wchar *s = ~text + li.pos;
			const wchar *lim = s + li.len;
			while(lim - 1 > s) {
				if(*(lim - 1) != ' ') break;
				lim--;
			}
			while(s < lim) {
				if(*s != ' ' && *s != 160) break;
				s++;
			}

			const wchar *beg = s;
			int nspc = 0;
			while(s < lim) {
				if(*s == ' ' || *s == 160) nspc++;
				s++;
			}
			s = beg;
			if(nspc) {
				int q = ((cx - format.lm - format.rm -
				          (i == 0 ? first_indent : next_indent) - li.cx) << 16)
				        / nspc;
				int *w = beg - ~text + width;
				int prec = 0;
				while(s < lim) {
					if(*s == ' ' || *s == 160) {
						*w += (prec + q) >> 16;
						prec = (prec + q) & 0xffff;
					}
					w++;
					s++;
				}
			}
		}
	}
}

int RichPara::Lines::BodyHeight() const
{
	int sum = 0;
	for(int i = 0; i < line.GetCount(); i++)
		sum += line[i].Sum();
	return sum;
}

}
