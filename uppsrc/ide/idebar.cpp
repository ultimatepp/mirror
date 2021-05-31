#include "ide.h"

#define KEYGROUPNAME "Ide"
#define KEYNAMESPACE IdeKeys
#define KEYFILE      <ide/ide.key>
#include             <CtrlLib/key_source.h>

using namespace IdeKeys;

void Ide::DoEditKeys()
{
	EditKeys();
	AKEditor();
}

void Ide::AKEditor()
{
	CodeEditor::find_next_key = AK_FINDNEXT().key[0];
	CodeEditor::find_prev_key = AK_FINDPREV().key[0];
	CodeEditor::replace_key = AK_DOREPLACE().key[0];
}

void Ide::PackageMenu(Bar& menu)
{
	Project(menu);
}

void Ide::FileBookmark(Bar& menu)
{
	int i;
	for(i = 0; i < 10; i++) {
		const Bookmark& b = bookmark[i];
		String txt = Format("Goto bookmark &%d", i);
		if(!b.file.IsEmpty())
			txt << " (" << bookmark[i].file << ')';
		menu.Add(!b.file.IsEmpty(), txt, THISBACK1(BookKey, K_CTRL_0 + i))
			.Key(K_CTRL_0 + i);
	}
	menu.MenuBreak();
	for(i = 0; i < 10; i++)
		menu.Add("Set", THISBACK1(BookKey, K_SHIFT_CTRL_0 + i))
		    .Key(K_SHIFT_CTRL_0 + i);
}

void Ide::File(Bar& menu)
{
	if (!IsEditorMode())
	{
		menu.Add(AK_SETMAIN, IdeImg::MainPackage(), THISBACK(NewMainPackage))
			.Enable(!IdeIsDebugLock())
			.Help("Select global configuration (var), select / add main project package");
	}
	
	menu.AddMenu(AK_EDITFILE, CtrlImg::open(), THISBACK(EditAnyFile))
		.Help("Select any file in file selector and open it in editor");
	menu.AddMenu(!IsNull(GetOpposite()), AK_OPPOSITE, IdeImg::opposite(), THISBACK(GoOpposite))
		.Help("Switch between source and header file");
	menu.AddMenu(AK_SAVEFILE, CtrlImg::save(), THISBACK(DoSaveFile))
		.Help("Save current file");
	if(!designer)
		menu.AddMenu(CanToggleReadOnly(), AK_READONLY, IdeImg::read_only(), THISBACK(ToggleReadOnly))
			.Check(editor.IsReadOnly())
			.Help("Set / clear read-only flag for current file");

	menu.AddMenu(!designer, AK_PRINT, CtrlImg::print(), THISBACK(Print));

//	menu.Add("Export project", THISBACK(ExportProject))
//		.Help("Copy all project files into given directory");

	if(menu.IsMenuBar())
	{
		menu.Separator();
		menu.Add(AK_CLOSETAB, THISBACK(ClearTab))
		    .Help("Close the current file tab");
		menu.Add(AK_CLOSETABS, THISBACK(ClearTabs))
		    .Help("Close all file tabs");
		if(!designer) {
			menu.Add("Bookmarks", THISBACK(FileBookmark))
				.Help("Set one of available bookmarks (1..9, 0) on current file");
			menu.MenuSeparator();
		}
		menu.Add("Show/hide bottom pane", THISBACK(SwapBottom))
			.Check(IsBottomShown())
			.Key(K_ESCAPE)
			.Help("Show / hide bottom pane (with console, calc and browser tabs)");
	}

	menu.Add(AK_PACKAGESFILES, THISBACK(SwapPackagesFiles))
	    .Check(weframe.IsShown());

	menu.MenuSeparator();

	bool split = editorsplit.GetZoom() < 0;
	menu.Add(AK_SPLIT, THISBACK1(KeySplit, false))
	    .Check(split && editorsplit.IsVert());
	menu.Add(AK_VSPLIT, THISBACK1(KeySplit, true))
	    .Check(split && editorsplit.IsHorz());
	menu.Add(split, AK_SWAP, THISBACK(SwapEditors));

	menu.MenuSeparator();
	
	menu.Add(AK_OPENFILEDIR, THISBACK(OpenFileFolder));
	menu.Add("Copy File Path", [=] { WriteClipboardText(GetActiveFilePath()); });
	menu.Sub("Properties", [=](Bar& bar) { FilePropertiesMenu(bar); });
	menu.MenuSeparator();

	menu.Add(AK_STATISTICS, THISBACK(Statistics))
		.Help("Display various statistics");

	menu.Add("Project licences..", THISBACK(Licenses));
	menu.Add("Elapsed times..", THISBACK(Times));

	menu.Add(AK_EXIT, THISBACK(Exit));
}

void Ide::AssistEdit(Bar& menu)
{
	bool b = !editor.IsReadOnly() && !designer;
	menu.Add(b, "Insert", THISBACK(InsertMenu));
	menu.Add(b, "Insert #include", THISBACK(InsertInclude));
}

void Ide::InsertAdvanced(Bar& bar)
{
	bool b = !editor.IsReadOnly();
	AssistEdit(bar);
	bar.Add(b, "Advanced", THISBACK(EditSpecial));
}

