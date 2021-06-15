#ifndef flagSKELETON

#include <CtrlLib/CtrlLib.h>
#include <CtrlCore/CocoMM.h>

namespace Upp {

#ifdef GUI_COCOA

FileSelNative::FileSelNative()
{
	confirm = true;
	multi = hidden = false;
	activetype = 0;
}

FileSelNative& FileSelNative::AllFilesType()
{
	return Type(t_("All files"), "*.*");
}

bool FileSelNative::Execute(bool open, const char *title)
{
	path.Clear();
	Ctrl::ReleaseCtrlCapture();
	if(!title)
		title = open ? t_("Open") : t_("Save as");
	CFRef<CFStringRef> mmtitle = CFStringCreateWithCString(NULL, title, kCFStringEncodingUTF8);

	NSWindow *window = nil;
	Ctrl *win = Ctrl::GetActiveWindow();
	if(win)
		window = (NSWindow *)win->GetNSWindow();
	
	if(open) {
		NSOpenPanel *panel = [NSOpenPanel openPanel];
		[panel setAllowsMultipleSelection: multi ? YES : NO];
		[panel setMessage:(NSString *)~mmtitle];
		if([panel runModal] == NSModalResponseOK) {
			NSArray* urls = [panel URLs];
			for(int i = 0; i < urls.count; i++)
				path.Add([[urls objectAtIndex:i] fileSystemRepresentation]);
		}
	}
	else {
		NSSavePanel *panel = [NSSavePanel savePanel];
		[panel setMessage:(NSString *)~mmtitle];
		if([panel runModal] == NSModalResponseOK)
			path.Add([[panel URL] fileSystemRepresentation]);
	}
	return path.GetCount();
}

#endif

}
#endif