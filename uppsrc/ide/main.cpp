#include "ide.h"
#include "CommandLineHandler.h"

#define FUNCTION_NAME UPP_FUNCTION_NAME << "(): "

void DelTemps()
{
	FindFile ff(ConfigFile("*.tmp"));
	while(ff) {
		DeleteFile(ConfigFile(ff.GetName()));
		ff.Next();
	}
}

#ifdef PLATFORM_WIN32
#include <mmsystem.h>
#pragma comment( lib, "winmm.lib" )
#endif

extern int MemoryProbeFlags;

void Uninstall();

bool SilentMode;

#if defined(PLATFORM_WIN32)
#include <wincon.h>

void Puts(const char *s)
{
	dword dummy;
	if(!SilentMode)
		WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), s, (int)strlen(s), &dummy, NULL);
}
#endif


#ifdef PLATFORM_POSIX
void Puts(const char *s)
{
	if(!SilentMode)
		puts(s);
}
#endif

bool splash_screen;

int CommaSpace(int c)
{
	return c == ',' ? ' ' : c;
}

void ReduceCache()
{
	String cfgdir = ConfigFile("cfg");
	FindFile ff(AppendFileName(cfgdir, "*.*"));
	while(ff) {
		if(ff.IsFile()) {
			String fn = ff.GetName();
			String ext = GetFileExt(fn);
			if(ext != ".aux" && ext != ".cfg")
				if((Date)Time(ff.GetLastAccessTime()) < GetSysDate() - 14)
					DeleteFile(AppendFileName(cfgdir, fn));
		}
		ff.Next();
	}
}

bool IsAssembly(const String& s)
{
	Vector<String> varlist;
	for(FindFile ff(ConfigFile("*.var")); ff; ff.Next())
		if(ff.IsFile())
			if(GetFileTitle(ff.GetName()) == s)
				return true;
	Vector<String> l = Split(s, ',');
	for(int i = 0; i < l.GetCount(); i++)
		if(FindFile(NormalizePath(l[i])).IsFolder())
			return true;
	return false;
}

void StartEditorMode(const Vector<String>& args, Ide& ide, bool& clset)
{
	if(args.IsEmpty() || clset) {
		return;
	}
	
	Vector<String> dir = Split(LoadFile(GetHomeDirFile("usc.path")), ';');
	for(int i = 0; i < dir.GetCount(); i++)
		ide.UscProcessDirDeep(dir[i]);
	for(int i = 0; i < args.GetCount(); i++) {
		if(args[i] != "-f") {
			String file_path = NormalizePath(args[i]);
			
			Logd() << FUNCTION_NAME << "Opening file \"" << file_path << "\".";
			
			ide.EditFile(file_path);
			ide.FileSelected();
		}
	}
	
	clset = true;
	ide.EditorMode();
}

// TODO: I do not like that we need to define macro here.
// I opt for std::function version. We need to fix that API
// in 2018.2 release. #1901

#undef  GUI_APP_MAIN_HOOK
#define GUI_APP_MAIN_HOOK \
{ \
	BaseCommandLineHandler cmd_handler(CommandLine()); \
	if (cmd_handler.Handle()) \
		return Upp::GetExitCode(); \
}

