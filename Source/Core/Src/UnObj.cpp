/*=============================================================================
	UnObj.cpp: Unreal object manager.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "CorePrivate.h" 

/*-----------------------------------------------------------------------------
	Globals.
-----------------------------------------------------------------------------*/

// Print debugging info.
#include "UnLinker.h"

/*-----------------------------------------------------------------------------
	Statics.
-----------------------------------------------------------------------------*/

// Static FObjectManager variables.
UBOOL				FObjectManager::Initialized		 = 0;
INT					FObjectManager::BeginLoadCount   = 0;
UObject*			FObjectManager::AutoRegister     = NULL;
UPackage*			FObjectManager::TransientPackage = NULL;
UObject*			FObjectManager::ObjHash[4096];
TArray<UObject*>    FObjectManager::Objects;
TArray<INT>         FObjectManager::Available;
TArray<UObject*>	FObjectManager::Loaders;
TArray<UObject*>	FObjectManager::Root;

// For development.
UBOOL GNoGC=0;
UBOOL GCheckConflicts=0;

/*-----------------------------------------------------------------------------
	UObject constructors.
-----------------------------------------------------------------------------*/

UObject::UObject()
:   MainFrame   ( NULL )
{}
UObject::UObject( EIntrinsicConstructor, UClass* InClass, FName InName, FName InPackageName )
:	Name        ( InName		)
,	Parent      ( NULL			)
,	Class		( InClass		)
,	ObjectFlags	( 0				)
,	LinkerIndex	( INDEX_NONE	)
,	Linker		( NULL			)
,	Index		( INDEX_NONE	)
,	HashNext	( NULL			)
,	MainFrame	( NULL	        )
{
	guard(UObject::UObject);

	// Register.
	GObj.PreRegister( this, InPackageName );

	unguard;
}

/*-----------------------------------------------------------------------------
	UObject class initializer.
-----------------------------------------------------------------------------*/

void UObject::InternalClassInitializer( UClass* Class )
{
	guard(UObject::InternalClassInitializer);
	unguard;
}

/*-----------------------------------------------------------------------------
	UObject implementation.
-----------------------------------------------------------------------------*/

//
// Rename this object to a unique name.
//
void UObject::Rename()
{
	guard(UObject::Rename);

	FName NewName = GObj.MakeUniqueObjectName( GetParent(), GetClass() );
	GObj.UnhashObject( this );
	//debugf( "Renaming %s to %s",*Name,*NewName );
	Name = NewName;
	GObj.HashObject( this );

	unguardobj;
}

//
// Shutdown after a critical error.
//
void UObject::ShutdownAfterError()
{}

//
// Make sure the object is valid.
//
UBOOL UObject::IsValid()
{
	guard(UObject::IsValid);
	if( !this )
	{
		debugf( NAME_Warning, "NULL object" );
		return 0;
	}
	else if( !GObj.Objects.IsValidIndex(GetIndex()) )
	{
		debugf( NAME_Warning, "Invalid object index %i", GetIndex() );
		return 0;
	}
	else if( GObj.Objects(GetIndex())!=this )
	{
		debugf( NAME_Warning, "Unlisted object" );
		return 0;
	}
	else return 1;
	unguardobj;
}

//
// Export an object to a buffer.  This must be overridden by
// all objects which are capable of exporting.  The type
// of data to be exported (text, binary, file format, etc) may
// be determined from the FileType string, which represents the
// file's extension, such as "PCX" for a PCX file.
//
// Buffer is a pointer to the data buffer to begin storing the
// data; the Export function must return a pointer to the end of
// the data it has written; if it exports 10 bytes, for example,
// it must return (Buffer+10).  If the export function fails,
// it should return NULL, indicating that the data should not be
// saved.
//
// Indent is an optional parameter that objects may use in order
// to properly format data exported to Unreal text files.
//
void UObject::Export( FOutputDevice& Out, const char* FileType, int Indent )
{
	// Default implementation should never be called.
	appErrorf( "%s", "UObject::Export called" );
}

//
// Do any object-specific cleanup required
// immediately after loading an object, and immediately
// after any undo/redo.
//
void UObject::PostLoad()
{
	guard(UObject::PostLoad);
	SetFlags( RF_DebugPostLoad );

	// Clear all non-persistent flags.
	ClearFlags( RF_Trans );

	unguardobj;
}

//
// Edit change notification.
//
void UObject::PostEditChange()
{
	guard(UObject::PostEditChange);
	Modify();
	unguard;
}

//
// Do any object-specific cleanup required immediately
// before an object is killed.  Child classes may override
// this if they have to do anything here.
//
void UObject::Destroy()
{
	guard(UObject::Destroy);
	SetFlags( RF_DebugDestroy );

	// Log message.
	if( GObj.Initialized && !GIsCriticalError )
		debugf( NAME_DevKill, "Destroying %s", GetFullName() );

	// Unhook from linker.
	SetLinker( NULL, INDEX_NONE );

	unguardobj;
}

//
// Set the object's linker.
//
void UObject::SetLinker( ULinkerLoad* InLinker, INT InLinkerIndex )
{
	guard(UObject::SetLinker);

	// Detach from existing linker.
	if( Linker )
	{
		check(Linker->ExportMap(LinkerIndex)._Object==this);
		Linker->ExportMap(LinkerIndex)._Object = NULL;
	}

	// Set new linker.
	Linker      = InLinker;
	LinkerIndex = InLinkerIndex;

	unguardobj;
}

class FOutBuffer : public FOutputDevice, public TArray<BYTE>
{
public:
	void WriteBinary( const void* InData, int Length, EName MsgType )
	{
		INT Index = Add(Length);
		appMemcpy( &(*this)(Index), InData, Length );
	}
};

//
// Return the object's path name.
//warning: Must be safe for NULL objects.
//warning: Must be safe for class-default meta-objects.
//
const char* UObject::GetPathName( UObject* StopParent, char* Str ) const
{
	guard(UObject::GetPathName);
	static char Results[8][256];
	static INT i=0;

	// Return one of 8 circular results.
	char* Result = Str ? Str : Results[i++%ARRAY_COUNT(Results)];
	if( this!=StopParent )
	{
		*Result = 0;
		if( Parent!=StopParent )
		{
			Parent->GetPathName( StopParent, Result );
			appStrcat( Result, "." );
		}
		appStrcat( Result, GetName() );
	}
	else appSprintf( Result, "None" );

	return Result;
	unguard;
}

//
// Return the object's full name.
//warning: Must be safe for NULL objects.
//
const char* UObject::GetFullName( char* Str ) const
{
	guard(UObject::GetFullName);
	static char Results[8][256];
	static int i=0;

	// Return one of 8 circular results.
	char* Result = Str ? Str : Results[i++ % ARRAY_COUNT(Results)];
	if( this )
	{
		appSprintf( Result, "%s ", GetClassName() );
		GetPathName( NULL, Result + appStrlen( Result ) );
	}
	else
	{
		appStrcpy( Result, "None" );
	}
	return Result;

	unguard;
}

//
// Export this object to a file.  Child classes do not
// override this, but they do provide an Export() function
// to do the resoource-specific export work.  Returns 1
// if success, 0 if failed.
//
UBOOL UObject::ExportToFile( const char* Filename )
{
	guard(UObject::ExportToFile);
	UBOOL Success = 0;

	FILE* File = appFopen(Filename,"wb");
	if( File )
	{
		FOutBuffer Buffer;
		Export( Buffer, appFExt(Filename), 0 );
		Success = Buffer.Num() && appFwrite( &Buffer(0), Buffer.Num(), 1, File )==1;
		appFclose(File);
		if( !Success )
		{
			GSystem->Warnf( LocalizeError("ExportWrite"), GetFullName(), Filename );
			appUnlink( Filename );
		}
	}
	else GSystem->Warnf( LocalizeError("ExportOpen"), GetFullName(), Filename );
	return Success;
	unguardobj;
}

//
// Destroy the object if necessary.
//
void UObject::ConditionalDestroy()
{
	guard(UObject::ConditionalDestroy);
	if( !(GetFlags() & RF_Destroyed) )
	{
		SetFlags( RF_Destroyed );
		ClearFlags( RF_DebugDestroy );
		Destroy();
		if( !(GetFlags()&RF_DebugDestroy) )
			appErrorf( "%s failed to route Destroy", GetFullName() );
	}
	unguardobj;
}

//
// Postload if needed.
//
void UObject::ConditionalPostLoad()
{
	guard(UObject::ConditionalPostLoad);
	if( GetFlags() & RF_NeedPostLoad )
	{
		check(GetLinker());
		ClearFlags( RF_NeedPostLoad | RF_DebugPostLoad );
		PostLoad();
		if( !(GetFlags() & RF_DebugPostLoad) )
			appErrorf( "%s failed to route PostLoad", GetFullName() );
	}
	unguard;
}

//
// UObject destructor.
//warning: Called at shutdown.
//
UObject::~UObject()
{
	guard(UObject::~UObject);

	// If not initialized, skip out.
	if( GObj.Initialized && !GIsCriticalError )
	{
		// Validate it.
		check(IsValid());

		// Destroy the object if necessary.
		ConditionalDestroy();

		// Remove object from table.
		GObj.UnhashObject( this );
		GObj.Objects(Index) = NULL;
		GObj.Available.AddItem( Index );
	}

	unguard;
}

//
// Archive for counting memory usage.
//
class FArchiveCountMem : public FArchive
{
public:
	FArchiveCountMem( UObject* Src )
	: Count(0)
	{
		Src->Serialize( *this );
	}
	INT GetCount()
	{
		return Count;
	}
	void CountBytes( INT InCount )
	{
		Count += InCount;
	}
	FArchive& Serialize( void* V, int Length )
	{
		Count += Length;
		return *this;
	}
	FArchive& operator<<( class FName& N )
	{
		Count += sizeof(FName);
		return *this;
	}
	FArchive& operator<<( class UObject*& Obj )
	{
		Count += sizeof(Obj);
		return *this;
	}
protected:
	INT Count;
};

//
// Count memory usage.
//
INT UObject::MemUsage()
{
	guard(UObject::MemUsage);
	return FArchiveCountMem(this).GetCount();
	unguardobj;
}

//
// Note that the object has been modified.
//
void UObject::Modify()
{
	guard(UObject::Modify);

	// Perform transaction tracking.
	if( GUndo && (GetFlags() & RF_Transactional) )
		GUndo->NoteObject( this );

	unguardobj;
}

//
// UObject serializer.
//
void UObject::Serialize( FArchive& Ar )
{
	guard(UObject::Serialize);
	SetFlags( RF_DebugSerialize );

	// Make sure this object's class's data is loaded.
	if( Class != UClass::StaticClass )
		Ar.Preload( Class );

	// Special info.
	if( !Ar.IsLoading() && !Ar.IsSaving() )
		Ar << Name << Parent << Class << Linker;

	// Database.
	if( Ar.Ver() < 40 )//oldver
		Ar << GObj.TempNum << GObj.TempMax;

	// Execution stack.
	guard(SerializeStack);
	if( (GetFlags() & RF_HasStack) || Ar.Ver()<47 )//oldver
	{
		if( !MainFrame )
			MainFrame = new FMainFrame( this );
		if( Ar.Ver()>=51 )
		{
			Ar << MainFrame->Node << MainFrame->StateNode;
		}
		else //oldver
		{
			UClass* OldClass;
			INT iOldNode;
			Ar << OldClass;
			if( OldClass )
				Ar << iOldNode;
			MainFrame->Node = NULL;
		}
		if( Ar.Ver() < 52 )
		{
			UClass* Tmp;
			Ar << Tmp;
		}
		Ar << MainFrame->ProbeMask;
		if( Ar.Ver() >= 55 )
			Ar << MainFrame->LatentAction;
		else
			MainFrame->LatentAction = 0;
		if( MainFrame->Node )
		{
			Ar.Preload( MainFrame->Node );
			if( Ar.IsSaving() && MainFrame->Code )
				check(MainFrame->Code>=&MainFrame->Node->Script(0) && MainFrame->Code<&MainFrame->Node->Script(MainFrame->Node->Script.Num()));
			INT Offset = MainFrame->Code ? MainFrame->Code - &MainFrame->Node->Script(0) : INDEX_NONE;
			Ar << AR_INDEX(Offset);
			if( Offset!=INDEX_NONE )
				if( Offset<0 || Offset>=MainFrame->Node->Script.Num() )
					appErrorf( "%s: Offset mismatch: %i %i", GetFullName(), Offset, MainFrame->Node->Script.Num() );
			MainFrame->Code = Offset!=INDEX_NONE ? &MainFrame->Node->Script(Offset) : NULL;
		}
		else MainFrame->Code = NULL;
		if( !(GetFlags() & RF_HasStack) )
		{
			delete MainFrame;
			MainFrame = NULL;
		}
	}
	else if( MainFrame )
	{
		delete MainFrame;
		MainFrame = NULL;
	}
	unguard;

	// Serialize object properties which are defined in the class.
	if( Class != UClass::StaticClass )
	{
		UObject* ptr = this;
		if( Ar.IsLoading() || Ar.IsSaving() )
			GetClass()->SerializeTaggedProperties( Ar, (BYTE*)ptr, Class );
		else
			GetClass()->SerializeBin( Ar, (BYTE*)ptr);
	}

	// State and group.
	if( Ar.Ver() < 57 ) //oldver: Serialize the state.
	{
		Ar << GObj.TempState;
		if( appStricmp(*GObj.TempState,"PlaySound")==0 )
			GObj.TempState = FName("PlaySoundEffect");
	}
	GObj.TempGroup = NAME_None;
	if( Ar.Ver() < 58 )
	{
		Ar << GObj.TempGroup;
		check(Linker);
		check(LinkerIndex!=INDEX_NONE);
		Linker->ExportMap(GetLinkerIndex()).OldGroup = GObj.TempGroup;
	}

	unguardobj;
}

