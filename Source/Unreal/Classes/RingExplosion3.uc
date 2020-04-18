//=============================================================================
// RingExplosion3.
//=============================================================================
class RingExplosion3 expands RingExplosion;


#exec OBJ LOAD FILE=Textures\fireeffect55.utx PACKAGE=Unreal.Effect55

simulated function PostBeginPlay()
{
	if ( Level.NetMode != NM_DedicatedServer )
	{
		SetTimer(0.3, false);
		SpawnEffects();
	}	
}

simulated function Timer()
{	
	PlayAnim  ( 'Explosion', 0.125 );
}

defaultproperties
{
     Skin=Unreal.Effect55.fireeffect55
     DrawScale=+00001.250000
}
