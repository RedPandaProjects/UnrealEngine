/*=============================================================================
	CoolBsp.cpp: Cool new Bsp code.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EditorPrivate.h"

/*-------------------------------------------------------------------------------
	FWinding.
-------------------------------------------------------------------------------*/

//
// Split flags.
//
enum ESplitWindingFlags
{
	SPW_Front	= 1,	// There was a front fragment.
	SPW_Back	= 2,	// There was a back fragment.
	SPW_Split	= 3,	// There were front and back fragments.
};

//
// Winding classification.
//
struct FClassify
{
	FLOAT Dist;
	INT Flags;
	FClassify( FPlane Cutter, FVector Vector, FLOAT Epsilon )
	:	Dist	( Cutter.PlaneDot( Vector ) )
	,	Flags	( Dist<-Epsilon ? SPW_Back : Dist>=Epsilon ? SPW_Front : 0 )
	{}
};

//
// A convex, dynamic, planar winding of points.
//
struct FWinding
{
	// Variables.
	FPlane          Plane;
	INT             iSurf;
	TArray<FVector> Points;

	// Constructor.
	FWinding( FPlane InPlane, INT iInSurf )
	:	Plane   ( InPlane )
	,	iSurf	( iInSurf )
	,	Points	()
	{}

	// Split the winding by a plane. If Epsilon==0.0, guaranteed to produce at least one fragment.
	DWORD Split( FPlane Cutter, FWinding* Front, FWinding* Back, FLOAT Epsilon )
	{
		guardSlow(FWinding::Split);
		DWORD AllFlags=0;

		// Classify all points as front or back.
		TArray<FClassify> Classifies(Points.Num());
		TIterator<FClassify>ItC(Classifies);
		for( TIterator<FVector>ItP(Points); ItP; ++ItP,++ItC )
		{
			*ItC = FClassify( Cutter, *ItP, Epsilon );
			AllFlags  |= ItC->Flags;
		}

		// Init front and back.
		if( Front )
			*Front = AllFlags==SPW_Front ? *this : FWinding( Plane, iSurf );
		if( Back )
			*Back = AllFlags==SPW_Back ? *this : FWinding( Plane, iSurf );

		// Split front and back windings.
		if( AllFlags==SPW_Split && (Front || Back) )
		{
			TIterator<FClassify>ItC(Classifies);
			for( TIterator<FVector>ItP(Points); ItP; ++ItP,++ItC )
			{
				// Add transition point to front and back.
				if( (ItC.GetCurrent().Dist>=0) != (ItC.GetPrev().Dist>=0) )
				{
					// Clamp safely clamps infinities.
					FLOAT   Alpha = Clamp( ItC.GetPrev().Dist / (ItC.GetPrev().Dist - ItC.GetCurrent().Dist), 0.f, 1.f );
					FVector Point = Lerp( ItP.GetPrev(), ItP.GetCurrent(), Alpha );
					if( Back )
						Back->Points.AddItem( Point );
					if( Front )
						Front->Points.AddItem( Point );
				}

				// Add current point to front or back.
				if( ItC->Dist>=0.0 )
				{
					if( Front )
						Front->Points.AddItem( *ItP );
				}
				else
				{
					if( Back )
						Back->Points.AddItem( *ItP );
				}
			}
		}
		return AllFlags;
		unguardSlow;
	}

	// Compute the normal plane.
	void CalcPlane()
	{
		guardSlow(FWinding::CalcPlane);
		if( Points.Num() )
		{
			FVector Accum(0,0,0);
			FLOAT F=0;
			for( TIterator<FVector>It(Points); It; ++It )
				Accum += ((It.GetNext()-It.GetCurrent())^(It.GetPrev()-It.GetCurrent()));
			Plane = FPlane( Points(0), Accum.SafeNormal() );
		}
		else Plane = FPlane(0,0,0,0);
		unguardSlow;
	}

	// Reverse the winding.
	void Reverse()
	{
		guardSlow(FWinding::Reverse);
		for( INT i=0; i<Points.Num()/2; i++ )
			Exchange( Points(i), Points(Points.Num()-i-1) );
		unguardSlow;
	}
};

/*-------------------------------------------------------------------------------
	Functions.
-------------------------------------------------------------------------------*/

