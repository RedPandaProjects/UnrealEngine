/*=============================================================================
	UnEdTran.cpp: Unreal transaction-tracking functions
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	The functions here are used for tracking modifications to the Bsp, editor
	solids, actors, etc., to support "Undo", "Redo", and automatically aborting
	operations which cause problems (while undoing their changes).

	See end of file for more information about all things tracked.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EditorPrivate.h"

/*-----------------------------------------------------------------------------
	All transaction-related data types.
-----------------------------------------------------------------------------*/

class FTransaction			// Transaction (one entry per transaction).
{
public:
	int	TransCount;			// Transaction number, starts at 0 when loaded and counts up.
	int	StartDataOffset;	// Data offset where transaction data starts.
	int	TotalDataSize;		// Total size of all data for the transaction.
	int	NumChanges;			// Number of changes thus far.
	char Name[80];			// Description of transaction.
};

class FTransChangeLog  		// Log of additions/deletions/modifications to records.
{
public:
	UObject		*Res;		// Object operated on.
	int			Index;		// Index of element.
	int			TransCount;	// Transaction number the change corresponds to.
	int			DataOffset;	// Offset of data in data buffer.
	int			DataSize;	// Size of this change's data in data buffer.
};

/*-----------------------------------------------------------------------------
	UTransBuffer constructor.
-----------------------------------------------------------------------------*/

UTransBuffer::UTransBuffer
(
	int ThisMaxTrans,
	int ThisMaxChanges,
	int ThisMaxDataOffset
)
{
	// Set values.
	MaxTrans		= ThisMaxTrans;
	MaxChanges		= ThisMaxChanges;
	MaxDataOffset	= ThisMaxDataOffset;

	// Allocate data.
	DWORD TransactionSize  = MaxTrans 	* sizeof (FTransaction);
	DWORD ChangeLogSize    = MaxChanges * sizeof (FTransChangeLog);
	DWORD DataSize		   = MaxDataOffset;
	DWORD TotalSize		   = TransactionSize + ChangeLogSize + DataSize;
	Data.Add( TotalSize );

	// Set action.
	appStrcpy( ResetAction, "startup" );

	// Subsystem init message.
	debugf( NAME_Init, "Transaction tracking system initialized" );
}

/*-----------------------------------------------------------------------------
	Lock, Unlock, and Reset.
-----------------------------------------------------------------------------*/

//
// Lock the transaction tracking object.
//
UBOOL UTransBuffer::Lock( DWORD )
{
	guard(UTransBuffer::Lock);
	
	DWORD	TransactionSize	= MaxTrans 		* sizeof (FTransaction);
	DWORD	ChangeLogSize	= MaxChanges	* sizeof (FTransChangeLog);
	DWORD	DataSize		= MaxDataOffset;

	if( !GIsRunning )
		return 1;

	Transactions 	= (FTransaction		*)&Data(0);
	ChangeLog 		= (FTransChangeLog  *)&Data(TransactionSize);
	Buffer 			= (BYTE				*)&Data(TransactionSize + ChangeLogSize);

	return 0;
	unguard;
}

//
// Unlock transaction buffer.
//
void UTransBuffer::Unlock( DWORD )
{
	guard(UTransBuffer::Unlock);
	
	if( !GIsRunning )
		return;

	unguard;
}

//
// Reset the transaction tracking system, clearing all transactions (so that
// following undo/redo operations can't occur).  This is called before a
// routine that invalidates the main set of level/transaction data, for example
// before rebuilding the Bsp.
//
// Action = description of reason for reset (i.e. "loading level", "rebuilding Bsp")
//
void UTransBuffer::Reset( const char* Action )
{
	guard(UTransBuffer::Reset);
	if( !GIsRunning )
		return;
	
	appStrcpy( ResetAction, Action );
	NumTrans		= 0;
	NumChanges		= 0;
	Overflow 		= 0;
	TransCount		= 0;
	UndoTransCount	= 0;

	debugf( NAME_Log, "Transaction buffer %s reset", GetName() );
	unguard;
}

/*-----------------------------------------------------------------------------
	Deletion.
-----------------------------------------------------------------------------*/

