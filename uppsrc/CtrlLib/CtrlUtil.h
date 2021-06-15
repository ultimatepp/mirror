void Animate(Ctrl& c, const Rect& target, int type = -1);
void Animate(Ctrl& c, int x, int y, int cx, int cy, int type = -1);

bool CtrlLibDisplayError(const Value& ev);

bool EditText(String& s, const char *title, const char *label, int (*filter)(int), int maxlen = 0);
bool EditText(String& s, const char *title, const char *label, int maxlen = 0);
bool EditTextNotNull(String& s, const char *title, const char *label, int (*filter)(int), int maxlen = 0);
bool EditTextNotNull(String& s, const char *title, const char *label, int maxlen = 0);

bool EditText(WString& s, const char *title, const char *label, int (*filter)(int), int maxlen = 0);
bool EditText(WString& s, const char *title, const char *label, int maxlen = 0);
bool EditTextNotNull(WString& s, const char *title, const char *label, int (*filter)(int), int maxlen = 0);
bool EditTextNotNull(WString& s, const char *title, const char *label, int maxlen = 0);

bool EditNumber(int& n, const char *title, const char *label, int min = INT_MIN, int max = INT_MAX, bool notnull = false);
bool EditNumber(double& n, const char *title, const char *label, double min = -DBL_MAX, double max = DBL_MAX, bool notnull = false);

bool EditDateDlg(Date& d, const char *title, const char *label, Date min = Date::Low(), Date max = Date::High(), bool notnull = false);

void Show2(Ctrl& ctrl1, Ctrl& ctrl, bool show = true);
void Hide2(Ctrl& ctrl1, Ctrl& ctrl);

#ifndef PLATFORM_WINCE //TODO?
void UpdateFile(const char *filename);
void SelfUpdate();
bool SelfUpdateSelf();
#endif

void WindowsList();
void WindowsMenu(Bar& bar);

class DelayCallback : public Pte<DelayCallback> {
	Event<>  target;
	int      delay;

public:
	void     Invoke();
	void     operator<<=(Event<> x)   { target = x; }
	void     SetDelay(int ms)         { delay = ms; }
	Event<>  Get()                    { return callback(this, &DelayCallback::Invoke); }
	Event<>  operator~()              { return Get(); }
	operator Event<>()                { return Get(); }

	DelayCallback()                   { delay = 2000; }
	~DelayCallback()                  { KillTimeCallback(this); }
};

#ifdef GUI_WIN
struct Win32PrintDlg_;
#endif

#ifndef PLATFORM_WINCE

#ifndef VIRTUALGUI

class PrinterJob {
#ifdef GUI_WIN
	One<Win32PrintDlg_> pdlg;
	bool Execute0(bool dodlg);
#endif
#ifdef PLATFORM_POSIX
	Size                pgsz;
	Size                GetDefaultPageSize(String *name = NULL);
#endif
	One<Draw>           draw;
	Vector<int>         page;
	int                 from, to, current;
	bool                landscape;
	String              name;
	String              options;
	bool                dlgSuccess;
	bool                Execute0();
	
public:
	Draw&               GetDraw();
	operator            Draw&()                         { return GetDraw(); }
	const Vector<int>&  GetPages() const                { return page; }
	int                 operator[](int i) const         { return page[i]; }
	int                 GetPageCount() const            { return page.GetCount(); }

	bool                Execute();

	PrinterJob& Landscape(bool b = true)                { landscape = b; return *this; }
	PrinterJob& MinMaxPage(int minpage, int maxpage);
	PrinterJob& PageCount(int n)                        { return MinMaxPage(0, n - 1); }
	PrinterJob& CurrentPage(int currentpage);
	PrinterJob& Name(const char *_name)                 { name = _name; return *this; }

	PrinterJob(const char *name = NULL);
	~PrinterJob();
};

#endif

#endif

#ifdef GUI_X11

class TrayIcon : Ctrl {
	virtual bool HookProc(XEvent *event);
	virtual void Paint(Draw& draw);
	virtual void LeftDown(Point p, dword keyflags);
	virtual void LeftUp(Point p, dword keyflags);
	virtual void LeftDouble(Point p, dword keyflags);
	virtual void RightDown(Point p, dword keyflags);

private:
	void AddToTray();

	Window traywin;
	Image  icon;
	void DoMenu(Bar& bar);
	void Call(int code, unsigned long d1, unsigned long d2, unsigned long d3);
	void Message(int type, const char *title, const char *text, int timeout);

public:
	virtual void    LeftDown();
	virtual void    LeftUp();
	virtual void    LeftDouble();
	virtual void    Menu(Bar& bar);
	virtual void    BalloonLeftDown();
	virtual void    BalloonShow();
	virtual void    BalloonHide();
	virtual void    BalloonTimeout();