//
// Group.
//
void UObject::FixOld()//oldver
{
	guard(UObject::FixOld);
	if( Linker )
	{
		check(GetLinkerIndex()>=0);
		check(GetLinkerIndex()<Linker->ExportMap.Num());
		FName Group = Linker->ExportMap(GetLinkerIndex()).OldGroup;
		if( Group!=NAME_None )
		{
			Parent = GObj.CreatePackage(GetParent(),*Group);
			Linker->ExportMap(GetLinkerIndex()).OldGroup = NAME_None;
		}
	}
	unguard;
}

//
// Export text object properties.
//
CORE_API void FObjectManager::ExportProperties
(
	UClass*			ObjectClass,
	BYTE*			Object,
	FOutputDevice*	Out,
	INT				Indent,
	UClass*			DiffClass,
	BYTE*			Diff
)
{
	const char* ThisName="(none)";
	guard(FObjectManager::ExportProperties);
	check(ObjectClass!=NULL);
	for( TFieldIterator<UProperty> It(ObjectClass); It; ++It )
	{
		if( It->Port() )
		{
			ThisName = It->GetName();
			for( INT j=0; j<It->ArrayDim; j++ )
			{
				// Export single element.
				char Value[4096];
				if( It->ExportText( j, Value, Object, (DiffClass && DiffClass->IsChildOf(It.GetStruct())) ? Diff : NULL, 0 ) )
				{
					if
					(	(It->IsA(UObjectProperty::StaticClass))
					&&	(It->PropertyFlags & CPF_ExportObject) )
					{
						UObject* Obj = *(UObject **)((BYTE*)Object + It->Offset + j*It->GetElementSize());
						if( Obj && !(Obj->GetFlags() & RF_TagImp) )
						{
							// Don't export more than once.
							Obj->Export( *Out, "", Indent+1 );
							Obj->SetFlags( RF_TagImp );
						}
					}
					if( It->ArrayDim == 1 )
						Out->Logf( "%s %s=%s\r\n", appSpc(Indent), It->GetName(), Value );
					else
						Out->Logf( "%s %s(%i)=%s\r\n", appSpc(Indent), It->GetName(), j, Value );
				}
			}
		}
	}
	unguardf(( "(%s)", ThisName ));
}

//
// Initialize script execution.
//
void UObject::InitExecution()
{
	guard(UObject::InitExecution);
	check(GetClass()!=NULL);

	MainFrame = new FMainFrame( this );
	SetFlags( RF_HasStack );

	unguard;
}

//
// Command line.
//
UBOOL UObject::ScriptConsoleExec( const char* Str, FOutputDevice* Out )
{
	guard(UObject::ScriptConsoleExec);

	// No script execution in the editor.
	if( GIsEditor )
		return 0;

	// Find UnrealScript exec function.
	char MsgStr[NAME_SIZE];
	FName Message;
	UFunction* Function;
	if
	(	!ParseToken(Str,MsgStr,ARRAY_COUNT(MsgStr),1)
	||	(Message=FName(MsgStr,FNAME_Find))==NAME_None 
	||	(Function=FindFunction(Message))==NULL 
	||	!(Function->FunctionFlags & FUNC_Exec) )
		return 0;

	// Parse all function parameters.
	FMemMark Mark(GMem);
	BYTE* Parms = new(GMem,MEM_Zeroed,Function->ParmsSize)BYTE;
	UBOOL Failed = 0;
	for( TFieldIterator<UProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm|CPF_ReturnParm))==CPF_Parm; ++It )
	{
		ParseNext( &Str );
		Str = It->ImportText( Str, Parms+It->Offset, 1 );
		if( !Str )
		{
			if( !(It->PropertyFlags & CPF_OptionalParm) )
			{
				Out->Logf( LocalizeError("BadProperty"), *Message, It->GetName() );
				Failed = 1;
			}
			break;
		}
	}
	if( !Failed )
		ProcessEvent( Function, Parms );
	Mark.Pop();

	// Success.
	return 1;
	unguard;
}

//
// Find an UnrealScript field.
//warning: Must be safe with class default metaobjects.
//
UField* UObject::FindField( FName InName, UBOOL Global )
{
	guardSlow(UObject::FindField);
	INT iHash = InName.GetIndex() & (UField::HASH_COUNT-1);

#if 1
	// Search current state scope.
	if( MainFrame && MainFrame->StateNode && !Global )
		for( UField* Node=MainFrame->StateNode->VfHash[iHash]; Node; Node=Node->HashNext )
			if( Node->GetFName()==InName )
				return Node;

	// Search the global scope.
	for( UField* Node=GetClass()->VfHash[iHash]; Node; Node=Node->HashNext )
		if( Node->GetFName()==InName )
			return Node;
#else
	// Search current state scope.
	if( MainFrame && MainFrame->StateNode && !Global )
		for( TFieldIterator<UField> It(MainFrame->StateNode); It; ++It )
			if( It->GetFName()==InName )
				return *It;

	// Search the global scope.
	for( TFieldIterator<UField> It(GetClass()); It; ++It )
		if( It->GetFName()==InName )
			return *It;
#endif

	// Not found.
	return NULL;
	unguardfSlow(( "%s (node %s)", GetFullName(), *InName ));
}
UFunction* UObject::FindFunction( FName InName, UBOOL Global )
{
	guardSlow(UObject::FindFunction);
	return Cast<UFunction>( FindField( InName, Global ) );
	unguardfSlow(( "%s (function %s)", GetFullName(), *InName ));
}
UFunction* UObject::FindFunctionChecked( FName InName, UBOOL Global )
{
	guardSlow(UObject::FindFunctionChecked);
	if( GIsEditor )
		return NULL;
	UFunction* Result = Cast<UFunction>( FindField( InName, Global ) );
	if( !Result )
		appErrorf( "Failed to find function %s in %s", *InName, GetFullName() );
	return Result;
	unguardfSlow(( "%s (function %s)", GetFullName(), *InName ));
}
UState* UObject::FindState( FName InName )
{
	guardSlow(UObject::FindState);
	return Cast<UState>( FindField( InName, 1 ) );
	unguardfSlow(( "%s (state %s)", GetFullName(), *InName ));
}
IMPLEMENT_CLASS(UObject);

/*-----------------------------------------------------------------------------
	UObject configuration.
-----------------------------------------------------------------------------*/

//
// Load configuration.
//warning: Must be safe on class-default metaobjects.
//
void UObject::LoadConfig( FName Type, UClass* Class, const char* Filename )
{
	guard(UObject::LoadConfig);
	if( Type==NAME_Localized )
		return;
	DWORD ClassFlags = Type==NAME_Config ? CLASS_Config : CLASS_Localized;
	DWORD Flags      = Type==NAME_Config ? CPF_Config   : CPF_Localized;
	if( !Class )
		Class = GetClass();
	if( !(Class->ClassFlags & ClassFlags) )
		return;
	if( Type==NAME_Localized && GIsEditor )
		return;
	if( Class->GetSuperClass() )
		LoadConfig( Type, Class->GetSuperClass() );
	for( TFieldIterator<UProperty> It(Class); It; ++It )
	{
		if( (It->PropertyFlags & Flags)==Flags )
		{
			for( INT i=0; i<It->ArrayDim; i++ )
			{
				char TempKey[256], Value[1024]="";
				const char* Key = It->GetName();
				if( It->ArrayDim!=1 )
				{
					appSprintf( TempKey, "%s[%i]", It->GetName(), i );
					Key = TempKey;
				}
				UClass* BaseClass = (It->PropertyFlags & CPF_GlobalConfig) ? It->GetOwnerClass() : Class;
				if( GetConfigString( BaseClass->GetPathName(), Key, Value, ARRAY_COUNT(Value), Filename ) )
					It->ImportText( Value, (BYTE*)this + It->Offset + i*It->GetElementSize(), 1 );
			}
		}
	}
	unguard;
}

//
// Save configuration.
//warning: Must be safe on class-default metaobjects.
//
void UObject::SaveConfig( DWORD Flags, const char* Filename )
{
	guard(UObject::SaveConfig);
	for( TFieldIterator<UProperty> It(GetClass()); It; ++It )
	{
		if( (It->PropertyFlags & Flags)==Flags )
		{
			for( INT Index=0; Index<It->ArrayDim; Index++ )
			{
				char TempKey[256], Value[1024]="";
				const char* Key = It->GetName();
				if( It->ArrayDim!=1 )
				{
					appSprintf( TempKey, "%s[%i]", It->GetName(), Index );
					Key = TempKey;
				}
				It->ExportText( Index, Value, (BYTE*)this, (BYTE*)this, 1 );
				UClass* SaveClass = (It->PropertyFlags & CPF_GlobalConfig) ? It->GetOwnerClass() : GetClass();
				SetConfigString( SaveClass->GetPathName(), Key, Value, Filename );
			}
		}
	}
	for( UClass* BaseClass=GetClass(); BaseClass->GetSuperClass(); BaseClass=BaseClass->GetSuperClass() )
		if( !(BaseClass->GetSuperClass()->ClassFlags & CLASS_Config) )
			break;
	if( BaseClass )
		for( TObjectIterator<UClass> It; It; ++It )
			if( It->IsChildOf(BaseClass) )
				It->GetDefaultObject()->LoadConfig( NAME_Config );
	unguard;
}

//
// Reset configuration.
//
void FObjectManager::ResetConfig( UClass* Class, const char* SrcFilename, const char* DestFilename )
{
	guard(FObjectManager::ResetConfig);
	char Buffer[32767], Src[256];
	appSprintf( Src, "%s%s", appBaseDir(), SrcFilename ? SrcFilename : "Default.ini" );
	if( GetConfigSection( Class->GetPathName(), Buffer, ARRAY_COUNT(Buffer), Src ) )
	{
		char* NewKey;
		for( char* Key=Buffer; *Key; Key=NewKey )
		{	
			NewKey = Key + appStrlen(Key) + 1;
			char* Value=appStrchr(Key,'=');
			if( Value )
			{
				*Value++ = 0;
				SetConfigString( Class->GetPathName(), Key, Value, DestFilename );
			}
		}
	}
	for( TObjectIterator<UClass> ItC; ItC; ++ItC )
		if( ItC->IsChildOf(Class) )
			ItC->GetDefaultObject()->LoadConfig( NAME_Config );
	for( TObjectIterator<UObject> ItO; ItO; ++ItO )
		if( ItO->IsA(Class) )
			{ItO->LoadConfig( NAME_Config ); ItO->PostEditChange();}
	unguard;
}

/*-----------------------------------------------------------------------------
	UObject IUnknown implementation.
-----------------------------------------------------------------------------*/

// Note: This are included as part of UObject in case we ever want
// to make Unreal objects into Component Object Model objects.
// The Unreal framework is set up so that it would be easy to componentize
// everything, but there is not yet any benefit to doing so, thus these
// functions aren't implemented.

//
// Query the object on behalf of an Ole client.
//
DWORD STDCALL UObject::QueryInterface( const FGuid &RefIID, void **InterfacePtr )
{
	guard(UObject::QueryInterface);
	// This is not implemented and might not ever be.
	*InterfacePtr = NULL;
	return 0;
	unguardobj;
}

//
// Add a reference to the object on behalf of an Ole client.
//
DWORD STDCALL UObject::AddRef()
{
	guard(UObject::AddRef);
	// This is not implemented and might not ever be.
	return 0;
	unguardobj;
}

//
// Release the object on behalf of an Ole client.
//
DWORD STDCALL UObject::Release()
{
	guard(UObject::Release);
	// This is not implemented and might not ever be.
	return 0;
	unguardobj;
}

/*-----------------------------------------------------------------------------
	FObjectManager.
-----------------------------------------------------------------------------*/

//
// Object accessor.
//
UObject* FObjectManager::GetIndexedObject( INT Index )
{
	guardSlow(FObjectManager::GetObject);
	if( Index>=0 && Index<Objects.Num() )
		return Objects(Index);
	else
		return NULL;
	unguardSlow;
}

//
// Find an optional object.
//
UObject* FObjectManager::FindObject( UClass* ObjectClass, UObject* InObjectPackage, const char* InName, UBOOL ExactClass )
{
	guard(FObjectManager::Find);

	// Resolve the object and package name.
	UObject* ObjectPackage = InObjectPackage!=ANY_PACKAGE ? InObjectPackage : NULL;
	if( !ResolveName( ObjectPackage, InName, 0, 0 ) )
		return NULL;

	// Make sure it's an existing name.
	FName ObjectName(InName,FNAME_Find);
	if( ObjectName==NAME_None )
		return NULL;

	// Find in the specified package.
	for( UObject* Hash=ObjHash[ObjectName.GetIndex() & (ARRAY_COUNT(ObjHash)-1)]; Hash!=NULL; Hash=Hash->HashNext )
	{
		if
		(	(Hash->GetFName()==ObjectName)
		&&	(Hash->Parent==ObjectPackage)
		&&	(ObjectClass==NULL || (ExactClass ? Hash->GetClass()==ObjectClass : Hash->IsA(ObjectClass))) )
			return Hash;
	}
	if( InObjectPackage==ANY_PACKAGE )
	{
		// Find in any package.
		for( UObject* Hash=ObjHash[ObjectName.GetIndex() & (ARRAY_COUNT(ObjHash)-1)]; Hash!=NULL; Hash=Hash->HashNext )
		{
			if
			(	(Hash->GetFName()==ObjectName)
			&&	(ObjectClass==NULL || (ExactClass ? Hash->GetClass()==ObjectClass : Hash->IsA(ObjectClass))) )
				return Hash;
		}
	}

	// Not found.
	return NULL;
	unguard;
}

