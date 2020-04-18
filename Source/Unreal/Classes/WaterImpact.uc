//=============================================================================
// WaterImpact.
//=============================================================================
class WaterImpact expands Effects;

var bool bSpawnOnce;

simulated function Timer()
{
	local WaterRing r;

	if ( Level.NetMode != NM_DedicatedServer )
	{
		r = Spawn(class'WaterRing',,,,rot(16384,0,0));
		r.DrawScale = 0.15;
		r.RemoteRole = ROLE_None;
	}
	else 
		Destroy();
	if (bSpawnOnce) Destroy();
	bSpawnOnce=True;
}


simulated function PostBeginPlay()
{
	SetTimer(0.3,True);
}

defaultproperties
{
     DrawType=DT_Mesh
     AmbientGlow=79
     bMeshCurvy=False
     Physics=PHYS_None
     AnimSequence=Burst
     RemoteRole=ROLE_SimulatedProxy
	 bNetOptional=True
}
