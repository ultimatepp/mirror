#ifndef IDEEDITOR_H
#define IDEEDITOR_H

//#include <ide/Common/Common.h>
#include <CtrlLib/CtrlLib.h>
#include <plugin/pcre/Pcre.h>

namespace Upp {

#define  LAYOUTFILE <CodeEditor/CodeEditor.lay>
#include <CtrlCore/lay.h>

#define IMAGEVECTOR Vector
#define IMAGECLASS  CodeEditorImg
#define IMAGEFILE   <CodeEditor/CodeEditor.iml>
#include <Draw/iml_header.h>


void FindWildcardMenu(Callback1<const char *> cb, Point p, bool tablf, Ctrl *owner, bool regexp);

struct LineInfoRecord {
	int    lineno;
	String breakpoint;
	int    count;
	int    error;
	int    firstedited;
	int    edited;

	LineInfoRecord() { error = 0; edited = 0; }
};

typedef Array<LineInfoRecord> LineInfo;

void ClearErrors(LineInfo& li);

struct LineInfoRemRecord : Moveable<LineInfoRemRecord> {
	int    firstedited;
	int    edited;
};

typedef Vector<LineInfoRemRecord> LineInfoRem;

void Renumber(LineInfo& lf);
void ClearBreakpoints(LineInfo& lf);
void ValidateBreakpoints(LineInfo& lf);

class CodeEditor;

class EditorBar : public FrameLeft<Ctrl> {
public:
	virtual void Paint(Draw& w);
	virtual void MouseMove(Point p, dword flags);
	virtual void MouseLeave();
	virtual void LeftDown(Point p, dword flags);
	virtual void LeftDouble(Point p, dword flags);
	virtual void RightDown(Point p, dword flags);
	virtual void MouseWheel(Point p, int zdelta, dword keyflags);

private:
	struct LnInfo : Moveable<LnInfo> {
		int    lineno;
		String breakpoint;
		int    error;
		int    firstedited;
		int    edited;
		Image  icon;
		String annotation;

		LnInfo() { lineno = -1; error = 0; firstedited = 0; edited = 0; }
	};
	
	Vector<LnInfo>   li;
	LineInfoRem      li_removed;

	CodeEditor       *editor;
	int              ptrline[2];
	Image            ptrimg[2];
	bool             bingenabled;
	bool             hilite_if_endif;
	bool             line_numbers;
	int              annotations;
	bool             ignored_next_edit;
	int              next_age;
	int              active_annotation;

	String& PointBreak(int& y);
	void    sPaintImage(Draw& w, int y, int fy, const Image& img);

public:
	Event<int> WhenBreakpoint;
	Event<>    WhenAnnotationMove;
	Event<>    WhenAnnotationClick;
	Event<>    WhenAnnotationRightClick;

	void InsertLines(int i, int count);
	void RemoveLines(int i, int count);
	void ClearLines();

	void Scroll()                          { Refresh(); }

	void SyncSize();

	void Renumber(int linecount);
	void ClearBreakpoints();
	void ValidateBreakpoints();

	String  GetBreakpoint(int ln);
	void    SetBreakpoint(int ln, const String& s);
	void    SetEdited(int ln, int count = 1);
	void    ClearEdited();
	void    SetError(int ln, int err);
	void    ClearErrors(int ln);

	void SetEditor(CodeEditor *e)           { editor = e; }

	LineInfo GetLineInfo() const;
	void     SetLineInfo(const LineInfo& li, int total);
	LineInfoRem & GetLineInfoRem()                   { return li_removed; }
	void     SetLineInfoRem(LineInfoRem pick_ li)    { li_removed = pick(li); }
	
	void     ClearAnnotations();
	void     SetAnnotation(int line, const Image& img, const String& ann);
	String   GetAnnotation(int line) const;

	int      GetLineNo(int lineno) const;
	int      GetNoLine(int line) const;

	void     SetPtr(int line, const Image& img, int i);
	void     HidePtr();

	void     EnableBreakpointing(bool b)     { bingenabled = b; }
	void     HiliteIfEndif(bool b)           { hilite_if_endif = b; Refresh(); }
	void     LineNumbers(bool b);
	void     Annotations(int width);
	
	bool     IsHiliteIfEndif() const         { return hilite_if_endif; }
	
	int      GetActiveAnnotationLine() const { return active_annotation; }