void Ide::EditSpecial(Bar& menu)
{
	bool b = !editor.IsReadOnly();
	menu.Add(AK_WORDWRAP, THISBACK(ToggleWordwrap))
	    .Check(wordwrap);
	    
	menu.Separator();
	
	menu.Add(b, AK_SPACESTOTABS, THISBACK(EditMakeTabs))
		.Help("Convert leading blanks on each line to tabs");
	menu.Add(b, AK_TABSTOSPACES, THISBACK(EditMakeSpaces))
		.Help("Convert all tabs to spaces");
	menu.Add(b, AK_LINEENDINGS, THISBACK(EditMakeLineEnds))
		.Help("Remove tabs and spaces at line endings");
	menu.Add(b, AK_TRANSLATESTRING, THISBACK(TranslateString))
		.Help("Mark the current selection as translated string");
	menu.Add(b, AK_SWAPCHARS, THISBACK(SwapChars))
	    .Help("Transpose characters");
	menu.Add(AK_COPYWORD, THISBACK(CopyWord))
	    .Help("Copy the current identifier to the clipboard");
	menu.Add(b, AK_DUPLICATELINE, THISBACK(DuplicateLine))
	    .Help("Duplicate the current line");
	menu.Add(b, AK_FORMATCODE, THISBACK(FormatCode))
	    .Help("Reformat code in editor");
	menu.Add(b && editor.IsSelection(), AK_TOUPPER, THISBACK(TextToUpper))
	    .Help("Convert letters in selection to uppercase");
	menu.Add(b && editor.IsSelection(), AK_TOLOWER, THISBACK(TextToLower))
	    .Help("Convert letters in selection to lowercase");
	menu.Add(b && editor.IsSelection(), AK_TOASCII, THISBACK(TextToAscii))
		.Help("Covert text to 7-bit ASCII removing all accents and special symbols");
	menu.Add(b && editor.IsSelection(), AK_INITCAPS, THISBACK(TextInitCaps))
	    .Help("Capitalize the first character of words in selection");
	menu.Add(b && editor.IsSelection(), AK_SWAPCASE, THISBACK(SwapCase))
	    .Help("Swap the case of letters in selection");
	menu.Add(b && editor.IsSelection(), AK_TOCSTRING, THISBACK(ToCString))
	    .Help("Convert selection to CString");
	menu.Add(b && editor.IsSelection(), AK_TOCOMMENT, THISBACK(ToComment))
		.Help("Comment code");
	menu.Add(b && editor.IsSelection(), AK_COMMENTLINES, THISBACK(CommentLines))
		.Help("Comment code lines");
	menu.Add(b && editor.IsSelection(), AK_UNCOMMENT, THISBACK(UnComment))
		.Help("Uncomment code");
	menu.Add(b, AK_REFORMAT_COMMENT, THISBACK(ReformatComment))
	    .Help("Reformat multiline comment into paragraph");
	menu.Add(b, "Remove debugging logs (DDUMP...)", [=] { RemoveDs(); });
	menu.MenuSeparator();
	menu.Add(AK_COPY_POSITION, [=] { CopyPosition(); });
	menu.Add(AK_GOTO_POSITION, [=] { GotoPosition(); });
}

void Ide::SearchMenu(Bar& menu)
{
	if(!designer) {
		menu.Add(AK_FIND, THISBACK(EditFind))
			.Help("Search for text or text pattern");
		menu.Add(!editor.IsReadOnly(), AK_REPLACE, THISBACK(EditReplace))
			.Help("Search for text or text pattern, with replace option");
//		menu.Add(AK_FINDSEL, THISBACK(EditFindReplacePickText))
//			.Help("Show find / replace dialog & set active text as the 'find' text");

		menu.Add(AK_FINDNEXT, THISBACK(EditFindNext))
			.Help("Find next occurrence");
		menu.Add(AK_FINDPREV, THISBACK(EditFindPrevious))
			.Help("Find previous occurrence");

		menu.MenuSeparator();

		menu.Add(AK_FINDSTRING, THISBACK1(FindString, false))
			.Help("Find any ordinary string constant (\"\" - delimited)");
		menu.Add(AK_FINDSTRINGBACK, THISBACK1(FindString, true))
			.Help("Find any ordinary string constant (\"\" - delimited) backwards");
		menu.MenuSeparator();
	}
	menu.Add(AK_FINDINFILES, THISBACK1(FindInFiles, false))
		.Help("Find text or text pattern in subtree of given path");
	menu.Add(AK_REPLACEINFILES, THISBACK1(FindInFiles, true))
		.Help("Find text or text pattern in subtree of given path, with replace option(s)");
	menu.Add(AK_FINDFILE, THISBACK(FindFileName))
		.Help("Locate file by filename (use *, ? when you're not sure)");
}

