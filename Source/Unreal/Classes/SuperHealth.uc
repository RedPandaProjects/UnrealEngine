//=============================================================================
// SuperHealth.
//=============================================================================
class SuperHealth expands Health;

#exec MESH IMPORT MESH=SuperHealthMesh ANIVFILE=MODELS\sheal_a.3D DATAFILE=MODELS\sheal_d.3D X=0 Y=0 Z=0
#exec  MESH ORIGIN MESH=SuperHealthMesh X=0 Y=0 Z=0 ROLL=128
#exec  MESH SEQUENCE MESH=SuperHealthMesh SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec  TEXTURE IMPORT NAME=Jshealth1 FILE=MODELS\shealth.PCX GROUP="Skins" 
#exec OBJ LOAD FILE=Textures\WaterEffect1.utx  PACKAGE=Unreal.WEffect1
#exec  MESHMAP SCALE MESHMAP=SuperHealthMesh X=0.04 Y=0.04 Z=0.08
#exec  MESHMAP SETTEXTURE MESHMAP=SuperHealthMesh NUM=1 TEXTURE=Jshealth1
#exec  MESHMAP SETTEXTURE MESHMAP=SuperHealthMesh NUM=0 TEXTURE=Unreal.WEffect1.WaterEffect1

defaultproperties
{
     HealingAmount=100
	 bSuperHeal=true
     MaxDesireability=+00001.000000
     PickupMessage="You picked up the Super Health Pack"
     RespawnTime=+00100.000000
     PickupViewMesh=Unreal.SuperHealthMesh
     Texture=None
     Mesh=Unreal.SuperHealthMesh
     CollisionRadius=+00016.000000
     CollisionHeight=+00019.500000
}
