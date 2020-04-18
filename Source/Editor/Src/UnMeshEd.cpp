/*=============================================================================
	UnMeshEd.cpp: Unreal editor mesh code
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EditorPrivate.h"
#include "UnRender.h"

/*-----------------------------------------------------------------------------
	Data types for importing James' creature meshes.
-----------------------------------------------------------------------------*/

// James mesh info.
struct FJSDataHeader
{
	_WORD	NumPolys;
	_WORD	NumVertices;
	_WORD	BogusRot;
	_WORD	BogusFrame;
	DWORD	BogusNormX,BogusNormY,BogusNormZ;
	DWORD	FixScale;
	DWORD	Unused1,Unused2,Unused3;
};

// James animation info.
struct FJSAnivHeader
{
	_WORD	NumFrames;		// Number of animation frames.
	_WORD	FrameSize;		// Size of one frame of animation.
};

// Mesh triangle.
struct FJSMeshTri
{
	_WORD		iVertex[3];		// Vertex indices.
	BYTE		Type;			// James' mesh type.
	BYTE		Color;			// Color for flat and Gouraud shaded.
	FMeshUV		Tex[3];			// Texture UV coordinates.
	BYTE		TextureNum;		// Source texture offset.
	BYTE		Flags;			// Unreal mesh flags (currently unused).
};

// Flags associated with a mesh triangle.
enum EJSMeshTriType
{
	// Triangle types.
	MTT_Normal				= 0,	// Normal.
	MTT_NormalTwoSided      = 1,    // Normal but two-sided.
	MTT_Translucent			= 2,	// Translucent.
	MTT_Masked				= 3,	// Masked.
	MTT_Modulate			= 4,	// Modulation blended.
	MTT_Placeholder			= 8,	// Placeholder triangle for positioning weapon.

	// Bit flags.
	MTT_Unlit				= 16,	// Full brightness, no lighting.
	MTT_Flat				= 32,	// Flat surface.
	MTT_Environment			= 64,	// Environment mapped.
	MTT_NoSmooth			= 128,	// No bilinear filtering.
};

/*-----------------------------------------------------------------------------
	Import functions.
-----------------------------------------------------------------------------*/

// Mesh sorting function.
INT Compare( const FMeshTri& A, const FMeshTri& B )
{
	if     ( A.TextureIndex > B.TextureIndex ) return 1;
	else if( A.TextureIndex < B.TextureIndex ) return -1;
	else if( A.PolyFlags    > B.PolyFlags    ) return 1;
	else if( A.PolyFlags    < B.PolyFlags    ) return -1;
	else                                       return 0;
}

