#define GUI_COCOA

namespace Upp {

struct PointCG; // represents CGPoint, used to isolate Cocoa/AppKit includes
struct RectCG; // represents CGRect, used to isolate Cocoa/AppKit includes

class SystemDraw : public Draw {
	virtual dword GetInfo() const;

	virtual void BeginOp();
	virtual void EndOp();
	virtual void OffsetOp(Point p);
	virtual bool ClipOp(const Rect& r);
	virtual bool ClipoffOp(const Rect& r);
	virtual bool ExcludeClipOp(const Rect& r);
	virtual bool IntersectClipOp(const Rect& r);
	virtual bool IsPaintingOp(const Rect& r) const;
	virtual Rect GetPaintRect() const;

	virtual	void DrawRectOp(int x, int y, int cx, int cy, Color color);
	virtual void SysDrawImageOp(int x, int y, const Image& img, Color color);
	virtual void DrawLineOp(int x1, int y1, int x2, int y2, int width, Color color);

	virtual void DrawPolyPolylineOp(const Point *vertices, int vertex_count,
	                                const int *counts, int count_count,
	                                int width, Color color, Color doxor);
	virtual void DrawPolyPolyPolygonOp(const Point *vertices, int vertex_count,
	                                   const int *subpolygon_counts, int scc,
	                                   const int *disjunct_polygon_counts, int dpcc,
	                                   Color color, int width, Color outline,
	                                   uint64 pattern, Color doxor);
	virtual void DrawArcOp(const Rect& rc, Point start, Point end, int width, Color color);

	virtual void DrawEllipseOp(const Rect& r, Color color, int pen, Color pencolor);
	virtual void DrawTextOp(int x, int y, int angle, const wchar *text, Font font,
		                    Color ink, int n, const int *dx);

private:
	Vector<Point> offset;
	Vector<Rect>  clip;

	Color  fill = Null;

	void   Push();
	void   Pop();
	
	void  *handle;
	void  *nsview;
	
	RectCG MakeRectCG(const Rect& r) const;
	void   ClipCG(const Rect& r);
	Rect   GetClip() const         { return clip.GetCount() ? clip.Top() : Rect(-999999, -999999, 999999, 999999); }
	RectCG Convert(int x, int y, int cx, int cy);
	RectCG Convert(const Rect& r);
	PointCG Convert(int x, int y);

	void  Set(Color c);
	void  SetStroke(Color c);

	void Init(void *cgContext, void *nsview);

	void Stroke(int width, Color color, bool fill);
	void DoPath(const Point *pp, const Point *end);

	SystemDraw() {}
	
	friend class ImageDraw;
	friend class BackDraw;
	friend class TopFrameDraw;
	friend struct BackDraw__;

public:
	Point    GetOffset() const       { return offset.GetCount() ? offset.Top() : Point(0, 0); }
	void    *GetCGHandle() const      { return handle; }

	bool     CanSetSurface()          { return false; }
	static void Flush() {}

	SystemDraw(void *cgContext, void *nsview);
	~SystemDraw();
};

 
inline void SetSurface(SystemDraw& w, const Rect& dest, const RGBA *pixels, Size srcsz, Point poff)
{ // TODO: Unless we can do this...
	NEVER();
}

class ImageDraw : public SystemDraw {
	ImageBuffer ib;
	
	One<ImageDraw> alpha;

	void Init(int cx, int cy);

public:
	Draw& Alpha();

	operator Image() const;

	Image GetStraight() const;

	ImageDraw(Size sz);
	ImageDraw(int cx, int cy);
	~ImageDraw();
};

struct BackDraw__ : public SystemDraw {
	BackDraw__() : SystemDraw() {}
};

class BackDraw : public BackDraw__ { // Dummy only, as we are running in GlobalBackBuffer mode
	Size        size;
	Draw       *painting;
	Point       painting_offset;
	ImageBuffer ib;
	
public:
	virtual bool  IsPaintingOp(const Rect& r) const;

public:
	void  Put(SystemDraw& w, int x, int y)             {}
	void  Put(SystemDraw& w, Point p)                  { Put(w, p.x, p.y); }

	void Create(SystemDraw& w, int cx, int cy)         {}
	void Create(SystemDraw& w, Size sz)                { Create(w, sz.cx, sz.cy); }
	void Destroy()                                     {}

	void SetPaintingDraw(Draw& w, Point off)           { painting = &w; painting_offset = off; }

	Point GetOffset() const                            { return Point(0, 0); }

	BackDraw();
	~BackDraw();
};

Image GetIconForFile(const char *path);

#ifndef PLATFORM_WIN32
#include <CtrlCore/stdids.h>
#endif

#include "CocoCode.h"

#define GUIPLATFORM_KEYCODES_INCLUDE <CtrlCore/CocoKeys.h>

struct CocoTop;
struct MMCtrl;
struct MMImp;

#define GUIPLATFORM_CTRL_TOP_DECLS \
	CocoTop *coco = NULL; \

#define GUIPLATFORM_CTRL_DECLS_INCLUDE <CtrlCore/CocoCtrl.h>

#define GUIPLATFORM_PASTECLIP_DECLS \
void *nspasteboard; \
friend struct MMImp;

#define GUIPLATFORM_TOPWINDOW_DECLS_INCLUDE <CtrlCore/CocoTop.h>

};

#define GUIPLATFORM_INCLUDE_AFTER <CtrlCore/CocoAfter.h>

#define HAS_TopFrameDraw
