#include "ide.h"

struct MainConfigDlg : public WithConfigLayout<TopWindow> {
	EditString ce, fe;
	FrameRight<Button> cb;
	const Workspace& wspc;
	
	void FlagDlg();

	bool Perform(const String& startwith);

	typedef MainConfigDlg CLASSNAME;

	MainConfigDlg(const Workspace& wspc);
};

bool SetSw(const String& flag, Ctrl& sw, const char *flg) {
	if(flag == flg) {
		sw <<= 1;
		return true;
	}
	return false;
}

String GetSw(Ctrl& sw, const char *flag) {
	if((int)~sw)
		return String(flag) + ' ';
	return Null;
}


static
void sSetOption(One<Ctrl>& ctrl)
{
	ctrl.Create<Option>().NoWantFocus();
}

void MainConfigDlg::FlagDlg()
{
	WithConfLayout<TopWindow> cfg;
	CtrlLayoutOKCancel(cfg, "Configuration flags");
	cfg.Sizeable().MaximizeBox();
	Vector<String> flg = SplitFlags0(~~fe);
	Vector<String> accepts = wspc.GetAllAccepts(0);
	Sort(accepts, GetLanguageInfo());
	enum { CC_SET, CC_NAME, CC_PACKAGES, CC_COUNT };
	cfg.accepts.AddColumn("Set").Ctrls(sSetOption).HeaderTab().Fixed(40);
	cfg.accepts.AddColumn("Flag", 1);
	cfg.accepts.AddColumn("Packages", 2);
	cfg.accepts.SetCount(accepts.GetCount());
	cfg.accepts.SetLineCy(Zy(20));
	for(int i = 0; i < accepts.GetCount(); i++) {
		String acc = accepts[i];
		Vector<String> pkg;
		for(int p = 0; p < wspc.GetCount(); p++)
			if(FindIndex(wspc.package[p].accepts, acc) >= 0)
				pkg.Add(wspc[p]);
		Sort(pkg, GetLanguageInfo());
		cfg.accepts.Set(i, CC_NAME, accepts[i]);
		cfg.accepts.Set(i, CC_PACKAGES, Join(pkg, ","));
	}

	cfg.other.SetFilter(FlagFilterM);
	cfg.dll <<= cfg.gui <<= cfg.mt <<= cfg.sse2 <<= 0;
	String other;
	for(int i = 0; i < flg.GetCount(); i++) {
		String f = flg[i];
		if(!SetSw(f, cfg.dll, "DLL")
		   && !SetSw(f, cfg.gui, "GUI")
		   && !SetSw(f, cfg.mt, "MT")
		   && !SetSw(f, cfg.sse2, "SSE2")) {
			int x = (*f == '.' ? cfg.accepts.Find(f.Mid(1), CC_NAME) : -1);
			if(x >= 0)
				cfg.accepts.Set(x, CC_SET, true);
			else {
				if(!other.IsEmpty())
					other << ' ';
				other << f;
			}
		}
	}
	cfg.other <<= other;
	if(cfg.Run() == IDOK) {
		String flags;
		flags
			<< GetSw(cfg.dll, "DLL")
		    << GetSw(cfg.gui, "GUI")
		    << GetSw(cfg.mt, "MT")
			<< GetSw(cfg.sse2, "SSE2");
		for(int i = 0; i < cfg.accepts.GetCount(); i++)
			if(cfg.accepts.Get(i, CC_SET))
				flags << '.' << cfg.accepts.Get(i, CC_NAME) << ' ';
		flags << cfg.other.GetText().ToString();
		fe = Join(SplitFlags0(flags), " ").ToWString();
	}
}

MainConfigDlg::MainConfigDlg(const Workspace& wspc_) : wspc(wspc_) {
	CtrlLayoutOKCancel(*this, "Main package configuration(s)");
	fe.AddFrame(cb);
	fe.SetFilter(FlagFilterM);
	cb.SetImage(CtrlImg::smallright()).NoWantFocus();
	cb <<= THISBACK(FlagDlg);
	list.AddColumn("Flags", 3).Edit(fe);
	list.AddColumn("Optional name", 2).Edit(ce);
	list.Appending().Removing().Moving().Duplicating();
	
	list.WhenDrag = [=] {
		list.DoDragAndDrop(InternalClip(list, "main_config-item"), list.GetDragSample(), DND_MOVE);
	};
	list.WhenDropInsert = [=](int line, PasteClip& d) {
		if(GetInternalPtr<ArrayCtrl>(d, "main_config-item") == &list && list.IsCursor() && d.Accept()) {
			int q = list.GetCursor();
			if(q == line)
				return;
			Vector<Value> h = list.ReadRow(q);
			list.Remove(q);
			if(q < line)
				line--;
			if(line >= 0 && line <= list.GetCount()) {
				list.Insert(line);
				list.Set(line, h);
				list.SetCursor(line);
			}
		}
	};

}

bool MainConfigDlg::Perform(const String& startwith) {
	list.SetCursor(0);
	list.FindSetCursor(startwith);
	return Run() == IDOK;
}

void Ide::MainConfig() {
	package.SetCursor(0);
	if(package.GetCursor() != 0) return;
	MainConfigDlg dlg(IdeWorkspace());
	for(int i = 0; i < actual.config.GetCount(); i++) {
		const Package::Config& f = actual.config[i];
		dlg.list.Add(f.param, f.name);
	}
	if(!dlg.Perform(mainconfigname)) return;
	actual.config.Clear();
	for(int i = 0; i < dlg.list.GetCount(); i++) {
		Package::Config& f = actual.config.Add();
		f.param = dlg.list.Get(i, 0);
		f.name = dlg.list.Get(i, 1);
	}
	SavePackage();
	if(dlg.list.IsCursor()) {
		mainconfigparam = dlg.list.Get(0);
		mainconfigname = Nvl((String)dlg.list.Get(1), mainconfigparam);
		MakeTitle();
	}
	SyncMainConfigList();
	SetHdependDirs();
	MakeTitle();
}
