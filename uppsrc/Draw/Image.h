#define NEWIMAGE

enum ImageKind {
	IMAGE_UNKNOWN,
	IMAGE_EMPTY, // deprecated
	IMAGE_ALPHA,
	IMAGE_MASK, // deprecated
	IMAGE_OPAQUE,
};

inline void Fill(RGBA *t, RGBA c, int n) { memset32(t, *(dword *)&c, n); }
inline void Copy(RGBA *t, const RGBA *s, int n) { memcpy_t(t, s, n); }

int  Premultiply(RGBA *t, const RGBA *s, int len);
int  Unmultiply(RGBA *t, const RGBA *s, int len);

void TransformComponents(RGBA *t, const RGBA *s, int len,
	const byte r[], const byte g[], const byte b[], const byte a[]);
void MultiplyComponents(RGBA *t, const RGBA *s, int len, int num, int den = 256);

void AlphaBlend(RGBA *t, const RGBA *s, int len);
void AlphaBlend(RGBA *t, const RGBA *s, int len, Color color);

void AlphaBlendOpaque(RGBA *t, const RGBA *s, int len, Color color);
void AlphaBlendOpaque(RGBA *t, const RGBA *s, int len);

void AlphaBlendStraight(RGBA *b, const RGBA *f, int len);
void AlphaBlendStraight(RGBA *b, const RGBA *f, int len, Color color);
void AlphaBlendStraightOpaque(RGBA *t, const RGBA *s, int len);
void AlphaBlendStraightOpaque(RGBA *t, const RGBA *s, int len, int alpha);

int  GetChMaskPos32(dword mask);
void AlphaBlendOverBgST(RGBA *b, RGBA bg, int len);

const byte *UnpackRLE(RGBA *t, const byte *src, int len);
String      PackRLE(const RGBA *s, int len);

inline int  Grayscale(int r, int g, int b) { return (77 * r + 151 * g + 28 * b) >> 8; }
inline int  Grayscale(const RGBA& c)       { return Grayscale(c.r, c.g, c.b); }
inline byte Saturate255(int x)             { return byte(~(x >> 24) & (x | (-(x >> 8) >> 24)) & 0xff); }

class  Image;

enum ImageResolutionIntent {
	IMAGE_RESOLUTION_NONE = -1,
	IMAGE_RESOLUTION_STANDARD = 0,
	IMAGE_RESOLUTION_UHD = 1,
};

class ImageBuffer : NoCopy {
	mutable int  kind;
	Size         size;
	Buffer<RGBA> pixels;
	Point        hotspot;
	Point        spot2;
	Size         dots;
	int8         resolution;
	bool         paintonce = false;

	void         Set(Image& img);
	void         DeepCopy(const ImageBuffer& img);

	RGBA*        Line(int i) const      { ASSERT(i >= 0 && i < size.cy); return (RGBA *)~pixels + i * size.cx; }
	friend void  DropPixels___(ImageBuffer& b) { b.pixels.Clear(); }
	void         InitAttrs();

	friend class Image;

public:
	void  SetKind(int k)                { kind = k; }
	int   GetKind() const               { return kind; }
	int   ScanKind() const;
	int   GetScanKind() const           { return kind == IMAGE_UNKNOWN ? ScanKind() : kind; }

	void  SetHotSpot(Point p)           { hotspot = p; }
	Point GetHotSpot() const            { return hotspot; }

	void  Set2ndSpot(Point p)           { spot2 = p; }
	Point Get2ndSpot() const            { return spot2; }
	
	void  SetHotSpots(const Image& src);

	void  SetDots(Size sz)              { dots = sz; }
	Size  GetDots() const               { return dots; }
	void  SetDPI(Size sz);
	Size  GetDPI();
	
	void  SetResolution(int i)          { resolution = i; }
	int   GetResolution() const         { return resolution; }

	void  CopyAttrs(const ImageBuffer& img);
	void  CopyAttrs(const Image& img);
	
	Size  GetSize() const               { return size; }
	int   GetWidth() const              { return size.cx; }
	int   GetHeight() const             { return size.cy; }
	int   GetLength() const             { return size.cx * size.cy; }

