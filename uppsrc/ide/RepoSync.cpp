#include "ide.h"

String GetGitUrl(const String& repo_dir)
{
	Vector<String> ln = Split(GitCmd(repo_dir, "config --get remote.origin.url"), CharFilterCrLf);
	return ln.GetCount() ? ln[0] : String();
}


String GetSvnUrl(const String& repo_dir)
{
	Vector<String> ln = Split(RepoSys("svn info --show-item repos-root-url " + repo_dir), CharFilterCrLf);
	return ln.GetCount() ? ln[0] : String();
}

int UrepoConsole::Git(const char *dir, const char *command, bool pwd)
{
	String h = GetCurrentDirectory();
	SetCurrentDirectory(dir);
	list.Add(AttrText(String("cd ") + dir).SetFont(font().Bold().Italic()).Ink(SLtBlue()));
	String cmd = String() << "git " << command;
	if(pwd) {
		String url = GetGitUrl(dir);
		String username, password;
		if(url.StartsWith("https://") && GetCredentials(url, dir, username, password)) {
			String https = "https://";
			cmd << ' ' << https;
			if(username.GetCount())
				cmd << UrlEncode(username) + ":";
			int p = cmd.GetCount();
			cmd << UrlEncode(password);
			HidePassword(p, cmd.GetCount());
			cmd << "@" << url.Mid(https.GetCount());
		}
	}
	int code = CheckSystem(cmd);
	SetCurrentDirectory(h);
	return code;
}

String RepoSync::SvnCmd(UrepoConsole& sys, const char *svncmd, const String& dir)
{
	String cmd;
	cmd << "svn " << svncmd << " --non-interactive ";
	String username, password;
	if(GetCredentials(GetSvnUrl(dir), dir, username, password)) {
		cmd << "--username " << username << " --password ";
		int i0 = cmd.GetCount();
		cmd << password;
		sys.HidePassword(i0, cmd.GetCount());
		cmd << " ";
	}
	return cmd;
}

RepoSync::RepoSync()
{
	CtrlLayoutOKCancel(*this, "Version control repository synchronize");
	list.AddIndex();
	list.AddIndex();
	list.AddColumn("Action");
	list.AddColumn("Path");
	list.AddColumn("Changes");
	list.ColumnWidths("220 500 100");
	list.NoCursor().EvenRowColor();
	list.SetLineCy(max(Draw::GetStdFontCy() + Zy(4), Zy(20)));
	Sizeable().Zoomable();
	BackPaint();
	credentials << [=] {
		Index<String> hint;
		for(const auto& w : work) {
			String path = w.dir;
			String s = decode(w.kind, SVN_DIR, GetSvnUrl(path), GIT_DIR, GetGitUrl(path), Null);
			if(s.GetCount())
				hint.FindAdd(s);
			if(path.GetCount())
				hint.FindAdd(path);
		}
		EditCredentials(hint.PickKeys());
	};
}

int CharFilterSvnMsgRepo(int c)
{
	return c >= 32 && c < 128 && c != '\"' ? c : 0;
}

bool IsConflictFile(String path)
{
	String ext = GetFileExt(path);
	if(*ext == '.') {
		ext = ext.Mid(1);
		if(findarg(ext, "mine", "theirs", "working") >= 0 || *ext == 'r' && IsDigit(ext[1])) {
			for(int i = 0; i < 3; i++) {
				int q = path.ReverseFind('.');
				if(q < 0)
					return false;
				path.Trim(q);
				if(FileExists(path))
					return true;
			}
		}
	}
	return false;
}

bool RepoSync::ListSvn(const String& path)
{
	Vector<String> ln = Split(RepoSys("svn status " + path), CharFilterCrLf);
	bool actions = false;
	for(int pass = 0; pass < 2; pass++)
		for(int i = 0; i < ln.GetCount(); i++) {
			String h = ln[i];
			if(h.GetCount() > 7) {
				String file = GetFullPath(TrimLeft(h.Mid(7)));
				if(IsFullPath(file)) {
					actions = true;
					h.Trim(7);
					bool simple = h.Mid(1, 6) == "      ";
					int action = simple ? String("MC?!~").Find(h[0]) : SVN_IGNORE;
					if(h == "    S  ")
						action = REPLACE;
					String an;
					Color  color;
					if(action == SVN_IGNORE) {
						color = Black;
						if(simple && h[0] == 'A') {
							an = "svn add";
							action = SVN_ACTION;
						}
						else
						if(simple && h[0] == 'D') {
							an = "svn delete";
							action = SVN_ACTION;
						}
						else {
							an = h.Mid(0, 7);
							color = Gray;
						}
					}
					else {
						if(action == ADD && IsConflictFile(file)) {
							action = DELETEC;
							an = "Delete (conflict resolved)";
							color = AdjustIfDark(Black());
						}
						else {
							static const char *as[] = {
								"Modify", "Resolved", "Add", "Remove", "Replace"
							};
							static Color c[] = { LtBlue, Magenta, Green, LtRed, LtMagenta };
							an = as[action];
							color = AdjustIfDark(c[action]);
						}
					}
					if(pass == action < 0 && action != DELETEC) {
						int ii = list.GetCount();
						list.Add(action, file, Null,
						         AttrText(action < 0 ? ln[i] : "  " + file.Mid(path.GetCount() + 1)).Ink(color));
						if(action >= 0) {
							list.SetCtrl(ii, 0, revert.Add().SetLabel(an + (action == ADD ? "\nSkip" : "\nRevert")).NoWantFocus());
							revert.Top() <<= 0;
							Ctrl& b = diff.Add().SetLabel("Changes..").SizePos().NoWantFocus();
							b << [=] { DoDiff(ii); };
							list.SetCtrl(ii, 2, b);
						}
					}
				}
			}
		}
	return actions;
}

