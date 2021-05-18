#include "www.h"

#include <plugin/ftp/ftp.h>

#define IMAGECLASS WWW
#define IMAGEFILE  <uppweb/www.iml>
#include <Draw/iml.h>

#define TFILE <uppweb/www.t>
#include <Core/t.h>

#define LLOG(x)  // LOG(x)

#ifdef PLATFORM_WIN32 
String rootdir =   "d:\\devweb\\upp.src";
String uppbox =    rootdir + "uppbox";
String uppsrc =    rootdir + "uppsrc";
String reference = rootdir + "reference";
String examples =  rootdir + "examples";
String targetdir = "u:\\devweb\\uppwww";
String pdfdir    = "u:\\devweb\\pdf";
#else

#ifdef _DEBUG
String rootdir   = GetHomeDirFile("upp.src");
String uppbox    = rootdir + "uppbox";
String uppsrc    = rootdir + "uppsrc";
String reference = rootdir + "reference";
String examples  = rootdir + "examples";
String targetdir = GetHomeDirFile("www");
String diffdir   = GetHomeDirFile("wwwd");
String pdfdir    = GetHomeDirFile("www");
#else
String rootdir = "/root/upp.src";
String uppbox =    rootdir + "uppbox";
String uppsrc =    rootdir + "uppsrc";
String reference = rootdir + "reference";
String examples =  rootdir + "examples";
String targetdir = "/var/www";
String diffdir   = "/root/wwwupp";
String pdfdir    = "/var/www";
#endif

#endif
String bazaar;
bool outPdf;
bool outHtml;
bool doSvn;

String GetRcFile(const char *s)
{
	String f = GetDataFile(s);
	if(FileExists(f))
		return f;
	return GetHomeDirFile("upp.src/uppbox/uppweb/" + AsString(s));
}

bool ContainsAt(const String &source, const String &pattern, int pos)
{
	return    pos >= 0
	       && pos + pattern.GetLength() <= source.GetLength()
	       && 0 == memcmp(source.Begin() + pos, pattern.Begin(), pattern.GetLength());
}

bool StartsWith(const String &source, const String &pattern)
{
	return ContainsAt(source, pattern, 0);
}

bool EndsWith(const String &source, const String &pattern)
{
	return ContainsAt(source, pattern, source.GetLength() - pattern.GetLength());
}

String ImgName(int q)
{
	return Sprintf("%di.png", q);
}

typedef Image (*ImageFn)();

namespace Upp {
template<>
hash_t GetHashValue(const ::ImageFn& fn) { return (unsigned)(uintptr_t) fn; }
};

String GetImageSrc(ImageFn img)
{
	static Index<ImageFn> il;
	int q = il.Find(img);
	if(il.Find(img) < 0) {
		q = il.GetCount();
		il.Add(img);
		PNGEncoder png;
		png.SaveFile(AppendFileName(targetdir, ImgName(q)),(*img)());
	}
	return ImgName(q);
}

Htmls Wimg(ImageFn img)
{
	return HtmlImg(GetImageSrc(img)).Border(0);
}

Htmls RoundFrame(Htmls data, String border, Color bg)
{
	return HtmlPackedTable().BgColor(bg).Width(-100)
	          .Attr("style", "border-style: solid; border-width: 1px; border-color: #" + border + ";")
	       / HtmlLine() / data;
}

HtmlTag BoxWidth(int width)
{
	return HtmlPackedTable().Width(width) / HtmlLine();
}

Htmls BarSection(const char *txt)
{
	Htmls bar;
	bar.Br();
	bar << HtmlPackedTable().BgColor(WhiteGray).Width(-100) /
			(HtmlRow() / (
				HtmlCell().Width(10) / "&nbsp;" +
				HtmlCell() / HtmlArial(8) / HtmlBold() / txt +
				HtmlCell().Width(10) / "&nbsp;"
			));
	return bar;
}

Htmls BarCaption(const char *text)
{
	return HtmlLine()
	         .Width(-100)
	         .VCenter()
	         .Style("background-image: url('" + GetImageSrc(WWW::Caption) + "'); "
			        "border: 0px solid black;"
	                "padding-left:12px; padding-right:0px; "
	                "padding-top:1px; padding-bottom:1px;"
	                "color:#FFFFFF;")
	         / HtmlBold() / HtmlArial(8) / text;
}

Htmls BarCaptionLang(const char *text)
{
	return HtmlLine()
	         .Width(20)
	         .VCenter()
	         .Style("background-image: url('" + GetImageSrc(WWW::News2) + "'); "
			        "border: 0px solid black;"
	                "padding-left:12px; padding-right:0px; "
	                "padding-top:1px; padding-bottom:1px;"
	                "color:#FFFFFF;")
	         / HtmlBold() / HtmlArial(8) / text;
}

Htmls BarItem(Htmls content, const char *style)
{
	String bgStyle = "background-image: url('" + GetImageSrc(WWW::Button) + "'); ";
	return HtmlLine().Width(-100).VCenter().Style(bgStyle + style)
	         / content;
}

Htmls BarLink(const char *link, const char *text, bool nxt = true)
{
	String style = "border: 0px solid black; "
	               "padding-left:12px; padding-right:0px; "
	               "padding-top:6px; padding-bottom:6px;";
	if(nxt)
	  style += " border-top: 1px solid #6E89AE;";

	return BarItem( HtmlLink(link).Class("l1") / text, style );
}

Htmls SearchBar(const char *domain)
{
	Htmls form =
	  HtmlForm("https://www.google.com/search", false, "GET")
	    .Style("margin:0px;padding:0px;") /
	    ( HtmlHidden("ie", "UTF-8") +
	      HtmlHidden("oe", "UTF-8") +
	      ~HtmlEdit("q", 15).Attr("placeholder", t_("Site search")) +
	      ~HtmlHidden("domains", domain) +
	      ~HtmlHidden("sitesearch", domain)
	    );

	Htmls content;
	content << HtmlPackedTable().Width(-100)
	           / HtmlRow() / (
	               HtmlCell() / Wimg(WWW::google) +
	               HtmlCell() / form
	            );

	String style = "border: 0px solid black; "
	               "padding-left:6px; padding-right:0px; "
	               "padding-top:4px; padding-bottom:4px;"
		           "border-top: 1px solid #6E89AE;";
	return BarItem(content, style);
}