//
// Find an object; can't fail.
//
UObject* FObjectManager::FindObjectChecked( UClass* ObjectClass, UObject* ObjectParent, const char* InName, UBOOL ExactClass )
{
	UObject* Result = FindObject( ObjectClass, ObjectParent, InName, ExactClass );
	if( !Result )
		appErrorf( LocalizeError("ObjectNotFound"), ObjectClass->GetName(), ObjectParent==ANY_PACKAGE ? "Any" : ObjectParent ? ObjectParent->GetName() : "None", InName );
	return Result;
}

//
// Binary initialize object properties to zero or defaults.
//
void FObjectManager::InitProperties( UClass* Class, BYTE* Data, INT DataCount, UClass* DefaultsClass, BYTE* Defaults, INT DefaultsCount )
{
	guard(FObjectManager::InitProperties);
	check(DataCount>=sizeof(UObject));

	// Count how much memory has been inited.
	INT Inited = sizeof(UObject);

	// Find class defaults if no template was specified.
	guard(FindDefaults);
	if( !Defaults )
	{
		for( DefaultsClass; DefaultsClass; DefaultsClass=DefaultsClass->GetSuperClass() )
		{
			if( DefaultsClass->Defaults.Num() )
			{
				Defaults      = &DefaultsClass->Defaults(0);
				DefaultsCount =  DefaultsClass->Defaults.Num();
				break;
			}
		}
	}
	unguard;

	// Copy defaults.
	guard(DefaultsFill);
	if( Defaults )
	{
		check(DefaultsCount>=sizeof(UObject));
		check(DefaultsCount<=DataCount);
		appMemcpy( Data+sizeof(UObject), Defaults+sizeof(UObject), DefaultsCount-sizeof(UObject) );
		Inited = Max( Inited, DefaultsCount );
	}
	unguard;

	// Zero-fill any remaining portion.
	guard(ZeroFill);
	if( Inited < DataCount )
		appMemset( Data+Inited, 0, DataCount-Inited );
	unguard;

	unguard;
}

//
// Global property setting.
//
void FObjectManager::GlobalSetProperty( const char* Value, UClass* Class, UProperty* Property, INT Offset, UBOOL Immediate )
{
	guard(FObjectManager::GlobalSetProperty);

	// Apply to existing objects of the class, with notification.
	if( Immediate )
	{
		for( FObjectIterator It(Class); It; ++It )
		{
			Property->ImportText( Value, (BYTE*)*It + Offset, 1 );
			It->PostEditChange();
		}
	}

	// Apply to defaults.
	Property->ImportText( Value, &Class->Defaults(Offset), 1 );
	Class->GetDefaultObject()->SaveConfig();

	unguard;
}

/*-----------------------------------------------------------------------------
	Object registration.
-----------------------------------------------------------------------------*/

//
// Preregister an object.
//warning: Called at program startup time.
//
void FObjectManager::PreRegister( UObject* InObject, FName InPackageName )
{
	if( Initialized )
	{
		// Add to the global object table.
		Register( InObject, InPackageName );
	}
	else
	{
		// Add to the autoregistry chain.
		InObject->LinkerIndex = (INT)AutoRegister;
		*(FName*)&InObject->Linker = InPackageName;
		AutoRegister = InObject;
	}
}

//
// Register an object.
//
void FObjectManager::Register( UObject* InObject, FName InPackageName )
{
	guard(FObjectManager::Register);
	check(Initialized);
	check(InObject);

	// Figure out package.
	InObject->Parent = InPackageName!=NAME_None ? CreatePackage(NULL,*InPackageName) : NULL;

	// Validate the object.
	if( InObject->GetFName()==NAME_None )
		appErrorf( "Autoregistered object %s is invalid", InObject->GetFullName() );
	if( FindObject( NULL, InObject->GetParent(), InObject->GetName() ) )
		return;

	// Bind.
	if( InObject->IsA(UPackage::StaticClass) )
		((UPackage*)InObject)->Bind();

	// Add to the global object table.
	AddObject( InObject, INDEX_NONE );
	InObject->SetFlags( RF_Intrinsic );

	unguard;
}

/*-----------------------------------------------------------------------------
	FObjectManager Init & Exit.
-----------------------------------------------------------------------------*/

//
// Init the object manager and allocate tables.
//
void FObjectManager::Init()
{
	guard(FObjectManager::Init);

	// Verify sizes.
	check(sizeof(BYTE)==1);
	check(sizeof(_WORD)==2);
	check(sizeof(DWORD)==4);
	check(sizeof(QWORD)==8);
	check(sizeof(CHAR)==1);
	check(sizeof(SWORD)==2);
	check(sizeof(INT)==4);
	check(sizeof(SQWORD)==8);
	check(sizeof(UBOOL)==4);
	check(sizeof(FLOAT)==4);
	check(sizeof(DOUBLE)==8);

	// Development.
	if( ParseParam(appCmdLine(),"CONFLICTS") )
		GCheckConflicts=1;
	if( ParseParam(appCmdLine(),"NOREPLACE") )
		GNoAutoReplace=1;
	if( ParseParam(appCmdLine(),"NOGC") )
		GNoGC=1;
	

	// Init names.
	FName::InitSubsystem();

	// Init hash.
	for( INT i=0; i<ARRAY_COUNT(ObjHash); i++ )
		ObjHash[i] = NULL;

	// Note initialized.
	Initialized = 1;

	// Allocate special packages.
	TransientPackage = new( NULL, "Transient" )UPackage;
	AddToRoot( TransientPackage );

	// Add all autoregistered classes.
	UObject* Next;
	for( UObject* Object=AutoRegister; Object!=NULL; Object=Next )
	{
		Next = (UObject*)Object->LinkerIndex;
		Register( Object, *(FName*)&Object->Linker );
		Object->Linker      = NULL;
		Object->LinkerIndex = INDEX_NONE;
		if( Object->GetClass()==UClass::StaticClass )
		{
			((UClass*)Object)->ClassInitializer( (UClass*)Object );
			((UClass*)Object)->GetDefaultObject()->LoadConfig(NAME_Config);
			((UClass*)Object)->GetDefaultObject()->LoadConfig(NAME_Localized);
		}
	}

	// Allocate hardcoded objects.
	AddToRoot( new UTextBufferFactory );

	debugf( NAME_Init, "Object subsystem initialized" );
	unguard;
}

//
// Profile comparator.
//
#if DO_SLOW_GUARD
static INT Compare( const UFunction*& A, const UFunction*& B )
{
	return B->Cycles - A->Cycles;
}
#endif

//
// Shut down the object manager.
//
void FObjectManager::Exit()
{
	guard(FObjectManager::Exit);

	// Dump all profile results.
#if DO_SLOW_GUARD
	if( ParseParam(appCmdLine(),"PROFILE") )
	{
		debugf( "Profile of %i ticks:", (INT)GTicks );
		debugf( "                                                    Function  nsec/tick  cyc/call        calls/tick" );
		debugf( "------------------------------------------------------------  ---------  --------------- ----------" );
		TArray<UFunction*> List;
		for( TObjectIterator<UFunction> ItF; ItF; ++ItF )
			if( ItF->Calls!=0 )
				List.AddItem( *ItF );
		appSort( &List(0), List.Num() );
		for( INT i=0; i<List.Num(); i++ )
			debugf
			(
				"%60s  %8.4f  %12.4f  %8.4f",
				List(i)->GetPathName(),
				1000.0*1000.0*GSecondsPerCycle*(DOUBLE)List(i)->Cycles/(DOUBLE)GTicks,
				(DOUBLE)List(i)->Cycles/(DOUBLE)List(i)->Calls,
				(DOUBLE)List(i)->Calls/(DOUBLE)GTicks
			);
	}
#endif

	// Cleanup root.
	RemoveFromRoot( TransientPackage );

	// Tag all objects as unreachable.
	for( FObjectIterator It; It; ++It )
		It->SetFlags( RF_Unreachable | RF_TagGarbage );

	// Tag all names as unreachable.
	for( INT i=0; i<FName::GetMaxNames(); i++ )
		if( FName::GetEntry(i) )
			FName::GetEntry(i)->Flags |= RF_Unreachable;

	// Purge all objects.
	PurgeGarbage( GSystem );
	Objects.Empty();

	// Shut down names.
	FName::ExitSubsystem();

	Initialized = 0;
	debugf( NAME_Exit, "Object subsystem successfully closed." );
	unguard;
}

/*-----------------------------------------------------------------------------
	FObjectManager Tick.
-----------------------------------------------------------------------------*/

//
// Mark one unit of passing time. This is used to update the object
// caching status. This must be called when the object manager is
// in a clean state (outside of all code which retains pointers to
// object data that was gotten).
//
void FObjectManager::Tick()
{
	guard(FObjectManager::Tick);
	check(BeginLoadCount==0);

	// Check intrinsics.
	CORE_API extern int GIntrinsicDuplicate;
	if( GIntrinsicDuplicate )
		appErrorf( "Duplicate intrinsic registered: %i", GIntrinsicDuplicate );

	unguard;
}

/*-----------------------------------------------------------------------------
   FObjectManager shutdown.
-----------------------------------------------------------------------------*/

void UObject::ConditionalShutdownAfterError()
{
	if( !(GetFlags() & RF_ErrorShutdown) )
	{
		SetFlags( RF_ErrorShutdown );
		try
		{
			ShutdownAfterError();
		}
		catch( ... )
		{
			debugf( NAME_Exit, "Double fault in object ShutdownAfterError" );
		}
	}
}

void FObjectManager::ShutdownAfterError()
{
	guard(FObjectManager::ShutdownAfterError);
	static UBOOL Shutdown=0;
	if( Shutdown )
		return;
	Shutdown=1;
	debugf( NAME_Exit, "Executing FObjectManager::ShutdownAfterError" );
	try
	{
		for( INT i=0; i<Objects.Num(); i++ )
			if( Objects(i) )
				Objects(i)->ConditionalShutdownAfterError();
	}
	catch( ... )
	{
		debugf( NAME_Exit, "Double fault in object manager ShutdownAfterError" );
	}
	unguard;
}

/*-----------------------------------------------------------------------------
   FObjectManager command line.
-----------------------------------------------------------------------------*/

class FArchiveShowReferences : public FArchive
{
public:
	FArchiveShowReferences( FOutputDevice* InOut, UObject* InParent, UObject* InObj, TArray<UObject*>& InExclude )
	: DidRef( 0 ), Out( InOut ), Parent( InParent ), Obj( InObj ), Exclude( InExclude )
	{
		Obj->Serialize( *this );
	}
	UBOOL DidRef;
private:
	FArchive& operator<<( UObject*& Obj )
	{
		guard(FArchiveShowReferences<<Obj);
		if( Obj && Obj->GetParent()!=Parent )
		{
			for( INT i=0; i<Exclude.Num(); i++ )
				if( Exclude(i) == Obj->GetParent() )
					break;
			if( i==Exclude.Num() )
			{
				if( !DidRef )
					Out->Logf( "   %s references:", Obj->GetFullName() );
				Out->Logf( "      %s", Obj->GetFullName() );
				DidRef=1;
			}
		}
		return *this;
		unguard;
	}
	FOutputDevice* Out;
	UObject* Parent;
	UObject* Obj;
	TArray<UObject*>& Exclude;
};

//
// Archive for finding who references an object.
//
class FArchiveFindCulprit : public FArchive
{
public:
	FArchiveFindCulprit( UObject* InFind, UObject* Src )
	: Find(InFind), Count(0)
	{
		Src->Serialize( *this );
	}
	INT GetCount()
	{
		return Count;
	}
	FArchive& operator<<( class UObject*& Obj )
	{
		if( Obj==Find )
			Count++;
		return *this;
	}
protected:
	UObject* Find;
	INT Count;
};

static void ShowClasses( UClass* Class, FOutputDevice* Out, int Indent )
{
	Out->Logf( "%s%s", appSpc(Indent), Class->GetName() );
	for( TObjectIterator<UClass> It; It; ++It )
		if( It->GetSuperClass() == Class )
			ShowClasses( *It, Out, Indent+2 );
}

