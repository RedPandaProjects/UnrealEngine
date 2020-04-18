//=============================================================================
// Object: The base class all objects.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class Object
	intrinsic;

//-----------------------------------------------------------------------------
// UObject variables.

// Internal stuff.
var intrinsic private const int ObjectInternal[6];
var intrinsic const object Parent;
var intrinsic const int ObjectFlags;
var(Object) intrinsic const editconst name Name;
var(Object) intrinsic const editconst class Class;

//=============================================================================
// Unreal base structures.

// A globally unique identifier.
struct Guid
{
	var int A, B, C, D;
};

// A dynamic array.
struct DynamicArray
{
	var const int Num, Max, Ptr;
};

// A dynamic string.
struct DynamicString
{
};

// A point or direction vector in 3d space.
struct Vector
{
	var() config float X, Y, Z;
};

// A plane definition in 3d space.
struct Plane expands Vector
{
	var() config float W;
};

// An orthogonal rotation in 3d space.
struct Rotator
{
	var() config int Pitch, Yaw, Roll;
};

// An arbitrary coordinate system in 3d space.
struct Coords
{
	var() config vector Origin, XAxis, YAxis, ZAxis;
};

// A scale and sheering.
struct Scale
{
	var() config vector Scale;
	var() config float SheerRate;
	var() config enum ESheerAxis
	{
		SHEER_None,
		SHEER_XY,
		SHEER_XZ,
		SHEER_YX,
		SHEER_YZ,
		SHEER_ZX,
		SHEER_ZY,
	} SheerAxis;
};

// A color.
struct Color
{
	var() config byte R, G, B, A;
};

// A bounding box.
struct BoundingBox
{
	var vector Min, Max;
	var byte IsValid;
};

// A bounding box sphere together.
struct BoundingVolume expands boundingbox
{
	var plane Sphere;
};

//=============================================================================
// Constants.

const MaxInt = 0x7fffffff;
const Pi     = 3.1415926535897932;

//=============================================================================
// Basic intrinsic operators and functions.

// Bool operators.
intrinsic(129) static final preoperator  bool  !  ( bool A );
intrinsic(242) static final operator(24) bool  == ( bool A, bool B );
intrinsic(243) static final operator(26) bool  != ( bool A, bool B );
intrinsic(130) static final operator(30) bool  && ( bool A, skip bool B );
intrinsic(131) static final operator(30) bool  ^^ ( bool A, bool B );
intrinsic(132) static final operator(32) bool  || ( bool A, skip bool B );

// Byte operators.
intrinsic(133) static final operator(34) byte *= ( out byte A, byte B );
intrinsic(134) static final operator(34) byte /= ( out byte A, byte B );
intrinsic(135) static final operator(34) byte += ( out byte A, byte B );
intrinsic(136) static final operator(34) byte -= ( out byte A, byte B );
intrinsic(137) static final preoperator  byte ++ ( out byte A );
intrinsic(138) static final preoperator  byte -- ( out byte A );
intrinsic(139) static final postoperator byte ++ ( out byte A );
intrinsic(140) static final postoperator byte -- ( out byte A );

// Integer operators.
intrinsic(141) static final preoperator  int  ~  ( int A );
intrinsic(143) static final preoperator  int  -  ( int A );
intrinsic(144) static final operator(16) int  *  ( int A, int B );
intrinsic(145) static final operator(16) int  /  ( int A, int B );
intrinsic(146) static final operator(20) int  +  ( int A, int B );
intrinsic(147) static final operator(20) int  -  ( int A, int B );
intrinsic(148) static final operator(22) int  << ( int A, int B );
intrinsic(149) static final operator(22) int  >> ( int A, int B );
intrinsic(150) static final operator(24) bool <  ( int A, int B );
intrinsic(151) static final operator(24) bool >  ( int A, int B );
intrinsic(152) static final operator(24) bool <= ( int A, int B );
intrinsic(153) static final operator(24) bool >= ( int A, int B );
intrinsic(154) static final operator(24) bool == ( int A, int B );
intrinsic(155) static final operator(26) bool != ( int A, int B );
intrinsic(156) static final operator(28) int  &  ( int A, int B );
intrinsic(157) static final operator(28) int  ^  ( int A, int B );
intrinsic(158) static final operator(28) int  |  ( int A, int B );
intrinsic(159) static final operator(34) int  *= ( out int A, float B );
intrinsic(160) static final operator(34) int  /= ( out int A, float B );
intrinsic(161) static final operator(34) int  += ( out int A, int B );
intrinsic(162) static final operator(34) int  -= ( out int A, int B );
intrinsic(163) static final preoperator  int  ++ ( out int A );
intrinsic(164) static final preoperator  int  -- ( out int A );
intrinsic(165) static final postoperator int  ++ ( out int A );
intrinsic(166) static final postoperator int  -- ( out int A );

// Integer functions.
intrinsic(167) static final Function     int  Rand  ( int Max );
intrinsic(249) static final function     int  Min   ( int A, int B );
intrinsic(250) static final function     int  Max   ( int A, int B );
intrinsic(251) static final function     int  Clamp ( int V, int A, int B );

