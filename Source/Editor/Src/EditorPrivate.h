/*=============================================================================
	EditorPrivate.h: Unreal editor public header file.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

/*----------------------------------------------------------------------------
	API.
----------------------------------------------------------------------------*/
#ifdef EDITOR_EXPORTS
#define EDITOR_API DLL_EXPORT
#else
#define EDITOR_API DLL_IMPORT
#endif


/*-----------------------------------------------------------------------------
	Editor public includes.
-----------------------------------------------------------------------------*/

#include "Editor.h"

/*-----------------------------------------------------------------------------
	Editor private includes.
-----------------------------------------------------------------------------*/

#include "UnEdTran.h"
#include "UnTopics.h"

/*-----------------------------------------------------------------------------
	Editor private definitions.
-----------------------------------------------------------------------------*/

//
// Quality level for rebuilding Bsp.
//
enum EBspOptimization
{
	BSP_Lame,
	BSP_Good,
	BSP_Optimal
};

//
// Things to set in mapSetBrush.
//
enum EMapSetBrushFlags				
{
	MSB_BrushColor	= 1,			// Set brush color.
	MSB_Group		= 2,			// Set group.
	MSB_PolyFlags	= 4,			// Set poly flags.
};

//
// Possible positions of a child Bsp node relative to its parent (for BspAddToNode).
//
enum ENodePlace 
{
	NODE_Back		= 0, // Node is in back of parent              -> Bsp[iParent].iBack.
	NODE_Front		= 1, // Node is in front of parent             -> Bsp[iParent].iFront.
	NODE_Plane		= 2, // Node is coplanar with parent           -> Bsp[iParent].iPlane.
	NODE_Root		= 3, // Node is the Bsp root and has no parent -> Bsp[0].
};

EDITOR_API extern class FGlobalTopicTable GTopics;

/*-----------------------------------------------------------------------------
	UClassFactory.
-----------------------------------------------------------------------------*/

class EDITOR_API UClassFactory : public UFactory
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UClassFactory,UFactory,0)

	// Constructor.
	UClassFactory();

	// UFactory interface.
	UObject* Create( UClass* Class, UObject* InParent, FName Name, UObject* Context, const char* Type, const char*& Buffer, const char* BufferEnd, FFeedbackContext* Warn=NULL );
};

/*-----------------------------------------------------------------------------
	ULevelFactory.
-----------------------------------------------------------------------------*/

class EDITOR_API ULevelFactory : public UFactory
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(ULevelFactory,UFactory,0)

	// Constructor.
	ULevelFactory();

	// UFactory interface.
	UObject* Create( UClass* Class, UObject* InParent, FName Name, UObject* Context, const char* Type, const char*& Buffer, const char* BufferEnd, FFeedbackContext* Warn=NULL );
};

/*-----------------------------------------------------------------------------
	UPolysFactory.
-----------------------------------------------------------------------------*/

class EDITOR_API UPolysFactory : public UFactory
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UPolysFactory,UFactory,0)

	// Constructor.
	UPolysFactory();

	// UFactory interface.
	UObject* Create( UClass* Class, UObject* InParent, FName Name, UObject* Context, const char* Type, const char*& Buffer, const char* BufferEnd, FFeedbackContext* Warn=NULL );
};

/*-----------------------------------------------------------------------------
	UModelFactory.
-----------------------------------------------------------------------------*/

class EDITOR_API UModelFactory : public UFactory
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UModelFactory,UFactory,0)

	// Constructor.
	UModelFactory();

	// UFactory interface.
	UObject* Create( UClass* Class, UObject* InParent, FName Name, UObject* Context, const char* Type, const char*& Buffer, const char* BufferEnd, FFeedbackContext* Warn=NULL );
};

/*-----------------------------------------------------------------------------
	USoundFactory.
-----------------------------------------------------------------------------*/

class EDITOR_API USoundFactory : public UFactory
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(USoundFactory,UFactory,0)

	// Constructor.
	USoundFactory();

	// UFactory interface.
	UObject* Create( UClass* Class, UObject* InParent, FName Name, UObject* Context, const char* Type, const char*& Buffer, const char* BufferEnd, FFeedbackContext* Warn=NULL );
};

/*-----------------------------------------------------------------------------
	UMusicFactory.
-----------------------------------------------------------------------------*/

class EDITOR_API UMusicFactory : public UFactory
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UMusicFactory,UFactory,0)

	// Constructor.
	UMusicFactory();

	// UFactory interface.
	UObject* Create( UClass* Class, UObject* InParent, FName Name, UObject* Context, const char* Type, const char*& Buffer, const char* BufferEnd, FFeedbackContext* Warn=NULL );
};

/*-----------------------------------------------------------------------------
	UTextureFactory.
-----------------------------------------------------------------------------*/

class EDITOR_API UTextureFactory : public UFactory
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UTextureFactory,UFactory,0)

	// Constructor.
	UTextureFactory();

	// UFactory interface.
	UObject* Create( UClass* Class, UObject* InParent, FName Name, UObject* Context, const char* Type, const char*& Buffer, const char* BufferEnd, FFeedbackContext* Warn=NULL );
};

/*-----------------------------------------------------------------------------
	UFontFactory.
-----------------------------------------------------------------------------*/

class EDITOR_API UFontFactory : public UTextureFactory
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UFontFactory,UTextureFactory,0)

	// Constructor.
	UFontFactory();

	// UFactory interface.
	UObject* Create( UClass* Class, UObject* InParent, FName Name, UObject* Context, const char* Type, const char*& Buffer, const char* BufferEnd, FFeedbackContext* Warn=NULL );
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