#ifdef flagMAIN
GUI_APP_MAIN
#else
void AppMain___()
#endif
{
//	Ctrl::ShowRepaint(50);

#ifdef flagPEAKMEM
	PeakMemoryProfile();
#endif

	Logi() << UPP_FUNCTION_NAME << "(): " << SplashCtrl::GenerateVersionInfo(' ');

	Ctrl::SetUHDEnabled();
	Ctrl::SetDarkThemeEnabled();
	Ctrl::SetAppName("TheIDE");

	SetLanguage(LNG_ENGLISH);
	SetDefaultCharset(CHARSET_UTF8);

	MainCommandLineHandler cmd_handler(CommandLine());
	if (cmd_handler.Handle())
		return;
	auto arg = clone(cmd_handler.GetArgs());

	SetVppLogSizeLimit(200000000);
	
	if(!CheckLicense())
		return;
#ifdef PLATFORM_WIN32
	AutoInstantSetup();
#endif

	bool hasvars = FindFile(ConfigFile("*.var"));

#ifdef PLATFORM_POSIX
	String home = Environment().Get("UPP_HOME", Null);
	if(!IsNull(home))
		SetHomeDirectory(home);
	if(!hasvars) {
		if(!Install(hasvars))
			return;
		SaveFile(ConfigFile("version"), IDE_VERSION);
	}
#endif

	if(!hasvars)
		SetupSVNTrunk();

	if(!FileExists(BlitzBaseFile()))
		ResetBlitz();
	
	if(!FileExists(SearchEnginesFile()))
		SearchEnginesDefaultSetup();

	bool dosplash = true;
	for(int i = 0; i < arg.GetCount(); i++)
		if(arg[i] == "--nosplash") {
			dosplash = false;
			arg.Remove(i);
			break;
		}
	for(int i = 0; i < arg.GetCount(); i++) {
	#ifdef PLATFORM_WIN32
		if(arg[i] == "!") {
			String cmdline;
			for(++i; i < arg.GetCount(); i++) {
				if(!IsNull(cmdline))
					cmdline << ' ';
				String h = arg[i];
				if(h.Find(' ') >= 0)
					cmdline << '\"' << arg[i] << '\"';
				else
					cmdline << arg[i];
			}
			int n = cmdline.GetLength() + 1;
			Buffer<char> cmd(n);
			memcpy(cmd, cmdline, n);
			SECURITY_ATTRIBUTES sa;
			sa.nLength = sizeof(SECURITY_ATTRIBUTES);
			sa.lpSecurityDescriptor = NULL;
			sa.bInheritHandle = TRUE;
			PROCESS_INFORMATION pi;
			STARTUPINFO si;
			ZeroMemory(&si, sizeof(STARTUPINFO));
			si.cb = sizeof(STARTUPINFO);
			AllocConsole();
			SetConsoleTitle(cmdline);
			int time = msecs();
			if(CreateProcess(NULL, cmd, &sa, &sa, TRUE,
				             NORMAL_PRIORITY_CLASS,
			                 NULL, NULL, &si, &pi)) {
				WaitForSingleObject(pi.hProcess, INFINITE);
				dword exitcode = 255;
				GetExitCodeProcess(pi.hProcess, &exitcode);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				Puts("<--- Finished in " + GetPrintTime(time) + ", exitcode: " + AsString(exitcode) + " --->");
			}
			else
				Puts("Unable to launch " + cmdline);
			char h[1];
			dword dummy;
			ReadFile(GetStdHandle(STD_INPUT_HANDLE), h, 1, &dummy, NULL);
			return;
		}
	#endif
	}

#ifdef _DEBUG0
#ifdef PLATFORM_WIN32
	InstantSetup();
#endif
#endif

#ifndef _DEBUG
	try {
#endif
		void RegisterLayDes(); RegisterLayDes();
		void RegisterIconDes(); RegisterIconDes();

		splash_screen = true;
		
		{ // setup skin and font before the 'real' Ide gets constructed
			Ide h;
			LoadFromFile(h);
			if(h.gui_font_override)
				SetStdFont(h.gui_font);
			auto v = GetAllChSkins();
			Ctrl::SetSkin(v[clamp(h.chstyle, 0, v.GetCount() - 1)].a);
		}

		Ide ide;
		ide.Maximize();
		bool clset = false;
		
		ide.LoadConfig();
		
		if(arg.GetCount() == 1) {
			if(arg[0].EndsWith(".upp")) {
				Vector<String> names = Split(arg[0], DIR_SEP);
				int last = names.GetCount() - 1;
				for(int i = last; i >= 0; --i) {
					if (IsAssembly(names[i])) {
						String package = names[last];
						package.Remove(package.GetCount() - 4, 4);
						LoadVars(names[i]);
						ide.SetMain(package);
						clset = true;
						break;
					}
				}
			}
		} else {
			if(arg.GetCount() == 2 && IsAssembly(arg[0])) {
				LoadVars(arg[0]);
				ide.SetMain(arg[1]);
				clset=true;
			}
		}
		
		ide.LoadAbbr();

		DelTemps();

#ifndef PLATFORM_COCOA // MacOS sends some weird things to app bundle - disable editor mode for now
		StartEditorMode(arg, ide, clset);
#endif

		if(splash_screen && dosplash && !ide.IsEditorMode()) {
			ShowSplash();
			Ctrl::ProcessEvents();
		}

		ide.editor_bottom.Zoom(0);
		ide.right_split.Zoom(0);
		if(FileExists(ConfigFile("developide"))) {
	#ifdef PLATFORM_WIN32
			InstallCrashDump();
	#endif
			Ini::user_log = true;
		}
		
		String ppdefs = ConfigFile("global.defs");
	#ifndef _DEBUG
		if(!FileExists(ppdefs))
	#endif
			SaveFile(ppdefs, GetStdDefs());

		SetPPDefs(LoadFile(ppdefs));
		
		if(!clset)
			ide.LoadLastMain();
		do {
			IdeAgain = false;
			if(clset || ide.OpenMainPackage()) {
				ide.SaveLastMain();
				ide.isscanning++;
				ide.MakeTitle();
				if(!ide.IsEditorMode())
					SyncRefs();
				ide.FileSelected();
				ide.isscanning--;
				ide.MakeTitle();
				if(!IdeExit)
					ide.Run();
				ide.SaveConfigOnTime();
				ide.SaveLastMain();
				ide.Close();
			}
		}
		while(IdeAgain);

		SaveCodeBase();
		DelTemps();
		DeletePCHFiles();
		ReduceCache();
#ifndef _DEBUG
	}
	catch(const CParser::Error& e) {
		ErrorOK("Parser error " + e);
		LOG("!!!!! Parser error " + e);
	}
	catch(const Exc& e) {
		ErrorOK("Exception " + e);
		LOG("!!!!! Exception " << e);
	}
#ifdef PLATFORM_POSIX
	catch(...) {
		ErrorOK("Unknown exception !");
		LOG("!!!!! Unknown exception");
	}
#endif
#endif
}

#ifdef flagPEAKMEM
EXITBLOCK {
	RDUMP(*PeakMemoryProfile());
	RDUMP(MemoryProfile());
	MemoryDump();
}
#endif
