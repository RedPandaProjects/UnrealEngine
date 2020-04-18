/*=============================================================================

    UnFractal.cpp: Unreal's  Animated Realtime Texture  engine  / main file
	Copyright 1997,1998 by Epic MegaGames, Inc. and Evolution Software.
	This software is a trade secret.

    Revision history:
        * Created by Erik de Neve 1996,97
        * Major revised version; slow-fire and techbumps - July 97
		* Tim merged with Unreal object classes.   Aug 97
		* Erik - ongoing revisions & incorporating requests Sept 97 - Dec 97

		* Erik - Jan4 - switched off palette copying - caused problems since
		                non-unique palette names were generated for the 
						duplicates - clashed on merging different UTX-es.

		* Revision remarks
		  - Make sure all non-serialized vars are initialized in the constructors,
		    and all serialized vars are set one-time only in the ::Init functions.
		  - PostLoad gets called in the editor at edit-time, after properties are 
		    edited ( eg. sliders moved ) etc.
		  - For palette changes to become visible while editing you might
		    need to use GCache.Flush() -> drastic but works.
		  - Palettes and sparks are implemented as dynamic arrays. 
		    See  Core: UnTemplate.h
		  - The Destroy() function is where you want memory deallocation to 
		    take place, never use the actual "~destructor()" in Unreal -> 
			because of the way garbage-collection is implemented.
          - All new textures need some (dummy) palette at ::Init time in all 
		    cases, the engine chokes on them otherwise.
		  - If extra memory needs to be allocated for an effect, we do it by 
		    checking the pointer for null and allocating inside PostLoad, not 
			in the ctor because that's too early, some needed things like texture 
			dimensions aren't set yet at ctor time.
		  - Jump to processor-specific routines by evaluating (GSystem->Features & PLAT_PentiumPro) 
		    and (GSystem->Features & PLAT_MMX)
          - Better intelligent in-editor awareness possible. ( UBOOL GIsEditor;)
 
=============================================================================*/

#include "FractalPrivate.h"

/* Define these to be either 0 or ASM, never 1 */

#define RANDASM         ASM

#define FIREASM         0
#define WATERASM        0 
#define WETASM          ASM    
#define ICEASM          ASM

// Copying palettes - warning: if activated, created non-unique palette names.
#define COPYPALETTE 1 

/*----------------------------------------------------------------------------
	Globals.
----------------------------------------------------------------------------*/

// Macros.
#define WIDTHBYTES(bits) ((((bits) + 31) / 32) * 4)

// Constants.
#define STAR_CUTOFF		38		/*  Cloud thickness above which stars vanish    */
#define DEL_RANGE		12		/*  Mouse range for deleting sparks/drops etc   */
#define LOWESTGLOW		50		/*  Delete glowing-out sparks below this intensity  */
#define LIGHTPHASEBIAS	32		/*	bias for lightphasetable  */

// Globals.
extern "C"
{
	DWORD	SpeedRindex;
    DWORD   StaleRindex;
	BYTE	SpeedRandArr	 [512];
	BYTE	PhaseTable		 [256];
	BYTE    SignedPhaseTable [256];
	BYTE    LightPhaseTable  [256];
	DOUBLE  LTimeTotal1, LTimeTotal2, LinePixels;
}


/*-----------------------------------------------------------------------------
	Package implementation.
-----------------------------------------------------------------------------*/


IMPLEMENT_PACKAGE(Fire);


/*----------------------------------------------------------------------------
	Random number and math functions.
----------------------------------------------------------------------------*/

// Shift-register random generator.
// based on trinomial x**63 + x**31 + 1.
// Period of about 2^63  - tested up to about 2^37.

inline BYTE SpeedRand()
#if RANDASM
#pragma warning (disable : 4035) // legalize implied return value EAX/AX/AL
{
	__asm
    {
		mov ecx,         SpeedRindex
        mov edx,         SpeedRindex
        add ecx,         4*(1+31)
        add edx,         4*(1)
		and ecx,         0xfc
		and edx,         0xfc
        mov eax,         dword ptr SpeedRandArr[ecx]
        mov SpeedRindex, edx
        xor dword ptr SpeedRandArr[edx], eax
	}
}

#pragma warning (default : 4035)
#else
{
    SpeedRindex = (SpeedRindex + 1) & 63;
    return( SpeedRandArr[(SpeedRindex+31)& 63 ] ^= SpeedRandArr[ SpeedRindex ] );
}
#endif



//  Approximate arctangent.

double FakeAtan( double X )
{
    return 3.1415F * 0.5F * X / (Abs((FLOAT)X) + 1.0);
}


/*----------------------------------------------------------------------------
	Initialization routines.
----------------------------------------------------------------------------*/

// Initialize all _global_ tables.

void InitTables()
{
	static INT  Initialized=0;
	if( !Initialized )
	{
		// Init 8-bit sine table.
		for( INT  t=0; t < 256; t++ )
		{
			PhaseTable[t] = appRound(127.45F + 127.5F*appSin( ((FLOAT)t/256.0F) * 6.2831853F ));
		}

		for( t=0; t < 256; t++ )
		{
		 	  LightPhaseTable[t] = Clamp (PhaseTable[t] + LIGHTPHASEBIAS, 0, 255);
			 SignedPhaseTable[t] = (BYTE)( -128 + (char)PhaseTable[t] );
		}

		for( t = 0 ; t < 512 ; t++)
			SpeedRandArr[t] = (BYTE)(appRand() & 255);

		// Speedy random number generator: initialize & align the index.

#if     RANDASM
		//SpeedRindex = ((DWORD)(&SpeedRandArr) +0xFF ) & 0xFFFFFF00;
		SpeedRindex = 0;
#else
		SpeedRindex = 0;
#endif
		StaleRindex = 0;

		// Now initialized;
		Initialized=1;
	}
}



/*----------------------------------------------------------------------------
	Fire calculation.
----------------------------------------------------------------------------*/

#if FIREASM

extern "C" void  __cdecl CalcWrapFire ( BYTE* BitmapAddr, BYTE* RenderTable, DWORD Xdimension, DWORD Ydimension );
extern "C" void  __cdecl CalcSlowFire ( BYTE* BitmapAddr, BYTE* RenderTable, DWORD Xdimension, DWORD Ydimension );

extern "C" void  __cdecl CalcWrapFireP2 ( BYTE* BitmapAddr, BYTE* RenderTable, DWORD Xdimension, DWORD Ydimension );
extern "C" void  __cdecl CalcSlowFireP2 ( BYTE* BitmapAddr, BYTE* RenderTable, DWORD Xdimension, DWORD Ydimension );

#else

//
// Update fire.
//

void CalculateFire( BYTE* BitmapAddr, BYTE* RenderTable, DWORD Xdimension, DWORD Ydimension )
{
	guard(CalculateFire);
    for  (DWORD Y = 0 ;Y < (Ydimension - 2) ; Y++ )
    {
        BYTE* ThisLine  = BitmapAddr + Y * Xdimension;
        BYTE* BelowLine =   ThisLine +   Xdimension;
        BYTE* LowerLine =  BelowLine +   Xdimension;

        // Special case: X=0
        *(ThisLine) = RenderTable[
            *(BelowLine    ) +
         // *(BelowLine -1 ) +
            *(BelowLine +1 ) +
            *(LowerLine    )
            ];

        for (DWORD X=1; X < (Xdimension-1); X++ )
        {
        *(ThisLine + X ) = RenderTable[
            *(BelowLine + X   ) +
            *(BelowLine + X-1 ) +
            *(BelowLine + X+1 ) +
            *(LowerLine + X   )
            ];
        }

        //Special case: X=(Xdimension-1)
        *(ThisLine + Xdimension -1 ) = RenderTable[
            *(BelowLine + Xdimension-1   ) +
            *(BelowLine + Xdimension-1-1 ) +
        //  *(BelowLine + Xdimension-1+1 ) +
            *(LowerLine + Xdimension-1   )
            ];

     } //Y
	 unguard;
}



void CalcWrapFire(  BYTE* BitmapAddr,BYTE* RenderTable,DWORD Xdimension,DWORD Ydimension  )
{

    for  (DWORD Y = 0 ;Y < (Ydimension-2) ; Y++ )
    {
        BYTE* ThisLine  = BitmapAddr + Y * Xdimension;
        BYTE* BelowLine = ThisLine  +   Xdimension;
        BYTE* LowerLine = BelowLine +   Xdimension;

        // Special case: X=0
        *(ThisLine) = RenderTable[
			*(BelowLine    ) +
			*(BelowLine +Xdimension-1 ) +     //wrapping around left edge
			*(BelowLine +1 ) +
			*(LowerLine    )
			];

        for (DWORD X=1; X < (Xdimension-1); X++ )
        {
            *(ThisLine + X ) = RenderTable[
            *(BelowLine + X   ) +
            *(BelowLine + X-1 ) +
            *(BelowLine + X+1 ) +
            *(LowerLine + X   )
            ];
        }

        //Special case: X=(Xdimension-1)
        *(ThisLine + Xdimension -1 ) = RenderTable[
			*(BelowLine + Xdimension-1   ) +
			*(BelowLine + Xdimension-1-1 ) +
			*(BelowLine + 0 )              + // Wrapping around right edge.
			*(LowerLine + Xdimension-1   )
			];

    } //Y


    // Special case: line-before-last -> lowest line wraps around.

    Y = (Ydimension - 2);
    {
        BYTE* ThisLine  = BitmapAddr + Y * Xdimension;
        BYTE* BelowLine =   ThisLine +   Xdimension;
        BYTE* LowerLine = BitmapAddr;

        // Special case: X=0
        *(ThisLine) = RenderTable[
			*(BelowLine    ) +
			*(BelowLine +Xdimension-1 ) +     // Wrapping around left edge.
			*(BelowLine +1 ) +
			*(LowerLine    )
			];

        for (DWORD X=1; X < (Xdimension-1); X++ )
        {
            *(ThisLine + X ) = RenderTable[
            *(BelowLine + X   ) +
            *(BelowLine + X-1 ) +
            *(BelowLine + X+1 ) +
            *(LowerLine + X   )
            ];
        }

        //Special case: X=(Xdimension-1)
        *(ThisLine + Xdimension -1 ) = RenderTable[
			*(BelowLine + Xdimension-1   ) +
			*(BelowLine + Xdimension-1-1 ) +
			*(BelowLine + 0 )              + // Wrapping around right edge.
			*(LowerLine + Xdimension-1   )
			];

    } //Y


    // Special case: last line -> both lower lines wrap around.

    Y = (Ydimension - 1);
    {
        BYTE* ThisLine  =  BitmapAddr + Y * Xdimension;
        BYTE* BelowLine =  BitmapAddr;
        BYTE* LowerLine =  BitmapAddr + Xdimension;

        // Special case: X=0
        *(ThisLine) = RenderTable[
			*(BelowLine    ) +
			*(BelowLine +Xdimension-1 ) +     // Wrapping around left edge.
			*(BelowLine +1 ) +
			*(LowerLine    )
			];

        for (DWORD X=1; X < (Xdimension-1); X++ )
        {
			*(ThisLine  + X ) = RenderTable[
				*(BelowLine + X   ) +
				*(BelowLine + X-1 ) +
				*(BelowLine + X+1 ) +
				*(LowerLine + X   )
				];
        }


        //Special case: X=(Xdimension-1)

		*(ThisLine + Xdimension -1 ) = RenderTable[
			*(BelowLine + Xdimension-1   ) +
			*(BelowLine + Xdimension-1-1 ) +
			*(BelowLine + 0 )              + // Wrapping around right edge.
			*(LowerLine + Xdimension-1   )
			];
    } //Y
}


// Update special fire.

void CalcSlowFire( BYTE* BitmapAddr,BYTE* RenderTable,DWORD Xdimension,DWORD Ydimension  )
{

	BYTE Line0Storage[256];

	DWORD* StorePtr   = (DWORD*) &Line0Storage[0];
	DWORD* BitmapSrc  = (DWORD*) &BitmapAddr[0];

	// Copy upper line:
	for (DWORD t = 0; t < ((Xdimension)>>2) ; t++ )
	{
		*StorePtr++ = *BitmapSrc++;
	}

    for  (DWORD Y = 0 ;Y < (Ydimension-1) ; Y++ )
    {
        BYTE* ThisLine  =  BitmapAddr + Y * Xdimension;
        BYTE* BelowLine =  ThisLine;
        BYTE* LowerLine =  BelowLine + Xdimension;

        // Special case: X=0
        *(ThisLine) = RenderTable[
			*(BelowLine     ) +
			*(BelowLine + Xdimension-1 ) +     // Wrapping around left edge.
			*(BelowLine + 1 ) +
			*(LowerLine     )
		    ];

        for (DWORD X=1; X < (Xdimension-1); X++ )
        {
		*(ThisLine  + X ) = RenderTable[
			*(BelowLine + X   ) +
			*(BelowLine + X-1 ) +
			*(BelowLine + X+1 ) +
			*(LowerLine + X   )
			];
        }

        //Special case: X=(Xdimension-1)
		*(ThisLine + Xdimension -1 ) = RenderTable[
			*(BelowLine + Xdimension-1   ) +
			*(BelowLine + Xdimension-1-1 ) +
			*(BelowLine + 0 )              + // Wrapping around right edge.
			*(LowerLine + Xdimension-1   )
        ];

    } //Y

    // Special case: last line -> lower line wraps around.
    Y = (Ydimension - 1);
    {
        BYTE* ThisLine  = BitmapAddr + Y * Xdimension;
        BYTE* BelowLine = ThisLine;
        BYTE* LowerLine = Line0Storage; // Old 'BitmapAddr[...]'

        // Special case: X=0
        *(ThisLine) = RenderTable[
			*(BelowLine    ) +
			*(BelowLine +Xdimension-1 ) +     // Wrapping around left edge.
			*(BelowLine +1 ) +
			*(LowerLine    )
			];

        for (DWORD X=1; X < (Xdimension-1); X++ )
        {
		*(ThisLine  + X ) = RenderTable[
			*(BelowLine + X   ) +
			*(BelowLine + X-1 ) +
			*(BelowLine + X+1 ) +
			*(LowerLine + X   )
			];
        }

        //Special case: X=(Xdimension-1)
        *(ThisLine  + Xdimension -1 ) = RenderTable[
			*(BelowLine + Xdimension-1   ) +
			*(BelowLine + Xdimension-1-1 ) +
			*(BelowLine + 0              ) + // Wrapping around right edge.
			*(LowerLine + Xdimension-1   )
			];
    } //Y
}
#endif


/*----------------------------------------------------------------------------
	Add a spark to the array.
----------------------------------------------------------------------------*/

enum {  // Internal spark types.
	// special-case ones 
	SPARK_LissajX = SPARK_LASTTYPE , 
 	SPARK_LissajY        ,
	// Spawned ones: all above SPARK_TRANSIENTS are transient ones.
	ISPARK_TRANSIENTS    , 
	ISPARK_Drifter       ,
	ISPARK_DriftSlow     ,
	ISPARK_Faller        , 
	ISPARK_VShooter      ,     
	ISPARK_Drop          ,    
    ISPARK_Move          ,    
    ISPARK_SpawnedEel	 ,
	ISPARK_SpawnedTwirl	 ,
	ISPARK_SprinklerTwirl,
	ISPARK_Custom	     ,
	ISPARK_Graviton      ,
	ISPARK_SpawnedSperm  ,
};


void UFireTexture::AddSpark( INT  MouseX, INT  MouseY )
{
	guard(UFireTexture::AddSpark);

	// Edit-time only.
	static INT  LightPinX = 0;
	static INT  LightPinY = 0;

	// Return if out of bounds or out of sparks.
    if( MouseX<0 || MouseY<0 || MouseX>=USize || MouseY>=VSize || ActiveSparkNum>=SparksLimit )
        return;

    // Dynamic spark arrays: ActiveSparkNum/SparksLimit  have the current sparks and the current maximum
	// size; with these, the dynamic array is used as a *static* array in all spark adders/setters.
	
	// Make new spark.
    INT  S = ActiveSparkNum++;  // ->Sparks[0] is the first one.

	// General new spark initialization.
    Sparks(S).Type = SparkType;
    Sparks(S).X    = MouseX;
    Sparks(S).Y    = MouseY;
	Sparks(S).Heat = FX_Heat; 

    // Spark-type specific assignments.
    switch( SparkType )
    {
		case SPARK_Sparkle:
			Sparks(S).ByteA  = FX_Size;
			Sparks(S).ByteB  = FX_AuxSize;
			break;

        case SPARK_Pulse:
            Sparks(S).Heat  = DrawPhase + AuxPhase;  
			DrawPhase      += FX_Phase;         //
			Sparks(S).ByteD = FX_Frequency;     // Improve useful range.
            break;

        case SPARK_Signal:
            Sparks(S).Heat  = SpeedRand();      //
			Sparks(S).ByteD = FX_Frequency;     // Improve useful range.
			Sparks(S).ByteC = (255 - FX_Heat);  //
            break;                           

        case SPARK_Stars:    // Stars, FieldA for this type holds (negative) start Intensity.
            Sparks(S).ByteA = FX_Heat;
			StarStatus = 1;  // At least one star..
            break;

		case SPARK_Organic:  // Organic blaze-up.
			Sparks(S).ByteC = FX_Area; 
			break;             

		case SPARK_Eels: 
			Sparks(S).ByteC = FX_Size;
			break;

		case SPARK_BlazeLeft:
		case SPARK_BlazeRight:
			Sparks(S).ByteC = FX_Size;
			break;

		case SPARK_OzHasSpoken: 
			break;  

		case SPARK_Emit: // Spawn at angle , lifetime , linear movers.
			Sparks(S).ByteA  = FX_HorizSpeed - 128;
			Sparks(S).ByteB  = 127 ^ FX_VertSpeed;
			Sparks(S).ByteD  = 255/( (int)FX_Size +1);
			break;

		case SPARK_Fountain: // As Emit, with gravity.
			Sparks(S).ByteA  = FX_HorizSpeed - 128;
			Sparks(S).ByteB  = 127 ^ FX_VertSpeed;
			Sparks(S).ByteD  = 255/( (int)FX_Size + 1);
			break;
	
		case SPARK_Cylinder: // Twister (X=sin()) movement.
		case SPARK_Cylinder3D: // Twister (X=sin()) movement.
			{
				int TempSize = FX_Size;
				int TempSpeed = FX_HorizSpeed - 128;
				int ThisIdx = S;
				int Axis2 = PenDownX*2 + FX_Size;

				// Scale to imaginary axis while mousebutton remains down.
				if ((DrawMode > DRAW_Normal) && (PenDownX != 0))
				{
					TempSize = Axis2 - MouseX*2;

					// Axis = at PenDownX + 0.5 * FX_Size.

					if (TempSize<0)  // Crossed the axis.
					{	
						TempSize = MouseX*2 - Axis2;
						Sparks(S).X = MouseX - TempSize;  // Minus total diameter.
						TempSpeed = (256-TempSpeed) & 255; 
					}
				};
				
				Sparks(S).ByteA  = ( GlobalPhase * FX_Frequency + FX_Phase ) & 255;
				Sparks(S).ByteB  = TempSize;  // Size.
				Sparks(S).ByteD  = TempSpeed;
				
				// Draw second spark at +0.5 phase
				if ((DrawMode == DRAW_Lathe_2) && (ActiveSparkNum < SparksLimit)) 
				{
					INT  S = ActiveSparkNum++; 
					Sparks(S).Type = SparkType;
					Sparks(S).X    = Sparks(ThisIdx).X;
					Sparks(S).Y    = MouseY;
					Sparks(S).Heat = FX_Heat; 

					Sparks(S).ByteA  = Sparks(ThisIdx).ByteA + 128;
					Sparks(S).ByteB  = TempSize;  // size
					Sparks(S).ByteD  = TempSpeed;
				}
				
				// Draw 2 more sparks at   +.33/.67 phases.
				if (( DrawMode == DRAW_Lathe_3 ) && ( (ActiveSparkNum+2) <= SparksLimit ) )
				{
					for(int t=1; t<3; t++)
					{
						INT  S = ActiveSparkNum++; 
						Sparks(S).Type   = SparkType;
						Sparks(S).X      = Sparks(ThisIdx).X;
						Sparks(S).Y      = MouseY;
						Sparks(S).Heat   = FX_Heat; 

						Sparks(S).ByteA  = Sparks(ThisIdx).ByteA + (t*(256/3) );
						Sparks(S).ByteB  = TempSize; // Size.
						Sparks(S).ByteD  = TempSpeed;
					}
				}			

				// Draw 2 more sparks at   +.33/.67 phases.
				if (( DrawMode == DRAW_Lathe_4 ) && ( (ActiveSparkNum+3) <= SparksLimit ) )
				{
					for(int t=1; t<4; t++)
					{
						INT  S = ActiveSparkNum++; 
						Sparks(S).Type   = SparkType;
						Sparks(S).X      = Sparks(ThisIdx).X;
						Sparks(S).Y      = MouseY;
						Sparks(S).Heat   = FX_Heat; 

						Sparks(S).ByteA  = Sparks(ThisIdx).ByteA +( t * 64 );
						Sparks(S).ByteB  = TempSize; // Size.
						Sparks(S).ByteD  = TempSpeed;
					}
				}			
			}
			break;

		case SPARK_Jugglers: // Twister (X=sin()) movement.
			Sparks(S).ByteA  = ( GlobalPhase * FX_Frequency + FX_Phase ) & 255;
			Sparks(S).ByteB  = FX_Size;  // size
			Sparks(S).ByteD  = FX_HorizSpeed - 128;
			break;

		case SPARK_Lissajous:
			Sparks(S).Heat  = FX_Size;       // size
			Sparks(S).ByteA = FX_Phase;      // phase u
			Sparks(S).ByteB = FX_Frequency;  // phase v
			Sparks(S).ByteC = FX_HorizSpeed - 128;
			Sparks(S).ByteD = FX_VertSpeed  - 128;
			// Special-cases redirected to more efficient setters.
			if (Sparks(S).ByteC == 0) Sparks(S).Type = SPARK_LissajY;
			if (Sparks(S).ByteD == 0) Sparks(S).Type = SPARK_LissajX;
			break;

        case SPARK_SphereLightning: // Spherical lightning.
            Sparks(S).Heat   = FX_Heat;  // heat
            Sparks(S).ByteC  = FX_Size;  // radius
            // minimal 8 (=radius 4)
            if (Sparks(S).ByteC <8) Sparks(S).ByteC = 8;
			Sparks(S).ByteD  = 96;  // Hardcoded chance.
            break;

		case SPARK_Flocks:
			Sparks(S).ByteA  = 128;
			Sparks(S).ByteB  = FX_Size;   // Lifetime
			Sparks(S).ByteC  = 128;
			Sparks(S).ByteD  = (255-FX_Area);   // spawn direction delta
			break;

		case SPARK_Wheel:  // Swirly emitter.
			Sparks(S).ByteA  = FX_Phase;  // Initial spawn dir.
			Sparks(S).ByteB  = FX_Size;   // Lifetime
			Sparks(S).ByteC  = FX_Frequency; 
			Sparks(S).ByteD  = (255-FX_Area);   // spawn direction delta
			break;
	
		case SPARK_Sprinkler: 
			Sparks(S).ByteA  = FX_Phase;  // Initial spawn dir.
			Sparks(S).ByteB  = - 128;
			Sparks(S).ByteC  = 128;       // lifetime
			Sparks(S).ByteD  = FX_Frequency;
			break;

		case SPARK_Gametes: 
			Sparks(S).ByteC  = FX_Size;    // Lifetime of spawned spermatozoids.
			break;

		case SPARK_LocalCloud:
			Sparks(S).ByteC  = FX_Area;
			Sparks(S).ByteA  = FX_HorizSpeed - 128;         
            Sparks(S).ByteB  = (255 ^ FX_VertSpeed) - 128;  
			Sparks(S).ByteD  = 255-FX_Size;
			break;

		case SPARK_CustomCloud:
			Sparks(S).ByteA  = FX_HorizSpeed - 128;         
            Sparks(S).ByteB  = (255 ^ FX_VertSpeed) - 128;  
			Sparks(S).ByteD  = 255-FX_Size;
			break;

        case SPARK_LineLightning: // 2-point lightning bolts.
        case SPARK_RampLightning: 
            // We have a new spark if no other was an 'open' one.
			// Not always the last one since short-lived sparks may shuffle the spark
			// order at any moment.
            do { S--; }
			while( (S>=0) && !( (Sparks(S).Type == SparkType) && (Sparks(S).ByteD == 0))  );

            if (S>=0)
            // Open point found, draw it while being stretched.
            {
                ActiveSparkNum--; // so don't allocate 'current' spark

				// Refresh to 'starting' point
				Sparks(S).X  = LightPinX;
				Sparks(S).Y  = LightPinY;

                // Dragging the lightning: Must have a heat value.
                // Close it by setting ByteD = heat ?
                // To nonzero, eventually.

                Sparks(S).Heat = 3 | (FX_Heat); // Brightness.

                INT  LenX = MouseX - (int)Sparks(S).X;
                INT  LenY = MouseY - (int)Sparks(S).Y;

				if (LenX<0)		LenX  = (- LenX) | 1;
                else			LenX &= 0xFFFFFFFE;
				if( LenY<0 )	LenY  = (- LenY) | 1;
                else			LenY &= 0xFFFFFFFE;

                if ( (LenX == 0) && (LenY == 0) )
                    Sparks(S).Heat = 0;  // too short!

                Sparks(S).ByteA = LenX;
                Sparks(S).ByteB = LenY;
                // ByteD Stays 0 while drawing to indicate open bolt.

            } // Open point found.

            else

            { // No open point found: new lightning thing.
                S = ActiveSparkNum - 1;
                Sparks(S).ByteD = 0; // Signify open bolt.
				Sparks(S).Heat  = 0; //
				LightPinX = MouseX;
				LightPinY = MouseY;
            }
			break;
	}

	unguard;
}


