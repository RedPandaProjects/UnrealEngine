/*=============================================================================
	UnMesh.h: Unreal mesh objects.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	FMeshVert.
-----------------------------------------------------------------------------*/

// Packed mesh vertex point for skinned meshes.
struct FMeshVert
{
	// Variables.
	union
	{
#if __INTEL__
		struct {INT X:11; INT Y:11; INT Z:10;};
#else
		struct {INT Z:10; INT Y:11; INT X:11;};
#endif
		struct {DWORD D;};
	};

	// Constructor.
	FMeshVert()
	{}
	FMeshVert( const FVector& In )
	: X(In.X), Y(In.Y), Z(In.Z)
	{}

	// Functions.
	FVector Vector() const
	{
		return FVector( X, Y, Z );
	}

	// Serializer.
	friend FArchive &operator<<( FArchive& Ar, FMeshVert& V )
	{
		return Ar << V.D;
	}
};

/*-----------------------------------------------------------------------------
	FMeshTri.
-----------------------------------------------------------------------------*/

// Texture coordinates associated with a vertex and one or more mesh triangles.
// All triangles sharing a vertex do not necessarily have the same texture
// coordinates at the vertex.
struct FMeshUV
{
	BYTE U;
	BYTE V;
	friend FArchive &operator<<( FArchive& Ar, FMeshUV& M )
		{return Ar << M.U << M.V;}
};

// One triangular polygon in a mesh, which references three vertices,
// and various drawing/texturing information.
struct FMeshTri
{
	_WORD		iVertex[3];		// Vertex indices.
	FMeshUV		Tex[3];			// Texture UV coordinates.
	DWORD		PolyFlags;		// Surface flags.
	INT			TextureIndex;	// Source texture index.
	friend FArchive &operator<<( FArchive& Ar, FMeshTri& T )
	{
		Ar << T.iVertex[0] << T.iVertex[1] << T.iVertex[2];
		Ar << T.Tex[0] << T.Tex[1] << T.Tex[2];
		Ar << T.PolyFlags << T.TextureIndex;
		return Ar;
	}
};

/*-----------------------------------------------------------------------------
	FMeshAnimNotify.
-----------------------------------------------------------------------------*/

// An actor notification event associated with an animation sequence.
struct FMeshAnimNotify
{
	FLOAT	Time;			// Time to occur, 0.0-1.0.
	FName	Function;		// Name of the actor function to call.
	friend FArchive &operator<<( FArchive& Ar, FMeshAnimNotify& N )
		{return Ar << N.Time << N.Function;}
	FMeshAnimNotify()
		: Time(0.0), Function(NAME_None) {}
};

/*-----------------------------------------------------------------------------
	FMeshAnimSeq.
-----------------------------------------------------------------------------*/

// Information about one animation sequence associated with a mesh,
// a group of contiguous frames.
struct FMeshAnimSeq
{
	FName					Name;		// Sequence's name.
	FName					Group;		// Group.
	INT						StartFrame;	// Starting frame number.
	INT						NumFrames;	// Number of frames in sequence.
	FLOAT					Rate;		// Playback rate in frames per second.
	TArray<FMeshAnimNotify> Notifys;	// Notifications.
	friend FArchive &operator<<( FArchive& Ar, FMeshAnimSeq& A )
		{return Ar << A.Name << A.Group << A.StartFrame << A.NumFrames << A.Notifys << A.Rate;}
	FMeshAnimSeq()
		: Name(NAME_None), Group(NAME_None), StartFrame(0), NumFrames(0), Rate(30.0), Notifys() {}
};

/*-----------------------------------------------------------------------------
	FMeshVertConnect.
-----------------------------------------------------------------------------*/

// Says which triangles a particular mesh vertex is associated with.
// Precomputed so that mesh triangles can be shaded with Gouraud-style
// shared, interpolated normal shading.
struct FMeshVertConnect
{
	INT	NumVertTriangles;
	INT	TriangleListOffset;
	friend FArchive &operator<<( FArchive& Ar, FMeshVertConnect& C )
		{return Ar << C.NumVertTriangles << C.TriangleListOffset;}
};

/*-----------------------------------------------------------------------------
	UMesh.
-----------------------------------------------------------------------------*/

//
// A mesh, completely describing a 3D object (creature, weapon, etc) and
// its animation sequences.  Does not reference textures.
//
class ENGINE_API UMesh : public UPrimitive
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UMesh,UPrimitive,0)

	// Objects.
	TArray<FMeshVert>		Verts;
	TArray<FMeshTri>		Tris;
	TArray<FMeshAnimSeq>	AnimSeqs;
	TArray<FMeshVertConnect>Connects;
	TArray<FBox>			BoundingBoxes;
	TArray<FSphere>			BoundingSpheres;
	TArray<INT>				VertLinks;
	TArray<UTexture*>		Textures;

	// Counts.
	INT						FrameVerts;
	INT						AnimFrames;

	// Render info.
	DWORD					AndFlags;
	DWORD					OrFlags;

	// Scaling.
	FVector					Scale;		// Mesh scaling.
	FVector 				Origin;		// Origin in original coordinate system.
	FRotator				RotOrigin;	// Amount to rotate when importing (mostly for yawing).

	// Editing info.
	INT						CurPoly;	// Index of selected polygon.
	INT						CurVertex;	// Index of selected vertex.

	// UObject interface.
	UMesh();
	void Serialize( FArchive& Ar );

	// UPrimitive interface.
	FBox GetRenderBoundingBox( const AActor* Owner, UBOOL Exact ) const;
	FSphere GetRenderBoundingSphere( const AActor* Owner, UBOOL Exact ) const;
	UBOOL LineCheck
	(
		FCheckResult&	Result,
		AActor*			Owner,
		FVector			End,
		FVector			Start,
		FVector			Size,
		DWORD           ExtraNodeFlags
	);

	// UMesh interface.
	UMesh( int NumPolys, int NumVerts, int NumFrames );
	const FMeshAnimSeq* GetAnimSeq( FName SeqName ) const
	{
		guardSlow(UMesh::GetAnimSeq);
		for( int i=0; i<AnimSeqs.Num(); i++ )
			if( SeqName == AnimSeqs(i).Name )
				return &AnimSeqs(i);
		return NULL;
		unguardSlow;
	}
	FMeshAnimSeq* GetAnimSeq( FName SeqName )
	{
		guardSlow(UMesh::GetAnimSeq);
		for( int i=0; i<AnimSeqs.Num(); i++ )
			if( SeqName == AnimSeqs(i).Name )
				return &AnimSeqs(i);
		return NULL;
		unguardSlow;
	}
	void GetFrame( FVector* Verts, INT Size, FCoords Coords, AActor* Owner );
	void AMD3DGetFrame( FVector* Verts, INT Size, FCoords Coords, AActor* Owner );
	UTexture* GetTexture( INT Count, AActor* Owner )
	{
		guardSlow(UMesh::GetTexture);
		if( Count!=0 && Textures(Count) )
			return Textures(Count);
		else if( Owner && Owner->Skin )
			return Owner->Skin;
		else
			return Textures(Count);
		unguardSlow;
	}
};

/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
