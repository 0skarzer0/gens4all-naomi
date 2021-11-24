
#define ypos	fr4
#define curcolor	r4
#define linesremaining	r5
#define lastcolor	r6
#define colormask	r7
#define sq	r8
#define rasters	r9
#define one	fr1

//void DrawBGPlane(float ystart);
.globl _DrawBGPlane
_DrawBGPlane:
		mov.l	.Lbgcolors, curcolor
		mov	#224/2, linesremaining
		mov.l	.Lcolormask, colormask
		shll	linesremaining
		
		//Load sprite context
		mova	.Lspritecxt, r0
		fschg
		fmov	@r0+, dr0
		fmov	@r0+, dr2
		fschg
		
	//
		mov.W	@curcolor+, r1
		
		mov	r1, lastcolor
		
		//blue
		mov	r1, r2
		and	colormask, r2
		shlr2	r2
		shlr2	r2
		
		//red
		mov	r1, r0
		and	#0xe, r0
		shll2	r0
		shll2	r0
		shll16	r0
		mov	r0, r3
		
		//green
		mov	r1, r0
		and	#0xe0, r0
		shll8	r0
		
		//combine colors (alpha is left as 0)
		or	r3, r0
		or	r2, r0
		
		//write context cmd
		fschg
		fmov.d	dr0, @sq
		add	#8, sq
		fmov.d	dr2, @sq
		mov.l	r0, @(8, sq)
		add	#24, sq
		pref	@sq
		
		//Write sprite to SQ
		//TODO
		
		mov.b	@rasters+, r0
		tst	#0x04, r0	//Test screen off
		//LOAD DEPTH
		
	//
	1:
		dt	linesremaining
		bt	.Lend
		
		mov.W	@curcolor+, r1
		cmp/eq	r1, lastcolor
		bt	1b
		
		fadd	one, fr4
	.Lend:
		rts
		 nop
	.align 5
	.Lspritecxt:
		.long	0
		.long	0
		.long	0
		.long	0
	.Lsprite:
		.long	0xffffffff //cmd
		.float	0	//left x
		.float	1000	//right x
	.Llineheight:
		.float	1
	.Lbgcolors:
		.long _Raster_BgColor
	.Lrasterlines:
		.long _Raster_Lines
	.Lcolormask:
		.long	0xe00
