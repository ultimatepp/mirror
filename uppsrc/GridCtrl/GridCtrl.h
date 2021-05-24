#ifndef _GridCtrl_GridCtrl_h_
#define _GridCtrl_GridCtrl_h_
#include <CtrlLib/CtrlLib.h>
#ifdef flagGRIDSQL
#include <Sql/Sql.h>
#endif

namespace Upp {

#include "GridUtils.h"
#include "GridDisplay.h"

#define FOREACH_ROW(x) for(x.Begin(); x.IsEnd(); x.Next())
#define FOREACH_SELECTED_ROW(x) FOREACH_ROW(x) if(x.IsSelected())
#define FOREACH_MODIFIED_ROW(x) FOREACH_ROW(x) if(x.IsUpdatedRow())
#define FOREACH_ROW_NOT_CURRENT(x) FOREACH_ROW(x) if(!x.IsCurrentRow())

namespace GF
{
	enum {
		SKIP_CURRENT_ROW = BIT(0),
		SKIP_HIDDEN = BIT(1)
	};
};

class GridFind : public EditString
{
	private:
		MultiButtonFrame button;
	public:

		GridFind();

		Event<> WhenEnter;
		Callback1<Bar &> WhenBar;

		virtual bool Key(dword key, int count);
		virtual Size GetMinSize() const;
		void Push();

		typedef GridFind CLASSNAME;

};

class GridPopUpHeader : public Ctrl
{
	private:
		bool open;
	public:

		GridDisplay *display;
		Value val;
		int sortmode;
		int sortcol;
		int sortcnt;
		bool chameleon;

		GridPopUpHeader() : open(false) {}

		virtual void Paint(Draw &w);
		void PopUp(Ctrl *owner, int x, int y, int width, int height);
		virtual void Close();
};

class GridButton : public Ctrl
{
	private:
		int img;
		int n;
		
	public:
		typedef GridButton CLASSNAME;
		GridButton();
		virtual void Paint(Draw& w);
		virtual void LeftDown(Point p, dword flags);
		virtual void LeftUp(Point p, dword flags);
		virtual void MouseEnter(Point p, dword flags);
		virtual void MouseLeave();
		virtual void State(int reason);
		virtual Size GetStdSize() const;
		void SetButton(int n);
};

class GridResizePanel : public FrameBottom<Ctrl>
{
	private:
		GridButton close;
		Point pos;
		Rect r;
		Size minsize;
	public:

		typedef GridResizePanel CLASSNAME;
		GridResizePanel();
		virtual void Paint(Draw& w);
		virtual Image CursorImage(Point p, dword flags);
		virtual void LeftDown(Point p, dword flags);
		virtual void LeftUp(Point p, dword flags);
		virtual void MouseMove(Point p, dword flags);
		bool MouseOnGrip(const Point &p);
		void SetMinSize(Size sz);

		Event<> WhenClose;
};

class GridPopUp : public Ctrl
{
	public:

		bool open;
		Value val;
		Ctrl* ctrl;
		GridDisplay* gd;
		Color fg, bg;
		Font fnt;
		dword style;

		GridPopUp() : open(false) {}
		Point Offset(Point p);

		virtual void  Paint(Draw &w);
		virtual void  LeftDown(Point p, dword flags);
		virtual void  LeftDrag(Point p, dword flags);
		virtual void  LeftDouble(Point p, dword flags);
		virtual void  RightDown(Point p, dword flags);
		virtual void  LeftUp(Point p, dword flags);
		virtual void  MouseWheel(Point p, int zdelta, dword flags);
		virtual void  MouseLeave();
		virtual void  MouseEnter(Point p, dword flags);
		virtual void  MouseMove(Point p, dword flags);
	    virtual Image CursorImage(Point p, dword flags);
	    virtual void  LostFocus();
		void PopUp(Ctrl *owner, int x, int y, int width, int height);
		virtual void Close();
};

class GridOperation
{
	public:

		enum
		{
			INSERT,
			UPDATE,
			REMOVE,
			NONE
		};

	private:

		int operation;
		int version;

	public:

		GridOperation() : operation(NONE), version(1) {}

		void SetOperation(int op)
		{
			switch(operation)
			{
				case NONE: operation = op; ++version; break;
				case UPDATE:
					if(op == REMOVE || op == NONE)
						Clear();
					break;
				case INSERT:
					if(op == REMOVE || op == NONE)
						Clear();
					break;
			}
		}

		void Clear()        { operation = NONE; }
		void ClearVersion() { version = 0;      }
		int  GetVersion()   { return version;   }

		GridOperation& operator=(const int op)
		{
			SetOperation(op);
			return *this;
		}

		operator int()          { return operation;       }
		bool operator!=(int op) { return operation != op; }
		bool operator==(int op) { return operation == op; }
};


class CtrlsHolder : public Ctrl
{
	private:
		Ctrl &parent;
		Point offset;
	public:
		CtrlsHolder(Ctrl &p) : parent(p)
		{
			Transparent();
			WantFocus(false);
		}
		virtual void  LeftUp(Point p, dword flags)      { parent.LeftUp(p + offset, flags);      }
		virtual void  LeftDown(Point p, dword flags)    { parent.LeftDown(p + offset, flags);    }
		virtual void  LeftDouble(Point p, dword flags)  { parent.LeftDouble(p + offset, flags);  }
		virtual void  RightUp(Point p, dword flags)     { parent.RightUp(p + offset, flags);     }
		virtual void  RightDown(Point p, dword flags)   { parent.RightDown(p + offset, flags);   }
		virtual void  RightDouble(Point p, dword flags) { parent.RightDouble(p + offset, flags); }
		virtual void  MouseMove(Point p, dword flags)   { parent.MouseMove(p + offset, flags);   }
		virtual void  MouseLeave()                      { parent.MouseLeave();                   }
	    virtual Image CursorImage(Point p, dword flags) { return parent.CursorImage(p + offset, flags); }

		virtual void ChildMouseEvent(Ctrl *child, int event, Point p, int zdelta, dword keyflags)
		{
			Ctrl::ChildMouseEvent(child, event, p, zdelta, keyflags);
		}
	    virtual void  Paint(Draw &w)
		{
			//DrawFrame(w, GetSize(), Red);
		}

		virtual void  MouseWheel(Point p, int zdelta, dword keyflags)
		{
			parent.MouseWheel(p + offset, zdelta, keyflags);
		}

		void SetOffset(Point p)
		{
			offset = p;
		}
};

class GridClipboard : Moveable<GridClipboard>
{
	public:
		rval_default(GridClipboard);
		GridClipboard() {}

		struct ClipboardData : Moveable<ClipboardData>
		{
			int col, row;
			Value v;
			
			void SerializeAttrText(Stream& s, AttrText& a)
			{
				s % a.text;
				s % a.font;
				s % a.ink;
				s % a.normalink;
				s % a.paper;
				s % a.align;
				s % a.img;
				s % a.imgspc;
			}
			
			void Serialize(Stream &s)
			{
				s % col % row;
				dword type = IsType<AttrText>(v) ? 1 : 0;
				s / type;
				if(type == 1)
				{
					if(s.IsStoring())
					{
						SerializeAttrText(s, (AttrText&) ValueTo<AttrText>(v));
					}
					else
					{
						AttrText a;
						SerializeAttrText(s, a);
						v = RawToValue<AttrText>(a);
					}
				}
				else
					s % v;
			}
		};

		Vector<ClipboardData> data;
		Point minpos, maxpos;
		bool shiftmode;

		virtual void Serialize(Stream &s)
		{
			s % shiftmode;
			s % minpos % maxpos;
			s % data;
		}
};

class GridCtrl : public Ctrl
{
	friend class DropGrid;

	public:

		int GD_COL_WIDTH;
		int GD_ROW_HEIGHT;
		int GD_HDR_HEIGHT;
		int GD_IND_WIDTH;

		enum GridOrder
		{
			SORT_ID   = 0,
			SORT_UP   = 1,
			SORT_DOWN = 2
		};

	private:

		enum GridCursor
		{
			GO_PREV,
			GO_NEXT,
			GO_LEFT,
			GO_RIGHT,
			GO_BEGIN,
			GO_END,
			GO_PAGEUP,
			GO_PAGEDN
		};