UBOOL FObjectManager::Exec( const char* Cmd, FOutputDevice* Out )
{
	guard(FObjectManager::Exec);
	const char *Str = Cmd;
	if( ParseCommand(&Str,"MEM") )
	{
		appDumpAllocs( Out );
		return 1;
	}
	else if( ParseCommand(&Str,"DUMPINTRINSICS") )
	{
		for( INT i=0; i<EX_Max; i++ )
			if( GIntrinsics[i] == &UObject::execUndefined )
				debugf( "Intrinsic %i is available", i );
		return 1;
	}
	else if( ParseCommand(&Str,"GET") )
	{
		// Get a class default variable.
		char ClassName[256], PropertyName[256];
		UClass* Class;
		UProperty* Property;
		if
		(	ParseToken( Str, ClassName, ARRAY_COUNT(ClassName), 1 )
		&&	(Class=::FindObject<UClass>( ANY_PACKAGE, ClassName))!=NULL )
		{
			if
			(	ParseToken( Str, PropertyName, ARRAY_COUNT(PropertyName), 1 )
			&&	(Property=FindField<UProperty>( Class, PropertyName))!=NULL )
			{
				char Temp[256]="";
				if( Class->Defaults.Num() )
					Property->ExportText( 0, Temp, &Class->Defaults(0), &Class->Defaults(0), 1 );
				Out->Log( Temp );
			}
			else Out->Logf( NAME_ExecWarning, "Unrecognized property %s", PropertyName );
		}
		else Out->Logf( NAME_ExecWarning, "Unrecognized class %s", ClassName );
		return 1;
	}
	else if( ParseCommand(&Str,"SET") )
	{
		// Set a class default variable.
		char ClassName[256], PropertyName[256];
		UClass* Class;
		UProperty* Property;
		if
		(	ParseToken( Str, ClassName, ARRAY_COUNT(ClassName), 1 )
		&&	(Class=::FindObject<UClass>( ANY_PACKAGE, ClassName))!=NULL )
		{
			if
			(	ParseToken( Str, PropertyName, ARRAY_COUNT(PropertyName), 1 )
			&&	(Property=FindField<UProperty>( Class, PropertyName))!=NULL )
			{
				while( *Str==' ' )
					Str++;
				GObj.GlobalSetProperty( Str, Class, Property, Property->Offset, 1 );
			}
			else Out->Logf( NAME_ExecWarning, "Unrecognized property %s", PropertyName );
		}
		else Out->Logf( NAME_ExecWarning, "Unrecognized class %s", ClassName );
		return 1;
	}
	else if( ParseCommand(&Str,"OBJ") )
	{
		if( ParseCommand(&Str,"GARBAGE") )
		{
			// Purge unclaimed objects.
			UBOOL GSavedNoGC=GNoGC;
			GNoGC = 0;
			CollectGarbage( Out, RF_Intrinsic | (GIsEditor ? RF_Standalone : 0) );
			GNoGC = GSavedNoGC;
			return 1;
		}
		else if( ParseCommand(&Str,"MARK") )
		{
			debugf( "Marking objects" );
			for( FObjectIterator It; It; ++It )
				It->SetFlags( RF_Marked );
			return 1;
		}
		else if( ParseCommand(&Str,"MARKCHECK") )
		{
			debugf( "Unmarked objects:" );
			for( FObjectIterator It; It; ++It )
				if( !(It->GetFlags() & RF_Marked) )
					debugf( "%s", It->GetFullName() );
			return 1;
		}
		else if( ParseCommand(&Str,"REFS") )
		{
			UClass* Class;
			UObject* Object;
			if
			(	ParseObject<UClass>( Str, "CLASS=", Class, ANY_PACKAGE )
			&&	ParseObject(Str,"NAME=", Class, Object, ANY_PACKAGE ) )
			{
				Out->Logf("Referencers of %s", Object->GetFullName());
				for( FObjectIterator It; It; ++It )
				{
					FArchiveFindCulprit Ar(Object,*It);
					if( Ar.GetCount() )
						Out->Logf( "%s", It->GetFullName() );
				}
			}
			return 1;
		}
		else if( ParseCommand(&Str,"HASH") )
		{
			// Hash info.
			FName::DisplayHash(Out);
			INT ObjCount=0, HashCount=0;
			for( FObjectIterator It; It; ++It )
				ObjCount++;
			for( int i=0; i<ARRAY_COUNT(ObjHash); i++ )
				if( ObjHash[i] )
					HashCount++;
			return 1;
		}
		else if( ParseCommand(&Str,"CLASSES") )
		{
			ShowClasses( UObject::StaticClass, Out, 0 );
			return 1;
		}
		else if( ParseCommand(&Str,"DEPENDENCIES") )
		{
			UPackage* Pkg;
			if( ParseObject<UPackage>(Str,"PACKAGE=",Pkg,NULL) )
			{
				TArray<UObject*> Exclude;
				for( int i=0; i<16; i++ )
				{
					char Temp[32];
					appSprintf( Temp, "EXCLUDE%i=", i );
					FName F;
					if( Parse(Str,Temp,F) )
						Exclude.AddItem( GObj.CreatePackage(NULL,*F) );
				}
				Out->Logf( "Dependencies of %s:", Pkg->GetName() );
				for( FObjectIterator It; It; ++It )
					if( It->GetParent()==Pkg )
						FArchiveShowReferences ArShowReferences( Out, Pkg, *It, Exclude );
			}
			return 1;
		}
		else if( ParseCommand(&Str,"LIST") )
		{
			UClass* CheckType = NULL;
			ParseObject<UClass>( Str, "CLASS=", CheckType, ANY_PACKAGE );

			UPackage* CheckPackage;
			UBOOL GotPackage = ParseObject<UPackage>( Str, "PACKAGE=", CheckPackage, NULL );

			enum{MAX=256};
			int Num=0, TotalCount=0, TotalSize=0, Count[MAX], Size[MAX];
			UClass* TypeList[MAX];

			Out->Log( "Objects:" );
			for( FObjectIterator It; It; ++It )
			{
				INT ThisSize = It->MemUsage();
				TotalCount++;

				for( int i=0; i<Num; i++ )
					if( TypeList[i] == It->GetClass() )
						break;
				if( i==Num && Num<MAX )
				{
					Count[i] = Size[i] = 0;
					TypeList[i] = It->GetClass();
					Num++;
				}
				Size[i] += ThisSize;
				Count[i]++;

				if
				(	(CheckType  && It->IsA(CheckType))
				||	(GotPackage && It->GetParent()==CheckPackage) )
				{
					Out->Logf( "%s - %i", It->GetFullName(), ThisSize );
					TotalSize += ThisSize;
				}
				else if( !CheckType )
				{
					TotalSize += ThisSize;
				}
			}
			for( INT i=0; i<Num; i++ )
			{
				UClass* Type = TypeList[i];
				if( Type && (CheckType==NULL || CheckType==Type) )
					Out->Logf(" %s...%i (%iK)", Type->GetName(), Count[i], Size[i]/1000 );
			}
			Out->Logf( "%i Objects (%.3fM)",TotalCount,(FLOAT)TotalSize/1000000.0 );
			return 1;
		}
		else return 0;
	}
	else return 0; // Not executed

	unguard;
}

/*-----------------------------------------------------------------------------
   FObjectManager file loading.
-----------------------------------------------------------------------------*/

//
// Safe load error-handling.
//
void FObjectManager::SafeLoadError( DWORD LoadFlags, const char* Error, const char* Fmt, ... )
{
	// Variable arguments setup.
	char TempStr[4096];
	GET_VARARGS( TempStr, Fmt );

	guard(FObjectManager::SafeLoadError);
	debugf( NAME_Warning, TempStr );
	if( LoadFlags & LOAD_Throw     ) appThrowf( "%s", Error   );
	if( LoadFlags & LOAD_NoFail    ) appErrorf( "%s", TempStr );
	if( !(LoadFlags & LOAD_NoWarn) ) GSystem->Warnf ( "%s", TempStr );
	unguard;
}

//
// Find or create the linker for a package.
//
ULinkerLoad* FObjectManager::GetPackageLinker
(
	UObject*		InParent,
	const char*		InFilename,
	DWORD			LoadFlags,
	FPackageMap*	Sandbox,
	FGuid*			CompatibleGuid
)
{
	guard(FObjectManager::GetPackageLinker);
	ULinkerLoad* Result = NULL;
	if( InParent )
		for( INT i=0; i<GObj.Loaders.Num() && !Result; i++ )
			if( GObj.GetLoader(i)->LinkerRoot == InParent )
				Result = GObj.GetLoader(i);
	try
	{
		// See if the linker is already loaded.
		char NewFilename[256]="";
		if( Result )
		{
			// Linker already found.
			appStrcpy( NewFilename, "" );
		}
		else if( !InFilename )
		{
			// Resolve filename from package name.
			if( !InParent )
				appThrowf( LocalizeError("PackageResolveFailed") );
			if( !appFindPackageFile( InParent->GetName(), CompatibleGuid, NewFilename ) )
			{
				// See about looking in the dll.
				if( (LoadFlags & LOAD_AllowDll) && InParent->IsA(UPackage::StaticClass) && ((UPackage*)InParent)->DllHandle )
					return NULL;
				appThrowf( LocalizeError("PackageNotFound"), InParent->GetName() );
			}
		}
		else
		{
			// Verify that file exists.
			if( !appFindPackageFile( InFilename, CompatibleGuid, NewFilename ) )
				appThrowf( LocalizeError("FileNotFound"), InFilename );

			// Resolve package name from filename.
			char Tmp[256], *T=Tmp;
			appStrcpy( Tmp, InFilename );
			while( 1 )
			{
				if( appStrchr(T,'\\') )
					T = appStrchr(T,'\\')+1;
				else if( appStrchr(T,'/') )
					T = appStrchr(T,'/')+1;
				else if( appStrchr(T,':') )
					T = appStrchr(T,':')+1;
				else
					break;
			}
			if( appStrchr(T,'.') )
				*appStrchr(T,'.') = 0;
			UPackage* FilenamePkg = CreatePackage( NULL, T );

			// If no package specified, use package from file.
			if( InParent==NULL )
			{
				if( !FilenamePkg )
					appThrowf( LocalizeError("FilenameToPackage"), InFilename );
				InParent = FilenamePkg;
				for( INT i=0; i<GObj.Loaders.Num() && !Result; i++ )
					if( GObj.GetLoader(i)->LinkerRoot == InParent )
						Result = GObj.GetLoader(i);
			}
			else if( InParent != FilenamePkg )
			{
				// Loading a new file into an existing package, so reset the loader.
				debugf( "New File, Existing Package (%s, %s)", InParent->GetFullName(), FilenamePkg->GetFullName() );
				ResetLoaders( InParent );
			}
		}

		// Make sure the package is accessible in the sandbox.
		for( INT i=0; Sandbox && i<Sandbox->Num(); i++ )
			if( (*Sandbox)(i).Parent == InParent )
				Sandbox = NULL;
		if( Sandbox )
			appThrowf( LocalizeError("Sandbox"), InParent->GetName() );

		// Create new linker.
		if( !Result )
			Result = new( GetTransientPackage() )ULinkerLoad( InParent, NewFilename, LoadFlags );

		// Verify compatibility.
		if( CompatibleGuid )
		{
			for( INT i=0; i<Result->Heritage.Num(); i++ )
				if( Result->Heritage(i)==*CompatibleGuid )
					break;
			if( i==Result->Heritage.Num() )
				appThrowf( LocalizeError("PackageVersion"), InParent->GetName() );
		}
	}
	catch( char* Error )
	{
		SafeLoadError( LoadFlags, Error, LocalizeError("FailedLoad"), InFilename ? InFilename : InParent ? InParent->GetName() : "NULL", Error );
	}

	// Success.
	check(Result);
	return Result;
	unguard;
}

//
// Find or optionally create a package.
//
UPackage* FObjectManager::CreatePackage( UObject* InParent, const char* InName )
{
	guard(FObjectManager::FindPackage);

	ResolveName( InParent, InName, 1, 0 );
	UPackage* Result = ::FindObject<UPackage>( InParent, InName );
	if( !Result )
		Result = new( InParent, InName, RF_Public )UPackage;
	return Result;

	unguard;
}

//
// Resolve a package and name.
//
UBOOL FObjectManager::ResolveName( UObject*& InPackage, const char*& InName, UBOOL Create, UBOOL Throw )
{
	guard(FObjectManager::ResolveName);
	check(InName);
	static char Results[8][256];
	static int i=0;

	// See if the name is specified in the .ini file.
	if( appStrnicmp( InName, "ini:", 4 )==0 && appStrlen(InName)<ARRAY_COUNT(Results[0]) && appStrchr(InName,'.') )
	{
		// Get .ini key and section.
		char Section[256];
		appStrcpy( Section, InName+4 );
		char* Key = Section;
		while( appStrchr(Key,'.') )
			Key = appStrchr(Key,'.')+1;
		check(Key!=Section);
		Key[-1] = 0;

		// Look up name.
		char* Temp = Results[i++%ARRAY_COUNT(Results)];
		if( !GetConfigString( Section, Key, Temp, ARRAY_COUNT(Results[0]) ) )
			if( Throw )
				appThrowf( LocalizeError("ConfigNotFound"), InName );
		InName = Temp;
	}

	// Handle specified packages.
	while( appStrchr(InName,'.') )
	{
		char PartialName[256];
		appStrcpy( PartialName, InName );
		*appStrchr( PartialName, '.' ) = 0;
		if( Create )
		{
			InPackage = CreatePackage( InPackage, PartialName );
		}
		else
		{
			InPackage = ::FindObject<UPackage>( InPackage, PartialName );
			if( !InPackage )
				return 0;
		}
		InName = appStrchr(InName,'.')+1;
	}
	return 1;
	unguard;
}

//
// Load an object.
//
UObject* FObjectManager::LoadObject( UClass* ObjectClass, UObject* InParent, const char* InName, const char* Filename, DWORD LoadFlags, FPackageMap* Sandbox )
{
	guard(FObjectManager::LoadObject);
	check(ObjectClass);
	check(InName);

	// Try to load.
	UObject* Result=NULL;
	BeginLoad();
	try
	{
		// Create a new linker object which goes off and tries load the file.
		ULinkerLoad* Linker = NULL;
		ResolveName( InParent, InName, 1, 1 );
		if( !(LoadFlags & LOAD_DisallowFiles) )
			Linker = GetPackageLinker( InParent, Filename, LoadFlags | LOAD_Throw | LOAD_AllowDll, Sandbox, NULL );
		if( Linker )
			Result = Linker->Create( ObjectClass, InName, LoadFlags, 0 );
		if( !Result )
			Result = FindObject( ObjectClass, InParent, InName );
		if( !Result )
			appThrowf( LocalizeError("ObjectNotFound"), ObjectClass->GetName(), InParent ? InParent->GetPathName() : "None", InName );
		EndLoad();
	}
	catch( char* Error )
	{
		EndLoad();
		SafeLoadError( LoadFlags, Error, LocalizeError("FailedLoadObject"), ObjectClass->GetName(), InParent ? InParent->GetName() : "None", InName, Error );
	}
	return Result;
	unguardf(( "(%s %s.%s %s)", ObjectClass->GetPathName(), InParent ? InParent->GetPathName() : "None", InName, Filename ? Filename : "null" ));
}

