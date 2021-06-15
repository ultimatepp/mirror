#include "CocoMM.h"

#ifdef GUI_COCOA

NSEvent *sCurrentMouseEvent__; // needed for drag operation

#define LLOG(x) // DLOG(x)

static Upp::Point coco_mouse_pos;
static bool       coco_mouse_left;
static bool       coco_mouse_right;
static int        coco_flags;
static Upp::Ptr<Upp::Ctrl> coco_capture;

Upp::Ptr<Upp::Ctrl> Upp::Ctrl::lastActive;

namespace Upp {
	
extern id menubar;


bool  GetShift() { return coco_flags & NSEventModifierFlagShift; }
bool  GetCtrl() { return coco_flags & NSEventModifierFlagCommand; }
bool  GetAlt() { return coco_flags & NSEventModifierFlagControl; }
bool  GetOption() { return coco_flags & NSEventModifierFlagOption; }
bool  GetCapsLock() { return coco_flags & NSEventModifierFlagCapsLock; }

/*
NSEventModifierFlagOption
NSEventModifierFlagNumericPad
NSEventModifierFlagHelp
NSEventModifierFlagFunction
NSEventModifierFlagDeviceIndependentFlagsMask
*/

bool  GetMouseLeft() { return coco_mouse_left; }
bool  GetMouseRight() { return coco_mouse_right; }
bool  GetMouseMiddle() { return false; } // TODO

Point GetMousePos() { return coco_mouse_pos; } // TODO: read it if no mouse events

void Ctrl::WndEnable(bool)
{
	GuiLock __;
	// empty - we are testing the status in events....
}

bool Ctrl::SetWndCapture()
{
	GuiLock __;
	LLOG("SetWndCapture");
	coco_capture = this;
	return true; // TODO: Always?
}

bool Ctrl::HasWndCapture() const
{
	GuiLock __;
	return coco_capture == this;
}

bool Ctrl::ReleaseWndCapture()
{
	GuiLock __;
	LLOG("ReleaseWndCapture");
	coco_capture = NULL;
	return true;
}

void Ctrl::ResetCocoaMouse()
{
	ReleaseCtrlCapture();
	coco_mouse_left = coco_mouse_right = false;
}

struct MMImp {
	static bool KeyFlags(Upp::Ctrl *ctrl, NSEvent *e) {
		bool alt = GetAlt();
		bool ctl = GetCtrl();
		bool sht = GetShift();
		bool opt = GetOption();
		Flags(e);
		if(!ctrl->IsEnabled())
			return false;
		
		if(alt != GetAlt())
			ctrl->DispatchKey(K_ALT_KEY|(alt * K_KEYUP), 1);
		if(ctl != GetCtrl())
			ctrl->DispatchKey(K_CTRL_KEY|(ctl * K_KEYUP), 1);
		if(sht != GetShift())
			ctrl->DispatchKey(K_SHIFT_KEY|(sht * K_KEYUP), 1);
		if(opt != GetOption())
			ctrl->DispatchKey(K_OPTION_KEY|(opt * K_KEYUP), 1);

		return true;
	}

	static void Flags(NSEvent *e)
	{
		coco_flags = [e modifierFlags];
	}
	