void Ide::Edit(Bar& menu)
{
	bool b = !editor.IsReadOnly();
	if(editfile.GetCount() && editashex.Find(editfile) < 0) {
		menu.Add(AK_EDITASHEX, THISBACK(EditAsHex));
		if(!designer)
			menu.MenuSeparator();
	}
	if(designer) {
		if(FileExists(designer->GetFileName())) {
			menu.Add(AK_EDITASTEXT, THISBACK(EditAsText))
			    .Help("Edit as text file (do not use a designer)");
			menu.MenuSeparator();
		}
		if(menu.IsMenuBar())
			designer->EditMenu(menu);
	}
	else {
		bool selection = editor.IsAnySelection();

		if(editor.IsView()) {
			menu.Add(AK_EDITASTEXT, THISBACK(EditAsText))
			    .Help("Edit file");
			menu.MenuSeparator();
		}
		if(GetFileExt(editfile) == ".t") {
			if(editastext.Find(editfile) >= 0)
				menu.Add(AK_DESIGNER, THISBACK(EditUsingDesigner))
				    .Help("Edit converted strings");
			else
				menu.Add(AK_EDITASTEXT, THISBACK(EditAsText))
				    .Help("Edit raw strings");
			menu.MenuSeparator();
		}
		else
		if(editastext.Find(editfile) >= 0/* && IsDesignerFile(editfile)*/) {
			menu.Add(AK_DESIGNER, THISBACK(EditUsingDesigner))
			    .Help(editor.GetLength() > 256*1024*1024 ? "View file" : "Edit using the designer (not as text)");
			menu.MenuSeparator();
		}
		menu.Add(b, "Undo", CtrlImg::undo(), callback(&editor, &LineEdit::Undo))
			.Key(K_CTRL_Z)
			.Enable(editor.IsUndo())
			.Help("Undo changes to text");
		menu.Add(b, "Redo", CtrlImg::redo(), callback(&editor, &LineEdit::Redo))
			.Key(K_SHIFT|K_CTRL_Z)
			.Enable(editor.IsRedo())
			.Help("Redo undone changes");

		menu.Separator();

		menu.Add(b, "Cut", CtrlImg::cut(), callback(&editor, &LineEdit::Cut))
			.Key(K_CTRL_X)
			.Enable(selection)
			.Help("Cut selection and place it on the system clipboard");
		menu.Add("Copy", CtrlImg::copy(), callback(&editor, &LineEdit::Copy))
			.Key(K_CTRL_C)
			.Enable(selection)
			.Help("Copy current selection on the system clipboard");
		menu.Add(b, "Paste", CtrlImg::paste(), THISBACK(EditPaste))
			.Key(K_CTRL_V)
			.Help("Insert text from clipboard at cursor location");

		menu.Separator();
		
		menu.Add("Select all", CtrlImg::select_all(), callback(&editor, &LineEdit::SelectAll))
			.Key(K_CTRL_A);
	}

	menu.MenuSeparator();
	
	menu.Add("Find and Replace", THISBACK(SearchMenu));

	if(!designer && menu.IsMenuBar())
		InsertAdvanced(menu);
}

bool Ide::HasMacros()
{
	const Array<IdeMacro>& mlist = UscMacros();
	if(!mlist.IsEmpty())
		for(int i = 0; i < mlist.GetCount(); i++) {
			const IdeMacro& m = mlist[i];
			if(!IsNull(m.menu))
				return true;
		}
	return false;
}

void Ide::MacroMenu(Bar& menu)
{
	const Array<IdeMacro>& mlist = UscMacros();
	if(!mlist.IsEmpty() && menu.IsMenuBar()) {
		VectorMap< String, Vector<int> > submenu_map;
		for(int i = 0; i < mlist.GetCount(); i++) {
			const IdeMacro& m = mlist[i];
			if(!IsNull(m.menu)) {
				if(IsNull(m.submenu))
					submenu_map.GetAdd(Null).Add(i);
				else
					submenu_map.GetAdd(m.menu).Add(i);
			}
		}
		if(!submenu_map.IsEmpty()) {
			Vector<int> order = GetSortOrder(submenu_map.GetKeys());
			for(int o = 0; o < order.GetCount(); o++) {
				String m = submenu_map.GetKey(order[o]);
				Vector<int>& mx = submenu_map[order[o]];
				ValueArray va;
				for(int i = 0; i < mx.GetCount(); i++)
					va.Add(mx[i]);
				if(!IsNull(m))
					menu.Add(m, THISBACK1(EditMacroMenu, va));
				else
					EditMacroMenu(menu, va);
			}
		}
	}
}

void Ide::EditMacroMenu(Bar& menu, ValueArray mx)
{
	const Array<IdeMacro>& mlist = UscMacros();
	Vector<String> names;
	Vector<int> index;
	names.Reserve(mx.GetCount());
	for(int i = 0; i < mx.GetCount(); i++) {
		int ii = mx[i];
		if(ii >= 0 && ii < mlist.GetCount()) {
			const IdeMacro& m = mlist[ii];
			names.Add(Nvl(m.submenu, m.menu));
			index.Add(ii);
		}
	}
	IndexSort(names, index);
	for(int i = 0; i < index.GetCount(); i++)
		menu.Add(names[i], THISBACK1(EditMacro, index[i]))
			.Key(mlist[index[i]].hotkey);
}

void Ide::EditMacro(int i)
{
	const Array<IdeMacro>& mlist = UscMacros();
	if(i >= 0 && i < mlist.GetCount()) {
		const IdeMacro& m = mlist[i];
		try {
			Vector<EscValue> arg;
			EscValue api = macro_api, code = m.code;
			::Execute(UscGlobal(), &api, code, arg, 1000000);
		}
		catch(Exc e) {
			PutConsole(e);
		}
	}
}