HtmlTag HtmlPadding(int p)
{
	return HtmlPackedTable().Width(-100) /
		   HtmlLine();
}

VectorMap<String, String> escape;

String QtfAsHtml(const char *qtf, Index<String>& css,
                 const VectorMap<String, String>& links,
                 const VectorMap<String, String>& labels,
                 const String& outdir, const String& fn = Null)
{
	return EncodeHtml(ParseQTF(qtf), css, links, labels, outdir, fn, Zoom(8, 40), escape, 40);
}

String GetText(const char *s)
{
	return GetTopic(s).text;
}

String ChangeTopicLanguage(const String &topic, int lang) {
	int pos = topic.ReverseFind('$');
	if (pos < 0)
		return "";			
	String langtxt = ToLower(LNGAsText(lang));		
	return topic.Left(pos+1) + langtxt + topic.Mid(pos+1+langtxt.GetCount()); 
}

String GetTopicLanguage(const String &topic) {
	int pos = topic.ReverseFind('$');
	if (pos < 0)
		return "";			
	return topic.Mid(pos+1, 5); 
}

String FormatDateRFC822(const Time& t) {
	int tz = int((GetSysTime()-GetUtcTime())/60);
	return Format("%s, %d %Mon %d %02d:%02d:%02d %s%04d",
	              DayName(DayOfWeek(t)).Left(3),
	              t.day,t.month,t.year,
	              t.hour,t.minute,t.second,
	              tz>0?"+":"",tz/60*100+(tz+1440)%60);
}

void CreateRssFeed() {
	String header="<rss version=\"2.0\" xmlns:atom=\"http://www.w3.org/2005/Atom\">\n" 
	"<channel>\n"
	"<title>Ultimate++ svn changes</title>\n" 
	"<link>http://ultimatepp.org/</link>\n"
	"<description>This feed offers list of commits to Ultimate++ framework svn repository</description>\n"
	"<lastBuildDate>"+ FormatDateRFC822(GetSysTime()) + "</lastBuildDate>\n"
	"<language>en-us</language>\n"
	"<atom:link href=\"http://ultimatepp.org/svnchanges.xml\" rel=\"self\" type=\"application/rss+xml\" />\n\n";
	
	String items;
	for(int i = 0; i < min(30,svnlog.GetCount()); i++){
		items+="<item>\n"
		"<title>Revision " + svnlog[i].revision + "</title>\n" 
		"<link>http://code.google.com/p/upp-mirror/source/detail?r=" + svnlog[i].revision + "</link>\n"
		"<guid>http://code.google.com/p/upp-mirror/source/detail?r=" + svnlog[i].revision + "</guid>\n"
		"<pubDate>" + FormatDateRFC822(svnlog[i].time) + "</pubDate>\n"
		"<description><![CDATA[\n"
		"	<table style=\"font-size:small;\">\n" 
		"		<tr><td>Revision:</td><td><b>" + svnlog[i].revision + "</b></td></tr>\n"
		"		<tr><td>Description:</td><td><code>" + svnlog[i].msg + "</code></td></tr>\n"
		"		<tr><td>Submitted:</td><td><i>" + FormatDateRFC822(svnlog[i].time) + "</i> by <i>" + svnlog[i].author + "</i></td></tr>\n"
		"		<tr><td>Affected files:</td><td>&nbsp;</td></tr>\n"
		"		<tr><td colspan=\"2\">\n"
		"			<div style=\"margin-left:20px;\">\n";
		for(int j = 0; j < svnlog[i].changes.GetCount(); j++)
			items+="<a href=\"http://code.google.com/p/upp-mirror/source/diff?spec=svn" + svnlog[i].revision + "&amp;r=" + svnlog[i].revision + "&amp;format=side&amp;path=" + svnlog[i].changes[j].path + "\" target=\"gcode\">" + svnlog[i].changes[j].action + "</a>&nbsp;"
			       "<a href=\"http://code.google.com/p/upp-mirror/source/browse" + svnlog[i].changes[j].path + "\" target=\"gcode\">" + svnlog[i].changes[j].path + "</a><br>\n";
		items+=
		"			</div>\n"
		"		</td></tr>\n"
		"	</table>\n"
		"]]></description>\n" 
		"</item>\n\n";
	}
	SaveFile(AppendFileName(targetdir, "svnchanges.xml"), header + items + "</channel>\n</rss>\n");
}

ArrayMap<String, Topic> tt;
//Vector<String> ttFullTitles;
Vector<int> ttId;
Vector<String> ttFullIds;

String Www(const char *topic, int lang, String topicLocation = "topic://uppweb/www/")
{
	String strLang = ToLower(LNGAsText(lang));
	String www = GatherTopics(tt, ttFullIds, String().Cat() << topicLocation << topic << "$" << strLang, "");
	if (www != "index.html")
		return www;
	return GatherTopics(tt, ttFullIds, String().Cat() << topicLocation << topic << "$" << "en-us", "");
}

struct Title {
	String title, link;
	Title(String title, String link) : title(TrimBoth(title)), link(link) {};
	bool operator < (const Title& other) const {
		const char *val = title.Begin();
		const char *valOther = other.title.Begin();
		for(;; val++, valOther++) {
			String str, strOther;
			str.Clear();
			strOther.Clear();
			for (; *val != '\0' && *val != '.' && *val != ' '; ++val)
				str.Cat(*val);
			for (; *valOther != '\0' && *valOther != '.' && *valOther != ' '; ++valOther)
				strOther.Cat(*valOther);
			
			int num = ScanInt(str);
			int numOther = ScanInt(strOther);
			if (IsNull(num) && IsNull(numOther))
				return str < strOther;
			if (IsNull(num))
				return true;
			if (IsNull(numOther))
				return false;
			if (num != numOther)
				return num < numOther;
		}
		return title < other.title;
	}
};