	RGBA *operator[](int i)             { return Line(i); }
	const RGBA *operator[](int i) const { return Line(i); }
	RGBA *operator~()                   { return pixels; }
	operator RGBA*()                    { return pixels; }
	const RGBA *operator~() const       { return pixels; }
	operator const RGBA*() const        { return pixels; }
	RGBA *Begin()                       { return pixels; }
	const RGBA *Begin() const           { return pixels; }
	RGBA *End()                         { return pixels + GetLength(); }
	const RGBA *End() const             { return pixels + GetLength(); }
	RGBA *begin()                       { return pixels; }
	const RGBA *begin() const           { return pixels; }
	RGBA *end()                         { return pixels + GetLength(); }
	const RGBA *end() const             { return pixels + GetLength(); }

	void  Create(int cx, int cy);
	void  Create(Size sz)               { Create(sz.cx, sz.cy); }
	bool  IsEmpty() const               { return (size.cx | size.cy) == 0; }
	void  Clear()                       { Create(0, 0); }

	void  PaintOnceHint(bool b = true)  { paintonce = b; }
	bool  IsPaintOnceHint() const       { return paintonce; }

	void  operator=(Image& img);
	void  operator=(ImageBuffer& img);

	ImageBuffer()                       { Create(0, 0); }
	ImageBuffer(int cx, int cy)         { Create(cx, cy); }
	ImageBuffer(Size sz)                { Create(sz.cx, sz.cy); }
	ImageBuffer(Image& img);
	ImageBuffer(ImageBuffer& b);
// BW, defined in CtrlCore:
	ImageBuffer(ImageDraw& iw);
};

void Premultiply(ImageBuffer& b);
void Unmultiply(ImageBuffer& b);

class Image : public ValueType< Image, 150, Moveable_<Image> > {
private:
	struct Data {
		Atomic refcount;
		int64  serial;
		uint64 aux_data;
		int    paintcount;

		void   Retain()  { AtomicInc(refcount); }
		void   Release() { if(AtomicDec(refcount) == 0) delete this; }
		
		ImageBuffer buffer;
		bool        paintonly;
		
		int         GetKind();

		Data(ImageBuffer& b);
	};

	Data *data;

	void Set(ImageBuffer& b);

	friend class  ImageBuffer;
	friend struct Data;
	friend class  SystemDraw;
	friend void   SetPaintOnly__(Image& img)   { img.data->paintonly = img.data->refcount == 1; }
	friend void   SysImageRealized(const Image& img);
	friend struct scImageMaker;

	void         SetAuxData(uint64 data);

public:
	Size  GetSize() const                     { return data ? data->buffer.GetSize() : Size(0, 0); }
	int   GetWidth() const                    { return GetSize().cx; }
	int   GetHeight() const                   { return GetSize().cy; }
	int   GetLength() const                   { return data ? data->buffer.GetLength() : 0; }
	Point GetHotSpot() const;
	Point Get2ndSpot() const;
	Size  GetDots() const;
	Size  GetDPI() const;
	int   GetKindNoScan() const;
	int   GetKind() const;
	int   GetResolution() const;
	bool  IsOpaque() const                    { return GetKind() == IMAGE_OPAQUE; }

	const RGBA *Begin() const                 { return data ? ~data->buffer : NULL; }
	const RGBA *begin() const                 { return Begin(); }
	const RGBA *End() const                   { return Begin() + GetLength(); }
	const RGBA *end() const                   { return End(); }
	const RGBA* operator~() const             { return Begin(); }
	operator const RGBA*() const              { return Begin(); }
	const RGBA* operator[](int i) const       { ASSERT(data); return data->buffer[i]; }

	int64 GetSerialId() const                 { return data ? data->serial : 0; }
	bool  IsSame(const Image& img) const      { return GetSerialId() == img.GetSerialId(); }

	bool   operator==(const Image& img) const;
	bool   operator!=(const Image& img) const;
	hash_t GetHashValue() const;
	String ToString() const;

	void  Serialize(Stream& s);
	void  Xmlize(XmlIO& xio)                  { XmlizeBySerialize(xio, *this); }
	void  Jsonize(JsonIO& jio)                { JsonizeBySerialize(jio, *this); }
	void  Clear();

	Image& operator=(const Image& img);
	Image& operator=(ImageBuffer& img);

