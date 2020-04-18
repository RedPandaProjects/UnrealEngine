/*=============================================================================
	Engine.h: Unreal engine public header file.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.
=============================================================================*/

#ifndef _INC_ENGINE
#define _INC_ENGINE

/*----------------------------------------------------------------------------
	API.
----------------------------------------------------------------------------*/

#ifdef ENGINE_EXPORTS
#define ENGINE_API DLL_EXPORT
#else
#define ENGINE_API DLL_IMPORT
#endif

/*-----------------------------------------------------------------------------
	Dependencies.
-----------------------------------------------------------------------------*/

#include "Core.h"

/*-----------------------------------------------------------------------------
	Global variables.
-----------------------------------------------------------------------------*/

ENGINE_API extern class FMemStack	GDynMem, GSceneMem;
ENGINE_API extern class FGlobalMath	GMath;

/*-----------------------------------------------------------------------------
	Engine public includes.
-----------------------------------------------------------------------------*/

#include "UnMath.h"			// Vector math functions.
#include "UnParams.h"		// Parameter parsing routines.
#include "UnObj.h"			// Standard object definitions.
#include "UnTex.h"			// Texture and palette.
#include "UnPrim.h"			// Primitive class.
#include "UnModel.h"		// Model class.
#include "EngineClasses.h"	// All actor classes.
#include "UnReach.h"		// Reach specs.
#include "UnURL.h"			// Uniform resource locators.
#include "UnLevel.h"		// Level object.
#include "UnIn.h"			// Input system.
#include "UnPlayer.h"		// Player class.
#include "UnEngine.h"		// Unreal engine.
#include "UnGame.h"			// Unreal game engine.
#include "UnCamera.h"		// Viewport subsystem.
#include "UnMesh.h"			// Mesh objects.
#include "UnActor.h"		// Actor inlines.
#include "UnAudio.h"		// Audio code.
#include "UnDynBsp.h"	    // Dynamic Bsp objects.

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
#endif