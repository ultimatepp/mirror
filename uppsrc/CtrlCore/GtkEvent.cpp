#include <CtrlCore/CtrlCore.h>

#ifdef GUI_GTK

namespace Upp {

#define LLOG(x)    // DLOG(x)
// #define LOG_EVENTS  _DBG_

BiVector<Ctrl::GEvent> Ctrl::Events;

Point         Ctrl::CurrentMousePos;
guint         Ctrl::CurrentState;
guint32       Ctrl::CurrentTime;
Ctrl::GEvent  Ctrl::CurrentEvent;
guint         Ctrl::MouseState;

bool  GetShift() { return Ctrl::CurrentState & GDK_SHIFT_MASK; }
bool  GetCtrl() { return Ctrl::CurrentState & GDK_CONTROL_MASK; }
bool  GetAlt() { return Ctrl::CurrentState & GDK_MOD1_MASK; }
bool  GetCapsLock() { return Ctrl::CurrentState & GDK_LOCK_MASK; }
bool  GetMouseLeft() { return Ctrl::CurrentState & GDK_BUTTON1_MASK; }
bool  GetMouseRight() { return Ctrl::CurrentState & GDK_BUTTON3_MASK; }
bool  GetMouseMiddle() { return Ctrl::CurrentState & GDK_BUTTON2_MASK; }
Point GetMousePos() { return Ctrl::CurrentMousePos; }

#ifdef LOG_EVENTS
Tuple2<int, const char *> xEvent[] = {
	{ GDK_NOTHING, "GDK_NOTHING" },
	{ GDK_DELETE, "GDK_DELETE" },
	{ GDK_DESTROY, "GDK_DESTROY" },
	{ GDK_EXPOSE, "GDK_EXPOSE" },
	{ GDK_MOTION_NOTIFY, "GDK_MOTION_NOTIFY" },
	{ GDK_BUTTON_PRESS, "GDK_BUTTON_PRESS" },
	{ GDK_2BUTTON_PRESS, "GDK_2BUTTON_PRESS" },
	{ GDK_3BUTTON_PRESS, "GDK_3BUTTON_PRESS" },
	{ GDK_BUTTON_RELEASE, "GDK_BUTTON_RELEASE" },
	{ GDK_KEY_PRESS, "GDK_KEY_PRESS" },
	{ GDK_KEY_RELEASE, "GDK_KEY_RELEASE" },
	{ GDK_ENTER_NOTIFY, "GDK_ENTER_NOTIFY" },
	{ GDK_LEAVE_NOTIFY, "GDK_LEAVE_NOTIFY" },
	{ GDK_FOCUS_CHANGE, "GDK_FOCUS_CHANGE" },
	{ GDK_CONFIGURE, "GDK_CONFIGURE" },
	{ GDK_MAP, "GDK_MAP" },
	{ GDK_UNMAP, "GDK_UNMAP" },
	{ GDK_PROPERTY_NOTIFY, "GDK_PROPERTY_NOTIFY" },
	{ GDK_SELECTION_CLEAR, "GDK_SELECTION_CLEAR" },
	{ GDK_SELECTION_REQUEST, "GDK_SELECTION_REQUEST" },
	{ GDK_SELECTION_NOTIFY, "GDK_SELECTION_NOTIFY" },
	{ GDK_PROXIMITY_IN, "GDK_PROXIMITY_IN" },
	{ GDK_PROXIMITY_OUT, "GDK_PROXIMITY_OUT" },
	{ GDK_DRAG_ENTER, "GDK_DRAG_ENTER" },
	{ GDK_DRAG_LEAVE, "GDK_DRAG_LEAVE" },
	{ GDK_DRAG_MOTION, "GDK_DRAG_MOTION" },
	{ GDK_DRAG_STATUS, "GDK_DRAG_STATUS" },
	{ GDK_DROP_START, "GDK_DROP_START" },
	{ GDK_DROP_FINISHED, "GDK_DROP_FINISHED" },
	{ GDK_CLIENT_EVENT, "GDK_CLIENT_EVENT" },
	{ GDK_VISIBILITY_NOTIFY, "GDK_VISIBILITY_NOTIFY" },
	{ GDK_SCROLL, "GDK_SCROLL" },
	{ GDK_WINDOW_STATE, "GDK_WINDOW_STATE" },
	{ GDK_SETTING, "GDK_SETTING" },
	{ GDK_OWNER_CHANGE, "GDK_OWNER_CHANGE" },
	{ GDK_GRAB_BROKEN, "GDK_GRAB_BROKEN" },
	{ GDK_DAMAGE, "GDK_DAMAGE" },
	{ GDK_EVENT_LAST, "GDK_EVENT_LAST" },
};
#endif

Ctrl *Ctrl::GetTopCtrlFromId(int id)
{
	int q = FindId(id);
	if(q >= 0) {
		Ctrl *p = wins[q].ctrl;
		if(p && p->top)
			return p;
	}
	return NULL;
}

gboolean Ctrl::GtkDraw(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	GuiLock __;
	Ctrl *p = GetTopCtrlFromId(user_data);
	if(p) {
		p->fullrefresh = false;
		cairo_scale(cr, 1.0 / scale, 1.0 / scale);
		p->SyncWndRect(p->GetWndScreenRect()); // avoid black areas when resizing

		SystemDraw w(cr);
		painting = true;
		double x1, y1, x2, y2;
		cairo_clip_extents (cr, &x1, &y1, &x2, &y2);
		Rect r = RectC((int)x1, (int)y1, (int)ceil(x2 - x1), (int)ceil(y2 - y1));
		w.Clip(r); // Because of IsPainting

		cairo_rectangle_list_t *list = cairo_copy_clip_rectangle_list(cr);
		if(list->status == CAIRO_STATUS_SUCCESS && list->num_rectangles < 10) {
			Vector<Rect> clip;
			for(int i = 0; i < list->num_rectangles; i++) {
				const cairo_rectangle_t& r = list->rectangles[i];
				clip.Add(Rect((int)r.x, (int)r.y, (int)(r.x + r.width), (int)(r.y + r.height)));
			}
			w.PickInvalid(pick(clip));
		}
		cairo_rectangle_list_destroy(list);

		p->UpdateArea(w, r);
		w.End();
		painting = false;
	}
	return true;
}

gboolean Ctrl::GtkEvent(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GuiLock __;
	GdkEventKey *key;
	bool pressed = false;
	bool  retval = true;
	Value value;
	Ctrl *p = GetTopCtrlFromId(user_data);
#ifdef LOG_EVENTS
	String ev = "?";
	Tuple2<int, const char *> *f = FindTuple(xEvent, __countof(xEvent), event->type);
	if(f)
		ev = f->b;
	LOG(rmsecs() << " FETCH EVENT " << ev << " ctrl: " << Name(p));
#endif

	switch(event->type) {
	case GDK_DELETE:
		break;
	case GDK_FOCUS_CHANGE:
		if(p) {
			if(((GdkEventFocus *)event)->in)
				gtk_im_context_focus_in(p->top->im_context);
			else
				gtk_im_context_focus_out(p->top->im_context);
			AddEvent(user_data, EVENT_FOCUS_CHANGE, value, event);
		}
		return false;
	case GDK_MOTION_NOTIFY:
	case GDK_LEAVE_NOTIFY:
		break;
	case GDK_BUTTON_PRESS:
		value = DoButtonEvent(event, true);
		if(IsNull(value))
			return false;
		break;
	case GDK_2BUTTON_PRESS:
		value = DoButtonEvent(event, true);
		if(IsNull(value))
			return false;
		break;
	case GDK_BUTTON_RELEASE:
		value = DoButtonEvent(event, false);
		if(IsNull(value))
			return false;
		break;
	case GDK_SCROLL: {
		GdkEventScroll *e = (GdkEventScroll *)event;
		if(findarg(e->direction, GDK_SCROLL_UP, GDK_SCROLL_LEFT) >= 0)
			value = 120;
		else
		if(findarg(e->direction, GDK_SCROLL_DOWN, GDK_SCROLL_RIGHT) >= 0)
			value = -120;
		else
			return false;
		break;
	}
	case GDK_KEY_PRESS:
		pressed = true;
	case GDK_KEY_RELEASE:
		key = (GdkEventKey *)event;
		value << (int) key->keyval << (int) key->hardware_keycode;
		if(pressed) {
			p = GetTopCtrlFromId(user_data);
			if(p && gtk_im_context_filter_keypress(p->top->im_context, key))
				return true;
		}
		break;
	case GDK_CONFIGURE: {
		retval = false;
		GdkEventConfigure *e = (GdkEventConfigure *)event;
		value = SCL(e->x, e->y, e->width, e->height);
		break;
	}
	default:
		return false;
	}
	AddEvent(user_data, event->type, value, event);
	return retval;
}

int Ctrl::DoButtonEvent(GdkEvent *event, bool press)
{
	GdkEventButton *e = (GdkEventButton *)event;
	static int mask[] = { GDK_BUTTON1_MASK, GDK_BUTTON2_MASK, GDK_BUTTON3_MASK };
	if(e->button >= 1 && e->button <= 3) {
		int m = mask[e->button - 1];
		if(press)
			MouseState |= m;
		else
			MouseState &= ~m;
		return e->button;
	}
	return findarg(e->button, 8, 9) >= 0 ? (int)e->button : (int)Null;
}

Ctrl::GEvent::GEvent()
{
	event = NULL;

	pen = false;
	pen_barrel = false;
	pen_inverted = false;
	pen_eraser = false;
	pen_pressure = Null;
	pen_rotation = Null;
	pen_tilt = Null;
}

void Ctrl::GEvent::Free()
{
	if(event) {
		gdk_event_free(event);
		event = NULL;
	}
}

void Ctrl::GEvent::Set(const GEvent& e)
{
	*(GEvent0 *)this = e;
	event = e.event ? gdk_event_copy(e.event) : NULL;
}

Ctrl::GEvent::~GEvent()
{
	Free();
}

Ctrl::GEvent::GEvent(const GEvent& e)
{
	Set(e);
}

void Ctrl::GEvent::operator=(const GEvent& e)
{
	if(this == &e)
		return;
	Free();
	Set(e);
}

static Point s_mousepos;

Point Ctrl::GetMouseInfo(GdkWindow *win, GdkModifierType& mod)
{
#if GTK_CHECK_VERSION(3, 20, 0)
	GdkDisplay *display = gdk_window_get_display (win);
	GdkDevice *pointer = gdk_seat_get_pointer (gdk_display_get_default_seat (display));
	double x, y;
	gdk_window_get_device_position_double (win, pointer, &x, &y, &mod);
	return s_mousepos; //return Point((int)SCL(x), (int)SCL(y));
#else
	gint x, y;
	gdk_window_get_pointer(win, &x, &y, &mod);
	return Point(SCL(x), SCL(y));
#endif
}

void Ctrl::AddEvent(gpointer user_data, int type, const Value& value, GdkEvent *event)
{
	if(Events.GetCount() > 50000)
		return;
	GEvent& e = Events.AddTail();
	e.windowid = (uint32)(uintptr_t)user_data;
	e.type = type;
	e.value = value;
	GdkModifierType mod;
	e.mousepos = GetMouseInfo(gdk_get_default_root_window(), mod);
	if(event && event->type == GDK_MOTION_NOTIFY){
		GdkEventMotion *mevent = (GdkEventMotion *)event;
		e.mousepos = s_mousepos = Point(SCL(mevent->x_root), SCL(mevent->y_root));
	}
	e.state = (mod & ~(GDK_BUTTON1_MASK|GDK_BUTTON2_MASK|GDK_BUTTON3_MASK)) | MouseState;
	e.count = 1;
	e.event = NULL;
#if GTK_CHECK_VERSION(3, 22, 0)
	GdkDevice *d = gdk_event_get_source_device(event);
	if(d && findarg(gdk_device_get_source(d), GDK_SOURCE_PEN, GDK_SOURCE_TOUCHSCREEN) >= 0) {
		e.pen = true;
		e.pen_barrel = MouseState & GDK_BUTTON3_MASK;
		double *axes = NULL;
		switch(event->type){
			case GDK_BUTTON_PRESS:
				gdk_window_set_event_compression(((GdkEventButton *)event)->window, false);
			case GDK_2BUTTON_PRESS:
			case GDK_3BUTTON_PRESS:
				axes = ((GdkEventButton *)event)->axes;
				break;
			case GDK_BUTTON_RELEASE:
				gdk_window_set_event_compression(((GdkEventButton *)event)->window, true);
				axes = ((GdkEventButton *)event)->axes;
				break;
			case GDK_MOTION_NOTIFY:{
				GdkEventMotion *mevent = (GdkEventMotion *)event;
				e.mousepos = s_mousepos = Point(SCL(mevent->x_root), SCL(mevent->y_root));
				axes = ((GdkEventMotion *)event)->axes;
				break;
			}
			default:;
		}
		if(axes) {
			if(!gdk_device_get_axis(d, axes, GDK_AXIS_PRESSURE, &e.pen_pressure)) e.pen_pressure=Null;
			if(!gdk_device_get_axis(d, axes, GDK_AXIS_ROTATION, &e.pen_rotation)) e.pen_rotation=Null;
			if(!gdk_device_get_axis(d, axes, GDK_AXIS_XTILT, &e.pen_tilt.x)) e.pen_tilt.x=Null;
			if(!gdk_device_get_axis(d, axes, GDK_AXIS_YTILT, &e.pen_tilt.y)) e.pen_tilt.y=Null;
		}
	}
#endif
}

void Ctrl::IMCommit(GtkIMContext *context, gchar *str, gpointer user_data)
{
	GuiLock __;
	AddEvent(user_data, EVENT_TEXT, FromUtf8(str), NULL);
}

bool Ctrl::ProcessInvalids()
{
	GuiLock __;
	if(invalids) {
		for(Win& win : wins) {
			for(const Rect& r : win.invalid)
				gdk_window_invalidate_rect(win.gdk, GdkRect(r), TRUE);
			win.invalid.Clear();
		}
		invalids = false;
	}
	return invalids;
}

void Ctrl::FetchEvents(bool may_block)
{
	LLOG("FetchEvents " << may_block);
	int level = LeaveGuiMutexAll();
	while(g_main_context_iteration(NULL, may_block))
		may_block = false;
	EnterGuiMutex(level);
}

bool Ctrl::IsWaitingEvent0(bool fetch)
{
	if(fetch)
		FetchEvents(FALSE);
	return Events.GetCount();
}

bool Ctrl::IsWaitingEvent()
{
	ASSERT_(IsMainThread(), "IsWaitingEvent can only run in the main thread");
	return IsWaitingEvent0(true);
}

struct ProcStop {
	TimeStop tm;
	String   ev;
	
