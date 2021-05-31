topic "Message Boxes";
[i448;a25;kKO9;2 $$1,0#37138531426314131252341829483380:class]
[l288;2 $$2,2#27521748481378242620020725143825:desc]
[0 $$3,0#96390100711032703541132217272105:end]
[H6;0 $$4,0#05600065144404261032431302351956:begin]
[i448;a25;kKO9;2 $$5,0#37138531426314131252341829483370:item]
[l288;a4;*@5;1 $$6,6#70004532496200323422659154056402:requirement]
[l288;i1121;b17;O9;~~~.1408;2 $$7,0#10431211400427159095818037425705:param]
[i448;b42;O9;2 $$8,8#61672508125594000341940100500538:tparam]
[b42;ph2 $$9,9#13035079074754324216151401829390:normal]
[b42;a42;ph2 $$10,10#E71D914572A169C91BE52E3FB27F810F:text]
[2 $$0,0#00000000000000000000000000000000:Default]
[{_} 
[ {{10000@(113.42.0) [s0;%% [*@7;4 Message Boxes]]}}&]
[s10;%% Message boxes in U`+`+ provide up to 3 buttons. The message 
is using [*^topic`:`/`/RichText`/srcdoc`/QTF`_en`-us^ QTF] rich 
text format `- it means that you can for example embed hyper`-link 
or even table or image. This however can cause issues if some 
non`-literal text is presented which can accidentally contain 
QTF formatting characters. To solve this, you can use [*^topic`:`/`/RichText`/src`/Utilities`$en`-us`#Upp`:`:DeQtf`(const char`*`)^ D
eQtf][*  ]function `'`\1`' switch (supresses QTF parsing till the 
next `'`\1`'), e.g.:&]
[s10;l160;%% [/@(28.42.150) PromptOK(DeQtf(`"C:`\`\Program Files`\`\Upp`\`\theide`"));]&]
[s10;l160;%% [/@(28.42.150) PromptOK(`"`\1C:`\`\Program Files`\`\Upp`\`\theide`");]&]
[s10; &]
[s5;:Upp`:`:PromptOK`(const char`*`): [@(0.0.255) void]_[* PromptOK]([@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 qtf])&]
[s2;%%  [%-*@3 qtf] .&]
[s3;%% &]
[s10; &]
[s10; &]
[s0; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Functions]]}}&]
[s3; &]
[s5;:Upp`:`:PromptOKCancel`(const char`*`): [@(0.0.255) int]_[* PromptOKCancel]([@(0.0.255) c
onst]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:PromptOKCancelAll`(const char`*`): [@(0.0.255) int]_[* PromptOKCancelAll]([@(0.0.255) c
onst]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:PromptYesNo`(const char`*`): [@(0.0.255) int]_[* PromptYesNo]([@(0.0.255) const
]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:PromptYesNoCancel`(const char`*`): [@(0.0.255) int]_[* PromptYesNoCancel]([@(0.0.255) c
onst]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:PromptYesNoAll`(const char`*`): [@(0.0.255) int]_[* PromptYesNoAll]([@(0.0.255) c
onst]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:PromptRetryCancel`(const char`*`): [@(0.0.255) int]_[* PromptRetryCancel]([@(0.0.255) c
onst]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:PromptAbortRetry`(const char`*`): [@(0.0.255) int]_[* PromptAbortRetry]([@(0.0.255) c
onst]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:PromptAbortRetryIgnore`(const char`*`): [@(0.0.255) int]_[* PromptAbortRetryI
gnore]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:PromptSaveDontSaveCancel`(const char`*`): [@(0.0.255) int]_[* PromptSaveDontS
aveCancel]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:ShowExc`(const Upp`:`:Exc`&`): [@(0.0.255) void]_[* ShowExc]([@(0.0.255) const]_
[_^Upp`:`:Exc^ Exc][@(0.0.255) `&]_[*@3 exc])&]
[s5;:Upp`:`:Exclamation`(const char`*`): [@(0.0.255) void]_[* Exclamation]([@(0.0.255) cons
t]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:ErrorOK`(const char`*`): [@(0.0.255) void]_[* ErrorOK]([@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 qtf])&]
[s5;:Upp`:`:ErrorOKCancel`(const char`*`): [@(0.0.255) int]_[* ErrorOKCancel]([@(0.0.255) c
onst]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:ErrorYesNo`(const char`*`): [@(0.0.255) int]_[* ErrorYesNo]([@(0.0.255) const]_
[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:ErrorYesNoCancel`(const char`*`): [@(0.0.255) int]_[* ErrorYesNoCancel]([@(0.0.255) c
onst]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:ErrorYesNoAll`(const char`*`): [@(0.0.255) int]_[* ErrorYesNoAll]([@(0.0.255) c
onst]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:ErrorRetryCancel`(const char`*`): [@(0.0.255) int]_[* ErrorRetryCancel]([@(0.0.255) c
onst]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:ErrorAbortRetry`(const char`*`): [@(0.0.255) int]_[* ErrorAbortRetry]([@(0.0.255) c
onst]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:ErrorAbortRetryIgnore`(const char`*`): [@(0.0.255) int]_[* ErrorAbortRetryIgn
ore]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf])&]
[s5;:Upp`:`:PromptOKOpt`(const char`*`,const char`*`): [@(0.0.255) void]_[* PromptOKOpt](
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 o
pt`_id]_`=_NULL)&]
[s5;:Upp`:`:PromptOKCancelOpt`(const char`*`,const char`*`): [@(0.0.255) int]_[* PromptOK
CancelOpt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 opt`_id]_`=_NULL)&]
[s5;:Upp`:`:PromptOKCancelAllOpt`(const char`*`,const char`*`): [@(0.0.255) int]_[* Promp
tOKCancelAllOpt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id]_`=_NULL)&]
[s5;:Upp`:`:PromptYesNoOpt`(const char`*`,const char`*`): [@(0.0.255) int]_[* PromptYesNo
Opt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) char]_
`*[*@3 opt`_id]_`=_NULL)&]
[s5;:Upp`:`:PromptYesNoCancelOpt`(const char`*`,const char`*`): [@(0.0.255) int]_[* Promp
tYesNoCancelOpt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id]_`=_NULL)&]
[s5;:Upp`:`:PromptYesNoAllOpt`(const char`*`,const char`*`): [@(0.0.255) int]_[* PromptYe
sNoAllOpt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 opt`_id]_`=_NULL)&]
[s5;:Upp`:`:PromptRetryCancelOpt`(const char`*`,const char`*`): [@(0.0.255) int]_[* Promp
tRetryCancelOpt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id]_`=_NULL)&]
[s5;:Upp`:`:PromptAbortRetryOpt`(const char`*`,const char`*`): [@(0.0.255) int]_[* Prompt
AbortRetryOpt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 opt`_id]_`=_NULL)&]
[s5;:Upp`:`:PromptAbortRetryIgnoreOpt`(const char`*`,const char`*`): [@(0.0.255) int]_[* P
romptAbortRetryIgnoreOpt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id]_`=_NULL)&]
[s5;:Upp`:`:PromptSaveDontSaveCancelOpt`(const char`*`,const char`*`): [@(0.0.255) int]_
[* PromptSaveDontSaveCancelOpt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id]_`=_NULL)&]
[s5;:Upp`:`:ExclamationOpt`(const char`*`,const char`*`): [@(0.0.255) void]_[* Exclamatio
nOpt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) char
]_`*[*@3 opt`_id]_`=_NULL)&]
[s5;:Upp`:`:ErrorOKOpt`(const char`*`,const char`*`): [@(0.0.255) void]_[* ErrorOKOpt]([@(0.0.255) c
onst]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id]_`=
_NULL)&]
[s5;:Upp`:`:ErrorOKCancelOpt`(const char`*`,const char`*`): [@(0.0.255) int]_[* ErrorOKCa
ncelOpt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 opt`_id]_`=_NULL)&]
[s5;:Upp`:`:ErrorYesNoOpt`(const char`*`,const char`*`): [@(0.0.255) int]_[* ErrorYesNoOp
t]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) char]_`*
[*@3 opt`_id]_`=_NULL)&]
[s5;:Upp`:`:ErrorYesNoCancelOpt`(const char`*`,const char`*`): [@(0.0.255) int]_[* ErrorY
esNoCancelOpt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 opt`_id]_`=_NULL)&]
[s5;:Upp`:`:ErrorYesNoAllOpt`(const char`*`,const char`*`): [@(0.0.255) int]_[* ErrorYesN
oAllOpt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 opt`_id]_`=_NULL)&]
[s5;:Upp`:`:ErrorRetryCancelOpt`(const char`*`,const char`*`): [@(0.0.255) int]_[* ErrorR
etryCancelOpt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 opt`_id]_`=_NULL)&]
[s5;:Upp`:`:ErrorAbortRetryOpt`(const char`*`,const char`*`): [@(0.0.255) int]_[* ErrorAb
ortRetryOpt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:ErrorAbortRetryIgnoreOpt`(const char`*`,const char`*`): [@(0.0.255) int]_[* E
rrorAbortRetryIgnoreOpt]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:ShowExcOpt`(const Upp`:`:Exc`&`,const char`*`): [@(0.0.255) void]_[* ShowExcO
pt]([@(0.0.255) const]_[_^Upp`:`:Exc^ Exc][@(0.0.255) `&]_[*@3 exc], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:PromptOKOpt1`(const char`*`,const char`*`): [@(0.0.255) void]_[* PromptOKOpt1
]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) char]_`*
[*@3 opt`_id])&]
[s5;:Upp`:`:PromptOKCancelOpt1`(const char`*`,const char`*`): [@(0.0.255) int]_[* PromptO
KCancelOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:PromptOKCancelAllOpt1`(const char`*`,const char`*`): [@(0.0.255) int]_[* Prom
ptOKCancelAllOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:PromptYesNoOpt1`(const char`*`,const char`*`): [@(0.0.255) int]_[* PromptYesN
oOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) cha
r]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:PromptYesNoCancelOpt1`(const char`*`,const char`*`): [@(0.0.255) int]_[* Prom
ptYesNoCancelOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:PromptYesNoAllOpt1`(const char`*`,const char`*`): [@(0.0.255) int]_[* PromptY
esNoAllOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:PromptRetryCancelOpt1`(const char`*`,const char`*`): [@(0.0.255) int]_[* Prom
ptRetryCancelOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:PromptAbortRetryOpt1`(const char`*`,const char`*`): [@(0.0.255) int]_[* Promp
tAbortRetryOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:PromptAbortRetryIgnoreOpt1`(const char`*`,const char`*`): [@(0.0.255) int]_
[* PromptAbortRetryIgnoreOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:PromptSaveDontSaveCancelOpt1`(const char`*`,const char`*`): [@(0.0.255) int
]_[* PromptSaveDontSaveCancelOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:ExclamationOpt1`(const char`*`,const char`*`): [@(0.0.255) void]_[* Exclamati
onOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) ch
ar]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:ErrorOKOpt1`(const char`*`,const char`*`): [@(0.0.255) void]_[* ErrorOKOpt1](
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 o
pt`_id])&]
[s5;:Upp`:`:ErrorOKCancelOpt1`(const char`*`,const char`*`): [@(0.0.255) int]_[* ErrorOKC
ancelOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:ErrorYesNoOpt1`(const char`*`,const char`*`): [@(0.0.255) int]_[* ErrorYesNoO
pt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) char]_
`*[*@3 opt`_id])&]
[s5;:Upp`:`:ErrorYesNoCancelOpt1`(const char`*`,const char`*`): [@(0.0.255) int]_[* Error
YesNoCancelOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:ErrorYesNoAllOpt1`(const char`*`,const char`*`): [@(0.0.255) int]_[* ErrorYes
NoAllOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:ErrorRetryCancelOpt1`(const char`*`,const char`*`): [@(0.0.255) int]_[* Error
RetryCancelOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:ErrorAbortRetryOpt1`(const char`*`,const char`*`): [@(0.0.255) int]_[* ErrorA
bortRetryOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:ErrorAbortRetryIgnoreOpt1`(const char`*`,const char`*`): [@(0.0.255) int]_[* E
rrorAbortRetryIgnoreOpt1]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 qtf], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id])&]
[s5;:Upp`:`:ShowExcOpt1`(const Upp`:`:Exc`&`,const char`*`): [@(0.0.255) void]_[* ShowExc
Opt1]([@(0.0.255) const]_[_^Upp`:`:Exc^ Exc][@(0.0.255) `&]_[*@3 exc], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 opt`_id])&]
[s2; There are many various functions providing message boxes. The 
first word in camel case function name designates the icon used 
in the message box and possible associated sound. It is followed 
by the name of buttons and optional [* Opt] or [* Opt1] part. [* Opt] 
and [* Opt1] mean that the dialog has `"Do not show this again`" 
option (more on that later).&]
[s2; If there are 2 or 3 buttons, the function returns an int to 
report which button was pressed. First button listed in the name 
returns 1, second 0 and third `-1 (e.g. in PromptYesNo Yes is 
1 and No is 0, in PromptYesNoCancle Yes is 1, No is 0 and Cancel 
is `-1).&]
[s2; If the name contains [* Opt] or [* Opt1], the process of ignoring 
the dialog next time is automated via storing specific String 
key `- this key is passed as second const char `* parameter to 
the function `- if it is NULL (default), it is generated by hashing 
the message.&]
[s2; The diference between [* Opt] and [* Opt1] is that [* Opt] remembers 
which button was pressed and returns the same value next time 
(e.g. if in PromptOKCancel `"Do not show again`" is checked and 
user presses Cancel, the next the same PromptOKCancel is invoked, 
it returns without displaying the dialog with 0). As this behaviour 
can be confusing in certain context, [* Opt1] variant only grants 
`"Do not show again`" request if the user chooses button that 
returns 1 (e.g. OK or Yes).&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:ClearPromptOptHistory`(`): [@(0.0.255) void]_[* ClearPromptOptHistory]()&]
[s2;%% Clears the list of `"Do not show again`" message boxes.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:ClearPromptOptHistory`(Upp`:`:Gate`<Upp`:`:String`>`): [@(0.0.255) void]_[* C
learPromptOptHistory]([_^Upp`:`:Gate^ Gate]<[_^Upp`:`:String^ String]>_[*@3 filter])&]
[s2;%% Clears the list of `"Do not show again`" message boxes `- 
only removes those where [%-*@3 filter] returns true for [%-*@3 opt`_id].&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SerializePromptOptHistory`(Upp`:`:Stream`&`): [@(0.0.255) void]_[* SerializeP
romptOptHistory]([_^Upp`:`:Stream^ Stream][@(0.0.255) `&]_[*@3 s])&]
[s2;%% Serializes the list of `"Do not show again`" message boxes.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:RedirectPrompts`(Upp`:`:RedirectPromptFn`): [@(0.0.255) void]_[* RedirectProm
pts]([_^Upp`:`:RedirectPromptFn^ RedirectPromptFn]_[*@3 r])&]
[s2;%% Completely redirects all message boxes to another function.&]
[s3;%% &]
[s0; ]]