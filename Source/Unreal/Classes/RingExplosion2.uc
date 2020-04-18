//=============================================================================
// RingExplosion2.
//=============================================================================
class RingExplosion2 expands RingExplosion;

#exec OBJ LOAD FILE=Textures\fireeffect51.utx PACKAGE=Unreal.Effect51
#exec AUDIO IMPORT FILE="Sounds\Tazer\ASMDEx3.wav" NAME="SpecialExpl" GROUP="General"

simulated function PostBeginPlay()
{
	if ( Level.NetMode != NM_DedicatedServer )
	{
		PlayAnim( 'Explosion', 0.2 );
		if (level.bHighDetailMode) SpawnEffects();
	}	
}

simulated function SpawnEffects()
{
	local Actor a;

	PlaySound(Sound'SpecialExpl',,20.0,,1000,0.6);
	PlaySound(Sound'SpecialExpl',,20.0,,1000,0.6);	
	a = Spawn(class'PurpleLight');
	a.RemoteRole = ROLE_None;
	a = Spawn(class'EnergyBurst');
	a.DrawScale = 3.5;
	a.RemoteRole = ROLE_None;
	a = Spawn(class'ParticleBurst');
	a.RemoteRole = ROLE_None;	
}

defaultproperties
{
     Skin=FireTexture'Unreal.Effect51.MyTex3'
     DrawScale=1.000000
     LightRadius=8
}
