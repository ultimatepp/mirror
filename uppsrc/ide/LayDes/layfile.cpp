#include "LayDes.h"

ArrayMap<String, LayDesEditPos>& LayFileDataVar()
{
	static ArrayMap<String, LayDesEditPos> x;
	return x;
}

void SerializeLayEditPos(Stream& s)
{
	ArrayMap<String, LayDesEditPos>& filedata = LayFileDataVar();
	int dummy = 0;
	if(s.IsStoring()) {
		for(int i = 0; i < filedata.GetCount(); i++) {
			String fn = filedata.GetKey(i);
			if(!fn.IsEmpty()) {
				FindFile ff(fn);
				LayDesEditPos& ep = filedata[i];
				if(ff && ff.GetLastWriteTime() == ep.filetime) {
					s % fn;
					s % ep.filetime;
					s % ep.layouti;
					s % dummy;
				}
			}
		}
		String h;
		s % h;
	}
	else {
		String fn;
		filedata.Clear();
		for(;;) {
			s % fn;
			if(fn.IsEmpty()) break;
			LayDesEditPos& ep = filedata.GetAdd(fn);
			ep.undo.Clear();
			ep.redo.Clear();
			ep.cursor.Clear();
			s % ep.filetime;
			s % ep.layouti;
			s % dummy;
		}
	}

}

void LayDes::SaveEditPos()
{
	LayDesEditPos& p = LayFileDataVar().GetAdd(filename);
	p.filetime = filetime;
	p.undo.SetCount(layout.GetCount());
	p.redo.SetCount(layout.GetCount());
	for(int i = 0; i < layout.GetCount(); i++) {
		p.undo[i] = pick(layout[i].undo);
		p.redo[i] = pick(layout[i].redo);
	}
	p.layouti = list.GetKey();
	p.cursor = pick(cursor);
}

void LayDes::RestoreEditPos()
{
	if(IsNull(filetime)) return;
	int q = LayFileDataVar().Find(filename);
	if(q < 0)
		return;
	LayDesEditPos& p = LayFileDataVar()[q];
	if(p.filetime != filetime) return;
	if(layout.GetCount() == p.undo.GetCount()) {
		for(int i = 0; i < layout.GetCount(); i++) {
			layout[i].undo = pick(p.undo[i]);
			layout[i].redo = pick(p.redo[i]);
		}
	}
	if(p.layouti >= 0 && p.layouti < layout.GetCount()) {
		GoTo(p.layouti);
		LayoutCursor();
	}
	cursor = pick(p.cursor);
	SyncItems();
}

void LayDes::FindLayout(const String& name, const String& item_name)
{
	for(int i = 0; i < layout.GetCount(); i++)
		if(layout[i].name == name) {
			GoTo(i);
			if(!IsNull(item_name)) {
				int q = item.Find(item_name, 1);
				if(q >= 0)
					SelectOne(q, 0);
			}
		}
}

String LayDes::GetLayoutName() const
{
	return currentlayout >= 0 && currentlayout < layout.GetCount() ? layout[currentlayout].name
	                                                               : String();
}

bool LayDes::Load(const char *file, byte _charset)
{
	charset = _charset;
	if(charset == CHARSET_UTF8_BOM)
		charset = CHARSET_UTF8;
	layout.Clear();
	filename = file;
	FileIn in(file);
	if(in) {
		layfile = LoadStream(in);
		newfile = false;
		filetime = in.GetTime();
		fileerror.Clear();
		EOL = GetLineEndings(layfile);
		try {
			CParser p(layfile);
			if(p.Char('#') && p.Id("ifdef")) {
				if(!p.Id("LAYOUTFILE"))
					p.Id("LAYOUT");
			}
			while(!p.IsEof()) {
				LayoutData& ld = layout.Add();
				ld.SetCharset(charset);
				ld.Read(p);
				if(p.Char('#'))
					p.Id("endif");
			}
		}
		catch(CParser::Error& e) {
			PutConsole(file + e);
			return false;
		}
	}
	else {
		newfile = true;
		filetime = Null;
		EOL = "\r\n";
	}
	search <<= Null;
	SyncLayoutList();
	if(list.GetCount()) {
		list.SetCursor(0);
		LayoutCursor();
	}
	RestoreEditPos();
	return true;
}

void LayDes::Save()
{
	StoreToGlobal(*this, "laydes-ctrl");

	if(!IsNull(fileerror))
		return;
	String r;
	for(int i = 0; i < layout.GetCount(); i++) {
		layout[i].SetCharset(charset);
		r << layout[i].Save(0, EOL) << EOL;
	}
	layfile = r;
	if(!SaveChangedFileFinish(filename, r))
		return;
	FindFile ff(filename);
	if(ff) {
		newfile = false;
		filetime = ff.GetLastWriteTime();
	}
}
