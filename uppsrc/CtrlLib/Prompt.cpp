#include "CtrlLib.h"

namespace Upp {

struct PromptDlgWnd__ : TopWindow {
	bool    esc;
	Button *b;

	virtual bool HotKey(dword key) {
		if(TopWindow::HotKey(key))
			return true;
		if(IsAlpha(key))
			return TopWindow::HotKey(K_ALT_A + ToUpper((int)key) - 'A');
		if(key == K_ESCAPE && esc)
			b->PseudoPush();
		return false;
	}
};

static void sAdd(Ctrl& dlg, int fcy, int bcy, int& bx, int bcx, int gap, Button& b, const char *button, const Image& img)
{
	if(button) {
		dlg << b.BottomPos(fcy, bcy).LeftPos(bx, bcx);
		b.SetLabel(button);
		if(!IsNull(img))
			b.SetImage(img);
		bx += gap + bcx;
	}
}

void sExecutePrompt(PromptDlgWnd__ *dlg, int *result)
{
	dlg->Open();
	Vector<Ctrl *> wins = Ctrl::GetTopWindows();
	for(int i = 0; i < wins.GetCount(); i++) {
		TopWindow *w = dynamic_cast<TopWindow *>(wins[i]);
		if(w && w->IsTopMost()) {
			dlg->TopMost();
			break;
		}
	}
	*result = dlg->RunAppModal();
	dlg->Close();
}
                        
RedirectPromptFn RedirectPrompt;

void RedirectPrompts(RedirectPromptFn r)
{
	RedirectPrompt = r;
}

VectorMap<String, int> dsa_history;

void ClearPromptOptHistory()
{
	GuiLock __;
	dsa_history.Clear();
}

void ClearPromptOptHistory(Gate<String> filter)
{
	GuiLock __;
	for(int i = 0; i < dsa_history.GetCount(); i++)
		if(filter(dsa_history.GetKey(i)))
			dsa_history.Unlink(i);
	dsa_history.Sweep();
}

void SerializePromptOptHistory(Stream& s)
{
	int version = 0;
	s / version;
	s % dsa_history;
}

int Prompt(bool dontshowagain, const char *dsa_id_, int beep,
           Event<const String&> WhenLink,
           const char *title, const Image& iconbmp, const char *qtf, bool okcancel,
           const char *button1, const char *button2, const char *button3,
		   int cx,
		   Image im1, Image im2, Image im3)
{
	if(RedirectPrompt)
		return (*RedirectPrompt)(WhenLink, title, iconbmp, qtf, okcancel,
                                 button1, button2, button3,
                                 cx, im1, im2, im3);
	String dsa_id;
	Option dsa;
	if(dontshowagain) {
		dsa_id = dsa_id_;
		if(IsNull(dsa_id)) {
			String body = title;
			body << "\1" << qtf;
			if(button1) body << "\1" << button1;
			if(button2) body << "\1" << button2;
			if(button3) body << "\1" << button3;
			dsa_id = SHA256String(body);
		}
		GuiLock __;
		int q = dsa_history.Find(dsa_id);
		if(q >= 0)
			return dsa_history[q];
		dsa.SetLabel(t_("Do not show this again"));
	}
	switch(beep) {
	case BEEP_INFORMATION:
		BeepInformation();
		break;
	case BEEP_EXCLAMATION:
		BeepExclamation();
		break;
	case BEEP_QUESTION:
		BeepQuestion();
		break;
	case BEEP_ERROR:
		BeepError();
		break;
	}
	int fcy = Draw::GetStdFontCy();
	EnterGuiMutex(); // Ctrl derived classes can only be initialized with GuiLock
	PromptDlgWnd__ dlg;
	RichTextCtrl qtfctrl;
	Icon         icon;
	qtfctrl.WhenLink = WhenLink;
	icon.SetImage(iconbmp);
	Button b1, b2, b3;
	qtfctrl.SetQTF(String("[g ") + qtf, GetRichTextStdScreenZoom());
	int bcy = Ctrl::VertLayoutZoom(24);
	int bcx = Ctrl::HorzLayoutZoom(72);
	if(button1)
		bcx = max(2 * fcy + GetTextSize(button1, Draw::GetStdFont()).cx, bcx);
	if(button2)
		bcx = max(2 * fcy + GetTextSize(button2, Draw::GetStdFont()).cx, bcx);
	if(button3)
		bcx = max(2 * fcy + GetTextSize(button3, Draw::GetStdFont()).cx, bcx);
	Size bsz = icon.GetStdSize();
	if(cx == 0) {
		cx = qtfctrl.GetWidth();
		if(!cx)
			cx = 350;
		if(dontshowagain)
			cx = max(cx, dsa.GetMinSize().cx);
		cx += 2 * fcy;
		if(bsz.cx)
			cx += bsz.cx + fcy;
	}
	int nbtn = !!button1 + !!button2 + !!button3;
	dlg.esc = okcancel && nbtn == 1;
	cx = min(Zx(520), max(nbtn * bcx + (1 + nbtn) * fcy, cx));
	int qcx = cx - 2 * fcy;
	if(bsz.cx)
		qcx -= bsz.cx + fcy;
	int ccy = qtfctrl.GetHeight(qcx);
	int qcy = min(Zy(400), ccy);
	if(qcy <= ccy) {
		qcx += ScrollBarSize() + fcy;
		cx += ScrollBarSize() + fcy;
	}
	int mcy = max(qcy, bsz.cy);
	int cy = mcy + 48 * fcy / 10;
	if(dontshowagain)
		cy += fcy;
	dlg.SetRect(Size(cx, cy));
	dlg << icon.TopPos(fcy, bsz.cy).LeftPos(fcy, bsz.cx);
	dlg << qtfctrl.TopPos(fcy + (mcy - qcy) / 2, qcy).RightPos(fcy, qcx);
	if(okcancel) {
		b1.Ok();
		if(nbtn == 2)
			b2.Cancel();
		if(nbtn == 3)
			b3.Cancel();
	}
	b1.WhenAction = dlg.Breaker(1);
	b2.WhenAction = dlg.Breaker(0);
	b3.WhenAction = dlg.Breaker(-1);
	dlg.b = &b1;
	int bx = bcx;
	int gap = fcy / 2;
	fcy = 8 * fcy / 10;
	if(button2)
		bx += gap + bcx;
	if(button3)
		bx += gap + bcx;
	bx = (cx - bx) / 2;
	if(dontshowagain) {
		dlg << dsa.BottomPos(bcy + 2 * fcy).RightPos(fcy, qcx);
	}
	if(SwapOKCancel()) {
		sAdd(dlg, fcy, bcy, bx, bcx, gap, b2, button2, im2);
		sAdd(dlg, fcy, bcy, bx, bcx, gap, b3, button3, im3);
		sAdd(dlg, fcy, bcy, bx, bcx, gap, b1, button1, im1);
	}
	else {
		sAdd(dlg, fcy, bcy, bx, bcx, gap, b1, button1, im1);
		sAdd(dlg, fcy, bcy, bx, bcx, gap, b2, button2, im2);
		sAdd(dlg, fcy, bcy, bx, bcx, gap, b3, button3, im3);
	}
	dlg.WhenClose = dlg.Breaker(button3 ? -1 : 0);
	dlg.Title(title);
	LeaveGuiMutex();
	int result;
	Ctrl::Call(callback2(sExecutePrompt, &dlg, &result));
	if(dontshowagain && dsa)
		dsa_history.Add(dsa_id, result);
	return result;
}

int Prompt(Event<const String&> WhenLink,
           const char *title, const Image& icon, const char *qtf, bool okcancel,
           const char *button1, const char *button2, const char *button3,
		   int cx, Image im1, Image im2, Image im3)
{
	return Prompt(false, NULL, BEEP_NONE, WhenLink, title, icon, qtf, okcancel, button1, button2, button3, cx, im1, im2, im3);
}

int Prompt(const char *title, const Image& icon, const char *qtf, bool okcancel,
           const char *button1, const char *button2, const char *button3,
		   int cx)
{
	return Prompt(callback(LaunchWebBrowser), title,
	              icon, qtf, okcancel, button1, button2, button3, cx, Null, Null, Null);
}

int Prompt(const char *title, const Image& icon, const char *qtf,
           const char *button1, const char *button2, const char *button3,
		   int cx)
{
	return Prompt(title, icon, qtf, true, button1, button2, button3, cx);
}

void PromptOK(const char *qtf) {
	BeepInformation();
	Prompt(Ctrl::GetAppName(), CtrlImg::information(), qtf, t_("OK"));
}

void Exclamation(const char *qtf) {
	BeepExclamation();
	Prompt(Ctrl::GetAppName(), CtrlImg::exclamation(), qtf, t_("OK"));
}

void ShowExc(const Exc& exc) {
	BeepExclamation();
	Prompt(Ctrl::GetAppName(), CtrlImg::exclamation(), DeQtf(exc), t_("OK"));
}

void ErrorOK(const char *qtf) {
	BeepError();
	Prompt(Ctrl::GetAppName(), CtrlImg::error(), qtf, t_("OK"));
}

int PromptOKCancel(const char *qtf) {
	BeepQuestion();
	return Prompt(Ctrl::GetAppName(), CtrlImg::question(), qtf, t_("OK"), t_("Cancel"));
}

int PromptOKCancelAll(const char *qtf) {
	BeepQuestion();
	return Prompt(Ctrl::GetAppName(), CtrlImg::question(), qtf, t_("OK"), t_("Cancel"), t_("All"));
}

int ErrorOKCancel(const char *qtf) {
	BeepError();
	return Prompt(Ctrl::GetAppName(), CtrlImg::error(), qtf, t_("OK"), t_("Cancel"));
}

CH_IMAGE(YesButtonImage, Null);
CH_IMAGE(NoButtonImage, Null);
CH_IMAGE(AbortButtonImage, Null);
CH_IMAGE(RetryButtonImage, Null);

int PromptYesNo(const char *qtf) {
	BeepQuestion();
	return Prompt(callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::question(), qtf, false,
	              t_("&Yes"), t_("&No"), NULL, 0,
	              YesButtonImage(), NoButtonImage(), Null);
}

int PromptYesNoAll(const char *qtf) {
	BeepQuestion();
	return Prompt(callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::question(), qtf, false,
	              t_("&Yes"), t_("&No"), t_("All"), 0,
	              YesButtonImage(), NoButtonImage(), Null);
}

int ErrorYesNo(const char *qtf) {
	BeepError();
	return Prompt(callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::error(), qtf, false,
	              t_("&Yes"), t_("&No"), NULL, 0,
	              YesButtonImage(), NoButtonImage(), Null);
}

int PromptYesNoCancel(const char *qtf) {
	BeepQuestion();
	return Prompt(callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::question(), qtf, true,
	              t_("&Yes"), t_("&No"), t_("Cancel"), 0,
	              YesButtonImage(), NoButtonImage(), Null);
}

int ErrorYesNoCancel(const char *qtf) {
	BeepError();
	return Prompt(callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::error(), qtf, true,
	              t_("&Yes"), t_("&No"), t_("Cancel"), 0,
	              YesButtonImage(), NoButtonImage(), Null);
}

int ErrorYesNoAll(const char *qtf) {
	BeepError();
	return Prompt(callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::error(), qtf, true,
	              t_("&Yes"), t_("&No"), t_("All"), 0,
	              YesButtonImage(), NoButtonImage(), Null);
}

int PromptAbortRetry(const char *qtf) {
	BeepExclamation();
	return Prompt(callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::exclamation(), qtf, false,
	              t_("&Abort"), t_("&Retry"), NULL, 0,
	              AbortButtonImage(), RetryButtonImage(), Null);
}

int ErrorAbortRetry(const char *qtf) {
	BeepError();
	return Prompt(callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::error(), qtf, false,
	              t_("&Abort"), t_("&Retry"), NULL, 0,
	              AbortButtonImage(), RetryButtonImage(), Null);
}

int PromptRetryCancel(const char *qtf) {
	BeepExclamation();
	return Prompt(callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::exclamation(), qtf, true,
	              t_("&Retry"), t_("Cancel"), NULL, 0,
	              RetryButtonImage(), Null, Null);
}

int ErrorRetryCancel(const char *qtf) {
	BeepError();
	return Prompt(callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::error(), qtf, true,
	              t_("&Retry"), t_("Cancel"), NULL, 0,
	              RetryButtonImage(), Null, Null);
}

int PromptAbortRetryIgnore(const char *qtf) {
	BeepExclamation();
	return Prompt(callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::exclamation(), qtf, false,
	              t_("&Abort"), t_("&Retry"), t_("&Ignore"), 0,
	              AbortButtonImage(), RetryButtonImage(), Null);
}

int ErrorAbortRetryIgnore(const char *qtf) {
	BeepError();
	return Prompt(callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::error(), qtf, false,
	              t_("&Abort"), t_("&Retry"), t_("&Ignore"), 0,
	              AbortButtonImage(), RetryButtonImage(), Null);
}

int PromptSaveDontSaveCancel(const char *qtf) {
	BeepQuestion();
	return Prompt(callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::question(), qtf, true,
	              t_("&Save"), t_("&Don't Save"), t_("Cancel"), 0,
	              CtrlImg::save(), NoButtonImage(), Null);
}

int PromptOpt(const char *opt_id, int beep, Event<const String&> WhenLink,
              const char *title, const Image& icon, const char *qtf, bool okcancel,
              const char *button1, const char *button2, const char *button3,
		      int cx, Image im1, Image im2, Image im3)
{
	return Prompt(true, opt_id, beep, WhenLink, title, icon, qtf, okcancel, button1, button2, button3, cx, im1, im2, im3);
}

int PromptOpt(const char *opt_id, int beep,
              const char *title, const Image& icon, const char *qtf, bool okcancel,
              const char *button1, const char *button2, const char *button3,
		      int cx)
{
	return PromptOpt(opt_id, beep, callback(LaunchWebBrowser), title,
	              icon, qtf, okcancel, button1, button2, button3, cx, Null, Null, Null);
}

int PromptOpt(const char *opt_id, int beep,
              const char *title, const Image& icon, const char *qtf,
              const char *button1, const char *button2, const char *button3,
		      int cx)
{
	return PromptOpt(opt_id, beep, title, icon, qtf, true, button1, button2, button3, cx);
}

void PromptOKOpt(const char *qtf, const char *opt_id) {
	PromptOpt(opt_id, BEEP_INFORMATION, Ctrl::GetAppName(), CtrlImg::information(), qtf, t_("OK"));
}

void ExclamationOpt(const char *qtf, const char *opt_id) {
	PromptOpt(opt_id, BEEP_EXCLAMATION,  Ctrl::GetAppName(), CtrlImg::exclamation(), qtf, t_("OK"));
}

void ShowExcOpt(const Exc& exc, const char *opt_id) {
	PromptOpt(opt_id, BEEP_EXCLAMATION, Ctrl::GetAppName(), CtrlImg::exclamation(), DeQtf(exc), t_("OK"));
}

void ErrorOKOpt(const char *qtf, const char *opt_id) {
	PromptOpt(opt_id, BEEP_ERROR, Ctrl::GetAppName(), CtrlImg::error(), qtf, t_("OK"));
}

int PromptOKCancelOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_QUESTION, Ctrl::GetAppName(), CtrlImg::question(), qtf, t_("OK"), t_("Cancel"));
}

int PromptOKCancelAllOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_QUESTION, Ctrl::GetAppName(), CtrlImg::question(), qtf, t_("OK"), t_("Cancel"), t_("All"));
}

int ErrorOKCancelOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_ERROR, Ctrl::GetAppName(), CtrlImg::error(), qtf, t_("OK"), t_("Cancel"));
}

int PromptYesNoOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_QUESTION, callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::question(), qtf, false,
	              t_("&Yes"), t_("&No"), NULL, 0,
	              YesButtonImage(), NoButtonImage(), Null);
}

int PromptYesNoAllOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_QUESTION, callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::question(), qtf, false,
	              t_("&Yes"), t_("&No"), t_("All"), 0,
	              YesButtonImage(), NoButtonImage(), Null);
}

int ErrorYesNoOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_ERROR, callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::error(), qtf, false,
	              t_("&Yes"), t_("&No"), NULL, 0,
	              YesButtonImage(), NoButtonImage(), Null);
}

int PromptYesNoCancelOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_QUESTION, callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::question(), qtf, true,
	              t_("&Yes"), t_("&No"), t_("Cancel"), 0,
	              YesButtonImage(), NoButtonImage(), Null);
}

int ErrorYesNoCancelOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_ERROR, callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::error(), qtf, true,
	              t_("&Yes"), t_("&No"), t_("Cancel"), 0,
	              YesButtonImage(), NoButtonImage(), Null);
}

int ErrorYesNoAllOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_ERROR, callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::error(), qtf, true,
	              t_("&Yes"), t_("&No"), t_("All"), 0,
	              YesButtonImage(), NoButtonImage(), Null);
}

int PromptAbortRetryOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_EXCLAMATION, callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::exclamation(), qtf, false,
	              t_("&Abort"), t_("&Retry"), NULL, 0,
	              AbortButtonImage(), RetryButtonImage(), Null);
}

int ErrorAbortRetryOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_ERROR, callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::error(), qtf, false,
	              t_("&Abort"), t_("&Retry"), NULL, 0,
	              AbortButtonImage(), RetryButtonImage(), Null);
}

int PromptRetryCancelOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_EXCLAMATION, callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::exclamation(), qtf, true,
	              t_("&Retry"), t_("Cancel"), NULL, 0,
	              RetryButtonImage(), Null, Null);
}

int ErrorRetryCancelOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_ERROR, callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::error(), qtf, true,
	              t_("&Retry"), t_("Cancel"), NULL, 0,
	              RetryButtonImage(), Null, Null);
}

int PromptAbortRetryIgnoreOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_EXCLAMATION, callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::exclamation(), qtf, false,
	              t_("&Abort"), t_("&Retry"), t_("&Ignore"), 0,
	              AbortButtonImage(), RetryButtonImage(), Null);
}

int ErrorAbortRetryIgnoreOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_ERROR, callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::error(), qtf, false,
	              t_("&Abort"), t_("&Retry"), t_("&Ignore"), 0,
	              AbortButtonImage(), RetryButtonImage(), Null);
}

int PromptSaveDontSaveCancelOpt(const char *qtf, const char *opt_id) {
	return PromptOpt(opt_id, BEEP_QUESTION, callback(LaunchWebBrowser),
	              Ctrl::GetAppName(), CtrlImg::question(), qtf, true,
	              t_("&Save"), t_("&Don't Save"), t_("Cancel"), 0,
	              CtrlImg::save(), NoButtonImage(), Null);
}

}
