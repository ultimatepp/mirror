class SshHosts {
public:
    struct Info {
        String          key;
        int             type;
        int             status;

        bool            IsRSA() const                   { return type == LIBSSH2_KNOWNHOST_KEY_SSHRSA;       }
        bool            IsDSS() const                   { return type == LIBSSH2_KNOWNHOST_KEY_SSHDSS;       }
        bool            IsECDSA256() const              { return type == LIBSSH2_KNOWNHOST_KEY_ECDSA_256;    }
        bool            IsECDSA384() const              { return type == LIBSSH2_KNOWNHOST_KEY_ECDSA_384;    }
        bool            IsECDSA521() const              { return type == LIBSSH2_KNOWNHOST_KEY_ECDSA_521;    }
        bool            IsED25519() const               { return type == LIBSSH2_KNOWNHOST_KEY_ED25519;      }
        bool            IsUnknown() const;

        bool            IsFailure() const               { return status == LIBSSH2_KNOWNHOST_CHECK_FAILURE;  }
        bool            IsNotFound() const              { return status == LIBSSH2_KNOWNHOST_CHECK_NOTFOUND; }
        bool            IsMismatch() const              { return status == LIBSSH2_KNOWNHOST_CHECK_MISMATCH; }
        bool            IsMatch() const                 { return status == LIBSSH2_KNOWNHOST_CHECK_MATCH;    }
    };

public:
    bool                Add(const String& host, int port, const Info& info, const String& comment);
    bool                Add(const String& host, const Info& info, const String& comment);
    bool                Remove(SshHost* host);
    bool                Load(const String& filename);
    bool                Save();
    bool                SaveAs(const String& filename);
    Info                Check(const String& host, int port);

    Vector<SshHost*>    GetHosts();

    int                 GetError() const                { return error.a; }
    String              GetErrorDesc() const            { return error.b; }

    SshHosts(SshSession& session);
    virtual ~SshHosts();

private:
    bool                Error();
    void                Clear()                         { error.a = 0; error.b = Null; }

    String              file_path;
    Tuple<int,String>   error;
    LIBSSH2_SESSION*    ssh_session;
    LIBSSH2_KNOWNHOSTS* handle;
};
