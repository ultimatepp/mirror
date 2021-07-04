#include "Painter.h"
#include "Painter.h"

#define LLOG(x)

namespace Upp {

#include "SvgInternal.h"

void SvgParser::ResolveGradient(int i)
{
	Gradient& g = gradient[i];
	if(g.resolved || g.href.GetCount() < 2)
		return;
	int q = gradient.Find(g.href.Mid(1));
	g.resolved = true;
	if(q < 0)
		return;
	ResolveGradient(q);
	Gradient& g2 = gradient[q];
	if(g.stop.GetCount() == 0)
		g.stop <<= g2.stop;
	g.a.x = Nvl(Nvl(g.a.x, g2.a.x));
	g.a.y = Nvl(Nvl(g.a.y, g2.a.y));
	g.b.x = Nvl(g.b.x, g2.b.x); // In user-space units, needs to be replaced by cx, in normal with 1
	g.b.y = Nvl(Nvl(g.b.y, g2.b.y));
	g.c.x = Nvl(Nvl(g.c.x, g2.c.x), 0.5);
	g.c.y = Nvl(Nvl(g.c.y, g2.c.y), 0.5);
	g.f.x = Nvl(Nvl(g.f.x, g2.f.x), g.c.x);
	g.f.y = Nvl(Nvl(g.f.y, g2.f.y), g.c.y);
	g.r = Nvl(Nvl(g.r, g2.r), 1.0);
	g.transform = Nvl(g.transform, g2.transform);
	g.style = Nvl(Nvl(g.style, g2.style), GRADIENT_PAD);
}

void SvgParser::StartElement(const XmlNode& n)
{
	State& s = state.Add();
	s = state[state.GetCount() - 2];
	s.n = current;
	current = &n;
	bp.Begin();
	bp.Transform(Transform(Txt("transform")));
	Style(Txt("style"));
	String classid = Txt("class");
	if(classid.GetCount())
		Style(classes.Get(classid, String()));
	for(int i = 0; i < n.GetAttrCount(); i++)
		ProcessValue(n.AttrId(i), n.Attr(i));
	closed = false;
}

void SvgParser::EndElement()
{
	if(!closed) {
		sw.Stroke(0, Black()); // Finish path to allow new transformations, if not yet done
	}
	current = state.Top().n;
	state.Drop();
	bp.End();
}

void SvgParser::DoGradient(int gi, bool stroke)
{
	State& s = state.Top();
	ResolveGradient(gi);
	Gradient& g = gradient[gi];
	if(g.stop.GetCount()) {
		for(int i = 0; i < g.stop.GetCount(); i++)
			sw.ColorStop(g.stop[i].offset, g.stop[i].color);
		Pointf a = g.a;
		Pointf b = g.b;
		Pointf c = g.c;
		Pointf f = g.f;
		Pointf r(g.r, g.r);
		Sizef sz = bp.boundingbox.GetSize();
		Pointf pos = bp.boundingbox.TopLeft();
		if(IsNull(b.x))
			b.x = g.user_space ? bp.boundingbox.right : 1.0;
		if(g.user_space) {
			a = (a - pos) / sz;
			b = (b - pos) / sz;
			c = (c - pos) / sz;
			f = (f - pos) / sz;
			r = r / sz;
		}
		Xform2D m;
		
		if(g.radial) {
			m.x.x = r.x;
			m.x.y = 0;
			m.y.x = 0;
			m.y.y = r.y;
			m.t = c;
			f = (f - c) / r;
		}
		else {
			Pointf d = b - a;
			m.x.x = d.x;
			m.x.y = -d.y;
			m.y.x = d.y;
			m.y.y = d.x;
			m.t = a;
		}
		m = m * Xform2D::Scale(sz.cx, sz.cy) * Xform2D::Translation(pos.x, pos.y);
		if(g.transform.GetCount())
			m = m * Transform(g.transform);
		RGBA c1 = g.stop[0].color;
		RGBA c2 = g.stop.Top().color;
		if(stroke)
			if(g.radial)
				sw.Stroke(s.stroke_width, f, c1, c2, m, g.style);
			else
				sw.Stroke(s.stroke_width, c1, c2, m, g.style);
		else
			if(g.radial)
				sw.Fill(f, c1, c2, m, g.style);
			else
				sw.Fill(c1, c2, m, g.style);
		bp.Finish(stroke * s.stroke_width);
		sw.ClearStops();
		closed = true;
	}
}

void SvgParser::StrokeFinishElement()
{
	State& s = state.Top();
	if(s.stroke_width > 0) {
		double o = s.opacity * s.stroke_opacity;
		if(o != 1) {
			sw.Begin();
			sw.Opacity(o);
		}
		if(s.stroke_gradient >= 0 && s.stroke_width > 0)
			DoGradient(s.stroke_gradient, true);
		else
		if(!IsNull(s.stroke) && s.stroke_width > 0) {
			sw.Stroke(s.stroke_width, s.stroke);
			bp.Finish(s.stroke_width);
			closed = true;
		}
		if(o != 1)
			sw.End();
	}
	EndElement();
}

void SvgParser::FinishElement()
{
	State& s = state.Top();
	double o = s.opacity * s.fill_opacity;
	if(o > 0) {
		if(o != 1) {
			sw.Begin();
			sw.Opacity(o);
		}
		if(s.fill_gradient >= 0)
			DoGradient(s.fill_gradient, false);
		else
		if(!IsNull(s.fill)) {
			sw.Fill(s.fill);
			bp.Finish(0);
			closed = true;
		}
		if(o != 1)
			sw.End();
	}
	StrokeFinishElement();
}

void SvgParser::ParseGradient(const XmlNode& n, bool radial)
{
	LLOG("ParseGradient " << n.Attr("id"));
	Gradient& g = gradient.Add(n.Attr("id"));
	g.radial = radial;
	g.user_space = n.Attr("gradientUnits") == "userSpaceOnUse";
	g.transform = n.Attr("gradientTransform");
	g.href = n.Attr("xlink:href");
	g.resolved = IsNull(g.href);
	double def = g.resolved ? 0.0 : (double)Null;
	double def5 = g.resolved ? 0.5 : (double)Null;
	auto Dbl = [&](const char *id, double def) { return Nvl(StrDbl(n.Attr(id)), def); };
	g.c.x = Dbl("cx", def5);
	g.c.y = Dbl("cy", def5);
	g.r = Dbl("r", g.resolved ? 1.0 : (double)Null);
	g.f.x = Dbl("fx", g.c.x);
	g.f.y = Dbl("fy", g.c.y);
	g.a.x = Dbl("x1", def);
	g.a.y = Dbl("y1", def);
	g.b.x = Dbl("x2", Null);
	g.b.y = Dbl("y2", def);
	g.style = decode(Txt("spreadMethod"), "pad", GRADIENT_PAD, "reflect", GRADIENT_REFLECT,
	                 "repeat", GRADIENT_REPEAT, (int)Null);

	for(const XmlNode& m : n)
		if(m.IsTag("stop")) {
			Stop &s = g.stop.Add();
			double offset = 0;
			String st = m.Attr("style");
			const char *style = st;
			double opacity = 1;
			Color  color;
			String key, value;
			for(;;) {
				if(*style == ';' || *style == '\0') {
					value = TrimBoth(value);
					if(key == "stop-color")
						color = GetColor(value);
					else
					if(key == "stop-opacity")
						opacity = StrDbl(value);
					else
					if(key == "offset")
						offset = StrDbl(value);
					value.Clear();
					key.Clear();
					if(*style == '\0')
						break;
					else
						style++;
				}
				else
				if(*style == ':') {
					key << TrimBoth(value);
					value.Clear();
					style++;
				}
				else
					value.Cat(*style++);
			}
			value = m.Attr("stop-color");
			if(value.GetCount())
				color = GetColor(value);
			value = m.Attr("stop-opacity");
			if(value.GetCount())
				opacity = Nvl(StrDbl(value), opacity);
			s.color = clamp(int(opacity * 255 + 0.5), 0, 255) * color;
			s.offset = Nvl(StrDbl(m.Attr("offset")), offset);
		}
}

void SvgParser::Poly(const XmlNode& n, bool line)
{
	Vector<Point> r;
	String value = n.Attr("points");
	try {
		CParser p(value);
		while(!p.IsEof()) {
			Pointf n;
			n.x = p.ReadDouble();
			p.Char(',');
			n.y = p.ReadDouble();
			r.Add(n);
			p.Char(',');
		}
	}
	catch(CParser::Error) {}
	if(r.GetCount()) {
		StartElement(n);
		bp.Move(r[0].x, r[0].y);
		for(int i = 1; i < r.GetCount(); ++i)
			bp.Line(r[i].x, r[i].y);
		if(!line)
			bp.Close();
		if(line)
			StrokeFinishElement();
		else
			FinishElement();
	}
}

double ReadNumber(CParser& p)
{
	while(!p.IsEof() && (!p.IsDouble() || p.IsChar('.')))
		p.SkipTerm();
	return p.ReadDouble();
}

Rectf GetSvgViewBox(const String& v)
{
	Rectf r = Null;
	if(v.GetCount()) {
		try {
			CParser p(v);
			r.left = ReadNumber(p);
			r.top = ReadNumber(p);
			r.right = r.left + ReadNumber(p);
			r.bottom = r.top + ReadNumber(p);
		}
		catch(CParser::Error) {
			r = Null;
		}
	}
	return r;
}

Rectf GetSvgViewBox(XmlParser& xml)
{
	return GetSvgViewBox(xml["viewBox"]);
}

Rectf GetSvgViewBox(const XmlNode& xml)
{
	return GetSvgViewBox(xml.Attr("viewBox"));
}

Sizef GetSvgSize(XmlParser& xml)
{
	Sizef sz;
	sz.cx = StrDbl(xml["width"]);
	sz.cy = StrDbl(xml["height"]);
	if(IsNull(sz.cx) || IsNull(sz.cy))
		sz = Null;
	return sz;
}

Pointf GetSvgPos(XmlParser& xml)
{
	Pointf p;
	p.x = StrDbl(xml["x"]);
	p.y = StrDbl(xml["y"]);
	if(IsNull(p.x) || IsNull(p.y))
		p = Null;
	return p;
}

Sizef GetSvgSize(const XmlNode& xml)
{
	Sizef sz;
	sz.cx = StrDbl(xml.Attr("width"));
	sz.cy = StrDbl(xml.Attr("height"));
	if(IsNull(sz.cx) || IsNull(sz.cy))
		sz = Null;
	return sz;
}

Pointf GetSvgPos(const XmlNode& xml)
{
	Pointf p;
	p.x = StrDbl(xml.Attr("x"));
	p.y = StrDbl(xml.Attr("y"));
	if(IsNull(p.x) || IsNull(p.y))
		p = Null;
	return p;
}

void SvgParser::Element(const XmlNode& n, int depth, bool dosymbols)
{
	if(depth > 100) // defend against id recursion
		return;
	LLOG("====== " << n.GetTag());
	if(n.IsTag("defs")) {
		for(const auto& m : n)
			if(m.IsTag("linearGradient"))
				ParseGradient(m, false);
			else
			if(m.IsTag("radialGradient"))
				ParseGradient(m, true);
	}
	else
	if(n.IsTag("linearGradient"))
		ParseGradient(n, false);
	else
	if(n.IsTag("radialGradient"))
		ParseGradient(n, true);
	else
	if(n.IsTag("rect")) {
		StartElement(n);
		bp.RoundedRectangle(Dbl("x"), Dbl("y"), Dbl("width"), Dbl("height"), Dbl("rx"), Dbl("ry"));
		FinishElement();
	}
	else
	if(n.IsTag("ellipse")) {
		StartElement(n);
		bp.Ellipse(Dbl("cx"), Dbl("cy"), Dbl("rx"), Dbl("ry"));
		FinishElement();
	}
	else
	if(n.IsTag("circle")) {
		StartElement(n);
		Pointf c(Dbl("cx"), Dbl("cy"));
		double r = Dbl("r");
		bp.Ellipse(c.x, c.y, r, r);
		FinishElement();
	}
	else
	if(n.IsTag("line")) {
		StartElement(n);
		Pointf a(Dbl("x1"), Dbl("y1"));
		Pointf b(Dbl("x2"), Dbl("y2"));
		bp.Move(a);
		bp.Line(b);
		FinishElement();
	}
	else
	if(n.IsTag("polygon"))
		Poly(n, false);
	else
	if(n.IsTag("polyline"))
		Poly(n, true);
	else
	if(n.IsTag("path")) {
		StartElement(n);
		bp.Path(Txt("d"));
		FinishElement();
	}
	else
	if(n.IsTag("image")) {
		StartElement(n);
		String fileName = Txt("xlink:href");
		String data;
		resloader(fileName, data);
		if(data.GetCount()) {
			Image img = StreamRaster::LoadFileAny(fileName);
			if(!IsNull(img)) {
				bp.Rectangle(Dbl("x"), Dbl("y"), Dbl("width"), Dbl("height"));
				sw.Fill(StreamRaster::LoadFileAny(fileName), Dbl("x"), Dbl("y"), Dbl("width"), 0);
			}
		}
		EndElement();
	}
	else
	if(n.IsTag("text")) {
		StartElement(n);
		auto DoText = [&](const XmlNode& n) {
			String text = n.GatherText();
			text.Replace("\n", " ");
			text.Replace("\r", "");
			text.Replace("\t", " ");
			if(text.GetCount()) {
				Font fnt = state.Top().font;
				int anchor = state.Top().text_anchor;
				double x = Dbl("x");
				if(anchor) {
					Sizef sz = GetTextSize(text, fnt); // TODO; GetTextSizef
					x -= anchor == 1 ? sz.cx / 2 : sz.cx;
				}
				bp.Text(x	, Dbl("y") - fnt.GetAscent(), text, fnt);
			}
		};
		DoText(n);
		for(const auto& m : n)
			if(m.IsTag("tspan")) {
				StartElement(m);
				DoText(m);
				FinishElement();
			}
		FinishElement();
	}
	else
	if(n.IsTag("g") || n.IsTag("symbol") && dosymbols)
		Items(n, depth);
	else
	if(n.IsTag("use")) {
		const XmlNode *idn = idmap.Get(Nvl(n.Attr("href"), n.Attr("xlink:href")), NULL);
		if(idn) {
			StartElement(n);
			Rectf vr = GetSvgViewBox(*idn);
			Sizef sz = GetSvgSize(*idn);
			bp.Translate(Dbl("x"), Dbl("y"));
			if(!IsNull(vr) && !IsNull(sz)) {
				bp.Rectangle(0, 0, sz.cx, sz.cy).Clip();
				sz /= vr.GetSize();
				bp.Scale(sz.cx, sz.cy);
				bp.Translate(-vr.left, -vr.top);
			}
			Element(*idn, depth + 1, true);
			EndElement();
		}
	}
	else
	if(n.IsTag("svg")) {
		Sizef sz = GetSvgSize(n);
		if(!IsNull(sz)) {
			Pointf p = Nvl(GetSvgPos(n), Pointf(0, 0));
			Rectf vb = Nvl(GetSvgViewBox(n), sz);
			//TODO: For now, we support "xyMid meet" only
			bp.Translate(p.x, p.y);
			bp.Scale(min(sz.cx / vb.GetWidth(), sz.cy / vb.GetHeight()));
			bp.Translate(-vb.TopLeft());
			Items(n, depth);
		}
	}
	else
	if(n.IsTag("style")) {
		String text = n.GatherText();
		try {
			CParser p(text);
			while(!p.IsEof()) {
				if(p.Char('.') && p.IsId()) {
					String id = p.ReadId();
					if(p.Char('{')) {
						const char *b = p.GetPtr();
						while(!p.IsChar('}') && !p.IsEof())
							p.SkipTerm();
						classes.Add(id, String(b, p.GetPtr()));
					}
					p.Char('}');
				}
				else
					p.SkipTerm();
			}
		}
		catch(CParser::Error) {}
	}
}

void SvgParser::Items(const XmlNode& n, int depth)
{
	StartElement(n);
	for(const auto& m : n)
		Element(m, depth);
	EndElement();
}

void SvgParser::MapIds(const XmlNode& n)
{
	String id = n.Attr("id");
	if(id.GetCount())
		idmap.Add('#' + id, &n);
	for(const auto& m : n)
		MapIds(m);
}

bool SvgParser::Parse(const char *xml) {
	try {
		XmlNode n = ParseXML(xml);
		MapIds(n);
		for(const auto& m : n)
			if(m.IsTag("svg"))
				Items(m, 0);
	}
	catch(XmlError e) {
		return false;
	}
	return true;
}

SvgParser::SvgParser(Painter& sw)
:	sw(sw), bp(sw)
{
	Reset();
}

bool ParseSVG(Painter& p, const char *svg, Event<String, String&> resloader, Rectf *boundingbox)
{
	SvgParser sp(p);
	sp.bp.compute_svg_boundingbox = boundingbox;
	sp.resloader = resloader;
	if(!sp.Parse(svg))
		return false;
	if(boundingbox)
		*boundingbox = sp.bp.svg_boundingbox;
	return true;
}

bool RenderSVG(Painter& p, const char *svg, Event<String, String&> resloader)
{
	return ParseSVG(p, svg, resloader, NULL);
}

bool RenderSVG(Painter& p, const char *svg)
{
	return RenderSVG(p, svg, Event<String, String&>());
}

void GetSVGDimensions(const char *svg, Sizef& sz, Rectf& viewbox)
{
	viewbox = Null;
	sz = Null;
	try {
		XmlParser xml(svg);
		while(!xml.IsTag())
			xml.Skip();
		xml.PassTag("svg");
		viewbox = GetSvgViewBox(xml);
		sz = GetSvgSize(xml);
	}
	catch(XmlError e) {
	}
}

Rectf GetSVGBoundingBox(const char *svg)
{
	NilPainter nil;
	Rectf bb;
	if(!ParseSVG(nil, svg, Event<String, String&>(), &bb))
		return Null;
	return bb;
}

Image RenderSVGImage(Size sz, const char *svg, Event<String, String&> resloader)
{
	Rectf f = GetSVGBoundingBox(svg);
	Sizef iszf = GetFitSize(f.GetSize(), Sizef(sz.cx, sz.cy) - 10.0);
	Size isz((int)ceil(iszf.cx), (int)ceil(iszf.cy));
	if(isz.cx <= 0 || isz.cy <= 0)
		return Null;
	ImageBuffer ib(isz);
	BufferPainter sw(ib);
	sw.Clear(White());
	sw.Scale(min(isz.cx / f.GetWidth(), isz.cy / f.GetHeight()));
	sw.Translate(-f.left, -f.top);
	RenderSVG(sw, svg, resloader);
	return ib;
}

Image RenderSVGImage(Size sz, const char *svg)
{
	return RenderSVGImage(sz, svg, Event<String, String&>());
}

bool IsSVG(const char *svg)
{
	try {
		XmlParser xml(svg);
		while(!xml.IsTag())
			xml.Skip();
		if(xml.Tag("svg"))
			return true;
	}
	catch(XmlError e) {
	}
	return false;
}

Rectf GetSVGPathBoundingBox(const char *path)
{
	NilPainter nilp;
	BoundsPainter p(nilp);
	p.Path(path).Fill(Black());
	return p.Get();
}

}