	Event<>         WhenLeftDown;
	Event<>         WhenLeftUp;
	Event<>         WhenLeftDouble;
	Event<Bar&> WhenBar;
	Event<>         WhenBalloonLeftDown;
	Event<>         WhenBalloonShow;
	Event<>         WhenBalloonHide;
	Event<>         WhenBalloonTimeout;

	void            Break()                                { EndLoop(0); }
	void            Run()                                  { EventLoop(this); }

	void            Show(bool b = true);
	void            Hide()                                 { Show(false); }
	bool            IsVisible() const                      { return true; }

	void            Info(const char *title, const char *text, int timeout = 10)    { Message(1, title, text, timeout); }
	void            Warning(const char *title, const char *text, int timeout = 10) { Message(2, title, text, timeout); }
	void            Error(const char *title, const char *text, int timeout = 10)   { Message(3, title, text, timeout); }

	TrayIcon&  Icon(const Image &img)                      { icon = img; Refresh(); return *this; }
	TrayIcon&  Tip(const char *text)                       { Ctrl::Tip(text); return *this; }

	typedef TrayIcon CLASSNAME;

	TrayIcon();
};

#endif

#ifdef GUI_GTK

class TrayIcon {
private:
	GtkStatusIcon *tray_icon;
	String         tooltip;
	ImageGdk       image;
	bool           active;

	static gboolean DoButtonPress(GtkStatusIcon *, GdkEventButton *e, gpointer user_data);
	static gboolean DoButtonRelease(GtkStatusIcon *, GdkEventButton *e, gpointer user_data);
	static void     PopupMenu(GtkStatusIcon *, guint, guint32, gpointer user_data);
	static void     DoActivate(GtkStatusIcon *, gpointer user_data);

	void DoMenu(Bar& bar);
	void ExecuteMenu();

	void Sync();

	void Message(int type, const char *title, const char *text, int timeout);

public:
	virtual void    Menu(Bar& bar);
	virtual void    LeftDown();
	virtual void    LeftUp();
	virtual void    LeftDouble();

	Event<>         WhenLeftDown;
	Event<>         WhenLeftUp;
	Event<>         WhenLeftDouble;
	Event<Bar&> WhenBar;

	void            Break();
	void            Run();

	void            Show(bool b = true);
	void            Hide()                                 { Show(false); }
	bool            IsVisible() const;

	// Not implemented by GTK:
	void            Info(const char *title, const char *text, int timeout = 10)    { Message(1, title, text, timeout); }
	void            Warning(const char *title, const char *text, int timeout = 10) { Message(2, title, text, timeout); }
	void            Error(const char *title, const char *text, int timeout = 10)   { Message(3, title, text, timeout); }

	TrayIcon&  Icon(const Image &img)                      { if(image.Set(img)) Sync(); return *this; }
	TrayIcon&  Tip(const char *text)                       { if(tooltip != text) tooltip = text; Sync(); return *this; }

	typedef TrayIcon CLASSNAME;

	TrayIcon();
	~TrayIcon();
};

#endif

#ifdef GUI_WIN
#ifndef PLATFORM_WINCE

class TrayIcon : private Ctrl {
	struct NotifyIconOld {
		dword sz;
		HWND  hwnd;
		dword id;
		dword flags;
		dword message;
		HICON icon;
		char  tip[64];
	};
	struct NotifyIconNew {
		dword sz;
		HWND  hwnd;
		dword id;
		dword flags;
		dword message;
		HICON icon;
		char  tip[128];

		dword state;
		dword statemask;
		char  info[256];
		dword timeout;
		char  title[64];
		dword infoflags;
	};

	Image          icon;
	bool           visible;
	String         tip;
	NotifyIconNew  nid;
	HWND           hwnd;

	void Notify(dword msg);
	void DoMenu(Bar& bar);
	void Message(int type, const char *title, const char *text, int timeout = 10);

protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

public:
	virtual void    Menu(Bar& bar);
	virtual void    LeftDown();
	virtual void    LeftUp();
	virtual void    LeftDouble();
	virtual void    BalloonLeftDown();
	virtual void    BalloonShow();
	virtual void    BalloonHide();
	virtual void    BalloonTimeout();

	Event<>         WhenLeftDown;
	Event<>         WhenLeftUp;
	Event<>         WhenLeftDouble;
	Event<Bar&> WhenBar;
	Event<>         WhenBalloonLeftDown;
	Event<>         WhenBalloonShow;
	Event<>         WhenBalloonHide;
	Event<>         WhenBalloonTimeout;

