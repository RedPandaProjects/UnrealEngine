/*=============================================================================
	UnViewport.h: Unreal viewport object.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	UCanvas.
-----------------------------------------------------------------------------*/

//
// A drawing canvas.
//
class ENGINE_API UCanvas : public UObject
{
	DECLARE_CLASS_WITHOUT_CONSTRUCT(UCanvas,UObject,CLASS_Transient)
	NO_DEFAULT_CONSTRUCTOR(UCanvas)

	// Variables.
	UFont*			Font;
	FLOAT			SpaceX, SpaceY;
	FLOAT			OrgX, OrgY;
	FLOAT			ClipX, ClipY;
	FLOAT			CurX, CurY;
	FLOAT			Z;
	BYTE			Style;
	FLOAT			CurYL;
	FColor			Color;
	DWORD			bCenter:1;
	DWORD			bNoSmooth:1;
	INT				X, Y;
	UFont			*SmallFont, *MedFont, *BigFont, *LargeFont;
	UViewport*		Viewport;
	struct FSceneNode* Frame;

	// UCanvas interface.
	virtual void Init( UViewport* InViewport );
	virtual void Update( FSceneNode* Frame );
	virtual void StrLen( UFont* Font, INT& XL, INT& YL, const char* Text, INT iStart=0, INT NumChars=MAXINT );
	virtual void WrappedStrLen( UFont* Font, INT& XL, INT& YL, INT Width, const char* Text );
	virtual void VARARGS Printf( UFont* Font, INT X, INT Y, const char* Fmt, ... );
	virtual void VARARGS WrappedPrintf( UFont* Font, UBOOL Center, const char* Fmt, ... );
	virtual void DrawTile( UTexture* Texture, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, class FSpanBuffer* SpanBuffer, FLOAT Z, FPlane Color, FPlane Fog, DWORD PolyFlags );
	virtual void DrawIcon( UTexture* Texture, FLOAT ScreenX, FLOAT ScreenY, FLOAT XSize, FLOAT YSize, class FSpanBuffer* SpanBuffer, FLOAT Z, FPlane Color, FPlane Fog, DWORD PolyFlags );
	virtual void DrawPattern( UTexture* Texture, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT Scale, FLOAT OrgX, FLOAT OrgY, class FSpanBuffer* SpanBuffer, FLOAT Z, FPlane Color, FPlane Fog, DWORD PolyFlags );

	// Intrinsics.
	INTRINSIC(execDrawText)
	INTRINSIC(execDrawTile)
};

/*-----------------------------------------------------------------------------
	FHitCause.
-----------------------------------------------------------------------------*/

//
// Information about a hit-test cause.
//
struct ENGINE_API FHitCause
{
	class FHitObserver* Observer;
	UViewport* Viewport;
	DWORD Buttons;
	FLOAT MouseX;
	FLOAT MouseY;
	FHitCause( FHitObserver* InObserver, UViewport* InViewport, DWORD InButtons, FLOAT InMouseX, FLOAT InMouseY )
	: Observer( InObserver ), Viewport( InViewport ), Buttons( InButtons ), MouseX( InMouseX ), MouseY( InMouseY )
	{}
};

/*-----------------------------------------------------------------------------
	FHitObserver.
-----------------------------------------------------------------------------*/

//
// Hit observer class for receiving hit-test notification.
//
class ENGINE_API FHitObserver
{
public:
	virtual void Click( const struct FHitCause& Cause, const struct HHitProxy& Hit ) {}
};

/*-----------------------------------------------------------------------------
	UViewport.
-----------------------------------------------------------------------------*/

// Information for rendering the viewport (detail level settings).
enum ERenderType
{
	REN_None			= 0,	// Hide completely.
	REN_Wire			= 1,	// Wireframe of EdPolys.
	REN_Zones			= 2,	// Show zones and zone portals.
	REN_Polys			= 3,	// Flat-shaded Bsp.
	REN_PolyCuts		= 4,	// Flat-shaded Bsp with normals displayed.
	REN_DynLight		= 5,	// Illuminated texture mapping.
	REN_PlainTex		= 6,	// Plain texture mapping.
	REN_OrthXY			= 13,	// Orthogonal overhead (XY) view.
	REN_OrthXZ			= 14,	// Orthogonal XZ view.
	REN_OrthYZ			= 15,	// Orthogonal YZ view.
	REN_TexView			= 16,	// Viewing a texture (no actor).
	REN_TexBrowser		= 17,	// Viewing a texture browser (no actor).
	REN_MeshView		= 18,	// Viewing a mesh.
	REN_MAX				= 20
};

// Viewport capabilities.
enum EViewportCaps
{
	CC_RGB565			= 1,	// RGB 565 format (otherwise 555).
	CC_Mask				= 0xff, // Caps mask which affects cached rendering info.
};

