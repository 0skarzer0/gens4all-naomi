#include "vdp_data.h"

#define rgbits	r2
#define loop	r3
#define	src	r4
#define	dst	r5
#define rshift4		r6
#define curcolor	r7

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
		add	#4, curcolor
	1:
		mov.w	@src+, r0	//0BGR
		mov	r0, rshift4	//Save G
		or	#0xf0, r0	//0BfR
		swap.b	r0, r1		//fR0B
		and	rgbits, r1	//fR0B
		mov	rshift4, r0
		and	#0xe0, r0	//00G0
		or	r1, r0		//fRGB
		swap.w	r0, r1
		or	r1, r0
		mov.l	r0, @curcolor
		
		dt	loop
		bf/s	1b
		 add	#4, curcolor
		
		//Write colors to codebook
		mov	#2048 >> 8, r0
		shll8	r0
		add	r0, dst
		
		mov.l	.Lcolorstart, curcolor
		mov	#-4, rshift4
	1:
.rept 4
		mov	curcolor, r0
		shld	rshift4, r0
		and	#0xf << 2, r0
		mov.l	@(r0, r15), r1
		
		mov	curcolor, r0
		and	#0xf << 2, r0
		mov.l	@(r0, r15), r2
		
		mov.l	r1, @-dst
		mov.l	r2, @-dst
		add	#-4, curcolor
.endr
		cmp/pl	curcolor
		bt/s	1b
		 pref	@dst
		
		rts
		 add	#4*16, r15
		
	.align 2
	.Lrgbits:
		.long	0xfe0e
	.Lcolorstart:
		.long	0xff << 2

