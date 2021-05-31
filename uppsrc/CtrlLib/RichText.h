class RichTextView : public Ctrl {
public:
	virtual void  Paint(Draw& w);
	virtual bool  Key(dword key, int count);
	virtual void  Layout();
	virtual Value GetData() const;
	virtual void  SetData(const Value& v);
	virtual void  MouseWheel(Point p, int zdelta, dword keyflags);
	virtual Image CursorImage(Point p, dword keyflags);
	virtual void  LeftDown(Point p, dword keyflags);
	virtual void  MouseMove(Point p, dword keyflags);
	virtual void  LeftRepeat(Point p, dword keyflags);
	virtual void  LeftDouble(Point p, dword keyflags);
	virtual void  LeftTriple(Point p, dword keyflags);
	virtual void  RightDown(Point p, dword keyflags);
	virtual String GetSelectionData(const String& fmt) const;

private:
	Rect          margin;
	Color         background;
	Color         textcolor;
	Zoom          zoom;
	int           cx;
	ScrollBar     sb;
	Scroller      scroller;
	RichText      text;
	bool          sizetracking;
	bool          vcenter;
	bool          hldec;
	int           highlight;
	int           sell, selh;
	int           cursor, anchor;
	bool          lazy;
	bool          shrink_oversized_objects;

	void          EndSizeTracking();
	void          SetSb();
	void          Scroll();
	int           GetPageCx(bool reduced = false) const;
	Point         GetTextPoint(Point p) const;
	int           GetPointPos(Point p) const;
	String        GetLink(int pos, Point p) const;
	void          RefreshSel();
	void          RefreshRange(int a, int b);
	WString       GetSelText() const;
	int           TopY() const;

protected:
	enum {
		TIMEID_ENDSIZETRACKING = Ctrl::TIMEID_COUNT,
		TIMEID_COUNT
	};

public:
	Event<const String&> WhenLink;
	Event<int>           WhenMouseMove;

	void            Clear();
	void            Pick(RichText&& t);
	void            Pick(RichText&& txt, Zoom z);
	void            SetQTF(const char *qtf, Zoom z = Zoom(1, 1));
	const RichText& Get() const                               { return text; }
	String          GetQTF(byte cs = CHARSET_UTF8) const      { return AsQTF(text, cs); }

	int             GetWidth() const                          { return text.GetWidth(); }
	int             GetHeight(int cx) const                   { return text.GetHeight(Zoom(1, 1), cx); }
	int             GetCy() const;
	int             GetHeight() const /* obsolete */          { return GetHeight(GetSize().cx); }

	int             GetSb() const                             { return sb; }
	void            SetSb(int i)                              { sb = i; }
	void            ScrollInto(int pos)                       { sb.ScrollInto(pos); }
	int             GetSbTotal() const                        { return sb.GetTotal(); }
	Zoom            GetZoom() const;
	Rect            GetPage() const;

	bool            GotoLabel(const String& lbl, bool highlight = false);
	void            ClearHighlight()                          { highlight = Null; Refresh(); }

	int             GetLength() const                         { return text.GetLength(); }

	bool            IsSelection() const                       { return anchor != cursor; }
	void            Copy();

	void            ScrollUp()                                { sb.PrevLine(); }
	void            ScrollDown()                              { sb.NextLine(); }
	void            ScrollPageUp()                            { sb.PrevPage(); }
	void            ScrollPageDown()                          { sb.NextPage(); }
	void            ScrollEnd()                               { sb.End(); }
	void            ScrollBegin()                             { sb.Begin(); }

	RichTextView&   PageWidth(int cx);
	RichTextView&   SetZoom(Zoom z);
	RichTextView&   Background(Color _color);
	RichTextView&   TextColor(Color _color);
	RichTextView&   VCenter(bool b = true);
	RichTextView&   NoVCenter()                               { return VCenter(false); }
	RichTextView&   Margins(const Rect& m);
	RichTextView&   HMargins(int a);
	RichTextView&   VMargins(int a);
	RichTextView&   Margins(int a);
	RichTextView&   NoSb(bool b = true)                       { sb.AutoHide(!b); sb.Show(!b); return *this; }
	RichTextView&   AutoHideSb(bool b = true)                 { sb.AutoHide(b); return *this; }
	RichTextView&   HyperlinkDecoration(bool b = true)        { hldec = b; Refresh(); return *this; }
	RichTextView&   NoHyperlinkDecoration()                   { return HyperlinkDecoration(false); }
	RichTextView&   Lazy(bool b = true)                       { lazy = b; return *this; }
	RichTextView&   NoLazy()                                  { return Lazy(false); }
	RichTextView&   ShrinkOversizedObjects(bool b = true)     { shrink_oversized_objects = b; Refresh(); return *this; }
	RichTextView&   NoShrinkOversizedObjects()                { return ShrinkOversizedObjects(false); }

	void            operator=(const char *qtf)                { SetQTF(qtf); }

	RichTextView&   SetScrollBarStyle(const ScrollBar::Style& s) { sb.SetStyle(s); return *this; }

	typedef RichTextView CLASSNAME;

	RichTextView();
	virtual ~RichTextView();
};

