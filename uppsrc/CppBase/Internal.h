#ifndef _CppBase_Internal_h_
#define _CppBase_Internal_h_

namespace Upp {
	
const char *SkipString(const char *s);

void RemoveComments(String& l, bool& incomment);

const VectorMap<String, String>& GetNamespaceMacros();
const Index<String>&             GetNamespaceEndMacros();

struct CppMacro : Moveable<CppMacro> {
	String        param;
	String        body;
	byte          md5[16];
	
	String Define(const char *s);
	void   SetUndef()                { body = "\x7f"; }
	bool   IsUndef() const           { return body[0] == '\x7f' && body[1] == '\0'; }

	String Expand(const Vector<String>& p, const Vector<String>& ep) const;

	void   Serialize(Stream& s);
	
	String ToString() const;
};

enum PPItemType {
	PP_DEFINES,
	PP_INCLUDE,
	PP_USING,
	PP_NAMESPACE,
	PP_NAMESPACE_END
};

struct PPItem {
	int      type;
	String   text;
	int      segment_id;
	
	void     Serialize(Stream& s) { s % type % text % segment_id; }
};

struct PPMacro : Moveable<PPMacro> {
	CppMacro  macro;
	int       segment_id;        // a group of macros in single file, between other elements (include, namespace. using, undef...)
	int       line;              // line in file
	int       undef_segment_id;  // macro has matching undef in the same file within this segment
	
	void   Serialize(Stream& s) { s % macro % segment_id % line % undef_segment_id; }
	String ToString() const     { return AsString(macro) + " " + AsString(segment_id); }
	
	PPMacro()                   { segment_id = undef_segment_id = 0; }
};

struct PPFile { // contains "macro extract" of file, only info about macros defined and namespaces
	Time           filetime;
	Array<PPItem>  item;
	Index<String>  includes;
	Vector<String> keywords;
	String         md5sum;

	void Parse(Stream& in);
	void Serialize(Stream& s) { s % filetime % item % includes % keywords % md5sum; }
	void Dump() const;

private:
	Vector<int>    ppmacro;   // indicies of macros in sAllMacros

	void CheckEndNamespace(Vector<int>& namespace_block, int level, Md5Stream& md5);
};

PPMacro            *FindPPMacro(const String& id, Index<int>& segment_id, int& segmenti);
const     CppMacro *FindMacro(const String& id, Index<int>& segment_id, int& segmenti);

String GetIncludePath();

String GetSegmentFile(int segment_id);

void  MakePP(const Index<String>& paths); // this is the only place to change PPFile info, cannot be run concurrently with anything else

const PPFile& GetPPFile(const char *path); // only returns information created by MakePP

String GetIncludePath(const String& s, const String& filedir);
bool   IncludesFile(const String& parent_path, const String& header_path);

struct FlatPP {
	Index<int>    segment_id;
	Index<String> usings;
};

const FlatPP& GetFlatPPFile(const char *path); // with #includes resolved

struct Cpp {
	static Index<String>        kw;

	bool                        incomment;
	bool                        done;
	
	Index<String>               visited;

	Index<int>                  segment_id; // segments of included macros
	VectorMap<String, PPMacro>  macro; // macros defined
	int                         std_macros; // standard macros (keywords and trick - fixed)
	Index<String>               notmacro; // accelerator / expanding helper
	String                      prefix_macro; // for handling multiline macros

	String                      output; // preprocessed file
//	Index<String>               usedmacro;
	Index<String>               namespace_using; // 'using namespace' up to start of file
	Vector<String>              namespace_stack; // namspace up to start of file
	
	Md5Stream                   md5;
	
	void   Define(const char *s);

	void   ParamAdd(Vector<String>& param, const char *b, const char *e);
	String Expand(const char *s);
	void   DoFlatInclude(const String& header_path);
	void   Do(const String& sourcefile, Stream& in, const String& currentfile, bool get_macros);

	bool   Preprocess(const String& sourcefile, Stream& in, const String& currentfile,
	                  bool just_get_macros = false);

	String GetDependeciesMd5(const Vector<String>& m);
	
