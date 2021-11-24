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
	#define vert_cmd	r11
	#define dst	r12
	#define lastpal	r13

//Front fregs
	#define topx	fr4
	#define topy	fr5
	#define lowx	fr6
	#define lowy	fr7
	#define splits	dr8
	#define topsplit	fr8
	#define lowsplit	fr9
	#define	hiprio	fr2
	#define	loprio	fr3
	#define w	fr11

//Back fregs
	#define lefttop	dr4
	#define rightbottom	dr6
	#define left	fr4
	#define top	fr5
	#define right	fr6
	#define bottom	fr7

	//Split ratios
	#define splitsback	xd8
	#define topsplit	fr8
	#define lowsplit	fr9

	//Calcuated tex coord Vs
	#define topv	fr10
	#define bottomv	fr12


//void DrawRowAsm(VDPDrawMode *mode, short *tmrow, int firstcol, int cols, float topx, float topy, float lowx, float lowy, float topsplit, float lowsplit)
.globl _DrawRowAsm
_DrawRowAsm:
		PUSH(r8)
		mov	#4, r2
		PUSH(r9)
		PUSH(r10)
		PUSH(r11)
		PUSH(r12)
		PUSH(r13)
		PUSH(r14)
		
		
		//mova	.Ltile_width, r0
		//fmov	@r0, w
		mov	#8*4, r0
		fmov	@(r0, mode), w
		
		mov	#16, r0
		fmov	@(r0, mode), hiprio
		mov	#20, r0
		fmov	@(r0, mode), loprio
		
		mov.l	@(0, mode), pvrcxt
		mov.l	@(8, mode), offset
		mov.l	@(12, mode), mask
		
		mov.l	.Ltiletable, uvtable
		mov.l	.Lvertex_cmd, vert_cmd
		mov.l	.Lsq, dst
		mov	#-1, lastpal
		
		fschg
		fmov	splits, splitsback
		fschg
		
		add	col, col
		add	mask, mask
		
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
		
		//Check if we have a new palette
		cmp/eq	r0, lastpal
		mov	r0, lastpal
		bt/s	2f
		
		//Write context command to sq and submit
		fschg
	//Double fmovs
		fmov.d	@(r0, pvrcxt), xd0
		add	#8, r0
		fmov	@(r0, pvrcxt), xd2
		add	#8, r0
		
		fmov.d	xd0, @dst
		add	#8, dst
		fmov.d	xd2, @dst
		add	#8, dst
		
		pref	@dst
		add	#16, dst
	2:
		//Get UVs
		mov	r1, r0
		frchg
		shld	r2, r0
	//Using back fregs	
		fmov.d	@(r0, uvtable), lefttop
		add	#8, r0
		
		fmov.d	@(r0, uvtable), rightbottom
		fschg
	//Single fmovs
		//Calc split V coords
		//Want:
		//	topv    = (bottom - top) * topsplit + top
		//	bottomv = (bottom - top) * lowsplit + top
		
		//fr0 = bottom - top
		fmov	bottom, fr0
		fsub	top, fr0
		
		//First vertex
		//Color
		add	#4*6, dst
		mov	#-1, r0
		mov.l	r0, @dst
		
		fmov	top, bottomv
		fmac	fr0, lowsplit, bottomv
		fmov	top, topv
		fmac	fr0, topsplit, topv
		
		//Write UVs
		fmov.s	bottomv, @-dst
		fmov.s	left, @-dst
		frchg
	//Using front fregs
		
		//Write position
		//Get priority
		swap.b	r1, r0
		fmov.s	loprio,	@-dst
		tst	#0x80, r0
		bt	2f
		fmov.s	hiprio,	@dst
	2:
		fmov.s	lowy, @-dst
		fmov.s	lowx, @-dst
		fadd	w, lowx
		mov.l	vert_cmd, @-dst
		pref	@dst
		
		//Second vertex
		add	#32 + 4*6, dst
		frchg
	//Using back fregs
		
		///Color
		mov	#-1, r0
		mov.l	r0, @dst
		
		//Write UVs
		fmov.s	topv, @-dst
		fmov.s	left, @-dst
		frchg
	//Using front fregs
	
		//Write position
		fmov.s	loprio,	@-dst
		bt	2f
		fmov.s	hiprio,	@dst
	2:
		fmov.s	topy, @-dst
		fmov.s	topx, @-dst
		fadd	w, topx
		mov.l	vert_cmd, @-dst
		pref	@dst
		
		//Third vertex
		//Only need to write U and X
		add	#32 + 4*4, dst
		frchg
	//Using back fregs
		//Write UV
		fmov.s	right, @dst
		mov	#-12, r0
		frchg
	//Using front fregs
		fmov.s	lowx, @(r0, dst)
		pref	@dst
		
		//Fourth vertex
		//Only need to write U, X, and end of strip
		add	#32, dst
		frchg
	//Using back fregs
		//Write UV
		fmov.s	right, @dst
		frchg
	//Using front fregs
		
		fmov.s	topx, @(r0, dst)
		add	#-4, r0
		//End of strip
		mov	#-1, r1
		mov.l	r1, @(r0, dst)
		pref	@dst
		
		dt	cols
		bf/s	1b
		 add	#16, dst
		
		
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
		.long	_TileUV32Table
	.Lvertex_cmd:
		.long	0xe0000000
	.Lsq:
		.long	0xe0000000

