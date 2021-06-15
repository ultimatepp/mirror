#include "CocoMM.h"

#ifdef GUI_COCOA

#define LLOG(x)  // LOG(x)

extern NSEvent *sCurrentMouseEvent__;

namespace Upp {

NSString *PasteboardType(const String& fmt)
{
	return decode(fmt, "text", NSPasteboardTypeString, "png", NSPasteboardTypePNG,
	                   "files", NSFilenamesPboardType, "url", NSURLPboardType,
	                   "rtf", NSPasteboardTypeRTF,
	                   [NSString stringWithUTF8String:~fmt]);
}


NSPasteboard *Pasteboard(bool dnd = false)
{
	return dnd ? [NSPasteboard pasteboardWithName:NSPasteboardNameDrag] : [NSPasteboard generalPasteboard];
}

};

@interface CocoClipboardOwner : NSObject {
	@public
	Upp::VectorMap<Upp::String, Upp::ClipData> data;
	Upp::Ptr<Upp::Ctrl> source;
	bool dnd;
}
@end

@implementation CocoClipboardOwner
-(void)pasteboard:(NSPasteboard *)sender provideDataForType:(NSString *)type
{
	auto render = [&](const Upp::String& fmt) -> Upp::String {
		int q = data.Find(fmt);
		if(q < 0)
			return Upp::Null;
		return data[q].Render();
	};
	
	NSPasteboard *pasteboard = Upp::Pasteboard(dnd);

	if([type isEqualTo:NSPasteboardTypeString]) {
		Upp::String raw = render("text");
		if(raw.GetCount() == 0 && source)
			raw = source->GetDropData("text");
	    [pasteboard setString:[NSString stringWithUTF8String:raw]
	                forType:type];
		return;
	}
	
	Upp::String fmt = [type isEqualTo:NSPasteboardTypePNG] ? "png" :
	                  [type isEqualTo:NSPasteboardTypeRTF] ? "rtf" :
	                                                          Upp::ToString(type);
	Upp::String raw = render(fmt);
	if(raw.GetCount() == 0 && source)
		raw = source->GetDropData(fmt);

	[pasteboard setData:[NSData dataWithBytes:~raw length:raw.GetCount()] forType:type];
}

- (void)pasteboardChangedOwner:(NSPasteboard *)sender
{
	LLOG("pasteboardCahandedOwner");
//	data.Clear();  // TODO: This seems to trigger on declareTypes despite owner being the same...
}

@end


