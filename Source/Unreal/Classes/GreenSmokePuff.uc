//=============================================================================
// GreenSmokePuff.
//=============================================================================
class GreenSmokePuff expands SpriteSmokePuff;

#exec OBJ LOAD FILE=textures\SmokeGreen.utx PACKAGE=Unreal.SmokeGreen

defaultproperties
{
     SSprites(0)=Texture'Unreal.SmokeGreen.gs_A00'
     SSprites(1)=Texture'Unreal.SmokeGreen.gs1_A00'
     SSprites(2)=Texture'Unreal.SmokeGreen.gs2_A00'
     SSprites(3)=Texture'Unreal.SmokeGreen.gs3_A00'
     SSprites(4)=None
     SSprites(5)=None
     SSprites(6)=None
     SSprites(7)=None
     SSprites(8)=None
     SSprites(9)=None
     NumSets=4
     RisingRate=80.000000
     Pause=0.070000
     bHighDetail=True
     Texture=Texture'Unreal.SmokeGreen.gs1_A00'
     DrawScale=2.000000
}
