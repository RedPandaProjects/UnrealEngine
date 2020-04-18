//=============================================================================
// Bitmap: An abstract bitmap.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class Bitmap expands Object
	intrinsic;

// Texture format.
var const enum ETextureFormat
{
	TEXF_P8,
	TEXF_RGB32,
	TEXF_MAX
} Format;

// Palette.
var(Texture) palette Palette;

// Internal info.
var const byte  UBits, VBits;
var const int   USize, VSize;
var(Texture) const int UClamp, VClamp;
var const color MipZero;
var const color MaxColor;
var const int   InternalTime[2];

defaultproperties
{
	MipMult=1
	Diffuse=1
	Specular=1
	Scale=1
	Friction=1
	MipZero=(R=64,G=128,B=64,A=0)
	MaxColor=(R=255,G=255,B=255,A=255)
}