namespace Upp {
	
CocoClipboardOwner *ClipboardOwner(bool dnd = false)
{
	GuiLock __;
	static CocoClipboardOwner *general = [[CocoClipboardOwner alloc] init];
	static CocoClipboardOwner *drag = [[CocoClipboardOwner alloc] init];
	general->dnd = false;
	drag->dnd = true;
	return dnd ? drag : general;
}

void ClearClipboard(bool dnd)
{
	GuiLock __;
	[Pasteboard(dnd) clearContents];
	ClipboardOwner()->data.Clear();
}

void ClearClipboard()
{
	ClearClipboard(false);
}

NSMutableSet *PasteboardTypes(const Vector<String>& fmt)
{
	NSMutableSet *types = [[[NSMutableSet alloc] init] autorelease];
	for(auto id : fmt)
		[types addObject:PasteboardType(id)];
	return types;
}

void AppendClipboard(bool dnd, const char *format, const Value& value, String (*render)(const Value& data))
{
	GuiLock __;

	auto& data = ClipboardOwner(dnd)->data;

	for(String fmt : Split(format, ';'))
		data.GetAdd(fmt) = ClipData(value, render);

	AutoreleasePool ___;

	[Pasteboard(dnd) declareTypes:[PasteboardTypes(data.GetKeys()) allObjects]
	                        owner:ClipboardOwner(dnd)];
}

Index<String> drop_formats;

void Ctrl::RegisterCocoaDropFormats()
{
	AutoreleasePool ___;
	NSView *nsview = (NSView *)GetNSView();
	[nsview registerForDraggedTypes:[PasteboardTypes(drop_formats.GetKeys()) allObjects]];
}

void Ctrl::RegisterDropFormats(const char *formats)
{
	int n = drop_formats.GetCount();
	for(String fmt : Split(formats, ';'))
		drop_formats.FindAdd(fmt);
	if(drop_formats.GetCount() != n)
		for(Ctrl *q : Ctrl::GetTopCtrls())
			q->RegisterCocoaDropFormats();
}

INITBLOCK {
	Ctrl::RegisterDropFormats("text;png;image;rtf;files;url");
}

void AppendClipboard(const char *format, const Value& value, String (*render)(const Value& data))
{
	AppendClipboard(false, format, value, render);
}

void AppendClipboard(const char *format, const String& data)
{
	GuiLock __;
	AppendClipboard(format, Value(data), NULL);
}

void AppendClipboard(const char *format, const byte *data, int length)
{
	GuiLock __;
	AppendClipboard(format, String(data, length));
}

bool IsFormatAvailable(NSPasteboard *pasteboard, const char *fmt)
{
	return [pasteboard availableTypeFromArray:[NSArray arrayWithObjects:PasteboardType(fmt), nil]];
}

String ReadFormat(NSPasteboard *pasteboard, const char *fmt)
{
	NSData *data = [pasteboard dataForType:PasteboardType(fmt)];
	return String((const char *)[data bytes], [data length]);
}

bool PasteClip::IsAvailable(const char *fmt) const
{
	return nspasteboard ? IsFormatAvailable((NSPasteboard *)nspasteboard, fmt)
	                    : IsClipboardAvailable(fmt);
}

String PasteClip::Get(const char *fmt) const
{
	return nspasteboard ? ReadFormat((NSPasteboard *)nspasteboard, fmt)
	                    : ReadClipboard(fmt);
}

void PasteClip::GuiPlatformConstruct()
{
	nspasteboard = NULL;
}

bool IsClipboardAvailable(const char *fmt)
{
	return IsFormatAvailable([NSPasteboard generalPasteboard], fmt);
}

String ReadClipboard(const char *fmt)
{
	return ReadFormat([NSPasteboard generalPasteboard], fmt);
}


bool IsClipboardAvailableText()
{
	return IsClipboardAvailable("text");
}

String ReadClipboardText()
{
	if(IsClipboardAvailableText())
		return ReadClipboard("text");
	return Null;
}

WString ReadClipboardUnicodeText()
{
	return ReadClipboardText().ToWString();
}

void AppendClipboardText(const String& s)
{
	AppendClipboard("text", s);
	// TODO Remove:
//	CFRef<CFStringRef> cs = CFStringCreateWithCString(NULL, (const char *)~s.ToString(), kCFStringEncodingUTF8);
  //  [[NSPasteboard generalPasteboard] setString:(NSString *)~cs forType:NSPasteboardTypeString];
}

void AppendClipboardUnicodeText(const WString& s)
{
	AppendClipboardText(s.ToString());
}

const char *ClipFmtsText()
{
	return "text";
}

String GetString(PasteClip& clip)
{
	GuiLock __;
	return clip.Get("text");
}

WString GetWString(PasteClip& clip)
{
	GuiLock __;
	return GetString(clip).ToWString();
}

bool AcceptText(PasteClip& clip)
{
	return clip.Accept(ClipFmtsText());
}

void Append(VectorMap<String, ClipData>& data, const String& text)
{
	data.GetAdd("text", ClipData(text, NULL));
}

void Append(VectorMap<String, ClipData>& data, const WString& text)
{
	data.GetAdd("text", ClipData(text.ToString(), NULL));
}

String GetTextClip(const WString& text, const String& fmt)
{
	return text.ToString();
}

String GetTextClip(const String& text, const String& fmt)
{
	return text;
}

const char *ClipFmtsImage()
{
	return "image;png";
}

bool AcceptImage(PasteClip& clip)
{
	GuiLock __;
	return clip.Accept(ClipFmtsImage());
}

Image GetImage(PasteClip& clip)
{
	GuiLock __;
	if(clip.Accept("image")) {
		Image m;
		LoadFromString(m, ~clip);
		if(!m.IsEmpty())
			return m;
	}
	if(clip.Accept("png"))
		return PNGRaster().LoadString(~clip);
	return Null;
}

Image ReadClipboardImage()
{
	GuiLock __;
	PasteClip d = Ctrl::Clipboard();
	return GetImage(d);
}

String sPng(const Value& image)
{
	if(IsNull(image))
		return Null;
	return PNGEncoder().SaveString(image);
}

String sImage(const Value& image)
{
	Image img = image;
	return StoreAsString(img);
}

String GetImageClip(const Image& img, const String& fmt)
{
	GuiLock __;
	if(img.IsEmpty()) return Null;
	if(fmt == "image")
		return sImage(img);
	if(fmt == "png")
		return sPng(img);
	return Null;
}

void Append(VectorMap<String, ClipData>& data, const Image& img)
{
	data.GetAdd("image", ClipData(img, sImage));
}

void AppendClipboardImage(const Image& img)
{
	GuiLock __;
	if(img.IsEmpty())
		return;
	AppendClipboard("image", img, sImage);
	AppendClipboard("png", img, sPng);
}

bool AcceptFiles(PasteClip& clip)
{
	if(clip.Accept("files;url")) {
		clip.SetAction(DND_COPY);
		return true;
	}
	return false;
}

bool IsAvailableFiles(PasteClip& clip)
{
	return clip.IsAvailable("files;url");
}

Vector<String> GetFiles(PasteClip& clip)
{
	GuiLock __;
	Vector<String> f;
	String raw;
	bool files = clip.IsAvailable("files");
	if(files)
		raw = clip.Get("files");
	else
	if(clip.IsAvailable("url"))
		raw = clip.Get("url");
	XmlNode n = ParseXML(raw);
	for(const auto& e : n["plist"]["array"])
		if(e.IsTag("string")) {
			String fn = e.GatherText();
			if(files ? fn.GetCount() : fn.TrimStart("file://"))
				f.Add(fn);
		}
	return f;
}

Ctrl * Ctrl::GetDragAndDropSource()
{
	return ClipboardOwner(true)->source;
}

};