//
// Load a class.
//
UClass* FObjectManager::LoadClass( UClass* BaseClass, UObject* InParent, const char* InName, const char* Filename, DWORD LoadFlags, FPackageMap* Sandbox )
{
	guard(FObjectManager::LoadClass);
	check(BaseClass);
	try
	{
		UClass* Class = (UClass*)GObj.LoadObject( UClass::StaticClass, InParent, InName, Filename, LoadFlags | LOAD_Throw, Sandbox );
		if( Class && !Class->IsChildOf(BaseClass) )
			appThrowf( LocalizeError("LoadClassMismatch"), Class->GetFullName(), BaseClass->GetFullName() );
		return Class;
	}
	catch( char* Error )
	{
		// Failed.
		SafeLoadError( LoadFlags, Error, Error );
		return NULL;
	}
	unguard;
}

//
// Load all objects in a package.
//
UObject* FObjectManager::LoadPackage( UObject* InParent, const char* Filename, DWORD LoadFlags )
{
	guard(FObjectManager::LoadPackage);
	//DWORD Time=0; clock(Time);
	UObject* Result;

	// Try to load.
	BeginLoad();
	try
	{
		// Create a new linker object which goes off and tries load the file.
		ULinkerLoad* Linker = GetPackageLinker( InParent, Filename ? Filename : InParent->GetName(), LoadFlags | LOAD_Throw, NULL, NULL );
		if( !(LoadFlags & LOAD_Verify) )
			Linker->LoadAllObjects();
		Result = Linker->LinkerRoot;
		EndLoad();
	}
	catch( char* Error )
	{
		EndLoad();
		SafeLoadError( LoadFlags, Error, LocalizeError("FailedLoadPackage"), Error );
		Result = NULL;
	}
	//unclock(Time); debugf(NAME_Log,"LoadPackage=%f",GSecondsPerCycle*1000*Time);
	return Result;
	unguard;
}

//
// Begin loading packages.
//warning: Objects may not be destroyed between BeginLoad/EndLoad calls.
//
void FObjectManager::BeginLoad()
{
	guard(FObjectManager::BeginLoad);
	if( ++BeginLoadCount == 1 )
	{
		// Initiate load.
		for( INT i=0; i<Loaders.Num(); i++ )
			check(GetLoader(i)->Success);
	}
	unguard;
}

//
// End loading packages.
//
void FObjectManager::EndLoad()
{
	guard(FObjectManager::EndLoad);
	check(BeginLoadCount>0);
	if( --BeginLoadCount == 0 )
	{
		try
		{
			// Finish loading everything.
			guard(LoadObjects);
			debugfSlow( NAME_DevLoad, "Loading objects..." );
			UBOOL Preloaded;
			do
			{
				Preloaded=0;
				for( FObjectIterator It; It; ++It )
				{
					if( It->GetFlags() & RF_NeedLoad )
					{
						check(It->GetLinker());
						Preloaded = 1;
						It->GetLinker()->Preload( *It );
					}
				}
			} while( Preloaded );
			unguard;

			// Postload the objects.
			guard(PostloadObjects);
			debugfSlow( NAME_DevLoad, "Linking all objects..." );
			for( FObjectIterator It; It; ++It )
				It->ConditionalPostLoad();
			unguard;

			// Dissociate all linker object imports, since they may be destroyed,
			// causing their pointers to become invalid.
			guard(DissociateImports);
			for( INT i=0; i<Loaders.Num(); i++ )
			{
				for( INT j=0; j<GetLoader(i)->ImportMap.Num(); j++ )
				{
					FObjectImport& Import = GetLoader(i)->ImportMap(j);
					if( Import.Object && !(Import.Object->GetFlags() & RF_Intrinsic) )
						Import.Object = NULL;
				}
			}
			unguard;
		}
		catch( const char* Error )
		{
			appErrorf( Error );
		}
	}
	unguard;
}

//
// Empty the loaders.
//
void FObjectManager::ResetLoaders( UObject* Pkg )
{
	guard(FObjectManager::ResetLoaders);

	for( INT i=Loaders.Num()-1; i>=0; i-- )
		if( Pkg==NULL || GetLoader(i)->LinkerRoot==Pkg )
			delete GetLoader(i);

	unguard;
}

/*-----------------------------------------------------------------------------
	FObjectManager file saving.
-----------------------------------------------------------------------------*/

//
// Archive for tagging objects and names that must be exported
// to the file.  It tags the objects passed to it, and recursively
// tags all of the objects this object references.
//
class FArchiveSaveTagExports : public FArchive
{
public:
	FArchiveSaveTagExports( UObject* InParent )
	: Parent(InParent)
	{
		ArIsSaving = 1;
	}
	FArchive& operator<<( UObject*& Obj )
	{
		guard(FArchiveSaveTagExports<<Obj);
		if( Obj && Obj->IsIn(Parent) && !(Obj->GetFlags() & (RF_Transient|RF_TagExp)) )
		{
			// Set flags.
			Obj->SetFlags(RF_TagExp);
			if( !(Obj->GetFlags() & RF_NotForEdit  ) ) Obj->SetFlags(RF_LoadForEdit);
			if( !(Obj->GetFlags() & RF_NotForClient) ) Obj->SetFlags(RF_LoadForClient);
			if( !(Obj->GetFlags() & RF_NotForServer) ) Obj->SetFlags(RF_LoadForServer);

			// Recurse with this object's class and package.
			UClass* Class = Obj->GetClass();
			UObject* Parent = Obj->GetParent();
			*this << Class << Parent;

			// Recurse with this object's children.
			Obj->Serialize( *this );
		}
		return *this;
		unguard;
	}
	UObject* Parent;
};

//
// QSort comparators.
//
static ULinkerSave* GTempSave;
INT CDECL LinkerNameSort( const void* A, const void* B )
{
	return GTempSave->MapName((FName*)B) - GTempSave->MapName((FName*)A);
}
INT CDECL LinkerImportSort( const void* A, const void* B )
{
	return GTempSave->MapObject(((FObjectImport*)B)->Object) - GTempSave->MapObject(((FObjectImport*)A)->Object);
}
INT CDECL LinkerExportSort( const void* A, const void* B )
{
	return GTempSave->MapObject(((FObjectExport*)B)->_Object) - GTempSave->MapObject(((FObjectExport*)A)->_Object);
}

//
// Archive for tagging objects and names that must be listed in the
// file's imports table.
//
class FArchiveSaveTagImports : public FArchive
{
public:
	DWORD ContextFlags;
	FArchiveSaveTagImports( ULinkerSave* InLinker, DWORD InContextFlags )
	: ContextFlags( InContextFlags ), Linker( InLinker )
	{
		ArIsSaving = 1;
	}
	FArchive& operator<<( UObject*& Obj )
	{
		guard(FArchiveSaveTagImports<<Obj);
		if( Obj && !Obj->IsPendingKill() )
		{
			if( !(Obj->GetFlags() & RF_Transient) || (Obj->GetFlags() & RF_Public) )
			{
				Linker->ObjectIndices(Obj->GetIndex())++;
				if( !(Obj->GetFlags() & RF_TagExp ) )
				{
					Obj->SetFlags( RF_TagImp );
					if( !(Obj->GetFlags() & RF_NotForEdit  ) ) Obj->SetFlags(RF_LoadForEdit);
					if( !(Obj->GetFlags() & RF_NotForClient) ) Obj->SetFlags(RF_LoadForClient);
					if( !(Obj->GetFlags() & RF_NotForServer) ) Obj->SetFlags(RF_LoadForServer);
					UObject* Parent = Obj->GetParent();
					if( Parent )
						*this << Parent;
				}
			}
		}
		return *this;
		unguard;
	}
	FArchive& operator<<( FName& Name )
	{
		guard(FArchiveSaveTagImports<<Name);
		Name.SetFlags( RF_TagExp | ContextFlags );
		Linker->NameIndices(Name.GetIndex())++;
		return *this;
		unguard;
	}
	ULinkerSave* Linker;
};

//
// Save one specific object into an Unrealfile.
//
UBOOL FObjectManager::SavePackage( UObject* InParent, UObject* Base, DWORD TopLevelFlags, const char* Filename, UBOOL NoWarn )
{
	guard(FObjectManager::SavePackage);
	check(InParent);
	check(Filename);
	DWORD Time=0; clock(Time);

	// Tag parent flags.
	if( Cast<UPackage>( InParent ) )
		Cast<UPackage>( InParent )->PackageFlags |= PKG_AllowDownload;

	// Make temp file.
	char TempFilename[256];
	appStrcpy( TempFilename, Filename );
	int c = appStrlen(TempFilename);
	while( c>0 && TempFilename[c-1]!='\\' && TempFilename[c-1]!='/' && TempFilename[c-1]!=':' )
		c--;
	TempFilename[c]=0;
	appStrcat( TempFilename, "Save.tmp" );

	// Init.
	GSystem->StatusUpdatef( 0, 0, LocalizeProgress("Saving"), Filename );
	UBOOL Success = 0;

	// If we have a load-linker for the package, unload it.
	guard(UncacheLoader);
	for( INT i=0; i<Loaders.Num(); i++ )
	{
		if( GetLoader(i)->LinkerRoot==InParent )
		{
			debugf( NAME_Log, "Uncaching loader for: %s", GetLoader(i)->Filename );
			delete Loaders(i);
		}
	}
	unguard;

	// Untag all objects and names.
	guard(Untag);
	for( FObjectIterator It; It; ++It )
		It->ClearFlags( RF_TagImp | RF_TagExp | RF_LoadForEdit | RF_LoadForClient | RF_LoadForServer );
	for( INT i=0; i<FName::GetMaxNames(); i++ )
		if( FName::GetEntry(i) )
			FName::GetEntry(i)->Flags &= ~(RF_TagImp | RF_TagExp | RF_LoadForEdit | RF_LoadForClient | RF_LoadForServer);
	unguard;

	// Export objects.
	guard(TagExports);
	FArchiveSaveTagExports Ar( InParent );
	if( Base )
		Ar << Base;
	for( FObjectIterator It; It; ++It )
	{
		if( (It->GetFlags() & TopLevelFlags) && It->IsIn(InParent) )
		{
			UObject* Obj = *It;
			Ar << Obj;
		}
	}
	unguard;

	ULinkerSave* Linker = NULL;
	try
	{
		// Allocate the linker.
		Linker = new ULinkerSave( InParent, TempFilename );

		// Import objects and names.
		guard(TagImports);
		for( FObjectIterator It; It; ++It )
		{
			if( It->GetFlags() & RF_TagExp )
			{
				// Build list.
				FArchiveSaveTagImports Ar( Linker, It->GetFlags() & RF_LoadContextFlags );
				It->Serialize( Ar );
				UClass* Class = It->GetClass();
				Ar << Class;
				if( It->GetParent()==GetTransientPackage() )
					appErrorf( LocalizeError("TransientImport"), It->GetFullName() );
			}
		}
		unguard;

		// Export all relevant object, class, and package names.
		guard(ExportNames);
		for( FObjectIterator It; It; ++It )
		{
			if( It->GetFlags() & (RF_TagExp|RF_TagImp) )
			{
				It->GetFName().SetFlags( RF_TagExp | RF_LoadForEdit | RF_LoadForClient | RF_LoadForServer );
				if( It->GetParent() )
					It->GetParent()->GetFName().SetFlags( RF_TagExp | RF_LoadForEdit | RF_LoadForClient | RF_LoadForServer );
				if( It->GetFlags() & RF_TagImp )
				{
					It->Class->GetFName().SetFlags( RF_TagExp | RF_LoadForEdit | RF_LoadForClient | RF_LoadForServer );
					check(It->Class->GetParent());
					It->Class->GetParent()->GetFName().SetFlags( RF_TagExp | RF_LoadForEdit | RF_LoadForClient | RF_LoadForServer );
					if( !(It->GetFlags() & RF_Public) )
						appThrowf( LocalizeError("FailedSavePrivate"), Filename, It->GetFullName() );
				}
				else debugfSlow( NAME_DevSave, "Saving %s", It->GetFullName() );
			}
		}
		unguard;

		// Write fixed-length file summary to overwrite later.
		guard(SaveSummary);
		*Linker << Linker->Summary;
		unguard;

		// Save heritage.
		guard(SaveHeritage);
		Linker->Heritage.AddItem( appCreateGuid() );
		Linker->Summary.HeritageCount = Linker->Heritage.Num();
		Linker->Summary.HeritageOffset = Linker->Tell();
		for( INT i=0; i<Linker->Heritage.Num(); i++ )
			*Linker << Linker->Heritage(i);
		unguard;

		// Build NameMap.
		guard(BuildNameMap);
		Linker->Summary.NameOffset = Linker->Tell();
		for( int i=0; i<FName::GetMaxNames(); i++ )
		{
			if( FName::GetEntry(i) )
			{
				FName Name( (EName)i );
				if( Name.GetFlags() & RF_TagExp )
					Linker->NameMap.AddItem( Name );
			}
		}
		Linker->Summary.NameCount = Linker->NameMap.Num();
		unguard;

		// Sort names by usage count in order to maximize compression.
		guard(SortNames);
		GTempSave = Linker;
		appQsort( &Linker->NameMap(0), Linker->NameMap.Num(), sizeof(Linker->NameMap(0)), LinkerNameSort );
		unguard;

		// Save names.
		guard(SaveNames);
		for( INT i=0; i<Linker->NameMap.Num(); i++ )
		{
			*Linker << *FName::GetEntry( Linker->NameMap(i).GetIndex() );
			Linker->NameIndices(Linker->NameMap(i).GetIndex()) = i;
		}
		unguard;

		// Build ImportMap.
		guard(BuildImportMap);
		for( FObjectIterator It; It; ++It )
			if( It->GetFlags() & RF_TagImp )
				new( Linker->ImportMap )FObjectImport( *It );
		Linker->Summary.ImportCount = Linker->ImportMap.Num();
		unguard;

		// Sort imports by usage count.
		guard(SortImports);
		GTempSave = Linker;
		appQsort( &Linker->ImportMap(0), Linker->ImportMap.Num(), sizeof(Linker->ImportMap(0)), LinkerImportSort );
		unguard;

		// Build ExportMap.
		guard(BuildExports);
		for( FObjectIterator It; It; ++It )
			if( It->GetFlags() & RF_TagExp )
				new( Linker->ExportMap )FObjectExport( *It );
		Linker->Summary.ExportCount = Linker->ExportMap.Num();
		unguard;

		// Sort exports by usage count.
		guard(SortExports);
		appQsort( &Linker->ExportMap(0), Linker->ExportMap.Num(), sizeof(Linker->ExportMap(0)), LinkerExportSort );
		unguard;

		// Set linker reverse mappings.
		guard(SetLinkerMappings);
		for( INT i=0; i<Linker->ExportMap.Num(); i++ )
			Linker->ObjectIndices(Linker->ExportMap(i)._Object->GetIndex()) = i+1;
		for( i=0; i<Linker->ImportMap.Num(); i++ )
			Linker->ObjectIndices(Linker->ImportMap(i).Object->GetIndex()) = -i-1;
		unguard;

		// Save exports.
		guard(SetExportIndices);
		for( INT i=0; i<Linker->ExportMap.Num(); i++ )
		{
			FObjectExport& Export = Linker->ExportMap(i);

			// Set class index.
			if( !Export._Object->IsA(UClass::StaticClass) )
			{
				Export.ClassIndex = Linker->ObjectIndices(Export._Object->GetClass()->GetIndex());
				check(Export.ClassIndex!=0);
			}
			if( Export._Object->IsA(UStruct::StaticClass) )
			{
				UStruct* Struct = (UStruct*)Export._Object;
				if( Struct->SuperField )
				{
					Export.ParentIndex = Linker->ObjectIndices(Struct->SuperField->GetIndex());
					check(Export.ParentIndex!=0);
				}
			}

			// Set package index.
			if( Export._Object->GetParent() != InParent )
			{
				check(Export._Object->GetParent()->IsIn(InParent));
				Export.PackageIndex = Linker->ObjectIndices(Export._Object->GetParent()->GetIndex());
				check(Export.PackageIndex>0);
			}

			// Save it.
			Export.SerialOffset = Linker->Tell();
			Export._Object->Serialize( *Linker );
			Export.SerialSize = Linker->Tell() - Linker->ExportMap(i).SerialOffset;
		}
		unguard;

		// Save the import map.
		guard(SaveImportMap);
		Linker->Summary.ImportOffset = Linker->Tell();
		for( INT i=0; i<Linker->ImportMap.Num(); i++ )
		{
			FObjectImport& Import = Linker->ImportMap( i );

			// Set the package index.
			if( Import.Object->GetParent() )
			{
				check(!Import.Object->GetParent()->IsIn(InParent));
				Import.PackageIndex = Linker->ObjectIndices(Import.Object->GetParent()->GetIndex());
				check(Import.PackageIndex<0);
			}

			// Save it.
			*Linker << Import;
		}
		unguard;

		// Save the export map.
		guard(SaveExportMap);
		Linker->Summary.ExportOffset = Linker->Tell();
		for( int i=0; i<Linker->ExportMap.Num(); i++ )
			*Linker << Linker->ExportMap( i );
		unguard;

		// Rewrite updated file summary.
		guard(RewriteSummary);
		GSystem->StatusUpdatef( 0, 0, "%s", LocalizeProgress("Closing") );
		Linker->Seek(0);
		*Linker << Linker->Summary;
		unguard;

		Success = 1;
	}
	catch( char* Error )
	{
		// Delete the temporary file.
		appUnlink( TempFilename );
		if( NoWarn )
			debugf( NAME_Warning, LocalizeError("FailedSaveFile"), Filename, Error );
		else
			GSystem->Warnf( "%s", Error );
	}
	if( Linker )
	{
		delete Linker;
	}
	unclock(Time); debugf(NAME_Log,"Save=%f",GSecondsPerCycle*1000*Time);
	if( Success )
	{
		// Move the temporary file.
		debugf( NAME_Log, "Moving '%s' to '%s'", TempFilename, Filename );
		if( !appMoveFile( TempFilename, Filename ) )
		{
			appUnlink( TempFilename );
			GSystem->Warnf( LocalizeError("SaveWarning"), Filename );
			Success = 0;
		}
	}
	return Success;
	unguard;
}