String FolderLinks(String package, String group, int lang, int level, String parentIds)
{
	String qtf;
	Array<Title> titles;
	FindFile ff(AppendFileName(AppendFileName(AppendFileName(uppsrc, package), group + ".tpp"), "*.tpp"));
	while(ff) {
		if(ff.IsFile()) {
			if (ff.GetName().Find("en-us") >= 0) {
				String title;
				String tl = "topic://" + package + '/' + group + '/' + GetFileTitle(ff.GetName());
				GatherTopics(tt, ttFullIds, tl, title, parentIds);
				titles.Add(Title(DeQtf(Nvl(title, tl)), tl));
				//qtf << "________[^" << tl << "^ " << DeQtf(Nvl(title, tl)) << "]&";
			}
		}
		ff.Next();
	}
	Sort(titles);
	for (int i = 0; i < titles.GetCount(); ++i)
		qtf << "________[^" << titles[i].link << "^ " << titles[i].title << "]&";
	return qtf;
}

void AddFiles(String& qtf, const String& dir, const char* ext, bool& b)
{
	FindFile ff(AppendFileName(dir, "*." + String(ext)));
	while(ff) {
		qtf << "[A4* " << DeQtf(ff.GetName()) << "]&&"
		    << CppAsQtf(LoadFile(AppendFileName(dir, ff.GetName())))
		    << "&&&";
		ff.Next();
		b = true;
	}
}

struct Isort {
	bool operator()(const String& a, const String& b) const
	{
		return ToUpper(a) < ToUpper(b);
	}
};

String MakeExamples(const char *dir, const char *www, int language, String parentIds)
{
	String ttxt;
	FindFile ff(AppendFileName(dir, "*.*"));
	ttxt << "{{1:3 ";
	bool next = false;
	Vector<String> ls;
	while(ff) {
		if(ff.IsFolder())
			ls.Add(ff.GetName());
		ff.Next();
	}
	Sort(ls, Isort());
	for(int i = 0; i < ls.GetCount(); i++) {
		String name = ls[i];
		String link = String().Cat() << www << '$' << name << "$" << ToLower(LNGAsText(language)) << ".html";
		Topic& topic = tt.Add(link);
		topic.title = name;
		int q = tt.GetCount() - 1;
		String fullIds = parentIds + "/" + FormatInt(q);
		ttFullIds.Add(fullIds);
		
		String fn = AppendFileName(
						AppendFileName(
							AppendFileName(uppbox, "uppweb"),
							String(www) + ".tpp"
						),
						topic.title + "_" + ToLower(LNGAsText(language))  + ".tpp"
					);
		String h = ReadTopic(LoadFile(fn)).text;
		Package p;
		p.Load(AppendFileName(AppendFileName(dir, name), name + ".upp"));
		topic.text << "[R6* " << name << "]&&" << DeQtf(p.description) << "&";
		if(h.GetCount())
			topic.text << h;
		topic.text << "[A2<l0r0 &&";
		String d = AppendFileName(dir, name);
		bool b = false;
		AddFiles(topic.text, d, "h", b);
		AddFiles(topic.text, d, "hpp", b);
		AddFiles(topic.text, d, "cpp", b);
		AddFiles(topic.text, d, "icpp", b);
		AddFiles(topic.text, d, "java", b);
		AddFiles(topic.text, d, "witz", b);
		AddFiles(topic.text, d, "usc", b);
		AddFiles(topic.text, d, "lay", b);
		AddFiles(topic.text, d, "key", b);
		AddFiles(topic.text, d, "brc", b);
		AddFiles(topic.text, d, "sch", b);
		AddFiles(topic.text, d, "xml", b);
		if(b) {
			if(next)
				ttxt << "\n::^ ";
			ttxt << "[^ " << link << "^* " << DeQtf(topic.title) << "]::^ "
			     << DeQtf(p.description);
			next = true;
		}
	}
	ttxt << "}}&&";
	return ttxt;
}

void SrcDocs(Index<String> &x, String& qtf, const char *folder, int lang, int level, String parent)
{
	if(x.Find(folder) >= 0)
		return;
	x.Add(folder);
	String srcdoc = FolderLinks(folder, "srcdoc", lang, level, parent);
	String src = FolderLinks(folder, "src", lang, level, parent);
	Package p;
	p.Load(AppendFileName(uppsrc, AppendFileName(folder, GetFileName(folder) + ".upp")));
	if(srcdoc.GetLength() || src.GetLength()) {
		qtf << "&&&[*4@b " << folder << "]&";
		if(!IsNull(p.description))
			qtf << "[2 " << p.description << "]&";
		if(srcdoc.GetCount()) {
			qtf << "&[3/* " + Format(t_("Using %s"), folder) << "]&";
			qtf << srcdoc;
		}
		if(src.GetCount()) {
			qtf << "&[3/* " << Format(t_("%s reference"), folder) << "]&";
			qtf << src;
		}
	}
}

int CharFilterLbl(int c)
{
	return IsAlNum(c) ? c : '.';
}


void QtfAsPdf(PdfDraw &pdf, const char *qtf)
{
	RichText txt = ParseQTF(qtf);
	Size page = Size(3968, 6074); 
	UPP::Print(pdf, txt, page);
}

VectorMap<String, String> links;
VectorMap<String, String> labels;
Htmls header, lastUpdate;

Array <Htmls> bar;
Array <int> languages;

int GetLinkLanguage(const String &link) {
	int pos = link.ReverseFind('$');
	if (pos < 0)
		return 0;
	int lang = LNGFromText(ToUpper(link.Mid(pos+1)));
	for (int i = 0; i < languages.GetCount(); ++i) {
		if (languages[i] == lang)
			return i;
	}
	return 0;
}

