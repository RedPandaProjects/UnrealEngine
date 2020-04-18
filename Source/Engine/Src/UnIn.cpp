/*=============================================================================
	UnIn.cpp: Unreal input system.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Revision history:
	* Created by Tim Sweeney
=============================================================================*/

#include "EnginePrivate.h"

/*-----------------------------------------------------------------------------
	Internal.
-----------------------------------------------------------------------------*/

class FInputVarCache
{
public:
	INT Count;
	UProperty* Properties[];
	static FInputVarCache* Get( UClass* Class, FCacheItem*& Item )
	{
		QWORD CacheId = MakeCacheID( CID_Extra3, Class );
		FInputVarCache* Result = (FInputVarCache*)GCache.Get(CacheId,Item);
		if( !Result )
		{
			INT Count=0, Temp=0;
			for( TFieldIterator<UProperty> It(Class); It; ++It )
				if( It->PropertyFlags & CPF_Input )
					Count++;
			Result = (FInputVarCache*)GCache.Create(CacheId,Item,sizeof(FInputVarCache)+Count*sizeof(UProperty*));
			Result->Count = Count;
			for( It=TFieldIterator<UProperty>(Class); It; ++It )
				if( It->PropertyFlags & CPF_Input )
					Result->Properties[Temp++] = *It;
		}
		return Result;
	}
};

/*-----------------------------------------------------------------------------
	Implementation.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(UInput);

/*-----------------------------------------------------------------------------
	UInput creation and destruction.
-----------------------------------------------------------------------------*/

//
// Constructor.
//
UInput::UInput()
{
	guard(UInput::UInput);
	static UBOOL Registered=0;
	InputKeys = FindObjectChecked<UEnum>( AActor::StaticClass, "EInputKey" );
	if( !Registered )
	{
		UClass* Class=GetClass();

		// Create input alias struct.
		UStruct* AliasStruct = new(Class,"Alias")UStruct( NULL );
		AliasStruct->SetPropertiesSize( sizeof(FName) + 64*sizeof(char));
		new(AliasStruct,"Alias",  RF_Public)UNameProperty  ( EC_CppProperty, 0,             "", CPF_Config );
		new(AliasStruct,"Command",RF_Public)UStringProperty( EC_CppProperty, sizeof(FName), "", CPF_Config, 64 );

		// Add alias list.
		(new(Class,"Aliases",RF_Public)UStructProperty( CPP_PROPERTY(Aliases), "Aliases", CPF_Config, AliasStruct ))->ArrayDim = ALIAS_MAX;

		// Add key list.
		UStruct* DynamicStringStruct=FindObject<UStruct>(ANY_PACKAGE,"DynamicString");
		for( INT i=0; i<IK_MAX; i++ )
			if( *GetKeyName((EInputKey)i) )
				new(Class,GetKeyName((EInputKey)i),RF_Public)UStructProperty( EC_CppProperty, (INT)&((UInput*)NULL)->Bindings[i], "RawKeys", CPF_Config, DynamicStringStruct );

		// Load config.
		Class->GetDefaultObject()->LoadConfig( NAME_Config );

		Registered=1;
	}
	unguard;
}

//
// Class initializer.
//
void UInput::InternalClassInitializer( UClass* Class )
{
	guard(UInput::InternalClassInitializer);
	if( appStricmp(Class->GetName(),"Input")==0 )
	{
	}
	unguard;
}

//
// Serialize
//
void UInput::Serialize( FArchive& Ar )
{
	guard(UInput::Serialize);
	Super::Serialize( Ar );
	Ar << InputKeys;
	unguard;
}

//
// Find a button.
//
BYTE* UInput::FindButtonName( AActor* Actor, const char* ButtonName ) const
{
	guard(UInput::FindButtonName);
	check(Viewport);
	check(Actor);

	// Map ButtonName to a FName.
	FName Button( ButtonName, FNAME_Find );

	// Find property.
	if( Button != NAME_None )
	{
		FCacheItem* Item;
		FInputVarCache* Cache = FInputVarCache::Get( Actor->GetClass(), Item );
		for( INT i=0; i<Cache->Count; i++ )
			if
			(	Cache->Properties[i]->GetFName()==Button
			&&	Cast<UByteProperty>(Cache->Properties[i]) )
				break;
		Item->Unlock();
		if( i<Cache->Count )
			return (BYTE*)Actor + Cache->Properties[i]->Offset;
	}

	// Not found.
	return NULL;
	unguard;
}

