#ifndef flagSKELETON

#include <CtrlLib/CtrlLib.h>

#ifdef GUI_COCOA

#include <CtrlCore/CocoMM.h>

#define LLOG(x) // DLOG(x)

namespace Upp {

struct CocoMenuBar;

};

@interface CocoMenu : NSMenu<NSMenuDelegate>
{
@public
	Upp::Ptr<Upp::CocoMenuBar> ptr;
	Upp::Event<Upp::Bar&>      proc;
}
-(void)cocoMenuAction:(id)sender;
@end

namespace Upp {

struct CocoMenuBar : public Bar {
	CocoMenu *cocomenu;
	int       lock = 0;
	bool      dockmenu = false;
	int       cy = 0; // estimate of height to place the menu correctly

	struct Item : Bar::Item {
		NSMenuItem      *nsitem;
		Event<>          cb;
		One<CocoMenuBar> submenu;

		virtual Item& Text(const char *text);
		virtual Item& Key(dword key);
		virtual Item& Image(const class Image& img);
		virtual Item& Check(bool check);
		virtual Item& Radio(bool check);
		virtual Item& Enable(bool _enable = true);
		virtual Item& Bold(bool bold = true);

		Item&   Label(const char *text);
		Item&   RightLabel(const char *text);

		Item& Key(KeyInfo& (*key)());

		Item()  { nsitem = [NSMenuItem new]; }
		~Item() { [nsitem release]; }
	};
	
	Array<Item> item;
	
	Item& AddItem() {
		Item& m = item.Add();
		[cocomenu addItem:m.nsitem];
		cy += GetStdFontCy();
		return m;
	}

	virtual bool IsMenuBar() const                  { return true; }

	virtual Item& AddItem(Event<> cb) {
		Item& m = AddItem();
		m.cb = cb;
		m.nsitem.target = cocomenu;
		m.nsitem.action = @selector(cocoMenuAction:);
		return m;
	}
	
	virtual Item&  AddSubMenu(Event<Bar&> proc) {
		Item& m = AddItem();
		m.submenu.Create();
		m.submenu->cocomenu->proc = proc;
		m.nsitem.action = @selector(cocoMenuAction:);
		m.nsitem.submenu = m.submenu->cocomenu;
		return m;
	}

	virtual void   AddCtrl(Ctrl *ctrl, int gapsize) {}
	virtual void   AddCtrl(Ctrl *ctrl, Size sz) {}

	virtual bool   IsEmpty() const;
	virtual void   Separator();
	
	void MenuAction(id item);
	
	void Set(Event<Bar&> bar);
	
	void Clear() {
		cy = 0;
		item.Clear();
		if(cocomenu) {
			[cocomenu release];
			cocomenu = NULL;
		}
	}
	void New() {
		Clear();
		cocomenu = [CocoMenu new];
		cocomenu.autoenablesItems = NO;
		cocomenu->ptr = this;
		cocomenu.delegate = cocomenu;
	}
	
	CocoMenuBar() {
		cocomenu = NULL;
		New();
	}
	~CocoMenuBar() {
		Clear();
	}
};

void CocoMenuBar::Set(Event<Bar&> bar)
{
	if(lock) return;
	lock++;
	[cocomenu removeAllItems];
	bar(*this);
	lock--;
}

void CocoMenuBar::MenuAction(id sender)
{
	for(const Item& m : item)
		if(m.nsitem == sender) {
			ResetCocoaMouse();
			if(GetParent()) // If not context menu use PostCallback to avoid visual glitches
				PostCallback(m.cb);
			else
				m.cb();
			break;
		}
}

void CocoMenuBar::Separator()
{
	[cocomenu addItem:[NSMenuItem separatorItem]];
	cy += GetStdFontCy();
}

CocoMenuBar::Item& CocoMenuBar::Item::Text(const char *text)
{
	String h;
	while(*text) {
		if(*text == '&') {
			text++;
			if(*text == '&') {
				h.Cat('&');
				text++;
			}
		}
		else
			h.Cat(*text++);
	}
	NSString *s = [NSString stringWithUTF8String:~h];
	nsitem.title = s;
	if(submenu)
		submenu->cocomenu.title = s;
	return *this;
}

CocoMenuBar::Item& CocoMenuBar::Item::Key(dword key)
{
	static Tuple2<int, int> code[] = {
		#include "NSMenuKeys.i"
	};
	auto *v = FindTuple(code, __countof(code), key & ~(K_CTRL|K_SHIFT|K_ALT|K_OPTION));
	if(v) {
		unichar chr = v->b;
		nsitem.keyEquivalent = [NSString stringWithCharacters:&chr length:1];
		nsitem.keyEquivalentModifierMask = (key & K_CTRL ? NSEventModifierFlagCommand : 0) |
		                                   (key & K_SHIFT ? NSEventModifierFlagShift : 0) |
		                                   (key & K_ALT ? NSEventModifierFlagControl : 0) |
		                                   (key & K_OPTION ? NSEventModifierFlagOption : 0);
	}
	return *this;
}

CocoMenuBar::Item& CocoMenuBar::Item::Image(const class Image& img)
{
	nsitem.image = GetNSImage(img);
	return *this;
}

CocoMenuBar::Item& CocoMenuBar::Item::Check(bool check)
{
	nsitem.state = check ? NSControlStateValueOn : NSControlStateValueOff;
	return *this;
}

CocoMenuBar::Item& CocoMenuBar::Item::Radio(bool check)
{
	return Check(check);
}

CocoMenuBar::Item& CocoMenuBar::Item::Enable(bool enable)
{
	nsitem.enabled = enable;
	return *this;
}

CocoMenuBar::Item& CocoMenuBar::Item::Bold(bool bold)
{
	return *this;
}

bool CocoMenuBar::IsEmpty() const
{
	return item.GetCount() == 0;
}

}