	bool IsNullInstance() const         { Size sz = GetSize(); return (sz.cx|sz.cy) == 0; }

	bool IsEmpty() const                { return IsNullInstance(); }
	operator Value() const              { return RichToValue(*this); }
	
	bool IsPaintOnly() const            { return data && data->paintonly; }
	bool IsPaintOnceHint() const        { return data && data->buffer.IsPaintOnceHint(); }

	Image()                             { data = NULL; }
	Image(const Nuller&)                { data = NULL; }
	Image(const Value& src);
	Image(const Image& img);
	Image(Image (*fn)());
	Image(ImageBuffer& b);
	~Image();

	// Defined in CtrlCore or by Rainbow:
	static Image Arrow();
	static Image Wait();
	static Image IBeam();
	static Image No();
	static Image SizeAll();
	static Image SizeHorz();
	static Image SizeVert();
	static Image SizeTopLeft();
	static Image SizeTop();
	static Image SizeTopRight();
	static Image SizeLeft();
	static Image SizeRight();
	static Image SizeBottomLeft();
	static Image SizeBottom();
	static Image SizeBottomRight();
	static Image Cross();
	static Image Hand();
	
	// standard mouse cursor support
	
	uint64       GetAuxData() const;

	// required by system image cache managemenent
	
	int GetRefCount() const         { return data ? (int)data->refcount : 0; }
};

Image Premultiply(const Image& img);
Image Unmultiply(const Image& img);

struct ImageIml : Moveable<ImageIml> {
	Image  image;
	dword  flags = 0;
};

Vector<ImageIml> UnpackImlData(const void *ptr, int len);
Vector<ImageIml> UnpackImlData(const String& d);

enum {
	GUI_MODE_NORMAL   = 0,
	GUI_MODE_DARK     = 1,
	GUI_MODE_UHD      = 2,
	GUI_MODE_DARK_UHD = 3,
};

enum {
	IML_IMAGE_FLAG_FIXED        = 0x1,
	IML_IMAGE_FLAG_FIXED_COLORS = 0x2,
	IML_IMAGE_FLAG_FIXED_SIZE   = 0x4,
	IML_IMAGE_FLAG_UHD          = 0x8,
	IML_IMAGE_FLAG_DARK         = 0x10,
};

class Iml {
	struct IImage : Moveable<IImage> {
		std::atomic<bool>  loaded;
		Image              image;

		IImage() { loaded = false; }
	};
	struct Data : Moveable<Data> {
		const char *data;
		int   len, count;
	};
	Vector<Data> data[4]; // 0 normal, 1 HiDPI - HD, 2 DK - Dark, 3 HDK - HiDPI + dark
	VectorMap<String, IImage> map;
	const char **name;
	dword global_flags = 0;
	bool  premultiply;

	Index<String> ex_name[3]; // 0 HiDPI - HD, 1 DK - Dark, 2 HDK - HiDPI + dark

	void  Init(int n);

public:
	void   Reset();
	int    GetCount() const                  { return map.GetCount(); }
	String GetId(int i)                      { return map.GetKey(i); }
	Image  Get(int i);
	int    Find(const String& id) const      { return map.Find(id); }
	void   Set(int i, const Image& img);

	ImageIml GetRaw(int mode, int i); // tries to get image for mode, can return Null
	ImageIml GetRaw(int mode, const String& id); // tries to get image for mode by id, can return Null

// these methods serve for .iml import
	Iml(const char **name, int n);//Deprecated - legacy .iml
	void AddData(const byte *data, int len, int count, int mode = 0);
	void AddId(int mode1, const char *name);
	void Premultiplied()                   { premultiply = false; }
	void GlobalFlag(dword f)               { global_flags |= f; }
	
	static void ResetAll(); // clears all .iml caches
};

void   Register(const char *imageclass, Iml& iml);

int    GetImlCount();
String GetImlName(int i);
Iml&   GetIml(int i);
int    FindIml(const char *name);
Image  GetImlImage(const char *name);
void   SetImlImage(const char *name, const Image& m);

String StoreImageAsString(const Image& img);
Image  LoadImageFromString(const String& s);
Size   GetImageStringSize(const String& src);
Size   GetImageStringDots(const String& src);

#include "Raster.h"
#include "ImageOp.h"
#include "SIMD.h"