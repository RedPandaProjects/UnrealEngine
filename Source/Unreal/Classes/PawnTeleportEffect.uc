//=============================================================================
// PawnTeleportEffect.
//=============================================================================
class PawnTeleportEffect expands Effects;

#exec MESH IMPORT MESH=TeleEffect3 ANIVFILE=MODELS\telepo_a.3D DATAFILE=MODELS\telepo_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=TeleEffect3 X=0 Y=0 Z=-200 YAW=64
#exec MESH SEQUENCE MESH=TeleEffect3 SEQ=All  STARTFRAME=0  NUMFRAMES=30
#exec MESH SEQUENCE MESH=TeleEffect3  SEQ=Burst  STARTFRAME=0  NUMFRAMES=30
#exec MESHMAP SCALE MESHMAP=TeleEffect3 X=0.06 Y=0.06 Z=0.16
#exec MESHMAP SETTEXTURE MESHMAP=TeleEffect3 NUM=1 TEXTURE=Default
 
#exec TEXTURE IMPORT NAME=T_PawnT FILE=MODELS\Dr_a03.pcx GROUP=Effects

function Initialize(pawn Other, bool bOut)
{

}

auto state Explode
{
	simulated function Tick(float DeltaTime)
	{
		if ( (Level.NetMode == NM_DedicatedServer) || !Level.bHighDetailMode )
		{
			Disable('Tick');
			return;
		}
		ScaleGlow = (Lifespan/Default.Lifespan);	
		LightBrightness = ScaleGlow*210.0;

	}

	simulated function BeginState()
	{
		PlaySound (EffectSound1,,0.5,,500);
		PlayAnim('All',0.6);
	}

	simulated function AnimEnd()
	{
		RemoteRole = ROLE_None;
		DrawType = DT_None;
	}
}

defaultproperties
{
     RemoteRole=ROLE_SimulatedProxy
     LifeSpan=1.000000
     AnimSequence=All
     DrawType=DT_Mesh
     Style=STY_Translucent
     Texture=Texture'Unreal.Effects.T_PawnT'
     Skin=Texture'Unreal.Effects.T_PawnT'
     Mesh=Mesh'Unreal.TeleEffect3'
     DrawScale=0.200000
     bParticles=True
     LightType=LT_Pulse
     LightEffect=LE_NonIncidence
     LightBrightness=255
     LightHue=170
     LightSaturation=96
     LightRadius=12
}