void ExportPage(int i)
{
	Index<String> css;
	String path = links.GetKey(i);
	RLOG("Exporting " << path);
	
	int ilang = GetLinkLanguage(path); 
	SetLanguage(languages[ilang]);
	String text = GetText(path);
	int h;
	h = ParseQTF(tt[i].text).GetHeight(1000);
	
	int isvn = svndata.Find(tt.GetKey(i));
	String qtflangs;
	String googleFile;	
	if (isvn > -1) {   				
		String txt = String("[2 ") + t_("Last edit by %s on %s") + ".]";	
		qtflangs += Format(txt, svndata[isvn].author, Format(Date(svndata[isvn].time)));
		googleFile = svndata[isvn].fullPath;
		if (googleFile.GetCount() > rootdir.GetCount())
			googleFile = UnixPath(googleFile.Mid(rootdir.GetCount()));
		else
			googleFile = "";
	}
	String strlang;
	String jslang;
	Array <String> arrLangs;
	for (int il = 0; il < languages.GetCount(); ++il) {
		String topic = ChangeTopicLanguage(path, languages[il]);
		int itopic;
		if ((itopic = tt.Find(topic)) >= 0) {
			if (tt[itopic].title.Find(" (translated)") < 0) {
				if (il != ilang) {
					if (!strlang.IsEmpty())
						strlang << ", ";
					arrLangs.Add("[^" + links[itopic] + "^ [2 " + ToLower(GetNativeLangName(languages[il])) + "]]");
				}
				jslang+="'"+ links[itopic] +"':'" + GetNativeLangName(languages[il]) + "',";
			}
		}
	}
	if(!jslang.IsEmpty())
		jslang.Trim(jslang.GetCount()-1);
	if (arrLangs.GetCount() > 0) {
		for (int i = 0; i < arrLangs.GetCount(); ++i) {
			if (i == arrLangs.GetCount()-1 && i != 0)
				strlang << String(" ") + t_("and") + " ";
			else if (i > 0)
				strlang << ", ";
			strlang << arrLangs[i];
		}
	}	
//	if (!strlang.IsEmpty())
//		qtflangs += Format(String("[2  ") + t_("This page is also in %s") + ".]", strlang);
	if (tt[i].title.Find("How to contribute. Web page") < 0) {
		String help = "topic://uppweb/www/contribweb$" + ToLower(LNGAsText(languages[ilang]));
		qtflangs += " " + String("[^") + help + "^ [<A2 " + t_("Do you want to contribute?") + "]]";
		if (googleFile != "")
			qtflangs += ". [^https`:`/`/raw`.githubusercontent`.com`/ultimatepp`/mirror`/master" + DeQtf(UrlEncode(googleFile)) + "^/1 " + "T`+`+" + "]";
	}

	String langs = QtfAsHtml(qtflangs, css, links, labels, targetdir, links[i]);
	String page = tt[i];
		
	Array<String> htmlrep;
	int posB = 0;
	while (true) {
		posB = page.Find("[IHTMLTEXT", posB);
		if (posB < 0)
			break; 
		int posBB = posB + (int)strlen("[IHTMLTEXT");
		int pos0 = page.ReverseFind("[", posB-1);
		int posE = page.Find(";2", posBB);
		String html0 = page.Mid(posBB, posE - posBB);
		html0.Replace("`", "");
		htmlrep.Add(html0);
		int posEE = page.Find("]&]", posE) + (int)strlen("]&]");
		
		page = page.Left(pos0) + "QTFHTMLTEXT" + FormatInt(htmlrep.GetCount()-1) + page.Mid(posEE+1);
	}
	
	page = QtfAsHtml(page, css, links, labels, targetdir, links[i]);
	for (int iHtml = 0; iHtml < htmlrep.GetCount(); ++iHtml) 
		page.Replace(String("QTFHTMLTEXT") + FormatInt(iHtml), htmlrep[iHtml]);
	
	Color paper = SWhite;
	if(path == "topic://uppweb/www/download$en-us")
		page << LoadFile(GetRcFile("adsense3.txt"));
/*		if(path == "topic://uppweb/www/index$en-us") {
		for(int q = 0; q < news.GetCount(); q++) {
			String n = GetText("uppweb/www_news/" + news[q]);
			String h = news[q];
			int i = h.Find('$');
			if(i >= 0)
				h = h.Mid(0, i);
			if(h.GetLength() == 8)
				h = h.Mid(0, 4) + '-' + h.Mid(4, 2) + '-' + h.Mid(6, 2);
			page << "<br>";
			page << "<div style='font-family:sans-serif; font-weight: bold; "
			        "font-height: 12px; color: White; background: #2020d0'>&nbsp;&nbsp;"
			     << h << "</div><br>";
			page << QtfAsHtml(n, css, links, targetdir, FormatIntAlpha(q) + "_n");
			page << "<br>";
		}
	}*/
	Color bg = Color(210, 217, 210);
	Htmls footer;
	footer << HtmlTable().Border(0).Width(-100) / HtmlLine() +
				~RoundFrame(HtmlPadding(8) / langs , "6E89AE;padding: 10px;", White);
	Htmls html;
	html <<
		HtmlPackedTable().Width(-100) /
		   	HtmlLine().ColSpan(3) / header +
		HtmlPackedTable().Width(-100) / (
			HtmlLine().ColSpan(3).BgColor(bg).Height(6) / "" +
			HtmlRow() / (
				HtmlTCell().Center() / BoxWidth(160).Center() / (
					bar[GetLinkLanguage(path)] +
					"<br>" +
					LoadFile(GetRcFile("facebook.txt")) +
					"<br><br>" +
//						"<p align=\"center\">" +
					LoadFile(GetRcFile("adsense2.txt")) +
					"<br><br>" +
					LoadFile(GetRcFile("adlinks.txt")) +
					(h > 25000 ? "<br><br>" + LoadFile(GetRcFile("adsense2.txt"))
					                                : "") +
			       	"<br><br><br>" +
//						LoadFile(GetRcFile("referral.txt")) +
//						LoadFile(GetRcFile("referral2.txt")) +
//						LoadFile(GetRcFile("donations.txt")) +
//						"<br><br>" +
//						amazon[i % amazon.GetCount()] +
			       	"<br><br><br>" +
					~(HtmlLink("https://sourceforge.net/projects/upp/") /
					   HtmlImg("https://sourceforge.net/sflogo.php?group_id=93970&type=2",
					           "SourceForge.net Logo").Border(0).Width(125).Height(37)) +
			       	"<br><br>" +
			       	(links[i] == "index.html" ? lastUpdate : Htmls())
				) +
				HtmlTCell().BgColor(bg) / BoxWidth(6) / "" +
				HtmlTCell().Width(-100).BgColor(bg) / (
					RoundFrame(HtmlPadding(8) / page , "6E89AE;padding: 10px;", White) +
					footer
				)
			)
		) + Htmls("<script type=\"text/javascript\">var a={"+jslang+"};"
				"var p=window.location.pathname.split(\"/\");p=p[p.length-1];"
				"var s='<select id=\"lang\" onchange=\"window.location=document.getElementById(\\'lang\\').value;\">';"
				"for(l in a){if(p==l){d=\" selected=1\"}else{d=\"\"}s+='<option value=\"'+l+'\"'+d+'>'+a[l]+'</option>'}"
				"s+='</select>';"
				"var d=document.createElement('div');d.innerHTML=s;"
				"var c=document.getElementById('langs');c.replaceChild(d,c.firstChild);"
				"document.getElementById('langbox').style.display='block';</script>");

	String topicTitle = tt.GetKey(i);
	String pageTitle = tt[i].title;
	if(IsNull(pageTitle))
		pageTitle = "Ultimate++";
	if(StartsWith(topicTitle, "examples_"))
		pageTitle = "Demos / " + pageTitle;
	else if(StartsWith(topicTitle, "reference_"))
		pageTitle = "Examples / " + pageTitle;

	if(pageTitle != "Ultimate++")
		pageTitle << " :: Ultimate++";

	Htmls content =
	    //"<!DOCTYPE html>" +
	    "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n" +
		HtmlHeader(pageTitle, AsCss(css) +
			"a.l1         { text-decoration:none; font-size: 8pt; font-family: sans-serif; "
			              "font-weight: normal; }\n"
			"a.l1:link    { color:#000000; }\n"
			"a.l1:visited { color:#000080; }\n"
			"a.l1:hover   { color:#9933CC; }\n"
			"a.l1:active  { color:#000000; }\n"
			"a.l2         { text-decoration:none; font-size: 12pt; font-family: sans-serif; "
			              "font-variant: small-caps; }\n"
			"a.l2:link    { color:#0066FF; }\n"
			"a.l2:visited { color:#FF6600; }\n"
			"a.l2:hover   { color:#BC0624; }\n"
			"a.l2:active  { color:#BC0024; }\n",
			"<META NAME=\"keywords\" "
			"CONTENT=\""
			"framework, toolkit, widget, c++, visual, studio, dev-cpp, builder, ide, class, component,"
			"wxwidgets, qt, rapid, application, development, rad, mfc, linux, gui, sdl, directx, desktop"
			"\">"
	        "<META name=\"robots\" content=\"index,follow\">\n"
            "<LINK rel=\"alternate\" type=\"application/rss+xml\" title=\"SVN changes\" href=\"svnchanges.xml\">\n"
            "<LINK rel=\"shortcut icon\" type=\"image/png\" href=\"favicon.png\">\n"
		)


	    .BgColor(bg)
	    .Alink(Red).Link(Black).Vlink(Blue)
	    / html;
	if(!SaveFile(AppendFileName(targetdir, links[i]), content)) {
		RLOG("SaveFile failed!");
		abort();
	}
	RLOG("Exported page " << AppendFileName(targetdir, links[i]));
}

