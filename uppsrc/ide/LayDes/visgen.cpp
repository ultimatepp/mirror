#include "LayDes.h"

struct VisGenDlg : public WithVisGenLayout<TopWindow> {
	LayoutData&  layout;
	Vector<int>  sel;

	void Refresh();
	bool HasItem(const char *id);
	void Type();
	String GetName();

	typedef VisGenDlg CLASSNAME;

	VisGenDlg(LayoutData& layout, const Vector<int>& cursor);
};

bool VisGenDlg::HasItem(const char *id)
{
	for(int i = 0; i < layout.item.GetCount(); i++)
		if(layout.item[i].variable == id)
			return true;
	return false;
}

String VisGenDlg::GetName()
{
	String n = layout.name;
	int l = n.GetCount() - 6;
	if(l > 0 && n.Mid(l) == "Layout")
		n = n.Mid(0, l);
	return n + "Dlg";
}

String IdInitCaps(const String& src)
{
	String h = InitCaps(src);
	h.Replace("_", "");
	return h;
}

void VisGenDlg::Refresh()
{
	String s;
	int q = ~type;
	parent_ctrl.Enable(q < 3);
	bool b = q >= 3 && q <= 4;
	pars.Enable(b);
	brackets.Enable(b);
	label1.Enable(b);
	toupper1.Enable(b);
	tolower1.Enable(b);
	initcaps1.Enable(b);
	name1.Enable(b);
	dname1.Enable(b);
	quotes1.Enable(b && !label1);
	b = q == 4;
	label2.Enable(b);
	toupper2.Enable(b);
	tolower2.Enable(b);
	initcaps2.Enable(b);
	name2.Enable(b);
	dname2.Enable(b);
	quotes2.Enable(b && !label2);
	String oce = "\tCtrlLayout";
	bool ok = false;
	if(HasItem("ok")) {
		ok = true;
		oce << "OK";
	}
	if(HasItem("cancel"))
		oce << "Cancel";
	if(HasItem("exit"))
		oce << "Exit";
	String n = ~name;
	if(IsNull(n))
		n = GetName();
	
	String b1, b2, b3;
	if(lambdas) {
		for(int i = 0; i < layout.item.GetCount(); i++) {
			String bn = layout.item[i].variable;
			if(layout.item[i].type == "Button" && findarg(bn, "cancel", "ok", "exit") < 0) {
				if(b2.GetCount() == 0)
					b2 = "\n";
				b2 << '\t' << bn << " << [=] { };\n";
			}
		}
	}
	if(buttons) {
		for(int i = 0; i < layout.item.GetCount(); i++) {
			String bn = layout.item[i].variable;
			if(layout.item[i].type == "Button" && findarg(bn, "cancel", "ok", "exit") < 0) {
				String mn = IdInitCaps(bn);
				mn.Replace("_", "");
				b1 << '\t' << "void " << mn << "();\n";
				b2 << '\t' << bn << " << [=] { " << mn << "(); };\n";
				b3 << '\n' << "void " << n << "::" << mn << "()\n{\n}\n";
			}
		}
		if(b1.GetCount())
			b1 << "\n";
	}
	
	String base = parent_ctrl ? "ParentCtrl" : "TopWindow";

	if(q == 0) {
		s << "class " << n << " : public With" << layout.name << "<" << base << "> {\n"
		  << b1
		  << "public:\n"
		  << "\t" << n << "();\n"
		  << "};\n"
		  << "\n"
		  << n << "::" << n << "()\n"
		  << "{\n"
		  << oce << "(*this, \"\");\n"
		  << b2
		  << "}\n"
		  << b3;
	}
	else
	if(q == 1) {
		s << "struct " << n << " : With" << layout.name << "<" << base << "> {\n"
		  << b1
		  << "\t" << n << "();\n"
		  << "};\n"
		  << "\n"
		  << n << "::" << n << "()\n"
		  << "{\n"
		  << oce << "(*this, \"\");\n"
		  << b2
		  << "}\n"
		  << b3;
	}
	else
	if(q == 2) {
		String n = ~name;
		if(IsNull(n))
			n = "dlg";
		s << "\tWith" << layout.name << "<" << base << "> " << n << ";\n"
		  << oce << '(' << n << ", \"\");\n";
		if(ok)
			s << "\tif(" << n << ".Execute() != IDOK)\n\t\treturn;\n";
		else
			s << '\t' << n << ".Execute();\n";
	}
	else
		for(int i = 0; i < sel.GetCount(); i++) {
			String id1 = layout.item[sel[i]].variable;
			String id2 = id1;
			int w = layout.item[sel[i]].FindProperty("SetLabel");
			String lbl;
			if(w >= 0 && IsString(~layout.item[sel[i]].property[w]))
				lbl = AsCString(ToUtf8((WString)~layout.item[sel[i]].property[w]));
			if(q <= 4) {
				if(label1)
					id1 = lbl;
				if(label2)
					id2 = lbl;
			}
			if(IsNull(id1) && (q != 4 || IsNull(id2)))
				continue;
			if(q == 5)
				s << "\t" << id1 << " << [=] { };\n";
			else
			if(q == 6)
				s << "\t" << IdInitCaps(id1) << "();\n";
			else
			if(q == 7)
				s << "\t" << id1 << " << [=] { " << IdInitCaps(id1) << "(); };\n";
			else {
				if((pars || brackets) && !(name1 || name2 || dname1 || dname2))
					s << ~name;
				if(pars)
					s << '(';
				String ss;
				if(name1 && !IsNull(~name))
					s << ~name << '.';
				if(toupper1)
					ss << ToUpper(id1);
				else
				if(tolower1)
					ss << ToLower(id1);
				else
				if(initcaps1)
					ss << IdInitCaps(id1);
				else
					ss << id1;
				if(quotes1 && !label1)
					ss = AsCString(ss);
				if(dname1 && !IsNull(~name))
					ss << '.' << ~name;
				if(brackets)
					s << '[';
				s << ss;
				if(brackets)
					s << ']';
				if(q == 4) {
					s << ", ";
					String ss;
					if(name2 && !IsNull(~name))
						ss << ~name << '.';
					if(toupper2)
						ss << ToUpper(id2);
					else
					if(tolower2)
						ss << ToLower(id2);
					else
					if(initcaps2)
						ss << IdInitCaps(id2);
					else
						ss << id2;
					if(quotes2 && !label2)
						ss = AsCString(ss);
					if(dname2 && !IsNull(~name))
						ss << '.' << ~name;
					if(brackets)
						s << '[';
					s << ss;
					if(brackets)
						s << ']';
				}
				if(pars)
					s << ')';
				if(semicolon)
					s << ';';
				s << '\n';
			}
		}
	view <<= s;
}