	typedef Cpp CLASSNAME;
};

enum tk_Keywords {
	Tmarker_before_first = 255,
#define CPPID(x)   tk_##x,
#include "keyword.i"
#undef  CPPID
};

enum t_Terms {
	t_eof,
	t_string = -200,
	t_integer,
	t_double,
	t_character,
	t_dblcolon,       // ::
	t_mulass,         // *=
	t_divass,         // /=
	t_modass,         // %=
	t_xorass,         // ^=
	t_neq,            // <>
	t_dot_asteriks,   // .*
	t_elipsis,        // ...
	t_inc,            // ++
	t_addass,         // +=
	t_dec,            // -
	t_arrow_asteriks, // ->*
	t_arrow,          // ->
	t_subass,         // -=
	t_and,            // &
	t_andass,         // &=
	t_or,             // |
	t_orass,          // |=
	t_eq,             // ==
	t_shl,            // <<
	t_shlass,         // <<=
	t_le,             // <=
	t_shr,            // >>
	t_shrass,         // >>=
	t_ge,             // >=
	te_integeroverflow,
	te_badcharacter,
	te_badstring,
};

class LexSymbolStat
{
public:
	LexSymbolStat();
	void Reset(int minSymbol, int maxSymbol);
	void IncStat(int symbol);
	int  GetStat(int symbol) const;
	int  SumStat(const Vector<int> & symbols) const;
	void Merge(const LexSymbolStat & other);

private:
	Vector<int> v;
	int minSymbol;
};


class Lex {
#ifdef _DEBUG
	const char *pp;
#endif
	const char *ptr;
	const char *pos;

	Index<String> id;
	int           endkey;
	int           braceslevel;

	struct Term  : Moveable<Term>{
		const  char *ptr;
		int    code;
		String text;
		double number;
		bool   grounding;
		
		Term() { grounding = false; }
	};

	bool statsCollected;
	LexSymbolStat symbolStat;
	BiVector<Term> term;
	int            body;
	bool           grounding;

	bool Char(int c)                 { if(*ptr == c) { ptr++; return true; } else return false; }
	void AddCode(int code)           { Term& tm = term.AddTail(); tm.code = code; tm.ptr = pos; tm.grounding = grounding; }
	void AssOp(int noass, int ass)   { AddCode(Char('=') ? ass : noass); }
	void Next();
	bool Prepare(int pos);
	int  GetCharacter();
	void ThrowError(const char *e);

public:
	Event<const String&> WhenError;

	struct Grounding {};

	int         Code(int pos = 0);
	bool        IsId(int pos = 0);
	String      Id(int pos = 0);
	int         Int(int pos = 0);
	double      Double(int pos = 0);
	int         Chr(int pos = 0);
	String      Text(int pos = 0);

	void        Get(int n = 1);
	int         GetCode()                   { int q = Code(); Get(); return q; }
	String      GetId()                     { String q = Id(); Get(); return q; }
	int         GetInt()                    { int q = Int(); Get(); return q; }
	double      GetDouble()                 { double q = Double(); Get(); return q; }
	int         GetChr()                    { int q = Chr(); Get(); return q; }
	String      GetText()                   { String q = Text(); Get(); return q; }

	int         Id(const String& s)         { return id.FindAdd(s) + 256; }

	int         GetBracesLevel() const      { return braceslevel; }
	void        ClearBracesLevel()          { braceslevel = 0; }
	
	void        BeginBody()                 { body++; }
	void        EndBody()                   { body--; }
	void        ClearBody()                 { body = 0; }
	bool        IsBody() const              { return body; }
	bool        IsGrounded()                { Code(); return term.GetCount() && term[0].grounding; }
	void        SkipToGrounding();

	const char *Pos(int pos = 0);
	int         operator[](int pos)         { return Code(pos); }
	operator    int()                       { return Code(0); }
	void        operator++()                { return Get(); }
	
	void        Dump(int pos);

	void        Init(const char *s);
	void        StartStatCollection();
	const LexSymbolStat & FinishStatCollection();

	Lex();
};

struct SrcFile {
	SrcFile();
	rval_default(SrcFile);

	String      text;
	Vector<int> linepos;
	int preprocessorLinesRemoved;
	int blankLinesRemoved;
	int commentLinesRemoved;
};

struct Parser;

SrcFile PreProcess(Stream& in, Parser& parser);

String CppItemKindAsString(int kind);

struct Parser : ParserContext {
	struct Decla {
		bool    s_static = false;
		bool    s_extern = false;
		bool    s_register = false;
		bool    s_mutable = false;
		bool    s_explicit = false;
		bool    s_virtual = false;
		String  name;
		bool    function = false;
		bool    type_def = false;
		bool    isfriend = false;
		bool    istemplate = false;
		bool    istructor = false;
		bool    isdestructor = false;
		bool    isptr = false;
		bool    nofn = false;
		bool    oper = false;
		bool    castoper = false;

