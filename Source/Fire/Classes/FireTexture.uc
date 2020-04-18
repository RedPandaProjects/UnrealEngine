//=============================================================================
// FireTexture: A FireEngine fire texture.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class FireTexture expands FractalTexture
	intrinsic;

//
// Spark types.
//
enum ESpark
{	
	SPARK_Burn				,
	SPARK_Sparkle			,
	SPARK_Pulse				,
	SPARK_Signal			,
	SPARK_Blaze				,
	SPARK_OzHasSpoken		,
	SPARK_Cone				,
	SPARK_BlazeRight		,
	SPARK_BlazeLeft			,
	SPARK_Cylinder			,
	SPARK_Cylinder3D		,
	SPARK_Lissajous 		,
	SPARK_Jugglers   		,
	SPARK_Emit				,
    SPARK_Fountain			,
	SPARK_Flocks			,
	SPARK_Eels				,
	SPARK_Organic			,
	SPARK_WanderOrganic		,
	SPARK_RandomCloud		,
	SPARK_CustomCloud		,
	SPARK_LocalCloud		,
	SPARK_Stars				,
	SPARK_LineLightning		,
	SPARK_RampLightning		,
    SPARK_SphereLightning	,
    SPARK_Wheel				,
	SPARK_Gametes    		,
	SPARK_Sprinkler			,
};


// Draw mode types
enum DMode
{
	DRAW_Normal  ,
	DRAW_Lathe   ,
	DRAW_Lathe_2 ,
	DRAW_Lathe_3 ,
	DRAW_Lathe_4 ,
};



//
// Information about a single spark.
//

struct Spark
{
    var ESpark Type;   // Spark type.
    var byte   Heat;   // Spark heat.
    var byte   X;      // Spark X location (0 - Xdimension-1).
    var byte   Y;      // Spark Y location (0 - Ydimension-1).

    var byte   ByteA;  // X-speed.
    var byte   ByteB;  // Y-speed.
    var byte   ByteC;  // Age, Emitter freq.
    var byte   ByteD;  // Exp.Time.
};


//
// Persistent fire parameters.
//

var(FirePaint)	ESpark  SparkType;
var(FirePaint)	byte    RenderHeat;
var(FirePaint)	bool    bRising;

var(FirePaint)	byte    FX_Heat;
var(FirePaint)	byte    FX_Size;
var(FirePaint)  byte    FX_AuxSize;
var(FirePaint)  byte    FX_Area;
var(FirePaint)	byte    FX_Frequency;
var(FirePaint)	byte    FX_Phase;
var(FirePaint)	byte    FX_HorizSpeed;
var(FirePaint)	byte    FX_VertSpeed;

var(FirePaint)  DMode   DrawMode;
var(FirePaint)  int     SparksLimit;

var             int     NumSparks;
var transient   DynamicArray Sparks;

//
// Transient fire parameters.
//

var transient   int     OldRenderHeat;
var transient	byte	RenderTable[1028];
var transient	byte	StarStatus;
var transient   byte    PenDownX;
var transient   byte    PenDownY;

defaultproperties
{
}