		enum GridControlState
		{
			UC_SHOW      = BIT(0),
			UC_HIDE      = BIT(1),
			UC_FOCUS     = BIT(2),
			UC_NOFOCUS   = BIT(3),
			UC_GOFIRST   = BIT(4),
			UC_SCROLL    = BIT(5),
			UC_NEXT      = BIT(6),
			UC_CTRLS     = BIT(7),
			UC_CURSOR    = BIT(8),
			UC_CHECK_VIS = BIT(9),
			UC_MOUSE     = BIT(10),
			UC_CTRLS_OFF = BIT(11),
			UC_UPDATES   = BIT(12),
			UC_OLDCUR    = BIT(13)
		};
		
		enum GridRepaintOpts
		{
			RP_TOOLBAR   = BIT(0),
			RP_UPDCTRLS  = BIT(1),
			RP_ALL       = BIT_ALL
		};
		
		enum GridCursorState
		{
			CU_MOUSE     = BIT(0),
			CU_HIGHLIGHT = BIT(1),
			CU_CTRLMODE  = BIT(2),
			CU_HIDECTRLS = BIT(3)
		};
		
		enum GridSummaryOperation
		{
			SOP_NONE = 0,
			SOP_MIN = 1,
			SOP_MAX = 2,
			SOP_SUM = 3,
			SOP_CNT = 4,
			SOP_AVG = 5,
		};

		enum GridState
		{
			GS_UP     = 0,
			GS_MOVE   = 1,
			GS_DOWN   = 2,
			GS_BORDER = 3,
			GS_POPUP  = 4
		};

		enum GridEdit
		{
			GE_ROW  = 0,
			GE_CELL = 1
		};
		
		enum GridItemCtrl
		{
			IC_INIT = BIT(1),
			IC_MANUAL = BIT(2),
			IC_FACTORY = BIT(3),
			IC_OWNED = BIT(4)
		};

		struct CurState
		{
			bool newx;
			bool newy;
			bool accepted;
			bool valid;

			CurState() { Clear(); }
			bool IsNewRow()   { return newy;         }
			bool IsNewCol()   { return newx;         }
			bool IsNew()      { return newx || newy; }
			bool IsAccepted() { return accepted;     }
			bool IsValid()    { return valid;        }
			
			void Clear()
			{
				newx = newy = false;
				accepted = false;
				valid = true;
			}

			operator bool() { return IsNew() && IsAccepted(); }
		};

		struct Edit : Moveable<Edit>
		{
			Ptr<Ctrl> ctrl;
			Convert * convert;
			Callback1<One<Ctrl>&> factory;

			Edit()
			{
				ctrl = NULL;
				convert = NULL;
			}
		};

		class Item : Moveable<Item>
		{
			public:
				friend class ItemRect;
				friend class GridCtrl;
	
				Item()
				{
					ctrl    = NULL;
					convert = NULL;
					display = NULL;
	
					fs = fe = 0;
					style = 0;
					editable = true;
					clickable = true;
					idx = 0;
					idy = 0;
					cx = 0;
					cy = 0;
					group = -1;
					isjoined = false;
					
					rcx = 0;
					rcy = 0;
	
					modified = false;
					sync_flag = -1;
					paint_flag = -1;
					ctrl_flag = 0;
				}
				~Item()
				{
					if(ctrl_flag & IC_OWNED)
						delete ctrl;
				}
				
				void SetCtrl(Ctrl& ctrl, bool owned);
				void ClearCtrl();
				
				void SetDisplay(GridDisplay& display);
				void NoDisplay();

			private:

				bool InvertSelect() { return BitInverse(style, GD::SELECT); }
				bool IsSelect() { return style & GD::SELECT; }
				bool IsLive()   { return style & GD::LIVE;   }
				bool IsCursor() { return style & GD::CURSOR; }
				bool IsFound()  { return style & GD::FOUND;  }

				void Select(bool s) { BitSet(style, GD::SELECT, s); }
				void Cursor(bool s) { BitSet(style, GD::CURSOR, s); }
				void Live(bool s)   { BitSet(style, GD::LIVE, s);   }
				void Found(bool s)  { BitSet(style, GD::FOUND, s);  }

				bool IsJoined() { return isjoined; }

				int idx, idy, cx, cy, group;
				int rcx, rcy;
				bool isjoined:1;
				int fs, fe;
				dword style;
				bool editable:1;
				bool clickable:1;
				bool modified:1;
				int sync_flag;
				int paint_flag;
				int ctrl_flag;

				Ptr<Ctrl> ctrl;

				Convert     *convert;
				GridDisplay *display;

			public:
				Value val;

				Item& Editable(bool b);
				Item& NoEditable();

				Item& Clickable(bool b)  { clickable = b;     return *this; }
				Item& NoClickable()      { clickable = false; return *this; }
		};

		typedef Vector< Vector<Item> > Items;
		typedef Vector< Edit > Edits;

		struct JoinRect : Moveable<JoinRect>
		{
			Rect r;
			int group;
			int idx, idy;
		};

