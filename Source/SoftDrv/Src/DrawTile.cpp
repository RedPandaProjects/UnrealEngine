/* ==============================================================================================

	DrawTile.cpp: Textured tile drawing.
	Copyright 1997 Epic MegaGames, Inc. This software is a trade secret.

	Revision history:
		* Created by Erik de Neve
	    
	Devnotes
	- allocates a destination-format palette for masked & normal surfaces,
	  and an MMX/float palette for modulated/translucent; along lines of
	  DrawPoly.C, with format-specific cache ID's
    - Relies on caller for correct clipping.

	Todo
	- Comment out the 'clipping check' below once GPF bug tracked down.
	- Feature-completeness check for all cases: ASM 0 and 1, GIsMMX true and false !
	- Better modulation in some cases by using MMX  ?
	- PF_Renderhint indicates nonscaled sprites, but a lot of calls without that flag set
	  are also unscaled -> therefore currently we use explicit detection.
	- LAST line is not properly span-clipped in (debug-fullbright) modulation sprites;
	  seems to extend to left completely. -> higher-level bug ?
	- Mipmapping: does it check du/dx and dv/dy as it should ?
	- writing out 2 aligned dwords with MMX is a speedup also..
	- Make some (dithering-use) smoothing on really close polys ? -> perhaps cool for the 
	  showoff-case of MMX/32bit.
    - Mipfactor is non-configurable for tiles, use member var DetailBias as in DrawSurf ??
	 
============================================================================================== */

#include "SoftDrvPrivate.h"

#pragma warning( disable : 4799 ) /* no EMMS instruction */

#define TESTMASKALL  0  // Mask ALL sprites
#define MODINDICATOR 0  // Have modulated show up full-white 


static DWORD SavedESP, SavedEBP, SavedEAX;
static DWORD ModLut[1024];
static DWORD ModLutInit = 0;

struct LitPalette
{
	FVector PalFog;
	FVector PalLight;
	DWORD Flags;
	FRainbowPtr OutPalette;
};

struct SpriteTexSetup
{
	INT VCo;
	INT UCo;
	INT UCoDelta;
	INT VCoDelta;
	INT UMask;
	INT VMask;
	INT USize;
	INT VSize;
	INT UBits;
	BYTE* TexBase;
	FRainbowPtr Palette;
	FRainbowPtr Screen;
	INT MinX,SizeX,MinY,SizeY,MaxX,MaxY;
};

static SpriteTexSetup Spr;

static DWORD UCoordTable[MaximumXScreenSize + 16]; 

//
// Read pixels and output to RGB 32/24-bit format.
//
void USoftwareRenderDevice::ReadPixels( FColor* Pixels )
{
	guardSlow(USoftwareRenderDevice::ReadPixels);
	Lock( FPlane(0,0,0,0), FPlane(0,0,0,0), FPlane(0,0,0,0), 0, NULL, NULL );
	for( INT i=0; i<Viewport->SizeY; i++ )
	{
		if (Viewport->ColorBytes==2)
		{
			if (Viewport->Caps & CC_RGB565) 
			{
				// 565
				_WORD* W = (_WORD*)Viewport->_Screen(0,i);
				for( INT j=0; j<Viewport->SizeX; j++ )
				{			
					_WORD WCol = *W++;
					*Pixels++ = FColor( (WCol << 3) & 0xF8, (WCol >> 3) & 0xFC, (WCol >> 8) & 0xF8 );
				}
			}
			else 
			{
				// 555
				_WORD* W = (_WORD*)Viewport->_Screen(0,i);
				for( INT j=0; j<Viewport->SizeX; j++ )
				{	
					_WORD WCol = *W++;
					*Pixels++ = FColor( (WCol << 3) & 0xF8, (WCol >> 2) & 0xF8, (WCol >> 7) & 0xF8 );
				}
			}
		}
		else if( Viewport->ColorBytes==4 )
		{
			DWORD* D = (DWORD*)Viewport->_Screen(0,i);
			for( INT j=0; j<Viewport->SizeX; j++ )
			{
				FColor C = *(FColor*)D++;
				*Pixels++ = C;   //FColor(C.R,C.G,C.B);
			}
		}
	}
	Unlock( 0 );
	unguardSlow;
}


//
// Fast clear-screen code. (Consider it a special tile drawer..)
//

void USoftwareRenderDevice::ClearScreenFast32(DWORD* Dest, DWORD Color)
{

	guardSlow(USoftwareRenderDevice::ClearScreenFast32);

#if !ASM
	for( int i=0; i< Viewport->SizeY; i++,Dest+= Viewport->Stride )
		for( int j=0; j< Viewport->SizeX; j++ )
	 		Dest[j]=Color;
#endif

#if ASM
			DWORD SizeX = Viewport->SizeX;
			DWORD SizeY = Viewport->SizeY;
			DWORD Stride = Viewport->Stride;

			// Clear the screen using assembler, using MMX if detected.		
			__asm
			{
				mov     ecx,[SizeX]			
				mov		esi,[Dest]
				test    ecx,ecx
				jz		Clear32End
				xor     eax,eax
				mov     edi,GIsMMX
				mov     eax,[Color]
				mov     edx,[SizeY]
				mov     ebx,[Stride]   // Pixel stride -> dword stride
				add     ebx,ebx
				add     ebx,ebx      
				test    edx,edx
				mov     edi,[edi]  // GIsMMX
				jz      Clear32End
				test    edi,edi
				jnz      MMXCLEAR32

				// Clear using aligned 32-bit stores. 
				// Y loop
				align 16
				YLoop32Pentium:				   
				    mov     edi,esi
					mov     ecx,[SizeX]
				    add     esi,ebx     // Dest += dword stride
				//////////////////////////
					test    ecx,ecx
					jz      SkipLoop32	//
					rep     stosd		// aligned writes.
				SkipLoop32:

					dec     edx			// sizeY
					jnz     YLoop32Pentium
				jmp Clear32End			//
				//////////////////////////
				
				// Clear using MMX. Adjusts destination alignment & pads if needed.
				MMXCLEAR32: 
					// Expand data to 32:32
				    movd    mm0,eax
					movd    mm1,eax
				    psllq   mm0,32
					por     mm0,mm1

				align 16
				YLoop32MMXPentium:				   
				    mov     edi,esi
					mov     ecx,[SizeX]
				    add     esi,ebx     // Dest += dword stride

				//////////////////////////////
					test    edi,4			// Destination alignment on 4 dwords ?
					jz      SkipPad32MMX	
					mov     [edi],eax		// Single dword start pad.
					add     edi,4
					dec     ecx				
					jz      Line32MMXEnd
				SkipPad32MMX:			//

					push    ecx			//
					and     ecx,~1      //  Even # of 32-bit words. 
					jz      SkipLoop32MMX	
					shr     ecx,1       //  dword counter to quadword counter 

					// Blit ecx * quadwords...
					align 16				
				InnerLoop32MMX:				
					movq    [edi],mm0		
					add     edi,8			
					dec     ecx				
					jnz     InnerLoop32MMX	

				//EndInner32MMX:

				SkipLoop32MMX:			 //
					pop		ecx			 //
					and     ecx,1        // Single dword left to do ?    
					jz      Line32MMXEnd //
					mov     [edi],eax	 // Single dword end pad.

					Line32MMXEnd:		 //
					dec     edx			 // SizeY
					jnz     YLoop32MMXPentium
										 //
					emms                 //
										 //
				jmp Clear32End			 //
				///////////////////////////

				Clear32End:
			}
#endif
	unguardSlow;
}


void USoftwareRenderDevice::ClearScreenFast16(_WORD* Dest, DWORD Color)
{
	guardSlow(USoftwareRenderDevice::ClearScreenFast16);

#if !ASM

	for( int i=0; i<Viewport->SizeY; i++,Dest+=Viewport->Stride )
		for( int j=0; j<Viewport->SizeX; j++ )
			Dest[j]=(_WORD)Color;

#endif

#if ASM
		DWORD SizeX  = Viewport->SizeX;
		DWORD SizeY  = Viewport->SizeY;
		DWORD Stride = Viewport->Stride;
		// Clear the screen using assembler, using MMX if detected.		
			__asm
			{
				mov     ecx,[SizeX]			
				mov		esi,[Dest]
				test    ecx,ecx
				jz		Clear16End
				xor     eax,eax
				mov     eax,[Color]
				mov     ebx,eax
				mov     edi,GIsMMX
				mov     edx,[SizeY]
				and     ebx,0x0000ffff
				shl     eax,16
				add     eax,ebx				
				mov     ebx,[Stride] // pixel stride -> _word stride
				add     ebx,ebx
				test    edx,edx
				mov     edi,[edi] // GIsMMX 
				jz      Clear16End
				test    edi,edi
				jnz     MMXCLEAR16

				// Clear using aligned 32-bit stores. 
				// Y loop
				align 16
				YLoop16Pentium:				   
				    mov     edi,esi
					mov     ecx,[SizeX]
				    add     esi,ebx     // Dest += stride
				//////////////////////////
					test    edi,2       // Destination alignment ?
					jz      SkipPad16	//
					mov     [edi],ax	// single word start pad.
					dec     ecx			//
					add     edi,2       // edi now dword aligned.
				SkipPad16:				//
					push    ecx			//
					and     ecx,0xfffffffe // Even number of 16-bit words
					jz      SkipLoop16	//
					shr     ecx,1       // word counter to dword counter 
					rep     stosd		// aligned writes.
				SkipLoop16:
					pop		ecx			//
					and     ecx,1       // single word left to do ?    
					jz      Line16End	//
					mov     [edi],ax	// single word end pad.

					Line16End:			//
					dec     edx			// sizeY
					jnz     YLoop16Pentium
				jmp Clear16End			//
				//////////////////////////
				
				// Clear using MMX. Adjusts destination alignment & pads if needed.
				MMXCLEAR16: 
					// expand data to 16:16:16:16					
				    movd    mm0,eax
					movd    mm1,eax
				    psllq   mm0,32
					por     mm0,mm1

				align 16
				YLoop16MMXPentium:				   
				    mov     edi,esi
					mov     ecx,[SizeX]
				    add     esi,ebx     // Dest += stride

				//////////////////////////////
					test    edi,6			// Destination alignment on 4 dwords ?
					jz      SkipPad16MMX	
					mov     [edi],ax		// Single word start pad.
					add     edi,2
					dec     ecx				//
					jz      Line16MMXEnd
	
					test    edi,6			// 
					jz      SkipPad16MMX
					mov     [edi],ax		//
					add     edi,2			// edi now dword aligned.
					dec     ecx				//
					jz      Line16MMXEnd

					test	edi,6		
					jz      SkipPad16MMX
					mov     [edi],ax	//
					add     edi,2		//
					dec     ecx			//
					jnz     Line16MMXEnd

				SkipPad16MMX:				//
					push    ecx				//
					and     ecx,~3			//  multiple-of-8 x 16-bit words. 
					jz      SkipLoop16MMX	//
					shr     ecx,2			//  word counter to quadword counter 

					// Blit ecx * quadwords...
					align 16				//
				InnerLoop16MMX:				//
					movq    [edi],mm0		// Loop cycles (3) amortized by write-overhead.
					add     edi,8			//
					dec     ecx				//
					jnz     InnerLoop16MMX	//

				//EndInner16MMX:

				SkipLoop16MMX:			//
					pop		ecx			//
					and     ecx,3       // single word left to do ?    
					jz      Line16MMXEnd	

					mov     [edi],ax	// Single word end pad.
					dec     ecx			 
					jz      Line16MMXEnd

					mov     [edi+2],ax	 
					dec     ecx			 
					jz      Line16MMXEnd

					mov     [edi+4],ax	 

					Line16MMXEnd:		 
					dec     edx			// sizeY
					jnz     YLoop16MMXPentium

					emms                //
				//////////////////////////

				Clear16End:
			}
#endif
	unguardSlow;
}

//
// Not implemented - no advantages over current ones ???
// void FlashSprite15ModulatedMMX( FSpanBuffer* Span, INT GByteStride)
// {}
// void FlashSprite16ModulatedMMX( FSpanBuffer* Span, INT GByteStride)
// {}
//


// Blit unscaled tile, no fixed point coordinates needed.