	static bool MouseEvent(CocoView *view, NSEvent *e, int event, double zd = 0)
	{
		Flags(e);
		sCurrentMouseEvent__ = e;
		if((event & Ctrl::ACTION) == Ctrl::UP && Ctrl::ignoreclick) {
			Ctrl::KillRepeat();
			Ctrl::ignoreclick = false;
			Ctrl::ignoremouseup = false;
			return false;
		}
		if(GetAlt()) { // Emulate right click
			if(event == Ctrl::LEFTDOWN) {
				coco_mouse_left = false;
				coco_mouse_right = true;
				event = Ctrl::RIGHTDOWN;
			}
			if(event == Ctrl::LEFTUP) {
				coco_mouse_right = false;
				event = Ctrl::RIGHTUP;
			}
		}
		NSPoint np = [view convertPoint:[e locationInWindow] fromView:nil];
		Rect r = view->ctrl->GetRect();
		Upp::Point p(DPI(np.x), DPI(np.y));
		coco_mouse_pos = p + r.TopLeft();

		if(event == Ctrl::MOUSEMOVE) {
			static Point coco_mouse_pos_old(-10000, -10000);
			if(coco_mouse_pos_old == coco_mouse_pos) { // duplicate for another window, ignore
				sCurrentMouseEvent__ = NULL;
				return false;
			}
			coco_mouse_pos_old = coco_mouse_pos;

			if(coco_capture)
				coco_capture->DispatchMouse(event, coco_mouse_pos - coco_capture->GetScreenRect().TopLeft(), 120 * sgn(zd));
			else {
				Vector<Ctrl *> t = Ctrl::GetTopCtrls(); // Find window that contains the mouse, from the top
				for(NSNumber *num in [NSWindow windowNumbersWithOptions:0]) { // All app windows
					NSWindow *win = [NSApp windowWithWindowNumber:[num integerValue]];
					if(win) {
					    int q = FindMatch(t, [&](Ctrl *t) { return t->GetNSWindow() == win; });
					    if(q >= 0) {
					        Ctrl *w = t[q];
					        Rect r = w->GetRect(); // same as ScreenRect
					        if(w->IsEnabled() && r.Contains(coco_mouse_pos)) {
								w->DispatchMouse(event, coco_mouse_pos - r.TopLeft(), 120 * sgn(zd));
								break;
					        }
					    }
					}
				}
			}
		}
		else
		if(view->ctrl->IsEnabled() && (view->ctrl->HasWndCapture() || r.Contains(coco_mouse_pos))) {
	        if((event & Ctrl::ACTION) == Ctrl::DOWN && !view->ctrl->HasFocusDeep() && view->ctrl->IsWantFocus())
	            view->ctrl->SetFocus();
			view->ctrl->DispatchMouse(event, p, 120 * sgn(zd));
		}
		
		sCurrentMouseEvent__ = NULL;
		return false;
	}

	static bool MouseDownEvent(CocoView *view, NSEvent *e, int button)
	{
		Upp::Ctrl::lastActive = view->ctrl;
		if(Ctrl::ignoremouseup) {
			Ctrl::KillRepeat();
			Ctrl::ignoreclick = false;
			Ctrl::ignoremouseup = false;
		}
		if(Ctrl::ignoreclick)
			return false;
		static int clicktime = msecs() - 100000;
		static CocoView *dbl_view;
		static NSPoint   dbl_pos;
		NSPoint np = [view convertPoint:[e locationInWindow] fromView:nil];
		bool b;
		if(msecs(clicktime) < GUI_DblClickTime() && view == dbl_view &&
		   max(abs(np.x - dbl_pos.x), abs(np.y - dbl_pos.y)) < 3) {
			b = MouseEvent(view, e, button|Upp::Ctrl::DOUBLE);
			clicktime = msecs() - 999999;
		}
		else {
			b = MouseEvent(view, e, button|Upp::Ctrl::DOWN);
			clicktime = msecs();
		}
		dbl_view = view;
		dbl_pos = np;
		return b;
	}
	
	static void Paint(Upp::Ctrl *ctrl, Upp::SystemDraw& w, const Rect& r)
	{
		ctrl->fullrefresh = false;
		ctrl->UpdateArea(w, r);
	}

