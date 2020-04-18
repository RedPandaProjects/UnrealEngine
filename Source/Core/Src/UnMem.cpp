/*=============================================================================
	UnMem.cpp: Unreal memory grabbing functions
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Revision history:
	* Created by Tim Sweeney
=============================================================================*/

#include "CorePrivate.h"

/*-----------------------------------------------------------------------------
	FMemStack statics.
-----------------------------------------------------------------------------*/

FMemStack::FTaggedMemory* FMemStack::UnusedChunks = NULL;

/*-----------------------------------------------------------------------------
	FMemStack implementation.
-----------------------------------------------------------------------------*/

//
// Initialize this memory stack.
//
void FMemStack::Init( INT InDefaultChunkSize )
{
	guard(FMemStack::Init);

	DefaultChunkSize = InDefaultChunkSize;
	TopChunk = NULL;
	End      = NULL;
	Top		 = NULL;

	unguard;
}

//
// Timer tick. Makes sure the memory stack is empty.
//
void FMemStack::Tick()
{
	guard(FMemStack::InitStack);
	check(TopChunk==NULL);
	unguard;
}

//
// Free this memory stack.
//
void FMemStack::Exit()
{
	guard(FMemStack::Exit);
	Tick();
	while( UnusedChunks )
	{
		void* Old = UnusedChunks;
		UnusedChunks = UnusedChunks->Next;
		appFree( Old );
	}
	unguard;
}

//
// Return the amount of bytes that have been allocated from the
// cache by this memory stack.
//
INT FMemStack::GetByteCount()
{
	guard(FMemStack::GetByteCount);
	INT Count = 0;
	for( FTaggedMemory* Chunk=TopChunk; Chunk; Chunk=Chunk->Next )
	{
		if( Chunk!=TopChunk )
			Count += Chunk->DataSize;
		else
			Count += Top - Chunk->Data;
	}
	return Count;
	unguard;
}

/*-----------------------------------------------------------------------------
	Chunk functions.
-----------------------------------------------------------------------------*/

//
// Allocate a new chunk of memory of at least MinSize size,
// and return it aligned to Align. Updates the memory stack's
// Chunks table and ActiveChunks counter.
//
BYTE* FMemStack::AllocateNewChunk( INT MinSize )
{
	guard(FMemStack::AllocateNewChunk);

	FTaggedMemory* Chunk=NULL;
	for( FTaggedMemory** Link=&UnusedChunks; *Link; Link=&(*Link)->Next )
	{
		// Find existing chunk.
		if( (*Link)->DataSize >= MinSize )
		{
			Chunk = *Link;
			*Link = (*Link)->Next;
			break;
		}
	}
	if( !Chunk )
	{
		// Create new chunk.
		INT DataSize    = Max(MinSize,DefaultChunkSize);
		Chunk           = (FTaggedMemory*)appMalloc( 256/*!!*/ + DataSize + sizeof(FTaggedMemory), "MemChunk" );
		Chunk->DataSize = DataSize;
	}
	Chunk->Next = TopChunk;
	TopChunk    = Chunk;
	Top         = Chunk->Data;
	End         = Top + Chunk->DataSize;

	return Top;
	unguard;
}

void FMemStack::FreeChunks( FTaggedMemory* NewTopChunk )
{
	guard(FMemStack::FreeChunks);
	while( TopChunk!=NewTopChunk )
	{
		FTaggedMemory* RemoveChunk = TopChunk;
		TopChunk                   = TopChunk->Next;
		RemoveChunk->Next          = UnusedChunks;
		UnusedChunks               = RemoveChunk;
	}
	Top = NULL;
	End = NULL;
	if( TopChunk )
	{
		Top = TopChunk->Data;
		End = Top + TopChunk->DataSize;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
