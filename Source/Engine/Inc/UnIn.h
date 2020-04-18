/*=============================================================================
	UnIn.h: Unreal input system.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Description:
	This subsystem contains all input.  The input is generated by the
	platform-specific code, and processed by the actor code.

Revision history:
	* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Enums.
-----------------------------------------------------------------------------*/

//
// Maximum aliases.
//
enum {ALIAS_MAX=40};

/*-----------------------------------------------------------------------------
	UInput.
-----------------------------------------------------------------------------*/

//
// An input alias.
//
struct FAlias
{
	FName Alias;
	char Command[64];
};

//
// The input system base class.
//
class ENGINE_API UInput : public USubsystem
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UInput,USubsystem,CLASS_Transient|CLASS_Config)

	// Variables.
	FAlias		Aliases[40];
	FString		Bindings[IK_MAX];
	UViewport*	Viewport;

	// Constructor.
	UInput();

	// UObject interface.
	static void InternalClassInitializer( UClass* Class );
	void Serialize( FArchive& Ar );

	// UInput interface.
	virtual void Init( UViewport* Viewport, FGlobalPlatform* App );
	virtual UBOOL Exec( const char *Cmd, FOutputDevice *Out );
	virtual UBOOL Process( FOutputDevice &Out, EInputKey iKey, EInputAction State, FLOAT Delta=0.0 );
	virtual void ReadInput( FLOAT DeltaSeconds, FOutputDevice* Out );
	virtual void ResetInput();
	virtual const char* GetKeyName( EInputKey Key ) const;
	virtual int FindKeyName( const char* KeyName, EInputKey& iKey ) const;

	// Accessors.
	void SetInputAction( EInputAction NewAction, FLOAT NewDelta=0.0 )
		{Action = NewAction; Delta = NewDelta;}
	EInputAction GetInputAction()
		{return Action;}
	FLOAT GetInputDelta()
		{return Delta;}
	BYTE KeyDown( int i )
		{return KeyDownTable[Clamp(i,0,IK_MAX-1)];}

protected:
	UEnum* InputKeys;
	EInputAction Action;
	FLOAT Delta;
	BYTE KeyDownTable[IK_MAX];
	virtual BYTE* FindButtonName( AActor* Actor, const char* ButtonName ) const;
	virtual FLOAT* FindAxisName( AActor* Actor, const char* ButtonName ) const;
	virtual void ExecInputCommands( const char* Cmd, FOutputDevice* Out );
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/