@interface DNDSource : NSObject
{
	@public
	int actions;
	int result;
}
@end

@implementation DNDSource
- (NSDragOperation)draggingSourceOperationMaskForLocal:(BOOL)isLocal
{
    return actions;
}

- (void)draggedImage:(NSImage *)anImage endedAt:(NSPoint)aPoint operation:(NSDragOperation)operation
{
	result = operation;
}

@end

namespace Upp {

bool Ctrl::local_dnd_copy;

int Ctrl::DoDragAndDrop(const char *fmts, const Image& sample, dword actions,
                        const VectorMap<String, ClipData>& data)
{
	ASSERT_(sCurrentMouseEvent__, "Drag can only start within LeftDrag!");
	if(!sCurrentMouseEvent__)
		return DND_NONE;
	NSWindow *nswindow = (NSWindow *)GetTopCtrl()->GetNSWindow();
	ASSERT_(nswindow, "Ctrl is not in open window");
	if(!nswindow)
		return DND_NONE;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	ClearClipboard(true);
	ClipboardOwner(true)->source = this;
	for(int i = 0; i < data.GetCount(); i++) {
		RegisterDropFormats(data.GetKey(i));
		AppendClipboard(true, data.GetKey(i), data[i].data, data[i].render);
	}
	for(String fmt : Split(fmts, ';')) // GetDropData formats
		AppendClipboard(true, fmt, String(), NULL);

	CGImageRef cgimg = createCGImage(sample);

	Size isz = sample.GetSize();
	NSSize size;
	double scale = 1.0 / DPI(1);
	size.width = scale * isz.cx;
	size.height = scale * isz.cy;

	NSImage *nsimg = [[[NSImage alloc] initWithCGImage:cgimg size:size] autorelease];

	static DNDSource *src = [[DNDSource alloc] init];
	
	src->actions = 0;
	if(actions & DND_COPY)
		src->actions |= NSDragOperationCopy;
	if(actions & DND_MOVE)
		src->actions |= NSDragOperationMove;
	
	NSPoint p = [sCurrentMouseEvent__ locationInWindow];
	p.y -= size.height;
	
	local_dnd_copy = false; // macos does not have ability to change action in performDragOperation

	[nswindow dragImage:nsimg
	                 at:p
	             offset:NSMakeSize(0, 0)
	              event:sCurrentMouseEvent__
	         pasteboard:Pasteboard(true)
	             source:src
	          slideBack:YES];

	ClipboardOwner(true)->source = NULL;

	[pool release];

    CGImageRelease(cgimg);
    
    if(local_dnd_copy) // action was local and changed to copy in DragAndDrop
        return DND_COPY;
    
	return decode(src->result, NSDragOperationCopy, DND_COPY,
	                           NSDragOperationMove, DND_MOVE,
	                           DND_NONE);
}

void Ctrl::SetSelectionSource(const char *fmts) {}

Image MakeDragImage(const Image& arrow, Image sample);

Image MakeDragImage(const Image& arrow, const Image& arrow98, Image sample)
{
#ifdef PLATFORM_WIN32
	if(IsWin2K())
		return MakeDragImage(arrow, sample);
	else
#endif
		return arrow98;
}

};

#endif
