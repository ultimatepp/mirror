topic "Image and ImageBuffer";
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
[ {{10000@(113.42.0) [s0;%% [*@7;4 Image]]}}&]
[s3; &]
[s1;:Image`:`:class: [*@(0.0.255) class][* _Image_:_][*@(0.0.255) public][* _][*@3 AssignValueTy
peNo][* <_Image, ][*@3 150][* , ][*_^Moveable^ Moveable][* <Image>_>_]&]
[s9;%% Image represents an immutable image value. Image can be directly 
painted to Draw. To create or change Image, use [^`:`:ImageBuffer^ ImageBuffer].&]
[s9;%% Image has low`-cost constant time deep copy. It is moveable 
and Rich`-Value compatible type.&]
[s9;%% U`+`+ expects pixels to be in premultiplied alpha format.&]
[s3; &]
[s0; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Public Member List]]}}&]
[s3; &]
[s5;:Image`:`:operator`~`(`)const: [@(0.0.255) const]_[_^RGBA^ RGBA][@(0.0.255) `*]_operato
r`~()_[@(0.0.255) const]&]
[s5;:Image`:`:operator const RGBA`*`(`)const: operator_const_RGBA`*()_[@(0.0.255) const
]&]
[s2;%% Returns a pointer to Image pixels.&]
[s3; &]
[s4; &]
[s5;:Image`:`:operator`[`]`(int`)const: [@(0.0.255) const]_[_^RGBA^ RGBA][@(0.0.255) `*]_op
erator`[`]([@(0.0.255) int]_[@3 i])_[@(0.0.255) const]&]
[s2;%% Returns a pointer to the first pixel in the line [%-*@3 i].&]
[s3; &]
[s4; &]
[s5;:Image`:`:GetSize`(`)const: [_^topic`:`/`/Core`/src`/Size`$en`-us^ Size]_GetSize()_
[@(0.0.255) const]&]
[s2;%% Returns the dimension of Image.&]
[s3; &]
[s4; &]
[s5;:Image`:`:GetWidth`(`)const: [@(0.0.255) int]_GetWidth()_[@(0.0.255) const]&]
[s2;%% Same as GetSize().cx.&]
[s3; &]
[s4; &]
[s5;:Image`:`:GetHeight`(`)const: [@(0.0.255) int]_GetHeight()_[@(0.0.255) const]&]
[s2;%% Same as GetSize().cy.&]
[s3; &]
[s4; &]
[s5;:Image`:`:GetLength`(`)const: [@(0.0.255) int]_GetLength()_[@(0.0.255) const]&]
[s2;%% Number of pixels in Image `- GetWidth() `* GetHeight().&]
[s3; &]
[s4; &]
[s5;:Image`:`:GetHotSpot`(`)const: [_^topic`:`/`/Core`/src`/Point`$en`-us^ Point]_GetHo
tSpot()[@(64) _][@(0.0.255) const]&]
[s2;%% Returns the reference point.&]
[s3; &]
[s4; &]
[s5;:Image`:`:Get2ndSpot`(`)const: [_^topic`:`/`/Core`/src`/Point`$en`-us^ Point]_Get2n
dSpot()_[@(0.0.255) const]&]
[s2;%% Returns secondary reference point.&]
[s3; &]
[s4; &]
[s5;:Image`:`:GetDots`(`)const: [_^topic`:`/`/Core`/src`/Size`$en`-us^ Size]_GetDots()[@(64) _
][@(0.0.255) const]&]
[s2;%% Gets the physical size of Image. If physical size is not set, 
returns Size(0, 0).&]
[s3; &]
[s4; &]
[s5;:Image`:`:GetDPI`(`): [_^Upp`:`:Size^ Size]_[* GetDPI]()_[@(0.0.255) const]&]
[s2;%% Returns the image resolution in dpi (dots per inch).&]
[s3; &]
[s4; &]
[s5;:Image`:`:GetKindNoScan`(`)const: [@(0.0.255) int]_GetKindNoScan()_[@(0.0.255) const]&]
[s2;%% Returns the kind of image. Unlike GetKind(), it does not perform 
the scan of image if the kind it not know, therefore it can return 
IMAGE`_UNKNOWN.&]
[s3; &]
[s4; &]
[s5;:Image`:`:GetKind`(`)const: [@(0.0.255) int]_GetKind()_[@(0.0.255) const]&]
[s2;%% Returns the kind of image, either set by ImageBuffer`::SetKind 
or (if it is IMAGE`_UNKNOWN) resolved by scanning pixels. If 
the kind is IMAGE`_UNKNOWN, the pixels are scanned and the type 
is determined, it is then stored (in private Image instance variable) 
for future use and returned.&]
[s3; &]
[s4; &]
[s5;:Image`:`:Begin`(`)const: [@(0.0.255) const]_[_^RGBA^ RGBA]_`*[* Begin]()_[@(0.0.255) con
st]&]
[s5;:Upp`:`:Image`:`:begin`(`)const: [@(0.0.255) const]_[_^Upp`:`:RGBA^ RGBA]_`*[* begin]()
_[@(0.0.255) const]&]
[s2;%% Returns a pointer to the first pixel.&]
[s3; &]
[s4; &]
[s5;:Image`:`:End`(`)const: [@(0.0.255) const]_[_^RGBA^ RGBA]_`*[* End]()_[@(0.0.255) const]&]
[s5;:Upp`:`:Image`:`:end`(`)const: [@(0.0.255) const]_[_^Upp`:`:RGBA^ RGBA]_`*[* end]()_[@(0.0.255) c
onst]&]
[s2;%% Returns begin() `+ GetLength().&]
[s3; &]
[s4; &]
[s5;:Image`:`:GetSerialId`(`)const: [_^int64^ int64]_GetSerialId()[@(64) _][@(0.0.255) cons
t]&]
[s2;%% Returns the unique, per application run, identifier of Image. 
All Images with the same serial id can be considered equal (this 
is useful for caching images).&]
[s3; &]
[s4; &]
[s5;:Image`:`:IsSame`(const Image`&`)const: [@(0.0.255) bool]_IsSame([@(0.0.255) const]_[_^Image^ I
mage][@(0.0.255) `&]_[@3 img])_[@(0.0.255) const]&]
[s2;%% Same as GetSerialId() `=`= img.GetSerialId().&]
[s3; &]
[s4; &]
[s5;:Image`:`:operator`=`=`(const Image`&`)const: [@(0.0.255) bool]_operator`=`=([@(0.0.255) c
onst]_[_^Image^ Image][@(0.0.255) `&]_[@3 img])_[@(0.0.255) const]&]
[s5;:Image`:`:operator`!`=`(const Image`&`)const: [@(0.0.255) bool]_operator!`=([@(0.0.255) c
onst]_[_^Image^ Image][@(0.0.255) `&]_[@3 img])_[@(0.0.255) const]&]
[s2;%% Tests whether two Images are equal (or not equal). Dimensions, 
hot`-spots, dots and all pixels are compared. Note that the comparison 
can be slow.&]
[s3; &]
[s4; &]
[s5;:Image`:`:GetHashValue`(`)const: [_^dword^ dword]_GetHashValue()[@(64) _][@(0.0.255) co
nst]&]
[s2;%% Returns the hash`-value for image. All pixels combined into 
hash value (potentially slow).&]
[s3; &]
[s4; &]
[s5;:Image`:`:ToString`(`)const: [_^topic`:`/`/Core`/src`/String`$en`-us^ String]_ToStr
ing()[@(64) _][@(0.0.255) const]&]
[s2;%% Returns the basic Image informations as String.&]
[s3; &]
[s4; &]
[s5;:Image`:`:Serialize`(Stream`&`): [@(0.0.255) void]_Serialize([_^Stream^ Stream][@(0.0.255) `&
]_[@3 s])&]
[s2;%% Serializes Image.&]
[s3; &]
[s4; &]
[s5;:Image`:`:Clear`(`): [@(0.0.255) void]_Clear()&]
[s2;%% Assigns an empty Image.&]
[s3; &]
[s4; &]
[s5;:Image`:`:operator`=`(const Image`&`): [_^Image^ Image][@(0.0.255) `&]_operator`=([@(0.0.255) c
onst]_[_^Image^ Image][@(0.0.255) `&]_[@3 img])&]
[s2;%% Assigns another Image.&]
[s3; &]
[s4; &]
[s5;:Image`:`:operator`=`(ImageBuffer`&`): [_^Image^ Image][@(0.0.255) `&]_operator`=([_^ImageBuffer^ I
mageBuffer][@(0.0.255) `&]_[@3 img])&]
[s2;%% Assigns Image created in ImageBuffer. ImageBuffer is emptied 
by this operation.&]
[s3; &]
[s4; &]
[s5;:Image`:`:IsNullInstance`(`)const: [@(0.0.255) bool]_IsNullInstance()_[@(0.0.255) con
st]&]
[s2;%% Test whether Image has nonzero dimension.&]
[s3; &]
[s4; &]
[s5;:Image`:`:IsEmpty`(`)const: [@(0.0.255) bool]_IsEmpty()_[@(0.0.255) const]&]
[s2;%% Same as IsNullInstance.&]
[s3; &]
[s4; &]
[s5;:Image`:`:operator Value`(`)const: operator_Value()[@(64) _][@(0.0.255) const]&]
[s2;%% Converts Image to Value.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:Image`:`:IsPaintOnceHint`(`)const: [@(0.0.255) bool]_[* IsPaintOnceHint]()_[@(0.0.255) c
onst]&]
[s2;%% Signals that Image is supposed to be painted only once with 
Draw`::DrawImage.,&]
[s3; &]
[s0; &]
[ {{10000t/25b/25@1 [s0; [* Constructor Detail]]}}&]
[s3; &]
[s5;:Image`:`:Image`(`): Image()&]
[s3; &]
[s4; &]
[s5;:Image`:`:Image`(const Nuller`&`): Image([@(0.0.255) const]_[_^Nuller^ Nuller][@(0.0.255) `&
])&]
[s2;%% Constructs empty Image.&]
[s3; &]
[s4; &]
[s5;:Image`:`:Image`(const Value`&`): Image([@(0.0.255) const]_[_^Value^ Value][@(0.0.255) `&
]_[@3 src])&]
[s2;%% Converts Image from Value.&]
[s3; &]
[s4; &]
[s5;:Image`:`:Image`(const Image`&`): Image([@(0.0.255) const]_Image[@(0.0.255) `&]_[@3 img
])&]
[s2;%% Copy constructor.&]
[s3; &]
[s4; &]
[s5;:Image`:`:Image`(Image`(`*`)`(`)`): Image(Image_(`*fn)())&]
[s2;%% This function allow Image to be directly constructed from 
pointer to function returning the Image. This allows omitting 
parenthesis when passing Iml image constants as arguments.&]
[s3; &]
[s4; &]
[s5;:Image`:`:Image`(ImageBuffer`&`): Image([_^ImageBuffer^ ImageBuffer][@(0.0.255) `&]_[@3 b
])&]
[s2;%% Uses Image created in ImageBuffer. ImageBuffer is emptied 
by this operation.&]
[s3;%% &]
[s0;%% &]
[ {{10000t/25b/25@1 [s0;%% [* Standard cursors]]}}&]
[s9;%% Image class contains several standard mouse cursor Images 
as static member methods:&]
[s0;%% &]
[ {{3333:3333:3334f0;g0; [ {{10000>274; [s0; Arrow]
:: [s0; Wait]
:: [s0; IBeam]
:: [s0; No]
:: [s0; SizeAll]}}]
:: [ {{10000>274; [s0; SizeHorz]
:: [s0; SizeVert]
:: [s0; SizeTopLeft]
:: [s0; SizeTop]
:: [s0; SizeTopRight]}}]
:: [ {{10000>274; [s0; SizeLeft]
:: [s0; SizeRight]
:: [s0; SizeBottomLeft]
:: [s0; SizeBottom]
:: [s0; SizeBottomRight]}}]}}&]
[s3; ]]