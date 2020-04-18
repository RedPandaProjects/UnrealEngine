//=============================================================================
// BlackSmoke.
//=============================================================================
class BlackSmoke expands SpriteSmokePuff;

#exec OBJ LOAD FILE=textures\SmokeBlack.utx PACKAGE=Unreal.SmokeBlack

defaultproperties
{
     SSprites(0)=Texture'Unreal.SmokeBlack.bs_a00'
     SSprites(1)=Texture'Unreal.SmokeBlack.bs2_a00'
     SSprites(2)=Texture'Unreal.SmokeBlack.bs3_a00'
     SSprites(3)=Texture'Unreal.SmokeBlack.bs4_a00'
     SSprites(4)=None
     SSprites(5)=None
     SSprites(6)=None
     SSprites(7)=None
     SSprites(8)=None
     SSprites(9)=None
     NumSets=4
     RisingRate=70.000000
     bHighDetail=True
     Style=STY_Modulated
     Texture=Texture'Unreal.SmokeBlack.bs2_a00'
     DrawScale=2.200000
}