String GitCmd(const char *dir, const char *command)
{
	LOG("GitCmd " << dir << ", " << command);
	String h = GetCurrentDirectory();
	SetCurrentDirectory(dir);
	String r = RepoSys(String() << "git " << command);
	SetCurrentDirectory(h);
	return r;
}

bool RepoSync::ListGit(const String& path)
{
	Vector<String> ln = Split(GitCmd(path, "status --porcelain ."), CharFilterCrLf);
	bool actions = false;
	for(int i = 0; i < ln.GetCount(); i++) {
		String h = ln[i];
		if(h.GetCount() > 3) {
			if(!h.TrimEnd("/"))
				h.TrimEnd("\\");
			String file = AppendFileName(path, h.Mid(3));
			actions = true;
			int action = String("M.?DR").Find(h[1]);
			if(action < 0 || h[0] != '?' && h[0] != ' ')
				action = SVN_IGNORE;
			String an;
			Color  color;
			if(action == SVN_IGNORE) {
				an = h;
				color = Gray;
			}
			else {
				static const char *as[] = {
					"Modify", "Resolved", "Add", "Remove", "Rename"
				};
				static Color c[] = { LtBlue, Magenta, Green, LtRed, LtMagenta };
				an = as[action];
				color = AdjustIfDark(c[action]);
			}
			int ii = list.GetCount();
			list.Add(action, file, Null, AttrText(action < 0 ? h : file).Ink(color));
			if(action >= 0) {
				list.SetCtrl(ii, 0, revert.Add().SetLabel(an + (action == ADD ? "\nSkip" : "\nRevert")).NoWantFocus());
				revert.Top() <<= 0;
				Ctrl& b = diff.Add().SetLabel("Changes..").SizePos().NoWantFocus();
				b <<= THISBACK1(DoDiff, ii);
				list.SetCtrl(ii, 2, b);
			}
		}
	}
	return actions;
}

void RepoSync::SyncCommits()
{
	bool commit = true;
	for(int i = 0; i < list.GetCount(); i++) {
		if(SvnOptions *o = dynamic_cast<SvnOptions *>(list.GetCtrl(i, 0)))
			commit = o->commit;
		else
		if(GitOptions *o = dynamic_cast<GitOptions *>(list.GetCtrl(i, 0)))
			commit = o->commit;
		else {
			for(int j = 0; j < 2; j++) {
				Ctrl *ctrl = list.GetCtrl(i, j);
				if(ctrl)
					ctrl->Enable(commit);
			}
		}
	}
}

void RepoSync::SyncList()
{
	list.Clear();
	credentials.Show();
	svndir.Clear();
	for(const auto& w : work) {
		String path = GetFullPath(w.dir);
		int hi = list.GetCount();
		Color bk = AdjustIfDark(LtYellow());
		list.Add(REPOSITORY, path,
		         AttrText().Paper(bk),
		         AttrText(path).SetFont(ArialZ(20).Bold()).Paper(bk),
		         AttrText().Paper(bk));
		list.SetLineCy(hi, Zy(26));
		bool actions = false;
		if(w.kind == SVN_DIR) {
			auto& o = list.CreateCtrl<SvnOptions>(hi, 0, false);
			o.SizePos();
			o.commit = true;
			o.commit << [=] { SyncCommits(); };
			o.update = true;
			actions = ListSvn(path);
			if(!actions || w.read_only) {
				o.commit = false;
				o.commit.Disable();
			}
			credentials.Show();
			svndir.FindAdd(GetSvnDir(w.dir));
		}
		if(w.kind == GIT_DIR) {
			auto& o = list.CreateCtrl<GitOptions>(hi, 0, false);
			o.SizePos();
			o.commit = true;
			o.commit << [=] { SyncCommits(); };
			o.push = true;
			o.pull = true;
			actions = ListGit(path);
			if(!actions || w.read_only) {
				o.commit = false;
				o.push = false;
				o.commit.Disable();
			}
			if(w.read_only) {
				o.push = false;
				o.push.Disable();
			}
		}
		if(actions) {
			list.Add(MESSAGE, Null, AttrText("Commit message:").SetFont(StdFont().Bold()));
			list.SetLineCy(list.GetCount() - 1, (3 * EditField::GetStdHeight()) + 4);
			list.SetCtrl(list.GetCount() - 1, 1, message.Add().SetFilter(CharFilterSvnMsgRepo).VSizePos(2, 2).HSizePos());
			int q = msgmap.Find(w.dir);
			if(q >= 0) {
				message.Top() <<= msgmap[q];
				msgmap.Unlink(q);
			}
		}
		else
			list.Add(-1, Null, "", AttrText("Nothing to do").SetFont(StdFont().Italic()));
	}
}