class RichTextCtrl : public RichTextView {
public:
	virtual void  SetData(const Value& v);

public:
	void            SetQTF(const char *qtf, Zoom z = GetRichTextStdScreenZoom()) { RichTextView::SetQTF(qtf, z); }
	void            operator=(const char *qtf)                                   { SetQTF(qtf); }
	RichTextCtrl();
};

int Prompt(int dontshowagain, const char *opt_id,
           Event<const String&> WhenLink, int beep,
           const char *title, const Image& iconbmp, const char *qtf, bool okcancel,
           const char *button1, const char *button2, const char *button3,
		   int cx,
		   Image im1, Image im2, Image im3);

int Prompt(Event<const String&> WhenLink,
           const char *title, const Image& iconbmp, const char *qtf, bool okcancel,
           const char *button1, const char *button2, const char *button3,
		   int cx,
		   Image im1, Image im2, Image im3);

int Prompt(Event<const String&> WhenLink,
           const char *title, const Image& icon, const char *qtf, bool okcancel,
           const char *button1, const char *button2 = NULL, const char *button3 = NULL,
		   int cx = 0);
int Prompt(const char *title, const Image& icon, const char *qtf, bool okcancel,
           const char *button1, const char *button2 = NULL, const char *button3 = NULL,
		   int cx = 0);
int Prompt(const char *title, const Image& icon, const char *qtf,
           const char *button1, const char *button2 = NULL, const char *button3 = NULL,
		   int cx = 0);

enum { BEEP_NONE, BEEP_INFORMATION, BEEP_EXCLAMATION, BEEP_QUESTION, BEEP_ERROR };

int PromptOpt(const char *opt_id, int beep, Event<const String&> WhenLink,
              const char *title, const Image& icon, const char *qtf, bool okcancel,
              const char *button1, const char *button2, const char *button3,
		      int cx, Image im1, Image im2, Image im3);

int PromptOpt(const char *opt_id, int beep,
              const char *title, const Image& icon, const char *qtf, bool okcancel,
              const char *button1, const char *button2 = NULL, const char *button3 = NULL,
		      int cx = 0);

int PromptOpt(const char *opt_id, int beep,
              const char *title, const Image& icon, const char *qtf,
              const char *button1, const char *button2 = NULL, const char *button3 = NULL,
		      int cx = 0);

int PromptOpt1(const char *opt_id, int beep, Event<const String&> WhenLink,
               const char *title, const Image& icon, const char *qtf, bool okcancel,
               const char *button1, const char *button2, const char *button3,
		       int cx, Image im1, Image im2, Image im3);

int PromptOpt1(const char *opt_id, int beep,
               const char *title, const Image& icon, const char *qtf, bool okcancel,
               const char *button1, const char *button2 = NULL, const char *button3 = NULL,
		       int cx = 0);