		public: class ItemRect : public Moveable<ItemRect>
		{
			friend class GridCtrl;

			public:
				ItemRect()
				{
					min = 5; max = 1000000;
					pos = npos = 0;
					id = uid = n = 0;
					prop = 1;
					size = nsize = 0;
					tsize = 0;
					sortmode = 0;
					hidden = false;
					index = false;
					convertion = true;
					editable = true;
					edit_insert = true;
					edit_update = true;
					sortable = true;
					clickable = true;
					locked = false;
					skip = false;
					ignore_display = false;
					style = 0;

					calign = 0;
					sl = sr = st = sb = 0;
					sx = sy = 0;
					join = 0;

					ismin = false;
					ismax = false;

					fnt = Null;
					hfnt = Null;
					balign = halign = GD::LEFT | GD::VCENTER;
					wrap = false;
					fg = Null;
					bg = Null;

					ctrl    = NULL;
					convert = NULL;
					display = NULL;

					found = false;

					level = 0;
					sop = SOP_NONE;

					operation = GridOperation::NONE;
				}

			private:

				Items *items;
				GridCtrl *parent;
				Edits *edits;

				int level;

				Ptr<Ctrl>   ctrl;
				Convert     *convert;
				GridDisplay *display;

				Callback1<One<Ctrl>&> factory;
				
				static VectorMap<Id, int> *aliases;

				double pos, size, prop;
				int npos, nsize;
				int min, max;
				int id, uid, n;
				int sl, sr, st, sb, sx, sy;

				double tsize;
				int  join;
				
				int sop;
				String sopfrm;

				bool hidden;
				bool index;
				bool convertion;
				bool editable;
				bool sortable;
				bool clickable;
				bool locked;
				bool found:1;
				bool skip:1;
				bool edit_insert:1;
				bool edit_update:1;

				Value defval;

				dword style;

				bool ismin;
				bool ismax;

				/* operatory typu friend moga odwolywac sie tylko do zmiennych statycznych klasy */

				static int sortCol;
				static int sortMode;

				int sortcol;
				int sortmode;

				int balign, halign, calign;
				bool wrap;
				Font fnt, hfnt;
				Color fg, bg;
				Image img;

				GridOperation operation;

				bool ignore_display:1;

				double Left(int scroll = 0)    { return pos - scroll;          }
				double Right(int scroll = 0)   { return pos + size - scroll;   }
				double Top(int scroll = 0)     { return pos - scroll;          }
				double Bottom(int scroll = 0)  { return pos + size - scroll;   }
				double Width()                 { return size;                  }
				double Height()                { return size;				   }

				bool   InvertSelect()          { return BitInverse(style, GD::SELECT); }
				bool   IsSelect()              { return style & GD::SELECT;    }
				bool   IsLive()                { return style & GD::LIVE;      }
				bool   IsCursor()              { return style & GD::CURSOR;    }
				bool   IsHighlight()           { return style & GD::HIGHLIGHT; }

				bool   IsFound()               { return found;                 }

				void   Select(bool s)          { BitSet(style, GD::SELECT, s);    }
				void   Cursor(bool s)          { BitSet(style, GD::CURSOR, s);    }
				void   Live(bool s)            { BitSet(style, GD::LIVE, s);      }
				void   Highlight(bool s)       { BitSet(style, GD::HIGHLIGHT, s); }

				int    nLeft(int scroll = 0)   { return npos - scroll;         }
				int    nRight(int scroll = 0)  { return npos + nsize - scroll; }
				int    nTop(int scroll = 0)    { return npos - scroll;         }
				int    nBottom(int scroll = 0) { return npos + nsize - scroll; }
				int    nWidth()                { return nsize;                 }
				int    nHeight()               { return nsize;				   }

				ItemRect& Size(int n, bool hv = false);

				bool IsMin()   { return ismin; }
				bool IsMax()   { return ismax; }
				bool IsFixed() { return ismin && ismax; }

				void ChangeSortMode(bool idsort = true);
				
				friend bool operator<(const ItemRect& a, const ItemRect& b)
				{
					if(sortMode)
						return (StdValueCompare(a.ExtractValue(a.id, sortCol), b.ExtractValue(b.id, sortCol), 0) < 0);
					else
						return a.id < b.id;
				}
				friend bool operator>(const ItemRect& a, const ItemRect& b)
				{
					if(sortMode)
						return (StdValueCompare(a.ExtractValue(a.id, sortCol), b.ExtractValue(b.id, sortCol), 0) > 0);
					else
						return a.id > b.id;
				}

				Callback1<int> SyncCtrls;

			public:

				ItemRect& Alias(Id id);
				ItemRect& Alias(const char * s);
				ItemRect& Name(String &s);
				ItemRect& Name(const char * s);
				ItemRect& Hidden(bool b = true);
				ItemRect& Width(int n);
				ItemRect& Height(int n);
				ItemRect& Min(int n);
				ItemRect& Max(int n);
				ItemRect& Fixed(int n);
				ItemRect& FixedAuto();
				ItemRect& Edit(Ctrl &ctrl, bool b = true);
				template<typename T>
				ItemRect& EditConvert(T &ctrl, bool b = true)    { return Edit(ctrl, b).SetConvert(ctrl); }
				template<typename T>
				ItemRect& EditConvertDisplay(T &ctrl)            { return Edit(ctrl).SetConvert(ctrl).SetDisplay(ctrl); }
				ItemRect& EditInsert(bool b = true);
				ItemRect& EditUpdate(bool b = true);
				ItemRect& SetConvert(Convert &c);
				ItemRect& NoConvert();
				ItemRect& SetFormat(const char *fmt);
				ItemRect& SetDisplay(GridDisplay &gd);
				ItemRect& IgnoreDisplay();
				ItemRect& NoConvertion();
				ItemRect& Default(Value v);
				ItemRect& Index(bool b = true);

				String GetName() const;
				Id     GetAlias() const;

				ItemRect& AlignTop()                             { balign = GD::TOP     | GD::LEFT;                return *this; }
				ItemRect& AlignBottom()                          { balign = GD::BOTTOM  | GD::LEFT;                return *this; }
				ItemRect& AlignLeft()                            { balign = GD::LEFT    | GD::VCENTER;             return *this; }
				ItemRect& AlignRight()                           { balign = GD::RIGHT   | GD::VCENTER;             return *this; }
				ItemRect& AlignTopLeft()                         { balign = GD::LEFT    | GD::TOP;                 return *this; }
				ItemRect& AlignTopRight()                        { balign = GD::RIGHT   | GD::TOP;                 return *this; }
				ItemRect& AlignTopCenter()                       { balign = GD::HCENTER | GD::TOP;                 return *this; }
				ItemRect& AlignCenterLeft()                      { balign = GD::VCENTER | GD::LEFT;                return *this; }
				ItemRect& AlignCenterRight()                     { balign = GD::VCENTER | GD::RIGHT;               return *this; }
				ItemRect& AlignCenter()                          { balign = GD::HCENTER | GD::VCENTER;             return *this; }
				ItemRect& AlignBottomLeft()                      { balign = GD::LEFT    | GD::BOTTOM;              return *this; }
				ItemRect& AlignBottomRight()                     { balign = GD::RIGHT   | GD::BOTTOM;              return *this; }
				ItemRect& AlignBottomCenter()                    { balign = GD::HCENTER | GD::BOTTOM;              return *this; }

				ItemRect& HeaderAlignTop()                       { halign = GD::TOP     | GD::LEFT;                return *this; }
				ItemRect& HeaderAlignBottom()                    { halign = GD::BOTTOM  | GD::LEFT;                return *this; }
				ItemRect& HeaderAlignLeft()                      { halign = GD::LEFT    | GD::VCENTER;             return *this; }
				ItemRect& HeaderAlignRight()                     { halign = GD::RIGHT   | GD::VCENTER;             return *this; }
				ItemRect& HeaderAlignTopLeft()                   { halign = GD::LEFT    | GD::TOP;                 return *this; }
				ItemRect& HeaderAlignTopRight()                  { halign = GD::RIGHT   | GD::TOP;                 return *this; }
				ItemRect& HeaderAlignTopCenter()                 { halign = GD::HCENTER | GD::TOP;                 return *this; }
				ItemRect& HeaderAlignCenterLeft()                { halign = GD::VCENTER | GD::LEFT;                return *this; }
				ItemRect& HeaderAlignCenterRight()               { halign = GD::VCENTER | GD::RIGHT;               return *this; }
				ItemRect& HeaderAlignCenter()                    { halign = GD::HCENTER | GD::VCENTER;             return *this; }
				ItemRect& HeaderAlignBottomLeft()                { halign = GD::LEFT    | GD::BOTTOM;              return *this; }
				ItemRect& HeaderAlignBottomRight()               { halign = GD::RIGHT   | GD::BOTTOM;              return *this; }
				ItemRect& HeaderAlignBottomCenter()              { halign = GD::HCENTER | GD::BOTTOM;              return *this; }

				ItemRect& CtrlAlignTop(int t = 0, int s = 0)     { calign |= GD::TOP;     st = t; sy = s;         return *this; }
				ItemRect& CtrlAlignBottom(int b = 0, int s = 0)  { calign |= GD::BOTTOM;  sb = b; sy = s;         return *this; }
				ItemRect& CtrlAlignLeft(int l = 0, int s = 0)    { calign |= GD::LEFT;    sl = l; sx = s;         return *this; }
				ItemRect& CtrlAlignRight(int r = 0, int s = 0)   { calign |= GD::RIGHT;   sr = r; sx = s;         return *this; }
				ItemRect& CtrlAlignHorzCenter(int s = 0)         { calign |= GD::HCENTER; sx = s;                 return *this; }
				ItemRect& CtrlAlignVertCenter(int s = 0)         { calign |= GD::VCENTER; sy = s;                 return *this; }
				ItemRect& CtrlAlignCenter(int s = 0, int d = 0)  { calign |= GD::VCENTER | GD::HCENTER; sx = s; sy = d; return *this; }
				ItemRect& CtrlAlignHorzPos(int l = 0, int r = 0) { calign |= GD::HPOS;    sl = l; sr = r; sx = 1; return *this; }
				ItemRect& CtrlAlignVertPos(int t = 0, int b = 0) { calign |= GD::VPOS;    st = t; sb = b; sy = 1; return *this; }

				int       GetAlign() const                       { return balign; }
				int       GetHeaderAlign() const                 { return halign; }
				int       GetCtrlAlign() const                   { return calign; }

				ItemRect& WrapText()                             { wrap = true; return *this; }

				ItemRect& SetFont(Font& f)                       { fnt = f;    return *this; }
				ItemRect& SetHeaderFont(Font& f)                 { hfnt = f;   return *this; }
				ItemRect& Bg(Color c)                            { bg = c;     return *this; }
				ItemRect& Fg(Color c)                            { fg = c;     return *this; }
				ItemRect& SetImage(const Image& i)               { img = i;    return *this; }
				ItemRect& ClearImage()                           { img = Null; return *this; }
				ItemRect& Ctrls(Callback1<One<Ctrl>&> _factory);
				ItemRect& Ctrls(void (*factory)(One<Ctrl>&))     { return Ctrls(callback(factory)); }
				template<class T> ItemRect&  Ctrls()             { return Ctrls(DefaultCtrlFactory<T>()); }
				ItemRect& NoCtrls();
				ItemRect& Option();
				ItemRect& ThreeStateOption();

				ItemRect& Editable(bool b);
				ItemRect& NoEditable();

				ItemRect& Sortable(bool b)                       { sortable = b;      return *this; }
				ItemRect& NoSortable()                           { sortable = false;  return *this; }

				ItemRect& Clickable(bool b)                      { clickable = b;     return *this; }
				ItemRect& NoClickable()                          { clickable = false; return *this; }
				
				ItemRect& Locked(bool b)                         { locked = b;        return *this; }
				ItemRect& NoLocked(bool b)                       { locked = false;    return *this; }

				ItemRect& Skip(bool b)                           { skip = b;          return *this; }
				
				ItemRect& DoAvg(const char *s = "")              { sop = SOP_AVG; sopfrm = s; return *this; }
				ItemRect& DoSum(const char *s = "")              { sop = SOP_SUM; sopfrm = s; return *this; }
				ItemRect& DoMin(const char *s = "")              { sop = SOP_MIN; sopfrm = s; return *this; }
				ItemRect& DoMax(const char *s = "")              { sop = SOP_MAX; sopfrm = s; return *this; }
				ItemRect& DoCount(const char *s = "")            { sop = SOP_CNT; sopfrm = s; return *this; }

				int  GetId()                                     { return id;                       }
				int  GetNumber()                                 { return id - parent->fixed_cols;  }

				bool IsConvertion() const                        { return convertion; }
				bool IsHidden() const                            { return hidden;     }

				bool IsSortOrg() const                           { return sortmode == 0; }
				bool IsSortAsc() const                           { return sortmode == 1; }
				bool IsSortDsc() const                           { return sortmode == 2; }

				Value ExtractValue(int r, int c) const;

				void Serialize(Stream &s);
				void Clear();
		};

