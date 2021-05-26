#include "CppBase.h"
#include "Internal.h"

namespace Upp {

#define LLOG(x)    // DLOG(x)
#define LTIMING(x) // DTIMING(x)

void Parser::ThrowError(const String& e)
{
	err(GetLine(lex.Pos()), e);
#ifdef _DEBUG
	int i = 0;
	while(i < 40 && lex[i] != t_eof)
		i++;
	LLOG("ERROR: (" << GetLine(lex.Pos()) << ") " << e << ", scope: " << current_scope <<
	     ", code:  " << AsCString(String(lex.Pos(), lex.Pos(i))));
#endif
	throw Error();
}

inline const char *bew(const char *s, const char *t)
{
	while(*s)
		if(*s++ != *t++)
			return NULL;
	return t;
}

// String == String comparison is likely faster than String == const char * comparison
static String s_operator("operator");
static String s_virtual("virtual");
static String s_inline("inline");
static String s_static("static");

static inline bool sSpaces(String& res, const char *& s)
{
	if((byte)*s <= ' ') {
		char c = *s++;
		if(c != '\2' && c != '\x1f') {
			res.Cat(' ');
			while((byte)*s <= ' ' && *s)
				s++;
		}
		return true;
	}
	return false;
}

String FnItem(const char *s, const char *pname, const char *qname, const String& name, bool oper)
{ // Converts function natural text to (unqualified) item
	String res;
	while(*s && (byte)*s <= ' ') s++;
	while(*s) { // Get the name of function into res
		while(*s && !iscid(*s) && *s != '~')
			if(*s == '[') { // Skip MSVC attributes
				while(*s)
					if(*s++ == '[')
						break;
			}
			else
				s++;
		int lvl = 0;
		int plvl = 0;
		for(;;) {
			if(*s == '<' && plvl == 0 && !oper) { // resolve template params, like Fn<int, true>
				res.Cat(*s++);
				lvl++;
			}
			if(*s == '>' && plvl == 0 && !oper) {
				res.Cat(*s++);
				lvl--;
			}
			if(*s == '(' && lvl) {
				res.Cat(*s++);
				plvl++;
			}
			if(*s == ')') {
				res.Cat(*s++);
				plvl--;
			}
			if(iscid(*s) || *s == '~' || *s && lvl)
				res.Cat(*s++);
			else
				break;
		}
		if(res == s_operator) {
			while(*s && *s != '(') {
				if((byte)*s >= ' ')
					res.Cat(*s);
				s++;
			}
			break;
		}
		while(*s && (byte)*s <= ' ')
			s++;
		if(*s == '(' && (res == name || res == '~' + name))
			break;
		res.Clear();
	}
	bool wasid = false;
	while(*s) {
		const char *w = bew(qname, s);
		byte c = *s;
		if(w && w > s && !iscid(*w)) {
			if(iscid(*res.Last()))
				res.Cat(' ');
			res.Cat(name);
			s = w;
			wasid = true;
		}
		else
		if(iscid(c)) {
			const char *b = s++;
			while(iscid(*s)) s++;
			String q(b, s);
			if(q != s_virtual && q != s_inline && q != s_static && !InScList(q, pname)) {
				if(iscid(*res.Last()))
					res.Cat(' ');
				res.Cat(q);
			}
			else
				while((byte)*s <= ' ' && *s) s++;
			wasid = true;
		}
		else
		if(c == '=') {
			s++;
			int l = 0;
			int tl = 0;
			while(*s && !(l == 0 && (*s == ',' && tl == 0 || *s == ')'))) {
				if(*s == '(' || *s == '[')
					l++;
				else
				if(*s == ')' || *s == ']')
					l--;
				if(*s == '<') // we always consider < > to be template bracket, not operator here
					tl++;
				else
				if(*s == '>')
					tl--;
				s++;
			}
		}
		else
		if((byte)*s <= ' ') {
			s++;
			while((byte)*s <= ' ' && *s)
				s++;
		}
		else
		if(c == '[' && !wasid) { // Skip MSVC attribute
			while(*s)
				if(*s++ == ']')
					break;
		}
		else
		if(c == '-' && s[1] == '>')
			break; // trailing return type
		else {
			res.Cat(c);
			if(c == ',')
				wasid = false;
			s++;
		}
	}
	return res;
}

String Purify(const char *s, const char *qname, const String& name) {
	String res;
	while(*s && (byte)*s <= ' ') s++;
	bool wasid = false;
	bool firstpar = true;
	while(*s) {
		const char *w = bew(qname, s);
		if(w && w > s) {
			res.Cat(name);
			s = w;
			wasid = true;
		}
		else
		if(iscid(*s)) {
			const char *b = s++;
			while(iscid(*s)) s++;
			String q(b, s);
			if(q != s_virtual && q != s_inline && q != s_static)
				res.Cat(q);
			else
				while((byte)*s <= ' ' && *s) s++;
			wasid = true;
		}
		else
		if(*s == '[' && !wasid) { // skip MSVC attribute
			do {
				while(*s)
					if(*s++ == ']')
						break;
				while(*s == ' ' || *s == '\2' || *s == '\x1f')
					s++;
			}
			while(*s == '[');
		}
		else
		if(!sSpaces(res, s)) {
			if(*s == ',')
				wasid = false;
			if(*s == '(' && firstpar)
				wasid = firstpar = false;
			res.Cat(*s++);
		}
	}
	return TrimRight(res);
}

String Purify(const char *s) {
	String res;
	while(*s && (byte)*s <= ' ') s++;
	while(*s) {
		if(iscid(*s)) {
			const char *b = s++;
			while(iscid(*s)) s++;
			String q(b, s);
			if(q != s_virtual && q != s_inline)
				res.Cat(q);
			else
				while((byte)*s <= ' ' && *s) s++;
		}
		else
		if(!sSpaces(res, s))
			res.Cat(*s++);
	}
	return TrimRight(res);
}

String Gpurify(const char *s)
{
	String res;
	while(*s)
		if(!sSpaces(res, s))
			res.Cat(*s++);
	return res;
}

void ScAdd(String& s, const String& a)
{
	if(a.IsEmpty()) return;
	if(!s.IsEmpty())
		s << ';';
	s << a;
}


void TpSkip(CParser& p)
{
	int lvl = 0;
	for(;;) {
		if(lvl == 0 && (p.IsChar(',') || p.IsChar('>')) || p.IsEof())
			break;
		if(p.Char('<'))
			lvl++;
		else
		if(p.Char('>'))
			lvl--;
		else
			p.SkipTerm();
	}
}

String Subst(const String& s, const Vector<String>& tpar)
{
	if(tpar.GetCount() == 0)
		return s;
	String r;
	CParser p(s);
	while(!p.IsEof()) {
		if(p.IsId()) {
			String id = p.ReadId();
			int q = FindIndex(tpar, id);
			if(q >= 0)
				r << AsString(q);
			else
				r << id;
		}
		else
			r << p.GetChar();
	}
	return r;
}

String CleanTp(const String& tp)
{
	int q = tp.Find('<');
	int w = tp.ReverseFind('>');
	if(q < 0 || w < 0) return tp;
	String a = TrimLeft(TrimRight(tp.Mid(q + 1, w - q - 1)));
	const char *s = a;
	String r;
	while(*s) {
		if(*s == ',') {
			r.Cat(';');
			s++;
			while(*s && *s <= ' ')
				s++;
		}
		else
			r.Cat(*s++);
	}
	return r;
}

String Parser::Context::Dump() const
{
	return "Scopeing: " + scope;
}

static String s_dblcln("::");

inline void ScopeCat(String& scope, const String& s)
{
	if(scope.GetCount())
		scope << s_dblcln;
	scope << s;
}

void Parser::Context::operator<<=(const Context& t)
{
	ns = t.ns;
	scope = t.scope;
	typenames = clone(t.typenames);
	tparam = clone(t.tparam);
	access = t.access;
	ctname = t.ctname;
	namespace_using = t.namespace_using;
}

bool Parser::IsNamespace(const String& scope)
{
	int l = scope.GetCount();
	return memcmp(~context.ns, ~scope, l) == 0 && findarg(context.ns[l], '\0', ':') >= 0;
}

bool Parser::Key(int code)
{
	if(lex == code) {
		++lex;
		return true;
	}
	return false;
}

int  Parser::GetLine(const char *p)
{
	int pos = int(p - ~file.text);
	int l = 0;
	int h = file.linepos.GetCount();
	while(l < h) {
		int q = (l + h) / 2;
		if(file.linepos[q] < pos)
			l = q + 1;
		else
			h = q;
	}
	return l;
}

void Parser::Line()
{
	int pos = int(lex.Pos() - ~file.text);
	while(line + 1 < file.linepos.GetCount() && file.linepos[line + 1] <= pos)
		line++;
}

void Parser::Check(bool b, const char *err)
{
	if(!b) ThrowError(err);
}

void Parser::CheckKey(int c)
{
	if(!Key(c)) ThrowError(Format("Missing %c", c));
}

String Parser::TemplateParams(String& param)
{
	String r;
	do {
		const char *pos = lex.Pos();
		CheckKey('<');
		int level = 1;
		String id;
		bool gp = true;
		while(lex != t_eof) {
			if(lex.IsId() && gp)
				id = lex.GetId();
			else
			if(Key(',')) {
				ScAdd(param, id);
				id.Clear();
				gp = true;
			}
			else
			if(Key('=')) {
				if(!id.IsEmpty()) {
					ScAdd(param, id);
					id.Clear();
				}
				gp = false;
			}
			else
			if(Key('>')) {
				level--;
				if(level <= 0) {
					ScAdd(param, id);
					break;
				}
			}
			else
			if(Key(t_shr) && level >= 2) {
				level -= 2;
				if(level <= 0) {
					ScAdd(param, id);
					break;
				}
			}
			else
			if(Key('<'))
				level++;
			else
			if(Key('('))
				level++;
			else
			if(Key(')'))
				level--;
			else
				++lex;
		}
		MergeWith(r, ",", String(pos, lex.Pos()));
	}
	while(Key(tk_template));
	return r;
}

String Parser::TemplateParams()
{
	String dummy;
	return TemplateParams(dummy);
}

String Parser::TemplatePnames()
{
	String pnames;
	TemplateParams(pnames);
	return pnames;
}

String Parser::ReadOper(bool& castoper) {
	const char *p = lex.Pos();
	Key(tk_operator);
	int level = 0;
	if(Key('('))
		level++;
	for(;;) {
		if(lex == t_eof || level <= 0 && lex == '(') break;
		if(Key('(') || Key('[')) level++;
		else
		if(Key(')')  || Key(']')) level--;
		else
			++lex;
	}
	StringBuffer r;
	bool spc = false;
	while(p < lex.Pos()) {
		if((byte)*p > ' ') {
			if(spc && iscid(*p)) {
				castoper = true;
				r.Cat(' ');
			}
			r.Cat(*p);
			spc = false;
		}
		else
			spc = true;
		p++;
	}
	return String(r);
}

String Parser::Name(String& name, bool& castoper, bool& oper)
{
	String s;
	if(Key(t_dblcolon)) {
		s = "::";
		name = s;
	}
	Check(lex.IsId() || lex == tk_operator, "Name expected");
	for(;;) {
		if(lex.IsId()) {
			name << lex.Id();
			s << lex.GetId();
		}
		else {
			String h = ReadOper(castoper);
			oper = true;
			name << h;
			s << h;
			break;
		}
		if(lex == '<') // void Fn<byte>(); situation
			s << TemplateParams();
		if(Key(t_dblcolon)) {
			s << "::";
			name << "::";
			if(Key('~')) {
				s << "~";
				name << "~";
			}
		}
		else
			break;
	}
	return s;
}

String Parser::Name(bool& castoper, bool& oper)
{
	String h;
	return Name(h, castoper, oper);
}

String Parser::Constant()
{
	const char *p = lex.Pos();
	const char *p1 = p;
	int level = 0;
	for(;;) {
		p1 = lex.Pos();
		if(lex == t_eof) break;
		if(level <= 0 && (lex == ',' || lex == ';' || lex == ')' || lex == '}' || lex == ']'))
			break;
		if(Key('(') || Key('[') || Key('{')) level++;
		else
		if(Key(')') || Key(']') || Key('}')) level--;
		else
			++lex;
	}
	return String(p, p1);
}

String Parser::TType()
{
	int q = FindIndex(context.tparam, lex[0]);
	if(q >= 0) return AsString(q);
	return lex.Id();
}

String Parser::StructDeclaration(const String& tn, const String& tp)
{
	int t = lex.GetCode(); // t is now struct/class/union
	context.typenames.FindAdd(lex);
	Context cc;
	cc <<= context;
	CParser p(tp);
	Vector<String> tpar;
	if(p.Char('<')) {
		while(!p.IsEof() && !p.Char('>')) {
			if((p.Id("class") || p.Id("typename") || p.Id("struct")) && p.IsId()) {
				tpar.Add(p.ReadId());
				context.tparam.Add(lex.Id(tpar.Top()));
			}
			else
				context.tparam.Add(0);
			TpSkip(p);
			p.Char(',');
		}
	}
	if(Key(t_dblcolon))
		context.scope = Null;
	String name;
	String new_scope = context.scope;
	if(lex.IsId())
		do {
			context.typenames.FindAdd(lex);
			name = lex.GetId(); // name of structure
			if(lex == '<')
				name << TemplateParams();
			ScopeCat(new_scope, name);
		}
		while(Key(t_dblcolon));
	else {
		name = AnonymousName();
		ScopeCat(new_scope, name);
	}
	if(lex.IsId() && findarg(lex.Id(), "override", "final") >= 0)
		++lex;
	if(lex.IsId() || lex == '*') { // struct My { struct My *p; }
		return name;
	}
	context.scope = new_scope;
	context.access = t == tk_class ? PRIVATE : PUBLIC;
	if(tn.GetCount()) {
		if(context.ctname.GetCount())
			context.ctname << ';';
		context.ctname << tn;
	}
	String nn;
	if(!tp.IsEmpty())
		nn = "template " + tp + " ";
	String key = (t == tk_class ? "class" : t == tk_union ? "union" : "struct");
	nn << key << ' ' << name;
	LLOG("Struct "  << context.scope << " using " << context.namespace_using);
	CppItem& im = Item(context.scope, context.namespace_using, key, name, lex != ';');
	im.kind = tp.IsEmpty() ? STRUCT : STRUCTTEMPLATE;
	im.type = name;
	im.access = cc.access;
	im.tname = tn;
	im.ctname = context.ctname;
	im.tparam = CleanTp(tp);
	im.ptype.Clear();
	im.pname.Clear();
	im.param.Clear();
	if(lex == ';') { // TODO: perhaps could be united with following code
		context = pick(cc);
		im.natural = Gpurify(nn);
		SetScopeCurrent();
		return name;
	}
	if(Key(':')) {
		nn << " : ";
		bool c = false;
		do {
			String access = t == tk_class ? "private" : "public";
			bool virt = Key(tk_virtual);
			if(Key(tk_public)) access = "public";
			else
			if(Key(tk_protected)) access = "protected";
			else
			if(Key(tk_private)) access = "private";
			if(Key(tk_virtual) || virt) access << " virtual";
			String h;
			bool dummy;
			String n = Name(h, dummy, dummy);
			ScAdd(im.pname, h);
			if(c)
				im.ptype << ';';
			im.ptype << Subst(n, tpar);
			ScAdd(im.param, access + ' ' + n);
			if(c)
				nn << ", ";
			nn << access + ' ' + n;
			c = true;
		}
		while(Key(','));
	}
	if(Key('{')) {
		struct_level++;
		ScopeBody();
		struct_level--;
		im.natural = Gpurify(nn);
		im.decla = true;
	}
	else
		if(IsNull(im.natural))
			im.natural = Gpurify(nn);
	context = pick(cc);
	SetScopeCurrent();
	return name;
}

String Parser::ReadType(Decla& d, const String& tname, const String& tparam)
{ // returns the name of constructor
	if(findarg((int)lex, tk_struct, tk_class, tk_union) >= 0 && !d.isfriend) {
		d.type = StructDeclaration(tname, tparam);
		return String();
	}
	Key(tk_typename) || Key(tk_struct) || Key(tk_class) || Key(tk_union) || Key(tk_enum) || Key(tk_template);
	if(Key(tk_bool) || Key(tk_float) || Key(tk_double) || Key(tk_void))
		return Null;
	bool sgn = Key(tk_signed) || Key(tk_unsigned);
	if(Key(tk_long)) {
		Key(tk_long);
		Key(tk_int) || Key(tk_double);
		return Null;
	}
	if(Key(tk_short)) {
		Key(tk_unsigned);
		Key(tk_int);
		return Null;
	}
	if(Key(tk_int) || Key(tk_char) ||
	   Key(tk___int8) || Key(tk___int16) || Key(tk___int32) || Key(tk___int64) || Key(tk___int128) ||
	   Key(tk_char16_t) || Key(tk_char32_t)) return Null;
	if(sgn) return Null;
	const char *p = lex.Pos();
	bool cs = false;
	Index<int> cix;
	if(Key(tk_decltype) && Key('(')) {
		const char *p = lex.Pos();
		int lvl = 1;
		for(;;) {
			if(lex == t_eof)
				break;
			if(lex == '(')
				lvl++;
			else
			if(lex == ')' && --lvl == 0) {
				d.type = "@" + String(p, lex.Pos());
				++lex;
				break;
			}
			++lex;
		}
	}
	else
	if(Key(tk_auto))
		d.type = "*";
	else {
		if(Key(t_dblcolon))
			d.type << "::";
		Key(tk_typename) || Key(tk_template);
		Check(lex.IsId(), "Name expected");
		while(lex.IsId()) {
			d.type << TType();
			if(cix.Find(lex) >= 0)
				cs = true;
			else
				cix.Add(lex);
			++lex;
			if(lex == '<')
				d.type << TemplateParams();
			if(Key(t_dblcolon)) {
				d.type << "::";
				if(Key('~'))
					cs = true;
				Key(tk_typename) || Key(tk_template);
			}
			else
				break;
		}
	}
	return cs ? String(p, lex.Pos()) : String();
}

void Parser::Qualifier(bool override_final)
{
	for(;;)
		if(Key(tk_const) || Key(tk_volatile) || Key(tk_constexpr) || Key(tk_thread_local) || VCAttribute())
			;
		else
		if(Key(tk_noexcept)) {
			if(Key('(')) {
				int lvl = 0;
				while(lex != t_eof && lex != ';' && !(lvl == 0 && Key(')')))
					if(Key('('))
						lvl++;
					else
					if(Key(')'))
						lvl--;
					else
						++lex;
			}
		}
		else
		if(override_final && lex.IsId() && findarg(lex.Id(), "override", "final") >= 0)
			++lex;
		else
		if(Key(tk_throw) || Key(tk_alignas)) {
			while(lex != t_eof && lex != ';' && !Key(')'))
				++lex;
		}
		else
		if(lex[0] == '=' && findarg(lex[1], tk_delete, tk_default) >= 0) {
			++lex;
			++lex;
		}
		else
			break;
}

void Parser::Elipsis(Decl& d)
{
	Decl& q = d.param.Add();
	q.name = "...";
	if(lex.IsId()) // bool emplace(_Args&&... __args);
		++lex;
	CheckKey(')');
}

void Parser::ParamList(Decl& d) {
	if(!Key(')'))
		for(;;) {
			if(Key(t_elipsis)) {
				Elipsis(d);
				break;
			}
			else {
				Array<Parser::Decl> decl = Declaration(false, false, Null, Null);
				if(decl.GetCount()) {
					d.param.Add() = pick(decl.Top());
					if(dobody) { // put arguments to the list of local variables
						Decl& p = d.param.Top();
						Local& l = local.Add(p.name);
						l.type = p.type;
						l.isptr = p.isptr;
						l.line = line + 1;
					}
				}
			}
			if(Key(t_elipsis)) {
				Elipsis(d);
				break;
			}
			if(Key(')')) break;
			CheckKey(',');
		}
}

int Parser::RPtr()
{
	int n = 0;
	int tlevel = 0;
	for(;;) {
		int t = lex[n];
		if(t == '*') return n + 1;
		if(t == '<') {
			tlevel++;
			n++;
		}
		else
		if(t == '>') {
			tlevel--;
			n++;
		}
		else
		if(t == t_shr && tlevel >= 2) {
			tlevel -= 2;
			n++;
		}
		else
		if(t == t_dblcolon || lex.IsId(n) || t == ',' && tlevel > 0)
			n++;
		else
			return 0;
	}
}

void Parser::EatInitializers()
{
	if(Key(':')) {
		int lvl = 0;
		for(;;) {
			if(lex == t_eof)
				break;
			if(lvl == 0) {
				Qualifier(false);
				if(lex == '{')
					break;
				if(lex.IsId() && lex[1] == '{') { // : X{123} { case
					lvl++;
					++lex;
					++lex;
				}
			}
			else
				if(lex == '{')
					lvl++;
			if(lex == '(')
				lvl++;
			if(lex == ')' || lex == '}')
				lvl--;
			++lex;
		}
	}
}

void Parser::Declarator(Decl& d, const char *p)
{
	int n = RPtr();
	if(n) {
		while(n--) lex.Get();
		Declarator(d, p);
		d.isptr = true;
		return;
	}
	if(Key('&') || Key(t_and) || Key(tk_const) || Key(tk_volatile)) { // t_and is r-value here
		Declarator(d, p);
		return;
	}
	if(Key('(')) {
		Declarator(d, p);
		if(d.isptr)
			d.nofn = true;
		CheckKey(')');
	}
//	if(lex == tk_operator)
//		d.name = ReadOper();
//	else
	if(lex.IsId() || lex == t_dblcolon || lex == tk_operator) {
		d.name = Name(d.castoper, d.oper);
		if(lex == ':' && lex[1] == t_integer) { // Bitfield, like 'unsigned x:5'
			++lex;
			++lex;
		}
	}
	if(Key('(')) {
		if(inbody || (lex < 256 || lex == tk_true || lex == tk_false)
		   && lex != ')' && lex != '[' && lex != t_dblcolon) {
			int level = 0;
			for(;;) {
				if(lex == t_eof) break;
				if(Key('(')) level++;
				else
				if(Key(')')) {
					if(--level < 0) break;
				}
				else
					++lex;
			}
			return;
		}
		else {
			d.function = !d.nofn;
			ParamList(d);
			p = lex.Pos();
			Qualifier(true);

			if(d.function && Key(t_arrow)) { // C++11 trailing return type
				d.type.Clear();
				ReadType(d, Null, Null);
			}
			
			if(filetype == FILE_C && lex != '{' && lex != ';') // K&R style function header
				while(lex != '{' && lex != t_eof)
					++lex;
		}
	}
	if(*d.type == '*') // C++11 auto declaration
		d.type = ResolveAutoType();
	else
		EatInitializers();
	while(Key('[')) {
		d.isptr = true;
		int level = 1;
		while(level && lex != t_eof) {
			if(Key('[')) level++;
			else
			if(Key(']')) level--;
			else
				++lex;
		}
	}
	if(Key('=') || (inbody && lex == '(')) { // TODO: Add C++11 initializers here (?)
		int level = 0;
		int tlevel = 0;
		for(;;) {
			TryLambda();
			if(lex == t_eof  || lex == ';'
			   || level == 0 && ((lex == ',' && tlevel == 0) || lex == ')'))
				break;
			if(Key('<')) // we ignore < > as operators, always consider them template bracket
				tlevel++;
			else
			if(Key('>'))
				tlevel--;
			else
			if(Key('(') || Key('{'))
				level++;
			else
			if(Key(')') || Key('}'))
				level--;
			else
				++lex;
		}
	}
}

Parser::Decl& Parser::Finish(Decl& d, const char *s)
{
	d.natural = String(s, lex.Pos());
	return d;
}

bool Parser::IsParamList(int q)
{
	return true;
}

void Parser::ReadMods(Decla& d)
{
	for(;;) {
		if(Key(tk_static))
			d.s_static = true;
		else
		if(Key(tk_extern))
			d.s_extern = true;
		else
		if(Key(tk_register))
			d.s_register = true;
		else
		if(Key(tk_mutable))
			d.s_mutable = true;
		else
		if(Key(tk_explicit))
			d.s_explicit = true;
		else
		if(Key(tk_virtual))
			d.s_virtual = true;
		else
		if(!(Key(tk_inline) || Key(tk_force_inline) || Key(tk_never_inline) || Key(tk___inline) || VCAttribute()))
			break;
	}
}

Array<Parser::Decl> Parser::Declaration0(bool l0, bool more, const String& tname, const String& tparam)
{
	Array<Decl> r;
	Decla d;
	const char *p = lex.Pos();
	if(Key(tk_friend))
		d.isfriend = true;
	ReadMods(d);
	Qualifier();
	if(l0) {
		if(lex == tk_SKYLARK && lex[1] == '(' && lex.IsId(2)) {
			++lex;
			++lex;
			Decl& a = r.Add();
			a.name = lex.GetId();
			a.function = true;
			a.natural = String().Cat() << "void " << a.name << "(Http& http)";
			Decl& p = a.param.Add();
			p.name = "http";
			p.type = "Http";
			p.natural = "Http& http";
			Key(',');
			lex.GetText();
			Key(')');
			return r;
		}
		else
		if((lex == tk_RPC_METHOD || lex == tk_RPC_GMETHOD) && lex[1] == '(' && lex.IsId(2)) {
			++lex;
			++lex;
			Decl& a = r.Add();
			a.name = lex.GetId();
			a.function = true;
			a.natural = String().Cat() << "void " << a.name << "(RpcData& rpc)";
			Decl& p = a.param.Add();
			p.name = "rpc";
			p.type = "RpcData";
			p.natural = "RpcData& rpc";
			if (Key(','))
				lex.GetText();
			Key(')');
			return r;
		}
	}
	bool isdestructor = Key('~');
	if(l0 && context.typenames.Find(lex) >= 0 && lex[1] == '(' && lex.IsId()) {
		Decl& a = r.Add();
		a.name = lex.GetId();
		a.isdestructor = isdestructor;
		a.function = true;
		a.istructor = true;
		++lex;
		ParamList(a);
		Qualifier();
		a.natural = String(p, lex.Pos());
		EatInitializers();
		return r;
	}
	if(lex == tk_operator) {
		Decl& a = r.Add();
		(Decla&)a = d;
		a.name = ReadOper(a.castoper);
		Key('(');
		ParamList(a);
		Qualifier();
		a.function = true;
		a.natural = String(p, lex.Pos());
		a.oper = true;
		return r;
	}
	String st = ReadType(d, tname, tparam);
	if(!lex.IsGrounded()) // 'static' etc.. can be after type too... (but not allow it on start of line)
		ReadMods(d);
	if(!st.IsEmpty()) {
		Decl& a = r.Add();
		int q = st.Find('~');
		if(q >= 0)
			st.Remove(q, 1);
		a.name = st;
		a.isdestructor = q >= 0;
		a.function = true;
		a.istructor = true;
		if(Key('('))
			ParamList(a);
		Qualifier();
		a.natural = String(p, lex.Pos());
		EatInitializers();
		return r;
	}
	String natural1 = String(p, lex.Pos());
	if(lex != ';') // struct/class declaration without defining variable
		do {
			const char *p1 = lex.Pos();
			Decl& a = r.Add();
			(Decla&)a = d;
			Declarator(a, p);
			if(a.castoper)
				a.name = Filter(natural1, CharFilterNotWhitespace) + a.name;
			a.natural = natural1 + String(p1, lex.Pos());
			p = lex.Pos();
		}
		while(more && Key(','));
	return r;
}

Array<Parser::Decl> Parser::Declaration(bool l0, bool more, const String& tname, const String& tparam)
{
	if(Key(tk_typedef)) {
		Array<Decl> r = Declaration0(false, true, tname, tparam);
		for(int i = 0; i < r.GetCount(); i++) {
			r[i].type_def = true;
			r[i].natural = "typedef " + r[i].natural;
		}
		return r;
	}
	const char *b = lex.Pos();
	if(Key(tk_using) && lex.IsId()) {
		String name = lex.GetId();
		Key('=');
		Array<Decl> r;
		Decl& d = r.Add();
		ReadType(d, tname, tparam);
		while(Key('*') || Key(tk_volatile) || Key(tk_const));
		d.name = name;
		d.natural = String(b, lex.Pos());
		d.type_def = true;
		return r;
	}
	return Declaration0(l0, more, tname, tparam);
}

String Parser::Tparam(int& q)
{
	if(lex[q] != '<')
		return Null;
	const char *p = lex.Pos(q);
	int level = 1;
	q++;
	while(lex[q] != t_eof && level) {
		if(lex[q] == '<')
			level++;
		if(lex[q] == '>')
			level--;
		else
		if(lex[q] == t_shr && level >= 2)
			level -= 2;
		q++;
	}
	return String(p, lex.Pos(q));
}

String NoTemplatePars(const String& s)
{
	int q = s.Find('<');
	return q >= 0 ? s.Mid(0, q) : s;
}

bool Parser::VCAttribute()
{
	if(lex[0] == '[') // Skip Visual C++ attribute
		for(;;) {
			if(lex[0] == ']') {
				++lex;
				return true;
			}
			if(lex[0] == t_eof)
				return false;
			++lex;
		}
	return false;
}

void Parser::SetScopeCurrent()
{
	current_scope = context.scope;
}

CppItem& Parser::Item(const String& scope, const String& using_namespace, const String& item,
                      const String& name, bool impl)
{
	current_scope = scope;
	if(dobody)
		current = CppItem();
	current_key = item;
	current_name = name;
	CppItem& im = dobody ? current : base->GetAdd(current_scope).Add();
	im.item = item;
	im.name = name;
	im.file = filei;
	im.line = line + 1;
	im.impl = impl;
	im.filetype = filetype;
	im.using_namespaces = using_namespace;
	LLOG("New item " << line + 1 << "    " << scope << "::" << item);
	return im;
}

void Parser::AddMacro(int lineno, const String& macro)
{
	String name;
	const char *s = macro;
	while(*s && iscid(*s))
		name.Cat(*s++);
	CppItem& im = Item("", "", macro, name);
	im.kind = MACRO;
	im.line = lineno;
	im.access = PUBLIC;
}

CppItem& Parser::Item(const String& scope, const String& using_namespace, const String& item,
                      const String& name)
{
	String h = Purify(item);
	CppItem& im = Item(scope, using_namespace, h, name, false);
	im.natural = h;
	return im;
}

void Parser::Resume(int bl)
{
	for(;;) {
		if(lex == t_eof || lex.GetBracesLevel() == bl && lex == ';')
			break;
		++lex;
	}
}

void Parser::ScopeBody()
{
	int bl = lex.GetBracesLevel();
	while(!Key('}')) {
		if(lex == t_eof)
			ThrowError("Unexpected end of file");
		try {
			Do();
		}
		catch(Error) {
			Resume(bl);
			Key(';');
		}
	}
}

String Parser::AnonymousName()
{
	int lvl = 0;
	for(int i = 0; lex[i] != t_eof; i++) {
		if(lex[i] == '{') lvl++;
		else
		if(lex[i] == '}')
			if(--lvl == 0) {
				if(lex.IsId(i + 1))
					return "." + lex.Id(i + 1);
				break;
			}
	}

	dword x[4];
	x[0] = Random();
	x[1] = Random();
	x[2] = Random();
	x[3] = Random();
	
	return "@" + Base64Encode(String((const char *)&x, sizeof(x))) + "/" + title;
}

void Parser::AddNamespace(const String& n, const String& name)
{
	String h = "namespace " + n;
	CppItem& m = Item(n, Null, h, name);
	m.kind = NAMESPACE;
	m.natural = h;
}

bool Parser::Scope(const String& tp, const String& tn) {
	if(Key(tk_namespace)) {
		Check(lex.IsId(), "Expected name of namespace");
		String name = lex.GetId();
		LLOG("namespace " << name);
		namespace_info << ';' << name;
		Context c0;
		c0 <<= context;
		int struct_level0 = struct_level;
		ScopeCat(context.scope, name);
		ScopeCat(context.ns, name);
		AddNamespace(context.scope, name);
		if(Key('{')) {
			Context cc;
			cc <<= context;
			while(!Key('}')) {
				if(lex == t_eof)
					ThrowError("Unexpected end of file");
				try {
					Do();
				}
				catch(Error) {
					if(struct_level0)
						throw;
					context <<= cc;
					struct_level = struct_level0;
					LLOG("---- Recovery to namespace level");
					++lex;
					lex.SkipToGrounding();
					lex.ClearBracesLevel();
					LLOG("Grounding skipped to " << GetLine(lex.Pos()));
				}
				catch(Lex::Grounding) {
					LLOG("---- Grounding to namespace level");
					context <<= cc;
					struct_level = struct_level0;
					lex.ClearBracesLevel();
					lex.ClearBody();
				}
			}
		}
		LLOG("End namespace");
		Key(';');
		context <<= c0;
		SetScopeCurrent();
		namespace_info << ";}";
		return true;
	}
	return false;
}

CppItem& Parser::Fn(const Decl& d, const String& templ, bool body,
                    const String& tname, const String& tparam)
{
	String param;
	String pname;
	String ptype;
	for(int i = 0; i < d.param.GetCount(); i++) {
		const Decla& p = d.param[i];
		ScAdd(param, p.natural);
		if(i)
			ptype << ';';
		ptype << p.type;
		ScAdd(pname, p.name);
	}
	String nn0;
	String nm = d.name;
	int q;
	if(d.castoper) {
		q = d.name.ReverseFind(' ');
		q = q > 0 ? d.name.ReverseFind(':', q) : d.name.ReverseFind(':');
	}
	else
		q = d.name.ReverseFind(':');
	if(q >= 0) {
		nm = d.name.Mid(q + 1);
		if(q > 0)
			nn0 = d.name.Mid(0, q - 1);
	}
	String item = FnItem(d.natural, pname, d.name, nm, d.oper);
	String scope = context.scope;
	String nn;
	const char *s = nn0;
	int l = 0;
	while(*s) {
		if(*s == '<')
			l++;
		else
		if(*s == '>')
			l--;
		else
		if(l == 0)
			nn.Cat(*s);
		s++;
	}
	s = nn;
	while(*s == ':') s++;
	if(*s)
		ScopeCat(scope, s);
	CppItem& im = Item(scope, context.namespace_using, item, nm, body);
	im.natural.Clear();
	if(!IsNull(templ)) {
		im.natural = TrimRight(Gpurify(templ)) + ' ';
		im.at = im.natural.GetLength();
	}
	im.natural << Purify(d.natural, d.name, nm);
	im.kind = templ.GetCount() ? IsNamespace(scope) ? FUNCTIONTEMPLATE
	                                           : d.s_static ? CLASSFUNCTIONTEMPLATE
	                                                        : INSTANCEFUNCTIONTEMPLATE
	                           : d.istructor ? (d.isdestructor ? DESTRUCTOR : CONSTRUCTOR)
	                                         : d.isfriend ? INLINEFRIEND
	                                                      : IsNamespace(scope) ? FUNCTION
	                                                                      : d.s_static ? CLASSFUNCTION
	                                                                                   : INSTANCEFUNCTION;
	im.param = param;
	im.pname = pname;
	im.ptype = ptype;
	im.access = context.access;
	im.virt = d.s_virtual;
	im.type = d.type;
	im.decla = true;
	im.tname = tname;
	im.tparam = tparam;
	im.ctname = context.ctname;
	LLOG("FnItem: " << scope << "::" << item << ", natural: " << im.natural
	                << ", ctname: " << im.ctname);
	return im;
}

void Parser::Enum(bool vars)
{
	String name;
	if(lex.IsId())
		name = lex.GetId();
	while(lex != '{' && lex != ';' && lex != t_eof)
		++lex;
	if(Key('{')) {
		for(;;) {
			Line();
			String val;
			Check(lex.IsId(), "Expected identifier");
			String id = lex.GetId();
			CppItem& im = Item(context.scope, context.namespace_using, id, id);
			im.natural = "enum ";
			if(!IsNull(name))
				im.natural << name << ' ';
			im.natural << id;
			if(Key('='))
				im.natural << " = " << Constant();
			im.kind = ENUM;
			im.access = context.access;
			Key(',');
			if(Key('}')) break;
		}
	}
	while(!Key(';')) {
		if(lex.IsId()) {
			if(vars) { // typedef name ignored here
				String scope = context.scope;
				String name = lex.GetId();
				CppItem& im = Item(scope, context.namespace_using, name, name);
				im.natural = "enum " + name;
				im.access = context.access;
				im.kind = IsNamespace(scope) ? VARIABLE : INSTANCEVARIABLE;
			}
			++lex;
		}
		else
		if(lex == ',' || lex == '*')
			++lex;
		else
			break;
	}
	Key(';');
	SetScopeCurrent();
}

bool Parser::UsingNamespace()
{
	if(lex == tk_using && !(lex.IsId(1) && lex[2] == '=')) {
		++lex;
		if(Key(tk_namespace))
			while(lex.IsId()) {
				Vector<String> h = Split(context.namespace_using, ';');
				String u;
				do {
					u << lex.GetId();
					if(Key(t_dblcolon))
						u << "::";
				}
				while(lex.IsId());
				if(FindIndex(h, u) < 0)
					h.Add(u);
				context.namespace_using = Join(h, ";");
				Key(',');
			}
		while(!Key(';') && lex != t_eof)
			++lex;
		namespace_info << ";using " << context.namespace_using;
		return true;
	}
	return false;
}

bool Parser::IsEnum(int i)
{
	for(;;) {
		if(lex[i] == '{')
			return true;
		else
		if(lex.IsId(i) || lex[i] == ':' ||
		   findarg(lex[i], tk_bool, tk_signed, tk_unsigned, tk_long, tk_int, tk_short,
		                       tk_char, tk___int8, tk___int16, tk___int32, tk___int64,
		                       tk_char16_t, tk_char32_t) >= 0)
			i++;
		else
			return false;
	}
}

void Parser::ClassEnum(const String& clss)
{
	context.typenames.FindAdd(lex);
	Context cc;
	cc <<= context;
	String name = lex.GetId();
	String new_scope = context.scope;
	ScopeCat(new_scope, name);
	context.scope = new_scope;
	context.access = PUBLIC;
	String key = "class";
	LLOG("enum class "  << context.scope << " using " << context.namespace_using);
	CppItem& im = Item(context.scope, context.namespace_using, key, name, lex != ';');
	im.kind = STRUCT;
	im.natural = clss + " enum " + name;
	im.type = name;
	im.access = cc.access;
	im.tname.Clear();
	im.ctname.Clear();
	im.tparam.Clear();
	im.ptype.Clear();
	im.pname.Clear();
	im.param.Clear();
	Enum(true);
	context = pick(cc);
	SetScopeCurrent();
}

void Parser::DoNamespace()
{
	if(Key('{'))
		while(!Key('}')) {
			if(lex == t_eof)
				ThrowError("Unexpected end of file");
			Do();
		}
	Key(';');
}

void Parser::Do()
{
	LLOG("Do, scope: " << current_scope);
	if(lex.IsGrounded() && struct_level)
		throw Lex::Grounding();
	Line();
	if(UsingNamespace())
		;
	else
	if(Key(tk_static_assert))
		while(lex != t_eof && lex != ';')
			++lex;
	else
	if(lex == tk_inline && lex[1] == tk_namespace) { // for now, inline namespace is simply ignored
		while(lex != t_eof && lex != '{')
			++lex;
		DoNamespace();
	}
	else
	if(Key(';')) // 'empty' declaration, result of some ignores
		;
	else
	if(lex == tk_extern && lex[1] == tk_template) { // skip 'extern template void Foo<char>();'
		while(lex != t_eof) {
			if(lex == ';') {
				++lex;
				break;
			}
			++lex;
		}
	}
	else
	if(Key(tk_extern) && lex == t_string) { // extern "C++" kind
		++lex;
		DoNamespace();
	}
	else
	if(Key(tk_template)) {
		if(lex.IsId() || lex == tk_class && lex.IsId(1)) {
			Key(tk_class);
			for(;;) {
				if(lex.IsId())
					lex.GetId();
				else
				if(!Key(t_dblcolon))
					break;
			}
			TemplateParams();
			Key(';');
		}
		else {
			String tnames;
			String tparam = TemplateParams(tnames);
			if(lex == tk_using) { //C++11 template alias template <...> using ID =
				Array<Decl> r = Declaration(true, true, tnames, tparam);
				for(int i = 0; i < r.GetCount(); i++) {
					Decla& d = r[i];
					if(d.type_def) {
						String scope = context.scope;
						ScopeCat(scope, d.name);
						CppItem& im = Item(scope, context.namespace_using, "typedef", d.name);
						im.natural = Purify(d.natural);
						im.type = d.type;
						im.access = context.access;
						im.kind = TYPEDEF;
					}
				}
			}
			else {
				if(!Scope(tparam, tnames)) {
					Array<Decl> r = Declaration(true, true, tnames, tparam);
					bool body = lex == '{';
					for(int i = 0; i < r.GetCount(); i++) {
						Decl& d = r[i];
						if(!d.isfriend && d.function)
							Fn(d, "template " + tparam + ' ', body, tnames, tparam);
					}
					EatBody();
					Key(';');
				}
				EatBody();
			}
		}
	}
	else
	if(lex == tk_enum && IsEnum(1)) {
		++lex;
		Enum(true);
	}
	else
	if(lex == tk_enum && lex[1] == tk_class && IsEnum(2)) {
		++lex;
		++lex;
		ClassEnum("class"); // like enum class Foo { A, B }
	}
	else
	if(lex == tk_enum && lex[1] == tk_struct && IsEnum(2)) {
		++lex;
		++lex;
		ClassEnum("struct"); // like enum struct Foo { A, B }
	}
	else
	if(lex == tk_typedef && lex[1] == tk_enum && IsEnum(2)) {
		++lex;
		++lex;
		Enum(false);
	}
	else
	if(!Scope(String(), String())) {
		if(Key(tk_public)) {
			context.access = PUBLIC;
			Key(':');
		}
		else
		if(Key(tk_private)) {
			context.access = PRIVATE;
			Key(':');
		}
		else
		if(Key(tk_protected)) {
			context.access = PROTECTED;
			Key(':');
		}
		else {
			Array<Decl> r = Declaration(true, true, Null, Null);
			bool body = lex == '{';
			for(int i = 0; i < r.GetCount(); i++) {
				Decl& d = r[i];
				if(d.name.GetCount()) {
					if(d.function) {
						if(!d.isfriend)
							Fn(d, Null, body, String(), String());
					}
					else {
						String h = d.natural;
						int q = h.Find('=');
						if(q >= 0)
							h = TrimRight(h.Mid(0, q));
						String scope = context.scope;
						if(d.type_def)
							ScopeCat(scope, d.name);
						String name = d.name;
						int member_type = d.s_static ? CLASSVARIABLE : INSTANCEVARIABLE;
						q = d.name.ReverseFind("::");
						if(q >= 0) { // class variable definition like: int Ctrl::EventLoop;
							ScopeCat(scope, d.name.Mid(0, q));
							current_scope = scope; // temporary until ';'
							name = d.name.Mid(q + 2);
							member_type = CLASSVARIABLE;
						}
						CppItem& im = Item(scope, context.namespace_using,
						                   d.isfriend ? "friend class"
						                   : d.type_def ? "typedef"
						                   : name, name);
						im.natural = Purify(h);
						im.type = d.type;
						im.access = context.access;
						im.kind = d.isfriend ? FRIENDCLASS :
						          d.type_def ? TYPEDEF :
						          IsNamespace(scope) ? VARIABLE :
						          member_type;
						if(im.IsData())
							im.isptr = d.isptr;
					}
				}
			}
			EatBody();
			if(Key(';'))
				SetScopeCurrent(); // need to be after ';' to make class variable definitions work
		}
	}
}

void  Parser::Do(Stream& in, CppBase& _base, int filei_, int filetype_,
                 const String& title_, Event<int, const String&> _err,
                 const Vector<String>& typenames,
                 const Vector<String>& namespace_stack,
                 const Index<String>& namespace_using)
{
	LTIMING("Parser::Do");
	LLOG("= C++ Parser ==================================== " << title_ << " " << namespace_stack << ", dobody: " << dobody);
	base = &_base;
	err = _err;
	filei = filei_;
	filetype = filetype_;
	title = title_;
	lpos = 0;
	line = 0;

	context.namespace_using = Join(namespace_using.GetKeys(), ";");
	
	String n;
	for(int i = 0; i < namespace_stack.GetCount(); i++) {
		MergeWith(n, "::", namespace_stack[i]);
		AddNamespace(n, namespace_stack[i]);
	}

	file = PreProcess(in, *this);
	lex.Init(~file.text);

	while(lex != t_eof)
		try {
			try {
				current_scope.Clear();
				context.access = PUBLIC;
				context.typenames.Clear();
				context.tparam.Clear();
				context.ns = context.scope = Join(namespace_stack, "::");
				inbody = false;
				struct_level = 0;
				for(int i = 0; i < typenames.GetCount(); i++)
					context.typenames.Add(lex.Id(typenames[i]));
				Do();
			}
			catch(Error) {
				if(lex.IsBody()) {
					LLOG("---- Recovery to next ';'");
					Resume(lex.GetBracesLevel());
					Key(';');
				}
				else {
					LLOG("---- Recovery to file level");
					++lex;
					lex.SkipToGrounding();
					lex.ClearBracesLevel();
					LLOG("Grounding skipped to " << GetLine(lex.Pos()));
				}
			}
		}
		catch(Lex::Grounding) {
			LLOG("---- Grounding to file level");
			lex.ClearBracesLevel();
			lex.ClearBody();
		}
}

Vector<String> ParserContext::GetNamespaces() const
{
	Vector<String> ns;
	Vector<String> h = Split(current_scope, ':');
	while(h.GetCount()) {
		ns.Add(Join(h, "::"));
		h.Drop();
	}
	ns.Append(Split(context.namespace_using, ';'));
	ns.Add(""); // Add global namespace too
	return ns;
}

}
