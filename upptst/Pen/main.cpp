#include <CtrlLib/CtrlLib.h>

using namespace Upp;

struct MyApp : TopWindow {
	Point pos;
	
	Vector<Vector<Tuple<double, Pointf>>> drawing;

	PenInfo pen;

	virtual void Pen(Point p, const PenInfo& pn, dword keyflags) override {
		LOG("PEN");
		if(keyflags & K_SHIFT) {
			RectTracker tracker(*this);
			tracker.MinSize(Size(-100000,-100000));
			tracker.Track(Rect(p,p));
		}
		if(pn.pressure) {
			if((!!pn.pressure == !!pen.pressure) && drawing.GetCount())
				drawing.Top().Add(MakeTuple(pn.pressure, p));
			else
				drawing.Add().Add(MakeTuple(pn.pressure, p));
		}
		pen = pn;
		Refresh();
	}

	virtual void Paint(Draw& w0) override {
		LOG("PAINT");
		TIMING("PAINT");
		DrawPainter w(w0, GetSize());
		w.Co();
		w.Clear(SColorPaper());
		
		w.LineCap(LINECAP_ROUND);
		for(const auto& stroke : drawing)
			if(stroke.GetCount())
				for(int i = 0; i < stroke.GetCount() - 1; i++) {
					w.Move(stroke[i].b);
					w.Line(stroke[i + 1].b);
					w.Stroke(DPI(20) * stroke[i].a, Black());
				}
		
		int fcy = GetStdFontCy();
		int y = 10;
		auto Text = [&] (const String& text) {
			w.DrawText(10, y, text);
			y += fcy;
		};
		Text(AsString(pos));
		Text(String() << "Pressure: " << pen.pressure);
		Text(String() << "Rotation: " << pen.rotation);
		Text(String() << "Tilt: " << pen.tilt);
		Text(String() << "Barrel: " << pen.barrel);
		Text(String() << "Inverted: " << pen.inverted);
		Text(String() << "Eraser: " << pen.eraser);
	}
};

GUI_APP_MAIN
{
	StdLogSetup(LOG_FILE|LOG_ELAPSED);
	MyApp().Run();
}
