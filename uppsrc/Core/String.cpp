#include "Core.h"

namespace Upp {

#ifdef _DEBUG
void String0::Dsyn()
{
	String *d_str = static_cast<String *>(this);
	d_str->s = Begin();
	d_str->len = GetCount();
}
#endif

String0::Rc String0::voidptr[2];

#ifndef UPP_HEAP

inline void  *MemoryAlloc32_i()              { return new byte[32]; }
inline void   MemoryFree32_i(void *ptr)      { delete[] (byte *)ptr; }

#include "StringMem.i"

#endif

bool String0::LEq(const String0& s) const
{
	int l = GetCount();
	return l == s.GetCount() && inline_memeq8_aligned(begin(), s.begin(), l);
}

hash_t String0::LHashValue() const
{
	int l = LLen();
	if(l < 15) { // must be the same as small hash
#ifdef HASH64
		qword m[2];
		m[0] = m[1] = 0;
		memcpy8((char *)m, ptr, l);
		((byte *)m)[SLEN] = l;
		return CombineHash(m[0], m[1]);
#else
		dword m[4];
		m[0] = m[1] = m[2] = m[3] = 0;
		memcpy8((char *)m, ptr, l);
		((byte *)m)[SLEN] = l;
		return CombineHash(m[0], m[1], m[2], m[3]);
#endif
	}
	return memhash(ptr, l);
}

int String0::CompareL(const String0& s) const
{
	const char *a = Begin();
	int la = GetLength();
	const char *b = s.Begin();
	int lb = s.GetLength();
	int q = inline_memcmp_aligned(a, b, min(la, lb));
	return q ? q : SgnCompare(la, lb);
}

char *String0::Insert(int pos, int count, const char *s)
{
	ASSERT(pos >= 0 && count >= 0 && pos <= GetCount());
	int len = GetCount();
	int newlen = len + count;
	if(newlen < len) // overflow, string >2GB
		Panic("String is too big!");
	char *str = (char *)Begin();
	if(newlen < GetAlloc() && !IsSharedRef() && (!s || s < str || s > str + len)) {
		if(pos < len)
			memmove(str + pos + count, str + pos, len - pos);
		if(IsSmall())
			SLen() = newlen;
		else
			LLen() = newlen;
		str[newlen] = 0;
		if(s)
			memcpy8(str + pos, s, count);
		Dsyn();
		return str + pos;
	}
	char kind;
	char *p = Alloc(max(len >= int((int64)2 * INT_MAX / 3) ? INT_MAX : len + (len >> 1), newlen),
	                kind);
	if(pos > 0)
		memcpy8(p, str, pos);
	if(pos < len)
		memcpy8(p + pos + count, str + pos, len - pos);
	if(s)
		memcpy8(p + pos, s, count);
	p[newlen] = 0;
	Free();
	ptr = p;
	LLen() = newlen;
	SLen() = 15;
	chr[KIND] = kind;
	Dsyn();
	return ptr + pos;
}

void String0::UnShare()
{
	if(IsSharedRef()) {
		int len = LLen();
		char kind;
		char *p = Alloc(len, kind);
		memcpy8(p, ptr, len + 1);
		Free();
		chr[KIND] = kind;
		ptr = p;
	}
}

void String0::SetSLen(int l)
{
	SLen() = l;
	memset(chr + l, 0, 15 - l);
}

void String0::Remove(int pos, int count)
{
	ASSERT(pos >= 0 && count >= 0 && pos + count <= GetCount());
	UnShare();
	char *s = (char *)Begin();
	memmove(s + pos, s + pos + count, GetCount() - pos - count + 1);
	if(IsSmall())
		SetSLen(SLen() - count);
	else
		LLen() -= count;
	Dsyn();
}

void String0::Set(int pos, int chr)
{
	ASSERT(pos >= 0 && pos < GetCount());
	UnShare();
	Ptr()[pos] = chr;
}

void String0::Trim(int pos)
{
	ASSERT(pos >= 0 && pos <= GetCount());
	if(IsSmall()) {
		chr[pos] = 0;
		SetSLen(pos);
	}
	else {
		UnShare();
		ptr[pos] = 0;
		LLen() = pos;
	}
	Dsyn();
}

void String0::Cat(const char *s, int len)
{
	if(IsSmall()) {
		if(SLen() + len < 14) {
			memcpy8(chr + SLen(), s, len);
			SLen() += len;
			chr[(int)SLen()] = 0;
			Dsyn();
			return;
		}
	}
	else
		if((int)LLen() + len < LAlloc() && !IsSharedRef()) {
			memcpy8(ptr + LLen(), s, len);
			LLen() += len;
			ptr[LLen()] = 0;
			Dsyn();
			return;
		}
	Insert(GetCount(), len, s);
}

void String0::Reserve(int r)
{
	int l = GetCount();
	Insert(GetCount(), r, NULL);
	Trim(l);
}

void String0::SetL(const char *s, int len)
{
	char *p = Alloc(len, chr[KIND]);
	memcpy8(p, s, len);
	p[len] = 0;
	ptr = p;
	LLen() = len;
	SLen() = 15;
}

void String::AssignLen(const char *s, int slen)
{
	int  len = GetCount();
	char *str = (char *)Begin();
	if(s >= str && s <= str + len)
		*this = String(s, slen);
	else {
		String0::Free();
		String0::Set0(s, slen);
	}
}

String String::GetVoid()
{
	String s;
	s.ptr = (char *)(voidptr + 1);
	s.LLen() = 0;
	s.SLen() = 15;
	s.chr[KIND] = 50;
	return s;
}

bool String::IsVoid() const
{
	return IsRef() && ptr == (char *)(voidptr + 1);
}

WString String::ToWString() const
{
	return WString(Begin(), GetCount());
}

int String::GetCharCount() const
{
	return GetDefaultCharset() == CHARSET_UTF8 ?  utf8len(Begin(), GetCount()) : GetCount();
}

String::String(StringBuffer& b)
{
	Zero();
	if(b.pbegin == b.buffer) {
		String0::Set0(b.pbegin, (int)(uintptr_t)(b.pend - b.pbegin));
		return;
	}
	int l = b.GetLength();
	if(l <= 14) {
		memcpy8(chr, b.pbegin, l);
		SLen() = l;
		b.Free();
	}
	else {
		ptr = b.pbegin;
		ptr[l] = 0;
		SLen() = 15;
		LLen() = l;
		chr[KIND] = min(b.GetAlloc(), 255);
		if(GetAlloc() > 4 * GetLength() / 3)
			Shrink();
	}
	b.Zero();

//	char h[100];
//	DLOG(sprintf(h, "String(StringBuffer) end %p (%p)", ptr, this));
	Dsyn();
//	DLOG(sprintf(h, "String(StringBuffer) end2 %p (%p)", ptr, this));
}

void StringBuffer::Realloc(dword n, const char *cat, int l)
{
	int al;
	size_t ep = pend - pbegin;
	if(n > INT_MAX)
		n = INT_MAX;
	bool realloced = false;
	char *p;
	if((int)(limit - pbegin) > 800) {
		size_t sz = sizeof(Rc) + n + 1;
		Rc *rc = (Rc *)pbegin - 1;
		if(MemoryTryRealloc(rc, sz)) {
			realloced = true;
			al = rc->alloc = (int)min((size_t)INT_MAX, sz - sizeof(Rc) - 1);
			p = pbegin;
		}
	}
	if(!realloced) {
		p = Alloc(n, al);
		memcpy8(p, pbegin, min((dword)GetLength(), n));
	}
	if(cat) {
		if(ep + l > INT_MAX)
			Panic("StringBuffer is too big (>2GB)!");
		memcpy8(p + ep, cat, l);
		ep += l;
	}
	if(!realloced) {
		Free();
		pbegin = p;
	}
	pend = pbegin + ep;
	limit = pbegin + al;
}

void StringBuffer::Expand()
{
	Realloc(GetLength() * 3 / 2);
	if(pend == limit)
		Panic("StringBuffer is too big!");
}

void StringBuffer::SetLength(int l)
{
	if(l > GetAlloc())
		Realloc(l);
	pend = pbegin + l;
}

void StringBuffer::Shrink()
{
	int l = GetLength();
	if(l < GetAlloc() && l > 14)
		Realloc(l);
	pend = pbegin + l;
}

void StringBuffer::ReallocL(const char *s, int l)
{
	Realloc(max(GetLength(), l) + GetLength(), s, l);
}

String TrimLeft(const String& str)
{
	const char *s = str;
	if(!IsSpace(*s))
		return str;
	while(IsSpace((byte)*s)) s++;
	return String(s, str.End());
}

String TrimRight(const String& str)
{
	if(str.IsEmpty())
		return str;
	const char *s = str.Last();
	if(!IsSpace(*s))
		return str;
	while(s >= ~str && IsSpace((byte)*s)) s--;
	return String(~str, s + 1);
}

String TrimBoth(const String& str)
{
	return TrimLeft(TrimRight(str));
}

String TrimLeft(const char *sw, int len, const String& s)
{
	return s.StartsWith(sw, len) ? s.Mid(len) : s;
}

String TrimRight(const char *sw, int len, const String& s)
{
	return s.EndsWith(sw, len) ? s.Mid(0, s.GetCount() - len) : s;
}

struct StringICompare__
{
	int encoding;
	int operator()(char a, char b) const { return ToUpper(a, encoding) - ToUpper(b, encoding); }

	StringICompare__(int e) : encoding(e) {}
};

int CompareNoCase(const String& a, const String& b, byte encoding)
{
	if(encoding == CHARSET_DEFAULT) encoding = GetDefaultCharset();
	if(encoding == CHARSET_UTF8) return CompareNoCase(FromUtf8(a), FromUtf8(b));
#ifdef DEPRECATED
	return IterCompare(a.Begin(), a.End(), b.Begin(), b.End(), StringICompare__(encoding));
#else
	return CompareRanges(a, b, StringICompare__(encoding));
#endif
}

int CompareNoCase(const String& a, const char *b, byte encoding)
{
	if(encoding == CHARSET_DEFAULT) encoding = GetDefaultCharset();
	if(encoding == CHARSET_UTF8) return CompareNoCase(FromUtf8(a), FromUtf8(b, (int)strlen(b)));
#ifdef DEPRECATED
	return IterCompare(a.Begin(), a.End(), b, b + strlen(b), StringICompare__(encoding));
#else
	return CompareRanges(a, SubRange(b, b + strlen(b)), StringICompare__(encoding));
#endif
}

}