	EditorBar();
	virtual ~EditorBar();
};

struct IdentPos {
	int    begin;
	int    end;
	String ident;
};

Array<IdentPos> GetLineIdent(const char *line);
Vector<Point>   GetLineString(const wchar *wline, bool& is_begin, bool& is_end);

inline int  CharFilterCIdent(int i)  { return IsAlNum(i) || i == '_' ? i : 0; }
inline bool iscidl(int c)            { return iscid(c) || IsLetter(c); }
inline bool islbrkt(int c)           { return c == '{' || c == '[' || c == '('; }
inline bool isrbrkt(int c)           { return c == '}' || c == ']' || c == ')'; }
inline bool isbrkt(int c)            { return islbrkt(c) || isrbrkt(c); }

struct FindReplaceDlg : FrameBottom< WithIDEFindReplaceLayout<TopWindow> > {
	WString itext;
	bool    replacing;

	virtual bool Key(dword key, int count);
	void Setup(bool doreplace);
	void Sync();
	bool IsIncremental() const              { return incremental.IsEnabled() && incremental; }
	
	typedef FindReplaceDlg CLASSNAME;

	FindReplaceDlg();
};

#include "Syntax.h"
#include "CSyntax.h"
#include "DiffSyntax.h"
#include "TagSyntax.h"
#include "LogSyntax.h"
#include "PythonSyntax.h"

class CodeEditor : public LineEdit, public HighlightSetup
{
	friend class EditorBar;

public:
	virtual bool  Key(dword code, int count);
	virtual void  LeftDown(Point p, dword keyflags);
	virtual void  LeftDouble(Point p, dword keyflags);
	virtual void  LeftTriple(Point p, dword keyflags);
	virtual void  LeftRepeat(Point p, dword keyflags);
	virtual void  MouseMove(Point p, dword keyflags);
	virtual Image CursorImage(Point p, dword keyflags);
	virtual void  Serialize(Stream& s);
	virtual void  MouseLeave();
	virtual void  MouseWheel(Point p, int zdelta, dword keyFlags);

protected:
	virtual void HighlightLine(int line, Vector<LineEdit::Highlight>& h, int64 pos);
	virtual void PreInsert(int pos, const WString& s);
	virtual void PostInsert(int pos, const WString& s);
	virtual void PreRemove(int pos, int size);
	virtual void PostRemove(int pos, int size);
	virtual void DirtyFrom(int line);
	virtual void SelectionChanged();

	virtual void ClearLines();
	virtual void InsertLines(int line, int count);
	virtual void RemoveLines(int line, int count);

	virtual void NewScrollPos();

	virtual String  GetPasteText();

	EditorBar   bar;
	Vector<int> line2;

	struct SyntaxPos {
		int    line;
		String data;
		
		void Clear() { line = 0; data.Clear(); }
	};
	
	SyntaxPos   syntax_cache[6];

//	EditorSyntax rm_ins;

	char    rmb;
	int64   highlight_bracket_pos0;
	int64   highlight_bracket_pos;
	bool    bracket_flash;
	int64   bracket_start;

	bool    barline;
	double  stat_edit_time;
	Time    last_key_time;

	bool    auto_enclose;
	bool    mark_lines;
	bool    check_edited;
	bool    persistent_find_replace;
	bool    do_ff_restore_pos;
	bool    withfindreplace;
	bool    wordwrap;

	int     ff_start_pos;

	FindReplaceDlg findreplace;
	
	enum {
		WILDANY = 16,
		WILDONE,
		WILDSPACE,
		WILDNUMBER,
		WILDID,
	};

	struct Found {
		int     type;
		WString text;
	};

	Array<Found> foundwild;
	WString      foundtext;
	bool   foundsel;
	bool   found, notfoundfw, notfoundbk;
	int64  foundpos;
	int    foundsize;
	
	enum { SEL_CHARS, SEL_WORDS, SEL_LINES };
	int    selkind;

	WString illuminated;

	String  iwc;

	String highlight;
	
	int    spellcheck_comments = 0;
	bool   wordwrap_comments = true;
	
	struct Tip : Ctrl {
		Value v;
		const Display *d;
		
		virtual void Paint(Draw& w);
		
		Tip();
	};
	
	Tip   tip;
	int   tippos;
	
	int   replacei;
	
	bool          search_canceled;
	int           search_time0;
	One<Progress> search_progress;
	
	String        refresh_info; // serialized next line syntax context to detect the need of full Refresh

	struct HlSt;
	
	bool   MouseSelSpecial(Point p, dword flags);
	void   InitFindReplace();
	void   CancelBracketHighlight(int64& pos);
	void   FindPrevNext(bool prev);
	void   CheckBrackets();
	void   OpenNormalFindReplace0(bool replace);
	void   OpenNormalFindReplace(bool replace);
	void   FindReplaceAddHistory();
	void   FindWildcard();
	void   ReplaceWildcard();
	void   InsertWildcard(const char *s);
	void   IncrementalFind();
	void   NotFound();
	void   NoFindError();
	void   CheckSyntaxRefresh(int64 pos, const WString& text);

	void   SetFound(int fi, int type, const WString& text);

