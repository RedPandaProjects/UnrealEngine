//=============================================================================
// BloodPuff.
//=============================================================================
class BloodPuff expands SpriteSmokePuff;

#exec OBJ LOAD FILE=textures\BloodyPuff.utx PACKAGE=Unreal.BloodyPuff

defaultproperties
{
     SSprites(0)=Texture'Unreal.BloodyPuff.bp_A01'
     SSprites(1)=Texture'Unreal.BloodyPuff.bp8_a00'
     SSprites(2)=Texture'Unreal.BloodyPuff.Bp6_a00'
     SSprites(3)=None
     SSprites(4)=None
     SSprites(5)=None
     SSprites(6)=None
     SSprites(7)=None
     SSprites(8)=None
     SSprites(9)=None
     NumSets=3
     RisingRate=-50.000000
     bHighDetail=True
     LifeSpan=0.500000
     Texture=Texture'Unreal.BloodyPuff.bp_A01'
     DrawScale=2.000000
}
