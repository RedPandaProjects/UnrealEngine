/*===============================================================================
	UnCache.h: Unreal fast memory cache support.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney.
===============================================================================*/

/*-----------------------------------------------------------------------------
	FMemCache.
-----------------------------------------------------------------------------*/

//
// Memory cache.
//
class CORE_API FMemCache
{
public:
	//////////////////////
	// Public interface //
	//////////////////////

	// Information about a cache item.
	class CORE_API FCacheItem
	{
	public:
		friend class FMemCache;
		void Unlock()
		{
#if CHECK_ALL
			if( this == NULL )
				appError( "Unlock: Null cache item" );
			if( Cost < COST_INFINITE )
				appErrorf( "Unlock: Item %08X.%08X is not locked", (DWORD)(Id>>32), (DWORD)Id );
#endif
			Cost -= COST_INFINITE;
		}
		QWORD GetId()
		{
			return Id;
		}
		BYTE* GetData()
		{
			return Data;
		}
		INT GetSize()
		{
			debug(LinearNext);
			return LinearNext->Data - Data;
		}
		BYTE GetExtra()
		{
			return Extra;
		}
		void SetExtra( BYTE B )
		{
			Extra = B;
		}
		typedef _WORD TCacheTime;
	private:
		// Private variables: Note 32-byte alignment.
		QWORD		Id;				// This item's cache id, 0=unused.
		BYTE*		Data;			// Pointer to the item's data.
		TCacheTime	Time;			// Last Get() time.
		BYTE		Segment;		// Number of the segment this item resides in.
		BYTE		Extra;			// Extra space for use.
		INT			Cost;			// Cost to flush this item.
		FCacheItem*	LinearNext;		// Next cache item in linear list, or NULL if last.
		FCacheItem*	LinearPrev;		// Previous cache item in linear list, or NULL if first.
		FCacheItem*	HashNext;		// Next cache item in hash table, or NULL if last.
	};

	// FMemCache interface.
	FMemCache() {Initialized=0;}
    void Init( INT BytesToAllocate, INT MaxItems, void* Start=NULL, INT SegSize=0 );
	void Exit( INT FreeMemory );
	void Flush( QWORD Id=0, DWORD Mask=~0, UBOOL IgnoreLocked=0 );
	BYTE* Create( QWORD Id, FCacheItem *&Item, INT CreateSize, INT Alignment=DEFAULT_ALIGNMENT, INT SafetyPad=0 );
	void Tick();
	void CheckState();
	UBOOL Exec( const char* Cmd, FOutputDevice* Out=GSystem );
	void Status( char* Msg );
	INT GetTime() {return Time;}

	// FMemCache inlines.
#if ASM
#pragma warning( disable : 4035 )
	DWORD GHash( DWORD Val )
	{
		return (Val ^ (Val>>12) ^ (Val>>24)) & (HASH_COUNT-1);
	}
	BYTE* Get( QWORD Id, FCacheItem*& Item, INT Alignment=DEFAULT_ALIGNMENT )
	{	
		guardSlow(FMemCache::Get);
		clockSlow(GetCycles);
		NumGets++;
		if( Id==MruId )
		{
			Item = MruItem;
			MruItem->Cost += COST_INFINITE;
			return Align( MruItem->Data, Alignment );
		}
		for( FCacheItem* HashItem=HashItems[GHash(Id)]; HashItem; HashItem=HashItem->HashNext )
		{
			if( HashItem->Id == Id )
			{
				// Set the item, lock it, and return its data.
				MruId			= Id;
				MruItem			= HashItem;
				Item            = HashItem;
				HashItem->Time  = Time;
				HashItem->Cost += COST_INFINITE;
				unclockSlow(GetCycles);
				return Align( HashItem->Data, Alignment );
			}
		}
		unclockSlow(GetCycles);
		return NULL;
		unguardSlow;
	}
#pragma warning( default : 4035 )
#else
	DWORD GHash( DWORD Val )
	{
		return (Val ^ (Val>>12) ^ (Val>>24)) & (HASH_COUNT-1);
	}
	BYTE* Get( QWORD Id, FCacheItem*& Item, INT Alignment=DEFAULT_ALIGNMENT )
	{	
		guardSlow(FMemCache::Get);
		clockSlow(GetCycles);
		NumGets++;
		for( FCacheItem* HashItem=HashItems[GHash(Id)]; HashItem; HashItem=HashItem->HashNext )
		{
			if( HashItem->Id == Id )
			{
				// Set the item, lock it, and return its data.
				Item            = HashItem;
				HashItem->Time  = Time;
				HashItem->Cost += COST_INFINITE;
				unclockSlow(GetCycles);
				return Align( HashItem->Data, Alignment );
			}
		}
		unclockSlow(GetCycles);
		return NULL;
		unguardSlow;
	}
#endif

private:
	// Constants.
	enum {COST_INFINITE=0x1000000};
	enum {HASH_COUNT=16384};
	enum {IGNORE_SIZE=256};

	// Variables.
	INT Initialized;
	INT Time;
	QWORD MruId;
	FCacheItem* MruItem;

	// Stats.
	INT NumGets,NumCreates,CreateCycles,GetCycles,TickCycles;
	INT ItemsFresh,ItemsStale,ItemsTotal,ItemGaps;
	INT MemFresh,MemStale,MemTotal;

	// Linked list of item associated with cache memory, linked via LinearNext and
	// LinearPrev order of memory.
	void*       ItemMemory;
	FCacheItem* CacheItems;
	FCacheItem* LastItem;
	void CreateNewFreeSpace( BYTE* Start, BYTE* End, FCacheItem* Prev, FCacheItem* Next, INT Segment );

	// First item in unused item list (these items are not associated with cache
	// memory). Linked via LinearNext in FIFO order.
	FCacheItem* UnusedItems;

	// The hash table.
	FCacheItem* HashItems[HASH_COUNT];
	void Unhash( QWORD Id )
	{
		for( FCacheItem** PrevLink=&HashItems[GHash(Id)]; *PrevLink; PrevLink=&(*PrevLink)->HashNext )
		{
			if( (*PrevLink)->Id == Id )
			{
				*PrevLink = (*PrevLink)->HashNext;
				return;
			}
		}
		appErrorf( "%s", "Unhashed item" );
	}

	// Merging items.
	FCacheItem* MergeWithNext( FCacheItem* First );

	// Flushing individual items.
	FCacheItem* FlushItem( FCacheItem* Item, UBOOL IgnoreLocked=0 );

	// Original memory allocations.
	FCacheItem *UnusedItemMemory;
	BYTE       *CacheMemory;

	// State checking.
	void ConditionalCheckState()
	{
#if CHECK_ALL || defined(_DEBUG)
		CheckState();
#endif
	}
	friend class FCacheItem;
};

// Global scope.
typedef FMemCache::FCacheItem FCacheItem;

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
