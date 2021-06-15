#include "CtrlLib.h"

#ifdef GUI_WIN
#ifndef PLATFORM_WINCE

#include <commdlg.h>
#pragma  comment(lib, "comdlg32.lib")

#endif
#else

#include <PdfDraw/PdfDraw.h>

#endif

namespace Upp {

#ifdef GUI_WIN

#ifndef PLATFORM_WINCE

#include <commdlg.h>
#pragma  comment(lib, "comdlg32.lib")

struct Win32PrintDlg_ : PRINTDLG {
	Win32PrintDlg_() {
		memset(this, 0, sizeof(PRINTDLG));
		lStructSize = sizeof(PRINTDLG);
	}
	~Win32PrintDlg_() {
		if(hDevMode)
			::GlobalFree(hDevMode);
		if(hDevNames)
			::GlobalFree(hDevNames);
	}
};

PrinterJob::PrinterJob(const char *_name)
{
	name = _name;
	landscape = false;
	from = to = 1;
	current = 1;
}

PrinterJob::~PrinterJob()
{
}

bool PrinterJob::Execute0(bool dodlg)
{
	pdlg.Create<Win32PrintDlg_>();
	PRINTDLG& dlg = *pdlg;
	dlg.Flags = PD_DISABLEPRINTTOFILE|PD_NOSELECTION|PD_HIDEPRINTTOFILE|PD_RETURNDEFAULT;
	dlg.nFromPage = current;
	dlg.nToPage = current;
	dlg.nMinPage = from;
	dlg.nMaxPage = to;
	if(from != to)
		dlg.Flags |= PD_ALLPAGES;
	dlg.hwndOwner = 0;
	dlg.Flags |= PD_RETURNDEFAULT;
	dlg.nCopies = 1;
	if(!PrintDlg(&dlg)) return false;
	if(dlg.hDevMode) {
		DEVMODE *pDevMode = (DEVMODE*)::GlobalLock(dlg.hDevMode);
		pDevMode->dmOrientation = landscape ? DMORIENT_LANDSCAPE : DMORIENT_PORTRAIT;
		::GlobalUnlock(dlg.hDevMode);
	}
	int copies = 1;
	if(dodlg) {
		dlg.Flags = PD_DISABLEPRINTTOFILE|PD_NOSELECTION|PD_HIDEPRINTTOFILE;
		Vector< Ptr<Ctrl> > disabled = DisableCtrls(Ctrl::GetTopCtrls());
		bool b = PrintDlg(&dlg);
		EnableCtrls(disabled);
		if(!b) return false;
		copies = dlg.nCopies;
		dlg.nCopies = 1; // because of buggy drivers for certain printers, we need to workaround copies problem
		if(dlg.hDevMode) {
			DEVMODE *pDevMode = (DEVMODE*)::GlobalLock(dlg.hDevMode);
			if(pDevMode->dmFields & DM_COPIES)
				copies = max((WORD)pDevMode->dmCopies, (WORD)copies);
			pDevMode->dmCopies = 1; // always set number of copies to 1 and deal with it by sending multiple pages to printer
			::GlobalUnlock(dlg.hDevMode);
		}
	}
	HDC hdc = NULL;
	if(dlg.hDevNames) {
		DEVNAMES *p = (DEVNAMES *)::GlobalLock(dlg.hDevNames);
		const char *driver = (const char *)p + p->wDriverOffset;
		const char *device = (const char *)p + p->wDeviceOffset;
		if(dlg.hDevMode && dlg.hDevNames) {
			DEVMODE *pDevMode = (DEVMODE*)::GlobalLock(dlg.hDevMode);
			hdc = CreateDC(driver, device, NULL, pDevMode);
			::GlobalUnlock(dlg.hDevMode);
		}
		else
			hdc = CreateDC(driver, device, NULL, NULL);
		::GlobalUnlock(dlg.hDevNames);
	}
	if(dlg.hDevMode) {
		::GlobalFree(dlg.hDevMode);
		dlg.hDevMode = NULL;
	}
	if(dlg.hDevNames) {
		::GlobalFree(dlg.hDevNames);
		dlg.hDevNames = NULL;
	}
		
	if(hdc) {
		draw = new PrintDraw(hdc, Nvl(name, Ctrl::GetAppName()));
		page.Clear();
		if(!(dlg.Flags & PD_PAGENUMS)) {
			dlg.nFromPage = dlg.nMinPage;
			dlg.nToPage = dlg.nMaxPage;
		}
		for(int n = 0; n < copies; n++)
			for(int i = dlg.nFromPage - 1; i <= dlg.nToPage - 1; i++)
				page.Add(i);
		return true;
	}
	return false;
}

bool PrinterJob::Execute()
{
	return Execute0(true);
}

Draw& PrinterJob::GetDraw()
{
	if(!draw) {
		Execute0(false);
		if(!draw)
			draw = new NilDraw;
	}
	return *draw;
}

PrinterJob& PrinterJob::MinMaxPage(int minpage, int maxpage)
{
	from = minpage + 1;
	to = maxpage + 1;
	return *this;
}

PrinterJob& PrinterJob::CurrentPage(int i)
{
	current = i + 1;
	return *this;
}

#endif

#endif

#if (defined(PLATFORM_X11) || defined(PLATFORM_COCOA)) && !defined(VIRTUALGUI)

struct PageSizeName {
	const char *name;
	int   cx, cy;
	
