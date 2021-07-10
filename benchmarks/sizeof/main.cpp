#include <Draw/Draw.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);

#ifdef CPU_64
	RLOG("CPU_64");
#endif
#ifdef CPU_UNALIGNED
	RLOG("CPU_UNALIGNED");
#endif
#ifdef CPU_LE
	RLOG("CPU_LE");
#endif
#ifdef CPP_11
	RLOG("CPP_11");
#endif
#ifdef CPU_X86
	RLOG("CPU_X86");
#endif
#ifdef CPU_ARM
	RLOG("CPU_ARM");
#endif
#ifdef CPU_NEON
	RLOG("CPU_NEON");
#endif
	RLOG("========================");
	RDUMP(sizeof(bool));
	RDUMP(sizeof(int));
	RDUMP(sizeof(unsigned));
	RDUMP(sizeof(long int));
	RDUMP(sizeof(long));
	RDUMP(sizeof(unsigned long));
	RDUMP(sizeof(short int));
	RDUMP(sizeof(byte));
	RDUMP(sizeof(word));
	RDUMP(sizeof(dword));
	RDUMP(sizeof(qword));
	RDUMP(sizeof(int64));
	RDUMP(sizeof(uint64));
	RDUMP(sizeof(float));
	RDUMP(sizeof(double));
	RDUMP(sizeof(long double));
	RDUMP(sizeof(long long));
	RDUMP(sizeof(unsigned long long));
	RLOG("========================");
	RDUMP(sizeof(Date));
	RDUMP(sizeof(Time));
	RDUMP(sizeof(Size));
	RDUMP(sizeof(Point));
	RDUMP(sizeof(Rect));
	RDUMP(sizeof(Size16));
	RDUMP(sizeof(Point16));
	RDUMP(sizeof(Rect16));
	RDUMP(sizeof(Size64));
	RDUMP(sizeof(Point64));
	RDUMP(sizeof(Rect64));
	RDUMP(sizeof(Sizef));
	RDUMP(sizeof(Pointf));
	RDUMP(sizeof(Rectf));
	RDUMP(sizeof(Color));
	RDUMP(sizeof(Complex));
	RDUMP(sizeof(Uuid));
	RDUMP(sizeof(Mutex));
	RDUMP(sizeof(RWMutex));
	RLOG("========================");
	RDUMP(sizeof(String));
	RDUMP(sizeof(WString));
	RDUMP(sizeof(Value));
	RDUMP(sizeof(ValueArray));
	RDUMP(sizeof(ValueMap));
	RDUMP(sizeof(XmlNode));
	RLOG("========================");
	RDUMP(sizeof(Vector<int>));
	RDUMP(sizeof(Array<int>));
	RDUMP(sizeof(Index<int>));
	RDUMP(sizeof(ArrayMap<int, int>));
	RDUMP(sizeof(VectorMap<int, int>));
	RDUMP(sizeof(BiArray<int>));
	RDUMP(sizeof(BiVector<int>));
	RDUMP(sizeof(InVector<int>));
	RDUMP(sizeof(InArray<int>));
	RDUMP(sizeof(SortedIndex<int>));
	RDUMP(sizeof(SortedVectorMap<int, int>));
	RDUMP(sizeof(SortedArrayMap<int, int>));
	RLOG("========================");
	RDUMP(sizeof(Image));
	RDUMP(sizeof(Font));
	RDUMP(sizeof(AttrText));
	RDUMP(sizeof(Painting));
	RDUMP(sizeof(Drawing));
}
