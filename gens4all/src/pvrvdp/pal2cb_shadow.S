#include "vdp_data.h"

#define rgbits	r2
#define loop	r3
#define	src	r4
#define	dst	r5
#define rshift4		r6
#define curcolor	r7
#define left		r6
#define right		r7

.align 5
.globl _PalToCodebook4SH
_PalToCodebook4SH:
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
		
		mov.l	.Lshadowval, r0
		mov.l	r0, @(15*4, curcolor)
		mov.l	.Llightval, r0
		mov.l	r0, @(14*4, curcolor)
		
		//Write colors to codebook
		mov	#8, r0
		mov	#16, loop	
		mov	r15, left
	.align 2
	1:
		fmov.s	@left+, fr1
		mov	r15, right
		dt	loop
		
		//1
		fmov.s	@right+, fr0
		fmov.s	@right+, fr2
		fmov	fr1, fr3
		fschg
		
		fmov.d	dr0, @dst
		fmov.d	dr2, @(r0,dst)
		fschg
		
		fmov.s	@right+, fr0
		add	#16, dst
		fmov.s	@right+, fr2
		fschg
		
		fmov.d	dr0, @dst
		fmov.d	dr2, @(r0,dst)
		fschg
		
		pref	@dst
		add	#16, dst
		
		//2
		fmov.s	@right+, fr0
		fmov.s	@right+, fr2
		fschg
		
		fmov.d	dr0, @dst
		fmov.d	dr2, @(r0,dst)
		fschg
		
		fmov.s	@right+, fr0
		add	#16, dst
		fmov.s	@right+, fr2
		fschg
		
		fmov.d	dr0, @dst
		fmov.d	dr2, @(r0,dst)
		fschg
		
		pref	@dst
		add	#16, dst
		
		//3
		fmov.s	@right+, fr0
		fmov.s	@right+, fr2
		fschg
		
		fmov.d	dr0, @dst
		fmov.d	dr2, @(r0,dst)
		fschg
		
		fmov.s	@right+, fr0
		add	#16, dst
		fmov.s	@right+, fr2
		fschg
		
		fmov.d	dr0, @dst
		fmov.d	dr2, @(r0,dst)
		fschg
		
		pref	@dst
		add	#16, dst
		
		//4
		fmov.s	@right+, fr0
		fmov.s	@right+, fr2
		fschg
		
		fmov.d	dr0, @dst
		fmov.d	dr2, @(r0,dst)
		fschg
		
		fmov.s	@right+, fr0
		add	#16, dst
		fmov.s	@right+, fr2
		fschg
		
		fmov.d	dr0, @dst
		fmov.d	dr2, @(r0,dst)
		fschg
		
		pref	@dst
		bf/s	1b
		 add	#16, dst
	
		
		rts
		 add	#4*16, r15
		
	.align 2
	.Lshadowval:
		.long	0x80008000
	.Llightval:
		.long	0x8fff8fff
	.Lrgbits:
		.long	0xfe0e
	.Lcolorstart:
		.long	0xff << 2