@implementation CocoMenu

-(void)cocoMenuAction:(id)sender {
	if(ptr)
		ptr->MenuAction(sender);
}

- (void)menuWillOpen:(NSMenu *)menu {
	CocoMenu *m = (CocoMenu *)menu;
	if(m && m->ptr && m->ptr->dockmenu)
		return;
	if(m && m->ptr && proc) {
		[m removeAllItems];
		proc(*m->ptr);
	}
}

- (void)menuDidClose:(NSMenu *)menu {
	CocoMenu *m = (CocoMenu *)menu;
	if(m && m->ptr && m->ptr->dockmenu)
		return;
	[m removeAllItems];
}

-(void)submenuAction:(id)sender {
	if(ptr)
		proc(*ptr);
	[super submenuAction:sender];
}

@end

namespace Upp {

void TopWindow::GuiPlatformDestruct()
{
	if(menubar)
		delete (CocoMenuBar *)menubar;
}

void TopWindow::SetMainMenu(Event<Bar&> menu)
{
	if(!menubar)
		menubar = new CocoMenuBar;
	CocoMenuBar& bar = *(CocoMenuBar *)menubar;
	bar.Set(menu);
	SyncMainMenu(true);
	MainMenu = menu;
}

bool TopWindow::HotKey(dword key)
{
	LLOG("TopWindow::HotKey " << GetKeyDesc(key));
	if(Bar::Scan(MainMenu, key))
		return true;
	return Ctrl::HotKey(key);
}

TopWindow *TopWindow::GetMenuTopWindow(bool dock)
{
	Ctrl *q = GetFocusCtrl();
	if(!q)
		q = GetActiveCtrl();
	if(!q)
		return NULL;
	q = q->GetTopCtrl();
	while(q) {
		TopWindow *w = dynamic_cast<TopWindow *>(q);
		if(w && (dock ? (bool)w->WhenDockMenu : (bool)w->menubar))
			return w;
		q = q->GetOwner();
	}
	return NULL;
}

void TopWindow::SyncMainMenu(bool force)
{
	TopWindow *w = GetMenuTopWindow(false);
	static TopWindow *current;
	if(w != current || force) {
		current = w;
		if(current) {
			CocoMenuBar& bar = *(CocoMenuBar *)current->menubar;
			[NSApp setMainMenu:bar.cocomenu];
			static NSMenu *dummy = [[NSMenu alloc] initWithTitle:@"Unused"];
			ONCELOCK { [dummy retain]; };
			[NSApp setHelpMenu: dummy]; //Avoid placing spotlight search field into Help
		}
	}
}

bool MenuBar::ExecuteHostBar(Ctrl *owner, Point p)
{
	if(host_bar && owner) {
		CocoMenuBar& bar = *(CocoMenuBar *)~host_bar;

		owner = owner->GetTopCtrl();
		
		int h = GetWorkArea().GetHeight();
		bool up = p.y > h / 2 && bar.cy + 2 * GetStdFontCy() + p.y > h;
		
		p -= owner->GetScreenRect().TopLeft();
		
		double scale = 1.0 / DPI(1);
		NSPoint np;
		np.x = scale * p.x;
		np.y = scale * p.y;
		
		ResetCocoaMouse(); // Because we will not get "MouseUp" event...

		return [bar.cocomenu popUpMenuPositioningItem:(up ? bar.item.Top().nsitem : nil)
	                                       atLocation:np
	                                           inView:(NSView *)owner->GetNSView()];
	}
	return false;
}

void MenuBar::CreateHostBar(One<Bar>& bar)
{
	host_bar.Create<CocoMenuBar>();
}

NSMenu *Cocoa_DockMenu() {
	Upp::TopWindow *w = Upp::TopWindow::GetMenuTopWindow(true);

	if(w && w->WhenDockMenu) {
		static Upp::CocoMenuBar bar;
		bar.dockmenu = true;
		bar.Clear();
		bar.cocomenu = [[[CocoMenu alloc] initWithTitle:@"DocTile Menu"] autorelease];
		bar.cocomenu.autoenablesItems = NO;
		bar.cocomenu->ptr = &bar;
		bar.cocomenu.delegate = bar.cocomenu;
		w->WhenDockMenu(bar);
		CocoMenu *m = bar.cocomenu;
		bar.cocomenu = NULL;
		return m;
	}
	return nil;
}

};

#endif

#endif