void UFireTexture::CloseSpark( INT  MouseX, INT  MouseY)
{
	guard(UFireTexture::CloseSpark);
    INT  S = ActiveSparkNum; // Sparks[S=0] is only one if ActiveSparkNum = 1.

	        // Warning: extremely kludgy.
            // Find ANY old spark of type 17/18 which has an 'open' end.
            do { S--; } while( (S>=0) && !(	(
				                   (Sparks(S).Type == SPARK_LineLightning)
							    || (Sparks(S).Type == SPARK_RampLightning)
								)
								&& (Sparks(S).ByteD == 0))  );
            if (S>=0)
            // Open point found, closin' it.
            {
                // ByteD, minimally 1...
				Sparks(S).ByteD  = FX_Frequency;
				if (Sparks(S).ByteD == 0)  Sparks(S).ByteD = 1; // Ensure != 0.
			}
	unguard;
}



/*----------------------------------------------------------------------------
	Spark deletion.
----------------------------------------------------------------------------*/

void UFireTexture::DeleteSparks( INT  SparkX, INT  SparkY, INT  AreaWidth)
{
	guard(UFireTexture::DeleteSparks);
    if( ActiveSparkNum > 0 )
	{
		for( INT S = 0; S < ActiveSparkNum; S++ )
		{
			// Diamond-shaped eraser.
			if( AreaWidth >= Abs(SparkX - Sparks(S).X) + Abs(SparkY - Sparks(S).Y ) )
			{
				// Delete spark by replacing it with last one (+ delete last one).
				int LastSpark = --ActiveSparkNum;
				Sparks(S) =   Sparks(LastSpark);
			}
		}
	}
	unguard;
}


/*----------------------------------------------------------------------------
	Spark line drawing.
----------------------------------------------------------------------------*/

void UFireTexture::DrawSparkLine( INT  StartX, INT  StartY, INT  DestX, INT  DestY, INT  Density )
{
	guard(UFireTexture::DrawSparkLine);
    INT  Xinc, Yinc;

    INT  DivX = DestX - StartX;
    INT  DivY = DestY - StartY;

    if ((DivX == 0) && (DivY == 0))
        return;

    if (DivX<0)
        Xinc = -1;
    else
    {
        if (DivX>0)
            Xinc = 1;
        else
            Xinc = 0;
    }

    if (DivY<0)
        Yinc = -1;
    else
    {
        if (DivY>0)
            Yinc = 1;
        else
            Yinc = 0;
    }

    DivX = Abs(DivX);
    DivY = Abs(DivY);

    INT  Xpoint = StartX;
    INT  Ypoint = StartY;

    if (DivX >= DivY)   // Draw line based on X loop.
    {
        INT  DivY2  = DivY + DivY;
        INT  Diff   = DivY2 - DivX;
        INT  DivXY2 = DivY2 - DivX - DivX;

        for (int LCount = 1; LCount <= DivX; LCount++)
        {
            AddSpark(Xpoint,Ypoint);

            if (Diff<0)
                Diff += DivY2;
            else
            {
                Diff   += DivXY2;
                Ypoint += Yinc;
            }
        Xpoint += Xinc;
        }
    }
    else    // Draw line based on Y loop.
    {
        INT  DivX2  = DivX  + DivX;
        INT  Diff   = DivX2 - DivY;
        INT  DivXY2 = DivX2 - DivY - DivY;

        for (int LCount = 1; LCount <= DivY; LCount++)
        {
            AddSpark(Xpoint,Ypoint);

            if (Diff<0)
                Diff += DivX2;
            else
            {
                Diff   += DivXY2;
                Xpoint += Xinc;
            }
            Ypoint += Yinc;
        }
    }
	unguard;
}


/*----------------------------------------------------------------------------
	Fire painting.
----------------------------------------------------------------------------*/

// Spark-paint routine - the fire-specific part of the editor.

void UFireTexture::FirePaint( INT MouseX, INT MouseY, DWORD Buttons )
{
	guard(UFireTexture::FirePaint);

	UBOOL RightButton = (Buttons & MOUSE_Right);
	UBOOL  LeftButton = (Buttons & MOUSE_Left);

	// Perform painting.
    static INT  LastMouseX=0, LastMouseY=0, LastLeftButton=0, LastRightButton=0;

    UBOOL  PosChanged   = ((LastMouseX != MouseX) || (LastMouseY != MouseY));
    UBOOL  RightChanged =  (LastRightButton != RightButton);
    UBOOL  LeftChanged  =  (LastLeftButton  != LeftButton);


	
	if ( LeftChanged && LeftButton )
	{
		PenDownX = MouseX;
		PenDownY = MouseY;
	}
	if ( LeftChanged && (!LeftButton) )
	{
		PenDownX = 0;
		PenDownY = 0;
	}
    
    // Draws the kind of spark/linetype currently selected.
    if( LeftButton )
    {
        if( !LeftChanged && PosChanged && ( SparkType<4 ))
            DrawSparkLine( LastMouseX, LastMouseY, MouseX, MouseY, 1);

        if( LeftChanged || ( PosChanged && ( SparkType>=4 )  )  )
			{	
            AddSpark( MouseX, MouseY);
			}
    }
    else // LeftButton == 0
	{
		// Lightning bolt close on release + movement.
		if( LeftChanged || PosChanged )
		{
			CloseSpark( MouseX, MouseY );
		}
	}

    // Lightning bolt close on non-moving re-clicking.
	if ( (!LeftChanged) && (! PosChanged) && LeftButton )
	{
		CloseSpark( MouseX, MouseY );
	}

	// Delete any sparks within certain range of mouse cursor.
    if( RightButton && (PosChanged || RightChanged) )
	{
        DeleteSparks( MouseX, MouseY, DEL_RANGE );
	}

	// Remember.
    LastMouseX      = MouseX;
    LastMouseY      = MouseY;
    LastLeftButton  = LeftButton;
    LastRightButton = RightButton;

	unguard;
}



/*----------------------------------------------------------------------------
	Fire update.
----------------------------------------------------------------------------*/


// Advance a spark stochastically using signed 8-bit speed.
// Spark->ByteA, ByteB treated as signed (char) speed == probability of 1-pixel movement.

#if ASM 
inline void UFireTexture::MoveSparkXY( FSpark* Spark, CHAR Xspeed, CHAR Yspeed )
{
	__asm
		{
		mov ecx,SpeedRindex
		mov esi,Spark

        mov edx,ecx
        add ecx,4*(1+31)  // Make more PPRO friendly...

        add edx,4*(1)     //
		and ecx,0xfc

		xor ebx,ebx
		and edx,0xfc

		mov bl,Xspeed
        mov eax,dword ptr SpeedRandArr[ecx]

        mov SpeedRindex,edx
        xor dword ptr SpeedRandArr[edx],eax
		// Get one huge random number (eax)

		and eax,0x007f007f

		// xor ebx,ebx
		xor ecx,ecx
		cmp bl,0x80  // (cmp bl,128) Carry for all non-negative

		adc ecx,ecx  // ECX = 1 for all positives
		and bl,0x7f  // mask out sign

		xor edx,edx
		cmp bl,al

		mov ebx, this
		mov dl,[esi]Spark.X

		sbb edi,edi   // Neg: 0/-1  => 0/-1
		//and eax,0x7F  // prepare low 7 bits
		mov ebx,[ebx].UMask

		add edi,ecx   // Pos: 0/-1 =>  1/0 -> needs +1  -> total: 
		xor ecx,ecx

		nop
		add edx,edi

		and edx,ebx
		xor ebx,ebx

		mov [esi]Spark.X,dl
		mov bl,Yspeed

		xor edx,edx
		cmp bl,0x80   // Carry for all non-negative.

		adc ecx,ecx   // ECX = 1 for all positives
		and bl,0x7f

		cmp bl,al
		mov ebx, this

		mov dl,[esi]Spark.Y
		sbb edi,edi   // 

		add edi,ecx   // Negative: 0/-1  positive:  1/0
		mov ebx,[ebx].VMask
		//
		add edx,edi
		and edx,ebx   

		mov [esi]Spark.Y,dl	
	}
}

#else

inline void UFireTexture::MoveSparkXY( FSpark* Spark, CHAR Xspeed, CHAR Yspeed )
{
    if (Xspeed<0) // move left
    {
        if ((SpeedRand()&127) < -Xspeed )
            Spark->X = UMask & (Spark->X-1);
    }
    else // move right
    {
        if ((SpeedRand()&127) <  Xspeed )
            Spark->X = UMask & (Spark->X+1);
    }

    if (Yspeed<0) // move up
    {
        if ((SpeedRand()&127) < -Yspeed )
            Spark->Y = VMask &(Spark->Y-1);
    }
    else // move down
    {
        if ((SpeedRand()&127) <  Yspeed )
			Spark->Y = VMask &(Spark->Y+1);
    }
}
#endif


#if ASM

inline void UFireTexture::MoveSpark( FSpark* Spark)
{
	__asm
	{

		mov ecx,SpeedRindex
		mov esi,Spark

        mov edx,ecx
        add ecx,4*(1+31)  // Make more PPRO friendly...

        add edx,4*(1)     //
		and ecx,0xfc

		xor ebx,ebx
		and edx,0xfc

		mov bl,[esi]Spark.ByteA
        mov eax,dword ptr SpeedRandArr[ecx]

		mov edi,dword ptr SpeedRandArr[edx]
        mov SpeedRindex,edx

		nop
		xor eax,edi

        mov dword ptr SpeedRandArr[edx],eax
		and eax,0x007f007f

		// xor ebx,ebx
		xor ecx,ecx
		cmp bl,0x80 // (cmp bl,128) Carry for all non-negative

		adc ecx,ecx  // ECX = 1 for all positives
		and bl,0x7f // mask out sign

		xor edx,edx
		cmp bl,al

		mov ebx,this
		mov dl,[esi]Spark.X

		sbb edi,edi   // Neg: 0/-1  => 0/-1
		//and eax,0x7F  // prepare low 7 bits
		mov ebx,[ebx].UMask

		add edi,ecx   // Pos: 0/-1 =>  1/0 -> needs +1  -> total: 
		xor ecx,ecx
		//
		nop
		add edx,edi

		and edx,ebx
		xor ebx,ebx

		mov [esi]Spark.X,dl
		mov bl,[esi]Spark.ByteB

		xor edx,edx
		cmp bl,0x80   // Carry for all non-negative.

		adc ecx,ecx   // ECX = 1 for all positives
		and bl,0x7f

		cmp bl,al
		mov ebx,this

		mov dl,[esi]Spark.Y
		sbb edi,edi   // 

		add edi,ecx   // Negative: 0/-1  positive:  1/0
		mov ebx,[ebx].VMask
		//
		add edx,edi
		and edx,ebx   
		mov [esi]Spark.Y,dl	
	}
}

#else

inline void UFireTexture::MoveSpark( FSpark* Spark)
{
    if ((CHAR)Spark->ByteA<0) // move left
    {
        if ((SpeedRand()&127) < -(CHAR)Spark->ByteA )
            Spark->X = UMask & (Spark->X-1);
    }
    else // move right
    {
        if ((SpeedRand()&127) <  (CHAR)Spark->ByteA )
            Spark->X = UMask & (Spark->X+1);
    }

    if ((CHAR)Spark->ByteB<0) // move up
    {
        if ((SpeedRand()&127) < -(CHAR)Spark->ByteB )
            Spark->Y = VMask &(Spark->Y-1);
    }
    else // move down
    {
        if ((SpeedRand()&127) <  (CHAR)Spark->ByteB )
			Spark->Y = VMask &(Spark->Y+1);
    }
}
#endif



inline void UFireTexture::MoveSparkAngle( FSpark* Spark, BYTE Angle)
{

	char Xdir = -127 + (CHAR)PhaseTable[ Angle ];
    char Ydir = -127 + (CHAR)PhaseTable[ BYTE(Angle+64 ) ];

    if ((CHAR) Xdir<0) 
    {
		// Move left.
        if ((SpeedRand()&127) < -(CHAR) Xdir )
             Spark->X = UMask & ( Spark->X-1);
    }
    else 
    {
		// Move right.
        if ((SpeedRand()&127) <  (CHAR) Xdir )
             Spark->X = UMask & ( Spark->X+1);
    }

    if ((CHAR) Ydir<0) 
    {
		// Move up.
        if ((SpeedRand()&127) < -(CHAR) Ydir )
             Spark->Y = VMask &( Spark->Y-1);
    }
    else 
    {
		// Move down.
        if ((SpeedRand()&127) <  (CHAR) Ydir )
			 Spark->Y = VMask &( Spark->Y+1);
    }
}


inline void UFireTexture::MoveSparkTwo( FSpark* Spark )
{    
    if ((CHAR)Spark->ByteA<0) 
    {
		// Negative update.
        if ((SpeedRand()&127) < -(CHAR)Spark->ByteA )
            Spark->X = UMask & (Spark->X-1);
    }
    else    
    {
		// Positive update.
        if ((SpeedRand()&127) <  (CHAR)Spark->ByteA )
            Spark->X = UMask & (Spark->X+1);
    }
    Spark->Y = VMask &(Spark->Y-2);
}



void UFireTexture::DrawFlashRamp( LineSeg LL, BYTE Color1, BYTE Color2 )
{
    DWORD	SparkDest;
    BYTE	FlashArray[256];
	INT  Xstep,Ystep,RealYlen,RealXlen;

	// Make writing cache-friendlier by drawing approximately to the right or downwards.

	if ( ( ( LL.Ylen & 1 ) && ((LL.Ylen*2) >= LL.Xlen ) ) || 
	     ( ( LL.Xlen & 1 ) && ((LL.Ylen*2) <  LL.Xlen ) ) )
    {
		LL.Xpos = LL.Xpos + ((LL.Xlen & 1)? -LL.Xlen:LL.Xlen );
		LL.Ypos = LL.Ypos + ((LL.Ylen & 1)? -LL.Ylen:LL.Ylen );
		LL.Xlen ^=1;
		LL.Ylen ^=1;	
		BYTE Tcol = Color1;
		Color1 = Color2;
		Color2 = Tcol;
    }

	int MajorLen = 1 | ( (LL.Xlen >= LL.Ylen) ?  LL.Xlen : LL.Ylen );

    // Fill array for the specific length.
    INT  FlashPos = 0;
    for (int Flash = 0; Flash < MajorLen; Flash++)
    {
        FlashPos += ( FlashArray[Flash] = SpeedRand() );
    }

	if (LL.Ylen & 1)
	{
		Ystep = -1;
		RealYlen = -LL.Ylen;
	}
	else
	{
		Ystep = 1;
		RealYlen =  LL.Ylen;
	}


	if (LL.Xlen & 1)
	{
		Xstep = -1;
		RealXlen = -LL.Xlen;
	}
	else
	{
		Xstep = 1;
		RealXlen =  LL.Xlen;
	}


	//  Setup color ramp.
	int RampColor  =  Color1 << 23 ;
	int ColorSlope =  ((Color2 - Color1 ) << 23 ) / MajorLen;
    

    if (LL.Xlen>=LL.Ylen)
    {
        // X major axis.
        // calculate BIAS:
        // Bias = (Ylen << 6) - FlashPos;

        INT  Ypoz = (LL.Ypos << 6);
        INT  FlashBias = (( (int)RealYlen << 6) - FlashPos) / MajorLen;  

        //
        for (Flash = 0; Flash < LL.Xlen; Flash++)
        {
			Ypoz += FlashArray[Flash] + FlashBias;
			SparkDest = (((Ypoz >> 6) & VMask) << UBits) + (LL.Xpos & UMask);
			LL.Xpos += Xstep;   // increase X
			GetMip(0)->DataArray(SparkDest) = ( (RampColor += ColorSlope) >> 23 );
        }

    }
    else
    {
        //  Y major axis.
        //  calculate BIAS:
        //  Bias = (Xlen << 6) - FlashPos;

        INT  Xpoz = (LL.Xpos << 6);
        INT  FlashBias = (( (int)RealXlen << 6) - FlashPos) / MajorLen;   

        //
        for (Flash = 0; Flash < LL.Ylen; Flash++)
        {
			Xpoz += FlashArray[Flash] + FlashBias;
			SparkDest = ( (LL.Ypos & VMask) << UBits) + ((Xpoz >> 6) & UMask);
			LL.Ypos += Ystep;   // increase Y
			GetMip(0)->DataArray(SparkDest) = ((RampColor += ColorSlope) >> 23);
		}
    }
}




/*----------------------------------------------------------------------------
	Spark redrawing.
----------------------------------------------------------------------------*/