	~ProcStop() { LOG("* " << ev << " elapsed " << tm); }
};

bool Ctrl::DispatchMouseIn(int act, int zd)
{
	Point p = GetMousePos();
	Rect r = GetScreenRect();
	if(r.Contains(p)) {
		p -= r.TopLeft();
		DispatchMouse(act, p, zd);
		return true;
	}
	return false;
}

Image Ctrl::GtkMouseEvent(int action, int act, int zd)
{
	if(grabpopup && activePopup.GetCount()) {
		for(int i = visiblePopup.GetCount(); --i >= 0;)
			if(visiblePopup[i] && visiblePopup[i]->DispatchMouseIn(act, zd))
				return Null;
		if(action == DOWN) { // Deactivate active popup(s) if clicked outside of active popups
			IgnoreMouseUp();
			if(activePopup.Top())
				activePopup.Top()->GetTopWindow()->ActivateWnd();
		}
		else
		if(activePopup[0]) { // Redirect other events to TopWindow that spawned first popup
			Ptr<TopWindow> w = activePopup[0]->GetTopWindow();
			if(w)
				w->DispatchMouseIn(act, zd);
		}
		return Null;
	}
	return DispatchMouse(act, GetMousePos() - GetScreenRect().TopLeft(), zd);
}

void Ctrl::GtkButtonEvent(int action)
{
	int act = action;
	int button = CurrentEvent.value;
	if(action != MOUSEMOVE)
		act |= button == 2 ? MIDDLE : button == 3 ? RIGHT : LEFT;
	GtkMouseEvent(action, act, 0);
}

void Ctrl::Proc()
{
#ifdef LOG_EVENTS
	String ev = "?";
	Tuple2<int, const char *> *f = FindTuple(xEvent, __countof(xEvent), CurrentEvent.type);
	if(f)
		ev = f->b;
	LOG("> PROCESS EVENT " << Upp::Name(this) << " " << ev);
	ProcStop tm;
	tm.ev = ev;
#endif
	if(!IsOpen())
		return;
	Ptr<Ctrl> _this = this;
	bool pressed = false;
	int  kv, hw;
	static int clicktime = msecs() - 100000;

#if GTK_CHECK_VERSION(3, 22, 0)
	pen.barrel = CurrentEvent.pen_barrel;
	pen.inverted = CurrentEvent.pen_inverted;
	pen.eraser = CurrentEvent.pen_eraser;
	pen.pressure = CurrentEvent.pen_pressure;
	pen.rotation = CurrentEvent.pen_rotation;
	pen.tilt = CurrentEvent.pen_tilt;
	
	is_pen_event = CurrentEvent.pen;

	auto DoPen = [&](Point p) {
		GuiLock __;
		eventCtrl = this;
		Ctrl *q = this;
		if(captureCtrl){
			q = captureCtrl;
			p += GetScreenRect().TopLeft() - captureCtrl->GetScreenRect().TopLeft();
		}
		else
			for(Ctrl *t = q; t; t=q->ChildFromPoint(p)) q = t;
		
		q->Pen(p, pen, GetMouseFlags());
		SyncCaret();
		Image m = CursorOverride();
		if(IsNull(m)) SetMouseCursor(q->CursorImage(p,GetMouseFlags()));
		else SetMouseCursor(m);
	};

	if(is_pen_event &&
	   findarg(CurrentEvent.type, GDK_MOTION_NOTIFY, GDK_BUTTON_PRESS, GDK_BUTTON_RELEASE) >= 0)
	{
		pen.action = decode(CurrentEvent.type, GDK_BUTTON_PRESS, PEN_DOWN, GDK_BUTTON_RELEASE, PEN_UP, 0);
		
		DoPen(GetMousePos() - GetScreenRect().TopLeft());
	}
#endif

	switch(CurrentEvent.type) {
	case GDK_MOTION_NOTIFY:
		GtkMouseEvent(MOUSEMOVE, MOUSEMOVE, 0);
		break;
	case GDK_BUTTON_PRESS:
		if(CurrentEvent.value == 8) {
			DispatchKey(K_MOUSE_BACKWARD, 1);
			break;
		}
		if(CurrentEvent.value == 9) {
			DispatchKey(K_MOUSE_FORWARD, 1);
			break;
		}
		if(!HasWndFocus() && !popup)
			SetWndFocus();
		ClickActivateWnd();
		if(ignoremouseup) {
			KillRepeat();
			ignoreclick = false;
			ignoremouseup = false;
		}
		
		if(!ignoreclick) {
			bool dbl = msecs(clicktime) < 250;
			clicktime = dbl ? clicktime - 1000 : msecs();
			GtkButtonEvent(dbl ? DOUBLE : DOWN);
		}
		break;
/*	case GDK_2BUTTON_PRESS:
		if(!ignoreclick)
			GtkButtonEvent(DOUBLE);
		break;
*/	case GDK_BUTTON_RELEASE:
		if(CurrentEvent.value == 8) {
			DispatchKey(K_MOUSE_BACKWARD|K_KEYUP, 1);
			break;
		}
		if(CurrentEvent.value == 9) {
			DispatchKey(K_MOUSE_FORWARD|K_KEYUP, 1);
			break;
		}
		if(ignoreclick)
			EndIgnore();
		else
		if(!dnd_events)
			GtkButtonEvent(UP);
		break;
	case GDK_SCROLL: {
		GtkMouseEvent(MOUSEWHEEL, MOUSEWHEEL, CurrentEvent.value);
		break;
	}
	case GDK_KEY_PRESS:
		pressed = true;
	case GDK_KEY_RELEASE:
		kv = CurrentEvent.value[0];
		hw = CurrentEvent.value[1];
		if(kv >= 0 && kv < 65536) {
			LLOG("keyval " << FormatIntHex(kv) << ' ' << (char)kv);
			if(kv >= 'a' && kv <= 'z')
				kv = kv - 'a' + 'A';
			static Tuple2<int, int> cv[] = {
				{ GDKEY(BackSpace), K_BACKSPACE },
				{ GDKEY(Tab), K_TAB },
				{ GDKEY(ISO_Left_Tab), K_TAB },
				{ GDKEY(Return), K_ENTER },
				{ GDKEY(Escape), K_ESCAPE },
				{ GDKEY(space), K_SPACE },
				{ GDKEY(Control_L), K_CTRL_KEY },
				{ GDKEY(Control_R), K_CTRL_KEY },
				{ GDKEY(Shift_L), K_SHIFT_KEY },
				{ GDKEY(Shift_R), K_SHIFT_KEY },
				{ GDKEY(Alt_L), K_ALT_KEY },
				{ GDKEY(Alt_R), K_ALT_KEY },

				{ GDKEY(KP_Space), K_SPACE },
				{ GDKEY(KP_Tab), K_TAB },
				{ GDKEY(KP_Enter), K_ENTER },
				{ GDKEY(KP_F1), K_F1 },
				{ GDKEY(KP_F2), K_F2 },
				{ GDKEY(KP_F3), K_F3 },
				{ GDKEY(KP_F4), K_F4 },
				{ GDKEY(KP_Home), K_HOME },
				{ GDKEY(KP_Left), K_LEFT },
				{ GDKEY(KP_Up), K_UP },
				{ GDKEY(KP_Right), K_RIGHT },
				{ GDKEY(KP_Down), K_DOWN },
				{ GDKEY(KP_Page_Up), K_PAGEUP },
				{ GDKEY(KP_Page_Down), K_PAGEDOWN },
				{ GDKEY(KP_End), K_END },
				{ GDKEY(KP_Begin), K_HOME },
				{ GDKEY(KP_Insert), K_INSERT },
				{ GDKEY(KP_Delete), K_DELETE },
			};

			if(kv > 256) { // Non-latin keyboard layout should still produce accelerators like Ctrl+C etc...
				static VectorMap<int, int> hwkv; // convert hw keycode to Latin GTK keyval
				ONCELOCK {
					for(int i = 1; i < 256; i++) { // Latin keyvals are in 0..255 range
						GdkKeymapKey *keys;
						gint n_keys;
						if(gdk_keymap_get_entries_for_keyval(gdk_keymap_get_for_display(gdk_display_get_default()), i, &keys, &n_keys)) {
							for(int j = 0; j < n_keys; j++)
								if(keys[j].group == 0)
									hwkv.Add(keys[j].keycode, i);
							g_free(keys);
						}
					}
				}
				kv = hwkv.Get(hw, kv);
			}

			Tuple2<int, int> *x = FindTuple(cv, __countof(cv), kv);
			if(x)
				kv = x->b;
			else {
				if(GetCtrl() | GetAlt()) {// fix Ctrl+Shift+1 etc...
					static VectorMap<int, int> hwkv;
					ONCELOCK {
						for(int k : { GDKEY(0), GDKEY(1), GDKEY(2), GDKEY(3), GDKEY(4),
						              GDKEY(5), GDKEY(6), GDKEY(7), GDKEY(8), GDKEY(9) }) {
							GdkKeymapKey *keys;
							gint n_keys;
							if(gdk_keymap_get_entries_for_keyval(NULL, k, &keys, &n_keys)) {
								for(int j = 0; j < n_keys; j++)
									if(keys[j].group == 0)
										hwkv.Add(keys[j].keycode, k);
								g_free(keys);
							}
						}
					};
					kv = hwkv.Get(hw, kv);
				}
				kv += K_DELTA;
			}
			if(GetShift() && kv != K_SHIFT_KEY)
				kv |= K_SHIFT;
			if(GetCtrl() && kv != K_CTRL_KEY)
				kv |= K_CTRL;
			if(GetAlt() && kv != K_ALT_KEY)
				kv |= K_ALT;
			LLOG(GetKeyDesc(kv) << ", pressed: " << pressed << ", count: " << CurrentEvent.count);
#ifdef GDK_WINDOWING_X11
			if(pressed)
				for(int i = 0; i < hotkey.GetCount(); i++) {
					if(hotkey[i] && keyhot[i] == (dword)kv) {
						hotkey[i]();
						return;
					}
				}
#endif
			DispatchKey(!pressed * K_KEYUP + kv, CurrentEvent.count);
		}
		break;
	case EVENT_TEXT: {
		WString h = CurrentEvent.value;
		for(int i = 0; i < h.GetCount(); i++) // TODO: Add compression
			DispatchKey(h[i], 1);
		break;
	}
	case EVENT_FOCUS_CHANGE:
		activeCtrl = NULL;
		break;
	case GDK_DELETE: {
		TopWindow *w = dynamic_cast<TopWindow *>(this);
		if(w) {
			if(IsEnabled()) {
				IgnoreMouseUp();
				w->WhenClose();
			}
		}
		return;
	}
	case GDK_CONFIGURE:
		SyncWndRect(CurrentEvent.value);
		break;
	default:
		return;
	}
	if(_this)
		_this->PostInput();
}

void Ctrl::SyncWndRect(const Rect& rect)
{
	if(GetRect() != rect)
		SetWndRect(rect);
	TopWindow *w = dynamic_cast<TopWindow *>(this);
	if(w && w->state == TopWindow::OVERLAPPED)
		w->overlapped = rect;
}

bool Ctrl::ProcessEvent0(bool *quit, bool fetch)
{
	ASSERT(IsMainThread());
	bool r = false;
	if(IsWaitingEvent0(fetch)) {
		while(Events.GetCount() > 1) { // GEvent compression (coalesce autorepeat, mouse moves/wheel, configure)
			GEvent& a = Events[0];
			GEvent& b = Events[1];
			if(b.type == a.type && a.windowid == b.windowid && a.state == b.state) {
				if(a.type == GDK_KEY_PRESS && a.value == b.value)
					b.count += a.count;
			    else
			    if(a.type == GDK_SCROLL)
			        b.value = (int)b.value + (int)a.value;
				else
				if(a.type != GDK_CONFIGURE)
					break;
				Events.DropHead();
			}
			else
				break;
			FocusSync();
		}
		GEvent& e = Events.Head();
		CurrentTime = e.time;
		CurrentMousePos = e.mousepos;
		CurrentState = e.state;
		CurrentEvent = e;
		Value val = e.value;
		Events.DropHead();
		Ctrl *w = GetTopCtrlFromId(e.windowid);
		FocusSync();
		CaptureSync();
		if(w)
			w->Proc();
		r = true;
	}
	if(quit)
		*quit = IsEndSession();
	FocusSync();
	SyncCaret();
	return r;
}

bool Ctrl::ProcessEvent(bool *quit)
{
	return ProcessEvent0(quit, true);
}

gboolean Ctrl::TimeHandler(GtkWidget *)
{ // we only need timer to periodically break blocking g_main_context_iteration
	return true;
}

void SweepMkImageCache();

bool Ctrl::ProcessEvents0(bool *quit, bool fetch)
{
	bool r = false;
	while(IsWaitingEvent0(fetch) && (!LoopCtrl || LoopCtrl->InLoop()))
		r = ProcessEvent0(quit, fetch) || r;
	TimerProc(msecs());
	AnimateCaret();
	if(quit)
		*quit = IsEndSession();
	for(int i = 0; i < wins.GetCount(); i++)
		if(wins[i].ctrl)
			wins[i].ctrl->SyncScroll();
	ProcessInvalids();
	FetchEvents(FALSE); // To perform any pending GDK_EXPOSE
	gdk_display_flush(gdk_display_get_default());
	return r;
}

bool Ctrl::ProcessEvents(bool *quit)
{
	ASSERT_(IsMainThread(), "ProcessEvents can only run in the main thread");
	return ProcessEvents0(quit, true);
}

void Ctrl::SysEndLoop()
{
}

void WakeUpGuiThread()
{
	g_main_context_wakeup(g_main_context_default());
}

void Ctrl::EventLoop(Ctrl *ctrl)
{
	GuiLock __;
	ASSERT_(IsMainThread(), "EventLoop can only run in the main thread");
	ASSERT(LoopLevel == 0 || ctrl);
	LoopLevel++;
	LLOG("Entering event loop at level " << LoopLevel << LOG_BEGIN);
	if(!GetMouseRight() && !GetMouseMiddle() && !GetMouseLeft())
		ReleaseCtrlCapture();
	Ptr<Ctrl> ploop;
	if(ctrl) {
		ploop = LoopCtrl;
		LoopCtrl = ctrl;
		ctrl->inloop = true;
	}

	while(!IsEndSession() &&
	      (ctrl ? ctrl->IsOpen() && ctrl->InLoop() : GetTopCtrls().GetCount())) {
		FetchEvents(TRUE);
		ProcessEvents();
	}

	if(ctrl)
		LoopCtrl = ploop;
	LoopLevel--;
	LLOG(LOG_END << "Leaving event loop ");
}

gboolean sOnce(GtkWidget *)
{
	return false;
}

void Ctrl::GuiSleep(int ms)
{
	GuiLock __;
	ASSERT_(IsMainThread(), "Only the main thread can perform GuiSleep");
	if(ms < 20) // Periodic timer is each 20ms, so that is the longest possible wait
		g_timeout_add(ms, (GSourceFunc) sOnce, NULL); // otherwise setup shorter timer
	FetchEvents(TRUE);
}

}

#endif
