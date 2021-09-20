
#define rowcount	r1
#define tilechunkskip	r2
#define chunkstride	r3
#define tile0		r4
#define dst		r5
#define tile1		r6
#define tile2		r7
#define tile3		r8
#define tile4		r9
#define tile5		r10
#define tile6		r11
#define tile7		r12
#define chunksinrow	r13


#include "vdp_data.h"

.macro SWAP reg
#if 0
	swap.b	\reg, \reg
	swap.w	\reg, \reg
	swap.b	\reg, \reg
#endif
	swap.w	\reg, \reg
.endm

#define CHUNKSIZE	8
#define CHUNKSPERROW	(TEXTILECOLS / CHUNKSIZE)

#define PUSH(reg)	mov.l	reg, @-r15
#define POP(reg)	mov.l	@r15+, reg

/*
	For 64x32 title texture:
	Each chunk is an 8x1 tile block
	There are 8 chunks per row
	There are 32 rows

*/

//void VRAMToTexture(void *src, void *dst);
.globl _VRAM8x8ToTexture2
_VRAM8x8ToTexture2:
		mov.l	.Lconsttilechunkskip, tilechunkskip
		mov.l	.Lconstchunkstride, chunkstride
		
		PUSH(r8)
		PUSH(r9)
		PUSH(r10)
		PUSH(r11)
		PUSH(r12)
		PUSH(r13)
		PUSH(r14)
		
		mov	#TEXTILEROWS, r0
		mov.l	r0, @-r15
		
		mov	tile0, tile1
		add	#TILESIZEOF, tile1
		
		mov	tile1, tile2
		add	#TILESIZEOF, tile2
		
		mov	tile2, tile3
		add	#TILESIZEOF, tile3
		
		mov	tile3, tile4
		add	#TILESIZEOF, tile4
		
		mov	tile4, tile5
		add	#TILESIZEOF, tile5
		
		mov	tile5, tile6
		add	#TILESIZEOF, tile6
		
		mov	tile6, tile7
		add	#TILESIZEOF, tile7
	3:
		mov	#CHUNKSPERROW, chunksinrow
	.align 2
	2:
		mov	#TILEHEIGHT, rowcount
	1: /////////////////////////////////////
	//Writing one row of a chunk each loop
		mov.l	@tile0+, r0
		mov.l	@tile1+, r14
		SWAP	r0
		SWAP	r14
		movca.l	r0, @dst
		mov.l	r14, @(4, dst)
		
		mov.l	@tile2+, r0
		mov.l	@tile3+, r14
		SWAP	r0
		SWAP	r14
		mov.l	r0, @(8, dst)
		mov.l	r14, @(12, dst)
		
		mov.l	@tile4+, r0
		mov.l	@tile5+, r14
		SWAP	r0
		SWAP	r14
		mov.l	r0, @(16, dst)
		mov.l	r14, @(20, dst)
		
		mov.l	@tile6+, r0
		mov.l	@tile7+, r14
		SWAP	r0
		SWAP	r14
		mov.l	r0, @(24, dst)
		mov.l	r14, @(28, dst)
		
		pref	@dst
		dt	rowcount
		bf/s	1b
		 add	chunkstride, dst
	/////////////////////////////////////
	//Finished one chunk
		mov.l	.Lconsttoptilestride, r0
		add	tilechunkskip, tile0
		add	tilechunkskip, tile1
		add	tilechunkskip, tile2
		add	tilechunkskip, tile3
		add	tilechunkskip, tile4
		add	tilechunkskip, tile5
		add	tilechunkskip, tile6
		add	tilechunkskip, tile7
		
		dt	chunksinrow
		bf/s	2b
		 add	r0, dst
		 
	//Finished one row
	//We've gone off the right edge of the texture and wrapped around
	//to the line below, skip down 7 lines
		mov.l	@r15, r0
		mov.l	.Lconst7linestride, r14
		dt	r0
		mov.l	r0, @r15
		bf/s	3b
		 add	r14, dst
	
	//Finished texture
		
		add	#4, r15
		POP(r14)
		POP(r13)
		POP(r12)
		POP(r11)
		POP(r10)
		POP(r9)
		mov	dst, r0
		rts
		 POP(r8)


.align 5
	.Lconst7linestride:
		.long	TEXSTRIDE * 7
	.Lconsttoptilestride:
		.long	-(TEXSTRIDE * 8) + TILESTRIDE * CHUNKSIZE
	.Lconstchunkstride:
		.long	32 + TEXSTRIDE - TILESTRIDE * CHUNKSIZE
	.Lconsttilechunkskip:
		.long	TILESIZEOF * (CHUNKSIZE-1)


//
