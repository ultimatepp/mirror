#include "MenuImp.h"

#ifdef PLATFORM_WIN32
#include <mmsystem.h>
#endif

namespace Upp {

#define LLOG(x)    // DLOG(x)
#define LTIMING(x) // RTIMING(x)

MenuItemBase::MenuItemBase()
{
	accel = 0;
	state = 0;
	isenabled = true;
	type = 0;
	font = StdFont();
	leftgap = DPI(16) + Zx(6);
	textgap = Zy(6);
	accesskey = 0;
	NoWantFocus();
	style = &MenuBar::StyleDefault();
	Transparent();
	maxiconsize = Size(INT_MAX, INT_MAX);
}

MenuBar *MenuItemBase::GetMenuBar() const
{
	Ctrl *q = GetParent();
	while(q) {
		MenuBar *bar = dynamic_cast<MenuBar *>(q);
		if(bar)
			return bar;
		q = q->GetParent();
	}
	return NULL;
}

bool MenuItemBase::InOpaqueBar() const
{
	const MenuBar *bar = GetMenuBar();
	return !(bar && bar->IsTransparent());
}

Bar::Item& MenuItemBase::Text(const char *s)
{
	accesskey = ExtractAccessKey(s, text);
	Refresh();
	return *this;
}

Bar::Item& MenuItemBase::Key(dword key)
{
	if(key) {
		accel = key;
		Refresh();
	}
	return *this;
}

Bar::Item& MenuItemBase::Image(const UPP::Image& img)
{
	return *this;
}

Bar::Item& MenuItemBase::Check(bool check)
{
	type = CHECK0 + check;
	return *this;
}

Bar::Item& MenuItemBase::Radio(bool check)
{
	type = RADIO0 + check;
	return *this;
}

Bar::Item& MenuItemBase::Bold(bool bold)
{
	font.Bold(bold);
	return *this;
}

Bar::Item& MenuItemBase::Tip(const char *s)
{
	return *this;
}

Bar::Item& MenuItemBase::Help(const char *s)
{
	HelpLine(s);
	return *this;
}

Bar::Item& MenuItemBase::Description(const char *s)
{
	Ctrl::Description(s);
	return *this;
}

Bar::Item& MenuItemBase::Topic(const char *s)
{
	HelpTopic(s);
	return *this;
}

Bar::Item& MenuItemBase::Enable(bool en)
{
	isenabled = en;
	Refresh();
	return *this;
}

String MenuItemBase::GetDesc() const
{
	return text;
}

dword  MenuItemBase::GetAccessKeys() const
{
	return AccessKeyBit(accesskey);
}

void   MenuItemBase::AssignAccessKeys(dword used)
{
	if(!accesskey) {
		accesskey = ChooseAccessKey(text, used);
		used |= AccessKeyBit(accesskey);
	}
	Ctrl::AssignAccessKeys(used);
}

void DrawMnemonicText(Draw& w, int x, int y, const String& s, Font font, Color color,
                      int mnemonic, bool menumark)
{
	int apos = HIWORD(mnemonic);
	int q;
	if(apos && apos < s.GetLength())
		q = apos - 1;
	else {
		q = s.Find(ToUpper(mnemonic));
		if(q < 0)
			q = s.Find(ToLower(mnemonic));
	}
	w.DrawText(x, y, s, font, color);
	if(q < 0) return;
	FontInfo f = font.Info();
	w.DrawRect(x + GetTextSize(~s, font, q).cx, y + f.GetAscent() + 1, f[s[q]], 1,
	           menumark ? SColorMenuMark() : SColorMark());
}

void DrawMenuText(Draw& w, int x, int y, const String& s, Font f, bool enabled,
                  bool hl, int mnemonic, Color color, Color hlcolor, bool menumark)
{
	if(enabled)
		DrawMnemonicText(w, x, y, s, f, hl ? hlcolor : color, mnemonic, menumark);
	else {
		if(GUI_GlobalStyle() >= GUISTYLE_XP)
			DrawMnemonicText(w, x, y, s, f, SColorDisabled, 0, menumark);
		else {
			DrawMnemonicText(w, x + 1, y + 1, s, f, SColorPaper, 0, menumark);
			DrawMnemonicText(w, x, y, s, f, SColorDisabled, 0, menumark);
		}
	}
}

void MenuItemBase::DrawMenuText(Draw& w, int x, int y, const String& s, Font f, bool enabled,
                                bool hl, Color color, Color hlcolor)
{
	UPP::DrawMenuText(w, x, y, s, f, enabled, hl, VisibleAccessKeys() ? accesskey : 0,
	                  color, hlcolor, InOpaqueBar());
}

void MenuItemBase::PaintTopItem(Draw& w, int state) {
	Size sz = GetSize();
	String text = GetText();
	Size isz = GetTextSize(text, StdFont());
	Point tp = (sz - isz) / 2;
	if(GUI_GlobalStyle() >= GUISTYLE_XP) {
		bool opaque = InOpaqueBar();
		bool opaque2 = opaque || state;
		Color bg = SColorFace();
		if(opaque2)
			ChPaint(w, 0, 0, sz.cx, sz.cy, style->topitem[state]);
		else
		if(opaque)
			w.DrawRect(0, 0, sz.cx, sz.cy, bg);
		Color txt = opaque ? style->topitemtext[0] : GetLabelTextColor(this);
		Color hltxt = opaque2 ? style->topitemtext[state] : GetLabelTextColor(this);
		if(!opaque && state != 2 && style->opaquetest) { // Fix issues when text color is not compatible with transparent background (e.g. Ubuntu Ambience)]
			Color c = state == 1 && !IsNull(style->topitem[0]) ? SColorHighlight() : bg;
			int g = Grayscale(c);
			bool dark = IsDark(c);
			if(abs(g - Grayscale(txt)) < 70)
				txt = dark ? White() : Black();
			if(abs(g - Grayscale(hltxt)) < 70)
				hltxt = dark ? White() : Black();
		}
		DrawMenuText(w, tp.x, tp.y, text, GetFont(), IsItemEnabled(), state,
		             txt, hltxt);
	}
	else {
		w.DrawRect(sz, SColorFace);
		static const ColorF b0[] = { (ColorF)1, SColorLight, SColorLight, SColorShadow, SColorShadow, };
		static const ColorF b1[] = { (ColorF)1, SColorShadow, SColorShadow, SColorLight, SColorLight, };
		DrawMenuText(w, tp.x, tp.y, text, GetFont(), IsItemEnabled(), false,
		             SColorMenuText, SColorHighlightText);
		if(state)
			DrawBorder(w, 0, 0, sz.cx, sz.cy, state == 2 ? b1 : b0);
	}
}

// -------------------------------------

Bar::Item& MenuItem::Image(const UPP::Image& img)
{
	licon = img;
	Refresh();
	return *this;
}

MenuItem& MenuItem::RightImage(const UPP::Image& img)
{
	ricon = img;
	Refresh();
	return *this;
}

void MenuItem::SendHelpLine()
{
	BarCtrl::SendHelpLine(this);
}

void MenuItem::ClearHelpLine()
{
	BarCtrl::ClearHelpLine(this);
}

void MenuItem::MouseEnter(Point, dword)
{
	SetFocus();
	Refresh();
	SendHelpLine();
}

void MenuItem::MouseLeave()
{
    if(HasFocus() && GetParent())
        GetParent()->SetFocus();
    ClearHelpLine();
}

void MenuItem::GotFocus()
{
	Refresh();
	SendHelpLine();
}

void MenuItem::LostFocus()
{
	Refresh();
	ClearHelpLine();
}

int MenuItem::GetVisualState()
{
	return HasFocus() ?
	       (HasMouse() && GetMouseLeft() || GetMouseRight()) ? PUSH : HIGHLIGHT : NORMAL;
}

void MenuItem::SyncState()
{
	int s = GetVisualState();
	if(s != state) {
		state = s;
		Refresh();
	}
}

void MenuItem::Paint(Draw& w)
{
	int q = text.Find('\t');
	String txt, keydesc;
	if(accel)
		keydesc = GetKeyDesc(accel);
	if(q >= 0) {
		keydesc = text.Mid(q + 1);
		txt = text.Mid(0, q);
	}
	else
		txt = text;
	state = GetVisualState();
	bool hl = state != NORMAL;
	Size sz = GetSize();

	if(hl) {
		if(GUI_GlobalStyle() >= GUISTYLE_XP)
			ChPaint(w, 0, 0, sz.cx, sz.cy, style->item);
		else
			w.DrawRect(sz, SColorHighlight);
	}
	UPP::Image li = licon;
	if(li.IsEmpty()) {
		switch(type) {
		case CHECK0: li = CtrlImg::MenuCheck0(); break;
		case CHECK1: li = CtrlImg::MenuCheck1(); break;
		case RADIO0: li = CtrlImg::MenuRadio0(); break;
		case RADIO1: li = CtrlImg::MenuRadio1(); break;
		}
	}
	Size isz = li.GetSize();
	int iy = (sz.cy - isz.cy) / 2;
	bool chk = false;
	int x = max(Zx(3), (leftgap + textgap - isz.cx) / 2);
	if(!licon.IsEmpty() && type) {
		chk = type == CHECK1 || type == RADIO1;
		Rect rr = RectC(x - Zx(2), iy - Zy(2), isz.cx + Zx(4), isz.cy + Zy(4));
		if(GUI_GlobalStyle() >= GUISTYLE_XP) {
			if(chk && !hl) {
				if(IsNull(style->icheck))
					DrawXPButton(w, rr, BUTTON_EDGE|BUTTON_CHECKED);
				else
					ChPaint(w, rr, style->icheck);
			}
		}
		else {
			w.DrawRect(x - Zx(1), iy - Zy(1), isz.cx + Zx(2), isz.cy + Zy(2),
			           chk ? Blend(SColorFace, SColorLight) : SColorFace);
			DrawBorder(w, rr, chk ? ThinInsetBorder : ThinOutsetBorder);
		}
	}
	if(isenabled)
		DrawHighlightImage(w, x, iy, li, hl || chk, true);
	else
		w.DrawImage(x, iy, DisabledImage(li));
	x = max(isz.cx + Zx(3), leftgap) + textgap;
	isz = GetTextSize(text, StdFont());
	DrawMenuText(w, x, (sz.cy - isz.cy) / 2, txt, font, isenabled, hl, style->menutext,
	             style->itemtext);
	isz = ricon.GetSize();
	if(isenabled)
		w.DrawImage(sz.cx - isz.cx, (sz.cy - isz.cy) / 2, ricon, hl ? style->itemtext : style->menutext);
	else
		w.DrawImage(sz.cx - isz.cx, (sz.cy - isz.cy) / 2, DisabledImage(ricon));
	x = sz.cx - max(isz.cx, Zx(16)) - Zx(1);
	if(!IsEmpty(keydesc)) {
		isz = GetTextSize(keydesc, StdFont());
		UPP::DrawMenuText(w, x - isz.cx - Zx(2), (sz.cy - isz.cy) / 2, keydesc, font, isenabled, hl,
		                  0, SColorMenuMark(), style->itemtext, false);
	}
}

Size MenuItem::GetMinSize() const
{
	Size sz1 = GetTextSize(text, font);
	Size sz2(0, 0);
	if(accel) {
		sz2 = GetTextSize(GetKeyDesc(accel), font);
		sz2.cx += Zx(12);
	}
	Size lsz = min(maxiconsize, licon.GetSize());
	Size rsz = ricon.GetSize();
	return AddFrameSize(Size(max(lsz.cx, leftgap) + sz1.cx + max(sz2.cx, (rsz.cx ? Zx(16) : 0))
	                         + max(rsz.cx, Zx(16)) + textgap + Zx(10),
	                         max(max(lsz.cy, rsz.cy) + Zy(4), sz1.cy + Zy(6))));
}

void MenuItem::LeftUp(Point, dword)
{
	if(!isenabled) return;
#ifdef PLATFORM_WIN32
#ifdef PLATFORM_WINCE
	PlaySound(L"MenuCommand", NULL, SND_ASYNC|SND_NODEFAULT); //TODO?
#else
	PlaySound("MenuCommand", NULL, SND_ASYNC|SND_NODEFAULT);
#endif
#endif
	LLOG("Menu Item pre Action");
	Ptr<MenuItem> p = this; // action can destroy the menu and this instance too
	Action();
	if(p) {
		MenuBar *bar = GetMenuBar();
		if(bar)
			bar->action_taken = true;
	}
	LLOG("Menu Item post Action");
}

void MenuItem::RightUp(Point p, dword w)
{
	LeftUp(p, w);
}

bool MenuItem::HotKey(dword key)
{
	if(isenabled && (key == accel || CompareAccessKey(accesskey, key)
	|| key < 256 && IsAlNum((char)key) && CompareAccessKey(accesskey, ToUpper((char)key) + K_DELTA + K_ALT))) {
		LLOG("MenuItem::HotKey(" << key << ") -> SetFocus");
		SetFocus();
		Sync();
		Sleep(50);
		Action();
		return true;
	}
	return false;
}

bool MenuItem::Key(dword key, int count)
{
	if(key == K_ENTER && isenabled) {
		Action();
		return true;
	}
	return false;
}

// ----------------------------------------------------

void SubMenuBase::Pull(Ctrl *item, Point p, Size sz)
{
	menu.KillDelayedClose();
	if(!item->IsOpen() || menu.IsOpen()) return;
	menu.Clear();
#ifdef GUI_COCOA
	menu.UppMenu();
#endif
	if(parentmenu)
		menu.SetStyle(*parentmenu->style);
	proc(menu);
	if(parentmenu) {
		parentmenu->SetActiveSubmenu(&menu, item);
		menu.SetParentMenu(parentmenu);
	}
	menu.PopUp(parentmenu, p, sz);
	if(parentmenu) {
		parentmenu->doeffect = false;
		parentmenu->WhenSubMenuOpen();
	}
	menu.KillDelayedClose();
}

// ----------------------------------------------------

SubMenuItem::SubMenuItem()
{
	RightImage(CtrlImg::right_arrow());
}

void SubMenuItem::GotFocus()
{
	Refresh();
}

void SubMenuItem::Pull()
{
	Rect r = GetScreenRect();
	Point p = r.TopRight();
	p.x -= 3;
	p.y -= 2;
	SetFocus(); // avoid returning focus to window widgets by closing submenu
	SubMenuBase::Pull(this, p, Size(-r.Width(), 0));
	if(parentmenu)
		parentmenu->SyncState();
}

void SubMenuItem::MouseEnter(Point, dword)
{
	SetFocus();
	Refresh();
	if(!menu.IsOpen() && isenabled)
		SetTimeCallback(400, THISBACK(Pull), TIMEID_PULL);
}

void SubMenuItem::MouseLeave()
{
	if(HasFocus() && GetParent())
		GetParent()->SetFocus();
	KillTimeCallback(TIMEID_PULL);
}

int SubMenuItem::GetVisualState()
{
	if(menu.IsOpen() && !GetParent()->HasFocusDeep())
		return PUSH;
	return MenuItem::GetVisualState();
}

bool SubMenuItem::Key(dword key, int count)
{
	if(key == K_RIGHT || key == K_ENTER) {
		Pull();
		return true;
	}
	return MenuItem::Key(key, count);
}

bool SubMenuItem::HotKey(dword key)
{
	if(isenabled && (key == accel || CompareAccessKey(accesskey, key))) {
		Pull();
		return true;
	}
	return false;
}


// ----------------------------------------------------

Size TopSubMenuItem::GetMinSize() const
{
	Size sz = Zsz(10, 5);
	sz.cx = (sz.cx + 1) & 0xfffffffe; // We need even number, otherwise it looks asymmetric
	return AddFrameSize(GetTextSize(text, font) + sz);
}

int  TopSubMenuItem::GetState()
{
	if(parentmenu && parentmenu->GetActiveSubmenu() == &menu) return PUSH;
	if(HasMouse() && GetParent() && !GetParent()->HasFocusDeep() &&
	                 (!parentmenu || !parentmenu->GetActiveSubmenu() || parentmenu->GetActiveSubmenu() == &menu)
	   || HasFocus())
		return HIGHLIGHT;
	return NORMAL;
}

void TopSubMenuItem::Paint(Draw& w)
{
	PaintTopItem(w, state = GetState());
}

void TopSubMenuItem::Pull()
{
	Rect r = GetScreenRect();
	if(parentmenu && parentmenu->IsChild() && !parentmenu->submenu)
		parentmenu->SetupRestoreFocus();
	Point p = r.BottomLeft();
	if(GUI_GlobalStyle() >= GUISTYLE_XP)
		p += style->pullshift;
	SetFocus(); // avoid returning focus to window widgets by closing submenu
	SubMenuBase::Pull(this, p, Size(r.Width(), -r.Height()));
	if(parentmenu)
		parentmenu->SyncState();
}

void TopSubMenuItem::MouseEnter(Point p, dword)
{
	LLOG("TopSubMenuItem::MouseEnter");
	Refresh();
	if(isenabled && parentmenu->GetActiveSubmenu())
		Pull();
}

bool TopSubMenuItem::Key(dword key, int) {
	if(isenabled && (key == K_ENTER || key == K_DOWN)) {
		Pull();
		return true;
	}
	return false;
}

void TopSubMenuItem::GotFocus()
{
	LLOG("TopSubMenuItem::GotFocus");
	Refresh();
}

void TopSubMenuItem::LostFocus()
{
	LLOG("TopSubMenuItem::LostFocus");
	Refresh();
}

bool TopSubMenuItem::HotKey(dword key)
{
	if(BarCtrl::Scan(proc, key))
		return true;
	if(isenabled && (key == accel || CompareAccessKey(accesskey, key))) {
		Pull();
		return true;
	}
	return false;
}

void TopSubMenuItem::MouseLeave()
{
	Refresh();
}

void TopSubMenuItem::LeftDown(Point, dword)
{
	if(isenabled && !menu.IsOpen()) {
		Pull();
		Refresh();
	}
}

void TopSubMenuItem::SyncState()
{
	int q = GetState();
	if(q != state) {
		state = q;
		Refresh();
	}
}

// ----------------------------------------------------

int  TopMenuItem::GetState()
{
	if(!IsEnabled()) return NORMAL;
	if(HasMouse() && GetMouseLeft() || GetMouseRight()) return PUSH;
	if(HasFocus() || HasMouse()) return HIGHLIGHT;
	return NORMAL;
}

void TopMenuItem::Paint(Draw& w)
{
	PaintTopItem(w, state = GetState());
}

void TopMenuItem::MouseEnter(Point, dword)
{
	Refresh();
}

void TopMenuItem::MouseLeave()
{
	Refresh();
}

void TopMenuItem::LeftUp(Point, dword)
{
	if(!isenabled) return;
	Action();
	Refresh();
}

void TopMenuItem::LeftDown(Point, dword)
{
	Refresh();
}

void TopMenuItem::GotFocus()
{
	Refresh();
}

void TopMenuItem::LostFocus()
{
	Refresh();
}

bool TopMenuItem::Key(dword key, int count)
{
	if(isenabled && key == K_ENTER) {
		Action();
		return true;
	}
	return false;
}

Size TopMenuItem::GetMinSize() const
{
	return AddFrameSize(GetTextSize(text, StdFont()) + Zsz(10, 5));
}

int TopMenuItem::GetStdHeight(Font font)
{
	return font.Info().GetHeight() + Zy(7);
}

void TopMenuItem::SyncState()
{
	if(state != GetState()) {
		state = GetState();
		Refresh();
	}
}

}
