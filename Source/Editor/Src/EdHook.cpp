/*=============================================================================
	EdHook.cpp: UnrealEd VB hooks.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

Revision history:
	* Created by Tim Sweeney.
=============================================================================*/

// Includes.
#pragma warning( disable : 4201 )
#pragma warning( disable : 4310 )
#define STRICT
#include <windows.h>
#include <commctrl.h>
#include "EditorPrivate.h"
#include "Window.h"
#include <process.h>

// Thread exchange.
HANDLE      hEngineThreadStarted;
HANDLE      hEngineThread;
HWND        hWndEngine;
DWORD       EngineThreadId;
FStringOut  GetPropResult;
const char* GTopic;
const char* GItem;
const char* GValue;

// Messages.
#define WM_USER_EXEC    (WM_USER+0)
#define WM_USER_SETPROP (WM_USER+1)
#define WM_USER_GETPROP (WM_USER+2)

// Misc.
extern CORE_API FGlobalPlatform GTempPlatform;
extern CORE_API DWORD hWndCallback, hWndMain;
UEngine* Engine;

/*-----------------------------------------------------------------------------
	Editor hook exec.
-----------------------------------------------------------------------------*/

void UEditorEngine::NotifyDestroy( void* Src )
{
	guard(UEditorEngine::NotifyDestroy);
	if( Src==ActorProperties )
		ActorProperties = NULL;
	if( Src==LevelProperties )
		LevelProperties = NULL;
	if( Src==Preferences )
		Preferences = NULL;
	if( Src==UseDest )
		UseDest = NULL;
	unguard;
}
void UEditorEngine::NotifyPreChange( void* Src )
{
	guard(UEditorEngine::NotifyPreChange);
	Trans->Begin( Level, "Edit Properties" );
	unguard;
}
void UEditorEngine::NotifyPostChange( void* Src )
{
	guard(UEditorEngine::NotifyPostChange);
	Trans->End();
	if( Src==Preferences )
	{
		GCache.Flush();
		for( TObjectIterator<UViewport> It; It; ++It )
			It->Actor->FovAngle = FovAngle;
	}
	RedrawLevel( Level );
	unguard;
}
AUTOREGISTER_TOPIC("Obj",ObjTopicHandler);
void ObjTopicHandler::Get(ULevel *Level, const char *Item, FOutputDevice &Out)
{
	guard(ObjTopicHandler::Get);
	if( ParseCommand(&Item,"QUERY") )
	{
		UClass* Class;
		if( ParseObject<UClass>(Item,"TYPE=",Class,ANY_PACKAGE) )
		{
			UPackage* BasePackage;
			UPackage* RealPackage;
			TArray<UObject*> Results;
			if( !ParseObject<UPackage>( Item, "PACKAGE=", BasePackage, NULL ) )
			{
				// Objects in any package.
				for( FObjectIterator It; It; ++It )
					if( It->IsA(Class) )
						Results.AddItem( *It );
			}
			else if( !ParseObject<UPackage>( Item, "GROUP=", RealPackage, BasePackage ) )
			{
				// All objects beneath BasePackage.
				for( FObjectIterator It; It; ++It )
					if( It->IsA(Class) && It->IsIn(BasePackage) )
						Results.AddItem( *It );
			}
			else
			{
				// All objects within RealPackage.
				for( FObjectIterator It; It; ++It )
					if( It->IsA(Class) && It->IsIn(RealPackage) )
						Results.AddItem( *It );
			}
			for( INT i=0; i<Results.Num(); i++ )
			{
				if( i )
					Out.Log( " " );
				Out.Log( Results(i)->GetName() );
			}
		}
	}
	else if( ParseCommand(&Item,"PACKAGES") )
	{
		UClass* Class;
		if( ParseObject<UClass>(Item,"CLASS=",Class,ANY_PACKAGE) )
		{
			TArray<UObject*> List;
			for( FObjectIterator It; It; ++It )
			{
				if( It->IsA(Class) && It->GetParent()!=GObj.GetTransientPackage() )
				{
					check(It->GetParent());
					for( UObject* TopParent=It->GetParent(); TopParent->GetParent()!=NULL; TopParent=TopParent->GetParent() );
					if( TopParent->IsA(UPackage::StaticClass) )
						List.AddUniqueItem( TopParent );
				}
			}
			for( INT i=0; i<List.Num(); i++ )
			{
				if( i )
					Out.Log( "," );
				Out.Log( List(i)->GetName() );
			}
		}
	}
	else if( ParseCommand(&Item,"DELETE") )
	{
		UPackage* Pkg=ANY_PACKAGE;
		UClass*   Class;
		UObject*  Object;
		ParseObject<UPackage>( Item, "PACKAGE=", Pkg, NULL );
		if
		(	!ParseObject<UClass>( Item,"CLASS=", Class, ANY_PACKAGE )
		||	!ParseObject(Item,"OBJECT=",Class,Object,Pkg) )
			Out.Logf( "Object not found" );
		else if( GObj.IsReferenced( Object, RF_Intrinsic | RF_Public, 0 ) )
			Out.Logf( "%s is in use", Object->GetFullName() );
		else delete Object;
	}
	else if( ParseCommand(&Item,"GROUPS") )
	{
		UClass* Class;
		UPackage* Pkg;
		if
		(	ParseObject<UPackage>(Item,"PACKAGE=",Pkg,NULL)
		&&	ParseObject<UClass>(Item,"CLASS=",Class,ANY_PACKAGE) )
		{
			TArray<UObject*> List;
			for( FObjectIterator It; It; ++It )
				if( It->IsA(Class) && It->GetParent() && It->GetParent()->GetParent()==Pkg )
					List.AddUniqueItem( It->GetParent() );
			for( INT i=0; i<List.Num(); i++ )
			{
				if( i )
					Out.Log( "," );
				Out.Log( List(i)->GetName() );
			}
		}
	}
	else if( ParseCommand(&Item,"BROWSECLASS") )
	{
		Out.Log( GEditor->BrowseClass->GetName() );
	}
	unguard;
}
void ObjTopicHandler::Set( ULevel* Level, const char* Item, const char* Data )
{
	guard(ObjTopicHandler::Set);
	if( ParseCommand(&Item,"NOTECURRENT") )
	{
		UClass* Class;
		UObject* Object;
		if
		(	GEditor->UseDest
		&&	ParseObject<UClass>( Data, "CLASS=", Class, ANY_PACKAGE )
		&&	ParseObject( Data, "OBJECT=", Class, Object, ANY_PACKAGE ) )
		{
			char Temp[256];
			appSprintf( Temp, "%s'%s'", Object->GetClassName(), Object->GetName() );
			GEditor->UseDest->SetValue( Temp );
		}
	}
	unguard;
}
void UEditorEngine::NotifyExec( void* Src, const char* Cmd )
{
	guard(UEditorEngine::NotifyExec);
	if( ParseCommand(&Cmd,"BROWSECLASS") )
	{
		ParseObject( Cmd, "CLASS=", BrowseClass, ANY_PACKAGE );
		UseDest = (WProperties*)Src;
		EdCallback( EDC_Browse, 1 );
	}
	else if( ParseCommand(&Cmd,"USECURRENT") )
	{
		ParseObject( Cmd, "CLASS=", BrowseClass, ANY_PACKAGE );
		UseDest = (WProperties*)Src;
		EdCallback( EDC_UseCurrent, 1 );
	}
	unguard;
}
void UEditorEngine::UpdatePropertiesWindows()
{
	guard(UEditorEngine::UpdatePropertiesWindow);
	if( ActorProperties )
	{
		TArray<UObject*> SelectedActors;
		for( INT i=0; i<Level->Num(); i++ )
			if( Level->Element(i) && Level->Element(i)->bSelected )
				SelectedActors.AddItem( Level->Element(i) );
		ActorProperties->Root.SetObjects( &SelectedActors(0), SelectedActors.Num() );
	}
	for( INT i=0; i<WProperties::PropertiesWindows.Num(); i++ )
	{
		WProperties* Properties=WProperties::PropertiesWindows(i);
		if( Properties!=ActorProperties && Properties!=Preferences )
			Properties->ForceRefresh();
	}
	unguard;
}
UBOOL UEditorEngine::HookExec( const char* Cmd, FOutputDevice* Out )
{
	guard(UEditorEngine::HookExec);
	if( ParseCommand(&Cmd,"PLAYMAP") )
	{
		char Parms[256];
		Exec( "MAP SAVE FILE=..\\Maps\\Autoplay.unr", Out );
		appSprintf( Parms, "Autoplay.unr HWND=%i %s", (INT)hWndMain, GameCommandLine );
		appLaunchURL( "Unreal", Parms );
		return 1;
	}
	else if( ParseCommand(&Cmd,"ACTORPROPERTIES") )
	{
		if( !ActorProperties )
		{
			ActorProperties = new WObjectProperties( "ActorProperties", CPF_Edit, "" );
			ActorProperties->OpenWindow( (HWND)hWndMain );
			ActorProperties->SetNotifyHook( GEditor );
		}
		UpdatePropertiesWindows();
		ShowWindow( *ActorProperties, SW_SHOWNORMAL );
		return 1;
	}
	else if( ParseCommand(&Cmd,"PREFERENCES") )
	{
		if( !Preferences )
		{
			Preferences = new WConfigProperties( "Preferences", "Advanced Options" );
			Preferences->OpenWindow( (HWND)hWndMain );
			Preferences->SetNotifyHook( GEditor );
			Preferences->ForceRefresh();
		}
		ShowWindow( *Preferences, SW_SHOWNORMAL );
		return 1;
	}
	else if( ParseCommand(&Cmd,"LEVELPROPERTIES") )
	{
		if( !LevelProperties )
		{
			LevelProperties = new WObjectProperties( "LevelProperties", CPF_Edit, "Level Properties" );
			LevelProperties->OpenWindow( (HWND)hWndMain );
			LevelProperties->SetNotifyHook( GEditor );
		}
		LevelProperties->Root.SetObjects( (UObject**)&Level->Actors(0), 1 );
		ShowWindow( *LevelProperties, SW_SHOWNORMAL );
		return 1;
	}
	else if( ParseCommand(&Cmd,"TEXTUREPROPERTIES") )
	{
		UTexture* Texture;
		if( ParseObject<UTexture>( Cmd, "TEXTURE=", Texture, ANY_PACKAGE ) )
		{
			char Title[256];
			appSprintf( Title, "Texture %s", Texture->GetPathName() );
			WObjectProperties* TextureProperties = new WObjectProperties( "TextureProperties", CPF_Edit, Title );
			TextureProperties->OpenWindow( (HWND)hWndMain );
			TextureProperties->Root.SetObjects( (UObject**)&Texture, 1 );
			TextureProperties->SetNotifyHook( GEditor );
			ShowWindow( *TextureProperties, SW_SHOWNORMAL );
		}
		return 1;
	}
	else if( ParseCommand(&Cmd,"CLASSPROPERTIES") )
	{
		UClass* Class;
		if( ParseObject<UClass>( Cmd, "Class=", Class, ANY_PACKAGE ) )
		{
			char Title[256];
			appSprintf( Title, "Default %s Properties", Class->GetPathName() );
			WClassProperties* ClassProperties = new WClassProperties( "ClassProperties", CPF_Edit, Title, Class );
			ClassProperties->OpenWindow( (HWND)hWndMain );
			ClassProperties->SetNotifyHook( GEditor );
			ClassProperties->ForceRefresh();
			ShowWindow( *ClassProperties, SW_SHOWNORMAL );
		}
		return 1;
	}
	return 0;
	unguard;
}

