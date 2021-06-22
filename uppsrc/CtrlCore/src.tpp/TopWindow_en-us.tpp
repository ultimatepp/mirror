topic "TopWindow";
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
[ {{10000t/25b/25@3 [s0; [*@(229)4 TopWindow]]}}&]
[s3; &]
[s1;:TopWindow`:`:class: [@(0.0.255)3 class][3 _][*3 TopWindow][3 _:_][@(0.0.255)3 public][3 _][*@3;3 C
trl]&]
[s9;%% TopWindow represents top`-level independent window of host 
OS, usually decorated with caption, close/zoom/restore buttons 
etc.&]
[s9;%% Important fact to note is that window`-rectangle (as defined 
by Ctrl`::SetRect and Ctrl`::GetRect methods) does not include 
any host GUI specific decorations (caption, thick border for 
resizing), just the area that is `"controlled`" by U`+`+.&]
[s9;%% When TopWindow is opened, it is by default centered w.r.t. 
to its owner (if there is any). This behaviour can be altered 
by NoCenter or CenterScreen methods.&]
[s3;%% &]
[s9; [%%/ Derived from][%%  ][^topic`:`/`/CtrlCore`/src`/Ctrl`$en`-us`#`:`:Ctrl`:`:class^ C
trl]&]
[s3;%% &]
[s0;%% &]
[ {{10000F(128)G(128)@1 [s0;%% [* Public Member List]]}}&]
[s3; &]
[s5;:TopWindow`:`:Reject`(`): [@(0.0.255) virtual] [@(0.0.255) void]_[* Reject]()&]
[s2;%% This virtual method is overloaded to perform Restore operation 
if there was any previous call to Backup method.&]
[s3; &]
[s4; &]
[s5;:TopWindow`:`:Close`(`): [@(0.0.255) virtual] [@(0.0.255) void]_[* Close]()&]
[s2;%% This virtual method is implemented in TopWindow so that it 
breaks modal loop if TopWindow runs it. If there is Breaker with 
IDCANCEL defined, performs RejectBreak(IDCANCEL). Otherwise, 
if there is Breaker with IDYES (as assigned to exit Ctrl by CtrlLayoutExit), 
performs AcceptBreak(IDYES). If none if that is true, AcceptBreak(IDOK) 
is performed.&]
[s2;%% If TopWindow does not run modal loop, it is closed `- in this 
case, any state backup created by Backup method is scratched. 
Override in descendant class is allowed to call `'delete this`' 
as the very last thing it does (as long as the instance is allocated 
on the heap).&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:WhenClose: [_^Callback^ Event<>]_[* WhenClose]&]
[s2;%% This callback is by default invoked when Close button of host 
GUI window is pushed. Default assigned action is to perform Close 
virtual method. Assigned event is allowed to call `'delete this`' 
as the very last thing it does (as long as the instance is allocated 
on the heap).&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Backup`(`): [@(0.0.255) void]_[* Backup]()&]
[s2;%% Serializes the content of all contained Ctrls (using [^topic`:`/`/CtrlCore`/src`/Ctrl`$en`-us`#`:`:Ctrl`:`:Serialize`(`:`:Stream`&`)^ S
erialize] virtual method) into internal storage of TopWindow. 
Useful when implementing Cancel buttons.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Restore`(`): [@(0.0.255) void]_[* Restore]()&]
[s2;%% Restores content of all contained Ctrls from internal storage. 
If internal storage is empty, does nothing.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Break`(int`): [@(0.0.255) void]_[* Break]([@(0.0.255) int]_[*@3 ID]_`=_IDEX
IT)&]
[s2;%% If TopWindow is running its loop (see [^topic`:`/`/CtrlCore`/src`/TopWindow`$en`-us`#`:`:TopWindow`:`:Run`(bool`)^ R
un] or [^topic`:`/`/CtrlCore`/src`/TopWindow`$en`-us`#`:`:TopWindow`:`:RunAppModal`(`)^ R
unAppModal] methods), breaks the loop with specified exit code.&]
[s7;%% [%-*C@3 ID]-|Exit code.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:AcceptBreak`(int`): [@(0.0.255) bool]_[* AcceptBreak]([@(0.0.255) int]_[*@3 I
D])&]
[s2;%% Tries to accept all contained Ctrls by calling Accept for 
all child Ctrls, until some returns false `- in such case this 
method returns false. If all child Ctrls return true, calls Break 
with specified ID and returns true.&]
[s7;%% [%-*C@3 ID]-|Exit code.&]
[s7;%% [*/ Return value]-|True if all child Ctrls were accepted and 
Break(ID) was invoked.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:RejectBreak`(int`): [@(0.0.255) void]_[* RejectBreak]([@(0.0.255) int]_[*@3 I
D])&]
[s2;%% Rejects all contained widgets by calling Reject for all child 
Ctrls, then calls Break with specified ID.&]
[s7;%% [%-*C@3 ID]-|Exit code.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:WorkAreaTrim`(`): [@(0.0.255) void]_[* WorkAreaTrim]()&]
[s2;%% Adjusts window position so that it does not exceeds desktop 
workspace. Exceeding parts are trimmed (decreasing window size).&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Breaker`(int`): [_^Callback^ Callback]_[* Breaker]([@(0.0.255) int]_[*@3 ID
]_`=_IDEXIT)&]
[s2;%% Returns Callback that, when invoked, invokes [* Break] with 
the specified exit code. Note that Callback as return value is 
used for backward compatibility with pre`-C`+`+11 U`+`+ and for 
practical purposes, you could consider Event<> being returned.&]
[s7;%% [%-*C@3 ID]-|Message loop exit code.&]
[s7;%% [*/ Return value]-|Callback.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Acceptor`(int`): [_^Callback^ Callback]_[* Acceptor]([@(0.0.255) int]_[*@3 I
D])&]
[s2;%% Returns Callback that, when invoked, invokes [* AcceptBreak] 
with the specified exit code. Note that Callback as return value 
is used for backward compatibility with pre`-C`+`+11 U`+`+ and 
for practical purposes, you could consider Event<> being returned.&]
[s7;%% [%-*C@3 ID]-|Message loop exit code.&]
[s7;%% [*/ Return value]-|Callback.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Rejector`(int`): [_^Callback^ Callback]_[* Rejector]([@(0.0.255) int]_[*@3 I
D])&]
[s2;%% Returns Callback that, when invoked, invokes [* RejectBreak] 
with the specified exit code. Note that Callback as return value 
is used for backward compatibility with pre`-C`+`+11 U`+`+ and 
for practical purposes, you could consider Event<> being returned.&]
[s7;%% [%-*C@3 ID]-|Message loop exit code.&]
[s7;%% [*/ Return value]-|Callback.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Breaker`(Ctrl`&`,int`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* Breake
r]([_^Ctrl^ Ctrl][@(0.0.255) `&]_[*@3 m], [@(0.0.255) int]_[*@3 ID]_`=_`-[@3 1])&]
[s2;%% Same as m <<`= [* Breaker](ID) `- assigns [* Breaker] callback 
to WhenAction method of specified Ctrl.&]
[s7;%% [%-*C@3 m]-|Ctrl.&]
[s7;%% [%-*C@3 ID]-|Message loop exit code.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Acceptor`(Ctrl`&`,int`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* Accep
tor]([_^Ctrl^ Ctrl][@(0.0.255) `&]_[*@3 m], [@(0.0.255) int]_[*@3 ID])&]
[s2;%% Same as m <<`= [* Acceptor](ID) `- assigns [* Acceptor] callback 
to WhenAction method of specified Ctrl.&]
[s7;%% [%-*C@3 m]-|Ctrl.&]
[s7;%% [%-*C@3 ID]-|Message loop exit code.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Rejector`(Ctrl`&`,int`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* Rejec
tor]([_^Ctrl^ Ctrl][@(0.0.255) `&]_[*@3 m], [@(0.0.255) int]_[*@3 ID])&]
[s2;%% Same as m <<`= [* Rejector](ID) `- assigns [* Rejector] callback 
to WhenAction method of specified Ctrl.&]
[s7;%% [%-*C@3 m]-|Ctrl.&]
[s7;%% [%-*C@3 ID]-|Message loop exit code.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:NoCenter`(`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* NoCenter]()&]
[s2;%% Sets TopWindow not to be centered when being created on the 
screen. Returns `*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:CenterOwner`(`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* CenterOwner](
)&]
[s2;%% Sets TopWindow to be centered w.r.t. its owner when being 
created on the screen. This is default. Returns `*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:CenterScreen`(`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* CenterScreen
]()&]
[s2;%% Sets TopWindow to be desktop centered when being created on 
the screen. Returns `*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:SetMinSize`(Size`): [@(0.0.255) virtual] [@(0.0.255) void]_[* SetMinSize](
[_^Size^ Size]_[*@3 sz])&]
[s2;%% Sets the minimal size for window. This size is then returned 
by GetMinSize virtual method. If window is resizable, it cannot 
be made smaller than this size.&]
[s7;%% [%-*C@3 sz]-|Minimal size of window.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Open`(HWND`): [@(0.0.255) void]_[* Open](HWND_[*@3 ownerhwnd])&]
[s2;%% Opens window with owner specified as Win32 handle.&]
[s6;%% Win32 specific.&]
[s7;%% [%-*C@3 ownerhwnd]-|Owner window handle.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Style`(dword`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* Style]([_^dword^ d
word]_[*@3 `_style])&]
[s2;%% Sets Win32 window style.&]
[s6;%% Win32 specific.&]
[s7;%% [%-*C@3 `_style]-|Style.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:GetStyle`(`)const: [_^dword^ dword]_[* GetStyle]()_[@(0.0.255) const]&]
[s2;%% Sets Win32 window extended style.&]
[s6;%% Win32 specific.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:ExStyle`(dword`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* ExStyle]([_^dword^ d
word]_[*@3 `_exstyle])&]
[s6;%% Win32 specific.&]
[s7;%% [%-*C@3 `_exstyle]-|style&]
[s7;%% [*/ Return value]-|`*this&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:GetExStyle`(`)const: [_^dword^ dword]_[* GetExStyle]()_[@(0.0.255) const]&]
[s6;%% Win32 specific.&]
[s7;%% [*/ Return value]-|style&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Open`(Ctrl`*`): [@(0.0.255) void]_[* Open]([_^Ctrl^ Ctrl]_`*[*@3 owner])&]
[s2;%% Opens TopWindow as window in host GUI with specified owner. 
Window can be open as hidden is visibility flag is set to false 
(Hide()), however on closing, window is set to visible.&]
[s7;%% [%-*C@3 owner]-|Owner of window. If NULL, window has no owner.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Open`(`): [@(0.0.255) void]_[* Open]()&]
[s2;%% Opens TopWindow with owner equal to current active window 
if it is owned by the process.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:OpenMain`(`): [@(0.0.255) void]_[* OpenMain]()&]
[s2;%% Opens window without owner.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Run`(bool`): [@(0.0.255) int]_[* Run]([@(0.0.255) bool]_[*@3 appmodal]_`=_[@(0.0.255) f
alse])&]
[s2;%% Execute modal loop for TopWindow. If TopWindow is not open, 
it calls Open before starting loop, but it does not close it.&]
[s7;%% [%-*C@3 appmodal]-|If true, all application window are disabled 
while performing loop. If false, only TopWindow`'s relative windows 
(with the same `"super`"`-owner) are disabled.&]
[s7;%% [*/ Return value]-|Result code of modal loop.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:RunAppModal`(`): [@(0.0.255) int]_[* RunAppModal]()&]
[s2;%% Same as Run(true).&]
[s7;%% [*/ Return value]-|Result code of modal loop.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Execute`(`): [@(0.0.255) int]_[* Execute]()&]
[s2;%% Similar to Run, but closes TopWindow when loop exits.&]
[s7;%% [*/ Return value]-|Result code of modal loop.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:ExecuteOK`(`): [@(0.0.255) bool]_[* ExecuteOK]()&]
[s2;%% Same as Execute but return true if button OK was pressed.&]
[s7;%% [*/ Return value]-|Result true if OK otherwise false.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:ExecuteCancel`(`): [@(0.0.255) bool]_[* ExecuteCancel]()&]
[s2;%% Same as Execute but return true if button Cancel was pressed.&]
[s7;%% [*/ Return value]-|Result true if Cancel otherwise false.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Minimize`(bool`): [@(0.0.255) void]_[* Minimize]([@(0.0.255) bool]_[*@3 eff
ect]_`=_[@(0.0.255) false])&]
[s2;%% Minimize window.&]
[s7;%% [%-*C@3 effect]-|Do platform specific GUI effects.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Maximize`(bool`): [@(0.0.255) void]_[* Maximize]([@(0.0.255) bool]_[*@3 eff
ect]_`=_[@(0.0.255) false])&]
[s2;%% Maximize window.&]
[s7;%% [%-*C@3 effect]-|Do platform specific GUI effects.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Overlap`(bool`): [@(0.0.255) void]_[* Overlap]([@(0.0.255) bool]_[*@3 effec
t]_`=_[@(0.0.255) false])&]
[s2;%% Put window to normal overlapped state.&]
[s7;%% [%-*C@3 effect]-|Do platform specific GUI effects.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:IsMaximized`(`)const: [@(0.0.255) bool]_[* IsMaximized]()_[@(0.0.255) con
st]&]
[s7;%% [*/ Return value]-|True if TopWindow is maximized.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:IsMinimized`(`)const: [@(0.0.255) bool]_[* IsMinimized]()_[@(0.0.255) con
st]&]
[s7;%% [*/ Return value]-|True if TopWindow is minimized.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:IsOverlapped`(`)const: [@(0.0.255) bool]_[* IsOverlapped]()_[@(0.0.255) c
onst]&]
[s7;%% [*/ Return value]-|True if TopWindow is in normal (overlapped) 
state.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:ActiveFocus`(Ctrl`&`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* ActiveF
ocus]([_^Ctrl^ Ctrl][@(0.0.255) `&]_[*@3 ctrl])&]
[s2;%% Sets the ActiveFocus Ctrl. This Ctrl gets focus when TopWindow 
is activated. If there is no Ctrl with active focus, first Ctrl 
is activated. When TopWindow is deactivated, Ctrl that has focus 
is stored as new ActiveFocus Ctrl and gets focus once TopWindow 
is activated again (unless this method changes ActiveFocus meanwhile). 
When this, method is called while TopWindow is active, it is 
equivalent to ctrl.SetFocus().&]
[s7;%% [%-*C@3 ctrl]-|Ctrl to have &]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Title`(const WString`&`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* Titl
e]([@(0.0.255) const]_[_^WString^ WString][@(0.0.255) `&]_[*@3 `_title])&]
[s5;:TopWindow`:`:Title`(const char`*`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* Title](
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 `_title])&]
[s2;%% Sets the title of the window.&]
[s7;%% [%-*C@3 `_title]-|Title.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:GetTitle`(`)const: [@(0.0.255) const]_[_^WString^ WString][@(0.0.255) `&]_
[* GetTitle]()_[@(0.0.255) const]&]
[s7;%% [*/ Return value]-|Current title of window.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Sizeable`(bool`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* Sizeable]([@(0.0.255) b
ool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% Indicates whether TopWindow should be resizeable by user.&]
[s7;%% [%-*C@3 b]-|True to set.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:NoSizeable`(`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* NoSizeable]()&]
[s2;%% Same as Sizeable(false).&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:IsSizeable`(`)const: [@(0.0.255) bool]_[* IsSizeable]()_[@(0.0.255) const
]&]
[s7;%% [*/ Return value]-|Value&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:MinimizeBox`(bool`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* MinimizeB
ox]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% Sets TopWindow to have a minimize box.&]
[s7;%% [%-*C@3 b]-|true to set.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:MaximizeBox`(bool`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* MaximizeB
ox]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% Sets TopWindow to have a maximize box.&]
[s7;%% [%-*C@3 b]-|true to set.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Zoomable`(bool`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* Zoomable]([@(0.0.255) b
ool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% Sets TopWindow to be Zoomable `- to have maximize/minimize 
flags, if supported by host platform. Same as MinimizeBox(b).MaximizeBox(b).&]
[s7;%% [%-*C@3 b]-|True to set.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:NoZoomable`(`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* NoZoomable]()&]
[s2;%% Same as Zoomable(false).&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:IsZoomable`(`)const: [@(0.0.255) bool]_[* IsZoomable]()_[@(0.0.255) const
]&]
[s7;%% [*/ Return value]-|Value of Zoomable flag.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Background`(const PaintRect`&`): [_^TopWindow^ TopWindow][@(0.0.255) `&
]_[* Background]([@(0.0.255) const]_[_^PaintRect^ PaintRect][@(0.0.255) `&]_[*@3 prect])&]
[s2;%% Sets PaintRect that will be used to paint TopWindow background. 
Default PaintRect paints background with SColorFace.&]
[s7;%% [%-*C@3 prect]-|PaintRect.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:GetBackground`(`)const: [@(0.0.255) const]_[_^PaintRect^ PaintRect][@(0.0.255) `&
]_[* GetBackground]()_[@(0.0.255) const]&]
[s2;%% Returns active PaintRect used to paint TopWindow background.&]
[s7;%% [*/ Return value]-|PaintRect.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:ToolWindow`(bool`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* ToolWindow
]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% Makes TopWindow appear with thin caption, if supported by 
host platform. Must be set before opening.&]
[s7;%% [%-*C@3 b]-|True to set the flag.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:NoToolWindow`(`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* NoToolWindow
]()&]
[s2;%% Same as ToolWindow(false).&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:IsToolWindow`(`)const: [@(0.0.255) bool]_[* IsToolWindow]()_[@(0.0.255) c
onst]&]
[s7;%% [*/ Return value]-|Value of ToolWindow flag.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:TopMost`(bool`,bool`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* TopMost
]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true], [@(0.0.255) bool]_[*@3 stay`_top]_`=_[@(0.0.255) t
rue])&]
[s2;%% Makes window stay over any other window. Returns `*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:NoTopMost`(`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* NoTopMost]()&]
[s2;%% Same as TopMost(false).&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:IsTopMost`(`)const: [@(0.0.255) bool]_[* IsTopMost]()_[@(0.0.255) const]&]
[s2;%% Value of TopMost flag.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:FullScreen`(bool`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* FullScreen
]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% Sets window to fullscreen if [%-*@3 b] is true, otherwise sets 
window to regular state. Returns `*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:IsFullScreen`(`)const: [@(0.0.255) bool]_[* IsFullScreen]()_[@(0.0.255) c
onst]&]
[s2;%% [*/ Return value]-|True if window is in fullscreen mode.&]
[s3; &]
[s4; &]
[s5;:TopWindow`:`:FrameLess`(bool`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* FrameLess](
[@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% Creates window without border decorations (like windows title 
var or any resizing borders).&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:IsFrameLess`(`)const: [@(0.0.255) bool]_[* IsFrameLess]()_[@(0.0.255) con
st]&]
[s2;%% Returns FrameLess status.&]
[s3; &]
[s4; &]
[s5;:TopWindow`:`:Urgent`(bool`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* Urgent]([@(0.0.255) b
ool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% Calling Urgent(true) causes title of inactive window and its 
item in taskbar to flash to catch users attention. The effect 
lasts until either Urgent(false) is called or the window is taken 
to foreground. If Urgent(true) is called on currently active 
window then it has no effect.&]
[s2;%% &]
[s2;%% Note: Most of the window managers on X11 do support this feature, 
but there still might be some obscure ones that simply ignore 
this setting.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:IsUrgent`(`)const: [@(0.0.255) bool]_[* IsUrgent]()_[@(0.0.255) const]&]
[s2;%% [*/ Return value]-|True if window tries to catch users attention 
by flashing.&]
[s3; &]
[s4; &]
[s5;:TopWindow`:`:NoAccessKeysDistribution`(`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* N
oAccessKeysDistribution]()&]
[s2;%% Disables distribution of access keys. Normally, TopWindow 
tries to assign access`-keys (also known as `"hot`-keys`") to 
all of its child windows before opening. This method disables 
this behaviour.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:CloseBoxRejects`(bool`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* Close
BoxRejects]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% When active, pressing window title close button always rejects 
the dialog. When not active, the action depends on Acceptors/Rejectors 
present, performing first of IDCANCEL `-> reject, IDNO `-> reject, 
IDEXIT `-> accept, IDYES `-> Accept, and IDOK `-> Accept as default 
action if nothing else is present.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Icon`(const Image`&`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* Icon]([@(0.0.255) c
onst]_[_^Image^ Image][@(0.0.255) `&]_[*@3 m])&]
[s2;%% Defines icon for TopWindow that is usually displayed in window 
caption (platform specif).&]
[s7;%% [%-*C@3 m]-|Icon.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:LargeIcon`(const Image`&`): [_^TopWindow^ TopWindow][@(0.0.255) `&]_[* La
rgeIcon]([@(0.0.255) const]_[_^Image^ Image][@(0.0.255) `&]_[*@3 m])&]
[s2;%% Defines `"large`" icon for TopWindow. It can be displayed 
e.g. in task switcher and other places (platform specific).&]
[s7;%% [%-*C@3 m]-|Icon.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:Icon`(const Image`&`,const Image`&`): [_^TopWindow^ TopWindow][@(0.0.255) `&
]_[* Icon]([@(0.0.255) const]_[_^Image^ Image][@(0.0.255) `&]_[*@3 smallicon], 
[@(0.0.255) const]_[_^Image^ Image][@(0.0.255) `&]_[*@3 largeicon])&]
[s2;%% Defines both `"small`" and `"large`" icons for TopWindow.&]
[s7;%% [%-*C@3 smallicon]-|Small icon.&]
[s7;%% [%-*C@3 largeicon]-|Large icon.&]
[s7;%% [*/ Return value]-|`*this.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:SerializePlacement`(Stream`&`,bool`): [@(0.0.255) void]_[* SerializePla
cement]([_^Stream^ Stream][@(0.0.255) `&]_[*@3 s], [@(0.0.255) bool]_[*@3 reminimize]_`=_[@(0.0.255) f
alse])&]
[s2;%% Serializes current placement of TopWindow. It is platform 
specific, however information can include screen position in 
`"normal`" state and current status (maximized, minimized, normal).&]
[s7;%% [%-*C@3 s]-|Stream.&]
[s7;%% [%-*C@3 reminimize]-|If false and loading the information from 
the stream, minimized state is not restored.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:TopWindow`(`): [* TopWindow]()&]
[s2;%% Constructor.&]
[s3;%% &]
[s4; &]
[s5;:TopWindow`:`:`~TopWindow`(`): [@(0.0.255) `~][* TopWindow]()&]
[s2;%% Destructor.&]
[s3;%% &]
[s0;%% ]]