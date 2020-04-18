//=============================================================================
// DispersionAmmo.
//=============================================================================
class DispersionAmmo expands Projectile;


#exec MESH IMPORT MESH=plasmaM ANIVFILE=MODELS\cros_t_a.3D DATAFILE=MODELS\cros_t_d.3D X=0 Y=0 Z=0 
#exec MESH ORIGIN MESH=plasmaM X=0 Y=-500 Z=0 YAW=-64
#exec MESH SEQUENCE MESH=plasmaM SEQ=All STARTFRAME=0  NUMFRAMES=1
#exec MESH SEQUENCE MESH=plasmaM SEQ=Still  STARTFRAME=0 NUMFRAMES=1
#exec MESHMAP SCALE MESHMAP=plasmaM X=0.09 Y=0.15 Z=0.08
#exec  OBJ LOAD FILE=Textures\fireeffect1.utx PACKAGE=Unreal.Effect1
#exec MESHMAP SETTEXTURE MESHMAP=plasmaM NUM=0 TEXTURE=Unreal.Effect1.FireEffect1u
#exec MESHMAP SETTEXTURE MESHMAP=plasmaM NUM=1 TEXTURE=Unreal.Effect1.FireEffect1t

#exec AUDIO IMPORT FILE="Sounds\Dispersion\DFly1.WAV" NAME="DispFly" GROUP="Dispersion"
  
var bool bExploded,bAltFire;
var float Count,SmokeRate;
var() float SparkScale;
var() class<SmallSpark> ParticleType;
var() float SparkModifier;
var() class<SpriteBlueExplo> ExpType;

simulated function Tick(float DeltaTime)
{
	local SmallSpark b;

	Count += DeltaTime;
	if ( (Count>(SmokeRate+FRand()*SmokeRate)) && (Level.NetMode!=NM_DedicatedServer) ) 
	{
		b = Spawn(ParticleType);
		b.DrawScale = b.DrawScale * SparkScale * SparkModifier;
		b.RemoteRole = ROLE_None;		
		Count=0.0;
	}
}

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	Velocity = Speed * vector(Rotation);
	Count = -0.1;
	if (Level.bHighDetailMode) SmokeRate = 0.01;		
	else 
	{
		SmokeRate = 10;;	
		DrawScale = 0.7;
	}
}

function InitSplash(float DamageScale)
{
	Damage *= DamageScale;
	MomentumTransfer *= DamageScale;
	SparkScale = FClamp(DamageScale*3.0 - 1.2,0.5,4.0);
	DrawScale = fMin(DamageScale,2.0);
	if (SparkScale>1.5 && SmokeRate<1.0) SmokeRate=(SparkScale-1.5)*0.007+0.01;	
}

function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
					Vector momentum, name damageType)
{
	bExploded = True;
}

function Explode(vector HitLocation, vector HitNormal)
{
	local SpriteBlueExplo d;

	if (bAltFire) HurtRadius(Damage,150.0, 'exploded', MomentumTransfer, HitLocation );	
	if (!bExploded) {
		if (Damage>30) {
			d = Spawn(ExpType,, '', HitLocation, Rotator(HitNormal));
			if (Level.bHighDetailMode) d.DrawScale = Min(Damage/12.0 + 0.8,2.5);
			else d.DrawScale = Min(Damage/20.0 + 0.8,1.5);							
		}
		else {
			d = Spawn(ExpType,, '', HitLocation, Rotator(HitNormal));
			if (Level.bHighDetailMode) d.DrawScale = Min(Damage/12.0 + 0.8,2.5);
			else d.DrawScale = Min(Damage/20.0 + 0.8,1.5);				
		}
	}
	Destroy();
}

function ProcessTouch (Actor Other, vector HitLocation)
{
	If (Other!=Instigator  && DispersionAmmo(Other)==None)
	{
		Other.TakeDamage( Damage, instigator, HitLocation, MomentumTransfer*Vector(Rotation), 'exploded');	
		Explode(HitLocation, vect(0,0,1));
	}
}

defaultproperties
{
     SparkScale=1.000000
     ParticleType=Class'Unreal.Spark3'
     SparkModifier=1.000000
     ExpType=Class'Unreal.SpriteBlueExplo'
     speed=1300.000000
     Damage=15.000000
     MomentumTransfer=6000
     ExploWallOut=10.000000
     RemoteRole=ROLE_SimulatedProxy
     LifeSpan=6.000000
     Style=STY_Translucent
     Texture=FireTexture'Unreal.Effect1.FireEffect1u'
     Mesh=Mesh'Unreal.plasmaM'
     DrawScale=0.800000
     AmbientGlow=187
     bUnlit=True
     SoundRadius=10
     SoundVolume=218
     AmbientSound=Sound'Unreal.Dispersion.DispFly'
     LightType=LT_Steady
     LightEffect=LE_NonIncidence
     LightBrightness=255
     LightHue=170
     LightSaturation=69
     LightRadius=5
     bFixedRotationDir=True
}