	Size  GetDots() const { return Size(6000 * cx / 254, 6000 * cy / 254); }
}
static const PageName2Size[] = {
	{ "A0", 841, 1189 },
	{ "A1", 594, 841 },
	{ "A2", 420, 594 },
	{ "A3", 297, 420 },
	{ "A4", 210, 297 },
	{ "A5", 148, 210 },
	{ "A6", 105, 148 },
	{ "A7", 74, 105 },
	{ "A8", 52, 74 },
	{ "A9", 37, 52 },
	{ "B0", 1030, 1456 },
	{ "B1", 728, 1030 },
	{ "B10", 32, 45 },
	{ "B2", 515, 728 },
	{ "B3", 364, 515 },
	{ "B4", 257, 364 },
	{ "B5", 182, 257 },
	{ "B6", 128, 182 },
	{ "B7", 91, 128 },
	{ "B8", 64, 91 },
	{ "B9", 45, 64 },
	{ "C5E", 163, 229 },
	{ "Comm10E", 105, 241 },
	{ "DLE", 110, 220 },
	{ "Executive", 191, 254 },
	{ "Folio", 210, 330 },
	{ "Ledger", 432, 279 },
	{ "Legal", 216, 356 },
	{ "Letter", 216, 279 },
	{ "Tabloid", 279, 432 }
};

const PageSizeName *FindPageSize(const String& name)
{
	for(int i = 0; i < __countof(PageName2Size); i++)
		if(PageName2Size[i].name == name)
			return &PageName2Size[i];
	return NULL;
}

String System(const char *cmd, const String& in)
{
	String ofn = GetTempFileName();
	String ifn = GetTempFileName();
	SaveFile(ifn, in);
	String c = cmd;
	c << " >" << ofn;
	if(in.GetCount())
		c << " <" << ifn;
	String q;
	if(system(c) >= 0)
		q = LoadFile(ofn);
	FileDelete(ofn);
	FileDelete(ifn);
	return q;
}

String System(const char *cmd)
{
	return System(cmd, Null);
}

class PrinterDlg : public WithPrinterLayout<TopWindow> {
	typedef PrinterDlg CLASSNAME;
	
public:
	void FillOpt(const String& s, const char *id, DropList& dl, bool pgsz);
	void SyncPrinterOptions();

