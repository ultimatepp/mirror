class Pusher : public Ctrl {
public:
	virtual void   CancelMode();
	virtual void   LeftDown(Point, dword);
	virtual void   MouseMove(Point, dword);
	virtual void   MouseLeave();
	virtual void   LeftRepeat(Point, dword);
	virtual void   LeftUp(Point, dword);
	virtual void   GotFocus();
	virtual void   LostFocus();
	virtual void   State(int);
	virtual String GetDesc() const;
	virtual bool   Key(dword key, int);
	virtual bool   HotKey(dword key);
	virtual dword  GetAccessKeys() const;
	virtual void   AssignAccessKeys(dword used);

private:
	bool    push:1;
	bool    keypush:1;
	bool    clickfocus:1;

	void    EndPush();

protected:
	int     accesskey;
	String  label;
	Font    font;

	void    KeyPush();
	bool    IsPush() const                                  { return push || keypush; }
	bool    IsKeyPush()                                     { return keypush; }
	bool    FinishPush();

protected:
	virtual void  RefreshPush();
	virtual void  RefreshFocus();
	virtual void  PerformAction();

public:
	Pusher&  SetFont(Font fnt);
	Pusher&  SetLabel(const char *text);
	Pusher&  ClickFocus(bool cf = true);
	Pusher&  NoClickFocus()                                 { return ClickFocus(false); }
	bool     IsClickFocus() const                           { return clickfocus; }
	
	Font     GetFont() const                                { return font; }
	String   GetLabel() const                               { return label; }

	void     PseudoPush();

	int      GetVisualState() const;

	Event<>  WhenPush;
	Event<>  WhenRepeat;

	Pusher();
	virtual ~Pusher();
};

class Button : public Pusher {
public:
	virtual void   Paint(Draw& draw);
	virtual bool   Key(dword key, int);
	virtual bool   HotKey(dword key);
	virtual void   MouseEnter(Point, dword);
	virtual void   MouseLeave();
	virtual dword  GetAccessKeys() const;
	virtual void   AssignAccessKeys(dword used);
	virtual void   Layout();
	virtual void   GotFocus();
	virtual void   LostFocus();
	virtual int    OverPaint() const;

public:
	struct Style : ChStyle<Style> {
		Value look[4];
		Color monocolor[4], textcolor[4];
		Point pressoffset;
		int   focusmargin;
		int   overpaint;
		Font  font;
		Image ok, cancel, exit;
		bool  transparent;
		bool  focus_use_ok;
	};

protected:
	enum { NORMAL, OK, CANCEL, EXIT };
	const Style *style;
	Image   img;
	bool    monoimg;
	byte    type;

	void  RefreshOK(Ctrl *p);
	const Style *St() const;

public:
	Button&  SetImage(const Image& img);
	Button&  SetMonoImage(const Image& img);

	static const Style& StyleNormal();
	static const Style& StyleOk();
	static const Style& StyleEdge();
	static const Style& StyleLeftEdge();
	static const Style& StyleScroll();
	static const Style& StyleNaked();

	Button&  SetStyle(const Style& s);
	Button&  AutoStyle();

	Button&  NormalStyle()                        { return SetStyle(StyleNormal()); }
	Button&  EdgeStyle()                          { return SetStyle(StyleEdge()); }
	Button&  LeftEdgeStyle()                      { return SetStyle(StyleLeftEdge()); }
	Button&  ScrollStyle()                        { return SetStyle(StyleScroll()); }
	Button&  NakedStyle()                         { return SetStyle(StyleNaked()); }

	Button&  Ok();
	Button&  Cancel();
	Button&  Exit();
	Button&  Normal()                             { type = NORMAL; Refresh(); return *this; }

	Button();
	virtual ~Button();
};

Color ButtonMonoColor(int i);

class SpinButtons : public CtrlFrame {
public:
	virtual void FrameLayout(Rect& r);
	virtual void FrameAddSize(Size& sz);
	virtual void FrameAdd(Ctrl& ctrl);
	virtual void FrameRemove();

public:
	struct Style : ChStyle<Style> {
		Button::Style inc;
		Button::Style dec;
		int           width;
		int           over;
		bool          onsides;
	};

private:
	bool         visible;
	const Style *style;

public:
	Button inc;
	Button dec;

