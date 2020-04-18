//=============================================================================
// Cannon.
//=============================================================================
class Cannon expands Decoration;

#exec MESH IMPORT MESH=CannonM ANIVFILE=MODELS\cannon_a.3D DATAFILE=MODELS\cannon_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=CannonM X=0 Y=270 Z=0 YAW=-64 ROLL=-64
#exec MESH SEQUENCE MESH=CannonM SEQ=All  STARTFRAME=0  NUMFRAMES=20
#exec MESH SEQUENCE MESH=CannonM SEQ=Activate STARTFRAME=0   NUMFRAMES=10
#exec MESH SEQUENCE MESH=CannonM SEQ=Angle0  STARTFRAME=10  NUMFRAMES=1
#exec MESH SEQUENCE MESH=CannonM SEQ=Angle1  STARTFRAME=11  NUMFRAMES=1
#exec MESH SEQUENCE MESH=CannonM SEQ=Angle2  STARTFRAME=12  NUMFRAMES=1
#exec MESH SEQUENCE MESH=CannonM SEQ=Angle3  STARTFRAME=13  NUMFRAMES=1
#exec MESH SEQUENCE MESH=CannonM SEQ=Angle4  STARTFRAME=14  NUMFRAMES=1
#exec MESH SEQUENCE MESH=CannonM SEQ=FAngle0  STARTFRAME=15  NUMFRAMES=1
#exec MESH SEQUENCE MESH=CannonM SEQ=FAngle1  STARTFRAME=16  NUMFRAMES=1
#exec MESH SEQUENCE MESH=CannonM SEQ=FAngle2  STARTFRAME=17  NUMFRAMES=1
#exec MESH SEQUENCE MESH=CannonM SEQ=FAngle3  STARTFRAME=18  NUMFRAMES=1
#exec MESH SEQUENCE MESH=CannonM SEQ=FAngle4 STARTFRAME=19  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JCannon1 FILE=MODELS\cannon.PCX GROUP=Skins
#exec OBJ LOAD FILE=Textures\fireeffect13.utx PACKAGE=Unreal.Effect13
#exec MESHMAP SCALE MESHMAP=CannonM X=0.2 Y=0.2 Z=0.4
#exec MESHMAP SETTEXTURE MESHMAP=CannonM NUM=0 TEXTURE=Unreal.Effect13.FireEffect13
#exec MESHMAP SETTEXTURE MESHMAP=CannonM NUM=1 TEXTURE=JCannon1

#exec AUDIO IMPORT FILE="Sounds\Cannon\turshot1.wav" NAME="CannonShot" GROUP="Cannon" 
#exec AUDIO IMPORT FILE="Sounds\Cannon\turdrop1.wav" NAME="CannonActivate" GROUP="Cannon" 
#exec AUDIO IMPORT FILE="Sounds\Cannon\turExpl.wav" NAME="CannonExplode" GROUP="Cannon" 

var() float DeactivateDistance;	// How far away Instigator must be to deactivate Cannon
var() float SampleTime; 			// How often we sample Instigator's location
var() int   TrackingRate;			// How fast Cannon tracks Instigator
var() float Drop;					// How far down to drop spawning of projectile
var() float Health;
var() sound FireSound;
var() sound ActivateSound;
var() sound ExplodeSound;
var actor cTarget;
var bool bShoot; 
var int ShotsFired;
var actor a;

function Shoot() {}   // To resolve error 'virtual function 'shoot' not found'

function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
					Vector momentum, name damageType)
{
	Instigator = InstigatedBy;
	if (Health<0) Return;
	if ( Instigator != None )
		MakeNoise(1.0);
	Health -= NDamage;
	if (Health <0) {
		PlaySound(ExplodeSound, SLOT_None,5.0);	
		skinnedFrag(class'Fragment1',texture'JCannon1', Momentum,1.0,17);	
		Destroy();
	}
}


function Trigger( actor Other, pawn EventInstigator )
{
	cTarget    = Other;
	Instigator = EventInstigator;
	GotoState( 'ActivateCannon');
}

state ActivateCannon
{
	function Trigger( actor Other, pawn EventInstigator ) {}

	function Timer()
	{

		if (VSize(cTarget.Location - Location) > DeactivateDistance) GoToState('Deactivate');
		if (Pawn(cTarget)!=None && Pawn(cTarget).Health>0 || cTarget==None) GoToState('Deactivate');
		DesiredRotation = rotator(cTarget.Location - Location + Vect(0,0,1)*Drop);
		DesiredRotation.Yaw = DesiredRotation.Yaw & 65535;
		if (Abs(DesiredRotation.Yaw - (Rotation.Yaw & 65535)) < 1000
			&& DesiredRotation.Pitch < 1000 && bShoot)	Shoot();
		else if (Abs(DesiredRotation.Yaw - (Rotation.Yaw & 65535)) > 64535 
			&& DesiredRotation.Pitch < 1000 && bShoot)	Shoot();
		else {
			if (DesiredRotation.Pitch < -6000 ) TweenAnim('Angle4', 0.25);
			else if (DesiredRotation.Pitch < -4000 ) TweenAnim('Angle3', 0.25);
			else if (DesiredRotation.Pitch < -2000 ) TweenAnim('Angle2', 0.25);
			else if (DesiredRotation.Pitch < -500 ) TweenAnim('Angle1', 0.25);
			else TweenAnim('Angle0', 0.25);		
			bShoot=True;			
		}
		bRotateToDesired = True;
		SetTimer(SampleTime,True);
	}
	
	function Shoot()
	{
		if (DesiredRotation.Pitch < -10000) Return;
		PlaySound(FireSound, SLOT_None,5.0);
		if (DesiredRotation.Pitch < -6000 ) PlayAnim('FAngle4',5.0);
		else if (DesiredRotation.Pitch < -4000 ) PlayAnim('FAngle3',5.0);
		else if (DesiredRotation.Pitch < -2000 ) PlayAnim('FAngle2',5.0);
		else if (DesiredRotation.Pitch < -500 ) PlayAnim('FAngle1',5.0);
		else PlayAnim('FAngle0',5.0);
		Spawn (class'CannonBolt',,,Location+Vector(DesiredRotation)*100 - Vect(0,0,1)*Drop,DesiredRotation);
		bShoot=False;
		SetTimer(0.05,True);
	}

Begin:
	PlayAnim('Activate',0.5);
	PlaySound(ActivateSound, SLOT_None, 2.0);
	FinishAnim();
	SetTimer(SampleTime,True);
	RotationRate.Yaw = TrackingRate;
	SetPhysics(PHYS_Rotating);
	bShoot=True;
}

state DeActivate
{
Begin:
	TweenAnim('Activate',3.0);
	if (Event!='')
		foreach AllActors( class 'Actor', A, Event )
			A.Trigger( Self, Pawn(cTarget) );
	
}

defaultproperties
{
     DeactivateDistance=+02000.000000
     SampleTime=+00000.300000
     TrackingRate=10000
     Drop=+00060.000000
     Health=+00100.000000
     FireSound=Unreal.CannonShot
     ActivateSound=Unreal.CannonActivate
     ExplodeSound=Unreal.CannonExplode
     bStatic=False
     DrawType=DT_Mesh
     Mesh=Unreal.CannonM
     bMeshCurvy=False
     CollisionRadius=+00044.000000
     CollisionHeight=+00044.000000
     bCollideActors=True
     bCollideWorld=True
     bProjTarget=True
     RotationRate=(Yaw=50000)
}