void USoftwareRenderDevice::BlitTile32(FSpanBuffer* Span)
{
	guardSlow(USoftwareRenderDevice::BlitTile32);

	Spr.VCo &= Spr.VMask;
	Spr.UCo &= Spr.UMask;

	if (!Span) 
	{	 
		// Blit without span clipping. 
		if (  ( (Spr.UCo + Spr.SizeX) > (Spr.USize) ) || ( (Spr.VCo + Spr.SizeY) > (Spr.VSize) ) ) 
		{

			// Wrapping C++
#if !ASM
			for ( INT L = Spr.MinY; L < Spr.MaxY; L++, Spr.Screen.PtrBYTE += GByteStride )
			{								
				INT UWalk = Spr.UCo;
				BYTE* TexelPtr = &Spr.TexBase[0] + ( Spr.VCo << Spr.UBits );
				Spr.VCo = ( ++Spr.VCo & Spr.VMask );
				for ( INT J = Spr.MinX; J < Spr.MaxX; J++ )
				{
					BYTE Texel = TexelPtr[UWalk];
					UWalk = ( (++UWalk) & Spr.UMask );
					Spr.Screen.PtrDWORD[J] =  Spr.Palette.PtrDWORD[Texel];
				}
			}
#endif

			// Wrapping, ASM
#if ASM
			static DWORD Stride, TexMapMask;
			static INT NegativeXPixels;
			static BYTE* TexelLimit;

			Spr.Screen.PtrDWORD += Spr.MinX;
			BYTE* TexelPtr = &Spr.TexBase[0] + ( Spr.VCo << Spr.UBits );		
			TexelLimit = &Spr.TexBase[0] + ( Spr.VSize << Spr.UBits );

			Stride = GByteStride;
			NegativeXPixels = - Spr.SizeX + 1;
			
			__asm
			{
				// esi = texture current line start,  offset by -SizeX.
				// edx = screen destination start,    offset by -SizeX.
				// edi = palette base

				mov     ecx, [Spr]SpriteTexSetup.SizeX 
				mov		edx, [Spr]SpriteTexSetup.Screen
				mov		esi, [TexelPtr]
				mov		edi, [Spr]SpriteTexSetup.Palette

				lea     edx, [edx+ecx*4-4]  // + N*4 - 4  screen base biasing

				mov		[SavedESP],esp
				mov		[SavedEBP],ebp

				mov     esp,[Spr]SpriteTexSetup.UMask

			align 16
			Blit32ReloopY:
				mov     eax,[Spr]SpriteTexSetup.UCo
				mov     ebp,[NegativeXPixels]   

				// Lead-in				
				xor     ebx,ebx				//
				mov     bl,[esi+eax]		// edx: starts as [0+N]-N 
				test    ebp,ebp				//
				jz      EndLineBlitWrap		//
				add     eax,1				//
				and     eax,esp				//
				
				// Inner loop
				align 16
				Blit32ReloopX:

				mov ecx,[edi+ebx*4]			// Get palette color 
				xor ebx,ebx					// 

				mov bl,[esi+eax]			// Get texture texel.
				add eax,1					// Advance texel

				mov [edx+ebp*4],ecx			// Write to screen
				and  eax,esp				// Mask

				add  ebp,1					// Screen
 				jnz  Blit32ReloopX			// 

				EndLineBlitWrap:			

				// Lead-out
					mov     ecx,[edi+ebx*4]		// Load from palette
					mov     [edx+ebp*4],ecx		// Store

				// Reloop for next line ?
					mov     ebx,[Spr]SpriteTexSetup.USize
					mov     ecx,[Stride]					//
					mov     eax,[Spr]SpriteTexSetup.SizeY	//
					add     esi,ebx	// Next line in texture map
					mov     ebx,[TexelLimit]
					cmp     esi,ebx   // wrap texture map Y coordinate 
					jb      NoYWrap
					mov     esi,[Spr]SpriteTexSetup.TexBase //
					NoYWrap:

					add		edx,ecx	  // Destination + linestride 
					dec     eax
					mov     [Spr]SpriteTexSetup.SizeY,eax
				jnz     Blit32ReloopY

				mov		ebp,[SavedEBP]
				mov		esp,[SavedESP]

			}	
#endif

		}
		else
		{
			//non-wrapping C++
			//non-wrapping ASM
#if  !ASM	

			for ( INT L = Spr.SizeY; L > 0; L--, Spr.Screen.PtrBYTE += GByteStride )
			{								
				INT UWalk = Spr.UCo;
				BYTE* TexelPtr = &Spr.TexBase[0] + ( Spr.VCo++ << Spr.UBits );
				for ( INT J = Spr.MinX; J < Spr.MaxX; J++ )
				{
					BYTE Texel = TexelPtr[UWalk++]; 
					Spr.Screen.PtrDWORD[J] = Spr.Palette.PtrDWORD[Texel]; 
				}
			}
#endif


#if ASM			
			BYTE* TexelPtr = &Spr.TexBase[0] + (Spr.VCo << Spr.UBits ) + Spr.UCo +  Spr.SizeX;
			DWORD Stride = GByteStride;
			Spr.Screen.PtrDWORD += Spr.MinX;
			INT NegativeXPixels = -Spr.SizeX + 1;
			
			__asm
			{
				// esi = texture current line start,  offset by -SizeX.
				// edx = screen destination start,    offset by -SizeX.
				// edi = palette base
				mov     ecx, [Spr]SpriteTexSetup.SizeX 
				mov		edx, [Spr]SpriteTexSetup.Screen
				mov		esi, [TexelPtr]
				mov		edi, [Spr]SpriteTexSetup.Palette
				lea     edx, [edx+ecx*4-4]  // +N*4 - 4 

			align 16
			BlitWrap32ReloopY:
				mov     eax,[NegativeXPixels]   

				// Lead-in 
				xor     ebx,ebx			//
				test    eax,eax         // Check for single-pixel case.
				mov     bl,[esi+eax-1]	// Esi: starts as [0+N]-N 
				je      EndLineWrap     // 
				
				// Inner loop
				align 16
				BlitWrap32ReloopX:				
					mov		ecx,[edi+ebx*4]		//  Get this color dword from palette edi
					xor     ebx,ebx				//					

					mov		[edx+eax*4],ecx		//  Store this color 32-bit dword to screen.
					mov		bl,[esi+eax]		//  Get next source texel 

					add     eax,1    			//  0 == end of line...
					jnz     BlitWrap32ReloopX	    //

				EndLineWrap:	

				// Lead-out
					mov     ebx,[edi+ebx*4]		// Load from palette

				// Reloop for next line ?
					mov     ecx,[Stride]		//
					mov     [edx+eax*4],ebx		// Store
					mov     eax,[Spr]SpriteTexSetup.SizeY
					add     esi,[Spr]SpriteTexSetup.USize	// Next line in texture map
					add		edx,ecx				// Destination + linestride 
					dec     eax
					mov     [Spr]SpriteTexSetup.SizeY,eax
					jnz     BlitWrap32ReloopY

			}	
#endif

		}
	}
	else
	{
		// Blit with span clipping. No fixed point needed.

		FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

		for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
		{
			BYTE* TexelPtr = &Spr.TexBase[0] + ( Spr.VCo << Spr.UBits );
			Spr.VCo = ( (++Spr.VCo) & Spr.VMask );

			INT UWalk = Spr.UCo;
		
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{ 
				INT SpanX0 = Spr.MinX;  
				INT SpanX1 = Spr.MaxX;

				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				if ( SpanX0 < Span->Start ) 
				{
					UWalk = (Spr.UCo + (Span->Start - SpanX0)) & Spr.UMask; // prestepping.
					SpanX0 = Span->Start;
				}

				if (SpanX1 > SpanX0) // Any to draw ?
				{
					for ( INT J = SpanX0; J < SpanX1; J++)
					{
						BYTE Texel = TexelPtr[UWalk]; 
						UWalk = ( (++UWalk) & Spr.UMask );
						Spr.Screen.PtrDWORD[J] =  Spr.Palette.PtrDWORD[Texel];
					}
				}
			} 
		}// per line 			
	}
	unguardSlow;
}



// Blit unscaled masked tile (FONTS!) no fixed point coordinates needed.

void USoftwareRenderDevice::BlitMask32(FSpanBuffer* Span)
{
	guardSlow(USoftwareRenderDevice::BlitMask32);

	Spr.VCo &= Spr.VMask;
	Spr.UCo &= Spr.UMask;

	if (!Span) 
	{	 
		// Detect if any texture wrapping is used.
		if (  ( (Spr.UCo + Spr.SizeX) > (Spr.USize) ) || ( (Spr.VCo+Spr.SizeY) > (Spr.VSize) ) ) 
		{
			// wrapping version
			for ( INT L = Spr.SizeY; L > 0; L--, Spr.Screen.PtrBYTE += GByteStride )
			{								
				INT UWalk = Spr.UCo;
				BYTE* TexelPtr = &Spr.TexBase[0] + ( Spr.VCo << Spr.UBits );
				Spr.VCo = ( ++Spr.VCo & Spr.VMask );
				
				for ( INT J = Spr.MinX; J < Spr.MaxX; J++ )
				{
					BYTE Texel = TexelPtr[UWalk]; 
					UWalk = ( (++UWalk) & Spr.UMask );
					if (Texel) Spr.Screen.PtrDWORD[J] = Spr.Palette.PtrDWORD[Texel];
				}
			}
		}
		else
		{

		// Non-wrapping versions.

#if  !ASM	

			for ( INT L = Spr.SizeY; L > 0; L--, Spr.Screen.PtrBYTE += GByteStride )
			{								
				INT UWalk = Spr.UCo;
				BYTE* TexelPtr = &Spr.TexBase[0] + ( Spr.VCo++ << Spr.UBits );
				for ( INT J = Spr.MinX; J < Spr.MaxX; J++ )
				{
					BYTE Texel = TexelPtr[UWalk++]; 
					if (Texel) Spr.Screen.PtrDWORD[J] = Spr.Palette.PtrDWORD[Texel]; 
				}
			}
#endif


#if ASM			
			BYTE* TexelPtr = &Spr.TexBase[0] + (Spr.VCo << Spr.UBits ) + Spr.UCo +  Spr.SizeX;
			DWORD Stride = GByteStride;
			Spr.Screen.PtrDWORD += Spr.MinX;
			INT NegativeXPixels = -Spr.SizeX + 1;
			
			__asm
			{
				// esi = texture current line start,  offset by -SizeX.
				// edx = screen destination start,    offset by -SizeX.
				// edi = palette base
				mov     ecx, [Spr]SpriteTexSetup.SizeX 
				mov		edx, [Spr]SpriteTexSetup.Screen
				mov		esi, [TexelPtr]
				mov		edi, [Spr]SpriteTexSetup.Palette
				lea     edx,[edx+ecx*4-4]  // +N*4 - 4 

			align 16
			Blit32ReloopY:
				mov     eax,[NegativeXPixels]   

				// Lead-in				
				xor     ebx,ebx			//
				test    eax,eax         // Check for single-pixel case.
				mov     bl,[esi+eax-1]	// esi: starts as [0+N]-N 
				je      EndLineMask     //
				
				// inner loop
				align 16
				Blit32ReloopX:				
					test    ebx,ebx				
					jz      DontTexel			      
				DoTexel:					
					mov		ecx,[edi+ebx*4]		// Get this color dword from palette edi
					xor     ebx,ebx				//					
					
					mov		[edx+eax*4],ecx		// Store this color 32-bit dword to screen.
					mov		bl,[esi+eax]		// get next source texel 

					add     eax,1    			// 0 == end of line...
					jnz     Blit32ReloopX	
				EndLineMask:					// next line of masked, unscaled tile.

				// Lead-out
					test    ebx,ebx
					jz      DontDoLastTexel
					mov     ecx,[edi+ebx*4]  // load from palette

					mov     [edx+eax*4],ecx  // store
					DontDoLastTexel:

				// Reloop for next line ?
					mov     eax,[Spr]SpriteTexSetup.SizeY
					mov     ecx,[Stride]	
					add     esi,[Spr]SpriteTexSetup.USize // Next line in texture map
					add		edx,ecx		// Destination + linestride 
					dec     eax
					mov     [Spr]SpriteTexSetup.SizeY,eax
					jnz     Blit32ReloopY
					jmp     EndOfBlit
				////////////////////////		
								
				// Masking coroutine
				align 16
				InMask32Reloop:
					test	ebx,ebx
					jnz		DoTexel

				DontTexel:
					xor		ebx,ebx
					mov		bl,[esi+eax]

					add		eax,1
					jnz		InMask32Reloop

				jmp EndLineMask		// go to lead-out
				//////////////////////

				EndOfBlit:

			}	

#endif
		}

	}
	else
	{
		// Blit with span clipping. 

		FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

		for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
		{
			BYTE* TexelPtr = &Spr.TexBase[0] + ( Spr.VCo << Spr.UBits );
			Spr.VCo = ( (++Spr.VCo) & Spr.VMask );

			INT UWalk = Spr.UCo;
		
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{ 
				INT SpanX0 = Spr.MinX;  
				INT SpanX1 = Spr.MaxX;

				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				if ( SpanX0 < Span->Start ) 
				{
					UWalk = (Spr.UCo + (Span->Start - SpanX0)) & Spr.UMask; // prestepping.
					SpanX0 = Span->Start;
				}

				if (SpanX1 > SpanX0) // Any to draw ?
				{
					for ( INT J = SpanX0; J < SpanX1; J++)
					{
						BYTE Texel = TexelPtr[UWalk]; 
						UWalk = ( (++UWalk) & Spr.UMask );
						if (Texel) Spr.Screen.PtrDWORD[J] = Spr.Palette.PtrDWORD[Texel];
					}
				}
			} 
		}// per line 			
	}
	unguardSlow;
}




