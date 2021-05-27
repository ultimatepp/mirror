#include "Core.h"

#ifdef PLATFORM_WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifdef PLATFORM_POSIX
#include <arpa/inet.h>
#endif

namespace Upp {

#ifdef PLATFORM_WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

#define LLOG(x)  // LOG("TCP " << x)

IpAddrInfo::Entry IpAddrInfo::pool[IpAddrInfo::COUNT];

Mutex IpAddrInfoPoolMutex;

void IpAddrInfo::EnterPool()
{
	IpAddrInfoPoolMutex.Enter();
}

void IpAddrInfo::LeavePool()
{
	IpAddrInfoPoolMutex.Leave();
}

int sGetAddrInfo(const char *host, const char *port, int family, addrinfo **result)
{
	if(!host || !*host)
		return EAI_NONAME;
	addrinfo hints;
	memset(&hints, 0, sizeof(addrinfo));
	const static int FamilyToAF[] = { AF_UNSPEC, AF_INET, AF_INET6 };
	hints.ai_family = FamilyToAF[(family > 0 && family < __countof(FamilyToAF)) ? family : 0];
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	return getaddrinfo(host, port, &hints, result);
}

auxthread_t auxthread__ IpAddrInfo::Thread(void *ptr)
{
	Entry *entry = (Entry *)ptr;
	EnterPool();
	if(entry->status == WORKING) {
		char host[1025];
		char port[257];
		int family = entry->family;
		strcpy(host, entry->host);
		strcpy(port, entry->port);
		LeavePool();
		addrinfo *result;
		if(sGetAddrInfo(host, port, family, &result) == 0 && result) {
			EnterPool();
			if(entry->status == WORKING) {
				entry->addr = result;
				entry->status = RESOLVED;
			}
			else {
				freeaddrinfo(result);
				entry->status = EMPTY;
			}
		}
		else {
			EnterPool();
			if(entry->status == CANCELED)
				entry->status = EMPTY;
			else
				entry->status = FAILED;
		}
	}
	LeavePool();
	return 0;
}

bool IpAddrInfo::Execute(const String& host, int port, int family)
{
	Clear();
	entry = exe;
	addrinfo *result;
	entry->addr = sGetAddrInfo(~host, ~AsString(port), family, &result) == 0 ? result : NULL;
	entry->status = entry->addr ? RESOLVED : FAILED;
	return entry->addr;
}

void IpAddrInfo::Start()
{
	if(entry)
		return;
	EnterPool();
	for(int i = 0; i < COUNT; i++) {
		Entry *e = pool + i;
		if(e->status == EMPTY) {
			entry = e;
			e->addr = NULL;
			if(host.GetCount() > 1024 || port.GetCount() > 256)
				e->status = FAILED;
			else {
				e->status = WORKING;
				e->host = host;
				e->port = port;
				e->family = family;
				StartAuxThread(&IpAddrInfo::Thread, e);
			}
			break;
		}
	}
	LeavePool();
}

void IpAddrInfo::Start(const String& host_, int port_, int family_)
{
	Clear();
	port = AsString(port_);
	host = host_;
	family = family_;
	Start();
}

bool IpAddrInfo::InProgress()
{
	if(!entry) {
		Start();
		return true;
	}
	EnterPool();
	int s = entry->status;
	LeavePool();
	return s == WORKING;
}

addrinfo *IpAddrInfo::GetResult() const
{
	EnterPool();
	addrinfo *ai = entry ? entry->addr : NULL;
	LeavePool();
	return ai;
}

void IpAddrInfo::Clear()
{
	EnterPool();
	if(entry) {
		if(entry->status == RESOLVED && entry->addr)
			freeaddrinfo(entry->addr);
		if(entry->status == WORKING)
			entry->status = CANCELED;
		else
			entry->status = EMPTY;
		entry = NULL;
	}
	LeavePool();
}

IpAddrInfo::IpAddrInfo()
{
	TcpSocket::Init();
	entry = NULL;
}

#ifdef PLATFORM_POSIX

#define SOCKERR(x) x

const char *TcpSocketErrorDesc(int code)
{
	return strerror(code);
}

int TcpSocket::GetErrorCode()
{
	return errno;
}

#else

#define SOCKERR(x) WSA##x

const char *TcpSocketErrorDesc(int code)
{
	static Tuple<int, const char *> err[] = {
		{ WSAEINTR,                 "Interrupted function call." },
		{ WSAEACCES,                "Permission denied." },
		{ WSAEFAULT,                "Bad address." },
		{ WSAEINVAL,                "Invalid argument." },
		{ WSAEMFILE,                "Too many open files." },
		{ WSAEWOULDBLOCK,           "Resource temporarily unavailable." },
		{ WSAEINPROGRESS,           "Operation now in progress." },
		{ WSAEALREADY,              "Operation already in progress." },
		{ WSAENOTSOCK,              "TcpSocket operation on nonsocket." },
		{ WSAEDESTADDRREQ,          "Destination address required." },
		{ WSAEMSGSIZE,              "Message too long." },
		{ WSAEPROTOTYPE,            "Protocol wrong type for socket." },
		{ WSAENOPROTOOPT,           "Bad protocol option." },
		{ WSAEPROTONOSUPPORT,       "Protocol not supported." },
		{ WSAESOCKTNOSUPPORT,       "TcpSocket type not supported." },
		{ WSAEOPNOTSUPP,            "Operation not supported." },
		{ WSAEPFNOSUPPORT,          "Protocol family not supported." },
		{ WSAEAFNOSUPPORT,          "Address family not supported by protocol family." },
		{ WSAEADDRINUSE,            "Address already in use." },
		{ WSAEADDRNOTAVAIL,         "Cannot assign requested address." },
		{ WSAENETDOWN,              "Network is down." },
		{ WSAENETUNREACH,           "Network is unreachable." },
		{ WSAENETRESET,             "Network dropped connection on reset." },
		{ WSAECONNABORTED,          "Software caused connection abort." },
		{ WSAECONNRESET,            "Connection reset by peer." },
		{ WSAENOBUFS,               "No buffer space available." },
		{ WSAEISCONN,               "TcpSocket is already connected." },
		{ WSAENOTCONN,              "TcpSocket is not connected." },
		{ WSAESHUTDOWN,             "Cannot send after socket shutdown." },
		{ WSAETIMEDOUT,             "Connection timed out." },
		{ WSAECONNREFUSED,          "Connection refused." },
		{ WSAEHOSTDOWN,             "Host is down." },
		{ WSAEHOSTUNREACH,          "No route to host." },
		{ WSAEPROCLIM,              "Too many processes." },
		{ WSASYSNOTREADY,           "Network subsystem is unavailable." },
		{ WSAVERNOTSUPPORTED,       "Winsock.dll version out of range." },
		{ WSANOTINITIALISED,        "Successful WSAStartup not yet performed." },
		{ WSAEDISCON,               "Graceful shutdown in progress." },
		{ WSATYPE_NOT_FOUND,        "Class type not found." },
		{ WSAHOST_NOT_FOUND,        "Host not found." },
		{ WSATRY_AGAIN,             "Nonauthoritative host not found." },
		{ WSANO_RECOVERY,           "This is a nonrecoverable error." },
		{ WSANO_DATA,               "Valid name, no data record of requested type." },
		{ WSASYSCALLFAILURE,        "System call failure." },
	};
	const Tuple<int, const char *> *x = FindTuple(err, __countof(err), code);
	return x ? x->b : "Unknown error code.";
}

int TcpSocket::GetErrorCode()
{
	return WSAGetLastError();
}

#endif

void TcpSocketInit()
{
#ifdef PLATFORM_WIN32
	ONCELOCK {
		WSADATA wsadata;
		WSAStartup(MAKEWORD(2, 2), &wsadata);
	}
#endif
#ifdef PLATFORM_POSIX
	signal(SIGPIPE, SIG_IGN);
#endif
}

void TcpSocket::Init()
{
	TcpSocketInit();
}

void TcpSocket::Reset()
{
	LLOG("Reset");
	is_eof = false;
	socket = INVALID_SOCKET;
	ipv6 = false;
	ptr = end = buffer;
	is_error = false;
	is_abort = false;
	is_timeout = false;
	mode = NONE;
	ssl.Clear();
	sslinfo.Clear();
	cert = pkey = Null;
#if defined(PLATFORM_WIN32) || defined(PLATFORM_BSD)
	connection_start = Null;
#endif
	ssl_start = Null;
}

TcpSocket::TcpSocket()
{
	ClearError();
	Reset();
	timeout = global_timeout = start_time = Null;
	waitstep = 10;
	asn1 = false;
}

bool TcpSocket::SetupSocket()
{
#ifdef PLATFORM_WIN32
	connection_start = msecs();
	u_long arg = 1;
	if(ioctlsocket(socket, FIONBIO, &arg)) {
		SetSockError("ioctlsocket(FIO[N]BIO)");
		return false;
	}
#else
	#ifdef PLATFORM_BSD
		connection_start = msecs();
	#endif
	if(fcntl(socket, F_SETFL, (fcntl(socket, F_GETFL, 0) | O_NONBLOCK))) {
		SetSockError("fcntl(O_[NON]BLOCK)");
		return false;
	}
#endif
	return true;
}

bool TcpSocket::Open(int family, int type, int protocol)
{
	Init();
	Close();
	ClearError();
	if((socket = ::socket(family, type, protocol)) == INVALID_SOCKET) {
		SetSockError("open");
		return false;
	}
	LLOG("TcpSocket::Data::Open() -> " << (int)socket);
	return SetupSocket();
}

bool TcpSocket::Listen(int port, int listen_count, bool ipv6_, bool reuse, void *addr)
{
	Close();
	Init();
	Reset();

	ipv6 = ipv6_;
	if(!Open(ipv6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0))
		return false;
	sockaddr_in sin;
#ifdef PLATFORM_WIN32
	SOCKADDR_IN6 sin6;
	if(ipv6 && IsWinVista())
#else
	sockaddr_in6 sin6;
	if(ipv6)
#endif
	{
		Zero(sin6);
		sin6.sin6_family = AF_INET6;
		sin6.sin6_port = htons(port);
		sin6.sin6_addr = addr?(*(in6_addr*)addr):in6addr_any;
	}
	else {
		Zero(sin);
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.s_addr = addr?(*(uint32*)addr):htonl(INADDR_ANY);
	}
	if(reuse) {
		int optval = 1;
		setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&optval, sizeof(optval));
	}
	if(bind(socket, ipv6 ? (const sockaddr *)&sin6 : (const sockaddr *)&sin,
	        ipv6 ? sizeof(sin6) : sizeof(sin))) {
		SetSockError(Format("bind(port=%d)", port));
		return false;
	}
	if(listen(socket, listen_count)) {
		SetSockError(Format("listen(port=%d, count=%d)", port, listen_count));
		return false;
	}
	return true;
}