void Ide::Setup(Bar& menu)
{
	menu.Add("Be verbose", THISBACK(ToggleVerboseBuild))
		.Check(console.verbosebuild)
		.Help("Log detailed description of build and debug");
	menu.MenuSeparator();
	menu.Add("Settings..", IdeImg::Settings(), THISBACK(SetupFormat))
		.Help("Fonts, tabs, indentation, status bar");
	menu.Add("Abbreviations..", THISBACK(Abbreviations))
		.Help("Edit abbreviation keywords and code");
	menu.Add("Keyboard shortcuts..", THISBACK(DoEditKeys))
		.Help("Edit key bindings");
	menu.Add("Macro Manager..", THISBACK(DoMacroManager))
		.Help("Manage macros collection..");
	menu.Add("Build methods..", THISBACK(SetupBuildMethods))
	    .Help("Setup build methods");
#ifdef PLATFORM_WIN32
	menu.Add("Automatic build methods setup..", callback(InstantSetup))
	    .Help("Setups/fixes build methods and basic assemblies..");
#endif
	menu.MenuSeparator();
	menu.Add(HasGit(), "UppHub..", IdeImg::UppHub(), [] { UppHub(); });
	menu.Add("Checkout and setup U++ SVN trunk sources..", [=] {
		if(SetupSVNTrunk()) {
			IdeAgain = true;
			Break();
		}
	});

#ifndef PLATFORM_COCOA
	const Workspace& wspc = IdeWorkspace();
	if(wspc[0] == "ide")
		for(int i = 0; i < wspc.GetCount(); i++)
			if(wspc[i] == "ide/Core")
				menu.Add("Upgrade TheIDE..", [=] { UpgradeTheIDE(); });
#ifdef PLATFORM_POSIX
	menu.Add("Install theide.desktop", [=] { InstallDesktop(); });
#endif
#endif

	if(menu.IsMenuBar())
		SetupMobilePlatforms(menu);
}

void Ide::SetupMobilePlatforms(Bar& menu)
{
	AndroidSDK androidSDK(GetAndroidSdkPath());
	
	if(androidSDK.Validate()) {
		menu.Separator();
		menu.Add("Android", THISBACK1(SetupAndroidMobilePlatform, androidSDK));
	}
	
}

void Ide::SetupAndroidMobilePlatform(Bar& menu, const AndroidSDK& androidSDK)
{
	menu.Add("SDK Manager", THISBACK1(LaunchAndroidSDKManager, androidSDK));
	menu.Add("AVD Manager", THISBACK1(LaunchAndroidAVDManager, androidSDK));
	menu.Add("Device monitor", THISBACK1(LauchAndroidDeviceMonitor, androidSDK));
}

void Ide::ProjectRepo(Bar& menu)
{
	Vector<String> w = RepoDirs(true);
	for(int i = 0; i < w.GetCount(); i++)
		menu.Add("Synchronize " + w[i], IdeImg::svn_dir(), THISBACK1(SyncRepoDir, w[i]));
	menu.Add("Synchronize everything..", IdeImg::svn(), THISBACK(SyncRepo));
}

void Ide::Project(Bar& menu)
{
	if(menu.IsToolBar() && !debugger && !IsEditorMode())
	{
		mainconfiglist.Enable(idestate == EDITING);
		buildmode.Enable(idestate == EDITING);
		menu.Add(mainconfiglist, HorzLayoutZoom(180));
		menu.Gap(4);
		menu.Add(buildmode, HorzLayoutZoom(180));
		menu.Separator();
	}
	if(!IsEditorMode()) {
		WorkspaceWork::PackageMenu(menu);
		menu.MenuSeparator();
		menu.Add(AK_ORGANIZER, IdeImg::package_organizer(), THISBACK(EditWorkspace))
			.Help("Package dependencies, compiler & linker options, output path override");
		menu.Add(AK_CUSTOM, THISBACK(CustomSteps))
			.Help("Building intermediate files using custom commands / applications");
		if(menu.IsMenuBar())
			menu.Add(AK_MAINCONFIG, IdeImg::main_package(), THISBACK(MainConfig))
				.Help("Configuring compiler, operating system, output application parameters, custom flags");
		menu.Separator();
		menu.Add(AK_SYNCT, IdeImg::Language(), THISBACK1(SyncT, 0))
		    .Help("Synchronize all language translation files of current workspace");
		menu.AddMenu(AK_TRIMPORT, IdeImg::Language(), THISBACK1(SyncT, 1))
		    .Help("Import runtime translation file");
		menu.AddMenu(AK_TREXPORT, IdeImg::Language(), THISBACK1(SyncT, 2))
		    .Help("Export runtime translation file");
		if(OldLang())
			menu.Add("Convert s_ -> t_", THISBACK(ConvertST));
	}
	menu.MenuSeparator();
	FilePropertiesMenu0(menu);
	if(!IsEditorMode()) {
		menu.MenuSeparator();
		if(repo_dirs) {
			String pp = GetActivePackagePath();
			menu.AddMenu(FileExists(pp) && editfile_repo,
			             (editfile_repo == SVN_DIR ? "Show svn history of " : "Show git history of ") + GetFileName(pp),
			             IdeImg::SvnDiff(), [=] {
				if(FileExists(pp))
					RunRepoDiff(pp);
			});
			if(menu.IsMenuBar())
				menu.Add("Repo", THISBACK(ProjectRepo));
			else
				menu.Add("Synchronize all repositories..", IdeImg::svn(), THISBACK(SyncRepo));
		}
	}
}

