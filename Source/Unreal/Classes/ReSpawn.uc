//=============================================================================
// ReSpawn.
//=============================================================================
class ReSpawn expands Effects;

#exec MESH IMPORT MESH=TeleEffect2 ANIVFILE=MODELS\telepo_a.3D DATAFILE=MODELS\telepo_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=TeleEffect2 X=0 Y=0 Z=-200 YAW=0
#exec MESH SEQUENCE MESH=TeleEffect2 SEQ=All  STARTFRAME=0  NUMFRAMES=30
#exec MESH SEQUENCE MESH=TeleEffect2  SEQ=Burst  STARTFRAME=0  NUMFRAMES=30
#exec MESHMAP SCALE MESHMAP=TeleEffect2 X=0.03 Y=0.03 Z=0.06
#exec MESHMAP SETTEXTURE MESHMAP=TeleEffect2 NUM=1 TEXTURE=Default
#exec AUDIO IMPORT FILE="Sounds\Pickups\Respawn1a.WAV" NAME="RespawnSound" GROUP="Generic"

simulated function BeginPlay()
{
	Super.BeginPlay();
	Playsound(EffectSound1);
	PlayAnim('All',0.8);
}

auto state Explode
{
	simulated function Tick( float DeltaTime )
	{
		ScaleGlow = (Lifespan/Default.Lifespan);	
		LightBrightness = ScaleGlow*210.0;
	}

	simulated function AnimEnd()
	{
		RemoteRole = ROLE_None;
		Destroy();
	}
}

defaultproperties
{
	 EffectSound1=RespawnSound
     RemoteRole=ROLE_SimulatedProxy
     LifeSpan=1.000000
     DrawType=DT_Mesh
     Style=STY_Translucent
     Texture=Texture'Unreal.DBEffect.de_A00'
     Mesh=Mesh'Unreal.TeleEffect2'
     DrawScale=0.250000
     bUnlit=True
     bParticles=True
     LightType=LT_Steady
     LightEffect=LE_NonIncidence
     LightBrightness=210
     LightHue=30
     LightSaturation=224
     LightRadius=8
}
