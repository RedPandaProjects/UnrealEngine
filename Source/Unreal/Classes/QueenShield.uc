//=============================================================================
// QueenShield.
//=============================================================================
class QueenShield expands Effects;

#exec MESH IMPORT MESH=Spot ANIVFILE=MODELS\spot_a.3D DATAFILE=MODELS\spot_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=Spot X=0 Y=0 Z=0 PITCH=-64
#exec MESH SEQUENCE MESH=Spot SEQ=All       STARTFRAME=0   NUMFRAMES=2
#exec MESH SEQUENCE MESH=Spot SEQ=Explo     STARTFRAME=0   NUMFRAMES=2
#exec MESHMAP SCALE MESHMAP=Spot X=0.3 Y=0.3 Z=0.6
#exec OBJ LOAD FILE=textures\stshield.utx PACKAGE=Unreal.STEffect
#exec MESHMAP SETTEXTURE MESHMAP=Spot NUM=1 TEXTURE=Unreal.STEffect.STShield

#exec TEXTURE IMPORT NAME=ExplosionPal3 FILE=textures\expal2.pcx GROUP=Effects


function PostBeginPlay()
{
	//PlayAnim  ( 'Explo', 0.5 );
	Super.PostBeginPlay();
}

function touch( Actor Other )
{
	if ( !Other.IsA('Projectile') && (Other != Instigator) )
		Destroy();
}

defaultproperties
{
     DrawType=DT_Mesh
     Skin=Texture'Unreal.ExplosionPal3'
     Mesh=Mesh'Unreal.spot'
     bUnlit=True
     bMeshCurvy=False
     bCollideActors=True
     bBlockActors=False
     bBlockPlayers=False
     bProjTarget=True
     CollisionRadius=+00080.000000
     CollisionHeight=+00080.000000
     LightType=LT_TexturePaletteOnce
     LightEffect=LE_NonIncidence
     Physics=PHYS_None
     LifeSpan=5.000000
}