//
// Convert the BSP to windings.
//
void BspToWindings( UModel* Model, INT iNode, TArray<FWinding>& Windings )
{
	guard(BspToWindings);
	FBspNode& Node = Model->Nodes->Element(iNode);
	if( Node.iFront )
		BspToWindings( Model, Node.iFront, Windings );
	if( Node.iBack )
		BspToWindings( Model, Node.iBack, Windings );
	while( iNode != INDEX_NONE )
	{
		FBspNode& Node = Model->Nodes->Element(iNode);
		if( Node.NumVertices )
		{
			FWinding* Winding = new(Windings)FWinding(Node.Plane.Flip(),Node.iSurf);
			for( INT i=Node.NumVertices-1; i>=0; i-- )
				new(Winding->Points)FVector(Model->Points->Element(Model->Verts->Element(Node.iVertPool+i).pVertex));
		}
		iNode = Node.iPlane;
	}
	unguard;
}

//
// Convert the brush FPolys to windings.
//
void BrushPolysToWindings( UModel* Model, TArray<FWinding>& Windings )
{
	guard(BrushPolysToWindings);
	for( INT i=0; i<Model->Polys->Num(); i++ )
	{
		FPoly&    Poly    = Model->Polys->Element(i);
		FWinding* Winding = new(Windings)FWinding(Poly.Normal,INDEX_NONE);
		for( INT i=0; i<Poly.NumVertices; i++ )
			new(Winding->Points)FVector(Poly.Vertex[i]);
	}
	unguard;
}

//
// Convert the windings to active brush FPolys.
//
void WindingsToBrushPolys( const TArray<FWinding>& Windings, UModel* Model )
{
	guard(WindingsToBrushPolys);
	Model->Polys->Empty();
	for( TConstIterator<FWinding>ItW(Windings); ItW; ++ItW )
	{
		INT    Index     = Model->Polys->Add();
		FPoly& Poly      = Model->Polys->Element(Index);
		Poly.Init();
		Poly.Normal      = ItW->Plane;
		Poly.NumVertices = ItW->Points.Num();
		Poly.iLink       = ItW.GetIndex();
		for( TConstIterator<FVector>ItP(ItW->Points); ItP; ++ItP )
			Poly.Vertex[ItP.GetIndex()] = *ItP;
	}
	unguard;
}

enum ENewNodeFlags
{
	NNF_Leaf = 1,
};

struct FNode
{
	// Variables.
	FPlane Plane;
	FNode* Front;
	FNode* Back;
	TArray<FWinding> Windings;

	// Structors.
	~FNode()
	{
		if( Front )
			delete Front;
		if( Back )
			delete Back;
	};

	// Functions.
	FNode( FPlane InPlane )
	:	Plane	( InPlane )
	,	Front	( NULL )
	,	Back	( NULL )
	,	Windings()
	{}
};

//
// For testing.
//
void TimTim( ULevel* Level )
{
	guard(TimTim);
	//TArray<FWinding> Windings;
	//BrushPolysToWindings( Level->Brush()->Brush, Windings );
	//BspToWindings( Level->Model, 0, Windings );
	//WindingsToBrushPolys( Windings, Level->Brush()->Brush );
	/*INT Id=0,Count=0,i,j;
	for( FStaticBrushIterator It1(Level); It1; ++It1 )
	{
		Count++;
		UModel* B1 = It1->Brush;
		for( FStaticBrushIterator It2(Level); *It2!=*It1; ++It2 )
		{
			UModel* B2 = It2->Brush;
			if( B1->Polys->Num()!=B2->Polys->Num() || *It1==Level->Brush() || *It2==Level->Brush() )
				goto Skip;
			for( i=0; i<B1->Polys->Num(); i++ )
			{
				FPoly* P1=&B1->Polys->Element(i);
				FPoly* P2=&B2->Polys->Element(i);
				if
				(	P1->NumVertices != P2->NumVertices 
				||	P1->Normal      != P2->Normal
				||	P1->TextureU    != P2->TextureU
				||	P1->TextureV    != P2->TextureV
				||	P1->PolyFlags   != P2->PolyFlags )
					goto Skip;
				for( j=0; j<B1->Polys->Element(i).NumVertices; j++ )
				{
					if( B1->Polys->Element(i).Vertex[j] != B2->Polys->Element(i).Vertex[j] )
						goto Skip;
				}
			}
			Id++;
			goto Next;
			Skip:;
		}
		Next:;
	}
	debugf( NAME_Log, "Brushes=%i Identical=%i", Count, Id );*/
	unguard;
}

/*-------------------------------------------------------------------------------
	The End.
-------------------------------------------------------------------------------*/
