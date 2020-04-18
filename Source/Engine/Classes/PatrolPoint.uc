//=============================================================================
// PatrolPoint.
//=============================================================================
class PatrolPoint expands NavigationPoint;

#exec Texture Import File=Textures\Pathnode.pcx Name=S_Patrol Mips=Off Flags=2

var() name Nextpatrol; //next point to go to
var() float pausetime; //how long to pause here
var	 vector lookdir; //direction to look while stopped
var() name PatrolAnim;
var() sound PatrolSound;
var() byte numAnims;
var int	AnimCount;
var PatrolPoint NextPatrolPoint;


function PreBeginPlay()
{
	if (pausetime > 0.0)
		lookdir = 200 * vector(Rotation);

	//find the patrol point with the tag specified by Nextpatrol
	foreach AllActors(class 'PatrolPoint', NextPatrolPoint, Nextpatrol)
		break; 
	
	Super.PreBeginPlay();
}
defaultproperties
{
     bDirectional=True
     SoundVolume=128
	 Texture=S_Patrol
}