		typedef Vector<ItemRect> RectItems;
		typedef Vector<ItemRect> VItems;
		typedef Vector<ItemRect> HItems;
		typedef Vector<JoinRect> JItems;

		typedef GridCtrl CLASSNAME;

		Items  items;
		HItems hitems;
		VItems vitems;
		JItems joins;
		
		Vector<Item> summary;
		Vector<Value> rowbkp;

		VectorMap<Id, int> aliases;
		Edits edits;

		CtrlsHolder holder;

		ScrollBar      sbx;
		ScrollBar      sby;
		ToolBar        bar;

		GridResizePanel resize_panel;
		GridPopUp popup;

		bool resize_panel_open:1;

		GridFind find;
		Label info;
		Button close;

		GridDisplay *display;
		GridDisplay *orgdisp;

		/* Properties */

		bool select_row:1;
		bool multi_select:1;
		bool indicator:1;
		bool resizing_cols:1;
		bool resizing_rows:1;
		bool moving_cols:1;
		bool moving_rows:1;
		bool resizing_fixed_cols:1;
		bool resizing_fixed_rows:1;
		bool dragging:1;
		bool horz_grid:1;
		bool vert_grid:1;
		bool draw_last_horz_line:1;
		bool draw_last_vert_line:1;
		bool sorting:1;
		bool sorting_multicol:1;
		bool header:1;
		bool live_cursor:1;
		bool row_changing:1;
		int  coloring_mode;

		int  resize_paint_mode;
		int  resize_col_mode;
		int  resize_row_mode;
		int  edit_mode;
		bool one_click_edit:1;
		bool goto_first_edit:1;

		bool inserting:1;
		bool appending:1;
		bool duplicating:1;
		bool removing:1;
		bool accepting:1;
		bool canceling:1;
		bool moving:1;
		bool navigating:1;
		bool searching:1;
		bool closing:1;
		bool editing:1;
		bool edits_in_new_row:1;
		bool hiding:1;
		bool clipboard:1;
		bool extra_paste:1;
		bool fixed_paste:1;
		bool copy_allowed:1;
		bool cut_allowed:1;
		bool paste_allowed:1;
		bool copy_column_names:1;
		bool draw_focus:1;
		bool cancel_all:1;
		bool ask_remove:1;

		bool reject_null_row:1;
		bool tab_changes_row:1;
		bool tab_adds_row:1;
		bool enter_like_tab:1;
		bool keep_last_row:1;
		bool remove_hides:1;

		bool full_col_resizing:1;
		bool full_row_resizing:1;

		bool chameleon:1;
		bool summary_row:1;
		bool update_summary:1;
		bool popups:1;
		bool focus_lost_accepting:1;

		bool search_hide:1;
		bool search_highlight:1;
		bool search_highlight_first:1;
		bool search_immediate:1;
		bool search_case:1;
		bool search_move_cursor:1;
		bool search_display:1;
		int  find_offset;
		
		static bool index_as_column;
		static bool reverse_sort_icon;

		/* States */

		bool newrow_inserted:1;
		bool newrow_appended:1;
		int  row_modified;
		bool row_removed:1;
		bool sel_begin:1;
		bool sel_end:1;
		bool moving_header:1;
		bool moving_body:1;
		bool moving_allowed:1;
		bool row_order:1;
		bool row_data:1;
		bool scrollLeftRight:1;
		bool doscroll:1;
		bool ready:1;
		bool isedit:1;
		int  genr_ctrls;
		bool edit_ctrls:1;
		bool recalc_cols;
		bool recalc_rows;
		bool cancel_update_cell:1;
		bool cancel_update:1;
		bool cancel_insert:1;
		bool cancel_remove:1;
		bool cancel_accept:1;
		bool cancel_duplicate:1;
		bool cancel_cursor:1;
		bool cancel_move:1;
		bool mouse_move:1;
		bool is_clipboard:1;
		bool enabled:1;

		bool call_whenchangecol:1;
		bool call_whenchangerow:1;
		bool call_whenremoverow:1;
		bool call_whenupdaterow:1;
		bool call_wheninsertrow:1;

		bool resizing;
		bool fixed_click;
		bool fixed_top_click;
		bool fixed_left_click;
		bool size_changed;
		bool top_click;
		bool just_clicked;
		bool synced;

		bool valid_cursor;


		/* Values */

		int  curSplitCol, curSplitRow;
		int  firstRow,    lastRow;
		int  firstCol,    lastCol;
		int  firstVisCol, lastVisCol;
		int  firstVisRow, lastVisRow;
		int  moveCol,     oldMoveCol;
		int  moveRow,     oldMoveRow;
		int  total_cols,  total_rows;
		int  fixed_cols,  fixed_rows;
		int  total_width, total_height;
		int  fixed_width, fixed_height;
		int  summary_height;
		
		int  oldSplitCol, oldSplitRow;

		int  selected_rows;
		int  selected_items;
		int  colidx;
		int  rowidx;
		int  bains;
		int  coluid;
		int  rowuid;
		int  rowfnd;

		int  bkp_rowidx;

		int  minRowSelected;
		int  maxRowSelected;

		int  sc_fr;
		int  sc_lr;

		int  join_group;

		int  sync_flag;
		int  paint_flag;		

		/* Points */

		Point curpos; // cursor position
		Point anchor; // selection anchor

		Point oldpos;
		Point oldcur;
		Point curid;
		Point livecur;
		Point ctrlpos;
		Point ctrlid;
		Point leftpnt;
		Size  osz;

		/* Others */

		Ctrl * focused_ctrl;
		int    focused_ctrl_id;
		Value  focused_ctrl_val;
		int    focused_col;

		WString search_string;

		bool curResizeCol;
		bool curResizeRow;

		bool resizeCol;
		bool resizeRow;

		int splitCol, splitRow;
		int firstColPos, firstRowPos;
		int hcol, hrow;
		int dx, dy;

		GridPopUpHeader pophdr;
		FrameRight<StaticRect> scrollbox;
		
		int sortCol;		
		Vector<int> sortOrder;

	public:
	
		struct SortOrder : Moveable<SortOrder>
		{
			int id;
			Id name;
			bool ascending;
			bool descending;
		};

	public:

		/* Colors */

		Color fg_focus,  bg_focus;
		Color fg_select, bg_select;
		Color fg_live,   bg_live;
		Color fg_found,  bg_found;
		Color fg_even,   bg_even;
		Color fg_odd,    bg_odd;
		Color fg_grid;


	public:
		GridCtrl();
		~GridCtrl();

		/* Properties */
		GridCtrl& HorzGrid(bool b = true) 	      { horz_grid           = b; return *this; }
		GridCtrl& VertGrid(bool b = true) 	      { vert_grid           = b; return *this; }
		GridCtrl& DrawLastHorzLine(bool b = true) { draw_last_horz_line = b; return *this; }
		GridCtrl& DrawLastVertLine(bool b = true) { draw_last_vert_line = b; return *this; }
		GridCtrl& MultiSelect(bool b = true)      { multi_select        = b; return *this; }
		GridCtrl& ResizingCols(bool b = true) 	  { resizing_cols       = b; return *this; }
		GridCtrl& ResizingRows(bool b = true) 	  { resizing_rows       = b; return *this; }
		GridCtrl& MovingCols(bool b = true)   	  { moving_cols         = b; return *this; }
		GridCtrl& MovingRows(bool b = true)   	  { moving_rows         = b; return *this; }
		GridCtrl& Dragging(bool b = true)         { dragging            = b; return *this; }
		GridCtrl& ResizePaintMode(int m = 0) 	  { resize_paint_mode   = m; return *this; }
		GridCtrl& LiveCursor(bool b = true)   	  { live_cursor         = b; return *this; }
		GridCtrl& Sorting(bool b = true)      	  { sorting             = b; return *this; }
		GridCtrl& MultiSorting(bool b = true) 	  { sorting_multicol    = b; return *this; }
		GridCtrl& Header(bool b = true)           { header              = b; return *this; }
		GridCtrl& EditMode(int m)      			  { edit_mode           = m; return *this; }
		GridCtrl& EditRow()         			  { edit_mode           = GE_ROW;  return *this; }
		GridCtrl& EditCell()         			  { edit_mode           = GE_CELL; return *this; }
		GridCtrl& OneClickEdit(bool b = true)     { one_click_edit      = b; return *this; }
		GridCtrl& GotoFirstEdit(bool b = true)    { goto_first_edit     = b; return *this; }
		GridCtrl& Absolute()					  { return ResizeColMode(0);               }
		GridCtrl& Proportional()				  { return ResizeColMode(1);               }
		GridCtrl& SelectRow(bool b = true)        { select_row          = b; return *this; }
		GridCtrl& AutoHideHorzSb(bool b = true)   { sbx.AutoHide(b);         return *this; }
		GridCtrl& AutoHideVertSb(bool b = true)   { sby.AutoHide(b);         return *this; }
		GridCtrl& AutoHideSb(bool b = true)       { sbx.AutoHide(b);
		                                            sby.AutoHide(b);         return *this; }