	PrinterDlg();
	bool IsCanceled();

private:
	void StandardizePrinterName(String& printerName);
	void OnOK();
	
private:
	bool canceled;
};

void PrinterDlg::FillOpt(const String& s, const char *id, DropList& dl, bool pgsz)
{
	int a = s.Find('/');
	int b = s.Find(':');
	if(a > 0 && b > a) {
		String opt = ToLower(s.Mid(0, a));
		if(opt == id) {
			Vector<String> v = Split(~s + b + 1, ' ');
			dl.Enable();
			for(int i = 0; i < v.GetCount(); i++) {
				String o = v[i];
				if(o[0] == '*') {
					o = o.Mid(1);
					dl <<= o;
				}
				if(!pgsz || FindPageSize(o))
					dl.Add(o);
			}
		}
	}
}

void PrinterDlg::SyncPrinterOptions()
{
	Vector<String> l = Split(System("lpoptions -d " + String(~printer) + " -l"), '\n');
	paper.Disable();
	paper.Clear();
	slot.Disable();
	slot.Clear();
	for(int i = 0; i < l.GetCount(); i++) {
		FillOpt(l[i], "pagesize", paper, true);
		FillOpt(l[i], "inputslot", slot, false);
	}
}

PrinterDlg::PrinterDlg() : canceled(true)
{
	CtrlLayoutOKCancel(*this, "Print");
	printer <<= THISBACK(SyncPrinterOptions);
	ok <<= THISBACK(OnOK);
	npage.Add(1);
	npage.Add(2);
	npage.Add(4);
	npage.Add(6);
	npage.Add(9);
	npage.Add(16);
	npage <<= 1;
	copies <<= 1;
	landscape <<= 0;
	range <<= 0;
	Vector<String> l = Split(System("lpstat -a"), '\n');
	for(int i = 0; i < l.GetCount(); i++) {
		Vector<String> w = Split(l[i], ' ');
		if(w.GetCount())
			printer.Add(w[0]);
	}
	String h = System("lpstat -d");
	int q = h.Find(':');
	if(q >= 0) {
		String p = h.Mid(q + 1);
		StandardizePrinterName(p);
		if(printer.HasKey(p)) {
			printer <<= p;
			SyncPrinterOptions();
			return;
		}
	}
	if(printer.GetCount()) {
		printer.SetIndex(0);
		SyncPrinterOptions();
	}
}

bool PrinterDlg::IsCanceled()
{
	return canceled;
}

void PrinterDlg::StandardizePrinterName(String& printerName)
{
	printerName.Replace(" ", "");
	printerName.Replace("\r", "");
	printerName.Replace("\n", "");
}

void PrinterDlg::OnOK()
{
	canceled = false;
	Close();
}

Size PrinterJob::GetDefaultPageSize(String *name)
{
	Size sz(6000 * 210 / 254, 6000 * 297 / 254);

	Vector<String> dpp = Split(System("lpoptions -l"), '\n');

	for (int i = 0; i < dpp.GetCount(); i++){
		int pos = max(dpp[i].FindAfter("Page Size"), dpp[i].FindAfter("PageSize"));
		if (pos >= 0){
			pos = dpp[i].Find('*', pos);
			//return A4 if there is not default page size
			if (pos < 0) return sz;
			//skip '*'
			pos++;
			int len = dpp[i].Find(' ', pos);
			if (len < 0) len = dpp[i].GetLength();
			len -= pos;
			//page name
			String nm = dpp[i].Mid(pos, len);
			if(name)
				*name = nm;
			const PageSizeName *p = FindPageSize(nm);
			if(p) {
				sz = p->GetDots();
				return sz;
			}
		}
	}
	//return A4 if there is not default page size
	return sz;
}

PrinterJob::PrinterJob(const char *_name)
{
	name = _name;
	landscape = false;
	from = to = 0;
	current = 0;
	pgsz = GetDefaultPageSize();
	dlgSuccess = true;
}

PrinterJob::~PrinterJob()
{
}

bool PrinterJob::Execute0()
{
	PrinterDlg dlg;
	dlg.from <<= from + 1;
	dlg.to <<= to + 1;
	dlg.from.Min(from + 1).Max(to + 1);
	dlg.to.Min(from + 1).Max(to + 1);
	dlg.from.Enable(from != to);
	dlg.to.Enable(from != to);
	dlg.range.EnableCase(1, from != to);
	dlg.range.EnableCase(2, from != to);
	dlg.landscape <<= landscape;
	String h;
	GetDefaultPageSize(&h);
	h.IsEmpty() ? dlg.paper <<= "A4" : dlg.paper <<= h;
	
	dlg.Run();
	if(dlg.IsCanceled())
		return false;
	
	options.Clear();
	options << "-d " << ~dlg.printer;
	options << " -o media=";
	dlg.paper.GetIndex() < 0 ? options << ~dlg.slot : options << ~dlg.paper << "," << ~dlg.slot;
	landscape = dlg.landscape;
	options << " -o number-up=" << ~dlg.npage;
	options << " -n " << ~dlg.copies;
	if(dlg.collate)
		options << " -o Collate=True";
	page.Clear();
	switch(dlg.range) {
	case 0:
		for(int i = from; i <= to; i++)
			page.Add(i);
		break;
	case 1:
		page.Add(current);
		break;
	case 2:
		for(int i = (int)~dlg.from - 1; i <= (int)~dlg.to - 1; i++)
			page.Add(i);
		break;
	}
	pgsz = Size(5100, 6600);

	const PageSizeName *p = FindPageSize(~dlg.paper);
	if(p)
		pgsz = p->GetDots();

	return true;
}

bool PrinterJob::Execute()
{
	dlgSuccess = Execute0();
	return dlgSuccess;
}

struct PrinterDraw : PdfDraw {
	String options;
	bool canceled;
	
	PrinterDraw(Size sz) : PdfDraw(sz), canceled(true) {}
	~PrinterDraw() {
		if(!canceled && !IsEmpty()) {
			System("lp " + options, Finish());
		}
	}
};

Draw& PrinterJob::GetDraw()
{
	if(!draw) {
		PrinterDraw *pd = new PrinterDraw(landscape ? Size(pgsz.cy, pgsz.cx) : pgsz);
		pd->canceled = !dlgSuccess;
		pd->options = options;
		if(landscape)
			pd->options << " -o landscape";
		draw = pd;
	}
	return *draw;
}

PrinterJob& PrinterJob::MinMaxPage(int minpage, int maxpage)
{
	from = minpage;
	to = maxpage;
	return *this;
}

PrinterJob& PrinterJob::CurrentPage(int i)
{
	current = i;
	return *this;
}

#endif

}
