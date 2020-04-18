//=============================================================================
// RocketCan.
//=============================================================================
class RocketCan expands Ammo;

#exec AUDIO IMPORT FILE="Sounds\Pickups\AMMOPUP1.WAV" NAME="AmmoSnd"       GROUP="Pickups"

#exec TEXTURE IMPORT NAME=I_RocketAmmo FILE=TEXTURES\HUD\i_rcan.PCX GROUP="Icons" MIPS=OFF

#exec MESH IMPORT MESH=RocketCanMesh ANIVFILE=MODELS\pshell_a.3D DATAFILE=MODELS\pshell_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=RocketCanMesh X=0 Y=0 Z=-15 YAW=0
#exec MESH SEQUENCE MESH=RocketCanMesh SEQ=All    STARTFRAME=0  NUMFRAMES=10
#exec MESH SEQUENCE MESH=RocketCanMesh SEQ=Open   STARTFRAME=0  NUMFRAMES=10
#exec TEXTURE IMPORT NAME=JRocketCan1 FILE=MODELS\RocketCn.PCX GROUP="Skins"
#exec MESHMAP SCALE MESHMAP=RocketCanMesh X=0.06 Y=0.06 Z=0.12
#exec MESHMAP SETTEXTURE MESHMAP=RocketCanMesh NUM=1 TEXTURE=JRocketCan1

var bool bOpened;

auto state Pickup
{
	function Touch( Actor Other )
	{
		local Vector Dist2D;

		if ( bOpened )
		{
			Super.Touch(Other);
			return;
		}	
		if ( (Pawn(Other) == None) || !Pawn(Other).bIsPlayer )
			return;
		Dist2D = Other.Location - Location;
		Dist2D.Z = 0;
		if ( VSize(Dist2D) <= 48.0 )
			Super.Touch(Other);
		else if ( !bOpened )
		{
			SetCollisionSize(27.0, CollisionHeight);
			SetLocation(Location); //to force untouch
			bOpened = true;
			PlayAnim('Open', 0.1);
		}
	}

	function Landed(vector HitNormal)
	{
		Super.Landed(HitNormal);
		if ( !bOpened )
		{
			bCollideWorld = false;
			SetCollisionSize(172,CollisionHeight);
		}
	}
}

defaultproperties
{
     AmmoAmount=12
     MaxAmmo=48
     UsedInWeaponSlot(5)=1
     PickupMessage="You picked up 12 Eightballs"
     PickupViewMesh=Mesh'Unreal.RocketCanMesh'
     MaxDesireability=0.300000
     PickupSound=Sound'Unreal.Pickups.AmmoSnd'
     Icon=Texture'Unreal.Icons.I_RocketAmmo'
     Physics=PHYS_Falling
     Mesh=Mesh'Unreal.RocketCanMesh'
     bMeshCurvy=False
     CollisionRadius=27.000000
     CollisionHeight=12.000000
     bCollideActors=True
}
