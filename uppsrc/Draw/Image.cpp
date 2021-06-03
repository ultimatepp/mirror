#include "Draw.h"

namespace Upp {

#define LTIMING(x) // RTIMING(x)

int ImageBuffer::ScanKind() const
{
	const RGBA *s = pixels;
	const RGBA *ee = pixels + (GetLength() & ~3);
	while(s < ee) {
		if((s[0].a & s[1].a & s[2].a & s[3].a) != 255)
			return IMAGE_ALPHA;
		s += 4;
	}
	const RGBA *e = pixels + GetLength();
	while(s < e) {
		if(s->a != 255)
			return IMAGE_ALPHA;
		s++;
	}
	return IMAGE_OPAQUE;
}

void ImageBuffer::SetHotSpots(const Image& src)
{
	SetHotSpot(src.GetHotSpot());
	Set2ndSpot(src.Get2ndSpot());
}

void ImageBuffer::Create(int cx, int cy)
{
	ASSERT(cx >= 0 && cy >= 0);
	size.cx = cx;
	size.cy = cy;
	pixels.Alloc(GetLength());
#ifdef _DEBUG
	RGBA *s = pixels;
	RGBA *e = pixels + GetLength();
	byte  a = 0;
	while(s < e) {
		s->a = a;
		a = ~a;
		s->r = a ? 255 : 0;
		s->g = s->b = 0;
		s++;
	}
#endif
	kind = IMAGE_UNKNOWN;
	InitAttrs();
}

void ImageBuffer::InitAttrs()
{
	spot2 = hotspot = Point(0, 0);
	dots = Size(0, 0);
	resolution = IMAGE_RESOLUTION_NONE;
	paintonce = false;
}

void ImageBuffer::CopyAttrs(const ImageBuffer& img)
{
	SetHotSpot(img.GetHotSpot());
	Set2ndSpot(img.Get2ndSpot());
	SetDots(img.GetDots());
	SetResolution(img.GetResolution());
	PaintOnceHint(img.IsPaintOnceHint());
}

void ImageBuffer::CopyAttrs(const Image& img)
{
	if(img.data)
		CopyAttrs(img.data->buffer);
	else
		InitAttrs();
}

void ImageBuffer::DeepCopy(const ImageBuffer& img)
{
	Create(img.GetSize());
	CopyAttrs(img);
	memcpy_t(~pixels, ~img.pixels, GetLength());
}

void ImageBuffer::Set(Image& img)
{
	if(img.data)
		if(img.data->refcount == 1) {
			size = img.GetSize();
			kind = IMAGE_UNKNOWN;
			CopyAttrs(img);
			pixels = pick(img.data->buffer.pixels);
			img.Clear();
		}
		else {
			DeepCopy(img.data->buffer);
			kind = IMAGE_UNKNOWN;
			img.Clear();
		}
	else
		Create(0, 0);
}


void ImageBuffer::operator=(Image& img)
{
	Clear();
	Set(img);
}

void ImageBuffer::operator=(ImageBuffer& img)
{
	Clear();
	Image m = img;
	Set(m);
}

ImageBuffer::ImageBuffer(Image& img)
{
	Set(img);
}

ImageBuffer::ImageBuffer(ImageBuffer& b)
{
	kind = b.kind;
	size = b.size;
	pixels = pick(b.pixels);
	CopyAttrs(b);
}

void ImageBuffer::SetDPI(Size dpi) 
{
	dots.cx = int(600.*size.cx/dpi.cx);
	dots.cy = int(600.*size.cy/dpi.cy);
}

Size ImageBuffer::GetDPI() 
{
	return Size(dots.cx ? int(600.*size.cx/dots.cx) : 0, dots.cy ? int(600.*size.cy/dots.cy) : 0);
}

void Image::Set(ImageBuffer& b)
{
	if(b.GetWidth() == 0 || b.GetHeight() == 0)
		data = NULL;
	else
		data = new Data(b);
}

void Image::Clear()
{
	if(data)
		data->Release();
	data = NULL;
}

Image& Image::operator=(ImageBuffer& img)
{
	if(data)
		data->Release();
	Set(img);
	return *this;
}

Image& Image::operator=(const Image& img)
{
	Data *d = data;
	data = img.data;
	if(data)
		data->Retain();
	if(d)
		d->Release();
	return *this;
}

Point Image::GetHotSpot() const
{
	return data ? data->buffer.GetHotSpot() : Point(0, 0);
}

Point Image::Get2ndSpot() const
{
	return data ? data->buffer.Get2ndSpot() : Point(0, 0);
}

Size Image::GetDots() const
{
	return data ? data->buffer.GetDots() : Size(0, 0);
}

Size Image::GetDPI() const
{
	Size size = GetSize();
	Size dots = GetDots();
	return data ?  Size(int(600.*size.cx/dots.cx), int(600.*size.cy/dots.cy)): Size(0, 0);
}

int Image::GetResolution() const
{
	return data ? data->buffer.GetResolution() : IMAGE_RESOLUTION_NONE;
}

int Image::GetKindNoScan() const
{
	return data ? data->buffer.GetKind() : IMAGE_ALPHA;
}

int Image::Data::GetKind()
{
	int k = buffer.GetKind();
	if(k != IMAGE_UNKNOWN)
		return k;
	k = buffer.ScanKind();
	buffer.SetKind(k);
	return k;
}

int Image::GetKind() const
{
	return data ? data->GetKind() : IMAGE_EMPTY;
}

void Image::Serialize(Stream& s)
{
	Point spot2 = Get2ndSpot();
	int version = spot2.x || spot2.y; // version 1 only if we need 2nd spot, to improve BW compatibility
	s / version;
	Size sz = GetSize();
	Point p = GetHotSpot();
	Size dots = GetDots();
	s % sz % p % dots;
	Point p2(0, 0);
	if(version >= 1) {
		p2 = spot2;
		s % p2;
	}
	int64 len = (int64)sz.cx * (int64)sz.cy * (int64)sizeof(RGBA);
	if(s.IsLoading()) {
		if(len) {
			ImageBuffer b;
			if(len < 6 * 1024 * 1024) {
				b.Create(sz);
				if(!s.GetAll(~b, (int)len)) {
					Clear();
					s.LoadError();
					return;
				}
			}
			else {
				Huge h;
				if(!s.GetAll(h, (size_t)len)) {
					Clear();
					s.LoadError();
					return;
				}
				b.Create(sz);
				h.Get(~b);
			}

			b.SetDots(dots);
			b.SetHotSpot(p);
			b.Set2ndSpot(p2);
			*this = b;
		}
		else
			Clear();
	}
	else
		s.Put64(~*this, len);
}
INITBLOCK {
	Value::Register<Image>("Image");
}

bool Image::operator==(const Image& img) const
{
	static_assert(sizeof(RGBA) == 4, "sizeof(RGBA)");
	return IsSame(img) ||
	   GetSize() == img.GetSize() &&
	   GetHotSpot() == img.GetHotSpot() &&
	   Get2ndSpot() == img.Get2ndSpot() &&
	   GetDots() == img.GetDots() &&
	   GetResolution() == img.GetResolution() &&
	   memeq32(~*this, ~img, GetLength());
}

bool Image::operator!=(const Image& img) const
{
	return !operator==(img);
}

hash_t Image::GetHashValue() const
{
	return memhash(~*this, GetLength() * sizeof(RGBA));
}

Image::Image(const Image& img)
{
	data = img.data;
	if(data)
		data->Retain();
}

Image::Image(Image (*fn)())
{
	data = NULL;
	*this = (*fn)();
}

Image::Image(const Value& src)
{
	data = NULL;
	*this = src.Get<Image>();
}

Image::Image(ImageBuffer& b)
{
	Set(b);
}

Image::~Image()
{
	if(data)
		data->Release();
}

String Image::ToString() const
{
	return String("Image ").Cat() << GetSize();
}

Image::Data::Data(ImageBuffer& b)
:	buffer(b)
{
	paintcount = 0;
	paintonly = false;
	refcount = 1;
	aux_data = 0;
	INTERLOCKED {
		static int64 gserial;
		serial = ++gserial;
	}
}

void Image::SetAuxData(uint64 adata)
{
	if(data)
		data->aux_data = adata;
}

uint64 Image::GetAuxData() const
{
	return data ? data->aux_data : 0;
}

static void sMultiply(ImageBuffer& b, int (*op)(RGBA *t, const RGBA *s, int len))
{
	if(b.GetKind() != IMAGE_OPAQUE && b.GetKind() != IMAGE_EMPTY)
		(*op)(~b, ~b, b.GetLength());
}

void Premultiply(ImageBuffer& b)
{
	sMultiply(b, Premultiply);
}

void Unmultiply(ImageBuffer& b)
{
	sMultiply(b, Unmultiply);
}

static Image sMultiply(const Image& img, int (*op)(RGBA *t, const RGBA *s, int len))
{
	int k = img.GetKind();
	if(k == IMAGE_OPAQUE || k == IMAGE_EMPTY)
		return img;
	ImageBuffer ib(img.GetSize());
	ib.CopyAttrs(img);
	ib.SetKind((*op)(~ib, ~img, ib.GetLength()));
	return ib;
}

Image Premultiply(const Image& img)
{
	return sMultiply(img, Premultiply);
}

Image Unmultiply(const Image& img)
{
	return sMultiply(img, Unmultiply);
}

String StoreImageAsString(const Image& img)
{
	if(img.GetKind() == IMAGE_EMPTY)
		return Null;
	int type = img.GetKind() == IMAGE_OPAQUE ? 3 : 4;
	type |= decode(img.GetResolution(), IMAGE_RESOLUTION_STANDARD, 0x40, IMAGE_RESOLUTION_UHD, 0x80, 0);
	StringStream ss;
	ss.Put(type);
	Size sz = img.GetSize();
	ss.Put16le(sz.cx);
	ss.Put16le(sz.cy);
	Point p = img.GetHotSpot();
	ss.Put16le(p.x);
	ss.Put16le(p.y);
	Size dots = img.GetDots();
	ss.Put16le(dots.cx);
	ss.Put16le(dots.cy);
	const RGBA *s = img;
	const RGBA *e = s + img.GetLength();
	Buffer<byte> b(type * img.GetLength());
	byte *t = b;
	if(type == 3)
		while(s < e) {
			*t++ = s->r;
			*t++ = s->g;
			*t++ = s->b;
			s++;
		}
	else
		while(s < e) {
			*t++ = s->r;
			*t++ = s->g;
			*t++ = s->b;
			*t++ = s->a;
			s++;
		}
	MemReadStream m(b, type * img.GetLength());
	ZCompress(ss, m);
	return ss;
}

Image  LoadImageFromString(const String& src)
{
	if(src.GetLength() < 13)
		return Null;
	StringStream ss(src);
	int type = ss.Get();
	int resolution = decode(type & 0xc0, 0x40, IMAGE_RESOLUTION_STANDARD, 0x80, IMAGE_RESOLUTION_UHD, 0);
	type &= 0x3f;
	Size sz;
	sz.cx = ss.Get16le();
	sz.cy = ss.Get16le();
	if(sz.cx < 0 || sz.cy < 0)
		return Null;
	Point p;
	p.x = ss.Get16le();
	p.y = ss.Get16le();
	if(p.x < 0 || p.y < 0)
		return Null;
	Size dots;
	dots.cx = ss.Get16le();
	dots.cy = ss.Get16le();
	if(dots.cx < 0 || dots.cy < 0)
		return Null;
	StringStream out;
	ZDecompress(out, ss);
	String data = out;
	if(data.GetLength() != type * sz.cx * sz.cy)
		return Image();
	ImageBuffer ib(sz);
	ib.SetHotSpot(p);
	ib.SetDots(dots);
	ib.SetResolution(resolution);
	RGBA *t = ib;
	const RGBA *e = t + ib.GetLength();
	const byte *s = data;
	if(type == 3)
		while(t < e) {
			t->r = *s++;
			t->g = *s++;
			t->b = *s++;
			t->a = 255;
			t++;
		}
	else
	if(type == 4)
		while(t < e) {
			t->r = *s++;
			t->g = *s++;
			t->b = *s++;
			t->a = *s++;
			t++;
		}
	else
		return Image();
	return ib;
}

Size GetImageStringSize(const String& src)
{
	if(src.GetLength() < 13)
		return Size(0, 0);
	StringStream ss(src);
	ss.Get();
	Size sz;
	sz.cx = ss.Get16le();
	sz.cy = ss.Get16le();
	return sz;
}

Size GetImageStringDots(const String& src)
{
	if(src.GetLength() < 13)
		return Size(0, 0);
	StringStream ss(src);
	ss.SeekCur(9);
	Size sz;
	sz.cx = ss.Get16le();
	sz.cy = ss.Get16le();
	return sz;
}

}
