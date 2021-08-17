#include "vdp_data.h"

#define PUSH(a)	mov.l	a, @-r15
#define POP(a)	mov.l	@r15+, a

#define tile	r4
#define sq	r5
#define color	r6
#define left	fr0
#define top	fr1
#define right	fr2
#define bottom	fr3
#define topx	fr4
#define bottomx	fr5
#define y	fr6
#define lowz	fr7
#define z	fr7
#define hiz	fr8
#define w	fr9
#define h	fr10

//void DrawTile32PVR_Asm(int tile, void *sq, int color, float topx, float bottomx, float y, float lowz, float hiz, float w, float h)
.globl _DrawTile32PVR_Asm
_DrawTile32PVR_Asm:
		//Set context
		swap.b	tile, r0
		mov.l	.Lpvrtilescxt, r1
		
		and	#VDP_PALMASKHI>>8, r0
		fschg
		
		//Only the first 16 bytes of the context are needed
		fmov.d	@(r0, r1), dr0
		add	#8, r0
		
		fmov.d	@(r0, r1), dr2
		mov	#4, r3
		
		fmov.d	dr0, @sq
		add	#8, sq
		
		fmov.d	dr2, @sq
		cmp/pz	tile		//check priority
		
		pref	@sq
		add	#24, sq
		
		//Read UV table
		mov.l	.Ltiletable, r2
		extu.W	tile, r0
		
		shld	r3, r0
		
		fmov.d	@(r0, r2), dr0
		add	#8, r0
		
		fmov.d	@(r0, r2), dr2
		fschg
		
		//vertex 0
		add	#4*6, sq	//point to argb
		fadd	y, h
		
		mov.l	color, @sq
		//Get depth value (priority checked above)
		bt	1f
		
		fmov	hiz, z
	1:
		fmov.s	bottom, @-sq
		
		fmov.s	left, @-sq
		
		fmov.s	z, @-sq
		
		mov	#0xffffffe0, r1
		fmov.s	h, @-sq
		
		shll16	r1
		fmov.s	bottomx, @-sq
		
		shll8	r1
		mov.l	r1, @-sq	//vertex cmd
		
		pref	@sq
		add	#32+4*6, sq	//point to argb of next vertex
		
		//vertex 1
		fadd	w, bottomx
		mov.l	color, @sq
		
		fmov.s	top, @-sq
		
		fmov.s	left, @-sq
		
		fmov.s	z, @-sq
		
		fmov.s	y, @-sq
		
		fmov.s	topx, @-sq
		fadd	w, topx
		
		mov.l	r1, @-sq	//vertex cmd
		
		pref	@sq
		add	#32+4*4, sq	//point to u of next vertex
		
		//vertex 2
		//reuse existing sq data from vertex 0
		fmov.s	right, @sq
		mov	#-12, r0
		
		fmov.s	bottomx, @(r0, sq)
		sett			//build 0xf0000000 end of strip command
		
		pref	@sq
		add	#32, sq		//point to u of next vertex
		
		//vertex 3
		//reuse existing sq data from vertex 1
		fmov.s	right, @sq
		rotcr	r1		//finish building 0xf0000000 end of strip command
		
		fmov.s	topx, @(r0, sq)
		mov	#-16, r0
		
		mov.l	r1, @(r0, sq)	//end of strip vertex cmd
		
		rts
		 pref	@sq
	
	.align 3	//Put both on same cacheline
	.Ltiletable:
		.long	_TileUV32Table
	.Lcurpvrtilescxt:
		.long	_CurPVRTilesCxt
	.Lpvrtilescxt:
		.long	_PVRTilesCxt