void UFireTexture::RedrawSparks()
{
	guard(UFireTexture::RedrawSparks);

	AuxPhase += FX_Frequency;  
	GlobalPhase++;

	int LocalUMask = UMask;
	int LocalVMask = VMask;
	int LocalUBits = UBits;
	
	// Warning: On any movement or new creation, the integrity of a spark's coordinates
    // must be assured by using UMask and VMask.
    
    for (int S = 0; S < ActiveSparkNum; S++)
	{
        FSpark* ThisSpark = &(Sparks(S));

        switch( ThisSpark->Type )
        {
        
        case SPARK_Burn:
			{
				DWORD SparkDest = (DWORD)(ThisSpark->X + (ThisSpark->Y << UBits) );
				GetMip(0)->DataArray(SparkDest) = SpeedRand();  
				break;
			}

        case SPARK_Sparkle: // Normal spark with positional jitter.
			{
				DWORD NewSparkX = ( ThisSpark->X + (( SpeedRand() * ThisSpark->ByteA ) >> 8 ) );
				DWORD NewSparkY = ( ThisSpark->Y + (( SpeedRand() * ThisSpark->ByteB ) >> 8 ) );
				DWORD SparkDest = (DWORD)( (UMask & NewSparkX) + ( (VMask & NewSparkY) << UBits  ) );
				GetMip(0)->DataArray(SparkDest) = ThisSpark->Heat;
				break;
			}

        case SPARK_Pulse: // Phased sparks.
			{
				DWORD SparkDest = (DWORD)( ThisSpark->X + ( ThisSpark->Y << UBits ));
				GetMip(0)->DataArray(SparkDest) = (BYTE) ThisSpark->Heat;
				ThisSpark->Heat +=ThisSpark->ByteD;
				break;
			}

        case SPARK_Signal: // Pulse-phased sparks.
			{
				DWORD SparkDest = (DWORD)(ThisSpark->X + ( ThisSpark->Y << UBits ));

				if ( ThisSpark->Heat > ThisSpark->ByteC )
					GetMip(0)->DataArray(SparkDest) = (BYTE) ThisSpark->Heat;

				if ( (ThisSpark->Heat +=ThisSpark->ByteD) < ThisSpark->ByteD ) 
					ThisSpark->Heat = SpeedRand(); // Renew phase...
				break;
			}
	
		case SPARK_Cylinder: // Draw [Phase>>?] nr of sparks in a SIZE twister, speed FREQ
            {  
				// Draw harmonic-motion spark based on size, phase, speed (freq).
				// 'Z' distance suggested by brightness.
				// DWORD SparkDest = (DWORD)( ThisSpark->X + ( ThisSpark->Y << UBits ));

				BYTE Htemp = Min( PhaseTable[ (ThisSpark->ByteA+64) & 255 ] +  ThisSpark->Heat, 255);
				BYTE XTemp = ThisSpark->X + (( (PhaseTable[ ThisSpark->ByteA ]) * ThisSpark->ByteB ) >> 8);
				DWORD SparkDest = (DWORD)( (XTemp & UMask) + ( ThisSpark->Y << UBits ) );
				GetMip(0)->DataArray(SparkDest) = Htemp;			

				ThisSpark->ByteA += ThisSpark->ByteD; // Angle increment/decrement
            }
            break;

		case SPARK_Cylinder3D: // Draw [Phase>>?] nr of sparks in a SIZE twister, speed FREQ
            {  
				// Draw harmonic-motion spark based on size, phase, speed (freq).
				// 'Z' distance suggested by brightness.
				// DWORD SparkDest = (DWORD)( ThisSpark->X + ( ThisSpark->Y << UBits ));

				if ( ((ThisSpark->ByteA+64)&255) < 128 )
				{
					BYTE Htemp = Min( PhaseTable[ (ThisSpark->ByteA+64) & 255 ] +  ThisSpark->Heat, 255);
					BYTE XTemp = ThisSpark->X + (( (PhaseTable[ ThisSpark->ByteA ]) * ThisSpark->ByteB ) >> 8);
					DWORD SparkDest = (DWORD)( (XTemp & UMask) + ( ThisSpark->Y << UBits ) );
					GetMip(0)->DataArray(SparkDest) = Htemp;			
				}

				ThisSpark->ByteA += ThisSpark->ByteD; // Angle increment/decrement
            }
            break;

		case SPARK_Jugglers: // Draw [Phase>>?] nr of sparks in a SIZE twister, speed FREQ
            {  
				// Draw harmonic-motion spark based on size, phase, speed (freq).
				// 'Z' distance suggested by brightness.
				// DWORD SparkDest = (DWORD)( ThisSpark->X + ( ThisSpark->Y << UBits ));

				BYTE Htemp = Min( PhaseTable[ (ThisSpark->ByteA+64) & 255 ] +  ThisSpark->Heat, 255 );
				BYTE YTemp = ThisSpark->Y + (( (PhaseTable[ ThisSpark->ByteA ]) * ThisSpark->ByteB ) >> 8);
				DWORD SparkDest = (DWORD)( ThisSpark->X + ( (YTemp & VMask) << UBits )  );
				GetMip(0)->DataArray(SparkDest) = Htemp;
				ThisSpark->ByteA += ThisSpark->ByteD; //
            }
            break;

		case SPARK_Lissajous: // Draw [Phase>>?] nr of sparks in a SIZE twister, speed FREQ
            {  
				// Draw harmonic-motion spark based on size, phase, speed (freq).
				// 'Z' distance suggested by brightness.
				// DWORD SparkDest = (DWORD)( ThisSpark->X + ( ThisSpark->Y << UBits ));

				BYTE Htemp = LightPhaseTable[ (BYTE) (ThisSpark->ByteA+64) ]; // Light phase == A
				BYTE XTemp = ThisSpark->X + (( PhaseTable[ ThisSpark->ByteA ] * ThisSpark->Heat ) >> 8);
				BYTE YTemp = ThisSpark->Y + (( PhaseTable[ ThisSpark->ByteB ] * ThisSpark->Heat ) >> 8);

				DWORD SparkDest = (DWORD)( (XTemp & UMask) + ( (YTemp & VMask) << UBits )  );
				GetMip(0)->DataArray(SparkDest) = Htemp;			

				ThisSpark->ByteA += ThisSpark->ByteC;
				ThisSpark->ByteB += ThisSpark->ByteD;
            }
            break;

		
		case SPARK_LissajX: // Special case: Lissajous without Y-movement.
			{  
				BYTE XTemp = ThisSpark->X + (( PhaseTable[ ThisSpark->ByteA ] * ThisSpark->Heat ) >> 8);
				BYTE Htemp = LightPhaseTable[ (BYTE)(ThisSpark->ByteA+64) ]; // Light phase == B

				DWORD SparkDest = (DWORD)( (XTemp & UMask) + ( ThisSpark->Y << UBits )  );
				GetMip(0)->DataArray(SparkDest) = Htemp;			

				ThisSpark->ByteA += ThisSpark->ByteC;
            }
            break;

		case SPARK_LissajY: // Special case: Lissajous without X-movement.
			{  
				BYTE YTemp = ThisSpark->Y + (( PhaseTable[ ThisSpark->ByteB ] * ThisSpark->Heat ) >> 8);
				BYTE Htemp = LightPhaseTable[ (BYTE)(ThisSpark->ByteB+64) ]; // Light phase == B

				DWORD SparkDest = (DWORD)( ThisSpark->X + ( (YTemp & VMask) << UBits )  );
				GetMip(0)->DataArray(SparkDest) = Htemp;

				ThisSpark->ByteB += ThisSpark->ByteD;
            }
            break;

        case SPARK_Blaze: // Emit sparks pseudo-radially.
            if ( (ActiveSparkNum < (SparksLimit)) && ( 128 > SpeedRand() ) )
            {   // create it..
                INT  NS = ActiveSparkNum++;
	            Sparks(NS).Type = ISPARK_Drifter;  // Dynamic type.
				Sparks(NS).Heat = ThisSpark->Heat; // Start heat 
                Sparks(NS).X = ThisSpark->X;
				Sparks(NS).Y = ThisSpark->Y;             
                Sparks(NS).ByteA = SpeedRand();  // Speed
                Sparks(NS).ByteB = SpeedRand();  // Speed
				Sparks(NS).ByteC = ThisSpark->ByteC;
				Sparks(NS).ByteD = ThisSpark->ByteD;
            }
            break;

        case SPARK_OzHasSpoken: // V-shaped output.
            if ( (ActiveSparkNum < (SparksLimit)) && ( 128 > SpeedRand() ) )
            {   // create it..
                INT  NS = ActiveSparkNum++;
                Sparks(NS).Type  = ISPARK_DriftSlow;
                Sparks(NS).Heat  = ThisSpark->Heat;        // Start heat.
                Sparks(NS).X     = ThisSpark->X;
                Sparks(NS).Y     = ThisSpark->Y;
				Sparks(NS).ByteA = (SpeedRand()&127) - 63; // X speed arbit.
                Sparks(NS).ByteB = (BYTE)-127;             // Y speed UP..
				Sparks(NS).ByteD = 2;  // Life decrement.
            }
            break;

        case SPARK_Cone: // Symmetric gravity-emitting - sparks of type 130.
            if ( (ActiveSparkNum < (SparksLimit)) && ( 64 > SpeedRand() ) )
            {   // create it..
                INT  NS = ActiveSparkNum++;
                Sparks(NS).Type = ISPARK_Faller;
				Sparks(NS).Heat = ThisSpark->Heat;   // Heat.
                Sparks(NS).X = ThisSpark->X;
                Sparks(NS).Y = ThisSpark->Y;

                Sparks(NS).ByteA = (SpeedRand()&127) - 63; // X speed arbit.
                Sparks(NS).ByteB =  0;  // Y speed UP
                Sparks(NS).ByteC =  50; // Timer.
            }
            break;

        case SPARK_BlazeRight: // Erupt to the right.
            if ( (ActiveSparkNum < (SparksLimit)) && ( 64 > SpeedRand() ) )
            {   // Create it.
                INT  NS = ActiveSparkNum++;
                Sparks(NS).Type = ISPARK_Faller; //
				Sparks(NS).Heat = ThisSpark->Heat;  // Heat.
				Sparks(NS).X = ThisSpark->X;
                Sparks(NS).Y = ThisSpark->Y;
                Sparks(NS).ByteA = (SpeedRand()&63) + 63; // X Speed.
                Sparks(NS).ByteB = (BYTE) -29; // Y speed UP.
                Sparks(NS).ByteC =  ThisSpark->ByteC; // Timer.
            }
            break;

        case SPARK_BlazeLeft: // Erupt to the left.
            if ( (ActiveSparkNum < (SparksLimit)) && ( 64 > SpeedRand() ) )
            {   // Create it.
                INT  NS = ActiveSparkNum++;
                Sparks(NS).Type = ISPARK_Faller;
				Sparks(NS).Heat = ThisSpark->Heat;	  // Heat.
				Sparks(NS).X = ThisSpark->X;
                Sparks(NS).Y = ThisSpark->Y;
                Sparks(NS).ByteA = (SpeedRand()&63) -128; // X speed 
                Sparks(NS).ByteB = (BYTE) -29;  // Y speed UP
                Sparks(NS).ByteC =  ThisSpark->ByteC; // Timer.
            }
            break;

        case SPARK_Emit: // Erupt to a preset speed & direction.
            if ( (ActiveSparkNum < (SparksLimit)) && ( 64 > SpeedRand() ) )
            {   // Create it..
                INT  NS = ActiveSparkNum++;
                Sparks(NS).Type = ISPARK_DriftSlow;
				Sparks(NS).Heat = ThisSpark->Heat; // heat
                Sparks(NS).X = ThisSpark->X;
                Sparks(NS).Y = ThisSpark->Y;
                Sparks(NS).ByteA = ThisSpark->ByteA; // X speed
                Sparks(NS).ByteB = ThisSpark->ByteB; // Y speed
				Sparks(NS).ByteD = ThisSpark->ByteD; // Decrement
            }
            break;

		case SPARK_Fountain: // Erupt to a preset speed & direction.
            if ( (ActiveSparkNum < (SparksLimit)) && ( 64 > SpeedRand() ) )
            {   // Create it..
                INT  NS = ActiveSparkNum++;
                Sparks(NS).Type  = ISPARK_Graviton;
				Sparks(NS).Heat  = ThisSpark->Heat; // heat
                Sparks(NS).X     = ThisSpark->X;
                Sparks(NS).Y     = ThisSpark->Y;
                Sparks(NS).ByteA = ThisSpark->ByteA; // X speed
                Sparks(NS).ByteB = ThisSpark->ByteB; // Y speed
				Sparks(NS).ByteD = ThisSpark->ByteD; // Decrement
            }
        	break;


		case SPARK_Organic: // Emitting - sparks of type SPARK_VShooter.
     						// Whirly-floaty fire sparks, go up & glow out.
            if ( (ActiveSparkNum < (SparksLimit)) && ( 128 > SpeedRand() ) )
            {   // create it..
                INT  NS = ActiveSparkNum++;
                Sparks(NS).Type = ISPARK_VShooter; //
				Sparks(NS).X = UMask & (ThisSpark->X + ((SpeedRand() * ThisSpark->ByteC) >> 8));
				Sparks(NS).Y = VMask & (ThisSpark->Y + ((SpeedRand() * ThisSpark->ByteC) >> 8));
                Sparks(NS).ByteA = SpeedRand() - 127; // X speed arbit.
                Sparks(NS).ByteB = 256-127;           // Y speed UP
                Sparks(NS).ByteC = 255;				  // Timer==heat - steps --2, to 128
            }
            break;

        case SPARK_WanderOrganic: // Emitting VShooters but randomly moves itself.
            if ( (ActiveSparkNum < (SparksLimit)) )
            {   // create it..
                INT  NS = ActiveSparkNum++;
                Sparks(NS).Type = ISPARK_VShooter;
                Sparks(NS).X = UMask & (ThisSpark->X + (SpeedRand()&31));
                Sparks(NS).Y = VMask & (ThisSpark->Y + (SpeedRand()&31));

                Sparks(NS).ByteA = SpeedRand() - 127; // X speed
                Sparks(NS).ByteB = 256-127;           // Y speed UP
                Sparks(NS).ByteC = 255;               // timer - steps --2, to 128
            }
            // move around a bit
            if (SpeedRand() & 15 == 15) ThisSpark->X = UMask & (ThisSpark->X+(SpeedRand()&15)-7);
            if (SpeedRand() & 15 == 15) ThisSpark->Y = VMask & (ThisSpark->Y+(SpeedRand()&15)-7);
            break;

        case SPARK_RandomCloud: // Emitting Drop but randomly moves itself eratically.
            if ( (ActiveSparkNum < (SparksLimit)) )
            {   // create it..
                INT  NS = ActiveSparkNum++;
                Sparks(NS).Type =  ISPARK_Drop;
                Sparks(NS).X = UMask & (ThisSpark->X + (SpeedRand()&31));
                Sparks(NS).Y = VMask & (ThisSpark->Y + (SpeedRand()&31));
                Sparks(NS).ByteA = (SpeedRand()&31) - 15; // X speed
                Sparks(NS).ByteB =  256-127;              // Y speed UP
                Sparks(NS).ByteC =  0;                    // timer=HEAT
            }
            // move around a bit
            if (SpeedRand() & 15 == 15) ThisSpark->X = UMask & (ThisSpark->X+(SpeedRand()&15)-7);
            if (SpeedRand() & 15 == 15) ThisSpark->Y = VMask & (ThisSpark->Y+(SpeedRand()&15)-7);
            break;

        case SPARK_Eels: // Spawns some Eels.
            if  ( (SpeedRand()<20) &&  ( (ActiveSparkNum < (SparksLimit)) ) )
            {
                // create it..
                INT  NS = ActiveSparkNum++;
				Sparks(NS).Heat  = ThisSpark->Heat;
                Sparks(NS).Type = ISPARK_SpawnedEel; //
                Sparks(NS).X = UMask & (ThisSpark->X + (SpeedRand()&31));
                Sparks(NS).Y = VMask & (ThisSpark->Y + (SpeedRand()&31));
                Sparks(NS).ByteA = SpeedRand();
                Sparks(NS).ByteB = SpeedRand();
                Sparks(NS).ByteC = ThisSpark->ByteC;  // Timer.
            }
            // move around a bit
            if (SpeedRand() & 15 == 15) ThisSpark->X = UMask & (ThisSpark->X+(SpeedRand()&15)-7);
            if (SpeedRand() & 15 == 15) ThisSpark->Y = VMask & (ThisSpark->Y+(SpeedRand()&15)-7);
            break;

		case SPARK_Gametes: // Spawns a spermatozoid.
            if  ( (SpeedRand()<20) &&  ( (ActiveSparkNum < (SparksLimit)) ) )
            {
				// Create it.
                INT  NS = ActiveSparkNum++;
                Sparks(NS).Type = ISPARK_SpawnedSperm; //
				Sparks(NS).Heat = ThisSpark->Heat;
                Sparks(NS).X = UMask & (ThisSpark->X + (SpeedRand()&31));
                Sparks(NS).Y = VMask & (ThisSpark->Y + (SpeedRand()&31));
                Sparks(NS).ByteA = SpeedRand(); // wriggle counter
                Sparks(NS).ByteC = ThisSpark->ByteC;  // Timer.
				Sparks(NS).ByteD = SpeedRand(); // Direction
            }
            // Move around a bit.
            if (SpeedRand() & 15 == 15) ThisSpark->X = UMask & (ThisSpark->X+(SpeedRand()&15)-7);
            if (SpeedRand() & 15 == 15) ThisSpark->Y = VMask & (ThisSpark->Y+(SpeedRand()&15)-7);
            break;

        case SPARK_CustomCloud:  // Custom CLOUDS that move at DrawByteA's speed.
            if ( (ActiveSparkNum < (SparksLimit)) )
            {
                // Create it.
                INT  NS = ActiveSparkNum++;
                Sparks(NS).Type = ISPARK_Move; //
                Sparks(NS).X = UMask & (ThisSpark->X + (SpeedRand()&31));
                Sparks(NS).Y = VMask & (ThisSpark->Y + (SpeedRand()&31));
                Sparks(NS).ByteA = ThisSpark->ByteA; // X speed
                Sparks(NS).ByteB = ThisSpark->ByteB; // Y speed
                Sparks(NS).ByteC = ThisSpark->ByteD; // timer==heat
            }

            // Move around a bit.
            ThisSpark->X = UMask & (ThisSpark->X+(SpeedRand()&7)-(SpeedRand()&7));
            ThisSpark->Y = VMask & (ThisSpark->Y+(SpeedRand()&7)-(SpeedRand()&7));
            break;

		case SPARK_LocalCloud: // Custom clouds that move at DrawByteA's speed.
            if ( (ActiveSparkNum < (SparksLimit)) )
            {
                // create it..
                INT  NS = ActiveSparkNum++;
                Sparks(NS).Type = ISPARK_Move; //
				Sparks(NS).X = UMask & (ThisSpark->X + ((SpeedRand() * ThisSpark->ByteC) >> 8));
				Sparks(NS).Y = VMask & (ThisSpark->Y + ((SpeedRand() * ThisSpark->ByteC) >> 8));
                Sparks(NS).ByteA = ThisSpark->ByteA;  
                Sparks(NS).ByteB = ThisSpark->ByteB; 
                Sparks(NS).ByteC = ThisSpark->ByteD; // timer==heat
            }
            break;

        case SPARK_Flocks: // CLOUDS that move at DrawByteA's speed...
            if ( (ActiveSparkNum < (SparksLimit)) )
            {
                // Create it..
                INT  NS = ActiveSparkNum++;
                Sparks(NS).Type  = ISPARK_SpawnedTwirl;
                Sparks(NS).X     = UMask & (ThisSpark->X + (SpeedRand()&31));
                Sparks(NS).Y     = VMask & (ThisSpark->Y + (SpeedRand()&31));
				Sparks(NS).ByteB = ThisSpark->ByteA;  // Initial direction.
				Sparks(NS).ByteA = 0;                 // low byte
                Sparks(NS).ByteC = ThisSpark->ByteB;  // Timer / Size 
				Sparks(NS).ByteD = ThisSpark->ByteD;  // Angle delta
				Sparks(NS).Heat  = ThisSpark->Heat;

				//if ( SpeedRand()<1 ) 
				//	ThisSpark->ByteA = SpeedRand(); // change about every XX sparks
				//else
				ThisSpark->ByteA += ThisSpark->ByteC; // Turn spawn direction.
            }
            // move around a bit
            ThisSpark->X = UMask & ( ThisSpark->X + (SpeedRand()&7) - (SpeedRand()&7) );
            ThisSpark->Y = VMask & ( ThisSpark->Y + (SpeedRand()&7) - (SpeedRand()&7) );
            break;

		case SPARK_Wheel:  // CLOUDS that move at DrawByteA's speed.
            if ( (ActiveSparkNum < (SparksLimit)) )
            {   // Create it..
                INT  NS = ActiveSparkNum++;
                Sparks(NS).Type = ISPARK_SpawnedTwirl;
                Sparks(NS).X = ThisSpark->X ;
                Sparks(NS).Y = ThisSpark->Y ;
                Sparks(NS).ByteB  = ThisSpark->ByteA;      // Initial Direction.
				Sparks(NS).ByteA  = 0;                     // low byte
                Sparks(NS).ByteC  = ThisSpark->ByteB;      // Timer/ Size
				Sparks(NS).ByteD  = ThisSpark->ByteD;      // 2 << 3 ; //ThisSpark->ByteD;      // Angle delta.
				Sparks(NS).Heat   = ThisSpark->Heat;
            }
			ThisSpark->ByteA += ThisSpark->ByteC;      // Turn spawn direction.
            break;

		case SPARK_Sprinkler:  
            if ( (ActiveSparkNum < (SparksLimit)) )
            {   // Create it..
                INT  NS = ActiveSparkNum++;
                Sparks(NS).Type = ISPARK_SprinklerTwirl;
                Sparks(NS).X = ThisSpark->X ;
                Sparks(NS).Y = ThisSpark->Y ;
				Sparks(NS).Heat = ThisSpark->Heat;
                Sparks(NS).ByteA  = ThisSpark->ByteA;      // Initial Direction.
                Sparks(NS).ByteB  = ThisSpark->ByteB;      // 
				Sparks(NS).ByteC  = ThisSpark->ByteC;
				Sparks(NS).ByteD  = 2; //ThisSpark->ByteD;      // Angle delta.
            }
			ThisSpark->ByteA += ThisSpark->ByteD;      // Turn spawn direction.
            break;

        case SPARK_Stars: // FIXED STARS ! dim according to fire Intensity..
			{
            // called here just BEFORE update: so RESTORE original pixel.
            // ByteA has the star itself, ByteB the saved bckgrnd
				DWORD SparkDest = (DWORD)(ThisSpark->X + (ThisSpark->Y << UBits ));
				GetMip(0)->DataArray(SparkDest) = ThisSpark->ByteB;
			}
            break;

        case SPARK_LineLightning:     // Emitting lightning - random bursts, FIXED locations.
            if (ThisSpark->Heat == 0) // Too-small bolts have Heat 0
                break;
            // Flash in progress ?
            if (ThisSpark->ByteC  > 0)
            {
                ThisSpark->ByteC--;  // Countdown effect
                LineSeg LL;
                LL.Xlen = ThisSpark->ByteA;
                LL.Ylen = ThisSpark->ByteB;
                LL.Xpos = ThisSpark->X;
                LL.Ypos = ThisSpark->Y;
				BYTE HeatA = ThisSpark->Heat;
				DrawFlashRamp(LL, HeatA, HeatA);
            }
            else
            if (SpeedRand() >= ThisSpark->ByteD)
            // Initiate new flash ?
            {
                ThisSpark->ByteC = 1+ SpeedRand() & 5;
            }
            break;

        case SPARK_RampLightning:     // Emitting lightning - random bursts, FIXED locations.
            if (ThisSpark->Heat == 0) // Too-small bolts have heat 0
                break;
            // Flash in progress ?
            if (ThisSpark->ByteC  > 0)
            {
                ThisSpark->ByteC--; // Countdown.
                LineSeg LL;
                LL.Xlen = ThisSpark->ByteA;
                LL.Ylen = ThisSpark->ByteB;
                LL.Xpos = ThisSpark->X;
                LL.Ypos = ThisSpark->Y;
				BYTE HeatA = ThisSpark->Heat;
				BYTE HeatB = HeatA >> 3;
                DrawFlashRamp( LL, HeatA, HeatB);
            }
            else
            if (SpeedRand() >= ThisSpark->ByteD)
            // Initiate new flash ?
            {
                ThisSpark->ByteC = 1 + SpeedRand() & 5;
            }
            break;

        case SPARK_SphereLightning: // Radial lightning from source point.
            if (SpeedRand() >= ThisSpark->ByteD) // Frequency.
            {
                INT  SparkAngle = SpeedRand();
                INT  Radius = ThisSpark->ByteC;

				BYTE  Col1 = ThisSpark->Heat;
				BYTE  Col2 = ThisSpark->Heat >> 2; // Taper off to (relative) darkness.

                INT  SdispX = ( Radius * ( (int)PhaseTable[SparkAngle] ) ) >> 8;
                INT  SdispY = ( Radius * ( (int)PhaseTable[(SparkAngle+64) & 255] ) ) >> 8;

  				LineSeg LL;

                INT  Xlen =  (int)SdispX - (int)(Radius/2);
                INT  Ylen =  (int)SdispY - (int)(Radius/2);

				LL.Xpos = ThisSpark->X;
				LL.Ypos = ThisSpark->Y;

                // Cram the sign bit into the lsbit.
				if (Xlen<=0)	Xlen  = (- Xlen) | 1;
                    else		Xlen &= 0xFFFFFFFE;
				if (Ylen<=0)	Ylen  = (- Ylen) | 1;
                    else		Ylen &= 0xFFFFFFFE;

				LL.Xlen = Xlen;
				LL.Ylen = Ylen;

                DrawFlashRamp( LL, Col1,Col2);
            }
            break;

        //
        //  Transient /spawned sparks, emitted by lower spark types.
        //

        //  Dynamic emitted sparks. General linear 'drifter', limited lifetime.
        case ISPARK_Drifter:
            if ( (( ThisSpark->Heat -= 5 ) < 251 )  )  // Glow out fast.
            {
                DWORD SparkDest = (DWORD) (ThisSpark->X + ( ThisSpark->Y << UBits ));
                // Set spark.
                GetMip(0)->DataArray(SparkDest) = ThisSpark->Heat;
                // Advance position.
                MoveSpark( ThisSpark );
                // No gravity.
            }
            else  // Delete the spark.
				*ThisSpark = Sparks(--ActiveSparkNum);
            break;


        // Dynamic emitted sparks. Linear movement, longer lifetime.
        case ISPARK_DriftSlow:
            if (( ThisSpark->Heat -= ThisSpark->ByteD ) > ThisSpark->ByteD ) // Glow out.
            {
                DWORD SparkDest = (DWORD)( ThisSpark->X + ( ThisSpark->Y << UBits ) );
                // Set spark.
                GetMip(0)->DataArray( SparkDest ) = ThisSpark->Heat;
                // Advance position.
                MoveSpark( ThisSpark );
                // No gravity.
            }
            else // Delete the spark.
				*ThisSpark = Sparks( --ActiveSparkNum );
            break;


        //  Dynamic emitted sparks. General: adjustable lifetime and speed.
        case ISPARK_Custom:
            if   ( (( ThisSpark->Heat -= ThisSpark->ByteC ) < 250 )  )  // Glow out slower.
            {
                DWORD SparkDest = (DWORD)( ThisSpark->X + ( ThisSpark->Y << UBits ));
                // Set spark.
                GetMip(0)->DataArray(SparkDest) = ThisSpark->Heat;
                // Advance position.
                MoveSpark( ThisSpark );
                // No gravity.
            }
            else  // Delete the spark.
				*ThisSpark = Sparks(--ActiveSparkNum);
            break;


        //  Dynamic falling ones: GRAVITY. if out of range, delete.
        case ISPARK_Faller:
            if ( (( ThisSpark->ByteC -= 1 ) > 0 )  )  // Timeout.
            {
                DWORD SparkDest = (DWORD)( ThisSpark->X + ( ThisSpark->Y << UBits ));
                // Set spark.
                GetMip(0)->DataArray(SparkDest) = ThisSpark->Heat;
                // Advance position.
                MoveSpark( ThisSpark );
                // Gravity:
                if ((CHAR)ThisSpark->ByteB < 122)
                    ThisSpark->ByteB +=3;
            }
            else  // Delete the spark.
                *ThisSpark = Sparks(--ActiveSparkNum);
            break;

		//  Dynamic falling ones for _Fountain.
		case ISPARK_Graviton:
			if    (( ThisSpark->Heat -= ThisSpark->ByteD ) > LOWESTGLOW )  // Glow out 
            {
                DWORD SparkDest = (DWORD)( ThisSpark->X + ( ThisSpark->Y << UBits ));

                // Set spark.
                GetMip(0)->DataArray(SparkDest ) = ThisSpark->Heat;

                // Advance position.
                MoveSpark( ThisSpark ); 

                // 1/2 Gravity.
				if (GlobalPhase & 1)
				{					
					if ((CHAR)ThisSpark->ByteB < 124)
						ThisSpark->ByteB +=3;
				}
            }
            else  // Delete the spark.
				*ThisSpark = Sparks(--ActiveSparkNum);
            break;


        //  Whirly-floaty fire sparks, go up & glow out.
        case ISPARK_VShooter:
            if   ((( ThisSpark->ByteC -= 3 ) > 190 )  )  // Timeout.
            {
                DWORD SparkDest = (DWORD)( ThisSpark->X + ( ThisSpark->Y << UBits ));
                // Set spark.
                GetMip(0)->DataArray(SparkDest) = (BYTE) (ThisSpark->ByteC);
                // Advance position.
                MoveSparkTwo( ThisSpark );
            }
            else  // Delete the spark.
                *ThisSpark = Sparks(--ActiveSparkNum);
            break;


        //  Whirly-floaty fire sparks, go up & glow out.
        case ISPARK_Drop:
            if ((( ThisSpark->ByteC += 4 ) < 250 )  )  // Timeout.
            {
                DWORD SparkDest = (DWORD)( ThisSpark->X + ( ThisSpark->Y << UBits ));
                // Set spark.
                GetMip(0)->DataArray(SparkDest) = ThisSpark->ByteC;
                // Advance position.
                MoveSparkTwo( ThisSpark );
            }
            else // Delete the spark.
                *ThisSpark = Sparks(--ActiveSparkNum);
            break;


        //  Whirly-floaty fire sparks, go down & glow out.
        case ISPARK_Move:
            if   ((( ThisSpark->ByteC += 4 ) < 250 )  )  // Timeout
            {
                DWORD SparkDest = (DWORD)( ThisSpark->X + ( ThisSpark->Y << UBits ));
                // Set spark.
                GetMip(0)->DataArray(SparkDest) = ThisSpark->ByteC;
                // Advance position.
                MoveSpark( ThisSpark );
            }
            else  // Delete the spark.
                *ThisSpark = Sparks(--ActiveSparkNum);
            break;


        //  'Wormy' sparks, long-lived and sparse.
        case ISPARK_SpawnedEel:
			if   ((( ThisSpark->ByteC -= 1 ) < 255 )  )  // Timeout
            {
	            DWORD SparkDest = (DWORD)( ThisSpark->X + ( ThisSpark->Y << UBits ) );
                // Set spark.
                GetMip(0)->DataArray(SparkDest) = ThisSpark->Heat;
                // (BYTE) (ThisSpark->ByteC);
                // Advance position.
                MoveSpark( ThisSpark );
            }
            else  // Delete the spark.
                *ThisSpark = Sparks(--ActiveSparkNum);
			break;


		//  Cruising sparks, long-lived and sparse.
        case ISPARK_SpawnedSperm:
			if   ((( ThisSpark->ByteC -= 1 ) < 255 )  )  // Timeout
            {
	            DWORD SparkDest = (DWORD)( ThisSpark->X + ( ThisSpark->Y << UBits ) );
                // Set spark.
                GetMip(0)->DataArray(SparkDest) = ThisSpark->Heat;

				// Jiggle direction = ByteD using a zigzaggy ByteA.
				BYTE SawTooth = 127 & (ThisSpark->ByteA += 7);
				if (SawTooth>63) SawTooth = 127-SawTooth;

                // Advance position, in direction of angle SawTooth+ByteD.
				MoveSparkAngle(ThisSpark, 255& (SawTooth + ThisSpark->ByteD) );
            }
            else  // Delete the spark.
                *ThisSpark = Sparks(--ActiveSparkNum);
			break;


		//   Moves in semicircular motions.		
		case ISPARK_SpawnedTwirl:
            if   ((( ThisSpark->ByteC -= 1 ) < 255 )  )  // Timeout.
            {
	            DWORD SparkDest = (DWORD)(ThisSpark->X + ( ThisSpark->Y << UBits ) );
                // Set spark.
                GetMip(0)->DataArray(SparkDest) = ThisSpark->Heat;

				//BYTE TempSpeedX   =  -128 +  (CHAR)PhaseTable[(ThisSpark->ByteB+64 ) & 255];
				//BYTE TempSpeedY   = (-128 + ((CHAR)PhaseTable[ ThisSpark->ByteB ]) );

				BYTE TempSpeedX = SignedPhaseTable[ (ThisSpark->ByteB+64 ) & 255 ];
				BYTE TempSpeedY = SignedPhaseTable[  ThisSpark->ByteB ];

				*(_WORD*) &(ThisSpark->ByteA) +=  (_WORD) ThisSpark->ByteD << 4 ;   // Add angle delta (4:4) "fixed"-point)

				// Advance position.
                MoveSparkXY( ThisSpark, TempSpeedX, TempSpeedY ); 
            }
            else // Delete the spark.
                *ThisSpark = Sparks(--ActiveSparkNum);
			break;


		//   Moves in pseudoturbulent motions.		
		case ISPARK_SprinklerTwirl:
            if   ((( ThisSpark->ByteC -= 1 ) < 255 )  )  // Timeout.
            {
	            DWORD SparkDest = (DWORD)(ThisSpark->X + ( ThisSpark->Y << UBits ));
                // Set spark.
                GetMip(0)->DataArray(SparkDest) = ThisSpark->Heat; 
				BYTE TempSpeedX   =  -128 + (CHAR)PhaseTable[(ThisSpark->ByteA+64 ) & 255]; 
				BYTE TempSpeedY   =  ThisSpark->ByteB; 
				ThisSpark->ByteA +=  ThisSpark->ByteD; 
				// Rotate angle.
                // (BYTE) (ThisSpark->ByteC);
                // Advance position.
                MoveSparkXY( ThisSpark, TempSpeedX, TempSpeedY );
            }
            else // Delete the spark.
                *ThisSpark = Sparks(--ActiveSparkNum);
			break;

        } //Switch.
	}
	unguard;
}