void USoftwareRenderDevice::BlitTile1516(FSpanBuffer* Span)
{
	guardSlow(USoftwareRenderDevice::BlitTile1516);

	Spr.VCo &= Spr.VMask;
	Spr.UCo &= Spr.UMask;

	if (!Span) 
	{	 
		// Blit without span clipping. 
		if (  ( (Spr.UCo + Spr.SizeX) > (Spr.USize) ) || ( (Spr.VCo + Spr.SizeY) > (Spr.VSize) ) ) 
		{

			// Wrapping C++
#if !ASM
			for ( INT L = Spr.MinY; L < Spr.MaxY; L++, Spr.Screen.PtrBYTE += GByteStride )
			{								
				INT UWalk = Spr.UCo;
				BYTE* TexelPtr = &Spr.TexBase[0] + ( Spr.VCo << Spr.UBits );
				Spr.VCo = ( ++Spr.VCo & Spr.VMask );
				for ( INT J = Spr.MinX; J < Spr.MaxX; J++ )
				{
					BYTE Texel = TexelPtr[UWalk]; 
					UWalk = ( (++UWalk) & Spr.UMask );
					Spr.Screen.PtrWORD[J] =  Spr.Palette.PtrDWORD[Texel];
				}
			}
#endif

			// Wrapping, ASM
#if ASM
			static DWORD Stride, TexMapMask;
			static INT NegativeXPixels;
			static BYTE* TexelLimit;

			Spr.Screen.PtrWORD += Spr.MinX;
			BYTE* TexelPtr = &Spr.TexBase[0] + ( Spr.VCo << Spr.UBits );		
			TexelLimit = &Spr.TexBase[0] + ( Spr.VSize << Spr.UBits );

			Stride = GByteStride;
			NegativeXPixels = - Spr.SizeX + 1;
			
			__asm
			{
				// esi = texture current line start,  offset by -SizeX.
				// edx = screen destination start,    offset by -SizeX.
				// edi = palette base

				mov     ecx, [Spr]SpriteTexSetup.SizeX 
				mov		edx, [Spr]SpriteTexSetup.Screen
				mov		esi, [TexelPtr]
				mov		edi, [Spr]SpriteTexSetup.Palette

				lea     edx, [edx+ecx*2-2]  // + N*4 - 4  screen base biasing

				mov		[SavedESP],esp
				mov		[SavedEBP],ebp

				mov     esp,[Spr]SpriteTexSetup.UMask

			align 16
			Blit1516ReloopY:
				mov     eax,[Spr]SpriteTexSetup.UCo
				mov     ebp,[NegativeXPixels]   

				// Lead-in				
				xor     ebx,ebx				//
				mov     bl,[esi+eax]		// edx: starts as [0+N]-N 
				test    ebp,ebp				//
				jz      EndLineBlitWrap		//
				add     eax,1				//
				and     eax,esp				//
				
				// Inner loop
				align 16
				Blit1516ReloopX:

				mov ecx,[edi+ebx*4]			// Get palette color 
				xor ebx,ebx					// 

				mov bl,[esi+eax]			// Get texture texel.
				add eax,1					// Advance texel

				mov [edx+ebp*2],cx			// Write to screen
				and  eax,esp				// Mask

				add  ebp,1					// Screen
 				jnz  Blit1516ReloopX		// 

				EndLineBlitWrap:			

				// Lead-out
					mov     ecx,[edi+ebx*4]		// Load from palette
					mov     [edx+ebp*2],cx		// Store

				// Reloop for next line ?
					mov     ebx,[Spr]SpriteTexSetup.USize
					mov     ecx,[Stride]					//
					mov     eax,[Spr]SpriteTexSetup.SizeY	//
					add     esi,ebx							// Next line in texture map
					mov     ebx,[TexelLimit]
					cmp     esi,ebx   // wrap texture map Y coordinate ?
					jb      NoYWrap
					mov     esi,[Spr]SpriteTexSetup.TexBase //
					NoYWrap:

					add		edx,ecx	  // Destination + linestride 
					dec     eax
					mov     [Spr]SpriteTexSetup.SizeY,eax
				jnz     Blit1516ReloopY

				mov		ebp,[SavedEBP]
				mov		esp,[SavedESP]

			}	
#endif

		}
		else
		{
			//non-wrapping C++
			//non-wrapping ASM
#if  !ASM	

			for ( INT L = Spr.SizeY; L > 0; L--, Spr.Screen.PtrBYTE += GByteStride )
			{								
				INT UWalk = Spr.UCo;
				BYTE* TexelPtr = &Spr.TexBase[0] + ( Spr.VCo++ << Spr.UBits );
				for ( INT J = Spr.MinX; J < Spr.MaxX; J++ )
				{
					BYTE Texel = TexelPtr[UWalk++]; 
					Spr.Screen.PtrWORD[J] = Spr.Palette.PtrDWORD[Texel]; 
				}
			}
#endif


#if ASM			
			BYTE* TexelPtr = &Spr.TexBase[0] + (Spr.VCo << Spr.UBits ) + Spr.UCo +  Spr.SizeX;
			DWORD Stride = GByteStride;
			Spr.Screen.PtrWORD += Spr.MinX;
			INT NegativeXPixels = -Spr.SizeX + 1;
			
			__asm
			{
				// esi = texture current line start,  offset by -SizeX.
				// edx = screen destination start,    offset by -SizeX.
				// edi = palette base
				mov     ecx, [Spr]SpriteTexSetup.SizeX 
				mov		edx, [Spr]SpriteTexSetup.Screen
				mov		esi, [TexelPtr]
				mov		edi, [Spr]SpriteTexSetup.Palette
				lea     edx, [edx+ecx*2-2]  // +N*4 - 4 

			align 16
			BlitWrap1516ReloopY:
				mov     eax,[NegativeXPixels]   

				// Lead-in 
				xor     ebx,ebx			//
				test    eax,eax         // Check for single-pixel case.
				mov     bl,[esi+eax-1]	// Esi: starts as [0+N]-N 
				je      EndLineWrap     // 
				
				// Inner loop
				align 16
				BlitWrap1516ReloopX:				
					mov		ecx,[edi+ebx*4]		//  Get this color dword from palette edi
					xor     ebx,ebx				//					

					mov		[edx+eax*2],cx		//  Store this color 1516-bit dword to screen.
					mov		bl,[esi+eax]		//  Get next source texel 

					add     eax,1    			//  0 == end of line...
					jnz     BlitWrap1516ReloopX	    //

				EndLineWrap:	

				// Lead-out
					mov     ebx,[edi+ebx*4]		// Load from palette

				// Reloop for next line ?
					mov     ecx,[Stride]		//
					mov     [edx+eax*2],bx		// Store
					mov     eax,[Spr]SpriteTexSetup.SizeY
					add     esi,[Spr]SpriteTexSetup.USize	// Next line in texture map
					add		edx,ecx				// Destination + linestride 
					dec     eax
					mov     [Spr]SpriteTexSetup.SizeY,eax
					jnz     BlitWrap1516ReloopY

			}	
#endif

		}
	}
	else
	{
		// Blit with span clipping. No fixed point needed.

		FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

		for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
		{
			BYTE* TexelPtr = &Spr.TexBase[0] + ( Spr.VCo << Spr.UBits );
			Spr.VCo = ( (++Spr.VCo) & Spr.VMask );

			INT UWalk = Spr.UCo;
		
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{ 
				INT SpanX0 = Spr.MinX;  
				INT SpanX1 = Spr.MaxX;

				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				if ( SpanX0 < Span->Start ) 
				{
					UWalk = (Spr.UCo + (Span->Start - SpanX0)) & Spr.UMask; // prestepping.
					SpanX0 = Span->Start;
				}

				if (SpanX1 > SpanX0) // Any to draw ?
				{
					for ( INT J = SpanX0; J < SpanX1; J++)
					{
						BYTE Texel = TexelPtr[UWalk]; 
						UWalk = ( (++UWalk) & Spr.UMask );
						Spr.Screen.PtrWORD[J] = Spr.Palette.PtrDWORD[Texel];
					}
				}
			} 
		}// per line 			
	}
	unguardSlow;
}




void USoftwareRenderDevice::BlitMask1516(FSpanBuffer* Span)
{
	guardSlow(USoftwareRenderDevice::BlitMask1516);

	Spr.VCo &= Spr.VMask;
	Spr.UCo &= Spr.UMask;

	if (!Span) 
	{	 
		// Blit without span clipping.
		// Detect any wrapping ?

		if ( ( (Spr.UCo + Spr.SizeX) > (Spr.USize) ) || ( (Spr.VCo+Spr.SizeY) > (Spr.VSize) ) ) 
		{
			for ( INT L = Spr.MinY; L < Spr.MaxY; L++, Spr.Screen.PtrBYTE += GByteStride )
			{								
				INT UWalk = Spr.UCo;
				BYTE* TexelPtr = &Spr.TexBase[0] + ( Spr.VCo << Spr.UBits );
				Spr.VCo = ( ++Spr.VCo & Spr.VMask );
				
				for ( INT J = Spr.MinX; J < Spr.MaxX; J++ )
				{
					BYTE Texel = TexelPtr[UWalk]; 
					UWalk = ( (++UWalk) & Spr.UMask );
					if (Texel) Spr.Screen.PtrWORD[J] = Spr.Palette.PtrDWORD[Texel];
				}
			}
		}
		else 
		{
			// non-wrapping versions
		
#if !ASM
			for ( INT L = Spr.SizeY; L > 0; L--, Spr.Screen.PtrBYTE += GByteStride )
			{								
				INT UWalk = Spr.UCo;
				BYTE* TexelPtr = &Spr.TexBase[0] + ( Spr.VCo++ << Spr.UBits );
				for ( INT J = Spr.MinX; J < Spr.MaxX; J++ )
				{
					BYTE Texel = TexelPtr[UWalk++]; 
					if (Texel) Spr.Screen.PtrWORD[J] = Spr.Palette.PtrDWORD[Texel]; 
				}
			}
#endif

			// Non-wrapping ASM version
#if ASM			
			BYTE* TexelPtr = &Spr.TexBase[0] + (Spr.VCo << Spr.UBits ) + Spr.UCo +  Spr.SizeX;
			DWORD Stride = GByteStride;
			Spr.Screen.PtrWORD += Spr.MinX;
			INT NegativeXPixels = -Spr.SizeX + 1;
			
			__asm
			{
				// esi = texture current line start,  offset by -SizeX.
				// edx = screen destination start,    offset by -SizeX.
				// edi = palette base
				mov     ecx, [Spr]SpriteTexSetup.SizeX 
				mov		edx, [Spr]SpriteTexSetup.Screen
				mov		esi, [TexelPtr]
				mov		edi, [Spr]SpriteTexSetup.Palette
				lea     edx,[edx+ecx*2-2]  // +N*2 - 2 

			align 16
			Blit1516ReloopY:
				mov     eax,[NegativeXPixels]   

				// Lead-in				
				xor     ebx,ebx			//
				test    eax,eax         // Check for single-pixel case.
				mov     bl,[esi+eax-1]	// esi: starts as [0+N]-N 
				je      EndLineMask     //
				
				// inner loop
				align 16
				Blit1516ReloopX:				
					test    ebx,ebx				
					jz      DontTexel			      
				DoTexel:					
					mov		ecx,[edi+ebx*4]		// Get this color dword from palette edi
					xor     ebx,ebx				//					

					mov		[edx+eax*2],cx		// Store this color 1516-bit dword to screen.
					mov		bl,[esi+eax]		// get next source texel 

					add     eax,1    			// 0 == end of line...
					jnz     Blit1516ReloopX	
				EndLineMask:					// next line of masked, unscaled tile.

				// Lead-out
					test    ebx,ebx
					jz      DontDoLastTexel
					mov     ecx,[edi+ebx*4]  // load from palette

					mov     [edx+eax*2],cx  // store
					DontDoLastTexel:

				// Reloop for next line ?
					mov     eax,[Spr]SpriteTexSetup.SizeY
					mov     ecx,[Stride]	
					add     esi,[Spr]SpriteTexSetup.USize // Next line in texture map
					add		edx,ecx		// Destination + linestride 
					dec     eax
					mov     [Spr]SpriteTexSetup.SizeY,eax
					jnz     Blit1516ReloopY
					jmp     EndOfBlit
				////////////////////////		
								
				// Masking coroutine
				align 16
				InMask1516Reloop:
					test	ebx,ebx
					jnz		DoTexel

				DontTexel:
					xor		ebx,ebx
					mov		bl,[esi+eax]

					add		eax,1
					jnz		InMask1516Reloop

				jmp EndLineMask		// go to lead-out
				//////////////////////

				EndOfBlit:

			}	
#endif

		}


	}
	else
	{
		// Blit with span clipping. 

		FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

		for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
		{
			BYTE* TexelPtr = &Spr.TexBase[0] + ( Spr.VCo << Spr.UBits );
			Spr.VCo = ( (++Spr.VCo) & Spr.VMask );

			INT UWalk = Spr.UCo;
		
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{ 
				INT SpanX0 = Spr.MinX;  
				INT SpanX1 = Spr.MaxX;

				if ( SpanX1 > Span->End ) SpanX1 = Span->End;

				if ( SpanX0 < Span->Start ) 
				{
					UWalk = (Spr.UCo + (Span->Start - SpanX0)) & Spr.UMask; // prestepping.
					SpanX0 = Span->Start;
				}

				if (SpanX1 > SpanX0) // Any to draw ?
				{
					for ( INT J = SpanX0; J < SpanX1; J++)
					{
						BYTE Texel = TexelPtr[UWalk]; 
						UWalk = ( (++UWalk) & Spr.UMask );
						if (Texel) Spr.Screen.PtrWORD[J] = Spr.Palette.PtrDWORD[Texel];
					}
				}
			} 
		}// per line 			
	}
	unguardSlow;
}








inline void FlashSprite32Normal
(
	 FSpanBuffer* Span,
	 INT GByteStride
)
{
	guardSlow(FlashSprite32Normal);

	FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

	for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
	{
			
		for( FSpan* Span=*Index++; Span; Span=Span->Next )
		{ 
			INT SpanX0 = Spr.MinX;  
			INT SpanX1 = Spr.MaxX;

			INT UIndex = 0;

			if ( SpanX1 > Span->End ) SpanX1 = Span->End;

			if ( SpanX0 < Span->Start ) 
			{
				UIndex = Span->Start - Spr.MinX; // prestepping.
				SpanX0 = Span->Start;
			}

			if (SpanX1 > SpanX0)
			{
				INT VOffset = ( (Spr.VCo >> 18) & Spr.VMask) << Spr.UBits ;

#if !ASM
				INT JSize = SpanX1 - SpanX0;
				for (INT J = 0; J < JSize; J++)
				{
					BYTE Texel = Spr.TexBase[ VOffset + UCoordTable[J+UIndex]];
					Spr.Screen.PtrDWORD[ J+SpanX0 ] = Spr.Palette.PtrDWORD[Texel];
				}
#endif

#if ASM				
				FRainbowPtr PaletteBase		= Spr.Palette;
				FRainbowPtr StartCIndex		= &UCoordTable[UIndex];
				FRainbowPtr StartScreenPix	= &Spr.Screen.PtrDWORD[SpanX0];
				FRainbowPtr EndScreenPix    = &Spr.Screen.PtrDWORD[SpanX1]; // StartScreenPix.PtrDWORD+JSize;
				FRainbowPtr TexVOffset      = &Spr.TexBase[VOffset];

 				__asm {
					// 4- cycle , P/PII friendly, innerloop specific for 32-bit color.
					mov     edi,[PaletteBase]
					mov     edx,[StartCIndex]
					mov     ebx,[StartScreenPix]
					mov		ecx,[EndScreenPix]
					mov     esi,[TexVOffset]

					mov		[SavedEBP],ebp
					mov		[SavedESP],esp

					mov		ebp,ebx
					sub     edx,ebx

					shr		ebp,1
					shr     ecx,1

					xor     ebx,ebx				//
					mov		eax,[edx+ebp*2]		// get 1st U coordinate
					mov		bl,[esi+eax]		// get texel
												//
					align 16					//
					//////////////////////////////
					Reloop32Normal:				//
												//
					mov		eax,[edx+ebp*2+4]	// get next U coordinate.
					add     ebp,2 				// advance video ptr
												//
					mov		esp,[edi+ebx*4]		// get this color dword
					xor     ebx,ebx				//
												//
					mov		[ebp*2-4],esp		// store this color dword
					cmp     ebp,ecx				//
												//
					mov		bl,[esi+eax]		// get next source texel
					jb      Reloop32Normal		//
												//
					//////////////////////////////
					mov		esp,[SavedESP]
					mov		ebp,[SavedEBP]
				}
#endif
			}
		} 
		// Setup new V-coordinate here
		Spr.VCo += Spr.VCoDelta;
	}// per line 
	unguardSlow;
}





