#include "Painter.h"

namespace Upp {

struct PaintCharPath : FontGlyphConsumer {
	Painter *sw;
	
	virtual void Move(Pointf p) {
		sw->Move(p);
	}
	virtual void Line(Pointf p) {
		sw->Line(p);
	}
	virtual void Quadratic(Pointf p1, Pointf p2) {
		sw->Quadratic(p1, p2);
	}
	virtual void Cubic(Pointf p1, Pointf p2, Pointf p3) {
		sw->Cubic(p1, p2, p3);
	}
	virtual void Close() {
		sw->Close();
	}
};

void PaintCharacter(Painter& sw, const Pointf& p, int chr, Font font)
{
	GlyphInfo gi = GetGlyphInfo(font, chr);
	PaintCharPath pw;
	pw.sw = &sw;
	if(gi.IsNormal())
		font.Render(pw, p.x, p.y, chr);
	else
	if(gi.IsReplaced()) {
		Font fnt = font;
		fnt.Face(gi.lspc);
		fnt.Height(gi.rspc);
		fnt.Render(pw, p.x, p.y + font.GetAscent() - fnt.GetAscent(), chr);
	}
	else
	if(gi.IsComposed()) {
		ComposedGlyph cg;
		Compose(font, chr, cg);
		font.Render(pw, p.x, p.y, cg.basic_char);
		sw.Div();
		cg.mark_font.Render(pw, p.x + cg.mark_pos.x, p.y + cg.mark_pos.y, cg.mark_char);
	}
	sw.EvenOdd(true);
}

Xform2D GetLineSzXform(const Pointf& p1, const Pointf& p2, const Sizef& sz)
{
	Xform2D m = Xform2D::Scale(Distance(p1, p2) / sz.cx);
	m = m * Xform2D::Rotation(Bearing(p2 - p1));
	m = m * Xform2D::Translation(p1.x, p1.y);
	return m;
}

Painter& Painter::Fill(const Image& image, Pointf p1, Pointf p2, dword flags)
{
	return Fill(image, GetLineSzXform(p1, p2, image.GetSize()), flags);
}

Painter& Painter::Fill(const Image& image, double x1, double y1,
                       double x2, double y2, dword flags)
{
	return Fill(image, Pointf(x1, y1), Pointf(x2, y2), flags);
}

Painter& Painter::Fill(double x1, double y1, const RGBA& color1, double x2, double y2, const RGBA& color2, int style)
{
	return Fill(Pointf(x1, y1), color1, Pointf(x2, y2), color2, style);
}

Painter& Painter::Fill(const RGBA& color1, const RGBA& color2, const Xform2D& transsrc, dword flags)
{
	FillOp(color1, color2, transsrc, flags);
	return *this;
}

Painter& Painter::Fill(double fx, double fy, const RGBA& color1, double cx, double cy, double r, const RGBA& color2, int style)
{
	return Fill(Pointf(fx, fy), color1, Pointf(cx, cy), r, color2, style);
}

Painter& Painter::Fill(const Pointf& c, const RGBA& color1, double r, const RGBA& color2, int style)
{
	return Fill(c, color1, c, r, color2, style);
}

Painter& Painter::Fill(double x, double y, const RGBA& color1, double r, const RGBA& color2, int style)
{
	return Fill(Pointf(x, y), color1, r, color2, style);
}

Painter& Painter::Fill(const Pointf& f, const RGBA& color1, const RGBA& color2, const Xform2D& transsrc, int style)
{
	FillOp(f, color1, color2, transsrc, style);
	return *this;
}

Painter& Painter::Stroke(double width, const Pointf& f, const RGBA& color1, const RGBA& color2, const Xform2D& transsrc, int style)
{
	StrokeOp(width, f, color1, color2, transsrc, style);
	return *this;
}

Painter& Painter::Translate(double x, double y)
{
	Transform(Xform2D::Translation(x, y));
	return *this;
}

Painter& Painter::Translate(const Pointf& p)
{
	return Translate(p.x, p.y);
}

Painter& Painter::Stroke(double width, const Image& image, const Pointf& p1, const Pointf& p2, dword flags)
{
	return Stroke(width, image, GetLineSzXform(p1, p2, image.GetSize()), flags);
}

Painter& Painter::Stroke(double width, const Image& image, double x1, double y1, double x2, double y2, dword flags)
{
	return Stroke(width, image, Pointf(x1, y1), Pointf(x2, y2), flags);
}

Painter& Painter::Stroke(double width, double x1, double y1, const RGBA& color1, double x2, double y2, const RGBA& color2, int style)
{
	return Stroke(width, Pointf(x1, y1), color1, Pointf(x2, y2), color2, style);
}

Painter& Painter::Stroke(double width, const RGBA& color1, const RGBA& color2, const Xform2D& transsrc, dword flags)
{
	StrokeOp(width, color1, color2, transsrc, flags);
	return *this;
}

Painter& Painter::Stroke(double width, double fx, double fy, const RGBA& color1, double cx, double cy, double r, const RGBA& color2, int style)
{
	return Stroke(width, Pointf(fx, fy), color1, Pointf(cx, cy), r, color2, style);
}

Painter& Painter::Stroke(double width, const Pointf& c, const RGBA& color1, double r, const RGBA& color2, int style)
{
	return Stroke(width, c, color1, c, r, color2, style);
}

Painter& Painter::Stroke(double width, double x, double y, const RGBA& color1, double r, const RGBA& color2, int style)
{
	return Stroke(width, Pointf(x, y), color1, r, color2, style);
}

Painter& Painter::Rotate(double a)
{
	Transform(Xform2D::Rotation(a));
	return *this;
}

Painter& Painter::Scale(double scalex, double scaley)
{
	Transform(Xform2D::Scale(scalex, scaley));
	return *this;
}

Painter& Painter::Scale(double scale)
{
	Scale(scale, scale);
	return *this;
}

Vector<double> StringToDash(const String& dash, double& start)
{
	Vector<double> d;
	CParser p(dash);
	try {
		while(!p.IsEof())
			if(p.Char(':'))
				start = p.ReadDouble();
			else
				d.Add(p.ReadDouble());
	}
	catch(CParser::Error) {}
	if(d.GetCount() & 1) {
		Vector<double> dash1;
		dash1.Append(d);
		dash1.Append(d);
		return dash1;
	}
	return d;
}

void Painter::DashOp(const String& dash, double start)
{
	Vector<double> h = StringToDash(dash, start);
	DashOp(h, start);
}

Painter& Painter::Character(double x, double y, int ch, Font fnt)
{
	return Character(Pointf(x, y), ch, fnt);
}

void Painter::TextOp(const Pointf& p, const wchar *text, Font fnt, int n, const double *dx)
{
	if(n == 0) {
		Move(0, 0);
		return;
	}
	Font fi = fnt;
	double m = 1;
	if(fnt.GetHeight() < 50) {
		fi.Height(100 * fnt.GetHeight());
		m = 0.01;
	}
	else
	if(fnt.GetHeight() < 500) {
		fi.Height(10 * fnt.GetHeight());
		m = 0.1;
	}
	double x = p.x;
	while(n) {
		int ch = *text++;
		Character(x, p.y, ch, fnt);
		Div();
		if(dx)
			x += *dx++;
		else
			x += fi[ch] * m;
		n--;
	}
	if(fnt.IsUnderline() || fnt.IsStrikeout()) {
		double a = fnt.GetAscent();
		double cy = max(a / 16, 1.0);
		double cx = x - p.x;
		if(fnt.IsUnderline())
			Rectangle(p.x, p.y + a + cy, cx, cy);
		if(fnt.IsStrikeout())
			Rectangle(p.x, p.y + 2 * a / 3, cx, cy);
	}
}

Painter& Painter::Text(double x, double y, const wchar *text, Font fnt, int n, const double *dx)
{
	return Text(Pointf(x, y), text, fnt, n < 0 ? wstrlen(text) : n, dx);
}

Painter& Painter::Text(const Pointf& p, const WString& s, Font fnt, const double *dx)
{
	return Text(p, ~s, fnt, s.GetLength(), dx);
}

Painter& Painter::Text(double x, double y, const WString& s, Font fnt, const double *dx)
{
	return Text(Pointf(x, y), s, fnt, dx);
}

Painter& Painter::Text(const Pointf& p, const String& s, Font fnt, const double *dx)
{
	return Text(p, s.ToWString(), fnt, dx);
}

Painter& Painter::Text(double x, double y, const String& s, Font fnt, const double *dx)
{
	return Text(Pointf(x, y), s, fnt, dx);
}

Painter& Painter::Text(const Pointf& p, const char *text, Font fnt, int n, const double *dx)
{
	WString s = ToUnicode(text, CHARSET_DEFAULT);
	return Text(p, s, fnt, n < 0 ? s.GetCount() : n, dx);
}

Painter& Painter::Text(double x, double y, const char *text, Font fnt, int n, const double *dx)
{
	return Text(Pointf(x, y), text, fnt, n, dx);
}

Painter& Painter::Rectangle(double x, double y, double cx, double cy)
{
	if (cx < 0) { x += cx; cx = -cx;}
	if (cy < 0) { y += cy; cy = -cy;}
	return Move(x, y).RelLine(cx, 0).RelLine(0, cy).RelLine(-cx, 0).Close();
}

Painter& Painter::RoundedRectangle(double x, double y, double cx, double cy, double r)
{
	return RoundedRectangle(x, y, cx, cy, r, r);
}

Painter& Painter::RoundedRectangle(double x, double y, double cx, double cy, double rx, double ry)
{
	ASSERT(rx >= 0 && ry >= 0);
	if (cx < 0) { x += cx; cx = -cx;}
	if (cy < 0) { y += cy; cy = -cy;}
	Move(x + rx, y).Arc(x + rx, y + ry, rx, ry, -M_PI / 2, -M_PI / 2)
	.Line(x, y + cy - ry).Arc(x + rx, y + cy - ry, rx, ry, M_PI, -M_PI / 2)
	.Line(x + cx - rx, y + cy).Arc(x + cx - rx, y + cy - ry, rx, ry, M_PI / 2, -M_PI / 2)
	.Line(x + cx, y + ry).Arc(x + cx - rx, y + ry, rx, ry, 0, -M_PI / 2).Line(x + rx, y);
	return *this;
}

Painter& Painter::Ellipse(double x, double y, double rx, double ry)
{
	return Move(x + rx, y).Arc(x, y, rx, ry, 0, 2 * M_PI).Close();
}

Painter& Painter::Circle(double x, double y, double r)
{
	return Ellipse(x, y, r, r);
}

void NilPainter::ClearOp(const RGBA& color) {}
void NilPainter::MoveOp(const Pointf& p, bool rel) {}
void NilPainter::LineOp(const Pointf& p, bool rel) {}
void NilPainter::QuadraticOp(const Pointf& p1, const Pointf& p, bool rel) {}
void NilPainter::QuadraticOp(const Pointf& p, bool rel) {}
void NilPainter::CubicOp(const Pointf& p1, const Pointf& p2, const Pointf& p, bool rel) {}
void NilPainter::CubicOp(const Pointf& p2, const Pointf& p, bool rel) {}
void NilPainter::ArcOp(const Pointf& c, const Pointf& r, double angle, double sweep, bool rel) {}
void NilPainter::SvgArcOp(const Pointf& r, double xangle, bool large, bool sweep, const Pointf& p, bool rel) {}
void NilPainter::CloseOp() {}
void NilPainter::DivOp() {}
void NilPainter::FillOp(const RGBA& color) {}
void NilPainter::FillOp(const Image& image, const Xform2D& transsrc, dword flags) {}
void NilPainter::FillOp(const RGBA& color1, const RGBA& color2, const Xform2D& transsrc, int style) {}
void NilPainter::FillOp(const Pointf& p1, const RGBA& color1, const Pointf& p2, const RGBA& color2, int style) {}
void NilPainter::FillOp(const Pointf& f, const RGBA& color1, const Pointf& c, double r, const RGBA& color2, int style) {}
void NilPainter::FillOp(const Pointf& f, const RGBA& color1, const RGBA& color2, const Xform2D& transsrc, int style) {}
void NilPainter::StrokeOp(double width, const RGBA& rgba) {}
void NilPainter::StrokeOp(double width, const Image& image, const Xform2D& transsrc, dword flags) {}
void NilPainter::StrokeOp(double width, const RGBA& color1, const RGBA& color2, const Xform2D& transsrc, int style) {}
void NilPainter::StrokeOp(double width, const Pointf& p1, const RGBA& color1, const Pointf& p2, const RGBA& color2, int style) {}
void NilPainter::StrokeOp(double width, const Pointf& f, const RGBA& color1, const Pointf& c, double r, const RGBA& color2, int style) {}
void NilPainter::StrokeOp(double width, const Pointf& f, const RGBA& color1, const RGBA& color2, const Xform2D& transsrc, int style) {}
void NilPainter::ClipOp() {}
void NilPainter::CharacterOp(const Pointf& p, int ch, Font fnt) {}
void NilPainter::TextOp(const Pointf& p, const wchar *text, Font fnt, int n, const double *dx) {}
void NilPainter::ColorStopOp(double pos, const RGBA& color) {}
void NilPainter::ClearStopsOp() {}
void NilPainter::OpacityOp(double o) {}
void NilPainter::LineCapOp(int linecap) {}
void NilPainter::LineJoinOp(int linejoin) {}
void NilPainter::MiterLimitOp(double l) {}
void NilPainter::EvenOddOp(bool evenodd) {}
void NilPainter::InvertOp(bool invert) {}
void NilPainter::DashOp(const Vector<double>& dash, double start) {}
void NilPainter::TransformOp(const Xform2D& m) {}
void NilPainter::BeginOp() {}
void NilPainter::EndOp() {}
void NilPainter::BeginMaskOp() {}
void NilPainter::BeginOnPathOp(double, bool) {}

ImagePainter::ImagePainter(Size sz, int mode)
:	ImageBuffer__(sz), BufferPainter(ImageBuffer__::ib, mode)
{}

ImagePainter::ImagePainter(int cx, int cy, int mode)
:	ImageBuffer__(Size(cx, cy)), BufferPainter(ImageBuffer__::ib, mode)
{}

DrawPainter::DrawPainter(Draw& w, Size sz, int mode)
:	ImagePainter(sz, mode), w(w)
{}

DrawPainter::~DrawPainter()
{
	Finish();
	GetBuffer().PaintOnceHint();
	w.DrawImage(0, 0, *this);
}
	
}
