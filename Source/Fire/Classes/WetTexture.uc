//=============================================================================
// WetTexture: Expands WaterTexture. Water amplitude used as displacement.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================

class WetTexture expands WaterTexture
	intrinsic;

var(WaterPaint)				texture     SourceTexture;
var              transient  texture     OldSourceTex;
var transient int LocalSourceBitmap;

defaultproperties
{
}