void UFireTexture::PostDrawSparks()
{
	guard(UFireTexture::PostDrawSparks);

    DWORD SparkDest;

	BYTE  FoundStar = 0;

	if (StarStatus == 0) return; // No stars so don't search the array.
		
	for( INT  S = 0; S < ActiveSparkNum; S++ )
	{
        if  (Sparks(S).Type == SPARK_Stars)
		{
			FoundStar = 1;
			//  Fixed stars disappear depending to fire Intensity.
			//  called here after the update: so restore the original star.
			//  ByteA has the star itself, ByteB the saved bckgrnd

			SparkDest = (DWORD)(Sparks(S).X
			+ (Sparks(S).Y << UBits ) );

			//  Save real sky value.
			BYTE DimStar = GetMip(0)->DataArray(SparkDest);
			Sparks(S).ByteB = DimStar;

			//  Put in the star if under cutoff.
			if ( DimStar < STAR_CUTOFF )
				GetMip(0)->DataArray(SparkDest) = Sparks(S).ByteA;
		}
    }

	if (FoundStar == 0)  StarStatus = 0;

	unguard;
}



/*---------------------------------------------------------------------------
	All water code
---------------------------------------------------------------------------*/

//
// WATER interpolated calculation
//
// Note that this C++ water code is rather 'spun out', it was
// designed as a test for the way the update is done in assembler.
// Most of the code results from having to make the water wrap correctly.
//

#if WATERASM

extern "C" void  CalcWaterASM( BYTE* PBitmap, BYTE* PWavMap, BYTE* PRendTable, BYTE* PWaveTable, DWORD XVar, DWORD YVar, DWORD Parity );

void UWaterTexture::CalculateWater( )
{
	WaveParity++;
	CalcWaterASM
	(
		&GetMip(0)->DataArray(0),
		SourceFields,
        RenderTable,
        WaveTable,
        USize,
        VSize,
        WaveParity
    );
}

#else


#define Output4Pix(SourceA,SourceC,SourceE,SourceG,SourceB,SourceD,SourceF,SourceH,DestCell,Dest1,Dest2,Dest3,Dest4)\
{\
*DestCell = *( WaveTable + 512 +\
               ( (int)SourceE\
               + (int)SourceG\
               + (int)SourceF\
               + (int)SourceH\
               )\
               - ( ((int)*DestCell)  << 1 )\
               );\
\
  INT  _EA = (int)SourceE-(int)SourceA;\
  INT  _FB = (int)SourceF-(int)SourceB;\
  INT  _GC = (int)SourceG-(int)SourceC;\
  INT  _HD = (int)SourceH-(int)SourceD;\
 \
  *Dest2 = *( RenderTable + 512 +  _GC + _HD );\
  *Dest3 = *( RenderTable + 512 +  _FB + _HD );\
  *Dest1 = *( RenderTable + 512 + (_FB+_HD+_EA+_GC)/2 );\
  *Dest4 = *( RenderTable + 512 +  _HD + _HD );\
}



//
// Interpolated water, C++ version.
//