	void         Show(bool s = true);
	bool         IsVisible() const          { return visible; }

	static const Style& StyleDefault();
	static const Style& StyleOnSides();

	SpinButtons& SetStyle(const Style& s);
	
	SpinButtons& OnSides(bool b = true)     { return SetStyle(b ? StyleOnSides() : StyleDefault()); }
	bool         IsOnSides() const          { return style->onsides; }

	SpinButtons();
	virtual ~SpinButtons();
};

class Option : public Pusher {
public:
	virtual void   Paint(Draw& draw);
	virtual Size   GetMinSize() const;
	virtual void   SetData(const Value& data);
	virtual Value  GetData() const;
	virtual void   MouseEnter(Point, dword);
	virtual void   MouseLeave();
	virtual void   State(int);

protected:
	virtual void  RefreshPush();
	virtual void  RefreshFocus();
	virtual void  PerformAction();

protected:
	Image  edge, edged;
	int    option;
	bool   switchimage;
	bool   threestate;
	bool   notnull;
	bool   blackedge;
	bool   showlabel;
	bool   box;
	bool   autobox;
	Color  color;
	
	void   AutoSync();

public:
	Option& Set(int b);
	int     Get() const                           { return option; }

	operator int() const                          { return option; }
	void operator=(int b)                         { Set(b); }
	
	void    EnableBox(bool b);
	void    EnableBox()                           { EnableBox(option); }

	Option& BlackEdge(bool b = true)              { blackedge = b; Refresh(); return *this; }
	bool    IsBlackEdge() const                   { return blackedge; }
	Option& SwitchImage(bool b = true)            { switchimage = b; Refresh(); return *this; }
	bool    IsSwitchImage() const                 { return switchimage; }
	Option& ThreeState(bool b = true)             { threestate = b; notnull = false; return *this; }
	bool    IsThreeState() const                  { return threestate; }
	Option& ShowLabel(bool b = true)              { showlabel = b; Refresh(); return *this; }
	bool    IsShowLabel() const                   { return showlabel; }
	Option& NotNull(bool nn = true)               { notnull = nn; Refresh(); return *this; }
	Option& NoNotNull()                           { return NotNull(false); }
	bool    IsNotNull() const                     { return notnull; }
	Option& SetColor(Color c)                     { color = c; Refresh(); return *this; }
	Option& Box(bool b = true)                    { box = b; return *this; }
	Option& AutoBox(bool b = true)                { Box(autobox = b); return *this; }

	Option();
	virtual ~Option();
};

class OptionBox : public Option {
public:
	OptionBox() { Box(); }
};

class ButtonOption : public Ctrl {
public:
	virtual void  Paint(Draw& w);
	virtual void  LeftDown(Point, dword);
	virtual void  LeftUp(Point, dword);
	virtual void  MouseMove(Point, dword);
	virtual void  MouseEnter(Point, dword);
	virtual void  MouseLeave();
	virtual void  SetData(const Value& v);
	virtual Value GetData() const;
	virtual void  Serialize(Stream& s);
	virtual dword GetAccessKeys() const;
	virtual void  AssignAccessKeys(dword used);

public:
	struct Style : ChStyle<Style> {
		Value look[4];
		Color textcolor[4];
		bool  drawfocus;
	};

private:
	Image        image;
	Image        image1;
	String       label;
	const Style *style;
	int          accesskey;
	bool         option;
	bool         push;

public:
	ButtonOption&  SetImage(const Image& img)                 { image = img; Refresh(); return *this; }
	ButtonOption&  SetImage(const Image& m, const Image& m1)  { image = m; image1 = m1; Refresh(); return *this; }
	void operator=(const Image& img)                          { SetImage(img); }
	
	ButtonOption& SetLabel(const String& text);
	String GetLabel() const                                   { return label; }
	
	void Set(bool b)                                          { option = b; UpdateRefresh(); }
	bool Get() const                                          { return option; }

	void operator=(bool b)                                    { Set(b); }
	operator bool() const                                     { return Get(); }

	static const Style& StyleDefault();
	static const Style& StyleFlat();
	
	ButtonOption& SetStyle(const Style& s)                    { style = &s; Refresh(); return *this; }
	ButtonOption& AutoStyle()                                 { style = NULL; Refresh(); return *this; }

