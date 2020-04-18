//=============================================================================
// DefaultAmmo.
//=============================================================================
class DefaultAmmo expands ASMDAmmo;

#exec TEXTURE IMPORT NAME=I_Dispersion FILE=TEXTURES\HUD\i_disp.PCX GROUP="Icons" MIPS=OFF

var() float RechargeDelay;

Auto State Idle2
{

function Timer()
{
	if( AmmoAmount < MaxAmmo)
		AmmoAmount++;
	if ( AmmoAmount < 10 )
		SetTimer(RechargeDelay, false);
	else
		SetTimer(RechargeDelay * 0.1 * AmmoAmount, false);		
}

Begin:
	SetTimer(RechargeDelay, false);

}

defaultproperties
{
     RechargeDelay=1.100000
     UsedInWeaponSlot(1)=1
     UsedInWeaponSlot(4)=0
     Icon=Texture'Unreal.Icons.I_Dispersion'
     Mesh=None
     bMeshCurvy=True
     CollisionRadius=30.000000
     CollisionHeight=30.000000
     bCollideActors=False
}
