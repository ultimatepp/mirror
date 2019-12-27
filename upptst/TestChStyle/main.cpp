#include "TestChStyle.h"

void Dl(DropList& dl)
{
	dl.Add("Case1");
	dl.Add("Case2");
	dl <<= "Case1";
}

void Dc(WithDropChoice<EditString>& dc)
{
	dc.AddList("Case1");
	dc.AddList("Case2");
	dc <<= "Case1";
}


TestChStyle::TestChStyle()
{
	AddFrame(menu);
	menu.Sub("Something", [=](Bar& bar) { bar.Add("Foo", [] {}); });

	AddFrame(bar);
	bar.Set([](Bar& bar) { bar.Add(CtrlImg::Diskette(), [] {}).Tip("This is test"); });
	
	CtrlLayout(*this, "Window title");
	
	normal.NullText("Normal");
	readonly <<= "Read only";
	disabled <<= "Disabled";
	disabled.Disable();
	
	Dl(dl_normal);
	Dl(dl_readonly);
	Dl(dl_disabled);
	dl_disabled.Disable();

	Dc(dc_normal);
	Dc(dc_readonly);
	Dc(dc_disabled);
	dc_disabled.Disable();

#ifdef CPP_11
	standard << [] { Ctrl::SetSkin(ChStdSkin); };
	classic << [] { Ctrl::SetSkin(ChClassicSkin); };
	host << [] { Ctrl::SetSkin(ChHostSkin); };
#endif

	for(int i = 0; i < 100; i++)
		tab.Add("Tab " + AsString(i));
	
	
	list.HeaderObject().Absolute();
	list.AddColumn("Col", 50);
	list.AddColumn("Col", 50);
	list.AddColumn("Col", 50);
	list.AddColumn("Col", 50);
	list.AddColumn("Col", 50);
	for(int i = 0; i < 50; i++)
		list.Add(i);
}

GUI_APP_MAIN
{
	Ctrl::SetDarkThemeEnabled();
	
	TestChStyle().Run();
}