/*-----------------------------------------------------------------------------
	FObjectManager misc.
-----------------------------------------------------------------------------*/

//
// Add an object to the root array. This prevents the object and all
// its descendents from being deleted during garbage collection.
//
void FObjectManager::AddToRoot( UObject* Obj )
{
	guard(FObjectManager::AddToRoot);
	Root.AddItem( Obj );
	unguard;
}

//
// Remove an object from the root array.
//
void FObjectManager::RemoveFromRoot( UObject* Obj )
{
	guard(FObjectManager::RemoveFromRoot);
	Root.RemoveItem( Obj );
	unguard;
}

/*-----------------------------------------------------------------------------
	Object name functions.
-----------------------------------------------------------------------------*/

//
// Create a unique name by combining a base name and an arbitrary number string.  
// The object name returned is guaranteed not to exist.
//
FName FObjectManager::MakeUniqueObjectName( UObject* Parent, UClass* Class )
{
	guard(FObjectManager::MakeUniqueObjectName);
	//DWORD Time=0; clock(Time);
	check(Class);

	char NewBase[NAME_SIZE], Result[NAME_SIZE];
	char TempIntStr[NAME_SIZE];
	FName ResultName;

	// Make base name sans appended numbers.
	appStrcpy( NewBase, Class->GetName() );
	char* End = NewBase + appStrlen(NewBase);
	while( End>NewBase && appIsDigit(End[-1]) )
		End--;
	*End = 0;

	// Append numbers to base name.
	do
	{
		appSprintf( TempIntStr, "%i", Class->ClassUnique++ );
		appStrncpy( Result, NewBase, NAME_SIZE-appStrlen(TempIntStr)-1 );
		appStrcat( Result, TempIntStr );
	} while( FindObject( NULL, Parent, Result ) );

	//unclock(Time); debugf(NAME_Log,"MakeUniqueObjectName=%f",GSecondsPerCycle*1000*Time);
	return Result;
	unguard;
}

/*-----------------------------------------------------------------------------
	Object hashing.
-----------------------------------------------------------------------------*/

//
// Add an object to the hash table.
//
void FObjectManager::HashObject( UObject* Obj )
{
	guard(FObjectManager::HashObject);

	INT iHash      = Obj->GetHash();
	Obj->HashNext  = ObjHash[iHash];
	ObjHash[iHash] = Obj;

	unguard;
}

//
// Remove an object from the hash table.
//
void FObjectManager::UnhashObject( UObject* Obj )
{
	guard(FObjectManager::UnhashObject);

	INT       iHash   = Obj->GetHash();
	UObject** Hash    = &ObjHash[iHash];
	INT       Removed = 0;
	while( *Hash != NULL )
	{
		if( *Hash != Obj )
		{
			Hash = &(*Hash)->HashNext;
 		}
		else
		{
			*Hash = (*Hash)->HashNext;
			Removed++;
		}
	}
	check(Removed!=0);
	check(Removed==1);

	unguard;
}

/*-----------------------------------------------------------------------------
	Creating and allocating data for new objects.
-----------------------------------------------------------------------------*/

//
// Add an object to the table.
//
void FObjectManager::AddObject( UObject* Obj, INT Index )
{
	guard(FObjectManager::AddObject);

	// Find an available index.
	if( Index==INDEX_NONE )
	{
		if( Available.Num() )
		{
			Index = Available( Available.Num()-1 );
			check(Objects(Index)==NULL);
			Available.Remove( Available.Num()-1 );
		}
		else Index = Objects.Add();
	}

	// Add to global table.
	Objects(Index)  = Obj;
	Obj->Index      = Index;
	HashObject( Obj );

	unguard;
}

//
// Create a new object or replace an existing one.
// If Name is NAME_None, tries to create an object with an arbitrary unique name.
// No failure conditions.
//
UObject* FObjectManager::AllocateObject
(
	UClass*			InClass,
	UObject*		InParent,
	FName			InName,
	DWORD			InFlags,
	UObject*		InTemplate
)
{
	guard(FObjectManager::AllocateObject);
	//DWORD Time=0; clock(Time);

	check(InClass!=NULL);
	if( !InParent && InClass!=UPackage::StaticClass )
		appErrorf( LocalizeError("NotPackaged"), InClass->GetName(), *InName );

	// Validation check.
	if( InClass->ClassFlags & CLASS_Abstract )
		appErrorf( LocalizeError("Abstract"), *InName, InClass->GetName() );

	// Compose name.
	if( InName==NAME_None )
		InName = GObj.MakeUniqueObjectName( InParent, InClass );

	// See if object already exists.
	UObject* Obj  = FindObject( InClass, InParent, *InName );
	INT Index     = INDEX_NONE;
	INT ClassSize = 0;
	void (*Constructor)(void*) = NULL;
	DWORD ClassFlags = 0;
	if( !Obj )
	{
		// Create a new object.
		Obj = (UObject *)appMalloc( InClass->GetPropertiesSize(), *InName );
	}
	else
	{
		// Replace an existing object without affecting the original's address or index.
		debugf( NAME_DevReplace, "Replacing %s", Obj->GetName() );

		if( Obj->GetClass() != InClass )
			appErrorf( LocalizeError("NoReplace"), Obj->GetFullName(), InClass->GetName() );

		InFlags |= (Obj->GetFlags() & RF_Keep);
		if( Obj->IsA( UClass::StaticClass ) )
		{
			Constructor = ((UClass*)Obj)->Constructor;
			ClassFlags  = ((UClass*)Obj)->ClassFlags & CLASS_Abstract;
		}
		Index = Obj->Index;
		Obj->~UObject();
		if( Available.Num() && Available(Available.Num()-1)==Index )
			Available.Remove(Available.Num()-1);
	}

	// If class is transient, objects must be transient.
	if( InClass->ClassFlags & CLASS_Transient )
		InFlags |= RF_Transient;

	// Set the base properties.
	guard(InitBase);
	Obj->Name			 = InName;
	Obj->Parent			 = InParent;
	Obj->Class			 = InClass;
	Obj->ObjectFlags	 = InFlags;
	Obj->LinkerIndex	 = INDEX_NONE;
	Obj->Linker			 = NULL;
	Obj->Index			 = INDEX_NONE;
	Obj->HashNext		 = NULL;
	unguard;

	// Init the properties.
	guard(CallInitProperties);
	GObj.InitProperties
	(
		InClass,
		(BYTE*)Obj,
		InClass!=UClass::StaticClass ? InClass->GetPropertiesSize() : sizeof(UClass),
		InClass,
		(BYTE*)InTemplate,
		InClass->GetPropertiesSize()
	);
	unguard;

	// Add to global table.
	AddObject( Obj, Index );

	// Restore class information.
	if( Obj->IsA( UClass::StaticClass ) )
	{
		((UClass*)Obj)->Constructor  = Constructor;
		((UClass*)Obj)->ClassFlags  |= ClassFlags;
	}

	// Success.
	//unclock(Time); debugf(NAME_Log,"AllocateObject=%f",GSecondsPerCycle*1000*Time);
	return Obj;
	unguardf(( "(%s %s)", InClass ? InClass->GetName() : "NULL", *InName ));
}

//
// Construct an object.
//
UObject* FObjectManager::ConstructObject
(
	UClass*			InClass,
	UObject*		InParent,
	FName			InName,
	DWORD			InFlags,
	UObject*		InTemplate
)
{
	guard(FObjectManager::ConstructObject);
	//DWORD Time=0; clock(Time);

	if( GCheckConflicts )
		for( UObject* Hash=ObjHash[InName.GetIndex() & (ARRAY_COUNT(ObjHash)-1)]; Hash!=NULL; Hash=Hash->HashNext )
			if
			(	Hash->GetFName()==InName
			&&	Hash->Parent==InParent
			&&	Hash->GetClass()!=InClass )
				debugf(NAME_Log,"CONFLICT: %s - %s",Hash->GetFullName(),InClass->GetName());

	// Allocate the object.
	UObject* Result = AllocateObject( InClass, InParent, InName, InFlags, InTemplate );
	
	// Call the default constructor.
	guard(InternalConstructor);
	if( !InClass->Constructor )
		appErrorf( "Class %s missing C++ constructor", InClass->GetPathName() );
	(*InClass->Constructor)( Result );
	unguard;

	// If autoconfigurable but not scripted, load configuration.
	if( InClass->GetFlags() & RF_Intrinsic )
	{
		check(Result->GetClass());
		Result->LoadConfig( NAME_Config );
		Result->LoadConfig( NAME_Localized );
	}

	//unclock(Time); debugf(NAME_Log,"ConstructObject=%f",GSecondsPerCycle*1000);
	return Result;
	unguard;
}