// ShowFlags for viewport.
enum EViewportShowFlags
{
	SHOW_Frame     		= 0x00000001, 	// Show world bounding cube.
	SHOW_ActorRadii		= 0x00000002, 	// Show actor collision radii.
	SHOW_Backdrop  		= 0x00000004, 	// Show background scene.
	SHOW_Actors    		= 0x00000008,	// Show actors.
	SHOW_Coords    		= 0x00000010,	// Show brush/actor coords.
	SHOW_ActorIcons		= 0x00000020,	// Show actors as icons.
	SHOW_Brush			= 0x00000040,	// Show the active brush.
	SHOW_StandardView	= 0x00000080,	// Viewport is a standard view.
	SHOW_Menu			= 0x00000100,	// Show menu on viewport.
	SHOW_ChildWindow	= 0x00000200,	// Show as true child window.
	SHOW_MovingBrushes	= 0x00000400,	// Show moving brushes.
	SHOW_PlayerCtrl		= 0x00000800,	// Player controls are on.
	SHOW_Paths          = 0x00001000,   // Show paths.
	SHOW_NoButtons		= 0x00002000,	// No menu/view buttons.
	SHOW_RealTime		= 0x00004000,	// Update window in realtime.
};

// Mouse buttons and commands.
enum EMouseButtons	
{
	MOUSE_Left			= 0x0001,		// Left mouse button.
	MOUSE_Right			= 0x0002,		// Right mouse button.
	MOUSE_Middle 		= 0x0004,		// Middle mouse button.
	MOUSE_FirstHit		= 0x0008,		// Sent when a mouse button is initially hit.
	MOUSE_LastRelease	= 0x0010,		// Sent when last mouse button is released.
	MOUSE_SetMode		= 0x0020,		// Called when a new viewport mode is first set.
	MOUSE_ExitMode		= 0x0040,		// Called when the existing mode is changed.
	MOUSE_Ctrl			= 0x0080,		// Ctrl is pressed.
	MOUSE_Shift			= 0x0100,		// Shift is pressed.
	MOUSE_Alt			= 0x0200,		// Alt is pressed.
	MOUSE_LeftDouble	= 0x0200,		// Left double click.
};

//
// A viewport object, which associates an actor (which defines
// most view parameters) with a Windows window.
//
class ENGINE_API UViewport : public UPlayer
{
	DECLARE_ABSTRACT_CLASS(UViewport,UPlayer,CLASS_Transient)
	NO_DEFAULT_CONSTRUCTOR(UViewport)

	// Referenced objects.
	class UClient*		 Client;	// Client that owns the viewport.
	class UCanvas*		 Canvas;	// Viewport's painting canvas.
	class UInput*		 Input;		// Input system.
	class URenderDevice* RenDev;	// Render device.

	// Normal variables.
	UObject*		MiscRes;		// Used in in modes like EM_TEXVIEW.
	FName			Group;			// Group for editing.
	DOUBLE			LastUpdateTime;	// Time of last update.
	INT				SizeX, SizeY;   // Buffer X & Y resolutions.
	INT				ColorBytes;		// 1=256-color, 4=32-bit color.
	INT				FrameCount;		// Frame count, incremented when locked.
	DWORD			Caps;			// Capabilities (CC_).
	UBOOL			OnHold;			// 1=on hold, can't be resized.
	UBOOL			Current;		// If this is the current input viewport.
	UBOOL			Dragging;		// Dragging mouse.
	DWORD			RenderFlags;	// Render locking flags (only when locked).
	DWORD			ExtraPolyFlags;	// Additional poly flags associated with camera.

	// Level traveling.
	ETravelType		TravelType;
	FString			TravelURL;

	// Frame buffer info; only valid when locked.
	DOUBLE			CurrentTime;	// Time when initially locked.
	BYTE*			ScreenPointer;	// Pointer to screen frame buffer, or NULL if none.
	INT				Stride;			// Stride in pixels.

	// Hit testing.
	UBOOL			HitTesting;		// Whether hit-testing.
	INT				HitX, HitY;		// Hit rectangle top left.
	INT				HitXL, HitYL;	// Hit size.
	TArray<INT>		HitSizes;		// Currently pushed hit sizes.

	// Saved-actor parameters.
	FLOAT SavedOrthoZoom, SavedFovAngle;
	INT SavedShowFlags, SavedRendMap, SavedMisc1, SavedMisc2;

	// Constructor.
	UViewport( ULevel* InLevel, UClient* InClient );

	// UObject interface.
	void Destroy();
	void Serialize( FArchive& Ar );

	// FOutputDevice interface.
	void WriteBinary( const void* Data, int Length, EName MsgType=NAME_Log );

	// UPlayer interface.
	void ReadInput( FLOAT DeltaSeconds );