void UWaterTexture::CalculateWater()
{
	guard(UWaterTexture::CalculateWater);

	BYTE* BitMapAddr  = &GetMip(0)->DataArray(0);   //pointer

    //BYTE* RenderTable = (BYTE*) &RenderTable;  // actual table
    //BYTE*   WaveTable = (BYTE*) &WaveTable;    // actual table

    DWORD Xdimension = USize/2;
    DWORD Ydimension = VSize/2;

    BYTE* WaterMap; // map1 & map 2: interleaved...

    WaterMap = SourceFields; // Ptr to interleaved simulated water field.

    WaveParity++;   // odd/even counter.

    INT  TotalSize = 2 * Xdimension * Ydimension;

    BYTE* DestCell;
    INT  DestPixel;


    if ((WaveParity & 1) == 0)
    {

    // EVEN water: source =  cells on ODD lines, destin = EVEN lines
    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // Start with UPPER row of even, 2 wrap up+left at beginning, n-2 all wrap up.

    DestCell  = (WaterMap+0);
    DestPixel = 0; // Always lower right pixel dest.

    Output4Pix(
                  *(DestCell+TotalSize-3),
                  *(DestCell+TotalSize-2),
                  *(DestCell+TotalSize-1),
                  *(DestCell+TotalSize-0-Xdimension),
                  *(DestCell-3 + Xdimension*2),
                  *(DestCell-2 + Xdimension*2),
                  *(DestCell-1 + Xdimension*2),
                  *(DestCell-0 + Xdimension  ),
                  DestCell,
                  (BitMapAddr+DestPixel-1               +TotalSize*2),  //
                  (BitMapAddr+DestPixel   - Xdimension*2+TotalSize*2),  //  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                  (BitMapAddr+DestPixel-1 + Xdimension*2),              //  3 4
                  (BitMapAddr+DestPixel) );                             //
                 

    DestCell++;
    DestPixel+=2;

    Output4Pix (
                  *(DestCell+TotalSize-3),
                  *(DestCell+TotalSize-2),
                  *(DestCell+TotalSize-1-Xdimension),
                  *(DestCell+TotalSize-0-Xdimension),
                  *(DestCell-3 + Xdimension*2),
                  *(DestCell-2 + Xdimension*2),
                  *(DestCell-1 + Xdimension  ),
                  *(DestCell-0 + Xdimension  ),
                  DestCell,
                  (BitMapAddr+DestPixel-1 - Xdimension*2+TotalSize*2) , //
                  (BitMapAddr+DestPixel   - Xdimension*2+TotalSize*2) , //  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                  (BitMapAddr+DestPixel-1),								//  3 4
                  (BitMapAddr+DestPixel)   );							//
                  

    DestCell++;
    DestPixel+=2;

    Output4Pix (
                  *(DestCell+TotalSize-3),
                  *(DestCell+TotalSize-2-Xdimension),
                  *(DestCell+TotalSize-1-Xdimension),
                  *(DestCell+TotalSize-0-Xdimension),
                  *(DestCell-3 + Xdimension*2),
                  *(DestCell-2 + Xdimension  ),
                  *(DestCell-1 + Xdimension  ),
                  *(DestCell-0 + Xdimension  ),
                  DestCell,
                  (BitMapAddr+DestPixel-1 -Xdimension*2+TotalSize*2) , //
                  (BitMapAddr+DestPixel   -Xdimension*2+TotalSize*2) , //  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                  (BitMapAddr+DestPixel-1),							   //  3 4
                  (BitMapAddr+DestPixel)  );						   //
                  


    /// Because of way ASM works (saved results) ASM needs only 2 wrappers.

    for (int X = 3; X < (int)Xdimension  ; X++ ) // Total Xdimension-1 cells
           {
    DestCell++;
    DestPixel+=2;

    Output4Pix (
                  *(DestCell+TotalSize-3-Xdimension),
                  *(DestCell+TotalSize-2-Xdimension),
                  *(DestCell+TotalSize-1-Xdimension),
                  *(DestCell+TotalSize-0-Xdimension),
                  *(DestCell-3 + Xdimension  ),
                  *(DestCell-2 + Xdimension  ),
                  *(DestCell-1 + Xdimension  ),
                  *(DestCell-0 + Xdimension  ),
                  DestCell,
                  (BitMapAddr+DestPixel-1 -Xdimension*2+TotalSize*2) , //
                  (BitMapAddr+DestPixel   -Xdimension*2+TotalSize*2) , //  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                  (BitMapAddr+DestPixel-1),                            //  3 4
                  (BitMapAddr+DestPixel)   );                          //
                  

           } //X loop end


    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    for (int Y = 1; Y < (int)Ydimension  ; Y++ ) // total Xdimension-1 cells
    {
         // First 2 wrap, rest always works same way.

        DestCell  = WaterMap+ Y*Xdimension*2 ;
        DestPixel = Y*Xdimension*2*2;          // Always lower right pixel dest.

        //DestCell++;
        //DestPixel+=2;

        Output4Pix (
                      *(DestCell-3),
                      *(DestCell-2),
                      *(DestCell-1),
                      *(DestCell-0 - Xdimension),
                      *(DestCell-3 + Xdimension*2),
                      *(DestCell-2 + Xdimension*2),
                      *(DestCell-1 + Xdimension*2),
                      *(DestCell-0 + Xdimension  ),
                      DestCell,
                      (BitMapAddr+DestPixel-1)               ,  //
                      (BitMapAddr+DestPixel   - Xdimension*2),  //  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                      (BitMapAddr+DestPixel-1 + Xdimension*2),  //  3 4
                      (BitMapAddr+DestPixel) );                 //
                      

        DestCell++;
        DestPixel+=2;

        Output4Pix (
                      *(DestCell-3),
                      *(DestCell-2),
                      *(DestCell-1 - Xdimension),
                      *(DestCell-0 - Xdimension),
                      *(DestCell-3 + Xdimension*2),
                      *(DestCell-2 + Xdimension*2),
                      *(DestCell-1 + Xdimension  ),
                      *(DestCell-0 + Xdimension  ),
                      DestCell,
                      (BitMapAddr+DestPixel-1 -Xdimension*2), //
                      (BitMapAddr+DestPixel   -Xdimension*2), //  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                      (BitMapAddr+DestPixel-1),				//  3 4
                      (BitMapAddr+DestPixel) );				//
                      

        DestCell++;
        DestPixel+=2;

        Output4Pix (
                      *(DestCell-3),
                      *(DestCell-2 - Xdimension),
                      *(DestCell-1 - Xdimension),
                      *(DestCell-0 - Xdimension),
                      *(DestCell-3 + Xdimension*2),
                      *(DestCell-2 + Xdimension  ),
                      *(DestCell-1 + Xdimension  ),
                      *(DestCell-0 + Xdimension  ),
                      DestCell,
                      (BitMapAddr+DestPixel-1 -Xdimension*2)  ,	//
                      (BitMapAddr+DestPixel   -Xdimension*2)  ,	//  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                      (BitMapAddr+DestPixel-1) 				  ,	//  3 4
                      (BitMapAddr+DestPixel) );					//
                      


        /// cuz of way ASM works (saved results) ASM needs only 2 wrappers

        for (int X = 3; X < (int)Xdimension  ; X++ ) // total Xdimension-1 cells
               {
        DestCell++;
        DestPixel+=2;

        Output4Pix (
                      *(DestCell-3 - Xdimension),
                      *(DestCell-2 - Xdimension),
                      *(DestCell-1 - Xdimension),
                      *(DestCell-0 - Xdimension),
                      *(DestCell-3 + Xdimension  ),
                      *(DestCell-2 + Xdimension  ),
                      *(DestCell-1 + Xdimension  ),
                      *(DestCell-0 + Xdimension  ),
                      DestCell,
                      (BitMapAddr+DestPixel-1 -Xdimension*2)  ,	//
                      (BitMapAddr+DestPixel   -Xdimension*2)  ,	//  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                      (BitMapAddr+DestPixel-1) ,				//  3 4
                      (BitMapAddr+DestPixel));					//
                      

               } //X loop end

        } //  Y loop end...

    } //  EVEN water end

    else

    {

    //  ODD water: source = EVEN lines, destin =  ODD lines

    // LAST line...

    DestCell  = (WaterMap+(Ydimension-1)*Xdimension*2 + Xdimension);
    DestPixel = (Ydimension-1)*Xdimension*4 + Xdimension*2  +1;
                // always lower right pixel dest

    Output4Pix (
                  *(DestCell-2),
                  *(DestCell-1),
                  *(DestCell-0-Xdimension),
                  *(DestCell+1-Xdimension),
                  *(DestCell-TotalSize-2 + Xdimension*2),
                  *(DestCell-TotalSize-1 + Xdimension*2),
                  *(DestCell-TotalSize-0 + Xdimension  ),
                  *(DestCell-TotalSize+1 + Xdimension  ),
                  DestCell,
                  (BitMapAddr+DestPixel-1 -Xdimension*2)  ,	//
                  (BitMapAddr+DestPixel   -Xdimension*2)  ,	//  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                  (BitMapAddr+DestPixel-1) ,				//  3 4
                  (BitMapAddr+DestPixel));					//
                  

    DestCell++;
    DestPixel+=2;

    Output4Pix (
                  *(DestCell-2),
                  *(DestCell-1-Xdimension),
                  *(DestCell-0-Xdimension),
                  *(DestCell+1-Xdimension),
                  *(DestCell-TotalSize-2 + Xdimension*2),
                  *(DestCell-TotalSize-1 + Xdimension  ),
                  *(DestCell-TotalSize-0 + Xdimension  ),
                  *(DestCell-TotalSize+1 + Xdimension  ),
                  DestCell,
                  (BitMapAddr+DestPixel-1 -Xdimension*2)  ,	//
                  (BitMapAddr+DestPixel   -Xdimension*2)  ,	//  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                  (BitMapAddr+DestPixel-1) ,				//  3 4
                  (BitMapAddr+DestPixel));					//
                 


    /// cuz of way ASM works (saved results) ASM needs only 2 wrappers

    for (int X = 2; X < (int)(Xdimension-1)  ; X++ ) // total Xdimension-1 cells
           {
    DestCell++;
    DestPixel+=2;

    Output4Pix (
                  *(DestCell-2-Xdimension),
                  *(DestCell-1-Xdimension),
                  *(DestCell-0-Xdimension),
                  *(DestCell+1-Xdimension),
                  *(DestCell-TotalSize-2 + Xdimension  ),
                  *(DestCell-TotalSize-1 + Xdimension  ),
                  *(DestCell-TotalSize-0 + Xdimension  ),
                  *(DestCell-TotalSize+1 + Xdimension  ),
                  DestCell,
                  (BitMapAddr+DestPixel-1 -Xdimension*2)  ,	//
                  (BitMapAddr+DestPixel   -Xdimension*2)  ,	//  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                  (BitMapAddr+DestPixel-1) ,				//  3 4
                  (BitMapAddr+DestPixel));					//
                  

           } //X loop end


    // last one needs SOURCE wrap to right...

    DestCell++;
    DestPixel+=2;

    Output4Pix (
                  *(DestCell-2-Xdimension),
                  *(DestCell-1-Xdimension),
                  *(DestCell-0-Xdimension),
                  *(DestCell+1-Xdimension-Xdimension),
                  *(DestCell-TotalSize-2 + Xdimension  ),
                  *(DestCell-TotalSize-1 + Xdimension  ),
                  *(DestCell-TotalSize-0 + Xdimension  ),
                  *(DestCell-TotalSize+1 + 0 ),
                  DestCell,
                  (BitMapAddr+DestPixel-1 -Xdimension*2)  ,	//
                  (BitMapAddr+DestPixel   -Xdimension*2)  ,	//  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                  (BitMapAddr+DestPixel-1) ,				//  3 4
                  (BitMapAddr+DestPixel) );					//
                  

    //
    //

    for (int Y = 0; Y < (int)(Ydimension-1)  ; Y++ ) // total Xdimension-1 cells
    {
        // First 2 wrap, rest goes always same

        // DestCell++;
        // DestPixel+=2;

        DestCell  = WaterMap+ Y*Xdimension*2 +Xdimension;
        DestPixel = Y*Xdimension*2*2 + Xdimension*2 + 1;

         // Always lower right pixel dest.

        Output4Pix (
                      *(DestCell-2),
                      *(DestCell-1),
                      *(DestCell-0-Xdimension),
                      *(DestCell+1-Xdimension),
                      *(DestCell-2 + Xdimension*2),
                      *(DestCell-1 + Xdimension*2),
                      *(DestCell-0 + Xdimension  ),
                      *(DestCell+1 + Xdimension  ),
                      DestCell,
                      (BitMapAddr+DestPixel-1 -Xdimension*2)  ,   //
                      (BitMapAddr+DestPixel   -Xdimension*2)  ,   //  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                      (BitMapAddr+DestPixel-1) ,				  //  3 4	
                      (BitMapAddr+DestPixel) );					  //
                      

        DestCell++;
        DestPixel+=2;

        Output4Pix (
                      *(DestCell-2),
                      *(DestCell-1-Xdimension),
                      *(DestCell-0-Xdimension),
                      *(DestCell+1-Xdimension),
                      *(DestCell-2 + Xdimension*2),
                      *(DestCell-1 + Xdimension  ),
                      *(DestCell-0 + Xdimension  ),
                      *(DestCell+1 + Xdimension  ),
                      DestCell,
                      (BitMapAddr+DestPixel-1 -Xdimension*2)  ,	//
                      (BitMapAddr+DestPixel   -Xdimension*2)  ,	//  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                      (BitMapAddr+DestPixel-1) ,				//  3 4
                      (BitMapAddr+DestPixel));					//
                      


        /// cuz of way ASM works (saved results) ASM needs only 2 wrappers

        for (int X = 2; X < (int)(Xdimension-1)  ; X++ ) // total Xdimension-1 cells
        {
            DestCell++;
            DestPixel+=2;

            Output4Pix (
                          *(DestCell-2-Xdimension),
                          *(DestCell-1-Xdimension),
                          *(DestCell-0-Xdimension),
                          *(DestCell+1-Xdimension),
                          *(DestCell-2 + Xdimension  ),
                          *(DestCell-1 + Xdimension  ),
                          *(DestCell-0 + Xdimension  ),
                          *(DestCell+1 + Xdimension  ),
                          DestCell,
                          (BitMapAddr+DestPixel-1 -Xdimension*2)  ,	//
                          (BitMapAddr+DestPixel   -Xdimension*2)  ,	//  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                          (BitMapAddr+DestPixel-1) ,				//  3 4
                          (BitMapAddr+DestPixel));					//
                          
        } //X loop end

        // last one needs SOURCE wrap to right...

        DestCell++;
        DestPixel+=2;

        Output4Pix (
                      *(DestCell-2-Xdimension),
                      *(DestCell-1-Xdimension),
                      *(DestCell-0-Xdimension),
                      *(DestCell+1-Xdimension-Xdimension),
                      *(DestCell-2 + Xdimension  ),
                      *(DestCell-1 + Xdimension  ),
                      *(DestCell-0 + Xdimension  ),
                      *(DestCell+1 + 0 ),
                      DestCell,
                      (BitMapAddr+DestPixel-1 -Xdimension*2)  ,	//
                      (BitMapAddr+DestPixel   -Xdimension*2)  ,	//  1 2   4==BitMapAddr+DestPixel*2 (for black/even)
                      (BitMapAddr+DestPixel-1) ,				//  3 4
                      (BitMapAddr+DestPixel));					//
                      

        } //  Y loop end...


    } // ODD water end

	unguard;

} // END of void CalculateWater / interpolating version

#endif


//
//   Apply texture: simple use of the texele itself as displacement-lookup into another texture.
//

void UWetTexture::ApplyWetTexture()
{
	guard(UWetTexture::ApplyWetTexture);

    // Apply texture: SourceTexture to
    // the current (output) texture,
    // using displacement effects.

	// Any source texture selected yet ?
    if (SourceTexture == NULL) return;

    BYTE* BitMapAddr     = &GetMip(0)->DataArray(0);     // pointer

	BYTE* SourceMapAddr;

	if (LocalSourceBitmap) SourceMapAddr = LocalSourceBitmap;
	else		     SourceMapAddr  = &SourceTexture->GetMip(0)->DataArray(0);

    INT  Xdimension = USize;
    INT  Ydimension = VSize;

	int UMask = USize - 1;

#if WETASM

    for ( INT  v=0; v < Ydimension; v++ )
    {
		BYTE* LineStart =  BitMapAddr + (v << UBits);
		BYTE* SourceStart = SourceMapAddr + (v << UBits);

        __asm
        {
            mov     esi,LineStart
            mov     edi,SourceStart

            xor     eax,eax
            push    ebp

            mov     ecx,UMask
			mov     ebp,Xdimension

			xor     edx,edx
            xor     ebx,ebx

			mov     dl,[esi+eax] // move up DL processing - skewed pipe
			sub     ebp,2  // last 2 texels done separately.

            ;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                     ;;
            align 16                 ;;
            CopyLoop:                ;;
                                     ;;
            add     dl,al            ;; Linestart[u]+u
            mov     bl,[esi+eax+1]   ;; Get linestart[u+1] (= water displ.ment)
                                     ;;
            and     dl,cl			 ;;
			add     bl,al			 ;;

            inc     bl               ;; Linestart[u+1] + u+1
            add     al,2			 ;;

            mov     dl,[edi+edx]     ;;
            and     bl,cl			 ;;

            cmp     eax,ebp          ;;
            mov     [esi+eax-2],dl   ;;

            mov     bl,[edi+ebx]     ;;
            mov     dl,[esi+eax]     ;; Get linestart[u] -  pixel 2
                                     ;;
            mov     [esi+eax-2+1],bl ;;
            jb      CopyLoop         ;;
                                     ;;
            ;;;;;;;;;;;;;;;;;;;;;;;;;;;

            nop
			mov     bl,[esi+eax+1]  ;; get pixel 2

			add     dl,al
			add     bl,al

			inc     bl		// ebx	    	   ;;
            and     dl,cl	// edx,ecx         ;;

            pop     ebp     //
            and     bl,cl	// ebx,ecx         ;;

            mov     dl,[edi+edx]    ;; Texel
            mov     bl,[edi+ebx]    ;; Texel

            mov     [esi+eax],dl    ;; Dest
            mov     [esi+eax+1],bl  ;; Dest
        }
    }

#else

    for ( INT  v=0; v < Ydimension; v++ )
    {
		BYTE* LineStart =  BitMapAddr + (v << UBits);
		BYTE* SourceStart = SourceMapAddr + (v << UBits);

		for( INT  u=0; u<Xdimension; u += 2 )
        {

		 // coolish effect combining half warped, half original.
		 // LineStart[u] = (  SourceStart[(u+LineStart[u]) & UMask ] + SourceStart[(u+128) & UMask]  ) >> 1;
		 INT  u2 = u+1;

		 LineStart[ u] =  SourceStart[(  u + LineStart[ u]) & UMask ];
		 LineStart[u2] =  SourceStart[( u2 + LineStart[u2]) & UMask ];

        }
    }

#endif

	unguard;
}



//
// Calculate panning movement for ice.
//

void  UIceTexture::MoveIcePosition(FLOAT DeltaTime)
{
	guard(UIceTexture::MoveIcePosition);

	MasterCount += DeltaTime*120.0f;  //Virtual time ticks.

	UDisplace += 2.0f * DeltaTime * -((CHAR)(HorizPanSpeed - 128));
	VDisplace += 2.0f * DeltaTime *  ((CHAR)(VertPanSpeed  - 128));

	switch (PanningStyle)
	{
		case SLIDE_Linear:
			UPosition  = UDisplace;
			VPosition  = VDisplace;
		break;

		case SLIDE_Circular: 
			UPosition =  UDisplace + appRound( (FLOAT)(Amplitude+1) * appSin( MasterCount * (Frequency+1) * 0.0012f));
			VPosition =  VDisplace + appRound( (FLOAT)(Amplitude+1) * appCos( MasterCount * (Frequency+1) * 0.0012f));
		break;

		case SLIDE_Gestation: // just some out-of-phase movement.
			UPosition =  UDisplace + appRound( (FLOAT)(Amplitude+1) * appSin( MasterCount * (Frequency+1) * 0.0012f));
			VPosition =  VDisplace + appRound( (FLOAT)(Amplitude+1) * appCos( MasterCount * (Frequency+1) * 0.0011f));
		break;

		case SLIDE_WavyX:  // X movement, with Y sinewave
			UPosition =  UDisplace + appRound( 0.5*(FLOAT)(Amplitude+1) * appSin( MasterCount * (Frequency+1) * 0.0012f));
			VPosition =  VDisplace;
		break;

		case SLIDE_WavyY: // Y movement, with X sinewave 			
			UPosition =  UDisplace;
			VPosition =  VDisplace + appRound( 0.5*(FLOAT)(Amplitude+1) * appCos( MasterCount * (Frequency+1) * 0.0012f));
		break;
	}	

	unguard;
}



//
// Move the 'ice' texture over a static source texture.
// Needs cache warming.
//