/*-----------------------------------------------------------------------------
   Garbage collection.
-----------------------------------------------------------------------------*/

//
// Archive for finding unused objects.
//
class FArchiveTagUsed : public FArchive
{
public:
	FArchiveTagUsed()
	: Context( NULL )
	{
		guard(FArchiveTagUsed::FArchiveTagUsed);

		// Tag all objects as unreachable.
		for( FObjectIterator It; It; ++It )
			It->SetFlags( RF_Unreachable | RF_TagGarbage );

		// Tag all names as unreachable.
		for( INT i=0; i<FName::GetMaxNames(); i++ )
			if( FName::GetEntry(i) )
				FName::GetEntry(i)->Flags |= RF_Unreachable;

		unguard;
	}
	void Tag( DWORD KeepFlags )
	{
		guard(FArchiveTagUsed::Tag);

		// Tag all root objects' references.
		*this << GObj.Root;
		for( FObjectIterator It; It; ++It )
		{
			if( (It->GetFlags()&KeepFlags) && (It->GetFlags()&RF_TagGarbage) )
			{
				UObject* Obj = *It;
				*this << Obj;
			}
		}
		unguard;
	}
private:
	FArchive& operator<<( UObject*& Obj )
	{
		guard(FArchiveTagUsed<<Obj);

		guard(CheckValid);
		if( Obj )
			check(Obj->IsValid());
		unguard;

		if( Obj && (Obj->GetFlags() & RF_Unreachable) )
		{
			// Only recurse the first time object is claimed.
			guard(TestReach);
			Obj->ClearFlags( RF_Unreachable | RF_DebugSerialize );

			// Recurse.
			if( Obj->GetFlags() & RF_TagGarbage )
			{
				// Recurse down the object graph.
				UObject* OriginalContext=Context;
				Context = Obj;
				Obj->Serialize( *this );
				if( !(Obj->GetFlags() & RF_DebugSerialize) )
					appErrorf( "%s failed to route Serialize", Obj->GetFullName() );
				Context = OriginalContext;
			}
			else
			{
				// For debugging.
				debugf( NAME_Log, "%s is referenced by %s", Obj->GetFullName(), Context ? Context->GetFullName() : NULL );
			}
			unguardf(( "(%s)", Obj->GetFullName() ));
		}

		return *this;
		unguard;
	}
	FArchive& operator<<( FName& Name )
	{
		guard(FArchiveTagUsed::Name);

		Name.ClearFlags( RF_Unreachable );

		return *this;
		unguard;
	}
	UObject* Context;
};

//
// Purge garbage.
//
void FObjectManager::PurgeGarbage( FOutputDevice* Out )
{
	guard(FObjectManager::PurgeGarbage);
	if( GNoGC )
	{
		debugf( NAME_Log, "Not purging garbage" );
		return;
	}
	debugf( NAME_Log, "Purging garbage" );

	// Dispatch all Destroy messages.
	guard(DispatchDestroys);
	for( INT i=0; i<Objects.Num(); i++ )
	{
		guard(DispatchDestroy);
		if
		(	Objects(i)
		&&	(Objects(i)->GetFlags() & RF_Unreachable)
		&& !(Objects(i)->GetFlags() & RF_Intrinsic) )
		{
			if( Out )
				Out->Logf( NAME_DevGarbage, "Garbage collected object %i: %s", i, Objects(i)->GetFullName() );
			Objects(i)->ConditionalDestroy();
			if( !(Objects(i)->GetFlags()&RF_DebugDestroy) )
				appErrorf( "%s failed to route Destroy", Objects(i)->GetFullName() );
		}
		unguardf(( "(%i: %s)", i, Objects(i)->GetFullName() ));
	}
	unguard;

	// Purge all unreachable objects.
	//warning: Can't use FObjectIterator here because classes may be destroyed before objects.
	guard(DeleteGarbage);
	for( INT i=0; i<Objects.Num(); i++ )
	{
		guard(DeleteObject);
		if
		(	Objects(i)
		&&	(Objects(i)->GetFlags() & RF_Unreachable) 
		&& !(Objects(i)->GetFlags() & RF_Intrinsic) )
		{
			delete Objects(i);
		}
		unguardf(( "(%i)", i ));
	}
	unguard;

	// Purge all unreachable names.
	guard(Names);
	for( INT i=0; i<FName::GetMaxNames(); i++ )
	{
		FNameEntry* Name = FName::GetEntry(i);
		if
		(	(Name)
		&&	(Name->Flags & RF_Unreachable)
		&& !(Name->Flags & RF_Intrinsic  ) )
		{
			if( Out )
				Out->Logf( NAME_DevGarbage, "Garbage collected name %i: %s", i, Name->Name );
			FName::DeleteEntry(i);
		}
	}
	unguard;

	unguard;
}

//
// Delete all unreferenced objects.
//
void FObjectManager::CollectGarbage( FOutputDevice* Out, DWORD KeepFlags )
{
	guard(FObjectManager::CollectGarbage);
	debugf( NAME_Log, "Collecting garbage" );

	// Tag and purge garbage.
	FArchiveTagUsed TagUsedAr;
	TagUsedAr.Tag( KeepFlags );

	// Purge it.
	PurgeGarbage( Out );

	unguard;
}

//
// Returns whether an object is referenced, not counting the
// one reference at Obj. No side effects.
//
UBOOL FObjectManager::IsReferenced( UObject*& Obj, DWORD KeepFlags, UBOOL IgnoreReference )
{
	guard(FObjectManager::RefCount);

	// Remember it.
	UObject* OriginalObj = Obj;
	if( IgnoreReference )
		Obj = NULL;

	// Tag all garbage.
	FArchiveTagUsed TagUsedAr;
	OriginalObj->ClearFlags( RF_TagGarbage );
	TagUsedAr.Tag( KeepFlags );

	// Stick the reference back.
	Obj = OriginalObj;

	// Return whether this is tagged.
	return (Obj->GetFlags() & RF_Unreachable)==0;
	unguard;
}

//
// Attempt to delete an object. Only succeeds if unreferenced.
//
UBOOL FObjectManager::AttemptDelete( UObject*& Obj, DWORD KeepFlags, UBOOL IgnoreReference )
{
	guard(FObjectManager::AttemptDelete);
	if( !(Obj->GetFlags() & RF_Intrinsic) && !IsReferenced( Obj, KeepFlags, IgnoreReference ) )
	{
		PurgeGarbage( GSystem );
		return 1;
	}
	else return 0;
	unguard;
}

/*-----------------------------------------------------------------------------
	Importing and exporting.
-----------------------------------------------------------------------------*/

//
// Import an object from a file.
//
UObject* FObjectManager::ImportObjectFromFile
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	const char*			Filename,
	FFeedbackContext*	Warn
)
{
	guard(FObjectManager::ImportObjectFromFile);

	// Open for reading.
	debugf( NAME_Log, "Importing file: %s", Filename );
	FILE* File = appFopen( Filename, "rb" );
	if( File == NULL )
	{
		Warn->Warnf( LocalizeError("NoFindImport"), Filename );
		return NULL;
	}

	// Get size and allocate data.
	TArray<char> Data;
	appFseek( File, 0, USEEK_END );
	Data.Add( appFtell(File) );
	appFseek( File, 0, USEEK_SET );
	INT Count = appFread( &Data(0), 1, Data.Num(), File );
	appFclose( File );
	if( Count!=Data.Num() )
	{
		Warn->Warnf( LocalizeError("ReadFileFailed"), Filename );
		return NULL;
	}

	// In case importing text, make sure it's null-terminated.
	Data.AddItem(0);

	// Find class factory.
	for( TObjectIterator<UFactory> It; It; ++It )
	{
		if( It->SupportedClass==Class )
		{
			const char* Buffer = &Data(0);
			UObject* Result = It->Create( Class, InParent, Name, NULL, appFExt(Filename), Buffer, Buffer+Count, GSystem );
			if( Result )
			{
				check(Result->IsA(Class));
				return Result;
			}
		}
	}

	// No importer found.
	return NULL;
	unguard;
}

/*-----------------------------------------------------------------------------
	FObjectManager driver support.
-----------------------------------------------------------------------------*/

static char CachedLanguage[32]="";
static TArray<FPreferencesInfo> AllPreferences;
static TArray<FRegistryObjectInfo> AllDrivers;
void CacheDrivers( UBOOL ForceRefresh )
{
	guard(CacheDrivers);
	char Buffer[32767];
	if( ForceRefresh || appStricmp(CachedLanguage,GetLanguage())!=0 )
	{
		appStrcpy( CachedLanguage, GetLanguage() );
		AllPreferences.Empty();
		AllDrivers.Empty();
		for( INT i=0; i<ARRAY_COUNT(GSys->Paths); i++ )
		{
			if( GSys->Paths[i] )
			{
				char Filename[256];
				appSprintf( Filename, "%s%s", appBaseDir(), GSys->Paths[i] );
				char* Tmp = appStrstr( Filename, "*." );
				if( Tmp )
				{
					appStrcpy( Tmp, "*.int" );
					TArray<FString> Files = appFindFiles( Filename );
					for( INT j=0; j<Files.Num(); j++ )
					{
						appSprintf( Tmp, "%s%s", appBaseDir(), *Files(j) );
						if( GetConfigSection( "Public", Buffer, ARRAY_COUNT(Buffer), Tmp ) )
						{
							char* Next;
							for( char* Key=Buffer; *Key; Key=Next )
							{
								Next = Key + appStrlen(Key) + 1;
								char* Value = appStrchr(Key,'=');
								if( Value )
								{
									*Value++ = 0;
									if( *Value=='(' )
										*Value++=0;
									if( *Value && Value[appStrlen(Value)-1]==')' )
										Value[appStrlen(Value)-1]=0;
									if( appStricmp(Key,"Object")==0 )
									{
										FRegistryObjectInfo& Info = *new(AllDrivers)FRegistryObjectInfo;
										Parse( Value, "Name=",  Info.Object, ARRAY_COUNT(Info.Object) );
										Parse( Value, "Class=", Info.Class, ARRAY_COUNT(Info.Class) );
										Parse( Value, "MetaClass=", Info.MetaClass, ARRAY_COUNT(Info.MetaClass) );
										Parse( Value, "Autodetect=", Info.Autodetect, ARRAY_COUNT(Info.Autodetect) );
									}
									else if( appStricmp(Key,"Preferences")==0 )
									{
										FPreferencesInfo& Info = *new(AllPreferences)FPreferencesInfo;
										Parse( Value, "Caption=", Info.Caption, ARRAY_COUNT(Info.Caption) );
										Parse( Value, "Parent=", Info.ParentCaption, ARRAY_COUNT(Info.ParentCaption) );
										Parse( Value, "Class=", Info.Class, ARRAY_COUNT(Info.Class) );
										Parse( Value, "Category=", Info.Category );
										ParseUBOOL( Value, "Immediate=", Info.Immediate );
										Info.Category.SetFlags( RF_Intrinsic );
									}
								}
							}
						}
					}

				}
			}
		}
		TArray<FString> CoreLangs = appFindFiles( "Core.*" );
		for( i=0; i<CoreLangs.Num(); i++ )
		{
			if
			(	!appStrstr(*CoreLangs(i),".dll")
			&&	!appStrstr(*CoreLangs(i),".u")
			&&	!appStrstr(*CoreLangs(i),".ilk") )
			{
				FRegistryObjectInfo& Info = *new(AllDrivers)FRegistryObjectInfo;
				appStrcpy( Info.Object, appStrchr(*CoreLangs(i),'.')+1 );
				appStrcpy( Info.Class, "Class" );
				appStrcpy( Info.MetaClass, ULanguage::StaticClass->GetPathName() );
			}
		}
	}
	unguard;
}

void FObjectManager::GetRegistryObjects
(
	TArray<FRegistryObjectInfo>&	Results,
	UClass*							Class,
	UClass*							MetaClass,
	UBOOL							ForceRefresh
)
{
	guard(FObjectManager::GetDriverClasses);
	check(Class);
	check(Class!=UClass::StaticClass || MetaClass);
	CacheDrivers( ForceRefresh );
	const char* ClassName = Class->GetName();
	const char* MetaClassName = MetaClass ? MetaClass->GetPathName() : "";
	for( INT i=0; i<AllDrivers.Num(); i++ )
		if
		(	appStricmp(AllDrivers(i).Class, ClassName)==0
		&&	appStricmp(AllDrivers(i).MetaClass, MetaClassName)==0 )
			new(Results)FRegistryObjectInfo(AllDrivers(i));
	unguard;
}

void FObjectManager::GetPreferences( TArray<FPreferencesInfo>& Results, const char* ParentCaption, UBOOL ForceRefresh )
{
	guard(FObjectManager::GetPreferences);
	CacheDrivers( ForceRefresh );
	Results.Empty();
	for( INT i=0; i<AllPreferences.Num(); i++ )
		if( appStricmp(AllPreferences(i).ParentCaption,ParentCaption)==0 )
			new(Results)FPreferencesInfo(AllPreferences(i));
	unguard;
}

/*-----------------------------------------------------------------------------
	UTextBuffer implementation.
-----------------------------------------------------------------------------*/

//
// UObject interface.
//
void UTextBuffer::Serialize( FArchive& Ar )
{
	guard(UTextBuffer::Serialize);
	UObject::Serialize(Ar);
	Ar << Pos << Top << Text;
	unguardobj;
}
void UTextBuffer::Export( FOutputDevice& Out, const char* FileType, INT Indent )
{
	guard(UTextBuffer::Export);

	const char* Start = *Text;
	const char* End   = Start + Text.Length();
	while( Start<End && (Start[0]=='\r' || Start[0]=='\n' || Start[0]==' ') )
		Start++;
	while( End>Start && (End [-1]=='\r' || End [-1]=='\n' || End [-1]==' ') )
		End--;

	Out.WriteBinary( Start, End-Start );

	unguardobj;
}
IMPLEMENT_CLASS(UTextBuffer);