	void            Show(bool b = true);
	void            Hide()                                 { Show(false); }
	bool            IsVisible() const                      { return visible; }
	void            Break()                                { EndLoop(0); }
	void            Run()                                  { EventLoop(this); }

	void            Info(const char *title, const char *text, int timeout = 10)    { Message(1, title, text, timeout); }
	void            Warning(const char *title, const char *text, int timeout = 10) { Message(2, title, text, timeout); }
	void            Error(const char *title, const char *text, int timeout = 10)   { Message(3, title, text, timeout); }

	TrayIcon&  Icon(const Image &img);
	TrayIcon&  Tip(const char *text);

	typedef TrayIcon CLASSNAME;

	TrayIcon();
	~TrayIcon();
};

class FileSelNative {
protected:
	struct FileType : Moveable<FileType> {
		String name;
		String ext;
	};

	Vector<FileType> type;

	int    activetype;
	String activedir;
	String defext;
	bool   rdonly;
	bool   multi;
	bool   readonly;
	bool   asking;

	Vector<String> filename;

public:
	void Serialize(Stream& s);

	void New()                                   { filename.Clear(); }
	bool IsNew() const                           { return filename.IsEmpty(); }
	bool Execute(bool open, const char *title = NULL);
	bool ExecuteOpen(const char *title = NULL)   { return Execute(true, title); }
	bool ExecuteSaveAs(const char *title = NULL) { return Execute(false, title); }

	String Get() const;
	void  Set(const String& s)                   { filename.At(0) = s; }

	operator String() const                      { return Get(); }
	void operator=(const String& s)              { Set(s); }

	String operator~() const                     { return Get(); }
	void operator<<=(const String& s)            { Set(s); }

	int   GetCount() const                       { return filename.GetCount(); }
	const String& operator[](int i) const        { return filename[i]; }

	bool   GetReadOnly() const                   { return readonly; }
	String GetActiveDir() const                  { return activedir; }

	FileSelNative& Type(const char *name, const char *ext);
	FileSelNative& AllFilesType();
	FileSelNative& ActiveDir(const String& dir)   { activedir = dir; return *this; }
	FileSelNative& ActiveType(int i)              { activetype = i; return *this;  }
	FileSelNative& DefaultExt(const char *ext)    { defext = ext; return *this; }
	FileSelNative& Multi(bool b = true)           { multi = b; return *this; }
	FileSelNative& ReadOnlyOption(bool b = true)  { rdonly = b; return *this; }
	FileSelNative& Asking(bool b = true)          { asking = b; return *this; }
	FileSelNative& NoAsking()                     { return Asking(false); }

	FileSelNative();
};

typedef FileSelNative FileSelector;

#endif
#endif

#ifdef GUI_X11
typedef FileSel FileSelNative;
#endif

#if defined(GUI_GTK) || defined(PLATFORM_COCOA)
class FileSelNative {
	Vector<String> path;
	Vector< Tuple2<String, String> > type;
	
	String ipath;
	bool   confirm;
	bool   multi;
	bool   hidden;
	int    activetype;

public:
	bool   Execute(bool open, const char *title = NULL);
	bool   ExecuteOpen(const char *title = NULL)          { return Execute(true, title); }
	bool   ExecuteSaveAs(const char *title = NULL)        { return Execute(false, title); }

	String Get() const                                    { return path.GetCount() ? path[0] : String::GetVoid(); }
	operator String() const                               { return Get(); }
	String operator~() const                              { return Get(); }
	
	void   Set(const String& s)                           { ipath = s; }
	void   operator=(const String& s)                     { Set(s); }
	void   operator<<=(const String& s)                   { Set(s); }

	int    GetCount() const                               { return path.GetCount(); }
	const  String& operator[](int i) const                { return path[i]; }

	FileSelNative& Type(const char *name, const char *ext) { type.Add(MakeTuple(String(name), String(ext))); return *this; }
	FileSelNative& AllFilesType();
	FileSelNative& Asking(bool b = true)                   { confirm = b; return *this; }
	FileSelNative& NoAsking()                              { return Asking(false); }
	FileSelNative& Multi(bool b = true)                    { multi = b; return *this; }
	FileSelNative& ShowHidden(bool b = true)               { hidden = b; return *this; }
	FileSelNative& ActiveDir(const String& dir)            { ipath = dir; return *this; }
	FileSelNative& ActiveType(int i)                       { activetype = i; return *this; }

	FileSelNative();
};

typedef FileSelNative FileSelector;
#endif