void UIceTexture::BlitTexIce()
{
	guard(UIceTexture::BlitTexIce);

    // Warning: Source/GlassTexture must have same dimensions.

    BYTE* TexAddr		= &SourceTexture->GetMip(0)->DataArray(0);
	BYTE* GlassAddr		= &GlassTexture->GetMip(0)->DataArray(0);
    BYTE* BitMapAddr	= &GetMip(0)->DataArray(0);  // Pointer.

	if (LocalSourceBitmap) TexAddr = LocalSourceBitmap;
	else				   TexAddr = &SourceTexture->GetMip(0)->DataArray(0);

    INT  Xdimension   = USize;  // Wrap needed for 8-bit counters.
    INT  Ydimension   = VSize;  // 

	INT  TempUMask    = UMask;  //

	static	DWORD ESPStorage;   // Temp ESP  storage.
	static	DWORD EBPStorage;   // Temp EBP  storage.

	// Positionments are (signed) fixed-point 3:4
	int  UDisp =  appRound(UPosition) & UMask;
	int  VDisp =  appRound(VPosition) & VMask;

    for ( INT  v=0; v < Ydimension; v++ )
    {
		BYTE* LineStart   = BitMapAddr +    (v << UBits);
		BYTE* TexStart    = TexAddr    +    (v << UBits);
		BYTE* GlassStart  = GlassAddr  +  (((v + VDisp ) & VMask) << UBits);

#if ICEASM

		if( GIsPentiumPro )
		{  		
		// Pentium PRO/II version 
			__asm                        //
			{                            //
				mov     esi, LineStart   //
				mov     edi, TexStart    //

				mov     ecx, [TempUMask] //
				mov     ebx, Xdimension  //
				xor     eax, eax         //

				mov     ebx, Xdimension  //
				sub     ebx, UDisp       //

				test    ebx, ecx 	     //  zero? - no split 
				jz      ppNoSplit          //

				cmp     ebx, 7           //  
				ja      ppSkipFirstBit     //  Skip if not in first 8 pixels.

				;; split is in first 8 pixels and Unaligned.

				mov     edx, GlassStart  //
				call    ppIceMisc8Pixelz   //
				jmp     ppDoEndChunk       //

			ppNoSplit:                     //
				mov     ebx, Xdimension  //
				mov     edx, GlassStart  //
				jmp     ppDoLastChunk      //

				///////////////////////////

			ppSkipFirstBit:

				and		ebx, 0x0FF-7;     // FF - 3     = stretch before wrapping needed
				mov		edx, GlassStart   //
				add		edx, UDisp        //
				call	ppIce4Npixelz       // 

				mov     ebx, UDisp        //
				and     ebx, 7            //
				jz      ppDoEndChunk        // No unaligned Positionment

				mov		edx, GlassStart   //
				call	ppIceMisc8Pixelz    //

			ppDoEndChunk:                   // 
				cmp		eax,Xdimension    // Any pixels left to do ? 
				je		ppEndIceRender      //

				mov     ebx, Xdimension   // Da end marker
				mov		edx, GlassStart   //
				add     edx, UDisp        // Need to since we're past the skip
				sub     edx, ebx          //
			ppDoLastChunk:
				Call    ppIce4Npixelz       //
				jmp     ppEndIceRender      //

				;;  esi = LineStart
				;;  edi = TexStart
				;;  edx = GlassStart
				;;
				;;  eax = (8bit) proceeding/startpoint
				;;  ebx = endpoint (compared against eax)
				;;  ecx = UMask

			align 16
			ppIce4Npixelz:
				mov     ESPStorage,esp    //
				mov     EBPStorage,ebp    //
				sub     ebx,2             // do last 2 pixels separately
				mov     ebp,edx           //

				mov     esp,ebx           //
										  //
				xor     edx,edx           //
				xor     ebx,ebx           //
				mov     dl,[ebp+ eax]    // Preload  dl

			 align 16 
			 ppCopyLoop:

				add     dl,al             ;;  Linestart[u]+u
				mov     bl,[ebp+eax+1]    ;;  Glass
                                      
				and     dl,cl			  ;;
				add     bl,al			  ;;
                                      
				inc     bl                ;;  Linestart[u+1] + u+1
				add     eax,2			  ;;
										  ;;
				mov     dl,[edi+edx]      ;;  Texel
				and     bl,cl			  ;;
										  ;;
				cmp     esp,eax           ;;  
				mov     [esi+eax-2],dl    ;;  Store
                                      
				mov     bl,[edi+ebx]      ;;  Texel  // non-Positiond one WILL have Pentium-clash with Glass...
				mov     dl,[ebp+eax]      ;;  Glass  // (_if still horizontally aligned...) but not usually..
										  ;;
				mov     [esi+eax-2+1],bl  ;;  Store
				jne     ppCopyLoop          ;;
										  ;;
				;;;;;;;;;;;;;;;;;;;;;;;;;;;; 8 cyc/2pix...

				mov     esp,ESPStorage    ;;
				mov     bl,[ebp+eax+1]    ;; get glasspixel 2

				add     dl,al             ;;
				add     bl,al             ;;

				inc     bl	              ;;
				and     dl,cl             ;;

				mov     ebp,EBPStorage    ;;
				and     bl,cl             ;;

				mov     dl,[edi+edx]      ;; Texel
				mov     bl,[edi+ebx]      ;; Texel

				mov     [esi+eax],dl      ;; Dest
				mov     [esi+eax+1],bl    ;; Dest

				add     eax,2             ;;
				retn                      ;;

				;;
				;;  esi = LineStart
				;;  edi = TexStart
				;;  edx = GlassStart
				;;
				;;  eax = (8bit) proceeding/startpoint
				;;  ecx = UMask
				;;

			align 16
			ppIceMisc8Pixelz:
				mov     ebx,[esi+eax] // First destin word - cache warming. 
				mov     ebx,UDisp
				push    ebp
				mov     ESPStorage, esp

				mov		ebp, eax   //
				add     ebp, 8     // current cntr + nr of pixels to do

				mov     esp, edx   // GlassStart
				
				add     ebx, eax
				xor     edx, edx
				and     ebx, ecx

			align 16
			ppCopyLp:
				mov     dl, [esp+ebx]   ;; GlassStart [edx]

				add     dl, al		    ;;
				inc     ebx             ;;

				and     dl, cl          ;;

				and     ebx, ecx        ;;
				mov     dl, [edi+edx]   ;; Texel

				mov     [esi+eax], dl   ;; Destin
				inc     eax      	    ;;

				cmp     ebp, eax	    ;;
				jne     ppCopyLp          ;;

				mov		esp, ESPStorage ;;
				nop
				pop		ebp             ;;
				retn

			 ppEndIceRender:
			}
		}
		else
		{
			// Regular Pentium version - using cache warming.
			__asm                        //
			{                            //
				mov     esi, LineStart   //
				mov     edi, TexStart    //

				mov     ecx, [TempUMask] //
				mov     ebx, Xdimension  //
				xor     eax, eax         //

				mov     ebx, Xdimension  //
				sub     ebx, UDisp       //

				test    ebx, ecx 	     //  zero? - no split 
				jz      NoSplit          //

				cmp     ebx, 7           //  
				ja      SkipFirstBit     //  Skip if not in first 8 pixels.

				;; split is in first 8 pixels and Unaligned.

				mov     edx, GlassStart  //
				call    IceMisc8Pixelz   //
				jmp     DoEndChunk       //

			NoSplit:                     //
				mov     ebx, Xdimension  //
				mov     edx, GlassStart  //
				jmp     DoLastChunk      //

				///////////////////////////

			SkipFirstBit:

				and		ebx, 0x0FF-7;     // FF - 3     = stretch before wrapping needed
				mov		edx, GlassStart   //
				add		edx, UDisp        //
				call	Ice4Npixelz       // 

				mov     ebx, UDisp        //
				and     ebx, 7            //
				jz      DoEndChunk        // No unaligned Positionment

				mov		edx, GlassStart   //
				call	IceMisc8Pixelz    //

			DoEndChunk:                   // 
				cmp		eax,Xdimension    // Any pixels left to do ? 
				je		EndIceRender      //

				mov     ebx, Xdimension   // Da end marker
				mov		edx, GlassStart   //
				add     edx, UDisp        // Need to since we're past the skip
				sub     edx, ebx          //
			DoLastChunk:
				Call    Ice4Npixelz       //
				jmp     EndIceRender      //

				;;  esi = LineStart
				;;  edi = TexStart
				;;  edx = GlassStart
				;;
				;;  eax = (8bit) proceeding/startpoint
				;;  ebx = endpoint (compared against eax)
				;;  ecx = UMask

			align 16
			Ice4Npixelz:
				mov     ESPStorage,esp    //
				mov     EBPStorage,ebp    //
				sub     ebx,2             // do last 2 pixels separately
				mov     ebp,edx           //

				mov     esp,ebx           //
										  //
				xor     edx,edx           //
				xor     ebx,ebx           //
				mov     dl,[ebp+ eax]    // Preload  dl

			 align 16 
			 CopyLoop:
				// cache warming by loading destination to dummy (bl)
				nop                       // xor ebx,ebx slightly faster on PII
				mov     bl,[esi+eax+2]    // This'll prime the cache EVERY 2 bytes - wasteful..
			
				add     dl,al             ;;  Linestart[u]+u
				mov     bl,[ebp+eax+1]    ;;  Glass
                                      
				and     dl,cl			  ;;
				add     bl,al			  ;;
                                      
				inc     bl                ;;  Linestart[u+1] + u+1
				add     eax,2			  ;;
										  ;;
				mov     dl,[edi+edx]      ;;  Texel
				and     bl,cl			  ;;
										  ;;
				cmp     esp,eax           ;;  
				mov     [esi+eax-2],dl    ;;  Store
                                      
				mov     bl,[edi+ebx]      ;;  Texel  // non-Positiond one WILL have Pentium-clash with Glass...
				mov     dl,[ebp+eax]      ;;  Glass  // (_if still horizontally aligned...) but not usually..
										  ;;
				mov     [esi+eax-2+1],bl  ;;  Store
				jne     CopyLoop          ;;
										  ;;
				;;;;;;;;;;;;;;;;;;;;;;;;;;;; 8 cyc/2pix...

				mov     esp,ESPStorage    ;;
				mov     bl,[ebp+eax+1]    ;; get glasspixel 2

				add     dl,al             ;;
				add     bl,al             ;;

				inc     bl	              ;;
				and     dl,cl             ;;

				mov     ebp,EBPStorage    ;;
				and     bl,cl             ;;

				mov     dl,[edi+edx]      ;; Texel
				mov     bl,[edi+ebx]      ;; Texel

				mov     [esi+eax],dl      ;; Dest
				mov     [esi+eax+1],bl    ;; Dest

				add     eax,2             ;;
				retn                      ;;

				;;
				;;  esi = LineStart
				;;  edi = TexStart
				;;  edx = GlassStart
				;;
				;;  eax = (8bit) proceeding/startpoint
				;;  ecx = UMask
				;;

			align 16
			IceMisc8Pixelz:
				mov     ebx,[esi+eax] // First destin word - cache warming. 
				mov     ebx,UDisp
				push    ebp
				mov     ESPStorage, esp

				mov		ebp, eax   //
				add     ebp, 8     // current cntr + nr of pixels to do

				mov     esp, edx   // GlassStart
				
				add     ebx, eax
				xor     edx, edx
				and     ebx, ecx

			align 16
			CopyLp:
				mov     dl, [esp+ebx]   ;; GlassStart [edx]

				add     dl, al		    ;;
				inc     ebx             ;;

				and     dl, cl          ;;

				and     ebx, ecx        ;;
				mov     dl, [edi+edx]   ;; Texel

				mov     [esi+eax], dl   ;; Destin
				inc     eax      	    ;;

				cmp     ebp, eax	    ;;
				jne     CopyLp          ;;

				mov		esp, ESPStorage ;;
				nop
				pop		ebp             ;;
				retn

			 EndIceRender:
			}
		}

#else
		for( INT  u=0; u < Xdimension; u+=2 )
        {
			// Coolish effect combining half warped, half original.
			// LineStart[u] = (  SourceStart[(u+LineStart[u]) & UMask ] + SourceStart[(u+128) & UMask]  ) >> 1;

			int u2 = u+1;
			// Help C++ with pipelining...

			LineStart[u]  =  TexStart[( u +  GlassStart[(u +UDisp) & UMask]) & UMask ] ;
			LineStart[u2] =  TexStart[( u2 + GlassStart[(u2+UDisp) & UMask]) & UMask ] ;
        }
#endif

	}

	unguard;
}


//
// Move the source texture underneath a layer of 'ice'.
//

void UIceTexture::BlitIceTex()
{	

	guard(UIceTexture::BlitIceTex);
    // Warning: Source/GlassTexture must have same dimensions..

    BYTE* TexAddr		= &SourceTexture->GetMip(0)->DataArray(0);
	BYTE* GlassAddr		= &GlassTexture->GetMip(0)->DataArray(0);
    BYTE* BitMapAddr	= &GetMip(0)->DataArray(0);  // Pointer

    INT  Xdimension   = USize;  // Wrap needed for 8-bit counters.
    INT  Ydimension   = VSize;  //

	INT  TempUMask    = UMask;

	static	DWORD ESPStorage;   // Temp ESP storage
	static	DWORD EBPStorage;   // Temp EBP storage

	// Positionments are (signed) fixed-point 3:4
	int  UDisp = appRound(UPosition) & UMask;
	int  VDisp = appRound(VPosition) & VMask;

    for ( INT  v=0; v < Ydimension; v++ )
    {
		BYTE* LineStart   = BitMapAddr +    ( v << UBits );
		BYTE* TexStart    = TexAddr    + (((v + VDisp) & VMask) << UBits);
		BYTE* GlassStart  = GlassAddr  +    ( v << UBits );

#if ICEASM

		// PPro-optimized version (no cache warming)
		if( GIsPentiumPro )
		{  		
			__asm                         
			{                             
				mov     esi, LineStart    //
				mov     edi, TexStart     //

				mov     ecx, [TempUMask]  //

				mov     ebx, Xdimension   //
				mov     edx, GlassStart   //

				mov     eax, UDisp        //

				add		ebx,eax           //  compensate for source-texture X-bias
				sub     edx,eax           //
				sub     esi,eax           //

			//Ice4Npixelz:

				mov     ESPStorage,esp    //
				mov     EBPStorage,ebp    //

				sub     ebx,2             //  Do last 2 pixels separately
				mov     ebp,edx           //
				mov     esp,ebx           //

				xor     edx,edx           //
				xor     ebx,ebx           //

				mov     dl,[ebp+eax]      //  Preload  dl

			 align 16
			 CopyLoop: 
				add     dl,al             //  Linestart[u]+u
				mov     bl,[ebp+eax+1]    //  Glass
										  //
				and     dl,cl			  //  wrap using mask
				add     bl,al			  //
										  //
				inc     bl                //  Linestart[u+1] + u+1
				add     eax,2			  //  // AL not changed itself so no stall
										  //
				mov     dl,[edi+edx]      //  Texel
				and     bl,cl			  //  wrap using mask
										  //
				cmp     esp,eax           //
				mov     [esi+eax-2],dl    //  Store
										  //
				mov     bl,[edi+ebx]      //  Texel
				mov     dl,[ebp+eax]      //  Glass
										  //
				mov     [esi+eax-2+1],bl  //  Store
				jne     CopyLoop          //
										  //
				////////////////////////////

				mov     esp,ESPStorage    //
				mov     bl,[ebp+eax+1]    // get glasspixel 2

				add     dl,al             //
				add     bl,al             //

				inc     bl	              // ebx	    
				and     dl,cl             // edx,ecx 

				mov     ebp,EBPStorage
				and     bl,cl             // ebx,ecx 

				mov     dl,[edi+edx]      // Texel
				mov     bl,[edi+ebx]      // Texel

				mov     [esi+eax],dl      // Dest
				mov     [esi+eax+1],bl    // Dest

			}
		}		
		else

		// Pentium / Pentium MMX optimized version (WITH cache warming)
		{
			__asm                         
			{                             
				mov     esi, LineStart    //
				mov     edi, TexStart     //

				mov     ecx, [TempUMask]  //

				mov     ebx, Xdimension   //
				mov     edx, GlassStart   //

				mov     eax, UDisp        //

				add		ebx,eax           //  compensate for source-texture X-bias
				sub     edx,eax           //
				sub     esi,eax           //

			//Ice4Npixelz:

				mov     ESPStorage,esp    //
				mov     EBPStorage,ebp    //

				sub     ebx,2             //  Do last 2 pixels separately
				mov     ebp,edx           //
				mov     esp,ebx           //

				xor     edx,edx           //
				xor     ebx,ebx           //

				mov     dl,[ebp+eax]      //  Preload  dl

			 align 16
    		 CopyLoop2: 

				nop				 		  //  
				mov     bl,[esi+eax-2]    //  cache-warming load - per word..

				add     dl,al             //  Linestart[u]+u
				mov     bl,[ebp+eax+1]    //  Glass
										  //
				and     dl,cl			  //
				add     bl,al			  //
										  //
				inc     bl                //  Linestart[u+1] + u+1
				add     eax,2			  //  // AL not changed itself so no stall
										  //
				mov     dl,[edi+edx]      //  Texel
				and     bl,cl			  //
										  //
				cmp     esp,eax           //
				mov     [esi+eax-2],dl    //  Store
										  //
				mov     bl,[edi+ebx]      //  Texel
				mov     dl,[ebp+eax]      //  Glass
										  //
				mov     [esi+eax-2+1],bl  //  Store
				jne     CopyLoop2         //
										  //
				////////////////////////////

				mov     esp,ESPStorage    //
				mov     bl,[ebp+eax+1]    // get glasspixel 2

				add     dl,al             //
				add     bl,al             //

				inc     bl				  //
				and     dl,cl			  //

				mov     ebp,EBPStorage    //
				and     bl,cl		      //

				mov     dl,[edi+edx]      // Texel
				mov     bl,[edi+ebx]      // Texel

				mov     [esi+eax],dl      // Dest
				mov     [esi+eax+1],bl    // Dest
			}
		}
#else
		for( INT  u=0; u < Xdimension; u+=2 )
        {
			// Coolish effect combining half warped, half original.
			// LineStart[u] = (  SourceStart[(u+LineStart[u]) & UMask ] + SourceStart[(u+128) & UMask]  ) >> 1;

			int u2 = u+1; // Help C++ with pipelining...

			LineStart[u]  =  TexStart[(  u + UDisp + GlassStart[ u]) & UMask ] ;
			LineStart[u2] =  TexStart[( u2 + UDisp + GlassStart[u2]) & UMask ] ;
        }
#endif

	}

	unguard;
}



enum {  // Internal drop types - special alternatives etc.
	DROP_WhirlyBack = 64 ,
	DROP_BigWhirlyBack   ,
	DROP_Transient1 = 128,
	DROP_Transient2		 ,
	DROP_Transient3      ,
};


void UWaterTexture::WaterRedrawDrops()
{
	guard(UWaterTexture::WaterRedrawDrops);
    //
    // Waterdrops setting from the editable list.
    //
	//
    // Warning: All code manipulating the wave source-field
    //          must be aware it only has 1/2 the dimension
    //          of the actual output bitmap.
	//

    BYTE  U2Mask = UMask >> 1;       // warning: 1/2 of full output size
    BYTE  V2Mask = VMask >> 1;       // 
    INT   XSize  = USize/2;          // 
    BYTE* WaveFieldA = SourceFields; //
    BYTE* WaveFieldB = SourceFields + XSize;
    INT   SegSize;
	BYTE  Depth;

    GlobalPhase++;

	int  t;

    for (int S = 0; S < NumDrops; S++)
    {
        BYTE Xnow = Drops[S].X;
        BYTE Ynow = Drops[S].Y;
        DWORD DropDest   = (DWORD)( ( Ynow << UBits ) + Xnow);

        switch( Drops[S].Type )
        {

        case DROP_FixedDepth:   // FIXEDD-depth pixy
            Depth = Drops[S].ByteD;
            WaveFieldA[ DropDest ]  =  Depth;
            WaveFieldB[ DropDest ]  =  Depth;
            break; 

		case DROP_PhaseSpot:   // Phased I
			{
       	    Drops[S].Depth += Drops[S].ByteD; // update phase
            Depth = PhaseTable[ Drops[S].Depth ];
            WaveFieldA[ DropDest ]  =  Depth;
            WaveFieldB[ DropDest ]  =  Depth;

			/*
			DWORD Phase16 = ((DWORD)Drops[S].ByteB << 8) + (DWORD)Drops[S].ByteA;
				  Phase16 = 0xFFFF & ( Phase16 + ((DWORD)Drops[S].ByteD << 8) + (DWORD)Drops[S].ByteC );

			Drops[S].ByteA =    Phase16 & 0xFF;
			Drops[S].ByteB =  ( Phase16 >> 8 ) & 0xFF;

			// Real 16-bit phase is B:A, delta= D:C
			Depth = PhaseTable[ Drops[S].ByteB ];
			WaveFieldA[ DropDest ]  =  Depth;
			WaveFieldB[ DropDest ]  =  Depth;
			*/

			}
            break;

		case DROP_HalfAmpl:     // half amplitude: only goes down INT  o water
			Drops[S].Depth += Drops[S].ByteD; // update phase
            Depth = PhaseTable[ Drops[S].Depth ];
			if (Depth<128) Depth = 128; // clamp it
            WaveFieldA[ DropDest ]  =  Depth;
            WaveFieldB[ DropDest ]  =  Depth;
            break;

        case DROP_ShallowSpot: // Phased II
       	    Drops[S].Depth += Drops[S].ByteD; // update phase
            Depth = 64+ (PhaseTable[ Drops[S].Depth ] >> 1) ;
            WaveFieldA[ DropDest ]  =  Depth;
            WaveFieldB[ DropDest ]  =  Depth;
            break;

        case DROP_RandomMover: // 'movers'
            Drops[S].X = U2Mask & (Xnow - (SpeedRand()&3) + (SpeedRand()&3) );
            Drops[S].Y = V2Mask & (Ynow - (SpeedRand()&3) + (SpeedRand()&3) );
            WaveFieldA[ DropDest ]  =  128+57;
            WaveFieldB[ DropDest ]  =  128-57;
            break;

        case DROP_FixedRandomSpot: // 'totally random'  fixed wavesource
            WaveFieldA[ DropDest ]  =  SpeedRand();
            WaveFieldB[ DropDest ]  =  SpeedRand();
            break;

		
		case DROP_WhirlyThing: // Circular stirring.
			{
				DWORD Phase16 = ((DWORD)Drops[S].ByteB << 8) + (DWORD)Drops[S].ByteA;
					  Phase16 = 0xFFFF & ( Phase16 + ((DWORD)Drops[S].ByteD << 8) + (DWORD)Drops[S].ByteC );

				Drops[S].ByteA =    Phase16 & 0xFF;
				Drops[S].ByteB =  ( Phase16 >> 8 ) & 0xFF;

				// So real 16-bit phase is B:A, delta= D:C

				DWORD AddDestX =    U2Mask & (Xnow + (PhaseTable[ Drops[S].ByteB ] >>4));
				DWORD AddDestY = (  V2Mask & (Ynow + (PhaseTable[(Drops[S].ByteB+64) & 255 ]>>4)) ) << UBits;
				WaveFieldA[ AddDestX + AddDestY ] = Drops[S].Depth;
				WaveFieldB[ AddDestX + AddDestY ] = Drops[S].Depth;
            }
			break;

		case DROP_WhirlyBack: // Circular stirring.
			{
				DWORD Phase16 = ((DWORD)Drops[S].ByteB << 8) + (DWORD)Drops[S].ByteA;
					  Phase16 = 0xFFFF & ( Phase16 - ((DWORD)Drops[S].ByteD << 8) - (DWORD)Drops[S].ByteC );

				Drops[S].ByteA =    Phase16 & 0xFF;
				Drops[S].ByteB =  ( Phase16 >> 8 ) & 0xFF;

				// So real 16-bit phase is B:A, delta= D:C

				DWORD AddDestX =    U2Mask & (Xnow + (PhaseTable[ Drops[S].ByteB ] >>4));
				DWORD AddDestY = (  V2Mask & (Ynow + (PhaseTable[(Drops[S].ByteB+64) & 255 ]>>4)) ) << UBits;
				WaveFieldA[ AddDestX + AddDestY ] = Drops[S].Depth;
				WaveFieldB[ AddDestX + AddDestY ] = Drops[S].Depth;
            }
			break;

		case DROP_BigWhirly: // Simple single-direction mover.  
			{
				DWORD Phase16 = ( *(DWORD*)&(Drops[S].ByteA) );
				      Phase16 = 0xFFFF & ( Phase16 + *(_WORD*)&(Drops[S].ByteC) );

				Drops[S].ByteB =  ( Phase16 >> 8 ) & 0xFF;
				Drops[S].ByteA =    Phase16 & 0xFF;

				// So real 16-bit phase is B:A, delta= D.

				DWORD AddDestX =    U2Mask & (Xnow + (PhaseTable[ Drops[S].ByteB] >>3) ) ;
				DWORD AddDestY = (  V2Mask & (Ynow + (PhaseTable[(Drops[S].ByteB+64) & 255] >>3) ) ) << UBits ;
				WaveFieldA[ AddDestX + AddDestY ]  =  Drops[S].Depth;
				WaveFieldB[ AddDestX + AddDestY ]  =  Drops[S].Depth;
            }
			break;

		case DROP_BigWhirlyBack: // Simple single-direction mover.  
			{
				DWORD Phase16 = ((DWORD)Drops[S].ByteB << 8) + (DWORD)Drops[S].ByteA;
				      Phase16 = 0xFFFF & ( Phase16 - ((DWORD)Drops[S].ByteD << 8) - (DWORD)Drops[S].ByteC );

				Drops[S].ByteB =  ( Phase16 >> 8 ) & 0xFF;
				Drops[S].ByteA =    Phase16 & 0xFF;

				// So real 16-bit phase is B:A, delta= D.

				DWORD AddDestX =    U2Mask & (Xnow + (PhaseTable[ Drops[S].ByteB] >>3) ) ;
				DWORD AddDestY = (  V2Mask & (Ynow + (PhaseTable[(Drops[S].ByteB+64) & 255] >>3) ) ) << UBits ;
				WaveFieldA[ AddDestX + AddDestY ]  =  Drops[S].Depth;
				WaveFieldB[ AddDestX + AddDestY ]  =  Drops[S].Depth;
            }
			break;

        case DROP_HorizontalLine: //  Horizontal linesegment.
			{
				Depth = Drops[S].Depth;
				SegSize = Drops[S].ByteD >> 1;
				DWORD DropDestX0 = (DWORD)  ( Ynow << UBits ); 
				for ( t=0; t <= SegSize; t++)
				{
					DWORD AddDestX = (DWORD) ((t+Xnow) & U2Mask);
					WaveFieldA[ DropDestX0 + AddDestX ] = Depth;
					WaveFieldB[ DropDestX0 + AddDestX ] = Depth;
				}
			}
            break;

        case DROP_VerticalLine: //  Vertical linesegment.
            Depth = Drops[S].Depth;
            SegSize = Drops[S].ByteD >> 1;
            for ( t=0; t<=SegSize; t++)
            {
                DWORD AddDestY =  (DWORD) ( ((Ynow+t) & V2Mask) << UBits );
                WaveFieldA[Xnow + AddDestY] = Depth;
                WaveFieldB[Xnow + AddDestY] = Depth;
				
            }
            break;

        case DROP_DiagonalLine1: //  diagonal (slash)
            Depth = Drops[S].Depth;
            SegSize = Drops[S].ByteD >> 1;
            for ( t=0; t<=SegSize; t++)
            {
                DWORD AddDestY = (DWORD) ( ((Ynow+t) & V2Mask) << UBits );
                DWORD AddDestX = (DWORD) (  (Xnow-t) & U2Mask);
                WaveFieldA[ AddDestY + AddDestX] = Depth;
                WaveFieldB[ AddDestY + AddDestX] = Depth;
            }
            break;


        case DROP_DiagonalLine2: //  diagonal (backslash)
            Depth = Drops[S].Depth;
            SegSize = Drops[S].ByteD >> 1;
            for ( t=0; t<=SegSize; t++)
            {
                DWORD AddDestY = (DWORD) ( ((Ynow+t) & V2Mask) << UBits );
                DWORD AddDestX = (DWORD) (  (Xnow+t) & U2Mask);
                WaveFieldA[ AddDestY + AddDestX] = Depth;
                WaveFieldB[ AddDestY + AddDestX] = Depth;
            }
            break;


        case DROP_HorizontalOsc: //  Horizontal linesegment.
			{
       			Drops[S].Depth += Drops[S].ByteC; // update phase
				Depth = PhaseTable[ Drops[S].Depth ];
				SegSize = Drops[S].ByteD >> 1;
				DWORD DropDestX0 = (DWORD)  ( Ynow << UBits ); 
				for ( t=0; t<=SegSize; t++)
				{
					DWORD AddDestX = (DWORD) ((t+Xnow) & U2Mask);
					WaveFieldA[ DropDestX0 + AddDestX ] = Depth;
					WaveFieldB[ DropDestX0 + AddDestX ] = Depth;
				}
			}
            break;


        case DROP_VerticalOsc: //  Vertical linesegment.
       	    Drops[S].Depth += Drops[S].ByteC; // update phase
            Depth = PhaseTable[ Drops[S].Depth ];
            SegSize = Drops[S].ByteD >> 1;
            for ( t=0; t<=SegSize; t++)
            {
                DWORD AddDestY = (DWORD) ( ((Ynow+t)& V2Mask) << UBits );
                WaveFieldA[Xnow + AddDestY] = Depth;
                WaveFieldB[Xnow + AddDestY] = Depth;
            }
            break;


        case DROP_DiagonalOsc1: //  diagonal (slash)
       	    Drops[S].Depth += Drops[S].ByteC; // update phase
            Depth = PhaseTable[ Drops[S].Depth ];
            SegSize = Drops[S].ByteD >> 1;
            for ( t=0; t<=SegSize; t++)
            {
                DWORD AddDestY = (DWORD) ( ((Ynow+t) & V2Mask) << UBits );
                DWORD AddDestX = (DWORD) (  (Xnow-t) & U2Mask);
                WaveFieldA[ AddDestY + AddDestX] = Depth;
                WaveFieldB[ AddDestY + AddDestX] = Depth;
            }
            break;


        case DROP_DiagonalOsc2: //  diagonal (backslash)
       	    Drops[S].Depth += Drops[S].ByteC; // update phase
            Depth = PhaseTable[ Drops[S].Depth ];
            SegSize = Drops[S].ByteD >> 1;
            for ( t=0; t<=SegSize; t++)
            {
                DWORD AddDestY = (DWORD) ( ((Ynow+t) & V2Mask) << UBits );
                DWORD AddDestX = (DWORD) (  (Xnow+t) & U2Mask);
                WaveFieldA[ AddDestY + AddDestX] = Depth;
                WaveFieldB[ AddDestY + AddDestX] = Depth;
            }
            break;

        
		case DROP_AreaClamp: // Fixed-depth/adaptive-depth rect areas.
			{
				Depth   = Drops[S].Depth;
				SegSize = Drops[S].ByteD >> 1;

				DWORD DestY = (DWORD) ( ((Ynow) & V2Mask) << UBits );

				for (int v = 0; v < SegSize; v++ )
				{
					DestY = (DWORD) ( (( Ynow+v ) & V2Mask) << UBits );
					DWORD Dest  = DestY + ( Xnow & U2Mask );

  					for (int u = 1; u <= SegSize; u++ )
					{
						WaveFieldA[ Dest] = Depth;
						WaveFieldB[ Dest] = Depth;
						Dest = DestY + ((DWORD) (u+Xnow) & U2Mask);
					}
				}
			}
			break; 
       

		case DROP_RainDrops: //  Rain drizzle.
			{
				// Keep the action sparse - just draw more sparks if needed...
				if ((SpeedRand()&15)==0)
				{
					Depth = Drops[S].Depth;
					BYTE  Spray = Drops[S].ByteD;
					DWORD  AddDestY = (DWORD) (((Ynow+(SpeedRand()*Spray >> 8) ) & V2Mask) << UBits );
					DWORD  AddDestX = (DWORD)  ((Xnow+(SpeedRand()*Spray >> 8) ) & U2Mask);
					WaveFieldA[ AddDestY + AddDestX] = Depth;
					WaveFieldB[ AddDestY + AddDestX] = 255^Depth;
				}
			}
            break;


		case DROP_LeakyTap: // Selfexplanatory.
			{
				// Keep the action sparse - just draw more sparks if needed...
				if ((Drops[S].ByteA +=Drops[S].ByteD) <= Drops[S].ByteD) 
				{
					WaveFieldA[ DropDest ]  =  Drops[S].Depth;
					WaveFieldB[ DropDest ]  =  255^Drops[S].Depth;
				}
			}
            break;

			
		case DROP_DrippyTap: // Irregular Leaky Tap.
			{
				// Keep the action sparse - just draw more sparks if needed...
				if ((Drops[S].ByteA +=Drops[S].ByteD) <= Drops[S].ByteD) 
				{ 
					Drops[S].ByteA = SpeedRand();
					WaveFieldA[ DropDest ]  =  Drops[S].Depth;
					WaveFieldB[ DropDest ]  =  255^Drops[S].Depth;
				}
			}
            break;
      
        } //switch

    }

	unguard;

}



