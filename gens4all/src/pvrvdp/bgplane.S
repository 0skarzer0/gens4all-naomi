

#define rgbits	r2
#define	src	r4
#define	count	r5
#define	dst	r6
#define	cachemask	r8
#define	lastcolor	r3
#define convcolor	r0

.align 5
.globl _MakeBGTex
_MakeBGTex:
		mov.l	cachemask, @-r15
		mov	#32/4-1, cachemask
		mov.l	.Lrgbits, rgbits
		mov	#1, lastcolor
		shll16	lastcolor
	.align 2
	1:
		mov.w	@src+, r1	//?BGR
		tst	count, count
		add	#-1, count
		bt	.Lend
		
		//Same color as last?
		cmp/eq	r1, lastcolor
		bt/s	2f
		 tst	cachemask, count
		
		mov	r1, lastcolor
		mov	r1, r0
		
		mov	r0, r7		//Save G
		swap.b	r0, r1		//GR?B
		and	rgbits, r1	//0R0B
		mov	r7, r0
		and	#0xe0, r0	//00G0
		or	r1, r0		//0RGB
		swap.w	r0, r1
		or	r1, r0
	.align 2
	2:
		mov.l	r0, @dst
		bf	3f
		pref	@dst
	3:
		bra	1b
		 add	#4, dst
	.Lend:
		rts
		 mov.l	@r15+, cachemask
		
	.align 2
	.Lrgbits:
		.long	0x0e0e