class CtrlMapper {
	bool toctrls = true;

public:
	template <class T>
	CtrlMapper& operator()(Ctrl& ctrl, T& val) { if(toctrls) ctrl <<= val; else val = ~ctrl; return *this; }
	
	CtrlMapper& ToCtrls()                      { toctrls = true; return *this; }
	CtrlMapper& ToValues()                     { toctrls = false; return *this; }
};

class CtrlRetriever {
public:
	struct Item {
		virtual void Set() {}
		virtual void Retrieve() = 0;
		virtual ~Item() {}
	};

private:
	struct CtrlItem0 : Item {
		Ctrl  *ctrl;
	};

	template <class T>
	struct CtrlItem : CtrlItem0 {
		T     *value;

		virtual void Set()       { *ctrl <<= *value; }
		virtual void Retrieve()  { *value = ~*ctrl; }
		virtual ~CtrlItem() {}
	};

	Array<Item> item;

public:
	void Put(Item *newitem)                       { item.Add(newitem); }

	void Put(One<Item>&& newitem)                 { item.Add(newitem.Detach()); }

	template <class T>
	void Put(Ctrl& ctrl, T& val);

	template <class T>
	CtrlRetriever& operator()(Ctrl& ctrl, T& val) { Put(ctrl, val); return *this; }

	void Set();
	void Retrieve();

	Event<>  operator^=(Event<> cb);
	Event<>  operator<<(Event<> cb);
	
	void Reset()                                  { item.Clear(); }

// Backward compatibility
	Event<>  operator<<=(Event<> cb)              { return *this ^= cb; }
};

template <class T>
void CtrlRetriever::Put(Ctrl& ctrl, T& val)
{
	CtrlItem<T> *m = new CtrlItem<T>();
	m->ctrl = &ctrl;
	m->value = &val;
	m->Set();
	Put(m);
}

class IdCtrls {
protected:
	struct Item {
		Id    id;
		Ctrl *ctrl;
	};
	Array<Item> item;

public:
	void        Reset()                              { item.Clear(); }

	void            Add(Id id, Ctrl& ctrl);
	IdCtrls&        operator()(Id id, Ctrl& ctrl)    { Add(id, ctrl); return *this; }
	int             GetCount() const                 { return item.GetCount(); }
	Ctrl&           operator[](int i)                { return *item[i].ctrl; }
	const Ctrl&     operator[](int i) const          { return *item[i].ctrl; }
	Id              GetKey(int i) const              { return item[i].id; }
	Id              operator()(int i) const          { return item[i].id; }

	bool            Accept();
	void            ClearModify();
	bool            IsModified();
	void            Enable(bool b = true);
	void            Disable()                        { Enable(false); }
	void            SetNull();
	
	Event<>         operator<<(Event<> action);
	Event<>         operator^=(Event<> action);

	ValueMap        Get() const;
	void            Set(const ValueMap& m);
	ValueMap        operator~() const                { return Get(); }
	const ValueMap& operator<<=(const ValueMap& m)   { Set(m); return m; }
};

class FileSelButton : public FileSel
{
public:
	enum MODE { MODE_OPEN, MODE_SAVE, MODE_DIR };
	FileSelButton(MODE mode = MODE_OPEN, const char *title = NULL);
	
	void               Attach(Ctrl& parent);
	void               Detach();
	void               Title(String t)      { title = t; }
	String             GetTitle() const     { return title; }
	
	Event<>            WhenSelected;
	
	FileSelButton&     Tip(const char *txt) { button.Tip(txt); return *this; }
	
private:
	void               OnAction();

private:
	FrameRight<Button> button;
	String             title;
	MODE               mode;
};

struct OpenFileButton  : FileSelButton { OpenFileButton(const char *title = NULL) : FileSelButton(MODE_OPEN, title) {} };
struct SaveFileButton  : FileSelButton { SaveFileButton(const char *title = NULL) : FileSelButton(MODE_SAVE, title) {} };
struct SelectDirButton : FileSelButton { SelectDirButton(const char *title = NULL) : FileSelButton(MODE_DIR,  title) {} };

void Set(ArrayCtrl& array, int ii, IdCtrls& m);
void Get(ArrayCtrl& array, int ii, IdCtrls& m);

void   UpdateSetDir(const char *path);
void   UpdateSetUpdater(const char *exename);
String UpdateGetDir();

void UpdateFile(String dst, String src);

void MemoryProfileInfo();

struct sPaintRedirectCtrl : Ctrl {
	Ctrl *ctrl;
	
	virtual void Paint(Draw& w) {
		ctrl->Paint(w);
	}
};
