/*=============================================================================
	UnStack.h: UnrealScript execution stack definition.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

class UStruct;

/*-----------------------------------------------------------------------------
	Execution stack helpers.
-----------------------------------------------------------------------------*/

//
// Information about script execution at one stack level.
//
struct CORE_API FFrame
{	
	// Variables.
	UStruct*	Node;
	UObject*	Object;
	BYTE*		Code;
	BYTE*		Locals;

	// Constructors.
	inline FFrame( UObject* InObject );
	inline FFrame( UObject* InObject, UStruct* InNode, INT CodeOffset, BYTE* InLocals );

	// Functions.
	void Step( UObject* Context, BYTE*& Result );
	void CDECL ScriptWarn( UBOOL Critical, char* Fmt, ... );
	INT ReadInt();
	FLOAT ReadFloat();
	INT ReadWord();
	FName ReadName();
};

//
// Information about script execution at the main stack level.
// This part of an actor's script state is saveable at any time.
//
struct CORE_API FMainFrame : public FFrame
{
	// Variables.
	UState* StateNode;
	QWORD   ProbeMask;
	INT     LatentAction;

	// Functions.
	FMainFrame( UObject* InObject );
	const char* Describe();
};

/*-----------------------------------------------------------------------------
	Script execution helpers.
-----------------------------------------------------------------------------*/

//
// Base class for UnrealScript iterator lists.
//
struct FIteratorList
{
	FIteratorList* Next;
	FIteratorList() {}
	FIteratorList( FIteratorList* InNext ) : Next( InNext ) {}
	FIteratorList* GetNext() { return (FIteratorList*)Next; }
};

//
// Information remembered about an Out parameter.
//
struct FOutParmRec
{
	BYTE* Dest;
	BYTE* Src;
	INT Size;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