void RepoSync::DoDiff(int ii)
{
	String f = list.Get(ii, 1);
	if(!IsNull(f))
		RunRepoDiff(f);
}

#ifdef PLATFORM_WIN32
void sRepoDeleteFolderDeep(const char *dir)
{
	{
		FindFile ff(AppendFileName(dir, "*.*"));
		while(ff) {
			String name = ff.GetName();
			String p = AppendFileName(dir, name);
			if(ff.IsFile()) {
				SetFileAttributes(p, GetFileAttributes(p) & ~FILE_ATTRIBUTE_READONLY);
				FileDelete(p);
			}
			else
			if(ff.IsFolder())
				sRepoDeleteFolderDeep(p);
			ff.Next();
		}
	}
	DirectoryDelete(dir);
}
#else
void sRepoDeleteFolderDeep(const char *path)
{
	DeleteFolderDeep(path);
}
#endif

void RepoSvnDel(const char *path)
{
	FindFile ff(AppendFileName(path, "*.*"));
	while(ff) {
		if(ff.IsFolder()) {
			String dir = AppendFileName(path, ff.GetName());
			if(ff.GetName() == ".svn")
				sRepoDeleteFolderDeep(dir);
			else
				RepoSvnDel(dir);
		}
		ff.Next();
	}
}

void RepoSync::Dir(bool read_only, const char *dir, int kind)
{
	Work& d = work.Add();
	d.kind = kind;
	d.dir = dir;
	d.read_only = read_only;
}

void RepoSync::Dir(bool read_only, const char *dir)
{
	int kind = GetRepoKind(dir);
	if(kind)
		Dir(read_only, dir, kind);
}

void RepoMoveSvn(const String& path, const String& tp)
{
	FindFile ff(AppendFileName(path, "*.*"));
	while(ff) {
		String nm = ff.GetName();
		String s = AppendFileName(path, nm);
		String t = AppendFileName(tp, nm);
		if(ff.IsFolder()) {
			if(nm == ".svn")
				FileMove(s, t);
			else
				RepoMoveSvn(s, t);
		}
		ff.Next();
	}
}

void RepoSync::DoSync()
{
	SyncList();
	msgmap.Sweep();
again:
	Enable();
	if(Run() != IDOK || list.GetCount() == 0) {
		int repoi = 0;
		for(int i = 0; i < list.GetCount(); i++)
			if(list.Get(i, 0) == MESSAGE)
				msgmap.GetAdd(work[repoi++].dir) = list.Get(i, 3);
		return;
	}
	Disable();
	bool changes = false;
	for(int i = 0; i < list.GetCount(); i++) {
		int action = list.Get(i, 0);
		if(action == MESSAGE) {
			if(changes && IsNull(list.Get(i, 3)) && list.GetCtrl(i, 1)->IsEnabled()
			   && !PromptYesNo("Commit message is empty.&Do you want to continue?"))
				goto again;
			changes = false;
		}
		else if(action != REPOSITORY && list.Get(i, 2) == 0)
			changes = true;
	}
	UrepoConsole sys;
	int repoi = 0;
	int l = 0;
	while(l < list.GetCount()) {
		SvnOptions *svn = dynamic_cast<SvnOptions *>(list.GetCtrl(l, 0));
		GitOptions *git = dynamic_cast<GitOptions *>(list.GetCtrl(l, 0));
		String repo_dir = work[repoi++].dir;
		String url;
		if(git) {
			url = GetGitUrl(repo_dir);
			if(url.GetCount())
				sys.Log("git origin url: " + url, Gray());
			
		}
		if(svn) {
			url = GetSvnUrl(repo_dir);
			if(url.GetCount())
				sys.Log("svn repository url: " + url, Gray());
		}
		l++;
		String message;
		String filelist;   // <-- list of files to update
		bool commit = false;
		while(l < list.GetCount()) {
			int action = list.Get(l, 0);
			if(action == REPOSITORY)
				break;
			String path = list.Get(l, 1);
			bool revert = list.Get(l, 2) == 1;
			if(svn && svn->commit) {
				if(action == MESSAGE && commit) {
					String msg = list.Get(l, 3);
					if(sys.CheckSystem(SvnCmd(sys, "commit", repo_dir) << filelist << " -m \"" << msg << "\""))
						msgmap.GetAdd(repo_dir) = msg;
					l++;
					break;
				}
				
				if(SvnFile(sys, filelist, action, path, revert))
					commit = true;
			}
			if(git && git->commit) {
				if(action == MESSAGE && commit) {
					String msg = list.Get(l, 3);
					if(sys.Git(repo_dir, "commit -a -m \"" << msg << "\""))
						msgmap.GetAdd(repo_dir) = msg;
					l++;
					break;
				}
				
				if(GitFile(sys, action, path, revert))
					commit = true;
			}
			l++;
		}
		if(svn && svn->update)
			sys.CheckSystem(SvnCmd(sys, "update", repo_dir).Cat() << repo_dir);
		if(git && git->push)
			sys.Git(repo_dir, "push", true);
		if(git && git->pull)
			sys.Git(repo_dir, "pull --ff-only", true);
	}
	sys.Log("Done", Gray());
	ResetBlitz();
	sys.Perform();
}

