#include <CtrlCore/CtrlCore.h>

#ifdef GUI_COCOA

#define LLOG(x) // DLOG(x)

NAMESPACE_UPP

void Ctrl::GuiPlatformConstruct()
{
}

void Ctrl::GuiPlatformDestruct()
{
}

void Ctrl::GuiPlatformRemove()
{
}

bool Ctrl::GuiPlatformRefreshFrameSpecial(const Rect& r)
{
	return false;
}

bool Ctrl::GuiPlatformSetFullRefreshSpecial()
{
	return false;
}

String GuiPlatformGetKeyDesc(dword key)
{
	return Null;
}

void Ctrl::GuiPlatformSelection(PasteClip&)
{
}

void GuiPlatformAdjustDragImage(ImageBuffer&)
{
}

bool GuiPlatformHasSizeGrip()
{
	return true;
}

void GuiPlatformGripResize(TopWindow *q)
{
}

Color GuiPlatformGetScreenPixel(int x, int y)
{
	return Black;
}

void GuiPlatformAfterMenuPopUp()
{
}

String Ctrl::Name() const {
	GuiLock __;
#ifdef CPU_64
	String s = String(typeid(*this).name()) + " : 0x" + FormatIntHex(this);
#else
	String s = String(typeid(*this).name()) + " : " + Format("0x%x", (int) this);
#endif
	if(IsChild())
		s << "(parent " << String(typeid(*parent).name()) << ")";
	return s;
}

void Ctrl::InstallPanicBox()
{
}

END_UPP_NAMESPACE

#endif