//
// Import a mesh from James' editor.  Uses file commands instead of object
// manager.  Slow but works fine.
//
void UEditorEngine::meshImport
(
	const char* MeshName,
	UObject*	InParent,
	const char* AnivFname, 
	const char* DataFname,
	UBOOL		Unmirror,
	UBOOL		ZeroTex
)
{
	guard(UEditorEngine::meshImport);

	UMesh			*Mesh;
	FILE			*AnivFile,*DataFile;
	FJSDataHeader	JSDataHdr;
	FJSAnivHeader	JSAnivHdr;
	int				i;
	int				Ok = 0;

	debugf( NAME_Log, "Importing %s", MeshName );
	GSystem->BeginSlowTask( "Importing mesh", 1, 0 );
	GSystem->StatusUpdatef( 0, 0, "%s", "Reading files" );

	// Open James' animation vertex file and read header.
	AnivFile = appFopen( AnivFname, "rb" );
	if( AnivFile == NULL )
	{
		debugf( NAME_Log, "Error opening file %s", AnivFname );
		goto Out1;
	}
	if( appFread( &JSAnivHdr, sizeof(FJSAnivHeader), 1, AnivFile ) !=1 )
	{
		debugf( NAME_Log, "Error reading %s", AnivFname );
		goto Out2;
	}

	// Open James' mesh data file and read header.
	DataFile = appFopen( DataFname, "rb" );
	if( DataFile == NULL )
	{
		debugf( NAME_Log, "Error opening file %s", DataFname );
		goto Out2;
	}
	if( appFread( &JSDataHdr, sizeof( FJSDataHeader ), 1, DataFile ) !=1 )
	{
		debugf( NAME_Log, "Error reading %s", DataFile );
		goto Out3;
	}

	// Allocate mesh object.
	Mesh = new( InParent, MeshName, RF_Public|RF_Standalone )UMesh
	(
		JSDataHdr.NumPolys,
		JSDataHdr.NumVertices,
		JSAnivHdr.NumFrames
	);

	// Display summary info.
	debugf(NAME_Log," * Triangles  %i",Mesh->Tris.Num());
	debugf(NAME_Log," * Vertices   %i",Mesh->FrameVerts);
	debugf(NAME_Log," * AnimFrames %i",Mesh->AnimFrames);
	debugf(NAME_Log," * FrameSize  %i",JSAnivHdr.FrameSize);
	debugf(NAME_Log," * AnimSeqs   %i",Mesh->AnimSeqs.Num());

	// Import mesh triangles.
	debugf( NAME_Log, "Importing triangles" );
	GSystem->StatusUpdatef( 0, 0, "%s", "Importing Triangles" );
	appFseek( DataFile, 12, USEEK_CUR );
	for( i=0; i<Mesh->Tris.Num(); i++ )
	{
		guard(Importing triangles);

		// Load triangle.
		FJSMeshTri Tri;
		if( appFread( &Tri, sizeof(Tri), 1, DataFile ) != 1 )
		{
			debugf( NAME_Log, "Error processing %s", DataFile );
			goto Out4;
		}
		if( Unmirror )
		{
			Exchange( Tri.iVertex[1], Tri.iVertex[2] );
			Exchange( Tri.Tex    [1], Tri.Tex    [2] );
		}
		if( ZeroTex )
		{
			Tri.TextureNum = 0;
		}

		// Copy to Unreal structures.
		Mesh->Tris(i).iVertex[0]	= Tri.iVertex[0];
		Mesh->Tris(i).iVertex[1]	= Tri.iVertex[1];
		Mesh->Tris(i).iVertex[2]	= Tri.iVertex[2];
		Mesh->Tris(i).Tex[0]		= Tri.Tex[0];
		Mesh->Tris(i).Tex[1]		= Tri.Tex[1];
		Mesh->Tris(i).Tex[2]		= Tri.Tex[2];
		Mesh->Tris(i).TextureIndex	= Tri.TextureNum;
		while( Tri.TextureNum >= Mesh->Textures.Num() )
			Mesh->Textures.AddItem( NULL );

		// Set style based on triangle type.
		DWORD PolyFlags=0;
		if     ( (Tri.Type&15)==MTT_Normal         ) PolyFlags |= 0;
		else if( (Tri.Type&15)==MTT_NormalTwoSided ) PolyFlags |= PF_TwoSided;
		else if( (Tri.Type&15)==MTT_Modulate       ) PolyFlags |= PF_TwoSided | PF_Modulated;
		else if( (Tri.Type&15)==MTT_Translucent    ) PolyFlags |= PF_TwoSided | PF_Translucent;
		else if( (Tri.Type&15)==MTT_Masked         ) PolyFlags |= PF_TwoSided | PF_Masked;
		else if( (Tri.Type&15)==MTT_Placeholder    ) PolyFlags |= PF_TwoSided | PF_Invisible;

		// Handle effects.
		if     ( Tri.Type&MTT_Unlit             ) PolyFlags |= PF_Unlit;
		if     ( Tri.Type&MTT_Flat              ) PolyFlags |= PF_Flat;
		if     ( Tri.Type&MTT_Environment       ) PolyFlags |= PF_Environment;
		if     ( Tri.Type&MTT_NoSmooth          ) PolyFlags |= PF_NoSmooth;

		// Set flags.
		Mesh->Tris(i).PolyFlags = PolyFlags;

		unguard;
	};

	// Sort triangles by texture and flags.
	appSort( &Mesh->Tris(0), Mesh->Tris.Num() );

	// Import mesh vertices.
	debugf( NAME_Log, "Importing vertices" );
	GSystem->StatusUpdatef( 0, 0, "%s", "Importing Vertices" );
	for( i=0; i<Mesh->AnimFrames; i++ )
	{
		guard(Importing animation frames);
		FMeshVert *TempVert = &Mesh->Verts(i * Mesh->FrameVerts);
		int BytesRead = appFread( TempVert, sizeof(FMeshVert), Mesh->FrameVerts, AnivFile );
		if( BytesRead != Mesh->FrameVerts )
		{
			debugf( NAME_Log, "Vertex error in mesh %s, frame %i: expecting %i verts, got %i verts", AnivFname, i, Mesh->FrameVerts, BytesRead );
			//goto Out4;
		}
		if( Unmirror )
			for( int j=0; j<Mesh->FrameVerts; j++ )
				Mesh->Verts(i * Mesh->FrameVerts + j).X *= -1;
		appFseek( AnivFile, JSAnivHdr.FrameSize - Mesh->FrameVerts * sizeof(FMeshVert), USEEK_CUR );
		unguard;
	}

	// Build list of triangles per vertex.
	GSystem->StatusUpdatef( i, Mesh->FrameVerts, "%s", "Linking mesh" );
	for( i=0; i<Mesh->FrameVerts; i++ )
	{
		guard(Importing vertices);
		
		Mesh->Connects(i).NumVertTriangles = 0;
		Mesh->Connects(i).TriangleListOffset = Mesh->VertLinks.Num();
		for( int j=0; j<Mesh->Tris.Num(); j++ )
		{
			for( int k=0; k<3; k++ )
			{
				if( Mesh->Tris(j).iVertex[k] == i )
				{
					Mesh->VertLinks.AddItem(j);
					Mesh->Connects(i).NumVertTriangles++;
				}
			}
		}
		unguard;
	}
	debugf( NAME_Log, "Made %i links", Mesh->VertLinks.Num() );

	// Compute per-frame bounding volumes plus overall bounding volume.
	meshBuildBounds(Mesh);

	// Exit labels.
	Ok = 1;
	Out4: if (!Ok) {delete Mesh;}
	Out3: appFclose( DataFile );
	Out2: appFclose( AnivFile );
	Out1: GSystem->EndSlowTask();
	unguard;
}

