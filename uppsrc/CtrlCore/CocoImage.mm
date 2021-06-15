#include "CocoMM.h"

#ifdef GUI_COCOA

#define LLOG(x)

namespace Upp {

CGImageRef createCGImage(const Image& img)
{
	if(IsNull(img))
		return NULL;
	Image *km = new Image(img); // to keep data alive
	CGDataProvider *dataProvider = CGDataProviderCreateWithData(km, ~img, img.GetLength() * sizeof(RGBA),
	                               [](void *info, const void *data, size_t size) { delete (Image *)info; });
	static CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB(); // TODO: This is probably wrong...
	Upp::Size isz = img.GetSize();
    CGImageRef cg_img = CGImageCreate(isz.cx, isz.cy, 8, 32, isz.cx * sizeof(RGBA),
                                      colorSpace, kCGImageAlphaPremultipliedFirst,
                                      dataProvider, 0, false, kCGRenderingIntentDefault);
	CGDataProviderRelease(dataProvider);
	return cg_img;
}

struct ImageSysData {
	Image            img;
	CGImageRef       cgimg = NULL;
	
	void Init(const Image& img);
	~ImageSysData();
};

// Note: Cocoa U++ has to ignore paintonly flag as the data of image are directly referenced by
// data provider

void ImageSysData::Init(const Image& m)
{
	img = m;
	cgimg = createCGImage(img);
	// Do not call SysImageRealized(img) here
}

ImageSysData::~ImageSysData()
{
	CGImageRelease(cgimg);
}

struct ImageSysDataMaker : LRUCache<ImageSysData, int64>::Maker {
	Image img;

