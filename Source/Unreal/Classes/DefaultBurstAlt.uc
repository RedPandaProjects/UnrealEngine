//=============================================================================
// DefaultBurstAlt.
//=============================================================================
class DefaultBurstAlt expands DefaultBurst;

simulated function Timer()
{
	local actor a;
	a = Spawn(class'RingExplosion');
	a.RemoteRole = ROLE_None;
}

simulated Function PostBeginPlay()
{
	if ( Level.NetMode != NM_DedicatedServer )
	{
		SetTimer(0.05,False);
		PlaySound (EffectSound1,,3.0);
	}
	Super.PostBeginPlay();
}

defaultproperties
{
}
