#include "vdp_data.h"

#define PUSH(a)	mov.l	a, @-r15
#define POP(a)	mov.l	@r15+, a

//Regs
	#define pvrcxt	r3
	#define mode	r4
	#define tmrow	r5
	#define col	r6
	#define cols	r7
	#define mask	r8
	#define offset	r9
	#define uvtable	r10
	#define lefttop	r11
	#define dst	r12
	#define lastpal	r13
	#define rightbottom	r14

//Front fregs
	#define topx	fr4
	#define topy	fr5
	#define lowx	fr6
	#define lowy	fr7
	#define	hiprio	fr8
	#define	loprio	fr9
	#define w	fr11


#define CMD_OFS	0
#define X0_OFS	4
#define Y0_OFS	8
#define Z0_OFS	12
#define X1_OFS	16
#define Y1_OFS	20
#define Z1_OFS	24
#define X2_OFS	28
#define Y2_OFS	32
#define Z2_OFS	36
#define X3_OFS	40
#define Y3_OFS	44
//Nothing	48
#define UV0_OFS	52
#define UV1_OFS	56
#define UV2_OFS	60


//void DrawRowNoClipAsm(VDPDrawMode *mode, short *tmrow, int firstcol, int cols, float topx, float topy, float lowx, float lowy)
.globl _DrawRowNoClipAsm
_DrawRowNoClipAsm:
		PUSH(r8)
		PUSH(r9)
		PUSH(r10)
		PUSH(r11)
		PUSH(r12)
		PUSH(r13)
		PUSH(r14)
		
		mov	#8*4, r0
		fmov	@(r0, mode), w
		
		mov	#16, r0
		fmov	@(r0, mode), hiprio
		mov	#20, r0
		fmov	@(r0, mode), loprio
		
		mov	#3, r2
		mov.l	@(4, mode), pvrcxt
		mov.l	@(8, mode), offset
		mov.l	@(12, mode), mask
		
		mov	#-1, lastpal
		mov.l	.Ltiletable, uvtable
		//mov.l	.Lvertex_cmd, vert_cmd
		mov.l	.Lsq, dst
		
		add	col, col
		add	mask, mask
	.align 2
	1:
		mov	col, r0
		and	mask, r0
		
		add	#2, col
		mov.W	@(r0, tmrow), r1
		
		//Get palette
		extu.W	r1, r1
		add	offset, r1
		
		swap.b	r1, r0
		and	#VDP_PALMASKHI>>8, r0
		
		cmp/eq	r0, lastpal
		mov	r0, lastpal
		bt	2f
		
		//Write context command to sq and submit
		fschg
		fmov.d	@(r0, pvrcxt), xd0
		add	#8, r0
		fmov	@(r0, pvrcxt), xd2
		add	#8, r0
		
		fmov.d	xd0, @dst
		add	#8, dst
		fmov.d	xd2, @dst
		add	#8, dst
		
		fmov.d	@(r0, pvrcxt), xd0
		add	#8, r0
		fmov	@(r0, pvrcxt), xd2
		add	#8, r0
		
		fmov.d	xd0, @dst
		add	#8, dst
		//fmov.d	xd2, @dst
		
		pref	@dst
		add	#8, dst
		fschg
		
	2:
		
		
		//Get UVs
		mov	r1, r0
		shld	r2, r0
		mov.l	@(r0, uvtable), lefttop
		add	#4, r0
		mov.l	@(r0, uvtable), rightbottom
		
		//Write quad
		mov	#-1, r0
		mov.l	r0, @dst
		
		mov	#X0_OFS, r0
		fmov	topx, @(r0, dst)
		
		fadd	w, topx
		fmov	loprio,	fr0
		
		mov	#X3_OFS, r0
		fmov	lowx, @(r0, dst)
		
		fadd	w, lowx
		
		mov	#Y0_OFS, r0
		fmov	topy, @(r0, dst)
		
		mov	#Y1_OFS, r0
		fmov	topy, @(r0, dst)
		
		
		swap.b	r1, r0
		tst	#0x80, r0
		bt	2f
		fmov	hiprio,	fr0
	2:
	
		mov	#Z0_OFS, r0
		fmov	fr0, @(r0, dst)
		
		mov	#Z1_OFS, r0
		fmov	fr0, @(r0, dst)
		
		mov	#X1_OFS, r0
		fmov	topx, @(r0, dst)
		
		mov	#X2_OFS, r0
		fmov	lowx, @(r0, dst)
		
		pref	@dst
		dt	cols
		
		mov	#Y2_OFS, r0	//second half
		fmov	lowy, @(r0, dst)
		
		mov	#Y3_OFS, r0
		fmov	lowy, @(r0, dst)
		
		mov	#Z2_OFS, r0
		fmov	fr0, @(r0, dst)
		
		mov.l	lefttop, @(UV0_OFS, dst)
		shll16	lefttop
		
		mov.l	rightbottom, @(UV2_OFS, dst)
		shlr16	rightbottom
		
		xtrct	rightbottom, lefttop
		
		mov.l	lefttop, @(UV1_OFS, dst)
		add	#32, dst
		
		pref	@dst
		bf/s	1b
		 add	#32, dst
		
		
		POP(r14)
		POP(r13)
		POP(r12)
		POP(r11)
		POP(r10)
		POP(r9)
		rts
		 POP(r8)
		
		
	.align 5	//Put on same cacheline
	.Ltiletable:
		.long	_TileUV16Table
	.Lvertex_cmd:
		.long	0xe0000000
	.Lsq:
		.long	0xe0000000