int PromptOpt1(const char *opt_id, int beep,
               const char *title, const Image& icon, const char *qtf,
               const char *button1, const char *button2 = NULL, const char *button3 = NULL,
		       int cx = 0);

void PromptOK(const char *qtf);
int  PromptOKCancel(const char *qtf);
int  PromptOKCancelAll(const char *qtf);
int  PromptYesNo(const char *qtf);
int  PromptYesNoCancel(const char *qtf);
int  PromptYesNoAll(const char *qtf);
int  PromptRetryCancel(const char *qtf);
int  PromptAbortRetry(const char *qtf);
int  PromptAbortRetryIgnore(const char *qtf);
int  PromptSaveDontSaveCancel(const char *qtf);
void ShowExc(const Exc& exc);

void Exclamation(const char *qtf);

void ErrorOK(const char *qtf);
int  ErrorOKCancel(const char *qtf);
int  ErrorYesNo(const char *qtf);
int  ErrorYesNoCancel(const char *qtf);
int  ErrorYesNoAll(const char *qtf);
int  ErrorRetryCancel(const char *qtf);
int  ErrorAbortRetry(const char *qtf);
int  ErrorAbortRetryIgnore(const char *qtf);

void PromptOKOpt(const char *qtf, const char *opt_id = NULL);
int  PromptOKCancelOpt(const char *qtf, const char *opt_id = NULL);
int  PromptOKCancelAllOpt(const char *qtf, const char *opt_id = NULL);
int  PromptYesNoOpt(const char *qtf, const char *opt_id = NULL);
int  PromptYesNoCancelOpt(const char *qtf, const char *opt_id = NULL);
int  PromptYesNoAllOpt(const char *qtf, const char *opt_id = NULL);
int  PromptRetryCancelOpt(const char *qtf, const char *opt_id = NULL);
int  PromptAbortRetryOpt(const char *qtf, const char *opt_id = NULL);
int  PromptAbortRetryIgnoreOpt(const char *qtf, const char *opt_id = NULL);
int  PromptSaveDontSaveCancelOpt(const char *qtf, const char *opt_id = NULL);

void ExclamationOpt(const char *qtf, const char *opt_id = NULL);

void ErrorOKOpt(const char *qtf, const char *opt_id = NULL);
int  ErrorOKCancelOpt(const char *qtf, const char *opt_id = NULL);
int  ErrorYesNoOpt(const char *qtf, const char *opt_id = NULL);
int  ErrorYesNoCancelOpt(const char *qtf, const char *opt_id = NULL);
int  ErrorYesNoAllOpt(const char *qtf, const char *opt_id = NULL);
int  ErrorRetryCancelOpt(const char *qtf, const char *opt_id = NULL);
int  ErrorAbortRetryOpt(const char *qtf, const char *opt_id = NULL);
int  ErrorAbortRetryIgnoreOpt(const char *qtf, const char *opt_id = NULL);
void ShowExcOpt(const Exc& exc, const char *opt_id);

void PromptOKOpt1(const char *qtf, const char *opt_id = NULL);
int  PromptOKCancelOpt1(const char *qtf, const char *opt_id = NULL);
int  PromptOKCancelAllOpt1(const char *qtf, const char *opt_id = NULL);
int  PromptYesNoOpt1(const char *qtf, const char *opt_id = NULL);
int  PromptYesNoCancelOpt1(const char *qtf, const char *opt_id = NULL);
int  PromptYesNoAllOpt1(const char *qtf, const char *opt_id = NULL);
int  PromptRetryCancelOpt1(const char *qtf, const char *opt_id = NULL);
int  PromptAbortRetryOpt1(const char *qtf, const char *opt_id = NULL);
int  PromptAbortRetryIgnoreOpt1(const char *qtf, const char *opt_id = NULL);
int  PromptSaveDontSaveCancelOpt1(const char *qtf, const char *opt_id = NULL);

