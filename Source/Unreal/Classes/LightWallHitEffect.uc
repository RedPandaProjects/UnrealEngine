//=============================================================================
// LightWallHitEffect.
//=============================================================================
class LightWallHitEffect expands WallHitEffect;

simulated function SpawnEffects()
{
	local Actor A;
	local float decision;

	decision = FRand();
	if (decision<0.2) 
		PlaySound(sound'ricochet',, 1,,1200, 0.5+FRand());		
	else if ( decision < 0.4 )
		PlaySound(sound'Impact1',, 3.0,,800);
	else if ( decision < 0.6 )
		PlaySound(sound'Impact2',, 3.0,,800);

	if (FRand()< 0.2) 
	{
		A = spawn(class'Chip');
		if ( A != None )
			A.RemoteRole = ROLE_None;
	}
	if (FRand()< 0.2)
	{
		A = spawn(class'SmallSpark',,,,Rotation + RotRand());
		if ( A != None )
			A.RemoteRole = ROLE_None;
	}
}

defaultproperties
{
}