	virtual int64  Key() const                      { return img.GetSerialId(); }
	virtual int    Make(ImageSysData& object) const { object.Init(img); return img.GetLength(); }
};

static LRUCache<ImageSysData, int64> cg_image_cache;

static void sCleanImageCache(const Image& img)
{
	static int Rsz;
	Rsz += img.GetLength();
	if(Rsz > 200 * 200) { // we do not want to do this for each small image painted...
		Rsz = 0;
		cg_image_cache.Remove([](const ImageSysData& object) {
			return object.img.GetRefCount() == 1; // the object.img is nowhere else than in cache
		});
	}
}

void SystemDraw::SysDrawImageOp(int x, int y, const Image& img, Color color)
{
	GuiLock __;

	if(img.GetLength() == 0)
		return;
#if 0
	Image m = IsNull(color) ? img : CachedSetColorKeepAlpha(img, color);
	Image *km = new Image(m); // to keep data alive
	CGDataProvider *dataProvider = CGDataProviderCreateWithData(km, ~m, m.GetLength() * sizeof(RGBA),
	                               [](void *info, const void *data, size_t size) { delete (Image *)info; });
	static CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB(); // TODO: This is probably wrong...
	Upp::Size isz = img.GetSize();
    CGImageRef cgimg = CGImageCreate(isz.cx, isz.cy, 8, 32, isz.cx * sizeof(RGBA),
                                      colorSpace, kCGImageAlphaPremultipliedFirst,
                                      dataProvider, 0, false, kCGRenderingIntentDefault);


	if(cgimg) {
		DLOG("Have cgimg");
		Size isz = img.GetSize();
		DDUMP(isz);
		CGContextSaveGState(cgHandle);
		Point off = GetOffset();
		CGContextTranslateCTM(cgHandle, x + off.x, y + off.y);
		CGContextScaleCTM(cgHandle, 1.0, -1.0);
		CGContextDrawImage(cgHandle, CGRectMake(0, -isz.cy, isz.cx, isz.cy), cgimg);
	    CGContextRestoreGState(cgHandle);
		CGImageRelease(cgimg);
	}

	CGDataProviderRelease(dataProvider);

#else
	sCleanImageCache(img);

	ImageSysDataMaker m;
	LLOG("SysImage cache pixels " << cache.GetSize() << ", count " << cache.GetCount());
	m.img = IsNull(color) ? img : CachedSetColorKeepAlpha(img, color); // TODO: Can setcolor be optimized out? By masks e.g.?
	ImageSysData& sd = cg_image_cache.Get(m);
	if(sd.cgimg) {
		Size isz = img.GetSize();
		CGContextSaveGState(cgHandle);
		Point off = GetOffset();
		CGContextTranslateCTM(cgHandle, x + off.x, y + off.y);
		CGContextScaleCTM(cgHandle, 1.0, -1.0);
		CGContextDrawImage(cgHandle, CGRectMake(0, -isz.cy, isz.cx, isz.cy), sd.cgimg);
	    CGContextRestoreGState(cgHandle);
	}
	Size sz = Ctrl::GetPrimaryScreenArea().GetSize();
	cg_image_cache.Shrink(4 * sz.cx * sz.cy, 1000); // Cache must be after Paint because of PaintOnly!
#endif
}

// TODO: https://stackoverflow.com/questions/10733228/native-osx-lion-resize-cursor-for-custom-nswindow-or-nsview

enum {
	MM_Arrow = 1,
	MM_Wait,
	MM_IBeam,
	MM_No,
	MM_SizeAll,
	MM_SizeHorz,
	MM_SizeVert,
	MM_SizeTopLeft,
	MM_SizeTop,
	MM_SizeTopRight,
	MM_SizeLeft,
	MM_SizeRight,
	MM_SizeBottomLeft,
	MM_SizeBottom,
	MM_SizeBottomRight,
	MM_Cross,
	MM_Hand,
};


#define FCURSOR_(x) { static Image h; ONCELOCK { h = CreateImage(Size(1, 1), Black); h.SetAuxData(x); } return h; }

Image Image::Arrow() FCURSOR_(MM_Arrow)
Image Image::Wait() FCURSOR_(MM_Wait)
Image Image::IBeam() FCURSOR_(MM_IBeam)
Image Image::No() FCURSOR_(MM_No)
Image Image::SizeAll() FCURSOR_(MM_SizeAll)
Image Image::SizeHorz() FCURSOR_(MM_SizeHorz)
Image Image::SizeVert() FCURSOR_(MM_SizeVert)
Image Image::SizeTopLeft() FCURSOR_(MM_SizeTopLeft)
Image Image::SizeTop() FCURSOR_(MM_SizeTop)
Image Image::SizeTopRight() FCURSOR_(MM_SizeTopRight)
Image Image::SizeLeft() FCURSOR_(MM_SizeLeft)
Image Image::SizeRight() FCURSOR_(MM_SizeRight)
Image Image::SizeBottomLeft() FCURSOR_(MM_SizeBottomLeft)
Image Image::SizeBottom() FCURSOR_(MM_SizeBottom)
Image Image::SizeBottomRight() FCURSOR_(MM_SizeBottomRight)
Image Image::Cross() FCURSOR_(MM_Cross)
Image Image::Hand() FCURSOR_(MM_Hand)

// TODO: Missing kinds (sizers mostly)
// https://stackoverflow.com/questions/10733228/native-osx-lion-resize-cursor-for-custom-nswindow-or-nsview/21786835#21786835

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundeclared-selector"

#define XCURSOR(id) \
	if([NSCursor respondsToSelector:@selector(id)]) return [NSCursor performSelector:@selector(id)];

NSCursor *GetNSCursor(int kind)
{
	switch(kind) {
	case MM_SizeAll:
		XCURSOR(_moveCursor);
		break;
	case MM_SizeHorz:
		XCURSOR(_windowResizeEastWestCursor);
		break;
	case MM_SizeVert:
		XCURSOR(_windowResizeNorthSouthCursor);
		break;
	case MM_SizeTopLeft:
		XCURSOR(_windowResizeNorthWestSouthEastCursor);
		break;
	case MM_SizeTop:
		XCURSOR(_windowResizeNorthCursor);
		break;
	case MM_SizeTopRight:
		XCURSOR(_windowResizeNorthEastSouthWestCursor);
		break;
	case MM_SizeLeft:
		XCURSOR(_windowResizeWestCursor);
		break;
	case MM_SizeRight:
		XCURSOR(_windowResizeEastCursor);
		break;
	case MM_SizeBottomLeft:
		XCURSOR(_windowResizeNorthEastSouthWestCursor);
		break;
	case MM_SizeBottom:
		XCURSOR(_windowResizeSouthCursor);
		break;
	case MM_SizeBottomRight:
		XCURSOR(_windowResizeNorthWestSouthEastCursor);
		break;
	case MM_IBeam: return [NSCursor IBeamCursor];
	case MM_Cross: return [NSCursor crosshairCursor];
	case MM_Hand:  return [NSCursor pointingHandCursor];
	case MM_No:    return [NSCursor operationNotAllowedCursor];
	}
	return [NSCursor arrowCursor];
}

#pragma clang diagnostic pop

NSImage *CreateNSImage(const Image& img, CGImageRef cgimg)
{
	sCleanImageCache(img);
	Size isz = img.GetSize();
	double scale = 1.0 / DPI(1);
	NSSize size;
	size.width = scale * isz.cx;
	size.height = scale * isz.cy;
	NSImage *nsimage = [[NSImage alloc] initWithCGImage:cgimg size:size];
	cg_image_cache.Shrink(4 * 1024 * 768, 1000); // Cache must be after Paint because of PaintOnly!
	return nsimage;
}

struct NSImageSysData {
	Image            img;
	NSImage         *nsimage = NULL;
	
	void Init(const Image& img);
	~NSImageSysData();
};

void NSImageSysData::Init(const Image& img)
{
	ImageSysDataMaker m;
	m.img = img;
	nsimage = CreateNSImage(img, cg_image_cache.Get(m).cgimg);
}

NSImageSysData::~NSImageSysData()
{
	if(nsimage)
		[nsimage release];
}

struct NSImageSysDataMaker : LRUCache<NSImageSysData, int64>::Maker {
	Image img;

