//=============================================================================
// Keypoint, the base class of invisible actors which mark things.
//=============================================================================
class Keypoint expands Actor
	abstract
	intrinsic;

// Sprite.
#exec Texture Import File=Textures\Keypoint.pcx Name=S_Keypoint Mips=Off Flags=2

defaultproperties
{
     bStatic=True
     bHidden=True
     SoundVolume=0
     CollisionRadius=+00010.000000
     CollisionHeight=+00010.000000
	 Texture=S_Keypoint
}