//
// FOutputDevice interface.
//

// Write a message.
void UTextBuffer::WriteBinary( const void* Data, int Length, EName MsgType )
{
	guard(UTextBuffer::WriteBinary);
	Text += (char*)Data;
	unguardobj;
}

/*-----------------------------------------------------------------------------
	UEnum implementation.
-----------------------------------------------------------------------------*/

UEnum::UEnum( UEnum* InSuperEnum )
: UField( InSuperEnum )
{}
void UEnum::Serialize( FArchive& Ar )
{
	guard(UEnum::Serialize);
	UField::Serialize(Ar);
	Ar << Names;
	unguardobj;
}
void UEnum::Export( FOutputDevice& Out, const char* FileType, int Indent )
{
	guard(UEnum::Export);
	if( appStricmp(FileType,"H")==0 )
	{
		// C++ header.
		Out.Logf("%senum %s\r\n{\r\n",appSpc(Indent),GetName());
		for( int i=0; i<Names.Num(); i++ )
			Out.Logf( "%s    %-24s=%i,\r\n", appSpc(Indent), *Names(i), i );

		if( appStrchr(*Names(0),'_') )
		{
			// Include tag_MAX enumeration.
			char Temp[256];
			appStrcpy(Temp,*Names(0));
			appStrcpy(appStrchr(Temp,'_'),"_MAX");
			Out.Logf("%s    %-24s=%i,\r\n",appSpc(Indent),Temp,i);
		}
		Out.Logf( "};\r\n\r\n" );
	}
	unguardobj;
}
IMPLEMENT_CLASS(UEnum);

/*-----------------------------------------------------------------------------
	FCompactIndex implementation.
-----------------------------------------------------------------------------*/

//
// FCompactIndex serializer.
//
FArchive& operator<<( FArchive& Ar, FCompactIndex& I )
{
	guard(FCompactIndex<<);
	if( (!Ar.IsLoading() && !Ar.IsSaving()) || Ar.Ver()<=31 )
	{
		Ar << I.Value;
	}
	else
	{
		INT   Original = I.Value;
		DWORD V        = Abs(I.Value);
		BYTE  B0       = ((I.Value>=0) ? 0 : 0x80) + ((V < 0x40) ? V : ((V & 0x3f)+0x40));
		I.Value        = 0;
		Ar << B0;
		if( B0 & 0x40 )
		{
			V >>= 6;
			BYTE B1 = (V < 0x80) ? V : ((V & 0x7f)+0x80);
			Ar << B1;
			if( B1 & 0x80 )
			{
				V >>= 7;
				BYTE B2 = (V < 0x80) ? V : ((V & 0x7f)+0x80);
				Ar << B2;
				if( B2 & 0x80 )
				{
					V >>= 7;
					BYTE B3 = (V < 0x80) ? V : ((V & 0x7f)+0x80);
					Ar << B3;
					if( B3 & 0x80 )
					{
						V >>= 7;
						BYTE B4 = V;
						Ar << B4;
						I.Value = B4;
					}
					I.Value = (I.Value << 7) + (B3 & 0x7f);
				}
				I.Value = (I.Value << 7) + (B2 & 0x7f);
			}
			I.Value = (I.Value << 7) + (B1 & 0x7f);
		}
		I.Value = (I.Value << 6) + (B0 & 0x3f);
		if( B0 & 0x80 )
			I.Value = -I.Value;
		if( Ar.IsSaving() && I.Value!=Original )
			appErrorf("Mismatch: %08X %08X",I.Value,Original);
	}
	return Ar;
	unguard;
}

/*-----------------------------------------------------------------------------
	FPackageInfo implementation.
-----------------------------------------------------------------------------*/

//
// FPackageInfo constructor.
//
FPackageInfo::FPackageInfo( ULinkerLoad* InLinker )
:	Linker		( InLinker )
,	Parent		( InLinker ? InLinker->LinkerRoot : NULL )
,	Guid		( InLinker && InLinker->Heritage.Num() ? InLinker->Heritage( InLinker->Heritage.Num()-1) : FGuid(0,0,0,0) )
,	FileSize	( InLinker ? InLinker->FileSize : 0 )
,	PackageFlags( InLinker ? InLinker->Summary.PackageFlags : 0 )
,	ObjectBase  ( 0 )
,	NameBase	( 0 )
{
	guard(FPackageInfo::FPackageInfo);
	*URL = 0;
	if( InLinker && InLinker->Filename && (InLinker->Summary.PackageFlags & PKG_AllowDownload) )
		appStrcpy( URL, InLinker->Filename );
	unguard;
}

//
// FPackageInfo serializer.
//
CORE_API FArchive& operator<<( FArchive& Ar, FPackageInfo& I )
{
	guard(FPackageInfo<<);
	return Ar << I.Parent << I.Linker;
	unguard;
}

/*-----------------------------------------------------------------------------
	FPackageMap implementation.
-----------------------------------------------------------------------------*/

//
// Construct a linker map from a top-level object.
//
void FPackageMap::Init( UObject* InRoot )
{
	guard(FPackageMap::FPackageMap);
	Empty();
	if( InRoot->GetLinker() )
		AddLinker( InRoot->GetLinker() );
	unguard;
}

//
// Add a linker to this linker map.
//
void FPackageMap::AddLinker( ULinkerLoad* Linker )
{
	guard(FPackageMap::AddLinker);

	// Skip if already on list.
	for( INT i=0; i<Num(); i++ )
		if( (*this)(i).Parent == Linker->LinkerRoot )
			return;

	// Add to list and recurse.
	FPackageInfo* Info = new(*this)FPackageInfo( Linker );
	for( i=0; i<Linker->ImportMap.Num(); i++ )
		if( Linker->ImportMap(i).ClassName==NAME_Package && Linker->ImportMap(i).PackageIndex==0 )
			for( INT j=0; j<GObj.Loaders.Num(); j++ )
				if( GObj.GetLoader(j)->LinkerRoot->GetFName()==Linker->ImportMap(i).ObjectName )
					AddLinker( GObj.GetLoader(j) );

	unguard;
}

//
// Compute mapping info.
//
void FPackageMap::Compute()
{
	guard(FPackageInfo::Compute);
	for( INT i=0; i<Num(); i++ )
	{
		if( (*this)(i).Linker==NULL )
			(*this)(i).Linker = GObj.GetPackageLinker( (*this)(i).Parent, NULL, LOAD_NoFail | LOAD_KeepImports, NULL, &(*this)(i).Guid );
	}
	NameIndices.Add( FName::GetMaxNames() );
	for( i=0; i<NameIndices.Num(); i++ )
	{
		NameIndices(i) = -1;
	}
	for( i=0; i<Num(); i++ )
	{
		(*this)(i).ObjectBase = (*this)(i).NameBase = 0;
		if( i>0 )
		{
			(*this)(i).ObjectBase = (*this)(i-1).ObjectBase + (*this)(i-1).Linker->ExportMap.Num();
			(*this)(i).NameBase   = (*this)(i-1).NameBase   + (*this)(i-1).Linker->NameMap.Num();
		}
		for( INT j=0; j<(*this)(i).Linker->NameMap.Num(); j++ )
			if( NameIndices((*this)(i).Linker->NameMap(j).GetIndex()) == -1 )
				NameIndices((*this)(i).Linker->NameMap(j).GetIndex()) = (*this)(i).NameBase + j;
	}
	unguard;
}

//
// Mapping functions.
//
INT FPackageMap::NameToIndex( FName Name )
{
	guard(FPackageMap::NameToIndex);
	DOUBLE d=appSeconds();

	// Intrinsic names.
	if( Name.GetFlags() & RF_Intrinsic )
		return -Name.GetIndex()-1;

	// Normal names.
	if( Name.GetIndex() < NameIndices.Num() )
		return NameIndices( Name.GetIndex() );

	// Untranslatable.
	return -1;
	unguard;
}
INT FPackageMap::ObjectToIndex( UObject* Object )
{
	guard(FPackageMap::ObjectToIndex);
	if( Object && Object->GetLinker() && Object->GetLinkerIndex()!=INDEX_NONE )
		for( INT i=0; i<Num(); i++ )
			if( (*this)(i).Linker == Object->GetLinker() )
				return (*this)(i).ObjectBase + Object->GetLinkerIndex();
	return -1;
	unguard;
}
FName FPackageMap::IndexToName( INT Index )
{
	guard(FPackageMap::PairToName);
	if( Index<0 )
	{
		// Intrinsic name.
		Index = -Index-1;
		if
		(	(Index<FName::GetMaxNames())
		&&	(FName::GetEntry(Index))
		&&	(FName::GetEntry(Index)->Flags & RF_Intrinsic) )
			return FName((EName)Index);
	}
	else
	{
		// Dynamic name.
		for( INT i=0; i<Num(); i++ )
		{
			INT Count = (*this)(i).Linker->NameMap.Num();
			if( Index < Count )
				return (*this)(i).Linker->NameMap(Index);
			Index -= Count;
			debug(Index>=0);
		}
	}
	return NAME_None;
	unguard;
}
UObject* FPackageMap::IndexToObject( INT Index, UBOOL Load )
{
	guard(FPackageMap::PairToObject);
	if( Index>=0 )
	{
		for( INT i=0; i<Num(); i++ )
		{
			INT NumExports = (*this)(i).Linker->ExportMap.Num();
			if( Index < NumExports )
			{
				UObject* Result = (*this)(i).Linker->ExportMap(Index)._Object;
				if( !Result && Load )
				{
					GObj.BeginLoad();
					Result = (*this)(i).Linker->CreateExport( Index );
					GObj.EndLoad();
				}
				return Result;
			}
			Index -= NumExports;
		}
	}
	return NULL;
	unguard;
}

/*-----------------------------------------------------------------------------
	ULinker related implementations.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(ULinker);
IMPLEMENT_CLASS(ULinkerLoad);
IMPLEMENT_CLASS(ULinkerSave);

/*-----------------------------------------------------------------------------
	USubsystem.
-----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(USubsystem);

/*-----------------------------------------------------------------------------
	UPackage.
-----------------------------------------------------------------------------*/

UPackage::UPackage()
{
	guard(UPackage::UPackage);

	// Bind to a matching DLL, if any.
	Bind();

	unguard;
}
void UPackage::Bind()
{
	guard(UPackage::Bind);
	if( !DllHandle )
	{
		DllHandle = appGetDllHandle( GetName() );
		if( DllHandle )
			debugf( NAME_Log, "Bound to %s.dll", GetName() );
	}
	unguard;
}
void UPackage::Serialize( FArchive& Ar )
{
	guard(UPackage::Serialize);
	UObject::Serialize( Ar );
	SetFlags( RF_Public );//oldver
	unguard;
}
void UPackage::Destroy()
{
	guard(UPackage::Destroy);
	if( DllHandle )
	{
		debugf( NAME_Log, "Unbound to %s.dll", GetName() );
		//Danger: If package is garbage collected before its autoregistered classes,
		//the autoregistered classes will be unmapped from memory.
		//GSystem->FreeDllHandle( DllHandle );
	}
	UObject::Destroy();
	unguard;
}
void* UPackage::GetDllExport( const char* ExportName, UBOOL Checked )
{
	guard(UPackage::GetDllExport);
	void* Result;
	if( !DllHandle )
	{
		if( Checked && !ParseParam(appCmdLine(),"nobind") )
			appErrorf( LocalizeError("NotDll"), GetName(), ExportName );
		Result = NULL;
	}
	else
	{
		Result = appGetDllExport( DllHandle, ExportName );
		if( !Result && Checked && !ParseParam(appCmdLine(),"nobind") )
			appErrorf( LocalizeError("NotInDll"), ExportName, GetName() );
		if( Result )
			debugf( NAME_DevBind, "Found %s in %s.dll", ExportName, GetName() );
	}
	return Result;
	unguard;
}
IMPLEMENT_CLASS(UPackage);

/*----------------------------------------------------------------------------
	UFactory.
----------------------------------------------------------------------------*/

UFactory::UFactory()
{}
IMPLEMENT_CLASS(UFactory);

/*-----------------------------------------------------------------------------
	UTextBufferFactory.
-----------------------------------------------------------------------------*/

UTextBufferFactory::UTextBufferFactory()
{
	guard(UTextBufferFactory::UTextBufferFactory);

	// Init UFactory properties.
	SupportedClass = UTextBuffer::StaticClass;
	new(Formats)FString("txt;Text files");
	bCreateNew = 0;

	unguard;
}
UObject* UTextBufferFactory::Create
(
	UClass*				Class,
	UObject*			InParent,
	FName				Name,
	UObject*			Context,
	const char*			Type,
	const char*&		Buffer,
	const char*			BufferEnd,
	FFeedbackContext*	Warn
)
{
	guard(UTextBufferFactory::Create);

	// Import.
	UTextBuffer* Result = new UTextBuffer;
	Result->Text = Buffer;
	return Result;

	unguard;
}
IMPLEMENT_CLASS(UTextBufferFactory);

/*----------------------------------------------------------------------------
	UExporter.
----------------------------------------------------------------------------*/

UExporter::UExporter()
{}
IMPLEMENT_CLASS(UExporter);

/*----------------------------------------------------------------------------
	ULanguage.
----------------------------------------------------------------------------*/

IMPLEMENT_CLASS(ULanguage);

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