	virtual int64  Key() const                        { return img.GetSerialId(); }
	virtual int    Make(NSImageSysData& object) const { object.Init(img); return img.GetLength(); }
};

static LRUCache<NSImageSysData, int64> nsimage_cache;

NSImage *GetNSImage(const Image& img)
{
	NSImageSysDataMaker m;
	m.img = img;
	NSImage *nsimage = nsimage_cache.Get(m).nsimage;
	nsimage_cache.Shrink(4 * 1024*768, 1000);
	return nsimage;
}

void Ctrl::SetNSAppImage(const Image& img)
{
	ImageSysDataMaker m;
	LLOG("SysImage cache pixels " << cache.GetSize() << ", count " << cache.GetCount());
	m.img = img;
	ImageSysData& sd = cg_image_cache.Get(m);
	static CGImageRef cgimg;
	static NSImage   *nsimg;
	if(sd.cgimg != cgimg) {
		cgimg = sd.cgimg;
		if(nsimg)
			[nsimg release];
		nsimg = CreateNSImage(img, cgimg);
	}
	[NSApp setApplicationIconImage:nsimg];
}

void  Ctrl::SetMouseCursor(const Image& img)
{
	if(GetDragAndDropSource())
			return;
	int64 h = img.GetAuxData();
	if(h) {
		[GetNSCursor(h) set];
		return;
	}
	ImageSysDataMaker m;
	LLOG("SysImage cache pixels " << cache.GetSize() << ", count " << cache.GetCount());
	m.img = img;
	ImageSysData& sd = cg_image_cache.Get(m);
	static CGImageRef cgimg;
	static NSCursor  *cursor;
	if(sd.cgimg != cgimg) {
		cgimg = sd.cgimg;
		if(cursor)
			[cursor release];
		NSImage *nsimg = CreateNSImage(img, cgimg);
		double scale = 1.0 / DPI(1);
		Point p = img.GetHotSpot();
		NSPoint hot;
		hot.x = scale * p.x;
		hot.y = scale * p.y;
		cursor = [[NSCursor alloc] initWithImage:nsimg hotSpot:hot];
		[nsimg release];
	}
	[cursor set];
}

void ImageDraw::Init(int cx, int cy)
{
	ib.Create(cx, cy);
	Fill(~ib, RGBAZero(), ib.GetLength());

	static CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

	SystemDraw::Init(CGBitmapContextCreateWithData(~ib, cx, cy, 8, cx * sizeof(RGBA),
	                                               colorSpace, kCGImageAlphaPremultipliedFirst,
	                                               NULL, NULL), NULL);
	CGContextTranslateCTM(cgHandle, 0, cy);
	if(IsUHDMode()) {
		CGContextScaleCTM(cgHandle, 2, -2);
		CGContextTranslateCTM(cgHandle, 0, -cy / 2.0);
	}
	else
		CGContextScaleCTM(cgHandle, 1, -1);
}

ImageDraw::ImageDraw(Size sz)
{
	Init(sz.cx, sz.cy);
}

ImageDraw::ImageDraw(int cx, int cy)
{
	Init(cx, cy);
}

Draw& ImageDraw::Alpha()
{
	if(!alpha)
		alpha.Create(ib.GetSize());
	return *alpha;
}

ImageDraw::~ImageDraw()
{
	CGContextRelease(cgHandle);
	handle = NULL; // avoid releasing invalid handle in ~SystemDraw
}

Image ImageDraw::GetStraight() const
{
	ImageBuffer ib2(ib.GetSize());
	memcpy_t(~ib2, ~ib, ib.GetLength());
	if(alpha) {
		RGBA *e = ib2.End();
		RGBA *t = ib2;
		const RGBA *s = alpha->ib;
		while(t < e) {
			t->a = s->r;
			s++;
			t++;
		}
	}
	return Image(ib2);
}

ImageDraw::operator Image() const
{
	if(alpha)
		return Premultiply(GetStraight());
	ImageBuffer ib2(ib.GetSize());
	memcpy_t(~ib2, ~ib, ib.GetLength());
	return Image(ib2);
}

Image GetIconForFile(const char *path)
{
	ImageDraw iw(DPI(16, 16));

	CGContextRef cg = (CGContextRef) iw.GetCGHandle();

	
	NSImage *image;
	CFRef<CFStringRef> fexe = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
	image = [[NSWorkspace sharedWorkspace]iconForFile:(__bridge NSString *)~fexe];
/*
	}
	else { // not used any more
		CFRef<CFStringRef> fext = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
		image = *ext == '*' ? [[NSWorkspace sharedWorkspace] iconForFileType:NSFileTypeForHFSTypeCode(kGenericFolderIcon)]
	                        : [[NSWorkspace sharedWorkspace]iconForFileType:(__bridge NSString *)~fext];
	}
*/
    NSGraphicsContext *gc = [NSGraphicsContext graphicsContextWithCGContext:cg flipped:YES];
    NSGraphicsContext* cgc = [NSGraphicsContext currentContext];
    [NSGraphicsContext setCurrentContext:gc];
    [image drawInRect:NSMakeRect(0, 0, DPI(16), DPI(16))];
	[NSGraphicsContext setCurrentContext:cgc];
	
	return iw;
}

};

#endif