struct DownloadItem {
	Time tm;
	String fn;
	String path;
	int64 len;
};

String Downloads()
{
	String r;
	r << "{{1:2:2:2^@L "
	     "Date:: "
	     "[^app$ide$install_win32_en-us.html^ U`+`+ for Windows (with CLANG)]:: "
	     "[^app$ide$install_posix_en-us.html^ U`+`+ for Linux/BSD/Solaris]:: "
	     "[^app$ide$install_macos_en-us.html^ U`+`+ for MacOS]";
	FindFile ff(AppendFileName(targetdir, "downloads/*.*"));

	SortedArrayMap<Date, Array<DownloadItem>> downs;
	while(ff) {
		if(ff.IsFile()) {
			DownloadItem down;
			down.tm = ff.GetLastWriteTime();
			down.fn = ff.GetName();
			down.path = ff.GetPath();
			down.len = ff.GetLength();
			Date date = down.tm;
			downs.GetAdd(date).Add(down);
		}
		ff.Next();
	}
	
	auto FindDown = [](Array<DownloadItem>& a, const char* arch) -> String {
		String r;
		for(int i = 0; i < a.GetCount(); i++) {
			if(a[i].fn.Find(arch) >= 0) {
				r << FormatTime(a[i].tm, "hh:mm")
				  << " [^downloads/" << a[i].fn << "^ \1" << a[i].fn << "\1]"
				  << " (" << (a[i].len >> 20) << " MB)";
				break;
			}
		}
		return r;
	};
	
	Date d = GetSysDate();
	
	for(int i = 0;i < 14; i++, d--) {
		Array<DownloadItem>& a = downs.GetAdd(d);
		r << "::@W " << FormatDate(d, "YYYY-MM-DD") << ":: "
		  << FindDown(a, "win") << ":: "
		  << FindDown(a, "posix") << ":: "
		  << FindDown(a, "macos");
	}
	r << "}}";
	
	for(int i = 14; i < downs.GetCount(); i++) {
		if(downs.GetKey(i) < d)
			for(int j = 0; j < downs[i].GetCount(); j++)
				DeleteFile(downs[i][j].path);
	}

	return r;
}

struct ProgramData {
	String rootdir;
	String targetdir;
	String diffdir;
	String pdfdir;
	bool ftpUpload;
	bool outPdf;
	bool outHtml;
	bool doSvn;
	void Xmlize(XmlIO xml)	{
		xml
			("rootdir", rootdir)
			("targetdir", targetdir)
			("diffdir", diffdir)
			("pdfdir", pdfdir)
			("ftpUpload", ftpUpload)
			("outPdf", outPdf)
			("doSvn", doSvn)
			("outHtml", outHtml)
		;
	}
};

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
#ifdef PLATFORM_POSIX
	rootdir = GetHomeDirFile("upp.src");
	targetdir = GetHomeDirFile("uppwww");
	diffdir   = GetHomeDirFile("wwwupp");
	pdfdir   = GetHomeDirFile("pdf");
#endif
	outPdf = true;
	outHtml = true;
	doSvn = true;


	RLOG("--- uppweb started at " << GetSysTime());
	
	ProgramData data;
	
	String configFile = GetHomeDirFile("uppweb.xml");
	const Vector<String>& cmd = CommandLine();
	if(cmd.GetCount() && FileExists(cmd[0]))
		configFile = cmd[0];
	
	Cout() << "ConfigFile: " << configFile << "\n";
	bool cfgloaded = false;
	if (FileExists(configFile)) {
		if (LoadFromXMLFile(data, configFile)) {
			rootdir   = data.rootdir;
			targetdir = data.targetdir;
//			diffdir   = data.diffdir;
			pdfdir    = data.pdfdir;	
//			ftpupload = data.ftpUpload;
			outPdf    = data.outPdf;
			outHtml   = data.outHtml;
			doSvn     = data.doSvn;
			cfgloaded = true;
		}
	}
	if (!cfgloaded) {
		data.rootdir   = rootdir;
		data.targetdir = targetdir;
//		data.diffdir   = diffdir;
		data.pdfdir    = pdfdir;
//		data.ftpUpload = ftpupload;
		data.outPdf    = outPdf;
		data.outHtml   = outHtml;
		data.doSvn	   = doSvn;
		StoreAsXMLFile(data, NULL, configFile);
	}
	Cout() << "RootDir: " << rootdir << "\n";
	Cout() << "OutHtml: " << outHtml << "\n";
	Cout() << "TargetDir: " << targetdir << "\n";
	Cout() << "OutPdf: " << outPdf << "\n";
	Cout() << "PdfDir: " << pdfdir << "\n";

