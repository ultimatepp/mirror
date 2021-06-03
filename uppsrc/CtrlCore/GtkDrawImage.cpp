#include <CtrlCore/CtrlCore.h>

#ifdef GUI_GTK

//#include <shellapi.h>

namespace Upp {

#define LTIMING(x) // RTIMING(x)
#define LLOG(x)

void SetSurface(SystemDraw& w, const Rect& dest, const RGBA *pixels, Size srcsz, Point poff)
{
	w.FlushText();
	Size dsz = dest.GetSize();
	cairo_surface_t *surface = cairo_image_surface_create_for_data((byte *)pixels, CAIRO_FORMAT_ARGB32, dsz.cx, dsz.cy, 4 * dsz.cx);
	cairo_set_source_surface(w, surface, dest.left, dest.top);
	cairo_paint(w);
	cairo_surface_destroy(surface);
}

struct ImageSysData {
	Image            img;
	cairo_surface_t *surface = NULL;
	
	void Init(const Image& m, cairo_surface_t *other);
	~ImageSysData();
};

cairo_surface_t *CreateCairoSurface(const Image& img, cairo_surface_t *other)
{
	Size isz = img.GetSize();
	cairo_format_t fmt = CAIRO_FORMAT_ARGB32;
	cairo_surface_t *surface = other ? cairo_surface_create_similar_image(other, fmt, isz.cx, isz.cy)
	                                 : cairo_image_surface_create(fmt, isz.cx, isz.cy);
	cairo_surface_flush(surface);
	byte *a = (byte *)cairo_image_surface_get_data(surface);
	int stride = cairo_format_stride_for_width(fmt, isz.cx);
	for(int yy = 0; yy < isz.cy; yy++) {
		Copy((RGBA *)a, img[yy], isz.cx);
		a += stride;
	}
	cairo_surface_mark_dirty(surface);
	return surface;
}

cairo_surface_t *CreateCairoSurface(const Image& img)
{
	return CreateCairoSurface(img, NULL);
}

void ImageSysData::Init(const Image& m, cairo_surface_t *other)
{
	img = m;
	surface = CreateCairoSurface(m, other);
	SysImageRealized(img);
}

ImageSysData::~ImageSysData()
{
	SysImageReleased(img);
	cairo_surface_destroy(surface);
}

struct ImageSysDataMaker : LRUCache<ImageSysData, int64>::Maker {
	Image img;
	cairo_surface_t *other;

	virtual int64  Key() const                      { return img.GetSerialId(); }
	virtual int    Make(ImageSysData& object) const { object.Init(img, other); return img.GetLength(); }
};

void SystemDraw::SysDrawImageOp(int x, int y, const Image& img, Color color)
{
	GuiLock __;
	FlushText();
	if(img.GetLength() == 0)
		return;
	if(img.IsPaintOnceHint()) {
		SetSurface(*this, x, y, img.GetWidth(), img.GetHeight(), ~img);
		return;
	}
	LLOG("SysDrawImageOp " << img.GetSerialId() << ' ' << x << ", " << y << ", "<< img.GetSize());
	ImageSysDataMaker m;
	static LRUCache<ImageSysData, int64> cache;
	static int Rsz;
	Rsz += img.GetLength();
	if(Rsz > 200 * 200) { // we do not want to do this for each small image painted...
		Rsz = 0;
		cache.Remove([](const ImageSysData& object) {
			return object.img.GetRefCount() == 1;
		});
	}
	LLOG("SysImage cache pixels " << cache.GetSize() << ", count " << cache.GetCount());
	m.img = img;
	m.other = cairo_get_target(cr);
	ImageSysData& sd = cache.Get(m);
	if(!IsNull(color)) {
		SetColor(color);
		cairo_mask_surface(cr, sd.surface, x, y);
	}
	else {
		cairo_set_source_surface(cr, sd.surface, x, y);
		cairo_paint(cr);
	}
	static Size ssz;
	if(ssz.cx == 0)
		ssz = Ctrl::GetVirtualScreenArea().GetSize();
	cache.Shrink(4 * ssz.cx * ssz.cy, 1000); // Cache must be after Paint because of PaintOnly!
}

Draw& ImageDraw::Alpha()
{
	if(!alpha_surface) {
		alpha_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, isz.cx, isz.cy);
		alpha.cr = cairo_create(alpha_surface);
	}
	return alpha;
}