bool TcpSocket::Listen(const IpAddrInfo& addr, int port, int listen_count, bool ipv6, bool reuse)
{
	addrinfo *a = addr.GetResult();
	return Listen(port, listen_count, ipv6, reuse, &(((sockaddr_in*)a->ai_addr)->sin_addr.s_addr));
}

bool TcpSocket::Accept(TcpSocket& ls)
{
	Close();
	Init();
	Reset();
	ASSERT(ls.IsOpen());
	int et = GetEndTime();
	for(;;) {
		int h = ls.GetTimeout();
		bool b = ls.Timeout(timeout).Wait(WAIT_READ, et);
		ls.Timeout(h);
		if(!b) // timeout
			return false;
		socket = accept(ls.GetSOCKET(), NULL, NULL);
		if(socket != INVALID_SOCKET)
			break;
		if(!WouldBlock() && GetErrorCode() != SOCKERR(EINTR)) { // In prefork condition, Wait is not enough, as other process can accept
			SetSockError("accept");
			return false;
		}
	}
	mode = ACCEPT;
	return SetupSocket();
}

String TcpSocket::GetPeerAddr() const
{
	if(!IsOpen())
		return Null;
	sockaddr_in addr;
	socklen_t l = sizeof(addr);
	if(getpeername(socket, (sockaddr *)&addr, &l) != 0)
		return Null;
	if(l > sizeof(addr))
		return Null;
#ifdef PLATFORM_WIN32
	return inet_ntoa(addr.sin_addr);
#else
	char h[200];
	return inet_ntop(AF_INET, &addr.sin_addr, h, 200);
#endif
}