		GridCtrl& ResizeColMode(int m = 0);
		GridCtrl& ResizeRowMode(int m = 0);

		GridCtrl& Indicator(bool b = true, int size = -1);

		GridCtrl& GridColor(Color fg = SColorShadow);
		GridCtrl& FocusColor(Color fg = SColorHighlightText, Color bg = SColorHighlight);
		GridCtrl& LiveColor(Color fg = SColorText, Color bg = SColorHighlight);
		GridCtrl& OddColor(Color fg = SColorText, Color bg = SColorInfo);
		GridCtrl& EvenColor(Color fg = SColorText, Color bg = Blend(SColorHighlight, SColorPaper, 220));
		GridCtrl& ColoringMode(int m);

		GridCtrl& ColorRows(bool b = true) { return ColoringMode(2 * b).EvenColor(); }
		GridCtrl& ColorCols(bool b = true) { return ColoringMode(1 * b).EvenColor(); }


		GridCtrl& SetDefaultRowHeight(int h)    { GD_ROW_HEIGHT = h; sby.SetLine(h); return *this; }
		GridCtrl& SetColWidth(int n, int width, bool recalc = true);
		GridCtrl& SetRowHeight(int n, int height, bool recalc = true);
		GridCtrl& SetColsMin(int size);
		GridCtrl& SetColsMax(int size);

		int GetDefaultRowHeight() { return GD_ROW_HEIGHT; }

		GridCtrl& Inserting(bool b = true)          { inserting            = b;  return *this; }
		GridCtrl& Appending(bool b = true)          { appending            = b;  return *this; }
		GridCtrl& Duplicating(bool b = true)        { duplicating          = b;  return *this; }
		GridCtrl& Moving(bool b = true)             { moving               = b;  return *this; }
		GridCtrl& Removing(bool b = true)           { removing             = b;  return *this; }
		GridCtrl& Accepting(bool b = true)          { accepting            = b;  return *this; }
		GridCtrl& Canceling(bool b = true)          { canceling            = b;  return *this; }
		GridCtrl& Navigating(bool b = true)         { navigating           = b;  return *this; }
		GridCtrl& Searching(bool b = true)          { searching            = b;  return *this; }
		GridCtrl& Closing(bool b = true)            { closing              = b;  return *this; }
		GridCtrl& Editing(bool b = true)            { editing              = b;  return *this; }
		GridCtrl& EditsInNewRow(bool b = true)      { edits_in_new_row     = b;  return *this; }
		GridCtrl& Hiding(bool b = true)             { hiding               = b;  return *this; }
		GridCtrl& Clipboard(bool b = true)          { clipboard            = b;  return *this; }
		GridCtrl& ExtraPaste(bool b = true)         { extra_paste          = b;  return *this; }
		GridCtrl& FixedPaste(bool b = true)         { fixed_paste          = b;  return *this; }
		GridCtrl& ClipboardCopy(bool b = true)		{ copy_allowed         = b;  return *this; }
		GridCtrl& ClipboardCut(bool b = true)		{ cut_allowed          = b;  return *this; }
		GridCtrl& ClipboardPaste(bool b = true)		{ paste_allowed        = b;  return *this; }
		GridCtrl& CopyColumnNames(bool b = true)    { copy_column_names    = b;  return *this; }
		GridCtrl& AskRemove(bool b = true)          { ask_remove           = b;  return *this; }
		GridCtrl& RowChanging(bool b = true)        { row_changing         = b;  return *this; }

		GridCtrl& DrawFocus(bool b = true)          { draw_focus           = b;  return *this; }

		GridCtrl& RejectNullRow(bool b = true)      { reject_null_row      = b;  return *this; }
		GridCtrl& KeepLastRow(bool b = true)        { keep_last_row        = b;  return *this; }
		GridCtrl& RemoveHides(bool b = true)        { remove_hides         = b;  return *this; }
		GridCtrl& TabChangesRow(bool b = true)      { tab_changes_row      = b;  return *this; }
		GridCtrl& TabAddsRow(bool b = true)         { tab_adds_row         = b;  return *this; }
		GridCtrl& EnterLikeTab(bool b = true)       { enter_like_tab       = b;  return *this; }
		GridCtrl& FullColResizing(bool b = true)    { full_col_resizing    = b;  return *this; }
		GridCtrl& FullRowResizing(bool b = true)    { full_row_resizing    = b;  return *this; }
		GridCtrl& Chameleon(bool b = true)          { chameleon            = b;  return *this; }
		GridCtrl& SummaryRow(bool b = true)         { summary_row          = b;  return *this; }
		GridCtrl& Popups(bool b = true)             { popups               = b;  return *this; }
		GridCtrl& FocusLostAccepting(bool b = true) { focus_lost_accepting = b;  return *this; }
		
		GridCtrl& SearchOffset(int offset)          { find_offset     = offset;  return *this; }
		GridCtrl& SearchMoveCursor(bool b = true)   { search_move_cursor   = b;  return *this; }
		GridCtrl& SearchImmediate(bool b = true)    { search_immediate     = b;  return *this; }
		GridCtrl& SearchHideRows(bool b = true)     { search_hide          = b;  return *this; }
		GridCtrl& SearchDisplay(bool b = true)      { search_display       = b;  return *this; }
		static void IndexAsColumn(bool b = true)	{ index_as_column      = b;                }
		static void ReverseSortIcon(bool b = true)	{ reverse_sort_icon    = b;                }

		GridCtrl& SetToolBar(bool b = true, int align = BarCtrl::BAR_BOTTOM, int frame = 1);
		ToolBar&  GetToolBar() { return bar; }

		GridCtrl& ResizePanel(bool b = true);

		GridCtrl& BeforeAfterInserting()     { bains = 1; return *this; }
		GridCtrl& AfterBeforeInserting()     { bains = 2; return *this; }

		GridCtrl& HideColumn(int n = -1, bool refresh = true);
		GridCtrl& ShowColumn(int n, bool refresh = true);

		GridCtrl& HideRow(int n = -1, bool refresh = true);
		GridCtrl& ShowRow(int n, bool refresh = true);

		void MenuHideColumn(int n);

		void HideRows(bool repaint = false);
		void ShowRows(bool repaint = false);

		/* Virutals */

		virtual void Paint(Draw &w);
		virtual void MouseMove(Point p, dword keyflags);
		virtual void LeftDown(Point p, dword keyflags);
		virtual void LeftUp(Point p, dword keyflags);
	    virtual void LeftDouble(Point p, dword keyflags);
	    virtual void LeftRepeat(Point p, dword keyflags);
	    virtual void RightDown(Point p, dword keyflags);
		virtual void Layout();
	    virtual void MouseLeave();
	    virtual bool Key(dword key, int);
		virtual void GotFocus();
		virtual void LostFocus();
		virtual void ChildGotFocus();
		virtual void ChildLostFocus();
		virtual void MouseWheel(Point p, int zdelta, dword keyflags);
	    virtual Image CursorImage(Point p, dword);
	    virtual void State(int reason);
	    virtual void ChildMouseEvent(Ctrl *child, int event, Point p, int zdelta, dword keyflags);
		virtual void ChildFrameMouseEvent(Ctrl *child, int event, Point p, int zdelta, dword keyflags);
		virtual bool Accept();
		virtual void Reject();

		virtual void DragAndDrop(Point p, PasteClip& d);