		String  tnames;
		String  type;
		String  natural;
	};

	struct Decl : Decla {
		Array<Decl> param;
		
		rval_default(Decl);
		Decl() {}
	};

	struct RecursionCounter
	{
		int change;
		int &count;
		RecursionCounter(int &count_i, int change_i = 1) :
		  change(change_i), count(count_i)
		{
		  count += change;
		}
		~RecursionCounter() { count -= change; }
	};

	SrcFile     file;
	Lex         lex;
	int         filei;
	byte        filetype;
	String      title;
	int         struct_level;

	Event<int, const String&> err;

	int     lpos, line;

	CppBase *base;

	int    RPtr();

	bool   Key(int code);
	bool   EatBody();
	String ResolveAutoType();
	void   TryLambda();
	bool   Skipars(int& q);

	void   Cv();
	String TType();
	String ReadType(Decla& d, const String& tname, const String& tparam);
	void   Qualifier(bool override_final = false);
	void   ParamList(Decl& d);
	void   Declarator(Decl& d, const char *p);
	void   EatInitializers();
	void   Vars(Array<Decl>& r, const char *p, bool type_def, bool more);
	void   ReadMods(Decla& d);
	Array<Decl> Declaration0(bool l0, bool more, const String& tname, const String& tparam);
	Array<Decl> Declaration(bool l0/* = false*/, bool more/* = false*/, const String& tname, const String& tparam);
	bool   IsParamList(int q);
	void   Elipsis(Decl& d);
	Decl&  Finish(Decl& d, const char *p);
	void   AddNamespace(const String& n, const String& name);
	bool   Scope(const String& tp, const String& tn);

	String TemplateParams(String& pnames);
	String TemplateParams();
	String TemplatePnames();
	String Name(String& h, bool& castoper, bool& oper);
	String Name(bool& castoper, bool& oper);
	String Constant();
	String ReadOper(bool& castoper);

	int    GetLine(const char *pos);
	void   Line();
	void   Check(bool b, const char *err);
	void   CheckKey(int c);

	void   ClassEnum(const String& clss);
	bool   IsEnum(int i);
	bool   UsingNamespace();
	void   SetScopeCurrent();
	void   ScopeBody();
	void   DoNamespace();
	void   Do();
	String AnonymousName();
	String StructDeclaration(const String& tp, const String& tn);
	void   Enum(bool vars);

	CppItem& Item(const String& scope, const String& using_namespace, const String& item,
	              const String& name, bool impl);
	CppItem& Item(const String& scope, const String& using_namespace, const String& item,
	              const String& name);

	CppItem& Fn(const Decl& d, const String& templ, bool body,
	            const String& tname, const String& tparam);

	struct Error {};

	void   ThrowError(const String& e);
	void   Resume(int bl);

	void   MatchPars();
	bool   VCAttribute();
	bool   TryDecl();
	void   Statement();
	void   Locals(const String& type);
	String Tparam(int& q);
	bool   IsNamespace(const String& scope);
	
	friend class Lex; // Fix to make Lex::ThrowError

	typedef Parser CLASSNAME;

public:
	void AddMacro(int lineno, const String& macro);

	bool                      dobody;
	Function<String(String, String, String)> qualify; // used to qualify local variable names (needs main codebase and its mutex)
	int                       currentScopeDepth;
	int                       maxScopeDepth;
	
	String                    namespace_info;

	const SrcFile &getPreprocessedFile() { return file; }

	void  Do(Stream& in, CppBase& _base, int file, int filetype,
	         const String& title, Event<int, const String&> _err,
	         const Vector<String>& typenames,
	         const Vector<String>& namespace_stack,
	         const Index<String>& namespace_using);

	Parser() : dobody(false) { lex.WhenError = THISBACK(ThrowError); }
};

void   QualifyTypes(CppBase& base, const String& scope, CppItem& m);
String QualifyKey(const CppBase& base, const String& scope, const String& type, const String& usings);

const Array<CppItem>& GetTypeItems(const CppBase& codebase, const String& type);

Vector<String> MakeXP(const char *s);

};

#endif