void Ide::FilePropertiesMenu0(Bar& menu)
{
	menu.Add(IsActiveFile(), AK_FILEPROPERTIES, THISBACK(FileProperties))
		.Help("File properties stored in package");
}

void Ide::FilePropertiesMenu(Bar& menu)
{
	FilePropertiesMenu0(menu);
	menu.Add(IsActiveFile() && !designer, AK_SAVEENCODING, THISBACK(ChangeCharset))
	    .Help("Convert actual file to different encoding");
	bool candiff = IsActiveFile() && !editfile_isfolder && !designer;
	String path;
	int i = filelist.GetCursor() + 1;
	if(i >= 0 && i < fileindex.GetCount() && fileindex[i] < actual.file.GetCount())
		path = SourcePath(actualpackage, actual.file[fileindex[i]]);
	menu.Sub(candiff, "Compare with", [=](Bar& bar) {
		bar.AddMenu(candiff, AK_DIFF, IdeImg::Diff(), THISBACK(Diff))
		    .Help("Show differences between the current and selected file");
		bar.AddMenu(candiff && FileExists(GetTargetLogPath()),
		            AK_DIFFLOG, IdeImg::DiffLog(), [=] { DiffWith(GetTargetLogPath()); })
		    .Help("Show differences between the current file and the log");
		if(FileExists(path))
			bar.AddMenu(candiff && FileExists(path), path,
			            IdeImg::DiffNext(), [=] { DiffWith(path); })
			    .Help("Show differences between the current and the next file");
		for(String p : difflru)
			if(p != path)
				bar.AddMenu(candiff && FileExists(p), p,
				            IdeImg::DiffNext(), [=] { DiffWith(p); })
				    .Help("Show differences between the current and that file");
	});
	if(editfile_repo) {
		String txt = String("Show ") + (editfile_repo == SVN_DIR ? "svn" : "git") + " history of ";
		menu.AddMenu(candiff, AK_SVNDIFF, IdeImg::SvnDiff(), [=] {
			if(!IsNull(editfile))
				RunRepoDiff(editfile);
		}).Text(txt + "file..");
		if(editfile.GetCount()) {
			String mine;
			String theirs;
			String original;
			Vector<String> r;
			Vector<int> rn;
			if(editfile_repo == SVN_DIR) {
				for(FindFile ff(editfile + ".*"); ff; ff.Next()) {
					if(ff.IsFile()) {
						String p = ff.GetPath();
						if(p.Find(".merge-left.r") >= 0)
							original = p;
						if(p.Find(".merge-right.r") >= 0)
							theirs = p;
						if(p.Find(".working") >= 0 || p.Find(".mine") >= 0)
							mine = p;
						try {
							CParser q(GetFileExt(~p));
							q.PassChar('.');
							q.PassChar('r');
							int n = q.ReadInt();
							if(q.IsEof()) {
								r.Add(p);
								rn.Add(n);
							}
						}
						catch(CParser::Error) {}
					}
				}
				if(IsNull(original) && IsNull(theirs) && r.GetCount() == 2) {
					original = r[0];
					theirs = r[1];
					if(rn[1] > rn[0])
						Swap(original, theirs);
				}
			}
			else {
				bool a = false, b = false, c = false;
				int n = min(editor.GetLineCount(), 10000);
				for(int i = 0; i < n; i++) { // check that we are in git conflict
					const String& s = editor.GetUtf8Line(i);
					int ch = *s;
					a = a || ch == '<' && s.StartsWith("<<<<<<<");
					b = b || ch == '=' && s.StartsWith("=======");
					c = c || ch == '>' && s.StartsWith(">>>>>>>");
				}
				if(a && b && c) {
					original = "1";
					mine = "2";
					theirs = "3";
				}
			}
				
			if(mine.GetCount() || theirs.GetCount() || original.GetCount()) {
				menu.Sub("SVN Conflict", [=] (Bar& bar) {
					if(mine.GetCount() && theirs.GetCount())
						bar.Add("Compare mine <-> theirs", [=] { DiffFiles("mine", mine, "theirs", theirs); });
					if(mine.GetCount() && original.GetCount())
						bar.Add("Compare mine <-> original", [=] { DiffFiles("mine", mine, "original", original); });
					if(theirs.GetCount() && original.GetCount())
						bar.Add("Compare theirs <-> original", [=] { DiffFiles("theirs", theirs, "original", original); });
					if(mine.GetCount())
						bar.Add("Compare current <-> mine", [=] { DiffFiles("current", editfile, "mine", mine); });
					if(theirs.GetCount())
						bar.Add("Compare current <-> theirs", [=] { DiffFiles("current", editfile, "theirs", theirs); });
					if(original.GetCount())
						bar.Add("Compare current <-> original", [=] { DiffFiles("current", editfile, "original", original); });
					bar.Separator();
					bar.Add("Use mine", [=] {
						if(PromptYesNo("Do you want to overwrite current with [* mine]?")) {
							SaveFile();
							Upp::SaveFile(editfile, LoadConflictFile(mine));
						}
					});
					bar.Add("Use theirs", [=] {
						if(PromptYesNo("Do you want to overwrite current with [* theirs]?")) {
							SaveFile();
							Upp::SaveFile(editfile, LoadConflictFile(theirs));
						}
					});
				});
			}
		}
		if(editfile.GetCount() && editfile_repo == GIT_DIR) {
		}
	}
}

