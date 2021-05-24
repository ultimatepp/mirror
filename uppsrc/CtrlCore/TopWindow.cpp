#include "CtrlCore.h"

namespace Upp {

#define LLOG(x)    // DLOG(x)

String TopWindow::GetDesc() const
{
	return title.ToString();
}

Size TopWindow::GetMinSize() const
{
	return minsize;
}

Size TopWindow::GetStdSize() const
{
	return GetMinSize();
}

void TopWindow::ActiveFocus0(Ctrl& ctrl)
{
	if(IsChild()) return;
	activefocus = &ctrl;
	LLOG("ActiveFocus0");
	if(IsForeground())
		ctrl.SetWantFocus();
}

void TopWindow::Activate()
{
	LLOG("Activate " << Name() << " activefocus = " << UPP::Name(activefocus));
	USRLOG("   ACTIVATE " + Desc(this));
	if(activefocus && (HasFocus() || !GetFocusChildDeep()) && IsEnabled()) {
		LLOG("activefocus->SetWantFocus()");
		activefocus->SetWantFocus();
	}
	if(urgent)
		SyncCaption();
	LLOG("Activate End");
}

void TopWindow::ChildGotFocus()
{
	activefocus = GetFocusCtrl();
}

void TopWindow::Deactivate()
{
	LLOG("DeActivate current focus " << UPP::Name(GetFocusCtrl()));
	if(HasFocusDeep())
		activefocus = GetFocusCtrl();
	USRLOG("   DEACTIVATE " + Desc(this));
	LLOG("DeActivate " << Name() << " activefocus = " << UPP::Name(activefocus));
}

void TopWindow::PlaceFocus()
{
	LLOG("PlaceFocus " << Upp::Name(this));
	if(activefocus)
		activefocus->SetFocus();
	else
		IterateFocusForward(this, this, true, true);
}

bool TopWindow::IsShowEnabled() const
{
	return true;
}

void TopWindow::DefaultBreak()
{
	if(FindAction(IDCANCEL) || close_rejects)
		RejectBreak(IDCANCEL);
	else
	if(FindAction(IDNO))
		RejectBreak(IDNO);
	else
	if(FindAction(IDEXIT))
		AcceptBreak(IDEXIT);
	else
	if(FindAction(IDYES))
		AcceptBreak(IDYES);
	else
		AcceptBreak(IDOK);
}

void TopWindow::Close()
{
	if(InLoop()) {
		if(!InCurrentLoop()) return;
		DefaultBreak();
		return;
	}
	if(IsOpen()) IgnoreMouseUp();
	Ctrl::Close();
}

void TopWindow::Backup()
{
	StringStream s;
	Ctrl::Serialize(s);
	backup = s;
}

void TopWindow::Restore()
{
	StringStream s(backup);
	Ctrl::Serialize(s);
}

bool TopWindow::Accept()
{
	Ctrl *q;
	for(q = GetFirstChild(); q; q = q->GetNext())
		if(!q->Accept())
			return false;
	return true;
}

void TopWindow::Reject()
{
	for(Ctrl *q = GetFirstChild(); q; q = q->GetNext())
		q->Reject();
	if(!backup.IsEmpty())
		Restore();
}

void TopWindow::Break(int ID)
{
	EndLoop(ID);
}

bool TopWindow::AcceptBreak(int ID)
{
	if(Accept()) {
		Break(ID);
		return true;
	}
	return false;
}

void TopWindow::RejectBreak(int ID)
{
	Reject();
	Break(ID);
}

void TopWindow::SetupRect(Ctrl *owner)
{
	Rect r = GetRect();
	if(r.IsEmpty())
	   SetRect(GetDefaultWindowRect());
	else {
		r.SetSize(max(r.GetSize(), GetMinSize()));
		SetRect(r);
		if(r.left == 0 && r.top == 0 && center == 1) {
			Rect area = owner ? owner->GetWorkArea() : Ctrl::GetWorkArea();
			SetRect(area.CenterRect(min(area.Size(), r.Size())));
		}
	}
}

void TopWindow::FixIcons()
{
	TopWindow *q = GetMainWindow();
	if(q) {
		if(IsNull(icon)) {
			icon = q->GetIcon();
			SyncCaption();
		}
		if(IsNull(largeicon)) {
			largeicon = q->GetIcon();
			SyncCaption();
		}
	}
}

TopWindow::Abreak *TopWindow::FindAction(int ID)
{
	for(int i = 0; i < action.GetCount(); i++)
		if(action[i].ID == ID) return &action[i];
	return NULL;
}

TopWindow::Abreak *TopWindow::FindAddAction(int ID)
{
	Abreak *x = FindAction(ID);
	if(x) return x;
	Abreak& a = action.Add();
	a.ID = ID;
	a.dlg = this;
	return &a;
}

Callback TopWindow::Breaker(int ID)
{
	return callback(FindAddAction(ID), &Abreak::Break);
}

Callback TopWindow::Acceptor(int ID)
{
	return callback(FindAddAction(ID), &Abreak::Accept);
}

Callback TopWindow::Rejector(int ID)
{
	return callback(FindAddAction(ID), &Abreak::Reject);
}

TopWindow& TopWindow::Breaker(Ctrl& m, int ID)
{
	m.WhenAction = Breaker(ID);
	return *this;
}

TopWindow& TopWindow::Acceptor(Ctrl& m, int ID)
{
	m.WhenAction = Acceptor(ID);
	return *this;
}

TopWindow& TopWindow::Rejector(Ctrl& m, int ID)
{
	m.WhenAction = Rejector(ID);
	return *this;
}

void TopWindow::Paint(Draw& w)
{
	if(!IsNull(st->background))
		ChPaint(w, GetSize(), st->background);
	else
		background.Paint(w, GetSize(), SColorText, SColorShadow);
}

TopWindow& TopWindow::Background(const PaintRect& prect)
{
	background = prect;
	Refresh();
	return *this;
}

bool TopWindow::Key(dword key, int count)
{
	if(Ctrl::Key(key, count))
		return true;
	if(IsChild()) return false;
	if(key == K_DOWN || key == K_RIGHT || key == K_TAB) {
		Ctrl *ctrl = GetFocusChildDeep();
		if(ctrl && IterateFocusForward(ctrl, this))
			return true;
		ctrl = GetFirstChild();
		if(ctrl) {
			if(ctrl->SetWantFocus()) return true;
			return IterateFocusForward(ctrl, this);
		}
	}
	if(key == K_UP || key == K_LEFT || key == K_SHIFT_TAB) {
		Ctrl *ctrl = GetFocusChildDeep();
		if(ctrl && IterateFocusBackward(ctrl, this))
			return true;
		ctrl = GetLastChild();
		if(ctrl) {
			if(ctrl->SetWantFocus()) return true;
			return IterateFocusBackward(ctrl, this);
		}
	}
	return false;
}

void TopWindow::WorkAreaTrim()
{
	Rect a = GetWorkArea();
	Rect h = GetRect();
	h.left = max(h.left, a.left);
	h.right = min(h.right, a.right);
	h.top = max(h.top, a.top);
	h.bottom = min(h.bottom, a.bottom);
	if(h != GetRect() && !IsChild())
		SetRect(h);
}


void GatherWindowTree(Ctrl *w, const Vector<Ctrl *>& ws, Vector<Ctrl *>& es)
{
	if(!w->InLoop())
		es.Add(w);
	for(int i = 0; i < ws.GetCount(); i++)
		if(ws[i]->GetOwner() == w)
			GatherWindowTree(ws[i], ws, es);
}

int  TopWindow::Run(bool appmodal)
{
	GuiLock __;
	LLOG("TopWindow::Run() <- " << typeid(*this).name());
	LLOG("Focus = " << UPP::Name(GetFocusCtrl()));
	if(!IsOpen())
		Open();
	if(!IsVisible()) Show();
	bool pinloop = inloop;
	int  pexitcode = exitcode;
	exitcode = Null;
	Vector<Ctrl *> es;
	if(appmodal)
		es = GetTopCtrls();
	else {
		Vector<Ctrl *> ws = GetTopCtrls();
		for(int i = 0; i < ws.GetCount(); i++)
			if(ws[i]->InLoop())
				es.Add(ws[i]);
		Ctrl *mw = GetMainWindow();
		if(mw) GatherWindowTree(mw, ws, es);
	}
	Vector< Ptr<Ctrl> > disabled = DisableCtrls(es, this);
#ifdef _DEBUG
	for(int d = 0; d < disabled.GetCount(); d++)
		LLOG("DisableCtrls[" << d << "] = " << UPP::Name(disabled[d]));
	LLOG("Running EventLoop in " << UPP::Name(this));
#endif
	EventLoop(this);
#ifdef _DEBUG
	LLOG("Finished EventLoop in " << UPP::Name(this));
	for(int e = 0; e < disabled.GetCount(); e++)
		LLOG("EnableCtrls[" << e << "] = " << UPP::Name(disabled[e]));
#endif
	EnableCtrls(disabled);
	if(IsNull(exitcode)) {
		WhenClose();
		if(IsNull(exitcode))
			DefaultBreak();
	}
	int q = exitcode;
	inloop = pinloop;
	exitcode = pexitcode;
	LLOG("TopWindow::Run() = " << q << " -> " << typeid(*this).name());
#ifdef GUI_WIN
	LLOG("Focus = " << UPP::Name(GetFocusCtrl()) << ", raw " << (void *)::GetFocus());
#endif
	return q;
}

int  TopWindow::Execute()
{
	int m = Run();
	Close();
	return m;
}

TopWindow& TopWindow::Title(const WString& _title)
{
	if(title != _title) {
		title = _title;
		SyncTitle();
	}
	return *this;
}

TopWindow& TopWindow::Title(const char *s)
{
	return Title(WString(s));
}

void TopWindow::SetMinSize(Size sz)
{
	minsize = sz;
	SyncSizeHints();
}

TopWindow& TopWindow::Sizeable(bool b)
{
	sizeable = b;
	SyncCaption();
	SyncSizeHints();
	return *this;
}

TopWindow& TopWindow::MinimizeBox(bool b)
{
	minimizebox = b;
	SyncCaption();
	SyncSizeHints();
	return *this;
}

TopWindow& TopWindow::MaximizeBox(bool b)
{
	maximizebox = b;
	SyncCaption();
	SyncSizeHints();
	return *this;
}

TopWindow& TopWindow::Icon(const Image& m)
{
	if(!icon.IsSame(m)) {
		icon = m;
		SyncCaption();
	}
	return *this;
}

TopWindow& TopWindow::LargeIcon(const Image& m)
{
	if(!largeicon.IsSame(m)) {
		largeicon = m;
		SyncCaption();
	}
	return *this;
}

TopWindow& TopWindow::Icon(const Image& smallicon, const Image& _largeicon)
{
	if(!icon.IsSame(smallicon) || !largeicon.IsSame(_largeicon)) {
		icon = smallicon;
		largeicon = _largeicon;
		SyncCaption();
	}
	return *this;
}

TopWindow& TopWindow::ToolWindow(bool b)
{
	tool = b;
	SyncCaption();
	return *this;
}

TopWindow& TopWindow::Urgent(bool b)
{
	GuiLock __;
	urgent = b;
	SyncCaption();
	return *this;
}

TopWindow& TopWindow::FrameLess(bool b)
{
	GuiLock __;
	frameless = b;
	SyncCaption();
	return *this;
}

void TopWindow::ShutdownWindow()
{
}

void TopWindow::ShutdownWindows()
{
	bool again = true;
	while(again) {
		Vector<Ctrl *> tc = GetTopCtrls();
		again = false;
		for(int i = 0; i < tc.GetCount(); i++) {
			Ptr<TopWindow> w = dynamic_cast<TopWindow *>(tc[i]);
			if(w && w->IsOpen() && w->IsEnabled()) {
				again = true;
				w->SetForeground();
				w->ShutdownWindow();
				if(w && w->IsOpen())
					w->WhenClose();
				if(!w || !w->IsOpen())
					break;
			}
		}
	}
}

struct DialogBackground : public Display {
	void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
	{
		w.DrawRect(r, SColorFace());
	}
};


CH_STYLE(TopWindow, TopStyle, StyleDefault)
{
	background = Null;
}

TopWindow& TopWindow::SetStyle(const TopWindow::TopStyle& s)
{
	st = &s;
	RefreshLayout();
	RefreshFrame();
	return *this;
}

TopWindow::TopWindow()
{
	GuiLock __;
	GuiPlatformConstruct();
	TransparentBackPaint();
	background = PaintRect(Single<DialogBackground>(), Null);
	SetStyle(StyleDefault());
	center = 1;
	minsize = Size(80, 20);
	maximizebox = minimizebox = sizeable = tool = noclosebox = false;
	state = OVERLAPPED;
	WhenClose = THISBACK(Close);
	overlapped.Clear();
	dokeys = true;
	fullscreen = frameless = urgent = false;
	close_rejects = false;
}

TopWindow::~TopWindow()
{
	GuiLock __;
	destroying = true;
	if(InLoop())
		EndLoop(IDOK);
	if(!IsChild())
		Close();
	GuiPlatformDestruct();
}

void Maxisize(TopWindow& win, int screencxmax)
{
	if(win.GetWorkArea().Width() <= screencxmax)
		win.Maximize();
}

CH_INT(SwapOKCancel, 0);

void ArrangeOKCancel(Ctrl& ok, Ctrl& cancel)
{
	if(SwapOKCancel() &&
	   ok.GetPos().x.GetB() == cancel.GetPos().x.GetB() &&
	   ok.GetPos().y.GetB() == cancel.GetPos().y.GetB()) {
	       Ctrl::LogPos h = ok.GetPos();
	       ok.SetPos(cancel.GetPos());
	       cancel.SetPos(h);
	}
}

void SetLayout_Size(Ctrl& ctrl, Size sz)
{
	sz = ctrl.AddFrameSize(sz);
	ctrl.SetMinSize(sz);
}

void SetLayout_Size(Ctrl& ctrl, int cx, int cy)
{
	SetLayout_Size(ctrl, Size(cx, cy));
}

}