		void ChildAction(Ctrl *child, int event);
		void RestoreFocus();

		/* Methods */

		ItemRect& InsertColumn(int pos, const char *name = "", int size = -1, bool idx = false);
		ItemRect& AddColumn(const Id id, const char *name = "", int size = -1, bool idx = false);
		ItemRect& AddColumn(const String& name, int size = -1, bool idx = false);
		ItemRect& AddColumn(const char *name = "", int size = -1, bool idx = false);

		void RemoveColumn(int n, int count = 1);

		ItemRect& AddHiddenColumn()                 { return AddColumn(0, 0);    }
		ItemRect& AddHiddenColumn(const char *name) { return AddColumn(name, 0); }
		ItemRect& AddHiddenColumn(String &name)     { return AddColumn(name, 0); }

		ItemRect& AddIndex()                        { return AddColumn("", 0, true);    }
		ItemRect& AddIndex(Id id)                   { return AddColumn(id, ~id, 0, true); }
		ItemRect& AddIndex(const char *name)        { return AddColumn(name, 0, true); }
		ItemRect& AddIndex(String &name)            { return AddColumn(name, 0, true); }

		ItemRect& AddRow(int n = 1, int size = -1);
		GridCtrl& Add() { AddRow(); return *this; }

		//$-GridCtrl& Add(const Value& [, const Value& ]...);
		#define  E__Add(I)      GridCtrl& Add(__List##I(E__Value));
			__Expand(E__Add)
		#undef   E__Add
		//$+

		//$-ItemRect& AddRow(const Value& [, const Value& ]...);
		#define  E__Add(I)      ItemRect& AddRow(__List##I(E__Value));
			__Expand(E__Add)
		#undef   E__Add
		//$+

		GridCtrl& AddSeparator(Color c);

		int       GetColumnCount() const      { return total_cols - fixed_cols; }
		int       GetFixedColumnCount() const { return fixed_cols;              }

		ItemRect& GetColumn(int n);
		ItemRect& GetColumn();
		ItemRect& GetRow(int n);
		ItemRect& GetRow();
		Item&     GetCell(int n, int m);
		Item&     GetCell(int n, Id id);
		
		bool IsColumn(const Id& id);

		int  GetCurrentRow() const;
		bool IsCurrentRow() const;
		void RestoreCurrentRow();

		int GetWidth(int n = -1);
		int GetHeight(int n = -1);
		int GetFixedWidth();
		int GetFixedHeight();

		void Set(int r, int c, const Value &val);
		void Set(int r, Id id, const Value &val);
		void Set(int r, const char *s, const Value &val);
		void Set(int c, const Value &val);
		void Set(Id id, const Value &val);
		void Set(int r, const Vector<Value> &v, int data_offset = 0, int column_offset = 0);
		void Set(const Vector<Value> &v, int data_offset = 0, int column_offset = 0);
		void SetAny(int r, int c, const Value &val);
		void SetRaw(int r, int c, const Value &val);
		void SetIndicator(int r, const Value &val);

		void SetCtrl(int r, int c, Ctrl& ctrl);
		void SetCtrl(int r, int c, Ctrl* ctrl);
		void SetCtrl(int c, Ctrl& ctrl);
		void SetCtrl(int c, Ctrl* ctrl);
		
		void ClearCtrl(int r, int c);
		void SetCtrlValue(int r, int c, const Value &val);
		void SetCtrlValue(int c, const Value &val);

		void   SetLast(int c, const Value &val);
		void   SetFixed(int r, int c, const Value &val);
		Value  GetFixed(int r, int c) const;
		Value  GetFixed(int c) const;
		Value  Get(int r, int c) const;
		Value  Get(int c) const;
		Value  Get(Id id) const;
		Value  Get(int r, Id id) const;
		Value  Get() const;
		Value  Get(const char * alias) const;
		Value  Get(int r, const char * alias) const;
		Value  GetRaw(int r, int c) const;
		Value  GetNew(int c) const;
		Value  GetFirst(int c) const;
		Value  GetLast(int c) const;
		Value  GetPrev(Id id) const;
		Value  GetPrev(int c) const;
		Value& operator() (int r, int c);
		Value& operator() (int c);
		Value& operator() (Id id);
		Value& operator() (int r, Id id);
		Value& operator() (const char * alias);
		Value& operator() (int r, const char * alias);
		void   SetSummary(int c, const Value& val);
		void   SetSummary(Id id, const Value& val);
		Value  GetSummary(int c);
		Value  GetSummary(Id id);
		
		Vector< Vector<Value> > GetValues();
		void SetValues(const Vector< Vector<Value> >& v);

		using Ctrl::IsModified;

		bool IsModified(int r, int c);
		bool IsModified(int c);
		bool IsModified(int r, Id id);
		bool IsModified(Id id);

		/* valid only in callbacks */

		bool IsNewRow()         { return newrow_inserted || newrow_appended; }
		bool IsNewRowInserted() { return newrow_inserted; }
		bool IsNewRowAppended() { return newrow_appended; }
		bool IsModifiedRow()    { return row_modified > 0; }
		void CommitNewRow()     { newrow_inserted = newrow_appended = false; }

		bool IsUpdatedRow()     { return vitems[rowidx].operation == GridOperation::UPDATE; }
		bool IsInsertedRow()    { return vitems[rowidx].operation == GridOperation::INSERT; }
		bool IsRemovedRow()     { return vitems[rowidx].operation == GridOperation::REMOVE; }
		bool IsChangedRow()     { return IsUpdatedRow() || IsInsertedRow();                 }
		bool IsVersionedRow()   { return vitems[rowidx].operation.GetVersion() > 0;         }
		int  GetRowOperation()  { return vitems[rowidx].operation;                          }

		Vector<Value> ReadCol(int n = -1, int start_row = -1, int end_row = -1) const;
		Vector<Value> ReadRow(int n = -1, int start_col = -1, int end_col = -1) const;
		GridCtrl& Add(const Vector<Value> &v, int offset = 0, int count = -1, bool hidden = false);

		void SetFixedRows(int n = 1);
		void SetFixedCols(int n = 1);

		int  GetStdRowHeight() { return GD_ROW_HEIGHT; }

		void RefreshRow(int n = -1, bool relative = true, bool fixed = false);
		void RefreshCol(int n = -1, bool relative = true, bool fixed = false);
		void RefreshRows(int from, int to, bool relative = true, bool fixed = false);
		void RefreshItem(int r, int c, bool relative = true);
		void RefreshNewRow();
		void RefreshFrom(int from);

		void RefreshTop();
		void RefreshLeft();
		void RefreshSummary();

		void Repaint(bool recalc_cols = false, bool recalc_rows = false, int opt = RP_ALL);
		void Ready(bool b);

		int  GetMouseRow(Point &p, bool relative = true, bool fixed = false, bool full = true);
		int  GetMouseCol(Point &p, bool relative = true, bool fixed = false, bool full = true);
		Point GetPos(Point p);

		void UpdateCursor();

		int Find(const Value &v, int col = 0, int start_from = 0, int opt = 0) const;
		int Find(const Value &v, Id id, int opt = 0) const;
		int Find(const Value &v0, Id id0, const Value &v1, Id id1, int opt = 0) const;
		int FindCurrent(Id id, int opt = GF::SKIP_CURRENT_ROW) const;
		int FindCurrent(Id id0, Id id1, int opt = GF::SKIP_CURRENT_ROW) const;

		int FindInRow(const Value& v, int row = 0, int start_from = 0) const;

		GridDisplay& GetDisplay() { return *display; }
		GridCtrl&    SetDisplay(GridDisplay &gd) { display = &gd; return *this; }
		
		void SetDisplay(int r, int c, GridDisplay& gd);

		bool IsEdit()  { return isedit; }
		bool StartEdit();
		bool SwitchEdit();
		bool EndEdit(bool accept = true, bool doall = false, bool remove_row = true);
		bool CancelEdit(bool remove_row = true) { return EndEdit(false, false, remove_row); }

		int  Append(int cnt = 1, bool refresh = true, int height = -1);
		void Insert(int i, int cnt = 1);
		void Remove(int i = -1, int cnt = 1);
		void RemoveFirst(int cnt = 1);
		void RemoveLast(int cnt = 1);
		void Duplicate(int i, int cnt = 1);

		void CancelRemove()     { cancel_remove      = true; }
		void CancelUpdate()     { cancel_update      = true; }
		void CancelInsert()     { cancel_insert      = true; }
		void CancelUpdateCell() { cancel_update_cell = true; }
		void CancelAccept()     { cancel_accept      = true; }
		void CancelDuplicate()  { cancel_duplicate   = true; }
		void CancelCursor()     { cancel_cursor      = true; }
		void CancelMove()       { cancel_move        = true; }

