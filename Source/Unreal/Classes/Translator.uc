//=============================================================================
// Translator.
//=============================================================================
class Translator expands Pickup;

#exec AUDIO IMPORT FILE="Sounds\Pickups\HEALTH1.WAV" NAME="HEALTH1" GROUP="Pickups"

#exec TEXTURE IMPORT NAME=I_Tran FILE=TEXTURES\HUD\i_TRAN.PCX GROUP="Icons" MIPS=OFF

#exec MESH IMPORT MESH=TranslatorMesh ANIVFILE=MODELS\tran_a.3D DATAFILE=MODELS\tran_d.3D X=0 Y=0 Z=0
#exec MESH ORIGIN MESH=TranslatorMesh X=0 Y=0 Z=0 YAW=0
#exec MESH SEQUENCE MESH=TranslatorMesh SEQ=All    STARTFRAME=0  NUMFRAMES=1
#exec TEXTURE IMPORT NAME=JTranslator1 FILE=MODELS\tran.PCX GROUP="Skins" 
#exec MESHMAP SCALE MESHMAP=TranslatorMesh X=0.04 Y=0.04 Z=0.08
#exec MESHMAP SETTEXTURE MESHMAP=TranslatorMesh NUM=1 TEXTURE=JTranslator1

var() string[255] NewMessage;
var string[255] OldMessage, OlderMessage, Hint;
var bool bNewMessage, bNotNewMessage, bShowHint;

replication
{
	// Things the server should send to the client.
	reliable if( Role==ROLE_Authority && bNetOwner )
		NewMessage, bNewMessage, bNotNewMessage;
}

state Activated
{

Begin: 
}

state Deactivated
{
Begin:
	bShowHint = False;
	bNewMessage = False;
	bNotNewMessage = False;
}

function ActivateTranslator(bool bHint)
{
	if (bHint && Hint=="")
	{
		bHint=False;
		Return;
	}
	bShowHint = bHint;
	Activate();
}

defaultproperties
{
     NewMessage="Universal Translator"
     bActivatable=True
     bDisplayableInv=True
     PickupMessage="Press F2 to activate the Translator"
     PickupViewMesh=Mesh'Unreal.TranslatorMesh'
     PickupSound=Sound'Unreal.Pickups.GenPickSnd'
     Icon=Texture'Unreal.Icons.I_Tran'
     Mesh=Mesh'Unreal.TranslatorMesh'
     bMeshCurvy=False
     CollisionHeight=5.000000
}
