//=============================================================================
// Shells.
//=============================================================================
class Shells expands Ammo;

#exec AUDIO IMPORT FILE="Sounds\Pickups\AMMOPUP1.WAV" NAME="AmmoSnd"       GROUP="Pickups"

#exec MESH IMPORT MESH=ShellsM ANIVFILE=MODELS\aniv34.3D DATAFILE=MODELS\data34.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=ShellsM X=0 Y=0 Z=-30 YAW=0
#exec MESH SEQUENCE MESH=ShellsM SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=Jshells1 FILE=MODELS\shells.PCX GROUP="Skins" FLAGS=2
#exec MESHMAP SCALE MESHMAP=ShellsM X=0.015 Y=0.015 Z=0.03
#exec MESHMAP SETTEXTURE MESHMAP=ShellsM NUM=4 TEXTURE=Jshells1
defaultproperties
{
     AmmoAmount=10
     MaxAmmo=100
     PickupMessage="You picked up 15 shells"
     PickupViewMesh=ShellsM
     PickupSound=AmmoSnd
     Mesh=ShellsM
     bMeshCurvy=False
     CollisionRadius=+00012.000000
     CollisionHeight=+00005.000000
     bCollideActors=True
}
