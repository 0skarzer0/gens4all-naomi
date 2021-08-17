
#define line7		r0
#define rowcount	r1
#define stride		r2
#define backstride	r3
#define src		r4
#define dst		r5
#define line0		r6
#define line1		r7
#define line2		r8
#define line3		r9
#define line4		r10
#define line5		r11
#define line6		r12
#define count		r13


#include "vdp_data.h"

#define PUSH(reg)	mov.l	reg, @-r15
#define POP(reg)	mov.l	@r15+, reg

//#define SWAP(reg)	swap.w	reg, reg
//#define SWAP(reg)	
.macro SWAP reg
	swap.b	\reg, \reg
	swap.w	\reg, \reg
	swap.b	\reg, \reg
	swap.w	\reg, \reg
.endm

//void VRAMToTexture(void *src, void *dst);
.globl _VRAMToTexture
_VRAMToTexture:
		PUSH(r8)
		PUSH(r9)
		PUSH(r10)
		PUSH(r11)
		PUSH(r12)
		PUSH(r13)
		
		mov.l	.Lconststride, stride
		mov	#TEXTILEROWS, rowcount
		mov.l	.Lconstbackstride, backstride
		
	.align 2
	2:
		mov	#TEXTILECOLS, count
	1: /////////////////////////////////////
		
		mov.l	@src+, line0
		
		dt	count
		mov.l	@src+, line1
		
		SWAP	line0
		mov.l	@src+, line2
		
		SWAP	line1
		mov.l	@src+, line3
		
		SWAP	line2
		mov.l	@src+, line4
		
		SWAP	line3
		mov.l	@src+, line5
		
		SWAP	line4
		mov.l	@src+, line6
		
		SWAP	line5
		mov.l	@src+, line7
		
		SWAP	line6
//		pref	@src
		
		SWAP	line7
		mov.l	line0, @dst
		
		add	stride, dst
		
		mov.l	line1, @dst
		add	stride, dst
		
		mov.l	line2, @dst
		add	stride, dst
		
		mov.l	line3, @dst
		add	stride, dst
		
		mov.l	line4, @dst
		add	stride, dst
		
		mov.l	line5, @dst
		add	stride, dst
		
		mov.l	line6, @dst
		add	stride, dst
		
		mov.l	line7, @dst
		pref	@dst
		bf/s	1b
		 add	backstride, dst
	/////////////////////////////////////
		dt	rowcount
		sub	backstride, dst
		bf/s	2b
		 add	#4, dst
		
		
		POP(r13)
		POP(r12)
		POP(r11)
		POP(r10)
		POP(r9)
		rts
		 POP(r8)


.align 2
	.Lconststride:
		.long	TEXSTRIDE
	.Lconstbackstride:
		.long	4-TEXSTRIDE*(TILEHEIGHT-1)

//void VRAMToTexture(void *src, void *dst);
.globl _VRAMToTexture2
_VRAMToTexture2:
		PUSH(r8)
		mov	#TEXSTRIDE>>8, stride
		PUSH(r9)
		shll8	stride
		PUSH(r10)
		mov	#TEXTILEROWS, rowcount
		PUSH(r11)
		PUSH(r12)
		PUSH(r13)
		
		//mov.l	.Lconstbackstride, backstride
		pref	@dst
		
		rts
		 nop
		 
	
//
