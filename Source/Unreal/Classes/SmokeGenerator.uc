//=============================================================================
// SmokeGenerator.
//=============================================================================
class SmokeGenerator expands Effects;

#exec TEXTURE IMPORT NAME=SmokePuff43 FILE=MODELS\sp43.pcx GROUP=Effects

var() float SmokeDelay;		// pause between drips
var() float SizeVariance;		// how different each drip is 
var() float BasePuffSize;
var() int TotalNumPuffs;
var() float RisingVelocity;
var() class<effects> GenerationType;
var int i;

Auto State Active
{

function Timer()
{
	local Effects d;
	
	d = Spawn(GenerationType);
	d.DrawScale = BasePuffSize+FRand()*SizeVariance;	
	if (SpriteSmokePuff(d)!=None) SpriteSmokePuff(d).RisingRate = RisingVelocity;	
	i++;
	if (i>TotalNumPuffs && TotalNumPuffs!=0) Destroy();
}


function Trigger( actor Other, pawn EventInstigator )
{
	SetTimer(SmokeDelay+FRand()*SmokeDelay,True);
	i=0;
}


function UnTrigger( actor Other, pawn EventInstigator )
{
	i=0;
	if (TotalNumPuffs==0) Destroy();
}

}

defaultproperties
{
     SmokeDelay=+00000.150000
     SizeVariance=+00001.000000
     BasePuffSize=+00001.750000
     TotalNumPuffs=200
     RisingVelocity=+00075.000000
     GenerationType=Unreal.SpriteSmokePuff
     bHidden=True
     DrawType=DT_Sprite
     Style=STY_Masked
     Texture=Unreal.SmokePuff43
     bMeshCurvy=False
     Physics=PHYS_None
}