#ifdef _DEBUG
	doSvn = false;
	outPdf = false;
#endif

	String downloads = Downloads();
	
	if (!DirectoryExists(rootdir)) {
		Cout() << ("Directory " + DeQtf(rootdir) + " does not exist\n");
		return;
	}

	uppbox =    AppendFileName(rootdir, "uppbox");
	uppsrc =    AppendFileName(rootdir, "uppsrc");
	reference = AppendFileName(rootdir, "reference");
	examples =  AppendFileName(rootdir, "examples");
	bazaar =    AppendFileName(rootdir, "bazaar");

	Cout() << "Loading www.tpp\n";
	InitWwwTpp();

	languages.Add(LNG_('E','N','U','S'));		// en-us has to be the first one
#if 0
#ifndef _DEBUG // too slow to have them all while developing
	languages.Add(LNG_('C','A','E','S'));
	languages.Add(LNG_('C','S','C','Z'));
	languages.Add(LNG_('D','E','D','E'));
	languages.Add(LNG_('E','S','E','S'));
	languages.Add(LNG_('E','U','E','S'));
	languages.Add(LNG_('F','R','F','R'));
	languages.Add(LNG_('R','O','R','O'));
	languages.Add(LNG_('R','U','R','U'));
	languages.Add(LNG_('Z','H','C','N'));
	languages.Add(LNG_('Z','H','T','W'));
