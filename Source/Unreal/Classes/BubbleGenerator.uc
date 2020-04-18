//=============================================================================
// BubbleGenerator.
//=============================================================================
class BubbleGenerator expands Decoration;

#exec Texture Import File=models\bubble1.pcx  Name=S_bubble1 Mips=Off Flags=2


var() float NumBubbles;
var int i;

Auto state Active
{

function Timer()
{
	local vector X,Y,Z;

	i++;
	if (i>NumBubbles) UnTouch(None);
    Spawn(class'Bubble');

	SetTimer(FRand()*2.0+2.0,False);
}

function Trigger( actor Other, pawn EventInstigator )
{
	SetTimer(0.3,False);
	i=0;
}

function Touch( actor Other)
{
	SetTimer(0.3,False);
	i=0;
}

function UnTouch( actor Other)
{
	SetTimer(0.0, False);
}


Begin:
	bHidden = True;	
}

defaultproperties
{
     NumBubbles=15.000000
     bStatic=False
     Texture=Texture'Unreal.S_bubble1'
     DrawScale=0.500000
     CollisionRadius=64.000000
     CollisionHeight=64.000000
     bCollideActors=True
}