	// UViewport interface.
	virtual UBOOL SetDrag( UBOOL NewDrag );
	virtual UBOOL Lock( FPlane FlashScale, FPlane FlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* HitData=NULL, INT* HitSize=NULL );
	virtual void Unlock( UBOOL Blit=0 );
	virtual void SetModeCursor()=0;
	virtual void UpdateWindow()=0;
	virtual void OpenWindow( DWORD ParentWindow, UBOOL Temporary, INT NewX, INT NewY, INT OpenX, INT OpenY )=0;
	virtual void CloseWindow()=0;
	virtual void UpdateInput( UBOOL Reset )=0;
	virtual void MakeCurrent()=0;
	virtual void* GetWindow()=0;
	virtual void SetMouseCapture( UBOOL Capture, UBOOL Clip, UBOOL FocusOnly=0 )=0;
	virtual void Repaint()=0;
	virtual void MakeFullscreen( INT NewX, INT NewY, UBOOL SaveConfig )=0;
	virtual UBOOL Exec( const char* Cmd, FOutputDevice* Out=GSystem );

	// Functions.
	void ExecuteHits( const FHitCause& Cause, BYTE* HitData, INT HitSize );
	void PushHit( const struct HHitProxy& Hit, INT Size );
	void PopHit( UBOOL bForce );
	UBOOL IsWire();
	void Hold();
	void Unhold();
	void ExecMacro( const char* Filename, FOutputDevice* Out=GSystem );

	// UViewport inlines.
	BYTE* _Screen( INT X, INT Y )
	{
		return ScreenPointer + (X+Y*Stride)*ColorBytes;
	}
	UBOOL IsOrtho()
	{
		return Actor->RendMap==REN_OrthXY||Actor->RendMap==REN_OrthXZ||Actor->RendMap==REN_OrthYZ;
	}
	UBOOL IsRealtime()
	{
		return Actor && (Actor->ShowFlags&(SHOW_RealTime | SHOW_PlayerCtrl));
	}
	BYTE ByteID()
	{
		BYTE ColorBytesID[5]={0,0,1,2,3};
		return ColorBytesID[ColorBytes] | (Caps & 7)*4;
	}
};

// Viewport hit-testing macros.
#define PUSH_HIT(frame,expr,...) if( (frame->Viewport)->HitTesting ) (frame->Viewport)->PushHit(expr(__VA_ARGS__),sizeof(expr));
#define POP_HIT(frame)       if( (frame->Viewport)->HitTesting ) (frame->Viewport)->PopHit(0);
#define POP_HIT_FORCE(frame) if( (frame->Viewport)->HitTesting ) (frame->Viewport)->PopHit(1);

/*-----------------------------------------------------------------------------
	Base hit proxy.
-----------------------------------------------------------------------------*/

// Hit proxy declaration macro.
#define DECLARE_HIT_PROXY(cls,parent) \
	const char* GetName() const \
		{ return #cls; } \
	UBOOL IsA( const char* Str ) const \
		{ return appStricmp(#cls,Str)==0 || parent::IsA(Str); }

// Base class for detecting user-interface hits.
struct ENGINE_API HHitProxy
{
	union
	{
		mutable INT Size;
		HHitProxy* Parent;
	};
	const char* GetName() const
	{
		return "HHitProxy";
	}
	virtual UBOOL IsA( const char* Str ) const
	{
		return appStricmp("HHitProxy",Str)==0;
	}
	virtual void Click( const FHitCause& Cause )
	{
		Cause.Observer->Click( Cause, *this );
	}
};

/*-----------------------------------------------------------------------------
	UClient.
-----------------------------------------------------------------------------*/

//
// Client, responsible for tracking viewports.
//
class ENGINE_API UClient : public UObject
{
	DECLARE_ABSTRACT_CLASS(UClient,UObject,CLASS_Config)

	// Variables.
	UEngine*			Engine;
	TArray<UViewport*>	Viewports;
	UViewport*			FullscreenViewport;
	INT					DrawCycles;

	// Configurable.
	INT		ViewportX;
	INT		ViewportY;
	FLOAT	Brightness;
	FLOAT	MipFactor;
	UBOOL	CaptureMouse;
	UBOOL	CurvedSurfaces;
	UBOOL	LowDetailTextures;
	UBOOL	ScreenFlashes;
	UBOOL	NoLighting;

	// Constructors.
	UClient();
	static void InternalClassInitializer( UClass* Class );

	// UObject interface.
	void Serialize( FArchive& Ar );
	void Destroy();
	void PostEditChange();

	// UClient interface.
	virtual void Init( UEngine* InEngine )=0;
	virtual void Flush();
	virtual void ShowViewportWindows( DWORD ShowFlags, int DoShow )=0;
	virtual void EnableViewportWindows( DWORD ShowFlags, int DoEnable )=0;
	virtual void Poll()=0;
	virtual UViewport* CurrentViewport()=0;
	virtual void Tick()=0;
	virtual UBOOL Exec( const char* Cmd, FOutputDevice* Out=GSystem )=0;
	virtual class UViewport* NewViewport( class ULevel* InLevel, const FName Name )=0;
	virtual void EndFullscreen()=0;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
