//=============================================================================
// Wire.
//=============================================================================
class Wire expands Decoration;

#exec MESH IMPORT MESH=WireM ANIVFILE=MODELS\wire_a.3D DATAFILE=MODELS\wire_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=WireM X=-10 Y=0 Z=-80 YAW=0
#exec MESH SEQUENCE MESH=WireM SEQ=All     STARTFRAME=0  NUMFRAMES=60
#exec MESH SEQUENCE MESH=WireM SEQ=Swing   STARTFRAME=0  NUMFRAMES=24
#exec MESH SEQUENCE MESH=WireM SEQ=Still   STARTFRAME=24 NUMFRAMES=2
#exec MESH SEQUENCE MESH=WireM SEQ=Still2  STARTFRAME=26 NUMFRAMES=1
#exec MESH SEQUENCE MESH=WireM SEQ=Wiggle  STARTFRAME=27  NUMFRAMES=33
#exec TEXTURE IMPORT NAME=JWire1 FILE=MODELS\wire.PCX GROUP="Skins"
#exec OBJ LOAD FILE=Textures\fireeffect25.utx PACKAGE=Unreal.Effect25
#exec MESHMAP SCALE MESHMAP=WireM X=0.13 Y=0.13 Z=0.26
#exec MESHMAP SETTEXTURE MESHMAP=WireM NUM=1 TEXTURE=JWire1
#exec MESHMAP SETTEXTURE MESHMAP=WireM NUM=0 TEXTURE=Unreal.Effect25.FireEffect25

var() enum EWireType
{
	E_WireHittable,
	E_WireWiggle
} WireType;

function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
					Vector momentum, name damageType)
{
	local rotator MyRotation,HitRotation;
	if (WireType == E_WireHittable && AnimSequence!='Swing') {
		MyRotation = rotator(momentum);
		MyRotation.pitch = 0;
		SetRotation(MyRotation);
		PlayAnim('Swing',0.35);
		GoToState('Idle');
	}
}



Auto State Animate
{

Begin:
	if (WireType == E_WireWiggle) PlayAnim('Wiggle',0.7);
	else PlayAnim('Still',FRand()*0.03+0.02);
	FinishAnim();
	GoTo('begin');
}

State Idle
{
Begin:
	FinishAnim();
	PlayAnim('Still',FRand()*0.03+0.02);
	GoTo('Begin');
}

defaultproperties
{
     WireType=E_WireWiggle
     bStatic=False
     DrawType=DT_Mesh
     Mesh=Unreal.WireM
     CollisionRadius=+00005.000000
     CollisionHeight=+00060.000000
     bCollideActors=True
     bProjTarget=True
}