	ButtonOption();
};

class Switch : public Ctrl {
public:
	virtual void   Paint(Draw& draw);
	virtual void   CancelMode();
	virtual void   MouseMove(Point p, dword keyflags);
	virtual void   LeftDown(Point p, dword keyflags);
	virtual void   LeftUp(Point p, dword keyflags);
	virtual void   MouseLeave();
	virtual bool   Key(dword key, int count);
	virtual bool   HotKey(dword key);
	virtual dword  GetAccessKeys() const;
	virtual void   AssignAccessKeys(dword used);
	virtual void   SetData(const Value& data);
	virtual Value  GetData() const;
	virtual void   GotFocus();
	virtual void   LostFocus();

public:
	struct Case  {
		String label;
		Value  value;
		int    accesskey = 0;
		bool   enabled = true;
		int    gap = 0;
		Rect16 rect = Rect16(0, 0, 0, 0);
	};

private:
	Font         font;
	Value        value;
	int          pushindex;
	Array<Case>  cs;
	int          linecy;
	int          light;
	int          mincy;
	int          direction;

	int   GetIndex() const;
	int   GetIndex(Point p);
	bool  DoHot(dword key);
	void  Updates();
	void  RefreshCase(int i);

	Rect    GetCaseRect(int i) const;
	Rect    GetCheckRect(int i) const;

public:
	enum { GAP_SEPARATOR = 1 << 20 };

	Switch& SetLabel(int i, const char *text, int gap = 0);
	Switch& SetLabel(const char *text);
	String  GetLabel() const                                    { return GetLabel(GetIndex()); }
	String  GetLabel(int i) const                               { return cs[i].label; }
	Switch& Set(int i, const Value& val, const char *text, int gap = 0);
	Switch& Set(int i, const Value& val);
	Switch& Add(const Value& val, const char *text, int gap = 0);
	Switch& Add(const char *text, int gap = 0);

	void    EnableCase(int i, bool enable = true);
	void    DisableCase(int i)                                  { EnableCase(i, false); }

	void    EnableValue(const Value& val, bool enable = true);
	void    DisableValue(const Value& val)                      { EnableValue(val, false); }

	void  Reset()                                               { cs.Clear(); }

	const Array<Case>& GetCases() const                         { return cs; }

	operator int() const                                        { return GetData(); }
	void operator=(const Value& v)                              { SetData(v); }

	Switch& SetFont(Font f)                                     { font = f; Refresh(); return *this; }
	Font    GetFont() const                                     { return font; }
	Switch& MinCaseHeight(int cy)                               { mincy = cy; Refresh(); return *this; }
	Switch& SetAutoDirection()                                  { direction = 0; return *this; }
	Switch& SetHorizontal()                                     { direction = 1; return *this; }
	Switch& SetVertical()                                       { direction = -1; return *this; }

	Switch();
	virtual ~Switch();
};

class DataPusher : public Pusher
{
public:
	virtual void   Paint(Draw& draw);
	virtual Value  GetData() const;
	virtual void   SetData(const Value& value);

private:
	const Convert *convert;
	const Display *display;
	Value          data;

	WString        nulltext;
	Color          nullink;
	Font           nullfont;
	
	ActiveEdgeFrame edge;
	
	void  RefreshAll();
	Color GetPaper();
	
protected:
	virtual void   RefreshPush();
	virtual void   RefreshFocus();
	virtual void   PerformAction();
	virtual void   DoAction();

public:
	Event<>        WhenPreAction;

	DataPusher&    SetConvert(const Convert& _convert) { convert = &_convert; Refresh(); return *this; }
	const Convert& GetConvert() const                  { return *convert; }

	DataPusher&    SetDisplay(const Display& _display) { display = &_display; Refresh(); return *this; }
	const Display& GetDisplay() const                  { return *display; }

	void           SetDataAction(const Value& value);

	void           Set(const Value& value);

	DataPusher&    NullText(const char *text = t_("(default)"), Color ink = Brown);
	DataPusher&    NullText(const char *text, Font fnt, Color ink);

	DataPusher();
	DataPusher(const Convert& convert, const Display& display = StdDisplay()); // deprecated
	DataPusher(const Display& display); // deprecated
};