#endif
#endif
	if (outHtml)
		RealizeDirectory(targetdir);
	
	if (outPdf)
		RealizeDirectory(pdfdir);
	
	Cout() << "Gather ref links " << uppsrc << "\n";
	GatherRefLinks(uppsrc);
	Cout() << "Gather ref links " << AppendFileName(rootdir, "bazaar") << "\n";
	GatherRefLinks(AppendFileName(rootdir, "bazaar"));
	
	if (outHtml)
		SaveFile(AppendFileName(targetdir, "sdj.gif"), LoadFile(GetRcFile("sdj.gif")));
	
	String release = "15947";
	escape.Add("RELEASE", release);
	escape.Add("RELEASET", release);
	escape.Add("UPDATETIME", Format("%`", GetUtcTime()));
	
	if (doSvn) {
		Cout() << "Processing svn\n";
		GetSvnList(svndata, rootdir);
		GetSvnLog(svnlog);
		CreateRssFeed();
		if (svnlog.GetCount() > 0) {
			escape.Add("LATESTSVN", svnlog[0].revision);
			//Index<String> css;
			//escape.Add("SVNTABLE", QtfAsHtml(SvnChanges(svnlog, "", 100), css, links, labels, targetdir));
			//escape.Add("ANCHOR", "<span id=\"svnTableAnchor\"></span>");
		}
	}

	escape.Add("PAYPAL", LoadFile(GetRcFile("donations.txt")));

	header = HtmlPackedTable()
	       .Width(-100)
	       .BgColor(White)
	       .Attr("style", "border: 1px solid #6E89AE;"
	                      "padding-left: 10px;padding-right: 0px;padding-top: 0px;padding-bottom: 0px;")
	       /
			HtmlRow() / (
			    HtmlCell() / HtmlLink("index.html") / Wimg(WWW::Logo6) +
			    HtmlCell().Right().Bottom()
			              .Style("padding-bottom: 5px; "
			                     "background-image: url('" + GetImageSrc(WWW::HB) + "')")
			    / HtmlArial(14) / (LoadFile(GetRcFile("adsense.txt")) + "&nbsp;&nbsp;"/* + "<br>..harnessing the real power of C++&nbsp;&nbsp;"*/)
			);
	
	bar.SetCount(languages.GetCount());

	int currentLang = GetCurrentLanguage();
	for (int i = 0; i < languages.GetCount(); ++i) {
		Cout() << "Language " << LNGAsText(languages[i]);
		Htmls bi, bex, bdoc, bcom, bcon, bsearch, blang;
		int lang = languages[i];
		
		SetLanguage(lang);
	
		Www("index", lang);
		Www("contribweb", lang);
	//	bi << BarLink("index.html", "Home", false);
		bi << BarLink(Www("overview", lang), t_("Overview"), false);
		bi << BarLink(Www("examples", lang), t_("Examples"));
		{
			IGNORE_RESULT(Www("reference", lang));
			
			int ri = tt.Find("topic://uppweb/www/reference$" + ToLower(LNGAsText(lang)));
			int di = tt.Find("topic://uppweb/www/examples$" + ToLower(LNGAsText(lang)));
			
			tt[di].text << MakeExamples(examples, "examples", lang, String("/") + FormatInt(di));
			tt[di].text << tt[ri].text << '\n';
			tt[di].text << MakeExamples(reference, "reference", lang, String("/") + FormatInt(di));
		}

		bi << BarLink(Www("ss", lang), t_("Screenshots"));
		bi << BarLink(Www("comparison", lang), t_("Comparisons"));
		bi << BarLink(Www("apps", lang), t_("Applications"));
		bi << BarLink(Www("download", lang), t_("Download"));

		bi << BarLink(Www("documentation", lang), t_("Documentation"));
		{
			int di = tt.Find("topic://uppweb/www/documentation$" + ToLower(LNGAsText(lang)));
			if (di >= 0) {
				Index<String> x;
				x.Clear();
				String qtf;
				SrcDocs(x, qtf, "Core", lang, 2, String("/") + FormatInt(di) + "/[Core]");
				SrcDocs(x, qtf, "Draw", lang, 2, String("/") + FormatInt(di) + "/[Draw]");
				SrcDocs(x, qtf, "CtrlCore", lang, 2, String("/") + FormatInt(di) + "/[CtrlCore]");
				SrcDocs(x, qtf, "CtrlLib", lang, 2, String("/") + FormatInt(di) + "/[CtrlLib]");
				SrcDocs(x, qtf, "RichText", lang, 2, String("/") + FormatInt(di) + "/[RichText]");
				SrcDocs(x, qtf, "RichEdit", lang, 2, String("/") + FormatInt(di) + "/[RichEdit]");
				SrcDocs(x, qtf, "Sql", lang, 2, String("/") + FormatInt(di) + "/[Sql]");
				SrcDocs(x, qtf, "Skylark", lang, 2, String("/") + FormatInt(di) + "/[Skylark]");
				String d = AppendFileName(uppsrc, "*.*");
				String p;
				for(int pass = 0; pass < 2; pass++) {
					FindFile ff(d);
					Array <String> folders;
					folders.Clear();
					while(ff) {
						if(ff.IsFolder() && ff.GetName() != "Web")
							folders.Add(p + ff.GetName());
						ff.Next();
					}
					Sort(folders);
					for (int ifold = 0; ifold < folders.GetCount(); ++ifold)
						SrcDocs(x, qtf, folders[ifold], lang, 2, String("/") + FormatInt(di) + "/[" + folders[ifold] + "]");
					d = AppendFileName(uppsrc, "plugin/*.*");
					p = "plugin/";
				}
				tt[di].text << qtf;
			}
		}
		bi << BarLink(Www("Tutorials", lang), t_("Tutorials"));
		bi << BarLink(Www("bazaar", lang), t_("Bazaar"));
		bi << BarLink(Www("Roadmap", lang), t_("Status & Roadmap"));
		bi << BarLink(Www("FAQ", lang), t_("FAQ"));
		bi << BarLink(Www("About", lang, "topic://ide/app/"), t_("Authors & License"));
	
		bi << BarLink("https://www.ultimatepp.org/forums", t_("Forums"));
	//	bcom << BarLink(Www("mailing"), "Mailing lists");
	//	bi << BarLink("http://www.ultimatepp.org/wiki/index.php", "Wiki");
		bi << BarLink(Www("Funding", lang), t_("Funding Ultimate++"));
	//	bcom << BarLink(Www("helpus"), "Getting involved");
	//	bcom << BarLink("mailto: upp@ntllib.org", "Contact developers");
		
		bsearch << BarCaption(t_("Search on this site"));
		bsearch << SearchBar("www.ultimatepp.org");

#if 0	
		blang << BarCaption(t_("Language"));
		blang << Htmls("<div id=\"langbox\" style=\"display:none;\">") + 
		         BarItem(HtmlPackedTable().Width(-100)
		           / HtmlRow() / (
		               HtmlCell() / Wimg(WWW::Language) +
		               HtmlCell() / Htmls("<div id=\"langs\">"+GetNativeLangName(lang)+"</div>")
		            ),"border: 0px solid black;"
		              "padding-left:6px; padding-right:0px;"
		              "padding-top:4px; padding-bottom:4px;"
		         ) + Htmls("</div>");
#endif

		
		HtmlTag bf = HtmlPackedTable()
		       .Width(-100)
		       .BgColor(White)
		       .Attr("style", "border-style: solid; border-width: 1px; border-color: #6E89AE;"
		                      "padding: 0px");
		String div = HtmlTable().Border(0).Width(-100) / HtmlLine();
		bar[i] = bf / bi + div +
	//	      bf / bex + div +
	//	      bf / bdoc + div +
	//	      bf / bcom + div +
	//	      bf / bcon + div +
		      bf / bsearch + div;
	//	      bf / blang + div;
	}
	SetLanguage(currentLang);

	for(int i = 0; i < tt.GetCount(); i++) {
		String topic = tt.GetKey(i);
		links.Add(topic, topic == "topic://uppweb/www/index$en-us" ? "index.html" :
		                 memcmp(topic, "topic://", 8) ? topic : TopicFileNameHtml(topic));
	}

	String svntableStr = DeQtf("[svntable]");
	for(int i = 0; i < tt.GetCount(); i++) {
		if (tt[i].title == "Svn releases") 
			tt[i].text.Replace(svntableStr, SvnChanges(svnlog, 300, ""));
		else if (tt[i].title == "Svn Web releases") 
			tt[i].text.Replace(svntableStr, SvnChanges(svnlog, 100, "uppweb"));
		else if (tt[i].title == "Svn Bazaar releases") 
			tt[i].text.Replace(svntableStr, SvnChanges(svnlog, 100, "bazaar"));
		else if (tt[i].title == "Svn Upp releases") 
			tt[i].text.Replace(svntableStr, SvnChanges(svnlog, 100, "uppsrc"));		
		else if (tt[i].title == "Svn major releases") 
			tt[i].text.Replace(svntableStr, SvnChanges(svnlog, 300, "", true));
		else if (tt[i].title == "Svn Web major releases") 
			tt[i].text.Replace(svntableStr, SvnChanges(svnlog, 100, "uppweb", true));
		else if (tt[i].title == "Svn Bazaar major releases") 
			tt[i].text.Replace(svntableStr, SvnChanges(svnlog, 100, "bazaar", true));
		else if (tt[i].title == "Svn Upp major releases") 
			tt[i].text.Replace(svntableStr, SvnChanges(svnlog, 100, "uppsrc", true));		
		else
		if(tt[i].title == "Nightly builds" || tt[i].title.Find("download") >= 0)
			tt[i].text.Replace(String("<#downloads#>"), downloads);
		else if (links[i].Find("index") >= 0) {
			String win32 = "upp-win32-RELEASE.exe";
			String win32release = win32;
			win32release.Replace(String("RELEASE"), release);
			String x11 = "upp-x11-src-RELEASE.tar.gz";
			String x11release = x11;
			x11release.Replace(String("RELEASE"), release);
			tt[i].text.Replace(DeQtf(x11), DeQtf(x11release));
			tt[i].text.Replace(DeQtf(win32), DeQtf(win32release));
		}
	}

	for(int i = 0; i < reflink.GetCount(); i++) {
		String l = reflink.GetKey(i);
		String lbl = Filter(l, CharFilterLbl);
		String f = links.Get(reflink[i], Null) + '#' + lbl;
		links.Add(l, f);
		static const char *x[] = { "::struct", "::class", "::union" };
		for(int ii = 0; ii < 3; ii++) {
			String e = x[ii];
			if(EndsWith(l, e)) {
				links.Add(l.Mid(0, l.GetLength() - e.GetLength()), f);
			}
		}
		labels.Add(l, lbl);
	}

	Date d = GetSysDate();
	lastUpdate = HtmlItalic() / HtmlArial(8) / HtmlFontColor(Gray()) /
	                   (String().Cat() << "Last update " << GetSysDate());