bool RepoSync::GitFile(UrepoConsole& sys, int action, const String& path, bool revert)
{
	String repo_dir = GetFileFolder(path);
	String file = GetFileName(path);
	if(revert) {
		if(action != ADD)
			sys.Git(repo_dir, "checkout \"" + file + "\"");
		return false;
	}
	if(action == ADD)
		sys.Git(repo_dir, "add \"" + file + "\"");
	return true;
}

bool RepoSync::SvnFile(UrepoConsole& sys, String& filelist, int action, const String& path, bool revert)
{
	if(revert) {
		if(action == REPLACE)
			DeleteFolderDeep(path);
		if(action != ADD)
			sys.CheckSystem("svn revert \"" + path + "\"");
		return false;
	}
	if(action >= 0 || action == SVN_ACTION)
		filelist << " \"" << path << "\"";   // <-- add the file to the list
	switch(action) {
	case ADD:
		RepoSvnDel(path);
		sys.CheckSystem("svn add --force \"" + path + "\"");
		break;
	case REMOVE:
		sys.CheckSystem("svn delete \"" + path + "\"");
		break;
	case CONFLICT:
		sys.CheckSystem("svn resolved \"" + path + "\"");
		break;
	case REPLACE: {
			RepoSvnDel(path);
			String tp = AppendFileName(GetFileFolder(path), Format(Uuid::Create()));
			FileMove(path, tp);
			sys.CheckSystem(SvnCmd(sys, "update", path) << " \"" << path << "\"");
			RepoMoveSvn(path, tp);
			sRepoDeleteFolderDeep(path);
			FileMove(tp, path);
			Vector<String> ln = Split(RepoSys("svn status \"" + path + "\""), CharFilterCrLf);
			for(int l = 0; l < ln.GetCount(); l++) {
				String h = ln[l];
				if(h.GetCount() > 7) {
					String file = h.Mid(7);
					if(IsFullPath(file)) {
						h.Trim(7);
						if(h == "?      ")
							sys.CheckSystem("svn add --force \"" + file + "\"");
						if(h == "!      ")
							sys.CheckSystem("svn delete \"" + file + "\"");
					}
				}
			}
		}
		break;
	case DELETEC:
		FileDelete(path);
		break;
	}
	return findarg(action, SVN_IGNORE, DELETEC) < 0;
}

void RepoSync::Serialize(Stream& s)
{
	int version = 0;
	s / version;
	s % msgmap;
}

void RepoSync::SetMsgs(const String& s)
{
	LoadFromString(*this, Garble(s));
}

String RepoSync::GetMsgs()
{
	return Garble(StoreAsString(*this));
}

int GetRepoKind(const String& p)
{
	if(IsNull(p))
		return NOT_REPO_DIR;
	if(DirectoryExists(AppendFileName(p, ".svn")) || DirectoryExists(AppendFileName(p, "_svn")))
		return SVN_DIR;
	String path = p;
	String path0;
	while(path != path0) {
		path0 = path;
		if(DirectoryExists(AppendFileName(path, ".git")))
			return GIT_DIR;
		DirectoryUp(path);
		if(DirectoryExists(AppendFileName(path, ".svn")))
			return SVN_DIR;
	}
	return NOT_REPO_DIR;
}
