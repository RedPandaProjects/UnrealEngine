//=============================================================================
// Ambient sound, sits there and emits its sound.  This class is no different 
// than placing any other actor in a level and setting its ambient sound.
//=============================================================================
class AmbientSound expands Keypoint;

// Import the sprite.
#exec Texture Import File=Textures\Ambient.pcx Name=S_Ambient Mips=Off Flags=2

defaultproperties
{
	 Texture=S_Ambient
	 SoundRadius=64
	 SoundVolume=190
	 SoundPitch=64
}
