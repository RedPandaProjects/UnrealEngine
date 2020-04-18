class NitrogenZone expands ZoneInfo;

//#exec AUDIO IMPORT FILE="Sounds\Generic\uLNitro1.WAV" NAME="InNitro" GROUP="Generic"
//	AmbientSound=InNitro

// When an actor enters this zone.
event ActorEntered( actor Other )
{
	Super.ActorEntered(Other);
	if ( Other.IsA('Pawn') && Pawn(Other).bIsPlayer )
	{
		Pawn(Other).UnderWaterTime = -1.0;
		Pawn(Other).WaterSpeed *= 2;
	}
}

// When an actor leaves this zone.
event ActorLeaving( actor Other )
{
	Super.ActorLeaving(Other);
	if ( Other.IsA('Pawn') && Pawn(Other).bIsPlayer )
	{
		Pawn(Other).UnderWaterTime = Pawn(Other).Default.UnderWaterTime;
		Pawn(Other).WaterSpeed *= 0.5;
	}
}

defaultproperties
{
	DamagePerSec=20
	DamageType=Frozen
	bPainZone=True
	bWaterZone=True
    ViewFog=(X=0.01171875,Y=0.0390625,Z=0.046875)
}
