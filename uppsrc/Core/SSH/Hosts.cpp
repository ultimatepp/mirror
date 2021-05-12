#include "SSH.h"

namespace Upp {
	
#define LLOG(x)       do { if(SSH::sTrace) RLOG("SshHosts: " << x); } while(false)
	
bool SshHosts::Add(const String& host, int port, const Info& info, const String& comment)
{
	return Add(Format("[%s]:%d", host, port), info, comment);
}

bool SshHosts::Add(const String& host, const Info& info, const String& comment)
{
	ASSERT(ssh_session);
	Clear();
	bool b = handle &&
		libssh2_knownhost_addc(
			handle,
			~host,
			nullptr,
			~info.key, info.key.GetLength(),
			~comment, comment.GetLength(),
			info.type |
			LIBSSH2_KNOWNHOST_TYPE_PLAIN |
			LIBSSH2_KNOWNHOST_KEYENC_RAW,
			nullptr
		) == 0;
	return b ? b : Error();
}

bool SshHosts::Remove(SshHost* host)
{
	ASSERT(ssh_session);
	Clear();
	auto b = handle && libssh2_knownhost_del(handle, host) == 0;
	return b ? b : Error();
}

bool SshHosts::Load(const String& filename)
{
	ASSERT(ssh_session);
	Clear();
	file_path = filename;
	auto b = libssh2_knownhost_readfile(handle, ~file_path, LIBSSH2_KNOWNHOST_FILE_OPENSSH) >= 0;
	return b ? b : Error();
}

bool SshHosts::Save()
{
	return SaveAs(file_path);
}

bool SshHosts::SaveAs(const String& filename)
{
	ASSERT(ssh_session);
	Clear();
	auto b = handle && libssh2_knownhost_writefile(handle, ~filename, LIBSSH2_KNOWNHOST_FILE_OPENSSH) == 0;
	return b ? b : Error();
}

SshHosts::Info SshHosts::Check(const String& host, int port)
{
	ASSERT(ssh_session);
	Clear();
	Info info;
	if(handle) {
		int	   type   = 0;
		size_t length = 0;
		auto*  p = libssh2_session_hostkey(ssh_session, &length, &type);
		if(!p) {
			Error();
			info.status = LIBSSH2_KNOWNHOST_CHECK_FAILURE;
			return pick(info);
		}
		info.status = libssh2_knownhost_checkp(
			handle,
			~host,
			port,
			p,
			length,
			LIBSSH2_KNOWNHOST_TYPE_PLAIN |
			LIBSSH2_KNOWNHOST_KEYENC_RAW,
			nullptr
		);
		info.key.Set(p, length);
		switch(type) {
			case LIBSSH2_HOSTKEY_TYPE_RSA:
				info.type = LIBSSH2_KNOWNHOST_KEY_SSHRSA;
				break;
			case LIBSSH2_HOSTKEY_TYPE_DSS:
				info.type = LIBSSH2_KNOWNHOST_KEY_SSHDSS;
				break;
			case LIBSSH2_HOSTKEY_TYPE_ECDSA_256:
				info.type = LIBSSH2_KNOWNHOST_KEY_ECDSA_256;
				break;
			case LIBSSH2_HOSTKEY_TYPE_ECDSA_384:
				info.type = LIBSSH2_KNOWNHOST_KEY_ECDSA_384;
				break;
			case LIBSSH2_HOSTKEY_TYPE_ECDSA_521:
				info.type = LIBSSH2_KNOWNHOST_KEY_ECDSA_521;
				break;
			case LIBSSH2_HOSTKEY_TYPE_ED25519:
				info.type = LIBSSH2_KNOWNHOST_KEY_ED25519;
				break;
			case LIBSSH2_HOSTKEY_TYPE_UNKNOWN:
			default:
				info.type = LIBSSH2_KNOWNHOST_KEY_UNKNOWN;
				break;
		}
	}
	return pick(info);
}

Vector<SshHost*> SshHosts::GetHosts()
{
	ASSERT(ssh_session);
	Clear();
	Vector<SshHost*> v;
	SshHost *prev = nullptr, *next = nullptr;
	int rc = libssh2_knownhost_get(handle, &prev, nullptr);
	if(rc >= 0) {
		v.Add(prev);
		if(rc == 0)
			while(rc < 1) {
				rc = libssh2_knownhost_get(handle, &next, prev);
				if(rc < 0) break;
				v.Add(next);
				prev = next;
			}
	}
	if(rc < 0) Error();
	return pick(v);
}

bool SshHosts::Error()
{
	ASSERT(ssh_session);
	Buffer<char*> libmsg(256, 0);
	int rc = libssh2_session_last_error(ssh_session, libmsg, nullptr, 0);
	error.a = rc;
	error.b = String(*libmsg);
	LLOG("Failed. " << error.b);
	return false;
}

SshHosts::SshHosts(SshSession& session)
{
	ASSERT(session.GetHandle());
	ssh_session   = session.GetHandle();
	handle = libssh2_knownhost_init(ssh_session);
}

SshHosts::~SshHosts()
{
	if(ssh_session && handle)
		libssh2_knownhost_free(handle);
}
 
bool SshHosts::Info::IsUnknown() const
{
	return findarg(type,
		LIBSSH2_KNOWNHOST_KEY_SSHRSA,
		LIBSSH2_KNOWNHOST_KEY_SSHDSS,
		LIBSSH2_KNOWNHOST_KEY_ECDSA_256,
		LIBSSH2_KNOWNHOST_KEY_ECDSA_384,
		LIBSSH2_KNOWNHOST_KEY_ECDSA_521,
		LIBSSH2_KNOWNHOST_KEY_ED25519) < 0;
}
}