	static bool KeyEvent(Upp::Ctrl *ctrl, NSEvent *e, int up) {
		Flags(e);
		if(!ctrl->IsEnabled())
			return false;
		Upp::dword k = e.keyCode;
		WString x = ToWString((CFStringRef)(e.charactersIgnoringModifiers));
		if(x.GetCount() == 1)
		#define KEY(x) case #x[0]: k = kVK_ANSI_##x; break;
			switch(ToUpper(x[0])) {
				KEY(A); KEY(B); KEY(C); KEY(D); KEY(E); KEY(F); KEY(G); KEY(H); KEY(I); KEY(J);
				KEY(K); KEY(L); KEY(M); KEY(N); KEY(O); KEY(P); KEY(Q); KEY(R); KEY(S); KEY(T);
				KEY(U); KEY(V); KEY(W); KEY(X); KEY(Y); KEY(Z);
			}
		#undef KEY

		k = decode(k, kVK_ANSI_KeypadEnter, K_ENTER, kVK_Tab, K_TAB, K_DELTA|k)|up;

		if(GetCtrl())
			k |= K_CTRL;
		if(GetShift())
			k |= K_SHIFT;
		if(GetAlt())
			k |= K_ALT;
		if(GetOption())
			k |= K_OPTION;
		
		if(e.keyCode == kVK_Help) // TODO: This is Insert key, but all this is dubious
			ctrl->DispatchKey(k & ~K_KEYUP, 1);

		ctrl->DispatchKey(k, 1);
		if(!up && !(k & (K_CTRL|K_ALT))) {
			WString x = ToWString((CFStringRef)(e.characters));
			for(wchar c : x) {
				if(c < 0xF700 &&
				   (c > 32 && c != 127 || /*c == 9 && !GetOption() || */c == 32 && !GetShift()))
					ctrl->DispatchKey(c, 1);
			}
			if(e.keyCode == kVK_ANSI_KeypadEnter && *x != 13)
				ctrl->DispatchKey(13, 1);
		}
		return true;
	}

	static void BecomeKey(Upp::Ctrl *ctrl)
	{
		LLOG("Become key " << Upp::Name(ctrl));
		Upp::Ctrl::lastActive = ctrl;
		ctrl->ActivateWnd();
		auto tw = dynamic_cast<TopWindow *>(ctrl);
		if(tw && tw->placefocus) {
			tw->PlaceFocus();
			tw->placefocus = false;
		}
		Upp::Ctrl::SyncAppIcon();
		Upp::TopWindow::SyncMainMenu(false);
		Upp::Ctrl::ReleaseCtrlCapture();
	}

	static void ResignKey(Upp::Ctrl *ctrl)
	{
		LLOG("Resign key " << Upp::Name(ctrl));
		ctrl->KillFocusWnd();
		Upp::Ctrl::ReleaseCtrlCapture();
	}
	
	static void DoClose(Upp::Ctrl *ctrl)
	{
		ctrl->MMClose();
		Upp::Ctrl::ReleaseCtrlCapture();
	}

	static int  DnD(Upp::Ctrl *ctrl, id<NSDraggingInfo> info, bool paste = false)
	{
		NSView *nsview = (NSView *)ctrl->GetNSView();
		PasteClip clip;
		clip.nspasteboard = info.draggingPasteboard;
		clip.paste = paste;
		clip.accepted = false;
		clip.allowed = DND_MOVE|DND_COPY; // TODO: Use draggingSourceOperationMask
		clip.action = info.draggingSourceOperationMask & NSDragOperationMove ? DND_MOVE
		                                                                     : DND_COPY;
		NSPoint np = [nsview convertPoint:[info draggingLocation] fromView:nil];
		ctrl->DnD(Upp::Point(DPI(np.x), DPI(np.y)) + ctrl->GetScreenRect().TopLeft(), clip);
		if(paste && clip.IsAccepted() && clip.GetAction() == DND_COPY)
			Ctrl::local_dnd_copy = true;
		return clip.IsAccepted() ? clip.GetAction() == DND_MOVE ? NSDragOperationMove
		                                                        : NSDragOperationCopy
		                         : NSDragOperationNone;
	}
	
	static void DnDLeave(Ctrl *ctrl)
	{
		ctrl->DnDLeave();
	}
	
	static void DoCursorShape()
	{
		Ctrl::DoCursorShape();
	}
};

};

@implementation CocoView

-(BOOL)isFlipped {
	return YES;
}

-(void)drawRect:(NSRect)r {
	if(ctrl) {
		Upp::SystemDraw w([[NSGraphicsContext currentContext] CGContext], self);
		Upp::MMImp::Paint(ctrl, w, MakeRect(r, Upp::DPI(1)));
	}
}

- (void)mouseDown:(NSEvent *)e {
	coco_mouse_left = true;
	if(!Upp::MMImp::MouseDownEvent(self, e, Upp::Ctrl::LEFT))
		[super mouseDown:e];
}

