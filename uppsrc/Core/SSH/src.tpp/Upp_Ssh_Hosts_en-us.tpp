topic "Known Hosts";
[i448;a25;kKO9;2 $$1,0#37138531426314131252341829483380:class]
[l288;2 $$2,2#27521748481378242620020725143825:desc]
[0 $$3,0#96390100711032703541132217272105:end]
[H6;0 $$4,0#05600065144404261032431302351956:begin]
[i448;a25;kKO9;2 $$5,0#37138531426314131252341829483370:item]
[l288;a4;*@5;1 $$6,6#70004532496200323422659154056402:requirement]
[l288;i1121;b17;O9;~~~.1408;2 $$7,0#10431211400427159095818037425705:param]
[i448;b42;O9;2 $$8,8#61672508125594000341940100500538:tparam]
[b42;2 $$9,9#13035079074754324216151401829390:normal]
[2 $$0,0#00000000000000000000000000000000:Default]
[{_} 
[ {{10000@(113.42.0) [s0;%% [*@7;4 SshHosts]]}}&]
[s3; &]
[s1;:Upp`:`:SshHosts`:`:class: [@(0.0.255)3 class][3 _][*3 SshHosts]&]
[s2;#%% This class encapsulates a list of known hosts which can be 
used to verify the identity of a remote ssh server, and provides 
methods for managing the list. Currently only OpenSSH known hosts 
file format is supported.  &]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Public Method List]]}}&]
[s3; &]
[s5;:Upp`:`:SshHosts`:`:Add`(const Upp`:`:String`&`,int`,const Upp`:`:SshHosts`:`:Info`&`,const Upp`:`:String`&`): [@(0.0.255) b
ool]_[* Add]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 host], 
[@(0.0.255) const]_[_^topic`:`/`/SSH`/src`/Upp`_Ssh`_Hosts`$en`-us`#Upp`:`:KnownHosts`:`:Info`:`:struct^ S
shHosts`::Info][@(0.0.255) `&]_[*@3 info], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&
]_[*@3 comment])&]
[s5;:Upp`:`:SshHosts`:`:Add`(const Upp`:`:String`&`,const Upp`:`:SshHosts`:`:Info`&`,const Upp`:`:String`&`): [@(0.0.255) b
ool]_[* Add]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 host], 
[@(0.0.255) int]_[*@3 port], [@(0.0.255) const]_[_^topic`:`/`/SSH`/src`/Upp`_Ssh`_Hosts`$en`-us`#Upp`:`:KnownHosts`:`:Info`:`:struct^ S
shHosts`::Info][@(0.0.255) `&]_[*@3 info], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&
]_[*@3 comment])&]
[s2;%% Adds a [%-*@3 host] to the known hosts list. Usually after verification. 
Returns true on success. &]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Remove`(Upp`:`:SshHost`*`): [@(0.0.255) bool]_[* Remove]([_^Upp`:`:SshHost^ S
shHost][@(0.0.255) `*]_[*@3 host])&]
[s2;%% Removes a [%-*@3 host] from the known hosts list. Returns true 
on success.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Load`(const Upp`:`:String`&`): [@(0.0.255) bool]_[* Load]([@(0.0.255) c
onst]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 filename])&]
[s2;%% Loads a list of known hosts from a specified file and adds 
them to the list of known hosts. Returns true on success.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Save`(`): [@(0.0.255) bool]_[* Save]()&]
[s2;%% Saves the list of known host to a previously loaded file. 
Returns true on success. . &]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:SaveAs`(const Upp`:`:String`&`): [@(0.0.255) bool]_[* SaveAs]([@(0.0.255) c
onst]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 filename])&]
[s2;%% Saves the list of known hosts to another file. Returns true 
on success. &]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Check`(const Upp`:`:String`&`,int`): [_^topic`:`/`/SSH`/src`/Upp`_Ssh`_Hosts`$en`-us`#Upp`:`:KnownHosts`:`:Info`:`:struct^ S
shHosts`::Info]_[* Check]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 h
ost], [@(0.0.255) int]_[*@3 port])&]
[s2;%% Checks a [%-*@3 host ]and its key against a list of known hosts. 
Returns a [%-_^topic`:`/`/SSH`/src`/Upp`_Ssh`_Hosts`$en`-us`#Upp`:`:KnownHosts`:`:Info`:`:struct^ S
shHosts`::Info] structure.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:GetHosts`(`): [_^Upp`:`:Vector^ Vector]<SshHost[@(0.0.255) `*]>_[* G
etHosts]()&]
[s2;%% Returns a list of known hosts. Note that an empty list doesn`'t 
necessarily mean error.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:GetError`(`)const: [@(0.0.255) int]_[* GetError]()_[@(0.0.255) cons
t]&]
[s2;%% Returns the last error code.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:GetErrorDesc`(`)const: [_^Upp`:`:String^ String]_[* GetErrorDesc](
)_[@(0.0.255) const]&]
[s2;%% Returns the decription of last error.&]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Constructor detail]]}}&]
[s3; &]
[s5;:Upp`:`:SshHosts`:`:SshHosts`(Upp`:`:SshSession`&`): [* SshHosts]([_^Upp`:`:Ssh^ SshS
ession][@(0.0.255) `&]_[*@3 session])&]
[s2;%% Constructor. Binds the SshHosts instance to [%-*@3 session]. 
&]
[s0; &]
[ {{10000@(113.42.0) [s0;%% [*@7;4 SshHosts`::Info]]}}&]
[s0;%% &]
[s1;:Upp`:`:SshHosts`:`:Info`:`:struct: [@(0.0.255)3 struct][3 _][*3 Info]&]
[s2;%% This helper structure is intended to simplify known host queries. 
&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Info`:`:key: [_^Upp`:`:String^ String]_[* key]&]
[s2;%% Public key of the given host. &]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Info`:`:IsRSA`(`)const: [@(0.0.255) bool]_[* IsRSA]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if the public key of the given host is using 
RSA algorithm.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Info`:`:IsDSS`(`)const: [@(0.0.255) bool]_[* IsDSS]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if the public key of the given host is using 
DSS algorithm.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Info`:`:IsECDSA256`(`)const: [@(0.0.255) bool]_[* IsECDSA256]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if the public key of the given host is using 
ECDSA 256 algorithm.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Info`:`:IsECDSA384`(`)const: [@(0.0.255) bool]_[* IsECDSA384]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if the public key of the given host is using 
ECDSA 384 algorithm.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Info`:`:IsECDSA521`(`)const: [@(0.0.255) bool]_[* IsECDSA521]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if the public key of the given host is using 
ECDSA 521 algorithm.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Info`:`:IsED25519`(`)const: [@(0.0.255) bool]_[* IsED25519]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if the public key of the given host is using 
ED 25519 algorithm.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Info`:`:IsUnknown`(`)const: [@(0.0.255) bool]_[* IsUnknown]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if the public key of the given host is of unknown 
type.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Info`:`:IsFailure`(`)const: [@(0.0.255) bool]_[* IsFailure]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if something prevented the check to be made. 
&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Info`:`:IsNotFound`(`)const: [@(0.0.255) bool]_[* IsNotFound]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if  no host match was found.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Info`:`:IsMismatch`(`)const: [@(0.0.255) bool]_[* IsMismatch]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if the host was found, but the keys didn`'t match. 
(This [/ may ]mean an attack!) &]
[s3; &]
[s4; &]
[s5;:Upp`:`:SshHosts`:`:Info`:`:IsMatch`(`)const: [@(0.0.255) bool]_[* IsMatch]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if the hosts and the keys match.&]
[s3; &]
[s0;%% ]]