void Ide::BuildFileMenu(Bar& menu)
{
	bool b = idestate == EDITING && !IdeIsDebugLock();
	menu.Add(b, "Compile " + GetFileName(editfile), IdeImg::Source(), THISBACK(FileCompile))
		.Key(AK_COMPILEFILE)
		.Help("Compile current file");
	menu.Add(b, "Preprocess " + GetFileName(editfile), IdeImg::Header(), THISBACK1(Preprocess, false))
		.Key(AK_PREPROCESSFILE)
		.Help("Preprocess current file into temporary file & open in editor");
	if(findarg(current_builder, "GCC", "CLANG") >= 0)
		menu.Add(b, "Show assembler code for " + GetFileName(editfile), THISBACK1(Preprocess, true))
			.Key(AK_ASSEMBLERCODE)
			.Help("Compile the file into assembler code");
	if(console.verbosebuild)
		menu.Add(b, "Internal Preprocess " + GetFileName(editfile), IdeImg::HeaderInternal(), THISBACK(PreprocessInternal));
}

void Ide::BuildPackageMenu(Bar& menu)
{
	int pi = GetPackageIndex();
	bool b = !IdeIsDebugLock() && idestate == EDITING && pi >= 0 && pi < IdeWorkspace().GetCount();
	String name;
	if(b)
		name = '\'' + IdeWorkspace()[pi] + '\'';
	menu.Add(b, "Build package " + name, THISBACK(PackageBuild))
		.Help("Build current package");
	menu.Add(b, "Clean package " + name, THISBACK(PackageClean))
		.Help("Remove all intermediate files of the current package");
	menu.MenuSeparator();
}

void Ide::BuildMenu(Bar& menu)
{
	bool b = !IdeIsDebugLock();
	menu.Add(AK_OUTPUTMODE, THISBACK(SetupOutputMode))
	    .Help("Setup how to build the target");
	if(idestate == BUILDING)
		menu.Add(b, "Stop build", IdeImg::build_stop(), THISBACK(StopBuild))
			.Key(AK_BUILD)
			.Help("Stop building");
	else
		menu.Add(b, "Build", IdeImg::build_make(), THISBACK(DoBuild))
			.Key(AK_BUILD)
			.Help("Perform minimal application rebuild");
	b = b && idestate == EDITING;
	menu.Add(b, AK_CLEAN, THISBACK(Clean))
		.Help("Remove all intermediate files");
//	menu.Add("Reset BLITZ", [=] { ResetBlitz(); })
//	    .Help("All files will be considered for BLITZ, regardless of time");
	menu.Add(b, AK_REBUILDALL, IdeImg::build_rebuild_all(), THISBACK(RebuildAll))
		.Help("Remove all intermediate files & build");
	menu.Add(b, AK_CLEANUPPOUT, THISBACK(CleanUppOut))
		.Help("Remove all files and subdirectories in the output & intermediate directory (see Base setup)");

//	menu.MenuSeparator();

//	menu.Add(b, AK_CREATEMAKEFILE, THISBACK(CreateMakefile))
//		.Help("Create makefile enabling IDE-independent project building");

	menu.MenuSeparator();


	if(menu.IsMenuBar() && !menu.IsScanKeys())
		BuildPackageMenu(menu);

	BuildFileMenu(menu);
	
	menu.MenuSeparator();

	menu.Add("Stop on errors", THISBACK(ToggleStopOnErrors))
		.Check(stoponerrors)
		.Help("Stop build after package when the package has errors");

	menu.MenuSeparator();

	bool ff = BottomIsFindInFiles();
	String hh = ff ? "position" : "error line";
	bool ffb = ff ? FFound().GetCount() : error.GetCount();
	menu.Add(ffb, AK_FINDNEXTERROR, THISBACK(FindNextError))
		.Help("Find next " + hh + "according to console pane");
	menu.Add(ffb, AK_FINDPREVERROR, THISBACK(FindPrevError))
		.Help("Find previous " + hh + "according to console pane");
	menu.MenuSeparator();
	menu.Add(!IsNull(target), AK_OPENOUTDIR, [=] { ShellOpenFolder(GetFileFolder(target)); });
	menu.Add(!IsNull(target), AK_COPYOUTDIR, [=] { WriteClipboardText(GetFileFolder(target)); });
	menu.Add(!IsNull(target), AK_COPYTARGET, [=] { WriteClipboardText(target); });
	menu.Add(!IsNull(target), AK_OUTDIRTERMINAL, [=] { LaunchTerminal(GetFileFolder(target)); });
}

