#include "ide.h"

#ifndef bmYEAR
#include <build_info.h>
#endif

bool SetupSVNTrunk()
{
	WithSetupSVNLayout<TopWindow> dlg;
	CtrlLayoutOKCancel(dlg, "Checkout U++ trunk");
	SelectDirButton dir_browse("Run in folder");
	dir_browse.Attach(dlg.dir);
#ifdef PLATFORM_WIN32
	dlg.dir <<= GetExeDirFile("upp.src");
#else
	dlg.dir <<= GetHomeDirFile("upp.src");
#endif
	static MapConvert revcv;
	ONCELOCK {
#if 0 // this requires Core/SSL
		for(int pass = 0; pass < 2; pass++) {
			HttpRequest r(pass ? "https://www.ultimatepp.org/www$uppweb$RoadmapHistorical$en-us.html"
			                   : "https://www.ultimatepp.org/www$uppweb$Roadmap$en-us.html");
			r.GlobalTimeout(10000);
			StringStream ss(r.Execute());
			while(!ss.IsEof()) {
				String l = ss.GetLine();
				if(l.Find("(rev") >= 0) {
					String r;
					int v = 0;
					for(int c : l)
						if(c == '<')
							v++;
						else
						if(c == '>')
							v--;
						else
						if(v == 0)
							r.Cat(c);
					int q = r.FindAfter("(rev");
					if(q >= 0) {
						int rev = atoi(~r + q);
						revcv.Add(rev, r);
					}
				}
			}
		}
#else
	#ifdef bmSVN_REVISION
		revcv.Add(atoi(bmSVN_REVISION), "2020.1 (rev " bmSVN_REVISION ") (May 2020)");
	#endif
		revcv.Add(13664, "2019.2 (rev 13664) (Oct 2019)");
		revcv.Add(13068, "2019.1 (rev 13068) (Apr 2019)");
		revcv.Add(11873, "2018.1 (rev 11873) (Mar 2018)");
		revcv.Add(11873, "2018.1 (rev 11873) (Mar 2018)");
		revcv.Add(11540, "2017.2 (rev 11540) (Dec 2017)");
		revcv.Add(10804, "2017.1 (rev 10804) (Jan 2017)");
		revcv.Add(9251, "2015.2 (rev 9251) (Dec 2015)");
		revcv.Add(8227, "2015.1 (rev 8227) (Mar 2015)");
#endif
	}
	dlg.revision.SetConvert(revcv);
	dlg.revision.DropWidthZ(200);
	for(int i = 0; i < revcv.GetCount(); i++)
		dlg.revision.AddList(revcv.GetKey(i));
	dlg.revision.NullText("HEAD");
	dlg.revision << [&] {
		String r = ~~dlg.revision;
		dlg.postfix <<= r.GetCount() ? '.' + r : String();
	};

	UrepoConsole console;
	String dir, tempdir;
	for(;;) {
		if(dlg.Run() != IDOK)
			return false;
		console.Clear();
		dir = ~dlg.dir;
		tempdir = AppendFileName(GetFileFolder(dir), "temp");
		if(DirectoryExists(tempdir))
			tempdir = AppendFileName(GetFileFolder(dir), AsString(Uuid::Create()));
		String revision = IsNull(dlg.revision) ? String() : "@" + ~~dlg.revision;
		bool direxists = DirectoryExists(dir);
		bool fileexists = FileExists(dir);
		if((direxists || fileexists) && !PromptYesNo("Target path already exists, overwrite?"))
			continue;
		int exitcode = 0;
#ifdef _DEBUG
		for(auto nest : { "uppsrc", "examples" })
#else
		for(auto nest : { "uppsrc", "reference", "examples", "tutorial", "bazaar" })
#endif
		{
			static String svn = "svn checkout svn://www.ultimatepp.org/upp/trunk";
			if(dlg.all) {
				exitcode = console.System(svn + revision + ' ' + tempdir);
				break;
			}
			exitcode = console.System(svn + '/' + nest + revision + ' ' + tempdir + '/' + nest);
			if(exitcode)
				break;
		}
		if(exitcode == 0)
			break;
		console.Perform();
		DeleteFolderDeep(tempdir);
	}

	FileDelete(dir);
	DeleteFolderDeep(dir);
	FileMove(tempdir, dir);

#ifdef PLATFORM_WIN32
	String out = GetExeDirFile("out");
	String ass = GetExeFolder();
	String myapps = GetExeDirFile("MyApps");
#else
	String out = GetHomeDirFile("out");
	String ass = GetConfigFolder();
	String myapps = GetHomeDirFile("MyApps");
#endif
	RealizeDirectory(out);
	
	String uppsrc = AppendFileName(dir, "uppsrc");

	auto MakeAssembly = [&](String b, String name = Null) {
		name = Nvl(name, GetFileTitle(b));
		String a = ass + '/' + ~~dlg.prefix + name + ~~dlg.postfix + ".var";
		if(name != "uppsrc")
			b << ';' << uppsrc;
		console.Log("Creating assembly " + a);
		SaveFile(a,
			"UPP = " + AsCString(b) + ";\r\n"
			"OUTPUT = " + AsCString(out) + ";\r\n"
		);
	};

	for(FindFile ff(dir + "/*"); ff; ff.Next())
		if(ff.IsFolder())
			MakeAssembly(ff.GetPath());
	
	MakeAssembly(myapps);
	uppsrc = AppendFileName(dir, "bazaar") + ';' + uppsrc;
	MakeAssembly(myapps, "MyApps-bazaar");

	console.Perform();
	return true;
}
