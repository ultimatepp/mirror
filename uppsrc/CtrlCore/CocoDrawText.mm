#include "CocoMM.h"

#ifdef GUI_COCOA

#define LLOG(x)
#define LTIMING(x)

namespace Upp {

CTFontRef CT_Font(Font fnt, bool& synth); // From Draw

void SystemDraw::DrawTextOp(int x, int y, int angle, const wchar *text, Font font, Color ink,
                            int n, const int *dx)
{
	Set(ink);

	bool synth;
	CFRef<CGFontRef> cgFont = CTFontCopyGraphicsFont(CT_Font(font, synth), NULL);
	
	CGContextSetFont(cgHandle, cgFont);

	Point off = GetOffset();
	CGAffineTransform tm = CGAffineTransformMakeTranslation(x + off.x, y + off.y);
	tm = CGAffineTransformScale(tm, 1, -1);
	tm = CGAffineTransformRotate(tm, M_2PI * angle / 3600);
	x = 0;
	y = -font.GetAscent();
	if(font.IsItalic() && synth) {
		x += font.GetDescent();
		tm = CGAffineTransformConcat(CGAffineTransformMake(1, 0, 0.2, 1, 0, 0), tm);
	}
	CGContextSetTextMatrix(cgHandle, tm);

	int nn = (1 + (synth && font.IsBold())) * n;
	Buffer<CGGlyph> g(nn);
	Buffer<CGPoint> p(nn);
	int ti = 0;
	for(int i = 0; i < n; i++) {
		int chr = text[i];
		GlyphInfo f = GetGlyphInfo(font, chr);
		p[ti].y = y;
		p[ti].x = x;
		g[ti] = f.glyphi;
		ti++;
		if(synth && font.IsBold()) {
			p[ti].y = y;
			p[ti].x = ++x;
			g[ti] = f.glyphi;
			ti++;
		}
		x += dx ? *dx++ : f.width;
	}

	CGContextSetFontSize(cgHandle, font.GetHeight());
    CGContextShowGlyphsAtPositions(cgHandle, g, p, nn); }
};

#endif