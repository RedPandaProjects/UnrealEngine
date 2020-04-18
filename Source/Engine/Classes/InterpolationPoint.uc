//=============================================================================
// InterpolationPoint.
//=============================================================================
class InterpolationPoint expands Keypoint
	intrinsic;

// Sprite.
#exec Texture Import File=Textures\IntrpPnt.pcx Name=S_Interp Mips=Off Flags=2

// Number in sequence sharing this tag.
var() int   Position;
var() float RateModifier;
var() bool  bEndOfPath;

// Other points in this interpolation path.
var InterpolationPoint Prev, Next;

//
// At start of gameplay, link all matching interpolation points together.
//
function BeginPlay()
{
	Super.BeginPlay();

	// Try to find previous.
	foreach AllActors( class 'InterpolationPoint', Prev, Tag )
		if( Prev.Position == Position-1 )
			break;
	if( Prev != None )
		Prev.Next = Self;

	// Try to find next.
	foreach AllActors( class 'InterpolationPoint', Next, Tag )
		if( Next.Position == Position+1 )
			break;
	if( Next == None )
		foreach AllActors( class 'InterpolationPoint', Next, Tag )
			if( Next.Position == 0 )
				break;
	if( Next != None )
		Next.Prev = Self;
}

//
// Verify that we're linked up.
//
function PostBeginPlay()
{
	Super.PostBeginPlay();
	//log( "Interpolation point " $ Tag $ " # " $ Position $ ":" );
	//if( Prev != None )
	//	log( "   Prev # " $ Prev.Position );
	//if( Next != None )
	//	log( "   Next # " $ Next.Position );
}

//
// When reach an interpolation point.
//
function InterpolateEnd( actor Other )
{
	if( bEndOfPath )	
	{
		if( Pawn(Other)!=None && Pawn(Other).bIsPlayer )
		{
			Pawn(Other).GotoState('PlayerWalking');
			Other.SetCollision(Other.bCollideActors,true,true);
		}
	}
}

defaultproperties
{
     RateModifier=+00001.000000
     bStatic=False
     bDirectional=True
	 Texture=S_Interp
}
