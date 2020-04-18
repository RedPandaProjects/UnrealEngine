//=============================================================================
// For internal testing.
//=============================================================================
class TestInfo expands Info;

var() bool bTrue1;
var() bool bFalse1;
var() bool bTrue2;
var() bool bFalse2;
var bool bBool1;
var bool bBool2;
var() int xnum;
var float ppp;
var string[32] sxx;
var int MyArray[2];
var vector v1,v2;

const Pie=3.14;
const Str="Tim";
const Lotus=vect(1,2,3);

function TestX( bool bResource )
{
	local int n;
	n = int(bResource);
	MyArray[ int(bResource) ] = 0;
	MyArray[ int(bResource) ]++;
}

function bool RecurseTest()
{
	bBool1=true;
	return false;
}

function TestLimitor( class c )
{
	local class<actor> NewClass;
	NewClass = class<actor>( c );
}

static function int OtherStatic( int i )
{
	assert(i==246);
	assert(default.xnum==777);
	return 555;
}

static function int TestStatic( int i )
{
	assert(i==123);
	assert(default.xnum==777);
	assert(OtherStatic(i*2)==555);
}

function Tick( float DeltaTime )
{
	local class C;
	local class<testinfo> TC;

	v1=vect(1,2,3);
	v2=vect(2,4,6);
	assert(v1!=v2);
	assert(!(v1==v2));
	assert(v1==vect(1,2,3));
	assert(v2==vect(2,4,6));
	assert(vect(1,2,5)!=v1);
	assert(v1*2==v2);
	assert(v1==v2/2);

	assert(Pie==3.14);
	assert(Pie!=2);
	assert(Str=="Tim");
	assert(Str!="Bob");
	assert(Lotus==vect(1,2,3));

	assert(GetPropertyText("sxx")=="Tim");
	assert(GetPropertyText("ppp")!="123");
	assert(GetPropertyText("bogus")=="");
	xnum=345;
	assert(GetPropertyText("xnum")=="345");
	SetPropertyText("xnum","999");
	assert(xnum==999);
	assert(xnum!=666);

	assert(bTrue1==true);
	assert(bFalse1==false);
	assert(bTrue2==true);
	assert(bFalse2==false);

	assert(default.bTrue1==true);
	assert(default.bFalse1==false);
	assert(default.bTrue2==true);
	assert(default.bFalse2==false);

	assert(class'TestInfo'.default.bTrue1==true);
	assert(class'TestInfo'.default.bFalse1==false);
	assert(class'TestInfo'.default.bTrue2==true);
	assert(class'TestInfo'.default.bFalse2==false);

	TC=Class;
	assert(TC.default.bTrue1==true);
	assert(TC.default.bFalse1==false);
	assert(TC.default.bTrue2==true);
	assert(TC.default.bFalse2==false);

	C=Class;
	assert(class<testinfo>(C).default.bTrue1==true);
	assert(class<testinfo>(C).default.bFalse1==false);
	assert(class<testinfo>(C).default.bTrue2==true);
	assert(class<testinfo>(C).default.bFalse2==false);

	assert(default.xnum==777);
	TestStatic(123);
	TC.static.TestStatic(123);
	class<testinfo>(C).static.TestStatic(123);

	bBool2=RecurseTest();
	assert(bBool2==false);

	log( "All tests passed" );
}

function f();

state AA
{
	function f();
}
state BB
{
	function f();
}
state CCAA expands AA
{
	function f();
}
state DDAA expands AA
{
	function f();
}
state EEDDAA expands DDAA
{
	function f();
}

defaultproperties
{
	bTrue1=true
	bTrue2=true
	bHidden=false
	sxx="Tim"
	ppp=3.14
	xnum=777
}