//
// Find an axis.
//
FLOAT* UInput::FindAxisName( AActor* Actor, const char* ButtonName ) const
{
	guard(UInput::FindAxisName);
	check(Viewport);
	check(Actor);

	// Map ButtonName to a FName.
	FName Button( ButtonName, FNAME_Find );

	// Find property.
	if( Button != NAME_None )
		for( TFieldIterator<UFloatProperty> It(Actor->GetClass()); It; ++It )
			if( It->GetFName()==Button )
				return (FLOAT*)((BYTE*)Actor + It->Offset);

	// Not found.
	return NULL;
	unguard;
}

//
// Execute input commands.
//
void UInput::ExecInputCommands( const char* Cmd, FOutputDevice* Out )
{
	guard(UInput::ExecInputCommands);
	char Line[256];
	while( ParseLine( &Cmd, Line, ARRAY_COUNT(Line)) )
	{
		const char* Str = Line;
		if( Action==IST_Press || (Action==IST_Release && ParseCommand(&Str,"OnRelease")) )
			Viewport->Exec( Str, Out );
		else
			Exec( Str, Out );
	}
	unguard;
}

//
// Init.
//
void UInput::Init( UViewport* InViewport, FGlobalPlatform* InApp )
{
	guard(UInput::Init);

	// Set objects.
	GSystem  = InApp;
	Viewport = InViewport;

	// Reset.
	ResetInput();

	debugf( NAME_Init, "Input system initialized for %s", Viewport->GetName() );
	unguard;
}

/*-----------------------------------------------------------------------------
	Command line.
-----------------------------------------------------------------------------*/

//
// Execute a command.
//
UBOOL UInput::Exec( const char* Str, FOutputDevice* Out )
{
	guard(UInput::Exec);
	char Temp[256];
	static UBOOL InAlias=0;

	if( ParseCommand( &Str, "BUTTON" ) )
	{
		// Normal button.
		BYTE* Button;
		if
		(	Viewport->Actor
		&&	ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 )
		&&	(Button=FindButtonName(Viewport->Actor,Temp))!=NULL )
		{
			if( GetInputAction() == IST_Press )
				*Button += 1;
			else if( GetInputAction()==IST_Release && *Button )
				*Button -= 1;
		}
		else Out->Log( "Bad Button command" );
		return 1;
	}
	else if( ParseCommand( &Str, "PULSE" ) )
	{
		// Normal button.
		BYTE* Button;
		if
		(	Viewport->Actor
		&&	ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 )
		&&	(Button=FindButtonName(Viewport->Actor,Temp))!=NULL )
		{
			if( GetInputAction() == IST_Press )
				*Button = 1;
		}
		else Out->Log( "Bad Button command" );
		return 1;
	}
	else if( ParseCommand( &Str, "TOGGLE" ) )
	{
		// Toggle button.
		BYTE* Button;
		if
		(	Viewport->Actor
		&&	ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 )
		&&	((Button=FindButtonName(Viewport->Actor,Temp))!=NULL) )
		{
			if( GetInputAction() == IST_Press )
				*Button ^= 0x80;
		}
		else Out->Log( "Bad Toggle command" );
		return 1;
	}
	else if( ParseCommand( &Str, "AXIS" ) )
	{
		// Axis movement.
		FLOAT* Axis;
		if
		(	Viewport->Actor
		&&	ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 )
		&&	(Axis=FindAxisName(Viewport->Actor,Temp))!=NULL )
		{
			FLOAT Speed=1.0;
			Parse( Str, "SPEED=", Speed );
			if( GetInputAction() == IST_Axis )
			{
				*Axis += 0.01 * GetInputDelta() * Speed;
			}
			else if( GetInputAction() == IST_Hold )
			{
				*Axis += GetInputDelta() * Speed;
			}
		}
		else Out->Logf( "Bad Axis command" );
		return 1;
	}
	else if( ParseCommand( &Str, "KEYNAME" ) )
	{
		INT keyNo = appAtoi(Str);
		appStrcpy( Temp, GetKeyName(EInputKey(keyNo)));
		Out->Log( Temp );
		return 1;
	}
	else if( ParseCommand( &Str, "KEYBINDING" ) )
	{
		EInputKey iKey;
		if( FindKeyName(Str,iKey) )
		{
			appStrcpy( Temp, *Bindings[iKey] );
			Out->Log( Temp );
		}
		return 1;
	}
	else if( !InAlias && ParseToken( Str, Temp, ARRAY_COUNT(Temp), 0 ) )
	{
		FName Name(Temp,FNAME_Find);
		if( Name!=NAME_None )
		{
			for( INT i=0; i<ARRAY_COUNT(Aliases); i++ )
			{
				if( Aliases[i].Alias==Name )
				{
					guard(ExecAlias);
					InAlias=1;
					ExecInputCommands( Aliases[i].Command, Out );
					InAlias=0;
					unguard;
					return 1;
				}
			}
		}
	}
	return 0;
	unguard;
}