//
// Adding a drop to the Pool structure (edit-time).
//

void UWaterTexture::AddDrop( INT  DropX, INT  DropY )
{
	guard(UWaterTexture::AddDrop);

    // Return if out of bounds or out of Drops.
    if( DropX>=USize || DropY>=VSize || DropX<0 || DropY<0 || NumDrops>=MaxDrops )
        return;

    INT  S = NumDrops++;

    Drops[S].X = DropX >> 1;
    Drops[S].Y = DropY >> 1;
    Drops[S].Type = DropType;

    DrawPhase++;

    // The rest are Drop-type specific assignments.

	switch( DropType )
    {
		// Force a certain area to FX_Depth.

		case DROP_PhaseSpot:  // Align new phase in-step.
		case DROP_HalfAmpl:   // 
			{
			Drops[S].Depth = ( GlobalPhase * ( FX_Frequency  ) + FX_Phase) & 255;
			Drops[S].ByteD = FX_Frequency;
			Drops[S].ByteA = 0;
			Drops[S].ByteB = 0;
			Drops[S].ByteC = 0;
			}
			break;

		case DROP_ShallowSpot: // Align new phase in-step, whatever the period.
			Drops[S].Depth = ( GlobalPhase * FX_Frequency + FX_Phase) & 255;
			Drops[S].ByteD = FX_Frequency;
			break;

		case DROP_FixedRandomSpot: // random fixed spot
			break;

		// All sparks that need ParamsB as their 'depth':
		case DROP_HorizontalLine:
		case DROP_VerticalLine:
		case DROP_DiagonalLine1:
		case DROP_DiagonalLine2:
		case DROP_RainDrops:
			Drops[S].Depth = FX_Depth;
			Drops[S].ByteD = FX_Size;
			break;

		// All linesegments that need a fixed relative phase:
		case DROP_HorizontalOsc:
		case DROP_VerticalOsc:
		case DROP_DiagonalOsc1:
		case DROP_DiagonalOsc2:
			Drops[S].Depth = ( GlobalPhase * FX_Frequency + FX_Phase) & 255;
			Drops[S].ByteC = FX_Frequency;
			Drops[S].ByteD = FX_Size;
			break;

		case DROP_AreaClamp:
			Drops[S].Depth = FX_Depth;
			Drops[S].ByteD = FX_Size;
			break;

		case DROP_WhirlyThing:
			{
			Drops[S].Depth = FX_Depth;
			Drops[S].ByteA = 0; // Low-byte of phase.

			int TempFreq = FX_Frequency;
			if (TempFreq > 127) // negative variant
				{
				Drops[S].Type = DROP_WhirlyBack;
				TempFreq = 255-TempFreq;
				}

			DWORD TimeDelta = 65535/( (int)(127-TempFreq) +1); //65536
			Drops[S].ByteC =  ( TimeDelta & 0x00FF );
			Drops[S].ByteD =  ( TimeDelta & 0xFF00 ) >> 8;
			Drops[S].ByteB = (( (GlobalPhase * ( TimeDelta )) + (FX_Phase << 8) ) >> 8 ) & 255;
			}
			break;

		case DROP_BigWhirly:
			{
			Drops[S].Depth = FX_Depth;
			Drops[S].ByteA = 0; // Low-byte of phase.

			int TempFreq = FX_Frequency;
			if (TempFreq > 127) // negative variant
				{
				Drops[S].Type = DROP_BigWhirlyBack;
				TempFreq = 255-TempFreq;
				}

			DWORD TimeDelta = 65535/( (int)(127-TempFreq) +1); //65537
			Drops[S].ByteC =  ( TimeDelta & 0x00FF );
			Drops[S].ByteD =  ( TimeDelta & 0xFF00 ) >> 8;
			Drops[S].ByteB = (( (GlobalPhase * ( TimeDelta )) + (FX_Phase << 8) ) >> 8 ) & 255;
			}
			break;

		case DROP_LeakyTap:
			Drops[S].Depth = FX_Depth;
			Drops[S].ByteA = ( GlobalPhase * FX_Frequency + FX_Phase) & 255;
			Drops[S].ByteD = ( FX_Frequency ) >>2;
			break;		

		case DROP_DrippyTap:
			Drops[S].Depth = FX_Depth;
			Drops[S].ByteA = SpeedRand();
			Drops[S].ByteD = (FX_Frequency ) >>2;
			break;		
	}

	unguard;
}


// Delete a drop within a certain area in the image. 
void UWaterTexture::DeleteDrops( INT  DropX, INT  DropY, INT  AreaWidth )
{
	guard(UWaterTexture::DeleteDrops);

    for( INT  S=0; S<NumDrops; S++ )
    {
        if( AreaWidth >= ( Abs(DropX - (Drops[S].X << 1) ) +
                           Abs(DropY - (Drops[S].Y << 1) ) ) )
        {
            // Delete Drop by replacing it with last one (+ delete last one).
            INT  LastDrop = --NumDrops;
            Drops[S] = Drops[LastDrop];
        }
    }

	unguard;
}


/*----------------------------------------------------------------------------
	Temp spark drawing (for editor)
----------------------------------------------------------------------------*/

void UFireTexture::TempDrawSpark (INT PosX, INT  PosY, INT Intensity )
{
    DWORD SparkDest;

    SparkDest = (DWORD)(   (UMask & PosX) +
                          ((VMask & PosY)<< UBits )   );

    GetMip(0)->DataArray(SparkDest) = (BYTE) Intensity;
}


/*----------------------------------------------------------------------------
	Very non-sucking default palette.
----------------------------------------------------------------------------*/

void BlueLagunaPalette(UPalette* TargetPal)
{
	for (int i = 0; i< 64; i++)
	{
		TargetPal->Colors(i).R     = Min(    0+ (i* 59)/64 ,255);
		TargetPal->Colors(i).G     = Min(    0+ (i* 67)/64 ,255);
		TargetPal->Colors(i).B     = Min(    0+ (i*100)/64 ,255);

		TargetPal->Colors(i+64).R  = Min(   59+ (i* 55)/64 ,255);
		TargetPal->Colors(i+64).G  = Min(   67+ (i* 60)/64 ,255);
		TargetPal->Colors(i+64).B  = Min(  100+ (i* 97)/64 ,255);

		TargetPal->Colors(i+128).R = Min(  114+ (i* 64)/64 ,255);
		TargetPal->Colors(i+128).G = Min(  127+ (i* 60)/64 ,255);
		TargetPal->Colors(i+128).B = Min(  197+ (i* 33)/64 ,255);

		TargetPal->Colors(i+192).R = Min(  178+ (i* 78)/64 ,255);
		TargetPal->Colors(i+192).G = Min(  187+ (i* 69)/64 ,255);
		TargetPal->Colors(i+192).B = Min(  230+ (i* 26)/64 ,255);
	}
};


/*----------------------------------------------------------------------------
	UFractalTexture object implementation.
----------------------------------------------------------------------------*/

UFractalTexture::UFractalTexture()
{
	guard(UFractalTexture::UFractalTexture);

	// Mark the texture as parametric (so bitmap is not saved).
	// Mark the texture as realtime   (so it is regenerated each tick).
	TextureFlags |= TF_Parametric | TF_Realtime;

	InitTables(); // Global LUTS, checks itself to see if already initzed.

	DrawPhase   = 0;
	AuxPhase    = 0;
    GlobalPhase = 0;

	unguard;
}


void UFractalTexture::Init( INT  InUSize, INT  InVSize )
{
	guard(UFractalTexture::Init);

	VERIFY_CLASS_OFFSET(U,FractalTexture,UMask); // verifies correct mirroring..

	// Proper-powers-of-two verification.
	check((InUSize&(InUSize-1))==0); 
	check((InVSize&(InVSize-1))==0);

	// Init base class.
	UTexture::Init( InUSize, InVSize );

    unguard;
}


void UFractalTexture::PostLoad()
{
	// Call base.
	UTexture::PostLoad();

	UMask = USize - 1;
	VMask = VSize - 1;
	
}

IMPLEMENT_CLASS(UFractalTexture);




/*----------------------------------------------------------------------------
	UFireTexture.
----------------------------------------------------------------------------*/

UFireTexture::UFireTexture() 
{
    // Init all non-serialized variables. 

	OldRenderHeat  =  -1; 
	AuxPhase       =   0;
	StarStatus	   =   1; // Assume some stars until, we find otherwise.

	// initialize the routines we should call for rising/nonrising fire;
	// one for Pentium/MMX es and one for Ppro/PII's

	//void (*MergePass)( INT Y, INT X, INT InnerX );

}

//
// ::Init  called only _once_ ,on creation in the editor)
//

void UFireTexture::Init( INT  InUSize, INT  InVSize )
{
	guard(UFireTexture::Init);

	VERIFY_CLASS_OFFSET(U,FireTexture,SparkType);

	// Init base class.
	UFractalTexture::Init( InUSize, InVSize );

	// Create a custom palette.
	Palette = new( GetParent() )UPalette;
	for( INT  i=0; i<256; i++ )
		Palette->Colors.AddItem( FColor(i,i,0) );
	BlueLagunaPalette(Palette);
	MipZero = Palette->Colors(128);

	// initialize all to-be-serialized variables
    RenderHeat  = 220;
	ActiveSparkNum   =   0;
    SparkType   = SPARK_Blaze;
	SparksLimit = MAXSPARKSINIT; // inital maximum

	//Allocate SparksLimit sparks:
	Sparks.Add(SparksLimit);

	FX_Frequency  =  16;
	FX_Phase	  =  16;
	FX_Heat		  = 255;
	FX_Size		  =  96;
	FX_Area		  =  24;
	FX_HorizSpeed = 130;
	FX_VertSpeed  = 142;

	unguard;
}


void UFireTexture::Clear( DWORD ClearFlags )
{
	guard(UFireTexture::Clear);

	// Init to zero.
	UFractalTexture::Clear( ClearFlags );

	// Clear sparks.
	if( ClearFlags & TCLEAR_Temporal )
	{
		ActiveSparkNum = 0;
		StarStatus = 0;
	}

	unguard;
}

void UFireTexture::PostLoad()
{
	guard(UFireTexture::PostLoad);
	check(sizeof(UFireTexture)==UFireTexture::StaticClass->GetPropertiesSize());
	check(sizeof(UWetTexture)==UWetTexture::StaticClass->GetPropertiesSize());
	check(sizeof(UWaveTexture)==UWaveTexture::StaticClass->GetPropertiesSize());
	check(sizeof(UFractalTexture)==UFractalTexture::StaticClass->GetPropertiesSize());

	// Call base class.
	UFractalTexture::PostLoad();

	// Make sure the texture has its _own_ copy of the palette.
#if COPYPALETTE
	if( Palette!=NULL && GetParent()!=Palette->GetParent() )
	{
		UPalette* NewPalette = new( GetParent(), GetName() )UPalette;
		for( INT i=0; i<256; i++ )
			NewPalette->Colors.AddItem( Palette->Colors(i) );
		Palette = NewPalette->ReplaceWithExisting();
		MipZero = Palette->Colors(128);
		GCache.Flush();
	}
#endif

	// Fill the fire table.
	if (OldRenderHeat != RenderHeat)
	{
		for( INT  T = 0; T<1024; T++ )
			RenderTable[T] = Clamp( T/4.0 + 1.0 - (255-RenderHeat)/16.0, 0.0, 255.0 );
		OldRenderHeat = RenderHeat;
	}

	// Expand/shrink the dynamic array to become
	// exactly SparksLimit elements.
	// Acts both at load time and edit time.
	
	if (Sparks.Num() != SparksLimit)
	{
		// Make sure not TOO big...
		if ( SparksLimit > MAXSPARKSLIMIT )
			SparksLimit = MAXSPARKSLIMIT;
		else
		if ( SparksLimit < MINSPARKSLIMIT )
			SparksLimit = MINSPARKSLIMIT;

		// New SparksLimit; current size is too big. 
		if (Sparks.Num() > SparksLimit)
		{
			// Try culling some transients if needed.
			if  (ActiveSparkNum > SparksLimit)
				for (int t = (ActiveSparkNum-1); t >=0; t--)
				{		
					if (Sparks(t).Type >= ISPARK_TRANSIENTS ) 
					{
						Sparks(t) = Sparks(--ActiveSparkNum);	
						// culled enough ?
						if (ActiveSparkNum<= SparksLimit) break;
					}			 
				}

			// Adjust active sparks the crude way:
			ActiveSparkNum = Min( ActiveSparkNum, SparksLimit );
			// Now delete the empty tail. 
			Sparks.Remove( SparksLimit, Sparks.Num()-SparksLimit); 

		}
		else 
		// Too small: just expand the array.
		{
			Sparks.Add( SparksLimit - Sparks.Num() );
		}
		//Sparks.Debug(); 
	}

	unguard;
}



void UFireTexture::TouchTexture(INT UPos, INT VPos, FLOAT Magnitude)
{
	guard(UFireTexture::TouchTexture);

    DWORD SparkDest = (DWORD)(UPos + (VPos << UBits) );
    GetMip(0)->DataArray(SparkDest) = (BYTE) Magnitude;

	unguard;
}


void UFireTexture::ConstantTimeTick()
{
	guard(UFireTexture::ConstantTimeTick);

	if ((USize>=8) && (VSize>=8)) // safe sizes ?
	{
		// Call FireEngine update.

		RedrawSparks();

#if 0
		if( 0 )
		{
			if( bRising ) CalcWrapFireP2( &GetMip(0)->DataArray(0), RenderTable, USize, VSize );
				else      CalcSlowFireP2( &GetMip(0)->DataArray(0), RenderTable, USize, VSize );
		}
		else
		{
			if( bRising ) CalcWrapFire( &GetMip(0)->DataArray(0), RenderTable, USize, VSize );
				else      CalcSlowFire( &GetMip(0)->DataArray(0), RenderTable, USize, VSize );
		}
#else
		if( bRising ) CalcWrapFire(&GetMip(0)->DataArray(0), RenderTable, USize, VSize );
		else      CalcSlowFire(&GetMip(0)->DataArray(0), RenderTable, USize, VSize );
#endif

		PostDrawSparks();
	}

	unguard;
}



void UFireTexture::MousePosition( DWORD Buttons, FLOAT X, FLOAT Y )
{
	guard(UFireTexture::MousePosition);

	// Call FireEngine update. (adds or removes sparks in spark array)
	FirePaint( appRound(X), appRound(Y), Buttons );

	// Mouse 'torch' animation. Won't work on still cursor.
	if( 1 )
	{
		TempDrawSpark(X  ,Y,255);
		TempDrawSpark(X+1,Y,255);
		TempDrawSpark(X-1,Y,255);
	}
    else
    {
    }

	unguard;
}