void Ide::DebugMenu(Bar& menu)
{
	bool b = idestate == EDITING && !IdeIsDebugLock();
	if(debugger) {
		debugger->DebugBar(menu);
		menu.MenuSeparator();
	}
	else {
		if(console.IsRunning())
			menu.Add("Stop!", THISBACK(StopDebug))
			    .Help("Stop controlled process");
		if(menu.IsMenuBar())
			menu.Add(AK_RUNOPTIONS, THISBACK(RunArgs))
				.Help("Current directory, command line, stdout redirection");
		menu.Add(b, AK_EXECUTE, IdeImg::execute(), THISBACK(BuildAndExecute))
			.Help("Build and execute the application");
		menu.Add(b, AK_DEBUG, IdeImg::debug_run(), THISBACK1(BuildAndDebug, false))
			.Help("Build application & run debugger");
		if(menu.IsMenuBar()) {
			menu.Add(b, AK_DEBUGTO, THISBACK1(BuildAndDebug, true))
				.Help("Build application & run to cursor in debugger");
			menu.Add(b, AK_DEBUGEXT, THISBACK(BuildAndExtDebug))
				.Help("Build application & run external debugger (see Base setup, default \"msdev.exe\")");
			menu.Add(b, AK_DEBUGFILEEXT, THISBACK(BuildAndExtDebugFile))
				.Help("Build application & run external debugger, trying to start with current file");
		#ifdef PLATFORM_POSIX
			if(IsValgrind())
				menu.Add(b, AK_VALGRIND, THISBACK(Valgrind))
					.Help("Build application & run in valgring");
		#endif

			menu.Separator();
		}
	}
	if(menu.IsMenuBar()) {
		menu.Add(!editfile.IsEmpty() /*&& !debuglock*/, AK_BREAKPOINT, THISBACK(DebugToggleBreak))
			.Help("Set / clear breakpoint on current line");
		menu.Add(!editfile.IsEmpty(), AK_CONDBREAKPOINT, THISBACK(ConditionalBreak))
			.Help("Edit conditional breakpoint");
		menu.Add(!editfile.IsEmpty() /*&& !debuglock*/, AK_CLEARBREAKPOINTS, THISBACK(DebugClearBreakpoints))
			.Help("Clear all breakpoints");
		menu.Separator();
		
		String targetLogPath = GetTargetLogPath();
		menu.Add(target.GetCount() && FileExists(targetLogPath), AK_OPENLOG, THISBACK1(OpenLog, targetLogPath));
	}
}

void Ide::BrowseMenu(Bar& menu)
{
	if(!IsEditorMode()) {
		if(menu.IsMenuBar()) {
			menu.AddMenu(AK_NAVIGATOR, IdeImg::Navigator(), THISBACK(ToggleNavigator))
				.Check(editor.IsNavigator())
				.Enable(!designer);
			menu.Add(AK_GOTO, THISBACK(SearchCode))
				.Enable(!designer)
				.Help("Go to given line");
			menu.Add(AK_GOTOGLOBAL, THISBACK(NavigatorDlg));
			menu.Add(!designer, AK_JUMPS, THISBACK(ContextGoto));
			menu.Add(!designer, AK_SWAPS, THISBACK(SwapS));
			menu.Add(!designer, AK_ASSIST, callback(&editor, &AssistEditor::Assist));
			menu.Add(!designer, AK_DCOPY, callback(&editor, &AssistEditor::DCopy));
			menu.Add(!designer, AK_VIRTUALS, callback(&editor, &AssistEditor::Virtuals));
			menu.Add(!designer, AK_THISBACKS, callback(&editor, &AssistEditor::Thisbacks));
			menu.Add(!designer, AK_COMPLETE, callback(&editor, &AssistEditor::Complete));
			menu.Add(!designer, AK_ABBR, callback(&editor, &AssistEditor::Abbr));
			menu.Add(!designer, AK_GO_TO_LINE, THISBACK(GoToLine));
			AssistEdit(menu);
			menu.MenuSeparator();
		}
		
		menu.Add("Go back", IdeImg::AssistGoBack(), THISBACK1(History, -1))
			.Key(K_ALT_LEFT)
			.Enable(GetHistory(-1) >= 0);
		menu.Add("Go forward", IdeImg::AssistGoForward(), THISBACK1(History, 1))
			.Key(K_ALT_RIGHT)
			.Enable(GetHistory(1) >= 0);
		
		if(menu.IsMenuBar()) {
			menu.MenuSeparator();
			menu.Add("Check source files for changes", THISBACK(CheckCodeBase));
			menu.Add("Rescan all source files", THISBACK(RescanCode));
			if(!auto_rescan)
				menu.Add(AK_RESCANCURRENTFILE, THISBACK(EditFileAssistSync));
			menu.MenuSeparator();
		}
	}
	else {
		menu.Add(!designer, AK_GO_TO_LINE, THISBACK(GoToLine));
		menu.MenuSeparator();
	}
	
	if(menu.IsMenuBar()) {
		menu.AddMenu(AK_CALC, IdeImg::calc(), THISBACK1(ToggleBottom, BCALC))
	     .Check(IsBottomShown() && btabs.GetCursor() == BCALC);
		menu.AddMenu(AK_QTF, IdeCommonImg::Qtf(), THISBACK(Qtf));
		menu.AddMenu(!designer, AK_XML, IdeCommonImg::xml(), THISBACK(Xml));
		menu.AddMenu(!designer, AK_JSON, IdeCommonImg::json(), THISBACK(Json));
		menu.AddMenu(!designer, AK_ASERRORS, IdeImg::errors(), THISBACK(AsErrors));
		menu.AddMenu(AK_DIRDIFF, DiffImg::DirDiff(), THISBACK(DoDirDiff));
		menu.AddMenu(AK_PATCH, DiffImg::PatchDiff(), THISBACK(DoPatchDiff));
	}
}