		void ClearCursor(bool remove = false);
		void ClearRow(int r = -1, int column_offset = 0);
		void Clear(bool columns = false);
		void Reset();
	
		void ClearOperations();
		void ClearVersions();

		void Begin();
		void Next();
		void Prev();
		void End();
		void Move(int r);
		bool IsEnd();

		bool IsFirst();
		bool IsLast();
		bool IsNext();
		bool IsPrev();

		int  SetCursor(int n);
		void SetCursor(const Point& p);
		int  SetCursorId(int n);
		int  GetCursor(bool rel = false) const;
		int  GetPrevCursor(bool rel = false) const;
		int  GetCursor(int uid) const;
		Point GetCursorPos() const;
		void CenterCursor();
		bool IsCursor() const      { return valid_cursor; }
		bool IsCursorBegin() const { return curpos.y == fixed_rows; }
		bool IsCursorEnd() const   { return curpos.y == total_rows - 1; }

		int  GetNewRowPos();
		int  GetNewRowId();
		int  GetRemovedRowPos();

		int  GetCursorId() const { return GetCursor(true); }
		int  GetPrevCursorId() const { return GetPrevCursor(true); }
		int  GetColId() const;
		int  GetRowId() const;
		int  GetColId(int n) const;
		int  GetRowId(int n) const;
		int  GetColUId() const;
		int  GetRowUId() const;
		int  FindCol(int id) const;
		int  FindCol(const Id& id) const;
		int  FindCol(const String& s) const;
		int  FindRow(int id) const;

		int  GetNextRow(int n);
		int  GetPrevRow(int n);

		bool GoBegin(bool scroll = true);
		bool GoEnd(bool scroll = true, bool goleft = false);
		bool GoNext(bool scroll = true);
		bool GoPrev(bool scroll = true);
		bool GoLeft(bool scroll = true, bool ctrlmode = false);
		bool GoRight(bool scroll = true, bool ctrlmode = false);
		bool GoPageUp(bool scroll = true);
		bool GoPageDn(bool scroll = true);
		bool GoFirstVisible(bool scroll = true);
		void GoTo(int r, bool setcursor = true, bool scroll = true);
		void GoTo(int r, int c, bool setcursor = true, bool scroll = true);
		bool GoTop() { return GoBegin(); }

		void SwapCols(int n, int m);
		bool SwapRows(int n, int m, bool repaint = true);

		void SwapUp(int cnt = 1);
		void SwapDown(int cnt = 1);

		bool CanMoveCol(int n, int m);

		void MoveCol(int n, int m);
		bool MoveRow(int n, int m, bool repaint = true);

		void MoveRows(int n, bool onerow = false);

		int  GetCount() const;
		int  GetRowCount() const { return GetCount(); }
		int  GetVisibleCount() const;
		int  GetFixedCount() const;
		int  GetTotalCount() const;

		void SetRowCount(int n, int size = -1);
		void SetColCount(int n, int size = 100);

		void Select(int n, int cnt = 1);
		void ClearSelection();
		bool IsSelection()           { return selected_rows > 0; }
		bool IsSelectionBegin()      { return sel_begin;         }
		bool IsSelectionEnd()        { return sel_end;           }
		int  GetSelectedCount()      { return selected_rows;     }
		int  GetSelectedItemsCount() { return selected_items;    }
		bool IsSelected(int n, bool relative = true);
		bool IsSelected(int n, int m, bool relative = true);
		bool IsSelected();
		
		void Copy(bool all = false)  { SetClipboard(all, true);  }
		void CopyAll()               { Copy(true);               }

		void DoInsert0(bool edit, bool after);
		void DoInsertBefore0(bool edit);
		void DoInsertBefore();
		void DoInsertBeforeNoEdit();
		void DoInsertAfter0(bool edit);
		void DoInsertAfter();
		void DoInsertAfterNoEdit();
		void DoRemove();
		void DoAppend0(bool edit);
		void DoAppend();
		void DoAppendNoEdit();
		void DoDuplicate0();
		void DoDuplicate();
		void DoEdit();
		void DoEndEdit();
		void DoCancelEdit();
		void DoSelectAll();
		void DoSwapUp();
		void DoSwapDown();
		void DoGoBegin();
		void DoGoEnd();
		void DoGoNext();
		void DoGoPrev();
		void DoGoLeft();
		void DoGoRight();
		void DoGoPageUp();
		void DoGoPageDn();
		void DoFind();

		void StdMenuBar(Bar &bar);
		void StdToolBar(Bar &bar);
		void FindOptsBar(Bar &bar);
		void FindBar(Bar &bar, int cx);
		void InfoBar(Bar &bar, int cx);
		void NavigatingBar(Bar &bar);
		void RemovingMenu(Bar &bar);
		void MovingMenu(Bar &bar);
		void SelectMenu(Bar &bar);
		void ColumnsMenu(Bar &bar);
		void ColumnList(Bar &bar);
		void ClipboardMenu(Bar &bar);
		void PasteAsMenu(Bar &bar);

		void RebuildToolBar() { bar.Set(WhenToolBar); }
		void SetToolBarInfo(String inf);

		void ClearFound(bool showrows = true, bool clear_string = true);

		bool IsEmpty()        { return total_rows <= fixed_rows; }
		bool IsFilled()       { return total_rows > fixed_rows;  }
		bool IsOrderChanged() { return row_order; }
		bool IsDataChanged()  { return row_data; }
		bool IsChanged()      { return row_order || row_data; }
		void ClearChanged()   { row_order = row_data = false; ClearModify(); }

		bool IsRowEditable(int r = -1);
		bool IsRowClickable(int r = -1);

		void Serialize(Stream &s);

		Ctrl * GetCtrl(int r, int c); // checks visibility - if widget is out of view, returns NULL
		Ctrl * GetCtrlAt(int r, int c); // does not check visibility
		Ctrl * GetCtrl(int c); // checks visibility - if widget is out of view, returns NULL

		static int GetStdHeight() { return Draw::GetStdFontCy() + 4; }

		void Debug(int n = 0);
		void UpdateScrollbars() { UpdateSb(); }

		void JoinCells(int left, int top, int right, int bottom, bool relative = true);
		void JoinFixedCells(int left, int top, int right, int bottom);
		void JoinRow(int n, int left, int right);
		void JoinRow(int left = -1, int right = -1);
		void UpdateJoins(int row, int col, int cnt = 1);

		GridCtrl& Sort(int sort_col, int sort_mode = SORT_UP, bool multisort = false, bool repaint = true);
		GridCtrl& Sort(Id id, int sort_mode = SORT_UP, bool multisort = false, bool repaint = true);
		GridCtrl& MultiSort(int sort_col, int sort_mode = SORT_UP);
		GridCtrl& MultiSort(Id id, int sort_mode = SORT_UP);
		void ClearSort();
		void ReSort();
		void MarkSort(int col, int sort_mode);
		void MarkSort(Id id, int sort_mode);
		
		Vector<SortOrder> GetSortOrder() const;
		Vector<Id> GetSortOrderId() const;

		ScrollBar& HorzScroll() { return sbx; }
		ScrollBar& VertScroll() { return sby; }

		Value GetStdConvertedValue(const Value& v) const;
		Value GetConvertedColumn(int col, const Value& v) const;
		Value GetStdConvertedColumn(int col, const Value& v) const;
		String GetString(Id id) const;

		bool Search(dword key);
		int GetResizePanelHeight() const;
		String GetColumnName(int n) const;
		Id GetColumnId(int n) const;

		void SetCtrlFocus(int col);
		void SetCtrlFocus(Id id);

		String GetColumnWidths();
		void ColumnWidths(const char* s);
		#ifdef flagGRIDSQL
		void FieldLayout(FieldOperator& f);
		operator Fields() { return THISBACK(FieldLayout); }
		operator SqlSet() const { return GetColumnList(); }
		SqlSet GetColumnList(bool skip_hidden = false) const;
		#endif
		
		void UpdateSummary(bool b = true);

	private:
		bool IsSelect(int n, int m, bool relative);
	
		bool TabKey(bool enter_mode);

		bool Go0(int jump, bool scroll = true, bool goleft = false, bool ctrlmode = false);

		CurState SetCursor0(Point p, int opt = 0, int dirx = 0, int diry = 0);
		CurState SetCursor0(int x, int y, int opt = 0, int dirx = 0, int diry = 0);
		int SetCursor0(int n);