void UFireTexture::Click( DWORD Buttons, FLOAT X, FLOAT Y )
{
	guard(UFireTexture::Click);

	// Call FireEngine update.
	FirePaint( appRound(X), appRound(Y), Buttons );

	// Mouse 'torch' animation.
	if( 1 )
	{
		GetMip(0)->DataArray((INT)X + (INT)Y * USize) = 255;
	}
    else
    {
        //SparkType = TouchType;
        //AddSpark( X, Y);
    }
	unguard;
}

void UFireTexture::Serialize( FArchive& Ar )
{
	guard(UFireTexture::Serialize);

    // Clean up & minimize our spark array for saving to disk:
	if( Ar.IsSaving() ) 
	{

		//if (ActiveSparkNum > Sparks.Num()) appErrorf(" Active Spark num out of bounds 1: ActiveSparkNum = %i, Sparks.Num() = %i",ActiveSparkNum,Sparks.Num());

		// Delete the transients, compact the spark array.
		for( INT t=ActiveSparkNum-1; t>=0; t-- )
			if( Sparks(t).Type >= ISPARK_TRANSIENTS ) 
				Sparks(t) = Sparks(--ActiveSparkNum);	

		// Deflate the empty tail.
		if( ActiveSparkNum < Sparks.Num() );
			Sparks.Remove( ActiveSparkNum, Sparks.Num()-ActiveSparkNum ); 
	}

	Super::Serialize( Ar );
	Ar << Sparks;

	// Expand the compacted array again. Sparks.Num() should always be equal to SparksLimit !
	if (Sparks.Num() < SparksLimit)
		Sparks.Add( SparksLimit - Sparks.Num() );

	unguard;
}


IMPLEMENT_CLASS(UFireTexture);


/*----------------------------------------------------------------------------
	UWaterTexture object implementation.
----------------------------------------------------------------------------*/

UWaterTexture::UWaterTexture()
{
	guard(UWaterTexture::UWaterTexture);

	// Fill the algorithm lookup table.
    for(int  t=0; t<(1024+512); t++)
		WaveTable[t] = Clamp( (t/2 - 256) + ( (t-512)< 256), 0, 255 );

	// Init most non-serialized vars.
	OldWaveAmp     = -1; 
	WaveParity     =  0;

	unguard;
}


// Init. Called only _once_, on creation of texture in editor.

void UWaterTexture::Init( INT  InUSize, INT  InVSize )
{
	guard(UWaterTexture::Init);

	VERIFY_CLASS_OFFSET(U,WaterTexture,DropType);

	// Init base class.
	UFractalTexture::Init( InUSize, InVSize );

	// One-time initialization of *serialized* parameters.
    NumDrops =   0;
    DropType =   1;        
	WaveAmp  = 128;

	// FX specifics
	FX_Frequency	= 010;
	FX_Phase		= 010;
	FX_Amplitude	= 255;
	FX_Speed		= 255;
	FX_Radius		= 128;
	FX_Size			= 30;
	FX_Depth		= 255;
	FX_Time         = 30;

	unguard;
}


void UWaterTexture::PostLoad()
{
	guard(UWaterTexture::PostLoad);

	// Call base class.
	UFractalTexture::PostLoad();

	if (SourceFields == NULL)
	{
		// Allocate the two wave height fields.
		SourceFields = new BYTE[ USize * VSize / 2 ]; 
		// initialize water to average height.
		for( INT  i=0; i< USize * VSize / 2; i++ )
			SourceFields[i] = 128;
	};

	unguard;
}



void UWaterTexture::TouchTexture(INT UPos, INT VPos, FLOAT Magnitude)
{
	guard(UWaterTexture::TouchTexture);

	BYTE* WaveFieldA = SourceFields;
    BYTE* WaveFieldB = SourceFields + USize;

    DWORD DropDest   = (DWORD)( ( VPos << UBits ) + UPos);

    // Depress a circular gradual area, radius = Magnitude.

	BYTE Depth = (BYTE) Magnitude;       

    WaveFieldA[ DropDest ]  =  Depth;
    WaveFieldB[ DropDest ]  =  Depth;
   
	unguard;
}




void UWaterTexture::Destroy()
{
	guard(UWaterTexture::Destroy);

	// Free memory.
	if( SourceFields )
		delete SourceFields;

	UTexture::Destroy(); // must call base class' destroy.
	unguard;
}


void UWaterTexture::Clear( DWORD ClearFlags )
{
	guard(UWaterTexture::Clear);

	// Init to zero.
	UFractalTexture::Clear( ClearFlags );

	// Clear fields.
	if( ClearFlags & TCLEAR_Bitmap )
		for( INT  i=0; i< USize * VSize / 2; i++ )
			SourceFields[i] = 128;

	// Clear drops.
	if( ClearFlags & TCLEAR_Temporal )
		NumDrops = 0;

	unguard;
}


void UWaterTexture::WaterPaint( INT X, INT Y, DWORD Buttons)
{
	guard(UWaterTexture::WaterPaint);
		
	INT MouseX=X, MouseY=Y;

	// Water drawing
	
	UBOOL RightButton = (Buttons & MOUSE_Right);
	UBOOL  LeftButton = (Buttons & MOUSE_Left);

	// Perform painting.
    static INT  LastMouseX=0, LastMouseY=0, LastLeftButton=0, LastRightButton=0;

    UBOOL  PosChanged   =  ((LastMouseX != MouseX) || (LastMouseY != MouseY));
    UBOOL  RightChanged =   (LastRightButton != RightButton);
    UBOOL  LeftChanged  =   (LastLeftButton  != LeftButton );

	UBOOL Continuous = true; 

	// Check types that don't require continuous drawing
	switch (DropType)
	{
		case DROP_HorizontalLine:
		case DROP_VerticalLine:
		case DROP_DiagonalLine1:
		case DROP_DiagonalLine2:
		case DROP_HorizontalOsc:
		case DROP_VerticalOsc:
		case DROP_DiagonalOsc1:
		case DROP_DiagonalOsc2:
		case DROP_AreaClamp:
			Continuous = false;
		break;
	};

	// Paint on the water.
    if( (Buttons & MOUSE_Left) && ( Continuous && (PosChanged  || LeftChanged ) ) )
        AddDrop( MouseX, MouseY);

    if( Buttons & MOUSE_Right )
        DeleteDrops( MouseX, MouseY, DEL_RANGE);

	unguard;
}


void UWaterTexture::MousePosition( DWORD Buttons, FLOAT X, FLOAT Y )
{
	guard(UWaterTexture::MousePosition);

	WaterPaint(appRound(X),appRound(Y),Buttons);
	
	unguard;
}



void UWaterTexture::Click( DWORD Buttons, FLOAT X, FLOAT Y )
{
	guard(UWaterTexture::Click);

	INT MouseX=X, MouseY=Y;

	// Paint on the water.
    if( Buttons & MOUSE_Left )
	{
        AddDrop( MouseX, MouseY);
	}

    if( Buttons & MOUSE_Right )
        DeleteDrops( MouseX, MouseY, DEL_RANGE);

	unguard;
}



IMPLEMENT_CLASS(UWaterTexture);


/*----------------------------------------------------------------------------
	UWaveTexture object implementation.
----------------------------------------------------------------------------*/

UWaveTexture::UWaveTexture()
{
	guard(UWaveTexture::UWaveTexture);


	unguard;
}

void UWaveTexture::Init( INT  InUSize, INT  InVSize )
{
	guard(UWaveTexture::Init);

	VERIFY_CLASS_OFFSET(U,WaveTexture,BumpMapAngle);

	// Init base class.
	UWaterTexture::Init( InUSize, InVSize );

	BumpMapAngle = 170;
	BumpMapLight = 50;

	PhongRange = 180;
	PhongSize  = 32;

	// Create a custom palette.	
	Palette = new( GetParent() )UPalette;
	for( INT  i=0; i<256; i++ )
		Palette->Colors.AddItem( FColor(0,0,0) );
	BlueLagunaPalette(Palette);
	MipZero = Palette->Colors(128);

	unguard;
}

void UWaveTexture::PostLoad()
{
	guard(UWaveTexture::PostLoad);

	// Call base class.
	UWaterTexture::PostLoad();

	// Make sure the texture has its _own_ copy of the palette.
#if COPYPALETTE	
	if( Palette!=NULL && GetParent()!=Palette->GetParent() )
	{
		UPalette* NewPalette = new( GetParent(), GetName() )UPalette;
		for( INT i=0; i<256; i++ )
			NewPalette->Colors.AddItem( Palette->Colors(i) );
		Palette = NewPalette->ReplaceWithExisting();
		MipZero = Palette->Colors(128);
		GCache.Flush();
	}
#endif
	
	// Recalculate the rendering LUT
	// Depends on: WaveAmp, PhongRange, and PhongSize.
	SetWaveLight(); //#debug Check params before superfluous updates ??

	unguard;
}

void UWaveTexture::Clear( DWORD ClearFlags )
{
	guard(UWaveTexture::Clear);

	// Init to zero.
	UWaterTexture::Clear( ClearFlags );

	// Clear fields.
	if( ClearFlags & TCLEAR_Bitmap )
		for( INT  i=0; i< USize * VSize / 2; i++ )
			SourceFields[i] = 128;

	// Clear drops.
	if( ClearFlags & TCLEAR_Temporal )
		NumDrops = 0;

	unguard;
}

void UWaveTexture::ConstantTimeTick()
{
	guard(UWaveTexture::ConstantTimeTick);

	if ((USize>=8) && (VSize>=8)) // safe sizes ?
	{
		WaterRedrawDrops();
		CalculateWater(); 
	}

	unguard;
}


void UWaveTexture::SetWaveLight() 
{
	guard(UWaveTexture::SetWaveLight);

	// Compute shades for surface normals.
    FLOAT Lamp   = PI * BumpMapLight / 255.0;
    FLOAT Viewer = PI * BumpMapAngle / 255.0;

    for( INT  i=0; i<1024 ; i++ )
    {
		// Get reflection magnitude.
        FLOAT Normal = FakeAtan(  ((FLOAT)WaveAmp/255.0F) * (512.0F - (float)i) / 196.0F )  + (PI * 0.5F);

        // Max in this palette is 255, diffuse light reaches to 256-(PhongRange/2). 
        INT  TempLight = (int) ( (256-(PhongRange/2)) * appCos ( Normal- Lamp ) );   //* ((FLOAT)WaveAmp/256.0F) );

        // Create a phong-ish highlight.
        // Based on angle between viewer direction and reflected light:
        // Reflected light angle : = Normal*2 - Lamp.
		// Old defaults: PhongRange = 75; PhongRadius = 0.11;
		FLOAT PhongRadius = (FLOAT) PhongSize / 512.0f; 
        FLOAT Reflected = (Normal*2 - Lamp);
        if( Square(Reflected-Viewer) < Square(PhongRadius) )
            TempLight +=  (PhongRange * 2 ) * (PhongRadius - Abs(Reflected-Viewer)) / PhongRadius;

        RenderTable[i] = Clamp( TempLight, 0, 255 );
    }
	unguard;
}


IMPLEMENT_CLASS(UWaveTexture);



/*----------------------------------------------------------------------------
	UWetTexture implementation.
----------------------------------------------------------------------------*/

UWetTexture::UWetTexture()
{
	guard(UWetTexture::UWetTexture);
	OldSourceTex = this; // Start self-referential ?
	LocalSourceBitmap = NULL;
	unguard;
}

void UWetTexture::Init( INT  InUSize, INT  InVSize )
{
	guard(UWetTexture::Init);

	// Init base class.
	UWaterTexture::Init( InUSize, InVSize );

	// Create a custom palette.
	Palette = new( GetParent() )UPalette;
	for( INT  i=0; i<256; i++ )
		Palette->Colors.AddItem( FColor(i,i,i) );
	BlueLagunaPalette(Palette);
	MipZero = Palette->Colors(128);

	unguard;
}

void UWetTexture::PostLoad()
{
	guard(UWetTexture::PostLoad);

	// Call base class.
	UWaterTexture::PostLoad();

	// Update palette if new source texture selected;
	// If through mipmap cutting our old source isn't available, 
	// try to find the current mipmap and create a local copy that *is* 
	// the right size.

	if (!SourceTexture) return;

	// Make sure this is the same size as our displacement-waves texture.
	if ( ( SourceTexture->UBits  != UBits) || (SourceTexture->VBits  != VBits)  )
	{
		// Size changed; try to recover a source texture by upsampling..

		INT UScaler = UBits - SourceTexture->UBits;
		INT VScaler = VBits - SourceTexture->VBits;

		if ((UScaler>0 ) && (VScaler>0))
		{
			LocalSourceBitmap = new BYTE[ USize * VSize ]; 
			BYTE* SourceMapAddr  = &SourceTexture->GetMip(0)->DataArray(0);
			
			for (INT V=0; V< VSize; V++)
			{
				for (INT U=0; U< USize; U++)
				{	
					LocalSourceBitmap[ U + ( V * USize )] = 
						SourceMapAddr[ ( U >> UScaler ) + ((V >> VScaler) << SourceTexture->UBits) ];
				}
			}
			
		}
		else 
		{
			SourceTexture = NULL; // Give up trying to recover a valid sourcetexture.
			return;
		}
	}

	if ( SourceTexture != OldSourceTex )
	{
		Palette = SourceTexture->Palette; // Make sure palette pointer gets updated.
		OldSourceTex = SourceTexture;
	}
	OldSourceTex = SourceTexture;

	// Recalculate the rendering LUT if needed.
	if (WaveAmp != OldWaveAmp)
	{
		SetRefractionTable();
	}
	OldWaveAmp = WaveAmp;

	unguard;
}

void UWetTexture::Clear( DWORD ClearFlags )
{
	guard(UWetTexture::Clear);

	// Init to zero.
	UWaterTexture::Clear( ClearFlags );

	// Clear fields.
	if( ClearFlags & TCLEAR_Bitmap )
		for( INT  i=0; i< USize * VSize / 2; i++ )
			SourceFields[i] = 128;

	// Clear drops.
	if( ClearFlags & TCLEAR_Temporal )
		NumDrops = 0;

	unguard;
}

void UWetTexture::ConstantTimeTick()
{
	guard(UWetTexture::ConstantTimeTick);

	if ((SourceTexture) && ((USize>=8) && (VSize>=8))) // safe sizes ?
	{
		//Update the water.
		WaterRedrawDrops();
		CalculateWater();  
        ApplyWetTexture();
	}
	unguard;
}

//
// Called from PostLoad  - for Wet texture it's simply linear.
//

void UWetTexture::SetRefractionTable()
{
	guard(UWetTexture::SetRefractionTable);
	// 'zero' is 512
	for( INT  i=0; i<1024 ; i++ )
    {
		int TempLight = (+i-511) * ((FLOAT)WaveAmp/512.0F); 
		RenderTable[i] = Clamp( TempLight, -128, 127 );
    }
	unguard;
}

void UWetTexture::Destroy()
{
	guard(UWetTexture::Destroy);

	// Free memory.
	if( LocalSourceBitmap )
		delete LocalSourceBitmap;

	// must call direct parents' class destroy.	
	Super::Destroy();
	unguard;
}



IMPLEMENT_CLASS(UWetTexture);

/*----------------------------------------------------------------------------
	UIceTexture object implementation.
----------------------------------------------------------------------------*/

UIceTexture::UIceTexture()
{
	guard(UIceTexture::UIceTexture);
	
	MasterCount = 0.0f;
	OldUDisp =   -1;
	OldVDisp =   -1;

	OldSourceTex = NULL;
	OldGlassTex = NULL;

	LocalSourceBitmap = NULL;

	ForceRefresh = 1;

	unguard;
}

void UIceTexture::PostLoad()
{
	guard(UIceTexture::PostLoad);

	// Call base class.
	UFractalTexture::PostLoad();

	if (!SourceTexture) return;

	// Validate all sizes: minima & matching requirements.
	if ((GlassTexture) && (SourceTexture)) 
	{
		if  (  ((USize<8) || (VSize<8))  )
		{
			SourceTexture = NULL;
			GlassTexture = NULL;
		}
		else
		{
			UBOOL  NoGlass = ((  GlassTexture->USize  != USize ) || (  GlassTexture->VSize  != VSize ) );
			UBOOL NoSource = ((  SourceTexture->USize != USize ) || (  SourceTexture->VSize != VSize ) );

			if (NoGlass || NoSource) 
			{
				// If either glass or source texture changed, just no longer animate, but DO
				// copy whatever's in the source texture to be sure there's something valid
				// on display..... and upsample if necessary.

				// Try to copy source texture to current texture.
				// Size changed; try to recover a source texture by upsampling..

				INT UScaler = UBits - SourceTexture->UBits;
				INT VScaler = VBits - SourceTexture->VBits;

				if ((UScaler>=0 ) && (VScaler>=0))
				{
					LocalSourceBitmap = &GetMip(0)->DataArray(0);  // our default data ?
					BYTE* SourceMapAddr  = &SourceTexture->GetMip(0)->DataArray(0);
					
					for (INT V=0; V< VSize; V++)
					{
						for (INT U=0; U< USize; U++)
						{	
							LocalSourceBitmap[ U + ( V * USize )] = 
								SourceMapAddr[ ( U >> UScaler ) + ((V >> VScaler) << SourceTexture->UBits) ];
						}
					}
					
				}

				GlassTexture = NULL;
				SourceTexture = NULL;
			}
		}
	}

	ForceRefresh = 1;

	unguard;
}

void UIceTexture::Init( INT  InUSize, INT  InVSize )
{
	guard(UIceTexture::Init);

	// Init base class.
	UFractalTexture::Init( InUSize, InVSize );

	UDisplace = 0;
	VDisplace = 0;

	HorizPanSpeed = 128;
	VertPanSpeed  = 128;

	PanningStyle = SLIDE_Linear;
	Frequency    = 11;
	Amplitude    = 44;

	MoveIce=1;  // move around the ICE if 1, TEXTURE if 0.

	// Create a dummy palette.
	Palette = new( GetParent() )UPalette;
	for( INT  i=0; i<256; i++ )
		Palette->Colors.AddItem( FColor(i,i,i) );
	MipZero = Palette->Colors(128);
	
	unguard;
}

void UIceTexture::Clear( DWORD ClearFlags )
{
	guard(UIceTexture::Clear);

	// Init to zero.
	UFractalTexture::Clear( ClearFlags );
	
	unguard;
}

void UIceTexture::ConstantTimeTick()
{
	guard(UIceTexture::ConstantTimeTick);

	// Simulated 'deltaseconds' for 120 fps.
	RenderIce(1.0f/120.0f );
	
	unguard;
}

// Full override of Tick().
// Ice is non-iterative so its panning can be synchronized to 
// any real-time speed.

void UIceTexture::Tick(FLOAT DeltaSeconds)  //"raw tick" override
{
	guard(UIceTexture::Tick);

	// Revert to parent Tick() if panning stile is 'FrameSynced'. 
	if (TimeMethod == 0) 
	{	
		// Call parent, which might call UIceTexture::ConstantTimeTick 
		UFractalTexture::Tick(DeltaSeconds); 
		return;
	}

	RenderIce(DeltaSeconds);	
	unguard;
}

void UIceTexture::RenderIce(FLOAT DeltaTime)
{
	guard(UIceTexture::RenderIce);

    // Safety & update checks. 
	if ( (GlassTexture == NULL) || (SourceTexture == NULL) ) return;

   	if ( SourceTexture != OldSourceTex )
	{
		Palette = SourceTexture->Palette;
		OldSourceTex = SourceTexture;
		ForceRefresh = 1;
	}

	if ( GlassTexture != OldGlassTex )
	{
		OldGlassTex = GlassTexture;
		ForceRefresh = 1;
	}

	MoveIcePosition(DeltaTime);

	// Skip update if last position == new position.
	if (   (appRound(UPosition) == OldUDisp) 
		&& (appRound(VPosition) == OldVDisp)
		&& (! ForceRefresh) ) 
		return;

	OldUDisp = appRound( UPosition );
	OldVDisp = appRound( VPosition );

	// Actual redrawing.
	if  (! MoveIce ) BlitIceTex();
		 else        BlitTexIce();

	ForceRefresh = 0;

	unguard;
}

void UIceTexture::MousePosition( DWORD Buttons, FLOAT X, FLOAT Y )
{
	guard(UIceTexture::MousePosition);

	static FLOAT  MouseLastU = 0.0f;
	static FLOAT  MouseLastV = 0.0f;

	if (Buttons & MOUSE_Left)
	{ 
		UDisplace +=(MouseLastU - X);
		VDisplace +=(MouseLastV - Y);

		UPosition = UDisplace;
		VPosition = VDisplace;
	}

	MouseLastU = X;
	MouseLastV = Y;

	unguard;
}

void UIceTexture::Click( DWORD Buttons, FLOAT X, FLOAT Y )
{
	guard(UIceTexture::Click);
	unguard;
}

void UIceTexture::Destroy()
{
	guard(UWetTexture::Destroy);

	// Free memory.
	if( LocalSourceBitmap )
		delete LocalSourceBitmap;

	UTexture::Destroy(); // must call base class' destroy.

	unguard;
}



IMPLEMENT_CLASS(UIceTexture);


/*----------------------------------------------------------------------------
	The End.
----------------------------------------------------------------------------*/