- (void)mouseUp:(NSEvent *)e {
	coco_mouse_left = false;
	if(!Upp::MMImp::MouseEvent(self, e, Upp::Ctrl::LEFTUP))
		[super mouseUp:e];
	Upp::Ctrl::ReleaseCtrlCapture();
}

- (void)mouseMoved:(NSEvent *)e {
	if(!Upp::MMImp::MouseEvent(self, e, Upp::Ctrl::MOUSEMOVE))
		[super mouseMoved:e];
}

- (void)mouseDragged:(NSEvent *)e {
	if(!Upp::MMImp::MouseEvent(self, e, Upp::Ctrl::MOUSEMOVE))
		[super mouseDragged:e];
}

- (void)rightMouseDown:(NSEvent*)e {
	coco_mouse_right = true;
	if(!Upp::MMImp::MouseDownEvent(self, e, Upp::Ctrl::RIGHT))
		[super rightMouseDown:e];
}

- (void)rightMouseUp:(NSEvent*)e {
	coco_mouse_right = false;
	if(!Upp::MMImp::MouseEvent(self, e, Upp::Ctrl::RIGHTUP))
		[super rightMouseUp:e];
}

- (void)scrollWheel:(NSEvent *)e {
	if(!Upp::MMImp::MouseEvent(self, e, Upp::Ctrl::MOUSEWHEEL, [e deltaY]))
		[super scrollWheel:e];
}

- (void)keyDown:(NSEvent *)e {
	if(!Upp::MMImp::KeyEvent(ctrl, e, 0))
		[super keyDown:e];
}

- (void)keyUp:(NSEvent *)e {
	if(!Upp::MMImp::KeyEvent(ctrl, e, Upp::K_KEYUP))
			[super keyUp:e];
}

- (void)flagsChanged:(NSEvent *)e {
	if(!Upp::MMImp::KeyFlags(ctrl, e))
		[super flagsChanged:e];
}

- (void)cursorUpdate:(NSEvent *)event {
	Upp::MMImp::Flags(event);
	Upp::MMImp::DoCursorShape();
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
	if(ctrl->IsEnabled())
		Upp::MMImp::DoClose(ctrl);
	return NO;
}

- (void)windowDidResize:(NSNotification *)notification {
	Upp::MMCtrl::SyncRect(self);
}

- (void)windowDidMove:(NSNotification *)notification {
	Upp::MMCtrl::SyncRect(self);
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
	Upp::MMImp::BecomeKey(ctrl);
}

- (void)windowDidResignKey:(NSNotification *)notification {
	Upp::MMImp::ResignKey(ctrl);
}

- (BOOL)acceptsFirstResponder {
	return YES;
}

- (BOOL)canBecomeKeyView {
	return ctrl->IsEnabled();
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
	return Upp::MMImp::DnD(ctrl, sender);
}

- (NSDragOperation)draggingUpdated:(id <NSDraggingInfo>)sender
{
	return Upp::MMImp::DnD(ctrl, sender);
}

- (void)draggingEnded:(id <NSDraggingInfo>)sender
{
	Upp::MMImp::DnDLeave(ctrl);
}

- (void)draggingExited:(id <NSDraggingInfo>)sender
{
	Upp::MMImp::DnDLeave(ctrl);
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
	return Upp::MMImp::DnD(ctrl, sender, true) != NSDragOperationNone;
}

- (void)updateTrackingAreas
{
	for(NSTrackingArea *t in [self trackingAreas])
		[self removeTrackingArea:t];

	Upp::Size sz = ctrl->GetScreenRect().GetSize();
	NSTrackingArea *ta = [[NSTrackingArea alloc]
		initWithRect:NSMakeRect(0, 0, sz.cx, sz.cy)
	         options:NSTrackingMouseEnteredAndExited|NSTrackingActiveAlways|
	                 NSTrackingInVisibleRect|NSTrackingMouseMoved|NSTrackingCursorUpdate
	           owner:self
	        userInfo:nil];
	[self addTrackingArea:ta];
	[ta release];
}

@end

#endif
