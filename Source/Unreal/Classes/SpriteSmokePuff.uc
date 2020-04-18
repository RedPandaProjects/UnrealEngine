//=============================================================================
// SpriteSmokePuff.
//=============================================================================
class SpriteSmokePuff expands AnimSpriteEffect;

#exec OBJ LOAD FILE=textures\SmokeGray.utx PACKAGE=Unreal.SmokeGray

var() Texture SSprites[20];
var() int NumSets;
var() float RisingRate;
	
simulated function PostBeginPlay()
{
	Velocity = Vect(0,0,1)*RisingRate;
	Texture = SSPrites[int(FRand()*NumSets*0.97)];
	if (Texture == None) Texture = Texture'S_Actor';
}

defaultproperties
{
     SSprites(0)=Texture'Unreal.SmokeGray.sp_A00'
     SSprites(1)=Texture'Unreal.SmokeGray.sp1_A00'
     SSprites(2)=Texture'Unreal.SmokeGray.sp2_A00'
     SSprites(3)=Texture'Unreal.SmokeGray.sp3_A00'
     SSprites(4)=Texture'Unreal.SmokeGray.sp4_A00'
     SSprites(5)=Texture'Unreal.SmokeGray.sp5_A00'
     SSprites(6)=Texture'Unreal.SmokeGray.sp6_A00'
     SSprites(7)=Texture'Unreal.SmokeGray.sp7_A00'
     SSprites(8)=Texture'Unreal.SmokeGray.sp8_A00'
     SSprites(9)=Texture'Unreal.SmokeGray.sp9_A00'
     NumSets=10
     RisingRate=50.000000
     NumFrames=8
     Pause=0.050000
     Physics=PHYS_Projectile
     RemoteRole=ROLE_SimulatedProxy
     LifeSpan=1.500000
     DrawType=DT_SpriteAnimOnce
     Style=STY_Translucent
     Texture=Texture'Unreal.SmokeGray.sp1_A00'
     DrawScale=2.000000
     bMeshCurvy=False
     LightType=LT_None
     LightBrightness=10
     LightHue=0
     LightSaturation=255
     LightRadius=7
     bCorona=False
     bNetOptional=True
}
