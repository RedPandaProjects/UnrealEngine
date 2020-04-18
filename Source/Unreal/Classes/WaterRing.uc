//=============================================================================
// WaterRing.
//=============================================================================
class WaterRing expands RingExplosion;

#exec OBJ LOAD FILE=Textures\fireeffect56.utx  PACKAGE=Unreal.Effect56

simulated function SpawnEffects()
{
}

defaultproperties
{
     Skin=Unreal.Effect56.fireeffect56
     Class=Unreal.WaterRing
	 bNetOptional=True
}