/*-----------------------------------------------------------------------------
	Bounds.
-----------------------------------------------------------------------------*/

//
// Build bounding boxes for each animation frame of the mesh,
// and one bounding box enclosing all animation frames.
//
void UEditorEngine::meshBuildBounds( UMesh* Mesh )
{
	guard(UEditorEngine::meshBuildBounds);
	GSystem->StatusUpdatef( 0, 0, "%s", "Bounding mesh" );

	// Bound all frames.
	TArray<FVector> AllFrames;
	for( INT i=0; i<Mesh->AnimFrames; i++ )
	{
		TArray<FVector> OneFrame;
		for( INT j=0; j<Mesh->FrameVerts; j++ )
		{
			FVector Vertex = Mesh->Verts( i * Mesh->FrameVerts + j ).Vector();
			OneFrame .AddItem( Vertex );
			AllFrames.AddItem( Vertex );
		}
		Mesh->BoundingBoxes  (i) = FBox   ( OneFrame );
		Mesh->BoundingSpheres(i) = FSphere( OneFrame );
	}
	Mesh->BoundingBox    = FBox   ( AllFrames );
	Mesh->BoundingSphere = FSphere( AllFrames );

	// Display bounds.
	debugf
	(
		NAME_Log,
		"BoundingBox (%f,%f,%f)-(%f,%f,%f) BoundingSphere (%f,%f,%f) %f",
		Mesh->BoundingBox.Min.X,
		Mesh->BoundingBox.Min.Y,
		Mesh->BoundingBox.Min.Z,
		Mesh->BoundingBox.Max.X,
		Mesh->BoundingBox.Max.Y,
		Mesh->BoundingBox.Max.Z,
		Mesh->BoundingSphere.X,
		Mesh->BoundingSphere.Y,
		Mesh->BoundingSphere.Z,
		Mesh->BoundingSphere.W
	);
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