/*-----------------------------------------------------------------------------
	UnrealEd startup and VB hooks.
-----------------------------------------------------------------------------*/

LONG APIENTRY VbWindowProc( HWND hWnd, UINT Message, UINT wParam, LONG lParam )
{
	guard(VbWindowProc);
	if( Message==WM_USER_EXEC&& GEditor)
	{
		char* Cmd = (char*)wParam;
		GEditor->Exec( Cmd );
		free( Cmd );
		return 0;
	}
	else if( Message==WM_USER_GETPROP && GEditor)
	{
		GetPropResult = FStringOut();
		GEditor->Get( GTopic, GItem, GetPropResult );
		return 0;
	}
	else if( Message==WM_USER_SETPROP && GEditor)
	{
		GEditor->Set( GTopic, GItem, GValue );
		return 0;
	}
	else return DefWindowProc( hWnd, Message, wParam, lParam );
	unguard;
}

DWORD __stdcall ThreadEntry( void* Arg )
{
	try
	{
		// Create this thread's window.
		GIsStarted = 1;
		appChdir( appBaseDir() );
		InitWindowing();
		hWndEngine = CreateWindow( "VbWindowProc", "VbWindowProc", 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL );
		check(hWndEngine);
		SetEvent( hEngineThreadStarted );

		// Set mode.
		GIsServer = 1;
		GIsClient = 1;
		GIsEditor = 1;

		// Init log window.
		GExecHook = GThisExecHook;
		GLog = new WLog( "Log" );
		GLog->OpenWindow( NULL, 1 );

		// Init.
		GIsGuarded=1;
		GSystem = &GTempPlatform;
		Engine = InitEngine();

		// Message pump.
		MainLoop( Engine );

		// Exit.
		ExitEngine( Engine );
		GExecHook=NULL;
		if( GLog )
			delete GLog;
		GIsGuarded=0;
	}
	catch( ... )
	{
		HandleError();
	}
	appExit();
	GIsStarted = 0;
	return 0;
}