void Ide::HelpMenu(Bar& menu)
{
	if(!IsEditorMode()) {
		menu.Add(AK_BROWSETOPICS, IdeImg::help(), THISBACK(ShowTopics));
		menu.Add(editor.GetWord().GetCount(), AK_SEARCHTOPICS, THISBACK(SearchTopics));
	}
	menu.Add(AK_BROWSETOPICS_WIN, IdeImg::help_win(), THISBACK(ShowTopicsWin));
	menu.MenuSeparator();
	menu.AddMenu("Get help / report bugs..", IdeImg::Go_forward(), callback1(LaunchWebBrowser, "http://www.ultimatepp.org/forums"));
	menu.AddMenu("Online documentation..", IdeImg::Go_forward(), callback1(LaunchWebBrowser, "http://www.ultimatepp.org/www$uppweb$documentation$en-us.html"));
	menu.AddMenu("Common information..", IdeImg::Go_forward(), callback1(LaunchWebBrowser, "http://www.ultimatepp.org/www$uppweb$community$en-us.html"));
	menu.Separator();
	OnlineSearchMenu(menu);
	if(menu.IsMenuBar()) {
		menu.Separator();
		menu.Add(FileExists(GetIdeLogPath()), "View application log file", THISBACK(ViewIdeLogFile));
		menu.Separator();
		menu.Add("About..", IdeImg::info(), THISBACK(About));
	}
}

void Ide::MainMenu(Bar& menu)
{
	menu.Add("File", THISBACK(File))
		.Help("Package & file functions, exports, bookmarks");
	menu.Add("Edit", THISBACK(Edit))
		.Help("Clipboard, find & replace, spaces / tabs conversion, scope highlighting");
	if(HasMacros())
		menu.Add("Macro", THISBACK(MacroMenu))
			.Help("Editor & IDE macros");
	menu.Add("Project", THISBACK(Project))
		.Help("Package organizer, custom steps, configuration manager");
	if(!IsEditorMode()) {
		menu.Add("Build", THISBACK(BuildMenu))
			.Help("Building & debugging, minor build options, showing errors");
		menu.Add("Debug", THISBACK(DebugMenu))
			.Help("Debugger commands (currently supports gdb-connection only)");
	}
	menu.Add("Assist", THISBACK(BrowseMenu))
		.Help("Informations, code browsing and assistance");
	menu.Add("Setup", THISBACK(Setup))
		.Help("Paths, editor settings, connection to remote host");
	menu.Add("Help", THISBACK(HelpMenu))
		.Help("Help, credits and license");
}

void Ide::MainTool(Bar& bar)
{
	if(!IsEditorMode()) {
		BrowseMenu(bar);
		bar.Separator();
	}
	bar.Add("Edit as text", IdeImg::EditText(), THISBACK(EditAsText))
	   .Check(!designer)
	   .Enable(!editfile_isfolder)
	   .Key(!designer ? 0 : K_CTRL_T);
	bool b = designer && !designer.Is<FileHexView>();
	bool d = IsDesignerFile(editfile);
	bar.Add("Edit using designer", IdeImg::EditDesigner(), THISBACK(EditUsingDesigner))
	   .Check(b || editfile_isfolder)
	   .Enable(d || editfile_isfolder)
	   .Key(b || !d ? 0 : K_CTRL_T);
	bar.Add("View as hex", IdeImg::EditHex(), THISBACK(EditAsHex))
	   .Check(designer.Is<FileHexView>())
	   .Enable(!editfile_isfolder)
	   .Key(K_CTRL_B);
	if(!designer)
		bar.Separator();
	Edit(bar);
	if(debugger) {
		DebugMenu(bar);
		bar.Separator();
	}
	Project(bar);
	if(!IsEditorMode()) {
		BuildMenu(bar);
		if(!debugger) {
			bar.Separator();
			DebugMenu(bar);
		}
		bar.Separator();
	}
	HelpMenu(bar);
#ifdef PLATFORM_COCOA
	bar.GapRight();
	bar.Add(display, HorzLayoutZoom(170));
#endif
}

void Ide::ConsoleMenu(Bar& menu)
{
	bool selection = console.IsAnySelection();
	menu.Add("Copy", CtrlImg::copy(), THISBACK(ConsoleCopy))
		.Key(K_CTRL_C)
		.Enable(selection)
		.Help("Copy selection on system clipboard");
	menu.Add("Paste", CtrlImg::paste(), THISBACK(ConsolePaste))
		.Key(K_CTRL_V)
		.Help("Append selection to system console");
	menu.Separator();
	menu.Add(AK_FIND, [=] {
		console.FindReplace(false, true, false);
	});
	menu.Separator();
	menu.Add("Clear", THISBACK(ConsoleClear))
		.Help("Empty system console");
}

void Ide::SetBar()
{
	SetMenuBar();
	SetToolBar();
}

void Ide::SetMenuBar()
{
#ifdef PLATFORM_COCOA
	SetMainMenu(THISBACK(MainMenu));
	menubar.Hide();
#else
	menubar.Set(THISBACK(MainMenu));
#endif
}

void Ide::SetToolBar()
{
	toolbar.Set(THISBACK(MainTool));
}