//	Vector<String> amazon = Split(LoadFile(GetRcFile("amazon.txt")), '\n');//440

	Vector <SvnBazaarItems> bazaarItems = SvnBazaarList(bazaar, svnlog);

	// To fill Bazaar page with real release dates and size
	for(int i = 0; i < tt.GetCount(); i++) {
		if (tt[i].title == "Bazaar") {
			String page = tt[i];
			for (int j = 0; j < bazaarItems.GetCount(); ++j) {
				int pos = page.Find("2 " + DeQtf(bazaarItems[j].name) + "]]");
				if (pos < 0)
					pos = page.Find("2 " + DeQtf(bazaarItems[j].name) + "\r\n]]");
				if (pos >= 0) {
					pos = page.Find("::", pos);	
					pos = page.Find("::", pos);
					pos = page.Find(":: [s0;=2", pos);
					if (pos >= 0)
						pos += (int)strlen(":: [s0;=2");
					String strDate;
					if (pos >= 0) {
						Time t = bazaarItems[j].lastChange;
						if (IsNull(t))
							strDate = "Not in Svn log";
						else 
							strDate = Format("%Mon", t.month) + " " + FormatInt(t.year);
						page.Insert(pos, " " + strDate);
					}
					pos = page.Find(":: [s0;=2", pos);
					if (pos >= 0)
						pos += (int)strlen(":: [s0;=2");
					String strSz;
					if (pos >= 0) {
						if (bazaarItems[j].size == 0)
							strSz = "Not in Svn log";
						else 
							strSz = BytesToString(bazaarItems[j].size);
		
						page.Insert(pos, " " + strSz);
					}
				}
			}
			tt[i].text = page;
		}
	}

	Vector <int> ttPdfId;
	for (int i = 0; i < ttFullIds.GetCount(); ++i) {
		Vector<int> ids;
		ids.Clear();
		String sid, fullTitle;
		sid.Clear();
		fullTitle.Clear();
		int level = 0;
		for (const char *idStr = ttFullIds[i].Begin() + 1; *idStr; idStr++) {
			if (*idStr != '/')
				sid.Cat(*idStr);
			else {
				int id = ScanInt(sid);
				if (IsNull(id))
					fullTitle << "/" << sid;
				else {
					if (id < 0 || id >= tt.GetCount()) {
						Cout() << "Problem in documentation";
						return;
					}
					fullTitle << "/" << tt[id].title;
					level++;
					ids.Add(id);
				}
				sid.Clear();
			}
		}
		int id = ScanInt(sid);
		if (IsNull(id)) 
			fullTitle << "/" << sid;
		else {
			if (id < 0 || id >= tt.GetCount()) {
				Cout() << "Problem in documentation";
				return;
			}
			fullTitle << "/" << tt[id].title;
			level++;
		}
		String com = "";
		for (int iid = 0; iid < ids.GetCount(); ++iid) {
			if (ids[iid] == id) {
				com = "END LINK";
				break;
			}
			if (tt[id].text == tt[ids[iid]].text) {
				com = "DUPLICATED";
				break;
			}
		}
		if (fullTitle.Find("/Documentation") >= 0 && level <= 2 && com != "DUPLICATED") {
			Cout() << i << "\t(" << level << ")\t" << com << "\n";
			Cout() << ttFullIds[i] << "\n";
			Cout() << fullTitle << "\n";
			ttPdfId << id;
		}
	}

	LLOG("G: " << MemoryUsedKb());

	if (outPdf) {
		Cout() << "\nCreating pdf (" << tt.GetCount() << " items)\n";
		PdfDraw pdf;
		//for(int i = 0; i < tt.GetCount(); i++) 
		for(int i = 0; i < ttPdfId.GetCount(); i++) 	
			QtfAsPdf(pdf, tt[ttPdfId[i]]);
		SaveFile(AppendFileName(pdfdir, "Upp.pdf"), pdf.Finish());
	}

	LLOG("H: " << MemoryUsedKb());

	if (outHtml) {
		Cout() << "\nCreating htmls\n";
		
		for(int i = 0; i < tt.GetCount(); i++)
			ExportPage(i);
		SetLanguage(currentLang);
	
		LLOG("I: " << MemoryUsedKb());
	
		FileCopy(AppendFileName(uppbox, "uppweb/favicon.png"), AppendFileName(targetdir, "favicon.png"));
		//	SaveFile(AppendFileName(targetdir, "favicon.ico"), LoadFile(AppendFileName(uppsrc, "ide/ide.ico")));
		SaveFile(AppendFileName(targetdir, "stats.html"),
		         HtmlLink("https://www.mygooglepagerank.com", "_blank") /
			         "<img src=\"https://www.mygooglepagerank.com/PRimage.php?url=http://upp.sf.net\" "
			         "border=\"0\" width=\"66\" height=\"13\" "
			         "alt=\"Google PageRank&trade; - Post your PR with MyGooglePageRank.com\">" +
		         "<noscript>" +
		         HtmlLink("https://www.mygooglepagerank.com").Title("My Google Page Rank") /
		           "My Google Page Rank" +
		         "</noscript>" +
		         HtmlLink("https://www.mygooglepagerank.com", "_blank") /
			         "<img src=\"http://www.mygooglepagerank.com/PRimage.php?url=http://www.ultimatepp.org\" "
			         "border=\"0\" width=\"66\" height=\"13\" "
			         "alt=\"Google PageRank&trade; - Post your PR with MyGooglePageRank.com\">" +
		         "<noscript>" +
		         HtmlLink("https://www.mygooglepagerank.com").Title("My Google Page Rank") /
		           "My Google Page Rank" +
		         "</noscript>"
		);
	}
	Cout() << "Finished OK\n";
}
