class WaterZone expands ZoneInfo;

//#exec AUDIO IMPORT FILE="Sounds\Generic\uWater1a.WAV" NAME="InWater" GROUP="Generic"
//	AmbientSound=InWater

defaultproperties
{
	bWaterZone=True
    ViewFog=(X=0.1289,Y=0.1953,Z=0.17578)
    ViewFlash=(X=-0.078,Y=-0.078,Z=-0.078)
    EntryActor=Unreal.WaterImpact
    ExitActor=Unreal.WaterImpact
    EntrySound=Unreal.DSplash
    ExitSound=Unreal.WtrExit1
}