void TcpSocket::NoDelay()
{
	ASSERT(IsOpen());
	int __true = 1;
	LLOG("NoDelay(" << (int)socket << ")");
	if(setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (const char *)&__true, sizeof(__true)))
		SetSockError("setsockopt(TCP_NODELAY)");
}

void TcpSocket::Linger(int msecs)
{
	ASSERT(IsOpen());
	linger ls;
	ls.l_onoff = !IsNull(msecs) ? 1 : 0;
	ls.l_linger = !IsNull(msecs) ? (msecs + 999) / 1000 : 0;
	if(setsockopt(socket, SOL_SOCKET, SO_LINGER, reinterpret_cast<const char *>(&ls), sizeof(ls)))
		SetSockError("setsockopt(SO_LINGER)");
}

void TcpSocket::Attach(SOCKET s)
{
	Close();
	socket = s;
}

bool TcpSocket::RawConnect(addrinfo *arp)
{
	if(!arp) {
		SetSockError("connect", -1, "not found");
		return false;
	}
	String err;
	for(int pass = 0; pass < 2; pass++) {
		addrinfo *rp = arp;
		while(rp) {
			if(rp->ai_family == AF_INET == !pass && // Try to connect IPv4 in the first pass
			   Open(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) {
				if(connect(socket, rp->ai_addr, (int)rp->ai_addrlen) == 0 ||
				   GetErrorCode() == SOCKERR(EINPROGRESS) || GetErrorCode() == SOCKERR(EWOULDBLOCK)
				) {
					mode = CONNECT;
					return true;
				}
				if(err.GetCount())
					err << '\n';
				err << TcpSocketErrorDesc(GetErrorCode());
				Close();
			}
			rp = rp->ai_next;
		}
    }
	SetSockError("connect", -1, Nvl(err, "failed"));
	return false;
}


bool TcpSocket::Connect(IpAddrInfo& info)
{
	LLOG("Connect addrinfo");
	Init();
	Reset();
	addrinfo *result = info.GetResult();
	return RawConnect(result);
}

bool TcpSocket::Connect(const char *host, int port)
{
	LLOG("Connect(" << host << ':' << port << ')');
	Close();
	Init();
	Reset();
	IpAddrInfo info;
	if(!info.Execute(host, port)) {
		SetSockError(Format("getaddrinfo(%s) failed", host));
		return false;
	}
	return Connect(info);
}

bool TcpSocket::WaitConnect()
{
	if(WaitWrite()) {
		int optval = 0;
		socklen_t optlen = sizeof(optval);
		if (getsockopt(GetSOCKET(), SOL_SOCKET, SO_ERROR, (char*)&optval, &optlen) == 0) {
			if (optval == 0)
				return true;
			else {
				SetSockError("wait connect", -1, Nvl(String(TcpSocketErrorDesc(optval)), "failed"));
				return false;
			}
		}
	}
	return false;
}

void TcpSocket::RawClose()
{
	LLOG("close " << (int)socket);
	if(socket != INVALID_SOCKET) {
		int res;
#if defined(PLATFORM_WIN32)
		res = closesocket(socket);
#elif defined(PLATFORM_POSIX)
		res = close(socket);
#else
	#error Unsupported platform
#endif
		if(res && !IsError())
			SetSockError("close");
		socket = INVALID_SOCKET;
	}
}

void TcpSocket::Close()
{
	if(ssl)
		ssl->Close();
	else
		RawClose();
	ssl.Clear();
}

bool TcpSocket::WouldBlock()
{
	int c = GetErrorCode();
#ifdef PLATFORM_POSIX
#ifdef PLATFORM_BSD
		if(c == SOCKERR(ENOTCONN) && !IsNull(connection_start) && msecs(connection_start) < 20000)
			return true;
#endif
	return c == SOCKERR(EWOULDBLOCK) || c == SOCKERR(EAGAIN);
#endif
#ifdef PLATFORM_WIN32
	if(c == SOCKERR(ENOTCONN) && !IsNull(connection_start) && msecs(connection_start) < 20000) {
		LLOG("ENOTCONN issue");
		return true;
	}
	return c == SOCKERR(EWOULDBLOCK);	       
#endif
}

int TcpSocket::RawSend(const void *buf, int amount)
{
#ifdef PLATFORM_POSIX
	int res = send(socket, (const char *)buf, amount, MSG_NOSIGNAL);
#else
	int res = send(socket, (const char *)buf, amount, 0);
#endif
	if(res < 0 && WouldBlock())
		res = 0;
	else
	if(res == 0 || res < 0)
		SetSockError("send");
	return res;
}

int TcpSocket::Send(const void *buf, int amount)
{
	if(SSLHandshake())
		return 0;
	return ssl ? ssl->Send(buf, amount) : RawSend(buf, amount);
}

void TcpSocket::Shutdown()
{
	ASSERT(IsOpen());
	if(shutdown(socket, SD_SEND))
		SetSockError("shutdown(SD_SEND)");
}

String TcpSocket::GetHostName()
{
	Init();
	char buffer[256];
	gethostname(buffer, __countof(buffer));
	return buffer;
}

bool TcpSocket::IsGlobalTimeout()
{
	if(!IsNull(global_timeout) && msecs() - start_time > global_timeout) {
		SetSockError("wait", ERROR_GLOBAL_TIMEOUT, "Timeout");
		return true;
	}
	return false;
}

bool TcpSocket::RawWait(dword flags, int end_time)
{ // wait till end_time
	LLOG("RawWait end_time: " << end_time << ", current time " << msecs() << ", to wait: " << end_time - msecs());
	is_timeout = false;
	if((flags & WAIT_READ) && ptr != end)
		return true;
	if(socket == INVALID_SOCKET)
		return false;
	for(;;) {
		if(IsError() || IsAbort())
			return false;
		int to = end_time - msecs();
		if(WhenWait)
			to = waitstep;
		timeval *tvalp = NULL;
		timeval tval;
		if(end_time != INT_MAX || WhenWait) {
			to = max(to, 0);
			tval.tv_sec = to / 1000;
			tval.tv_usec = 1000 * (to % 1000);
			tvalp = &tval;
			LLOG("RawWait timeout: " << to);
		}
		fd_set fdsetr[1], fdsetw[1], fdsetx[1];;
		FD_ZERO(fdsetr);
		if(flags & WAIT_READ)
			FD_SET(socket, fdsetr);
		FD_ZERO(fdsetw);
		if(flags & WAIT_WRITE)
			FD_SET(socket, fdsetw);
		FD_ZERO(fdsetx);
		FD_SET(socket, fdsetx);
		int avail = select((int)socket + 1, fdsetr, fdsetw, fdsetx, tvalp);
		LLOG("Wait select avail: " << avail);
		if(avail < 0 && GetErrorCode() != SOCKERR(EINTR)) {
			SetSockError("wait");
			return false;
		}
		if(avail > 0) {
		#if defined(PLATFORM_WIN32) || defined(PLATFORM_BSD)
			connection_start = Null;
		#endif
			return true;
		}
		if(IsGlobalTimeout() || to <= 0 && timeout) {
			is_timeout = true;
			return false;
		}
		WhenWait();
		if(timeout == 0) {
			is_timeout = true;
			return false;
		}
	}
}

TcpSocket& TcpSocket::GlobalTimeout(int ms)
{
	start_time = msecs();
	global_timeout = ms;
	return *this;
}

bool TcpSocket::Wait(dword flags, int end_time)
{
	return ssl ? ssl->Wait(flags, end_time) : RawWait(flags, end_time);
}

int  TcpSocket::GetEndTime() const
{ // Compute time limit for operation, based on global timeout and per-operation timeout settings
	int o = min(IsNull(global_timeout) ? INT_MAX : start_time + global_timeout,
	            IsNull(timeout) ? INT_MAX : msecs() + timeout);
#if defined(PLATFORM_WIN32) || defined(PLATFORM_BSD)
	if(GetErrorCode() == SOCKERR(ENOTCONN) && !IsNull(connection_start))
		if(msecs(connection_start) < 20000)
			o = connection_start + 20000;
#endif
	return o;
}

bool TcpSocket::Wait(dword flags)
{
	return Wait(flags, GetEndTime());
}

int TcpSocket::Put(const void *s_, int length)
{
	LLOG("Put " << socket << ": " << length);
	ASSERT(IsOpen());
	const char *s = (const char *)s_;
	if(length < 0 && s)
		length = (int)strlen(s);
	if(!s || length <= 0 || IsError() || IsAbort())
		return 0;
	done = 0;
	bool peek = false;
	int end_time = GetEndTime();
	while(done < length) {
		if(peek && !Wait(WAIT_WRITE, end_time))
			return done;
		peek = false;
		int count = Send(s + done, length - done);
		if(IsError() || timeout == 0 && count == 0 && peek)
			return done;
		if(count > 0)
			done += count;
		else
			peek = true;
	}
	LLOG("//Put() -> " << done);
	return done;
}

bool TcpSocket::PutAll(const void *s, int len)
{
	if(Put(s, len) != len) {
		if(!IsError())
			SetSockError("GePutAll", -1, "timeout");
		return false;
	}
	return true;
}

bool TcpSocket::PutAll(const String& s)
{
	if(Put(s) != s.GetCount()) {
		if(!IsError())
			SetSockError("GePutAll", -1, "timeout");
		return false;
	}
	return true;
}

int TcpSocket::RawRecv(void *buf, int amount)
{
	int res = recv(socket, (char *)buf, amount, 0);
	if(res == 0)
		is_eof = true;
	else
	if(res < 0 && WouldBlock())
		res = 0;
	else
	if(res < 0)
		SetSockError("recv");
	LLOG("recv(" << socket << "): " << res << " bytes: "
	     << AsCString((char *)buf, (char *)buf + min(res, 16))
	     << (res ? "" : IsEof() ? ", EOF" : ", WOULDBLOCK"));
	return res;
}

int TcpSocket::Recv(void *buffer, int maxlen)
{
	if(SSLHandshake())
		return 0;
	return ssl ? ssl->Recv(buffer, maxlen) : RawRecv(buffer, maxlen);
}

void TcpSocket::ReadBuffer(int end_time)
{
	ptr = end = buffer;
	if(Wait(WAIT_READ, end_time))
		end = buffer + Recv(buffer, BUFFERSIZE);
}

bool TcpSocket::IsEof() const
{
	return is_eof && ptr == end || IsAbort() || !IsOpen() || IsError();
}

int TcpSocket::Get_()
{
	if(!IsOpen() || IsError() || IsEof() || IsAbort())
		return -1;
	ReadBuffer(GetEndTime());
	return ptr < end ? (byte)*ptr++ : -1;
}

int TcpSocket::Peek_(int end_time)
{
	if(!IsOpen() || IsError() || IsEof() || IsAbort())
		return -1;
	ReadBuffer(end_time);
	return ptr < end ? (byte)*ptr : -1;
}

int TcpSocket::Peek_()
{
	return Peek_(GetEndTime());
}

int TcpSocket::Get(void *buffer, int count)
{
	LLOG("Get " << count);

	if(!IsOpen() || IsError() || IsEof() || IsAbort())
		return 0;
	
	int l = (int)(end - ptr);
	done = 0;
	if(l > 0) {
		if(l < count) {
			memcpy(buffer, ptr, l);
			done += l;
			ptr = end;
		}
		else {
			memcpy(buffer, ptr, count);
			ptr += count;
			return count;
		}
	}
	int end_time = GetEndTime();
	while(done < count && !IsError() && !IsEof()) {
		if(!Wait(WAIT_READ, end_time))
			break;
		int part = Recv((char *)buffer + done, count - done);
		if(part > 0)
			done += part;
		if(timeout == 0)
			break;
	}
	return done;
}

String TcpSocket::Get(int count)
{
	if(count == 0)
		return Null;
	StringBuffer out(count);
	int done = Get(out, count);
	if(!done && IsEof())
		return String::GetVoid();
	out.SetLength(done);
	return String(out);
}

bool  TcpSocket::GetAll(void *buffer, int len)
{
	if(Get(buffer, len) == len)
		return true;
	if(!IsError())
		SetSockError("GetAll", -1, "timeout");
	return false;
}

String TcpSocket::GetAll(int len)
{
	String s = Get(len);
	if(s.GetCount() != len) {
		if(!IsError())
			SetSockError("GetAll", -1, "timeout");
		return String::GetVoid();
	}
	return s;
}

String TcpSocket::GetLine(int maxlen)
{
	LLOG("GetLine " << maxlen << ", iseof " << IsEof());
	String ln;
	int end_time = GetEndTime();
	for(;;) {
		if(IsEof())
			return String::GetVoid();
		int c = Peek(end_time);
		if(c < 0) {
			if(!IsError()) {
				if(msecs() > end_time)
					SetSockError("GetLine", -1, "timeout");
				else
					continue;
			}
			return String::GetVoid();
		}
		Get();
		if(c == '\n')
			return ln;
		if(ln.GetCount() >= maxlen) {
			if(!IsError())
				SetSockError("GetLine", -1, "maximal length exceeded");
			return String::GetVoid();
		}
		if(c != '\r')
			ln.Cat(c);
	}
}

void TcpSocket::SetSockError(const char *context, int code, const char *errdesc)
{
	errorcode = code;
	errordesc.Clear();
	if(socket != INVALID_SOCKET)
		errordesc << "socket(" << (int)socket << ") / ";
	errordesc << context << ": " << errdesc;
	is_error = true;
	LLOG("ERROR " << errordesc);
}

void TcpSocket::SetSockError(const char *context, const char *errdesc)
{
	SetSockError(context, GetErrorCode(), errdesc);
}

void TcpSocket::SetSockError(const char *context)
{
	SetSockError(context, TcpSocketErrorDesc(GetErrorCode()));
}

TcpSocket::SSL *(*TcpSocket::CreateSSL)(TcpSocket& socket);

bool TcpSocket::StartSSL()
{
	ASSERT(IsOpen());
	if(!CreateSSL) {
		SetSockError("StartSSL", -1, "Missing SSL support (Core/SSL)");
		return false;
	}
	if(!IsOpen()) {
		SetSockError("StartSSL", -1, "Socket is not open");
		return false;
	}
	if(mode != CONNECT && mode != ACCEPT) {
		SetSockError("StartSSL", -1, "Socket is not connected");
		return false;
	}
	ssl = (*CreateSSL)(*this);
	if(!ssl->Start()) {
		ssl.Clear();
		return false;
	}
	ssl_start = msecs();
	SSLHandshake();
	return true;
}

dword TcpSocket::SSLHandshake()
{
	if(ssl && (mode == CONNECT || mode == ACCEPT)) {
		dword w = ssl->Handshake();
		if(w) {
			if(msecs(ssl_start) > 20000) {
				SetSockError("ssl handshake", ERROR_SSLHANDSHAKE_TIMEOUT, "Timeout");
				return false;
			}
			if(IsGlobalTimeout())
				return false;
			Wait(w);
			return ssl->Handshake();
		}
	}
	return 0;
}

void TcpSocket::SSLCertificate(const String& cert_, const String& pkey_, bool asn1_)
{
	cert = cert_;
	pkey = pkey_;
	asn1 = asn1_;
}

void TcpSocket::SSLServerNameIndication(const String& name)
{
	sni = name;
}

void TcpSocket::Clear()
{
	ClearError();
	if(IsOpen())
		Close();
	Reset();
}

int SocketWaitEvent::Wait(int timeout)
{
	FD_ZERO(read);
	FD_ZERO(write);
	FD_ZERO(exception);
	int maxindex = -1;
	for(int i = 0; i < socket.GetCount(); i++) {
		const Tuple<int, dword>& s = socket[i];
		if(s.a >= 0) {
			const Tuple<int, dword>& s = socket[i];
			if(s.b & WAIT_READ)
				FD_SET(s.a, read);
			if(s.b & WAIT_WRITE)
				FD_SET(s.a, write);
			FD_SET(s.a, exception);
			maxindex = max(s.a, maxindex);
		}
	}
	timeval *tvalp = NULL;
	timeval tval;
	if(!IsNull(timeout)) {
		tval.tv_sec = timeout / 1000;
		tval.tv_usec = 1000 * (timeout % 1000);
		tvalp = &tval;
	}
	return select(maxindex + 1, read, write, exception, tvalp);
}

dword SocketWaitEvent::Get(int i) const
{
	int s = socket[i].a;
	if(s < 0)
		return 0;
	dword events = 0;
	if(FD_ISSET(s, read))
		events |= WAIT_READ;
	if(FD_ISSET(s, write))
		events |= WAIT_WRITE;
	if(FD_ISSET(s, exception))
		events |= WAIT_IS_EXCEPTION;
	return events;
}

SocketWaitEvent::SocketWaitEvent()
{
	FD_ZERO(read);
	FD_ZERO(write);
	FD_ZERO(exception);
}

}