// Float operators.
intrinsic(169) static final preoperator  float -  ( float A );
intrinsic(170) static final operator(12) float ** ( float A, float B );
intrinsic(171) static final operator(16) float *  ( float A, float B );
intrinsic(172) static final operator(16) float /  ( float A, float B );
intrinsic(173) static final operator(18) float %  ( float A, float B );
intrinsic(174) static final operator(20) float +  ( float A, float B );
intrinsic(175) static final operator(20) float -  ( float A, float B );
intrinsic(176) static final operator(24) bool  <  ( float A, float B );
intrinsic(177) static final operator(24) bool  >  ( float A, float B );
intrinsic(178) static final operator(24) bool  <= ( float A, float B );
intrinsic(179) static final operator(24) bool  >= ( float A, float B );
intrinsic(180) static final operator(24) bool  == ( float A, float B );
intrinsic(210) static final operator(24) bool  ~= ( float A, float B );
intrinsic(181) static final operator(26) bool  != ( float A, float B );
intrinsic(182) static final operator(34) float *= ( out float A, float B );
intrinsic(183) static final operator(34) float /= ( out float A, float B );
intrinsic(184) static final operator(34) float += ( out float A, float B );
intrinsic(185) static final operator(34) float -= ( out float A, float B );

// Float functions.
intrinsic(186) static final function     float Abs   ( float A );
intrinsic(187) static final function     float Sin   ( float A );
intrinsic(188) static final function     float Cos   ( float A );
intrinsic(189) static final function     float Tan   ( float A );
intrinsic(190) static final function     float Atan  ( float A );
intrinsic(191) static final function     float Exp   ( float A );
intrinsic(192) static final function     float Loge  ( float A );
intrinsic(193) static final function     float Sqrt  ( float A );
intrinsic(194) static final function     float Square( float A );
intrinsic(195) static final function     float FRand ();
intrinsic(244) static final function     float FMin  ( float A, float B );
intrinsic(245) static final function     float FMax  ( float A, float B );
intrinsic(246) static final function     float FClamp( float V, float A, float B );
intrinsic(247) static final function     float Lerp  ( float Alpha, float A, float B );
intrinsic(248) static final function     float Smerp ( float Alpha, float A, float B );

// String operators.
intrinsic(228) static final operator(40) string[255] $  ( coerce string[255] A, coerce String[255] B );
intrinsic(197) static final operator(24) bool        <  ( string[255] A, string[255] B );
intrinsic(198) static final operator(24) bool        >  ( string[255] A, string[255] B );
intrinsic(199) static final operator(24) bool        <= ( string[255] A, string[255] B );
intrinsic(200) static final operator(24) bool        >= ( string[255] A, string[255] B );
intrinsic(201) static final operator(24) bool        == ( string[255] A, string[255] B );
intrinsic(202) static final operator(26) bool        != ( string[255] A, string[255] B );
intrinsic(168) static final operator(24) bool        ~= ( string[255] A, string[255] B );

// String functions.
intrinsic(204) static final function int         Len   ( coerce string[255] S );
intrinsic(205) static final function int         InStr ( coerce string[255] S, coerce string[255] t);
intrinsic(206) static final function string[255] Mid   ( coerce string[255] S, int i, optional int j );
intrinsic(207) static final function string[255] Left  ( coerce string[255] S, int i );
intrinsic(208) static final function string[255] Right ( coerce string[255] S, int i );
intrinsic(209) static final function string[255] Caps  ( coerce string[255] S );
intrinsic      static final function string[16]  Chr   ( int i );
intrinsic      static final function int         Asc   ( string[255] S );

// Object operators.
intrinsic(114) static final operator(24) bool == ( Object A, Object B );
intrinsic(119) static final operator(26) bool != ( Object A, Object B );

// Name operators.
intrinsic(254) static final operator(24) bool == ( name A, name B );
intrinsic(255) static final operator(26) bool != ( name A, name B );

//=============================================================================
// General functions.

// Logging.
intrinsic(231) final static function Log( coerce string[240] S, optional name Tag );
intrinsic(232) final static function Warn( coerce string[240] S );
intrinsic static function string[192] Localize( name SectionName, name KeyName, name PackageName );

// Goto state and label.
intrinsic(113) final function GotoState( optional name NewState, optional name Label );
intrinsic(281) final function bool IsInState( name TestState );
intrinsic(284) final function name GetStateName();

// Classes.
intrinsic(258) static final function bool ClassIsChildOf( class TestClass, class ParentClass );

// Probe messages.
intrinsic(117) final function Enable( name ProbeFunc );
intrinsic(118) final function Disable( name ProbeFunc );

// Properties.
intrinsic final function string[192] GetPropertyText( string[32] PropName );
intrinsic final function SetPropertyText( string[32] PropName, string[192] PropValue );
intrinsic static final function name GetEnum( object E, int i );
intrinsic static final function object DynamicLoadObject( string[32] ObjectName, class ObjectClass );

// Configuration.
intrinsic(536) final function SaveConfig();
intrinsic(543) final function ResetConfig();

//=============================================================================
// Engine notification functions.

//
// Called immediately when entering a state, while within
// the GotoState call that caused the state change.
//
event BeginState();

//
// Called immediately before going out of the current state,
// while within the GotoState call that caused the state change.
// 
event EndState();

defaultproperties
{
}
