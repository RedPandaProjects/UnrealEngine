// ===================================================================
//  WaterTexture: Expands WaveTexture. Simple phongish water surface.
//  This is a built-in Unreal class and it shouldn't be modified.
// ===================================================================

class IceTexture expands FractalTexture
    intrinsic;


// Ice movement definitions.

enum PanningType
{
    SLIDE_Linear,
	SLIDE_Circular,
	SLIDE_Gestation,
	SLIDE_WavyX,
	SLIDE_WavyY,
};



enum TimingType
{
	TIME_FrameRateSync,
	TIME_RealTimeScroll,
};


// Persistent IceTexture Parameters.

var(IceLayer)		texture		GlassTexture;
var(IceLayer)		texture		SourceTexture;
var(IceLayer)       PanningType PanningStyle;
var(IceLayer)       TimingType  TimeMethod;
var(IceLayer)       byte		HorizPanSpeed;
var(IceLayer)       byte		VertPanSpeed;
var(IceLayer)       byte        Frequency;
var(IceLayer)       byte        Amplitude;

var(IceLayer)       bool		MoveIce;
var                 float       MasterCount;
var                 float		UDisplace;
var                 float		VDisplace;
var                 float       UPosition;
var                 float       VPosition;

// Transient IceTexture Parameters

var	transient		float       TickAccu;
var	transient		int         OldUDisplace;
var	transient		int         OldVDisplace;
var transient       texture     OldGlassTex;
var transient		texture     OldSourceTex;
var transient       int			LocalSource;
var transient       int			ForceRefresh;

defaultproperties
{
}