		bool IsValidCursor(const Point &p, int fc, int lc, int fr, int lr) const;
		bool IsValidCursorVis(const Point &p) const;
		bool IsValidCursorAll(const Point &p) const;
		bool IsValidCursor(const Point &p) const;
		bool IsValidCursor(int c) const;

		void SetItemCursor(Point p, bool b, bool highlight);

		void Insert0(int row, int cnt = 1, bool recalc = true, bool refresh = true, int size = -1);
		bool Remove0(int row, int cnt = 1, bool recalc = true, bool refresh = true, bool whens = true);
		bool Duplicate0(int row, int cnt = 1, bool recalc = true, bool refresh = true);
		int  Append0(int cnt = 1, int size = -1, bool refresh = true);

		void GoCursorLeftRight();

		void UpdateSb(bool horz = true, bool vert = true);
		void UpdateHolder(bool force = false);
		bool UpdateCols(bool force = false);
		bool UpdateRows(bool force = false);
		bool UpdateSizes();
		void UpdateHighlighting(int mode, Point p = Null);
		void UpdateVisColRow(bool col);

		bool HasCtrls();
		bool ShowNextCtrl();
		bool ShowPrevCtrl();
		public:
		void SyncCtrls(int r = -1, int c = -1);
		private:
		void UpdateCtrls(int opt = UC_CHECK_VIS | UC_SHOW | UC_CURSOR | UC_FOCUS);
		void SyncSummary();
		void SyncPopup();

		void SetCtrlsData();
		bool GetCtrlsData(bool samerow = false, bool doall = false, bool updates = true);
		bool CancelCtrlsData(bool all = false);
		void UpdateDefaults(int ri);

		int  GetFocusedCtrlIndex();
		Point GetCtrlPos(Ctrl * ctrl);

		void Split(int state = 0, bool sync = false);
		void Scroll();

		bool IsTopHeader()  { return fixed_rows > 0; }
		bool IsLeftHeader() { return fixed_cols > 1 || indicator; }

		bool IsMouseBody(Point &p);

		void DrawLine(bool iniLine, bool delLine);

		Rect GetItemRect(int r, int c, bool hgrid = false, bool vgrid = false, bool hrel = false, bool vrel = false);

		bool Match(const WString &f, const WString &s, int &fs, int &fe);
		int  ShowMatchedRows(const WString &f);

		void SelectCount(int i, int cnt = 1, bool sel = true);
		void SelectRange(int from, int to, bool sel = true);
		void SelectRange(Point from, Point to, bool sel = true, bool fullrow = false);
		void SelectRange(const Rect& r, bool sel , bool fullrow );
		void ShiftSelect(int from, int to);
		void ShiftSelect(Point from, Point to);
		void SelectInverse(int from, int to);
		int  GetMinRowSelected();
		int  GetMaxRowSelected();

		void CloseGrid();
		String RowFormat(const char *s);

		void SetFindOpts(int n);

		Item& GetItem(const Point &p) { return GetItem(p.y, p.x); }
		Item& GetItem(int n, int m)   { return items[vitems[n].id][hitems[m].id]; }

		void Set0(int r, int c, const Value &val, bool paste = false);
		Value Get0(int r, int c) const;

		int GetSplitCol(const Point &p, int splitSize = 5, bool full = false);
		int GetSplitRow(const Point &p, int splitSize = 5, bool full = false);

		int GetFirst0(Vector<ItemRect> &its, int total, int sb, int diff);
		int GetFirstVisCol(int diff);
		int GetFirstVisRow(int diff);

		bool SetDiffItemSize(bool horizontal, RectItems &its, int n, int diff, bool newsize = true);
		void Recalc(bool horizontal, RectItems &its, int n, double size, double diff);
		bool Recalc(bool horizontal, RectItems &its, int mode = -1);
		bool RecalcCols(int mode = -1);
		bool RecalcRows(int mode = -1);
		void CalcIntPos(RectItems &its, int n, int maxsize, int cnt, int resize_mode, bool renumber = false);

		Rect& AlignRect(Rect &r, int i);

		void MouseAccel(const Point &p, bool horz, bool vert, dword keyflags);

		void GSort(int scol = -1);
		void GSort(int col, int order, int from, int count = -1);
		void Multisort();
		int  InMultisort(int col);
		void ClearMultisort(int n = 0);
		bool IsSorted();
		void MarkSort(int col, int sort_mode, bool refresh);

		void DrawHorzDragLine(Draw &w, int pos, int cx, int size, Color c);
		void DrawVertDragLine(Draw &w, int pos, int size, int dx, Color c);

		void SetOrder();
		
		void Nothing();
		void Init();

		Ctrl * GetCtrl(int x, int y, bool check_visibility, bool hrel = false, bool vrel = false, bool check_edits = true);
		Ctrl * GetCtrl(const Point &p, bool check_visibility, bool hrel = false, bool vrel = false, bool check_edits = true);
		bool IsCtrl(Point &p, bool check_visibility = true);

		GridClipboard GetClipboard();
		void SetClipboard(bool all = false, bool silent = false);
		bool IsClipboardAvailable();
		void PasteCallbacks(bool new_row);
		void Paste(int mode = 0);
		void DoCopy();
		void DoPaste();
		void DoPasteInsertedRows();
		void DoPasteAppendedRows();

		Point GetBarOffset();
		void ClearModified();

		int  GetIdCol(int id, bool checkall = false) const;
		int  GetIdRow(int id, bool checkall = false) const;
		Value GetItemValue(const Item& it, int id, const ItemRect& hi, const ItemRect& vi);
		void  GetItemAttrs(const Item& it, const Value& val, int r, int c, const ItemRect& vi, const ItemRect& hi, dword& style, GridDisplay*& gd, Color& fg, Color& bg, Font& fnt);
		Item& GetItemSize(int &r, int &c, int &x, int &y, int &cx, int &cy, bool &skip, bool relx = true, bool rely = true);
		Value GetAttrTextVal(const Value& val);
		
		Image HorzPosImage();
		Image VertPosImage();

	private:
		bool WhenInsertRow0();

	public:

		Event<Bar&> WhenMenuBar;
		Event<Bar&> WhenToolBar;

		Event<> WhenLeftDouble;
		Event<> WhenLeftClick;
		Event<> WhenEnter;
		Event<> WhenEscape;

		Event<> WhenStartEdit;
		Event<> WhenEndEdit;

		Event<> WhenCreateRow;

		Event<> WhenAcceptRow;
		Event<> WhenAcceptedRow;

		Event<> WhenInsertRow;
		Event<> WhenUpdateRow;
		Event<> WhenRemoveRow;
		Event<> WhenRemovedRow;
		Event<> WhenDuplicateRow;
		
		Callback2<int, int> WhenMoveRow;

		Event<> WhenCancelNewRow;

		Event<> WhenUpdateCell;
		
		Event<> WhenUpdateSummary;

		Event<> WhenNewRow;
		Event<> WhenChangeCol;
		Event<> WhenBeforeChangeCol;
		Event<> WhenChangeRow;
		Event<> WhenBeforeChangeRow;
		Event<> WhenCursor;
		Event<> WhenEmpty;

		Event<> WhenCtrlAction;
		Event<> WhenCtrlsAction;

		Event<> WhenSearchCursor;
		Event<> WhenClose;
		Event<> WhenChangeOrder;

		Event<> StdInsert;
		Event<> StdAppend;
		Event<> StdRemove;
		Event<> StdDuplicate;
		Event<> StdEdit;
		
		Event<> WhenSort;
		Event<> WhenSorted;
		
		Event<Value&> ProcessSummaryValue;

		Event<int, int, Value&> WhenPasteCell;
		
		Event<String&> WhenPasting;
		Event<>        WhenPaste;
};

class GridText : Ctrl
{
	private:
		GridDisplay display;
		GridCtrl* parent;
		const Id* column;
		Color fg, bg;
		Font fnt;

	public:

		GridText() {}

		virtual void Paint(Draw& w)
		{
			Rect r = GetRect();
			display.Paint(w, r.left, r.top, r.Width(), r.Height(), parent->Get(*column), 0, fg, bg, fnt);
		}

		void Ink(Color c)    { fg = c;  }
		void Paper(Color c)  { bg = c;  }
		void SetFont(Font f) { fnt = f; }
		void Column(const Id& c) { column = &c; }
};

template<> void Xmlize(XmlIO& xml, GridCtrl& g);
template<> void Jsonize(JsonIO& json, GridCtrl& g);

}

#endif
