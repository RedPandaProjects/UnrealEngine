//=============================================================================
// WallHitEffect.
//=============================================================================
class WallHitEffect expands SpriteSmokePuff;

#exec AUDIO IMPORT FILE="Sounds\minigun\imp01.WAV" NAME="Impact1" GROUP="Minigun"
#exec AUDIO IMPORT FILE="Sounds\minigun\imp02.WAV" NAME="Impact2" GROUP="Minigun"
#exec AUDIO IMPORT FILE="Sounds\stinger\Ricochet.WAV" NAME="Ricochet" GROUP="Stinger"

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	if ( Instigator != None )
		MakeNoise(0.3);

	SpawnEffects();
}

simulated function SpawnEffects()
{
	local Actor A;
	local float decision;

	decision = FRand();
	if (decision<0.1) 
		PlaySound(sound'ricochet',, 1,,1200, 0.5+FRand());		
	if ( decision < 0.35 )
		PlaySound(sound'Impact1',, 2.0,,1200);
	else if ( decision < 0.6 )
		PlaySound(sound'Impact2',, 2.0,,1200);

	if (FRand()< 0.3) 
	{
		A = spawn(class'Chip');
		if ( A != None )
			A.RemoteRole = ROLE_None;
	}
	if (FRand()< 0.3) 
	{
		A = spawn(class'Chip');
		if ( A != None )
			A.RemoteRole = ROLE_None;
	}
	if (FRand()< 0.3)
	{
		A = spawn(class'Chip');
		if ( A != None )
			A.RemoteRole = ROLE_None;
	}
	A = spawn(class'SmallSpark2',,,,Rotation + RotRand());
	if ( A != None )
		A.RemoteRole = ROLE_None;
}

defaultproperties
{
     bNetOptional=True
}