/*-----------------------------------------------------------------------------
	Key and axis movement processing.
-----------------------------------------------------------------------------*/

//
// Process input. Returns 1 if handled, 0 if not.
//
UBOOL UInput::Process( FOutputDevice& Out, EInputKey iKey, EInputAction State, FLOAT Delta )
{
	guard(UInput::Process);
	check(iKey>=0&&iKey<IK_MAX);

	// Update key flags.
	switch( State )
	{
		case IST_Press:
			if( KeyDownTable[iKey] )
				return 0;
			KeyDownTable[iKey] = 1;
			break;
		case IST_Release:
			if( !KeyDownTable[iKey] )
				return 0;
			KeyDownTable[iKey] = 0;
			break;
	}

	// Make sure there is a binding.
	if( Bindings[iKey].Length()==0 )
		return 0;

	// Process each line of the binding string.
	SetInputAction( State, Delta );
	ExecInputCommands( *Bindings[iKey], &Out );
	SetInputAction( IST_None );

	return 1;
	unguard;
}

/*-----------------------------------------------------------------------------
	Input reading.
-----------------------------------------------------------------------------*/

//
// Read input for the viewport.
//
void UInput::ReadInput( FLOAT DeltaSeconds, FOutputDevice* Out )
{
	guard(UInput::ReadInput);
	FCacheItem* Item;
	FInputVarCache* Cache = FInputVarCache::Get( Viewport->Actor->GetClass(), Item );

	// Update everything with IST_Hold.
	if( DeltaSeconds != 0.0 )
		for( INT i=0; i<IK_MAX; i++ )
			if( KeyDownTable[i] )
				Process( *GSystem, (EInputKey)i, IST_Hold, DeltaSeconds );

	// Scale the axes.
	FLOAT Scale = DeltaSeconds!=0.0 ? 20.0/DeltaSeconds : 0.0;
	for( INT i=0; i<Cache->Count; i++ )
		if( Cast<UFloatProperty>(Cache->Properties[i]) )
			*(FLOAT*)((BYTE*)Viewport->Actor + Cache->Properties[i]->Offset) *= Scale;

	Item->Unlock();
	unguard;
}

/*-----------------------------------------------------------------------------
	Input resetting.
-----------------------------------------------------------------------------*/

//
// Reset the input system's state.
//
void UInput::ResetInput()
{
	guard(UInput::ResetInput);
	check(Viewport);

	// Reset all keys.
	for( INT i=0; i<IK_MAX; i++ )
		KeyDownTable[i] = 0;

	// Reset all input bytes.
	for( TFieldIterator<UByteProperty> ItB(Viewport->Actor->GetClass()); ItB; ++ItB )
		if( ItB->PropertyFlags & CPF_Input )
			*(BYTE *)((BYTE*)Viewport->Actor + ItB->Offset) = 0;

	// Reset all input floats.
	for( TFieldIterator<UFloatProperty> ItF(Viewport->Actor->GetClass()); ItF; ++ItF )
		if( ItF->PropertyFlags & CPF_Input )
			*(FLOAT *)((BYTE*)Viewport->Actor + ItF->Offset) = 0;

	// Set the state.
	SetInputAction( IST_None );

	// Reset viewport input.
	Viewport->UpdateInput( 1 );

	unguard;
}

/*-----------------------------------------------------------------------------
	Utility functions.
-----------------------------------------------------------------------------*/

//
// Return the name of a key.
//
const char* UInput::GetKeyName( EInputKey Key ) const
{
	guard(UInput::GetKeyName);
	if( Key>=0 && Key<IK_MAX )
		if( appStrlen(*InputKeys->Names(Key)) > 3 )
			return *InputKeys->Names(Key)+3;
	return "";
	unguard;
}

//
// Find the index of a named key.
//
UBOOL UInput::FindKeyName( const char* KeyName, EInputKey& iKey ) const
{
	guard(UInput::FindKeyName);
	char Temp[256];
	appSprintf( Temp, "IK_%s", KeyName );
	FName N( Temp, FNAME_Find );
	if( N != NAME_None )
		return InputKeys->Names.FindItem( N, *(INT*)&iKey );
	return 0;
	unguard;
}

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
