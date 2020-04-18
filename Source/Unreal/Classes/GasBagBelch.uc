//=============================================================================
// GasbagBelch.
//=============================================================================
class GasbagBelch expands Projectile;

#exec TEXTURE IMPORT NAME=gbProj0 FILE=MODELS\gb_a00.pcx GROUP=Effects
#exec TEXTURE IMPORT NAME=gbProj1 FILE=MODELS\gb_a01.pcx GROUP=Effects
#exec TEXTURE IMPORT NAME=gbProj2 FILE=MODELS\gb_a02.pcx GROUP=Effects
#exec TEXTURE IMPORT NAME=gbProj3 FILE=MODELS\gb_a03.pcx GROUP=Effects
#exec TEXTURE IMPORT NAME=gbProj4 FILE=MODELS\gb_a04.pcx GROUP=Effects
#exec TEXTURE IMPORT NAME=gbProj5 FILE=MODELS\gb_a05.pcx GROUP=Effects

#exec AUDIO IMPORT FILE="Sounds\flak\expl2.wav" NAME="expl2" GROUP="flak"

var() texture SpriteAnim[6];
var int i;


function Timer()
{
	Texture = SpriteAnim[i];
	i++;
	if (i>=6) i=0;
}


function PostBeginPlay()
{
	if ( ScriptedPawn(Instigator) != None )
		Speed = ScriptedPawn(Instigator).ProjectileSpeed;
	PlaySound(SpawnSound);
	Velocity = Vector(Rotation) * speed;
	MakeNoise ( 1.0 );
	Texture = SpriteAnim[0];
	i=1;
	SetTimer(0.15,True);
	Super.PostBeginPlay();
}

auto state Flying
{


function ProcessTouch (Actor Other, Vector HitLocation)
{
	if (Other != instigator)
	{
		Other.TakeDamage(Damage, instigator,HitLocation,
				15000.0 * Normal(velocity), 'burned');
		Explode(HitLocation, Vect(0,0,0));
	}
}

function Explode(vector HitLocation, vector HitNormal)
{
	if (FRand() < 0.5)
		MakeNoise(1.0); //FIXME - set appropriate loudness
	Spawn(class'SpriteBallExplosion',,,HitLocation+HitNormal*9);
	Destroy();
}

Begin:
	Sleep(3);
	Explode(Location, Vect(0,0,0));
}

defaultproperties
{
     SpriteAnim(0)=Texture'Unreal.gbProj0'
     SpriteAnim(1)=Texture'Unreal.gbProj1'
     SpriteAnim(2)=Texture'Unreal.gbProj2'
     SpriteAnim(3)=Texture'Unreal.gbProj3'
     SpriteAnim(4)=Texture'Unreal.gbProj4'
     SpriteAnim(5)=Texture'Unreal.gbProj5'
     speed=600.000000
     Damage=40.000000
     ImpactSound=Sound'Unreal.expl2'
     DrawType=DT_Sprite
     Style=STY_Translucent
     Texture=Texture'Unreal.gbProj0'
     DrawScale=1.800000
     Fatness=0
     bUnlit=True
     bMeshCurvy=False
     LightType=LT_Steady
     LightEffect=LE_NonIncidence
     LightBrightness=255
     LightHue=5
     LightSaturation=16
     LightRadius=9
     RemoteRole=ROLE_SimulatedProxy
}
