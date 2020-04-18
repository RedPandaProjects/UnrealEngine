//=============================================================================
// LeglessKrall.
//=============================================================================
class LeglessKrall expands Krall;

var float startTime;

function PreBeginPlay()
{
	bCanSpeak = true;
	voicePitch = 0.25 + 0.5 * FRand();
	if ( CombatStyle == Default.CombatStyle)
		CombatStyle = CombatStyle + 0.4 * FRand() - 0.2;
	bCanDuck = (FRand() < 0.5);
	Super.PreBeginPlay();
	CombatStyle = 1.0;
	bCanDuck = false;
	bCanStrafe = false;
	startTime = Level.TimeSeconds;
}

function TakeDamage( int Damage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
{
	if ( Level.timeSeconds - startTime < 0.3 )
		return;
	Health = Min(-1, Health - Damage);
	PlayDeathHit(Damage, hitLocation, damageType);
	Enemy = instigatedBy;
	Died(instigatedBy, damageType, HitLocation);
}

function WhatToDoNext(name LikelyState, name LikelyLabel)
{
	Health = -1;
	Died(self, '', Location);
}

function PlayTakeHit(float tweentime, vector HitLoc, int damage)
{
	TweenAnim('Drag', tweentime);
}

function PlayWaiting()
{
	TweenAnim('Drag', 0.3);
}

function PlayPatrolStop()
{
	TweenAnim('Drag', 0.3);
}

function PlayWaitingAmbush()
{
	TweenAnim('Drag', 0.3);
}

function PlayChallenge()
{
	TweenAnim('Shoot3', 0.3);
}

function TweenToFighter(float tweentime)
{
	TweenAnim('Shoot3', tweentime);
}

function TweenToRunning(float tweentime)
{
	TweenAnim('Drag', tweentime);
}

function TweenToWalking(float tweentime)
{
	TweenAnim('Drag', tweentime);
}

function TweenToWaiting(float tweentime)
{
	TweenAnim('Drag', tweentime);
}

function TweenToPatrolStop(float tweentime)
{
	TweenAnim('Drag', tweentime);
}

function TweenToFalling()
{
	TweenAnim('Shoot3', 0.3);
}

function PlayInAir()
{
	TweenAnim('Shoot3', 0.3);
}

function PlayOutOfWater()
{
	TweenAnim('Shoot3', 0.3);
}

function PlayLanded(float impactVel)
{
	TweenAnim('LeglessDeath', 0.1);
}

function PlayMovingAttack()
{
	LoopAnim('Shoot3');
}

function PlayRunning()
{
	LoopAnim('Drag');
}

function PlayWalking()
{
	LoopAnim('Drag');
}

function TweenToSwimming(float tweentime)
{
	TweenAnim('Drag', tweentime);
}

function PlaySwimming()
{
	LoopAnim('Drag');
}

function PlayThreatening()
{
	TweenAnim('Shoot3', 0.3);
}

function PlayTurning()
{
	PlayAnim('Drag');
}

function PlayDying(name DamageType, vector HitLoc)
{
	PlaySound(Die, SLOT_Talk, 4 * TransientSoundVolume);
	TweenAnim('LeglessDeath', 0.4);
}

function PlayVictoryDance()
{
	TweenAnim('Shoot3', 0.3);
}

function bool CanFireAtEnemy()
{
	local vector HitLocation, HitNormal,X,Y,Z, projStart, EnemyDir, EnemyUp;
	local actor HitActor;
	local float EnemyDist;
		
	EnemyDir = Enemy.Location - Location;
	EnemyDist = VSize(EnemyDir);
	EnemyUp = Enemy.CollisionHeight * vect(0,0,0.9);
	if ( EnemyDist > 300 )
	{
		EnemyDir = 300 * EnemyDir/EnemyDist;
		EnemyUp = 300 * EnemyUp/EnemyDist;
	}
	
	GetAxes(Rotation,X,Y,Z);
	projStart = Location + 0.9 * CollisionRadius * X - 0.9 * CollisionRadius * Y;
	HitActor = Trace(HitLocation, HitNormal, projStart + EnemyDir + EnemyUp, projStart, true);

	if ( (HitActor == None) || (HitActor == Enemy) 
		|| ((Pawn(HitActor) != None) && (AttitudeTo(Pawn(HitActor)) <= ATTITUDE_Ignore)) )
		return true;

	HitActor = Trace(HitLocation, HitNormal, projStart + EnemyDir, projStart , true);

	return ( (HitActor == None) || (HitActor == Enemy) 
			|| ((Pawn(HitActor) != None) && (AttitudeTo(Pawn(HitActor)) <= ATTITUDE_Ignore)) );
}
	
function SpawnShot()
{
	FireProjectile( vect(0.9, -0.9, 0), 500);
}

function PlayMeleeAttack()
{
	PlayAnim('Shoot3');
}

function PlayRangedAttack()
{
	PlayAnim('Shoot3');
}

function Initfor(Krall Other)
{
	local rotator carcRotation;

	bMeshCurvy = Other.bMeshCurvy;	
	bMeshEnviroMap = Other.bMeshEnviroMap;	
	Mesh = Other.Mesh;
	Skin = Other.Skin;
	Texture = Other.Texture;
	Fatness = Other.Fatness;
	DrawScale = Other.DrawScale;
	Tag = Other.Tag;
	Event = '';

	PlayAnim('LegLoss', 0.7);
	SetPhysics(PHYS_Falling);
	Velocity = other.Velocity;
	Enemy=Other.Enemy;
	OldEnemy=Other.OldEnemy;
	NextState='Attacking';
	NextLabel='Begin';
	GotoState('TakeHit');
}

state TacticalMove
{
ignores SeePlayer, HearNoise;

	function TweenToRunning(float tweentime)
	{
		TweenAnim('Drag', tweentime);
	}
}

defaultproperties
{
	 PrePivot=(X=0.000000,Y=0.000000,Z=25.00000)
     CollisionHeight=+00021.000000
	 MeleeRange=-00030.000000
     UnderWaterTime=+00001.000000
     Aggressiveness=+00010.000000
     JumpZ=-00001.000000
     bMovingRangedAttack=False
     bCanStrafe=False
     GroundSpeed=+00080.000000
	 AnimSequence=LegLoss
}