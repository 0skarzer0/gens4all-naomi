.globl _memsetl_cl
.globl _memcpy_cl
_memsetl_cl:
		mov	r5, r0
	1:
		movca.l	r0, @r4
		mov.l	r0, @(4, r4)
		dt	r6
		mov.l	r0, @(8, r4)
		mov.l	r0, @(12, r4)
		mov.l	r0, @(16, r4)
		mov.l	r0, @(20, r4)
		mov.l	r0, @(24, r4)
		mov.l	r0, @(28, r4)
	//	ocbwb	@r4
		bf/s	1b
		 add	#32, r4
		
		rts
		 nop

_memcpy_cl:
	//pref	@r5
	//mov	r5, r2
	//add	#32, r2
1:
	//pref	@r2
		mov.l	@r5, r0
	//add	#32, r2
		mov.l	@(4, r5), r1
		dt	r6
		movca.l	r0, @r4
		mov.l	r1, @(4, r4)
		mov.l	@(8, r5), r0
		mov.l	@(12, r5), r1
		mov.l	r0, @(8, r4)
		mov.l	r1, @(12, r4)
		mov.l	@(16, r5), r0
		mov.l	@(20, r5), r1
		mov.l	r0, @(16, r4)
		mov.l	r1, @(20, r4)
		mov.l	@(24, r5), r0
		mov.l	@(28, r5), r1
		add	#32, r5
		mov.l	r0, @(24, r4)
		mov.l	r1, @(28, r4)
		bf/s	1b
		 add	#32, r4
		rts
		 nop
		
