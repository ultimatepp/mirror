#ifndef _CtrlCore_CocoMM_h_
#define _CtrlCore_CocoMM_h_

#include <Core/config.h>

#if defined(PLATFORM_COCOA) && !defined(VIRTUALGUI)

#define Point NS_Point
#define Rect  NS_Rect
#define Size  NS_Size
#include <AppKit/AppKit.h>
#undef  Point
#undef  Rect
#undef  Size

#endif

#include "CtrlCore.h"

#ifdef GUI_COCOA

namespace Upp {

template <class T>
struct CFRef {
	T ptr;
	T operator~()   { return ptr; }
	operator T()    { return ptr; }
	T  operator->() { return ptr; }
	T  Detach()     { T h = ptr; ptr = NULL; return h; }
	CFRef(T p)      { ptr = p; }
	~CFRef()        { if(ptr) CFRelease(ptr); }
};

struct AutoreleasePool {
	NSAutoreleasePool *pool;

	AutoreleasePool() {
		pool = [[NSAutoreleasePool alloc] init];
	}
	~AutoreleasePool() {
	    [pool release];
	}
};

CGImageRef createCGImage(const Image& img);

NSImage *GetNSImage(const Image& img);

// From Draw/FontCoco.cpp
WString ToWString(CFStringRef s);
String  ToString(CFStringRef s);

inline WString ToWString(NSString *s) { return ToWString((CFStringRef)s); }
inline String  ToString(NSString *s)  { return ToString((CFStringRef)s); }

inline CGContextRef GetCG(SystemDraw& w) { return (CGContextRef)w.GetCGHandle(); }

#define      cgHandle   (CGContextRef)handle

struct PointCG {
	int x, y;

	operator CGPoint() const { return CGPointMake(x, y); }
	
	PointCG(int x, int y) : x(x), y(y) {}
	PointCG();
};

struct RectCG {
	int x, y, cx, cy;

	operator CGRect() const { return CGRectMake(x, y, cx, cy); }
	
	RectCG(int x, int y, int cx, int cy) : x(x), y(y), cx(cx), cy(cy) {}
	RectCG();
};

}

@interface CocoView : NSView <NSWindowDelegate>
{
	@public
	Upp::Ptr<Upp::Ctrl> ctrl;
}
@end

@interface CocoWindow : NSWindow
{
	@public
	Upp::Ptr<Upp::Ctrl> ctrl;
	bool active;
}
@end

struct Upp::MMCtrl {
	static void SyncRect(CocoView *view);
};

struct Upp::CocoTop {
	CocoWindow *window = NULL;
	CocoView *view = NULL;
	Ptr<Ctrl> owner;
};

void SyncRect(CocoView *view);

inline Upp::Rect MakeRect(const CGRect& r, int dpi) {
	return Upp::RectC(dpi * r.origin.x, dpi * r.origin.y, dpi * r.size.width, dpi * r.size.height);
}

inline CGRect CGRectDPI(const Upp::Rect& r) {
	if(Upp::IsUHDMode())
		return CGRectMake(0.5 * r.left, 0.5 * r.top, 0.5 * r.GetWidth(), 0.5 * r.GetHeight());
	else
		return CGRectMake(r.left, r.top, r.GetWidth(), r.GetHeight());
}

#endif

#endif
