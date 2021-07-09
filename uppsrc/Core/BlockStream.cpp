#include "Core.h"

namespace Upp {

#define LLOG(x)           // DLOG(x)
#define LDUMP(x)          // DUMP(x)
#define LLOGHEXDUMP(x, s) // LOGHEXDUMP(x, s)

void BlockStream::SetBufferSize(dword size)
{
	int64 p = 0;
	if(IsOpen()) {
		p = GetPos();
		Flush();
	}
	int n = 1;
	while(size >> (n + 1))
		n++;
	pagesize = 1 << n;
	pagemask = (uint64)-1 << n;
	if(buffer)
		MemoryFree(buffer);
	buffer = (byte *)MemoryAlloc(pagesize);
	pos = 0;
	ptr = rdlim = wrlim = buffer;
	pagepos = -1;
	if(IsOpen())
		Seek(p);
}

BlockStream::BlockStream()
{
	buffer = NULL;
}

BlockStream::~BlockStream()
{
	if(buffer)
		MemoryFree(buffer);
}

int64 BlockStream::GetSize() const {
	if(IsError()) return 0;
	return max(streamsize, ptr - buffer + pos);
}

void BlockStream::SyncSize()
{
	streamsize = max(streamsize, ptr - buffer + pos);
}

void BlockStream::Flush() {
	if(!IsOpen() || IsError()) return;
	if(pagedirty && pagepos >= 0) {
		SyncSize();
		int size = (int)min<int64>(streamsize - pagepos, pagesize);
		LLOG("Write: " << pagepos << ", " << size);
		Write(pagepos, buffer, size);
		streamsize = max(streamsize, pagepos + size);
	}
	wrlim = buffer;
	pagedirty = false;
}

void BlockStream::SetPos(int64 p)
{
	SyncSize();
	pos = p & pagemask;
	ptr = p - pos + buffer;
	rdlim = wrlim = buffer;
}

void BlockStream::Seek(int64 apos) {
	if(IsError()) return;
	LLOG("Seek " << apos);
	if(style & STRM_WRITE) {
		SetPos(apos);
		if(apos > streamsize) {
			SetStreamSize(apos);
			streamsize = apos;
		}
	}
	else {
		if(apos > streamsize)
			apos = streamsize;
		SetPos(apos);
	}
}

bool BlockStream::SyncPage()
{
	if(pagepos != pos) {
		int n = (int)min<int64>(streamsize - pos, pagesize);
		Flush();
		pagepos = pos;
		LLOG("Read:" << pagepos << ", " << n);
		if(n > 0 && (int)Read(pagepos, buffer, n) != n) {
			SetLastError();
			return false;
		}
	}
	rdlim = wrlim = buffer;
	return true;
}

bool BlockStream::SyncPos()
{
	if(IsError())
		return false;
	SetPos(GetPos());
	return SyncPage();
}

int BlockStream::_Term() {
	if(IsError() || !IsOpen()) return -1;
	if(ptr < rdlim)
		return *ptr;
	if(SyncPos())
		rdlim = buffer + (int)min<int64>(streamsize - pos, pagesize);
	else {
		rdlim = buffer;
		return -1;
	}
	return ptr < rdlim ? *ptr : -1;
}

void BlockStream::_Put(int c) {
	if(!IsOpen()) return;
	if(IsError() || !SyncPos())
		ptr = buffer;
	wrlim = buffer + pagesize;
	pagedirty = true;
	*ptr++ = c;
}

int  BlockStream::_Get() {
	if(IsError() || !IsOpen()) return -1;
	int c = _Term();
	if(c >= 0) ptr++;
	return c;
}

void BlockStream::_Put(const void *data, dword size) {
	if(IsError() || !IsOpen()) return;
	LLOG("Put " << size);
	if(!size)
		return;
	const byte *s = (const byte *)data;
	if(!SyncPos())
		return;
	int64 pos0 = GetPos();
	int64 pg0 = pos0 & pagemask;
	int64 pos1 = pos0 + size;
	int64 pg1 = pos1 & pagemask;
	wrlim = buffer + pagesize;
	pagedirty = true;
	if(pg0 == pg1) {
		memcpy8(buffer + pos0 - pos, data, size);
		ptr = buffer + pos1 - pos;
	}
	else {
		int n = int(pos + pagesize - pos0);
		memcpy8(buffer + pos0 - pos, s, n);
		s += n;
		n = dword(pg1 - pg0) - pagesize;
		streamsize = max(pos + pagesize + n, streamsize);
		int64 wpos = pos + pagesize;
		SetPos(pos0 + size);
		SyncPage();
		if(n)
			Write(wpos, s, n);
		s += n;
		if(pos1 > pg1) {
			wrlim = buffer + pagesize;
			pagedirty = true;
			memcpy8(buffer, s, int(pos1 - pg1));
		}
	}
}

dword BlockStream::_Get(void *data, dword size) {
	if(IsError() || !IsOpen()) return 0;
	LLOG("Get " << size);
	if(size == 0) return 0;
	_Term();
	byte *t = (byte *)data;
	int64 pos0 = GetPos();
	int64 pg0 = pos0 & pagemask;
	size = (int)min<int64>(GetSize() - pos0, size);
	int64 pos1 = pos0 + size;
	int64 pg1 = pos1 & pagemask;
	if(pg0 == pg1) {
		SyncPage();
		memcpy8(data, buffer + pos0 - pos, size);
		ptr = buffer + pos1 - pos;
		_Term();
	}
	else {
		int last = int(pos1 - pg1);
		if(pagepos == pg1) {
			memcpy8(t + size - last, buffer, last);
			last = 0;
		}
		SyncPage();
		int n = int(pos + pagesize - pos0);
		memcpy8(t, buffer + pos0 - pos, n);
		dword q = dword(pg1 - pg0) - pagesize;
		if(q && Read(pos + pagesize, t + n, q) != q) {
			SetError();
			return 0;
		}
		SetPos(pos0 + size);
		if(last) {
			SyncPage();
			memcpy8(t + size - last, buffer, last);
		}
	}
	return size;
}

void BlockStream::SetSize(int64 size)
{
	if(IsError() || !IsOpen()) return;
	int64 pos = GetPos();
	Flush();
	Seek(0);
	SetStreamSize(size);
	streamsize = size;
	Seek(pos < size ? pos : size);
}

dword BlockStream::Read(int64 at, void *ptr, dword size) {
	NEVER();
	return 0;
}

void  BlockStream::Write(int64 at, const void *data, dword size) {
	NEVER();
}

void  BlockStream::SetStreamSize(int64 pos) {
	NEVER();
}

void BlockStream::OpenInit(dword mode, int64 _filesize) {
	streamsize = _filesize;
	style = STRM_READ|STRM_SEEK;
	SetLoading();
	mode &= ~SHAREMASK;
	if(mode != READ) {
		style |= STRM_WRITE;
		SetStoring();
	}
	rdlim = wrlim = ptr = buffer;
	pos = 0;
	pagepos = -1;
	pagedirty = false;
	if(!buffer)
		SetBufferSize(4096);
	if(mode == APPEND) SeekEnd();
	ClearError();
}

// ---------------------------- File stream -----------------------------

#ifdef PLATFORM_WIN32

void FileStream::SetStreamSize(int64 pos) {
	long lo = (dword)pos, hi = (dword)(pos >> 32);
	if(SetFilePointer(handle, lo, &hi, FILE_BEGIN) == 0xffffffff && GetLastError() != NO_ERROR ||
	   !SetEndOfFile(handle)) {
		SetLastError();
	}
}

void FileStream::SetPos(int64 pos) {
	ASSERT(IsOpen());
	long lo = (dword)pos, hi = (dword)(pos >> 32);
	if(SetFilePointer(handle, lo, &hi, FILE_BEGIN) == 0xffffffff && GetLastError() != NO_ERROR)
		SetLastError();
}

dword FileStream::Read(int64 at, void *ptr, dword size) {
	ASSERT(IsOpen() && (style & STRM_READ));
	dword n;
	SetPos(at);
	if(IsError()) return 0;
	if(!ReadFile(handle, ptr, size, (DWORD *)&n, NULL)) {
		SetLastError();
		return 0;
	}
	return n;
}

void FileStream::Write(int64 at, const void *ptr, dword size) {
	ASSERT(IsOpen() && (style & STRM_WRITE));
	dword n;
	SetPos(at);
	if(IsError()) return;
	if(!WriteFile(handle, ptr, size, &n, NULL)) {
		SetLastError();
		return;
	}
	if(n != size)
		SetError(ERROR_NOT_ENOUGH_SPACE);
}

FileTime FileStream::GetTime() const {
	ASSERT(IsOpen());
	FileTime tm;
	GetFileTime(handle, NULL, NULL, &tm);
	return tm;
}

void     FileStream::SetTime(const FileTime& tm) {
	ASSERT(IsOpen());
	Flush();
	if(!SetFileTime(handle, NULL, NULL, &tm))
		SetLastError();
}

bool FileStream::Open(const char *name, dword mode) {
	LLOG("Open " << name << " mode: " << mode);
	Close();
	int iomode = mode & ~SHAREMASK;
	handle = CreateFileW(ToSystemCharsetW(name),
		iomode == READ ? GENERIC_READ : GENERIC_READ|GENERIC_WRITE,
	#ifdef DEPRECATED
		(mode & NOREADSHARE ? 0 : FILE_SHARE_READ) | (mode & DELETESHARE ? FILE_SHARE_DELETE : 0) |
	#endif
		(mode & NOWRITESHARE ? 0 : FILE_SHARE_WRITE),
		NULL,
		iomode == READ ? OPEN_EXISTING : iomode == CREATE ? CREATE_ALWAYS : OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
	);
	if(handle == INVALID_HANDLE_VALUE) {
		SetLastError();
		return FALSE;
	}
	dword fsz_lo, fsz_hi;
	fsz_lo = ::GetFileSize(handle, &fsz_hi);
	int64 fsz;
	if(fsz_lo == 0xffffffff && GetLastError() != NO_ERROR)
		fsz = 0;
	else
		fsz = fsz_lo | (int64(fsz_hi) << 32);
	OpenInit(iomode, fsz);
	LLOG("OPEN " << handle);
	return TRUE;
}

void FileStream::Close() {
	if(!IsOpen()) return;
	Flush();
	LLOG("CLOSE " << handle);
	if(!CloseHandle(handle)) {
		LLOG("CLOSE ERROR");
		LDUMP(GetLastErrorMessage());
		SetLastError();
	}
	handle = INVALID_HANDLE_VALUE;
}

bool FileStream::IsOpen() const {
	return handle != INVALID_HANDLE_VALUE;
}

FileStream::FileStream(const char *filename, dword mode) {
	handle = INVALID_HANDLE_VALUE;
	Open(filename, mode);
}

FileStream::FileStream() {
	handle = INVALID_HANDLE_VALUE;
}

FileStream::~FileStream() {
	Close();
}

bool FileOut::Open(const char *fn)
{
	return FileStream::Open(fn, FileStream::CREATE|FileStream::NOWRITESHARE);
}

#endif

#ifdef PLATFORM_POSIX

void FileStream::SetStreamSize(int64 pos)
{
	if(handle < 0) return;
	LOFF_T_ cur = LSEEK64_(handle, 0, SEEK_CUR);
	if(cur < 0) {
		SetLastError();
		return;
	}
	LOFF_T_ len = LSEEK64_(handle, 0, SEEK_END);
	if(len < 0) {
		SetLastError();
		LSEEK64_(handle, cur, SEEK_SET);
		return;
	}
	while(pos > len) {
		static char buffer[1024];
		int64 diff = pos - len;
		int chunk = (diff > (int64)sizeof(buffer) ? sizeof(buffer) : (int)diff);
		if(write(handle, buffer, chunk) != chunk) {
			SetLastError();
			LSEEK64_(handle, cur, SEEK_SET);
			return;
		}
		len += chunk;
	}
	if(pos < len) {
		if(cur > pos)
			LSEEK64_(handle, cur = pos, SEEK_SET);
		if(FTRUNCATE64_(handle, pos) < 0)
			SetLastError();
	}
	if(LSEEK64_(handle, cur, SEEK_SET) < 0)
		SetLastError();
}

void FileStream::SetPos(int64 pos) {
	ASSERT(IsOpen());
	if(LSEEK64_(handle, pos, SEEK_SET) < 0)
		SetLastError();
}

dword FileStream::Read(int64 at, void *ptr, dword size) {
	ASSERT(IsOpen() && (style & STRM_READ));
	SetPos(at);
	if(IsError()) return 0;
	int n = read(handle, ptr, size);
	if(n < 0) {
		SetLastError();
		return 0;
	}
	return n;
}

void FileStream::Write(int64 at, const void *ptr, dword size) {
	ASSERT(IsOpen() && (style & STRM_WRITE));
	SetPos(at);
	if(IsError()) return;
	int n = write(handle, ptr, size);
	if(n < 0) {
		SetLastError();
		return;
	}
	if((dword)n != size)
		SetError(ERROR_NOT_ENOUGH_SPACE);
}

FileTime FileStream::GetTime() const {
	ASSERT(IsOpen());
	struct stat fst;
	fstat(handle, &fst);
	return fst.st_mtime;
}

bool FileStream::Open(const char *name, dword mode, mode_t tmode) {
	Close();
	DLOG("Open " << name);
	int iomode = mode & ~SHAREMASK;
	handle = open(ToSystemCharset(name), iomode == READ ? O_RDONLY :
	                    iomode == CREATE ? O_CREAT|O_RDWR|O_TRUNC :
	                    O_RDWR|O_CREAT,
	              tmode);
	if(handle >= 0) {
		struct stat st[1];
		fstat(handle, st);
		if(!(st->st_mode & S_IFREG) ||  // not a regular file, e.g. folder - bad things would happen
		   (mode & NOWRITESHARE) && flock(handle, LOCK_EX|LOCK_NB) < 0) { // lock if not sharing
			close(handle);
			handle = -1;
			return false;
		}
		int64 fsz = st->st_size;
		if(fsz >= 0) {
			OpenInit(mode, fsz);
			LLOG("OPEN handle " << handle);
			return true;
		}
	}
	SetLastError();
	return false;
}

void FileStream::Close() {
	if(!IsOpen()) return;
	LLOG("CLOSE handle " << handle);
	Flush();
	if(close(handle) < 0)
		SetLastError();
	handle = -1;
}

bool FileStream::IsOpen() const {
	return handle != -1;
}

FileStream::FileStream(const char *filename, dword mode, mode_t acm) {
	handle = -1;
	Open(filename, mode, acm);
}

FileStream::FileStream() {
	handle = -1;
}

FileStream::~FileStream() {
	Close();
}

bool FileOut::Open(const char *fn, mode_t acm)
{
	return FileStream::Open(fn, FileStream::CREATE|FileStream::NOWRITESHARE, acm);
}

#endif

}
