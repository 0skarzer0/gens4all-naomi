#include "vdp_data.h"

#define rgbits	r2
#define loop	r3
#define	src	r4
#define	dst	r5
#define rshift4		r6
#define curcolor	r7

.align 5
.globl _PalToCodebook4
_PalToCodebook4:
		add	#-4*16, r15
		mov.l	.Lrgbits, rgbits
		mov	#15, loop
		add	#2, src
		
		//Create colors
		mov	r15, curcolor
		mov	#0, r0
		mov.l	r0, @curcolor
	1:
		mov.w	@src+, r0	//xBGR
		add	#4, curcolor
		dt	loop
		mov	r0, rshift4	//Save G
		or	#0xf0, r0	//xBfR
		swap.b	r0, r1		//fRxB
		and	rgbits, r1	//fR0B
		mov	rshift4, r0
		and	#0xe0, r0	//00G0
		or	r1, r0		//fRGB
		swap.w	r0, r1
		or	r1, r0
		bf/s	1b
		 mov.l	r0, @curcolor
				
		//Write colors to codebook
		mov.l	.Lcolorstart, curcolor
		mov	#-4, rshift4
		
		mov	#2048 >> 8, r0
		shll8	r0
		add	r0, dst
	
	.align 2
	1:
		mov	curcolor, r0
		shld	rshift4, r0
		
		and	#0xf << 2, r0
		
		mov.l	@(r0, r15), r1
		mov	curcolor, r0
		
		and	#0xf << 2, r0
		
		mov.l	@(r0, r15), r2
		add	#-4, curcolor
		
		mov.l	r1, @-dst
		mov	curcolor, r0
		
		mov.l	r2, @-dst
		shld	rshift4, r0
		
		and	#0xf << 2, r0
		
		mov.l	@(r0, r15), r1
		mov	curcolor, r0
		
		and	#0xf << 2, r0
		
		mov.l	@(r0, r15), r2
		add	#-4, curcolor
		
		mov.l	r1, @-dst
		mov	curcolor, r0
		
		mov.l	r2, @-dst
		shld	rshift4, r0
		
		and	#0xf << 2, r0
		
		mov.l	@(r0, r15), r1
		mov	curcolor, r0
		
		and	#0xf << 2, r0
		
		mov.l	@(r0, r15), r2
		add	#-4, curcolor
		
		mov.l	r1, @-dst
		mov	curcolor, r0
		
		mov.l	r2, @-dst
		shld	rshift4, r0
		
		and	#0xf << 2, r0
		
		mov.l	@(r0, r15), r1
		mov	curcolor, r0
		
		and	#0xf << 2, r0
		
		mov.l	@(r0, r15), r2
		add	#-4, curcolor
		
		mov.l	r1, @-dst
		cmp/pl	curcolor
		
		mov.l	r2, @-dst
		bt/s	1b
		 pref	@dst
		
		rts
		 add	#4*16, r15
		
	.align 2
	.Lrgbits:
		.long	0xfe0e
	.Lcolorstart:
		.long	0xff << 2

