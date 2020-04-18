/*=============================================================================
	UnBunch.h: Unreal bunch (sub-packet) functions.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"
#include "UnNet.h"

/*-----------------------------------------------------------------------------
	FBunch implementation.
-----------------------------------------------------------------------------*/

static void** PoolFirst=NULL;
ENGINE_API void FreePooledBunch( void* Bunch )
{
	guard(FBunch::FreePooledBunch);
	*(void**)Bunch = PoolFirst;
	PoolFirst = (void**)Bunch;
	unguard;
}
ENGINE_API void* AllocPooledBunch()
{
	guard(FBunch::AllocPooledBunch);
	if( PoolFirst )
	{
		void* Result = PoolFirst;
		PoolFirst = *(void***)PoolFirst;
		return Result;
	}
	else
	{
		return appMalloc( Max(sizeof(FInBunch),sizeof(FOutBunch)), "PooledBunch" );
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	Name exchange.
-----------------------------------------------------------------------------*/

//
// Write a name.
//
FArchive& FOutBunch::operator<<( class FName& N )
{
	guard(FOutBunch<<FName);

	// Map to indices.
	INT Index = Channel->Connection->Driver->Map.NameToIndex( N );
	*this << AR_INDEX( Index );

	return *this;
	unguard;
}

//
// Read a name.
//
FArchive& FInBunch::operator<<( class FName& N )
{
	guard(FInBunch<<FName);

	// Map to indices.
	INT Index;
	*this << AR_INDEX( Index );
	N = Overflowed ? NAME_None : Connection->Driver->Map.IndexToName( Index );

	return *this;
	unguard;
}

/*-----------------------------------------------------------------------------
	Object exchange.
-----------------------------------------------------------------------------*/

//
// Write an object.
//
UBOOL FOutBunch::SendObject( UObject* Object )
{
	guard(FOutBunch::SendObject);
	AActor* Actor = Cast<AActor>(Object);
	UBOOL Result = 1;
	INT   Index  = -1;
	if( Actor && !Actor->bStatic && !Actor->bNoDelete )
	{
		// Map dynamic actor through channel index.
		FActorChannel* Ch = Channel->Connection->GetActorChannel( (AActor*)Object );
		if( Ch && Ch->State==UCHAN_Open )
		{
			Index = -Ch->ChIndex-2;
		}
		else
		{
			// Actor has no channel, so don't update defaults.
			Result = 0;
		}
	}
	else if( Object )
	{
		// Map regular object.
		Index  = Channel->Connection->Driver->Map.ObjectToIndex( Object );
		Result = !Object || Index!=-1;
	}
	*this << AR_INDEX(Index);
	return Result;
	unguard;
}

//
// Write an object.
//
FArchive& FOutBunch::operator<<( UObject*& Object )
{
	guard(FOutBunch<<UObject);
	SendObject( Object );
	return *this;
	unguard;
}

//
// Read an object.
//
FArchive& FInBunch::operator<<( UObject*& Object )
{
	guard(FInBunch<<UObject);
	INT Index;
	*this << AR_INDEX(Index);
	if( !Overflowed )
	{
		if( Index >= -1 )
		{
			// Map to normal object.
			Object = Connection->Driver->Map.IndexToObject( Index, 1 );
		}
		else
		{
			// Map to an actor channel index.
			Index = -Index-2;
			if
			(	Index<UNetConnection::MAX_CHANNELS
			&&	Connection->Channels[Index]
			&&	Connection->Channels[Index]->ChType==CHTYPE_Actor 
			&&	Connection->Channels[Index]->State==UCHAN_Open )
			{
				//debugf("Read actor %s",Res?Res->GetName():"null");
				Object = ((FActorChannel*)Connection->Channels[Index])->Actor;
			}
			else
			{
				//debugf("Read null actor");
				Object = NULL;
			}
		}
	}
	return *this;
	unguard;
}

/*-----------------------------------------------------------------------------
	FInBunch implementation.
-----------------------------------------------------------------------------*/

//
// Receive a property.
//
UBOOL FInBunch::ReceiveProperty( UProperty* Property, BYTE* InData, BYTE* Recent )
{
	guard(FInBunch::ReceiveProperty);

	// Receive array dimension.
	BYTE Element=0;
	if( Property->ArrayDim != 1 )
		*this << Element;

	// Receive property data.
	INT Offset = Property->Offset + Element*Property->GetElementSize();
	BYTE* Data = InData + Offset;
	if( Property->GetClass()==UByteProperty::StaticClass )
	{
		guard(Byte);
		*this << *(BYTE*)Data;
		unguard;
	}
	else if( Property->GetClass()==UIntProperty::StaticClass )
	{
		guard(Int);
		*this << *(INT*)Data;
		unguard;
	}
	else if( Property->GetClass()==UBoolProperty::StaticClass )
	{
		guard(Bool);
		BYTE BoolValue;
		*this << BoolValue;
		if( BoolValue )
			*(DWORD*)Data |= CastChecked<UBoolProperty>(Property)->BitMask;
		else
			*(DWORD*)Data &= ~CastChecked<UBoolProperty>(Property)->BitMask;
		unguard;
	}
	else if( Property->GetClass()==UFloatProperty::StaticClass )
	{
		guard(Float);
		*this << *(FLOAT*)Data;
		unguard;
	}
	else if( Property->IsA(UObjectProperty::StaticClass) )
	{
		guard(Object);
		UObject* Obj;
		*this << Obj;
		*(UObject**)Data = NULL;
		UClass* Class = ((UObjectProperty*)Property)->PropertyClass;
		if( Obj!=NULL )
		{
			if( !Obj->IsA(Class) )
			{
				debugf( NAME_DevNet, "Property %s: Got %s, Expecting %s", Property->GetFullName(), Obj->GetFullName(), Class->GetName() );
			}
			else if( Obj->IsA(AActor::StaticClass) && ((AActor*)Obj)->bDeleteMe )
			{
				debugf( NAME_DevNet, "Property %s: Received deleted actor %s", Property->GetFullName(), Obj->GetFullName() );
			}
			else
			{
				*(UObject**)Data = Obj;
			}
		}
		unguard;
	}
	else if( Property->GetClass()==UNameProperty::StaticClass )
	{
		guard(Name);
		*this << *(FName*)Data;
		unguard;
	}
	else if( Property->GetClass()==UStringProperty::StaticClass )
	{
		guard(String);
		UStringProperty* StringProp = CastChecked<UStringProperty>(Property);
		String( (char*)Data, StringProp->StringSize );
		unguard;
	}
	else if( Property->GetClass()==UStructProperty::StaticClass )
	{
		guard(Struct);
		UStructProperty* StructProperty = CastChecked<UStructProperty>( Property );
		if( StructProperty->Struct->GetFName()==NAME_Vector )
		{
			SWORD X,Y,Z;
			*this << X << Y << Z;
			((FVector*)Data)->X = X;
			((FVector*)Data)->Y = Y;
			((FVector*)Data)->Z = Z;
		}
		else if( StructProperty->Struct->GetFName()==NAME_Rotator )
		{
			BYTE Pitch, Yaw, Roll;
			*this << Pitch << Yaw << Roll;
			((FRotator*)Data)->Pitch = Pitch << 8;
			((FRotator*)Data)->Yaw   = Yaw   << 8;
			((FRotator*)Data)->Roll  = Roll  << 8;
		}
		else if( StructProperty->Struct->GetFName()==NAME_Plane )
		{
			SWORD X,Y,Z,W;
			*this << X << Y << Z << W;
			((FPlane*)Data)->X = X;
			((FPlane*)Data)->Y = Y;
			((FPlane*)Data)->Z = Z;
			((FPlane*)Data)->W = W;
		}
		else
		{
			//caveat: Doesn't support general struct serializing.
		}
		unguard;
	}

	// Copy the property back to recent version.
	guard(UpdateRecent);
	if( Recent )
		appMemcpy( Recent + Offset, Data, Property->GetElementSize() );
	unguard;

	// Success.
	return 1;
	unguardf(( "(%s)", Property->GetName() ));
}

/*-----------------------------------------------------------------------------
	FOutBunch implementation.
-----------------------------------------------------------------------------*/

//
// Construct an outgoing bunch for a channel.
// It is ok to either send or discard an FOutbunch after construction.
//
FOutBunch::FOutBunch( FChannel* InChannel, UBOOL bClose )
:	Channel		( InChannel )
,	Overflowed	( 0 )
{
	guard(FOutBunch::FOutBunch);
	check(Channel->State==UCHAN_Open);
	check(Channel->Connection->Channels[Channel->ChIndex]==Channel);
	ArIsNet = 1;

	// Set channel info.
	Header.ChIndex      = Channel->ChIndex;
	Header._ChType      = Channel->ChType;
	Header.DataSize     = 0;
	MaxDataSize         = sizeof(Data);

	// Reserve channel and set bunch info.
	if( Channel->ReserveOutgoingIndex(bClose)==INDEX_NONE )
	{
		Overflowed = 1;
		return;
	}
	Header.Sequence     = Channel->Connection->OutSequence[Channel->ChIndex];
	Header.PrevSequence = Channel->Connection->OutReliable[Channel->ChIndex];

	unguard;
}

//
// Byte stream serializer.
//
FArchive& FOutBunch::Serialize( void* V, INT Length )
{
	guard(FOutBunch::Serialize);	
	if( Header.DataSize+Length<=MaxDataSize && !Overflowed )
	{
		appMemcpy( &Data[Header.DataSize], V, Length );
		Header.DataSize += Length;
	}
	else Overflowed = 1;
	return *this;		
	unguard;
}

//
// Send a property.
//
UBOOL FOutBunch::SendProperty( UProperty* Property, INT ArrayIndex, BYTE* InData, BYTE* Defaults, UBOOL Named )
{
	guard(FOutBunch::SendProperty);
	INT SavedSize       = Header.DataSize;
	INT SavedOverflowed = Overflowed;

	// Setup.
	INT   Offset  = Property->Offset + ArrayIndex*Property->GetElementSize();
	BYTE* Data    = InData + Offset;

	// Send property name and optional array index.
	if( Named )
	{
		FName PropertyName = Property->GetFName();
		*this << PropertyName;
		if( Property->ArrayDim != 1 )
		{
			BYTE Element = ArrayIndex;
			*this << Element;
		}
	}

	// Send the value.
	if( Property->GetClass()==UByteProperty::StaticClass )
	{
		guard(Byte);
		*this << *(BYTE*)Data;
		unguard;
	}
	else if( Property->GetClass()==UIntProperty::StaticClass )
	{
		guard(Int);
		*this << *(INT*)Data;
		unguard;
	}
	else if( Property->GetClass()==UBoolProperty::StaticClass )
	{
		guard(Bool);
		BYTE BoolValue = (*(DWORD*)Data & CastChecked<UBoolProperty>(Property)->BitMask) ? 1 : 0;
		*this << BoolValue;
		unguard;
	}
	else if( Property->GetClass()==UFloatProperty::StaticClass )
	{
		guard(Float);
		*this << *(FLOAT*)Data;
		unguard;
	}
	else if( Property->IsA(UObjectProperty::StaticClass) )
	{
		guard(Object);
		if( !SendObject( *(UObject**)Data ) )
			Defaults = NULL;
		unguard;
	}
	else if( Property->GetClass()==UNameProperty::StaticClass )
	{
		guard(Name);
		*this << *(FName*)Data;
		unguard;
	}
	else if( Property->GetClass()==UStringProperty::StaticClass )
	{
		guard(String);
		UStringProperty* StringProp = CastChecked<UStringProperty>(Property);
		String( (char*)Data, StringProp->StringSize );
		unguard;
	}
	else if( Property->GetClass()==UStructProperty::StaticClass )
	{
		guard(Struct);
		UStructProperty* StructProperty = CastChecked<UStructProperty>( Property );
		if( StructProperty->Struct->GetFName()==NAME_Vector )
		{
			SWORD X = ((FVector*)Data)->X;
			SWORD Y = ((FVector*)Data)->Y;
			SWORD Z = ((FVector*)Data)->Z;
			*this << X << Y << Z;
		}
		else if( StructProperty->Struct->GetFName()==NAME_Rotator )
		{
			BYTE Pitch = ((FRotator*)Data)->Pitch >> 8;
			BYTE Yaw   = ((FRotator*)Data)->Yaw   >> 8;
			BYTE Roll  = ((FRotator*)Data)->Roll  >> 8;
			*this << Pitch << Yaw << Roll;
		}
		else if( StructProperty->Struct->GetFName()==NAME_Plane )
		{
			SWORD X = ((FPlane*)Data)->X;
			SWORD Y = ((FPlane*)Data)->Y;
			SWORD Z = ((FPlane*)Data)->Z;
			SWORD W = ((FPlane*)Data)->W;
			*this << X << Y << Z << W;
		}
		else
		{
			//caveat: Doesn't support general struct replication.
		}
		unguard;
	}
	if( Overflowed )
	{
		// Rollback the changes because we overflowed.
		guard(Overflowed);
		Header.DataSize = SavedSize;
		Overflowed      = SavedOverflowed;
		return 1;
		unguard;
	}
	else
	{
		// Update the defaults.
		guard(Defaults);
		if( Defaults )
		{
			UBoolProperty* Bool = Cast<UBoolProperty>( Property );
			if( !Bool )
				appMemcpy( Defaults + Offset, InData + Offset, Property->GetElementSize() );
			else
				*(DWORD*)(Defaults + Offset ) ^= Bool->BitMask;
		}
		unguard;
		return 0;
	}
	unguardf(( "(%s)", Property->GetName() ));
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