	int     Match(const wchar *f, const wchar *s, int line, bool we, bool icase, int fi = 0);
	WString GetWild(int type, int& i);
	WString GetReplaceText();

	bool   InsertRS(int chr, int count = 1);

	void   IndentInsert(int chr, int count);

	void   ForwardWhenBreakpoint(int i);

	bool   ToggleSimpleComment(int &start_line, int &end_line, bool usestars = true);
	void   ToggleLineComments(bool usestars = false);
	void   ToggleStarComments();
	void   Enclose(const char *c1, const char *c2, int l = -1, int h = -1);
	void   Make(Event<String&> op);
	void   TabsOrSpaces(String& out, bool maketabs);
	void   LineEnds(String& out);

	enum {
		TIMEID_PERIODIC = Ctrl::TIMEID_COUNT,
		TIMEID_COUNT,
	};

	void   Periodic();

	void   StartSearchProgress(int64 l, int64 h);
	bool   SearchProgress(int line);
	bool   SearchCanceled();
	void   EndSearchProgress();

	String GetRefreshInfo(int pos);

public:
	struct MouseTip {
		int            pos;
		Value          value;
		const Display *display;
		Size           sz;
	};

	Event<>            WhenSelection;
	Gate<MouseTip&>    WhenTip;
	Event<>            WhenLeftDown;
	Event<int64>       WhenCtrlClick;
	Event<>            WhenAnnotationMove;
	Event<>            WhenAnnotationClick;
	Event<>            WhenAnnotationRightClick;
	Event<>            WhenOpenFindReplace;
	Event<String&>     WhenPaste;
	Event<>            WhenUpdate;
	Event<int>         WhenBreakpoint;

	Event<const Vector<Tuple<int64, int>>&> WhenFindAll;

	FrameTop<Button>    topsbbutton;
	FrameTop<Button>    topsbbutton1;

	static dword find_next_key;
	static dword find_prev_key;
	static dword replace_key;

	void   Clear();

	void   Highlight(const String& h);
	String GetHighlight() const       { return highlight; }

	void   EscapeFindReplace();
	void   CloseFindReplace();
	void   FindReplace(bool pick_selection, bool pick_text, bool replace);
	void   FindAll();
	bool   FindFrom(int64 pos, bool back, bool block);
	bool   RegExpFind(int64 pos, bool block);
	bool   Find(bool back, bool block);
	bool   Find(bool back, bool blockreplace, bool replace);
	void   FindNext();
	void   FindPrev();
	bool   GetStringRange(int64 cursor, int64& b, int64& e) const;
	bool   GetStringRange(int64& b, int64& e) const { return GetStringRange(GetCursor64(), b, e); }
	bool   FindString(bool back);
	bool   FindLangString(bool back);
	void   Replace();
	void   ReplaceAll(bool rest);
	int    BlockReplace();

	void   MakeTabsOrSpaces(bool tabs);
	void   MakeLineEnds();

	void   CopyWord();
	void   SwapChars();
	void   DuplicateLine();
	void   Put(int chr);
	void   FinishPut();

	void   SerializeFind(Stream& s);
	bool   IsFindOpen() const                       { return findreplace.IsOpen(); }
	void   FindClose()                              { CloseFindReplace(); }

	void   Goto();

	void   DoFind();
	void   DoFindBack();

//	void    FindWord(bool back);
	WString GetI();
	void    SetI(Ctrl *edit);
	void    PutI(WithDropChoice<EditString>& edit);

	void   MoveNextWord(bool sel);
	void   MovePrevWord(bool sel);
	void   MoveNextBrk(bool sel);
	void   MovePrevBrk(bool sel);

	String GetWord(int64 pos);
	String GetWord();

	bool   GetWordPos(int64 pos, int64& l, int64& h);

	void   DeleteWord();
	void   DeleteWordBack();
	void   SetLineSelection(int l, int h);
	bool   GetLineSelection(int& l, int& h);
	void   TabRight();
	void   TabLeft();
	void   SwapUpDown(bool up);

	void    CheckEdited(bool e = true)                { check_edited = e; }
	bool    GetCheckEdited()                          { return check_edited; }

	void    EditorBarLayout()                         { bar.SyncSize(); }

