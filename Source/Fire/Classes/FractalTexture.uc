//=============================================================================
// FractalTexture: Base class of FireEngine fractal textures.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================

class FractalTexture expands Texture
	intrinsic
	abstract;

// Transient editing parameters.
var transient   int  UMask;
var transient   int  VMask;
var transient	int  LightOutput;
var transient	int  SoundOutput;
var	transient   int  GlobalPhase;
var transient	byte DrawPhase;
var transient	byte AuxPhase;


defaultproperties
{
}
