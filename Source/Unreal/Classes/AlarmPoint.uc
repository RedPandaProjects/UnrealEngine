//=============================================================================
// AlarmPoint.
//=============================================================================
class AlarmPoint expands NavigationPoint;

#exec Texture Import File=Textures\SiteLite.pcx Name=S_Alarm Mips=Off Flags=2

var() name NextAlarm; //next point to go to
var() float pausetime; //how long to pause here
var() float ducktime; //how long to pause after playing anim before starting attack while paused
var	 vector lookdir; //direction to look while stopped
var() name AlarmAnim;
var() bool bStrafeTo;
var() bool bAttackWhilePaused;
var() bool bNoFail;
var() bool bStopIfNoEnemy;
var() bool bKillMe;	// tells event triggered creatures to kill alarm triggerer, even if not normally
					// hate
var() bool bDestroyAlarmTriggerer;
var() name ShootTarget;
var() sound AlarmSound;
var	actor	NextAlarmObject;

function PreBeginPlay()
{
	if ( !bAttackWhilePaused && (pausetime > 0.0) )
		lookdir = 2000 * vector(Rotation);

	if ( NextAlarm != '' )
		foreach AllActors(class 'Actor', NextAlarmObject, NextAlarm)
			break; 
	
	Super.PreBeginPlay();
}

defaultproperties
{
     bDirectional=True
	 Texture=S_Alarm
}