void ExclamationOpt1(const char *qtf, const char *opt_id = NULL);

void ErrorOKOpt1(const char *qtf, const char *opt_id = NULL);
int  ErrorOKCancelOpt1(const char *qtf, const char *opt_id = NULL);
int  ErrorYesNoOpt1(const char *qtf, const char *opt_id = NULL);
int  ErrorYesNoCancelOpt1(const char *qtf, const char *opt_id = NULL);
int  ErrorYesNoAllOpt1(const char *qtf, const char *opt_id = NULL);
int  ErrorRetryCancelOpt1(const char *qtf, const char *opt_id = NULL);
int  ErrorAbortRetryOpt1(const char *qtf, const char *opt_id = NULL);
int  ErrorAbortRetryIgnoreOpt1(const char *qtf, const char *opt_id = NULL);
void ShowExcOpt1(const Exc& exc, const char *opt_id);

Image YesButtonImage();
Image NoButtonImage();
Image AbortButtonImage();
Image RetryButtonImage();

void YesButtonImage_Write(Image m);
void NoButtonImage_Write(Image m);
void AbortButtonImage_Write(Image m);
void RetryButtonImage_Write(Image m);

void ClearPromptOptHistory();
void ClearPromptOptHistory(Gate<String> filter);
void SerializePromptOptHistory(Stream& s);

typedef
int (*RedirectPromptFn)(Event<const String&> WhenLink,
                        const char *title, const Image& iconbmp, const char *qtf, bool okcancel,
                        const char *button1, const char *button2, const char *button3,
                        int cx, Image im1, Image im2, Image im3);

void RedirectPrompts(RedirectPromptFn r);

class HelpWindow : public TopWindow {
public:
	virtual bool Key(dword key, int);

private:
	RichTextView   view;
	TreeCtrl       tree;
	Splitter       tree_view;
	ToolBar        toolbar;
	Zoom           zoom;

	struct Pos : Moveable<Pos> {
		String link;
		int    scy;
	};

	Vector<Pos>    back;
	Vector<Pos>    forward;
	Index<String>  tree_ndx;
	String         topic;
	String         label;
	String         current_link;

	bool GoTo0(const String& link);
	void Back();
	void Forward();
	void FontSize();
	void Tools(Bar& bar);
	void TreeCursor();
	void SetZoom();
	void Print();
	Pos  GetPos();
	Vector<int> ScPositions(const Vector<int>& p);
	void Ids(int pid, Vector<int>& r);
	Vector<int> Ids();
	bool PrevNext(int d, bool allowempty);

public:
	virtual Topic AcquireTopic(const String& topic);
	virtual void  FinishText(RichText& text);
	virtual void  BarEx(Bar& bar);

	bool GoTo(const String& link);

	void SetBar();
	void Serialize(Stream& s);

	void ClearTree();
	int  AddTree(int parent, const Image& img, const String& topic, const String& title);
	void SortTree(int id, int (*cmp)(const Value& k1, const Value& v1,
	                                 const Value& k2, const Value& v2));
	void SortTree(int id, int (*cmp)(const Value& v1, const Value& v2));
	void SortTree(int id);
	void FinishTree();
	void OpenDeep(int id = 0);
	void CurrentOrHome();
	void ClearCurrentLink()                       { current_link.Clear(); }
	bool Up(const Vector<int>& poslist);
	bool Down(const Vector<int>& poslist);
	bool Next(bool allowempty = false);
	bool Prev(bool allowempty = false);

	String GetCurrent() const                     { return topic; }
	String GetCurrentLabel() const                { return label; }
	String GetCurrentLink() const                 { return current_link; }

	typedef HelpWindow CLASSNAME;

	HelpWindow();
};

#ifndef PLATFORM_PDA

void Print(Draw& w, const RichText& text, const Rect& page, const Vector<int>& pagelist);
void Print(Draw& w, const RichText& text, const Rect& page);
bool Print(const RichText& text, const Rect& page, int currentpage, const char *name = NULL);

#endif
