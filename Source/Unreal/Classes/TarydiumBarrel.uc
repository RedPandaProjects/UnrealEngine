//=============================================================================
// TarydiumBarrel.
//=============================================================================
class TarydiumBarrel expands SteelBarrel;

#exec TEXTURE IMPORT NAME=Jsteelbarrel2 FILE=MODELS\steelbt.PCX GROUP="Skins"

var bool bChainedExplosion, bDestroy;

auto state active
{

	function Timer()
	{
		local int NumChunks;
		
		Super.Timer();
		
		if (!bDestroy) Return;
		
		NumChunks = 12;
		spawn(class'SpriteBallExplosion');
		spawn(class'RingExplosion3',,,Location - Vect(0,0,16),rot(16384,0,0));
		HurtRadius(250, 100, 'destroyed', 0, Location);
		if (bChainedExplosion) NumChunks = 4;
		skinnedFrag(class'Fragment1', texture'JSteelBarrel2', Vect(20000,0,0),1.0, 7);		
	}

	function TakeDamage( int NDamage, Pawn instigatedBy, Vector hitlocation, 
						Vector momentum, name damageType)
	{
		bChainedExplosion = False;
		bDestroy=True;
		if (DamageType=='destroyed') {
			SetTimer(FRand()*0.4+0.2,False);
			bChainedExplosion = True;
		}
		else Timer();
		Instigator = InstigatedBy;		
		if ( Instigator != None )
			MakeNoise(1.0);
	}

Begin:
}

defaultproperties
{
     Health=1
     Skin=Texture'Unreal.Jsteelbarrel2'
}