inline void FlashSprite32Masked
(
	FSpanBuffer* Span,
	INT GByteStride
)
{
	guardSlow(FlashSprite32Masked);

	FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

	for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
	{
			
		for( FSpan* Span=*Index++; Span; Span=Span->Next )
		{ 
			INT SpanX0 = Spr.MinX;  
			INT SpanX1 = Spr.MaxX;

			INT UIndex = 0;

			if ( SpanX1 > Span->End ) SpanX1 = Span->End;

			if ( SpanX0 < Span->Start ) 
			{
				UIndex = Span->Start - Spr.MinX; // prestepping.
				SpanX0 = Span->Start;
			}

			if (SpanX1 > SpanX0)
			{
				INT VOffset = ( (Spr.VCo >> 18) & Spr.VMask) << Spr.UBits ;						

#if !ASM
				INT JSize = SpanX1 - SpanX0;
				for (INT J = 0; J < JSize; J++)
				{
					BYTE Texel = Spr.TexBase[ VOffset + UCoordTable[J+UIndex]];
					if (Texel) Spr.Screen.PtrDWORD[ J+SpanX0 ] = Spr.Palette.PtrDWORD[Texel];							
				}
#endif


#if ASM				
				FRainbowPtr PaletteBase		= Spr.Palette;
				FRainbowPtr StartCIndex		= &UCoordTable[UIndex];
				FRainbowPtr StartScreenPix	= &Spr.Screen.PtrDWORD[SpanX0];
				FRainbowPtr EndScreenPix    = &Spr.Screen.PtrDWORD[SpanX1]; // StartScreenPix.PtrDWORD+JSize;
				FRainbowPtr TexVOffset      = &Spr.TexBase[VOffset];

 				__asm {
					// 4- cycle , P/PII friendly, innerloop specific for 32-bit color.
					mov     edi,[PaletteBase]
					mov     edx,[StartCIndex]
					mov     ebx,[StartScreenPix]
					mov		ecx,[EndScreenPix]
					mov     esi,[TexVOffset]

					mov		[SavedEBP],ebp
					mov		[SavedESP],esp

					mov		ebp,ebx
					sub     edx,ebx        

					shr		ebp,1
					shr     ecx,1

					xor     ebx,ebx				//
					mov		eax,[edx+ebp*2]		// get 1st U coordinate
					mov		bl,[esi+eax]		// get texel
												//
					jmp Reloop32Masked 

					align 16
					//////////////////////////////
					Reloop32Skipped:
					mov		eax,[edx+ebp*2+4]	// get next U coordinate.
					add     ebp,2 				// advance video ptr
												//
					test    ebx,ebx				//
					jnz     DoTexel             //

					DontTexel:
					xor     ebx,ebx				//
					cmp     ebp,ecx				//
												//
					mov		bl,[esi+eax]		// get next source texel
					jb      Reloop32Skipped		//
					//////////////////////////////
					jmp		EndSpan

					
					align 16					//
					//////////////////////////////
					Reloop32Masked:				//
												//
					mov		eax,[edx+ebp*2+4]	// get next U coordinate.
					add     ebp,2 				// advance video ptr
												//
					test    ebx,ebx				//
					jz      DontTexel           //

					DoTexel:
					mov		esp,[edi+ebx*4]		// get this color dword
					xor     ebx,ebx				//
												//
					mov		[ebp*2-4],esp		// store this color dword
					cmp     ebp,ecx				//
												//
					mov		bl,[esi+eax]		// get next source texel
					jb      Reloop32Masked		//
												//
					//////////////////////////////
					EndSpan:
					mov		esp,[SavedESP]
					mov		ebp,[SavedEBP]
				}
#endif	
			}
		} 
		// Setup new V-coordinate here
		Spr.VCo += Spr.VCoDelta;
	}// per line 
	unguardSlow;
}






inline void FlashSprite32Modulated(FSpanBuffer* Span, INT GByteStride)
{
	guardSlow(FlashSprite32Modulated);

	FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

	for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
	{			
		for( FSpan* Span=*Index++; Span; Span=Span->Next )
		{ 
			INT SpanX0 = Spr.MinX;  
			INT SpanX1 = Spr.MaxX;

			INT UIndex = 0;

			if ( SpanX1 > Span->End ) SpanX1 = Span->End;

			if ( SpanX0 < Span->Start ) 
			{
				UIndex = Span->Start - Spr.MinX; // prestepping.
				SpanX0 = Span->Start;
			}

			if (SpanX1 > SpanX0)
			{
				INT VOffset = ( (Spr.VCo >> 18) & Spr.VMask) << Spr.UBits ;						

#if (1) //!ASM
			
				INT JSize = SpanX1 - SpanX0;
				for (INT J = 0; J < JSize; J++)
				{
					BYTE Texel = Spr.TexBase[ VOffset + UCoordTable[J+UIndex]];
					if (1) //(Texel)
					{
						// Modulation without mmx: uses simple R,R/G,G/B,B lookup... 6 bits screen, 4 bits modulation.
						DWORD ScreenRGB = Spr.Screen.PtrDWORD[J+SpanX0];
						DWORD ColourRGB = Spr.Palette.PtrDWORD[Texel];
						DWORD ModulatedRGB = (ModLut[ ((ColourRGB&0xF0)<<2)      + ((ScreenRGB&0xFC)>>2)	 ] & 0xff)
										   + (ModLut[ ((ColourRGB&0xF000)>>6)    + ((ScreenRGB&0xFC00)>>10)  ] & 0xff00)
										   + (ModLut[ ((ColourRGB&0xF00000)>>14) + ((ScreenRGB&0xFC0000)>>18)] & 0xff0000);
						Spr.Screen.PtrDWORD[ J+SpanX0 ] = ModulatedRGB;
					}
				}
#endif
			}
		} 
		// Setup new V-coordinate here
		Spr.VCo += Spr.VCoDelta;
	}// per line 
	unguardSlow;
}





inline void FlashSprite32TranslucentMMX
(
	FSpanBuffer* Span,
	INT GByteStride
)
{
	guardSlow(FlashSprite32TranslucentMMX);

	FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

	for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
	{
			
		for( FSpan* Span=*Index++; Span; Span=Span->Next )
		{ 
			INT SpanX0 = Spr.MinX;  
			INT SpanX1 = Spr.MaxX;

			INT UIndex = 0;

			if ( SpanX1 > Span->End ) SpanX1 = Span->End;

			if ( SpanX0 < Span->Start ) 
			{
				UIndex = Span->Start - Spr.MinX; // prestepping.
				SpanX0 = Span->Start;
			}

			if (SpanX1 > SpanX0)
			{
				INT VOffset = ( (Spr.VCo >> 18) & Spr.VMask) << Spr.UBits ;						


#if !ASM
				INT JSize = SpanX1 - SpanX0;
				for (INT J = 0; J < JSize; J++)
				{					

					BYTE Texel = Spr.TexBase[ VOffset + UCoordTable[J+UIndex]];
					if (Texel)
					{
						DWORD TestSat = Spr.Palette.PtrDWORD[Texel] + ( 0xfefeff & Spr.Screen.PtrDWORD[J+SpanX0]);

						if ( DWORD TestMask = TestSat & 0x1010100 )
						{
							TestSat |= ( TestMask - (TestMask >> 8)); //saturate
						}
						Spr.Screen.PtrDWORD[ J+SpanX0 ] = TestSat; // (_WORD) Spr.Palette.PtrDWORD[Texel];							
					}
				}

#endif

#if ASM				
				FRainbowPtr PaletteBase		= Spr.Palette;
				FRainbowPtr StartCIndex		= &UCoordTable[UIndex];
				FRainbowPtr StartScreenPix	= &Spr.Screen.PtrDWORD[SpanX0];
				FRainbowPtr EndScreenPix    = &Spr.Screen.PtrDWORD[SpanX1]; // StartScreenPix.PtrDWORD+JSize;
				FRainbowPtr TexVOffset      = &Spr.TexBase[VOffset];

 				__asm {
					// 4- cycle , P/PII friendly, innerloop specific for 32-bit color.
					mov     edi,[PaletteBase]
					mov     edx,[StartCIndex]
					mov     ebx,[StartScreenPix]
					mov		ecx,[EndScreenPix]
					mov     esi,[TexVOffset]

					mov		[SavedEBP],ebp

					mov		ebp,ebx
					sub     edx,ebx        

					shr		ebp,1
					shr     ecx,1

					xor     ebx,ebx				//
					mov		eax,[edx+ebp*2]		// get 1st U coordinate
					mov		bl,[esi+eax]		// get texel
												//
					jmp Reloop32Masked 

					align 16
					//////////////////////////////
					Reloop32Skipped:
					mov		eax,[edx+ebp*2+4]	// get next U coordinate.
					add     ebp,2 				// advance video ptr
												//
					test    ebx,ebx				//
					jnz     DoTexel             //

					DontTexel:
					xor     ebx,ebx				//
					cmp     ebp,ecx				//
												//
					mov		bl,[esi+eax]		// get next source texel
					jb      Reloop32Skipped		//
					//////////////////////////////
					jmp		EndSpan

					
					align 16					//
					//////////////////////////////
					Reloop32Masked:				//
												//
					mov		eax,[edx+ebp*2+4]	// get next U coordinate.
					add     ebp,2 				// advance video ptr
												//
					test    ebx,ebx				//
					jz      DontTexel           //

					DoTexel:
					movd	mm1,[edi+ebx*4]		// get this color dword
					movd    mm2,[ebp*2-4]       // get screen dword (Expensive if from video memory)

					xor     ebx,ebx				//
					paddusb mm1,mm2
												//
					mov		bl,[esi+eax]		// get next source texel
					cmp     ebp,ecx				//
												//
					movd	[ebp*2-4],mm1		// store this color dword
					jb      Reloop32Masked		//
												//
					//////////////////////////////
					EndSpan:
					mov		ebp,[SavedEBP]
				}
#endif	
			}
		} 
		// Setup new V-coordinate here
		Spr.VCo += Spr.VCoDelta;
	}// per line 

#if ASM
	{
		__asm emms;
	}
#endif

	unguardSlow;
}





inline void FlashSprite32ModulatedMMX
(
	 FSpanBuffer* Span,
	INT GByteStride
)
{
	guardSlow(FlashSprite32ModulatedMMX);

	FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

	for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
	{
			
		for( FSpan* Span=*Index++; Span; Span=Span->Next )
		{ 
			INT SpanX0 = Spr.MinX;  
			INT SpanX1 = Spr.MaxX;

			INT UIndex = 0;

			if ( SpanX1 > Span->End ) SpanX1 = Span->End;

			if ( SpanX0 < Span->Start ) 
			{
				UIndex = Span->Start - Spr.MinX; // prestepping.
				SpanX0 = Span->Start;
			}

			if (SpanX1 > SpanX0)
			{
				INT VOffset = ( (Spr.VCo >> 18) & Spr.VMask) << Spr.UBits ;						

#if !ASM
				INT JSize = SpanX1 - SpanX0;
				for (INT J = 0; J < JSize; J++)
				{
					BYTE Texel = Spr.TexBase[ VOffset + UCoordTable[J+UIndex]];
					if (1) //(Texel)
					{
						// Modulation without mmx: uses simple R,R/G,G/B,B lookup... 6 bits screen, 4 bits modulation.
						DWORD ScreenRGB = Spr.Screen.PtrDWORD[J+SpanX0];
						DWORD ColourRGB = Spr.Palette.PtrDWORD[Texel];
						DWORD ModulatedRGB = (ModLut[ ((ColourRGB&0xF0)<<2)      + ((ScreenRGB&0xFC)>>2)	 ] & 0xff)
										   + (ModLut[ ((ColourRGB&0xF000)>>6)    + ((ScreenRGB&0xFC00)>>10)  ] & 0xff00)
										   + (ModLut[ ((ColourRGB&0xF00000)>>14) + ((ScreenRGB&0xFC0000)>>18)] & 0xff0000);
						Spr.Screen.PtrDWORD[ J+SpanX0 ] = ModulatedRGB;
					}
				}
#endif

#if ASM				
				FRainbowPtr PaletteBase		= Spr.Palette;
				FRainbowPtr StartCIndex		= &UCoordTable[UIndex];
				FRainbowPtr StartScreenPix	= &Spr.Screen.PtrDWORD[SpanX0];
				FRainbowPtr EndScreenPix    = &Spr.Screen.PtrDWORD[SpanX1]; // StartScreenPix.PtrDWORD+JSize;
				FRainbowPtr TexVOffset      = &Spr.TexBase[VOffset];

 				__asm {
					// 4- cycle , P/PII friendly, innerloop specific for 32-bit color.
					mov     edi,[PaletteBase]
					mov     edx,[StartCIndex]
					mov     ebx,[StartScreenPix]
					mov		ecx,[EndScreenPix]
					mov     esi,[TexVOffset]

					mov		[SavedEBP],ebp

					mov		ebp,ebx
					sub     edx,ebx        

					shr		ebp,1
					shr     ecx,1

					xor     ebx,ebx				//
					mov		eax,[edx+ebp*2]		// get 1st U coordinate
					mov		bl,[esi+eax]		// get texel
												//
					psubd       mm1,mm1         // prepare for unpacking

					jmp Reloop32Masked 

					align 16
					//////////////////////////////
					Reloop32Skipped:
					mov		eax,[edx+ebp*2+4]	// get next U coordinate.
					add     ebp,2 				// advance video ptr
												//
					test    ebx,ebx				//
					jnz     DoTexel             //

					//DontTexel:
					xor     ebx,ebx				//
					cmp     ebp,ecx				//
												//
					mov		bl,[esi+eax]		// get next source texel
					jb      Reloop32Skipped		//
					//////////////////////////////
					jmp		EndSpan

					
					align 16					
					//////////////////////////////
					Reloop32Masked:				//
												//
					mov		eax,[edx+ebp*2+4]	// Get next U coordinate.
					add     ebp,2 				// Advance video ptr
												//
					//test    ebx,ebx 			// Neutral modulation level == palette color 0...
					//jz      DontTexel           //

					DoTexel:
					punpcklbw	mm1,[edi+ebx*4]	// Get this color qword. 
					psubd		mm2,mm2			//


					punpcklbw	mm2,[ebp*2-4]   // Screen dword (very expensive if video RAM.)

#if MODINDICATOR
					pcmpeqd     mm2,mm2
#endif
					psrlw       mm1,1			//
					psrlw       mm2,1			//	
					pmulhw      mm2,mm1			//		
					xor			ebx,ebx			//
					psrlw       mm2,5		    //
					mov			bl,[esi+eax]	// Get next source texel.
					packuswb    mm2,mm2			//
					cmp			ebp,ecx			//
					movd		[ebp*2-4],mm2	// Store this color dword.
					psubd		mm1,mm1			//
					jb			Reloop32Masked	//
												//
					//////////////////////////////
					EndSpan:
					mov		ebp,[SavedEBP]
				}
#endif	
			}
		} 
		// Setup new V-coordinate here
		Spr.VCo += Spr.VCoDelta;
	}// per line 

#if ASM
	{
		__asm emms;
	}
#endif
	unguardSlow;
}