void CairoGet(ImageBuffer& b, Size isz, cairo_surface_t *surface, cairo_surface_t *alpha_surface)
{
	cairo_surface_flush(surface);
	byte *a = (byte *)cairo_image_surface_get_data(surface);
	int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, isz.cx);
	RGBA *t = b;
	byte *aa = NULL;
	if(alpha_surface) {
		cairo_surface_flush(alpha_surface);
		aa = (byte *)cairo_image_surface_get_data(alpha_surface);
	}
	for(int yy = 0; yy < isz.cy; yy++) {
		RGBA *s = (RGBA *)a;
		RGBA *e = s + isz.cx;
		if(aa) {
			RGBA *ss = (RGBA *)aa;
			while(s < e) {
				*t = *s++;
				(t++)->a = (ss++)->r;
			}
			aa += stride;
			b.SetKind(IMAGE_ALPHA);
		}
		else {
			while(s < e) {
				*t = *s++;
				(t++)->a = 255;
			}
			b.SetKind(IMAGE_OPAQUE);
		}
		a += stride;
	}
}

void ImageDraw::FetchStraight(ImageBuffer& b) const
{
	ImageDraw *m = const_cast<ImageDraw *>(this);
	m->FlushText();
	if(alpha_surface)
		m->alpha.FlushText();
	CairoGet(b, isz, surface, alpha_surface);
}

ImageDraw::operator Image() const
{
	ImageBuffer img(isz);
	FetchStraight(img);
	Premultiply(img);
	return img;
}

Image ImageDraw::GetStraight() const
{
	ImageBuffer img(isz);
	FetchStraight(img);
	return img;
}

void ImageDraw::Init(Size sz)
{
	isz = sz;
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, isz.cx, isz.cy);
	cr = cairo_create(surface);
	alpha_surface = NULL;
	del = true;
}

ImageDraw::ImageDraw(Size sz)
{
	Init(sz);
}

ImageDraw::ImageDraw(int cx, int cy)
{
	Init(Size(cx, cy));
}

ImageDraw::ImageDraw(cairo_t *cr_, Size sz)
{
	isz = sz;
	cr = cr_;
	surface = cairo_get_target(cr);
	alpha_surface = NULL;
	del = false;
}

ImageDraw::~ImageDraw()
{
	if(del) {
		cairo_destroy(cr);
		cairo_surface_destroy(surface);
		if(alpha_surface) {
			cairo_destroy(alpha.cr);
			cairo_surface_destroy(alpha_surface);
		}
	}
}

void BackDraw::Put(SystemDraw& w, int x, int y)
{
}

void BackDraw::Create(SystemDraw& w, int cx, int cy)
{
}

void BackDraw::Destroy()
{
}

#define FCURSOR_(x) { static Image h; ONCELOCK { h = CreateImage(Size(1, 1), Black); h.SetAuxData(x + 1); } return h; }

Image Image::Arrow() FCURSOR_(GDK_LEFT_PTR)
Image Image::Wait() FCURSOR_(GDK_WATCH)
Image Image::IBeam() FCURSOR_(GDK_XTERM)
Image Image::No() FCURSOR_(GDK_CIRCLE)
Image Image::SizeAll() FCURSOR_(GDK_FLEUR)
Image Image::SizeHorz() FCURSOR_(GDK_SB_H_DOUBLE_ARROW)
Image Image::SizeVert() FCURSOR_(GDK_SB_V_DOUBLE_ARROW)
Image Image::SizeTopLeft() FCURSOR_(GDK_TOP_LEFT_CORNER)
Image Image::SizeTop() FCURSOR_(GDK_TOP_SIDE)
Image Image::SizeTopRight() FCURSOR_(GDK_TOP_RIGHT_CORNER)
Image Image::SizeLeft() FCURSOR_(GDK_LEFT_SIDE)
Image Image::SizeRight() FCURSOR_(GDK_RIGHT_SIDE)
Image Image::SizeBottomLeft() FCURSOR_(GDK_BOTTOM_LEFT_CORNER)
Image Image::SizeBottom() FCURSOR_(GDK_BOTTOM_SIDE)
Image Image::SizeBottomRight()  FCURSOR_(GDK_BOTTOM_RIGHT_CORNER)
Image Image::Cross() FCURSOR_(GDK_CROSSHAIR)
Image Image::Hand() FCURSOR_(GDK_HAND1)

}

#endif
