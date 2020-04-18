//=============================================================================
// HeavyWallHitEffect.
//=============================================================================
class HeavyWallHitEffect expands WallHitEffect;

simulated function SpawnEffects()
{
	local Actor A;
	local float decision;

	decision = FRand();
	if (decision<0.15) 
		PlaySound(sound'ricochet',, 0.5,,1200, 0.3 + 0.7 * FRand());	
	else if ( decision < 0.5 )
		PlaySound(sound'Impact1',, 4.0,,800);
	else if ( decision < 0.9 )
		PlaySound(sound'Impact2',, 4.0,,800);

	if (FRand()< 0.5) 
	{
		A = spawn(class'Chip');
		if ( A != None )
			A.RemoteRole = ROLE_None;
	}
	if (FRand()< 0.5) 
	{
		A = spawn(class'Chip');
		if ( A != None )
			A.RemoteRole = ROLE_None;
	}
	if (FRand()< 0.5)
	{
		A = spawn(class'Chip');
		if ( A != None )
			A.RemoteRole = ROLE_None;
	}
	A = spawn(class'SmallSpark',,,,Rotation + RotRand());
	if ( A != None )
		A.RemoteRole = ROLE_None;
}

defaultproperties
{
}
