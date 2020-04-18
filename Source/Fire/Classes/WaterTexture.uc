//=======================================================================================
//  WaterTexture: Base class for fractal water textures. Parent of Wave- and WetTexture.
//  This is a built-in Unreal class and it shouldn't be modified.
//=======================================================================================

class WaterTexture expands FractalTexture
	intrinsic
	abstract;

//
// Water drops.
//

enum WDrop
{
	DROP_FixedDepth			, // Fixed depth spot, A=depth
	DROP_PhaseSpot			, // Phased depth spot, A=frequency B=phase
	DROP_ShallowSpot		, // Shallower phased depth spot, A=frequency B=phase
	DROP_HalfAmpl           , // Half-amplitude (only 128+ values)
	DROP_RandomMover		, // Randomly moves around
	DROP_FixedRandomSpot	, // Fixed spot with random output
	DROP_WhirlyThing		, // Moves in small circles, A=speed B=depth
	DROP_BigWhirly			, // Moves in large circles, A=speed B=depth
	DROP_HorizontalLine		, // Horizontal line segment
	DROP_VerticalLine		, // Vertical line segment
	DROP_DiagonalLine1		, // Diagonal '/'
	DROP_DiagonalLine2		, // Diagonal '\'
	DROP_HorizontalOsc		, // Horizontal oscillating line segment
	DROP_VerticalOsc		, // Vertical oscillating line segment
	DROP_DiagonalOsc1		, // Diagonal oscillating '/'
	DROP_DiagonalOsc2		, // Diagonal oscillating '\'
	DROP_RainDrops			, // General random raindrops, A=depth B=distribution radius
	DROP_AreaClamp          , // Clamp spots to indicate shallow/dry areas
	DROP_LeakyTap			,
	DROP_DrippyTap			,
};


//
// Information about a single drop.
//

struct ADrop
{
    var WDrop Type;   // Drop type.
    var byte  Depth;  // Drop heat.
    var byte  X;      // Spark X location (0 - Xdimension-1).
    var byte  Y;      // Spark Y location (0 - Ydimension-1).

    var byte  ByteA;  // X-speed.
    var byte  ByteB;  // Y-speed.
    var byte  ByteC;  // Age, Emitter freq. etc.
    var byte  ByteD;  // Exp.Time etc.
};


//
// Water parameters.
//

var(WaterPaint)					WDrop  DropType;
var(WaterPaint)					byte   WaveAmp;

var(WaterPaint)					byte   FX_Frequency;
var(WaterPaint)					byte   FX_Phase;
var(WaterPaint)					byte   FX_Amplitude;
var(WaterPaint)					byte   FX_Speed;
var(WaterPaint)					byte   FX_Radius;
var(WaterPaint)					byte   FX_Size;
var(WaterPaint)					byte   FX_Depth;
var(WaterPaint)                 byte   FX_Time;

var								int    NumDrops;
var								ADrop  Drops[256];

var		 			transient   int    SourceFields;
var					transient   byte   RenderTable[1028];
var					transient	byte   WaterTable[1536];
var					transient	byte   WaterParity;
var					transient	int    OldWaveAmp;

defaultproperties
{
}