void UEditorEngine::EdCallback( DWORD Code, UBOOL Send )
{
	guard(FGlobalPlatform::EdCallback);
	if( hWndCallback )
	{
		if( Send ) SendMessage( (HWND)hWndCallback, WM_CHAR, 32+Code, 0 );
		else       PostMessage( (HWND)hWndCallback, WM_CHAR, 32+Code, 0 );
	}
	unguard;
}

#define EDHOOK_TRY \
	try {

#define EDHOOK_CATCH \
	} catch( ... ) { \
		HandleError(); \
		appForceExit(); \
	}

extern "C"
{
	DLL_EXPORT void __stdcall EdInitServer( HWND hInWndMain, HWND hInWndCallback )
	{
		EDHOOK_TRY;

		hWndCallback = (DWORD)hInWndCallback;
		hWndMain     = (DWORD)hInWndMain;

		WNDCLASS Cls;
		memset( &Cls, 0, sizeof(Cls) );
		Cls.lpfnWndProc   = VbWindowProc;
		Cls.hInstance     = hInstance;
		Cls.lpszClassName = "VbWindowProc"; 
		verify(RegisterClass( &Cls ));

		hEngineThreadStarted = CreateEvent( NULL, 0, 0, NULL );
		hEngineThread        = CreateThread( NULL, 0, ThreadEntry, NULL, 0, &EngineThreadId );
		check(EngineThreadId!=-1);
		WaitForSingleObject( hEngineThreadStarted, INFINITE );
	
		EDHOOK_CATCH;
	}
	DLL_EXPORT void __stdcall EdExitServer()
	{
		EDHOOK_TRY;
		verify(PostThreadMessage( EngineThreadId, WM_QUIT, 0, 0 ));
		WaitForSingleObject( hEngineThread, INFINITE );
		EDHOOK_CATCH;
	}
	DLL_EXPORT void __stdcall EdExec( const char* Cmd )
	{
		EDHOOK_TRY;
		SendMessage( hWndEngine, WM_USER_EXEC, (WPARAM)strdup(Cmd), 0 );
		EDHOOK_CATCH;
	}
	DLL_EXPORT void __stdcall EdSetProp( const char* Topic, const char* Item, const char* Value )
	{
		EDHOOK_TRY;
		GTopic = Topic;
		GItem  = Item;
		GValue = Value;
		SendMessage( hWndEngine, WM_USER_SETPROP, 0, 0 );
		EDHOOK_CATCH;
	}
	DLL_EXPORT BSTR __stdcall EdGetProp( const char* Topic, const char* Item )
	{
		EDHOOK_TRY;
		GTopic = Topic;
		GItem  = Item;
		SendMessage( hWndEngine, WM_USER_GETPROP, 0, 0 );
		return SysAllocStringByteLen( *GetPropResult, GetPropResult.Length() );
		EDHOOK_CATCH
	}
}

/*-----------------------------------------------------------------------------
	The end.
-----------------------------------------------------------------------------*/