	LineInfo GetLineInfo() const                      { return bar.GetLineInfo(); }
	void     SetLineInfo(const LineInfo& lf);
	LineInfoRem GetLineInfoRem()                      { return LineInfoRem(bar.GetLineInfoRem(), 0); }
	void     SetLineInfoRem(LineInfoRem pick_  lf)    { bar.SetLineInfoRem(LineInfoRem(lf, 0)); }
	double   GetStatEditTime() const                  { return stat_edit_time; }
	void     Renumber()                               { bar.Renumber(GetLineCount()); }
	void     ClearBreakpoints()                       { bar.ClearBreakpoints(); }
	void     ValidateBreakpoints()                    { bar.ValidateBreakpoints(); }
	int      GetLineNo(int line) const                { return bar.GetLineNo(line); }
	int      GetNoLine(int line) const                { return bar.GetNoLine(line); }
	void     SetPtr(int line, const Image& img, int i){ bar.SetPtr(line, img, i); }
	void     HidePtr()                                { bar.HidePtr(); }
	String   GetBreakpoint(int line)                  { return bar.GetBreakpoint(line); }
	void     SetBreakpoint(int line, const String& b) { bar.SetBreakpoint(line, b); }
	void     SetError(int line, int err)              { bar.SetError(line, err); }
	void     ClearErrors(int line = -1)               { bar.ClearErrors(line); }
	void     ClearEdited()                            { bar.ClearEdited(); }
	int		 GetUndoCount()                           { return undo.GetCount(); }
	void     GotoLine(int line);
	void     EnableBreakpointing()                    { bar.EnableBreakpointing(true); }
	void     DisableBreakpointing()                   { bar.EnableBreakpointing(false); }
	void     Renumber2();
	int      GetLine2(int i) const;
	void     ReformatComment();

// TODO: Syntax: Remove
	void     HiliteScope(byte b)                      { EditorSyntax::hilite_scope = b; Refresh(); }
	void     HiliteBracket(byte b)                    { EditorSyntax::hilite_bracket = b; Refresh(); }
	void     HiliteIfDef(byte b)                      { EditorSyntax::hilite_ifdef = b; Refresh(); }
	void     HiliteIfEndif(bool b)                    { bar.HiliteIfEndif(b); }

	void     ThousandsSeparator(bool b)               { thousands_separator = b; Refresh(); }
	void     IndentSpaces(bool is)                    { indent_spaces = is; }
	void     IndentAmount(int ia)                     { indent_amount = ia; }
	void     NoParenthesisIndent(bool b)              { no_parenthesis_indent = b; }
	
	void     SpellcheckComments(int lang)             { spellcheck_comments = lang; Refresh(); }
	int      GetSpellcheckComments() const            { return spellcheck_comments; }
	void     WordwrapComments(bool b)                 { wordwrap_comments = b; }
	bool     IsWordwrapComments() const               { return wordwrap_comments; }

	void     NoFindReplace()                          { withfindreplace = false; }

	void     LineNumbers(bool b)                      { bar.LineNumbers(b); }
	void     MarkLines(bool b)                        { mark_lines = b; }
	bool     GetMarkLines()                           { return mark_lines; }
	void     AutoEnclose(bool b)                      { auto_enclose = b; }
	void     BarLine(bool b)                          { barline = b; }
	void     WordWrap(bool b)                         { wordwrap = b; }
	
	void     PersistentFindReplace(bool b = true)     { persistent_find_replace = b; }
	bool     IsPersistentFindReplace() const          { return persistent_find_replace; }

	void     FindReplaceRestorePos(bool b = true)     { do_ff_restore_pos = b; }
	bool     IsFindReplaceRestorePos() const          { return do_ff_restore_pos; }
	
	void     Annotations(int width)                   { bar.Annotations(width); }
	void     ClearAnnotations()                       { bar.ClearAnnotations(); }
	void     SetAnnotation(int i, const Image& icon, const String& a) { bar.SetAnnotation(i, icon, a); }
	String   GetAnnotation(int i) const               { return bar.GetAnnotation(i); }
	int      GetActiveAnnotationLine() const          { return bar.GetActiveAnnotationLine(); }

	void     HideBar()                                { bar.Hide(); }

	void     SyncTip();
	void     CloseTip()                               { if(tip.IsOpen()) tip.Close(); tip.d = NULL;  }
	
	void     Illuminate(const WString& text);
	WString  GetIlluminated() const                   { return illuminated; }

	void     Zoom(int d);

	One<EditorSyntax> GetSyntax(int line);
	bool IsCursorBracket(int64 pos) const;
	bool IsMatchingBracket(int64 pos) const;

// TODO: Do we really need this ?
	Vector<IfState> GetIfStack(int line)              { return GetSyntax(line)->PickIfStack(); }

	struct FindReplaceData {
		String find, replace;
		String find_list, replace_list;
		bool   wholeword, wildcards, ignorecase, samecase, regexp;
	};
	
	FindReplaceData GetFindReplaceData();
	void            SetFindReplaceData(const FindReplaceData& d);

	typedef CodeEditor CLASSNAME;

	CodeEditor();
	virtual ~CodeEditor();

	static void InitKeywords();
};

String ReadList(WithDropChoice<EditString>& e);
void   WriteList(WithDropChoice<EditString>& e, const String& data);

}

#endif
