#include "CocoMM.h"

#ifdef GUI_COCOA

namespace Upp {

void SystemDraw::Stroke(int width, Color color, bool fill)
{
	static double dash[] = { 18, 6 };
	static double dot[] = { 3, 3 };
	static double dashdot[] = { 9, 6, 3, 6 };
	static double dashdotdot[] = { 9, 3, 3, 3, 3, 3 };
	if(IsNull(width))
		width = PEN_NULL;
	switch(width) {
	case PEN_NULL:       if(!fill) return;
	case PEN_DASH:       CGContextSetLineDash(cgHandle, 0, dash, __countof(dash)); break;
	case PEN_DOT:        CGContextSetLineDash(cgHandle, 0, dot, __countof(dot)); break;
	case PEN_DASHDOT:    CGContextSetLineDash(cgHandle, 0, dashdot, __countof(dashdot)); break;
	case PEN_DASHDOTDOT: CGContextSetLineDash(cgHandle, 0, dashdotdot, __countof(dashdotdot)); break;
	default:             break;
	}
	if(fill && width == 0)
		width = PEN_NULL;
	else
	    CGContextSetLineWidth(cgHandle, width > 0 ? width : 1);
    SetStroke(color);
    CGContextDrawPath(cgHandle, fill ? width == PEN_NULL ? kCGPathFill : kCGPathFillStroke
                                     : kCGPathStroke);
    if(width < 0)
        CGContextSetLineDash(cgHandle, 0, NULL, 0);
}

void SystemDraw::DrawLineOp(int x1, int y1, int x2, int y2, int width, Color color)
{
	if(IsNull(color))
		return;
	CGPoint p[2];
	p[0] = Convert(x1, y1);
	p[1] = Convert(x2, y2);
	CGContextAddLines(cgHandle, p, 2);
	Stroke(width, color, false);
}

void SystemDraw::DoPath(const Point *pp, const Point *end)
{
	CGPoint p = Convert(pp->x, pp->y);
	CGContextMoveToPoint(cgHandle, p.x, p.y);
	while(++pp < end) {
		p = Convert(pp->x, pp->y);
		CGContextAddLineToPoint(cgHandle, p.x, p.y);
	}
}

void SystemDraw::DrawPolyPolylineOp(const Point *vertices, int vertex_count, const int *counts, int count_count, int width, Color color, Color doxor)
{
	if(vertex_count < 2 || IsNull(color))
		return;
	while(--count_count >= 0) {
		const Point *pp = vertices;
		vertices += *counts++;
		DoPath(pp, vertices);
		Stroke(width, color, false);
	}
}

void SystemDraw::DrawPolyPolyPolygonOp(const Point *vertices, int vertex_count, const int *subpolygon_counts, int scc, const int *disjunct_polygon_counts, int dpcc, Color color, int width, Color outline, uint64 pattern, Color doxor)
{
	Set(color);
	while(--dpcc >= 0) {
		const Point *sp = vertices;
		vertices += *disjunct_polygon_counts++;
		while(sp < vertices) {
			const Point *pp = sp;
			sp += *subpolygon_counts++;
			DoPath(pp, sp);
			CGContextClosePath(cgHandle);
		}
		Stroke(width, outline, true);
	}
}

void SystemDraw::DrawArcOp(const Rect& rc, Point start, Point end, int width, Color color)
{
	if(rc.Width() <= 0 || rc.Height() <= 0)
		return;

	Sizef radius = Sizef(rc.Size()) / 2.0;
	Pointf center = Pointf(rc.TopLeft()) + radius;
	double ang1 = Bearing((Pointf(start) - center) / radius);
	double ang2 = Bearing((Pointf(end) - center) / radius);

	CGContextSaveGState(cgHandle);
	CGPoint p = Convert(rc.left + radius.cx, rc.top + radius.cy);
	CGContextTranslateCTM(cgHandle, p.x, p.y);
	CGContextScaleCTM(cgHandle, radius.cx, -radius.cy);
	CGContextAddArc(cgHandle, 0, 0, 1, ang1, ang2, 1);
	CGContextRestoreGState(cgHandle);
	Stroke(width, color, false);
}

void SystemDraw::DrawEllipseOp(const Rect& r, Color color, int pen, Color pencolor)
{
	if(IsNull(color) && (pen <= 0 || IsNull(pencolor)))
		return;
	Set(color);
    CGContextSetLineWidth(cgHandle, pen);
    if(pen > 0)
	    SetStroke(pencolor);
    CGContextBeginPath(cgHandle);
    CGContextAddEllipseInRect(cgHandle, Convert(r));
    Stroke(pen, pencolor, true);
}

/*
void SystemDraw::Flush()
{
	CGContextSynchronize(cgHandle);
}
*/

};

#endif
