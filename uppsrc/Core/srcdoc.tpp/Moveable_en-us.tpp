topic "Moveable";
[l288;i704;a17;O9;~~~.992;2 $$1,0#10431211400427159095818037425705:param]
[a83;*R6 $$2,5#31310162474203024125188417583966:caption]
[b83;*2 $$3,5#07864147445237544204411237157677:title]
[b167;a42;C2 $$4,6#40027414424643823182269349404212:item]
[b42;a42;ph2 $$5,5#45413000475342174754091244180557:text]
[l288;a17;2 $$6,6#27521748481378242620020725143825:desc]
[l321;t246;C@5;1 $$7,7#20902679421464641399138805415013:code]
[b2503;2 $$8,0#65142375456100023862071332075487:separator]
[*@(0.0.255)2 $$9,0#83433469410354161042741608181528:base]
[t4167;C2 $$10,0#37138531426314131251341829483380:class]
[l288;a17;*1 $$11,11#70004532496200323422659154056402:requirement]
[i417;b42;a42;O9;~~~.416;2 $$12,12#10566046415157235020018451313112:tparam]
[b167;C2 $$13,13#92430459443460461911108080531343:item1]
[i288;a42;O9;C2 $$14,14#77422149456609303542238260500223:item2]
[*@2$(0.128.128)2 $$15,15#34511555403152284025741354420178:NewsDate]
[l321;*C$7;2 $$16,16#03451589433145915344929335295360:result]
[l321;b83;a83;*C$7;2 $$17,17#07531550463529505371228428965313:result`-line]
[l160;t4167;*C+117 $$18,5#88603949442205825958800053222425:package`-title]
[2 $$0,0#00000000000000000000000000000000:Default]
[{_}%EN-US 
[s2; Moveable&]
[s5; First important note: U`+`+ Moveable is not to be confused with 
C`+`+ standard library move semantics.&]
[s5; Moveable concept represents basic requirement for types stored 
in Vector flavor of containers (namely Vector, BiVector, Index, 
VectorMap, InVector, SortedIndex, SortedVectorMap). To explain 
what it is and why it is so important let us first create a very 
primitive Vector`-like container template&]
[s0; &]
[s7; template <class T>&]
[s7; class SimpleVector `{&]
[s7; -|T  `*vector;&]
[s7; -|int capacity;&]
[s7; -|int items;&]
[s7; &]
[s7; -|void [* Expand()] `{&]
[s7; -|-|capacity `= max(1, 2 `* capacity);&]
[s7; -|-|T `*newvector `= (T `*) new char`[capacity `* sizeof(T)`];&]
[s7; -|-|for(int i `= 0; i < items; i`+`+) `{&]
[s7; -|-|-|[* new(newvector`[i`]) T(vector`[i`])];&]
[s7; -|-|-|[* vector`[i`].T`::`~T();]&]
[s7; -|-|`}&]
[s7; -|-|delete`[`] (char `*) vector;&]
[s7; -|-|vector `= newvector;&]
[s7; -|`}&]
[s7; public:&]
[s7; -|void Add(const T`& x) `{&]
[s7; -|-|if(items >`= capacity) [* Expand()];&]
[s7; -|-|new(vector`[items`+`+`]) T(x);&]
[s7; -|`}&]
[s7; -|T`& operator`[`](int i) `{ return vector`[i`]; `}&]
[s7; -|SimpleVector() `{&]
[s7; -|-|vector `= NULL;&]
[s7; -|-|capacity `= items `= 0;&]
[s7; -|`}&]
[s7; -|`~SimpleVector() `{&]
[s7; -|-|for(int i `= 0; i < items; i`+`+)&]
[s7; -|-|-|vector`[i`].T`::`~T();&]
[s7; -|-|delete`[`] (char `*)vector;&]
[s7; -|`}&]
[s7; `};&]
[s0; &]
[s5; This [* SimpleVector] stores added elements in the [* vector] member 
field. If there is no more empty storage space left in [* vector], 
[* SimpleVector] simply doubles its capacity using [* Expand] method. 
This method is what interests us `- because [* Expand ]requires 
means to copy values of elements from the original memory area 
to the newly allocated one. The version above uses placement 
new and copy constructor for this purpose. This also means that 
[* SimpleVector] requires T to have copy constructor (in standard 
C`+`+ library terms: it must be [/ copy`-constructible]). Now let 
us create a typical element that can be stored in such container&]
[s0; &]
[s7; class SimpleString `{&]
[s7; -|char `*text;&]
[s7; public:&]
[s7; -|SimpleString(const char `*txt) `{&]
[s7; -|-|text `= new char`[strlen(txt)`+1`];&]
[s7; -|-|strcpy(text, txt);&]
[s7; -|`}&]
[s7; -|SimpleString(const SimpleString`& s) `{&]
[s7; -|-|text `= new char`[strlen(s.text)`+1`];&]
[s7; -|-|strcpy(text, s.text);&]
[s7; -|`}&]
[s7; -|void operator`=(const SimpleString`& s) `{&]
[s7; -|-|delete`[`] text;&]
[s7; -|-|text `= new char`[strlen(s.text)`+1`];&]
[s7; -|-|strcpy(text, s.text);-|-|&]
[s7; -|`}&]
[s7; -|`~SimpleString() `{&]
[s7; -|-|delete`[`] text;&]
[s7; -|`}&]
[s7; `};&]
[s0; &]
[s5; and see what happens when [* SimpleVector] of [* SimpleString]s 
is expanded: First, copies of all elements are created, that means 
allocating new storage for [* text] member of new element and copying 
source [* text] to it using [* strcpy]. A moment later, [* Expand] 
invokes destructor for element, thus deleting all [* text]s in 
the original elements. Does not it seem we are wasting a lot 
of CPU cycles just to make copies of things that we throw away 
a moment later anyway? What if instead of making copies we could 
find a way to transfer original elements`' [* text] members to 
new elements and somehow disallow [* delete`[`] text] in destructor? 
See how primitive it can be:&]
[s0; &]
[s7; template <class T>&]
[s7; class SimpleVector `{&]
[s7; -|T  `*vector;&]
[s7; -|int capacity;&]
[s7; -|int items;&]
[s7; &]
[s7; -|void Expand() `{&]
[s7; -|-|capacity `= max(1, 2 `* capacity);&]
[s7; -|-|T `*newvector `= (T `*) new char`[capacity `* sizeof(T)`];&]
[s7; -|-|[* memcpy(newvector, vector, items `* sizeof(T));]&]
[s7; -|-|delete`[`](char `*)vector;&]
[s7; -|-|vector `= newvector;&]
[s7; -|`}&]
[s7; public:&]
[s7; -|void Add(const T`& x) `{&]
[s7; -|-|if(items >`= capacity) Expand();&]
[s7; -|-|new(vector`[items`+`+`]) T(x);&]
[s7; -|`}&]
[s7; -|SimpleVector() `{&]
[s7; -|-|vector `= NULL;&]
[s7; -|-|capacity `= items `= 0;&]
[s7; -|`}&]
[s7; -|`~SimpleVector() `{&]
[s7; -|-|for(int i `= 0; i < items; i`+`+)&]
[s7; -|-|-|vector`[i`].T`::`~T();&]
[s7; -|-|delete`[`] (char `*)vector;&]
[s7; -|`}&]
[s7; `};&]
[s0; &]
[s5; For the moment please ignore fact that by using memcpy to move 
non`-POD types we are violating C`+`+ standard (we will discuss 
it later). Now, what we get here is exactly what we wanted `- 
instead of a series of copy construction and destruction we simply 
copy raw binary data to the new location. This way we simply 
transfer the old value in the [* text ]member of elements into 
the new expanded [* vector]. We need to invoke neither copy constructor 
nor destructor when expanding vector. Not a single CPU cycle 
is lost.&]
[s5; The types that can be moved in memory using memcpy are called 
[*/ moveable.]&]
[s5; Clearly not all types are moveable. Being moveable has a lot 
to do with [/ not] storing references to the object itself or to 
it`'s parts. Example:&]
[s0; &]
[s7; struct Link `{&]
[s7; -|Link `*prev;&]
[s7; public:&]
[s7; -|Link()        `{ prev `= nullptr; `}&]
[s7; -|Link(Link `*p) `{ prev `= p; `}&]
[s7; `};&]
[s0; &]
[s5; is [* not] moveable, because memcpy`-ing to a new location would 
break the existing links. All of the following requirements should 
be fullfilled for moveable types:&]
[s5;l128;i150;O0; It does not have any virtual method nor virtual 
base class.&]
[s5;l128;i150;O0; Base classes (if any) and any instance member variables 
are moveable.&]
[s5;l128;i150;O0; No references or pointers are stored to the object 
itself or to subobjects in the methods of the type, into variables 
that exist after the method finishes execution. &]
[s5; Fundamental types fulfills these requirements so they are moveable.&]
[s5; Example:&]
[s0; &]
[s7; struct Foo;&]
[s7; &]
[s7; Foo `*global`_foo;&]
[s0; &]
[s7; struct Foo `{&]
[s7; -|int a;&]
[s7; -|Foo `*foo;&]
[s7; -|int `*ptr;&]
[s7; public:&]
[s7; -|void Set(Foo `* f) `{&]
[s7; -|-|foo `= f;&]
[s7; -|`}&]
[s7; -|void Ok1() `{&]
[s7; -|-|Foo `*x `= this;&]
[s7; -|// local variable will not exist outside method&]
[s7; -|// `-> does not prevent Foo from being moveable&]
[s7; -|`}&]
[s7; -|void Ok2() `{&]
[s7; -|-|memset(`&a, 0, sizeof(int));&]
[s7; -|// pointer will not exist outside method&]
[s7; -|// `-> does not prevent Foo from being moveable&]
[s7; -|`}&]
[s7; -|void Bad1() `{&]
[s7; -|-|foo `= this;&]
[s7; -|// member variable foo exists outside method&]
[s7; -|// `-> makes Foo non`-moveable&]
[s7; -|`}&]
[s7; -|void Bad2() `{&]
[s7; -|-|ptr `= `&a;&]
[s7; -|// pointer to subobject stored, ptr exists outside method&]
[s7; -|// `-> makes Foo non`-moveable&]
[s7; -|`}&]
[s7; -|void Bad3() `{&]
[s7; -|-|global`_foo `= this;&]
[s7; -|// pointer stored to global variable&]
[s7; -|// `-> makes Foo non`-moveable&]
[s7; -|`}&]
[s7; -|void Bad4(Foo`& another) `{&]
[s7; -|-|another.Set(this);&]
[s7; -|// pointer stored indirectly in object that exist outside method&]
[s7; -|// `-> makes Foo non`-moveable&]
[s7; -|`}&]
[s7; `};&]
[s0; &]
[s5; These requirements satisfies fairly large number of types, incidentally 
most of the types you ever wanted to store in a container of 
elements of a single type are moveable. Most important, all NTL 
containers [* are ]moveable.&]
[s5; Now we have an effective method how to organize the storing 
of elements in containers. We have to deal with the fact that 
being moveable is part of an object`'s interface, and we should 
ensure that only movable elements are stored into NTL containers. 
For this we need a way to declare at compile time that a certain 
type is moveable and also a way to check it.&]
[s5; To achieve this goal, you can mark moveable types by deriving 
them from the [* Moveable] template class e.g.:&]
[s0; &]
[s7; class SimpleString : Moveable<SimpleString> `{ ... `}&]
[s0; &]
[s5; Alternatively the [* NTL`_MOVEABLE ]macro can be used to mark 
types as moveable if the class interface can not be changed, 
such as in:&]
[s0; &]
[s7; NTL`_MOVEABLE(std`::string);&]
[s0; &]
[s5; Now that we can mark types as moveable, we need a way to check 
a type for being moveable. This is done by adding the line:&]
[s0; &]
[s7; AssertMoveable<T>()&]
[s0; &]
[s5; to one of the methods of a template that gets compiled for any 
template argument `- the destructor is the most obvious place. 
The [* AssertMovable] function is defined only if [* T] is marked 
as moveable, so it results in compilation error for non`-moveable 
T types.&]
[s5; Finally, time has come to deal with the C`+`+ standard. Current 
C`+`+ defines memcpy only for POD types. The moveable concept 
requires memcpy of non`-POD types to be defined. In fact, difference 
between POD and moveable non`-POD types is the existence of constructors 
and non`-virtual destructor. To get things work, all we need 
is that the result of memcpy`-ing non`-POD type [* T ]is same as 
memcpy`-ing the POD [* T1 ]type which you would get by removing 
the destructor and the constructors from [* T]. While this operation 
is still undefined by C`+`+, it is really hard to imagine an 
optimal C`+`+ implementation that would break this rule. Indeed, 
all current implementation we have met so far support moveable 
semantics in the way we have defined here. Performance and other 
gains realized by using the moveable concept are too big to ignore.&]
[s5; ]]