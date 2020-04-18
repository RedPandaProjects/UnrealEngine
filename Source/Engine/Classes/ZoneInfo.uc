//=============================================================================
// ZoneInfo, the built-in Unreal class for defining properties
// of zones.  If you place one ZoneInfo actor in a
// zone you have partioned, the ZoneInfo defines the 
// properties of the zone.
// This is a built-in Unreal class and it shouldn't be modified.
//=============================================================================
class ZoneInfo expands Info
	localized
	intrinsic;

#exec Texture Import File=Textures\ZoneInfo.pcx Name=S_ZoneInfo Mips=Off Flags=2

//-----------------------------------------------------------------------------
// Zone properties.

var() name   ZoneTag;
var() vector ZoneGravity;
var() vector ZoneVelocity;
var() float  ZoneGroundFriction;
var() float  ZoneFluidFriction;
var() float	 ZoneTerminalVelocity;
var() name   ZonePlayerEvent;
var   int    ZonePlayerCount;
var   int	 NumCarcasses;
var() int	 DamagePerSec;
var() name	 DamageType;
var() localized string[64] DamageString;
var() int	 MaxCarcasses;
var() sound  EntrySound;	//only if waterzone
var() sound  ExitSound;		// only if waterzone
var() class<actor> EntryActor;	// e.g. a splash (only if water zone)
var() class<actor> ExitActor;	// e.g. a splash (only if water zone)
var skyzoneinfo SkyZone; // Optional sky zone containing this zone's sky.

//-----------------------------------------------------------------------------
// Zone flags.

var() const bool   bWaterZone;   // Zone is water-filled.
var() const bool   bFogZone;     // Zone is fog-filled.
var() const bool   bKillZone;    // Zone instantly kills those who enter.
var() const bool   bNeutralZone; // Players can't take damage in this zone.
var()		bool   bGravityZone; // Use ZoneGravity.
var()		bool   bPainZone;	 // Zone causes pain.
var()		bool   bDestructive; // Destroys carcasses.

//-----------------------------------------------------------------------------
// Zone light.

var(ZoneLight) byte AmbientBrightness, AmbientHue, AmbientSaturation;
var(ZoneLight) color FogColor;
var(ZoneLight) float FogDistance;

var(ZoneLight) const texture EnvironmentMap;
var(ZoneLight) float TexUPanSpeed, TexVPanSpeed;
var(ZoneLight) vector ViewFlash, ViewFog;

//-----------------------------------------------------------------------------
// Reverb.

// Settings.
var(Reverb) bool bReverbZone;
var(Reverb) bool bRaytraceReverb;
var(Reverb) float SpeedOfSound;
var(Reverb) byte MasterGain;
var(Reverb) int  CutoffHz;
var(Reverb) byte Delay[6];
var(Reverb) byte Gain[6];

//=============================================================================
// Network replication.

replication
{
	reliable if( Role==ROLE_Authority )
		ZoneGravity, ZoneVelocity, ZoneTerminalVelocity,
		ZoneGroundFriction, ZoneFluidFriction,
		AmbientBrightness, AmbientHue, AmbientSaturation,
		TexUPanSpeed, TexVPanSpeed,
		ViewFlash, ViewFog,
		bReverbZone,
		FogColor;
}

//=============================================================================
// Iterator functions.

// Iterate through all actors in this zone.
intrinsic(308) final iterator function ZoneActors( class<actor> BaseClass, out actor Actor );

//=============================================================================
// Engine notification functions.

simulated function PreBeginPlay()
{
	local skyzoneinfo TempSkyZone;
	Super.PreBeginPlay();

	// Enter and exit sounds.
	if( bWaterZone && Level.Game!=None )
	{
		if( EntryActor == None )
			EntryActor = Level.Game.DefaultWaterEntryActor;
		if( ExitActor == None )
			ExitActor = Level.Game.DefaultWaterExitActor;
		if( EntrySound == None )
			EntrySound = Level.Game.DefaultWaterEntrySound;
		if( ExitSound == None )
			ExitSound = Level.Game.DefaultWaterExitSound;
	}

	// SkyZone.
	foreach AllActors( class 'SkyZoneInfo', TempSkyZone, '' )
		SkyZone = TempSkyZone;
	foreach AllActors( class 'SkyZoneInfo', TempSkyZone, '' )
		if( TempSkyZone.bHighDetail == Level.bHighDetailMode )
			SkyZone = TempSkyZone;
}

function Trigger( actor Other, pawn EventInstigator )
{
	if (DamagePerSec != 0)
		bPainZone = true;
}

// When an actor enters this zone.
event ActorEntered( actor Other )
{
	local actor A;
	if( Pawn(Other)!=None && Pawn(Other).bIsPlayer )
		if( ++ZonePlayerCount==1 && ZonePlayerEvent!='' )
			foreach AllActors( class 'Actor', A, ZonePlayerEvent )
				A.Trigger( Self, Pawn(Other) );
}

// When an actor leaves this zone.
event ActorLeaving( actor Other )
{
	local actor A;
	if( Pawn(Other)!=None && Pawn(Other).bIsPlayer )
		if( --ZonePlayerCount==0 && ZonePlayerEvent!='' )
			foreach AllActors( class 'Actor', A, ZonePlayerEvent )
				A.UnTrigger( Self, Pawn(Other) );
}

defaultproperties
{
     bStatic=True
     bNoDelete=True
	 bAlwaysRelevant=true
     Texture=S_ZoneInfo
     ZoneGravity=(X=0.000000,Y=0.000000,Z=-950.000000)
     ZoneGroundFriction=+00004.000000
     ZoneFluidFriction=+00001.200000
     ZoneTerminalVelocity=+02500.000000
     MaxCarcasses=3
     AmbientSaturation=255
	 FogColor=(R=0,G=0,B=0,A=0)
     TexUPanSpeed=+00001.000000
     TexVPanSpeed=+00001.000000
     SpeedOfSound=+08000.000000
     MasterGain=100
     CutoffHz=6000
	 FogDistance=0
     Delay(0)=20
     Delay(1)=34
     Gain(0)=150
     Gain(1)=70
	 ReverbVolume=0.7
	 HfDampHz=8000.0
}