/*
	// Write last pixel
			test        eax,eax
			movq		mm0, qword ptr [ebp+eax*8] //  // edx = colors palette base  eax= texelcolor index.
	jz          EndRender4 
			psubw       mm1, mm1
			punpcklbw   mm1, [edi+esi*4]
			psrlw       mm1,1	 // 
			pmulhw      mm0, mm1 //
			psrlw       mm0,5	 //
			packuswb    mm0, mm0
			movd		[edi+esi*4], mm0  //  Store to screen. 
	jmp		EndRender4

*/


inline void FlashSprite32TranslucentPentium
(
    FSpanBuffer* Span,
	INT GByteStride
)
{
	guardSlow(FlashSprite32TranslucentPentium);


	FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

	for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
	{
			
		for( FSpan* Span=*Index++; Span; Span=Span->Next )
		{ 
			INT SpanX0 = Spr.MinX;  
			INT SpanX1 = Spr.MaxX;

			INT UIndex = 0;

			if ( SpanX1 > Span->End ) SpanX1 = Span->End;

			if ( SpanX0 < Span->Start ) 
			{
				UIndex = Span->Start - Spr.MinX; // prestepping.
				SpanX0 = Span->Start;
			}

			if (SpanX1 > SpanX0)
			{
				INT VOffset = ( (Spr.VCo >> 18) & Spr.VMask) << Spr.UBits ;						

#if !ASM
				INT JSize = SpanX1 - SpanX0;
				for (INT J = 0; J < JSize; J++)
				{					

					BYTE Texel = Spr.TexBase[ VOffset + UCoordTable[J+UIndex]];
					if (Texel)
					{
						DWORD TestSat = Spr.Palette.PtrDWORD[Texel] + ( 0xfefeff & Spr.Screen.PtrDWORD[J+SpanX0]);

						if ( DWORD TestMask = TestSat & 0x1010100 )
						{
							TestSat |= ( TestMask - (TestMask >> 8)); //saturate
						}
						Spr.Screen.PtrDWORD[ J+SpanX0 ] = TestSat; // (_WORD) Spr.Palette.PtrDWORD[Texel];							
					}
				}
#endif


#if ASM				
				FRainbowPtr PaletteBase		= Spr.Palette;
				FRainbowPtr StartCIndex		= &UCoordTable[UIndex];
				FRainbowPtr StartScreenPix	= &Spr.Screen.PtrDWORD[SpanX0];
				FRainbowPtr EndScreenPix    = &Spr.Screen.PtrDWORD[SpanX1]; // StartScreenPix.PtrDWORD+JSize;
				FRainbowPtr TexVOffset      = &Spr.TexBase[VOffset];

 				__asm {
					// 4- cycle , P/PII friendly, innerloop specific for 32-bit color.
					mov     edi,[PaletteBase]
					mov     edx,[StartCIndex]
					mov     ebx,[StartScreenPix]
					mov		ecx,[EndScreenPix]
					mov     esi,[TexVOffset]

					mov		[SavedEBP],ebp
					mov		[SavedESP],esp

					mov		ebp,ebx
					sub     edx,ebx        

					shr		ebp,1
					shr     ecx,1

					xor     ebx,ebx				//
					mov		eax,[edx+ebp*2]		// get 1st U coordinate
					mov		bl,[esi+eax]		// get texel
												//
					jmp Reloop32Masked 

					align 16
					//////////////////////////////
					Reloop32Skipped:
					mov		eax,[edx+ebp*2+4]	// get next U coordinate.
					add     ebp,2 				// advance video ptr
												//
					test    ebx,ebx				//
					jnz     DoTexel             //

					DontTexel:
					xor     ebx,ebx				//
					cmp     ebp,ecx				//
												//
					mov		bl,[esi+eax]		// get next source texel
					jb      Reloop32Skipped		//
					//////////////////////////////
					jmp		EndSpan

					align 16
					//////////////////////////////
					Overflow32Normal:		 
					mov     ebx,esp
					and     esp,0x1010100
					mov     [SavedEAX],eax
					mov     eax,esp
					shr     esp,8
					sub     eax,esp
					or      ebx,eax
					mov     eax,[SavedEAX]
					mov     [ebp*2-4],ebx
					xor     ebx,ebx
					cmp     ebp,ecx
					mov     bl,[esi+eax]
					jb      Reloop32Masked
					//////////////////////////////
					jmp		EndSpan
				
					align 16					
					//////////////////////////////
					Reloop32Masked:				//
												//
					mov		eax,[edx+ebp*2+4]	// get next U coordinate.
					add     ebp,2 				// advance video ptr
												//
					test    ebx,ebx				//
					jz      DontTexel           //
												//
					DoTexel:					//
					mov		esp,[edi+ebx*4]		// get this color dword
					mov     ebx,[ebp*2-4]		//
												//
					and     ebx,0x0fefeff       //
												//
					add     esp,ebx				//
					xor     ebx,ebx				//
												//
					test    esp,0x1010100		//
					jnz		Overflow32Normal    //
												//
					mov		[ebp*2-4],esp		// store this color dword
					cmp     ebp,ecx				//
												//
					mov		bl,[esi+eax]		// get next source texel
					jb      Reloop32Masked		//
					//////////////////////////////
					EndSpan:
					mov		esp,[SavedESP]
					mov		ebp,[SavedEBP]
				}
#endif	
			}
		} 
		// Setup new V-coordinate here
		Spr.VCo += Spr.VCoDelta;
	}// per line 

	unguardSlow;
}





inline void FlashSprite1516Normal
(
	FSpanBuffer* Span,
	INT GByteStride
)
{
	guardSlow(FlashSprite1516Normal);

	FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

	for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
	{
			
		for( FSpan* Span=*Index++; Span; Span=Span->Next )
		{ 
			INT SpanX0 = Spr.MinX;  
			INT SpanX1 = Spr.MaxX;

			INT UIndex = 0;

			if ( SpanX1 > Span->End ) SpanX1 = Span->End;

			if ( SpanX0 < Span->Start ) 
			{
				UIndex = Span->Start - Spr.MinX; // prestepping.
				SpanX0 = Span->Start;
			}

			if (SpanX1 > SpanX0)
			{
				INT VOffset = ( (Spr.VCo >> 18) & Spr.VMask) << Spr.UBits ;						

#if !ASM
				INT JSize = SpanX1 - SpanX0;
				for (INT J = 0; J < JSize; J++)
				{
					BYTE Texel = Spr.TexBase[ VOffset + UCoordTable[J+UIndex]];
					Spr.Screen.PtrWORD[ J+SpanX0 ] = (_WORD) Spr.Palette.PtrDWORD[Texel];							
				}
#endif

#if ASM				
				FRainbowPtr PaletteBase		= Spr.Palette;
				FRainbowPtr StartCIndex		= &UCoordTable[UIndex];
				FRainbowPtr StartScreenPix	= &Spr.Screen.PtrWORD[SpanX0];
				FRainbowPtr EndScreenPix    = &Spr.Screen.PtrWORD[SpanX1]; // StartScreenPix.PtrDWORD+JSize;
				FRainbowPtr TexVOffset      = &Spr.TexBase[VOffset];

 				__asm {
					// 4- cycle , P/PII friendly, innerloop specific for 32-bit color.
					mov     edi,[PaletteBase]
					mov     edx,[StartCIndex]
					mov     ebx,[StartScreenPix]
					mov		ecx,[EndScreenPix]
					mov     esi,[TexVOffset]

					mov		[SavedEBP],ebp
					mov		[SavedESP],esp

					mov		ebp,ebx

					sub     edx,ebx        
					sub     edx,ebx

					//shr	  ebp,1
					//shr     ecx,1

					xor     ebx,ebx				//
					mov		eax,[edx+ebp*2]		// get 1st U coordinate
					mov		bl,[esi+eax]		// get texel
												//
					align 16					//
					//////////////////////////////
					Reloop1516Normal:			//
												//
					mov		eax,[edx+ebp*2+4]	// get next U coordinate.
					add     ebp,2 				// advance video ptr
												//
					mov		esp,[edi+ebx*4]	// get this color dword
					xor     ebx,ebx				//
												//
					mov		[ebp-2],sp	        // store this color _word_
					cmp     ebp,ecx				//
												//
					mov		bl,[esi+eax]		// get next source texel
					jb      Reloop1516Normal	//
												//
					//////////////////////////////
					mov		esp,[SavedESP]
					mov		ebp,[SavedEBP]
				}
#endif
			}
		} 
		// Setup new V-coordinate here
		Spr.VCo += Spr.VCoDelta;
	}// per line 
	unguardSlow;
}



inline void FlashSprite1516Masked
(
	 FSpanBuffer* Span,
	INT GByteStride
)
{
	guardSlow(FlashSprite1516Masked);

	FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

	for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
	{
			
		for( FSpan* Span=*Index++; Span; Span=Span->Next )
		{ 
			INT SpanX0 = Spr.MinX;  
			INT SpanX1 = Spr.MaxX;

			INT UIndex = 0;

			if ( SpanX1 > Span->End ) SpanX1 = Span->End;

			if ( SpanX0 < Span->Start ) 
			{
				UIndex = Span->Start - Spr.MinX; // prestepping.
				SpanX0 = Span->Start;
			}

			if (SpanX1 > SpanX0)
			{
				INT VOffset = ( (Spr.VCo >> 18) & Spr.VMask) << Spr.UBits ;						

#if !ASM
				INT JSize = SpanX1 - SpanX0;
				for (INT J = 0; J < JSize; J++)
				{
					BYTE Texel = Spr.TexBase[ VOffset + UCoordTable[J+UIndex]];
					if (Texel) Spr.Screen.PtrWORD[ J+SpanX0 ] = (_WORD) Spr.Palette.PtrDWORD[Texel];							
				}
#endif


#if ASM				
				FRainbowPtr PaletteBase		= Spr.Palette;
				FRainbowPtr StartCIndex		= &UCoordTable[UIndex];
				FRainbowPtr StartScreenPix	= &Spr.Screen.PtrWORD[SpanX0];
				FRainbowPtr EndScreenPix    = &Spr.Screen.PtrWORD[SpanX1]; // StartScreenPix.PtrDWORD+JSize;
				FRainbowPtr TexVOffset      = &Spr.TexBase[VOffset];

 				__asm {
					mov     edi,[PaletteBase]
					mov     edx,[StartCIndex]
					mov     ebx,[StartScreenPix]
					mov		ecx,[EndScreenPix]
					mov     esi,[TexVOffset]

					mov		[SavedEBP],ebp
					mov		[SavedESP],esp

					mov		ebp,ebx
					sub     edx,ebx        
					sub     edx,ebx

					xor     ebx,ebx				//
					mov		eax,[edx+ebp*2]		// get 1st U coordinate
					mov		bl,[esi+eax]		// get texel
												//
					jmp Reloop1516Masked 

					align 16
					//////////////////////////////
					Reloop1516Skipped:			//
					mov		eax,[edx+ebp*2+4]	// get next U coordinate.
					add     ebp,2 				// advance video ptr
												//
					test    ebx,ebx				//
					jnz     DoTexel             //

					DontTexel:
					xor     ebx,ebx				//
					cmp     ebp,ecx				//
												//
					mov		bl,[esi+eax]		// get next source texel
					jb      Reloop1516Skipped   //
					//////////////////////////////
					jmp		EndSpan

					
					align 16					
					//////////////////////////////
					Reloop1516Masked:				//
												//
					mov		eax,[edx+ebp*2+4]	// get next U coordinate.
					add     ebp,2 				// advance video ptr
												//
					test    ebx,ebx				//
					jz      DontTexel           //

					DoTexel:
					mov		esp,[edi+ebx*4]		// get this color dword
					xor     ebx,ebx				//
												//
					mov		[ebp-2],sp		// store this color dword
					cmp     ebp,ecx				//
												//
					mov		bl,[esi+eax]		// get next source texel
					jb      Reloop1516Masked	//
												//
					//////////////////////////////
					EndSpan:
					mov		esp,[SavedESP]
					mov		ebp,[SavedEBP]
				}
#endif	
			}
		} 
		// Setup new V-coordinate here
		Spr.VCo += Spr.VCoDelta;
	}// per line 
	unguardSlow;
}