void VisGenDlg::Type()
{
	String n = GetName();
	buttons.Disable();
	switch((int)~type) {
	case 0:
	case 1:
		name <<= GetName();
		buttons.Enable();
		break;
	case 2:
		name <<= "dlg";
		break;
	default:
		name <<= "";
	}
	Refresh();
}

VisGenDlg::VisGenDlg(LayoutData& layout, const Vector<int>& cursor)
:	layout(layout)
{
	type <<= 0;
	CtrlLayoutOKCancel(*this, "Code generator");
	type <<= THISBACK(Type);

	// needs to be before Refresh to maintain the proper order of action
	toupper1 << [=] { tolower1 <<= false; initcaps1 <<= false; };
	tolower1 << [=] { toupper1 <<= false; initcaps1 <<= false; };
	initcaps1 << [=] { toupper1 <<= false; tolower1 <<= false; };

	toupper2 << [=] { tolower2 <<= false; initcaps2 <<= false; };
	tolower2 << [=] { toupper2 <<= false; initcaps2 <<= false; };
	initcaps2 << [=] { toupper2 <<= false; tolower2 <<= false; };

	for(Ctrl *q = GetFirstChild(); q; q = q->GetNext())
		if(dynamic_cast<Option *>(q))
			*q << [=] { Refresh(); };
			
	name << [=] { Refresh(); };
	

	Refresh();
	view.Highlight("cpp");
	view.HideBar();
	view.SetFont(CourierZ(12));
	if(cursor.GetCount())
		sel <<= cursor;
	else
		for(int i = 0; i < layout.item.GetCount(); i++)
			sel.Add(i);
}

void LayDes::VisGen()
{
	if(IsNull(currentlayout))
		return;
	VisGenDlg dlg(CurrentLayout(), cursor);
	if(dlg.Run() == IDOK)
		WriteClipboardText(~dlg.view);
}
