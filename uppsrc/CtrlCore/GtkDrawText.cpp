#include <CtrlCore/CtrlCore.h>

#ifdef GUI_GTK

#include <cairo/cairo-ft.h>

// #include <freetype2/freetype/ftoutln.h>

namespace Upp {

#define LLOG(x)

FcPattern *CreateFcPattern(Font font);
FT_Face    FTFace(Font fnt, String *rpath = NULL);

struct FontSysData {
	cairo_scaled_font_t *scaled_font;
	
	void Init(Font font, int angle);
	~FontSysData() { cairo_scaled_font_destroy(scaled_font); }
};

void FontSysData::Init(Font font, int angle)
{
	LLOG("FontSysData::Init " << font << ", " << angle);
	Std(font);

	FcPattern *p = CreateFcPattern(font);
	cairo_font_face_t *font_face = cairo_ft_font_face_create_for_pattern(p);
	FcPatternDestroy(p);
	
	cairo_matrix_t font_matrix[1], ctm[1];
	cairo_matrix_init_identity(ctm);
	cairo_matrix_init_scale(font_matrix, font.GetHeight(), font.GetHeight());

	if(font.IsItalic() && !(FTFace(font)->style_flags & FT_STYLE_FLAG_ITALIC)) {
		cairo_matrix_t sheer[1];
		cairo_matrix_init_identity(sheer);
		sheer->xy = -0.2;
		cairo_matrix_multiply(font_matrix, font_matrix, sheer);
	}
	
	if(angle)
		cairo_matrix_rotate(font_matrix, -angle * M_2PI / 3600);
	cairo_font_options_t *opt = cairo_font_options_create();
	scaled_font = cairo_scaled_font_create(font_face, font_matrix, ctm, opt);

	cairo_font_options_destroy(opt);
	cairo_font_face_destroy(font_face);
}

struct FontDataSysMaker : LRUCache<FontSysData, Tuple2<Font, int> >::Maker {
	Font font;
	int  angle;

	virtual Tuple2<Font, int>  Key() const        { return MakeTuple(font, angle); }
	virtual int Make(FontSysData& object) const   { object.Init(font, angle); return 1; }
};

int    gtk_antialias = -1; // These are not really used with GTK backed.....
int    gtk_hinting = -1;
String gtk_hintstyle;
String gtk_rgba;

void SystemDraw::FlushText()
{
	if(textcache.GetCount() == 0)
		return;
	Buffer<cairo_glyph_t> gs(textcache.GetCount());
	for(int i = 0; i < textcache.GetCount(); i++) {
		cairo_glyph_t& g = gs[i];
		g.index = textcache[i].index;
		g.x = textcache[i].x;
		g.y = textcache[i].y;
	}

	static LRUCache<FontSysData, Tuple2<Font, int> > cache;
	FontDataSysMaker m;
	m.font = textfont;
	m.angle = textangle;
	FontSysData& sf = cache.Get(m);
	cairo_set_scaled_font(cr, sf.scaled_font);

	SetColor(textink);
	cairo_show_glyphs(cr, gs, textcache.GetCount());
	
	cache.Shrink(INT_MAX, 128);

	textcache.Clear();
}

void SystemDraw::DrawTextOp(int x, int y, int angle, const wchar *text, Font font, Color ink, int n, const int *dx)
{
	GuiLock __;
	
	if(textcache.GetCount() && (font != textfont || ink != textink || angle != textangle))
		FlushText();
	
	textfont = font;
	textink = ink;
	textangle = angle;
	
	int ascent = font.GetAscent();
	double sina = 0;
	double cosa = 1;
	if(angle)
		Draw::SinCos(angle, sina, cosa);
	int xpos = 0;
	for(int i = 0; i < n; i++) {
		TextGlyph& g = textcache.Add();
		g.index = GetGlyphInfo(font, text[i]).glyphi;
		g.x = int(x + cosa * xpos + sina * ascent);
		g.y = int(y + cosa * ascent - sina * xpos);
		xpos += dx ? dx[i] : font[text[i]];
	}
}

}

#endif