inline void FlashSprite15Translucent
(
	FSpanBuffer* Span,
	INT GByteStride
)
{
	guardSlow(FlashSprite15Translucent);

	FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

	for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
	{
			
		for( FSpan* Span=*Index++; Span; Span=Span->Next )
		{ 
			INT SpanX0 = Spr.MinX;  
			INT SpanX1 = Spr.MaxX;

			INT UIndex = 0;

			if ( SpanX1 > Span->End ) SpanX1 = Span->End;

			if ( SpanX0 < Span->Start ) 
			{
				UIndex = Span->Start - Spr.MinX; // prestepping.
				SpanX0 = Span->Start;
			}

			if (SpanX1 > SpanX0)
			{
				INT VOffset = ( (Spr.VCo >> 18) & Spr.VMask) << Spr.UBits ;						

#if !ASM
				INT JSize = SpanX1 - SpanX0;
				for (INT J = 0; J < JSize; J++)
				{
					BYTE Texel = Spr.TexBase[ VOffset + UCoordTable[J+UIndex]];

					if (Texel)
					{
						DWORD TestSat = Spr.Palette.PtrDWORD[Texel] + ((0xFFFFFFFF - 0x08420) & Spr.Screen.PtrWORD[J+SpanX0]);
						DWORD TestMask = TestSat & 0x08420;
						if (TestMask)
						{
							TestSat |= (TestMask - (TestMask >> 5)); //saturate
						}
						Spr.Screen.PtrWORD[ J+SpanX0 ] = TestSat; // (_WORD) Spr.Palette.PtrDWORD[Texel];							
					}
				}
#endif

#if ASM
				FRainbowPtr PaletteBase		= Spr.Palette;
				FRainbowPtr StartCIndex		= &UCoordTable[UIndex];
				FRainbowPtr StartScreenPix	= &Spr.Screen.PtrWORD[SpanX0];
				FRainbowPtr EndScreenPix    = &Spr.Screen.PtrWORD[SpanX1]; // StartScreenPix.PtrDWORD+JSize;
				FRainbowPtr TexVOffset      = &Spr.TexBase[VOffset];

 				__asm {
					// 4- cycle , P/PII friendly, innerloop specific for 32-bit color.
					mov     edi,[PaletteBase]
					mov     edx,[StartCIndex]
					mov     ebx,[StartScreenPix]
					mov		ecx,[EndScreenPix]
					mov     esi,[TexVOffset]

					mov		[SavedEBP],ebp
					mov		[SavedESP],esp

					mov		ebp,ebx
					sub     edx,ebx        
					sub     edx,ebx

					xor     ebx,ebx				//
					mov		eax,[edx+ebp*2]		// get 1st U coordinate
					mov		bl,[esi+eax]		// get texel
												//
					jmp Reloop15Masked 

					align 16
					//////////////////////////////
					Reloop15Skipped:			//
					mov		eax,[edx+ebp*2+4]	// get next U coordinate.
					add     ebp,2 				// advance video ptr
												//
					test    ebx,ebx				//
					jnz     DoTexel             //
												//
					DontTexel:					//
					xor     ebx,ebx				//
					cmp     ebp,ecx				//
												//
					mov		bl,[esi+eax]		// get next source texel
					jb      Reloop15Skipped     //
					//////////////////////////////
					jmp		EndSpan

					align 16
					//////////////////////////////
					Overflow15Normal:		 
					mov     ebx,esp
					and     esp,0x8420
					mov     [SavedEAX],eax
					mov     eax,esp
					shr     esp,5
					sub     eax,esp
					or      ebx,eax
					mov     eax,[SavedEAX]
					mov     [ebp-2],bx        // 15-bit store
					xor     ebx,ebx
					cmp     ebp,ecx
					mov     bl,[esi+eax]
					jb      Reloop15Masked
					///////////////////////////////
					jmp		EndSpan

					
					align 16					
					///////////////////////////////
					Reloop15Masked:				 //
												 //
					mov		eax,[edx+ebp*2+4]	 // get next U coordinate.
					add     ebp,2 				 // advance video ptr
												 //
					test    ebx,ebx				 //
					jz      DontTexel            //

					DoTexel:
					
					mov		esp,[edi+ebx*4]		 // get this color dword
					mov      bx,[ebp-2]			 // get screen_word
                                                 //
					and     ebx,(0xffff - 0x8420)//
					add     esp,ebx				 //
					xor     ebx,ebx				 //
												 //
					test    esp,0x8420  		 //
					jnz		Overflow15Normal     //
												 //
					mov		[ebp-2],sp			 // store this color dword
					cmp     ebp,ecx				 //
												 //
					mov		bl,[esi+eax]		 // get next source texel
					jb      Reloop15Masked	     //
												 //
					///////////////////////////////
					EndSpan:
					mov		esp,[SavedESP]
					mov		ebp,[SavedEBP]
				}
#endif	
			}
		} 
		// Setup new V-coordinate here
		Spr.VCo += Spr.VCoDelta;
	}// per line 
	unguardSlow;
}



inline void FlashSprite15Modulated
(
	FSpanBuffer* Span,
	INT GByteStride
)
{
	guardSlow(FlashSprite15Modulated);

	FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

	for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
	{
			
		for( FSpan* Span=*Index++; Span; Span=Span->Next )
		{ 
			INT SpanX0 = Spr.MinX;  
			INT SpanX1 = Spr.MaxX;

			INT UIndex = 0;

			if ( SpanX1 > Span->End ) SpanX1 = Span->End;

			if ( SpanX0 < Span->Start ) 
			{
				UIndex = Span->Start - Spr.MinX; // prestepping.
				SpanX0 = Span->Start;
			}

			if (SpanX1 > SpanX0)
			{
				INT VOffset = ( (Spr.VCo >> 18) & Spr.VMask) << Spr.UBits ;						
#if (1)
				INT JSize = SpanX1 - SpanX0;
				for (INT J = 0; J < JSize; J++)
				{

					BYTE Texel = Spr.TexBase[ VOffset + UCoordTable[J+UIndex]];
					if (1) //(Texel)
					{
						// Modulation without mmx: uses simple R,R/G,G/B,B lookup... 5 bits screen, 5 bits modulation.
						DWORD ScreenRGB = Spr.Screen.PtrWORD[J+SpanX0];
						DWORD ColourRGB = Spr.Palette.PtrDWORD[Texel];
						DWORD ModulatedRGB = (ModLut[ ((ColourRGB&0xF8)<<2)      + ((ScreenRGB&0x001F)>>0) ] & 0x001F)
										   + (ModLut[ ((ColourRGB&0xF800)>>6)    + ((ScreenRGB&0x03E0)>>5) ] & 0x03E0)
										   + (ModLut[ ((ColourRGB&0xF80000)>>14) + ((ScreenRGB&0x7C00)>>10)] & 0x7C00);
						Spr.Screen.PtrWORD[ J+SpanX0 ] = ModulatedRGB;
					}
				}
#endif
			}
		} 
		// Setup new V-coordinate here
		Spr.VCo += Spr.VCoDelta;
	}// per line 
	unguardSlow;
}	




inline void FlashSprite16Translucent
(
	FSpanBuffer* Span,
	INT GByteStride
)
{
	guardSlow(FlashSprite16Translucent);

	FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

	for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
	{
			
		for( FSpan* Span=*Index++; Span; Span=Span->Next )
		{ 
			INT SpanX0 = Spr.MinX;  
			INT SpanX1 = Spr.MaxX;

			INT UIndex = 0;

			if ( SpanX1 > Span->End ) SpanX1 = Span->End;

			if ( SpanX0 < Span->Start ) 
			{
				UIndex = Span->Start - Spr.MinX; // prestepping.
				SpanX0 = Span->Start;
			}

			if (SpanX1 > SpanX0)
			{
				INT VOffset = ( (Spr.VCo >> 18) & Spr.VMask) << Spr.UBits ;						

#if !ASM
				INT JSize = SpanX1 - SpanX0;
				for (INT J = 0; J < JSize; J++)
				{
					BYTE Texel = Spr.TexBase[ VOffset + UCoordTable[J+UIndex]];

					if (Texel)
					{
						DWORD TestSat = Spr.Palette.PtrDWORD[Texel] + ((0xFFFFFFFF - 0x10820) & Spr.Screen.PtrWORD[J+SpanX0]);
						DWORD TestMask = TestSat & 0x10820;
						if (TestMask)
						{
							TestSat |= (TestMask - (TestMask >> 5)); //saturate
						}
						Spr.Screen.PtrWORD[ J+SpanX0 ] = TestSat; // (_WORD) Spr.Palette.PtrDWORD[Texel];
					}
				}
#endif

#if ASM
				FRainbowPtr PaletteBase		= Spr.Palette;
				FRainbowPtr StartCIndex		= &UCoordTable[UIndex];
				FRainbowPtr StartScreenPix	= &Spr.Screen.PtrWORD[SpanX0];
				FRainbowPtr EndScreenPix    = &Spr.Screen.PtrWORD[SpanX1]; // StartScreenPix.PtrDWORD+JSize;
				FRainbowPtr TexVOffset      = &Spr.TexBase[VOffset];

 				__asm {
					// 4- cycle , P/PII friendly, innerloop specific for 32-bit color.
					mov     edi,[PaletteBase]
					mov     edx,[StartCIndex]
					mov     ebx,[StartScreenPix]
					mov		ecx,[EndScreenPix]
					mov     esi,[TexVOffset]

					mov		[SavedEBP],ebp
					mov		[SavedESP],esp

					mov		ebp,ebx
					sub     edx,ebx        
					sub     edx,ebx

					xor     ebx,ebx				//
					mov		eax,[edx+ebp*2]		// get 1st U coordinate
					mov		bl,[esi+eax]		// get texel
												//
					jmp Reloop16Masked 

					align 16
					//////////////////////////////
					Reloop16Skipped:			//
					mov		eax,[edx+ebp*2+4]	// get next U coordinate.
					add     ebp,2 				// advance video ptr
												//
					test    ebx,ebx				//
					jnz     DoTexel             //
												//
					DontTexel:					//
					xor     ebx,ebx				//
					cmp     ebp,ecx				//
												//
					mov		bl,[esi+eax]		// get next source texel
					jb      Reloop16Skipped     //
					//////////////////////////////
					jmp		EndSpan

					align 16
					//////////////////////////////
					Overflow16Normal:		 
					mov     ebx,esp
					and     esp,0x10820
					mov     [SavedEAX],eax
					mov     eax,esp
					shr     esp,5
					sub     eax,esp
					or      ebx,eax
					mov     eax,[SavedEAX]
					mov     [ebp-2],bx        // 15-bit store
					xor     ebx,ebx
					cmp     ebp,ecx
					mov     bl,[esi+eax]
					jb      Reloop16Masked
					//////////////////////////////
					jmp		EndSpan

					
					align 16					
					//////////////////////////////
					Reloop16Masked:				//
												//
					mov		eax,[edx+ebp*2+4]	// get next U coordinate.
					add     ebp,2 				// advance video ptr
												//
					test    ebx,ebx				//
					jz      DontTexel           //

					DoTexel:					//
					
					mov		esp,[edi+ebx*4]		// get this color dword
					mov      bx,[ebp-2]			// get screen_word

					and     ebx,(0xfffff - 0x10820)
					add     esp,ebx				 //
					xor     ebx,ebx				 //
												 //
					test    esp,0x10820  		 //
					jnz		Overflow16Normal     //
												 //
					mov		[ebp-2],sp			 // store this color dword
					cmp     ebp,ecx				 //
												 //
					mov		bl,[esi+eax]		 // get next source texel
					jb      Reloop16Masked	     //
												 //
					///////////////////////////////
					EndSpan:
					mov		esp,[SavedESP]
					mov		ebp,[SavedEBP]
				}
#endif	
			}
		} 
		// Setup new V-coordinate here
		Spr.VCo += Spr.VCoDelta;
	}// per line 
	unguardSlow;
}


inline void FlashSprite16Modulated
(
	FSpanBuffer* Span,
	INT GByteStride
)
{
	guardSlow(FlashSprite16Modulated);

	FSpan** Index   = Span->Index + Spr.MinY - Span->StartY;

	for( INT L=Spr.MinY; L<Spr.MaxY; L++, Spr.Screen.PtrBYTE+= GByteStride )
	{
			
		for( FSpan* Span=*Index++; Span; Span=Span->Next )
		{ 
			INT SpanX0 = Spr.MinX;  
			INT SpanX1 = Spr.MaxX;

			INT UIndex = 0;

			if ( SpanX1 > Span->End ) SpanX1 = Span->End;

			if ( SpanX0 < Span->Start ) 
			{
				UIndex = Span->Start - Spr.MinX; // prestepping.
				SpanX0 = Span->Start;
			}

			if (SpanX1 > SpanX0)
			{
				INT VOffset = ( (Spr.VCo >> 18) & Spr.VMask) << Spr.UBits ;						
#if (1)
				INT JSize = SpanX1 - SpanX0;
				for (INT J = 0; J < JSize; J++)
				{

					BYTE Texel = Spr.TexBase[ VOffset + UCoordTable[J+UIndex]];
					if (1) //(Texel)
					{
						// Modulation without mmx: uses simple R,R/G,G/B,B lookup... 5 bits screen, 5 bits modulation.
						DWORD ScreenRGB = Spr.Screen.PtrWORD[J+SpanX0];
						DWORD ColourRGB = Spr.Palette.PtrDWORD[Texel];
						DWORD ModulatedRGB = (ModLut[ ((ColourRGB&0xF8)<<2)      + ((ScreenRGB&0x001F)>>0) ] & 0x001F)
										   + (ModLut[ ((ColourRGB&0xF800)>>6)    + ((ScreenRGB&0x07C0)>>6) ] & 0x07E0)
										   + (ModLut[ ((ColourRGB&0xF80000)>>14) + ((ScreenRGB&0xF800)>>11)] & 0xF800);
						//ModulatedRGB = 0xffff;
						Spr.Screen.PtrWORD[ J+SpanX0 ] = (_WORD) ModulatedRGB;
					}
				}
#endif
			}
		} 
		// Setup new V-coordinate here
		Spr.VCo += Spr.VCoDelta;
	}// per line 
	unguardSlow;
}