//
// Delete the first transaction from the transaction list, remove all of its entries
// from the change log and data tables, and scroll the list down.
//
void UTransBuffer::DeleteFirstTransaction()
{
	guard(UTransBuffer::DeleteFirstTransaction);

	FTransaction 	*FirstTrans,*LastTrans;
	int				MinTransCount;
	int				MinDataOffset,MaxDataOffset;
	int 			i,j;

	if( !GIsRunning )
		return;
	
	check(NumTrans!=0);	

	// Remove from *Transactions.
	for( i=1; i<NumTrans; i++ )
		Transactions[i-1] = Transactions[i];
	
	NumTrans--;

	if( NumTrans == 0 )
	{
		// All transactions are gone, so just empty everything.
		NumChanges = 0;
	}
	else
	{
		FirstTrans = &Transactions [0];
		LastTrans  = &Transactions [NumTrans-1];

		MinTransCount = FirstTrans->TransCount;

		MinDataOffset = FirstTrans->StartDataOffset;
		MaxDataOffset = LastTrans ->StartDataOffset + LastTrans->TotalDataSize;

		// Move data and update data offsets for all remaining transactions.
		appMemmove( Buffer, Buffer + MinDataOffset, MaxDataOffset-MinDataOffset );
		for( i=0; i<NumTrans; i++ )
			Transactions[i].StartDataOffset -= MinDataOffset;

		// Delete all old entries from TRANS.ChangeLog.
		j=0;
		for( i=0; i<NumChanges; i++ )
		{
			if( ChangeLog[i].TransCount >= MinTransCount )
			{
				if( j != i )
					ChangeLog[j] = ChangeLog[i];
				j++;
			}
			else
			{
				// The entry has been passed over (deleted).
			}
		}
		NumChanges = j;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Begin, End, Rollback, Continue.
-----------------------------------------------------------------------------*/

//
// Begin a new transaction.  If any undone transactions haven't been redone,
// they will be overwritten.
//
void UTransBuffer::Begin( ULevel* Level, const char* SessionName )
{
	guard(UTransBuffer::Begin);

	FTransaction	*Transaction;
	int				Index,Count;

	// Check existing transaction status.
	if (!GIsRunning) return;
	debugf(NAME_Log,"Begin transaction %s",SessionName);

	Lock(0);
	Overflow = 0;

	//
	// If transaction list is full, must scroll it back by deleting first entry.
	if (NumTrans >= MaxTrans) DeleteFirstTransaction();

	// If undo without matching redo's, cut off the rest of the list.
	Count = 0;
	for( int i=0; i<NumTrans; i++ )
	{
		if( Transactions[i].TransCount >= UndoTransCount )
		{
			NumTrans    = i;
			TransCount  = UndoTransCount;
			break;
		}
		else
		{
			Count += Transactions[i].NumChanges;
		}
	}
	NumChanges = Count;

	// Fill in *Transaction information.
	Index                	= NumTrans++;
	Transaction				= &Transactions[Index];
	Transaction->TransCount	= TransCount++;
	Transaction->NumChanges = 0;

	// Will be increased as changes are logged.
	appStrcpy( Transaction->Name, SessionName );
	Transaction->TotalDataSize = 0;

	// Set undo pointer to very top of transaction list.
	UndoTransCount    = TransCount;

	// Calc starting data offset.
	if( NumTrans==1 )
	{
		Transaction->StartDataOffset = 0;
	}
	else
	{
		Transaction->StartDataOffset =
		Transactions [Index-1].StartDataOffset +
		Transactions [Index-1].TotalDataSize;
	}
	GUndo=this;
	unguard;
}

//
// End a transaction
//
void UTransBuffer::End()
{
	guard(UTransBuffer::End);
	if( !GIsRunning )
		return;

	Unlock(0);
	if( Overflow )
	{
		debugf(NAME_Log,"End overflowed transaction");
		Reset("Undo buffer filled up");
	}
	else
	{
		//debugf (LOG_Trans,"End transaction");
	}
	GUndo=NULL;
	unguard;
}

void UTransBuffer::ForceOverflow( const char* Reason )
{
	guard(UTransBuffer::ForceOverflow);
	if (!GIsRunning) return;
	Overflow = 1;
	unguard;
}

//
// Abort a transaction and restore original state
//
void UTransBuffer::Rollback( ULevel* Level )
{
	guard(UTransBuffer::Rollback);

	if( !GIsRunning )
		return;
	
	if( Overflow )
	{
		// Can't rollback overflowed transactions because data is gone.
		End();
	}
	else
	{
		End();
		Undo(Level);

		// Prevent rolled-back transaction from being redone.
		if (NumTrans>0)		NumTrans--;
		if (TransCount>0)	TransCount--;

		UndoTransCount = TransCount;

		// Wipe out the rolled-back transaction's changes.
		NumChanges = 0;
		for( int i=0; i<NumTrans; i++ )
			NumChanges += Transactions[i].NumChanges;
	}
	unguard;
}

//
// Reopen the previous transaction and continue it
//
void UTransBuffer::Continue()
{
	guard(UTransBuffer::Continue);

	if( !GIsRunning )
		return;
	
	debugf(NAME_Log,"Continue previous transaction");

	Lock(0);
	if (NumTrans==0)	Overflow    = 1; // Assume previous transaction overflowed
	else				Overflow 	= 0;
	unguard;
}

/*-----------------------------------------------------------------------------
	Applying Undo/Redo changes.
-----------------------------------------------------------------------------*/

//
// Begin applying changes.
//
void UTransBuffer::BeginChanges( FTransaction* Trans )
{
	guard(UTransBuffer::BeginChanges);

	// Tag all objects as "not affected by transaction tracking system".
	for( FObjectIterator It; It; ++It )
		It->ClearFlags( RF_Trans | RF_TransData );

	unguard;
}

//
// End applying changes and do any necessary cleanup work.
//
void UTransBuffer::EndChanges( FTransaction* Trans )
{
	guard(UTransBuffer::EndChanges);

	// Postload all objects.
	for( FObjectIterator It; It; ++It )
	{
		if( It->GetFlags() & RF_Trans )
		{
			if( It->IsA(UModel::StaticClass) )
			{
				UModel* Model = (UModel*)*It;
				if
				(	Model->Nodes
				&&	Model->Surfs
				&&	((Model->Nodes->GetFlags() & RF_TransData) || (Model->Surfs->GetFlags() & RF_TransData)) )
					GEditor->bspBuildBounds(Model);
			}
			else if( It->IsA(UBspNodes::StaticClass) )
			{
				UBspNodes* Nodes = (UBspNodes*)*It;
				for( INT i=0; i<Nodes->Num(); i++ )
					Nodes->Element(i).NodeFlags &= ~(NF_IsNew);
			}
			It->ClearFlags( RF_Trans );
		}
	}
	unguard;
}

//
// Apply one logged change, Undo = 1 (Undo), 0 (Redo).
//
void UTransBuffer::ApplyChange( FTransChangeLog* Change, BYTE* SourcePtr, INT DataSize )
{
	guard(UTransBuffer::ApplyChange);

	if( Change->Index >= 0 )
	{
		UDatabase*	Res   	= (UDatabase*)Change->Res;
		UClass      *Class 	= Res->GetClass();
		BYTE		*DestPtr;
		check(DataSize==Class->ClassRecordSize);
		//debugf( "Applying %s %i", Res->GetFullName(), Change->Index );

		// Change a data record.
		DestPtr = ((BYTE*)((UDatabase*)Change->Res)->GetData()) + Change->Index * Class->ClassRecordSize;

		// Mark it.
		Res->SetFlags(RF_TransData);

		// Swap the memory.
		appMemswap( DestPtr, SourcePtr, DataSize );
	}
	else
	{
		// Change object.
		UObject *Dest   = Change->Res;
		UObject *Source	= (UObject *)SourcePtr;
		check(DataSize == Dest->GetClass()->GetPropertiesSize());
		//debugf( "Applying %s", Dest->GetFullName() );

		// Remember that we have performed a transaction on this object.
		DWORD DestFlags = RF_Trans | (Dest->GetFlags() & RF_TransData);

		void* CurrentData=NULL;
		int CurrentMax=0, NewMax;
		if( Dest->GetClass()->ClassRecordSize != 0 )
		{
			// Remember database's current data pointer.
			CurrentData	= ((UDatabase*)Dest)->GetData();
			CurrentMax  = ((UDatabase*)Dest  )->GetMax();
			NewMax      = ((UDatabase*)Source)->GetMax();

			// Reallocate data if max increased.
			if( NewMax > CurrentMax )
			{
				CurrentData = appRealloc
				(
					CurrentData,
					NewMax * Dest->GetClass()->ClassRecordSize,
					"Trans"
				);
				CurrentMax = NewMax;
			}
		}

		// Swap the object.
		appMemswap( Dest, Source, DataSize );

		if( Dest->GetClass()->ClassRecordSize != 0 )
		{
			// Update the data pointer and max.
			((UDatabase*)Dest)->SetData(CurrentData);
			((UDatabase*)Dest)->SetMax( CurrentMax );
		}

		// Mark the thing as unlocked.
		Dest->SetFlags(DestFlags);
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Undo.
-----------------------------------------------------------------------------*/

//
// Returns 1 if "undo" is possible, sets Str to name.
//
int UTransBuffer::CanUndo( char* Str )
{
	guard(UTransBuffer::CanUndo);

	if( !GIsRunning )
		return 0;
	
	Lock(0);

	if( (NumTrans==0) ||
		(UndoTransCount <= Transactions[0].TransCount) )
	{
		appStrcpy( Str, ResetAction );
		Unlock(0);
		return 0;
	}

	// Find transaction name.
	for( int i=0; i < NumTrans; i++ )
	{
		if( Transactions[i].TransCount == (UndoTransCount-1) )
		{
			appStrcpy( Str, Transactions[i].Name );
			Unlock(0);
			return 1;
		}
	}
	appStrcpy( Str, "aborted transaction" );
	Unlock(0);
	return 1;
	unguard;
}

//
// Undo a transaction, 1 if undone, 0 if not possible.
//
UBOOL UTransBuffer::Undo( ULevel* Level )
{
	guard(UTransBuffer::Undo);

	FTransaction	*Transaction;
	FTransChangeLog	*Change;
	char			Descr[256];
	INT 			i;

	if( !GIsRunning )
		return 0;

	// See if undo is possible.
	if( !CanUndo(Descr) )
	{
		if( UndoTransCount == 0 )
		{
			// At beginning of transaction list.
			debugf(NAME_Log,"Can't undo after %s",Descr);
		}
		else
		{
			debugf(NAME_Log,"At end of undo buffer");
		}
		return 0;
	}

	// Print name of what we're undoing.
	debugf(NAME_Log,"Undo %s",Descr);

	Lock(0);
	UndoTransCount--;

	// Find transaction corresponding to UndoTransCount.
	Transaction = &Transactions[0];
	for( i=0; i<NumTrans; i++ )
	{
		if (Transaction->TransCount == UndoTransCount) goto Found;
		Transaction++;
	}
	debugf(NAME_Log,"Not found");
	Unlock(0);
	return 0;

	// Apply all "undo" changes corresponding to UndoTransCount, in reverse order.
	Found:
	BeginChanges(Transaction);
	for( i=NumChanges-1; i >= 0; i-- )
	{
		Change = &ChangeLog[i];
		if( Change->TransCount == UndoTransCount )
		{
			ApplyChange
			(
				Change,
				Buffer + Transaction->StartDataOffset + Change->DataOffset,
				Change->DataSize
			);
		}
	}
	EndChanges( Transaction );
	Unlock(0);
	return 1;
	unguard;
}

/*-----------------------------------------------------------------------------
	Redo.
-----------------------------------------------------------------------------*/

//
// Returns 1 if "redo" is possible, sets Str to name.
//
int UTransBuffer::CanRedo( char* Str )
{
	guard(UTransBuffer::CanRedo);

	FTransaction	*Transaction;
	int 			i;

	if( !GIsRunning )
		return 0;
	
	Lock(0);

	if( (NumTrans==0) || (UndoTransCount >= TransCount) )
	{
		appStrcpy( Str, "" );
		Unlock(0);
		return 0;
	}

	// Find transaction name.
	Transaction = &Transactions[0];
	for( i=0; i<NumTrans; i++ )
	{
		if( Transaction->TransCount ==  UndoTransCount )
		{
			appStrcpy( Str, Transaction->Name );
			Unlock(0);
			return 1;
		}
		Transaction++;
	}
	appStrcpy( Str, "aborted transaction" );
	Unlock(0);
	return 1;
	unguard;
}

//
// Redo a transaction, 1 if undone, 0 if not possible.
//
int UTransBuffer::Redo( ULevel* Level )
{
	guard(UTransBuffer::Redo);

	FTransaction*		Transaction;
	FTransChangeLog*	Change;
	char				Descr[256];

	if( !GIsRunning )
		return 0;

	// See if redo is possible.
	if( !CanRedo(Descr) )
	{
		debugf(NAME_Log,"Nothing to redo",Descr);
		return 0; // Can't redo
	}
	
	// Print name of what we're undoing.
	debugf(NAME_Log,"Redo %s",Descr);

	// Find transaction corresponding to UndoTransCount.
	Lock(0);

	Transaction = &Transactions [0];
	for( int i=0; i<NumTrans; i++ )
	{
		if (Transaction->TransCount == UndoTransCount) goto Found;
		Transaction++;
	}
	debugf( NAME_Log, "Not found" );
	UndoTransCount++;
	Unlock(0);
	return 0;

	// Apply all "redo" changes corresponding to TRANS.UndoTransCount.
	Found:

	BeginChanges( Transaction );
	for( i=0; i<NumChanges; i++ )
	{
		Change = &ChangeLog[i];
		if( Change->TransCount == UndoTransCount )
			ApplyChange (Change,Buffer + Transaction->StartDataOffset + Change->DataOffset,Change->DataSize);
	}
	EndChanges( Transaction );
	UndoTransCount++;
	Unlock(0);
	return 1;
	unguard;
}

/*-----------------------------------------------------------------------------
	Logging individual transactions.
-----------------------------------------------------------------------------*/

//
// Notce a single record added, deleted, or changed during a transaction.  Index
// is the index of the record in the object Res, or -1 to note changes to stuff
// in the object.
//
void UTransBuffer::NoteSingleChange( UObject* Res, INT Index )
{
	guard(UTransBuffer::NoteSingleChange);

	UClass 			*Class 		 = Res->GetClass();
 	FTransaction	*Transaction = &Transactions [NumTrans-1];
	FTransChangeLog	*Changes;
	BYTE			*SourcePtr,*DestPtr;
	INT				DataSize;

	if( !GIsRunning || !Res || Overflow )
		return;

	// There are more changes on file from past transactions than the system
	// can hold, so delete the first one.
	Transaction->NumChanges++;
	while( NumChanges>=MaxChanges && NumTrans>0 )
	{
		DeleteFirstTransaction();
		Transaction = &Transactions[NumTrans-1];
	}

	if( NumTrans == 0 )
	{
		Overflow:
		debugf( NAME_Log, "Transaction overflow" );

		// This single transaction had more changes than the system can hold; end
		// transaction and mark it as "overflowed".
		if( NumChanges != 0 )
			appErrorf("NumChanges inconsistent");
		
		NumChanges = 0;
		Overflow   = 1;
		return;
	}

	// Add to change log.
	guard(1);
	Changes				= &ChangeLog[NumChanges++];
	Changes->TransCount	= TransCount - 1;
	Changes->Res		= Res;
	Changes->Index	 	= Index;
	Changes->DataOffset	= Transaction->TotalDataSize;
	unguardf(("%i/%i:%i/%i",NumChanges,MaxChanges,NumTrans-1,MaxTrans));

	if( Index >= 0 )
	{
		// Save a record of the object's data.
		DataSize  = Class->ClassRecordSize;
		SourcePtr = ((BYTE*)((UDatabase*)Res)->GetData()) + Index * Res->GetClass()->ClassRecordSize;
		//debugf( "Change %s %i", Res->GetFullName(), Index );
	}
	else
	{
		// Save object.
		DataSize  = Class->GetPropertiesSize();
		SourcePtr = (BYTE *)Res;
		//debugf( "Change %s", Res->GetFullName() );
	}
	Changes->DataSize = DataSize;

	// If there's not enough room in the data buffer, make room.
	guard(2);
	while( NumTrans>0 && (Transaction->StartDataOffset + Transaction->TotalDataSize + DataSize) > MaxDataOffset )
	{
		DeleteFirstTransaction();
		Transaction = &Transactions[NumTrans-1];
	}
	if( NumTrans == 0 )
		goto Overflow;
	unguardf(("%i/%i",NumChanges,MaxChanges));

	// Copy data.
	guard(3);
	DestPtr = Buffer + Transaction->StartDataOffset + Transaction->TotalDataSize;
	appMemcpy( DestPtr, SourcePtr, DataSize );
	unguard;

	// Update transaction's running data length.
	Transaction->TotalDataSize += DataSize;
	unguard;
}

/*-----------------------------------------------------------------------------
	Object modification.
-----------------------------------------------------------------------------*/

//
// Note that the object is about to change.
//
void UTransBuffer::NoteObject( UObject* Res )
{
	guard(UTransBuffer::NoteObject);
	
	if( GIsRunning && Res )
		NoteSingleChange( Res, -1 );
	
	unguard;
}

/*-----------------------------------------------------------------------------
	UTransBuffer object implementation.
-----------------------------------------------------------------------------*/

void UTransBuffer::Serialize( FArchive& Ar )
{
	guard(UTransBuffer::Serialize);

	UObject::Serialize( Ar );
	FTransChangeLog* Changes = (FTransChangeLog  *)&Data(MaxTrans * sizeof (FTransaction));
	for( int i=0; i<NumChanges; i++ )
		if( Changes[i].Index == -1 )
			Ar << Changes[i].Res;

	unguard;
}
IMPLEMENT_CLASS(UTransBuffer);

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
