#include "CtrlCore.h"

namespace Upp {

#define LLOG(x)   // DLOG(x)

#define IMAGECLASS CtrlCoreImg
#define IMAGEFILE <CtrlCore/CtrlCore.iml>
#include <Draw/iml_source.h>

static bool StdDisplayErrorFn(const Value& e)
{
	GuiLock __;
	if(!e.IsError())
		return false;
	String s = GetErrorText(e);
#ifdef PLATFORM_WIN32
	MessageBox(NULL, s, GetExeTitle(), MB_OK | MB_ICONQUESTION);
#else
	fputs(String().Cat() << GetExeTitle() << ": " << s << '\n', stderr);
#endif
	return true;
}

bool (*&DisplayErrorFn())(const Value& v)
{
	static bool (*errfn)(const Value& v) = &StdDisplayErrorFn;
	return errfn;
}

int64 Ctrl::eventid;

Ctrl *Ctrl::LoopCtrl;
int   Ctrl::LoopLevel;
int64 Ctrl::EventLoopNo;
int64 Ctrl::EndSessionLoopNo;

bool Ctrl::MemoryCheck;

bool Ctrl::painting = false;

void   Ctrl::SetData(const Value&) {}
Value  Ctrl::GetData() const       { return Value(); }

void Ctrl::Paint(Draw& draw)                        {}
int  Ctrl::OverPaint() const                        { return 0; }

void Ctrl::Activate()                               {}
void Ctrl::Deactivate()                             {}
void Ctrl::DeactivateBy(Ctrl *)                     {}

void Ctrl::CancelMode()                             {}
void Ctrl::MouseEnter(Point p, dword keyflags)      {}
void Ctrl::LeftDown(Point p, dword keyflags)        {}
void Ctrl::RightDown(Point p, dword keyflags)       {}
void Ctrl::LeftRepeat(Point p, dword keyflags)      {}
void Ctrl::RightRepeat(Point p, dword keyflags)     {}
void Ctrl::MouseMove(Point p, dword keyflags)       {}
void Ctrl::LeftUp(Point, dword keyflags)            {}
void Ctrl::RightUp(Point p, dword keyflags)         {}
void Ctrl::MouseLeave()                             {}
void Ctrl::LeftDrag(Point p, dword keyflags)        {}
void Ctrl::LeftHold(Point p, dword keyflags)        {}
void Ctrl::RightDrag(Point p, dword keyflags)       {}
void Ctrl::RightHold(Point p, dword keyflags)       {}
void Ctrl::MiddleDown(Point p, dword keyflags)      {}
void Ctrl::MiddleDouble(Point p, dword keyflags)    {}
void Ctrl::MiddleTriple(Point p, dword keyflags)    {}
void Ctrl::MiddleRepeat(Point p, dword keyflags)    {}
void Ctrl::MiddleDrag(Point p, dword keyflags)      {}
void Ctrl::MiddleHold(Point p, dword keyflags)      {}
void Ctrl::MiddleUp(Point p, dword keyflags)        {}

void Ctrl::Pen(Point p, const PenInfo& pen, dword keyflags) {}

void Ctrl::Layout()                                 {}

void Ctrl::PostInput()
{
	GuiLock __;
	if(parent) parent->PostInput();
}

void Ctrl::LeftDouble(Point p, dword keyflags)
{
	LeftDown(p, keyflags);
}

void Ctrl::LeftTriple(Point p, dword keyflags)
{
	LeftDown(p, keyflags);
}

void Ctrl::RightDouble(Point p, dword keyflags)
{
	RightDown(p, keyflags);
}

void Ctrl::RightTriple(Point p, dword keyflags)
{
	RightDown(p, keyflags);
}

void Ctrl::ChildGotFocus()
{
	GuiLock __;
	if(parent) parent->ChildGotFocus();
}

void Ctrl::ChildLostFocus()
{
	GuiLock __;
	if(parent) parent->ChildLostFocus();
}

void Ctrl::ChildAdded(Ctrl *q)
{
	GuiLock __;
	if(parent) parent->ChildAdded(q);
}

void Ctrl::ChildRemoved(Ctrl *q)
{
	GuiLock __;
	if(parent) parent->ChildRemoved(q);
}

void Ctrl::ParentChange() {}

bool Ctrl::Key(dword key, int count)
{
	return false;
}

void Ctrl::GotFocus()                               {}
void Ctrl::LostFocus()                              {}

dword  Ctrl::AccessKeyBit(int accesskey)
{
	accesskey &= 255;
	if(accesskey >= 'A' && accesskey <= 'Z')
		return (uint64)2 << (accesskey - 'A');
	return !!accesskey;
}

dword Ctrl::GetAccessKeysDeep() const
{
	GuiLock __;
	dword used = GetAccessKeys();
	for(Ctrl *ctrl = GetFirstChild(); ctrl; ctrl = ctrl->GetNext())
		used |= ctrl->GetAccessKeysDeep();
	return used;
}

void Ctrl::AssignAccessKeys(dword used)
{
	GuiLock __;
	for(Ctrl *ctrl = GetFirstChild(); ctrl; ctrl = ctrl->GetNext()) {
		ctrl->AssignAccessKeys(used);
		used |= ctrl->GetAccessKeys();
	}
}

dword Ctrl::GetAccessKeys() const
{
	return 0;
}

void Ctrl::DistributeAccessKeys()
{
	AssignAccessKeys(GetAccessKeysDeep());
}

bool Ctrl::VisibleAccessKeys()
{
	GuiLock __;
	if(GUI_AltAccessKeys())
		return GetAlt() && GetTopCtrl() == GetActiveCtrl();
	return true;
}

void Ctrl::State(int) {}

void Ctrl::StateDeep(int reason)
{
	GuiLock __;
	if(destroying)
		return;
	for(Ctrl *q = GetFirstChild(); q; q = q->GetNext())
		q->StateDeep(reason);
	State(reason);
}

void Ctrl::StateH(int reason)
{
	GuiLock __;
	for(int i = 0; i < statehook().GetCount(); i++)
		if((*statehook()[i])(this, reason))
			return;
	StateDeep(reason);
}

bool   Ctrl::Accept()
{
	GuiLock __;
	if(!IsEnabled() || !IsShown())
		return true;
	if(DisplayError(GetData())) {
		SetWantFocus();
		return false;
	}
	for(Ctrl *q = GetFirstChild(); q; q = q->GetNext())
		if(!q->Accept()) return false;
	return true;
}

void   Ctrl::Reject()
{
	GuiLock __;
	for(Ctrl *q = GetFirstChild(); q; q = q->GetNext())
		q->Reject();
}

void   Ctrl::Serialize(Stream& s)
{
	GuiLock __;
	Value x;
	if(s.IsStoring())
		x = GetData();
	s % x;
	if(s.IsLoading())
		SetData(x);
	for(Ctrl *q = GetFirstChild(); q; q = q->GetNext())
		q->Serialize(s);
}

void Ctrl::Jsonize(JsonIO& jio)
{
	GuiLock __;
	Value x;
	if(jio.IsStoring())
		x = GetData();
	x.Jsonize(jio);
	if(jio.IsLoading())
		SetData(x);
}

void Ctrl::Xmlize(XmlIO& xio)
{
	GuiLock __;
	Value x;
	if(xio.IsStoring())
		x = GetData();
	x.Xmlize(xio);
	if(xio.IsLoading())
		SetData(x);
}

void Ctrl::Updated() {}

bool Ctrl::IsForeground() const
{
	GuiLock __;
	return GetTopCtrl()->IsWndForeground();
}

void Ctrl::SetForeground()
{
	GuiLock __;
	GetTopCtrl()->SetWndForeground();
}

bool Ctrl::IsOpen() const
{
	GuiLock __;
	const Ctrl *q = GetTopCtrl();
	return q->isopen && q->IsWndOpen();
}

void Ctrl::Show(bool ashow) {
	GuiLock __;
	if(visible != ashow) {
		visible = true;
		fullrefresh = false;
		RefreshFrame();
		visible = ashow;
		fullrefresh = false;
		RefreshFrame();
		if(parent)
			StateH(SHOW);
		if(top)
			WndShow(visible);
		if(InFrame() && parent)
			RefreshParentLayout();
	}
}

bool Ctrl::IsVisible() const {
	GuiLock __;
	const Ctrl *q = this;
	for(;;) {
		if(!q->visible) return false;
		if(!q->parent) break;
		q = q->parent;
	}
	return q->visible;
}

void Ctrl::Enable(bool aenable) {
	GuiLock __;
	if(enabled != aenable) {
		enabled = aenable;
		if(top) WndEnable(enabled);
		if(!enabled && parent && HasFocusDeep())
			IterateFocusForward(this, GetTopCtrl());
		RefreshFrame();
		StateH(ENABLE);
		SyncCaret();
	}
}

bool Ctrl::IsShowEnabled() const {
	GuiLock __;
	return IsEnabled() && (!parent || parent->IsShowEnabled());
}

Ctrl& Ctrl::SetEditable(bool aeditable) {
	GuiLock __;
	if(editable != aeditable) {
		editable = aeditable;
		RefreshFrame();
		StateH(EDITABLE);
	}
	return *this;
}

void Ctrl::SetModify()
{
	GuiLock __;
	modify = true;
}

void Ctrl::ClearModify()
{
	GuiLock __;
	modify = false;
}

void Ctrl::ClearModifyDeep()
{
	GuiLock __;
	ClearModify();
	for(Ctrl *q = firstchild; q; q = q->next)
		q->ClearModifyDeep();
}


bool Ctrl::IsModified() const
{
	GuiLock __;
	return modify;
}

bool Ctrl::IsModifiedDeep() const
{
	GuiLock __;
	if(IsModified()) return true;
	for(Ctrl *q = firstchild; q; q = q->next)
		if(q->IsModified()) return true;
	return false;
}

void Ctrl::SetCaret(const Rect& r)
{
	SetCaret(r.left, r.top, r.GetWidth(), r.GetHeight());
}

Rect Ctrl::GetCaret() const
{
	return RectC(caretx, carety, caretcx, caretcy);
}

void Ctrl::KillCaret()
{
	SetCaret(0, 0, 0, 0);
}

void Ctrl::SetInfoPart(int i, const char *txt)
{
	Vector<String> f = Split(info, '\x7f', false);
	f.At(i) = txt;
	info = Join(f, "\x7f");
}

Ctrl& Ctrl::Tip(const char *txt)
{
	SetInfoPart(0, txt);
	return *this;
}

Ctrl& Ctrl::HelpLine(const char *txt)
{
	SetInfoPart(1, txt);
	return *this;
}

Ctrl& Ctrl::Description(const char *txt)
{
	SetInfoPart(2, txt);
	return *this;
}

Ctrl& Ctrl::HelpTopic(const char *txt)
{
	SetInfoPart(3, txt);
	return *this;
}

Ctrl& Ctrl::LayoutId(const char *txt)
{
	SetInfoPart(4, txt);
	return *this;
}

String Ctrl::GetInfoPart(int i) const
{
	Vector<String> f = Split(info, '\x7f', false);
	return i < f.GetCount() ? f[i] : String();
}

String Ctrl::GetTip() const
{
	return GetInfoPart(0);
}

String Ctrl::GetHelpLine() const
{
	return GetInfoPart(1);
}

String Ctrl::GetDescription() const
{
	return GetInfoPart(2);
}

String Ctrl::GetHelpTopic() const
{
	return GetInfoPart(3);
}

String Ctrl::GetLayoutId() const
{
	return GetInfoPart(4);
}

bool  Ctrl::SetWantFocus() {
	GuiLock __;
	if(IsWantFocus() && IsEnabled() && IsVisible() && IsOpen())
		return SetFocus();
	return false;
}

void Ctrl::UpdateRefresh() {
	Update();
	Refresh();
}

void Ctrl::Update() {
	SetModify();
	Updated();
}

void Ctrl::Action()
{
	Event<> h = WhenAction; // we make copy of action just in case widget is destroyed during the call
	h();
}

void Ctrl::UpdateAction() {
	Update();
	Action();
}

void Ctrl::UpdateActionRefresh() {
	Update();
	Action();
	Refresh();
};

void  Ctrl::CancelModeDeep() {
	GuiLock __;
	CancelMode();
	for(Ctrl *q = firstchild; q; q = q->next)
		q->CancelModeDeep();
}

String Ctrl::GetDesc() const
{
	return "";
}


String Name(const Ctrl *ctrl)
{
	return ctrl ? ctrl->Name() : "NULL";
}

String Desc(const Ctrl *ctrl)
{
	if(!ctrl)
		return "NULL";
	String s;
	s << typeid(*ctrl).name();
	String q = ctrl->GetDesc();
	if(IsNull(q)) {
		if(ctrl->GetPrev()) {
			q = ctrl->GetPrev()->GetDesc();
			if(!IsNull(q))
			  s << " <<\"" << q << "\">>";
		}
	}
	else
	  s << " \"" << q << '\"';
	const Ctrl *top = ctrl->GetTopWindow();
	if(top && top != ctrl) {
 		String q = top->GetDesc();
 		if(IsNull(q))
 			s << " (" << typeid(*top).name() << ")";
 		else
	 		s << " (\"" << q << "\")";
 	}
	return s;
}

#ifdef _DEBUG

#define sFLAG(x)  (x ? #x" " : "")
#define LG(x)     s << x << '\n'

void Ctrl::Dump(Stream& s) const {
	GuiLock __;
	LG(Name());
	LG(sFLAG(backpaint) << sFLAG(inframe) << sFLAG(visible) << sFLAG(enabled) <<
	   sFLAG(wantfocus) << sFLAG(editable) << sFLAG(IsModified()) << sFLAG(transparent));
	LG("Rect:   " << GetRect());
	LG("View:   " << GetView());
	for(int i = 0; i < frame.GetCount(); i++)
		LG("Frame " << i << ": " << typeid(decltype(*frame[i].frame)).name() << " - " << frame[i].view);
	LG("Data: " << GetData().ToString());
	if(firstchild) {
		LG("Children");
		s << LOG_BEGIN;
		for(Ctrl *q = GetFirstChild(); q; q = q->GetNext()) {
			q->Dump(s);
			LG("------");
		}
		s << LOG_END;
	}
	else
		LG("No child");
}

void Ctrl::Dump() const {
	Dump(VppLog());
}

void   Dump(const Ctrl *ctrl)
{
	if(ctrl)
		ctrl->Dump();
	else
		LOG("NULL");
}

#endif

bool Ctrl::IsOcxChild()
{
	return false;
}

Ctrl::Ctrl() {
	ONCELOCK {
		InstallPanicBox();
	}
	ASSERT_(IsMainRunning(), "GUI widgets cannot be global variables");
	ASSERT_(ThreadHasGuiLock(), "GUI widgets cannot be initialized in non-main thread without GuiLock");
	GuiLock __; // Beware: Even if we have ThreadHasGuiLock ASSERT, we still can be the main thread!
	LLOG("Ctrl::Ctrl");
	GuiPlatformConstruct();
	destroying = false;
	parent = prev = next = firstchild = lastchild = NULL;
	top = NULL;
	exitcode = 0;
	frame.Add().frame = &NullFrame();
	enabled = visible = wantfocus = initfocus = true;
	editable = true;
	backpaint = IsCompositedGui() ? FULLBACKPAINT : TRANSPARENTBACKPAINT;
	inframe = false;
	ignoremouse = transparent = false;
	caretcx = caretcy = caretx = carety = 0;
	pos.x = PosLeft(0, 0);
	pos.y = PosTop(0, 0);
	rect = Rect(0, 0, 0, 0);
	inloop = popup = isopen = false;
	modify = false;
	unicode = false;
	popupgrab = false;
	fullrefresh = false;
	akv = false;
}

void KillTimeCallbacks(void *id, void *idlim);

void Ctrl::DoRemove() {
	GuiLock __;
	if(!IsOpen()) return;
	ReleaseCapture();
	if(HasChildDeep(captureCtrl))
		captureCtrl->ReleaseCapture();
	CancelModeDeep();
	if(HasChildDeep(mouseCtrl) || mouseCtrl == this)
		mouseCtrl = NULL;
	LLOG("DoRemove " << Name() << " focusCtrl: " << UPP::Name(focusCtrl));
	GuiPlatformRemove();
	if(HasFocusDeep()) {
		LLOG("DoRemove - HasFocusDeep");
		if(destroying) {
			if(parent) {
				LLOG("parent - deferred SetFocus / ChildLostFocus; parent = " << UPP::Name(parent));
				defferedSetFocus = parent;
				defferedChildLostFocus.Add(parent);
			}
			else
				if(IsPopUp()) {
					LLOG("Remove Popup");
					Ctrl *owner = GetOwner();
					if(owner && owner->IsEnabled())
						owner->ActivateWnd();
				}
			NoWantFocus();
		}
		else {
			Ptr<Ctrl> fc = focusCtrl;
			focusCtrl = NULL;
			DoKillFocus(fc, NULL);
			if(parent) {
				LLOG("DoRemove -> SetFocus(" << UPP::Name(parent) << "), focusCtrl = " << UPP::Name(focusCtrl) << ", fc = " << UPP::Name(fc));
				bool b = IsWantFocus();
				NoWantFocus();
				parent->SetFocus0(false);
				WantFocus(b);
			}
			else
				if(IsPopUp()) {
					LLOG("Remove Popup");
					Ctrl *owner = GetOwner();
					if(owner && owner->IsEnabled()) {
						LLOG("Remove popup -> owner->ActivateWnd");
						owner->ActivateWnd();
					}
				}
		}
		SyncCaret();
	}
	LLOG("//DoRemove " << Name() << " focusCtrl: " << UPP::Name(focusCtrl));
}

void Ctrl::Close()
{
	GuiLock __;
	Ctrl *q = GetTopCtrl();
	if(!q->top) return;
	DoRemove();
	if(parent) return;
	StateH(CLOSE);
	USRLOG("   CLOSE " + Desc(this));
	WndDestroy();
	visible = true;
	popup = false;
}

Ctrl::~Ctrl() {
	GuiLock __;
	LLOG("Ctrl::~Ctrl");
	destroying = true;
	while(GetFirstChild())
		RemoveChild(GetFirstChild());
	if(parent)
		parent->RemoveChild(this);
	Close();
	KillTimeCallbacks(this, (byte *) this + sizeof(Ctrl));
}

Vector<Ctrl::MouseHook>& Ctrl::mousehook() { static Vector<Ctrl::MouseHook> h; return h; }
Vector<Ctrl::KeyHook>&   Ctrl::keyhook() { static Vector<Ctrl::KeyHook> h; return h; }
Vector<Ctrl::StateHook>& Ctrl::statehook() { static Vector<Ctrl::StateHook> h; return h; }

void Ctrl::InstallMouseHook(MouseHook hook)
{
	GuiLock __;
	mousehook().Add(hook);
}

void Ctrl::DeinstallMouseHook(MouseHook hook)
{
	GuiLock __;
	int q = FindIndex(mousehook(), hook);
	if(q >= 0) mousehook().Remove(q);
}

void Ctrl::InstallKeyHook(KeyHook hook)
{
	GuiLock __;
	keyhook().Add(hook);
}

void Ctrl::DeinstallKeyHook(KeyHook hook)
{
	GuiLock __;
	int q = FindIndex(keyhook(), hook);
	if(q >= 0) keyhook().Remove(q);
}

void Ctrl::InstallStateHook(StateHook hook)
{
	GuiLock __;
	statehook().Add(hook);
}

void Ctrl::DeinstallStateHook(StateHook hook)
{
	GuiLock __;
	int q = FindIndex(statehook(), hook);
	if(q >= 0) statehook().Remove(q);
}

static char sZoomText[] = "OK Cancel Exit Retry";

const char *Ctrl::GetZoomText()
{
	GuiLock __;
	return sZoomText;
}

Size Ctrl::Bsize;
Size Ctrl::Dsize;
Size Ctrl::Csize;
bool Ctrl::IsNoLayoutZoom;

/*
void InitRichTextZoom()
{
	SetRichTextStdScreenZoom(96 * GetTextSize(sZoomText, StdFont()).cy / 13, 600);
	Ctrl::ReSkin();
}
*/
void InitRichTextZoom()
{
	Size h = 96 * Ctrl::Bsize / Ctrl::Dsize;
	SetRichTextStdScreenZoom(min(h.cx, h.cy), 600);
	Ctrl::ReSkin();
}


void Ctrl::Csizeinit()
{
	GuiLock __;
	if(Csize.cx == 0 || Dsize.cx == 0) {
		bool bigger = GetStdFontCy() > 20;
		if(Csize.cx == 0) {
			Csize = GetTextSize(sZoomText, StdFont());
			Csize.cy += 4 * bigger; // this is intended to compensate for editfield / droplist edges - in any case looks better on UHD
		}
		Bsize = Csize;
		if(Dsize.cx == 0)
			Dsize = Size(99, 13 + 4 * bigger);
		Csize.cx = max(Csize.cx, Dsize.cx);
		Csize.cy = max(Csize.cy, Dsize.cy);
		InitRichTextZoom();
	}
}

void Ctrl::SetZoomSize(Size sz, Size bsz)
{
	GuiLock __;
	Csize = sz;
	Dsize = bsz;
	IsNoLayoutZoom = false;
	ReSkin();
}

void Ctrl::NoLayoutZoom()
{
	GuiLock __;
	IsNoLayoutZoom = true;
	Csize = Dsize = Size(1, 1);
	ReSkin();
}

void Ctrl::GetZoomRatio(Size& m, Size& d)
{
	GuiLock __;
	m = Csize;
	d = Dsize;
}

int  Ctrl::HorzLayoutZoom(int cx)
{
	Csizeinit();
	return cx > -16000 ? Csize.cx * cx / Dsize.cx : cx;
}

double  Ctrl::HorzLayoutZoomf(double cx)
{
	Csizeinit();
	return cx > -16000 ? Csize.cx * cx / Dsize.cx : cx;
}

int  Ctrl::VertLayoutZoom(int cy)
{
	Csizeinit();
	return cy > -16000 ? Csize.cy * cy / Dsize.cy : cy;
}

Size Ctrl::LayoutZoom(int cx, int cy)
{
	return Size(HorzLayoutZoom(cx), VertLayoutZoom(cy));
}

Size Ctrl::LayoutZoom(Size sz)
{
	return LayoutZoom(sz.cx, sz.cy);
}

Font FontZ(int face, int height)
{
	return Font(face, Ctrl::VertLayoutZoom(height));
}

bool ApplicationUHDEnabled = true;

void Ctrl::SetUHDEnabled(bool set)
{
	ApplicationUHDEnabled = set;
	ReSkin();
}

bool Ctrl::IsUHDEnabled()
{
	if(GetEnv("UPP_DISABLE_UHD__") == "1")
		return false;
	return ApplicationUHDEnabled;
}

bool ApplicationDarkThemeEnabled = true;

void Ctrl::SetDarkThemeEnabled(bool set)
{
	ApplicationDarkThemeEnabled = set;
	ReSkin();
}

bool Ctrl::IsDarkThemeEnabled()
{
	return ApplicationDarkThemeEnabled;
}

Font StdFontZ(int height)   { return FontZ(Font::STDFONT, height); }
Font SansSerifZ(int height) { return FontZ(Font::SANSSERIF, height); }
Font SerifZ(int height)     { return FontZ(Font::SERIF, height); }
Font MonospaceZ(int height) { return FontZ(Font::MONOSPACE, height); }

Font ScreenSansZ(int height) { return FontZ(Font::SCREEN_SANS, height); }
Font ScreenSerifZ(int height) { return FontZ(Font::SCREEN_SERIF, height); }
Font ScreenFixedZ(int height) { return FontZ(Font::SCREEN_FIXED, height); }
Font RomanZ(int height) { return FontZ(Font::ROMAN, height); }
Font ArialZ(int height) { return FontZ(Font::ARIAL, height); }
Font CourierZ(int height) { return FontZ(Font::COURIER, height); }

String Ctrl::appname;

void Ctrl::SetAppName(const String& nm)
{
	GuiLock __;
	appname = nm;
}

String Ctrl::GetAppName()
{
	GuiLock __;
	if(appname.IsEmpty())
		appname = GetExeTitle();
	return appname;
}

static bool _ClickFocus;
bool Ctrl::ClickFocus() { return _ClickFocus; }
void Ctrl::ClickFocus(bool cf) { _ClickFocus = cf; }


Vector<Ctrl *> Ctrl::GetTopWindows()
{
	GuiLock __;
	Vector<Ctrl *> c = GetTopCtrls();
	Vector<Ctrl *> r;
	for(int i = 0; i < c.GetCount(); i++)
		if(!c[i]->IsPopUp())
			r.Add(c[i]);
	return r;
}

void Ctrl::CloseTopCtrls()
{
	GuiLock __;
	Vector<Ctrl *> tc = Ctrl::GetTopCtrls();
	for(int i = 0; i < tc.GetCount(); i++)
		tc[i]->Close();
}

bool xpstyle;

bool IsOrOwnedBy(Ctrl *q, Ctrl *window)
{
	while(q) {
		if(q == window)
			return true;
		q = q->GetOwner();
	}
	return false;
}

Vector< Ptr<Ctrl> > DisableCtrls(const Vector<Ctrl *>& ctrl, Ctrl *exclude)
{
	Vector< Ptr<Ctrl> > disabled;
	for(int i = 0; i < ctrl.GetCount(); i++) {
		Ctrl *q = ctrl[i];
		if(q && q->IsEnabled() && !IsOrOwnedBy(q, exclude)) {
			q->Disable();
			disabled.Add(q);
		}
	}
	return disabled;
}

void EnableCtrls(const Vector< Ptr<Ctrl> >& ctrl)
{
	for(int i = ctrl.GetCount(); --i >= 0;) {
		Ctrl *q = ctrl[i];
		if(q) q->Enable();
	}
}

void Modality::Begin(Ctrl *modal, bool fo)
{
	active = Ctrl::GetFocusCtrl();
	fore_only = fo;
	LLOG("Active " << Name(active));
	enable = DisableCtrls(Ctrl::GetTopWindows(), modal);
}

void Modality::End()
{
	EnableCtrls(enable);
	if(active && (!fore_only || active->IsForeground()))
		active->SetFocus();
	enable.Clear();
	active = NULL;
}

extern void (*whenSetStdFont)();

INITBLOCK {
	whenSetStdFont = &Ctrl::ReSkin;
}

void (*Ctrl::skin)();

void CtrlSetDefaultSkin(void (*_skin)())
{
	Ctrl::skin = _skin;
}

void Ctrl::SetSkin(void (*_skin)())
{
	GuiLock __;
	skin = _skin;
	ReSkin();
}

void Ctrl::ReSkin()
{
	GuiLock __;
	static int lock;
	if(lock)
		return;
	lock++;
	ChReset();
	Iml::ResetAll();
	Csize.cx = Dsize.cx = IsNoLayoutZoom;
	if(skin)
		(*skin)();
	Csize.cx = Dsize.cx = IsNoLayoutZoom;
	Csizeinit();
	ChFinish();
	Vector<Ctrl *> ctrl = GetTopCtrls();
	for(int i = 0; i < ctrl.GetCount(); i++) {
		ctrl[i]->RefreshLayoutDeep();
		ctrl[i]->RefreshFrame();
	}
	lock--;
}

CH_INT(GUI_GlobalStyle, GUISTYLE_CLASSIC);
CH_INT(GUI_DragFullWindow, 1);
CH_INT(GUI_PopUpEffect, GUIEFFECT_SLIDE);
CH_INT(GUI_ToolTips, 1);
CH_INT(GUI_ToolTipDelay, 1000);
CH_INT(GUI_DropShadows, 1);
CH_INT(GUI_AltAccessKeys, 1);
CH_INT(GUI_AKD_Conservative, 0);
CH_INT(GUI_DragDistance, 4);
CH_INT(GUI_DblClickTime, 500);
CH_INT(GUI_WheelScrollLines, 3);

String Ctrl::Name0() const {
	GuiLock __;
	String s = CppDemangle(typeid(*this).name());
	const Ctrl *q = this;
	String path;
	while(q) {
		String id = q->GetLayoutId();
		if(id.GetCount())
			path = '.' + q->GetLayoutId() + path;
		q = q->parent;
	}
	s << ' ' << path;
#ifdef CPU_64
	s << " : 0x" + FormatIntHex(this);
#else
	s << " : " + Format("0x%x", (int) this);
#endif
	if(IsChild())
		s << " (parent " << CppDemangle(typeid(*parent).name()) << ")";
	return s;
}

String Ctrl::Name(Ctrl *ctrl)
{
	return Upp::Name(ctrl);
}

void   Ctrl::EndLoop()
{
	GuiLock __;
	inloop = false;
	SysEndLoop();
}

void   Ctrl::EndLoop(int code)
{
	GuiLock __;
	ASSERT(!parent);
	exitcode = code;
	EndLoop();
}

bool   Ctrl::InLoop() const
{
	GuiLock __;
	return inloop;
}

bool   Ctrl::InCurrentLoop() const
{
	GuiLock __;
	return GetLoopCtrl() == this;
}

int    Ctrl::GetExitCode() const
{
	GuiLock __;
	return exitcode;
}

#ifdef HAS_TopFrameDraw

ViewDraw::ViewDraw(Ctrl *ctrl, const Rect& r)
:	TopFrameDraw(ctrl, (ctrl->GetScreenView() & (r + ctrl->GetScreenView().TopLeft()))
                       - ctrl->GetTopCtrl()->GetScreenRect().TopLeft())
{
	Point p = r.TopLeft();
	Offset(min(p.x, 0), min(p.y, 0));
}

#endif

}
