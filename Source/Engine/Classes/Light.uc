//=============================================================================
// The light class.
//=============================================================================
class Light expands Actor;

#exec Texture Import File=Textures\S_Light.pcx  Name=S_Light Mips=Off Flags=2

defaultproperties
{
     bStatic=True
     bHidden=True
     bNoDelete=True
     Texture=S_Light
     CollisionRadius=+00024.000000
     CollisionHeight=+00024.000000
     LightType=LT_Steady
     LightBrightness=64
     LightSaturation=255
     LightRadius=64
     LightPeriod=32
     LightCone=128
     VolumeBrightness=64
     VolumeRadius=0
	 bMovable=False
}
