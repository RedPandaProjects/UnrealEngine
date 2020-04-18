//=============================================================================
// FlameExplosion.
//=============================================================================
class FlameExplosion expands AnimSpriteEffect;

#exec TEXTURE IMPORT NAME=ExplosionPal2 FILE=textures\exppal.pcx GROUP=Effects
#exec OBJ LOAD FILE=textures\FlameEffect.utx PACKAGE=Unreal.FlameEffect

#exec AUDIO IMPORT FILE="sounds\general\expl04.wav" NAME="Expl04" GROUP="General"

simulated function PostBeginPlay()
{
	local actor a;

	Super.PostBeginPlay();
	if (!Level.bHighDetailMode) Drawscale = 1.4;
	else 
	{	
		a = Spawn(class'ShortSmokeGen');
		a.RemoteRole = ROLE_None;	
	}
	PlaySound (EffectSound1,,3.0);	
}

defaultproperties
{
     NumFrames=8
     Pause=0.050000
     EffectSound1=Sound'Unreal.General.Expl04'
     RemoteRole=ROLE_SimulatedProxy
     LifeSpan=0.500000
     DrawType=DT_SpriteAnimOnce
     Style=STY_Translucent
     Texture=Texture'Unreal.FlameEffect.e_a01'
     Skin=Texture'Unreal.Effects.ExplosionPal2'
     DrawScale=2.800000
     bMeshCurvy=False
     LightType=LT_TexturePaletteOnce
     LightEffect=LE_NonIncidence
     LightBrightness=159
     LightHue=32
     LightSaturation=79
     LightRadius=8
     bCorona=False
}