void USoftwareRenderDevice::DrawTile( FSceneNode* Frame, FTextureInfo& Texture, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, FSpanBuffer* Span, FLOAT Z, FPlane Light, FPlane Fog, DWORD PolyFlags )
{
	guardSlow(USoftwareRenderDevice::DrawTile);


	// if (Z < 2.0) then do NOT do Screenflashing; means sprites are above the screen.
	// If PolyFlags & PF_RenderHint, this will be a non-scaled sprite (ie font character, hud icon).

    // GIsMMX = 0; 

	// PolyFlags = PF_Modulated;
	
	// For testing: equivalent of DrawTile by calling DrawGouraudPoly.

	/*	
	// Build poly.
	FLOAT RZ = 1.0/Z;
	FTransTexture P[4];
	P[0].ScreenX = X   +0.5; P[0].ScreenY = Y+YL+0.5; P[0].U = U;    P[0].V = V+VL; P[0].Light = Light; P[0].Fog = Fog; P[0].Point.Z=Z; P[0].RZ=RZ;
	P[1].ScreenX = X   +0.5; P[1].ScreenY = Y   +0.5; P[1].U = U;    P[1].V = V;    P[1].Light = Light; P[1].Fog = Fog; P[1].Point.Z=Z; P[1].RZ=RZ;
	P[2].ScreenX = X+XL+0.5; P[2].ScreenY = Y   +0.5; P[2].U = U+UL; P[2].V = V;    P[2].Light = Light; P[2].Fog = Fog; P[2].Point.Z=Z; P[2].RZ=RZ;
	P[3].ScreenX = X+XL+0.5; P[3].ScreenY = Y+YL+0.5; P[3].U = U+UL; P[3].V = V+VL; P[3].Light = Light; P[3].Fog = Fog; P[3].Point.Z=Z; P[3].RZ=RZ;
	// Draw that cutie.
	DrawGouraudPolygon( Frame, Texture, P, 4, PolyFlags, Span );
	return;
	*/
	
	//
	// Fast tile drawer; maintain and rebuild ONE cached palette per unique texture palette,
	// in a ready-to-write destination 15/16/32 bit color depth format.
	//
	// - Causes lots of recomputation when screenflashes happen, and when variable-transparency 
	//   projectiles go off -> but, unless textures are all < 16x16, palette conversion gives back plenty of speed.
	//   Worst case: lots of small distant projectiles all with variable transparency.
	//
	//   >> One concern: UnCache speed !! if needed, use custom caching routines, or even adapt stuff -
	//   -> keep stuff locked in cache, possibly ?

	// Stats. Static INT RemakeCalled = 0;
	// if (!(FrameLocksCounter & 63)) debugf(NAME_Log," Palette remake: %i  Framelocks: %i  Surfpals: %i", RemakeCalled,FrameLocksCounter,SurfPalBuilds);

	FSpanBuffer* InitialSpan = Span;
	FVector FinalLight,FinalFog;

	if ( Z < 2.0) // on top of screenflash....
	{
		FinalLight.R = MinPositiveFloat(GMasterScale*Light.R, GMaxColor.R);
		FinalLight.G = MinPositiveFloat(GMasterScale*Light.G, GMaxColor.G);
		FinalLight.B = MinPositiveFloat(GMasterScale*Light.B, GMaxColor.B);
		FinalFog.R = 0.f;
		FinalFog.G = 0.f;
		FinalFog.B = 0.f;
	}
	else
	if (PolyFlags & PF_RenderFog)   // any VOLUMETRIC FOG supplied ? (Code currently NEVER does that on sprites tho...)
	{
		FinalFog.R		= MinPositiveFloat( Fog.R * GFloatScale.R + GFloatFog.R, GMaxColor.R );
		FinalFog.G		= MinPositiveFloat( Fog.G * GFloatScale.G + GFloatFog.G, GMaxColor.G );
		FinalFog.B		= MinPositiveFloat( Fog.B * GFloatScale.B + GFloatFog.B, GMaxColor.B );
		FinalLight.R	= MinPositiveFloat( Light.R * GFloatScale.R, GMaxColor.R - FinalFog.R );
		FinalLight.G	= MinPositiveFloat( Light.G * GFloatScale.G, GMaxColor.G - FinalFog.G );
		FinalLight.B	= MinPositiveFloat( Light.B * GFloatScale.B, GMaxColor.B - FinalFog.B );
	}
	else // Use only global fog.
	{
		FinalFog.R		= GFloatFog.R;
		FinalFog.G		= GFloatFog.G;
		FinalFog.B		= GFloatFog.B;
		FinalLight.R	= MinPositiveFloat( Light.R * GFloatScale.R, GFloatRange.R );
		FinalLight.G	= MinPositiveFloat( Light.G * GFloatScale.G, GFloatRange.G );
		FinalLight.B	= MinPositiveFloat( Light.B * GFloatScale.B, GFloatRange.B );		
	}

	//
	// If none given, use a cached screenwide spanbuffer for clipping.
	//
	if ( !Span ) 
	{
		static FSpanBuffer TempSpanBuffer;
		static INT SavedX=0, SavedY=0;
		static FSpan *SpanIndex[1200], DefaultSpan;
		if( SavedX!=Viewport->SizeX || SavedY!=Viewport->SizeY )
		{
			SavedX = Viewport->SizeX;
			SavedY = Viewport->SizeY;
			TempSpanBuffer.Index  = SpanIndex;
			TempSpanBuffer.StartY = 0;
			TempSpanBuffer.EndY   = Viewport->SizeY;
			DefaultSpan.Start = 0;
			DefaultSpan.End   = Viewport->SizeX;
			DefaultSpan.Next  = 0;
			for( INT i=0; i<Viewport->SizeY; i++ ) SpanIndex[i]  = &DefaultSpan;
		}
		Span = &TempSpanBuffer;
	}

	// Calculate integer coords, clip Y against spanbuffer.

	Spr.MinX = appRound(X);
	Spr.MaxX = appRound(X+XL);
	Spr.MinY = appRound(Y);
	Spr.MaxY = appRound(Y+YL); 

	// 'NaN/#IND' coordinate inputs become  0x80000000 on FISTP's and cause occasional crashes in
	// DrawTile and DrawPoly..


	// #debug clipping check -> Should not happen, since clipped at a higher level... 
#if DO_SLOW_GUARD

	if (Spr.MinX == 0x80000000) appErrorf("-Maxint coordinate MinX detected.");
	if (Spr.MaxX == 0x80000000) appErrorf("-Maxint coordinate MaxX detected.");
	if (Spr.MinY == 0x80000000) appErrorf("-Maxint coordinate MinY detected.");
	if (Spr.MaxY == 0x80000000) appErrorf("-Maxint coordinate MaxY detected.");

	// check clipping bounds
	if ( (Spr.MinX < 0) ||
		 (Spr.MinY < 0) ||
		 (Spr.MaxX >Viewport->SizeX) ||
		 (Spr.MaxY >Viewport->SizeY) )
		appErrorf(" Warning - BAD tile clipping detected: X Y X1 Y1  %i %i %i %i ", Spr.MinX, Spr.MinY, Spr.MaxX, Spr.MaxY );

	if ( (Spr.MinX > Spr.MaxX) || (Spr.MinY > Spr.MaxY) )
		appErrorf(" Warning - BAD tile size detected: X Y X1 Y1  %i %i %i %i ", Spr.MinX, Spr.MinY, Spr.MaxX, Spr.MaxY );


	/*
	//
	// Check spanbuffer validity.
	//
	{
		// Walk over the whole spanbuffer and check the sizes against the viewport.
		//
		// other things that *MIGHT* go wrong:
		// - triangle vertices are outside the screenbounds, and read outside the span->index
		//   when looking into the span for clipping ? (a maxY/MinY problem)
		// - some floating-point dependency ?????   
		//

		INT MinY = Span->StartY;
		INT MaxY = Span->EndY;

		FSpan** Index   = Span->Index;

		if (MinY < 0) appErrorf(" SpanStartY negative!");
		if (MaxY > Viewport->SizeY) appErrorf(" SpanEndY bigger than ViewportSizeY!");

		for( INT Y=MinY; Y<MaxY; Y++ )
		{
			for( FSpan* Span=*Index++; Span; Span=Span->Next )
			{
				INT X0 = Span->Start;
				INT X1 = Span->End;	

				if ( (X0 <0) || ( X1 > Viewport->SizeX))
					appErrorf( "Span out of bounds. Ypos: %i XStart: %i XEnd: %i",Y, Span->Start,Span->End);
			}
		}
		*/
#endif

	
	INT OldMinY = Spr.MinY;
	Spr.MinY	= Max( Spr.MinY,Span->StartY);
	INT ClipY   = Spr.MinY - OldMinY;
	Spr.MaxY	= Min( Spr.MaxY,Span->EndY);

	Spr.SizeX	= Spr.MaxX-Spr.MinX;  
	Spr.SizeY	= Spr.MaxY-Spr.MinY;

	// check sizes, AND see if we actually have a palette and a texture... superfluous checks ?
	if ( (Spr.SizeX <= 0) || (Spr.SizeY <= 0) || (! (&Texture)) || (! Texture.Palette) )  return;

	LitPalette* SpritePalet = NULL;
	INT RemakeColors = 0;
	INT NewPaletteSize = NUM_PAL_COLORS * 4;
	
	FCacheItem* Item; 

	BYTE PaletteCID = CID_LitTilePal;
	if (PolyFlags & (PF_Translucent | PF_Modulated))
	{
		// Transparent special cases.
		if (PolyFlags & PF_Translucent) 
		{	
			//if (! ((GIsMMX) && (Viewport->ColorBytes ==4)) ) // Special palette xcept for MMX32bit translucency.
			PaletteCID = CID_LitTileTrans;
		}
		else // MMX+Modulation+32-bits: => use special 15:15:15:15 MMX palette.
		{
			PaletteCID = CID_LitTileMod;// MMX 32-bit modulation 
			/*
			if ( (GIsMMX)  && (Viewport->ColorBytes == 4)  ) 
			{
				PaletteCID = CID_LitTileMod;// MMX 32-bit modulation 
			}
			else // non (MMX32) modulation schemes.
			{
				PaletteCID = CID_LitTileMod;
			}
			*/
		}
	}


	QWORD CacheID = ((Texture.PaletteCacheID)&~(QWORD)255) + PaletteCID + 65536 * Viewport->ByteID();
	SpritePalet = (LitPalette*)GCache.Get(CacheID,Item);

	if ( SpritePalet ) 
	{
		RemakeColors = 0;					
		if ( (SpritePalet->PalLight != FinalLight) ||(SpritePalet->PalFog != FinalFog) )
			RemakeColors = 1;
	}
	else
	{
		// Allocate a new Spritepalet, including the (colordepth specific) palette memory.
		SpritePalet = (LitPalette*)GCache.Create( CacheID, Item, sizeof( LitPalette ) +  NewPaletteSize );
		SpritePalet->OutPalette.PtrVOID =  ((BYTE*)&SpritePalet->OutPalette + 4) ; // point to allocated palette 
		RemakeColors = 1;
	}


	// Rebuild pret-a-ecrire couleurs.  
	// Except for Translucent+Modulated+MMX: then we'll need MMX format, for multiplication....
	// And for Translucent+Modulated+NonMMX: we'd like FLOATS just like drawpoly ?->
	// nope->  Translucent NonMMX benefits from having a DWORD table also, but with the 
	// potential-overflow-bits cleared (or set...) to do our trick....

	// Concern: what happens when we cache a palette for MODULATION and another one in another format
	// but for the same source PALETTE, for eg. translucency ???? -> just factor the 
	// poly flags into the Cache ID ?
	

	// #debug Optimize palette setters below - if needed.

	if (RemakeColors)
	{
		// No fog for translucent & modulated 
		// Only fog if Z >=1.0 else it's unfogged hud/menu stuff.
		if ( !(PolyFlags  & (PF_Modulated|PF_Translucent)) && (Z >= 2.0) )
		{
			FinalFog.R =  255.0 * FinalFog.R;
			FinalFog.G =  255.0 * FinalFog.G;
			FinalFog.B =  255.0 * FinalFog.B;
		}
		else FinalFog = FVector(0,0,0);
		
		if (PaletteCID == CID_LitTileMMX)
		{
			for (INT C=0; C<NUM_PAL_COLORS; C++)
			{
				SpritePalet->OutPalette.PtrWORD[C*4+0] = Min( appRound(FinalLight.B * Texture.Palette[C].B + FinalFog.B),255) << 7;
				SpritePalet->OutPalette.PtrWORD[C*4+1] = Min( appRound(FinalLight.G * Texture.Palette[C].G + FinalFog.G),255) << 7;
				SpritePalet->OutPalette.PtrWORD[C*4+2] = Min( appRound(FinalLight.R * Texture.Palette[C].R + FinalFog.R),255) << 7;
			}
		}
		else if (PaletteCID == CID_LitTileMod) 
		{
			for (INT C=0; C<NUM_PAL_COLORS; C++)
			{
				// integer color palette.
				SpritePalet->OutPalette.PtrBYTE[C*4+0] = Texture.Palette[C].B ;
				SpritePalet->OutPalette.PtrBYTE[C*4+1] = Texture.Palette[C].G ;
				SpritePalet->OutPalette.PtrBYTE[C*4+2] = Texture.Palette[C].R ;
				SpritePalet->OutPalette.PtrBYTE[C*4+3] = 0;
			}	
		}
		else if (Viewport->ColorBytes == 4)
		{	
			if (PaletteCID == CID_LitTilePal)
				for (INT C=0; C<NUM_PAL_COLORS; C++)
				{
					SpritePalet->OutPalette.PtrBYTE[C*4+0] = Min( appRound(FinalLight.B * Texture.Palette[C].B + FinalFog.B),255);
					SpritePalet->OutPalette.PtrBYTE[C*4+1] = Min( appRound(FinalLight.G * Texture.Palette[C].G + FinalFog.G),255);
					SpritePalet->OutPalette.PtrBYTE[C*4+2] = Min( appRound(FinalLight.R * Texture.Palette[C].R + FinalFog.R),255);
				}
			else if (PaletteCID == CID_LitTileTrans)
				for (INT C=0; C<NUM_PAL_COLORS; C++)
				{
					SpritePalet->OutPalette.PtrBYTE[C*4+0] =       Min( appRound(FinalLight.B * Texture.Palette[C].B + FinalFog.B),255);
					SpritePalet->OutPalette.PtrBYTE[C*4+1] = 254 & Min( appRound(FinalLight.G * Texture.Palette[C].G + FinalFog.G),255);
					SpritePalet->OutPalette.PtrBYTE[C*4+2] = 254 & Min( appRound(FinalLight.R * Texture.Palette[C].R + FinalFog.R),255);
					SpritePalet->OutPalette.PtrBYTE[C*4+3] = 0; // needed for overflow determination
				}
		}
		else if (Viewport->Caps & CC_RGB565) 
		{
			if (PaletteCID == CID_LitTilePal)			
				for (INT C=0; C<NUM_PAL_COLORS; C++)
				{
					DWORD R = Min( appRound(FinalLight.R * Texture.Palette[C].R + FinalFog.R),255);
					DWORD G = Min( appRound(FinalLight.G * Texture.Palette[C].G + FinalFog.G),255);
					DWORD B = Min( appRound(FinalLight.B * Texture.Palette[C].B + FinalFog.B),255);
					SpritePalet->OutPalette.PtrDWORD[C] = (_WORD) ( (DWORD)((R&0xf8)<<8 ) + (DWORD)((G&0xfc)<< 3) + (DWORD)((B&0xf8)>>3) );
				}
			else if (PaletteCID == CID_LitTileTrans)
				for (INT C=0; C<NUM_PAL_COLORS; C++)
				{
					DWORD R = Min( appRound(FinalLight.R * Texture.Palette[C].R + FinalFog.R),255);
					DWORD G = Min( appRound(FinalLight.G * Texture.Palette[C].G + FinalFog.G),255);
					DWORD B = Min( appRound(FinalLight.B * Texture.Palette[C].B + FinalFog.B),255);
					SpritePalet->OutPalette.PtrDWORD[C] = (_WORD) ( (DWORD)((R&0xf0)<<8 ) + (DWORD)((G&0xf8)<< 3) + (DWORD)((B&0xf8)>>3) );
				}
		}
		else
		{
			if (PaletteCID == CID_LitTilePal)			
				for (INT C=0; C<NUM_PAL_COLORS; C++)
				{
					DWORD R = Min( appRound(FinalLight.R * Texture.Palette[C].R + FinalFog.R),255);
					DWORD G = Min( appRound(FinalLight.G * Texture.Palette[C].G + FinalFog.G),255);
					DWORD B = Min( appRound(FinalLight.B * Texture.Palette[C].B + FinalFog.B),255);
					SpritePalet->OutPalette.PtrDWORD[C] = (_WORD) ( (DWORD)((R&0xf8)<<7 ) + (DWORD)((G&0xf8)<< 2) + (DWORD)((B&0xf8)>>3) );
				}
			else if (PaletteCID == CID_LitTileTrans)
				for (INT C=0; C<NUM_PAL_COLORS; C++)
				{
					DWORD R = Min( appRound(FinalLight.R * Texture.Palette[C].R + FinalFog.R),255);
					DWORD G = Min( appRound(FinalLight.G * Texture.Palette[C].G + FinalFog.G),255);
					DWORD B = Min( appRound(FinalLight.B * Texture.Palette[C].B + FinalFog.B),255);
					SpritePalet->OutPalette.PtrDWORD[C] = (_WORD) ( (DWORD)((R&0xf0)<<7 ) + (DWORD)((G&0xf0)<< 2) + (DWORD)((B&0xf8)>>3) );
				}
		}
		SpritePalet->PalFog   = FinalFog;
		SpritePalet->PalLight = FinalLight;		
	}// remakecolors

	guardSlow(USoftwareRenderDevice::PrepareSpr);

	//
	// Digest texture->UScale, VScale: first a quick check to see if they're both ONE ...
	// if so, leave  U/UL/V/VL unscaled.
	//

	if ( (1.f != Texture.UScale) || (1.f != Texture.VScale) )
	{
		FLOAT InvTexUScale = 1.f/Texture.UScale;
		FLOAT InvTexVScale = 1.f/Texture.VScale;

		U  *=InvTexUScale;
		UL *=InvTexUScale;
		V  *=InvTexVScale;
		VL *=InvTexVScale;
	}

	// 1:1 scaling detector
	if ( EqualPositiveFloat(XL,UL) && EqualPositiveFloat(YL,VL) ) PolyFlags |= PF_RenderHint;

	// Prepare setup globals structure Spr.
	if ( (PolyFlags & PF_RenderHint) && !(PolyFlags & (PF_Translucent|PF_Modulated)) )
	{
		// Superfast blitters for unscaled normal & masked canvas items, 0th mipmap...
		Spr.UCo		 = appRound(U);
		Spr.VCo		 = appRound(V);
		Spr.USize    = Texture.Mips[0]->USize;
		Spr.VSize    = Texture.Mips[0]->VSize;
		Spr.UMask    = Texture.Mips[0]->USize-1;
		Spr.VMask    = Texture.Mips[0]->VSize-1;
		Spr.TexBase	 = Texture.Mips[0]->DataPtr;
		Spr.UBits    = Texture.Mips[0]->UBits;
		Spr.Palette  = SpritePalet->OutPalette;
		Spr.Screen   = Frame->Screen(0, Spr.MinY );

		if (PolyFlags & PF_Masked) 
		{
			if	(Viewport->ColorBytes == 4)	
				BlitMask32( InitialSpan ); 
			else
				BlitMask1516( InitialSpan );
		}
		else
		{
			if	(Viewport->ColorBytes == 4)	
				BlitTile32( InitialSpan ); 
			else
				BlitTile1516( InitialSpan );
		}
		Item->Unlock(); 	
		return;
	}
	else
	{
		// Compute fixed-point deltas, conclude mipmap, then readjust deltas accordingly.

		/*
		if ( (appRound(XL) == appRound(UL)) && (appRound(YL) == appRound(VL))) 
			debugf(NAME_Log," 1:1 scaled texture detected: XL UL YL VL %e  %e  %e %e ",XL,UL,YL,VL );
		*/

		int iMip = 0;
		FLOAT FUDelta	= (UL/XL);
		FLOAT FVDelta   = (VL/YL);

		// Conclude mipmap only IF any are available.
		if (Texture.NumMips > 1)
		{
			INT MipFactor = Max( ( ( (*(DWORD*)&FUDelta) &0x7FFFFFFF) + 0x600000 ) >> 23, 
				                 ( ( (*(DWORD*)&FVDelta) &0x7FFFFFFF) + 0x600000 ) >> 23 );
			iMip = Clamp( (MipFactor-127), 0, Texture.NumMips-1 );
		}

		// Combine mip scaling with float -> fixed conversion.
		FLOAT  MipScale = (1 << (18- iMip));
		Spr.UCoDelta	= appRound ( FUDelta * MipScale );
		Spr.VCoDelta	= appRound ( FVDelta * MipScale );
		Spr.UCo			= appRound ( U * MipScale );
		Spr.VCo			= appRound ( ( (FLOAT)ClipY * FVDelta + V )* MipScale );
		FMipmap* Mip	= Texture.Mips[iMip];

		Spr.UMask		= Mip->USize - 1;
		Spr.VMask		= Mip->VSize - 1;
		Spr.USize		= Mip->USize;
		Spr.VSize		= Mip->VSize;
		Spr.UBits		= Mip->UBits;
		Spr.TexBase		= Mip->DataPtr;
		Spr.Palette     = SpritePalet->OutPalette;
		Spr.Screen      = Frame->Screen(0,Spr.MinY);
			
		INT UCoWalk		= Spr.UCo;

		for (int CS	= 0; CS <= ( Spr.MaxX - Spr.MinX); CS++ ) // total pixels + 1 to do
		{
			UCoordTable[CS] = (UCoWalk >> 18) & Spr.UMask;
			UCoWalk += Spr.UCoDelta;
		}
	}


	unguardSlow;

	guardSlow( USoftwareRenderDevice::DrawTile_ActualBlitting);
	//
	// Normal blitting, fast for all targets and doesn't need any MMX:
	//

	if ( !(PolyFlags & (PF_Translucent|PF_Masked|PF_Modulated)) )
	{
		if (Viewport->ColorBytes == 4)
			FlashSprite32Normal( Span, GByteStride);
		else 
			FlashSprite1516Normal( Span, GByteStride); 
		Item->Unlock(); 	
		return;
	}


	// TEST to see if masking works OK: including modulated...
#if TESTMASKALL
	PolyFlags &= ~PF_Modulated;
	PolyFlags &= ~PF_Translucent;
	PolyFlags |=  PF_Masked;
#endif

	//
	// Not translucent or modulated -> then it's masked.
	//

	if ( !(PolyFlags & (PF_Translucent|PF_Modulated))) 
	{
		if (Viewport->ColorBytes == 4)
			FlashSprite32Masked( Span, GByteStride);
		else	
			FlashSprite1516Masked( Span, GByteStride);
		Item->Unlock(); 	
		return;
	}


	//
	// Modulated and translucent.
	//

	if (GIsMMX)
	{
		if (Viewport->ColorBytes == 4)
		{
		// 32 bit color
			if (PolyFlags & PF_Translucent)
				FlashSprite32TranslucentMMX(Span, GByteStride);
			else if (PolyFlags & PF_Modulated)
			{
#if !ASM    // build table only if assembler disabled
			if (ModLutInit !=32)
				{	
					for (DWORD M=0; M< 1024; M++)
						ModLut[M] =  Min((DWORD)( (M&0x03F) * (M&0x3C0)) >> 7,(DWORD)255) * 0x010101;
					ModLutInit = 32;
				}
#endif
			    FlashSprite32ModulatedMMX(Span, GByteStride);
			}
		}	
		else if (Viewport->Caps & CC_RGB565) 
		{	
		// 16 bit color 
			if (PolyFlags & PF_Translucent)
				FlashSprite16Translucent(Span, GByteStride);
			else if (PolyFlags & PF_Modulated)
			{
				if (ModLutInit !=16)
				{	
					for (DWORD M=0; M<1024; M++)
					{
						ModLut[M] =    (Min((DWORD)( (M&0x01F) * (M&0x3E0)) >>9, (DWORD)31 )      )
									|  (Min((DWORD)( (M&0x01F) * (M&0x3E0)) >>8, (DWORD)63 ) <<5  )
									|  (Min((DWORD)( (M&0x01F) * (M&0x3E0)) >>9, (DWORD)31 ) <<11 );
					}
					ModLutInit = 16;
				}
				FlashSprite16Modulated(Span, GByteStride);
			}
		}		
		else
		{
		// 15 bit color 
			if (PolyFlags & PF_Translucent)
				FlashSprite15Translucent(Span, GByteStride);
			else if (PolyFlags & PF_Modulated)
			{
				if (ModLutInit !=15)
				{	
					for (DWORD M=0; M< 1024; M++)
					{
						ModLut[M] =    (Min((DWORD)( (M&0x01F) * (M&0x3E0)) >>9,(DWORD)31 )      )
									|  (Min((DWORD)( (M&0x01F) * (M&0x3E0)) >>9,(DWORD)31 ) <<5  )
									|  (Min((DWORD)( (M&0x01F) * (M&0x3E0)) >>9,(DWORD)31 ) <<10 );
					}
					ModLutInit = 15;
				}
			    FlashSprite15Modulated(Span, GByteStride);
			}
		}
	}
	else  // All nonmmx routines
	{

		if (Viewport->ColorBytes == 4)
		{
			// 32 bit color
			if (PolyFlags & PF_Translucent)
			{
				FlashSprite32TranslucentPentium(Span, GByteStride);
			}
			else if (PolyFlags & PF_Modulated)
			{
				if (ModLutInit !=32)
				{	
					for (DWORD M=0; M< 1024; M++)
						ModLut[M] =  Min((DWORD)( (M&0x03F) * (M&0x3C0)) >> 7,(DWORD)255) * 0x010101;
					ModLutInit = 32;
				}
			    FlashSprite32Modulated(Span, GByteStride);
			}
		}	
		else if (Viewport->Caps & CC_RGB565) 
		{	
		// 16 bit color 

			if (PolyFlags & PF_Translucent)
				FlashSprite16Translucent(Span, GByteStride);
			else if (PolyFlags & PF_Modulated)
			{
				if (ModLutInit !=16)
				{	
					for (DWORD M=0; M<1024; M++)
					{
						ModLut[M] =    (Min((DWORD)( (M&0x01F) * (M&0x3E0)) >>9, (DWORD)31 )      )
									|  (Min((DWORD)( (M&0x01F) * (M&0x3E0)) >>8, (DWORD)63 ) <<5  )
									|  (Min((DWORD)( (M&0x01F) * (M&0x3E0)) >>9, (DWORD)31 ) <<11 );
					}
					ModLutInit = 16;
				}
				FlashSprite16Modulated(Span, GByteStride);
			}
		}		
		else
		{
		// 15 bit color 

			if (PolyFlags & PF_Translucent)
				FlashSprite15Translucent(Span, GByteStride);
			else if (PolyFlags & PF_Modulated)
			{
				if (ModLutInit !=15)
				{	
					for (DWORD M=0; M< 1024; M++)
					{
						ModLut[M] =    (Min((DWORD)( (M&0x01F) * (M&0x3E0)) >>9,(DWORD)31 )      )
									|  (Min((DWORD)( (M&0x01F) * (M&0x3E0)) >>9,(DWORD)31 ) <<5  )
									|  (Min((DWORD)( (M&0x01F) * (M&0x3E0)) >>9,(DWORD)31 ) <<10 );
					}
					ModLutInit = 15;
				}
				FlashSprite15Modulated( Span, GByteStride );
			}
		}		
	}

	unguardSlow; //Drawtile::ActualBlitting

	Item->Unlock(); 	

	unguardSlow;
}



/*-----------------------------------------------------------------------------
	The end.
-----------------------------------------------------------------------------*/
