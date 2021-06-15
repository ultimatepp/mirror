#include "CocoMM.h"

#ifdef GUI_COCOA

#define LLOG(x)

@implementation CocoWindow

- (void)becomeKeyWindow {
	[super becomeKeyWindow];
}

- (BOOL)canBecomeKeyWindow {
    return active && ctrl && ctrl->IsEnabled();
}

- (BOOL)canBecomeMainWindow {
	LLOG("canBecomeMainWindow " << Upp::Name(ctrl) << ", owner " << Upp::Name(ctrl->GetOwner()));
	return active && ctrl && ctrl->IsEnabled() && dynamic_cast<Upp::TopWindow *>(~ctrl) && !ctrl->GetOwner();
}

- (NSMenu *)applicationDockMenu:(NSApplication *)sender {
	NSMenu *menu = [[[NSMenu alloc] initWithTitle:@"DocTile Menu"] autorelease];
	NSMenuItem *item = [[[NSMenuItem alloc] initWithTitle:@"Hello" action:@selector(hello) keyEquivalent:@"k"] autorelease];
	[menu addItem:item];
	return menu;
}

@end

namespace Upp {

static Vector<Ptr<Ctrl>> mmtopctrl; // should work without Ptr, but let us be defensive....

Ctrl *Ctrl::GetOwner()
{
	GuiLock __;
	return top && top->coco ? top->coco->owner : NULL;
}

Ctrl *Ctrl::GetActiveCtrl()
{
	GuiLock __;
	for(Ctrl *p : mmtopctrl)
		if(p && p->top && p->top->coco && p->top->coco->window.keyWindow)
			return p;
	return lastActive;
}

bool Ctrl::SetWndFocus()
{
	GuiLock __;
	if(top && top->coco) {
		[top->coco->window orderFront:top->coco->window];
		if([top->coco->window canBecomeKeyWindow])
			[top->coco->window makeKeyWindow];
		if(dynamic_cast<TopWindow *>(this) && [top->coco->window canBecomeMainWindow])
			[top->coco->window makeMainWindow];
	}
	return true;
}

bool Ctrl::HasWndFocus() const
{
	GuiLock __;
	return GetActiveCtrl() == this;
}


void Ctrl::SetWndForeground()
{
	GuiLock __;
	SetWndFocus();
}

bool Ctrl::IsWndForeground() const
{
	GuiLock __;
	return HasWndFocus();
}

NSRect DesktopRect(const Rect& r)
{
	double scalei = 1.0 / DPI(1);
	return NSMakeRect(scalei * r.left,
	                  scalei * (Ctrl::GetScreenArea(r.TopLeft()).GetHeight() - r.top - r.GetHeight()),
	                  scalei * r.GetWidth(), scalei * r.GetHeight());
}

void *Ctrl::GetNSWindow() const
{
	return top && top->coco ? top->coco->window : NULL;
}

void *Ctrl::GetNSView() const
{
	return top && top->coco ? top->coco->view : NULL;
}

void Ctrl::Create(Ctrl *owner, dword style, bool active)
{
	if(owner)
		owner = owner->GetTopCtrl();

	top = new Top;
	top->coco = new CocoTop;
	top->coco->owner = owner;
	
	NSRect frame = DesktopRect(GetRect());
	CocoWindow *window = [[CocoWindow alloc] initWithContentRect:frame styleMask: style
	                                         backing:NSBackingStoreBuffered defer:false];
	top->coco->window = window;
	if(owner && owner->top && owner->top->coco)
		[owner->top->coco->window addChildWindow:window ordered:NSWindowAbove];

	window->ctrl = this;
	window->active = active;
	window.backgroundColor = [NSColor clearColor];

	isopen = true;
		
	CocoView *view = [[[CocoView alloc] initWithFrame:frame] autorelease];
	view->ctrl = this;
	top->coco->view = view;
	[window setContentView:view];
	[window setDelegate:view];
	[window setAcceptsMouseMovedEvents:YES];
	[window makeFirstResponder:view];
	[window makeKeyAndOrderFront:window];
	
	ONCELOCK {
		[NSApp activateIgnoringOtherApps:YES];
	}

	MMCtrl::SyncRect(view);
	mmtopctrl.Add(this);

	RegisterCocoaDropFormats();
	
	StateH(OPEN);
}

void Ctrl::WndDestroy()
{
	LLOG("WndDestroy " << Name());
	if(!top)
		return;
	bool focus = HasFocusDeep();
	Ptr<Ctrl> owner = GetOwner();
	[top->coco->window close];
	delete top->coco;
	delete top;
	top = NULL;
	popup = isopen = false;
	int ii = FindIndex(mmtopctrl, this);
	if(ii >= 0)
		mmtopctrl.Remove(ii);
	if(owner && owner->IsOpen() && (focus || !GetFocusCtrl()))
		owner->SetWndFocus();
}

Vector<Ctrl *> Ctrl::GetTopCtrls()
{
	Vector<Ctrl *> h;
	for(Ctrl *p : mmtopctrl)
		if(p) h.Add(p);
	return h;
}

void Ctrl::WndInvalidateRect(const Rect& r)
{
	GuiLock __;
	LLOG("Invalidate Rect " << r);
	if(top)
		[top->coco->view setNeedsDisplayInRect:(NSRect)CGRectDPI(r.Inflated(10, 10))];
}

void Ctrl::WndScrollView(const Rect& r, int dx, int dy)
{
	GuiLock __;
	LLOG("Scroll View " << r);
	WndInvalidateRect(r);
}

bool Ctrl::IsWndOpen() const {
	GuiLock __;
//	DLOG("IsWndOpen " << Upp::Name(this) << ": " << (bool)top);
	return top;
}

void Ctrl::PopUp(Ctrl *owner, bool savebits, bool activate, bool dropshadow, bool topmost)
{
	Create(owner ? owner->GetTopCtrl() : GetActiveCtrl(), NSWindowStyleMaskBorderless, 0);
	popup = true;
	if(activate && IsEnabled())
		SetFocus();
}

dword TopWindow::GetMMStyle() const
{
	dword style = 0;
	if(!frameless)
		style |= NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskResizable;
	if(minimizebox)
		style |= NSWindowStyleMaskMiniaturizable;
//	if(maximizebox)
//		style |= ;
	return style;
}

void TopWindow::Open(Ctrl *owner)
{
	GuiLock __;
	SetupRect(owner);
	if((owner && center == 1) || center == 2)
		SetRect((center == 1 ? owner->GetRect() : owner ? owner->GetWorkArea()
		                                                : GetPrimaryWorkArea())
		        .CenterRect(GetRect().GetSize()));
	placefocus = true;
	Create(owner, GetMMStyle(), true);
	SyncCaption();
	SyncSizeHints();
	if(state == MAXIMIZED)
		Maximize();
	if(state == MINIMIZED)
		Minimize();
	// Note: window is activated and PlaceFocus invoked by event, later
}

void TopWindow::Open()
{
	Open(GetActiveCtrl());
}

void TopWindow::OpenMain()
{
	Open(NULL);
}

void TopWindow::SyncTitle()
{
	GuiLock __;
	if(top) {
		LLOG("SyncTitle " << title);
		CFRef<CFStringRef> s = CFStringCreateWithCString(NULL, (const char *)~title.ToString(), kCFStringEncodingUTF8);
		[top->coco->window setTitle:(NSString *)~s];
	}
}

void Ctrl::SyncAppIcon()
{
	Ctrl *q = GetFocusCtrl();
	if(!q)
		q = lastActive;
	if(q) {
		q = q->GetTopWindow();
		while(q->GetOwner())
			q = q->GetOwner();
		TopWindow *w = dynamic_cast<TopWindow *>(q);
		if(w) {
			Image m = Nvl(w->GetLargeIcon(), w->GetIcon());
			if(!IsNull(m))
				SetNSAppImage(m);
		}
	}
}

void TopWindow::SyncCaption()
{
	GuiLock __;
	if(top) {
		SyncTitle();
		NSWindow *window = top->coco->window;
		[[window standardWindowButton:NSWindowMiniaturizeButton] setHidden:!minimizebox];
		[[window standardWindowButton:NSWindowZoomButton] setHidden:!maximizebox];
	}
	SyncAppIcon();
}

CGSize MMFrameSize(Size sz, dword style)
{
	double scale = 1.0 / DPI(1);
	return [NSWindow frameRectForContentRect:
				(NSRect)CGRectMake(100, 100, scale * sz.cx, scale * sz.cy) styleMask:style].size;
}

void TopWindow::SyncSizeHints()
{
	GuiLock __;
	if(top) {
		NSWindow *window = top->coco->window;
		dword style = GetMMStyle();
		Size sz = GetRect().GetSize();
		[window setMinSize:MMFrameSize(sizeable ? GetMinSize() : sz, style)];
		[window setMaxSize:MMFrameSize(sizeable ? GetMaxSize() : sz, style)];
	}
}

Rect Ctrl::GetWndScreenRect() const
{ // THIS IS NOT NEEDED
	GuiLock __;
	Rect r = GetRect();
	return r;
}

void Ctrl::WndSetPos(const Rect& rect)
{
	GuiLock __;
	if(top)
		[top->coco->window setFrame:
			[top->coco->window frameRectForContentRect:DesktopRect(rect)]
			display:YES];
}

void TopWindow::SerializePlacement(Stream& s, bool reminimize)
{
	GuiLock __;
	int version = 0;
	s / version;
	Rect rect = GetRect();
	s % rect;
	if(s.IsLoading())
		SetRect(rect);
}

void TopWindow::Maximize(bool effect)
{
	state = MAXIMIZED;
	if(top && top->coco && top->coco->window && !top->coco->window.zoomed) {
		if(effect)
			[top->coco->window performZoom:top->coco->window];
		else
			[top->coco->window zoom:top->coco->window];
	}
}

void TopWindow::Minimize(bool effect)
{
	state = MINIMIZED;
	if(top && top->coco && top->coco->window && !top->coco->window.miniaturized) {
		if(effect)
			[top->coco->window performMiniaturize:top->coco->window];
		else
			[top->coco->window miniaturize:top->coco->window];
	}
}

void TopWindow::Overlap(bool effect)
{
	state = OVERLAPPED;
	if(top && top->coco && top->coco->window && top->coco->window.zoomed)
		[top->coco->window zoom:top->coco->window];
	if(top && top->coco && top->coco->window && top->coco->window.miniaturized)
		[top->coco->window deminiaturize:top->coco->window];
}

bool Ctrl::IsCocoActive() const
{
	return top && top->coco && top->coco->window && top->coco->window->active;
}

}

#endif
