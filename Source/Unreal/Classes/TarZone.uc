class TarZone expands ZoneInfo;

#exec AUDIO IMPORT FILE="Sounds\Generic\GoopE1.WAV" NAME="LavaEx" GROUP="Generic"
#exec AUDIO IMPORT FILE="Sounds\Generic\GoopJ1.WAV" NAME="LavaEn" GROUP="Generic"
//#exec AUDIO IMPORT FILE="Sounds\Generic\uGoop1.WAV" NAME="InGoop" GROUP="Generic"
//	 AmbientSound=InGoop

// When an actor enters this zone.
event ActorEntered( actor Other )
{
	Super.ActorEntered(Other);
	if ( Other.IsA('Pawn') && Pawn(Other).bIsPlayer )
		Pawn(Other).WaterSpeed *= 0.1;
}

// When an actor leaves this zone.
event ActorLeaving( actor Other )
{
	Super.ActorLeaving(Other);
	if ( Other.IsA('Pawn') && Pawn(Other).bIsPlayer )
		Pawn(Other).WaterSpeed *= 10;
}

defaultproperties
{
     ZoneTerminalVelocity=+0250.000000
     ZoneFluidFriction=+00004.000000
     bWaterZone=True
     ViewFog=(X=0.3125,Y=0.3125,Z=0.234375)
     ViewFlash=(X=-0.39,Y=-0.39,Z=-0.39)
     EntrySound=Unreal.LavaEn
	 ExitSound=Unreal.LavaEx
}